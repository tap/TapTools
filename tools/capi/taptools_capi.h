/// @file taptools_capi.h
/// @brief Minimal C ABI over the portable TapTools DSP cores, for language bindings and the
///        verification notebooks (notebooks/ drive it via ctypes).
///
///        Conventions: plain C types only; the caller owns all arrays and sizes them. Handle-based
///        functions return 0 on success and -1 on any error (bad argument, unconfigured engine).
///        No global state. Exposes tap.convolve~'s conv_engine (uniformly-partitioned overlap-save
///        convolution) plus the parameter-indexed kernels behind tap.svf~, tap.ladder~, tap.vco~,
///        and tap.autowah~ (param indices and mode/solver/waveform constants match the enums in
///        each kernel header; ..._set() takes the kernel's param_index).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#define TAPTOOLS_API __declspec(dllexport)
#else
#define TAPTOOLS_API __attribute__((visibility("default")))
#endif

typedef void* taptools_conv;

/// Create / destroy a convolution engine instance.
TAPTOOLS_API taptools_conv taptools_conv_create(void);
TAPTOOLS_API void          taptools_conv_destroy(taptools_conv engine);

/// Allocate the engine for a partition size (samples, power of two) and a maximum partition count.
/// Discards any loaded IR and clears all state. Returns 0, or -1 on a bad handle.
TAPTOOLS_API int taptools_conv_configure(taptools_conv engine, int blocksize, int max_partitions);

/// Load the four true-stereo IR paths (LL, LR, RL, RR) of `length` samples into the engine and
/// publish them atomically. Any of the four pointers may be NULL for a silent path; `scale` is
/// applied to every sample. Returns 0, or -1 on a bad/unconfigured handle.
TAPTOOLS_API int taptools_conv_load_ir(taptools_conv engine, const float* ll, const float* lr, const float* rl,
                                       const float* rr, int length, double scale);

/// Flush the running state (input history + pending output); keeps the loaded IR. Returns 0/-1.
TAPTOOLS_API int taptools_conv_clear(taptools_conv engine);

/// Process n stereo samples. Wet (fully convolved) output is written to outL/outR (double).
/// Input and output buffers must not alias. Returns 0, or -1 on a bad handle.
TAPTOOLS_API int taptools_conv_process(taptools_conv engine, const double* inL, const double* inR, double* outL,
                                       double* outR, int n);

/// Introspection.
TAPTOOLS_API int taptools_conv_block_size(taptools_conv engine);     // partition size (= latency)
TAPTOOLS_API int taptools_conv_max_partitions(taptools_conv engine); // capacity in partitions
TAPTOOLS_API int taptools_conv_has_ir(taptools_conv engine);         // 1 if an IR is published, else 0

// ---- tap.svf~ (tap::tools::svf::svf_filter, mono) -----------------------------------------------

typedef void* taptools_svf;

TAPTOOLS_API taptools_svf taptools_svf_create(void);
TAPTOOLS_API void         taptools_svf_destroy(taptools_svf h);
TAPTOOLS_API int          taptools_svf_prepare(taptools_svf h, double sr);
TAPTOOLS_API int          taptools_svf_set(taptools_svf h, int param, double value); // svf::param_index
TAPTOOLS_API int          taptools_svf_set_mode(taptools_svf h, int mode);           // svf::filter_mode
TAPTOOLS_API int          taptools_svf_set_order(taptools_svf h, int order);         // 2, 4, or 8
TAPTOOLS_API int          taptools_svf_set_circuit(taptools_svf h, int circuit);     // svf::circuit_mode
TAPTOOLS_API int          taptools_svf_set_oversample(taptools_svf h, int os);       // 1, 2, or 4
TAPTOOLS_API int          taptools_svf_set_smooth_ms(taptools_svf h, double ms);
TAPTOOLS_API int          taptools_svf_clear(taptools_svf h);
TAPTOOLS_API int          taptools_svf_process(taptools_svf h, const double* in, double* out, int n);
/// Per-sample signal-rate cutoff (Hz) — the external's right-inlet path.
TAPTOOLS_API int taptools_svf_process_mod(taptools_svf h, const double* in, const double* cutoff_hz, double* out,
                                          int n);

