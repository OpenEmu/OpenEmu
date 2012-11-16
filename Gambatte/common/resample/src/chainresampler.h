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
#ifndef CHAINRESAMPLER_H
#define CHAINRESAMPLER_H

#include <cmath>
#include <cstdlib>
#include <cassert>
#include <cstddef>
#include <list>
#include "array.h"
#include "subresampler.h"
#include "../resampler.h"
#include "upsampler.h"

class ChainResampler : public Resampler {
	typedef std::list<SubResampler*> list_t;
	
	list_t list;
	SubResampler *bigSinc;
	Array<short> buffer;
	short *buffer2;
	std::size_t periodSize;
	std::size_t maxOut_;
	
	static float get1ChainCost(float ratio, float finalRollOffLen) { return ratio / finalRollOffLen; }

	static float get2ChainMidRatio(float ratio, float finalRollOffLen, float midRollOffStartPlusEnd);
	static float get2ChainCost(float ratio, float finalRollOffLen, float midRatio, float midRollOffStartPlusEnd);

	static float get3ChainRatio2(float ratio1, float finalRollOffLen, float midRollOffStartPlusEnd) {
		return get2ChainMidRatio(ratio1, finalRollOffLen, midRollOffStartPlusEnd);
	}

	static float get3ChainRatio1(float ratio1, float finalRollOffLen, float ratio, float midRollOffStartPlusEnd);
	static float get3ChainCost(float ratio, float finalRollOffLen, float ratio1, float ratio2, float midRollOffStartPlusEnd);
	
	template<template<unsigned,unsigned> class Sinc>
	std::size_t downinit(long inRate, long outRate, std::size_t periodSize);
	
	template<template<unsigned,unsigned> class Sinc>
	std::size_t upinit(long inRate, long outRate, std::size_t periodSize);
	
	std::size_t reallocateBuffer();
	
public:
	enum { channels = 2 };
	ChainResampler();
	~ChainResampler() { uninit(); }
	
	void adjustRate(long inRate, long outRate);
	void exactRatio(unsigned long &mul, unsigned long &div) const;
	
	template<template<unsigned,unsigned> class Sinc>
	std::size_t init(long inRate, long outRate, std::size_t periodSize);
	std::size_t maxOut(std::size_t /*inlen*/) const { return maxOut_; }
	std::size_t resample(short *out, const short *in, std::size_t inlen);
	void uninit();
};

template<template<unsigned,unsigned> class Sinc>
std::size_t ChainResampler::init(const long inRate, const long outRate, const std::size_t periodSize) {
	setRate(inRate, outRate);
	
	if (outRate > inRate)
		return upinit<Sinc>(inRate, outRate, periodSize);
	else
		return downinit<Sinc>(inRate, outRate, periodSize);
}

