/// @file
/// @brief      Portable TR-808 handclap/maracas kernel for tap.808.clap~ — no Max/Min dependency.
/// @details    A circuit-informed model of the Roland TR-808 CP/MA voice channel (the hardware
///             switches one circuit between the two sounds; here it's the `model` selector).
///             From the TR-808 Service Notes (first edition, June 15 1981): circuit description
///             and Figure 13 "HAND CLAP GENERATING CYCLE" p.6, schematic p.9 (designators
///             below), p.14 voice chart (CP ~100 ms decay; MA 25-35 ms).
///
///             HANDCLAP — "White noise passed through the band pass filter is applied to two
///             VCAs in parallel to have different envelopes":
///             - Band-pass: the multiple-feedback filter around IC21 — R333 10k in, R332 27k
///               feedback, C128 = C129 = 0.0047 uF -> fc = 1/(2*pi*C*sqrt(R333*R332)) ~= 2.06
///               kHz, Q = sqrt(R332/R333)/2 ~= 0.82. (Implemented exactly from those values;
///               the popular "~1 kHz" folklore does not match the schematic.)
///             - The main VCA (IC22 BA662, driven exponentially via Q71/Q72) gets the "unique
///               sawtooth" envelope from the IC23 (AN6912) generator: per Figure 13, the 30 ms
///               trigger window contains THREE sawtooth teeth ~10 ms apart (C144 0.47 uF
///               abruptly charged, discharged through R365 82k + D71; the third cycle ends the
///               window) — the "multiple hands" transient.
///             - The reverberation VCA (Q70) gets a longer RC decay (D68 charging C143 1 uF,
///               discharged through R362 330k) — the clap's noise wash. Tooth decay, tail
///               decay, and tail level here are behavioral fits to Figure 13's traces and the
///               p.14 chart (~100 ms), since the exponential transistor drive makes the audio
///               decay faster than the raw RC constants.
///
///             MARACAS — "White noise is gated by Q65 ... through the filter Q68; envelope by
///             Q66/Q67": a bright high-passed noise burst. Filter corner and envelope are
///             behavioral (the Q68 stage resists clean closed-form analysis): ~5.5 kHz
///             second-order high-pass, ~0.4 ms rise, tau ~7 ms decay -> the chart's 25-35 ms.
///
///             Both models share the family trigger contract (accent = trigger-bus voltage,
///             scaling the envelope drive) and the seeded deterministic noise source.
///
///             §7.2 calibration (2026-07-17), vs the Fischer 1994 set (unit 103852):
///             clap tail tau 45 -> 100 ms — the chart's "~100 ms" reads as the audible
///             class, while the real tail measures ~375 ms to -40 dB (ours now 388) —
///             and band-pass Q 0.82 -> 1.0 (centroid residual +17% -> +11%). Maracas:
///             added the ~15 kHz low-pass (centroid 16.2 -> 11.2 kHz vs 10.9 measured);
///             t40 +13% residual.
///
///             Plain C++17, stdlib only, per-sample, allocation-free after prepare().
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <cmath>

#include "bridged_t.h" // first_order + k_pi
#include "swing_vca.h"

namespace taptools {
    namespace tr808 {

        // Handclap band-pass, from the IC21 multiple-feedback filter (see header). IC21 is a
        // dual op-amp and the schematic shows a second section (C136/R372 area); modeled as
        // two cascaded identical sections, which matches the measured rolloff above the band.
        constexpr double k_cp_bp_hz = 2060.0;
        constexpr double k_cp_bp_q  = 1.0;

        // Figure 13 timing: three sawtooth teeth across the 30 ms window.
        constexpr int    k_cp_teeth      = 3;
        constexpr double k_cp_tooth_ms   = 10.0;
        constexpr double k_cp_tooth_tau  = 4e-3;   // per-tooth decay (fit to Fig. 13 trace 7)
        constexpr double k_cp_tail_tau   = 100e-3; // reverberation decay (see calibration note)
        constexpr double k_cp_tail_level = 0.4;    // tail level relative to the teeth

        // Maracas (behavioral, see header): bright burst.
        constexpr double k_ma_hp_hz = 5500.0;
        constexpr double k_ma_lp_hz = 15000.0;
        constexpr double k_ma_att_s = 0.4e-3;
        constexpr double k_ma_tau_s = 7e-3;

        // Output normalization: full-accent hits peak just under +-1.
        constexpr double k_cp_out_scale = 2.4;
        constexpr double k_ma_out_scale = 0.85;

        /// The TR-808 handclap/maracas channel. `model` 0 = clap, 1 = maracas.
        class clap {
          public:
            enum model_type { model_clap = 0, model_maracas = 1 };

