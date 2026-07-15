/// @file taptools_capi.cpp
/// @brief C ABI implementation — see taptools_capi.h.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "taptools_capi.h"

// The DSP cores are the same headers the Max externals compile — no Max/Min dependency.
#include <taptools/autowah.h>
#include <taptools/conv_engine.h>
#include <taptools/ladder.h>
#include <taptools/svf.h>
#include <taptools/vco.h>

using taptools::conv_engine;

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

using taptools::svf::svf_filter;

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

using taptools::ladder::ladder_filter;

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

using taptools::vco::vco_osc;

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

// ---- tap.autowah~ ------------------------------------------------------------------------------

using taptools::autowah::wah_filter;

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

} // extern "C"
