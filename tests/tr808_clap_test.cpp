/// @file
/// @brief      Unit tests for the TR-808 handclap/maracas kernel (taptools::tr808::clap).
/// @details    Pins the Service Notes documentation: the clap's three sawtooth teeth ~10 ms
///             apart inside the 30 ms window (Figure 13), the reverberation tail and its
///             disconnection bend, the ~2 kHz band-pass voicing, the ~100 ms decay class;
///             the maracas' short bright burst (25-35 ms chart class); accent, seeded
///             determinism, silence/boundedness.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/tr808_clap.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    using taptools::tr808::clap;

    clap make(int model = clap::model_clap) {
        clap c;
        c.prepare(k_sr);
        c.set_model(model);
        c.set_level(1.0);
        return c;
    }

    std::vector<double> render(clap& c, double seconds, double accent = 1.0) {
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

    /// Rectified-peak envelope in 1 ms hops.
    std::vector<double> envelope_1ms(const std::vector<double>& x, size_t upto) {
        std::vector<double> env;
        for (size_t i = 0; i + ms(1) <= std::min(upto, x.size()); i += ms(1))
            env.push_back(peak(x, i, i + ms(1)));
        return env;
    }

} // namespace

SCENARIO("the clap fires three teeth ~10 ms apart (Figure 13)") {
    auto c = make();
    c.set_tail(0.0); // teeth only, per the bend
    auto y   = render(c, 0.2);
    auto env = envelope_1ms(y, ms(35));
    // Count local rises: a new tooth abruptly lifts the 1 ms envelope after it has fallen.
    int rises = 0;
    for (size_t i = 1; i < env.size(); ++i)
        if (env[i] > env[i - 1] * 1.8 && env[i] > 0.05)
            ++rises;
    INFO("abrupt envelope rises in the first 35 ms: " << rises);
    CHECK(rises >= 2); // teeth 2 and 3 (tooth 1 starts the note)
    CHECK(rises <= 3);

    // And with the tail disconnected the note is over well before 100 ms.
    CHECK(decay_40db(y) < ms(70));
}

SCENARIO("the reverberation tail carries the clap past the teeth (Q70 path)") {
    auto c_tail = make();
    auto y_tail = render(c_tail, 0.6);
    auto c_dry  = make();
    c_dry.set_tail(0.0);
    auto y_dry = render(c_dry, 0.6);

    const double wash_tail = peak(y_tail, ms(50), ms(90));
    const double wash_dry  = peak(y_dry, ms(50), ms(90));
    INFO("50-90 ms wash: tail " << wash_tail << " vs disconnected " << wash_dry);
    CHECK(wash_tail > 5.0 * wash_dry);

    // Overall decay: the chart says ~100 ms, but a real unit's tail measures ~375 ms
    // to -40 dB (Fischer set, unit 103852) — the calibrated class.
    const size_t t = decay_40db(y_tail);
    INFO("-40 dB at " << t / k_sr * 1000.0 << " ms");
    CHECK(t > ms(250));
    CHECK(t < ms(550));
}

SCENARIO("the clap is voiced by the schematic's ~2 kHz band-pass") {
    auto         c   = make();
    auto         y   = render(c, 0.2);
    const double mid = goertzel(y, 2000.0, 0, ms(30));
    const double lo  = goertzel(y, 300.0, 0, ms(30));
    const double hi  = goertzel(y, 10000.0, 0, ms(30));
    INFO("300/2000/10000 Hz: " << lo << " / " << mid << " / " << hi);
    CHECK(mid > 2.0 * lo);
    CHECK(mid > 2.0 * hi);
}

SCENARIO("maracas is a short bright burst") {
    auto c = make(clap::model_maracas);
    auto y = render(c, 0.3);

    // Chart class: 25-35 ms to silence.
    const size_t t = decay_40db(y);
    INFO("-40 dB at " << t / k_sr * 1000.0 << " ms");
    CHECK(t > ms(10));
    CHECK(t < ms(80));

    // Brighter than the clap: energy sits above the clap's band-pass.
    const double hi = goertzel(y, 8000.0, 0, ms(20));
    const double lo = goertzel(y, 1000.0, 0, ms(20));
    INFO("8 kHz " << hi << " vs 1 kHz " << lo);
    CHECK(hi > 2.0 * lo);

    // Single burst: no tooth-like re-rises after the onset.
    auto env   = envelope_1ms(y, ms(30));
    int  rises = 0;
    for (size_t i = 2; i < env.size(); ++i)
        if (env[i] > env[i - 1] * 1.8 && env[i] > 0.05)
            ++rises;
    CHECK(rises == 0);
}

SCENARIO("accent scales both models monotonically") {
    for (int model : {clap::model_clap, clap::model_maracas}) {
        auto         c1 = make(model);
        auto         c2 = make(model);
        auto         c3 = make(model);
        const double p1 = peak(render(c1, 0.3, 0.0));
        const double p2 = peak(render(c2, 0.3, 0.5));
        const double p3 = peak(render(c3, 0.3, 1.0));
        INFO("model " << model << " peaks " << p1 << " / " << p2 << " / " << p3);
        CHECK(p1 > 0.05);
        CHECK(p1 < p2);
        CHECK(p2 < p3);
        CHECK(p3 < 1.2);
    }
}

SCENARIO("clap rendering is deterministic for a fixed seed") {
    auto c1 = make();
    auto c2 = make();
    c1.set_seed(7);
    c2.set_seed(7);
    auto y1    = render(c1, 0.3);
    auto y2    = render(c2, 0.3);
    bool equal = true;
    for (size_t i = 0; i < y1.size(); ++i)
        equal = equal && y1[i] == y2[i];
    CHECK(equal);
}

SCENARIO("the channel is silent at rest and decays to true silence") {
    auto                c = make();
    std::vector<double> quiet(static_cast<size_t>(0.1 * k_sr));
    for (auto& v : quiet)
        v = c.process();
    CHECK(peak(quiet) == 0.0);

    auto y      = render(c, 1.5);
    bool finite = true;
    for (double v : y)
        finite = finite && std::isfinite(v);
    CHECK(finite);
    CHECK(peak(y, y.size() - static_cast<size_t>(0.05 * k_sr)) < 1e-6);
}
