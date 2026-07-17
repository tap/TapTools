/// @file
/// @brief      Offline renderer for the tap.808.* drum-voice kernels — demo and reference WAVs.
/// @details    Exercises every tr808_*.h voice with no Max involved. Uses:
///
///             1. Demo mode (no arguments): renders each voice's characteristic moves — the
///                kick's decay/tone/sigh, the snare's tone/snappy travel, the clap's teeth and
///                wash, hats with the choke, cymbal decay/tone, cowbell, the tom/conga matrix,
///                rimshot/claves — plus the full eight-voice kit pattern, as 48 kHz mono
///                float32 WAVs.
///             2. Single-hit mode (`--hit VOICE [accent]`): one full-accent (or specified)
///                hit of `kick|snare|clap|maracas|hat|openhat|cymbal|cowbell|tom|conga|
///                rimshot|claves` — the stimulus for the golden-render comparisons against
///                real-808 sample packs (TapTools-Max plans/tap.808.md §7.2).
///
///             Usage: tr808_render [output-directory]
///                    tr808_render --hit kick [accent] [output-directory]
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include <taptools/tr808_clap.h>
#include <taptools/tr808_cowbell.h>
#include <taptools/tr808_cymbal.h>
#include <taptools/tr808_hat.h>
#include <taptools/tr808_kick.h>
#include <taptools/tr808_rim.h>
#include <taptools/tr808_snare.h>
#include <taptools/tr808_tom.h>

using namespace taptools::tr808;

namespace {

    constexpr double k_g_sr = 48000.0;

    bool write_wav(const std::string& path, const std::vector<double>& samples, double sr) {
        std::FILE* f = std::fopen(path.c_str(), "wb");
        if (!f) {
            std::fprintf(stderr, "cannot open %s\n", path.c_str());
            return false;
        }
        const uint32_t n          = static_cast<uint32_t>(samples.size());
        const uint32_t data_bytes = n * 4;
        const uint32_t rate       = static_cast<uint32_t>(sr);

        auto u16 = [&](uint16_t v) { std::fwrite(&v, 2, 1, f); };
        auto u32 = [&](uint32_t v) { std::fwrite(&v, 4, 1, f); };

        std::fwrite("RIFF", 1, 4, f);
        u32(36 + data_bytes);
        std::fwrite("WAVE", 1, 4, f);
        std::fwrite("fmt ", 1, 4, f);
        u32(16);
        u16(3); // IEEE float
        u16(1);
        u32(rate);
        u32(rate * 4);
        u16(4);
        u16(32);
        std::fwrite("data", 1, 4, f);
        u32(data_bytes);
        for (double s : samples) {
            const float v = static_cast<float>(s);
            std::fwrite(&v, 4, 1, f);
        }
        std::fclose(f);
        std::printf("wrote %s (%.2f s)\n", path.c_str(), n / sr);
        return true;
    }

    /// Render `seconds` of a per-sample callback, firing `fire` at t = 0.05 s.
    std::vector<double> take(double seconds, const std::function<void()>& fire, const std::function<double()>& tick) {
        std::vector<double> y(static_cast<size_t>(seconds * k_g_sr), 0.0);
        const size_t        at = static_cast<size_t>(0.05 * k_g_sr);
        for (size_t i = 0; i < y.size(); ++i) {
            if (i == at)
                fire();
            y[i] = tick();
        }
        return y;
    }

    void append(std::vector<double>& dst, const std::vector<double>& src) {
        dst.insert(dst.end(), src.begin(), src.end());
    }

    /// One hit of a named voice at a given accent; empty on unknown name.
    std::vector<double> single_hit(const std::string& name, double accent) {
        if (name == "kick") {
            kick v;
            v.prepare(k_g_sr);
            return take(1.5, [&] { v.trigger(accent); }, [&] { return v.process(); });
        }
        if (name == "snare") {
            snare v;
            v.prepare(k_g_sr);
            return take(0.6, [&] { v.trigger(accent); }, [&] { return v.process(); });
        }
        if (name == "clap" || name == "maracas") {
            clap v;
            v.prepare(k_g_sr);
            v.set_model(name == "maracas" ? clap::model_maracas : clap::model_clap);
            return take(0.6, [&] { v.trigger(accent); }, [&] { return v.process(); });
        }
        if (name == "hat" || name == "openhat") {
            hat v;
            v.prepare(k_g_sr);
            return take(
                1.0,
                [&] {
                    if (name == "openhat")
                        v.trigger_open(accent);
                    else
                        v.trigger_closed(accent);
                },
                [&] { return v.process(); });
        }
        if (name == "cymbal") {
            cymbal v;
            v.prepare(k_g_sr);
            return take(2.5, [&] { v.trigger(accent); }, [&] { return v.process(); });
        }
        if (name == "cowbell") {
            cowbell v;
            v.prepare(k_g_sr);
            return take(0.5, [&] { v.trigger(accent); }, [&] { return v.process(); });
        }
        if (name == "tom" || name == "conga") {
            tom v;
            v.prepare(k_g_sr);
            v.set_model(name == "conga" ? tom::model_conga : tom::model_tom);
            v.set_size(1);
            return take(0.8, [&] { v.trigger(accent); }, [&] { return v.process(); });
        }
        if (name == "rimshot" || name == "claves") {
            rim v;
            v.prepare(k_g_sr);
            v.set_model(name == "claves" ? rim::model_claves : rim::model_rimshot);
            return take(0.4, [&] { v.trigger(accent); }, [&] { return v.process(); });
        }
        return {};
    }

