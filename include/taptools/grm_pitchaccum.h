/// @file
/// @brief      Portable pitch-accumulator kernel for tap.pitchaccum~ — no Max/Min dependency.
/// @details    Modeled on the GRM Tools Classic "PitchAccum" plugin: two independent granular
///             transposers ("shadows"), each transposing up to +-2 octaves with its own delay
///             (up to 3 s), feedback, and gain. The feedback re-enters the transposer, so every
///             pass through the loop transposes AGAIN — the pitch "accumulation" spiral the
///             plugin is named for. A global LFO and per-voice random generators modulate the
///             transposition (the LFO reaches voice 2 with a settable phase offset); window and
///             crossfade expose the grain engine; an optional pitch follower adapts the window
///             to the input's detected period.
///
///             The transposer is the classic two-tap delay-line shifter (a phasor sweeps two
///             taps half a cycle apart across a window — same engine as tap.shift~ / tt_shift),
///             modernized: Hermite-interpolated fractional taps, and a complementary envelope
///             pair (cos^2 flanks of settable width, exact sum of 1) instead of tt_shift's fixed
///             padded-Welch table, so the output level is constant at any crossfade setting.
///
///             As in grm_comb.h (tap.5comb~): every parameter rides a per-sample linear ramp
///             (no zipper, glitch-free glides), and the 16-slot preset-morph engine lives in the
///             kernel — recall interpolates everything over a settable time; direct edits
///             override the morph for that one parameter. Double precision, allocation-free
///             after prepare(); setters only retarget ramps and are safe while audio runs.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

#include "numeric.h"

namespace tap::tools {
    namespace pitchaccum {

        constexpr int    k_voices            = 2;
        constexpr int    k_presets           = 16;     // same slot count as the other GRM recreations
        constexpr double k_max_delay_ms      = 3000.0; // Classic spec: up to ~2972 ms per transposer
        constexpr double k_min_window_ms     = 5.0;
        constexpr double k_max_window_ms     = 200.0;
        constexpr double k_max_trans_st      = 24.0; // +-2 octaves
        constexpr double k_fb_max            = 0.99; // loop gain cap; DC blocker guards the rest
        constexpr double k_dc_block_r        = 0.999;
        constexpr double k_base_delay        = 3.0; // samples; Hermite interpolation headroom
        constexpr double k_default_smooth_ms = 20.0;
        constexpr double k_pi                = 3.14159265358979323846;

        // pitch follower (normalized autocorrelation on decimated input; only runs when enabled)
        constexpr int    k_flw_decim      = 8;    // 48k -> 6k
        constexpr int    k_flw_buf        = 1024; // decimated ring (~170 ms @ 48k)
        constexpr int    k_flw_win        = 512;  // correlation window (~85 ms @ 48k)
        constexpr int    k_flw_interval   = 512;  // input samples between analyses
        constexpr double k_flw_fmin       = 50.0;
        constexpr double k_flw_fmax       = 800.0;
        constexpr double k_flw_confidence = 0.6;
        constexpr double k_flw_periods    = 2.0; // effective window = this many detected periods

        enum param_index : int {
            p_gain = 0,  // output gain, dB
            p_mix,       // 0..100 dry->wet, equal-power
            p_window,    // grain window, ms (manual value; the follower can override the effective one)
            p_xfade,     // 1..100, crossfade width as % of the half-window
            p_modfreq,   // LFO frequency, Hz
            p_moddepth,  // LFO depth, semitones
            p_modphase,  // LFO phase offset for voice 2, degrees
            p_randdepth, // random modulation depth, semitones
            p_randrate,  // random modulation rate, Hz
            p_trans1,    // voice transposition, semitones (fractional ok)
            p_trans2,
            p_delay1, // voice delay, ms
            p_delay2,
            p_fb1, // voice feedback, 0..99 (%)
            p_fb2,
            p_gain1, // voice gain, 0..100 (linear %)
            p_gain2,
            k_num_params // == 17
        };

        /// One full parameter snapshot — a preset slot, and the unit the morph engine interpolates.
        template <typename Sample>
        struct basic_params {
            std::array<Sample, k_num_params> v{};

