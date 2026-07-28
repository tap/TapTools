/// @file
/// @brief      Portable Simper state-variable-filter kernel for tap.svf~ — no Max/Min dependency.
/// @details    A trapezoidal-integration (TPT / zero-delay-feedback) state-variable filter after
///             Andy Simper's Cytomic technical papers ("Solving the continuous SVF equations
///             using trapezoidal integration and equivalent currents", the SvfLinearTrapOptimised2
///             form) — the same lineage as the Cytomic-designed filters in Ableton Live. Design
///             points:
///
///             - One 2nd-order TPT section: g = tan(pi*fc/fs) (prewarped — tuning is exact all the
///               way to Nyquist), damping k = 1/Q, states are the two integrator "equivalent
///               currents". Unconditionally stable and well behaved under per-sample cutoff
///               modulation — no Chamberlin-style oversampling trick needed.
///             - Every response is an output mix y = m0*v0 + m1*v1 + m2*v2 over the shared states:
///               lowpass, highpass, bandpass, notch, peak, allpass — plus the parametric-EQ trio
///               (bell, low shelf, high shelf) with a boost/cut gain in dB, straight from Simper's
///               coefficient tables.
///             - mode_morph sweeps the mix continuously around the circle LP -> BP -> HP -> notch
///               -> LP (morph 0 -> 0.25 -> 0.5 -> 0.75 -> 1), like the Morph filter in Live's
///               Auto Filter. The corner positions are bit-identical to the discrete modes.
///             - Orders 2 / 4 / 8 (12 / 24 / 48 dB per octave): a cascade of 1 / 2 / 4 sections at
///               the same cutoff with the Butterworth Q spread, so resonance 0 is maximally flat
///               (-3 dB at fc) at every order. The user resonance (0..1) sharpens only the
///               highest-Q (last) section — one clean resonant peak on top of a flat passband.
///               The EQ modes (bell/shelves) always run a single section: cascading them would
///               square the boost/cut.
///             - Two circuits: circuit_clean is the pure linear filter (cheap, transparent, never
///               oversampled — Live's "Clean"); circuit_driven adds an input drive (dB) and a tanh
///               limiter on each section's band node (OTA-style, in the resonance-damping path),
///               which colors the tone and bounds resonance. At resonance 1.0 the driven circuit
///               is pushed slightly past the oscillation threshold, so it self-oscillates at the
///               cutoff with the saturator limiting the amplitude (give it a ping — an all-zero
///               state is a fixed point). The nonlinear step is the fast one-pass scheme: linear
///               zero-delay prediction of the band node, then the saturated value is committed
///               (same approach as tap.ladder~'s solver_fast).
///             - The driven circuit runs oversampled (1x/2x/4x, default 2x): zero-stuff +
///               4th-order Butterworth anti-imaging up, matching anti-alias before decimation
///               (the tap.ladder~ / tap.verb~ pattern, self-contained here per house rule).
///             - Resonance is normalized 0..1; q_from_resonance()/resonance_from_q() convert to
///               and from the equivalent Q of the resonant section (Butterworth base Q at 0).
///             - The kernel itself is multichannel: coefficients are computed once per sample
///               (tick(), optionally with a signal-rate cutoff override) and shared by every
///               channel's state (process(channel, x)) — usable as a true N-channel engine
///               outside Max. The Max wrapper runs it single-channel and leaves multichannel to
///               mc. wrapping.
///
///             As in the other TapTools kernels: parameters ride per-sample linear ramps (no
///             zipper), and everything is allocation-free after prepare(); setters are safe while
///             audio runs.
///
///             **Numeric profiles.** The filter is `basic_svf_filter<Sample>`, instantiated as
///             `svf_filter` (double — the golden model, what Max's double signal chain and every
///             measured claim in the book/benchmarks/notebooks run on) and `svf_filter32` (float —
///             the embedded profile). This follows the DspTap convention (see its `fft.h` /
///             `yin.h`): the double path never changes for speed, and the two instantiations run
///             the identical algorithm in their own precision, with cross-precision agreement
///             pinned by the test battery. The float profile exists for single-precision targets
///             — an Arm Cortex-M33's FPv5-SP FPU has no double support at all, so the double
///             profile there compiles to ~142 soft-float library calls per sample while the float
///             profile compiles to hardware `vmul.f32`/`vfma.f32` (measured, `tests/svf_test.cpp`
///             documents the geometry). Note the transcendentals (`std::tanh` in the driven
///             circuit, `std::tan` under cutoff modulation) remain libm calls in both profiles —
///             precision alone does not make the driven circuit cheap on such a target.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <vector>

