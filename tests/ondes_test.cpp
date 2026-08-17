/// @file
/// @brief      Catch2 scenarios pinning the Ondes Martenot voice kernels (ondes.h).
/// @details    This file's contract is mostly *reproduction*, like tests/touche_test.cpp and
///             unlike most of the library: the tube model, its fitted parameters and the stage
///             operating points are all published, so the load-bearing scenarios check that the
///             published numbers come back out — the tube against its datasheet operating point,
///             the demodulator's harmonic series against its closed form, and the closed-form
///             detector against the full heterodyne simulation it replaces.
///
///             The last of those is the one to keep: the whole reason this kernel is affordable
///             is that the envelope of two summed oscillators has a closed form, so the 80 kHz
///             carrier never has to exist. If that equivalence ever stops holding, the object is
///             no longer a reduction of the published model, it is just a synthesizer.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/ondes.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    namespace O = tap::tools::ondes;

    /// Magnitude at `hz` over `x[from..)` — the house Goertzel probe.
    double bin(const std::vector<double>& x, double hz, size_t from, double sr = k_sr) {
        const double w  = 2.0 * k_pi * hz / sr;
        const double c  = 2.0 * std::cos(w);
        double       s1 = 0.0, s2 = 0.0;
        for (size_t i = from; i < x.size(); ++i) {
            const double s = x[i] + c * s1 - s2;
            s2             = s1;
            s1             = s;
        }
        return std::sqrt(std::max(0.0, s1 * s1 + s2 * s2 - c * s1 * s2)) * 2.0 / static_cast<double>(x.size() - from);
    }

    /// The full published demodulator, simulated the expensive way: two oscillators at F and
    /// F - f, summed, half-wave rectified by the near-zero-bias grid, loaded by R4*C21. This is
    /// the thing the kernel's closed form is claimed to replace, so the test builds it rather
    /// than trusting the claim.
    std::vector<double> full_heterodyne(double f, double sr, double carrier_hz, double tau_s, int cycles) {
        const size_t        n = static_cast<size_t>(sr * static_cast<double>(cycles) / f);
        std::vector<double> y(n);
        const double        decay = std::exp(-1.0 / (tau_s * sr));
        double              env   = 0.0;
        for (size_t i = 0; i < n; ++i) {
            const double t  = static_cast<double>(i) / sr;
            const double am = std::cos(2.0 * k_pi * carrier_hz * t) + std::cos(2.0 * k_pi * (carrier_hz - f) * t);
            env             = std::max(am, env * decay);
            y[i]            = env;
        }
        return y;
    }

    std::vector<double> run_detector(double f, double sr, int cycles) {
        O::detector d;
        d.prepare(sr);
        d.set_frequency(f);
        const size_t        n = static_cast<size_t>(sr * static_cast<double>(cycles) / f);
        std::vector<double> y(n);
        for (size_t i = 0; i < n; ++i) {
            y[i] = d.process();
        }
        return y;
    }

    std::vector<double> run_voice(double semitones, double drive, int os, double seconds = 1.0) {
        O::voice v;
        v.prepare(k_sr);
        v.set_oversample(os);
        v.set_smooth_ms(0.0);
        v.set_ribbon(semitones);
        v.set_drive(drive);
        v.set_key(1.0);
        v.set_level(1.0);
        const size_t        n = static_cast<size_t>(k_sr * seconds);
        std::vector<double> y(n);
        for (size_t i = 0; i < n; ++i) {
            y[i] = v.process();
        }
        return y;
    }

    /// Total harmonic content relative to the fundamental, harmonics 2..10.
    double thd(const std::vector<double>& y, double f0) {
        const size_t from = y.size() / 2;
        const double h1   = bin(y, f0, from);
        double       s    = 0.0;
        for (int k = 2; k <= 10; ++k) {
            const double h = bin(y, f0 * k, from);
            s += h * h;
        }
        return std::sqrt(s) / h1;
    }

} // namespace

