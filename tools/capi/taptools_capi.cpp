/// @file taptools_capi.cpp
/// @brief C ABI implementation — see taptools_capi.h.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "taptools_capi.h"

// The DSP cores are the same headers the Max externals compile — no Max/Min dependency.
#include <taptools/autowah.h>
#include <taptools/conv_engine.h>
#include <taptools/diode_ladder.h>
#include <taptools/ladder.h>
#include <taptools/step_seq.h>
#include <taptools/svf.h>
#include <taptools/tb303_voice.h>
#include <taptools/tr808_kick.h>
#include <taptools/tune.h>
#include <taptools/vco.h>

using tap::tools::conv_engine;

extern "C" {

taptools_conv taptools_conv_create(void) {
    return static_cast<taptools_conv>(new conv_engine());
}

void taptools_conv_destroy(taptools_conv engine) {
    delete static_cast<conv_engine*>(engine);
}

int taptools_conv_configure(taptools_conv engine, int blocksize, int max_partitions) {
    if (!engine)
        return -1;
    static_cast<conv_engine*>(engine)->configure(blocksize, max_partitions);
    return 0;
}

int taptools_conv_load_ir(taptools_conv engine, const float* ll, const float* lr, const float* rl, const float* rr,
                          int length, double scale) {
    if (!engine)
        return -1;
    const float* paths[conv_engine::k_paths] = {ll, lr, rl, rr};
    static_cast<conv_engine*>(engine)->load_ir(paths, length, scale);
    return 0;
}

int taptools_conv_clear(taptools_conv engine) {
    if (!engine)
        return -1;
    static_cast<conv_engine*>(engine)->clear();
    return 0;
}

int taptools_conv_process(taptools_conv engine, const double* inL, const double* inR, double* outL, double* outR,
                          int n) {
    if (!engine)
        return -1;
    static_cast<conv_engine*>(engine)->process(inL, inR, outL, outR, static_cast<long>(n));
    return 0;
}

int taptools_conv_block_size(taptools_conv engine) {
    return engine ? static_cast<conv_engine*>(engine)->block_size() : -1;
}

int taptools_conv_max_partitions(taptools_conv engine) {
    return engine ? static_cast<conv_engine*>(engine)->max_partitions() : -1;
}

int taptools_conv_has_ir(taptools_conv engine) {
    return (engine && static_cast<conv_engine*>(engine)->has_ir()) ? 1 : 0;
}

} // extern "C"

// ---- shared plumbing for the handle kernels ----------------------------------------------------

namespace {

    template <typename T, typename Fn>
    int with(void* h, Fn&& fn) {
        if (!h)
            return -1;
        fn(*static_cast<T*>(h));
        return 0;
    }

} // namespace

