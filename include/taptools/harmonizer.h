/// @file
/// @brief      Portable formant-preserving multi-voice harmonizer kernel — no Max/Min dependency.
/// @details    The keyboard-harmonizer effect (the DigiTech Vocalist lineage, "Hide and Seek"):
///             up to four pitch-shifted copies of a monophonic source, each holding a musical
///             interval, summed with an internally *latency-aligned* dry path so chords land as
///             chords rather than as slapback. Every voice is a tap::dsp::pvoc — the DspTap
///             phase vocoder's Laroche–Dolson peak-locked shifting — with LPC source-filter
///             formant preservation on by default, so shifted voices keep the singer's envelope
///             (the property that separates a harmonizer from a chipmunk chorus). Both
///             algorithms are implemented in DspTap from the published literature only, per the
///             project's IP policy; this kernel is composition and control.
///
///             Intervals are set in (fractional) semitones, clamped to ±24 — exactly the pvoc
///             ratio contract [1/4, 4] — and glide toward their targets through a one-pole slew
///             in the semitone domain (`set_glide`), so chord changes are click-free at the
///             default 10 ms and become an audible portamento at hundreds of ms. Gains ride
///             their own short slews (no zippers, the house rule).
///
///             A 16-slot preset-morph engine (the house pattern shared with vco.h):
///             `store_preset(slot)` snapshots the parameter *targets* — the four intervals, the
///             four gains, the dry gain, and the glide time, plus the formant flag — and
///             `recall_preset(slot, seconds)` glides every continuous parameter to the stored
///             values over `seconds` via per-sample linear ramps on the targets. The ordinary
///             one-pole slews keep smoothing underneath the moving targets, so chord-change
///             glide semantics outside morphs are untouched, and a voice whose gain rises from
///             zero during a morph still takes the cold re-entry path below. The formant flag
///             is a bool: it *snaps* at recall start rather than morphing. An explicit setter
///             called mid-morph cancels that one parameter's morph ramp — the performer wins.
///
///             Honest limits, stated where the next reader will look:
///             - Latency is exactly one FFT frame (`latency()` samples, fft_size; 1024 at
///               48 kHz ≈ 21 ms). The dry path is delayed inside the kernel to match, so the
///               output is time-coherent but the *object* is not a zero-latency insert.
///             - The phase-vocoder class smears transients; dense percussive input is the
///               wrong material. The formant model (LPC order 48) is speech/voice-oriented.
///             - A voice whose gain sits at zero is skipped entirely to save CPU and its
///               engine is cleared on re-entry: expect one frame of silence, then the gain
///               slew, when a voice is enabled mid-performance (by a setter or by a morph).
///             - The source should be monophonic for musical results — polyphonic input
///               shifts, but the formant estimate and the intervals stop meaning anything.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <vector>

#include "tap/dsp/pvoc.h"

namespace tap::tools {
    namespace harmony {

        constexpr int    k_max_voices       = 4;
        constexpr int    k_presets          = 16;
        constexpr double k_max_interval_st  = 24.0; // == the pvoc ratio clamp [1/4, 4]
        constexpr double k_max_gain         = 2.0;  // linear; wrappers speak dB
        constexpr double k_default_glide_ms = 10.0; // click-free, not yet audible
        constexpr double k_max_glide_ms     = 2000.0;
        constexpr double k_gain_slew_ms     = 10.0;
        constexpr double k_gain_epsilon     = 1e-4; // below this a voice is "off"
        constexpr size_t k_default_fft      = 1024; // the pvoc desktop operating point

        enum param_index : int {
            p_interval_0 = 0, // voice intervals, fractional semitones, ±24
            p_interval_1,
            p_interval_2,
            p_interval_3,
            p_gain_0, // voice gains, linear [0, 2]
            p_gain_1,
            p_gain_2,
            p_gain_3,
            p_dry,      // dry-path gain, linear [0, 2]
            p_glide_ms, // interval glide time constant, ms [0, 2000]
            k_num_params
        };

        /// One full parameter snapshot — a preset slot, and the unit the morph engine
        /// interpolates. The formant flag rides along but is not morphable: a bool has no
        /// middle, so it snaps at recall start.
        struct params {
            std::array<double, k_num_params> v{};
            bool                             formant = true;

