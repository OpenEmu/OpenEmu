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

#include "lure/res.h"
#include "lure/disk.h"
#include "lure/scripts.h"
#include "lure/screen.h"
#include "lure/lure.h"
#include "common/endian.h"
#include "common/events.h"

namespace Lure {

static Resources *int_resources = NULL;

Resources &Resources::getReference() {
	return *int_resources;
}

Resources::Resources() : _rnd(LureEngine::getReference().rnd()) {
	int_resources = this;
	reloadData();

	// Load the string list
	MemoryBlock *mb = Disk::getReference().getEntry(STRING_LIST_RESOURCE_ID);
	_stringList.load(mb);
	delete mb;
}

Resources::~Resources() {
	// Free up any loaded data
	freeData();

	// Free up constant data
	_stringList.clear();
}

void Resources::freeData() {
	_activeHotspots.clear();
	_roomData.clear();
	_hotspotData.clear();
	_hotspotOverrides.clear();
	_animData.clear();
	_exitJoins.clear();
	_delayList.clear();
	_charSchedules.clear();
	_randomActions.clear();
	_indexedRoomExitHospots.clear();
	_pausedList.clear();
	_actionsList.clear();
	_coordinateList.clear();
	_talkHeaders.clear();
	_talkData.clear();
	_giveTalkIds.clear();

	free(_hotspotScriptData);
	delete _paletteSubset;
	delete _scriptData;
	delete _script2Data;
	delete _talkDialogData;
	delete _messagesData;
	delete _cursors;
	delete[] _charOffsets;
}

struct AnimRecordTemp {
	uint16 *offset;
	MovementDataList *list;
};

void Resources::reset() {
	freeData();

	_fieldList.reset();
	_barmanLists.reset();
	_talkState = TALK_NONE;
	_activeTalkData = NULL;

	reloadData();
}

void Resources::reloadData() {
	Disk &d = Disk::getReference();
	bool isEGA = LureEngine::getReference().isEGA();
	MemoryBlock *mb, *paths;
	uint16 *offset, offsetVal;
	uint16 recordId, startOffset;
	int ctr;
	uint16 *v;

	// Get the palette subset data
	_paletteSubset = isEGA ? NULL : new Palette(ALT_PALETTE_RESOURCE_ID);

	// Load room data
	mb = d.getEntry(ROOM_DATA_RESOURCE_ID);
	paths = d.getEntry(ROOM_PATHS_RESOURCE_ID);

	offset = (uint16 *) mb->data();
	while ((offsetVal = READ_LE_UINT16(offset++)) != 0xffff) {
		if (offsetVal != 0) {
			// Get room resource
			RoomResource *rec = (RoomResource *) (mb->data() + offsetVal);

			RoomData *newEntry = new RoomData(rec, paths);
			_roomData.push_back(RoomDataList::value_type(newEntry));

			uint8 numExits = rec->numExits;
			if (numExits > 0) {
				RoomExitResource *exitRes = (RoomExitResource *)
					(mb->data() + offsetVal + sizeof(RoomResource));

				for (uint16 exitCtr = 0; exitCtr < numExits; ++exitCtr, ++exitRes) {
					RoomExitData *exit = new RoomExitData(exitRes);
					newEntry->exits.push_back(RoomExitList::value_type(exit));
				}
			}
		}
	}
	delete mb;
	delete paths;

	// Load room exits
	mb = d.getEntry(ROOM_EXITS_RESOURCE_ID);
	ctr = 0;
	for (;;) {
		offsetVal = READ_LE_UINT16(mb->data() + (ctr * 2));
		if (offsetVal == 0xffff) break;

		if (offsetVal != 0) {
			RoomData *room = getRoom(ctr);
			if (room) {
				RoomExitHotspotResource *re = (RoomExitHotspotResource *)
					(mb->data() + offsetVal);
				while (READ_LE_UINT16(&re->hotspotId) != 0xffff) {
					RoomExitHotspotData *newEntry = new RoomExitHotspotData(re);
					room->exitHotspots.push_back(RoomExitHotspotList::value_type(newEntry));
					++re;
				}
			}
		}
		++ctr;
	}
	delete mb;

	// Load room joins
	mb = d.getEntry(ROOM_EXIT_JOINS_RESOURCE_ID);
	RoomExitJoinResource *joinRec = (RoomExitJoinResource *) mb->data();
	while (READ_LE_UINT16(&joinRec->hotspot1Id) != 0xffff) {
		RoomExitJoinData *newEntry = new RoomExitJoinData(joinRec);
		_exitJoins.push_back(RoomExitJoinList::value_type(newEntry));

		GET_NEXT(joinRec, RoomExitJoinResource);
	}
	delete mb;

	// Load the set of NPC schedules
	mb = d.getEntry(NPC_SCHEDULES_RESOURCE_ID);

	// Load the lookup list of support data indexes used in the script engine
	numCharOffsets = 0;
	offset = (uint16 *) mb->data();
	while (READ_LE_UINT16(offset++) != 0xffff) ++numCharOffsets;
	_charOffsets = new uint16[numCharOffsets];
	offset = (uint16 *) mb->data();
	for (ctr = 0; ctr < numCharOffsets; ++ctr, ++offset)
		_charOffsets[ctr] = READ_LE_UINT16(offset);

	// Next load up the list of random actions your follower can do in each room

	++offset;
	while (READ_LE_UINT16(offset) != 0xffff) {
		RandomActionSet *actionSet = new RandomActionSet(offset);
		_randomActions.push_back(RandomActionList::value_type(actionSet));
	}

	// Loop through loading the schedules
	ctr = 0;
	while ((startOffset = READ_LE_UINT16(++offset)) != 0xffff) {
		CharacterScheduleResource *res = (CharacterScheduleResource *) (mb->data() + startOffset);
		CharacterScheduleSet *newEntry = new CharacterScheduleSet(res, ++ctr);
		_charSchedules.push_back(CharacterScheduleList::value_type(newEntry));
	}
	delete mb;

	// Load the hotspot list
	mb = d.getEntry(HOTSPOT_DATA_RESOURCE_ID);
	HotspotResource *hsRec = (HotspotResource *) mb->data();
	while (READ_LE_UINT16(&hsRec->hotspotId) != 0xffff) {
		HotspotData *newEntry = new HotspotData(hsRec);
		_hotspotData.push_back(HotspotDataList::value_type(newEntry));

		GET_NEXT(hsRec, HotspotResource);
	}
	delete mb;

	// Load the hotspot overrides
	mb = d.getEntry(HOTSPOT_OVERRIDE_DATA_RESOURCE_ID);
	HotspotOverrideResource *hsoRec = (HotspotOverrideResource *) mb->data();
	while (READ_LE_UINT16(&hsoRec->hotspotId) != 0xffff) {
		HotspotOverrideData *newEntry = new HotspotOverrideData(hsoRec);
		_hotspotOverrides.push_back(HotspotOverrideList::value_type(newEntry));
		++hsoRec;
	}
	delete mb;

	// Load the animation list
	mb = d.getEntry(ANIM_DATA_RESOURCE_ID);
	HotspotAnimResource *animRec = (HotspotAnimResource *) mb->data();
	while (READ_LE_UINT16(&animRec->animRecordId) != 0xffff) {
		HotspotAnimData *newEntry = new HotspotAnimData(animRec);
		_animData.push_back(HotspotAnimList::value_type(newEntry));

		// Handle any direction frames
		AnimRecordTemp dirEntries[4] = {
			{&animRec->leftOffset, &newEntry->leftFrames},
			{&animRec->rightOffset, &newEntry->rightFrames},
			{&animRec->upOffset, &newEntry->upFrames},
			{&animRec->downOffset, &newEntry->downFrames}};
		for (int dirCtr = 0; dirCtr < 4; ++dirCtr) {
			offsetVal = READ_LE_UINT16(dirEntries[dirCtr].offset);
			if (offsetVal != 0) {
				MovementResource *moveRec = (MovementResource *)
					(mb->data() + offsetVal);
				while (READ_LE_UINT16(&moveRec->frameNumber) != 0xffff) {
					MovementData *newMove = new MovementData(moveRec);
					dirEntries[dirCtr].list->push_back(MovementDataList::value_type(newMove));
					++moveRec;
				}
			}
		}

		++animRec;
	}
	delete mb;

	// Hotspot scripts
	mb = d.getEntry(HOTSPOT_SCRIPT_LIST_RESOURCE_ID);
	uint16 numEntries = mb->size() / 2;
	uint16 *srcVal = (uint16 *) mb->data();
	uint16 *destVal = _hotspotScriptData = (uint16 *)
		Memory::alloc(numEntries * sizeof(uint16));
	for (ctr = 0; ctr < numEntries; ++ctr, ++srcVal, ++destVal) {
		*destVal = READ_LE_UINT16(srcVal);
	}
	delete mb;

	// Handle the hotspot action lists
	mb = d.getEntry(ACTION_LIST_RESOURCE_ID);
	v = (uint16 *) mb->data();
	while ((recordId = READ_LE_UINT16(v)) != 0xffff) {
		++v;
		offsetVal = READ_LE_UINT16(v);
		++v;

		HotspotActionList *list = new HotspotActionList(
			recordId, mb->data() + offsetVal);
		_actionsList.push_back(HotspotActionSet::value_type(list));
	}
	delete mb;

	// Read in the talk data header
	mb = d.getEntry(TALK_HEADER_RESOURCE_ID);
	TalkHeaderResource *thHeader = (TalkHeaderResource *) mb->data();
	uint16 hotspotId;
	while ((hotspotId = READ_LE_UINT16(&thHeader->hotspotId)) != 0xffff) {
		uint16 *offsets = (uint16 *) (mb->data() + READ_LE_UINT16(&thHeader->offset));
		TalkHeaderData *newEntry = new TalkHeaderData(hotspotId, offsets);

		_talkHeaders.push_back(TalkHeaderList::value_type(newEntry));
		++thHeader;
	}
	delete mb;

	// Read in the talk data entries
	mb = d.getEntry(TALK_DATA_RESOURCE_ID);

	// First get the list of give talk Ids
	v = (uint16 *) mb->data();

	for (int talkIndex = 0; talkIndex < NUM_GIVE_TALK_IDS; ++talkIndex)
		_giveTalkIds.push_back(READ_LE_UINT16(v++));

	// Get the following talk ata

	byte *dataStart = (byte *) v;
	TalkDataHeaderResource *tdHeader = (TalkDataHeaderResource *) dataStart;

	while ((recordId = READ_LE_UINT16(&tdHeader->recordId)) != 0xffff) {
		TalkData *data = new TalkData(recordId);

		TalkDataResource *entry = (TalkDataResource *) (dataStart + READ_LE_UINT16(&tdHeader->listOffset));
		while (READ_LE_UINT16(&entry->preSequenceId) != 0xffff) {
			TalkEntryData *newEntry = new TalkEntryData(entry);
			data->entries.push_back(TalkEntryList::value_type(newEntry));
			++entry;
		}

		entry = (TalkDataResource *) (dataStart + READ_LE_UINT16(&tdHeader->responsesOffset));
		while (READ_LE_UINT16(&entry->preSequenceId) != 0xffff) {
			TalkEntryData *newEntry = new TalkEntryData(entry);
			data->responses.push_back(TalkEntryList::value_type(newEntry));
			++entry;
		}

		_talkData.push_back(TalkDataList::value_type(data));
		++tdHeader;
	}
	delete mb;

	// Load in the list of room exit coordinates
	mb = d.getEntry(EXIT_COORDINATES_RESOURCE_ID);
	RoomExitCoordinateEntryResource *coordRec = (RoomExitCoordinateEntryResource *) mb->data();
	while (READ_LE_UINT16(coordRec) != 0xffff) {
		RoomExitCoordinates *newEntry = new RoomExitCoordinates(coordRec);
		_coordinateList.push_back(RoomExitCoordinatesList::value_type(newEntry));
		++coordRec;
	}
	delete mb;

	// Load the list of room exit hotspot Ids
	mb = d.getEntry(EXIT_HOTSPOT_ID_LIST);
	RoomExitIndexedHotspotResource *indexedRec = (RoomExitIndexedHotspotResource *) mb->data();
	while (READ_LE_UINT16(indexedRec) != 0xffff) {
		_indexedRoomExitHospots.push_back(RoomExitIndexedHotspotList::value_type(new RoomExitIndexedHotspotData(indexedRec)));
		indexedRec++;
	}
	delete mb;

	// Initialize delay list
	_delayList.clear(true);

	// Load miscellaneous data
	_cursors = d.getEntry(CURSOR_RESOURCE_ID);
	_scriptData = d.getEntry(SCRIPT_DATA_RESOURCE_ID);
	_script2Data = d.getEntry(SCRIPT2_DATA_RESOURCE_ID);
	_messagesData = d.getEntry(MESSAGES_LIST_RESOURCE_ID);
	_talkDialogData = d.getEntry(TALK_DIALOG_RESOURCE_ID);

	_activeTalkData = NULL;
	_currentAction = NONE;
	_talkState = TALK_NONE;
	_talkSelection = 0;
	_talkStartEntry = 0;
	_talkDetails.active = false;
	_talkingCharacter = 0;
}

RoomExitJoinData *Resources::getExitJoin(uint16 hotspotId) {
	RoomExitJoinList::iterator i;

	for (i = _exitJoins.begin(); i != _exitJoins.end(); ++i) {
		RoomExitJoinData *rec = (*i).get();
		if ((rec->hotspots[0].hotspotId == hotspotId) || (rec->hotspots[1].hotspotId == hotspotId))
			return rec;
	}

	return NULL;
}

uint16 Resources::getHotspotScript(uint16 index) {
	return _hotspotScriptData[index];
}

RoomData *Resources::getRoom(uint16 roomNumber) {
	RoomDataList::iterator i;

	for (i = _roomData.begin(); i != _roomData.end(); ++i) {
		RoomData *rec = (*i).get();
		if (rec->roomNumber == roomNumber) return rec;
	}

	return NULL;
}

bool Resources::checkHotspotExtent(HotspotData *hotspot) {
	uint16 roomNum = hotspot->roomNumber;
	RoomData *room = getRoom(roomNum);
	return (hotspot->startX >= room->clippingXStart) && ((room->clippingXEnd == 0) ||
			(hotspot->startX + 32 < room->clippingXEnd));
}

void Resources::insertPaletteSubset(Palette &p) {
	p.palette()->copyFrom(_paletteSubset->palette(), 0, 129*4, 60*4);
	p.palette()->copyFrom(_paletteSubset->palette(), 60*4, 220*4, 8*4);
}

byte *Resources::getCursor(uint8 cursorNum) {
	if (!LureEngine::getReference().isEGA())
		return _cursors->data() + (cursorNum * CURSOR_SIZE);

	Common::fill(&_cursor[0], &_cursor[0] + CURSOR_SIZE, 0);
	byte *pSrc = _cursors->data() + (cursorNum * 64);
	byte *pDest = &_cursor[0];

	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 2; ++x) {
			for (int planeNum = 0; planeNum < 2; ++planeNum, ++pSrc) {

				byte v = *pSrc;
				for (int bitCtr = 0; bitCtr < 8; ++bitCtr, v <<= 1) {
					if ((v & 0x80) != 0)
						*(pDest + bitCtr) |= 1 << planeNum;
					else
						*(pDest + bitCtr) &= ~(1 << planeNum);
				}
			}

			pDest += 8;
		}
	}

	// Post-process the cells to adjust the color
	for (int index = 0; index < CURSOR_SIZE; ++index) {
		if (_cursor[index] == 3) _cursor[index] = 15;
	}

	return &_cursor[0];
}

