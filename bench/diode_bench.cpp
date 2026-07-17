/// @file
/// @brief      CPU benchmark for the tap.diode~ kernel (diode_ladder.h) — no Max dependency.
/// @details    Times the kernel over a matrix of configurations (solver x oversampling, static
///             vs signal-rate-modulated cutoff) on deterministic noise, with the svf_bench
///             estimator: several repetitions, report the BEST time, accumulate a checksum so
///             the optimizer cannot elide the work. Same ratchet workflow as svf_bench —
///             --json + compare.py against the committed baseline.
///
///             Usage: diode_bench [--json] [--seconds N] [--reps N]
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include <taptools/diode_ladder.h>

namespace dio = taptools::diode;

namespace {

    constexpr double k_sr = 48000.0;

    struct bench_case {
        std::string                             name;
        std::function<void(dio::diode_filter&)> setup;
        bool modulated; // true: per-sample process(x, cutoff) — the signal-rate-cutoff path
    };

    // Deterministic LCG noise in [-0.25, 0.25].
    struct noise {
        unsigned s{22222u};
        double   next() {
            s = s * 1664525u + 1013904223u;
            return (static_cast<double>(s) / 2147483648.0 - 1.0) * 0.25;
        }
    };

    struct result {
        std::string name;
        double      ns_per_sample;
        double      checksum;
    };

    result run_case(const bench_case& bc, double seconds, int reps) {
        const size_t n = static_cast<size_t>(seconds * k_sr);

        // Pre-generate input (and the modulation ramp) so we time the filter, not the generator.
        std::vector<double> in(n), cf(n);
        noise               ng;
        for (size_t i = 0; i < n; ++i) {
            in[i] = ng.next();
            // 20 Hz exponential cutoff sweep 200..8000 Hz — a demanding but musical modulator
            const double ph = std::fmod(20.0 * i / k_sr, 1.0);
            cf[i]           = 200.0 * std::pow(40.0, ph < 0.5 ? 2.0 * ph : 2.0 - 2.0 * ph);
        }

        double best     = 1e30;
        double checksum = 0.0;
        for (int r = 0; r < reps; ++r) {
            dio::diode_filter f;
            f.prepare(k_sr);
            f.set_smooth_ms(0.0);
            bc.setup(f);

            double     acc = 0.0;
            const auto t0  = std::chrono::steady_clock::now();
            if (bc.modulated) {
                for (size_t i = 0; i < n; ++i) {
                    acc += f.process(in[i], cf[i]);
                }
            }
            else {
                for (size_t i = 0; i < n; ++i) {
                    acc += f.process(in[i]);
                }
            }
            const auto t1   = std::chrono::steady_clock::now();
            checksum        = acc;
            const double ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / static_cast<double>(n);
            if (ns < best) {
                best = ns;
            }
        }
        return {bc.name, best, checksum};
    }

} // namespace

int main(int argc, char** argv) {
    bool   json    = false;
    double seconds = 4.0;
    int    reps    = 5;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--json")) {
            json = true;
        }
        else if (!std::strcmp(argv[i], "--seconds") && i + 1 < argc) {
            seconds = std::atof(argv[++i]);
        }
        else if (!std::strcmp(argv[i], "--reps") && i + 1 < argc) {
            reps = std::atoi(argv[++i]);
        }
    }

    auto cfg = [](int solver, int os, double res, double drive) {
        return [=](dio::diode_filter& f) {
            f.set_solver(solver);
            f.set_oversample(os);
            f.set_frequency(1000.0);
            f.set_resonance(res);
            f.set_drive(drive);
        };
    };

    const std::vector<bench_case> cases = {
        {"fast.os1.static", cfg(dio::solver_fast, 1, 0.5, 0.0), false},
        {"fast.os2.static", cfg(dio::solver_fast, 2, 0.5, 0.0), false},
        {"fast.os4.static", cfg(dio::solver_fast, 4, 0.5, 0.0), false},
        {"fast.os2.mod", cfg(dio::solver_fast, 2, 0.5, 0.0), true},
        {"fast.os2.hot", cfg(dio::solver_fast, 2, 1.4, 18.0), false},
        {"exact.os2.static", cfg(dio::solver_exact, 2, 0.5, 0.0), false},
        {"exact.os2.mod", cfg(dio::solver_exact, 2, 0.5, 0.0), true},
        {"exact.os2.hot", cfg(dio::solver_exact, 2, 1.4, 18.0), false},
    };

    std::vector<result> results;
    for (const auto& bc : cases) {
        results.push_back(run_case(bc, seconds, reps));
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
        std::printf("%-24s %14s %12s %14s\n", "case", "ns/sample", "x realtime", "checksum");
        for (const auto& r : results) {
            const double xrt = 1e9 / (r.ns_per_sample * k_sr);
            std::printf("%-24s %14.2f %12.0f %14.6f\n", r.name.c_str(), r.ns_per_sample, xrt, r.checksum);
        }
    }
    return 0;
}
