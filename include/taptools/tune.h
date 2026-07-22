/// @file
/// @brief      Portable pitch-correction kernel for tap.tune~ — no Max/Min dependency.
/// @details    Classic real-time monophonic pitch correction: detect the input's period, snap it to
///             the nearest allowed note, and retune by that ratio — with the retune speed (the time
///             constant of the glide toward the target) as the primary musical control, from
///             transparent correction down to the hard-snap effect at zero.
///
///             The pipeline is the public-domain design (the foundational 1998 patent expired in
///             2018): a YIN-family detector (the shared DspTap primitive, tap::dsp::yin — full-rate,
///             cumulative-mean normalized, parabolic sub-sample lag) feeds a scale/key mapper, and a
///             two-tap delay-line transposer resynthesizes — the same engine as tap.shift~ and the
///             tap.pitchaccum~ voices (Hermite-interpolated fractional taps, complementary sin^2 /
///             cos^2 envelope pair summing to 1), with its grain window locked to two detected
///             periods. Period-locking makes the two taps sit exactly one period apart on voiced
///             input, so they sum coherently — the property that carries a delay-line shifter most
///             of the way to pitch-synchronous (PSOLA) quality. On unpitched input the correction
///             relaxes to zero and the tap pair imposes a mild moving comb coloration — the known
///             trade of this engine class; a pitch-synchronous backend behind this same interface
///             is the planned upgrade.
///
///             Two target modes: `scale` snaps to a key + 12-bit scale mask (per-note enables,
///             editable individually), `midi` snaps to the nearest currently-held MIDI note and
///             leaves the signal untouched when none are held. Detection runs every ~5 ms on a
///             worst-case lag range fixed at prepare(); the user frequency range only filters
///             results, so every setter is allocation-free and safe while audio runs. Double
///             precision throughout; C++ stdlib + DspTap only.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <optional>
#include <vector>

#include "tap/dsp/yin.h"

namespace tap::tools {
    namespace tune {

        constexpr double k_pi = 3.14159265358979323846;

        // detection geometry (worst-case bounds; the user range filters within them)
        constexpr double k_floor_freq_hz  = 55.0;            // deepest searched lag (A1)
        constexpr double k_ceil_freq_hz   = 2000.0;          // shallowest searched lag
        constexpr double k_hop_seconds    = 256.0 / 48000.0; // analysis cadence (~5.3 ms)
        constexpr double k_default_min_hz = 60.0;
        constexpr double k_default_max_hz = 1500.0;

        // correction
        constexpr double k_default_speed_ms  = 20.0; // retune time constant; 0 = hard snap
        constexpr double k_max_correction_st = 12.0;

        // resynthesis
        constexpr double k_window_periods    = 2.0; // grain window = this many detected periods
        constexpr double k_min_window_ms     = 5.0;
        constexpr double k_max_window_ms     = 100.0;
        constexpr double k_default_window_ms = 30.0; // used until the first detection lands
        constexpr double k_window_slew_ms    = 15.0;
        constexpr double k_base_delay        = 3.0; // samples; Hermite interpolation headroom

        /// Build a 12-bit pitch-class mask from scale degrees (bit 0 = the root).
        constexpr unsigned make_mask(std::initializer_list<int> degrees) {
            unsigned mask = 0u;
            for (const int d : degrees) {
                mask |= 1u << (((d % 12) + 12) % 12);
            }
            return mask;
        }

        // scale presets, expressed relative to the key root
        constexpr unsigned k_scale_chromatic        = 0xFFFu;
        constexpr unsigned k_scale_major            = make_mask({0, 2, 4, 5, 7, 9, 11});
        constexpr unsigned k_scale_minor            = make_mask({0, 2, 3, 5, 7, 8, 10});
        constexpr unsigned k_scale_harmonic_minor   = make_mask({0, 2, 3, 5, 7, 8, 11});
        constexpr unsigned k_scale_melodic_minor    = make_mask({0, 2, 3, 5, 7, 9, 11});
        constexpr unsigned k_scale_major_pentatonic = make_mask({0, 2, 4, 7, 9});
        constexpr unsigned k_scale_minor_pentatonic = make_mask({0, 3, 5, 7, 10});

        /// Target-selection mode.
        enum class mode : int {
            scale = 0, // snap to the key/scale note mask
            midi       // snap to the nearest held MIDI note; none held = no correction
        };

