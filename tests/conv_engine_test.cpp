/// @file
/// @brief      Unit tests for the partitioned convolution engine behind tap.convolve~.
/// @details    `tap::tools::conv_engine` is a pure-C++ kernel, so these tests drive it directly — no
///             Max, min-api, or mock kernel needed — and check its output against a plain
///             time-domain convolution computed here as the reference.
// SPDX-License-Identifier: MIT
// Copyright 2003-2026 Timothy Place.

#include <atomic>
#include <cmath>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <taptools/conv_engine.h>

namespace {

    // Deterministic pseudo-random sequence in [-1, 1] (a small LCG — no <random> needed, reproducible).
    std::vector<double> noise(int n, unsigned seed) {
        std::vector<double> v(n);
        unsigned            s = seed;
        for (int i = 0; i < n; ++i) {
            s    = s * 1664525u + 1013904223u;
            v[i] = (static_cast<double>(s) / 4294967295.0) * 2.0 - 1.0;
        }
        return v;
    }

    // Direct (naive) linear convolution reference: out[t] = sum_k h[k] * x[t - k].
    std::vector<double> direct_convolve(const std::vector<double>& x, const std::vector<float>& h) {
        std::vector<double> out(x.size(), 0.0);
        for (int t = 0; t < static_cast<int>(x.size()); ++t) {
            double acc = 0.0;
            for (int k = 0; k < static_cast<int>(h.size()); ++k) {
                if (t - k >= 0) {
                    acc += static_cast<double>(h[k]) * x[t - k];
                }
            }
            out[t] = acc;
        }
        return out;
    }

} // namespace

SCENARIO("the engine reproduces a true-stereo convolution, latency-shifted by one block") {
    const int B    = 16;  // partition size (power of two)
    const int Pmax = 8;   // capacity 128 samples
    const int len  = 40;  // IR length -> 3 partitions
    const int n    = 300; // signal length

    tap::tools::conv_engine engine;
    engine.configure(B, Pmax);

    GIVEN("four distinct IR paths and two distinct input channels") {
        // Independent IRs on every path so cross-terms can't cancel a bug.
        std::vector<float> h_ll(len), h_lr(len), h_rl(len), h_rr(len);
        for (int k = 0; k < len; ++k) {
            h_ll[k] = static_cast<float>(std::sin(0.13 * k) * std::exp(-0.04 * k));
            h_lr[k] = static_cast<float>(std::cos(0.21 * k) * std::exp(-0.05 * k));
            h_rl[k] = static_cast<float>(0.5 * std::sin(0.30 * k) * std::exp(-0.06 * k));
            h_rr[k] = static_cast<float>(std::cos(0.17 * k) * std::exp(-0.03 * k));
        }
        const float* paths[4] = {h_ll.data(), h_lr.data(), h_rl.data(), h_rr.data()};
        engine.load_ir(paths, len, 1.0);

        const std::vector<double> x_l = noise(n, 12345u);
        const std::vector<double> x_r = noise(n, 99999u);

        // Reference: out_l = x_l*h_ll + x_r*h_rl ; out_r = x_l*h_lr + x_r*h_rr (no latency shift yet).
        const std::vector<double> ref_ll = direct_convolve(x_l, h_ll);
        const std::vector<double> ref_rl = direct_convolve(x_r, h_rl);
        const std::vector<double> ref_lr = direct_convolve(x_l, h_lr);
        const std::vector<double> ref_rr = direct_convolve(x_r, h_rr);

        WHEN("processed in awkward chunk sizes that straddle block boundaries") {
            std::vector<double> out_l(n, 0.0), out_r(n, 0.0);
            int                 i     = 0;
            const int           chunk = 10; // not a divisor of B — exercises mid-vector block crossing
            while (i < n) {
                const long c = std::min<long>(chunk, n - i);
                engine.process(&x_l[i], &x_r[i], &out_l[i], &out_r[i], c);
                i += c;
            }

            THEN("the output equals the reference convolution delayed by exactly one block (B samples)") {
                bool ok = true;
                for (int t = B; t < n; ++t) {
                    const double exp_l = ref_ll[t - B] + ref_rl[t - B];
                    const double exp_r = ref_lr[t - B] + ref_rr[t - B];
                    if (std::abs(out_l[t] - exp_l) > 1e-9) {
                        ok = false;
                    }
                    if (std::abs(out_r[t] - exp_r) > 1e-9) {
                        ok = false;
                    }
                }
                REQUIRE(ok);
            }
            THEN("the first block of output is the pre-roll latency (silence)") {
                bool silent = true;
                for (int t = 0; t < B; ++t) {
                    if (std::abs(out_l[t]) > 1e-12 || std::abs(out_r[t]) > 1e-12) {
                        silent = false;
                    }
                }
                REQUIRE(silent);
            }
        }
    }
}

