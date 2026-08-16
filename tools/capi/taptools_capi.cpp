/// @file taptools_capi.cpp
/// @brief C ABI implementation — see taptools_capi.h.
// SPDX-License-Identifier: MIT
// Copyright 2003-2026 Timothy Place.

#include "taptools_capi.h"

#include <algorithm>
#include <array>
#include <vector>

// The DSP cores are the same headers the Max externals compile — no Max/Min dependency.
#include <taptools/adsr.h>
#include <taptools/airport.h>
#include <taptools/autowah.h>
#include <taptools/conv_engine.h>
#include <taptools/delay.h>
#include <taptools/diode_ladder.h>
#include <taptools/discreet.h>
#include <taptools/fuzz.h>
#include <taptools/garden.h>
#include <taptools/harmonizer.h>
#include <taptools/ladder.h>
#include <taptools/overdrive.h>
#include <taptools/stammer.h>
#include <taptools/step_seq.h>
#include <taptools/svf.h>
#include <taptools/tapecho.h>
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

// ---- tap.harmony~ ------------------------------------------------------------------------------

using harmony_kernel = tap::tools::harmony::harmonizer;

taptools_harmonizer taptools_harmonizer_create(void) {
    return static_cast<taptools_harmonizer>(new harmony_kernel());
}

void taptools_harmonizer_destroy(taptools_harmonizer h) {
    delete static_cast<harmony_kernel*>(h);
}

int taptools_harmonizer_prepare(taptools_harmonizer h, double sr, int fft_size) {
    if (fft_size < 64 || (fft_size & (fft_size - 1)) != 0) {
        return -1;
    }
    return with<harmony_kernel>(h, [&](harmony_kernel& k) { k.prepare(sr, static_cast<size_t>(fft_size)); });
}

int taptools_harmonizer_clear(taptools_harmonizer h) {
    return with<harmony_kernel>(h, [&](harmony_kernel& k) { k.clear(); });
}

int taptools_harmonizer_set_interval(taptools_harmonizer h, int voice, double st) {
    return with<harmony_kernel>(h, [&](harmony_kernel& k) { k.set_interval(voice, st); });
}

int taptools_harmonizer_set_gain(taptools_harmonizer h, int voice, double gain) {
    return with<harmony_kernel>(h, [&](harmony_kernel& k) { k.set_gain(voice, gain); });
}

int taptools_harmonizer_set_dry(taptools_harmonizer h, double gain) {
    return with<harmony_kernel>(h, [&](harmony_kernel& k) { k.set_dry(gain); });
}

int taptools_harmonizer_set_formant(taptools_harmonizer h, int on) {
    return with<harmony_kernel>(h, [&](harmony_kernel& k) { k.set_formant(on != 0); });
}

int taptools_harmonizer_set_glide(taptools_harmonizer h, double ms) {
    return with<harmony_kernel>(h, [&](harmony_kernel& k) { k.set_glide(ms); });
}

int taptools_harmonizer_latency(taptools_harmonizer h) {
    auto* k = static_cast<harmony_kernel*>(h);
    return k ? static_cast<int>(k->latency()) : -1;
}

int taptools_harmonizer_process(taptools_harmonizer h, const double* in, double* out, int n) {
    if (!in || !out || n < 0) {
        return -1;
    }
    return with<harmony_kernel>(h, [&](harmony_kernel& k) {
        for (int i = 0; i < n; ++i) {
            out[i] = k.process(in[i]);
        }
    });
}

// ---- tap.adsr~ ---------------------------------------------------------------------------------

using adsr_generator = tap::tools::adsr::generator;

taptools_adsr taptools_adsr_create(void) {
    return static_cast<taptools_adsr>(new adsr_generator());
}

void taptools_adsr_destroy(taptools_adsr h) {
    delete static_cast<adsr_generator*>(h);
}

int taptools_adsr_prepare(taptools_adsr h, double sr) {
    return with<adsr_generator>(h, [&](adsr_generator& g) { g.prepare(sr); });
}

int taptools_adsr_clear(taptools_adsr h) {
    return with<adsr_generator>(h, [&](adsr_generator& g) { g.clear(); });
}

int taptools_adsr_set_attack(taptools_adsr h, double ms) {
    return with<adsr_generator>(h, [&](adsr_generator& g) { g.set_attack_ms(ms); });
}

int taptools_adsr_set_decay(taptools_adsr h, double ms) {
    return with<adsr_generator>(h, [&](adsr_generator& g) { g.set_decay_ms(ms); });
}

int taptools_adsr_set_sustain_db(taptools_adsr h, double db) {
    return with<adsr_generator>(h, [&](adsr_generator& g) { g.set_sustain_db(db); });
}

