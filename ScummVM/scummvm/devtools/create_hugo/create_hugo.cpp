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
 * This is a utility for storing all the hardcoded data of Hugo in a separate
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

#include "enums.h"

#include "create_hugo.h"
#include "staticdata.h"
#include "staticdisplay.h"
#include "staticengine.h"
#include "staticintro.h"
#include "staticmouse.h"
#include "staticparser.h"
#include "staticutil.h"
#include "staticfont.h"

static void writeByte(FILE *fp, uint8 b) {
	fwrite(&b, 1, 1, fp);
}

static void writeSByte(FILE *fp, int8 b) {
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
	int i;
	int nbrElem, nbrSubElem;

	outFile = fopen("hugo.dat", "wb");

	// Write header
	fwrite("HUGO", 4, 1, outFile);

	writeByte(outFile, HUGO_DAT_VER_MAJ);
	writeByte(outFile, HUGO_DAT_VER_MIN);

	// game versions/variantes
	writeUint16BE(outFile, NUM_VARIANTE);

	// Write palette
	writeUint16BE(outFile, SIZE_PAL_ARRAY);
	for (i = 0; i < SIZE_PAL_ARRAY; i++) {
		writeByte(outFile, _palette[i]);
	}

	// The following fonts info have been added to avoid temporarly the .FON
	// used in the DOS version
	// font5
	nbrElem = sizeof(font5) / sizeof(byte);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeByte(outFile, font5[j]);

	// font6
	nbrElem = sizeof(font6) / sizeof(byte);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeByte(outFile, font6[j]);

	// font8
	nbrElem = sizeof(font8) / sizeof(byte);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeByte(outFile, font8[j]);

	// Write textData
	// textData_1w
	nbrElem = sizeof(textData_1w) / sizeof(char *);
	writeTextArray(outFile, textData_1w, nbrElem);

	// textData_2w
	nbrElem = sizeof(textData_2w) / sizeof(char *);
	writeTextArray(outFile, textData_2w, nbrElem);

	// textData_3w
	nbrElem = sizeof(textData_3w) / sizeof(char *);
	writeTextArray(outFile, textData_3w, nbrElem);

	// textData_1d
	nbrElem = sizeof(textData_1d) / sizeof(char *);
	writeTextArray(outFile, textData_1d, nbrElem);

	// textData_2d
	nbrElem = sizeof(textData_2d) / sizeof(char *);
	writeTextArray(outFile, textData_2d, nbrElem);

	// textData_3d
	nbrElem = sizeof(textData_3d) / sizeof(char *);
	writeTextArray(outFile, textData_3d, nbrElem);

	// Write string_t_Data
	// string_t_Data_1w
	nbrElem = sizeof(string_t_Data_1w) / sizeof(char *);
	writeTextArray(outFile, string_t_Data_1w, nbrElem);

	// string_t_Data_2w
	nbrElem = sizeof(string_t_Data_2w) / sizeof(char *);
	writeTextArray(outFile, string_t_Data_2w, nbrElem);

	// string_t_Data_3w
	nbrElem = sizeof(string_t_Data_3w) / sizeof(char *);
	writeTextArray(outFile, string_t_Data_3w, nbrElem);

	// string_t_Data_1d
	nbrElem = sizeof(string_t_Data_1d) / sizeof(char *);
	writeTextArray(outFile, string_t_Data_1d, nbrElem);

	// string_t_Data_2d
	nbrElem = sizeof(string_t_Data_2d) / sizeof(char *);
	writeTextArray(outFile, string_t_Data_2d, nbrElem);

	// string_t_Data_3d
	nbrElem = sizeof(string_t_Data_3d) / sizeof(char *);
	writeTextArray(outFile, string_t_Data_3d, nbrElem);

	// arrayNouns_1w
	nbrElem = sizeof(arrayNouns_1w) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayNouns_1w[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayNouns_1w[j], nbrSubElem);
	}

	// arrayNouns_2w
	nbrElem = sizeof(arrayNouns_2w) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayNouns_2w[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayNouns_2w[j], nbrSubElem);
	}

	// arrayNouns_3w
	nbrElem = sizeof(arrayNouns_3w) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayNouns_3w[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayNouns_3w[j], nbrSubElem);
	}

	// arrayNouns_1d
	nbrElem = sizeof(arrayNouns_1d) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayNouns_1d[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayNouns_1d[j], nbrSubElem);
	}

	// arrayNouns_2d
	nbrElem = sizeof(arrayNouns_2d) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayNouns_2d[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayNouns_2d[j], nbrSubElem);
	}

	// arrayNouns_3d
	nbrElem = sizeof(arrayNouns_3d) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayNouns_3d[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayNouns_3d[j], nbrSubElem);
	}

	// arrayVerbs_1w
	nbrElem = sizeof(arrayVerbs_1w) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayVerbs_1w[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayVerbs_1w[j], nbrSubElem);
	}

	// arrayVerbs_2w
	nbrElem = sizeof(arrayVerbs_2w) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayVerbs_2w[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayVerbs_2w[j], nbrSubElem);
	}

	// arrayVerbs_3w
	nbrElem = sizeof(arrayVerbs_3w) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayVerbs_3w[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayVerbs_3w[j], nbrSubElem);
	}

	// arrayVerbs_1d
	nbrElem = sizeof(arrayVerbs_1d) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayVerbs_1d[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayVerbs_1d[j], nbrSubElem);
	}

	// arrayVerbs_2d
	nbrElem = sizeof(arrayVerbs_2d) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayVerbs_2d[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayVerbs_2d[j], nbrSubElem);
	}

	// arrayVerbs_3d
	nbrElem = sizeof(arrayVerbs_3d) / sizeof(char **);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; strcmp(arrayVerbs_3d[j][nbrSubElem], ""); nbrSubElem++)
			;
		nbrSubElem++;
		writeTextArray(outFile, arrayVerbs_3d[j], nbrSubElem);
	}

	// Write screenNames
	// screenNames_1w
	nbrElem = sizeof(screenNames_1w) / sizeof(char *);
	writeTextArray(outFile, screenNames_1w, nbrElem);

	// screenNames_2w
	nbrElem = sizeof(screenNames_2w) / sizeof(char *);
	writeTextArray(outFile, screenNames_2w, nbrElem);

	// screenNames_3w
	nbrElem = sizeof(screenNames_3w) / sizeof(char *);
	writeTextArray(outFile, screenNames_3w, nbrElem);

	// screenNames_1d
	nbrElem = sizeof(screenNames_1d) / sizeof(char *);
	writeTextArray(outFile, screenNames_1d, nbrElem);

	// screenNames_2d
	nbrElem = sizeof(screenNames_2d) / sizeof(char *);
	writeTextArray(outFile, screenNames_2d, nbrElem);

	// screenNames_3d
	nbrElem = sizeof(screenNames_3d) / sizeof(char *);
	writeTextArray(outFile, screenNames_3d, nbrElem);

	// Write textEngine
	writeTextArray(outFile, textEngine, NUM_ENGINE_TEXT);

	// Write textIntro
	writeTextArray(outFile, textIntro_dummy, NUM_INTRO_TEXT_DUMMY);
	writeTextArray(outFile, textIntro_dummy, NUM_INTRO_TEXT_DUMMY);
	writeTextArray(outFile, textIntro_v3, NUM_INTRO_TEXT_V3);
	writeTextArray(outFile, textIntro_dummy, NUM_INTRO_TEXT_DUMMY);
	writeTextArray(outFile, textIntro_dummy, NUM_INTRO_TEXT_DUMMY);
	writeTextArray(outFile, textIntro_v3, NUM_INTRO_TEXT_V3);

	// Write textMouse
	writeTextArray(outFile, textMouse, NUM_MOUSE_TEXT);

	// Write textParser
	writeTextArray(outFile, textParser, NUM_PARSER_TEXT);

	// Write textUtil
	writeTextArray(outFile, textUtil_v1w, NUM_UTIL_TEXT);
	writeTextArray(outFile, textUtil_v1w, NUM_UTIL_TEXT);
	writeTextArray(outFile, textUtil_v1w, NUM_UTIL_TEXT);
	writeTextArray(outFile, textUtil_v1d, NUM_UTIL_TEXT);
	writeTextArray(outFile, textUtil_v1d, NUM_UTIL_TEXT);
	writeTextArray(outFile, textUtil_v1d, NUM_UTIL_TEXT);

	// Write x_intro and y_intro
	writeUint16BE(outFile, NUM_INTRO_TICK_DUMMY);
	for (i = 0; i < NUM_INTRO_TICK_DUMMY; i++) {
		writeByte(outFile, x_intro_dummy[i]);
		writeByte(outFile, y_intro_dummy[i]);
	}

	writeUint16BE(outFile, NUM_INTRO_TICK_DUMMY);
	for (i = 0; i < NUM_INTRO_TICK_DUMMY; i++) {
		writeByte(outFile, x_intro_dummy[i]);
		writeByte(outFile, y_intro_dummy[i]);
	}

	writeUint16BE(outFile, NUM_INTRO_TICK_V3);
	for (i = 0; i < NUM_INTRO_TICK_V3; i++) {
		writeByte(outFile, x_intro_v3[i]);
		writeByte(outFile, y_intro_v3[i]);
	}

	writeUint16BE(outFile, NUM_INTRO_TICK_V1D);
	for (i = 0; i < NUM_INTRO_TICK_V1D; i++) {
		writeByte(outFile, x_intro_v1d[i]);
		writeByte(outFile, y_intro_v1d[i]);
	}

	writeUint16BE(outFile, NUM_INTRO_TICK_DUMMY);
	for (i = 0; i < NUM_INTRO_TICK_DUMMY; i++) {
		writeByte(outFile, x_intro_dummy[i]);
		writeByte(outFile, y_intro_dummy[i]);
	}

	writeUint16BE(outFile, NUM_INTRO_TICK_V3);
	for (i = 0; i < NUM_INTRO_TICK_V3; i++) {
		writeByte(outFile, x_intro_v3[i]);
		writeByte(outFile, y_intro_v3[i]);
	}

	// arrayReqs_1w
	nbrElem = sizeof(arrayReqs_1w) / sizeof(uint16 *);
	writeUint16Array(outFile, arrayReqs_1w, nbrElem);

	// arrayReqs_2w
	nbrElem = sizeof(arrayReqs_2w) / sizeof(uint16 *);
	writeUint16Array(outFile, arrayReqs_2w, nbrElem);

	// arrayReqs_3w
	nbrElem = sizeof(arrayReqs_3w) / sizeof(uint16 *);
	writeUint16Array(outFile, arrayReqs_3w, nbrElem);

	// arrayReqs_1d
	nbrElem = sizeof(arrayReqs_1d) / sizeof(uint16 *);
	writeUint16Array(outFile, arrayReqs_1d, nbrElem);

	// arrayReqs_2d
	nbrElem = sizeof(arrayReqs_2d) / sizeof(uint16 *);
	writeUint16Array(outFile, arrayReqs_2d, nbrElem);

	// arrayReqs_3d
	nbrElem = sizeof(arrayReqs_3d) / sizeof(uint16 *);
	writeUint16Array(outFile, arrayReqs_3d, nbrElem);

	// catchall_1w
	nbrElem = sizeof(catchall_1w) / sizeof(background_t);
	writeBackgroundArray(outFile, catchall_1w, nbrElem);

	// catchall_2w
	nbrElem = sizeof(catchall_2w) / sizeof(background_t);
	writeBackgroundArray(outFile, catchall_2w, nbrElem);

	// catchall_3w
	nbrElem = sizeof(catchall_3w) / sizeof(background_t);
	writeBackgroundArray(outFile, catchall_3w, nbrElem);

	// catchall_1d
	nbrElem = sizeof(catchall_1d) / sizeof(background_t);
	writeBackgroundArray(outFile, catchall_1d, nbrElem);

	// catchall_2d
	nbrElem = sizeof(catchall_2d) / sizeof(background_t);
	writeBackgroundArray(outFile, catchall_2d, nbrElem);

	// catchall_3d
	nbrElem = sizeof(catchall_3d) / sizeof(background_t);
	writeBackgroundArray(outFile, catchall_3d, nbrElem);

	// backgroundList_1w
	nbrElem = sizeof(backgroundList_1w) / sizeof(background_t *);
	writeUint16BE(outFile, nbrElem);
	for (int j = 0; j < nbrElem; j++) {
		nbrSubElem = 1;
		for (int k = 0; backgroundList_1w[j][k].verbIndex != 0; k++)
			nbrSubElem ++;
		writeBackgroundArray(outFile, backgroundList_1w[j], nbrSubElem);
	}

	// backgroundList_2w
	nbrElem = sizeof(backgroundList_2w) / sizeof(background_t *);
	writeUint16BE(outFile, nbrElem);
	for (int j = 0; j < nbrElem; j++) {
		nbrSubElem = 1;
		for (int k = 0; backgroundList_2w[j][k].verbIndex != 0; k++)
			nbrSubElem ++;
		writeBackgroundArray(outFile, backgroundList_2w[j], nbrSubElem);
	}

	// backgroundList_3w
	nbrElem = sizeof(backgroundList_3w) / sizeof(background_t *);
	writeUint16BE(outFile, nbrElem);
	for (int j = 0; j < nbrElem; j++) {
		nbrSubElem = 1;
		for (int k = 0; backgroundList_3w[j][k].verbIndex != 0; k++)
			nbrSubElem ++;
		writeBackgroundArray(outFile, backgroundList_3w[j], nbrSubElem);
	}

	// backgroundList_1d
	nbrElem = sizeof(backgroundList_1d) / sizeof(background_t *);
	writeUint16BE(outFile, nbrElem);
	for (int j = 0; j < nbrElem; j++) {
		nbrSubElem = 1;
		for (int k = 0; backgroundList_1d[j][k].verbIndex != 0; k++)
			nbrSubElem ++;
		writeBackgroundArray(outFile, backgroundList_1d[j], nbrSubElem);
	}

	// backgroundList_2d
	nbrElem = sizeof(backgroundList_2d) / sizeof(background_t *);
	writeUint16BE(outFile, nbrElem);
	for (int j = 0; j < nbrElem; j++) {
		nbrSubElem = 1;
		for (int k = 0; backgroundList_2d[j][k].verbIndex != 0; k++)
			nbrSubElem ++;
		writeBackgroundArray(outFile, backgroundList_2d[j], nbrSubElem);
	}

	// backgroundList_3d
	nbrElem = sizeof(backgroundList_3d) / sizeof(background_t *);
	writeUint16BE(outFile, nbrElem);
	for (int j = 0; j < nbrElem; j++) {
		nbrSubElem = 1;
		for (int k = 0; backgroundList_3d[j][k].verbIndex != 0; k++)
			nbrSubElem ++;
		writeBackgroundArray(outFile, backgroundList_3d[j], nbrSubElem);
	}

	// cmdList_1w
	nbrElem = sizeof(cmdList_1w) / sizeof(cmd **);
	writeCmdArray(outFile, cmdList_1w, nbrElem);

	// cmdList_2w
	nbrElem = sizeof(cmdList_2w) / sizeof(cmd **);
	writeCmdArray(outFile, cmdList_2w, nbrElem);

	// cmdList_3w
	nbrElem = sizeof(cmdList_3w) / sizeof(cmd **);
	writeCmdArray(outFile, cmdList_3w, nbrElem);

	// cmdList_1d
	nbrElem = sizeof(cmdList_1d) / sizeof(cmd **);
	writeCmdArray(outFile, cmdList_1d, nbrElem);

	// cmdList_2d
	nbrElem = sizeof(cmdList_2d) / sizeof(cmd **);
	writeCmdArray(outFile, cmdList_2d, nbrElem);

	// cmdList_3d
	nbrElem = sizeof(cmdList_3d) / sizeof(cmd **);
	writeCmdArray(outFile, cmdList_3d, nbrElem);

	// hotspots_1w
	nbrElem = sizeof(hotspots_1w) / sizeof(hotspot_t);
	writeHotspot(outFile, hotspots_1w, nbrElem);

	// hotspots_2w
	nbrElem = sizeof(hotspots_2w) / sizeof(hotspot_t);
	writeHotspot(outFile, hotspots_2w, nbrElem);

	// hotspots_3w
	nbrElem = sizeof(hotspots_3w) / sizeof(hotspot_t);
	writeHotspot(outFile, hotspots_3w, nbrElem);

	// hotspots_1d
	nbrElem = sizeof(hotspots_1d) / sizeof(hotspot_t);
	writeHotspot(outFile, hotspots_1d, nbrElem);

	// hotspots_2d
	nbrElem = sizeof(hotspots_2d) / sizeof(hotspot_t);
	writeHotspot(outFile, hotspots_2d, nbrElem);

	// hotspots_3d
	nbrElem = sizeof(hotspots_3d) / sizeof(hotspot_t);
	writeHotspot(outFile, hotspots_3d, nbrElem);

	// invent_1w
	nbrElem = sizeof(invent_1w) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, invent_1w[j]);

	// invent_2w
	nbrElem = sizeof(invent_2w) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, invent_2w[j]);

	// invent_3w
	nbrElem = sizeof(invent_3w) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, invent_3w[j]);

	// invent_1d
	nbrElem = sizeof(invent_1d) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, invent_1d[j]);

	// invent_2d
	nbrElem = sizeof(invent_2d) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, invent_2d[j]);

	// invent_3d
	nbrElem = sizeof(invent_3d) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, invent_3d[j]);

	// uses_1w
	nbrElem = sizeof(uses_1w) / sizeof(uses_t);
	writeUseArray(outFile, uses_1w, nbrElem);

	// uses_2w
	nbrElem = sizeof(uses_2w) / sizeof(uses_t);
	writeUseArray(outFile, uses_2w, nbrElem);

	// uses_3w
	nbrElem = sizeof(uses_3w) / sizeof(uses_t);
	writeUseArray(outFile, uses_3w, nbrElem);

	// uses_1d
	nbrElem = sizeof(uses_1d) / sizeof(uses_t);
	writeUseArray(outFile, uses_1d, nbrElem);

	// uses_2d
	nbrElem = sizeof(uses_2d) / sizeof(uses_t);
	writeUseArray(outFile, uses_2d, nbrElem);

	// uses_3d
	nbrElem = sizeof(uses_3d) / sizeof(uses_t);
	writeUseArray(outFile, uses_3d, nbrElem);

	// objects_1w
	nbrElem = sizeof(objects_1w) / sizeof(object_t);
	writeObjectArray(outFile, objects_1w, nbrElem);

	// objects_2w
	nbrElem = sizeof(objects_2w) / sizeof(object_t);
	writeObjectArray(outFile, objects_2w, nbrElem);

	// objects_3w
	nbrElem = sizeof(objects_3w) / sizeof(object_t);
	writeObjectArray(outFile, objects_3w, nbrElem);

	// objects_1d
	nbrElem = sizeof(objects_1d) / sizeof(object_t);
	writeObjectArray(outFile, objects_1d, nbrElem);

	// objects_2d
	nbrElem = sizeof(objects_2d) / sizeof(object_t);
	writeObjectArray(outFile, objects_2d, nbrElem);

	// objects_3d
	nbrElem = sizeof(objects_3d) / sizeof(object_t);
	writeObjectArray(outFile, objects_3d, nbrElem);

	// Save LASTOBJ
	writeUint16BE(outFile, LASTOBJ_1w);
	writeUint16BE(outFile, LASTOBJ_2w);
	writeUint16BE(outFile, LASTOBJ_3w);
	writeUint16BE(outFile, LASTOBJ_1d);   //(not set in original, as Hugo1 DOS doesn't use a DAT file to pack the screens)
	writeUint16BE(outFile, LASTOBJ_2d);
	writeUint16BE(outFile, LASTOBJ_3d);

	// points_1w
	nbrElem = sizeof(points_1w) / sizeof(byte);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeByte(outFile, points_1w[j]);

	// points_2w
	nbrElem = sizeof(points_2w) / sizeof(byte);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeByte(outFile, points_2w[j]);

	// points_3w
	nbrElem = sizeof(points_3w) / sizeof(byte);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeByte(outFile, points_3w[j]);

	// points_1d
	nbrElem = sizeof(points_1d) / sizeof(byte);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeByte(outFile, points_1d[j]);

	// points_2d
	nbrElem = sizeof(points_2d) / sizeof(byte);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeByte(outFile, points_2d[j]);

	// points_3d
	nbrElem = sizeof(points_3d) / sizeof(byte);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeByte(outFile, points_3d[j]);

	// screenActs_1w
	nbrElem = sizeof(screenActs_1w) / sizeof(uint16 *);
	writeScreenActs(outFile, screenActs_1w, nbrElem);

	// screenActs_2w
	nbrElem = sizeof(screenActs_2w) / sizeof(uint16 *);
	writeScreenActs(outFile, screenActs_2w, nbrElem);

	// screenActs_3w
	nbrElem = sizeof(screenActs_3w) / sizeof(uint16 *);
	writeScreenActs(outFile, screenActs_3w, nbrElem);

	// screenActs_1d
	nbrElem = sizeof(screenActs_1d) / sizeof(uint16 *);
	writeScreenActs(outFile, screenActs_1d, nbrElem);

	// screenActs_2d
	nbrElem = sizeof(screenActs_2d) / sizeof(uint16 *);
	writeScreenActs(outFile, screenActs_2d, nbrElem);

	// screenActs_3d
	nbrElem = sizeof(screenActs_3d) / sizeof(uint16 *);
	writeScreenActs(outFile, screenActs_3d, nbrElem);

	// actlistArr_1w
	nbrElem = sizeof(actListArr_1w) / sizeof(actList);
	writeActListArray(outFile, actListArr_1w, nbrElem);

	// actlistArr_2w
	nbrElem = sizeof(actListArr_2w) / sizeof(actList);
	writeActListArray(outFile, actListArr_2w, nbrElem);

	// actlistArr_3w
	nbrElem = sizeof(actListArr_3w) / sizeof(actList);
	writeActListArray(outFile, actListArr_3w, nbrElem);

	// actlistArr_1d
	nbrElem = sizeof(actListArr_1d) / sizeof(actList);
	writeActListArray(outFile, actListArr_1d, nbrElem);

	// actlistArr_2d
	nbrElem = sizeof(actListArr_2d) / sizeof(actList);
	writeActListArray(outFile, actListArr_2d, nbrElem);

	// actlistArr_3d
	nbrElem = sizeof(actListArr_3d) / sizeof(actList);
	writeActListArray(outFile, actListArr_3d, nbrElem);

	// Maze ALnewscr
	writeUint16BE(outFile, 0);
	writeUint16BE(outFile, kALnewscr_2w);
	writeUint16BE(outFile, 0);
	writeUint16BE(outFile, 0);
	writeUint16BE(outFile, kALnewscr_2d);
	writeUint16BE(outFile, 0);

	writeSByte(outFile, NUM_TUNES_1w);
	writeSByte(outFile, SILENCE_1w);
	writeSByte(outFile, TEST_SOUND_1w);

	writeSByte(outFile, NUM_TUNES_2w);
	writeSByte(outFile, SILENCE_2w);
	writeSByte(outFile, TEST_SOUND_2w);

	writeSByte(outFile, NUM_TUNES_3w);
	writeSByte(outFile, SILENCE_3w);
	writeSByte(outFile, TEST_SOUND_3w);

	writeSByte(outFile, NUM_TUNES_1d);
	writeSByte(outFile, SILENCE_1d);
	writeSByte(outFile, TEST_SOUND_1d);

	writeSByte(outFile, NUM_TUNES_2d);
	writeSByte(outFile, SILENCE_2d);
	writeSByte(outFile, TEST_SOUND_2d);

	writeSByte(outFile, NUM_TUNES_3d);
	writeSByte(outFile, SILENCE_3d);
	writeSByte(outFile, TEST_SOUND_3d);

	// def_tunes_1w
	nbrElem = sizeof(def_tunes_1w) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, def_tunes_1w[j]);

	// def_tunes_2w
	nbrElem = sizeof(def_tunes_2w) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, def_tunes_2w[j]);

	// def_tunes_3w
	nbrElem = sizeof(def_tunes_3w) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, def_tunes_3w[j]);

	// def_tunes_1d
	nbrElem = sizeof(def_tunes_1d) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, def_tunes_1d[j]);

	// def_tunes_2d
	nbrElem = sizeof(def_tunes_2d) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, def_tunes_2d[j]);

	// def_tunes_3d
	nbrElem = sizeof(def_tunes_3d) / sizeof(int16);
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++)
		writeSint16BE(outFile, def_tunes_3d[j]);

	// Save _screnStates array size
	writeUint16BE(outFile, LASTOBJ_1w);
	writeUint16BE(outFile, LASTOBJ_2w);
	writeUint16BE(outFile, NUM_PICS_3w);

	writeUint16BE(outFile, NUM_PICS_1d);   //(not set in original, as Hugo1 DOS doesn't use a DAT file to pack the screens)
	writeUint16BE(outFile, LASTOBJ_2d);
	writeUint16BE(outFile, NUM_PICS_3d);

	// Save Look, Take and Drop constants
	writeUint16BE(outFile, kVLook_1w);
	writeUint16BE(outFile, kVTake_1w);
	writeUint16BE(outFile, kVDrop_1w);

	writeUint16BE(outFile, kVLook_2w);
	writeUint16BE(outFile, kVTake_2w);
	writeUint16BE(outFile, kVDrop_2w);

	writeUint16BE(outFile, kVLook_3w);
	writeUint16BE(outFile, kVTake_3w);
	writeUint16BE(outFile, kVDrop_3w);

	writeUint16BE(outFile, kVLook_1d);
	writeUint16BE(outFile, kVTake_1d);
	writeUint16BE(outFile, kVDrop_1d);

	writeUint16BE(outFile, kVLook_2d);
	writeUint16BE(outFile, kVTake_2d);
	writeUint16BE(outFile, kVDrop_2d);

	writeUint16BE(outFile, kVLook_3d);
	writeUint16BE(outFile, kVTake_3d);
	writeUint16BE(outFile, kVDrop_3d);

	// DOS Intro music
	// Win version do not use it
	// H1 Dos doesn't have an intro
	// H2 Dos handles the intro song in its scripts
	writeUint16BE(outFile, 0);
	writeUint16BE(outFile, 0);
	writeUint16BE(outFile, 0);
	writeUint16BE(outFile, 0);
	writeUint16BE(outFile, 0);
	writeUint16BE(outFile, kDTsong11_3d);

	//bitmap images for menu
	writeUint16BE(outFile, 18);

	FILE* src_file;
	char buf[2];
	src_file = fopen("btn_1.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_1.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_1_off.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_1_off.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_2.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_2.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_2_off.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_2_off.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_3.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_3.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_3_off.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_3_off.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_4.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_4.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_4_off.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_4_off.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_5.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_5.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_5_off.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_5_off.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_6.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_6.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_6_off.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_6_off.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_7.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_7.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_7_off.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_7_off.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_8.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_8.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_8_off.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_8_off.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_9.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_9.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

	src_file = fopen("btn_9_off.bmp", "rb");
	if (src_file == NULL) {
		perror("btn_9_off.bmp");
		return -1;
	}
	fseek(src_file , 0 , SEEK_END);
	nbrElem = ftell(src_file);
	writeUint16BE(outFile, nbrElem);
	rewind(src_file);
	for (int j = 0; j < nbrElem; j++) {
		fread(buf, 1, 1, src_file);
		writeByte(outFile, buf[0]);
	}
	fclose(src_file);

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

void writeUint16Array(FILE *outFile, const uint16 *uint16Array[], int nbrElem) {
	int nbrSubElem;

	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		for (nbrSubElem = 1; uint16Array[j][nbrSubElem] != 0; nbrSubElem++)
			;
		nbrSubElem++;
		writeUint16BE(outFile, nbrSubElem);

		for (int i = 0; i < nbrSubElem; i++) {
			writeUint16BE(outFile, uint16Array[j][i]);
		}
	}
}

