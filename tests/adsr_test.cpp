/// @file
/// @brief      Unit tests for the virtual-analog ADSR kernel (tap::tools::adsr::generator).
/// @details    Pins the analog model's contract numbers (attack reaches full scale at the knob
///             time with the truncated-charge midpoint near 0.65; decay and release close 95 %
///             of their gap at the knob time and taper asymptotically), the family trigger
///             contract (default threshold hears the sequencer's plain level; gate amplitude is
///             velocity under the sensitivity control), retrigger-from-current-level, and the
///             faithful preservation of the legacy Jamoma curves.
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/adsr.h>

namespace {

    constexpr double k_sr = 48000.0;

    /// Run `n` samples at a constant gate level, returning every output sample.
    std::vector<double> run(tap::tools::adsr::generator& g, double gate, int n) {
        std::vector<double> out(static_cast<size_t>(n));
        for (int t = 0; t < n; ++t) {
            out[static_cast<size_t>(t)] = g.process(gate);
        }
        return out;
    }

} // namespace

SCENARIO("the analog attack reaches full scale at the knob time, convex like a charging capacitor") {
    tap::tools::adsr::generator g;
    g.prepare(k_sr);
    g.set_attack_ms(100.0);

    const int  attack_samples = static_cast<int>(0.100 * k_sr);
    const auto out            = run(g, 1.0, attack_samples + 100);

    // Reaches the peak at the knob time (within a handful of samples).
    int first_at_peak = -1;
    for (int t = 0; t < static_cast<int>(out.size()); ++t) {
        if (out[static_cast<size_t>(t)] >= 0.999) {
            first_at_peak = t;
            break;
        }
    }
    REQUIRE(first_at_peak > 0);
    REQUIRE(std::abs(first_at_peak - attack_samples) < attack_samples / 50);

    // The truncated-charge law puts the midpoint near 0.65 — visibly not a straight line.
    const double mid = out[static_cast<size_t>(attack_samples / 2)];
    REQUIRE(mid > 0.60);
    REQUIRE(mid < 0.70);
}

SCENARIO("the analog decay tapers into sustain, closing 95 percent at the knob time") {
    tap::tools::adsr::generator g;
    g.prepare(k_sr);
    g.set_attack_ms(1.0);
    g.set_decay_ms(200.0);
    g.set_sustain_db(-12.0);
    const double sus = std::pow(10.0, -12.0 / 20.0);

    const int  attack_samples = static_cast<int>(0.001 * k_sr);
    const int  decay_samples  = static_cast<int>(0.200 * k_sr);
    const auto out            = run(g, 1.0, attack_samples + 2 * decay_samples);

    // At the decay knob time past the peak: 95 % of the way down, and still above sustain.
    const double at_knob = out[static_cast<size_t>(attack_samples + decay_samples)];
    REQUIRE(at_knob > sus);                      // asymptotic — never crosses
    REQUIRE(at_knob - sus < 0.07 * (1.0 - sus)); // ~95 % closed (tolerance for the attack tail)
    // And still tapering, not parked: strictly decreasing toward sustain.
    const double later = out[static_cast<size_t>(attack_samples + 2 * decay_samples - 1)];
    REQUIRE(later < at_knob);
    REQUIRE(later > sus);
}

SCENARIO("the analog release closes 95 percent at the knob time and ends at exactly zero") {
    tap::tools::adsr::generator g;
    g.prepare(k_sr);
    g.set_attack_ms(1.0);
    g.set_decay_ms(1.0);
    g.set_sustain_db(0.0); // sustain at unity so the release starts from a known level
    g.set_release_ms(100.0);

    run(g, 1.0, static_cast<int>(0.05 * k_sr)); // open and settle at sustain
    const int  release_samples = static_cast<int>(0.100 * k_sr);
    const auto tail            = run(g, 0.0, 6 * release_samples);

    REQUIRE(tail[static_cast<size_t>(release_samples)] < 0.07);
    REQUIRE(tail.back() == 0.0); // the stage genuinely ends
    REQUIRE_FALSE(g.active());
}