            void prepare(double sample_rate) {
                m_sr = sample_rate;

                // Band-pass as an RBJ-style biquad from (fc, Q) — constant-peak-gain form.
                const double w  = 2.0 * k_pi * k_cp_bp_hz / sample_rate;
                const double al = std::sin(w) / (2.0 * k_cp_bp_q);
                const double a0 = 1.0 + al;
                m_bp_b0         = al / a0;
                m_bp_a1         = -2.0 * std::cos(w) / a0;
                m_bp_a2         = (1.0 - al) / a0;

                m_ma_hp1.prepare(sample_rate);
                m_ma_hp2.prepare(sample_rate);
                m_ma_lp.prepare(sample_rate);
                const double tau = 1.0 / (2.0 * k_pi * k_ma_hp_hz);
                m_ma_hp1.set(tau, 0.0, tau, 1.0);
                m_ma_hp2.set(tau, 0.0, tau, 1.0);
                const double tau_lp = 1.0 / (2.0 * k_pi * k_ma_lp_hz);
                m_ma_lp.set(0.0, 1.0, tau_lp, 1.0);

                m_ma_env.prepare(sample_rate);
                m_ma_env.set_times(k_ma_att_s, k_ma_tau_s);

                reset();
            }

            void reset() {
                m_bp_z1 = m_bp_z2 = m_bp2_z1 = m_bp2_z2 = 0.0;
                m_ma_hp1.reset();
                m_ma_hp2.reset();
                m_ma_lp.reset();
                m_ma_env.reset();
                m_noise.reset();
                m_tooth_env = m_tail_env = 0.0;
                m_teeth_left             = 0;
                m_tooth_timer            = 0;
            }

            // -- panel / config ------------------------------------------------------------

            /// 0 = handclap, 1 = maracas (the hardware CP/MA switch).
            void set_model(int model) { m_model = model == model_maracas ? model_maracas : model_clap; }
            int  model() const { return m_model; }

            /// Output level, 0..1 (VR17, CP/MA LEVEL).
            void set_level(double amount) { m_level = std::clamp(amount, 0.0, 1.0); }

            /// Circuit bend: reverberation-tail level scale, 0..2 (1 = stock; 0 removes the
            /// wash, leaving only the three teeth).
            void set_tail(double amount) { m_tail = std::clamp(amount, 0.0, 2.0); }

            /// Noise-source seed (deterministic; mc. instances decorrelate by seed).
            void set_seed(uint64_t seed) { m_noise.set_seed(seed); }

            // -- performance ---------------------------------------------------------------

            /// Fire the voice. `accent` 0..1 scales the envelope drive (the trigger bus).
            void trigger(double accent = 1.0) {
                m_accent = 0.3 + 0.7 * std::clamp(accent, 0.0, 1.0);
                if (m_model == model_clap) {
                    m_teeth_left  = k_cp_teeth;
                    m_tooth_timer = 0; // first tooth fires immediately
                    m_tail_env    = m_accent;
                }
                else {
                    m_ma_env.trigger(m_accent);
                }
            }

            double process() {
                const double n = m_noise.process();

                if (m_model == model_maracas) {
                    const double y =
                        swing_vca(m_ma_lp.process(m_ma_hp2.process(m_ma_hp1.process(n))), m_ma_env.process());
                    return y * m_level * k_ma_out_scale;
                }

                // Handclap: sawtooth-tooth scheduler (Fig. 13).
                if (m_teeth_left > 0) {
                    if (m_tooth_timer <= 0) {
                        m_tooth_env = m_accent; // abrupt charge of C144
                        --m_teeth_left;
                        m_tooth_timer = static_cast<int>(k_cp_tooth_ms * 0.001 * m_sr);
                    }
                    --m_tooth_timer;
                }
                m_tooth_env *= std::exp(-1.0 / (k_cp_tooth_tau * m_sr));
                m_tail_env *= std::exp(-1.0 / (k_cp_tail_tau * m_sr));
                if (m_tooth_env < 1e-12)
                    m_tooth_env = 0.0;
                if (m_tail_env < 1e-12)
                    m_tail_env = 0.0;

                // Band-pass the noise (two cascaded sections, transposed direct form II;
                // b1 = 0, b2 = -b0).
                const double b1 = m_bp_b0 * n + m_bp_z1;
                m_bp_z1         = m_bp_z2 - m_bp_a1 * b1;
                m_bp_z2         = -m_bp_b0 * n - m_bp_a2 * b1;
                const double bp = m_bp_b0 * b1 + m_bp2_z1;
                m_bp2_z1        = m_bp2_z2 - m_bp_a1 * bp;
                m_bp2_z2        = -m_bp_b0 * b1 - m_bp_a2 * bp;

                const double env = m_tooth_env + k_cp_tail_level * m_tail * m_tail_env;
                return swing_vca(bp, env) * m_level * k_cp_out_scale;
            }

          private:
            double m_sr{48000.0};
            int    m_model{model_clap};

            // clap band-pass biquad (shared coefficients, two sections of state)
            double m_bp_b0{0.0}, m_bp_a1{0.0}, m_bp_a2{0.0};
            double m_bp_z1{0.0}, m_bp_z2{0.0}, m_bp2_z1{0.0}, m_bp2_z2{0.0};

            // clap envelopes
            double m_tooth_env{0.0}, m_tail_env{0.0};
            int    m_teeth_left{0}, m_tooth_timer{0};

            // maracas path
            first_order m_ma_hp1, m_ma_hp2, m_ma_lp;
            decay_env   m_ma_env;

            white_noise m_noise;

            double m_level{1.0}, m_tail{1.0}, m_accent{1.0};
        };

    } // namespace tr808
} // namespace taptools
