/// @file
/// @brief      Catch2 scenarios pinning the tap.discreet~ kernel (discreet.h + tape_loop.h).
/// @details    Oracle-based where the promise is musical: pitch claims (wow depth, the tape-speed
///             doppler of a loop-time change) are measured with the DspTap YIN detector, spectral
///             claims (per-pass darkening) with a local Goertzel against the analytically
///             predicted per-pass transfer, and the headline stability claim — regeneration at
///             exactly 1.0 stays bounded because the wear path is the stabilizer — with the
///             two-window RMS pattern from grm_comb_test.cpp.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <tap/dsp/yin.h>
#include <taptools/discreet.h>

namespace {

    constexpr double k_sr = 48000.0;

    using tap::tools::discreet::machine;

    /// A machine with the transport parked and instant setters: tests opt into wow explicitly.
    machine make(double max_loop_seconds = 4.0) {
        machine m;
        m.prepare(k_sr, max_loop_seconds);
        m.set_smooth_ms(0.0);
        m.set_wow(0.0, 0.0);
        m.set_flutter(0.0, 0.0);
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

    double mean(const std::vector<double>& x, size_t begin, size_t end) {
        double acc = 0.0;
        for (size_t i = begin; i < end; ++i) {
            acc += x[i];
        }
        return acc / static_cast<double>(end - begin);
    }

    double peak(const std::vector<double>& x, size_t begin, size_t end) {
        double p = 0.0;
        for (size_t i = begin; i < end; ++i) {
            p = std::max(p, std::abs(x[i]));
        }
        return p;
    }

    /// Single-bin magnitude, 2|X(f)|/N — same probe as the tr808 tests.
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

    /// YIN oracle at an offset — same detector setup as tune_test.cpp / harmonizer_test.cpp.
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

    /// Predicted per-pass magnitude of the wear path at drive 0: the exact one-pole lowpass times
    /// the normalized DC blocker, evaluated on the unit circle — the same formulas the kernel
    /// applies, computed independently here.
    double wear_gain(double f, double cutoff_hz) {
        const double w   = 2.0 * 3.14159265358979323846 * f / k_sr;
        const double a   = 1.0 - std::exp(-2.0 * 3.14159265358979323846 * cutoff_hz / k_sr);
        const auto   ejw = std::exp(std::complex<double>(0.0, -w));
        const double lp  = std::abs(a / (1.0 - (1.0 - a) * ejw));
        const double r   = tap::tools::tape::k_dc_block_r;
        const double nm  = tap::tools::tape::k_dc_block_norm;
        const double dc  = std::abs(nm * (1.0 - ejw) / (1.0 - r * ejw));
        return lp * dc;
    }

    /// Deterministic noise, never denormal-small — same LCG as delay_test.cpp.
    struct noise {
        uint32_t state{2463534242u};
        double   operator()() {
            state = state * 1664525u + 1013904223u;
            return (static_cast<double>(state) / 2147483648.0) - 1.0;
        }
    };

} // namespace

SCENARIO("the loop echoes at exactly the loop period") {
    machine m = make();
    m.set_loop_seconds(0.5);
    m.set_regen(0.7);
    m.set_drive(0.0);
    m.set_darken_hz(20000.0);

    const size_t        loop = at(0.5);
    std::vector<double> y(at(1.8), 0.0);
    for (size_t i = 0; i < y.size(); ++i) {
        y[i] = m.process(i == 0 ? 1.0 : 0.0);
    }

    // The first return is the recorded impulse itself, bit-exact at one loop (integer span,
    // Hermite frac 0 reads x0 exactly; wear touches only the return path, not the first read).
    REQUIRE(y[loop] == 1.0);

    // Later returns have passed the wear filter — smeared, but the peak stays on the grid.
    for (size_t k = 2; k <= 3; ++k) {
        const size_t lo    = k * loop - 16;
        const size_t hi    = k * loop + 16;
        size_t       argmx = lo;
        for (size_t i = lo; i < hi; ++i) {
            if (std::abs(y[i]) > std::abs(y[argmx])) {
                argmx = i;
            }
        }
        INFO("echo " << k << " peak at " << argmx << ", grid " << k * loop);
        CHECK(argmx >= k * loop - 1);
        CHECK(argmx <= k * loop + 1);
    }
}

// Regeneration at exactly 1.0 is a legal, sustaining regime: boundedness comes from the wear
// path (bounded saturator + darkening + DC blocker), not from a feedback cap. Ten seconds of
// ring covers ~36 passes of a 0.25 s loop — long enough that the +0.2 dB/s class of swell the
// comb bank once had (grm_comb_test.cpp) would show clearly.
SCENARIO("regen 1.0 with drive engaged is bounded and does not grow") {
    machine m = make();
    m.set_loop_seconds(0.25);
    m.set_regen(1.0);
    m.set_drive(0.5);
    m.set_darken_hz(3000.0);

    noise               rng;
    std::vector<double> y(at(10.0), 0.0);
    for (size_t i = 0; i < y.size(); ++i) {
        const double in = (i < at(1.0)) ? 0.5 * rng() : 0.0;
        y[i]            = m.process(in);
    }

    const double early = rms(y, at(2.0), at(5.0));
    const double late  = rms(y, at(6.0), at(9.0));
    INFO("ring RMS: [2,5)s = " << early << ", [6,9)s = " << late);
    REQUIRE(std::isfinite(late));
    REQUIRE(late <= early * 1.02);       // sustain is the contract: no growth, decay not required
    REQUIRE(peak(y, 0, y.size()) < 3.0); // |wear out| <= 1/drive = 2, plus the direct send
}

SCENARIO("every pass through the loop is darker by the wear filter") {
    machine m = make();
    m.set_loop_seconds(0.25);
    m.set_regen(0.9);
    m.set_drive(0.0); // linear wear: the per-pass ratio is exactly regen * |H_wear|
    m.set_darken_hz(2000.0);

    // A two-tone burst, one tone well above the darkening corner and one well below, so the
    // test can assert both sides: highs die fast, lows barely fade (the honest tape story).
    const double        f_hi = 6000.0;
    const double        f_lo = 300.0;
    const size_t        loop = at(0.25);
    std::vector<double> y(at(1.5), 0.0);
    for (size_t i = 0; i < y.size(); ++i) {
        double in = 0.0;
        if (i < at(0.1)) {
            const double t = static_cast<double>(i) / k_sr;
            in             = 0.4 * std::sin(2.0 * 3.14159265358979323846 * f_hi * t)
                 + 0.4 * std::sin(2.0 * 3.14159265358979323846 * f_lo * t);
        }
        y[i] = m.process(in);
    }

    const double expect_hi = 0.9 * wear_gain(f_hi, 2000.0);
    const double expect_lo = 0.9 * wear_gain(f_lo, 2000.0);
    for (size_t k = 1; k <= 3; ++k) {
        const double hi_a     = goertzel(y, f_hi, k * loop, k * loop + at(0.1));
        const double hi_b     = goertzel(y, f_hi, (k + 1) * loop, (k + 1) * loop + at(0.1));
        const double lo_a     = goertzel(y, f_lo, k * loop, k * loop + at(0.1));
        const double lo_b     = goertzel(y, f_lo, (k + 1) * loop, (k + 1) * loop + at(0.1));
        const double hi_ratio = hi_b / hi_a;
        const double lo_ratio = lo_b / lo_a;
        INFO("pass " << k << " -> " << k + 1 << ": hi ratio " << hi_ratio << " (predicted " << expect_hi
                     << "), lo ratio " << lo_ratio << " (predicted " << expect_lo << ")");
        CHECK(std::abs(hi_ratio - expect_hi) < 0.15 * expect_hi);
        CHECK(std::abs(lo_ratio - expect_lo) < 0.05 * expect_lo);
        CHECK(hi_ratio < lo_ratio); // both sides: the wear is a tilt, not a fader
    }
}

SCENARIO("a dc step does not accumulate, even at regen 1.0") {
    machine m = make();
    m.set_loop_seconds(0.25);
    m.set_regen(1.0);
    m.set_drive(0.0);
    m.set_darken_hz(3000.0);

    // Without the in-loop DC blocker, a held 0.5 input at regen 1.0 would add 0.5 every pass,
    // without bound. With it, the running mean stays put and the tail's mean returns to zero.
    std::vector<double> y(at(4.0), 0.0);
    for (size_t i = 0; i < y.size(); ++i) {
        y[i] = m.process(i < at(2.0) ? 0.5 : 0.0);
    }

    const double driven = peak(y, 0, at(2.0));
    const double tail   = mean(y, at(3.5), at(4.0)); // 2 whole loops: an unbiased DC estimate
    INFO("driven peak " << driven << ", tail mean " << tail);
    REQUIRE(driven < 3.0);
    REQUIRE(std::abs(tail) < 0.02);
}

SCENARIO("wow bends pitch by the set depth, and two runs are bit-exact") {
    const double depth_ms = 2.0;
    const double rate_hz  = 0.5;
    // Peak deviation of a sinusoidally modulated read: ratio swings by depth * 2*pi*rate.
    const double predicted = 1200.0 / std::log(2.0) * depth_ms * 0.001 * 2.0 * 3.14159265358979323846 * rate_hz;

    auto render = [&] {
        machine m = make();
        m.set_loop_seconds(1.0);
        m.set_regen(0.0);
        m.set_wow(depth_ms, rate_hz);
        std::vector<double> y(at(4.5), 0.0);
        for (size_t i = 0; i < y.size(); ++i) {
            const double t = static_cast<double>(i) / k_sr;
            y[i]           = m.process(0.8 * std::sin(2.0 * 3.14159265358979323846 * 440.0 * t));
        }
        return y;
    };

    const std::vector<double> y = render();

    // Track the wet pitch across one full wow cycle (2 s), after the tape has filled.
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

SCENARIO("mix endpoints are bitwise exact") {
    machine m = make();
    m.set_loop_seconds(0.5);
    m.set_regen(0.5);

    m.set_mix(0.0);
    noise rng;
    bool  exact = true;
    for (int i = 0; i < 4800; ++i) {
        const double in = rng();
        exact           = exact && (m.process(in) == in); // bitwise, not approximately
    }
    REQUIRE(exact);
}

SCENARIO("a loop-time change glides as tape speed, not a splice") {
    machine m = make();
    m.set_loop_seconds(0.5);
    m.set_regen(0.0);

    std::vector<double> y;
    y.reserve(at(4.5));
    auto run = [&](double seconds) {
        for (size_t i = 0; i < at(seconds); ++i) {
            const double t = static_cast<double>(y.size()) / k_sr;
            y.push_back(m.process(0.8 * std::sin(2.0 * 3.14159265358979323846 * 440.0 * t)));
        }
    };

    run(2.0); // fill the tape at the short span
    m.set_smooth_ms(500.0);
    m.set_loop_seconds(0.75); // respool +0.25 s of span over 0.5 s: tape speed halves
    run(0.5);
    m.set_smooth_ms(0.0);
    run(2.0);

    // Mid-glide the playback sits an octave down; after the ramp lands it re-locks to pitch.
    const double gliding = measure_hz(y, at(2.2));
    const double settled = measure_hz(y, at(3.5));
    INFO("mid-glide " << gliding << " Hz, settled " << settled << " Hz");
    CHECK(std::abs(cents(gliding, 220.0)) < 60.0);
    CHECK(std::abs(cents(settled, 440.0)) < 5.0);

    // And it is a glide: no splice discontinuity anywhere in the move.
    double worst_step = 0.0;
    for (size_t i = at(2.0) + 1; i < at(2.5); ++i) {
        worst_step = std::max(worst_step, std::abs(y[i] - y[i - 1]));
    }
    INFO("largest sample step during the glide: " << worst_step);
    CHECK(worst_step < 0.1); // a 440 Hz sine at 0.8 moves ~0.046/sample; a splice would jump ~1.6
}

SCENARIO("unprepared, the machine passes input through") {
    machine m;
    REQUIRE(m.process(0.7) == 0.7);
    REQUIRE(m.process(-0.3) == -0.3);
}