        /// The full monophonic corrector: detector -> mapper -> transposer.
        class corrector {
          public:
            // -- lifecycle -------------------------------------------------------------------------------

            /// Allocate for the sample rate. Call before processing; resets all running state.
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;

                const size_t tau_min = std::max<size_t>(2, static_cast<size_t>(m_sr / k_ceil_freq_hz));
                const size_t tau_max = static_cast<size_t>(std::ceil(m_sr / k_floor_freq_hz));
                m_detector.emplace(tau_max, tau_min, tau_max); // window = tau_max, the paper's W
                m_detector->set_threshold(m_threshold);

                m_frame.assign(m_detector->frame_size(), 0.0);
                m_ring.assign(m_detector->frame_size(), 0.0);
                m_ring_write = 0;

                m_hop       = std::max(64, static_cast<int>(std::lround(k_hop_seconds * m_sr)));
                m_hop_count = 0;

                const size_t n = static_cast<size_t>(std::ceil(k_max_window_ms * 0.001 * m_sr + k_base_delay)) + 16;
                m_buffer.assign(n, 0.0);
                m_write = 0;
                m_phase = 0.0;

                m_window_samples = m_window_target = k_default_window_ms * 0.001 * m_sr;
                m_window_coeff                     = 1.0 - std::exp(-1.0 / (k_window_slew_ms * 0.001 * m_sr));

                m_applied_st  = 0.0;
                m_target_st   = 0.0;
                m_detected_hz = 0.0;
                m_target_midi = -1.0;
            }

            bool prepared() const { return m_detector.has_value(); }

            /// Zero the audio state (buffers, glides). Parameters and held notes are untouched.
            void clear() {
                std::fill(m_ring.begin(), m_ring.end(), 0.0);
                std::fill(m_frame.begin(), m_frame.end(), 0.0);
                std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
                m_ring_write  = 0;
                m_write       = 0;
                m_phase       = 0.0;
                m_hop_count   = 0;
                m_applied_st  = 0.0;
                m_target_st   = 0.0;
                m_detected_hz = 0.0;
                m_target_midi = -1.0;
                if (prepared()) {
                    m_window_samples = m_window_target = k_default_window_ms * 0.001 * m_sr;
                }
            }

            // -- parameters (allocation-free; safe while audio runs) -------------------------------------

            /// Key root as a pitch class, 0 = C .. 11 = B. Re-derives the note mask from key + scale,
            /// discarding any individual note_enable() edits.
            void set_key(int pitch_class) {
                m_key = ((pitch_class % 12) + 12) % 12;
                rebuild_notes();
            }

            /// 12-bit scale mask relative to the key (bit 0 = root; see the k_scale_* presets).
            /// Re-derives the note mask, discarding any individual note_enable() edits.
            void set_scale(unsigned mask) {
                m_scale = mask & 0xFFFu;
                rebuild_notes();
            }

            /// Set the effective note mask directly, as ABSOLUTE pitch classes (bit 0 = C .. bit 11 = B).
            void set_notes(unsigned absolute_mask) { m_notes = absolute_mask & 0xFFFu; }

            /// Toggle one absolute pitch class (0 = C .. 11 = B) in the effective note mask.
            void note_enable(int pitch_class, bool enabled) {
                const unsigned bit = 1u << (((pitch_class % 12) + 12) % 12);
                if (enabled) {
                    m_notes |= bit;
                }
                else {
                    m_notes &= ~bit;
                }
            }

            void set_mode(mode m) { m_mode = m; }

            /// Retune speed: the exponential time constant (ms) of the glide onto the target.
            /// 0 snaps instantly — the hard quantize effect.
            void set_speed(double ms) { m_speed_ms = std::max(0.0, ms); }

            /// Correction depth, 0..100%. 100 lands on the target; 50 corrects half the distance.
            void set_amount(double pct) { m_amount = std::clamp(pct, 0.0, 100.0) * 0.01; }

            /// Detection range filter, Hz. Estimates outside it are treated as unpitched.
            void set_range(double min_hz, double max_hz) {
                m_min_hz = std::clamp(min_hz, k_floor_freq_hz, k_ceil_freq_hz);
                m_max_hz = std::clamp(max_hz, m_min_hz, k_ceil_freq_hz);
            }

