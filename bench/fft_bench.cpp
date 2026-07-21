/// @file
/// @brief      Before/after CPU benchmark for the spectral-kernel FFT swap.
/// @details    The spectral kernels (conv_engine / stft / nr / spectra) used to carry a private
///             complex radix-2 Cooley–Tukey FFT; they now share DspTap's real FFT (tap::dsp::real_fft
///             — the split-radix Ooura transform, plus vDSP / CMSIS-Helium float32 backends on Apple
///             and Arm). This times a forward+inverse round trip of each, over the FFT sizes the
///             kernels actually use (a convolution partition of B samples runs a 2B-point transform),
///             so the ratio is the per-transform speedup on this host.
///
///             The "before" path is a verbatim copy of the retired radix-2 routine (double, full
///             complex, engineering convention, inverse /N) kept HERE only as the benchmark baseline —
///             the kernels no longer contain it. On this Linux/x86 host the win is the split-radix
///             algorithm plus doing a real (not complex) transform; on Apple/Arm the vDSP/CMSIS
///             backends add a further ~3x per transform that this desktop build does not exercise.
///
///             Usage: fft_bench [--json] [--reps N]
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <tap/dsp/fft.h>

namespace {

    constexpr double k_pi = 3.14159265358979323846;

    // The retired complex radix-2 Cooley–Tukey FFT (baseline only — see file header).
    void old_transform(std::vector<double>& re, std::vector<double>& im, bool inverse) {
        const int n = static_cast<int>(re.size());
        for (int i = 1, j = 0; i < n; ++i) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) {
                j ^= bit;
            }
            j ^= bit;
            if (i < j) {
                std::swap(re[i], re[j]);
                std::swap(im[i], im[j]);
            }
        }
        for (int len = 2; len <= n; len <<= 1) {
            const double ang = 2.0 * k_pi / len * (inverse ? 1.0 : -1.0);
            const double wr = std::cos(ang), wi = std::sin(ang);
            for (int i = 0; i < n; i += len) {
                double cwr = 1.0, cwi = 0.0;
                for (int k = 0; k < len / 2; ++k) {
                    const double vr   = re[i + k + len / 2] * cwr - im[i + k + len / 2] * cwi;
                    const double vi   = re[i + k + len / 2] * cwi + im[i + k + len / 2] * cwr;
                    const double ur   = re[i + k];
                    const double ui   = im[i + k];
                    re[i + k]         = ur + vr;
                    im[i + k]         = ui + vi;
                    re[i + k + len / 2] = ur - vr;
                    im[i + k + len / 2] = ui - vi;
                    const double ncwr = cwr * wr - cwi * wi;
                    cwi               = cwr * wi + cwi * wr;
                    cwr               = ncwr;
                }
            }
        }
        if (inverse) {
            for (int i = 0; i < n; ++i) {
                re[i] /= n;
                im[i] /= n;
            }
        }
    }

    std::vector<double> noise(int n, unsigned seed) {
        std::vector<double> v(n);
        unsigned            s = seed;
        for (int i = 0; i < n; ++i) {
            s    = s * 1664525u + 1013904223u;
            v[i] = (static_cast<double>(s) / 2147483648.0 - 1.0);
        }
        return v;
    }

    // Best-of-reps nanoseconds for one forward+inverse round trip at size n.
    struct pair_result {
        double old_ns;
        double new_ns;
        double checksum;
    };

    pair_result run_size(int n, int reps, long iters) {
        const auto x = noise(n, 4321u + static_cast<unsigned>(n));

        double best_old = 1e30, best_new = 1e30, checksum = 0.0;

        // Old complex radix-2 round trip.
        for (int r = 0; r < reps; ++r) {
            std::vector<double> re(n), im(n);
            double              acc = 0.0;
            const auto          t0  = std::chrono::steady_clock::now();
            for (long it = 0; it < iters; ++it) {
                re.assign(x.begin(), x.end());
                std::fill(im.begin(), im.end(), 0.0);
                old_transform(re, im, false);
                old_transform(re, im, true);
                acc += re[it % n];
            }
            const auto   t1 = std::chrono::steady_clock::now();
            const double ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / static_cast<double>(iters);
            best_old        = std::min(best_old, ns);
            checksum += acc;
        }

        // DspTap real FFT round trip.
        tap::dsp::real_fft fft(static_cast<size_t>(n));
        for (int r = 0; r < reps; ++r) {
            std::vector<double> a(n);
            double              acc = 0.0;
            const auto          t0  = std::chrono::steady_clock::now();
            for (long it = 0; it < iters; ++it) {
                a.assign(x.begin(), x.end());
                fft.forward_inplace(a.data());
                fft.inverse_inplace(a.data());
                acc += a[it % n];
            }
            const auto   t1 = std::chrono::steady_clock::now();
            const double ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / static_cast<double>(iters);
            best_new        = std::min(best_new, ns);
            checksum += acc;
        }

        return {best_old, best_new, checksum};
    }

} // namespace

int main(int argc, char** argv) {
    bool json = false;
    int  reps = 5;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--json")) {
            json = true;
        }
        else if (!std::strcmp(argv[i], "--reps") && i + 1 < argc) {
            reps = std::atoi(argv[++i]);
        }
    }

    const std::vector<int> sizes = {256, 512, 1024, 2048, 4096};

    if (json) {
        std::printf("{\n  \"cases\": {\n");
    }
    else {
        std::printf("%-8s %14s %14s %10s %14s\n", "N", "old ns/xform", "new ns/xform", "speedup", "checksum");
    }
    for (size_t s = 0; s < sizes.size(); ++s) {
        const int  n     = sizes[s];
        const long iters = 2'000'000L / n; // constant total work across sizes
        const auto res   = run_size(n, reps, iters);
        if (json) {
            std::printf("    \"n%d\": { \"old_ns\": %.1f, \"new_ns\": %.1f, \"speedup\": %.2f }%s\n", n, res.old_ns,
                        res.new_ns, res.old_ns / res.new_ns, s + 1 < sizes.size() ? "," : "");
        }
        else {
            std::printf("%-8d %14.1f %14.1f %9.2fx %14.3f\n", n, res.old_ns, res.new_ns, res.old_ns / res.new_ns,
                        res.checksum);
        }
    }
    if (json) {
        std::printf("  }\n}\n");
    }
    return 0;
}
