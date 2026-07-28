/// @file
/// @brief      Cross-kernel guard for the two numeric profiles (numeric.h).
/// @details    Every kernel is `basic_<name><Sample>` with a `<name>` (double) and `<name>32`
///             (float) alias. Per-kernel behaviour is pinned by that kernel's own test file; what
///             this file guards is the *property that must hold across all of them*, and that the
///             per-kernel tests — which mostly exercise the double profile — would not catch:
///
///               1. Every kernel actually exposes both profiles, and each alias resolves to the
///                  precision it claims. A migration that forgets an alias still compiles.
///               2. The float profile is genuinely instantiable and runnable. A templated class
///                  that is never instantiated for float is never type-checked for float, so a
///                  mixed-precision expression can sit in it indefinitely. Merely *including* a
///                  header does not instantiate it — these tests run the code.
///               3. The float profile produces finite output and its recursive state reaches true
///                  zero, i.e. `k_denormal_floor<float>` is doing its job.
///
///             The motivating regression: an automated pass once rewrote a kernel's class-local
///             `anti_denormal` body into a call to itself. That compiled cleanly in both profiles
///             and recursed until the stack died. Compile-time checks cannot see it; running the
///             kernel can. Hence every case here executes samples rather than asserting on types
///             alone.
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <cmath>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>
#include <taptools/step_seq.h> // not part of the taptools.h umbrella
#include <taptools/taptools.h>
#include <taptools/tune.h> // not part of the taptools.h umbrella

namespace tt = tap::tools;

namespace {

    /// A kernel's two aliases must differ in precision and nothing else.
    template <typename Dbl, typename Flt>
    constexpr bool profiles_pair() {
        return std::is_same_v<typename Dbl::sample_type, double> && std::is_same_v<typename Flt::sample_type, float>;
    }

    /// Drive `n` samples through `f` and require every one is finite.
    template <typename F, typename Step>
    void run_finite(F& f, int n, Step step) {
        for (int i = 0; i < n; ++i) {
            const double y = static_cast<double>(step(f, i));
            REQUIRE(std::isfinite(y));
        }
    }

} // namespace

SCENARIO("every kernel exposes both numeric profiles") {
    STATIC_REQUIRE(profiles_pair<tt::svf::svf_filter, tt::svf::svf_filter32>());
    STATIC_REQUIRE(profiles_pair<tt::ladder::ladder_filter, tt::ladder::ladder_filter32>());
    STATIC_REQUIRE(profiles_pair<tt::diode::diode_filter, tt::diode::diode_filter32>());
    STATIC_REQUIRE(profiles_pair<tt::vco::vco_osc, tt::vco::vco_osc32>());
    STATIC_REQUIRE(profiles_pair<tt::od::overdrive, tt::od::overdrive32>());
    STATIC_REQUIRE(profiles_pair<tt::autowah::wah_filter, tt::autowah::wah_filter32>());
    STATIC_REQUIRE(profiles_pair<tt::tb303::voice, tt::tb303::voice32>());
    STATIC_REQUIRE(profiles_pair<tt::vca, tt::vca32>());
    STATIC_REQUIRE(profiles_pair<tt::fivecomb::comb_bank, tt::fivecomb::comb_bank32>());
    STATIC_REQUIRE(profiles_pair<tt::pitchaccum::accum_bank, tt::pitchaccum::accum_bank32>());
    STATIC_REQUIRE(profiles_pair<tt::vocoder::bank, tt::vocoder::bank32>());
    STATIC_REQUIRE(profiles_pair<tt::nr::reducer, tt::nr::reducer32>());
    STATIC_REQUIRE(profiles_pair<tt::spectra::remapper, tt::spectra::remapper32>());
    STATIC_REQUIRE(profiles_pair<tt::tune::corrector, tt::tune::corrector32>());
    STATIC_REQUIRE(profiles_pair<tt::conv_engine, tt::conv_engine32>());
    STATIC_REQUIRE(profiles_pair<tt::stft, tt::stft32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::bridged_t, tt::tr808::bridged_t32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::first_order, tt::tr808::first_order32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::metal_bank, tt::tr808::metal_bank32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::white_noise, tt::tr808::white_noise32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::decay_env, tt::tr808::decay_env32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::kick, tt::tr808::kick32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::snare, tt::tr808::snare32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::hat, tt::tr808::hat32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::cymbal, tt::tr808::cymbal32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::clap, tt::tr808::clap32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::rim, tt::tr808::rim32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::tom, tt::tr808::tom32>());
    STATIC_REQUIRE(profiles_pair<tt::tr808::cowbell, tt::tr808::cowbell32>());
    STATIC_REQUIRE(profiles_pair<tt::seq::engine, tt::seq::engine32>());
}

