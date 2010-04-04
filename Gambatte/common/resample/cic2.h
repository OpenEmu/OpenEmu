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
#ifndef CIC2_H
#define CIC2_H

#include "subresampler.h"

template<unsigned channels> 
class Cic2Core {
// 	enum { BUFLEN = 64 };
// 	unsigned long buf[BUFLEN];
	unsigned long sum1;
	unsigned long sum2;
	unsigned long prev1;
	unsigned long prev2;
	unsigned div_;
	unsigned nextdivn;
// 	unsigned bufpos;
	
public:
	Cic2Core(const unsigned div = 2) {
		reset(div);
	}
	
	unsigned div() const { return div_; }
	std::size_t filter(short *out, const short *in, std::size_t inlen);
	void reset(unsigned div);
};

template<const unsigned channels> 
void Cic2Core<channels>::reset(const unsigned div) {
	sum2 = sum1 = 0;
	prev2 = prev1 = 0;
	this->div_ = div;
	nextdivn = div;
// 	bufpos = div - 1;
}

template<const unsigned channels>
std::size_t Cic2Core<channels>::filter(short *out, const short *const in, std::size_t inlen) {
// 	const std::size_t produced = (inlen + div_ - (bufpos + 1)) / div_;
	const std::size_t produced = (inlen + div_ - nextdivn) / div_;
	const long mul = 0x10000 / (div_ * div_); // trouble if div is too large, may be better to only support power of 2 div
	const short *s = in;
	
	/*unsigned long sm1 = sum1;
	unsigned long sm2 = sum2;
	
	while (inlen >> 2) {
		unsigned n = (inlen < BUFLEN ? inlen >> 2 : BUFLEN >> 2);
		const unsigned end = n * 4;
		unsigned i = 0;
		
		do {
			unsigned long s1 = sm1 += static_cast<long>(*s);
			s += channels;
			sm1 += static_cast<long>(*s);
			s += channels;
			buf[i++] = sm2 += s1;
			buf[i++] = sm2 += sm1;
			s1 = sm1 += static_cast<long>(*s);
			s += channels;
			sm1 += static_cast<long>(*s);
			s += channels;
			buf[i++] = sm2 += s1;
			buf[i++] = sm2 += sm1;
		} while (--n);
		
		while (bufpos < end) {
			const unsigned long out2 = buf[bufpos] - prev2;
			prev2 = buf[bufpos];
			bufpos += div_;
			
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
			buf[i++] = sm2 += sm1;
		} while (--n);
		
		while (bufpos < inlen) {
			const unsigned long out2 = buf[bufpos] - prev2;
			prev2 = buf[bufpos];
			bufpos += div_;
			
			*out = static_cast<long>(out2 - prev1) * mul / 0x10000;
			prev1 = out2;
			out += channels;
		}
		
		bufpos -= inlen;
	}
	
	sum1 = sm1;
	sum2 = sm2;*/
	
	unsigned long sm1 = sum1;
	unsigned long sm2 = sum2;
	
	if (inlen >= nextdivn) {
		unsigned divn = nextdivn;
		std::size_t n = produced;
		
		do {
			do {
				sm1 += static_cast<long>(*s);
				s += channels;
				sm2 += sm1;
			} while (--divn);
			
			const unsigned long out2 = sm2 - prev2;
			prev2 = sm2;
			
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
		}
	}
	
	sum1 = sm1;
	sum2 = sm2;
	
	return produced;
}

template<unsigned channels>
class Cic2 : public SubResampler {
	Cic2Core<channels> cics[channels];
	
public:
	enum { MAX_DIV = 64 };
	Cic2(unsigned div);
	std::size_t resample(short *out, const short *in, std::size_t inlen);
	unsigned mul() const { return 1; }
	unsigned div() const { return cics[0].div(); }
};

template<const unsigned channels>
Cic2<channels>::Cic2(const unsigned div) {
	for (unsigned i = 0; i < channels; ++i)
		cics[i].reset(div);
}

template<const unsigned channels>
std::size_t Cic2<channels>::resample(short *const out, const short *const in, const std::size_t inlen) {
	std::size_t samplesOut;
	
	for (unsigned i = 0; i < channels; ++i) {
		samplesOut = cics[i].filter(out + i, in + i, inlen);
	}
	
	return samplesOut;
}

#endif
