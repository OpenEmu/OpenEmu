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

#ifndef CRUISE_CELL_H
#define CRUISE_CELL_H


#include "common/scummsys.h"

namespace Cruise {

struct gfxEntryStruct;

struct cellStruct {
	struct cellStruct *next;
	struct cellStruct *prev;
	int16 idx;
	int16 type;
	int16 overlay;
	int16 x;
	int16 field_C;
	int16 spriteIdx;
	int16 color;
	int16 backgroundPlane;
	int16 freeze;
	int16 parent;
	int16 parentOverlay;
	int16 parentType;
	int16 followObjectOverlayIdx;
	int16 followObjectIdx;
	int16 animStart;
	int16 animEnd;
	int16 animWait;
	int16 animStep;
	int16 animChange;
	int16 animType;
	int16 animSignal;
	int16 animCounter;
	int16 animLoop;
	gfxEntryStruct *gfxPtr;
};

extern cellStruct cellHead;

void resetPtr(cellStruct * ptr);
cellStruct *addCell(cellStruct *pHead, int16 overlayIdx, int16 objIdx, int16 type, int16 backgroundPlane, int16 scriptOverlay, int16 scriptNumber, int16 scriptType);
void createTextObject(cellStruct *pObject, int overlayIdx, int messageIdx, int x, int y, int width, int16 color, int backgroundPlane, int parentOvl, int parentIdx);
void removeCell(cellStruct *objPtr, int ovlNumber, int objectIdx, int objType, int backgroundPlane);
void freezeCell(cellStruct * pObject, int overlayIdx, int objIdx, int objType, int backgroundPlane, int oldFreeze, int newFreeze);
void sortCells(int16 param1, int16 param2, cellStruct *objPtr);
void linkCell(cellStruct *pHead, int ovl, int obj, int type, int ovl2, int obj2);


} // End of namespace Cruise

#endif
