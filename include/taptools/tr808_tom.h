/// @file
/// @brief      Portable TR-808 tom/conga kernel for tap.808.tom~ — no Max/Min dependency.
/// @details    A circuit-informed model of the Roland TR-808's three tom/conga channels
///             (LT/LC, MT/MC, HT/HC — one circuit per size, a panel switch choosing the
///             voice), after the TR-808 Service Notes (p.6): "a multi-feedback bridged
///             T-network ... voices are switched by SW8. While the oscillation is large in
///             amplitude immediately after triggering, it is on a higher frequency due to
///             conductions of D80 and D81, which reduce time constant of the filter. As the
///             resonance is damped, its frequency is lowered ... Pink noise with a slightly
///             longer decay time is mixed for [the toms] to provide artificial reverberation."
///
///             - Resonator: the shared bridged-T core (bridged_t.h), tuned per size/model/knob
///               to the p.14 chart: toms 80-100 / 120-160 / 165-220 Hz with ~200/130/100 ms
///               decays; congas 165-220 / 250-310 / 370-455 Hz with ~180/100/80 ms. The
///               tuning knob (VR11/13/15) sweeps the chart's low..high span (log interp); the
///               decay classes set the leg/bridge ratio (congas ring purer — higher Q — than
///               the noise-damped toms, as on the hardware).
///             - The D80/D81 pitch fall: while the center node swings beyond about a diode
///               drop, the conducting diodes shunt the leg — higher frequency at the attack,
///               relaxing down as the ring decays. Modeled as an amplitude-dependent leg
///               resistance (same mechanism class as the bass drum's Q43 leakage), depth fit
///               to the audible few-percent sag.
///             - Toms mix the seeded pink noise layer (Kellet-style pinking filter) with a
///               slightly longer decay; congas omit it, like the hardware.
///
///             Family contract: trigger(accent) on the 4-14 V bus; seeded determinism.
///
///             §7.2 calibration (2026-07-17), vs the Fischer 1994 set (unit 103852):
///             decay classes re-fit to the measured -40 dB tails — toms 360/250/200 ms,
///             congas 350/170/145 ms (the chart's 200/130/100 and 180/100/80 read as a
///             hotter reference level) — bringing every size/tuning cell within +-11%.
///             Tunings kept from the chart: fundamentals within +-4% at every dial
///             position (low-conga dial-min -11%, their trim). Mix gains renormalized
///             for the doubled Q.
///
///             Plain C++17, stdlib only, per-sample, allocation-free after prepare().
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <cmath>

#include "bridged_t.h"
#include "numeric.h"
#include "swing_vca.h"

namespace tap::tools {
    namespace tr808 {

        // p.14 chart tuning spans [low, high] in Hz per size; decay classes calibrated to
        // the measured -40 dB tails (see the §7.2 note).
        constexpr double k_tom_hz[3][2]     = {{80.0, 100.0}, {120.0, 160.0}, {165.0, 220.0}};
        constexpr double k_conga_hz[3][2]   = {{165.0, 220.0}, {250.0, 310.0}, {370.0, 455.0}};
        constexpr double k_tom_decay_s[3]   = {0.360, 0.250, 0.200};
        constexpr double k_conga_decay_s[3] = {0.350, 0.170, 0.145};

        // The resonator bridge (R218/R276-class 2.2 M on the schematic); leg set per decay.
        constexpr double k_tomc_r_bridge = 2.2e6;

        // D80/D81 pitch fall: leg shunt depth and the conduction knee (one diode drop-ish).
        constexpr double k_tomc_bend_depth = 0.45;
        constexpr double k_tomc_bend_knee  = 0.55;
        constexpr double k_tomc_bend_span  = 0.6;

        // Tom noise layer ("artificial reverberation"): decay stretch and mix level.
        constexpr double k_tom_noise_tau_ratio = 1.4;
        constexpr double k_tom_noise_mix       = 0.16;
        constexpr double k_tom_noise_lp_hz     = 1800.0;

        constexpr double k_tomc_vtrig_min = 4.0;
        constexpr double k_tomc_vtrig_max = 14.0;
        constexpr double k_tomc_pulse_ms  = 1.0;