// The tube model against a datasheet operating point. Tung-Sol's 6C5 sheet gives 8 mA of plate
// current at 250 V on the plate and -8 V on the grid; the parameter set the circuit paper fitted
// to that sheet has to land near it, or the fit was mis-transcribed.
SCENARIO("the published tube parameters reproduce their datasheet operating point") {
    const double ip = O::plate_current(O::k_6c5, 250.0, -8.0);
    INFO("6C5 at Vp = 250 V, Vg = -8 V: " << ip * 1000.0 << " mA (datasheet typical is 8 mA)");
    CHECK(ip * 1000.0 > 7.0);
    CHECK(ip * 1000.0 < 10.0);

    // And the model's basic physics: current rises with plate voltage and with grid voltage, and
    // there is none at all below the plate.
    CHECK(O::plate_current(O::k_6c5, 250.0, -4.0) > O::plate_current(O::k_6c5, 250.0, -8.0));
    CHECK(O::plate_current(O::k_6c5, 300.0, -8.0) > O::plate_current(O::k_6c5, 250.0, -8.0));
    CHECK(O::plate_current(O::k_6c5, 0.0, 0.0) == 0.0);

    // The grid-current branch: nothing below the threshold, ohmic above it (TASLP Eq. 9).
    CHECK(O::grid_current(O::k_6c5, 0.0) == 0.0);
    CHECK(O::grid_current(O::k_6c5, O::k_6c5.va - 0.01) == 0.0);
    CHECK(std::abs(O::grid_current(O::k_6c5, O::k_6c5.va + 1.3) - 1.3 / O::k_6c5.rgk) < 1e-15);
}

SCENARIO("the published operating points bias their tubes into class A") {
    struct expect {
        int                tube;
        O::operating_point op;
        const char*        name;
    };
    const expect cases[] = {{O::tube_6c5, O::k_op_demod, "6C5 demodulator"},
                            {O::tube_6c5, O::k_op_preamp, "6C5 preamplifier"},
                            {O::tube_2a3, O::k_op_power, "2A3 power amplifier"}};

    for (const expect& e : cases) {
        O::triode t;
        t.prepare(k_sr);
        t.set_tube(e.tube);
        t.set_operating_point(e.op);
        INFO(e.name << ": Vk = " << t.bias_v() << " V, Vp = " << t.quiescent_plate_v()
                    << " V, Ip = " << t.quiescent_current_a() * 1000.0 << " mA, gain " << t.small_signal_gain());
        CHECK(t.bias_v() > 0.5);                          // the cathode really is biased
        CHECK(t.quiescent_plate_v() > 0.25 * e.op.vbias); // and not slammed against either rail
        CHECK(t.quiescent_plate_v() < 0.95 * e.op.vbias);
        CHECK(t.quiescent_current_a() > 1e-4);
        CHECK(t.small_signal_gain() > 2.0); // a working stage, not a cut-off one
        CHECK(t.small_signal_gain() < e.op.vbias);
    }
}

// The reason the stage is worth solving from the tube model rather than reaching for a tanh: a
// triode is strongly asymmetric, and the asymmetry is where its even harmonics come from.
SCENARIO("a triode stage is asymmetric, and inverts") {
    O::triode t;
    t.prepare(k_sr);
    t.set_tube(O::tube_6c5);
    t.set_operating_point(O::k_op_demod);
    t.set_drive(1.0);

    // Inverting: a common-cathode stage is, and the sign is load-bearing here because the tube's
    // asymmetry acts on whichever side of the waveform actually reaches its grid.
    CHECK(t.curve_at(1.0) < 0.0);
    CHECK(t.curve_at(-1.0) > 0.0);

    // Asymmetric: equal grid swings either way do NOT give equal plate swings.
    const double up   = std::abs(t.curve_at(4.0));
    const double down = std::abs(t.curve_at(-4.0));
    INFO("plate swing at +-4 V of grid: " << up << " vs " << down);
    CHECK(up / down > 1.5);

    // Unity small-signal gain after normalization, so `drive` changes the distortion and not the
    // level — the gain-staging lesson fuzz.h learned the hard way.
    CHECK(std::abs(t.curve_at(0.001) / -0.001 - 1.0) < 0.02);
    CHECK(t.curve_at(0.0) == 0.0);
}

