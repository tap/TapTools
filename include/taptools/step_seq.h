/// @file
/// @brief      The shared step-sequencer engine behind tap.808.seq~ and tap.303.seq~.
/// @details    One phase-clocked step engine plus two thin emitters — the design of record is
///             plans/tap.seq.md in the Max package repo (author-approved 2026-07-18), resolving
///             the phase-3 sections of the tap.808 and tap.303 plans with one shared kernel.
///
///             The clocking idiom (the load-bearing decision): the caller feeds a phase ramp,
///             0..1 per pattern cycle (phasor~ in Max). The engine derives the current step as
///             floor(phase * length) with swing warping the odd-step boundaries, and reports
///             sample-accurate step entries. The engine is stateless against phase — no
///             run/stop of its own; jump, scrub, or reverse the phase and the step derivation
///             follows. Rows driven from one phasor stay phase-coherent forever, and different
///             `length` values off the same ramp are polymeter (which subsumes the TR-808's
///             triplet pre-scale).
///
///             The two emitters speak the shipped voice contracts verbatim:
///             - `trigger_row` (tap.808.seq~): impulses at step starts whose amplitude is the
///               step's velocity 0..1 — the tap.808.* accent bus (amplitude maps onto the
///               hardware's 4-14 V common trigger bus; TR-808 Service Notes, and see
///               tr808_kick.h k_bd_vtrig_min/max). Pinned levels: k_trig_plain = 0.01, the
///               un-accented 4 V base trigger (accent ~= 0, held just above the voices' 1e-3
///               edge threshold so the edge always registers); k_trig_accented = 0.5, the
///               accent level VR3 at noon (1.0 = the full 14 V bus).
///             - `note_row` (tap.303.seq~): a pitch output (MIDI note number, held between
///               notes) and a gate output at 1.0 plain / 2.0 accented (the tap.303~ inlet
///               contract: accent depth = amplitude - 1). The gate opens at the step start and
///               closes at k_gate_duty = 0.5 of the step — Open303's AcidPattern
///               (stepLength = 0.5, "the time while gate is open ... in units of one step"),
///               the same reference the voice's calibrated constants came from. A step whose
///               `slide` flag is set is approached legato: the gate does NOT fall during the
///               step before it — it holds through the boundary while the pitch steps, and the
///               voice's legato detection does the ~60 ms glide without retriggering. (The
///               hardware stores the flag on the source note, "slide to next"; the package
///               convention puts it on the target step, matching tap.303~'s
///               `note <pitch> [accent] [slide]` message — the data models convert trivially.)
///               An accented slid step keeps the held gate level: the voice samples accent at
///               the rising edge only (the shipped contract), so accent does not re-arm
///               without a retrigger.
///
///             Swing: 0..1, delaying each odd-numbered step's start by up to half a step
///             (0 = straight, 2/3 = the classic triplet shuffle: the off-16th lands at 1/3 of
///             the pair). Beyond-hardware (neither machine had swing), so it defaults to 0 —
///             the house "documented bends, stock defaults" posture. Gate duty is measured
///             against the swung span, so gates never collide.
///
///             Patterns: up to 64 steps (default 16), 16 storage slots with store/recall;
///             recall arms and applies at the next cycle boundary by default (quantize
///             cycle|step|now) — which is the TR-808's A/B-half and basic/fill switching as
///             one message. No randomness anywhere: bit-exact by construction, and pinned by
///             test anyway.
///
///             Plain C++17, stdlib only, header-only, allocation-free, no Max/Min dependency.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <cmath>

namespace taptools {
    namespace seq {

        constexpr int k_max_steps      = 64;
        constexpr int k_num_slots      = 16;
        constexpr int k_default_length = 16;

        // The 303 gate opens for half the step (Open303 AcidPattern stepLength = 0.5).
        constexpr double k_gate_duty = 0.5;

        // tap.808.* trigger amplitudes (the 4-14 V bus mapped to 0..1, see header note).
        constexpr double k_trig_plain    = 0.01;
        constexpr double k_trig_accented = 0.5;