            static basic_params defaults() {
                basic_params p;
                p.v[p_gain]      = Sample(0.0);
                p.v[p_mix]       = Sample(50.0);
                p.v[p_window]    = Sample(87.0); // tt_shift / tap.shift~ default
                p.v[p_xfade]     = Sample(50.0);
                p.v[p_modfreq]   = Sample(1.0);
                p.v[p_moddepth]  = Sample(0.0);
                p.v[p_modphase]  = Sample(90.0);
                p.v[p_randdepth] = Sample(0.0);
                p.v[p_randrate]  = Sample(5.0);
                p.v[p_trans1] = p.v[p_trans2] = Sample(0.0);
                p.v[p_delay1] = p.v[p_delay2] = Sample(0.0);
                p.v[p_fb1] = p.v[p_fb2] = Sample(0.0);
                p.v[p_gain1] = p.v[p_gain2] = Sample(50.0);
                return p;
            }
        };

        /// Clamp a value to the legal range of a parameter. Gain (dB) is unclamped.
        /// `Sample` is non-deduced and defaults to double so existing call sites — including
        /// the Max wrapper's, which passes a Min `atom` — keep compiling unchanged.
        template <typename Sample = double>
        Sample clamp_param(int index, std::type_identity_t<Sample> value) {
            switch (index) {
            case p_gain:
                return value;
            case p_mix:
                return std::clamp(value, Sample(0.0), Sample(100.0));
            case p_window:
                return std::clamp(value, k_min_window_ms, k_max_window_ms);
            case p_xfade:
                return std::clamp(value, Sample(1.0), Sample(100.0));
            case p_modfreq:
                return std::clamp(value, Sample(0.0), Sample(20.0));
            case p_moddepth:
                return std::clamp(value, Sample(0.0), k_max_trans_st);
            case p_modphase:
                return std::clamp(value, Sample(0.0), Sample(360.0));
            case p_randdepth:
                return std::clamp(value, Sample(0.0), k_max_trans_st);
            case p_randrate:
                return std::clamp(value, Sample(0.0), Sample(50.0));
            case p_trans1:
            case p_trans2:
                return std::clamp(value, -k_max_trans_st, k_max_trans_st);
            case p_delay1:
            case p_delay2:
                return std::clamp(value, Sample(0.0), k_max_delay_ms);
            case p_fb1:
            case p_fb2:
                return std::clamp(value, Sample(0.0), k_fb_max * Sample(100.0));
            case p_gain1:
            case p_gain2:
                return std::clamp(value, Sample(0.0), Sample(100.0));
            default:
                return value;
            }
        }

        /// One granular transposer with an accumulating (re-transposing) feedback loop.
        template <typename Sample>
        class basic_transposer {
            static_assert(is_sample_profile<Sample>,
                          "basic_transposer supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            void prepare(Sample sr) {
                const size_t n = static_cast<size_t>(
                                     std::ceil((Sample(k_max_delay_ms) + Sample(k_max_window_ms)) * Sample(0.001) * sr))
                                 + 16;
                m_buffer.assign(n, Sample(0.0));
                m_write = 0;
                m_phase = Sample(0.0);
                clear_state();
            }

            void clear() {
                std::fill(m_buffer.begin(), m_buffer.end(), Sample(0.0));
                clear_state();
            }

            bool prepared() const { return !m_buffer.empty(); }

            void reseed(uint32_t seed) { m_rng = seed; }

            /// Advance the random-modulation generator and return its current value in semitones.
            Sample tick_random(Sample depth_st, Sample rate_hz, Sample sr) {
                if (rate_hz <= Sample(0.0) || depth_st <= Sample(0.0)) {
                    m_rand_phase = Sample(1.0); // re-arm so the next enable starts a fresh segment
                    return Sample(0.0);
                }
                m_rand_phase += rate_hz / sr;
                if (m_rand_phase >= Sample(1.0)) {
                    m_rand_phase -= std::floor(m_rand_phase);
                    m_rand_from = m_rand_to;
                    m_rand_to   = uniform();
                }
                // cosine interpolation between held random values
                const Sample t = Sample(0.5) - Sample(0.5) * std::cos(Sample(k_pi_for<Sample>) * m_rand_phase);
                return depth_st * (m_rand_from + t * (m_rand_to - m_rand_from));
            }