SCENARIO("the drive knob changes the distortion and not the level") {
    O::triode t;
    t.prepare(k_sr);
    t.set_tube(O::tube_6c5);
    t.set_operating_point(O::k_op_demod);

    double last_slope = 0.0;
    for (double d : {0.1, 1.0, 4.0}) {
        t.set_drive(d);
        const double slope = t.curve_at(0.0005) / -0.0005;
        INFO("drive " << d << ": small-signal slope " << slope);
        CHECK(std::abs(slope - 1.0) < 0.02); // the level is normalized out at every setting
        last_slope = slope;
    }
    CHECK(last_slope > 0.0);
}

// The load-bearing scenario. The whole affordability of this kernel rests on the envelope of the
// published oscillator sum having a closed form, so the carrier never needs to exist. If this
// equivalence stops holding, the object stops being a reduction of the published model.
SCENARIO("the closed-form detector reproduces the full heterodyne simulation") {
    for (double f : {110.0, 440.0, 1760.0}) {
        // The expensive version, at a rate that resolves an 80 kHz carrier properly.
        const double              full_sr = 3.0e6;
        const std::vector<double> full    = full_heterodyne(f, full_sr, 80000.0, O::k_detect_ms * 0.001, 60);
        const std::vector<double> cheap   = run_detector(f, 384000.0, 60);

        const size_t ff = full.size() / 2;
        const size_t cf = cheap.size() / 2;
        const double f1 = bin(full, f, ff, full_sr);
        const double c1 = bin(cheap, f, cf, 384000.0);

        for (int k = 2; k <= 4; ++k) {
            const double a = 20.0 * std::log10(bin(full, f * k, ff, full_sr) / f1);
            const double b = 20.0 * std::log10(bin(cheap, f * k, cf, 384000.0) / c1);
            INFO(f << " Hz, harmonic " << k << ": full " << a << " dB, closed form " << b << " dB");
            CHECK(std::abs(a - b) < 0.15);
        }
        // The one systematic difference: the closed form is a few percent louder, because a
        // follower chasing real carrier half-cycles never quite reaches the peak between them.
        INFO(f << " Hz: level ratio " << c1 / f1);
        CHECK(c1 / f1 > 1.0);
        CHECK(c1 / f1 < 1.05);
    }
}

// And what that envelope is: not a sinusoid. This is the correction the family plan needed — the
// demodulator generates a substantial harmonic series before any tube touches the signal.
SCENARIO("the demodulated envelope carries the published harmonic series before any tube") {
    const std::vector<double> y    = run_detector(440.0, 384000.0, 60);
    const size_t              from = y.size() / 2;
    const double              h1   = bin(y, 440.0, from, 384000.0);

    // |cos| has Fourier coefficients (4/pi)/(4n^2 - 1), so relative to the fundamental the second
    // harmonic sits at 3/15 and the third at 3/35 — -14.0 dB and -21.3 dB. The RC detector moves
    // them a little (it cannot follow the envelope down), which is why the bound is generous.
    const double h2 = 20.0 * std::log10(bin(y, 880.0, from, 384000.0) / h1);
    const double h3 = 20.0 * std::log10(bin(y, 1320.0, from, 384000.0) / h1);
    INFO("H2 " << h2 << " dB (ideal -14.0), H3 " << h3 << " dB (ideal -21.3)");
    CHECK(h2 > -17.0);
    CHECK(h2 < -12.0);
    CHECK(h3 > -25.0);
    CHECK(h3 < -19.0);
}