extern "C" {

// ---- tap.svf~ ----------------------------------------------------------------------------------

using tap::tools::svf::svf_filter;

taptools_svf taptools_svf_create(void) {
    return static_cast<taptools_svf>(new svf_filter());
}

void taptools_svf_destroy(taptools_svf h) {
    delete static_cast<svf_filter*>(h);
}

int taptools_svf_prepare(taptools_svf h, double sr) {
    return with<svf_filter>(h, [&](svf_filter& f) { f.prepare(sr, 1); });
}

int taptools_svf_set(taptools_svf h, int param, double value) {
    return with<svf_filter>(h, [&](svf_filter& f) { f.set_param(param, value); });
}

int taptools_svf_set_mode(taptools_svf h, int mode) {
    return with<svf_filter>(h, [&](svf_filter& f) { f.set_mode(mode); });
}

int taptools_svf_set_order(taptools_svf h, int order) {
    return with<svf_filter>(h, [&](svf_filter& f) { f.set_order(order); });
}

int taptools_svf_set_circuit(taptools_svf h, int circuit) {
    return with<svf_filter>(h, [&](svf_filter& f) { f.set_circuit(circuit); });
}

int taptools_svf_set_oversample(taptools_svf h, int os) {
    return with<svf_filter>(h, [&](svf_filter& f) { f.set_oversample(os); });
}

int taptools_svf_set_smooth_ms(taptools_svf h, double ms) {
    return with<svf_filter>(h, [&](svf_filter& f) { f.set_smooth_ms(ms); });
}

int taptools_svf_clear(taptools_svf h) {
    return with<svf_filter>(h, [&](svf_filter& f) { f.clear(); });
}

int taptools_svf_process(taptools_svf h, const double* in, double* out, int n) {
    if (!in || !out)
        return -1;
    return with<svf_filter>(h, [&](svf_filter& f) { f.process(in, out, static_cast<size_t>(n)); });
}

int taptools_svf_process_mod(taptools_svf h, const double* in, const double* cutoff_hz, double* out, int n) {
    if (!in || !cutoff_hz || !out)
        return -1;
    return with<svf_filter>(h, [&](svf_filter& f) {
        for (int i = 0; i < n; ++i) {
            out[i] = f.process(in[i], cutoff_hz[i]);
        }
    });
}

// ---- tap.ladder~ -------------------------------------------------------------------------------

using tap::tools::ladder::ladder_filter;

taptools_ladder taptools_ladder_create(void) {
    return static_cast<taptools_ladder>(new ladder_filter());
}

void taptools_ladder_destroy(taptools_ladder h) {
    delete static_cast<ladder_filter*>(h);
}

int taptools_ladder_prepare(taptools_ladder h, double sr) {
    return with<ladder_filter>(h, [&](ladder_filter& f) { f.prepare(sr); });
}

int taptools_ladder_set(taptools_ladder h, int param, double value) {
    return with<ladder_filter>(h, [&](ladder_filter& f) { f.set_param(param, value); });
}

int taptools_ladder_set_mode(taptools_ladder h, int mode) {
    return with<ladder_filter>(h, [&](ladder_filter& f) { f.set_mode(mode); });
}

int taptools_ladder_set_solver(taptools_ladder h, int solver) {
    return with<ladder_filter>(h, [&](ladder_filter& f) { f.set_solver(solver); });
}

int taptools_ladder_set_oversample(taptools_ladder h, int os) {
    return with<ladder_filter>(h, [&](ladder_filter& f) { f.set_oversample(os); });
}

int taptools_ladder_set_smooth_ms(taptools_ladder h, double ms) {
    return with<ladder_filter>(h, [&](ladder_filter& f) { f.set_smooth_ms(ms); });
}

int taptools_ladder_clear(taptools_ladder h) {
    return with<ladder_filter>(h, [&](ladder_filter& f) { f.clear(); });
}

int taptools_ladder_process(taptools_ladder h, const double* in, double* out, int n) {
    if (!in || !out)
        return -1;
    return with<ladder_filter>(h, [&](ladder_filter& f) { f.process(in, out, static_cast<size_t>(n)); });
}

int taptools_ladder_process_mod(taptools_ladder h, const double* in, const double* cutoff_hz, double* out, int n) {
    if (!in || !cutoff_hz || !out)
        return -1;
    return with<ladder_filter>(h, [&](ladder_filter& f) {
        for (int i = 0; i < n; ++i) {
            out[i] = f.process(in[i], cutoff_hz[i]);
        }
    });
}

// ---- tap.vco~ ----------------------------------------------------------------------------------

using tap::tools::vco::vco_osc;

taptools_vco taptools_vco_create(void) {
    return static_cast<taptools_vco>(new vco_osc());
}

void taptools_vco_destroy(taptools_vco h) {
    delete static_cast<vco_osc*>(h);
}

int taptools_vco_prepare(taptools_vco h, double sr) {
    return with<vco_osc>(h, [&](vco_osc& o) { o.prepare(sr); });
}

int taptools_vco_set(taptools_vco h, int param, double value) {
    return with<vco_osc>(h, [&](vco_osc& o) { o.set_param(param, value); });
}

int taptools_vco_set_seed(taptools_vco h, unsigned int seed) {
    return with<vco_osc>(h, [&](vco_osc& o) { o.set_seed(seed); });
}

int taptools_vco_set_smooth_ms(taptools_vco h, double ms) {
    return with<vco_osc>(h, [&](vco_osc& o) { o.set_smooth_ms(ms); });
}

int taptools_vco_clear(taptools_vco h) {
    return with<vco_osc>(h, [&](vco_osc& o) { o.clear(); });
}

int taptools_vco_process(taptools_vco h, double* out, int n) {
    if (!out)
        return -1;
    return with<vco_osc>(h, [&](vco_osc& o) { o.process(out, static_cast<size_t>(n)); });
}

int taptools_vco_process_mod(taptools_vco h, const double* fm_hz, const double* sync, double* out, int n) {
    if (!out)
        return -1;
    return with<vco_osc>(h, [&](vco_osc& o) {
        for (int i = 0; i < n; ++i) {
            out[i] = o.process(fm_hz ? fm_hz[i] : 0.0, sync ? sync[i] : 0.0);
        }
    });
}

// ---- tap.diode~ --------------------------------------------------------------------------------

using tap::tools::diode::diode_filter;

taptools_diode taptools_diode_create(void) {
    return static_cast<taptools_diode>(new diode_filter());
}

void taptools_diode_destroy(taptools_diode h) {
    delete static_cast<diode_filter*>(h);
}

int taptools_diode_prepare(taptools_diode h, double sr) {
    return with<diode_filter>(h, [&](diode_filter& f) { f.prepare(sr); });
}

int taptools_diode_set(taptools_diode h, int param, double value) {
    return with<diode_filter>(h, [&](diode_filter& f) { f.set_param(param, value); });
}

int taptools_diode_set_solver(taptools_diode h, int solver) {
    return with<diode_filter>(h, [&](diode_filter& f) { f.set_solver(solver); });
}

int taptools_diode_set_oversample(taptools_diode h, int os) {
    return with<diode_filter>(h, [&](diode_filter& f) { f.set_oversample(os); });
}

int taptools_diode_set_smooth_ms(taptools_diode h, double ms) {
    return with<diode_filter>(h, [&](diode_filter& f) { f.set_smooth_ms(ms); });
}

int taptools_diode_clear(taptools_diode h) {
    return with<diode_filter>(h, [&](diode_filter& f) { f.clear(); });
}

int taptools_diode_process(taptools_diode h, const double* in, double* out, int n) {
    if (!h || !in || !out || n < 0) {
        return -1;
    }
    static_cast<diode_filter*>(h)->process(in, out, static_cast<size_t>(n));
    return 0;
}

int taptools_diode_process_mod(taptools_diode h, const double* in, const double* cutoff_hz, double* out, int n) {
    if (!h || !in || !cutoff_hz || !out || n < 0) {
        return -1;
    }
    diode_filter& f = *static_cast<diode_filter*>(h);
    for (int i = 0; i < n; ++i) {
        out[i] = f.process(in[i], cutoff_hz[i]);
    }
    return 0;
}

// ---- tap.303~ ----------------------------------------------------------------------------------

using tb303_voice = tap::tools::tb303::voice;

taptools_tb303 taptools_tb303_create(void) {
    return static_cast<taptools_tb303>(new tb303_voice());
}

void taptools_tb303_destroy(taptools_tb303 h) {
    delete static_cast<tb303_voice*>(h);
}

int taptools_tb303_prepare(taptools_tb303 h, double sr) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.prepare(sr); });
}