int taptools_adsr_set_release(taptools_adsr h, double ms) {
    return with<adsr_generator>(h, [&](adsr_generator& g) { g.set_release_ms(ms); });
}

int taptools_adsr_set_mode(taptools_adsr h, int mode) {
    return with<adsr_generator>(h, [&](adsr_generator& g) { g.set_mode(mode); });
}

int taptools_adsr_set_threshold(taptools_adsr h, double t) {
    return with<adsr_generator>(h, [&](adsr_generator& g) { g.set_threshold(t); });
}

int taptools_adsr_set_velocity(taptools_adsr h, double s) {
    return with<adsr_generator>(h, [&](adsr_generator& g) { g.set_velocity(s); });
}

int taptools_adsr_process(taptools_adsr h, const double* gate, double* out, int n) {
    if (!gate || !out || n < 0) {
        return -1;
    }
    return with<adsr_generator>(h, [&](adsr_generator& g) {
        for (int i = 0; i < n; ++i) {
            out[i] = g.process(gate[i]);
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

// ---- tap.delay~ ----------------------------------------------------------------------------------

using delay_line = tap::tools::delay::line;

taptools_delay taptools_delay_create(void) {
    return static_cast<taptools_delay>(new delay_line());
}

void taptools_delay_destroy(taptools_delay h) {
    delete static_cast<delay_line*>(h);
}

int taptools_delay_prepare(taptools_delay h, double sr, double max_ms) {
    if (max_ms <= 0.0) {
        return -1;
    }
    return with<delay_line>(h, [&](delay_line& d) { d.prepare(sr, max_ms); });
}

int taptools_delay_set_time_ms(taptools_delay h, double ms) {
    return with<delay_line>(h, [&](delay_line& d) { d.set_time_ms(ms); });
}

int taptools_delay_set_feedback(taptools_delay h, double fb) {
    return with<delay_line>(h, [&](delay_line& d) { d.set_feedback(fb); });
}

int taptools_delay_set_mix(taptools_delay h, double pct) {
    return with<delay_line>(h, [&](delay_line& d) { d.set_mix(pct); });
}

int taptools_delay_set_interp(taptools_delay h, int mode) {
    return with<delay_line>(h, [&](delay_line& d) { d.set_interp(mode); });
}

int taptools_delay_set_smooth_ms(taptools_delay h, double ms) {
    return with<delay_line>(h, [&](delay_line& d) { d.set_smooth_ms(ms); });
}

int taptools_delay_clear(taptools_delay h) {
    return with<delay_line>(h, [&](delay_line& d) { d.clear(); });
}

int taptools_delay_process(taptools_delay h, const double* in, double* out, int n) {
    if (!in || !out || n < 0) {
        return -1;
    }
    return with<delay_line>(h, [&](delay_line& d) {
        for (int i = 0; i < n; ++i) {
            out[i] = d.process(in[i]);
        }
    });
}

int taptools_delay_process_mod(taptools_delay h, const double* in, const double* time_ms, double* out, int n) {
    if (!in || !time_ms || !out || n < 0) {
        return -1;
    }
    return with<delay_line>(h, [&](delay_line& d) {
        for (int i = 0; i < n; ++i) {
            out[i] = d.process(in[i], time_ms[i]);
        }
    });
}

// ---- tap.multitap~ -------------------------------------------------------------------------------

using multitap_kernel = tap::tools::delay::multitap;

taptools_multitap taptools_multitap_create(void) {
    return static_cast<taptools_multitap>(new multitap_kernel());
}

void taptools_multitap_destroy(taptools_multitap h) {
    delete static_cast<multitap_kernel*>(h);
}

int taptools_multitap_prepare(taptools_multitap h, double sr, double max_ms) {
    if (max_ms <= 0.0) {
        return -1;
    }
    return with<multitap_kernel>(h, [&](multitap_kernel& m) { m.prepare(sr, max_ms); });
}

int taptools_multitap_set_taps(taptools_multitap h, int count) {
    return with<multitap_kernel>(h, [&](multitap_kernel& m) { m.set_taps(count); });
}

int taptools_multitap_set_time_ms(taptools_multitap h, int tap, double ms) {
    return with<multitap_kernel>(h, [&](multitap_kernel& m) { m.set_time_ms(tap, ms); });
}

int taptools_multitap_set_gain(taptools_multitap h, int tap, double gain) {
    return with<multitap_kernel>(h, [&](multitap_kernel& m) { m.set_gain(tap, gain); });
}

int taptools_multitap_set_pan(taptools_multitap h, int tap, double pan) {
    return with<multitap_kernel>(h, [&](multitap_kernel& m) { m.set_pan(tap, pan); });
}

int taptools_multitap_set_interp(taptools_multitap h, int mode) {
    return with<multitap_kernel>(h, [&](multitap_kernel& m) { m.set_interp(mode); });
}

int taptools_multitap_set_smooth_ms(taptools_multitap h, double ms) {
    return with<multitap_kernel>(h, [&](multitap_kernel& m) { m.set_smooth_ms(ms); });
}

int taptools_multitap_clear(taptools_multitap h) {
    return with<multitap_kernel>(h, [&](multitap_kernel& m) { m.clear(); });
}

int taptools_multitap_process(taptools_multitap h, const double* in, double* outL, double* outR, int n) {
    if (!in || !outL || !outR || n < 0) {
        return -1;
    }
    return with<multitap_kernel>(h, [&](multitap_kernel& m) {
        for (int i = 0; i < n; ++i) {
            m.process(in[i], outL[i], outR[i]);
        }
    });
}

// ---- tap.overdrive~ ------------------------------------------------------------------------------

using tap::tools::od::overdrive;

taptools_od taptools_od_create(void) {
    return static_cast<taptools_od>(new overdrive());
}

void taptools_od_destroy(taptools_od h) {
    delete static_cast<overdrive*>(h);
}

int taptools_od_prepare(taptools_od h, double sr) {
    return with<overdrive>(h, [&](overdrive& o) { o.prepare(sr, 1); });
}

int taptools_od_set(taptools_od h, int param, double value) {
    return with<overdrive>(h, [&](overdrive& o) { o.set_param(param, value); });
}

int taptools_od_set_oversample(taptools_od h, int os) {
    return with<overdrive>(h, [&](overdrive& o) { o.set_oversample(os); });
}

int taptools_od_set_smooth_ms(taptools_od h, double ms) {
    return with<overdrive>(h, [&](overdrive& o) { o.set_smooth_ms(ms); });
}

int taptools_od_clear(taptools_od h) {
    return with<overdrive>(h, [&](overdrive& o) { o.clear(); });
}

int taptools_od_process(taptools_od h, const double* in, double* out, int n) {
    if (!in || !out || n < 0) {
        return -1;
    }
    return with<overdrive>(h, [&](overdrive& o) { o.process(in, out, static_cast<size_t>(n)); });
}

// ---- tap.discreet~ -------------------------------------------------------------------------------

using discreet_machine = tap::tools::discreet::machine;

taptools_discreet taptools_discreet_create(void) {
    return static_cast<taptools_discreet>(new discreet_machine());
}

void taptools_discreet_destroy(taptools_discreet h) {
    delete static_cast<discreet_machine*>(h);
}

int taptools_discreet_prepare(taptools_discreet h, double sr, double max_loop_seconds) {
    if (max_loop_seconds <= 0.0) {
        return -1;
    }
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.prepare(sr, max_loop_seconds); });
}

int taptools_discreet_set_loop_seconds(taptools_discreet h, double s) {
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.set_loop_seconds(s); });
}

