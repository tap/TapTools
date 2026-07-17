/// @file
/// @brief      Unit tests for the TB-303 voice kernel (taptools::tb303::voice).
/// @details    Pins the slice-2 voice behaviors: gate/retrigger/legato-slide semantics (the
///             melodic-voice contract), pitch and tuning accuracy, the envelope-modulated
///             cutoff sweep and its decay-knob scaling, the fixed VCA envelope, the slice-2
///             accent scope (louder + faster MEG — the C13 sweep is slice 3), waveform
///             switching, determinism, and silence hygiene.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/tb303_voice.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    namespace tb = taptools::tb303;

    tb::voice make() {
        tb::voice v;
        v.prepare(k_sr);
        v.set_smooth_ms(0.0);
        return v;
    }

    std::vector<double> render(tb::voice& v, double seconds) {
        std::vector<double> y(static_cast<size_t>(seconds * k_sr));
        v.process(y.data(), y.size());
        return y;
    }

    double rms(const std::vector<double>& y, size_t lo, size_t hi) {
        double acc = 0.0;
        size_t n   = 0;
        for (size_t i = lo; i < hi && i < y.size(); ++i, ++n) {
            acc += y[i] * y[i];
        }
        return n ? std::sqrt(acc / n) : 0.0;
    }

    size_t at_s(double seconds) {
        return static_cast<size_t>(seconds * k_sr);
    }

    // Goertzel magnitude at one frequency (Hann-windowed) over [lo, hi).
    double level_at(const std::vector<double>& x, size_t lo, size_t hi, double f) {
        const double w  = 2.0 * k_pi * f / k_sr;
        const double c  = 2.0 * std::cos(w);
        double       s1 = 0.0, s2 = 0.0;
        const size_t n = hi - lo;
        for (size_t i = lo; i < hi && i < x.size(); ++i) {
            const double win = 0.5 - 0.5 * std::cos(2.0 * k_pi * (i - lo) / (n - 1));
            const double s0  = x[i] * win + c * s1 - s2;
            s2               = s1;
            s1               = s0;
        }
        const double re = s1 - s2 * std::cos(w);
        const double im = s2 * std::sin(w);
        return std::sqrt(re * re + im * im);
    }

    // Fundamental via averaged rising-zero-crossing periods over [lo, hi).
    double measure_f0(const std::vector<double>& y, size_t lo, size_t hi) {
        double first = -1.0, last = -1.0;
        int    count = 0;
        for (size_t i = lo + 1; i < hi && i < y.size(); ++i) {
            if (y[i - 1] <= 0.0 && y[i] > 0.0) {
                const double frac = y[i - 1] / (y[i - 1] - y[i]);
                const double t    = (static_cast<double>(i - 1) + frac) / k_sr;
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

    double midi_hz(double n) {
        return 440.0 * std::exp2((n - 69.0) / 12.0);
    }

} // namespace

SCENARIO("the voice is silent until gated, sounds while held, and chops at gate-off") {
    auto v = make();

    auto idle = render(v, 0.25);
    REQUIRE(rms(idle, 0, idle.size()) == 0.0);

    v.note_on(45.0);
    auto held = render(v, 0.5);
    REQUIRE(rms(held, at_s(0.05), at_s(0.4)) > 0.01);

    v.note_off();
    auto off = render(v, 0.25);
    REQUIRE(rms(off, at_s(0.05), at_s(0.25)) < 1e-4); // the ~2 ms release has long finished
}

SCENARIO("pitch and tuning are accurate") {
    auto v = make();
    v.set_cutoff(tb::k_cutoff_max); // open the filter so the saw's crossings read cleanly
    v.set_envmod(0.0);
    v.set_resonance(0.0);

    v.note_on(45.0); // A1 = 110 Hz
    auto y = render(v, 1.0);
    REQUIRE(std::abs(measure_f0(y, at_s(0.3), at_s(1.0)) / midi_hz(45.0) - 1.0) < 0.01);

    v.set_tuning(100.0); // +1 semitone
    v.note_off();
    render(v, 0.1);
    v.note_on(45.0);
    auto y2 = render(v, 1.0);
    REQUIRE(std::abs(measure_f0(y2, at_s(0.3), at_s(1.0)) / midi_hz(46.0) - 1.0) < 0.01);
}

SCENARIO("a held pitch change slides (~60 ms RC, no retrigger); a retrigger snaps") {
    GIVEN("a legato pitch change while the gate is held") {
        auto v = make();
        v.set_cutoff(tb::k_cutoff_max);
        v.set_envmod(0.0);
        v.note_on(45.0);
        render(v, 0.5);
        v.set_pitch(52.0); // up a fifth, gate still on

        auto y = render(v, 1.0);
        THEN("mid-glide the pitch sits strictly between the notes") {
            const double mid = measure_f0(y, at_s(0.02), at_s(0.06));
            REQUIRE(mid > midi_hz(45.5));
            REQUIRE(mid < midi_hz(51.5));
        }
        THEN("it settles on the target") {
            REQUIRE(std::abs(measure_f0(y, at_s(0.5), at_s(1.0)) / midi_hz(52.0) - 1.0) < 0.01);
        }
        THEN("the amplitude never dips to a retrigger notch") {
            REQUIRE(rms(y, 0, at_s(0.05)) > 0.01);
        }
    }
    GIVEN("a retrigger (gate released first)") {
        auto v = make();
        v.set_cutoff(tb::k_cutoff_max);
        v.set_envmod(0.0);
        v.note_on(45.0);
        render(v, 0.3);
        v.note_off();
        render(v, 0.05);
        v.note_on(52.0);
        auto y = render(v, 0.3);
        THEN("the new pitch is there immediately — no glide tail") {
            REQUIRE(std::abs(measure_f0(y, at_s(0.03), at_s(0.15)) / midi_hz(52.0) - 1.0) < 0.02);
        }
    }
}

SCENARIO("envmod sweeps the cutoff with the MEG, and the decay knob sets how fast it closes") {
    // Brightness proxy: 8th-harmonic level early (just after the attack) vs late.
    auto brightness_drop = [](double envmod, double decay_ms) {
        auto v = make();
        v.set_cutoff(300.0);
        v.set_envmod(envmod);
        v.set_decay(decay_ms);
        v.set_resonance(0.0);
        v.note_on(45.0);
        auto         y     = render(v, 1.5);
        const double h8    = 8.0 * midi_hz(45.0);
        const double early = level_at(y, at_s(0.02), at_s(0.12), h8);
        const double late  = level_at(y, at_s(1.3), at_s(1.4), h8);
        return early / (late + 1e-12);
    };

    THEN("with envmod up, the note opens bright and closes dark") {
        REQUIRE(brightness_drop(1.0, 300.0) > 10.0);
    }
    THEN("with envmod off, brightness barely moves") {
        REQUIRE(brightness_drop(0.0, 300.0) < 3.0);
    }
    THEN("a long decay holds the sweep open longer than a short one") {
        auto h8_at = [](double decay_ms) {
            auto v = make();
            v.set_cutoff(300.0);
            v.set_envmod(1.0);
            v.set_decay(decay_ms);
            v.note_on(45.0);
            auto y = render(v, 0.8);
            return level_at(y, at_s(0.55), at_s(0.75), 8.0 * midi_hz(45.0));
        };
        REQUIRE(h8_at(tb::k_decay_max_ms) > 4.0 * h8_at(tb::k_decay_min_ms));
    }
}

SCENARIO("the VCA envelope decays to silence on a held note — no sustain, like the hardware") {
    auto v = make();
    v.set_cutoff(2000.0);
    v.set_envmod(0.0);
    v.note_on(45.0);
    auto y = render(v, 5.0);
    // decay tau = 1.23 s: by 4.5 s the level is ~2.6% of the early peak
    const double early = rms(y, at_s(0.05), at_s(0.3));
    const double late  = rms(y, at_s(4.5), at_s(5.0));
    REQUIRE(early > 0.01);
    REQUIRE(late < 0.12 * early);
    REQUIRE(late > 0.0); // still a held note, not hard-gated off
}

SCENARIO("accent (slice-2 scope): an accented note is louder and its MEG closes faster") {
    auto play = [](double depth) {
        auto v = make();
        v.set_cutoff(300.0);
        v.set_envmod(1.0);
        v.set_decay(tb::k_decay_max_ms);
        v.set_accent(1.0);
        v.note_on(45.0, depth);
        return render(v, 1.0);
    };
    auto plain    = play(0.0);
    auto accented = play(1.0);

    THEN("louder") {
        REQUIRE(rms(accented, at_s(0.02), at_s(0.2)) > 1.5 * rms(plain, at_s(0.02), at_s(0.2)));
    }
    THEN("the sweep closes on the accent clock (~200 ms), not the decay knob (2 s)") {
        const double h8            = 8.0 * 110.0;
        const double plain_late    = level_at(plain, at_s(0.6), at_s(0.8), h8);
        const double accented_late = level_at(accented, at_s(0.6), at_s(0.8), h8);
        // normalize for the accent loudness before comparing brightness
        const double gain_ratio = rms(accented, at_s(0.02), at_s(0.2)) / rms(plain, at_s(0.02), at_s(0.2));
        REQUIRE(accented_late / gain_ratio < 0.5 * plain_late);
    }
}

SCENARIO("waveform switches between saw and square") {
    auto h2_ratio = [](int wave) {
        auto v = make();
        v.set_cutoff(tb::k_cutoff_max);
        v.set_envmod(0.0);
        v.set_waveform(wave);
        v.note_on(45.0);
        auto         y  = render(v, 1.0);
        const double f0 = midi_hz(45.0);
        return level_at(y, at_s(0.3), at_s(0.9), 2.0 * f0) / (level_at(y, at_s(0.3), at_s(0.9), f0) + 1e-12);
    };
    // the saw has a strong 2nd harmonic; the 50% square suppresses it
    REQUIRE(h2_ratio(tb::wave_saw) > 8.0 * h2_ratio(tb::wave_square));
}

SCENARIO("the voice is deterministic and its tails are clean") {
    auto run = [] {
        auto v = make();
        v.set_resonance(1.0);
        v.set_envmod(0.8);
        v.note_on(45.0, 1.0);
        auto a = render(v, 0.4);
        v.set_pitch(50.0);
        auto b = render(v, 0.4);
        v.note_off();
        auto c = render(v, 0.4);
        a.insert(a.end(), b.begin(), b.end());
        a.insert(a.end(), c.begin(), c.end());
        return a;
    };
    const auto x = run();
    const auto y = run();
    REQUIRE(x == y);
    for (double s : x) {
        REQUIRE(std::isfinite(s));
        REQUIRE(std::abs(s) < 4.0);
    }

    auto v = make();
    v.note_on(45.0);
    render(v, 0.2);
    v.note_off();
    auto tail = render(v, 3.0);
    REQUIRE(std::abs(tail.back()) < 1e-12);
}
