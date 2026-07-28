/// @file
/// @brief      Unit tests for the Simper SVF kernel (tap::tools::svf), both numeric profiles.
/// @details    Pins the contract points the header promises, run against BOTH instantiations of
///             `basic_svf_filter<Sample>` — `svf_filter` (double, the golden model) and
///             `svf_filter32` (float, the embedded profile) — plus their cross-precision
///             agreement. The templated battery is the DspTap discipline applied here: the two
///             profiles run the identical algorithm in their own precision, and a claim that
///             holds for one is expected to hold for the other within its precision's reach.
///
///             What is pinned:
///               - Butterworth base Q per order, and the q_from_resonance/resonance_from_q pair.
///               - Resonance 0 is maximally flat: -3 dB at the cutoff at every order (this is the
///                 property the Butterworth Q spread buys, and the reason order changes do not
///                 shift the corner).
///               - Tuning is exact: the prewarped `g = tan(pi*fc/fs)` puts the measured corner on
///                 the requested cutoff even at high fractions of Nyquist, where a naive
///                 (unprewarped) design would be audibly flat.
///               - The morph circle's corners are the discrete modes, exactly.
///               - Stability and boundedness under per-sample cutoff modulation and at the
///                 driven circuit's self-oscillation threshold.
///               - The denormal guard actually reaches zero in each profile — the reason the
///                 float profile uses a different flush threshold than the double one.
///
///             ## Numeric-profile geometry (the measured claim in svf.h)
///
///             svf.h states that the double profile is unusable on a single-precision target.
///             That claim was measured, not remembered, and this is where the geometry is
///             recorded. Building the per-sample path for an Arm Cortex-M33 (GCC 13.2,
///             `-O3 -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard`), counting calls into
///             the soft-float runtime inside the sample loop:
///
///               profile              .text     __aeabi_d* calls per sample
///               svf_filter   (double) 6812     142
///               svf_filter32 (float)  4532       0   (hardware vmul.f32 / vfma.f32 / vadd.f32)
///
///             The M33's FPv5-SP FPU has no double support, so every double operation becomes a
///             library call. The float profile removes all of them and is 33% smaller. Honest
///             limit, stated here because it is the next question a reader will have: the
///             transcendentals do NOT go away — `std::tanh` (driven circuit) and `std::tan`
///             (cutoff tier) remain libm calls in both profiles, so the driven circuit still
///             wants a polynomial approximation before it is affordable on such a part.
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <taptools/svf.h>

namespace ksv = tap::tools::svf;

namespace {

    constexpr double k_sr = 48000.0;
    constexpr double k_pi = 3.14159265358979323846;

    /// Tolerances scale with the profile: the float path carries ~7 decimal digits against
    /// double's ~16, and the recursive state accumulates that difference over a settle window.
    template <typename Sample>
    struct profile;

    template <>
    struct profile<double> {
        static constexpr double k_gain_db_tol = 0.01;  ///< magnitude-response agreement, dB
        static constexpr double k_tuning_tol  = 0.005; ///< measured corner vs requested, relative
        static constexpr double k_exact_tol   = 1e-12; ///< "bit-identical" claims (morph corners)
    };

    template <>
    struct profile<float> {
        static constexpr double k_gain_db_tol = 0.05;
        static constexpr double k_tuning_tol  = 0.02;
        static constexpr double k_exact_tol   = 1e-5;
    };

    /// Steady-state magnitude response at `hz`, in dB. Drives a unit sine, discards a settle
    /// window, then measures RMS over a whole number of cycles and refers it to the 1/sqrt(2) RMS
    /// of the input. Deliberately an output measurement (the house oracle pattern) rather than an
    /// assertion about the filter's internal coefficients.
    template <typename Filter>
    double response_db(Filter& f, double hz) {
        using sample_type = typename Filter::sample_type;
        f.clear();
        const double w       = 2.0 * k_pi * hz / k_sr;
        const int    settle  = 20000;
        const int    cycles  = 64;
        const int    measure = static_cast<int>(cycles * k_sr / hz);
        double       acc     = 0.0;
        for (int i = 0; i < settle + measure; ++i) {
            const auto x = static_cast<sample_type>(std::sin(w * i));
            const auto y = static_cast<double>(f.process(x));
            if (i >= settle) {
                acc += y * y;
            }
        }
        const double out_rms = std::sqrt(acc / measure);
        return 20.0 * std::log10(out_rms / (1.0 / std::sqrt(2.0)));
    }

