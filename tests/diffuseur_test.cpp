/// @file
/// @brief      Catch2 scenarios pinning the Ondes Martenot diffuseur kernels (diffuseur.h).
/// @details    Two things make this kernel's contract unusual enough to shape the suite. First,
///             the bodies are **recreations** — the mode data is Fletcher & Rossing's general
///             physics, not a measurement of Martenot's instruments — so there is no published
///             table to reproduce the way tests/touche_test.cpp reproduces one. What can be
///             pinned instead is that the maths is honest: unit peak gain per mode, weights that
///             sum to one, the ring time you asked for, and the published ratios coming back out.
///             Second, the signal order is a *claim about the instrument* — the transducer drives
///             the body, so the nonlinearity is upstream — and a claim like that is worth a null
///             test, which is what "the cabinet is its parts, wired in the order the instrument
///             wires them" is.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/diffuseur.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    namespace D = tap::tools::diffuseur;

    /// Magnitude of `x[from .. from+n)` at `hz` — the house's independent detector for a single
    /// bin (same Goertzel the fuzz suite measures harmonics with).
    double bin(const std::vector<double>& x, double hz, size_t from, size_t n) {
        const double w  = 2.0 * k_pi * hz / k_sr;
        const double c  = 2.0 * std::cos(w);
        double       s1 = 0.0, s2 = 0.0;
        for (size_t i = 0; i < n; ++i) {
            const double s = x[from + i] + c * s1 - s2;
            s2             = s1;
            s1             = s;
        }
        return std::sqrt(std::max(0.0, s1 * s1 + s2 * s2 - c * s1 * s2)) * 2.0 / static_cast<double>(n);
    }

    std::vector<double> sine(double hz, double amp, size_t n) {
        std::vector<double> x(n);
        for (size_t i = 0; i < n; ++i) {
            x[i] = amp * std::sin(2.0 * k_pi * hz * static_cast<double>(i) / k_sr);
        }
        return x;
    }

    /// Peak of a mode's response to a settled sine at `hz`, measured over the last tenth.
    double mode_response(double f0, double t60, double hz, double settle_s) {
        D::mode m;
        m.prepare(k_sr);
        m.set(f0, t60);
        const int n     = static_cast<int>(settle_s * k_sr);
        double    peak  = 0.0;
        const int start = n - static_cast<int>(0.1 * k_sr);
        for (int i = 0; i < n; ++i) {
            const double y = m.process(std::sin(2.0 * k_pi * hz * static_cast<double>(i) / k_sr));
            if (i >= start) {
                peak = std::max(peak, std::abs(y));
            }
        }
        return peak;
    }

} // namespace

// The whole boundedness argument rests on this: Steiglitz's b0 = (1 - R^2)/2 makes the peak
// magnitude 1 whatever the pole radius, so a bank of weighted modes is bounded by the sum of its
// weights and needs no limiter after it. If this drifts, every other claim in the file goes with it.
SCENARIO("a driven mode peaks at unity gain whatever its ring time") {
    for (double t60 : {0.02, 0.2, 2.0}) {
        // Settle for several ring times, and probe at the pole angle and a little either side —
        // a very high-Q mode is narrower than a coarse probe grid, which is a measurement trap.
        const double f0     = 440.0;
        const double settle = std::max(0.5, 4.0 * t60);
        double       peak   = 0.0;
        for (int k = -4; k <= 4; ++k) {
            peak = std::max(peak, mode_response(f0, t60, f0 * (1.0 + 0.0002 * k), settle));
        }
        INFO("t60 " << t60 << " s: peak gain " << peak);
        CHECK(peak > 0.98);
        CHECK(peak < 1.0001); // never above unity: that is what "bounded by the weights" means
    }
}

SCENARIO("the ring time a mode is asked for is the ring time it delivers") {
    D::mode m;
    m.prepare(k_sr);
    m.set(180.0, 4.0);

    std::vector<double> y(static_cast<size_t>(k_sr * 5.0));
    for (size_t i = 0; i < y.size(); ++i) {
        y[i] = m.process((i == 0) ? 1.0 : 0.0);
    }
    auto peak_at = [&](double t) {
        double       p = 0.0;
        const size_t a = static_cast<size_t>(t * k_sr);
        for (size_t i = 0; i < 9600; ++i) {
            p = std::max(p, std::abs(y[a + i]));
        }
        return p;
    };
    // 60 dB in 4 s is 45 dB in 3 s, and the envelope is exponential so that is exact.
    const double drop = 20.0 * std::log10(peak_at(3.5) / peak_at(0.5));
    INFO("decay over 3 s: " << drop << " dB (a 4 s T60 predicts -45)");
    CHECK(std::abs(drop + 45.0) < 0.2);
}

