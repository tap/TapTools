/// @file
/// @brief      Catch2 scenarios pinning the tap.stammer~ kernel (stammer.h).
/// @details    The suite leans on a configuration that is fully deterministic *regardless of the
///             seed* — density 1, whole-step slices, one repeat, no reverse, no jump, no fade —
///             because in that corner the machine must reduce to an exact one-step delay, and
///             that single identity pins the grid timing, the origin arithmetic, and the playback
///             head together, bitwise. Around it: the seeded-performance contract (same seed,
///             same render; different seeds, different renders; density 0 never touches the rng
///             at all — the garden.h idle contract), the repeat invariant, the reverse identity,
///             and the envelope's exact edges.
///
///             Material contract, per the header: the musical scenarios drive plucks, not sines.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/stammer.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    using tap::tools::stammer::machine;

    size_t at(double seconds) {
        return static_cast<size_t>(seconds * k_sr);
    }

    /// A machine with everything the tests care about pinned; scenarios opt into the dice.
    machine make(double max_history_ms = 2000.0) {
        machine m;
        m.prepare(k_sr, max_history_ms);
        m.set_smooth_ms(0.0);
        m.set_input_level(1.0);
        m.set_mix(100.0);
        m.set_fade_ms(0.0); // bitwise comparisons want the raw material
        m.set_jump_ms(0.0);
        m.set_reverse(0.0);
        m.set_divisions(1);
        m.set_repeats(1);
        m.set_density(1.0);
        return m;
    }

    /// The documented material: a train of decaying plucks, not a sine. Transients are what this
    /// object is for, and what makes a wrong slice boundary audible in a render.
    std::vector<double> pluck_train(size_t n, double period_s = 0.31) {
        std::vector<double> x(n, 0.0);
        for (size_t i = 0; i < n; ++i) {
            const double t   = static_cast<double>(i) / k_sr;
            const double phi = std::fmod(t, period_s);
            const double hz  = 196.0;
            double       sum = 0.0;
            for (int k = 1; k <= 5; ++k) {
                const double kk = static_cast<double>(k);
                sum += (1.0 / kk) * std::exp(-phi * (4.0 + 3.0 * kk)) * std::sin(2.0 * k_pi * hz * kk * phi);
            }
            x[i] = 0.5 * sum;
        }
        return x;
    }

    std::vector<double> render(machine& m, const std::vector<double>& x) {
        std::vector<double> y(x.size(), 0.0);
        for (size_t i = 0; i < x.size(); ++i) {
            y[i] = m.process(x[i]);
        }
        return y;
    }

} // namespace

// The load-bearing identity. With the dice pinned to a single outcome — always fire, whole-step
// slices, one pass, forwards, no reach-back, no flank — every grid point grabs exactly the step
// that just went past and plays it once. That is a pure delay of one step less one sample, and it
// must hold sample for sample. Grid timing, origin arithmetic and the playback head all fail this
// test if any of them is off by one.
SCENARIO("with the dice pinned, the machine is exactly a one-step delay") {
    const double step_ms = 100.0;
    const size_t step    = static_cast<size_t>(step_ms * 0.001 * k_sr);

    machine m = make();
    m.set_step_ms(step_ms);

    const std::vector<double> x = pluck_train(at(1.5));
    const std::vector<double> y = render(m, x);

    bool exact = true;
    for (size_t i = step; i < y.size(); ++i) {
        exact = exact && (y[i] == x[i - step + 1]); // bitwise, not approximately
    }
    REQUIRE(exact);

    // Not vacuous: the material is live, and the delay really did move it.
    REQUIRE(std::any_of(y.begin() + static_cast<long>(step), y.end(), [](double v) { return std::abs(v) > 0.01; }));
    REQUIRE(!std::equal(x.begin() + static_cast<long>(step), x.end(), y.begin() + static_cast<long>(step)));
}

SCENARIO("a reversed slice plays the same step backwards") {
    const double step_ms = 100.0;
    const size_t step    = static_cast<size_t>(step_ms * 0.001 * k_sr);

    machine m = make();
    m.set_step_ms(step_ms);
    m.set_reverse(1.0); // every repeat runs backwards

    const std::vector<double> x = pluck_train(at(1.0));
    const std::vector<double> y = render(m, x);

    // Forwards the grab at grid point k reads x[k*step + 1 + j]; backwards it reads the same
    // window end-first.
    bool exact = true;
    for (size_t k = 1; k * step + step <= y.size(); ++k) {
        for (size_t j = 0; j < step; ++j) {
            exact = exact && (y[k * step + j] == x[k * step - j]);
        }
    }
    REQUIRE(exact);
}

// Repeats are the only thing that keeps the machine busy past one pass, and a slice in flight is
// never interrupted. So every output sample is either a fresh grab (the one-step delay above) or
// a bit-exact copy of the block one slice-length earlier. Nothing else is legal.
SCENARIO("every sample is either a fresh grab or an exact repeat of the block before it") {
    const double step_ms = 80.0;
    const size_t step    = static_cast<size_t>(step_ms * 0.001 * k_sr);

    machine m = make();
    m.set_step_ms(step_ms);
    m.set_repeats(4); // draws 1..4 passes per fired slice
    m.set_seed(20260815);

    const std::vector<double> x = pluck_train(at(2.0));
    const std::vector<double> y = render(m, x);

    size_t fresh = 0, repeated = 0;
    bool   legal = true;
    for (size_t i = 2 * step; i < y.size(); ++i) {
        const bool is_fresh  = (y[i] == x[i - step + 1]);
        const bool is_repeat = (y[i] == y[i - step]);
        if (is_fresh) {
            ++fresh;
        }
        else if (is_repeat) {
            ++repeated;
        }
        else {
            legal = false;
        }
    }
    INFO("fresh grabs " << fresh << ", repeated samples " << repeated);
    REQUIRE(legal);
    REQUIRE(fresh > 0);    // it does grab new material
    REQUIRE(repeated > 0); // and it does hold on: repeats are reaching the output
}

