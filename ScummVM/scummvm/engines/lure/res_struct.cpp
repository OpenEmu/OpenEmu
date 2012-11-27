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

#include "lure/disk.h"
#include "lure/lure.h"
#include "lure/res.h"
#include "lure/res_struct.h"
#include "lure/scripts.h"
#include "common/endian.h"

namespace Lure {

const int actionNumParams[NPC_JUMP_ADDRESS+1] = {0,
	1, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 0, 1,
	0, 1, 1, 1, 1, 0, 0, 2, 1, 1, 0, 0, 1, 1, 2, 2, 5, 2, 2, 1};

// Barman related frame lists

static const uint16 basicPolish[] = {8+13,8+14,8+15,8+16,8+17,8+18,8+17,8+16,8+15,8+14,
	8+15,8+16,8+17,8+18,8+17,8+16,8+15,8+14,8+13,0};

static const uint16 sidsFetch[] = {12+1,12+2,12+3,12+4,12+5,12+6,12+5,12+6,12+5,12+4,12+3,12+7,12+8,0};

static const uint16 nelliesScratch[] = {11+1,11+2,11+3,11+4,11+5,11+4,11+5,11+4,11+5,11+4,11+3,11+2,11+1,0};

static const uint16 nelliesFetch[] = {1,2,3,4,5,4,5,4,3,2,6,7,0};

static const uint16 ewansFetch[] = {13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,0};

static const uint16 ewanExtraGraphic1[]= {
	28,29,30,31,32,33,34,35,36,37,
	38,39,40,41,42,43,44,45,46,47,
	48,
	40,39,38,37,36,35,34,33,32,31,30,29,28,
	0};

static const uint16 ewanExtraGraphic2[] = {
	1,2,3,4,5,6,7,8,9,
	10,11,12,13,14,15,16,17,18,19,
	20,21,22,23,24,0};

static const BarEntry default_barList[3] = {
	{29, SID_ID, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {&basicPolish[0], &sidsFetch[0], NULL, NULL}, 13, NULL},
	{32, NELLIE_ID, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {&nelliesScratch[0], &nelliesFetch[0], NULL, NULL}, 14, NULL},
	{35, EWAN_ID, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {&ewansFetch[0], &ewansFetch[0],
		&ewanExtraGraphic1[0], &ewanExtraGraphic2[0]}, 16, NULL}
};

const RoomTranslationRecord roomTranslations[] = {
	{0x1E, 0x13}, {0x07, 0x08}, {0x1C, 0x12}, {0x26, 0x0F},
	{0x27, 0x0F}, {0x28, 0x0F}, {0x29, 0x0F}, {0x22, 0x0A},
	{0x23, 0x13}, {0x24, 0x14}, {0x31, 0x2C}, {0x2F, 0x2C},
	{0, 0}};

// Room data holding class

RoomData::RoomData(RoomResource *rec, MemoryBlock *pathData) {
	roomNumber = READ_LE_UINT16(&rec->roomNumber);
	hdrFlags = rec->hdrFlags;

	actions = READ_LE_UINT32(&rec->actions) & 0xfffffff;
	flags = (READ_LE_UINT32(&rec->actions) >> 24) & 0xf0;
	descId = READ_LE_UINT16(&rec->descId);
	sequenceOffset = READ_LE_UINT16(&rec->sequenceOffset);
	numLayers = READ_LE_UINT16(&rec->numLayers);

	paths.load(pathData->data() + (roomNumber - 1) * ROOM_PATHS_SIZE);

	for (int ctr = 0; ctr < 4; ++ctr)
		layers[ctr] = READ_LE_UINT16(&rec->layers[ctr]);

	clippingXStart = READ_LE_UINT16(&rec->clippingXStart);
	clippingXEnd = READ_LE_UINT16(&rec->clippingXEnd);
	exitTime = READ_LE_UINT32(&rec->exitTime);
	areaFlag = rec->areaFlag;
	walkBounds.left = READ_LE_UINT16(&rec->walkBounds.xs);
	walkBounds.right = READ_LE_UINT16(&rec->walkBounds.xe);
	walkBounds.top = READ_LE_UINT16(&rec->walkBounds.ys);
	walkBounds.bottom = READ_LE_UINT16(&rec->walkBounds.ye);
}

// Room exit hotspot area holding class

RoomExitHotspotData::RoomExitHotspotData(RoomExitHotspotResource *rec) {
	hotspotId = READ_LE_UINT16(&rec->hotspotId);
	xs = READ_LE_UINT16(&rec->xs);
	ys = READ_LE_UINT16(&rec->ys);
	xe = READ_LE_UINT16(&rec->xe);
	ye = READ_LE_UINT16(&rec->ye);
	cursorNum = rec->cursorNum;
	destRoomNumber = READ_LE_UINT16(&rec->destRoomNumber);
}

//  Room exit class

RoomExitData::RoomExitData(RoomExitResource *rec) {
	xs = READ_LE_INT16(&rec->xs);
	ys = READ_LE_INT16(&rec->ys);
	xe = READ_LE_INT16(&rec->xe);
	ye = READ_LE_INT16(&rec->ye);
	sequenceOffset = READ_LE_UINT16(&rec->sequenceOffset);
	roomNumber = rec->newRoom;
	x = READ_LE_INT16(&rec->newRoomX);
	y = READ_LE_INT16(&rec->newRoomY);

	switch (rec->direction) {
	case 0x80:
		direction = UP;
		break;
	case 0x40:
		direction = DOWN;
		break;
	case 0x20:
		direction = LEFT;
		break;
	case 0x10:
		direction = RIGHT;
		break;
	default:
		direction = NO_DIRECTION;
		break;
	}
}

bool RoomExitData::insideRect(int16 xp, int16 yp) {
	return ((xp >= xs) && (xp < xe) && (yp >= ys) && (yp < ye));
}

RoomExitData *RoomExitList::checkExits(int16 xp, int16 yp) {
	iterator i;
	for (i = begin(); i != end(); ++i) {
		RoomExitData *rec = (*i).get();
		if (rec->insideRect(xp, yp)) {
			return rec;
		}
	}
	return NULL;
}

// Room paths

bool RoomPathsData::isOccupied(int x, int y) {
	if ((x < 0) || (y < 0) || (x >= ROOM_PATHS_WIDTH) || (y >= ROOM_PATHS_HEIGHT))
		// Off screen, so flag as not occupied
		return false;
	return (_data[y * 5 + (x >> 3)] & (0x80 >> (x % 8))) != 0;
}

bool RoomPathsData::isOccupied(int x, int y, int width) {
	for (int blockCtr = 0; blockCtr < width; ++blockCtr) {
		if (isOccupied(x + 8 * blockCtr, y))
			return true;
	}

	return false;
}

void RoomPathsData::setOccupied(int x, int y, int width) {
	if ((x < 0) || (y < 0) || (x >= ROOM_PATHS_WIDTH) || (y >= ROOM_PATHS_HEIGHT))
		return;

	byte *p = &_data[y * 5 + (x / 8)];
	byte bitMask = 0x80 >> (x % 8);

	for (int bitCtr = 0; bitCtr < width; ++bitCtr) {
		*p |= bitMask;
		bitMask >>= 1;
		if (bitMask == 0) {
			++p;
			bitMask = 0x80;
		}
	}
}

void RoomPathsData::clearOccupied(int x, int y, int width) {
	if ((x < 0) || (y < 0) || (x >= ROOM_PATHS_WIDTH) || (y >= ROOM_PATHS_HEIGHT))
		return;

	byte *p = &_data[y * 5 + (x / 8)];
	byte bitMask = 0x80 >> (x % 8);

	for (int bitCtr = 0; bitCtr < width; ++bitCtr) {
		*p &= ~bitMask;
		bitMask >>= 1;
		if (bitMask == 0) {
			++p;
			bitMask = 0x80;
		}
	}
}

// decompresses the bit-packed data for which parts of a room are occupied
// into a byte array. It also adds a column and row of padding around the
// edges of the screen, and extends occupied areas to adjust for the width
// of the chracter

void RoomPathsData::decompress(RoomPathsDecompressedData &dataOut, int characterWidth) {
	byte *pIn = &_data[ROOM_PATHS_SIZE - 1];
	uint16 *pOut = &dataOut[DECODED_PATHS_WIDTH * DECODED_PATHS_HEIGHT - 1];
	byte v;
	int paddingCtr;
	int charWidth = characterWidth >> 3;
	int charCtr = 0;
	bool charState = false;

	// Handle padding for last row, including left/right edge padding, as
	// well as the right column of the second row
	for (paddingCtr = 0; paddingCtr < (DECODED_PATHS_WIDTH + 1); ++paddingCtr)
		*pOut-- = 0;

	for (int y = 0; y < ROOM_PATHS_HEIGHT; ++y) {
		charState = false;

		for (int x = 0; x < (ROOM_PATHS_WIDTH / 8); ++x) {
			// Get next byte, which containing bits for 8 blocks
			v = *pIn--;

			for (int bitCtr = 0; bitCtr < 8; ++bitCtr) {
				bool isSet = (v & 1) != 0;
				v >>= 1;

				if (charState) {
					// Handling occupied characters adjusted for character width
					if (isSet)
						// Reset character counter
						charCtr = charWidth;

					*pOut-- = 0xffff;
					charState = (--charCtr != 0);

				} else {
					// Normal decompression
					if (!isSet) {
						// Flag block is available for walking on
						*pOut-- = 0;
					} else {
						// Flag block is occupied
						*pOut-- = 0xffff;

						// Handling for adjusting for character widths
						charCtr = charWidth - 1;
						charState = charCtr >= 0;
					}
				}
			}
		}

		// Store 2 words to allow off-screen row-start/prior row end
		*pOut-- = 0;
		*pOut-- = 0;
		charState = false;
	}

	// Handle padding for final top row - no need for end column, as end of prior
	// row provided padding for it
	for (paddingCtr = 0; paddingCtr < (ROOM_PATHS_WIDTH + 1); ++paddingCtr)
		*pOut-- = 0;
}

// Room data class

void RoomDataList::saveToStream(Common::WriteStream *stream) const {
	for (RoomDataList::const_iterator i = begin(); i != end(); ++i) {
		RoomData const &rec = **i;
		stream->writeByte(rec.flags);
		const byte *pathData = rec.paths.data();
		stream->write(pathData, ROOM_PATHS_HEIGHT * ROOM_PATHS_WIDTH);
	}
}

void RoomDataList::loadFromStream(Common::ReadStream *stream) {
	RoomDataList::iterator i;
	byte data[ROOM_PATHS_HEIGHT * ROOM_PATHS_WIDTH];

	for (i = begin(); i != end(); ++i) {
		RoomData &rec = **i;
		rec.flags = stream->readByte();
		stream->read(data, ROOM_PATHS_HEIGHT * ROOM_PATHS_WIDTH);
		rec.paths.load(data);
	}
}

// Room exit joins class

RoomExitJoinData::RoomExitJoinData(RoomExitJoinResource *rec) {
	hotspots[0].hotspotId = READ_LE_UINT16(&rec->hotspot1Id);
	hotspots[0].currentFrame = rec->h1CurrentFrame;
	hotspots[0].destFrame = rec->h1DestFrame;
	hotspots[0].openSound = rec->h1OpenSound;
	hotspots[0].closeSound = rec->h1CloseSound;
	hotspots[1].hotspotId = READ_LE_UINT16(&rec->hotspot2Id);
	hotspots[1].currentFrame = rec->h2CurrentFrame;
	hotspots[1].destFrame = rec->h2DestFrame;
	hotspots[1].openSound = rec->h2OpenSound;
	hotspots[1].closeSound = rec->h2CloseSound;
	blocked = rec->blocked;
}

void RoomExitJoinList::saveToStream(Common::WriteStream *stream) const {
	for (RoomExitJoinList::const_iterator i = begin(); i != end(); ++i) {
		RoomExitJoinData const &rec = **i;

		stream->writeUint16LE(rec.hotspots[0].hotspotId);
		stream->writeUint16LE(rec.hotspots[1].hotspotId);
		stream->writeByte(rec.hotspots[0].currentFrame);
		stream->writeByte(rec.hotspots[0].destFrame);
		stream->writeByte(rec.hotspots[1].currentFrame);
		stream->writeByte(rec.hotspots[1].destFrame);
		stream->writeByte(rec.blocked);
	}

	// Write end of list marker
	stream->writeUint16LE(0xffff);
}

void RoomExitJoinList::loadFromStream(Common::ReadStream *stream) {
	for (RoomExitJoinList::iterator i = begin(); i != end(); ++i) {
		RoomExitJoinData &rec = **i;

		uint16 hotspot1Id = stream->readUint16LE();
		if (hotspot1Id == 0xffff) error("Invalid room exit join list");
		uint16 hotspot2Id = stream->readUint16LE();

		if ((rec.hotspots[0].hotspotId != hotspot1Id) ||
			(rec.hotspots[1].hotspotId != hotspot2Id))
			break;

		rec.hotspots[0].currentFrame = stream->readByte();
		rec.hotspots[0].destFrame    = stream->readByte();
		rec.hotspots[1].currentFrame = stream->readByte();
		rec.hotspots[1].destFrame    = stream->readByte();
		rec.blocked = stream->readByte();
	}

	// Read final end of list marker
	stream->readUint16LE();
}

// Hotspot action record

HotspotActionData::HotspotActionData(HotspotActionResource *rec) {
	action = (Action) rec->action;
	// FIXME: some compilers may add padding to properly align the second member
	sequenceOffset = READ_LE_UINT16(((byte *)rec) + 1);
}

uint16 HotspotActionList::getActionOffset(Action action) {
	iterator i;
	for (i = begin(); i != end(); ++i) {
		HotspotActionData const &rec = **i;
		if (rec.action == action) return rec.sequenceOffset;
	}

	return 0;
}


// Hotspot data

HotspotData::HotspotData(HotspotResource *rec) {
	hotspotId = READ_LE_UINT16(&rec->hotspotId);
	nameId = READ_LE_UINT16(&rec->nameId);
	descId = READ_LE_UINT16(&rec->descId);
	descId2 = READ_LE_UINT16(&rec->descId2);
	actions = READ_LE_UINT32(&rec->actions);
	actionsOffset = READ_LE_UINT16(&rec->actionsOffset);
	flags = (byte) (actions >> 24) & 0xf0;
	actions &= 0xfffffff;

	roomNumber = READ_LE_UINT16(&rec->roomNumber);
	layer = rec->layer;
	scriptLoadFlag = rec->scriptLoadFlag;
	loadOffset = READ_LE_UINT16(&rec->loadOffset);
	startX = READ_LE_UINT16(&rec->startX);
	startY = READ_LE_UINT16(&rec->startY);
	width = READ_LE_UINT16(&rec->width);
	height = READ_LE_UINT16(&rec->height);
	widthCopy = READ_LE_UINT16(&rec->widthCopy);
	heightCopy = READ_LE_UINT16(&rec->heightCopy);
	yCorrection = READ_LE_UINT16(&rec->yCorrection);
	walkX = READ_LE_UINT16(&rec->walkX);
	walkY = READ_LE_UINT16(&rec->walkY);
	talkX = rec->talkX;
	talkY = rec->talkY;
	colorOffset = READ_LE_UINT16(&rec->colorOffset);
	animRecordId = READ_LE_UINT16(&rec->animRecordId);
	hotspotScriptOffset = READ_LE_UINT16(&rec->hotspotScriptOffset);
	talkScriptOffset = READ_LE_UINT16(&rec->talkScriptOffset);
	tickProcId = READ_LE_UINT16(&rec->tickProcId);
	tickTimeout = READ_LE_UINT16(&rec->tickTimeout);
	tickScriptOffset = READ_LE_UINT16(&rec->tickScriptOffset);
	characterMode = (CharacterMode) READ_LE_UINT16(&rec->characterMode);
	delayCtr = READ_LE_UINT16(&rec->delayCtr);
	flags2 = READ_LE_UINT16(&rec->flags2);
	headerFlags = READ_LE_UINT16(&rec->hdrFlags);

	// Initialize runtime fields
	actionCtr = 0;
	blockedState = BS_NONE;
	blockedFlag = false;
	coveredFlag = VB_INITIAL;
	talkMessageId = 0;
	talkerId = 0;
	talkDestCharacterId = 0;
	talkCountdown = 0;
	useHotspotId = 0;
	pauseCtr = 0;
	actionHotspotId = 0;
	talkOverride = 0;
	talkGate = 0;
	scriptHotspotId = 0;

	// Get the NPC schedule, if any
	npcScheduleId = READ_LE_UINT16(&rec->npcSchedule);
}

void HotspotData::saveToStream(Common::WriteStream *stream) const {
	// Write out the basic fields
	stream->writeUint16LE(nameId);
	stream->writeUint16LE(descId);
	stream->writeUint16LE(descId2);
	stream->writeUint32LE(actions);
	stream->writeByte(flags);
	stream->writeByte(flags2);
	stream->writeByte(headerFlags);
	stream->writeSint16LE(startX);
	stream->writeSint16LE(startY);
	stream->writeUint16LE(roomNumber);
	stream->writeByte(layer);
	stream->writeUint16LE(walkX);
	stream->writeUint16LE(walkY);

	stream->writeUint16LE(width);
	stream->writeUint16LE(height);
	stream->writeUint16LE(widthCopy);
	stream->writeUint16LE(heightCopy);
	stream->writeUint16LE(yCorrection);
	stream->writeUint16LE(hotspotScriptOffset);
	stream->writeUint16LE(tickProcId);
	stream->writeUint16LE(tickTimeout);
	stream->writeUint16LE(tickScriptOffset);
	stream->writeUint16LE(characterMode);
	stream->writeUint16LE(delayCtr);
	stream->writeUint16LE(animRecordId);

	// Write out the runtime fields
	stream->writeUint16LE(actionCtr);
	stream->writeUint16LE(blockedState);
	stream->writeByte((byte)blockedFlag);
	stream->writeByte((byte)coveredFlag);
	stream->writeUint16LE(talkMessageId);
	stream->writeUint16LE(talkerId);
	stream->writeUint16LE(talkDestCharacterId);
	stream->writeUint16LE(talkCountdown);
	stream->writeUint16LE(pauseCtr);
	stream->writeUint16LE(useHotspotId);
	stream->writeUint16LE(scriptHotspotId);
	stream->writeUint16LE(talkGate);
	stream->writeUint16LE(actionHotspotId);
	stream->writeUint16LE(talkOverride);
}

void HotspotData::loadFromStream(Common::ReadStream *stream) {
	// Read in the basic fields
	nameId = stream->readUint16LE();
	descId = stream->readUint16LE();
	descId2 = stream->readUint16LE();
	actions = stream->readUint32LE();
	flags = stream->readByte();
	flags2 = stream->readByte();
	headerFlags = stream->readByte();
	startX = stream->readSint16LE();
	startY = stream->readSint16LE();
	roomNumber = stream->readUint16LE();

	uint8 saveVersion = LureEngine::getReference().saveVersion();
	if (saveVersion >= 29)
		layer = stream->readByte();
	if (saveVersion >= 33) {
		walkX = stream->readUint16LE();
		walkY = stream->readUint16LE();
	}

	width = stream->readUint16LE();
	height = stream->readUint16LE();
	widthCopy = stream->readUint16LE();
	heightCopy = stream->readUint16LE();
	yCorrection = stream->readUint16LE();
	hotspotScriptOffset = stream->readUint16LE();
	tickProcId = stream->readUint16LE();
	tickTimeout = stream->readUint16LE();
	tickScriptOffset = stream->readUint16LE();
	characterMode = (CharacterMode) stream->readUint16LE();
	delayCtr = stream->readUint16LE();
	animRecordId = stream->readUint16LE();

	// Read in the runtime fields
	actionCtr = stream->readUint16LE();
	blockedState = (BlockedState)stream->readUint16LE();
	blockedFlag = stream->readByte() != 0;
	coveredFlag = (VariantBool)stream->readByte();
	talkMessageId = stream->readUint16LE();
	talkerId = stream->readUint16LE();
	talkDestCharacterId = stream->readUint16LE();
	talkCountdown = stream->readUint16LE();
	pauseCtr = stream->readUint16LE();
	useHotspotId = stream->readUint16LE();
	scriptHotspotId = stream->readUint16LE();
	talkGate = stream->readUint16LE();
	actionHotspotId = stream->readUint16LE();
	talkOverride = stream->readUint16LE();
}

// Hotspot data list

void HotspotDataList::saveToStream(Common::WriteStream *stream) const {
	for (const_iterator i = begin(); i != end(); ++i) {
		HotspotData const &hotspot = **i;
		stream->writeUint16LE(hotspot.hotspotId);
		hotspot.saveToStream(stream);
	}
	stream->writeUint16LE(0);
}

void HotspotDataList::loadFromStream(Common::ReadStream *stream) {
	Resources &res = Resources::getReference();
	uint16 hotspotId = stream->readUint16LE();
	while (hotspotId != 0) {
		HotspotData *hotspot = res.getHotspot(hotspotId);
		assert(hotspot);
		hotspot->loadFromStream(stream);
		hotspotId = stream->readUint16LE();
	}
}

// Hotspot override data

HotspotOverrideData::HotspotOverrideData(HotspotOverrideResource *rec) {
	hotspotId = READ_LE_UINT16(&rec->hotspotId);
	xs = READ_LE_UINT16(&rec->xs);
	ys = READ_LE_UINT16(&rec->ys);
	xe = READ_LE_UINT16(&rec->xe);
	ye = READ_LE_UINT16(&rec->ye);
}

// Hotspot animation movement frame

MovementData::MovementData(MovementResource *rec) {
	frameNumber = READ_LE_UINT16(&rec->frameNumber);
	xChange = READ_LE_UINT16(&rec->xChange);
	yChange = READ_LE_UINT16(&rec->yChange);
}

// List of movement frames

bool MovementDataList::getFrame(uint16 currentFrame, int16 &xChange,
							   int16 &yChange, uint16 &nextFrame) {
	if (empty()) return false;
	bool foundFlag = false;
	iterator i;

	for (i = begin(); i != end(); ++i) {
		MovementData const &rec = **i;
		if (foundFlag || (i == begin())) {
			xChange = rec.xChange;
			yChange = rec.yChange;
			nextFrame = rec.frameNumber;
			if (foundFlag) return true;
		}
		if (rec.frameNumber == currentFrame) foundFlag = true;
	}

	return true;
}

// Hotspot animation data

HotspotAnimData::HotspotAnimData(HotspotAnimResource *rec) {
	animRecordId = READ_LE_UINT16(&rec->animRecordId);
	animId = READ_LE_UINT16(&rec->animId);
	flags = READ_LE_UINT16(&rec->flags);

	upFrame = rec->upFrame;
	downFrame = rec->downFrame;
	leftFrame = rec->leftFrame;
	rightFrame = rec->rightFrame;
}

// Hotspot action lists

HotspotActionList::HotspotActionList(uint16 id, byte *data) {
	recordId = id;
	uint16  numItems = READ_LE_UINT16(data);
	data += 2;

	HotspotActionResource *actionRec = (HotspotActionResource *) data;

	for (int actionCtr = 0; actionCtr < numItems; ++actionCtr,
		GET_NEXT(actionRec, HotspotActionResource)) {

		HotspotActionData *actionEntry = new HotspotActionData(actionRec);
		push_back(HotspotActionList::value_type(actionEntry));
	}
}

HotspotActionList *HotspotActionSet::getActions(uint16 recordId) {
	HotspotActionSet::iterator i;
	for (i = begin(); i != end(); ++i) {
		HotspotActionList *list = (*i).get();
		if (list->recordId == recordId) return list;
	}

	return NULL;
}

// The following class holds the set of offsets for a character's talk set

TalkHeaderData::TalkHeaderData(uint16 charId, uint16 *entries) {
	uint16 *src, *dest;
	characterId = charId;

	// Get number of entries
	_numEntries = 0;
	src = entries;
	while (READ_LE_UINT16(src) != 0xffff) { ++src; ++_numEntries; }

	// Duplicate the list
	_data = (uint16 *) Memory::alloc(_numEntries * sizeof(uint16));
	src = entries; dest = _data;

	for (int ctr = 0; ctr < _numEntries; ++ctr, ++src, ++dest)
		*dest = READ_LE_UINT16(src);
}

TalkHeaderData::~TalkHeaderData() {
	free(_data);
}

uint16 TalkHeaderData::getEntry(int index) {
	if (index >= _numEntries)
		error("Invalid talk index %d specified for hotspot %xh",
			_numEntries, characterId);
	return _data[index];
}

// The following class holds a single talking entry

TalkEntryData::TalkEntryData(TalkDataResource *rec) {
	preSequenceId = READ_LE_UINT16(&rec->preSequenceId);
	descId = READ_LE_UINT16(&rec->descId);
	postSequenceId = READ_LE_UINT16(&rec->postSequenceId);
}

// The following class acts as a container for all the talk entries and
// responses for a single record Id

TalkData::TalkData(uint16 id) {
	recordId = id;
}

TalkData::~TalkData() {
	entries.clear();
	responses.clear();
}

TalkEntryData *TalkData::getResponse(int index) {
	TalkEntryList::iterator i = responses.begin();
	int v = index;
	while (v-- > 0) {
		if (i == responses.end())
			error("Invalid talk response index %d specified", index);
		++i;
	}

	return (*i).get();
}

// The following class acts as a container for all the NPC conversations

void TalkDataList::saveToStream(Common::WriteStream *stream) const {
	for (TalkDataList::const_iterator i = begin(); i != end(); ++i) {
		TalkData const &rec = **i;

		for (TalkEntryList::const_iterator i2 = rec.entries.begin(); i2 != rec.entries.end(); ++i2) {
			stream->writeUint16LE((*i2)->descId);
		}
	}
}

void TalkDataList::loadFromStream(Common::ReadStream *stream) {
	TalkDataList::iterator i;
	for (i = begin(); i != end(); ++i) {
		TalkData const &rec = **i;

		for (TalkEntryList::const_iterator i2 = rec.entries.begin(); i2 != rec.entries.end(); ++i2) {
			(*i2)->descId = stream->readUint16LE();
		}
	}
}

// The following class handles a set of coordinates a character should walk to
// if they're to exit a room to a designated secondary room

RoomExitCoordinates::RoomExitCoordinates(RoomExitCoordinateEntryResource *rec) {
	int ctr;

	for (ctr = 0; ctr < ROOM_EXIT_COORDINATES_NUM_ENTRIES; ++ctr) {
		uint16 tempY = READ_LE_UINT16(&rec->entries[ctr].y);
		_entries[ctr].x = READ_LE_UINT16(&rec->entries[ctr].x);
		_entries[ctr].y = tempY & 0xfff;
		_entries[ctr].roomNumber = READ_LE_UINT16(&rec->entries[ctr].roomNumber);
		_entries[ctr].hotspotIndexId = (tempY >> 12) << 4;
	}

	for (ctr = 0; ctr < ROOM_EXIT_COORDINATES_NUM_ROOMS; ++ctr)
		_roomIndex[ctr] = rec->roomIndex[ctr];
}

RoomExitCoordinates &RoomExitCoordinatesList::getEntry(uint16 roomNumber) {
	RoomExitCoordinatesList::iterator i = begin();
	while (--roomNumber > 0)
		++i;
	return **i;
}

RoomExitCoordinateData &RoomExitCoordinates::getData(uint16 destRoomNumber) {
	return _entries[_roomIndex[destRoomNumber - 1]];
}

// The following classes hold any sequence offsets that are being delayed

SequenceDelayData::SequenceDelayData(uint16 delay, uint16 seqOffset, bool canClearFlag) {
	// The delay is in number of seconds - convert it to remaining milliseconds
	timeoutCtr = delay * 1000;
	sequenceOffset = seqOffset;
	canClear = canClearFlag;
}

SequenceDelayData *SequenceDelayData::load(uint32 delay, uint16 seqOffset, bool canClearFlag) {
	SequenceDelayData *result = new SequenceDelayData();
	result->sequenceOffset = seqOffset;
	result->timeoutCtr = delay;
	result->canClear = canClearFlag;
	return result;
}

void SequenceDelayList::add(uint16 delay, uint16 seqOffset, bool canClear) {
	debugC(ERROR_DETAILED, kLureDebugScripts, "Delay List add sequence=%xh delay=%d canClear=%d",
		seqOffset, delay, (int)canClear);
	SequenceDelayData *entry = new SequenceDelayData(delay, seqOffset, canClear);
	push_front(SequenceDelayList::value_type(entry));
}

void SequenceDelayList::tick() {
	SequenceDelayList::iterator i;

	debugC(ERROR_DETAILED, kLureDebugScripts, "Delay List check start at time %d",
		g_system->getMillis());

	for (i = begin(); i != end(); ++i) {
		SequenceDelayData &entry = **i;
		debugC(ERROR_DETAILED, kLureDebugScripts, "Delay List check %xh at time %d", entry.sequenceOffset, entry.timeoutCtr);

		if (entry.timeoutCtr <= GAME_FRAME_DELAY) {
			// Timeout reached - delete entry from list and execute the sequence
			uint16 seqOffset = entry.sequenceOffset;
			erase(i);
			Script::execute(seqOffset);
			return;
		} else {
			entry.timeoutCtr -= GAME_FRAME_DELAY;
		}
	}
}

void SequenceDelayList::clear(bool forceClear) {
	SequenceDelayList::iterator i = begin();

	while (i != end()) {
		if ((*i)->canClear || forceClear)
			i = erase(i);
		else
			++i;
	}
}

void SequenceDelayList::saveToStream(Common::WriteStream *stream) const {
	for (SequenceDelayList::const_iterator i = begin(); i != end(); ++i) {
		SequenceDelayData const &entry = **i;
		stream->writeUint16LE(entry.sequenceOffset);
		stream->writeUint32LE(entry.timeoutCtr);
		stream->writeByte(entry.canClear);
	}

	stream->writeUint16LE(0);
}

void SequenceDelayList::loadFromStream(Common::ReadStream *stream) {
	clear(true);
	uint16 seqOffset;

	while ((seqOffset = stream->readUint16LE()) != 0) {
		uint32 delay = stream->readUint32LE();
		bool canClear = stream->readByte() != 0;
		push_back(SequenceDelayList::value_type(SequenceDelayData::load(delay, seqOffset, canClear)));
	}
}

// The following classes hold the NPC schedules

CharacterScheduleEntry::CharacterScheduleEntry(Action theAction, ...) {
	_parent = NULL;
	_action = theAction;

	va_list u_Arg;
	va_start(u_Arg, theAction);

	for (int paramCtr = 0; paramCtr < actionNumParams[_action]; ++paramCtr)
		_params[paramCtr] = (uint16) va_arg(u_Arg, int);

	va_end(u_Arg);
	_numParams = actionNumParams[_action];
}

CharacterScheduleEntry::CharacterScheduleEntry(CharacterScheduleSet *parentSet,
		CharacterScheduleResource *&rec) {
	_parent = parentSet;

	if ((rec->action == 0) || (READ_LE_UINT16(&rec->action) > NPC_JUMP_ADDRESS))
		error("Invalid action encountered reading NPC schedule");

	_action = (Action) READ_LE_UINT16(&rec->action);
	_numParams = actionNumParams[_action];
	for (int index = 0; index < _numParams; ++index)
		_params[index] = READ_LE_UINT16(&rec->params[index]);

	rec = (CharacterScheduleResource *) ((byte *) rec +
		(_numParams + 1) * sizeof(uint16));
}

CharacterScheduleEntry::CharacterScheduleEntry(CharacterScheduleEntry *src) {
	_parent = src->_parent;
	_action = src->_action;
	_numParams = src->_numParams;
	Common::copy(src->_params, src->_params + MAX_TELL_COMMANDS * 3, _params);
}

uint16 CharacterScheduleEntry::param(int index) {
	if ((index < 0) || (index >= numParams()))
		error("Invalid parameter index %d on handling action %d", index, _action);
	return _params[index];
}

void CharacterScheduleEntry::setDetails(Action theAction, ...) {
	_action = theAction;
	_numParams = actionNumParams[_action];

	va_list list;
	va_start(list, theAction);

	for (int paramCtr = 0; paramCtr < actionNumParams[_action]; ++paramCtr)
		_params[paramCtr] = (uint16) va_arg(list, int);

	va_end(list);
}

void CharacterScheduleEntry::setDetails2(Action theAction, int numParamEntries, uint16 *paramList) {
	_action = theAction;
	_numParams = numParamEntries;

	assert((numParamEntries >= 0) && (numParamEntries < (MAX_TELL_COMMANDS * 3)));
	for (int paramCtr = 0; paramCtr < numParamEntries; ++paramCtr)
		_params[paramCtr] = paramList[paramCtr];
}

CharacterScheduleEntry *CharacterScheduleEntry::next() {
	if (_parent) {
		CharacterScheduleSet::iterator i;
		for (i = _parent->begin(); i != _parent->end(); ++i) {
			if ((*i).get() == this) {
				++i;
				CharacterScheduleEntry *result = (i == _parent->end()) ? NULL : (*i).get();
				return result;
			}
		}
	}

	return NULL;
}

uint16 CharacterScheduleEntry::id() {
	return (_parent == NULL) ? 0 : _parent->getId(this);
}

CharacterScheduleSet::CharacterScheduleSet(CharacterScheduleResource *rec, uint16 setId) {
	// Load up all the entries in the schedule

	while (rec->action != 0) {
		CharacterScheduleEntry *r = new CharacterScheduleEntry(this, rec);
		push_back(CharacterScheduleSet::value_type(r));
	}

	_id = setId;
}

// Given a support data entry identifier, locates that entry in the list of data sets

CharacterScheduleEntry *CharacterScheduleList::getEntry(uint16 id, CharacterScheduleSet *currentSet) {
	// Respond to the special no entry with no record
	if (id == 0xffff) return NULL;

	// Handle jumps within a current set versus external jumps
	if ((id >> 10) == 0) {
		// Jump within current set
		if (currentSet == NULL)
			error("Local support data jump encountered outside of a support data sequence");
	} else {
		// Inter-set jump - locate the appropriate set
		int index = (id >> 10) - 1;

		iterator i = begin();
		while ((i != end()) && (index > 0)) {
			++i;
			--index;
		}

		if (i == end())
			error("Invalid index %d specified for support data set", id >> 8);
		currentSet = (*i).get();
	}

	// Get the indexed instruction in the specified set
	int instructionIndex = id & 0x3ff;
	CharacterScheduleSet::iterator i = currentSet->begin();
	while ((i != currentSet->end()) && (instructionIndex > 0)) {
		++i;
		--instructionIndex;
	}
	if (i == currentSet->end())
		error("Invalid index %d specified within support data set", id & 0x3ff);

	return (*i).get();
}

uint16 CharacterScheduleSet::getId(CharacterScheduleEntry *rec) {
	// Return an Id for the entry based on the id of the set combined with the
	// index of the specific entry
	uint16 result = _id << 10;

	iterator i;
	for (i = begin(); i != end(); ++i, ++result)
		if ((*i).get() == rec) break;
	if (i == end())
		error("Parent child relationship missing in character schedule set");
	return result;
}

// This classes is used to store a list of random action sets - one set per room

RandomActionSet::RandomActionSet(uint16 *&offset) {
	_roomNumber = READ_LE_UINT16(offset++);
	uint16 actionDetails = READ_LE_UINT16(offset++);
	_numActions = (actionDetails & 0xff);
	assert(_numActions <= 8);
	_types = new RandomActionType[_numActions];
	_ids = new uint16[_numActions];

	for (int actionIndex = 0; actionIndex < _numActions; ++actionIndex) {
		_ids[actionIndex] = READ_LE_UINT16(offset++);
		_types[actionIndex] = (actionDetails & (0x100 << actionIndex)) != 0 ? REPEATABLE : REPEAT_ONCE;
	}
}

RandomActionSet::~RandomActionSet() {
	delete[] _types;
	delete[] _ids;
}

RandomActionSet *RandomActionList::getRoom(uint16 roomNumber) {
	iterator i;
	for (i = begin(); i != end(); ++i) {
		RandomActionSet *v = (*i).get();
		if (v->roomNumber() == roomNumber)
			return v;
	}
	return NULL;
}

void RandomActionSet::saveToStream(Common::WriteStream *stream) const {
	stream->writeByte(numActions());
	for (int actionIndex = 0; actionIndex < _numActions; ++actionIndex)
		stream->writeByte((byte)_types[actionIndex]);
}

void RandomActionSet::loadFromStream(Common::ReadStream *stream) {
	int amount = stream->readByte();
	assert(amount == _numActions);
	for (int actionIndex = 0; actionIndex < _numActions; ++actionIndex)
		_types[actionIndex] = (RandomActionType)stream->readByte();
}


void RandomActionList::saveToStream(Common::WriteStream *stream) const {
	for (const_iterator i = begin(); i != end(); ++i)
		(*i)->saveToStream(stream);
}

void RandomActionList::loadFromStream(Common::ReadStream *stream) {
	for (iterator i = begin(); i != end(); ++i)
		(*i)->loadFromStream(stream);
}

// This class handles an indexed hotspot entry - which is used by the NPC code to
// determine whether exiting a room to another given room has an exit hotspot or not

RoomExitIndexedHotspotData::RoomExitIndexedHotspotData(RoomExitIndexedHotspotResource *rec) {
	roomNumber = rec->roomNumber;
	hotspotIndex = rec->hotspotIndex;
	hotspotId = READ_LE_UINT16(&rec->hotspotId);
}

uint16 RoomExitIndexedHotspotList::getHotspot(uint16 roomNumber, uint8 hotspotIndexId) {
	iterator i;
	for (i = begin(); i != end(); ++i) {
		RoomExitIndexedHotspotData const &entry = **i;
		if ((entry.roomNumber == roomNumber) && (entry.hotspotIndex == hotspotIndexId))
			return entry.hotspotId;
	}

	// No hotspot
	return 0xffff;
}

// Paused character list methods

PausedCharacter::PausedCharacter(uint16 SrcCharId, uint16 DestCharId) {
	srcCharId = SrcCharId;
	destCharId = DestCharId;
	counter = IDLE_COUNTDOWN_SIZE;
	charHotspot = Resources::getReference().getHotspot(DestCharId);
	assert(charHotspot);
}

void PausedCharacterList::reset(uint16 hotspotId) {
	iterator i;
	for (i = begin(); i != end(); ++i) {
		PausedCharacter &rec = **i;

		if (rec.srcCharId == hotspotId) {
			rec.counter = 1;
			if (rec.destCharId < START_EXIT_ID)
				rec.charHotspot->pauseCtr = 1;
		}
	}
}

void PausedCharacterList::countdown() {
	iterator i = begin();

	while (i != end()) {
		PausedCharacter &rec = **i;
		--rec.counter;

		// Handle reflecting counter to hotspot
		if (rec.destCharId < START_EXIT_ID)
			rec.charHotspot->pauseCtr = rec.counter + 1;

		// If counter has reached zero, remove entry from list
		if (rec.counter == 0)
			i = erase(i);
		else
			++i;
	}
}

void PausedCharacterList::scan(Hotspot &h) {
	iterator i;

	if (h.blockedState() != BS_NONE) {

		for (i = begin(); i != end(); ++i) {
			PausedCharacter &rec = **i;

			if (rec.srcCharId == h.hotspotId()) {
				rec.counter = IDLE_COUNTDOWN_SIZE;

				if (rec.destCharId < START_EXIT_ID)
					rec.charHotspot->pauseCtr = IDLE_COUNTDOWN_SIZE;
			}
		}
	}
}

int PausedCharacterList::check(uint16 charId, int numImpinging, uint16 *impingingList) {
	Resources &res = Resources::getReference();
	PausedCharacterList::iterator i;
	int result = 0;
	Hotspot *charHotspot = res.getActiveHotspot(charId);
	assert(charHotspot);

	for (int index = 0; index < numImpinging; ++index) {
		Hotspot *hotspot = res.getActiveHotspot(impingingList[index]);
		if ((!hotspot) || (!hotspot->currentActions().isEmpty() &&
			(hotspot->currentActions().top().action() == EXEC_HOTSPOT_SCRIPT)))
			// Entry is skipped if hotspot not present or is executing hotspot script
			continue;

		// Scan through the pause list to see if there's a record for the
		// calling character and the impinging list entry
		bool foundEntry = false;
		for (i = res.pausedList().begin(); !foundEntry && (i != res.pausedList().end()); ++i) {
			PausedCharacter const &rec = **i;
			foundEntry = (rec.srcCharId == charId) &&
				(rec.destCharId == hotspot->hotspotId());
		}

		if (foundEntry)
			// There was, so move to next impinging character entry
			continue;

		if ((hotspot->hotspotId() == PLAYER_ID) && !hotspot->coveredFlag()) {
			hotspot->updateMovement();
			return 1;
		}

		// Add a new paused character entry
		PausedCharacter *entry = new PausedCharacter(charId, hotspot->hotspotId());
		res.pausedList().push_back(PausedCharacterList::value_type(entry));
		charHotspot->setBlockedState(BS_INITIAL);

		if (hotspot->hotspotId() < START_EXIT_ID) {
			if ((charHotspot->characterMode() == CHARMODE_PAUSED) ||
				((charHotspot->pauseCtr() == 0) &&
				(charHotspot->characterMode() == CHARMODE_NONE))) {
				if (hotspot->characterMode() != CHARMODE_WAIT_FOR_INTERACT)
					hotspot->resource()->scriptHotspotId = charId;
			}

			hotspot->setPauseCtr(IDLE_COUNTDOWN_SIZE);
		}

		if (result == 0)
			charHotspot->setRandomDest();
		result = 2;
	}

	return result;
}

// Wrapper class for the barman lists

BarmanLists::BarmanLists() {
	for (int index = 0; index < 3; ++index)
		_barList[index] = default_barList[index];
}

void BarmanLists::reset() {
	for (int index = 0; index < 3; ++index)
		_barList[index] = default_barList[index];
}

BarEntry &BarmanLists::getDetails(uint16 roomNumber) {
	for (int index = 0; index < 3; ++index)
		if (_barList[index].roomNumber == roomNumber)
			return _barList[index];
	error("Invalid room %d specified for barman details retrieval", roomNumber);
}

void BarmanLists::saveToStream(Common::WriteStream *stream) const {
	for (int index = 0; index < 3; ++index) {
		uint16 value = (_barList[index].currentCustomer == NULL) ? 0 :
			(_barList[index].currentCustomer - &_barList[index].customers[0]) / sizeof(BarEntry) + 1;
		stream->writeUint16LE(value);
		for (int ctr = 0; ctr < NUM_SERVE_CUSTOMERS; ++ctr) {
			stream->writeUint16LE(_barList[index].customers[ctr].hotspotId);
			stream->writeByte(_barList[index].customers[ctr].serveFlags);
		}
	}
}

void BarmanLists::loadFromStream(Common::ReadStream *stream) {
	uint8 saveVersion = LureEngine::getReference().saveVersion();
	int numEntries = (saveVersion >= 30) ? 3 : 2;

	reset();
	for (int index = 0; index < numEntries; ++index) {
		int16 value = stream->readUint16LE();
		_barList[index].currentCustomer = ((value < 1) || (value > NUM_SERVE_CUSTOMERS)) ? NULL :
			&_barList[index].customers[value - 1];

		for (int ctr = 0; ctr < NUM_SERVE_CUSTOMERS; ++ctr) {
			_barList[index].customers[ctr].hotspotId = stream->readUint16LE();
			_barList[index].customers[ctr].serveFlags = stream->readByte();
		}
	}
}

// String list resource class

void StringList::load(MemoryBlock *data) {
	_data = Memory::allocate(data->size());
	_data->copyFrom(data);

	_numEntries = READ_LE_UINT16(_data->data());
	char *p = (char *) _data->data() + sizeof(uint16);

	_entries = (char **) Memory::alloc(_numEntries * sizeof(char *));

	for (int index = 0; index < _numEntries; ++index) {
		_entries[index] = p;
		p += strlen(p) + 1;
	}
}

void StringList::clear() {
	if (_numEntries != 0) {
		Memory::dealloc(_entries);
		delete _data;
		_numEntries = 0;
	}
}

// Field list and miscellaneous variables

ValueTableData::ValueTableData() {
	reset();
}

void ValueTableData::reset() {
	_numGroats = 0;
	_playerNewPos.roomNumber = 0;
	_playerNewPos.position.x = 0;
	_playerNewPos.position.y = 0;
	_hdrFlagMask = 1;

	for (uint16 index = 0; index < NUM_VALUE_FIELDS; ++index)
		_fieldList[index] = 0;
}

bool ValueTableData::isKnownField(uint16 fieldIndex) {
	return ((fieldIndex <= 10) && (fieldIndex != 6)) ||
		(fieldIndex == 15) || ((fieldIndex >= 18) && (fieldIndex <= 20));
}

uint16 ValueTableData::getField(uint16 fieldIndex) {
	if (fieldIndex > NUM_VALUE_FIELDS)
		error("Invalid field index specified %d", fieldIndex);
//	if (!isKnownField(fieldIndex))
//		warning("Unknown field index %d in GET_FIELD opcode", fieldIndex);
	return _fieldList[fieldIndex];
}

uint16 ValueTableData::getField(FieldName fieldName) {
	return getField((uint16) fieldName);
}

void ValueTableData::setField(uint16 fieldIndex, uint16 value) {
	if (fieldIndex > NUM_VALUE_FIELDS)
		error("Invalid field index specified %d", fieldIndex);
	_fieldList[fieldIndex] = value;
//	if (!isKnownField(fieldIndex))
//		warning("Unknown field index %d in SET_FIELD opcode", fieldIndex);
}

void ValueTableData::setField(FieldName fieldName, uint16 value) {
	setField((uint16) fieldName, value);
}

void ValueTableData::saveToStream(Common::WriteStream *stream) const {
	// Write out the special fields
	stream->writeUint16LE(_numGroats);
	stream->writeSint16LE(_playerNewPos.position.x);
	stream->writeSint16LE(_playerNewPos.position.y);
	stream->writeUint16LE(_playerNewPos.roomNumber);
	stream->writeByte(_hdrFlagMask);

	// Write out the special fields
	for (int index = 0; index < NUM_VALUE_FIELDS; ++index)
		stream->writeUint16LE(_fieldList[index]);
}

void ValueTableData::loadFromStream(Common::ReadStream *stream) {
	// Load special fields
	_numGroats = stream->readUint16LE();
	_playerNewPos.position.x = stream->readSint16LE();
	_playerNewPos.position.y = stream->readSint16LE();
	_playerNewPos.roomNumber = stream->readUint16LE();
	_hdrFlagMask = stream->readByte();

	// Read in the field list
	for (int index = 0; index < NUM_VALUE_FIELDS; ++index)
		_fieldList[index] = stream->readUint16LE();
}

/*-------------------------------------------------------------------------*/

// Current action entry class methods

CurrentActionEntry::CurrentActionEntry(CurrentAction newAction, uint16 roomNum) {
	_action = newAction;
	_supportData = NULL;
	_dynamicSupportData = false;
	_roomNumber = roomNum;
}

CurrentActionEntry::CurrentActionEntry(CurrentAction newAction, CharacterScheduleEntry *data, uint16 roomNum) {
	assert(data->parent() != NULL);
	_action = newAction;
	_supportData = data;
	_dynamicSupportData = false;
	_roomNumber = roomNum;
}

CurrentActionEntry::CurrentActionEntry(Action newAction, uint16 roomNum, uint16 param1, uint16 param2) {
	_action = DISPATCH_ACTION;
	_dynamicSupportData = true;
	_supportData = new CharacterScheduleEntry();
	uint16 params[2] = {param1, param2};
	_supportData->setDetails2(newAction, 2, params);
	_roomNumber = roomNum;
}

CurrentActionEntry::CurrentActionEntry(CurrentActionEntry *src) {
	_action = src->_action;
	_dynamicSupportData = src->_dynamicSupportData;
	_roomNumber = src->_roomNumber;
	if (!_dynamicSupportData)
		_supportData = src->_supportData;
	else if (src->_supportData == NULL)
		_supportData = NULL;
	else {
		_supportData = new CharacterScheduleEntry(src->_supportData);
	}
}

void CurrentActionEntry::setSupportData(uint16 entryId) {
	CharacterScheduleEntry &entry = supportData();

	CharacterScheduleEntry *newEntry = Resources::getReference().
		charSchedules().getEntry(entryId, entry.parent());
	setSupportData(newEntry);
}

void CurrentActionEntry::saveToStream(Common::WriteStream *stream) const {
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Saving hotspot action entry dyn=%d id=%d",
		hasSupportData(), hasSupportData() ? supportData().id() : 0);
	stream->writeByte((uint8) _action);
	stream->writeUint16LE(_roomNumber);
	stream->writeByte(hasSupportData());
	if (hasSupportData()) {
		// Handle the support data
		stream->writeByte(_dynamicSupportData);
		if (_dynamicSupportData) {
			// Write out the dynamic data
			stream->writeByte(supportData().action());
			stream->writeSint16LE(supportData().numParams());
			for (int index = 0; index < supportData().numParams(); ++index)
				stream->writeUint16LE(supportData().param(index));
		} else {
			// Write out the Id for the static entry
			stream->writeUint16LE(supportData().id());
		}
	}
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Finished saving hotspot action entry");
}

