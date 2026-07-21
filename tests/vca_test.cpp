/// @file
/// @brief      Unit tests for the VCA kernel (tap::tools::vca).
/// @details    Pins the two-circuit contract: `clean` is an exact linear multiply; `warm` is the
///             303's slope-normalized biased-tanh transistor stage — unity slope through zero
///             (quiet signals near-clean), asymmetric even-harmonic saturation and compression on
///             hot signals, the operating-point DC removed by construction, and the optional
///             output-coupling DC block on AC material. Also pins the shared-implementation
///             invariant: vca::shape() with the stock constants reproduces the exact saturator the
///             TB-303 voice uses, so the extraction left tap.303~ bit-identical.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/tb303_voice.h>
#include <taptools/vca.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    // The reference saturator, written longhand exactly as the 303 voice used to inline it.
    double reference_shape(double v, double d, double b) {
        const double off  = std::tanh(b);
        const double norm = 1.0 / (d * (1.0 - off * off));
        return (std::tanh(d * v + b) - off) * norm;
    }

    double rms(const std::vector<double>& x) {
        double s = 0.0;
        for (double v : x)
            s += v * v;
        return std::sqrt(s / x.size());
    }

    double mean(const std::vector<double>& x) {
        double s = 0.0;
        for (double v : x)
            s += v;
        return s / x.size();
    }

} // namespace

SCENARIO("the clean circuit is an exact linear multiply") {
    tap::tools::vca a;
    a.prepare(k_sr);
    REQUIRE(a.circuit() == tap::tools::vca::mode_clean); // default

    THEN("process(x, gain) == x * gain to the bit, for any gain incl. negative and > 1") {
        for (double g : {0.0, 0.5, 1.0, 2.5, -0.75}) {
            for (double x : {-1.0, -0.3, 0.0, 0.42, 0.9}) {
                REQUIRE(a.process(x, g) == x * g);
            }
        }
    }
    THEN("shape() is the identity in clean mode") {
        REQUIRE(a.shape(0.37) == 0.37);
        REQUIRE(a.shape(-1.5) == -1.5);
    }
}

SCENARIO("the warm circuit reproduces the 303 transistor saturator exactly") {
    tap::tools::vca a;
    a.prepare(k_sr);
    a.set_mode(tap::tools::vca::mode_warm);
    REQUIRE(a.drive() == tap::tools::vca::k_default_drive); // stock 303 constants
    REQUIRE(a.bias() == tap::tools::vca::k_default_bias);

    THEN("shape(v) matches the closed-form reference at the stock constants, bit for bit") {
        for (double v : {-1.2, -0.5, -0.1, 0.0, 0.1, 0.5, 1.0, 1.7}) {
            REQUIRE(a.shape(v) == reference_shape(v, a.drive(), a.bias()));
        }
    }
    THEN("unity slope through zero: tiny signals pass essentially clean") {
        REQUIRE(a.shape(0.0) == 0.0);                   // operating-point DC removed
        REQUIRE(std::abs(a.shape(1e-4) - 1e-4) < 1e-7); // slope 1 at the origin
    }
    THEN("hot signals compress (|out| < |in|) and stay monotonic") {
        double prev = a.shape(-2.0);
        for (double v = -2.0 + 0.05; v <= 2.0; v += 0.05) {
            const double y = a.shape(v);
            REQUIRE(y > prev); // strictly increasing
            prev = y;
        }
        REQUIRE(std::abs(a.shape(1.5)) < 1.5); // compression at the top
    }
}

SCENARIO("the extraction left the TB-303 voice routed through the shared stage") {
    namespace tb = tap::tools::tb303;

    auto play = [](int mode) {
        tb::voice v;
        v.prepare(k_sr);
        v.set_vca(mode);
        v.note_on(45.0, 1.0); // hot accented A1 so the warm stage bites
        std::vector<double> out(static_cast<size_t>(0.2 * k_sr));
        v.process(out.data(), out.size());
        return out;
    };

    WHEN("the warm voice runs") {
        auto warm  = play(tb::vca_warm);
        auto clean = play(tb::vca_clean);
        THEN("it differs from clean (the saturator is engaged) and stays finite") {
            double d = 0.0;
            for (size_t i = 0; i < warm.size(); ++i) {
                d += std::abs(warm[i] - clean[i]);
                REQUIRE(std::isfinite(warm[i]));
            }
            REQUIRE(d > 0.0);
        }
    }
}

