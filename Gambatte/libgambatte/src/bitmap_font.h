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
#ifndef BITMAP_FONT_H
#define BITMAP_FONT_H

#include "gbint.h"

namespace bitmapfont {
enum Char {
	NUL,
	N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,
	A, B, C, D, E, F, G, H, I, J, K, L, M,
	N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	a, b, c, d, e, f, g, h, i, j, k, l, m,
	n, o, p, q, r, s, t, u, v, w, x, y, z,
	SPC
};

enum { HEIGHT = 10 };
enum { MAX_WIDTH = 9 };
enum { NUMBER_WIDTH = 6 };

unsigned getWidth(const char *chars);

// struct Fill { void operator()(RandomAccessIterator dest, unsigned pitch) { fill pixels at dest } }
template<class RandomAccessIterator, class Fill>
void print(RandomAccessIterator dest, unsigned pitch, Fill fill, const char *chars);

void print(gambatte::uint_least32_t *dest, unsigned pitch, unsigned long color, const char *chars);
void utoa(unsigned u, char *a);

// --- INTERFACE END ---



extern const unsigned char *const font[];

template<class RandomAccessIterator, class Fill>
void print(RandomAccessIterator dest, const unsigned pitch, Fill fill, const char *chars) {
	while (const int character = *chars++) {
		RandomAccessIterator dst = dest;
		const unsigned char *s = font[character];
		
		const unsigned width = *s >> 4;
		unsigned h = *s++ & 0xF;
		
		while (h--) {
			RandomAccessIterator d = dst;
			
			unsigned line = *s++;
			
			if (width > 8)
				line |= *s++ << 8;
			
			while (line) {
				if (line & 1)
					fill(d, pitch);
				
				line >>= 1;
				++d;
			}
			
			dst += pitch;
		}
		
		dest += width;
	}
}
}

#endif
