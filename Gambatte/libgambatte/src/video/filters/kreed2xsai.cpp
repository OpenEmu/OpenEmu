/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   Copyright (C) 1999 Derek Liauw Kie Fa (Kreed)                         *
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
#include "kreed2xsai.h"
#include "filterinfo.h"
#include <cstring>

static inline int getResult1(const unsigned long a, const unsigned long b, const unsigned long c, const unsigned long d) {
	int x = 0;
	int y = 0;
	int r = 0;
	
	if (a == c) ++x;
	else if (b == c) ++y;
	
	if (a == d) ++x;
	else if (b == d) ++y;
	
	if (x <= 1) ++r;
	
	if (y <= 1) --r;
	
	return r;
}

static inline int getResult2(const unsigned long a, const unsigned long b, const unsigned long c, const unsigned long d) {
	int x = 0;
	int y = 0;
	int r = 0;
	
	if (a == c) ++x;
	else if (b == c) ++y;
	
	if (a == d) ++x;
	else if (b == d) ++y;
	
	if (x <= 1) --r;
	
	if (y <= 1) ++r;
	
	return r;
}

static inline unsigned long interpolate(const unsigned long a, const unsigned long b) {
	return (a + b - ((a ^ b) & 0x010101)) >> 1;
}

static inline unsigned long qInterpolate(const unsigned long a, const unsigned long b, const unsigned long c, const unsigned long d) {
	const unsigned long lowBits = ((a & 0x030303) + (b & 0x030303) + (c & 0x030303) + (d & 0x030303)) & 0x030303;
	
	return (a + b + c + d - lowBits) >> 2;
}

static void filter(Gambatte::uint_least32_t *dstPtr, const unsigned dstPitch,
		   const Gambatte::uint_least32_t *srcPtr, const unsigned srcPitch, const unsigned width, unsigned height)
{
	while (height--) {
		const Gambatte::uint_least32_t *bP = srcPtr;
		Gambatte::uint_least32_t *dP = dstPtr;
		
		for (unsigned finish = width; finish--;) {
			register unsigned long colorA, colorB;
			unsigned long colorC, colorD,
				colorE, colorF, colorG, colorH,
				colorI, colorJ, colorK, colorL,
				
				colorM, colorN, colorO, colorP;
			unsigned long product, product1, product2;
			
      //---------------------------------------
      // Map of the pixels:                    I|E F|J
      //                                       G|A B|K
      //                                       H|C D|L
      //                                       M|N O|P
			colorI = *(bP - srcPitch - 1);
			colorE = *(bP - srcPitch);
			colorF = *(bP - srcPitch + 1);
			colorJ = *(bP - srcPitch + 2);
			
			colorG = *(bP - 1);
			colorA = *(bP);
			colorB = *(bP + 1);
			colorK = *(bP + 2);
			
			colorH = *(bP + srcPitch - 1);
			colorC = *(bP + srcPitch);
			colorD = *(bP + srcPitch + 1);
			colorL = *(bP + srcPitch + 2);
			
			colorM = *(bP + srcPitch * 2 - 1);
			colorN = *(bP + srcPitch * 2);
			colorO = *(bP + srcPitch * 2 + 1);
			colorP = *(bP + srcPitch * 2 + 2);
			
			if (colorA == colorD && colorB != colorC) {
				if ((colorA == colorE && colorB == colorL) ||
				    (colorA == colorC && colorA == colorF
				     && colorB != colorE && colorB == colorJ)) {
					     product = colorA;
				     } else {
					     product = interpolate(colorA, colorB);
				     }
				
				if ((colorA == colorG && colorC == colorO) ||
				    (colorA == colorB && colorA == colorH
				     && colorG != colorC && colorC == colorM)) {
					     product1 = colorA;
				     } else {
					     product1 = interpolate(colorA, colorC);
				     }
				product2 = colorA;
			} else if (colorB == colorC && colorA != colorD) {
				if ((colorB == colorF && colorA == colorH) ||
				    (colorB == colorE && colorB == colorD
				     && colorA != colorF && colorA == colorI)) {
					     product = colorB;
				     } else {
					     product = interpolate(colorA, colorB);
				     }
				
				if ((colorC == colorH && colorA == colorF) ||
				    (colorC == colorG && colorC == colorD
				     && colorA != colorH && colorA == colorI)) {
					     product1 = colorC;
				     } else {
					     product1 = interpolate(colorA, colorC);
				     }
				product2 = colorB;
			} else if (colorA == colorD && colorB == colorC) {
				if (colorA == colorB) {
					product = colorA;
					product1 = colorA;
					product2 = colorA;
				} else {
					register int r = 0;
					
					product1 = interpolate(colorA, colorC);
					product = interpolate(colorA, colorB);
					
					r += getResult1(colorA, colorB, colorG, colorE);
					r += getResult2(colorB, colorA, colorK, colorF);
					r += getResult2(colorB, colorA, colorH, colorN);
					r += getResult1(colorA, colorB, colorL, colorO);
					
					if (r > 0)
						product2 = colorA;
					else if (r < 0)
						product2 = colorB;
					else {
						product2 = qInterpolate(colorA, colorB, colorC, colorD);
					}
				}
			} else {
				product2 = qInterpolate(colorA, colorB, colorC, colorD);
				
				if (colorA == colorC && colorA == colorF
				    && colorB != colorE && colorB == colorJ) {
					    product = colorA;
				    } else if (colorB == colorE && colorB == colorD
				               && colorA != colorF && colorA == colorI) {
					               product = colorB;
				               } else {
					               product = interpolate(colorA, colorB);
				               }
				
				if (colorA == colorB && colorA == colorH
				    && colorG != colorC && colorC == colorM) {
					    product1 = colorA;
				    } else if (colorC == colorG && colorC == colorD
				               && colorA != colorH && colorA == colorI) {
					               product1 = colorC;
				               } else {
					               product1 = interpolate(colorA, colorC);
				               }
			}
			*dP = colorA;
			*(dP + 1) = product;
			*(dP + dstPitch) = product1;
			*(dP + dstPitch + 1) = product2;
			
			++bP;
			dP += 2;
		}
		
		srcPtr += srcPitch;
		dstPtr += dstPitch * 2;
	}
}

Kreed_2xSaI::Kreed_2xSaI() {
	buffer = NULL;
}

Kreed_2xSaI::~Kreed_2xSaI() {
	delete []buffer;
}

void Kreed_2xSaI::init() {
	delete []buffer;

	buffer = new Gambatte::uint_least32_t[145 * 161];
	std::memset(buffer, 0, 145ul * 161 * sizeof(Gambatte::uint_least32_t));
}

void Kreed_2xSaI::outit() {
	delete []buffer;
	buffer = NULL;
}

const Gambatte::FilterInfo& Kreed_2xSaI::info() {
	static Gambatte::FilterInfo fInfo = { "Kreed's 2xSaI", 160 * 2, 144 * 2 };
	
	return fInfo;
}

Gambatte::uint_least32_t* Kreed_2xSaI::inBuffer() {
	return buffer;
}

unsigned Kreed_2xSaI::inPitch() {
	return 161;
}

void Kreed_2xSaI::filter(Gambatte::uint_least32_t *const dbuffer, const unsigned pitch) {
	::filter(dbuffer, pitch, buffer, 161, 160, 144);
}
