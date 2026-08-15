/// @file
/// @brief      Catch2 scenarios pinning the tap.tapecho~ kernel (tapecho.h over tape_loop.h).
/// @details    Two house patterns carry most of the suite. The NULL TEST is the load-bearing one:
///             with the tape path neutralized (no wow, no wear, no regeneration) a one-head echo
///             must be *bitwise* the plain Hermite multitap of delay.h — that is what makes
///             "tape_loop.h is a library, and this kernel is only composition" a measurement
///             rather than a claim. The pitch promises (wow depth) are ORACLE-BASED, measured out
///             of the output with the DspTap YIN detector, as in discreet_test.cpp; the
///             self-oscillation bound uses the two-window RMS pattern from grm_comb_test.cpp.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <tap/dsp/yin.h>
#include <taptools/delay.h>
#include <taptools/tapecho.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    using tap::tools::tapecho::machine;

    /// A machine with the transport parked, the wear path neutral, and instant setters: tests opt
    /// into wow, drive, and regeneration explicitly.
    machine make(double max_span_seconds = 2.0) {
        machine m;
        m.prepare(k_sr, max_span_seconds);
        m.set_smooth_ms(0.0);
        m.set_wow(0.0, 0.0);
        m.set_flutter(0.0, 0.0);
        m.set_regen(0.0);
        m.set_drive(0.0);
        m.set_darken_hz(20000.0);
        m.set_input_level(1.0);
        m.set_mix(100.0); // wet only: the tape is what these tests measure
        return m;
    }

    size_t at(double seconds) {
        return static_cast<size_t>(seconds * k_sr);
    }

    double rms(const std::vector<double>& x, size_t begin, size_t end) {
        double acc = 0.0;
        for (size_t i = begin; i < end; ++i) {
            acc += x[i] * x[i];
        }
        return std::sqrt(acc / static_cast<double>(end - begin));
    }

    double peak(const std::vector<double>& x, size_t begin, size_t end) {
        double p = 0.0;
        for (size_t i = begin; i < end; ++i) {
            p = std::max(p, std::abs(x[i]));
        }
        return p;
    }

    /// YIN oracle at an offset — same detector setup as discreet_test.cpp / tune_test.cpp.
    double measure_hz(const std::vector<double>& x, size_t offset) {
        const size_t  tau_min = static_cast<size_t>(k_sr / 2000.0);
        const size_t  tau_max = static_cast<size_t>(std::ceil(k_sr / 55.0));
        tap::dsp::yin det(tau_max, tau_min, tau_max);
        REQUIRE(x.size() >= offset + det.frame_size());
        const auto r = det.analyze(x.data() + offset);
        REQUIRE(r.voiced());
        return k_sr / r.period;
    }

    double cents(double f, double ref) {
        return 1200.0 * std::log2(f / ref);
    }

    /// Deterministic noise, never denormal-small — same LCG as delay_test.cpp.
    struct noise {
        uint32_t state{2463534242u};
        double   operator()() {
            state = state * 1664525u + 1013904223u;
            return (static_cast<double>(state) / 2147483648.0) - 1.0;
        }
    };

    /// Render an impulse through a machine and return the left bus.
    std::vector<double> impulse_response(machine& m, double seconds) {
        std::vector<double> y(at(seconds), 0.0);
        for (size_t i = 0; i < y.size(); ++i) {
            double r = 0.0;
            m.process(i == 0 ? 1.0 : 0.0, y[i], r);
        }
        return y;
    }

} // namespace

SCENARIO("each head echoes at its own position along the tape path") {
    machine m = make();
    m.set_span_ms(100.0);
    m.set_heads(4); // the default even spacing: 0.25, 0.5, 0.75, 1.0 of the span

    const std::vector<double> y = impulse_response(m, 0.15);

    // A head at ratio r returns the impulse at exactly r * span. Integer spans read Hermite frac 0,
    // so the returned sample is the recorded one scaled only by level and the centre pan law.
    const double centre = std::cos(0.25 * k_pi); // pan 0, the multitap equal-power law
    for (int i = 0; i < 4; ++i) {
        const double ratio = static_cast<double>(i + 1) / 4.0;
        const size_t k     = at(0.1 * ratio);
        INFO("head " << i << " at ratio " << ratio << ", expected return at sample " << k);
        CHECK(std::abs(y[k] - centre) < 1e-12);
        CHECK(std::abs(y[k - 1]) < 1e-12); // and nowhere else: the neighbours are silent
        CHECK(std::abs(y[k + 1]) < 1e-12);
    }
}

