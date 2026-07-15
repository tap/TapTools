/// @file
/// @brief      Unit tests for the envelope-filter kernel (taptools::autowah::wah_filter).
/// @details    Pins the behaviors that define the Snow White model: follower attack/decay timing,
///             the exponential sweep law and its ceiling, upward (and inverted) sweep motion on a
///             burst, the sensitivity-off "cocked wah" equivalence against a bare svf_filter, Q
///             monotonicity, the sidechain input, abuse boundedness, determinism, and the
///             preset-morph engine (including the factory voicings in slots 0-3).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/autowah.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    namespace wah = taptools::autowah;

    double sine(int t, double f, double sr = k_sr) {
        return std::sin(2.0 * k_pi * f * t / sr);
    }

    // Invert the tanh soft knee to read the raw follower value back out of envelope().
    double raw_env(const wah::wah_filter& w) {
        const double s = std::min(w.envelope(), 0.999999999);
        return std::atanh(s) / wah::k_env_knee;
    }

    // Goertzel magnitude of one bin — the house measurement idiom.
    double goertzel(const std::vector<double>& x, double f, double sr = k_sr) {
        const double w  = 2.0 * k_pi * f / sr;
        const double c  = 2.0 * std::cos(w);
        double       s0 = 0.0, s1 = 0.0, s2 = 0.0;
        for (double v : x) {
            s0 = v + c * s1 - s2;
            s2 = s1;
            s1 = s0;
        }
        const double re = s1 - s2 * std::cos(w);
        const double im = s2 * std::sin(w);
        return 2.0 * std::sqrt(re * re + im * im) / static_cast<double>(x.size());
    }

    wah::wah_filter make(double smooth_ms = 0.0) {
        wah::wah_filter w;
        w.prepare(k_sr);
        w.set_smooth_ms(smooth_ms);
        return w;
    }

} // namespace

SCENARIO("the follower rises at the attack rate and falls at the decay rate") {
    auto w = make();
    w.set_attack(2.0);
    w.set_decay(250.0);

    // drive the detector with a DC key of 1.0 (sidechain); the audio path gets silence
    const int n_att_tau = static_cast<int>(2.0 * 0.001 * k_sr); // 96 samples
    for (int t = 0; t < n_att_tau; ++t) {
        w.process(0.0, 1.0);
    }
    // one time constant => 63.2% charge
    REQUIRE(raw_env(w) > 0.55);
    REQUIRE(raw_env(w) < 0.70);

    // charge fully, then release
    for (int t = 0; t < 48000; ++t) {
        w.process(0.0, 1.0);
    }
    const double charged = raw_env(w);
    REQUIRE(charged > 0.95);

    const int n_dec_tau = static_cast<int>(250.0 * 0.001 * k_sr);
    for (int t = 0; t < n_dec_tau; ++t) {
        w.process(0.0, 0.0);
    }
    const double after_one_tau = raw_env(w) / charged;
    REQUIRE(after_one_tau > 0.30);
    REQUIRE(after_one_tau < 0.44); // e^-1 = 0.368

    // a shorter decay releases faster
    auto fast = make();
    fast.set_decay(50.0);
    for (int t = 0; t < 48000; ++t) {
        fast.process(0.0, 1.0);
    }
    for (int t = 0; t < n_dec_tau; ++t) {
        fast.process(0.0, 0.0);
    }
    REQUIRE(raw_env(fast) < raw_env(w));
}

