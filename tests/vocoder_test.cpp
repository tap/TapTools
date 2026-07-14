/// @file
/// @brief      Unit tests for the channel-vocoder kernel (taptools::vocoder::bank).
/// @details    Checks the structural invariants: a silent carrier yields silence; makeup gain
///             scales the output linearly; a silent modulator lets the per-band envelopes decay to
///             silence; and processing is deterministic.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/vocoder.h>

namespace {

    constexpr double k_pi = 3.14159265358979323846;

    double sine(int t, double f, double sr) {
        return std::sin(2.0 * k_pi * f * t / sr);
    }

} // namespace

SCENARIO("a silent carrier yields a silent output regardless of the modulator") {
    taptools::vocoder::bank v;
    v.prepare(48000.0);

    double peak = 0.0;
    for (int t = 0; t < 8000; ++t) {
        const double out = v.process(sine(t, 220.0, 48000.0), 0.0);
        peak             = std::max(peak, std::abs(out));
    }
    REQUIRE(peak < 1e-12);
}

SCENARIO("makeup gain scales the output linearly") {
    taptools::vocoder::bank a, b;
    a.prepare(48000.0);
    b.prepare(48000.0);
    a.set_gain(1.0);
    b.set_gain(2.0);

    double maxerr = 0.0;
    for (int t = 0; t < 8000; ++t) {
        const double mod = sine(t, 300.0, 48000.0);
        const double car = sine(t, 1700.0, 48000.0);
        const double ya  = a.process(mod, car);
        const double yb  = b.process(mod, car);
        maxerr           = std::max(maxerr, std::abs(yb - 2.0 * ya));
    }
    REQUIRE(maxerr < 1e-12);
}

SCENARIO("a silent modulator lets the output decay to silence") {
    taptools::vocoder::bank v;
    v.prepare(48000.0);
    v.set_response_ms(20.0);

    // Warm up with a full-band modulator and carrier so the envelopes charge; record the level.
    double warm_peak = 0.0;
    for (int t = 0; t < 8000; ++t) {
        const double out = v.process(sine(t, 500.0, 48000.0), sine(t, 500.0, 48000.0));
        if (t > 6000) {
            warm_peak = std::max(warm_peak, std::abs(out));
        }
    }
    // Now silence the modulator; keep driving the carrier. The envelopes decay exponentially, so the
    // output falls far below the warmed level (a long window lets the 20 ms follower unwind).
    double late_peak = 0.0;
    for (int t = 0; t < 48000; ++t) {
        const double out = v.process(0.0, sine(t, 500.0, 48000.0));
        if (t > 44000) {
            late_peak = std::max(late_peak, std::abs(out));
        }
    }
    REQUIRE(warm_peak > 1e-6);             // the warm-up actually produced signal
    REQUIRE(late_peak < 1e-4 * warm_peak); // and it decayed away
}

SCENARIO("processing is deterministic") {
    auto run = []() {
        taptools::vocoder::bank v;
        v.prepare(44100.0);
        v.set_q(30.0);
        std::vector<double> out;
        for (int t = 0; t < 4000; ++t) {
            out.push_back(v.process(sine(t, 440.0, 44100.0), sine(t, 130.0, 44100.0)));
        }
        return out;
    };
    const std::vector<double> a = run(), b = run();
    REQUIRE(a == b);
}
