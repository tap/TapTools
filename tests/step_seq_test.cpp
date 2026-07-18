/// @file
/// @brief      Unit tests for the shared step-sequencer engine (taptools::seq).
/// @details    Pins the plans/tap.seq.md contract: sample-accurate step derivation from a
///             phase ramp, the swing warp, polymeter off one ramp, reverse phase, the
///             trigger row's amplitude-as-accent impulses with re-arming gaps, the note
///             row's gate duty / accent levels / gate-hold slide (including chained slides
///             and the slide across the pattern wrap), quantized slot recall, determinism,
///             and a smoke pairing with the tb303 voice.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/step_seq.h>
#include <taptools/tb303_voice.h>

namespace {

    constexpr double k_sr    = 48000.0;
    constexpr double k_freq  = 2.0; // pattern cycles per second
    constexpr int    k_cycle = static_cast<int>(k_sr / k_freq);

    using taptools::seq::engine;
    using taptools::seq::note_row;
    using taptools::seq::trigger_row;

    double phase_at(int n) {
        const double p = static_cast<double>(n) * (k_freq / k_sr);
        return p - std::floor(p);
    }

    /// Sample indices where the row emits a trigger edge (output rises from silence).
    std::vector<int> trigger_edges(trigger_row& row, int samples) {
        std::vector<int> edges;
        double           prev = 0.0;
        for (int n = 0; n < samples; ++n) {
            const double y = row.process(phase_at(n));
            if (prev <= 1e-3 && y > 1e-3)
                edges.push_back(n);
            prev = y;
        }
        return edges;
    }

    bool near(int a, int b, int tol = 1) {
        return std::abs(a - b) <= tol;
    }

} // namespace

SCENARIO("steps land exactly on the analytic grid with no swing") {
    trigger_row row;
    row.prepare(k_sr);
    for (auto& s : row.clock().data().steps)
        s.velocity = 1.0;

    const auto edges = trigger_edges(row, 2 * k_cycle);
    REQUIRE(edges.size() == 32); // 16 steps x 2 cycles
    const int step_len = k_cycle / 16;
    for (int i = 0; i < 32; ++i) {
        INFO("edge " << i << " at " << edges[i]);
        CHECK(near(edges[i], i * step_len));
    }
}

SCENARIO("different lengths off one ramp are polymeter") {
    trigger_row a, b;
    a.prepare(k_sr);
    b.prepare(k_sr);
    a.clock().data().set_length(16);
    b.clock().data().set_length(12);
    for (auto& s : a.clock().data().steps)
        s.velocity = 1.0;
    for (auto& s : b.clock().data().steps)
        s.velocity = 1.0;

    std::vector<int> ea, eb;
    double           pa = 0.0, pb = 0.0;
    for (int n = 0; n < k_cycle; ++n) {
        const double ya = a.process(phase_at(n));
        const double yb = b.process(phase_at(n));
        if (pa <= 1e-3 && ya > 1e-3)
            ea.push_back(n);
        if (pb <= 1e-3 && yb > 1e-3)
            eb.push_back(n);
        pa = ya;
        pb = yb;
    }
    CHECK(ea.size() == 16);
    CHECK(eb.size() == 12);
}

SCENARIO("swing delays the odd steps by swing/2 of a step") {
    trigger_row row;
    row.prepare(k_sr);
    for (auto& s : row.clock().data().steps)
        s.velocity = 1.0;
    row.clock().set_swing(0.5);

    const auto edges    = trigger_edges(row, k_cycle);
    const int  step_len = k_cycle / 16;
    const int  late     = step_len / 4; // swing 0.5 -> 0.25 step
    REQUIRE(edges.size() == 16);
    for (int k = 0; k < 16; ++k) {
        const int expect = k * step_len + ((k & 1) ? late : 0);
        INFO("step " << k << " at " << edges[k] << " expected " << expect);
        CHECK(near(edges[k], expect));
    }
}

