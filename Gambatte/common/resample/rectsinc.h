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
#ifndef RECTSINC_H
#define RECTSINC_H

#include "convoluter.h"
#include "subresampler.h"
#include "makesinckernel.h"
#include "cic2.h"
#include <cmath>
#include <cstdlib>

template<unsigned channels, unsigned phases>
class RectSinc : public SubResampler {
	PolyPhaseConvoluter<channels, phases> convoluters[channels];
	short *kernel;
	
	static double rectWin(const long /*i*/, const long /*M*/) {
		return 1;
	}
	
	void init(unsigned div, unsigned phaseLen, double fc);
	
public:
	enum { MUL = phases };
	
	typedef Cic2<channels> Cic;
	static float cicLimit() { return 2.0f; }

	class RollOff {
		static unsigned toTaps(const float rollOffWidth) {
			static const float widthTimesTaps = 0.9f;
			return std::ceil(widthTimesTaps / rollOffWidth);
		}
		
		static float toFc(const float rollOffStart, const int taps) {
			static const float startToFcDeltaTimesTaps = 0.43f;
			return startToFcDeltaTimesTaps / taps + rollOffStart;
		}
		
	public:
		const unsigned taps;
		const float fc;
		
		RollOff(float rollOffStart, float rollOffWidth) : taps(toTaps(rollOffWidth)), fc(toFc(rollOffStart, taps)) {}
	};

	RectSinc(unsigned div, unsigned phaseLen, double fc) { init(div, phaseLen, fc); }
	RectSinc(unsigned div, RollOff ro) { init(div, ro.taps, ro.fc); }
	~RectSinc() { delete[] kernel; }
	std::size_t resample(short *out, const short *in, std::size_t inlen);
	void adjustDiv(unsigned div);
	unsigned mul() const { return MUL; }
	unsigned div() const { return convoluters[0].div(); }
};

template<const unsigned channels, const unsigned phases>
void RectSinc<channels, phases>::init(const unsigned div, const unsigned phaseLen, const double fc) {
	kernel = new short[phaseLen * phases];
	
	makeSincKernel(kernel, phases, phaseLen, fc, rectWin);
	
	for (unsigned i = 0; i < channels; ++i)
		convoluters[i].reset(kernel, phaseLen, div);
}

template<const unsigned channels, const unsigned phases>
std::size_t RectSinc<channels, phases>::resample(short *const out, const short *const in, const std::size_t inlen) {
	std::size_t samplesOut;
	
	for (unsigned i = 0; i < channels; ++i)
		samplesOut = convoluters[i].filter(out + i, in + i, inlen);
	
	return samplesOut;
}

template<const unsigned channels, const unsigned phases>
void RectSinc<channels, phases>::adjustDiv(const unsigned div) {
	for (unsigned i = 0; i < channels; ++i)
		convoluters[i].adjustDiv(div);
}

#endif