int taptools_tb303_set(taptools_tb303 h, int param, double value) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.set_param(param, value); });
}

int taptools_tb303_set_vca(taptools_tb303 h, int mode) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.set_vca(mode); });
}

int taptools_tb303_set_solver(taptools_tb303 h, int solver) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.set_solver(solver); });
}

int taptools_tb303_set_oversample(taptools_tb303 h, int os) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.set_oversample(os); });
}

int taptools_tb303_set_seed(taptools_tb303 h, unsigned int seed) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.set_seed(seed); });
}

int taptools_tb303_set_tolerance(taptools_tb303 h, double t) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.set_tolerance(t); });
}

int taptools_tb303_set_smooth_ms(taptools_tb303 h, double ms) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.set_smooth_ms(ms); });
}

int taptools_tb303_recall(taptools_tb303 h, int slot, double seconds) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.recall_preset(slot, seconds); });
}

int taptools_tb303_clear(taptools_tb303 h) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.clear(); });
}

int taptools_tb303_note_on(taptools_tb303 h, double midi_note, double accent) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.note_on(midi_note, accent); });
}

int taptools_tb303_note_off(taptools_tb303 h) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.note_off(); });
}

int taptools_tb303_set_pitch(taptools_tb303 h, double midi_note) {
    return with<tb303_voice>(h, [&](tb303_voice& v) { v.set_pitch(midi_note); });
}

