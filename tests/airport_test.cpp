/// @file
/// @brief      Catch2 scenarios pinning the tap.airport~ kernel (airport.h).
/// @details    The promises are structural, so the measurements are exact: recorded phrases must
///             return on their loop's grid bit-for-bit, no setter may touch a phase (the free-run
///             IS the piece), hard pans must be bitwise absent from the far bus, and the
///             composite period of coprime loop lengths must be their lcm to the sample.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/airport.h>

namespace {

    constexpr double k_sr = 48000.0;

    using tap::tools::airport::loop_bank;

    loop_bank make(double max_loop_seconds = 2.0) {
        loop_bank b;
        b.prepare(k_sr, max_loop_seconds);
        b.set_smooth_ms(0.0);
        return b;
    }

    size_t at(double seconds) {
        return static_cast<size_t>(seconds * k_sr);
    }

    /// Punch a single unit impulse onto `loop` at its current head position.
    void plant_click(loop_bank& b, int loop) {
        double l = 0.0, r = 0.0;
        b.record(loop, true);
        b.process(1.0, l, r);
        b.record(loop, false);
    }

    double goertzel(const std::vector<double>& x, double f, size_t begin, size_t end) {
        const double w    = 2.0 * 3.14159265358979323846 * f / k_sr;
        const double coef = 2.0 * std::cos(w);
        double       s1 = 0.0, s2 = 0.0;
        for (size_t i = begin; i < end; ++i) {
            const double s = x[i] + coef * s1 - s2;
            s2             = s1;
            s1             = s;
        }
        const double n  = static_cast<double>(end - begin);
        const double re = s1 - s2 * std::cos(w);
        const double im = s2 * std::sin(w);
        return 2.0 * std::sqrt(re * re + im * im) / n;
    }

} // namespace

SCENARIO("a recorded phrase returns every loop period and no setter resets the phase") {
    loop_bank b = make();
    b.set_loops(1);
    b.set_length_seconds(0, 0.5);
    b.set_pan(0, -1.0); // hard left: the left bus carries the loop bitwise

    plant_click(b, 0);

    const size_t        loop = at(0.5);
    std::vector<double> yl(4 * loop, 0.0);
    double              r = 0.0;
    for (size_t i = 0; i < yl.size(); ++i) {
        if (i == loop + 100) { // mid-run setter storm: none of these may touch the head
            b.set_level(0, 1.0);
            b.set_darken_hz(0, tap::tools::tape::k_darken_ceil_hz);
            b.record(0, false);
            b.set_length_seconds(0, 0.5);
            b.set_loops(1);
        }
        b.process(0.0, yl[i], r);
    }

    // The click was planted one sample into the run, so it returns at loop - 1, 2*loop - 1, ...
    for (size_t k = 1; k <= 3; ++k) {
        INFO("return " << k);
        CHECK(yl[k * loop - 1] == 1.0); // bitwise: transparent playback of the same imprint
    }

    // And the head advances by exactly the samples processed, storm or no storm.
    const double ph = b.phase(0);
    INFO("phase after 4 loops + 1 planted sample: " << ph);
    CHECK(std::abs(ph - 1.0 / static_cast<double>(loop)) < 1e-9);
}

SCENARIO("record off freezes the tape bit-exactly") {
    loop_bank b = make();
    b.set_loops(1);
    b.set_length_seconds(0, 0.5);
    b.set_pan(0, -1.0);

    const size_t loop = at(0.5);
    double       l = 0.0, r = 0.0;
    b.record(0, true);
    for (size_t i = 0; i < loop; ++i) { // one full pass of a phrase, then freeze
        const double t = static_cast<double>(i) / k_sr;
        b.process(0.7 * std::sin(2.0 * 3.14159265358979323846 * 440.0 * t), l, r);
    }
    b.record(0, false);

    std::vector<double> pass_a(loop, 0.0), pass_b(loop, 0.0);
    for (size_t i = 0; i < loop; ++i) {
        b.process(0.0, pass_a[i], r);
    }
    for (size_t i = 0; i < loop; ++i) {
        b.process(0.0, pass_b[i], r);
    }
    bool exact = true;
    for (size_t i = 0; i < loop; ++i) {
        exact = exact && (pass_a[i] == pass_b[i]); // bitwise, not approximately
    }
    REQUIRE(exact);
    REQUIRE(*std::max_element(pass_a.begin(), pass_a.end()) > 0.5); // and it is the phrase, not silence
}