            /// YIN voicing threshold (see tap::dsp::basic_yin) — lower is stricter.
            void set_threshold(double t) {
                m_threshold = std::clamp(t, 0.001, 1.0);
                if (prepared()) {
                    m_detector->set_threshold(m_threshold);
                }
            }

            // -- MIDI target mode ------------------------------------------------------------------------

            void note_on(int note) {
                if (note >= 0 && note < 128) {
                    m_held[static_cast<size_t>(note)] = true;
                }
            }

            void note_off(int note) {
                if (note >= 0 && note < 128) {
                    m_held[static_cast<size_t>(note)] = false;
                }
            }

            void notes_off() { m_held.fill(false); }

            // -- introspection (for meters / outlets / tests) --------------------------------------------

            int      key() const { return m_key; }
            unsigned scale() const { return m_scale; }
            unsigned notes() const { return m_notes; }
            mode     target_mode() const { return m_mode; }
            double   speed() const { return m_speed_ms; }
            double   amount() const { return m_amount * 100.0; }
            double   threshold() const { return m_threshold; }

            /// Last detected fundamental, Hz; 0 while unpitched.
            double detected_hz() const { return m_detected_hz; }

            /// Current target note as (fractional) MIDI; -1 while there is no target.
            double target_midi() const { return m_target_midi; }

            /// Correction currently applied, semitones (the slewed value the ratio follows).
            double applied_semitones() const { return m_applied_st; }

            // -- audio -----------------------------------------------------------------------------------

            double process(double in) {
                if (!prepared()) {
                    return in;
                }

                // feed the detector ring; analyze every hop
                m_ring[m_ring_write] = in;
                if (++m_ring_write >= m_ring.size()) {
                    m_ring_write = 0;
                }
                if (++m_hop_count >= m_hop) {
                    m_hop_count = 0;
                    analyze();
                }

                // glide the applied correction onto its target (0 ms = snap)
                if (m_speed_ms <= 0.0) {
                    m_applied_st = m_target_st;
                }
                else {
                    const double a = 1.0 - std::exp(-1.0 / (m_speed_ms * 0.001 * m_sr));
                    m_applied_st += (m_target_st - m_applied_st) * a;
                }
                m_window_samples += (m_window_target - m_window_samples) * m_window_coeff;

                // two-tap transposer, window locked to the detected period (tap.shift~ engine)
                m_buffer[m_write] = in;

                const double ph_a = m_phase;
                double       ph_b = ph_a + 0.5;
                if (ph_b >= 1.0) {
                    ph_b -= 1.0;
                }
                const double ea = envelope(ph_a);
                const double eb = 1.0 - ea; // exact complement: sin^2 + cos^2

                double y = 0.0;
                if (ea > 0.0) {
                    y += ea * read_hermite(k_base_delay + m_window_samples * ph_a);
                }
                if (eb > 0.0) {
                    y += eb * read_hermite(k_base_delay + m_window_samples * ph_b);
                }

                const double ratio = std::exp2(m_applied_st / 12.0);
                m_phase += -(ratio - 1.0) / m_window_samples;
                m_phase -= std::floor(m_phase);

                if (++m_write >= m_buffer.size()) {
                    m_write = 0;
                }
                return y;
            }

          private:
            void rebuild_notes() {
                unsigned absolute = 0u;
                for (int d = 0; d < 12; ++d) {
                    if ((m_scale >> d) & 1u) {
                        absolute |= 1u << ((d + m_key) % 12);
                    }
                }
                m_notes = absolute;
            }

            /// One detection pass over the last frame_size() input samples, then retarget.
            void analyze() {
                const size_t n = m_ring.size();
                for (size_t i = 0; i < n; ++i) {
                    m_frame[i] = m_ring[(m_ring_write + i) % n]; // oldest first
                }
                const auto r = m_detector->analyze(m_frame.data());

                double hz = 0.0;
                if (r.voiced()) {
                    const double f = m_sr / r.period;
                    if (f >= m_min_hz && f <= m_max_hz) {
                        hz = f;
                    }
                }
                m_detected_hz = hz;

                if (hz <= 0.0) {
                    m_target_st   = 0.0; // unpitched: relax toward no correction, hold the window
                    m_target_midi = -1.0;
                    return;
                }

                const double detected = 69.0 + 12.0 * std::log2(hz / 440.0);
                const double target   = select_target(detected);
                m_target_midi         = target;
                if (target < 0.0) {
                    m_target_st = 0.0;
                }
                else {
                    m_target_st = std::clamp(target - detected, -k_max_correction_st, k_max_correction_st) * m_amount;
                }

                // Window = an EVEN multiple of the period, so the two taps (window/2 apart) always sit
                // an integer number of periods apart — the coherence that makes the average ratio exact.
                // Clamping to a fixed ms instead measurably biases the pitch (~5 cents at 452 Hz).
                const double period_samples = m_sr / hz;
                const double two_periods    = k_window_periods * period_samples;
                const double min_w          = k_min_window_ms * 0.001 * m_sr;
                const double max_w          = k_max_window_ms * 0.001 * m_sr;
                const double multiple       = std::max(1.0, std::ceil(min_w / two_periods));
                m_window_target             = std::min(multiple * two_periods, max_w);
            }

