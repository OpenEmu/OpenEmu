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
#include "../resamplerinfo.h"
#include "chainresampler.h"
#include "kaiser50sinc.h"
#include "kaiser70sinc.h"
// #include "hammingsinc.h"
// #include "blackmansinc.h"
#include "rectsinc.h"
#include "linint.h"

struct LinintInfo {
	static Resampler* create(long inRate, long outRate, std::size_t) { return new Linint<ChainResampler::channels>(inRate, outRate); }
};

template<template<unsigned,unsigned> class T>
struct ChainSincInfo {
	static Resampler* create(long inRate, long outRate, std::size_t periodSz) {
		ChainResampler *r = new ChainResampler;
		r->init<T>(inRate, outRate, periodSz);
		return r;
	}
};

const ResamplerInfo ResamplerInfo::resamplers[] = {
	{ "Fast", LinintInfo::create },
	{ "High quality (CIC + sinc chain)", ChainSincInfo<RectSinc>::create },
// 	{ "Hamming windowed sinc (~50 dB SNR)", ChainSincInfo<HammingSinc>::create },
// 	{ "Blackman windowed sinc (~70 dB SNR)", ChainSincInfo<BlackmanSinc>::create },
	{ "Very high quality (CIC + sinc chain)", ChainSincInfo<Kaiser50Sinc>::create },
	{ "Highest quality (CIC + sinc chain)", ChainSincInfo<Kaiser70Sinc>::create }
};

const std::size_t ResamplerInfo::num_ = sizeof ResamplerInfo::resamplers / sizeof *ResamplerInfo::resamplers;
