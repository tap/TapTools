/// @file
/// @brief      Catch2 scenarios pinning the tap.scrub~ kernel (scrub.h).
/// @details    Everything this object does is a departure from a plain delay — a moving position,
///             a transposition that does not move with it, a frozen tape, a scattered origin —
///             and a departure is only trustworthy if the identity is exact when it should be. So
///             the load-bearing scenario is the null: at unity pitch, overlap 2, no spray and a
///             held whole-sample position, the scrub is the input delayed and nothing else. The
///             rest pin the departures one at a time, and the seed contract the family shares.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/scrub.h>

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    namespace S = tap::tools::scrub;

    double ms_of(long samples) {
        return static_cast<double>(samples) * 1000.0 / k_sr;
    }

    /// A pluck train: transients, which is what a scrub has something to bite on (the stammer's
    /// material contract, same reasoning).
    std::vector<double> plucks(size_t n) {
        std::vector<double> x(n);
        double              env = 0.0, phase = 0.0, hz = 220.0;
        for (size_t i = 0; i < n; ++i) {
            if (i % 6000 == 0) {
                env = 1.0;
                hz  = 180.0 + 40.0 * static_cast<double>((i / 6000) % 5);
            }
            env *= 0.99975;
            phase += hz / k_sr;
            phase -= std::floor(phase);
            x[i] = env * (std::sin(2.0 * k_pi * phase) + 0.4 * std::sin(6.0 * k_pi * phase));
        }
        return x;
    }

    double rms(const std::vector<double>& x, size_t from) {
        double s = 0.0;
        for (size_t i = from; i < x.size(); ++i) {
            s += x[i] * x[i];
        }
        return std::sqrt(s / static_cast<double>(x.size() - from));
    }

    double bin(const std::vector<double>& x, double hz, size_t from, size_t n) {
        const double w  = 2.0 * k_pi * hz / k_sr;
        const double c  = 2.0 * std::cos(w);
        double       s1 = 0.0, s2 = 0.0;
        for (size_t i = 0; i < n; ++i) {
            const double s = x[from + i] + c * s1 - s2;
            s2             = s1;
            s1             = s;
        }
        return std::sqrt(std::max(0.0, s1 * s1 + s2 * s2 - c * s1 * s2)) * 2.0 / static_cast<double>(n);
    }

    S::machine make(double size_samples = 96.0, double lag_samples = 480.0) {
        S::machine m;
        m.prepare(k_sr, 2000.0);
        m.set_smooth_ms(0.0);
        m.set_overlap(2);
        m.set_size_ms(ms_of(static_cast<long>(size_samples)));
        m.set_position_ms(ms_of(static_cast<long>(lag_samples)));
        m.set_mix(100.0);
        m.set_level(1.0);
        return m;
    }

} // namespace

// The load-bearing scenario. Hann satisfies the overlap-add condition at hop = size/2, so with the
// position held on a whole sample and no transposition, the two live grains sum to exactly one and
// the machine is a delay line — to within the floating-point cost of computing two cosines whose
// arguments differ by pi rather than one cosine and its negation.
SCENARIO("held still at unity pitch, the scrub is exactly a delay") {
    S::machine m = make(96.0, 480.0);

    const std::vector<double> x = plucks(30000);
    std::vector<double>       y(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        y[i] = m.process(x[i]);
    }

    double worst = 0.0;
    for (size_t i = 2000; i < x.size(); ++i) {
        worst = std::max(worst, std::abs(y[i] - x[i - 480]));
    }
    INFO("largest departure from a 480-sample delay: " << worst);
    REQUIRE(worst < 1e-12);
}

SCENARIO("the position is a lag in milliseconds, measured from the live edge") {
    for (long lag : {0L, 240L, 4800L}) {
        S::machine                m = make(96.0, static_cast<double>(lag));
        const std::vector<double> x = plucks(20000);
        std::vector<double>       y(x.size());
        for (size_t i = 0; i < x.size(); ++i) {
            y[i] = m.process(x[i]);
        }
        double worst = 0.0;
        for (size_t i = static_cast<size_t>(lag) + 2000; i < x.size(); ++i) {
            worst = std::max(worst, std::abs(y[i] - x[i - static_cast<size_t>(lag)]));
        }
        INFO("lag " << lag << " samples: worst error " << worst);
        CHECK(worst < 1e-12);
    }
}

