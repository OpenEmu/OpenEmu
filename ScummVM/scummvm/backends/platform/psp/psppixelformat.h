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

#ifndef PSP_PIXEL_FORMAT_H
#define PSP_PIXEL_FORMAT_H

#include "graphics/pixelformat.h"
#include "backends/platform/psp/trace.h"

/**
 *	Specialized PixelFormat class
 *  Supports only those formats which the PSP allows, including 4 bit palettes.
 *  Also provides accurate color conversion (needed for color masking)
 *  As well as swapping of red and blue channels (needed by HE games, for example)
 */
struct PSPPixelFormat {
	enum Type {
		Type_None,
		Type_4444,
		Type_5551,
		Type_5650,
		Type_8888,
		Type_Palette_8bit,
		Type_Palette_4bit,
		Type_Unknown
	};

	Type format;
	uint32 bitsPerPixel;					///< Must match bpp of selected type
	bool swapRB;							///< Swap red and blue values when reading and writing

	PSPPixelFormat() : format(Type_Unknown), bitsPerPixel(0), swapRB(false) {}
	void set(Type type, bool swap = false);
	static void convertFromScummvmPixelFormat(const Graphics::PixelFormat *pf,
	        PSPPixelFormat::Type &bufferType,
	        PSPPixelFormat::Type &paletteType,
	        bool &swapRedBlue);
	static Graphics::PixelFormat convertToScummvmPixelFormat(PSPPixelFormat::Type type);
	uint32 convertTo32BitColor(uint32 color);

	inline uint32 rgbaToColor(uint32 r, uint32 g, uint32 b, uint32 a) {
		uint32 color;

		switch (format) {
		case Type_4444:
			color = (((b >> 4) << 8) | ((g >> 4) << 4) | ((r >> 4) << 0) | ((a >> 4) << 12));
			break;
		case Type_5551:
			color = (((b >> 3) << 10) | ((g >> 3) << 5) | ((r >> 3) << 0) | ((a >> 7) << 15));
			break;
		case Type_5650:
			color = (((b >> 3) << 11) | ((g >> 2) << 5) | ((r >> 3) << 0));
			break;
		case Type_8888:
			color = (((b >> 0) << 16) | ((g >> 0) << 8) | ((r >> 0) << 0) | ((a >> 0) << 24));
			break;
		default:
			color = 0;
			break;
		}
		return color;
	}

	inline void colorToRgba(uint32 color, uint32 &r, uint32 &g, uint32 &b, uint32 &a) {
		switch (format) {
		case Type_4444:
			a = (color >> 12) & 0xF; // Interpolate to get true colors
			b = (color >> 8)  & 0xF;
			g = (color >> 4)  & 0xF;
			r = (color >> 0)  & 0xF;
			a = a << 4 | a;
			b = b << 4 | b;
			g = g << 4 | g;
			r = r << 4 | r;
			break;
		case Type_5551:
			a = (color >> 15) ? 0xFF : 0;
			b = (color >> 10) & 0x1F;
			g = (color >> 5)  & 0x1F;
			r = (color >> 0)  & 0x1F;
			b = b << 3 | b >> 2;
			g = g << 3 | g >> 2;
			r = r << 3 | r >> 2;
			break;
		case Type_5650:
			a = 0xFF;
			b = (color >> 11) & 0x1F;
			g = (color >> 5)  & 0x3F;
			r = (color >> 0)  & 0x1F;
			b = b << 3 | b >> 2;
			g = g << 2 | g >> 4;
			r = r << 3 | r >> 2;
			break;
		case Type_8888:
			a = (color >> 24) & 0xFF;
			b = (color >> 16) & 0xFF;
			g = (color >> 8)  & 0xFF;
			r = (color >> 0)  & 0xFF;
			break;
		default:
			a = b = g = r = 0;
			break;
		}
	}

	inline uint32 setColorAlpha(uint32 color, byte alpha) {
		switch (format) {
		case Type_4444:
			color = (color & 0x0FFF) | (((uint32)alpha >> 4) << 12);
			break;
		case Type_5551:
			color = (color & 0x7FFF) | (((uint32)alpha >> 7) << 15);
			break;
		case Type_8888:
			color = (color & 0x00FFFFFF) | ((uint32)alpha << 24);
			break;
		case Type_5650:
		default:
			break;
		}
		return color;
	}

	inline uint32 pixelsToBytes(uint32 pixels) {
		switch (bitsPerPixel) {
		case 4:
			pixels >>= 1;
			break;
		case 16:
			pixels <<= 1;
			break;
		case 32:
			pixels <<= 2;
			break;
		case 8:
			break;
		default:
			PSP_ERROR("Incorrect bitsPerPixel value[%u]. pixels[%u]\n", bitsPerPixel, pixels);
			break;
		}
		return pixels;
	}

	inline uint16 swapRedBlue16(uint16 color) {
		uint16 output;

		switch (format) {
		case Type_4444:
			output = (color & 0xf0f0) | ((color & 0x000f) << 8)  | ((color & 0x0f00) >> 8);
			break;
		case Type_5551:
			output = (color & 0x83e0) | ((color & 0x001f) << 10) | ((color & 0x7c00) >> 10);
			break;
		case Type_5650:
			output = (color & 0x07e0) | ((color & 0x001f) << 11) | ((color & 0xf800) >> 11);
			break;
		default:
			PSP_ERROR("invalid format[%u] for swapping\n", format);
			output = 0;
			break;
		}
		return output;
	}

	inline uint32 swapRedBlue32(uint32 color) {
		uint32 output;

		switch (format) {
		case Type_4444:
			output = (color & 0xf0f0f0f0) |
			         ((color & 0x000f000f) << 8)  | ((color & 0x0f000f00) >> 8);
			break;
		case Type_5551:
			output = (color & 0x83e083e0) |
			         ((color & 0x001f001f) << 10) | ((color & 0x7c007c00) >> 10);
			break;
		case Type_5650:
			output = (color & 0x07e007e0) |
			         ((color & 0x001f001f) << 11) | ((color & 0xf800f800) >> 11);
			break;
		case Type_8888:
			output = (color & 0xff00ff00) |
			         ((color & 0x000000ff) << 16) | ((color & 0x00ff0000) >> 16);
			break;
		default:
			PSP_ERROR("invalid format[%u] for swapping\n", format);
			output = 0;
			break;
		}

		return output;
	}

	// Return whatever color we point at
	inline uint32 getColorValueAt(byte *pointer) {
		uint32 result;

		switch (bitsPerPixel) {
		case 4:	// We can't distinguish a 4 bit color with a pointer
		case 8:
			result = *pointer;
			break;
		case 16:
			result = *(uint16 *)pointer;
			break;
		case 32:
			result = *(uint32 *)pointer;
			break;
		default:
			result = 0;
			PSP_ERROR("Incorrect bitsPerPixel value[%u].\n", bitsPerPixel);
			break;
		}
		return result;
	}
};

#endif /* PSP_PIXEL_FORMAT_H */
