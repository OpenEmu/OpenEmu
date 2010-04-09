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
#ifndef LININT_H
#define LININT_H

#include <cstddef>
#include "resampler.h"
#include "u48div.h"

template<unsigned channels>
class LinintCore {
	unsigned long ratio;
	std::size_t pos_;
	unsigned fracPos_;
	int prevSample_;
	
public:
	LinintCore(long inRate = 1, long outRate = 1) { init(inRate, outRate); }
	void adjustRate(long inRate, long outRate) { ratio = (static_cast<double>(inRate) / outRate) * 0x10000 + 0.5; }
	void exactRatio(unsigned long &mul, unsigned long &div) const { mul = 0x10000; div = ratio; }
	void init(long inRate, long outRate);
	std::size_t maxOut(std::size_t inlen) const { return inlen ? u48div(inlen - 1, 0xFFFF, ratio) + 1 : 0; }
	std::size_t resample(short *out, const short *in, std::size_t inlen);
};

template<const unsigned channels>
void LinintCore<channels>::init(const long inRate, const long outRate) {
	adjustRate(inRate, outRate);
	pos_ = (ratio >> 16) + 1;
	fracPos_ = ratio & 0xFFFF;
	prevSample_ = 0;
}

template<const unsigned channels>
std::size_t LinintCore<channels>::resample(short *const out, const short *const in, const std::size_t inlen) {
	std::size_t opos = 0;
	std::size_t pos = pos_;
	unsigned fracPos = fracPos_;
	int prevSample = prevSample_;

	if (pos < inlen) {
		if (pos != 0)
			prevSample = in[(pos-1) * channels];
	
		for (;;) {
			out[opos] = prevSample + (in[pos * channels] - prevSample) * static_cast<long>(fracPos) / 0x10000;
			opos += channels;
			
			{
				const unsigned long next = ratio + fracPos;
				
				pos += next >> 16;
				fracPos = next & 0xFFFF;
			}
			
			if (pos < inlen) {
				prevSample = in[(pos-1) * channels];
			} else
				break;
		}
		
		if (pos == inlen)
			prevSample = in[(pos-1) * channels];
	}
	
// 	const std::size_t produced = ((pos - pos_) * 0x10000 + fracPos - fracPos_) / ratio;
	
	pos_ = pos - inlen;
	fracPos_ = fracPos;
	prevSample_ = prevSample;
	
	return opos / channels;
}

template<unsigned channels>
class Linint : public Resampler {
	LinintCore<channels> cores[channels];
	
public:
	Linint(long inRate, long outRate);
	void adjustRate(long inRate, long outRate);
	void exactRatio(unsigned long &mul, unsigned long &div) const { cores[0].exactRatio(mul, div); }
	std::size_t maxOut(std::size_t inlen) const { return cores[0].maxOut(inlen); }
	std::size_t resample(short *out, const short *in, std::size_t inlen);
};

template<const unsigned channels>
Linint<channels>::Linint(const long inRate, const long outRate) {
	setRate(inRate, outRate);
	
	for (unsigned i = 0; i < channels; ++i)
		cores[i].init(inRate, outRate);
}

template<const unsigned channels>
void Linint<channels>::adjustRate(const long inRate, const long outRate) {
	setRate(inRate, outRate);
	
	for (unsigned i = 0; i < channels; ++i)
		cores[i].adjustRate(inRate, outRate);
}

template<const unsigned channels>
std::size_t Linint<channels>::resample(short *const out, const short *const in, const std::size_t inlen) {
	std::size_t outlen = 0;
	
	for (unsigned i = 0; i < channels; ++i)
		outlen = cores[i].resample(out + i, in + i, inlen);
	
	return outlen;
}

#endif
