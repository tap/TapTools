/// @file autowah.h
/// @brief      Portable envelope-filter (auto-wah) kernel for tap.autowah~ — no Max/Min dependency.
/// @details    A behavioral model of the Mad Professor Snow White AutoWah (Björn Juhl's design):
///             a 2-pole state-variable filter whose cutoff is pushed up from a resting point by an
///             envelope follower with a fast attack and a musician-set decay. The hardware is an
///             LM13700 OTA SVF swept by a diode + RC detector; published sweep range 250–2500 Hz.
///             Model (traced circuit facts vs. modeling choices documented per point):
///
///             - Filter core: composes the Simper SVF kernel (svf.h) as a single 2nd-order section,
///               driven per sample through its signal-rate cutoff path (tick(cutoff_hz)) — the TPT
///               core is unconditionally stable under per-sample modulation, which is the whole job
///               here. `mode` selects the lowpass or bandpass tap: the traced circuit has both, the
///               stock voicing is believed to be the resonant lowpass node (inference — flagged in
///               the plan; the default may flip after hardware calibration). Q comes from the
///               normalized resonance (0..1) via the shared svf mapping.
///             - Envelope detector: sensitivity gain (dB) -> rectifier -> one-pole attack/release
///               follower. Attack is fast by default (2 ms, the pedal's fixed-attack character) but
///               exposed; decay is the player control (10 ms .. 5 s, matching the widened
///               1 M-pot revision of the hardware). The rectifier is full-wave by default;
///               half-wave (the traced single-diode topology) is selectable for the hardware's
///               sweep-rate ripple — an A/B for the calibration pass (set_rectifier()).
///             - Sweep law: cutoff = bias * 2^(sweep * range_octaves), where sweep in [0, 1) is the
///               envelope through a tanh soft knee (hard playing compresses into the ceiling
///               instead of slamming a rail). Exponential-in-Hz is a modeling *choice*: the LM13700
///               frequency is linear in control current, so the hardware law hinges on its BJT
///               driver stage — the law lives in one function (map_cutoff()) so the hardware
///               calibration pass can swap it without touching anything else. `range` is signed:
///               negative sweeps downward from bias (a deliberate extension; the pedal is up-only).
///             - Sensitivity at its floor (-60 dB) is treated as exactly off: the filter parks at
///               `bias` and the object becomes the pedal's secondary "cocked wah" — a fixed,
///               manually swept resonant filter (bias *is* the sweep knob).
///             - `drive` (dB, 0 = clean/linear) engages the svf driven circuit — tanh band-node
///               limiting, 2x oversampled — as the optional OTA-flavored color stage.
///             - Wet-only like the pedal by default; `mix` is an equal-power dry/wet extension
///               (100 % = hardware behavior). `gain` is a master output level in dB.
///             - House kit: every audible parameter rides a per-sample linear ramp (no zipper), a
///               16-slot preset-morph engine (store/recall with timed interpolation; slots 0–3 ship
///               factory voicings: guitar, bass, slow swell, cocked wah), allocation-free after
///               prepare(), deterministic, setters safe while audio runs. Single-channel by design
///               (per-channel envelopes are correct under mc. wrapping); process(x, key) exposes a
///               sidechain input for the wrapper's key inlet.
///
///             Reference provenance: freestompboxes.org trace (t=29137), La Révolution Deux block
///             analysis, PedalPCB "Poison Apple" build doc, official Mad Professor manuals. See
///             the TapTools-Max plan doc (plans/tap.autowah~.md) for the validation method.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include "svf.h"

namespace taptools {
    namespace autowah {

        constexpr double k_pi                = 3.14159265358979323846;
        constexpr int    k_presets           = 16; // house slot count (grm_comb / grm_pitchaccum)
        constexpr double k_default_smooth_ms = 20.0;

