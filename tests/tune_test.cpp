/// @file
/// @brief      Unit tests for the pitch-correction kernel (tap::tools::tune::corrector).
/// @details    Drives the full chain — YIN detection, scale/MIDI target mapping, retune glide,
///             period-locked resynthesis — with synthesized tones and measures the output pitch
///             using the DspTap YIN primitive (certified by DspTap's own test battery) as the
///             oracle. Checks chromatic snapping, scale masks, per-note enables, MIDI targeting,
///             retune speed, correction amount, unpitched relaxation, and amplitude sanity.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <random>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/tune.h>

namespace {

    constexpr double k_pi = 3.14159265358979323846;
    constexpr double k_sr = 48000.0;

    double sine(int t, double f) {
        return std::sin(2.0 * k_pi * f * t / k_sr);
    }

    /// Run `seconds` of a sine through the corrector and return the output tail.
    std::vector<double> run_sine(tap::tools::tune::corrector& c, double freq, double seconds) {
        const int           n = static_cast<int>(seconds * k_sr);
        std::vector<double> out(static_cast<size_t>(n));
        for (int t = 0; t < n; ++t) {
            out[static_cast<size_t>(t)] = c.process(sine(t, freq));
        }
        return out;
    }

    /// Measure the fundamental of the last stretch of a signal with the DspTap detector.
    double measure_hz(const std::vector<double>& x) {
        const size_t  tau_min = static_cast<size_t>(k_sr / 2000.0);
        const size_t  tau_max = static_cast<size_t>(std::ceil(k_sr / 55.0));
        tap::dsp::yin det(tau_max, tau_min, tau_max);
        REQUIRE(x.size() >= det.frame_size());
        const auto r = det.analyze(x.data() + (x.size() - det.frame_size()));
        REQUIRE(r.voiced());
        return k_sr / r.period;
    }

    double cents(double f, double ref) {
        return 1200.0 * std::log2(f / ref);
    }

} // namespace

SCENARIO("chromatic hard snap retunes a sharp note to the nearest semitone") {
    tap::tools::tune::corrector c;
    c.prepare(k_sr);
    c.set_speed(0.0); // hard snap

    const auto out = run_sine(c, 452.0, 1.0); // 46.7 cents sharp of A4
    REQUIRE(std::abs(cents(measure_hz(out), 440.0)) < 5.0);
    REQUIRE(std::abs(c.detected_hz() - 452.0) < 2.0);
    REQUIRE(c.target_midi() == 69.0);
}

SCENARIO("the scale mask steers the target to an enabled note") {
    tap::tools::tune::corrector c;
    c.prepare(k_sr);
    c.set_speed(0.0);
    c.set_key(0); // C
    c.set_scale(tap::tools::tune::k_scale_major);

    // 460 Hz sits closest to A#4 (466.16), which C major disables; the nearest
    // enabled note is A4.
    const auto out = run_sine(c, 460.0, 1.0);
    REQUIRE(std::abs(cents(measure_hz(out), 440.0)) < 5.0);
}

SCENARIO("per-note enables edit the effective mask") {
    tap::tools::tune::corrector c;
    c.prepare(k_sr);
    c.set_speed(0.0);
    c.note_enable(9, false); // remove A from the chromatic mask

    // With A gone, 452 Hz snaps up to A#4 instead.
    const auto out = run_sine(c, 452.0, 1.0);
    REQUIRE(std::abs(cents(measure_hz(out), 466.16)) < 5.0);
}

SCENARIO("midi mode targets the nearest held note, and no held notes means no correction") {
    tap::tools::tune::corrector c;
    c.prepare(k_sr);
    c.set_speed(0.0);
    c.set_mode(tap::tools::tune::mode::midi);

    WHEN("a note is held") {
        c.note_on(64); // E4, 329.63 Hz
        const auto out = run_sine(c, 350.0, 1.0);
        REQUIRE(std::abs(cents(measure_hz(out), 329.63)) < 5.0);
        REQUIRE(c.target_midi() == 64.0);
    }

    WHEN("no note is held") {
        const auto out = run_sine(c, 350.0, 1.0);
        REQUIRE(std::abs(cents(measure_hz(out), 350.0)) < 5.0);
        REQUIRE(c.applied_semitones() == 0.0);
    }
}

SCENARIO("retune speed sets the glide onto the target") {
    tap::tools::tune::corrector c;
    c.prepare(k_sr);
    c.set_speed(100.0);

    // 452 -> A4 is -0.467 st. With a 100 ms time constant the glide should be
    // essentially complete (10 taus) after a second, but must still be in
    // motion shortly after detection first lands.
    double early = 0.0;
    for (int t = 0; t < static_cast<int>(0.1 * k_sr); ++t) {
        c.process(sine(t, 452.0));
    }
    early = c.applied_semitones();

    for (int t = static_cast<int>(0.1 * k_sr); t < static_cast<int>(1.5 * k_sr); ++t) {
        c.process(sine(t, 452.0));
    }
    const double full = c.applied_semitones();

    REQUIRE(std::abs(full - (-0.467)) < 0.03);
    REQUIRE(std::abs(early) > 0.0);
    REQUIRE(std::abs(early) < std::abs(full) * 0.95);
}

