/// @file
/// @brief      Channel-vocoder kernel (the DSP core of tap.vocoder~).
/// @details    A classic 24-band channel vocoder: a bank of log-spaced bandpass filters analyses the
///             modulator, a per-band envelope follower tracks each band's amplitude, an identical
///             bank splits the carrier, and each carrier band is multiplied by the matching
///             modulator envelope before the bands are summed — imposing the modulator's spectral
///             envelope onto the carrier. The bandpass sections are RBJ Audio-EQ-Cookbook constant
///             0 dB-peak bandpass biquads (unconditionally stable across the band range).
///             Reconstructed from tap.vocoder~'s reference documentation. Time-domain, no FFT.
///             Follows the svf.h / ladder.h idiom: prepare(samplerate), then per-sample process().
///             Plain C++17, Min-free.
///
///             Two conveniences beyond the original object:
///
///             - **Sibilance path** (`set_sibilance`, default 0): a deterministic seeded white-noise
///               source blended into the *carrier* path of the bands whose centre frequency lies
///               above ~4 kHz. Because the modulator envelopes still gate every band, the noise is
///               heard only while the modulator carries high-band energy — the channel vocoder's
///               classic unvoiced/sibilance excitation, in the lineage of Dudley's original design,
///               which substitutes an aperiodic (noise) source for the periodic one during unvoiced
///               speech (H. Dudley, "Remaking Speech", JASA 11(2), 1939; "The Vocoder", Bell Labs
///               Record 18, 1939; see also U. Zölzer (ed.), *DAFX: Digital Audio Effects*, 2nd ed.,
///               Wiley 2011, ch. 8 — vocoder-based effects). **Contract note:** with sibilance > 0
///               the pinned "a silent carrier yields silence" contract is deliberately relaxed for
///               those top bands (the noise *is* carrier there); at the default 0 the original
///               contract holds and the output is bit-identical to the pre-sibilance kernel.
///             - **Mix** (`set_mix`, default 100 = full wet): an equal-power blend of the dry
///               *carrier* against the vocoded output. The carrier is the dry side because it is
///               the program material you hear — blending it back under the vocoded signal is the
///               classic parallel move (synth pad under robot voice); the modulator is an analysis
///               control input, and bleeding raw speech through would defeat the effect.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2001-2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace tap::tools {
    namespace vocoder {

        class bank {
          public:
            static constexpr int    k_bands = 24; // "basic 24-band vocoder" (per the reference page)
            static constexpr double k_pi    = 3.14159265358979323846;
            static constexpr double k_fmin  = 50.0;    // lowest band centre (Hz)
            static constexpr double k_fmax  = 12000.0; // highest band centre (Hz)

            // Bands whose nominal centre frequency (band_frequency, sample-rate independent) lies
            // above this receive the sibilance noise in their carrier path. With the 24-band
            // 50 Hz..12 kHz log spacing that is bands 19..23 (centres ~4.63 / 5.87 / 7.45 / 9.45 /
            // 12 kHz; band 18 sits at ~3.65 kHz and stays clean).
            static constexpr double k_sibilance_hz = 4000.0;

            bank() {
                for (int i = 0; i < k_bands; ++i) {
                    m_sib_band[i] = band_frequency(i) > k_sibilance_hz;
                }
            }

            // Set the sample rate and (re)compute all coefficients. Call before processing and whenever
            // the sample rate changes (the Min wrapper calls this from dspsetup).
            void prepare(double samplerate) {
                m_sr = samplerate;
                recalc_filters();
                recalc_envelope();
            }

            // Q factor (resonance) shared by every bandpass filter. Higher is narrower / more robotic.
            void set_q(double q) {
                m_q = q;
                recalc_filters();
            }

            // Envelope-follower analysis period, in milliseconds. Shorter tracks more sharply.
            void set_response_ms(double ms) {
                m_response_ms = ms;
                recalc_envelope();
            }

            // Linear makeup gain applied to the summed output.
            void set_gain(double g) { m_gain = g; }

            // Sibilance amount, 0..1 (default 0): how much seeded white noise is blended into the
            // carrier path of the bands above k_sibilance_hz. At 0 the output is bit-identical to
            // the noise-free kernel and the silent-carrier contract holds; above 0 that contract is
            // deliberately relaxed for the top bands (see the file docstring).
            void set_sibilance(double amount) { m_sibilance = std::clamp(amount, 0.0, 1.0); }

            // Reseed the sibilance noise source (deterministic; same seed renders bit-identically,
            // different seeds decorrelate instances). Seed 0 is mapped to 1, as in vco.h.
            void set_seed(uint32_t seed) {
                m_seed = (seed == 0) ? 1u : seed;
                m_rng  = m_seed;
            }

            // Wet/dry mix in percent, 0 (dry carrier) .. 100 (fully vocoded, the default), as an
            // equal-power crossfade. The endpoints are snapped exactly, so 100 is bit-identical to
            // the pre-mix kernel and 0 passes the carrier through untouched (makeup gain applies to
            // the vocoded side only).
            void set_mix(double pct) {
                const double wet = std::clamp(pct, 0.0, 100.0) / 100.0;
                if (wet <= 0.0) {
                    m_mix_dry = 1.0;
                    m_mix_wet = 0.0;
                }
                else if (wet >= 1.0) {
                    m_mix_dry = 0.0;
                    m_mix_wet = 1.0;
                }
                else {
                    const double theta = wet * (k_pi / 2.0);
                    m_mix_dry          = std::cos(theta);
                    m_mix_wet          = std::sin(theta);
                }
            }

            // Reset all filter, envelope-follower, and noise state (the noise source rewinds to its
            // seed, so a cleared bank replays its exact sibilance sequence).
            void clear() {
                for (auto& b : m_mod) {
                    b.clear();
                }
                for (auto& b : m_car) {
                    b.clear();
                }
                m_env.fill(0.0);
                m_rng = m_seed;
            }

            // Process one sample: shape the carrier by the modulator's per-band envelope, feeding
            // the top bands' carrier path with the sibilance noise, then mix against the dry carrier.
            double process(double modulator, double carrier) {
                // The noise generator advances every sample regardless of amount (like vco.h's
                // vibrato phase), so changing the amount never shifts the sequence.
                const double noise  = uniform();
                const bool   sib_on = m_sibilance > 0.0;
                const double sib    = m_sibilance * noise;

                double out = 0.0;
                for (int i = 0; i < k_bands; ++i) {
                    const double m    = m_mod[i].process(modulator);
                    const double rect = std::fabs(m);
                    m_env[i]          = m_env_coef * m_env[i] + (1.0 - m_env_coef) * rect;

                    const double cin = (sib_on && m_sib_band[i]) ? carrier + sib : carrier;
                    const double c   = m_car[i].process(cin);
                    out += c * m_env[i];
                }
                out *= m_gain;
                return (m_mix_dry > 0.0) ? m_mix_dry * carrier + m_mix_wet * out : m_mix_wet * out;
            }

          private:
            // One RBJ constant-0 dB-peak bandpass biquad section, Direct Form I.
            struct biquad {
                double b0{0.0}, b1{0.0}, b2{0.0}, a1{0.0}, a2{0.0};
                double x1{0.0}, x2{0.0}, y1{0.0}, y2{0.0};

                double process(double x) {
                    const double y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
                    x2             = x1;
                    x1             = x;
                    y2             = y1;
                    y1             = y;
                    return y;
                }
                void clear() { x1 = x2 = y1 = y2 = 0.0; }
            };

            // Log-spaced band centre frequency for band i.
            static double band_frequency(int i) {
                const double t = (k_bands > 1) ? static_cast<double>(i) / (k_bands - 1) : 0.0;
                return k_fmin * std::pow(k_fmax / k_fmin, t);
            }

            void recalc_filters() {
                if (m_sr <= 0.0) {
                    return;
                }
                const double q = (m_q > 0.001) ? m_q : 0.001;

                for (int i = 0; i < k_bands; ++i) {
                    double fc = band_frequency(i);
                    fc        = std::min(fc, 0.45 * m_sr); // keep every band well below Nyquist

                    const double w0    = 2.0 * k_pi * fc / m_sr;
                    const double cosw0 = std::cos(w0);
                    const double alpha = std::sin(w0) / (2.0 * q);
                    const double a0    = 1.0 + alpha;

                    // Constant 0 dB peak-gain bandpass (RBJ cookbook), normalised by a0.
                    const double b0 = alpha / a0;
                    const double b2 = -alpha / a0;
                    const double a1 = (-2.0 * cosw0) / a0;
                    const double a2 = (1.0 - alpha) / a0;

                    for (auto* b : {&m_mod, &m_car}) {
                        auto& bq = (*b)[i];
                        bq.b0    = b0;
                        bq.b1    = 0.0;
                        bq.b2    = b2;
                        bq.a1    = a1;
                        bq.a2    = a2;
                    }
                }
            }

            void recalc_envelope() {
                if (m_sr <= 0.0) {
                    return;
                }
                const double tau = (m_response_ms > 0.0001 ? m_response_ms : 0.0001) * 0.001; // ms → s
                m_env_coef       = std::exp(-1.0 / (tau * m_sr));
            }

            double uniform() { // deterministic white noise in [-1, 1] (LCG, as in vco.h)
                m_rng = m_rng * 1664525u + 1013904223u;
                return (m_rng / 2147483648.0) - 1.0;
            }

            double   m_sr{0.0};
            double   m_q{20.0};
            double   m_response_ms{20.0};
            double   m_env_coef{0.0};
            double   m_gain{1.0};
            double   m_sibilance{0.0};
            double   m_mix_dry{0.0};
            double   m_mix_wet{1.0};
            uint32_t m_seed{1u};
            uint32_t m_rng{1u};

            std::array<biquad, k_bands> m_mod{};
            std::array<biquad, k_bands> m_car{};
            std::array<double, k_bands> m_env{};
            std::array<bool, k_bands>   m_sib_band{};
        };

    } // namespace vocoder
} // namespace tap::tools
