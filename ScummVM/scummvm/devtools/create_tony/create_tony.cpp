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
 * This is a utility for storing all the hardcoded data of Tony Tough in a separate
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

#include "create_tony.h"
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

	outFile = fopen("tony.dat", "wb");

	// Write header
	fwrite("TONY", 4, 1, outFile);

	writeByte(outFile, TONY_DAT_VER_MAJ);
	writeByte(outFile, TONY_DAT_VER_MIN);

	// game versions/variants
	writeUint16BE(outFile, NUM_VARIANTS);

	// Italian
	for (int i = 0; i < 256; i++) {
		writeSint16BE(outFile, _cTableDialogIta[i]);
		writeSint16BE(outFile, _lTableDialogIta[i]);
		writeSint16BE(outFile, _cTableMaccIta[i]);
		writeSint16BE(outFile, _lTableMaccIta[i]);
		writeSint16BE(outFile, _cTableCredIta[i]);
		writeSint16BE(outFile, _lTableCredIta[i]);
		writeSint16BE(outFile, _cTableObjIta[i]);
		writeSint16BE(outFile, _lTableObjIta[i]);
	}

	// Polish
	for (int i = 0; i < 256; i++) {
		writeSint16BE(outFile, _cTableDialogPol[i]);
		writeSint16BE(outFile, _lTableDialogPol[i]);
		writeSint16BE(outFile, _cTableMaccPol[i]);
		writeSint16BE(outFile, _lTableMaccPol[i]);
		writeSint16BE(outFile, _cTableCredPol[i]);
		writeSint16BE(outFile, _lTableCredPol[i]);
		writeSint16BE(outFile, _cTableObjPol[i]);
		writeSint16BE(outFile, _lTableObjPol[i]);
	}

	//Russian
	for (int i = 0; i < 256; i++) {
		writeSint16BE(outFile, _cTableDialogRus[i]);
		writeSint16BE(outFile, _lTableDialogRus[i]);
		writeSint16BE(outFile, _cTableMaccRus[i]);
		writeSint16BE(outFile, _lTableMaccRus[i]);
		writeSint16BE(outFile, _cTableCredRus[i]);
		writeSint16BE(outFile, _lTableCredRus[i]);
		writeSint16BE(outFile, _cTableObjRus[i]);
		writeSint16BE(outFile, _lTableObjRus[i]);
	}

	// Czech
	for (int i = 0; i < 256; i++) {
		writeSint16BE(outFile, _cTableDialogCze[i]);
		writeSint16BE(outFile, _lTableDialogCze[i]);
		writeSint16BE(outFile, _cTableMaccCze[i]);
		writeSint16BE(outFile, _lTableMaccCze[i]);
		writeSint16BE(outFile, _cTableCredCze[i]);
		writeSint16BE(outFile, _lTableCredCze[i]);
		writeSint16BE(outFile, _cTableObjCze[i]);
		writeSint16BE(outFile, _lTableObjCze[i]);
	}

	// French
	for (int i = 0; i < 256; i++) {
		writeSint16BE(outFile, _cTableDialogFra[i]);
		writeSint16BE(outFile, _lTableDialogFra[i]);
		writeSint16BE(outFile, _cTableMaccFra[i]);
		writeSint16BE(outFile, _lTableMaccFra[i]);
		writeSint16BE(outFile, _cTableCredFra[i]);
		writeSint16BE(outFile, _lTableCredFra[i]);
		writeSint16BE(outFile, _cTableObjFra[i]);
		writeSint16BE(outFile, _lTableObjFra[i]);
	}

	// Deutsch
	for (int i = 0; i < 256; i++) {
		writeSint16BE(outFile, _cTableDialogDeu[i]);
		writeSint16BE(outFile, _lTableDialogDeu[i]);
		writeSint16BE(outFile, _cTableMaccDeu[i]);
		writeSint16BE(outFile, _lTableMaccDeu[i]);
		writeSint16BE(outFile, _cTableCredDeu[i]);
		writeSint16BE(outFile, _lTableCredDeu[i]);
		writeSint16BE(outFile, _cTableObjDeu[i]);
		writeSint16BE(outFile, _lTableObjDeu[i]);
	}

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
