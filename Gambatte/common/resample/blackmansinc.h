/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aamås                                    *
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
#ifndef BLACKMANSINC_H
#define BLACKMANSINC_H

#include "convoluter.h"
#include "subresampler.h"
#include "makesinckernel.h"
#include "cic4.h"
#include <cmath>
#include <cstdlib>

template<unsigned channels, unsigned phases>
class BlackmanSinc : public SubResampler {
	PolyPhaseConvoluter<channels, phases> convoluters[channels];
	short *kernel;
	
	static double blackmanWin(const long i, const long M) {
		static const double PI = 3.14159265358979323846;
		return 0.42 - 0.5 * std::cos(2 * PI * i / M) + 0.08 * std::cos(4 * PI * i / M);
	}
	
	void init(unsigned div, unsigned phaseLen, double fc);
	
public:
	enum { MUL = phases };
	
	typedef Cic4<channels> Cic;
	static float cicLimit() { return 4.7f; }

	class RollOff {
		static unsigned toTaps(const float rollOffWidth) {
			static const float widthTimesTaps = 4.5f;
			return std::ceil(widthTimesTaps / rollOffWidth);
		}
		
		static float toFc(const float rollOffStart, const int taps) {
			static const float startToFcDeltaTimesTaps = 1.69f;
			return startToFcDeltaTimesTaps / taps + rollOffStart;
		}
		
	public:
		const unsigned taps;
		const float fc;
		
		RollOff(float rollOffStart, float rollOffWidth) : taps(toTaps(rollOffWidth)), fc(toFc(rollOffStart, taps)) {}
	};

	BlackmanSinc(unsigned div, unsigned phaseLen, double fc) { init(div, phaseLen, fc); }
	BlackmanSinc(unsigned div, RollOff ro) { init(div, ro.taps, ro.fc); }
	~BlackmanSinc() { delete[] kernel; }
	std::size_t resample(short *out, const short *in, std::size_t inlen);
	void adjustDiv(unsigned div);
	unsigned mul() const { return MUL; }
	unsigned div() const { return convoluters[0].div(); }
};

template<const unsigned channels, const unsigned phases>
void BlackmanSinc<channels, phases>::init(const unsigned div, const unsigned phaseLen, const double fc) {
	kernel = new short[phaseLen * phases];
	
	makeSincKernel(kernel, phases, phaseLen, fc, blackmanWin);
	
	for (unsigned i = 0; i < channels; ++i)
		convoluters[i].reset(kernel, phaseLen, div);
}

template<const unsigned channels, const unsigned phases>
std::size_t BlackmanSinc<channels, phases>::resample(short *const out, const short *const in, const std::size_t inlen) {
	std::size_t samplesOut;
	
	for (unsigned i = 0; i < channels; ++i)
		samplesOut = convoluters[i].filter(out + i, in + i, inlen);
	
	return samplesOut;
}

template<const unsigned channels, const unsigned phases>
void BlackmanSinc<channels, phases>::adjustDiv(const unsigned div) {
	for (unsigned i = 0; i < channels; ++i)
		convoluters[i].adjustDiv(div);
}

#endif
