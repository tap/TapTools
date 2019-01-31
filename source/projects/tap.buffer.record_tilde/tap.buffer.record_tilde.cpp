/// @file	
///	@ingroup 	minexamples
///	@copyright	Copyright (c) 2019, Cycling '74
/// @author		Timothy Place
///	@license	Usage of this file and its contents is governed by the MIT License

#include "c74_min.h"

using namespace c74::min;

class tap_buffer_record_tilde : public object<tap_buffer_record_tilde>, public vector_operator<> {
private:
	vector<sample>	m_inc;
	unsigned int	m_i {0};
	unsigned int	m_j {0};

public:

	MIN_DESCRIPTION { "loop recording for single-channel buffers with fade in of new material" };
	MIN_TAGS		{ "buffer" };
	MIN_AUTHOR		{ "Timothy Place" };
	MIN_RELATED		{ "buffer~, record~" };
	
	inlet<>		m_input	{ this, "(signal) input" };
	outlet<>	m_sync	{ this, "(signal) sync", "signal" };
	
	
	argument<symbol> name_arg	{ this, "name", "Name of the buffer to reference.",
		MIN_ARGUMENT_FUNCTION {
			m_buffer.set(arg);
		}
	};
	
	
	argument<int> fade_arg	{ this, "fade", "Size of the crossfade.",
		MIN_ARGUMENT_FUNCTION {
			m_fade = arg;
		}
	};

	
	buffer_reference m_buffer { this };
	
	
	message<> m_int { this, "int", "Turn on/off recording.",
		MIN_FUNCTION {
			m_record = args[0];
			return {};
		}
	};
	
	
	message<> dspsetup { this, "dspsetup",
		MIN_FUNCTION {
			m_inc.assign(m_inc.size(), 0.0);
			m_i = 0;
			m_j = 0;
			return {};
		}
	};

				
	attribute<int, threadsafe::no, limit::clamp> m_fade { this, "fade", 16,
		description { "The number of samples over which to blend in new input." },
		range {1, 4096},
		setter {
			MIN_FUNCTION {
				auto val = limit_to_power_of_two<int>(args[0]);
				m_inc.resize(val);
				m_inc.assign(val, 0.0);
				m_j = 0;
				return { val };
			}
		}
	};

	
	attribute<bool> m_record { this, "record", false,
		description { "Record audio input." },
		setter {
			MIN_FUNCTION {
				bool val = args[0];
				if (val == false) {
					m_i = 0;
					m_j = 0;
				}
				return args;
			}
		}
	};
	
				
	attribute<bool> m_loop { this, "loop", 1,
		description { "Enable loop recording." }
	};

	
	void operator()(audio_bundle input, audio_bundle output) {
		auto			in = input.samples(0);
		auto			sync = output.samples(0);
		buffer_lock<>	b(m_buffer);
		
		if (m_record && b.valid()) {
			auto	frames = b.frame_count();
			auto 	frames_inv = 1.0 / static_cast<double>(frames);
			int		fade = m_fade;
			auto	fade_inv = 1.0 / static_cast<double>(fade);
			auto	fade_max = fade - 1;
			auto 	i = m_i;
			auto	j = m_j;

			for (auto n=0; n<input.frame_count(); ++n) {
				auto x = in[n];
				
				// wrap the heads
				if ((i >= frames) && (m_loop == true))
					i = 0;
				else if ((i >= frames) && (m_loop == false)) {
					i = 0;
					m_record = false;
					for (; n<input.frame_count(); ++n)
						sync[n] = 0.0;
					return;
				}
				
				j &= fade_max;
				
				// calculate the the amount to increment on each iteration to ramp to the new value in the buffer
				m_inc[j] = (x - b[i]) * fade_inv;
				
				// increment all values currently being faded-in
				for (unsigned int k=0; k<fade; ++k) {
					auto buffer_index = wrap_once<int>(static_cast<int>(i) - static_cast<int>(k), 0, frames-1);
					auto inc_index = (j-k) & fade_max;
					b[ buffer_index ] += m_inc[ inc_index ];
				}

				// move the heads.
				++i;
				++j;
				
				sync[n] = i * frames_inv;	// output the normalized index
			}
			b.dirty();
			m_i = i;
			m_j = j;
		}
		else {
			output.clear();
		}
	}
};


MIN_EXTERNAL(tap_buffer_record_tilde);
