/// @file
/// @brief      The oversampling chain's Butterworth low-pass: DspTap's stage in the shape the kernels call.
/// @details    ladder.h, diode_ladder.h, overdrive.h and svf.h (order 4) and fuzz.h and ondes.h (order
///             8) run their nonlinear core oversampled between Butterworth low-passes: anti-imaging
///             after the zero-stuff on the way up, anti-aliasing before the decimation on the way down.
///             Each used to carry a private copy: two or four RBJ cookbook biquads (Bristow-Johnson,
///             "Audio EQ Cookbook") at the Butterworth Q set rounded to eight decimals, in transposed
///             direct form II. The filter is now DspTap's, tap::dsp::basic_butterworth_lowpass<double,
///             N> (tap/dsp/butterworth.h, DspTap 0.7.0, whose header is the contract): designed by the
///             bilinear transform with pre-warping (Oppenheim & Schafer, "Discrete-Time Signal
///             Processing", 3rd ed., sec. 7.2), its sections in increasing pole radius, the order the
///             copies ran theirs in, through DspTap's floating biquad cascade, the same transposed
///             direct form II arithmetic. In exact arithmetic it is the cookbook cascade at the exact
///             Butterworth Qs; against the copies' eight-decimal Qs a coefficient differs by up to
///             2.5e-9 (DspTap's `SectionsAreTheCookbookBiquadsAtTheButterworthQs`), the filter's
///             output by at most 3.7e-9 on noise at 0.9 of full scale (butterworth_test.cpp pins it
///             under 1e-8 against a reference copy of the cascade), and the kernels' outputs by at
///             most 2.5e-8, fuzz.h driven hard (the commit that made the swap states how that was
///             measured).
///
///             butterworth_lowpass<N> keeps the copies' call shape, so the kernels' call sites read as
///             they did:
///             - Construction: DspTap's stage has no default constructor (a design needs a cutoff);
///               this one designs it at k_nominal_cutoff, 0.225 cycles per sample: 0.45 / 2, the
///               corner every 2x chain here designs, and fuzz.h's corner at each of its 2x stages. A
///               kernel designs its own corner before it runs the filter, so the nominal one only
///               makes a default-constructed filter a working low-pass (the copies' default was a
///               pass-through) that a member array or a per-channel state vector can hold.
///               Construction runs one design (below): it allocates nothing, but it is not free, so
///               the kernels construct these with themselves or in prepare(), never per sample.
///             - design(fc): DspTap's set_cutoff(), in place: noexcept, allocation-free, bit for bit
///               what construction at fc holds, the state left alone (the kernels reset() after it,
///               as they did). Precondition, a debug-build assertion (TAP_EXPECTS):
///               tap::dsp::butterworth_supports_cutoff<double>(fc), 1e-5 <= fc <= 0.5 - 1e-5 cycles
///               per sample; the kernels ask for 0.45 / os, from 0.05625 at 8x to 0.225 at 2x. One
///               design costs 1.1 to 2.8 us at order 4 and 5.5 to 14 us at order 8 (DspTap 0.7.0's
///               measurement, x86-64, at the cutoffs 0.45 / 2 ... 0.45 / 16; nearly all of it the
///               flush window): fine for a setter such as set_oversample(), never per sample. At the
///               cutoff the filter already has it returns at once, since the design would reproduce
///               the same bits.
///             - tick(x): one sample, a one-frame call to the stage's process(). The kernels call it
///               per oversampled sample, as they called the copies; handing the stage each input
///               sample's 2 to 8 oversampled frames as one block (its block kernel) measured no
///               faster on x86-64 (GCC 13, bench/oversampling_bench.cpp; the swap's commit has the
///               numbers), so the call sites stayed as they were.
///             - reset(): the state to zero; the design stays.
///
///             The host's flush-to-zero. The stage's process() states, as a debug-build precondition
///             (tap/dsp/subnormals.h, TAP_DSP_EXPECTS_SUBNORMALS_FLUSHED), that the calling thread
///             flushes subnormals: MXCSR FTZ and DAZ on x86-64, FPCR.FZ on AArch64. Setting it is the
///             host's job, never a kernel's: TapTools-Max's on the audio threads that run the shipping
///             objects, and this tree's own hosts' (the tests, the C ABI, the benchmarks and the
///             render tools set it through tools/support/flush_to_zero.h). A debug build that runs
///             these kernels without it stops at the assertion. A release build without it behaves
///             as the copies did, which had no such guard either: once the input stops, the exact
///             recursion decays into subnormals, slower arithmetic on x86 while it lasts and a
///             residue of a few multiples of the smallest subnormal at most, a matter of CPU, not of
///             correctness. With it, the state settles to zero or, for some designs, a limit cycle
///             within 256 times the smallest normal double (about 6e-306, DspTap's pinned bound over
///             these cutoffs, its honest limit).
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place and the TapTools contributors

#pragma once

#include <cstddef>

#include "dsptap_version.h"
#include "tap/dsp/butterworth.h"

namespace tap::tools {

    /// An order-N Butterworth low-pass in double, one channel: DspTap's stage behind the oversampling
    /// chain's design / tick / reset (the file comment). N even, 2 ... 16.
    template <std::size_t N>
    class butterworth_lowpass {
      public:
        using stage_type = tap::dsp::basic_butterworth_lowpass<double, N, 1>;

        static constexpr std::size_t k_order = N;

        /// The cutoff a default-constructed filter is designed at, in cycles per sample: 0.45 / 2.
        static constexpr double k_nominal_cutoff = 0.225;

        butterworth_lowpass()
            : m_stage(k_nominal_cutoff) {}

        /// Redesign at fc_norm cycles per sample, the state left alone (the file comment).
        /// @pre tap::dsp::butterworth_supports_cutoff<double>(fc_norm)
        void design(double fc_norm) noexcept {
            if (fc_norm != m_stage.cutoff()) {
                m_stage.set_cutoff(fc_norm);
            }
        }

        /// One sample in, one out.
        double tick(double x) noexcept {
            double y = 0.0;
            m_stage.process(&x, 1, &y);
            return y;
        }

        /// Zero the state; the design stays.
        void reset() noexcept { m_stage.reset(); }

        /// The cutoff the filter is designed at, in cycles per sample.
        [[nodiscard]] double cutoff() const noexcept { return m_stage.cutoff(); }

      private:
        stage_type m_stage;
    };

    /// The order-4 chain of ladder.h, diode_ladder.h, overdrive.h and svf.h.
    using butterworth4 = butterworth_lowpass<4>;

    /// The order-8 chain of fuzz.h and ondes.h.
    using butterworth8 = butterworth_lowpass<8>;

} // namespace tap::tools
