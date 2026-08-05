/// @file
/// @brief      Unit tests for the delay pair (tap.delay~ / tap.multitap~ kernels).
/// @details    Pins the rebuild's contract: Hermite fractional taps are phase-accurate against the
///             analytic shift; interp 0 reproduces the legacy integer truncation bit-for-bit;
///             feedback is capped at k_fb_max with a DC-blocked loop (a DC step recirculates once
///             and decays instead of accumulating); the equal-power mix and pan laws have exact
///             endpoints; and a modulated time under Hermite stays smooth where truncation
///             zipper-steps by whole samples.
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <cmath>
#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/delay.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    namespace dly = tap::tools::delay;

    dly::line make_line(double max_ms = 1000.0) {
        dly::line d;
        d.prepare(k_sr, max_ms);
        d.set_smooth_ms(0.0);
        return d;
    }

    // Deterministic noise in about [-1, 1], never denormal-small (LCG granularity ~4.7e-10).
    struct noise {
        uint32_t state{2463534242u};
        double   operator()() {
            state = state * 1664525u + 1013904223u;
            return (static_cast<double>(state) / 2147483648.0) - 1.0;
        }
    };

} // namespace

SCENARIO("a fractional Hermite tap of a sine is phase-accurate against the analytic shift") {
    // 440 Hz through a 123.37-sample tap: once the line is charged, every output sample must sit
    // on the analytically delayed sine to interpolator precision.
    const double d_samples = 123.37;
    const double w         = 2.0 * k_pi * 440.0 / k_sr;

    auto d = make_line();
    d.set_mix(100.0);
    d.set_feedback(0.0);
    d.set_interp(dly::interp_hermite);
    d.set_time_ms(d_samples * 1000.0 / k_sr);

    const int n       = 4000;
    double    max_err = 0.0;
    for (int i = 0; i < n; ++i) {
        const double y = d.process(std::sin(w * i));
        if (i > 200) { // past the charge-up
            max_err = std::max(max_err, std::abs(y - std::sin(w * (i - d_samples))));
        }
    }
    INFO("max deviation from the analytic shift: " << max_err);
    REQUIRE(max_err < 1e-4);
}

SCENARIO("interp 0 reproduces the legacy integer truncation exactly") {
    // 7.9 ms at 48 kHz is 379.2 samples; the legacy map long(ms * sr / 1000) truncates to 379.
    // With mix 100 (exact wet endpoint) and no feedback the kernel must be bit-identical to a
    // plain x[n - 379] reference.
    auto d = make_line();
    d.set_mix(100.0);
    d.set_feedback(0.0);
    d.set_interp(dly::interp_trunc);
    d.set_time_ms(7.9);

    noise               rng;
    const int           n = 2000;
    const long          k = 379;
    std::vector<double> x(n);
    for (auto& v : x) {
        v = rng();
    }

    bool exact = true;
    for (int i = 0; i < n; ++i) {
        const double y   = d.process(x[static_cast<size_t>(i)]);
        const double ref = (i >= k) ? x[static_cast<size_t>(i - k)] : 0.0;
        exact            = exact && (y == ref); // bitwise, not approximately
    }
    REQUIRE(exact);
}

SCENARIO("feedback is capped at 0.99 and the loop is DC-blocked") {
    // The cap is a hard contract point.
    auto d = make_line();
    d.set_feedback(2.0);
    REQUIRE(d.feedback() == dly::k_fb_max);

    // Drive a DC step through a 5 ms loop at the cap. Without the blocker the wet output would
    // integrate toward 1/(1 - 0.99) = 100 (past 5.0 within ~25 ms); with it, DC passes the first
    // tap once and every recirculation is high-passed. The recirculating step *edge* does overlap
    // itself (the blocker's ~1000-sample tail is longer than the 240-sample loop), so the driven
    // peak measures ~3.2 — bounded, not integrating. When the step is released, the DC caught in
    // the loop must decay away.
    d.set_mix(100.0);
    d.set_time_ms(5.0);

    const int drive = static_cast<int>(1.0 * k_sr);
    const int tail  = static_cast<int>(2.5 * k_sr);

    double driven_max = 0.0;
    for (int i = 0; i < drive; ++i) {
        driven_max = std::max(driven_max, std::abs(d.process(1.0)));
    }
    INFO("max driven output: " << driven_max);
    REQUIRE(driven_max < 5.0); // an unblocked loop passes this within ~25 ms and never comes back

    double late = 0.0;
    for (int i = 0; i < tail; ++i) {
        const double y = d.process(0.0);
        if (i >= tail - static_cast<int>(0.1 * k_sr)) {
            late += std::abs(y);
        }
    }
    late /= 0.1 * k_sr;
    INFO("mean |out| over the last 100 ms of the tail: " << late);
    REQUIRE(late < 0.05); // the step has decayed out of the loop
}

