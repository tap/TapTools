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
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#pragma once

#include <cmath>
#include <vector>

#include "stft.h"

namespace taptools {
    namespace nr {

        class reducer {
          public:
            // Allocate for the given FFT size (a power of two). Resets running state.
            void configure(int fftsize) { m_stft.configure(fftsize); }

            // Flush the STFT buffers without changing the size.
            void reset() { m_stft.reset(); }

            // Linear-amplitude threshold: bins below this level are attenuated, bins above pass. 0
            // disables the gate (the output then reconstructs the input).
            void set_threshold(double t) { m_threshold = t; }

            // Soft-knee slope: 0 passes everything, higher values approach a hard gate.
            void set_slope(double s) { m_slope = s; }

            int fftsize() const { return m_stft.fftsize(); }
            int latency() const { return m_stft.latency(); }

            // Process n samples. Input and output must not alias.
            void process(const double* in, double* out, long n) {
                m_stft.process(in, out, n,
                               [this](std::vector<double>& re, std::vector<double>& im, int N) { gate(re, im, N); });
            }

          private:
            // Attenuate bins whose magnitude is below the threshold (soft-knee downward expansion).
            void gate(std::vector<double>& re, std::vector<double>& im, int N) const {
                const double thr = m_threshold;
                for (int k = 0; k < N; ++k) {
                    const double mag  = std::sqrt(re[k] * re[k] + im[k] * im[k]) * (2.0 / N);
                    double       gain = 1.0;
                    if (thr > 0.0 && mag < thr) {
                        gain = (m_slope <= 0.0) ? 1.0 : std::pow(mag / thr, m_slope);
                    }
                    re[k] *= gain;
                    im[k] *= gain;
                }
            }

            stft   m_stft;
            double m_threshold{0.01};
            double m_slope{2.0};
        };

    } // namespace nr
} // namespace taptools
