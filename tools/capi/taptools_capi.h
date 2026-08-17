/// @file taptools_capi.h
/// @brief Minimal C ABI over the portable TapTools DSP cores, for language bindings and the
///        verification notebooks (notebooks/ drive it via ctypes).
///
///        Conventions: plain C types only; the caller owns all arrays and sizes them. Handle-based
///        functions return 0 on success and -1 on any error (bad argument, unconfigured engine).
///        No global state. Exposes tap.convolve~'s conv_engine (uniformly-partitioned overlap-save
///        convolution) plus the parameter-indexed kernels behind tap.svf~, tap.ladder~, tap.vco~,
///        tap.autowah~, and tap.overdrive~ (param indices and mode/solver/waveform constants match
///        the enums in each kernel header; ..._set() takes the kernel's param_index).
// SPDX-License-Identifier: MIT
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

// ---- tap.harmony~ (tap::tools::harmony::harmonizer) ----------------------------------------------

typedef void* taptools_harmonizer;

TAPTOOLS_API taptools_harmonizer taptools_harmonizer_create(void);
TAPTOOLS_API void                taptools_harmonizer_destroy(taptools_harmonizer h);
TAPTOOLS_API int                 taptools_harmonizer_prepare(taptools_harmonizer h, double sr, int fft_size);
TAPTOOLS_API int                 taptools_harmonizer_clear(taptools_harmonizer h);
TAPTOOLS_API int                 taptools_harmonizer_set_interval(taptools_harmonizer h, int voice, double st);
TAPTOOLS_API int                 taptools_harmonizer_set_gain(taptools_harmonizer h, int voice, double gain);
TAPTOOLS_API int                 taptools_harmonizer_set_dry(taptools_harmonizer h, double gain);
TAPTOOLS_API int                 taptools_harmonizer_set_formant(taptools_harmonizer h, int on);
TAPTOOLS_API int                 taptools_harmonizer_set_glide(taptools_harmonizer h, double ms);
TAPTOOLS_API int                 taptools_harmonizer_latency(taptools_harmonizer h);
TAPTOOLS_API int taptools_harmonizer_process(taptools_harmonizer h, const double* in, double* out, int n);

// ---- tap.adsr~ (tap::tools::adsr::generator) -----------------------------------------------------

typedef void* taptools_adsr;

TAPTOOLS_API taptools_adsr taptools_adsr_create(void);
TAPTOOLS_API void          taptools_adsr_destroy(taptools_adsr h);
TAPTOOLS_API int           taptools_adsr_prepare(taptools_adsr h, double sr);
TAPTOOLS_API int           taptools_adsr_clear(taptools_adsr h);
TAPTOOLS_API int           taptools_adsr_set_attack(taptools_adsr h, double ms);
TAPTOOLS_API int           taptools_adsr_set_decay(taptools_adsr h, double ms);
TAPTOOLS_API int           taptools_adsr_set_sustain_db(taptools_adsr h, double db);
TAPTOOLS_API int           taptools_adsr_set_release(taptools_adsr h, double ms);
TAPTOOLS_API int           taptools_adsr_set_mode(taptools_adsr h, int mode); // adsr::mode
TAPTOOLS_API int           taptools_adsr_set_threshold(taptools_adsr h, double t);
TAPTOOLS_API int           taptools_adsr_set_velocity(taptools_adsr h, double s);
TAPTOOLS_API int           taptools_adsr_process(taptools_adsr h, const double* gate, double* out, int n);

// ---- pitch detector passthrough (tap::dsp::yin, for the notebooks' pitch tracking) ---------------

typedef void* taptools_yin;

TAPTOOLS_API taptools_yin taptools_yin_create(int window, int tau_min, int tau_max);
TAPTOOLS_API void         taptools_yin_destroy(taptools_yin h);
TAPTOOLS_API int          taptools_yin_frame_size(taptools_yin h);
/// Analyze every `hop` samples across x; writes up to max_out fractional periods in samples
/// (0 where unvoiced). Returns the number written, or -1 on error.
TAPTOOLS_API int taptools_yin_track(taptools_yin h, const double* x, int n, int hop, double* periods, int max_out);

// ---- tap.delay~ (tap::tools::delay::line) --------------------------------------------------------

typedef void* taptools_delay;

TAPTOOLS_API taptools_delay taptools_delay_create(void);
TAPTOOLS_API void           taptools_delay_destroy(taptools_delay h);
/// Allocate the line for `max_ms` at `sr`; snaps ramps and clears state.
TAPTOOLS_API int taptools_delay_prepare(taptools_delay h, double sr, double max_ms);
TAPTOOLS_API int taptools_delay_set_time_ms(taptools_delay h, double ms);
TAPTOOLS_API int taptools_delay_set_feedback(taptools_delay h, double fb); // clamped to [0, 0.99]
TAPTOOLS_API int taptools_delay_set_mix(taptools_delay h, double pct);     // 0..100, equal-power
TAPTOOLS_API int taptools_delay_set_interp(taptools_delay h, int mode);    // 0 trunc, 1 Hermite
TAPTOOLS_API int taptools_delay_set_smooth_ms(taptools_delay h, double ms);
TAPTOOLS_API int taptools_delay_clear(taptools_delay h);
TAPTOOLS_API int taptools_delay_process(taptools_delay h, const double* in, double* out, int n);
/// Per-sample signal-rate delay time (ms) — the external's right-inlet path.
TAPTOOLS_API int taptools_delay_process_mod(taptools_delay h, const double* in, const double* time_ms, double* out,
                                            int n);

// ---- tap.multitap~ (tap::tools::delay::multitap) -------------------------------------------------

typedef void* taptools_multitap;

