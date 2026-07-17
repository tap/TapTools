/// @file
/// @brief      Unit tests for the TR-808 tom/conga kernel (taptools::tr808::tom).
/// @details    Pins the p.14 chart tunings and decay classes per size/model, the tuning knob's
///             low..high sweep, the D80/D81 attack pitch fall, the tom-only noise layer,
///             accent, determinism, silence.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/tr808_tom.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    using taptools::tr808::tom;

    tom make(int size = 0, int model = tom::model_tom, double tuning = 0.5) {
        tom t;
        t.prepare(k_sr);
        t.set_size(size);
        t.set_model(model);
        t.set_tuning(tuning);
        t.set_level(1.0);
        return t;
    }

    std::vector<double> render(tom& t, double seconds, double accent = 1.0) {
        std::vector<double> out(static_cast<size_t>(seconds * k_sr));
        t.trigger(accent);
        for (auto& v : out)
            v = t.process();
        return out;
    }

    double peak(const std::vector<double>& x, size_t begin = 0, size_t end = SIZE_MAX) {
        end      = std::min(end, x.size());
        double p = 0.0;
        for (size_t i = begin; i < end; ++i)
            p = std::max(p, std::abs(x[i]));
        return p;
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

    /// Zero-crossing frequency over [begin, end).
    double zc_frequency(const std::vector<double>& x, size_t begin, size_t end) {
        end = std::min(end, x.size());
        std::vector<double> crossings;
        for (size_t i = begin + 1; i < end; ++i) {
            if (x[i - 1] < 0.0 && x[i] >= 0.0) {
                crossings.push_back(static_cast<double>(i - 1) - x[i - 1] / (x[i] - x[i - 1]));
            }
        }
        if (crossings.size() < 2)
            return 0.0;
        return k_sr * static_cast<double>(crossings.size() - 1) / (crossings.back() - crossings.front());
    }

    double spectral_peak(const std::vector<double>& x, double lo, double hi, size_t begin, size_t end);

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

    /// Frequency of the strongest bin in [lo, hi] (2 Hz steps) — robust against the tom's
    /// own noise layer, which pollutes zero-crossing counts.
    double spectral_peak(const std::vector<double>& x, double lo, double hi, size_t begin, size_t end) {
        double best = 0.0, best_f = lo;
        for (double f = lo; f <= hi; f += 2.0) {
            const double m = goertzel(x, f, begin, end);
            if (m > best) {
                best   = m;
                best_f = f;
            }
        }
        return best_f;
    }

} // namespace

SCENARIO("tom and conga tunings land in the chart's spans per size") {
    // Tom mids: ~90 / ~135 / ~185; conga mids: ~185 / ~280 / ~400 (p.14, knob at noon).
    const double tom_lo[3] = {80, 120, 165}, tom_hi[3] = {100, 160, 220};
    const double cga_lo[3] = {165, 250, 370}, cga_hi[3] = {220, 310, 455};
    for (int size = 0; size < 3; ++size) {
        auto   t = make(size, tom::model_tom);
        auto   y = render(t, 0.4);
        double f = spectral_peak(y, tom_lo[size] * 0.7, tom_hi[size] * 1.4, ms(30), ms(150));
        INFO("tom size " << size << ": " << f << " Hz");
        CHECK(f > tom_lo[size] * 0.9);
        CHECK(f < tom_hi[size] * 1.1);

        auto   c  = make(size, tom::model_conga);
        auto   yc = render(c, 0.4);
        double fc = zc_frequency(yc, ms(30), ms(90));
        INFO("conga size " << size << ": " << fc << " Hz");
        CHECK(fc > cga_lo[size] * 0.9);
        CHECK(fc < cga_hi[size] * 1.1);
    }
}

SCENARIO("the tuning knob sweeps the chart's low..high span") {
    auto         t_lo = make(1, tom::model_tom, 0.0);
    auto         t_hi = make(1, tom::model_tom, 1.0);
    auto         y_lo = render(t_lo, 0.4);
    auto         y_hi = render(t_hi, 0.4);
    const double f_lo = spectral_peak(y_lo, 90.0, 200.0, ms(30), ms(150));
    const double f_hi = spectral_peak(y_hi, 90.0, 200.0, ms(30), ms(150));
    INFO("MT knob 0 -> " << f_lo << " Hz, knob 1 -> " << f_hi << " Hz");
    CHECK(std::abs(f_lo - 120.0) < 15.0);
    CHECK(std::abs(f_hi - 160.0) < 18.0);
}

