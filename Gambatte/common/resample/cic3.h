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
#ifndef CIC3_H
#define CIC3_H

#include "subresampler.h"

template<unsigned channels> 
class Cic3Core {
// 	enum { BUFLEN = 64 };
// 	unsigned long buf[BUFLEN];
	unsigned long sum1;
	unsigned long sum2;
	unsigned long sum3;
	unsigned long prev1;
	unsigned long prev2;
	unsigned long prev3;
	unsigned div_;
	unsigned nextdivn;
// 	unsigned bufpos;
	
public:
	Cic3Core(const unsigned div = 1) {
		reset(div);
	}
	
	unsigned div() const { return div_; }
	std::size_t filter(short *out, const short *in, std::size_t inlen);
	void reset(unsigned div);
};

template<const unsigned channels> 
void Cic3Core<channels>::reset(const unsigned div) {
	sum3 = sum2 = sum1 = 0;
	prev3 = prev2 = prev1 = 0;
	this->div_ = div;
	nextdivn = div;
// 	bufpos = div - 1;
}

template<const unsigned channels>
std::size_t Cic3Core<channels>::filter(short *out, const short *const in, std::size_t inlen) {
// 	const std::size_t produced = (inlen + div_ - (bufpos + 1)) / div_;
	const std::size_t produced = (inlen + div_ - nextdivn) / div_;
	const long mul = 0x10000 / (div_ * div_ * div_); // trouble if div is too large, may be better to only support power of 2 div
	const short *s = in;
	
	/*unsigned long sm1 = sum1;
	unsigned long sm2 = sum2;
	unsigned long sm3 = sum3;
	
	while (inlen >> 1) {
		unsigned n = (inlen < BUFLEN ? inlen >> 1 : BUFLEN >> 1);
		const unsigned end = n * 2;
		unsigned i = 0;
		
		do {
			unsigned long s1 = sm1 += static_cast<long>(*s);
			s += channels;
			sm1 += static_cast<long>(*s);
			s += channels;
			unsigned long s2 = sm2 += s1;
			sm2 += sm1;
			buf[i++] = sm3 += s2;
			buf[i++] = sm3 += sm2;
		} while (--n);
		
		while (bufpos < end) {
			const unsigned long out3 = buf[bufpos] - prev3;
			prev3 = buf[bufpos];
			bufpos += div_;
			
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
			buf[i++] = sm3 += sm2;
		} while (--n);
		
		while (bufpos < inlen) {
			const unsigned long out3 = buf[bufpos] - prev3;
			prev3 = buf[bufpos];
			bufpos += div_;
			
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
	sum3 = sm3;*/
	
	
	unsigned long sm1 = sum1;
	unsigned long sm2 = sum2;
	unsigned long sm3 = sum3;
	
	if (inlen >= nextdivn) {
		unsigned divn = nextdivn;
		std::size_t n = produced;
		
		do {
			do {
				sm1 += static_cast<long>(*s);
				sm2 += sm1;
				sm3 += sm2;
				s += channels;
			} while (--divn);
			
			const unsigned long out3 = sm3 - prev3;
			prev3 = sm3;
				
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
			sm2 += sm1;
			sm3 += sm2;
			s += channels;
		}
	}
	
	sum1 = sm1;
	sum2 = sm2;
	sum3 = sm3;
	
	return produced;
}

