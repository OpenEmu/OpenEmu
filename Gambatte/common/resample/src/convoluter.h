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
#include "array.h"
#include "rshift16_round.h"

template<unsigned channels, unsigned phases>
class PolyPhaseConvoluter {
	const short *kernel;
	Array<short> prevbuf;
	
	unsigned div_;
	unsigned x_;
	
public:
	PolyPhaseConvoluter() : kernel(0), div_(0), x_(0) {}
	PolyPhaseConvoluter(const short *kernel, unsigned phaseLen, unsigned div) { reset(kernel, phaseLen, div); }
	void reset(const short *kernel, unsigned phaseLen, unsigned div);
	std::size_t filter(short *out, const short *in, std::size_t inlen);
	void adjustDiv(const unsigned div) { this->div_ = div; }
	unsigned div() const { return div_; }
};

template<unsigned channels, unsigned phases>
void PolyPhaseConvoluter<channels, phases>::reset(const short *const kernel, const unsigned phaseLen, const unsigned div) {
	this->kernel = kernel;
	this->div_ = div;
	x_ = 0;
	prevbuf.reset(phaseLen);
	std::fill(prevbuf.get(), prevbuf.get() + phaseLen, 0);
}

template<unsigned channels, unsigned phases>
std::size_t PolyPhaseConvoluter<channels, phases>::filter(short *out, const short *const in, std::size_t inlen) {
	if (!kernel || !inlen)
		return 0;
	
	// The gist of what happens here is given by the commented pseudo-code below.
	// Note that the order of the kernel elements has been changed for efficiency in the real implementation.
	
	/*for (std::size_t x = 0; x < inlen + M; ++x) {
		const int end = x < inlen ? M + 1 : inlen + M - x;
		int j = x < M ? M - x : 0;
		j += (phases - (x - M + j) % phases) % phases; // adjust j so we don't start on a virtual 0 sample

		for (; j < end; j += phases) {
			buffer[x] += kernel[j] * start[(x - M + j) / phases];
		}
	}*/
	
	// Slightly more optimized version.
	
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
	
	const std::size_t phaseLen = prevbuf.size();
	const std::size_t M = phaseLen * phases - 1;
	inlen *= phases;
	std::size_t x = x_;
	
	for (; x < (M < inlen ? M : inlen); x += div_) {
		const short *k = kernel + ((x + 1) % phases) * phaseLen; // adjust phase so we don't start on a virtual 0 sample
		const short *s = prevbuf + x / phases + 1;
		long acc = 0;
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
		
		*out = rshift16_round(acc);
		out += channels;
	}
	
	// We could easily get rid of the division and modulus here by updating the
	// k and s pointers incrementally. However, we currently only use powers of 2
	// and we would end up referencing more variables which often compiles to bad
	// code on x86, which is why I'm also hesistant to get rid of the template arguments.
	for (; x < inlen; x += div_) {
		const short *k = kernel + ((x + 1) % phases) * phaseLen; // adjust phase so we don't start on a virtual 0 sample
		const short *s = in + (x / phases + 1 - phaseLen) * channels;
		long acc = 0;
// 		unsigned n = (M + 1/* - phase + phases - 1*/) / phases;
		unsigned n = phaseLen;
		
		do {
			acc += *k++ * *s;
			s += channels;
		} while (--n);
		
		*out = rshift16_round(acc);
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
