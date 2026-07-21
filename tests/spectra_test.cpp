/// @file
/// @brief      Unit tests for the spectral-remapping kernel (tap::tools::spectra::remapper).
/// @details    remap = 1 reconstructs the input (delayed by one frame); remap = 2 relocates a tone
///             at input bin 2k to output bin k (checked by transforming the steady-state output).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/fft.h>
#include <taptools/spectra.h>

namespace {

    constexpr double k_pi = 3.14159265358979323846;

    std::vector<double> noise(int n, unsigned seed) {
        std::vector<double> v(n);
        unsigned            s = seed;
        for (int i = 0; i < n; ++i) {
            s    = s * 1664525u + 1013904223u;
            v[i] = (static_cast<double>(s) / 4294967295.0) * 2.0 - 1.0;
        }
        return v;
    }

    std::vector<double> tone(int n, int fftsize, int bin, double amp) {
        std::vector<double> v(n);
        for (int t = 0; t < n; ++t) {
            v[t] = amp * std::sin(2.0 * k_pi * bin * t / fftsize);
        }
        return v;
    }

    // Peak magnitude bin of an N-sample slice of `sig` starting at `off` (rectangular window).
    int peak_bin(const std::vector<double>& sig, int off, int N) {
        std::vector<double> re(sig.begin() + off, sig.begin() + off + N), im(N, 0.0);
        tap::tools::fft::transform(re, im, false);
        int    best    = 0;
        double bestmag = -1.0;
        for (int k = 1; k <= N / 2; ++k) {
            const double mag = re[k] * re[k] + im[k] * im[k];
            if (mag > bestmag) {
                bestmag = mag;
                best    = k;
            }
        }
        return best;
    }

} // namespace

SCENARIO("remap = 1 reconstructs the input, delayed by one frame") {
    const int                   N = 256;
    tap::tools::spectra::remapper r;
    r.configure(N);
    r.set_remap(1.0);

    REQUIRE(r.fftsize() == N);
    REQUIRE(r.latency() == N);

    const int           len = 4096;
    std::vector<double> in  = noise(len, 314u);
    std::vector<double> out(len, 0.0);
    r.process(in.data(), out.data(), len);

    THEN("out[t] == in[t - N] once the pipeline has filled") {
        double maxerr = 0.0;
        for (int t = 2 * N; t < len; ++t) {
            maxerr = std::max(maxerr, std::abs(out[t] - in[t - N]));
        }
        REQUIRE(maxerr < 1e-9);
    }
}

SCENARIO("remap = 2 relocates input bin 2k to output bin k") {
    const int                   N = 512;
    tap::tools::spectra::remapper r;
    r.configure(N);
    r.set_remap(2.0);

    GIVEN("a tone at input bin 16") {
        const int           len = 8192;
        std::vector<double> in  = tone(len, N, 16, 0.8);
        std::vector<double> out(len, 0.0);

        WHEN("processed") {
            r.process(in.data(), out.data(), len);

            THEN("the steady-state output tone is at bin 8 (= 16 / 2)") {
                // Frame-aligned slice well into steady state.
                REQUIRE(peak_bin(out, 4 * N, N) == 8);
            }
        }
    }
}