// The load-bearing test of the whole kernel: neutralize the tape (no transport error, no
// regeneration) and the echo must be the plain Hermite delay of delay.h, sample for sample.
// Not vacuous — a 1e-12 nudge on the span, the level, or the pan breaks it (checked during
// development), and it is bitwise because both paths are the same Hermite read at the same
// fractional position under the same equal-power pan law.
SCENARIO("with the tape path neutral, a one-head echo is bitwise the multitap of delay.h") {
    const double span_ms = 137.31; // deliberately not a whole number of samples

    machine m = make(1.0);
    m.set_heads(1);
    m.set_head_ratio(0, 1.0);
    m.set_head_level(0, 1.0);
    m.set_head_pan(0, 0.0);
    m.set_span_ms(span_ms);

    tap::tools::delay::multitap ref;
    ref.prepare(k_sr, 1000.0);
    ref.set_smooth_ms(0.0);
    ref.set_taps(1);
    ref.set_time_ms(0, span_ms);
    ref.set_gain(0, 1.0);
    ref.set_pan(0, 0.0);

    noise rng;
    bool  exact = true;
    for (size_t i = 0; i < at(0.5); ++i) {
        const double in = rng();
        double       ml = 0.0, mr = 0.0, rl = 0.0, rr = 0.0;
        m.process(in, ml, mr);
        ref.process(in, rl, rr);
        exact = exact && (ml == rl) && (mr == rr); // bitwise, not approximately
    }
    REQUIRE(exact);
}

SCENARIO("the motor moves every head together") {
    // Doubling the span doubles every head's return time — one motor, one tape path.
    machine a = make();
    a.set_heads(4);
    a.set_span_ms(100.0);
    const std::vector<double> y_short = impulse_response(a, 0.25);

    machine b = make();
    b.set_heads(4);
    b.set_span_ms(200.0);
    const std::vector<double> y_long = impulse_response(b, 0.25);

    for (int i = 0; i < 4; ++i) {
        const double ratio = static_cast<double>(i + 1) / 4.0;
        INFO("head " << i << " at ratio " << ratio);
        CHECK(std::abs(y_short[at(0.1 * ratio)]) > 0.5);
        CHECK(std::abs(y_long[at(0.2 * ratio)]) > 0.5);
    }

    // And the heads really moved: the doubled span vacates the two short-span positions it does
    // not also occupy (25 and 75 ms; 50 and 100 ms are shared by both layouts).
    CHECK(std::abs(y_long[at(0.025)]) < 1e-12);
    CHECK(std::abs(y_long[at(0.075)]) < 1e-12);
}

// The design statement of the kernel: regeneration past unity is legal, self-oscillates, and
// stays bounded because the saturator — not a feedback cap — is the stabilizer. The analytic
// bound on the tape is |in|max + regen * L1(wear) / drive, with the DC blocker's L1 gain of
// ~2 on top of the saturator's 1/drive ceiling: 0.5 + 1.4 * 2 / 0.6 = ~5.2. The measured peak
// sits far below that (real signals do not excite the L1 worst case); the ceiling asserted here
// is the measured value with margin, and the promise that matters is the non-growth.
SCENARIO("regeneration past unity self-oscillates but stays bounded") {
    machine m = make();
    m.set_heads(1);
    m.set_head_ratio(0, 1.0);
    m.set_span_ms(250.0);
    m.set_regen(1.4);
    m.set_drive(0.6);
    m.set_darken_hz(6000.0);

    noise               rng;
    std::vector<double> y(at(12.0), 0.0);
    for (size_t i = 0; i < y.size(); ++i) {
        const double in = (i < at(0.5)) ? 0.5 * rng() : 0.0;
        double       r  = 0.0;
        m.process(in, y[i], r);
    }

    const double early = rms(y, at(6.0), at(9.0));
    const double late  = rms(y, at(9.0), at(12.0));
    INFO("oscillation RMS: [6,9)s = " << early << ", [9,12)s = " << late);
    REQUIRE(std::isfinite(late));
    REQUIRE(late > 0.01);          // it really is oscillating, not decaying away
    REQUIRE(late <= early * 1.02); // and it has plateaued: bounded, not runaway
    REQUIRE(peak(y, 0, y.size()) < 3.0);
}

