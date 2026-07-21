/// @file
/// @brief      Spectral-remapping kernel (the DSP core of tap.spectra~).
/// @details    Reorients the bins of an FFT onto different bins of an IFFT: each output bin k takes
///             its complex value from input bin round(k · remap), over the lower half, then the
///             upper half is mirrored to keep the spectrum Hermitian (so the inverse transform is
///             real). remap = 1 is the identity (the output reconstructs the input); > 1 and < 1
///             remap the spectrum for an ultra-non-linear effect. Reconstructed from tap.spectra~'s
///             reference documentation (the original was a pfft~ subpatcher); this is self-contained
///             and runs its own STFT (see stft.h). Plain C++17, Min-free.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#pragma once

#include <cmath>
#include <vector>

#include "stft.h"

namespace tap::tools {
    namespace spectra {

        class remapper {
          public:
            // Allocate for the given FFT size (a power of two). Resets running state.
            void configure(int fftsize) {
                m_stft.configure(fftsize);
                m_ore.assign(fftsize, 0.0);
                m_oim.assign(fftsize, 0.0);
            }

            // Flush the STFT buffers without changing the size.
            void reset() { m_stft.reset(); }

            // Remapping ratio: output bin k takes input bin round(k * remap). 1 is the identity.
            void set_remap(double r) { m_remap = r; }

            int fftsize() const { return m_stft.fftsize(); }
            int latency() const { return m_stft.latency(); }

            // Process n samples. Input and output must not alias.
            void process(const double* in, double* out, long n) {
                m_stft.process(in, out, n,
                               [this](std::vector<double>& re, std::vector<double>& im, int N) { remap(re, im, N); });
            }

          private:
            // Reorient the lower-half bins, enforce Hermitian symmetry, and write the result back into
            // the scaffold's spectrum buffers for the inverse transform.
            void remap(std::vector<double>& re, std::vector<double>& im, int N) {
                const int half = N / 2;

                for (int k = 0; k <= half; ++k) {
                    const int src = static_cast<int>(std::lround(k * m_remap));
                    if (src >= 0 && src <= half) {
                        m_ore[k] = re[src];
                        m_oim[k] = im[src];
                    }
                    else {
                        m_ore[k] = 0.0;
                        m_oim[k] = 0.0;
                    }
                }
                m_oim[0]    = 0.0; // DC is real
                m_oim[half] = 0.0; // Nyquist is real
                for (int k = 1; k < half; ++k) {
                    m_ore[N - k] = m_ore[k];
                    m_oim[N - k] = -m_oim[k];
                }

                for (int k = 0; k < N; ++k) {
                    re[k] = m_ore[k];
                    im[k] = m_oim[k];
                }
            }

            stft                m_stft;
            std::vector<double> m_ore, m_oim; // remapped spectrum scratch
            double              m_remap{1.0};
        };

    } // namespace spectra
} // namespace tap::tools
