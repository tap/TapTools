/// @file
/// @brief      Unit tests for the vco kernel's analog-character section (taptools::vco::vco_osc).
/// @details    Pins the four analog upgrades added 2026-07-15 — waveform imperfection (saw
///             curvature + corner rounding, triangle asymmetry, seeded pulse-width tolerance),
///             fast pitch jitter, V/oct tracking error — plus the invariant that makes them safe:
///             at imperfect 0 every seed is exactly the ideal unit. The original waveform/sync/FM
///             scenarios live with the wrapper (source/projects/tap.vco_tilde in TapTools-Max).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/vco.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    namespace vco = taptools::vco;

    vco::vco_osc make(uint32_t seed = 1) {
        vco::vco_osc o;
        o.prepare(k_sr);
        o.set_smooth_ms(0.0);
        o.set_seed(seed);
        o.clear();
        return o;
    }

    std::vector<double> render(vco::vco_osc& o, int n) {
        std::vector<double> y(static_cast<size_t>(n));
        o.process(y.data(), y.size());
        return y;
    }

    // Goertzel magnitude at one frequency (Hann-windowed).
    double level_at(const std::vector<double>& x, double f) {
        const double w  = 2.0 * k_pi * f / k_sr;
        const double c  = 2.0 * std::cos(w);
        double       s1 = 0.0, s2 = 0.0;
        for (size_t i = 0; i < x.size(); ++i) {
            const double win = 0.5 - 0.5 * std::cos(2.0 * k_pi * i / (x.size() - 1));
            const double s0  = x[i] * win + c * s1 - s2;
            s2               = s1;
            s1               = s0;
        }
        const double re = s1 - s2 * std::cos(w);
        const double im = s2 * std::sin(w);
        return std::sqrt(re * re + im * im);
    }

    // Interpolated FFT-free fundamental estimate via averaged zero-crossing periods.
    double measure_f0(const std::vector<double>& y) {
        double first = -1.0, last = -1.0;
        int    count = 0;
        for (size_t i = 1; i < y.size(); ++i) {
            if (y[i - 1] <= 0.0 && y[i] > 0.0) {
                const double frac = y[i - 1] / (y[i - 1] - y[i]);
                const double t    = (static_cast<double>(i - 1) + frac) / k_sr;
                if (first < 0.0) {
                    first = t;
                }
                else {
                    ++count;
                }
                last = t;
            }
        }
        return (count > 0) ? count / (last - first) : 0.0;
    }

} // namespace

SCENARIO("at imperfect 0, every seed is exactly the ideal unit") {
    auto a = make(7);
    auto b = make(8);
    for (auto* o : {&a, &b}) {
        o->set_frequency(220.0);
        o->set_shape(vco::wave_pulse);
        o->set_pw(25.0);
    }
    const auto ya = render(a, 24000);
    const auto yb = render(b, 24000);
    REQUIRE(ya == yb);

    // and the PWM calibration holds (guards the pw path against the tolerance plumbing)
    double mean = 0.0;
    for (size_t i = 12000; i < ya.size(); ++i) {
        mean += ya[i];
    }
    mean /= 12000.0;
    REQUIRE(std::abs(mean - (-0.5)) < 0.03);
}

SCENARIO("imperfection makes the triangle asymmetric: even harmonics appear") {
    auto ratio_h2 = [](double imperfect) {
        auto o = make(7);
        o.set_frequency(220.0);
        o.set_shape(vco::wave_triangle);
        o.set_imperfect(imperfect);
        auto y = render(o, 48000);
        y.erase(y.begin(), y.begin() + 24000); // let the leaky integrator settle
        const double f0 = measure_f0(y);       // seeded static offset shifts the partials slightly
        return level_at(y, 2.0 * f0) / (level_at(y, f0) + 1e-30);
    };
    const double ideal = ratio_h2(0.0);
    const double worn  = ratio_h2(0.8);
    INFO("h2/h1: ideal " << ideal << ", imperfect 0.8 " << worn);
    REQUIRE(ideal < 1e-3);
    REQUIRE(worn > ideal * 30.0);
}