int taptools_discreet_set_regen(taptools_discreet h, double r) {
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.set_regen(r); });
}

int taptools_discreet_set_darken_hz(taptools_discreet h, double hz) {
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.set_darken_hz(hz); });
}

int taptools_discreet_set_drive(taptools_discreet h, double d) {
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.set_drive(d); });
}

int taptools_discreet_set_input_level(taptools_discreet h, double lin) {
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.set_input_level(lin); });
}

int taptools_discreet_set_mix(taptools_discreet h, double pct) {
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.set_mix(pct); });
}

int taptools_discreet_set_wow(taptools_discreet h, double depth_ms, double rate_hz) {
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.set_wow(depth_ms, rate_hz); });
}

int taptools_discreet_set_flutter(taptools_discreet h, double depth_ms, double rate_hz) {
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.set_flutter(depth_ms, rate_hz); });
}

int taptools_discreet_set_smooth_ms(taptools_discreet h, double ms) {
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.set_smooth_ms(ms); });
}

int taptools_discreet_clear(taptools_discreet h) {
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.clear(); });
}

int taptools_discreet_process(taptools_discreet h, const double* in, double* out, int n) {
    if (!in || !out || n < 0) {
        return -1;
    }
    return with<discreet_machine>(h, [&](discreet_machine& m) { m.process(in, out, static_cast<size_t>(n)); });
}

// ---- tap.tapecho~ --------------------------------------------------------------------------------

using tapecho_machine = tap::tools::tapecho::machine;

taptools_tapecho taptools_tapecho_create(void) {
    return static_cast<taptools_tapecho>(new tapecho_machine());
}

void taptools_tapecho_destroy(taptools_tapecho h) {
    delete static_cast<tapecho_machine*>(h);
}

int taptools_tapecho_prepare(taptools_tapecho h, double sr, double max_span_seconds) {
    if (max_span_seconds <= 0.0) {
        return -1;
    }
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.prepare(sr, max_span_seconds); });
}