/*template<unsigned channels> 
class Cic3EvenOddCore {
	unsigned long sum1;
	unsigned long sum2;
	unsigned long sum3;
	unsigned long prev1;
	unsigned long prev2;
	unsigned long prev3;
	unsigned div_;
	unsigned nextdivn;
	
	static int getMul(unsigned div) {
		return 0x10000 / (div * div * div); // trouble if div is too large, may be better to only support power of 2 div
	}
	
	void filterEven(short *out, const short *s, std::size_t n);
	void filterOdd(short *out, const short *s, std::size_t n);
	
public:
	Cic3EvenOddCore(const unsigned div = 2) {
		reset(div);
	}
	
	unsigned div() const { return div_; }
	std::size_t filter(short *out, const short *in, std::size_t inlen);
	void reset(unsigned div);
};

template<const unsigned channels> 
void Cic3EvenOddCore<channels>::reset(const unsigned div) {
	sum3 = sum2 = sum1 = 0;
	prev3 = prev2 = prev1 = 0;
	this->div_ = div;
	nextdivn = div;
}

template<const unsigned channels>
void Cic3EvenOddCore<channels>::filterEven(short *out, const short *s, std::size_t n) {
	const int mul = getMul(div_);
	unsigned long sm1 = sum1;
	unsigned long sm2 = sum2;
	unsigned long sm3 = sum3;
	
	while (n--) {
		{
			unsigned sn = div_ >> 1;
			
			do {
				unsigned long s1 = sm1 += static_cast<long>(*s);
				s += channels;
				sm1 += static_cast<long>(*s);
				s += channels;
				unsigned long s2 = sm2 += s1;
				sm2 += sm1;
				sm3 += s2;
				sm3 += sm2;
			} while (--sn);
		}
			
		const unsigned long out3 = sm3 - prev3;
		prev3 = sm3;
		const unsigned long out2 = out3 - prev2;
		prev2 = out3;
		*out = static_cast<long>(out2 - prev1) * mul / 0x10000;
		prev1 = out2;
		out += channels;
	}
	
	sum1 = sm1;
	sum2 = sm2;
	sum3 = sm3;
}

template<const unsigned channels>
void Cic3EvenOddCore<channels>::filterOdd(short *out, const short *s, std::size_t n) {
	const int mul = getMul(div_);
	unsigned long sm1 = sum1;
	unsigned long sm2 = sum2;
	unsigned long sm3 = sum3;
	
	while (n--) {
		{
			unsigned sn = div_ >> 1;
			
			do {
				unsigned long s1 = sm1 += static_cast<long>(*s);
				s += channels;
				sm1 += static_cast<long>(*s);
				s += channels;
				unsigned long s2 = sm2 += s1;
				sm2 += sm1;
				sm3 += s2;
				sm3 += sm2;
			} while (--sn);
		}
		
		sm1 += static_cast<long>(*s);
		s += channels;
		sm2 += sm1;
		sm3 += sm2;
		
		const unsigned long out3 = sm3 - prev3;
		prev3 = sm3;
		const unsigned long out2 = out3 - prev2;
		prev2 = out3;
		*out = static_cast<long>(out2 - prev1) * mul / 0x10000;
		prev1 = out2;
		out += channels;
	}
	
	sum1 = sm1;
	sum2 = sm2;
	sum3 = sm3;
}

template<const unsigned channels>
std::size_t Cic3EvenOddCore<channels>::filter(short *out, const short *const in, std::size_t inlen) {
	short *const outStart = out;
	const short *s = in;
	
	if (inlen >= nextdivn) {
		{
			{
				unsigned divn = nextdivn;
				
				do {
					sum1 += static_cast<long>(*s);
					s += channels;
					sum2 += sum1;
					sum3 += sum2;
				} while (--divn);
			}
			
			const unsigned long out3 = sum3 - prev3;
			prev3 = sum3;
			const unsigned long out2 = out3 - prev2;
			prev2 = out3;
			*out = static_cast<long>(out2 - prev1) * getMul(div_) / 0x10000;
			prev1 = out2;
			out += channels;
		}
		
		std::size_t n = (inlen - nextdivn) / div_;
		
		if (div_ & 1)
			filterOdd(out, s, n);
		else
			filterEven(out, s, n);
		
		s += n * div_ * channels;
		out += n * channels;
		nextdivn = div_;
	}
	
	{
		unsigned divn = inlen - (s - in) / channels;
		nextdivn -= divn;
		
		while (divn--) {
			sum1 += static_cast<long>(*s);
			s += channels;
			sum2 += sum1;
			sum3 += sum2;
		}
	}
	
	return (out - outStart) / channels;
}*/

template<unsigned channels>
class Cic3 : public SubResampler {
	Cic3Core<channels> cics[channels];
	
public:
	enum { MAX_DIV = 23 };
	Cic3(unsigned div);
	std::size_t resample(short *out, const short *in, std::size_t inlen);
	unsigned mul() const { return 1; }
	unsigned div() const { return cics[0].div(); }
};

template<const unsigned channels>
Cic3<channels>::Cic3(const unsigned div) {
	for (unsigned i = 0; i < channels; ++i)
		cics[i].reset(div);
}

template<const unsigned channels>
std::size_t Cic3<channels>::resample(short *const out, const short *const in, const std::size_t inlen) {
	std::size_t samplesOut;
	
	for (unsigned i = 0; i < channels; ++i) {
		samplesOut = cics[i].filter(out + i, in + i, inlen);
	}
	
	return samplesOut;
}

#endif
