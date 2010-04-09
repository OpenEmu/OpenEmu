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
#ifndef CIC4_H
#define CIC4_H

#include "subresampler.h"

template<unsigned channels> 
class Cic4Core {
	enum { BUFLEN = 64 };
	unsigned long buf[BUFLEN];
	unsigned long sum1;
	unsigned long sum2;
	unsigned long sum3;
	unsigned long sum4;
	unsigned long prev1;
	unsigned long prev2;
	unsigned long prev3;
	unsigned long prev4;
	unsigned div_;
// 	unsigned nextdivn;
	unsigned bufpos;
	
public:
	Cic4Core(const unsigned div = 1) {
		reset(div);
	}
	
	unsigned div() const { return div_; }
	std::size_t filter(short *out, const short *in, std::size_t inlen);
	void reset(unsigned div);
};

template<const unsigned channels> 
void Cic4Core<channels>::reset(const unsigned div) {
	sum4 = sum3 = sum2 = sum1 = 0;
	prev4 = prev3 = prev2 = prev1 = 0;
	this->div_ = div;
// 	nextdivn = div;
	bufpos = div - 1;
}

template<const unsigned channels>
std::size_t Cic4Core<channels>::filter(short *out, const short *const in, std::size_t inlen) {
	const std::size_t produced = (inlen + div_ - (bufpos + 1)) / div_;
// 	const std::size_t produced = (inlen + div_ - nextdivn) / div_;
	const long mul = 0x10000 / (div_ * div_ * div_ * div_); // trouble if div is too large, may be better to only support power of 2 div
	const short *s = in;
	
	unsigned long sm1 = sum1;
	unsigned long sm2 = sum2;
	unsigned long sm3 = sum3;
	unsigned long sm4 = sum4;
	
	while (inlen >> 2) {
		unsigned n = (inlen < BUFLEN ? inlen >> 2 : BUFLEN >> 2);
		const unsigned end = n * 4;
		unsigned i = 0;
		
		do {
			unsigned long s1 = sm1 += static_cast<long>(*s);
			s += channels;
			sm1 += static_cast<long>(*s);
			s += channels;
			unsigned long s2 = sm2 += s1;
			sm2 += sm1;
			unsigned long s3 = sm3 += s2;
			sm3 += sm2;
			buf[i++] = sm4 += s3;
			buf[i++] = sm4 += sm3;
			s1 = sm1 += static_cast<long>(*s);
			s += channels;
			sm1 += static_cast<long>(*s);
			s += channels;
			s2 = sm2 += s1;
			sm2 += sm1;
			s3 = sm3 += s2;
			sm3 += sm2;
			buf[i++] = sm4 += s3;
			buf[i++] = sm4 += sm3;
		} while (--n);
		
		while (bufpos < end) {
			const unsigned long out4 = buf[bufpos] - prev4;
			prev4 = buf[bufpos];
			bufpos += div_;
			
			const unsigned long out3 = out4 - prev3;
			prev3 = out4;
			const unsigned long out2 = out3 - prev2;
			prev2 = out3;
			
			*out = static_cast<long>(out2 - prev1) * mul / 0x10000;
			prev1 = out2;
			out += channels;
		}
		
		bufpos -= end;
		inlen -= end;
	}
	
	if (inlen) {
		unsigned n = inlen;
		unsigned i = 0;
		
		do {
			sm1 += static_cast<long>(*s);
			s += channels;
			sm2 += sm1;
			sm3 += sm2;
			buf[i++] = sm4 += sm3;
		} while (--n);
		
		while (bufpos < inlen) {
			const unsigned long out4 = buf[bufpos] - prev4;
			prev4 = buf[bufpos];
			bufpos += div_;
			
			const unsigned long out3 = out4 - prev3;
			prev3 = out4;
			const unsigned long out2 = out3 - prev2;
			prev2 = out3;
			
			*out = static_cast<long>(out2 - prev1) * mul / 0x10000;
			prev1 = out2;
			out += channels;
		}
		
		bufpos -= inlen;
	}
	
	sum1 = sm1;
	sum2 = sm2;
	sum3 = sm3;
	sum4 = sm4;
	
	/*unsigned long sm1 = sum1;
	unsigned long sm2 = sum2;
	unsigned long sm3 = sum3;
	unsigned long sm4 = sum4;
	
	if (produced) {
		unsigned divn = nextdivn;
		std::size_t n = produced;
		
		do {
			do {
				sm1 += static_cast<long>(*s);
				s += channels;
				sm2 += sm1;
				sm3 += sm2;
				sm4 += sm3;
			} while (--divn);
			
			const unsigned long out4 = sm4 - prev4;
			prev4 = sm4;
			const unsigned long out3 = out4 - prev3;
			prev3 = out4;
			const unsigned long out2 = out3 - prev2;
			prev2 = out3;
			*out = static_cast<long>(out2 - prev1) * mul / 0x10000;
			prev1 = out2;
			out += channels;
			
			divn = div_;
		} while (--n);
		
		nextdivn = div_;
	}
	
	{
		unsigned divn = (in + inlen * channels - s) / channels;
		nextdivn -= divn;
		
		while (divn--) {
			sm1 += static_cast<long>(*s);
			s += channels;
			sm2 += sm1;
			sm3 += sm2;
			sm4 += sm3;
		}
	}
	
	sum1 = sm1;
	sum2 = sm2;
	sum3 = sm3;
	sum4 = sm4;*/
	
	return produced;
}

template<unsigned channels>
class Cic4 : public SubResampler {
	Cic4Core<channels> cics[channels];
	
public:
	enum { MAX_DIV = 13 };
	Cic4(unsigned div);
	std::size_t resample(short *out, const short *in, std::size_t inlen);
	unsigned mul() const { return 1; }
	unsigned div() const { return cics[0].div(); }
};

template<const unsigned channels>
Cic4<channels>::Cic4(const unsigned div) {
	for (unsigned i = 0; i < channels; ++i)
		cics[i].reset(div);
}

template<const unsigned channels>
std::size_t Cic4<channels>::resample(short *const out, const short *const in, const std::size_t inlen) {
	std::size_t samplesOut;
	
	for (unsigned i = 0; i < channels; ++i) {
		samplesOut = cics[i].filter(out + i, in + i, inlen);
	}
	
	return samplesOut;
}

#endif
