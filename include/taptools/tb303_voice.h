/// @file
/// @brief      Portable TB-303-style acid-bass voice kernel for tap.303~ — no Max/Min dependency.
/// @details    The complete monophonic voice around the diode-ladder filter: oscillator ->
///             diode ladder -> VCA, with the envelope generators and gate/slide logic that make
///             the instrument. Composition of two existing kernels plus the voice circuits:
///
///             - Oscillator: vco.h (`taptools::vco::vco_osc`), driven per sample through
///               `process_at()` so pitch (note + tuning + slide) is signal-rate. `waveform`
///               snaps saw <-> square; the shape rides vco.h's ramps, so switching is
///               click-free. The 303's square is derived from the saw by a transistor shaper
///               and is not an ideal 50% pulse — the polyBLEP pulse here is the slice-2
///               baseline, flagged for a measured-shaper refinement (cf. Open303's tables).
///             - Filter: diode_ladder.h (`taptools::diode::diode_filter`), cutoff overridden
///               per sample with the envelope-modulated value; `resonance`/`solver`/
///               `oversample` pass through. Input/output coupling: a 44.486 Hz one-pole
///               high-pass before the filter and a 24.167 Hz one-pole high-pass after it —
///               Open303's measured values for the 303's coupling network.
///             - Main Envelope Generator (MEG): decay-only RC discharge — ~3 ms rise, then an
///               exponential decay set by `decay` (200..2000 ms, the stock knob range; Open303/
///               Devil Fish). On an ACCENTED note the decay pot is bypassed and the MEG runs at
///               ~200 ms (the hardware shorts it — Devil Fish, Open303's accentDecay). The MEG
///               reaches the cutoff as
///                   fc_eff = cutoff * 2^(envmod * k_envmod_octaves * (meg - (1 - k_env_up)))
///               with k_env_up = 2/3 (Open303's envUpFraction): 2/3 of the sweep goes above the
///               knob and 1/3 below — the "gimmick" offset, so raising envmod also lowers the
///               resting point. k_envmod_octaves = 4 is an informed approximation pending the
///               slice-4 render calibration against Open303.
///             - VCA envelope: fast (~3 ms) attack, fixed ~1230 ms exponential decay to zero
///               (no sustain — Open303's measured ampEnv), cut at gate-off by a ~2 ms release
///               (the hardware chops; 2 ms keeps it click-free).
///             - Gate/slide (the melodic-voice contract): a (re)trigger snaps the pitch and
///               fires both envelopes. A pitch change WHILE THE GATE IS HELD is a slide —
///               legato: no retrigger, and the pitch glides through the hardware's fixed
///               ~60 ms RC lag (Open303's slideTime). Note: slide is pulled forward from
///               plan slice 3 because the note contract requires legato semantics from day one.
///             - Accent, slice-2 scope: an accented trigger (depth 0..1, scaled by the `accent`
///               knob) shortens the MEG as above and boosts the VCA level (up to 2x at full
///               knob and depth). The C13 accent-sweep capacitor (the across-notes "wow") and
///               its resonance-pot scaling are DELIBERATELY NOT HERE YET — they are plan
///               slice 3, the coupling that finishes the machine.
///
///             As in the other TapTools kernels: per-sample linear ramps on every parameter, a
///             16-slot preset-morph engine, allocation-free after prepare(), setters safe while
///             audio runs. Sources: Open303 (Robin Schmidt) for the measured constants, the
///             Devil Fish documentation (Robin Whittle) for the envelope/accent circuit
///             behavior, Stinchcombe for the filter (see diode_ladder.h).
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

#include "diode_ladder.h"
#include "vco.h"

namespace taptools {
    namespace tb303 {

