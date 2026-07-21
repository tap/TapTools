/// @file
/// @brief      Short-time Fourier transform scaffold for the TapTools spectral kernels.
/// @details    The overlap-add machinery shared verbatim by tap.nr~ and tap.spectra~: a Hann
///             analysis/synthesis window, fixed 4× overlap, circular input and output-accumulator
///             buffers, COLA normalisation for perfect reconstruction, and the per-sample pump that
///             fires an FFT frame every hop. The only thing that differs between objects is what is
///             done to the spectrum between the forward and inverse transforms, so `process` takes
///             that step as a callable: `op(re, im, N)` operates in place on the half-spectrum — bins
///             0..N/2, the N/2+1 unique bins of the real transform (im[0] and im[N/2] are zero, DC
///             and Nyquist being real). With an identity op, the output reconstructs the input delayed
///             by one FFT frame (the latency).
///
///             The transform is the shared DspTap real FFT (tap::dsp::real_fft): half the work of the
///             old complex radix-2 for a real signal, and on Apple / Arm targets it dispatches to the
///             vDSP / CMSIS-Helium backends. Plain C++20, standard library only.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#pragma once

#include <cmath>
#include <optional>
#include <utility>
#include <vector>

#include "tap/dsp/fft.h"

namespace tap::tools {

    class stft {
      public:
        static constexpr int    k_default_overlap = 4;
        static constexpr double k_pi              = 3.14159265358979323846;

        // Allocate the window and buffers for the given FFT size (a power of two) and overlap factor.
        // Resets all running state. Call before processing and whenever the size changes.
        void configure(int fftsize, int overlap = k_default_overlap) {
            m_fftsize = fftsize;
            m_overlap = overlap;
            m_hop     = m_fftsize / m_overlap;

            m_window.assign(m_fftsize, 0.0);
            for (int k = 0; k < m_fftsize; ++k) {
                m_window[k] = 0.5 - 0.5 * std::cos(2.0 * k_pi * k / m_fftsize);
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

            m_fft.emplace(static_cast<size_t>(m_fftsize));
            m_spec.assign(m_fftsize, 0.0);
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
        // the half-spectrum (bins 0..N/2) in place; input and output must not alias.
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
            const int N    = m_fftsize;
            const int half = N / 2;

            // Assemble the most recent N samples (oldest first), apply the analysis window, and
            // forward-transform in place into DspTap's packed spectrum.
            for (int k = 0; k < N; ++k) {
                m_spec[k] = m_inbuf[(m_pos + k) % N] * m_window[k];
            }
            m_fft->forward_inplace(m_spec.data());

            // Unpack the packed spectrum into the N/2+1 unique bins for the operation.
            m_re[0]    = m_spec[0]; // DC (real)
            m_im[0]    = 0.0;
            m_re[half] = m_spec[1]; // Nyquist (real)
            m_im[half] = 0.0;
            for (int k = 1; k < half; ++k) {
                m_re[k] = m_spec[2 * k];
                m_im[k] = m_spec[2 * k + 1];
            }

            op(m_re, m_im, N); // in-place spectral operation on the half-spectrum (bins 0..N/2)

            // Repack for the inverse transform.
            m_spec[0] = m_re[0];
            m_spec[1] = m_re[half];
            for (int k = 1; k < half; ++k) {
                m_spec[2 * k]     = m_re[k];
                m_spec[2 * k + 1] = m_im[k];
            }
            m_fft->inverse_inplace(m_spec.data()); // unscaled — the 2/N is folded into the overlap-add below

            // Synthesis window + overlap-add into the output accumulator. The real inverse is
            // unnormalised, so the 2/N that completes the round trip rides in here with the COLA norm.
            const double scale = 2.0 / static_cast<double>(N);
            for (int k = 0; k < N; ++k) {
                m_outbuf[(m_pos + k) % N] += m_spec[k] * scale * m_window[k] * m_norm;
            }
        }

        int m_fftsize{1024};
        int m_overlap{k_default_overlap};
        int m_hop{256};

        double m_norm{1.0};
        int    m_pos{0};
        int    m_hopcount{0};

        std::optional<tap::dsp::real_fft> m_fft;    // sized in configure()
        std::vector<double>               m_window; // Hann (analysis and synthesis)
        std::vector<double>               m_inbuf;  // circular input buffer (length N)
        std::vector<double>               m_outbuf; // circular overlap-add accumulator (length N)
        std::vector<double>               m_spec;   // packed FFT scratch (time in / spectrum / time out)
        std::vector<double>               m_re;     // unpacked half-spectrum real (bins 0..N/2)
        std::vector<double>               m_im;     // unpacked half-spectrum imag (bins 0..N/2)
    };

} // namespace tap::tools
