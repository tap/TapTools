/// @file
/// @brief      Unit tests for the TR-808 bass drum kernel (tap::tools::tr808::kick).
/// @details    Pins the behaviors the DAFx-14 analysis documents: the ~49 Hz nominal ring, the
///             attack frequency jump (~an octave-plus for the first few ms), the pitch sigh (and
///             its disconnection bend), decay-knob monotonicity and range, accent as excitation
///             (not gain), tone brightness, the tuning bend, state persistence across triggers
///             (no "machine gun effect"), determinism, and long-tail boundedness/silence.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/tr808_kick.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    using tap::tools::tr808::kick;

    kick make(double decay = 0.5, double tone = 0.5, double level = 1.0) {
        kick k;
        k.prepare(k_sr);
        k.set_decay(decay);
        k.set_tone(tone);
        k.set_level(level);
        return k;
    }

    std::vector<double> render(kick& k, double seconds, double accent = 1.0) {
        std::vector<double> out(static_cast<size_t>(seconds * k_sr));
        k.trigger(accent);
        for (auto& s : out)
            s = k.process();
        return out;
    }

    double peak(const std::vector<double>& x, size_t begin = 0, size_t end = SIZE_MAX) {
        end      = std::min(end, x.size());
        double p = 0.0;
        for (size_t i = begin; i < end; ++i)
            p = std::max(p, std::abs(x[i]));
        return p;
    }

    /// Mean frequency from interpolated upward zero crossings inside [begin, end).
    double zc_frequency(const std::vector<double>& x, size_t begin, size_t end) {
        end = std::min(end, x.size());
        std::vector<double> crossings;
        for (size_t i = begin + 1; i < end; ++i) {
            if (x[i - 1] < 0.0 && x[i] >= 0.0) {
                const double frac = -x[i - 1] / (x[i] - x[i - 1]);
                crossings.push_back(static_cast<double>(i - 1) + frac);
            }
        }
        if (crossings.size() < 2)
            return 0.0;
        return k_sr * static_cast<double>(crossings.size() - 1) / (crossings.back() - crossings.front());
    }

    // Goertzel magnitude of one bin — the house measurement idiom.
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

    /// Samples until the rectified-peak envelope stays 40 dB below the note's peak.
    size_t decay_40db(const std::vector<double>& x) {
        const double thresh = peak(x) * 0.01;
        size_t       last   = 0;
        for (size_t i = 0; i < x.size(); ++i)
            if (std::abs(x[i]) > thresh)
                last = i;
        return last;
    }

} // namespace

SCENARIO("the bass drum rings near the schematic's 49 Hz") {
    auto k = make(0.8);
    auto y = render(k, 1.0);
    // Measure well past the ~6 ms attack shift.
    const double f = zc_frequency(y, ms(150), ms(400));
    INFO("tail frequency " << f << " Hz");
    CHECK(f > 45.0);
    CHECK(f < 55.0);
}

SCENARIO("the attack rings fast: the Q43 shift shortens the first half-cycle") {
    // The paper (§8.1): the shift lasts ~6 ms — less than one period at the raised
    // frequency — so it can't be measured as a pitch; what it changes is how fast the
    // waveform completes its first swing. At ~129 Hz the first half-cycle is ~4 ms;
    // without the shift (the bend disconnected) it takes ~10 ms at ~49 Hz.
    auto first_flip_ms = [](const std::vector<double>& y) {
        const double thresh = peak(y) * 0.01;
        size_t       onset  = 0;
        while (onset < y.size() && std::abs(y[onset]) < thresh)
            ++onset;
        const bool positive = y[onset] > 0.0;
        size_t     i        = onset;
        while (i < y.size() && (positive ? y[i] > -thresh : y[i] < thresh))
            ++i;
        return static_cast<double>(i) / k_sr * 1000.0;
    };

    auto k_stock = make(0.8);
    auto y_stock = render(k_stock, 0.3);
    auto k_soft  = make(0.8);
    k_soft.set_attack(0.0);
    auto y_soft = render(k_soft, 0.3);

    const double t_stock = first_flip_ms(y_stock);
    INFO("first half-cycle: stock " << t_stock << " ms");
    CHECK(t_stock < 6.5);

    // The shift is where the punch comes from: with the leg held at its resting value the
    // 1 ms pulse barely couples into the ~49 Hz resonator, and the note collapses.
    const double p_stock = peak(y_stock, 0, ms(25));
    const double p_soft  = peak(y_soft, 0, ms(25));
    INFO("early peak: stock " << p_stock << " vs attack-disconnected " << p_soft);
    CHECK(p_stock > 3.0 * p_soft);
}

SCENARIO("the pitch sighs: early ring is sharp and relaxes down as the note decays") {
    auto         k       = make(1.0);
    auto         y       = render(k, 1.5);
    const double f_early = zc_frequency(y, ms(30), ms(120));
    const double f_late  = zc_frequency(y, ms(400), ms(700));
    INFO("early " << f_early << " Hz vs late " << f_late << " Hz");
    CHECK(f_early > f_late + 0.3);

    // The paper's §2 example bend: disconnecting the sigh flattens the trajectory.
    auto kb = make(1.0);
    kb.set_sigh(0.0);
    auto         yb       = render(kb, 1.5);
    const double fb_early = zc_frequency(yb, ms(30), ms(120));
    const double fb_late  = zc_frequency(yb, ms(400), ms(700));
    INFO("sigh off: early " << fb_early << " Hz vs late " << fb_late << " Hz");
    CHECK(std::abs(fb_early - fb_late) < (f_early - f_late) * 0.5);
}