        constexpr int    k_presets           = 16;
        constexpr double k_tuning_range      = 1200.0; // cents
        constexpr double k_cutoff_min        = 100.0;  // stock knob is ~302..2394 Hz (Open303, measured);
        constexpr double k_cutoff_max        = 5000.0; // the wider range is the Devil-Fish-style bend
        constexpr double k_decay_min_ms      = 200.0;  // MEG decay pot range (Open303 / Devil Fish)
        constexpr double k_decay_max_ms      = 2000.0;
        constexpr double k_meg_attack_ms     = 3.0;       // Open303 normalAttack
        constexpr double k_meg_accent_ms     = 200.0;     // accent bypasses the decay pot (Open303 accentDecay)
        constexpr double k_vca_attack_ms     = 3.0;       // ~3 ms stock attack (Devil Fish)
        constexpr double k_vca_decay_ms      = 1230.0;    // Open303 measured ampEnv decay, sustain 0
        constexpr double k_vca_release_ms    = 2.0;       // gate-off chop (Open303 ~1 ms; 2 ms is click-free)
        constexpr double k_env_up            = 2.0 / 3.0; // Open303 envUpFraction
        constexpr double k_envmod_octaves    = 4.0;       // full-envmod sweep span (informed approx., see header)
        constexpr double k_slide_ms          = 60.0;      // pitch CV RC lag (Open303 slideTime)
        constexpr double k_pre_hpf_hz        = 44.486;    // pre-filter coupling high-pass (Open303)
        constexpr double k_post_hpf_hz       = 24.167;    // post-filter coupling high-pass (Open303)
        constexpr double k_default_smooth_ms = 20.0;
        constexpr double k_pi                = 3.14159265358979323846;

        enum param_index : int {
            p_gain = 0,  // output gain, dB
            p_tuning,    // master tune, cents
            p_cutoff,    // VCF cutoff knob, Hz
            p_resonance, // 0..1.5 (diode_ladder semantics: 1.0 = stock max)
            p_envmod,    // 0..1 MEG -> cutoff depth
            p_decay,     // MEG decay, ms
            p_accent,    // 0..1 accent amount (the AC knob)
            k_num_params
        };

        enum waveform : int { wave_saw = 0, wave_square, k_num_waveforms };

        /// One full parameter snapshot — a preset slot, and the unit the morph engine interpolates.
        struct params {
            std::array<double, k_num_params> v{};

            static params defaults() {
                params p;
                p.v[p_gain]      = 0.0;
                p.v[p_tuning]    = 0.0;
                p.v[p_cutoff]    = 1000.0;
                p.v[p_resonance] = 0.5;
                p.v[p_envmod]    = 0.5;
                p.v[p_decay]     = 400.0;
                p.v[p_accent]    = 0.5;
                return p;
            }
        };

        /// Clamp a value to the legal range of a parameter. Gain (dB) is unclamped.
        inline double clamp_param(int index, double value) {
            switch (index) {
            case p_gain:
                return value;
            case p_tuning:
                return std::clamp(value, -k_tuning_range, k_tuning_range);
            case p_cutoff:
                return std::clamp(value, k_cutoff_min, k_cutoff_max);
            case p_resonance:
                return std::clamp(value, 0.0, diode::k_res_max);
            case p_envmod:
                return std::clamp(value, 0.0, 1.0);
            case p_decay:
                return std::clamp(value, k_decay_min_ms, k_decay_max_ms);
            case p_accent:
                return std::clamp(value, 0.0, 1.0);
            default:
                return value;
            }
        }

        class voice {
          public:
            voice() {
                const params d = params::defaults();
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = d.v[i];
                }
                m_presets.fill(d);
            }

            // -- lifecycle -------------------------------------------------------------------------------

            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_osc.prepare(m_sr);
                m_osc.set_smooth_ms(m_smooth_ms);
                m_osc.set_waveform(vco::wave_saw);
                m_osc.snap();
                m_filter.prepare(m_sr);
                m_filter.set_smooth_ms(0.0); // the voice owns all smoothing
                m_meg_attack  = attack_coef(k_meg_attack_ms);
                m_vca_attack  = attack_coef(k_vca_attack_ms);
                m_vca_decay   = decay_coef(k_vca_decay_ms);
                m_vca_release = decay_coef(k_vca_release_ms);
                m_slide_coef  = one_pole_coef(k_slide_ms);
                m_pre_hp.set(k_pre_hpf_hz, m_sr);
                m_post_hp.set(k_post_hpf_hz, m_sr);
                clear();
                snap();
                m_prepared = true;
            }

