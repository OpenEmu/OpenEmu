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

#include "stdafx.h"
#include "cpthelp.h"
#include "TextFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32 crop(char *line);
uint16 findCptId(char *name, TextFile *cptFile);

#define MAX_CPTS 0xA000
#define MAX_OBJ_SIZE (0x2000 * 2)
#define NUM_DATA_LISTS 9
#define ASCII_SIZE (65536 * 2)

enum CptType {
	PTR_NULL = 0,
	COMPACT,
	TURNTAB,
	ANIMSEQ,
	MISCBIN,
	GETTOTAB,
	ROUTEBUF,
	MAINLIST
};

void processMainLists(FILE *inf, CptObj *destArr, uint16 *idList) {
	char line[1024];
	dofgets(line, 1024, inf);
	assert(lineMatchSection(line, "MAINLISTS"));
	uint16 *resBuf = (uint16 *)malloc(MAX_OBJ_SIZE);
	uint32 idNum = 0;
	do {
		dofgets(line, 1024, inf);
		if (!isEndOfSection(line)) {
			char cptName[50];
			uint16 id = getInfo(line, "MAINLST", cptName);
			CptObj *dest = destArr + id;
			assertEmpty(dest);
			dest->type = MAINLIST;
			dest->dbgName = (char *)malloc(strlen(cptName) + 1);
			strcpy(dest->dbgName, cptName);
			memset(resBuf, 0, MAX_OBJ_SIZE);
			uint32 resPos = 0;
			idList[idNum] = id;
			idNum++;

			do {
				dofgets(line, 1024, inf);
				if (!isEndOfObject(line, "MAINLST", id)) {
					assert((line[0] == '\t') && (line[1] == '\t'));
					char *stopCh;
					uint16 destId = (uint16)strtoul(line + 2, &stopCh, 16);
					assert(stopCh == (line + 6));
					assert((stopCh[0] == ':') && (stopCh[1] == ':'));
                    resBuf[resPos] = destId;
					resPos++;
				} else
					break;
			} while (1);
			assert(resPos < (MAX_OBJ_SIZE / 2));
			dest->len = resPos;
			dest->data = (uint16 *)malloc(resPos * 2);
			memcpy(dest->data, resBuf, resPos * 2);
		} else
			break;
	} while (1);

	free(resBuf);
}

void processCpts(FILE *inf, CptObj *destArr) {
	char line[1024];
	dofgets(line, 1024, inf);
	assert(lineMatchSection(line, "COMPACTS"));
	uint16 *resBuf = (uint16 *)malloc(MAX_OBJ_SIZE);
	do {
		dofgets(line, 1024, inf);
		if (!isEndOfSection(line)) {
			char cptName[50];
			uint16 id = getInfo(line, "COMPACT", cptName);
			CptObj *dest = destArr + id;
			assertEmpty(dest);
			dest->dbgName = (char *)malloc(strlen(cptName) + 1);
			dest->type = COMPACT;
			strcpy(dest->dbgName, cptName);
			memset(resBuf, 0, MAX_OBJ_SIZE);
			uint32 resPos = 0;

			do {
				dofgets(line, 1024, inf);
				if (!isEndOfObject(line, "COMPACT", id)) {
					assert((line[0] == '\t') && (line[1] == '\t'));
					char *stopCh;
					uint16 destId = (uint16)strtoul(line + 2, &stopCh, 16);
					assert(stopCh != (line + 2));
					assert((stopCh[0] == '-') && (stopCh[1] == '>'));
					if (resPos == 23) { // grafixProg
						assert(destId == 0);
						resBuf[resPos] = resBuf[resPos + 1] = 0;
						resPos += 2;
					} else if (resPos == 48) {	// turnProg. shouldn't it be 49?
						assert(destId == 0);
						resBuf[resPos] = resBuf[resPos + 1] = 0;
						resPos += 2;
					} else {
						resBuf[resPos] = destId;
						resPos++;
					}
				} else
					break;
			} while (1);
			assert(resPos < (MAX_OBJ_SIZE / 2));
			dest->len = resPos;
			dest->data = (uint16 *)malloc(resPos * 2);
			memcpy(dest->data, resBuf, resPos * 2);
		} else
			break;
	} while (1);

	free(resBuf);
}

