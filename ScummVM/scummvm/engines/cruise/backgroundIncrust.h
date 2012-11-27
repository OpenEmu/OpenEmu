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

#ifndef CRUISE_BACKGROUNDINCRUST_H
#define CRUISE_BACKGROUNDINCRUST_H

namespace Cruise {

struct backgroundIncrustStruct {
	struct backgroundIncrustStruct *next;
	struct backgroundIncrustStruct *prev;

	uint16 objectIdx;
	int16 type;
	uint16 overlayIdx;
	int16 X;
	int16 Y;
	uint16 frame;
	uint16 scale;
	uint16 backgroundIdx;
	uint16 scriptNumber;
	uint16 scriptOverlayIdx;
	uint8 *ptr;
	int16 saveWidth;
	int16 saveHeight;
	uint16 saveSize;
	int16 savedX;
	int16 savedY;
	char name[13];
	uint16 spriteId;
};

extern backgroundIncrustStruct backgroundIncrustHead;

void resetBackgroundIncrustList(backgroundIncrustStruct * pHead);
backgroundIncrustStruct *addBackgroundIncrust(int16 overlayIdx, int16 param2, backgroundIncrustStruct * pHead, int16 scriptNumber, int16 scriptOverlay, int16 backgroundIdx, int16 param4);
void regenerateBackgroundIncrust(backgroundIncrustStruct * pHead);
void freeBackgroundIncrustList(backgroundIncrustStruct * pHead);
void removeBackgroundIncrust(int overlay, int idx, backgroundIncrustStruct * pHead);
void unmergeBackgroundIncrust(backgroundIncrustStruct * pHead, int ovl, int idx);

} // End of namespace Cruise

#endif
