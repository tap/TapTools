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
    // Spelled `lane` here: several scenarios use `loop` as a local sample count.
    using lane = tap::tools::airport::loop;

    loop_bank make(double max_loop_seconds = 2.0) {
        loop_bank b;
        b.prepare(k_sr, max_loop_seconds);
        b.set_smooth_ms(0.0);
        return b;
    }

    void make_lane(lane& l, double max_loop_seconds = 2.0) {
        l.prepare(k_sr, max_loop_seconds);
        l.set_smooth_ms(0.0);
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

SCENARIO("standalone lanes summed are the bank, bitwise") {
    // The decomposition's load-bearing claim: tap.reel~ patched N times into a sum IS
    // tap.airport~. Bitwise, because every stage the claim passes through (transparent
    // playback, exact pan endpoints, the bypassed shade) is a bitwise promise already.
    constexpr int n_lanes      = 3;
    const double  len[n_lanes] = {0.53, 0.61, 0.71}; // incommensurate, all above the floor
    const double  lvl[n_lanes] = {0.4, 0.7, 0.55};
    const double  pn[n_lanes]  = {-1.0, 0.25, 1.0}; // both exact endpoints and one interior
    const double  drk[n_lanes] = {1000.0, tap::tools::tape::k_darken_ceil_hz, 4000.0}; // shaded, bypassed, shaded

    loop_bank b = make(1.5);
    b.set_loops(n_lanes);
    for (int i = 0; i < n_lanes; ++i) {
        b.set_length_seconds(i, len[i]);
        b.set_level(i, lvl[i]);
        b.set_pan(i, pn[i]);
        b.set_darken_hz(i, drk[i]);
    }

    std::vector<lane> lanes(n_lanes);
    for (int i = 0; i < n_lanes; ++i) {
        make_lane(lanes[static_cast<size_t>(i)], 1.5);
        lanes[static_cast<size_t>(i)].set_length_seconds(len[i]);
        lanes[static_cast<size_t>(i)].set_level(lvl[i]);
        lanes[static_cast<size_t>(i)].set_pan(pn[i]);
        lanes[static_cast<size_t>(i)].set_darken_hz(drk[i]);
    }

    bool         exact = true;
    double       peak  = 0.0;
    const size_t n     = at(2.0);
    for (size_t i = 0; i < n; ++i) {
        // Punch each lane in and out at staggered, unquantized points — identical schedules.
        for (int k = 0; k < n_lanes; ++k) {
            const size_t on  = 500 + 1300 * static_cast<size_t>(k);
            const size_t off = 20000 + 4100 * static_cast<size_t>(k);
            if (i == on) {
                b.record(k, true);
                lanes[static_cast<size_t>(k)].record(true);
            }
            if (i == off) {
                b.record(k, false);
                lanes[static_cast<size_t>(k)].record(false);
            }
        }

        const double t = static_cast<double>(i) / k_sr;
        const double x = 0.6 * std::sin(2.0 * 3.14159265358979323846 * 220.0 * t)
                         + 0.3 * std::sin(2.0 * 3.14159265358979323846 * 987.0 * t);

        double lb = 0.0, rb = 0.0;
        b.process(x, lb, rb);

        double ls = 0.0, rs = 0.0; // the patch: zero the busses, sum the lanes
        for (auto& one : lanes) {
            one.process(x, ls, rs);
        }

        exact = exact && (ls == lb) && (rs == rb);
        peak  = std::max(peak, std::max(std::abs(lb), std::abs(rb)));
    }
    REQUIRE(exact);
    REQUIRE(peak > 0.1); // and it was carrying the phrases, not agreeing about silence
}

SCENARIO("a lone lane's head is as sacred as one in the bank") {
    lane ln;
    make_lane(ln);
    ln.set_length_seconds(0.5);
    ln.set_pan(-1.0); // hard left: the left bus carries the lane bitwise

    double l = 0.0, r = 0.0;
    ln.record(true);
    ln.process(1.0, l, r); // plant a click wherever the head is
    ln.record(false);

    const size_t        loop = at(0.5);
    std::vector<double> yl(4 * loop, 0.0);
    for (size_t i = 0; i < yl.size(); ++i) {
        if (i == loop + 100) { // the same storm the bank scenario fires, one level down
            ln.set_level(1.0);
            ln.set_darken_hz(tap::tools::tape::k_darken_ceil_hz);
            ln.record(false);
            ln.set_length_seconds(0.5);
        }
        double rr = 0.0;
        ln.process(0.0, yl[i], rr); // process accumulates; yl[i] starts at zero
    }

    for (size_t k = 1; k <= 3; ++k) {
        INFO("return " << k);
        CHECK(yl[k * loop - 1] == 1.0);
    }
    INFO("phase after 4 loops + 1 planted sample: " << ln.phase());
    CHECK(std::abs(ln.phase() - 1.0 / static_cast<double>(loop)) < 1e-9);
}

SCENARIO("unprepared, a lone lane is silent and leaves the busses alone") {
    lane   ln;
    double l = 1.0, r = -1.0; // process() accumulates, so an unprepared lane must add nothing
    ln.process(0.7, l, r);
    REQUIRE(l == 1.0);
    REQUIRE(r == -1.0);
    REQUIRE(ln.phase() == 0.0);
}