            static params defaults() {
                params p; // intervals and gains default to 0
                p.v[p_dry]      = 1.0;
                p.v[p_glide_ms] = k_default_glide_ms;
                return p;
            }
        };

        /// Clamp a value to the legal range of a parameter.
        inline double clamp_param(int index, double value) {
            switch (index) {
            case p_interval_0:
            case p_interval_1:
            case p_interval_2:
            case p_interval_3:
                return std::clamp(value, -k_max_interval_st, k_max_interval_st);
            case p_gain_0:
            case p_gain_1:
            case p_gain_2:
            case p_gain_3:
            case p_dry:
                return std::clamp(value, 0.0, k_max_gain);
            case p_glide_ms:
                return std::clamp(value, 0.0, k_max_glide_ms);
            default:
                return value;
            }
        }

        /// Formant-preserving multi-voice harmonizer. House shape: prepare(sr) buys all
        /// geometry, process(in) is per-sample and allocation-free, every setter is safe
        /// while audio runs.
        class harmonizer {
          public:
            harmonizer() {
                const params d = params::defaults();
                m_target       = d.v;
                m_presets.fill(d);
            }

            /// Allocate the voice engines and the dry-alignment delay. @p fft_size follows
            /// the pvoc contract (power of two, >= 64); the default is the intended
            /// desktop operating point. Any pending preset morph completes instantly —
            /// its sample counts belonged to the previous rate.
            void prepare(double sr, size_t fft_size = k_default_fft) {
                m_sr  = sr;
                m_fft = fft_size;

                m_voices.clear();
                m_voices.reserve(static_cast<size_t>(k_max_voices));
                for (int v = 0; v < k_max_voices; ++v) {
                    auto& voice = m_voices.emplace_back(fft_size);
                    voice.set_formant(m_formant);
                }

                m_dry_ring.assign(fft_size, 0.0);
                m_dry_write = 0;

                finish_morphs();
                m_gain_coeff = slew_coeff(k_gain_slew_ms);
                refresh_glide_coeff();

                for (int v = 0; v < k_max_voices; ++v) {
                    const size_t i    = static_cast<size_t>(v);
                    m_current_st[i]   = m_target[static_cast<size_t>(p_interval_0 + v)];
                    m_current_gain[i] = m_target[static_cast<size_t>(p_gain_0 + v)];
                    m_active[i]       = m_current_gain[i] > k_gain_epsilon;
                }
                m_current_dry = m_target[p_dry];
            }

            bool prepared() const { return !m_voices.empty(); }

            /// Emission delay, in samples: one FFT frame, dry path included. 0 before prepare().
            size_t latency() const { return prepared() ? m_fft : 0; }

            /// Set a voice's interval in fractional semitones, clamped to ±24. The voice
            /// glides there through the `set_glide` slew.
            void set_interval(int voice, double semitones) {
                if (voice < 0 || voice >= k_max_voices) {
                    return;
                }
                set_target(p_interval_0 + voice, semitones);
            }

            double interval(int voice) const {
                return (voice >= 0 && voice < k_max_voices) ? m_target[static_cast<size_t>(p_interval_0 + voice)] : 0.0;
            }

            /// Set a voice's linear gain, clamped to [0, 2]. 0 disables the voice (its
            /// engine is skipped and re-enters cold — see the header note).
            void set_gain(int voice, double gain) {
                if (voice < 0 || voice >= k_max_voices) {
                    return;
                }
                set_target(p_gain_0 + voice, gain);
            }

            double gain(int voice) const {
                return (voice >= 0 && voice < k_max_voices) ? m_target[static_cast<size_t>(p_gain_0 + voice)] : 0.0;
            }

            /// Dry-path gain (latency-aligned inside the kernel), linear [0, 2], default 1.
            void   set_dry(double gain) { set_target(p_dry, gain); }
            double dry() const { return m_target[p_dry]; }

            /// LPC formant preservation on every voice (see tap::dsp::basic_pvoc). On by
            /// default — it is the point of the object; off is the chipmunk-chorus bend.
            /// A bool: preset recalls snap it at recall start, never morph it.
            void set_formant(bool on) {
                m_formant = on;
                for (auto& voice : m_voices) {
                    voice.set_formant(on);
                }
            }
            bool formant() const { return m_formant; }

