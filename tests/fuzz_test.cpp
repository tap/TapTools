/// @file
/// @brief      Catch2 scenarios pinning the tap.fuzz~ kernel (fuzz.h).
/// @details    Oracle-based where the promise is audible: harmonic structure is measured out of
///             the output with a local Goertzel probe rather than by asserting internals, which
///             is how the even/odd asymmetry contract and the tone-stack claims are pinned. The
///             aliasing claim is measured the only honest way — by looking for energy at
///             frequencies that are *not* harmonics of the input and watching it fall as the
///             oversample factor rises.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/fuzz.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    using tap::tools::fuzz::pedal;

    /// A pedal with instant setters and a flat voicing: scenarios opt into tone and asymmetry.
    pedal make() {
        pedal p;
        p.prepare(k_sr);
        p.set_smooth_ms(0.0);
        p.set_bass(0.0);
        p.set_treble(0.0);
        p.set_contrast(0.0);
        p.set_asymmetry(0.0);
        p.set_level_db(0.0);
        return p;
    }

    std::vector<double> render(pedal& p, double hz, double amp, double seconds) {
        const size_t        n = static_cast<size_t>(seconds * k_sr);
        std::vector<double> y(n, 0.0);
        for (size_t i = 0; i < n; ++i) {
            y[i] = p.process(amp * std::sin(2.0 * k_pi * hz * static_cast<double>(i) / k_sr));
        }
        return y;
    }

    /// Single-bin magnitude, 2|X(f)|/N — the same probe as the tr808 and discreet suites.
    double goertzel(const std::vector<double>& x, double f, size_t begin, size_t end) {
        const double w    = 2.0 * k_pi * f / k_sr;
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

    double rms(const std::vector<double>& x, size_t begin, size_t end) {
        double acc = 0.0;
        for (size_t i = begin; i < end; ++i) {
            acc += x[i] * x[i];
        }
        return std::sqrt(acc / static_cast<double>(end - begin));
    }

    size_t at(double seconds) {
        return static_cast<size_t>(seconds * k_sr);
    }

} // namespace

SCENARIO("silence in is exactly silence out, at any asymmetry") {
    // The bias lives inside the curve and is corrected at the stage output, so a asymmetric
    // pedal must still be exactly quiet when nothing is playing — no DC pedestal, no hum.
    for (double asym : {0.0, 0.5, 1.0}) {
        pedal p = make();
        p.set_gain(1.0);
        p.set_asymmetry(asym);
        bool silent = true;
        for (size_t i = 0; i < at(0.25); ++i) {
            silent = silent && (p.process(0.0) == 0.0);
        }
        INFO("asymmetry " << asym);
        REQUIRE(silent);
    }
}

SCENARIO("the clipping curve is monotonic, odd, and bounded at every knee") {
    // The curve is the only nonlinear thing in the file, so it is tested directly. A DC sweep
    // through the *stage* would measure nothing: the conditioning highpass settles a constant
    // input to zero, which is what it is for.
    using tap::tools::fuzz::shape;
    for (double k : {0.0, 0.5, 1.6, 2.0, 12.0}) {
        double last = -1e9;
        bool   mono = true, bounded = true, odd = true;
        for (int i = 0; i <= 4000; ++i) {
            const double x = -8.0 + 16.0 * static_cast<double>(i) / 4000.0;
            const double y = shape(x, k);
            mono           = mono && (y >= last);
            bounded        = bounded && std::isfinite(y) && (k < 1e-6 || std::abs(y) <= 1.0 / std::tanh(k) + 1e-12);
            odd            = odd && (std::abs(y + shape(-x, k)) < 1e-12);
            last           = y;
        }
        INFO("knee " << k);
        CHECK(mono);    // never folds back
        CHECK(bounded); // asymptote is 1/tanh(k)
        CHECK(odd);     // symmetric curve, so odd harmonics only (see the asymmetry scenario)
    }
    // And the normalization the cascade depends on: full scale in is full scale out, any knee.
    for (double k : {0.5, 1.6, 2.0, 12.0}) {
        INFO("knee " << k);
        CHECK(std::abs(shape(1.0, k) - 1.0) < 1e-12);
    }
}