HotspotData *Resources::getHotspot(uint16 hotspotId) {
	HotspotDataList::iterator i;

	for (i = _hotspotData.begin(); i != _hotspotData.end(); ++i) {
		HotspotData *rec = (*i).get();
		if (rec->hotspotId == hotspotId) return rec;
	}

	return NULL;
}

Hotspot *Resources::getActiveHotspot(uint16 hotspotId) {
	HotspotList::iterator i;

	for (i = _activeHotspots.begin(); i != _activeHotspots.end(); ++i) {
		Hotspot *rec = (*i).get();
		if (rec->hotspotId() == hotspotId) return rec;
	}

	return NULL;
}


HotspotOverrideData *Resources::getHotspotOverride(uint16 hotspotId) {
	HotspotOverrideList::iterator i;

	for (i = _hotspotOverrides.begin(); i != _hotspotOverrides.end(); ++i) {
		HotspotOverrideData *rec = (*i).get();
		if (rec->hotspotId == hotspotId) return rec;
	}

	return NULL;
}

HotspotAnimData *Resources::getAnimation(uint16 animRecordId) {
	HotspotAnimList::iterator i;

	for (i = _animData.begin(); i != _animData.end(); ++i) {
		HotspotAnimData *rec = (*i).get();
		if (rec->animRecordId == animRecordId) return rec;
	}

	return NULL;
}

