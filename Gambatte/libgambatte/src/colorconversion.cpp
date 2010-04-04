/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aam�s                                    *
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
#include "colorconversion.h"
#include <algorithm>

Rgb32ToUyvy::Rgb32ToUyvy() {
#ifdef WORDS_BIGENDIAN
	const CacheUnit c = { 0, 128ul << 24 | 16ul << 16 | 128 << 8 | 16 };
#else
	const CacheUnit c = { 0, 16ul << 24 | 128ul << 16 | 16 << 8 | 128 };
#endif
	std::fill(cache, cache + cache_size, c);
}

void Rgb32ToUyvy::operator()(const Gambatte::uint_least32_t *s, Gambatte::uint_least32_t *d, const unsigned w, unsigned h, const unsigned d_pitch) {
	while (h--) {
		for (const Gambatte::uint_least32_t *const ends = s + w; s != ends;) {
			if ((cache[*s & cache_mask].rgb32 - *s) | (cache[*(s+1) & cache_mask].rgb32 - *(s+1))) {
				cache[*s & cache_mask].rgb32 = *s;
				cache[*(s+1) & cache_mask].rgb32 = *(s+1);

				const unsigned long r = (*s >> 16 & 0x000000FF) | (*(s+1) & 0x00FF0000);
				const unsigned long g = (*s >> 8 & 0x000000FF) | (*(s+1) << 8 & 0x00FF0000);
				const unsigned long b = (*s & 0x000000FF) | (*(s+1) << 16 & 0x00FF0000);

				const unsigned long y = r * 66 + g * 129 + b * 25 + (16 * 256 + 128) * 0x00010001ul;
				const unsigned long u = b * 112 - r * 38 - g * 74 + (128 * 256 + 128) * 0x00010001ul;
				const unsigned long v = r * 112 - g * 94 - b * 18 + (128 * 256 + 128) * 0x00010001ul;

#ifdef WORDS_BIGENDIAN
				*d++ = cache[*s & cache_mask].uyvy = (u << 16 & 0xFF000000) | (y << 8 & 0x00FF0000) | (v & 0x0000FF00) | (y >> 8 & 0x000000FF);
				*d++ = cache[*(s+1) & cache_mask].uyvy = (u & 0xFF000000) | (y >> 8 & 0x00FF0000) | (v >> 16 & 0x0000FF00) | y >> 24;
#else
				*d++ = cache[*s & cache_mask].uyvy = (y << 16 & 0xFF000000) | (v << 8 & 0x00FF0000) | (y & 0x0000FF00) | (u >> 8 & 0x000000FF);
				*d++ = cache[*(s+1) & cache_mask].uyvy = (y & 0xFF000000) | (v >> 8 & 0x00FF0000) | (y >> 16 & 0x0000FF00) | u >> 24;
#endif
			} else {
				*d++ = cache[*s & cache_mask].uyvy;
				*d++ = cache[*(s+1) & cache_mask].uyvy;
			}

			s += 2;
		}

		d += d_pitch - w;
	}
}

unsigned long rgb32ToUyvy(unsigned long rgb32) {
	const unsigned r = rgb32 >> 16 & 0xFF;
	const unsigned g = rgb32 >> 8 & 0xFF;
	const unsigned b = rgb32 & 0xFF;

	const unsigned long y = (r * 66 + g * 129 + b * 25 + 16 * 256 + 128) >> 8;
	const unsigned long u = (b * 112 - r * 38 - g * 74 + 128 * 256 + 128) >> 8;
	const unsigned long v = (r * 112 - g * 94 - b * 18 + 128 * 256 + 128) >> 8;

#ifdef WORDS_BIGENDIAN
	return u << 24 | y << 16 | v << 8 | y;
#else
	return y << 24 | v << 16 | y << 8 | u;
#endif
}

void rgb32ToRgb16(const Gambatte::uint_least32_t *s, Gambatte::uint_least16_t *d, const unsigned w, unsigned h, const unsigned dstPitch) {
	do {
		unsigned n = w;

		do {
			*d++ = (*s >> 8 & 0xF800) | (*s >> 5 & 0x07E0) | (*s >> 3 & 0x001F);
			++s;
		} while (--n);

		d += dstPitch - w;
	} while (--h);
}

unsigned rgb32ToRgb16(const unsigned long rgb32) {
	return (rgb32 >> 8 & 0xF800) | (rgb32 >> 5 & 0x07E0) | (rgb32 >> 3 & 0x001F);
}
