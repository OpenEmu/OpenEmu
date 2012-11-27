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

#include "cruise/cruise.h"
#include "cruise/cruise_main.h"
#include "cruise/cell.h"
#include "cruise/sound.h"
#include "cruise/staticres.h"

#include "common/textconsole.h"
#include "common/util.h"

namespace Cruise {

uint32 Period(uint32 hz) {
	return ((uint32)(100000000L / ((uint32)hz * 28L)));
}

//#define FUNCTION_DEBUG

int16 Op_LoadOverlay() {
	char *pOverlayName;
	char overlayName[38] = "";
	int overlayLoadResult;

	pOverlayName = (char *)popPtr();

	if (strlen(pOverlayName) == 0)
		return 0;

	strcpy(overlayName, pOverlayName);
	strToUpper(overlayName);

	//gfxModuleData.field_84();
	//gfxModuleData.field_84();

	overlayLoadResult = loadOverlay(overlayName);

	updateAllScriptsImports();

	Common::strlcpy(nextOverlay, overlayName, sizeof(nextOverlay));

	return overlayLoadResult;
}

int16 Op_Strcpy() {
	char *ptr1 = (char *)popPtr();
	char *ptr2 = (char *)popPtr();

	while (*ptr1) {
		*ptr2 = *ptr1;

		ptr2++;
		ptr1++;
	}

	*ptr2 = 0;

	return (0);
}

int16 Op_Exec() {
	int scriptIdx;
	int ovlIdx;
	uint8 *ptr;
	uint8 *ptr2;
	int16 popTable[200];

	int numOfArgToPop = popVar();

	int i = 0;

	for (i = 0; i < numOfArgToPop; i++) {
		popTable[numOfArgToPop - i - 1] = popVar();
	}

	scriptIdx = popVar();
	ovlIdx = popVar();

	if (!ovlIdx) {
		ovlIdx = currentScriptPtr->overlayNumber;
	}

	ptr = attacheNewScriptToTail(&procHead, ovlIdx, scriptIdx, currentScriptPtr->type, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, scriptType_MinusPROC);

	if (!ptr)
		return (0);

	if (numOfArgToPop <= 0) {
		return (0);
	}

	ptr2 = ptr;

	for (i = 0; i < numOfArgToPop; i++) {
		WRITE_BE_UINT16(ptr2, popTable[i]);
		ptr2 += 2;
	}

	return (0);
}

int16 Op_AddProc() {
	int pop1 = popVar();
	int pop2;
	int overlay;
	int param[160];

	for (long int i = 0; i < pop1; i++) {
		param[i] = popVar();
	}

	pop2 = popVar();
	overlay = popVar();

	if (!overlay)
		overlay = currentScriptPtr->overlayNumber;

	if (!overlay)
		return (0);

	uint8* procBss = attacheNewScriptToTail(&procHead, overlay, pop2, currentScriptPtr->type, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, scriptType_PROC);

	if (procBss) {
		for (long int i = 0; i < pop1; i++) {
			int16* ptr = (int16 *)(procBss + i * 2);
			*ptr = param[i];
			bigEndianShortToNative(ptr);
		}
	}

	return (0);
}

int16 Op_Narrator() {
	int pop1 = popVar();
	int pop2 = popVar();

	if (!pop2)
		pop2 = currentScriptPtr->overlayNumber;

	narratorOvl = pop2;
	narratorIdx = pop1;

	return (0);
}

int16 Op_GetMouseX() {
	int16 dummy;
	int16 mouseX;
	int16 mouseY;
	int16 mouseButton;

	getMouseStatus(&dummy, &mouseX, &mouseButton, &mouseY);

	return (mouseX);
}

int16 Op_GetMouseY() {
	int16 dummy;
	int16 mouseX;
	int16 mouseY;
	int16 mouseButton;

	getMouseStatus(&dummy, &mouseX, &mouseButton, &mouseY);

	return (mouseY);
}

int16 Op_Random() {
	int var = popVar();

	if (var < 2) {
		return (0);
	}

	return (_vm->_rnd.getRandomNumber(var - 1));
}

int16 Op_PlayFX() {
	int volume = popVar();
	int speed = popVar();
	/*int channelNum = */popVar();
	int sampleNum = popVar();

	if ((sampleNum >= 0) && (sampleNum < NUM_FILE_ENTRIES) && (filesDatabase[sampleNum].subData.ptr)) {
		if (speed == -1)
			speed = filesDatabase[sampleNum].subData.transparency;

		_vm->sound().playSound(filesDatabase[sampleNum].subData.ptr,
			filesDatabase[sampleNum].width, volume);
	}

	return (0);
}

int16 Op_LoopFX() {
	int volume = popVar();
	int speed = popVar();
	/*int channelNum = */popVar();
	int sampleNum = popVar();

	if ((sampleNum >= 0) && (sampleNum < NUM_FILE_ENTRIES) && (filesDatabase[sampleNum].subData.ptr)) {
		if (speed == -1)
			speed = filesDatabase[sampleNum].subData.transparency;

		_vm->sound().playSound(filesDatabase[sampleNum].subData.ptr,
			filesDatabase[sampleNum].width, volume);
	}

	return (0);
}

int16 Op_StopFX() {
	int channelNum = popVar();

	if (channelNum == -1) {
		_vm->sound().stopChannel(0);
		_vm->sound().stopChannel(1);
		_vm->sound().stopChannel(2);
		_vm->sound().stopChannel(3);
	} else {
		_vm->sound().stopChannel(channelNum);
	}

	return 0;
}

int16 Op_FreqFX() {
	int volume = popVar();
	int freq2 = popVar();
	int channelNum = popVar();
	int sampleNum = popVar();

	if ((sampleNum >= 0) && (sampleNum < NUM_FILE_ENTRIES) && (filesDatabase[sampleNum].subData.ptr)) {
		int freq = Period(freq2 * 1000);

		_vm->sound().startNote(channelNum, volume, freq);
	}

	return (0);
}

int16 Op_FreeCT() {
	freeCTP();
	return (0);
}

void freeObjectList(cellStruct *pListHead) {
	int var_2 = 0;
	cellStruct *pCurrent = pListHead->next;

	while (pCurrent) {
		cellStruct *pNext = pCurrent->next;

		if (pCurrent->freeze == 0) {
			if (pCurrent->gfxPtr)
				freeGfx(pCurrent->gfxPtr);
			MemFree(pCurrent);
		}

		var_2 = 1;

		pCurrent = pNext;
	}

	if (var_2) {
		resetPtr(pListHead);
	}
}

int16 Op_FreeCell() {
	freeObjectList(&cellHead);
	return (0);
}

int16 Op_freeBackgroundInscrustList() {
	freeBackgroundIncrustList(&backgroundIncrustHead);
	return (0);
}


int16 Op_UnmergeBackgroundIncrust() {
	int obj = popVar();
	int ovl = popVar();

	if (!ovl) {
		ovl = currentScriptPtr->overlayNumber;
	}

	unmergeBackgroundIncrust(&backgroundIncrustHead, ovl, obj);

	return (0);
}

int16 Op_FreePreload() {
	// TODO: See if this is needed
	debug(1, "Op_FreePreload not implemented");
	return (0);
}

int16 Op_RemoveMessage() {
	int idx;
	int overlay;

	idx = popVar();
	overlay = popVar();

	if (!overlay) {
		overlay = currentScriptPtr->overlayNumber;
	}

	removeCell(&cellHead, overlay, idx, 5, masterScreen);

	return (0);
}

int16 Op_FindSet() {
	int16 i;
	char name[36] = "";
	char *ptr;

	ptr = (char *) popPtr();

	if (!ptr) {
		return -1;
	}

	strcpy(name, ptr);
	strToUpper(name);

	for (i = 0; i < NUM_FILE_ENTRIES; i++) {
		if (!strcmp(name, filesDatabase[i].subData.name)) {
			return (i);
		}
	}

	return -1;
}

int16 Op_RemoveFrame() {
	int count = popVar();
	int start = popVar();

	resetFileEntryRange(start, count);

	return (0);
}

int16 Op_comment() {
	char *var;

	var = (char *)popPtr();

	debug(1, "COMMENT: \"%s\"", var);

	return (0);
}

int16 Op_RemoveProc() {
	int idx;
	int overlay;

	idx = popVar();
	overlay = popVar();

	if (!overlay) {
		overlay = currentScriptPtr->overlayNumber;
	}

	removeScript(overlay, idx, &procHead);

	return (0);
}

int16 Op_FreeOverlay() {
	char localName[36] = "";
	char *namePtr;

	namePtr = (char *) popPtr();

	strcpy(localName, namePtr);

	if (localName[0]) {
		strToUpper(localName);
		releaseOverlay((char *)localName);
	}

	return 0;
}

int16 Op_FindProc() {
	char name[36] = "";
	char *ptr;
	int param;

	ptr = (char *)popPtr();

	strcpy(name, ptr);

	param = getProcParam(popVar(), 20, name);

	return param;
}

int16 Op_GetRingWord() {
	// Original method had a ringed queue allowing this method to return words one at a time.
	// But it never seemed to be used; no entries were ever added to the list
	return 0;
}

int16 Op_KillMenu() {
	// Free menus, if active
	if (menuTable[0]) {
		freeMenu(menuTable[0]);
		menuTable[0] = NULL;
		currentActiveMenu = -1;
	}

	if (menuTable[1]) {
		freeMenu(menuTable[1]);
		menuTable[1] = NULL;
		currentActiveMenu = -1;
	}

	// Free the message list
//	if (linkedMsgList) freeMsgList(linkedMsgList);
	linkedMsgList = NULL;
	linkedRelation = NULL;

	return 0;
}

int16 Op_UserMenu() {
	int oldValue = playerMenuEnabled;
	playerMenuEnabled = popVar();

	return oldValue;
}

int16 Op_UserOn() {
	int oldValue = userEnabled;
	int newValue = popVar();

	if (newValue != -1) {
		userEnabled = newValue;
	}

	return oldValue;
}

int16 Op_Display() {
	int oldValue = displayOn;
	int newValue = popVar();

	if (newValue != -1) {
		displayOn = newValue;
	}

	return oldValue;
}

int16 Op_FreezeParent() {
	if (currentScriptPtr->var1A == 20) {
		changeScriptParamInList(currentScriptPtr->var18, currentScriptPtr->var16, &procHead, -1, 9997);
	} else if (currentScriptPtr->var1A == 30) {
		changeScriptParamInList(currentScriptPtr->var18, currentScriptPtr->var16, &relHead, -1, 9997);
	}

	return 0;
}

int16 Op_LoadBackground() {
	int result = 0;
	char bgName[36] = "";
	char *ptr;
	int bgIdx;

	ptr = (char *) popPtr();

	strcpy(bgName, ptr);

	bgIdx = popVar();

	if (bgIdx >= 0 || bgIdx < NBSCREENS) {
		strToUpper(bgName);

		gfxModuleData_gfxWaitVSync();
		gfxModuleData_gfxWaitVSync();

		result = loadBackground(bgName, bgIdx);

		gfxModuleData_addDirtyRect(Common::Rect(0, 0, 320, 200));
	}

	changeCursor(CURSOR_NORMAL);

	return result;
}

int16 Op_FrameExist() {
	int param;

	param = popVar();

	if (param < 0 || param > 255) {
		return 0;
	}

	if (filesDatabase[param].subData.ptr) {
		return 1;
	}

	return 0;
}

int16 Op_LoadFrame() {
	int param1;
	int param2;
	int param3;
	char name[36] = "";
	char *ptr;

	ptr = (char *) popPtr();

	strcpy(name, ptr);

	param1 = popVar();
	param2 = popVar();
	param3 = popVar();

	if (param3 >= 0 || param3 < NUM_FILE_ENTRIES) {
		strToUpper(name);

		gfxModuleData_gfxWaitVSync();
		gfxModuleData_gfxWaitVSync();

		lastAni[0] = 0;

		loadFileRange(name, param2, param3, param1);

		lastAni[0] = 0;
	}

	changeCursor(CURSOR_NORMAL);
	return 0;
}

int16 Op_LoadAbs() {
	int slot;
	char name[36] = "";
	char *ptr;
	int result = 0;

	ptr = (char *) popPtr();
	slot = popVar();

	if ((slot >= 0) && (slot < NUM_FILE_ENTRIES)) {
		strcpy(name, ptr);
		strToUpper(name);

		gfxModuleData_gfxWaitVSync();
		gfxModuleData_gfxWaitVSync();

		result = loadFullBundle(name, slot);
	}

	changeCursor(CURSOR_NORMAL);
	return result;
}

int16 Op_InitializeState() {
	int param1 = popVar();
	int objIdx = popVar();
	int ovlIdx = popVar();

	if (!ovlIdx)
		ovlIdx = currentScriptPtr->overlayNumber;

#ifdef FUNCTION_DEBUG
	debug(1, "Init %s state to %d", getObjectName(objIdx, overlayTable[ovlIdx].ovlData->arrayNameObj), param1);
#endif

	objInit(ovlIdx, objIdx, param1);

	return (0);
}

int16 Op_GetlowMemory() {
	return lowMemory;
}

int16 Op_AniDir() {
	int type = popVar();
	int objIdx = popVar();
	int ovlIdx = popVar();

	if (!ovlIdx)
		ovlIdx = currentScriptPtr->overlayNumber;

	actorStruct *pActor = findActor(&actorHead, ovlIdx, objIdx, type);
	if (pActor)
		return pActor->startDirection;

	return -1;
}

int16 Op_FadeOut() {
	for (long int i = 0; i < 256; i += 32) {
		for (long int j = 0; j < 256; j++) {
			int offsetTable[3];
			offsetTable[0] = -32;
			offsetTable[1] = -32;
			offsetTable[2] = -32;
			calcRGB(&workpal[3*j], &workpal[3*j], offsetTable);
		}
		gfxModuleData_setPal256(workpal);
		gfxModuleData_flipScreen();
	}

	memset(globalScreen, 0, 320 * 200);
	flip();

	fadeFlag = 1;
	PCFadeFlag = 1;

	return 0;
}

int16 isOverlayLoaded(const char * name) {
	int16 i;

	for (i = 1; i < numOfLoadedOverlay; i++) {
		if (!strcmp(overlayTable[i].overlayName, name) && overlayTable[i].alreadyLoaded) {
			return i;
		}
	}

	return 0;
}

int16 Op_FindOverlay() {
	char name[36] = "";
	char *ptr;

	ptr = (char *) popPtr();

	strcpy(name, ptr);
	strToUpper(name);

	return (isOverlayLoaded(name));
}

int16 Op_WriteObject() {
	int16 returnParam;

	int16 param1 = popVar();
	int16 param2 = popVar();
	int16 param3 = popVar();
	int16 param4 = popVar();

	getSingleObjectParam(param4, param3, param2, &returnParam);
	setObjectPosition(param4, param3, param2, param1);

	return returnParam;
}

int16 Op_ReadObject() {
	int16 returnParam;

	int member = popVar();
	int obj = popVar();
	int ovl = popVar();

	getSingleObjectParam(ovl, obj, member, &returnParam);

	return returnParam;
}

int16 Op_FadeIn() {
	doFade = 1;
	return 0;
}

int16 Op_GetMouseButton() {
	int16 dummy;
	int16 mouseX;
	int16 mouseY;
	int16 mouseButton;

	getMouseStatus(&dummy, &mouseX, &mouseButton, &mouseY);

	return mouseButton;
}

int16 Op_AddCell() {
	int16 objType = popVar();
	int16 objIdx = popVar();
	int16 overlayIdx = popVar();

	if (!overlayIdx)
		overlayIdx = currentScriptPtr->overlayNumber;

	addCell(&cellHead, overlayIdx, objIdx, objType, masterScreen, currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, currentScriptPtr->type);

	return 0;
}

int16 Op_AddBackgroundIncrust() {

	int16 objType = popVar();
	int16 objIdx = popVar();
	int16 overlayIdx = popVar();

	if (!overlayIdx)
		overlayIdx = currentScriptPtr->overlayNumber;

	addBackgroundIncrust(overlayIdx, objIdx, &backgroundIncrustHead, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, masterScreen, objType);

	return 0;
}

int16 Op_RemoveCell() {
	int objType = popVar();
	int objectIdx = popVar();
	int ovlNumber = popVar();

	if (!ovlNumber) {
		ovlNumber = currentScriptPtr->overlayNumber;
	}

	removeCell(&cellHead, ovlNumber, objectIdx, objType, masterScreen);

	return 0;
}

int16 fontFileIndex = -1;

int16 Op_SetFont() {
	fontFileIndex = popVar();

	return 0;
}

int16 Op_UnfreezeParent() {
	if (currentScriptPtr->var1A == 0x14) {
		changeScriptParamInList(currentScriptPtr->var18, currentScriptPtr->var16, &procHead, -1, 0);
	} else if (currentScriptPtr->var1A == 0x1E) {
		changeScriptParamInList(currentScriptPtr->var18, currentScriptPtr->var16, &relHead, -1, 0);
	}

	return 0;
}

int16 Op_ProtectionFlag() {
	int16 temp = protectionCode;
	int16 newVar;

	newVar = popVar();
	if (newVar != -1) {
		protectionCode = newVar;
	}

	return temp;
}

int16 Op_ClearScreen() {
	int bgIdx = popVar();

	if ((bgIdx >= 0) && (bgIdx < NBSCREENS) && (backgroundScreens[bgIdx])) {
		memset(backgroundScreens[bgIdx], 0, 320 * 200);
		backgroundChanged[bgIdx] = true;
		strcpy(backgroundTable[0].name, "");
	}

	return 0;
}

int16 Op_AddMessage() {
	int16 color = popVar();
	int16 var_2 = popVar();
	int16 var_4 = popVar();
	int16 var_6 = popVar();
	int16 var_8 = popVar();
	int16 overlayIdx = popVar();

	if (!overlayIdx)
		overlayIdx = currentScriptPtr->overlayNumber;

	if (color == -1) {
		color = findHighColor();
	} else {
		if (CVTLoaded) {
			color = cvtPalette[color];
		}
	}

	createTextObject(&cellHead, overlayIdx, var_8, var_6, var_4, var_2, color, masterScreen, currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber);

	return 0;
}

int16 Op_Preload() {
	popPtr();
	popVar();

	return 0;
}

int16 Op_LoadCt() {
	return initCt((const char *)popPtr());
}

int16 Op_EndAnim() {
	int param1 = popVar();
	int param2 = popVar();
	int overlay = popVar();

	if (!overlay)
		overlay = currentScriptPtr->overlayNumber;

	return isAnimFinished(overlay, param2, &actorHead, param1);
}

int16 Op_Protect() {
	popPtr();
	popVar();

	return 0;
}

int16 Op_AutoCell() {
	cellStruct *pObject;

	int signal = popVar();
	int loop = popVar();
	int wait = popVar();
	int animStep = popVar();
	int end = popVar();
	int start = popVar();
	int type = popVar();
	int change = popVar();
	int obj = popVar();
	int overlay = popVar();

	if (!overlay)
		overlay = currentScriptPtr->overlayNumber;

	pObject = addCell(&cellHead, overlay, obj, 4, masterScreen, currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, currentScriptPtr->type);

	if (!pObject)
		return 0;

	pObject->animSignal = signal;
	pObject->animLoop = loop;
	pObject->animWait = wait;
	pObject->animStep = animStep;
	pObject->animEnd = end;
	pObject->animStart = start;
	pObject->animType = type;
	pObject->animChange = change;

	if (type) {
		if (currentScriptPtr->type == scriptType_PROC) {
			changeScriptParamInList(currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, &procHead, -1, 9996);
		} else if (currentScriptPtr->type == scriptType_REL) {
			changeScriptParamInList(currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, &relHead, -1, 9996);
		}
	}

	if (change == 5) {
		objInit(pObject->overlay, pObject->idx, start);
	} else {
		setObjectPosition(pObject->overlay, pObject->idx, pObject->animChange, start);
	}

	if (wait < 0) {
		objectParamsQuery params;

		getMultipleObjectParam(overlay, obj, &params);
		pObject->animCounter = params.state2 - 1;
	}

	return 0;
}

int16 Op_Sizeof() {
	objectParamsQuery params;
	int index = popVar();
	int overlay = popVar();

	if (!overlay)
		overlay = currentScriptPtr->overlayNumber;

	getMultipleObjectParam(overlay, index, &params);

	return params.nbState - 1;
}

int16 Op_SetActiveBackground() {
	int currentPlane = masterScreen;
	int newPlane = popVar();

	if (newPlane >= 0 && newPlane < NBSCREENS) {
		if (backgroundScreens[newPlane]) {
			masterScreen = newPlane;
			backgroundChanged[newPlane] = true;
			switchPal = 1;
		}
	}

	return currentPlane;
}

int16 Op_RemoveBackground() {
	int backgroundIdx = popVar();

	if (backgroundIdx > 0 && backgroundIdx < 8) {
		if (backgroundScreens[backgroundIdx])
			MemFree(backgroundScreens[backgroundIdx]);

		if (masterScreen == backgroundIdx) {
			masterScreen = 0;
			backgroundChanged[0] = true;
		}

		strcpy(backgroundTable[backgroundIdx].name, "");
	} else {
		strcpy(backgroundTable[0].name, "");
	}

	return (0);
}

int vblLimit;

int16 Op_VBL() {
	vblLimit = popVar();
	return 0;
}

int op7BVar = 0;

int16 Op_Sec() {
	int di = popVar();
	int si = 1 - op7BVar;
	int sign;

	if (di) {
		sign = di / (ABS(di));
	} else {
		sign = 0;
	}

	op7BVar = -sign;

	return si;
}

int16 Op_RemoveBackgroundIncrust() {
	int idx = popVar();
	int overlay = popVar();

	if (!overlay) {
		overlay = currentScriptPtr->overlayNumber;
	}

	removeBackgroundIncrust(overlay, idx, &backgroundIncrustHead);

	return 0;
}

int16 Op_SetColor()	{
	int colorB = popVar();
	int colorG = popVar();
	int colorR = popVar();
	int endIdx = popVar();
	int startIdx = popVar();

	int i;

#define convertRatio 36.571428571428571428571428571429

	for (i = startIdx; i <= endIdx; i++) {
		int offsetTable[3];

		offsetTable[0] = (int)(colorR * convertRatio);
		offsetTable[1] = (int)(colorG * convertRatio);
		offsetTable[2] = (int)(colorB * convertRatio);

		if (CVTLoaded) {
			int colorIdx = cvtPalette[i];
			calcRGB(&palScreen[masterScreen][3*colorIdx], &workpal[3*colorIdx], offsetTable);
		} else {
			calcRGB(&palScreen[masterScreen][3*i], &workpal[3*i], offsetTable);
		}
	}

	gfxModuleData_setPalEntries(workpal, 0, 32);

	return 0;
}

int16 Op_Inventory() {
	int si = var41;

	var41 = popVar();

	return si;
}

int16 Op_RemoveOverlay() {
	int overlayIdx;

	overlayIdx = popVar();

	if (strlen(overlayTable[overlayIdx].overlayName)) {
		releaseOverlay(overlayTable[overlayIdx].overlayName);
	}

	return 0;
}

int16 Op_ComputeLine() {
	int y2 = popVar();
	int x2 = popVar();
	int y1 = popVar();
	int x1 = popVar();

	point* pDest = (point *)popPtr();

	int maxValue = cor_droite(x1, y1, x2, y2, pDest);

	flipGen(pDest, maxValue * 4);

	return maxValue;
}

int16 Op_FindMsg() {
	int si = popVar();
	popVar();

	return si;
}

int16 Op_SetZoom() {
	var46 = popVar();
	var45 = popVar();
	var42 = popVar();
	var39 = popVar();
	return 0;
}

int16 computeZoom(int param) {
	return (((param - var46) * (var39 - var42)) / (var45 - var46)) + var42;
}

int16 subOp23(int param1, int param2) {
	return (param1 * param2) >> 8;
}

int16 Op_GetStep() {
	int si = popVar();
	int dx = popVar();

	return subOp23(dx, si);
}

int16 Op_GetZoom() {
	return (computeZoom(popVar()));
}

actorStruct *addAnimation(actorStruct * pHead, int overlay, int objIdx, int param, int param2) {
	actorStruct *pPrevious = pHead;
	actorStruct *pCurrent = pHead->next;

	// go to the end of the list
	while (pCurrent) {
		pPrevious = pCurrent;
		pCurrent = pPrevious->next;
	}

	if (pCurrent && (pCurrent->overlayNumber == overlay)
	        && (pCurrent->idx == objIdx) && (pCurrent->type == param2)) {
		return NULL;
	}

	actorStruct *pNewElement = (actorStruct *) MemAlloc(sizeof(actorStruct));
	if (!pNewElement)
		return NULL;

	memset(pNewElement, 0, sizeof(actorStruct));
	pNewElement->next = pPrevious->next;
	pPrevious->next = pNewElement;

	if (!pCurrent) {
		pCurrent = pHead;
	}

	pNewElement->prev = pCurrent->prev;
	pCurrent->prev = pNewElement;

	pNewElement->idx = objIdx;
	pNewElement->type = param2;
	pNewElement->pathId = -1;
	pNewElement->overlayNumber = overlay;
	pNewElement->startDirection = param;
	pNewElement->nextDirection = -1;
	pNewElement->stepX = 5;
	pNewElement->stepY = 2;
	pNewElement->phase = ANIM_PHASE_WAIT;
	pNewElement->flag = 0;
	pNewElement->freeze = 0;

	return pNewElement;
}

int removeAnimation(actorStruct * pHead, int overlay, int objIdx, int objType) {
	actorStruct* pl;
	actorStruct* pl2;
	actorStruct* pl3;
	actorStruct* pl4;

	int dir = 0;

	pl = pHead;
	pl2 = pl;
	pl = pl2->next;

	while (pl) {
		pl2 = pl;

		if (((pl->overlayNumber == overlay) || (overlay == -1)) &&
		        ((pl->idx == objIdx) || (objIdx == -1)) &&
		        ((pl->type == objType) || (objType == -1))) {
			pl->type = -1;
		}

		pl = pl2->next;
	}

	pl = pHead;
	pl2 = pl;
	pl = pl2->next;

	while (pl) {
		if (pl->type == -1) {
			pl4 = pl->next;
			pl2->next = pl4;
			pl3 = pl4;

			if (pl3 == NULL)
				pl3 = pHead;

			pl3->prev = pl->prev;

			dir = pl->startDirection;

			if (pl->pathId >= 0)
				freePerso(pl->pathId);

			MemFree(pl);
			pl = pl4;
		} else {
			pl2 = pl;
			pl = pl2->next;
		}
	}

	return dir;
}

int flag_obstacle;		// numPolyBis

// add animation
int16 Op_AddAnimation() {
	int stepY = popVar();
	int stepX = popVar();
	int direction = popVar();
	int start = popVar();
	int type = popVar();
	int obj = popVar();
	int overlay = popVar();

	if (!overlay) {
		overlay = currentScriptPtr->overlayNumber;
	}

	if (direction >= 0 && direction <= 3) {
		actorStruct *si;

		si = addAnimation(&actorHead, overlay, obj, direction, type);

		if (si) {
			objectParamsQuery params;

			getMultipleObjectParam(overlay, obj, &params);

			si->x = params.X;
			si->y = params.Y;
			si->x_dest = -1;
			si->y_dest = -1;
			si->endDirection = -1;
			si->start = start;
			si->stepX = stepX;
			si->stepY = stepY;

			int newFrame = ABS(actor_end[direction][0]) - 1;

			int zoom = computeZoom(params.Y);

			if (actor_end[direction][0] < 0) {
				zoom = -zoom;
			}

			getPixel(params.X, params.Y);

			setObjectPosition(overlay, obj, 3, newFrame + start);
			setObjectPosition(overlay, obj, 4, zoom);
			setObjectPosition(overlay, obj, 5, numPoly);

			animationStart = false;
		}
	}

	return 0;
}

int16 Op_RemoveAnimation() {
	int objType = popVar();
	int objIdx = popVar();
	int ovlIdx = popVar();

	if (!ovlIdx) {
		ovlIdx = currentScriptPtr->overlayNumber;
	}

	return removeAnimation(&actorHead, ovlIdx, objIdx, objType);
}

int16 Op_regenerateBackgroundIncrust() {
	regenerateBackgroundIncrust(&backgroundIncrustHead);
	return 0;
}

int16 Op_SetStringColors() {
	// TODO: here ignore if low color mode

	subColor = (uint8) popVar();
	itemColor = (uint8) popVar();
	selectColor = (uint8) popVar();
	titleColor = (uint8) popVar();

	return 0;
}

int16 Op_XClick() {
	int x = popVar();

	if (x != -1) {
		aniX = x;
		animationStart = true;
	}

	return aniX;
}

int16 Op_YClick() {
	int y = popVar();

	if (y != -1) {
		aniY = y;
		animationStart = true;
	}

	return aniY;
}

int16 Op_GetPixel() {
	int x = popVar();
	int y = popVar();

	getPixel(x, y);
	return numPoly;
}

int16 Op_TrackAnim() {		// setup actor position
	actorStruct *pActor;

	int var0 = popVar();
	int actorY = popVar();
	int actorX = popVar();
	int var1 = popVar();
	int var2 = popVar();
	int overlay = popVar();

	if (!overlay) {
		overlay = currentScriptPtr->overlayNumber;
	}

	pActor = findActor(&actorHead, overlay, var2, var1);

	if (!pActor) {
		return 1;
	}

	animationStart = false;

	pActor->x_dest = actorX;
	pActor->y_dest = actorY;
	pActor->flag = 1;
	pActor->endDirection = var0;

	return 0;
}

int16 Op_BgName() {
	char* bgName = (char *)popPtr();
	int bgIdx = popVar();

	if ((bgIdx >= 0) && (bgIdx < NBSCREENS) && bgName) {
		strcpy(bgName, backgroundTable[bgIdx].name);

		if (strlen(bgName))
			return 1;

		return 0;
	}

	return 0;
}

int16 Op_LoadSong() {
	const char *ptr = (const char *)popPtr();
	char buffer[33];

	strcpy(buffer, ptr);
	strToUpper(buffer);
	_vm->sound().loadMusic(buffer);

	changeCursor(CURSOR_NORMAL);
	return 0;
}

int16 Op_PlaySong() {
	if (_vm->sound().songLoaded() && !_vm->sound().songPlayed())
		_vm->sound().playMusic();

	return 0;
}

int16 Op_StopSong() {
	if (_vm->sound().isPlaying())
		_vm->sound().stopMusic();

	return 0;
}

int16 Op_RestoreSong() {
	// Used in the original to restore the contents of a song. Doesn't seem to be used,
	// since the backup buffer it uses is never set
	return 0;
}

int16 Op_SongSize() {
	int size, oldSize;

	if (_vm->sound().songLoaded()) {
		oldSize = _vm->sound().numOrders();

		size = popVar();
		if ((size >= 1) && (size < 128))
			_vm->sound().setNumOrders(size);
	} else
		oldSize = 0;

	return oldSize;
}

int16 Op_SetPattern() {
	int value = popVar();
	int offset = popVar();

	if (_vm->sound().songLoaded()) {
		_vm->sound().setPattern(offset, value);
	}

	return 0;
}

int16 Op_FadeSong() {
	_vm->sound().fadeSong();

	return 0;
}

int16 Op_FreeSong() {
	_vm->sound().stopMusic();
	_vm->sound().removeMusic();
	return 0;
}

int16 Op_SongLoop() {
	bool oldLooping = _vm->sound().musicLooping();
	_vm->sound().musicLoop(popVar() != 0);

	return oldLooping;
}

int16 Op_SongPlayed() {
	return _vm->sound().songPlayed();
}

void setVar49Value(int value) {
	flagCt = value;
}

int16 Op_CTOn() {
	setVar49Value(1);
	return 0;
}

int16 Op_CTOff() {
	setVar49Value(0);
	return 0;
}

int16 Op_FreezeOverlay() {
	//int var0;
	//int var1;
	int temp;

	int var0 = popVar();
	int var1 = popVar();

	if (!var1) {
		var1 = currentScriptPtr->overlayNumber;
	}

	temp = overlayTable[var1].executeScripts;
	overlayTable[var1].executeScripts = var0;

	return temp;
}

int16 Op_FreezeCell() {
	int newFreezz = popVar();
	int oldFreeze = popVar();
	int backgroundPlante = popVar();
	int objType = popVar();
	int objIdx = popVar();
	int overlayIdx = popVar();

	if (!overlayIdx) {
		overlayIdx = currentScriptPtr->overlayNumber;
	}

	freezeCell(&cellHead, overlayIdx, objIdx, objType, backgroundPlante, oldFreeze, newFreezz);

	return 0;
}

void Op_60Sub(int overlayIdx, actorStruct * pActorHead, int _var0, int _var1, int _var2, int _var3) {
	actorStruct *pActor = findActor(pActorHead, overlayIdx, _var0, _var3);

	if (pActor) {
		if ((pActor->freeze == _var2) || (_var2 == -1)) {
			pActor->freeze = _var1;
		}
	}
}

int16 Op_FreezeAni() {
	/*
	 * int var0;
	 * int var1;
	 * int var2;
	 * int var3;
	 * int var4;
	 */

	int var0 = popVar();
	int var1 = popVar();
	int var2 = popVar();
	int var3 = popVar();
	int var4 = popVar();

	if (!var4) {
		var4 = currentScriptPtr->overlayNumber;
	}

	Op_60Sub(var4, &actorHead, var3, var0, var1, var2);

	return 0;
}

int16 Op_Itoa() {
	int nbp = popVar();
	int param[160];
	char txt[40];
	char format[30];
	char nbf[20];

	for (int i = nbp - 1; i >= 0; i--)
		param[i] = popVar();

	int val = popVar();
	char* pDest = (char *)popPtr();

	if (!nbp)
		sprintf(txt, "%d", val);
	else {
		strcpy(format, "%");
		sprintf(nbf, "%d", param[0]);
		strcat(format, nbf);
		strcat(format, "d");
		sprintf(txt, format, val);
	}

	for (int i = 0; txt[i]; i++)
		*(pDest++) = txt[i];
	*(pDest++) = '\0';

	return 0;
}

int16 Op_Strcat() {
	char *pSource = (char *)popPtr();
	char *pDest = (char *)popPtr();

	while (*pDest)
		pDest++;

	while (*pSource)
		*(pDest++) = *(pSource++);
	*(pDest++) = '\0';

	return 0;
}

int16 Op_FindSymbol() {
	int var0 = popVar();
	char *ptr = (char *)popPtr();
	int var1 = popVar();

	if (!var1)
		var1 = currentScriptPtr->overlayNumber;

	return getProcParam(var1, var0, ptr);
}

int16 Op_FindObject() {
	char var_26[36];
	char *ptr = (char *)popPtr();
	int overlayIdx;

	var_26[0] = 0;

	if (ptr) {
		strcpy(var_26, ptr);
	}

	overlayIdx = popVar();

	if (!overlayIdx)
		overlayIdx = currentScriptPtr->overlayNumber;

	return getProcParam(overlayIdx, 40, var_26);
}

int16 Op_SetObjectAtNode() {
	int16 node = popVar();
	int16 obj = popVar();
	int16 ovl = popVar();

	if (!ovl)
		ovl = currentScriptPtr->overlayNumber;

	int nodeInfo[2];

	if (!getNode(nodeInfo, node)) {
		setObjectPosition(ovl, obj, 0, nodeInfo[0]);
		setObjectPosition(ovl, obj, 1, nodeInfo[1]);
		setObjectPosition(ovl, obj, 2, nodeInfo[1]);
		setObjectPosition(ovl, obj, 4, computeZoom(nodeInfo[1]));
	}

	return 0;
}

int16 Op_GetNodeX() {
	int16 node = popVar();

	int nodeInfo[2];

	int result = getNode(nodeInfo, node);

	ASSERT(result == 0);

	return nodeInfo[0];
}

int16 Op_GetNodeY() {
	int16 node = popVar();

	int nodeInfo[2];

	int result = getNode(nodeInfo, node);

	ASSERT(result == 0);

	return nodeInfo[1];
}

int16 Op_SetVolume() {
	int oldVolume = _vm->sound().getVolume();
	int newVolume = popVar();

	if (newVolume > 63) newVolume = 63;
	if (newVolume >= 0) {
		int volume = 63 - newVolume;
		_vm->sound().setVolume(volume);
	}

	return oldVolume >> 2;
}

int16 Op_SongExist() {
	const char *songName = (char *)popPtr();

	if (songName) {
		char name[33];
		strcpy(name, songName);
		strToUpper(name);

		if (!strcmp(_vm->sound().musicName(), name))
			return 1;
	}

	return 0;
}

int16 Op_TrackPos() {
	// This function returns a variable that never seems to change from 0
	return 0;
}

int16 Op_SetNodeState() {
	int16 state = popVar();
	int16 node = popVar();

	return setNodeState(node, state);
}

int16 Op_SetNodeColor() {
	int16 color = popVar();
	int16 node = popVar();

	return setNodeColor(node, color);
}

int16 Op_SetXDial() {
	int16 old = xdial;
	xdial = popVar();

	return old;
}

int16 Op_DialogOn() {
	dialogueObj = popVar();
	dialogueOvl = popVar();

	if (dialogueOvl == 0)
		dialogueOvl = currentScriptPtr->overlayNumber;

	dialogueEnabled = true;

	return 0;
}

int16 Op_DialogOff() {
	dialogueEnabled = false;

	objectReset();

	if (menuTable[0]) {
		freeMenu(menuTable[0]);
		menuTable[0] = NULL;
		changeCursor(CURSOR_NORMAL);
		currentActiveMenu = -1;
	}

	return 0;
}

int16 Op_LinkObjects() {
	int type = popVar();
	int obj2 = popVar();
	int ovl2 = popVar();
	int obj = popVar();
	int ovl = popVar();

	if (!ovl)
		ovl = currentScriptPtr->overlayNumber;
	if (!ovl2)
		ovl2 = currentScriptPtr->overlayNumber;

	linkCell(&cellHead, ovl, obj, type, ovl2, obj2);

	return 0;
}

int16 Op_UserClick() {
	sysKey = popVar();
	sysY = popVar();
	sysX = popVar();

	return 0;
}

int16 Op_XMenuItem() {
	int index = popVar();
	int count = 0;

	if (!menuTable[0] || (menuTable[0]->numElements == 0))
		return 0;

	menuElementStruct *p = menuTable[0]->ptrNextElement;

	while (p) {
		if (count == index)
			return p->x + 1;

		++count;
		p = p->next;
	}

	return 0;
}

int16 Op_YMenuItem() {
	int index = popVar();
	int count = 0;

	if (!menuTable[0] || (menuTable[0]->numElements == 0))
		return 0;

	menuElementStruct *p = menuTable[0]->ptrNextElement;

	while (p) {
		if (count == index)
			return p->y + 1;

		++count;
		p = p->next;
	}

	return 0;
}


int16 Op_Menu() {
	return (int16)(menuTable[0] != NULL);
}

int16 Op_AutoControl() {
	int oldValue = automaticMode;
	int newValue = popVar();

	if (newValue >= 0) {
		automaticMode = newValue;
		activeMouse = newValue;
	}

	return oldValue;
}

int16 Op_MouseMove() {
	int16 handle, button;
	Common::Point pt;

	getMouseStatus(&handle, &pt.x, &button, &pt.y);

	// x/y parameters aren't used
	popVar();
	popVar();

	return 0;
}

int16 Op_MouseEnd() {
	if (automoveInc < automoveMax)
		return (int16)false;

	return (int16)true;
}

int16 Op_MsgExist() {
	return isMessage;
}

int16 Op_UserDelay() {
	int delay = popVar();

	if (delay >= 0) {
		userDelay = delay;
	}

	return userDelay;
}

int16 Op_ThemeReset() {
	objectReset();

	return 0;
}

int16 Op_UserWait() {
	userWait = 1;
	if (currentScriptPtr->type == scriptType_PROC) {
		changeScriptParamInList(currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, &procHead, -1, 9999);
	} else if (currentScriptPtr->type == scriptType_REL) {
		changeScriptParamInList(currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, &relHead, -1, 9999);
	}

	return 0;
}

opcodeFunction opcodeTablePtr[] = {
	NULL, // 0x00
	Op_FadeIn,
	Op_FadeOut,
	Op_LoadBackground,
	Op_LoadAbs,
	Op_AddCell,
	Op_AddProc,
	Op_InitializeState,
	Op_RemoveCell,
	Op_FreeCell,
	Op_RemoveProc,
	Op_RemoveFrame,
	Op_LoadOverlay,
	Op_SetColor,
	Op_PlayFX,
	NULL,	// used to be debug

	Op_FreeOverlay, // 0x10
	Op_FindOverlay,
	NULL,	// used to be exec debug
	Op_AddMessage,
	Op_RemoveMessage,
	Op_UserWait,
	Op_FreezeCell,
	Op_LoadCt,
	Op_AddAnimation,
	Op_RemoveAnimation,
	Op_SetZoom,
	Op_SetObjectAtNode,
	Op_SetNodeState,
	Op_SetNodeColor,
	Op_TrackAnim,
	Op_GetNodeX,

	Op_GetNodeY, // 0x20
	Op_EndAnim,
	Op_GetZoom,
	Op_GetStep,
	Op_SetStringColors,
	Op_XClick,
	Op_YClick,
	Op_GetPixel,
	Op_UserOn,
	Op_FreeCT,
	Op_FindObject,
	Op_FindProc,
	Op_WriteObject,
	Op_ReadObject,
	Op_RemoveOverlay,
	Op_AddBackgroundIncrust,

	Op_RemoveBackgroundIncrust, // 0x30
	Op_UnmergeBackgroundIncrust,
	Op_freeBackgroundInscrustList,
	Op_DialogOn,
	Op_DialogOff,
	Op_UserDelay,
	Op_ThemeReset,
	Op_Narrator,
	Op_RemoveBackground,
	Op_SetActiveBackground,
	Op_CTOn,
	Op_CTOff,
	Op_Random,
	Op_LoadSong,
	Op_FadeSong,
	Op_PlaySong,

	Op_FreeSong, // 0x40
	Op_FrameExist,
	Op_SetVolume,
	Op_SongExist,
	Op_TrackPos,
	Op_StopSong,
	Op_RestoreSong,
	Op_SongSize,
	Op_SetPattern,
	Op_SongLoop,
	Op_SongPlayed,
	Op_LinkObjects,
	Op_UserClick,
	Op_XMenuItem,
	Op_YMenuItem,
	Op_Menu,

	Op_AutoControl, // 0x50
	Op_MouseMove,
	Op_MouseEnd,
	Op_MsgExist,
	Op_SetFont,
	NULL, // MergeMsg
	Op_Display,
	Op_GetMouseX,
	Op_GetMouseY,
	Op_GetMouseButton,
	Op_FindSet,
	Op_regenerateBackgroundIncrust,
	Op_BgName,
	Op_LoopFX,
	Op_StopFX,
	Op_FreqFX,

	Op_FreezeAni, // 0x60
	Op_FindMsg,
	Op_FreezeParent,
	Op_UnfreezeParent,
	Op_Exec,
	Op_AutoCell,
	Op_Sizeof,
	Op_Preload,
	Op_FreePreload,
	NULL, // DeletePreload
	Op_VBL,
	Op_LoadFrame,
	Op_FreezeOverlay,
	Op_Strcpy,
	Op_Strcat,
	Op_Itoa,

	Op_comment, // 0x70
	Op_ComputeLine,
	Op_FindSymbol,
	Op_SetXDial,
	Op_GetlowMemory,
	Op_AniDir,
	Op_Protect,
	Op_ClearScreen,
	Op_Inventory,
	Op_UserMenu,
	Op_GetRingWord,
	Op_Sec,
	Op_ProtectionFlag,
	Op_KillMenu,
};

int32 opcodeType8() {
	int opcode = getByteFromScript();

	if (!opcode)
		return (-21);

	if (opcode > 0x100)
		return (-21);

	if (opcode < ARRAYSIZE(opcodeTablePtr) && opcodeTablePtr[opcode]) {
		pushVar(opcodeTablePtr[opcode]());
		return (0);
	} else {
		warning("Unsupported opcode %d in opcode type 8", opcode);
		pushVar(0);
		// exit(1);
	}

	return 0;

}

} // End of namespace Cruise