template<template<unsigned,unsigned> class Sinc>
std::size_t ChainResampler::downinit(const long inRate, const long outRate, const std::size_t periodSize) {
	typedef Sinc<channels,2048> BigSinc;
	typedef Sinc<channels,32> SmallSinc;
	
	uninit();
	this->periodSize = periodSize;
	
	double ratio = static_cast<double>(inRate) / outRate;
	
	while (ratio >= BigSinc::cicLimit() * 2) {
		const int div = std::min<int>(static_cast<int>(ratio / BigSinc::cicLimit()), BigSinc::Cic::MAX_DIV);
		
		list.push_back(new typename BigSinc::Cic(div));
		ratio /= div;
	}
	
	// For high outRate: Start roll-off at 36000 Hz continue until outRate Hz, then wrap around back down to 40000 Hz.
	const float outPeriod = 1.0f / outRate;
	const float finalRollOffLen = std::max((outRate - 36000.0f + outRate - 40000.0f) * outPeriod, 0.2f);
	
	{
		const float midRollOffStart = std::min(36000.0f * outPeriod, 1.0f);
		const float midRollOffEnd   = std::min(40000.0f * outPeriod, 1.0f); // after wrap at folding freq.
		const float midRollOffStartPlusEnd = midRollOffStart + midRollOffEnd;
		
		int div_2c = static_cast<int>(ratio * SmallSinc::MUL / get2ChainMidRatio(ratio, finalRollOffLen, midRollOffStartPlusEnd) + 0.5f);
		double ratio_2c = ratio * SmallSinc::MUL / div_2c;
		float cost_2c = get2ChainCost(ratio, finalRollOffLen, ratio_2c, midRollOffStartPlusEnd);
		
		if (cost_2c < get1ChainCost(ratio, finalRollOffLen)) {
			const int div1_3c = static_cast<int>(
					ratio * SmallSinc::MUL / get3ChainRatio1(ratio_2c, finalRollOffLen, ratio, midRollOffStartPlusEnd) + 0.5f);
			const double ratio1_3c = ratio * SmallSinc::MUL / div1_3c;
			const int div2_3c = static_cast<int>(
					ratio1_3c * SmallSinc::MUL / get3ChainRatio2(ratio1_3c, finalRollOffLen, midRollOffStartPlusEnd) + 0.5f);
			const double ratio2_3c = ratio1_3c * SmallSinc::MUL / div2_3c;
			
			if (get3ChainCost(ratio, finalRollOffLen, ratio1_3c, ratio2_3c, midRollOffStartPlusEnd) < cost_2c) {
				list.push_back(new SmallSinc(div1_3c, typename SmallSinc::RollOff(
						0.5f * midRollOffStart / ratio, (ratio1_3c - 0.5f * midRollOffStartPlusEnd) / ratio)));
				ratio = ratio1_3c;
				div_2c = div2_3c;
				ratio_2c = ratio2_3c;
			}
			
			list.push_back(new SmallSinc(div_2c, typename SmallSinc::RollOff(
					0.5f * midRollOffStart / ratio, (ratio_2c - 0.5f * midRollOffStartPlusEnd) / ratio)));
			ratio = ratio_2c;
		}
	}
	
	list.push_back(bigSinc = new BigSinc(static_cast<int>(BigSinc::MUL * ratio + 0.5), typename BigSinc::RollOff(
			0.5f * (1.0f + std::max((outRate - 40000.0f) * outPeriod, 0.0f) - finalRollOffLen) / ratio, 0.5f * finalRollOffLen / ratio)));
	
	return reallocateBuffer();
}

template<template<unsigned,unsigned> class Sinc>
std::size_t ChainResampler::upinit(const long inRate, const long outRate, const std::size_t periodSize) {
	typedef Sinc<channels,2048> BigSinc;
	typedef Sinc<channels,32> SmallSinc;

	uninit();
	this->periodSize = periodSize;
	
	double ratio = static_cast<double>(outRate) / inRate;
	
	// Spectral images above 20 kHz assumed inaudible
	{
		const int div = outRate / std::max(inRate, 40000l);
		
		if (div >= 2) {
			list.push_front(new Upsampler<channels>(div));
			ratio /= div;
		}
	}
	
	const float rollOff = std::max((inRate - 36000.0f) / inRate, 0.2f);
	
	/*{
		int div_2c = get2ChainMidRatio(ratio, rollOff) * SmallSinc::MUL / ratio + 0.5f;
		double ratio_2c = ratio * div_2c / SmallSinc::MUL;
		float cost_2c = get2ChainCost(ratio, rollOff, ratio_2c);
		
		if (cost_2c < get1ChainCost(ratio, rollOff)) {
			const int div1_3c = get3ChainRatio1(ratio_2c, rollOff, ratio) * SmallSinc::MUL / ratio + 0.5f;
			const double ratio1_3c = ratio * div1_3c / SmallSinc::MUL;
			const int div2_3c = get3ChainRatio2(ratio1_3c, rollOff) * SmallSinc::MUL / ratio1_3c + 0.5f;
			const double ratio2_3c = ratio1_3c * div2_3c / SmallSinc::MUL;
			
			if (get3ChainCost(ratio, rollOff, ratio1_3c, ratio2_3c) < cost_2c) {
				list.push_front(new SmallSinc(div1_3c, typename SmallSinc::RollOff(0.5f / ratio1_3c, (ratio1_3c - 1) / ratio1_3c)));
				ratio = ratio1_3c;
				div_2c = div2_3c;
				ratio_2c = ratio2_3c;
			}
			
			list.push_front(new SmallSinc(div_2c, typename SmallSinc::RollOff(0.5f / ratio_2c, (ratio_2c - 1) / ratio_2c)));
			ratio = ratio_2c;
		}
	}*/
	
	list.push_front(bigSinc = new BigSinc(
			static_cast<int>(BigSinc::MUL / ratio + 0.5), typename BigSinc::RollOff(0.5f * (1 - rollOff), 0.5f * rollOff)));
	
	return reallocateBuffer();
}

#endif
