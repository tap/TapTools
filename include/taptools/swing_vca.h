/// @file
/// @brief      Shared TR-808 drum-voice blocks: swing-type VCA, RC decay envelopes, white noise.
/// @details    The 808's voices shape their percussive decays with one- and two-transistor
///             "swing type" VCAs driven by simple RC discharge envelopes (TR-808 Service Notes,
///             Fig. 12 "Representative Swing Type VCA") — not ADSRs. This header collects the
///             small portable blocks the noise-path voices share:
///
///             - `decay_env` — trigger to a level, one-pole rise (fast, configurable), then
///               exponential decay with a settable time constant: the RC discharge shape.
///             - `swing_vca` — the envelope applied as a gain. Linear by default (`drive` 0),
///               with an opt-in `drive` that engages the swing VCA's symmetric harmonic
///               saturation — the "many high harmonics" the Service Notes note (RS/CL VCA) —
///               via the shared `vca.h` `swing_shape` (also `tap.vca~`'s `swing` circuit). Off
///               by default so every calibrated voice stays bit-identical until a voice opts in.
///             - `white_noise` — the shared white-noise source (the 808 has a single noise
///               generator feeding SD snappy, CP, MA, and the tom "reverberation"). Seeded
///               xorshift64*: deterministic per seed, so renders and tests reproduce and mc.
///               instances decorrelate by seed, per the house `vco.h` convention.
///
///             Plain C++17, stdlib only, allocation-free, no Max/Min dependency.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "vca.h" // tap::tools::vca::swing_shape — the shared swing-type saturator

namespace tap::tools {
    namespace tr808 {

        /// Deterministic white noise in [-1, 1] — xorshift64* keyed by seed.
        class white_noise {
          public:
            void set_seed(uint64_t seed) {
                m_state = seed ? seed : 0x9e3779b97f4a7c15ULL;
                m_seed  = seed;
            }

            uint64_t seed() const { return m_seed; }

            void reset() { set_seed(m_seed); }

            double process() {
                m_state ^= m_state >> 12;
                m_state ^= m_state << 25;
                m_state ^= m_state >> 27;
                const uint64_t r = m_state * 0x2545f4914f6cdd1dULL;
                // top 53 bits -> [0,1) -> [-1,1)
                return static_cast<double>(r >> 11) * (2.0 / 9007199254740992.0) - 1.0;
            }

          private:
            uint64_t m_seed{1};
            uint64_t m_state{1};
        };

        /// Trigger-to-level envelope: one-pole rise at `attack_s`, exponential decay at
        /// `decay_s` (the RC discharge). Retriggering re-aims the rise without a reset click.
        class decay_env {
          public:
            void prepare(double sample_rate) {
                m_sr = sample_rate;
                update();
            }

            void set_times(double attack_s, double decay_s) {
                m_attack_s = std::max(attack_s, 1e-6);
                m_decay_s  = std::max(decay_s, 1e-6);
                update();
            }

            void reset() {
                m_env    = 0.0;
                m_target = 0.0;
            }

            /// Fire: aim the envelope at `level` (it then decays back to zero).
            void trigger(double level) { m_target = level; }

            double process() {
                if (m_target > 0.0) {
                    m_env += m_attack_a * (m_target - m_env);
                    if (m_env >= m_target * 0.995) {
                        m_env    = m_target;
                        m_target = 0.0; // rise complete; fall from here
                    }
                }
                else {
                    m_env *= m_decay_a;
                    if (m_env < 1e-12)
                        m_env = 0.0;
                }
                return m_env;
            }

            double value() const { return m_env; }

          private:
            void update() {
                m_attack_a = 1.0 - std::exp(-1.0 / (m_attack_s * m_sr));
                m_decay_a  = std::exp(-1.0 / (m_decay_s * m_sr));
            }

            double m_sr{48000.0};
            double m_attack_s{50e-6}, m_decay_s{10e-3};
            double m_attack_a{1.0}, m_decay_a{0.0};
            double m_env{0.0}, m_target{0.0};
        };

        /// The swing-type VCA: envelope as gain. Linear by default (`drive` 0 → `x * env`, the
        /// calibrated model, bit-for-bit); `drive > 0` engages the swing VCA's symmetric harmonic
        /// saturation (the "many high harmonics" the Service Notes note) on the enveloped signal,
        /// via the shared tap::tools::vca::swing_shape. The character rides the envelope — quiet tails
        /// stay clean, hot transients pick up grit and gentle compression.
        inline double swing_vca(double x, double env, double drive = 0.0) {
            return vca::swing_shape(x * env, drive);
        }

    } // namespace tr808
} // namespace tap::tools
