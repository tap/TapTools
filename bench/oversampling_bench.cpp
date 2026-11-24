/// @file
/// @brief      CPU benchmark for the oversampled kernels: the six that run a nonlinear core between
///             Butterworth anti-image and anti-alias filters.
/// @details    ladder.h, diode_ladder.h, svf.h (the driven circuit), overdrive.h, fuzz.h and ondes.h
///             run their nonlinear core oversampled between Butterworth low-passes
///             (taptools/butterworth.h, over DspTap's basic_butterworth_lowpass). This times each
///             kernel at each of its oversampling factors, 1x included for the core alone, over 20 s
///             of audio at 48 kHz by default, with the svf_bench estimator: several repetitions, the
///             BEST time, a checksum so the optimizer cannot elide the work. The material is a 110 Hz
///             naive sawtooth at 0.5 plus a little noise, each kernel driven into its nonlinearity;
///             ondes is a source and takes none. The thread runs with flush-to-zero set, as a host
///             runs these kernels (tools/support/flush_to_zero.h).
///
///             Two figures per case: ns per input sample, and the share of one core that real time at
///             48 kHz takes, ns/sample x 48000 / 1e9 as a percentage (fuzz.h's "% of a core"
///             figures are this run's). Absolute numbers are machine-specific: compare like with like
///             (bench/README.md).
///
///             Usage: oversampling_bench [--json] [--seconds N] [--reps N] [--case NAME]
///             (--case runs the one case of that name, e.g. fuzz.os8, for an instruction counter).
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place and the TapTools contributors

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <taptools/diode_ladder.h>
#include <taptools/fuzz.h>
#include <taptools/ladder.h>
#include <taptools/ondes.h>
#include <taptools/overdrive.h>
#include <taptools/svf.h>

#include "../tools/support/flush_to_zero.h"

namespace {

    constexpr double k_sr = 48000.0;

    struct result {
        std::string name;
        double      ns_per_sample;
        double      checksum;
    };

    /// The material: a 110 Hz naive sawtooth at 0.5 plus LCG noise at 0.02.
    std::vector<double> material(size_t n) {
        std::vector<double> x(n);
        unsigned            s     = 22222u;
        double              phase = 0.0;
        for (size_t i = 0; i < n; ++i) {
            s = s * 1664525u + 1013904223u;
            phase += 110.0 / k_sr;
            phase -= (phase >= 1.0) ? 1.0 : 0.0;
            x[i] = 0.5 * (2.0 * phase - 1.0) + 0.02 * (static_cast<double>(s) / 2147483648.0 - 1.0);
        }
        return x;
    }

    /// Best of `reps` runs of a freshly constructed and set-up Kernel over the material; `step`
    /// produces one output sample from one input sample.
    template <typename Kernel, typename Setup, typename Step>
    result time_kernel(const std::string& name, const std::vector<double>& in, int reps, Setup setup, Step step) {
        double best     = 1e30;
        double checksum = 0.0;
        for (int r = 0; r < reps; ++r) {
            const auto k = std::make_unique<Kernel>();
            setup(*k);
            double     acc = 0.0;
            const auto t0  = std::chrono::steady_clock::now();
            for (const double x : in) {
                acc += step(*k, x);
            }
            const auto t1 = std::chrono::steady_clock::now();
            checksum      = acc;
            const double ns =
                std::chrono::duration<double, std::nano>(t1 - t0).count() / static_cast<double>(in.size());
            if (ns < best) {
                best = ns;
            }
        }
        return {name, best, checksum};
    }

} // namespace

