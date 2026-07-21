/// @file
/// @brief      Portable TB-303-style acid-bass voice kernel for tap.303~ — no Max/Min dependency.
/// @details    The complete monophonic voice around the diode-ladder filter: oscillator ->
///             diode ladder -> VCA, with the envelope generators and gate/slide logic that make
///             the instrument. Composition of two existing kernels plus the voice circuits:
///
///             - Oscillator: vco.h (`tap::tools::vco::vco_osc`) running the polyBLEP saw, driven
///               per sample through `process_at()` so pitch (note + tuning + slide) is
///               signal-rate. The 303's square is not a pulse — the hardware derives it from
///               the saw with a transistor shaper. Slice 4 adopts Open303's measured shaper:
///                   square = -tanh(10^(36.9/20) * saw_shifted + 4.37)
///               applied to the half-cycle-shifted saw (Open303's MipMappedWaveTable
///               constants: 36.9 dB drive, 4.37 offset, 180 deg alignment) — the biased tanh
///               gives the rounded, slightly-asymmetric square of the hardware, and applying
///               it to the BLEP-smoothed saw keeps the edges bandlimited. `waveform` is a
///               continuous 0 (saw) .. 1 (square) blend riding the ramps, like Open303's
///               blend oscillator.
///             - Filter: diode_ladder.h (`tap::tools::diode::diode_filter`), cutoff overridden
///               per sample with the envelope-modulated value; `resonance`/`solver`/
///               `oversample` pass through. Input/output coupling: a 44.486 Hz one-pole
///               high-pass before the filter and a 24.167 Hz one-pole high-pass after it —
///               Open303's measured values for the 303's coupling network.
///             - Main Envelope Generator (MEG): decay-only RC discharge — ~3 ms rise, then an
///               exponential decay set by `decay` (200..2000 ms, the stock knob range; Open303/
///               Devil Fish). On an ACCENTED note the decay pot is bypassed and the MEG runs at
///               the accent clock (`accdecay`, stock ~200 ms — Open303's accentDecay). The MEG
///               reaches the cutoff through Open303's HARDWARE-MEASURED envmod law (slice-4
///               calibration; their calculateEnvModScalerAndOffset "measured mapping"):
///                   c      = log-position of the cutoff knob in [313.8 Hz, 2394.4 Hz]
///                   scaler = (1-c)*(3.774*envmod + 0.737) + c*(4.195*envmod + 0.864)
///                   offset = 0.0483*c + 0.2944
///                   fc_eff = cutoff * 2^(scaler*(meg - offset) + accent_sweep)
///               i.e. ~4.5-5 octaves of sweep at full envmod with ~2/3 of it above the knob
///               (the "gimmick" resting-point drop), a residual ~0.8-octave sweep even at
///               envmod 0, and slightly deeper sweeps at higher knob positions — all measured
///               off hardware by the Open303 project.
///             - VCA envelope: fast attack (`attack`, stock ~3 ms; the 0.3..30 ms range is the
///               Devil Fish "Soft Attack" bend), fixed ~1230 ms exponential decay to zero
///               (no sustain — Open303's measured ampEnv), cut at gate-off by a ~2 ms release
///               (the hardware chops; 2 ms keeps it click-free).
///             - Gate/slide (the melodic-voice contract): a (re)trigger snaps the pitch and
///               fires both envelopes. A pitch change WHILE THE GATE IS HELD is a slide —
///               legato: no retrigger, and the pitch glides through the hardware's fixed
///               ~60 ms RC lag (Open303's slideTime). Note: slide is pulled forward from
///               plan slice 3 because the note contract requires legato semantics from day one.
///             - Accent: an accented trigger (depth 0..1, scaled by the `accent` knob)
///               shortens the MEG as above, boosts the VCA level (up to 2x at full knob and
///               depth), and drives the ACCENT SWEEP CIRCUIT — the C13 capacitor (slice 3,
///               the coupling that finishes the machine). Circuit (Devil Fish, "the accent
///               sweep circuit"): the accented MEG passes a diode + 47k into the resonance
///               pot's second section (100k, 1 uF to ground at the CW end), whose wiper feeds
///               a 100k mixing resistor into the cutoff summing node. Modeled:
///                 * the diode charges the cap only upward (tau ~= 47k*1uF = 47 ms), and the
///                   cap discharges through the ~100-200k output path (tau ~150 ms nominal —
///                   the across-notes memory). Closely spaced accents find residual charge,
///                   so successive sweeps peak HIGHER — the build-up ("wow"); the charge also
///                   bleeds into following plain notes as it drains, like the hardware's
///                   always-connected summing node.
///                 * the cutoff contribution combines a direct MEG term reduced by the cap
///                   voltage (Devil Fish's "~100/147 of the MEG minus the capacitor voltage"
///                   — what rounds the first accent's curve) with the cap voltage itself:
///                   sweep_oct = k_accent_sweep_oct * res_mix *
///                               (k_accent_direct*max(A*meg - c13, 0) + c13),  A = knob*depth
///                 * res_mix = 0.3 + 0.7*min(resonance, 1): the pot is ganged with resonance,
///                   so accent quacks harder at high resonance — hardware behavior.
///               The sweep injects into the cutoff sum DIRECTLY, bypassing envmod (as in the
///               circuit): accents sweep even with envmod at zero. The RC taus are
///               component-derived; k_accent_sweep_oct = 2 and the 0.4 direct weight remain
///               informed approximations — Open303 models its accent path as a plain 15 ms
///               leaky integrator with NO across-notes memory, so the C13 model here follows
///               the Devil Fish circuit description instead (deliberate divergence, noted).
///             - Devil-Fish-style bends, all stock at defaults: `slide` (the pitch-glide RC,
///               stock 60 ms), `attack` (VCA attack, stock 3 ms), `accdecay` (the accent
///               MEG clock, stock 200 ms), `drive` (dB into the diode ladder's saturation).
///             - `seed`/`tolerance`: deterministic per-unit component spread (house vco.h
///               convention) — tuning, cutoff scale, envelope times, slide and C13 RCs take
///               per-seed offsets scaled by tolerance, and the oscillator gets the seed plus
///               a proportional `imperfect` amount. tolerance 0 is the nominal schematic,
///               bit-identical to a voice with no seed set; every seed is a different unit
///               off the line, so mc. stacks decorrelate like real hardware.
///             - Factory presets: slots 1..8 ship authored (classic squelch, deep sub,
///               screamer, rubber, knock, bloom, overdriven, glass); 9..16 are the defaults.
///               `recall` morphs to them out of the box.
///             - Phase 2 — the transistor VCA (`vca` mode, svf.h two-circuit pattern):
///               `vca_clean` (default) is the linear multiply, bit-identical to phase 1.
///               `vca_warm` models the 303's one-transistor class-A VCA stage as a
///               slope-normalized biased saturator applied AFTER the envelope gain and BEFORE
///               the output coupling (the hardware order, so the coupling HPF absorbs the
///               shaper's signal-dependent DC):
///                   S(v) = (tanh(d*v + b) - tanh(b)) / (d*sech^2(b)),  d = 2.0, b = 0.3
///               Unity slope at 0: quiet notes are essentially clean (a few % harmonic content at typical
///               levels), hot accented notes pick up even harmonics (~11% h2 on a full-scale
///               sine) and audible compression (~ -4 dB at full whack) that TRACK the envelope — the discrete-VCA
///               warmth. d/b are informed constants (probe-calibrated for musical THD);
///               schematic-derived values are an audition-time refinement.
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
#include <cstdint>

