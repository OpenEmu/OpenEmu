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

#ifndef LURE_HOTSPOTS_H
#define LURE_HOTSPOTS_H

#include "lure/luredefs.h"
#include "lure/screen.h"
#include "lure/disk.h"
#include "lure/res_struct.h"

namespace Lure {

#define MAX_NUM_IMPINGING 10

class Hotspot;
class HotspotTickHandlers;

class Support {
private:
	static bool changeRoomCheckBumped(Hotspot &h);
public:
	static int findIntersectingCharacters(Hotspot &h, uint16 *charList, int16 xp = -1, int16 yp = -1, int roomNumber = -1);
	static bool checkForIntersectingCharacter(Hotspot &h, int16 xp = -1, int16 yp = -1, int roomNumber = -1);
	static bool checkRoomChange(Hotspot &h);
	static void characterChangeRoom(Hotspot &h, uint16 roomNumber,
								  int16 newX, int16 newY, Direction dir);
	static bool charactersIntersecting(HotspotData *hotspot1, HotspotData *hotspot2);
	static bool isCharacterInList(uint16 *lst, int numEntries, uint16 charId);
};

typedef void (HotspotTickHandlers::*HandlerMethodPtr)(Hotspot &h);

class HotspotTickHandlers {
private:
	// Special variable used across multiple calls to followerAnimHandler
	int countdownCtr;

	// Special variables used across multiple calls to talkAnimHandler
	TalkEntryData *_talkResponse;
	uint16 talkDestCharacter;

	// Special variable used across multiple calls to barmanAnimHandler
	bool ewanXOffset;

	// Support methods
	void npcRoomChange(Hotspot &h);
	void talkEndConversation();

	// Handler methods
	void defaultHandler(Hotspot &h);
	void voiceBubbleAnimHandler(Hotspot &h);
	void standardAnimHandler(Hotspot &h);
	void standardAnimHandler2(Hotspot &h);
	void standardCharacterAnimHandler(Hotspot &h);
	void puzzledAnimHandler(Hotspot &h);
	void roomExitAnimHandler(Hotspot &h);
	void playerAnimHandler(Hotspot &h);
	void followerAnimHandler(Hotspot &h);
	void jailorAnimHandler(Hotspot &h);
	void sonicRatAnimHandler(Hotspot &h);
	void droppingTorchAnimHandler(Hotspot &h);
	void playerSewerExitAnimHandler(Hotspot &h);
	void fireAnimHandler(Hotspot &h);
	void sparkleAnimHandler(Hotspot &h);
	void teaAnimHandler(Hotspot &h);
	void goewinCaptiveAnimHandler(Hotspot &h);
	void prisonerAnimHandler(Hotspot &h);
	void catrionaAnimHandler(Hotspot &h);
	void morkusAnimHandler(Hotspot &h);
	void talkAnimHandler(Hotspot &h);
	void grubAnimHandler(Hotspot &h);
	void barmanAnimHandler(Hotspot &h);
	void skorlAnimHandler(Hotspot &h);
	void gargoyleAnimHandler(Hotspot &h);
	void goewinShopAnimHandler(Hotspot &h);
	void skullAnimHandler(Hotspot &h);
	void dragonFireAnimHandler(Hotspot &h);
	void castleSkorlAnimHandler(Hotspot &h);
	void rackSerfAnimHandler(Hotspot &h);
	void fighterAnimHandler(Hotspot &h);
	void playerFightAnimHandler(Hotspot &h);
public:
	HotspotTickHandlers();

	HandlerMethodPtr getHandler(uint16 procIndex);
};

class WalkingActionEntry {
private:
	Direction _direction;
	int _numSteps;
public:
	WalkingActionEntry(Direction dir, int steps): _direction(dir), _numSteps(steps) {}
	Direction direction() const { return _direction; }
	int &rawSteps() { return _numSteps; }
	int numSteps() const;
};

enum PathFinderResult {PF_UNFINISHED, PF_OK, PF_DEST_OCCUPIED, PF_PART_PATH, PF_NO_WALK};

class PathFinder {
private:
	Hotspot *_hotspot;
	bool _inUse;
	typedef Common::List<Common::SharedPtr<WalkingActionEntry> > WalkingActionList;
	WalkingActionList _list;
	RoomPathsDecompressedData _layer;
	int _stepCtr;
	bool _inProgress;
	int _countdownCtr;
	int16 _destX, _destY;
	int16 _xPos, _yPos;
	int16 _xCurrent, _yCurrent;
	int16 _xDestPos, _yDestPos;
	int16 _xDestCurrent, _yDestCurrent;
	bool _destOccupied;
	bool _cellPopulated;
	uint16 *_pSrc, *_pDest;
	int _xChangeInc, _xChangeStart;
	int _yChangeInc, _yChangeStart;
	int _xCtr, _yCtr;

