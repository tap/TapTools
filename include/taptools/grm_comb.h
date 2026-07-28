/// @file
/// @brief      Portable comb-filter-bank kernel for tap.5comb~ — no Max/Min dependency.
/// @details    Five parallel feedback comb filters modeled on the GRM Tools Classic "Comb Filters"
///             plugin: per-voice frequency / resonance / feedback-loop lowpass, master FREQ/RES/LP
///             multipliers, equal-power dry/wet mix and output gain, plus the modern GRM Comb's
///             Warp (in-loop allpass -> inharmonic partial stretch) and Phase (pickup position ->
///             odd/even harmonic balance), both neutral by default.
///
///             Provenance and deliberate deviations from the legacy tap.5comb~ (an abstraction over
///             five tap.comb~ objects, recovered from git history at b62bba8^):
///             - The 20-name parameter surface (freq/res/lp masters, freq1..5/res1..5/lp1..5,
///               gain, mix) and the 5 Hz effective-frequency floor (its 200 ms buffer) are kept.
///             - Delays are FRACTIONAL (4-point Hermite) and every parameter rides a per-sample
///               linear ramp; the legacy integer-sample, control-rate stepping detuned the combs
///               and zippered on sweeps — the main reason it never sounded like the GRM original.
///             - Resonance maps to ring time (log curve, k_rt60_min..k_rt60_max seconds), not
///               linearly to feedback: the GRM manual describes res as a duration ("longest
///               possible resonance") and a decay map is musical across the whole 0-100 travel.
///             - The feedback loop uses an exact one-pole lowpass coefficient and a DC blocker
///               instead of tap.comb~'s hard +-1 autoclip; feedback is capped just under unity, so
///               res 100 rings effectively forever *cleanly* (the legacy clipper distorted there).
///
///             The preset-morph engine (16 slots, timed interpolation of all parameters, GRM's
///             hallmark) lives here in the kernel so any host gets it.
///
///             All processing is double-precision, per-sample, allocation-free after prepare().
///             Setters are safe to call while audio runs (they only retarget ramps).
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

#include "numeric.h"

namespace tap::tools {
    namespace fivecomb {

        constexpr int    k_voices            = 5;
        constexpr int    k_presets           = 16;  // GRM Classic slot count
        constexpr double k_freq_floor_hz     = 5.0; // legacy 200 ms buffer implied this floor
        constexpr double k_freq_ceil_hz      = 20000.0;
        constexpr double k_fb_max            = 0.99999; // "longest possible resonance": clean, still BIBO-stable
        constexpr double k_min_delay_samples = 2.5;     // Hermite interpolation headroom
        constexpr double k_rt60_min          = 0.02;    // ring time at res -> 0+ (seconds)
        constexpr double k_rt60_max          = 100.0;   // ring time at res == 100 (fb cap makes it ~infinite)
        constexpr double k_warp_coef_max     = 0.85;    // |allpass coefficient| at warp == 100 (negative c:
                                                        // dispersion sits in the audible partial range and
                                                        // stretches upper partials sharp, stiff-string-like)
        constexpr double k_dc_block_r = 0.999;          // in-loop DC blocker pole (~7 Hz corner @ 48k)
        // The raw blocker (1 - z^-1)/(1 - R z^-1) peaks at 2/(1+R) > 1 at Nyquist; normalizing by
        // (1+R)/2 caps its gain at exactly 1, so the loop gain is bounded by fb alone and the loop
        // is strictly contractive for fb < 1 at every setting (found while proving boundedness for
        // the book's machine chapter: unnormalized, res 100 + lp wide open could swell ~0.2 dB/s).
        constexpr double k_dc_block_norm     = (1.0 + k_dc_block_r) * 0.5;
        constexpr double k_wet_norm          = 0.2;  // 1/k_voices wet-sum normalization
        constexpr double k_default_smooth_ms = 20.0; // anti-zipper ramp for direct setters
        constexpr double k_pi                = 3.14159265358979323846;