SCENARIO("a delayed unit impulse on one path is a pure delay on that path only") {
    const int B    = 8;
    const int Pmax = 8;

    tap::tools::conv_engine engine;
    engine.configure(B, Pmax);

    GIVEN("h_ll = delta at index 5, all other paths silent") {
        std::vector<float> h_ll(16, 0.0f);
        h_ll[5]               = 1.0f;
        const float* paths[4] = {h_ll.data(), nullptr, nullptr, nullptr};
        engine.load_ir(paths, 16, 1.0);

        const int                 n   = 128;
        const std::vector<double> x_l = noise(n, 7u);
        const std::vector<double> x_r = noise(n, 8u);

        WHEN("processed") {
            std::vector<double> out_l(n, 0.0), out_r(n, 0.0);
            engine.process(x_l.data(), x_r.data(), out_l.data(), out_r.data(), n);

            THEN("out_l is in_l delayed by (block latency + 5), and out_r is silent") {
                const int delay = B + 5;
                bool      ok_l = true, ok_r = true;
                for (int t = delay; t < n; ++t) {
                    if (std::abs(out_l[t] - x_l[t - delay]) > 1e-9) {
                        ok_l = false;
                    }
                }
                for (int t = 0; t < n; ++t) {
                    if (std::abs(out_r[t]) > 1e-12) {
                        ok_r = false;
                    }
                }
                REQUIRE(ok_l);
                REQUIRE(ok_r);
            }
        }
    }
}

SCENARIO("with no IR loaded the engine is silent") {
    tap::tools::conv_engine engine;
    engine.configure(32, 4);
    REQUIRE(engine.has_ir() == false);

    const int                 n   = 96;
    const std::vector<double> x_l = noise(n, 1u);
    const std::vector<double> x_r = noise(n, 2u);
    std::vector<double>       out_l(n, 0.0), out_r(n, 0.0);
    engine.process(x_l.data(), x_r.data(), out_l.data(), out_r.data(), n);

    bool silent = true;
    for (int t = 0; t < n; ++t) {
        if (std::abs(out_l[t]) > 1e-12 || std::abs(out_r[t]) > 1e-12) {
            silent = false;
        }
    }
    REQUIRE(silent);
}

SCENARIO("a hot IR swap while running settles to the new response") {
    const int B    = 16;
    const int Pmax = 8;

    tap::tools::conv_engine engine;
    engine.configure(B, Pmax);

    GIVEN("an initial decaying IR on the diagonal paths, running mid-stream") {
        std::vector<float> h1(20), h2(20);
        for (int k = 0; k < 20; ++k) {
            h1[k] = static_cast<float>(std::exp(-0.1 * k));
            h2[k] = (k == 3) ? 1.0f : 0.0f; // the swap target: a delta at index 3
        }
        const float* pa[4] = {h1.data(), nullptr, nullptr, h1.data()};
        engine.load_ir(pa, 20, 1.0);

        const int                 n   = 200;
        const std::vector<double> x_l = noise(n, 5u);
        const std::vector<double> x_r = noise(n, 6u);
        std::vector<double>       out_l(n, 0.0), out_r(n, 0.0);

        WHEN("a new IR is published atomically partway through the stream") {
            engine.process(x_l.data(), x_r.data(), out_l.data(), out_r.data(), 100);
            const float* pb[4] = {h2.data(), nullptr, nullptr, h2.data()};
            engine.load_ir(pb, 20, 1.0); // lock-free slot swap
            engine.process(&x_l[100], &x_r[100], &out_l[100], &out_r[100], 100);

            THEN("after the swap settles, out_l is in_l delayed by (block latency + 3)") {
                const int delay = B + 3;
                bool      ok    = true;
                for (int t = 150; t < n; ++t) {
                    if (std::abs(out_l[t] - x_l[t - delay]) > 1e-9) {
                        ok = false;
                    }
                }
                REQUIRE(ok);
            }
        }
    }
}

