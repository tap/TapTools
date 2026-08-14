/// @file
/// @brief      Catch2 scenarios pinning the tap.garden~ kernel (garden.h).
/// @details    The event-level restatement of the family's stability story, pinned the house
///             way: the return grid to the sample, per-pass decay and softening measured on the
///             output (Goertzel partials, YIN pitch for the scale contract), the population
///             bounds (event ring and voice pool), and the full seeded-RNG triad the tr808
///             voices established — same seed bit-exact, different seed different, seed
///             irrelevant while the idle gardener is disabled.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <tap/dsp/yin.h>
#include <taptools/garden.h>

namespace {

    constexpr double k_sr = 48000.0;

    using tap::tools::garden::bed;

    /// A quiet, instrument-neutral bed: idle gardener off, instant level, percussive bell so
    /// grid promises are sharp. Tests opt into slow bells and idling explicitly.
    bed make() {
        bed g;
        g.prepare(k_sr);
        g.set_smooth_ms(0.0);
        g.set_idle_seconds(0.0);
        g.set_bell(0.001, 0.02, 1.0);
        g.set_scale(tap::tools::garden::scale_chromatic);
        return g;
    }

    size_t at(double seconds) {
        return static_cast<size_t>(seconds * k_sr);
    }

    void render(bed& g, std::vector<double>& y) {
        for (auto& s : y) {
            s = g.process();
        }
    }

    double peak(const std::vector<double>& x, size_t begin, size_t end) {
        double p = 0.0;
        for (size_t i = begin; i < end; ++i) {
            p = std::max(p, std::abs(x[i]));
        }
        return p;
    }

    double goertzel(const std::vector<double>& x, double f, size_t begin, size_t end) {
        const double w    = 2.0 * 3.14159265358979323846 * f / k_sr;
        const double coef = 2.0 * std::cos(w);
        double       s1 = 0.0, s2 = 0.0;
        for (size_t i = begin; i < end; ++i) {
            const double s = x[i] + coef * s1 - s2;
            s2             = s1;
            s1             = s;
        }
        const double n  = static_cast<double>(end - begin);
        const double re = s1 - s2 * std::cos(w);
        const double im = s2 * std::sin(w);
        return 2.0 * std::sqrt(re * re + im * im) / n;
    }

    /// YIN oracle at an offset — same detector setup as tune_test.cpp / harmonizer_test.cpp.
    double measure_hz(const std::vector<double>& x, size_t offset) {
        const size_t  tau_min = static_cast<size_t>(k_sr / 2000.0);
        const size_t  tau_max = static_cast<size_t>(std::ceil(k_sr / 55.0));
        tap::dsp::yin det(tau_max, tau_min, tau_max);
        REQUIRE(x.size() >= offset + det.frame_size());
        const auto r = det.analyze(x.data() + offset);
        REQUIRE(r.voiced());
        return k_sr / r.period;
    }

    double cents(double f, double ref) {
        return 1200.0 * std::log2(f / ref);
    }

    double midi_hz(double pitch) {
        return 440.0 * std::exp2((pitch - 69.0) / 12.0);
    }

} // namespace

SCENARIO("a planted note blooms again every loop period") {
    bed g = make();
    g.set_loop_seconds(0.5);
    g.set_decay(0.9);
    g.set_floor(0.001);
    g.set_bell(1e-6, 0.02, 1.0); // instant attack: the envelope is at target one sample in

    g.note(81.0, 0.8); // 880 Hz: the sine leaves the threshold within a few samples
    std::vector<double> y(at(2.0), 0.0);
    render(g, y);

    // The 20 ms bell is ~1e-11 by the next return, so an amplitude threshold separates the
    // return from the previous tail cleanly. The onset detector is a threshold on a sine, so
    // it can sit a few samples into the cycle — the grid claim is "within 8 samples", which at
    // 48 kHz is a sixth of a millisecond.
    const size_t loop = at(0.5);
    REQUIRE(y[0] != 0.0); // the plant sounds on the next processed sample
    for (size_t k = 1; k <= 3; ++k) {
        const double vel   = 0.8 * std::pow(0.9, static_cast<double>(k));
        size_t       onset = 0;
        for (size_t i = k * loop - 1000; i < k * loop + 1000; ++i) {
            if (std::abs(y[i]) > 0.05 * vel) {
                onset = i;
                break;
            }
        }
        INFO("return " << k << " onset at " << onset << ", grid " << k * loop);
        CHECK(onset >= k * loop);
        CHECK(onset < k * loop + 8);
    }
}