        enum param_index : int {
            p_gain = 0,                        // output gain, dB
            p_mix,                             // 0..100 dry->wet, equal-power
            p_freq_master,                     // 0..2, multiplies freq1..5  (legacy name: "freq")
            p_res_master,                      // 0..2, multiplies res1..5   (legacy name: "res")
            p_lp_master,                       // 0..2, multiplies lp1..5    (legacy name: "lp")
            p_warp,                            // 0..100, in-loop allpass partial warp (0 = harmonic/Classic)
            p_phase,                           // 0..100, pickup position (100 = odd harmonics only)
            p_freq1,                           // .. p_freq1 + 4, per-voice frequency, 45..20000 Hz
            p_res1       = p_freq1 + k_voices, // .. p_res1 + 4, per-voice resonance, 0..100
            p_lp1        = p_res1 + k_voices,  // .. p_lp1 + 4, per-voice feedback lowpass cutoff, 0..20000 Hz
            k_num_params = p_lp1 + k_voices    // == 22
        };

        /// One full parameter snapshot — a preset slot, and the unit the morph engine interpolates.
        template <typename Sample>
        struct basic_params {
            std::array<Sample, k_num_params> v{};

            static basic_params defaults() {
                basic_params p;
                p.v[p_gain]                      = Sample(0.0);
                p.v[p_mix]                       = Sample(100.0);
                p.v[p_freq_master]               = Sample(1.0);
                p.v[p_res_master]                = Sample(1.0);
                p.v[p_lp_master]                 = Sample(1.0);
                p.v[p_warp]                      = Sample(0.0);
                p.v[p_phase]                     = Sample(0.0);
                constexpr Sample freqs[k_voices] = {Sample(80.0), Sample(120.0), Sample(160.0), Sample(200.0),
                                                    Sample(102.0)}; // legacy factory preset 1
                for (int i = 0; i < k_voices; ++i) {
                    p.v[p_freq1 + i] = freqs[i];
                    p.v[p_res1 + i]  = Sample(50.0);
                    p.v[p_lp1 + i]   = Sample(20000.0);
                }
                return p;
            }
        };

        /// Clamp a value to the legal range of a parameter. Gain (dB) is unclamped, like the legacy flonum.
        /// `Sample` is non-deduced and defaults to double so existing call sites — including
        /// the Max wrapper's, which passes a Min `atom` — keep compiling unchanged.
        template <typename Sample = double>
        Sample clamp_param(int index, std::type_identity_t<Sample> value) {
            if (index == p_gain) {
                return value;
            }
            if (index == p_mix || index == p_warp || index == p_phase) {
                return std::clamp(value, Sample(0.0), Sample(100.0));
            }
            if (index == p_freq_master || index == p_res_master || index == p_lp_master) {
                return std::clamp(value, Sample(0.0), Sample(2.0));
            }
            if (index >= p_freq1 && index < p_freq1 + k_voices) {
                return std::clamp(value, Sample(45.0), k_freq_ceil_hz);
            }
            if (index >= p_res1 && index < p_res1 + k_voices) {
                return std::clamp(value, Sample(0.0), Sample(100.0));
            }
            if (index >= p_lp1 && index < p_lp1 + k_voices) {
                return std::clamp(value, Sample(0.0), k_freq_ceil_hz);
            }
            return value;
        }

        /// A single feedback comb: fractional-delay circular buffer with a one-pole lowpass, DC blocker,
        /// and first-order allpass (warp) in the feedback path, plus a half-loop pickup tap (phase).
        /// All per-sample parameters are passed in by the bank; the voice owns only signal state.
        template <typename Sample>
        class basic_comb_voice {
            static_assert(is_sample_profile<Sample>,
                          "basic_comb_voice supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            void prepare(Sample sr) {
                const size_t n = static_cast<size_t>(std::ceil(sr / Sample(k_freq_floor_hz))) + 8;
                m_buffer.assign(n, Sample(0.0));
                m_write = 0;
                clear_filters();
            }

            void clear() {
                std::fill(m_buffer.begin(), m_buffer.end(), Sample(0.0));
                clear_filters();
            }

            bool prepared() const { return !m_buffer.empty(); }