            /// Nearest allowed note (as MIDI) for a detected MIDI pitch, or -1 when nothing is allowed.
            double select_target(double detected) const {
                if (m_mode == mode::midi) {
                    double best      = -1.0;
                    double best_dist = 1.0e9;
                    for (int n = 0; n < 128; ++n) {
                        if (m_held[static_cast<size_t>(n)]) {
                            const double dist = std::abs(detected - static_cast<double>(n));
                            if (dist < best_dist) {
                                best_dist = dist;
                                best      = static_cast<double>(n);
                            }
                        }
                    }
                    return best;
                }

                if (m_notes == 0u) {
                    return -1.0;
                }
                const int center    = static_cast<int>(std::lround(detected));
                double    best      = -1.0;
                double    best_dist = 1.0e9;
                for (int off = -6; off <= 6; ++off) { // any non-empty mask has a note within a tritone
                    const int      n  = center + off;
                    const unsigned pc = static_cast<unsigned>(((n % 12) + 12) % 12);
                    if ((m_notes >> pc) & 1u) {
                        const double dist = std::abs(detected - static_cast<double>(n));
                        if (dist < best_dist) {
                            best_dist = dist;
                            best      = static_cast<double>(n);
                        }
                    }
                }
                return best;
            }

            /// Grain envelope: sin^2 rise/fall over the half cycle — the two taps' envelopes sum to 1.
            static double envelope(double ph) {
                const double s = std::sin(k_pi * ph);
                return s * s;
            }

            size_t wrap(long i) const {
                const long n = static_cast<long>(m_buffer.size());
                return static_cast<size_t>(((i % n) + n) % n);
            }

            double read_hermite(double d) const {
                const double pos  = static_cast<double>(m_write) - d;
                const double fpos = std::floor(pos);
                const double frac = pos - fpos;
                const long   base = static_cast<long>(fpos);
                const double xm1  = m_buffer[wrap(base - 1)];
                const double x0   = m_buffer[wrap(base)];
                const double x1   = m_buffer[wrap(base + 1)];
                const double x2   = m_buffer[wrap(base + 2)];
                const double c    = (x1 - xm1) * 0.5;
                const double v    = x0 - x1;
                const double w    = c + v;
                const double a    = w + v + (x2 - x0) * 0.5;
                const double b    = w + a;
                return (((a * frac - b) * frac + c) * frac + x0);
            }

            // configuration
            double   m_sr{48000.0};
            int      m_key{0};
            unsigned m_scale{k_scale_chromatic};
            unsigned m_notes{k_scale_chromatic};
            mode     m_mode{mode::scale};
            double   m_speed_ms{k_default_speed_ms};
            double   m_amount{1.0};
            double   m_min_hz{k_default_min_hz};
            double   m_max_hz{k_default_max_hz};
            double   m_threshold{tap::dsp::yin::k_default_threshold};

            // detection
            std::optional<tap::dsp::yin> m_detector;
            std::vector<double>          m_ring;
            std::vector<double>          m_frame;
            size_t                       m_ring_write{0};
            int                          m_hop{256};
            int                          m_hop_count{0};
            std::array<bool, 128>        m_held{};

            // correction state
            double m_detected_hz{0.0};
            double m_target_midi{-1.0};
            double m_target_st{0.0};
            double m_applied_st{0.0};

            // resynthesis
            std::vector<double> m_buffer;
            size_t              m_write{0};
            double              m_phase{0.0};
            double              m_window_samples{0.0};
            double              m_window_target{0.0};
            double              m_window_coeff{0.0};
        };

    } // namespace tune
} // namespace tap::tools
