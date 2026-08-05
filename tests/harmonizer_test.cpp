/// @file
/// @brief      Unit tests for the multi-voice harmonizer kernel (tap::tools::harmony::harmonizer).
/// @details    Drives the kernel with synthesized material and measures the output with the
///             DspTap YIN detector as the oracle (the house pattern) — a solo voice must land
///             its commanded interval, the dry path must be sample-aligned with the voices,
///             chords must stay bounded, formant preservation must keep a synthetic envelope
///             bump in place while the excitation moves, and disabled voices must re-enter
///             cleanly. Saw material throughout, per the shifter family's material contract.
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <cmath>
#include <complex>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <tap/dsp/yin.h>
#include <taptools/harmonizer.h>

namespace {

    constexpr double k_pi = 3.14159265358979323846;
    constexpr double k_sr = 48000.0;

    /// Band-limited-enough saw for test purposes: 20 harmonics.
    double saw(int t, double f) {
        double y = 0.0;
        for (int h = 1; h <= 20; ++h) {
            y += std::sin(2.0 * k_pi * f * h * t / k_sr) / h;
        }
        return y * (2.0 / k_pi);
    }

    std::vector<double> run_saw(tap::tools::harmony::harmonizer& hz, double freq, double seconds) {
        const int           n = static_cast<int>(seconds * k_sr);
        std::vector<double> out(static_cast<size_t>(n));
        for (int t = 0; t < n; ++t) {
            out[static_cast<size_t>(t)] = hz.process(saw(t, freq));
        }
        return out;
    }

    /// Fundamental of the signal's tail, via the certified DspTap detector.
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

    /// Projected magnitude at one frequency over the signal's tail (a one-bin DFT).
    double band_mag(const std::vector<double>& x, double f, size_t tail) {
        std::complex<double> acc{0.0, 0.0};
        const size_t         start = x.size() - tail;
        for (size_t t = start; t < x.size(); ++t) {
            const double ph = 2.0 * k_pi * f * static_cast<double>(t) / k_sr;
            acc += x[t] * std::complex<double>(std::cos(ph), -std::sin(ph));
        }
        return std::abs(acc) / static_cast<double>(tail);
    }

} // namespace

SCENARIO("a solo voice lands its commanded interval") {
    tap::tools::harmony::harmonizer hz;
    hz.prepare(k_sr);
    hz.set_dry(0.0);
    hz.set_gain(0, 1.0);
    hz.set_interval(0, 7.0); // a fifth up

    const auto   out      = run_saw(hz, 220.0, 1.5);
    const double expected = 220.0 * std::exp2(7.0 / 12.0);
    REQUIRE(std::abs(cents(measure_hz(out), expected)) < 10.0);
}

SCENARIO("a downward voice lands too") {
    tap::tools::harmony::harmonizer hz;
    hz.prepare(k_sr);
    hz.set_dry(0.0);
    hz.set_gain(1, 1.0);
    hz.set_interval(1, -5.0); // a fourth down

    const auto   out      = run_saw(hz, 220.0, 1.5);
    const double expected = 220.0 * std::exp2(-5.0 / 12.0);
    REQUIRE(std::abs(cents(measure_hz(out), expected)) < 10.0);
}

SCENARIO("the dry path is sample-aligned with a unity voice") {
    // At interval 0 the pvoc reconstructs its input delayed by exactly one frame
    // (pinned in DspTap); the kernel's dry ring must line up with it exactly.
    tap::tools::harmony::harmonizer hz;
    hz.prepare(k_sr);
    hz.set_dry(1.0);
    hz.set_gain(0, 1.0);
    hz.set_interval(0, 0.0);

    const int    n       = static_cast<int>(1.0 * k_sr);
    const size_t latency = hz.latency();
    REQUIRE(latency == 1024);

    std::vector<double> in(static_cast<size_t>(n));
    std::vector<double> out(static_cast<size_t>(n));
    for (int t = 0; t < n; ++t) {
        in[static_cast<size_t>(t)]  = saw(t, 220.0);
        out[static_cast<size_t>(t)] = hz.process(in[static_cast<size_t>(t)]);
    }

    // After the slews settle, out[t] must equal 2 * in[t - latency] to numerical noise.
    double max_err = 0.0;
    for (size_t t = 8192; t < static_cast<size_t>(n); ++t) {
        max_err = std::max(max_err, std::abs(out[t] - 2.0 * in[t - latency]));
    }
    REQUIRE(max_err < 1e-6);
}

SCENARIO("a chord stays bounded and both voices contribute") {
    tap::tools::harmony::harmonizer hz;
    hz.prepare(k_sr);
    hz.set_dry(1.0);
    hz.set_gain(0, 1.0);
    hz.set_interval(0, 4.0);
    hz.set_gain(1, 1.0);
    hz.set_interval(1, 7.0);

    const auto out = run_saw(hz, 220.0, 1.5);

    double peak = 0.0;
    double rms  = 0.0;
    for (size_t t = out.size() / 2; t < out.size(); ++t) {
        peak = std::max(peak, std::abs(out[t]));
        rms += out[t] * out[t];
    }
    rms = std::sqrt(rms / (out.size() / 2.0));

    REQUIRE(peak < 4.0); // three summed unit-ish voices, sane headroom
    REQUIRE(rms > 0.05); // and actually sounding
    const size_t tail = 1 << 15;
    REQUIRE(band_mag(out, 220.0 * std::exp2(4.0 / 12.0), tail) > 0.01); // the third is present
    REQUIRE(band_mag(out, 220.0 * std::exp2(7.0 / 12.0), tail) > 0.01); // the fifth is present
}

