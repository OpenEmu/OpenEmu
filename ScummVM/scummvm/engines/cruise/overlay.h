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

#ifndef CRUISE_OVERLAY_H
#define CRUISE_OVERLAY_H

namespace Cruise {

struct importScriptStruct {
	uint16 var0;
	uint16 var1;
	uint16 type;
	uint16 offset;
	uint16 offsetToName;
};

struct exportEntryStruct {
	uint16 var0;
	uint16 var2;
	uint16 var4;
	uint16 idx;
	uint16 offsetToName;
};

struct ovlData3Struct {
	uint8 *dataPtr;		//0
	short int sizeOfData;	//4
	short int offsetToSubData3;	//6
	short int offsetToImportData;	//8
	short int offsetToSubData2;
	short int offsetToImportName;
	short int offsetToSubData5;
	short int sysKey;
	short int var12;
	short int numRelocGlob;
	short int subData2Size;
	short int var18;
	short int var1A;
};

struct stringEntryStruct {
	char *string;
	short int length;
	short int idx;
};

struct linkDataStruct {
	int16 type;
	int16 id;
	int16 offsetVerbeName;
	int16 verbOverlay;
	int16 verbNumber;

	int16 obj1Overlay;
	int16 obj1Number;
	int16 obj2Overlay;
	int16 obj2Number;

	int16 trackX;
	int16 trackY;

	int16 obj1NewState;
	int16 obj2NewState;

	int16 obj1OldState;
	int16 obj2OldState;

	int16 trackDirection;
	int16 dialog;
};

struct importDataStruct {
	uint16 var0;		// 0
	uint16 var1;		// 2
	uint16 linkType;	// 4
	uint16 linkIdx;		// 6
	uint16 nameOffset;
};

enum eClass {
	MULTIPLE = 0,
	VARIABLE = 1,
	UNIQUE = 2,
	THEME = 3
};

struct objDataStruct {
	int16 _type;
	eClass _class;
	int16 _nameOffset;
	int16 _numStates;
	int16 _varTableIdx;
	int16 _firstStateIdx;
	int16 _stateTableIdx;
};

struct objectParams {
	int16 X;
	int16 Y;
	int16 Z;
	int16 frame;
	int16 scale;
	int16 state;
};

struct ovlDataStruct {

	ovlData3Struct *arrayProc;
	ovlData3Struct *ptr1;
	objDataStruct *arrayObject;
	objectParams *arrayStates;
	objectParams *arrayObjVar;
	stringEntryStruct *stringTable;
	exportEntryStruct *arraySymbGlob;
	importDataStruct *arrayRelocGlob;
	linkDataStruct *arrayMsgRelHeader;

	char *nameVerbGlob;
	char *arrayNameObj;
	char *arrayNameRelocGlob;
	char *arrayNameSymbGlob;

	uint8 *data4Ptr;
	uint8 *ptr8;

	unsigned short int numProc;
	unsigned short int numRel;
	unsigned short int numSymbGlob;
	unsigned short int numRelocGlob;
	unsigned short int numMsgRelHeader;
	unsigned short int numObj;
	unsigned short int numStrings;
	unsigned short int size8;
	unsigned short int size9;

	unsigned short int nameExportSize;
	unsigned short int exportNamesSize;
	unsigned short int specialString2Length;
	unsigned short int sizeOfData4;
	unsigned short int size12;
	unsigned short int specialString1Length;
	unsigned short int scriptNumber;
};

struct overlayStruct {
	char overlayName[13];
	ovlDataStruct *ovlData;
	short int alreadyLoaded;
	short int state;
	char* pDebug;
	long int debugSize;
	char* pSource;
	long int sourceSize;
	short int executeScripts;
};

extern overlayStruct overlayTable[90];
extern int numOfLoadedOverlay;

void initOverlayTable();
int loadOverlay(const char * scriptName);
int32 findOverlayByName2(const char * name);
int findOverlayByName(const char *overlayName);
int releaseOverlay(const char *name);
int freeOverlay(int overlayIdx);
void freeOverlayTable();

} // End of namespace Cruise

#endif