SCENARIO("imperfection bows the saw and rounds its top end") {
    auto saw_of = [](double imperfect) {
        auto o = make(7);
        o.set_frequency(440.0);
        o.set_shape(vco::wave_saw);
        o.set_imperfect(imperfect);
        return render(o, 48000);
    };
    const auto ideal = saw_of(0.0);
    const auto worn  = saw_of(1.0);

    // The ramp visibly bows (the shark-fin shape). Note this is deliberately a *waveform*
    // claim, not a harmonic-magnitude claim: the symmetric bend adds its parabola in
    // quadrature with the saw's sine components, so magnitudes barely move — the audible
    // spectral work is done by the corner rounding below (and by the triangle/pulse/sine
    // tolerances elsewhere). Both oscillators start at phase 0, and the seeded pitch offset
    // (~cents) is negligible across the first cycle.
    double bow = 0.0;
    for (int i = 5; i < 100; ++i) { // interior of the first ~109-sample cycle, reset excluded
        bow = std::max(bow, std::abs(worn[static_cast<size_t>(i)] - ideal[static_cast<size_t>(i)]));
    }
    INFO("max interior waveform deviation: " << bow);
    REQUIRE(bow > 0.03);

    // The rounded reset corner takes the extreme top end down audibly. Measure at multiples
    // of the *measured* fundamental — the seeded pitch offset moves h40 off its nominal bin.
    const double f0i     = measure_f0(ideal);
    const double f0w     = measure_f0(worn);
    const double drop_db = 20.0 * std::log10(level_at(ideal, f0i * 40.0) / (level_at(worn, f0w * 40.0) + 1e-30));
    INFO("h40 drop: " << drop_db << " dB");
    REQUIRE(drop_db > 4.0);
}

SCENARIO("jitter adds bounded, deterministic short-time pitch instability") {
    auto period_spread = [](double jitter_cents, uint32_t seed) {
        auto o = make(seed);
        o.set_frequency(440.0);
        o.set_shape(vco::wave_sine);
        o.set_jitter(jitter_cents);
        const auto          y = render(o, 96000);
        std::vector<double> periods;
        double              prev = -1.0;
        for (size_t i = 1; i < y.size(); ++i) {
            if (y[i - 1] <= 0.0 && y[i] > 0.0) {
                const double t = (static_cast<double>(i - 1) + y[i - 1] / (y[i - 1] - y[i])) / k_sr;
                if (prev >= 0.0) {
                    periods.push_back(t - prev);
                }
                prev = t;
            }
        }
        double mean = 0.0, var = 0.0;
        for (double p : periods) {
            mean += p;
        }
        mean /= static_cast<double>(periods.size());
        for (double p : periods) {
            var += (p - mean) * (p - mean);
        }
        return std::pair{std::sqrt(var / static_cast<double>(periods.size())) / mean, mean};
    };

    const auto [steady, m0] = period_spread(0.0, 7);
    const auto [alive, m1]  = period_spread(10.0, 7);
    INFO("relative period spread: jitter 0 -> " << steady << ", jitter 10 -> " << alive);
    REQUIRE(steady < 1e-6);
    REQUIRE(alive > steady * 100.0);
    REQUIRE(alive < 0.02); // a few cents of wobble, not vibrato

    // deterministic: the same seed renders bit-identically
    auto a = make(9), b = make(9);
    for (auto* o : {&a, &b}) {
        o->set_frequency(440.0);
        o->set_jitter(10.0);
    }
    REQUIRE(render(a, 24000) == render(b, 24000));
}

SCENARIO("track models V/oct calibration error away from A440") {
    auto cents_error = [](double freq, double track) {
        auto o = make(1);
        o.set_frequency(freq);
        o.set_shape(vco::wave_sine);
        o.set_track(track);
        const auto y = render(o, 96000);
        return 1200.0 * std::log2(measure_f0(y) / freq);
    };

    REQUIRE(std::abs(cents_error(440.0, 5.0)) < 1.0); // exact at the trim point
    const double up3 = cents_error(3520.0, 5.0);      // +3 octaves
    const double dn3 = cents_error(55.0, 5.0);        // -3 octaves
    INFO("track 5 cents/oct: +3 oct -> " << up3 << " cents, -3 oct -> " << dn3);
    REQUIRE(std::abs(up3 - 15.0) < 2.0);
    REQUIRE(std::abs(dn3 + 15.0) < 2.0);
}

SCENARIO("with imperfection up, different seeds are different units off the line") {
    auto unit = [](uint32_t seed) {
        auto o = make(seed);
        o.set_frequency(220.0);
        o.set_shape(vco::wave_pulse);
        o.set_imperfect(0.6);
        return render(o, 24000);
    };
    const auto u7  = unit(7);
    const auto u7b = unit(7);
    const auto u8  = unit(8);
    REQUIRE(u7 == u7b); // same seed, same unit
    double maxdiff = 0.0;
    for (size_t i = 0; i < u7.size(); ++i) {
        maxdiff = std::max(maxdiff, std::abs(u7[i] - u8[i]));
    }
    REQUIRE(maxdiff > 1e-3); // different seed, audibly different tolerances
}
