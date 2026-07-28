/// @file
/// @brief      Portable TR-808 rimshot/claves kernel for tap.808.rim~ — no Max/Min dependency.
/// @details    A circuit-informed model of the Roland TR-808's RS/CL channel (one circuit, a
///             panel switch choosing the voice), after the TR-808 Service Notes (p.6 and the
///             main-board schematic):
///
///             - CLAVES: a single high-Q bridged-T resonator (IC20, with R313 390k switched
///               into the network) — the p.14 chart's ~2500 Hz, ~25 ms class. A pure, woody
///               tick.
///             - RIM SHOT: two resonators — IC20's network re-voiced by SW11 (C118 0.0022 uF
///               in the leg) to the chart's ~1667 Hz, plus IC21's bridged-T at ~455 Hz
///               (computes to 452 Hz from its own parts: R315 5.6k, R316 1M, C115/C116
///               0.0047 uF). Both are cut short (~10 ms class) by the Q62 VCA, whose envelope
///               is the D61/R107/C24/R108 discharge; the Service Notes note this VCA type "is
///               intended to provide many high harmonics in the output signals" — modeled as a
///               gentle tanh drive on the summed resonators, the crack of the rimshot.
///             - Q74 (2SK30A) gates the output around the note to keep the high-Q networks
///               from leaking at idle — free in a digital model, noted for provenance.
///
///             Family contract: trigger(accent) on the 4-14 V bus; deterministic (no noise
///             source in this channel).
///
///             §7.2 calibration (2026-07-17), vs the Fischer 1994 set (unit 103852): the
///             rimshot re-voiced low-dominant (k_rs_hi_mix 0.08) — the real RS spectrum
///             peaks at ~455 Hz (IC21's network), which an even sum buried under the
///             hotter 1667 Hz network (impulse gain grows with fc.Q); measured dominant
///             pitch now 451 Hz vs 468. Decay classes 10 -> 14 ms (RS) and 25 -> 62 ms
///             (CL) for the measured 12.6 / 33.8 ms t40s (ours 11.8 / 33.4); claves
///             pitch +2.3%.
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

        // p.14 chart voicings.
        constexpr double k_rs_hi_hz   = 1667.0; // IC20 network, RS position
        constexpr double k_rs_lo_hz   = 455.0;  // IC21 (computes to 452 Hz from its parts)
        constexpr double k_rs_decay_s = 0.014;
        constexpr double k_cl_hz      = 2500.0;
        constexpr double k_cl_decay_s = 0.062;

        // Resonator balance in the rimshot sum: the high network's impulse gain grows with
        // fc·Q, so an even sum buries the ~455 Hz component the real rimshot is built on.
        constexpr double k_rs_hi_mix = 0.08;

        // Resonator bridges (R308/R316-class values).
        constexpr double k_rim_r_bridge = 820e3;

        // The VCA's harmonic generation (see header): drive into tanh.
        constexpr double k_rs_drive = 2.2;

        constexpr double k_rim_vtrig_min = 4.0;
        constexpr double k_rim_vtrig_max = 14.0;
        constexpr double k_rim_pulse_ms  = 1.0;

        constexpr double k_rim_out_scale = 1.0 / 16.0;

        // Claves summing balance: the CL network (higher fc, higher Q, and no tanh VCA
        // compressing it) leaves the resonator far hotter than the rimshot sum; the hardware
        // brings both switch positions to comparable level at the mix bus. Trimmed so a
        // full-accent claves peaks at ~0.55 (measured), in the rimshot's neighborhood.
        constexpr double k_cl_mix = 0.095;

        /// The TR-808 rimshot/claves channel. `model` 0 = rimshot, 1 = claves.
        template <typename Sample>
        class basic_rim {
            static_assert(is_sample_profile<Sample>,
                          "basic_rim supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            enum model_type { model_rimshot = 0, model_claves = 1 };

            void prepare(Sample sample_rate) {
                m_sr = sample_rate;
                m_env.prepare(sample_rate);
                revoice();
                reset();
            }

            void reset() {
                m_hi.reset();
                m_lo.reset();
                m_env.reset();
                m_pulse_remaining = 0;
                m_vtrig           = Sample(0.0);
            }

            /// 0 = rimshot, 1 = claves (the hardware SW11).
            void set_model(int model) {
                m_model = model == model_claves ? model_claves : model_rimshot;
                revoice();
            }

            int model() const { return m_model; }

            /// Output level, 0..1 (VR16, RS/CL LEVEL).
            void set_level(Sample amount) { m_level = std::clamp(amount, Sample(0.0), Sample(1.0)); }

            /// Swing-VCA drive (the Q62 harmonic VCA). Sentinel < 0 (default) uses each model's
            /// calibrated value — rimshot k_rs_drive (2.2, as always shipped), claves linear (0).
            /// A value >= 0 overrides both models — the experimental hook for the claves fidelity
            /// sweep (plans/tap.808.md; the rimshot already ships saturated).
            void   set_drive(Sample amount) { m_drive = std::max(-Sample(1.0), amount); }
            Sample drive() const { return m_drive; }

            void trigger(Sample accent = Sample(1.0)) {
                const Sample a    = std::clamp(accent, Sample(0.0), Sample(1.0));
                m_vtrig           = Sample(k_rim_vtrig_min) + a * (Sample(k_rim_vtrig_max) - Sample(k_rim_vtrig_min));
                m_pulse_remaining = std::max(1, static_cast<int>(Sample(k_rim_pulse_ms) * Sample(0.001) * m_sr));
                m_env.trigger(m_vtrig / Sample(k_rim_vtrig_max));
            }

            Sample process() {
                Sample v_pulse = Sample(0.0);
                if (m_pulse_remaining > 0) {
                    v_pulse = m_vtrig;
                    --m_pulse_remaining;
                }

                const Sample exc  = v_pulse * Sample(0.02);
                Sample       ring = m_hi.process(exc, Sample(0.0), Sample(0.0));
                if (m_model == model_rimshot) {
                    ring = ring * Sample(k_rs_hi_mix) + m_lo.process(exc, Sample(0.0), Sample(0.0));
                }

                const Sample env = m_env.process();
                if (m_model == model_rimshot) {
                    // The Q62 swing VCA's harmonic generation (Service Notes, RS/CL VCA): the shared
                    // swing_shape (vca.h) IS tanh(d*v)/d, so swing_vca(ring, env, k_rs_drive) is the
                    // exact tanh(ring*k_rs_drive*env)/k_rs_drive this always shipped — now unified.
                    const Sample d = (m_drive < Sample(0.0)) ? Sample(k_rs_drive) : m_drive;
                    return swing_vca(ring, env, d) * m_level * Sample(k_rim_out_scale) * Sample(k_rim_vtrig_max);
                }
                // Claves: linear by default (m_drive < 0 → 0), with the same opt-in swing saturation.
                const Sample d = (m_drive < Sample(0.0)) ? Sample(0.0) : m_drive;
                return swing_vca(ring, env, d) * Sample(k_cl_mix) * m_level * Sample(k_rim_out_scale);
            }

          private:
            static void voice(basic_bridged_t<Sample>& bt, Sample sample_rate, Sample fc, Sample decay_s) {
                const Sample tau = decay_s / Sample(4.6);
                const Sample q   = k_pi_for<Sample> * fc * tau;
                const Sample rl  = Sample(k_rim_r_bridge) / (Sample(4.0) * q * q);
                const Sample c =
                    Sample(1.0) / (Sample(2.0) * k_pi_for<Sample> * fc * std::sqrt(rl * Sample(k_rim_r_bridge)));
                typename basic_bridged_t<Sample>::config cfg;
                cfg.c_arm_in   = c;
                cfg.c_arm_out  = c;
                cfg.r_bridge   = Sample(k_rim_r_bridge);
                cfg.r_inject_a = Sample(0.0);
                cfg.r_inject_b = Sample(0.0);
                cfg.r_leg      = rl;
                bt.configure(cfg);
                bt.prepare(sample_rate);
            }

            void revoice() {
                if (m_model == model_rimshot) {
                    voice(m_hi, m_sr, Sample(k_rs_hi_hz), Sample(k_rs_decay_s));
                    voice(m_lo, m_sr, Sample(k_rs_lo_hz), Sample(k_rs_decay_s));
                    m_env.set_times(Sample(0.1e-3), Sample(k_rs_decay_s) / Sample(4.6));
                }
                else {
                    voice(m_hi, m_sr, Sample(k_cl_hz), Sample(k_cl_decay_s));
                    m_env.set_times(Sample(0.1e-3), Sample(k_cl_decay_s) / Sample(4.6));
                }
            }

            Sample m_sr{Sample(48000.0)};
            int    m_model{model_rimshot};

            basic_bridged_t<Sample> m_hi, m_lo;
            basic_decay_env<Sample> m_env;

            Sample m_level{Sample(1.0)};
            Sample m_drive{-Sample(1.0)}; // swing-VCA drive; <0 = per-model calibrated (RS 2.2 / CL linear)
            Sample m_vtrig{Sample(0.0)};
            int    m_pulse_remaining{0};
        };

        /// The double profile — the golden model.
        using rim = basic_rim<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using rim32 = basic_rim<float>;

    } // namespace tr808
} // namespace tap::tools