void processTurntabs(FILE *inf, CptObj *destArr) {
	char line[1024];
	dofgets(line, 1024, inf);
	assert(lineMatchSection(line, "TURNTABS"));
	uint16 *resBuf = (uint16 *)malloc(MAX_OBJ_SIZE);
	do {
		dofgets(line, 1024, inf);
		if (!isEndOfSection(line)) {
			char cptName[50];
			uint16 id = getInfo(line, "TURNTAB", cptName);
			CptObj *dest = destArr + id;
			assertEmpty(dest);
			dest->dbgName = (char *)malloc(strlen(cptName) + 1);
			dest->type = TURNTAB;
			strcpy(dest->dbgName, cptName);
			memset(resBuf, 0, MAX_OBJ_SIZE);
			uint32 resPos = 0;

			do {
				dofgets(line, 1024, inf);
				if (!isEndOfObject(line, "TURNTAB", id)) {
					assert((line[0] == '\t') && (line[1] == '\t'));
					char *stopCh;
					uint16 destId = (uint16)strtoul(line + 2, &stopCh, 16);
					assert(stopCh == (line + 6));
					assert((stopCh[0] == '-') && (stopCh[1] == '>'));
                    resBuf[resPos] = destId;
					resPos++;
				} else
					break;
			} while (1);
			assert(resPos < (MAX_OBJ_SIZE / 2));
			dest->len = resPos;
			dest->data = (uint16 *)malloc(resPos * 2);
			memcpy(dest->data, resBuf, resPos * 2);
		} else
			break;
	} while (1);

	free(resBuf);
}

void processBins(FILE *inf, CptObj *destArr, const char *typeName, const char *objName, uint8 cTypeId) {
	char line[1024];
	dofgets(line, 1024, inf);
	assert(lineMatchSection(line, typeName));
	uint16 *resBuf = (uint16 *)malloc(MAX_OBJ_SIZE);
	do {
		dofgets(line, 1024, inf);
		if (!isEndOfSection(line)) {
			char cptName[50];
			uint16 id = getInfo(line, objName, cptName);
			CptObj *dest = destArr + id;
			assertEmpty(dest);
			dest->dbgName = (char *)malloc(strlen(cptName) + 1);
			dest->type = cTypeId;
			strcpy(dest->dbgName, cptName);
			memset(resBuf, 0, MAX_OBJ_SIZE);
			uint32 resPos = 0;

			do {
				dofgets(line, 1024, inf);
				if (!isEndOfObject(line, objName, id)) {
					assert((line[0] == '\t') && (line[1] == '\t'));
					char *stopCh;
					uint16 destId = (uint16)strtoul(line + 2, &stopCh, 16);
					assert(stopCh == (line + 6));
					assert(*stopCh == '\0');
                    resBuf[resPos] = destId;
					resPos++;
				} else
					break;
			} while (1);
			assert(resPos < (MAX_OBJ_SIZE / 2));
			dest->len = resPos;
			dest->data = (uint16 *)malloc(resPos * 2);
			memcpy(dest->data, resBuf, resPos * 2);
		} else
			break;
	} while (1);

	free(resBuf);
}

uint16 dlinkCount = 0;
static uint16 dlinks[1024];
static char* dlinkNames[512];

void processSymlinks(FILE *inf, CptObj *destArr, uint16 *baseLists) {
	char line[1024];
	dofgets(line, 1024, inf);
	assert(lineMatchSection(line, "SYMLINKS"));
	do {
		dofgets(line, 1024, inf);
		if (!isEndOfSection(line)) {
			char cptName[50];
			uint16 fromId = getInfo(line, "SYMLINK", cptName);
			CptObj *from = destArr + fromId;
			assertEmpty(from);
			dlinkNames[dlinkCount] = (char *)malloc(strlen(cptName) + 1);
			strcpy(dlinkNames[dlinkCount], cptName);

			dofgets(line, 1024, inf);
			assert((line[0] == '\t') && (line[1] == '\t') && (line[2] == '-') && (line[3] == '>'));
			char *stopCh;
			uint16 destId = (uint16)strtoul(line + 4, &stopCh, 16);
			assert(stopCh == (line + 8));
			assert((stopCh[0] == ':') && (stopCh[1] == ':'));

			dlinks[dlinkCount * 2 + 0] = fromId;
			dlinks[dlinkCount * 2 + 1] = destId;

			dlinkCount++;

			dofgets(line, 1024, inf);
			assert(isEndOfObject(line, "SYMLINK", fromId));
		} else
			break;
	} while (1);
}