// The object's reason to exist: pitch and position are two hands, not one.
SCENARIO("pitch transposes without the position moving") {
    auto run = [](double semitones) {
        S::machine m = make(4800.0, 24000.0); // a long grain and a deep position: room to transpose
        m.set_pitch(semitones);
        std::vector<double> y(static_cast<size_t>(k_sr * 2.0));
        for (size_t i = 0; i < y.size(); ++i) {
            y[i] = m.process(0.5 * std::sin(2.0 * k_pi * 300.0 * static_cast<double>(i) / k_sr));
        }
        const size_t from = y.size() / 2;
        const size_t n    = y.size() - from;
        return std::pair{bin(y, 300.0, from, n), bin(y, 300.0 * std::exp2(semitones / 12.0), from, n)};
    };

    const auto up = run(7.0);
    INFO("up a fifth: energy at 300 Hz " << up.first << ", at the transposed pitch " << up.second);
    CHECK(up.second > 4.0 * up.first);

    const auto down = run(-5.0);
    INFO("down a fourth: energy at 300 Hz " << down.first << ", at the transposed pitch " << down.second);
    CHECK(down.second > 4.0 * down.first);

    const auto flat = run(0.0);
    CHECK(flat.first > 0.4); // and at unity the pitch is simply the input's
}

// The defect this scenario exists to catch, because the first cut had it and it is invisible to
// every other test here: if grain origins are anchored at the position, they advance at the WRITE
// head's speed while each grain plays at `rate`, so the transposition applies only inside a grain
// and the average read rate comes back to 1. A steady tone then comes out at its ORIGINAL pitch
// with a comb of grain-rate sidebands, and the pitch knob does nothing but add texture. The fix is
// a phase-continuous read head, wrapped back toward the position only when it has wandered far
// enough; this pins the outcome rather than the mechanism.
SCENARIO("the pitch control moves the pitch, and does not merely colour the original") {
    auto run = [](double f0, double st) {
        S::machine m = make(4800.0, 43200.0); // 100 ms grains, the position a comfortable 900 ms back
        m.set_pitch(st);
        std::vector<double> y(static_cast<size_t>(k_sr * 2.0));
        for (size_t i = 0; i < y.size(); ++i) {
            y[i] = m.process(0.5 * std::sin(2.0 * k_pi * f0 * static_cast<double>(i) / k_sr));
        }
        return y;
    };

    double sum_moved = 0.0;
    int    count     = 0;
    // Fundamentals deliberately chosen NOT to be whole numbers of periods in the grain: that case
    // is transparent to every wrap and would hide the defect completely.
    for (double f0 : {173.0, 311.0, 443.0}) {
        for (double st : {-12.0, -7.0, -3.0, 3.0, 7.0, 12.0}) {
            const std::vector<double> y      = run(f0, st);
            const size_t              from   = y.size() / 2;
            const size_t              n      = y.size() - from;
            const double              target = f0 * std::exp2(st / 12.0);
            double                    moved  = 0.0;
            for (int k = -6; k <= 6; ++k) { // the comb is a few Hz wide; a single bin misses it
                moved = std::max(moved, bin(y, target + 0.5 * k, from, n));
            }
            const double stayed = bin(y, f0, from, n);
            INFO(f0 << " Hz, " << st << " semitones: energy at the transposed pitch " << moved
                    << ", left at the original " << stayed);
            CHECK(moved > 10.0 * stayed); // the pitch moved; it was not merely coloured
            sum_moved += moved;
            ++count;
        }
    }
    // And it is not merely quiet everywhere. Note the probe: a single-bin reading UNDERSTATES
    // this object badly, because the wraps spread the transposed partial into a narrow comb a few
    // Hz wide rather than moving it — an early version of this scenario measured one bin, read
    // 0.02 where the real figure was 0.43, and nearly sent a correct kernel back for repair. The
    // band figures are in the header and in notebooks/scrub.ipynb; here the bin is widened just
    // enough to catch the comb.
    INFO("mean energy at the transposed pitch: " << sum_moved / count);
    CHECK(sum_moved / count > 0.3);
}

