/***************************************************************************
 *   Copyright (C) 2008-2009 by Sindre Aamås                               *
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
#ifndef KAISER70SINC_H
#define KAISER70SINC_H

#include "convoluter.h"
#include "subresampler.h"
#include "makesinckernel.h"
#include "cic4.h"
#include "array.h"
#include <algorithm>
#include <cmath>
#include <cstddef>

double kaiser70SincWin(long n, long M);

template<unsigned channels, unsigned phases>
class Kaiser70Sinc : public SubResampler {
	Array<short> const kernel;
	PolyPhaseConvoluter<channels, phases> convoluter_;

public:
	enum { MUL = phases };
	typedef Cic4<channels> Cic;
	static float cicLimit() { return 4.7f; }

	class RollOff {
		static unsigned toTaps(const float rollOffWidth) {
			const float widthTimesTaps = 3.75f;
			return std::max(static_cast<unsigned>(std::ceil(widthTimesTaps / rollOffWidth)), 4u);
		}
		
		static float toFc(const float rollOffStart, const int taps) {
			const float startToFcDeltaTimesTaps = 1.5f;
			return startToFcDeltaTimesTaps / taps + rollOffStart;
		}
		
	public:
		const unsigned taps;
		const float fc;
		
		RollOff(float rollOffStart, float rollOffWidth) : taps(toTaps(rollOffWidth)), fc(toFc(rollOffStart, taps)) {}
	};

	Kaiser70Sinc(unsigned div, unsigned phaseLen, double fc)
	: kernel(phaseLen * phases), convoluter_(kernel, phaseLen, div)
	{ makeSincKernel(kernel, phases, phaseLen, fc, kaiser70SincWin, 1.0); }
	
	Kaiser70Sinc(unsigned div, RollOff ro, double gain)
	: kernel(ro.taps * phases), convoluter_(kernel, ro.taps, div)
	{ makeSincKernel(kernel, phases, ro.taps, ro.fc, kaiser70SincWin, gain);}

	std::size_t resample(short *out, const short *in, std::size_t inlen) { return convoluter_.filter(out, in, inlen); }
	void adjustDiv(unsigned div) { convoluter_.adjustDiv(div); }
	unsigned mul() const { return MUL; }
	unsigned div() const { return convoluter_.div(); }
};

#endif
