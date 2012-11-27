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

#ifndef CRUISE_VARS_H
#define CRUISE_VARS_H

#include "common/file.h"

namespace Cruise {

#define NBCOLORS 256
#define NBSCREENS 8

struct menuElementSubStruct {
	struct menuElementSubStruct *pNext;
	int16 ovlIdx;
	int16 header;
};

struct menuElementStruct {
	struct menuElementStruct *next;
	const char *string;
	int x;
	int y;
	int varA;
	bool selected;
	unsigned char color;
	gfxEntryStruct *gfx;
	menuElementSubStruct *ptrSub;
};

typedef int32(*opcodeTypeFunction)();
typedef int16(*opcodeFunction)();

extern uint8 *_systemFNT;
extern int16 fontFileIndex;

extern uint8 itemColor;
extern uint8 selectColor;
extern uint8 titleColor;
extern uint8 subColor;

extern int16 lowMemory;
extern int16 scroll;
extern int16 switchPal;
extern char cmdLine[90];

extern int16 masterScreen;
extern int16 doFade;
extern int16 fadeFlag;

struct preloadStruct {
	char name[15];
	int32 size;
	int32 sourceSize;
	uint8 *ptr;
	int16 nofree;
	int16 protect;
	int16 ovl;
};

struct filesData2Struct {
	int16 field_0;
	int16 field_2;
};

struct dataFileName {
	char name[13];
};

struct setHeaderEntry {
	int32 offset;		// offset ptr
	int16 width;
	int16 height;
	int16 type;		// resource type, ie. sprites 0,1,4,5 and 8
	int16 transparency;
	int16 hotspotY;
	int16 hotspotX;
};

struct volumeDataStruct {
	char ident[10];
	dataFileName *ptr;
	int16 diskNumber;
	int32 size;
};

struct fileEntry {
	char name[14];
	int32 offset;
	int32 size;
	int32 extSize;
	int32 unk3;		// unused
};

struct dataFileEntrySub {
	uint8 *ptr;
	int16 index;		// sprite index
	char name[13];
	int16 transparency;	// sprite transparency
	uint8 *ptrMask;
	uint8 resourceType;	// sprite and image type 2,4,8 , fnt = 7, spl = 6
	int16 compression;
};

struct dataFileEntry {
	uint16 widthInColumn;
	uint16 width;
	uint16 resType;
	uint16 height;
	dataFileEntrySub subData;
};

struct SoundEntry {
	int16 frameNum;
	uint16 frequency;
	int16 volume;
};

/*
struct systemStringsStruct {
	int8 param;
	char string[12];
	char bootScriptName[8];
};
*/
extern preloadStruct preloadData[64];

extern volumeDataStruct volumeData[20];

extern int32 volumeDataLoaded;

extern int16 numOfDisks;

extern char lastOverlay[38];
extern char nextOverlay[38];

extern int16 currentActiveMenu;
extern int16 autoMsg;
extern menuElementSubStruct* linkedRelation;
extern int16 main21;
extern int16 main22;
extern int16 userWait;
extern int16 autoTrack;

extern int16 currentDiskNumber;

extern int16 volumeNumEntry;
extern fileEntry *volumePtrToFileDescriptor;

extern uint32 volumeFileDescriptorSize;
extern int16 volumeSizeOfEntry;
extern int16 volumeNumberOfEntry;

extern int16 displayOn;

extern int16 protectionCode;

#define NUM_FILE_ENTRIES 257

extern int16 globalVars[2000];
extern dataFileEntry filesDatabase[NUM_FILE_ENTRIES];

extern int16 bootOverlayNumber;

extern SoundEntry soundList[4];

extern opcodeTypeFunction opcodeTypeTable[64];

extern int16 positionInStack;
extern actorStruct actorHead;

extern int16 stateID;
extern int16 xdial;

extern uint8 *currentData3DataPtr;
extern uint8 *scriptDataPtrTable[7];

extern int16 currentScriptOpcodeType;

extern int16 saveOpcodeVar;

extern int16 narratorOvl;
extern int16 narratorIdx;

extern int16 songLoaded;
extern int16 songPlayed;
extern int16 songLoop;
extern int16 activeMouse;
extern int16 userEnabled;
extern int16 var5;
extern int16 dialogueEnabled;
extern int16 dialogueOvl;
extern int16 dialogueObj;
extern int16 userDelay;
extern int16 sysKey;
extern int16 sysX;
extern int16 sysY;
extern int16 automoveInc;
extern int16 automoveMax;
extern int16 isMessage;
extern int16 automaticMode;
extern int16 aniX;
extern int16 aniY;
extern bool animationStart;

extern int16 autoOvl;
extern int16 var39;
extern int16 playerMenuEnabled;
extern int16 var39;
extern int16 var41;
extern int16 var42;
extern int16 var45;
extern int16 var46;
extern int16 var47;
extern int16 var48;
extern int16 flagCt;

extern uint8 newPal[NBCOLORS*3];
extern uint8 workpal[NBCOLORS*3];
extern uint8 palScreen[NBSCREENS][NBCOLORS*3];

//extern systemStringsStruct systemStrings;

extern char currentCtpName[40];

extern int16 saveVar1;
extern uint8 saveVar2[97];	// recheck size

extern int16 numberOfWalkboxes;	// saveVar3
extern int16 walkboxColor[15];	// saveVar4     // Type: 0x00 - non walkable, 0x01 - walkable, 0x02 - exit zone
extern int16 walkboxState[15];	// saveVar5 // walkbox can change its type: 0x00 - not changeable, 0x01 - changeable
// Assumption: To change the type: walkboxColor[i] -= walkboxChane[i] and vice versa
extern uint8 lastAni[16];

extern int32 loadFileVar1;

extern int16 loadCtFromSave;
extern int16 ctp_routeCoordCount;	// ctpVar2  // number of path-finding coordinates
extern int16 ctp_routeCoords[20][2];	// ctpVar3      // path-finding coordinates array

/* ctp_routeCoords:

	correct size would be: ctp_routes[routeCoordCount * 4]
	coordinate information with x (2 bytes) and y (2 bytes)
*/

extern int16 ctp_routes[20][10];	// path-finding line information

/* ctp_routes:

   correct size would be: ctp_routes[routeCoordCount * 20 * 2]
   array is seperate in 20 * 2 bytes slices.
   first 2 bytes of the slice indicate how many coordinates/lines are following (lineCount)
   after that there are lineCount * 2 bytes following with indexes pointing on the routeCoords table
   the root x,y for the lines is the coordinate in the routeCoords array, which fits to the current slice
   for the 20 * i slice the root x,y is routeCoords[i], routeCoords[i+2]
   the unused rest of the slice if filled up with 0xFF
*/
extern int16 ctp_walkboxTable[15][40];	// ctpVar5      // walkboxes coordinates and lines
extern int16 walkboxColorIndex[16];
extern int16 walkboxZoom[15];	// ctpVar7  // scaling information for walkboxes
extern int16 distanceTable[20][10];

extern int16 flagSpeed;
extern int16 speedGame;
extern int16 oldSpeedGame;

extern uint8 globalScreen[320 * 200];

//extern OSystem *osystem;

} // End of namespace Cruise

#endif
