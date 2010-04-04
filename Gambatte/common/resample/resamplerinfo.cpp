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
#include "resamplerinfo.h"
#include "chainresampler.h"
#include "hammingsinc.h"
#include "blackmansinc.h"
#include "rectsinc.h"
#include "linint.h"

struct LinintInfo {
	static Resampler* create(long inRate, long outRate, std::size_t) { return new Linint<2>(inRate, outRate); }
};

struct RectsincInfo {
	static Resampler* create(long inRate, long outRate, std::size_t periodSz) {
		ChainResampler *r = new ChainResampler;
		r->init<RectSinc>(inRate, outRate, periodSz);
		return r;
	}
};

struct HammingsincInfo {
	static Resampler* create(long inRate, long outRate, std::size_t periodSz) {
		ChainResampler *r = new ChainResampler;
		r->init<HammingSinc>(inRate, outRate, periodSz);
		return r;
	}
};

struct BlackmansincInfo {
	static Resampler* create(long inRate, long outRate, std::size_t periodSz) {
		ChainResampler *r = new ChainResampler;
		r->init<BlackmanSinc>(inRate, outRate, periodSz);
		return r;
	}
};

const ResamplerInfo ResamplerInfo::resamplers[] = {
	{ "2-tap linear interpolation", LinintInfo::create },
	{ "Rectangular windowed sinc (~20 dB SNR)", RectsincInfo::create },
	{ "Hamming windowed sinc (~50 dB SNR)", HammingsincInfo::create },
	{ "Blackman windowed sinc (~70 dB SNR)", BlackmansincInfo::create }
};

const unsigned ResamplerInfo::num_ = sizeof(ResamplerInfo::resamplers) / sizeof(ResamplerInfo);