        constexpr double k_tomc_out_scale = 1.0 / 30.0;

        // Per-channel summing balance. The bridged-T rings hotter as the tuning class rises
        // (its impulse gain grows with fc and Q), so the six channels leave the resonator at
        // very different levels; the hardware evens them out with per-channel summing
        // resistors into the mix bus. Normalized so every channel's full-accent, knob-max
        // peak lands at ~0.9 (measured), keeping the family's output band consistent.
        constexpr double k_tomc_mix[2][3] = {{0.48, 0.29, 0.20}, {0.11, 0.13, 0.09}};

        /// The TR-808 tom/conga channel. `size` 0/1/2 = low/mid/high; `model` 0 = tom
        /// (with the noise layer), 1 = conga.
        template <typename Sample>
        class basic_tom {
            static_assert(is_sample_profile<Sample>,
                          "basic_tom supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            enum model_type { model_tom = 0, model_conga = 1 };

            void prepare(Sample sample_rate) {
                m_sr = sample_rate;
                m_noise_lp.prepare(sample_rate);
                const Sample tau = Sample(1.0) / (Sample(2.0) * k_pi_for<Sample> * Sample(k_tom_noise_lp_hz));
                m_noise_lp.set(Sample(0.0), Sample(1.0), tau, Sample(1.0));
                m_noise_env.prepare(sample_rate);
                retune(true);
                reset();
            }

            void reset() {
                m_bt.reset();
                m_noise_lp.reset();
                m_noise_env.reset();
                m_noise.reset();
                m_pink1 = m_pink2 = m_pink3 = Sample(0.0);
                m_pulse_remaining           = 0;
                m_vtrig                     = Sample(0.0);
            }

            // -- panel / config ------------------------------------------------------------

            void set_size(int size) {
                m_size = std::clamp(size, 0, 2);
                retune(true);
            }

            void set_model(int model) {
                m_model = model == model_conga ? model_conga : model_tom;
                retune(true);
            }

            int size() const { return m_size; }
            int model() const { return m_model; }

            /// Tuning knob, 0..1 (VR11/13/15): sweeps the chart's low..high span.
            void set_tuning(Sample amount) {
                m_tuning = std::clamp(amount, Sample(0.0), Sample(1.0));
                retune(false);
            }

            /// Output level, 0..1.
            void set_level(Sample amount) { m_level = std::clamp(amount, Sample(0.0), Sample(1.0)); }

            /// Swing-VCA drive on the noise "reverberation" path (0 = the calibrated linear model,
            /// bit-identical; > 0 engages the swing VCA's symmetric harmonic saturation on the
            /// noise layer, riding its envelope). Toms only (congas have no noise layer). See
            /// swing_vca.h / vca.h swing_shape.
            void   set_drive(Sample amount) { m_drive = std::max(Sample(0.0), amount); }
            Sample drive() const { return m_drive; }

            /// Noise-layer seed (toms only audible; deterministic).
            void set_seed(uint64_t seed) { m_noise.set_seed(seed); }

            // -- performance ---------------------------------------------------------------

            void trigger(Sample accent = Sample(1.0)) {
                const Sample a = std::clamp(accent, Sample(0.0), Sample(1.0));
                m_vtrig        = Sample(k_tomc_vtrig_min) + a * (Sample(k_tomc_vtrig_max) - Sample(k_tomc_vtrig_min));
                m_pulse_remaining = std::max(1, static_cast<int>(Sample(k_tomc_pulse_ms) * Sample(0.001) * m_sr));
                m_noise_env.trigger(m_vtrig / Sample(k_tomc_vtrig_max));
            }

            Sample process() {
                Sample v_pulse = Sample(0.0);
                if (m_pulse_remaining > 0) {
                    v_pulse = m_vtrig;
                    --m_pulse_remaining;
                }

                // D80/D81: big center-node swings shunt the leg -> higher fc at the attack.
                const Sample vc = std::abs(m_bt.v_comm());
                const Sample g =
                    std::clamp((vc - Sample(k_tomc_bend_knee)) / Sample(k_tomc_bend_span), Sample(0.0), Sample(1.0));
                m_bt.set_leg_resistance(m_r_leg / (Sample(1.0) + Sample(k_tomc_bend_depth) * g));

                const Sample ring = m_bt.process(v_pulse * Sample(0.1), Sample(0.0), Sample(0.0));

                Sample noise = Sample(0.0);
                if (m_model == model_tom) {
                    // Kellet-style pinking of the seeded white source.
                    const Sample w = m_noise.process();
                    m_pink1        = Sample(0.99765) * m_pink1 + w * Sample(0.0990460);
                    m_pink2        = Sample(0.96300) * m_pink2 + w * Sample(0.2965164);
                    m_pink3        = Sample(0.57000) * m_pink3 + w * Sample(1.0526913);
                    const Sample p = m_pink1 + m_pink2 + m_pink3 + w * Sample(0.1848);
                    noise = swing_vca(m_noise_lp.process(p), m_noise_env.process(), m_drive) * Sample(k_tom_noise_mix)
                            * Sample(k_tomc_vtrig_max);
                }
                else {
                    m_noise_env.process(); // keep envelope state moving for model switches
                }

                return (ring + noise) * m_level * Sample(k_tomc_mix[m_model][m_size]) * Sample(k_tomc_out_scale);
            }

          private:
            void retune(bool reconfigure) {
                const auto&  span = m_model == model_tom ? k_tom_hz[m_size] : k_conga_hz[m_size];
                const Sample fc   = Sample(span[0]) * std::pow(Sample(span[1]) / Sample(span[0]), m_tuning);
                const Sample dec =
                    m_model == model_tom ? Sample(k_tom_decay_s[m_size]) : Sample(k_conga_decay_s[m_size]);
                // -40 dB decay -> amplitude tau -> Q -> leg resistance (Q = sqrt(Rb/Rl)/2).
                const Sample tau = dec / Sample(4.6);
                const Sample q   = k_pi_for<Sample> * fc * tau;
                m_r_leg          = Sample(k_tomc_r_bridge) / (Sample(4.0) * q * q);
                // Arm capacitance for fc at that leg.
                const Sample c =
                    Sample(1.0) / (Sample(2.0) * k_pi_for<Sample> * fc * std::sqrt(m_r_leg * Sample(k_tomc_r_bridge)));

                if (reconfigure) {
                    typename basic_bridged_t<Sample>::config cfg;
                    cfg.c_arm_in   = c;
                    cfg.c_arm_out  = c;
                    cfg.r_bridge   = Sample(k_tomc_r_bridge);
                    cfg.r_inject_a = Sample(0.0);
                    cfg.r_inject_b = Sample(0.0);
                    cfg.r_leg      = m_r_leg;
                    m_bt.configure(cfg);
                    m_bt.prepare(m_sr);
                    m_base_c = c;
                }
                else {
                    // Knob moves retune via the cap scale (state-preserving).
                    m_bt.set_leg_resistance(m_r_leg);
                    m_bt.set_cap_scale(c / m_base_c);
                }
                m_noise_env.set_times(Sample(0.3e-3), tau * Sample(k_tom_noise_tau_ratio));
            }

            Sample m_sr{Sample(48000.0)};
            int    m_size{0};
            int    m_model{model_tom};

            basic_bridged_t<Sample>   m_bt;
            basic_white_noise<Sample> m_noise;
            basic_first_order<Sample> m_noise_lp;
            basic_decay_env<Sample>   m_noise_env;
            Sample                    m_pink1{Sample(0.0)}, m_pink2{Sample(0.0)}, m_pink3{Sample(0.0)};

            Sample m_tuning{Sample(0.5)}, m_level{Sample(1.0)};
            Sample m_drive{Sample(0.0)}; // swing-VCA saturation on the noise layer; 0 = linear (default)
            Sample m_r_leg{Sample(4.7e3)}, m_base_c{Sample(1e-8)};
            Sample m_vtrig{Sample(0.0)};
            int    m_pulse_remaining{0};
        };

        /// The double profile — the golden model.
        using tom = basic_tom<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using tom32 = basic_tom<float>;

    } // namespace tr808
} // namespace tap::tools