TAPTOOLS_API taptools_multitap taptools_multitap_create(void);
TAPTOOLS_API void              taptools_multitap_destroy(taptools_multitap h);
TAPTOOLS_API int               taptools_multitap_prepare(taptools_multitap h, double sr, double max_ms);
TAPTOOLS_API int               taptools_multitap_set_taps(taptools_multitap h, int count); // 0..100 active taps
/// Per-tap setters; `tap` is 0-based. Gain is linear; pan is -1 (left) .. 1 (right), equal-power.
TAPTOOLS_API int taptools_multitap_set_time_ms(taptools_multitap h, int tap, double ms);
TAPTOOLS_API int taptools_multitap_set_gain(taptools_multitap h, int tap, double gain);
TAPTOOLS_API int taptools_multitap_set_pan(taptools_multitap h, int tap, double pan);
TAPTOOLS_API int taptools_multitap_set_interp(taptools_multitap h, int mode); // 0 trunc, 1 Hermite
TAPTOOLS_API int taptools_multitap_set_smooth_ms(taptools_multitap h, double ms);
TAPTOOLS_API int taptools_multitap_clear(taptools_multitap h);
/// Process n samples; the stereo tap sum lands in outL/outR (no dry path).
TAPTOOLS_API int taptools_multitap_process(taptools_multitap h, const double* in, double* outL, double* outR, int n);

// ---- tap.overdrive~ (tap::tools::od::overdrive, mono) --------------------------------------------

typedef void* taptools_od;

TAPTOOLS_API taptools_od taptools_od_create(void);
TAPTOOLS_API void        taptools_od_destroy(taptools_od h);
TAPTOOLS_API int         taptools_od_prepare(taptools_od h, double sr);
TAPTOOLS_API int         taptools_od_set(taptools_od h, int param, double value); // od::param_index
TAPTOOLS_API int         taptools_od_set_oversample(taptools_od h, int os);       // 1, 2, 4, or 8
TAPTOOLS_API int         taptools_od_set_smooth_ms(taptools_od h, double ms);
TAPTOOLS_API int         taptools_od_clear(taptools_od h);
TAPTOOLS_API int         taptools_od_process(taptools_od h, const double* in, double* out, int n);

// ---- tap.discreet~ (tap::tools::discreet::machine) -----------------------------------------------

typedef void* taptools_discreet;

TAPTOOLS_API taptools_discreet taptools_discreet_create(void);
TAPTOOLS_API void              taptools_discreet_destroy(taptools_discreet h);
/// Buy tape for `max_loop_seconds` at `sr`; snaps ramps and erases the tape.
TAPTOOLS_API int taptools_discreet_prepare(taptools_discreet h, double sr, double max_loop_seconds);
TAPTOOLS_API int taptools_discreet_set_loop_seconds(taptools_discreet h, double s);  // slewed: tape-speed doppler
TAPTOOLS_API int taptools_discreet_set_regen(taptools_discreet h, double r);         // 0..1; 1.0 legally sustains
TAPTOOLS_API int taptools_discreet_set_darken_hz(taptools_discreet h, double hz);    // per-pass wear corner
TAPTOOLS_API int taptools_discreet_set_drive(taptools_discreet h, double d);         // >= 0; 0 exactly linear
TAPTOOLS_API int taptools_discreet_set_input_level(taptools_discreet h, double lin); // the send fader
TAPTOOLS_API int taptools_discreet_set_mix(taptools_discreet h, double pct);         // 0..100, equal-power
TAPTOOLS_API int taptools_discreet_set_wow(taptools_discreet h, double depth_ms, double rate_hz);
TAPTOOLS_API int taptools_discreet_set_flutter(taptools_discreet h, double depth_ms, double rate_hz);
TAPTOOLS_API int taptools_discreet_set_smooth_ms(taptools_discreet h, double ms);
TAPTOOLS_API int taptools_discreet_clear(taptools_discreet h);
TAPTOOLS_API int taptools_discreet_process(taptools_discreet h, const double* in, double* out, int n);

// ---- tap.tapecho~ (tap::tools::tapecho::machine) -------------------------------------------------

typedef void* taptools_tapecho;

TAPTOOLS_API taptools_tapecho taptools_tapecho_create(void);
TAPTOOLS_API void             taptools_tapecho_destroy(taptools_tapecho h);
/// Buy tape for `max_span_seconds` at `sr`; snaps ramps and erases the tape.
TAPTOOLS_API int taptools_tapecho_prepare(taptools_tapecho h, double sr, double max_span_seconds);
TAPTOOLS_API int taptools_tapecho_set_span_ms(taptools_tapecho h, double ms); // the motor; slewed = varispeed
TAPTOOLS_API int taptools_tapecho_set_heads(taptools_tapecho h, int count);   // 0..4 active playback heads
/// Per-head setters; `head` is 0-based. `ratio` is the head's position as a fraction of the span.
TAPTOOLS_API int taptools_tapecho_set_head_ratio(taptools_tapecho h, int head, double ratio); // (0, 1]
TAPTOOLS_API int taptools_tapecho_set_head_level(taptools_tapecho h, int head, double lin);
TAPTOOLS_API int taptools_tapecho_set_head_pan(taptools_tapecho h, int head, double pan); // -1..1 equal-power
/// 0..1.5. Above 1.0 self-oscillates and is only reached while drive > 0 (the saturator bounds it).
TAPTOOLS_API int taptools_tapecho_set_regen(taptools_tapecho h, double r);
TAPTOOLS_API int taptools_tapecho_set_darken_hz(taptools_tapecho h, double hz);    // per-pass wear corner
TAPTOOLS_API int taptools_tapecho_set_drive(taptools_tapecho h, double d);         // >= 0; 0 caps regen at 1.0
TAPTOOLS_API int taptools_tapecho_set_input_level(taptools_tapecho h, double lin); // into the record head
TAPTOOLS_API int taptools_tapecho_set_mix(taptools_tapecho h, double pct);         // 0..100, equal-power
TAPTOOLS_API int taptools_tapecho_set_wow(taptools_tapecho h, double depth_ms, double rate_hz);
TAPTOOLS_API int taptools_tapecho_set_flutter(taptools_tapecho h, double depth_ms, double rate_hz);
TAPTOOLS_API int taptools_tapecho_set_smooth_ms(taptools_tapecho h, double ms);
TAPTOOLS_API int taptools_tapecho_clear(taptools_tapecho h);
/// Process n samples mono-in / stereo-out (the dry path is mixed to both busses).
TAPTOOLS_API int taptools_tapecho_process(taptools_tapecho h, const double* in, double* outL, double* outR, int n);