void writeHotspot(FILE *outFile, const hotspot_t hotspots[], int nbrElem) {
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		writeSint16BE(outFile, hotspots[j].screenIndex);
		writeSint16BE(outFile, hotspots[j].x1);
		writeSint16BE(outFile, hotspots[j].y1);
		writeSint16BE(outFile, hotspots[j].x2);
		writeSint16BE(outFile, hotspots[j].y2);
		writeUint16BE(outFile, hotspots[j].actIndex);
		writeSint16BE(outFile, hotspots[j].viewx);
		writeSint16BE(outFile, hotspots[j].viewy);
		writeSint16BE(outFile, hotspots[j].direction);
	}
}

void writeUseArray(FILE *outFile, const uses_t uses[], int nbrElem) {
	int nbrSubElem;
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		writeSint16BE(outFile, uses[j].objid);
		writeUint16BE(outFile, uses[j].dataIndex);

		nbrSubElem = sizeof(uses_1w[j].targets) / sizeof(target_t);
		writeUint16BE(outFile, nbrSubElem);
		for (int k = 0; k < nbrSubElem; k++) {
			writeUint16BE(outFile, uses[j].targets[k].nounIndex);
			writeUint16BE(outFile, uses[j].targets[k].verbIndex);
		}
	}
}

void writeBackgroundArray(FILE *outFile, const background_t background[], int nbrElem) {
	writeUint16BE(outFile, nbrElem);

	for (int j = 0; j < nbrElem; j++) {
		writeUint16BE(outFile, background[j].verbIndex);
		writeUint16BE(outFile, background[j].nounIndex);
		writeSint16BE(outFile, background[j].commentIndex);
		writeByte(outFile, (background[j].matchFl) ? 1 : 0);
		writeByte(outFile, background[j].roomState);
		writeByte(outFile, background[j].bonusIndex);
	}
}

