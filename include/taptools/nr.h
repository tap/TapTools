/// @file
/// @brief      Spectral noise-reduction kernel (the DSP core of tap.nr~).
/// @details    A spectral expander / gate: each STFT frame is transformed, and every bin whose
///             magnitude falls below `threshold` is attenuated while bins above it pass. `slope`
///             sets how gradually the gate engages across the threshold (a soft knee): low values
///             fade gently, high values approach a hard gate, 0 passes everything. Reconstructed
///             from tap.nr~'s reference documentation (the original was a pfft~-hosted object); this
///             is self-contained and runs its own STFT (see stft.h). With the gate open the output
///             reconstructs the input delayed by one FFT frame. Plain C++17, Min-free.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2003-2026 Timothy Place.

#pragma once

#include <cmath>
#include <vector>

#include "numeric.h"
#include "stft.h"

namespace tap::tools {
    namespace nr {

        template <typename Sample>

        class basic_reducer {
            static_assert(is_sample_profile<Sample>,

                          "basic_reducer supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            // Allocate for the given FFT size (a power of two). Resets running state.
            void configure(int fftsize) { m_stft.configure(fftsize); }

            // Flush the STFT buffers without changing the size.
            void reset() { m_stft.reset(); }

            // Linear-amplitude threshold: bins below this level are attenuated, bins above pass. 0
            // disables the gate (the output then reconstructs the input).
            void set_threshold(Sample t) { m_threshold = t; }

            // Soft-knee slope: 0 passes everything, higher values approach a hard gate.
            void set_slope(Sample s) { m_slope = s; }

            int fftsize() const { return m_stft.fftsize(); }
            int latency() const { return m_stft.latency(); }

            // Process n samples. Input and output must not alias.
            void process(const Sample* in, Sample* out, long n) {
                m_stft.process(in, out, n,
                               [this](std::vector<Sample>& re, std::vector<Sample>& im, int N) { gate(re, im, N); });
            }

          private:
            // Attenuate bins whose magnitude is below the threshold (soft-knee downward expansion).
            // Operates on the half-spectrum (bins 0..N/2); the real transform mirrors the rest.
            void gate(std::vector<Sample>& re, std::vector<Sample>& im, int N) const {
                const Sample thr  = m_threshold;
                const int    half = N / 2;
                for (int k = 0; k <= half; ++k) {
                    const Sample mag  = std::sqrt(re[k] * re[k] + im[k] * im[k]) * (Sample(2.0) / N);
                    Sample       gain = Sample(1.0);
                    if (thr > Sample(0.0) && mag < thr) {
                        gain = (m_slope <= Sample(0.0)) ? Sample(1.0) : std::pow(mag / thr, m_slope);
                    }
                    re[k] *= gain;
                    im[k] *= gain;
                }
            }

            basic_stft<Sample> m_stft;
            Sample             m_threshold{Sample(0.01)};
            Sample             m_slope{Sample(2.0)};
        };

        /// The double profile — the golden model.
        using reducer = basic_reducer<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using reducer32 = basic_reducer<float>;

    } // namespace nr
} // namespace tap::tools