// ---- tap.transducer (tap::tools::diffuseur::transducer) ------------------------------------------

/// The diffuseurs' moving-iron driver on its own — a component, not an external. Reachable here
/// because the family's rule is that parts get C ABI reachability from the start, and because
/// measuring the driver through a body means measuring the body too.
typedef void* taptools_transducer;

TAPTOOLS_API taptools_transducer taptools_transducer_create(void);
TAPTOOLS_API void                taptools_transducer_destroy(taptools_transducer h);
TAPTOOLS_API int                 taptools_transducer_prepare(taptools_transducer h, double sr);
TAPTOOLS_API int                 taptools_transducer_set_drive(taptools_transducer h, double lin);
TAPTOOLS_API int                 taptools_transducer_set_asymmetry(taptools_transducer h, double a);
TAPTOOLS_API int                 taptools_transducer_set_saturation(taptools_transducer h, double s);
TAPTOOLS_API int                 taptools_transducer_clear(taptools_transducer h);
TAPTOOLS_API int taptools_transducer_process(taptools_transducer h, const double* in, double* out, int n);

// ---- the Ondes Martenot tube model (tap::tools::ondes) -------------------------------------------

/// The enhanced Norman Koren law itself, with the circuit paper's fitted parameter sets selected
/// by index (0 = 6F5, 1 = 6C5, 2 = 2A3). Stateless, so no handle: a notebook plotting the
/// published tube curves should be plotting the shipping code, not a copy of the equations.
TAPTOOLS_API double taptools_tube_plate_current(int tube, double vpc, double vgc);
TAPTOOLS_API double taptools_tube_grid_current(int tube, double vgc);
/// The fitted parameters, so a plot can be labelled with the numbers it was drawn from. Fields in
/// Table II order: mu, Ex, Kg, Kp, Kvb, Vct, Va, Rgk. Returns 0 on a bad index, 8 on success.
TAPTOOLS_API int taptools_tube_params(int tube, double* out8);

// ---- tap.triode~ (tap::tools::ondes::triode) -----------------------------------------------------

typedef void* taptools_triode;

TAPTOOLS_API taptools_triode taptools_triode_create(void);
TAPTOOLS_API void            taptools_triode_destroy(taptools_triode h);
TAPTOOLS_API int             taptools_triode_prepare(taptools_triode h, double sr);
TAPTOOLS_API int             taptools_triode_set_tube(taptools_triode h, int tube);
/// Supply volts, cathode resistor and plate load — the published stage operating points are
/// (100, 1000, 4000) demodulator, (180, 1000, 4000) preamplifier, (230, 750, 1500) power.
TAPTOOLS_API int taptools_triode_set_operating_point(taptools_triode h, double vbias, double rk, double rp);
TAPTOOLS_API int taptools_triode_set_drive(taptools_triode h, double grid_volts);
TAPTOOLS_API int taptools_triode_set_corners(taptools_triode h, double hp_hz, double lp_hz);
TAPTOOLS_API int taptools_triode_clear(taptools_triode h);
TAPTOOLS_API int taptools_triode_process(taptools_triode h, const double* in, double* out, int n);
/// The stage's static curve without running audio: normalized in, normalized out.
TAPTOOLS_API double taptools_triode_curve_at(taptools_triode h, double x);
/// And in the tube's own units: grid volts in, plate volts of swing out.
TAPTOOLS_API double taptools_triode_plate_swing_at(taptools_triode h, double grid_volts);
/// The quiescent point the curve was built around: cathode bias V, plate V, plate current A, and
/// the small-signal gain magnitude.
TAPTOOLS_API double taptools_triode_bias_v(taptools_triode h);
TAPTOOLS_API double taptools_triode_quiescent_plate_v(taptools_triode h);
TAPTOOLS_API double taptools_triode_quiescent_current_a(taptools_triode h);
TAPTOOLS_API double taptools_triode_gain(taptools_triode h);

// ---- the heterodyne detector (tap::tools::ondes::detector) ---------------------------------------

typedef void* taptools_detector;

TAPTOOLS_API taptools_detector taptools_detector_create(void);
TAPTOOLS_API void              taptools_detector_destroy(taptools_detector h);
TAPTOOLS_API int               taptools_detector_prepare(taptools_detector h, double sr);
TAPTOOLS_API int               taptools_detector_set_frequency(taptools_detector h, double hz);
TAPTOOLS_API int               taptools_detector_set_ribbon(taptools_detector h, double semitones);
TAPTOOLS_API int               taptools_detector_set_depth(taptools_detector h, double d);
TAPTOOLS_API int               taptools_detector_set_detect_ms(taptools_detector h, double ms);
TAPTOOLS_API int               taptools_detector_clear(taptools_detector h);
/// A source: render n samples of the detected envelope (DC included — the circuit's coupling
/// removes it downstream, and so does the triode stage's conditioning highpass).
TAPTOOLS_API int taptools_detector_process(taptools_detector h, double* out, int n);
/// The ideal envelope at a phase in [0, 1), with no detector on it — the closed form itself.
TAPTOOLS_API double taptools_detector_envelope_at(taptools_detector h, double phase);

