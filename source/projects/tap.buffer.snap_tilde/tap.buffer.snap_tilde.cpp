/// @file
///	@copyright	Copyright (c) 2019, Cycling '74
/// @author		Timothy Place
///	@license	Usage of this file and its contents is governed by the MIT License

#include "c74_min.h"

using namespace c74::min;

class tap_buffer_snap_tilde : public object<tap_buffer_snap_tilde>, public vector_operator<> {	
public:
	
	MIN_DESCRIPTION { "Snap a time to the nearest zero-crossing location in a buffer." };
	MIN_TAGS		{ "buffer" };
	MIN_AUTHOR		{ "Timothy Place" };
	MIN_RELATED		{ "buffer~, record~, tap.buffer.record~" };
	
	inlet<>		m_input			{ this, "(signal/number) buffer location in milliseconds" };
	outlet<>	m_output_signal	{ this, "(signal) nearest zero-crossing in milliseconds", "signal" };
	outlet<>	m_output_number	{ this, "(number) nearest zero-crossing in milliseconds" };
	
	argument<symbol> name_arg	{ this, "name", "Name of the buffer to reference.",
		MIN_ARGUMENT_FUNCTION {
			m_buffer.set(arg);
		}
	};

	buffer_reference m_buffer { this };
	
	tap_buffer_snap_tilde() {
		update_samplerate();
	}
	
	message<> m_number { this, "number", "A location in the buffer to be quantized to the nearest zero-crossing.",
		MIN_FUNCTION {
			if (m_buffer) {
				buffer_lock<false> b(m_buffer);
				m_output_number.send(calc(b, args[0]));
			}
			else
				cerr << "invalid buffer" << endl;
			return {};
		}
	};
	
	message<> dspsetup { this, "dspsetup",
		MIN_FUNCTION {
			update_samplerate();
			return {};
		}
	};
				
	void operator()(audio_bundle input, audio_bundle output) {
		auto			in = input.samples(0);
		auto			out = output.samples(0);
		buffer_lock<>	b(m_buffer);
		
		if (b.valid()) {
			for (auto n=0; n<input.frame_count(); ++n)
				out[n] = calc(b, in[n]);
		}
		else {
			output.clear();
		}
	}
				
private:
	number m_ms_samplerate;
	number m_ms_inv_samplerate;
				
	void update_samplerate() {
		m_ms_samplerate = samplerate() * 0.001;
		m_ms_inv_samplerate = (1.0 / samplerate()) * 1000.0;
	}
				
	template<bool audio_thread>
	number calc(buffer_lock<audio_thread>&b, number x) {
		auto	frames = b.frame_count();
		int		snap = std::round(x * m_ms_samplerate);	// convert to samples
		int		index = MIN_CLAMP(snap, 0, frames);
		sample	current_samp = b[index];
		
		if (current_samp != 0.0) {
			enum {
				unknown,
				nearest_zerocross_is_higher,
				nearest_zerocross_is_lower
			} flag = unknown;
			bool	above_zero = (current_samp > 0);
			auto	i = 0;
			int		index1;
			int		index2;
			
			while (flag == unknown) {
				++i;
				index1 = wrap<int>(index+i, 0, frames);
				index2 = wrap<int>(index-i, 0, frames);
				
				if (above_zero != (b[index1] > 0.0))
					flag = nearest_zerocross_is_higher;
				else if (above_zero != (b[index2] > 0.0))
					flag = nearest_zerocross_is_lower;
				
				if (i >= frames) // no zero-crossing found!
					break;
			}
			
			if (flag == nearest_zerocross_is_higher)
				snap = index1;
			else if (flag == nearest_zerocross_is_lower)
				snap = index2;
		}
		
		x = snap * m_ms_inv_samplerate;
		return x;
	}
};


MIN_EXTERNAL(tap_buffer_snap_tilde);
