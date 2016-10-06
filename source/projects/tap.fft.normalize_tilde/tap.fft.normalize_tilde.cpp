/// @file	
///	@ingroup 	minexamples
///	@copyright	Copyright (c) 2016, Cycling '74
/// @author		Timothy Place
///	@license	Usage of this file and its contents is governed by the MIT License

#include "c74_min.h"

using namespace c74::min;

class tap_fft_normalize_tilde : public object<tap_fft_normalize_tilde>, public sample_operator<3,2> {
public:

	MIN_DESCRIPTION { "Normalize the results of an FFT. See <a href='https://www.mathworks.com/matlabcentral/answers/15770-scaling-the-fft-and-the-ifft'>this article</a> for more information." };
	MIN_TAGS		{ "fft, spectral, analysis" };
	MIN_AUTHOR		{ "Timothy Place" };
	MIN_RELATED		{ "fft~, tap.fft.list~" };
	
	inlet<>		real_in	{ this, "(signal) real input from fft~" };
	inlet<>		imag_in	{ this, "(signal) imaginary input from fft~" };
	inlet<>		index	{ this, "(signal) index from fft~" };
	outlet<>	real_out{ this, "(signal) normalized real output", "signal" };
	outlet<>	imag_out{ this, "(signal) normalized imaginary output", "signal" };

	argument<number> framesize_arg	{ this, "framesize", "The number of bins in each FFT frame.",
		MIN_ARGUMENT_FUNCTION {
			framesize = arg;
		}
	};

	attribute<int> framesize { this, "framesize", 512,
		description { "Number of bins in the FFT." },
		setter { MIN_FUNCTION {
			int new_framesize = args[0];

			m_framesize_half = new_framesize / 2.0;
			m_last_index = new_framesize - 1;
			return args;
		}}
	};

	samples<2> operator()(sample real, sample imag, sample index) {
		real /= m_framesize_half;
		imag /= m_framesize_half;
		imag *= -1.0;

		auto indexi = std::lround(index);
		if (indexi == 0 || indexi == m_last_index)
			real *= 0.5;
		return {{ real, imag }};
	}

private:
	double	m_framesize_half;
	int		m_last_index;
};

MIN_EXTERNAL(tap_fft_normalize_tilde);
