/// @file
/// @brief      Flush-to-zero for this tree's own hosts: the tests, the C ABI, the benchmarks and the
///             render tools.
/// @details    The oversampling chain's Butterworth stage (taptools/butterworth.h, over DspTap's
///             basic_butterworth_lowpass) is a recursive kernel, and DspTap leaves subnormals to the
///             host (tap/dsp/subnormals.h): the host sets flush-to-zero on the thread that runs it, the
///             kernel only reads the bit, as a debug-build precondition, and no kernel ever writes it.
///             TapTools-Max is the host for the shipping objects, and setting it on its audio threads
///             is its job, not this tree's. The programs in this tree are hosts too, and set it with
///             flush_to_zero_scope: the bits subnormals.h names for the target (MXCSR FTZ | DAZ on
///             x86-64, FPCR.FZ on AArch64, FPSCR.FZ on 32-bit Arm; nothing where the target has
///             none), with the caller's register restored on exit, so a Python interpreter calling
///             the C ABI keeps its own setting.
///             The register write mirrors DspTap's test support (tests/support/iir_support.h,
///             write_floating_point_control), written out here because that file is DspTap's test
///             code, not its API.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place and the TapTools contributors

#pragma once

#include <cstdint>

#include "tap/dsp/subnormals.h"

namespace tap::tools::support {

    /// Writes the calling thread's floating-point control register, where the target has one
    /// (tap::dsp::floating_point_control() reads it).
    inline void write_floating_point_control(std::uint64_t v) noexcept {
#if defined(TAP_DSP_FP_CONTROL_X86_64)
        _mm_setcsr(static_cast<unsigned>(v));
#elif defined(TAP_DSP_FP_CONTROL_AARCH64)
#if defined(_MSC_VER) && !defined(__clang__)
        _WriteStatusReg(ARM64_FPCR, static_cast<__int64>(v));
#else
        __asm__ volatile("msr fpcr, %0" : : "r"(v));
#endif
#elif defined(TAP_DSP_FP_CONTROL_AARCH32)
        __builtin_arm_set_fpscr(static_cast<unsigned>(v));
#else
        static_cast<void>(v);
#endif
    }

    /// Sets the host's flush-to-zero bits on the calling thread for the scope's lifetime, and
    /// restores the register as it was on exit.
    class flush_to_zero_scope {
      public:
        flush_to_zero_scope() noexcept
            : m_saved(tap::dsp::floating_point_control()) {
            write_floating_point_control(m_saved | tap::dsp::k_flush_to_zero_bits);
        }
        ~flush_to_zero_scope() { write_floating_point_control(m_saved); }

        flush_to_zero_scope(const flush_to_zero_scope&)            = delete;
        flush_to_zero_scope& operator=(const flush_to_zero_scope&) = delete;
        flush_to_zero_scope(flush_to_zero_scope&&)                 = delete;
        flush_to_zero_scope& operator=(flush_to_zero_scope&&)      = delete;

      private:
        std::uint64_t m_saved;
    };

} // namespace tap::tools::support
