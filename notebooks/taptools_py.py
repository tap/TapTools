"""ctypes bridge to the TapTools C ABI, shared by the verification notebooks.

Loads build_capi/libtaptools_capi.{so,dylib,dll} relative to the kernel root
(kernel/ in the TapTools repo), building it first if missing (requires cmake
in PATH):

    cmake -B kernel/build_capi -S kernel/tools/capi
    cmake --build kernel/build_capi

The C ABI (kernel/tools/capi/) wraps the *same* portable DSP headers the Max
externals compile — so the notebooks exercise the real shipping code, not a
Python re-implementation. Exposed kernels: tap.convolve~'s conv_engine
(`Convolver`), tap.svf~ (`Svf`), tap.ladder~ (`Ladder`), tap.diode~
(`Diode`), tap.303~ (`TB303`), tap.vco~ (`Vco`), tap.autowah~ (`Wah`),
tap.overdrive~ (`Overdrive`), the step-sequencer rows behind tap.808.seq~ /
tap.303.seq~ (`TriggerRow`, `NoteRow`), tap.808.kick~ (`Kick`),
tap.delay~ (`Delay`), tap.multitap~ (`Multitap`), the Discreet Music
two-machine tape loop tap.discreet~ (`Discreet`), the multi-head tape echo
tap.tapecho~ (`TapEcho`), the live buffer-stutter rig tap.stammer~
(`Stammer`), the two-stage fuzz tap.fuzz~ (`Fuzz`), the Ondes Martenot intensity
key tap.touche~ (`Touche`), the Music for Airports
incommensurate loop bank tap.airport~ (`Airport`), the generative event
loop tap.garden~ (`Garden`), and tap.tune~'s pitch corrector (`Tune`, with
the shared DspTap detector passed through as `Yin` for the notebooks'
pitch tracking). The components those last two are built from are reachable
too — one tape lane (`Reel`), the chime rack (`Chime`), the event ring
(`Bloom`), the idle wind (`Gardener`), and the entry quantizer
(`scale_quantize`) — so a notebook can null-test the patch against the
object through one ABI. Parameter names on the
kernel classes mirror each kernel header's param_index enum.

Copyright 2003-2026 Timothy Place. MIT License.
"""

from __future__ import annotations

import ctypes
import pathlib
import subprocess
import sys

import numpy as np

# The kernel root (this file lives in kernel/notebooks/).
KERNEL = pathlib.Path(__file__).resolve().parent.parent

# Categorical palette for the notebooks (colorblind-safe, fixed assignment
# order — never cycled). Sequential maps use viridis; diverging use RdBu_r.
PALETTE = ["#4269d0", "#efb118", "#ff725c", "#6cc5b0", "#3ca951", "#ff8ab7", "#a463f2"]

_BUILD = KERNEL / "build_capi"


def _lib_path() -> pathlib.Path:
    stem = "taptools_capi"
    names = {"linux": f"lib{stem}.so", "darwin": f"lib{stem}.dylib", "win32": f"{stem}.dll"}
    name = next(v for k, v in names.items() if sys.platform.startswith(k))
    # single-config generators put the lib at the build root; MSVC nests it under a config dir
    for cand in (_BUILD / name, _BUILD / "Release" / name, _BUILD / "Debug" / name):
        if cand.exists():
            return cand
    return _BUILD / name


def _build_lib() -> None:
    subprocess.run(["cmake", "-B", str(_BUILD), "-S", str(KERNEL / "tools" / "capi")],
                   cwd=KERNEL, check=True, capture_output=True)
    subprocess.run(["cmake", "--build", str(_BUILD), "--config", "Release", "--parallel"],
                   cwd=KERNEL, check=True, capture_output=True)


