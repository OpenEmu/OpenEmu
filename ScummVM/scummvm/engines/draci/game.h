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

#ifndef DRACI_GAME_H
#define DRACI_GAME_H

#include "common/str.h"
#include "draci/script.h"
#include "draci/walking.h"

namespace Common {
class Serializer;
}

namespace Draci {

class BArchive;
class DraciEngine;

enum {
	kDragonObject = 0
};

enum {
	kDialogueLines = 4
};

enum {
	kBlackPalette = -1
};

enum {
	kMouseEnableSwitching = -1,
	kMouseDoNotSwitch = -2
};

// Constants tuned such that with ScummVM's default talkspeed kStandardSpeed, the speed
// computed by equation (kBaseSpeechDuration + kSpeechTimeUnit * #characters) /
// talkspeed is equal to the original game.
enum SpeechConstants {
	kBaseSpeechDuration = 12000,
	kSpeechTimeUnit = 2640,
	kStandardSpeed = 60
};

enum FadeConstants {
	// One fading phase called from the game scripts is 50ms.
	kFadingTimeUnit = 50,
	// Fading in/out when entering/leaving a location takes 15 iterations of (at least) 7ms each.
	kBlackFadingIterations = 15,
	kBlackFadingTimeUnit = 7
};

enum AnimationConstants {
	kTimeUnit = 20
};

/** Inventory related magical constants */
enum InventoryConstants {
  kInventoryItemWidth = 25,
  kInventoryItemHeight = 25,
  kInventoryColumns = 7,
  kInventoryLines = 5,
  kInventoryX = 70, ///< Used for positioning of the inventory sprite on the X axis
  kInventoryY = 30, ///< Used for positioning of the inventory sprite on the Y axis
  kInventorySlots = kInventoryLines * kInventoryColumns,
  kStatusChangeTimeout = 500
};

class GameObject {
public:
	int _absNum;
	uint _init, _look, _use, _canUse;
	bool _imInit, _imLook, _imUse;
	int _walkDir;
	byte _z;
	uint _lookX, _lookY, _useX, _useY;
	SightDirection _lookDir, _useDir;
	GPL2Program _program;
	Common::String _title;
	int _location;
	bool _visible;

	Common::Array<Animation *> _anim;
	int _playingAnim;

	int getAnim(int animID) const;
	int addAnim(Animation *anim);
	int playingAnim() const { return _playingAnim; }
	void playAnim(int i);
	void stopAnim();
	void deleteAnims();
	void deleteAnimsFrom(int index);
	void load(uint objNum, BArchive *archive);
};

struct GameInfo {
	int _startRoom;
	int _mapRoom;
	uint _numObjects;
	uint _numItems;
	byte _numVariables;
	byte _numPersons;
	byte _numDialogues;
	uint _maxItemWidth, _maxItemHeight;
	uint _musicLength;
	uint _crc[4];
	uint _numDialogueBlocks;
};

class GameItem {
public:
	int _absNum;
	uint _init, _look, _use, _canUse;
	bool _imInit, _imLook, _imUse;
	GPL2Program _program;
	Common::String _title;

	Animation *_anim;

	void load(int itemID, BArchive *archive);
};

struct Person {
	uint _x, _y;
	byte _fontColor;
};

struct Dialogue {
	int _canLen;
	byte *_canBlock;
	Common::String _title;
	GPL2Program _program;
};

class Room {
public:
	int _roomNum;
	byte _music;
	int _mapID;
	int _palette;
	int _numOverlays;
	int _init, _look, _use, _canUse;
	bool _imInit, _imLook, _imUse;
	bool _mouseOn, _heroOn;
	double _pers0, _persStep;
	int _escRoom;
	byte _numGates;
	Common::Array<int> _gates;
	GPL2Program _program;

	void load(int roomNum, BArchive *archive);
};

enum LoopStatus {
	kStatusOrdinary,	// normal game-play: everything allowed
	kStatusGate,		// during running init-scripts when entering a room: disable interactivity
	kStatusInventory,	// inventory is open: cannot change the room or go to map
	kStatusDialogue		// during a dialogue: cannot change the room, go to inventory
};

enum LoopSubstatus {
	kOuterLoop,		// outer loop: everything is allowed
	kInnerWhileTalk,	// playing a voice: inner loop will exit afterwards
	kInnerWhileFade,	// fading a palette: inner loop will exit when done
	kInnerDuringDialogue,	// selecting continuation block: inner block will exit afterwards
	kInnerUntilExit		// other inner loop: either immediately exiting or waiting for an animation to end (whose callback ends the loop)
};

class Game {
public:
	Game(DraciEngine *vm);
	~Game();