            /// Interval glide time constant, ms, clamped [0, 2000]. 0 snaps.
            void   set_glide(double ms) { set_target(p_glide_ms, ms); }
            double glide() const { return m_target[p_glide_ms]; }

            // -- presets / morph (16 slots, the vco.h house pattern) -------------------------------

            /// Snapshot the current parameter *targets* (and the formant flag) into a slot.
            bool store_preset(int slot) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[static_cast<size_t>(slot)] = snap_targets();
                return true;
            }

            /// Glide every continuous parameter to the slot's values over @p seconds via
            /// per-sample linear ramps on the targets; the one-pole slews keep smoothing
            /// underneath. 0 s applies the preset immediately — exactly as if every setter
            /// had been called at once, so the ordinary glide/gain slews still de-click the
            /// landing. The formant bool snaps at recall start. A voice whose gain rises
            /// from zero mid-morph re-enters through the cold path, as always.
            bool recall_preset(int slot, double seconds) {
                if (!valid_slot(slot)) {
                    return false;
                }
                const params& p = m_presets[static_cast<size_t>(slot)];
                set_formant(p.formant);
                const long n = (m_sr > 0.0) ? static_cast<long>(std::max(0.0, seconds) * m_sr) : 0;
                for (int i = 0; i < k_num_params; ++i) {
                    morph_to(i, clamp_param(i, p.v[static_cast<size_t>(i)]), n);
                }
                return true;
            }

            /// Write a slot directly (e.g. from a wrapper's saved state).
            bool set_preset(int slot, const params& p) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[static_cast<size_t>(slot)] = p;
                return true;
            }

            const params& preset(int slot) const {
                return m_presets[static_cast<size_t>(std::clamp(slot, 0, k_presets - 1))];
            }

            bool morphing() const { return m_morph_active > 0; }

            /// The current parameter targets as a params snapshot (what store_preset saves).
            params snap_targets() const {
                params p;
                p.v       = m_target;
                p.formant = m_formant;
                return p;
            }

            /// Zero all running state. Slews jump to their targets and pending preset
            /// morphs complete instantly, so nothing fades in from stale values after a
            /// transport reset.
            void clear() {
                for (auto& voice : m_voices) {
                    voice.clear();
                }
                std::fill(m_dry_ring.begin(), m_dry_ring.end(), 0.0);
                m_dry_write = 0;
                finish_morphs();
                for (int v = 0; v < k_max_voices; ++v) {
                    const size_t i    = static_cast<size_t>(v);
                    m_current_st[i]   = m_target[static_cast<size_t>(p_interval_0 + v)];
                    m_current_gain[i] = m_target[static_cast<size_t>(p_gain_0 + v)];
                    m_active[i]       = m_current_gain[i] > k_gain_epsilon;
                }
                m_current_dry = m_target[p_dry];
            }

            /// Consume one input sample; produce the harmonized sample for time
            /// n - latency(). Passes the input through untouched before prepare().
            double process(double in) {
                if (!prepared()) {
                    return in;
                }

                tick_morph();

                // dry path, delayed to the voices' emission time
                const double dry        = m_dry_ring[m_dry_write];
                m_dry_ring[m_dry_write] = in;
                m_dry_write             = (m_dry_write + 1) % m_dry_ring.size();
                m_current_dry += m_gain_coeff * (m_target[p_dry] - m_current_dry);

                double out = m_current_dry * dry;

                for (int v = 0; v < k_max_voices; ++v) {
                    const size_t i           = static_cast<size_t>(v);
                    const double target_st   = m_target[static_cast<size_t>(p_interval_0 + v)];
                    const double target_gain = m_target[static_cast<size_t>(p_gain_0 + v)];
                    const bool   wanted      = target_gain > k_gain_epsilon;

                    if (!wanted && m_current_gain[i] <= k_gain_epsilon) {
                        m_active[i]       = false;
                        m_current_gain[i] = 0.0;
                        continue; // fully off: skip the engine entirely
                    }
                    if (wanted && !m_active[i]) {
                        m_voices[i].clear(); // re-enter cold, not with a stale splice
                        m_active[i] = true;
                    }

                    m_current_st[i] += m_glide_coeff * (target_st - m_current_st[i]);
                    m_current_gain[i] += m_gain_coeff * (target_gain - m_current_gain[i]);

                    const double ratio = std::exp2(m_current_st[i] / 12.0);
                    out += m_current_gain[i] * m_voices[i].process(in, ratio);
                }
                return out;
            }

