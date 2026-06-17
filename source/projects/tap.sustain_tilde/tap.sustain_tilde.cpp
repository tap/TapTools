/// @file
/// @brief      tap.sustain~ — capture recent audio into a seamless sustaining loop.
/// @details    Continuously records the input into a ring buffer. On a bang, the most recently
///             recorded material is captured: trimmed to significant-power rising zero-crossings at
///             both ends, then played back as a crossfaded forward/backward loop so it sustains
///             without clicks. DSP is plain portable C++.
///
///             EXPERIMENTAL — ported from the 2019 "taptools-min" effort (originally Cycling '74,
///             MIT). That version was an in-development prototype: only a single voice is wired up
///             and the multi-voice / rise behavior is still stubbed. Brought onto the modern
///             toolchain (C++20, MSVC-safe) and made safe to instantiate, but the feature set is
///             not yet complete — see REVIVAL.md and the tap.sustain1~..4~ redesign patchers.
/// @author     Timothy Place
/// @copyright  Copyright 2019-2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min.h"
#include <cmath>
#include <numbers>

using namespace c74::min;


// A single sustaining voice: holds a captured snippet and plays it as a crossfaded forward/backward
// loop. Each voice could eventually be routed to its own channel for independent filtering/rise/tail.
class sustain_voice {
public:
    void resize(int newsize) {
        m_buffer.resize(newsize);
    }

    // Capture the `m_buffer.size()` samples immediately before `record_head` from the master input,
    // trimming each end to a rising zero-crossing whose level exceeds the snippet's RMS.
    void capture(vector<sample>& input, int record_head) {
        const int start_initial = record_head - static_cast<int>(m_buffer.size());
        const int end_initial   = record_head - 1;
        int       start         = start_initial;
        int       end           = end_initial;

        // average power of the captured region
        number rms {};
        for (auto i = start; i < end; ++i) {
            auto j = i % input.size();
            rms += input[j] * input[j];
        }
        rms /= static_cast<number>(input.size());
        rms = std::sqrt(rms);

        // trim to a (rising) zero-crossing of significant power at the beginning
        for (auto i = start + 1; i < end; i++) {
            auto k = i % input.size();
            if (input[k] > 0.0 && input[k - 1] < 0.0) {
                for (auto j = k; j < input.size(); j++) {
                    if (input[j] > input[j - 1]) {
                        if (input[j] >= rms) {
                            start = i;
                            break;
                        }
                    }
                    else
                        break;
                }
            }
            if (start != start_initial)
                break;
        }

        // trim to a (rising) zero-crossing of significant power at the end
        for (auto i = end; i > 0; --i) {
            auto k = i % input.size();
            if (input[k] > 0.0 && input[k - 1] < 0.0) {
                for (auto j = k; j > 0; j--) {
                    if (input[j] < input[j - 1])
                        break;
                    if (input[j] <= -rms) {
                        end = i;
                        break;
                    }
                }
            }
            if (end != end_initial)
                break;
        }

        m_size = end - start;
        for (auto i = 0; i < m_size; ++i) {
            auto j = (start + i) % input.size();
            m_buffer[i] = input[j];
        }
        m_frame = 0;
    }

    void clear() {
        m_buffer.assign(m_buffer.size(), 0.0);
        m_size  = 0;
        m_frame = 0;
    }

    void fade(int fadetime_in_samples) {
        m_fade_size      = fadetime_in_samples;
        m_fade_step_size = (m_fade_size > 0) ? 1.0 / m_fade_size : 0.0;
    }
    int fade() const { return m_fade_size; }

    void rise(int risetime_in_samples) {
        m_rise_size      = risetime_in_samples;
        m_rise_step_size = (m_rise_size > 0) ? 1.0 / m_rise_size : 0.0;
    }
    int rise() const { return m_rise_size; }

    void operator()(audio_bundle output) {
        if (m_size <= 0 || m_fade_size <= 0)
            return;    // nothing captured yet — stay silent rather than read uninitialized state

        auto      out = output.samples(0);
        const int N   = m_size * 2 - m_fade_size * 2;
        if (N <= 0)
            return;

        for (auto n = 0; n < output.frame_count(); ++n) {
            if (m_frame < m_fade_size) {
                sample s1 = m_buffer[m_frame] * equal_power(m_fade_step_size * m_frame);
                int    i  = (m_fade_size - 1) - m_frame;
                sample s2 = m_buffer[i] * equal_power(m_fade_step_size * i);
                out[n] += s1 + s2;
            }
            else if (m_frame < m_size - m_fade_size) {
                out[n] += m_buffer[m_frame];
            }
            else if (m_frame < m_size) {
                sample s1 = m_buffer[m_frame] * equal_power(m_fade_step_size * (m_size - m_frame));
                int    i  = (N - m_frame) + (m_fade_size - 1);
                sample s2 = m_buffer[i] * equal_power(m_fade_step_size * (m_size - i));
                out[n] += s1 + s2;
            }
            else {
                int i = (N - m_frame) + (m_fade_size - 1);
                out[n] += m_buffer[i];
            }

            ++m_frame;
            if (m_frame >= N)
                m_frame = 0;
        }
    }

private:
    vector<sample> m_buffer;
    int            m_size { 0 };
    int            m_fade_size { 0 };
    number         m_fade_step_size { 0.0 };
    int            m_rise_size { 0 };
    number         m_rise_step_size { 0.0 };
    int            m_frame { 0 };