// ---- tap.ondes~ (tap::tools::ondes::voice) -------------------------------------------------------

typedef void* taptools_ondes;

TAPTOOLS_API taptools_ondes taptools_ondes_create(void);
TAPTOOLS_API void           taptools_ondes_destroy(taptools_ondes h);
TAPTOOLS_API int            taptools_ondes_prepare(taptools_ondes h, double sr);
TAPTOOLS_API int            taptools_ondes_set_ribbon(taptools_ondes h, double semitones); // above A1
TAPTOOLS_API int            taptools_ondes_set_frequency(taptools_ondes h, double hz);
TAPTOOLS_API int            taptools_ondes_set_depth(taptools_ondes h, double d);      // 0..1
TAPTOOLS_API int            taptools_ondes_set_detect_ms(taptools_ondes h, double ms); // published 0.2
TAPTOOLS_API int            taptools_ondes_set_drive(taptools_ondes h, double x);      // 0..8
TAPTOOLS_API int            taptools_ondes_set_key(taptools_ondes h, double position); // 0..1 of the travel
TAPTOOLS_API int            taptools_ondes_set_key_mm(taptools_ondes h, double mm);
TAPTOOLS_API int            taptools_ondes_set_key_placement(taptools_ondes h, int where); // 0 after, 1 before
TAPTOOLS_API int            taptools_ondes_set_power_stage(taptools_ondes h, int on);
TAPTOOLS_API int            taptools_ondes_set_polarity(taptools_ondes h, int sign);
TAPTOOLS_API int            taptools_ondes_set_level(taptools_ondes h, double lin);
TAPTOOLS_API int            taptools_ondes_set_oversample(taptools_ondes h, int os);
TAPTOOLS_API int            taptools_ondes_set_smooth_ms(taptools_ondes h, double ms);
TAPTOOLS_API int            taptools_ondes_clear(taptools_ondes h);
/// A source: render n samples of the instrument, minus its diffuseur.
TAPTOOLS_API int taptools_ondes_process(taptools_ondes h, double* out, int n);
/// Signal-rate performance: the ribbon in semitones and the key position, per sample.
TAPTOOLS_API int taptools_ondes_process_mod(taptools_ondes h, const double* semitones, const double* key, double* out,
                                            int n);
TAPTOOLS_API double taptools_ondes_frequency(taptools_ondes h);

// ---- tap.plate (tap::tools::diffuseur::plate) ----------------------------------------------------

/// The metallique's body on its own — the mode bank without the driver in front of it. Same
/// reason as the transducer above: the parts are reachable so a measurement of one is not
/// silently a measurement of both.
typedef void* taptools_plate;

TAPTOOLS_API taptools_plate taptools_plate_create(void);
TAPTOOLS_API void           taptools_plate_destroy(taptools_plate h);
TAPTOOLS_API int            taptools_plate_prepare(taptools_plate h, double sr);
TAPTOOLS_API int            taptools_plate_set_pitch_hz(taptools_plate h, double hz);
TAPTOOLS_API int            taptools_plate_set_decay(taptools_plate h, double t60_s);
TAPTOOLS_API int            taptools_plate_set_tilt(taptools_plate h, double tilt);
TAPTOOLS_API int            taptools_plate_set_brightness(taptools_plate h, double b);
TAPTOOLS_API int            taptools_plate_clear(taptools_plate h);
TAPTOOLS_API int            taptools_plate_process(taptools_plate h, const double* in, double* out, int n);
TAPTOOLS_API double         taptools_plate_mode_hz(taptools_plate h, int mode);
TAPTOOLS_API double         taptools_plate_mode_level(taptools_plate h, int mode);

// ---- tap.metallique~ (tap::tools::diffuseur::metallique) -----------------------------------------

typedef void* taptools_metallique;

TAPTOOLS_API taptools_metallique taptools_metallique_create(void);
TAPTOOLS_API void                taptools_metallique_destroy(taptools_metallique h);
TAPTOOLS_API int                 taptools_metallique_prepare(taptools_metallique h, double sr);
TAPTOOLS_API int                 taptools_metallique_set_pitch_hz(taptools_metallique h, double hz);
TAPTOOLS_API int                 taptools_metallique_set_decay(taptools_metallique h, double t60_s);
/// Upper modes decay by ratio^tilt faster than the fundamental.
TAPTOOLS_API int taptools_metallique_set_tilt(taptools_metallique h, double tilt);
TAPTOOLS_API int taptools_metallique_set_brightness(taptools_metallique h, double b); // 0..1
TAPTOOLS_API int taptools_metallique_set_drive(taptools_metallique h, double lin);
TAPTOOLS_API int taptools_metallique_set_asymmetry(taptools_metallique h, double a);  // 0..1, moving-iron squared term
TAPTOOLS_API int taptools_metallique_set_saturation(taptools_metallique h, double s); // 0 is exactly linear
TAPTOOLS_API int taptools_metallique_set_mix(taptools_metallique h, double pct);      // 0..100, equal-power
TAPTOOLS_API int taptools_metallique_set_level(taptools_metallique h, double lin);
TAPTOOLS_API int taptools_metallique_set_smooth_ms(taptools_metallique h, double ms);
TAPTOOLS_API int taptools_metallique_clear(taptools_metallique h);
TAPTOOLS_API int taptools_metallique_process(taptools_metallique h, const double* in, double* out, int n);
/// The body's tuning, so a notebook can plot where the modes landed (NaN on a bad handle).
TAPTOOLS_API double taptools_metallique_mode_hz(taptools_metallique h, int mode);
TAPTOOLS_API double taptools_metallique_mode_level(taptools_metallique h, int mode);

