/// @file
/// @brief      Unit tests for the overdrive kernel (tap::tools::od::overdrive).
/// @details    Pins the design goals from the overdrive handoff brief: the feedback loop's
///             frequency-dependent gain (bass pinned near-clean while mids take the full drive —
///             the claim a static shaper cannot satisfy), even harmonics appearing with asymmetry
///             and absent without it, the DC blocker holding the mean at zero under heavy
///             asymmetric clipping, the never-flat clean-through slope, the oversampling alias
///             improvement, body-voicing tilt, decay to silence (loop stability), determinism,
///             and parameter clamping.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/overdrive.h>

namespace {

    constexpr double k_pi = 3.14159265358979323846;
    constexpr double k_sr = 48000.0;

    namespace od = tap::tools::od;

    od::overdrive make(double sr = k_sr) {
        od::overdrive o;
        o.prepare(sr);
        o.set_smooth_ms(0.0);
        return o;
    }

    // Goertzel magnitude at one frequency (Hann-windowed).
    double level_at(const std::vector<double>& x, double f, double sr) {
        const double w  = 2.0 * k_pi * f / sr;
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

    // Drive a sine through the object; discard a settle window, return the following second.
    std::vector<double> run_sine(od::overdrive& o, double hz, double amp, double sr = k_sr) {
        const size_t        settle = static_cast<size_t>(0.5 * sr);
        const size_t        n      = static_cast<size_t>(1.0 * sr);
        std::vector<double> y(n);
        double              ph = 0.0;
        for (size_t i = 0; i < settle + n; ++i) {
            const double x = amp * std::sin(ph);
            ph += 2.0 * k_pi * hz / sr;
            const double v = o.process(x);
            if (i >= settle) {
                y[i - settle] = v;
            }
        }
        return y;
    }

    // Small-signal gain (dB) at a probe frequency: settle, then compare the Goertzel level of the
    // output window against the same window of the raw input.
    double gain_db_at(od::overdrive& o, double probe_hz, double sr = k_sr) {
        const size_t        settle = static_cast<size_t>(0.5 * sr);
        const size_t        n      = static_cast<size_t>(1.0 * sr);
        std::vector<double> in(n), out(n);
        double              ph = 0.0;
        for (size_t i = 0; i < settle + n; ++i) {
            const double x = 0.0005 * std::sin(ph);
            ph += 2.0 * k_pi * probe_hz / sr;
            const double y = o.process(x);
            if (i >= settle) {
                in[i - settle]  = x;
                out[i - settle] = y;
            }
        }
        return 20.0 * std::log10(level_at(out, probe_hz, sr) / level_at(in, probe_hz, sr));
    }

    double db(double ratio) {
        return 20.0 * std::log10(ratio);
    }

} // namespace

TEST_CASE("overdrive: silence in, silence out; bounded output across the parameter grid") {
    for (double drive : {0.0, 0.5, 1.0}) {
        for (double asym : {0.0, 1.0}) {
            auto o = make();
            o.set_drive(drive);
            o.set_asymmetry(asym);
            for (int i = 0; i < 4800; ++i) {
                REQUIRE(o.process(0.0) == 0.0);
            }
            const auto y = run_sine(o, 220.0, 1.0);
            for (double v : y) {
                REQUIRE(std::isfinite(v));
                REQUIRE(std::abs(v) < 4.0);
            }
        }
    }
}

TEST_CASE("overdrive: even harmonics appear with asymmetry and are absent without it") {
    const double f0 = 220.5;

    auto odd = make();
    odd.set_drive(0.6);
    odd.set_asymmetry(0.0);
    const auto   y_odd  = run_sine(odd, f0, 0.5);
    const double h1_odd = level_at(y_odd, f0, k_sr);
    const double h2_odd = level_at(y_odd, 2.0 * f0, k_sr);
    const double h3_odd = level_at(y_odd, 3.0 * f0, k_sr);

    // the signal path is odd-symmetric at asymmetry 0: strong odd harmonics, no even ones
    REQUIRE(db(h2_odd / h1_odd) < -50.0);
    REQUIRE(db(h3_odd / h1_odd) > -40.0);

    auto asym = make();
    asym.set_drive(0.6);
    asym.set_asymmetry(0.6);
    const auto   y_asym = run_sine(asym, f0, 0.5);
    const double h1     = level_at(y_asym, f0, k_sr);
    const double h2     = level_at(y_asym, 2.0 * f0, k_sr);

    REQUIRE(db(h2 / h1) > -35.0);
    REQUIRE(db(h2 / h1) > db(h2_odd / h1_odd) + 20.0);
}

TEST_CASE("overdrive: the feedback loop tilts gain with frequency — bass pinned, mids take the drive") {
    auto tilt_db = [](double drive) {
        auto lo = make();
        lo.set_drive(drive);
        auto hi = make();
        hi.set_drive(drive);
        return gain_db_at(hi, 4000.0) - gain_db_at(lo, 80.0);
    };

    const double t_low  = tilt_db(0.0);
    const double t_mid  = tilt_db(0.5);
    const double t_high = tilt_db(0.9);

    // at minimum drive the loop is barely tilted (just the voicing HP and mid bell)
    REQUIRE(t_low < 8.0);
    // the tilt grows with drive — the behavior a memoryless shaper cannot produce
    REQUIRE(t_high > 12.0);
    REQUIRE(t_high > t_mid);
    REQUIRE(t_mid > t_low);
}

TEST_CASE("overdrive: DC stays blocked under heavy asymmetric clipping") {
    auto o = make();
    o.set_drive(1.0);
    o.set_asymmetry(1.0);
    const auto y = run_sine(o, 100.0, 0.8); // exactly 100 periods in the 1 s window

    double mean = 0.0;
    for (double v : y) {
        mean += v;
    }
    mean /= static_cast<double>(y.size());
    REQUIRE(std::abs(mean) < 0.005);
}

TEST_CASE("overdrive: the clean-through path never lets the transfer flatten") {
    auto peak_for = [](double amp) {
        auto o = make();
        o.set_drive(1.0);
        const auto y = run_sine(o, 500.0, amp);
        double     p = 0.0;
        for (double v : y) {
            p = std::max(p, std::abs(v));
        }
        return p;
    };

    const double p25  = peak_for(0.25);
    const double p50  = peak_for(0.5);
    const double p100 = peak_for(1.0);

    REQUIRE(p50 > p25 + 0.01);
    REQUIRE(p100 > p50 + 0.02);
}

TEST_CASE("overdrive: oversampling lowers the alias floor without moving the fundamental") {
    const double f0    = 5001.0;
    const double alias = k_sr - 7.0 * f0; // H7 folded at 1x: 12993 Hz, clear of the harmonics

    auto rel_alias_db = [&](int os) {
        auto o = make();
        o.set_drive(0.9);
        o.set_asymmetry(0.2);
        o.set_oversample(os);
        const auto y = run_sine(o, f0, 0.6);
        return std::make_pair(db(level_at(y, alias, k_sr) / level_at(y, f0, k_sr)), db(level_at(y, f0, k_sr)));
    };

    const auto [alias_1x, fund_1x] = rel_alias_db(1);
    const auto [alias_4x, fund_4x] = rel_alias_db(4);

    REQUIRE(alias_1x - alias_4x > 10.0);
    REQUIRE(std::abs(fund_1x - fund_4x) < 1.5);
}

TEST_CASE("overdrive: body voices the spectrum — CCW keeps lows, CW thins them and pushes upper mids") {
    auto gain_at = [](double body, double probe_hz) {
        auto o = make();
        o.set_drive(0.3);
        o.set_body(body);
        return gain_db_at(o, probe_hz);
    };

    // CCW (fuller lows) passes far more 100 Hz than CW (thin, tight lows)
    REQUIRE(gain_at(-1.0, 100.0) - gain_at(1.0, 100.0) > 6.0);
    // CW pushes the upper mids
    REQUIRE(gain_at(1.0, od::k_voice_mid_hz) - gain_at(-1.0, od::k_voice_mid_hz) > 2.0);
    // CCW lifts the treble slightly
    REQUIRE(gain_at(-1.0, 8000.0) - gain_at(0.0, 8000.0) > 1.0);
}

TEST_CASE("overdrive: output decays to silence after the input stops (no limit cycles)") {
    auto o = make();
    o.set_drive(1.0);
    o.set_asymmetry(1.0);
    run_sine(o, 220.0, 1.0);

    double tail_max = 0.0;
    for (int i = 0; i < static_cast<int>(k_sr); ++i) {
        const double v = o.process(0.0);
        if (i >= static_cast<int>(0.8 * k_sr)) {
            tail_max = std::max(tail_max, std::abs(v));
        }
    }
    REQUIRE(tail_max < 1e-6);
}

TEST_CASE("overdrive: parameter ramps keep the output click-free") {
    auto o = make();
    o.set_smooth_ms(20.0);
    o.set_drive(0.0);
    o.snap();

    double ph = 0.0, prev = 0.0, max_step = 0.0;
    for (int i = 0; i < static_cast<int>(k_sr); ++i) {
        if (i == static_cast<int>(0.25 * k_sr)) {
            o.set_drive(1.0); // hard parameter jump mid-signal, ramped internally
            o.set_body(1.0);
        }
        const double x = 0.5 * std::sin(ph);
        ph += 2.0 * k_pi * 500.0 / k_sr;
        const double v = o.process(x);
        if (i > 0) {
            max_step = std::max(max_step, std::abs(v - prev));
        }
        prev = v;
    }
    // a 500 Hz sine at these levels moves ~0.05/sample; a click would be an order larger
    REQUIRE(max_step < 0.25);
}

TEST_CASE("overdrive: deterministic across identical instances") {
    auto a = make();
    auto b = make();
    a.set_drive(0.7);
    b.set_drive(0.7);
    a.set_asymmetry(0.4);
    b.set_asymmetry(0.4);
    a.set_body(-0.5);
    b.set_body(-0.5);

    double ph = 0.0;
    for (int i = 0; i < 48000; ++i) {
        const double x = 0.6 * std::sin(ph) + 0.2 * std::sin(2.7 * ph);
        ph += 2.0 * k_pi * 173.0 / k_sr;
        REQUIRE(a.process(x) == b.process(x));
    }
}

TEST_CASE("overdrive: parameter clamping") {
    REQUIRE(od::clamp_param(od::p_drive, 2.0) == 1.0);
    REQUIRE(od::clamp_param(od::p_drive, -1.0) == 0.0);
    REQUIRE(od::clamp_param(od::p_body, 2.0) == 1.0);
    REQUIRE(od::clamp_param(od::p_body, -2.0) == -1.0);
    REQUIRE(od::clamp_param(od::p_asymmetry, 1.5) == 1.0);
    REQUIRE(od::clamp_param(od::p_preamp, 100.0) == od::k_gain_range_db);
    REQUIRE(od::clamp_param(od::p_output, -100.0) == -od::k_gain_range_db);

    auto o = make();
    o.set_oversample(3); // snaps down to 2
    REQUIRE(o.oversample() == 2);
    o.set_oversample(100); // snaps to 8
    REQUIRE(o.oversample() == 8);
}