SCENARIO("the warm circuit's output DC block sheds signal-dependent DC on AC material") {
    auto run = [](bool dc_block) {
        tap::tools::vca a;
        a.prepare(k_sr);
        a.set_mode(tap::tools::vca::mode_warm);
        a.set_dc_block(dc_block);
        std::vector<double> out;
        out.reserve(static_cast<size_t>(k_sr));
        const double f = 220.0;
        for (size_t i = 0; i < static_cast<size_t>(k_sr); ++i)
            out.push_back(a.process(std::sin(2.0 * k_pi * f * static_cast<double>(i) / k_sr), 1.0));
        return out;
    };

    THEN("with the block on the mean is ~0; with it off a DC offset remains; energy is preserved") {
        auto                on  = run(true);
        auto                off = run(false);
        std::vector<double> on_ss(on.begin() + on.size() / 2, on.end()); // skip settling
        std::vector<double> off_ss(off.begin() + off.size() / 2, off.end());
        REQUIRE(std::abs(mean(on_ss)) < 1e-3);
        REQUIRE(std::abs(mean(off_ss)) > 10.0 * std::abs(mean(on_ss))); // off carries real DC
        REQUIRE(std::abs(mean(off_ss)) > 1e-3);
        REQUIRE(rms(on_ss) > 0.5 * rms(off_ss)); // DC-only: signal not gutted
    }
}

SCENARIO("the swing circuit is the TR-808 symmetric harmonic saturator") {
    THEN("swing_shape is the exact linear passthru at drive 0 (calibrated voices stay bit-identical)") {
        for (double v : {-1.3, -0.4, 0.0, 0.25, 0.9, 1.6})
            REQUIRE(tap::tools::vca::swing_shape(v, 0.0) == v);
    }
    THEN("with drive > 0 it is an odd function — symmetric, no even harmonics, no DC") {
        for (double d : {0.5, 2.0, 6.0})
            for (double v : {0.15, 0.6, 1.2})
                REQUIRE(std::abs(tap::tools::vca::swing_shape(v, d) + tap::tools::vca::swing_shape(-v, d)) < 1e-12);
    }
    THEN("unity slope through zero (quiet signals near-clean) and compression on hot signals") {
        const double d = 2.0;
        REQUIRE(std::abs(tap::tools::vca::swing_shape(1e-4, d) - 1e-4) < 1e-7);
        REQUIRE(std::abs(tap::tools::vca::swing_shape(1.5, d)) < 1.5);
    }
    GIVEN("a vca in swing mode") {
        tap::tools::vca a;
        a.prepare(k_sr);
        a.set_mode(tap::tools::vca::mode_swing);
        a.set_drive(2.0);
        THEN("shape() routes through the shared swing_shape and needs no DC block (symmetric)") {
            REQUIRE(a.shape(0.7) == tap::tools::vca::swing_shape(0.7, a.drive()));
            // a symmetric shaper on a symmetric sweep has zero mean either way
            double mean_on = 0.0;
            int    n       = 0;
            for (double ph = 0.0; ph < 2.0 * k_pi; ph += 0.001, ++n)
                mean_on += a.process(std::sin(ph), 1.0);
            REQUIRE(std::abs(mean_on / n) < 1e-3);
        }
    }
}

SCENARIO("drive and bias are live and reshape the curve") {
    tap::tools::vca a;
    a.prepare(k_sr);
    a.set_mode(tap::tools::vca::mode_warm);

    THEN("more drive means more compression on a hot signal") {
        a.set_drive(1.0);
        const double soft = a.shape(1.0);
        a.set_drive(4.0);
        const double hard = a.shape(1.0);
        REQUIRE(hard < soft);
    }
    THEN("zero bias is an odd function (no even harmonics, no DC term)") {
        a.set_bias(0.0);
        for (double v : {0.2, 0.6, 1.1})
            REQUIRE(std::abs(a.shape(v) + a.shape(-v)) < 1e-12);
    }
}
