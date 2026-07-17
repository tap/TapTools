/// @file
/// @brief      Unit tests for the TR-808 cowbell kernel (taptools::tr808::cowbell).
/// @details    Pins the documented behaviors: the 540/800 Hz oscillator pair (the chart's own
///             values), the ~50 ms decay class with the two-slope "abrupt initial decay"
///             envelope, accent, determinism, silence.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/tr808_cowbell.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    using taptools::tr808::cowbell;

    cowbell make() {
        cowbell c;
        c.prepare(k_sr);
        c.set_level(1.0);
        return c;
    }

    std::vector<double> render(cowbell& c, double seconds, double accent = 1.0) {
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

SCENARIO("the cowbell clanks on the 540/800 Hz trimmed pair") {
    auto         c     = make();
    auto         y     = render(c, 0.3);
    const double at540 = goertzel(y, 540.0, 0, ms(40));
    const double at800 = goertzel(y, 800.0, 0, ms(40));
    const double off   = goertzel(y, 670.0, 0, ms(40)); // between the two
    INFO("540: " << at540 << ", 800: " << at800 << ", 670: " << off);
    CHECK(at540 > 2.0 * off);
    CHECK(at800 > 2.0 * off);
}

SCENARIO("the cowbell decays in the chart's ~50 ms class, attack-heavy") {
    auto         c = make();
    auto         y = render(c, 0.5);
    const size_t t = decay_40db(y);
    INFO("-40 dB at " << t / k_sr * 1000.0 << " ms");
    CHECK(t > ms(25));
    CHECK(t < ms(150));

    // The two-slope envelope: the first 10 ms carry far more than the 20-30 ms window.
    const double head = peak(y, 0, ms(10));
    const double tail = peak(y, ms(20), ms(30));
    INFO("0-10 ms " << head << " vs 20-30 ms " << tail);
    CHECK(head > 2.0 * tail);
    CHECK(tail > 0.01); // ...but the tail is still there (C34's slower slope)
}

SCENARIO("cowbell accent scales monotonically and stays in range") {
    auto         c1 = make();
    auto         c2 = make();
    auto         c3 = make();
    const double p1 = peak(render(c1, 0.3, 0.0));
    const double p2 = peak(render(c2, 0.3, 0.5));
    const double p3 = peak(render(c3, 0.3, 1.0));
    INFO("peaks " << p1 << " / " << p2 << " / " << p3);
    CHECK(p1 > 0.05);
    CHECK(p1 < p2);
    CHECK(p2 < p3);
    CHECK(p3 < 1.2);
}

SCENARIO("cowbell rendering is deterministic, silent at rest, and decays to silence") {
    auto c1    = make();
    auto c2    = make();
    auto y1    = render(c1, 0.3);
    auto y2    = render(c2, 0.3);
    bool equal = true;
    for (size_t i = 0; i < y1.size(); ++i)
        equal = equal && y1[i] == y2[i];
    CHECK(equal);

    cowbell q;
    q.prepare(k_sr);
    double p = 0.0;
    for (int i = 0; i < 4800; ++i)
        p = std::max(p, std::abs(q.process()));
    CHECK(p == 0.0);

    auto c3     = make();
    auto y3     = render(c3, 1.0);
    bool finite = true;
    for (double v : y3)
        finite = finite && std::isfinite(v);
    CHECK(finite);
    CHECK(peak(y3, y3.size() - static_cast<size_t>(0.05 * k_sr)) < 1e-6);
}