        // tap.303~ gate amplitudes (accent depth = amplitude - 1).
        constexpr double k_gate_plain  = 1.0;
        constexpr double k_gate_accent = 2.0;

        enum quantize_mode {
            quantize_cycle = 0, // armed recall applies when step 0 is entered (default)
            quantize_step  = 1, // ... when any step is entered
            quantize_now   = 2  // ... immediately
        };

        /// One step of the union payload: `velocity` serves trigger rows (0 = rest);
        /// `pitch`/`gate`/`accent`/`slide` serve note rows.
        struct step {
            double velocity{0.0};
            double pitch{45.0}; // A2, the 303's home note
            bool   gate{false};
            bool   accent{false};
            bool   slide{false};
        };

        struct pattern {
            int  length{k_default_length};
            step steps[k_max_steps];

            void set_length(int n) { length = std::clamp(n, 1, k_max_steps); }

            void clear() {
                for (auto& s : steps)
                    s = step{};
            }
        };

        /// The phase->step engine: boundary detection with swing warp, plus the slot store.
        /// Feed `process()` one phase sample (any real number; wrapped into [0,1)) and read
        /// back which step the sample is in, whether it just entered it, and how far through
        /// the step's actual (swung) span it is.
        class engine {
          public:
            struct tick {
                bool   entered{false};
                int    index{0};
                double pos{0.0}; // position within the step's swung span, 0..1
            };

            pattern&       data() { return m_pattern; }
            const pattern& data() const { return m_pattern; }

            void   set_swing(double s) { m_swing = std::clamp(s, 0.0, 1.0); }
            double swing() const { return m_swing; }

            void set_quantize(int m) { m_quantize = std::clamp(m, 0, 2); }
            int  quantize() const { return m_quantize; }

            void store(int slot) {
                if (slot >= 0 && slot < k_num_slots)
                    m_slots[slot] = m_pattern;
            }

            /// Arm (or, with quantize_now, immediately apply) a stored pattern.
            void recall(int slot) {
                if (slot < 0 || slot >= k_num_slots)
                    return;
                if (m_quantize == quantize_now) {
                    m_pattern = m_slots[slot];
                    m_armed   = -1;
                }
                else {
                    m_armed = slot;
                }
            }

            int armed() const { return m_armed; }

            /// Forget the previous step: the next processed sample re-enters whatever step
            /// its phase lands in (so a transport start fires its downbeat).
            void reset() { m_prev = -1; }

            /// The step the engine is currently in (-1 before any processing) — for UI feedback.
            int current_step() const { return m_prev; }

            tick process(double phase) {
                int k = derive(phase);

                tick t;
                t.entered = (k != m_prev);

                // An armed recall applies on the quantize boundary; the step grid may change
                // with the new pattern, so re-derive this sample against it.
                if (t.entered && m_armed >= 0 && (m_quantize == quantize_step || k == 0)) {
                    m_pattern = m_slots[m_armed];
                    m_armed   = -1;
                    k         = derive(phase);
                }

                m_prev  = k;
                t.index = k;
                t.pos   = position(phase, k);
                return t;
            }

          private:
            double wrap(double p) const {
                p -= std::floor(p);
                return (p < 0.0 || p >= 1.0) ? 0.0 : p; // guard float edge cases
            }

            /// Swung start of step k, in phase units. Odd steps start late by swing/2 steps.
            double start(int k) const {
                const int    length = m_pattern.length;
                const double late   = (k & 1) ? m_swing * 0.5 : 0.0;
                return (static_cast<double>(k) + late) / static_cast<double>(length);
            }

            int derive(double phase) const {
                const int    length = m_pattern.length;
                const double u      = wrap(phase) * static_cast<double>(length);
                int          k      = std::min(static_cast<int>(u), length - 1);
                // An odd step's start is delayed: its first swing/2 belongs to the step before.
                if ((k & 1) && (u - static_cast<double>(k)) < m_swing * 0.5)
                    --k;
                return k;
            }