double taptools_tb303_accent_charge(taptools_tb303 h) {
    if (!h) {
        return 0.0;
    }
    return static_cast<tb303_voice*>(h)->accent_charge();
}

int taptools_tb303_process(taptools_tb303 h, double* out, int n) {
    if (!h || !out || n < 0) {
        return -1;
    }
    static_cast<tb303_voice*>(h)->process(out, static_cast<size_t>(n));
    return 0;
}

// ---- tap.autowah~ ------------------------------------------------------------------------------

using tap::tools::autowah::wah_filter;

taptools_wah taptools_wah_create(void) {
    return static_cast<taptools_wah>(new wah_filter());
}

void taptools_wah_destroy(taptools_wah h) {
    delete static_cast<wah_filter*>(h);
}

int taptools_wah_prepare(taptools_wah h, double sr) {
    return with<wah_filter>(h, [&](wah_filter& w) { w.prepare(sr); });
}

int taptools_wah_set(taptools_wah h, int param, double value) {
    return with<wah_filter>(h, [&](wah_filter& w) { w.set_param(param, value); });
}

int taptools_wah_set_mode(taptools_wah h, int mode) {
    return with<wah_filter>(h, [&](wah_filter& w) { w.set_mode(mode); });
}

int taptools_wah_set_rectifier(taptools_wah h, int rect) {
    return with<wah_filter>(h, [&](wah_filter& w) { w.set_rectifier(rect); });
}

int taptools_wah_set_smooth_ms(taptools_wah h, double ms) {
    return with<wah_filter>(h, [&](wah_filter& w) { w.set_smooth_ms(ms); });
}

int taptools_wah_recall(taptools_wah h, int slot, double seconds) {
    if (!h)
        return -1;
    return static_cast<wah_filter*>(h)->recall_preset(slot, seconds) ? 0 : -1;
}

int taptools_wah_clear(taptools_wah h) {
    return with<wah_filter>(h, [&](wah_filter& w) { w.clear(); });
}

int taptools_wah_process(taptools_wah h, const double* in, const double* key, double* out, double* env_out,
                         double* cutoff_out, int n) {
    if (!in || !out)
        return -1;
    return with<wah_filter>(h, [&](wah_filter& w) {
        for (int i = 0; i < n; ++i) {
            out[i] = key ? w.process(in[i], key[i]) : w.process(in[i]);
            if (env_out) {
                env_out[i] = w.envelope();
            }
            if (cutoff_out) {
                cutoff_out[i] = w.cutoff_hz();
            }
        }
    });
}

// ---- tap.808.seq~ ------------------------------------------------------------------------------

using seq_trigger = tap::tools::seq::trigger_row;

taptools_seqtrig taptools_seqtrig_create(void) {
    return static_cast<taptools_seqtrig>(new seq_trigger());
}