        constexpr double k_freq_floor_hz = 20.0;
        constexpr double k_freq_ceil_hz  = 20000.0;
        constexpr double k_sens_floor_db = -60.0; // exactly off: fixed-filter ("cocked wah") mode
        constexpr double k_sens_ceil_db  = 24.0;
        constexpr double k_attack_min_ms = 0.05;
        constexpr double k_attack_max_ms = 100.0;
        constexpr double k_decay_min_ms  = 10.0;
        constexpr double k_decay_max_ms  = 5000.0;
        constexpr double k_range_max_oct = 5.0; // signed; hardware span is ~ +3.3
        constexpr double k_drive_max_db  = 24.0;
        constexpr double k_env_knee      = 1.5; // tanh soft-knee gain, envelope -> sweep
        constexpr int    k_oversample    = 2;   // fixed for the driven circuit (svf house default)

        enum param_index : int {
            p_sensitivity = 0, // envelope-detector input gain, dB (k_sens_floor_db = off/manual)
            p_attack,          // follower attack, ms (fast + fixed on the pedal; default 2)
            p_decay,           // follower release, ms — the pedal's Decay knob
            p_bias,            // resting/center frequency, Hz — the pedal's Bias knob
            p_range,           // sweep span in octaves, signed (negative sweeps down)
            p_resonance,       // 0..1 normalized Q — the pedal's Resonance knob
            p_drive,           // dB into the svf driven circuit; 0 = clean linear
            p_gain,            // master output gain, dB
            p_mix,             // 0..100 dry->wet, equal-power (100 = wet-only, the hardware)
            k_num_params
        };

        enum filter_mode : int {
            mode_lowpass = 0, // stock voicing (inferred tap — see header note)
            mode_bandpass,    // the circuit's other tap (the known hardware mod)
            k_num_modes
        };

        enum rectifier_mode : int {
            rect_fullwave = 0, // |x| — cleaner tracking (default)
            rect_halfwave,     // max(x, 0) — the traced single-diode topology
            k_num_rectifiers
        };

        /// One full parameter snapshot — a preset slot, and the unit the morph engine interpolates.
        struct params {
            std::array<double, k_num_params> v{};

            static params defaults() {
                params p;
                p.v[p_sensitivity] = 0.0;
                p.v[p_attack]      = 2.0;
                p.v[p_decay]       = 250.0;
                p.v[p_bias]        = 250.0; // hardware resting point
                p.v[p_range]       = 3.3;   // hardware span, 250 -> ~2500 Hz
                p.v[p_resonance]   = 0.55;
                p.v[p_drive]       = 0.0;
                p.v[p_gain]        = 0.0;
                p.v[p_mix]         = 100.0;
                return p;
            }
        };

        /// Clamp a value to the legal range of a parameter. Gain (dB) is unclamped, house-style.
        inline double clamp_param(int index, double value) {
            switch (index) {
            case p_sensitivity:
                return std::clamp(value, k_sens_floor_db, k_sens_ceil_db);
            case p_attack:
                return std::clamp(value, k_attack_min_ms, k_attack_max_ms);
            case p_decay:
                return std::clamp(value, k_decay_min_ms, k_decay_max_ms);
            case p_bias:
                return std::clamp(value, k_freq_floor_hz, k_freq_ceil_hz);
            case p_range:
                return std::clamp(value, -k_range_max_oct, k_range_max_oct);
            case p_resonance:
                return std::clamp(value, 0.0, 1.0);
            case p_drive:
                return std::clamp(value, 0.0, k_drive_max_db);
            case p_mix:
                return std::clamp(value, 0.0, 100.0);
            default:
                return value;
            }
        }

        /// The envelope filter: detector + sweep law + composed Simper SVF + the preset-morph engine.
        class wah_filter {
          public:
            wah_filter() {
                const params d = params::defaults();
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = d.v[i];
                }
                install_factory_presets();
            }

            // -- lifecycle -------------------------------------------------------------------------------

            /// Set the sample rate, configure the composed filter, clear state, snap ramps.
            /// Allocates (inside the svf) — call from the main thread, not the perform loop.
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_svf.prepare(m_sr, 1);
                m_svf.set_smooth_ms(0.0); // this kernel owns all smoothing; svf setters snap
                m_svf.set_order(2);
                m_svf.set_oversample(k_oversample);
                apply_mode();
                m_svf_resonance = m_svf_drive = -1.0; // force a forward on the first sample
                m_svf_circuit                 = -1;
                snap();
                m_env = m_sweep = 0.0;
            }

            /// Zero the filter state and the envelope; parameters untouched.
            void clear() {
                m_svf.clear();
                m_env = m_sweep = 0.0;
            }