// The zeros at z = +-1 are why a driven bank cannot accumulate DC and needs no blocker after it.
SCENARIO("a mode passes nothing at DC and nothing at Nyquist") {
    D::mode m;
    m.prepare(k_sr);
    m.set(440.0, 0.5);

    double dc = 0.0;
    for (int i = 0; i < 48000; ++i) {
        dc = m.process(1.0);
    }
    INFO("settled response to a constant: " << dc);
    CHECK(std::abs(dc) < 1e-9);

    D::mode n;
    n.prepare(k_sr);
    n.set(440.0, 0.5);
    double nyq = 0.0;
    for (int i = 0; i < 48000; ++i) {
        nyq = n.process((i % 2 == 0) ? 1.0 : -1.0);
    }
    INFO("settled response to alternating ones: " << nyq);
    CHECK(std::abs(nyq) < 1e-9);
}

SCENARIO("the plate's weights sum to one, so the body cannot amplify what drives it") {
    D::plate p;
    p.prepare(k_sr);
    p.set_pitch_hz(180.0);
    p.set_decay(20.0);
    p.set_brightness(1.0);

    double sum = 0.0;
    for (int m = 0; m < D::k_plate_modes; ++m) {
        sum += p.mode_level(m);
    }
    INFO("sum of doublet weights: " << sum);
    CHECK(std::abs(sum - 1.0) < 1e-12);

    // And measured: a long ring time, full brightness, and a bounded input never exceeds it.
    unsigned r    = 12345u;
    double   peak = 0.0;
    for (int i = 0; i < static_cast<int>(k_sr * 5.0); ++i) {
        r              = r * 1664525u + 1013904223u;
        const double x = (r / 2147483648.0) - 1.0;
        peak           = std::max(peak, std::abs(p.process(x)));
    }
    INFO("peak output for |x| <= 1: " << peak);
    CHECK(peak <= 1.0);
}

SCENARIO("the plate's modes sit at the published free-plate ratios") {
    D::plate p;
    p.prepare(k_sr);
    p.set_pitch_hz(150.0);

    for (int m = 0; m < D::k_plate_modes; ++m) {
        const double got  = p.mode_hz(m) / p.mode_hz(0);
        const double want = D::k_plate_ratio[static_cast<size_t>(m)];
        INFO("mode " << m << ": " << got << " vs published " << want);
        // Within the fixed per-mode scatter (k_scatter_cents) and nothing more — the ratios are
        // the citation, the scatter is the imperfection, and the two must not be confused.
        CHECK(std::abs(got / want - 1.0) < 0.003);
    }
}

SCENARIO("brightness closes the plate down toward its fundamental") {
    auto upper_energy = [](double bright) {
        D::plate p;
        p.prepare(k_sr);
        p.set_pitch_hz(150.0);
        p.set_decay(2.0);
        p.set_brightness(bright);

        std::vector<double> y(static_cast<size_t>(k_sr * 2.0));
        unsigned            r = 999u;
        for (size_t i = 0; i < y.size(); ++i) {
            r    = r * 1664525u + 1013904223u;
            y[i] = p.process((r / 2147483648.0) - 1.0);
        }
        const size_t from = y.size() / 2;
        return bin(y, p.mode_hz(4), from, y.size() - from) / bin(y, p.mode_hz(0), from, y.size() - from);
    };

    const double open   = upper_energy(1.0);
    const double closed = upper_energy(0.2);
    INFO("mode 4 relative to the fundamental: brightness 1 -> " << open << ", brightness 0.2 -> " << closed);
    CHECK(closed < 0.2 * open); // b^4 at 0.2 is 1.6e-3 of b^4 at 1; a factor of five is a floor, not a fit
}

SCENARIO("a plate mode above the band is silenced rather than folded") {
    D::plate p;
    p.prepare(k_sr);
    p.set_brightness(1.0);
    p.set_pitch_hz(D::k_max_pitch_hz); // 4 kHz fundamental puts the 7.34 ratio past 0.45 sr

    CHECK(p.mode_level(0) > 0.0);
    CHECK(p.mode_level(D::k_plate_modes - 1) == 0.0);
}

