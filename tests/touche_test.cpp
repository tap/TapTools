/// @file
/// @brief      Catch2 scenarios pinning the tap.touche~ kernel (touche.h).
/// @details    This kernel's contract is unusual for the library: the curve is not a design
///             choice to be measured after the fact, it is a *published measurement* the code
///             is obliged to reproduce. So the load-bearing scenario simply checks that every
///             one of Quartier et al.'s seven points comes back out of the object, and the rest
///             pin the properties that make the interpolation trustworthy — monotone, no
///             overshoot between points, exact endpoints, and demonstrably not the straight
///             line a lazier implementation would have fitted.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/touche.h>

namespace {

    constexpr double k_sr = 48000.0;

    using tap::tools::touche::key;
    namespace T = tap::tools::touche;

    key make() {
        key k;
        k.prepare(k_sr);
        k.set_smooth_ms(0.0);
        return k;
    }

    /// Normalized position of a displacement in millimetres — over the physical travel, which
    /// is wider than the measured band (the bottom of the throw is silent).
    double pos_of_mm(double mm) {
        return mm / T::k_travel_mm;
    }

} // namespace

// The whole point of the object: it reproduces the published table. Every measured nuance
// boundary must come back at the dB the paper reports, referenced to full press.
SCENARIO("the published measurement comes back out of the curve") {
    key k = make();

    for (int i = 0; i < T::k_points; ++i) {
        const double mm      = T::k_table_mm[static_cast<size_t>(i)];
        const double want_db = T::k_table_db[static_cast<size_t>(i)] - T::k_table_db[T::k_points - 1];
        const double got_db  = k.db_at(pos_of_mm(mm));
        INFO("point " << i << ": " << mm << " mm, published " << T::k_table_db[static_cast<size_t>(i)] << " dB_SPL -> "
                      << want_db << " dB relative; got " << got_db);
        CHECK(std::abs(got_db - want_db) < 0.05); // dense-table resolution, not curve error
    }
}

SCENARIO("the range is exactly the published 50 dB over the published travel") {
    key k = make();
    INFO("full press " << k.db_at(1.0) << " dB, floor " << k.db_at(pos_of_mm(T::k_min_mm)) << " dB");
    CHECK(std::abs(k.db_at(1.0)) < 1e-9); // full press is exactly unity
    CHECK(std::abs(k.gain_at(1.0) - 1.0) < 1e-12);
    CHECK(std::abs(k.db_at(pos_of_mm(T::k_min_mm)) + T::k_range_db) < 0.05); // the floor is -50 dB
    CHECK(std::abs(T::k_max_mm - T::k_min_mm - 4.5) < 1e-9);                 // over 4.5 mm of travel
}

// The dead zone is the instrument's, not a modelling artifact: the key bends before it reaches
// the powder bag, so the bottom of the throw makes no sound at all.
SCENARIO("the bottom of the throw is silent, and that is the key's own first phase") {
    key k = make();
    for (double mm : {0.0, 1.0, 2.5, 4.0}) {
        INFO(mm << " mm");
        CHECK(k.gain_at(pos_of_mm(mm)) == 0.0);
    }
    CHECK(k.gain_at(pos_of_mm(4.4)) > 0.0); // and it opens just past the measured floor
    INFO("silent fraction of the throw: " << T::k_min_mm / T::k_travel_mm);
    CHECK(T::k_min_mm / T::k_travel_mm > 0.4); // roughly the first 45 %
}

SCENARIO("the curve is monotone and never overshoots between measured points") {
    key k = make();

    double last   = -1.0;
    bool   mono   = true;
    bool   inside = true;
    for (int i = 0; i <= 4000; ++i) {
        const double p = static_cast<double>(i) / 4000.0;
        const double g = k.gain_at(p);
        mono           = mono && (g >= last - 1e-12);
        inside         = inside && (g >= 0.0) && (g <= 1.0 + 1e-12);
        last           = g;
    }
    REQUIRE(mono);   // pressing harder is never quieter
    REQUIRE(inside); // and the interpolant stays inside the measured envelope
}