SCENARIO("formant preservation keeps the envelope bump where the source put it") {
    // Source: 220 Hz saw with a strong synthetic formant near 1320 Hz (harmonic 6
    // boosted). Shift up a fifth. With formant preservation the output's energy near
    // 1320 Hz must beat the formant-off run's; with it off, the bump rides up to ~1980.
    auto run = [](bool formant) {
        tap::tools::harmony::harmonizer hz;
        hz.prepare(k_sr);
        hz.set_dry(0.0);
        hz.set_formant(formant);
        hz.set_gain(0, 1.0);
        hz.set_interval(0, 7.0);

        const int           n = static_cast<int>(1.5 * k_sr);
        std::vector<double> out(static_cast<size_t>(n));
        for (int t = 0; t < n; ++t) {
            double x = 0.0;
            for (int h = 1; h <= 20; ++h) {
                const double boost = (h == 6) ? 8.0 : 1.0;
                x += boost * std::sin(2.0 * k_pi * 220.0 * h * t / k_sr) / h;
            }
            out[static_cast<size_t>(t)] = hz.process(x * (2.0 / k_pi));
        }
        return out;
    };

    const auto   on   = run(true);
    const auto   off  = run(false);
    const size_t tail = 1 << 15;

    // The shifted harmonic nearest the source bump: 1320 Hz region on the +7 grid.
    const double f_bump  = 220.0 * std::exp2(7.0 / 12.0) * 4.0; // ≈ 1318.5 Hz
    const double f_moved = 220.0 * std::exp2(7.0 / 12.0) * 6.0; // ≈ 1977.8 Hz — the bump if it rode up

    const double on_ratio  = band_mag(on, f_bump, tail) / std::max(band_mag(on, f_moved, tail), 1e-12);
    const double off_ratio = band_mag(off, f_bump, tail) / std::max(band_mag(off, f_moved, tail), 1e-12);
    REQUIRE(on_ratio > 2.0 * off_ratio); // the envelope stayed home only with the flag on
}

SCENARIO("interval glide walks the pitch instead of jumping it") {
    tap::tools::harmony::harmonizer hz;
    hz.prepare(k_sr);
    hz.set_dry(0.0);
    hz.set_gain(0, 1.0);
    hz.set_interval(0, 0.0);
    hz.set_glide(300.0);

    auto out = run_saw(hz, 220.0, 0.75);
    hz.set_interval(0, 12.0);
    // Mid-glide (about one time constant in), the pitch must sit strictly between.
    auto         mid    = run_saw(hz, 220.0, 0.35);
    const double mid_hz = measure_hz(mid);
    REQUIRE(mid_hz > 240.0);
    REQUIRE(mid_hz < 425.0);
    // And well past the glide it must settle on the octave.
    auto end = run_saw(hz, 220.0, 2.5);
    REQUIRE(std::abs(cents(measure_hz(end), 440.0)) < 10.0);
    (void)out;
}

SCENARIO("a disabled voice re-enters cleanly and the kernel stays finite") {
    tap::tools::harmony::harmonizer hz;
    hz.prepare(k_sr);
    hz.set_dry(1.0);
    hz.set_gain(0, 1.0);
    hz.set_interval(0, 3.0);

    auto a = run_saw(hz, 220.0, 0.5);
    hz.set_gain(0, 0.0); // off — the engine is skipped
    auto b = run_saw(hz, 220.0, 0.5);
    hz.set_gain(0, 1.0); // back on — cleared engine, one frame of silence, gain slew
    auto c = run_saw(hz, 220.0, 1.0);

    for (const auto* seg : {&a, &b, &c}) {
        for (const double y : *seg) {
            REQUIRE(std::isfinite(y));
        }
    }
    // With the voice off, the tail is dry-only: level near the dry saw's.
    double rms_b = 0.0;
    for (size_t t = b.size() / 2; t < b.size(); ++t) {
        rms_b += b[t] * b[t];
    }
    rms_b = std::sqrt(rms_b / (b.size() / 2.0));
    REQUIRE(rms_b > 0.05);
    // And the re-entered voice is audible again at the end.
    const size_t tail = 1 << 15;
    REQUIRE(band_mag(c, 220.0 * std::exp2(3.0 / 12.0), tail) > 0.01);
}

SCENARIO("the harmonizer passes input through before prepare") {
    tap::tools::harmony::harmonizer hz;
    REQUIRE(hz.process(0.25) == 0.25);
    REQUIRE(hz.latency() == 0);
}
