/// @file
/// @brief      Unit tests for the channel-vocoder kernel (tap::tools::vocoder::bank).
/// @details    Checks the structural invariants: a silent carrier yields silence (at the default
///             sibilance of 0); makeup gain scales the output linearly; a silent modulator lets
///             the per-band envelopes decay to silence; processing is deterministic; the sibilance
///             path excites only when the modulator has high-band energy; the mix control's
///             endpoints are exact; and the seeded noise renders bit-identically per seed.
// SPDX-License-Identifier: MIT
// Copyright 2001-2026 Timothy Place.

#include <cmath>
#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/vocoder.h>

namespace {

    constexpr double k_pi = 3.14159265358979323846;

    double sine(int t, double f, double sr) {
        return std::sin(2.0 * k_pi * f * t / sr);
    }

} // namespace

SCENARIO("a silent carrier yields a silent output regardless of the modulator") {
    // The pinned contract at the default sibilance of 0 (set explicitly below so the pin survives
    // any future default change): with no carrier there is nothing to shape, even with an HF-rich
    // modulator charging the top-band envelopes.
    tap::tools::vocoder::bank v;
    v.prepare(48000.0);
    v.set_sibilance(0.0);

    double peak = 0.0;
    for (int t = 0; t < 8000; ++t) {
        const double mod = sine(t, 220.0, 48000.0) + sine(t, 6000.0, 48000.0);
        const double out = v.process(mod, 0.0);
        peak             = std::max(peak, std::abs(out));
    }
    REQUIRE(peak < 1e-12);
}

SCENARIO("the sibilance path excites the top bands only when the modulator has high-band energy") {
    const double sr = 48000.0;

    // Sibilance full up, carrier silent: any output can only come from the noise fed to the >4 kHz
    // carrier bands, gated by the modulator envelopes.
    auto peak_with_modulator = [&](double mod_hz) {
        tap::tools::vocoder::bank v;
        v.prepare(sr);
        v.set_sibilance(1.0);
        double peak = 0.0;
        for (int t = 0; t < 48000; ++t) {
            const double out = v.process(sine(t, mod_hz, sr), 0.0);
            if (t > 8000) { // past the envelope charge-up
                peak = std::max(peak, std::abs(out));
            }
        }
        return peak;
    };

    const double hf_peak = peak_with_modulator(6000.0); // inside the sibilance bands
    const double lf_peak = peak_with_modulator(200.0);  // far below every sibilance band

    // The LF case is not mathematically zero — the Q=20 skirts leak a little 200 Hz energy into
    // the top-band envelopes (~0.0006 peak vs ~0.14 for the HF case, measured) — so pin the two
    // regimes with an order-of-magnitude gap rather than absolute silence.
    REQUIRE(hf_peak > 0.05);           // HF modulator energy opens the noise path
    REQUIRE(lf_peak < 0.005);          // an LF-only modulator leaves it essentially shut
    REQUIRE(lf_peak < hf_peak / 20.0); // and far below the open state
}

SCENARIO("at sibilance 0 the output is bit-identical to the noise-free kernel") {
    // Same input, one bank at the default, one with the sibilance setter touched at 0 and a
    // different seed: the noise path must contribute nothing at all.
    tap::tools::vocoder::bank a, b;
    a.prepare(48000.0);
    b.prepare(48000.0);
    b.set_sibilance(0.0);
    b.set_seed(12345u);

    for (int t = 0; t < 8000; ++t) {
        const double mod = sine(t, 300.0, 48000.0) + sine(t, 7000.0, 48000.0);
        const double car = sine(t, 1700.0, 48000.0);
        REQUIRE(a.process(mod, car) == b.process(mod, car));
    }
}