SCENARIO("each return is quieter by the decay ratio and the bloom retires below the floor") {
    bed g = make();
    g.set_loop_seconds(0.25);
    g.set_decay(0.5);
    g.set_floor(0.05);

    g.note(69.0, 0.8);
    std::vector<double> y(at(2.5), 0.0);
    render(g, y);

    // Velocity walks 0.8, 0.4, 0.2, 0.1, 0.05 and then retires: five audible returns.
    const size_t loop = at(0.25);
    double       prev = peak(y, 0, loop);
    for (size_t k = 1; k <= 4; ++k) {
        const double p     = peak(y, k * loop, (k + 1) * loop);
        const double ratio = p / prev;
        INFO("return " << k << ": peak " << p << ", ratio " << ratio);
        CHECK(std::abs(ratio - 0.5) < 0.075);
        prev = p;
    }
    REQUIRE(g.active_events() == 0);             // retired below the floor
    REQUIRE(peak(y, 6 * loop, y.size()) < 1e-6); // and audibly gone
}

SCENARIO("each return is purer: the upper chime modes fade by the soften ratio") {
    bed g = make();
    g.set_loop_seconds(0.5);
    g.set_decay(1.0); // hold velocity still so only brightness moves
    g.set_floor(0.001);
    g.set_soften(0.6);
    g.set_bell(0.005, 0.06, 1.0);

    // 440 Hz fundamental; the chime's second mode rings at the free-free bar ratio 2.756.
    g.note(69.0, 0.8);
    std::vector<double> y(at(2.5), 0.0);
    render(g, y);

    const double        mode2 = 440.0 * tap::tools::garden::k_mode_ratio[1];
    const size_t        loop  = at(0.5);
    std::vector<double> tilt;
    for (size_t k = 0; k <= 3; ++k) {
        const double fund = goertzel(y, 440.0, k * loop, k * loop + at(0.2));
        const double side = goertzel(y, mode2, k * loop, k * loop + at(0.2));
        tilt.push_back(side / fund);
        INFO("return " << k << ": mode2/fundamental = " << side / fund);
    }
    for (size_t k = 1; k < tilt.size(); ++k) {
        CHECK(tilt[k] < tilt[k - 1]); // strictly purer every pass
    }
    CHECK(tilt.back() < 0.3 * tilt.front()); // and substantially so over three passes
}

SCENARIO("every bloom lands on the scale") {
    // Off-scale and fractional plants, C major pentatonic: each must sound a scale member.
    const double planted[] = {61.0, 63.4, 66.0, 70.6};
    for (const double pitch : planted) {
        bed g = make();
        g.set_scale(tap::tools::garden::scale_major_pentatonic);
        g.set_root(0);
        g.set_loop_seconds(2.0);
        g.set_bell(0.01, 0.5, 0.4); // gentle brightness: the tail is the fundamental, where yin reads

        g.note(pitch, 0.8);
        std::vector<double> y(at(0.5), 0.0);
        render(g, y);

        const double hz   = measure_hz(y, at(0.1));
        const double midi = 69.0 + 12.0 * std::log2(hz / 440.0);
        const int    pc   = ((static_cast<int>(std::lround(midi)) % 12) + 12) % 12;
        const bool   in_scale =
            (tap::tools::garden::k_scale_masks[tap::tools::garden::scale_major_pentatonic] & (1u << pc)) != 0u;
        INFO("planted " << pitch << " -> sounded " << midi << " (pc " << pc << ")");
        CHECK(in_scale);
        CHECK(std::abs(cents(hz, midi_hz(static_cast<double>(std::lround(midi))))) < 20.0);
    }
}

