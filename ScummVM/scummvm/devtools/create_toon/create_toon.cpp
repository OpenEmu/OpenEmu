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
 * This is a utility for storing all the hardcoded data of Toonstruck in a separate
 * data file, used by the game engine
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/scummsys.h"
#include "common/events.h"

#include "create_toon.h"
#include "staticdata.h"

static void writeByte(FILE *fp, uint8 b) {
	fwrite(&b, 1, 1, fp);
}

static void writeUint16BE(FILE *fp, uint16 value) {
	writeByte(fp, (uint8)(value >> 8));
	writeByte(fp, (uint8)(value & 0xFF));
}

void writeSint16BE(FILE *fp, int16 value) {
	writeUint16BE(fp, (uint16)value);
}

static void writeUint32BE(FILE *fp, uint32 value) {
	writeByte(fp, (uint8)(value >> 24));
	writeByte(fp, (uint8)((value >> 16) & 0xFF));
	writeByte(fp, (uint8)((value >> 8) & 0xFF));
	writeByte(fp, (uint8)(value & 0xFF));
}

void writeSint32BE(FILE *fp, int32 value) {
	writeUint32BE(fp, (uint16)value);
}

int main(int argc, char *argv[]) {
	FILE *outFile;
	int nbrElem;

	outFile = fopen("toon.dat", "wb");

	// Write header
	fwrite("TOON", 4, 1, outFile);

	writeByte(outFile, TOON_DAT_VER_MAJ);
	writeByte(outFile, TOON_DAT_VER_MIN);

	// game versions/variants
	writeUint16BE(outFile, NUM_VARIANTS);

	// Write locationDirNotVisited
	nbrElem = sizeof(locationDirNotVisited_EN) / sizeof(char *);
	writeTextArray(outFile, locationDirNotVisited_EN, nbrElem);

	nbrElem = sizeof(locationDirNotVisited_FR) / sizeof(char *);
	writeTextArray(outFile, locationDirNotVisited_FR, nbrElem);

	nbrElem = sizeof(locationDirNotVisited_DE) / sizeof(char *);
	writeTextArray(outFile, locationDirNotVisited_DE, nbrElem);

	nbrElem = sizeof(locationDirNotVisited_RU) / sizeof(char *);
	writeTextArray(outFile, locationDirNotVisited_RU, nbrElem);

	nbrElem = sizeof(locationDirNotVisited_SP) / sizeof(char *);
	writeTextArray(outFile, locationDirNotVisited_SP, nbrElem);

	// Write locationDirVisited
	nbrElem = sizeof(locationDirVisited_EN) / sizeof(char *);
	writeTextArray(outFile, locationDirVisited_EN, nbrElem);

	nbrElem = sizeof(locationDirVisited_FR) / sizeof(char *);
	writeTextArray(outFile, locationDirVisited_FR, nbrElem);

	nbrElem = sizeof(locationDirVisited_DE) / sizeof(char *);
	writeTextArray(outFile, locationDirVisited_DE, nbrElem);

	nbrElem = sizeof(locationDirVisited_RU) / sizeof(char *);
	writeTextArray(outFile, locationDirVisited_RU, nbrElem);

	nbrElem = sizeof(locationDirVisited_SP) / sizeof(char *);
	writeTextArray(outFile, locationDirVisited_SP, nbrElem);

	// Write specialInfoLine
	nbrElem = sizeof(specialInfoLine_EN) / sizeof(char *);
	writeTextArray(outFile, specialInfoLine_EN, nbrElem);

	nbrElem = sizeof(specialInfoLine_FR) / sizeof(char *);
	writeTextArray(outFile, specialInfoLine_FR, nbrElem);

	nbrElem = sizeof(specialInfoLine_DE) / sizeof(char *);
	writeTextArray(outFile, specialInfoLine_DE, nbrElem);

	nbrElem = sizeof(specialInfoLine_RU) / sizeof(char *);
	writeTextArray(outFile, specialInfoLine_RU, nbrElem);

	nbrElem = sizeof(specialInfoLine_SP) / sizeof(char *);
	writeTextArray(outFile, specialInfoLine_SP, nbrElem);

	// Not yet handled: miscTexts, endingLine and exitLine. Are they useful?

	fclose(outFile);
	return 0;
}

void writeTextArray(FILE *outFile, const char *textArray[], int nbrText) {
	int len, len1, pad;
	uint8 padBuf[DATAALIGNMENT];

	for (int i = 0; i < DATAALIGNMENT; i++)
		padBuf[i] = 0;

	writeUint16BE(outFile, nbrText);
	len = DATAALIGNMENT - 2;
	for (int i = 0; i < nbrText; i++) {
		len1 = strlen(textArray[i]) + 1;
		pad = DATAALIGNMENT - (len1 + 2) % DATAALIGNMENT;
		len += 2 + len1 + pad;
	}
	writeUint16BE(outFile, len);

	fwrite(padBuf, DATAALIGNMENT - 2, 1, outFile); // padding
	for (int i = 0; i < nbrText; i++) {
		len = strlen(textArray[i]) + 1;
		pad = DATAALIGNMENT - (len + 2) % DATAALIGNMENT;

		writeUint16BE(outFile, len + pad + 2);
		fwrite(textArray[i], len, 1, outFile);
		fwrite(padBuf, pad, 1, outFile);
	}
}