int Resources::getAnimationIndex(HotspotAnimData *animData) {
	HotspotAnimList::iterator i;
	int index = 0;

	for (i = _animData.begin(); i != _animData.end(); ++i, ++index) {
		HotspotAnimData *rec = (*i).get();
		if (rec == animData)
			return index;
	}

	return -1;
}

uint16 Resources::getHotspotAction(uint16 actionsOffset, Action action) {
	HotspotActionList *list = _actionsList.getActions(actionsOffset);
	uint16 offset = (!list) ? 0 : list->getActionOffset(action);
	debugC(ERROR_DETAILED, kLureDebugHotspots,
		"Resources::getHotspotAction actionsOffset=%xh result=%xh", actionsOffset, offset);
	return offset;
}

TalkHeaderData *Resources::getTalkHeader(uint16 hotspotId) {
	TalkHeaderList::iterator i;
	for (i = _talkHeaders.begin(); i != _talkHeaders.end(); ++i) {
		TalkHeaderData *rec = (*i).get();
		if (rec->characterId == hotspotId) return rec;
	}
	return NULL;
}

HotspotActionList *Resources::getHotspotActions(uint16 actionsOffset) {
	return _actionsList.getActions(actionsOffset);
}

void Resources::setTalkingCharacter(uint16 id) {
	Resources &res = Resources::getReference();

	if (_talkingCharacter != 0) {
		deactivateHotspot(_talkingCharacter, true);
		HotspotData *charHotspot = res.getHotspot(_talkingCharacter);
		assert(charHotspot);
		charHotspot->talkDestCharacterId = 0;

		if (_talkingCharacter != id)
			charHotspot->talkCountdown = 0;
	}

	_talkingCharacter = id;

	if (_talkingCharacter != 0) {
		Hotspot *character = getActiveHotspot(id);
		assert(character);

		// Add the special "voice" animation above the character
		Hotspot *hotspot = new Hotspot(character, VOICE_ANIM_IDX);
		addHotspot(hotspot);
	}
}
uint16 englishLoadOffsets[] = {0x3afe, 0x41BD, 0x7167, 0x7172, 0x8617, 0x88ac, 0};