SCENARIO("the decay knob lengthens the note monotonically") {
    auto         k1 = make(0.1);
    auto         k2 = make(0.5);
    auto         k3 = make(1.0);
    auto         y1 = render(k1, 4.0);
    auto         y2 = render(k2, 4.0);
    auto         y3 = render(k3, 4.0);
    const size_t t1 = decay_40db(y1);
    const size_t t2 = decay_40db(y2);
    const size_t t3 = decay_40db(y3);
    INFO("-40 dB times: " << t1 / k_sr << " / " << t2 / k_sr << " / " << t3 / k_sr << " s");
    CHECK(t1 < t2);
    CHECK(t2 < t3);
    // Hardware ballpark: 50-800 ms across the pot's travel (loose bounds on the model).
    CHECK(t1 > ms(20));
    CHECK(t2 > ms(80));
    CHECK(t2 < static_cast<size_t>(2.0 * k_sr));
}

SCENARIO("accent scales the excitation monotonically") {
    auto         k1 = make();
    auto         k2 = make();
    auto         k3 = make();
    const double p1 = peak(render(k1, 0.5, 0.0));
    const double p2 = peak(render(k2, 0.5, 0.5));
    const double p3 = peak(render(k3, 0.5, 1.0));
    INFO("peaks " << p1 << " / " << p2 << " / " << p3);
    CHECK(p1 > 0.05); // unaccented is still a note (4 V floor on the trigger bus)
    CHECK(p1 < p2);
    CHECK(p2 < p3);
    CHECK(p3 < 1.2); // calibration: full accent stays in sane sample range
}

SCENARIO("tone controls the click brightness") {
    auto         k_dark    = make(0.5, 0.0);
    auto         k_bright  = make(0.5, 1.0);
    auto         y_dark    = render(k_dark, 0.2);
    auto         y_bright  = render(k_bright, 0.2);
    const double hf_dark   = goertzel(y_dark, 2000.0, 0, ms(20));
    const double hf_bright = goertzel(y_bright, 2000.0, 0, ms(20));
    INFO("2 kHz energy dark " << hf_dark << " vs bright " << hf_bright);
    CHECK(hf_bright > 2.0 * hf_dark);
    // The fundamental survives the tone pot at both extremes.
    CHECK(goertzel(y_dark, 49.0, 0, ms(200)) > 0.5 * goertzel(y_bright, 49.0, 0, ms(200)));
}

SCENARIO("level is a clean output gain") {
    auto k0 = make(0.5, 0.5, 0.0);
    auto k1 = make(0.5, 0.5, 1.0);
    CHECK(peak(render(k0, 0.3)) == 0.0);
    CHECK(peak(render(k1, 0.3)) > 0.1);
}

SCENARIO("the tuning bend scales the ring frequency") {
    auto k_up = make(0.8);
    k_up.set_tuning(2.0);
    auto         y_up = render(k_up, 0.6);
    const double f    = zc_frequency(y_up, ms(80), ms(250));
    INFO("tuning 2.0 tail " << f << " Hz");
    CHECK(f > 85.0);
    CHECK(f < 115.0);
}

SCENARIO("retriggering into a ringing tail is not a machine gun") {
    // An isolated note...
    auto k_a = make(0.9);
    auto y_a = render(k_a, 1.0);

    // ...vs the same note retriggered while the first is still ringing.
    auto k_b = make(0.9);
    k_b.trigger(1.0);
    std::vector<double> y_b(static_cast<size_t>(1.0 * k_sr));
    const size_t        retrig_at = ms(150);
    for (size_t i = 0; i < y_b.size(); ++i) {
        if (i == retrig_at)
            k_b.trigger(1.0);
        y_b[i] = k_b.process();
    }

    // The post-retrigger tail must be a real note, but not a bit-identical copy of the
    // isolated one — the surviving filter states interfere with the new excitation.
    double diff = 0.0;
    for (size_t i = retrig_at; i < retrig_at + ms(100); ++i)
        diff = std::max(diff, std::abs(y_b[i] - y_a[i - retrig_at]));
    INFO("max deviation " << diff);
    CHECK(peak(y_b, retrig_at, y_b.size()) > 0.1);
    CHECK(diff > 1e-3);
}

SCENARIO("rendering is deterministic") {
    auto k1 = make();
    auto k2 = make();
    auto y1 = render(k1, 0.5);
    auto y2 = render(k2, 0.5);
    REQUIRE(y1.size() == y2.size());
    for (size_t i = 0; i < y1.size(); ++i)
        REQUIRE(y1[i] == y2[i]);
}

SCENARIO("a long tail decays to true silence and stays finite") {
    auto k      = make(0.9);
    auto y      = render(k, 6.0);
    bool finite = true;
    for (double v : y)
        finite = finite && std::isfinite(v);
    CHECK(finite);
    CHECK(peak(y) < 1.2);
    CHECK(peak(y, y.size() - static_cast<size_t>(0.1 * k_sr)) < 1e-6);
}
