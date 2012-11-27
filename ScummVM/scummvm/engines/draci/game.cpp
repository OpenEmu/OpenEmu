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

#include "common/keyboard.h"
#include "common/serializer.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/util.h"

#include "draci/draci.h"
#include "draci/animation.h"
#include "draci/game.h"
#include "draci/barchive.h"
#include "draci/font.h"
#include "draci/mouse.h"
#include "draci/screen.h"
#include "draci/script.h"
#include "draci/sound.h"
#include "draci/surface.h"

namespace Draci {

static const char *const dialoguePath = "ROZH";

static double real_to_double(byte real[6]);

enum {
	kWalkingMapOverlayColor = 2,
	kWalkingShortestPathOverlayColor = 120,
	kWalkingObliquePathOverlayColor = 73
};

Game::Game(DraciEngine *vm) : _vm(vm), _walkingState(vm) {
	uint i;

	BArchive *initArchive = _vm->_initArchive;
	const BAFile *file;

	// Read in persons
	file = initArchive->getFile(5);
	Common::MemoryReadStream personData(file->_data, file->_length);

	const int personSize = sizeof(uint16) * 2 + sizeof(byte);
	uint numPersons = file->_length / personSize;
	_persons = new Person[numPersons];

	for (i = 0; i < numPersons; ++i) {
		_persons[i]._x = personData.readUint16LE();
		_persons[i]._y = personData.readUint16LE();
		_persons[i]._fontColor = personData.readByte();
	}

	// Read in dialogue offsets
	file = initArchive->getFile(4);
	Common::MemoryReadStream dialogueData(file->_data, file->_length);

	uint numDialogues = file->_length / sizeof(uint16);
	_dialogueOffsets = new uint[numDialogues];

	uint curOffset;
	for (i = 0, curOffset = 0; i < numDialogues; ++i) {
		_dialogueOffsets[i] = curOffset;
		curOffset += dialogueData.readUint16LE();
	}

	_dialogueVars = new int[curOffset];
	memset(_dialogueVars, 0, sizeof (int) * curOffset);

	// Read in game info
	file = initArchive->getFile(3);
	Common::MemoryReadStream gameData(file->_data, file->_length);

	_info._startRoom = gameData.readByte() - 1;
	_info._mapRoom = gameData.readByte() - 1;
	_info._numObjects = gameData.readUint16LE();
	_info._numItems = gameData.readUint16LE();
	_info._numVariables = gameData.readByte();
	_info._numPersons = gameData.readByte();
	_info._numDialogues = gameData.readByte();
	_info._maxItemWidth = gameData.readUint16LE();
	_info._maxItemHeight = gameData.readUint16LE();
	_info._musicLength = gameData.readUint16LE();
	_info._crc[0] = gameData.readUint16LE();
	_info._crc[1] = gameData.readUint16LE();
	_info._crc[2] = gameData.readUint16LE();
	_info._crc[3] = gameData.readUint16LE();

	_info._numDialogueBlocks = curOffset;

	// Read in variables
	file = initArchive->getFile(2);
	uint numVariables = file->_length / sizeof (int16);

	_variables = new int[numVariables];
	Common::MemoryReadStream variableData(file->_data, file->_length);

	for (i = 0; i < numVariables; ++i) {
		_variables[i] = variableData.readUint16LE();
	}

	// Read in item icon status
	file = initArchive->getFile(1);
	uint numItems = file->_length;
	_itemStatus = new byte[numItems];
	memcpy(_itemStatus, file->_data, numItems);
	_items = new GameItem[numItems];

	// Read in object status
	file = initArchive->getFile(0);
	uint numObjects = file->_length;

	_objects = new GameObject[numObjects];
	Common::MemoryReadStream objStatus(file->_data, file->_length);

	for (i = 0; i < numObjects; ++i) {
		byte tmp = objStatus.readByte();

		// Set object visibility
		_objects[i]._visible = tmp & (1 << 7);

		// Set object location
		_objects[i]._location = (~(1 << 7) & tmp) - 1;

		_objects[i]._playingAnim = -1;
		_objects[i]._absNum = i;
		// _anims have been initialized by the constructor
	}

	assert(numDialogues == _info._numDialogues);
	assert(numPersons == _info._numPersons);
	assert(numVariables == _info._numVariables);
	assert(numObjects == _info._numObjects);
	assert(numItems == _info._numItems);

	// Deallocate all cached files, because we have copied them into our own data structures.
	initArchive->clearCache();
}

void Game::start() {
	while (!gameShouldQuit()) {
		// Reset the flag allowing to run the scripts.
		_vm->_script->endCurrentProgram(false);

		enterNewRoom();

		if (_vm->_script->shouldEndProgram()) {
			// Escape pressed during the intro or map animations run in the
			// init scripts.  This flag was turned on to skip the rest of
			// those programs.  Don't call loop(), because the
			// location may have changed.
			continue;
		}

		// Call the outer loop doing all the hard job.
		loop(kOuterLoop, false);

		// Fade out the palette after leaving the location.
		fadePalette(true);

		if (!isReloaded()) {
			// We are changing location.  Run the hero's LOOK
			// program to trigger a possible cut-scene.  This is
			// the behavior of the original game player, whose
			// intention was to run the cut sequences after the
			// certain location change.
			const GameObject *dragon = getObject(kDragonObject);
			_vm->_script->run(dragon->_program, dragon->_look);
		}
	}
}

void Game::init() {
	setQuit(false);
	setExitLoop(false);
	setIsReloaded(false);
	_scheduledPalette = 0;
	_fadePhases = _fadePhase = 0;
	setEnableQuickHero(true);
	setWantQuickHero(false);
	setEnableSpeedText(true);
	setLoopStatus(kStatusGate);
	setLoopSubstatus(kOuterLoop);

	_animUnderCursor = NULL;

	_currentItem = _itemUnderCursor = NULL;
	_previousItemPosition = -1;

	_vm->_mouse->setCursorType(kHighlightedCursor);	// anything different from kNormalCursor

	_objUnderCursor = NULL;

	// Set the inventory to empty initially
	memset(_inventory, 0, kInventorySlots * sizeof(GameItem *));

	// Initialize animation for object / room titles
	_titleAnim = new Animation(_vm, kTitleText, 257, true);
	_titleAnim->addFrame(new Text("", _vm->_smallFont, kTitleColor, 0, 0, 0), NULL);
	_vm->_anims->insert(_titleAnim, false);

	// Initialize animation for speech text
	Animation *speechAnim = new Animation(_vm, kSpeechText, 257, true);
	speechAnim->addFrame(new Text("", _vm->_bigFont, kFontColor1, 0, 0, 0), NULL);
	_vm->_anims->insert(speechAnim, false);

	// Initialize inventory animation.  _iconsArchive is never flushed.
	const BAFile *f = _vm->_iconsArchive->getFile(13);
	_inventoryAnim = new Animation(_vm, kInventorySprite, 255, false);
	Sprite *inventorySprite = new Sprite(f->_data, f->_length, 0, 0, true);
	_inventoryAnim->addFrame(inventorySprite, NULL);
	_inventoryAnim->setRelative((kScreenWidth - inventorySprite->getWidth()) / 2,
	                           (kScreenHeight - inventorySprite->getHeight()) / 2);
	_vm->_anims->insert(_inventoryAnim, true);

	for (uint i = 0; i < kDialogueLines; ++i) {
		_dialogueAnims[i] = new Animation(_vm, kDialogueLinesID - i, 254, true);
		_dialogueAnims[i]->addFrame(new Text("", _vm->_smallFont, kLineInactiveColor, 0, 0, 0), NULL);

		_dialogueAnims[i]->setRelative(1,
		                      kScreenHeight - (i + 1) * _vm->_smallFont->getFontHeight());
		_vm->_anims->insert(_dialogueAnims[i], false);

		Text *text = reinterpret_cast<Text *>(_dialogueAnims[i]->getCurrentFrame());
		text->setText("");
	}

	for (uint i = 0; i < _info._numItems; ++i) {
		_items[i].load(i, _vm->_itemsArchive);
	}

	_objects[kDragonObject].load(kDragonObject, _vm->_objectsArchive);

	const GameObject *dragon = getObject(kDragonObject);
	debugC(4, kDraciLogicDebugLevel, "Running init program for the dragon object...");
	_vm->_script->run(dragon->_program, dragon->_init);

	// Add overlays for the walking map and shortest/obliqued paths.
	initWalkingOverlays();

	// Make sure we enter the right room in start().
	setRoomNum(-1);
	rememberRoomNumAsPrevious();
	scheduleEnteringRoomUsingGate(_info._startRoom, 0);
	_pushedNewRoom = _pushedNewGate = -1;
	_mouseChangeTick = kMouseDoNotSwitch;
}

void Game::handleOrdinaryLoop(int x, int y) {
	// During the normal game-play, in particular not when
	// running the init-scripts, enable interactivity.
	if (_loopSubstatus != kOuterLoop) {
		return;
	}

	if (_vm->_mouse->lButtonPressed()) {
		_vm->_mouse->lButtonSet(false);

		if (getCurrentItem()) {
			putItem(getCurrentItem(), getPreviousItemPosition());
			updateOrdinaryCursor();
		} else {
			if (_objUnderCursor) {
				_walkingState.setCallback(&_objUnderCursor->_program, _objUnderCursor->_look);

				if (_objUnderCursor->_imLook || !_currentRoom._heroOn) {
					_walkingState.callback();
				} else {
					if (_objUnderCursor->_lookDir == kDirectionLast) {
						walkHero(x, y, _objUnderCursor->_lookDir);
					} else {
						walkHero(_objUnderCursor->_lookX, _objUnderCursor->_lookY, _objUnderCursor->_lookDir);
					}
				}
			} else {
				_walkingState.setCallback(NULL, 0);
				walkHero(x, y, kDirectionLast);
			}
		}
	}

	if (_vm->_mouse->rButtonPressed()) {
		_vm->_mouse->rButtonSet(false);

		if (_objUnderCursor) {
			if (_vm->_script->testExpression(_objUnderCursor->_program, _objUnderCursor->_canUse)) {
				_walkingState.setCallback(&_objUnderCursor->_program, _objUnderCursor->_use);

				if (_objUnderCursor->_imUse || !_currentRoom._heroOn) {
					_walkingState.callback();
				} else {
					if (_objUnderCursor->_useDir == kDirectionLast) {
						walkHero(x, y, _objUnderCursor->_useDir);
					} else {
						walkHero(_objUnderCursor->_useX, _objUnderCursor->_useY, _objUnderCursor->_useDir);
					}
				}
			} else {
				_walkingState.setCallback(NULL, 0);
				walkHero(x, y, kDirectionLast);
			}
		} else {
			if (_vm->_script->testExpression(_currentRoom._program, _currentRoom._canUse)) {
				_walkingState.setCallback(&_currentRoom._program, _currentRoom._use);
				_walkingState.callback();
			} else {
				_walkingState.setCallback(NULL, 0);
				walkHero(x, y, kDirectionLast);
			}
		}
	}
}

int Game::inventoryPositionFromMouse() const {
	const int column = CLIP(scummvm_lround(
		(_vm->_mouse->getPosX() - kInventoryX + kInventoryItemWidth / 2.) /
		kInventoryItemWidth) - 1, 0L, (long) kInventoryColumns - 1);
	const int line = CLIP(scummvm_lround(
		(_vm->_mouse->getPosY() - kInventoryY + kInventoryItemHeight / 2.) /
		kInventoryItemHeight) - 1, 0L, (long) kInventoryLines - 1);
	return line * kInventoryColumns + column;
}

void Game::handleInventoryLoop() {
	if (_loopSubstatus != kOuterLoop) {
		return;
	}

	// If we are in inventory mode, all the animations except game items'
	// images will necessarily be paused so we can safely assume that any
	// animation under the cursor (a value returned by
	// AnimationManager::getTopAnimation()) will be an item animation or
	// an overlay, for which we check. Item animations have their IDs
	// calculated by offseting their itemID from the ID of the last "special"
	// animation ID. In this way, we obtain its itemID.
	if (_animUnderCursor != NULL && _animUnderCursor != _inventoryAnim && _animUnderCursor->getID() != kOverlayImage) {
		_itemUnderCursor = getItem(kInventoryItemsID - _animUnderCursor->getID());
		assert(_itemUnderCursor != NULL);
		assert(_itemUnderCursor->_anim == _animUnderCursor);
	} else {
		_itemUnderCursor = NULL;
	}

	// If the user pressed the left mouse button
	if (_vm->_mouse->lButtonPressed()) {
		_vm->_mouse->lButtonSet(false);

		// If there is an inventory item under the cursor and we aren't
		// holding any item, run its look GPL program
		if (_itemUnderCursor && !getCurrentItem()) {
			_vm->_script->runWrapper(_itemUnderCursor->_program, _itemUnderCursor->_look, true, false);
		// Otherwise, if we are holding an item, try to place it inside the
		// inventory
		} else if (getCurrentItem()) {
			putItem(getCurrentItem(), inventoryPositionFromMouse());
			updateInventoryCursor();
		}
	} else if (_vm->_mouse->rButtonPressed()) {
		_vm->_mouse->rButtonSet(false);

		// If we right-clicked outside the inventory, close it
		if (_animUnderCursor != _inventoryAnim && !_itemUnderCursor) {
			inventoryDone();

		// If there is an inventory item under our cursor
		} else if (_itemUnderCursor) {
			// Again, we have two possibilities:

			// The first is that there is no item in our hands.
			// In that case, just take the inventory item from the inventory.
			if (!getCurrentItem()) {
				setCurrentItem(_itemUnderCursor);
				setPreviousItemPosition(inventoryPositionFromMouse());
				removeItem(_itemUnderCursor);

			// The second is that there *is* an item in our hands.
			// In that case, run the canUse script for the inventory item
			// which will check if the two items are combinable and, finally,
			// run the use script for the item.
			} else {
				if (_vm->_script->testExpression(_itemUnderCursor->_program, _itemUnderCursor->_canUse)) {
					_vm->_script->runWrapper(_itemUnderCursor->_program, _itemUnderCursor->_use, true, false);
				}
			}
			updateInventoryCursor();
		}
	}
}

void Game::handleDialogueLoop() {
	if (_loopSubstatus != kInnerDuringDialogue) {
		return;
	}

	Text *text;
	for (int i = 0; i < kDialogueLines; ++i) {
		text = reinterpret_cast<Text *>(_dialogueAnims[i]->getCurrentFrame());

		if (_animUnderCursor == _dialogueAnims[i]) {
			text->setColor(kLineActiveColor);
		} else {
			text->setColor(kLineInactiveColor);
		}
	}

	if (_vm->_mouse->lButtonPressed() || _vm->_mouse->rButtonPressed()) {
		setExitLoop(true);
		_vm->_mouse->lButtonSet(false);
		_vm->_mouse->rButtonSet(false);
	}
}

void Game::fadePalette(bool fading_out) {
	const byte *startPal = NULL;
	const byte *endPal = _currentRoom._palette >= 0
		? _vm->_paletteArchive->getFile(_currentRoom._palette)->_data
		: NULL;
	if (fading_out) {
		startPal = endPal;
		endPal = NULL;
	}
	for (int i = 1; i <= kBlackFadingIterations; ++i) {
		_vm->_system->delayMillis(kBlackFadingTimeUnit);
		_vm->_screen->interpolatePalettes(startPal, endPal, 0, kNumColors, i, kBlackFadingIterations);
		_vm->_screen->copyToScreen();
	}
}

void Game::advanceAnimationsAndTestLoopExit() {
	// Fade the palette if requested
	if (_fadePhase > 0 && (_vm->_system->getMillis() - _fadeTick) >= kFadingTimeUnit) {
		_fadeTick = _vm->_system->getMillis();
		--_fadePhase;
		const byte *startPal = _currentRoom._palette >= 0 ? _vm->_paletteArchive->getFile(_currentRoom._palette)->_data : NULL;
		const byte *endPal = getScheduledPalette() >= 0 ? _vm->_paletteArchive->getFile(getScheduledPalette())->_data : NULL;
		_vm->_screen->interpolatePalettes(startPal, endPal, 0, kNumColors, _fadePhases - _fadePhase, _fadePhases);
		if (_fadePhase == 0) {
			if (_loopSubstatus == kInnerWhileFade) {
				setExitLoop(true);
			}
			// Rewrite the palette index of the current room.  This
			// is necessary when two fadings are called after each
			// other, such as in the intro.
			_currentRoom._palette = getScheduledPalette();
		}
	}

	// Handle character talking (if there is any)
	if (_loopSubstatus == kInnerWhileTalk) {
		// If the current speech text has expired or the user clicked a mouse button,
		// advance to the next line of text
		if ((getEnableSpeedText() && (_vm->_mouse->lButtonPressed() || _vm->_mouse->rButtonPressed())) ||
			(_vm->_system->getMillis() - _speechTick) >= _speechDuration) {

			setExitLoop(true);
		}
		_vm->_mouse->lButtonSet(false);
		_vm->_mouse->rButtonSet(false);
	}

	// A script has scheduled changing the room (either triggered
	// by the user clicking on something or run at the end of a
	// gate script in the intro).
	if ((_loopStatus == kStatusOrdinary || _loopStatus == kStatusGate) && (_newRoom != getRoomNum() || _newGate != _variables[0] - 1)) {
		// TODO: don't use _variables but a new named attribute
		setExitLoop(true);
	}

	// This returns true if we got a signal to quit the game
	if (gameShouldQuit()) {
		setExitLoop(true);
	}

	// Walk the hero.  The WalkingState class handles everything including
	// proper timing.
	bool walkingFinished = false;
	if (_walkingState.isActive()) {
		walkingFinished = !_walkingState.continueWalkingOrClearPath();
		// If walking has finished, the path won't be active anymore.
	}

	// Advance animations (this may also call setExitLoop(true) in the
	// callbacks) and redraw screen
	_vm->_anims->drawScene(_vm->_screen->getSurface());
	_vm->_screen->copyToScreen();
	_vm->_system->delayMillis(kTimeUnit);

	// If the hero has arrived at his destination, after even the last
	// phase was correctly animated, run the callback.
	if (walkingFinished) {
		bool exitLoop = false;
		if (_loopSubstatus == kInnerUntilExit) {
			// The callback may run another inner loop (for
			// example, a dialogue).  Reset the loop
			// substatus temporarily to the outer one.
			exitLoop = true;
			setLoopSubstatus(kOuterLoop);
		}
		debugC(2, kDraciWalkingDebugLevel, "Finished walking");
		_walkingState.callback();	// clears callback pointer first
		if (exitLoop) {
			debugC(3, kDraciWalkingDebugLevel, "Exiting from the inner loop");
			setExitLoop(true);
			setLoopSubstatus(kInnerUntilExit);
		}
	}
}

void Game::loop(LoopSubstatus substatus, bool shouldExit) {
	// Can run both as an outer and inner loop.  In both mode it updates
	// the screen according to the timer.  It the outer mode (kOuterLoop)
	// it also reacts to user events.  In the inner mode (all kInner*
	// enums), the loop runs until its stopping condition, possibly
	// stopping earlier if the user interrupts it, however no other user
	// intervention is allowed.
	assert(getLoopSubstatus() == kOuterLoop);
	setLoopSubstatus(substatus);
	setExitLoop(shouldExit);

	// Always enter the first pass of the loop, even if shouldExitLoop() is
	// true, exactly to ensure to make at least one pass.
	do {
		debugC(4, kDraciLogicDebugLevel, "loopstatus: %d, loopsubstatus: %d",
			_loopStatus, _loopSubstatus);

		_vm->handleEvents();
		if (isReloaded()) {
			// Cannot continue with the same animation objects,
			// because the real data structures of the game have
			// completely been changed.
			break;
		}

		if (_vm->_mouse->isCursorOn()) {
			// Find animation under cursor and the game object
			// corresponding to it
			int x = _vm->_mouse->getPosX();
			int y = _vm->_mouse->getPosY();
			_animUnderCursor = _vm->_anims->getTopAnimation(x, y);
			_objUnderCursor = getObjectWithAnimation(_animUnderCursor);
			debugC(5, kDraciLogicDebugLevel, "Anim under cursor: %d", _animUnderCursor ? _animUnderCursor->getID() : -1);

			switch (_loopStatus) {
			case kStatusOrdinary:
				updateOrdinaryCursor();
				updateTitle(x, y);
				handleOrdinaryLoop(x, y);
				handleStatusChangeByMouse();
				break;
			case kStatusInventory:
				updateInventoryCursor();
				updateTitle(x, y);
				handleInventoryLoop();
				handleStatusChangeByMouse();
				break;
			case kStatusDialogue:
				handleDialogueLoop();
				break;
			case kStatusGate:
				// cannot happen when isCursonOn; added for completeness
				break;
			}
		}

		advanceAnimationsAndTestLoopExit();

	} while (!shouldExitLoop());

	setLoopSubstatus(kOuterLoop);
	setExitLoop(false);
}

void Game::handleStatusChangeByMouse() {
	const int mouseY = _vm->_mouse->getPosY();
	bool wantsChange = false;
	if (_loopStatus == kStatusOrdinary) {
		if (getRoomNum() == getMapRoom()) {
			wantsChange = mouseY >= kScreenHeight - 1;
		} else {
			wantsChange = mouseY == 0 || mouseY >= kScreenHeight - 1;
		}
	} else if (_loopStatus == kStatusInventory) {
		wantsChange = _animUnderCursor != _inventoryAnim && !_itemUnderCursor && mouseY != 0;
	}

	if (!wantsChange) {
		// Turn off the timer, but enable switching.
		_mouseChangeTick = kMouseEnableSwitching;

	// Otherwise the mouse signalizes that the mode should be changed.
	} else if (_mouseChangeTick == kMouseEnableSwitching) {
		// If the timer is currently disabled, this is the first time
		// when the mouse left the region.  Start counting.
		_mouseChangeTick = _vm->_system->getMillis();
	} else if (_mouseChangeTick == kMouseDoNotSwitch) {
		// Do nothing.  This exception is good when the status has just
		// changed.  Even if the mouse starts in the outside region
		// (e.g., due to flipping the change by a key or due to
		// flipping back being triggered by the same hot area), the
		// timeout won't kick in until it moves into the inside region
		// for the first time.
	} else if (_vm->_system->getMillis() - _mouseChangeTick >= kStatusChangeTimeout) {
		if (_loopStatus == kStatusOrdinary) {
			if (getRoomNum() == getMapRoom()) {
				scheduleEnteringRoomUsingGate(getPreviousRoomNum(), 0);
			} else if (mouseY >= kScreenHeight - 1) {
				scheduleEnteringRoomUsingGate(getMapRoom(), 0);
			} else if (mouseY == 0) {
				inventoryInit();
			}
		} else {
			inventoryDone();
		}
	}

	// We don't implement the original game player's main menu that pops up
	// when the mouse gets to the bottom of the screen.  It contains icons
	// for displaying the map, loading/saving the game, quiting the game,
	// and displaying the credits.  The essential options are implemented
	// in ScummVM's main menu, I don't wanna implement the credits, and so
	// I allocate the whole bottom line for switching to/from the map.
}

void Game::updateOrdinaryCursor() {
	// Fetch mouse coordinates
	bool mouseChanged = false;

	// If there is no game object under the cursor, try using the room itself
	if (!_objUnderCursor) {
		if (_vm->_script->testExpression(_currentRoom._program, _currentRoom._canUse)) {
			if (!getCurrentItem()) {
				_vm->_mouse->setCursorType(kHighlightedCursor);
			} else {
				_vm->_mouse->loadItemCursor(getCurrentItem(), true);
			}
			mouseChanged = true;
		}
	// If there *is* a game object under the cursor, update the cursor image
	} else {
		// If there is no walking direction set on the object (i.e. the object
		// is not a gate / exit), test whether it can be used and, if so,
		// update the cursor image (highlight it).
		if (_objUnderCursor->_walkDir == 0) {
			if (_vm->_script->testExpression(_objUnderCursor->_program, _objUnderCursor->_canUse)) {
				if (!getCurrentItem()) {
					_vm->_mouse->setCursorType(kHighlightedCursor);
				} else {
					_vm->_mouse->loadItemCursor(getCurrentItem(), true);
				}
				mouseChanged = true;
			}
		// If the walking direction *is* set, the game object is a gate, so update
		// the cursor image to the appropriate arrow.
		} else {
			_vm->_mouse->setCursorType((CursorType)_objUnderCursor->_walkDir);
			mouseChanged = true;
		}
	}
	// Load the appropriate cursor (item image if an item is held or ordinary cursor
	// if not)
	if (!mouseChanged) {
		if (!getCurrentItem()) {
			_vm->_mouse->setCursorType(kNormalCursor);
		} else {
			_vm->_mouse->loadItemCursor(getCurrentItem(), false);
		}
	}
}

void Game::updateInventoryCursor() {
	// Fetch mouse coordinates
	bool mouseChanged = false;

	if (_itemUnderCursor) {
		if (_vm->_script->testExpression(_itemUnderCursor->_program, _itemUnderCursor->_canUse)) {
			if (!getCurrentItem()) {
				_vm->_mouse->setCursorType(kHighlightedCursor);
			} else {
				_vm->_mouse->loadItemCursor(getCurrentItem(), true);
			}
			mouseChanged = true;
		}
	}
	if (!mouseChanged) {
		if (!getCurrentItem()) {
			_vm->_mouse->setCursorType(kNormalCursor);
		} else {
			_vm->_mouse->loadItemCursor(getCurrentItem(), false);
		}
	}
}

void Game::updateTitle(int x, int y) {
	// Fetch current surface and height of the small font (used for titles)
	Surface *surface = _vm->_screen->getSurface();
	const int smallFontHeight = _vm->_smallFont->getFontHeight();

	// Fetch the dedicated objects' title animation / current frame
	Text *title = reinterpret_cast<Text *>(_titleAnim->getCurrentFrame());

	// Mark dirty rectangle to delete the previous text
	_titleAnim->markDirtyRect(surface);

	if (_loopStatus == kStatusInventory) {
		// If there is no item under the cursor, delete the title.
		// Otherwise, show the item's title.
		title->setText(_itemUnderCursor ? _itemUnderCursor->_title : "");
	} else {
		// If there is no object under the cursor, delete the title.
		// Otherwise, show the object's title.
		title->setText(_objUnderCursor ? _objUnderCursor->_title : "");
	}

	// Move the title to the correct place (just above the cursor)
	int newX = surface->centerOnX(x, title->getWidth());
	int newY = surface->putAboveY(y - smallFontHeight / 2, title->getHeight());
	_titleAnim->setRelative(newX, newY);

	// If we are currently playing the title, mark it dirty so it gets updated.
	// Otherwise, start playing the title animation.
	if (_titleAnim->isPlaying()) {
		_titleAnim->markDirtyRect(surface);
	} else {
		_titleAnim->play();
	}
}

const GameObject *Game::getObjectWithAnimation(const Animation *anim) const {
	for (uint i = 0; i < _info._numObjects; ++i) {
		GameObject *obj = &_objects[i];
		if (obj->_playingAnim >= 0 && obj->_anim[obj->_playingAnim] == anim) {
			return obj;
		}
	}

	return NULL;
}

void Game::removeItem(GameItem *item) {
	if (!item)
		return;
	for (uint i = 0; i < kInventorySlots; ++i) {
		if (_inventory[i] == item) {
			_inventory[i] = NULL;
			item->_anim->stop();
			break;
		}
	}
}

void Game::loadItemAnimation(GameItem *item) {
	if (item->_anim)
		return;
	item->_anim = new Animation(_vm, kInventoryItemsID - item->_absNum, 256, false);
	_vm->_anims->insert(item->_anim, false);
	// _itemImagesArchive is never flushed.
	const BAFile *img = _vm->_itemImagesArchive->getFile(2 * item->_absNum);
	item->_anim->addFrame(new Sprite(img->_data, img->_length, 0, 0, true), NULL);
}

void Game::putItem(GameItem *item, int position) {
	// Empty our hands
	setCurrentItem(NULL);

	if (!item)
		return;
	assert(position >= 0);

	for (int i = 0; i < kInventorySlots; ++i) {
		int pos = (position + i) % kInventorySlots;
		if (!_inventory[pos] || _inventory[pos] == item) {
			_inventory[pos] = item;
			position = pos;
			break;
		}
	}
	setPreviousItemPosition(position);

	const int line = position / kInventoryColumns + 1;
	const int column = position % kInventoryColumns + 1;

	loadItemAnimation(item);
	Animation *anim = item->_anim;
	Drawable *frame = anim->getCurrentFrame();

	const int x = kInventoryX +
	              (column * kInventoryItemWidth) -
	              (kInventoryItemWidth / 2) -
	              (frame->getWidth() / 2);

	const int y = kInventoryY +
	              (line * kInventoryItemHeight) -
	              (kInventoryItemHeight / 2) -
	              (frame->getHeight() / 2);

	debug(2, "itemID: %d position: %d line: %d column: %d x: %d y: %d", item->_absNum, position, line, column, x, y);

	anim->setRelative(x, y);

	// If we are in inventory mode, we need to play the item animation, immediately
	// upon returning it to its slot but *not* in other modes because it should be
	// invisible then (along with the inventory)
	if (_loopStatus == kStatusInventory && _loopSubstatus == kOuterLoop) {
		anim->play();
	}
}

void Game::inventoryInit() {
	// Pause all "background" animations
	_vm->_anims->pauseAnimations();
	if (_walkingState.isActive()) {
		walkHero(_hero.x, _hero.y, kDirectionLast);
	}

	// Draw the inventory and the current items
	inventoryDraw();

	// Turn cursor on if it is off
	_vm->_mouse->cursorOn();

	// Set the appropriate loop status
	setLoopStatus(kStatusInventory);

	// Don't return from the inventory mode immediately if the mouse is out.
	_mouseChangeTick = kMouseDoNotSwitch;
}

void Game::inventoryDone() {
	_vm->_mouse->cursorOn();
	setLoopStatus(kStatusOrdinary);

	_vm->_anims->unpauseAnimations();

	_inventoryAnim->stop();

	for (uint i = 0; i < kInventorySlots; ++i) {
		if (_inventory[i]) {
			_inventory[i]->_anim->stop();
		}
	}

	// Reset item under cursor
	_itemUnderCursor = NULL;

	// Don't start the inventory mode again if the mouse is on the top.
	_mouseChangeTick = kMouseDoNotSwitch;
}

void Game::inventoryDraw() {
	_inventoryAnim->play();

	for (uint i = 0; i < kInventorySlots; ++i) {
		if (_inventory[i]) {
			_inventory[i]->_anim->play();
		}
	}
}

void Game::inventoryReload() {
	// Make sure all items are loaded into memory (e.g., after loading a
	// savegame) by re-putting them on the same spot in the inventory.
	for (uint i = 0; i < kInventorySlots; ++i) {
		putItem(_inventory[i], i);
	}
	setPreviousItemPosition(0);
}

void Game::inventorySwitch(int keycode) {
	switch (keycode) {
	case Common::KEYCODE_SLASH:
		// Switch between holding an item and the ordinary mouse cursor.
		if (!getCurrentItem()) {
			if (getPreviousItemPosition() >= 0) {
				GameItem* last_item = _inventory[getPreviousItemPosition()];
				setCurrentItem(last_item);
				removeItem(last_item);
			}
		} else {
			putItem(getCurrentItem(), getPreviousItemPosition());
		}
		break;
	case Common::KEYCODE_COMMA:
	case Common::KEYCODE_PERIOD:
		// Iterate between the items in the inventory.
		if (getCurrentItem()) {
			assert(getPreviousItemPosition() >= 0);
			int direction = keycode == Common::KEYCODE_PERIOD ? +1 : -1;
			// Find the next available item.
			int pos = getPreviousItemPosition() + direction;
			while (true) {
			      if (pos < 0)
				      pos += kInventorySlots;
			      else if (pos >= kInventorySlots)
				      pos -= kInventorySlots;
			      if (pos == getPreviousItemPosition() || _inventory[pos]) {
				      break;
			      }
			      pos += direction;
			}
			// Swap it with the current item.
			putItem(getCurrentItem(), getPreviousItemPosition());
			GameItem* new_item = _inventory[pos];
			setCurrentItem(new_item);
			setPreviousItemPosition(pos);
			removeItem(new_item);
		}
		break;
	}
	if (getLoopStatus() == kStatusOrdinary) {
		updateOrdinaryCursor();
	} else {
		updateInventoryCursor();
	}
}

void Game::dialogueMenu(int dialogueID) {
	int oldLines, hit;

	Common::String name;
	name = dialoguePath + Common::String::format("%d.dfw", dialogueID + 1);
	_dialogueArchive = new BArchive(name);

	debugC(4, kDraciLogicDebugLevel, "Starting dialogue (ID: %d, Archive: %s)",
	    dialogueID, name.c_str());

	_currentDialogue = dialogueID;
	oldLines = 255;
	dialogueInit(dialogueID);

	do {
		_dialogueExit = false;
		hit = dialogueDraw();

		debugC(7, kDraciLogicDebugLevel,
			"hit: %d, _lines[hit]: %d, lastblock: %d, dialogueLines: %d, dialogueExit: %d",
			hit, _lines[hit], _lastBlock, _dialogueLinesNum, _dialogueExit);

		if ((!_dialogueExit) && (hit != -1) && (_lines[hit] != -1)) {
			if ((oldLines == 1) && (_dialogueLinesNum == 1) && (_lines[hit] == _lastBlock)) {
				break;
			}
			_currentBlock = _lines[hit];

			// Run the dialogue program
			_vm->_script->runWrapper(_dialogueBlocks[_lines[hit]]._program, 1, false, true);
		} else {
			break;
		}
		_lastBlock = _lines[hit];
		_dialogueVars[_dialogueOffsets[dialogueID] + _lastBlock] += 1;
		_dialogueBegin = false;
		oldLines = _dialogueLinesNum;

	} while (!_dialogueExit);

	dialogueDone();
	_currentDialogue = -1;
}

int Game::dialogueDraw() {
	_dialogueLinesNum = 0;
	int i = 0;
	int ret = 0;

	Animation *anim;
	Text *dialogueLine;

	while ((_dialogueLinesNum < 4) && (i < _blockNum)) {
		GPL2Program blockTest;
		blockTest._bytecode = _dialogueBlocks[i]._canBlock;
		blockTest._length = _dialogueBlocks[i]._canLen;
		debugC(3, kDraciLogicDebugLevel, "Testing dialogue block %d", i);
		if (_vm->_script->testExpression(blockTest, 1)) {
			anim = _dialogueAnims[_dialogueLinesNum];
			dialogueLine = reinterpret_cast<Text *>(anim->getCurrentFrame());
			dialogueLine->setText(_dialogueBlocks[i]._title);

			dialogueLine->setColor(kLineInactiveColor);
			_lines[_dialogueLinesNum] = i;
			_dialogueLinesNum++;
		}
		++i;
	}

	for (i = _dialogueLinesNum; i < kDialogueLines; ++i) {
		_lines[i] = -1;
		anim = _dialogueAnims[i];
		dialogueLine = reinterpret_cast<Text *>(anim->getCurrentFrame());
		dialogueLine->setText("");
	}

	if (_dialogueLinesNum > 1) {
		// Call the game loop to enable interactivity until the user
		// selects his choice.  _animUnderCursor will be set.
		_vm->_mouse->cursorOn();
		loop(kInnerDuringDialogue, false);
		_vm->_mouse->cursorOff();

		bool notDialogueAnim = true;
		for (uint j = 0; j < kDialogueLines; ++j) {
			if (_dialogueAnims[j] == _animUnderCursor) {
				notDialogueAnim = false;
				break;
			}
		}

		if (notDialogueAnim) {
			ret = -1;
		} else {
			ret = kDialogueLinesID - _animUnderCursor->getID();
		}
	} else {
		ret = _dialogueLinesNum - 1;
	}

	for (i = 0; i < kDialogueLines; ++i) {
		dialogueLine = reinterpret_cast<Text *>(_dialogueAnims[i]->getCurrentFrame());
		_dialogueAnims[i]->markDirtyRect(_vm->_screen->getSurface());
		dialogueLine->setText("");
	}

	return ret;
}

void Game::dialogueInit(int dialogID) {
	_vm->_mouse->setCursorType(kDialogueCursor);

	_blockNum = _dialogueArchive->size() / 3;
	_dialogueBlocks = new Dialogue[_blockNum];

	const BAFile *f;

	for (uint i = 0; i < kDialogueLines; ++i) {
		_lines[i] = 0;
	}

	for (int i = 0; i < _blockNum; ++i) {
		f = _dialogueArchive->getFile(i * 3);
		_dialogueBlocks[i]._canLen = f->_length;
		_dialogueBlocks[i]._canBlock = f->_data;

		f = _dialogueArchive->getFile(i * 3 + 1);

		// The first byte of the file is the length of the string (without the length)
		assert(f->_length - 1 == f->_data[0]);

		_dialogueBlocks[i]._title = Common::String((char *)(f->_data+1), f->_length-1);

		f = _dialogueArchive->getFile(i * 3 + 2);
		_dialogueBlocks[i]._program._bytecode = f->_data;
		_dialogueBlocks[i]._program._length = f->_length;
	}

	for (uint i = 0; i < kDialogueLines; ++i) {
		_dialogueAnims[i]->play();
	}

	setLoopStatus(kStatusDialogue);
	_lastBlock = -1;
	_dialogueBegin = true;
}

void Game::dialogueDone() {
	for (uint i = 0; i < kDialogueLines; ++i) {
		_dialogueAnims[i]->stop();
	}

	delete _dialogueArchive;
	delete[] _dialogueBlocks;

	setLoopStatus(kStatusOrdinary);
	_vm->_mouse->setCursorType(kNormalCursor);
}

int Game::playHeroAnimation(int anim_index) {
	GameObject *dragon = getObject(kDragonObject);
	const int current_anim_index = dragon->_playingAnim;
	Animation *anim = dragon->_anim[anim_index];

	if (anim_index == current_anim_index) {
		anim->markDirtyRect(_vm->_screen->getSurface());
	} else {
		dragon->stopAnim();
	}
	positionAnimAsHero(anim);
	if (anim_index == current_anim_index) {
		anim->markDirtyRect(_vm->_screen->getSurface());
	} else {
		dragon->playAnim(anim_index);
	}

	return anim->currentFrameNum();
}

void Game::redrawWalkingPath(Animation *anim, byte color, const WalkingPath &path) {
	Sprite *ov = _walkingMap.newOverlayFromPath(path, color);
	delete anim->getFrame(0);
	anim->replaceFrame(0, ov, NULL);
	anim->markDirtyRect(_vm->_screen->getSurface());
}

void Game::setHeroPosition(const Common::Point &p) {
	debugC(3, kDraciWalkingDebugLevel, "Jump to x: %d y: %d", p.x, p.y);
	_hero = p;
}

void Game::walkHero(int x, int y, SightDirection dir) {
	if (!_currentRoom._heroOn) {
		// Nothing to do.  Happens for example in the map.
		return;
	}

	// Find the closest walkable point.
	Common::Point target = findNearestWalkable(x, y);
	if (target.x < 0 || target.y < 0) {
		debug(1, "The is no walkable point on the map");
		return;
	}

	// Compute the shortest and obliqued path.
	WalkingPath shortestPath, obliquePath;
	if (!_walkingMap.findShortestPath(_hero, target, &shortestPath)) {
		debug(1, "Unreachable point [%d,%d]", target.x, target.y);
		return;
	}
	_walkingMap.obliquePath(shortestPath, &obliquePath);
	debugC(2, kDraciWalkingDebugLevel, "Walking path lengths: shortest=%d oblique=%d", shortestPath.size(), obliquePath.size());
	if (_vm->_showWalkingMap) {
		redrawWalkingPath(_walkingShortestPathOverlay, kWalkingShortestPathOverlayColor, shortestPath);
		redrawWalkingPath(_walkingObliquePathOverlay, kWalkingObliquePathOverlayColor, obliquePath);
	}

	// Start walking.  Walking will be gradually advanced by
	// advanceAnimationsAndTestLoopExit(), which also handles calling the
	// callback and stopping the walk at the end.  If the hero is already
	// walking at this point, this command will cancel the previous path
	// and replace it by the current one (the callback has already been
	// reset by our caller).
	_walkingState.startWalking(_hero, target, Common::Point(x, y), dir,
		_walkingMap.getDelta(), obliquePath);
}

void Game::initWalkingOverlays() {
	_walkingMapOverlay = new Animation(_vm, kWalkingMapOverlay, 256, _vm->_showWalkingMap);
	_walkingMapOverlay->addFrame(NULL, NULL);	// rewritten below by loadWalkingMap()
	_vm->_anims->insert(_walkingMapOverlay, true);

	_walkingShortestPathOverlay = new Animation(_vm, kWalkingShortestPathOverlay, 257, _vm->_showWalkingMap);
	_walkingObliquePathOverlay = new Animation(_vm, kWalkingObliquePathOverlay, 258, _vm->_showWalkingMap);
	WalkingPath emptyPath;
	_walkingShortestPathOverlay->addFrame(_walkingMap.newOverlayFromPath(emptyPath, 0), NULL);
	_walkingObliquePathOverlay->addFrame(_walkingMap.newOverlayFromPath(emptyPath, 0), NULL);
	_vm->_anims->insert(_walkingShortestPathOverlay, true);
	_vm->_anims->insert(_walkingObliquePathOverlay, true);
}

void Game::loadRoomObjects() {
	// Load the room's objects
	for (uint i = 0; i < _info._numObjects; ++i) {
		debugC(7, kDraciLogicDebugLevel,
			"Checking if object %d (%d) is at the current location (%d)", i,
			_objects[i]._location, getRoomNum());

		if (_objects[i]._location == getRoomNum()) {
			debugC(6, kDraciLogicDebugLevel, "Loading object %d from room %d", i, getRoomNum());
			_objects[i].load(i, _vm->_objectsArchive);
		}
	}

	// Run the init scripts for room objects
	// We can't do this in the above loop because some objects' scripts reference
	// other objects that may not yet be loaded
	for (uint i = 0; i < _info._numObjects; ++i) {
		if (_objects[i]._location == getRoomNum()) {
			const GameObject *obj = getObject(i);
			debugC(6, kDraciLogicDebugLevel,
				"Running init program for object %d (offset %d)", i, obj->_init);
			_vm->_script->run(obj->_program, obj->_init);
		}
	}

	// Run the init part of the GPL program
	debugC(4, kDraciLogicDebugLevel, "Running room init program...");
	_vm->_script->run(_currentRoom._program, _currentRoom._init);
}

void Game::loadWalkingMap(int mapID) {
	const BAFile *f;
	f = _vm->_walkingMapsArchive->getFile(mapID);
	_walkingMap.load(f->_data, f->_length);

	Sprite *ov = _walkingMap.newOverlayFromMap(kWalkingMapOverlayColor);
	delete _walkingMapOverlay->getFrame(0);
	_walkingMapOverlay->replaceFrame(0, ov, NULL);
	_walkingMapOverlay->markDirtyRect(_vm->_screen->getSurface());
}

void Game::switchWalkingAnimations(bool enabled) {
	if (enabled) {
		_walkingMapOverlay->play();
		_walkingShortestPathOverlay->play();
		_walkingObliquePathOverlay->play();
	} else {
		_walkingMapOverlay->stop();
		_walkingShortestPathOverlay->stop();
		_walkingObliquePathOverlay->stop();
	}
}

void Game::loadOverlays() {
	uint x, y, z, num;

	const BAFile *overlayHeader;

	overlayHeader = _vm->_roomsArchive->getFile(getRoomNum() * 4 + 2);
	Common::MemoryReadStream overlayReader(overlayHeader->_data, overlayHeader->_length);

	for (int i = 0; i < _currentRoom._numOverlays; i++) {
		num = overlayReader.readUint16LE() - 1;
		x = overlayReader.readUint16LE();
		y = overlayReader.readUint16LE();
		z = overlayReader.readByte();

		// _overlaysArchive is flushed when entering a room and this
		// code is called after the flushing has been done.
		const BAFile *overlayFile;
		overlayFile = _vm->_overlaysArchive->getFile(num);
		Sprite *sp = new Sprite(overlayFile->_data, overlayFile->_length, x, y, true);

		Animation *anim = new Animation(_vm, kOverlayImage, z, true);
		anim->addFrame(sp, NULL);
		// Since this is an overlay, we don't need it to be deleted
		// when the GPL Release command is invoked
		_vm->_anims->insert(anim, false);
	}

	_vm->_screen->getSurface()->markDirty();
}

void Game::deleteObjectAnimations() {
	// Deallocate all animations, because their sound samples will not
	// survive clearing the sound sample cache when changing the location.
	// It's OK to unload them even if they are still in the inventory,
	// because we only need their icons which survive.
	// Start from 1, because 0==kDragonObject.
	for (uint i = 1; i < _info._numObjects; ++i) {
		GameObject *obj = &_objects[i];
		obj->deleteAnims();
	}

	// WORKAROUND
	//
	// An absolutely horrible hack follows.  The current memory management
	// is completely broken and it needs to be seriously hacked to work at
	// all.  The problem is caching sound samples in BArchive and clearing
	// all caches when entering a new location.  The animation sequences
	// store pointers to samples owned by the BArchive cache, which gets
	// invalidated during the location change.  If an animation sequence
	// survives location change and refers to any sound sample, we get
	// SIGSEGV when next played, because this sound sample will have been
	// deallocated by the cache although still referred to by the animation.
	//
	// Caveat: when I tried to perform a deep copy and make the animation
	// object own the sound samples and deallocate them when the animation
	// has been deleted, I get an almost immediate SIGSEGV in runWrapper()
	// on many objects, because often an animation graphically ends and is
	// stopped, but the last sound sample still plays for a short while
	// afterwards.  If the sound sample is cached, it's OK, but if it's
	// deallocated with releaseAnims==true, we get SIGSEGV in the sound
	// mixer.  This problem doesn't occur when changing locations, because
	// there we first explicitly stop all playing sounds.
	//
	// The loop above deallocates all animations corresponding to non-hero
	// objects.  Hero is special, because the first ~20 animations are
	// standard and loaded for the whole game (standing, walking, etc.).
	// They are loaded by the GPL2 init routine for object hero.  Luckily
	// the animations don't refer to any sound samples.  The remaining
	// animations thus must be deallocated manually, otherwise they won't
	// be re-loaded next time assuming that they are already correctly
	// loaded.
	//
	// Why this only occurred for sound samples and not sprites?  This bug is
	// concealed by a complete coincidence, that all sprites are stored
	// column-wise and our class Sprite detects this and creates a local
	// copy.  If this wasn't the case, each animation (not just with sound
	// samples) would fail and preserving the ~20 hero animations wouldn't
	// work either.
	//
	// TODO: completely rewrite the resource management.  maybe implement
	// usage counters?  maybe completely ignore the GPL2 hints and manage
	// memory completely on my own?

	// We don't want to deallocate the first ~20 resident dragon
	// animations, because they are loaded exactly once in dragon's init
	// script and we rely upon their existence.
	GameObject *dragon = &_objects[kDragonObject];
	dragon->deleteAnimsFrom(kFirstTemporaryAnimation);
	if (dragon->_playingAnim < 0) {
		// For the hero, we always need to have exactly 1 playing
		// animation, otherwise we index an array with -1.
		dragon->_playingAnim = 0;
	}
}

void Game::enterNewRoom() {
	debugC(1, kDraciLogicDebugLevel, "Entering room %d using gate %d", _newRoom, _newGate);
	_vm->_mouse->cursorOff();

	// Make sure all sounds are stopped before we deallocate their memory
	// by clearing the cache.  We don't have to wait for sounds to end,
	// because the timeout for voice is set exactly according to the length
	// of the sound.  If the loop ends earlier, e.g. per user's request, we
	// do wanna end the sounds immediately.
	_vm->_sound->stopAll();

	// Clear archives
	_vm->_roomsArchive->clearCache();
	_vm->_spritesArchive->clearCache();
	_vm->_paletteArchive->clearCache();
	_vm->_animationsArchive->clearCache();
	_vm->_walkingMapsArchive->clearCache();
	_vm->_soundsArchive->clearCache();
	_vm->_dubbingArchive->clearCache();
	_vm->_overlaysArchive->clearCache();

	_vm->_screen->clearScreen();

	_vm->_anims->deleteOverlays();

	GameObject *dragon = getObject(kDragonObject);
	dragon->stopAnim();

	// Remember the previous room for returning back from the map.
	rememberRoomNumAsPrevious();
	deleteObjectAnimations();

	// Before setting these variables we have to convert the values to 1-based indexing
	// because this is how everything is stored in the data files
	_variables[0] = _newGate + 1;
	_variables[1] = _newRoom + 1;

	// If the new room is the map room, set the appropriate coordinates
	// for the dragon in the persons array
	if (_newRoom == _info._mapRoom) {
		_persons[kDragonObject]._x = 160;
	  	_persons[kDragonObject]._y = 0;
	}

	// Set the appropriate loop status before loading the room
	setLoopStatus(kStatusGate);
	setIsReloaded(false);

	// Make sure the possible walking path from the previous room is
	// cleaned up.  Some rooms (e.g., the map) don't support walking.
	_walkingState.stopWalking();

	// Stop a possible palette fading.
	_fadePhases = _fadePhase = 0;

	_currentRoom.load(_newRoom, _vm->_roomsArchive);
	loadWalkingMap(getMapID());
	loadRoomObjects();
	loadOverlays();

	// Draw the scene with the black palette and slowly fade into the right palette.
	_vm->_screen->setPalette(NULL, 0, kNumColors);
	_vm->_anims->drawScene(_vm->_screen->getSurface());
	_vm->_screen->copyToScreen();
	fadePalette(false);

	// Run the program for the gate the dragon came through
	debugC(6, kDraciLogicDebugLevel, "Running program for gate %d", _newGate);
	_vm->_script->runWrapper(_currentRoom._program, _currentRoom._gates[_newGate], true, true);

	// Reset the loop status.
	setLoopStatus(kStatusOrdinary);
	setExitLoop(false);

	// Don't immediately switch to the map or inventory even if the mouse
	// position tell us to.
	_mouseChangeTick = kMouseDoNotSwitch;

	// Set cursor state
	// Need to do this after we set the palette since the cursors use it
	if (_currentRoom._mouseOn) {
		debugC(6, kDraciLogicDebugLevel, "Mouse: ON");
		_vm->_mouse->cursorOn();
		_vm->_mouse->setCursorType(kNormalCursor);
	} else {
		debugC(6, kDraciLogicDebugLevel, "Mouse: OFF");
		_vm->_mouse->cursorOff();
	}
}

void Game::positionAnimAsHero(Animation *anim) {
	// Calculate scaling factors
	const double scale = getPers0() + getPersStep() * _hero.y;

	// Set the Z coordinate for the dragon's animation
	anim->setZ(_hero.y);

	// Fetch current frame
	Drawable *frame = anim->getCurrentFrame();

	// We naturally want the dragon to position its feet to the location of the
	// click but sprites are drawn from their top-left corner so we subtract
	// the current height of the dragon's sprite
	Common::Point p = _hero;
	p.x -= scummvm_lround(scale * frame->getWidth() / 2);
	p.y -= scummvm_lround(scale * frame->getHeight());

	// Since _persons[] is used for placing talking text, we use the non-adjusted x value
	// so the text remains centered over the dragon.
	_persons[kDragonObject]._x = _hero.x;
	_persons[kDragonObject]._y = p.y;

	if (anim->isRelative()) {
		// Set the per-animation scaling factor and relative position
		anim->setScaleFactors(scale, scale);
		anim->setRelative(p.x, p.y);

		// Clear the animation's shift so that the real sprite stays at place
		// regardless of what the current phase is.  If the animation starts
		// from the beginning, the shift is already [0,0], but if it is in the
		// middle, it may be different.
		anim->clearShift();

		// Otherwise this dragon animation is used at exactly one place
		// in the game (such as jumping into the secret entrance),
		// which can is recognized by it using absolute coordinates.
		// Bypass our animation positioning system, otherwise there two
		// shifts will get summed and the animation will be placed
		// outside the screen.
	}
}

void Game::positionHeroAsAnim(Animation *anim) {
	// Check out where the hero has moved to by composing the relative
	// shifts of the sprites.
	_hero = anim->getCurrentFramePosition();

	// Update our hero coordinates (don't forget that our control point is
	// elsewhere).  This is formula is the exact inverse of the formula
	// used in positionAnimAsHero() and even rounding errors are exactly
	// the same.
	Drawable *frame = anim->getCurrentFrame();
	_hero.x += scummvm_lround(anim->getScaleX() * frame->getWidth() / 2);
	_hero.y += scummvm_lround(anim->getScaleY() * frame->getHeight());
}

void Game::pushNewRoom() {
	_pushedNewRoom = _newRoom;
	_pushedNewGate = _newGate;
}

void Game::popNewRoom() {
	if (_loopStatus != kStatusInventory && _pushedNewRoom >= 0) {
		scheduleEnteringRoomUsingGate(_pushedNewRoom, _pushedNewGate);
		_pushedNewRoom = _pushedNewGate = -1;
	}
}

void Game::setSpeechTiming(uint tick, uint duration) {
	_speechTick = tick;
	_speechDuration = duration;
}

void Game::shiftSpeechAndFadeTick(int delta) {
	_speechTick += delta;
	_fadeTick += delta;
}

void Game::initializeFading(int phases) {
	_fadePhases = _fadePhase = phases;
	_fadeTick = _vm->_system->getMillis();
}

void Game::deleteAnimationsAfterIndex(int lastAnimIndex) {
	// Delete all animations loaded after the marked one
	// (from objects and from the AnimationManager)
	for (uint i = 0; i < getNumObjects(); ++i) {
		GameObject *obj = &_objects[i];

		for (int j = obj->_anim.size() - 1; j >= 0; --j) {
			Animation *anim = obj->_anim[j];
			if (anim->getIndex() > lastAnimIndex) {
				obj->_anim.remove_at(j);
				if (j == obj->_playingAnim) {
					obj->_playingAnim = -1;
				}
			}
		}
	}

	_vm->_anims->deleteAfterIndex(lastAnimIndex);
}

Game::~Game() {
	delete[] _persons;
	delete[] _variables;
	delete[] _dialogueOffsets;
	delete[] _dialogueVars;
	delete[] _objects;
	delete[] _itemStatus;
	delete[] _items;
}

void Game::DoSync(Common::Serializer &s) {
	s.syncAsUint16LE(_currentRoom._roomNum);

	for (uint i = 0; i < _info._numObjects; ++i) {
		GameObject& obj = _objects[i];
		s.syncAsSint16LE(obj._location);
		s.syncAsByte(obj._visible);
	}

	for (uint i = 0; i < _info._numItems; ++i) {
		s.syncAsByte(_itemStatus[i]);
	}

	for (int i = 0; i < kInventorySlots; ++i) {
		if (s.isSaving()) {
			int itemID = _inventory[i] ? _inventory[i]->_absNum : -1;
			s.syncAsSint16LE(itemID);
		} else {
			int itemID = -1;
			s.syncAsSint16LE(itemID);
			_inventory[i] = getItem(itemID);
		}
	}

	for (int i = 0; i < _info._numVariables; ++i) {
		s.syncAsSint16LE(_variables[i]);
	}
	for (uint i = 0; i < _info._numDialogueBlocks; ++i) {
		s.syncAsSint16LE(_dialogueVars[i]);
	}

}

static double real_to_double(byte real[6]) {
	// Extract sign bit
	int sign = real[0] & (1 << 7);

	// Extract exponent and adjust for bias
	int exp = real[5] - 129;

	double mantissa;
	double tmp = 0.0;

	if (real[5] == 0) {
		mantissa = 0.0;
	} else {
		// Process the first four least significant bytes
		for (int i = 4; i >= 1; --i) {
			tmp += real[i];
			tmp /= 1 << 8;
		}

		// Process the most significant byte (remove the sign bit)
		tmp += real[0] & ((1 << 7) - 1);
		tmp /= 1 << 8;

		// Calculate mantissa
		mantissa = 1.0;
		mantissa += 2.0 * tmp;
	}

	// Flip sign if necessary
	if (sign) {
		mantissa = -mantissa;
	}

	// Calculate final value
	return ldexp(mantissa, exp);
}

int GameObject::getAnim(int animID) const {
	for (uint i = 0; i < _anim.size(); ++i) {
		if (_anim[i]->getID() == animID) {
			return i;
		}
	}
	return -1;
}

int GameObject::addAnim(Animation *anim) {
	anim->setZ(_z);
	_anim.push_back(anim);
	int index = _anim.size() - 1;
	if (_absNum == kDragonObject && index <= kLastTurning) {
		// Index to _anim is the Movement type.  All walking and
		// turning movements can be accelerated.
		anim->supportsQuickAnimation(true);
	}
	return index;
}

void GameObject::playAnim(int i) {
	_anim[i]->play();
	_playingAnim = i;
}

void GameObject::stopAnim() {
	if (_playingAnim >= 0) {
		_anim[_playingAnim]->stop();
		_playingAnim = -1;
	}
}

void GameObject::deleteAnims() {
	deleteAnimsFrom(0);
}

void GameObject::deleteAnimsFrom(int index) {
	for (int j = _anim.size() - 1; j >= index; --j) {
		_anim.back()->del();
		_anim.pop_back();
	}
	if (_playingAnim >= index) {
		_playingAnim = -1;
	}
}

void GameObject::load(uint objNum, BArchive *archive) {
	const BAFile *file;

	file = archive->getFile(objNum * 3);
	Common::MemoryReadStream objReader(file->_data, file->_length);

	_init = objReader.readUint16LE();
	_look = objReader.readUint16LE();
	_use = objReader.readUint16LE();
	_canUse = objReader.readUint16LE();
	_imInit = objReader.readByte();
	_imLook = objReader.readByte();
	_imUse = objReader.readByte();
	_walkDir = objReader.readByte() - 1;
	_z = objReader.readByte();
	objReader.readUint16LE(); // idxSeq field, not used
	objReader.readUint16LE(); // numSeq field, not used
	_lookX = objReader.readUint16LE();
	_lookY = objReader.readUint16LE();
	_useX = objReader.readUint16LE();
	_useY = objReader.readUint16LE();
	_lookDir = static_cast<SightDirection> (objReader.readByte());
	_useDir = static_cast<SightDirection> (objReader.readByte());

	_absNum = objNum;

	file = archive->getFile(objNum * 3 + 1);

	// The first byte of the file is the length of the string (without the length)
	assert(file->_length - 1 == file->_data[0]);

	_title = Common::String((char *)(file->_data+1), file->_length-1);

	file = archive->getFile(objNum * 3 + 2);
	_program._bytecode = file->_data;
	_program._length = file->_length;

	_playingAnim = -1;
	deleteAnims();		// If the object has already been loaded, then discard the previous animations
}

void GameItem::load(int itemID, BArchive *archive) {
	const BAFile *f = archive->getFile(itemID * 3);
	Common::MemoryReadStream itemReader(f->_data, f->_length);

	_init = itemReader.readSint16LE();
	_look = itemReader.readSint16LE();
	_use = itemReader.readSint16LE();
	_canUse = itemReader.readSint16LE();
	_imInit = itemReader.readByte();
	_imLook = itemReader.readByte();
	_imUse = itemReader.readByte();

	_absNum = itemID;

	f = archive->getFile(itemID * 3 + 1);

	// The first byte is the length of the string
	_title = Common::String((const char *)f->_data + 1, f->_length - 1);
	assert(f->_data[0] == _title.size());

	f = archive->getFile(itemID * 3 + 2);

	_program._bytecode = f->_data;
	_program._length = f->_length;

	_anim = NULL;
}

void Room::load(int roomNum, BArchive *archive) {
	const BAFile *f;
	f = archive->getFile(roomNum * 4);
	Common::MemoryReadStream roomReader(f->_data, f->_length);

	roomReader.readUint32LE(); // Pointer to room program, not used
	roomReader.readUint16LE(); // Program length, not used
	roomReader.readUint32LE(); // Pointer to room title, not used

	// Set the current room to the new value
	_roomNum = roomNum;

	// Music will be played by the GPL2 command startMusic when needed.
	_music = roomReader.readByte();
	_mapID = roomReader.readByte() - 1;
	_palette = roomReader.readByte() - 1;
	_numOverlays = roomReader.readSint16LE();
	_init = roomReader.readSint16LE();
	_look = roomReader.readSint16LE();
	_use = roomReader.readSint16LE();
	_canUse = roomReader.readSint16LE();
	_imInit = roomReader.readByte();
	_imLook = roomReader.readByte();
	_imUse = roomReader.readByte();
	_mouseOn = roomReader.readByte();
	_heroOn = roomReader.readByte();

	// Read in pers0 and persStep (stored as 6-byte Pascal reals)
	byte real[6];

	for (int i = 5; i >= 0; --i) {
		real[i] = roomReader.readByte();
	}

	_pers0 = real_to_double(real);

	for (int i = 5; i >= 0; --i) {
		real[i] = roomReader.readByte();
	}

	_persStep = real_to_double(real);

	_escRoom = roomReader.readByte() - 1;
	_numGates = roomReader.readByte();

	debugC(4, kDraciLogicDebugLevel, "Music: %d", _music);
	debugC(4, kDraciLogicDebugLevel, "Map: %d", _mapID);
	debugC(4, kDraciLogicDebugLevel, "Palette: %d", _palette);
	debugC(4, kDraciLogicDebugLevel, "Overlays: %d", _numOverlays);
	debugC(4, kDraciLogicDebugLevel, "Init: %d", _init);
	debugC(4, kDraciLogicDebugLevel, "Look: %d", _look);
	debugC(4, kDraciLogicDebugLevel, "Use: %d", _use);
	debugC(4, kDraciLogicDebugLevel, "CanUse: %d", _canUse);
	debugC(4, kDraciLogicDebugLevel, "ImInit: %d", _imInit);
	debugC(4, kDraciLogicDebugLevel, "ImLook: %d", _imLook);
	debugC(4, kDraciLogicDebugLevel, "ImUse: %d", _imUse);
	debugC(4, kDraciLogicDebugLevel, "MouseOn: %d", _mouseOn);
	debugC(4, kDraciLogicDebugLevel, "HeroOn: %d", _heroOn);
	debugC(4, kDraciLogicDebugLevel, "Pers0: %f", _pers0);
	debugC(4, kDraciLogicDebugLevel, "PersStep: %f", _persStep);
	debugC(4, kDraciLogicDebugLevel, "EscRoom: %d", _escRoom);
	debugC(4, kDraciLogicDebugLevel, "Gates: %d", _numGates);

	// Read in the gates' numbers
	_gates.clear();
	for (uint i = 0; i < _numGates; ++i) {
		_gates.push_back(roomReader.readSint16LE());
	}

	// Load the room's GPL program
	f = archive->getFile(roomNum * 4 + 3);
	_program._bytecode = f->_data;
	_program._length = f->_length;
}

} // End of namespace Draci