// ---- tap.palme~ (tap::tools::diffuseur::palme) ---------------------------------------------------

typedef void* taptools_palme;

TAPTOOLS_API taptools_palme taptools_palme_create(void);
TAPTOOLS_API void           taptools_palme_destroy(taptools_palme h);
TAPTOOLS_API int            taptools_palme_prepare(taptools_palme h, double sr);
TAPTOOLS_API int            taptools_palme_set_root_hz(taptools_palme h, double hz);
TAPTOOLS_API int            taptools_palme_set_tuning(taptools_palme h, int tuning); // 0 chromatic, 1 harmonic
TAPTOOLS_API int            taptools_palme_set_decay(taptools_palme h, double t60_s);
TAPTOOLS_API int            taptools_palme_set_damping(taptools_palme h, double hz);
TAPTOOLS_API int            taptools_palme_set_detune(taptools_palme h, double cents);
TAPTOOLS_API int            taptools_palme_set_drive(taptools_palme h, double lin);
TAPTOOLS_API int            taptools_palme_set_asymmetry(taptools_palme h, double a);
TAPTOOLS_API int            taptools_palme_set_saturation(taptools_palme h, double s);
TAPTOOLS_API int            taptools_palme_set_mix(taptools_palme h, double pct);
TAPTOOLS_API int            taptools_palme_set_level(taptools_palme h, double lin);
TAPTOOLS_API int            taptools_palme_set_smooth_ms(taptools_palme h, double ms);
TAPTOOLS_API int            taptools_palme_clear(taptools_palme h);
TAPTOOLS_API int            taptools_palme_process(taptools_palme h, const double* in, double* out, int n);
/// Where a string ended up after tuning and scatter, in Hz (NaN on a bad handle).
TAPTOOLS_API double taptools_palme_string_hz(taptools_palme h, int index);
/// The loop gain a string settled on — the cap is visible here when damping and ring time fight.
TAPTOOLS_API double taptools_palme_string_feedback(taptools_palme h, int index);

// ---- tap.scrub~ (tap::tools::scrub::machine) -----------------------------------------------------

typedef void* taptools_scrub;

TAPTOOLS_API taptools_scrub taptools_scrub_create(void);
TAPTOOLS_API void           taptools_scrub_destroy(taptools_scrub h);
TAPTOOLS_API int            taptools_scrub_prepare(taptools_scrub h, double sr, double max_history_ms);
TAPTOOLS_API int            taptools_scrub_set_position_ms(taptools_scrub h, double ms); // lag behind the live edge
TAPTOOLS_API int            taptools_scrub_set_pitch(taptools_scrub h, double semitones);
TAPTOOLS_API int            taptools_scrub_set_drift(taptools_scrub h, double rate); // playback-rate units
TAPTOOLS_API int            taptools_scrub_set_freeze(taptools_scrub h, int on);
TAPTOOLS_API int            taptools_scrub_set_size_ms(taptools_scrub h, double ms);
TAPTOOLS_API int            taptools_scrub_set_overlap(taptools_scrub h, int n); // 1..4
TAPTOOLS_API int            taptools_scrub_set_spray_ms(taptools_scrub h, double ms);
TAPTOOLS_API int            taptools_scrub_set_seed(taptools_scrub h, unsigned long long seed);
TAPTOOLS_API int            taptools_scrub_set_mix(taptools_scrub h, double pct);
TAPTOOLS_API int            taptools_scrub_set_level(taptools_scrub h, double lin);
TAPTOOLS_API int            taptools_scrub_set_smooth_ms(taptools_scrub h, double ms);
TAPTOOLS_API int            taptools_scrub_clear(taptools_scrub h);
TAPTOOLS_API int            taptools_scrub_process(taptools_scrub h, const double* in, double* out, int n);
/// Signal-rate performance path: position (ms behind the edge) and pitch (semitones) per sample.
TAPTOOLS_API int taptools_scrub_process_mod(taptools_scrub h, const double* in, const double* position_ms,
                                            const double* pitch_st, double* out, int n);
TAPTOOLS_API int taptools_scrub_active_grains(taptools_scrub h); // -1 on a bad handle

// ---- tap.touche~ (tap::tools::touche::key) -------------------------------------------------------

typedef void* taptools_touche;

TAPTOOLS_API taptools_touche taptools_touche_create(void);
TAPTOOLS_API void            taptools_touche_destroy(taptools_touche h);
TAPTOOLS_API int             taptools_touche_prepare(taptools_touche h, double sr);
TAPTOOLS_API int             taptools_touche_set_position(taptools_touche h, double p);     // 0..1 of the travel
TAPTOOLS_API int             taptools_touche_set_position_mm(taptools_touche h, double mm); // published units
TAPTOOLS_API int             taptools_touche_set_force_n(taptools_touche h, double n);
TAPTOOLS_API int             taptools_touche_set_mode(taptools_touche h, int mode); // 0 displacement, 1 force
TAPTOOLS_API int             taptools_touche_set_smooth_ms(taptools_touche h, double ms);
TAPTOOLS_API int             taptools_touche_clear(taptools_touche h);
/// The curve itself: linear gain at a normalized position (NaN on a bad handle). No state touched.
TAPTOOLS_API double taptools_touche_gain_at(taptools_touche h, double p);
TAPTOOLS_API int    taptools_touche_process(taptools_touche h, const double* in, double* out, int n);
/// Signal-rate position: `position` drives the gain sample by sample.
TAPTOOLS_API int taptools_touche_process_mod(taptools_touche h, const double* in, const double* position, double* out,
                                             int n);

