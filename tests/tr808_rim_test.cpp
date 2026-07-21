/// @file
/// @brief      Unit tests for the TR-808 rimshot/claves kernel (tap::tools::tr808::rim).
/// @details    Pins the p.14 chart: the rimshot's ~1667 + ~455 Hz pair in the ~10 ms class
///             (with the VCA's harmonic generation), the claves' pure ~2500 Hz ~25 ms tick,
///             accent, determinism, silence.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/tr808_rim.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    using tap::tools::tr808::rim;

    rim make(int model = rim::model_rimshot) {
        rim r;
        r.prepare(k_sr);
        r.set_model(model);
        r.set_level(1.0);
        return r;
    }

    std::vector<double> render(rim& r, double seconds, double accent = 1.0) {
        std::vector<double> out(static_cast<size_t>(seconds * k_sr));
        r.trigger(accent);
        for (auto& v : out)
            v = r.process();
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

SCENARIO("the rimshot cracks on the ~1667 + ~455 Hz pair, ~10 ms class") {
    auto         r   = make();
    auto         y   = render(r, 0.2);
    const double hi  = goertzel(y, 1667.0, 0, ms(12));
    const double lo  = goertzel(y, 455.0, 0, ms(12));
    const double off = goertzel(y, 900.0, 0, ms(12));
    INFO("1667: " << hi << ", 455: " << lo << ", 900: " << off);
    CHECK(hi > 1.5 * off);
    CHECK(lo > 1.5 * off);

    const size_t t = decay_40db(y);
    INFO("-40 dB at " << t / k_sr * 1000.0 << " ms");
    CHECK(t > ms(3));
    CHECK(t < ms(40));
}

SCENARIO("the claves tick purely at ~2500 Hz, ~25 ms class") {
    auto         r   = make(rim::model_claves);
    auto         y   = render(r, 0.2);
    const double at  = goertzel(y, 2500.0, 0, ms(25));
    const double off = goertzel(y, 1800.0, 0, ms(25));
    INFO("2500: " << at << ", 1800: " << off);
    CHECK(at > 2.0 * off);

    const size_t t = decay_40db(y);
    INFO("-40 dB at " << t / k_sr * 1000.0 << " ms");
    CHECK(t > ms(8));
    CHECK(t < ms(80));
}

SCENARIO("the rimshot's VCA adds harmonics the claves path doesn't") {
    auto r = make();
    auto y = render(r, 0.1);
    // Harmonic energy at 2x the low resonator, beyond both fundamentals.
    const double h2 = goertzel(y, 910.0, 0, ms(10));
    INFO("910 Hz (2x455) " << h2);
    CHECK(h2 > 0.0005);
}

SCENARIO("rim accent scales monotonically; deterministic; silent at rest") {
    auto         r1 = make();
    auto         r2 = make();
    auto         r3 = make();
    const double p1 = peak(render(r1, 0.1, 0.0));
    const double p2 = peak(render(r2, 0.1, 0.5));
    const double p3 = peak(render(r3, 0.1, 1.0));
    INFO("peaks " << p1 << " / " << p2 << " / " << p3);
    CHECK(p1 > 0.05);
    CHECK(p1 < p2);
    CHECK(p2 < p3);
    CHECK(p3 < 1.2);

    auto a     = make();
    auto b     = make();
    auto ya    = render(a, 0.2);
    auto yb    = render(b, 0.2);
    bool equal = true;
    for (size_t i = 0; i < ya.size(); ++i)
        equal = equal && ya[i] == yb[i];
    CHECK(equal);

    rim q;
    q.prepare(k_sr);
    double silent = 0.0;
    for (int i = 0; i < 4800; ++i)
        silent = std::max(silent, std::abs(q.process()));
    CHECK(silent == 0.0);

    auto r4     = make(rim::model_claves);
    auto y4     = render(r4, 0.6);
    bool finite = true;
    for (double v : y4)
        finite = finite && std::isfinite(v);
    CHECK(finite);
    CHECK(peak(y4, y4.size() - static_cast<size_t>(0.05 * k_sr)) < 1e-6);
}

SCENARIO("rim family balance: both switch positions peak in the family band") {
    // Slice-5 polish pin: the claves trim (k_cl_mix) brings the hotter CL network into
    // the rimshot's neighborhood, as the hardware's summing does.
    auto         rs  = make();
    auto         cl  = make(rim::model_claves);
    const double prs = peak(render(rs, 0.4));
    const double pcl = peak(render(cl, 0.4));
    INFO("rimshot " << prs << " claves " << pcl);
    CHECK(prs > 0.2);
    CHECK(prs < 1.0);
    CHECK(pcl > 0.2);
    CHECK(pcl < 1.0);
    CHECK(pcl < 3.0 * prs);
    CHECK(prs < 3.0 * pcl);
}