SCENARIO("the detector loses harmonics and level as it goes up, because the RC cannot keep up") {
    std::vector<double> h2, level;
    for (double f : {110.0, 1760.0}) {
        const std::vector<double> y    = run_detector(f, 384000.0, 60);
        const size_t              from = y.size() / 2;
        const double              a    = bin(y, f, from, 384000.0);
        level.push_back(a);
        h2.push_back(20.0 * std::log10(bin(y, 2.0 * f, from, 384000.0) / a));
    }
    INFO("second harmonic: " << h2[0] << " dB at A2, " << h2[1] << " dB at A6");
    CHECK(h2[1] < h2[0] - 3.0);
    INFO("level: " << 20.0 * std::log10(level[1] / level[0]) << " dB across five octaves");
    CHECK(level[1] < level[0]);
    CHECK(20.0 * std::log10(level[1] / level[0]) > -6.0); // a tilt, not a collapse
}

SCENARIO("oscillator balance is the cheapest timbre control the object has") {
    std::vector<double> h2;
    for (double depth : {0.25, 0.5, 1.0}) {
        O::detector d;
        d.prepare(384000.0);
        d.set_frequency(220.0);
        d.set_depth(depth);
        std::vector<double> y(static_cast<size_t>(384000.0 * 60.0 / 220.0));
        for (size_t i = 0; i < y.size(); ++i) {
            y[i] = d.process();
        }
        const size_t from = y.size() / 2;
        const double h1   = bin(y, 220.0, from, 384000.0);
        h2.push_back(20.0 * std::log10(bin(y, 440.0, from, 384000.0) / h1));
        INFO("depth " << depth << ": H2 " << h2.back() << " dB");
    }
    REQUIRE(h2.size() == 3);
    CHECK(h2[0] < h2[1]); // unequal oscillators never close the envelope, so the series thins
    CHECK(h2[1] < h2[2]);

    // At depth 0 the envelope is a constant: two oscillators that are not beating make no note.
    O::detector flat;
    flat.prepare(k_sr);
    flat.set_depth(0.0);
    bool constant = true;
    for (int i = 0; i < 4800; ++i) {
        constant = constant && (std::abs(flat.process() - 1.0) < 1e-12);
    }
    REQUIRE(constant);
}

SCENARIO("the ribbon is linear in semitones, which is the published law") {
    O::detector d;
    d.prepare(k_sr);
    for (double st : {0.0, 12.0, 24.0, 36.0, 60.0}) {
        d.set_ribbon(st);
        INFO(st << " semitones -> " << d.frequency() << " Hz");
        CHECK(std::abs(d.frequency() - O::k_a1_hz * std::exp2(st / 12.0)) < 1e-9);
        CHECK(std::abs(d.semitones() - st) < 1e-9);
    }
    d.set_ribbon(0.0);
    CHECK(std::abs(d.frequency() - 55.0) < 1e-12); // A1, the ribbon's lowest note
}

SCENARIO("the voice plays the note the ribbon asks for") {
    for (double st : {0.0, 24.0, 48.0}) {
        const std::vector<double> y    = run_voice(st, 1.0, 4);
        const double              want = O::k_a1_hz * std::exp2(st / 12.0);
        const size_t              from = y.size() / 2;
        // The fundamental has to be the strongest thing in the signal, not merely present.
        const double h1 = bin(y, want, from);
        INFO(st << " st (" << want << " Hz): H1 " << h1);
        CHECK(h1 > 0.3);
        for (int k = 2; k <= 5; ++k) {
            CHECK(bin(y, want * k, from) < h1);
        }
    }
}

