/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "graphics/scaler/intern.h"



static inline int GetResult(uint32 A, uint32 B, uint32 C, uint32 D) {
	const bool ac = (A==C);
	const bool bc = (B==C);
	const int x1 = ac;
	const int y1 = (bc & !ac);
	const bool ad = (A==D);
	const bool bd = (B==D);
	const int x2 = ad;
	const int y2 = (bd & !ad);
	const int x = x1+x2;
	const int y = y1+y2;
	return (y>>1) - (x>>1);
}

#define interpolate_1_1		interpolate16_1_1<ColorMask>
#define interpolate_3_1		interpolate16_3_1<ColorMask>
#define interpolate_6_1_1	interpolate16_6_1_1<ColorMask>
#define interpolate_1_1_1_1	interpolate16_1_1_1_1<ColorMask>

template<typename ColorMask>
void Super2xSaITemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	const uint16 *bP;
	uint16 *dP;
	const uint32 nextlineSrc = srcPitch >> 1;

	while (height--) {
		bP = (const uint16 *)srcPtr;
		dP = (uint16 *)dstPtr;

		for (int i = 0; i < width; ++i) {
			unsigned color4, color5, color6;
			unsigned color1, color2, color3;
			unsigned colorA0, colorA1, colorA2, colorA3;
			unsigned colorB0, colorB1, colorB2, colorB3;
			unsigned colorS1, colorS2;
			unsigned product1a, product1b, product2a, product2b;

//---------------------------------------    B1 B2
//                                         4  5  6 S2
//                                         1  2  3 S1
//                                           A1 A2

			colorB0 = *(bP - nextlineSrc - 1);
			colorB1 = *(bP - nextlineSrc);
			colorB2 = *(bP - nextlineSrc + 1);
			colorB3 = *(bP - nextlineSrc + 2);

			color4 = *(bP - 1);
			color5 = *(bP);
			color6 = *(bP + 1);
			colorS2 = *(bP + 2);

			color1 = *(bP + nextlineSrc - 1);
			color2 = *(bP + nextlineSrc);
			color3 = *(bP + nextlineSrc + 1);
			colorS1 = *(bP + nextlineSrc + 2);

			colorA0 = *(bP + 2 * nextlineSrc - 1);
			colorA1 = *(bP + 2 * nextlineSrc);
			colorA2 = *(bP + 2 * nextlineSrc + 1);
			colorA3 = *(bP + 2 * nextlineSrc + 2);

//--------------------------------------
			if (color2 == color6 && color5 != color3) {
				product2b = product1b = color2;
			} else if (color5 == color3 && color2 != color6) {
				product2b = product1b = color5;
			} else if (color5 == color3 && color2 == color6) {
				register int r = 0;

				r += GetResult(color6, color5, color1, colorA1);
				r += GetResult(color6, color5, color4, colorB1);
				r += GetResult(color6, color5, colorA2, colorS1);
				r += GetResult(color6, color5, colorB2, colorS2);

				if (r > 0)
					product2b = product1b = color6;
				else if (r < 0)
					product2b = product1b = color5;
				else {
					product2b = product1b = interpolate_1_1(color5, color6);
				}
			} else {
				if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
					product2b = interpolate_3_1(color3, color2);
				else if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
					product2b = interpolate_3_1(color2, color3);
				else
					product2b = interpolate_1_1(color2, color3);

				if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
					product1b = interpolate_3_1(color6, color5);
				else if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
					product1b = interpolate_3_1(color5, color6);
				else
					product1b = interpolate_1_1(color5, color6);
			}

			if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
				product2a = interpolate_1_1(color2, color5);
			else if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
				product2a = interpolate_1_1(color2, color5);
			else
				product2a = color2;

			if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
				product1a = interpolate_1_1(color2, color5);
			else if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
				product1a = interpolate_1_1(color2, color5);
			else
				product1a = color5;

			*(dP + 0) = (uint16) product1a;
			*(dP + 1) = (uint16) product1b;
			*(dP + dstPitch/2 + 0) = (uint16) product2a;
			*(dP + dstPitch/2 + 1) = (uint16) product2b;

			bP += 1;
			dP += 2;
		}

		srcPtr += srcPitch;
		dstPtr += dstPitch * 2;
	}
}