SCENARIO("freeze stops the recorder, so the scrub holds while the input moves on") {
    S::machine m = make(4800.0, 24000.0);

    // Half a second of tone, then freeze and switch the input to silence.
    for (int i = 0; i < static_cast<int>(k_sr * 0.5); ++i) {
        m.process(0.5 * std::sin(2.0 * k_pi * 300.0 * i / k_sr));
    }
    m.set_freeze(true);

    std::vector<double> y(static_cast<size_t>(k_sr * 1.0));
    for (size_t i = 0; i < y.size(); ++i) {
        y[i] = m.process(0.0); // nothing going in at all
    }
    INFO("output rms with a frozen tape and a silent input: " << rms(y, y.size() / 2));
    CHECK(rms(y, y.size() / 2) > 0.1);

    // Unfrozen, with the input still silent, the tape fills with silence and the scrub follows it.
    m.set_freeze(false);
    std::vector<double> z(static_cast<size_t>(k_sr * 3.0));
    for (size_t i = 0; i < z.size(); ++i) {
        z[i] = m.process(0.0);
    }
    INFO("output rms once the recorder is running again: " << rms(z, z.size() * 3 / 4));
    CHECK(rms(z, z.size() * 3 / 4) < 1e-9);
}

SCENARIO("drift walks the playhead through the tape on its own") {
    S::machine still  = make(2400.0, 24000.0);
    S::machine moving = make(2400.0, 24000.0);
    moving.set_drift(0.5);

    const std::vector<double> x = plucks(static_cast<size_t>(k_sr * 2.0));
    std::vector<double>       a(x.size()), b(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        a[i] = still.process(x[i]);
        b[i] = moving.process(x[i]);
    }
    double diff = 0.0;
    for (size_t i = x.size() / 2; i < x.size(); ++i) {
        diff = std::max(diff, std::abs(a[i] - b[i]));
    }
    INFO("largest difference a drift of 0.5 makes: " << diff);
    CHECK(diff > 1e-3);

    // Drift 0 changes nothing at all — the accumulator must not creep.
    S::machine zero = make(2400.0, 24000.0);
    zero.set_drift(0.0);
    bool       same = true;
    S::machine ref  = make(2400.0, 24000.0);
    for (size_t i = 0; i < x.size(); ++i) {
        same = same && (zero.process(x[i]) == ref.process(x[i]));
    }
    REQUIRE(same);
}

// The family's seed contract, same shape as garden.h and stammer.h: the dice are only rolled when
// something is actually random, so with spray off the seed provably cannot matter.
SCENARIO("with spray off the seed cannot matter, and with it on the seed is the performance") {
    const std::vector<double> x = plucks(static_cast<size_t>(k_sr * 1.0));

    S::machine a = make(2400.0, 12000.0);
    S::machine b = make(2400.0, 12000.0);
    a.set_seed(1);
    b.set_seed(999999);
    bool identical = true;
    for (size_t i = 0; i < x.size(); ++i) {
        identical = identical && (a.process(x[i]) == b.process(x[i]));
    }
    REQUIRE(identical); // spray defaults to 0: no draw, no divergence

    S::machine c = make(2400.0, 12000.0);
    S::machine d = make(2400.0, 12000.0);
    c.set_spray_ms(120.0);
    d.set_spray_ms(120.0);
    c.set_seed(1);
    d.set_seed(999999);
    double diverged = 0.0;
    for (size_t i = 0; i < x.size(); ++i) {
        diverged = std::max(diverged, std::abs(c.process(x[i]) - d.process(x[i])));
    }
    INFO("two seeds, sprayed: largest divergence " << diverged);
    CHECK(diverged > 1e-3);

    // And a seed is replayable: the same seed twice is the same performance, bitwise.
    S::machine e = make(2400.0, 12000.0);
    S::machine f = make(2400.0, 12000.0);
    e.set_spray_ms(120.0);
    f.set_spray_ms(120.0);
    e.set_seed(4242);
    f.set_seed(4242);
    bool replayed = true;
    for (size_t i = 0; i < x.size(); ++i) {
        replayed = replayed && (e.process(x[i]) == f.process(x[i]));
    }
    REQUIRE(replayed);
}