	void init();
	void start();
	void loop(LoopSubstatus substatus, bool shouldExit);

	// HACK: this is only for testing
	int nextRoomNum() const {
		int n = _currentRoom._roomNum;
		n = n < 37 ? n+1 : n;
		return n;
	}

	// HACK: same as above
	int prevRoomNum() const {
		int n = _currentRoom._roomNum;
		n = n > 0 ? n-1 : n;
		return n;
	}

	Common::Point findNearestWalkable(int x, int y) const { return _walkingMap.findNearestWalkable(x, y); }
	void heroAnimationFinished() { _walkingState.heroAnimationFinished(); }
	void stopWalking() { _walkingState.stopWalking(); }	// and clear callback
	void walkHero(int x, int y, SightDirection dir);	// start walking and leave callback as is
	void setHeroPosition(const Common::Point &p);
	const Common::Point &getHeroPosition() const { return _hero; }
	void positionAnimAsHero(Animation *anim);
	void positionHeroAsAnim(Animation *anim);

	// Makes sure animation anim_index plays on the hero.  If the hero's
	// position has changed, it updates the animation position.  If the new
	// animation is different, it stops the old one and starts the new one,
	// otherwise it just marks dirty rectangles for moving the position.
	// Returns the current animation phase of the new animation (usually 0
	// unless the animation hasn't changed).
	int playHeroAnimation(int anim_index);

	void loadOverlays();
	void loadWalkingMap(int mapID);		// but leaves _currentRoom._mapID untouched
	void switchWalkingAnimations(bool enabled);

	uint getNumObjects() const { return _info._numObjects; }
	GameObject *getObject(uint objNum) { return _objects + objNum; }
	const GameObject *getObjectWithAnimation(const Animation *anim) const;
	void deleteObjectAnimations();
	void deleteAnimationsAfterIndex(int lastAnimIndex);

	int getVariable(int varNum) const { return _variables[varNum]; }
	void setVariable(int varNum, int value) { _variables[varNum] = value; }

	const Person *getPerson(int personID) const { return &_persons[personID]; }

	int getRoomNum() const { return _currentRoom._roomNum; }
	void setRoomNum(int num) { _currentRoom._roomNum = num; }
	int getPreviousRoomNum() const { return _previousRoom; }
	void rememberRoomNumAsPrevious() { _previousRoom = getRoomNum(); }
	void scheduleEnteringRoomUsingGate(int room, int gate) { _newRoom = room; _newGate = gate; }
	void pushNewRoom();
	void popNewRoom();

	double getPers0() const { return _currentRoom._pers0; }
	double getPersStep() const { return _currentRoom._persStep; }
	int getMusicTrack() const { return _currentRoom._music; }
	void setMusicTrack(int num) { _currentRoom._music = num; }

	int getItemStatus(int itemID) const { return _itemStatus[itemID]; }
	void setItemStatus(int itemID, int status) { _itemStatus[itemID] = status; }
	GameItem *getItem(int id) { return id >= 0 && id < (int) _info._numItems ? &_items[id] : NULL; }
	GameItem *getCurrentItem() const { return _currentItem; }
	void setCurrentItem(GameItem *item) { _currentItem = item; }
	int getPreviousItemPosition() const { return _previousItemPosition; }
	void setPreviousItemPosition(int pos) { _previousItemPosition = pos; }
	void removeItem(GameItem *item);
	void loadItemAnimation(GameItem *item);
	void putItem(GameItem *item, int position);
	void addItem(int itemID);

	int getEscRoom() const { return _currentRoom._escRoom; }
	int getMapRoom() const { return _info._mapRoom; }
	int getMapID() const { return _currentRoom._mapID; }

	/**
	 * The GPL command Mark sets the animation index (which specifies the
	 * order in which animations were loaded in) which is then used by the
	 * Release command to delete all animations that have an index greater
	 * than the one marked.
	 */
	int getMarkedAnimationIndex() const { return _markedAnimationIndex; }
	void setMarkedAnimationIndex(int index) { _markedAnimationIndex = index; }

	void setLoopStatus(LoopStatus status) { _loopStatus = status; }
	void setLoopSubstatus(LoopSubstatus status) { _loopSubstatus = status; }
	LoopStatus getLoopStatus() const { return _loopStatus; }
	LoopSubstatus getLoopSubstatus() const { return _loopSubstatus; }

	bool gameShouldQuit() const { return _shouldQuit; }
	void setQuit(bool quit) { _shouldQuit = quit; }
	bool shouldExitLoop() const { return _shouldExitLoop; }
	void setExitLoop(bool exit) { _shouldExitLoop = exit; }
	bool isReloaded() const { return _isReloaded; }
	void setIsReloaded(bool value) { _isReloaded = value; }

