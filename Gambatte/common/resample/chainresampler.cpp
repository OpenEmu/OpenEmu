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
#include "chainresampler.h"

float ChainResampler::get2ChainMidRatio(const float ratio, const float rollOff) {
	return std::sqrt(0.5f * rollOff * ratio) + 1;
}

float ChainResampler::get2ChainCost(const float ratio, const float rollOff, const float midRatio) {
	return midRatio * ratio / ((midRatio - 1) * 2) + midRatio / rollOff;
}

float ChainResampler::get3ChainRatio1(float ratio1, const float rollOff, const float ratio) {
	for (unsigned n = 8; n--;) {
		ratio1 = std::sqrt(ratio - ratio / get3ChainRatio2(ratio1, rollOff)) + 1;
	}

	return ratio1;
}

float ChainResampler::get3ChainCost(const float ratio, const float rollOff, const float ratio1, const float ratio2) {
	return ratio1 * ratio / ((ratio1 - 1) * 2) + ratio2 * ratio1 / ((ratio2 - 1) * 2) + ratio2 / rollOff;
}

std::size_t ChainResampler::reallocateBuffer() {
	std::size_t bufSz[2] = { 0, 0 };
	std::size_t inSz = periodSize;
	int i = -1;
	
	for (list_t::iterator it = list.begin(); it != list.end(); ++it) {
		inSz = (inSz * (*it)->mul() - 1) / (*it)->div() + 1;
		
		++i;
		
		if (inSz > bufSz[i&1])
			bufSz[i&1] = inSz;
	}
	
	if (inSz >= bufSz[i&1])
		bufSz[i&1] = 0;
	
	if (bufferSize < bufSz[0] + bufSz[1]) {
		delete[] buffer;
		buffer = (bufferSize = bufSz[0] + bufSz[1]) ? new short[bufferSize * channels] : NULL;
	}
	
	buffer2 = bufSz[1] ? buffer + bufSz[0] * channels : NULL;
	
	return (maxOut_ = inSz);
}

void ChainResampler::adjustRate(const long inRate, const long outRate) {
	unsigned long mul, div;
	
	exactRatio(mul, div);
	
	bigSinc->adjustDiv(static_cast<double>(inRate) * mul / (static_cast<double>(div / bigSinc->div()) * outRate) + 0.5);
	
	reallocateBuffer();
	setRate(inRate, outRate);
}

void ChainResampler::exactRatio(unsigned long &mul, unsigned long &div) const {
	mul = 1;
	div = 1;
	
	for (list_t::const_iterator it = list.begin(); it != list.end(); ++it) {
		mul *= (*it)->mul();
		div *= (*it)->div();
	}
}

std::size_t ChainResampler::resample(short *const out, const short *const in, std::size_t inlen) {
	assert(inlen <= periodSize);
	
	short *const buf = buffer != buffer2 ? buffer : out;
	short *const buf2 = buffer2 ? buffer2 : out;
	
	const short *inbuf = in;
	short *outbuf = NULL;
	
	for (list_t::iterator it = list.begin(); it != list.end(); ++it) {
		outbuf = ++list_t::iterator(it) == list.end() ? out : (inbuf == buf ? buf2 : buf);
		inlen = (*it)->resample(outbuf, inbuf, inlen);
		inbuf = outbuf;
	}
	
	return inlen;
}

void ChainResampler::uninit() {
	delete[] buffer;
	buffer2 = buffer = NULL;
	bufferSize = 0;
	periodSize = 0;
	bigSinc = NULL;
	
	for (list_t::iterator it = list.begin(); it != list.end(); ++it)
		delete *it;
	
	list.clear();
}
