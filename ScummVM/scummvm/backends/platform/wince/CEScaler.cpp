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
#include "CEScaler.h"

extern int gBitFormat;
#ifdef ARM
extern "C" {
	void SmartphoneLandscapeARM(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height, int mask);
}

void SmartphoneLandscape(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	// Rounding constants and masks used for different pixel formats
	static const int redbluegreenMasks[] = { 0x03E07C1F, 0x07E0F81F };
	const int maskUsed = (gBitFormat == 565);
	SmartphoneLandscapeARM(srcPtr, srcPitch, dstPtr, dstPitch, width, height, redbluegreenMasks[maskUsed]);
}

#else

template<typename ColorMask>
void SmartphoneLandscapeTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	int line = 0;

	assert((width % 16) == 0);

	while (height--) {
		uint16 *d = (uint16 *)dstPtr;

		const uint16 *s = (const uint16 *)srcPtr;
		for (int i = 0; i < width; i += 16) {
			// Downscale horizontally to 11/16.
			// See smartLandScale.s for an explanation of the scale pattern.
			*d++ = interpolate32_3_1<ColorMask>(s[0], s[1]);
			*d++ = interpolate32_1_1<ColorMask>(s[1], s[2]);
			*d++ = interpolate32_3_1<ColorMask>(s[3], s[2]);
			*d++ = interpolate32_1_1<ColorMask>(s[4], s[5]);
			*d++ = interpolate32_3_1<ColorMask>(s[6], s[7]);
			*d++ = interpolate32_1_1<ColorMask>(s[7], s[8]);
			*d++ = interpolate32_3_1<ColorMask>(s[9], s[8]);
			*d++ = interpolate32_1_1<ColorMask>(s[10], s[11]);
			*d++ = interpolate32_3_1<ColorMask>(s[12], s[13]);
			*d++ = interpolate32_1_1<ColorMask>(s[13], s[14]);
			*d++ = interpolate32_3_1<ColorMask>(s[15], s[14]);

			s += 16;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch;
		line++;

		// Skip every 8th row
		if (line == 7) {
			line = 0;
			srcPtr += srcPitch;
			height--;
		}
	}
}

void SmartphoneLandscape(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	if (gBitFormat == 565)
		SmartphoneLandscapeTemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		SmartphoneLandscapeTemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

#endif