// ---- tap.ladder~ (tap::tools::ladder::ladder_filter) --------------------------------------------

typedef void* taptools_ladder;

TAPTOOLS_API taptools_ladder taptools_ladder_create(void);
TAPTOOLS_API void            taptools_ladder_destroy(taptools_ladder h);
TAPTOOLS_API int             taptools_ladder_prepare(taptools_ladder h, double sr);
TAPTOOLS_API int             taptools_ladder_set(taptools_ladder h, int param, double value); // ladder::param_index
TAPTOOLS_API int             taptools_ladder_set_mode(taptools_ladder h, int mode);           // ladder::filter_mode
TAPTOOLS_API int             taptools_ladder_set_solver(taptools_ladder h, int solver);       // ladder::solver_mode
TAPTOOLS_API int             taptools_ladder_set_oversample(taptools_ladder h, int os);       // 1, 2, or 4
TAPTOOLS_API int             taptools_ladder_set_smooth_ms(taptools_ladder h, double ms);
TAPTOOLS_API int             taptools_ladder_clear(taptools_ladder h);
TAPTOOLS_API int             taptools_ladder_process(taptools_ladder h, const double* in, double* out, int n);
TAPTOOLS_API int taptools_ladder_process_mod(taptools_ladder h, const double* in, const double* cutoff_hz, double* out,
                                             int n);

// ---- tap.vco~ (tap::tools::vco::vco_osc) ---------------------------------------------------------

typedef void* taptools_vco;

TAPTOOLS_API taptools_vco taptools_vco_create(void);
TAPTOOLS_API void         taptools_vco_destroy(taptools_vco h);
TAPTOOLS_API int          taptools_vco_prepare(taptools_vco h, double sr);
TAPTOOLS_API int          taptools_vco_set(taptools_vco h, int param, double value); // vco::param_index
TAPTOOLS_API int          taptools_vco_set_seed(taptools_vco h, unsigned int seed);
TAPTOOLS_API int          taptools_vco_set_smooth_ms(taptools_vco h, double ms);
TAPTOOLS_API int          taptools_vco_clear(taptools_vco h);
TAPTOOLS_API int          taptools_vco_process(taptools_vco h, double* out, int n);
/// Through-zero FM (Hz) and/or hard-sync inputs, either may be NULL.
TAPTOOLS_API int taptools_vco_process_mod(taptools_vco h, const double* fm_hz, const double* sync, double* out, int n);

// ---- tap.diode~ (tap::tools::diode::diode_filter) ------------------------------------------------

typedef void* taptools_diode;

TAPTOOLS_API taptools_diode taptools_diode_create(void);
TAPTOOLS_API void           taptools_diode_destroy(taptools_diode h);
TAPTOOLS_API int            taptools_diode_prepare(taptools_diode h, double sr);
TAPTOOLS_API int            taptools_diode_set(taptools_diode h, int param, double value); // diode::param_index
TAPTOOLS_API int            taptools_diode_set_solver(taptools_diode h, int solver);       // diode::solver_mode
TAPTOOLS_API int            taptools_diode_set_oversample(taptools_diode h, int os);       // 1, 2, or 4
TAPTOOLS_API int            taptools_diode_set_smooth_ms(taptools_diode h, double ms);
TAPTOOLS_API int            taptools_diode_clear(taptools_diode h);
TAPTOOLS_API int            taptools_diode_process(taptools_diode h, const double* in, double* out, int n);
TAPTOOLS_API int taptools_diode_process_mod(taptools_diode h, const double* in, const double* cutoff_hz, double* out,
                                            int n);