SCENARIO("swing 0 is bit-identical to the straight grid") {
    trigger_row a, b;
    a.prepare(k_sr);
    b.prepare(k_sr);
    for (auto& s : a.clock().data().steps)
        s.velocity = 0.7;
    for (auto& s : b.clock().data().steps)
        s.velocity = 0.7;
    b.clock().set_swing(0.0);
    for (int n = 0; n < k_cycle; ++n)
        REQUIRE(a.process(phase_at(n)) == b.process(phase_at(n)));
}

SCENARIO("the trigger row emits velocities as amplitudes and rests as nothing") {
    trigger_row row;
    row.prepare(k_sr);
    auto& p = row.clock().data();
    p.steps[0].velocity = taptools::seq::k_trig_plain;
    p.steps[4].velocity = taptools::seq::k_trig_accented;
    p.steps[8].velocity = 1.0; // full 14 V accent

    int    fired = 0;
    double prev  = 0.0;
    for (int n = 0; n < k_cycle; ++n) {
        const double y = row.process(phase_at(n));
        if (prev <= 1e-3 && y > 1e-3) {
            ++fired;
            const int k = n / (k_cycle / 16);
            CHECK(y == p.steps[k].velocity);
        }
        prev = y;
    }
    CHECK(fired == 3);
}

SCENARIO("default impulses are single-sample with a re-arming gap between adjacent steps") {
    trigger_row row;
    row.prepare(k_sr);
    row.clock().data().steps[2].velocity = 1.0;
    row.clock().data().steps[3].velocity = 1.0; // back to back

    int high = 0, edges = 0;
    double prev = 0.0;
    for (int n = 0; n < k_cycle; ++n) {
        const double y = row.process(phase_at(n));
        if (y > 1e-3)
            ++high;
        if (prev <= 1e-3 && y > 1e-3)
            ++edges;
        prev = y;
    }
    CHECK(high == 2);  // one sample each
    CHECK(edges == 2); // both edges register downstream
}

SCENARIO("pulse_ms widens the impulse into a held gate") {
    trigger_row row;
    row.prepare(k_sr);
    row.set_pulse_ms(10.0);
    row.clock().data().steps[0].velocity = 0.8;

    int high = 0;
    for (int n = 0; n < k_cycle; ++n)
        if (row.process(phase_at(n)) > 1e-3)
            ++high;
    CHECK(near(high, static_cast<int>(0.010 * k_sr), 2));
}

SCENARIO("a reversed phase still enters every step") {
    engine e;
    int    entered = 0;
    for (int n = 0; n < k_cycle; ++n) {
        const double p = 1.0 - phase_at(n); // backwards ramp
        if (e.process(p).entered)
            ++entered;
    }
    CHECK(entered >= 16);
    CHECK(entered <= 17); // the initial entry may add one
}

SCENARIO("length changes while running keep indices in range") {
    engine e;
    for (int n = 0; n < 2 * k_cycle; ++n) {
        if (n == k_cycle / 2)
            e.data().set_length(12);
        const auto t = e.process(phase_at(n));
        REQUIRE(t.index >= 0);
        REQUIRE(t.index < e.data().length);
    }
}

SCENARIO("the note row opens at the step start and closes at the gate duty") {
    note_row row;
    row.prepare(k_sr);
    auto& p            = row.clock().data();
    p.steps[0].gate    = true;
    p.steps[0].pitch   = 45.0;

    const int step_len = k_cycle / 16;
    int       rise = -1, fall = -1;
    double    prev = 0.0;
    for (int n = 0; n < k_cycle; ++n) {
        const auto o = row.process(phase_at(n));
        if (prev <= 1e-3 && o.gate > 1e-3)
            rise = n;
        if (prev > 1e-3 && o.gate <= 1e-3)
            fall = n;
        prev = o.gate;
    }
    CHECK(near(rise, 0));
    CHECK(near(fall, static_cast<int>(step_len * taptools::seq::k_gate_duty), 2));
}

