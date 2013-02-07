/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aamås                                    *
 *   sinamas@users.sourceforge.net                                         *
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
#include "array.h"
#include <algorithm>
#include <cmath>
#include <cstddef>

template<unsigned channels, unsigned phases>
class RectSinc : public SubResampler {
	Array<short> const kernel;
	PolyPhaseConvoluter<channels, phases> convoluter_;

	static double rectWin(const long /*i*/, const long /*M*/) { return 1; }

public:
	enum { MUL = phases };
	typedef Cic2<channels> Cic;
	static float cicLimit() { return 2.0f; }

	class RollOff {
		static unsigned toTaps(const float rollOffWidth) {
			const float widthTimesTaps = 0.9f;
			return std::max(static_cast<unsigned>(std::ceil(widthTimesTaps / rollOffWidth)), 4u);
		}
		
		static float toFc(const float rollOffStart, const int taps) {
			const float startToFcDeltaTimesTaps = 0.43f;
			return startToFcDeltaTimesTaps / taps + rollOffStart;
		}
		
	public:
		const unsigned taps;
		const float fc;
		
		RollOff(float rollOffStart, float rollOffWidth) : taps(toTaps(rollOffWidth)), fc(toFc(rollOffStart, taps)) {}
	};

	RectSinc(unsigned div, unsigned phaseLen, double fc)
	: kernel(phaseLen * phases), convoluter_(kernel, phaseLen, div)
	{ makeSincKernel(kernel, phases, phaseLen, fc, rectWin, 1.0); }
	
	RectSinc(unsigned div, RollOff ro, double gain)
	: kernel(ro.taps * phases), convoluter_(kernel, ro.taps, div)
	{ makeSincKernel(kernel, phases, ro.taps, ro.fc, rectWin, gain);}
	
	std::size_t resample(short *out, const short *in, std::size_t inlen) { return convoluter_.filter(out, in, inlen); }
	void adjustDiv(unsigned div) { convoluter_.adjustDiv(div); }
	unsigned mul() const { return MUL; }
	unsigned div() const { return convoluter_.div(); }
};

#endif