int taptools_tapecho_set_span_ms(taptools_tapecho h, double ms) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_span_ms(ms); });
}

int taptools_tapecho_set_heads(taptools_tapecho h, int count) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_heads(count); });
}

int taptools_tapecho_set_head_ratio(taptools_tapecho h, int head, double ratio) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_head_ratio(head, ratio); });
}

int taptools_tapecho_set_head_level(taptools_tapecho h, int head, double lin) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_head_level(head, lin); });
}

int taptools_tapecho_set_head_pan(taptools_tapecho h, int head, double pan) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_head_pan(head, pan); });
}

int taptools_tapecho_set_regen(taptools_tapecho h, double r) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_regen(r); });
}

int taptools_tapecho_set_darken_hz(taptools_tapecho h, double hz) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_darken_hz(hz); });
}

int taptools_tapecho_set_drive(taptools_tapecho h, double d) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_drive(d); });
}

int taptools_tapecho_set_input_level(taptools_tapecho h, double lin) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_input_level(lin); });
}

int taptools_tapecho_set_mix(taptools_tapecho h, double pct) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_mix(pct); });
}

int taptools_tapecho_set_wow(taptools_tapecho h, double depth_ms, double rate_hz) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_wow(depth_ms, rate_hz); });
}

int taptools_tapecho_set_flutter(taptools_tapecho h, double depth_ms, double rate_hz) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_flutter(depth_ms, rate_hz); });
}

int taptools_tapecho_set_smooth_ms(taptools_tapecho h, double ms) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.set_smooth_ms(ms); });
}

int taptools_tapecho_clear(taptools_tapecho h) {
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.clear(); });
}

int taptools_tapecho_process(taptools_tapecho h, const double* in, double* outL, double* outR, int n) {
    if (!in || !outL || !outR || n < 0) {
        return -1;
    }
    return with<tapecho_machine>(h, [&](tapecho_machine& m) { m.process(in, outL, outR, static_cast<size_t>(n)); });
}

// ---- tap.fuzz~ -----------------------------------------------------------------------------------

using fuzz_pedal = tap::tools::fuzz::pedal;

taptools_fuzz taptools_fuzz_create(void) {
    return static_cast<taptools_fuzz>(new fuzz_pedal());
}

void taptools_fuzz_destroy(taptools_fuzz h) {
    delete static_cast<fuzz_pedal*>(h);
}

int taptools_fuzz_prepare(taptools_fuzz h, double sr) {
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.prepare(sr); });
}

int taptools_fuzz_set_gain(taptools_fuzz h, double g) {
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.set_gain(g); });
}

int taptools_fuzz_set_edge(taptools_fuzz h, double e) {
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.set_edge(e); });
}

int taptools_fuzz_set_asymmetry(taptools_fuzz h, double a) {
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.set_asymmetry(a); });
}

int taptools_fuzz_set_bass(taptools_fuzz h, double b) {
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.set_bass(b); });
}

int taptools_fuzz_set_treble(taptools_fuzz h, double t) {
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.set_treble(t); });
}

int taptools_fuzz_set_contrast(taptools_fuzz h, double c) {
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.set_contrast(c); });
}

int taptools_fuzz_set_level_db(taptools_fuzz h, double db) {
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.set_level_db(db); });
}

int taptools_fuzz_set_oversample(taptools_fuzz h, int os) {
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.set_oversample(os); });
}

int taptools_fuzz_set_smooth_ms(taptools_fuzz h, double ms) {
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.set_smooth_ms(ms); });
}

int taptools_fuzz_clear(taptools_fuzz h) {
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.clear(); });
}

int taptools_fuzz_process(taptools_fuzz h, const double* in, double* out, int n) {
    if (!in || !out || n < 0) {
        return -1;
    }
    return with<fuzz_pedal>(h, [&](fuzz_pedal& p) { p.process(in, out, static_cast<size_t>(n)); });
}

// ---- tap.stammer~ --------------------------------------------------------------------------------

using stammer_machine = tap::tools::stammer::machine;

taptools_stammer taptools_stammer_create(void) {
    return static_cast<taptools_stammer>(new stammer_machine());
}

void taptools_stammer_destroy(taptools_stammer h) {
    delete static_cast<stammer_machine*>(h);
}

int taptools_stammer_prepare(taptools_stammer h, double sr, double max_history_ms) {
    if (max_history_ms <= 0.0) {
        return -1;
    }
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.prepare(sr, max_history_ms); });
}

int taptools_stammer_set_step_ms(taptools_stammer h, double ms) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.set_step_ms(ms); });
}

int taptools_stammer_set_density(taptools_stammer h, double p) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.set_density(p); });
}

int taptools_stammer_set_divisions(taptools_stammer h, int n) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.set_divisions(n); });
}