// ---- tap.fuzz~ (tap::tools::fuzz::pedal) ---------------------------------------------------------

typedef void* taptools_fuzz;

TAPTOOLS_API taptools_fuzz taptools_fuzz_create(void);
TAPTOOLS_API void          taptools_fuzz_destroy(taptools_fuzz h);
TAPTOOLS_API int           taptools_fuzz_prepare(taptools_fuzz h, double sr);
TAPTOOLS_API int           taptools_fuzz_set_gain(taptools_fuzz h, double g);      // 0..1, first-stage drive
TAPTOOLS_API int           taptools_fuzz_set_edge(taptools_fuzz h, double e);      // 0..1, second-stage knee
TAPTOOLS_API int           taptools_fuzz_set_asymmetry(taptools_fuzz h, double a); // 0..1, the even harmonics
TAPTOOLS_API int           taptools_fuzz_set_bass(taptools_fuzz h, double b);      // -1..1 low shelf
TAPTOOLS_API int           taptools_fuzz_set_treble(taptools_fuzz h, double t);    // -1..1 high shelf
TAPTOOLS_API int           taptools_fuzz_set_contrast(taptools_fuzz h, double c);  // 0..1 mid scoop
TAPTOOLS_API int           taptools_fuzz_set_level_db(taptools_fuzz h, double db);
TAPTOOLS_API int           taptools_fuzz_set_oversample(taptools_fuzz h, int os); // 1, 2, 4 or 8
TAPTOOLS_API int           taptools_fuzz_set_smooth_ms(taptools_fuzz h, double ms);
TAPTOOLS_API int           taptools_fuzz_clear(taptools_fuzz h);
TAPTOOLS_API int           taptools_fuzz_process(taptools_fuzz h, const double* in, double* out, int n);

// ---- tap.stammer~ (tap::tools::stammer::machine) -------------------------------------------------

typedef void* taptools_stammer;

TAPTOOLS_API taptools_stammer taptools_stammer_create(void);
TAPTOOLS_API void             taptools_stammer_destroy(taptools_stammer h);
/// Buy the capture for `max_history_ms` at `sr`; resets the grid and re-seeds.
TAPTOOLS_API int taptools_stammer_prepare(taptools_stammer h, double sr, double max_history_ms);
TAPTOOLS_API int taptools_stammer_set_step_ms(taptools_stammer h, double ms); // the grid
TAPTOOLS_API int taptools_stammer_set_density(taptools_stammer h, double p);  // 0..1; 0 never rolls
TAPTOOLS_API int taptools_stammer_set_divisions(taptools_stammer h, int n);   // slice = step / [1, n]
TAPTOOLS_API int taptools_stammer_set_repeats(taptools_stammer h, int n);     // passes per fired slice
TAPTOOLS_API int taptools_stammer_set_reverse(taptools_stammer h, double p);  // 0..1, drawn per repeat
TAPTOOLS_API int taptools_stammer_set_jump_ms(taptools_stammer h, double ms); // extra reach-back
TAPTOOLS_API int taptools_stammer_set_fade_ms(taptools_stammer h, double ms); // per-repeat flank
TAPTOOLS_API int taptools_stammer_set_seed(taptools_stammer h, unsigned long long seed);
TAPTOOLS_API int taptools_stammer_set_input_level(taptools_stammer h, double lin);
TAPTOOLS_API int taptools_stammer_set_mix(taptools_stammer h, double pct); // only bites while firing
TAPTOOLS_API int taptools_stammer_set_smooth_ms(taptools_stammer h, double ms);
TAPTOOLS_API int taptools_stammer_clear(taptools_stammer h);
/// 1 while a slice is sounding, else 0 (-1 on a bad handle).
TAPTOOLS_API int taptools_stammer_playing(taptools_stammer h);
TAPTOOLS_API int taptools_stammer_process(taptools_stammer h, const double* in, double* out, int n);

// ---- tap.airport~ (tap::tools::airport::loop_bank) -----------------------------------------------

typedef void* taptools_airport;

TAPTOOLS_API taptools_airport taptools_airport_create(void);
TAPTOOLS_API void             taptools_airport_destroy(taptools_airport h);
/// Buy k_max_loops (8) reels for `max_loop_seconds` at `sr`; erases tape and rewinds heads.
TAPTOOLS_API int taptools_airport_prepare(taptools_airport h, double sr, double max_loop_seconds);
TAPTOOLS_API int taptools_airport_set_loops(taptools_airport h, int count); // 0..8 active loops
/// Per-loop setters; `loop` is 0-based. A length change is a splice (head re-wraps, no rewind).
TAPTOOLS_API int taptools_airport_set_length_seconds(taptools_airport h, int loop, double s);
TAPTOOLS_API int taptools_airport_record(taptools_airport h, int loop, int on); // 1 punch, 0 freeze
TAPTOOLS_API int taptools_airport_set_level(taptools_airport h, int loop, double lin);
TAPTOOLS_API int taptools_airport_set_pan(taptools_airport h, int loop, double pan); // -1..1 equal-power
TAPTOOLS_API int taptools_airport_set_darken_hz(taptools_airport h, int loop, double hz);
TAPTOOLS_API int taptools_airport_set_smooth_ms(taptools_airport h, double ms);
TAPTOOLS_API int taptools_airport_clear(taptools_airport h);
/// This loop's head position as a fraction of its length, 0..1 (-1 on a bad handle/index).
TAPTOOLS_API double taptools_airport_phase(taptools_airport h, int loop);
/// lcm of the active loop lengths in seconds; +inf on 64-bit overflow; 0 if unprepared.
TAPTOOLS_API double taptools_airport_composite_period_seconds(taptools_airport h);
/// Process n samples; the stereo loop sum lands in outL/outR (no dry path).
TAPTOOLS_API int taptools_airport_process(taptools_airport h, const double* in, double* outL, double* outR, int n);

