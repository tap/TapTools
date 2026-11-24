/// @file
/// @brief      Tests for the oversampling chain's Butterworth low-pass (butterworth.h).
/// @details    The promise the swap from the kernels' private copies rests on: DspTap's stage is the
///             cookbook cascade ladder.h, diode_ladder.h, overdrive.h, svf.h, fuzz.h and ondes.h
///             carried, to within the copies' eight-decimal rounding of the Butterworth Qs. The
///             oracle is a reference copy of that cascade, written out here as the kernels had it:
///             RBJ cookbook low-pass biquads (Bristow-Johnson, "Audio EQ Cookbook") in transposed
///             direct form II, at the Q pairs 0.54119610 / 1.30656296 (order 4) and 0.50979558 /
///             0.60134489 / 0.89997622 / 2.56291545 (order 8). Every corner a kernel designs is
///             swept: 0.45 / os for os = 2, 4, 8 (ladder, diode, overdrive, svf, ondes) and fuzz's
///             0.225 per 2x stage.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place and the TapTools contributors

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/butterworth.h>

namespace {

    constexpr double k_pi = 3.14159265358979323846;

    /// The kernels' former private biquad, verbatim in its arithmetic.
    struct cookbook_biquad {
        double b0{1.0}, b1{0.0}, b2{0.0}, a1{0.0}, a2{0.0};
        double z1{0.0}, z2{0.0};

        void design_lowpass(double fc_norm, double q) {
            const double w     = 2.0 * k_pi * fc_norm;
            const double alpha = std::sin(w) / (2.0 * q);
            const double cw    = std::cos(w);
            const double a0    = 1.0 + alpha;
            b0                 = ((1.0 - cw) * 0.5) / a0;
            b1                 = (1.0 - cw) / a0;
            b2                 = b0;
            a1                 = (-2.0 * cw) / a0;
            a2                 = (1.0 - alpha) / a0;
        }
        double tick(double x) {
            const double y = b0 * x + z1;
            z1             = b1 * x - a1 * y + z2;
            z2             = b2 * x - a2 * y;
            return y;
        }
    };

    /// The kernels' former butterworth4 / butterworth8: the cascade at their Qs.
    template <std::size_t K>
    struct cookbook_cascade {
        std::array<cookbook_biquad, K> s{};

        cookbook_cascade(double fc_norm, const std::array<double, K>& q) {
            for (std::size_t i = 0; i < K; ++i) {
                s[i].design_lowpass(fc_norm, q[i]);
            }
        }
        double tick(double x) {
            for (auto& b : s) {
                x = b.tick(x);
            }
            return x;
        }
    };

    constexpr std::array<double, 2> k_q4 = {0.54119610, 1.30656296};
    constexpr std::array<double, 4> k_q8 = {0.50979558, 0.60134489, 0.89997622, 2.56291545};

    /// White noise at 0.9 of full scale, the same LCG as delay_test.cpp.
    std::vector<double> noise(std::size_t n) {
        std::vector<double> x(n);
        unsigned            s = 12345u;
        for (auto& v : x) {
            s = s * 1664525u + 1013904223u;
            v = 0.9 * (static_cast<double>(s) / 2147483648.0 - 1.0);
        }
        return x;
    }

    /// The largest |stage - cookbook| over the stream, the stage designed at fc as a kernel does it.
    template <std::size_t N>
    double max_difference(double fc, const std::array<double, N / 2>& q, const std::vector<double>& x) {
        tap::tools::butterworth_lowpass<N> stage;
        stage.design(fc);
        cookbook_cascade<N / 2> ref(fc, q);
        double                  worst = 0.0;
        for (const double v : x) {
            worst = std::max(worst, std::abs(stage.tick(v) - ref.tick(v)));
        }
        return worst;
    }

} // namespace

SCENARIO("the oversampling Butterworth is the cookbook cascade the kernels carried, within its Q rounding") {
    const std::vector<double> x = noise(48000);
    // Measured (x86-64, GCC 13 and Clang 18 alike): at most 2.9e-9 at order 4 and 3.7e-9 at order 8
    // over these corners, the copies' Q rounding (DspTap's stage is the cookbook cascade at the exact
    // Qs). Pinned at 1e-8, under three times the larger.
    for (const double fc : {0.45 / 2.0, 0.45 / 4.0, 0.45 / 8.0}) {
        CAPTURE(fc);
        CHECK(max_difference<4>(fc, k_q4, x) < 1e-8);
        CHECK(max_difference<8>(fc, k_q8, x) < 1e-8);
    }
}

SCENARIO("a default-constructed filter is the 2x chain's design, and redesigning there changes nothing") {
    GIVEN("a default-constructed order-4 filter and one designed at 0.45 / 2") {
        tap::tools::butterworth4 a;
        tap::tools::butterworth4 b;
        b.design(0.45 / 4.0);
        b.design(0.45 / 2.0);
        REQUIRE(a.cutoff() == tap::tools::butterworth4::k_nominal_cutoff);
        REQUIRE(b.cutoff() == 0.45 / 2.0);
        THEN("the two run the same bits, and design() at the current corner mid-stream is a no-op") {
            const std::vector<double> x = noise(4096);
            for (std::size_t i = 0; i < x.size(); ++i) {
                if (i == 2048) {
                    a.design(0.45 / 2.0);
                }
                REQUIRE(a.tick(x[i]) == b.tick(x[i]));
            }
        }
    }
}

SCENARIO("reset() returns the filter to silence and keeps its design") {
    tap::tools::butterworth8 f;
    f.design(0.45 / 8.0);
    for (const double v : noise(1024)) {
        f.tick(v);
    }
    f.reset();
    REQUIRE(f.cutoff() == 0.45 / 8.0);
    for (int i = 0; i < 1024; ++i) {
        REQUIRE(f.tick(0.0) == 0.0);
    }
}