def load() -> ctypes.CDLL:
    if not _lib_path().exists():
        print("building taptools_capi ...")
        _build_lib()
    lib = ctypes.CDLL(str(_lib_path()))

    vp = ctypes.c_void_p
    f32p = ctypes.POINTER(ctypes.c_float)
    f64p = ctypes.POINTER(ctypes.c_double)
    sigs = {
        "taptools_conv_create":         ([], vp),
        "taptools_conv_destroy":        ([vp], None),
        "taptools_conv_configure":      ([vp, ctypes.c_int, ctypes.c_int], ctypes.c_int),
        "taptools_conv_load_ir":        ([vp, f32p, f32p, f32p, f32p, ctypes.c_int, ctypes.c_double],
                                         ctypes.c_int),
        "taptools_conv_clear":          ([vp], ctypes.c_int),
        "taptools_conv_process":        ([vp, f64p, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_conv_block_size":     ([vp], ctypes.c_int),
        "taptools_conv_max_partitions": ([vp], ctypes.c_int),
        "taptools_conv_has_ir":         ([vp], ctypes.c_int),
        # the four parameter-indexed kernels share one shape (see _Kernel below)
        "taptools_svf_create":          ([], vp),
        "taptools_svf_destroy":         ([vp], None),
        "taptools_svf_prepare":         ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_svf_set":             ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_svf_set_mode":        ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_svf_set_order":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_svf_set_circuit":     ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_svf_set_oversample":  ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_svf_set_smooth_ms":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_svf_clear":           ([vp], ctypes.c_int),
        "taptools_svf_process":         ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_svf_process_mod":     ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_ladder_create":         ([], vp),
        "taptools_ladder_destroy":        ([vp], None),
        "taptools_ladder_prepare":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_ladder_set":            ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_ladder_set_mode":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_ladder_set_solver":     ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_ladder_set_oversample": ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_ladder_set_smooth_ms":  ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_ladder_clear":          ([vp], ctypes.c_int),
        "taptools_ladder_process":        ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_ladder_process_mod":    ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_vco_create":        ([], vp),
        "taptools_vco_destroy":       ([vp], None),
        "taptools_vco_prepare":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_vco_set":           ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_vco_set_seed":      ([vp, ctypes.c_uint], ctypes.c_int),
        "taptools_vco_set_smooth_ms": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_vco_clear":         ([vp], ctypes.c_int),
        "taptools_vco_process":       ([vp, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_vco_process_mod":   ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_diode_create":          ([], vp),
        "taptools_diode_destroy":         ([vp], None),
        "taptools_diode_prepare":         ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_diode_set":             ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_diode_set_solver":      ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_diode_set_oversample":  ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_diode_set_smooth_ms":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_diode_clear":           ([vp], ctypes.c_int),
        "taptools_diode_process":         ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_diode_process_mod":     ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_tb303_create":          ([], vp),
        "taptools_tb303_destroy":         ([vp], None),
        "taptools_tb303_prepare":         ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tb303_set":             ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_tb303_set_vca":         ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_tb303_set_solver":      ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_tb303_set_oversample":  ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_tb303_set_seed":        ([vp, ctypes.c_uint], ctypes.c_int),
        "taptools_tb303_set_tolerance":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tb303_set_smooth_ms":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tb303_recall":          ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_tb303_clear":           ([vp], ctypes.c_int),
        "taptools_tb303_note_on":         ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_tb303_note_off":        ([vp], ctypes.c_int),
        "taptools_tb303_set_pitch":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tb303_accent_charge":   ([vp], ctypes.c_double),
        "taptools_tb303_process":         ([vp, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_wah_create":        ([], vp),
        "taptools_wah_destroy":       ([vp], None),
        "taptools_wah_prepare":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_wah_set":           ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_wah_set_mode":      ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_wah_set_rectifier": ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_wah_set_smooth_ms": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_wah_recall":        ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_wah_clear":         ([vp], ctypes.c_int),
        "taptools_wah_process":       ([vp, f64p, f64p, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_seqtrig_create":       ([], vp),
        "taptools_seqtrig_destroy":      ([vp], None),
        "taptools_seqtrig_prepare":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_seqtrig_set_length":   ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_seqtrig_set_swing":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_seqtrig_set_quantize": ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_seqtrig_set_pulse_ms": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_seqtrig_set_step":     ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_seqtrig_store":        ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_seqtrig_recall":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_seqtrig_reset":        ([vp], ctypes.c_int),
        "taptools_seqtrig_process":      ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_seqnote_create":        ([], vp),
        "taptools_seqnote_destroy":       ([vp], None),
        "taptools_seqnote_prepare":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_seqnote_set_length":    ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_seqnote_set_swing":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_seqnote_set_quantize":  ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_seqnote_set_transpose": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_seqnote_set_step":      ([vp, ctypes.c_int, ctypes.c_double, ctypes.c_int, ctypes.c_int,
                                            ctypes.c_int], ctypes.c_int),
        "taptools_seqnote_store":         ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_seqnote_recall":        ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_seqnote_reset":         ([vp], ctypes.c_int),
        "taptools_seqnote_process":       ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_od_create":         ([], vp),
        "taptools_od_destroy":        ([vp], None),
        "taptools_od_prepare":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_od_set":            ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_od_set_oversample": ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_od_set_smooth_ms":  ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_od_clear":          ([vp], ctypes.c_int),
        "taptools_od_process":        ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_kick_create":     ([], vp),
        "taptools_kick_destroy":    ([vp], None),
        "taptools_kick_prepare":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_kick_set_decay":  ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_kick_set_tone":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_kick_set_level":  ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_kick_trigger":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_kick_reset":      ([vp], ctypes.c_int),
        "taptools_kick_process":    ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_tune_create":            ([], vp),
        "taptools_tune_destroy":           ([vp], None),
        "taptools_tune_prepare":           ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tune_clear":             ([vp], ctypes.c_int),
        "taptools_tune_set_key":           ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_tune_set_scale":         ([vp, ctypes.c_uint], ctypes.c_int),
        "taptools_tune_set_notes":         ([vp, ctypes.c_uint], ctypes.c_int),
        "taptools_tune_set_mode":          ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_tune_set_backend":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_tune_set_speed":         ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tune_set_amount":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tune_set_range":         ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_tune_set_threshold":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tune_set_formant":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_tune_set_autokey":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_tune_autokey_reset":     ([vp], ctypes.c_int),
        "taptools_tune_autokey_estimate":  ([vp, ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int),
                                             ctypes.POINTER(ctypes.c_double)], ctypes.c_int),
        "taptools_tune_autokey_apply":     ([vp], ctypes.c_int),
        "taptools_tune_note_on":           ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_tune_note_off":          ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_tune_notes_off":         ([vp], ctypes.c_int),
        "taptools_tune_detected_hz":       ([vp], ctypes.c_double),
        "taptools_tune_target_midi":       ([vp], ctypes.c_double),
        "taptools_tune_applied_semitones": ([vp], ctypes.c_double),
        "taptools_tune_process":           ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_adsr_create":             ([], vp),
        "taptools_adsr_destroy":            ([vp], None),
        "taptools_adsr_prepare":            ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_adsr_clear":              ([vp], ctypes.c_int),
        "taptools_adsr_set_attack":         ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_adsr_set_decay":          ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_adsr_set_sustain_db":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_adsr_set_release":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_adsr_set_mode":           ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_adsr_set_threshold":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_adsr_set_velocity":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_adsr_process":            ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_harmonizer_create":       ([], vp),
        "taptools_harmonizer_destroy":      ([vp], None),
        "taptools_harmonizer_prepare":      ([vp, ctypes.c_double, ctypes.c_int], ctypes.c_int),
        "taptools_harmonizer_clear":        ([vp], ctypes.c_int),
        "taptools_harmonizer_set_interval": ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_harmonizer_set_gain":     ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_harmonizer_set_dry":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_harmonizer_set_formant":  ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_harmonizer_set_glide":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_harmonizer_latency":      ([vp], ctypes.c_int),
        "taptools_harmonizer_process":      ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_delay_create":           ([], vp),
        "taptools_delay_destroy":          ([vp], None),
        "taptools_delay_prepare":          ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_delay_set_time_ms":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_delay_set_feedback":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_delay_set_mix":          ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_delay_set_interp":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_delay_set_smooth_ms":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_delay_clear":            ([vp], ctypes.c_int),
        "taptools_delay_process":          ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_delay_process_mod":      ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_multitap_create":        ([], vp),
        "taptools_multitap_destroy":       ([vp], None),
        "taptools_multitap_prepare":       ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_multitap_set_taps":      ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_multitap_set_time_ms":   ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_multitap_set_gain":      ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_multitap_set_pan":       ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_multitap_set_interp":    ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_multitap_set_smooth_ms": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_multitap_clear":         ([vp], ctypes.c_int),
        "taptools_multitap_process":       ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_discreet_create":        ([], vp),
        "taptools_discreet_destroy":       ([vp], None),
        "taptools_discreet_prepare":       ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_discreet_set_loop_seconds": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_discreet_set_regen":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_discreet_set_darken_hz": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_discreet_set_drive":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_discreet_set_input_level": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_discreet_set_mix":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_discreet_set_wow":       ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_discreet_set_flutter":   ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_discreet_set_smooth_ms": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_discreet_clear":         ([vp], ctypes.c_int),
        "taptools_discreet_process":       ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),

        "taptools_tapecho_create":         ([], vp),
        "taptools_tapecho_destroy":        ([vp], None),
        "taptools_tapecho_prepare":        ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_span_ms":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_heads":      ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_tapecho_set_head_ratio": ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_head_level": ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_head_pan":   ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_regen":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_darken_hz":  ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_drive":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_input_level": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_mix":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_wow":        ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_flutter":    ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_set_smooth_ms":  ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_tapecho_clear":          ([vp], ctypes.c_int),
        "taptools_tapecho_process":        ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),

        "taptools_plate_create":           ([], vp),
        "taptools_plate_destroy":          ([vp], None),
        "taptools_plate_prepare":          ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_plate_set_pitch_hz":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_plate_set_decay":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_plate_set_tilt":         ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_plate_set_brightness":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_plate_clear":            ([vp], ctypes.c_int),
        "taptools_plate_process":          ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_plate_mode_hz":          ([vp, ctypes.c_int], ctypes.c_double),
        "taptools_plate_mode_level":       ([vp, ctypes.c_int], ctypes.c_double),

        "taptools_transducer_create":      ([], vp),
        "taptools_transducer_destroy":     ([vp], None),
        "taptools_transducer_prepare":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_transducer_set_drive":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_transducer_set_asymmetry": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_transducer_set_saturation": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_transducer_clear":       ([vp], ctypes.c_int),
        "taptools_transducer_process":     ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),

        "taptools_metallique_create":      ([], vp),
        "taptools_metallique_destroy":     ([vp], None),
        "taptools_metallique_prepare":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_metallique_set_pitch_hz": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_metallique_set_decay":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_metallique_set_tilt":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_metallique_set_brightness": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_metallique_set_drive":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_metallique_set_asymmetry": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_metallique_set_saturation": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_metallique_set_mix":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_metallique_set_level":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_metallique_set_smooth_ms": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_metallique_clear":       ([vp], ctypes.c_int),
        "taptools_metallique_process":     ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_metallique_mode_hz":     ([vp, ctypes.c_int], ctypes.c_double),
        "taptools_metallique_mode_level":  ([vp, ctypes.c_int], ctypes.c_double),

        "taptools_palme_create":           ([], vp),
        "taptools_palme_destroy":          ([vp], None),
        "taptools_palme_prepare":          ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_palme_set_root_hz":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_palme_set_tuning":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_palme_set_decay":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_palme_set_damping":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_palme_set_detune":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_palme_set_drive":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_palme_set_asymmetry":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_palme_set_saturation":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_palme_set_mix":          ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_palme_set_level":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_palme_set_smooth_ms":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_palme_clear":            ([vp], ctypes.c_int),
        "taptools_palme_process":          ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_palme_string_hz":        ([vp, ctypes.c_int], ctypes.c_double),
        "taptools_palme_string_feedback":  ([vp, ctypes.c_int], ctypes.c_double),

        "taptools_scrub_create":           ([], vp),
        "taptools_scrub_destroy":          ([vp], None),
        "taptools_scrub_prepare":          ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_scrub_set_position_ms":  ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_scrub_set_pitch":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_scrub_set_drift":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_scrub_set_freeze":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_scrub_set_size_ms":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_scrub_set_overlap":      ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_scrub_set_spray_ms":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_scrub_set_seed":         ([vp, ctypes.c_ulonglong], ctypes.c_int),
        "taptools_scrub_set_mix":          ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_scrub_set_level":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_scrub_set_smooth_ms":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_scrub_clear":            ([vp], ctypes.c_int),
        "taptools_scrub_process":          ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_scrub_process_mod":      ([vp, f64p, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_scrub_active_grains":    ([vp], ctypes.c_int),

        "taptools_touche_create":          ([], vp),
        "taptools_touche_destroy":         ([vp], None),
        "taptools_touche_prepare":         ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_touche_set_position":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_touche_set_position_mm": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_touche_set_force_n":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_touche_set_mode":        ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_touche_set_smooth_ms":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_touche_clear":           ([vp], ctypes.c_int),
        "taptools_touche_gain_at":         ([vp, ctypes.c_double], ctypes.c_double),
        "taptools_touche_process":         ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_touche_process_mod":     ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),

        "taptools_fuzz_create":            ([], vp),
        "taptools_fuzz_destroy":           ([vp], None),
        "taptools_fuzz_prepare":           ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_fuzz_set_gain":          ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_fuzz_set_edge":          ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_fuzz_set_asymmetry":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_fuzz_set_bass":          ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_fuzz_set_treble":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_fuzz_set_contrast":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_fuzz_set_level_db":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_fuzz_set_oversample":    ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_fuzz_set_smooth_ms":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_fuzz_clear":             ([vp], ctypes.c_int),
        "taptools_fuzz_process":           ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),

        "taptools_stammer_create":         ([], vp),
        "taptools_stammer_destroy":        ([vp], None),
        "taptools_stammer_prepare":        ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_stammer_set_step_ms":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_stammer_set_density":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_stammer_set_divisions":  ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_stammer_set_repeats":    ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_stammer_set_reverse":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_stammer_set_jump_ms":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_stammer_set_fade_ms":    ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_stammer_set_seed":       ([vp, ctypes.c_ulonglong], ctypes.c_int),
        "taptools_stammer_set_input_level": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_stammer_set_mix":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_stammer_set_smooth_ms":  ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_stammer_clear":          ([vp], ctypes.c_int),
        "taptools_stammer_playing":        ([vp], ctypes.c_int),
        "taptools_stammer_process":        ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_airport_create":         ([], vp),
        "taptools_airport_destroy":        ([vp], None),
        "taptools_airport_prepare":        ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_airport_set_loops":      ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_airport_set_length_seconds": ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_airport_record":         ([vp, ctypes.c_int, ctypes.c_int], ctypes.c_int),
        "taptools_airport_set_level":      ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_airport_set_pan":        ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_airport_set_darken_hz":  ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_airport_set_smooth_ms":  ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_airport_clear":          ([vp], ctypes.c_int),
        "taptools_airport_phase":          ([vp, ctypes.c_int], ctypes.c_double),
        "taptools_airport_composite_period_seconds": ([vp], ctypes.c_double),
        "taptools_airport_process":        ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_garden_create":          ([], vp),
        "taptools_garden_destroy":         ([vp], None),
        "taptools_garden_prepare":         ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_garden_note":            ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_garden_set_loop_seconds": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_garden_set_decay":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_garden_set_soften":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_garden_set_floor":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_garden_set_bell":        ([vp, ctypes.c_double, ctypes.c_double, ctypes.c_double],
                                            ctypes.c_int),
        "taptools_garden_set_material":    ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_garden_set_spread":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_garden_set_root":        ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_garden_set_scale":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_garden_set_idle_seconds": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_garden_set_gust":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_garden_set_seed":        ([vp, ctypes.c_ulonglong], ctypes.c_int),
        "taptools_garden_set_level":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_garden_set_smooth_ms":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_garden_clear":           ([vp], ctypes.c_int),
        "taptools_garden_active_events":   ([vp], ctypes.c_int),
        "taptools_garden_active_voices":   ([vp], ctypes.c_int),
        "taptools_garden_process":         ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        # the components the monoliths are made of
        "taptools_reel_create":            ([], vp),
        "taptools_reel_destroy":           ([vp], None),
        "taptools_reel_prepare":           ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_reel_set_length_seconds": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_reel_record":            ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_reel_set_level":         ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_reel_set_pan":           ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_reel_set_darken_hz":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_reel_set_smooth_ms":     ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_reel_clear":             ([vp], ctypes.c_int),
        "taptools_reel_phase":             ([vp], ctypes.c_double),
        "taptools_reel_length_seconds":    ([vp], ctypes.c_double),
        "taptools_reel_loop_samples":      ([vp], ctypes.c_int),
        "taptools_reel_process":           ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_chime_create":           ([], vp),
        "taptools_chime_destroy":          ([vp], None),
        "taptools_chime_prepare":          ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_chime_set_times":        ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_chime_set_material":     ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_chime_set_spread":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_chime_clear":            ([vp], ctypes.c_int),
        "taptools_chime_strike":           ([vp, ctypes.c_double, ctypes.c_double, ctypes.c_double],
                                            ctypes.c_int),
        "taptools_chime_strike_hz":        ([vp, ctypes.c_double, ctypes.c_double, ctypes.c_double],
                                            ctypes.c_int),
        "taptools_chime_active_voices":    ([vp], ctypes.c_int),
        "taptools_chime_process":          ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_bloom_create":           ([], vp),
        "taptools_bloom_destroy":          ([vp], None),
        "taptools_bloom_prepare":          ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_bloom_set_loop_seconds": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_bloom_set_decay":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_bloom_set_soften":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_bloom_set_floor":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_bloom_set_brightness":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_bloom_clear":            ([vp], ctypes.c_int),
        "taptools_bloom_plant":            ([vp, ctypes.c_double, ctypes.c_double], ctypes.c_int),
        "taptools_bloom_due":              ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_bloom_step":             ([vp], ctypes.c_int),
        "taptools_bloom_active_events":    ([vp], ctypes.c_int),
        "taptools_bloom_loop_samples":     ([vp], ctypes.c_int),
        "taptools_gardener_create":        ([], vp),
        "taptools_gardener_destroy":       ([vp], None),
        "taptools_gardener_prepare":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_gardener_set_idle_seconds": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_gardener_set_gust":      ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_gardener_set_seed":      ([vp, ctypes.c_ulonglong], ctypes.c_int),
        "taptools_gardener_notice_plant":  ([vp], ctypes.c_int),
        "taptools_gardener_clear":         ([vp], ctypes.c_int),
        "taptools_gardener_tick":          ([vp, ctypes.c_int, f64p, f64p], ctypes.c_int),
        "taptools_scale_quantize":         ([ctypes.c_double, ctypes.c_int, ctypes.c_int],
                                            ctypes.c_double),
        "taptools_chime_process_voices":   ([vp, f64p, ctypes.c_int, ctypes.c_int], ctypes.c_int),
        "taptools_chime_voice_hz":         ([vp, ctypes.c_int], ctypes.c_double),
        "taptools_chime_voice_level":      ([vp, ctypes.c_int], ctypes.c_double),
        "taptools_chime_voice_gain_left":  ([vp, ctypes.c_int], ctypes.c_double),
        "taptools_chime_voice_gain_right": ([vp, ctypes.c_int], ctypes.c_double),
        "taptools_composite_period_seconds": ([f64p, ctypes.c_int, ctypes.c_double], ctypes.c_double),
        "taptools_yin_create":             ([ctypes.c_int, ctypes.c_int, ctypes.c_int], vp),
        "taptools_yin_destroy":            ([vp], None),
        "taptools_yin_frame_size":         ([vp], ctypes.c_int),
        "taptools_yin_track":              ([vp, f64p, ctypes.c_int, ctypes.c_int, f64p, ctypes.c_int],
                                            ctypes.c_int),
    }
    for name, (argtypes, restype) in sigs.items():
        fn = getattr(lib, name)
        fn.argtypes = argtypes
        fn.restype = restype
    return lib


_LIB = load()


def _f32(a) -> np.ndarray:
    return np.ascontiguousarray(a, dtype=np.float32)


def _f64(a) -> np.ndarray:
    return np.ascontiguousarray(a, dtype=np.float64)


def _p32(a: np.ndarray):
    return a.ctypes.data_as(ctypes.POINTER(ctypes.c_float))


def _p64(a: np.ndarray):
    return a.ctypes.data_as(ctypes.POINTER(ctypes.c_double))


def _check(code: int, who: str) -> None:
    if code is not None and code < 0:
        raise RuntimeError(f"{who} failed")


class Convolver:
    """A live tap.convolve~ conv_engine — true-stereo uniformly-partitioned overlap-save.

    >>> c = Convolver(blocksize=256, max_partitions=64)
    >>> c.load_ir(ll=ir, rr=ir)          # any of ll/lr/rl/rr; missing paths are silent
    >>> outL, outR = c.process(inL, inR) # wet (fully convolved) stereo output
    """

    def __init__(self, blocksize: int, max_partitions: int):
        self._h = _LIB.taptools_conv_create()
        if not self._h:
            raise RuntimeError("taptools_conv_create failed")
        _check(_LIB.taptools_conv_configure(self._h, int(blocksize), int(max_partitions)), "configure")

    def load_ir(self, ll=None, lr=None, rl=None, rr=None, scale: float = 1.0) -> None:
        """Publish the four true-stereo IR paths (LL, LR, RL, RR). Missing paths are silent; the
        arrays may differ in length (shorter ones are zero-padded to the longest)."""
        arrs = [ll, lr, rl, rr]
        length = max((len(a) for a in arrs if a is not None), default=0)
        self._ir_keep = []  # keep the float buffers alive across the C call
        ptrs = []
        for a in arrs:
            if a is None:
                ptrs.append(None)
            else:
                f = _f32(np.pad(np.asarray(a, dtype=np.float32), (0, length - len(a))))
                self._ir_keep.append(f)
                ptrs.append(_p32(f))
        _check(_LIB.taptools_conv_load_ir(self._h, ptrs[0], ptrs[1], ptrs[2], ptrs[3],
                                          int(length), float(scale)), "load_ir")

    def process(self, inL, inR) -> tuple[np.ndarray, np.ndarray]:
        inL, inR = _f64(inL), _f64(inR)
        n = len(inL)
        outL, outR = np.empty(n), np.empty(n)
        _check(_LIB.taptools_conv_process(self._h, _p64(inL), _p64(inR), _p64(outL), _p64(outR), n),
               "process")
        return outL, outR

    def clear(self) -> None:
        _check(_LIB.taptools_conv_clear(self._h), "clear")

    @property
    def block_size(self) -> int:
        return _LIB.taptools_conv_block_size(self._h)

    @property
    def max_partitions(self) -> int:
        return _LIB.taptools_conv_max_partitions(self._h)

    @property
    def has_ir(self) -> bool:
        return bool(_LIB.taptools_conv_has_ir(self._h))

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_conv_destroy(h)
            self._h = None


def convolve_stereo(inL, inR, ll=None, lr=None, rl=None, rr=None, *, blocksize: int = 256,
                    scale: float = 1.0) -> tuple[np.ndarray, np.ndarray]:
    """One-shot helper: run inL/inR through a fresh engine with the given IR paths, sized to hold
    the whole IR. Returns the wet stereo output (latency = blocksize samples)."""
    length = max((len(a) for a in (ll, lr, rl, rr) if a is not None), default=0)
    max_parts = max(1, (length + blocksize - 1) // blocksize)
    c = Convolver(blocksize, max_parts)
    c.load_ir(ll=ll, lr=lr, rl=rl, rr=rr, scale=scale)
    return c.process(inL, inR)


class _Kernel:
    """Base for the parameter-indexed kernels (Svf/Ladder/Vco/Wah): a live C-ABI handle with
    keyword parameter setters mirroring the kernel header's param_index enum.

    Subclasses define PREFIX (the C-ABI function prefix), PARAMS (name -> index), and MODES /
    structural setter names. `set(smooth_ms=0)` is applied by default so notebook measurements
    are exact unless a test wants the ramps."""

    PREFIX: str = ""
    PARAMS: dict[str, int] = {}

    def __init__(self, sr: float = 48000.0, smooth_ms: float = 0.0, **params):
        self._c = {name: getattr(_LIB, f"{self.PREFIX}_{name}", None)
                   for name in ("create", "destroy", "prepare", "set", "set_smooth_ms", "clear")}
        self._h = self._c["create"]()
        if not self._h:
            raise RuntimeError(f"{self.PREFIX}_create failed")
        _check(self._c["prepare"](self._h, float(sr)), "prepare")
        self.sr = float(sr)
        _check(self._c["set_smooth_ms"](self._h, float(smooth_ms)), "set_smooth_ms")
        self.set(**params)

    def set(self, **params) -> "_Kernel":
        """Set ramped parameters by name (e.g. svf.set(frequency=1000, resonance=0.5)) and any
        structural setter exposed by the subclass (mode=..., order=..., ...)."""
        for name, value in params.items():
            if name in self.PARAMS:
                _check(self._c["set"](self._h, self.PARAMS[name], float(value)), name)
            else:
                fn = getattr(_LIB, f"{self.PREFIX}_set_{name}", None)
                if fn is None:
                    raise AttributeError(f"{type(self).__name__} has no parameter '{name}'")
                arg = float(value) if name == "smooth_ms" else int(value)
                _check(fn(self._h, arg), name)
        return self

    def clear(self) -> None:
        _check(self._c["clear"](self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            self._c["destroy"](h)
            self._h = None


class Svf(_Kernel):
    """tap.svf~'s Simper/Cytomic morphing SVF (taptools::svf::svf_filter, mono).

    >>> f = Svf(48000, frequency=1000, resonance=0.5, mode=Svf.LOWPASS, order=4)
    >>> y = f.process(x)                      # block process
    >>> y = f.process(x, cutoff_hz=sweep)     # per-sample signal-rate cutoff
    """

    PREFIX = "taptools_svf"
    PARAMS = {"frequency": 0, "resonance": 1, "morph": 2, "gain": 3, "drive": 4}
    LOWPASS, HIGHPASS, BANDPASS, NOTCH, PEAK, ALLPASS, MORPH, BELL, LOWSHELF, HIGHSHELF = range(10)
    CLEAN, DRIVEN = 0, 1

    def process(self, x, cutoff_hz=None) -> np.ndarray:
        x = _f64(x)
        out = np.empty(len(x))
        if cutoff_hz is None:
            _check(_LIB.taptools_svf_process(self._h, _p64(x), _p64(out), len(x)), "process")
        else:
            c = _f64(cutoff_hz)
            _check(_LIB.taptools_svf_process_mod(self._h, _p64(x), _p64(c), _p64(out), len(x)),
                   "process_mod")
        return out


class Ladder(_Kernel):
    """tap.ladder~'s ZDF transistor ladder (taptools::ladder::ladder_filter).

    >>> f = Ladder(48000, frequency=800, resonance=0.7, mode=Ladder.LP24)
    >>> y = f.process(x)                      # block process
    >>> y = f.process(x, cutoff_hz=sweep)     # per-sample signal-rate cutoff
    """

    PREFIX = "taptools_ladder"
    PARAMS = {"gain": 0, "frequency": 1, "resonance": 2, "drive": 3, "comp": 4, "asym": 5}
    LP24, LP12, BP12, BP24, HP12, HP24 = range(6)
    FAST, EXACT = 0, 1

    def process(self, x, cutoff_hz=None) -> np.ndarray:
        x = _f64(x)
        out = np.empty(len(x))
        if cutoff_hz is None:
            _check(_LIB.taptools_ladder_process(self._h, _p64(x), _p64(out), len(x)), "process")
        else:
            c = _f64(cutoff_hz)
            _check(_LIB.taptools_ladder_process_mod(self._h, _p64(x), _p64(c), _p64(out), len(x)),
                   "process_mod")
        return out


class Diode(_Kernel):
    """tap.diode~'s ZDF TB-303 diode ladder (taptools::diode::diode_filter).

    >>> f = Diode(48000, frequency=1000, resonance=0.9)
    >>> y = f.process(x)                      # block process
    >>> y = f.process(x, cutoff_hz=sweep)     # per-sample signal-rate cutoff
    """

    PREFIX = "taptools_diode"
    PARAMS = {"gain": 0, "frequency": 1, "resonance": 2, "drive": 3, "fbhp": 4}
    FAST, EXACT = 0, 1

    def process(self, x, cutoff_hz=None) -> np.ndarray:
        x = _f64(x)
        out = np.empty(len(x))
        if cutoff_hz is None:
            _check(_LIB.taptools_diode_process(self._h, _p64(x), _p64(out), len(x)), "process")
        else:
            c = _f64(cutoff_hz)
            _check(_LIB.taptools_diode_process_mod(self._h, _p64(x), _p64(c), _p64(out), len(x)),
                   "process_mod")
        return out


class TB303(_Kernel):
    """tap.303~'s acid-bass voice (taptools::tb303::voice) — the full note interface.

    >>> v = TB303(48000, cutoff=500, resonance=0.9, envmod=0.7, decay=300, accent=0.8)
    >>> v.note_on(33, accent=1.0); y = v.process(24000); v.note_off()
    >>> v.set(vca=TB303.WARM)                 # phase-2 transistor VCA
    """

    PREFIX = "taptools_tb303"
    PARAMS = {"gain": 0, "tuning": 1, "cutoff": 2, "resonance": 3, "envmod": 4, "decay": 5,
              "accent": 6, "waveform": 7, "slide": 8, "attack": 9, "accdecay": 10, "drive": 11}
    SAW, SQUARE = 0.0, 1.0
    CLEAN, WARM = 0, 1
    FAST, EXACT = 0, 1

    def set(self, **params) -> "TB303":
        tol = params.pop("tolerance", None)
        if tol is not None:
            _check(_LIB.taptools_tb303_set_tolerance(self._h, float(tol)), "tolerance")
        return super().set(**params)

    def note_on(self, midi_note: float, accent: float = 0.0) -> None:
        _check(_LIB.taptools_tb303_note_on(self._h, float(midi_note), float(accent)), "note_on")

    def note_off(self) -> None:
        _check(_LIB.taptools_tb303_note_off(self._h), "note_off")

    def set_pitch(self, midi_note: float) -> None:
        _check(_LIB.taptools_tb303_set_pitch(self._h, float(midi_note)), "set_pitch")

    def recall(self, slot: int, seconds: float = 0.0) -> None:
        """Morph to a preset slot (1-based like the Max wrapper; 1-8 = factory patches)."""
        _check(_LIB.taptools_tb303_recall(self._h, int(slot) - 1, float(seconds)), "recall")

    @property
    def accent_charge(self) -> float:
        return float(_LIB.taptools_tb303_accent_charge(self._h))

    def process(self, n: int) -> np.ndarray:
        out = np.empty(int(n))
        _check(_LIB.taptools_tb303_process(self._h, _p64(out), len(out)), "process")
        return out


class Vco(_Kernel):
    """tap.vco~'s virtual-analog oscillator (taptools::vco::vco_osc).

    >>> o = Vco(48000, frequency=110, shape=Vco.SAW)
    >>> y = o.process(4800)                                  # n samples
    >>> y = o.process(fm_hz=fm)                              # through-zero FM (Hz)
    >>> y = o.process(sync=master)                           # hard sync input
    """

    PREFIX = "taptools_vco"
    PARAMS = {"gain": 0, "frequency": 1, "shape": 2, "pw": 3, "drift": 4, "detune": 5,
              "imperfect": 6, "jitter": 7, "track": 8, "vibrato": 9, "vibrato_rate": 10,
              "vibrato_delay": 11, "bend": 12}
    SINE, TRIANGLE, SAW, PULSE = range(4)

    def process(self, n: int | None = None, fm_hz=None, sync=None) -> np.ndarray:
        if fm_hz is None and sync is None:
            out = np.empty(int(n))
            _check(_LIB.taptools_vco_process(self._h, _p64(out), len(out)), "process")
            return out
        length = len(fm_hz) if fm_hz is not None else len(sync)
        if n is not None and int(n) != length:
            raise ValueError("n disagrees with the modulation array length")
        fm_arr = _f64(fm_hz) if fm_hz is not None else None
        sy_arr = _f64(sync) if sync is not None else None
        out = np.empty(length)
        _check(_LIB.taptools_vco_process_mod(self._h,
                                             _p64(fm_arr) if fm_arr is not None else None,
                                             _p64(sy_arr) if sy_arr is not None else None,
                                             _p64(out), length), "process_mod")
        return out

    def seed(self, seed: int) -> "Vco":
        _check(_LIB.taptools_vco_set_seed(self._h, int(seed)), "set_seed")
        return self


class Wah(_Kernel):
    """tap.autowah~'s Snow White-style envelope filter (taptools::autowah::wah_filter).

    >>> w = Wah(48000, sensitivity=0, decay=250)
    >>> y = w.process(x)                          # envelope tracks the input (the pedal)
    >>> y = w.process(x, key=other)               # sidechain
    >>> y, env, cutoff = w.process(x, trace=True) # + per-sample control trajectories
    """

    PREFIX = "taptools_wah"
    PARAMS = {"sensitivity": 0, "attack": 1, "decay": 2, "bias": 3, "range": 4,
              "resonance": 5, "drive": 6, "gain": 7, "mix": 8}
    LOWPASS, BANDPASS = 0, 1
    FULLWAVE, HALFWAVE = 0, 1

    def process(self, x, key=None, trace: bool = False):
        x = _f64(x)
        n = len(x)
        out = np.empty(n)
        key_arr = _f64(key) if key is not None else None
        env = np.empty(n) if trace else None
        cut = np.empty(n) if trace else None
        _check(_LIB.taptools_wah_process(self._h, _p64(x),
                                         _p64(key_arr) if key_arr is not None else None,
                                         _p64(out),
                                         _p64(env) if trace else None,
                                         _p64(cut) if trace else None, n), "process")
        return (out, env, cut) if trace else out

    def recall(self, slot: int, seconds: float = 0.0) -> "Wah":
        """Morph to a preset slot (0-based; 0-3 = factory guitar/bass/swell/cocked)."""
        _check(_LIB.taptools_wah_recall(self._h, int(slot), float(seconds)), "recall")
        return self


class Overdrive(_Kernel):
    """tap.overdrive~'s voiced feedback soft clipper (taptools::od::overdrive, mono).

    >>> o = Overdrive(48000, drive=0.7, asymmetry=0.3, body=-0.5)
    >>> y = o.process(x)
    >>> o.set(oversample=1)   # structural: 1/2/4/8 (default 4)
    """

    PREFIX = "taptools_od"
    PARAMS = {"drive": 0, "body": 1, "asymmetry": 2, "preamp": 3, "output": 4}

    def process(self, x) -> np.ndarray:
        x = _f64(x)
        n = len(x)
        out = np.empty(n)
        _check(_LIB.taptools_od_process(self._h, _p64(x), _p64(out), n), "process")
        return out


class _SeqRow:
    """Base for the step-sequencer rows (the step_seq.h engine behind tap.808.seq~ /
    tap.303.seq~): a live C-ABI handle plus the shared clock surface. `phase(cycles)` builds
    the phasor~-style ramp the rows are clocked from."""

    PREFIX: str = ""
    CYCLE, STEP, NOW = 0, 1, 2  # quantize modes

    def __init__(self, sr: float = 48000.0, length: int = 16, swing: float = 0.0):
        self._c = {name: getattr(_LIB, f"{self.PREFIX}_{name}")
                   for name in ("create", "destroy", "prepare", "set_length", "set_swing",
                                "set_quantize", "store", "recall", "reset")}
        self._h = self._c["create"]()
        if not self._h:
            raise RuntimeError(f"{self.PREFIX}_create failed")
        _check(self._c["prepare"](self._h, float(sr)), "prepare")
        self.sr = float(sr)
        self.length = int(length)
        _check(self._c["set_length"](self._h, int(length)), "set_length")
        _check(self._c["set_swing"](self._h, float(swing)), "set_swing")

    def set(self, *, length=None, swing=None, quantize=None) -> "_SeqRow":
        if length is not None:
            self.length = int(length)
            _check(self._c["set_length"](self._h, int(length)), "set_length")
        if swing is not None:
            _check(self._c["set_swing"](self._h, float(swing)), "set_swing")
        if quantize is not None:
            _check(self._c["set_quantize"](self._h, int(quantize)), "set_quantize")
        return self

    def store(self, slot: int) -> None:
        _check(self._c["store"](self._h, int(slot)), "store")

    def recall(self, slot: int) -> None:
        _check(self._c["recall"](self._h, int(slot)), "recall")

    def reset(self) -> None:
        _check(self._c["reset"](self._h), "reset")

    def phase(self, cycles: float, cycle_hz: float = 2.0) -> np.ndarray:
        """A phasor~-style ramp: `cycles` pattern cycles at `cycle_hz` cycles/second."""
        n = int(round(cycles * self.sr / cycle_hz))
        return (np.arange(n) * (cycle_hz / self.sr)) % 1.0

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            self._c["destroy"](h)
            self._h = None


class TriggerRow(_SeqRow):
    """One tap.808.seq~ drum row: velocities in, amplitude-as-accent impulses out.

    >>> row = TriggerRow()
    >>> row.steps([1.0, 0, 0, 0, 0.5, 0, 0, 0] * 2)
    >>> y = row.process(row.phase(cycles=2))
    """

    PREFIX = "taptools_seqtrig"

    def steps(self, velocities) -> "TriggerRow":
        for k, v in enumerate(velocities):
            _check(_LIB.taptools_seqtrig_set_step(self._h, k, float(v)), "set_step")
        return self

    def set(self, *, pulse_ms=None, **kw) -> "TriggerRow":
        if pulse_ms is not None:
            _check(_LIB.taptools_seqtrig_set_pulse_ms(self._h, float(pulse_ms)), "set_pulse_ms")
        super().set(**kw)
        return self

    def process(self, phase) -> np.ndarray:
        phase = _f64(phase)
        out = np.empty(len(phase))
        _check(_LIB.taptools_seqtrig_process(self._h, _p64(phase), _p64(out), len(phase)), "process")
        return out


class NoteRow(_SeqRow):
    """One tap.303.seq~ line: per-step pitch/gate/accent/slide in, the tap.303~ inlet pair out.

    >>> row = NoteRow()
    >>> row.steps([(33, 1, 1, 0), (33, 1, 0, 0), (45, 1, 0, 1), ...])  # (pitch, gate, accent, slide)
    >>> pitch, gate = row.process(row.phase(cycles=2))
    """

    PREFIX = "taptools_seqnote"

    def steps(self, steps) -> "NoteRow":
        for k, s in enumerate(steps):
            pitch, gate, accent, slide = s
            _check(_LIB.taptools_seqnote_set_step(self._h, k, float(pitch), int(gate), int(accent),
                                                  int(slide)), "set_step")
        return self

    def set(self, *, transpose=None, **kw) -> "NoteRow":
        if transpose is not None:
            _check(_LIB.taptools_seqnote_set_transpose(self._h, float(transpose)), "set_transpose")
        super().set(**kw)
        return self

    def process(self, phase) -> tuple[np.ndarray, np.ndarray]:
        phase = _f64(phase)
        pitch = np.empty(len(phase))
        gate = np.empty(len(phase))
        _check(_LIB.taptools_seqnote_process(self._h, _p64(phase), _p64(pitch), _p64(gate),
                                             len(phase)), "process")
        return pitch, gate


class Kick:
    """The tap.808.kick~ voice (tr808_kick.h) with the wrapper's signal-edge trigger logic —
    wire a TriggerRow's output straight into `process(trig=...)`."""

    def __init__(self, sr: float = 48000.0, decay: float = 0.5, tone: float = 0.5,
                 level: float = 1.0):
        self._h = _LIB.taptools_kick_create()
        if not self._h:
            raise RuntimeError("taptools_kick_create failed")
        _check(_LIB.taptools_kick_prepare(self._h, float(sr)), "prepare")
        self.sr = float(sr)
        self.set(decay=decay, tone=tone, level=level)

    def set(self, *, decay=None, tone=None, level=None) -> "Kick":
        for name, v in (("decay", decay), ("tone", tone), ("level", level)):
            if v is not None:
                _check(getattr(_LIB, f"taptools_kick_set_{name}")(self._h, float(v)), name)
        return self

    def trigger(self, accent: float = 1.0) -> None:
        _check(_LIB.taptools_kick_trigger(self._h, float(accent)), "trigger")

    def process(self, n: int | None = None, trig=None) -> np.ndarray:
        if trig is not None:
            trig = _f64(trig)
            n = len(trig)
        out = np.empty(int(n))
        _check(_LIB.taptools_kick_process(self._h, _p64(trig) if trig is not None else None,
                                          _p64(out), int(n)), "process")
        return out

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_kick_destroy(h)
            self._h = None


# scale masks and enums mirroring taptools/tune.h
TUNE_SCALES = {
    "chromatic": 0xFFF,
    "major": sum(1 << d for d in (0, 2, 4, 5, 7, 9, 11)),
    "minor": sum(1 << d for d in (0, 2, 3, 5, 7, 8, 10)),
}
TUNE_MODES = {"scale": 0, "midi": 1}
TUNE_BACKENDS = {"grain": 0, "psola": 1, "pvoc": 2}


class Tune:
    """tap.tune~'s corrector (tap::tools::tune::corrector) — the full chain:
    YIN detection, scale/MIDI target mapping, retune glide, and the selectable
    resynthesis backend (grain / psola / pvoc)."""

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_tune_create()
        _check(_LIB.taptools_tune_prepare(self._h, float(sr)), "prepare")
        self.set(**params)

    def set(self, *, key=None, scale=None, notes=None, mode=None, backend=None, speed=None,
            amount=None, threshold=None, formant=None) -> "Tune":
        if key is not None:
            _check(_LIB.taptools_tune_set_key(self._h, int(key)), "key")
        if scale is not None:
            mask = TUNE_SCALES[scale] if isinstance(scale, str) else int(scale)
            _check(_LIB.taptools_tune_set_scale(self._h, mask), "scale")
        if notes is not None:
            _check(_LIB.taptools_tune_set_notes(self._h, int(notes)), "notes")
        if mode is not None:
            _check(_LIB.taptools_tune_set_mode(self._h, TUNE_MODES[mode]), "mode")
        if backend is not None:
            _check(_LIB.taptools_tune_set_backend(self._h, TUNE_BACKENDS[backend]), "backend")
        if speed is not None:
            _check(_LIB.taptools_tune_set_speed(self._h, float(speed)), "speed")
        if amount is not None:
            _check(_LIB.taptools_tune_set_amount(self._h, float(amount)), "amount")
        if threshold is not None:
            _check(_LIB.taptools_tune_set_threshold(self._h, float(threshold)), "threshold")
        if formant is not None:
            _check(_LIB.taptools_tune_set_formant(self._h, int(bool(formant))), "formant")
        return self

    def set_autokey(self, on: bool) -> None:
        _check(_LIB.taptools_tune_set_autokey(self._h, int(bool(on))), "autokey")

    def autokey_estimate(self) -> tuple[int, bool, float]:
        """(key 0-11 or -1, minor, confidence) from the Krumhansl-Kessler scorer."""
        key = ctypes.c_int()
        minor = ctypes.c_int()
        conf = ctypes.c_double()
        _check(_LIB.taptools_tune_autokey_estimate(self._h, ctypes.byref(key), ctypes.byref(minor),
                                                   ctypes.byref(conf)), "autokey_estimate")
        return key.value, bool(minor.value), conf.value

    def autokey_apply(self) -> bool:
        return _LIB.taptools_tune_autokey_apply(self._h) == 1

    def note_on(self, note: int) -> None:
        _check(_LIB.taptools_tune_note_on(self._h, int(note)), "note_on")

    def note_off(self, note: int) -> None:
        _check(_LIB.taptools_tune_note_off(self._h, int(note)), "note_off")

    @property
    def detected_hz(self) -> float:
        return _LIB.taptools_tune_detected_hz(self._h)

    @property
    def applied_semitones(self) -> float:
        return _LIB.taptools_tune_applied_semitones(self._h)

    def process(self, x) -> np.ndarray:
        x = _f64(x)
        out = np.empty_like(x)
        _check(_LIB.taptools_tune_process(self._h, _p64(x), _p64(out), x.size), "process")
        return out

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_tune_destroy(h)
            self._h = None


ADSR_MODES = {"analog": 0, "hybrid": 1, "linear": 2, "exponential": 3}


class Adsr:
    """tap.adsr~'s kernel (tap::tools::adsr::generator): the virtual-analog
    envelope (truncated RC attack, asymptotic decay/release) with the Jamoma
    curves as compatibility modes. Gate amplitude is velocity under the
    `velocity` sensitivity; the gate opens above `threshold`."""

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_adsr_create()
        _check(_LIB.taptools_adsr_prepare(self._h, float(sr)), "prepare")
        self.set(**params)

    def set(self, *, attack=None, decay=None, sustain=None, release=None, mode=None,
            threshold=None, velocity=None) -> "Adsr":
        if attack is not None:
            _check(_LIB.taptools_adsr_set_attack(self._h, float(attack)), "attack")
        if decay is not None:
            _check(_LIB.taptools_adsr_set_decay(self._h, float(decay)), "decay")
        if sustain is not None:
            _check(_LIB.taptools_adsr_set_sustain_db(self._h, float(sustain)), "sustain")
        if release is not None:
            _check(_LIB.taptools_adsr_set_release(self._h, float(release)), "release")
        if mode is not None:
            _check(_LIB.taptools_adsr_set_mode(self._h, ADSR_MODES[mode]), "mode")
        if threshold is not None:
            _check(_LIB.taptools_adsr_set_threshold(self._h, float(threshold)), "threshold")
        if velocity is not None:
            _check(_LIB.taptools_adsr_set_velocity(self._h, float(velocity)), "velocity")
        return self

    def process(self, gate) -> np.ndarray:
        gate = _f64(gate)
        out = np.zeros_like(gate)
        _check(_LIB.taptools_adsr_process(self._h, _p64(gate), _p64(out), gate.size), "process")
        return out

    def clear(self) -> None:
        _check(_LIB.taptools_adsr_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_adsr_destroy(h)
            self._h = None


class Harmonizer:
    """tap.harmony~'s kernel (tap::tools::harmony::harmonizer): up to four
    formant-preserving pvoc voices at fixed intervals plus a latency-aligned
    dry path. Intervals are fractional semitones; gains are linear."""

    def __init__(self, sr: float = 48000.0, fft_size: int = 1024, **params):
        self._h = _LIB.taptools_harmonizer_create()
        _check(_LIB.taptools_harmonizer_prepare(self._h, float(sr), int(fft_size)), "prepare")
        self.set(**params)

    def set(self, *, intervals=None, gains=None, dry=None, formant=None, glide=None) -> "Harmonizer":
        if intervals is not None:
            for v, st in enumerate(intervals):
                _check(_LIB.taptools_harmonizer_set_interval(self._h, v, float(st)), "interval")
        if gains is not None:
            for v, g in enumerate(gains):
                _check(_LIB.taptools_harmonizer_set_gain(self._h, v, float(g)), "gain")
        if dry is not None:
            _check(_LIB.taptools_harmonizer_set_dry(self._h, float(dry)), "dry")
        if formant is not None:
            _check(_LIB.taptools_harmonizer_set_formant(self._h, int(bool(formant))), "formant")
        if glide is not None:
            _check(_LIB.taptools_harmonizer_set_glide(self._h, float(glide)), "glide")
        return self

    def chord(self, intervals, gain: float = 1.0) -> "Harmonizer":
        """Enable the given intervals at equal gain; silence the remaining voices."""
        sts = list(intervals)[:4]
        self.set(intervals=sts + [0.0] * (4 - len(sts)),
                 gains=[gain] * len(sts) + [0.0] * (4 - len(sts)))
        return self

    @property
    def latency(self) -> int:
        return _LIB.taptools_harmonizer_latency(self._h)

    def process(self, x) -> np.ndarray:
        x = _f64(x)
        out = np.zeros_like(x)
        _check(_LIB.taptools_harmonizer_process(self._h, _p64(x), _p64(out), x.size), "process")
        return out

    def clear(self) -> None:
        _check(_LIB.taptools_harmonizer_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_harmonizer_destroy(h)
            self._h = None


class Delay:
    """tap.delay~'s kernel (tap::tools::delay::line): a single slewed feedback
    delay with an equal-power dry/wet mix. interp 1 (default) is 4-point
    Hermite fractional; interp 0 is the legacy integer-sample truncation.
    Feedback is clamped to 0.99 and the loop is DC-blocked."""

    def __init__(self, sr: float = 48000.0, max_ms: float = 2000.0, **params):
        self._h = _LIB.taptools_delay_create()
        _check(_LIB.taptools_delay_prepare(self._h, float(sr), float(max_ms)), "prepare")
        self.set(**params)

    def set(self, *, time_ms=None, feedback=None, mix=None, interp=None,
            smooth_ms=None) -> "Delay":
        # configuration first, so ramped targets in the same call honor the new slew
        if smooth_ms is not None:
            _check(_LIB.taptools_delay_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        if interp is not None:
            _check(_LIB.taptools_delay_set_interp(self._h, int(interp)), "interp")
        if time_ms is not None:
            _check(_LIB.taptools_delay_set_time_ms(self._h, float(time_ms)), "time_ms")
        if feedback is not None:
            _check(_LIB.taptools_delay_set_feedback(self._h, float(feedback)), "feedback")
        if mix is not None:
            _check(_LIB.taptools_delay_set_mix(self._h, float(mix)), "mix")
        return self

    def process(self, x, time_ms=None) -> np.ndarray:
        """Run the line. `time_ms` may be a per-sample array (the signal-rate
        override, which bypasses the time slew); omit it to use the slewed
        set() target."""
        x = _f64(x)
        out = np.zeros_like(x)
        if time_ms is None:
            _check(_LIB.taptools_delay_process(self._h, _p64(x), _p64(out), x.size), "process")
        else:
            t = _f64(np.broadcast_to(np.asarray(time_ms, dtype=np.float64), x.shape))
            _check(_LIB.taptools_delay_process_mod(self._h, _p64(x), _p64(t), _p64(out), x.size),
                   "process_mod")
        return out

    def clear(self) -> None:
        _check(_LIB.taptools_delay_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_delay_destroy(h)
            self._h = None


class Multitap:
    """tap.multitap~'s kernel (tap::tools::delay::multitap): up to 100 slewed
    feedforward taps off one line, each with time (ms), linear gain, and
    equal-power pan (-1..1), summed to stereo. No dry path, no feedback."""

    def __init__(self, sr: float = 48000.0, max_ms: float = 2000.0, **params):
        self._h = _LIB.taptools_multitap_create()
        _check(_LIB.taptools_multitap_prepare(self._h, float(sr), float(max_ms)), "prepare")
        self.set(**params)

    def set(self, *, taps=None, times=None, gains=None, pans=None, interp=None,
            smooth_ms=None) -> "Multitap":
        """`times`/`gains`/`pans` are per-tap sequences; if `taps` (the active
        count) is omitted and `times` is given, the count follows len(times)."""
        # configuration first, so ramped targets in the same call honor the new slew
        if smooth_ms is not None:
            _check(_LIB.taptools_multitap_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        if interp is not None:
            _check(_LIB.taptools_multitap_set_interp(self._h, int(interp)), "interp")
        if times is not None:
            for i, ms in enumerate(times):
                _check(_LIB.taptools_multitap_set_time_ms(self._h, i, float(ms)), "time_ms")
            if taps is None:
                taps = len(list(times))
        if gains is not None:
            for i, g in enumerate(gains):
                _check(_LIB.taptools_multitap_set_gain(self._h, i, float(g)), "gain")
        if pans is not None:
            for i, p in enumerate(pans):
                _check(_LIB.taptools_multitap_set_pan(self._h, i, float(p)), "pan")
        if taps is not None:
            _check(_LIB.taptools_multitap_set_taps(self._h, int(taps)), "taps")
        return self

    def process(self, x):
        """Returns the stereo tap sum as an (outL, outR) pair."""
        x = _f64(x)
        out_l = np.zeros_like(x)
        out_r = np.zeros_like(x)
        _check(_LIB.taptools_multitap_process(self._h, _p64(x), _p64(out_l), _p64(out_r), x.size),
               "process")
        return out_l, out_r

    def clear(self) -> None:
        _check(_LIB.taptools_multitap_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_multitap_destroy(h)
            self._h = None


class Discreet:
    """tap.discreet~'s kernel (tap::tools::discreet::machine): the Discreet
    Music two-tape-machine regeneration loop. Regen legally reaches 1.0 —
    stability comes from the wear path (darkening lowpass, bounded soft
    saturation, DC blocker), not a feedback cap. Loop-time changes glide as
    tape-speed doppler; wow/flutter are a deterministic periodic transport."""

    def __init__(self, sr: float = 48000.0, max_loop_seconds: float = 30.0, **params):
        self._h = _LIB.taptools_discreet_create()
        _check(_LIB.taptools_discreet_prepare(self._h, float(sr), float(max_loop_seconds)),
               "prepare")
        self.set(**params)

    def set(self, *, loop_seconds=None, regen=None, darken_hz=None, drive=None,
            input_level=None, mix=None, wow=None, flutter=None, smooth_ms=None) -> "Discreet":
        """`wow` and `flutter` take (depth_ms, rate_hz) pairs."""
        # configuration first, so ramped targets in the same call honor the new slew
        if smooth_ms is not None:
            _check(_LIB.taptools_discreet_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        if wow is not None:
            depth, rate = wow
            _check(_LIB.taptools_discreet_set_wow(self._h, float(depth), float(rate)), "wow")
        if flutter is not None:
            depth, rate = flutter
            _check(_LIB.taptools_discreet_set_flutter(self._h, float(depth), float(rate)),
                   "flutter")
        if loop_seconds is not None:
            _check(_LIB.taptools_discreet_set_loop_seconds(self._h, float(loop_seconds)),
                   "loop_seconds")
        if regen is not None:
            _check(_LIB.taptools_discreet_set_regen(self._h, float(regen)), "regen")
        if darken_hz is not None:
            _check(_LIB.taptools_discreet_set_darken_hz(self._h, float(darken_hz)), "darken_hz")
        if drive is not None:
            _check(_LIB.taptools_discreet_set_drive(self._h, float(drive)), "drive")
        if input_level is not None:
            _check(_LIB.taptools_discreet_set_input_level(self._h, float(input_level)),
                   "input_level")
        if mix is not None:
            _check(_LIB.taptools_discreet_set_mix(self._h, float(mix)), "mix")
        return self

    def process(self, x) -> np.ndarray:
        x = _f64(x)
        out = np.zeros_like(x)
        _check(_LIB.taptools_discreet_process(self._h, _p64(x), _p64(out), x.size), "process")
        return out

    def clear(self) -> None:
        """The eject button: erases the tape, keeps the parameters."""
        _check(_LIB.taptools_discreet_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_discreet_destroy(h)
            self._h = None


class TapEcho:
    """tap.tapecho~'s kernel (tap::tools::tapecho::machine): the multi-head
    tape echo of the Copicat / Space Echo school, composed over the same
    tape_loop.h machinery as `Discreet`. One motor (`span_ms`) sets the
    delay of a ratio-1.0 head and moves every head together; up to four
    heads sit at settable positions along the path with their own level and
    equal-power pan. Regeneration may pass 1.0 into deliberate
    self-oscillation, bounded by the saturator rather than a feedback cap —
    at drive 0 the effective regen is capped back to 1.0. Mono in, stereo
    out."""

    def __init__(self, sr: float = 48000.0, max_span_seconds: float = 4.0, **params):
        self._h = _LIB.taptools_tapecho_create()
        _check(_LIB.taptools_tapecho_prepare(self._h, float(sr), float(max_span_seconds)),
               "prepare")
        self.set(**params)

    def set(self, *, span_ms=None, heads=None, ratios=None, levels=None, pans=None,
            regen=None, darken_hz=None, drive=None, input_level=None, mix=None,
            wow=None, flutter=None, smooth_ms=None) -> "TapEcho":
        """`ratios`/`levels`/`pans` are per-head sequences (head i gets element
        i); `wow` and `flutter` take (depth_ms, rate_hz) pairs."""
        # configuration first, so ramped targets in the same call honor the new slew
        if smooth_ms is not None:
            _check(_LIB.taptools_tapecho_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        if wow is not None:
            depth, rate = wow
            _check(_LIB.taptools_tapecho_set_wow(self._h, float(depth), float(rate)), "wow")
        if flutter is not None:
            depth, rate = flutter
            _check(_LIB.taptools_tapecho_set_flutter(self._h, float(depth), float(rate)),
                   "flutter")
        if ratios is not None:
            for i, r in enumerate(ratios):
                _check(_LIB.taptools_tapecho_set_head_ratio(self._h, i, float(r)), "head_ratio")
            if heads is None:
                heads = len(list(ratios))
        if heads is not None:
            _check(_LIB.taptools_tapecho_set_heads(self._h, int(heads)), "heads")
        if levels is not None:
            for i, v in enumerate(levels):
                _check(_LIB.taptools_tapecho_set_head_level(self._h, i, float(v)), "head_level")
        if pans is not None:
            for i, p in enumerate(pans):
                _check(_LIB.taptools_tapecho_set_head_pan(self._h, i, float(p)), "head_pan")
        if span_ms is not None:
            _check(_LIB.taptools_tapecho_set_span_ms(self._h, float(span_ms)), "span_ms")
        if regen is not None:
            _check(_LIB.taptools_tapecho_set_regen(self._h, float(regen)), "regen")
        if darken_hz is not None:
            _check(_LIB.taptools_tapecho_set_darken_hz(self._h, float(darken_hz)), "darken_hz")
        if drive is not None:
            _check(_LIB.taptools_tapecho_set_drive(self._h, float(drive)), "drive")
        if input_level is not None:
            _check(_LIB.taptools_tapecho_set_input_level(self._h, float(input_level)),
                   "input_level")
        if mix is not None:
            _check(_LIB.taptools_tapecho_set_mix(self._h, float(mix)), "mix")
        return self

    def process(self, x):
        x = _f64(x)
        out_l = np.zeros_like(x)
        out_r = np.zeros_like(x)
        _check(_LIB.taptools_tapecho_process(self._h, _p64(x), _p64(out_l), _p64(out_r), x.size),
               "process")
        return out_l, out_r

    def clear(self) -> None:
        """Erase the tape and the transport/wear state; parameters are kept.
        Also the fastest way to stop a self-oscillating loop."""
        _check(_LIB.taptools_tapecho_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_tapecho_destroy(h)
            self._h = None


class Plate:
    """The metallique's body on its own (tap::tools::diffuseur::plate) — eight
    driven modes at the free circular plate's transverse ratios, each split
    into a beating doublet, with no driver in front of it. A component, not an
    external: reachable so that a measurement of the body is not silently a
    measurement of the transducer too."""

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_plate_create()
        _check(_LIB.taptools_plate_prepare(self._h, float(sr)), "prepare")
        self.set(**params)

    def set(self, *, pitch_hz=None, decay=None, tilt=None, brightness=None) -> "Plate":
        for value, fn, name in (
            (pitch_hz, _LIB.taptools_plate_set_pitch_hz, "pitch_hz"),
            (decay, _LIB.taptools_plate_set_decay, "decay"),
            (tilt, _LIB.taptools_plate_set_tilt, "tilt"),
            (brightness, _LIB.taptools_plate_set_brightness, "brightness"),
        ):
            if value is not None:
                _check(fn(self._h, float(value)), name)
        return self

    def modes(self):
        """Where the eight modes landed: (Hz, doublet weight) arrays."""
        hz = np.array([_LIB.taptools_plate_mode_hz(self._h, i) for i in range(8)])
        lv = np.array([_LIB.taptools_plate_mode_level(self._h, i) for i in range(8)])
        return hz, lv

    def process(self, x) -> np.ndarray:
        x = _f64(x)
        out = np.zeros_like(x)
        _check(_LIB.taptools_plate_process(self._h, _p64(x), _p64(out), x.size), "process")
        return out

    def clear(self) -> None:
        _check(_LIB.taptools_plate_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_plate_destroy(h)
            self._h = None


class Transducer:
    """The diffuseurs' moving-iron driver on its own
    (tap::tools::diffuseur::transducer) — a component, not an external.

    `asymmetry` is the moving-iron squared term: force follows the square of
    the gap flux, so with a bias current the residual i² puts a second harmonic
    on the output at exactly asymmetry x amplitude / 2 relative to the
    fundamental, and nothing at the third. `saturation` is the bounding stage
    (0 is exactly linear); the output is bounded by 2/saturation rather than
    1/saturation, because taking the DC out of a hard-driven squared law
    doubles the worst-case swing."""

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_transducer_create()
        _check(_LIB.taptools_transducer_prepare(self._h, float(sr)), "prepare")
        self.set(**params)

    def set(self, *, drive=None, asymmetry=None, saturation=None) -> "Transducer":
        for value, fn, name in (
            (drive, _LIB.taptools_transducer_set_drive, "drive"),
            (asymmetry, _LIB.taptools_transducer_set_asymmetry, "asymmetry"),
            (saturation, _LIB.taptools_transducer_set_saturation, "saturation"),
        ):
            if value is not None:
                _check(fn(self._h, float(value)), name)
        return self

    def process(self, x) -> np.ndarray:
        x = _f64(x)
        out = np.zeros_like(x)
        _check(_LIB.taptools_transducer_process(self._h, _p64(x), _p64(out), x.size), "process")
        return out

    def clear(self) -> None:
        _check(_LIB.taptools_transducer_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_transducer_destroy(h)
            self._h = None


class Metallique:
    """tap.metallique~'s kernel (tap::tools::diffuseur::metallique): the Ondes
    Martenot's motor-driven gong diffuseur, as a *driven* resonator — a
    moving-iron transducer feeding a bank of plate modes, in that order,
    because that is the order the instrument wires them.

    The mode ratios are Fletcher & Rossing's free circular plate (Rayleigh's
    Chladni set, 1 : 1.730 : 2.328 : 3.910 : 4.110 : 6.300 : 6.710 : 7.340),
    each split into a slowly beating doublet. No ondes-specific modal
    measurement exists in any of the sources, so the body is a **recreation of
    the general physics**, not a model of Martenot's instrument.

    `drive` / `asymmetry` / `saturation` are the transducer: asymmetry is the
    moving-iron squared term (force follows the square of the gap flux), and
    saturation is the bounding stage that keeps the squared law finite. Neither
    coefficient is fitted to a measurement — set both to 0 for a linear body."""

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_metallique_create()
        _check(_LIB.taptools_metallique_prepare(self._h, float(sr)), "prepare")
        self.set(**params)

    def set(self, *, pitch_hz=None, decay=None, tilt=None, brightness=None, drive=None,
            asymmetry=None, saturation=None, mix=None, level=None, smooth_ms=None) -> "Metallique":
        # configuration first, so ramped targets in the same call honor the new slew
        if smooth_ms is not None:
            _check(_LIB.taptools_metallique_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        for value, fn, name in (
            (pitch_hz, _LIB.taptools_metallique_set_pitch_hz, "pitch_hz"),
            (decay, _LIB.taptools_metallique_set_decay, "decay"),
            (tilt, _LIB.taptools_metallique_set_tilt, "tilt"),
            (brightness, _LIB.taptools_metallique_set_brightness, "brightness"),
            (drive, _LIB.taptools_metallique_set_drive, "drive"),
            (asymmetry, _LIB.taptools_metallique_set_asymmetry, "asymmetry"),
            (saturation, _LIB.taptools_metallique_set_saturation, "saturation"),
            (mix, _LIB.taptools_metallique_set_mix, "mix"),
            (level, _LIB.taptools_metallique_set_level, "level"),
        ):
            if value is not None:
                _check(fn(self._h, float(value)), name)
        return self

    def modes(self):
        """Where the eight modes landed: (Hz, doublet weight) arrays."""
        hz = np.array([_LIB.taptools_metallique_mode_hz(self._h, i) for i in range(8)])
        lv = np.array([_LIB.taptools_metallique_mode_level(self._h, i) for i in range(8)])
        return hz, lv

    def process(self, x) -> np.ndarray:
        x = _f64(x)
        out = np.zeros_like(x)
        _check(_LIB.taptools_metallique_process(self._h, _p64(x), _p64(out), x.size), "process")
        return out

    def clear(self) -> None:
        """Silence the body and reset the driver; parameters are untouched."""
        _check(_LIB.taptools_metallique_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_metallique_destroy(h)
            self._h = None


class Palme:
    """tap.palme~'s kernel (tap::tools::diffuseur::palme): the Ondes Martenot's
    string diffuseur — an electromagnet driving twelve metal strings on a
    soundboard, here a moving-iron transducer into twelve damped waveguide
    loops. Only the strings whose partials line up with the drive ring loudly,
    which is the halo the instrument is known for.

    **Twelve** strings, per the peer-reviewed source; the widely copied
    hobbyist figure of twenty-four is not followed. Their tuning is not
    published anywhere found, so it is a parameter: `tuning` 0 lays them out
    chromatically across an octave from `root_hz` (a string for every pitch
    class), 1 as the harmonic series on the root.

    Note that `decay` and `damping` are not independent — a heavily damped
    string cannot ring for the time you ask, and `string_feedback()` shows the
    loop gain pinned at its cap when they fight."""

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_palme_create()
        _check(_LIB.taptools_palme_prepare(self._h, float(sr)), "prepare")
        self.set(**params)

    def set(self, *, root_hz=None, tuning=None, decay=None, damping=None, detune=None,
            drive=None, asymmetry=None, saturation=None, mix=None, level=None,
            smooth_ms=None) -> "Palme":
        if tuning is not None:
            _check(_LIB.taptools_palme_set_tuning(self._h, int(tuning)), "tuning")
        if smooth_ms is not None:
            _check(_LIB.taptools_palme_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        for value, fn, name in (
            (root_hz, _LIB.taptools_palme_set_root_hz, "root_hz"),
            (decay, _LIB.taptools_palme_set_decay, "decay"),
            (damping, _LIB.taptools_palme_set_damping, "damping"),
            (detune, _LIB.taptools_palme_set_detune, "detune"),
            (drive, _LIB.taptools_palme_set_drive, "drive"),
            (asymmetry, _LIB.taptools_palme_set_asymmetry, "asymmetry"),
            (saturation, _LIB.taptools_palme_set_saturation, "saturation"),
            (mix, _LIB.taptools_palme_set_mix, "mix"),
            (level, _LIB.taptools_palme_set_level, "level"),
        ):
            if value is not None:
                _check(fn(self._h, float(value)), name)
        return self

    def strings(self):
        """Where the twelve strings ended up: (Hz, loop gain) arrays."""
        hz = np.array([_LIB.taptools_palme_string_hz(self._h, i) for i in range(12)])
        fb = np.array([_LIB.taptools_palme_string_feedback(self._h, i) for i in range(12)])
        return hz, fb

    def process(self, x) -> np.ndarray:
        x = _f64(x)
        out = np.zeros_like(x)
        _check(_LIB.taptools_palme_process(self._h, _p64(x), _p64(out), x.size), "process")
        return out

    def clear(self) -> None:
        """Damp every string and reset the driver; parameters are untouched."""
        _check(_LIB.taptools_palme_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_palme_destroy(h)
            self._h = None


class Scrub:
    """tap.scrub~'s kernel (tap::tools::scrub::machine): a granular scrub pad
    over live capture. The tape is stammer.h's `capture` — the same live reel,
    shared rather than copied — and the playhead is a Hann-windowed grain
    scheduler whose position and pitch are two independent performable signals.

    `position_ms` is a lag behind the live edge; `pitch` transposes without the
    position moving; `freeze` stops the recorder so the position addresses
    fixed tape; `drift` walks the playhead through the tape on its own.

    Hann satisfies the overlap-add condition at hop = size/overlap, so at
    overlap 2 with pitch 0, no spray and a held whole-sample position, the
    scrub *is* the input delayed — pinned to under 1e-12 by the kernel tests."""

    def __init__(self, sr: float = 48000.0, max_history_ms: float = 4000.0, **params):
        self._h = _LIB.taptools_scrub_create()
        _check(_LIB.taptools_scrub_prepare(self._h, float(sr), float(max_history_ms)), "prepare")
        self.set(**params)

    def set(self, *, position_ms=None, pitch=None, drift=None, freeze=None, size_ms=None,
            overlap=None, spray_ms=None, seed=None, mix=None, level=None,
            smooth_ms=None) -> "Scrub":
        if overlap is not None:
            _check(_LIB.taptools_scrub_set_overlap(self._h, int(overlap)), "overlap")
        if freeze is not None:
            _check(_LIB.taptools_scrub_set_freeze(self._h, 1 if freeze else 0), "freeze")
        if seed is not None:
            _check(_LIB.taptools_scrub_set_seed(self._h, int(seed)), "seed")
        if smooth_ms is not None:
            _check(_LIB.taptools_scrub_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        for value, fn, name in (
            (position_ms, _LIB.taptools_scrub_set_position_ms, "position_ms"),
            (pitch, _LIB.taptools_scrub_set_pitch, "pitch"),
            (drift, _LIB.taptools_scrub_set_drift, "drift"),
            (size_ms, _LIB.taptools_scrub_set_size_ms, "size_ms"),
            (spray_ms, _LIB.taptools_scrub_set_spray_ms, "spray_ms"),
            (mix, _LIB.taptools_scrub_set_mix, "mix"),
            (level, _LIB.taptools_scrub_set_level, "level"),
        ):
            if value is not None:
                _check(fn(self._h, float(value)), name)
        return self

    @property
    def active_grains(self) -> int:
        return int(_LIB.taptools_scrub_active_grains(self._h))

    def process(self, x, position_ms=None, pitch=None) -> np.ndarray:
        """Run n samples. Pass arrays for `position_ms` / `pitch` to drive the
        performance surface at signal rate (both must be given together)."""
        x = _f64(x)
        out = np.zeros_like(x)
        if position_ms is None and pitch is None:
            _check(_LIB.taptools_scrub_process(self._h, _p64(x), _p64(out), x.size), "process")
        else:
            pos = _f64(np.broadcast_to(np.asarray(position_ms if position_ms is not None else 0.0,
                                                  dtype=np.float64), x.shape))
            pit = _f64(np.broadcast_to(np.asarray(pitch if pitch is not None else 0.0,
                                                  dtype=np.float64), x.shape))
            _check(_LIB.taptools_scrub_process_mod(self._h, _p64(x), _p64(pos), _p64(pit),
                                                   _p64(out), x.size), "process_mod")
        return out

    def clear(self) -> None:
        """Erase the tape, kill every grain, and restart the seeded stream."""
        _check(_LIB.taptools_scrub_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_scrub_destroy(h)
            self._h = None


class Touche:
    """tap.touche~'s kernel (tap::tools::touche::key): the Ondes Martenot
    intensity key as a gain law. The curve is not modelled — it is Quartier
    et al.'s published measurement (Acta Acustica 101(2), 2015, Table II),
    interpolated with monotone cubic segments through all seven points: 50 dB
    over 4.5 mm of the key's travel, referenced to 0 dB at full press.
    `position` spans the physical 9.5 mm throw, so the bottom ~45% is silent
    — that dead zone is the key bending before it reaches the powder bag.
    `mode` 0 drives from displacement (primary), 1 from finger force."""

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_touche_create()
        _check(_LIB.taptools_touche_prepare(self._h, float(sr)), "prepare")
        self.set(**params)

    def set(self, *, position=None, position_mm=None, force_n=None, mode=None,
            smooth_ms=None) -> "Touche":
        # configuration first, so ramped targets in the same call honor the new slew
        if mode is not None:
            _check(_LIB.taptools_touche_set_mode(self._h, int(mode)), "mode")
        if smooth_ms is not None:
            _check(_LIB.taptools_touche_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        if position is not None:
            _check(_LIB.taptools_touche_set_position(self._h, float(position)), "position")
        if position_mm is not None:
            _check(_LIB.taptools_touche_set_position_mm(self._h, float(position_mm)), "position_mm")
        if force_n is not None:
            _check(_LIB.taptools_touche_set_force_n(self._h, float(force_n)), "force_n")
        return self

    def gain_at(self, p) -> float:
        """The curve itself: linear gain at a normalized position. No state touched."""
        return float(_LIB.taptools_touche_gain_at(self._h, float(p)))

    def curve(self, n: int = 512):
        """The whole law as (position, linear gain) arrays — for plotting."""
        p = np.linspace(0.0, 1.0, int(n))
        return p, np.array([self.gain_at(v) for v in p])

    def process(self, x, position=None) -> np.ndarray:
        x = _f64(x)
        out = np.zeros_like(x)
        if position is None:
            _check(_LIB.taptools_touche_process(self._h, _p64(x), _p64(out), x.size), "process")
        else:
            pos = _f64(position)
            _check(_LIB.taptools_touche_process_mod(self._h, _p64(x), _p64(pos), _p64(out), x.size),
                   "process_mod")
        return out

    def clear(self) -> None:
        """Return the key to rest (silent)."""
        _check(_LIB.taptools_touche_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_touche_destroy(h)
            self._h = None


class Fuzz:
    """tap.fuzz~'s kernel (tap::tools::fuzz::pedal): a two-stage, tone-stacked
    distortion built on the Yeh/Abel/Smith DAFx-07 simplified cascade
    (conditioning filter -> memoryless nonlinearity -> equalization filter,
    twice), with a bass / contrast / treble voicing section outside the
    nonlinearity. `gain` sweeps the first stage's drive, `edge` the second
    stage's knee sharpness, `asymmetry` buys even harmonics. The clipper pair
    runs oversampled (1/2/4/8x, default 4). A recreation of a circuit class,
    not a component model of any one pedal."""

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_fuzz_create()
        _check(_LIB.taptools_fuzz_prepare(self._h, float(sr)), "prepare")
        self.set(**params)

    def set(self, *, gain=None, edge=None, asymmetry=None, bass=None, treble=None,
            contrast=None, level_db=None, oversample=None, smooth_ms=None) -> "Fuzz":
        # configuration first, so ramped targets in the same call honor the new slew
        if oversample is not None:
            _check(_LIB.taptools_fuzz_set_oversample(self._h, int(oversample)), "oversample")
        if smooth_ms is not None:
            _check(_LIB.taptools_fuzz_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        for name, value in (("gain", gain), ("edge", edge), ("asymmetry", asymmetry),
                            ("bass", bass), ("treble", treble), ("contrast", contrast),
                            ("level_db", level_db)):
            if value is not None:
                _check(getattr(_LIB, "taptools_fuzz_set_" + name)(self._h, float(value)), name)
        return self

    def process(self, x) -> np.ndarray:
        x = _f64(x)
        out = np.zeros_like(x)
        _check(_LIB.taptools_fuzz_process(self._h, _p64(x), _p64(out), x.size), "process")
        return out

    def clear(self) -> None:
        """Flush the filters and the oversampling chain; parameters are kept."""
        _check(_LIB.taptools_fuzz_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_fuzz_destroy(h)
            self._h = None


class Stammer:
    """tap.stammer~'s kernel (tap::tools::stammer::machine): the live
    buffer-stutter rig. The input is captured continuously; on a `step_ms`
    grid the machine rolls dice and re-fires a slice of what just went past
    — `density` how often it grabs, `divisions` how finely it chops (slice =
    step / [1, divisions]), `repeats` how many passes it holds on for,
    `reverse` the per-repeat chance of running backwards, `jump_ms` how far
    further back it may reach. Every draw comes from the family's seeded
    xorshift64*, so a seed is a performance you can replay; at density 0 the
    dice are never rolled and the object is a bitwise bypass. Mono."""

    def __init__(self, sr: float = 48000.0, max_history_ms: float = 4000.0, **params):
        self._h = _LIB.taptools_stammer_create()
        _check(_LIB.taptools_stammer_prepare(self._h, float(sr), float(max_history_ms)),
               "prepare")
        self.set(**params)

    def set(self, *, step_ms=None, density=None, divisions=None, repeats=None, reverse=None,
            jump_ms=None, fade_ms=None, seed=None, input_level=None, mix=None,
            smooth_ms=None) -> "Stammer":
        # configuration first, so ramped targets in the same call honor the new slew
        if smooth_ms is not None:
            _check(_LIB.taptools_stammer_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        if seed is not None:
            _check(_LIB.taptools_stammer_set_seed(self._h, int(seed)), "seed")
        if step_ms is not None:
            _check(_LIB.taptools_stammer_set_step_ms(self._h, float(step_ms)), "step_ms")
        if density is not None:
            _check(_LIB.taptools_stammer_set_density(self._h, float(density)), "density")
        if divisions is not None:
            _check(_LIB.taptools_stammer_set_divisions(self._h, int(divisions)), "divisions")
        if repeats is not None:
            _check(_LIB.taptools_stammer_set_repeats(self._h, int(repeats)), "repeats")
        if reverse is not None:
            _check(_LIB.taptools_stammer_set_reverse(self._h, float(reverse)), "reverse")
        if jump_ms is not None:
            _check(_LIB.taptools_stammer_set_jump_ms(self._h, float(jump_ms)), "jump_ms")
        if fade_ms is not None:
            _check(_LIB.taptools_stammer_set_fade_ms(self._h, float(fade_ms)), "fade_ms")
        if input_level is not None:
            _check(_LIB.taptools_stammer_set_input_level(self._h, float(input_level)),
                   "input_level")
        if mix is not None:
            _check(_LIB.taptools_stammer_set_mix(self._h, float(mix)), "mix")
        return self

    @property
    def playing(self) -> bool:
        """True while a slice is sounding."""
        return bool(_LIB.taptools_stammer_playing(self._h))

    def process(self, x) -> np.ndarray:
        x = _f64(x)
        out = np.zeros_like(x)
        _check(_LIB.taptools_stammer_process(self._h, _p64(x), _p64(out), x.size), "process")
        return out

    def clear(self) -> None:
        """Erase the capture, drop the slice in flight, and rewind the seeded
        stream — the same seed replays the same performance."""
        _check(_LIB.taptools_stammer_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_stammer_destroy(h)
            self._h = None


class Airport:
    """tap.airport~'s kernel (tap::tools::airport::loop_bank): up to eight
    free-running tape loops of unequal, incommensurate lengths, each with a
    single head that both plays and records. No setter ever resets a phase —
    the free-run is the piece. Per-loop level / equal-power pan / darken;
    stereo sum, no dry path."""

    def __init__(self, sr: float = 48000.0, max_loop_seconds: float = 30.0, **params):
        self._h = _LIB.taptools_airport_create()
        _check(_LIB.taptools_airport_prepare(self._h, float(sr), float(max_loop_seconds)),
               "prepare")
        self.set(**params)

    def set(self, *, loops=None, lengths=None, levels=None, pans=None, darkens=None,
            smooth_ms=None) -> "Airport":
        """`lengths`/`levels`/`pans`/`darkens` are per-loop sequences (loop i
        gets element i)."""
        # configuration first, so ramped targets in the same call honor the new slew
        if smooth_ms is not None:
            _check(_LIB.taptools_airport_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        if lengths is not None:
            for i, s in enumerate(lengths):
                _check(_LIB.taptools_airport_set_length_seconds(self._h, i, float(s)), "length")
            if loops is None:
                loops = len(list(lengths))
        if loops is not None:
            _check(_LIB.taptools_airport_set_loops(self._h, int(loops)), "loops")
        if levels is not None:
            for i, v in enumerate(levels):
                _check(_LIB.taptools_airport_set_level(self._h, i, float(v)), "level")
        if pans is not None:
            for i, p in enumerate(pans):
                _check(_LIB.taptools_airport_set_pan(self._h, i, float(p)), "pan")
        if darkens is not None:
            for i, hz in enumerate(darkens):
                _check(_LIB.taptools_airport_set_darken_hz(self._h, i, float(hz)), "darken")
        return self

    def record(self, loop: int, on: bool) -> "Airport":
        """Punch the process() input onto this loop's tape (True) or freeze it
        bit-exactly (False). Recording starts wherever the head happens to be."""
        _check(_LIB.taptools_airport_record(self._h, int(loop), 1 if on else 0), "record")
        return self

    def phase(self, loop: int) -> float:
        """This loop's head position as a fraction of its length, 0..1."""
        return float(_LIB.taptools_airport_phase(self._h, int(loop)))

    @property
    def composite_period_seconds(self) -> float:
        """lcm of the active loop lengths (seconds); inf once it overflows."""
        return float(_LIB.taptools_airport_composite_period_seconds(self._h))

    def process(self, x):
        x = _f64(x)
        out_l = np.zeros_like(x)
        out_r = np.zeros_like(x)
        _check(_LIB.taptools_airport_process(self._h, _p64(x), _p64(out_l), _p64(out_r), x.size),
               "process")
        return out_l, out_r

    def clear(self) -> None:
        """Erase every tape and rewind every head; parameters are untouched."""
        _check(_LIB.taptools_airport_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_airport_destroy(h)
            self._h = None


class Garden:
    """tap.garden~'s kernel (tap::tools::garden::bed): a generative event
    loop on the Bloom principle. Planted notes snap to the scale, strike a
    small modal wind chime (mode ratios by material: 0 chime, the free-free
    tube's 1 : 2.756 : 5.404 : 8.933; 1 bar, the tuned bar's 1 : 4 : 10 : 20),
    and return every loop pass a step quieter (decay) and purer (soften)
    until they retire below the floor; left idle, a seeded gardener plants
    for you. Each tube hangs at a fixed stereo seat (width set by spread)
    with its own fixed upper-mode scatter, both keyed by pitch. Scales:
    0 chromatic, 1 major, 2 minor, 3 major pentatonic, 4 minor pentatonic."""

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_garden_create()
        _check(_LIB.taptools_garden_prepare(self._h, float(sr)), "prepare")
        self.set(**params)

    def set(self, *, loop_seconds=None, decay=None, soften=None, floor=None, bell=None,
            material=None, spread=None, root=None, scale=None, idle_seconds=None, gust=None,
            seed=None, level=None, smooth_ms=None) -> "Garden":
        """`bell` takes an (attack_s, decay_s, brightness) triple."""
        # configuration first, so ramped targets in the same call honor the new slew
        if smooth_ms is not None:
            _check(_LIB.taptools_garden_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        if seed is not None:
            _check(_LIB.taptools_garden_set_seed(self._h, int(seed)), "seed")
        if material is not None:
            _check(_LIB.taptools_garden_set_material(self._h, int(material)), "material")
        if spread is not None:
            _check(_LIB.taptools_garden_set_spread(self._h, float(spread)), "spread")
        if root is not None:
            _check(_LIB.taptools_garden_set_root(self._h, int(root)), "root")
        if scale is not None:
            _check(_LIB.taptools_garden_set_scale(self._h, int(scale)), "scale")
        if bell is not None:
            attack_s, decay_s, brightness = bell
            _check(_LIB.taptools_garden_set_bell(self._h, float(attack_s), float(decay_s),
                                                 float(brightness)), "bell")
        if loop_seconds is not None:
            _check(_LIB.taptools_garden_set_loop_seconds(self._h, float(loop_seconds)),
                   "loop_seconds")
        if decay is not None:
            _check(_LIB.taptools_garden_set_decay(self._h, float(decay)), "decay")
        if soften is not None:
            _check(_LIB.taptools_garden_set_soften(self._h, float(soften)), "soften")
        if floor is not None:
            _check(_LIB.taptools_garden_set_floor(self._h, float(floor)), "floor")
        if idle_seconds is not None:
            _check(_LIB.taptools_garden_set_idle_seconds(self._h, float(idle_seconds)),
                   "idle_seconds")
        if gust is not None:
            _check(_LIB.taptools_garden_set_gust(self._h, float(gust)), "gust")
        if level is not None:
            _check(_LIB.taptools_garden_set_level(self._h, float(level)), "level")
        return self

    def note(self, pitch: float, velocity: float) -> "Garden":
        """Plant: MIDI pitch (fractional ok, snaps to root/scale at entry),
        velocity (0, 1]. Sounds on the next processed sample."""
        _check(_LIB.taptools_garden_note(self._h, float(pitch), float(velocity)), "note")
        return self

    @property
    def active_events(self) -> int:
        return int(_LIB.taptools_garden_active_events(self._h))

    @property
    def active_voices(self) -> int:
        return int(_LIB.taptools_garden_active_voices(self._h))

    def process(self, n: int) -> tuple[np.ndarray, np.ndarray]:
        """Render n samples of the stereo rack (a source: no input);
        returns (left, right)."""
        out_l = np.zeros(int(n))
        out_r = np.zeros(int(n))
        _check(_LIB.taptools_garden_process(self._h, _p64(out_l), _p64(out_r), out_l.size),
               "process")
        return out_l, out_r

    def clear(self) -> None:
        """Uproot everything; parameters are untouched."""
        _check(_LIB.taptools_garden_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_garden_destroy(h)
            self._h = None


class Reel:
    """One lane of tap.airport~ (tap::tools::airport::loop) — a single
    free-running tape loop with one head that both plays and records. A bank
    is an array of these and nothing more, so summing N of them reproduces
    `Airport` exactly; that is what the null-test cells check."""

    def __init__(self, sr: float = 48000.0, max_loop_seconds: float = 30.0, **params):
        self._h = _LIB.taptools_reel_create()
        _check(_LIB.taptools_reel_prepare(self._h, float(sr), float(max_loop_seconds)), "prepare")
        self.set(**params)

    def set(self, *, length=None, level=None, pan=None, darken=None, smooth_ms=None) -> "Reel":
        # configuration first, so ramped targets in the same call honor the new slew
        if smooth_ms is not None:
            _check(_LIB.taptools_reel_set_smooth_ms(self._h, float(smooth_ms)), "smooth_ms")
        if length is not None:
            _check(_LIB.taptools_reel_set_length_seconds(self._h, float(length)), "length")
        if level is not None:
            _check(_LIB.taptools_reel_set_level(self._h, float(level)), "level")
        if pan is not None:
            _check(_LIB.taptools_reel_set_pan(self._h, float(pan)), "pan")
        if darken is not None:
            _check(_LIB.taptools_reel_set_darken_hz(self._h, float(darken)), "darken")
        return self

    def record(self, on: bool) -> "Reel":
        """Punch the process() input onto the tape (True) or freeze it
        bit-exactly (False). Recording starts wherever the head happens to be."""
        _check(_LIB.taptools_reel_record(self._h, 1 if on else 0), "record")
        return self

    @property
    def phase(self) -> float:
        """The head position as a fraction of the loop length, 0..1."""
        return float(_LIB.taptools_reel_phase(self._h))

    @property
    def length_seconds(self) -> float:
        """The loop length, as quantized to a whole number of samples."""
        return float(_LIB.taptools_reel_length_seconds(self._h))

    @property
    def loop_samples(self) -> int:
        return int(_LIB.taptools_reel_loop_samples(self._h))

    def process(self, x):
        x = _f64(x)
        out_l = np.zeros_like(x)
        out_r = np.zeros_like(x)
        _check(_LIB.taptools_reel_process(self._h, _p64(x), _p64(out_l), _p64(out_r), x.size),
               "process")
        return out_l, out_r

    def clear(self) -> None:
        """Erase the tape and rewind the head; parameters are untouched."""
        _check(_LIB.taptools_reel_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_reel_destroy(h)
            self._h = None


class Chime:
    """tap.garden~'s voice rack (tap::tools::garden::rack) — the fixed pool of
    16 wind chimes plus the quietest-first allocator, which steals by re-aiming
    rather than resetting so a steal glides instead of clicking. The allocator
    is kernel code on purpose: Max's poly~ steals round-robin and does not
    exist off Max."""

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_chime_create()
        _check(_LIB.taptools_chime_prepare(self._h, float(sr)), "prepare")
        self.set(**params)

    def set(self, *, attack=None, decay=None, material=None, spread=None) -> "Chime":
        if attack is not None or decay is not None:
            a = 0.004 if attack is None else float(attack)
            d = 4.0 if decay is None else float(decay)
            _check(_LIB.taptools_chime_set_times(self._h, a, d), "times")
        if material is not None:
            _check(_LIB.taptools_chime_set_material(self._h, int(material)), "material")
        if spread is not None:
            _check(_LIB.taptools_chime_set_spread(self._h, float(spread)), "spread")
        return self

    def strike(self, pitch: float, velocity: float = 1.0, brightness: float = 1.0) -> "Chime":
        """Strike the tube at a MIDI pitch (fractional pitches are distinct
        tubes, with their own scatter and their own seat on the rack)."""
        _check(_LIB.taptools_chime_strike(self._h, float(pitch), float(velocity), float(brightness)),
               "strike")
        return self

    def strike_hz(self, freq_hz: float, velocity: float = 1.0, brightness: float = 1.0) -> "Chime":
        _check(_LIB.taptools_chime_strike_hz(self._h, float(freq_hz), float(velocity),
                                            float(brightness)), "strike_hz")
        return self

    @property
    def active_voices(self) -> int:
        return int(_LIB.taptools_chime_active_voices(self._h))

    def process(self, n: int):
        """A source: render n samples of the stereo rack."""
        out_l = np.zeros(int(n))
        out_r = np.zeros(int(n))
        _check(_LIB.taptools_chime_process(self._h, _p64(out_l), _p64(out_r), out_l.size), "process")
        return out_l, out_r

    VOICES = 16

    def process_voices(self, n: int, voices: int = 16):
        """Render n samples of each voice, RAW — before each bell's seat is
        applied. Returns a (voices, n) array. This is the same advance as
        process(); take one or the other for a given span, never both."""
        out = np.zeros((int(voices), int(n)))
        _check(_LIB.taptools_chime_process_voices(self._h, _p64(out), int(voices), int(n)),
               "process_voices")
        return out

    def voice_hz(self, voice: int) -> float:
        """Which tube this voice is holding, in Hz (0 if never struck). The pool
        reassigns bells as it steals, so voice i is whatever was last put there."""
        return float(_LIB.taptools_chime_voice_hz(self._h, int(voice)))

    def voice_level(self, voice: int) -> float:
        return float(_LIB.taptools_chime_voice_level(self._h, int(voice)))

    def voice_gains(self, voice: int):
        """The seat gains process() would multiply this voice's mono sum by —
        enough to rebuild the stereo rack from the per-voice taps."""
        return (float(_LIB.taptools_chime_voice_gain_left(self._h, int(voice))),
                float(_LIB.taptools_chime_voice_gain_right(self._h, int(voice))))

    def clear(self) -> None:
        _check(_LIB.taptools_chime_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_chime_destroy(h)
            self._h = None


class Bloom:
    """tap.garden~'s event ring (tap::tools::garden::ring) — plant a bloom and
    it returns at its own loop position every pass, velocity times `decay` and
    brightness times `soften`, retiring below `floor`. It knows nothing about
    chimes: it emits strikes, and what sounds them is your business. A plant at
    velocity v lives exactly ceil(log(floor/v)/log(decay)) strikes."""

    _MAX_EVENTS = 64

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_bloom_create()
        _check(_LIB.taptools_bloom_prepare(self._h, float(sr)), "prepare")
        self._pitch = np.zeros(self._MAX_EVENTS)
        self._vel = np.zeros(self._MAX_EVENTS)
        self._bright = np.zeros(self._MAX_EVENTS)
        self.set(**params)

    def set(self, *, loop_seconds=None, decay=None, soften=None, floor=None,
            brightness=None) -> "Bloom":
        if loop_seconds is not None:
            _check(_LIB.taptools_bloom_set_loop_seconds(self._h, float(loop_seconds)), "loop")
        if decay is not None:
            _check(_LIB.taptools_bloom_set_decay(self._h, float(decay)), "decay")
        if soften is not None:
            _check(_LIB.taptools_bloom_set_soften(self._h, float(soften)), "soften")
        if floor is not None:
            _check(_LIB.taptools_bloom_set_floor(self._h, float(floor)), "floor")
        if brightness is not None:
            _check(_LIB.taptools_bloom_set_brightness(self._h, float(brightness)), "brightness")
        return self

    def plant(self, pitch: float, velocity: float) -> "Bloom":
        """Plant at the current loop position; it fires on the next due()."""
        _check(_LIB.taptools_bloom_plant(self._h, float(pitch), float(velocity)), "plant")
        return self

    def due(self):
        """The strikes due on this sample as a list of (pitch, velocity,
        brightness). Fires and wears them, but does NOT advance — call step()."""
        n = _LIB.taptools_bloom_due(self._h, _p64(self._pitch), _p64(self._vel),
                                    _p64(self._bright), self._MAX_EVENTS)
        _check(0 if n >= 0 else -1, "due")
        return [(self._pitch[i], self._vel[i], self._bright[i]) for i in range(n)]

    def step(self) -> None:
        """Advance the loop one sample."""
        _check(_LIB.taptools_bloom_step(self._h), "step")

    @property
    def active_events(self) -> int:
        return int(_LIB.taptools_bloom_active_events(self._h))

    @property
    def loop_samples(self) -> int:
        return int(_LIB.taptools_bloom_loop_samples(self._h))

    def clear(self) -> None:
        _check(_LIB.taptools_bloom_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_bloom_destroy(h)
            self._h = None


class Gardener:
    """tap.garden~'s idle wind (tap::tools::garden::gardener) — after
    `idle_seconds` without a caller plant, strikes arrive on a calm/gust cycle.
    The rng is drawn from ONLY while idling, which is what makes the seed triad
    hold. Pitches come out RAW: quantizing them is the caller's job, because the
    scale is the caller's field, not the wind's."""

    def __init__(self, sr: float = 48000.0, **params):
        self._h = _LIB.taptools_gardener_create()
        _check(_LIB.taptools_gardener_prepare(self._h, float(sr)), "prepare")
        self._pitch = np.zeros(1)
        self._vel = np.zeros(1)
        self.set(**params)

    def set(self, *, idle_seconds=None, gust=None, seed=None) -> "Gardener":
        if idle_seconds is not None:
            _check(_LIB.taptools_gardener_set_idle_seconds(self._h, float(idle_seconds)), "idle")
        if gust is not None:
            _check(_LIB.taptools_gardener_set_gust(self._h, float(gust)), "gust")
        if seed is not None:
            _check(_LIB.taptools_gardener_set_seed(self._h, int(seed)), "seed")
        return self

    def notice_plant(self) -> "Gardener":
        """A caller planted: close the idle gate."""
        _check(_LIB.taptools_gardener_notice_plant(self._h), "notice_plant")
        return self

    def tick(self, loop_samples: int):
        """Advance one sample. Returns (pitch, velocity) if the wind wants a
        strike this sample, else None."""
        n = _LIB.taptools_gardener_tick(self._h, int(loop_samples), _p64(self._pitch),
                                        _p64(self._vel))
        _check(0 if n >= 0 else -1, "tick")
        return (self._pitch[0], self._vel[0]) if n == 1 else None

    def clear(self) -> None:
        """Re-seed the rng and restart the idle clock and the wind."""
        _check(_LIB.taptools_gardener_clear(self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_gardener_destroy(h)
            self._h = None


def composite_period_seconds(loop_seconds, sr: float = 48000.0) -> float:
    """How long until a set of free-running loops realigns, in seconds — the lcm
    of their lengths once each is quantized to the sample grid exactly as a reel
    would quantize it. Returns inf once the lcm leaves the 64-bit range, which
    incommensurate lengths reach fast; that is the point, not a failure. This is
    what `tap.period` wraps, and what `Airport.composite_period_seconds` reports
    for a bank."""
    x = _f64(np.asarray(loop_seconds, dtype=float).ravel())
    return float(_LIB.taptools_composite_period_seconds(_p64(x), x.size, float(sr)))


def scale_quantize(pitch, root: int = 0, scale: int = 3):
    """tap.garden~'s entry quantizer (tap::tools::garden::scale_quantizer):
    snap MIDI semitones to the nearest pitch in root/scale. `scale` indexes
    garden::scale_index — 0 chromatic, 1 major, 2 minor, 3 major pentatonic,
    4 minor pentatonic. Accepts a scalar or an array."""
    if np.isscalar(pitch):
        return float(_LIB.taptools_scale_quantize(float(pitch), int(root), int(scale)))
    return np.array([float(_LIB.taptools_scale_quantize(float(p), int(root), int(scale)))
                     for p in np.asarray(pitch, dtype=float)])


class Yin:
    """The shared DspTap pitch detector (tap::dsp::yin), passed through the C ABI
    so the notebooks can track pitch with the same detector the corrector uses."""

    def __init__(self, window: int = 873, tau_min: int = 24, tau_max: int = 873):
        self._h = _LIB.taptools_yin_create(window, tau_min, tau_max)
        if not self._h:
            raise ValueError("bad yin geometry")

    @property
    def frame_size(self) -> int:
        return _LIB.taptools_yin_frame_size(self._h)

    def track(self, x, hop: int = 256) -> np.ndarray:
        """Fractional periods in samples (0 = unvoiced) every `hop` samples."""
        x = _f64(x)
        out = np.zeros(x.size // hop + 1)
        n = _LIB.taptools_yin_track(self._h, _p64(x), x.size, hop, _p64(out), out.size)
        return out[:max(n, 0)]

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_yin_destroy(h)
            self._h = None