// The gain-staging promise, at the level of the whole instrument this time.
SCENARIO("drive adds harmonics monotonically without running away in level") {
    double last_thd = 0.0;
    double first_h1 = 0.0;
    for (double d : {0.0, 1.0, 2.0, 4.0, 8.0}) {
        const std::vector<double> y = run_voice(24.0, d, 4);
        const double              t = thd(y, 220.0);
        const double              h = bin(y, 220.0, y.size() / 2);
        INFO("drive " << d << ": THD " << t << ", H1 " << h);
        CHECK(t > last_thd); // more drive, more harmonics — every step
        last_thd = t;
        if (first_h1 == 0.0) {
            first_h1 = h;
        }
        CHECK(h > 0.5 * first_h1); // and the level does not collapse on the way
    }
    // At drive 0 the tubes are as linear as they get and the harmonics are still there, because
    // the DEMODULATOR made them. That is the object's whole thesis in one assertion.
    const double clean = thd(run_voice(24.0, 0.0, 4), 220.0);
    INFO("harmonic content with the tubes barely driven: " << clean);
    CHECK(clean > 0.15);
}

SCENARIO("the coupling polarity is audible, which is why it is a switch") {
    O::voice a, b;
    for (O::voice* v : {&a, &b}) {
        v->prepare(k_sr);
        v->set_smooth_ms(0.0);
        v->set_ribbon(24.0);
        v->set_drive(4.0);
        v->set_key(1.0);
        v->set_level(1.0);
    }
    b.set_polarity(-1);

    std::vector<double> ya(static_cast<size_t>(k_sr)), yb(ya.size());
    for (size_t i = 0; i < ya.size(); ++i) {
        ya[i] = a.process();
        yb[i] = b.process();
    }
    INFO("THD: +1 " << thd(ya, 220.0) << ", -1 " << thd(yb, 220.0));
    CHECK(std::abs(thd(ya, 220.0) - thd(yb, 220.0)) > 0.05);
}

// The paper drops the power amplifier for real-time and says why. This checks their reason
// rather than their conclusion.
SCENARIO("the power stage really is the least important of the three") {
    O::voice off, on;
    for (O::voice* v : {&off, &on}) {
        v->prepare(k_sr);
        v->set_smooth_ms(0.0);
        v->set_ribbon(24.0);
        v->set_drive(2.0);
        v->set_key(1.0);
        v->set_level(1.0);
    }
    on.set_power_stage(true);

    std::vector<double> a(static_cast<size_t>(k_sr)), b(a.size());
    for (size_t i = 0; i < a.size(); ++i) {
        a[i] = off.process();
        b[i] = on.process();
    }
    INFO("THD without the 2A3 " << thd(a, 220.0) << ", with it " << thd(b, 220.0));
    CHECK(std::abs(thd(a, 220.0) - thd(b, 220.0)) < 0.02);
    CHECK(thd(b, 220.0) > thd(a, 220.0)); // it does something, just not much
}

// A wrapper test asked for silence at a rest position and got 20 ms of sound: the voice's
// smoothing was not reaching the intensity key, which keeps its own slew, so the one control the
// instrument is played with was slewing at a time nobody had set.
SCENARIO("the voice's smoothing reaches the intensity key") {
    O::voice v;
    v.prepare(k_sr);
    v.set_smooth_ms(0.0);
    v.set_ribbon(24.0);
    v.set_level(1.0);
    v.set_key(1.0);
    for (int i = 0; i < 4800; ++i) {
        v.process(); // get it sounding
    }

    v.set_key(0.0);
    bool immediate = true;
    for (int i = 0; i < 4800; ++i) {
        immediate = immediate && (v.process() == 0.0);
    }
    REQUIRE(immediate); // smooth 0 means smooth 0, everywhere

    // And with smoothing on, the key really does take that long rather than snapping.
    v.set_smooth_ms(50.0);
    v.set_key(1.0);
    for (int i = 0; i < 4800; ++i) {
        v.process();
    }
    v.set_key(0.0);
    int sounded = 0;
    for (int i = 0; i < 4800; ++i) {
        sounded += (v.process() != 0.0) ? 1 : 0;
    }
    INFO("samples still sounding after a slewed release: " << sounded);
    CHECK(sounded > 100);
}