            /// Silence the voice and zero all DSP state; parameters untouched.
            void clear() {
                m_gate = false;
                m_meg = m_vca = 0.0;
                m_meg_rising = m_vca_rising = false;
                m_note_accent               = 0.0;
                m_pitch                     = m_pitch_target;
                m_osc.clear();
                m_filter.clear();
                m_pre_hp.reset();
                m_post_hp.reset();
            }

            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = 0.0;
                    r.remaining = 0;
                }
                m_ramps_active = 0;
                push_filter_params();
            }

            // -- the note interface (the package-wide melodic-voice contract) -------------------------------

            /// (Re)trigger: snap the pitch, fire both envelopes. `accent_depth` 0..1 (0 = plain note;
            /// the wrapper maps gate amplitude 1..2 onto it). If the gate is already held this is a
            /// SLIDE instead: the pitch glides (~60 ms RC), nothing retriggers.
            void note_on(double midi_note, double accent_depth = 0.0) {
                m_pitch_target = midi_note;
                if (!m_gate) {
                    m_pitch       = midi_note; // non-legato: the step happens while the gate is low
                    m_note_accent = std::clamp(accent_depth, 0.0, 1.0);
                    m_meg_rising = m_vca_rising = true;
                    m_gate                      = true;
                }
            }

            /// Change the pitch target without touching the gate — while held, this is the slide.
            void set_pitch(double midi_note) { m_pitch_target = midi_note; }

            void note_off() { m_gate = false; }
            bool gate() const { return m_gate; }

            // -- modes (not part of the morphable parameter set) --------------------------------------------

            void set_waveform(int w) {
                m_wave = std::clamp(w, 0, k_num_waveforms - 1);
                m_osc.set_waveform(m_wave == wave_saw ? vco::wave_saw : vco::wave_pulse);
            }
            int wave() const { return m_wave; }

            void set_solver(int s) { m_filter.set_solver(s); }
            int  solver() const { return m_filter.solver(); }

            void set_oversample(int os) { m_filter.set_oversample(os); }
            int  oversample() const { return m_filter.oversample(); }

            void set_smooth_ms(double ms) {
                m_smooth_ms = std::max(0.0, ms);
                m_osc.set_smooth_ms(m_smooth_ms);
            }
            double smooth_ms() const { return m_smooth_ms; }

            // -- parameter targets (click-free; safe while audio runs) --------------------------------------

            void set_param(int index, double value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                ramp_to(index, clamp_param(index, value), static_cast<long>(m_smooth_ms * 0.001 * m_sr));
            }

            void set_gain(double db) { set_param(p_gain, db); }
            void set_tuning(double cents) { set_param(p_tuning, cents); }
            void set_cutoff(double hz) { set_param(p_cutoff, hz); }
            void set_resonance(double r) { set_param(p_resonance, r); }
            void set_envmod(double e) { set_param(p_envmod, e); }
            void set_decay(double ms) { set_param(p_decay, ms); }
            void set_accent(double a) { set_param(p_accent, a); }

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

            /// One output sample.
            double process() {
                tick_ramps();

                // pitch: snap on retrigger, RC glide while sliding
                m_pitch += (m_pitch_target - m_pitch) * m_slide_coef;
                const double note = m_pitch + m_ramp[p_tuning].current * 0.01;
                const double hz   = 440.0 * std::exp2((note - 69.0) / 12.0);

                // envelopes
                const double meg_decay_ms = (m_note_accent > 0.0) ? k_meg_accent_ms : m_ramp[p_decay].current;
                step_env(m_meg, m_meg_rising, m_meg_attack, decay_coef(meg_decay_ms));
                step_env(m_vca, m_vca_rising, m_vca_attack, m_gate ? m_vca_decay : m_vca_release);

                // envelope-modulated cutoff: 2/3 of the sweep above the knob, 1/3 below (the gimmick)
                const double sweep  = m_ramp[p_envmod].current * k_envmod_octaves;
                const double fc_eff = m_ramp[p_cutoff].current * std::exp2(sweep * (m_meg - (1.0 - k_env_up)));

                // oscillator -> coupling HPF -> diode ladder -> coupling HPF -> VCA
                const double osc      = m_osc.process_at(hz);
                const double filtered = m_filter.process(m_pre_hp.tick(osc), fc_eff);
                const double shaped   = m_post_hp.tick(filtered);

                const double accent_boost = 1.0 + m_ramp[p_accent].current * m_note_accent;
                return shaped * m_vca * accent_boost * m_out_gain;
            }

            void process(double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process();
                }
            }

          private:
            struct ramp {
                double current{0.0};
                double target{0.0};
                double inc{0.0};
                long   remaining{0};
            };

            // One-pole high-pass (x minus its one-pole lowpass) for the coupling networks.
            struct hp1 {
                double a{0.0};
                double lp{0.0};
                void   set(double hz, double sr) { a = 1.0 - std::exp(-2.0 * k_pi * hz / sr); }
                double tick(double x) {
                    lp += (x - lp) * a;
                    lp = (std::abs(lp) < 1e-15) ? 0.0 : lp;
                    return x - lp;
                }
                void reset() { lp = 0.0; }
            };

            static bool valid_slot(int s) { return s >= 0 && s < k_presets; }

            // True RC lag: tau = ms (the slide circuit's time constant).
            double one_pole_coef(double ms) const { return 1.0 - std::exp(-1000.0 / (std::max(0.01, ms) * m_sr)); }
            // Attack: tau = ms/3, so the envelope reaches ~95% of full in roughly the stated time.
            double attack_coef(double ms) const { return 1.0 - std::exp(-3000.0 / (std::max(0.01, ms) * m_sr)); }
            double decay_coef(double ms) const { return std::exp(-1000.0 / (std::max(0.01, ms) * m_sr)); }

            // Attack: one-pole rise toward 1; past the knee, exponential decay toward 0.
            static void step_env(double& e, bool& rising, double attack_coef, double decay_mult) {
                if (rising) {
                    e += (1.0 - e) * attack_coef;
                    if (e >= 0.999) {
                        e      = 1.0;
                        rising = false;
                    }
                }
                else {
                    e *= decay_mult;
                    e = (e < 1e-15) ? 0.0 : e;
                }
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
                push_filter_params();
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
                push_filter_params();
            }

            // The filter's own smoothing is off; the voice's ramps feed it directly.
            void push_filter_params() {
                m_filter.set_param(diode::p_resonance, m_ramp[p_resonance].current);
                m_out_gain = std::pow(10.0, m_ramp[p_gain].current * 0.05);
            }

            // configuration
            double m_sr{48000.0};
            double m_smooth_ms{k_default_smooth_ms};
            int    m_wave{wave_saw};
            bool   m_prepared{false};

            // parameters
            std::array<ramp, k_num_params> m_ramp;
            std::array<params, k_presets>  m_presets;
            int                            m_ramps_active{0};
            double                         m_out_gain{1.0};

            // note state
            bool   m_gate{false};
            double m_pitch{45.0}; // A1 — an acid-appropriate resting pitch
            double m_pitch_target{45.0};
            double m_note_accent{0.0};

            // envelope state
            double m_meg{0.0}, m_vca{0.0};
            bool   m_meg_rising{false}, m_vca_rising{false};
            double m_meg_attack{0.05}, m_vca_attack{0.05};
            double m_vca_decay{0.9999}, m_vca_release{0.99};
            double m_slide_coef{0.001};

            // DSP blocks
            vco::vco_osc        m_osc;
            diode::diode_filter m_filter;
            hp1                 m_pre_hp, m_post_hp;
        };

    } // namespace tb303
} // namespace taptools