int main(int argc, char** argv) {
    bool        json    = false;
    double      seconds = 20.0;
    int         reps    = 5;
    std::string only;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--json")) {
            json = true;
        }
        else if (!std::strcmp(argv[i], "--case") && i + 1 < argc) {
            only = argv[++i];
        }
        else if (!std::strcmp(argv[i], "--seconds") && i + 1 < argc) {
            seconds = std::atof(argv[++i]);
        }
        else if (!std::strcmp(argv[i], "--reps") && i + 1 < argc) {
            reps = std::atoi(argv[++i]);
        }
    }

    // The host's job (tap/dsp/subnormals.h): flush-to-zero on the thread that runs the kernels.
    const tap::tools::support::flush_to_zero_scope ftz;

    namespace dio = tap::tools::diode;
    namespace fz  = tap::tools::fuzz;
    namespace lad = tap::tools::ladder;
    namespace od  = tap::tools::od;
    namespace on  = tap::tools::ondes;
    namespace ksv = tap::tools::svf;

    const std::vector<double>                                    in = material(static_cast<size_t>(seconds * k_sr));
    std::vector<std::pair<std::string, std::function<result()>>> cases;
    const auto name = [](const char* kernel, int os) { return std::string(kernel) + ".os" + std::to_string(os); };
    const auto filter_step = [](auto& f, double x) { return f.process(x); };

    for (const int os : {1, 2, 4}) {
        cases.emplace_back(name("ladder", os), [&, os] {
            return time_kernel<lad::ladder_filter>(
                name("ladder", os), in, reps,
                [os](lad::ladder_filter& f) {
                    f.prepare(k_sr);
                    f.set_smooth_ms(0.0);
                    f.set_oversample(os);
                    f.set_frequency(1200.0);
                    f.set_resonance(0.6);
                    f.set_drive(12.0);
                },
                filter_step);
        });
    }
    for (const int os : {1, 2, 4}) {
        cases.emplace_back(name("diode", os), [&, os] {
            return time_kernel<dio::diode_filter>(
                name("diode", os), in, reps,
                [os](dio::diode_filter& f) {
                    f.prepare(k_sr);
                    f.set_smooth_ms(0.0);
                    f.set_oversample(os);
                    f.set_frequency(1200.0);
                    f.set_resonance(0.6);
                    f.set_drive(12.0);
                },
                filter_step);
        });
    }
    for (const int os : {1, 2, 4}) {
        cases.emplace_back(name("svf.driven", os), [&, os] {
            return time_kernel<ksv::svf_filter>(
                name("svf.driven", os), in, reps,
                [os](ksv::svf_filter& f) {
                    f.prepare(k_sr, 1);
                    f.set_smooth_ms(0.0);
                    f.set_circuit(ksv::circuit_driven);
                    f.set_oversample(os);
                    f.set_frequency(1200.0);
                    f.set_resonance(0.6);
                    f.set_drive(12.0);
                },
                filter_step);
        });
    }
    for (const int os : {1, 2, 4, 8}) {
        cases.emplace_back(name("overdrive", os), [&, os] {
            return time_kernel<od::overdrive>(
                name("overdrive", os), in, reps,
                [os](od::overdrive& f) {
                    f.prepare(k_sr, 1);
                    f.set_smooth_ms(0.0);
                    f.set_oversample(os);
                    f.set_drive(0.7);
                    f.set_asymmetry(0.3);
                },
                filter_step);
        });
    }
    for (const int os : {1, 2, 4, 8}) {
        cases.emplace_back(name("fuzz", os), [&, os] {
            return time_kernel<fz::pedal>(
                name("fuzz", os), in, reps,
                [os](fz::pedal& f) {
                    f.set_smooth_ms(0.0);
                    f.prepare(k_sr);
                    f.set_oversample(os);
                    f.set_gain(0.8);
                    f.set_edge(0.5);
                    f.set_asymmetry(0.3);
                },
                filter_step);
        });
    }
    for (const int os : {1, 2, 4, 8}) {
        cases.emplace_back(name("ondes", os), [&, os] {
            return time_kernel<on::voice>(
                name("ondes", os), in, reps,
                [os](on::voice& v) {
                    v.set_smooth_ms(0.0);
                    v.prepare(k_sr);
                    v.set_oversample(os);
                    v.set_frequency(220.0);
                    v.set_drive(2.0);
                    v.set_key(0.8);
                },
                [](on::voice& v, double) { return v.process(); });
        });
    }

    std::vector<result> results;
    for (const auto& [case_name, run] : cases) {
        if (only.empty() || only == case_name) {
            results.push_back(run());
        }
    }

    if (json) {
        std::printf("{\n  \"samplerate\": %.0f,\n  \"cases\": {\n", k_sr);
        for (size_t i = 0; i < results.size(); ++i) {
            std::printf("    \"%s\": { \"ns_per_sample\": %.3f }%s\n", results[i].name.c_str(),
                        results[i].ns_per_sample, i + 1 < results.size() ? "," : "");
        }
        std::printf("  }\n}\n");
    }
    else {
        std::printf("%-16s %12s %12s %22s\n", "case", "ns/sample", "% of a core", "checksum");
        for (const auto& r : results) {
            const double share = r.ns_per_sample * k_sr * 1e-9 * 100.0;
            std::printf("%-16s %12.2f %12.3f %22.12f\n", r.name.c_str(), r.ns_per_sample, share, r.checksum);
        }
    }
    return 0;
}
