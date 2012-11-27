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

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/system.h"	// for g_system->getEventManager()
#include "common/textconsole.h"

#include "cruise/cruise.h"
#include "cruise/cruise_main.h"
#include "cruise/cell.h"
#include "cruise/staticres.h"

namespace Cruise {

enum RelationType {RT_REL = 30, RT_MSG = 50};

static int playerDontAskQuit;
unsigned int timer = 0;

gfxEntryStruct* linkedMsgList = NULL;

typedef CruiseEngine::MemInfo MemInfo;

void MemoryList() {
	if (!_vm->_memList.empty()) {
		debug("Current list of un-freed memory blocks:");
		Common::List<MemInfo *>::iterator i;
		for (i = _vm->_memList.begin(); i != _vm->_memList.end(); ++i) {
			MemInfo const *const v = *i;
			debug("%s - %d", v->fname, v->lineNum);
		}
	}
}

void *MemoryAlloc(uint32 size, bool clearFlag, int32 lineNum, const char *fname) {
	void *result;

	if (gDebugLevel > 0) {
		// Find the point after the final slash
		const char *fnameP = fname + strlen(fname);
		while ((fnameP > fname) && (*(fnameP - 1) != '/') && (*(fnameP - 1) != '\\'))
			--fnameP;

		// Create the new memory block and add it to the memory list
		MemInfo *const v = (MemInfo *)malloc(sizeof(MemInfo) + size);
		v->lineNum = lineNum;
		strncpy(v->fname, fnameP, sizeof(v->fname));
		v->fname[ARRAYSIZE(v->fname) - 1] = '\0';
		v->magic = MemInfo::cookie;

		// Add the block to the memory list
		_vm->_memList.push_back(v);
		result = v + 1;
	} else
		result = malloc(size);

	if (clearFlag)
		memset(result, 0, size);

	return result;
}

void MemoryFree(void *v) {
	if (!v)
		return;

	if (gDebugLevel > 0) {
		MemInfo *const p = (MemInfo *)v - 1;
		assert(p->magic == MemInfo::cookie);

		_vm->_memList.remove(p);
		free(p);
	} else
		free(v);
}

void drawBlackSolidBoxSmall() {
//  gfxModuleData.drawSolidBox(64,100,256,117,0);
	drawSolidBox(64, 100, 256, 117, 0);
}

void loadPackedFileToMem(int fileIdx, uint8 *buffer) {
	changeCursor(CURSOR_DISK);

	_vm->_currentVolumeFile.seek(volumePtrToFileDescriptor[fileIdx].offset, SEEK_SET);
	_vm->_currentVolumeFile.read(buffer, volumePtrToFileDescriptor[fileIdx].size);
}

int getNumObjectsByClass(int scriptIdx, int param) {
	objDataStruct *ptr2;
	int counter;
	int i;

	if (!overlayTable[scriptIdx].ovlData)
		return (0);

	ptr2 = overlayTable[scriptIdx].ovlData->arrayObject;

	if (!ptr2)
		return (0);

	if (overlayTable[scriptIdx].ovlData->numObj == 0)
		return (0);

	counter = 0;

	for (i = 0; i < overlayTable[scriptIdx].ovlData->numObj; i++) {
		if (ptr2[i]._class == param) {
			counter++;
		}
	}

	return (counter);
}

void resetFileEntryRange(int start, int count) {
	int i;

	for (i = 0; i < count; ++i)
		resetFileEntry(start + i);
}

int getProcParam(int overlayIdx, int param2, const char *name) {
	int numSymbGlob;
	int i;
	exportEntryStruct *arraySymbGlob;
	char *exportNamePtr;
	char exportName[80];

	if (!overlayTable[overlayIdx].alreadyLoaded)
		return 0;

	if (!overlayTable[overlayIdx].ovlData)
		return 0;

	numSymbGlob = overlayTable[overlayIdx].ovlData->numSymbGlob;
	arraySymbGlob = overlayTable[overlayIdx].ovlData->arraySymbGlob;
	exportNamePtr = overlayTable[overlayIdx].ovlData->arrayNameSymbGlob;

	if (!exportNamePtr)
		return 0;

	for (i = 0; i < numSymbGlob; i++) {
		if (arraySymbGlob[i].var4 == param2) {
			strcpy(exportName, arraySymbGlob[i].offsetToName + exportNamePtr);

			if (!strcmp(exportName, name)) {
				return (arraySymbGlob[i].idx);
			}
		}
	}

	return 0;
}

void changeScriptParamInList(int param1, int param2, scriptInstanceStruct *pScriptInstance, int oldFreeze, int newValue) {
	pScriptInstance = pScriptInstance->nextScriptPtr;
	while (pScriptInstance) {
		if ((pScriptInstance->overlayNumber == param1) || (param1 == -1))
			if ((pScriptInstance->scriptNumber == param2) || (param2 == -1))
				if ((pScriptInstance->freeze == oldFreeze) || (oldFreeze == -1)) {
					pScriptInstance->freeze = newValue;
				}

		pScriptInstance = pScriptInstance->nextScriptPtr;
	}
}

void initBigVar3() {
	int i;

	for (i = 0; i < NUM_FILE_ENTRIES; i++) {
		if (filesDatabase[i].subData.ptr) {
			MemFree(filesDatabase[i].subData.ptr);
		}

		filesDatabase[i].subData.ptr = NULL;
		filesDatabase[i].subData.ptrMask = NULL;

		filesDatabase[i].subData.index = -1;
		filesDatabase[i].subData.resourceType = 0;
	}
}

void resetPtr2(scriptInstanceStruct *ptr) {
	ptr->nextScriptPtr = NULL;
	ptr->scriptNumber = -1;
}

void resetActorPtr(actorStruct *ptr) {
	actorStruct *p = ptr;

	if (p->next) {
		p = p->next;
		do {
			actorStruct *pNext = p->next;
			MemFree(p);
			p = pNext;
		} while (p);
	}

	ptr->next = NULL;
	ptr->prev = NULL;
}

ovlData3Struct *getOvlData3Entry(int32 scriptNumber, int32 param) {
	ovlDataStruct *ovlData = overlayTable[scriptNumber].ovlData;

	if (!ovlData) {
		return NULL;
	}

	if (param < 0) {
		return NULL;
	}

	if (ovlData->numProc <= param) {
		return NULL;
	}

	if (!ovlData->arrayProc) {
		return NULL;
	}

	return (&ovlData->arrayProc[param]);
}

ovlData3Struct *scriptFunc1Sub2(int32 scriptNumber, int32 param) {
	ovlDataStruct *ovlData = overlayTable[scriptNumber].ovlData;

	if (!ovlData) {
		return NULL;
	}

	if (param < 0) {
		return NULL;
	}

	if (ovlData->numRel <= param) {
		return NULL;
	}

	if (!ovlData->ptr1) {
		return NULL;
	}

	return &ovlData->ptr1[param];
}

void scriptFunc2(int scriptNumber, scriptInstanceStruct * scriptHandle,
                 int param, int param2) {
	if (scriptHandle->nextScriptPtr) {
		if (scriptNumber == scriptHandle->nextScriptPtr->overlayNumber
		        || scriptNumber != -1) {
			if (param2 == scriptHandle->nextScriptPtr->scriptNumber
			        || param2 != -1) {
				scriptHandle->nextScriptPtr->sysKey = param;
			}
		}
	}
}

uint8 *getDataFromData3(ovlData3Struct *ptr, int param) {
	uint8 *dataPtr;

	if (!ptr)
		return (NULL);

	dataPtr = ptr->dataPtr;

	if (!dataPtr)
		return (NULL);

	switch (param) {
	case 0:
		return (dataPtr);
	case 1:
		return (dataPtr + ptr->offsetToSubData3);	// strings
	case 2:
		return (dataPtr + ptr->offsetToSubData2);
	case 3:
		return (dataPtr + ptr->offsetToImportData);	// import data
	case 4:
		return (dataPtr + ptr->offsetToImportName);	// import names
	case 5:
		return (dataPtr + ptr->offsetToSubData5);
	default:
		return NULL;
	}
}

void printInfoBlackBox(const char *string) {
}

void waitForPlayerInput() {
}

void getFileExtention(const char *name, char *buffer) {
	while (*name != '.' && *name) {
		name++;
	}

	strcpy(buffer, name);
}

void removeExtention(const char *name, char *buffer) {	// not like in original
	char *ptr;

	strcpy(buffer, name);

	ptr = strchr(buffer, '.');

	if (ptr)
		*ptr = 0;
}

int lastFileSize;

int loadFileSub1(uint8 **ptr, const char *name, uint8 *ptr2) {
	int i;
	char buffer[256];
	int fileIdx;
	int unpackedSize;
	uint8 *unpackedBuffer;

	for (i = 0; i < 64; i++) {
		if (preloadData[i].ptr) {
			if (!strcmp(preloadData[i].name, name)) {
				error("Unsupported code in loadFIleSub1");
			}
		}
	}

	getFileExtention(name, buffer);

	if (!strcmp(buffer, ".SPL")) {
		removeExtention(name, buffer);

		/* if (useH32)
		 *{
		 *	strcat(buffer, ".H32");
		 *}
		 * else
		 * if (useAdLib)
		 * { */
		 strcat(buffer,".ADL");
		/* }
		 * else
		 * {
		 * strcatuint8(buffer,".HP");
		 * } */
	} else {
		strcpy(buffer, name);
	}

	fileIdx = findFileInDisks(buffer);

	if (fileIdx < 0)
		return (-18);

	unpackedSize = loadFileVar1 = volumePtrToFileDescriptor[fileIdx].extSize + 2;

	unpackedBuffer = (uint8 *)mallocAndZero(unpackedSize);

	if (!unpackedBuffer) {
		return (-2);
	}

	lastFileSize = unpackedSize;

	if (volumePtrToFileDescriptor[fileIdx].size + 2 != unpackedSize) {
		uint8 *pakedBuffer = (uint8 *) mallocAndZero(volumePtrToFileDescriptor[fileIdx].size + 2);

		loadPackedFileToMem(fileIdx, pakedBuffer);

		uint32 realUnpackedSize = READ_BE_UINT32(pakedBuffer + volumePtrToFileDescriptor[fileIdx].size - 4);

		lastFileSize = realUnpackedSize;

		delphineUnpack(unpackedBuffer, pakedBuffer, volumePtrToFileDescriptor[fileIdx].size);

		MemFree(pakedBuffer);
	} else {
		loadPackedFileToMem(fileIdx, unpackedBuffer);
	}

	*ptr = unpackedBuffer;

	return (1);
}

void resetFileEntry(int32 entryNumber) {
	if (entryNumber >= NUM_FILE_ENTRIES)
		return;

	if (!filesDatabase[entryNumber].subData.ptr)
		return;

	MemFree(filesDatabase[entryNumber].subData.ptr);
	if (filesDatabase[entryNumber].subData.ptrMask)
		MemFree(filesDatabase[entryNumber].subData.ptrMask);

	filesDatabase[entryNumber].subData.ptr = NULL;
	filesDatabase[entryNumber].subData.ptrMask = NULL;
	filesDatabase[entryNumber].widthInColumn = 0;
	filesDatabase[entryNumber].width = 0;
	filesDatabase[entryNumber].resType = OBJ_TYPE_LINE;
	filesDatabase[entryNumber].height = 0;
	filesDatabase[entryNumber].subData.index = -1;
	filesDatabase[entryNumber].subData.resourceType = 0;
	filesDatabase[entryNumber].subData.compression = 0;
	filesDatabase[entryNumber].subData.name[0] = 0;
}

uint8 *mainProc14(uint16 overlay, uint16 idx) {
	ASSERT(0);

	return NULL;
}

void CruiseEngine::initAllData() {
	int i;

	setupFuncArray();
	initOverlayTable();

	stateID = 0;
	masterScreen = 0;

	freeDisk();

	soundList[0].frameNum = -1;
	soundList[1].frameNum = -1;
	soundList[2].frameNum = -1;
	soundList[3].frameNum = -1;

	menuTable[0] = NULL;

	for (i = 0; i < 2000; i++) {
		globalVars[i] = 0;
	}

	for (i = 0; i < 8; i++) {
		backgroundTable[i].name[0] = 0;
	}

	for (i = 0; i < NUM_FILE_ENTRIES; i++) {
		filesDatabase[i].subData.ptr = NULL;
		filesDatabase[i].subData.ptrMask = NULL;
	}

	initBigVar3();

	resetPtr2(&procHead);
	resetPtr2(&relHead);

	resetPtr(&cellHead);

	resetActorPtr(&actorHead);
	resetBackgroundIncrustList(&backgroundIncrustHead);

	bootOverlayNumber = loadOverlay("AUTO00");

#ifdef DUMP_SCRIPT
	loadOverlay("TITRE");
	loadOverlay("TOM");
	loadOverlay("XX2");
	loadOverlay("SUPER");
	loadOverlay("BEBE1");
	loadOverlay("BIBLIO");
	loadOverlay("BRACAGE");
	loadOverlay("CONVERS");
	loadOverlay("DAF");
	loadOverlay("DAPHNEE");
	loadOverlay("DESIRE");
	loadOverlay("FAB");
	loadOverlay("FABIANI");
	loadOverlay("FIN");
	loadOverlay("FIN01");
	loadOverlay("FINBRAC");
	loadOverlay("GEN");
	loadOverlay("GENDEB");
	loadOverlay("GIFLE");
	loadOverlay("HECTOR");
	loadOverlay("HECTOR2");
	loadOverlay("I00");
	loadOverlay("I01");
	loadOverlay("I04");
	loadOverlay("I06");
	loadOverlay("I07");
	loadOverlay("INVENT");
	loadOverlay("JULIO");
	loadOverlay("LOGO");
	loadOverlay("MANOIR");
	loadOverlay("MISSEL");
	loadOverlay("POKER");
	loadOverlay("PROJ");
	loadOverlay("REB");
	loadOverlay("REBECCA");
	loadOverlay("ROS");
	loadOverlay("ROSE");
	loadOverlay("S01");
	loadOverlay("S02");
	loadOverlay("S03");
	loadOverlay("S04");
	loadOverlay("S06");
	loadOverlay("S07");
	loadOverlay("S08");
	loadOverlay("S09");
	loadOverlay("S10");
	loadOverlay("S103");
	loadOverlay("S11");
	loadOverlay("S113");
	loadOverlay("S12");
	loadOverlay("S129");
	loadOverlay("S131");
	loadOverlay("S132");
	loadOverlay("S133");
	loadOverlay("int16");
	loadOverlay("S17");
	loadOverlay("S18");
	loadOverlay("S19");
	loadOverlay("S20");
	loadOverlay("S21");
	loadOverlay("S22");
	loadOverlay("S23");
	loadOverlay("S24");
	loadOverlay("S25");
	loadOverlay("S26");
	loadOverlay("S27");
	loadOverlay("S29");
	loadOverlay("S30");
	loadOverlay("S31");
	loadOverlay("int32");
	loadOverlay("S33");
	loadOverlay("S33B");
	loadOverlay("S34");
	loadOverlay("S35");
	loadOverlay("S36");
	loadOverlay("S37");
	loadOverlay("SHIP");
	loadOverlay("SUPER");
	loadOverlay("SUZAN");
	loadOverlay("SUZAN2");
	loadOverlay("TESTA1");
	loadOverlay("TESTA2");
	//exit(1);
#endif

	if (bootOverlayNumber) {
		positionInStack = 0;

		attacheNewScriptToTail(&procHead, bootOverlayNumber, 0, 20, 0, 0, scriptType_PROC);
		scriptFunc2(bootOverlayNumber, &procHead, 1, 0);
	}

	strcpy(lastOverlay, "AUTO00");

	_gameSpeed = GAME_FRAME_DELAY_1;
	_speedFlag = false;

	return;
}

int removeFinishedScripts(scriptInstanceStruct *ptrHandle) {
	scriptInstanceStruct *ptr = ptrHandle->nextScriptPtr;	// can't destruct the head
	scriptInstanceStruct *oldPtr = ptrHandle;

	if (!ptr)
		return (0);

	do {
		if (ptr->scriptNumber == -1) {
			oldPtr->nextScriptPtr = ptr->nextScriptPtr;

			if (ptr->data)
				MemFree(ptr->data);

			MemFree(ptr);

			ptr = oldPtr->nextScriptPtr;
		} else {
			oldPtr = ptr;
			ptr = ptr->nextScriptPtr;
		}
	} while (ptr);

	return (0);
}

void removeAllScripts(scriptInstanceStruct *ptrHandle) {
	scriptInstanceStruct *ptr = ptrHandle->nextScriptPtr;	// can't destruct the head
	scriptInstanceStruct *oldPtr = ptrHandle;

	if (!ptr)
		return;

	do {
		oldPtr->nextScriptPtr = ptr->nextScriptPtr;

		if (ptr->data)
			MemFree(ptr->data);

		MemFree(ptr);

		ptr = oldPtr->nextScriptPtr;
	} while (ptr);
}


bool testMask(int x, int y, unsigned char* pData, int stride) {
	unsigned char* ptr = y * stride + x / 8 + pData;

	unsigned char bitToTest = 0x80 >> (x & 7);

	if ((*ptr) & bitToTest)
		return true;
	return false;
}

int buttonDown;
int selectDown = 0;
int menuDown = 0;

int findObject(int mouseX, int mouseY, int *outObjOvl, int *outObjIdx) {
	char objectName[80];

	cellStruct *currentObject = cellHead.prev;

	while (currentObject) {
		if (currentObject->overlay > 0 && overlayTable[currentObject->overlay].alreadyLoaded &&
				(currentObject->type == OBJ_TYPE_SPRITE || currentObject->type == OBJ_TYPE_MASK ||
				currentObject->type == OBJ_TYPE_EXIT || currentObject->type == OBJ_TYPE_VIRTUAL)) {
			const char* pObjectName = getObjectName(currentObject->idx, overlayTable[currentObject->overlay].ovlData->arrayNameObj);

			strcpy(objectName, pObjectName);

			if (strlen(objectName) && (currentObject->freeze == 0)) {
				int objIdx = currentObject->idx;
				int objOvl = currentObject->overlay;
				int linkedObjIdx = currentObject->followObjectIdx;
				int linkedObjOvl = currentObject->followObjectOverlayIdx;

				objectParamsQuery params;
				getMultipleObjectParam(objOvl, objIdx, &params);

				int x2 = 0;
				int y2 = 0;
				int j2 = 0;

				if ((objOvl != linkedObjOvl) || (objIdx != linkedObjIdx)) {
					objectParamsQuery params2;
					getMultipleObjectParam(linkedObjOvl, linkedObjIdx, &params2);

					x2 = params2.X;
					y2 = params2.Y;
					j2 = params2.fileIdx;
				}

				if (params.state >= 0 && params.fileIdx >= 0) {
					if (currentObject->type == OBJ_TYPE_SPRITE || currentObject->type == OBJ_TYPE_MASK || currentObject->type == OBJ_TYPE_EXIT) {
						int x = params.X + x2;
						int y = params.Y + y2;
						int j = params.fileIdx;

						if (j >= 0) {
							j += j2;
						}

						if ((filesDatabase[j].subData.resourceType == OBJ_TYPE_POLY) && (filesDatabase[j].subData.ptr)) {
							int zoom = params.scale;

							int16* dataPtr = (int16 *)filesDatabase[j].subData.ptr;

							if (*dataPtr == 0) {
								int16 offset;
								int16 newX;
								int16 newY;

								dataPtr ++;

								offset = (int16)READ_BE_UINT16(dataPtr);
								dataPtr++;

								newX = (int16)READ_BE_UINT16(dataPtr);
								dataPtr++;

								newY = (int16)READ_BE_UINT16(dataPtr);
								dataPtr++;

								offset += j;

								if (offset >= 0) {
									if (filesDatabase[offset].resType == OBJ_TYPE_LINE &&
											filesDatabase[offset].subData.ptr) {
										dataPtr = (int16 *)filesDatabase[offset].subData.ptr;
									}
								}

								zoom = -zoom;
								x -= newX;
								y -= newY;
							}

							if (dataPtr && findPoly((char *)dataPtr, x, y, zoom, mouseX, mouseY)) {
								*outObjOvl = linkedObjOvl;
								*outObjIdx = linkedObjIdx;

								return (currentObject->type);
							}
						} else {
							// int numBitPlanes = filesDatabase[j].resType;

							int nWidth;
							int nHeight;

							nWidth = filesDatabase[j].width;
							nHeight = filesDatabase[j].height;

							int offsetX = mouseX - x;
							int offsetY = mouseY - y;

							if ((offsetX >= 0) && (offsetX < nWidth) && (offsetY >= 0) && (offsetY <= nHeight) && filesDatabase[j].subData.ptr) {
								if (testMask(offsetX, offsetY, filesDatabase[j].subData.ptrMask, filesDatabase[j].width / 8)) {
									*outObjOvl = linkedObjOvl;
									*outObjIdx = linkedObjIdx;
									return currentObject->type;
								}
							}
						}
					} else if (currentObject->type == OBJ_TYPE_VIRTUAL) {
						int x = params.X + x2;
						int y = params.Y + y2;
						int width = params.fileIdx;
						int height = params.scale;

						if ((mouseX >= x) && (mouseX <= x + width) && (mouseY >= y) && (mouseY <= y + height)) {
							*outObjOvl = linkedObjOvl;
							*outObjIdx = linkedObjIdx;

							return (currentObject->type);
						}
					}
				}
			}
		}

		currentObject = currentObject->prev;
	}

	*outObjOvl = 0;
	*outObjIdx = 0;

	return -1;
}

Common::KeyCode keyboardCode = Common::KEYCODE_INVALID;

void freeStuff2() {
	warning("implement freeStuff2");
}

void *allocAndZero(int size) {
	void *ptr;

	ptr = MemAlloc(size);
	memset(ptr, 0, size);

	return ptr;
}

void buildInventory(int X, int Y) {
	menuStruct *pMenu;

	pMenu = createMenu(X, Y, _vm->langString(ID_INVENTORY));
	menuTable[1] = pMenu;

	if (pMenu == NULL)
		return;

	int numObjectInInventory = 0;
	for (int i = 1; i < numOfLoadedOverlay; i++) {
		ovlDataStruct *pOvlData = overlayTable[i].ovlData;

		if (overlayTable[i].alreadyLoaded) {
			if (overlayTable[i].ovlData->arrayObject) {
				for (int j = 0; j < pOvlData->numObj; j++) {
					if (getObjectClass(i, j) != 3) {
						int16 returnVar;

						getSingleObjectParam(i, j, 5, &returnVar);

						if (returnVar < -1) {
							addSelectableMenuEntry(i, j, pMenu, 1, -1, getObjectName(j, pOvlData->arrayNameObj));
							numObjectInInventory++;
						}
					}
				}
			}
		}
	}

	if (numObjectInInventory == 0) {
		freeMenu(menuTable[1]);
		menuTable[1] = NULL;
	}
}

int currentMenuElementX;
int currentMenuElementY;
menuElementStruct *currentMenuElement;

menuElementSubStruct *getSelectedEntryInMenu(menuStruct *pMenu) {
	menuElementStruct *pMenuElement;

	if (pMenu == NULL) {
		return NULL;
	}

	if (pMenu->numElements == 0) {
		return NULL;
	}

	pMenuElement = pMenu->ptrNextElement;

	while (pMenuElement) {
		if (pMenuElement->selected) {
			currentMenuElementX = pMenuElement->x;
			currentMenuElementY = pMenuElement->y;
			currentMenuElement = pMenuElement;

			return pMenuElement->ptrSub;
		}

		pMenuElement = pMenuElement->next;
	}

	return NULL;
}

bool createDialog(int objOvl, int objIdx, int x, int y) {
	bool found = false;
	int testState1 = -1;
	int testState2 = -1;
	int j;
	int16 objectState;
	int16 objectState2;

	getSingleObjectParam(objOvl, objIdx, 5, &objectState);

	menuTable[0] = createMenu(x, y, _vm->langString(ID_SPEAK_ABOUT));

	for (j = 1; j < numOfLoadedOverlay; j++) {
		if (overlayTable[j].alreadyLoaded) {
			int idHeader = overlayTable[j].ovlData->numMsgRelHeader;

			for (int i = 0; i < idHeader; i++) {
				linkDataStruct* ptrHead = &overlayTable[j].ovlData->arrayMsgRelHeader[i];
				int thisOvl = ptrHead->obj1Overlay;

				if (!thisOvl) {
					thisOvl = j;
				}

				objDataStruct* pObject = getObjectDataFromOverlay(thisOvl, ptrHead->obj1Number);

				getSingleObjectParam(thisOvl, ptrHead->obj1Number, 5, &objectState2);

				if (pObject && (pObject->_class == THEME) && (objectState2 < -1)) {

					thisOvl = ptrHead->obj2Overlay;
					if (!thisOvl) {
						thisOvl = j;
					}

					if ((thisOvl == objOvl) && (ptrHead->obj2Number == objIdx)) {
						int verbeOvl = ptrHead->verbOverlay;
						int obj1Ovl = ptrHead->obj1Overlay;
						int obj2Ovl = ptrHead->obj2Overlay;

						if (!verbeOvl) verbeOvl = j;
						if (!obj1Ovl)  obj1Ovl = j;
						if (!obj2Ovl)  obj2Ovl = j;

						char verbe_name[80];

						verbe_name[0]	= 0;

						ovlDataStruct *ovl2 = NULL;
						ovlDataStruct *ovl3 = NULL;
						ovlDataStruct *ovl4 = NULL;

						if (verbeOvl > 0)
							ovl2 = overlayTable[verbeOvl].ovlData;

						if (obj1Ovl > 0)
							ovl3 = overlayTable[obj1Ovl].ovlData;

						if (obj2Ovl > 0)
							ovl4 = overlayTable[obj2Ovl].ovlData;

						if ((ovl3) && (ptrHead->obj1Number >= 0)) {
							testState1 = ptrHead->obj1OldState;
						}
						if ((ovl4) && (ptrHead->obj2Number >= 0)) {
							testState2 = ptrHead->obj2OldState;
						}

						if ((ovl4) && (ptrHead->verbNumber >= 0) &&
						        ((testState1 == -1) || (testState1 == objectState2)) &&
						        ((testState2 == -1) || (testState2 == objectState))) {
							if (ovl2->nameVerbGlob) {
								const char *ptr = getObjectName(ptrHead->verbNumber, ovl2->nameVerbGlob);
								strcpy(verbe_name, ptr);

								if (!strlen(verbe_name))
									attacheNewScriptToTail(&relHead, j, ptrHead->id, 30, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, scriptType_REL);
								else if (ovl2->nameVerbGlob) {
									found = true;
									int color;

									if (objectState2 == -2)
										color = subColor;
									else
										color = -1;

									ptr = getObjectName(ptrHead->obj1Number, ovl3->arrayNameObj);
									addSelectableMenuEntry(j, i, menuTable[0], 1, color, ptr);
								}
							}
						}
					}
				}
			}
		}
	}

	return found;
}

bool findRelation(int objOvl, int objIdx, int x, int y) {
	bool found = false;
	bool first = true;
	int testState = -1;
	int j;
	int16 objectState;

	getSingleObjectParam(objOvl, objIdx, 5, &objectState);

	for (j = 1; j < numOfLoadedOverlay; j++) {
		if (overlayTable[j].alreadyLoaded) {
			int idHeader = overlayTable[j].ovlData->numMsgRelHeader;

			for (int i = 0; i < idHeader; i++) {
				linkDataStruct* ptrHead = &overlayTable[j].ovlData->arrayMsgRelHeader[i];
				int thisOvl = ptrHead->obj1Overlay;

				if (!thisOvl) {
					thisOvl = j;
				}

				//const char* pName = getObjectName(ptrHead->obj1Number, overlayTable[thisOvl].ovlData->arrayNameObj);

				objDataStruct* pObject = getObjectDataFromOverlay(thisOvl, ptrHead->obj1Number);

				if ((thisOvl == objOvl) && (objIdx == ptrHead->obj1Number) && pObject && (pObject->_class != THEME)) {
					int verbeOvl = ptrHead->verbOverlay;
					int obj1Ovl = ptrHead->obj1Overlay;
					int obj2Ovl = ptrHead->obj2Overlay;

					if (!verbeOvl) verbeOvl = j;
					if (!obj1Ovl)  obj1Ovl = j;
					if (!obj2Ovl)  obj2Ovl = j;

					char verbe_name[80];
					verbe_name[0]	= 0;

					ovlDataStruct *ovl2 = NULL;
					ovlDataStruct *ovl3 = NULL;
					//ovlDataStruct *ovl4 = NULL;

					if (verbeOvl > 0)
						ovl2 = overlayTable[verbeOvl].ovlData;

					if (obj1Ovl > 0)
						ovl3 = overlayTable[obj1Ovl].ovlData;

					//if (obj2Ovl > 0)
					//	ovl4 = overlayTable[obj2Ovl].ovlData;

					if ((ovl3) && (ptrHead->obj1Number >= 0)) {
						testState = ptrHead->obj1OldState;

						if ((first) && (ovl3->arrayNameObj) && ((testState == -1) || (testState == objectState))) {
							const char *ptrName = getObjectName(ptrHead->obj1Number, ovl3->arrayNameObj);

							menuTable[0] = createMenu(x, y, ptrName);
							first = false;
						}
					}
					if ((ovl2) && (ptrHead->verbNumber >= 0)) {
						if (ovl2->nameVerbGlob) {
							const char *ptr = getObjectName(ptrHead->verbNumber, ovl2->nameVerbGlob);
							strcpy(verbe_name, ptr);

							if ((!first) && ((testState == -1) || (testState == objectState))) {
								if (!strlen(verbe_name)) {
									if (currentScriptPtr) {
										attacheNewScriptToTail(&relHead, j, ptrHead->id, 30, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, scriptType_REL);
									} else {
										attacheNewScriptToTail(&relHead, j, ptrHead->id, 30, 0, 0, scriptType_REL);
									}
								} else if (ovl2->nameVerbGlob) {
									found = true;
									ptr = getObjectName(ptrHead->verbNumber, ovl2->nameVerbGlob);
									addSelectableMenuEntry(j, i, menuTable[0], 1, -1, ptr);
								}
							}
						}
					}
				}
			}
		}
	}

	return found;
}

int processInventory() {
	if (menuTable[1]) {
		menuElementSubStruct *pMenuElementSub = getSelectedEntryInMenu(menuTable[1]);

		if (pMenuElementSub) {
			int var2 = pMenuElementSub->ovlIdx;
			int var4 = pMenuElementSub->header;

			freeMenu(menuTable[1]);
			menuTable[1] = NULL;

			findRelation(var2, var4, currentMenuElementX + 80, currentMenuElementY);

			return 1;
		} else {
			freeMenu(menuTable[1]);
			menuTable[1] = NULL;
		}
	}

	return 0;
}

void callSubRelation(menuElementSubStruct *pMenuElement, int nOvl, int nObj) {
	if (pMenuElement == NULL)
		return;

	menuElementSubStruct* pCurrent = pMenuElement;

	while (pCurrent != NULL) {
		int ovlIdx = pCurrent->ovlIdx;
		int header = pCurrent->header;

		linkDataStruct* pHeader = &overlayTable[ovlIdx].ovlData->arrayMsgRelHeader[header];

		int obj2Ovl = pHeader->obj2Overlay;
		if (obj2Ovl == 0) {
			obj2Ovl = ovlIdx;
		}

		if ((obj2Ovl == nOvl) && (pHeader->obj2Number != -1) && (pHeader->obj2Number == nObj)) {
			int x = 60;
			int y = 60;

			objectParamsQuery params;
			memset(&params, 0, sizeof(objectParamsQuery)); // to remove warning

			if (pHeader->obj2Number >= 0) {
				getMultipleObjectParam(obj2Ovl, pHeader->obj2Number, &params);
			}

			if ((pHeader->obj2OldState == -1) || (params.state == pHeader->obj2OldState)) {
				if (pHeader->type == RT_REL) { // REL
					if (currentScriptPtr) {
						attacheNewScriptToTail(&relHead, ovlIdx, pHeader->id, 30, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, scriptType_REL);
					} else {
						attacheNewScriptToTail(&relHead, ovlIdx, pHeader->id, 30, 0, 0, scriptType_REL);
					}

					if ((narratorOvl > 0) && (pHeader->trackX != -1) && (pHeader->trackY != -1)) {
						actorStruct* pTrack = findActor(&actorHead, narratorOvl, narratorIdx, 0);

						if (pTrack) {
							animationStart = false;

							if (pHeader->trackDirection == 9999) {
								objectParamsQuery naratorParams;
								getMultipleObjectParam(narratorOvl, narratorIdx, &naratorParams);
								pTrack->x_dest = naratorParams.X;
								pTrack->y_dest = naratorParams.Y;
								pTrack->endDirection = direction(naratorParams.X, naratorParams.Y, pTrack->x_dest, pTrack->y_dest, 0, 0);
							} else if ((pHeader->trackX == 9999) && (pHeader->trackY == 9999)) {
								objectParamsQuery naratorParams;
								getMultipleObjectParam(narratorOvl, narratorIdx, &naratorParams);
								pTrack->x_dest = naratorParams.X;
								pTrack->y_dest = naratorParams.Y;
								pTrack->endDirection = pHeader->trackDirection;
							} else {
								pTrack->x_dest = pHeader->trackX;
								pTrack->y_dest = pHeader->trackY;
								pTrack->endDirection = pHeader->trackDirection;
							}

							pTrack->flag = 1;

							autoTrack = true;
							userEnabled = 0;
							changeScriptParamInList(ovlIdx, pHeader->id, &relHead, 0, 9998);
						}
					}
				} else if (pHeader->type == RT_MSG) {

					if (pHeader->obj2Number >= 0) {
						if ((pHeader->trackX !=-1) && (pHeader->trackY !=-1) &&
								(pHeader->trackX != 9999) && (pHeader->trackY != 9999)) {
							x = pHeader->trackX - 100;
							y = pHeader->trackY - 150;
						} else if (params.scale >= 0) {
							x = params.X - 100;
							y = params.Y - 40;
						}

						if (pHeader->obj2NewState != -1) {
							objInit(obj2Ovl, pHeader->obj2Number, pHeader->obj2NewState);
						}
					}

					if ((pHeader->obj1Number >= 0) && (pHeader->obj1NewState != -1)) {
						int obj1Ovl = pHeader->obj1Overlay;
						if (!obj1Ovl) obj1Ovl = ovlIdx;
						objInit(obj1Ovl, pHeader->obj1Number, pHeader->obj1NewState);
					}

					if (currentScriptPtr) {
						createTextObject(&cellHead, ovlIdx, pHeader->id, x, y, 200, findHighColor(), masterScreen, currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber);
					} else {
						createTextObject(&cellHead, ovlIdx, pHeader->id, x, y, 200, findHighColor(), masterScreen, 0, 0);
					}

					userWait = 1;
					autoOvl = ovlIdx;
					autoMsg = pHeader->id;

					if ((narratorOvl > 0) && (pHeader->trackX != -1) && (pHeader->trackY != -1)) {
						actorStruct *pTrack = findActor(&actorHead, narratorOvl, narratorIdx, 0);

						if (pTrack)	 {
							objectParamsQuery naratorParams;
							animationStart = false;

							if (pHeader->trackDirection == 9999) {
								getMultipleObjectParam(narratorOvl, narratorIdx, &naratorParams);
								pTrack->x_dest = naratorParams.X;
								pTrack->y_dest = naratorParams.Y;
								pTrack->endDirection = direction(naratorParams.X, naratorParams.Y, pHeader->trackX,pHeader->trackY, 0, 0);
							} else if ((pHeader->trackX == 9999) && (pHeader->trackY == 9999)) {
								getMultipleObjectParam(narratorOvl, narratorIdx, &naratorParams);
								pTrack->x_dest = naratorParams.X;
								pTrack->y_dest = naratorParams.Y;
								pTrack->endDirection = pHeader->trackDirection;
							} else {
								pTrack->x_dest = pHeader->trackX;
								pTrack->y_dest = pHeader->trackY;
								pTrack->endDirection = pHeader->trackDirection;
							}

							pTrack->flag = 1;
							autoTrack = true;
							userWait = 0;
							userEnabled = 0;
							freezeCell(&cellHead, ovlIdx, pHeader->id, 5, -1, 0, 9998);
						}
					}
				}
			}
		}

		pCurrent = pCurrent->pNext;
	}
}

int findHighColor() {
	int bestColorResult = -1;
	int bestColorIdx = -1;

	for (unsigned long int i = 0; i < 256; i++) {
		int colorResult = (workpal[i*3+0] + workpal[i*3+1] + workpal[i*3+2]) / 256;

		if (colorResult > bestColorResult) {
			bestColorIdx = i;
			bestColorResult =  colorResult;
		}
	}
	return bestColorIdx;
}

void callRelation(menuElementSubStruct *pMenuElement, int nObj2) {
	if (pMenuElement == NULL)
		return;

	menuElementSubStruct* pCurrent = pMenuElement;

	while (pCurrent != NULL) {
		int ovlIdx = pCurrent->ovlIdx;
		int header = pCurrent->header;

		linkDataStruct* pHeader = &overlayTable[ovlIdx].ovlData->arrayMsgRelHeader[header];

		if (pHeader->obj2Number == nObj2) {
			// REL
			if (pHeader->type == RT_REL) {
				if (currentScriptPtr) {
					attacheNewScriptToTail(&relHead, ovlIdx, pHeader->id, 30, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, scriptType_REL);
				} else {
					attacheNewScriptToTail(&relHead, ovlIdx, pHeader->id, 30, 0, 0, scriptType_REL);
				}

				if ((narratorOvl > 0) && (pHeader->trackX != -1) && (pHeader->trackY != -1)) {
					actorStruct* pTrack = findActor(&actorHead, narratorOvl, narratorIdx, 0);

					if (pTrack) {
						animationStart = false;

						if (pHeader->trackDirection == 9999) {
							objectParamsQuery naratorParams;
							getMultipleObjectParam(narratorOvl, narratorIdx, &naratorParams);
							pTrack->x_dest = naratorParams.X;
							pTrack->y_dest = naratorParams.Y;
							pTrack->endDirection = direction(naratorParams.X, naratorParams.Y, pTrack->x_dest, pTrack->y_dest, 0, 0);
						} else if ((pHeader->trackX == 9999) && (pHeader->trackY == 9999)) {
							objectParamsQuery naratorParams;
							getMultipleObjectParam(narratorOvl, narratorIdx, &naratorParams);
							pTrack->x_dest = naratorParams.X;
							pTrack->y_dest = naratorParams.Y;
							pTrack->endDirection = pHeader->trackDirection;
						} else {
							pTrack->x_dest = pHeader->trackX;
							pTrack->y_dest = pHeader->trackY;
							pTrack->endDirection = pHeader->trackDirection;
						}

						pTrack->flag = 1;

						autoTrack = true;
						userEnabled = 0;
						changeScriptParamInList(ovlIdx, pHeader->id, &relHead, 0, 9998);
					}
				}
			} else if (pHeader->type == RT_MSG) { // MSG
				int obj1Ovl = pHeader->obj1Overlay;
				if (!obj1Ovl)
					obj1Ovl = ovlIdx;

				int x = 60;
				int y = 40;

				if (pHeader->obj1Number >= 0) {
					objectParamsQuery params;
					getMultipleObjectParam(obj1Ovl, pHeader->obj1Number, &params);

					if (narratorOvl > 0) {
						if ((pHeader->trackX != -1) && (pHeader->trackY != -1) && (pHeader->trackX != 9999) && (pHeader->trackY != 9999)) {
							x = pHeader->trackX - 100;
							y = pHeader->trackY - 150;
						} else {
							getMultipleObjectParam(narratorOvl, narratorIdx, &params);
							x = params.X - 100;
							y = params.Y - 150;
						}
					} else if (params.scale >= 0) {
						x = params.X - 100;
						y = params.Y - 40;
					}

					if (pHeader->obj1NewState != -1) {
						objInit(obj1Ovl, pHeader->obj1Number, pHeader->obj1NewState);
					}
				}

				if (currentScriptPtr) {
					createTextObject(&cellHead, ovlIdx, pHeader->id, x, y, 200, findHighColor(), masterScreen, currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber);
				} else {
					createTextObject(&cellHead, ovlIdx, pHeader->id, x, y, 200, findHighColor(), masterScreen, 0, 0);
				}

				userWait = 1;
				autoOvl = ovlIdx;
				autoMsg = pHeader->id;

				if ((narratorOvl > 0) && (pHeader->trackX != -1) && (pHeader->trackY != -1)) {
					actorStruct* pTrack = findActor(&actorHead, narratorOvl, narratorIdx, 0);

					if (pTrack) {
						animationStart = false;

						if (pHeader->trackDirection == 9999) {
							objectParamsQuery naratorParams;
							getMultipleObjectParam(narratorOvl, narratorIdx, &naratorParams);
							pTrack->x_dest = naratorParams.X;
							pTrack->y_dest = naratorParams.Y;
							pTrack->endDirection = direction(naratorParams.X, naratorParams.Y, pTrack->x_dest, pTrack->y_dest, 0, 0);
						} else if ((pHeader->trackX == 9999) && (pHeader->trackY == 9999)) {
							objectParamsQuery naratorParams;
							getMultipleObjectParam(narratorOvl, narratorIdx, &naratorParams);
							pTrack->x_dest = naratorParams.X;
							pTrack->y_dest = naratorParams.Y;
							pTrack->endDirection = pHeader->trackDirection;
						} else {
							pTrack->x_dest = pHeader->trackX;
							pTrack->y_dest = pHeader->trackY;
							pTrack->endDirection = pHeader->trackDirection;
						}

						pTrack->flag = 1;

						autoTrack = true;
						userWait = 0;
						userEnabled = 0;
						freezeCell(&cellHead, ovlIdx, pHeader->id, 5, -1, 0, 9998);
					}
				}
			}
		} else {
			linkedRelation = pMenuElement;
		}

		pCurrent = pCurrent->pNext;
	}
}

void closeAllMenu() {
	if (menuTable[0]) {
		freeMenu(menuTable[0]);
		menuTable[0] = NULL;
	}

	if (menuTable[1]) {
		freeMenu(menuTable[1]);
		menuTable[1] = NULL;
	}
	if (linkedMsgList) {
		ASSERT(0);
//					freeMsgList(linkedMsgList);
	}

	linkedMsgList = NULL;
	linkedRelation = NULL;
}

bool checkInput(int16 *buttonPtr) {
	int16 handle, button;
	Common::Point pt;

	getMouseStatus(&handle, &pt.x, &button, &pt.y);

	if (!button)
		buttonDown = 0;
	else if (!buttonDown && button) {
		*buttonPtr = button;
		buttonDown = 1;
	}

	return false;
}

extern bool manageEvents();

int CruiseEngine::processInput() {
	int16 mouseX = 0;
	int16 mouseY = 0;
	int16 button = 0;

	/*if (inputSub1keyboad())
	 * {
	 * return 1;
	 * } */

	button = 0;

	if (sysKey != -1) {
		button = sysKey;
		mouseX = sysX;
		mouseY = sysY;
		sysKey = -1;
	} else if (automaticMode == 0) {
		getMouseStatus(&main10, &mouseX, &button, &mouseY);
	}

	if (!button) {
		buttonDown = 0;
	}

	// Check for Exit 'X' key
	if (keyboardCode == Common::KEYCODE_x)
		return 1;

	// Check for Pause 'P' key
	if (keyboardCode == Common::KEYCODE_p) {
		keyboardCode = Common::KEYCODE_INVALID;
		_vm->pauseEngine(true);
		mouseOff();

		bool pausedButtonDown = false;
		while (!_vm->shouldQuit()) {
			manageEvents();
			getMouseStatus(&main10, &mouseX, &button, &mouseY);

			if (button) pausedButtonDown = true;
			else if (pausedButtonDown)
				// Button released, so exit pause
				break;
			else if (keyboardCode != Common::KEYCODE_INVALID)
				break;

			g_system->delayMillis(10);
		}

		if (keyboardCode == Common::KEYCODE_x)
			// Exit the game
			return 1;

		keyboardCode = Common::KEYCODE_INVALID;
		_vm->pauseEngine(false);
		mouseOn();
		return 0;
	}

	// Player Menu - test for both buttons or the F10 key
	if (((button & CRS_MB_BOTH) == CRS_MB_BOTH) || (keyboardCode == Common::KEYCODE_F10)) {
		changeCursor(CURSOR_NORMAL);
		keyboardCode = Common::KEYCODE_INVALID;
		return (playerMenu(mouseX, mouseY));
	}

	if (userWait) {
		// Check for left mouse button click or Space to end user waiting
		if ((keyboardCode == Common::KEYCODE_SPACE) || (button == CRS_MB_LEFT))
			userWait = 0;

		keyboardCode = Common::KEYCODE_INVALID;
		return 0;
	}

	// Handle any changes in game speed
	if (_speedFlag) {
		if ((keyboardCode == Common::KEYCODE_KP_PLUS) && (_gameSpeed >= 30)) {
			_gameSpeed -= 10;
			keyboardCode = Common::KEYCODE_INVALID;
		}
		if ((keyboardCode == Common::KEYCODE_KP_MINUS) && (_gameSpeed <= 200)) {
			_gameSpeed += 10;
			keyboardCode = Common::KEYCODE_INVALID;
		}
	}

	if (!userEnabled) {
		return 0;
	}

	if ((currentActiveMenu != -1) && menuTable[currentActiveMenu]) {
		updateMenuMouse(mouseX, mouseY, menuTable[currentActiveMenu]);
	}

	if (dialogueEnabled) {

		if (menuDown || selectDown || linkedRelation) {
			closeAllMenu();
			menuDown = 0;
			selectDown = 0;
			currentActiveMenu = -1;
			changeCursor(CURSOR_NORMAL);
		}

		if ((menuTable[0] == NULL) && (!buttonDown)) {
			int dialogFound = createDialog(dialogueOvl, dialogueObj, xdial, 0);

			if (menuTable[0]) {
				if (dialogFound) {
					currentActiveMenu = 0;
				} else {
					freeMenu(menuTable[0]);
					menuTable[0] = NULL;
					currentActiveMenu = -1;
				}
			} else {
				menuDown = 0;
			}
		} else {
			if ((button & CRS_MB_LEFT) && (buttonDown == 0)) {
				if (menuTable[0]) {
					callRelation(getSelectedEntryInMenu(menuTable[0]), dialogueObj);

					freeMenu(menuTable[0]);
					menuTable[0] = NULL;

					if (linkedMsgList) {
						ASSERT(0);
						//					freeMsgList(linkedMsgList);
					}

					linkedMsgList = NULL;
					linkedRelation = NULL;

					changeCursor(CURSOR_NORMAL);
					currentActiveMenu = -1;
				}
				buttonDown = 1;
			}
		}

	} else if ((button & CRS_MB_LEFT) && (buttonDown == 0)) {
		// left click
		buttonDown = 1;

		// is there a relation
		if (linkedRelation) {
			// call sub relation when clicking on an object
			if (menuDown == 0) {
				if (menuTable[0]) {
					int objOvl;
					int objIdx;
					int objType;

					objType = findObject(mouseX, mouseY, &objOvl, &objIdx);

					if (objType != -1) {
						callSubRelation(linkedRelation, objOvl, objIdx);
					}
					freeMenu(menuTable[0]);
					menuTable[0] = NULL;
				}

				if (linkedMsgList) {
//					freeMsgList(linkedMsgList);
				}
				linkedMsgList = NULL;
				linkedRelation = NULL;
				changeCursor(CURSOR_NORMAL);
			} else { // call sub relation when clicking in inventory
				if (menuTable[0] && menuTable[1]) {
					menuElementSubStruct * p0 = getSelectedEntryInMenu(menuTable[1]);

					if (p0)
						callSubRelation(linkedRelation, p0->ovlIdx, p0->header);

					closeAllMenu();
					changeCursor(CURSOR_NORMAL);
				}
			}
			selectDown = 0;
			menuDown = 0;
		} else {
			// manage click on object menu
			if (menuDown == 0) {
				// Handle left click on an object
				if (menuTable[0] == 0) {
					int objOvl;
					int objIdx;
					int objType;

					objType = findObject(mouseX, mouseY, &objOvl, &objIdx);

					if (objType != -1) {
						int relation = findRelation(objOvl, objIdx, mouseX, mouseY);
						if (menuTable[0]) {
							if (relation) {
								currentActiveMenu = 0;
								selectDown = 1;
							} else {
								// object has a name but no relation, just move the character
								freeMenu(menuTable[0]);
								menuTable[0] = NULL;

								aniX = mouseX;
								aniY = mouseY;
								animationStart = true;
							}
						} else {
							aniX = mouseX;
							aniY = mouseY;
							animationStart = true;
						}
					} else {
						// No object found, we move the character to the cursor
						aniX = mouseX;
						aniY = mouseY;
						animationStart = true;
					}
				} else {
					// handle click in menu
					if (menuTable[0]) {
						menuElementSubStruct *pMenuElementSub = getSelectedEntryInMenu(menuTable[0]);

						callRelation(pMenuElementSub, -1);

						// if there is a linked relation, close menu
						if (!linkedRelation) {
							freeMenu(menuTable[0]);
							menuTable[0] = NULL;
							changeCursor(CURSOR_NORMAL);
						} else { // else create the message for the linked relation
							char text[80];
							strcpy(text, menuTable[0]->stringPtr);
							strcat(text, ":");
							strcat(text, currentMenuElement->string);
							linkedMsgList = renderText(320, (const char *)text);
							changeCursor(CURSOR_CROSS);
						}
					}

					currentActiveMenu = -1;
					selectDown = 0;
				}
			} else {
				// Handle left click in inventory
				if (processInventory()) {
					currentActiveMenu = 0;
					selectDown = 1;
					menuDown = 0;
				} else {
					currentActiveMenu = -1;
					menuDown = 0;
				}
			}
		}
	} else if ((button & CRS_MB_RIGHT) || (keyboardCode == Common::KEYCODE_F9)) {
		if (buttonDown == 0) {
			keyboardCode = Common::KEYCODE_INVALID;

			// close object menu if there is no linked relation
			if ((linkedRelation == 0) && (menuTable[0])) {
				freeMenu(menuTable[0]);
				menuTable[0] = NULL;
				selectDown = 0;
				menuDown = 0;
				currentActiveMenu = -1;
			}

			if ((!selectDown) && (!menuDown) && (menuTable[1] == NULL)) {
				buildInventory(mouseX, mouseY);

				if (menuTable[1]) {
					currentActiveMenu = 1;
					menuDown = 1;
				} else {
					menuDown = 1;
				}
			}
			buttonDown = 1;
		}
	}

	return 0;
}

int currentMouseX = 0;
int currentMouseY = 0;
int currentMouseButton = 0;

bool bFastMode = false;

bool manageEvents() {
	Common::Event event;

	Common::EventManager * eventMan = g_system->getEventManager();
	while (eventMan->pollEvent(event)) {
		bool abortFlag = true;

		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
			currentMouseButton |= CRS_MB_LEFT;
			break;
		case Common::EVENT_LBUTTONUP:
			currentMouseButton &= ~CRS_MB_LEFT;
			break;
		case Common::EVENT_RBUTTONDOWN:
			currentMouseButton |= CRS_MB_RIGHT;
			break;
		case Common::EVENT_RBUTTONUP:
			currentMouseButton &= ~CRS_MB_RIGHT;
			break;
		case Common::EVENT_MOUSEMOVE:
			currentMouseX = event.mouse.x;
			currentMouseY = event.mouse.y;
			abortFlag = false;
			break;
		case Common::EVENT_QUIT:
		case Common::EVENT_RTL:
			playerDontAskQuit = 1;
			break;
		case Common::EVENT_KEYUP:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				currentMouseButton &= ~CRS_MB_MIDDLE;
				break;
			default:
				break;
			}
			break;
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				currentMouseButton |= CRS_MB_MIDDLE;
				break;
			default:
				keyboardCode = event.kbd.keycode;
				break;
			}