// The reason the kernel interpolates rather than fits. Equal dB steps in the published table
// correspond to displacement steps of 1.0, 0.6, 0.5, 0.4, 0.5, 1.5 mm — the curve steepens
// through the middle and flattens at the top, and a straight line in dB-against-mm would be
// visibly and audibly wrong.
SCENARIO("the law is emphatically not a straight line in dB against displacement") {
    key k = make();

    // Compare across the *measured* band only — the silent dead zone below it is not part of
    // the curve and would swamp the comparison.
    const double lo    = pos_of_mm(T::k_min_mm);
    const double hi    = pos_of_mm(T::k_max_mm);
    double       worst = 0.0;
    for (int i = 1; i < 100; ++i) {
        const double f      = static_cast<double>(i) / 100.0;
        const double p      = lo + f * (hi - lo);
        const double linear = -T::k_range_db * (1.0 - f);
        worst               = std::max(worst, std::abs(k.db_at(p) - linear));
    }
    INFO("largest departure from a straight line: " << worst << " dB");
    REQUIRE(worst > 6.0); // nowhere near a line — this is the expressiveness

    // And the departure has the measured shape: halfway along the measured band is louder than
    // a line would predict, because the steep part of the curve happens early.
    const double mid = k.db_at(lo + 0.5 * (hi - lo));
    INFO("at half the measured band: " << mid << " dB, a line would say " << -T::k_range_db * 0.5);
    CHECK(mid > -T::k_range_db * 0.5);
}

SCENARIO("below the measured floor is exact silence, not extrapolation") {
    key k = make();
    REQUIRE(k.gain_at(0.0) == 0.0);

    // And through the audio path: a key at rest passes nothing at all.
    k.set_position(0.0);
    bool silent = true;
    for (int i = 0; i < 2000; ++i) {
        silent = silent && (k.process(0.7) == 0.0);
    }
    REQUIRE(silent);
}

SCENARIO("above full press the curve clamps rather than extrapolating") {
    key k = make();
    CHECK(k.gain_at(1.0) == k.gain_at(2.0));
    k.set_position_mm(20.0); // far past the 9.5 mm travel
    CHECK(k.position() == 1.0);
}

SCENARIO("the force column reproduces its own published points") {
    key k = make();
    k.set_mode(T::mode_force);

    for (int i = 0; i < T::k_points; ++i) {
        const double n       = T::k_table_n[static_cast<size_t>(i)];
        const double p       = n / T::k_travel_n;
        const double want_db = T::k_table_db[static_cast<size_t>(i)] - T::k_table_db[T::k_points - 1];
        INFO("point " << i << ": " << n << " N, want " << want_db << " dB, got " << k.db_at(p));
        CHECK(std::abs(k.db_at(p) - want_db) < 0.2); // coarser: the force axis is far from uniform
    }
}

// The paper's finding that the map does not depend on the speed of the gesture is why a static
// curve is legitimate at all. The kernel-side consequence a test can pin: the gain depends only
// on where the key is, not on what the audio is doing or how it got there.
SCENARIO("the gain depends only on position, not on the signal or the approach") {
    key fast = make();
    key slow = make();

    // Same destination, reached instantly vs over a long ramp — once settled, identical.
    fast.set_position(0.62);
    slow.set_smooth_ms(200.0);
    slow.set_position(0.62);
    for (int i = 0; i < static_cast<int>(0.5 * k_sr); ++i) {
        fast.process(0.0);
        slow.process(0.0);
    }
    INFO("fast " << fast.gain() << ", slow " << slow.gain());
    CHECK(std::abs(fast.gain() - slow.gain()) < 1e-12);

    // And it is a pure gain: the ratio out/in is the same whatever the input.
    key k = make();
    k.set_position(0.4);
    k.process(0.0); // settle
    const double g = k.gain();
    for (double x : {0.01, 0.5, -0.3, 1.0}) {
        INFO("input " << x);
        CHECK(std::abs(k.process(x) - x * g) < 1e-12);
    }
}

SCENARIO("a position move is slewed, so the key does not click") {
    key k = make();
    k.set_smooth_ms(50.0);
    k.set_position(0.0);
    k.process(1.0);

    k.set_position(1.0); // slam it open
    double worst = 0.0, last = k.process(1.0);
    for (int i = 0; i < static_cast<int>(0.1 * k_sr); ++i) {
        const double y = k.process(1.0);
        worst          = std::max(worst, std::abs(y - last));
        last           = y;
    }
    INFO("largest single-sample step on a full-travel move: " << worst);
    REQUIRE(worst < 0.01); // no discontinuity anywhere in the sweep
}

SCENARIO("the signal-rate path tracks a control signal") {
    key k = make();

    // Sweep the position as a signal and check the output follows the curve sample by sample.
    bool exact = true;
    for (int i = 0; i <= 100; ++i) {
        const double p = static_cast<double>(i) / 100.0;
        const double y = k.process(1.0, p);
        exact          = exact && (std::abs(y - k.gain_at(p)) < 1e-12);
    }
    REQUIRE(exact);
}

SCENARIO("unprepared, the key still follows its curve") {
    key k;
    k.set_smooth_ms(0.0);
    k.set_position(1.0);
    CHECK(std::abs(k.process(0.5) - 0.5) < 1e-9);
}