	void initVars();
	void processCell(uint16 *p);
	void scanLine(int numScans, int changeAmount, uint16 *&pEnd, int &v);

	void add(Direction dir, int steps) {
		_list.push_front(WalkingActionList::value_type(new WalkingActionEntry(dir, steps)));
	}
	void addBack(Direction dir, int steps) {
		_list.push_back(WalkingActionList::value_type(new WalkingActionEntry(dir, steps)));
	}
public:
	PathFinder(Hotspot *h);
	void clear();
	void reset(RoomPathsData &src);
	PathFinderResult process();
	Common::String getDebugInfo() const;

	void pop() { _list.erase(_list.begin()); }
	WalkingActionEntry &top() const { return **_list.begin(); }
	bool isEmpty() const { return _list.empty(); }
	int &stepCtr() { return _stepCtr; }

	void saveToStream(Common::WriteStream *stream) const;
	void loadFromStream(Common::ReadStream *stream);
};

enum HotspotPrecheckResult {PC_EXECUTE, PC_NOT_IN_ROOM, PC_FAILED, PC_WAIT, PC_EXCESS};

enum BarPlaceResult {BP_KEEP_TRYING, BP_GOT_THERE, BP_FAIL};

struct DestStructure {
	uint8 counter;
	Common::Point position;
};


#define MAX_NUM_FRAMES 16

class Hotspot {
private:
	HotspotTickHandlers _tickHandlers;
	HotspotData *_data;
	uint16 _animId;
	HotspotAnimData *_anim;
	HandlerMethodPtr _tickHandler;
	Surface *_frames;
	uint16 _hotspotId;
	uint16 _originalId;
	uint16 _roomNumber;
	int16 _startX, _startY;
	uint16 _height, _width;
	uint16 _heightCopy, _widthCopy;
	uint16 _yCorrection;
	uint16 _charRectY;
	int8 _talkX, _talkY;
	uint16 _numFrames;
	uint16 _frameNumber;
	Direction _direction;
	uint8 _layer;
	uint16 _hotspotScriptOffset;
	uint8 _colorOffset;
	bool _persistant;
	HotspotOverrideData *_override;
	bool _skipFlag;
	PathFinder _pathFinder;
	uint16 _frameWidth;
	bool _frameStartsUsed;
	uint16 _frameStarts[MAX_NUM_FRAMES];
	char _nameBuffer[MAX_HOTSPOT_NAME_SIZE];
	DestStructure _tempDest;

	// Runtime fields
	uint16 _frameCtr;
	uint8 _voiceCtr;
	int16 _destX, _destY;
	uint16 _destHotspotId;
	uint16 _blockedOffset;
	uint8 _exitCtr;
	bool _walkFlag;
	uint16 _startRoomNumber;
	uint16 _supportValue;

	// Support methods
	uint16 getTalkId(HotspotData *charHotspot);
	void startTalk(HotspotData *charHotspot, uint16 id);
	void startTalkDialog();

	// Action support methods
	HotspotPrecheckResult actionPrecheck(HotspotData *hotspot);
	BarPlaceResult getBarPlace();
	bool findClearBarPlace();
	bool characterWalkingCheck(uint16 id);
	void resetDirection();

