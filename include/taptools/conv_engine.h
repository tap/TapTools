/// @file
/// @brief      conv_engine — a Min-free, uniformly-partitioned overlap-save convolution engine.
/// @details    The DSP core of tap.convolve~, kept deliberately independent of Max/Min so it can be
///             unit-tested directly against a reference convolution (see kernel/tests/conv_engine_test.cpp)
///             — buffer~-backed Min objects can't link against the mock test kernel, so the portable
///             engine lives here on its own. Plain C++20: no Jamoma, no min-lib.
///
///             Uniformly-partitioned overlap-save: the impulse response is split into equal blocks,
///             each transformed once with the shared DspTap real FFT (tap::dsp::real_fft); the running
///             output is a frequency-domain multiply-accumulate over a frequency-domain delay line
///             (FDL) of past input spectra. Latency is one partition; everything else is exact linear
///             convolution.
///
///             True stereo — four IR paths give the full 2×2 response, with the two input channels
///             transformed once per block and shared across the paths:
///                 out_l = in_l ∗ h_LL + in_r ∗ h_RL ;  out_r = in_l ∗ h_LR + in_r ∗ h_RR
///             Path indexing: path = in_channel * 2 + out_channel, i.e. 0=LL, 1=LR, 2=RL, 3=RR.
///
///             Spectra are stored in the real FFT's packed half-spectrum layout (N reals for an
///             N-point transform: DC in slot 0, Nyquist in slot 1, then re/im pairs for bins
///             1..N/2-1). That halves both the spectral storage and the multiply-accumulate versus a
///             full-complex form, and — because a product of two Hermitian spectra is Hermitian — the
///             convolution is identical to the full-complex version (the exp(+i) sign convention and
///             the 2/N inverse cancel through the forward→multiply→inverse pipeline).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <optional>
#include <vector>

#include "tap/dsp/fft.h" // the shared DspTap real FFT (split-radix Ooura + vDSP / CMSIS backends)

namespace tap::tools {

    class conv_engine {
      public:
        static constexpr int k_paths    = 4;
        static constexpr int k_channels = 2;

        // Allocate all buffers for the given partition size and maximum partition count. Must be called
        // where the audio thread is not running concurrently (i.e. from dspsetup). Resets all state and
        // discards any loaded IR.
        void configure(int blocksize, int max_partitions) {
            m_block     = std::max(1, blocksize);
            m_fftsize   = 2 * m_block;
            m_max_parts = std::max(1, max_partitions);
            m_fft.emplace(static_cast<size_t>(m_fftsize));

            const int flat = m_max_parts * m_fftsize;
            for (int path = 0; path < k_paths; ++path) {
                for (int slot = 0; slot < 2; ++slot) {
                    m_ir[path][slot].assign(flat, 0.0);
                }
            }
            for (int ch = 0; ch < k_channels; ++ch) {
                m_fdl[ch].assign(flat, 0.0);
                m_prev[ch].assign(m_block, 0.0);
                m_inblk[ch].assign(m_block, 0.0);
                m_outblk[ch].assign(m_block, 0.0);
            }
            m_fbuf.assign(m_fftsize, 0.0);
            m_acc.assign(m_fftsize, 0.0);

            m_slot_parts[0] = m_slot_parts[1] = 0;
            m_active.store(-1, std::memory_order_release); // no IR yet
            clear();
        }

        // Flush the running state (input history + pending output) without discarding the loaded IR.
        // Zeroes buffers the audio thread reads; safe to call from a message handler (no reallocation).
        void clear() {
            for (int ch = 0; ch < k_channels; ++ch) {
                std::fill(m_fdl[ch].begin(), m_fdl[ch].end(), 0.0);
                std::fill(m_prev[ch].begin(), m_prev[ch].end(), 0.0);
                std::fill(m_inblk[ch].begin(), m_inblk[ch].end(), 0.0);
                std::fill(m_outblk[ch].begin(), m_outblk[ch].end(), 0.0);
            }
            m_pos     = 0;
            m_fdl_pos = 0;
        }

        int  block_size() const { return m_block; }
        int  max_partitions() const { return m_max_parts; }
        bool configured() const { return m_block > 0 && !m_ir[0][0].empty(); }
        bool has_ir() const { return m_active.load(std::memory_order_acquire) >= 0; }

        // Build the four IR paths into the currently-inactive slot and publish it atomically. `paths` holds
        // four source pointers (any may be null for a silent path); `length` is the IR length in samples
        // (clamped to capacity); `scale` is applied to every sample (used for normalisation / gain).
        // Runs off the audio thread; only writes the inactive slot, then flips one atomic.
        void load_ir(const float* const paths[k_paths], int length, double scale) {
            if (!configured()) {
                return;
            }

            const int a        = m_active.load(std::memory_order_acquire);
            const int inactive = (a < 0) ? 0 : (1 - a);

            const int P = std::min(m_max_parts, (std::max(0, length) + m_block - 1) / m_block);

            for (int path = 0; path < k_paths; ++path) {
                const float* src = paths[path];
                double*      H   = m_ir[path][inactive].data();

                for (int p = 0; p < P; ++p) {
                    std::fill(m_fbuf.begin(), m_fbuf.end(), 0.0);
                    if (src) {
                        for (int j = 0; j < m_block; ++j) {
                            const int idx = p * m_block + j;
                            if (idx < length) {
                                m_fbuf[j] = static_cast<double>(src[idx]) * scale;
                            }
                        }
                    }
                    m_fft->forward_inplace(m_fbuf.data());
                    std::copy(m_fbuf.begin(), m_fbuf.end(), H + p * m_fftsize);
                }
            }

            m_slot_parts[inactive] = P;                          // written before the publish...
            m_active.store(inactive, std::memory_order_release); // ...so (slot, P) stay consistent.
        }