SCENARIO("a seed is a performance you can replay") {
    auto run = [](uint64_t seed) {
        machine m = make();
        m.set_step_ms(70.0);
        m.set_density(0.6);
        m.set_divisions(4);
        m.set_repeats(6);
        m.set_reverse(0.4);
        m.set_jump_ms(120.0);
        m.set_fade_ms(3.0);
        m.set_seed(seed);
        return render(m, pluck_train(at(3.0)));
    };

    const std::vector<double> a = run(12345);
    const std::vector<double> b = run(12345);
    const std::vector<double> c = run(999);

    bool same = true;
    for (size_t i = 0; i < a.size(); ++i) {
        same = same && (a[i] == b[i]); // bitwise: the dice are deterministic
    }
    REQUIRE(same);

    // And a different seed is a different performance, not a cosmetic reshuffle.
    size_t differing = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != c[i]) {
            ++differing;
        }
    }
    INFO("samples differing between seeds: " << differing << " of " << a.size());
    REQUIRE(differing > a.size() / 20);
}

// The garden.h idle contract, same shape: a disabled generator must not consume its stream, so
// the seed provably cannot matter. Here that also means the object is a bitwise bypass.
SCENARIO("at density 0 the dice are never rolled, so the seed cannot matter") {
    auto run = [](uint64_t seed) {
        machine m = make();
        m.set_step_ms(70.0);
        m.set_density(0.0);
        m.set_seed(seed);
        return render(m, pluck_train(at(1.0)));
    };

    const std::vector<double> a = run(1);
    const std::vector<double> b = run(0xfeedface);
    const std::vector<double> x = pluck_train(at(1.0));

    bool identical = true, passthrough = true;
    for (size_t i = 0; i < a.size(); ++i) {
        identical   = identical && (a[i] == b[i]);
        passthrough = passthrough && (a[i] == x[i]);
    }
    REQUIRE(identical);
    REQUIRE(passthrough); // and idle is a bitwise bypass
}

SCENARIO("an idle machine passes the input through bitwise at any mix") {
    machine m = make();
    m.set_step_ms(70.0);
    m.set_density(0.0);
    m.set_mix(50.0); // mid-mix: there is nothing to blend against, so it must not change the level

    const std::vector<double> x = pluck_train(at(0.5));
    const std::vector<double> y = render(m, x);

    bool exact = true;
    for (size_t i = 0; i < y.size(); ++i) {
        exact = exact && (y[i] == x[i]);
    }
    REQUIRE(exact);
}

SCENARIO("the per-repeat flanks reach exactly zero at both edges") {
    const double step_ms = 50.0;
    const size_t step    = static_cast<size_t>(step_ms * 0.001 * k_sr);
    const double fade_ms = 2.0;
    const size_t fade    = static_cast<size_t>(fade_ms * 0.001 * k_sr);

    machine m = make();
    m.set_step_ms(step_ms);
    m.set_fade_ms(fade_ms);

    // A held DC input makes the slice material exactly 1.0, so the output IS the envelope. (A
    // mechanism check, not a musical one — the material contract is about what the object is for.)
    std::vector<double>       x(at(0.5), 1.0);
    const std::vector<double> y = render(m, x);

    // The second grid point onward grabs fully-recorded material, so the envelope stands alone.
    const size_t base = 2 * step;
    INFO("slice length " << step << ", flank " << fade);
    REQUIRE(y[base] == 0.0);            // opens from silence
    REQUIRE(y[base + step - 1] == 0.0); // and closes to it
    REQUIRE(y[base + fade] == 1.0);     // the plateau is unity, exactly
    REQUIRE(y[base + step / 2] == 1.0);

    // Monotonic rise across the flank: no ripple, no overshoot.
    bool rising = true;
    for (size_t i = 1; i <= fade; ++i) {
        rising = rising && (y[base + i] >= y[base + i - 1]);
    }
    REQUIRE(rising);
}

SCENARIO("clear drops the slice in flight and restarts the seeded stream") {
    machine m = make();
    m.set_step_ms(60.0);
    m.set_density(0.7);
    m.set_repeats(6);
    m.set_seed(4242);

    const std::vector<double> x = pluck_train(at(1.0));
    const std::vector<double> a = render(m, x);

    m.clear();
    const std::vector<double> b = render(m, x);

    bool same = true;
    for (size_t i = 0; i < a.size(); ++i) {
        same = same && (a[i] == b[i]); // clear rewinds the performance exactly
    }
    REQUIRE(same);
}

SCENARIO("unprepared, the stammer passes input through") {
    machine m;
    REQUIRE(m.process(0.7) == 0.7);
    REQUIRE(m.process(-0.3) == -0.3);
}