	// Action set
	void doGet(HotspotData *hotspot);
	void doOperate(HotspotData *hotspot);
	void doOpen(HotspotData *hotspot);
	void doClose(HotspotData *hotspot);
	void doLockUnlock(HotspotData *hotspot);
	void doUse(HotspotData *hotspot);
	void doGive(HotspotData *hotspot);
	void doTalkTo(HotspotData *hotspot);
	void doTell(HotspotData *hotspot);
	void doLook(HotspotData *hotspot);
	void doLookAt(HotspotData *hotspot);
	void doLookThrough(HotspotData *hotspot);
	void doAsk(HotspotData *hotspot);
	void doDrink(HotspotData *hotspot);
	void doStatus(HotspotData *hotspot);
	void doGoto(HotspotData *hotspot);
	void doReturn(HotspotData *hotspot);
	void doBribe(HotspotData *hotspot);
	void doExamine(HotspotData *hotspot);
	void npcSetRoomAndBlockedOffset(HotspotData *hotspot);
	void npcHeySir(HotspotData *hotspot);
	void npcExecScript(HotspotData *hotspot);
	void npcResetPausedList(HotspotData *hotspot);
	void npcSetRandomDest(HotspotData *hotspot);
	void npcWalkingCheck(HotspotData *hotspot);
	void npcSetSupportOffset(HotspotData *hotspot);
	void npcSupportOffsetConditional(HotspotData *hotspot);
	void npcDispatchAction(HotspotData *hotspot);
	void npcTalkNpcToNpc(HotspotData *hotspot);
	void npcPause(HotspotData *hotspot);
	void npcStartTalking(HotspotData *hotspot);
	void npcJumpAddress(HotspotData *hotspot);

	// Auxillaries
	void doLookAction(HotspotData *hotspot, Action action);
public:
	Hotspot(HotspotData *res);
	Hotspot(Hotspot *character, uint16 objType);
	Hotspot();
	~Hotspot();

	void setAnimation(uint16 newAnimId);
	void setAnimationIndex(int animIndex);
	void setAnimation(HotspotAnimData *newRecord);
	uint16 hotspotId() const { return _hotspotId; }
	uint16 originalId() const { return _originalId; }
	Surface &frames() const { return *_frames; }
	HotspotAnimData &anim() const { return *_anim; }
	HotspotData *resource() const { return _data; }
	uint16 numFrames() const { return _numFrames; }
	uint16 frameNumber() const { return _frameNumber; }
	void setFrameNumber(uint16 frameNum) {
		assert(frameNum < _numFrames);
		_frameNumber = frameNum;
	}
	void incFrameNumber();
	Direction direction() const { return _direction; }
	uint16 frameWidth() const { return _width; }
	int16 x() const { return _startX; }
	int16 y() const { return _startY; }
	int16 destX() const { return _destX; }
	int16 destY() const { return _destY; }
	int8 talkX() const { return _talkX; }
	int8 talkY() const { return _talkY; }
	uint16 destHotspotId() const { return _destHotspotId; }
	uint16 blockedOffset() const { return _blockedOffset; }
	uint8 exitCtr() const { return _exitCtr; }
	bool walkFlag() const { return _walkFlag; }
	uint16 startRoomNumber() const { return _startRoomNumber; }
	uint16 width() const { return _width; }
	uint16 height() const { return _height; }
	uint16 widthCopy() const { return _widthCopy; }
	uint16 heightCopy() const { return _heightCopy; }
	uint16 yCorrection() const { return _yCorrection; }
	uint16 charRectY() const { return _charRectY; }
	uint16 roomNumber() const { return _roomNumber; }
	uint16 talkScript() const {
		assert(_data);
		return _data->talkScriptOffset;
	}
	uint16 hotspotScript() const { return _hotspotScriptOffset; }
	uint8 layer() const { return _layer; }
	bool skipFlag() const { return _skipFlag; }
	void setTickProc(uint16 newVal);
	bool persistant() const { return _persistant; }
	void setPersistant(bool value) { _persistant = value; }
	uint8 colorOffset() const { return _colorOffset; }
	void setColorOffset(uint8 value) { _colorOffset = value; }
	void setRoomNumber(uint16 roomNum) {
		_roomNumber = roomNum;
		if (_data) _data->roomNumber = roomNum;
	}
	uint16 nameId() const;
	const char *getName();
	bool isActiveAnimation();
	void setPosition(int16 newX, int16 newY);
	void setDestPosition(int16 newX, int16 newY) { _destX = newX; _destY = newY; }
	void setDestHotspot(uint16 id) { _destHotspotId = id; }
	void setExitCtr(uint8 value) { _exitCtr = value; }
	BlockedState blockedState() const {
		assert(_data);
		return _data->blockedState;
	}
	void setBlockedState(BlockedState newState) {
		assert(_data);
		_data->blockedState = newState;
	}
	bool blockedFlag() const {
		assert(_data);
		return _data->blockedFlag;
	}
	void setBlockedFlag(bool newValue) {
		assert(_data);
		_data->blockedFlag = newValue;
	}
	void setWalkFlag(bool value) { _walkFlag = value; }
	void setStartRoomNumber(uint16 value) { _startRoomNumber = value; }
	void setSize(uint16 newWidth, uint16 newHeight);
	void setWidth(uint16 newWidth) {
		_width = newWidth;
		_frameWidth = newWidth;
	}
	void setHeight(uint16 newHeight) {
		_height = newHeight;
	}
	void setHotspotScript(uint16 offset) {
		assert(_data != NULL);
		_hotspotScriptOffset = offset;
		_data->hotspotScriptOffset = offset;
	}
	void setLayer(uint8 newLayer) {
		assert(_data != NULL);
		_layer = newLayer;
		_data->layer = newLayer;
	}
	void setActions(uint32 newActions) {
		assert(_data);
		_data->actions = newActions;
	}
	void setCharRectY(uint16 value) { _charRectY = value; }
	void setSkipFlag(bool value) { _skipFlag = value; }
	CharacterMode characterMode() const {
		assert(_data != NULL);
		return _data->characterMode;
	}
	void setCharacterMode(CharacterMode value) {
		assert(_data != NULL);
		_data->characterMode = value;
	}
	uint16 delayCtr() const {
		assert(_data);
		return _data->delayCtr;
	}
	void setDelayCtr(uint16 value) {
		assert(_data);
		_data->delayCtr = value;
	}
	uint16 pauseCtr() const {
		assert(_data);
		return _data->pauseCtr;
	}
	void setPauseCtr(uint16 value) {
		assert(_data);
		_data->pauseCtr = value;
	}
	VariantBool coveredFlag() const {
		assert(_data);
		return _data->coveredFlag;
	}
	void setCoveredFlag(VariantBool value) {
		assert(_data);
		_data->coveredFlag = value;
	}
	uint16 useHotspotId() const {
		assert(_data);
		return _data->useHotspotId;
	}
	void setUseHotspotId(uint16 value) {
		assert(_data);
		_data->useHotspotId = value;
	}
	uint16 talkGate() const {
		assert(_data);
		return _data->talkGate;
	}
	void setTalkGate(uint16 value) {
		assert(_data);
		_data->talkGate = value;
	}
	uint16 supportValue() const { return _supportValue; }
	void setSupportValue(uint16 value) { _supportValue = value; }

