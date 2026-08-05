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
///             Honest limits, stated where the next reader will look:
///             - Latency is exactly one FFT frame (`latency()` samples, fft_size; 1024 at
///               48 kHz ≈ 21 ms). The dry path is delayed inside the kernel to match, so the
///               output is time-coherent but the *object* is not a zero-latency insert.
///             - The phase-vocoder class smears transients; dense percussive input is the
///               wrong material. The formant model (LPC order 48) is speech/voice-oriented.
///             - A voice whose gain sits at zero is skipped entirely to save CPU and its
///               engine is cleared on re-entry: expect one frame of silence, then the gain
///               slew, when a voice is enabled mid-performance.
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
        constexpr double k_max_interval_st  = 24.0; // == the pvoc ratio clamp [1/4, 4]
        constexpr double k_max_gain         = 2.0;  // linear; wrappers speak dB
        constexpr double k_default_glide_ms = 10.0; // click-free, not yet audible
        constexpr double k_max_glide_ms     = 2000.0;
        constexpr double k_gain_slew_ms     = 10.0;
        constexpr double k_gain_epsilon     = 1e-4; // below this a voice is "off"
        constexpr size_t k_default_fft      = 1024; // the pvoc desktop operating point

        /// Formant-preserving multi-voice harmonizer. House shape: prepare(sr) buys all
        /// geometry, process(in) is per-sample and allocation-free, every setter is safe
        /// while audio runs.
        class harmonizer {
          public:
            /// Allocate the voice engines and the dry-alignment delay. @p fft_size follows
            /// the pvoc contract (power of two, >= 64); the default is the intended
            /// desktop operating point.
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

                m_gain_coeff = slew_coeff(k_gain_slew_ms);
                set_glide(m_glide_ms); // recompute the glide coefficient for this rate

                for (int v = 0; v < k_max_voices; ++v) {
                    m_current_st[static_cast<size_t>(v)]   = m_target_st[static_cast<size_t>(v)];
                    m_current_gain[static_cast<size_t>(v)] = m_target_gain[static_cast<size_t>(v)];
                    m_active[static_cast<size_t>(v)]       = m_target_gain[static_cast<size_t>(v)] > k_gain_epsilon;
                }
                m_current_dry = m_target_dry;
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
                m_target_st[static_cast<size_t>(voice)] = std::clamp(semitones, -k_max_interval_st, k_max_interval_st);
            }

            double interval(int voice) const {
                return (voice >= 0 && voice < k_max_voices) ? m_target_st[static_cast<size_t>(voice)] : 0.0;
            }

            /// Set a voice's linear gain, clamped to [0, 2]. 0 disables the voice (its
            /// engine is skipped and re-enters cold — see the header note).
            void set_gain(int voice, double gain) {
                if (voice < 0 || voice >= k_max_voices) {
                    return;
                }
                m_target_gain[static_cast<size_t>(voice)] = std::clamp(gain, 0.0, k_max_gain);
            }

            double gain(int voice) const {
                return (voice >= 0 && voice < k_max_voices) ? m_target_gain[static_cast<size_t>(voice)] : 0.0;
            }

            /// Dry-path gain (latency-aligned inside the kernel), linear [0, 2], default 1.
            void   set_dry(double gain) { m_target_dry = std::clamp(gain, 0.0, k_max_gain); }
            double dry() const { return m_target_dry; }

            /// LPC formant preservation on every voice (see tap::dsp::basic_pvoc). On by
            /// default — it is the point of the object; off is the chipmunk-chorus bend.
            void set_formant(bool on) {
                m_formant = on;
                for (auto& voice : m_voices) {
                    voice.set_formant(on);
                }
            }
            bool formant() const { return m_formant; }

            /// Interval glide time constant, ms, clamped [0, 2000]. 0 snaps.
            void set_glide(double ms) {
                m_glide_ms    = std::clamp(ms, 0.0, k_max_glide_ms);
                m_glide_coeff = (m_glide_ms <= 0.0 || m_sr <= 0.0) ? 1.0 : slew_coeff(m_glide_ms);
            }
            double glide() const { return m_glide_ms; }

            /// Zero all running state. Slews jump to their targets so nothing fades in
            /// from stale values after a transport reset.
            void clear() {
                for (auto& voice : m_voices) {
                    voice.clear();
                }
                std::fill(m_dry_ring.begin(), m_dry_ring.end(), 0.0);
                m_dry_write = 0;
                for (int v = 0; v < k_max_voices; ++v) {
                    m_current_st[static_cast<size_t>(v)]   = m_target_st[static_cast<size_t>(v)];
                    m_current_gain[static_cast<size_t>(v)] = m_target_gain[static_cast<size_t>(v)];
                    m_active[static_cast<size_t>(v)]       = m_target_gain[static_cast<size_t>(v)] > k_gain_epsilon;
                }
                m_current_dry = m_target_dry;
            }

            /// Consume one input sample; produce the harmonized sample for time
            /// n - latency(). Passes the input through untouched before prepare().
            double process(double in) {
                if (!prepared()) {
                    return in;
                }

                // dry path, delayed to the voices' emission time
                const double dry        = m_dry_ring[m_dry_write];
                m_dry_ring[m_dry_write] = in;
                m_dry_write             = (m_dry_write + 1) % m_dry_ring.size();
                m_current_dry += m_gain_coeff * (m_target_dry - m_current_dry);

                double out = m_current_dry * dry;

                for (int v = 0; v < k_max_voices; ++v) {
                    const size_t i      = static_cast<size_t>(v);
                    const bool   wanted = m_target_gain[i] > k_gain_epsilon;

                    if (!wanted && m_current_gain[i] <= k_gain_epsilon) {
                        m_active[i]       = false;
                        m_current_gain[i] = 0.0;
                        continue; // fully off: skip the engine entirely
                    }
                    if (wanted && !m_active[i]) {
                        m_voices[i].clear(); // re-enter cold, not with a stale splice
                        m_active[i] = true;
                    }

                    m_current_st[i] += m_glide_coeff * (m_target_st[i] - m_current_st[i]);
                    m_current_gain[i] += m_gain_coeff * (m_target_gain[i] - m_current_gain[i]);

                    const double ratio = std::exp2(m_current_st[i] / 12.0);
                    out += m_current_gain[i] * m_voices[i].process(in, ratio);
                }
                return out;
            }

          private:
            double slew_coeff(double ms) const { return 1.0 - std::exp(-1.0 / (ms * 0.001 * m_sr)); }

            double m_sr  = 0.0;
            size_t m_fft = 0;

            std::vector<tap::dsp::pvoc> m_voices;
            std::vector<double>         m_dry_ring;
            size_t                      m_dry_write = 0;

            std::array<double, k_max_voices> m_target_st{};
            std::array<double, k_max_voices> m_current_st{};
            std::array<double, k_max_voices> m_target_gain{};
            std::array<double, k_max_voices> m_current_gain{};
            std::array<bool, k_max_voices>   m_active{};

            double m_target_dry  = 1.0;
            double m_current_dry = 1.0;
            bool   m_formant     = true;
            double m_glide_ms    = k_default_glide_ms;
            double m_glide_coeff = 1.0;
            double m_gain_coeff  = 1.0;
        };

    } // namespace harmony
} // namespace tap::tools