namespace tap::tools {
    namespace svf {

        constexpr double k_pi                = 3.14159265358979323846;
        constexpr double k_freq_min          = 1.0;
        constexpr double k_freq_max          = 20000.0;
        constexpr double k_gain_range_db     = 24.0; // bell/shelf boost-cut range
        constexpr double k_drive_range_db    = 24.0;
        constexpr double k_default_smooth_ms = 20.0;
        constexpr int    k_max_sections      = 4; // order 8

        enum param_index : int {
            p_frequency = 0, // cutoff / center, Hz
            p_resonance,     // 0..1 (0 = maximally flat Butterworth, 1 = self-oscillation threshold)
            p_morph,         // 0..1 around the circle LP -> BP -> HP -> notch -> LP (mode_morph only)
            p_gain,          // bell/shelf boost or cut, dB
            p_drive,         // input gain into the driven circuit, dB
            k_num_params
        };

        enum filter_mode : int {
            mode_lowpass = 0,
            mode_highpass,
            mode_bandpass,
            mode_notch,
            mode_peak,
            mode_allpass,
            mode_morph, // continuous LP -> BP -> HP -> notch -> LP
            mode_bell,  // parametric EQ modes (always a single 2nd-order section)
            mode_lowshelf,
            mode_highshelf,
            k_num_modes
        };

        enum circuit_mode : int {
            circuit_clean = 0, // pure linear Simper SVF, never oversampled
            circuit_driven,    // input drive + tanh band-node limiting, oversampled
            k_num_circuits
        };

        inline bool is_eq_mode(int mode) {
            return mode == mode_bell || mode == mode_lowshelf || mode == mode_highshelf;
        }

        inline int sections_for_order(int order) {
            return (order >= 8) ? 4 : (order >= 4) ? 2 : 1;
        }

        /// Butterworth Q values per section (ascending; the last is the resonant one).
        inline const double* butterworth_q(int order) {
            static constexpr double k_q2[1] = {0.70710678118654752};
            static constexpr double k_q4[2] = {0.54119610014619698, 1.30656296487637653};
            static constexpr double k_q8[4] = {0.50979557910415918, 0.60134488693504529, 0.89997622313641570,
                                               2.56291544774150610};
            switch (sections_for_order(order)) {
            case 4:
                return k_q8;
            case 2:
                return k_q4;
            default:
                return k_q2;
            }
        }

        /// The Q of the resonant (last) section at resonance 0 — the Butterworth base Q for the order.
        inline double base_q(int order) {
            return butterworth_q(order)[sections_for_order(order) - 1];
        }

        // The three helpers below are parameterized over the numeric profile, but `Sample` is
        // deliberately NOT deduced from the value argument (std::type_identity_t) and defaults to
        // double. Two reasons, both load-bearing:
        //   - Existing call sites (`clamp_param(index, v)`) keep compiling unchanged and keep
        //     returning double.
        //   - The Max wrapper passes a Min `atom`, which converts implicitly to double but is not
        //     itself a floating-point type. With a deduced parameter those call sites would fail
        //     to deduce `Sample` — the "clang-tidy is a second compiler" class of breakage.
        // Callers inside the float profile ask for it explicitly: q_from_resonance<float>(r, n).

        /// Equivalent Q of the resonant section for a normalized resonance (0..1). At resonance 0 this is
        /// the Butterworth base Q; it grows as 1/(1-resonance) toward infinity at 1.
        template <typename Sample = double>
        Sample q_from_resonance(std::type_identity_t<Sample> resonance, int order) {
            const Sample d = std::max(Sample(1) - std::clamp(resonance, Sample(0), Sample(1)), Sample(1e-4));
            return static_cast<Sample>(base_q(order)) / d;
        }

        /// Inverse of q_from_resonance. Q at or below the Butterworth base maps to 0.
        template <typename Sample = double>
        Sample resonance_from_q(std::type_identity_t<Sample> q, int order) {
            if (q <= Sample(0)) {
                return Sample(0);
            }
            return std::clamp(Sample(1) - static_cast<Sample>(base_q(order)) / q, Sample(0), Sample(1));
        }