SCENARIO("a lightly damped string rings for the time it is asked to") {
    D::sympathetic s;
    s.prepare(k_sr);
    s.set(220.0, 3.0, 12000.0);

    std::vector<double> y(static_cast<size_t>(k_sr * 4.0));
    for (size_t i = 0; i < y.size(); ++i) {
        y[i] = s.process((i < 8) ? 1.0 : 0.0);
    }
    // Measure the FUNDAMENTAL, not broadband energy: the in-loop lowpass kills the upper partials
    // faster by design, so a wideband envelope decays faster than the string's stated ring time.
    const size_t win  = static_cast<size_t>(0.4 * k_sr);
    const double a    = bin(y, 220.0, static_cast<size_t>(0.3 * k_sr), win);
    const double b    = bin(y, 220.0, static_cast<size_t>(2.3 * k_sr), win);
    const double drop = 20.0 * std::log10(b / a);
    INFO("fundamental decay over 2 s: " << drop << " dB (a 3 s T60 predicts -40)");
    CHECK(std::abs(drop + 40.0) < 3.0);
}

// An honest limit made into a test, because it is the one that will surprise someone: the two
// controls are not independent, and the header says so.
SCENARIO("damping and ring time are not independent, and the cap is where the string stops") {
    D::sympathetic light;
    light.prepare(k_sr);
    light.set(220.0, 3.0, 12000.0);

    D::sympathetic heavy;
    heavy.prepare(k_sr);
    heavy.set(220.0, 3.0, D::k_min_damp_hz); // the same 3 s asked of a heavily damped string

    INFO("loop gain: light " << light.feedback() << ", heavy " << heavy.feedback());
    CHECK(light.feedback() < D::k_fb_max);  // the light string gets what it asked for
    CHECK(heavy.feedback() == D::k_fb_max); // the heavy one is pinned at the cap
    CHECK(heavy.feedback() * 0.95 < 1.0);   // and the loop is still strictly contractive
}

SCENARIO("the harp answers the strings it has and ignores the pitches between them") {
    // The drive is faded in and out. Switching a tone on and off is a step, and a step excites
    // every string on the board — measured without the fades, the "off-note" reading is mostly
    // that transient rather than any sympathy, and the test would be measuring its own edges.
    auto tail = [](double hz) {
        D::harp h;
        h.prepare(k_sr);
        h.set_root_hz(110.0);
        h.set_tuning(D::tuning_chromatic);
        h.set_decay(6.0);
        h.set_detune(0.0);

        const int on     = static_cast<int>(k_sr * 2.0);
        const int all    = static_cast<int>(k_sr * 3.0);
        const int fade   = static_cast<int>(k_sr * 0.25);
        double    energy = 0.0;
        int       count  = 0;
        for (int i = 0; i < all; ++i) {
            double g = 0.0;
            if (i < on) {
                g = 1.0;
                if (i < fade) {
                    g = 0.5 - 0.5 * std::cos(k_pi * static_cast<double>(i) / static_cast<double>(fade));
                }
                if (i > on - fade) {
                    g = 0.5 - 0.5 * std::cos(k_pi * static_cast<double>(on - i) / static_cast<double>(fade));
                }
            }
            const double y = h.process(g * 0.3 * std::sin(2.0 * k_pi * hz * static_cast<double>(i) / k_sr));
            if (i > on + static_cast<int>(0.2 * k_sr)) {
                energy += y * y;
                ++count;
            }
        }
        return std::sqrt(energy / static_cast<double>(count));
    };

    // Every one of the twelve, against a drive a quarter-tone sharp of it — which is neither any
    // string's fundamental nor any string's harmonic.
    for (int i = 0; i < D::k_strings; ++i) {
        const double on_note  = tail(110.0 * std::exp2(static_cast<double>(i) / 12.0));
        const double off_note = tail(110.0 * std::exp2((static_cast<double>(i) + 0.5) / 12.0));
        INFO("string " << i << ": ring on the note " << on_note << ", a quarter-tone off " << off_note << ", ratio "
                       << on_note / off_note);
        CHECK(on_note > 4.0 * off_note);
    }
    // The ratio measured here climbs from about 4 on the lowest string to about a thousand on the
    // highest, and that is physics rather than a defect: at a fixed ring time a loop's Q scales
    // with f x T60, so the top of the board is far the more selective end of it.
}

