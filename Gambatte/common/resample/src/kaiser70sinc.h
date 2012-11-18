/***************************************************************************
 *   Copyright (C) 2008-2009 by Sindre Aamås                               *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef KAISER70SINC_H
#define KAISER70SINC_H

#include "convoluter.h"
#include "subresampler.h"
#include "makesinckernel.h"
#include "i0.h"
#include "cic4.h"
#include "array.h"
#include <cmath>
#include <cstdlib>

template<unsigned channels, unsigned phases>
class Kaiser70Sinc : public SubResampler {
	PolyPhaseConvoluter<channels, phases> convoluters[channels];
	Array<short> kernel;
	
	static double kaiserWin(const long n, const long M) {
		static const double beta = 6.9;
		static const double i0beta_rec = 1.0 / i0(beta);
		
		double x = static_cast<double>(n * 2) / M - 1.0;
		x = x * x;
		x = beta * std::sqrt(1.0 - x);
		
		return i0(x) * i0beta_rec;
	}
	
	void init(unsigned div, unsigned phaseLen, double fc);
	
public:
	enum { MUL = phases };
	
	typedef Cic4<channels> Cic;
	static float cicLimit() { return 4.7f; }

	class RollOff {
		static unsigned toTaps(const float rollOffWidth) {
			static const float widthTimesTaps = 3.75f;
			return static_cast<unsigned>(std::ceil(widthTimesTaps / rollOffWidth));
		}
		
		static float toFc(const float rollOffStart, const int taps) {
			static const float startToFcDeltaTimesTaps = 1.5f;
			return startToFcDeltaTimesTaps / taps + rollOffStart;
		}
		
	public:
		const unsigned taps;
		const float fc;
		
		RollOff(float rollOffStart, float rollOffWidth) : taps(toTaps(rollOffWidth)), fc(toFc(rollOffStart, taps)) {}
	};

	Kaiser70Sinc(unsigned div, unsigned phaseLen, double fc) { init(div, phaseLen, fc); }
	Kaiser70Sinc(unsigned div, RollOff ro) { init(div, ro.taps, ro.fc); }
	std::size_t resample(short *out, const short *in, std::size_t inlen);
	void adjustDiv(unsigned div);
	unsigned mul() const { return MUL; }
	unsigned div() const { return convoluters[0].div(); }
};

template<unsigned channels, unsigned phases>
void Kaiser70Sinc<channels, phases>::init(const unsigned div, const unsigned phaseLen, const double fc) {
	kernel.reset(phaseLen * phases);
	
	makeSincKernel(kernel, phases, phaseLen, fc, kaiserWin);
	
	for (unsigned i = 0; i < channels; ++i)
		convoluters[i].reset(kernel, phaseLen, div);
}

template<unsigned channels, unsigned phases>
std::size_t Kaiser70Sinc<channels, phases>::resample(short *const out, const short *const in, const std::size_t inlen) {
	std::size_t samplesOut;
	
	for (unsigned i = 0; i < channels; ++i)
		samplesOut = convoluters[i].filter(out + i, in + i, inlen);
	
	return samplesOut;
}

template<unsigned channels, unsigned phases>
void Kaiser70Sinc<channels, phases>::adjustDiv(const unsigned div) {
	for (unsigned i = 0; i < channels; ++i)
		convoluters[i].adjustDiv(div);
}

#endif
