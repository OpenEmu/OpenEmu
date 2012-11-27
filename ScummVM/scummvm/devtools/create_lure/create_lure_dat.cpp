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
 * This is a utility for extracting needed resource data from different language
 * version of the Lure of the Temptress lure.exe executable files into a new file
 * lure.dat - this file is required for the ScummVM  Lure of the Temptress module
 * to work properly
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
#include "create_lure_dat.h"

using namespace Common;

File outputFile, lureExe;
Common::Language language;
uint16 dataSegment;

#define NUM_BYTES_VALIDATE 1024
#define ENGLISH_FILE_CHECKSUM 0xFD70
#define ITALIAN_FILE_CHECKSUM 0x109AD
#define FRENCH_FILE_CHECKSUM 0xD38C
#define GERMAN_FILE_CHECKSUM 0xD143
#define SPANISH_FILE_CHECKSUM 0xFFDA

Common::Language processedLanguages[100];
int langIndex = 0;
uint16 animOffsets[MAX_NUM_ANIM_RECORDS];
int animIndex = 0;
uint16 actionOffsets[MAX_NUM_ACTION_RECORDS];
int actionIndex = 0;

#define TALK_NUM_ENTRIES 28
#define MAX_TALK_LISTS 300

uint16 talkOffsets[MAX_TALK_LISTS];
int talkOffsetIndex = 0;

#define NUM_LANGUAGES 5
struct AnimListRecord {
	uint16 languages[NUM_LANGUAGES];
};

AnimListRecord animDataList[] = {
	{{0x1830, 0x1830, 0x1830, 0x1830, 0x1830}},	// Copy protection header
	{{0x1839, 0x1839, 0x1839, 0x1839, 0x1839}},	// Copy protection wording header
	{{0x1842, 0x1842, 0x1842, 0x1842, 0x1842}},	// Copy protection numbers
	{{0x184B, 0x184B, 0x184B, 0x184B, 0x184B}},	// Restart/Restore buttons
	{{0x55C0, 0x5680, 0x56A0, 0x56D0, 0x56a0}},	// Player midswing animation
	{{0x55C9, 0x5689, 0x56A9, 0x56D9, 0x56a9}},	// Player mid-level defend
	{{0x55D2, 0x5692, 0x56B2, 0x56E2, 0x56b2}},	// Player high-level strike
	{{0x55DB, 0x569B, 0x56BB, 0x56EB, 0x56bb}},	// Player high-level defend
	{{0x55E4, 0x56A4, 0x56C4, 0x56F4, 0x56c4}},	// Player low-level strike
	{{0x55ED, 0x56AD, 0x56CD, 0x56FD, 0x56cd}},	// Player low-level defend
	{{0x55F6, 0x56B6, 0x56D6, 0x5706, 0x56d6}},	// Player fight animation
	{{0x55FF, 0x56BF, 0x56DF, 0x570F, 0x56df}},	// Pig fight animation
	{{0x5608, 0x56C8, 0x56E8, 0x5718, 0x56e8}}, // Pig fight animation
	{{0x5611, 0x56D1, 0x56F1, 0x5721, 0x56f1}},	// Player mid-level strike
	{{0x561A, 0x56DA, 0x56FA, 0x572A, 0x56FA}}, // Pig upper block
	{{0x5623, 0x56E3, 0x5703, 0x5733, 0x5703}},	// Pig fight animation
	{{0x562C, 0x56EC, 0x570C, 0x573C, 0x570c}},	// Misc fight animation
	{{0x5635, 0x56F5, 0x5715, 0x5745, 0x5715}},	// Pig fight animation
	{{0x563E, 0x56FE, 0x571E, 0x574E, 0x571e}},	// Player recoiling from hit
	{{0x5647, 0x5707, 0x5727, 0x5757, 0x5727}},	// Pig recoiling from hit
	{{0x5650, 0x5710, 0x5730, 0x5760, 0x5730}},	// Pig dies
	{{0x5810, 0x58D0, 0x58F0, 0x5920, 0x58f0}},	// Voice bubble
	{{0x5915, 0x59D5, 0x59F5, 0x5a25, 0x59f5}},	// Blacksmith hammering
	{{0x59E4, 0x5AA4, 0x5AC4, 0x5af4, 0x5ac4}}, // Ewan's standard animation
	{{0x59ED, 0x5AAD, 0x5ACD, 0x5afd, 0x5acd}},	// Ewan's alternate animation
	{{0x59FF, 0x5ABF, 0x5ADF, 0x5b0f, 0x5adf}},	// Dragon breathing fire
	{{0x5A08, 0x5AC8, 0x5AE8, 0x5b18, 0x5af1}},	// Dragon breathing fire 2
	{{0x5A11, 0x5AD1, 0x5AF1, 0x5b21, 0x5af1}},	// Dragon breathing fire 3
	{{0x5A1A, 0x5ADA, 0x5AFA, 0x5b2a, 0x5afa}},	// Player turning winch in room #48
	{{0x5A59, 0x5B19, 0x5B39, 0x5b69, 0x5b39}},	// Player pulling lever in room #48
	{{0x5A62, 0x5B22, 0x5B42, 0x5b72, 0x5b42}},	// Minnow pulling lever in room #48
	{{0x5AAA, 0x5B6A, 0x5B8A, 0x5bba, 0x5b8a}},	// Goewin mixing potion
	{{0x5C80, 0x5D40, 0x5D60, 0x5d90, 0x5d60}}, // Player standard animation
	{{0x5C95, 0x5D55, 0x5D75, 0x5da5, 0x5d75}}, // Player operating rack
	{{0x5CAA, 0x5D6A, 0x5D8A, 0x5dba, 0x5d8a}},	// Selena animation
	{{0x5CE9, 0x5DA9, 0x5DC9, 0x5df9, 0x5dc9}},	// Blacksmith default
	{{0x5D28, 0x5DE8, 0x5E08, 0x5e38, 0x5e08}},	// Goewin animation
	{{0, 0, 0, 0}}
};

void errorExit(const char *msg) {
	printf("%s\n", msg);
	exit(1);
}

void add_anim_record(uint16 offset) {
	for (int ctr = 0; ctr < animIndex; ++ctr)
		if (animOffsets[ctr] == offset) return;
	if (animIndex == MAX_NUM_ANIM_RECORDS) {
		printf("Animation record offset table size exceeded\n");
		exit(1);
	}

	if (animIndex == 0) {
		// First call to the method, so add the fixed list for the current
		// language in at the start, so they'll be at fixed indexes
		int index = 0;
		if (language == IT_ITA) index = 1;
		else if (language == FR_FRA) index = 2;
		else if (language == DE_DEU) index = 3;
		else if (language == ES_ESP) index = 4;
		else if (language != EN_ANY) errorExit("add_anim_record: Unknown language");

		AnimListRecord *p = &animDataList[0];
		while (p->languages[index] != 0) {
			animOffsets[animIndex++] = p->languages[index];
			++p;
		}
	}

	animOffsets[animIndex++] = offset;
}

void add_action_list(uint16 offset) {
	for (int ctr = 0; ctr < actionIndex; ++ctr)
		if (actionOffsets[ctr] == offset) return;
	if (actionIndex == MAX_NUM_ACTION_RECORDS) {
		printf("Action record offset table size exceeded\n");
		exit(1);
	}
	actionOffsets[actionIndex++] = offset;
}