int taptools_stammer_set_repeats(taptools_stammer h, int n) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.set_repeats(n); });
}

int taptools_stammer_set_reverse(taptools_stammer h, double p) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.set_reverse(p); });
}

int taptools_stammer_set_jump_ms(taptools_stammer h, double ms) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.set_jump_ms(ms); });
}

int taptools_stammer_set_fade_ms(taptools_stammer h, double ms) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.set_fade_ms(ms); });
}

int taptools_stammer_set_seed(taptools_stammer h, unsigned long long seed) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.set_seed(static_cast<uint64_t>(seed)); });
}

int taptools_stammer_set_input_level(taptools_stammer h, double lin) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.set_input_level(lin); });
}

int taptools_stammer_set_mix(taptools_stammer h, double pct) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.set_mix(pct); });
}

int taptools_stammer_set_smooth_ms(taptools_stammer h, double ms) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.set_smooth_ms(ms); });
}

int taptools_stammer_clear(taptools_stammer h) {
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.clear(); });
}

int taptools_stammer_playing(taptools_stammer h) {
    stammer_machine* m = static_cast<stammer_machine*>(h);
    return m ? (m->playing() ? 1 : 0) : -1;
}

int taptools_stammer_process(taptools_stammer h, const double* in, double* out, int n) {
    if (!in || !out || n < 0) {
        return -1;
    }
    return with<stammer_machine>(h, [&](stammer_machine& m) { m.process(in, out, static_cast<size_t>(n)); });
}

// ---- tap.airport~ --------------------------------------------------------------------------------

using airport_bank = tap::tools::airport::loop_bank;

taptools_airport taptools_airport_create(void) {
    return static_cast<taptools_airport>(new airport_bank());
}

void taptools_airport_destroy(taptools_airport h) {
    delete static_cast<airport_bank*>(h);
}

int taptools_airport_prepare(taptools_airport h, double sr, double max_loop_seconds) {
    if (max_loop_seconds <= 0.0) {
        return -1;
    }
    return with<airport_bank>(h, [&](airport_bank& b) { b.prepare(sr, max_loop_seconds); });
}

int taptools_airport_set_loops(taptools_airport h, int count) {
    return with<airport_bank>(h, [&](airport_bank& b) { b.set_loops(count); });
}

int taptools_airport_set_length_seconds(taptools_airport h, int loop, double s) {
    return with<airport_bank>(h, [&](airport_bank& b) { b.set_length_seconds(loop, s); });
}

int taptools_airport_record(taptools_airport h, int loop, int on) {
    return with<airport_bank>(h, [&](airport_bank& b) { b.record(loop, on != 0); });
}

int taptools_airport_set_level(taptools_airport h, int loop, double lin) {
    return with<airport_bank>(h, [&](airport_bank& b) { b.set_level(loop, lin); });
}

int taptools_airport_set_pan(taptools_airport h, int loop, double pan) {
    return with<airport_bank>(h, [&](airport_bank& b) { b.set_pan(loop, pan); });
}

int taptools_airport_set_darken_hz(taptools_airport h, int loop, double hz) {
    return with<airport_bank>(h, [&](airport_bank& b) { b.set_darken_hz(loop, hz); });
}

int taptools_airport_set_smooth_ms(taptools_airport h, double ms) {
    return with<airport_bank>(h, [&](airport_bank& b) { b.set_smooth_ms(ms); });
}

int taptools_airport_clear(taptools_airport h) {
    return with<airport_bank>(h, [&](airport_bank& b) { b.clear(); });
}

double taptools_airport_phase(taptools_airport h, int loop) {
    if (!h) {
        return -1.0;
    }
    return static_cast<airport_bank*>(h)->phase(loop);
}

double taptools_airport_composite_period_seconds(taptools_airport h) {
    if (!h) {
        return -1.0;
    }
    return static_cast<airport_bank*>(h)->composite_period_seconds();
}

int taptools_airport_process(taptools_airport h, const double* in, double* outL, double* outR, int n) {
    if (!in || !outL || !outR || n < 0) {
        return -1;
    }
    return with<airport_bank>(h, [&](airport_bank& b) { b.process(in, outL, outR, static_cast<size_t>(n)); });
}

// ---- tap.garden~ ---------------------------------------------------------------------------------

using garden_bed = tap::tools::garden::bed;

taptools_garden taptools_garden_create(void) {
    return static_cast<taptools_garden>(new garden_bed());
}

void taptools_garden_destroy(taptools_garden h) {
    delete static_cast<garden_bed*>(h);
}

int taptools_garden_prepare(taptools_garden h, double sr) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.prepare(sr); });
}

int taptools_garden_note(taptools_garden h, double pitch, double velocity) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.note(pitch, velocity); });
}

