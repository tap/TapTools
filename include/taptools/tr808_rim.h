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
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <cmath>

#include "bridged_t.h"
#include "swing_vca.h"

namespace taptools {
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
        class rim {
          public:
            enum model_type { model_rimshot = 0, model_claves = 1 };

            void prepare(double sample_rate) {
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
                m_vtrig           = 0.0;
            }

            /// 0 = rimshot, 1 = claves (the hardware SW11).
            void set_model(int model) {
                m_model = model == model_claves ? model_claves : model_rimshot;
                revoice();
            }

            int model() const { return m_model; }

            /// Output level, 0..1 (VR16, RS/CL LEVEL).
            void set_level(double amount) { m_level = std::clamp(amount, 0.0, 1.0); }

            void trigger(double accent = 1.0) {
                const double a    = std::clamp(accent, 0.0, 1.0);
                m_vtrig           = k_rim_vtrig_min + a * (k_rim_vtrig_max - k_rim_vtrig_min);
                m_pulse_remaining = std::max(1, static_cast<int>(k_rim_pulse_ms * 0.001 * m_sr));
                m_env.trigger(m_vtrig / k_rim_vtrig_max);
            }

            double process() {
                double v_pulse = 0.0;
                if (m_pulse_remaining > 0) {
                    v_pulse = m_vtrig;
                    --m_pulse_remaining;
                }

                const double drive = v_pulse * 0.02;
                double       ring  = m_hi.process(drive, 0.0, 0.0);
                if (m_model == model_rimshot) {
                    ring = ring * k_rs_hi_mix + m_lo.process(drive, 0.0, 0.0);
                }

                const double env = m_env.process();
                if (m_model == model_rimshot) {
                    // The Q62 swing VCA's harmonic generation: drive the gated sum.
                    return std::tanh(ring * k_rs_drive * env) / k_rs_drive * m_level * k_rim_out_scale
                           * k_rim_vtrig_max;
                }
                return swing_vca(ring, env) * k_cl_mix * m_level * k_rim_out_scale;
            }

          private:
            static void voice(bridged_t& bt, double sample_rate, double fc, double decay_s) {
                const double      tau = decay_s / 4.6;
                const double      q   = k_pi * fc * tau;
                const double      rl  = k_rim_r_bridge / (4.0 * q * q);
                const double      c   = 1.0 / (2.0 * k_pi * fc * std::sqrt(rl * k_rim_r_bridge));
                bridged_t::config cfg;
                cfg.c_arm_in   = c;
                cfg.c_arm_out  = c;
                cfg.r_bridge   = k_rim_r_bridge;
                cfg.r_inject_a = 0.0;
                cfg.r_inject_b = 0.0;
                cfg.r_leg      = rl;
                bt.configure(cfg);
                bt.prepare(sample_rate);
            }

            void revoice() {
                if (m_model == model_rimshot) {
                    voice(m_hi, m_sr, k_rs_hi_hz, k_rs_decay_s);
                    voice(m_lo, m_sr, k_rs_lo_hz, k_rs_decay_s);
                    m_env.set_times(0.1e-3, k_rs_decay_s / 4.6);
                }
                else {
                    voice(m_hi, m_sr, k_cl_hz, k_cl_decay_s);
                    m_env.set_times(0.1e-3, k_cl_decay_s / 4.6);
                }
            }

            double m_sr{48000.0};
            int    m_model{model_rimshot};

            bridged_t m_hi, m_lo;
            decay_env m_env;

            double m_level{1.0};
            double m_vtrig{0.0};
            int    m_pulse_remaining{0};
        };

    } // namespace tr808
} // namespace taptools