    static sample equal_power(sample position) {
        return std::sin(position * std::numbers::pi_v<sample> / 2.0);
    }
};


class sustain : public object<sustain>, public vector_operator<> {
    vector<sustain_voice> m_voices;
    vector<sample>        m_buffer;
    int                   m_buffer_sizemask { 0 };
    int                   m_record_head { 0 };
    number                m_ms_samplerate { 0.0 };
    number                m_ms_inv_samplerate { 0.0 };
    bool                  m_need_to_capture { false };

public:
    MIN_DESCRIPTION { "Capture recent audio into a seamless sustaining loop. Continuously records the "
                      "input; on a bang the most recent material is trimmed to zero-crossings and "
                      "played back as a crossfaded loop so it sustains without clicks. EXPERIMENTAL "
                      "(single-voice prototype ported from the 2019 taptools-min effort)." };
    MIN_TAGS    { "buffers" };
    MIN_AUTHOR  { "Timothy Place" };
    MIN_RELATED { "tap.buffer.record~, buffer~, record~" };

    inlet<>  input         { this, "(signal) audio to record; bang captures a sustaining loop" };
    outlet<> output_signal { this, "(signal) sustaining loop output", "signal" };

    sustain(const atoms& args = {})
    : m_voices(5) {
        update_samplerate();
    }

    message<> bang { this, "bang", "Capture the most recently recorded audio into a sustaining loop.",
        MIN_FUNCTION {
            m_need_to_capture = true;
            return {};
        }
    };

    message<> clear { this, "clear", "Erase the captured loop.",
        MIN_FUNCTION {
            for (auto& voice : m_voices)
                voice.clear();
            return {};
        }
    };

    attribute<number> length { this, "length", 1000.0,
        description { "Maximum length of the captured loop, in milliseconds. (Changing this restarts "
                      "the recording buffer.)" }
    };

    attribute<number> fade { this, "fade", 100.0,
        description { "Length of the crossfade across the loop points, in milliseconds." },
        setter { MIN_FUNCTION {
            number fadetime_in_samples = static_cast<number>(args[0]) * m_ms_samplerate;
            if (!m_voices.empty() && fadetime_in_samples != m_voices[0].fade()) {
                for (auto& voice : m_voices)
                    voice.fade(static_cast<int>(fadetime_in_samples));
            }
            return args;
        }}
    };

    attribute<number> rise { this, "rise", 100.0,
        description { "Time over which newly captured material grows to full power, in milliseconds." },
        setter { MIN_FUNCTION {
            number rise_in_samples = static_cast<number>(args[0]) * m_ms_samplerate;
            if (!m_voices.empty() && rise_in_samples != m_voices[0].rise()) {
                for (auto& voice : m_voices)
                    voice.rise(static_cast<int>(rise_in_samples));
            }
            return args;
        }}
    };

    message<> dspsetup { this, "dspsetup",
        MIN_FUNCTION {
            update_samplerate();
            return {};
        }
    };

    void operator()(audio_bundle input_bundle, audio_bundle output) override {
        auto in = input_bundle.samples(0);

        // always recording into the master ring buffer
        for (auto n = 0; n < output.frame_count(); ++n) {
            m_buffer[m_record_head] = in[n];
            ++m_record_head;
            m_record_head &= m_buffer_sizemask;
        }

        if (m_need_to_capture) {
            m_need_to_capture = false;
            capture();
        }

        // accumulate all of the voices
        output.clear();
        for (auto& voice : m_voices)
            voice(output);
    }

private:
    void update_samplerate() {
        m_ms_samplerate     = samplerate() * 0.001;
        m_ms_inv_samplerate = (1.0 / samplerate()) * 1000.0;
        resize();
    }

    void resize() {
        int  size_in_samples       = static_cast<int>(static_cast<number>(length) * m_ms_samplerate);
        auto size_in_samples_voice = size_in_samples;

        size_in_samples = limit_to_power_of_two(size_in_samples);
        if (size_in_samples != static_cast<int>(m_buffer.size())) {
            m_buffer.resize(size_in_samples);
            m_buffer_sizemask = size_in_samples - 1;
            m_record_head     = 0;
            for (auto& voice : m_voices)
                voice.resize(size_in_samples_voice);
        }

        fade = static_cast<number>(fade);    // re-derive the fade in samples for the new rate
    }

    void capture() {
        m_voices[0].capture(m_buffer, m_record_head);
    }
};


MIN_EXTERNAL(sustain);
