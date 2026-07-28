/// @file
/// @brief      The TR-808 "metal bank": six square oscillators + the shared filter voicings.
/// @details    The 808's metallic voices (cymbal, open/closed hi-hat, cowbell) all draw on one
///             bank of six Schmitt-trigger relaxation oscillators (HD14584 on the voicing
///             board; TR-808 Service Notes p.6 and the voicing-board schematic). Frequencies
///             per Werner, Abel & Smith, "The TR-808 Cymbal: a Physically-Informed,
///             Circuit-Bendable, Digital Model" (ICMC|SMC 2014): nominal 205.3, 369.6, 304.4,
///             and 522.7 Hz, plus two trimpot-tuned oscillators factory-set to 800 and 540 Hz
///             (TM2/TM1 — the pair the cowbell taps; Roland's own schematic margin notes them
///             as 1.25 ms / 1.85 ms). Duty cycle 47.98%, per the paper's HD14584 analysis.
///
///             Component tolerance is part of the instrument: the bank's RC parts can put any
///             given unit's oscillators off nominal by up to ~20% — why no two 808s' cymbals
///             sound alike. `tolerance` scales a deterministic per-seed frequency spread
///             (0 = exact nominal, 1 = full production spread), the `vco.h` `imperfect`
///             convention.
///
///             Also here, shared by the metal voices:
///             - the two band-pass voicings the cymbal/hat paths filter the bank through
///               (~3440 Hz and ~7100 Hz centers, from the paper's Fig. 4 analysis of the IC3
///               filters; implemented as constant-peak biquads fit to the published response),
///             - the trigger "attack smoother" (Q19 one-pole: tau = 102.44 us, less a 0.7258 V
///               base-emitter drop — the paper's least-squares fit).
///
///             Naive (non-bandlimited) squares are faithful here: fixed sub-1.2 kHz
///             fundamentals whose upper hash is then band-passed; the residual aliasing folds
///             into the same inharmonic wash the circuit itself produces.
///
///             Plain C++17, stdlib only, allocation-free, no Max/Min dependency.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

#include "bridged_t.h" // k_pi
#include "numeric.h"
#include "swing_vca.h" // white_noise (seed machinery)

namespace tap::tools {
    namespace tr808 {

        constexpr int    k_bank_oscs            = 6;
        constexpr double k_bank_hz[k_bank_oscs] = {205.3, 369.6, 304.4, 522.7, 800.0, 540.0};
        constexpr double k_bank_duty            = 0.4798;
        constexpr double k_bank_tolerance       = 0.20; // +-20% at tolerance = 1
        constexpr double k_bank_bp1_hz          = 3440.0;
        constexpr double k_bank_bp2_hz          = 7100.0;
        constexpr double k_bank_bp_q            = 1.5; // fit to the paper's Fig. 4 skirts
        constexpr double k_bank_smoother_tau_s  = 1.0244e-4;
        constexpr double k_bank_smoother_vbe    = 0.7258;

        /// One constant-peak band-pass biquad (RBJ), transposed direct form II.
        template <typename Sample>
        class basic_bandpass {
            static_assert(is_sample_profile<Sample>,
                          "basic_bandpass supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            void prepare(Sample sample_rate, Sample fc_hz, Sample q) {
                const Sample w  = Sample(2.0) * k_pi_for<Sample> * fc_hz / sample_rate;
                const Sample al = std::sin(w) / (Sample(2.0) * q);
                const Sample a0 = Sample(1.0) + al;
                m_b0            = al / a0;
                m_a1            = -Sample(2.0) * std::cos(w) / a0;
                m_a2            = (Sample(1.0) - al) / a0;
                reset();
            }

            void reset() { m_z1 = m_z2 = Sample(0.0); }

            Sample process(Sample x) {
                const Sample y = m_b0 * x + m_z1;
                m_z1           = m_z2 - m_a1 * y;
                m_z2           = -m_b0 * x - m_a2 * y;
                return y;
            }

          private:
            Sample m_b0{Sample(0.0)}, m_a1{Sample(0.0)}, m_a2{Sample(0.0)};
            Sample m_z1{Sample(0.0)}, m_z2{Sample(0.0)};
        };