            double position(double phase, int k) const {
                const double b0   = start(k);
                const double b1   = (k + 1 < m_pattern.length) ? start(k + 1) : 1.0;
                const double span = b1 - b0;
                if (span <= 0.0)
                    return 0.0;
                return std::clamp((wrap(phase) - b0) / span, 0.0, 1.0);
            }

            pattern m_pattern;
            pattern m_slots[k_num_slots];
            double  m_swing{0.0};
            int     m_quantize{quantize_cycle};
            int     m_armed{-1};
            int     m_prev{-1};
        };

        /// The tap.808.seq~ emitter: one drum row. Impulses at step starts, amplitude =
        /// the step's velocity (the family's amplitude-as-accent trigger contract). With
        /// `pulse_ms` above 0 the impulse widens into a held gate for envelope consumers —
        /// keep it shorter than a step at your clock rate, or back-to-back steps merge and
        /// downstream edge detectors (which re-arm below 1e-3) will miss the second edge.
        class trigger_row {
          public:
            void prepare(double sample_rate) {
                m_sr = std::max(sample_rate, 1.0);
                set_pulse_ms(m_pulse_ms);
                reset();
            }

            engine&       clock() { return m_engine; }
            const engine& clock() const { return m_engine; }

            void set_pulse_ms(double ms) {
                m_pulse_ms      = std::max(ms, 0.0);
                m_pulse_samples = std::max(1, static_cast<int>(std::lround(m_pulse_ms * 0.001 * m_sr)));
            }

            void reset() {
                m_engine.reset();
                m_hold  = 0;
                m_level = 0.0;
            }

            double process(double phase) {
                const engine::tick t = m_engine.process(phase);
                if (t.entered) {
                    const step& st = m_engine.data().steps[t.index];
                    if (st.velocity > 0.0) {
                        m_level = std::clamp(st.velocity, 0.0, 1.0);
                        m_hold  = m_pulse_samples;
                    }
                }
                if (m_hold > 0) {
                    --m_hold;
                    return m_level;
                }
                return 0.0;
            }

          private:
            engine m_engine;
            double m_sr{48000.0};
            double m_pulse_ms{0.0};
            int    m_pulse_samples{1};
            int    m_hold{0};
            double m_level{0.0};
        };

        /// The tap.303.seq~ emitter: one bass line. Emits the tap.303~ inlet pair — pitch
        /// (MIDI note number, held between notes) and gate (0 / 1.0 plain / 2.0 accented),
        /// with gate-hold slide (see the header note for the full semantics).
        class note_row {
          public:
            struct out {
                double pitch{45.0};
                double gate{0.0};
            };

            void prepare(double) { reset(); } // nothing rate-dependent; kept for house shape

            engine&       clock() { return m_engine; }
            const engine& clock() const { return m_engine; }

            void   set_transpose(double semitones) { m_transpose = semitones; }
            double transpose() const { return m_transpose; }

            void reset() {
                m_engine.reset();
                m_gate = 0.0;
            }

            out process(double phase) {
                const engine::tick t  = m_engine.process(phase);
                const pattern&     pn = m_engine.data();
                const step&        st = pn.steps[t.index];

                if (t.entered) {
                    if (st.gate) {
                        m_pitch = st.pitch + m_transpose;
                        // Legato only if a note is sounding to slide from; a slide flag on a
                        // step after a rest is a plain trigger (the voice does the same).
                        if (!(st.slide && m_gate > 0.0))
                            m_gate = st.accent ? k_gate_accent : k_gate_plain;
                    }
                    else {
                        m_gate = 0.0;
                    }
                }
                else if (m_gate > 0.0 && st.gate && t.pos >= k_gate_duty) {
                    // Close at the duty point — unless the next step slides in, which holds
                    // the gate through the boundary. Read live so pattern edits apply now.
                    const step& nx = pn.steps[(t.index + 1) % pn.length];
                    if (!(nx.gate && nx.slide))
                        m_gate = 0.0;
                }

                out o;
                o.pitch = m_pitch;
                o.gate  = m_gate;
                return o;
            }

          private:
            engine m_engine;
            double m_pitch{45.0};
            double m_gate{0.0};
            double m_transpose{0.0};
        };

    } // namespace seq
} // namespace taptools
