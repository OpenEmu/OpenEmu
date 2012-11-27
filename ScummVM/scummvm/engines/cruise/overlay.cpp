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

#include "common/memstream.h"
#include "common/textconsole.h"

#include "cruise/cruise.h"
#include "cruise/cruise_main.h"

namespace Cruise {

overlayStruct overlayTable[90];
int numOfLoadedOverlay;

void initOverlayTable() {
	int i;

	for (i = 0; i < 90; i++) {
		overlayTable[i].overlayName[0] = 0;
		overlayTable[i].ovlData = NULL;
		overlayTable[i].alreadyLoaded = 0;
		overlayTable[i].executeScripts = 0;
	}

	numOfLoadedOverlay = 1;
}

void freeOverlayTable() {
	for (int i = 0; i < 90; i++) {
		if (overlayTable[i].alreadyLoaded)
			freeOverlay(i);
	}
}

int freeOverlay(int overlayIdx) {
	ovlDataStruct *ovlDataPtr;
	int i;

	if (overlayTable[overlayIdx].alreadyLoaded == 0)
		return -4;

	overlayTable[overlayIdx].alreadyLoaded = 0;

	ovlDataPtr = overlayTable[overlayIdx].ovlData;

	if (!ovlDataPtr)
		return -4;


	/*
	  if (overlayTable[overlayIdx].var1E) {
	    MemFree(overlayTable[overlayIdx].var1E);
	    overlayTable[overlayIdx].var1E = NULL;
	  }

	  if (overlayTable[overlayIdx].var16) {
	    MemFree(overlayTable[overlayIdx].var16);
	    overlayTable[overlayIdx].var16 = NULL;
	  } */

	removeScript(overlayIdx, -1, &procHead);
	removeScript(overlayIdx, -1, &procHead);

	removeScript(overlayIdx, -1, &relHead);
	removeScript(overlayIdx, -1, &relHead);

	if (ovlDataPtr->stringTable) {
		for (i = 0; i < ovlDataPtr->numStrings; ++i)
			MemFree(ovlDataPtr->stringTable[i].string);
		MemFree(ovlDataPtr->stringTable);
	}
	if (ovlDataPtr->arrayProc) {
		ovlData3Struct *tempPtr = ovlDataPtr->arrayProc;
		for (i = 0; i < ovlDataPtr->numProc; ++i, ++tempPtr)
			MemFree(tempPtr->dataPtr);
		MemFree(ovlDataPtr->arrayProc);
	}
	if (ovlDataPtr->ptr1) {
		ovlData3Struct *tempPtr = (ovlData3Struct *)ovlDataPtr->ptr1;
		for (i = 0; i < ovlDataPtr->numRel; ++i, ++tempPtr)
			MemFree(tempPtr->dataPtr);
		MemFree(ovlDataPtr->ptr1);
	}

	MemFree(ovlDataPtr->arraySymbGlob);
	MemFree(ovlDataPtr->arrayNameSymbGlob);
	MemFree(ovlDataPtr->data4Ptr);
	MemFree(ovlDataPtr->arrayMsgRelHeader);
	MemFree(ovlDataPtr->ptr8);
	MemFree(ovlDataPtr->arrayObject);
	MemFree(ovlDataPtr->arrayObjVar);
	MemFree(ovlDataPtr->arrayStates);
	MemFree(ovlDataPtr->nameVerbGlob);
	MemFree(ovlDataPtr->arrayNameObj);
	MemFree(ovlDataPtr->arrayRelocGlob);
	MemFree(ovlDataPtr->arrayNameRelocGlob);

	MemFree(ovlDataPtr);
	overlayTable[overlayIdx].ovlData = NULL;

	debug(1, "freeOverlay: finish !");

	return 0;
}

int loadOverlay(const char *scriptName) {
	int newNumberOfScript;
	bool scriptNotLoadedBefore;
	int scriptIdx;
	char fileName[50];
	int fileIdx;
	int unpackedSize;
	byte *unpackedBuffer;
	ovlDataStruct *ovlData;

	debug(1, "Load overlay: %s", scriptName);

	newNumberOfScript = numOfLoadedOverlay;

	scriptNotLoadedBefore = false;

	scriptIdx = findOverlayByName((const char *)scriptName);

	if (scriptIdx == -4) {
		scriptIdx = numOfLoadedOverlay;

		newNumberOfScript++;

		scriptNotLoadedBefore = true;
	}

	if (overlayTable[scriptIdx].alreadyLoaded) {
		return (scriptIdx);
	}

	overlayTable[scriptIdx].ovlData =
	    (ovlDataStruct *) mallocAndZero(sizeof(ovlDataStruct));

	if (!overlayTable[scriptIdx].ovlData)
		return (-2);

	if (scriptName != overlayTable[scriptIdx].overlayName)
		strcpy(overlayTable[scriptIdx].overlayName, scriptName);

	overlayTable[scriptIdx].alreadyLoaded = 1;

	numOfLoadedOverlay = newNumberOfScript;

	overlayTable[scriptIdx].ovlData->scriptNumber = scriptIdx;

	strcpy(fileName, scriptName);

	strcat(fileName, ".OVL");

	debug(1, "Attempting to load overlay file %s...", fileName);

	fileIdx = findFileInDisks(fileName);

	if (fileIdx < 0) {
		warning("Unable to load overlay %s", scriptName);
		//releaseScript(scriptName);
		return (-18);
	}

	unpackedSize = volumePtrToFileDescriptor[fileIdx].extSize + 2;

	// This memory block will be later passed to a MemoryReadStream, which will dispose of it
	unpackedBuffer = (byte *)malloc(unpackedSize);
	if (!unpackedBuffer) {
		return (-2);
	}
	memset(unpackedBuffer, 0, unpackedSize);

	if (volumePtrToFileDescriptor[fileIdx].size + 2 != unpackedSize) {
		char *pakedBuffer =
		    (char *)mallocAndZero(volumePtrToFileDescriptor[fileIdx].
		                          size + 2);

		loadPackedFileToMem(fileIdx, (uint8 *) pakedBuffer);

		delphineUnpack((uint8 *)unpackedBuffer, (const uint8 *)pakedBuffer, volumePtrToFileDescriptor[fileIdx].size);

		MemFree(pakedBuffer);
	} else {
		loadPackedFileToMem(fileIdx, (uint8 *) unpackedBuffer);
	}

	debug(1, "OVL loading done...");

	Common::MemoryReadStream s(unpackedBuffer, unpackedSize, DisposeAfterUse::YES);
	unpackedBuffer = NULL;

	ovlData = overlayTable[scriptIdx].ovlData;

	// Skip pointers
	s.skip(60);

	ovlData->arrayProc = NULL;
	ovlData->ptr1 = NULL;
	ovlData->arrayObject = NULL;
	ovlData->arrayStates = NULL;
	ovlData->arrayObjVar = NULL;
	ovlData->stringTable = NULL;
	ovlData->arraySymbGlob = NULL;
	ovlData->arrayRelocGlob = NULL;
	ovlData->arrayMsgRelHeader = NULL;
	ovlData->nameVerbGlob = NULL;
	ovlData->arrayNameObj = NULL;
	ovlData->arrayNameRelocGlob = NULL;
	ovlData->arrayNameSymbGlob = NULL;
	ovlData->data4Ptr = NULL;
	ovlData->ptr8 = NULL;
	ovlData->numProc = s.readUint16BE();
	ovlData->numRel = s.readUint16BE();
	ovlData->numSymbGlob = s.readUint16BE();
	ovlData->numRelocGlob = s.readUint16BE();
	ovlData->numMsgRelHeader = s.readUint16BE();
	ovlData->numObj = s.readUint16BE();
	ovlData->numStrings = s.readUint16BE();
	ovlData->size8 = s.readUint16BE();
	ovlData->size9 = s.readUint16BE();
	ovlData->nameExportSize = s.readUint16BE();
	ovlData->exportNamesSize = s.readUint16BE();
	ovlData->specialString2Length = s.readUint16BE();
	ovlData->sizeOfData4 = s.readUint16BE();
	ovlData->size12 = s.readUint16BE();
	ovlData->specialString1Length = s.readUint16BE();
	ovlData->scriptNumber = s.readUint16BE();

	if (ovlData->numSymbGlob)	{ // export data
		int i;
		ovlData->arraySymbGlob =
		    (exportEntryStruct *) mallocAndZero(ovlData->numSymbGlob * sizeof(exportEntryStruct));

		if (!ovlData->arraySymbGlob) {
			return (-2);
		}

		for (i = 0; i < ovlData->numSymbGlob; i++) {
			ovlData->arraySymbGlob[i].var0 = s.readUint16BE();
			ovlData->arraySymbGlob[i].var2 = s.readUint16BE();
			ovlData->arraySymbGlob[i].var4 = s.readUint16BE();
			ovlData->arraySymbGlob[i].idx = s.readUint16BE();
			ovlData->arraySymbGlob[i].offsetToName = s.readUint16BE();
		}
	}

	if (ovlData->exportNamesSize) {	// export names
		ovlData->arrayNameSymbGlob = (char *) mallocAndZero(ovlData->exportNamesSize);

		if (!ovlData->arrayNameSymbGlob) {
			return (-2);
		}

		s.read(ovlData->arrayNameSymbGlob, ovlData->exportNamesSize);
	}

	if (ovlData->numRelocGlob) {	// import data
		int i;

		ovlData->arrayRelocGlob =
		    (importDataStruct *) mallocAndZero(ovlData->numRelocGlob *
		                                       sizeof(importDataStruct));

		if (!ovlData->arrayRelocGlob) {
			return (-2);
		}

		for (i = 0; i < ovlData->numRelocGlob; i++) {
			ovlData->arrayRelocGlob[i].var0 = s.readUint16BE();
			ovlData->arrayRelocGlob[i].var1 = s.readUint16BE();
			ovlData->arrayRelocGlob[i].linkType = s.readUint16BE();
			ovlData->arrayRelocGlob[i].linkIdx = s.readUint16BE();
			ovlData->arrayRelocGlob[i].nameOffset = s.readUint16BE();
		}
	}

	if (ovlData->nameExportSize) {	// import name
		ovlData->arrayNameRelocGlob = (char *) mallocAndZero(ovlData->nameExportSize);

		if (!ovlData->arrayNameRelocGlob) {
			return (-2);
		}

		s.read(ovlData->arrayNameRelocGlob, ovlData->nameExportSize);
	}

	if (ovlData->numMsgRelHeader) {	// link data
		int i;
		ASSERT(sizeof(linkDataStruct) == 0x22);

		ovlData->arrayMsgRelHeader = (linkDataStruct *) mallocAndZero(ovlData->numMsgRelHeader * sizeof(linkDataStruct));

		if (!ovlData->arrayMsgRelHeader) {
			return (-2);
		}

		for (i = 0; i < ovlData->numMsgRelHeader; i++) {
			ovlData->arrayMsgRelHeader[i].type = s.readUint16BE();
			ovlData->arrayMsgRelHeader[i].id = s.readUint16BE();
			ovlData->arrayMsgRelHeader[i].offsetVerbeName = s.readUint16BE();
			ovlData->arrayMsgRelHeader[i].verbOverlay = s.readUint16BE();
			ovlData->arrayMsgRelHeader[i].verbNumber = s.readUint16BE();

			ovlData->arrayMsgRelHeader[i].obj1Overlay = s.readUint16BE();
			ovlData->arrayMsgRelHeader[i].obj1Number = s.readUint16BE();
			ovlData->arrayMsgRelHeader[i].obj2Overlay = s.readUint16BE();
			ovlData->arrayMsgRelHeader[i].obj2Number = s.readUint16BE();

			ovlData->arrayMsgRelHeader[i].trackX = s.readUint16BE();
			ovlData->arrayMsgRelHeader[i].trackY = s.readUint16BE();

			ovlData->arrayMsgRelHeader[i].obj1NewState = s.readUint16BE();
			ovlData->arrayMsgRelHeader[i].obj2NewState = s.readUint16BE();

			ovlData->arrayMsgRelHeader[i].obj1OldState = s.readUint16BE();
			ovlData->arrayMsgRelHeader[i].obj2OldState = s.readUint16BE();

			ovlData->arrayMsgRelHeader[i].trackDirection = s.readUint16BE();
			ovlData->arrayMsgRelHeader[i].dialog = s.readUint16BE();
		}
	}

	if (ovlData->numProc) {	// script
		ovlData3Struct *tempPtr;
		int i;

		ovlData->arrayProc =
		    (ovlData3Struct *) mallocAndZero(ovlData->numProc * sizeof(ovlData3Struct));

		if (!ovlData->arrayProc) {
			return (-2);
		}

		for (i = 0; i < ovlData->numProc; i++) {
			s.skip(4);
			ovlData->arrayProc[i].dataPtr = NULL;
			ovlData->arrayProc[i].sizeOfData = s.readUint16BE();
			ovlData->arrayProc[i].offsetToSubData3 = s.readUint16BE();
			ovlData->arrayProc[i].offsetToImportData = s.readUint16BE();
			ovlData->arrayProc[i].offsetToSubData2 = s.readUint16BE();
			ovlData->arrayProc[i].offsetToImportName = s.readUint16BE();
			ovlData->arrayProc[i].offsetToSubData5 = s.readUint16BE();
			ovlData->arrayProc[i].sysKey = s.readUint16BE();
			ovlData->arrayProc[i].var12 = s.readUint16BE();
			ovlData->arrayProc[i].numRelocGlob = s.readUint16BE();
			ovlData->arrayProc[i].subData2Size = s.readUint16BE();
			ovlData->arrayProc[i].var18 = s.readUint16BE();
			ovlData->arrayProc[i].var1A = s.readUint16BE();
		}

		tempPtr = ovlData->arrayProc;

		for (i = 0; i < ovlData->numProc; i++) {
			tempPtr->dataPtr = (uint8 *) mallocAndZero(tempPtr->sizeOfData);

			if (!tempPtr->dataPtr) {
				return (-2);
			}

			s.read(tempPtr->dataPtr, tempPtr->sizeOfData);

			if (tempPtr->offsetToImportData) {
				flipGen(tempPtr->dataPtr + tempPtr->offsetToImportData,
				        tempPtr->numRelocGlob * 10);
			}

			if (tempPtr->offsetToSubData2) {
				flipGen(tempPtr->dataPtr + tempPtr->offsetToSubData2,
				        tempPtr->subData2Size * 10);
			}

			tempPtr++;
		}
	}

	if (ovlData->numRel) {
		ovlData3Struct *tempPtr;
		int i;

		ovlData->ptr1 =
		    (ovlData3Struct *) mallocAndZero(ovlData->numRel * sizeof(ovlData3Struct));

		if (!ovlData->ptr1) {
			return (-2);
		}

		for (i = 0; i < ovlData->numRel; i++) {
			s.skip(4);
			ovlData->ptr1[i].dataPtr = NULL;
			ovlData->ptr1[i].sizeOfData = s.readUint16BE();
			ovlData->ptr1[i].offsetToSubData3 = s.readUint16BE();
			ovlData->ptr1[i].offsetToImportData = s.readUint16BE();
			ovlData->ptr1[i].offsetToSubData2 = s.readUint16BE();
			ovlData->ptr1[i].offsetToImportName = s.readUint16BE();
			ovlData->ptr1[i].offsetToSubData5 = s.readUint16BE();
			ovlData->ptr1[i].sysKey = s.readUint16BE();
			ovlData->ptr1[i].var12 = s.readUint16BE();
			ovlData->ptr1[i].numRelocGlob = s.readUint16BE();
			ovlData->ptr1[i].subData2Size = s.readUint16BE();
			ovlData->ptr1[i].var18 = s.readUint16BE();
			ovlData->ptr1[i].var1A = s.readUint16BE();
		}

		tempPtr = (ovlData3Struct *) ovlData->ptr1;

		for (i = 0; i < ovlData->numRel; i++) {
			tempPtr->dataPtr = (uint8 *) mallocAndZero(tempPtr->sizeOfData);

			if (!tempPtr->dataPtr) {
				return (-2);
			}

			s.read(tempPtr->dataPtr, tempPtr->sizeOfData);

			if (tempPtr->offsetToImportData) {
				flipGen(tempPtr->dataPtr + tempPtr->offsetToImportData,
				        tempPtr->numRelocGlob * 10);
			}

			if (tempPtr->offsetToSubData2) {
				flipGen(tempPtr->dataPtr + tempPtr->offsetToSubData2,
				        tempPtr->subData2Size * 10);
			}

			tempPtr++;
		}
	}

	if (ovlData->size12) {
		ovlData->ptr8 = (uint8 *) mallocAndZero(ovlData->size12);

		if (!ovlData->ptr8) {
			/*      releaseScript(scriptIdx,scriptName);

			      if (freeIsNeeded) {
			        freePtr(unpackedBuffer);
			      } */

			return (-2);
		}

		s.read(ovlData->ptr8, ovlData->size12);
	}

	if (ovlData->numObj) {
		int i;
		ovlData->arrayObject =
		    (objDataStruct *) mallocAndZero(ovlData->numObj *
		                                    sizeof(objDataStruct));

		if (!ovlData->arrayObject) {
			return (-2);
		}

		for (i = 0; i < ovlData->numObj; i++) {
			ovlData->arrayObject[i]._type = s.readUint16BE();
			ovlData->arrayObject[i]._class = (eClass) s.readUint16BE();
			ovlData->arrayObject[i]._nameOffset = s.readUint16BE();
			ovlData->arrayObject[i]._numStates = s.readUint16BE();
			ovlData->arrayObject[i]._varTableIdx = s.readUint16BE();
			ovlData->arrayObject[i]._firstStateIdx = s.readUint16BE();
			ovlData->arrayObject[i]._stateTableIdx = s.readUint16BE();
		}

		// allocate states for object with multiple states

		if (scriptNotLoadedBefore) {
			overlayTable[scriptIdx].state = stateID;
			stateID += getNumObjectsByClass(scriptIdx, MULTIPLE) + getNumObjectsByClass(scriptIdx, THEME);
		}
	}

	if (ovlData->size9) {
		ovlData->arrayObjVar =
		    (objectParams *) mallocAndZero(ovlData->size9 *
		                                   sizeof(objectParams));

		if (!ovlData->arrayObjVar) {
			return (-2);
		}
	}

	if (ovlData->size8) {
		int i;
		ovlData->arrayStates =
		    (objectParams *) mallocAndZero(ovlData->size8 *
		                                   sizeof(objectParams));

		if (!ovlData->arrayStates) {
			return (-2);
		}

		for (i = 0; i < ovlData->size8; i++) {
			ovlData->arrayStates[i].X = s.readUint16BE();
			ovlData->arrayStates[i].Y = s.readUint16BE();
			ovlData->arrayStates[i].Z = s.readUint16BE();
			ovlData->arrayStates[i].frame = s.readUint16BE();
			ovlData->arrayStates[i].scale = s.readUint16BE();
			ovlData->arrayStates[i].state = s.readUint16BE();
		}
	}

	if (ovlData->numStrings) {
		int i;

		ovlData->stringTable =
		    (stringEntryStruct *) mallocAndZero(ovlData->numStrings *
		                                        sizeof(stringEntryStruct));

		for (i = 0; i < ovlData->numStrings; i++) {
			ovlData->stringTable[i].idx = s.readUint16BE();
		}
	}

	if (ovlData->sizeOfData4) {
		ovlData->data4Ptr =
		    (uint8 *) mallocAndZero(ovlData->sizeOfData4);

		if (!ovlData->data4Ptr) {
			return (-2);
		}
	}

	if (ovlData->
	        specialString1Length /*|| ovlData->specialString2Length */
	        || ovlData->stringTable) {
		int i;
		//int unpackedSize;
		//int fileIdx;
		//uint8 fileName[50];
		//char* unpackedBuffer;

		strcpy(fileName, scriptName);

		strcat(fileName, ".FR");

		fileIdx = findFileInDisks(fileName);

		if (fileIdx < 0) {
			//releaseScript(scriptName);
			return (-18);
		}

		unpackedSize = volumePtrToFileDescriptor[fileIdx].extSize + 2;

		// This memory block will be later passed to a MemoryReadStream, which will dispose of it
		unpackedBuffer = (byte *)malloc(unpackedSize);
		if (!unpackedBuffer) {
			return (-2);
		}
		memset(unpackedBuffer, 0, unpackedSize);

		if (volumePtrToFileDescriptor[fileIdx].size + 2 !=
		        unpackedSize) {
			char *pakedBuffer =
			    (char *)
			    mallocAndZero(volumePtrToFileDescriptor[fileIdx].
			                  size + 2);

			loadPackedFileToMem(fileIdx, (uint8 *) pakedBuffer);

			delphineUnpack((uint8 *) unpackedBuffer, (const uint8 *)pakedBuffer, volumePtrToFileDescriptor[fileIdx].size);

			MemFree(pakedBuffer);
		} else {
			loadPackedFileToMem(fileIdx, (uint8 *) unpackedBuffer);
		}

		Common::MemoryReadStream s2(unpackedBuffer, unpackedSize, DisposeAfterUse::YES);
		unpackedBuffer = NULL;

		ovlData->specialString1Length = s2.readUint16BE();
		if (ovlData->specialString1Length) {
			ovlData->nameVerbGlob = (char *) mallocAndZero(ovlData->specialString1Length);

			if (!ovlData->nameVerbGlob) {
				/*      releaseScript(scriptIdx,scriptName);
				 *
				 * if (freeIsNeeded)
				 * {
				 * freePtr(unpackedBuffer);
				 * } */

				return (-2);
			}

			s2.read(ovlData->nameVerbGlob, ovlData->specialString1Length);
		}

		ovlData->specialString2Length = s2.readUint16BE();
		if (ovlData->specialString2Length) {
			ovlData->arrayNameObj = (char *) mallocAndZero(ovlData->specialString2Length);

			if (!ovlData->arrayNameObj) {
				/*      releaseScript(scriptIdx,scriptName);
				 *
				 * if (freeIsNeeded)
				 * {
				 * freePtr(unpackedBuffer);
				 * } */

				return (-2);
			}

			s2.read(ovlData->arrayNameObj, ovlData->specialString2Length);
		}

		for (i = 0; i < ovlData->numStrings; i++) {
			ovlData->stringTable[i].length = s2.readUint16BE();

			if (ovlData->stringTable[i].length) {
				ovlData->stringTable[i].string =
				    (char *)mallocAndZero(ovlData->
				                          stringTable[i].length);

				if (!ovlData->stringTable[i].string) {
					/*      releaseScript(scriptIdx,scriptName);
					 *
					 * if (freeIsNeeded)
					 * {
					 * freePtr(unpackedBuffer);
					 * } */

					return (-2);
				}

				s2.read(ovlData->stringTable[i].string, ovlData->stringTable[i].length);
			}
		}
	}
#ifdef DUMP_SCRIPT
	{
		int i;
		for (i = 0; i < ovlData->numProc; i++) {
			dumpScript(scriptName, ovlData, i);
		}
	}
#endif
#ifdef DUMP_OBJECT
	{
		// TODO: Rewrite this to use Common::DumpFile
		int i;
		FILE *fHandle;
		char nameBundle[100];
		sprintf(nameBundle, "%s-objs.txt", scriptName);

		fHandle = fopen(nameBundle, "w+");
		ASSERT(fHandle);

		for (i = 0; i < ovlData->numMsgRelHeader; i++) {
			linkDataStruct *var_34;
			var_34 = &ovlData->arrayMsgRelHeader[i];

			if (ovlData->arrayNameObj) {
				fprintf(fHandle, "----- object %02d -----\n",
				        i);
				if (var_34->stringNameOffset != 0xFFFF) {
					fprintf(fHandle, "name: %s\n",
					        getObjectName(var_34->
					                      stringNameOffset,
					                      ovlData->arrayNameObj));
				}
			}
		}

		fclose(fHandle);
	}
#endif

	return (scriptIdx);
}

int releaseOverlay(const char *name) {
	int overlayIdx = findOverlayByName(name);

	if (overlayIdx == -4)
		return -4;

	return freeOverlay(overlayIdx);
}

int32 findOverlayByName2(const char *name) {
	int i;

	for (i = 1; i < numOfLoadedOverlay; i++) {
		if (!strcmp(overlayTable[i].overlayName, name))
			return (i);
	}

	return (-4);
}

int findOverlayByName(const char *overlayName) {
	int i;

	for (i = 1; i < numOfLoadedOverlay; i++) {
		if (!strcmp(overlayTable[i].overlayName, overlayName)) {
			return (i);
		}
	}

	return (-4);
}

} // End of namespace Cruise
