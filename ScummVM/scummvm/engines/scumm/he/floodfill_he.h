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

#if !defined(SCUMM_HE_FLOODFILL_HE_H) && defined(ENABLE_HE)
#define SCUMM_HE_FLOODFILL_HE_H

#include "common/rect.h"

namespace Scumm {

struct FloodFillParameters {
	Common::Rect box;
	int32 x;
	int32 y;
	int32 flags;
};

struct FloodFillLine {
	int y;
	int x1;
	int x2;
	int inc;
};

struct FloodFillState {
	FloodFillLine *fillLineTable;
	FloodFillLine *fillLineTableEnd;
	FloodFillLine *fillLineTableCur;
	Common::Rect dstBox;
	Common::Rect srcBox;
	uint8 *dst;
	int dst_w;
	int dst_h;
	int color1;
	int color2;
	int fillLineTableCount;
};

class ScummEngine_v90he;

typedef bool (*FloodFillPixelCheckCallback)(int x, int y, const FloodFillState *ffs);

void floodFill(FloodFillParameters *ffp, ScummEngine_v90he *vm);

} // End of namespace Scumm

#endif
