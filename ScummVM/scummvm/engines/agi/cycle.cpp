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

#include "agi/agi.h"
#include "agi/sprite.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"
#include "agi/menu.h"

namespace Agi {

/**
 * Set up new room.
 * This function is called when ego enters a new room.
 * @param n room number
 */
void AgiEngine::newRoom(int n) {
	VtEntry *v;
	int i;

	// Simulate slowww computer.
	// Many effects rely on it.
	pause(kPauseRoom);

	debugC(4, kDebugLevelMain, "*** room %d ***", n);
	_sound->stopSound();

	i = 0;
	for (v = _game.viewTable; v < &_game.viewTable[MAX_VIEWTABLE]; v++) {
		v->entry = i++;
		v->flags &= ~(fAnimated | fDrawn);
		v->flags |= fUpdate;
		v->stepTime = 1;
		v->stepTimeCount = 1;
		v->cycleTime = 1;
		v->cycleTimeCount = 1;
		v->stepSize = 1;
	}
	agiUnloadResources();

	_game.playerControl = true;
	_game.block.active = false;
	_game.horizon = 36;
	_game.vars[vPrevRoom] = _game.vars[vCurRoom];
	_game.vars[vCurRoom] = n;
	_game.vars[vBorderTouchObj] = 0;
	_game.vars[vBorderCode] = 0;
	_game.vars[vEgoViewResource] = _game.viewTable[0].currentView;

	agiLoadResource(rLOGIC, n);

	// Reposition ego in the new room
	switch (_game.vars[vBorderTouchEgo]) {
	case 1:
		_game.viewTable[0].yPos = _HEIGHT - 1;
		break;
	case 2:
		_game.viewTable[0].xPos = 0;
		break;
	case 3:
		_game.viewTable[0].yPos = HORIZON + 1;
		break;
	case 4:
		_game.viewTable[0].xPos = _WIDTH - _game.viewTable[0].xSize;
		break;
	}

	_game.vars[vBorderTouchEgo] = 0;
	setflag(fNewRoomExec, true);

	_game.exitAllLogics = true;

	writeStatus();
	writePrompt();
}

void AgiEngine::resetControllers() {
	int i;

	for (i = 0; i < MAX_DIRS; i++) {
		_game.controllerOccured[i] = false;
	}
}

void AgiEngine::interpretCycle() {
	int oldSound, oldScore;

	if (_game.playerControl)
		_game.vars[vEgoDir] = _game.viewTable[0].direction;
	else
		_game.viewTable[0].direction = _game.vars[vEgoDir];

	checkAllMotions();

	oldScore = _game.vars[vScore];
	oldSound = getflag(fSoundOn);

	_game.exitAllLogics = false;
	while (runLogic(0) == 0 && !(shouldQuit() || _restartGame)) {
		_game.vars[vWordNotFound] = 0;
		_game.vars[vBorderTouchObj] = 0;
		_game.vars[vBorderCode] = 0;
		oldScore = _game.vars[vScore];
		setflag(fEnteredCli, false);
		_game.exitAllLogics = false;
		resetControllers();
	}
	resetControllers();

	_game.viewTable[0].direction = _game.vars[vEgoDir];

	if (_game.vars[vScore] != oldScore || getflag(fSoundOn) != oldSound)
		writeStatus();

	_game.vars[vBorderTouchObj] = 0;
	_game.vars[vBorderCode] = 0;
	setflag(fNewRoomExec, false);
	setflag(fRestartGame, false);
	setflag(fRestoreJustRan, false);

	if (_game.gfxMode) {
		updateViewtable();
		_gfx->doUpdate();
	}
}

/**
 * Update AGI interpreter timer.
 */
void AgiEngine::updateTimer() {
	_clockCount++;
	if (_clockCount <= TICK_SECONDS)
		return;

	_clockCount -= TICK_SECONDS;

	if (!_game.clockEnabled)
		return;

	setvar(vSeconds, getvar(vSeconds) + 1);
	if (getvar(vSeconds) < 60)
		return;

	setvar(vSeconds, 0);
	setvar(vMinutes, getvar(vMinutes) + 1);
	if (getvar(vMinutes) < 60)
		return;

	setvar(vMinutes, 0);
	setvar(vHours, getvar(vHours) + 1);
	if (getvar(vHours) < 24)
		return;

	setvar(vHours, 0);
	setvar(vDays, getvar(vDays) + 1);
}

void AgiEngine::newInputMode(InputMode mode) {
	if (mode == INPUT_MENU && !getflag(fMenusWork) && !(getFeatures() & GF_MENUS))
		return;

	_oldMode = _game.inputMode;
	_game.inputMode = mode;
}

void AgiEngine::oldInputMode() {
	_game.inputMode = _oldMode;
}

// If main_cycle returns false, don't process more events!
int AgiEngine::mainCycle() {
	unsigned int key, kascii;
	VtEntry *v = &_game.viewTable[0];

	pollTimer();
	updateTimer();

	key = doPollKeyboard();

	// In AGI Mouse emulation mode we must update the mouse-related
	// vars in every interpreter cycle.
	//
	// We run AGIMOUSE always as a side effect
	//if (getFeatures() & GF_AGIMOUSE) {
		_game.vars[28] = _mouse.x / 2;
		_game.vars[29] = _mouse.y;
	//}

	if (key == KEY_PRIORITY) {
		_sprites->eraseBoth();
		_debug.priority = !_debug.priority;
		_picture->showPic();
		_sprites->blitBoth();
		_sprites->commitBoth();
		key = 0;
	}

	if (key == KEY_STATUSLN) {
		_debug.statusline = !_debug.statusline;
		writeStatus();
		key = 0;
	}

	// Click-to-walk mouse interface
	if (_game.playerControl && v->flags & fAdjEgoXY) {
		int toX = v->parm1;
		int toY = v->parm2;

		// AGI Mouse games use ego's sprite's bottom left corner for mouse walking target.
		// Amiga games use ego's sprite's bottom center for mouse walking target.
		// TODO: Check what Atari ST AGI and Apple IIGS AGI use for mouse walking target.
		if (getPlatform() == Common::kPlatformAmiga)
			toX -= (v->xSize / 2); // Center ego's sprite horizontally

		// Adjust ego's sprite's mouse walking target position (These parameters are
		// controlled with the adj.ego.move.to.x.y-command). Note that these values rely
		// on the horizontal centering of the ego's sprite at least on the Amiga platform.
		toX += _game.adjMouseX;
		toY += _game.adjMouseY;

		v->direction = getDirection(v->xPos, v->yPos, toX, toY, v->stepSize);

		if (v->direction == 0)
			inDestination(v);
	}

	kascii = KEY_ASCII(key);

	if (kascii)
		setvar(vKey, kascii);

	bool restartProcessKey;
	do {
		restartProcessKey = false;

		switch (_game.inputMode) {
		case INPUT_NORMAL:
			if (!handleController(key)) {
				if (key == 0 || !_game.inputEnabled)
					break;
				handleKeys(key);

				// if ESC pressed, activate menu before
				// accept.input from the interpreter cycle
				// sets the input mode to normal again
				// (closes: #540856)
				if (key == KEY_ESCAPE) {
					key = 0;
					restartProcessKey = true;
				}

				// commented out to close Sarien bug #438872
				//if (key)
				//	_game.keypress = key;
			}
			break;
		case INPUT_GETSTRING:
			handleController(key);
			handleGetstring(key);
			setvar(vKey, 0);	// clear ENTER key
			break;
		case INPUT_MENU:
			_menu->keyhandler(key);
			_gfx->doUpdate();
			return false;
		case INPUT_NONE:
			handleController(key);
			if (key)
				_game.keypress = key;
			break;
		}
	} while (restartProcessKey);
	_gfx->doUpdate();

	if (_game.msgBoxTicks > 0)
		_game.msgBoxTicks--;

	return true;
}

int AgiEngine::playGame() {
	int ec = errOK;

	debugC(2, kDebugLevelMain, "initializing...");
	debugC(2, kDebugLevelMain, "game version = 0x%x", getVersion());

	_sound->stopSound();
	_gfx->clearScreen(0);

	_game.horizon = HORIZON;
	_game.playerControl = false;

	setflag(fLogicZeroFirsttime, true);	// not in 2.917
	setflag(fNewRoomExec, true);	// needed for MUMG and SQ2!
	setflag(fSoundOn, true);	// enable sound
	setvar(vTimeDelay, 2);	// "normal" speed

	_game.gfxMode = true;
	_game.clockEnabled = true;
	_game.lineUserInput = 22;

	// We run AGIMOUSE always as a side effect
	//if (getFeatures() & GF_AGIMOUSE)
		debug(1, "Using AGI Mouse 1.0 protocol");

	if (getFeatures() & GF_AGIPAL)
		debug(1, "Running AGIPAL game");

	debug(0, "Running AGI script.\n");

	setflag(fEnteredCli, false);
	setflag(fSaidAcceptedInput, false);
	_game.vars[vWordNotFound] = 0;
	_game.vars[vKey] = 0;

	debugC(2, kDebugLevelMain, "Entering main loop");
	bool firstLoop = !getflag(fRestartGame); // Do not restore on game restart

	do {

		if (!mainCycle())
			continue;

		if (getvar(vTimeDelay) == 0 || (1 + _clockCount) % getvar(vTimeDelay) == 0) {
			if (!_game.hasPrompt && _game.inputMode == INPUT_NORMAL) {
				writePrompt();
				_game.hasPrompt = 1;
			} else if (_game.hasPrompt && _game.inputMode == INPUT_NONE) {
				writePrompt();
				_game.hasPrompt = 0;
			}

			interpretCycle();

			// Check if the user has asked to load a game from the command line
			// or the launcher
			if (firstLoop) {
				checkQuickLoad();
				firstLoop = false;
			}

			setflag(fEnteredCli, false);
			setflag(fSaidAcceptedInput, false);
			_game.vars[vWordNotFound] = 0;
			_game.vars[vKey] = 0;
		}

		if (shouldPerformAutoSave(_lastSaveTime)) {
			saveGame(getSavegameFilename(0), "Autosave");
		}

	} while (!(shouldQuit() || _restartGame));

	_sound->stopSound();

	return ec;
}

int AgiEngine::runGame() {
	int ec = errOK;

	// Execute the game
	do {
		debugC(2, kDebugLevelMain, "game loop");
		debugC(2, kDebugLevelMain, "game version = 0x%x", getVersion());

		if (agiInit() != errOK)
			break;

		if (_restartGame) {
			setflag(fRestartGame, true);
			setvar(vTimeDelay, 2);	// "normal" speed
			_restartGame = false;
		}

		// Set computer type (v20 i.e. vComputer) and sound type
		switch (getPlatform()) {
		case Common::kPlatformAtariST:
			setvar(vComputer, kAgiComputerAtariST);
			setvar(vSoundgen, kAgiSoundPC);
			break;
		case Common::kPlatformAmiga:
			if (getFeatures() & GF_OLDAMIGAV20)
				setvar(vComputer, kAgiComputerAmigaOld);
			else
				setvar(vComputer, kAgiComputerAmiga);
			setvar(vSoundgen, kAgiSoundTandy);
			break;
		case Common::kPlatformApple2GS:
			setvar(vComputer, kAgiComputerApple2GS);
			if (getFeatures() & GF_2GSOLDSOUND)
				setvar(vSoundgen, kAgiSound2GSOld);
			else
				setvar(vSoundgen, kAgiSoundTandy);
			break;
		case Common::kPlatformPC:
		default:
			setvar(vComputer, kAgiComputerPC);
			setvar(vSoundgen, kAgiSoundPC);
			break;
		}

		// Set monitor type (v26 i.e. vMonitor)
		switch (_renderMode) {
		case Common::kRenderCGA:
			setvar(vMonitor, kAgiMonitorCga);
			break;
		case Common::kRenderHercG:
		case Common::kRenderHercA:
			setvar(vMonitor, kAgiMonitorHercules);
			break;
		// Don't know if Amiga AGI games use a different value than kAgiMonitorEga
		// for vMonitor so I just use kAgiMonitorEga for them (As was done before too).
		case Common::kRenderAmiga:
		case Common::kRenderDefault:
		case Common::kRenderEGA:
		default:
			setvar(vMonitor, kAgiMonitorEga);
			break;
		}

		setvar(vFreePages, 180); // Set amount of free memory to realistic value
		setvar(vMaxInputChars, 38);
		_game.inputMode = INPUT_NONE;
		_game.inputEnabled = false;
		_game.hasPrompt = 0;

		_game.state = STATE_RUNNING;
		ec = playGame();
		_game.state = STATE_LOADED;
		agiDeinit();
	} while (_restartGame);

	delete _menu;
	_menu = NULL;

	releaseImageStack();

	return ec;
}

} // End of namespace Agi
