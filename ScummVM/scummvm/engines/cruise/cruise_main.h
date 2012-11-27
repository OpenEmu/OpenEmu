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

#ifndef CRUISE_CRUISE_MAIN_H
#define CRUISE_CRUISE_MAIN_H

#include "common/scummsys.h"
#include "common/savefile.h"

#include "cruise/overlay.h"
#include "cruise/object.h"
#include "cruise/ctp.h"
#include "cruise/actor.h"
#include "cruise/vars.h"
#include "cruise/font.h"
#include "cruise/volume.h"
#include "cruise/stack.h"
#include "cruise/script.h"
#include "cruise/various.h"
#include "cruise/function.h"
#include "cruise/saveload.h"
#include "cruise/linker.h"
#include "cruise/mouse.h"
#include "cruise/gfxModule.h"
#include "cruise/dataLoader.h"
#include "cruise/perso.h"
#include "cruise/menu.h"

#include "cruise/background.h"
#include "cruise/backgroundIncrust.h"

#include "cruise/mainDraw.h"

namespace Cruise {

enum MouseButton {
	CRS_MB_LEFT = 1,
	CRS_MB_RIGHT = 2,
	CRS_MB_MIDDLE = 4,
	CRS_MB_BOTH = CRS_MB_LEFT | CRS_MB_RIGHT
};

/*#define DUMP_SCRIPT
#define DUMP_OBJECT*/

#define ASSERT_PTR assert
#define ASSERT assert

enum ResType {
	OBJ_TYPE_LINE = 0,
	OBJ_TYPE_MASK = 1,
	OBJ_TYPE_BGMASK = 2,
	OBJ_TYPE_VIRTUAL = 3,
	OBJ_TYPE_SPRITE = 4,
	OBJ_TYPE_MESSAGE = 5,
	OBJ_TYPE_SOUND = 6,
	OBJ_TYPE_FONT = 7,
	OBJ_TYPE_POLY = 8,
	OBJ_TYPE_EXIT = 9
};

extern gfxEntryStruct* linkedMsgList;

extern int buttonDown;
extern int selectDown;
extern int menuDown;

bool delphineUnpack(byte *dst, const byte *src, int len);
int findHighColor();
ovlData3Struct *getOvlData3Entry(int32 scriptNumber, int32 param);
ovlData3Struct *scriptFunc1Sub2(int32 scriptNumber, int32 param);
void resetFileEntry(int32 entryNumber);
uint8 *mainProc14(uint16 overlay, uint16 idx);
void printInfoBlackBox(const char *string);
void waitForPlayerInput();
void loadPackedFileToMem(int fileIdx, uint8 * buffer);
int getNumObjectsByClass(int scriptIdx, int param);
void resetFileEntryRange(int param1, int param2);
int getProcParam(int overlayIdx, int param2, const char * name);
void changeScriptParamInList(int param1, int param2, scriptInstanceStruct * pScriptInstance, int newValue, int param3);
uint8 *getDataFromData3(ovlData3Struct * ptr, int param);
void removeExtention(const char *name, char *buffer);
void resetPtr2(scriptInstanceStruct * ptr);
void getFileExtention(const char *name, char *buffer);
void *allocAndZero(int size);
void freeStuff2();
void mainLoop();
void getMouseStatus(int16 *pMouseVar, int16 *pMouseX, int16 *pMouseButton, int16 *pMouseY);
bool testMask(int x, int y, unsigned char* pData, int stride);
menuElementSubStruct *getSelectedEntryInMenu(menuStruct *pMenu);
void closeAllMenu();
int removeFinishedScripts(scriptInstanceStruct *ptrHandle);
void initBigVar3();
void resetActorPtr(actorStruct *ptr);
void removeAllScripts(scriptInstanceStruct *ptrHandle);

void MemoryList();
void *MemoryAlloc(uint32 size, bool clearFlag, int32 lineNum, const char *fname);
void MemoryFree(void *v);

#define mallocAndZero(size) MemoryAlloc(size, true, __LINE__, __FILE__)
#define MemAlloc(size) MemoryAlloc(size, false, __LINE__, __FILE__)
#define MemFree(v) MemoryFree(v)

} // End of namespace Cruise

#endif
