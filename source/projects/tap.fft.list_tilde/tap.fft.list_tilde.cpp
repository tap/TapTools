/// @file	
///	@ingroup 	minexamples
///	@copyright	Copyright (c) 2016, Cycling '74
/// @author		Timothy Place
///	@license	Usage of this file and its contents is governed by the MIT License

#include "c74_min.h"

using namespace c74::min;

class tap_fft_list_tilde : public object<tap_fft_list_tilde>, public sample_operator<2,1> {
public:

	MIN_DESCRIPTION { "Output FFT frames as Max lists." };
	MIN_TAGS		{ "fft, spectral, analysis" };
	MIN_AUTHOR		{ "Timothy Place" };
	MIN_RELATED		{ "fft~, tap.fft.binmodulator~" };
	
	inlet	input	{ this, "(signal) frames of output from the fft~ object" };
	inlet	index	{ this, "(signal) index of the frames from the fft~ object" };
	outlet<>	output	{ this, "(list) frames formatted as lists" };
//	outlet<fifo::scheduler>	output

	argument<number> framesize_arg	{ this, "framesize", "The number of bins in each FFT frame.",
		MIN_ARGUMENT_FUNCTION {
			framesize = arg;
		}
	};


	attribute<int> framesize { this, "framesize", 512,
		description { "Number of bins in the FFT." },
		setter { MIN_FUNCTION {
			update_framesize(args[0], nyquist);
			return args;
		}}
	};


	attribute<bool> nyquist { this, "nyquist", true,
		description { "Limit output list to run from zero to the nyquist frequency." },
		setter { MIN_FUNCTION {
			update_framesize(framesize, args[0]);
			return args;
		}}
	};


	attribute<bool> autopoll { this, "autopoll", true,
		description { "automatically output any time a new frame is complete." }
	};


	attribute<number> mult { this, "mult", 1.0,
		description { "Factor by which to multiply values." }
	};


	message bang { this, "bang", "Output the frame of data.",
		MIN_FUNCTION {
			output.send(to_atoms(m_last));
			return {};
		}
	};


	sample operator()(sample value, sample index) {
		if (!nyquist || index < framesize/2)
			m_current[index] = value * mult;

		if (autopoll && index == framesize-1) {
			m_last = m_current;
			deliverer.delay(0);
		}

		return 0.0;
	}


	timer deliverer { this, MIN_FUNCTION {
		bang();
		return {};
	}};


private:
	numbers		m_current;	///< storage for the current frame
	numbers		m_last;		///< the most recent complete frame

	void update_framesize(int new_framesize, bool limit_at_nyquiest) {
		if (limit_at_nyquiest)
			m_current.resize(new_framesize / 2);
		else
			m_current.resize(new_framesize);
	}

};


MIN_EXTERNAL(tap_fft_list_tilde);