SCENARIO("the harp is the same harp in every instance") {
    D::harp a, b;
    a.prepare(k_sr);
    b.prepare(k_sr);
    a.set_detune(20.0);
    b.set_detune(20.0);

    bool same = true;
    for (int i = 0; i < D::k_strings; ++i) {
        same = same && (a.string_hz(i) == b.string_hz(i));
    }
    REQUIRE(same); // the scatter is an index-keyed hash, not a draw — no seed, no drift

    // And the strings are actually scattered, or the check above would be vacuous.
    bool scattered = false;
    for (int i = 0; i < D::k_strings; ++i) {
        scattered = scattered || (std::abs(a.string_hz(i) - 110.0 * std::exp2(i / 12.0)) > 0.1);
    }
    CHECK(scattered);
}

// The moving-iron principle, and the only part of the transducer with a physical argument behind
// it: force follows the square of the flux, so the residual squared term puts a second harmonic
// on the output at exactly (asymmetry x amplitude / 2) relative to the fundamental.
SCENARIO("the driver's asymmetry is exactly the moving-iron squared term") {
    const double amp = 0.5;
    for (double asym : {0.1, 0.3, 0.6, 1.0}) {
        D::transducer t;
        t.prepare(k_sr);
        t.set_drive(1.0);
        t.set_asymmetry(asym);
        t.set_saturation(0.0); // the bounding stage off: this measures the squared law alone

        const std::vector<double> x = sine(200.0, amp, static_cast<size_t>(k_sr * 0.5));
        std::vector<double>       y(x.size());
        for (size_t i = 0; i < x.size(); ++i) {
            y[i] = t.process(x[i]);
        }
        const size_t from  = y.size() / 2;
        const size_t n     = y.size() - from;
        const double ratio = bin(y, 400.0, from, n) / bin(y, 200.0, from, n);
        INFO("asymmetry " << asym << ": second harmonic / fundamental = " << ratio << ", predicted "
                          << asym * amp * 0.5);
        CHECK(std::abs(ratio - asym * amp * 0.5) < 0.002);
        CHECK(bin(y, 600.0, from, n) < 1e-6); // a squared term makes a second harmonic and nothing else
    }
}

SCENARIO("with asymmetry and saturation at zero the driver is a plain gain") {
    D::transducer t;
    t.prepare(k_sr);
    t.set_drive(2.0);
    t.set_asymmetry(0.0);
    t.set_saturation(0.0);

    const std::vector<double> x = sine(1000.0, 0.4, static_cast<size_t>(k_sr * 0.4));
    std::vector<double>       y(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        y[i] = t.process(x[i]);
    }
    const size_t from = y.size() / 2;
    const size_t n    = y.size() - from;
    INFO("fundamental " << bin(y, 1000.0, from, n) << " (drive 2 on 0.4 predicts 0.8)");
    CHECK(std::abs(bin(y, 1000.0, from, n) - 0.8) < 0.002);
    CHECK(bin(y, 2000.0, from, n) < 1e-9);
    CHECK(bin(y, 3000.0, from, n) < 1e-9);
}

SCENARIO("the driver is bounded however hard it is driven") {
    D::transducer t;
    t.prepare(k_sr);
    t.set_drive(200.0);
    t.set_asymmetry(1.0);
    t.set_saturation(0.8); // swing_shape is bounded by 1/drive

    double peak = 0.0;
    for (int i = 0; i < 48000; ++i) {
        peak = std::max(peak, std::abs(t.process(std::sin(2.0 * k_pi * 137.0 * i / k_sr))));
    }
    // The bound is 2/saturation, not the saturator's own 1/saturation: a hard-driven squared law
    // is a nearly-constant positive waveform with brief negative excursions, and taking that DC
    // offset out doubles the worst-case swing. Measured at 1.49 here, against a 1.25 that the
    // obvious argument would have predicted.
    INFO("peak output at drive 200, asymmetry 1, saturation 0.8: " << peak);
    CHECK(peak > 1.0 / 0.8); // the naive bound is genuinely exceeded — this is not slack
    CHECK(peak < 2.0 / 0.8 + 1e-9);
}

