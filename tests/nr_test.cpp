/// @file
/// @brief      Unit tests for the spectral noise-reduction kernel (tap::tools::nr::reducer).
/// @details    Exercises the three behaviours that define the object: with the gate open the STFT
///             perfectly reconstructs the input (delayed by one frame); a tone below the threshold
///             is strongly attenuated; a tone above it passes through.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/nr.h>

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

    // Bin-centred sine (integer periods over N) so a single tone lands cleanly on one FFT bin.
    std::vector<double> tone(int n, int fftsize, int bin, double amp) {
        std::vector<double> v(n);
        for (int t = 0; t < n; ++t) {
            v[t] = amp * std::sin(2.0 * k_pi * bin * t / fftsize);
        }
        return v;
    }

    double rms(const std::vector<double>& v, int from, int to) {
        double acc = 0.0;
        for (int i = from; i < to; ++i) {
            acc += v[i] * v[i];
        }
        return std::sqrt(acc / (to - from));
    }

} // namespace

SCENARIO("with the gate open the STFT reconstructs the input, delayed by one frame") {
    const int             N = 256;
    tap::tools::nr::reducer r;
    r.configure(N);
    r.set_threshold(0.0); // gate disabled → identity spectral op

    REQUIRE(r.fftsize() == N);
    REQUIRE(r.latency() == N);

    GIVEN("a broadband noise input") {
        const int           len = 4096;
        std::vector<double> in  = noise(len, 42u);
        std::vector<double> out(len, 0.0);

        WHEN("processed") {
            r.process(in.data(), out.data(), len);

            THEN("out[t] == in[t - N] once the overlap pipeline has filled") {
                double maxerr = 0.0;
                for (int t = 2 * N; t < len; ++t) {
                    maxerr = std::max(maxerr, std::abs(out[t] - in[t - N]));
                }
                REQUIRE(maxerr < 1e-9);
            }
        }
    }
}

SCENARIO("a tone below the threshold is strongly attenuated") {
    const int             N = 512;
    tap::tools::nr::reducer r;
    r.configure(N);
    r.set_threshold(0.5); // high threshold
    r.set_slope(4.0);

    GIVEN("a quiet tone (amplitude 0.05, magnitude well under 0.5)") {
        const int           len = 8192;
        std::vector<double> in  = tone(len, N, 8, 0.05);
        std::vector<double> out(len, 0.0);

        WHEN("processed") {
            r.process(in.data(), out.data(), len);

            THEN("the output tail is far quieter than the input") {
                const double in_rms  = rms(in, 4 * N, len);
                const double out_rms = rms(out, 4 * N, len);
                REQUIRE(out_rms < 0.05 * in_rms);
            }
        }
    }
}

SCENARIO("a tone above the threshold passes through") {
    const int             N = 512;
    tap::tools::nr::reducer r;
    r.configure(N);
    r.set_threshold(0.01); // low threshold
    r.set_slope(4.0);

    GIVEN("a loud tone (amplitude 0.8, magnitude well over 0.01)") {
        const int           len = 8192;
        std::vector<double> in  = tone(len, N, 8, 0.8);
        std::vector<double> out(len, 0.0);

        WHEN("processed") {
            r.process(in.data(), out.data(), len);

            THEN("the output tail matches the input level (delayed reconstruction)") {
                const double in_rms  = rms(in, 4 * N, len);
                const double out_rms = rms(out, 4 * N, len);
                REQUIRE(std::abs(out_rms - in_rms) < 0.02 * in_rms);
            }
        }
    }
}