    /// Bisect for the frequency where the response crosses `target_db`, over [lo, hi].
    template <typename Filter>
    double find_crossing(Filter& f, double target_db, double lo, double hi) {
        for (int i = 0; i < 40; ++i) {
            const double mid = std::sqrt(lo * hi); // bisect in log frequency
            if (response_db(f, mid) > target_db) {
                lo = mid;
            }
            else {
                hi = mid;
            }
        }
        return std::sqrt(lo * hi);
    }

} // namespace

using profiles = std::tuple<double, float>;

TEMPLATE_LIST_TEST_CASE("the Butterworth Q spread is the documented one", "[svf]", profiles) {
    using sample_type = TestType;

    THEN("base Q per order matches the published Butterworth values") {
        REQUIRE(ksv::base_q(2) == Catch::Approx(0.70710678).epsilon(1e-6));
        REQUIRE(ksv::base_q(4) == Catch::Approx(1.30656296).epsilon(1e-6));
        REQUIRE(ksv::base_q(8) == Catch::Approx(2.56291545).epsilon(1e-6));
    }

    THEN("resonance 0 is the base Q, and the pair round-trips") {
        for (int order : {2, 4, 8}) {
            REQUIRE(static_cast<double>(ksv::q_from_resonance<sample_type>(sample_type(0), order))
                    == Catch::Approx(ksv::base_q(order)).epsilon(1e-6));
            for (double r = 0.0; r < 0.95; r += 0.1) {
                const sample_type q  = ksv::q_from_resonance<sample_type>(static_cast<sample_type>(r), order);
                const sample_type rt = ksv::resonance_from_q<sample_type>(q, order);
                REQUIRE(static_cast<double>(rt) == Catch::Approx(r).margin(profile<sample_type>::k_exact_tol));
            }
        }
    }

    THEN("Q at or below the base maps to resonance 0") {
        REQUIRE(ksv::resonance_from_q<sample_type>(sample_type(0.5), 2) == sample_type(0));
        REQUIRE(ksv::resonance_from_q<sample_type>(sample_type(0), 2) == sample_type(0));
    }
}

TEMPLATE_LIST_TEST_CASE("resonance 0 is maximally flat at every order", "[svf]", profiles) {
    using filter_type = ksv::basic_svf_filter<TestType>;

    // The promise the Butterworth Q spread buys: -3 dB at the cutoff regardless of order, so
    // switching 12 -> 24 -> 48 dB/oct does not move the corner.
    for (int order : {2, 4, 8}) {
        filter_type f;
        f.prepare(static_cast<TestType>(k_sr), 1);
        f.set_mode(ksv::mode_lowpass);
        f.set_order(order);
        f.set_circuit(ksv::circuit_clean);
        f.set_frequency(static_cast<TestType>(1000.0));
        f.set_resonance(static_cast<TestType>(0.0));
        f.snap();

        INFO("order " << order);
        REQUIRE(response_db(f, 1000.0) == Catch::Approx(-3.0103).margin(0.05));
        // and the passband really is flat well below the corner
        REQUIRE(response_db(f, 100.0) == Catch::Approx(0.0).margin(profile<TestType>::k_gain_db_tol));
    }
}

TEMPLATE_LIST_TEST_CASE("prewarped tuning stays exact near Nyquist", "[svf]", profiles) {
    using filter_type = ksv::basic_svf_filter<TestType>;

    // g = tan(pi*fc/fs) is prewarped, so the measured -3 dB corner lands on the requested cutoff
    // even at a large fraction of Nyquist, where an unprewarped bilinear design runs flat.
    for (double fc : {1000.0, 8000.0, 16000.0}) {
        filter_type f;
        f.prepare(static_cast<TestType>(k_sr), 1);
        f.set_mode(ksv::mode_lowpass);
        f.set_order(2);
        f.set_circuit(ksv::circuit_clean);
        f.set_frequency(static_cast<TestType>(fc));
        f.set_resonance(static_cast<TestType>(0.0));
        f.snap();

        const double measured = find_crossing(f, -3.0103, fc * 0.5, fc * 2.0);
        INFO("requested " << fc << " Hz, measured " << measured << " Hz");
        REQUIRE(measured == Catch::Approx(fc).epsilon(profile<TestType>::k_tuning_tol));
    }
}