// ---- tap.303~ (tap::tools::tb303::voice) ---------------------------------------------------------

typedef void* taptools_tb303;

TAPTOOLS_API taptools_tb303 taptools_tb303_create(void);
TAPTOOLS_API void           taptools_tb303_destroy(taptools_tb303 h);
TAPTOOLS_API int            taptools_tb303_prepare(taptools_tb303 h, double sr);
TAPTOOLS_API int            taptools_tb303_set(taptools_tb303 h, int param, double value); // tb303::param_index
TAPTOOLS_API int            taptools_tb303_set_vca(taptools_tb303 h, int mode);            // tb303::vca_mode
TAPTOOLS_API int            taptools_tb303_set_solver(taptools_tb303 h, int solver);
TAPTOOLS_API int            taptools_tb303_set_oversample(taptools_tb303 h, int os);
TAPTOOLS_API int            taptools_tb303_set_seed(taptools_tb303 h, unsigned int seed);
TAPTOOLS_API int            taptools_tb303_set_tolerance(taptools_tb303 h, double t);
TAPTOOLS_API int            taptools_tb303_set_smooth_ms(taptools_tb303 h, double ms);
/// Morph to a preset slot (0-based; 0-7 are the factory acid patches) over `seconds`.
TAPTOOLS_API int taptools_tb303_recall(taptools_tb303 h, int slot, double seconds);
TAPTOOLS_API int taptools_tb303_clear(taptools_tb303 h);
/// The note interface (the melodic-voice contract): trigger with accent depth 0..1; a note_on
/// while the gate is held slides (legato). set_pitch changes the target without gating.
TAPTOOLS_API int    taptools_tb303_note_on(taptools_tb303 h, double midi_note, double accent);
TAPTOOLS_API int    taptools_tb303_note_off(taptools_tb303 h);
TAPTOOLS_API int    taptools_tb303_set_pitch(taptools_tb303 h, double midi_note);
TAPTOOLS_API double taptools_tb303_accent_charge(taptools_tb303 h); // the C13 wow memory, 0..~1
TAPTOOLS_API int    taptools_tb303_process(taptools_tb303 h, double* out, int n);

// ---- tap.autowah~ (tap::tools::autowah::wah_filter) ----------------------------------------------

typedef void* taptools_wah;

TAPTOOLS_API taptools_wah taptools_wah_create(void);
TAPTOOLS_API void         taptools_wah_destroy(taptools_wah h);
TAPTOOLS_API int          taptools_wah_prepare(taptools_wah h, double sr);
TAPTOOLS_API int          taptools_wah_set(taptools_wah h, int param, double value); // autowah::param_index
TAPTOOLS_API int          taptools_wah_set_mode(taptools_wah h, int mode);           // autowah::filter_mode
TAPTOOLS_API int          taptools_wah_set_rectifier(taptools_wah h, int rect);      // autowah::rectifier_mode
TAPTOOLS_API int          taptools_wah_set_smooth_ms(taptools_wah h, double ms);
/// Morph to a preset slot (0-based; 0-3 are the factory voicings) over `seconds`.
TAPTOOLS_API int taptools_wah_recall(taptools_wah h, int slot, double seconds);
TAPTOOLS_API int taptools_wah_clear(taptools_wah h);
/// Process n samples. `key` (sidechain) may be NULL to track `in`; `env_out` (sweep, 0..1) and
/// `cutoff_out` (Hz) may be NULL, or receive the per-sample control trajectories for analysis.
TAPTOOLS_API int taptools_wah_process(taptools_wah h, const double* in, const double* key, double* out, double* env_out,
                                      double* cutoff_out, int n);

// ---- tap.808.seq~ (tap::tools::seq::trigger_row) -------------------------------------------------

typedef void* taptools_seqtrig;