void writeCmdArray(FILE *outFile, const cmd *cmdList[], int nbrElem) {
	int nbrSubElem;

	writeUint16BE(outFile, nbrElem);
	for (int i = 0; i < nbrElem; i++) {
		nbrSubElem = 1;
		for (int j = 0; cmdList[i][j].verbIndex != 0; j++)
			nbrSubElem ++;
		writeUint16BE(outFile, nbrSubElem);
		for (int j = 0; j < nbrSubElem; j++) {
			writeUint16BE(outFile, cmdList[i][j].verbIndex);
			writeUint16BE(outFile, cmdList[i][j].reqIndex);
			writeUint16BE(outFile, cmdList[i][j].textDataNoCarryIndex);
			writeByte(outFile, cmdList[i][j].reqstate);
			writeByte(outFile, cmdList[i][j].newstate);
			writeUint16BE(outFile, cmdList[i][j].textDataWrongIndex);
			writeUint16BE(outFile, cmdList[i][j].textDataDoneIndex);
			writeUint16BE(outFile, cmdList[i][j].actIndex);
		}
	}
}

void writeScreenActs(FILE *outFile, const uint16 *screenActs[], int nbrElem) {
	int nbrSubElem;

	writeUint16BE(outFile, nbrElem);
	for (int i = 0; i < nbrElem; i++) {
		if (screenActs[i] == 0)
			writeUint16BE(outFile, 0);
		else {
			nbrSubElem = 1;
			for (int j = 0; screenActs[i][j] != 0; j++)
				nbrSubElem ++;
			writeUint16BE(outFile, nbrSubElem);
			for (int j = 0; j < nbrSubElem; j++)
				writeUint16BE(outFile, screenActs[i][j]);
		}
	}
}