            /// @param in         input sample
            /// @param d_read     main loop tap in samples (already warp-compensated), >= 2.5, fractional
            /// @param d_half     pickup tap in samples (half the nominal loop delay), >= 1
            /// @param fb         feedback coefficient, [0, k_fb_max]
            /// @param lp_a       one-pole lowpass coefficient a in state += a*(x - state), [0, 1]
            /// @param ap_c       warp allpass coefficient, [0, k_warp_coef_max]
            /// @param pickup     phase pickup amount, [0, 1]
            Sample process(Sample in, Sample d_read, Sample d_half, Sample fb, Sample lp_a, Sample ap_c,
                           Sample pickup) {
                // feedback path: delayed -> lowpass -> DC block -> warp allpass -> * fb
                const Sample delayed = read_hermite(d_read);

                m_lp_state = anti_denormal(m_lp_state + lp_a * (delayed - m_lp_state));

                const Sample dc_out = Sample(k_dc_block_norm) * (m_lp_state - m_dc_x1) + Sample(k_dc_block_r) * m_dc_y1;
                m_dc_x1             = m_lp_state;
                m_dc_y1             = anti_denormal(dc_out);

                const Sample ap_out = ap_c * dc_out + m_ap_x1 - ap_c * m_ap_y1;
                m_ap_x1             = dc_out;
                m_ap_y1             = anti_denormal(ap_out);

                const Sample y    = anti_denormal(in + fb * ap_out);
                m_buffer[m_write] = y;

                // pickup: subtracting the half-loop tap cancels even harmonics as pickup -> 1
                const Sample out = y - pickup * read_linear(d_half);

                if (++m_write >= m_buffer.size()) {
                    m_write = 0;
                }
                return out;
            }

          private:
            void clear_filters() {
                m_lp_state = Sample(0.0);
                m_dc_x1 = m_dc_y1 = Sample(0.0);
                m_ap_x1 = m_ap_y1 = Sample(0.0);
            }

            size_t wrap(long i) const {
                const long n = static_cast<long>(m_buffer.size());
                return static_cast<size_t>(((i % n) + n) % n);
            }

            // 4-point, 3rd-order Hermite. Position D samples behind the current write index; needs D >= 2
            // strictly so the youngest point used is the most recently written sample.
            Sample read_hermite(Sample d) const {
                const Sample pos  = static_cast<Sample>(m_write) - d;
                const Sample fpos = std::floor(pos);
                const Sample frac = pos - fpos;
                const long   base = static_cast<long>(fpos);
                const Sample xm1  = m_buffer[wrap(base - 1)];
                const Sample x0   = m_buffer[wrap(base)];
                const Sample x1   = m_buffer[wrap(base + 1)];
                const Sample x2   = m_buffer[wrap(base + 2)];
                const Sample c    = (x1 - xm1) * Sample(0.5);
                const Sample v    = x0 - x1;
                const Sample w    = c + v;
                const Sample a    = w + v + (x2 - x0) * Sample(0.5);
                const Sample b    = w + a;
                return (((a * frac - b) * frac + c) * frac + x0);
            }

            Sample read_linear(Sample d) const {
                const Sample pos  = static_cast<Sample>(m_write) - d;
                const Sample fpos = std::floor(pos);
                const Sample frac = pos - fpos;
                const long   base = static_cast<long>(fpos);
                const Sample x0   = m_buffer[wrap(base)];
                const Sample x1   = m_buffer[wrap(base + 1)];
                return x0 + frac * (x1 - x0);
            }

            std::vector<Sample> m_buffer;
            size_t              m_write{0};
            Sample              m_lp_state{Sample(0.0)};
            Sample              m_dc_x1{Sample(0.0)}, m_dc_y1{Sample(0.0)}; // DC blocker: y = x - x1 + R*y1
            Sample              m_ap_x1{Sample(0.0)}, m_ap_y1{Sample(0.0)}; // allpass:    y = c*x + x1 - c*y1
        };

        /// The bank: five voices, 22 per-sample-ramped parameters, and the preset-morph engine.
        template <typename Sample>
        class basic_comb_bank {
            static_assert(is_sample_profile<Sample>,
                          "basic_comb_bank supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            basic_comb_bank() {
                const basic_params<Sample> d = basic_params<Sample>::defaults();
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = d.v[i];
                }
                m_presets.fill(d);
            }