			if (event.kbd.hasFlags(Common::KBD_CTRL)) {
				if (event.kbd.keycode == Common::KEYCODE_d) {
					// Start the debugger
					_vm->getDebugger()->attach();
					keyboardCode = Common::KEYCODE_INVALID;
				} else if (event.kbd.keycode == Common::KEYCODE_f) {
					bFastMode = !bFastMode;
					keyboardCode = Common::KEYCODE_INVALID;
				}
			}

		default:
			break;
		}

		if (abortFlag)
			return true;
	}

	return false;
}

void getMouseStatus(int16 *pMouseVar, int16 *pMouseX, int16 *pMouseButton, int16 *pMouseY) {
	*pMouseX = currentMouseX;
	*pMouseY = currentMouseY;
	*pMouseButton = currentMouseButton;
}


void CruiseEngine::mainLoop() {
	//int32 t_start,t_left;
	//uint32 t_end;
	//int32 q=0;                     /* Dummy */
	int16 mouseX, mouseY;
	int16 mouseButton;

	int enableUser = 0;

	strcpy(nextOverlay, "");
	strcpy(lastOverlay, "");
	strcpy(cmdLine, "");

	currentActiveMenu = -1;
	autoMsg = -1;
	linkedRelation = 0;
	main21 = 0;
	main22 = 0;
	userWait = 0;
	autoTrack = false;

	initAllData();

	playerDontAskQuit = 0;
	int quitValue2 = 1;
	int quitValue = 0;

	if (ConfMan.hasKey("save_slot"))
		loadGameState(ConfMan.getInt("save_slot"));

	do {
		// Handle frame delay
		uint32 currentTick = g_system->getMillis();

		// Delay for the specified amount of time, but still respond to events
		bool skipEvents = false;

		do {
			if (userEnabled && !userWait && !autoTrack) {
				if (currentActiveMenu == -1) {
					static int16 oldMouseX = -1;
					static int16 oldMouseY = -1;

					getMouseStatus(&main10, &mouseX, &mouseButton, &mouseY);

					if (mouseX != oldMouseX || mouseY != oldMouseY) {
						int objectType;
						int newCursor1;
						int newCursor2;

						oldMouseX = mouseX;
						oldMouseY = mouseY;

						objectType = findObject(mouseX, mouseY, &newCursor1, &newCursor2);

						if (objectType == 9) {
							changeCursor(CURSOR_EXIT);
						} else if (objectType != -1) {
							changeCursor(CURSOR_MAGNIFYING_GLASS);
						} else {
							changeCursor(CURSOR_WALK);
						}
					}
				} else {
					changeCursor(CURSOR_NORMAL);
				}
			} else {
				changeCursor(CURSOR_NORMAL);
			}

			g_system->updateScreen();

			if (!skipEvents || bFastMode)
				skipEvents = manageEvents();

			if (bFastMode) {
				if (currentTick >= (lastTickDebug + 10))
					lastTickDebug = currentTick;
			} else {
				g_system->delayMillis(10);
				currentTick = g_system->getMillis();
			}

			if (playerDontAskQuit)
				break;

			_vm->getDebugger()->onFrame();
		} while (currentTick < lastTick + _gameSpeed && !bFastMode);
		if (playerDontAskQuit)
			break;

		lastTick = g_system->getMillis();

		// Handle switchover in game speed after intro
		if (!_speedFlag && canLoadGameStateCurrently()) {
			_speedFlag = true;
			_gameSpeed = GAME_FRAME_DELAY_2;
		}

		// Handle the next frame

//		frames++;
//      t_start=Osystem_GetTicks();

//      readKeyboard();

		bool isUserWait = userWait != 0;
		// WORKAROUND: This prevents hotspots responding during
		// delays i.e. Menu opening if you click fast on another
		// hotspot after trying to open a locked door, which
		// occurred with the original interpreter.
		if (userDelay) {
			currentMouseButton = 0;
		}

		playerDontAskQuit = processInput();
		if (playerDontAskQuit)
			break;

		if (enableUser) {
			userEnabled = 1;
			enableUser = 0;
		}

		if (userDelay && !userWait) {
			userDelay--;
		}

		if (isUserWait & !userWait) {
			// User waiting has ended
			changeScriptParamInList(-1, -1, &procHead, 9999, 0);
			changeScriptParamInList(-1, -1, &relHead, 9999, 0);

			// Disable any mouse click used to end the user wait
			currentMouseButton = 0;
		}

		// FIXME: I suspect that the original game does multiple script executions between game frames; the bug with
		// Raoul appearing when looking at the book is being there are 3 script iterations separation between the
		// scene being changed to the book, and the Raoul actor being frozen/disabled. This loop is a hack to ensure
		// that does a few extra script executions for that scene
		bool bgChanged;
		int numIterations = 1;

		while (numIterations-- > 0) {
			bgChanged = backgroundChanged[masterScreen];

			manageScripts(&relHead);
			manageScripts(&procHead);

			removeFinishedScripts(&relHead);
			removeFinishedScripts(&procHead);

			if (!bgChanged && backgroundChanged[masterScreen] &&
					!strcmp(backgroundTable[0].name, "S06B.PI1")) {
				bgChanged = true;
				numIterations += 2;
			}
		}

		processAnimation();

		if (remdo) {
			// ASSERT(0);
			/*    main3 = 0;
			 * var24 = 0;
			 * var23 = 0;
			 *
			 * freeStuff2(); */
		}

		if (cmdLine[0]) {
			ASSERT(0);
			/*        redrawStrings(0,&cmdLine,8);

			        waitForPlayerInput();

			        cmdLine = 0; */
		}

		if (displayOn) {
			if (doFade)
				PCFadeFlag = 0;

			/*if (!PCFadeFlag)*/
			mainDraw(userWait);
			flipScreen();

			if (userWait == 1) {
				// Waiting for press - original wait loop has been integrated into the
				// main event loop
				continue;
			}

			// wait for character to finish auto track
			if (autoTrack) {
				if (isAnimFinished(narratorOvl, narratorIdx, &actorHead, ATP_MOUSE)) {
					if (autoMsg != -1) {
						freezeCell(&cellHead, autoOvl, autoMsg, 5, -1, 9998, 0);

						char* pText = getText(autoMsg, autoOvl);

						if (strlen(pText))
							userWait = 1;
					}

					changeScriptParamInList(-1, -1, &relHead, 9998, 0);
					autoTrack = false;
					enableUser = 1;
				} else {
					userEnabled = false;
				}
			} else if (autoMsg != -1) {
				removeCell(&cellHead, autoOvl, autoMsg, 5, masterScreen);
				autoMsg = -1;
			}
		} else {
			// Keep ScummVM being responsive even when displayOn is false
			g_system->updateScreen();
		}

	} while (!playerDontAskQuit && quitValue2 && quitValue != 7);

	// Free data
	removeAllScripts(&relHead);
	removeAllScripts(&procHead);
	resetActorPtr(&actorHead);
	freeOverlayTable();
	closeCnf();
	closeBase();
	resetFileEntryRange(0, NUM_FILE_ENTRIES);
	freeObjectList(&cellHead);
	freeBackgroundIncrustList(&backgroundIncrustHead);
}

} // End of namespace Cruise