Hotspot *Resources::activateHotspot(uint16 hotspotId) {
	Resources &resources = Resources::getReference();
	HotspotData *res = getHotspot(hotspotId);
	if (!res) return NULL;
	res->roomNumber &= 0x7fff; // clear any suppression bit in room #

	// Make sure that the hotspot isn't already active
	Hotspot *h = getActiveHotspot(hotspotId);
	if (h != NULL)
		return h;

	// If it's NPC with a schedule, then activate the schedule
	if ((res->npcScheduleId != 0) && (res->npcSchedule.isEmpty())) {
		CharacterScheduleEntry *entry = resources.charSchedules().getEntry(res->npcScheduleId);
		res->npcSchedule.addFront(DISPATCH_ACTION, entry, res->roomNumber);
	}

	// Check the script load flag
	if (res->scriptLoadFlag) {
		// Execute a script rather than doing a standard load
		Script::execute(res->loadOffset);
	} else {
		// Standard load
		bool loadFlag = true;
		uint16 talkIndex;

		switch (res->loadOffset) {
		case 1:
			// Copy protection check - since the game is freeware now, ignore it
			loadFlag = false;
			break;

		case 2:
			// Empty handler used to prevent loading hotspots that
			// are yet to be active (such as the straw fire)
			loadFlag = false;
			break;

		case 3:
		case 4:
			// Standard animation load
			break;

		case 5:
			// Custom loader used by the notice hotspot 42ah in room #20
			talkIndex = _fieldList.getField(TALK_INDEX);
			if ((talkIndex < 8) || (talkIndex >= 14))
				// Don't load hotspot
				loadFlag = false;
			else
				// Make the notice be on-screen
				res->startY = 85;
			break;

		case 6:
			// Torch in room #1
			loadFlag = _fieldList.getField(TORCH_HIDE) == 0;
			break;

		default:
			// All others simply activate the hotspot
			warning("Hotspot %d uses unknown load offset index %d",
				res->hotspotId, res->loadOffset);
		}

		if (loadFlag) {
			Hotspot *hotspot = addHotspot(hotspotId);
			assert(hotspot);

			// Special post-load handling
			if (res->loadOffset == 3) hotspot->setPersistant(true);
			if (res->loadOffset == 5) hotspot->handleTalkDialog();
			if (hotspotId == CASTLE_SKORL_ID) {
				// The Castle skorl has a default room #99, so it needs to be adjusted dynamically
				res->npcSchedule.clear();
				CharacterScheduleEntry *entry = resources.charSchedules().getEntry(res->npcScheduleId);
				res->npcSchedule.addFront(DISPATCH_ACTION, entry, res->roomNumber);
			}
			if ((hotspotId == GOEWIN_ID) && (hotspot->roomNumber() == 39)) {
				// WORKAROUND: When you re-join Goewin in the caves, clear her schedule. This may prevent a
				// situation where you could close the left door, and she'd be permanently stuck trying to go
				// the next room on the left, since her old schedule still had her following your old path
				hotspot->currentActions().clear();

				// Since she's no longer a follower, clear her start room field
				hotspot->setStartRoomNumber(0);
			}

			// TODO: Figure out why there's a room set in the animation decode for a range of characters,
			// particularly since it doesn't seem to match what happens in-game
			/*
			if ((hotspot->hotspotId() >= RATPOUCH_ID) &&
				(hotspot->hotspotId() < FIRST_NONCHARACTER_ID) &&
				(hotspot->roomNumber() < 42)) {
				// Start wandering characters off in room 24
				hotspot->setRoomNumber(24);
				hotspot->setPosition(64, 116);
				_fieldList.wanderingCharsLoaded() = true;
			}
			*/

			return hotspot;
		}
	}

	return NULL;
}

