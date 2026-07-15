/// @file
/// @brief      Unit tests for the comb-bank kernel's loop stability and ring-time calibration.
/// @details    Pins the 2026-07 DC-blocker fix: the raw blocker (1 - z^-1)/(1 - R z^-1) peaks at
///             2/(1+R) > 1 at Nyquist, so an fb near the cap times that shelf could exceed unity
///             loop gain — at res 100 with the feedback lowpass wide open the bank measurably
///             swelled (~+0.2 dB/s; found while proving boundedness for the book's machine
///             chapter). The blocker is now normalized to unity peak gain (loop gain bounded by
///             fb < 1 at every setting) and the res -> ring-time map is compensated for the
///             blocker's gain at each voice's fundamental. The bank's behavioral tests live
///             wrapper-side (TapTools-Max, tap.5comb_tilde); these two scenarios guard the fix.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/grm_comb.h>

namespace {

    constexpr double k_sr = 48000.0;

    namespace fc = taptools::fivecomb;

    fc::comb_bank make_single_voice(double freq, double res, double lp) {
        fc::comb_bank bank;
        bank.prepare(k_sr);
        bank.set_smooth_ms(0.0);
        bank.set_mix(100.0);
        for (int v = 0; v < fc::k_voices; ++v) {
            bank.set_param(fc::p_res1 + v, 0.0); // silence all voices...
        }
        bank.set_param(fc::p_freq1 + 0, freq); // ...except the first
        bank.set_param(fc::p_res1 + 0, res);
        bank.set_param(fc::p_lp1 + 0, lp);
        return bank;
    }

    double rms(const std::vector<double>& y, size_t lo, size_t hi) {
        double acc = 0.0;
        for (size_t i = lo; i < hi && i < y.size(); ++i) {
            acc += y[i] * y[i];
        }
        return std::sqrt(acc / static_cast<double>(hi - lo));
    }

} // namespace

SCENARIO("the loop is strictly contractive at the extreme corner (res 100, lp wide open)") {
    // The exact configuration that swelled before the blocker was normalized: maximum resonance,
    // no feedback damping, a voice near 450 Hz (where the unnormalized blocker's >1 shelf beat
    // the fb cap). Excite with a short noise burst, then let it ring for 12 seconds: the tail
    // must decay monotonically window-over-window, never grow.
    auto bank = make_single_voice(450.0, 100.0, 20000.0);

    uint32_t            rng = 1234u;
    std::vector<double> y(static_cast<size_t>(14.0 * k_sr));
    for (size_t i = 0; i < y.size(); ++i) {
        double in = 0.0;
        if (i < static_cast<size_t>(0.2 * k_sr)) {
            rng = rng * 1664525u + 1013904223u;
            in  = ((rng / 2147483648.0) - 1.0) * 0.5;
        }
        y[i] = bank.process(in);
    }

    const auto   at    = [](double s) { return static_cast<size_t>(s * k_sr); };
    const double early = rms(y, at(2.0), at(6.0));
    const double late  = rms(y, at(8.0), at(12.0));
    INFO("ring-out RMS: [2,6)s = " << early << ", [8,12)s = " << late);
    REQUIRE(std::isfinite(late));
    REQUIRE(late < early); // decaying, not swelling
}

SCENARIO("the res -> ring-time map is honest at the fundamental") {
    // res 50 maps to rt60 = sqrt(20 ms * 100 s) ~ 1.414 s on the log curve. Measure the actual
    // decay of the ringing fundamental between two points of the tail and recover RT60; the
    // blocker-gain compensation is what makes this land on target.
    const double res      = 50.0;
    const double expected = fc::k_rt60_min * std::pow(fc::k_rt60_max / fc::k_rt60_min, res * 0.01);

    auto bank = make_single_voice(220.0, res, 20000.0);

    std::vector<double> y(static_cast<size_t>(3.0 * k_sr));
    for (size_t i = 0; i < y.size(); ++i) {
        y[i] = bank.process(i == 0 ? 1.0 : 0.0); // impulse excitation
    }

    const auto   at = [](double s) { return static_cast<size_t>(s * k_sr); };
    const double w  = 0.25; // RMS window, seconds
    const double t1 = 0.5, t2 = 2.0;
    const double l1       = rms(y, at(t1), at(t1 + w));
    const double l2       = rms(y, at(t2), at(t2 + w));
    const double db_drop  = 20.0 * std::log10(l1 / l2);
    const double measured = 60.0 * (t2 - t1) / db_drop;
    INFO("expected rt60 " << expected << " s, measured " << measured << " s");
    REQUIRE(std::abs(measured - expected) / expected < 0.1);
}