SCENARIO("the intensity key silences the voice at rest and opens it at full press") {
    O::voice v;
    v.prepare(k_sr);
    v.set_smooth_ms(0.0);
    v.set_ribbon(24.0);
    v.set_level(1.0);
    v.set_key(0.0);

    bool silent = true;
    for (int i = 0; i < 24000; ++i) {
        silent = silent && (v.process() == 0.0);
    }
    REQUIRE(silent); // the bottom of the key's travel is exact silence — touche.h's contract

    v.set_key(1.0);
    double peak = 0.0;
    for (int i = 0; i < 24000; ++i) {
        peak = std::max(peak, std::abs(v.process()));
    }
    CHECK(peak > 0.1);
}

SCENARIO("where the key sits in the chain changes what it does") {
    O::voice after, before;
    for (O::voice* v : {&after, &before}) {
        v->prepare(k_sr);
        v->set_smooth_ms(0.0);
        v->set_ribbon(24.0);
        v->set_drive(6.0);
        v->set_key(0.62);
        v->set_level(1.0);
    }
    before.set_key_placement(O::key_before);

    std::vector<double> a(static_cast<size_t>(k_sr)), b(a.size());
    for (size_t i = 0; i < a.size(); ++i) {
        a[i] = after.process();
        b[i] = before.process();
    }
    // Placed after, the key is a clean output law and the harmonic content is whatever full
    // press would give. Placed before, a half-pressed key drives the tubes less hard, so the
    // sound is cleaner — pressure becomes dirt, which is the whole reason to offer the choice.
    INFO("THD with the key after " << thd(a, 220.0) << ", before " << thd(b, 220.0));
    CHECK(thd(b, 220.0) < thd(a, 220.0));
}

SCENARIO("oversampling improves the aliasing, and never makes it worse") {
    // Two traps this probe is built to avoid, both of which caught earlier drafts in this repo.
    // A tone that divides the sample rate folds every alias onto a harmonic, where it is
    // invisible; and probing halfway between harmonics finds nothing at all, because a perfectly
    // periodic signal has exactly zero energy there. So: a fundamental that divides nothing, and
    // probes at the COMPUTED fold frequencies, skipping any that land near a real harmonic.
    const double f0 = 2637.0; // roughly E7, and coprime with anything that matters

    std::vector<double> floors;
    for (int os : {1, 2, 4}) {
        O::voice v;
        v.prepare(k_sr);
        v.set_oversample(os);
        v.set_smooth_ms(0.0);
        v.set_frequency(f0);
        v.set_drive(4.0);
        v.set_key(1.0);
        v.set_level(1.0);
        std::vector<double> y(static_cast<size_t>(k_sr));
        for (size_t i = 0; i < y.size(); ++i) {
            y[i] = v.process();
        }

        const size_t from  = y.size() / 2;
        const double h1    = bin(y, f0, from);
        double       worst = 0.0;
        for (int k = 2; k <= 40; ++k) {
            double fold = std::fmod(f0 * k, k_sr);
            if (fold > 0.5 * k_sr) {
                fold = k_sr - fold;
            }
            if (fold < 60.0 || fold > 0.45 * k_sr) {
                continue;
            }
            bool on_harmonic = false;
            for (int q = 1; q * f0 < 0.5 * k_sr; ++q) {
                on_harmonic = on_harmonic || (std::abs(fold - q * f0) < 200.0);
            }
            if (!on_harmonic) {
                worst = std::max(worst, bin(y, fold, from));
            }
        }
        floors.push_back(20.0 * std::log10(worst / h1));
        INFO("oversample " << os << ": worst fold " << floors.back() << " dB");
    }
    REQUIRE(floors.size() == 3);
    CHECK(floors[1] < floors[0] - 5.0);
    CHECK(floors[2] < floors[1] - 5.0);
    // Unlike fuzz.h, where 4x came out worse than 2x, the sequence here never reverses — see
    // the header on why that is a data point rather than a coincidence.
}

SCENARIO("unprepared, the voice is silent rather than undefined") {
    O::voice v;
    bool     silent = true;
    for (int i = 0; i < 1000; ++i) {
        silent = silent && (v.process() == 0.0);
    }
    REQUIRE(silent);
}