Hotspot *Resources::addHotspot(uint16 hotspotId) {
	HotspotData *hData = getHotspot(hotspotId);
	assert(hData);
	Hotspot *hotspot = new Hotspot(hData);
	_activeHotspots.push_back(HotspotList::value_type(hotspot));

	if (hotspotId < FIRST_NONCHARACTER_ID) {
		// Default characters to facing upwards until they start moving
		hotspot->setDirection(UP);
		hotspot->setCharRectY(0);

		// When reactivating an NPC, ensure that their previous state wasn't PROCESSING_PATH, since
		// the pause has destroyed the previously decided destination position
		if (!hData->npcSchedule.isEmpty() && (hData->npcSchedule.top().action() == PROCESSING_PATH))
			hData->npcSchedule.top().setAction(DISPATCH_ACTION);
	}

	return hotspot;
}

void Resources::addHotspot(Hotspot *hotspot) {
	_activeHotspots.push_back(HotspotList::value_type(hotspot));
}

void Resources::deactivateHotspot(uint16 hotspotId, bool isDestId) {
	HotspotList::iterator i = _activeHotspots.begin();

	while (i != _activeHotspots.end()) {
		Hotspot const &h = **i;
		if ((!isDestId && (h.hotspotId() == hotspotId)) ||
			(isDestId && (h.destHotspotId() == hotspotId) && (h.hotspotId() == 0xffff))) {
			_activeHotspots.erase(i);
			break;
		}

		i++;
	}
}

