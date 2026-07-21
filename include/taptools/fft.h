/// @file
/// @brief      Shared in-place radix-2 Cooley–Tukey FFT for the TapTools spectral kernels.
/// @details    A single copy of the routine that tap.convolve~ (conv_engine), tap.nr~, and
///             tap.spectra~ all use — it lived, byte-identical, in each of them before being
///             consolidated here. Plain C++17, standard library only: no Jamoma, no external FFT
///             library. The transform is in place; `inverse` divides by N (forward is unscaled),
///             so a forward followed by an inverse reconstructs the input. Size must be a power of
///             two (the callers guarantee this).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#pragma once

#include <cmath>
#include <vector>

namespace tap::tools {
    namespace fft {

        inline constexpr double k_pi = 3.14159265358979323846;

        // In-place iterative radix-2 Cooley–Tukey FFT. `inverse` divides by N (forward is unscaled).
        // re/im must have the same, power-of-two length.
        inline void transform(std::vector<double>& re, std::vector<double>& im, bool inverse) {
            const int n = static_cast<int>(re.size());

            for (int i = 1, j = 0; i < n; ++i) {
                int bit = n >> 1;
                for (; j & bit; bit >>= 1) {
                    j ^= bit;
                }
                j ^= bit;
                if (i < j) {
                    std::swap(re[i], re[j]);
                    std::swap(im[i], im[j]);
                }
            }

            for (int len = 2; len <= n; len <<= 1) {
                const double ang = 2.0 * k_pi / len * (inverse ? 1.0 : -1.0);
                const double wr  = std::cos(ang);
                const double wi  = std::sin(ang);
                for (int i = 0; i < n; i += len) {
                    double cwr = 1.0, cwi = 0.0;
                    for (int k = 0; k < len / 2; ++k) {
                        const double vr     = re[i + k + len / 2] * cwr - im[i + k + len / 2] * cwi;
                        const double vi     = re[i + k + len / 2] * cwi + im[i + k + len / 2] * cwr;
                        const double ur     = re[i + k];
                        const double ui     = im[i + k];
                        re[i + k]           = ur + vr;
                        im[i + k]           = ui + vi;
                        re[i + k + len / 2] = ur - vr;
                        im[i + k + len / 2] = ui - vi;
                        const double ncwr   = cwr * wr - cwi * wi;
                        cwi                 = cwr * wi + cwi * wr;
                        cwr                 = ncwr;
                    }
                }
            }

            if (inverse) {
                for (int i = 0; i < n; ++i) {
                    re[i] /= n;
                    im[i] /= n;
                }
            }
        }

    } // namespace fft
} // namespace tap::tools
