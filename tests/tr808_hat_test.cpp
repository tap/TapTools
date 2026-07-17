/// @file
/// @brief      Unit tests for the TR-808 hi-hat kernel (taptools::tr808::hat).
/// @details    Pins the documented behaviors: the closed hat's fixed ~50 ms class, the open
///             hat's 90-600 ms decay-pot range, the hardware choke (a CH trigger terminates a
///             sounding OH), per-path levels, accent, determinism, silence.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/tr808_hat.h>

namespace {

    constexpr double k_sr = 48000.0;

    using taptools::tr808::hat;

    hat make(double decay = 0.5) {
        hat h;
        h.prepare(k_sr);
        h.set_decay(decay);
        return h;
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

} // namespace

SCENARIO("the closed hat is a fixed short tick") {
    auto h = make();
    h.trigger_closed(1.0);
    std::vector<double> y(static_cast<size_t>(0.4 * k_sr));
    for (auto& v : y)
        v = h.process();
    const size_t t = decay_40db(y);
    INFO("-40 dB at " << t / k_sr * 1000.0 << " ms");
    CHECK(t > ms(15));
    CHECK(t < ms(120));
}

SCENARIO("the open hat's decay pot spans the chart's 90-600 ms classes") {
    auto h1 = make(0.0);
    auto h2 = make(1.0);
    h1.trigger_open(1.0);
    h2.trigger_open(1.0);
    std::vector<double> y1(static_cast<size_t>(1.5 * k_sr)), y2(y1.size());
    for (size_t i = 0; i < y1.size(); ++i) {
        y1[i] = h1.process();
        y2[i] = h2.process();
    }
    const size_t t1 = decay_40db(y1);
    const size_t t2 = decay_40db(y2);
    INFO("-40 dB: decay 0 -> " << t1 / k_sr * 1000.0 << " ms, decay 1 -> " << t2 / k_sr * 1000.0 << " ms");
    CHECK(t1 < t2);
    CHECK(t1 > ms(40));
    CHECK(t1 < ms(250));
    CHECK(t2 > ms(300));
    CHECK(t2 < ms(1000));
}

SCENARIO("a closed trigger chokes a sounding open hat (Q23/R173)") {
    // Reference: an unchoked open hat, long decay.
    auto h_ref = make(1.0);
    h_ref.trigger_open(1.0);
    std::vector<double> y_ref(static_cast<size_t>(1.0 * k_sr));
    for (auto& v : y_ref)
        v = h_ref.process();

    // The same hit choked by a closed trigger at 100 ms.
    auto h = make(1.0);
    h.trigger_open(1.0);
    std::vector<double> y(y_ref.size());
    for (size_t i = 0; i < y.size(); ++i) {
        if (i == ms(100))
            h.trigger_closed(1.0);
        y[i] = h.process();
    }

    const double tail_ref    = peak(y_ref, ms(200), ms(400));
    const double tail_choked = peak(y, ms(200), ms(400));
    INFO("200-400 ms tail: unchoked " << tail_ref << " vs choked " << tail_choked);
    CHECK(tail_ref > 8.0 * tail_choked);

    // And a later open hit rings at the pot's decay again (the choke is not sticky).
    auto y_after = [&] {
        h.trigger_open(1.0);
        std::vector<double> out(static_cast<size_t>(1.0 * k_sr));
        for (auto& v : out)
            v = h.process();
        return out;
    }();
    CHECK(decay_40db(y_after) > ms(300));
}

SCENARIO("per-path levels work independently") {
    auto h = make();
    h.set_closed_level(0.0);
    h.trigger_closed(1.0);
    double p = 0.0;
    for (int i = 0; i < 9600; ++i)
        p = std::max(p, std::abs(h.process()));
    CHECK(p == 0.0);

    h.set_closed_level(1.0);
    h.set_open_level(0.0);
    h.trigger_open(1.0);
    p = 0.0;
    for (int i = 0; i < 9600; ++i)
        p = std::max(p, std::abs(h.process()));
    CHECK(p < 1e-6); // tiny residue: the earlier closed hit is still fading
}

SCENARIO("hat accent scales monotonically; rendering is deterministic and bounded") {
    double peaks[3];
    int    n = 0;
    for (double a : {0.0, 0.5, 1.0}) {
        auto h = make();
        h.trigger_closed(a);
        double p = 0.0;
        for (int i = 0; i < 9600; ++i)
            p = std::max(p, std::abs(h.process()));
        peaks[n++] = p;
    }
    INFO("peaks " << peaks[0] << " / " << peaks[1] << " / " << peaks[2]);
    CHECK(peaks[0] > 0.02);
    CHECK(peaks[0] < peaks[1]);
    CHECK(peaks[1] < peaks[2]);
    CHECK(peaks[2] < 1.2);

    auto h1 = make();
    auto h2 = make();
    h1.trigger_open(1.0);
    h2.trigger_open(1.0);
    bool equal = true;
    for (int i = 0; i < 24000; ++i)
        equal = equal && h1.process() == h2.process();
    CHECK(equal);

    hat q;
    q.prepare(k_sr);
    double silent = 0.0;
    for (int i = 0; i < 4800; ++i)
        silent = std::max(silent, std::abs(q.process()));
    CHECK(silent == 0.0);
}