    void render_demos(const std::string& dir) {
        { // kick: decay travel then the attack/sigh bends
            kick v;
            v.prepare(k_g_sr);
            std::vector<double> y;
            for (double d : {0.2, 0.5, 0.9}) {
                v.set_decay(d);
                append(y, take(1.2, [&] { v.trigger(1.0); }, [&] { return v.process(); }));
            }
            v.set_decay(0.6);
            v.set_sigh(0.0);
            append(y, take(1.2, [&] { v.trigger(1.0); }, [&] { return v.process(); }));
            v.set_sigh(1.0);
            v.set_attack(0.0);
            append(y, take(1.2, [&] { v.trigger(1.0); }, [&] { return v.process(); }));
            write_wav(dir + "/tr808_kick.wav", y, k_g_sr);
        }
        { // snare: tone travel then snappy travel (level ridden down: snappy-max runs hot)
            snare v;
            v.prepare(k_g_sr);
            v.set_level(0.65);
            std::vector<double> y;
            for (double t : {0.0, 0.5, 1.0}) {
                v.set_tone(t);
                append(y, take(0.5, [&] { v.trigger(1.0); }, [&] { return v.process(); }));
            }
            v.set_tone(0.5);
            for (double s : {0.0, 1.0}) {
                v.set_snappy(s);
                append(y, take(0.5, [&] { v.trigger(1.0); }, [&] { return v.process(); }));
            }
            write_wav(dir + "/tr808_snare.wav", y, k_g_sr);
        }
        { // clap: stock, dry teeth, maracas
            clap v;
            v.prepare(k_g_sr);
            std::vector<double> y;
            append(y, take(0.6, [&] { v.trigger(1.0); }, [&] { return v.process(); }));
            v.set_tail(0.0);
            append(y, take(0.5, [&] { v.trigger(1.0); }, [&] { return v.process(); }));
            v.set_tail(1.0);
            v.set_model(clap::model_maracas);
            append(y, take(0.4, [&] { v.trigger(1.0); }, [&] { return v.process(); }));
            write_wav(dir + "/tr808_clap.wav", y, k_g_sr);
        }
        { // hats: closed ticks, open ring, the choke
            hat v;
            v.prepare(k_g_sr);
            v.set_decay(0.8);
            std::vector<double> y(static_cast<size_t>(2.4 * k_g_sr), 0.0);
            for (size_t i = 0; i < y.size(); ++i) {
                const double t = i / k_g_sr;
                if (i == static_cast<size_t>(0.05 * k_g_sr) || i == static_cast<size_t>(0.30 * k_g_sr))
                    v.trigger_closed(1.0);
                if (i == static_cast<size_t>(0.60 * k_g_sr))
                    v.trigger_open(1.0);
                if (i == static_cast<size_t>(1.60 * k_g_sr))
                    v.trigger_open(1.0);
                if (i == static_cast<size_t>(1.85 * k_g_sr))
                    v.trigger_closed(1.0); // chokes the ring above
                (void)t;
                y[i] = v.process();
            }
            write_wav(dir + "/tr808_hat.wav", y, k_g_sr);
        }
        { // cymbal: decay travel, then tone extremes, then a worn unit
            cymbal v;
            v.prepare(k_g_sr);
            std::vector<double> y;
            for (double d : {0.0, 1.0}) {
                v.set_decay(d);
                append(y, take(d > 0.5 ? 2.0 : 1.0, [&] { v.trigger(1.0); }, [&] { return v.process(); }));
            }
            v.set_decay(0.5);
            for (double t : {0.0, 1.0}) {
                v.set_tone(t);
                append(y, take(1.2, [&] { v.trigger(1.0); }, [&] { return v.process(); }));
            }
            v.set_tone(0.5);
            v.set_tolerance(1.0);
            v.set_seed(13);
            append(y, take(1.5, [&] { v.trigger(1.0); }, [&] { return v.process(); }));
            write_wav(dir + "/tr808_cymbal.wav", y, k_g_sr);
        }
        { // cowbell + rimshot + claves + tom/conga ladder
            std::vector<double> y;
            cowbell             cbv;
            cbv.prepare(k_g_sr);
            append(y, take(0.5, [&] { cbv.trigger(1.0); }, [&] { return cbv.process(); }));
            rim rv;
            rv.prepare(k_g_sr);
            append(y, take(0.35, [&] { rv.trigger(1.0); }, [&] { return rv.process(); }));
            rv.set_model(rim::model_claves);
            append(y, take(0.35, [&] { rv.trigger(1.0); }, [&] { return rv.process(); }));
            for (int model : {tom::model_tom, tom::model_conga})
                for (int size : {0, 1, 2}) {
                    tom tv;
                    tv.prepare(k_g_sr);
                    tv.set_model(model);
                    tv.set_size(size);
                    append(y, take(0.45, [&] { tv.trigger(1.0); }, [&] { return tv.process(); }));
                }
            write_wav(dir + "/tr808_percussion.wav", y, k_g_sr);
        }
        { // the full kit, two bars at 116 BPM
            kick    k;
            snare   s;
            clap    cp;
            hat     h;
            cymbal  cy;
            cowbell cb;
            tom     tm;
            rim     rs;
            k.prepare(k_g_sr);
            k.set_decay(0.4);
            s.prepare(k_g_sr);
            cp.prepare(k_g_sr);
            h.prepare(k_g_sr);
            h.set_decay(0.5);
            cy.prepare(k_g_sr);
            cy.set_decay(0.5);
            cy.set_level(0.6);
            cb.prepare(k_g_sr);
            cb.set_level(0.45);
            tm.prepare(k_g_sr);
            tm.set_size(1);
            rs.prepare(k_g_sr);
            rs.set_level(0.8);
            const double        beat = 60.0 / 116.0;
            std::vector<double> y(static_cast<size_t>((0.1 + 8 * beat + 1.5) * k_g_sr), 0.0);
            auto                at = [&](double t) { return static_cast<size_t>((0.05 + t * beat) * k_g_sr); };
            for (size_t i = 0; i < y.size(); ++i) {
                for (int b = 0; b < 8; ++b) {
                    if (at(b) == i) {
                        if (b % 2 == 0)
                            k.trigger(b % 4 == 0 ? 1.0 : 0.8);
                        if (b % 4 == 1)
                            s.trigger(0.9);
                        if (b == 7)
                            cp.trigger(1.0);
                        if (b == 0)
                            cy.trigger(0.8);
                        if (b == 5)
                            cb.trigger(0.7);
                        if (b == 6)
                            tm.trigger(0.9);
                        if (b == 3)
                            rs.trigger(0.6);
                    }
                }
                for (int e = 0; e < 16; ++e)
                    if (at(e * 0.5) == i) {
                        if (e % 8 == 6)
                            h.trigger_open(0.8);
                        else
                            h.trigger_closed(e % 2 ? 0.45 : 0.75);
                    }
                y[i] = 0.9 * k.process() + 0.65 * s.process() + 0.6 * cp.process() + 0.75 * h.process() + cy.process()
                       + cb.process() + 0.8 * tm.process() + rs.process();
            }
            write_wav(dir + "/tr808_kit.wav", y, k_g_sr);
        }
    }

} // namespace

int main(int argc, char** argv) {
    if (argc >= 3 && std::strcmp(argv[1], "--hit") == 0) {
        const std::string name   = argv[2];
        const double      accent = argc >= 4 ? std::atof(argv[3]) : 1.0;
        const std::string dir    = argc >= 5 ? argv[4] : ".";
        auto              y      = single_hit(name, accent);
        if (y.empty()) {
            std::fprintf(stderr,
                         "unknown voice '%s' (kick|snare|clap|maracas|hat|openhat|cymbal|"
                         "cowbell|tom|conga|rimshot|claves)\n",
                         name.c_str());
            return 1;
        }
        return write_wav(dir + "/tr808_" + name + "_hit.wav", y, k_g_sr) ? 0 : 1;
    }

    const std::string dir = argc >= 2 ? argv[1] : ".";
    render_demos(dir);
    return 0;
}