// ---- tap.garden~ (tap::tools::garden::bed) -------------------------------------------------------

typedef void* taptools_garden;

TAPTOOLS_API taptools_garden taptools_garden_create(void);
TAPTOOLS_API void            taptools_garden_destroy(taptools_garden h);
TAPTOOLS_API int             taptools_garden_prepare(taptools_garden h, double sr);
/// Plant a note: MIDI pitch (fractional ok, snaps to root/scale at entry), velocity (0, 1].
TAPTOOLS_API int taptools_garden_note(taptools_garden h, double pitch, double velocity);
TAPTOOLS_API int taptools_garden_set_loop_seconds(taptools_garden h, double s);
TAPTOOLS_API int taptools_garden_set_decay(taptools_garden h, double per_pass);  // velocity/pass, 0..1
TAPTOOLS_API int taptools_garden_set_soften(taptools_garden h, double per_pass); // brightness/pass, 0..1
TAPTOOLS_API int taptools_garden_set_floor(taptools_garden h, double v);         // retirement threshold
/// Bell envelope times in SECONDS + base brightness 0..1 (the upper modes' weight).
TAPTOOLS_API int taptools_garden_set_bell(taptools_garden h, double attack_s, double decay_s, double brightness);
TAPTOOLS_API int taptools_garden_set_material(taptools_garden h, int material); // garden::material_index
TAPTOOLS_API int taptools_garden_set_spread(taptools_garden h, double amount);  // rack width, 0 mono .. 1
TAPTOOLS_API int taptools_garden_set_root(taptools_garden h, int semitone);     // 0..11, 0 = C
TAPTOOLS_API int taptools_garden_set_scale(taptools_garden h, int scale);       // garden::scale_index
TAPTOOLS_API int taptools_garden_set_idle_seconds(taptools_garden h, double s); // 0 disables the gardener
TAPTOOLS_API int taptools_garden_set_gust(taptools_garden h, double amount);    // wind: 0 even, 1 blustery
TAPTOOLS_API int taptools_garden_set_seed(taptools_garden h, unsigned long long seed);
TAPTOOLS_API int taptools_garden_set_level(taptools_garden h, double lin);
TAPTOOLS_API int taptools_garden_set_smooth_ms(taptools_garden h, double ms);
TAPTOOLS_API int taptools_garden_clear(taptools_garden h);
TAPTOOLS_API int taptools_garden_active_events(taptools_garden h); // live blooms (-1 on bad handle)
TAPTOOLS_API int taptools_garden_active_voices(taptools_garden h); // ringing bells (-1 on bad handle)
/// A source: renders n samples of the stereo rack into outL/outR.
TAPTOOLS_API int taptools_garden_process(taptools_garden h, double* outL, double* outR, int n);

// ---- the components the monoliths are made of ----------------------------------------------------
//
// tap.airport~ is a sum of tap.reel~ lanes; tap.garden~ is tap.scale into tap.bloom into
// tap.chime~ with tap.gardener planting when idle. These entry points reach the same classes the
// monoliths hold, so a notebook can null-test the patch against the object through one ABI.

// ---- tap.reel~ (tap::tools::airport::loop) -------------------------------------------------------

typedef void* taptools_reel;

TAPTOOLS_API taptools_reel taptools_reel_create(void);
TAPTOOLS_API void          taptools_reel_destroy(taptools_reel h);

TAPTOOLS_API int taptools_reel_prepare(taptools_reel h, double sr, double max_loop_seconds);
TAPTOOLS_API int taptools_reel_set_length_seconds(taptools_reel h, double s);
TAPTOOLS_API int taptools_reel_record(taptools_reel h, int on); // 1 punch, 0 freeze
TAPTOOLS_API int taptools_reel_set_level(taptools_reel h, double lin);
TAPTOOLS_API int taptools_reel_set_pan(taptools_reel h, double pan); // -1..1 equal-power
TAPTOOLS_API int taptools_reel_set_darken_hz(taptools_reel h, double hz);
TAPTOOLS_API int taptools_reel_set_smooth_ms(taptools_reel h, double ms);
TAPTOOLS_API int taptools_reel_clear(taptools_reel h);

TAPTOOLS_API double taptools_reel_phase(taptools_reel h);          // 0..1 (-1 on bad handle)
TAPTOOLS_API double taptools_reel_length_seconds(taptools_reel h); // as quantized to samples
TAPTOOLS_API int    taptools_reel_loop_samples(taptools_reel h);   // what the lcm arithmetic reads

/// Renders n samples. ASSIGNS outL/outR (the lane's block form), so this is one reel on its own.
TAPTOOLS_API int taptools_reel_process(taptools_reel h, const double* in, double* outL, double* outR, int n);

// ---- tap.chime~ (tap::tools::garden::rack) -------------------------------------------------------

typedef void* taptools_chime;

TAPTOOLS_API taptools_chime taptools_chime_create(void);
TAPTOOLS_API void           taptools_chime_destroy(taptools_chime h);