SCENARIO("amount scales the correction depth") {
    tap::tools::tune::corrector c;
    c.prepare(k_sr);
    c.set_speed(0.0);
    c.set_amount(50.0);

    // Half of the -46.7 cent correction: expect the output about 23 cents
    // above A4.
    const auto   out      = run_sine(c, 452.0, 1.0);
    const double measured = cents(measure_hz(out), 440.0);
    REQUIRE(std::abs(measured - 23.3) < 5.0);
}

SCENARIO("unpitched input relaxes to no correction and stays finite") {
    tap::tools::tune::corrector c;
    c.prepare(k_sr);
    c.set_speed(10.0);

    std::mt19937                           gen(42);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    double peak = 0.0;
    for (int t = 0; t < static_cast<int>(1.0 * k_sr); ++t) {
        const double y = c.process(dist(gen));
        REQUIRE(std::isfinite(y));
        peak = std::max(peak, std::abs(y));
    }
    REQUIRE(c.detected_hz() == 0.0);
    REQUIRE(std::abs(c.applied_semitones()) < 1e-3);
    REQUIRE(peak > 0.1);
    REQUIRE(peak < 2.0);
}

SCENARIO("voiced amplitude survives the two-tap resynthesis") {
    tap::tools::tune::corrector c;
    c.prepare(k_sr);
    c.set_speed(0.0);

    const auto out  = run_sine(c, 452.0, 1.0);
    double     peak = 0.0;
    for (size_t i = out.size() - 4800; i < out.size(); ++i) {
        peak = std::max(peak, std::abs(out[i]));
    }
    REQUIRE(peak > 0.8);
    REQUIRE(peak < 1.2);
}

SCENARIO("processing before prepare passes the input through") {
    tap::tools::tune::corrector c;
    REQUIRE(c.process(0.25) == 0.25);
    REQUIRE_FALSE(c.prepared());
}

namespace {

    /// Band-limited sawtooth normalized to peak 1 — harmonic-rich material every
    /// backend handles (PSOLA resamples the spectral envelope, so it needs
    /// harmonics; see tap/dsp/psola.h).
    std::vector<double> run_saw(tap::tools::tune::corrector& c, double freq, double seconds) {
        const int           n = static_cast<int>(seconds * k_sr);
        std::vector<double> wave(static_cast<size_t>(n), 0.0);
        double              peak = 0.0;
        for (int t = 0; t < n; ++t) {
            for (int h = 1; h <= 12; ++h) {
                wave[static_cast<size_t>(t)] += std::sin(2.0 * k_pi * freq * h * t / k_sr) / h;
            }
            peak = std::max(peak, std::abs(wave[static_cast<size_t>(t)]));
        }
        std::vector<double> out(static_cast<size_t>(n));
        for (int t = 0; t < n; ++t) {
            out[static_cast<size_t>(t)] = c.process(wave[static_cast<size_t>(t)] / peak);
        }
        return out;
    }

} // namespace

SCENARIO("every resynthesis backend snaps a sharp note onto the target") {
    using tap::tools::tune::backend;

    for (const auto b : {backend::grain, backend::psola, backend::pvoc}) {
        tap::tools::tune::corrector c;
        c.prepare(k_sr);
        c.set_speed(0.0);
        c.set_backend(b);
        REQUIRE(c.resynth_backend() == b);

        // 226 Hz saw: 46.4 cents sharp of A3 (110 * 2 = 220). Voice-like input so
        // the comparison is fair to all three engines.
        const auto out = run_saw(c, 226.0, 1.5);
        REQUIRE(std::abs(cents(measure_hz(out), 220.0)) < 6.0);

        double peak = 0.0;
        for (size_t i = out.size() - 4800; i < out.size(); ++i) {
            peak = std::max(peak, std::abs(out[i]));
        }
        REQUIRE(peak > 0.4);
        REQUIRE(peak < 2.0);
    }
}

SCENARIO("switching backends while running stays finite and keeps correcting") {
    using tap::tools::tune::backend;

    tap::tools::tune::corrector c;
    c.prepare(k_sr);
    c.set_speed(0.0);

    const backend sequence[] = {backend::grain, backend::pvoc, backend::psola, backend::grain};
    int           seg        = 0;
    for (const auto b : sequence) {
        c.set_backend(b);
        for (int t = seg * 12000; t < (seg + 1) * 12000; ++t) {
            const double y = c.process(std::sin(2.0 * k_pi * 452.0 * t / k_sr));
            REQUIRE(std::isfinite(y));
        }
        ++seg;
    }
    REQUIRE(std::abs(c.applied_semitones() - (-0.467)) < 0.03);
}
