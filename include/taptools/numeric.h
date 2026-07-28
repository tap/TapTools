/// @file
/// @brief      The numeric-profile substrate shared by every TapTools kernel.
/// @details    TapTools kernels are written once as `basic_<name><Sample>` and instantiated for
///             the two Tap numeric profiles, following the DspTap convention (`fft.h`, `yin.h`):
///
///                 using <name>   = basic_<name><double>;  // the golden model
///                 using <name>32 = basic_<name><float>;   // the embedded profile
///
///             **double is the golden model.** It is what Max's double signal chain runs, what
///             the C ABI and the verification notebooks drive, and what every measured claim in
///             the book and the benchmarks was recorded against. The double path never changes
///             for speed, and a kernel's migration to a template is expected to leave its double
///             output *bit-identical* — the kernel tests pin that.
///
///             **float is the embedded profile.** It exists for single-precision targets: an Arm
///             Cortex-M33's FPv5-SP FPU has no double support at all, so a double kernel there
///             compiles entirely into the soft-float runtime (measured on the SVF: 142
///             `__aeabi_d*` calls per sample, versus zero and hardware `vmul.f32`/`vfma.f32` for
///             the float profile). It is also the natural precision for SIMD on desktop.
///
///             This header carries the handful of decisions that would otherwise be re-derived,
///             and re-argued, in every kernel. Everything here is constexpr or a single
///             comparison — no runtime cost over writing the constant inline.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <cmath>
#include <type_traits>

namespace tap::tools {

    /// The two profiles a kernel may be instantiated for. Kernels static_assert on this so an
    /// accidental `basic_foo<int>` fails with a sentence instead of a template backtrace.
    template <typename Sample>
    inline constexpr bool is_sample_profile = std::is_same_v<Sample, float> || std::is_same_v<Sample, double>;

    /// pi in the working precision.
    ///
    /// Load-bearing: writing a double `k_pi` into a float expression promotes the whole
    /// expression to double, which on a single-precision target is exactly the soft-float path
    /// the float profile exists to avoid. Kernels use this, never a bare double literal.
    template <typename Sample>
    inline constexpr Sample k_pi_for = static_cast<Sample>(3.14159265358979323846);

    /// Flush-to-zero threshold for recursive filter state.
    ///
    /// The house idiom (from tap.comb~) is 1e-15. That is a *double*-scale constant: it sits far
    /// above the double denormal range (~1e-308), so it reliably snaps a decaying recursion to
    /// true zero before the CPU starts taking denormal stalls.
    ///
    /// Reused verbatim in the float profile it would not do that job. Float denormals live in
    /// ~1.4e-45 .. 1.18e-38, so 1e-15 flushes a wide swath of perfectly normal floats —
    /// everything below about -300 dBFS. That is still far under audibility, so it is not a bug,
    /// but it is no longer a denormal guard; it is an arbitrary noise gate that happens to also
    /// catch denormals. The float profile therefore uses 1e-30: above the entire float denormal
    /// range, and ~600 dB below full scale.
    ///
    /// The double value is unchanged at 1e-15 precisely so every kernel's double output stays
    /// bit-identical to its pre-template self.
    ///
    /// Note for embedded targets: an FPv5 FPU has a flush-to-zero mode, and when it is enabled
    /// this guard is redundant (though harmless). It is kept unconditional because the kernels
    /// are portable code and cannot assume the FPU's control register state.
    template <typename Sample>
    inline constexpr Sample k_denormal_floor = std::is_same_v<Sample, float> ? Sample(1e-30) : Sample(1e-15);

    /// The house anti-denormal idiom, in the working precision. Snaps a decaying recursive state
    /// to exactly zero once it stops mattering.
    template <typename Sample>
    inline Sample anti_denormal(Sample x) {
        return (std::abs(x) < k_denormal_floor<Sample>) ? Sample(0) : x;
    }

} // namespace tap::tools