void read_basic_palette(byte *&data, uint16 &totalSize) {
	totalSize = PALETTE_SIZE;
	uint32 segmentStart = 0xC0A7;
	if (language == IT_ITA) segmentStart = 0xC107;
	else if (language == FR_FRA) segmentStart = 0xC4B7;
	else if (language == DE_DEU) segmentStart = 0xC517;
	else if (language == ES_ESP) segmentStart = 0xC177;
	else if (language != EN_ANY) errorExit("read_basic_palette: Unknown language");

	lureExe.seek(segmentStart);
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

#define ALT_PALETTE_1 0x1757
#define ALT_PALETTE_1_SIZE 180
#define ALT_PALETTE_2 0x180B
#define ALT_PALETTE_2_SIZE 24

void read_replacement_palette(byte *&data, uint16 &totalSize) {
	totalSize = ALT_PALETTE_1_SIZE + ALT_PALETTE_2_SIZE;
	data = (byte *) malloc(totalSize);

	lureExe.seek(dataSegment + ALT_PALETTE_1);
	lureExe.read(data, ALT_PALETTE_1_SIZE);
	lureExe.seek(dataSegment + ALT_PALETTE_2);
	lureExe.read(data + ALT_PALETTE_1_SIZE, ALT_PALETTE_2_SIZE);
}

void read_dialog_data(byte *&data, uint16 &totalSize) {
	uint32 segmentStart = 0x1dcb0;
	if (language == IT_ITA) segmentStart = 0x1ddd0;
	else if (language == FR_FRA) segmentStart = 0x1e1a0;
	else if (language == DE_DEU) segmentStart = 0x1e230;
	else if (language == ES_ESP) segmentStart = 0x1de60;
	else if (language != EN_ANY) errorExit("read_dialog_data: Unknown language");

	totalSize = DIALOG_SIZE;
	lureExe.seek(segmentStart);
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_talk_dialog_data(byte *&data, uint16 &totalSize) {
	uint32 segmentStart = 0x1de00;
	if (language == IT_ITA) segmentStart = 0x1df20;
	else if (language == FR_FRA) segmentStart = 0x1e2f0;
	else if (language == DE_DEU) segmentStart = 0x1e380;
	else if (language == ES_ESP) segmentStart = 0x1dfb0;
	else if (language != EN_ANY) errorExit("read_talk_dialog_data: Unknown language");

	totalSize = TALK_DIALOG_SIZE;
	lureExe.seek(segmentStart);
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_room_data(byte *&data, uint16 &totalSize)  {
	data = (byte *) malloc(MAX_DATA_SIZE);
	memset(data, 0, MAX_DATA_SIZE);

	uint16 *offsetPtr = (uint16 *) data;
	uint16 offset = (ROOM_NUM_ENTRIES + 1) * sizeof(uint16);
	uint16 pixelOffset;
	RoomResource buffer;
	RoomHeaderEntry headerEntry;
	RoomRectIn bounds;

	uint16 dataStart = 0xbf40;
	uint16 walkAreaOffset = 0x2eb1;
	switch (language) {
		case EN_ANY:
		break;
	case IT_ITA:
		dataStart = 0xc000;
		walkAreaOffset = 0x2ec0;
		break;
	case FR_FRA:
		dataStart = 0xc020;
		walkAreaOffset = 0x2ed0;
		break;
	case DE_DEU:
		dataStart = 0xc050;
		walkAreaOffset = 0x2edf;
		break;
	case ES_ESP:
		dataStart = 0xc020;
		walkAreaOffset = 0x2ecb;
		break;
	default:
		errorExit("read_room_data: Unknown language");
	}

	for (int index = 0; index < ROOM_NUM_ENTRIES; ++index) {

		lureExe.seek(dataSegment + dataStart + index * 9);
		lureExe.read(&headerEntry, sizeof(RoomHeaderEntry));

		if ((FROM_LE_16(headerEntry.offset) != 0) &&
			(FROM_LE_16(headerEntry.offset) != 0xffff) &&
			(FROM_LE_16(headerEntry.roomNumber) != 0)) {
			// Store offset of room entry
			*offsetPtr++ = TO_LE_16(offset);

			// Copy over basic room details
			lureExe.seek(dataSegment + FROM_LE_16(headerEntry.offset));
			lureExe.read(&buffer, sizeof(RoomResource));
			RoomResourceOutput *rec = (RoomResourceOutput *) (data + offset);
			rec->hdrFlags = headerEntry.hdrFlags;
			rec->actions = FROM_LE_32(buffer.actions);
			rec->roomNumber = index;
			rec->descId = headerEntry.descId;
			rec->numLayers = buffer.numLayers;
			memcpy(rec->layers, buffer.layers, 8);
			rec->sequenceOffset = buffer.sequenceOffset;
			rec->clippingXStart = TO_LE_16(FROM_LE_16(buffer.clippingXStart) - 0x80);
			rec->clippingXEnd = (FROM_LE_16(buffer.clippingXEnd) == 0) ? 0 :
			  TO_LE_16(FROM_LE_16(buffer.clippingXEnd) - 0x80);
			rec->exitTime = FROM_LE_32(buffer.exitTime);
			rec->areaFlag = buffer.areaFlag;
			rec->numExits = 0;

			offset += sizeof(RoomResourceOutput);

			// Copy over room exits
			for (int foo = 0; ; ++foo) {
				RoomResourceExit1 *p = (RoomResourceExit1 *) (data + offset);
				lureExe.read(p, sizeof(RoomResourceExit1));
				if (FROM_LE_16(p->xs) == 0xffff) break;

				if (++rec->numExits == 255)
					errorExit("Too many rooms read in");

				p->xs = TO_LE_16(FROM_LE_16(p->xs) - 0x80);
				p->ys = TO_LE_16(FROM_LE_16(p->ys) - 0x80);
				p->xe = TO_LE_16(FROM_LE_16(p->xe) - 0x80);
				p->ye = TO_LE_16(FROM_LE_16(p->ye) - 0x80);

				offset += sizeof(RoomResourceExit1);
				RoomResourceExit2 *p2 = (RoomResourceExit2 *) (data + offset);

				if (FROM_LE_16(p->sequenceOffset) == 0xffff) {
					lureExe.read(p2, sizeof(RoomResourceExit2));
					p2->newRoomX = TO_LE_16(FROM_LE_16(p2->newRoomX) - 0x80);
					p2->newRoomY = TO_LE_16(FROM_LE_16(p2->newRoomY) - 0x80);
				} else {
					p2->newRoom = 0;
					p2->direction = 0;
					p2->newRoomX = 0;
					p2->newRoomY = 0;
				}

				offset += sizeof(RoomResourceExit2);
			}

			// Handle the random destination walk bounds for the room

			lureExe.seek(dataSegment + walkAreaOffset +
				buffer.walkBoundsIndex * sizeof(RoomRectIn));
			lureExe.read(&bounds, sizeof(RoomRectIn));
			rec->walkBounds.xs = TO_LE_16(FROM_LE_16(bounds.xs) - 0x80);
			rec->walkBounds.xe = TO_LE_16(FROM_LE_16(bounds.xe) - 0x80);
			rec->walkBounds.ys = TO_LE_16(FROM_LE_16(bounds.ys) - 0x80);
			rec->walkBounds.ye = TO_LE_16(FROM_LE_16(bounds.ye) - 0x80);

			// If the room has a default pixel blocks list, add the references
			if (buffer.pixelListOffset != 0) {
				lureExe.seek(dataSegment + FROM_LE_16(buffer.pixelListOffset));
				pixelOffset = lureExe.readWord();
				while (pixelOffset != 0) {
					add_anim_record(pixelOffset);
					pixelOffset = lureExe.readWord();
				}
			}
		}
	}

	WRITE_LE_UINT16(offsetPtr, 0xffff);
	totalSize = offset;
}

uint16 englishTickProcOffsets[] = {
	0x41BD, 0x4f82, 0x5e44, 0x625e, 0x6571, 0x7207, 0x7c14, 0x7c24, 0x7efa, 0x7f02,
	0x7F37, 0x7f3a, 0x7f54, 0x7f69, 0x7fa1, 0x8009, 0x80c6, 0x813f, 0x8180, 0x81b3,
	0x81f3, 0x820e, 0x8241, 0x82a0, 0x85ce, 0x862d, 0x865A, 0x86FA, 0x86FF, 0x871E,
	0x873D, 0x8742, 0x8747, 0x87B3, 0x87EC, 0x882A, 0x8ABD, 0x982D, 0x98B6,
	0xffff
};

uint16 italianTickProcOffsets[] = {
	0x4205, 0x4fca, 0x5e8c, 0x62a6, 0x65b9, 0x724f, 0x7c5c, 0x7c6c, 0x7f58, 0x7f60,
	0x7f95, 0x7f98, 0x7fb2, 0x7fc7, 0x7fff, 0x8067, 0x8124, 0x819d, 0x81de, 0x8211,
	0x8251, 0x826c, 0x829f, 0x82fe, 0x862c, 0x868b, 0x86b8, 0x8758, 0x875D, 0x877C,
	0x879B, 0x87a0, 0x87a5, 0x8811, 0x884a, 0x8888,	0x8b20,	0x988f, 0x9918,
	0xffff
};

uint16 frenchTickProcOffsets[] = {
	0x457b, 0x5340, 0x6202, 0x661c, 0x692f, 0x75fb, 0x8008, 0x8018, 0x82f8, 0x8300,
	0x8335, 0x8338, 0x8352, 0x8367, 0x839f, 0x8407, 0x84c4, 0x853d, 0x857e, 0x85b1,
	0x85f1, 0x860c, 0x863f, 0x869e, 0x89cf, 0x8a2e, 0x8a5b, 0x8afb, 0x8b00, 0x8b1f,
	0x8b3e, 0x8b43, 0x8b48, 0x8bb4, 0x8bed, 0x8c26, 0x8ebe, 0x9c41, 0x9cca,
	0xffff
};

uint16 germanTickProcOffsets[] = {
	0x4543, 0x5308, 0x61ca, 0x65e4, 0x68fa, 0x7591, 0x7f9e, 0x7fae,	0x8358, 0x8360,
	0x8395, 0x8398, 0x83b2, 0x83c7, 0x83ff, 0x8467, 0x8524, 0x859d, 0x85de, 0x8611,
	0x8651, 0x866c, 0x869f, 0x86fe, 0x8a2c, 0x8a8b, 0x8ab8, 0x8b58, 0x8b5d, 0x8b7c,
	0x8b9b, 0x8ba0, 0x8ba5, 0x8c11, 0x8c4a, 0x8c83, 0x8f1a, 0x9c9b, 0x9d24,
	0xffff
};

uint16 spanishTickProcOffsets[] = {
	0x4247, 0x500c, 0x5ece,	0x62e8, 0x65fb, 0x7291, 0x7c9e, 0x7cae, 0x7fbe, 0x7fc6,
	0x7ffb, 0x7ffe, 0x8018, 0x802d, 0x8065, 0x80cd, 0x818a, 0x8203, 0x8244, 0x8277,
	0x82b7, 0x82d2, 0x8305, 0x8364, 0x8692, 0x86f1, 0x871e, 0x87be, 0x87c3, 0x87e2,
	0x8801, 0x8806, 0x880b, 0x8877, 0x88b0, 0x88ee, 0x8b86, 0x98f5, 0x997e,
	0xffff
};


uint16 englishOffsets[4] = {0x5d98, 0x5eb8, 0x623e, 0x63b1};
uint16 italianOffsets[4] = {0x5e58, 0x5f78, 0x62fe, 0x6471};
uint16 frenchOffsets[4]  = {0x5e78, 0x5f98, 0x631e, 0x6491};
uint16 germanOffsets[4]  = {0x5ea8, 0x5fc8, 0x634e, 0x64c1};
uint16 spanishOffsets[4] = {0x5e78, 0x5f98, 0x631e, 0x6491};

uint16 englishLoadOffsets[] = {0x3afe, 0x41BD, 0x7167, 0x7172, 0x8617, 0x88ac, 0};
uint16 italianLoadOffsets[] = {0x3b46, 0x4205, 0x71af, 0x71ba, 0x8675, 0x890a, 0};
uint16 frenchLoadOffsets[]  = {0x3ebc, 0x457B, 0x755b, 0x7566, 0x8a18, 0x8ca8, 0};
uint16 germanLoadOffsets[]  = {0x3e84, 0x4543, 0x74f1, 0x74fc, 0x8a75, 0x8d05, 0};
uint16 spanishLoadOffsets[] = {0x3b88, 0x4247, 0x71f1, 0x71fc, 0x86db, 0x8970, 0};

void read_hotspot_data(byte *&data, uint16 &totalSize)  {
	uint16 startId[4] = {0x3e8, 0x408, 0x2710, 0x7530};
	int walkNumEntries = 0;
	int walkCtr;
	int numEntries;
	HotspotWalkToRecord rec;
	HotspotWalkToRecord *walkList;
	HotspotHeaderEntry entryHeader;
	HotspotResource entry;
	uint16 dataSize;
	HotspotResourceOutput *r;
	CurrentActionInput action;

	// Set up list pointers for various languages
	uint16 *offsets = &englishOffsets[0];
	uint16 *procList = &englishTickProcOffsets[0];
	uint16 *loadOffsets = &englishLoadOffsets[0];
	uint16 walkToOffset = 0xBC4B;
	switch (language) {
	case EN_ANY:
		break;
	case IT_ITA:
		offsets = &italianOffsets[0];
		procList = &italianTickProcOffsets[0];
		walkToOffset = 0xBD0B;
		loadOffsets = &italianLoadOffsets[0];
		break;
	case FR_FRA:
		offsets = &frenchOffsets[0];
		procList = &frenchTickProcOffsets[0];
		walkToOffset = 0xBD2B;
		loadOffsets = &frenchLoadOffsets[0];
		break;
	case DE_DEU:
		offsets = &germanOffsets[0];
		procList = &germanTickProcOffsets[0];
		walkToOffset = 0xBD5B;
		loadOffsets = &germanLoadOffsets[0];
		break;
	case ES_ESP:
		offsets = &spanishOffsets[0];
		procList = &spanishTickProcOffsets[0];
		walkToOffset = 0xBD2B;
		loadOffsets = &spanishLoadOffsets[0];
		break;
	default:
		errorExit("read_hotspot_data: Unknown language");
		break;
	}

	// Allocate enough space for output hotspot list
	data = (byte *) malloc(MAX_HOTSPOTS * sizeof(HotspotResourceOutput));

	// Determine number of hotspot walk to entries

	lureExe.seek(dataSegment + walkToOffset);
	do {
		++walkNumEntries;
		lureExe.read(&rec, sizeof(HotspotWalkToRecord));
	} while (TO_LE_16(rec.hotspotId) != 0);
	--walkNumEntries;

	dataSize = walkNumEntries * sizeof(HotspotWalkToRecord);
	walkList = (HotspotWalkToRecord *) malloc(dataSize);
	lureExe.seek(dataSegment + walkToOffset);
	lureExe.read(walkList, sizeof(HotspotWalkToRecord) * walkNumEntries);

	// Main code for creating the hotspot list

	r = (HotspotResourceOutput *) data;
	numEntries = 0;

	for (int tableNum = 0; tableNum < 4; ++tableNum) {
		uint16 hotspotIndex = 0;
		for (;;) {
			uint16 currentHotspotId = startId[tableNum] + hotspotIndex;

			lureExe.seek(dataSegment + offsets[tableNum] +  hotspotIndex * 9);
			lureExe.read(&entryHeader, sizeof(HotspotHeaderEntry));
			if (FROM_LE_16(entryHeader.offset) == 0xffff) break;
			if (FROM_LE_16(entryHeader.offset) == 0) {
				++hotspotIndex;
				continue;
			}

			memset(r, 0, sizeof(HotspotResourceOutput));
			r->hotspotId = TO_LE_16(startId[tableNum] + hotspotIndex);
			r->nameId = entryHeader.resourceId;
			r->descId = entryHeader.descId;
			r->descId2 = entryHeader.descId2;
			r->hdrFlags = entryHeader.hdrFlags;

			// Get the hotspot data
			lureExe.seek(dataSegment + entryHeader.offset);
			lureExe.read(&entry, sizeof(HotspotResource));

			r->actions = entry.actions;
			r->roomNumber = entry.roomNumber;
			r->startX = TO_LE_16(FROM_LE_16(entry.startX) - 0x80);
			r->startY = TO_LE_16(FROM_LE_16(entry.startY) - 0x80);

			r->width = entry.width;
			r->height = entry.height;
			r->widthCopy = entry.widthCopy;
			r->heightCopy = entry.heightCopy;
			r->yCorrection = entry.yCorrection;
			r->talkX = entry.talkX;
			r->talkY = entry.talkY;
			r->characterMode = entry.characterMode;
			r->delayCtr = entry.delayCtr;
			r->tickSequenceOffset = entry.tickSequenceOffset;

			r->layer = entry.layer;
			r->colorOffset = entry.colorOffset;
			r->hotspotScriptOffset = entry.hotspotScriptOffset;
			r->talkScriptOffset = entry.talkScriptOffset;
			r->flags = entry.flags;

			// Handle any necessary translation of script load offsets

			r->scriptLoadFlag = entry.scriptLoadFlag;
			if (r->scriptLoadFlag || (tableNum == 3))
				// Load offset is in script segment, so leave as is
				r->loadOffset = entry.loadOffset;
			else {
				// Translate load offset to an index
				int loadIndex = 0;
				while ((loadOffsets[loadIndex] != FROM_LE_16(entry.loadOffset)) &&
						(loadOffsets[loadIndex] != 0))
					++loadIndex;

				if (loadOffsets[loadIndex] == 0) {
					printf("Unknown load offset encountered for hotspot %xh offset %xh\n",
						startId[tableNum] + hotspotIndex,
						FROM_LE_16(entry.loadOffset));
					exit(1);
				}

				r->loadOffset = TO_LE_16(loadIndex + 1);
			}

			if (tableNum == 3) {
				r->tickProcId = 0;
			} else {
				// Scan through the proc list for the correct offset
				int procIndex = 0;
				while ((procList[procIndex] != FROM_LE_16(entry.tickProcOffset)) &&
						(procList[procIndex] != 0xffff))
					++procIndex;

				if (procList[procIndex] == 0xffff) {
					if ((FROM_LE_16(entry.tickProcOffset) != 0xe00) &&
						(FROM_LE_16(entry.tickProcOffset) != 2))
//						printf("Could not find a tick proc handler for hotspot %xh offset %xh\n",
//							startId[tableNum] + hotspotIndex,
printf("%xh,\n",
							FROM_LE_16(entry.tickProcOffset));
					r->tickProcId = 0;
				}
				else
					r->tickProcId = TO_LE_16(procIndex + 1);
			}

			// WORKAROUND: Special check for the tinderbox hotspot to set it's room number correctly - the original
			// game used this as a backup against people trying to hack the copy protection
			if (currentHotspotId == 0x271C)
				r->roomNumber = TO_LE_16(28);

			// WORKAROUND: Sets a null handler for a hotspot that has an invalid tick proc offset
			if (currentHotspotId == 0x46b) r->tickProcId = 1;

			// Find the walk-to coordinates for the hotspot
			uint16 findId = FROM_LE_16(r->hotspotId);
			walkCtr = 0;
			while (walkCtr < walkNumEntries) {
				uint16 id = FROM_LE_16(walkList[walkCtr].hotspotId);
				if (id == findId)
					break;
				++walkCtr;
			}

			if (walkCtr == walkNumEntries) {
				r->walkX = 0;
				r->walkY = 0;
			} else {
				r->walkX = TO_LE_16(FROM_LE_16(walkList[walkCtr].x) - 0x80);
				uint16 y = FROM_LE_16(walkList[walkCtr].y);

				// WORKAROUND: Edwina's walk-to position is actually inside the table, which meant that walking over
				// to her could fail, depending on your start position. This increments it into the clear
				int tempY = (int16) (y & 0x7fff) - 0x80;
				if (currentHotspotId == 0x442)
					tempY += 8;
				r->walkY = TO_LE_16((y & 0x8000) | (uint16) tempY);
			}

			// Use the offset of the animation data as a dummy Id for the data
			r->animRecordId = entry.animOffset;
			r->tickTimeout = entry.tickTimeout;
			add_anim_record(FROM_LE_16(entry.animOffset));

			// Add in the actions offset table
			r->actionsOffset = entry.actionsOffset;
			if (FROM_LE_16(entry.actionsOffset) != 0)
				add_action_list(FROM_LE_16(entry.actionsOffset));

			if (FROM_LE_16(r->hotspotId) >= 0x408) {
				// Hotspot is not an NPC
				r->npcSchedule = 0;
			} else {
				// Check for an NPC schedule
				lureExe.seek(dataSegment + entryHeader.offset + 0x63);
				lureExe.read(&action, sizeof(CurrentActionInput));

				if (action.action != 2)
					r->npcSchedule = 0;
				else {
					r->npcSchedule = get_sequence_index(FROM_LE_16(action.dataOffset));
				}
			}

			++hotspotIndex;
			++r;
			++numEntries;

			if (numEntries == MAX_HOTSPOTS) {
				printf("Ran out of stack spaces for hotspot copying\n");
				exit(1);
			}
		}
	}

	r->hotspotId = TO_LE_16(0xffff);
	totalSize = numEntries * sizeof(HotspotResourceOutput) + 2;

	// Dispose of hotspot walk-to co-ordinate list
	free(walkList);
}

void read_hotspot_override_data(byte *&data, uint16 &totalSize)
{
	lureExe.seek(dataSegment + HOTSPOT_OVERRIDE_OFFSET);
	int numOverrides = 0;
	HotspotOverride rec;

	// Determine number of hotspot overrides
	do {
		++numOverrides;
		lureExe.read(&rec, sizeof(HotspotOverride));
	} while (FROM_LE_16(rec.hotspotId) != 0);
	--numOverrides;

	// Prepare output data and read in all entries at once
	totalSize = numOverrides * sizeof(HotspotOverride) + 2;
	data = (byte *) malloc(totalSize);
	lureExe.seek(dataSegment + HOTSPOT_OVERRIDE_OFFSET);
	lureExe.read(data, totalSize - 2);
	WRITE_LE_UINT16(data + totalSize - 2, 0xffff);

	// Post-process the coordinates
	HotspotOverride *p = (HotspotOverride *) data;
	for (int overrideCtr = 0; overrideCtr < numOverrides; ++overrideCtr, ++p) {
		p->xs = TO_LE_16(FROM_LE_16(p->xs) - 0x80);
		p->xe = TO_LE_16(FROM_LE_16(p->xe) - 0x80);
		p->ys = TO_LE_16(FROM_LE_16(p->ys) - 0x80);
		p->ye = TO_LE_16(FROM_LE_16(p->ye) - 0x80);
	}
}

void read_room_exits(byte *&data, uint16 &totalSize) {
	RoomExitHotspotRecord rec;
	uint16 offsets[NUM_ROOM_EXITS];
	uint16 numEntries[NUM_ROOM_EXITS];
	int roomCtr;
	totalSize = (NUM_ROOM_EXITS + 1) * sizeof(uint16);

	uint16 dataStart = 0x2F61;
	if (language == IT_ITA) dataStart = 0x2f70;
	else if (language == FR_FRA) dataStart = 0x2f80;
	else if (language == DE_DEU) dataStart = 0x2f8f;
	else if (language == ES_ESP) dataStart = 0x2f7b;
	else if (language != EN_ANY) errorExit("read_room_exits: Unknown language");

	lureExe.seek(dataSegment + dataStart);
	for (roomCtr = 0; roomCtr < NUM_ROOM_EXITS; ++roomCtr)
		offsets[roomCtr] = lureExe.readWord();

	// First loop to find total of room exit records there are
	for (roomCtr = 0; roomCtr < NUM_ROOM_EXITS; ++roomCtr) {
		numEntries[roomCtr] = 0;
		if (offsets[roomCtr] == 0) continue;

		// Get number of exits for the room
		lureExe.seek(dataSegment + offsets[roomCtr]);
		lureExe.read(&rec, sizeof(RoomExitHotspotRecord));
		while (FROM_LE_16(rec.xs) != 0) {
			totalSize += sizeof(RoomExitHotspotOutputRecord);
			numEntries[roomCtr]++;
			lureExe.read(&rec, sizeof(RoomExitHotspotRecord));
		}
		totalSize += sizeof(uint16); // save room for room list end flag
	}

	// Alloacte the total needed space
	data = (byte *) malloc(totalSize);
	uint16 *offset = (uint16 *) data;
	uint16 destIndex = (NUM_ROOM_EXITS + 1) * sizeof(uint16);
	uint16 entryCtr;

	// Loop to build up the result table

	for (roomCtr = 0; roomCtr < NUM_ROOM_EXITS; ++roomCtr) {
		if (offsets[roomCtr] == 0) {
			*offset++ = 0;		// No entries
		} else {
			// Read in the entries for the room
			*offset++ = TO_LE_16(destIndex);

			RoomExitHotspotOutputRecord *destP = (RoomExitHotspotOutputRecord *)
				(data + destIndex);

			lureExe.seek(dataSegment + offsets[roomCtr]);

			for (entryCtr = 0; entryCtr < numEntries[roomCtr]; ++entryCtr, ++destP) {
				lureExe.read(&rec, sizeof(RoomExitHotspotRecord));

				// Copy over the record
				destP->xs = TO_LE_16(FROM_LE_16(rec.xs) - 0x80);
				destP->xe = TO_LE_16(FROM_LE_16(rec.xe) - 0x80);
				destP->ys = TO_LE_16(FROM_LE_16(rec.ys) - 0x80);
				destP->ye = TO_LE_16(FROM_LE_16(rec.ye) - 0x80);
				destP->hotspotId = rec.hotspotId;
				destP->cursorNum = rec.cursorNum;
				destP->destRoomNumber = rec.destRoomNumber;
			}

			destIndex += numEntries[roomCtr] * sizeof(RoomExitHotspotOutputRecord);
			WRITE_LE_UINT16(data + destIndex, 0xffff);
			destIndex += sizeof(uint16);
		}
	}
	WRITE_LE_UINT16(offset, 0xffff);
}

void read_room_exit_joins(byte *&data, uint16 &totalSize) {
	RoomExitHotspotJoinRecord rec, *p;
	int numRecords = 0;
	uint32 unused;

	uint16 dataStart = 0xce30;
	if (language == IT_ITA) dataStart = 0xcef0;
	else if (language == FR_FRA) dataStart = 0xcf10;
	else if (language == DE_DEU) dataStart = 0xcf40;
	else if (language == ES_ESP) dataStart = 0xcf10;
	else if (language != EN_ANY) errorExit("read_room_exit_joins: Unknown language");

	lureExe.seek(dataSegment + dataStart);

	do {
		lureExe.read(&rec, sizeof(RoomExitHotspotJoinRecord));
		lureExe.read(&unused, sizeof(uint32));
		++numRecords;
	} while (FROM_LE_16(rec.hotspot1Id) != 0);
	--numRecords;

	// Allocate the data and read in all the records
	totalSize = (numRecords * sizeof(RoomExitHotspotJoinRecord)) + 2;
	data = (byte *) malloc(totalSize);
	lureExe.seek(dataSegment + dataStart);

	p = (RoomExitHotspotJoinRecord *) data;
	for (int recordCtr = 0; recordCtr < numRecords; ++recordCtr)
	{
		lureExe.read(p, sizeof(RoomExitHotspotJoinRecord));
		lureExe.read(&unused, sizeof(uint32));
		++p;
	}

	WRITE_LE_UINT16(p, 0xffff);
}

// This next method reads in the animation and movement data

void read_anim_data(byte *&data, uint16 &totalSize) {
	AnimRecord inRec;
	MovementRecord move;
	MovementRecord *destMove;
	uint16 offset, moveOffset;
	uint16 startOffset;
	int ctr, dirCtr;
	int movementSize = 0;
	bool *includeAnim = (bool *)malloc(animIndex);

	// Loop to figure out the total number of movement records there are
	for (ctr = 0; ctr < animIndex; ++ctr) {
		lureExe.seek(dataSegment + animOffsets[ctr]);
		lureExe.read(&inRec, sizeof(AnimRecord));

		if ((FROM_LE_16(inRec.leftOffset) < 0x5000) ||
			(FROM_LE_16(inRec.rightOffset) < 0x5000) ||
			(abs(FROM_LE_16(inRec.leftOffset)-FROM_LE_16(inRec.rightOffset)) > 0x800) ||
			(abs(FROM_LE_16(inRec.rightOffset)-FROM_LE_16(inRec.upOffset)) > 0x800) ||
			(abs(FROM_LE_16(inRec.upOffset)-FROM_LE_16(inRec.downOffset)) > 0x800)) {
			// Animation doesn't have valid movement data
			includeAnim[ctr] = false;
		} else {
			includeAnim[ctr] = true;
			for (dirCtr=0; dirCtr<4; ++dirCtr) {
				switch (dirCtr) {
				case 0:
					offset = FROM_LE_16(inRec.leftOffset);
					break;
				case 1:
					offset = FROM_LE_16(inRec.rightOffset);
					break;
				case 2:
					offset = FROM_LE_16(inRec.upOffset);
					break;
				default:
					offset = FROM_LE_16(inRec.downOffset);
				}

				if (offset != 0) {
					lureExe.seek(dataSegment + offset);
					lureExe.read(&move, sizeof(MovementRecord));

					while (FROM_LE_16(move.frameNumber) != 0xffff) {
						movementSize += sizeof(MovementRecord);
						lureExe.read(&move, sizeof(MovementRecord));
					}
					movementSize += 2;
				}
			}
		}
	}

	totalSize = animIndex * sizeof(AnimRecordOutput) + 2 + movementSize;
	AnimRecordOutput *rec = (AnimRecordOutput *) malloc(totalSize);
	data = (byte *) rec;
	moveOffset = animIndex * sizeof(AnimRecordOutput) + 2;

	// Loop to get in the animation records
	for (ctr = 0; ctr < animIndex; ++ctr, ++rec) {
		lureExe.seek(dataSegment + animOffsets[ctr]);
		lureExe.read(&inRec, sizeof(AnimRecord));

		rec->animRecordId = animOffsets[ctr];
		rec->animId = inRec.animId;
		rec->flags = TO_LE_16(inRec.flags);

		rec->leftOffset = 0;
		rec->rightOffset = 0;
		rec->upOffset = 0;
		rec->downOffset = 0;

		rec->upFrame = inRec.upFrame;
		rec->downFrame = inRec.downFrame;
		rec->leftFrame = inRec.leftFrame;
		rec->rightFrame = inRec.rightFrame;

		if (includeAnim[ctr]) {
			// Loop to get movement records
			uint16 *inDirs[4] = {&inRec.leftOffset, &inRec.rightOffset,
				&inRec.upOffset, &inRec.downOffset};
			uint16 *outDirs[4] = {&rec->leftOffset, &rec->rightOffset,
				&rec->upOffset, &rec->downOffset};

			for (dirCtr=0; dirCtr<4; ++dirCtr) {
				offset = READ_LE_UINT16(inDirs[dirCtr]);

				if (offset == 0) {
					startOffset = 0;
				} else {
					startOffset = moveOffset;

					lureExe.seek(dataSegment + offset);
					lureExe.read(&move, sizeof(MovementRecord));
					destMove = (MovementRecord *) (data + moveOffset);

					while (FROM_LE_16(move.frameNumber) != 0xffff) {
						destMove->frameNumber = move.frameNumber;
						destMove->xChange = move.xChange;
						destMove->yChange = move.yChange;

						moveOffset += sizeof(MovementRecord);
						++destMove;
						lureExe.read(&move, sizeof(MovementRecord));
					}

					destMove->frameNumber = TO_LE_16(0xffff);
					moveOffset += 2;
				}

				WRITE_LE_UINT16(outDirs[dirCtr], startOffset);
			}
		}
	}

	rec->animRecordId = TO_LE_16(0xffff);
	free(includeAnim);
}

void read_script_data(byte *&data, uint16 &totalSize) {
	uint32 segmentOffset = 0x1df00;
	if (language == IT_ITA) segmentOffset = 0x1e020;
	else if (language == FR_FRA) segmentOffset = 0x1e3f0;
	else if (language == DE_DEU) segmentOffset = 0x1e480;
	else if (language == ES_ESP) segmentOffset = 0x1e0b0;
	else if (language != EN_ANY) errorExit("read_script_data: Unknown language");
	lureExe.seek(segmentOffset);

	totalSize = SCRIPT_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_script2_data(byte *&data, uint16 &totalSize) {
	uint32 segmentOffset = 0x19c70;
	if (language == IT_ITA) segmentOffset = 0x19D90;
	else if (language == FR_FRA) segmentOffset = 0x1a160;
	else if (language == DE_DEU) segmentOffset = 0x1a1f0;
	else if (language == ES_ESP) segmentOffset = 0x19e20;
	else if (language != EN_ANY) errorExit("read_script2_data: Unknown language");
	lureExe.seek(segmentOffset);

	totalSize = SCRIPT2_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_hotspot_script_offsets(byte *&data, uint16 &totalSize) {
	uint16 dataStart = 0x57e0;
	if (language == IT_ITA) dataStart = 0x58a0;
	else if (language == FR_FRA) dataStart = 0x58c0;
	else if (language == DE_DEU) dataStart = 0x58f0;
	else if (language == ES_ESP) dataStart = 0x58c0;
	else if (language != EN_ANY) errorExit("read_hotspot_script_offsets: Unknown language");

	lureExe.seek(dataSegment + dataStart);
	totalSize = HOTSPOT_SCRIPT_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_messages_segment(byte *&data, uint16 &totalSize) {
	uint32 segmentOffset = 0x20b60;
	if (language == IT_ITA) segmentOffset = 0x20c80;
	else if (language == FR_FRA) segmentOffset = 0x21050;
	else if (language == DE_DEU) segmentOffset = 0x210E0;
	else if (language == ES_ESP) segmentOffset = 0x20d10;
	else if (language != EN_ANY) errorExit("read_messages_segment: Unknown language");

	lureExe.seek(segmentOffset);
	totalSize = MESSAGES_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

// Reads in the list of actions used

void read_actions_list(byte *&data, uint16 &totalSize) {
	// Allocate enough space for output action list
	data = (byte *) malloc(MAX_DATA_SIZE);
	HotspotActionsRecord *header = (HotspotActionsRecord *) data;
	uint16 offset = actionIndex * sizeof(HotspotActionsRecord) + sizeof(uint16);

	for (int ctr = 0; ctr < actionIndex; ++ctr) {
		header->recordId = actionOffsets[ctr];
		header->offset = offset;
		++header;

		lureExe.seek(dataSegment + actionOffsets[ctr]);
		uint16 *numItems = (uint16 *) (data + offset);
		lureExe.read(numItems, sizeof(uint16));
		offset += 2;

		if (READ_UINT16(numItems) > 0) {
			lureExe.read(data + offset, READ_UINT16(numItems) * 3);
			offset += READ_UINT16(numItems) * 3;
		}
	}
	header->recordId = TO_LE_16(0xffff);
}

// Reads in the talk data

void add_talk_offset(uint16 offset) {
	for (int ctr = 0; ctr < talkOffsetIndex; ++ctr)
		if (talkOffsets[ctr] == offset) return;
	if (talkOffsetIndex == MAX_TALK_LISTS) {
		printf("Exceeded maximum talk offset list size\n");
		exit(1);
	}

	talkOffsets[talkOffsetIndex++] = offset;
}

struct TalkEntry {
	uint16 hotspotId;
	uint16 offset;
};

void read_talk_headers(byte *&data, uint16 &totalSize) {
	TalkEntry entries[TALK_NUM_ENTRIES];
	uint16 sortedOffsets[TALK_NUM_ENTRIES+1];
	int entryCtr, subentryCtr;

	uint16 dataStart = 0x505c;
	if (language == IT_ITA) dataStart = 0x511C;
	else if (language == FR_FRA) dataStart = 0x513c;
	else if (language == DE_DEU) dataStart = 0x515c;
	else if (language == ES_ESP) dataStart = 0x512c;
	else if (language != EN_ANY) errorExit("read_talk_headers: Unknown language");

	lureExe.seek(dataSegment + dataStart);
	lureExe.read(&entries[0], sizeof(TalkEntry) * TALK_NUM_ENTRIES);

	// Sort the entry offsets into a list - this is used to figure out each entry's size
	int currVal, prevVal = 0;
	for (entryCtr = 0; entryCtr < TALK_NUM_ENTRIES; ++entryCtr) {
		currVal = 0xffff;
		for (subentryCtr = 0; subentryCtr < TALK_NUM_ENTRIES; ++subentryCtr) {
			if ((FROM_LE_16(entries[subentryCtr].offset) < currVal) &&
				(FROM_LE_16(entries[subentryCtr].offset) > prevVal))
				currVal = FROM_LE_16(entries[subentryCtr].offset);
		}
		if (currVal == 0xffff) break;

		sortedOffsets[entryCtr] = currVal;
		prevVal = currVal;
	}

	// Assume that the last talk header will have the same number of entries across language versions,
	// so create an end address based on the start of the last entry using start/end from English version
	sortedOffsets[entryCtr] = sortedOffsets[entryCtr - 1] + (0x5540 - 0x5504);

	data = (byte *) malloc(MAX_DATA_SIZE);
	TalkEntry *entry = (TalkEntry *) data;
	uint16 offset = TALK_NUM_ENTRIES * sizeof(TalkEntry) + sizeof(uint16);

	for (entryCtr = 0; entryCtr < TALK_NUM_ENTRIES; ++entryCtr) {
		entry->hotspotId = entries[entryCtr].hotspotId;
		entry->offset = TO_LE_16(offset);
		++entry;

		// Find the following offset in a sorted list
		int startOffset = FROM_LE_16(entries[entryCtr].offset);
		int nextOffset = 0;
		for (subentryCtr = 0; subentryCtr < TALK_NUM_ENTRIES; ++subentryCtr) {
			if (sortedOffsets[subentryCtr] == startOffset) {
				nextOffset = sortedOffsets[subentryCtr+1];
				break;
			}
		}
		if (nextOffset == 0)
			exit(1);

		// Read in line entries into the data
		lureExe.seek(dataSegment + startOffset);
		int size = nextOffset - startOffset;
		uint16 *talkOffset = (uint16 *) (data + offset);
		lureExe.read(talkOffset, size);

		while (size > 0) {
			if (READ_UINT16(talkOffset) != 0)
				add_talk_offset(READ_UINT16(talkOffset));
			size -= sizeof(uint16);
			offset += sizeof(uint16);
			talkOffset++;
		}

		WRITE_LE_UINT16(talkOffset, 0xffff);
		offset += 2;
	}

	add_talk_offset(0xffff);
	entry->hotspotId = TO_LE_16(0xffff);
	totalSize = offset + 2;
}

// Reads in the contents of the previously loaded talk lists

struct TalkRecord {
	uint16 recordId;
	uint16 listOffset;
	uint16 responsesOffset;
};

#define NUM_GIVE_TALK_IDS 6
uint16 englishGiveTalkIds[7] = {0xCF5E, 0xCF14, 0xCF90, 0xCFAA, 0xCFD0, 0xCFF6, 0xf010};
uint16 italianGiveTalkIds[7] = {0xD01E, 0xCFD4, 0xD050, 0xD06A, 0xD090, 0xD0B6, 0xf0d0};
uint16 frenchGiveTalkIds[7]  = {0xD03E, 0xCFF4, 0xD070, 0xD08A, 0xD0B0, 0xD0D6, 0xf0f0};
uint16 germanGiveTalkIds[7]  = {0xD06E, 0xD024, 0xD0A0, 0xD0BA, 0xD0E0, 0xD106, 0xf120};
uint16 spanishGiveTalkIds[7] = {0xD03E, 0xCFF4, 0xD070, 0xD08A, 0xD0B0, 0xD0D6, 0xf0f0};

void read_talk_data(byte *&data, uint16 &totalSize) {
	uint16 responseOffset;
	int talkCtr, subentryCtr;
	uint16 size;

	uint16 *giveTalkIds = &englishGiveTalkIds[0];
	if (language == IT_ITA) giveTalkIds = &italianGiveTalkIds[0];
	else if (language == FR_FRA) giveTalkIds = &frenchGiveTalkIds[0];
	else if (language == DE_DEU) giveTalkIds = &germanGiveTalkIds[0];
	else if (language == ES_ESP) giveTalkIds = &spanishGiveTalkIds[0];
	else if (language != EN_ANY) errorExit("read_talk_data: Unknown language");

	data = (byte *) malloc(MAX_DATA_SIZE);
	uint16 *v = (uint16 *) data;

	for (talkCtr = 0; talkCtr < NUM_GIVE_TALK_IDS; ++talkCtr) {
		add_talk_offset(giveTalkIds[talkCtr]);
		*v++ = TO_LE_16(giveTalkIds[talkCtr]);
	}

	byte *dataStart = (byte *) v;
	TalkRecord *header = (TalkRecord *) dataStart;
	uint16 offset = talkOffsetIndex * sizeof(TalkRecord) + sizeof(uint16);

	uint16 *sortedList = (uint16 *) malloc((talkOffsetIndex+1) * sizeof(uint16));
	memset(sortedList, 0, (talkOffsetIndex+1) * sizeof(uint16));

	// Sort the entry offsets into a list - this is used to figure out each entry's size
	int currVal, prevVal = 0;
	for (talkCtr = 0; talkCtr < talkOffsetIndex; ++talkCtr) {
		currVal = 0xffff;
		for (subentryCtr = 0; subentryCtr < talkOffsetIndex; ++subentryCtr) {
			if ((talkOffsets[subentryCtr] < currVal) &&
				(talkOffsets[subentryCtr] > prevVal))
				currVal = talkOffsets[subentryCtr];
		}
		if (currVal == 0xffff) break;

		sortedList[talkCtr] = currVal;
		prevVal = currVal;
	}
	sortedList[talkCtr] = giveTalkIds[6];
	int numTalks = talkCtr;

	// Loop through the talk list

	for (talkCtr = 0; talkCtr < numTalks; ++talkCtr) {
		uint16 startOffset = sortedList[talkCtr];
		uint16 nextOffset = sortedList[talkCtr+1];

		header->recordId = startOffset;
		header->listOffset = offset;

		lureExe.seek(dataSegment + startOffset);
		responseOffset = lureExe.readWord();
		startOffset += 2;

		// Special handling for entry at 0d930h
		if (responseOffset == 0x8000) continue;

		// Calculate talk data size - if response is within record range,
		// use simple calculation size. Otherwise, read in full data until
		// end of record
		if ((responseOffset < startOffset) || (responseOffset >= nextOffset))
			size = nextOffset - startOffset;
		else
			size = responseOffset - startOffset;
		if ((size % 6) == 2) size -= 2;
		if ((size % 6) != 0) {
			printf("Failure reading talk data: size=%d\n", size);
			exit(1);
		}

		// Read in the list of talk entries
		lureExe.read(dataStart + offset, size);
		offset += size;
		memset(dataStart + offset, 0xff, 2);
		offset += 2;

		// Handle the response data
		header->responsesOffset = offset;

		// Scan through the list of record offsets and find the offset of
		// the following record. This is done because although the talk
		// records and responses are normally sequential, it can also
		// point into another record's talk responses

		nextOffset = 0;
		for (subentryCtr = 0; subentryCtr < numTalks; ++subentryCtr) {
			if ((responseOffset >= sortedList[subentryCtr]) &&
				(responseOffset < sortedList[subentryCtr+1])) {
				// Found a record
				nextOffset = sortedList[subentryCtr+1];
				break;
			}
		}
		if (nextOffset < responseOffset) {
			printf("Failure reading talk data: no response found\n");
			exit(1);
		}

		size = nextOffset - responseOffset;
		if ((size % 6) != 0) size -= (size % 6);

		if ((size % 6) != 0) {
			printf("Failure reading talk data: newSize=%d\n", size);
			exit(1);
		}

		lureExe.read(dataStart + offset, size);
		offset += size;
		WRITE_LE_UINT16(dataStart + offset, 0xffff);
		offset += 2;

		++header;
	}

	header->recordId = TO_LE_16(0xffff);
	totalSize = offset + NUM_GIVE_TALK_IDS * sizeof(uint16);
	free(sortedList);
}

void read_room_pathfinding_data(byte *&data, uint16 &totalSize) {
	uint16 dataStart = 0x984A;
	if (language == IT_ITA) dataStart = 0x990A;
	else if (language == FR_FRA) dataStart = 0x992A;
	else if (language == DE_DEU) dataStart = 0x995A;
	else if (language == ES_ESP) dataStart = 0x992A;
	else if (language != EN_ANY) errorExit("read_room_pathfinding_data: Unknown language");
	lureExe.seek(dataSegment + dataStart);

	totalSize = PATHFIND_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_room_exit_coordinate_data(byte *&data, uint16 &totalSize)
{
	// Read in the exit coordinates list
	int roomNum, entryNum;
	uint16 x, y;
	RoomExitCoordinateEntryInputResource dataIn;

	totalSize = EXIT_COORDINATES_NUM_ROOMS * sizeof(RoomExitCoordinateEntryOutputResource) + 2;
	data = (byte *) malloc(totalSize);
	lureExe.seek(dataSegment + EXIT_COORDINATES_OFFSET);
	WRITE_LE_UINT16(data + totalSize - 2, 0xffff);

	// Post process the list to adjust data
	RoomExitCoordinateEntryOutputResource *rec = (RoomExitCoordinateEntryOutputResource *) data;
	for (roomNum = 1; roomNum <= EXIT_COORDINATES_NUM_ROOMS; ++roomNum, ++rec) {
		lureExe.read(&dataIn, sizeof(RoomExitCoordinateEntryInputResource));

		for (entryNum = 0; entryNum < ROOM_EXIT_COORDINATES_NUM_ENTRIES; ++entryNum) {
			x = FROM_LE_16(dataIn.entries[entryNum].x);
			y = FROM_LE_16(dataIn.entries[entryNum].y);
			if ((x != 0) || (y != 0)) {
				x -= 0x80;
				y = ((y & 0xfff) - 0x80) | (y & 0xf000);
			}

			RoomExitCoordinateResource *p = &rec->entries[entryNum];
			p->x = TO_LE_16(x);
			p->y = TO_LE_16(y);
			p->roomNumber = dataIn.entries[entryNum].roomNumber;
		}

		for (entryNum = 0; entryNum < ROOM_EXIT_COORDINATES_ENTRY_NUM_ROOMS; ++entryNum) {
			rec->roomIndex[entryNum] = TO_LE_16(FROM_LE_16(dataIn.roomIndex[entryNum]) / 6);
		}

		// WORKAROUND: Bugfix for the original game data to get to room #27 via rooms #10 or #11
		if ((roomNum == 10) || (roomNum == 11))
			rec->roomIndex[26] = TO_LE_16(1);
	}
}

void read_room_exit_hotspots_data(byte *&data, uint16 &totalSize) {
	totalSize = 0;
	data = (byte *) malloc(MAX_DATA_SIZE);

	RoomExitIndexedHotspotResource *rec = (RoomExitIndexedHotspotResource *) data;

	uint16 dataStart = 0x2E57;
	if (language == IT_ITA) dataStart = 0x2E66;
	else if (language == FR_FRA) dataStart = 0x2e76;
	else if (language == DE_DEU) dataStart = 0x2e85;
	else if (language == ES_ESP) dataStart = 0x2e71;
	else if (language != EN_ANY) errorExit("read_room_exit_hotspots_data: Unknown language");
	lureExe.seek(dataSegment + dataStart);

	lureExe.read(rec, sizeof(RoomExitIndexedHotspotResource));
	while (FROM_LE_16(rec->roomNumber) != 0) {
		++rec;
		totalSize += sizeof(RoomExitIndexedHotspotResource);
		lureExe.read(rec, sizeof(RoomExitIndexedHotspotResource));
	}

	WRITE_LE_UINT16(rec, 0xffff);
	totalSize += sizeof(uint16);
}

void save_fight_segment(byte *&data, uint16 &totalSize) {
	uint32 fightSegment = 0x1C400;
	if (language == IT_ITA) fightSegment = 0x1c520;
	else if (language == FR_FRA) fightSegment = 0x1c8f0;
	else if (language == DE_DEU) fightSegment = 0x1c980;
	else if (language == ES_ESP) fightSegment = 0x1c5b0;
	else if (language != EN_ANY) errorExit("save_fight_segment: Unknown language");
	lureExe.seek(fightSegment);

	totalSize = FIGHT_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

#define NUM_TEXT_ENTRIES 56
const char *englishTextStrings[NUM_TEXT_ENTRIES] = {
	"Get", NULL, "Push", "Pull", "Operate", "Open", "Close", "Lock", "Unlock", "Use",
	"Give", "Talk to", "Tell", "Buy", "Look", "Look at", "Look through", "Ask", NULL,
	"Drink", "Status", "Go to", "Return", "Bribe", "Examine",
	"Credits", "Restart game", "Save game", "Restore game", "Quit", "Fast Text\x8B",
	"Slow Text\x8B", "Sound on", "Sound off", "(nothing)", " for ", " to ", " on ",
	"and then", "finish", "Are you sure (y/n)?",
	"You are carrying ", "nothing", "You have ", "groat", "groats",
	NULL, "the ", "a ", "a ", "an ", "an ", "an ", "an ", NULL, NULL
};

const char *italianTextStrings[NUM_TEXT_ENTRIES] = {
	"Prendi", NULL,	"Spingi", "Tira", "Aziona", "Apri", "Chiudi", "Blocca",
	"Sblocca", "Usa", "Dai", "Parla con", "Ordina a", "Compra", "Guarda", "Osserva",
	"Guarda attraverso", "Chiedi", NULL,  "Bevi", "Stato", "Vai a", "Ritorna",
	"Corrompi", "Esamina",
	"Crediti", "Ricomincia", "Salva gioco", "Ripristina", "Abbandona", "Testo lento\x8B",
	"Testo veloce\x8B",  "Sonoro acceso", "Sonoro spento",
	"(niente)", " per ", " a ", " su ",
	"e poi", "finito", "Sei sicuro (s/n)?",
	"Stai portando ", "niente", "Hai ", "soldo", "soldi",
	NULL, "l' ", "la ", NULL, "le ", "i ", "il ", NULL, NULL, NULL
};

const char frenchUnlockStr[] = {'D', '\x7f', 'v', 'e', 'r', 'r', 'o', 'u', 'i', 'l', 'l', 'e', '\0'};
const char frenchCreditsStr[] = {'C', 'r', '\x7f', 'd', 'i', 't', 's', '\0'};

const char *frenchTextStrings[NUM_TEXT_ENTRIES] = {
	"Prends", NULL, "Pousse", "Tire", "Actionne", "Ouvre", "Ferme", "Verrouille",
	frenchUnlockStr, "Utilise", "Donne", "Parle \0x81", "Dis \x81", NULL,
	"Regarde", "Observe", "Regarde par", "Demande \x81", NULL, "Bois", "Statut",
	"Va \x81", "Reviens", "Corromps", "Examine",
	frenchCreditsStr, "Recommencer", "Sauvegarder", "Restituer", "Quitter",
	"Texte rapide\x8b", "Texte lent  \x8b", "Avec son", "Sans son",
	"(rien)", " avec ", " \x81 ", " sur ", "et puis", "fin",
	"Vous  \x83tes s\x89r (o/n)?",
	"Vous avez ", "rien", "et vous avez ", "sou", "sous",
	NULL, "les", "l'", "l'", "le", "le", "la", "la", NULL, NULL
};

const char *germanTextStrings[NUM_TEXT_ENTRIES] = {
	"Nimm", NULL, "Schiebe", "Ziehe", "Bet\x8dtige", "\x90" "ffne", "Schlie\x92h" "e",
	"Sperre", "Steck Schl\x8cssel in", "Benutze", "Gib", "Sprich mit", "Befehl an",
	NULL, "Betrachte", "Betrachte", "Schau durch", "Verlange", NULL, "Trink",
	"Status", "Geh zu", "Zur\x8c" "ck", "Besteche", "Untersuche",
	"Mitwirkende", "Spiel neu starten", "Spiel sichern", "Gesichertes Spiel laden",
	"Abbrechen", "Schneller Text\x8b",
	"Langsamer Text\x8b", "Sound an", "Sound aus", "(nichts)", " nach ", " an ", " f\x8cr ",
	"und dann", "Schlu\x92", "Sicher (j/n)?",
	"Du tr\x8dgst", "niets", "und du hast ", "silberm\x8cnzen", "silberm\x8cnzen"
	"keinen ", "keine ", "kein ", "der ", "die ", "das ", "den ", "einen ", "eine ",  "ein "
};

const char *spanishTextStrings[NUM_TEXT_ENTRIES] = {
	"Coge", NULL, "Empuja", "Tira de", "Opera", "Abre", "Cierra", "Atranca", "Desatranca", "Usa",
	"Dale", "Habla con", "Dile a", "Compra", "Obsevar", "Observe", "Mirar por", "P\x97" "dele", NULL,
	"Bebe", "Estado", "Ve a", "Regresa", "Sobornar", "Examinar",
	"Cr\x7f" "ditos", "Reiniciar", "Salvar juego", "Recuperar jue", "Abandonar", "Texto r\x98pido\x8b",
	"Texto lento \x8b", "Sonido activado   ", "Sonido desactivado ", "(nada)", " con ", " a ", " con ",
	"y luego", "eso es todo", "\x94" "Est\x98s seguro? (S/N)",
	"Llevas ", "nada", "y tienes ", "moneda", "monedas"
	"el ", "la ", "los ", "las ", "este ", "esta ", "estos ", "estas ", NULL, NULL
};

void save_text_strings(byte *&data, uint16 &totalSize) {
	int index;

	const char **textStrings = &englishTextStrings[0];
	if (language == IT_ITA) textStrings = &italianTextStrings[0];
	else if (language == FR_FRA) textStrings = &frenchTextStrings[0];
	else if (language == DE_DEU) textStrings = &germanTextStrings[0];
	else if (language == ES_ESP) textStrings = &spanishTextStrings[0];
	else if (language != EN_ANY) errorExit("save_text_strings: Unknown language");

	// Calculate the total needed space
	totalSize = sizeof(uint16);
	for (index = 0; index < NUM_TEXT_ENTRIES; ++index) {
		if (textStrings[index] != NULL)
			totalSize += strlen(textStrings[index]);
		++totalSize;
	}

	// Duplicate the text strings list into a data buffer
	data = (byte *) malloc(totalSize);
	*((uint16 *) data) = TO_LE_16(NUM_TEXT_ENTRIES);
	char *p = (char *) data + sizeof(uint16);

	for (index = 0; index < NUM_TEXT_ENTRIES; ++index) {
		if (textStrings[index] == NULL)
			*p++ = '\0';
		else {
			strcpy(p, textStrings[index]);
			p += strlen(p) + 1;
		}
	}
}

void save_sound_desc_data(byte *&data, uint16 &totalSize) {
	uint16 dataStart = 0x5671;
	if (language == IT_ITA) dataStart = 0x5731;
	else if (language == FR_FRA) dataStart = 0x5751;
	else if (language == DE_DEU) dataStart = 0x5781;
	else if (language == ES_ESP) dataStart = 0x5751;
	else if (language != EN_ANY) errorExit("save_sound_desc_data: Unknown language");
	lureExe.seek(dataSegment + dataStart);

	totalSize = SOUND_DESCS_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

struct DecoderEntry {
	const char *sequence;
	char character;
};

const DecoderEntry englishDecoders[] = {
	{"00", ' '}, {"0100", 'e'}, {"0101", 'o'}, {"0110", 't'}, {"01110", 'a'},
	{"01111", 'n'}, {"1000", 's'}, {"1001", 'i'}, {"1010", 'r'}, {"10110", 'h'},
	{"101110", 'u'}, {"1011110", 'l'}, {"1011111", 'd'}, {"11000", 'y'},
	{"110010", 'g'}, {"110011", '\0'}, {"110100", 'w'}, {"110101", 'c'},
	{"110110", 'f'}, {"1101110", '.'}, {"1101111", 'm'}, {"111000", 'p'},
	{"111001", 'b'}, {"1110100", ','}, {"1110101", 'k'}, {"1110110", '\''},
	{"11101110", 'I'}, {"11101111", 'v'}, {"1111000", '!'}, {"1111001", '\xb4'},
	{"11110100", 'T'}, {"11110101", '\xb5'}, {"11110110", '?'}, {"111101110", '\xb2'},
	{"111101111", '\xb3'}, {"11111000", 'W'}, {"111110010", 'H'}, {"111110011", 'A'},
	{"111110100", '\xb1'}, {"111110101", 'S'}, {"111110110", 'Y'}, {"1111101110", 'G'},
	{"11111011110", 'M'}, {"11111011111", 'N'}, {"111111000", 'O'}, {"1111110010", 'E'},
	{"1111110011", 'L'}, {"1111110100", '-'}, {"1111110101", 'R'}, {"1111110110", 'B'},
	{"11111101110", 'D'}, {"11111101111", '\xa6'}, {"1111111000", 'C'},
	{"11111110010", 'x'}, {"11111110011", 'j'}, {"1111111010", '\xac'},
	{"11111110110", '\xa3'}, {"111111101110", 'P'}, {"111111101111", 'U'},
	{"11111111000", 'q'}, {"11111111001", '\xad'}, {"111111110100", 'F'},
	{"111111110101", '1'}, {"111111110110", '\xaf'}, {"1111111101110", ';'},
	{"1111111101111", 'z'}, {"111111111000", '\xa5'}, {"1111111110010", '2'},
	{"1111111110011", '\xb0'}, {"111111111010", 'K'}, {"1111111110110", '%'},
	{"11111111101110", '\xa2'}, {"11111111101111", '5'}, {"1111111111000", ':'},
	{"1111111111001", 'J'}, {"1111111111010", 'V'}, {"11111111110110", '6'},
	{"11111111110111", '3'}, {"1111111111100", '\xab'}, {"11111111111010", '\xae'},
	{"111111111110110", '0'}, {"111111111110111", '4'}, {"11111111111100", '7'},
	{"111111111111010", '9'}, {"111111111111011", '"'}, {"111111111111100", '8'},
	{"111111111111101", '\xa7'}, {"1111111111111100", '/'}, {"1111111111111101", 'Q'},
	{"11111111111111100", '\xa8'}, {"11111111111111101", '('}, {"111111111111111100", ')'},
	{"111111111111111101", '\x99'}, {"11111111111111111", '\xa9'},
	{NULL, '\0'}
};

const DecoderEntry italianDecoders[] = {
	{"00", ' '}, {"010", (char) 0x69},	{"0110", (char) 0x6F}, {"01110", (char) 0x61}, {"01111", (char) 0x65},
	{"1000", (char) 0x72}, {"1001", (char) 0x6E}, {"1010", (char) 0x74}, {"10110", (char) 0x73}, {"101110", (char) 0x6C},
	{"101111", (char) 0x63}, {"11000", (char) 0x75}, {"110010", (char) 0x70}, {"110011", (char) 0x64}, {"110100", 0},
	{"110101", (char) 0x6D}, {"110110", (char) 0x67}, {"1101110", (char) 0x2E}, {"1101111", (char) 0x76},
	{"111000", (char) 0x68}, {"1110010", (char) 0x2C}, {"1110011", (char) 0x62}, {"1110100", (char) 0x66},
	{"1110101", (char) 0x21}, {"1110110", (char) 0xB5}, {"11101110", (char) 0xB1}, {"111011110", (char) 0xB3},
	{"111011111", (char) 0x7A}, {"1111000", (char) 0xB4}, {"11110010", (char) 0x27}, {"111100110", (char) 0x4E},
	{"111100111", (char) 0x4C}, {"11110100", (char) 0x3F}, {"111101010", (char) 0x85}, {"111101011", (char) 0x53},
	{"11110110", (char) 0x43}, {"111101110", (char) 0x4D}, {"1111011110", (char) 0xAC}, {"1111011111", (char) 0x49},
	{"11111000", (char) 0x45}, {"111110010", (char) 0x41}, {"1111100110", (char) 0x54}, {"1111100111", (char) 0xB2},
	{"111110100", (char) 0x71}, {"111110101", (char) 0x4F}, {"111110110", (char) 0x47}, {"1111101110", (char) 0xAB},
	{"11111011110", (char) 0x50}, {"11111011111", (char) 0x44}, {"111111000", (char) 0x81},
	{"1111110010", (char) 0x55}, {"11111100110", (char) 0xAE}, {"11111100111", (char) 0x52},
	{"1111110100", (char) 0xA6}, {"1111110101", (char) 0x56}, {"1111110110", (char) 0xA8},
	{"11111101110", (char) 0x42}, {"111111011110", (char) 0x51}, {"111111011111", (char) 0xB0},
	{"1111111000", (char) 0x95}, {"11111110010", (char) 0x48}, {"11111110011", (char) 0x2D},
	{"11111110100", (char) 0xA9}, {"11111110101", (char) 0x8A}, {"11111110110", (char) 0xA3},
	{"111111101110", (char) 0x46}, {"111111101111", (char) 0xA7}, {"11111111000", (char) 0x8D},
	{"11111111001", (char) 0x77}, {"11111111010", (char) 0x79}, {"111111110110", (char) 0x7F},
	{"1111111101110", (char) 0x6B}, {"1111111101111", (char) 0x31}, {"111111111000", (char) 0x3B},
	{"111111111001", (char) 0xA5}, {"111111111010", (char) 0x57}, {"1111111110110", (char) 0x32},
	{"11111111101110", (char) 0xAF}, {"11111111101111", (char) 0x35}, {"1111111111000", (char) 0xA2},
	{"1111111111001", (char) 0xAD}, {"1111111111010", (char) 0x25}, {"11111111110110", (char) 0x36},
	{"11111111110111", (char) 0x3A}, {"1111111111100", (char) 0x5A}, {"11111111111010", (char) 0x33},
	{"11111111111011", (char) 0x30}, {"11111111111100", (char) 0x34}, {"111111111111010", (char) 0x39},
	{"111111111111011", (char) 0x37}, {"111111111111100", (char) 0x38}, {"111111111111101", (char) 0x2F},
	{"1111111111111100", (char) 0x4B}, {"1111111111111101", (char) 0x22}, {"111111111111111000", (char) 0x09},
	{"111111111111111001", (char) 0x28}, {"11111111111111101", (char) 0x29}, {"111111111111111100", (char) 0x4A},
	{"111111111111111101", (char) 0x59}, {"11111111111111111", (char) 0x78},
	{NULL, '\0'}
};

const DecoderEntry frenchDecoders[] = {
	{"00", (char) 0x20}, {"0100", (char) 0x65}, {"0101", (char) 0x73}, {"0110", (char) 0x61}, {"01110", (char) 0x69},
	{"011110", (char) 0x6E}, {"011111", (char) 0x72}, {"1000", (char) 0x6F}, {"10010", (char) 0x74},
	{"100110", (char) 0x75}, {"100111", (char) 0x70}, {"10100", (char) 0x6C}, {"10101", (char) 0x6D},
	{"101100", (char) 0x63}, {"101101", (char) 0x76}, {"101110", (char) 0x00}, {"1011110", (char) 0x2E},
	{"1011111", (char) 0x7F}, {"110000", (char) 0xAA}, {"110001", (char) 0x27}, {"110010", (char) 0x64},
	{"1100110", (char) 0xB8}, {"1100111", (char) 0x2C}, {"110100", (char) 0x62}, {"1101010", (char) 0x68},
	{"11010110", (char) 0x66}, {"11010111", (char) 0xBE}, {"1101100", (char) 0x21}, {"1101101", (char) 0xB0},
	{"1101110", (char) 0x67}, {"11011110", (char) 0xF2}, {"11011111", (char) 0xC5}, {"1110000", (char) 0xC0},
	{"11100010", (char) 0x6A}, {"11100011", (char) 0xA5}, {"1110010", (char) 0xB1}, {"11100110", (char) 0x4C},
	{"111001110", (char) 0x2D}, {"111001111", (char) 0x4A}, {"1110100", (char) 0xBD}, {"11101010", (char) 0x3F},
	{"111010110", (char) 0x71}, {"111010111", (char) 0xB5}, {"11101100", (char) 0x81}, {"111011010", (char) 0x43},
	{"111011011", (char) 0x45}, {"11101110", (char) 0x85}, {"111011110", (char) 0xC4}, {"1110111110", (char) 0x4D},
	{"1110111111", (char) 0x79}, {"11110000", (char) 0xBC}, {"111100010", (char) 0xBA}, {"111100011", (char) 0xAF},
	{"11110010", (char) 0x83}, {"111100110", (char) 0x53}, {"1111001110", (char) 0x56}, {"1111001111", (char) 0xA7},
	{"11110100", (char) 0xA6}, {"111101010", (char) 0xB2}, {"1111010110", (char) 0xB7}, {"1111010111", (char) 0xD9},
	{"111101100", (char) 0x41}, {"111101101", (char) 0xA3}, {"111101110", (char) 0x4E}, {"1111011110", (char) 0xB4},
	{"11110111110", (char) 0x54}, {"11110111111", (char) 0x49}, {"111110000", (char) 0x4F}, {"111110001", (char) 0xBF},
	{"111110010", (char) 0x47}, {"1111100110", (char) 0x50}, {"11111001110", (char) 0x7A}, {"11111001111", (char) 0x52},
	{"111110100", (char) 0x44}, {"1111101010", (char) 0x55}, {"1111101011", (char) 0x51}, {"1111101100", (char) 0xE0},
	{"1111101101", (char) 0xAC}, {"1111101110", (char) 0x78}, {"11111011110", (char) 0xC8}, {"11111011111", (char) 0x42},
	{"1111110000", (char) 0xEE}, {"1111110001", (char) 0xC2}, {"1111110010", (char) 0x82}, {"11111100110", (char) 0x87},
	{"11111100111", (char) 0x77}, {"1111110100", (char) 0xC7}, {"11111101010", (char) 0xEF}, {"11111101011", (char) 0x80},
	{"1111110110", (char) 0xE4}, {"11111101110", (char) 0x89}, {"111111011110", (char) 0xE8}, {"111111011111", (char) 0xA9},
	{"11111110000", (char) 0x46}, {"11111110001", (char) 0x48}, {"11111110010", (char) 0xDC},
	{"111111100110", (char) 0xAE}, {"111111100111", (char) 0x88}, {"11111110100", (char) 0x6B},
	{"111111101010", (char) 0x7E}, {"111111101011", (char) 0x31}, {"11111110110", (char) 0xC3},
	{"111111101110", (char) 0xD8}, {"1111111011110", (char) 0x3B}, {"1111111011111", (char) 0xBB},
	{"11111111000", (char) 0xCD}, {"111111110010", (char) 0x8A}, {"1111111100110", (char) 0xB3},
	{"1111111100111", (char) 0x32}, {"111111110100", (char) 0xC1}, {"111111110101", (char) 0xD0},
	{"111111110110", (char) 0x57}, {"1111111101110", (char) 0xB6}, {"1111111101111", (char) 0x25},
	{"111111111000", (char) 0xA1}, {"1111111110010", (char) 0xDF}, {"1111111110011", (char) 0xEA},
	{"111111111010", (char) 0x35}, {"1111111110110", (char) 0x3A}, {"11111111101110", (char) 0x36},
	{"11111111101111", (char) 0x33}, {"1111111111000", (char) 0x39}, {"1111111111001", (char) 0xDB},
	{"1111111111010", (char) 0xE6}, {"11111111110110", (char) 0x30}, {"111111111101110", (char) 0x22},
	{"111111111101111", (char) 0x34}, {"11111111111000", (char) 0xE3}, {"11111111111001", (char) 0x37},
	{"11111111111010", (char) 0x38}, {"111111111110110", (char) 0xD7}, {"111111111110111", (char) 0xCC},
	{"11111111111100", (char) 0xD5}, {"111111111111010", (char) 0xE5}, {"1111111111110110", (char) 0x2F},
	{"1111111111110111", (char) 0x4B}, {"111111111111100", (char) 0xE9}, {"1111111111111010", (char) 0x59},
	{"1111111111111011", (char) 0x28}, {"1111111111111100", (char) 0x29}, {"1111111111111101", (char) 0x5A},
	{"11111111111111100", (char) 0x86}, {"11111111111111101", (char) 0x58}, {"11111111111111110", (char) 0x84},
	{"11111111111111111", (char) 0xA2},
	{NULL, '\0'}
};

const DecoderEntry germanDecoders[] = {
	{"000", (char) 0x20}, {"001", (char) 0x65}, {"0100", (char) 0x6E}, {"0101", (char) 0x61}, {"0110", (char) 0x72},
	{"01110", (char) 0x69}, {"011110", (char) 0x74}, {"011111", (char) 0x73}, {"10000", (char) 0x68},
	{"10001", (char) 0x6C}, {"10010", (char) 0x75}, {"100110", (char) 0x6D}, {"100111", (char) 0xB0},
	{"10100", (char) 0x67}, {"101010", (char) 0x00}, {"101011", (char) 0x62}, {"101100", (char) 0x63},
	{"101101", (char) 0x2C}, {"101110", (char) 0x6F}, {"1011110", (char) 0xAA}, {"1011111", (char) 0x77},
	{"110000", (char) 0x64}, {"110001", (char) 0x2E}, {"110010", (char) 0x6B}, {"1100110", (char) 0x66},
	{"1100111", (char) 0xDB}, {"110100", (char) 0x21}, {"1101010", (char) 0x7A}, {"11010110", (char) 0xC5},
	{"11010111", (char) 0x8C}, {"1101100", (char) 0x49}, {"11011010", (char) 0xCD}, {"11011011", (char) 0xCC},
	{"1101110", (char) 0x53}, {"11011110", (char) 0x45}, {"110111110", (char) 0x57}, {"110111111", (char) 0x8D},
	{"1110000", (char) 0xB8}, {"11100010", (char) 0x47}, {"11100011", (char) 0x76}, {"1110010", (char) 0x92},
	{"11100110", (char) 0xED}, {"111001110", (char) 0x44}, {"111001111", (char) 0x4D}, {"1110100", (char) 0x70},
	{"11101010", (char) 0x48}, {"111010110", (char) 0xD0}, {"111010111", (char) 0xDC}, {"11101100", (char) 0xE8},
	{"11101101", (char) 0x3F}, {"11101110", (char) 0x41}, {"111011110", (char) 0xDD}, {"1110111110", (char) 0x8F},
	{"1110111111", (char) 0xF2}, {"11110000", (char) 0x42}, {"11110001", (char) 0xEE}, {"11110010", (char) 0x46},
	{"111100110", (char) 0x4E}, {"1111001110", (char) 0x4C}, {"1111001111", (char) 0xE9}, {"11110100", (char) 0xEF},
	{"111101010", (char) 0x54}, {"111101011", (char) 0xEB}, {"111101100", (char) 0x4B}, {"111101101", (char) 0xD1},
	{"111101110", (char) 0xD9}, {"1111011110", (char) 0xD8}, {"1111011111", (char) 0x52}, {"111110000", (char) 0xEC},
	{"111110001", (char) 0xE4}, {"111110010", (char) 0xD7}, {"1111100110", (char) 0x55}, {"1111100111", (char) 0xEA},
	{"111110100", (char) 0xDF}, {"1111101010", (char) 0xE6}, {"1111101011", (char) 0x5A}, {"111110110", (char) 0x56},
	{"1111101110", (char) 0xD5}, {"11111011110", (char) 0xD6}, {"11111011111", (char) 0xE0}, {"111111000", (char) 0x6A},
	{"1111110010", (char) 0xA3}, {"1111110011", (char) 0xB1}, {"1111110100", (char) 0x2D}, {"1111110101", (char) 0xC8},
	{"1111110110", (char) 0x50}, {"11111101110", (char) 0xE7}, {"111111011110", (char) 0x27},
	{"111111011111", (char) 0xD4}, {"1111111000", (char) 0xCE}, {"11111110010", (char) 0x4F},
	{"11111110011", (char) 0x79}, {"1111111010", (char) 0xC9}, {"11111110110", (char) 0xAF},
	{"111111101110", (char) 0x4A}, {"111111101111", (char) 0xC2}, {"11111111000", (char) 0xC7},
	{"111111110010", (char) 0x31}, {"111111110011", (char) 0xDA}, {"111111110100", (char) 0xA1},
	{"111111110101", (char) 0x32}, {"111111110110", (char) 0xC4}, {"1111111101110", (char) 0xB5},
	{"11111111011110", (char) 0x91}, {"11111111011111", (char) 0xE5}, {"111111111000", (char) 0x25},
	{"1111111110010", (char) 0x35}, {"1111111110011", (char) 0x3A}, {"1111111110100", (char) 0x43},
	{"1111111110101", (char) 0xE3}, {"1111111110110", (char) 0x36}, {"11111111101110", (char) 0x78},
	{"11111111101111", (char) 0x90}, {"1111111111000", (char) 0xE2}, {"11111111110010", (char) 0x33},
	{"11111111110011", (char) 0x8E}, {"1111111111010", (char) 0xA7}, {"11111111110110", (char) 0x34},
	{"111111111101110", (char) 0x39}, {"111111111101111", (char) 0x30}, {"11111111111000", (char) 0x37},
	{"11111111111001", (char) 0x38}, {"11111111111010", (char) 0x71}, {"111111111110110", (char) 0xB7},
	{"111111111110111", (char) 0xBA}, {"11111111111100", (char) 0xB2}, {"111111111111010", (char) 0x3B},
	{"1111111111110110", (char) 0xAE}, {"1111111111110111", (char) 0x22}, {"111111111111100", (char) 0x2F},
	{"1111111111111010", (char) 0x51}, {"1111111111111011", (char) 0xA2}, {"1111111111111100", (char) 0xC3},
	{"11111111111111010", (char) 0xBC}, {"11111111111111011", (char) 0x28}, {"111111111111111000", (char) 0x29},
	{"111111111111111001", (char) 0x59}, {"11111111111111101", (char) 0xBB}, {"11111111111111110", (char) 0xBD},
	{"11111111111111111", (char) 0xC0},
	{NULL, '\0'}
};

const DecoderEntry spanishDecoders[] = {
	{"00", (char) 0x20}, {"010", (char) 0x61}, {"0110", (char) 0x6F}, {"01110", (char) 0x65},
	{"01111", (char) 0x72}, {"1000", (char) 0x73}, {"1001", (char) 0x6E}, {"10100", (char) 0x69},
	{"10101", (char) 0x75}, {"10110", (char) 0x63}, {"101110", (char) 0x74}, {"1011110", (char) 0x6C},
	{"1011111", (char) 0x64}, {"11000", (char) 0x70}, {"110010", (char) 0x2E}, {"110011", (char) 0x6D},
	{"110100", (char) 0x00}, {"110101", (char) 0xB3}, {"1101100", (char) 0x62}, {"1101101", (char) 0x2C},
	{"1101110", (char) 0x68}, {"11011110", (char) 0x67}, {"11011111", (char) 0xB1}, {"1110000", (char) 0x76},
	{"1110001", (char) 0xB5}, {"1110010", (char) 0x79}, {"11100110", (char) 0xAB}, {"11100111", (char) 0xB4},
	{"1110100", (char) 0x97}, {"11101010", (char) 0xB2}, {"11101011", (char) 0x98}, {"11101100", (char) 0x45},
	{"11101101", (char) 0x66}, {"11101110", (char) 0x95}, {"111011110", (char) 0x21}, {"111011111", (char) 0x6A},
	{"11110000", (char) 0x4C}, {"11110001", (char) 0x4E}, {"11110010", (char) 0x7F}, {"111100110", (char) 0x96},
	{"111100111", (char) 0x3F}, {"11110100", (char) 0xAC}, {"11110101", (char) 0x94}, {"11110110", (char) 0x7A},
	{"111101110", (char) 0x41}, {"1111011110", (char) 0x53}, {"1111011111", (char) 0x71}, {"11111000", (char) 0x54},
	{"111110010", (char) 0x93}, {"1111100110", (char) 0xAF}, {"1111100111", (char) 0x43}, {"111110100", (char) 0x50},
	{"111110101", (char) 0x4D}, {"111110110", (char) 0x44}, {"1111101110", (char) 0xAE}, {"11111011110", (char) 0xAD},
	{"11111011111", (char) 0xB0}, {"111111000", (char) 0x48}, {"1111110010", (char) 0x51}, {"1111110011", (char) 0x55},
	{"1111110100", (char) 0x4F}, {"1111110101", (char) 0x47}, {"1111110110", (char) 0x99},
	{"11111101110", (char) 0xA8}, {"11111101111", (char) 0x22}, {"1111111000", (char) 0xA9},
	{"11111110010", (char) 0xA6}, {"11111110011", (char) 0x52}, {"1111111010", (char) 0x59},
	{"11111110110", (char) 0xA3}, {"111111101110", (char) 0x78}, {"111111101111", (char) 0x56},
	{"11111111000", (char) 0x77}, {"11111111001", (char) 0x42}, {"111111110100", (char) 0x6B},
	{"111111110101", (char) 0x49}, {"111111110110", (char) 0x31}, {"1111111101110", (char) 0xAA},
	{"1111111101111", (char) 0x2D}, {"111111111000", (char) 0x46}, {"111111111001", (char) 0xA5},
	{"1111111110100", (char) 0x32}, {"1111111110101", (char) 0x3B}, {"1111111110110", (char) 0xA7},
	{"11111111101110", (char) 0x35}, {"11111111101111", (char) 0x25}, {"1111111111000", (char) 0x3A},
	{"1111111111001", (char) 0xA2}, {"1111111111010", (char) 0x57}, {"11111111110110", (char) 0x36},
	{"11111111110111", (char) 0x33}, {"1111111111100", (char) 0x4A}, {"11111111111010", (char) 0x30},
	{"11111111111011", (char) 0x34}, {"11111111111100", (char) 0x37}, {"111111111111010", (char) 0x39},
	{"111111111111011", (char) 0x38}, {"111111111111100", (char) 0xA4}, {"111111111111101", (char) 0x2F},
	{"1111111111111100", (char) 0x5A}, {"1111111111111101", (char) 0x8C}, {"11111111111111100", (char) 0x28},
	{"11111111111111101", (char) 0x29}, {"11111111111111110", (char) 0x4B}, {"111111111111111110", (char) 0x58},
	{"111111111111111111", (char) 0x91},
	{NULL, '\0'}
};

void save_string_decoder_data(byte *&data, uint16 &totalSize) {
	const DecoderEntry *list = &englishDecoders[0];
	if (language == IT_ITA) list = &italianDecoders[0];
	else if (language == FR_FRA) list = &frenchDecoders[0];
	else if (language == DE_DEU) list = &germanDecoders[0];
	else if (language == ES_ESP) list = &spanishDecoders[0];
	else if (language != EN_ANY) errorExit("save_string_decoder_data: Unknown language");

	totalSize = 1;
	const DecoderEntry *pSrc = list;
	while (pSrc->sequence != NULL) {
		totalSize += strlen(pSrc->sequence) + 2;
		++pSrc;
	}

	data = (byte *) malloc(totalSize);
	char *pDest = (char *)data;

	pSrc = list;
	while (pSrc->sequence != NULL) {
		*pDest++ = pSrc->character;
		strcpy(pDest, pSrc->sequence);
		pDest += strlen(pSrc->sequence) + 1;

		++pSrc;
	}

	*pDest = (char) 0xff;
}

void getEntry(uint8 entryIndex, uint16 &resourceId, byte *&data, uint16 &size) {
	resourceId = 0x3f01 + entryIndex;
	printf("Get resource #%d\n", entryIndex);
	switch (entryIndex) {
	case 0:
		// Copy the default palette to file
		read_basic_palette(data, size);
		break;

	case 1:
		// Copy the replacement palette fragments to file
		read_replacement_palette(data, size);
		break;

	case 2:
		// Copy the dialog segment data into the new vga file
		read_dialog_data(data, size);
		break;

	case 3:
		// Copy the talk dialog segment data into the new vga file
		read_talk_dialog_data(data, size);
		break;

	case 4:
		// Get the room info data
		read_room_data(data, size);
		break;

	case 5:
		// Get the action sequence set for NPC characters
		read_action_sequence(data, size);
		break;

	case 6:
		// Get the hotspot info data
		read_hotspot_data(data, size);
		break;

	case 7:
		// Get the hotspot override info data
		read_hotspot_override_data(data, size);
		break;

	case 8:
		// Get the list of room exits
		read_room_exits(data, size);
		break;

	case 9:
		// Get the list of room exit joins
		read_room_exit_joins(data, size);
		break;

	case 10:
		// Get the hotspot animation record data
		read_anim_data(data, size);
		break;

	case 11:
		// Get the script segment data
		read_script_data(data, size);
		break;

	case 12:
		// Get the second script segment data
		read_script2_data(data, size);
		break;

	case 13:
		// Get a list of hotspot script offsets
		read_hotspot_script_offsets(data, size);
		break;

	case 14:
		// Get the messages segment
		read_messages_segment(data, size);
		break;

	case 15:
		// Get the actions list
		read_actions_list(data, size);
		break;

	case 16:
		// Get the talk header information
		read_talk_headers(data, size);
		break;

	case 17:
		// Get the talk data
		read_talk_data(data, size);
		break;

	case 18:
		// Get the pathfinding data
		read_room_pathfinding_data(data, size);
		break;

	case 19:
		// Get the room exit coordinate list
		read_room_exit_coordinate_data(data, size);
		break;

	case 20:
		// Read the room exit hotspot list
		read_room_exit_hotspots_data(data, size);
		break;

	case 21:
		// Save the fight segment data
		save_fight_segment(data, size);
		break;

	case 22:
		// Set up the list of text strings used by the game
		save_text_strings(data, size);
		break;

	case 23:
		// Save the sound header desc data
		save_sound_desc_data(data, size);
		break;

	case 24:
		// Save the decoder sequence list
		save_string_decoder_data(data, size);
		break;

	default:
		data = NULL;
		size = 0;
		resourceId = 0xffff;
		break;
	}
}

void openOutputFile(const char *outFilename) {
	outputFile.open(outFilename, kFileWriteMode);

	// Write header
	outputFile.write("lure", 4);
	outputFile.writeWord(0);

	outputFile.seek(0xBF * 8);
	FileEntry fileVersion;
	memset(&fileVersion, 0xff, sizeof(FileEntry));
	fileVersion.unused = VERSION_MAJOR;
	fileVersion.sizeExtension = VERSION_MINOR;
	outputFile.write(&fileVersion, sizeof(FileEntry));
}

void closeOutputFile() {
	outputFile.seek(6 + 5 * langIndex);
	outputFile.writeByte(0xff);
	outputFile.close();
}

void createFile(const char *outFilename) {
	FileEntry rec;
	uint32 startOffset, numBytes;
	uint32 outputStart;
	uint16 resourceId;
	uint16 resourceSize;
	byte *resourceData;
	bool resourceFlag;
	byte tempBuffer[32];

	memset(tempBuffer, 0, 32);

	// Reset list counters
	outputStart = ((outputFile.pos() + 0xff) / 0x100) * 0x100;
	startOffset = 0x600;
	animIndex = 0;
	actionIndex = 0;
	talkOffsetIndex = 0;

	// Write out the position of the next language set
	outputFile.seek(6 + 5 * (langIndex - 1));

	switch (language) {
	case IT_ITA:
		outputFile.writeByte(LANG_IT_ITA);
		break;
	case FR_FRA:
		outputFile.writeByte(LANG_FR_FRA);
		break;
	case DE_DEU:
		outputFile.writeByte(LANG_DE_DEU);
		break;
	case ES_ESP:
		outputFile.writeByte(LANG_ES_ESP);
		break;
	case EN_ANY:
		outputFile.writeByte(LANG_EN_ANY);
		break;
	default:
		printf("Unknown language encountered\n");
		exit(1);
	}

	outputFile.writeLong(outputStart);

	// Write out start header
	outputFile.seek(outputStart);
	outputFile.write("heywow", 6);
	outputFile.writeWord(0);

	resourceFlag = true;
	for (int resIndex=0; resIndex < 0xBE; ++resIndex) {
		resourceData = NULL;

		// Get next data entry
		if (resourceFlag)
			// Get resource details
			getEntry(resIndex, resourceId, resourceData, resourceSize);

		// Write out the next header entry
		outputFile.seek(outputStart + (resIndex + 1) * 8);
		if (resourceSize == 0) {
			// Unused entry
			memset(&rec, 0xff, sizeof(FileEntry));
			resourceFlag = false;
		} else {
			rec.id = TO_LE_16(resourceId);
			rec.offset = TO_LE_16(startOffset >> 5);
			rec.sizeExtension = 0; //(uint8) ((resourceSize >> 16) & 0xff); --never needed
			rec.size = TO_LE_16(resourceSize & 0xffff);
			rec.unused = 0xff;
		}

		outputFile.write(&rec, sizeof(FileEntry));

		// Write out the resource
		if (resourceFlag) {
			outputFile.seek(outputStart + startOffset);
			outputFile.write(resourceData, resourceSize);
			startOffset += resourceSize;
			free(resourceData);		// Free the data block

			// Write out enough bytes to move to the next 32 byte boundary
			numBytes = 0x20 * ((startOffset + 0x1f) / 0x20) - startOffset;
			if (numBytes != 0)
			{
				outputFile.write(tempBuffer, numBytes);
				startOffset += numBytes;
			}
		}
	}

	// Move to the end of the written file
	outputFile.seek(0, SEEK_END);
}

// validate_executable
// Validates that the correct executable is being used to generate the
// resource file. Eventually the resource file creator will need to work
// with the other language executables, but for now just make

bool validate_executable() {
	uint32 sumTotal = 0;
	byte buffer[NUM_BYTES_VALIDATE];
	lureExe.read(buffer, NUM_BYTES_VALIDATE);
	for (int ctr = 0; ctr < NUM_BYTES_VALIDATE; ++ctr)
		sumTotal += buffer[ctr];

	if (sumTotal == ENGLISH_FILE_CHECKSUM) {
		language = EN_ANY;
		dataSegment = 0xAC50;
		printf("Detected English version\n");
	} else if (sumTotal == ITALIAN_FILE_CHECKSUM) {
		language = IT_ITA;
		dataSegment = 0xACB0;
		printf("Detected Italian version\n");
	} else if (sumTotal == FRENCH_FILE_CHECKSUM) {
		language = FR_FRA;
		dataSegment = 0xB060;
		printf("Detected French version\n");
	} else if (sumTotal == GERMAN_FILE_CHECKSUM) {
		language = DE_DEU;
		dataSegment = 0xB0C0;
		printf("Detected German version\n");
	} else if (sumTotal == SPANISH_FILE_CHECKSUM) {
		language = ES_ESP;
		dataSegment = 0xAD20;
		printf("Detected Spanish version\n");
	} else {
		printf("Lure executable version not recognized. Checksum = %xh\n", sumTotal);
		return false;
	}

	// Double-check that the given language has not already been done
	for (int index = 0; index < langIndex; ++index) {
		if (processedLanguages[index] == language) {
			printf("Identical language executable listed multiple times\n");
			return false;
		}
	}

	processedLanguages[langIndex++] = language;
	return true;
}


int main(int argc, char *argv[]) {
	const char /**inFilename,*/ *outFilename = 0;

	if (argc == 1) {
		printf("Format: %s output_filename [lureExecutable ..]\n", argv[0]);
		exit(0);
	}

	openOutputFile(argv[1]);

	for (int argi = 2; argi < argc; ++argi) {
		if (!lureExe.open(argv[argi]))
			printf("Could not open file: %s\n", argv[argi]);
		else {
			if (validate_executable())
				createFile(outFilename);
			lureExe.close();
		}
	}

	closeOutputFile();
}