	void copyTo(Surface *dest);
	bool executeScript();
	void tick();
	bool isRoomExit(uint16 id);

	// Walking
	void walkTo(int16 endPosX, int16 endPosY, uint16 destHotspot = 0);
	void stopWalking();
	void endAction();
	void setDirection(Direction dir);
	void faceHotspot(HotspotData *hotspot);
	void faceHotspot(uint16 hotspotId);
	void setRandomDest();
	void setOccupied(bool occupiedFlag);
	bool walkingStep();
	void updateMovement();
	void updateMovement2(CharacterMode value);
	void resetPosition();
	bool doorCloseCheck(uint16 doorId);

	void doAction();
	void doAction(Action action, HotspotData *hotspot);
	CurrentActionStack &currentActions() const {
		assert(_data);
		return _data->npcSchedule;
	}
	PathFinder &pathFinder() { return _pathFinder; }
	DestStructure &tempDest() { return _tempDest; }
	uint16 frameCtr() const { return _frameCtr; }
	void setFrameCtr(uint16 value) { _frameCtr = value; }
	void decrFrameCtr() { if (_frameCtr > 0) --_frameCtr; }
	uint8 actionCtr() const {
		assert(_data);
		return _data->actionCtr;
	}
	void setActionCtr(uint8 v) {
		assert(_data);
		_data->actionCtr = v;
	}
	uint8 voiceCtr() const { return _voiceCtr; }
	void setVoiceCtr(uint8 v) { _voiceCtr = v; }

	// Miscellaneous
	void doNothing(HotspotData *hotspot);
	void converse(uint16 destCharacterId, uint16 messageId, bool srcStandStill = false,
					   bool destStandStill = false);
	void showMessage(uint16 messageId, uint16 destCharacterId = NOONE_ID);
	void scheduleConverse(uint16 destHotspot, uint16 messageId);
	void handleTalkDialog();

	void saveToStream(Common::WriteStream *stream) const;
	void loadFromStream(Common::ReadStream *stream);
};

class HotspotList: public Common::List<Common::SharedPtr<Hotspot> > {
public:
	void saveToStream(Common::WriteStream *stream) const;
	void loadFromStream(Common::ReadStream *stream);
};

} // End of namespace Lure

#endif