SCENARIO("the pedal stays bounded on absurd input") {
    pedal p = make();
    p.set_gain(1.0);
    p.set_edge(1.0);
    p.set_asymmetry(1.0);

    bool bounded = true;
    for (size_t i = 0; i < at(0.2); ++i) {
        const double x = 50.0 * std::sin(2.0 * k_pi * 110.0 * static_cast<double>(i) / k_sr);
        const double y = p.process(x);
        bounded        = bounded && std::isfinite(y) && std::abs(y) < 4.0;
    }
    REQUIRE(bounded);
}

SCENARIO("more gain is more harmonic content") {
    const double f0             = 220.0;
    auto         harmonic_ratio = [&](double gain) {
        pedal p = make();
        p.set_gain(gain);
        const std::vector<double> y = render(p, f0, 0.3, 0.4);
        const size_t              b = at(0.2), e = at(0.4);
        const double              fund = goertzel(y, f0, b, e);
        double                    harm = 0.0;
        for (int k = 2; k <= 8; ++k) {
            const double m = goertzel(y, f0 * k, b, e);
            harm += m * m;
        }
        return std::sqrt(harm) / fund;
    };

    const double quiet = harmonic_ratio(0.0);
    const double loud  = harmonic_ratio(1.0);
    INFO("harmonic/fundamental: gain 0 = " << quiet << ", gain 1 = " << loud);
    REQUIRE(loud > quiet * 1.5);
}

// The DAFx-07 note this kernel's asymmetry exists for: a symmetric static curve produces odd
// harmonics only, and a real op-amp stage clips asymmetrically, which is where the even
// harmonics come from. Measured out of the output, not asserted about the code.
SCENARIO("asymmetry is what puts even harmonics in the spectrum") {
    const double f0       = 220.0;
    auto         even_odd = [&](double asym) {
        pedal p = make();
        p.set_gain(0.8);
        p.set_asymmetry(asym);
        const std::vector<double> y = render(p, f0, 0.3, 0.4);
        const size_t              b = at(0.2), e = at(0.4);
        double                    even = 0.0, odd = 0.0;
        for (int k = 2; k <= 8; ++k) {
            const double m = goertzel(y, f0 * k, b, e);
            (k % 2 == 0 ? even : odd) += m * m;
        }
        return std::sqrt(even) / std::sqrt(odd);
    };

    const double symmetric  = even_odd(0.0);
    const double asymmetric = even_odd(1.0);
    INFO("even/odd ratio: symmetric = " << symmetric << ", asymmetric = " << asymmetric);
    REQUIRE(symmetric < 0.05);              // a symmetric curve is odd-only, to the noise floor
    REQUIRE(asymmetric > symmetric * 10.0); // and asymmetry is what changes that
}

// Aliasing is the honest weakness of a static curve, so it gets a real measurement. Two things
// this test had to get right, both of which caught a bug the first time round:
//
//   * The tone must NOT divide the sample rate. At 3 kHz into 48 kHz every alias folds back
//     exactly onto a harmonic of the input and is invisible; 3733 Hz puts the folds at
//     frequencies nothing else occupies.
//   * The probe frequencies must be far from the fundamental. Probes a few hundred Hz away
//     measure spectral leakage from it (~1e-3 here) rather than aliasing, which swamps the
//     thing being measured.
//
// What is asserted is what is true: oversampling drops aliasing by two orders of magnitude
// against no oversampling. It is deliberately NOT asserted that 8x beats 2x — measured, the
// residual above 2x sits around -60 dB where filter numerics and window leakage dominate, and a
// test that pinned an ordering there would be pinning noise.
SCENARIO("oversampling drops the aliased energy by orders of magnitude") {
    const double f0          = 3733.0; // deliberately not a submultiple of the sample rate
    auto         alias_floor = [&](int os) {
        pedal p = make();
        p.set_gain(1.0);
        p.set_edge(1.0);
        p.set_oversample(os);
        const std::vector<double> y = render(p, f0, 0.5, 0.4);
        const size_t              b = at(0.2), e = at(0.4);
        // Where harmonics 8..13 of f0 fold back, skipping any fold that lands near the
        // fundamental (those probes read leakage, not aliasing).
        double acc = 0.0;
        for (int k = 8; k <= 13; ++k) {
            double f = k * f0;
            while (f > k_sr * 0.5) {
                f = (f > k_sr) ? f - k_sr : k_sr - f;
            }
            if (std::abs(f - f0) < 1000.0) {
                continue;
            }
            const double m = goertzel(y, f, b, e);
            acc += m * m;
        }
        return std::sqrt(acc);
    };

    const double none  = alias_floor(1);
    const double two   = alias_floor(2);
    const double four  = alias_floor(4);
    const double eight = alias_floor(8);
    INFO("alias energy: 1x = " << none << ", 2x = " << two << ", 4x = " << four << ", 8x = " << eight);
    REQUIRE(none > 0.05);       // the test material really does alias when nothing is done
    REQUIRE(two < none * 0.05); // and oversampling really does fix it
    REQUIRE(four < none * 0.05);
    REQUIRE(eight < none * 0.05);
}