int taptools_garden_set_loop_seconds(taptools_garden h, double s) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_loop_seconds(s); });
}

int taptools_garden_set_decay(taptools_garden h, double per_pass) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_decay(per_pass); });
}

int taptools_garden_set_soften(taptools_garden h, double per_pass) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_soften(per_pass); });
}

int taptools_garden_set_floor(taptools_garden h, double v) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_floor(v); });
}

int taptools_garden_set_bell(taptools_garden h, double attack_s, double decay_s, double brightness) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_bell(attack_s, decay_s, brightness); });
}

int taptools_garden_set_material(taptools_garden h, int material) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_material(material); });
}

int taptools_garden_set_spread(taptools_garden h, double amount) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_spread(amount); });
}

int taptools_garden_set_root(taptools_garden h, int semitone) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_root(semitone); });
}

int taptools_garden_set_scale(taptools_garden h, int scale) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_scale(scale); });
}

int taptools_garden_set_idle_seconds(taptools_garden h, double s) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_idle_seconds(s); });
}

int taptools_garden_set_gust(taptools_garden h, double amount) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_gust(amount); });
}

int taptools_garden_set_seed(taptools_garden h, unsigned long long seed) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_seed(static_cast<uint64_t>(seed)); });
}

int taptools_garden_set_level(taptools_garden h, double lin) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_level(lin); });
}

int taptools_garden_set_smooth_ms(taptools_garden h, double ms) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.set_smooth_ms(ms); });
}

int taptools_garden_clear(taptools_garden h) {
    return with<garden_bed>(h, [&](garden_bed& g) { g.clear(); });
}

int taptools_garden_active_events(taptools_garden h) {
    if (!h) {
        return -1;
    }
    return static_cast<garden_bed*>(h)->active_events();
}

int taptools_garden_active_voices(taptools_garden h) {
    if (!h) {
        return -1;
    }
    return static_cast<garden_bed*>(h)->active_voices();
}

int taptools_garden_process(taptools_garden h, double* outL, double* outR, int n) {
    if (!outL || !outR || n < 0) {
        return -1;
    }
    return with<garden_bed>(h, [&](garden_bed& g) { g.process(outL, outR, static_cast<size_t>(n)); });
}

// ---- tap.reel~ -----------------------------------------------------------------------------------

using airport_lane = tap::tools::airport::loop;

taptools_reel taptools_reel_create(void) {
    return static_cast<taptools_reel>(new airport_lane());
}

void taptools_reel_destroy(taptools_reel h) {
    delete static_cast<airport_lane*>(h);
}

int taptools_reel_prepare(taptools_reel h, double sr, double max_loop_seconds) {
    if (max_loop_seconds <= 0.0) {
        return -1;
    }
    return with<airport_lane>(h, [&](airport_lane& l) { l.prepare(sr, max_loop_seconds); });
}

int taptools_reel_set_length_seconds(taptools_reel h, double s) {
    return with<airport_lane>(h, [&](airport_lane& l) { l.set_length_seconds(s); });
}

int taptools_reel_record(taptools_reel h, int on) {
    return with<airport_lane>(h, [&](airport_lane& l) { l.record(on != 0); });
}

int taptools_reel_set_level(taptools_reel h, double lin) {
    return with<airport_lane>(h, [&](airport_lane& l) { l.set_level(lin); });
}

int taptools_reel_set_pan(taptools_reel h, double pan) {
    return with<airport_lane>(h, [&](airport_lane& l) { l.set_pan(pan); });
}

int taptools_reel_set_darken_hz(taptools_reel h, double hz) {
    return with<airport_lane>(h, [&](airport_lane& l) { l.set_darken_hz(hz); });
}

int taptools_reel_set_smooth_ms(taptools_reel h, double ms) {
    return with<airport_lane>(h, [&](airport_lane& l) { l.set_smooth_ms(ms); });
}

int taptools_reel_clear(taptools_reel h) {
    return with<airport_lane>(h, [&](airport_lane& l) { l.clear(); });
}

double taptools_reel_phase(taptools_reel h) {
    if (!h) {
        return -1.0;
    }
    return static_cast<airport_lane*>(h)->phase();
}

double taptools_reel_length_seconds(taptools_reel h) {
    if (!h) {
        return -1.0;
    }
    return static_cast<airport_lane*>(h)->length_seconds();
}

int taptools_reel_loop_samples(taptools_reel h) {
    if (!h) {
        return -1;
    }
    return static_cast<int>(static_cast<airport_lane*>(h)->loop_samples());
}

int taptools_reel_process(taptools_reel h, const double* in, double* outL, double* outR, int n) {
    if (!in || !outL || !outR || n < 0) {
        return -1;
    }
    return with<airport_lane>(h, [&](airport_lane& l) { l.process(in, outL, outR, static_cast<size_t>(n)); });
}