            /// @param in              bank input sample
            /// @param ratio           pitch ratio (2 = up an octave), already modulated
            /// @param delay_samples   voice delay in samples (>= 0, fractional)
            /// @param window_samples  grain window in samples
            /// @param flank           envelope crossfade flank width as a fraction of the cycle, (0, 0.5]
            /// @param fb              feedback amount, [0, k_fb_max]
            Sample process(Sample in, Sample ratio, Sample delay_samples, Sample window_samples, Sample flank,
                           Sample fb) {
                // accumulating feedback: the voice's own transposed+delayed output re-enters the
                // transposer, DC-blocked, so each pass transposes again
                const Sample dc_out = m_fb_state - m_dc_x1 + Sample(k_dc_block_r) * m_dc_y1;
                m_dc_x1             = m_fb_state;
                m_dc_y1             = anti_denormal(dc_out);

                m_buffer[m_write] = anti_denormal(in + fb * dc_out);

                // two taps half a cycle apart on the same phasor, complementary envelopes summing to 1
                const Sample ph_a = m_phase;
                Sample       ph_b = ph_a + Sample(0.5);
                if (ph_b >= Sample(1.0)) {
                    ph_b -= Sample(1.0);
                }

                const Sample base = Sample(k_base_delay) + delay_samples;
                Sample       y    = Sample(0.0);
                const Sample ea   = envelope(ph_a, flank);
                const Sample eb   = envelope(ph_b, flank);
                if (ea > Sample(0.0)) {
                    y += ea * read_hermite(base + window_samples * ph_a);
                }
                if (eb > Sample(0.0)) {
                    y += eb * read_hermite(base + window_samples * ph_b);
                }

                // phasor rate keeps the taps sweeping at (1 - ratio) x real time (tt_shift provenance)
                m_phase += -(ratio - Sample(1.0)) / window_samples;
                m_phase -= std::floor(m_phase);

                if (++m_write >= m_buffer.size()) {
                    m_write = 0;
                }

                m_fb_state = y;
                return y;
            }

          private:
            void clear_state() {
                m_fb_state = Sample(0.0);
                m_dc_x1 = m_dc_y1 = Sample(0.0);
                m_rand_phase      = Sample(1.0);
                m_rand_from = m_rand_to = Sample(0.0);
            }

            Sample uniform() { // deterministic white noise in [-1, 1]
                m_rng = m_rng * 1664525u + 1013904223u;
                return (m_rng / Sample(2147483648.0)) - Sample(1.0);
            }

            // Complementary grain envelope: cos^2 rise over [0, flank], plateau to 0.5, cos^2 fall over
            // [0.5, 0.5 + flank], zero after — env(ph) + env(ph + 0.5 mod 1) == 1 exactly, so the two
            // taps crossfade at constant amplitude for any flank width. flank = 0.5 gives a full Hann.
            static Sample envelope(Sample ph, Sample flank) {
                if (ph <= flank) {
                    const Sample s = std::sin(Sample(k_pi_for<Sample>) * ph / (Sample(2.0) * flank));
                    return s * s;
                }
                if (ph <= Sample(0.5)) {
                    return Sample(1.0);
                }
                if (ph <= Sample(0.5) + flank) {
                    const Sample c = std::cos(Sample(k_pi_for<Sample>) * (ph - Sample(0.5)) / (Sample(2.0) * flank));
                    return c * c;
                }
                return Sample(0.0);
            }

            size_t wrap(long i) const {
                const long n = static_cast<long>(m_buffer.size());
                return static_cast<size_t>(((i % n) + n) % n);
            }

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

