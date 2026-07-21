/// @file
/// @brief      Short-time Fourier transform scaffold for the TapTools spectral kernels.
/// @details    The overlap-add machinery shared verbatim by tap.nr~ and tap.spectra~: a Hann
///             analysis/synthesis window, fixed 4× overlap, circular input and output-accumulator
///             buffers, COLA normalisation for perfect reconstruction, and the per-sample pump that
///             fires an FFT frame every hop. The only thing that differs between objects is what is
///             done to the spectrum between the forward and inverse transforms, so `process` takes
///             that step as a callable: `op(re, im, N)` operates in place on the full N-point
///             complex spectrum. With an identity op, the output reconstructs the input delayed by
///             one FFT frame (the latency). Plain C++17, standard library only.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#pragma once

#include <cmath>
#include <utility>
#include <vector>

#include "fft.h"

namespace tap::tools {

    class stft {
      public:
        static constexpr int k_default_overlap = 4;

        // Allocate the window and buffers for the given FFT size (a power of two) and overlap factor.
        // Resets all running state. Call before processing and whenever the size changes.
        void configure(int fftsize, int overlap = k_default_overlap) {
            m_fftsize = fftsize;
            m_overlap = overlap;
            m_hop     = m_fftsize / m_overlap;

            m_window.assign(m_fftsize, 0.0);
            for (int k = 0; k < m_fftsize; ++k) {
                m_window[k] = 0.5 - 0.5 * std::cos(2.0 * fft::k_pi * k / m_fftsize);
            }

            // COLA normalisation: overlap-add `overlap` copies of window^2 and read the steady-state
            // value. Dividing by it makes analysis+synthesis windowing reconstruct unity gain.
            std::vector<double> cola(m_fftsize, 0.0);
            for (int o = 0; o < m_overlap; ++o) {
                for (int k = 0; k < m_fftsize; ++k) {
                    cola[(o * m_hop + k) % m_fftsize] += m_window[k] * m_window[k];
                }
            }
            const double c = cola[m_fftsize / 2];
            m_norm         = (c > 0.0) ? (1.0 / c) : 1.0;

            m_re.assign(m_fftsize, 0.0);
            m_im.assign(m_fftsize, 0.0);
            reset();
        }

        // Flush the running buffers without touching the window/size (safe from a message handler).
        void reset() {
            m_inbuf.assign(m_fftsize, 0.0);
            m_outbuf.assign(m_fftsize, 0.0);
            m_pos      = 0;
            m_hopcount = 0;
        }

        int fftsize() const { return m_fftsize; }
        int overlap() const { return m_overlap; }
        int hop() const { return m_hop; }
        int latency() const { return m_fftsize; } // reconstruction delay with an identity op

        // Pump n samples through the STFT. `op` is invoked once per hop as op(re, im, N) and may modify
        // the spectrum in place; input and output must not alias.
        template <class SpectralOp>
        void process(const double* in, double* out, long n, SpectralOp&& op) {
            for (long i = 0; i < n; ++i) {
                m_inbuf[m_pos]  = in[i];
                out[i]          = m_outbuf[m_pos];
                m_outbuf[m_pos] = 0.0;

                m_pos = (m_pos + 1) % m_fftsize;

                if (++m_hopcount >= m_hop) {
                    m_hopcount = 0;
                    process_frame(std::forward<SpectralOp>(op));
                }
            }
        }

      private:
        template <class SpectralOp>
        void process_frame(SpectralOp&& op) {
            const int N = m_fftsize;

            // Assemble the most recent N samples (oldest first) and apply the analysis window.
            for (int k = 0; k < N; ++k) {
                m_re[k] = m_inbuf[(m_pos + k) % N] * m_window[k];
                m_im[k] = 0.0;
            }

            fft::transform(m_re, m_im, false);
            op(m_re, m_im, N); // in-place spectral operation on the full N-point spectrum
            fft::transform(m_re, m_im, true);

            // Synthesis window + overlap-add into the output accumulator.
            for (int k = 0; k < N; ++k) {
                m_outbuf[(m_pos + k) % N] += m_re[k] * m_window[k] * m_norm;
            }
        }

        int m_fftsize{1024};
        int m_overlap{k_default_overlap};
        int m_hop{256};

        double m_norm{1.0};
        int    m_pos{0};
        int    m_hopcount{0};

        std::vector<double> m_window; // Hann (analysis and synthesis)
        std::vector<double> m_inbuf;  // circular input buffer (length N)
        std::vector<double> m_outbuf; // circular overlap-add accumulator (length N)
        std::vector<double> m_re;     // scratch FFT real
        std::vector<double> m_im;     // scratch FFT imag
    };

} // namespace tap::tools