SCENARIO("the sweep law starts at bias and never exceeds the range ceiling") {
    auto w = make();

    // at rest the cutoff sits exactly at bias
    w.process(0.0, 0.0);
    REQUIRE(w.cutoff_hz() == 250.0);

    // a sustained full-scale key opens the filter toward (but never past) bias * 2^range
    const double ceiling = 250.0 * std::exp2(3.3);
    double       prev    = 0.0;
    bool         rising  = true;
    for (int t = 0; t < 48000; ++t) {
        w.process(0.0, 1.0);
        if (w.cutoff_hz() < prev - 1e-9) {
            rising = false;
        }
        prev = w.cutoff_hz();
        REQUIRE(w.cutoff_hz() <= ceiling + 1e-6);
    }
    REQUIRE(rising);
    // tanh knee: a 0 dB DC key lands at tanh(1.5) = 0.905 of the range
    REQUIRE(w.cutoff_hz() > 1800.0);
    REQUIRE(w.cutoff_hz() < 2100.0);

    // even an absurdly hot key saturates at the ceiling instead of running away
    auto hot = make();
    hot.set_sensitivity(wah::k_sens_ceil_db);
    for (int t = 0; t < 48000; ++t) {
        hot.process(0.0, 8.0);
        REQUIRE(hot.cutoff_hz() <= ceiling + 1e-6);
    }
    REQUIRE(hot.cutoff_hz() > ceiling * 0.99);
}

SCENARIO("a burst sweeps the cutoff up and back; direction inverts with negative range") {
    auto up = make();
    up.set_decay(100.0);
    double peak_cutoff = 0.0;
    for (int t = 0; t < 4800; ++t) { // 100 ms burst
        up.process(0.0, sine(t, 220.0));
        peak_cutoff = std::max(peak_cutoff, up.cutoff_hz());
    }
    REQUIRE(peak_cutoff > 700.0);     // well above the 250 Hz resting point
    for (int t = 0; t < 48000; ++t) { // 1 s of silence: back to rest
        up.process(0.0, 0.0);
    }
    REQUIRE(up.cutoff_hz() < 260.0);

    auto down = make();
    down.set_range(-3.3);
    down.set_bias(2500.0);
    double low_cutoff = 1e9;
    for (int t = 0; t < 4800; ++t) {
        down.process(0.0, sine(t, 220.0));
        low_cutoff = std::min(low_cutoff, down.cutoff_hz());
    }
    REQUIRE(low_cutoff < 1000.0); // swept below the resting point
}

SCENARIO("sensitivity at the floor is the cocked-wah: bit-close to a bare svf at bias") {
    auto w = make();
    w.set_sensitivity(wah::k_sens_floor_db);
    w.set_bias(800.0);
    w.set_resonance(0.7);

    taptools::svf::svf_filter ref;
    ref.prepare(k_sr, 1);
    ref.set_smooth_ms(0.0);
    ref.set_order(2);
    ref.set_mode(taptools::svf::mode_lowpass);
    ref.set_resonance(0.7);

    double maxerr = 0.0;
    for (int t = 0; t < 24000; ++t) {
        const double x  = sine(t, 500.0) * 0.5;
        const double yw = w.process(x);
        const double yr = ref.process(x, 800.0);
        maxerr          = std::max(maxerr, std::abs(yw - yr));
    }
    REQUIRE(maxerr < 1e-12);
}

SCENARIO("resonance sharpens the peak at the resting frequency monotonically") {
    auto gain_at_bias = [](double resonance) {
        auto w = make();
        w.set_sensitivity(wah::k_sens_floor_db); // fixed filter at bias
        w.set_bias(1000.0);
        w.set_resonance(resonance);
        std::vector<double> out;
        out.reserve(24000);
        for (int t = 0; t < 48000; ++t) {
            const double y = w.process(sine(t, 1000.0) * 0.1);
            if (t >= 24000) {
                out.push_back(y);
            }
        }
        return goertzel(out, 1000.0);
    };

    const double lo  = gain_at_bias(0.2);
    const double mid = gain_at_bias(0.5);
    const double hi  = gain_at_bias(0.8);
    REQUIRE(lo < mid);
    REQUIRE(mid < hi);
}

SCENARIO("the sidechain key drives the sweep, not the audio input") {
    auto w = make();
    // steady audio, silent key: the filter stays parked at bias
    for (int t = 0; t < 24000; ++t) {
        w.process(sine(t, 300.0), 0.0);
    }
    REQUIRE(w.cutoff_hz() == 250.0);

    // key comes up: the filter opens while the audio input is unchanged
    for (int t = 0; t < 24000; ++t) {
        w.process(sine(t, 300.0), 1.0);
    }
    REQUIRE(w.cutoff_hz() > 1500.0);
}