            std::vector<Sample> m_buffer;
            size_t              m_write{0};
            Sample              m_phase{Sample(0.0)};
            Sample              m_fb_state{Sample(0.0)};
            Sample              m_dc_x1{Sample(0.0)}, m_dc_y1{Sample(0.0)};
            uint32_t            m_rng{1u};
            Sample              m_rand_phase{Sample(1.0)};
            Sample              m_rand_from{Sample(0.0)}, m_rand_to{Sample(0.0)};
        };

        /// Lightweight pitch follower: normalized autocorrelation over a decimated ring of recent input.
        template <typename Sample>
        class basic_pitch_follower {
            static_assert(is_sample_profile<Sample>,
                          "basic_pitch_follower supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            void prepare(Sample sr) {
                m_sr = sr;
                m_ring.fill(0.0f);
                m_pos            = 0;
                m_decim_count    = 0;
                m_since_analysis = 0;
                m_period_s       = Sample(0.0);
            }

            /// Feed one input sample; returns true when a new analysis just ran.
            bool push(Sample x) {
                if (++m_decim_count >= k_flw_decim) {
                    m_decim_count = 0;
                    m_ring[m_pos] = static_cast<float>(x);
                    m_pos         = (m_pos + 1) % k_flw_buf;
                }
                if (++m_since_analysis >= k_flw_interval) {
                    m_since_analysis = 0;
                    analyze();
                    return true;
                }
                return false;
            }

            /// Detected period in seconds; 0 when unpitched / low confidence.
            Sample period_s() const { return m_period_s; }

          private:
            void analyze() {
                const Sample dsr     = m_sr / k_flw_decim;
                const int    lag_min = std::max(2, static_cast<int>(dsr / Sample(k_flw_fmax)));
                const int    lag_max = std::min(k_flw_buf - k_flw_win - 1, static_cast<int>(dsr / Sample(k_flw_fmin)));

                // most recent k_flw_win + lag_max samples, oldest first
                auto at = [&](int i) -> Sample {
                    return m_ring[(m_pos + k_flw_buf - 1 - i) % k_flw_buf]; // i samples ago
                };

                Sample e0 = Sample(0.0);
                for (int n = 0; n < k_flw_win; ++n) {
                    const Sample s = at(n);
                    e0 += s * s;
                }
                if (e0 < Sample(1e-9)) { // silence
                    m_period_s = Sample(0.0);
                    return;
                }

                std::array<Sample, k_flw_buf> corr{};
                Sample                        best_r = Sample(0.0);
                for (int lag = lag_min; lag <= lag_max; ++lag) {
                    Sample r = Sample(0.0), el = Sample(0.0);
                    for (int n = 0; n < k_flw_win; ++n) {
                        const Sample a = at(n);
                        const Sample b = at(n + lag);
                        r += a * b;
                        el += b * b;
                    }
                    const Sample norm = std::sqrt(e0 * el);
                    corr[lag]         = (norm > Sample(1e-12)) ? r / norm : Sample(0.0);
                    best_r            = std::max(best_r, corr[lag]);
                }

                // A periodic signal correlates at every multiple of its period — take the SMALLEST lag
                // whose correlation is close to the global maximum, not the global maximum itself,
                // or a subharmonic wins.
                int best_lag = 0;
                if (best_r >= Sample(k_flw_confidence)) {
                    const Sample accept = std::max(Sample(k_flw_confidence), Sample(0.85) * best_r);
                    for (int lag = lag_min; lag <= lag_max; ++lag) {
                        if (corr[lag] >= accept) {
                            best_lag = lag;
                            break;
                        }
                    }
                }
                m_period_s = (best_lag > 0) ? best_lag / dsr : Sample(0.0);
            }

            Sample                       m_sr{Sample(48000.0)};
            std::array<float, k_flw_buf> m_ring{};
            int                          m_pos{0};
            int                          m_decim_count{0};
            int                          m_since_analysis{0};
            Sample                       m_period_s{Sample(0.0)};
        };

