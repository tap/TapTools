/// @file
/// @brief      Unit tests for the diode-ladder kernel (tap::tools::diode::diode_filter).
/// @details    Pins the topology against Tim Stinchcombe's published TB-303 analysis: the exact
///             normalized magnitude response (|D(j*2^(1/4))| = 17 at fc, the ~14 dB first octave,
///             the 24 dB/oct asymptote), the k = 17 / sqrt(2)-rate oscillation results, the
///             feedback-HPF behaviors (unity DC at any resonance, thinning toward the corner, the
///             authentic stock-never-quite-self-oscillates trait), solver agreement/boundedness,
///             oversampling consistency, morph continuity, determinism, and tail safety.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <array>
#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/diode_ladder.h>

namespace {

    constexpr double k_pi = 3.14159265358979323846;

    namespace dio = tap::tools::diode;

    dio::diode_filter make(double sr = 48000.0) {
        dio::diode_filter f;
        f.prepare(sr);
        f.set_smooth_ms(0.0);
        return f;
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

    // Small-signal gain (dB) of the filter at probe frequency f: settle, then compare the
    // Goertzel level of the output window against the same window of the raw input.
    double gain_db_at(dio::diode_filter& f, double probe_hz, double sr) {
        const size_t        settle = static_cast<size_t>(0.25 * sr);
        const size_t        n      = static_cast<size_t>(1.0 * sr);
        std::vector<double> in(n), out(n);
        double              ph = 0.0;
        for (size_t i = 0; i < settle + n; ++i) {
            const double x = 0.01 * std::sin(ph);
            ph += 2.0 * k_pi * probe_hz / sr;
            const double y = f.process(x);
            if (i >= settle) {
                in[i - settle]  = x;
                out[i - settle] = y;
            }
        }
        return 20.0 * std::log10(level_at(out, probe_hz, sr) / level_at(in, probe_hz, sr));
    }

    // Fundamental estimate via averaged rising-zero-crossing periods.
    double measure_f0(const std::vector<double>& y, double sr) {
        double first = -1.0, last = -1.0;
        int    count = 0;
        for (size_t i = 1; i < y.size(); ++i) {
            if (y[i - 1] <= 0.0 && y[i] > 0.0) {
                const double frac = y[i - 1] / (y[i - 1] - y[i]);
                const double t    = (static_cast<double>(i - 1) + frac) / sr;
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

    // Ping the filter, discard a settle window, return the following seconds of free response.
    std::vector<double> ring(dio::diode_filter& f, double settle_s, double seconds, double sr) {
        f.clear();
        f.process(0.5);
        for (size_t i = 1; i < static_cast<size_t>(settle_s * sr); ++i) {
            f.process(0.0);
        }
        std::vector<double> y(static_cast<size_t>(seconds * sr));
        for (auto& v : y) {
            v = f.process(0.0);
        }
        return y;
    }

    double rms(const std::vector<double>& y, size_t from = 0) {
        double acc = 0.0;
        for (size_t i = from; i < y.size(); ++i) {
            acc += y[i] * y[i];
        }
        return std::sqrt(acc / static_cast<double>(y.size() - from));
    }

} // namespace

SCENARIO("the linearized response is Stinchcombe's TB-303 transfer function") {
    // Analytic references from H(s) = 1/(s^4 + 4*2^(3/4) s^3 + 10*sqrt(2) s^2 + 8*2^(1/4) s + 1)
    // probed at f/fc = 1, 2, 8, 16 (s = j*2^(1/4)*f/fc): |D| = 17 exactly at fc (imag part
    // cancels — the oscillation frequency), 82.57, 8961, 134145.
    const double sr = 48000.0;
    const double fc = 200.0;

    auto probe = [&](double mult) {
        auto f = make(sr);
        f.set_frequency(fc);
        f.set_resonance(0.0);
        return gain_db_at(f, fc * mult, sr);
    };

    const double at_fc = probe(1.0);
    const double at_2  = probe(2.0);
    const double at_8  = probe(8.0);
    const double at_16 = probe(16.0);

    REQUIRE(std::abs(at_fc - (-24.61)) < 1.0);
    REQUIRE(std::abs(at_2 - (-38.34)) < 1.0);
    REQUIRE(std::abs(at_8 - (-79.05)) < 1.5);

    // the diode-ladder slope story: shallow (~14 dB) in the first octave, 24 dB/oct asymptotic
    REQUIRE(std::abs((at_fc - at_2) - 13.73) < 1.5);
    REQUIRE(std::abs((at_8 - at_16) - 23.50) < 2.0);

    // DC follows a step at unity (settled step response)
    auto f = make(sr);
    f.set_frequency(fc);
    double y = 0.0;
    for (int i = 0; i < 48000; ++i) {
        y = f.process(0.01);
    }
    REQUIRE(std::abs(y - 0.01) < 1e-4);
}

SCENARIO("with fbhp 0 the filter self-oscillates at fc, top and bottom") {
    // Just past threshold the oscillation rides slightly flat of fc — the edges saturate
    // unevenly with amplitude, lowering the effective stage gains (~ -0.7x the resonance
    // excess; at threshold it vanishes). The tuning itself is exact: the residual detune is
    // identical at both ends of the range, which the cross-check below pins to 0.5%.
    std::array<double, 2> ratio{};
    int                   idx = 0;
    for (double fc : {500.0, 5000.0}) {
        auto f = make(48000.0);
        f.set_fbhp(0.0);
        f.set_resonance(1.02);
        f.set_frequency(fc);
        const auto y = ring(f, 2.0, 1.0, 48000.0);
        REQUIRE(rms(y) > 0.01); // it sings
        ratio[idx] = measure_f0(y, 48000.0) / fc;
        REQUIRE(std::abs(ratio[idx] - 1.0) < 0.025); // in tune
        for (double v : y) {
            REQUIRE(std::abs(v) < 3.0); // and bounded by the diodes
        }
        ++idx;
    }
    REQUIRE(std::abs(ratio[0] - ratio[1]) < 0.005); // prewarped tuning is fc-independent
}

SCENARIO("with the hardware feedback HPF, stock resonance never quite self-oscillates") {
    // The authentic TB-303 trait: the 150 Hz high-pass adds phase lead, the loop needs k > 17,
    // and the stock range tops out below threshold. The extended range pushes through, riding
    // slightly sharp of fc — the same phase lead, quantitatively.
    auto stock = make(48000.0);
    stock.set_resonance(1.0);
    stock.set_frequency(1000.0);
    const auto y_stock = ring(stock, 1.0, 1.0, 48000.0);
    REQUIRE(rms(y_stock) < 1e-4); // rings down, never sustains

    // Near the corner the closed loop needs k ~ 24.5 (see header): full extended resonance
    // just clears it, and the HPF's phase lead parks the oscillation ~19% sharp of fc.
    auto bent = make(48000.0);
    bent.set_resonance(1.5);
    bent.set_frequency(500.0);
    const auto y_bent = ring(bent, 4.0, 1.0, 48000.0);
    REQUIRE(rms(y_bent) > 0.005); // the bend sings
    const double ratio = measure_f0(y_bent, 48000.0) / 500.0;
    REQUIRE(ratio > 1.10); // sharp of fc,
    REQUIRE(ratio < 1.28); // by the predicted phase-lead shift
}

SCENARIO("resonance thins toward the feedback corner but DC stays at unity") {
    auto boost_at = [](double fc) {
        auto lo = make(48000.0);
        lo.set_frequency(fc);
        lo.set_resonance(0.0);
        auto hi = make(48000.0);
        hi.set_frequency(fc);
        hi.set_resonance(0.95);
        return gain_db_at(hi, fc, 48000.0) - gain_db_at(lo, fc, 48000.0);
    };
    const double boost_high = boost_at(2000.0);
    const double boost_low  = boost_at(250.0);
    REQUIRE(boost_high > boost_low + 4.0); // the squelch lives up high
    REQUIRE(boost_high > 10.0);

    // feedback is high-passed, so resonance costs no bass: a step still settles at unity
    auto f = make(48000.0);
    f.set_frequency(1000.0);
    f.set_resonance(0.9);
    double y = 0.0;
    for (int i = 0; i < 96000; ++i) {
        y = f.process(0.01);
    }
    REQUIRE(std::abs(y - 0.01) < 2e-4);
}

SCENARIO("solvers agree at gentle settings and stay bounded at the extremes") {
    auto run = [](int solver) {
        auto f = make(48000.0);
        f.set_solver(solver);
        f.set_frequency(1200.0);
        f.set_resonance(0.5);
        std::vector<double> y(24000);
        double              ph = 0.0;
        for (auto& v : y) {
            ph += 2.0 * k_pi * 110.0 / 48000.0;
            v = f.process(0.1 * std::sin(ph) + 0.05 * std::sin(2.7 * ph));
        }
        return y;
    };
    const auto fast  = run(dio::solver_fast);
    const auto exact = run(dio::solver_exact);
    double     acc   = 0.0;
    for (size_t i = 0; i < fast.size(); ++i) {
        acc += (fast[i] - exact[i]) * (fast[i] - exact[i]);
    }
    REQUIRE(std::sqrt(acc / fast.size()) < 1e-3);

    for (int solver : {dio::solver_fast, dio::solver_exact}) {
        auto f = make(48000.0);
        f.set_solver(solver);
        f.set_frequency(10000.0);
        f.set_resonance(dio::k_res_max);
        f.set_drive(dio::k_drive_range_db);
        double ph = 0.0;
        for (int i = 0; i < 24000; ++i) {
            ph += 2.0 * k_pi * 220.0 / 48000.0;
            const double y = f.process(0.9 * std::sin(ph));
            REQUIRE(std::isfinite(y));
            REQUIRE(std::abs(y) < 10.0);
        }
    }
}

SCENARIO("oversampling keeps the passband and reduces aliasing under drive") {
    auto response = [](int os) {
        auto f = make(48000.0);
        f.set_oversample(os);
        f.set_frequency(1000.0);
        f.set_resonance(0.3);
        return gain_db_at(f, 500.0, 48000.0);
    };
    REQUIRE(std::abs(response(1) - response(4)) < 1.0);

    // A 17 kHz tone driven hard: its 3rd harmonic (51 kHz) folds at 1x to 6.9 kHz — right in
    // the open passband (fc 16 kHz), where the bilinear warp can't hide it. At 4x the
    // harmonic really exists and dies in the decimation chain instead.
    auto alias_level = [](int os) {
        const double sr = 44100.0;
        auto         f  = make(sr);
        f.set_oversample(os);
        f.set_frequency(16000.0);
        f.set_resonance(0.0);
        f.set_drive(24.0);
        const size_t        settle = static_cast<size_t>(0.2 * sr);
        const size_t        n      = static_cast<size_t>(0.5 * sr);
        std::vector<double> out(n);
        double              ph = 0.0;
        for (size_t i = 0; i < settle + n; ++i) {
            ph += 2.0 * k_pi * 17000.0 / sr;
            const double y = f.process(0.8 * std::sin(ph));
            if (i >= settle) {
                out[i - settle] = y;
            }
        }
        return 20.0 * std::log10(level_at(out, 6900.0, sr) + 1e-12);
    };
    REQUIRE(alias_level(1) - alias_level(4) > 8.0);
}

SCENARIO("preset morphs and signal-rate cutoff modulation are continuous") {
    auto f = make(48000.0);
    f.set_frequency(300.0);
    f.set_resonance(0.2);
    f.store_preset(0);
    f.set_frequency(4000.0);
    f.set_resonance(0.9);
    f.store_preset(1);
    f.recall_preset(0, 0.0);

    f.recall_preset(1, 0.1);
    REQUIRE(f.morphing());
    double ph = 0.0, prev = 0.0;
    for (int i = 0; i < 9600; ++i) {
        ph += 2.0 * k_pi * 220.0 / 48000.0;
        const double y = f.process(0.3 * std::sin(ph));
        REQUIRE(std::abs(y - prev) < 0.25);
        prev = y;
    }
    REQUIRE(!f.morphing());

    // signal-rate override: audio-rate cutoff wobble stays finite and actually modulates
    auto g = make(48000.0);
    g.set_resonance(0.8);
    double ph2 = 0.0, mod = 0.0, acc = 0.0;
    for (int i = 0; i < 24000; ++i) {
        ph2 += 2.0 * k_pi * 110.0 / 48000.0;
        mod += 2.0 * k_pi * 200.0 / 48000.0;
        const double y = g.process(0.2 * std::sin(ph2), 2000.0 + 1500.0 * std::sin(mod));
        REQUIRE(std::isfinite(y));
        acc += std::abs(y);
    }
    REQUIRE(acc > 1.0);
}

SCENARIO("the diode ladder is deterministic") {
    auto run = [] {
        auto f = make(48000.0);
        f.set_frequency(800.0);
        f.set_resonance(1.2);
        f.set_drive(6.0);
        std::vector<double> y(12000);
        double              ph = 0.0;
        for (auto& v : y) {
            ph += 2.0 * k_pi * 110.0 / 48000.0;
            v = f.process(0.3 * std::sin(ph), 800.0 + 400.0 * std::sin(0.001 * ph));
        }
        return y;
    };
    REQUIRE(run() == run());
}

SCENARIO("long tails decay clean — no NaN, no denormal buzz") {
    auto f = make(48000.0);
    f.set_frequency(2000.0);
    f.set_resonance(0.5);
    f.process(0.5);
    double last = 1.0;
    for (int i = 0; i < 3 * 48000; ++i) {
        last = f.process(0.0);
        REQUIRE(std::isfinite(last));
    }
    REQUIRE(std::abs(last) < 1e-12);
}