SCENARIO("maximum everything on square bursts stays bounded") {
    auto w = make(5.0);
    w.set_sensitivity(wah::k_sens_ceil_db);
    w.set_resonance(1.0);
    w.set_drive(wah::k_drive_max_db);
    w.set_gain(6.0);

    double peak = 0.0;
    for (int t = 0; t < 96000; ++t) {
        const bool   gate = (t / 4800) % 2 == 0; // 100 ms on/off bursts
        const double x    = gate ? (sine(t, 55.0) > 0.0 ? 0.9 : -0.9) : 0.0;
        if (t % 4096 == 0) { // thrash parameters mid-signal, on ramps
            w.set_bias((t / 4096) % 2 == 0 ? 100.0 : 4000.0);
            w.set_decay((t / 4096) % 2 == 0 ? 20.0 : 2000.0);
        }
        const double y = w.process(x);
        REQUIRE(std::isfinite(y));
        peak = std::max(peak, std::abs(y));
    }
    // resonance 1.0 + drive engages the svf's bounded self-oscillation: the tanh limits the band
    // node, and the lowpass node sits an integrator above it (~15x here, x2 for the 6 dB gain).
    // The assertion is boundedness — no runaway/inf — not politeness.
    REQUIRE(peak < 64.0);
}

SCENARIO("processing is bit-exactly deterministic") {
    auto a = make(5.0);
    auto b = make(5.0);
    for (auto* w : {&a, &b}) {
        w->set_resonance(0.9);
        w->set_drive(12.0); // driven circuit engaged
        w->set_decay(80.0);
    }
    for (int t = 0; t < 24000; ++t) {
        const double x  = sine(t, 110.0) * ((t / 2400) % 2 ? 0.8 : 0.0);
        const double ya = a.process(x);
        const double yb = b.process(x);
        REQUIRE(ya == yb);
    }
}

SCENARIO("presets store, morph continuously, and land exactly; factory slots ship the voicings") {
    auto w = make();

    // factory content (author-approved voicings)
    REQUIRE(w.preset(1).v[wah::p_bias] == 120.0);                       // bass
    REQUIRE(w.preset(2).v[wah::p_decay] == 1500.0);                     // slow swell
    REQUIRE(w.preset(3).v[wah::p_sensitivity] == wah::k_sens_floor_db); // cocked wah

    // store a custom voicing and morph to it over 100 ms
    w.set_bias(250.0);
    wah::params p         = w.snap_targets();
    p.v[wah::p_bias]      = 2000.0;
    p.v[wah::p_resonance] = 0.9;
    REQUIRE(w.set_preset(5, p));
    REQUIRE(w.recall_preset(5, 0.1));
    REQUIRE(w.morphing());

    // bias moves monotonically upward through the morph, never jumping
    double prev = w.snap_current().v[wah::p_bias];
    for (int t = 0; t < static_cast<int>(0.1 * k_sr); ++t) {
        w.process(0.0);
        const double cur = w.snap_current().v[wah::p_bias];
        REQUIRE(cur >= prev - 1e-9);
        REQUIRE(cur - prev < 1.0); // no step bigger than a ramp increment could make
        prev = cur;
    }
    REQUIRE_FALSE(w.morphing());
    REQUIRE(w.snap_current().v[wah::p_bias] == 2000.0);
    REQUIRE(w.snap_current().v[wah::p_resonance] == 0.9);

    // recall with 0 s jumps
    REQUIRE(w.recall_preset(3, 0.0));
    REQUIRE(w.snap_current().v[wah::p_sensitivity] == wah::k_sens_floor_db);

    // invalid slots are rejected
    REQUIRE_FALSE(w.store_preset(-1));
    REQUIRE_FALSE(w.store_preset(wah::k_presets));
}