void Resources::deactivateHotspot(Hotspot *hotspot) {
	HotspotList::iterator i = _activeHotspots.begin();

	while (i != _activeHotspots.end()) {
		Hotspot *h = (*i).get();
		if (h == hotspot) {
			_activeHotspots.erase(i);
			break;
		}

		i++;
	}
}

uint16 Resources::numInventoryItems() {
	uint16 numItems = 0;
	HotspotDataList &list = _hotspotData;
	HotspotDataList::iterator i;
	for (i = list.begin(); i != list.end(); ++i) {
		if ((*i)->roomNumber == PLAYER_ID) ++numItems;
	}

	return numItems;
}

void Resources::copyCursorTo(Surface *s, uint8 cursorNum, int16 x, int16 y) {
	byte *pSrc = getCursor(cursorNum);
	byte *pDest = s->data().data() + (y * FULL_SCREEN_WIDTH) + x;

	for (int yP = 0; yP < CURSOR_HEIGHT; ++yP) {
		for (int xP = 0; xP < CURSOR_WIDTH; ++xP) {
			if (*pSrc != 0) *pDest = *pSrc;
			++pSrc;
			++pDest;
		}
		pDest += FULL_SCREEN_WIDTH - CURSOR_WIDTH;
	}
}

void Resources::setTalkData(uint16 offset) {
	if (offset == 0) {
		_activeTalkData = NULL;
		return;
	}

	TalkDataList::iterator i;
	for (i = _talkData.begin(); i != _talkData.end(); ++i) {
		TalkData *rec = (*i).get();
		if (rec->recordId == offset) {
			_activeTalkData = rec;
			return;
		}
	}

	error("Unknown talk entry offset %d requested", offset);
}