SCENARIO("at drive 0 the regeneration cap falls back to unity") {
    // Same excessive regen target, no saturator. The per-sample cap holds the effective value at
    // 1.0, where the linear wear path (|H| <= 1) sustains without growing.
    machine m = make();
    m.set_heads(1);
    m.set_head_ratio(0, 1.0);
    m.set_span_ms(250.0);
    m.set_regen(1.4);
    m.set_drive(0.0);
    m.set_darken_hz(6000.0);

    noise               rng;
    std::vector<double> y(at(12.0), 0.0);
    for (size_t i = 0; i < y.size(); ++i) {
        const double in = (i < at(0.5)) ? 0.5 * rng() : 0.0;
        double       r  = 0.0;
        m.process(in, y[i], r);
    }

    const double early = rms(y, at(6.0), at(9.0));
    const double late  = rms(y, at(9.0), at(12.0));
    INFO("capped RMS: [6,9)s = " << early << ", [9,12)s = " << late);
    REQUIRE(std::isfinite(late));
    REQUIRE(late <= early * 1.02); // no growth, though the target says 1.4
    REQUIRE(peak(y, 0, y.size()) < 2.0);
    REQUIRE(m.regen() == 1.4); // the target is kept: it takes effect again when drive returns
}

SCENARIO("the echo transport bends pitch by the set wow depth, and two runs are bit-exact") {
    const double depth_ms = 2.0;
    const double rate_hz  = 0.5;
    // Peak deviation of a sinusoidally modulated read: ratio swings by depth * 2*pi*rate.
    const double predicted = 1200.0 / std::log(2.0) * depth_ms * 0.001 * 2.0 * k_pi * rate_hz;

    auto render = [&] {
        machine m = make();
        m.set_heads(1);
        m.set_head_ratio(0, 1.0);
        m.set_span_ms(1000.0);
        m.set_wow(depth_ms, rate_hz);
        std::vector<double> y(at(4.5), 0.0);
        for (size_t i = 0; i < y.size(); ++i) {
            const double t = static_cast<double>(i) / k_sr;
            double       r = 0.0;
            m.process(0.8 * std::sin(2.0 * k_pi * 440.0 * t), y[i], r);
        }
        return y;
    };

    const std::vector<double> y = render();

    double worst = 0.0;
    for (size_t off = at(1.5); off + at(0.05) < at(3.5); off += 2048) {
        worst = std::max(worst, std::abs(cents(measure_hz(y, off), 440.0)));
    }
    INFO("peak deviation " << worst << " cents, predicted " << predicted);
    CHECK(worst > 0.6 * predicted);
    CHECK(worst < 1.4 * predicted);

    const std::vector<double> z     = render();
    bool                      exact = true;
    for (size_t i = 0; i < y.size(); ++i) {
        exact = exact && (y[i] == z[i]); // bitwise: the transport is deterministic
    }
    REQUIRE(exact);
}