SCENARIO("the attack rings sharp and falls as it damps (D80/D81)") {
    auto         t       = make(0, tom::model_conga); // congas: pure ring, cleanest to measure
    auto         y       = render(t, 0.5);
    const double f_early = zc_frequency(y, ms(3), ms(35));
    const double f_late  = zc_frequency(y, ms(80), ms(160));
    INFO("early " << f_early << " Hz vs late " << f_late << " Hz");
    CHECK(f_early > f_late * 1.02);
}

SCENARIO("toms carry the noise layer; congas do not") {
    // The direct pin: the seeded noise source is audible in a tom (different seeds render
    // differently) and absent from a conga (seed cannot matter). Spectral probes can't
    // separate the noise from the diode bend's own harmonics, so ask the seed instead.
    auto t1 = make(0, tom::model_tom);
    auto t2 = make(0, tom::model_tom);
    t1.set_seed(2);
    t2.set_seed(3);
    auto yt1         = render(t1, 0.3);
    auto yt2         = render(t2, 0.3);
    bool toms_differ = false;
    for (size_t i = 0; i < yt1.size(); ++i)
        toms_differ = toms_differ || yt1[i] != yt2[i];
    CHECK(toms_differ);

    auto c1 = make(0, tom::model_conga);
    auto c2 = make(0, tom::model_conga);
    c1.set_seed(2);
    c2.set_seed(3);
    auto yc1         = render(c1, 0.3);
    auto yc2         = render(c2, 0.3);
    bool congas_same = true;
    for (size_t i = 0; i < yc1.size(); ++i)
        congas_same = congas_same && yc1[i] == yc2[i];
    CHECK(congas_same);
}

SCENARIO("decay classes: low rings longer than high; congas shorter than toms") {
    auto         t_lo = make(0, tom::model_tom);
    auto         t_hi = make(2, tom::model_tom);
    auto         c_lo = make(0, tom::model_conga);
    auto         y_tl = render(t_lo, 1.0);
    auto         y_th = render(t_hi, 1.0);
    auto         y_cl = render(c_lo, 1.0);
    const size_t d_tl = decay_40db(y_tl);
    const size_t d_th = decay_40db(y_th);
    const size_t d_cl = decay_40db(y_cl);
    INFO("LT " << d_tl / k_sr * 1e3 << " ms, HT " << d_th / k_sr * 1e3 << " ms, LC " << d_cl / k_sr * 1e3 << " ms");
    CHECK(d_tl > d_th);
    CHECK(d_tl > ms(100));
    CHECK(d_tl < ms(450));
    CHECK(d_cl < d_tl);
}

SCENARIO("tom accent scales monotonically; deterministic; silent at rest") {
    auto         t1 = make();
    auto         t2 = make();
    auto         t3 = make();
    const double p1 = peak(render(t1, 0.4, 0.0));
    const double p2 = peak(render(t2, 0.4, 0.5));
    const double p3 = peak(render(t3, 0.4, 1.0));
    INFO("peaks " << p1 << " / " << p2 << " / " << p3);
    CHECK(p1 > 0.05);
    CHECK(p1 < p2);
    CHECK(p2 < p3);
    CHECK(p3 < 1.2);

    auto a = make();
    auto b = make();
    a.set_seed(7);
    b.set_seed(7);
    auto ya    = render(a, 0.3);
    auto yb    = render(b, 0.3);
    bool equal = true;
    for (size_t i = 0; i < ya.size(); ++i)
        equal = equal && ya[i] == yb[i];
    CHECK(equal);

    tom q;
    q.prepare(k_sr);
    double silent = 0.0;
    for (int i = 0; i < 4800; ++i)
        silent = std::max(silent, std::abs(q.process()));
    CHECK(silent == 0.0);

    auto t4     = make();
    auto y4     = render(t4, 1.5);
    bool finite = true;
    for (double v : y4)
        finite = finite && std::isfinite(v);
    CHECK(finite);
    CHECK(peak(y4, y4.size() - static_cast<size_t>(0.05 * k_sr)) < 1e-6);
}

SCENARIO("tom family balance: every channel's full-accent peak sits in the family band") {
    // Slice-5 polish pin: the per-channel summing gains (k_tomc_mix) keep all six
    // size/model channels in a consistent band at any knob position, like the hardware's
    // per-channel summing resistors into the mix bus.
    for (int model : {tom::model_tom, tom::model_conga})
        for (int size : {0, 1, 2})
            for (double tuning : {0.0, 0.5, 1.0}) {
                auto         t = make(size, model, tuning);
                const double p = peak(render(t, 0.8));
                INFO("model " << model << " size " << size << " tuning " << tuning << " peak " << p);
                CHECK(p > 0.35);
                CHECK(p < 1.0);
            }
}