            // -- lifecycle -----------------------------------------------------------------------------

            /// (Re)allocate voice buffers for the sample rate, snap all ramps to their targets (a DSP
            /// restart is not a morph), and clear signal state. Not real-time-safe.
            void prepare(Sample sr) {
                m_sr = (sr > Sample(0.0)) ? sr : Sample(48000.0);
                for (auto& v : m_voice) {
                    v.prepare(m_sr);
                }
                snap();
            }

            /// Zero all delay lines and filter state; parameters are untouched.
            void clear() {
                for (auto& v : m_voice) {
                    v.clear();
                }
            }

            /// Finish all ramps immediately (jump to targets).
            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = Sample(0.0);
                    r.remaining = 0;
                }
                m_ramps_active  = 0;
                m_derived_dirty = true;
            }

            // -- parameter targets (click-free; safe while audio runs) ----------------------------------

            /// Generic path: clamp and ramp to the new value over the anti-zipper smoothing time.
            void set_param(int index, Sample value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                ramp_to(index, clamp_param(index, value), smooth_samples());
            }

            void set_gain(Sample db) { set_param(p_gain, db); }
            void set_mix(Sample pct) { set_param(p_mix, pct); }
            void set_freq_master(Sample x) { set_param(p_freq_master, x); }
            void set_res_master(Sample x) { set_param(p_res_master, x); }
            void set_lp_master(Sample x) { set_param(p_lp_master, x); }
            void set_warp(Sample pct) { set_param(p_warp, pct); }
            void set_phase(Sample pct) { set_param(p_phase, pct); }
            void set_freq(int voice, Sample hz) {
                if (valid_voice(voice)) {
                    set_param(p_freq1 + voice, hz);
                }
            }
            void set_res(int voice, Sample pct) {
                if (valid_voice(voice)) {
                    set_param(p_res1 + voice, pct);
                }
            }
            void set_lp(int voice, Sample hz) {
                if (valid_voice(voice)) {
                    set_param(p_lp1 + voice, hz);
                }
            }

            /// Anti-zipper ramp time for direct setters, in ms. 0 = instant (useful for tests).
            void   set_smooth_ms(Sample ms) { m_smooth_ms = std::max(Sample(0.0), ms); }
            Sample smooth_ms() const { return m_smooth_ms; }

            // -- presets / morph -----------------------------------------------------------------------

            /// Capture the current *targets* (knob positions, not mid-ramp instantaneous values).
            bool store_preset(int slot) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[slot] = snap_targets();
                return true;
            }

            /// Morph every parameter from wherever it currently is to the preset, over `seconds`.
            /// Re-targeting mid-morph stays continuous; seconds <= 0 jumps.
            bool recall_preset(int slot, Sample seconds) {
                if (!valid_slot(slot)) {
                    return false;
                }
                const long n = static_cast<long>(std::max(Sample(0.0), seconds) * m_sr);
                for (int i = 0; i < k_num_params; ++i) {
                    ramp_to(i, clamp_param(i, m_presets[slot].v[i]), n);
                }
                return true;
            }

            /// Programmatic preset load (factory presets, state restore).
            bool set_preset(int slot, const basic_params<Sample>& p) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[slot] = p;
                return true;
            }

            const basic_params<Sample>& preset(int slot) const {
                return m_presets[static_cast<size_t>(std::clamp(slot, 0, k_presets - 1))];
            }

            bool morphing() const { return m_ramps_active > 0; }

            // -- introspection ---------------------------------------------------------------------------

            basic_params<Sample> snap_targets() const {
                basic_params<Sample> p;
                for (int i = 0; i < k_num_params; ++i) {
                    p.v[i] = m_ramp[i].target;
                }
                return p;
            }

            basic_params<Sample> snap_current() const {
                basic_params<Sample> p;
                for (int i = 0; i < k_num_params; ++i) {
                    p.v[i] = m_ramp[i].current;
                }
                return p;
            }

            Sample samplerate() const { return m_sr; }

            // -- audio -----------------------------------------------------------------------------------

            Sample process(Sample in) {
                if (!m_voice[0].prepared()) {
                    return in;
                }

                if (m_ramps_active > 0) {
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
                if (m_derived_dirty) {
                    update_derived();
                    m_derived_dirty = (m_ramps_active > 0); // keep recomputing per sample while moving
                }

                Sample wet = Sample(0.0);
                for (int v = 0; v < k_voices; ++v) {
                    wet += m_voice[v].process(in, m_d_read[v], m_d_half[v], m_fb[v], m_lp_a[v], m_ap_c, m_pickup);
                }

                return in * m_dry_gain + wet * m_wet_gain;
            }

            void process(const Sample* in, Sample* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

          private:
            struct ramp {
                Sample current{Sample(0.0)};
                Sample target{Sample(0.0)};
                Sample inc{Sample(0.0)};
                long   remaining{0};
            };

            static bool valid_voice(int v) { return v >= 0 && v < k_voices; }
            static bool valid_slot(int s) { return s >= 0 && s < k_presets; }

            long smooth_samples() const { return static_cast<long>(m_smooth_ms * Sample(0.001) * m_sr); }

            void ramp_to(int index, Sample tgt, long nsamples) {
                ramp&      r   = m_ramp[index];
                const bool was = r.remaining > 0;
                if (nsamples < 1 || tgt == r.current) {
                    r.current   = tgt;
                    r.target    = tgt;
                    r.inc       = Sample(0.0);
                    r.remaining = 0;
                }
                else {
                    r.target    = tgt;
                    r.inc       = (tgt - r.current) / static_cast<Sample>(nsamples);
                    r.remaining = nsamples;
                }
                m_ramps_active += static_cast<int>(r.remaining > 0) - static_cast<int>(was);
                m_derived_dirty = true;
            }

            // Magnitude of the normalized DC blocker at normalized frequency w — used to keep the
            // res -> ring-time calibration exact at each voice's fundamental (the blocker slightly
            // attenuates low fundamentals; uncompensated, that shortens the ring). Bounded by 1.
            static Sample dc_block_gain(Sample w) {
                const Sample num = Sample(2.0) - Sample(2.0) * std::cos(w);
                const Sample den = Sample(1.0) - Sample(2.0) * Sample(k_dc_block_r) * std::cos(w)
                                   + Sample(k_dc_block_r) * Sample(k_dc_block_r);
                return Sample(k_dc_block_norm) * std::sqrt(num / std::max(den, Sample(1e-30)));
            }

            // Exact phase delay (in samples) of the first-order allpass (c + z^-1)/(1 + c z^-1) at
            // normalized frequency w. Used to keep each voice's fundamental in tune under warp.
            static Sample allpass_phase_delay(Sample c, Sample w) {
                if (w < Sample(1e-9)) {
                    return (Sample(1.0) - c) / (Sample(1.0) + c); // group delay at DC
                }
                const Sample num = std::atan2(std::sin(w), c + std::cos(w));
                const Sample den = std::atan2(c * std::sin(w), Sample(1.0) + c * std::cos(w));
                return (num - den) / w;
            }

            // Recompute all derived per-sample DSP values from the (smoothed) parameter values.
            // Runs every sample while any ramp is active, once when everything has settled.
            void update_derived() {
                const auto& val = m_ramp;

                // warp allpass: c = 0 is a pure one-sample delay (linear phase), so warp 0 is exactly the
                // Classic harmonic comb. Negative c disperses the loop — its phase delay falls with
                // frequency, so upper partials round-trip faster and stretch sharp (piano-string-like).
                // The main tap is compensated at each voice's *fundamental*, which therefore stays in tune.
                m_ap_c = -Sample(k_warp_coef_max) * std::clamp(val[p_warp].current, Sample(0.0), Sample(100.0))
                         * Sample(0.01);

                m_pickup = std::clamp(val[p_phase].current, Sample(0.0), Sample(100.0)) * Sample(0.01);

                const Sample f_ceil = std::min(Sample(k_freq_ceil_hz), m_sr / Sample(k_min_delay_samples));

                for (int v = 0; v < k_voices; ++v) {
                    const Sample f       = std::clamp(val[p_freq1 + v].current * val[p_freq_master].current,
                                                      Sample(k_freq_floor_hz), f_ceil);
                    const Sample d_total = m_sr / f; // nominal loop delay
                    const Sample ap_tau =
                        allpass_phase_delay(m_ap_c, Sample(2.0) * Sample(k_pi_for<Sample>) * f / m_sr);
                    // At extreme warp x high tuning the loop can't get shorter than the dispersion and the
                    // pitch flattens — physical, and documented in the maxref.
                    m_d_read[v] = std::max(d_total - ap_tau, Sample(k_min_delay_samples));
                    m_d_half[v] = std::max(d_total * Sample(0.5), Sample(1.0));

                    // res -> ring time (log curve) -> feedback for the *current* delay, so a voice keeps
                    // its ring time as its frequency sweeps. RT60 relation: fb = 10^(-3 * d / rt60).
                    const Sample res_eff =
                        std::clamp(val[p_res1 + v].current * val[p_res_master].current, Sample(0.0), Sample(200.0));
                    if (res_eff <= Sample(0.0)) {
                        m_fb[v] = Sample(0.0);
                    }
                    else {
                        const Sample rt60 = Sample(k_rt60_min)
                                            * std::pow(Sample(k_rt60_max) / Sample(k_rt60_min), res_eff * Sample(0.01));
                        // Compensate the blocker's gain at the fundamental (same philosophy as the
                        // warp phase compensation above) so the per-pass level really is the RT60
                        // target; the k_fb_max clamp keeps the loop contractive regardless, since
                        // the normalized blocker never exceeds unity at any frequency.
                        const Sample dc_g = dc_block_gain(Sample(2.0) * Sample(k_pi_for<Sample>) * f / m_sr);
                        const Sample tgt  = std::pow(Sample(10.0), -Sample(3.0) * d_total / (rt60 * m_sr));
                        m_fb[v]           = std::min(tgt / std::max(dc_g, Sample(0.5)), Sample(k_fb_max));
                    }

                    // exact one-pole coefficient (tap.comb~ used the cruder hz*2/sr approximation)
                    const Sample fc = std::clamp(val[p_lp1 + v].current * val[p_lp_master].current, Sample(0.0),
                                                 Sample(k_freq_ceil_hz));
                    m_lp_a[v]       = Sample(1.0) - std::exp(-Sample(2.0) * Sample(k_pi_for<Sample>) * fc / m_sr);
                }

                const Sample g     = std::pow(Sample(10.0), val[p_gain].current * Sample(0.05));
                const Sample theta = std::clamp(val[p_mix].current, Sample(0.0), Sample(100.0)) * Sample(0.01)
                                     * (Sample(k_pi_for<Sample>) * Sample(0.5));
                m_dry_gain = std::cos(theta) * g; // equal-power, like tap.crossfade~
                m_wet_gain = std::sin(theta) * g * Sample(k_wet_norm);
            }

            Sample m_sr{Sample(48000.0)};
            Sample m_smooth_ms{Sample(k_default_smooth_ms)};

            std::array<ramp, k_num_params>                 m_ramp;
            std::array<basic_comb_voice<Sample>, k_voices> m_voice;
            std::array<basic_params<Sample>, k_presets>    m_presets;
            int                                            m_ramps_active{0};
            bool                                           m_derived_dirty{true};

            // derived (cached while parameters are settled)
            std::array<Sample, k_voices> m_d_read{}, m_d_half{}, m_fb{}, m_lp_a{};
            Sample                       m_ap_c{Sample(0.0)};
            Sample                       m_pickup{Sample(0.0)};
            Sample                       m_dry_gain{Sample(0.0)};
            Sample                       m_wet_gain{Sample(0.0)};
        };

        using params   = basic_params<double>;
        using params32 = basic_params<float>;

        /// The double profile — the golden model.
        using comb_voice = basic_comb_voice<double>;
        using comb_bank  = basic_comb_bank<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using comb_voice32 = basic_comb_voice<float>;
        using comb_bank32  = basic_comb_bank<float>;

    } // namespace fivecomb
} // namespace tap::tools