        /// Clamp a value to the legal range of a parameter.
        template <typename Sample = double>
        Sample clamp_param(int index, std::type_identity_t<Sample> value) {
            switch (index) {
            case p_frequency:
                return std::clamp(value, Sample(k_freq_min), Sample(k_freq_max));
            case p_resonance:
                return std::clamp(value, Sample(0), Sample(1));
            case p_morph:
                return std::clamp(value, Sample(0), Sample(1));
            case p_gain:
                return std::clamp(value, Sample(-k_gain_range_db), Sample(k_gain_range_db));
            case p_drive:
                return std::clamp(value, Sample(-k_drive_range_db), Sample(k_drive_range_db));
            default:
                return value;
            }
        }

        template <typename Sample>
        class basic_svf_filter {
            static_assert(std::is_floating_point_v<Sample>,
                          "basic_svf_filter supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            basic_svf_filter() {
                static constexpr Sample k_defaults[k_num_params] = {Sample(1000), Sample(0), Sample(0), Sample(0),
                                                                    Sample(0)};
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = k_defaults[i];
                }
                m_state.resize(1);
            }

            // -- lifecycle -------------------------------------------------------------------------------

            /// Set the sample rate and channel count, (re)configure oversampling, clear state, snap ramps.
            /// Allocates (the per-channel state vector) — call from the main thread, not the perform loop.
            void prepare(Sample sr, int channels = 1) {
                m_sr       = (sr > Sample(0)) ? sr : Sample(48000);
                m_channels = std::max(1, channels);
                m_state.assign(static_cast<size_t>(m_channels), channel_state{});
                configure_resampler();
                snap();
            }

            /// Zero all filter and resampler state in every channel; parameters untouched.
            void clear() {
                for (auto& c : m_state) {
                    c.clear();
                }
            }

