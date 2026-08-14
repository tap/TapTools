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
    using tap::tools::garden::k_mode_ratio;
    using tap::tools::garden::material_bar;
    using tap::tools::garden::material_chime;

    /// A quiet, instrument-neutral bed: idle gardener off, instant level, percussive bell so
    /// grid promises are sharp, spread 0 so mono measurements read either bus. Tests opt into
    /// slow bells, idling, and stereo explicitly.
    bed make() {
        bed g;
        g.prepare(k_sr);
        g.set_smooth_ms(0.0);
        g.set_idle_seconds(0.0);
        g.set_spread(0.0);
        g.set_bell(0.001, 0.02, 1.0);
        g.set_scale(tap::tools::garden::scale_chromatic);
        return g;
    }

    size_t at(double seconds) {
        return static_cast<size_t>(seconds * k_sr);
    }

    /// Mono render onto the left bus — every make() bed has spread 0, where left == right.
    void render(bed& g, std::vector<double>& y) {
        for (auto& s : y) {
            double right = 0.0;
            g.process(s, right);
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

    // Velocity walks 0.8, 0.4, 0.2, 0.1, 0.05 and then retires: five audible returns. The
    // decay ratio is pinned on the fundamental — the whole strike fades a shade faster than
    // velocity because quieter returns are also duller (the hardness coupling, pinned in its
    // own scenario below).
    const size_t loop = at(0.25);
    double       prev = goertzel(y, 440.0, 0, loop);
    for (size_t k = 1; k <= 4; ++k) {
        const double p     = goertzel(y, 440.0, k * loop, (k + 1) * loop);
        const double ratio = p / prev;
        INFO("return " << k << ": fundamental " << p << ", ratio " << ratio);
        CHECK(std::abs(ratio - 0.5) < 0.05);
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

    const double        mode2 = 440.0 * k_mode_ratio[material_chime][1];
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

SCENARIO("the strike carries a tick that the returns lose") {
    bed g = make();
    g.set_loop_seconds(1.0);
    g.set_decay(1.0); // hold velocity: only soften moves the tick
    g.set_floor(0.001);
    g.set_soften(0.5);
    g.set_bell(0.001, 1.0, 1.0);

    // The 4th bar mode (8.933f, ~3930 Hz here) is the contact tick: present at the strike,
    // gone in tens of milliseconds (haste ~ f^2), and fading as b^3 across returns.
    g.note(69.0, 0.9);
    std::vector<double> y(at(2.5), 0.0);
    render(g, y);

    const double tick      = 440.0 * k_mode_ratio[material_chime][3];
    const double at_strike = goertzel(y, tick, 0, at(0.05));
    const double late      = goertzel(y, tick, at(0.2), at(0.4));
    INFO("tick at strike " << at_strike << ", 200-400 ms later " << late);
    CHECK(at_strike > 10.0 * late); // confined to the contact

    const double tilt_0 = at_strike / goertzel(y, 440.0, 0, at(0.05));
    const double tilt_2 = goertzel(y, tick, 2 * at(1.0), 2 * at(1.0) + at(0.05))
                          / goertzel(y, 440.0, 2 * at(1.0), 2 * at(1.0) + at(0.05));
    INFO("tick/fundamental at strike " << tilt_0 << ", at return 2 " << tilt_2);
    CHECK(tilt_0 > 10.0 * tilt_2); // b^3: the returns lose their attack first
}

SCENARIO("the tail beats: a struck tube is a doublet, not a lab sine") {
    bed g = make();
    g.set_loop_seconds(4.0);
    g.set_bell(0.001, 4.0, 0.0); // brightness 0: the fundamental pair alone

    g.note(69.0, 0.8);
    std::vector<double> y(at(3.0), 0.0);
    render(g, y);

    // The pair starts aligned at the strike and beats at delta-f; the fundamental dips near
    // the half-period null and recovers by the full period — a beat, not a decay.
    const double split   = std::exp2(tap::tools::garden::k_doublet_cents / 2400.0);
    const double df      = 440.0 * (split - 1.0 / split);
    const size_t null_at = at(0.5 / df);
    const double m0      = goertzel(y, 440.0, at(0.1), at(0.3));
    const double m1      = goertzel(y, 440.0, null_at - at(0.1), null_at + at(0.1));
    const double m2      = goertzel(y, 440.0, 2 * null_at - at(0.1), 2 * null_at + at(0.1));
    INFO("fundamental early " << m0 << ", at the beat null " << m1 << ", recovered " << m2);
    REQUIRE(std::isfinite(m1));
    CHECK(m1 < 0.3 * m0); // dips far below what the envelope alone would do
    CHECK(m2 > 2.0 * m1); // and comes back: a beat, not a decay
}

SCENARIO("a soft strike is duller than a hard one") {
    auto tilt_at = [](double velocity) {
        bed g = make();
        g.set_loop_seconds(2.0);
        g.set_bell(0.001, 0.5, 1.0);
        g.note(69.0, velocity);
        std::vector<double> y(at(0.2), 0.0);
        render(g, y);
        return goertzel(y, 440.0 * k_mode_ratio[material_chime][1], 0, at(0.15)) / goertzel(y, 440.0, 0, at(0.15));
    };

    const double hard = tilt_at(0.9);
    const double soft = tilt_at(0.25);
    INFO("mode2/fundamental: hard strike " << hard << ", soft strike " << soft);
    CHECK(hard > 1.3 * soft); // hardness couples brightness to velocity
}

SCENARIO("small high tubes ring shorter than long low ones") {
    auto retention = [](double pitch) {
        bed g = make();
        g.set_loop_seconds(2.0);
        g.set_bell(0.001, 1.0, 0.0); // the fundamental alone
        g.note(pitch, 0.8);
        std::vector<double> y(at(1.0), 0.0);
        render(g, y);
        const double f = midi_hz(pitch);
        return goertzel(y, f, at(0.8), at(1.0)) / goertzel(y, f, 0, at(0.2));
    };

    const double low  = retention(48.0); // ~131 Hz: ring time scaled up
    const double high = retention(84.0); // ~1047 Hz: scaled down
    INFO("late/early fundamental: low tube " << low << ", high tube " << high);
    CHECK(low > 1.5 * high);
}

SCENARIO("the wind arrives in gusts, and calm wind strikes singly") {
    auto onsets = [](double gust) {
        bed g = make();
        g.set_loop_seconds(1.0);
        g.set_idle_seconds(0.2);
        g.set_seed(7);
        g.set_gust(gust);
        g.set_decay(0.0);             // plants retire after one sounding: only the wind counts
        g.set_bell(0.001, 0.02, 1.0); // fast pings so strikes are separable
        std::vector<double> y(at(12.0), 0.0);
        render(g, y);
        std::vector<size_t> found;
        for (size_t i = at(0.01); i < y.size(); ++i) {
            if (std::abs(y[i]) > 0.02 && peak(y, i - at(0.01), i) < 0.02) {
                found.push_back(i);
            }
        }
        return found;
    };

    const std::vector<size_t> blustery = onsets(1.0);
    const std::vector<size_t> calm     = onsets(0.0);
    INFO("strikes: blustery " << blustery.size() << ", calm " << calm.size());
    REQUIRE(blustery.size() >= 4);
    REQUIRE(calm.size() >= 3);

    bool clustered = false;
    for (size_t i = 1; i < blustery.size(); ++i) {
        clustered = clustered || (blustery[i] - blustery[i - 1] < at(0.35));
    }
    CHECK(clustered); // at gust 1, some strikes tumble within a gust

    bool evenly = true;
    for (size_t i = 1; i < calm.size(); ++i) {
        evenly = evenly && (calm[i] - calm[i - 1] >= at(0.45));
    }
    CHECK(evenly); // at gust 0, single strikes separated by at least the minimum calm
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
            double left  = 0.0;
            double right = 0.0;
            g.process(left, right);
            y.push_back(left);
        }
        REQUIRE(g.active_voices() <= tap::tools::garden::k_voices);
    }
    while (y.size() < at(2.0)) {
        double left  = 0.0;
        double right = 0.0;
        g.process(left, right);
        y.push_back(left);
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
    double left  = 1.0;
    double right = 1.0;
    g.process(left, right);
    REQUIRE(left == 0.0);
    REQUIRE(right == 0.0);
    REQUIRE(g.active_events() == 0);
}

SCENARIO("material re-voices the rack: chime partials at tube ratios, bar partials at double octaves") {
    // The same strike through both mode tables: each material's second partial carries the
    // energy and the other material's slot is empty (2.756f vs 4f — 1213 Hz vs 1760 Hz here).
    auto probe = [](int material) {
        bed g = make();
        g.set_loop_seconds(2.0);
        g.set_material(material);
        g.set_bell(0.001, 0.5, 1.0);
        g.note(69.0, 0.9);
        std::vector<double> y(at(0.2), 0.0);
        render(g, y);
        const double chime2 = goertzel(y, 440.0 * k_mode_ratio[material_chime][1], 0, at(0.15));
        const double bar2   = goertzel(y, 440.0 * k_mode_ratio[material_bar][1], 0, at(0.15));
        return std::make_pair(chime2, bar2);
    };

    const auto [chime_on_chime, bar_on_chime] = probe(material_chime);
    const auto [chime_on_bar, bar_on_bar]     = probe(material_bar);
    INFO("chime material: 2.756f " << chime_on_chime << ", 4f " << bar_on_chime);
    INFO("bar material:   2.756f " << chime_on_bar << ", 4f " << bar_on_bar);
    CHECK(chime_on_chime > 5.0 * bar_on_chime);
    CHECK(bar_on_bar > 5.0 * chime_on_bar);
}

SCENARIO("each tube's upper modes sit a fixed few cents off — the same cents every strike, different per tube") {
    // The scatter is a property of the tube, not the strike: a stateless hash of (pitch, mode),
    // so the detune is bounded by k_scatter_cents, identical across independent instances, and
    // different from tube to tube. Measured by scanning a Goertzel probe across the second
    // mode's neighborhood (the scan resolves ~0.25 cents on this window).
    auto detune_of = [](double pitch) {
        bed g = make();
        g.set_loop_seconds(2.0);
        g.set_bell(0.001, 0.5, 1.0);
        g.note(pitch, 0.9);
        std::vector<double> y(at(0.25), 0.0);
        render(g, y);
        const double ideal      = midi_hz(pitch) * k_mode_ratio[material_chime][1];
        double       best_cents = 0.0;
        double       best       = 0.0;
        for (int q = -24; q <= 24; ++q) {
            const double c = 0.25 * static_cast<double>(q);
            const double m = goertzel(y, ideal * std::exp2(c / 1200.0), at(0.01), at(0.2));
            if (m > best) {
                best       = m;
                best_cents = c;
            }
        }
        return best_cents;
    };

    const double a      = detune_of(69.0);
    const double a_gain = detune_of(69.0); // an independent instance: the same tube, the same flaw
    const double b      = detune_of(84.0);
    INFO("mode-2 detune: tube 69 " << a << " cents (again " << a_gain << "), tube 84 " << b);
    CHECK(a == a_gain);
    CHECK(std::abs(a) < tap::tools::garden::k_scatter_cents + 0.5);
    CHECK(std::abs(b) < tap::tools::garden::k_scatter_cents + 0.5);
    CHECK(std::abs(a - b) > 1.0); // a different tube is differently imperfect
    // And the fundamental stays true: a maker tunes the fundamental (pinned via yin elsewhere;
    // here, the scattered mode still beats around an in-tune first mode).
    bed g = make();
    g.set_loop_seconds(2.0);
    g.set_bell(0.01, 0.5, 0.4);
    g.note(69.0, 0.8);
    std::vector<double> y(at(0.5), 0.0);
    render(g, y);
    CHECK(std::abs(cents(measure_hz(y, at(0.1)), 440.0)) < 5.0);
}

SCENARIO("the rack is stereo: every tube keeps its seat, and spread 0 collapses to mono, bitwise") {
    // spread 0: the two busses are bitwise identical (the sqrt equal-power seat at pan 0).
    {
        bed g = make();
        g.note(69.0, 0.8);
        bool same = true;
        for (size_t i = 0; i < at(0.5); ++i) {
            double left  = 0.0;
            double right = 0.0;
            g.process(left, right);
            same = same && (left == right);
        }
        REQUIRE(same);
    }

    // spread up: a tube's seat is a fixed property of its pitch — the same left/right balance
    // in every independent instance, and different tubes hang in different places.
    auto seat_of = [](double pitch) {
        bed g = make();
        g.set_spread(1.0);
        g.set_loop_seconds(2.0);
        g.note(pitch, 0.8);
        double energy_l = 0.0;
        double energy_r = 0.0;
        for (size_t i = 0; i < at(0.5); ++i) {
            double left  = 0.0;
            double right = 0.0;
            g.process(left, right);
            energy_l += left * left;
            energy_r += right * right;
        }
        return energy_r / (energy_l + energy_r);
    };

    const double a      = seat_of(69.0);
    const double a_gain = seat_of(69.0);
    const double b      = seat_of(67.0);
    INFO("right-bus energy share: tube 69 " << a << " (again " << a_gain << "), tube 67 " << b);
    REQUIRE(a == a_gain);            // the seat is the tube's, deterministically
    CHECK(std::abs(a - b) > 0.1);    // a different tube hangs somewhere else
    CHECK(std::abs(a - 0.5) > 0.05); // and a full-spread seat is audibly off center
}