void taptools_seqtrig_destroy(taptools_seqtrig h) {
    delete static_cast<seq_trigger*>(h);
}

int taptools_seqtrig_prepare(taptools_seqtrig h, double sr) {
    return with<seq_trigger>(h, [&](seq_trigger& r) { r.prepare(sr); });
}

int taptools_seqtrig_set_length(taptools_seqtrig h, int steps) {
    return with<seq_trigger>(h, [&](seq_trigger& r) { r.clock().data().set_length(steps); });
}

int taptools_seqtrig_set_swing(taptools_seqtrig h, double swing) {
    return with<seq_trigger>(h, [&](seq_trigger& r) { r.clock().set_swing(swing); });
}

int taptools_seqtrig_set_quantize(taptools_seqtrig h, int mode) {
    return with<seq_trigger>(h, [&](seq_trigger& r) { r.clock().set_quantize(mode); });
}

int taptools_seqtrig_set_pulse_ms(taptools_seqtrig h, double ms) {
    return with<seq_trigger>(h, [&](seq_trigger& r) { r.set_pulse_ms(ms); });
}

int taptools_seqtrig_set_step(taptools_seqtrig h, int step, double velocity) {
    if (step < 0 || step >= tap::tools::seq::k_max_steps) {
        return -1;
    }
    return with<seq_trigger>(h, [&](seq_trigger& r) { r.clock().data().steps[step].velocity = velocity; });
}

int taptools_seqtrig_store(taptools_seqtrig h, int slot) {
    return with<seq_trigger>(h, [&](seq_trigger& r) { r.clock().store(slot); });
}

int taptools_seqtrig_recall(taptools_seqtrig h, int slot) {
    return with<seq_trigger>(h, [&](seq_trigger& r) { r.clock().recall(slot); });
}

int taptools_seqtrig_reset(taptools_seqtrig h) {
    return with<seq_trigger>(h, [&](seq_trigger& r) { r.reset(); });
}

int taptools_seqtrig_process(taptools_seqtrig h, const double* phase, double* out, int n) {
    if (!phase || !out || n < 0) {
        return -1;
    }
    return with<seq_trigger>(h, [&](seq_trigger& r) {
        for (int i = 0; i < n; ++i) {
            out[i] = r.process(phase[i]);
        }
    });
}

// ---- tap.303.seq~ ------------------------------------------------------------------------------

using seq_note = tap::tools::seq::note_row;

taptools_seqnote taptools_seqnote_create(void) {
    return static_cast<taptools_seqnote>(new seq_note());
}

void taptools_seqnote_destroy(taptools_seqnote h) {
    delete static_cast<seq_note*>(h);
}

int taptools_seqnote_prepare(taptools_seqnote h, double sr) {
    return with<seq_note>(h, [&](seq_note& r) { r.prepare(sr); });
}

int taptools_seqnote_set_length(taptools_seqnote h, int steps) {
    return with<seq_note>(h, [&](seq_note& r) { r.clock().data().set_length(steps); });
}

int taptools_seqnote_set_swing(taptools_seqnote h, double swing) {
    return with<seq_note>(h, [&](seq_note& r) { r.clock().set_swing(swing); });
}

int taptools_seqnote_set_quantize(taptools_seqnote h, int mode) {
    return with<seq_note>(h, [&](seq_note& r) { r.clock().set_quantize(mode); });
}

int taptools_seqnote_set_transpose(taptools_seqnote h, double semitones) {
    return with<seq_note>(h, [&](seq_note& r) { r.set_transpose(semitones); });
}

int taptools_seqnote_set_step(taptools_seqnote h, int step, double pitch, int gate, int accent, int slide) {
    if (step < 0 || step >= tap::tools::seq::k_max_steps) {
        return -1;
    }
    return with<seq_note>(h, [&](seq_note& r) {
        auto& st  = r.clock().data().steps[step];
        st.pitch  = pitch;
        st.gate   = gate != 0;
        st.accent = accent != 0;
        st.slide  = slide != 0;
    });
}

