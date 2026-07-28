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
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2001-2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include "numeric.h"

namespace tap::tools {
    namespace vocoder {

        template <typename Sample>

        class basic_bank {
            static_assert(is_sample_profile<Sample>,

                          "basic_bank supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            static constexpr int    k_bands = 24;      // "basic 24-band vocoder" (per the reference page)
            static constexpr double k_fmin  = 50.0;    // lowest band centre (Hz)
            static constexpr double k_fmax  = 12000.0; // highest band centre (Hz)

            // Set the sample rate and (re)compute all coefficients. Call before processing and whenever
            // the sample rate changes (the Min wrapper calls this from dspsetup).
            void prepare(Sample samplerate) {
                m_sr = samplerate;
                recalc_filters();
                recalc_envelope();
            }

            // Q factor (resonance) shared by every bandpass filter. Higher is narrower / more robotic.
            void set_q(Sample q) {
                m_q = q;
                recalc_filters();
            }

            // Envelope-follower analysis period, in milliseconds. Shorter tracks more sharply.
            void set_response_ms(Sample ms) {
                m_response_ms = ms;
                recalc_envelope();
            }

            // Linear makeup gain applied to the summed output.
            void set_gain(Sample g) { m_gain = g; }

            // Reset all filter and envelope-follower state.
            void clear() {
                for (auto& b : m_mod) {
                    b.clear();
                }
                for (auto& b : m_car) {
                    b.clear();
                }
                m_env.fill(Sample(0.0));
            }

            // Process one sample: shape the carrier by the modulator's per-band envelope.
            Sample process(Sample modulator, Sample carrier) {
                Sample out = Sample(0.0);
                for (int i = 0; i < k_bands; ++i) {
                    const Sample m    = m_mod[i].process(modulator);
                    const Sample rect = std::fabs(m);
                    m_env[i]          = m_env_coef * m_env[i] + (Sample(1.0) - m_env_coef) * rect;

                    const Sample c = m_car[i].process(carrier);
                    out += c * m_env[i];
                }
                return out * m_gain;
            }

          private:
            // One RBJ constant-0 dB-peak bandpass biquad section, Direct Form I.
            struct biquad {
                Sample b0{Sample(0.0)}, b1{Sample(0.0)}, b2{Sample(0.0)}, a1{Sample(0.0)}, a2{Sample(0.0)};
                Sample x1{Sample(0.0)}, x2{Sample(0.0)}, y1{Sample(0.0)}, y2{Sample(0.0)};

                Sample process(Sample x) {
                    const Sample y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
                    x2             = x1;
                    x1             = x;
                    y2             = y1;
                    y1             = y;
                    return y;
                }
                void clear() { x1 = x2 = y1 = y2 = Sample(0.0); }
            };

            // Log-spaced band centre frequency for band i.
            static Sample band_frequency(int i) {
                const Sample t = (k_bands > 1) ? static_cast<Sample>(i) / (k_bands - 1) : Sample(0.0);
                return Sample(k_fmin) * std::pow(Sample(k_fmax) / Sample(k_fmin), t);
            }

            void recalc_filters() {
                if (m_sr <= Sample(0.0)) {
                    return;
                }
                const Sample q = (m_q > Sample(0.001)) ? m_q : Sample(0.001);

                for (int i = 0; i < k_bands; ++i) {
                    Sample fc = band_frequency(i);
                    fc        = std::min(fc, Sample(0.45) * m_sr); // keep every band well below Nyquist

                    const Sample w0    = Sample(2.0) * Sample(k_pi_for<Sample>) * fc / m_sr;
                    const Sample cosw0 = std::cos(w0);
                    const Sample alpha = std::sin(w0) / (Sample(2.0) * q);
                    const Sample a0    = Sample(1.0) + alpha;

                    // Constant 0 dB peak-gain bandpass (RBJ cookbook), normalised by a0.
                    const Sample b0 = alpha / a0;
                    const Sample b2 = -alpha / a0;
                    const Sample a1 = (-Sample(2.0) * cosw0) / a0;
                    const Sample a2 = (Sample(1.0) - alpha) / a0;

                    for (auto* b : {&m_mod, &m_car}) {
                        auto& bq = (*b)[i];
                        bq.b0    = b0;
                        bq.b1    = Sample(0.0);
                        bq.b2    = b2;
                        bq.a1    = a1;
                        bq.a2    = a2;
                    }
                }
            }

            void recalc_envelope() {
                if (m_sr <= Sample(0.0)) {
                    return;
                }
                const Sample tau =
                    (m_response_ms > Sample(0.0001) ? m_response_ms : Sample(0.0001)) * Sample(0.001); // ms → s
                m_env_coef = std::exp(-Sample(1.0) / (tau * m_sr));
            }

            Sample m_sr{Sample(0.0)};
            Sample m_q{Sample(20.0)};
            Sample m_response_ms{Sample(20.0)};
            Sample m_env_coef{Sample(0.0)};
            Sample m_gain{Sample(1.0)};

            std::array<biquad, k_bands> m_mod{};
            std::array<biquad, k_bands> m_car{};
            std::array<Sample, k_bands> m_env{};
        };

        /// The double profile — the golden model.
        using bank = basic_bank<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using bank32 = basic_bank<float>;

    } // namespace vocoder
} // namespace tap::tools