void Super2xSaI(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	extern int gBitFormat;
	if (gBitFormat == 565)
		Super2xSaITemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		Super2xSaITemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

template<typename ColorMask>
void SuperEagleTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	const uint16 *bP;
	uint16 *dP;
	const uint32 nextlineSrc = srcPitch >> 1;

	while (height--) {
		bP = (const uint16 *)srcPtr;
		dP = (uint16 *)dstPtr;
		for (int i = 0; i < width; ++i) {
			unsigned color4, color5, color6;
			unsigned color1, color2, color3;
			unsigned colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
			unsigned product1a, product1b, product2a, product2b;

			colorB1 = *(bP - nextlineSrc);
			colorB2 = *(bP - nextlineSrc + 1);

			color4 = *(bP - 1);
			color5 = *(bP);
			color6 = *(bP + 1);
			colorS2 = *(bP + 2);

			color1 = *(bP + nextlineSrc - 1);
			color2 = *(bP + nextlineSrc);
			color3 = *(bP + nextlineSrc + 1);
			colorS1 = *(bP + nextlineSrc + 2);

			colorA1 = *(bP + 2 * nextlineSrc);
			colorA2 = *(bP + 2 * nextlineSrc + 1);

			// --------------------------------------
			if (color5 != color3) {
				if (color2 == color6) {
					product1b = product2a = color2;
					if ((color1 == color2) || (color6 == colorB2)) {
						product1a = interpolate_3_1(color2, color5);
					} else {
						product1a = interpolate_1_1(color5, color6);
					}

					if ((color6 == colorS2) || (color2 == colorA1)) {
						product2b = interpolate_3_1(color2, color3);
					} else {
						product2b = interpolate_1_1(color2, color3);
					}
				} else {
					product2b = interpolate_6_1_1(color3, color2, color6);
					product1a = interpolate_6_1_1(color5, color2, color6);

					product2a = interpolate_6_1_1(color2, color5, color3);
					product1b = interpolate_6_1_1(color6, color5, color3);
				}
			} else {
				if (color2 != color6) {
					product2b = product1a = color5;

					if ((colorB1 == color5) || (color3 == colorS1)) {
						product1b = interpolate_3_1(color5, color6);
					} else {
						product1b = interpolate_1_1(color5, color6);
					}

					if ((color3 == colorA2) || (color4 == color5)) {
						product2a = interpolate_3_1(color5, color2);
					} else {
						product2a = interpolate_1_1(color2, color3);
					}
				} else {
					register int r = 0;

					r += GetResult(color6, color5, color1, colorA1);
					r += GetResult(color6, color5, color4, colorB1);
					r += GetResult(color6, color5, colorA2, colorS1);
					r += GetResult(color6, color5, colorB2, colorS2);

					if (r > 0) {
						product1b = product2a = color2;
						product1a = product2b = interpolate_1_1(color5, color6);
					} else if (r < 0) {
						product2b = product1a = color5;
						product1b = product2a = interpolate_1_1(color5, color6);
					} else {
						product2b = product1a = color5;
						product1b = product2a = color2;
					}
				}
			}

			*(dP + 0) = (uint16) product1a;
			*(dP + 1) = (uint16) product1b;
			*(dP + dstPitch/2 + 0) = (uint16) product2a;
			*(dP + dstPitch/2 + 1) = (uint16) product2b;

			bP += 1;
			dP += 2;
		}

		srcPtr += srcPitch;
		dstPtr += dstPitch * 2;
	}
}


