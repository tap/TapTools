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
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <type_traits>

#include "numeric.h"
#include "vca.h" // tap::tools::vca::swing_shape — the shared swing-type saturator

namespace tap::tools {
    namespace tr808 {

        /// Deterministic white noise in [-1, 1] — xorshift64* keyed by seed.
        template <typename Sample>
        class basic_white_noise {
            static_assert(is_sample_profile<Sample>,
                          "basic_white_noise supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            void set_seed(uint64_t seed) {
                m_state = seed ? seed : 0x9e3779b97f4a7c15ULL;
                m_seed  = seed;
            }

            uint64_t seed() const { return m_seed; }

            void reset() { set_seed(m_seed); }

            Sample process() {
                m_state ^= m_state >> 12;
                m_state ^= m_state << 25;
                m_state ^= m_state >> 27;
                const uint64_t r = m_state * 0x2545f4914f6cdd1dULL;
                // top 53 bits -> [0,1) -> [-1,1). Note the float profile rounds the 53-bit
                // integer into a 24-bit mantissa, so the two profiles produce DIFFERENT noise
                // sequences (both uniform white — this is the one place where cross-precision
                // agreement is not expected, and the tests compare statistics, not samples).
                return static_cast<Sample>(r >> 11) * (Sample(2.0) / Sample(9007199254740992.0)) - Sample(1.0);
            }

          private:
            uint64_t m_seed{1};
            uint64_t m_state{1};
        };

        /// Trigger-to-level envelope: one-pole rise at `attack_s`, exponential decay at
        /// `decay_s` (the RC discharge). Retriggering re-aims the rise without a reset click.
        template <typename Sample>
        class basic_decay_env {
            static_assert(is_sample_profile<Sample>,
                          "basic_decay_env supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            void prepare(Sample sample_rate) {
                m_sr = sample_rate;
                update();
            }

            void set_times(Sample attack_s, Sample decay_s) {
                m_attack_s = std::max(attack_s, Sample(1e-6));
                m_decay_s  = std::max(decay_s, Sample(1e-6));
                update();
            }

            void reset() {
                m_env    = Sample(0.0);
                m_target = Sample(0.0);
            }

            /// Fire: aim the envelope at `level` (it then decays back to zero).
            void trigger(Sample level) { m_target = level; }

            Sample process() {
                if (m_target > Sample(0.0)) {
                    m_env += m_attack_a * (m_target - m_env);
                    if (m_env >= m_target * Sample(0.995)) {
                        m_env    = m_target;
                        m_target = Sample(0.0); // rise complete; fall from here
                    }
                }
                else {
                    m_env *= m_decay_a;
                    if (m_env < Sample(1e-12))
                        m_env = Sample(0.0);
                }
                return m_env;
            }

            Sample value() const { return m_env; }

          private:
            void update() {
                m_attack_a = Sample(1.0) - std::exp(-Sample(1.0) / (m_attack_s * m_sr));
                m_decay_a  = std::exp(-Sample(1.0) / (m_decay_s * m_sr));
            }

            Sample m_sr{Sample(48000.0)};
            Sample m_attack_s{Sample(50e-6)}, m_decay_s{Sample(10e-3)};
            Sample m_attack_a{Sample(1.0)}, m_decay_a{Sample(0.0)};
            Sample m_env{Sample(0.0)}, m_target{Sample(0.0)};
        };

        /// The swing-type VCA: envelope as gain. Linear by default (`drive` 0 → `x * env`, the
        /// calibrated model, bit-for-bit); `drive > 0` engages the swing VCA's symmetric harmonic
        /// saturation (the "many high harmonics" the Service Notes note) on the enveloped signal,
        /// via the shared tap::tools::vca::swing_shape. The character rides the envelope — quiet tails
        /// stay clean, hot transients pick up grit and gentle compression.
        template <typename Sample>
        Sample swing_vca(Sample x, std::type_identity_t<Sample> env, std::type_identity_t<Sample> drive = Sample(0.0)) {
            return basic_vca<Sample>::swing_shape(x * env, drive);
        }

        /// The double profile — the golden model.
        using white_noise = basic_white_noise<double>;
        using decay_env   = basic_decay_env<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using white_noise32 = basic_white_noise<float>;
        using decay_env32   = basic_decay_env<float>;

    } // namespace tr808
} // namespace tap::tools
