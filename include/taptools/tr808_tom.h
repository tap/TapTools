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
///             Plain C++17, stdlib only, per-sample, allocation-free after prepare().
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <cmath>

#include "bridged_t.h"
#include "swing_vca.h"

namespace taptools {
    namespace tr808 {

        // p.14 chart tuning spans [low, high] in Hz and decay classes (to -40 dB), per size.
        constexpr double k_tom_hz[3][2]     = {{80.0, 100.0}, {120.0, 160.0}, {165.0, 220.0}};
        constexpr double k_conga_hz[3][2]   = {{165.0, 220.0}, {250.0, 310.0}, {370.0, 455.0}};
        constexpr double k_tom_decay_s[3]   = {0.200, 0.130, 0.100};
        constexpr double k_conga_decay_s[3] = {0.180, 0.100, 0.080};

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

        /// The TR-808 tom/conga channel. `size` 0/1/2 = low/mid/high; `model` 0 = tom
        /// (with the noise layer), 1 = conga.
        class tom {
          public:
            enum model_type { model_tom = 0, model_conga = 1 };

            void prepare(double sample_rate) {
                m_sr = sample_rate;
                m_noise_lp.prepare(sample_rate);
                const double tau = 1.0 / (2.0 * k_pi * k_tom_noise_lp_hz);
                m_noise_lp.set(0.0, 1.0, tau, 1.0);
                m_noise_env.prepare(sample_rate);
                retune(true);
                reset();
            }

            void reset() {
                m_bt.reset();
                m_noise_lp.reset();
                m_noise_env.reset();
                m_noise.reset();
                m_pink1 = m_pink2 = m_pink3 = 0.0;
                m_pulse_remaining           = 0;
                m_vtrig                     = 0.0;
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
            void set_tuning(double amount) {
                m_tuning = std::clamp(amount, 0.0, 1.0);
                retune(false);
            }

            /// Output level, 0..1.
            void set_level(double amount) { m_level = std::clamp(amount, 0.0, 1.0); }

            /// Noise-layer seed (toms only audible; deterministic).
            void set_seed(uint64_t seed) { m_noise.set_seed(seed); }

            // -- performance ---------------------------------------------------------------

            void trigger(double accent = 1.0) {
                const double a    = std::clamp(accent, 0.0, 1.0);
                m_vtrig           = k_tomc_vtrig_min + a * (k_tomc_vtrig_max - k_tomc_vtrig_min);
                m_pulse_remaining = std::max(1, static_cast<int>(k_tomc_pulse_ms * 0.001 * m_sr));
                m_noise_env.trigger(m_vtrig / k_tomc_vtrig_max);
            }

            double process() {
                double v_pulse = 0.0;
                if (m_pulse_remaining > 0) {
                    v_pulse = m_vtrig;
                    --m_pulse_remaining;
                }

                // D80/D81: big center-node swings shunt the leg -> higher fc at the attack.
                const double vc = std::abs(m_bt.v_comm());
                const double g  = std::clamp((vc - k_tomc_bend_knee) / k_tomc_bend_span, 0.0, 1.0);
                m_bt.set_leg_resistance(m_r_leg / (1.0 + k_tomc_bend_depth * g));

                const double ring = m_bt.process(v_pulse * 0.1, 0.0, 0.0);

                double noise = 0.0;
                if (m_model == model_tom) {
                    // Kellet-style pinking of the seeded white source.
                    const double w = m_noise.process();
                    m_pink1        = 0.99765 * m_pink1 + w * 0.0990460;
                    m_pink2        = 0.96300 * m_pink2 + w * 0.2965164;
                    m_pink3        = 0.57000 * m_pink3 + w * 1.0526913;
                    const double p = m_pink1 + m_pink2 + m_pink3 + w * 0.1848;
                    noise =
                        swing_vca(m_noise_lp.process(p), m_noise_env.process()) * k_tom_noise_mix * k_tomc_vtrig_max;
                }
                else {
                    m_noise_env.process(); // keep envelope state moving for model switches
                }

                return (ring + noise) * m_level * k_tomc_out_scale;
            }

          private:
            void retune(bool reconfigure) {
                const auto&  span = m_model == model_tom ? k_tom_hz[m_size] : k_conga_hz[m_size];
                const double fc   = span[0] * std::pow(span[1] / span[0], m_tuning);
                const double dec  = m_model == model_tom ? k_tom_decay_s[m_size] : k_conga_decay_s[m_size];
                // -40 dB decay -> amplitude tau -> Q -> leg resistance (Q = sqrt(Rb/Rl)/2).
                const double tau = dec / 4.6;
                const double q   = k_pi * fc * tau;
                m_r_leg          = k_tomc_r_bridge / (4.0 * q * q);
                // Arm capacitance for fc at that leg.
                const double c = 1.0 / (2.0 * k_pi * fc * std::sqrt(m_r_leg * k_tomc_r_bridge));

                if (reconfigure) {
                    bridged_t::config cfg;
                    cfg.c_arm_in   = c;
                    cfg.c_arm_out  = c;
                    cfg.r_bridge   = k_tomc_r_bridge;
                    cfg.r_inject_a = 0.0;
                    cfg.r_inject_b = 0.0;
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
                m_noise_env.set_times(0.3e-3, tau * k_tom_noise_tau_ratio);
            }

            double m_sr{48000.0};
            int    m_size{0};
            int    m_model{model_tom};

            bridged_t   m_bt;
            white_noise m_noise;
            first_order m_noise_lp;
            decay_env   m_noise_env;
            double      m_pink1{0.0}, m_pink2{0.0}, m_pink3{0.0};

            double m_tuning{0.5}, m_level{1.0};
            double m_r_leg{4.7e3}, m_base_c{1e-8};
            double m_vtrig{0.0};
            int    m_pulse_remaining{0};
        };

    } // namespace tr808
} // namespace taptools