SCENARIO("retrigger rises from the current level without a jump") {
    tap::tools::adsr::generator g;
    g.prepare(k_sr);
    g.set_attack_ms(50.0);
    g.set_release_ms(500.0);

    run(g, 1.0, static_cast<int>(0.2 * k_sr));  // up to sustain
    run(g, 0.0, static_cast<int>(0.05 * k_sr)); // partway into the release
    // Re-gate and confirm the largest sample-to-sample move stays at attack-slope scale.
    double prev     = g.process(1.0);
    double max_step = 0.0;
    for (int t = 0; t < static_cast<int>(0.1 * k_sr); ++t) {
        const double y = g.process(1.0);
        max_step       = std::max(max_step, std::abs(y - prev));
        prev           = y;
    }
    REQUIRE(max_step < 0.005); // no discontinuity — the capacitor charges from where it sat
}

SCENARIO("gate amplitude is velocity under the sensitivity control") {
    auto peak_for = [](double gate, double sensitivity) {
        tap::tools::adsr::generator g;
        g.prepare(k_sr);
        g.set_attack_ms(10.0);
        g.set_velocity(sensitivity);
        const auto out = run(g, gate, static_cast<int>(0.05 * k_sr));
        double     pk  = 0.0;
        for (const double y : out) {
            pk = std::max(pk, y);
        }
        return pk;
    };

    // Full sensitivity: the 303 convention lands — accented 2.0 hits twice as hard.
    REQUIRE(std::abs(peak_for(2.0, 1.0) / peak_for(1.0, 1.0) - 2.0) < 0.05);
    // Half-level gates land softer.
    REQUIRE(peak_for(0.5, 1.0) < 0.6);
    // Zero sensitivity is the legacy amplitude-blind behavior.
    REQUIRE(std::abs(peak_for(2.0, 0.0) - peak_for(1.0, 0.0)) < 1e-9);
}

SCENARIO("the default threshold hears the sequencer's plain level, and 0.5 is no longer an edge case") {
    tap::tools::adsr::generator g;
    g.prepare(k_sr);
    g.set_attack_ms(5.0);

    // The 808 rows' plain level (0.01) opens the envelope now.
    auto out = run(g, 0.01, static_cast<int>(0.02 * k_sr));
    REQUIRE(out.back() > 0.5);

    // The old wrapper's exact-0.5 trap: an accented seq level (0.5) opens too.
    g.clear();
    out = run(g, 0.5, static_cast<int>(0.02 * k_sr));
    REQUIRE(out.back() > 0.5);

    // Below the threshold stays silent.
    g.clear();
    out = run(g, 0.004, static_cast<int>(0.02 * k_sr));
    REQUIRE(out.back() == 0.0);
}

SCENARIO("the legacy hybrid curve is preserved: linear attack, dB-linear decay") {
    tap::tools::adsr::generator g;
    g.prepare(k_sr);
    g.set_mode(tap::tools::adsr::mode::hybrid);
    g.set_attack_ms(100.0);
    g.set_decay_ms(500.0);
    g.set_sustain_db(-60.0);

    const int  attack_samples = static_cast<int>(0.100 * k_sr);
    const auto out            = run(g, 1.0, attack_samples + static_cast<int>(0.4 * k_sr));

    // Linear attack: the midpoint is 0.5, not the analog 0.65.
    REQUIRE(std::abs(out[static_cast<size_t>(attack_samples / 2)] - 0.5) < 0.02);

    // dB-linear decay: equal time steps lose equal decibels.
    const int    hop = static_cast<int>(0.05 * k_sr);
    const double d1  = 20.0 * std::log10(out[static_cast<size_t>(attack_samples + hop)])
                      - 20.0 * std::log10(out[static_cast<size_t>(attack_samples + 2 * hop)]);
    const double d2 = 20.0 * std::log10(out[static_cast<size_t>(attack_samples + 2 * hop)])
                      - 20.0 * std::log10(out[static_cast<size_t>(attack_samples + 3 * hop)]);
    REQUIRE(std::abs(d1 - d2) < 0.1);
}

SCENARIO("the envelope is silent before prepare and after clear") {
    tap::tools::adsr::generator g;
    REQUIRE(g.process(1.0) == 0.0);
    g.prepare(k_sr);
    run(g, 1.0, 1000);
    g.clear();
    REQUIRE_FALSE(g.active());
    REQUIRE(g.process(0.0) == 0.0);
}
