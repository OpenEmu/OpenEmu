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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// The YUV to RGB conversion code is derived from SDL's YUV overlay code, which
// in turn appears to be derived from mpeg_play. The following copyright
// notices have been included in accordance with the original license. Please
// note that the term "software" in this context only applies to the
// buildLookup() and plotYUV*() functions below.

// Copyright (c) 1995 The Regents of the University of California.
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without written agreement is
// hereby granted, provided that the above copyright notice and the following
// two paragraphs appear in all copies of this software.
//
// IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
// OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
// CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
// ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
// PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

// Copyright (c) 1995 Erik Corry
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without written agreement is
// hereby granted, provided that the above copyright notice and the following
// two paragraphs appear in all copies of this software.
//
// IN NO EVENT SHALL ERIK CORRY BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
// SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF
// THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF ERIK CORRY HAS BEEN ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ERIK CORRY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
// BASIS, AND ERIK CORRY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
// UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

// Portions of this software Copyright (c) 1995 Brown University.
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without written agreement
// is hereby granted, provided that the above copyright notice and the
// following two paragraphs appear in all copies of this software.
//
// IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
// OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BROWN
// UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// BROWN UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
// BASIS, AND BROWN UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
// SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

#include "graphics/surface.h"
#include "graphics/yuv_to_rgb.h"

namespace Common {
DECLARE_SINGLETON(Graphics::YUVToRGBManager);
}

namespace Graphics {

class YUVToRGBLookup {
public:
	YUVToRGBLookup(Graphics::PixelFormat format, YUVToRGBManager::LuminanceScale scale);

