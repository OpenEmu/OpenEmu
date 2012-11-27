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

#ifndef CRUISE_SCRIPT_H
#define CRUISE_SCRIPT_H

namespace Cruise {

enum scriptTypeEnum {
	scriptType_MinusPROC = -20,
	scriptType_Minus30 = -30,
	scriptType_PROC = 20,
	scriptType_REL = 30
};

struct scriptInstanceStruct {
	struct scriptInstanceStruct *nextScriptPtr;
	int16 ccr;
	int16 scriptOffset;
	uint8 *data;
	int16 dataSize;
	int16 scriptNumber;
	int16 overlayNumber;
	int16 sysKey;
	int16 freeze;
	scriptTypeEnum type;
	int16 var16;
	int16 var18;
	int16 var1A;
};

extern scriptInstanceStruct relHead;
extern scriptInstanceStruct procHead;
extern scriptInstanceStruct *currentScriptPtr;

void setupFuncArray();
int8 getByteFromScript();

int removeScript(int overlay, int idx, scriptInstanceStruct * headPtr);
uint8 *attacheNewScriptToTail(scriptInstanceStruct *scriptHandlePtr, int16 overlayNumber, int16 param, int16 arg0, int16 arg1, int16 arg2, scriptTypeEnum scriptType);
void manageScripts(scriptInstanceStruct * scriptHandle);

} // End of namespace Cruise

#endif