// The structural claim of the whole file: the electrical signal reaches the transducer first, and
// the transducer's motion excites the body. Wiring the parts by hand in that order reproduces the
// machine exactly; wiring them the other way round does not, so the order is a real choice.
SCENARIO("a cabinet is its parts, wired in the order the instrument wires them") {
    D::metallique cab;
    cab.prepare(k_sr);
    cab.set_smooth_ms(0.0);
    cab.set_drive(1.7);
    cab.set_asymmetry(0.4);
    cab.set_saturation(0.9);
    cab.set_mix(100.0);
    cab.set_level(1.0);
    cab.set_pitch_hz(210.0);
    cab.set_decay(3.0);
    cab.set_brightness(0.8);

    D::transducer driver;
    driver.prepare(k_sr);
    driver.set_drive(1.7);
    driver.set_asymmetry(0.4);
    driver.set_saturation(0.9);
    D::plate body;
    body.prepare(k_sr);
    body.set_pitch_hz(210.0);
    body.set_decay(3.0);
    body.set_brightness(0.8);

    // And the same parts the other way round, to show the order is audible rather than notional.
    D::transducer rev_driver;
    rev_driver.prepare(k_sr);
    rev_driver.set_drive(1.7);
    rev_driver.set_asymmetry(0.4);
    rev_driver.set_saturation(0.9);
    D::plate rev_body;
    rev_body.prepare(k_sr);
    rev_body.set_pitch_hz(210.0);
    rev_body.set_decay(3.0);
    rev_body.set_brightness(0.8);

    bool   identical  = true;
    double difference = 0.0;
    for (int i = 0; i < 24000; ++i) {
        const double x = 0.7 * std::sin(2.0 * k_pi * 190.0 * i / k_sr);
        const double a = cab.process(x);
        const double b = body.process(driver.process(x));
        identical      = identical && (a == b);
        difference     = std::max(difference, std::abs(a - rev_driver.process(rev_body.process(x))));
    }
    REQUIRE(identical); // fully wet, the cabinet IS transducer -> body, bitwise
    INFO("largest difference against the reversed wiring: " << difference);
    CHECK(difference > 1e-3); // and the reversed wiring is a different machine
}

SCENARIO("a cabinet's balance ends are exact at both extremes") {
    D::palme p;
    p.prepare(k_sr);
    p.set_smooth_ms(0.0);
    p.set_level(1.0);

    p.set_mix(0.0);
    bool dry = true;
    for (int i = 0; i < 4800; ++i) {
        const double x = std::sin(2.0 * k_pi * 300.0 * i / k_sr);
        dry            = dry && (p.process(x) == x);
    }
    REQUIRE(dry); // fully dry is the input, bitwise — not cos(pi/2) times the input

    p.clear();
    p.set_mix(100.0);
    double wet_energy = 0.0;
    for (int i = 0; i < 48000; ++i) {
        const double y = p.process(0.5 * std::sin(2.0 * k_pi * 110.0 * i / k_sr));
        wet_energy += y * y;
    }
    CHECK(wet_energy > 0.0); // fully wet is the body, and the body is doing something
}

SCENARIO("a level move is slewed, so a cabinet does not click") {
    D::metallique cab;
    cab.prepare(k_sr);
    cab.set_smooth_ms(50.0);
    cab.set_mix(100.0);
    cab.set_level(0.0);
    // One continuous tone throughout: a break in the INPUT would show up as a step in the output
    // and the test would be measuring its own seam rather than the level move.
    auto tone = [](int i) { return 0.5 * std::sin(2.0 * k_pi * 180.0 * static_cast<double>(i) / k_sr); };
    int  n    = 0;
    for (; n < 24000; ++n) {
        cab.process(tone(n));
    }

    cab.set_level(1.0); // slam it open
    double last  = cab.process(tone(n++));
    double worst = 0.0;
    for (int i = 0; i < static_cast<int>(0.1 * k_sr); ++i, ++n) {
        const double y = cab.process(tone(n));
        worst          = std::max(worst, std::abs(y - last));
        last           = y;
    }
    INFO("largest single-sample step during a full-scale level move: " << worst);
    CHECK(worst < 0.02);
}

SCENARIO("unprepared, both cabinets pass their input through") {
    D::metallique m;
    D::palme      p;
    bool          clean = true;
    for (int i = 0; i < 100; ++i) {
        const double x = 0.01 * static_cast<double>(i);
        clean          = clean && (m.process(x) == x) && (p.process(x) == x);
    }
    REQUIRE(clean);
}