	Graphics::PixelFormat getFormat() const { return _format; }
	YUVToRGBManager::LuminanceScale getScale() const { return _scale; }
	const uint32 *getRGBToPix() const { return _rgbToPix; }

private:
	Graphics::PixelFormat _format;
	YUVToRGBManager::LuminanceScale _scale;
	uint32 _rgbToPix[3 * 768]; // 9216 bytes
};

YUVToRGBLookup::YUVToRGBLookup(Graphics::PixelFormat format, YUVToRGBManager::LuminanceScale scale) {
	_format = format;
	_scale = scale;

	uint32 *r_2_pix_alloc = &_rgbToPix[0 * 768];
	uint32 *g_2_pix_alloc = &_rgbToPix[1 * 768];
	uint32 *b_2_pix_alloc = &_rgbToPix[2 * 768];

	if (scale == YUVToRGBManager::kScaleFull) {
		// Set up entries 0-255 in rgb-to-pixel value tables.
		for (int i = 0; i < 256; i++) {
			r_2_pix_alloc[i + 256] = format.RGBToColor(i, 0, 0);
			g_2_pix_alloc[i + 256] = format.RGBToColor(0, i, 0);
			b_2_pix_alloc[i + 256] = format.RGBToColor(0, 0, i);
		}

		// Spread out the values we have to the rest of the array so that we do
		// not need to check for overflow.
		for (int i = 0; i < 256; i++) {
			r_2_pix_alloc[i] = r_2_pix_alloc[256];
			r_2_pix_alloc[i + 512] = r_2_pix_alloc[511];
			g_2_pix_alloc[i] = g_2_pix_alloc[256];
			g_2_pix_alloc[i + 512] = g_2_pix_alloc[511];
			b_2_pix_alloc[i] = b_2_pix_alloc[256];
			b_2_pix_alloc[i + 512] = b_2_pix_alloc[511];
		}
	} else {
		// Set up entries 16-235 in rgb-to-pixel value tables
		for (int i = 16; i < 236; i++) {
			int scaledValue = (i - 16) * 255 / 219;
			r_2_pix_alloc[i + 256] = format.RGBToColor(scaledValue, 0, 0);
			g_2_pix_alloc[i + 256] = format.RGBToColor(0, scaledValue, 0);
			b_2_pix_alloc[i + 256] = format.RGBToColor(0, 0, scaledValue);
		}

		// Spread out the values we have to the rest of the array so that we do
		// not need to check for overflow. We have to do it here in two steps.
		for (int i = 0; i < 256 + 16; i++) {
			r_2_pix_alloc[i] = r_2_pix_alloc[256 + 16];
			g_2_pix_alloc[i] = g_2_pix_alloc[256 + 16];
			b_2_pix_alloc[i] = b_2_pix_alloc[256 + 16];
		}

		for (int i = 256 + 236; i < 768; i++) {
			r_2_pix_alloc[i] = r_2_pix_alloc[256 + 236 - 1];
			g_2_pix_alloc[i] = g_2_pix_alloc[256 + 236 - 1];
			b_2_pix_alloc[i] = b_2_pix_alloc[256 + 236 - 1];
		}
	}
}

YUVToRGBManager::YUVToRGBManager() {
	_lookup = 0;

	int16 *Cr_r_tab = &_colorTab[0 * 256];
	int16 *Cr_g_tab = &_colorTab[1 * 256];
	int16 *Cb_g_tab = &_colorTab[2 * 256];
	int16 *Cb_b_tab = &_colorTab[3 * 256];

	// Generate the tables for the display surface

	for (int i = 0; i < 256; i++) {
		// Gamma correction (luminescence table) and chroma correction
		// would be done here. See the Berkeley mpeg_play sources.

		int16 CR = (i - 128), CB = CR;
		Cr_r_tab[i] = (int16) ( (0.419 / 0.299) * CR) + 0 * 768 + 256;
		Cr_g_tab[i] = (int16) (-(0.299 / 0.419) * CR) + 1 * 768 + 256;
		Cb_g_tab[i] = (int16) (-(0.114 / 0.331) * CB);
		Cb_b_tab[i] = (int16) ( (0.587 / 0.331) * CB) + 2 * 768 + 256;
	}
}

YUVToRGBManager::~YUVToRGBManager() {
	delete _lookup;
}

const YUVToRGBLookup *YUVToRGBManager::getLookup(Graphics::PixelFormat format, YUVToRGBManager::LuminanceScale scale) {
	if (_lookup && _lookup->getFormat() == format && _lookup->getScale() == scale)
		return _lookup;

	delete _lookup;
	_lookup = new YUVToRGBLookup(format, scale);
	return _lookup;
}

#define PUT_PIXEL(s, d) \
	L = &rgbToPix[(s)]; \
	*((PixelInt *)(d)) = (L[cr_r] | L[crb_g] | L[cb_b])

template<typename PixelInt>
void convertYUV444ToRGB(byte *dstPtr, int dstPitch, const YUVToRGBLookup *lookup, int16 *colorTab, const byte *ySrc, const byte *uSrc, const byte *vSrc, int yWidth, int yHeight, int yPitch, int uvPitch) {
	// Keep the tables in pointers here to avoid a dereference on each pixel
	const int16 *Cr_r_tab = colorTab;
	const int16 *Cr_g_tab = Cr_r_tab + 256;
	const int16 *Cb_g_tab = Cr_g_tab + 256;
	const int16 *Cb_b_tab = Cb_g_tab + 256;
	const uint32 *rgbToPix = lookup->getRGBToPix();

	for (int h = 0; h < yHeight; h++) {
		for (int w = 0; w < yWidth; w++) {
			register const uint32 *L;

			int16 cr_r  = Cr_r_tab[*vSrc];
			int16 crb_g = Cr_g_tab[*vSrc] + Cb_g_tab[*uSrc];
			int16 cb_b  = Cb_b_tab[*uSrc];
			++uSrc;
			++vSrc;

			PUT_PIXEL(*ySrc, dstPtr);
			ySrc++;
			dstPtr += sizeof(PixelInt);
		}

		dstPtr += dstPitch - yWidth * sizeof(PixelInt);
		ySrc += yPitch - yWidth;
		uSrc += uvPitch - yWidth;
		vSrc += uvPitch - yWidth;
	}
}

void YUVToRGBManager::convert444(Graphics::Surface *dst, YUVToRGBManager::LuminanceScale scale, const byte *ySrc, const byte *uSrc, const byte *vSrc, int yWidth, int yHeight, int yPitch, int uvPitch) {
	// Sanity checks
	assert(dst && dst->pixels);
	assert(dst->format.bytesPerPixel == 2 || dst->format.bytesPerPixel == 4);
	assert(ySrc && uSrc && vSrc);

	const YUVToRGBLookup *lookup = getLookup(dst->format, scale);

	// Use a templated function to avoid an if check on every pixel
	if (dst->format.bytesPerPixel == 2)
		convertYUV444ToRGB<uint16>((byte *)dst->pixels, dst->pitch, lookup, _colorTab, ySrc, uSrc, vSrc, yWidth, yHeight, yPitch, uvPitch);
	else
		convertYUV444ToRGB<uint32>((byte *)dst->pixels, dst->pitch, lookup, _colorTab, ySrc, uSrc, vSrc, yWidth, yHeight, yPitch, uvPitch);
}

template<typename PixelInt>
void convertYUV420ToRGB(byte *dstPtr, int dstPitch, const YUVToRGBLookup *lookup, int16 *colorTab, const byte *ySrc, const byte *uSrc, const byte *vSrc, int yWidth, int yHeight, int yPitch, int uvPitch) {
	int halfHeight = yHeight >> 1;
	int halfWidth = yWidth >> 1;

	// Keep the tables in pointers here to avoid a dereference on each pixel
	const int16 *Cr_r_tab = colorTab;
	const int16 *Cr_g_tab = Cr_r_tab + 256;
	const int16 *Cb_g_tab = Cr_g_tab + 256;
	const int16 *Cb_b_tab = Cb_g_tab + 256;
	const uint32 *rgbToPix = lookup->getRGBToPix();

	for (int h = 0; h < halfHeight; h++) {
		for (int w = 0; w < halfWidth; w++) {
			register const uint32 *L;

			int16 cr_r  = Cr_r_tab[*vSrc];
			int16 crb_g = Cr_g_tab[*vSrc] + Cb_g_tab[*uSrc];
			int16 cb_b  = Cb_b_tab[*uSrc];
			++uSrc;
			++vSrc;

			PUT_PIXEL(*ySrc, dstPtr);
			PUT_PIXEL(*(ySrc + yPitch), dstPtr + dstPitch);
			ySrc++;
			dstPtr += sizeof(PixelInt);
			PUT_PIXEL(*ySrc, dstPtr);
			PUT_PIXEL(*(ySrc + yPitch), dstPtr + dstPitch);
			ySrc++;
			dstPtr += sizeof(PixelInt);
		}

		dstPtr += dstPitch;
		ySrc += (yPitch << 1) - yWidth;
		uSrc += uvPitch - halfWidth;
		vSrc += uvPitch - halfWidth;
	}
}

void YUVToRGBManager::convert420(Graphics::Surface *dst, YUVToRGBManager::LuminanceScale scale, const byte *ySrc, const byte *uSrc, const byte *vSrc, int yWidth, int yHeight, int yPitch, int uvPitch) {
	// Sanity checks
	assert(dst && dst->pixels);
	assert(dst->format.bytesPerPixel == 2 || dst->format.bytesPerPixel == 4);
	assert(ySrc && uSrc && vSrc);
	assert((yWidth & 1) == 0);
	assert((yHeight & 1) == 0);

	const YUVToRGBLookup *lookup = getLookup(dst->format, scale);

	// Use a templated function to avoid an if check on every pixel
	if (dst->format.bytesPerPixel == 2)
		convertYUV420ToRGB<uint16>((byte *)dst->pixels, dst->pitch, lookup, _colorTab, ySrc, uSrc, vSrc, yWidth, yHeight, yPitch, uvPitch);
	else
		convertYUV420ToRGB<uint32>((byte *)dst->pixels, dst->pitch, lookup, _colorTab, ySrc, uSrc, vSrc, yWidth, yHeight, yPitch, uvPitch);
}

#define READ_QUAD(ptr, prefix) \
	byte prefix##A = ptr[index]; \
	byte prefix##B = ptr[index + 1]; \
	byte prefix##C = ptr[index + uvPitch]; \
	byte prefix##D = ptr[index + uvPitch + 1]

#define DO_INTERPOLATION(out) \
	out = (out##A * (4 - xDiff) * (4 - yDiff) + out##B * xDiff * (4 - yDiff) + \
			out##C * yDiff * (4 - xDiff) + out##D * xDiff * yDiff) >> 4

#define DO_YUV410_PIXEL() \
	DO_INTERPOLATION(u); \
	DO_INTERPOLATION(v); \
	\
	cr_r  = Cr_r_tab[v]; \
	crb_g = Cr_g_tab[v] + Cb_g_tab[u]; \
	cb_b  = Cb_b_tab[u]; \
	\
	PUT_PIXEL(*ySrc, dstPtr); \
	dstPtr += sizeof(PixelInt); \
	\
	ySrc++; \
	xDiff++

template<typename PixelInt>
void convertYUV410ToRGB(byte *dstPtr, int dstPitch, const YUVToRGBLookup *lookup, int16 *colorTab, const byte *ySrc, const byte *uSrc, const byte *vSrc, int yWidth, int yHeight, int yPitch, int uvPitch) {
	// Keep the tables in pointers here to avoid a dereference on each pixel
	const int16 *Cr_r_tab = colorTab;
	const int16 *Cr_g_tab = Cr_r_tab + 256;
	const int16 *Cb_g_tab = Cr_g_tab + 256;
	const int16 *Cb_b_tab = Cb_g_tab + 256;
	const uint32 *rgbToPix = lookup->getRGBToPix();

	int quarterWidth = yWidth >> 2;

	for (int y = 0; y < yHeight; y++) {
		for (int x = 0; x < quarterWidth; x++) {
			// Perform bilinear interpolation on the the chroma values
			// Based on the algorithm found here: http://tech-algorithm.com/articles/bilinear-image-scaling/
			// Feel free to optimize further
			int targetY = y >> 2;
			int xDiff = 0;
			int yDiff = y & 3;
			int index = targetY * uvPitch + x;

			// Declare some variables for the following macros
			byte u, v;
			int16 cr_r, crb_g, cb_b;
			register const uint32 *L;

			READ_QUAD(uSrc, u);
			READ_QUAD(vSrc, v);

			DO_YUV410_PIXEL();
			DO_YUV410_PIXEL();
			DO_YUV410_PIXEL();
			DO_YUV410_PIXEL();
		}

		dstPtr += dstPitch - yWidth * sizeof(PixelInt);
		ySrc += yPitch - yWidth;
	}
}

#undef READ_QUAD
#undef DO_INTERPOLATION
#undef DO_YUV410_PIXEL

void YUVToRGBManager::convert410(Graphics::Surface *dst, YUVToRGBManager::LuminanceScale scale, const byte *ySrc, const byte *uSrc, const byte *vSrc, int yWidth, int yHeight, int yPitch, int uvPitch) {
	// Sanity checks
	assert(dst && dst->pixels);
	assert(dst->format.bytesPerPixel == 2 || dst->format.bytesPerPixel == 4);
	assert(ySrc && uSrc && vSrc);
	assert((yWidth & 3) == 0);
	assert((yHeight & 3) == 0);

	const YUVToRGBLookup *lookup = getLookup(dst->format, scale);

	// Use a templated function to avoid an if check on every pixel
	if (dst->format.bytesPerPixel == 2)
		convertYUV410ToRGB<uint16>((byte *)dst->pixels, dst->pitch, lookup, _colorTab, ySrc, uSrc, vSrc, yWidth, yHeight, yPitch, uvPitch);
	else
		convertYUV410ToRGB<uint32>((byte *)dst->pixels, dst->pitch, lookup, _colorTab, ySrc, uSrc, vSrc, yWidth, yHeight, yPitch, uvPitch);
}

} // End of namespace Graphics