	void setSpeechTiming(uint tick, uint duration);
	void shiftSpeechAndFadeTick(int delta);

	void inventoryInit();
	void inventoryDraw();
	void inventoryDone();
	void inventoryReload();
	void inventorySwitch(int keycode);

	void dialogueMenu(int dialogueID);
	int dialogueDraw();
	void dialogueInit(int dialogID);
	void dialogueDone();

	bool isDialogueBegin() const { return _dialogueBegin; }
	bool shouldExitDialogue() const { return _dialogueExit; }
	void setDialogueExit(bool exit) { _dialogueExit = exit; }
	int getDialogueBlockNum() const { return _blockNum; }
	int getDialogueVar(int dialogueID) const { return _dialogueVars[dialogueID]; }
	void setDialogueVar(int dialogueID, int value) { _dialogueVars[dialogueID] = value; }
	int getCurrentDialogue() const { return _currentDialogue; }
	int getDialogueCurrentBlock() const { return _currentBlock; }
	int getDialogueLastBlock() const { return _lastBlock; }
	int getDialogueLinesNum() const { return _dialogueLinesNum; }
	int getCurrentDialogueOffset() const { return _dialogueOffsets[_currentDialogue]; }

	void schedulePalette(int paletteID) { _scheduledPalette = paletteID; }
	int getScheduledPalette() const { return _scheduledPalette; }
	void initializeFading(int phases);
	void setEnableQuickHero(bool value) { _enableQuickHero = value; }
	bool getEnableQuickHero() const { return _enableQuickHero; }
	void setWantQuickHero(bool value) { _wantQuickHero = value; }
	bool getWantQuickHero() const { return _wantQuickHero; }
	void setEnableSpeedText(bool value) { _enableSpeedText = value; }
	bool getEnableSpeedText() const { return _enableSpeedText; }

	void DoSync(Common::Serializer &s);

private:
	void updateOrdinaryCursor();
	void updateInventoryCursor();
	int inventoryPositionFromMouse() const;
	void handleOrdinaryLoop(int x, int y);
	void handleInventoryLoop();
	void handleDialogueLoop();
	void updateTitle(int x, int y);
	void updateCursor();
	void fadePalette(bool fading_out);
	void advanceAnimationsAndTestLoopExit();
	void handleStatusChangeByMouse();

	void enterNewRoom();
	void initWalkingOverlays();
	void loadRoomObjects();
	void redrawWalkingPath(Animation *anim, byte color, const WalkingPath &path);

	DraciEngine *_vm;

	GameInfo _info;

	Common::Point _hero;

	int *_variables;
	Person *_persons;
	GameObject *_objects;

	byte *_itemStatus;
	GameItem *_items;
	GameItem *_currentItem;
	GameItem *_itemUnderCursor;

	// Last position in the inventory of the item currently in the hands, resp. of the item that
	// was last in our hands.
	int _previousItemPosition;

	GameItem *_inventory[kInventorySlots];

	Room _currentRoom;
	int _newRoom;
	int _newGate;
	int _previousRoom;
	int _pushedNewRoom;	// used in GPL programs
	int _pushedNewGate;

	uint *_dialogueOffsets;
	int _currentDialogue;
	int *_dialogueVars;
	BArchive *_dialogueArchive;
	Dialogue *_dialogueBlocks;
	bool _dialogueBegin;
	bool _dialogueExit;
	int _currentBlock;
	int _lastBlock;
	int _dialogueLinesNum;
	int _blockNum;
	int _lines[kDialogueLines];
	Animation *_dialogueAnims[kDialogueLines];

	LoopStatus _loopStatus;
	LoopSubstatus _loopSubstatus;

	bool _shouldQuit;
	bool _shouldExitLoop;
	bool _isReloaded;

	uint _speechTick;
	uint _speechDuration;

	const GameObject *_objUnderCursor;
	const Animation *_animUnderCursor;

	int _markedAnimationIndex; ///< Used by the Mark GPL command

	int _scheduledPalette;
	int _fadePhases;
	int _fadePhase;
	uint _fadeTick;
	int _mouseChangeTick;

	bool _enableQuickHero;
	bool _wantQuickHero;
	bool _enableSpeedText;

	WalkingMap _walkingMap;
	WalkingState _walkingState;

	Animation *_titleAnim;
	Animation *_inventoryAnim;
	Animation *_walkingMapOverlay;
	Animation *_walkingShortestPathOverlay;
	Animation *_walkingObliquePathOverlay;
};

} // End of namespace Draci

#endif // DRACI_GAME_H
