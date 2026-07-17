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
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

#include "bridged_t.h" // k_pi
#include "swing_vca.h" // white_noise (seed machinery)

namespace taptools {
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
        class bandpass {
          public:
            void prepare(double sample_rate, double fc_hz, double q) {
                const double w  = 2.0 * k_pi * fc_hz / sample_rate;
                const double al = std::sin(w) / (2.0 * q);
                const double a0 = 1.0 + al;
                m_b0            = al / a0;
                m_a1            = -2.0 * std::cos(w) / a0;
                m_a2            = (1.0 - al) / a0;
                reset();
            }

            void reset() { m_z1 = m_z2 = 0.0; }

            double process(double x) {
                const double y = m_b0 * x + m_z1;
                m_z1           = m_z2 - m_a1 * y;
                m_z2           = -m_b0 * x - m_a2 * y;
                return y;
            }

          private:
            double m_b0{0.0}, m_a1{0.0}, m_a2{0.0};
            double m_z1{0.0}, m_z2{0.0};
        };

        /// The six-oscillator bank. process() returns the passively-mixed sum (each square
        /// +-1/6); osc(i) exposes the individual outputs (the cowbell taps #5/#6, indices 4/5).
        class metal_bank {
          public:
            void prepare(double sample_rate) {
                m_sr = sample_rate;
                update_increments();
            }

            void reset() {
                for (auto& p : m_phase)
                    p = 0.0;
            }

            /// Pitch ratio bend (scales every oscillator; 1 = stock).
            void set_tuning(double ratio) {
                m_tuning = std::clamp(ratio, 0.25, 4.0);
                update_increments();
            }

            /// Per-unit component spread, 0..1 (0 = exact nominal frequencies).
            void set_tolerance(double amount) {
                m_tolerance = std::clamp(amount, 0.0, 1.0);
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
                    if (m_phase[i] >= 1.0)
                        m_phase[i] -= 1.0;
                    m_out[i] = m_phase[i] < k_bank_duty ? 1.0 : -1.0;
                }
            }

            double osc(int i) const { return m_out[i]; }

            double sum() const {
                double s = 0.0;
                for (double v : m_out)
                    s += v;
                return s / static_cast<double>(k_bank_oscs);
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
                    const double u =
                        static_cast<double>((s * 0x2545f4914f6cdd1dULL) >> 11) / 9007199254740992.0; // [0,1)
                    const double dev = 1.0 + (2.0 * u - 1.0) * k_bank_tolerance * m_tolerance;
                    m_inc[i]         = k_bank_hz[i] * m_tuning * dev / m_sr;
                }
            }

            double                          m_sr{48000.0};
            double                          m_tuning{1.0}, m_tolerance{0.0};
            uint64_t                        m_seed{1};
            std::array<double, k_bank_oscs> m_phase{}, m_inc{}, m_out{};
        };

        /// The trigger attack smoother (Q19): a one-pole lag on the accent pulse, less a
        /// base-emitter drop. Returns 0 for inputs below the drop.
        class attack_smoother {
          public:
            void prepare(double sample_rate) {
                m_a = 1.0 - std::exp(-1.0 / (k_bank_smoother_tau_s * sample_rate));
                reset();
            }

            void reset() { m_z = 0.0; }

            double process(double v_trig) {
                const double in = std::max(v_trig - k_bank_smoother_vbe, 0.0);
                m_z += m_a * (in - m_z);
                return m_z;
            }

          private:
            double m_a{1.0}, m_z{0.0};
        };

    } // namespace tr808
} // namespace taptools
