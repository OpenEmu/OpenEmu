/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
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
#include "catrom2x.h"
#include <algorithm>

namespace {
enum { WIDTH  = VfilterInfo::IN_WIDTH };
enum { HEIGHT = VfilterInfo::IN_HEIGHT };
enum { PITCH  = WIDTH + 3 };

struct Colorsum {
	gambatte::uint_least32_t r, g, b;
};

static void merge_columns(gambatte::uint_least32_t *dest, const Colorsum *sums) {
	unsigned w = WIDTH;
	
	while (w--) {
		{
			gambatte::uint_least32_t rsum = sums[1].r;
			gambatte::uint_least32_t gsum = sums[1].g;
			gambatte::uint_least32_t bsum = sums[1].b;
			
			if (rsum & 0x80000000) rsum = 0;
			if (gsum & 0x80000000) gsum = 0;
			if (bsum & 0x80000000) bsum = 0;
			
			rsum <<= 12;
			rsum += 0x008000;
			gsum >>= 4;
			gsum += 0x0080;
			bsum += 0x0008;
			bsum >>= 4;
			
			if (rsum > 0xFF0000) rsum = 0xFF0000;
			if (gsum > 0x00FF00) gsum = 0x00FF00;
			if (bsum > 0x0000FF) bsum = 0x0000FF;
			
			*dest++ = (rsum & 0xFF0000) | (gsum & 0x00FF00) | bsum;
		}
		
		{
			gambatte::uint_least32_t rsum = sums[1].r * 9;
			gambatte::uint_least32_t gsum = sums[1].g * 9;
			gambatte::uint_least32_t bsum = sums[1].b * 9;
			
			rsum -= sums[0].r;
			gsum -= sums[0].g;
			bsum -= sums[0].b;
			
			rsum += sums[2].r * 9;
			gsum += sums[2].g * 9;
			bsum += sums[2].b * 9;
			
			rsum -= sums[3].r;
			gsum -= sums[3].g;
			bsum -= sums[3].b;
			
			if (rsum & 0x80000000) rsum = 0;
			if (gsum & 0x80000000) gsum = 0;
			if (bsum & 0x80000000) bsum = 0;
			
			rsum <<= 8;
			rsum += 0x008000;
			gsum >>= 8;
			gsum += 0x000080;
			bsum += 0x000080;
			bsum >>= 8;
			
			if (rsum > 0xFF0000) rsum = 0xFF0000;
			if (gsum > 0x00FF00) gsum = 0x00FF00;
			if (bsum > 0x0000FF) bsum = 0x0000FF;
			
			*dest++ = (rsum & 0xFF0000) | (gsum & 0x00FF00) | bsum;
		}
		
		++sums;
	}
}

static void filter(gambatte::uint_least32_t *dline, const int pitch, const gambatte::uint_least32_t *sline) {
	Colorsum sums[PITCH];
	
	for (unsigned h = HEIGHT; h--;) {
		{
			const gambatte::uint_least32_t *s = sline;
			Colorsum *sum = sums;
			unsigned n = PITCH;
			
			while (n--) {
				unsigned long pixel = *s;
				sum->r = pixel >> 12 & 0x000FF0 ;
				pixel <<= 4;
				sum->g = pixel & 0x0FF000;
				sum->b = pixel & 0x000FF0;
				
				++s;
				++sum;
			}
		}
		
		merge_columns(dline, sums);
		dline += pitch;
		
		{
			const gambatte::uint_least32_t *s = sline;
			Colorsum *sum = sums;
			unsigned n = PITCH;
			
			while (n--) {
				unsigned long pixel = *s;
				unsigned long rsum = (pixel >> 16) * 9;
				unsigned long gsum = (pixel & 0x00FF00) * 9;
				unsigned long bsum = (pixel & 0x0000FF) * 9;
				
				pixel = s[-1*PITCH];
				rsum -= pixel >> 16;
				gsum -= pixel & 0x00FF00;
				bsum -= pixel & 0x0000FF;
				
				pixel = s[1*PITCH];
				rsum += (pixel >> 16) * 9;
				gsum += (pixel & 0x00FF00) * 9;
				bsum += (pixel & 0x0000FF) * 9;
				
				pixel = s[2*PITCH];
				rsum -= pixel >> 16;
				gsum -= pixel & 0x00FF00;
				bsum -= pixel & 0x0000FF;
				
				sum->r = rsum;
				sum->g = gsum;
				sum->b = bsum;
				
				++s;
				++sum;
			}
		}
		
		merge_columns(dline, sums);
		dline += pitch;
		sline += PITCH;
	}
}
}

Catrom2x::Catrom2x()
: buffer_((HEIGHT + 3UL) * PITCH)
{
	std::fill_n(buffer_.get(), buffer_.size(), 0);
}

void* Catrom2x::inBuf() const {
	return buffer_ + PITCH + 1;
}

int Catrom2x::inPitch() const {
	return PITCH;
}

void Catrom2x::draw(void *const dbuffer, const int pitch) {
	::filter(static_cast<gambatte::uint_least32_t*>(dbuffer), pitch, buffer_ + PITCH);
}