SCENARIO("the level holds across overlap settings") {
    std::vector<double> level;
    for (int n : {2, 3, 4}) {
        S::machine m = make(4800.0, 24000.0);
        m.set_overlap(n);
        std::vector<double> y(static_cast<size_t>(k_sr * 1.5));
        for (size_t i = 0; i < y.size(); ++i) {
            y[i] = m.process(0.5 * std::sin(2.0 * k_pi * 300.0 * static_cast<double>(i) / k_sr));
        }
        level.push_back(rms(y, y.size() / 2));
        INFO("overlap " << n << ": rms " << level.back() << ", normalization " << m.grains().normalization());
    }
    for (double v : level) {
        CHECK(std::abs(v / level[0] - 1.0) < 0.02);
    }
}

SCENARIO("the grain pool never overflows, and every grain retires") {
    S::machine m = make(960.0, 12000.0);
    m.set_overlap(S::k_max_overlap);

    int worst = 0;
    for (int i = 0; i < static_cast<int>(k_sr * 2.0); ++i) {
        m.process(0.3 * std::sin(2.0 * k_pi * 200.0 * i / k_sr));
        worst = std::max(worst, m.active_grains());
    }
    INFO("most grains alive at once: " << worst << " of " << S::k_max_grains);
    CHECK(worst <= S::k_max_overlap);

    m.clear();
    CHECK(m.active_grains() == 0);
}

SCENARIO("the signal path and the attribute path agree when they are told the same thing") {
    S::machine a = make(2400.0, 0.0);
    S::machine b = make(2400.0, 0.0);
    a.set_position_ms(ms_of(1200));
    a.set_pitch(3.0);

    const std::vector<double> x    = plucks(20000);
    bool                      same = true;
    for (size_t i = 0; i < x.size(); ++i) {
        same = same && (a.process(x[i]) == b.process(x[i], ms_of(1200), 3.0));
    }
    REQUIRE(same);
}

SCENARIO("fully dry, the scrub is a bitwise passthrough") {
    S::machine m = make(2400.0, 12000.0);
    m.set_mix(0.0);
    bool dry = true;
    for (int i = 0; i < 4800; ++i) {
        const double x = std::sin(2.0 * k_pi * 300.0 * i / k_sr);
        dry            = dry && (m.process(x) == x);
    }
    REQUIRE(dry); // fully dry is the input, bitwise
}

SCENARIO("a position move is slewed, so a scrub gesture does not click") {
    S::machine m = make(2400.0, 0.0);
    m.set_smooth_ms(50.0);
    // One continuous tone throughout: a break in the INPUT would come straight back out of the
    // delay and the test would be measuring its own seam rather than the scrub gesture.
    auto tone = [](int i) { return 0.5 * std::sin(2.0 * k_pi * 300.0 * static_cast<double>(i) / k_sr); };
    int  n    = 0;
    for (; n < 48000; ++n) {
        m.process(tone(n));
    }

    m.set_position_ms(500.0); // slam the playhead half a second back
    double last  = m.process(tone(n++));
    double worst = 0.0;
    for (int i = 0; i < static_cast<int>(0.2 * k_sr); ++i, ++n) {
        const double y = m.process(tone(n));
        worst          = std::max(worst, std::abs(y - last));
        last           = y;
    }
    // A steady tone read from anywhere on the tape is the same tone, so the only thing a scrub
    // across it can produce is a phase discontinuity — bounded here well under the tone's peak.
    INFO("largest single-sample step during a half-second scrub: " << worst);
    CHECK(worst < 0.1);
}

SCENARIO("unprepared, the scrub passes its input through") {
    S::machine m;
    bool       clean = true;
    for (int i = 0; i < 100; ++i) {
        const double x = 0.01 * static_cast<double>(i);
        clean          = clean && (m.process(x) == x);
    }
    REQUIRE(clean);
}
