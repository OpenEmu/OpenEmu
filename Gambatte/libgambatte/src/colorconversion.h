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
#ifndef COLORCONVERSION_H
#define COLORCONVERSION_H

#include "int.h"
#include <algorithm>

class Rgb32ToUyvy {
	struct CacheUnit {
		Gambatte::uint_least32_t rgb32;
		Gambatte::uint_least32_t uyvy;
	};
	
	enum { cache_size = 0x100 };
	enum { cache_mask = cache_size - 1 };
	
	CacheUnit cache[cache_size];
	
public:
	Rgb32ToUyvy();
	void operator()(const Gambatte::uint_least32_t *s, Gambatte::uint_least32_t *d, unsigned w, unsigned h, unsigned dstPitch);
};

unsigned long rgb32ToUyvy(unsigned long rgb32);

void rgb32ToRgb16(const Gambatte::uint_least32_t *s, Gambatte::uint_least16_t *d, unsigned w, unsigned h, unsigned dstPitch);
unsigned rgb32ToRgb16(unsigned long rgb32);

#endif