#include "diode_ladder.h"
#include "vca.h"
#include "vco.h"

namespace tap::tools {
    namespace tb303 {

        constexpr int    k_presets        = 16;
        constexpr double k_tuning_range   = 1200.0; // cents
        constexpr double k_cutoff_min     = 100.0;  // stock knob is ~302..2394 Hz (Open303, measured);
        constexpr double k_cutoff_max     = 5000.0; // the wider range is the Devil-Fish-style bend
        constexpr double k_decay_min_ms   = 200.0;  // MEG decay pot range (Open303 / Devil Fish)
        constexpr double k_decay_max_ms   = 2000.0;
        constexpr double k_meg_attack_ms  = 3.0;    // Open303 normalAttack
        constexpr double k_meg_accent_ms  = 200.0;  // accent bypasses the decay pot (Open303 accentDecay)
        constexpr double k_vca_attack_ms  = 3.0;    // ~3 ms stock attack (Devil Fish)
        constexpr double k_vca_decay_ms   = 1230.0; // Open303 measured ampEnv decay, sustain 0
        constexpr double k_vca_release_ms = 2.0;    // gate-off chop (Open303 ~1 ms; 2 ms is click-free)
        // Open303's hardware-measured envmod mapping (calculateEnvModScalerAndOffset):
        constexpr double k_knob_lo_hz      = 3.138152786059267e+02; // lowest nominal cutoff (measured)
        constexpr double k_knob_hi_hz      = 2.394411986817546e+03; // highest nominal cutoff (measured)
        constexpr double k_env_offset_f    = 0.048292930943553;     // offset line: f*c + c0
        constexpr double k_env_offset_c    = 0.294391201442418;
        constexpr double k_env_scaler_lo_f = 3.773996325111173; // scaler line at low cutoff
        constexpr double k_env_scaler_lo_c = 0.736965594166206;
        constexpr double k_env_scaler_hi_f = 4.194548788411135; // scaler line at high cutoff
        constexpr double k_env_scaler_hi_c = 0.864344900642434;
        // Open303's measured 303-square shaper (MipMappedWaveTable): -tanh(10^(36.9/20)*saw + 4.37)
        constexpr double k_square_gain       = 69.98419960022734; // 10^(36.9/20)
        constexpr double k_square_bias       = 4.37;
        constexpr double k_inv_log_knob_span = 0.4921045673846009; // 1 / ln(k_knob_hi_hz / k_knob_lo_hz)
        constexpr double k_slide_ms          = 60.0;               // stock pitch CV RC lag (Open303 slideTime)
        constexpr double k_slide_min_ms      = 10.0;               // bend range (Devil Fish extends slide ~5x)
        constexpr double k_slide_max_ms      = 500.0;
        constexpr double k_attack_min_ms     = 0.3; // Devil Fish Soft Attack range
        constexpr double k_attack_max_ms     = 30.0;
        constexpr double k_accdecay_min_ms   = 50.0; // accent-MEG clock bend range (stock 200)
        constexpr double k_accdecay_max_ms   = 2000.0;
        constexpr double k_drive_range_db    = 24.0;   // into the diode ladder's saturation
        constexpr double k_c13_charge_ms     = 47.0;   // accent-sweep cap charge: 47k * 1uF (Devil Fish)
        constexpr double k_c13_discharge_ms  = 150.0;  // discharge via the ~100-200k output path, nominal
        constexpr double k_accent_sweep_oct  = 2.0;    // full-charge sweep span (informed approx., see header)
        constexpr double k_accent_direct     = 0.4;    // direct-MEG weight in the sweep sum
        constexpr double k_pre_hpf_hz        = 44.486; // pre-filter coupling high-pass (Open303)
        constexpr double k_post_hpf_hz       = 24.167; // post-filter coupling high-pass (Open303)
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
            p_waveform,  // 0 saw .. 1 square (continuous blend, like Open303)
            p_slide,     // pitch-glide RC, ms (stock 60 — bend)
            p_attack,    // VCA attack, ms (stock 3 — the Soft Attack bend)
            p_accdecay,  // accent-MEG clock, ms (stock 200 — bend)
            p_drive,     // dB into the filter's diode saturation (bend)
            k_num_params
        };

