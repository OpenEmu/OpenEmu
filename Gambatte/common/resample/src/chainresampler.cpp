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

float ChainResampler::get2ChainMidRatio(const float ratio, const float finalRollOffLen, const float midRollOffStartPlusEnd) {
	return 0.5f * (std::sqrt(ratio * midRollOffStartPlusEnd * finalRollOffLen) + midRollOffStartPlusEnd);
}

float ChainResampler::get2ChainCost(const float ratio, const float finalRollOffLen, const float midRatio, const float midRollOffStartPlusEnd) {
	const float midRollOffLen = midRatio * 2 - midRollOffStartPlusEnd;
	return midRatio * ratio / midRollOffLen + get1ChainCost(midRatio, finalRollOffLen);
}

float ChainResampler::get3ChainRatio1(float ratio1, const float finalRollOffLen, const float ratio, const float midRollOffStartPlusEnd) {
	for (unsigned n = 8; n--;) {
		const float ratio2 = get3ChainRatio2(ratio1, finalRollOffLen, midRollOffStartPlusEnd);
		ratio1 = 0.5f * (std::sqrt(ratio * midRollOffStartPlusEnd * (2 - midRollOffStartPlusEnd / ratio2)) + midRollOffStartPlusEnd);
	}

	return ratio1;
}

float ChainResampler::get3ChainCost(const float ratio, const float finalRollOffLen,
		const float ratio1, const float ratio2, const float midRollOffStartPlusEnd) {
	const float firstRollOffLen  = ratio1 * 2 - midRollOffStartPlusEnd;
	return ratio1 * ratio / firstRollOffLen + get2ChainCost(ratio1, finalRollOffLen, ratio2, midRollOffStartPlusEnd);
}

ChainResampler::ChainResampler()
: bigSinc(0), buffer2(0), periodSize(0)
{
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
	
	if (buffer.size() < (bufSz[0] + bufSz[1]) * channels)
		buffer.reset((bufSz[0] + bufSz[1]) * channels);
	
	buffer2 = bufSz[1] ? buffer + bufSz[0] * channels : 0;
	
	return (maxOut_ = inSz);
}

void ChainResampler::adjustRate(const long inRate, const long outRate) {
	unsigned long mul, div;
	
	exactRatio(mul, div);
	
	bigSinc->adjustDiv(static_cast<int>(static_cast<double>(inRate) * mul / (static_cast<double>(div / bigSinc->div()) * outRate) + 0.5));
	
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
	short *outbuf = 0;
	
	for (list_t::iterator it = list.begin(); it != list.end(); ++it) {
		outbuf = ++list_t::iterator(it) == list.end() ? out : (inbuf == buf ? buf2 : buf);
		inlen = (*it)->resample(outbuf, inbuf, inlen);
		inbuf = outbuf;
	}
	
	return inlen;
}

void ChainResampler::uninit() {
	buffer2 = 0;
	buffer.reset();
	periodSize = 0;
	bigSinc = 0;
	
	for (list_t::iterator it = list.begin(); it != list.end(); ++it)
		delete *it;
	
	list.clear();
}
