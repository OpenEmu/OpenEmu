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

#include "cruise/cruise_main.h"

namespace Cruise {

uint8 *_systemFNT = NULL;

uint8 itemColor = 1;
uint8 selectColor = 3;
uint8 titleColor = 2;
uint8 subColor = 5;

int16 lowMemory;
int16 scroll;
int16 switchPal;
char cmdLine[90];

int16 masterScreen;
int16 doFade;
int16 fadeFlag = 0;

preloadStruct preloadData[64];

volumeDataStruct volumeData[20];

int32 volumeDataLoaded = 0;

int16 numOfDisks;

char lastOverlay[38];
char nextOverlay[38];

int16 currentActiveMenu;
int16 autoMsg;
menuElementSubStruct* linkedRelation;
int16 main21;
int16 main22;
int16 userWait;
int16 autoTrack;

int16 currentDiskNumber = 1;

int16 volumeNumEntry;
fileEntry *volumePtrToFileDescriptor = NULL;

uint32 volumeFileDescriptorSize;
int16 volumeSizeOfEntry;
int16 volumeNumberOfEntry;

int16 displayOn = 1;

int16 protectionCode = 0;

int16 globalVars[2000];

dataFileEntry filesDatabase[NUM_FILE_ENTRIES];

int16 bootOverlayNumber;

SoundEntry soundList[4];

opcodeTypeFunction opcodeTypeTable[64];

int16 positionInStack;

actorStruct actorHead;

int16 stateID;
int16 xdial = 0;

uint8 *currentData3DataPtr;
uint8 *scriptDataPtrTable[7];

int16 currentScriptOpcodeType;

int16 saveOpcodeVar;

int16 narratorOvl = 0;
int16 narratorIdx = 0;

int16 songLoaded;
int16 songPlayed;
int16 songLoop;
int16 activeMouse;
int16 userEnabled;
int16 var5;
int16 dialogueEnabled;
int16 dialogueOvl;
int16 dialogueObj;
int16 userDelay;

int16 sysKey = -1;
int16 sysX = 0;
int16 sysY = 0;

int16 automoveInc;
int16 automoveMax;
int16 isMessage;
int16 automaticMode;
int16 aniX;
int16 aniY;
bool animationStart;

int16 autoOvl;
int16 var39;
int16 playerMenuEnabled = 0;
int16 var41;
int16 var42;
int16 var45;
int16 var46;
int16 var47;
int16 var48;
int16 flagCt;

uint8 newPal[NBCOLORS*3];
uint8 workpal[NBCOLORS*3];
uint8 palScreen[NBSCREENS][NBCOLORS*3];

//systemStringsStruct systemStrings;

char currentCtpName[40];

int16 saveVar1;
uint8 saveVar2[97];		// recheck size

int16 numberOfWalkboxes;	// saveVar3
int16 walkboxColor[15];		// saveVar4
int16 walkboxState[15];	// saveVar5

uint8 lastAni[16];

int32 loadFileVar1;

int16 loadCtFromSave = 0;
int16 ctp_routeCoordCount;	// ctpVar2
int16 ctp_routeCoords[20][2];	// ctpVar3
int16 ctp_routes[20][10];
int16 ctp_walkboxTable[15][40];	// ctpVar5
int16 walkboxColorIndex[16];
int16 walkboxZoom[15];		//  ctpVar7
int16 distanceTable[20][10];

int16 flagSpeed;
int16 speedGame;
int16 oldSpeedGame;

uint8 globalScreen[320 * 200];

//OSystem *osystem;

} // End of namespace Cruise