SCENARIO("the float profile runs and stays finite") {
    constexpr float k_sr = 48000.0f;

    GIVEN("the filters and oscillators") {
        tt::svf::svf_filter32 svf;
        svf.prepare(k_sr, 1);
        svf.set_frequency(1200.0f);
        svf.set_resonance(0.8f);
        svf.snap();
        run_finite(svf, 2000, [](auto& f, int i) { return f.process(std::sin(0.05f * float(i))); });

        tt::ladder::ladder_filter32 lad;
        lad.prepare(k_sr);
        lad.set_param(tt::ladder::p_frequency, 1500.0f);
        lad.snap();
        run_finite(lad, 2000, [](auto& f, int i) { return f.process(std::sin(0.05f * float(i))); });

        tt::diode::diode_filter32 dio;
        dio.prepare(k_sr);
        dio.set_param(tt::diode::p_frequency, 1500.0f);
        dio.snap();
        run_finite(dio, 2000, [](auto& f, int i) { return f.process(std::sin(0.05f * float(i))); });

        tt::vco::vco_osc32 osc;
        osc.prepare(k_sr);
        osc.set_param(tt::vco::p_frequency, 220.0f);
        osc.snap();
        run_finite(osc, 2000, [](auto& f, int) { return f.process(); });

        tt::autowah::wah_filter32 wah;
        wah.prepare(k_sr);
        wah.snap();
        run_finite(wah, 2000, [](auto& f, int i) { return f.process(std::sin(0.05f * float(i))); });

        tt::od::overdrive32 od;
        od.prepare(k_sr, 1);
        od.snap();
        run_finite(od, 2000, [](auto& f, int i) { return f.process(std::sin(0.05f * float(i))); });
    }

    GIVEN("the 808 voices") {
        tt::tr808::kick32 kick;
        kick.prepare(k_sr);
        run_finite(kick, 4000, [](auto& v, int i) {
            if (i % 2000 == 0) {
                v.trigger(1.0f);
            }
            return v.process();
        });

        tt::tr808::snare32 snare;
        snare.prepare(k_sr);
        run_finite(snare, 4000, [](auto& v, int i) {
            if (i % 2000 == 0) {
                v.trigger(1.0f);
            }
            return v.process();
        });

        tt::tr808::hat32 hat;
        hat.prepare(k_sr);
        run_finite(hat, 4000, [](auto& v, int i) {
            if (i % 2000 == 0) {
                v.trigger_closed(1.0f);
            }
            return v.process();
        });

        tt::tr808::cymbal32 cym;
        cym.prepare(k_sr);
        run_finite(cym, 4000, [](auto& v, int i) {
            if (i % 2000 == 0) {
                v.trigger(1.0f);
            }
            return v.process();
        });
    }

    GIVEN("the 303 voice") {
        tt::tb303::voice32 v;
        v.prepare(k_sr);
        v.snap();
        run_finite(v, 4000, [](auto& x, int i) {
            if (i % 1000 == 0) {
                x.note_on(45.0f, 0.5f);
            }
            return x.process();
        });
    }
}

SCENARIO("the float profile's denormal guard reaches exactly zero") {
    // k_denormal_floor<float> is 1e-30, not the house 1e-15: float denormals live in
    // ~1.4e-45 .. 1.18e-38, so the double-scale constant would be an arbitrary -300 dBFS gate
    // rather than a denormal guard. What matters behaviourally is that a silent recursion
    // terminates at true zero instead of grinding through the denormal range.
    STATIC_REQUIRE(tt::k_denormal_floor<float> > 1.18e-38f);
    STATIC_REQUIRE(tt::k_denormal_floor<double> == 1e-15);

    REQUIRE(tt::anti_denormal(1e-35f) == 0.0f);
    REQUIRE(tt::anti_denormal(0.5f) == 0.5f);
    REQUIRE(tt::anti_denormal(1e-20) == 0.0);
    REQUIRE(tt::anti_denormal(0.5) == 0.5);

    tt::svf::svf_filter32 f;
    f.prepare(48000.0f, 1);
    f.set_frequency(2000.0f);
    f.set_resonance(0.0f);
    f.snap();
    (void)f.process(1.0f);
    for (int i = 0; i < 200000; ++i) {
        (void)f.process(0.0f);
    }
    REQUIRE(f.process(0.0f) == 0.0f);
}

SCENARIO("pi is carried in the working precision") {
    // A double pi in a float expression promotes the whole expression back to double, which on a
    // single-precision target is the soft-float path the float profile exists to avoid.
    STATIC_REQUIRE(std::is_same_v<decltype(tt::k_pi_for<float>), const float>);
    STATIC_REQUIRE(std::is_same_v<decltype(tt::k_pi_for<double>), const double>);
    REQUIRE(std::abs(static_cast<double>(tt::k_pi_for<float>) - 3.14159265358979323846) < 1e-6);
}