TAPTOOLS_API taptools_seqtrig taptools_seqtrig_create(void);
TAPTOOLS_API void             taptools_seqtrig_destroy(taptools_seqtrig h);
TAPTOOLS_API int              taptools_seqtrig_prepare(taptools_seqtrig h, double sr);
TAPTOOLS_API int              taptools_seqtrig_set_length(taptools_seqtrig h, int steps);
TAPTOOLS_API int              taptools_seqtrig_set_swing(taptools_seqtrig h, double swing);
TAPTOOLS_API int              taptools_seqtrig_set_quantize(taptools_seqtrig h, int mode); // seq::quantize_mode
TAPTOOLS_API int              taptools_seqtrig_set_pulse_ms(taptools_seqtrig h, double ms);
/// Set one step's velocity (0 = rest); `step` is 0-based.
TAPTOOLS_API int taptools_seqtrig_set_step(taptools_seqtrig h, int step, double velocity);
TAPTOOLS_API int taptools_seqtrig_store(taptools_seqtrig h, int slot);
TAPTOOLS_API int taptools_seqtrig_recall(taptools_seqtrig h, int slot);
TAPTOOLS_API int taptools_seqtrig_reset(taptools_seqtrig h);
/// Run n samples of the phase ramp through the row; impulses land in `out`.
TAPTOOLS_API int taptools_seqtrig_process(taptools_seqtrig h, const double* phase, double* out, int n);

// ---- tap.303.seq~ (tap::tools::seq::note_row) ----------------------------------------------------

typedef void* taptools_seqnote;

TAPTOOLS_API taptools_seqnote taptools_seqnote_create(void);
TAPTOOLS_API void             taptools_seqnote_destroy(taptools_seqnote h);
TAPTOOLS_API int              taptools_seqnote_prepare(taptools_seqnote h, double sr);
TAPTOOLS_API int              taptools_seqnote_set_length(taptools_seqnote h, int steps);
TAPTOOLS_API int              taptools_seqnote_set_swing(taptools_seqnote h, double swing);
TAPTOOLS_API int              taptools_seqnote_set_quantize(taptools_seqnote h, int mode); // seq::quantize_mode
TAPTOOLS_API int              taptools_seqnote_set_transpose(taptools_seqnote h, double semitones);
/// Set one step (0-based): pitch as MIDI note, gate/accent/slide as 0/1 flags.
TAPTOOLS_API int taptools_seqnote_set_step(taptools_seqnote h, int step, double pitch, int gate, int accent, int slide);
TAPTOOLS_API int taptools_seqnote_store(taptools_seqnote h, int slot);
TAPTOOLS_API int taptools_seqnote_recall(taptools_seqnote h, int slot);
TAPTOOLS_API int taptools_seqnote_reset(taptools_seqnote h);
/// Run n samples of the phase ramp through the row; the tap.303~ inlet pair lands in
/// `pitch_out` (MIDI note) and `gate_out` (0 / 1.0 plain / 2.0 accented).
TAPTOOLS_API int taptools_seqnote_process(taptools_seqnote h, const double* phase, double* pitch_out, double* gate_out,
                                          int n);

// ---- tap.808.kick~ (tap::tools::tr808::kick) -----------------------------------------------------

typedef void* taptools_kick;

TAPTOOLS_API taptools_kick taptools_kick_create(void);
TAPTOOLS_API void          taptools_kick_destroy(taptools_kick h);
TAPTOOLS_API int           taptools_kick_prepare(taptools_kick h, double sr);
TAPTOOLS_API int           taptools_kick_set_decay(taptools_kick h, double v);    // 0..1 (panel VR6)
TAPTOOLS_API int           taptools_kick_set_tone(taptools_kick h, double v);     // 0..1 (panel VR5)
TAPTOOLS_API int           taptools_kick_set_level(taptools_kick h, double v);    // 0..1 (panel VR4)
TAPTOOLS_API int           taptools_kick_trigger(taptools_kick h, double accent); // 0..1 -> the 4-14 V bus
TAPTOOLS_API int           taptools_kick_reset(taptools_kick h);
/// Process n samples; `trig` may be NULL (free-run) or a signal whose rising edges above 1e-3
/// fire the voice with the edge value as accent — exactly the tap.808.kick~ wrapper's edge logic.
TAPTOOLS_API int taptools_kick_process(taptools_kick h, const double* trig, double* out, int n);

