/// @file
/// @brief      Portable virtual-analog diode-ladder filter kernel for tap.diode~ — no Max/Min dependency.
/// @details    A zero-delay-feedback (topology-preserving-transform) 4-stage *diode* ladder
///             lowpass — the TB-303 filter topology, and the nonlinear sibling of the buffered
///             transistor ladder in ladder.h. The defining difference: a diode ladder's stages
///             are NOT buffered from each other — every diode pair both feeds the next capacitor
///             and loads the previous one — so the four poles spread apart instead of piling up
///             at one frequency. Provenance (all verified against Tim Stinchcombe's published
///             TB-303 circuit analysis, timstinchcombe.co.uk):
///
///             - The linearized state equations are the coupled diffusion chain
///                   v1' = w*(S(u - v1) - S(v1 - v2))
///                   v2' = w*(S(v1 - v2) - S(v2 - v3))
///                   v3' = w*(S(v2 - v3) - S(v3 - v4))
///                   v4' = 2w*S(v3 - v4)            [top capacitor halved, per the 303 schematic]
///               with S = identity. Its transfer function, frequency-normalized, is exactly
///               Stinchcombe's measured/derived TB-303 response
///                   H(s) = 1 / (s^4 + 6.727 s^3 + 14.142 s^2 + 9.514 s + 1)
///               (the coefficients are 4*2^(3/4), 10*sqrt(2), 8*2^(1/4) — the equal-component
///               chain with the top cap halved, which is why Stinchcombe finds that cap shifts
///               cutoff by 2^0.25). Poles are real and spread ~25:1 (-0.13, -1.04, -2.33,
///               -3.24 normalized): asymptotically 24 dB/oct, but only ~14 dB in the first
///               octave above cutoff — the honest version of the panel's "18 dB" claim.
///             - Feedback: u = drive*x - k*hp(v4). Routh–Hurwitz on the closed loop puts the
///               oscillation threshold at exactly k = 17 (cf. the transistor ladder's k = 4 —
///               Open303 uses the same 1/17), oscillating at exactly sqrt(2)x the stage rate.
///               resonance maps k = 17*resonance, so 1.0 = the ideal chain's threshold — but
///               with the 150 Hz feedback high-pass in the loop, its phase lead pushes the
///               would-be oscillation frequency up to where the ladder attenuates more, and the
///               closed loop needs k ~ 17.5 even at 8 kHz, ~ 19 at 2 kHz, ~ 25 at 500 Hz. The
///               emergent result: at stock settings this filter — like the hardware TB-303,
///               famously — never quite self-oscillates. The range therefore extends to
///               resonance 1.5 (k = 25.5) as the documented Devil-Fish-style bend: past ~1.1
///               it sings at high cutoffs, rides slightly sharp of fc near the corner (the same
///               phase lead), and the tanh diodes bound the amplitude. With `fbhp 0` the ideal
///               analysis is exact: threshold at 1.0, oscillation at fc — drifting flat as
///               resonance pushes past threshold (~ -0.7x the excess: the growing amplitude
///               saturates the edges unevenly, lowering the effective stage gains), an
///               amplitude effect, not a tuning error — it is identical at every fc. (Ping
///               it — all-zero state is a fixed point, as with ladder.h.)
///             - Tuning convention: `frequency` is the resonance-peak / self-oscillation
///               frequency, held exact at any sample rate by prewarped TPT gains
///               g = tan(pi*fc/fs)/sqrt(2) per stage (2g on the top stage). At resonance 0 the
///               -3 dB point sits ~3.2 octaves BELOW fc — that spread is the real filter (see
///               above), not a bug.
///             - The feedback path is high-passed (one-pole, default 150 Hz — Open303's
///               hardware-calibrated value): resonance audibly thins as cutoff approaches the
///               corner, which is why a 303 keeps its bass at high resonance. It also makes the
///               closed-loop DC gain exactly 1 regardless of resonance — the hardware's own
///               passband compensation, so there is no `comp` parameter here. `fbhp` is exposed
///             	 (a documented bend): 0 = DC-coupled feedback, transistor-ladder-style droop.
///             - Nonlinearity: tanh on every diode-pair edge (each S() above) — in the circuit
///               the coupling elements themselves saturate, not buffer amps between stages. No
///               `asym` parameter: the pairs are complementary, so the mismatch story of
///               ladder.h doesn't apply. Two solvers: `solver_fast` (default) solves the exact
///               linear ZDF system through per-edge secant gains carried from the previous
///               sample, then one corrective re-solve at the new operating point;
///               `solver_exact` repeats that secant re-linearization to convergence — audibly
///               identical until drive and resonance are both pushed hard.
///             - Oversampling 1x/2x/4x (default 2x): zero-stuff + 4th-order Butterworth
///               anti-imaging up, matching anti-alias cascade before decimation (the
///               tap.verb~/ladder.h pattern, self-contained here per house rule).
///
///             As in the other TapTools kernels: every parameter rides a per-sample linear ramp
///             (no zipper), a 16-slot preset-morph engine lives in the kernel, and the cutoff
///             can be overridden per sample for signal-rate modulation. Allocation-free after
///             prepare(); setters are safe while audio runs.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace tap::tools {
    namespace diode {

        constexpr int    k_presets           = 16;
        constexpr double k_freq_min          = 10.0;
        constexpr double k_freq_max          = 20000.0;
        constexpr double k_res_max           = 1.5; // k = 25.5: past self-oscillation even mid-keyboard (see header)
        constexpr double k_drive_range_db    = 24.0;
        constexpr double k_fbhp_default_hz   = 150.0; // Open303's hardware-calibrated feedback HPF
        constexpr double k_fbhp_max_hz       = 2000.0;
        constexpr double k_default_smooth_ms = 20.0;
        constexpr double k_fb_at_osc         = 17.0; // Routh-Hurwitz threshold of the ideal chain
        constexpr double k_pi                = 3.14159265358979323846;

        enum param_index : int {
            p_gain = 0,  // output gain, dB
            p_frequency, // resonance-peak / self-oscillation frequency, Hz
            p_resonance, // 0..1.5 (1.0 = ideal-chain threshold; >1.1 sings — see header on the fbhp)
            p_drive,     // input gain into the diode nonlinearity, dB
            p_fbhp,      // feedback high-pass corner, Hz (150 = hardware; 0 = DC-coupled bend)
            k_num_params
        };

        enum solver_mode : int {
            solver_fast = 0, // carried secant gains + one corrective re-solve (default)
            solver_exact,    // secant re-linearization iterated to convergence
            k_num_solvers
        };

        /// One full parameter snapshot — a preset slot, and the unit the morph engine interpolates.
        struct params {
            std::array<double, k_num_params> v{};

            static params defaults() {
                params p;
                p.v[p_gain]      = 0.0;
                p.v[p_frequency] = 1000.0;
                p.v[p_resonance] = 0.0;
                p.v[p_drive]     = 0.0;
                p.v[p_fbhp]      = k_fbhp_default_hz;
                return p;
            }
        };

        /// Clamp a value to the legal range of a parameter. Gain (dB) is unclamped.
        inline double clamp_param(int index, double value) {
            switch (index) {
            case p_gain:
                return value;
            case p_frequency:
                return std::clamp(value, k_freq_min, k_freq_max);
            case p_resonance:
                return std::clamp(value, 0.0, k_res_max);
            case p_drive:
                return std::clamp(value, -k_drive_range_db, k_drive_range_db);
            case p_fbhp:
                return std::clamp(value, 0.0, k_fbhp_max_hz);
            default:
                return value;
            }
        }

        class diode_filter {
          public:
            diode_filter() {
                const params d = params::defaults();
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = d.v[i];
                }
                m_presets.fill(d);
            }

            // -- lifecycle -------------------------------------------------------------------------------

            /// Set the sample rate, (re)configure the oversampling chain, clear state, snap all ramps.
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                configure_resampler();
                clear();
                snap();
                m_prepared = true;
            }

            /// Zero all filter and resampler state; parameters untouched.
            void clear() {
                m_s1 = m_s2 = m_s3 = m_s4 = m_sh = 0.0;
                m_gamma.fill(1.0);
                m_up.reset();
                m_down.reset();
            }

            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = 0.0;
                    r.remaining = 0;
                }
                m_ramps_active  = 0;
                m_derived_dirty = true;
            }

            // -- modes (not part of the morphable parameter set) --------------------------------------------

            /// Oversampling factor 1, 2, or 4. Takes effect immediately (state is cleared).
            void set_oversample(int os) {
                const int v = (os >= 4) ? 4 : (os >= 2) ? 2 : 1;
                if (v != m_os) {
                    m_os = v;
                    configure_resampler();
                    clear();
                    m_derived_dirty = true;
                }
            }
            int oversample() const { return m_os; }

            void   set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }
            double smooth_ms() const { return m_smooth_ms; }

            // -- parameter targets (click-free; safe while audio runs) --------------------------------------

            void set_param(int index, double value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                ramp_to(index, clamp_param(index, value), static_cast<long>(m_smooth_ms * 0.001 * m_sr));
            }

            void set_gain(double db) { set_param(p_gain, db); }
            void set_frequency(double hz) { set_param(p_frequency, hz); }
            void set_resonance(double r) { set_param(p_resonance, r); }
            void set_drive(double db) { set_param(p_drive, db); }
            void set_fbhp(double hz) { set_param(p_fbhp, hz); }

            void set_solver(int s) { m_solver = std::clamp(s, 0, k_num_solvers - 1); }
            int  solver() const { return m_solver; }

            // -- presets / morph -----------------------------------------------------------------------------

            bool store_preset(int slot) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[slot] = snap_targets();
                return true;
            }

            bool recall_preset(int slot, double seconds) {
                if (!valid_slot(slot)) {
                    return false;
                }
                const long n = static_cast<long>(std::max(0.0, seconds) * m_sr);
                for (int i = 0; i < k_num_params; ++i) {
                    ramp_to(i, clamp_param(i, m_presets[slot].v[i]), n);
                }
                return true;
            }

            bool set_preset(int slot, const params& p) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[slot] = p;
                return true;
            }

            const params& preset(int slot) const {
                return m_presets[static_cast<size_t>(std::clamp(slot, 0, k_presets - 1))];
            }

            bool morphing() const { return m_ramps_active > 0; }

            params snap_targets() const {
                params p;
                for (int i = 0; i < k_num_params; ++i) {
                    p.v[i] = m_ramp[i].target;
                }
                return p;
            }

            params snap_current() const {
                params p;
                for (int i = 0; i < k_num_params; ++i) {
                    p.v[i] = m_ramp[i].current;
                }
                return p;
            }

            double samplerate() const { return m_sr; }

            // -- audio ---------------------------------------------------------------------------------------

            /// Process one sample with the (ramped) frequency parameter.
            double process(double x) {
                tick_ramps();
                if (m_derived_dirty) {
                    update_derived(m_ramp[p_frequency].current);
                }
                return run(x);
            }

            /// Process one sample with a signal-rate cutoff override (Hz). The coefficient is recomputed
            /// every call on this path; the frequency parameter/ramp is left untouched.
            double process(double x, double cutoff_hz) {
                tick_ramps();
                update_derived(clamp_param(p_frequency, cutoff_hz));
                m_derived_dirty = true; // the cached gains belong to the override, not the parameter
                return run(x);
            }

            void process(const double* in, double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

          private:
            struct ramp {
                double current{0.0};
                double target{0.0};
                double inc{0.0};
                long   remaining{0};
            };

            // RBJ lowpass biquad (Transposed Direct Form II) — two in series make the 4th-order
            // Butterworth anti-image/anti-alias filters for the oversampling chain.
            struct biquad {
                double b0{1.0}, b1{0.0}, b2{0.0}, a1{0.0}, a2{0.0};
                double z1{0.0}, z2{0.0};

                void design_lowpass(double fc_norm, double q) { // fc_norm = fc / fs
                    const double w     = 2.0 * k_pi * fc_norm;
                    const double alpha = std::sin(w) / (2.0 * q);
                    const double cw    = std::cos(w);
                    const double a0    = 1.0 + alpha;
                    b0                 = ((1.0 - cw) * 0.5) / a0;
                    b1                 = (1.0 - cw) / a0;
                    b2                 = b0;
                    a1                 = (-2.0 * cw) / a0;
                    a2                 = (1.0 - alpha) / a0;
                }
                double tick(double x) {
                    const double y = b0 * x + z1;
                    z1             = b1 * x - a1 * y + z2;
                    z2             = b2 * x - a2 * y;
                    return y;
                }
                void reset() { z1 = z2 = 0.0; }
            };

            struct butterworth4 { // 4th-order Butterworth: two biquads with the standard Q pair
                biquad s1, s2;
                void   design(double fc_norm) {
                    s1.design_lowpass(fc_norm, 0.54119610);
                    s2.design_lowpass(fc_norm, 1.30656296);
                }
                double tick(double x) { return s2.tick(s1.tick(x)); }
                void   reset() {
                    s1.reset();
                    s2.reset();
                }
            };

            static bool valid_slot(int s) { return s >= 0 && s < k_presets; }

            static double anti_denormal(double x) {
                return (std::abs(x) < 1e-15) ? 0.0 : x; // house idiom (tap.comb~)
            }

            void ramp_to(int index, double tgt, long nsamples) {
                ramp&      r   = m_ramp[index];
                const bool was = r.remaining > 0;
                if (nsamples < 1 || tgt == r.current) {
                    r.current   = tgt;
                    r.target    = tgt;
                    r.inc       = 0.0;
                    r.remaining = 0;
                }
                else {
                    r.target    = tgt;
                    r.inc       = (tgt - r.current) / static_cast<double>(nsamples);
                    r.remaining = nsamples;
                }
                m_ramps_active += static_cast<int>(r.remaining > 0) - static_cast<int>(was);
                m_derived_dirty = true;
            }

            void tick_ramps() {
                if (m_ramps_active <= 0) {
                    return;
                }
                for (auto& r : m_ramp) {
                    if (r.remaining > 0) {
                        r.current += r.inc;
                        if (--r.remaining == 0) {
                            r.current = r.target;
                            --m_ramps_active;
                        }
                    }
                }
                m_derived_dirty = true;
            }

            void configure_resampler() {
                if (m_os > 1) {
                    // cut just below the original Nyquist, normalized to the oversampled rate
                    const double fc_norm = 0.45 / m_os;
                    m_up.design(fc_norm);
                    m_down.design(fc_norm);
                }
            }

            // Recompute the DSP coefficients from the (smoothed) parameter values for a given cutoff.
            void update_derived(double cutoff_hz) {
                const double fs_os = m_sr * m_os;
                const double fc    = std::min(cutoff_hz, 0.49 * fs_os);
                // Prewarped so the closed loop's sqrt(2)*w_stage oscillation lands exactly on fc.
                m_g             = std::tan(k_pi * fc / fs_os) * (1.0 / 1.41421356237309515);
                m_k             = k_fb_at_osc * m_ramp[p_resonance].current;
                m_gh            = 1.0 - hp_lp_gain(m_ramp[p_fbhp].current, fs_os); // instantaneous HP gain 1-G
                m_in_gain       = std::pow(10.0, m_ramp[p_drive].current * 0.05);
                m_out_gain      = std::pow(10.0, m_ramp[p_gain].current * 0.05);
                m_derived_dirty = (m_ramps_active > 0);
            }

            static double hp_lp_gain(double f_hp, double fs_os) {
                if (f_hp <= 0.0) {
                    return 0.0; // fbhp 0: the lowpass never moves — feedback is DC-coupled
                }
                const double gh = std::tan(k_pi * std::min(f_hp, 0.45 * fs_os) / fs_os);
                return gh / (1.0 + gh);
            }

            // The diode-pair conduction curve. Slope 1 at the origin, so at small signals the
            // filter IS the linearized Stinchcombe response; the knee bounds self-oscillation.
            static double sat(double e) { return std::tanh(e); }

            // Secant gain S(e)/e of the diode curve (series expansion near zero).
            static double secant_gain(double e) {
                const double a = std::abs(e);
                if (a < 1e-6) {
                    return 1.0 - e * e * (1.0 / 3.0);
                }
                return std::tanh(e) / e;
            }

            struct solution {
                double v1, v2, v3, v4, u;
            };

            // Exact solve of the linear ZDF system with per-edge gains gamma (the coupled chain +
            // high-passed feedback, eliminated bottom-up to closed form; unconditionally stable —
            // every divisor is >= 1 for g > 0, gamma in (0, 1]).
            solution solve_linear(double drive_in, const std::array<double, 4>& gamma) const {
                const double g   = m_g;
                const double g0  = g * gamma[0];
                const double g1  = g * gamma[1];
                const double g2  = g * gamma[2];
                const double g3  = g * gamma[3];
                const double amp = m_k * m_gh;            // instantaneous feedback gain onto v4
                const double l2  = drive_in + amp * m_sh; // feedback HPF state enters the loop here

                const double c4d = 1.0 + 2.0 * g3; // v4 = (s4 + 2*g3*v3) / c4d
                const double c43 = 2.0 * g3 / c4d;
                const double c40 = m_s4 / c4d;

                const double p3d = 1.0 + g2 + g3 - g3 * c43; // v3 = p30 + p32*v2
                const double p30 = (m_s3 + g3 * c40) / p3d;
                const double p32 = g2 / p3d;

                const double q2d = 1.0 + g1 + g2 - g2 * p32; // v2 = q20 + q21*v1
                const double q20 = (m_s2 + g2 * p30) / q2d;
                const double q21 = g1 / q2d;

                const double r30 = p30 + p32 * q20; // v3 = r30 + r31*v1
                const double r31 = p32 * q21;
                const double w40 = c40 + c43 * r30; // v4 = w40 + w41*v1
                const double w41 = c43 * r31;

                const double den = 1.0 + g0 + g1 - g1 * q21 + g0 * amp * w41;
                const double num = m_s1 + g0 * l2 + g1 * q20 - g0 * amp * w40;

                solution s;
                s.v1 = num / den;
                s.v2 = q20 + q21 * s.v1;
                s.v3 = r30 + r31 * s.v1;
                s.v4 = w40 + w41 * s.v1;
                s.u  = l2 - amp * s.v4;
                return s;
            }

            // Refresh the per-edge secant gains at a solution's operating point.
            void refresh_gamma(const solution& s, std::array<double, 4>& gamma) const {
                gamma[0] = secant_gain(s.u - s.v1);
                gamma[1] = secant_gain(s.v1 - s.v2);
                gamma[2] = secant_gain(s.v2 - s.v3);
                gamma[3] = secant_gain(s.v3 - s.v4);
            }

            // The nonlinear diode-ladder core at the oversampled rate.
            double core(double x) {
                const double drive_in = m_in_gain * x;

                // fast: solve at the previous sample's operating point, correct once.
                // exact: iterate the re-linearized solve until the node voltages settle.
                solution s = solve_linear(drive_in, m_gamma);
                refresh_gamma(s, m_gamma);
                if (m_solver == solver_fast) {
                    s = solve_linear(drive_in, m_gamma);
                    refresh_gamma(s, m_gamma); // carry gains at the committed operating point
                }
                else {
                    for (int it = 0; it < 32; ++it) {
                        const solution next = solve_linear(drive_in, m_gamma);
                        const double   step = std::abs(next.v1 - s.v1) + std::abs(next.v2 - s.v2)
                                            + std::abs(next.v3 - s.v3) + std::abs(next.v4 - s.v4);
                        s = next;
                        refresh_gamma(s, m_gamma);
                        if (step < 1e-12) {
                            break;
                        }
                    }
                }

                // trapezoidal state advance with the true diode currents at the solution
                const double f0 = sat(s.u - s.v1);
                const double f1 = sat(s.v1 - s.v2);
                const double f2 = sat(s.v2 - s.v3);
                const double f3 = sat(s.v3 - s.v4);
                m_s1            = anti_denormal(s.v1 + m_g * (f0 - f1));
                m_s2            = anti_denormal(s.v2 + m_g * (f1 - f2));
                m_s3            = anti_denormal(s.v3 + m_g * (f2 - f3));
                m_s4            = anti_denormal(s.v4 + 2.0 * m_g * f3);
                // feedback HPF's one-pole lowpass state (hp = (1-G)*(v4 - state) = m_gh*(v4 - m_sh))
                const double ylp = s.v4 - m_gh * (s.v4 - m_sh);
                m_sh             = anti_denormal(2.0 * ylp - m_sh);

                return s.v4;
            }

            double run(double x) {
                double y;
                if (m_os == 1) {
                    y = core(x);
                }
                else {
                    // zero-stuff + anti-image filter up, core at the high rate, anti-alias + decimate down
                    y = 0.0;
                    for (int j = 0; j < m_os; ++j) {
                        const double up = m_up.tick(j == 0 ? x * m_os : 0.0);
                        y               = m_down.tick(core(up));
                    }
                }
                return y * m_out_gain;
            }

            // configuration
            double m_sr{48000.0};
            double m_smooth_ms{k_default_smooth_ms};
            int    m_os{2};
            int    m_solver{solver_fast};
            bool   m_prepared{false};

            // parameters
            std::array<ramp, k_num_params> m_ramp;
            std::array<params, k_presets>  m_presets;
            int                            m_ramps_active{0};
            bool                           m_derived_dirty{true};

            // derived
            double m_g{0.05};
            double m_k{0.0};
            double m_gh{1.0};
            double m_in_gain{1.0};
            double m_out_gain{1.0};

            // state
            double                m_s1{0.0}, m_s2{0.0}, m_s3{0.0}, m_s4{0.0};
            double                m_sh{0.0};                     // feedback HPF one-pole lowpass state
            std::array<double, 4> m_gamma{{1.0, 1.0, 1.0, 1.0}}; // carried secant gains
            butterworth4          m_up, m_down;
        };

    } // namespace diode
} // namespace tap::tools
