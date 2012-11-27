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

#include "common/scummsys.h"
#include "backends/platform/psp/psppixelformat.h"

//#define __PSP_DEBUG_FUNCS__	/* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */

#include "backends/platform/psp/trace.h"

// class PSPPixelFormat --------------------------------------

void PSPPixelFormat::set(Type type, bool swap /* = false */) {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("type = %d\n", type);

	format = type;
	swapRB = swap;

	switch (type) {
	case Type_4444:
	case Type_5551:
	case Type_5650:
		bitsPerPixel = 16;
		break;
	case Type_8888:
		bitsPerPixel = 32;
		break;
	case Type_Palette_8bit:
		bitsPerPixel = 8;
		break;
	case Type_Palette_4bit:
		bitsPerPixel = 4;
		break;
	case Type_None:
		bitsPerPixel = 0;
		break;
	default:	// This is an error, but let's continue anyway
		PSP_ERROR("Unhandled value of pixel type[%d]\n", type);
		bitsPerPixel = 16;
		break;
	}

	PSP_DEBUG_PRINT("bitsPerPixel[%u]\n", bitsPerPixel);
}

// Convert from ScummVM general PixelFormat to our pixel format
// For buffer and palette.
void PSPPixelFormat::convertFromScummvmPixelFormat(const Graphics::PixelFormat *pf,
        PSPPixelFormat::Type &bufferType,
        PSPPixelFormat::Type &paletteType,
        bool &swapRedBlue) {
	swapRedBlue = false;	 // no red-blue swap by default
	PSPPixelFormat::Type *target = 0;	// which one we'll be filling

	if (!pf) {	// Default, pf is NULL
		bufferType = Type_Palette_8bit;
		paletteType = Type_5551;
	} else {	// We have a pf
		if (pf->bytesPerPixel == 1) {
			bufferType = Type_Palette_8bit;
			target = &paletteType;	// The type describes the palette
		} else if (pf->bytesPerPixel == 2) {
			paletteType = Type_None;
			target = &bufferType;	// The type describes the buffer
		} else {
			PSP_ERROR("Unknown bpp[%u] in pixeltype. Reverting to 8bpp\n", pf->bytesPerPixel);
			bufferType = Type_Palette_8bit;
			target = &paletteType;	// The type describes the palette
		}

		// Find out the exact type of the target
		if (pf->rLoss == 3 && pf->bLoss == 3) {
			if (pf->gLoss == 3)
				*target = Type_5551;
			else
				*target = Type_5650;
		} else if (pf->rLoss == 4 && pf->gLoss == 4 && pf->bLoss == 4) {
			*target = Type_4444;
		} else if (pf->gLoss == 0 && pf->gShift == 8) {
			*target = Type_8888;
		} else if ((pf->gLoss == 0 && pf->gShift == 0) ||
		           (pf->gLoss == 8 && pf->gShift == 0)) {	// Default CLUT8 can have weird values
			*target = Type_5551;
		} else {
			PSP_ERROR("Unknown Scummvm pixel format.\n");
			PSP_ERROR("\trLoss[%d], gLoss[%d], bLoss[%d], aLoss[%d]\n\trShift[%d], gShift[%d], bShift[%d], aShift[%d]\n",
			          pf->rLoss, pf->gLoss, pf->bLoss, pf->aLoss,
			          pf->rShift, pf->gShift, pf->bShift, pf->aShift);
			*target = Type_Unknown;
		}

		if (pf->rShift != 0)	{// We allow backend swap of red and blue
			swapRedBlue = true;
			PSP_DEBUG_PRINT("detected red/blue swap\n");
		}
	}
}

Graphics::PixelFormat PSPPixelFormat::convertToScummvmPixelFormat(PSPPixelFormat::Type type) {
	Graphics::PixelFormat pf;

	switch (type) {
	case Type_4444:
		pf.bytesPerPixel = 2;
		pf.aLoss = 4;
		pf.rLoss = 4;
		pf.gLoss = 4;
		pf.bLoss = 4;
		pf.aShift = 12;
		pf.rShift = 0;
		pf.gShift = 4;
		pf.bShift = 8;
		break;
	case Type_5551:
		pf.bytesPerPixel = 2;
		pf.aLoss = 7;
		pf.rLoss = 3;
		pf.gLoss = 3;
		pf.bLoss = 3;
		pf.aShift = 15;
		pf.rShift = 0;
		pf.gShift = 5;
		pf.bShift = 10;
		break;
	case Type_5650:
		pf.bytesPerPixel = 2;
		pf.aLoss = 8;
		pf.rLoss = 3;
		pf.gLoss = 2;
		pf.bLoss = 3;
		pf.aShift = 0;
		pf.rShift = 0;
		pf.gShift = 5;
		pf.bShift = 11;
		break;
	case Type_8888:
		pf.bytesPerPixel = 4;
		pf.aLoss = 0;
		pf.rLoss = 0;
		pf.gLoss = 0;
		pf.bLoss = 0;
		pf.aShift = 24;
		pf.rShift = 0;
		pf.gShift = 8;
		pf.bShift = 16;
		break;
	default:
		PSP_ERROR("Unhandled PSPPixelFormat[%u]\n", type);
		break;
	}

	return pf;
}

uint32 PSPPixelFormat::convertTo32BitColor(uint32 color) {
	DEBUG_ENTER_FUNC();
	uint32 r, g, b, a, output;

	colorToRgba(color, r, g, b, a);
	output = ((b << 16) | (g << 8) | (r << 0) | (a << 24));
	PSP_DEBUG_PRINT_FUNC("input color[%x], output[%x]\n", color, output);

	return output;
}