        /// The six-oscillator bank. process() returns the passively-mixed sum (each square
        /// +-1/6); osc(i) exposes the individual outputs (the cowbell taps #5/#6, indices 4/5).
        template <typename Sample>
        class basic_metal_bank {
            static_assert(is_sample_profile<Sample>,
                          "basic_metal_bank supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            void prepare(Sample sample_rate) {
                m_sr = sample_rate;
                update_increments();
            }

            void reset() {
                for (auto& p : m_phase)
                    p = Sample(0.0);
            }

            /// Pitch ratio bend (scales every oscillator; 1 = stock).
            void set_tuning(Sample ratio) {
                m_tuning = std::clamp(ratio, Sample(0.25), Sample(4.0));
                update_increments();
            }

            /// Per-unit component spread, 0..1 (0 = exact nominal frequencies).
            void set_tolerance(Sample amount) {
                m_tolerance = std::clamp(amount, Sample(0.0), Sample(1.0));
                update_increments();
            }

            /// Deterministic unit identity: each seed is a different 808 off the line.
            void set_seed(uint64_t seed) {
                m_seed = seed ? seed : 1;
                update_increments();
            }

            void process() {
                for (int i = 0; i < k_bank_oscs; ++i) {
                    m_phase[i] += m_inc[i];
                    if (m_phase[i] >= Sample(1.0))
                        m_phase[i] -= Sample(1.0);
                    m_out[i] = m_phase[i] < Sample(k_bank_duty) ? Sample(1.0) : -Sample(1.0);
                }
            }

            Sample osc(int i) const { return m_out[i]; }

            Sample sum() const {
                Sample s = Sample(0.0);
                for (Sample v : m_out)
                    s += v;
                return s / static_cast<Sample>(k_bank_oscs);
            }

          private:
            void update_increments() {
                // Per-seed frequency deviations: a fixed draw per oscillator, +-k_bank_tolerance
                // at tolerance 1. xorshift64* keyed by (seed, oscillator index).
                for (int i = 0; i < k_bank_oscs; ++i) {
                    uint64_t s = m_seed * 0x9e3779b97f4a7c15ULL + static_cast<uint64_t>(i + 1) * 0xbf58476d1ce4e5b9ULL;
                    s ^= s >> 12;
                    s ^= s << 25;
                    s ^= s >> 27;
                    const Sample u =
                        static_cast<Sample>((s * 0x2545f4914f6cdd1dULL) >> 11) / Sample(9007199254740992.0); // [0,1)
                    const Sample dev =
                        Sample(1.0) + (Sample(2.0) * u - Sample(1.0)) * Sample(k_bank_tolerance) * m_tolerance;
                    m_inc[i] = Sample(k_bank_hz[i]) * m_tuning * dev / m_sr;
                }
            }

            Sample                          m_sr{Sample(48000.0)};
            Sample                          m_tuning{Sample(1.0)}, m_tolerance{Sample(0.0)};
            uint64_t                        m_seed{1};
            std::array<Sample, k_bank_oscs> m_phase{}, m_inc{}, m_out{};
        };

        /// The trigger attack smoother (Q19): a one-pole lag on the accent pulse, less a
        /// base-emitter drop. Returns 0 for inputs below the drop.
        template <typename Sample>
        class basic_attack_smoother {
            static_assert(is_sample_profile<Sample>,
                          "basic_attack_smoother supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            void prepare(Sample sample_rate) {
                m_a = Sample(1.0) - std::exp(-Sample(1.0) / (Sample(k_bank_smoother_tau_s) * sample_rate));
                reset();
            }

            void reset() { m_z = Sample(0.0); }

            Sample process(Sample v_trig) {
                const Sample in = std::max(v_trig - Sample(k_bank_smoother_vbe), Sample(0.0));
                m_z += m_a * (in - m_z);
                return m_z;
            }

          private:
            Sample m_a{Sample(1.0)}, m_z{Sample(0.0)};
        };

        /// The double profile — the golden model.
        using bandpass        = basic_bandpass<double>;
        using metal_bank      = basic_metal_bank<double>;
        using attack_smoother = basic_attack_smoother<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using bandpass32        = basic_bandpass<float>;
        using metal_bank32      = basic_metal_bank<float>;
        using attack_smoother32 = basic_attack_smoother<float>;

    } // namespace tr808
} // namespace tap::tools