SCENARIO("configure refuses a block size the FFT engine does not support and keeps the previous geometry") {
    tap::tools::conv_engine engine;

    GIVEN("an engine configured at a supported size") {
        REQUIRE(engine.configure(16, 8));
        REQUIRE(engine.block_size() == 16);
        REQUIRE(engine.max_partitions() == 8);

        THEN("a block size whose doubled FFT size is not a power of two is refused, unchanged") {
            REQUIRE_FALSE(engine.configure(48, 8)); // N = 96: a heap overflow in DspTap's constructor if built
            REQUIRE_FALSE(engine.configure(24, 8)); // N = 48
            REQUIRE_FALSE(engine.configure(0, 8));
            REQUIRE_FALSE(engine.configure(-16, 8));
            REQUIRE_FALSE(engine.configure(1, 8)); // N = 2 is below the engine's minimum of 4
            REQUIRE_FALSE(engine.configure(16, 0));
            REQUIRE(engine.block_size() == 16);
            REQUIRE(engine.max_partitions() == 8);
            REQUIRE(engine.configured());
        }
        THEN("the predicate names exactly the accepted sizes") {
            REQUIRE(tap::tools::conv_engine::supports_block_size(2));
            REQUIRE(tap::tools::conv_engine::supports_block_size(1024));
            REQUIRE_FALSE(tap::tools::conv_engine::supports_block_size(1));
            REQUIRE_FALSE(tap::tools::conv_engine::supports_block_size(3));
            REQUIRE_FALSE(tap::tools::conv_engine::supports_block_size(1000));
        }
    }

    GIVEN("an engine that was never configured") {
        THEN("process emits silence instead of touching unallocated buffers, and load_ir is refused") {
            const std::vector<double> x(64, 1.0);
            std::vector<double>       out_l(64, 7.0), out_r(64, 7.0);
            engine.process(x.data(), x.data(), out_l.data(), out_r.data(), 64);
            bool silent = true;
            for (int i = 0; i < 64; ++i) {
                silent = silent && out_l[i] == 0.0 && out_r[i] == 0.0;
            }
            REQUIRE(silent);
            const float  h        = 1.0f;
            const float* paths[4] = {&h, nullptr, nullptr, &h};
            REQUIRE_FALSE(engine.load_ir(paths, 1, 1.0));
        }
    }
}

SCENARIO("an IR load on another thread never corrupts the audio thread's output or the published IR") {
    // Two one-tap IRs, gains 1 and 2, on the diagonal paths; a constant input of 1. Every output
    // sample is then exactly the gain of whichever IR its block saw: 1 or 2, never anything else.
    // Before load_ir had its own scratch buffer the two threads transformed through one buffer, so
    // a load during a block corrupted either the IR partition being built or the audio frame.
    const int B = 32;
    const int n = 8 * 1024;

    tap::tools::conv_engine engine;
    REQUIRE(engine.configure(B, 2));
    const float  g1 = 1.0f, g2 = 2.0f;
    const float* ir1[4] = {&g1, nullptr, nullptr, &g1};
    const float* ir2[4] = {&g2, nullptr, nullptr, &g2};
    REQUIRE(engine.load_ir(ir1, 1, 1.0));

    GIVEN("a control thread swapping the two IRs as fast as it can while audio runs") {
        std::atomic<bool> stop{false};
        std::atomic<int>  loads{0};
        std::thread       loader([&] {
            while (!stop.load(std::memory_order_relaxed)) {
                if (engine.load_ir(ir1, 1, 1.0)) {
                    loads.fetch_add(1, std::memory_order_relaxed);
                }
                if (engine.load_ir(ir2, 1, 1.0)) {
                    loads.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });

        const std::vector<double> x(n, 1.0);
        std::vector<double>       out_l(n, 0.0), out_r(n, 0.0);
        int                       bad = 0;
        for (int round = 0; round < 40; ++round) {
            engine.process(x.data(), x.data(), out_l.data(), out_r.data(), n);
            for (int t = B; t < n; ++t) { // the first block of the first round is the latency
                const bool ok_l = std::abs(out_l[t] - 1.0) < 1e-9 || std::abs(out_l[t] - 2.0) < 1e-9;
                const bool ok_r = std::abs(out_r[t] - 1.0) < 1e-9 || std::abs(out_r[t] - 2.0) < 1e-9;
                if (!ok_l || !ok_r) {
                    ++bad;
                }
            }
        }
        stop.store(true, std::memory_order_relaxed);
        loader.join();

        THEN("every output sample is one of the two gains and the loads were not starved") {
            REQUIRE(bad == 0);
            REQUIRE(loads.load() > 0);
        }
    }
}