SCENARIO("a span change glides as tape speed, not a splice") {
    machine m = make();
    m.set_heads(1);
    m.set_head_ratio(0, 1.0);
    m.set_span_ms(500.0);

    std::vector<double> y;
    y.reserve(at(4.5));
    auto run = [&](double seconds) {
        for (size_t i = 0; i < at(seconds); ++i) {
            const double t = static_cast<double>(y.size()) / k_sr;
            double       l = 0.0, r = 0.0;
            m.process(0.8 * std::sin(2.0 * k_pi * 440.0 * t), l, r);
            y.push_back(l);
        }
    };

    run(2.0); // fill the tape at the short span
    m.set_smooth_ms(500.0);
    m.set_span_ms(750.0); // respool +0.25 s of span over 0.5 s: tape speed halves
    run(0.5);
    m.set_smooth_ms(0.0);
    run(2.0);

    const double gliding = measure_hz(y, at(2.2));
    const double settled = measure_hz(y, at(3.5));
    INFO("mid-glide " << gliding << " Hz, settled " << settled << " Hz");
    CHECK(std::abs(cents(gliding, 220.0)) < 60.0);
    CHECK(std::abs(cents(settled, 440.0)) < 5.0);

    double worst_step = 0.0;
    for (size_t i = at(2.0) + 1; i < at(2.5); ++i) {
        worst_step = std::max(worst_step, std::abs(y[i] - y[i - 1]));
    }
    INFO("largest sample step during the glide: " << worst_step);
    CHECK(worst_step < 0.1); // a 440 Hz sine at 0.8 moves ~0.046/sample; a splice would jump ~1.6
}

SCENARIO("a hard-panned head is bitwise absent from the far bus") {
    machine m = make();
    m.set_heads(2);
    m.set_head_ratio(0, 0.5);
    m.set_head_pan(0, -1.0); // hard left
    m.set_head_ratio(1, 1.0);
    m.set_head_pan(1, 1.0); // hard right

    noise  rng;
    bool   left_clean = true, right_clean = true;
    double left_energy = 0.0, right_energy = 0.0;
    for (size_t i = 0; i < at(0.5); ++i) {
        double l = 0.0, r = 0.0;
        m.process(i < 16 ? rng() : 0.0, l, r);
        // Head 0 returns only on the left at 0.5 span, head 1 only on the right at 1.0 span.
        const size_t half = at(0.375 * 0.5);
        const size_t full = at(0.375);
        if (i >= half && i < half + 16) {
            right_clean = right_clean && (r == 0.0);
            left_energy += l * l;
        }
        if (i >= full && i < full + 16) {
            left_clean = left_clean && (l == 0.0);
            right_energy += r * r;
        }
    }
    REQUIRE(left_energy > 0.0); // the heads really did return
    REQUIRE(right_energy > 0.0);
    REQUIRE(right_clean); // and the far bus is bitwise zero, not just small
    REQUIRE(left_clean);
}

SCENARIO("mix endpoints are bitwise exact on both busses") {
    machine m = make();
    m.set_heads(4);
    m.set_span_ms(200.0);
    m.set_regen(0.5);

    m.set_mix(0.0);
    noise rng;
    bool  exact = true;
    for (int i = 0; i < 4800; ++i) {
        const double in = rng();
        double       l = 0.0, r = 0.0;
        m.process(in, l, r);
        exact = exact && (l == in) && (r == in); // bitwise dry, both busses
    }
    REQUIRE(exact);
}

SCENARIO("no heads is silence, and the tape still turns underneath") {
    machine m = make();
    m.set_span_ms(200.0);
    m.set_heads(0);

    noise rng;
    bool  silent = true;
    for (size_t i = 0; i < at(0.5); ++i) {
        double l = 0.0, r = 0.0;
        m.process(rng(), l, r);
        silent = silent && (l == 0.0) && (r == 0.0);
    }
    REQUIRE(silent);

    // Bring a head back and the tape it was recording all along is there, immediately.
    m.set_heads(1);
    m.set_head_ratio(0, 1.0);
    double energy = 0.0;
    for (size_t i = 0; i < at(0.25); ++i) {
        double l = 0.0, r = 0.0;
        m.process(0.0, l, r);
        energy += l * l;
    }
    REQUIRE(energy > 0.0);
}

SCENARIO("unprepared, the echo passes input through") {
    machine m;
    double  l = 0.0, r = 0.0;
    m.process(0.7, l, r);
    REQUIRE(l == 0.7);
    REQUIRE(r == 0.7);
    m.process(-0.3, l, r);
    REQUIRE(l == -0.3);
    REQUIRE(r == -0.3);
}