SCENARIO("the tone stack moves the band it says it moves") {
    auto band = [](const std::vector<double>& y, double f) { return goertzel(y, f, at(0.2), at(0.4)); };

    // Drive the pedal with a low and a high tone at once and watch each shelf move its own end.
    auto render_pair = [&](double bass, double treble, double contrast) {
        pedal p = make();
        p.set_gain(0.3);
        p.set_bass(bass);
        p.set_treble(treble);
        p.set_contrast(contrast);
        std::vector<double> y(at(0.4), 0.0);
        for (size_t i = 0; i < y.size(); ++i) {
            const double t = static_cast<double>(i) / k_sr;
            y[i]           = p.process(0.15 * std::sin(2.0 * k_pi * 80.0 * t) + 0.15 * std::sin(2.0 * k_pi * 620.0 * t)
                                       + 0.15 * std::sin(2.0 * k_pi * 6000.0 * t));
        }
        return y;
    };

    const std::vector<double> flat    = render_pair(0.0, 0.0, 0.0);
    const std::vector<double> bassy   = render_pair(1.0, 0.0, 0.0);
    const std::vector<double> bright  = render_pair(0.0, 1.0, 0.0);
    const std::vector<double> scooped = render_pair(0.0, 0.0, 1.0);

    INFO("80 Hz: flat " << band(flat, 80.0) << " -> bassy " << band(bassy, 80.0));
    CHECK(band(bassy, 80.0) > band(flat, 80.0) * 1.5);
    CHECK(band(bassy, 6000.0) < band(flat, 6000.0) * 1.1); // and leaves the top alone

    INFO("6 kHz: flat " << band(flat, 6000.0) << " -> bright " << band(bright, 6000.0));
    CHECK(band(bright, 6000.0) > band(flat, 6000.0) * 1.5);
    CHECK(band(bright, 80.0) < band(flat, 80.0) * 1.1);

    INFO("620 Hz: flat " << band(flat, 620.0) << " -> scooped " << band(scooped, 620.0));
    CHECK(band(scooped, 620.0) < band(flat, 620.0) * 0.6); // contrast is a mid scoop
    CHECK(band(scooped, 80.0) > band(flat, 80.0) * 0.8);   // and it is a scoop, not a fader
}

SCENARIO("the output carries no DC, even wide open and asymmetric") {
    pedal p = make();
    p.set_gain(1.0);
    p.set_asymmetry(1.0);
    p.set_edge(1.0);

    const std::vector<double> y    = render(p, 220.0, 0.4, 1.0);
    double                    mean = 0.0;
    for (size_t i = at(0.5); i < y.size(); ++i) {
        mean += y[i];
    }
    mean /= static_cast<double>(y.size() - at(0.5));
    INFO("tail mean " << mean << ", rms " << rms(y, at(0.5), y.size()));
    REQUIRE(std::abs(mean) < 1e-3);
}

SCENARIO("level is a clean output trim") {
    auto level_rms = [](double db) {
        pedal p = make();
        p.set_gain(0.5);
        p.set_level_db(db);
        const std::vector<double> y = render(p, 220.0, 0.3, 0.4);
        return rms(y, at(0.2), at(0.4));
    };

    const double unity = level_rms(0.0);
    const double up    = level_rms(6.0);
    INFO("rms at 0 dB " << unity << ", at +6 dB " << up);
    REQUIRE(std::abs(up / unity - std::pow(10.0, 6.0 / 20.0)) < 0.02);
}

SCENARIO("unprepared, the pedal passes input through") {
    pedal p;
    REQUIRE(p.process(0.7) == 0.7);
    REQUIRE(p.process(-0.3) == -0.3);
}