            /// Jump all parameter ramps to their targets.
            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = Sample(0);
                    r.remaining = 0;
                }
                m_ramps_active = 0;
                m_shape_dirty  = true;
            }

            int    channels() const { return m_channels; }
            Sample samplerate() const { return m_sr; }

            // -- modes (structural; not ramped) ----------------------------------------------------------

            void set_mode(int mode) {
                m_mode        = std::clamp(mode, 0, k_num_modes - 1);
                m_shape_dirty = true;
            }
            int mode() const { return m_mode; }

            /// Filter order: 2, 4, or 8 (12/24/48 dB per octave). Other values snap down.
            void set_order(int order) {
                m_order       = (order >= 8) ? 8 : (order >= 4) ? 4 : 2;
                m_shape_dirty = true;
            }
            int order() const { return m_order; }

            void set_circuit(int circuit) {
                m_circuit     = std::clamp(circuit, 0, k_num_circuits - 1);
                m_shape_dirty = true; // the effective rate (oversampling) hangs off the circuit
            }
            int circuit() const { return m_circuit; }

            /// Oversampling factor 1, 2, or 4 for the driven circuit (the clean circuit is linear and
            /// always runs 1x). Takes effect immediately; resampler state is cleared.
            void set_oversample(int os) {
                const int v = (os >= 4) ? 4 : (os >= 2) ? 2 : 1;
                if (v != m_os) {
                    m_os = v;
                    configure_resampler();
                    for (auto& c : m_state) {
                        c.up.reset();
                        c.down.reset();
                    }
                    m_shape_dirty = true;
                }
            }
            int oversample() const { return m_os; }

            void   set_smooth_ms(Sample ms) { m_smooth_ms = std::max(Sample(0), ms); }
            Sample smooth_ms() const { return m_smooth_ms; }

            // -- parameter targets (click-free; safe while audio runs) ------------------------------------

            void set_param(int index, Sample value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                ramp_to(index, clamp_param<Sample>(index, value),
                        static_cast<long>(m_smooth_ms * Sample(0.001) * m_sr));
            }

            void set_frequency(Sample hz) { set_param(p_frequency, hz); }
            void set_resonance(Sample r) { set_param(p_resonance, r); }
            void set_morph(Sample m) { set_param(p_morph, m); }
            void set_gain(Sample db) { set_param(p_gain, db); }
            void set_drive(Sample db) { set_param(p_drive, db); }

            Sample param(int index) const {
                return (index >= 0 && index < k_num_params) ? m_ramp[index].target : Sample(0);
            }

            // -- audio -----------------------------------------------------------------------------------
            //
            // Frame protocol for multichannel use: call tick() once per sample frame (it advances the
            // parameter ramps and refreshes the shared coefficients), then process(ch, x) once per
            // channel. The mono conveniences below fold the two together.

            /// Advance ramps and refresh coefficients from the (ramped) frequency parameter.
            /// The update is split in two tiers so per-sample cutoff modulation stays cheap: the "shape"
            /// (damping, output mix, drive/EQ gains — everything the cutoff does not touch) is refreshed
            /// only when a non-frequency parameter or a mode actually changed; the cutoff tier (tan and
            /// the three solve constants per section) is refreshed only when the cutoff value differs
            /// from the cached one.
            void tick() {
                tick_ramps();
                if (m_shape_dirty) {
                    update_shape();
                }
                update_cutoff(m_ramp[p_frequency].current);
            }

            /// Advance ramps and refresh coefficients with a signal-rate cutoff override (Hz). The
            /// frequency parameter/ramp is left untouched.
            void tick(Sample cutoff_hz) {
                tick_ramps();
                if (m_shape_dirty) {
                    update_shape();
                }
                update_cutoff(clamp_param<Sample>(p_frequency, cutoff_hz));
            }

            /// Process one sample of one channel using the coefficients computed by the last tick().
            /// Precondition: 0 <= channel < channels().
            Sample process(int channel, Sample x) {
                channel_state& c = m_state[static_cast<size_t>(channel)];
                if (m_circuit == circuit_clean) {
                    return core<false>(c, x);
                }
                if (m_os == 1) {
                    return core<true>(c, x);
                }
                // zero-stuff + anti-image filter up, core at the high rate, anti-alias + decimate down
                Sample y = Sample(0);
                for (int j = 0; j < m_os; ++j) {
                    const Sample up = c.up.tick(j == 0 ? x * static_cast<Sample>(m_os) : Sample(0));
                    y               = c.down.tick(core<true>(c, up));
                }
                return y;
            }

            /// Mono conveniences.
            Sample process(Sample x) {
                tick();
                return process(0, x);
            }
            Sample process(Sample x, Sample cutoff_hz) {
                tick(cutoff_hz);
                return process(0, x);
            }

            void process(const Sample* in, Sample* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

            /// Multichannel block processing: nch channel pointers in and out.
            void process(const Sample* const* in, Sample* const* out, int nch, size_t n) {
                const int chans = std::min(nch, m_channels);
                for (size_t i = 0; i < n; ++i) {
                    tick();
                    for (int ch = 0; ch < chans; ++ch) {
                        out[ch][i] = process(ch, in[ch][i]);
                    }
                }
            }

          private:
            /// pi in the working precision. Using the double `k_pi` directly would promote the
            /// whole prewarp expression to double in the float profile — the exact soft-float
            /// path this profile exists to avoid.
            static constexpr Sample k_pi_s = static_cast<Sample>(k_pi);

            struct ramp {
                Sample current{0};
                Sample target{0};
                Sample inc{0};
                long   remaining{0};
            };

            // RBJ lowpass biquad (Transposed Direct Form II) — two in series make the 4th-order
            // Butterworth anti-image/anti-alias filters for the oversampling chain (tap.ladder~ pattern).
            struct biquad {
                Sample b0{1}, b1{0}, b2{0}, a1{0}, a2{0};
                Sample z1{0}, z2{0};

                void design_lowpass(Sample fc_norm, Sample q) { // fc_norm = fc / fs
                    const Sample w     = Sample(2) * k_pi_s * fc_norm;
                    const Sample alpha = std::sin(w) / (Sample(2) * q);
                    const Sample cw    = std::cos(w);
                    const Sample a0    = Sample(1) + alpha;
                    b0                 = ((Sample(1) - cw) * Sample(0.5)) / a0;
                    b1                 = (Sample(1) - cw) / a0;
                    b2                 = b0;
                    a1                 = (Sample(-2) * cw) / a0;
                    a2                 = (Sample(1) - alpha) / a0;
                }
                Sample tick(Sample x) {
                    const Sample y = b0 * x + z1;
                    z1             = b1 * x - a1 * y + z2;
                    z2             = b2 * x - a2 * y;
                    return y;
                }
                void reset() { z1 = z2 = Sample(0); }
            };

            struct butterworth4 {
                biquad s1, s2;
                void   design(Sample fc_norm) {
                    s1.design_lowpass(fc_norm, Sample(0.54119610));
                    s2.design_lowpass(fc_norm, Sample(1.30656296));
                }
                Sample tick(Sample x) { return s2.tick(s1.tick(x)); }
                void   reset() {
                    s1.reset();
                    s2.reset();
                }
            };

            // One SVF section's coefficients: the TPT solve constants and the output mix.
            struct section_coeffs {
                Sample g{0}, k{1};
                Sample a1{0}, a2{0}, a3{0};
                Sample m0{0}, m1{0}, m2{1};
            };

            // One section's state: the two integrator equivalent currents.
            struct section_state {
                Sample ic1{0}, ic2{0};
            };

            struct channel_state {
                std::array<section_state, k_max_sections> s{};
                butterworth4                              up, down;
                void                                      clear() {
                    for (auto& sec : s) {
                        sec.ic1 = sec.ic2 = Sample(0);
                    }
                    up.reset();
                    down.reset();
                }
            };

            /// Flush-to-zero threshold for the recursive state. The house idiom (tap.comb~) is
            /// 1e-15, which is a *double*-scale constant: it sits just above the double denormal
            /// range (~2.2e-308 .. 1e-308) with enormous margin. Reusing it verbatim in the float
            /// profile would not do its stated job — float denormals live in
            /// ~1.4e-45 .. 1.18e-38, so 1e-15 flushes a huge swath of perfectly normal floats
            /// (everything below -300 dBFS, still inaudible, so it is not a *bug* — but it is no
            /// longer the denormal guard it claims to be). The float profile therefore uses 1e-30:
            /// above the entire float denormal range, and ~600 dB below full scale.
            /// The double profile keeps 1e-15 exactly, so its output stays bit-identical.
            static constexpr Sample k_denormal_floor = std::is_same_v<Sample, float> ? Sample(1e-30) : Sample(1e-15);

            static Sample anti_denormal(Sample x) { return (std::abs(x) < k_denormal_floor) ? Sample(0) : x; }

            void ramp_to(int index, Sample tgt, long nsamples) {
                ramp&      r   = m_ramp[index];
                const bool was = r.remaining > 0;
                if (nsamples < 1 || tgt == r.current) {
                    r.current   = tgt;
                    r.target    = tgt;
                    r.inc       = Sample(0);
                    r.remaining = 0;
                }
                else {
                    r.target    = tgt;
                    r.inc       = (tgt - r.current) / static_cast<Sample>(nsamples);
                    r.remaining = nsamples;
                }
                m_ramps_active += static_cast<int>(r.remaining > 0) - static_cast<int>(was);
                if (index != p_frequency) {
                    m_shape_dirty = true; // frequency changes are caught by the cutoff cache instead
                }
            }

            void tick_ramps() {
                if (m_ramps_active <= 0) {
                    return;
                }
                for (int i = 0; i < k_num_params; ++i) {
                    ramp& r = m_ramp[static_cast<size_t>(i)];
                    if (r.remaining > 0) {
                        r.current += r.inc;
                        if (--r.remaining == 0) {
                            r.current = r.target;
                            --m_ramps_active;
                        }
                        if (i != p_frequency) {
                            m_shape_dirty = true;
                        }
                    }
                }
            }

            void configure_resampler() {
                if (m_os > 1) {
                    // cut just below the original Nyquist, normalized to the oversampled rate
                    const Sample fc_norm = Sample(0.45) / static_cast<Sample>(m_os);
                    for (auto& c : m_state) {
                        c.up.design(fc_norm);
                        c.down.design(fc_norm);
                    }
                }
            }

            int os_active() const { return (m_circuit == circuit_driven) ? m_os : 1; }

            // When a mode/order change activates sections that were dormant, zero them so they don't
            // replay stale state from a previous configuration.
            void set_active_sections(int n) {
                if (n > m_active) {
                    for (auto& c : m_state) {
                        for (int i = m_active; i < n; ++i) {
                            c.s[static_cast<size_t>(i)].ic1 = c.s[static_cast<size_t>(i)].ic2 = Sample(0);
                        }
                    }
                }
                m_active = n;
            }

            // Shape tier: everything the cutoff does not touch — active section count, per-section
            // damping k, output mix, drive/EQ gains, and the shelf tuning scale. Runs only when a
            // non-frequency parameter ramps or a mode/order/circuit/oversample change lands.
            void update_shape() {
                const Sample res = m_ramp[p_resonance].current;
                const Sample A   = std::pow(Sample(10), m_ramp[p_gain].current / Sample(40)); // Simper's EQ amplitude

                m_in_gain = (m_circuit == circuit_driven) ? std::pow(Sample(10), m_ramp[p_drive].current / Sample(20))
                                                          : Sample(1);

                set_active_sections(is_eq_mode(m_mode) ? 1 : sections_for_order(m_order));

                m_g_scale = (m_mode == mode_lowshelf)    ? Sample(1) / std::sqrt(A)
                            : (m_mode == mode_highshelf) ? std::sqrt(A)
                                                         : Sample(1);

                const double* qt = butterworth_q(m_order);

                for (int i = 0; i < m_active; ++i) {
                    section_coeffs& c = m_coef[static_cast<size_t>(i)];

                    Sample k;
                    if (is_eq_mode(m_mode)) {
                        // EQ modes: a single section whose Q comes from the normalized resonance
                        // (Butterworth base at 0); the bell's k folds in the gain for symmetric boost/cut.
                        const Sample q = q_from_resonance<Sample>(res, 2);
                        k              = (m_mode == mode_bell) ? Sample(1) / (q * A) : Sample(1) / q;
                    }
                    else {
                        k = Sample(1) / static_cast<Sample>(qt[i]);
                        if (i == m_active - 1) {
                            // The user resonance sharpens only the highest-Q section. The clean circuit
                            // floors the damping (finite Q ~ 100x base); the driven circuit is allowed
                            // slightly past the threshold at 1.0 so it truly self-oscillates, with the
                            // tanh limiter bounding the amplitude.
                            if (m_circuit == circuit_driven) {
                                k *= Sample(1) - Sample(1.05) * res;
                            }
                            else {
                                k = std::max(k * (Sample(1) - res), Sample(0.01));
                            }
                        }
                    }

                    c.k = k;

                    switch (m_mode) {
                    case mode_lowpass:
                        c.m0 = Sample(0);
                        c.m1 = Sample(0);
                        c.m2 = Sample(1);
                        break;
                    case mode_highpass:
                        c.m0 = Sample(1);
                        c.m1 = -k;
                        c.m2 = Sample(-1);
                        break;
                    case mode_bandpass:
                        c.m0 = Sample(0);
                        c.m1 = Sample(1);
                        c.m2 = Sample(0);
                        break;
                    case mode_notch:
                        c.m0 = Sample(1);
                        c.m1 = -k;
                        c.m2 = Sample(0);
                        break;
                    case mode_peak:
                        c.m0 = Sample(1);
                        c.m1 = -k;
                        c.m2 = Sample(-2);
                        break;
                    case mode_allpass:
                        c.m0 = Sample(1);
                        c.m1 = Sample(-2) * k;
                        c.m2 = Sample(0);
                        break;
                    case mode_bell:
                        c.m0 = Sample(1);
                        c.m1 = k * (A * A - Sample(1));
                        c.m2 = Sample(0);
                        break;
                    case mode_lowshelf:
                        c.m0 = Sample(1);
                        c.m1 = k * (A - Sample(1));
                        c.m2 = A * A - Sample(1);
                        break;
                    case mode_highshelf:
                        c.m0 = A * A;
                        c.m1 = k * (Sample(1) - A) * A;
                        c.m2 = Sample(1) - A * A;
                        break;
                    case mode_morph:
                    default:
                        morph_mix(c, m_ramp[p_morph].current);
                        break;
                    }
                }

                m_shape_dirty = false;
                m_cur_fc      = Sample(-1); // the solve constants depend on k and m_g_scale — force a refresh
            }

            // Cutoff tier: the prewarped tan and the three solve constants per section. This is the only
            // work that runs per sample under cutoff modulation, and it is skipped entirely when the
            // requested cutoff matches the cached one.
            void update_cutoff(Sample fc) {
                if (fc == m_cur_fc) {
                    return;
                }
                m_cur_fc        = fc;
                const Sample fs = m_sr * static_cast<Sample>(os_active());
                const Sample f  = std::min(std::max(fc, Sample(k_freq_min)), Sample(0.49) * fs);
                const Sample g  = std::tan(k_pi_s * f / fs) * m_g_scale;
                for (int i = 0; i < m_active; ++i) {
                    section_coeffs& c = m_coef[static_cast<size_t>(i)];
                    c.g               = g;
                    c.a1              = Sample(1) / (Sample(1) + g * (g + c.k));
                    c.a2              = g * c.a1;
                    c.a3              = g * c.a2;
                }
            }

            // Continuous output-mix interpolation around the circle LP -> BP -> HP -> notch -> LP.
            // The corners are exactly the discrete modes' mixes (with this section's k).
            static void morph_mix(section_coeffs& c, Sample morph) {
                const Sample k            = c.k;
                const Sample corner[5][3] = {
                    {Sample(0), Sample(0), Sample(1)}, // lowpass
                    {Sample(0), Sample(1), Sample(0)}, // bandpass
                    {Sample(1), -k, Sample(-1)},       // highpass
                    {Sample(1), -k, Sample(0)},        // notch
                    {Sample(0), Sample(0), Sample(1)}, // back to lowpass
                };
                Sample       p = std::clamp(morph, Sample(0), Sample(1)) * Sample(4);
                int          i = std::min(static_cast<int>(p), 3);
                const Sample t = p - static_cast<Sample>(i);
                c.m0           = corner[i][0] + t * (corner[i + 1][0] - corner[i][0]);
                c.m1           = corner[i][1] + t * (corner[i + 1][1] - corner[i][1]);
                c.m2           = corner[i][2] + t * (corner[i + 1][2] - corner[i][2]);
            }

            // The cascade core for one channel at the (possibly oversampled) processing rate.
            // Per section (Simper's optimised form): v3 = v0 - ic2; v1 = a1*ic1 + a2*v3; v2 = ic2 + g*v1;
            // then the trapezoidal state advance. The driven circuit commits tanh(v1) — a one-pass
            // corrective solve of the band-node limiter seeded by the linear zero-delay prediction.
            // Templated on the circuit so the per-section branch is resolved at compile time.
            template <bool Driven>
            Sample core(channel_state& ch, Sample x) {
                Sample v0 = m_in_gain * x;
                for (int i = 0; i < m_active; ++i) {
                    const section_coeffs& q  = m_coef[static_cast<size_t>(i)];
                    section_state&        s  = ch.s[static_cast<size_t>(i)];
                    const Sample          v3 = v0 - s.ic2;
                    Sample                v1 = q.a1 * s.ic1 + q.a2 * v3;
                    if (Driven) {
                        v1 = std::tanh(v1);
                    }
                    const Sample v2 = s.ic2 + q.g * v1;
                    s.ic1           = anti_denormal(Sample(2) * v1 - s.ic1);
                    s.ic2           = anti_denormal(Sample(2) * v2 - s.ic2);
                    v0              = q.m0 * v0 + q.m1 * v1 + q.m2 * v2;
                }
                return v0;
            }

            // configuration
            Sample m_sr{48000};
            Sample m_smooth_ms{static_cast<Sample>(k_default_smooth_ms)};
            int    m_channels{1};
            int    m_mode{mode_lowpass};
            int    m_order{2};
            int    m_circuit{circuit_clean};
            int    m_os{2};

            // parameters
            std::array<ramp, k_num_params> m_ramp;
            int                            m_ramps_active{0};
            bool                           m_shape_dirty{true};

            // derived
            std::array<section_coeffs, k_max_sections> m_coef;
            int                                        m_active{1};
            Sample                                     m_in_gain{1};
            Sample                                     m_g_scale{1}; // shelf tuning scale (sqrt(A) up or down)
            Sample                                     m_cur_fc{-1}; // cutoff the solve constants were
                                                                     // computed for (-1 = stale)

            // per-channel state
            std::vector<channel_state> m_state;
        };

        /// The double profile — the golden model. This is what Max's double signal chain, the
        /// C ABI, the benchmarks and the verification notebooks all use, and its output is
        /// bit-identical to the pre-template kernel.
        using svf_filter = basic_svf_filter<double>;

        /// The float profile — for single-precision targets (Cortex-M33 and friends), where the
        /// double profile would run entirely in soft-float. Same algorithm, its own precision.
        using svf_filter32 = basic_svf_filter<float>;

    } // namespace svf
} // namespace tap::tools