void doCompile(FILE *inf, FILE *debOutf, FILE *resOutf, TextFile *cptDef, FILE *sve) {
	uint16 maxStrl = 0;
	uint16 maxCptl = 0;

	printf("Processing...\n");
	CptObj *resCpts;
	uint16 baseLists[NUM_DATA_LISTS];
	memset(baseLists, 0, NUM_DATA_LISTS * 2);
	resCpts = (CptObj *)malloc(MAX_CPTS * sizeof(CptObj));
	memset(resCpts, 0, MAX_CPTS * sizeof(CptObj));
	printf(" MainLists...\n");
	processMainLists(inf, resCpts, baseLists);
	printf(" Compacts...\n");
	processCpts(inf, resCpts);
	printf(" Turntables...\n");
	processTurntabs(inf, resCpts);
	printf(" Animation tables...\n");
	processBins(inf, resCpts, "ANIMSEQS", "ANIMSEQ", ANIMSEQ);
	printf(" Unknown binaries...\n");
	processBins(inf, resCpts, "MISCBINS", "MISCBIN", MISCBIN);
	printf(" Get To tables...\n");
	processBins(inf, resCpts, "GETTOTAB", "GET_TOS", GETTOTAB);
	printf(" Scratch buffers...\n");
	processBins(inf, resCpts, "SCRATCHR", "SCRATCH", ROUTEBUF);
	printf(" Symbolic links...\n");
	processSymlinks(inf, resCpts, baseLists);
	printf("Converting to binary data...\n");
	uint32 numCpts = 1;
	for (uint32 cnt = 1; cnt < MAX_CPTS; cnt++)
		if (resCpts[cnt].data || resCpts[cnt].dbgName || resCpts[cnt].len)
			numCpts++;

	uint16 dataListLen[NUM_DATA_LISTS];
	for (uint32 cnt = 0; cnt < NUM_DATA_LISTS; cnt++)
		for (uint16 elemCnt = 0; elemCnt < 0x1000; elemCnt++) {
			uint32 id = (cnt << 12) | elemCnt;
			if (resCpts[id].data || resCpts[id].dbgName || resCpts[id].len)
				dataListLen[cnt] = elemCnt + 1;
		}

	// write the header
	uint32 rev = 0;
	fwrite(&rev, 2, 1, debOutf);
	fwrite(&rev, 2, 1, resOutf);
	rev = NUM_DATA_LISTS;
	fwrite(&rev, 2, 1, debOutf);
	fwrite(&rev, 2, 1, resOutf);
	for (uint32 cnt = 0; cnt < NUM_DATA_LISTS; cnt++) {
		fwrite(dataListLen + cnt, 2, 1, debOutf);
		fwrite(dataListLen + cnt, 2, 1, resOutf);
	}

	uint32 binSize = 0;
	uint32 binDest = ftell(debOutf);
	fwrite(&binSize, 1, 4, debOutf);
	fwrite(&binSize, 1, 4, resOutf);
	fwrite(&binSize, 1, 4, debOutf);
	fwrite(&binSize, 1, 4, resOutf);

	char *asciiBuf = (char *)malloc(ASCII_SIZE);
	char *asciiPos = asciiBuf;

	// now process all the compacts
	uint32 cptSize[2];
	cptSize[0] = ftell(debOutf);
	cptSize[1] = ftell(resOutf);
	for (uint32 lcnt = 0; lcnt < NUM_DATA_LISTS; lcnt++) {
		for (uint32 eCnt = 0; eCnt < dataListLen[lcnt]; eCnt++) {
			uint32 cId = (lcnt << 12) | eCnt;
			CptObj *cpt = resCpts + cId;
			if (resCpts[cId].data || resCpts[cId].dbgName || resCpts[cId].len || resCpts[cId].type) {
				strcpy(asciiPos, cpt->dbgName);
				asciiPos += strlen(cpt->dbgName) + 1;

				assert(cpt->len < 0xFFFF);
				uint16 dlen = (uint16)cpt->len;
				if (dlen > maxCptl)
					maxCptl = dlen;
				binSize += dlen;
				assert(dlen != 0);

				fwrite(&dlen, 2, 1, debOutf);
				fwrite(&dlen, 2, 1, resOutf);

				uint16 field = resCpts[cId].type;
				fwrite(&field, 2, 1, debOutf);

				fwrite(cpt->data, 2, dlen, debOutf);
				fwrite(cpt->data, 2, dlen, resOutf);
			} else {
				uint16 tmp = 0;
				fwrite(&tmp, 2, 1, debOutf);
				fwrite(&tmp, 2, 1, resOutf);
			}
		}
		printf("DEBUG lcnt: %lu Output File Position: 0x%08lX\r\n", lcnt, ftell(debOutf));
	}
	cptSize[0] = ftell(debOutf) - cptSize[0];
	cptSize[1] = ftell(resOutf) - cptSize[1];
	assert(!(cptSize[0] & 1));
	assert(!(cptSize[1] & 1));
	cptSize[0] /= 2;
	cptSize[1] /= 2;

	for (uint32 cnt = 0; cnt < dlinkCount; cnt++) {
		strcpy(asciiPos, dlinkNames[cnt]);
		asciiPos += strlen(dlinkNames[cnt]) + 1;
	}

	uint32 asciiSize = (uint32)(asciiPos - asciiBuf);
	fwrite(&asciiSize, 1, 4, debOutf);
	fwrite(asciiBuf, 1, asciiSize, debOutf);

	// the direct links...
	fwrite(&dlinkCount, 2, 1, debOutf);
	fwrite(&dlinkCount, 2, 1, resOutf);
	for (uint32 cnt = 0; cnt < dlinkCount; cnt++) {
		fwrite(dlinks + cnt * 2 + 0, 2, 1, debOutf);
		fwrite(dlinks + cnt * 2 + 0, 2, 1, resOutf);

		fwrite(dlinks + cnt * 2 + 1, 2, 1, debOutf);
		fwrite(dlinks + cnt * 2 + 1, 2, 1, resOutf);
	}
	printf("Processing diff data...\n");
	printf("DEBUG Output File Position: 0x%08lX\r\n", ftell(debOutf));
	// 288 diffdata
	FILE *dif = fopen("288diff.txt", "r");
	assert(dif);
	char line[1024];
	uint16 diff[8192];
	uint16 diffDest = 0;
	uint16 diffNo = 0;
	while (fgets(line, 1024, dif)) {
		crop(line);
		if (line[0] != '$') {
			assert(memcmp(line, "data_", 5) == 0);
			char *pos = line + 5;
			char *stopCh;
			uint16 lId = (uint16)strtoul(pos, &stopCh, 10);
			assert(*stopCh == '[');
			uint16 eId = (uint16)strtoul(stopCh + 1, &stopCh, 10);
			assert((stopCh[0] == ']') && (stopCh[1] == '[') && (eId <= 0xFFF) && (lId <= 7));
			uint16 id = (lId << 12) | eId;
			uint16 elemNo = (uint16)strtoul(stopCh + 2, &stopCh, 10);
			assert(*stopCh == ']');
			stopCh = strstr(stopCh, "0x") + 2;
			uint16 val = (uint16)strtoul(stopCh, &stopCh, 16);
			assert(*stopCh == ';');
			diff[diffDest++] = id;
			diff[diffDest++] = elemNo;
			diff[diffDest++] = 1;
			diff[diffDest++] = val;
			diffNo++;
		} else {
			char *pos = strchr(line, ' ');
			*pos = '\0';
			uint16 id = findCptId(line + 1, cptDef);
			assert(id);
			diff[diffDest++] = id;
			diff[diffDest++] = 0;
            pos++;
			uint16 len = (uint16)strtoul(pos, &pos, 10);
			diff[diffDest++] = len;
			assert(len);
			assert(resCpts[id].len == len);
			for (uint16 cnt = 0; cnt < len; cnt++) {
				assert(*pos == ' ');
				pos++;
				diff[diffDest++] = (uint16)strtoul(pos, &pos, 16);
			}
			assert(diff[diffDest - 1] == 0xFFFF);
			diffNo++;
		}
	}
	assert(diffDest <= 8192);
	fwrite(&diffNo, 1, 2, debOutf);
	fwrite(&diffDest, 1, 2, debOutf);
	fwrite(diff, 2, diffDest, debOutf);
	fwrite(&diffNo, 1, 2, resOutf);
	fwrite(&diffDest, 1, 2, resOutf);
	fwrite(diff, 2, diffDest, resOutf);

	printf("Converting Save data...\n");
	printf("DEBUG Output File Position: 0x%08lX\r\n", ftell(debOutf));
	// the IDs of the compacts to be saved
	char cptName[1024];
	uint16 saveIds[2048];
	uint16 numIds = 0;
	while (fgets(cptName, 1024, sve)) {
		crop(cptName);
		uint16 resId = findCptId(cptName, cptDef);
		if (!resId)
			printf("ERROR: Can't find definition of %s\n", cptName);
		else {
			saveIds[numIds] = resId;
			numIds++;
		}
	}
	printf("%d saveIds\n", numIds);
	fwrite(&numIds, 2, 1, debOutf);
	fwrite(saveIds, 2, numIds, debOutf);
	fwrite(&numIds, 2, 1, resOutf);
	fwrite(saveIds, 2, numIds, resOutf);

	printf("Converting Reset data...\n");
	// now append the reset data
	uint16 gameVers[7] = { 303, 331, 348, 365, 368, 372, 288 };
	// make sure all files exist
	bool filesExist = true;
	char inName[32];
	for (int i = 0; i < 7; i++) {
		sprintf(inName, "RESET.%03d", gameVers[i]);
		FILE *test = fopen(inName, "rb");
		if (test)
			fclose(test);
		else {
			filesExist = false;
			printf("File %s not found\n", inName);
		}
	}

	if (filesExist) {
		FILE *res288 = fopen("RESET.288", "rb");
		fseek(res288, 0, SEEK_END);
		assert((ftell(res288) / 2) < 65536);
		uint16 resSize = (uint16)(ftell(res288) / 2);
		fseek(res288, 0, SEEK_SET);
		uint16 *buf288 = (uint16 *)malloc(resSize * 2);
		fread(buf288, 2, resSize, res288);
		fclose(res288);
		fwrite(&resSize, 1, 2, debOutf);
		fwrite(buf288, 2, resSize, debOutf);

		uint16 tmp = 7;
		fwrite(&tmp, 2, 1, debOutf);
		tmp = 288;
		fwrite(&tmp, 2, 1, debOutf);
		tmp = 0;
		fwrite(&tmp, 2, 1, debOutf);

		printf("DEBUG Output File Position: 0x%08lX\r\n", ftell(debOutf));
		printf("reset destination: %ld\n", ftell(debOutf));
		for (int cnt = 0; cnt < 6; cnt++) {
			printf("Processing diff v0.0%03d\n", gameVers[cnt]);
			uint16 diffPos = 0;
			sprintf(inName, "RESET.%03d", gameVers[cnt]);
			FILE *resDiff = fopen(inName, "rb");
			fseek(resDiff, 0, SEEK_END);
			assert(ftell(resDiff) == (resSize * 2));
			fseek(resDiff, 0, SEEK_SET);
			uint16 *bufDif = (uint16 *)malloc(resSize *2);
			fread(bufDif, 2, resSize, resDiff);
			fclose(resDiff);
			for (uint16 eCnt = 0; eCnt < resSize; eCnt++)
				if (buf288[eCnt] != bufDif[eCnt]) {
					diff[diffPos++] = eCnt;
					diff[diffPos++] = bufDif[eCnt];
				}
			free(bufDif);
			fwrite(gameVers + cnt, 1, 2, debOutf);
			assert(!(diffPos & 1));
			diffPos /= 2;
			fwrite(&diffPos, 1, 2, debOutf);
			fwrite(diff, 2, 2 * diffPos, debOutf);
			printf("diff v0.0%03d: 2 * 2 * %d\n", gameVers[cnt], diffPos);
			printf("DEBUG Output File Position: 0x%08lX\r\n", ftell(debOutf));
		}
		free(buf288);
	} else {
		printf("Creating CPT file with Dummy reset data @ %ld\n", ftell(debOutf));
		uint16 resetFields16 = 4;
		fwrite(&resetFields16, 2, 1, debOutf);
		uint32 blah = 8;
		fwrite(&blah, 4, 1, debOutf); // size field: 8 bytes
		blah = (uint32)-1;
		fwrite(&blah, 4, 1, debOutf); // save file revision. -1 is unknown to scummvm, so it'll refuse to load it.
		resetFields16 = 0;
		fwrite(&resetFields16, 2, 1, debOutf); // numDiffs: 0, no further reset blocks.
	}

	// now fill the raw-compact-data-size header field
	fseek(resOutf, binDest, SEEK_SET);
	fseek(debOutf, binDest, SEEK_SET);
	fwrite(&binSize, 1, 4, debOutf);
	fwrite(&binSize, 1, 4, resOutf);
	fwrite(cptSize + 0, 1, 4, debOutf);
	fwrite(cptSize + 1, 1, 4, resOutf);

	printf("%d diffs\n", diffNo);
	printf("%ld Compacts in total\n", numCpts);
	printf("max strlen = %d\n", maxStrl);
	printf("raw size = 2 * %ld\n", binSize);
	printf("max cptlen = %d\n", maxCptl);
}