// ---- tap.chime~ ----------------------------------------------------------------------------------

using garden_rack = tap::tools::garden::rack;

taptools_chime taptools_chime_create(void) {
    return static_cast<taptools_chime>(new garden_rack());
}

void taptools_chime_destroy(taptools_chime h) {
    delete static_cast<garden_rack*>(h);
}

int taptools_chime_prepare(taptools_chime h, double sr) {
    return with<garden_rack>(h, [&](garden_rack& r) { r.prepare(sr); });
}

int taptools_chime_set_times(taptools_chime h, double attack_s, double decay_s) {
    return with<garden_rack>(h, [&](garden_rack& r) { r.set_times(attack_s, decay_s); });
}

int taptools_chime_set_material(taptools_chime h, int material) {
    return with<garden_rack>(h, [&](garden_rack& r) { r.set_material(material); });
}

int taptools_chime_set_spread(taptools_chime h, double amount) {
    return with<garden_rack>(h, [&](garden_rack& r) { r.set_spread(amount); });
}

int taptools_chime_clear(taptools_chime h) {
    return with<garden_rack>(h, [&](garden_rack& r) { r.clear(); });
}

int taptools_chime_strike(taptools_chime h, double pitch, double velocity, double brightness) {
    return with<garden_rack>(h, [&](garden_rack& r) { r.strike(pitch, velocity, brightness); });
}

int taptools_chime_strike_hz(taptools_chime h, double freq_hz, double velocity, double brightness) {
    if (freq_hz <= 0.0) {
        return -1;
    }
    return with<garden_rack>(h, [&](garden_rack& r) { r.strike_hz(freq_hz, velocity, brightness); });
}

int taptools_chime_active_voices(taptools_chime h) {
    if (!h) {
        return -1;
    }
    return static_cast<garden_rack*>(h)->active_voices();
}

int taptools_chime_process(taptools_chime h, double* outL, double* outR, int n) {
    if (!outL || !outR || n < 0) {
        return -1;
    }
    return with<garden_rack>(h, [&](garden_rack& r) {
        for (int i = 0; i < n; ++i) { // rack::process accumulates, so a standalone caller zeroes
            outL[i] = 0.0;
            outR[i] = 0.0;
            r.process(outL[i], outR[i]);
        }
    });
}

// ---- tap.bloom -----------------------------------------------------------------------------------

using garden_ring = tap::tools::garden::ring;

taptools_bloom taptools_bloom_create(void) {
    return static_cast<taptools_bloom>(new garden_ring());
}

void taptools_bloom_destroy(taptools_bloom h) {
    delete static_cast<garden_ring*>(h);
}

int taptools_bloom_prepare(taptools_bloom h, double sr) {
    return with<garden_ring>(h, [&](garden_ring& r) { r.prepare(sr); });
}

int taptools_bloom_set_loop_seconds(taptools_bloom h, double s) {
    return with<garden_ring>(h, [&](garden_ring& r) { r.set_loop_seconds(s); });
}

int taptools_bloom_set_decay(taptools_bloom h, double per_pass) {
    return with<garden_ring>(h, [&](garden_ring& r) { r.set_decay(per_pass); });
}

int taptools_bloom_set_soften(taptools_bloom h, double per_pass) {
    return with<garden_ring>(h, [&](garden_ring& r) { r.set_soften(per_pass); });
}

int taptools_bloom_set_floor(taptools_bloom h, double v) {
    return with<garden_ring>(h, [&](garden_ring& r) { r.set_floor(v); });
}

int taptools_bloom_set_brightness(taptools_bloom h, double b) {
    return with<garden_ring>(h, [&](garden_ring& r) { r.set_brightness(b); });
}

int taptools_bloom_clear(taptools_bloom h) {
    return with<garden_ring>(h, [&](garden_ring& r) { r.clear(); });
}

int taptools_bloom_plant(taptools_bloom h, double pitch, double velocity) {
    if (velocity <= 0.0) {
        return -1;
    }
    return with<garden_ring>(h, [&](garden_ring& r) { r.plant(pitch, velocity); });
}

int taptools_bloom_due(taptools_bloom h, double* pitch, double* velocity, double* brightness, int max) {
    if (!h || !pitch || !velocity || !brightness || max < 0) {
        return -1;
    }
    std::array<tap::tools::garden::strike, tap::tools::garden::k_max_events> fired{};
    const int limit = std::min(max, tap::tools::garden::k_max_events);
    const int n     = static_cast<garden_ring*>(h)->due(fired.data(), limit);
    for (int i = 0; i < n; ++i) {
        pitch[i]      = fired[static_cast<size_t>(i)].pitch;
        velocity[i]   = fired[static_cast<size_t>(i)].velocity;
        brightness[i] = fired[static_cast<size_t>(i)].brightness;
    }
    return n;
}

