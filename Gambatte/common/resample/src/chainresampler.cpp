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

void ChainResampler::downinitAddSincResamplers(double ratio, float const outRate,
		CreateSinc const createBigSinc, CreateSinc const createSmallSinc,
		unsigned const bigSincMul, unsigned const smallSincMul, double gain) {
	// For high outRate: Start roll-off at 36000 Hz continue until outRate Hz, then wrap around back down to 40000 Hz.
	const float outPeriod = 1.0f / outRate;
	const float finalRollOffLen = std::max((outRate - 36000.0f + outRate - 40000.0f) * outPeriod, 0.2f);
	
	{
		const float midRollOffStart = std::min(36000.0f * outPeriod, 1.0f);
		const float midRollOffEnd   = std::min(40000.0f * outPeriod, 1.0f); // after wrap at folding freq.
		const float midRollOffStartPlusEnd = midRollOffStart + midRollOffEnd;
		
		int div_2c = static_cast<int>(ratio * smallSincMul / get2ChainMidRatio(ratio, finalRollOffLen, midRollOffStartPlusEnd) + 0.5f);
		double ratio_2c = ratio * smallSincMul / div_2c;
		float cost_2c = get2ChainCost(ratio, finalRollOffLen, ratio_2c, midRollOffStartPlusEnd);
		
		if (cost_2c < get1ChainCost(ratio, finalRollOffLen)) {
			const int div1_3c = static_cast<int>(
					ratio * smallSincMul / get3ChainRatio1(ratio_2c, finalRollOffLen, ratio, midRollOffStartPlusEnd) + 0.5f);
			const double ratio1_3c = ratio * smallSincMul / div1_3c;
			const int div2_3c = static_cast<int>(
					ratio1_3c * smallSincMul / get3ChainRatio2(ratio1_3c, finalRollOffLen, midRollOffStartPlusEnd) + 0.5f);
			const double ratio2_3c = ratio1_3c * smallSincMul / div2_3c;
			
			if (get3ChainCost(ratio, finalRollOffLen, ratio1_3c, ratio2_3c, midRollOffStartPlusEnd) < cost_2c) {
				list.push_back(createSmallSinc(div1_3c, 0.5f * midRollOffStart / ratio,
						(ratio1_3c - 0.5f * midRollOffStartPlusEnd) / ratio, gain));
				ratio = ratio1_3c;
				div_2c = div2_3c;
				ratio_2c = ratio2_3c;
				gain = 1.0;
			}
			
			list.push_back(createSmallSinc(div_2c, 0.5f * midRollOffStart / ratio,
					(ratio_2c - 0.5f * midRollOffStartPlusEnd) / ratio, gain));
			ratio = ratio_2c;
			gain = 1.0;
		}
	}
	
	list.push_back(bigSinc =
		createBigSinc(static_cast<int>(bigSincMul * ratio + 0.5),
			0.5f * (1.0f + std::max((outRate - 40000.0f) * outPeriod, 0.0f) - finalRollOffLen) / ratio,
			0.5f * finalRollOffLen / ratio, gain));
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