SCENARIO(
    "the seeded garden is bit-exact per seed, differs across seeds, and the seed cannot matter while idle seeding is off") {
    auto render_idle = [](uint64_t seed) {
        bed g = make();
        g.set_loop_seconds(0.25);
        g.set_idle_seconds(0.5);
        g.set_seed(seed);
        std::vector<double> y(at(3.0), 0.0);
        render(g, y);
        return y;
    };

    const std::vector<double> a = render_idle(1111);
    const std::vector<double> b = render_idle(1111);
    const std::vector<double> c = render_idle(2222);

    bool same = true, differ = false;
    for (size_t i = 0; i < a.size(); ++i) {
        same   = same && (a[i] == b[i]);
        differ = differ || (a[i] != c[i]);
    }
    REQUIRE(same);   // same seed: bit-exact
    REQUIRE(differ); // different seed: a different garden

    // Idle seeding off: the rng is never consumed, so the seed cannot matter at all.
    auto render_planted = [](uint64_t seed) {
        bed g = make();
        g.set_seed(seed);
        g.note(60.0, 0.8);
        std::vector<double> y(at(1.0), 0.0);
        render(g, y);
        return y;
    };
    const std::vector<double> p     = render_planted(1111);
    const std::vector<double> q     = render_planted(2222);
    bool                      exact = true;
    for (size_t i = 0; i < p.size(); ++i) {
        exact = exact && (p[i] == q[i]);
    }
    REQUIRE(exact);
    REQUIRE(peak(p, 0, p.size()) > 0.1); // a real render, not silence agreeing with silence
}

SCENARIO("left alone, the garden starts playing after idle_seconds — and never when idle is disabled") {
    bed g = make();
    g.set_loop_seconds(0.25);
    g.set_idle_seconds(0.5);
    g.set_seed(1111);

    std::vector<double> y(at(4.0), 0.0);
    render(g, y);

    // Deterministic per seed: with seed 1111 the gardener's first plant is a fixed fact.
    REQUIRE(peak(y, 0, at(0.5)) == 0.0); // patient until the threshold
    REQUIRE(peak(y, at(0.5), y.size()) > 0.05);

    bed                 quiet = make(); // idle 0: disabled
    std::vector<double> z(at(4.0), 0.0);
    render(quiet, z);
    REQUIRE(peak(z, 0, z.size()) == 0.0);
}

SCENARIO("when the garden is full the oldest bloom yields to the newest") {
    bed g = make();
    g.set_loop_seconds(1.0);
    g.set_decay(0.99);
    g.set_floor(0.001);

    // The first plant: a high, distinctive bell.
    g.note(96.0, 0.8);
    REQUIRE(g.active_events() == 1);

    // Fill the garden and one more: 64 quiet low plants push the first bloom out.
    std::vector<double> scratch(200, 0.0);
    for (int i = 0; i < tap::tools::garden::k_max_events; ++i) {
        render(g, scratch);
        g.note(45.0, 0.1);
    }
    REQUIRE(g.active_events() == tap::tools::garden::k_max_events);

    // Render across where the first bloom would have returned: its pitch is gone.
    std::vector<double> y(at(1.2), 0.0);
    render(g, y);
    const double high = goertzel(y, midi_hz(96.0), at(0.95), at(1.15));
    INFO("energy at the dropped bloom's pitch: " << high);
    CHECK(high < 0.01);
}

SCENARIO("the bell pool never exceeds its size and stays finite and bounded under heavy stealing") {
    bed g = make();
    g.set_loop_seconds(0.5);
    g.set_decay(0.9);
    g.set_floor(0.001);
    g.set_bell(0.01, 1.5, 1.0); // long ringing bells force constant stealing

    // Plant twice the pool size in quick succession, then let everything recirculate.
    std::vector<double> y;
    y.reserve(at(2.0));
    for (int i = 0; i < 2 * tap::tools::garden::k_voices; ++i) {
        g.note(48.0 + i, 0.9);
        for (int s = 0; s < 400; ++s) {
            y.push_back(g.process());
        }
        REQUIRE(g.active_voices() <= tap::tools::garden::k_voices);
    }
    while (y.size() < at(2.0)) {
        y.push_back(g.process());
    }

    // The hard bound is structural: k_voices bells, each |env * sin| <= 1, level 1.
    double worst  = 0.0;
    bool   finite = true;
    for (const double v : y) {
        worst  = std::max(worst, std::abs(v));
        finite = finite && std::isfinite(v);
    }
    INFO("peak under heavy stealing: " << worst);
    REQUIRE(finite);
    REQUIRE(worst <= static_cast<double>(tap::tools::garden::k_voices));
    REQUIRE(g.active_voices() <= tap::tools::garden::k_voices);
}

SCENARIO("unprepared, the garden is silent") {
    bed g;
    g.note(60.0, 1.0); // a safe no-op before prepare
    REQUIRE(g.process() == 0.0);
    REQUIRE(g.active_events() == 0);
}