int taptools_bloom_step(taptools_bloom h) {
    return with<garden_ring>(h, [&](garden_ring& r) { r.step(); });
}

int taptools_bloom_active_events(taptools_bloom h) {
    if (!h) {
        return -1;
    }
    return static_cast<garden_ring*>(h)->active_events();
}

int taptools_bloom_loop_samples(taptools_bloom h) {
    if (!h) {
        return -1;
    }
    return static_cast<int>(static_cast<garden_ring*>(h)->loop_samples());
}

// ---- tap.gardener --------------------------------------------------------------------------------

using garden_wind = tap::tools::garden::gardener;

taptools_gardener taptools_gardener_create(void) {
    return static_cast<taptools_gardener>(new garden_wind());
}

void taptools_gardener_destroy(taptools_gardener h) {
    delete static_cast<garden_wind*>(h);
}

int taptools_gardener_prepare(taptools_gardener h, double sr) {
    return with<garden_wind>(h, [&](garden_wind& g) { g.prepare(sr); });
}

int taptools_gardener_set_idle_seconds(taptools_gardener h, double s) {
    return with<garden_wind>(h, [&](garden_wind& g) { g.set_idle_seconds(s); });
}

int taptools_gardener_set_gust(taptools_gardener h, double amount) {
    return with<garden_wind>(h, [&](garden_wind& g) { g.set_gust(amount); });
}

int taptools_gardener_set_seed(taptools_gardener h, unsigned long long seed) {
    return with<garden_wind>(h, [&](garden_wind& g) { g.set_seed(static_cast<uint64_t>(seed)); });
}

int taptools_gardener_notice_plant(taptools_gardener h) {
    return with<garden_wind>(h, [&](garden_wind& g) { g.notice_plant(); });
}

int taptools_gardener_clear(taptools_gardener h) {
    return with<garden_wind>(h, [&](garden_wind& g) { g.clear(); });
}

int taptools_gardener_tick(taptools_gardener h, int loop_samples, double* pitch, double* velocity) {
    if (!h || !pitch || !velocity || loop_samples < 1) {
        return -1;
    }
    const garden_wind::request req = static_cast<garden_wind*>(h)->tick(static_cast<long>(loop_samples));
    *pitch                         = req.pitch;
    *velocity                      = req.velocity;
    return req.wanted ? 1 : 0;
}

// ---- tap.scale -----------------------------------------------------------------------------------

double taptools_scale_quantize(double pitch, int root, int scale) {
    tap::tools::garden::scale_quantizer q;
    q.set_root(root);
    q.set_scale(scale);
    return q.quantize(pitch);
}

// ---- per-voice taps ------------------------------------------------------------------------------

int taptools_chime_process_voices(taptools_chime h, double* out, int voices, int n) {
    if (!out || voices < 1 || n < 0) {
        return -1;
    }
    return with<garden_rack>(h, [&](garden_rack& r) {
        std::vector<double> frame(static_cast<size_t>(voices), 0.0);
        for (int i = 0; i < n; ++i) {
            r.process_voices(frame.data(), voices);
            for (int v = 0; v < voices; ++v) { // voice-major, so each voice is a contiguous block
                out[static_cast<size_t>(v) * static_cast<size_t>(n) + static_cast<size_t>(i)] =
                    frame[static_cast<size_t>(v)];
            }
        }
    });
}

double taptools_chime_voice_hz(taptools_chime h, int voice) {
    if (!h) {
        return -1.0;
    }
    return static_cast<garden_rack*>(h)->voice_hz(voice);
}

double taptools_chime_voice_level(taptools_chime h, int voice) {
    if (!h) {
        return -1.0;
    }
    return static_cast<garden_rack*>(h)->voice_level(voice);
}

double taptools_chime_voice_gain_left(taptools_chime h, int voice) {
    if (!h) {
        return -1.0;
    }
    return static_cast<garden_rack*>(h)->voice_gain_left(voice);
}

double taptools_chime_voice_gain_right(taptools_chime h, int voice) {
    if (!h) {
        return -1.0;
    }
    return static_cast<garden_rack*>(h)->voice_gain_right(voice);
}

// ---- tap.period ----------------------------------------------------------------------------------

double taptools_composite_period_seconds(const double* loop_seconds, int count, double sr) {
    if (!loop_seconds || count < 1 || sr <= 0.0) {
        return 0.0;
    }
    std::vector<long> samples(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        samples[static_cast<size_t>(i)] = tap::tools::airport::loop_samples_for(loop_seconds[i], sr);
    }
    return tap::tools::airport::composite_period_seconds(samples.data(), count, sr);
}

} // extern "C"
