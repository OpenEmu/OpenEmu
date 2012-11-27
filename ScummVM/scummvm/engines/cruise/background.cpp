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

#include "common/textconsole.h"

#include "cruise/cruise_main.h"

namespace Cruise {

uint8 colorMode = 0;

uint8 *backgroundScreens[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };	// wasn't initialized in original, but it's probably better
bool backgroundChanged[8] = { false, false, false, false, false, false, false, false };
backgroundTableStruct backgroundTable[8];

char hwPage[64000];

char *hwMemAddr[] = {
	hwPage,
};

short int cvtPalette[0x20];

int loadMEN(uint8 **ptr) {
	char *localPtr = (char *) * ptr;

	if (!strcmp(localPtr, "MEN")) {
		localPtr += 4;

		titleColor = *(localPtr++);
		selectColor = *(localPtr++);
		itemColor = *(localPtr++);
		subColor = *(localPtr++);

		*ptr = (uint8 *) localPtr;

		return 1;
	} else {
		return 0;
	}
}

int CVTLoaded;

int loadCVT(uint8 **ptr) {
	char *localPtr = (char *) * ptr;

	if (!strcmp(localPtr, "CVT")) {
		int i;
		localPtr += 4;

		for (i = 0; i < 0x20; i++) {
			cvtPalette[i] = *(localPtr++);
		}

		*ptr = (uint8 *) localPtr;

		CVTLoaded = 1;

		return 1;
	} else {
		CVTLoaded = 0;
		return 0;
	}
}

extern int lastFileSize;

int loadBackground(const char *name, int idx) {
	uint8 *ptr;
	uint8 *ptr2;
	uint8 *ptrToFree;

	debug(1, "Loading BG: %s", name);

	if (!backgroundScreens[idx]) {
		backgroundScreens[idx] = (uint8 *)mallocAndZero(320 * 200);
	}

	if (!backgroundScreens[idx]) {
		backgroundTable[idx].name[0] = 0;
		return (-2);
	}

	backgroundChanged[idx] = true;

	ptrToFree = gfxModuleData.pPage10;
	if (loadFileSub1(&ptrToFree, name, NULL) < 0) {
		if (ptrToFree != gfxModuleData.pPage10)
			MemFree(ptrToFree);

		return (-18);
	}

	if (lastFileSize == 32078 || lastFileSize == 32080 || lastFileSize == 32034) {
		colorMode = 0;
	} else {
		colorMode = 1;
	}

	ptr = ptrToFree;
	ptr2 = ptrToFree;

	if (!strcmp(name, "LOGO.PI1")) {
		oldSpeedGame = speedGame;
		flagSpeed = 1;
		speedGame = 1;
	} else if (flagSpeed) {
		speedGame = oldSpeedGame;
		flagSpeed = 0;
	}

	if (!strcmp((char *)ptr, "PAL")) {
		memcpy(palScreen[idx], ptr + 4, 256*3);
		gfxModuleData_setPal256(palScreen[idx]);
	} else {
		int mode = ptr2[1];
		ptr2 += 2;
		// read palette
		switch (mode) {
		case 0:
		case 4: { // color on 3 bit
			uint16 oldPalette[32];

			memcpy(oldPalette, ptr2, 0x20);
			ptr2 += 0x20;
			flipGen(oldPalette, 0x20);

			for (unsigned long int i = 0; i < 32; i++) {
				gfxModuleData_convertOldPalColor(oldPalette[i], &palScreen[idx][i*3]);
			}

			// duplicate the palette
			for (unsigned long int i = 1; i < 8; i++) {
				memcpy(&palScreen[idx][32*i*3], &palScreen[idx][0], 32*3);
			}

			break;
		}
		case 5: { // color on 4 bit
			for (unsigned long int i = 0; i < 32; i++) {
				uint8* inPtr = ptr2 + i * 2;
				uint8* outPtr = palScreen[idx] + i * 3;

				outPtr[2] = ((inPtr[1]) & 0x0F) * 17;
				outPtr[1] = (((inPtr[1]) & 0xF0) >> 4) * 17;
				outPtr[0] = ((inPtr[0]) & 0x0F) * 17;
			}
			ptr2 += 2 * 32;

			// duplicate the palette
			for (unsigned long int i = 1; i < 8; i++) {
				memcpy(&palScreen[idx][32*i*3], &palScreen[idx][0], 32*3);
			}

			break;
		}
		case 8:
			memcpy(palScreen[idx], ptr2, 256*3);
			ptr2 += 256 * 3;
			break;

		default:
			ASSERT(0);
		}

		gfxModuleData_setPal256(palScreen[idx]);

		// read image data
		gfxModuleData_gfxClearFrameBuffer(backgroundScreens[idx]);

		switch (mode) {
		case 0:
		case 4:
			convertGfxFromMode4(ptr2, 320, 200, backgroundScreens[idx]);
			ptr2 += 32000;
			break;
		case 5:
			convertGfxFromMode5(ptr2, 320, 200, backgroundScreens[idx]);
			break;
		case 8:
			memcpy(backgroundScreens[idx], ptr2, 320 * 200);
			ptr2 += 320 * 200;
			break;
		}

		loadMEN(&ptr2);
		loadCVT(&ptr2);
	}

	MemFree(ptrToFree);

	// NOTE: the following is really meant to compare pointers and not the actual
	// strings. See r48092 and r48094.
	if (name != backgroundTable[idx].name) {
		if (strlen(name) >= sizeof(backgroundTable[idx].name))
			warning("background name length exceeded allowable maximum");

		Common::strlcpy(backgroundTable[idx].name, name, sizeof(backgroundTable[idx].name));
	}

	return (0);
}

} // End of namespace Cruise
