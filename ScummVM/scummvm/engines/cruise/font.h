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

#ifndef CRUISE_FONT_H
#define CRUISE_FONT_H

#include "common/scummsys.h"

namespace Cruise {

#include "common/pack-start.h"	// START STRUCT PACKING

struct FontInfo {
	uint32 size;
	uint32 offset;
	uint16 numChars;
	int16 hSpacing;
	int16 vSpacing;
} PACKED_STRUCT;

struct FontEntry {
	uint32 offset;
	int16 v1;
	int16 charHeight;
	int16 height2;
	int16 charWidth;
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

void loadFNT(const char *fileName);
void initSystem();
void freeSystem();

//////////////////////////////////////////////////
void bigEndianShortToNative(void *var);
void bigEndianLongToNative(void *var);	// TODO: move away
void flipGen(void *var, int32 length);

int32 getLineHeight(int16 charCount, const FontEntry *fontPtr, const uint8 *fontPrt_Desc);	// fontProc1
int32 getTextLineCount(int32 rightBorder_X, int32 wordSpacingWidth, const FontEntry *fontData,
					   const char *textString);
void renderWord(uint8 *fontPtr_Data, uint8 *outBufferPtr,
                int32 drawPosPixel_X, int32 heightOff, int32 height, int32 param4,
                int32 stringRenderBufferSize, int32 width, int32 charWidth);
gfxEntryStruct *renderText(int inRightBorder_X, const char *string);
void drawString(int32 x, int32 y, const char *string, uint8 * buffer, uint8 fontColor,
                int32 inRightBorder_X);
void freeGfx(gfxEntryStruct *pGfx);

} // End of namespace Cruise

#endif