void SuperEagle(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	extern int gBitFormat;
	if (gBitFormat == 565)
		SuperEagleTemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		SuperEagleTemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

template<typename ColorMask>
void _2xSaITemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	const uint16 *bP;
	uint16 *dP;
	const uint32 nextlineSrc = srcPitch >> 1;

	while (height--) {
		bP = (const uint16 *)srcPtr;
		dP = (uint16 *)dstPtr;

		for (int i = 0; i < width; ++i) {

			register unsigned colorA, colorB;
			unsigned colorC, colorD,
				colorE, colorF, colorG, colorH, colorI, colorJ, colorK, colorL, colorM, colorN, colorO;
			unsigned product, product1, product2;

//---------------------------------------
// Map of the pixels:                    I|E F|J
//                                       G|A B|K
//                                       H|C D|L
//                                       M|N O|P
			colorI = *(bP - nextlineSrc - 1);
			colorE = *(bP - nextlineSrc);
			colorF = *(bP - nextlineSrc + 1);
			colorJ = *(bP - nextlineSrc + 2);

			colorG = *(bP - 1);
			colorA = *(bP);
			colorB = *(bP + 1);
			colorK = *(bP + 2);

			colorH = *(bP + nextlineSrc - 1);
			colorC = *(bP + nextlineSrc);
			colorD = *(bP + nextlineSrc + 1);
			colorL = *(bP + nextlineSrc + 2);

			colorM = *(bP + 2 * nextlineSrc - 1);
			colorN = *(bP + 2 * nextlineSrc);
			colorO = *(bP + 2 * nextlineSrc + 1);

			if ((colorA == colorD) && (colorB != colorC)) {
				if (((colorA == colorE) && (colorB == colorL)) ||
					((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ))) {
					product = colorA;
				} else {
					product = interpolate_1_1(colorA, colorB);
				}

				if (((colorA == colorG) && (colorC == colorO)) ||
					((colorA == colorB) && (colorA == colorH) && (colorG != colorC)  && (colorC == colorM))) {
					product1 = colorA;
				} else {
					product1 = interpolate_1_1(colorA, colorC);
				}
				product2 = colorA;
			} else if ((colorB == colorC) && (colorA != colorD)) {
				if (((colorB == colorF) && (colorA == colorH)) ||
					((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI))) {
					product = colorB;
				} else {
					product = interpolate_1_1(colorA, colorB);
				}

				if (((colorC == colorH) && (colorA == colorF)) ||
					((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI))) {
					product1 = colorC;
				} else {
					product1 = interpolate_1_1(colorA, colorC);
				}
				product2 = colorB;
			} else if ((colorA == colorD) && (colorB == colorC)) {
				if (colorA == colorB) {
					product = colorA;
					product1 = colorA;
					product2 = colorA;
				} else {
					register int r = 0;

					product1 = interpolate_1_1(colorA, colorC);
					product = interpolate_1_1(colorA, colorB);

					r += GetResult(colorA, colorB, colorG, colorE);
					r -= GetResult(colorB, colorA, colorK, colorF);
					r -= GetResult(colorB, colorA, colorH, colorN);
					r += GetResult(colorA, colorB, colorL, colorO);

					if (r > 0)
						product2 = colorA;
					else if (r < 0)
						product2 = colorB;
					else {
						product2 = interpolate_1_1_1_1(colorA, colorB, colorC, colorD);
					}
				}
			} else {
				product2 = interpolate_1_1_1_1(colorA, colorB, colorC, colorD);

				if ((colorA == colorC) && (colorA == colorF)
						&& (colorB != colorE) && (colorB == colorJ)) {
					product = colorA;
				} else if ((colorB == colorE) && (colorB == colorD)
									 && (colorA != colorF) && (colorA == colorI)) {
					product = colorB;
				} else {
					product = interpolate_1_1(colorA, colorB);
				}

				if ((colorA == colorB) && (colorA == colorH)
						&& (colorG != colorC) && (colorC == colorM)) {
					product1 = colorA;
				} else if ((colorC == colorG) && (colorC == colorD)
									 && (colorA != colorH) && (colorA == colorI)) {
					product1 = colorC;
				} else {
					product1 = interpolate_1_1(colorA, colorC);
				}
			}

			*(dP + 0) = (uint16) colorA;
			*(dP + 1) = (uint16) product;
			*(dP + dstPitch/2 + 0) = (uint16) product1;
			*(dP + dstPitch/2 + 1) = (uint16) product2;

			bP += 1;
			dP += 2;
		}

		srcPtr += srcPitch;
		dstPtr += dstPitch * 2;
	}
}


void _2xSaI(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	extern int gBitFormat;
	if (gBitFormat == 565)
		_2xSaITemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		_2xSaITemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}