int taptools_seqnote_store(taptools_seqnote h, int slot) {
    return with<seq_note>(h, [&](seq_note& r) { r.clock().store(slot); });
}

int taptools_seqnote_recall(taptools_seqnote h, int slot) {
    return with<seq_note>(h, [&](seq_note& r) { r.clock().recall(slot); });
}

int taptools_seqnote_reset(taptools_seqnote h) {
    return with<seq_note>(h, [&](seq_note& r) { r.reset(); });
}

int taptools_seqnote_process(taptools_seqnote h, const double* phase, double* pitch_out, double* gate_out, int n) {
    if (!phase || !pitch_out || !gate_out || n < 0) {
        return -1;
    }
    return with<seq_note>(h, [&](seq_note& r) {
        for (int i = 0; i < n; ++i) {
            const auto o = r.process(phase[i]);
            pitch_out[i] = o.pitch;
            gate_out[i]  = o.gate;
        }
    });
}

// ---- tap.808.kick~ -----------------------------------------------------------------------------

using tr808_kick = tap::tools::tr808::kick;

taptools_kick taptools_kick_create(void) {
    return static_cast<taptools_kick>(new tr808_kick());
}

void taptools_kick_destroy(taptools_kick h) {
    delete static_cast<tr808_kick*>(h);
}

int taptools_kick_prepare(taptools_kick h, double sr) {
    return with<tr808_kick>(h, [&](tr808_kick& k) { k.prepare(sr); });
}

int taptools_kick_set_decay(taptools_kick h, double v) {
    return with<tr808_kick>(h, [&](tr808_kick& k) { k.set_decay(v); });
}

int taptools_kick_set_tone(taptools_kick h, double v) {
    return with<tr808_kick>(h, [&](tr808_kick& k) { k.set_tone(v); });
}

int taptools_kick_set_level(taptools_kick h, double v) {
    return with<tr808_kick>(h, [&](tr808_kick& k) { k.set_level(v); });
}

int taptools_kick_trigger(taptools_kick h, double accent) {
    return with<tr808_kick>(h, [&](tr808_kick& k) { k.trigger(accent); });
}

int taptools_kick_reset(taptools_kick h) {
    return with<tr808_kick>(h, [&](tr808_kick& k) { k.reset(); });
}

int taptools_kick_process(taptools_kick h, const double* trig, double* out, int n) {
    if (!out || n < 0) {
        return -1;
    }
    return with<tr808_kick>(h, [&](tr808_kick& k) {
        double prev = 0.0;
        for (int i = 0; i < n; ++i) {
            if (trig) {
                const double x = trig[i];
                if (x > 1e-3 && prev <= 1e-3) {
                    k.trigger(x < 0.0 ? 0.0 : (x > 1.0 ? 1.0 : x));
                }
                prev = x;
            }
            out[i] = k.process();
        }
    });
}

// ---- tap.tune~ ---------------------------------------------------------------------------------

using tune_corrector = tap::tools::tune::corrector;

taptools_tune taptools_tune_create(void) {
    return static_cast<taptools_tune>(new tune_corrector());
}

void taptools_tune_destroy(taptools_tune h) {
    delete static_cast<tune_corrector*>(h);
}

int taptools_tune_prepare(taptools_tune h, double sr) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.prepare(sr); });
}

int taptools_tune_clear(taptools_tune h) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.clear(); });
}

int taptools_tune_set_key(taptools_tune h, int pitch_class) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.set_key(pitch_class); });
}

int taptools_tune_set_scale(taptools_tune h, unsigned mask) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.set_scale(mask); });
}

int taptools_tune_set_notes(taptools_tune h, unsigned absolute_mask) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.set_notes(absolute_mask); });
}

int taptools_tune_set_mode(taptools_tune h, int mode) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.set_mode(static_cast<tap::tools::tune::mode>(mode)); });
}