CurrentActionEntry *CurrentActionEntry::loadFromStream(Common::ReadStream *stream) {
	Resources &res = Resources::getReference();
	uint8 actionNum = stream->readByte();
	if (actionNum == 0xff) return NULL;
	CurrentActionEntry *result;

	uint16 roomNumber = stream->readUint16LE();
	bool hasSupportData = stream->readByte() != 0;

	if (!hasSupportData) {
		// An entry that doesn't have support data
		result = new CurrentActionEntry(
			(CurrentAction) actionNum, roomNumber);
	} else {
		// Handle support data for the entry
		bool dynamicData = stream->readByte() != 0;
		if (dynamicData) {
			// Load action entry that has dynamic data
			result = new CurrentActionEntry(
				(CurrentAction) actionNum, roomNumber);
			result->_supportData = new CharacterScheduleEntry();
			Action action = (Action) stream->readByte();
			int numParams = stream->readSint16LE();
			uint16 *paramList = new uint16[numParams];
			for (int index = 0; index < numParams; ++index)
				paramList[index] = stream->readUint16LE();

			result->_supportData->setDetails2(action, numParams, paramList);
			delete[] paramList;
			result->_dynamicSupportData = true;
		} else {
			// Load action entry with an NPC schedule entry
			uint16 entryId = stream->readUint16LE();
			CharacterScheduleEntry *entry = res.charSchedules().getEntry(entryId);
			result = new CurrentActionEntry((CurrentAction) actionNum, roomNumber);
			result->setSupportData(entry);
		}
	}

	return result;
}

