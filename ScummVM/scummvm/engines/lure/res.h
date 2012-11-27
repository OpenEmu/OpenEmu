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

#ifndef LURE_RES_H
#define LURE_RES_H

#include "lure/luredefs.h"
#include "lure/memory.h"
#include "lure/res_struct.h"
#include "lure/hotspots.h"
#include "lure/palette.h"

#include "common/array.h"
#include "common/file.h"
#include "common/list.h"
#include "common/random.h"

namespace Lure {

enum TalkState {TALK_NONE, TALK_START, TALK_SELECT, TALK_RESPOND, TALK_RESPONSE_WAIT,
	TALK_RESPOND_2, TALK_RESPOND_3};

#define MAX_TALK_SELECTIONS 4
typedef TalkEntryData *TalkSelections[MAX_TALK_SELECTIONS];

struct TalkDialogDetails {
	Common::Rect bounds;
	bool active;
};

#define NUM_GIVE_TALK_IDS 6

class Resources {
private:
	Common::RandomSource &_rnd;
	Palette *_paletteSubset;
	MemoryBlock *_cursors;
	RoomDataList _roomData;
	HotspotDataList _hotspotData;
	HotspotOverrideList _hotspotOverrides;
	HotspotAnimList _animData;
	MemoryBlock *_scriptData;
	MemoryBlock *_script2Data;
	MemoryBlock *_messagesData;
	uint16 *_hotspotScriptData;
	RoomExitJoinList _exitJoins;
	HotspotList _activeHotspots;
	ValueTableData _fieldList;
	HotspotActionSet _actionsList;
	TalkHeaderList _talkHeaders;
	TalkDataList _talkData;
	Common::Array<uint16> _giveTalkIds;
	SequenceDelayList _delayList;
	Action _currentAction;
	MemoryBlock *_talkDialogData;
	RoomExitCoordinatesList _coordinateList;
	CharacterScheduleList _charSchedules;
	RandomActionList _randomActions;
	RoomExitIndexedHotspotList _indexedRoomExitHospots;
	PausedCharacterList _pausedList;
	BarmanLists _barmanLists;
	StringList _stringList;

	int numCharOffsets;
	uint16 *_charOffsets;

	TalkData *_activeTalkData;
	TalkState _talkState;
	TalkSelections _talkSelections;
	TalkDialogDetails _talkDetails;
	int _talkSelection;
	int _talkStartEntry;
	uint16 _talkingCharacter;
	byte _cursor[CURSOR_WIDTH * CURSOR_HEIGHT];

	void reloadData();
	void freeData();
public:
	Resources();
	~Resources();
	static Resources &getReference();
	void reset();

	byte *getResource(uint16 resId);
	RoomDataList &roomData() { return _roomData; }
	RoomData *getRoom(uint16 roomNumber);
	bool checkHotspotExtent(HotspotData *hotspot);
	void insertPaletteSubset(Palette &p);
	byte *getCursor(uint8 cursorNum);
	HotspotDataList &hotspotData() { return _hotspotData; }
	HotspotOverrideList &hotspotOverrides() { return _hotspotOverrides; }
	HotspotAnimList &animRecords() { return _animData; }
	MemoryBlock *scriptData() { return _scriptData; }
	MemoryBlock *hotspotScriptData() { return _script2Data; }
	MemoryBlock *messagesData() { return _messagesData; }
	uint16 getHotspotScript(uint16 index);
	HotspotList &activeHotspots() { return _activeHotspots; }
	uint16 getRandom() { return _rnd.getRandomNumber(0xffff); }
	HotspotData *getHotspot(uint16 hotspotId);
	Hotspot *getActiveHotspot(uint16 hotspotId);
	HotspotOverrideData *getHotspotOverride(uint16 hotspotId);
	HotspotAnimData *getAnimation(uint16 animRecordId);
	int getAnimationIndex(HotspotAnimData *animData);
	RoomExitJoinList &exitJoins() { return _exitJoins; }
	RoomExitJoinData *getExitJoin(uint16 hotspotId);
	uint16 getHotspotAction(uint16 actionsOffset, Action action);
	HotspotActionList *getHotspotActions(uint16 actionsOffset);
	TalkHeaderData *getTalkHeader(uint16 hotspotId);
	ValueTableData &fieldList() { return _fieldList; }
	SequenceDelayList &delayList() { return _delayList; }
	MemoryBlock &getTalkDialogData() { return *_talkDialogData; }
	RoomExitCoordinatesList &coordinateList() { return _coordinateList; }
	CharacterScheduleList &charSchedules() { return _charSchedules; }
	RandomActionList &randomActions() { return _randomActions; }
	RoomExitIndexedHotspotList &exitHotspots() { return _indexedRoomExitHospots; }
	PausedCharacterList &pausedList() { return _pausedList; }
	BarmanLists &barmanLists() { return _barmanLists; }
	StringList &stringList() { return _stringList; }
	uint16 getCharOffset(int index) {
		if (index >= numCharOffsets)
			error("Invalid index %d passed to script engine support data offset list", index);
		if (index == 1)
			error("support data list index #1 was referenced - special handlng TODO");
		return _charOffsets[index];
	}
	void copyCursorTo(Surface *s, uint8 cursorNum, int16 x, int16 y);

	uint16 numInventoryItems();
	void setTalkData(uint16 offset);
	TalkData *getTalkData() { return _activeTalkData; }
	void setTalkState(TalkState state) { _talkState = state; }
	TalkState getTalkState() { return _talkState; }
	TalkSelections &getTalkSelections() { return _talkSelections; }
	TalkDialogDetails &getTalkDetails() { return _talkDetails; }
	void setTalkSelection(int index) { _talkSelection = index; }
	int getTalkSelection() { return _talkSelection; }
	void setTalkStartEntry(int index) { _talkStartEntry = index; }
	int getTalkStartEntry() { return _talkStartEntry; }
	uint16 getTalkingCharacter() { return _talkingCharacter; }
	void setTalkingCharacter(uint16 index);
	uint16 getGiveTalkId(uint16 index) {
		if (index >= _giveTalkIds.size())
			error("Invalid give talk id specified");
		return _giveTalkIds[index];
	}
	void setCurrentAction(Action action) { _currentAction = action; }
	Action getCurrentAction() { return _currentAction; }
	const char *getCurrentActionStr() {
		if (_currentAction > EXAMINE)
			error("Invalid current action %d", _currentAction);
		return _stringList.getString(_currentAction);
	}
	Hotspot *activateHotspot(uint16 hotspotId);
	Hotspot *addHotspot(uint16 hotspotId);
	void addHotspot(Hotspot *hotspot);
	void deactivateHotspot(uint16 hotspotId, bool isDestId = false);
	void deactivateHotspot(Hotspot *hotspot);
	void saveToStream(Common::WriteStream *stream);
	void loadFromStream(Common::ReadStream *stream);
};

} // End of namespace Lure

#endif
