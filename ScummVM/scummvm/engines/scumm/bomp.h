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

#ifndef SCUMM_BOMP_H
#define SCUMM_BOMP_H

#include "common/scummsys.h"
#include "graphics/surface.h"

namespace Scumm {

void bompApplyMask(byte *line_buffer, byte *mask, byte maskbit, int32 size, byte transparency);
void bompApplyShadow(int shadowMode, const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency, bool HE7Check = false);

void decompressBomp(byte *dst, const byte *src, int w, int h);
void bompDecodeLine(byte *dst, const byte *src, int size);
void bompDecodeLineReverse(byte *dst, const byte *src, int size);


/** Bomp graphics data */
struct BompDrawData {
	Graphics::Surface dst;
	int x, y;

	const byte *src;
	int srcwidth, srcheight;

	byte scale_x, scale_y;

	byte *maskPtr;
	int numStrips;

	uint16 shadowMode;
	byte *shadowPalette;

	uint16 *actorPalette;

	bool mirror;
};

void drawBomp(const BompDrawData &bd);

} // End of namespace Scumm

#endif
