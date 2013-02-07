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
#ifndef HAMMINGSINC_H
#define HAMMINGSINC_H

#include "convoluter.h"
#include "subresampler.h"
#include "makesinckernel.h"
#include "cic3.h"
#include "array.h"
#include <algorithm>
#include <cmath>
#include <cstddef>

template<unsigned channels, unsigned phases>
class HammingSinc : public SubResampler {
	Array<short> const kernel;
	PolyPhaseConvoluter<channels, phases> convoluter_;

	static double hammingWin(const long i, const long M) {
		const double PI = 3.14159265358979323846;
		return 0.53836 - 0.46164 * std::cos(2 * PI * i / M);
	}

public:
	enum { MUL = phases };
	typedef Cic3<channels> Cic;
	static float cicLimit() { return 4.2f; }

	class RollOff {
		static unsigned toTaps(const float rollOffWidth) {
			const float widthTimesTaps = 3.0f;
			return std::max(static_cast<unsigned>(std::ceil(widthTimesTaps / rollOffWidth)), 4u);
		}
		
		static float toFc(const float rollOffStart, const int taps) {
			const float startToFcDeltaTimesTaps = 1.27f;
			return startToFcDeltaTimesTaps / taps + rollOffStart;
		}
		
	public:
		const unsigned taps;
		const float fc;
		
		RollOff(float rollOffStart, float rollOffWidth) : taps(toTaps(rollOffWidth)), fc(toFc(rollOffStart, taps)) {}
	};

	HammingSinc(unsigned div, unsigned phaseLen, double fc)
	: kernel(phaseLen * phases), convoluter_(kernel, phaseLen, div)
	{ makeSincKernel(kernel, phases, phaseLen, fc, hammingWin, 1.0); }
	
	HammingSinc(unsigned div, RollOff ro, double gain)
	: kernel(ro.taps * phases), convoluter_(kernel, ro.taps, div)
	{ makeSincKernel(kernel, phases, ro.taps, ro.fc, hammingWin, gain);}
	
	std::size_t resample(short *out, const short *in, std::size_t inlen) { return convoluter_.filter(out, in, inlen); }
	void adjustDiv(unsigned div) { convoluter_.adjustDiv(div); }
	unsigned mul() const { return MUL; }
	unsigned div() const { return convoluter_.div(); }
};

#endif