        /// The full effect: two transposer voices, 17 per-sample-ramped parameters, LFO + random
        /// modulation, optional pitch follower, and the preset-morph engine.
        template <typename Sample>
        class basic_accum_bank {
            static_assert(is_sample_profile<Sample>,
                          "basic_accum_bank supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            basic_accum_bank() {
                const basic_params<Sample> d = basic_params<Sample>::defaults();
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = d.v[i];
                }
                m_presets.fill(d);
                m_voice[0].reseed(1111u);
                m_voice[1].reseed(2222u);
            }

            // -- lifecycle -------------------------------------------------------------------------------

            void prepare(Sample sr) {
                m_sr = (sr > Sample(0.0)) ? sr : Sample(48000.0);
                for (auto& v : m_voice) {
                    v.prepare(m_sr);
                }
                m_follower.prepare(m_sr);
                m_window_eff_ms = m_ramp[p_window].target;
                m_lfo_phase     = Sample(0.0);
                snap();
            }

            void clear() {
                for (auto& v : m_voice) {
                    v.clear();
                }
            }

            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = Sample(0.0);
                    r.remaining = 0;
                }
                m_ramps_active  = 0;
                m_derived_dirty = true;
            }

            // -- parameter targets (click-free; safe while audio runs) ------------------------------------

            void set_param(int index, Sample value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                ramp_to(index, clamp_param(index, value), smooth_samples());
            }

            void set_gain(Sample db) { set_param(p_gain, db); }
            void set_mix(Sample pct) { set_param(p_mix, pct); }
            void set_window(Sample ms) { set_param(p_window, ms); }
            void set_xfade(Sample pct) { set_param(p_xfade, pct); }
            void set_modfreq(Sample hz) { set_param(p_modfreq, hz); }
            void set_moddepth(Sample st) { set_param(p_moddepth, st); }
            void set_modphase(Sample deg) { set_param(p_modphase, deg); }
            void set_randdepth(Sample st) { set_param(p_randdepth, st); }
            void set_randrate(Sample hz) { set_param(p_randrate, hz); }
            void set_trans(int voice, Sample st) {
                if (valid_voice(voice)) {
                    set_param(p_trans1 + voice, st);
                }
            }
            void set_delay(int voice, Sample ms) {
                if (valid_voice(voice)) {
                    set_param(p_delay1 + voice, ms);
                }
            }
            void set_fb(int voice, Sample pct) {
                if (valid_voice(voice)) {
                    set_param(p_fb1 + voice, pct);
                }
            }
            void set_voice_gain(int voice, Sample pct) {
                if (valid_voice(voice)) {
                    set_param(p_gain1 + voice, pct);
                }
            }

            /// The pitch follower is a mode, not a fader — it is not part of the morphable parameter set.
            void set_follow(bool enabled) { m_follow = enabled; }
            bool follow() const { return m_follow; }

            void   set_smooth_ms(Sample ms) { m_smooth_ms = std::max(Sample(0.0), ms); }
            Sample smooth_ms() const { return m_smooth_ms; }

            // -- presets / morph ---------------------------------------------------------------------------

            bool store_preset(int slot) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[slot] = snap_targets();
                return true;
            }

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

            // -- introspection -----------------------------------------------------------------------------

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
            Sample effective_window_ms() const { return m_window_eff_ms; }

            // -- audio -------------------------------------------------------------------------------------

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
                    m_derived_dirty = (m_ramps_active > 0);
                }

                // follower: adapt the effective window toward ~2 detected periods (smoothed); relax back
                // to the manual window when disabled or unpitched
                Sample window_goal_ms = m_ramp[p_window].current;
                if (m_follow) {
                    m_follower.push(in);
                    const Sample period = m_follower.period_s();
                    if (period > Sample(0.0)) {
                        window_goal_ms = std::clamp(Sample(k_flw_periods) * period * Sample(1000.0),
                                                    Sample(k_min_window_ms), Sample(k_max_window_ms));
                    }
                }
                m_window_eff_ms += Sample(0.0005) * (window_goal_ms - m_window_eff_ms); // ~40 ms slew @ 48k
                const Sample window_samples = m_window_eff_ms * Sample(0.001) * m_sr;

                // modulation
                const Sample modfreq  = m_ramp[p_modfreq].current;
                const Sample moddepth = m_ramp[p_moddepth].current;
                m_lfo_phase += modfreq / m_sr;
                m_lfo_phase -= std::floor(m_lfo_phase);
                const Sample randdepth = m_ramp[p_randdepth].current;
                const Sample randrate  = m_ramp[p_randrate].current;

                Sample wet = Sample(0.0);
                for (int v = 0; v < k_voices; ++v) {
                    Sample lfo = Sample(0.0);
                    if (moddepth > Sample(0.0)) {
                        Sample ph = m_lfo_phase + (v == 1 ? m_ramp[p_modphase].current / Sample(360.0) : Sample(0.0));
                        ph -= std::floor(ph);
                        lfo = moddepth * std::sin(Sample(2.0) * Sample(k_pi_for<Sample>) * ph);
                    }
                    const Sample rnd       = m_voice[v].tick_random(randdepth, randrate, m_sr);
                    const Sample trans_eff = m_ramp[p_trans1 + v].current + lfo + rnd;
                    const Sample ratio     = std::exp2(trans_eff / Sample(12.0));

                    wet += m_voice[v].process(in, ratio, m_delay_samp[v], window_samples, m_flank, m_fb[v])
                           * m_voice_gain[v];
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

            // Recompute the ramp-derived values (per sample while ramps move, once when settled).
            // The modulation/ratio path is inherently per-sample and lives in process() instead.
            void update_derived() {
                const auto& val = m_ramp;

                m_flank = std::clamp(val[p_xfade].current, Sample(1.0), Sample(100.0)) * Sample(0.01)
                          * Sample(0.5); // fraction of cycle

                for (int v = 0; v < k_voices; ++v) {
                    m_delay_samp[v] = std::clamp(val[p_delay1 + v].current, Sample(0.0), Sample(k_max_delay_ms))
                                      * Sample(0.001) * m_sr;
                    m_fb[v]         = std::clamp(val[p_fb1 + v].current * Sample(0.01), Sample(0.0), Sample(k_fb_max));
                    m_voice_gain[v] = std::clamp(val[p_gain1 + v].current * Sample(0.01), Sample(0.0), Sample(1.0));
                }

                const Sample g     = std::pow(Sample(10.0), val[p_gain].current * Sample(0.05));
                const Sample theta = std::clamp(val[p_mix].current, Sample(0.0), Sample(100.0)) * Sample(0.01)
                                     * (Sample(k_pi_for<Sample>) * Sample(0.5));
                m_dry_gain = std::cos(theta) * g; // equal-power, matching tap.5comb~ / tap.crossfade~
                m_wet_gain = std::sin(theta) * g; // per-voice gain faders handle the wet balance
            }

            Sample m_sr{Sample(48000.0)};
            Sample m_smooth_ms{Sample(k_default_smooth_ms)};
            bool   m_follow{false};

            std::array<ramp, k_num_params>                 m_ramp;
            std::array<basic_transposer<Sample>, k_voices> m_voice;
            std::array<basic_params<Sample>, k_presets>    m_presets;
            basic_pitch_follower<Sample>                   m_follower;
            int                                            m_ramps_active{0};
            bool                                           m_derived_dirty{true};

            Sample m_lfo_phase{Sample(0.0)};
            Sample m_window_eff_ms{Sample(87.0)};

            // derived (cached while parameters are settled)
            std::array<Sample, k_voices> m_delay_samp{}, m_fb{}, m_voice_gain{};
            Sample                       m_flank{Sample(0.25)};
            Sample                       m_dry_gain{Sample(0.0)};
            Sample                       m_wet_gain{Sample(0.0)};
        };

        using params   = basic_params<double>;
        using params32 = basic_params<float>;

        /// The double profile — the golden model.
        using transposer     = basic_transposer<double>;
        using pitch_follower = basic_pitch_follower<double>;
        using accum_bank     = basic_accum_bank<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using transposer32     = basic_transposer<float>;
        using pitch_follower32 = basic_pitch_follower<float>;
        using accum_bank32     = basic_accum_bank<float>;

    } // namespace pitchaccum
} // namespace tap::tools