SCENARIO("two incommensurate loops realign only at the lcm") {
    loop_bank b = make();
    b.set_loops(2);
    b.set_length_seconds(0, 0.5);   // 24000 samples
    b.set_length_seconds(1, 0.625); // 30000 samples; gcd 6000 -> lcm 120000 samples = 2.5 s
    b.set_pan(0, -1.0);
    b.set_pan(1, -1.0); // both on the left bus: the sum is where coincidence lives

    REQUIRE(b.composite_period_seconds() == 2.5);

    plant_click(b, 0);
    plant_click(b, 1);

    const size_t        period = at(2.5);
    std::vector<double> yl(2 * period, 0.0);
    double              r = 0.0;
    for (size_t i = 0; i < yl.size(); ++i) {
        b.process(0.0, yl[i], r);
    }

    bool repeats_at_lcm = true;
    for (size_t i = 0; i < period; ++i) {
        repeats_at_lcm = repeats_at_lcm && (yl[i] == yl[i + period]);
    }
    REQUIRE(repeats_at_lcm);

    bool differs_at_half = false;
    for (size_t i = 0; i < period / 2; ++i) {
        differs_at_half = differs_at_half || (yl[i] != yl[i + period / 2]);
    }
    REQUIRE(differs_at_half); // half the lcm is not a period: the pattern is still drifting
}

SCENARIO("a hard-panned loop is bitwise absent from the far bus") {
    loop_bank b = make();
    b.set_loops(1);
    b.set_length_seconds(0, 0.5);
    b.set_pan(0, -1.0);

    plant_click(b, 0);

    double l = 0.0, r = 0.0;
    bool   right_silent = true;
    double left_peak    = 0.0;
    for (size_t i = 0; i < at(1.5); ++i) {
        b.process(0.0, l, r);
        right_silent = right_silent && (r == 0.0);
        left_peak    = std::max(left_peak, std::abs(l));
    }
    REQUIRE(right_silent);
    REQUIRE(left_peak == 1.0);
}

SCENARIO("darken shades one loop's playback and only that loop's") {
    loop_bank b = make();
    b.set_loops(2);
    b.set_length_seconds(0, 0.5);
    b.set_length_seconds(1, 0.5);
    b.set_pan(0, -1.0); // shaded loop on the left bus
    b.set_pan(1, 1.0);  // transparent loop on the right
    b.set_darken_hz(0, 1000.0);

    const double f = 6000.0;
    double       l = 0.0, r = 0.0;
    b.record(0, true);
    b.record(1, true);
    for (size_t i = 0; i < at(0.5); ++i) { // the same phrase onto both tapes
        const double t = static_cast<double>(i) / k_sr;
        b.process(0.6 * std::sin(2.0 * 3.14159265358979323846 * f * t), l, r);
    }
    b.record(0, false);
    b.record(1, false);

    std::vector<double> yl(at(1.0), 0.0), yr(at(1.0), 0.0);
    for (size_t i = 0; i < yl.size(); ++i) {
        b.process(0.0, yl[i], yr[i]);
    }

    // Predicted shade at 6 kHz for a 1 kHz one-pole (+ the wear DC blocker, ~1 up there).
    const double a         = 1.0 - std::exp(-2.0 * 3.14159265358979323846 * 1000.0 / k_sr);
    const double w         = 2.0 * 3.14159265358979323846 * f / k_sr;
    const double re        = 1.0 - (1.0 - a) * std::cos(w);
    const double im        = (1.0 - a) * std::sin(w);
    const double predicted = a / std::sqrt(re * re + im * im);

    const double shaded      = goertzel(yl, f, at(0.25), at(0.75));
    const double transparent = goertzel(yr, f, at(0.25), at(0.75));
    const double measured    = shaded / transparent;
    INFO("6 kHz through the 1 kHz shade: measured " << measured << ", predicted " << predicted);
    CHECK(std::abs(measured - predicted) < 0.2 * predicted);
    CHECK(transparent > 0.5); // and the transparent loop really is untouched
}

SCENARIO("a length change is a splice: phase re-wraps and never rewinds") {
    loop_bank b = make();
    b.set_loops(1);
    b.set_length_seconds(0, 1.0);

    double l = 0.0, r = 0.0;
    for (size_t i = 0; i < at(0.9); ++i) {
        b.process(0.0, l, r);
    }
    REQUIRE(std::abs(b.phase(0) - 0.9) < 1e-9);

    b.set_length_seconds(0, 0.5); // head at 43200 of 24000: re-wraps to 19200, not to zero
    INFO("phase after the splice: " << b.phase(0));
    REQUIRE(std::abs(b.phase(0) - 0.8) < 1e-9);
}

SCENARIO("unprepared, the bank emits silence") {
    loop_bank b;
    b.set_loops(2);
    double l = 1.0, r = 1.0;
    b.process(0.7, l, r);
    REQUIRE(l == 0.0);
    REQUIRE(r == 0.0);
}