        // Process n stereo samples. Wet (fully convolved) output is written to out_l/out_r. Safe for any n;
        // input/output must not alias each other.
        void process(const double* in_l, const double* in_r, double* out_l, double* out_r, long n) {
            for (long i = 0; i < n; ++i) {
                m_inblk[0][m_pos] = in_l[i];
                m_inblk[1][m_pos] = in_r[i];
                out_l[i]          = m_outblk[0][m_pos];
                out_r[i]          = m_outblk[1][m_pos];
                if (++m_pos == m_block) {
                    process_block();
                    m_pos = 0;
                }
            }
        }

      private:
        // Multiply-accumulate one partition into the packed accumulator: acc += h * x, with DC (slot 0)
        // and Nyquist (slot 1) purely real and the interior bins full complex.
        void mac_partition(const double* h, const double* x) {
            const int half = m_fftsize / 2;
            m_acc[0] += h[0] * x[0]; // DC
            m_acc[1] += h[1] * x[1]; // Nyquist
            for (int k = 1; k < half; ++k) {
                const double hr = h[2 * k];
                const double hi = h[2 * k + 1];
                const double xr = x[2 * k];
                const double xi = x[2 * k + 1];
                m_acc[2 * k] += hr * xr - hi * xi;
                m_acc[2 * k + 1] += hr * xi + hi * xr;
            }
        }

        // Transform this block's two input frames, store them in the frequency-domain delay line, and form
        // the four-path multiply-accumulate for both outputs.
        void process_block() {
            const int cur = m_fdl_pos;

            // 1. Analyse both input channels into the FDL (overlap-save frame = [prev block ; this block]).
            for (int ch = 0; ch < k_channels; ++ch) {
                for (int j = 0; j < m_block; ++j) {
                    m_fbuf[j]           = m_prev[ch][j];
                    m_fbuf[m_block + j] = m_inblk[ch][j];
                }
                m_fft->forward_inplace(m_fbuf.data());
                std::copy(m_fbuf.begin(), m_fbuf.end(), m_fdl[ch].data() + cur * m_fftsize);
                std::copy(m_inblk[ch].begin(), m_inblk[ch].end(), m_prev[ch].begin());
            }

            const int a = m_active.load(std::memory_order_acquire);
            if (a < 0) {
                // No IR loaded — emit silence.
                for (int ch = 0; ch < k_channels; ++ch) {
                    std::fill(m_outblk[ch].begin(), m_outblk[ch].end(), 0.0);
                }
                m_fdl_pos = (cur + 1) % m_max_parts;
                return;
            }
            const int P = m_slot_parts[a];

            // 2. For each output channel, accumulate the frequency-domain product over both input channels
            //    and every partition, then inverse-transform and keep the non-aliased second half.
            const double scale = 2.0 / static_cast<double>(m_fftsize); // completes the real inverse
            for (int oc = 0; oc < k_channels; ++oc) {
                std::fill(m_acc.begin(), m_acc.end(), 0.0);

                for (int ic = 0; ic < k_channels; ++ic) {
                    const int     path = ic * 2 + oc;
                    const double* H    = m_ir[path][a].data();
                    const double* X    = m_fdl[ic].data();

                    for (int p = 0; p < P; ++p) {
                        int slot = cur - p;
                        if (slot < 0) {
                            slot += m_max_parts;
                        }
                        mac_partition(H + p * m_fftsize, X + slot * m_fftsize);
                    }
                }

                m_fft->inverse_inplace(m_acc.data()); // unscaled; the 2/N below completes the round trip
                for (int j = 0; j < m_block; ++j) {
                    m_outblk[oc][j] = m_acc[m_block + j] * scale; // overlap-save: discard the aliased first half
                }
            }

            m_fdl_pos = (cur + 1) % m_max_parts;
        }

        int m_block{0};     // partition (block) size = latency in samples
        int m_fftsize{0};   // FFT size = 2 * m_block
        int m_max_parts{0}; // capacity in partitions

        std::optional<tap::dsp::real_fft> m_fft; // sized in configure()

        std::atomic<int> m_active{-1};          // published IR slot (0/1), or -1 for none. Audio thread reads it.
        int              m_slot_parts[2]{0, 0}; // partition count per slot (kept consistent with m_active)

        std::array<std::array<std::vector<double>, 2>, k_paths> m_ir;  // packed IR spectra [path][slot]
        std::array<std::vector<double>, k_channels>             m_fdl; // packed input FDL [channel]
        std::array<std::vector<double>, k_channels>             m_prev, m_inblk, m_outblk;

        std::vector<double> m_fbuf, m_acc; // packed scratch (analysis frame / accumulator)

        int m_pos{0};     // fill/read index within the current block [0, m_block)
        int m_fdl_pos{0}; // ring index of the newest input spectrum
    };

} // namespace tap::tools
