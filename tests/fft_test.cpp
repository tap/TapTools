/// @file
/// @brief      Unit tests for the shared radix-2 FFT (taptools::fft::transform).
/// @details    Checks the forward transform against a naive O(N^2) DFT, forward→inverse
///             reconstruction, linearity of the convention (forward unscaled, inverse /N), and a
///             couple of closed-form cases (impulse → flat spectrum, single cosine → two bins).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/fft.h>

namespace {

    constexpr double k_pi = 3.14159265358979323846;

    // Naive O(N^2) DFT reference, same sign convention as taptools::fft (forward: exp(-i...)).
    void naive_dft(const std::vector<double>& re, const std::vector<double>& im, std::vector<double>& ore,
                   std::vector<double>& oim) {
        const int n = static_cast<int>(re.size());
        ore.assign(n, 0.0);
        oim.assign(n, 0.0);
        for (int k = 0; k < n; ++k) {
            double sr = 0.0, si = 0.0;
            for (int t = 0; t < n; ++t) {
                const double ang = -2.0 * k_pi * k * t / n;
                const double c = std::cos(ang), s = std::sin(ang);
                sr += re[t] * c - im[t] * s;
                si += re[t] * s + im[t] * c;
            }
            ore[k] = sr;
            oim[k] = si;
        }
    }

    std::vector<double> noise(int n, unsigned seed) {
        std::vector<double> v(n);
        unsigned            s = seed;
        for (int i = 0; i < n; ++i) {
            s    = s * 1664525u + 1013904223u;
            v[i] = (static_cast<double>(s) / 4294967295.0) * 2.0 - 1.0;
        }
        return v;
    }

    double max_abs_diff(const std::vector<double>& a, const std::vector<double>& b) {
        double m = 0.0;
        for (size_t i = 0; i < a.size(); ++i) {
            m = std::max(m, std::abs(a[i] - b[i]));
        }
        return m;
    }

} // namespace

SCENARIO("the forward FFT matches a naive DFT") {
    for (int n : {2, 4, 8, 16, 64, 256}) {
        GIVEN("random complex input of length " + std::to_string(n)) {
            std::vector<double> re = noise(n, 100u + n), im = noise(n, 900u + n);
            std::vector<double> ref_re, ref_im;
            naive_dft(re, im, ref_re, ref_im);

            std::vector<double> fre = re, fim = im;
            taptools::fft::transform(fre, fim, false);

            THEN("the outputs agree to within numerical tolerance") {
                REQUIRE(max_abs_diff(fre, ref_re) < 1e-9);
                REQUIRE(max_abs_diff(fim, ref_im) < 1e-9);
            }
        }
    }
}

SCENARIO("forward followed by inverse reconstructs the input") {
    const int                 n  = 128;
    std::vector<double>       re = noise(n, 7u), im = noise(n, 8u);
    const std::vector<double> re0 = re, im0 = im;

    taptools::fft::transform(re, im, false);
    taptools::fft::transform(re, im, true);

    THEN("the round trip returns the original (inverse divides by N)") {
        REQUIRE(max_abs_diff(re, re0) < 1e-9);
        REQUIRE(max_abs_diff(im, im0) < 1e-9);
    }
}

SCENARIO("a unit impulse transforms to a flat unit spectrum") {
    const int           n = 16;
    std::vector<double> re(n, 0.0), im(n, 0.0);
    re[0] = 1.0;
    taptools::fft::transform(re, im, false);
    bool flat = true;
    for (int k = 0; k < n; ++k) {
        if (std::abs(re[k] - 1.0) > 1e-12 || std::abs(im[k]) > 1e-12) {
            flat = false;
        }
    }
    REQUIRE(flat);
}

SCENARIO("a real cosine lands on the two symmetric bins") {
    const int           n    = 32;
    const int           freq = 3;
    std::vector<double> re(n), im(n, 0.0);
    for (int t = 0; t < n; ++t) {
        re[t] = std::cos(2.0 * k_pi * freq * t / n);
    }
    taptools::fft::transform(re, im, false);

    THEN("energy is N/2 at bin freq and N-freq, ~0 elsewhere") {
        REQUIRE(std::abs(re[freq] - n / 2.0) < 1e-9);
        REQUIRE(std::abs(re[n - freq] - n / 2.0) < 1e-9);
        for (int k = 0; k < n; ++k) {
            if (k != freq && k != n - freq) {
                REQUIRE(std::abs(re[k]) < 1e-9);
            }
        }
    }
}