SCENARIO("accented steps gate at 2.0, plain at 1.0") {
    note_row row;
    row.prepare(k_sr);
    auto& p          = row.clock().data();
    p.steps[0].gate  = true;
    p.steps[2].gate  = true;
    p.steps[2].accent = true;

    double plain = 0.0, accented = 0.0;
    for (int n = 0; n < k_cycle; ++n) {
        const auto o = row.process(phase_at(n));
        const int  k = n / (k_cycle / 16);
        if (k == 0)
            plain = std::max(plain, o.gate);
        if (k == 2)
            accented = std::max(accented, o.gate);
    }
    CHECK(plain == taptools::seq::k_gate_plain);
    CHECK(accented == taptools::seq::k_gate_accent);
}

SCENARIO("a slide step holds the gate through the boundary while the pitch steps") {
    note_row row;
    row.prepare(k_sr);
    auto& p          = row.clock().data();
    p.steps[4].gate  = true;
    p.steps[4].pitch = 33.0;
    p.steps[5].gate  = true;
    p.steps[5].slide = true;
    p.steps[5].pitch = 45.0;

    const int step_len = k_cycle / 16;
    const int boundary = 5 * step_len;

    int    rises = 0, falls = 0;
    int    pitch_step_at = -1;
    double prev_gate = 0.0, prev_pitch = 0.0;
    for (int n = 4 * step_len; n < 6 * step_len; ++n) {
        const auto o = row.process(phase_at(n));
        if (prev_gate <= 1e-3 && o.gate > 1e-3)
            ++rises;
        if (prev_gate > 1e-3 && o.gate <= 1e-3)
            ++falls;
        if (n > 4 * step_len && o.pitch != prev_pitch)
            pitch_step_at = n;
        prev_gate  = o.gate;
        prev_pitch = o.pitch;
    }
    CHECK(rises == 1);                    // one note-on for the pair: no retrigger
    CHECK(falls == 1);                    // ... closing at step 5's own duty point
    CHECK(near(pitch_step_at, boundary)); // the pitch steps exactly at the boundary
}

SCENARIO("chained slides stay legato from the first note to the last duty point") {
    note_row row;
    row.prepare(k_sr);
    auto& p = row.clock().data();
    for (int k = 4; k <= 6; ++k) {
        p.steps[k].gate  = true;
        p.steps[k].pitch = 30.0 + k;
        p.steps[k].slide = (k > 4);
    }

    int    rises = 0;
    double prev  = 0.0;
    for (int n = 0; n < k_cycle; ++n) {
        const auto o = row.process(phase_at(n));
        if (prev <= 1e-3 && o.gate > 1e-3)
            ++rises;
        prev = o.gate;
    }
    CHECK(rises == 1);
}

SCENARIO("a slide across the pattern wrap holds the gate through phase 1 -> 0") {
    note_row row;
    row.prepare(k_sr);
    auto& p           = row.clock().data();
    p.steps[15].gate  = true;
    p.steps[15].pitch = 40.0;
    p.steps[0].gate   = true;
    p.steps[0].slide  = true;
    p.steps[0].pitch  = 52.0;

    // Second cycle: step 15 must hold into the wrapped step 0 without an edge.
    int    rises_near_wrap = 0;
    bool   held            = true;
    double prev            = 0.0;
    const int step_len     = k_cycle / 16;
    for (int n = 0; n < 2 * k_cycle; ++n) {
        const auto o = row.process(phase_at(n));
        const bool near_wrap = n > k_cycle - step_len / 2 && n < k_cycle + step_len / 4;
        if (near_wrap) {
            if (prev <= 1e-3 && o.gate > 1e-3)
                ++rises_near_wrap;
            if (o.gate <= 1e-3)
                held = false;
        }
        prev = o.gate;
    }
    CHECK(rises_near_wrap == 0);
    CHECK(held);
}

SCENARIO("a slide flag after a rest is a plain trigger") {
    note_row row;
    row.prepare(k_sr);
    auto& p          = row.clock().data();
    p.steps[8].gate  = true;
    p.steps[8].slide = true; // nothing sounding before it

    int    rises = 0;
    double prev  = 0.0;
    for (int n = 0; n < k_cycle; ++n) {
        const auto o = row.process(phase_at(n));
        if (prev <= 1e-3 && o.gate > 1e-3)
            ++rises;
        prev = o.gate;
    }
    CHECK(rises == 1);
}