// ---- tap.tune~ (tap::tools::tune::corrector) -----------------------------------------------------

typedef void* taptools_tune;

TAPTOOLS_API taptools_tune taptools_tune_create(void);
TAPTOOLS_API void          taptools_tune_destroy(taptools_tune h);
TAPTOOLS_API int           taptools_tune_prepare(taptools_tune h, double sr);
TAPTOOLS_API int           taptools_tune_clear(taptools_tune h);
TAPTOOLS_API int           taptools_tune_set_key(taptools_tune h, int pitch_class);
TAPTOOLS_API int           taptools_tune_set_scale(taptools_tune h, unsigned mask); // relative to key
TAPTOOLS_API int           taptools_tune_set_notes(taptools_tune h, unsigned absolute_mask);
TAPTOOLS_API int           taptools_tune_set_mode(taptools_tune h, int mode);       // tune::mode
TAPTOOLS_API int           taptools_tune_set_backend(taptools_tune h, int backend); // tune::backend
TAPTOOLS_API int           taptools_tune_set_speed(taptools_tune h, double ms);
TAPTOOLS_API int           taptools_tune_set_amount(taptools_tune h, double pct);
TAPTOOLS_API int           taptools_tune_set_range(taptools_tune h, double min_hz, double max_hz);
TAPTOOLS_API int           taptools_tune_set_threshold(taptools_tune h, double t);
TAPTOOLS_API int           taptools_tune_set_formant(taptools_tune h, int on);
TAPTOOLS_API int           taptools_tune_set_autokey(taptools_tune h, int on);
TAPTOOLS_API int           taptools_tune_autokey_reset(taptools_tune h);
/// Current auto-key estimate: writes key (0-11, or -1 if none yet), minor (0/1), and the
/// profile-correlation confidence. Returns 0, or -1 on a bad handle.
TAPTOOLS_API int taptools_tune_autokey_estimate(taptools_tune h, int* key, int* minor, double* confidence);
/// Adopt the current estimate as key + scale. Returns 1 if applied, 0 if no estimate yet, -1 on error.
TAPTOOLS_API int    taptools_tune_autokey_apply(taptools_tune h);
TAPTOOLS_API int    taptools_tune_note_on(taptools_tune h, int note);
TAPTOOLS_API int    taptools_tune_note_off(taptools_tune h, int note);
TAPTOOLS_API int    taptools_tune_notes_off(taptools_tune h);
TAPTOOLS_API double taptools_tune_detected_hz(taptools_tune h);
TAPTOOLS_API double taptools_tune_target_midi(taptools_tune h);
TAPTOOLS_API double taptools_tune_applied_semitones(taptools_tune h);
TAPTOOLS_API int    taptools_tune_process(taptools_tune h, const double* in, double* out, int n);

// ---- pitch detector passthrough (tap::dsp::yin, for the notebooks' pitch tracking) ---------------

typedef void* taptools_yin;

TAPTOOLS_API taptools_yin taptools_yin_create(int window, int tau_min, int tau_max);
TAPTOOLS_API void         taptools_yin_destroy(taptools_yin h);
TAPTOOLS_API int          taptools_yin_frame_size(taptools_yin h);
/// Analyze every `hop` samples across x; writes up to max_out fractional periods in samples
/// (0 where unvoiced). Returns the number written, or -1 on error.
TAPTOOLS_API int taptools_yin_track(taptools_yin h, const double* x, int n, int hop, double* periods, int max_out);

#ifdef __cplusplus
}
#endif