        enum waveform : int { wave_saw = 0, wave_square, k_num_waveforms };

        enum vca_mode : int { vca_clean = 0, vca_warm, k_num_vca_modes };

        // Phase-2 transistor-VCA saturator (see header): informed constants, probe-calibrated.
        // The saturator itself lives in the shared vca.h kernel (tap::tools::vca) so tap.303~ and the
        // standalone tap.vca~ run one implementation; these are the values the voice configures it with.
        constexpr double k_vca_sat_drive = tap::tools::vca::k_default_drive; // 2.0
        constexpr double k_vca_sat_bias  = tap::tools::vca::k_default_bias;  // 0.3

        constexpr int k_factory_presets = 8; // slots 0..7 ship authored; 8..15 are defaults

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
                p.v[p_waveform]  = 0.0;
                p.v[p_slide]     = k_slide_ms;
                p.v[p_attack]    = k_vca_attack_ms;
                p.v[p_accdecay]  = k_meg_accent_ms;
                p.v[p_drive]     = 0.0;
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
            case p_waveform:
                return std::clamp(value, 0.0, 1.0);
            case p_slide:
                return std::clamp(value, k_slide_min_ms, k_slide_max_ms);
            case p_attack:
                return std::clamp(value, k_attack_min_ms, k_attack_max_ms);
            case p_accdecay:
                return std::clamp(value, k_accdecay_min_ms, k_accdecay_max_ms);
            case p_drive:
                return std::clamp(value, -k_drive_range_db, k_drive_range_db);
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
                // Factory presets, slots 0..7 (recall 1..8 in the wrapper). Field order:
                // gain, tuning, cutoff, res, envmod, decay, accent, waveform, slide, attack, accdecay, drive
                const double factory[k_factory_presets][k_num_params] = {
                    {0, 0, 500, 0.9, 0.7, 300, 0.8, 0, 60, 3, 200, 0},     // 1 classic squelch
                    {0, 0, 250, 0.45, 0.3, 800, 0.4, 1, 60, 3, 200, 0},    // 2 deep sub
                    {0, 0, 900, 1.3, 0.9, 250, 1.0, 0, 60, 3, 150, 6},     // 3 screamer
                    {0, 0, 400, 0.75, 0.55, 500, 0.6, 1, 90, 5, 250, 0},   // 4 rubber
                    {0, 0, 650, 1.0, 0.2, 200, 0.9, 1, 60, 3, 120, 0},     // 5 hollow knock
                    {0, 0, 350, 0.85, 1.0, 2000, 0.5, 0, 120, 10, 400, 0}, // 6 slow bloom
                    {0, 0, 700, 1.1, 0.65, 350, 0.85, 0, 60, 3, 200, 12},  // 7 overdriven
                    {0, 0, 2000, 0.6, 0.35, 600, 0.3, 0, 40, 3, 200, -6},  // 8 glass
                };
                for (int s = 0; s < k_factory_presets; ++s) {
                    for (int i = 0; i < k_num_params; ++i) {
                        m_presets[s].v[i] = factory[s][i];
                    }
                }
            }

            // -- lifecycle -------------------------------------------------------------------------------

            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_osc.prepare(m_sr);
                m_osc.set_smooth_ms(m_smooth_ms);
                m_osc.set_waveform(vco::wave_saw); // always the saw core; the square is shaped from it
                m_osc.snap();
                m_filter.prepare(m_sr);
                m_filter.set_smooth_ms(0.0);            // the voice owns all smoothing
                m_vca_stage.set_drive(k_vca_sat_drive); // stock 303 transistor-stage calibration
                m_vca_stage.set_bias(k_vca_sat_bias);
                m_vca_stage.set_mode(m_vca_mode);
                apply_unit_spread();
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
                m_c13                       = 0.0;
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

            /// The accent-sweep capacitor's current charge (0..~1) — the across-notes memory.
            double accent_charge() const { return m_c13; }

            // -- modes (not part of the morphable parameter set) --------------------------------------------

            void set_waveform(int w) {
                set_param(p_waveform, static_cast<double>(std::clamp(w, 0, k_num_waveforms - 1)));
            }
            int wave() const { return (m_ramp[p_waveform].target >= 0.5) ? wave_square : wave_saw; }

            /// Phase-2 VCA circuit: vca_clean (linear, default — bit-identical to phase 1) or
            /// vca_warm (the one-transistor stage's biased saturation; see the header).
            void set_vca(int mode) {
                m_vca_mode = std::clamp(mode, 0, k_num_vca_modes - 1);
                m_vca_stage.set_mode(m_vca_mode);
            }
            int vca() const { return m_vca_mode; }

            /// Deterministic per-unit component spread: `seed` picks the unit, `tolerance` (0..1)
            /// scales how far off nominal it is. tolerance 0 = the nominal schematic exactly.
            void set_seed(uint32_t seed) {
                m_seed = seed;
                apply_unit_spread();
            }
            uint32_t seed() const { return m_seed; }

            void set_tolerance(double t) {
                m_tolerance = std::clamp(t, 0.0, 1.0);
                apply_unit_spread();
            }
            double tolerance() const { return m_tolerance; }

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
            void set_wave_blend(double w) { set_param(p_waveform, w); }
            void set_slide(double ms) { set_param(p_slide, ms); }
            void set_attack(double ms) { set_param(p_attack, ms); }
            void set_accdecay(double ms) { set_param(p_accdecay, ms); }
            void set_drive(double db) { set_param(p_drive, db); }

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
                const double note = m_pitch + (m_ramp[p_tuning].current + m_unit_tune_cents) * 0.01;
                const double hz   = 440.0 * std::exp2((note - 69.0) / 12.0);

                // envelopes
                step_env(m_meg, m_meg_rising, m_meg_attack, (m_note_accent > 0.0) ? m_meg_accent_decay : m_meg_decay);
                step_env(m_vca, m_vca_rising, m_vca_attack, m_gate ? m_vca_decay : m_vca_release);

                // the accent sweep circuit: diode-gated charge onto C13, slow drain (the wow memory)
                const double acc_amt = m_ramp[p_accent].current * m_note_accent;
                const double drive   = acc_amt * m_meg;
                if (drive > m_c13) {
                    m_c13 += (drive - m_c13) * m_c13_charge;
                }
                m_c13 -= m_c13 * m_c13_drain;
                m_c13 = (m_c13 < 1e-15) ? 0.0 : m_c13;

                // cutoff: Open303's measured envmod law (see header), plus the accent sweep injected
                // directly (bypassing envmod), scaled by the ganged resonance-pot section
                const double cutoff_hz = m_ramp[p_cutoff].current * m_unit_cutoff_scale;
                const double e         = m_ramp[p_envmod].current;
                const double c         = std::clamp(std::log(cutoff_hz / k_knob_lo_hz) * k_inv_log_knob_span, 0.0, 1.0);
                const double scaler    = (1.0 - c) * (k_env_scaler_lo_f * e + k_env_scaler_lo_c)
                                      + c * (k_env_scaler_hi_f * e + k_env_scaler_hi_c);
                const double offset  = k_env_offset_f * c + k_env_offset_c;
                const double res_mix = 0.3 + 0.7 * std::min(m_ramp[p_resonance].current, 1.0);
                const double acc_oct =
                    k_accent_sweep_oct * res_mix * (k_accent_direct * std::max(drive - m_c13, 0.0) + m_c13);
                const double fc_eff = cutoff_hz * std::exp2(scaler * (m_meg - offset) + acc_oct);

                // oscillator: polyBLEP saw, with the square shaped from its half-cycle-shifted copy
                // (Open303's measured biased-tanh shaper) and the waveform blend riding the ramps
                const double saw = m_osc.process_at(hz);
                const double mix = m_ramp[p_waveform].current;
                double       osc = saw;
                if (mix > 0.0) {
                    const double shifted = (saw < 0.0) ? saw + 1.0 : saw - 1.0;
                    const double square  = -std::tanh(k_square_gain * shifted + k_square_bias);
                    osc                  = saw + mix * (square - saw);
                }

                // -> coupling HPF -> diode ladder -> VCA -> output coupling
                const double filtered     = m_filter.process(m_pre_hp.tick(osc), fc_eff);
                const double accent_boost = 1.0 + m_ramp[p_accent].current * m_note_accent;

                if (m_vca_mode == vca_warm) {
                    // hardware order: the transistor stage saturates the enveloped signal, then the
                    // output coupling removes the shaper's signal-dependent DC
                    const double v = filtered * m_vca * accent_boost;
                    const double s = m_vca_stage.shape(v); // shared transistor-stage saturator (vca.h)
                    return m_post_hp.tick(s) * m_out_gain;
                }
                const double shaped = m_post_hp.tick(filtered);
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

            // The filter's own smoothing is off; the voice's ramps feed it directly. Also refreshes
            // every parameter-derived envelope/slide coefficient (only runs when a ramp moves).
            void push_filter_params() {
                m_filter.set_param(diode::p_resonance, m_ramp[p_resonance].current);
                m_filter.set_param(diode::p_drive, m_ramp[p_drive].current);
                m_out_gain         = std::pow(10.0, m_ramp[p_gain].current * 0.05);
                m_meg_decay        = decay_coef(m_ramp[p_decay].current * m_unit_env);
                m_meg_accent_decay = decay_coef(m_ramp[p_accdecay].current * m_unit_env);
                m_vca_attack       = attack_coef(m_ramp[p_attack].current * m_unit_attack);
                m_slide_coef       = one_pole_coef(m_ramp[p_slide].current * m_unit_slide);
            }

            // Deterministic per-unit draw in [-1, 1]: xorshift64* on (seed, salt), house convention.
            static double unit_draw(uint32_t seed, uint32_t salt) {
                uint64_t x = (static_cast<uint64_t>(seed) << 32) ^ (0x9e3779b97f4a7c15ULL * (salt + 1));
                x ^= x >> 12;
                x ^= x << 25;
                x ^= x >> 27;
                x *= 0x2545f4914f6cdd1dULL;
                return 2.0 * (static_cast<double>(x >> 11) / 9007199254740992.0) - 1.0;
            }

            // Recompute the per-unit component offsets and every coefficient built on them.
            // tolerance 0 leaves the nominal schematic exactly (all factors 1, imperfect 0).
            void apply_unit_spread() {
                const double t      = m_tolerance;
                m_unit_tune_cents   = 8.0 * t * unit_draw(m_seed, 0);             // converter trim
                m_unit_cutoff_scale = std::exp2(0.12 * t * unit_draw(m_seed, 1)); // VCF tracking
                m_unit_env          = 1.0 + 0.10 * t * unit_draw(m_seed, 2);      // envelope caps
                m_unit_vca          = 1.0 + 0.10 * t * unit_draw(m_seed, 3);
                m_unit_slide        = 1.0 + 0.15 * t * unit_draw(m_seed, 4); // slide RC
                m_unit_c13          = 1.0 + 0.15 * t * unit_draw(m_seed, 5); // accent-sweep RC
                m_unit_attack       = 1.0 + 0.20 * t * unit_draw(m_seed, 6);
                m_meg_attack        = attack_coef(k_meg_attack_ms);
                m_vca_decay         = decay_coef(k_vca_decay_ms * m_unit_vca);
                m_vca_release       = decay_coef(k_vca_release_ms);
                m_c13_charge        = one_pole_coef(k_c13_charge_ms * m_unit_c13);
                m_c13_drain         = one_pole_coef(k_c13_discharge_ms * m_unit_c13);
                m_osc.set_seed(m_seed);
                m_osc.set_imperfect(0.6 * t); // waveform imperfection rides the same tolerance
                push_filter_params();
            }

            // configuration
            double   m_sr{48000.0};
            double   m_smooth_ms{k_default_smooth_ms};
            uint32_t m_seed{1u};
            double   m_tolerance{0.0};
            bool     m_prepared{false};

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
            double m_meg_decay{0.9999}, m_meg_accent_decay{0.999};
            double m_vca_decay{0.9999}, m_vca_release{0.99};
            double m_slide_coef{0.001};

            // phase-2 VCA circuit — the saturator math lives in the shared tap::tools::vca stage
            int             m_vca_mode{vca_clean};
            ::tap::tools::vca m_vca_stage; // shape() only; the voice runs its own coupling + gain

            // per-unit component spread (seed/tolerance)
            double m_unit_tune_cents{0.0};
            double m_unit_cutoff_scale{1.0};
            double m_unit_env{1.0}, m_unit_vca{1.0};
            double m_unit_slide{1.0}, m_unit_c13{1.0}, m_unit_attack{1.0};

            // accent-sweep circuit state (C13)
            double m_c13{0.0};
            double m_c13_charge{0.001}, m_c13_drain{0.0001};

            // DSP blocks
            vco::vco_osc        m_osc;
            diode::diode_filter m_filter;
            hp1                 m_pre_hp, m_post_hp;
        };

    } // namespace tb303
} // namespace tap::tools