TEMPLATE_LIST_TEST_CASE("the morph circle's corners are the discrete modes", "[svf]", profiles) {
    using sample_type = TestType;
    using filter_type = ksv::basic_svf_filter<sample_type>;

    // The header claims the corner positions are bit-identical to the discrete modes. Pin it by
    // driving both configurations with the same signal and comparing output sample by sample.
    const int    corners[4] = {ksv::mode_lowpass, ksv::mode_bandpass, ksv::mode_highpass, ksv::mode_notch};
    const double morphs[4]  = {0.0, 0.25, 0.5, 0.75};

    for (int c = 0; c < 4; ++c) {
        auto make = [&](int mode, double morph) {
            filter_type f;
            f.prepare(static_cast<sample_type>(k_sr), 1);
            f.set_mode(mode);
            f.set_order(2);
            f.set_circuit(ksv::circuit_clean);
            f.set_frequency(static_cast<sample_type>(2000.0));
            f.set_resonance(static_cast<sample_type>(0.6));
            f.set_morph(static_cast<sample_type>(morph));
            f.snap();
            return f;
        };
        filter_type discrete = make(corners[c], 0.0);
        filter_type morphed  = make(ksv::mode_morph, morphs[c]);

        INFO("corner " << c << " (morph " << morphs[c] << ")");
        for (int i = 0; i < 500; ++i) {
            const auto x = static_cast<sample_type>(std::sin(0.05 * i) + 0.3 * std::sin(0.31 * i));
            REQUIRE(
                static_cast<double>(discrete.process(x))
                == Catch::Approx(static_cast<double>(morphed.process(x))).margin(profile<sample_type>::k_exact_tol));
        }
    }
}

TEMPLATE_LIST_TEST_CASE("the filter stays bounded under per-sample cutoff modulation", "[svf]", profiles) {
    using sample_type = TestType;
    using filter_type = ksv::basic_svf_filter<sample_type>;

    // Unconditional stability under modulation is the reason for the TPT form — no oversampling
    // trick needed. Sweep the cutoff across nearly the whole range every few hundred samples at
    // high resonance and require the output stays finite and bounded.
    for (int circuit : {ksv::circuit_clean, ksv::circuit_driven}) {
        filter_type f;
        f.prepare(static_cast<sample_type>(k_sr), 1);
        f.set_mode(ksv::mode_lowpass);
        f.set_order(8);
        f.set_circuit(circuit);
        f.set_resonance(static_cast<sample_type>(0.95));
        f.snap();

        uint32_t s    = 22222u;
        double   peak = 0.0;
        for (int i = 0; i < 200000; ++i) {
            s               = s * 1664525u + 1013904223u;
            const auto   x  = static_cast<sample_type>(static_cast<int32_t>(s) / 2147483648.0);
            const auto   fc = static_cast<sample_type>(80.0 * std::pow(200.0, 0.5 + 0.5 * std::sin(0.0013 * i)));
            const double y  = static_cast<double>(f.process(x, fc));
            REQUIRE(std::isfinite(y));
            peak = std::max(peak, std::abs(y));
        }
        INFO("circuit " << circuit << " peak " << peak);
        REQUIRE(peak < 100.0);
    }
}

TEMPLATE_LIST_TEST_CASE("the driven circuit self-oscillates and the limiter bounds it", "[svf]", profiles) {
    using sample_type = TestType;
    using filter_type = ksv::basic_svf_filter<sample_type>;

    filter_type f;
    f.prepare(static_cast<sample_type>(k_sr), 1);
    f.set_mode(ksv::mode_lowpass);
    f.set_order(2);
    f.set_circuit(ksv::circuit_driven);
    f.set_frequency(static_cast<sample_type>(500.0));
    f.set_resonance(static_cast<sample_type>(1.0));
    f.snap();

    // An all-zero state is a fixed point, per the header — it needs a ping.
    (void)f.process(static_cast<sample_type>(1.0));
    for (int i = 0; i < 40000; ++i) {
        (void)f.process(static_cast<sample_type>(0.0));
    }

    double peak = 0.0, energy = 0.0;
    for (int i = 0; i < 20000; ++i) {
        const double y = static_cast<double>(f.process(static_cast<sample_type>(0.0)));
        REQUIRE(std::isfinite(y));
        peak = std::max(peak, std::abs(y));
        energy += y * y;
    }
    THEN("it is still ringing with no input") {
        REQUIRE(std::sqrt(energy / 20000) > 0.01);
    }
    THEN("the tanh limiter keeps the amplitude bounded") {
        REQUIRE(peak < 10.0);
    }
}