SCENARIO("mix 0 returns the dry carrier and mix 100 matches the full-wet kernel exactly") {
    const double sr = 48000.0;

    GIVEN("mix 0") {
        tap::tools::vocoder::bank v;
        v.prepare(sr);
        v.set_mix(0.0);
        v.set_gain(3.0); // makeup gain is wet-side only; the dry carrier must pass untouched
        for (int t = 0; t < 8000; ++t) {
            const double car = sine(t, 1700.0, sr);
            REQUIRE(v.process(sine(t, 300.0, sr), car) == car);
        }
    }

    GIVEN("mix 100 against a default (never-touched) bank") {
        tap::tools::vocoder::bank a, b;
        a.prepare(sr);
        b.prepare(sr);
        b.set_mix(100.0);
        for (int t = 0; t < 8000; ++t) {
            const double mod = sine(t, 300.0, sr);
            const double car = sine(t, 1700.0, sr);
            REQUIRE(a.process(mod, car) == b.process(mod, car));
        }
    }

    GIVEN("an intermediate mix, which blends with equal power") {
        // At 50 % both gains are cos/sin(pi/4); spot-check one sample stream against the formula.
        tap::tools::vocoder::bank wet, mixed;
        wet.prepare(sr);
        mixed.prepare(sr);
        mixed.set_mix(50.0);
        const double g      = std::sqrt(0.5);
        double       maxerr = 0.0;
        for (int t = 0; t < 8000; ++t) {
            const double mod = sine(t, 300.0, sr);
            const double car = sine(t, 1700.0, sr);
            const double yw  = wet.process(mod, car);
            const double ym  = mixed.process(mod, car);
            maxerr           = std::max(maxerr, std::abs(ym - (g * car + g * yw)));
        }
        REQUIRE(maxerr < 1e-12);
    }
}

SCENARIO("the sibilance noise renders bit-identically for the same seed") {
    auto run = [](uint32_t seed) {
        tap::tools::vocoder::bank v;
        v.prepare(48000.0);
        v.set_seed(seed);
        v.set_sibilance(0.7);
        std::vector<double> out;
        for (int t = 0; t < 8000; ++t) {
            out.push_back(v.process(sine(t, 6000.0, 48000.0), 0.0));
        }
        return out;
    };
    const std::vector<double> a = run(42u), b = run(42u), c = run(43u);
    REQUIRE(a == b); // same seed: bit-identical
    REQUIRE(a != c); // different seed: a different noise sequence
}

SCENARIO("makeup gain scales the output linearly") {
    tap::tools::vocoder::bank a, b;
    a.prepare(48000.0);
    b.prepare(48000.0);
    a.set_gain(1.0);
    b.set_gain(2.0);

    double maxerr = 0.0;
    for (int t = 0; t < 8000; ++t) {
        const double mod = sine(t, 300.0, 48000.0);
        const double car = sine(t, 1700.0, 48000.0);
        const double ya  = a.process(mod, car);
        const double yb  = b.process(mod, car);
        maxerr           = std::max(maxerr, std::abs(yb - 2.0 * ya));
    }
    REQUIRE(maxerr < 1e-12);
}

SCENARIO("a silent modulator lets the output decay to silence") {
    tap::tools::vocoder::bank v;
    v.prepare(48000.0);
    v.set_response_ms(20.0);

    // Warm up with a full-band modulator and carrier so the envelopes charge; record the level.
    double warm_peak = 0.0;
    for (int t = 0; t < 8000; ++t) {
        const double out = v.process(sine(t, 500.0, 48000.0), sine(t, 500.0, 48000.0));
        if (t > 6000) {
            warm_peak = std::max(warm_peak, std::abs(out));
        }
    }
    // Now silence the modulator; keep driving the carrier. The envelopes decay exponentially, so the
    // output falls far below the warmed level (a long window lets the 20 ms follower unwind).
    double late_peak = 0.0;
    for (int t = 0; t < 48000; ++t) {
        const double out = v.process(0.0, sine(t, 500.0, 48000.0));
        if (t > 44000) {
            late_peak = std::max(late_peak, std::abs(out));
        }
    }
    REQUIRE(warm_peak > 1e-6);             // the warm-up actually produced signal
    REQUIRE(late_peak < 1e-4 * warm_peak); // and it decayed away
}

SCENARIO("processing is deterministic") {
    auto run = []() {
        tap::tools::vocoder::bank v;
        v.prepare(44100.0);
        v.set_q(30.0);
        std::vector<double> out;
        for (int t = 0; t < 4000; ++t) {
            out.push_back(v.process(sine(t, 440.0, 44100.0), sine(t, 130.0, 44100.0)));
        }
        return out;
    };
    const std::vector<double> a = run(), b = run();
    REQUIRE(a == b);
}
