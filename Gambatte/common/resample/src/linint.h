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
#ifndef LININT_H
#define LININT_H

#include <cstddef>
#include "../resampler.h"
#include "u48div.h"
#include "rshift16_round.h"

template<int channels>
class LinintCore {
	unsigned long ratio_;
	std::size_t pos_;
	unsigned fracPos_;
	int prevSample_;
	
public:
	explicit LinintCore(long inRate = 1, long outRate = 1) { init(inRate, outRate); }

	void adjustRate(long inRate, long outRate) {
		ratio_ = static_cast<unsigned long>((static_cast<double>(inRate) / outRate) * 0x10000 + 0.5);
	}

	void exactRatio(unsigned long &mul, unsigned long &div) const { mul = 0x10000; div = ratio_; }
	void init(long inRate, long outRate);
	std::size_t maxOut(std::size_t inlen) const { return inlen ? u48div(inlen - 1, 0xFFFF, ratio_) + 1 : 0; }
	std::size_t resample(short *out, const short *in, std::size_t inlen);
};

template<int channels>
void LinintCore<channels>::init(const long inRate, const long outRate) {
	adjustRate(inRate, outRate);
	pos_ = (ratio_ >> 16) + 1;
	fracPos_ = ratio_ & 0xFFFF;
	prevSample_ = 0;
}

template<int channels>
std::size_t LinintCore<channels>::resample(short *const out, const short *const in, const std::size_t inlen) {
	if (pos_ < inlen) {
		std::ptrdiff_t pos = pos_;
		const unsigned long ratio = ratio_;
		unsigned fracPos = fracPos_;
		short *o = out;

		while (pos == 0) {
			long const lhs = prevSample_;
			long const rhs = in[0];
			*o = lhs + rshift16_round((rhs - lhs) * static_cast<long>(fracPos));
			o += channels;

			unsigned long const nfrac = fracPos + ratio;
			fracPos = nfrac & 0xFFFF;
			pos    += nfrac >> 16;
		}

		const short *const inend = in + inlen * channels;
		pos -= static_cast<std::ptrdiff_t>(inlen);

		while (pos < 0) {
			long const lhs = inend[(pos-1) * channels];
			long const rhs = inend[ pos    * channels];
			*o = lhs + rshift16_round((rhs - lhs) * static_cast<long>(fracPos));
			o += channels;

			unsigned long const nfrac = fracPos + ratio;
			fracPos = nfrac & 0xFFFF;
			pos    += nfrac >> 16;
		}

		prevSample_ = inend[-channels];
		pos_ = pos;
		fracPos_ = fracPos;

		return (o - out) / channels;
	}

	return 0;
}

template<int channels>
class Linint : public Resampler {
	LinintCore<channels> cores[channels];
	
public:
	Linint(long inRate, long outRate);
	void adjustRate(long inRate, long outRate);
	void exactRatio(unsigned long &mul, unsigned long &div) const { cores[0].exactRatio(mul, div); }
	std::size_t maxOut(std::size_t inlen) const { return cores[0].maxOut(inlen); }
	std::size_t resample(short *out, const short *in, std::size_t inlen);
};

template<int channels>
Linint<channels>::Linint(const long inRate, const long outRate) {
	setRate(inRate, outRate);
	
	for (int i = 0; i < channels; ++i)
		cores[i].init(inRate, outRate);
}

template<int channels>
void Linint<channels>::adjustRate(const long inRate, const long outRate) {
	setRate(inRate, outRate);
	
	for (int i = 0; i < channels; ++i)
		cores[i].adjustRate(inRate, outRate);
}

template<int channels>
std::size_t Linint<channels>::resample(short *const out, const short *const in, const std::size_t inlen) {
	std::size_t outlen = 0;
	
	for (int i = 0; i < channels; ++i)
		outlen = cores[i].resample(out + i, in + i, inlen);
	
	return outlen;
}

#endif
