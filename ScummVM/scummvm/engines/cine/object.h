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

#ifndef CINE_OBJECT_H
#define CINE_OBJECT_H

#include "script.h"

namespace Cine {

struct ObjectStruct {
	int16 x;
	int16 y;
	uint16 mask;
	int16 frame;
	int16 costume;
	char name[20];
	uint16 part;

	/** Sets all member variables to zero. */
	void clear() {
		this->x = 0;
		this->y = 0;
		this->mask = 0;
		this->frame = 0;
		this->costume = 0;
		memset(this->name, 0, sizeof(this->name));
		this->part = 0;
	}
};

struct overlay {
	uint16 objIdx;
	uint16 type;
	int16 x;
	int16 y;
	int16 width;
	int16 color;
};

#define NUM_MAX_OBJECT 255
#define NUM_MAX_VAR 255

void resetObjectTable();
void loadObject(char *pObjectName);
void setupObject(byte objIdx, uint16 param1, uint16 param2, uint16 param3, uint16 param4);
void modifyObjectParam(byte objIdx, byte paramIdx, int16 newValue);

void addOverlay(uint16 objIdx, uint16 type);
int removeOverlay(uint16 objIdx, uint16 param);
void addGfxElement(int16 objIdx, int16 param, int16 type);
void removeGfxElement(int16 objIdx, int16 param, int16 type);

int16 getObjectParam(uint16 objIdx, uint16 paramIdx);

void addObjectParam(byte objIdx, byte paramIdx, int16 newValue);
void subObjectParam(byte objIdx, byte paramIdx, int16 newValue);
bool compareRanges(uint16 aStart, uint16 aEnd, uint16 bStart, uint16 bEnd);
uint16 compareObjectParamRanges(uint16 objIdx1, uint16 xAdd1, uint16 yAdd1, uint16 maskAdd1, uint16 objIdx2, uint16 xAdd2, uint16 yAdd2, uint16 maskAdd2);
uint16 compareObjectParam(byte objIdx, byte param1, int16 param2);

} // End of namespace Cine

#endif