            /// Jump all parameter ramps to their targets.
            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = 0.0;
                    r.remaining = 0;
                }
                m_ramps_active  = 0;
                m_derived_dirty = true;
            }

            double samplerate() const { return m_sr; }

            // -- structural modes (not ramped, not morphed) ------------------------------------------------

            void set_mode(int mode) {
                m_mode = std::clamp(mode, 0, k_num_modes - 1);
                apply_mode();
            }
            int mode() const { return m_mode; }

            void set_rectifier(int rect) { m_rectifier = std::clamp(rect, 0, k_num_rectifiers - 1); }
            int  rectifier() const { return m_rectifier; }

            /// Anti-zipper ramp time for direct setters, in ms. 0 = instant (useful for tests).
            void   set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }
            double smooth_ms() const { return m_smooth_ms; }

            // -- parameter targets (click-free; safe while audio runs) ------------------------------------

            void set_param(int index, double value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                ramp_to(index, clamp_param(index, value), smooth_samples());
            }

            void set_sensitivity(double db) { set_param(p_sensitivity, db); }
            void set_attack(double ms) { set_param(p_attack, ms); }
            void set_decay(double ms) { set_param(p_decay, ms); }
            void set_bias(double hz) { set_param(p_bias, hz); }
            void set_range(double octaves) { set_param(p_range, octaves); }
            void set_resonance(double r) { set_param(p_resonance, r); }
            void set_drive(double db) { set_param(p_drive, db); }
            void set_gain(double db) { set_param(p_gain, db); }
            void set_mix(double pct) { set_param(p_mix, pct); }

            double param(int index) const { return (index >= 0 && index < k_num_params) ? m_ramp[index].target : 0.0; }

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

            /// Programmatic preset load (state restore; overwrite a factory slot).
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

            // -- introspection ---------------------------------------------------------------------------

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

            /// The current sweep position, 0..1 (post-knee) — the wrapper's envelope outlet.
            double envelope() const { return m_sweep; }

            /// The cutoff computed on the last sample, Hz — for tests and analysis.
            double cutoff_hz() const { return m_cutoff; }

            // -- audio -----------------------------------------------------------------------------------

            /// Process one sample; the envelope tracks the input itself (the pedal).
            double process(double x) { return process(x, x); }

            /// Process one sample with a sidechain: the filter runs on `x`, the envelope tracks `key`.
            double process(double x, double key) {
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

                // detector: gain -> rectify -> one-pole attack/release
                const double driven = key * m_sens_gain;
                const double rect   = (m_rectifier == rect_halfwave) ? std::max(driven, 0.0) : std::abs(driven);
                const double coef   = (rect > m_env) ? m_attack_coef : m_decay_coef;
                m_env               = anti_denormal(m_env + coef * (rect - m_env));

                // sweep law — isolated so the hardware calibration pass can swap it (see header)
                m_sweep  = std::tanh(k_env_knee * m_env);
                m_cutoff = map_cutoff(m_sweep);

                m_svf.tick(m_cutoff);
                const double wet = m_svf.process(0, x);

                return x * m_dry_gain + wet * m_wet_gain;
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

            static bool valid_slot(int s) { return s >= 0 && s < k_presets; }

            static double anti_denormal(double x) {
                return (std::abs(x) < 1e-15) ? 0.0 : x; // house idiom (tap.comb~)
            }

            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

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

            void apply_mode() { m_svf.set_mode((m_mode == mode_bandpass) ? svf::mode_bandpass : svf::mode_lowpass); }

            /// The sweep law: resting point at sweep 0, `range` octaves away at sweep 1, exponential
            /// in between. THE swappable function — if the hardware calibration pass finds the OTA
            /// driver is a linear V->I stage, this becomes bias + sweep * span_hz, and nothing else
            /// in the kernel changes.
            double map_cutoff(double sweep) const {
                const double hz = m_bias * std::exp2(sweep * m_range);
                return std::clamp(hz, k_freq_floor_hz, m_cutoff_ceil);
            }

            // Recompute all derived per-sample DSP values from the (smoothed) parameter values.
            // Runs every sample while any ramp is active, once when everything has settled.
            void update_derived() {
                const auto& val = m_ramp;

                // sensitivity: dB -> linear, with the floor treated as exactly off (manual mode)
                const double sens_db = val[p_sensitivity].current;
                m_sens_gain          = (sens_db <= k_sens_floor_db + 1e-9) ? 0.0 : std::pow(10.0, sens_db * 0.05);

                // follower coefficients: 63% time constants from the ms parameters
                m_attack_coef = 1.0 - std::exp(-1000.0 / (std::max(val[p_attack].current, 0.01) * m_sr));
                m_decay_coef  = 1.0 - std::exp(-1000.0 / (std::max(val[p_decay].current, 0.01) * m_sr));

                m_bias        = val[p_bias].current;
                m_range       = val[p_range].current;
                m_cutoff_ceil = std::min(k_freq_ceil_hz, 0.45 * m_sr);

                // forward into the composed svf only on change (its setters snap; see prepare())
                if (val[p_resonance].current != m_svf_resonance) {
                    m_svf_resonance = val[p_resonance].current;
                    m_svf.set_resonance(m_svf_resonance);
                }
                if (val[p_drive].current != m_svf_drive) {
                    m_svf_drive = val[p_drive].current;
                    m_svf.set_drive(m_svf_drive);
                }
                // drive 0 runs the pure linear circuit; any drive engages the saturating one. The
                // switch happens where tanh is near-identity for typical levels, so it is benign.
                const int circuit = (m_svf_drive > 1e-6) ? svf::circuit_driven : svf::circuit_clean;
                if (circuit != m_svf_circuit) {
                    m_svf_circuit = circuit;
                    m_svf.set_circuit(circuit);
                }

                const double g     = std::pow(10.0, val[p_gain].current * 0.05);
                const double theta = std::clamp(val[p_mix].current, 0.0, 100.0) * 0.01 * (k_pi * 0.5);
                m_dry_gain         = std::cos(theta) * g; // equal-power, like tap.crossfade~
                m_wet_gain         = std::sin(theta) * g;
            }

            // Factory voicings in slots 0-3 (author-approved 2026-07-15); the rest hold defaults.
            void install_factory_presets() {
                params guitar = params::defaults(); // slot 0: the hardware's home position

                params bass         = params::defaults(); // slot 1: the GB switch, as a preset
                bass.v[p_bias]      = 120.0;
                bass.v[p_range]     = 3.0;
                bass.v[p_decay]     = 300.0;
                bass.v[p_resonance] = 0.5;

                params swell         = params::defaults(); // slot 2: slow filter swells
                swell.v[p_bias]      = 200.0;
                swell.v[p_decay]     = 1500.0;
                swell.v[p_attack]    = 5.0;
                swell.v[p_resonance] = 0.65;

                params cocked           = params::defaults(); // slot 3: sensitivity off = fixed filter
                cocked.v[p_sensitivity] = k_sens_floor_db;
                cocked.v[p_bias]        = 800.0;
                cocked.v[p_resonance]   = 0.7;

                m_presets.fill(params::defaults());
                m_presets[0] = guitar;
                m_presets[1] = bass;
                m_presets[2] = swell;
                m_presets[3] = cocked;
            }

            double m_sr{48000.0};
            double m_smooth_ms{k_default_smooth_ms};
            int    m_mode{mode_lowpass};
            int    m_rectifier{rect_fullwave};

            std::array<ramp, k_num_params> m_ramp;
            std::array<params, k_presets>  m_presets;
            int                            m_ramps_active{0};
            bool                           m_derived_dirty{true};

            svf::svf_filter m_svf;

            // detector / sweep state
            double m_env{0.0};
            double m_sweep{0.0};
            double m_cutoff{250.0};

            // derived (cached while parameters are settled)
            double m_sens_gain{1.0};
            double m_attack_coef{0.0};
            double m_decay_coef{0.0};
            double m_bias{250.0};
            double m_range{3.3};
            double m_cutoff_ceil{k_freq_ceil_hz};
            double m_dry_gain{0.0};
            double m_wet_gain{1.0};
            double m_svf_resonance{-1.0};
            double m_svf_drive{-1.0};
            int    m_svf_circuit{-1};
        };

    } // namespace autowah
} // namespace taptools