SCENARIO("recall quantized to the cycle swaps exactly at the wrap") {
    trigger_row row;
    row.prepare(k_sr);
    auto& e = row.clock();
    for (auto& s : e.data().steps)
        s.velocity = 0.25;
    e.store(0);
    for (auto& s : e.data().steps)
        s.velocity = 1.0;
    e.store(1);

    // Running pattern is slot 1 (all 1.0); arm slot 0 mid-cycle.
    bool armed = false;
    double prev = 0.0;
    std::vector<std::pair<int, double>> edges;
    for (int n = 0; n < 2 * k_cycle; ++n) {
        if (n == k_cycle / 3 && !armed) {
            e.recall(0);
            armed = true;
        }
        const double y = row.process(phase_at(n));
        if (prev <= 1e-3 && y > 1e-3)
            edges.emplace_back(n, y);
        prev = y;
    }
    for (const auto& [n, y] : edges) {
        INFO("edge at " << n << " amp " << y);
        if (n < k_cycle)
            CHECK(y == 1.0); // armed but not yet applied
        else
            CHECK(y == 0.25); // swapped at the wrap
    }
}

SCENARIO("recall quantize now applies immediately") {
    engine e;
    e.data().steps[0].velocity = 0.5;
    e.store(0);
    e.data().steps[0].velocity = 1.0;
    e.set_quantize(taptools::seq::quantize_now);
    e.recall(0);
    CHECK(e.data().steps[0].velocity == 0.5);
    CHECK(e.armed() == -1);
}

SCENARIO("two identical runs are bit-exact") {
    auto run = [] {
        note_row row;
        row.prepare(k_sr);
        auto& p = row.clock().data();
        for (int k = 0; k < 16; k += 2) {
            p.steps[k].gate   = true;
            p.steps[k].pitch  = 30.0 + k;
            p.steps[k].accent = (k % 4) == 0;
        }
        row.clock().set_swing(0.61);
        std::vector<double> y;
        y.reserve(2 * k_cycle);
        for (int n = 0; n < 2 * k_cycle; ++n) {
            const auto o = row.process(phase_at(n));
            y.push_back(o.gate + o.pitch);
        }
        return y;
    };
    CHECK(run() == run());
}

SCENARIO("the note row drives the tb303 voice: slid steps glide, plain steps retrigger") {
    note_row row;
    row.prepare(k_sr);
    auto& p = row.clock().data();
    p.steps[0].gate  = true;
    p.steps[0].pitch = 33.0;
    p.steps[1].gate  = true;
    p.steps[1].slide = true;
    p.steps[1].pitch = 45.0;
    p.steps[4].gate  = true;
    p.steps[4].pitch = 33.0;

    taptools::tb303::voice v;
    v.prepare(k_sr);

    // The tap.303~ wrapper loop, verbatim.
    double note_ons  = 0;
    double prev_gate = 0.0;
    bool   gate_held_at_boundary = false;
    double peak                  = 0.0;
    const int step_len           = k_cycle / 16;
    for (int n = 0; n < k_cycle; ++n) {
        const auto o = row.process(phase_at(n));
        if (prev_gate < 1e-3 && o.gate >= 1e-3) {
            v.note_on(o.pitch, std::clamp(o.gate - 1.0, 0.0, 1.0));
            ++note_ons;
        }
        else if (prev_gate >= 1e-3 && o.gate < 1e-3) {
            v.note_off();
        }
        prev_gate = o.gate;
        if (v.gate())
            v.set_pitch(o.pitch);
        const double y = v.process();
        REQUIRE(std::isfinite(y));
        peak = std::max(peak, std::abs(y));
        if (n == step_len + 2) // just past the slid boundary
            gate_held_at_boundary = v.gate();
    }
    CHECK(note_ons == 2);          // steps 0 and 4; step 1 arrived legato
    CHECK(gate_held_at_boundary);  // the voice never saw a note-off at the slide
    CHECK(peak > 0.01);
}
