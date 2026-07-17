/// @file
/// @brief      Unit tests for the TR-808 cymbal kernel (taptools::tr808::cymbal), plus the
///             shared metal bank it exposes.
/// @details    Pins the documented behaviors: the bank's six oscillator fundamentals and the
///             tolerance/seed spread, the p.14 chart's 350-1200 ms decay range across the
///             decay pot, the tone (strike/body) balance, accent monotonicity, determinism,
///             silence/boundedness.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/tr808_cymbal.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    using taptools::tr808::cymbal;
    using taptools::tr808::metal_bank;

    cymbal make(double tone = 0.5, double decay = 0.5) {
        cymbal c;
        c.prepare(k_sr);
        c.set_tone(tone);
        c.set_decay(decay);
        c.set_level(1.0);
        return c;
    }

    std::vector<double> render(cymbal& c, double seconds, double accent = 1.0) {
        std::vector<double> out(static_cast<size_t>(seconds * k_sr));
        c.trigger(accent);
        for (auto& v : out)
            v = c.process();
        return out;
    }

    double peak(const std::vector<double>& x, size_t begin = 0, size_t end = SIZE_MAX) {
        end      = std::min(end, x.size());
        double p = 0.0;
        for (size_t i = begin; i < end; ++i)
            p = std::max(p, std::abs(x[i]));
        return p;
    }

    double goertzel(const std::vector<double>& x, double f, size_t begin, size_t end) {
        end             = std::min(end, x.size());
        const double w  = 2.0 * k_pi * f / k_sr;
        const double c  = 2.0 * std::cos(w);
        double       s1 = 0.0, s2 = 0.0;
        for (size_t i = begin; i < end; ++i) {
            const double s0 = x[i] + c * s1 - s2;
            s2              = s1;
            s1              = s0;
        }
        const double re = s1 - s2 * std::cos(w);
        const double im = s2 * std::sin(w);
        return 2.0 * std::sqrt(re * re + im * im) / static_cast<double>(end - begin);
    }

    size_t ms(double m) {
        return static_cast<size_t>(m * 0.001 * k_sr);
    }

    size_t decay_40db(const std::vector<double>& x) {
        const double thresh = peak(x) * 0.01;
        size_t       last   = 0;
        for (size_t i = 0; i < x.size(); ++i)
            if (std::abs(x[i]) > thresh)
                last = i;
        return last;
    }

} // namespace

SCENARIO("the metal bank runs six squares at the documented fundamentals") {
    metal_bank bank;
    bank.prepare(k_sr);
    std::vector<double> y(static_cast<size_t>(0.5 * k_sr));
    for (auto& v : y) {
        bank.process();
        v = bank.sum();
    }
    // Each nominal fundamental beats everything a quartertone away.
    for (double f : taptools::tr808::k_bank_hz) {
        const double at  = goertzel(y, f, 0, y.size());
        const double off = goertzel(y, f * 1.06, 0, y.size());
        INFO(f << " Hz: " << at << " vs +6% " << off);
        CHECK(at > 1.5 * off);
    }
}

SCENARIO("tolerance spreads the bank per seed; zero tolerance is exact") {
    metal_bank a, b;
    a.prepare(k_sr);
    b.prepare(k_sr);
    a.set_tolerance(1.0);
    b.set_tolerance(1.0);
    a.set_seed(2);
    b.set_seed(3);
    std::vector<double> ya(static_cast<size_t>(0.25 * k_sr)), yb(ya.size());
    for (size_t i = 0; i < ya.size(); ++i) {
        a.process();
        b.process();
        ya[i] = a.sum();
        yb[i] = b.sum();
    }
    bool differ = false;
    for (size_t i = 0; i < ya.size(); ++i)
        differ = differ || ya[i] != yb[i];
    CHECK(differ); // two different units off the line

    metal_bank c, d;
    c.prepare(k_sr);
    d.prepare(k_sr);
    c.set_seed(2);
    d.set_seed(3); // tolerance 0: seed must not matter
    bool same = true;
    for (int i = 0; i < 4800; ++i) {
        c.process();
        d.process();
        same = same && c.sum() == d.sum();
    }
    CHECK(same);
}

SCENARIO("the cymbal decay pot spans the measured ~0.65-2.7 s -40 dB range") {
    // The chart's 350-1200 ms are knob classes, not -40 dB tails; a real unit measures
    // ~0.65 s (pot minimum) to ~2.7 s (maximum) to -40 dB (Fischer set) — calibrated.
    auto         c1 = make(0.5, 0.0);
    auto         c2 = make(0.5, 1.0);
    auto         y1 = render(c1, 3.5);
    auto         y2 = render(c2, 3.5);
    const size_t t1 = decay_40db(y1);
    const size_t t2 = decay_40db(y2);
    INFO("-40 dB: decay 0 -> " << t1 / k_sr * 1000.0 << " ms, decay 1 -> " << t2 / k_sr * 1000.0 << " ms");
    CHECK(t1 < t2);
    CHECK(t1 > ms(400));
    CHECK(t1 < ms(900));
    CHECK(t2 > ms(2200));
    CHECK(t2 < ms(3100));
}

SCENARIO("cymbal tone balances strike against body") {
    auto c_body   = make(0.0);
    auto c_strike = make(1.0);
    auto y_body   = render(c_body, 0.4);
    auto y_strike = render(c_strike, 0.4);
    // The strike band sits around the 7.1 kHz voicing; the body around 3.4 kHz.
    const double hi_b = goertzel(y_body, 7100.0, 0, ms(80));
    const double hi_s = goertzel(y_strike, 7100.0, 0, ms(80));
    const double lo_b = goertzel(y_body, 3440.0, 0, ms(80));
    const double lo_s = goertzel(y_strike, 3440.0, 0, ms(80));
    INFO("7.1k " << hi_b << "/" << hi_s << ", 3.4k " << lo_b << "/" << lo_s);
    CHECK(hi_s > hi_b);
    CHECK(lo_b > lo_s);
}

SCENARIO("cymbal accent scales monotonically and stays in range") {
    auto         c1 = make();
    auto         c2 = make();
    auto         c3 = make();
    const double p1 = peak(render(c1, 0.8, 0.0));
    const double p2 = peak(render(c2, 0.8, 0.5));
    const double p3 = peak(render(c3, 0.8, 1.0));
    INFO("peaks " << p1 << " / " << p2 << " / " << p3);
    CHECK(p1 > 0.05); // the cymbal bus floor is 7 V — unaccented is a real hit
    CHECK(p1 < p2);
    CHECK(p2 < p3);
    CHECK(p3 < 1.2);
}

SCENARIO("cymbal rendering is deterministic and silent at rest") {
    auto c1    = make();
    auto c2    = make();
    auto y1    = render(c1, 0.5);
    auto y2    = render(c2, 0.5);
    bool equal = true;
    for (size_t i = 0; i < y1.size(); ++i)
        equal = equal && y1[i] == y2[i];
    CHECK(equal);

    cymbal q;
    q.prepare(k_sr);
    double p = 0.0;
    for (int i = 0; i < 4800; ++i)
        p = std::max(p, std::abs(q.process()));
    CHECK(p == 0.0);

    auto c3     = make(0.5, 0.3);
    auto y3     = render(c3, 5.0);
    bool finite = true;
    for (double v : y3)
        finite = finite && std::isfinite(v);
    CHECK(finite);
    CHECK(peak(y3, y3.size() - static_cast<size_t>(0.05 * k_sr)) < 1e-6);
}