TEMPLATE_LIST_TEST_CASE("state decays to exactly zero after silence", "[svf]", profiles) {
    using sample_type = TestType;
    using filter_type = ksv::basic_svf_filter<sample_type>;

    // The anti-denormal guard exists so the recursive state reaches true zero instead of
    // grinding through the denormal range. The float profile needs its own flush threshold for
    // this to hold — 1e-15 would work by accident, 1e-30 works by design.
    filter_type f;
    f.prepare(static_cast<sample_type>(k_sr), 1);
    f.set_mode(ksv::mode_lowpass);
    f.set_order(2);
    f.set_circuit(ksv::circuit_clean);
    f.set_frequency(static_cast<sample_type>(2000.0));
    f.set_resonance(static_cast<sample_type>(0.0));
    f.snap();

    (void)f.process(static_cast<sample_type>(1.0));
    for (int i = 0; i < 200000; ++i) {
        (void)f.process(static_cast<sample_type>(0.0));
    }
    REQUIRE(f.process(static_cast<sample_type>(0.0)) == sample_type(0));
}

TEMPLATE_LIST_TEST_CASE("clear() zeroes the state without touching parameters", "[svf]", profiles) {
    using sample_type = TestType;
    using filter_type = ksv::basic_svf_filter<sample_type>;

    filter_type f;
    f.prepare(static_cast<sample_type>(k_sr), 1);
    f.set_frequency(static_cast<sample_type>(3000.0));
    f.set_resonance(static_cast<sample_type>(0.7));
    f.snap();

    for (int i = 0; i < 100; ++i) {
        (void)f.process(static_cast<sample_type>(std::sin(0.1 * i)));
    }
    f.clear();

    REQUIRE(f.param(ksv::p_frequency) == static_cast<sample_type>(3000.0));
    REQUIRE(f.param(ksv::p_resonance) == static_cast<sample_type>(0.7));
    REQUIRE(f.process(static_cast<sample_type>(0.0)) == sample_type(0));
}

SCENARIO("the float profile agrees with the double golden model") {
    // The cross-precision contract: svf_filter32 runs the same algorithm as svf_filter, so its
    // output tracks the golden model to within single-precision reach. Checked on the linear
    // circuit (where the comparison is meaningful over a long window) across several modes and
    // orders; the driven circuit's tanh feedback is chaotic at high resonance and is covered by
    // the boundedness scenario instead.
    for (int mode : {ksv::mode_lowpass, ksv::mode_highpass, ksv::mode_bandpass, ksv::mode_notch}) {
        for (int order : {2, 4, 8}) {
            ksv::svf_filter   d;
            ksv::svf_filter32 s;
            auto              setup = [&](auto& f) {
                using sample_type = typename std::remove_reference_t<decltype(f)>::sample_type;
                f.prepare(static_cast<sample_type>(k_sr), 1);
                f.set_mode(mode);
                f.set_order(order);
                f.set_circuit(ksv::circuit_clean);
                f.set_frequency(static_cast<sample_type>(1500.0));
                f.set_resonance(static_cast<sample_type>(0.75));
                f.snap();
            };
            setup(d);
            setup(s);

            double worst = 0.0;
            for (int i = 0; i < 20000; ++i) {
                const double x  = std::sin(0.037 * i) + 0.4 * std::sin(0.211 * i);
                const double yd = d.process(x);
                const double ys = static_cast<double>(s.process(static_cast<float>(x)));
                worst           = std::max(worst, std::abs(yd - ys));
            }
            INFO("mode " << mode << " order " << order << " worst abs deviation " << worst);
            REQUIRE(worst < 1e-4);
        }
    }
}

SCENARIO("both profiles are exposed under the documented alias names") {
    STATIC_REQUIRE(std::is_same_v<ksv::svf_filter, ksv::basic_svf_filter<double>>);
    STATIC_REQUIRE(std::is_same_v<ksv::svf_filter32, ksv::basic_svf_filter<float>>);
    STATIC_REQUIRE(std::is_same_v<ksv::svf_filter::sample_type, double>);
    STATIC_REQUIRE(std::is_same_v<ksv::svf_filter32::sample_type, float>);
}