void Resources::saveToStream(Common::WriteStream *stream) {
	// Save basic fields
	stream->writeUint16LE(_talkingCharacter);

	// Save out the schedule for any non-active NPCs
	HotspotDataList::iterator i;
	for (i = _hotspotData.begin(); i != _hotspotData.end(); ++i) {
		HotspotData const &rec = **i;
		if (!rec.npcSchedule.isEmpty()) {
			Hotspot *h = getActiveHotspot(rec.hotspotId);
			if (h == NULL) {
				stream->writeUint16LE(rec.hotspotId);
				rec.npcSchedule.saveToStream(stream);
			}
		}
	}
	stream->writeUint16LE(0xffff);

	// Save sublist data
	_hotspotData.saveToStream(stream);
	_activeHotspots.saveToStream(stream);
	_fieldList.saveToStream(stream);
	_randomActions.saveToStream(stream);
	_barmanLists.saveToStream(stream);
	_exitJoins.saveToStream(stream);
	_roomData.saveToStream(stream);
	_delayList.saveToStream(stream);
	_talkData.saveToStream(stream);
}

void Resources::loadFromStream(Common::ReadStream *stream) {
	uint8 saveVersion = LureEngine::getReference().saveVersion();

	if (saveVersion >= 26) {
		debugC(ERROR_DETAILED, kLureDebugScripts, "Loading resource data");
		_talkingCharacter = stream->readUint16LE();
	} else {
		_talkingCharacter = 0;
	}

	_talkState = TALK_NONE;
	_activeTalkData = NULL;

	if (saveVersion >= 31) {
		// Load in any schedules for non-active NPCS
		debugC(ERROR_DETAILED, kLureDebugScripts, "Loading NPC schedules");
		uint16 hotspotId;
		while ((hotspotId = stream->readUint16LE()) != 0xffff) {
			HotspotData *hotspot = getHotspot(hotspotId);
			assert(hotspot);
			hotspot->npcSchedule.loadFromStream(stream);
		}
	}

	debugC(ERROR_DETAILED, kLureDebugScripts, "Loading hotspot data");
	_hotspotData.loadFromStream(stream);
	debugC(ERROR_DETAILED, kLureDebugScripts, "Loading active hotspots");
	_activeHotspots.loadFromStream(stream);
	debugC(ERROR_DETAILED, kLureDebugScripts, "Loading fields");
	_fieldList.loadFromStream(stream);
	debugC(ERROR_DETAILED, kLureDebugScripts, "Loading random actions");
	_randomActions.loadFromStream(stream);
	debugC(ERROR_DETAILED, kLureDebugScripts, "Loading barman lists");
	_barmanLists.loadFromStream(stream);
	debugC(ERROR_DETAILED, kLureDebugScripts, "Loading room exit joins");
	_exitJoins.loadFromStream(stream);
	debugC(ERROR_DETAILED, kLureDebugScripts, "Loading walkable paths");
	_roomData.loadFromStream(stream);
	debugC(ERROR_DETAILED, kLureDebugScripts, "Loading delay list");
	_delayList.loadFromStream(stream);

	if (saveVersion >= 32) {
		debugC(ERROR_DETAILED, kLureDebugScripts, "Loading talk data");
		_talkData.loadFromStream(stream);
	}

	debugC(ERROR_DETAILED, kLureDebugScripts, "Finished loading");
}

} // End of namespace Lure
