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
#include "graphics/scaler/downscaler.h"
#include "graphics/scaler/intern.h"

#ifdef USE_ARM_SCALER_ASM
extern "C" {
	void DownscaleAllByHalfARM(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height, int mask, int round);
}

void DownscaleAllByHalf(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	// Rounding constants and masks used for different pixel formats
	static const int roundingconstants[] = { 0x00200802, 0x00201002 };
	static const int redbluegreenMasks[] = { 0x03E07C1F, 0x07E0F81F };

	extern int gBitFormat;

	const int maskUsed = (gBitFormat == 565);
	DownscaleAllByHalfARM(srcPtr, srcPitch, dstPtr, dstPitch, width, height, redbluegreenMasks[maskUsed], roundingconstants[maskUsed]);
}

#else

template<typename ColorMask>
void DownscaleAllByHalfTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	uint8 *work;
	uint16 srcPitch16 = (uint16)(srcPitch / sizeof(uint16));

	while ((height -= 2) >= 0) {
		work = dstPtr;

		for (int i=0; i<width; i+=2) {
			// Another lame filter attempt :)
			uint16 color1 = *(((const uint16 *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)srcPtr) + (i + 1));
			uint16 color3 = *(((const uint16 *)srcPtr) + (i + srcPitch16));
			uint16 color4 = *(((const uint16 *)srcPtr) + (i + srcPitch16 + 1));
			*(((uint16 *)work) + 0) = interpolate16_1_1_1_1<ColorMask>(color1, color2, color3, color4);

			work += sizeof(uint16);
		}
		srcPtr += 2 * srcPitch;
		dstPtr += dstPitch;
	}
}

void DownscaleAllByHalf(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	extern int gBitFormat;
	if (gBitFormat == 565)
		DownscaleAllByHalfTemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		DownscaleAllByHalfTemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

#endif


/**
 * This filter (down)scales the source image horizontally by a factor of 1/2.
 * For example, a 320x200 image is scaled to 160x200.
 */
template<typename ColorMask>
void DownscaleHorizByHalfTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	uint16 *work;

	// Various casts below go via (void *) to avoid warning. This is
	// safe as these are all even addresses.
	while (height--) {
		work = (uint16 *)(void *)dstPtr;

		for (int i = 0; i < width; i += 2) {
			uint16 color1 = *(((const uint16 *)(const void *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)(const void *)srcPtr) + (i + 1));
			*work++ = interpolate32_1_1<ColorMask>(color1, color2);
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

void DownscaleHorizByHalf(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	extern int gBitFormat;
	if (gBitFormat == 565)
		DownscaleHorizByHalfTemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		DownscaleHorizByHalfTemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

/**
 * This filter (down)scales the source image horizontally by a factor of 3/4.
 * For example, a 320x200 image is scaled to 240x200.
 */
template<typename ColorMask>
void DownscaleHorizByThreeQuartersTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	uint16 *work;

	// Various casts below go via (void *) to avoid warning. This is
	// safe as these are all even addresses.
	while (height--) {
		work = (uint16 *)(void *)dstPtr;

		for (int i = 0; i < width; i += 4) {
			// Work with 4 pixels
			uint16 color1 = *(((const uint16 *)(const void *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)(const void *)srcPtr) + (i + 1));
			uint16 color3 = *(((const uint16 *)(const void *)srcPtr) + (i + 2));
			uint16 color4 = *(((const uint16 *)(const void *)srcPtr) + (i + 3));

			work[0] = interpolate32_3_1<ColorMask>(color1, color2);
			work[1] = interpolate32_1_1<ColorMask>(color2, color3);
			work[2] = interpolate32_3_1<ColorMask>(color4, color3);

			work += 3;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

void DownscaleHorizByThreeQuarters(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	extern int gBitFormat;
	if (gBitFormat == 565)
		DownscaleHorizByThreeQuartersTemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		DownscaleHorizByThreeQuartersTemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}