int taptools_tune_set_backend(taptools_tune h, int backend) {
    return with<tune_corrector>(
        h, [&](tune_corrector& c) { c.set_backend(static_cast<tap::tools::tune::backend>(backend)); });
}

int taptools_tune_set_speed(taptools_tune h, double ms) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.set_speed(ms); });
}

int taptools_tune_set_amount(taptools_tune h, double pct) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.set_amount(pct); });
}

int taptools_tune_set_range(taptools_tune h, double min_hz, double max_hz) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.set_range(min_hz, max_hz); });
}

int taptools_tune_set_threshold(taptools_tune h, double t) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.set_threshold(t); });
}

int taptools_tune_set_formant(taptools_tune h, int on) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.set_formant(on != 0); });
}

int taptools_tune_set_autokey(taptools_tune h, int on) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.set_autokey(on != 0); });
}

int taptools_tune_autokey_reset(taptools_tune h) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.autokey_reset(); });
}

int taptools_tune_autokey_estimate(taptools_tune h, int* key, int* minor, double* confidence) {
    if (!h || !key || !minor || !confidence) {
        return -1;
    }
    const auto e = static_cast<tune_corrector*>(h)->autokey_estimate();
    *key         = e.key;
    *minor       = e.minor ? 1 : 0;
    *confidence  = e.confidence;
    return 0;
}

int taptools_tune_autokey_apply(taptools_tune h) {
    if (!h) {
        return -1;
    }
    return static_cast<tune_corrector*>(h)->autokey_apply() ? 1 : 0;
}

int taptools_tune_note_on(taptools_tune h, int note) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.note_on(note); });
}

int taptools_tune_note_off(taptools_tune h, int note) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.note_off(note); });
}

int taptools_tune_notes_off(taptools_tune h) {
    return with<tune_corrector>(h, [&](tune_corrector& c) { c.notes_off(); });
}

double taptools_tune_detected_hz(taptools_tune h) {
    return h ? static_cast<tune_corrector*>(h)->detected_hz() : -1.0;
}

double taptools_tune_target_midi(taptools_tune h) {
    return h ? static_cast<tune_corrector*>(h)->target_midi() : -1.0;
}

double taptools_tune_applied_semitones(taptools_tune h) {
    return h ? static_cast<tune_corrector*>(h)->applied_semitones() : 0.0;
}

int taptools_tune_process(taptools_tune h, const double* in, double* out, int n) {
    if (!in || !out || n < 0) {
        return -1;
    }
    return with<tune_corrector>(h, [&](tune_corrector& c) {
        for (int i = 0; i < n; ++i) {
            out[i] = c.process(in[i]);
        }
    });
}

// ---- pitch detector passthrough ----------------------------------------------------------------

taptools_yin taptools_yin_create(int window, int tau_min, int tau_max) {
    if (tau_min < 2 || tau_min >= tau_max || window < tau_max) {
        return nullptr;
    }
    return static_cast<taptools_yin>(
        new tap::dsp::yin(static_cast<size_t>(window), static_cast<size_t>(tau_min), static_cast<size_t>(tau_max)));
}

void taptools_yin_destroy(taptools_yin h) {
    delete static_cast<tap::dsp::yin*>(h);
}

int taptools_yin_frame_size(taptools_yin h) {
    return h ? static_cast<int>(static_cast<tap::dsp::yin*>(h)->frame_size()) : -1;
}

int taptools_yin_track(taptools_yin h, const double* x, int n, int hop, double* periods, int max_out) {
    if (!h || !x || !periods || hop < 1) {
        return -1;
    }
    auto*     det   = static_cast<tap::dsp::yin*>(h);
    const int frame = static_cast<int>(det->frame_size());
    int       count = 0;
    for (int start = 0; start + frame <= n && count < max_out; start += hop) {
        periods[count++] = det->analyze(x + start).period;
    }
    return count;
}

} // extern "C"
