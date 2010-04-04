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
#ifndef CONVOLUTER_H
#define CONVOLUTER_H

#include <algorithm>
#include <cstring>

template<unsigned channels, unsigned phases>
class PolyPhaseConvoluter {
	const short *kernel;
	short *prevbuf;
	
	unsigned phaseLen;
	unsigned div_;
	unsigned x_;
	
public:
	PolyPhaseConvoluter() : kernel(NULL), prevbuf(NULL), phaseLen(0), div_(0), x_(0) {}
	PolyPhaseConvoluter(const short *kernel, unsigned phaseLen, unsigned div) { reset(kernel, phaseLen, div); }
	~PolyPhaseConvoluter() { delete[] prevbuf; }
	void reset(const short *kernel, unsigned phaseLen, unsigned div);
	std::size_t filter(short *out, const short *in, std::size_t inlen);
	void adjustDiv(const unsigned div) { this->div_ = div; }
	unsigned div() const { return div_; }
};

template<const unsigned channels, const unsigned phases>
void PolyPhaseConvoluter<channels, phases>::reset(const short *const kernel, const unsigned phaseLen, const unsigned div) {
	this->kernel = kernel;
	this->phaseLen = phaseLen;
	this->div_ = div;
	x_ = 0;
	delete[] prevbuf;
	prevbuf = new short[phaseLen];
	std::fill(prevbuf, prevbuf + phaseLen, 0);
}

template<const unsigned channels, const unsigned phases>
std::size_t PolyPhaseConvoluter<channels, phases>::filter(short *out, const short *const in, std::size_t inlen) {
	if (!kernel || !inlen)
		return 0;
	
	/*for (std::size_t x = 0; x < inlen + M; ++x) {
		const int end = x < inlen ? M + 1 : inlen + M - x;
		int j = x < M ? M - x : 0;
		j += (phases - (x - M + j) % phases) % phases; // adjust j so we don't start on a virtual 0 sample

		for (; j < end; j += phases) {
			buffer[x] += kernel[j] * start[(x - M + j) / phases];
		}
	}*/
	
	/*for (std::size_t x = 0; x < inlen + M; ++x) {
		const int end = x < inlen ? M + 1 : inlen + M - x;
		int j = x < M ? M - x : 0;
		j += (phases - (x - M + j) % phases) % phases; // adjust j so we don't start on a virtual 0 sample
		const short *k = kernel + (j % phases) * phaseLen + j / phases;
		const short *s = start + (x - M + j) / phases;
		int n = ((end - j) + phases - 1) / phases;

		do {
			buffer[x] += *k++ * *s++;
		} while (--n);
	}*/
	
	const std::size_t M = phaseLen * phases - 1;
	inlen *= phases;
	std::size_t x = x_;
	
	for (; x < (M < inlen ? M : inlen); x += div_) {
		long acc = 0;
		const unsigned phase = (phases - (x + 1) % phases) % phases; // adjust phase so we don't start on a virtual 0 sample
		const short *s = prevbuf + (x + 1 + phase) / phases;
		const short *k = kernel + phase * phaseLen;
		unsigned n = prevbuf + phaseLen - s;

		while (n--) {
			acc += *k++ * *s++;
		}
		
		s = in;
		n = x / phases + 1;
		
		do {
			acc += *k++ * *s;
			s += channels;
		} while (--n);
		
		*out = acc / 0x10000;
		out += channels;
	}
	
	for (; x < inlen; x += div_) {
		long acc = 0;
		const unsigned phase = (phases - (x - M) % phases) % phases; // adjust phase so we don't start on a virtual 0 sample
		const short *s = in + ((x - M + phase) / phases) * channels;
		const short *k = kernel + phase * phaseLen;
// 		unsigned n = (M + 1/* - phase + phases - 1*/) / phases;
		unsigned n = phaseLen;
		
		do {
			acc += *k++ * *s;
			s += channels;
		} while (--n);
		
		*out = acc / 0x10000;
		out += channels;
	}
	
	const std::size_t produced = (x - x_) / div_;
	x_ = x - inlen;
	
	inlen /= phases;
	
	{
		short *p = prevbuf;
		const short *s = in + (inlen - phaseLen) * channels;
		unsigned n = phaseLen;
		
		if (inlen < phaseLen) {
			const unsigned i = phaseLen - inlen;
			
			std::memmove(p, p + inlen, i * sizeof(short));
			
			p += i;
			n -= i;
			s = in;
		}
		
		do {
			*p++ = *s;
			s += channels;
		} while (--n);
	}
	
	return produced;
}

#endif
