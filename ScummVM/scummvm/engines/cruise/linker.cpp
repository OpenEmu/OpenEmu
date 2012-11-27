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

#include "common/endian.h"
#include "cruise/cruise_main.h"

namespace Cruise {

exportEntryStruct *parseExport(int *out1, int *pExportedFuncionIdx, char *buffer) {
	char localBuffer[256];
	char functionName[256];
	char overlayName[256];
	char *dotPtr;
	char *ptr2;
	int idx;
	int numSymbGlob;
	exportEntryStruct *currentExportEntry;
	char *entity1Name;
	int i;

	*out1 = 0;
	*pExportedFuncionIdx = 0;

	strcpy(localBuffer, buffer);
	dotPtr = strchr(localBuffer, '.');

	if (dotPtr) {
		strcpy(functionName, dotPtr + 1);
		*dotPtr = 0;

		strcpy(overlayName, localBuffer);
	} else {
		overlayName[0] = 0;

		strcpy(functionName, buffer);
	}

	ptr2 = strchr((char *)functionName, ':');

	if (ptr2) {
		*ptr2 = 0;

		*out1 = 1;
	}

	strToUpper(overlayName);
	strToUpper(functionName);
	if (strlen((char *)overlayName) == 0)
		return NULL;

	idx = findOverlayByName2(overlayName);

	if (idx == -4)
		return (NULL);

	if (overlayTable[idx].alreadyLoaded == 0)
		return (NULL);

	if (!overlayTable[idx].ovlData)
		return (NULL);

	numSymbGlob = overlayTable[idx].ovlData->numSymbGlob;
	currentExportEntry = overlayTable[idx].ovlData->arraySymbGlob;
	entity1Name = overlayTable[idx].ovlData->arrayNameSymbGlob;

	if (!entity1Name)
		return (0);

	for (i = 0; i < numSymbGlob; i++) {
		char exportedName[256];
		char *name = entity1Name + currentExportEntry->offsetToName;

		strcpy(exportedName, name);
		strToUpper(exportedName);

		if (!strcmp(functionName, exportedName)) {
			*pExportedFuncionIdx = idx;

			return (currentExportEntry);
		}

		currentExportEntry++;
	}

	return (NULL);
}

int updateScriptImport(int ovlIdx) {
	char buffer[256];
	ovlDataStruct *ovlData;
	int numData3;
	int size5;
	int numRelocGlob;
	int param;
	int var_32;
	ovlData3Struct *pScript;
//  char* arrayRelocGlob;
//  char* namePtr;
//  char* arrayMsgRelHeader;

	if (!overlayTable[ovlIdx].ovlData)
		return (-4);

	ovlData = overlayTable[ovlIdx].ovlData;

	numData3 = ovlData->numProc;
	size5 = ovlData->numRel;
	numRelocGlob = ovlData->numRelocGlob;
	param = 0;

	// do it for the 2 first string types
	do {

		int i = 0;

		if (param == 0) {
			var_32 = numData3;
		} else {
			var_32 = size5;
		}

		if (var_32) {
			do {
				importScriptStruct *ptrImportData;
				const char *ptrImportName;
				uint8 *ptrData;

				if (param == 0) {
					pScript = getOvlData3Entry(ovlIdx, i);
				} else {
					pScript = scriptFunc1Sub2(ovlIdx, i);
				}

				ptrImportData = (importScriptStruct *)(pScript->dataPtr + pScript->offsetToImportData);	// import data
				ptrImportName = (const char*)(pScript->dataPtr + pScript->offsetToImportName);	// import name
				ptrData = pScript->dataPtr;

				if (pScript->numRelocGlob > 0) {
					int counter = pScript->numRelocGlob;

					do {
						int param2 = ptrImportData->type;

						if (param2 != 70) {
							exportEntryStruct * ptrDest2;
							int out1;
							int out2;

							strcpy(buffer, ptrImportName + ptrImportData->offsetToName);
							ptrDest2 = parseExport(&out1, &out2, buffer);

							if (ptrDest2 && out2) {
								int temp =
								    ptrImportData->
								    offset;
								if (out1) {	//is sub function... (ie  'invent.livre:s')
									uint8 *ptr = ptrData + temp;

									*(ptr + 1) = out2;
									WRITE_BE_UINT16(ptr + 2, ptrDest2->idx);
								} else {
									if (param2 == 20 || param2 == 30 || param2 == 40 || param2 == 50) {	// this patch a double push
										uint8 *ptr = ptrData + temp;

										*(ptr + 1) = 0;
										*(ptr + 2) = out2;	// update the overlay number

										WRITE_BE_UINT16(ptr + 4, ptrDest2->idx);
									} else {
										int var_4 = ptrDest2->var4;

										if (var_4 & 1) {
											param2 = 8;
										} else {
											param2 = 16;
										}

										if (var_4 >= 0 && var_4 <= 3) {
											param2 |= 5;
										} else {
											param2 |= 6;
										}

										*(ptrData + temp) = param2;
										*(ptrData + temp + 1) = out2;

										WRITE_BE_UINT16(ptrData + temp + 2, ptrDest2->idx);
									}
								}
							}
						}

						ptrImportData++;
					} while (--counter);
				}

			} while (++i < var_32);

		}

	} while (++param < 2);

	if (ovlData->arrayRelocGlob && ovlData->arrayNameRelocGlob && numRelocGlob) {
		int numImport2 = numRelocGlob;
		int i;

		for (i = 0; i < numImport2; i++) {
			int out1;
			int foundExportIdx;
			exportEntryStruct *pFoundExport;
			int linkType;
			int linkEntryIdx;

			strcpy(buffer, ovlData->arrayNameRelocGlob + ovlData->arrayRelocGlob[i].nameOffset);

			pFoundExport = parseExport(&out1, &foundExportIdx, buffer);

			linkType = ovlData->arrayRelocGlob[i].linkType;
			linkEntryIdx = ovlData->arrayRelocGlob[i].linkIdx;

			if (pFoundExport && foundExportIdx) {
				switch (linkType) {
				case 0: {	// verb
					ovlData->arrayMsgRelHeader[linkEntryIdx].verbOverlay = foundExportIdx;
					ovlData->arrayMsgRelHeader[linkEntryIdx].verbNumber = pFoundExport->idx;
					break;
				}
				case 1: {	// obj1
					ovlData->arrayMsgRelHeader[linkEntryIdx].obj1Overlay = foundExportIdx;
					ovlData->arrayMsgRelHeader[linkEntryIdx].obj1Number = pFoundExport->idx;
					break;
				}
				case 2: {	// obj2
					ovlData->arrayMsgRelHeader[linkEntryIdx].obj2Overlay = foundExportIdx;
					ovlData->arrayMsgRelHeader[linkEntryIdx].obj2Number = pFoundExport->idx;
					break;
				}
				}
			}
		}
	}

	return (0);
}

// check that the newly loaded isn't used by the already loaded overlays
void updateAllScriptsImports() {
	int i;

	for (i = 0; i < 90; i++) {
		if (overlayTable[i].ovlData && overlayTable[i].alreadyLoaded) {
			updateScriptImport(i);
		}
	}
}

} // End of namespace Cruise