TAPTOOLS_API int taptools_chime_prepare(taptools_chime h, double sr);
TAPTOOLS_API int taptools_chime_set_times(taptools_chime h, double attack_s, double decay_s);
TAPTOOLS_API int taptools_chime_set_material(taptools_chime h, int material); // garden::material_index
TAPTOOLS_API int taptools_chime_set_spread(taptools_chime h, double amount);  // 0 mono .. 1
TAPTOOLS_API int taptools_chime_clear(taptools_chime h);

/// Strike a tube: MIDI pitch (fractional accepted) or a raw frequency. Velocity and brightness 0..1.
TAPTOOLS_API int taptools_chime_strike(taptools_chime h, double pitch, double velocity, double brightness);
TAPTOOLS_API int taptools_chime_strike_hz(taptools_chime h, double freq_hz, double velocity, double brightness);

TAPTOOLS_API int taptools_chime_active_voices(taptools_chime h); // ringing bells (-1 on bad handle)

/// A source: renders n samples of the stereo rack, ASSIGNING outL/outR.
TAPTOOLS_API int taptools_chime_process(taptools_chime h, double* outL, double* outR, int n);

// ---- tap.bloom (tap::tools::garden::ring) --------------------------------------------------------

typedef void* taptools_bloom;

TAPTOOLS_API taptools_bloom taptools_bloom_create(void);
TAPTOOLS_API void           taptools_bloom_destroy(taptools_bloom h);

TAPTOOLS_API int taptools_bloom_prepare(taptools_bloom h, double sr);
TAPTOOLS_API int taptools_bloom_set_loop_seconds(taptools_bloom h, double s);
TAPTOOLS_API int taptools_bloom_set_decay(taptools_bloom h, double per_pass);
TAPTOOLS_API int taptools_bloom_set_soften(taptools_bloom h, double per_pass);
TAPTOOLS_API int taptools_bloom_set_floor(taptools_bloom h, double v);
TAPTOOLS_API int taptools_bloom_set_brightness(taptools_bloom h, double b);
TAPTOOLS_API int taptools_bloom_clear(taptools_bloom h);

/// Plant at the current loop position; it fires on the next due() and every pass after.
TAPTOOLS_API int taptools_bloom_plant(taptools_bloom h, double pitch, double velocity);

/// The strikes due on this sample, written into caller arrays of at least `max` entries. Returns
/// how many were written, or -1 on a bad handle. Does NOT advance the loop — call step() after.
TAPTOOLS_API int taptools_bloom_due(taptools_bloom h, double* pitch, double* velocity, double* brightness, int max);
TAPTOOLS_API int taptools_bloom_step(taptools_bloom h);

TAPTOOLS_API int taptools_bloom_active_events(taptools_bloom h); // live blooms (-1 on bad handle)
TAPTOOLS_API int taptools_bloom_loop_samples(taptools_bloom h);

// ---- tap.gardener (tap::tools::garden::gardener) -------------------------------------------------

typedef void* taptools_gardener;

TAPTOOLS_API taptools_gardener taptools_gardener_create(void);
TAPTOOLS_API void              taptools_gardener_destroy(taptools_gardener h);

TAPTOOLS_API int taptools_gardener_prepare(taptools_gardener h, double sr);
TAPTOOLS_API int taptools_gardener_set_idle_seconds(taptools_gardener h, double s); // 0 disables
TAPTOOLS_API int taptools_gardener_set_gust(taptools_gardener h, double amount);
TAPTOOLS_API int taptools_gardener_set_seed(taptools_gardener h, unsigned long long seed);
TAPTOOLS_API int taptools_gardener_notice_plant(taptools_gardener h); // a caller plant closes the gate
TAPTOOLS_API int taptools_gardener_clear(taptools_gardener h);

/// Advance the idle clock one sample. Returns 1 if the wind wants a strike (writing the RAW,
/// unquantized pitch and the velocity), 0 if not, -1 on a bad handle.
TAPTOOLS_API int taptools_gardener_tick(taptools_gardener h, int loop_samples, double* pitch, double* velocity);

// ---- tap.scale (tap::tools::garden::scale_quantizer) ---------------------------------------------

/// Stateless enough to need no handle: snap MIDI semitones to the nearest pitch in root/scale.
TAPTOOLS_API double taptools_scale_quantize(double pitch, int root, int scale);

// ---- per-voice taps (tap.chime.voices~) ----------------------------------------------------------

/// Render n samples of each of the rack's voices, RAW — before each bell's seat is applied.
/// `out` is voice-major and must hold voices*n doubles: voice v's block starts at out[v*n].
/// `voices` beyond the rack's pool are filled with silence. This is the same advance as
/// taptools_chime_process; take one or the other for a given span, never both.
TAPTOOLS_API int taptools_chime_process_voices(taptools_chime h, double* out, int voices, int n);

/// Which tube a voice is holding (Hz, 0 if never struck), how loudly, and the seat gains that
/// taptools_chime_process would multiply its mono sum by.
TAPTOOLS_API double taptools_chime_voice_hz(taptools_chime h, int voice);
TAPTOOLS_API double taptools_chime_voice_level(taptools_chime h, int voice);
TAPTOOLS_API double taptools_chime_voice_gain_left(taptools_chime h, int voice);
TAPTOOLS_API double taptools_chime_voice_gain_right(taptools_chime h, int voice);

// ---- tap.period (tap::tools::airport::composite_period_seconds) ----------------------------------

/// How long until a set of free-running loops realigns, in seconds — the lcm of their lengths
/// once each is quantized to the sample grid exactly as a reel would quantize it. Returns +inf
/// when the lcm leaves the 64-bit range (which incommensurate lengths reach fast, and which is
/// the point of the piece), and 0 on degenerate input.
TAPTOOLS_API double taptools_composite_period_seconds(const double* loop_seconds, int count, double sr);

#ifdef __cplusplus
}
#endif
