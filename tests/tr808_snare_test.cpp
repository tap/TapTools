/// @file
/// @brief      Unit tests for the TR-808 snare drum kernel (tap::tools::tr808::snare).
/// @details    Pins the Service Notes documentation: the two resonators near the late-revision
///             173/336 Hz pair, the VR8 tone crossfade, the VR9 snappy noise path (bright,
///             enveloped, ~60 ms-class decay per the p.14 chart), accent monotonicity, seeded
///             determinism, and silence/boundedness.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/tr808_snare.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    using tap::tools::tr808::snare;

    snare make(double tone = 0.5, double snappy = 0.5) {
        snare s;
        s.prepare(k_sr);
        s.set_tone(tone);
        s.set_snappy(snappy);
        s.set_level(1.0);
        return s;
    }

    std::vector<double> render(snare& s, double seconds, double accent = 1.0) {
        std::vector<double> out(static_cast<size_t>(seconds * k_sr));
        s.trigger(accent);
        for (auto& v : out)
            v = s.process();
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

SCENARIO("the snare's two resonators ring near the late-revision 173/336 Hz pair") {
    auto s = make(0.5, 0.0); // resonators only
    auto y = render(s, 0.4);
    // Sweep for the two spectral peaks in the drum's range.
    double best_lo = 0.0, best_lo_f = 0.0, best_hi = 0.0, best_hi_f = 0.0;
    for (double f = 120.0; f <= 260.0; f += 4.0) {
        const double m = goertzel(y, f, 0, ms(120));
        if (m > best_lo) {
            best_lo   = m;
            best_lo_f = f;
        }
    }
    for (double f = 280.0; f <= 420.0; f += 4.0) {
        const double m = goertzel(y, f, 0, ms(120));
        if (m > best_hi) {
            best_hi   = m;
            best_hi_f = f;
        }
    }
    INFO("peaks at " << best_lo_f << " Hz and " << best_hi_f << " Hz");
    CHECK(std::abs(best_lo_f - 173.0) < 20.0);
    CHECK(std::abs(best_hi_f - 336.0) < 25.0);
}

SCENARIO("the tone control crossfades fundamental against harmonic (VR8)") {
    auto         s_f       = make(0.0, 0.0);
    auto         s_h       = make(1.0, 0.0);
    auto         y_f       = render(s_f, 0.3);
    auto         y_h       = render(s_h, 0.3);
    const double f_at_fund = goertzel(y_f, 173.0, 0, ms(100));
    const double f_at_harm = goertzel(y_h, 173.0, 0, ms(100));
    const double h_at_fund = goertzel(y_f, 336.0, 0, ms(100));
    const double h_at_harm = goertzel(y_h, 336.0, 0, ms(100));
    INFO("173 Hz: " << f_at_fund << " vs " << f_at_harm << "; 336 Hz: " << h_at_fund << " vs " << h_at_harm);
    CHECK(f_at_fund > 4.0 * f_at_harm);
    CHECK(h_at_harm > 4.0 * h_at_fund);
}

SCENARIO("snappy adds the bright enveloped noise burst (VR9)") {
    auto s_dry = make(0.5, 0.0);
    auto s_wet = make(0.5, 1.0);
    auto y_dry = render(s_dry, 0.3);
    auto y_wet = render(s_wet, 0.3);
    // Noise energy well above the resonators, in the snappy high-pass band.
    const double hf_dry = goertzel(y_dry, 6000.0, 0, ms(40));
    const double hf_wet = goertzel(y_wet, 6000.0, 0, ms(40));
    INFO("6 kHz energy: snappy 0 -> " << hf_dry << ", snappy 1 -> " << hf_wet);
    CHECK(hf_wet > 5.0 * hf_dry);

    // The burst is over quickly: late-window HF energy collapses vs the early window.
    const double hf_late = goertzel(y_wet, 6000.0, ms(80), ms(120));
    CHECK(hf_wet > 5.0 * hf_late);
}

SCENARIO("the snare decays in the hardware's ~60 ms class") {
    auto         s = make();
    auto         y = render(s, 1.0);
    const size_t t = decay_40db(y);
    INFO("-40 dB at " << t / k_sr * 1000.0 << " ms");
    CHECK(t > ms(25));
    CHECK(t < ms(300));
}

SCENARIO("accent scales the snare's excitation monotonically") {
    auto         s1 = make();
    auto         s2 = make();
    auto         s3 = make();
    const double p1 = peak(render(s1, 0.3, 0.0));
    const double p2 = peak(render(s2, 0.3, 0.5));
    const double p3 = peak(render(s3, 0.3, 1.0));
    INFO("peaks " << p1 << " / " << p2 << " / " << p3);
    CHECK(p1 > 0.05);
    CHECK(p1 < p2);
    CHECK(p2 < p3);
    CHECK(p3 < 1.2);
}

SCENARIO("snare rendering is deterministic for a fixed seed") {
    auto s1 = make();
    auto s2 = make();
    s1.set_seed(7);
    s2.set_seed(7);
    auto y1    = render(s1, 0.3);
    auto y2    = render(s2, 0.3);
    bool equal = true;
    for (size_t i = 0; i < y1.size(); ++i)
        equal = equal && y1[i] == y2[i];
    CHECK(equal);

    // ...and different seeds decorrelate the noise (mc. behavior).
    auto s3 = make();
    s3.set_seed(8);
    auto y3   = render(s3, 0.3);
    bool same = true;
    for (size_t i = 0; i < y1.size(); ++i)
        same = same && y1[i] == y3[i];
    CHECK_FALSE(same);
}

SCENARIO("the swing-VCA drive: off is bit-identical, on saturates the snappy crack") {
    GIVEN("two snares with a hot snappy hit") {
        THEN("drive 0 (default) is bit-identical to an explicit set_drive(0)") {
            auto a = make(0.5, 1.0); // default: no set_drive call
            auto b = make(0.5, 1.0);
            b.set_drive(0.0); // explicit linear
            auto ya   = render(a, 0.3, 1.0);
            auto yb   = render(b, 0.3, 1.0);
            bool same = true;
            for (size_t i = 0; i < ya.size(); ++i)
                same = same && ya[i] == yb[i]; // exact equality — the linear path is untouched
            REQUIRE(same);
        }
        THEN("drive > 0 changes the signal, compresses the transient peak, and stays bounded") {
            auto clean = make(0.5, 1.0);
            auto warm  = make(0.5, 1.0);
            warm.set_drive(6.0); // hard swing-VCA saturation
            auto yc = render(clean, 0.3, 1.0);
            auto yw = render(warm, 0.3, 1.0);

            double diff   = 0.0;
            bool   finite = true;
            for (size_t i = 0; i < yc.size(); ++i) {
                diff += std::abs(yc[i] - yw[i]);
                finite = finite && std::isfinite(yw[i]);
            }
            REQUIRE(diff > 0.0); // the saturator is engaged
            REQUIRE(finite);
            REQUIRE(peak(yw) < peak(yc));       // compression softens the crack
            REQUIRE(peak(yw) > 0.3 * peak(yc)); // but does not gut the hit
        }
    }
}

SCENARIO("the snare is silent at rest and its tail reaches true silence") {
    auto                s = make();
    std::vector<double> quiet(static_cast<size_t>(0.1 * k_sr));
    for (auto& v : quiet)
        v = s.process();
    CHECK(peak(quiet) == 0.0);

    auto y      = render(s, 2.0);
    bool finite = true;
    for (double v : y)
        finite = finite && std::isfinite(v);
    CHECK(finite);
    CHECK(peak(y, y.size() - static_cast<size_t>(0.05 * k_sr)) < 1e-6);
}