Common::String CurrentActionStack::getDebugInfo() const {
	Common::String buffer;
	ActionsList::const_iterator i;

	buffer += Common::String::format("CurrentActionStack::list num_actions=%d\n", size());

	for (i = _actions.begin(); i != _actions.end(); ++i) {
		CurrentActionEntry const &entry = **i;
		buffer += Common::String::format("style=%d room#=%d", entry.action(), entry.roomNumber());

		if (entry.hasSupportData()) {
			CharacterScheduleEntry &rec = entry.supportData();

			buffer += Common::String::format(", action=%d params=", rec.action());

			if (rec.numParams() == 0)
				buffer += "none";
			else {
				buffer += Common::String::format("%d", rec.param(0));
				for (int ctr = 1; ctr < rec.numParams(); ++ctr) {
					buffer += Common::String::format(", %d", rec.param(ctr));
				}
			}
		}
		buffer += "\n";
	}

	return buffer;
}

void CurrentActionStack::saveToStream(Common::WriteStream *stream) const {
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Saving hotspot action stack");
	Common::String buffer = getDebugInfo();
	debugC(ERROR_DETAILED, kLureDebugAnimations, "%s", buffer.c_str());

	for (ActionsList::const_iterator i = _actions.begin(); i != _actions.end(); ++i) {
		(*i)->saveToStream(stream);
	}
	stream->writeByte(0xff);      // End of list marker
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Finished saving hotspot action stack");
}

void CurrentActionStack::loadFromStream(Common::ReadStream *stream) {
	CurrentActionEntry *rec;

	_actions.clear();
	while ((rec = CurrentActionEntry::loadFromStream(stream)) != NULL)
		_actions.push_back(ActionsList::value_type(rec));
}

void CurrentActionStack::copyFrom(CurrentActionStack &stack) {
	ActionsList::iterator i;

	for (i = stack._actions.begin(); i != stack._actions.end(); ++i) {
		CurrentActionEntry *rec = (*i).get();
		_actions.push_back(ActionsList::value_type(new CurrentActionEntry(rec)));
	}
}

} // End of namespace Lure