void writeObjectArray(FILE *outFile, const object_t objects[], int nbrElem) {
	int nbrSubElem;

	writeUint16BE(outFile, nbrElem);
	for (int i = 0; i < nbrElem; i++) {
		writeUint16BE(outFile, objects[i].nounIndex);
		writeUint16BE(outFile, objects[i].dataIndex);
		if (objects[i].stateDataIndex == NULL)
			writeUint16BE(outFile, 0);
		else {
			nbrSubElem = 1;
			for (int j = 0; objects[i].stateDataIndex[j] != 0; j++)
				nbrSubElem ++;
			writeUint16BE(outFile, nbrSubElem);
			for (int j = 0; j < nbrSubElem; j++)
				writeUint16BE(outFile, objects[i].stateDataIndex[j]);
		}
		writeSint16BE(outFile, objects[i].pathType);
		writeSint16BE(outFile, objects[i].vxPath);
		writeSint16BE(outFile, objects[i].vyPath);
		writeUint16BE(outFile, objects[i].actIndex);
		writeByte(outFile, objects[i].seqNumb);
// curr_seq_p is skipped: always equal to zero during initialization
		for (int j = 0; j < objects[i].seqNumb; j++)
			writeUint16BE(outFile, objects[i].seqList[j].imageNbr);
// seq_list[].seq_p is always null during initialization, thus skipped
		writeByte(outFile, objects[i].cycling);
		writeByte(outFile, objects[i].cycleNumb);
		writeByte(outFile, objects[i].frameInterval);
		writeByte(outFile, objects[i].frameTimer);
		writeByte(outFile, objects[i].radius);
		writeByte(outFile, objects[i].screenIndex);
		writeSint16BE(outFile, objects[i].x);
		writeSint16BE(outFile, objects[i].y);
		writeSint16BE(outFile, objects[i].oldx);
		writeSint16BE(outFile, objects[i].oldy);
		writeByte(outFile, objects[i].vx);
		writeByte(outFile, objects[i].vy);
		writeByte(outFile, objects[i].objValue);
		writeSint16BE(outFile, objects[i].genericCmd);
		writeUint16BE(outFile, objects[i].cmdIndex);
		writeByte(outFile, (objects[i].carriedFl) ? 1 : 0);
		writeByte(outFile, objects[i].state);
		writeByte(outFile, (objects[i].verbOnlyFl) ? 1 : 0);
		writeByte(outFile, objects[i].priority);
		writeSint16BE(outFile, objects[i].viewx);
		writeSint16BE(outFile, objects[i].viewy);
		writeSint16BE(outFile, objects[i].direction);
		writeByte(outFile, objects[i].curSeqNumb);
		writeByte(outFile, objects[i].curImageNumb);
		writeByte(outFile, objects[i].oldvx);
		writeByte(outFile, objects[i].oldvy);
	}
}