          private:
            struct morph_ramp {
                double goal{0.0};
                double inc{0.0};
                long   remaining{0};
            };

            static bool valid_slot(int s) { return s >= 0 && s < k_presets; }

            double slew_coeff(double ms) const { return 1.0 - std::exp(-1.0 / (ms * 0.001 * m_sr)); }

            void refresh_glide_coeff() {
                const double ms = m_target[p_glide_ms];
                m_glide_coeff   = (ms <= 0.0 || m_sr <= 0.0) ? 1.0 : slew_coeff(ms);
            }

            /// A setter wins over a running morph: cancel that parameter's ramp, set the target.
            void set_target(int index, double value) {
                cancel_morph(index);
                m_target[static_cast<size_t>(index)] = clamp_param(index, value);
                if (index == p_glide_ms) {
                    refresh_glide_coeff();
                }
            }

            void cancel_morph(int index) {
                morph_ramp& m = m_morph[static_cast<size_t>(index)];
                if (m.remaining > 0) {
                    m.remaining = 0;
                    m.inc       = 0.0;
                    --m_morph_active;
                }
            }

            void morph_to(int index, double goal, long nsamples) {
                morph_ramp& m   = m_morph[static_cast<size_t>(index)];
                const bool  was = m.remaining > 0;
                if (nsamples < 1 || goal == m_target[static_cast<size_t>(index)]) {
                    m_target[static_cast<size_t>(index)] = goal;
                    m.inc                                = 0.0;
                    m.remaining                          = 0;
                    if (index == p_glide_ms) {
                        refresh_glide_coeff();
                    }
                }
                else {
                    m.goal      = goal;
                    m.inc       = (goal - m_target[static_cast<size_t>(index)]) / static_cast<double>(nsamples);
                    m.remaining = nsamples;
                }
                m_morph_active += static_cast<int>(m.remaining > 0) - static_cast<int>(was);
            }

            void finish_morphs() {
                if (m_morph_active <= 0) {
                    return;
                }
                for (int i = 0; i < k_num_params; ++i) {
                    morph_ramp& m = m_morph[static_cast<size_t>(i)];
                    if (m.remaining > 0) {
                        m_target[static_cast<size_t>(i)] = m.goal;
                        m.inc                            = 0.0;
                        m.remaining                      = 0;
                    }
                }
                m_morph_active = 0;
                refresh_glide_coeff();
            }

            void tick_morph() {
                if (m_morph_active <= 0) {
                    return;
                }
                for (int i = 0; i < k_num_params; ++i) {
                    morph_ramp& m = m_morph[static_cast<size_t>(i)];
                    if (m.remaining > 0) {
                        m_target[static_cast<size_t>(i)] += m.inc;
                        if (--m.remaining == 0) {
                            m_target[static_cast<size_t>(i)] = m.goal;
                            --m_morph_active;
                        }
                        if (i == p_glide_ms) {
                            refresh_glide_coeff();
                        }
                    }
                }
            }

            double m_sr  = 0.0;
            size_t m_fft = 0;

            std::vector<tap::dsp::pvoc> m_voices;
            std::vector<double>         m_dry_ring;
            size_t                      m_dry_write = 0;

            // parameter targets (indexed by param_index) and their morph ramps; the slewed
            // running values live below
            std::array<double, k_num_params>     m_target{};
            std::array<morph_ramp, k_num_params> m_morph{};
            std::array<params, k_presets>        m_presets{};
            int                                  m_morph_active = 0;

            std::array<double, k_max_voices> m_current_st{};
            std::array<double, k_max_voices> m_current_gain{};
            std::array<bool, k_max_voices>   m_active{};

            double m_current_dry = 1.0;
            bool   m_formant     = true;
            double m_glide_coeff = 1.0;
            double m_gain_coeff  = 1.0;
        };

    } // namespace harmony
} // namespace tap::tools