SCENARIO("the mix law endpoints are exact: 0 is dry only, 100 is wet only") {
    noise rng;

    // mix 0: output is the input, bitwise.
    {
        auto d = make_line();
        d.set_mix(0.0);
        d.set_feedback(0.5);
        d.set_time_ms(3.0);
        bool exact = true;
        for (int i = 0; i < 1000; ++i) {
            const double x = rng();
            exact          = exact && (d.process(x) == x);
        }
        REQUIRE(exact);
    }

    // mix 100: an impulse produces nothing until the tap, then exactly the impulse.
    {
        auto d = make_line();
        d.set_mix(100.0);
        d.set_feedback(0.0);
        d.set_time_ms(10.0); // 480 samples exactly
        bool pre_silent = true;
        for (int i = 0; i < 480; ++i) {
            pre_silent = pre_silent && (d.process(i == 0 ? 1.0 : 0.0) == 0.0);
        }
        REQUIRE(pre_silent);
        REQUIRE(d.process(0.0) == 1.0);
    }
}

SCENARIO("the multitap pan law is equal-power with exact endpoints") {
    dly::multitap m;
    m.prepare(k_sr, 1000.0);
    m.set_smooth_ms(0.0);
    m.set_taps(1);
    m.set_time_ms(0, 5.0); // 240 samples exactly
    m.set_gain(0, 1.0);
    m.set_pan(0, -1.0);

    // Full left: the right bus never sees the tap, bitwise; the left bus gets the impulse whole.
    const int n = 600;
    double    l = 0.0, r = 0.0;
    bool      right_silent = true;
    double    left_at_tap  = 0.0;
    for (int i = 0; i < n; ++i) {
        m.process(i == 0 ? 1.0 : 0.0, l, r);
        right_silent = right_silent && (r == 0.0);
        if (i == 240) {
            left_at_tap = l;
        }
    }
    REQUIRE(right_silent);
    REQUIRE(left_at_tap == 1.0);

    // Center: both buses at cos(pi/4) = 1/sqrt(2).
    m.clear();
    m.set_pan(0, 0.0);
    double center_l = 0.0, center_r = 0.0;
    for (int i = 0; i <= 240; ++i) {
        m.process(i == 0 ? 1.0 : 0.0, l, r);
        if (i == 240) {
            center_l = l;
            center_r = r;
        }
    }
    REQUIRE(std::abs(center_l - 1.0 / std::sqrt(2.0)) < 1e-12);
    REQUIRE(std::abs(center_r - 1.0 / std::sqrt(2.0)) < 1e-12);
}

SCENARIO("modulating the time under Hermite keeps sample deltas bounded where truncation steps") {
    // Sweep the tap from 15 ms down to 5 ms (480 samples of travel) under a 2 kHz sine, via the
    // signal-rate override. Hermite output is a mild doppler (deltas within a few percent of the
    // static sine's analytic bound); truncation skips a whole input sample at each integer
    // crossing, roughly doubling the worst delta. This is the promise interp 1 exists to keep.
    const double freq        = 2000.0;
    const double w           = 2.0 * k_pi * freq / k_sr;
    const double delta_clean = 2.0 * std::sin(0.5 * w); // analytic max per-sample delta of the sine
    const int    n_fill      = 2000;
    const int    n_sweep     = 24000;

    const auto max_delta = [&](int interp) {
        auto d = make_line();
        d.set_mix(100.0);
        d.set_feedback(0.0);
        d.set_interp(interp);
        double worst = 0.0;
        double prev  = 0.0;
        for (int i = 0; i < n_fill + n_sweep; ++i) {
            const double t =
                (i < n_fill) ? 15.0 : 15.0 - 10.0 * static_cast<double>(i - n_fill) / static_cast<double>(n_sweep);
            const double y = d.process(std::sin(w * i), t);
            if (i > n_fill + 100) {
                worst = std::max(worst, std::abs(y - prev));
            }
            prev = y;
        }
        return worst;
    };

    const double hermite = max_delta(dly::interp_hermite);
    const double trunc   = max_delta(dly::interp_trunc);
    INFO("analytic clean delta " << delta_clean << ", hermite " << hermite << ", trunc " << trunc);
    REQUIRE(hermite < 1.25 * delta_clean); // smooth doppler, no steps
    REQUIRE(trunc > 1.5 * delta_clean);    // the legacy whole-sample zipper, preserved on purpose
}