void writeActListArray(FILE *outFile, const actList actListArr[], int nbrElem) {
	int nbrSubElem, nbrCpt;
	byte subElemType;

	writeUint16BE(outFile, nbrElem);
	for (int i = 0; i < nbrElem; i++) {
		for (nbrSubElem = 0; actListArr[i][nbrSubElem] != NULL; nbrSubElem++)
			;
		writeUint16BE(outFile, nbrSubElem);
		for (int j = 0; j < nbrSubElem; j++) {
			subElemType = ((act *) actListArr[i][j])->a0.actType;
			writeByte(outFile, subElemType);
			switch (subElemType) {
			case ANULL: // -1
				break;
			case ASCHEDULE:          // 0
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a0.timer);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a0.actIndex);
				break;
			case START_OBJ:          // 1
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a1.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a1.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a1.cycleNumb);
				writeByte(outFile, ((act *) actListArr[i][j])->a1.cycle);
				break;
			case INIT_OBJXY:         // 2
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a2.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a2.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a2.x);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a2.y);
				break;
			case PROMPT:             // 3
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a3.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a3.promptIndex);
				for (nbrCpt = 0; ((act *) actListArr[i][j])->a3.responsePtr[nbrCpt] != -1; nbrCpt++)
					;
				nbrCpt++;
				writeUint16BE(outFile, nbrCpt);
				for (int k = 0; k < nbrCpt; k++)
					writeSint16BE(outFile, ((act *) actListArr[i][j])->a3.responsePtr[k]);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a3.actPassIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a3.actFailIndex);
				writeByte(outFile, (((act *) actListArr[i][j])->a3.encoded) ? 1 : 0);
				break;
			case BKGD_COLOR:         // 4
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a4.timer);
				writeUint32BE(outFile, ((act *) actListArr[i][j])->a4.newBkgColor);
				break;
			case INIT_OBJVXY:        // 5
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a5.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a5.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a5.vx);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a5.vy);
				break;
			case INIT_CARRY:         // 6
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a6.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a6.objNumb);
				writeByte(outFile, (((act *) actListArr[i][j])->a6.carriedFl) ? 1 : 0);
				break;
			case INIT_HF_COORD:      // 7
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a7.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a7.objNumb);
				break;
			case NEW_SCREEN:         // 8
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a8.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a8.screenIndex);
				break;
			case INIT_OBJSTATE:      // 9
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a9.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a9.objNumb);
				writeByte(outFile, ((act *) actListArr[i][j])->a9.newState);
				break;
			case INIT_PATH:          // 10
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a10.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a10.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a10.newPathType);
				writeByte(outFile, ((act *) actListArr[i][j])->a10.vxPath);
				writeByte(outFile, ((act *) actListArr[i][j])->a10.vyPath);
				break;
			case COND_R:             // 11
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a11.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a11.objNumb);
				writeByte(outFile, ((act *) actListArr[i][j])->a11.stateReq);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a11.actPassIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a11.actFailIndex);
				break;
			case TEXT:               // 12
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a12.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a12.stringIndex);
				break;
			case SWAP_IMAGES:        // 13
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a13.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a13.obj1);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a13.obj2);
				break;
			case COND_SCR:           // 14
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a14.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a14.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a14.screenReq);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a14.actPassIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a14.actFailIndex);
				break;
			case AUTOPILOT:          // 15
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a15.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a15.obj1);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a15.obj2);
				writeByte(outFile, ((act *) actListArr[i][j])->a15.vx);
				writeByte(outFile, ((act *) actListArr[i][j])->a15.vy);
				break;
			case INIT_OBJ_SEQ:       // 16
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a16.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a16.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a16.seqIndex);
				break;
			case SET_STATE_BITS:     // 17
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a17.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a17.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a17.stateMask);
				break;
			case CLEAR_STATE_BITS:   // 18
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a18.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a18.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a18.stateMask);
				break;
			case TEST_STATE_BITS:    // 19
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a19.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a19.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a19.stateMask);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a19.actPassIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a19.actFailIndex);
				break;
			case DEL_EVENTS:         // 20
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a20.timer);
				writeByte(outFile, ((act *) actListArr[i][j])->a20.actTypeDel);
				break;
			case GAMEOVER:           // 21
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a21.timer);
				break;
			case INIT_HH_COORD:      // 22
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a22.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a22.objNumb);
				break;
			case EXIT:               // 23
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a23.timer);
				break;
			case BONUS:              // 24
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a24.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a24.pointIndex);
				break;
			case COND_BOX:           // 25
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a25.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a25.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a25.x1);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a25.y1);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a25.x2);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a25.y2);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a25.actPassIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a25.actFailIndex);
				break;
			case SOUND:              // 26
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a26.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a26.soundIndex);
				break;
			case ADD_SCORE:          // 27
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a27.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a27.objNumb);
				break;
			case SUB_SCORE:          // 28
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a28.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a28.objNumb);
				break;
			case COND_CARRY:         // 29
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a29.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a29.objNumb);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a29.actPassIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a29.actFailIndex);
				break;
			case INIT_MAZE:          // 30
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a30.timer);
				writeByte(outFile, ((act *) actListArr[i][j])->a30.mazeSize);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a30.x1);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a30.y1);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a30.x2);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a30.y2);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a30.x3);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a30.x4);
				writeByte(outFile, ((act *) actListArr[i][j])->a30.firstScreenIndex);
				break;
			case EXIT_MAZE:          // 31
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a31.timer);
				break;
			case INIT_PRIORITY:      // 32
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a32.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a32.objNumb);
				writeByte(outFile, ((act *) actListArr[i][j])->a32.priority);
				break;
			case INIT_SCREEN:        // 33
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a33.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a33.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a33.screenIndex);
				break;
			case AGSCHEDULE:         // 34
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a34.timer);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a34.actIndex);
				break;
			case REMAPPAL:           // 35
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a35.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a35.oldColorIndex);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a35.newColorIndex);
				break;
			case COND_NOUN:          // 36
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a36.timer);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a36.nounIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a36.actPassIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a36.actFailIndex);
				break;
			case SCREEN_STATE:       // 37
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a37.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a37.screenIndex);
				writeByte(outFile, ((act *) actListArr[i][j])->a37.newState);
				break;
			case INIT_LIPS:          // 38
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a38.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a38.lipsObjNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a38.objNumb);
				writeByte(outFile, ((act *) actListArr[i][j])->a38.dxLips);
				writeByte(outFile, ((act *) actListArr[i][j])->a38.dyLips);
				break;
			case INIT_STORY_MODE:    // 39
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a39.timer);
				writeByte(outFile, (((act *) actListArr[i][j])->a39.storyModeFl) ? 1 : 0);
				break;
			case WARN:               // 40
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a40.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a40.stringIndex);
				break;
			case COND_BONUS:         // 41
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a41.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a41.BonusIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a41.actPassIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a41.actFailIndex);
				break;
			case TEXT_TAKE:          // 42
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a42.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a42.objNumb);
				break;
			case YESNO:              // 43
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a43.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a43.prompt);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a43.actYesIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a43.actNoIndex);
				break;
			case STOP_ROUTE:         // 44
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a44.timer);
				break;
			case COND_ROUTE:         // 45
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a45.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a45.routeIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a45.actPassIndex);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a45.actFailIndex);
				break;
			case INIT_JUMPEXIT:      // 46
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a46.timer);
				writeByte(outFile, (((act *) actListArr[i][j])->a46.jumpExitFl) ? 1 : 0);
				break;
			case INIT_VIEW:          // 47
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a47.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a47.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a47.viewx);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a47.viewy);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a47.direction);
				break;
			case INIT_OBJ_FRAME:     // 48
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a48.timer);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a48.objNumb);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a48.seqIndex);
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a48.frameIndex);
				break;
			case OLD_SONG:           // 49, Added by Strangerke for DOS versions
				writeSint16BE(outFile, ((act *) actListArr[i][j])->a49.timer);
				writeUint16BE(outFile, ((act *) actListArr[i][j])->a49.songIndex);
				break;
			default:
				printf("Unknown action %d", subElemType);
				exit(-1);
			}
		}
	}
}
