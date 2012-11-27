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

#include "base/version.h"

#include "agi/agi.h"
#include "agi/sprite.h"
#include "agi/graphics.h"
#include "agi/opcodes.h"
#include "agi/menu.h"

#include "common/random.h"
#include "common/textconsole.h"

namespace Agi {

#define p0	(p[0])
#define p1	(p[1])
#define p2	(p[2])
#define p3	(p[3])
#define p4	(p[4])
#define p5	(p[5])
#define p6	(p[6])

#define code state->_curLogic->data
#define ip	state->_curLogic->cIP
#define vt	state->viewTable[p0]
#define vt_v state->viewTable[state->vars[p0]]

#define _v state->vars

#define getGameID() state->_vm->getGameID()
#define getFeatures() state->_vm->getFeatures()
#define getVersion() state->_vm->getVersion()
#define getLanguage() state->_vm->getLanguage()
#define setflag(a,b) state->_vm->setflag(a,b)
#define getflag(a) state->_vm->getflag(a)

void cmdIncrement(AgiGame *state, uint8 *p) {
	if (getVersion() < 0x2000) {
		if (_v[p0] < 0xf0)
			++_v[p0];
	} else {
		if (_v[p0] != 0xff)
			++_v[p0];
	}
}

void cmdDecrement(AgiGame *state, uint8 *p) {
	if (_v[p0] != 0)
		--_v[p0];
}

void cmdAssignN(AgiGame *state, uint8 *p) {
	_v[p0] = p1;

	// WORKAROUND for a bug in fan game "Get outta SQ"
	// Total number of points is stored in variable 7, which
	// is then incorrectly assigned to 0. Thus, when the game
	// is restarted, "Points 0 of 0" is shown. We set the
	// variable to the correct value here
	// Fixes bug #1942476 - "AGI: Fan(Get Outta SQ) - Score
	// is lost on restart"
	if (getGameID() == GID_GETOUTTASQ && p0 == 7)
		_v[p0] = 8;
}

void cmdAddN(AgiGame *state, uint8 *p) {
	_v[p0] += p1;
}

void cmdSubN(AgiGame *state, uint8 *p) {
	_v[p0] -= p1;
}

void cmdAssignV(AgiGame *state, uint8 *p) {
	_v[p0] = _v[p1];
}

void cmdAddV(AgiGame *state, uint8 *p) {
	_v[p0] += _v[p1];
}

void cmdSubV(AgiGame *state, uint8 *p) {
	_v[p0] -= _v[p1];
}

void cmdMulN(AgiGame *state, uint8 *p) {
	_v[p0] *= p1;
}

void cmdMulV(AgiGame *state, uint8 *p) {
	_v[p0] *= _v[p1];
}

void cmdDivN(AgiGame *state, uint8 *p) {
	_v[p0] /= p1;
}

void cmdDivV(AgiGame *state, uint8 *p) {
	_v[p0] /= _v[p1];
}

void cmdRandomV1(AgiGame *state, uint8 *p) {
	_v[p0] = state->_vm->_rnd->getRandomNumber(250);
}

void cmdRandom(AgiGame *state, uint8 *p) {
	_v[p2] = state->_vm->_rnd->getRandomNumber(p1 - p0) + p0;
}

void cmdLindirectN(AgiGame *state, uint8 *p) {
	_v[_v[p0]] = p1;
}

void cmdLindirectV(AgiGame *state, uint8 *p) {
	_v[_v[p0]] = _v[p1];
}

void cmdRindirect(AgiGame *state, uint8 *p) {
	_v[p0] = _v[_v[p1]];
}

void cmdSet(AgiGame *state, uint8 *p) {
	setflag(*p, true);
}

void cmdReset(AgiGame *state, uint8 *p) {
	setflag(*p, false);
}

void cmdToggle(AgiGame *state, uint8 *p) {
	setflag(*p, !getflag(*p));
}

void cmdSetV(AgiGame *state, uint8 *p) {
	if (getVersion() < 0x2000) {
		_v[p0] = 1;
	} else {
		setflag(_v[p0], true);
	}
}

void cmdResetV(AgiGame *state, uint8 *p) {
	if (getVersion() < 0x2000) {
		_v[p0] = 0;
	} else {
		setflag(_v[p0], false);
	}
}

void cmdToggleV(AgiGame *state, uint8 *p) {
	if (getVersion() < 0x2000) {
		_v[p0] ^= 1;
	} else {
		setflag(_v[p0], !getflag(_v[p0]));
	}
}

void cmdNewRoom(AgiGame *state, uint8 *p) {
	state->_vm->newRoom(p0);

	// WORKAROUND: Works around intro skipping bug (#1737343) in Gold Rush.
	// Intro was skipped because the enter-keypress finalizing the entering
	// of the copy protection string (Copy protection is in logic.128) was
	// left over to the intro scene (Starts with room 73 i.e. logic.073).
	// The intro scene checks for any keys pressed and if it finds any it
	// jumps to the game's start (Room 1 i.e. logic.001). We clear the
	// keyboard buffer when the intro sequence's first room (Room 73) is
	// loaded so that no keys from the copy protection scene can be left
	// over to cause the intro to skip to the game's start.
	if (getGameID() == GID_GOLDRUSH && p0 == 73)
		state->keypress = 0;
}

void cmdNewRoomF(AgiGame *state, uint8 *p) {
	state->_vm->newRoom(_v[p0]);
}

void cmdLoadView(AgiGame *state, uint8 *p) {
	state->_vm->agiLoadResource(rVIEW, p0);
}

void cmdLoadLogic(AgiGame *state, uint8 *p) {
	state->_vm->agiLoadResource(rLOGIC, p0);
}

void cmdLoadSound(AgiGame *state, uint8 *p) {
	state->_vm->agiLoadResource(rSOUND, p0);
}

void cmdLoadViewF(AgiGame *state, uint8 *p) {
	state->_vm->agiLoadResource(rVIEW, _v[p0]);
}

void cmdLoadLogicF(AgiGame *state, uint8 *p) {
	state->_vm->agiLoadResource(rLOGIC, _v[p0]);
}

void cmdDiscardView(AgiGame *state, uint8 *p) {
	state->_vm->agiUnloadResource(rVIEW, p0);
}

void cmdObjectOnAnything(AgiGame *state, uint8 *p) {
	vt.flags &= ~(fOnWater | fOnLand);
}

void cmdObjectOnLand(AgiGame *state, uint8 *p) {
	vt.flags |= fOnLand;
}

void cmdObjectOnWater(AgiGame *state, uint8 *p) {
	vt.flags |= fOnWater;
}

void cmdObserveHorizon(AgiGame *state, uint8 *p) {
	vt.flags &= ~fIgnoreHorizon;
}

void cmdIgnoreHorizon(AgiGame *state, uint8 *p) {
	vt.flags |= fIgnoreHorizon;
}

void cmdObserveObjs(AgiGame *state, uint8 *p) {
	vt.flags &= ~fIgnoreObjects;
}

void cmdIgnoreObjs(AgiGame *state, uint8 *p) {
	vt.flags |= fIgnoreObjects;
}

void cmdObserveBlocks(AgiGame *state, uint8 *p) {
	vt.flags &= ~fIgnoreBlocks;
}

void cmdIgnoreBlocks(AgiGame *state, uint8 *p) {
	vt.flags |= fIgnoreBlocks;
}

void cmdSetHorizon(AgiGame *state, uint8 *p) {
	state->horizon = p0;
}

void cmdGetPriority(AgiGame *state, uint8 *p) {
	_v[p1] = vt.priority;
}

void cmdSetPriority(AgiGame *state, uint8 *p) {
	vt.flags |= fFixedPriority;
	vt.priority = p1;

	// WORKAROUND: this fixes bug #1712585 in KQ4 (dwarf sprite priority)
	// For this scene, ego (Rosella) hasn't got a fixed priority till script 54
	// explicitly sets priority 8 for her, so that she can walk back to the table
	// without being drawn over the other dwarfs
	// It seems that in this scene, ego's priority is set to 8, but the priority of
	// the last dwarf with the soup bowls (view 152) is also set to 8, which causes
	// the dwarf to be drawn behind ego
	// With this workaround, when the game scripts set the priority of view 152
	// (seventh dwarf with soup bowls), ego's priority is set to 7
	// The game script itself sets priotity 8 for ego before she starts walking,
	// and then releases the fixed priority set on ego after ego is seated
	// Therefore, this workaround only affects that specific part of this scene
	// Ego is set to object 19 by script 54
	if (getGameID() == GID_KQ4 && vt.currentView == 152) {
		state->viewTable[19].flags |= fFixedPriority;
		state->viewTable[19].priority = 7;
	}
}

void cmdSetPriorityF(AgiGame *state, uint8 *p) {
	vt.flags |= fFixedPriority;
	vt.priority = _v[p1];
}

void cmdReleasePriority(AgiGame *state, uint8 *p) {
	vt.flags &= ~fFixedPriority;
}

void cmdSetUpperLeft(AgiGame *state, uint8 *p) {				// do nothing (AGI 2.917)
}

void cmdStartUpdate(AgiGame *state, uint8 *p) {
	state->_vm->startUpdate(&vt);
}

void cmdStopUpdate(AgiGame *state, uint8 *p) {
	state->_vm->stopUpdate(&vt);
}

void cmdCurrentView(AgiGame *state, uint8 *p) {
	_v[p1] = vt.currentView;
}

void cmdCurrentCel(AgiGame *state, uint8 *p) {
	_v[p1] = vt.currentCel;
	debugC(4, kDebugLevelScripts, "v%d=%d", p1, _v[p1]);
}

void cmdCurrentLoop(AgiGame *state, uint8 *p) {
	_v[p1] = vt.currentLoop;
}

void cmdLastCel(AgiGame *state, uint8 *p) {
	_v[p1] = vt.loopData->numCels - 1;
}

void cmdSetCel(AgiGame *state, uint8 *p) {
	state->_vm->setCel(&vt, p1);

	if (getVersion() >= 0x2000) {
		vt.flags &= ~fDontupdate;
	}
}

void cmdSetCelF(AgiGame *state, uint8 *p) {
	state->_vm->setCel(&vt, _v[p1]);
	vt.flags &= ~fDontupdate;
}

void cmdSetView(AgiGame *state, uint8 *p) {
	state->_vm->setView(&vt, p1);
}

void cmdSetViewF(AgiGame *state, uint8 *p) {
	state->_vm->setView(&vt, _v[p1]);
}

void cmdSetLoop(AgiGame *state, uint8 *p) {
	state->_vm->setLoop(&vt, p1);
}

void cmdSetLoopF(AgiGame *state, uint8 *p) {
	state->_vm->setLoop(&vt, _v[p1]);
}

void cmdNumberOfLoops(AgiGame *state, uint8 *p) {
	_v[p1] = vt.numLoops;
}

void cmdFixLoop(AgiGame *state, uint8 *p) {
	vt.flags |= fFixLoop;
}

void cmdReleaseLoop(AgiGame *state, uint8 *p) {
	vt.flags &= ~fFixLoop;
}

void cmdStepSize(AgiGame *state, uint8 *p) {
	vt.stepSize = _v[p1];
}

void cmdStepTime(AgiGame *state, uint8 *p) {
	vt.stepTime = vt.stepTimeCount = _v[p1];
}

void cmdCycleTime(AgiGame *state, uint8 *p) {
	vt.cycleTime = vt.cycleTimeCount = _v[p1];
}

void cmdStopCycling(AgiGame *state, uint8 *p) {
	vt.flags &= ~fCycling;
}

void cmdStartCycling(AgiGame *state, uint8 *p) {
	vt.flags |= fCycling;
}

void cmdNormalCycle(AgiGame *state, uint8 *p) {
	vt.cycle = kCycleNormal;
	vt.flags |= fCycling;
}

void cmdReverseCycle(AgiGame *state, uint8 *p) {
	vt.cycle = kCycleReverse;
	vt.flags |= fCycling;
}

void cmdSetDir(AgiGame *state, uint8 *p) {
	vt.direction = _v[p1];
}

void cmdGetDir(AgiGame *state, uint8 *p) {
	_v[p1] = vt.direction;
}

void cmdGetRoomF(AgiGame *state, uint8 *p) {
	_v[p1] = state->_vm->objectGetLocation(_v[p0]);
}

void cmdPut(AgiGame *state, uint8 *p) {
	state->_vm->objectSetLocation(p0, _v[p1]);
}

void cmdPutF(AgiGame *state, uint8 *p) {
	state->_vm->objectSetLocation(_v[p0], _v[p1]);
}

void cmdDrop(AgiGame *state, uint8 *p) {
	state->_vm->objectSetLocation(p0, 0);
}

void cmdGet(AgiGame *state, uint8 *p) {
	state->_vm->objectSetLocation(p0, EGO_OWNED);
}

void cmdGetV1(AgiGame *state, uint8 *p) {
	state->_vm->objectSetLocation(p0, EGO_OWNED_V1);
}

void cmdGetF(AgiGame *state, uint8 *p) {
	state->_vm->objectSetLocation(_v[p0], EGO_OWNED);
}

void cmdWordToString(AgiGame *state, uint8 *p) {
	strcpy(state->strings[p0], state->egoWords[p1].word);
}

void cmdOpenDialogue(AgiGame *state, uint8 *p) {
	state->hasWindow = true;
}

void cmdCloseDialogue(AgiGame *state, uint8 *p) {
	state->hasWindow = false;
}

void cmdCloseWindow(AgiGame *state, uint8 *p) {
	state->_vm->closeWindow();
}

void cmdStatusLineOn(AgiGame *state, uint8 *p) {
	state->statusLine = true;
	state->_vm->writeStatus();
}

void cmdStatusLineOff(AgiGame *state, uint8 *p) {
	state->statusLine = false;
	state->_vm->writeStatus();
}

void cmdShowObj(AgiGame *state, uint8 *p) {
	state->_vm->_sprites->showObj(p0);
}

void cmdShowObjV(AgiGame *state, uint8 *p) {
	state->_vm->_sprites->showObj(_v[p0]);
}

void cmdSound(AgiGame *state, uint8 *p) {
	state->_vm->_sound->startSound(p0, p1);
}

void cmdStopSound(AgiGame *state, uint8 *p) {
	state->_vm->_sound->stopSound();
}

void cmdMenuInput(AgiGame *state, uint8 *p) {
	state->_vm->newInputMode(INPUT_MENU);
}

void cmdEnableItem(AgiGame *state, uint8 *p) {
	state->_vm->_menu->setItem(p0, true);
}

void cmdDisableItem(AgiGame *state, uint8 *p) {
	state->_vm->_menu->setItem(p0, false);
}

void cmdSubmitMenu(AgiGame *state, uint8 *p) {
	state->_vm->_menu->submit();
}

void cmdSetScanStart(AgiGame *state, uint8 *p) {
	state->_curLogic->sIP = state->_curLogic->cIP;
}

void cmdResetScanStart(AgiGame *state, uint8 *p) {
	state->_curLogic->sIP = 2;
}

void cmdSaveGame(AgiGame *state, uint8 *p) {
	state->simpleSave ? state->_vm->saveGameSimple() : state->_vm->saveGameDialog();
}

void cmdLoadGame(AgiGame *state, uint8 *p) {
	assert(1);
	state->simpleSave ? state->_vm->loadGameSimple() : state->_vm->loadGameDialog();
}

void cmdInitDisk(AgiGame *state, uint8 *p) {				// do nothing
}

void cmdLog(AgiGame *state, uint8 *p) {				// do nothing
}

void cmdTraceOn(AgiGame *state, uint8 *p) {				// do nothing
}

void cmdTraceInfo(AgiGame *state, uint8 *p) {				// do nothing
}

void cmdShowMem(AgiGame *state, uint8 *p) {
	state->_vm->messageBox("Enough memory");
}

void cmdInitJoy(AgiGame *state, uint8 *p) { // do nothing
}

void cmdScriptSize(AgiGame *state, uint8 *p) {
	debug(0, "script.size(%d)", p0);
}

void cmdCancelLine(AgiGame *state, uint8 *p) {
	state->inputBuffer[0] = 0;
	state->cursorPos = 0;
	state->_vm->writePrompt();
}

// This implementation is based on observations of Amiga's Gold Rush.
// You can try this out (in the original and in ScummVM) by writing "bird man"
// to enter Gold Rush's debug mode and then writing "show position" or "sp".
// TODO: Make the cycle and motion status lines more like in Amiga's Gold Rush.
// TODO: Add control status line (After stepsize, before cycle status).
// Don't know what the control status means yet, possibly flags?
// Examples of the control-value (Taken in the first screen i.e. room 1):
// 4051 (When ego is stationary),
// 471 (When walking on the first screen's bridge),
// 71 (When walking around, using the mouse or the keyboard).
void cmdObjStatusF(AgiGame *state, uint8 *p) {
	const char *cycleDesc;  // Object's cycle description line
	const char *motionDesc; // Object's motion description line
	char msg[256];          // The whole object status message

	// Generate cycle description line
	switch (vt_v.cycle) {
	case kCycleNormal:
		cycleDesc = "normal cycle";
		break;
	case kCycleEndOfLoop:
		cycleDesc = "end of loop";
		break;
	case kCycleRevLoop:
		cycleDesc = "reverse loop";
		break;
	case kCycleReverse:
		cycleDesc = "reverse cycle";
		break;
	default:
		cycleDesc = "unknown cycle type";
		break;
	}

	// Generate motion description line
	switch (vt_v.motion) {
	case kMotionNormal:
		motionDesc = "normal motion";
		break;
	case kMotionWander:
		motionDesc = "wandering";
		break;
	case kMotionFollowEgo:
		motionDesc = "following ego";
		break;
	case kMotionMoveObj:
		// Amiga's Gold Rush! most probably uses "move to (x, y)"
		// here with real values for x and y. The same output
		// is used when moving the ego around using the mouse.
		motionDesc = "moving to a point";
		break;
	default:
		motionDesc = "unknown motion type";
		break;
	}

	sprintf(msg,
		"Object %d:\n" \
		"x: %d  xsize: %d\n" \
		"y: %d  ysize: %d\n" \
		"pri: %d\n" \
		"stepsize: %d\n" \
		"%s\n" \
		"%s",
		_v[p0],
		vt_v.xPos, vt_v.xSize,
		vt_v.yPos, vt_v.ySize,
		vt_v.priority,
		vt_v.stepSize,
		cycleDesc,
		motionDesc);
	state->_vm->messageBox(msg);
}

// unknown commands:
// unk_170: Force savegame name -- j5
// unk_171: script save -- j5
// unk_172: script restore -- j5
// unk_173: Activate keypressed control (ego only moves while key is pressed)
// unk_174: Change priority table (used in KQ4) -- j5
// unk_177: Disable menus completely -- j5
// unk_181: Deactivate keypressed control (default control of ego)
void cmdSetSimple(AgiGame *state, uint8 *p) {
	if (!(getFeatures() & (GF_AGI256 | GF_AGI256_2))) {
		state->simpleSave = true;
	} else { // AGI256 and AGI256-2 use this unknown170 command to load 256 color pictures.
		// Load the picture. Similar to void cmdLoad_pic(AgiGame *state, uint8 *p).
		state->_vm->_sprites->eraseBoth();
		state->_vm->agiLoadResource(rPICTURE, _v[p0]);

		// Draw the picture. Similar to void cmdDraw_pic(AgiGame *state, uint8 *p).
		state->_vm->_picture->decodePicture(_v[p0], false, true);
		state->_vm->_sprites->blitBoth();
		state->pictureShown = 0;

		// Show the picture. Similar to void cmdShow_pic(AgiGame *state, uint8 *p).
		setflag(fOutputMode, false);
		state->_vm->closeWindow();
		state->_vm->_picture->showPic();
		state->pictureShown = 1;

		// Simulate slowww computer. Many effects rely on this
		state->_vm->pause(kPausePicture);
	}
}

void cmdPopScript(AgiGame *state, uint8 *p) {
	if (getVersion() >= 0x2915) {
		debug(0, "pop.script");
	}
}

void cmdHoldKey(AgiGame *state, uint8 *p) {
	if (getVersion() >= 0x3098) {
		state->_vm->_egoHoldKey = true;
	}
}

void cmdDiscardSound(AgiGame *state, uint8 *p) {
	if (getVersion() >= 0x2936) {
		debug(0, "discard.sound");
	}
}

void cmdHideMouse(AgiGame *state, uint8 *p) {
	// WORKAROUND: Turns off current movement that's being caused with the mouse.
	// This fixes problems with too many popup boxes appearing in the Amiga
	// Gold Rush's copy protection failure scene (i.e. the hanging scene, logic.192).
	// Previously multiple popup boxes appeared one after another if you tried
	// to walk somewhere else than to the right using the mouse.
	// FIXME: Write a proper implementation using disassembly and
	//        apply it to other games as well if applicable.
	state->viewTable[0].flags &= ~fAdjEgoXY;

	g_system->showMouse(false);
}

void cmdAllowMenu(AgiGame *state, uint8 *p) {
	if (getVersion() >= 0x3098) {
		setflag(fMenusWork, ((p0 != 0) ? true : false));
	}
}

void cmdShowMouse(AgiGame *state, uint8 *p) {
	g_system->showMouse(true);
}

void cmdFenceMouse(AgiGame *state, uint8 *p) {
	state->mouseFence.moveTo(p0, p1);
	state->mouseFence.setWidth(p2 - p0);
	state->mouseFence.setHeight(p3 - p1);
}

void cmdReleaseKey(AgiGame *state, uint8 *p) {
	if (getVersion() >= 0x3098) {
		state->_vm->_egoHoldKey = false;
	}
}

void cmdAdjEgoMoveToXY(AgiGame *state, uint8 *p) {
	int8 x, y;

	switch (logicNamesCmd[182].argumentsLength()) {
	// The 2 arguments version is used at least in Amiga Gold Rush!
	// (v2.05 1989-03-09, Amiga AGI 2.316) in logics 130 and 150
	// (Using arguments (0, 0), (0, 7), (0, 8), (9, 9) and (-9, 9)).
	case 2:
		// Both arguments are signed 8-bit (i.e. in range -128 to +127).
		x = (int8) p0;
		y = (int8) p1;

		// Turn off ego's current movement caused with the mouse if
		// adj.ego.move.to.x.y is called with other arguments than previously.
		// Fixes weird looping behavior when walking to a ladder in the mines
		// (Rooms 147-162) in Gold Rush using the mouse. Sometimes the ego didn't
		// stop when walking to a ladder using the mouse but kept moving on the
		// ladder in a horizontally looping manner i.e. from right to left, from
		// right to left etc. In the Amiga Gold Rush the ego stopped when getting
		// onto the ladder so this is more like it (Although that may be caused
		// by something else because this command doesn't do any flag manipulations
		// in the Amiga version - checked it with disassembly).
		if (x != state->adjMouseX || y != state->adjMouseY)
			state->viewTable[EGO_VIEW_TABLE].flags &= ~fAdjEgoXY;

		state->adjMouseX = x;
		state->adjMouseY = y;

		debugC(4, kDebugLevelScripts, "adj.ego.move.to.x.y(%d, %d)", x, y);
		break;
	// TODO: Check where (if anywhere) the 0 arguments version is used
	case 0:
	default:
		state->viewTable[0].flags |= fAdjEgoXY;
		break;
	}
}

void cmdParse(AgiGame *state, uint8 *p) {
	_v[vWordNotFound] = 0;
	setflag(fEnteredCli, false);
	setflag(fSaidAcceptedInput, false);

	state->_vm->dictionaryWords(state->_vm->agiSprintf(state->strings[p0]));
}

void cmdCall(AgiGame *state, uint8 *p) {
	int oldCIP;
	int oldLognum;

	// CM: we don't save sIP because set.scan.start can be
	//     used in a called script (fixes xmas demo)
	oldCIP = state->_curLogic->cIP;
	oldLognum = state->lognum;

	state->_vm->runLogic(p0);

	state->lognum = oldLognum;
	state->_curLogic = &state->logics[state->lognum];
	state->_curLogic->cIP = oldCIP;
}

void cmdCallF(AgiGame *state, uint8 *p) {
	cmdCall(state, &_v[p0]);
}

void cmdDrawPicV1(AgiGame *state, uint8 *p) {
	debugC(6, kDebugLevelScripts, "=== draw pic V1 %d ===", _v[p0]);
	state->_vm->_picture->decodePicture(_v[p0], true);

	state->_vm->clearPrompt();

	// Simulate slowww computer. Many effects rely on this
	state->_vm->pause(kPausePicture);
}

void cmdDrawPic(AgiGame *state, uint8 *p) {
	debugC(6, kDebugLevelScripts, "=== draw pic %d ===", _v[p0]);
	state->_vm->_sprites->eraseBoth();
	state->_vm->_picture->decodePicture(_v[p0], true);
	state->_vm->_sprites->blitBoth();
	state->_vm->_sprites->commitBoth();
	state->pictureShown = 0;
	debugC(6, kDebugLevelScripts, "--- end of draw pic %d ---", _v[p0]);

	// WORKAROUND for a script bug which exists in SQ1, logic scripts
	// 20 and 110. Flag 103 is not reset correctly, which leads to erroneous
	// behavior from view 46 (the spider droid). View 46 is supposed to
	// follow ego and explode when it comes in contact with him. However, as
	// flag 103 is not reset correctly, when the player goes down the path
	// and back up, the spider is always at the base of the path (since it
	// can't go up) and kills the player when he goes down at ground level
	// (although the spider droid sprite itself seems to be correctly positioned).
	// With this workaround, when the player goes back to picture 20 (1 screen
	// above the ground), flag 103 is reset, thereby fixing this issue. Note
	// that this is a script bug and occurs in the original interpreter as well.
	// Fixes bug #1658514: AGI: SQ1 (2.2 DOS ENG) bizzare exploding roger
	if (getGameID() == GID_SQ1 && _v[p0] == 20)
		setflag(103, false);

	// Simulate slowww computer. Many effects rely on this
	state->_vm->pause(kPausePicture);
}

void cmdShowPic(AgiGame *state, uint8 *p) {
	debugC(6, kDebugLevelScripts, "=== show pic ===");

	setflag(fOutputMode, false);
	state->_vm->closeWindow();
	state->_vm->_picture->showPic();
	state->pictureShown = 1;

	debugC(6, kDebugLevelScripts, "--- end of show pic ---");
}

void cmdLoadPic(AgiGame *state, uint8 *p) {
	state->_vm->_sprites->eraseBoth();
	state->_vm->agiLoadResource(rPICTURE, _v[p0]);
	state->_vm->_sprites->blitBoth();
	state->_vm->_sprites->commitBoth();
}

void cmdLoadPicV1(AgiGame *state, uint8 *p) {
	state->_vm->agiLoadResource(rPICTURE, _v[p0]);
}

void cmdDiscardPic(AgiGame *state, uint8 *p) {
	debugC(6, kDebugLevelScripts, "--- discard pic ---");
	// do nothing
}

void cmdOverlayPic(AgiGame *state, uint8 *p) {
	debugC(6, kDebugLevelScripts, "--- overlay pic ---");

	state->_vm->_sprites->eraseBoth();
	state->_vm->_picture->decodePicture(_v[p0], false);
	state->_vm->_sprites->blitBoth();
	state->pictureShown = 0;
	state->_vm->_sprites->commitBoth();

	// Simulate slowww computer. Many effects rely on this
	state->_vm->pause(kPausePicture);
}

void cmdShowPriScreen(AgiGame *state, uint8 *p) {
	state->_vm->_debug.priority = 1;
	state->_vm->_sprites->eraseBoth();
	state->_vm->_picture->showPic();
	state->_vm->_sprites->blitBoth();

	state->_vm->waitKey();

	state->_vm->_debug.priority = 0;
	state->_vm->_sprites->eraseBoth();
	state->_vm->_picture->showPic();
	state->_vm->_sprites->blitBoth();
}

void cmdAnimateObj(AgiGame *state, uint8 *p) {
	if (getVersion() < 0x2000) {
		if (vt.flags & fDidntMove)
			return;
	} else {
		if (vt.flags & fAnimated)
			return;
	}

	debugC(4, kDebugLevelScripts, "animate vt entry #%d", p0);
	vt.flags = fAnimated | fUpdate | fCycling;

	if (getVersion() < 0x2000) {
		vt.flags |= fDidntMove;
	}

	vt.motion = kMotionNormal;
	vt.cycle = kCycleNormal;
	vt.direction = 0;
}

void cmdUnanimateAll(AgiGame *state, uint8 *p) {
	int i;

	for (i = 0; i < MAX_VIEWTABLE; i++)
		state->viewTable[i].flags &= ~(fAnimated | fDrawn);
}

void cmdDraw(AgiGame *state, uint8 *p) {
	if (vt.flags & fDrawn)
		return;

	if (vt.ySize <= 0 || vt.xSize <= 0)
		return;

	debugC(4, kDebugLevelScripts, "draw entry %d", vt.entry);

	vt.flags |= fUpdate;
	if (getVersion() >= 0x3000) {
		state->_vm->setLoop(&vt, vt.currentLoop);
		state->_vm->setCel(&vt, vt.currentCel);
	}

	state->_vm->fixPosition(p0);
	vt.xPos2 = vt.xPos;
	vt.yPos2 = vt.yPos;
	vt.celData2 = vt.celData;
	state->_vm->_sprites->eraseUpdSprites();
	vt.flags |= fDrawn;

	// WORKAROUND: This fixes a bug with AGI Fanmade game Space Trek.
	// The original workaround checked if AGI version was <= 2.440, which could
	// cause regressions with some AGI games. The original workaround no longer
	// works for Space Trek in ScummVM, as all fanmade games are set to use
	// AGI version 2.917, but it applies to all other games where AGI version is
	// <= 2.440, which was not the original purpose of this workaround. It is
	// assumed that this bug is caused by AGI Studio, so this applies to all
	// fanmade games only.
	// TODO: Investigate this further and check if any other fanmade AGI
	// games are affected. If yes, then it'd be best to set this for Space
	// Trek only
	if (getFeatures() & GF_FANMADE)	// See Sarien bug #546562
		vt.flags |= fAnimated;

	state->_vm->_sprites->blitUpdSprites();
	vt.flags &= ~fDontupdate;

	state->_vm->_sprites->commitBlock(vt.xPos, vt.yPos - vt.ySize + 1, vt.xPos + vt.xSize - 1, vt.yPos, true);

	debugC(4, kDebugLevelScripts, "vt entry #%d flags = %02x", p0, vt.flags);
}

void cmdErase(AgiGame *state, uint8 *p) {
	if (~vt.flags & fDrawn)
		return;

	state->_vm->_sprites->eraseUpdSprites();

	if (vt.flags & fUpdate) {
		vt.flags &= ~fDrawn;
	} else {
		state->_vm->_sprites->eraseNonupdSprites();
		vt.flags &= ~fDrawn;
		state->_vm->_sprites->blitNonupdSprites();
	}
	state->_vm->_sprites->blitUpdSprites();

	int x1, y1, x2, y2;

	x1 = MIN((int)MIN(vt.xPos, vt.xPos2), MIN(vt.xPos + vt.celData->width, vt.xPos2 + vt.celData2->width));
	x2 = MAX((int)MAX(vt.xPos, vt.xPos2), MAX(vt.xPos + vt.celData->width, vt.xPos2 + vt.celData2->width));
	y1 = MIN((int)MIN(vt.yPos, vt.yPos2), MIN(vt.yPos - vt.celData->height, vt.yPos2 - vt.celData2->height));
	y2 = MAX((int)MAX(vt.yPos, vt.yPos2), MAX(vt.yPos - vt.celData->height, vt.yPos2 - vt.celData2->height));

	state->_vm->_sprites->commitBlock(x1, y1, x2, y2, true);
}

void cmdPosition(AgiGame *state, uint8 *p) {
	vt.xPos = vt.xPos2 = p1;
	vt.yPos = vt.yPos2 = p2;

	// WORKAROUND: Part of the fix for bug #1659209 "AGI: Space Trek sprite duplication"
	// with an accompanying identical workaround in position.v-command (i.e. command 0x26).
	// These two workarounds together make up the whole fix. The bug was caused by
	// wrongly written script data in Space Trek v1.0's scripts (At least logics 4 and 11).
	// Position-command was called with horizontal values over 200 (Outside the screen!).
	// Clipping the coordinates so the views stay wholly on-screen seems to fix the problems.
	//   It is probable (Would have to check better with disassembly to be completely sure)
	// that AGI 2.440 clipped its coordinates in its position and position.v-commands
	// although AGI 2.917 certainly doesn't (Checked that with disassembly) and that's why
	// Space Trek may have worked better with AGI 2.440 than with some other AGI versions.
	//   I haven't checked but if Space Trek solely abuses the position-command we wouldn't
	// strictly need the identical workaround in the position.v-command but it does make
	// for a nice symmetry.
	if (getFeatures() & GF_CLIPCOORDS)
		state->_vm->clipViewCoordinates(&vt);
}

void cmdPositionV1(AgiGame *state, uint8 *p) {
	vt.xPos = p1;
	vt.yPos = p2;
}

void cmdPositionF(AgiGame *state, uint8 *p) {
	vt.xPos = vt.xPos2 = _v[p1];
	vt.yPos = vt.yPos2 = _v[p2];

	// WORKAROUND: Part of the fix for bug #1659209 "AGI: Space Trek sprite duplication"
	// with an accompanying identical workaround in position-command (i.e. command 0x25).
	// See that workaround's comment for more in-depth information.
	if (getFeatures() & GF_CLIPCOORDS)
		state->_vm->clipViewCoordinates(&vt);
}

void cmdPositionFV1(AgiGame *state, uint8 *p) {
	vt.xPos = _v[p1];
	vt.yPos = _v[p2];
}

void cmdGetPosn(AgiGame *state, uint8 *p) {
	state->vars[p1] = (unsigned char)vt.xPos;
	state->vars[p2] = (unsigned char)vt.yPos;
}

void cmdReposition(AgiGame *state, uint8 *p) {
	int dx = (int8) _v[p1], dy = (int8) _v[p2];

	debugC(4, kDebugLevelScripts, "dx=%d, dy=%d", dx, dy);
	vt.flags |= fUpdatePos;

	if (dx < 0 && vt.xPos < -dx)
		vt.xPos = 0;
	else
		vt.xPos += dx;

	if (dy < 0 && vt.yPos < -dy)
		vt.yPos = 0;
	else
		vt.yPos += dy;

	state->_vm->fixPosition(p0);
}

void cmdRepositionV1(AgiGame *state, uint8 *p) {
	vt.xPos2 = vt.xPos;
	vt.yPos2 = vt.yPos;
	vt.flags |= fUpdatePos;

	vt.xPos = (vt.xPos + p1) & 0xff;
	vt.yPos = (vt.yPos + p2) & 0xff;
}

void cmdRepositionTo(AgiGame *state, uint8 *p) {
	vt.xPos = p1;
	vt.yPos = p2;
	vt.flags |= fUpdatePos;
	state->_vm->fixPosition(p0);
}

void cmdRepositionToF(AgiGame *state, uint8 *p) {
	vt.xPos = _v[p1];
	vt.yPos = _v[p2];
	vt.flags |= fUpdatePos;
	state->_vm->fixPosition(p0);
}

void cmdAddToPic(AgiGame *state, uint8 *p) {
	state->_vm->_sprites->addToPic(p0, p1, p2, p3, p4, p5, p6);
}

void cmdAddToPicV1(AgiGame *state, uint8 *p) {
	state->_vm->_sprites->addToPic(p0, p1, p2, p3, p4, p5, -1);
}

void cmdAddToPicF(AgiGame *state, uint8 *p) {
	state->_vm->_sprites->addToPic(_v[p0], _v[p1], _v[p2], _v[p3], _v[p4], _v[p5], _v[p6]);
}

void cmdForceUpdate(AgiGame *state, uint8 *p) {
	state->_vm->_sprites->eraseBoth();
	state->_vm->_sprites->blitBoth();
	state->_vm->_sprites->commitBoth();
}

void cmdReverseLoop(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts, "o%d, f%d", p0, p1);
	vt.cycle = kCycleRevLoop;
	vt.flags |= (fDontupdate | fUpdate | fCycling);
	vt.parm1 = p1;
	setflag(p1, false);
}

void cmdReverseLoopV1(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts, "o%d, f%d", p0, p1);
	vt.cycle = kCycleRevLoop;
	state->_vm->setCel(&vt, 0);
	vt.flags |= (fDontupdate | fUpdate | fCycling);
	vt.parm1 = p1;
	vt.parm3 = 0;
}

void cmdEndOfLoop(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts, "o%d, f%d", p0, p1);
	vt.cycle = kCycleEndOfLoop;
	vt.flags |= (fDontupdate | fUpdate | fCycling);
	vt.parm1 = p1;
	setflag(p1, false);
}

void cmdEndOfLoopV1(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts, "o%d, f%d", p0, p1);
	vt.cycle = kCycleEndOfLoop;
	state->_vm->setCel(&vt, 0);
	vt.flags |= (fDontupdate | fUpdate | fCycling);
	vt.parm1 = p1;
	vt.parm3 = 0;
}

void cmdBlock(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts, "x1=%d, y1=%d, x2=%d, y2=%d", p0, p1, p2, p3);
	state->block.active = true;
	state->block.x1 = p0;
	state->block.y1 = p1;
	state->block.x2 = p2;
	state->block.y2 = p3;
}

void cmdUnblock(AgiGame *state, uint8 *p) {
	state->block.active = false;
}

void cmdNormalMotion(AgiGame *state, uint8 *p) {
	vt.motion = kMotionNormal;
}

void cmdStopMotion(AgiGame *state, uint8 *p) {
	vt.direction = 0;
	vt.motion = kMotionNormal;
	if (p0 == 0) {		// ego only
		_v[vEgoDir] = 0;
		state->playerControl = false;
	}
}

void cmdStopMotionV1(AgiGame *state, uint8 *p) {
	vt.flags &= ~fAnimated;
}

void cmdStartMotion(AgiGame *state, uint8 *p) {
	vt.motion = kMotionNormal;
	if (p0 == 0) {		// ego only
		_v[vEgoDir] = 0;
		state->playerControl = true;
	}
}

void cmdStartMotionV1(AgiGame *state, uint8 *p) {
	vt.flags |= fAnimated;
}

void cmdPlayerControl(AgiGame *state, uint8 *p) {
	state->playerControl = true;
	state->viewTable[0].motion = kMotionNormal;
}

void cmdProgramControl(AgiGame *state, uint8 *p) {
	state->playerControl = false;
}

void cmdFollowEgo(AgiGame *state, uint8 *p) {
	vt.motion = kMotionFollowEgo;
	vt.parm1 = p1 > vt.stepSize ? p1 : vt.stepSize;
	vt.parm2 = p2;
	vt.parm3 = 0xff;

	if (getVersion() < 0x2000) {
		_v[p2] = 0;
		vt.flags |= fUpdate | fAnimated;
	} else {
		setflag(p2, false);
		vt.flags |= fUpdate;
	}
}

void cmdMoveObj(AgiGame *state, uint8 *p) {
	// _D (_D_WARN "o=%d, x=%d, y=%d, s=%d, f=%d", p0, p1, p2, p3, p4);

	vt.motion = kMotionMoveObj;
	vt.parm1 = p1;
	vt.parm2 = p2;
	vt.parm3 = vt.stepSize;
	vt.parm4 = p4;

	if (p3 != 0)
		vt.stepSize = p3;

	if (getVersion() < 0x2000) {
		_v[p4] = 0;
		vt.flags |= fUpdate | fAnimated;
	} else {
		setflag(p4, false);
		vt.flags |= fUpdate;
	}

	if (p0 == 0)
		state->playerControl = false;

	// AGI 2.272 (ddp, xmas) doesn't call move_obj!
	if (getVersion() > 0x2272)
		state->_vm->moveObj(&vt);
}

void cmdMoveObjF(AgiGame *state, uint8 *p) {
	vt.motion = kMotionMoveObj;
	vt.parm1 = _v[p1];
	vt.parm2 = _v[p2];
	vt.parm3 = vt.stepSize;
	vt.parm4 = p4;

	if (_v[p3] != 0)
		vt.stepSize = _v[p3];

	setflag(p4, false);
	vt.flags |= fUpdate;

	if (p0 == 0)
		state->playerControl = false;

	// AGI 2.272 (ddp, xmas) doesn't call move_obj!
	if (getVersion() > 0x2272)
		state->_vm->moveObj(&vt);
}

void cmdWander(AgiGame *state, uint8 *p) {
	if (p0 == 0)
		state->playerControl = false;

	vt.motion = kMotionWander;
	if (getVersion() < 0x2000) {
		vt.flags |= fUpdate | fAnimated;
	} else {
		vt.flags |= fUpdate;
	}
}

void cmdSetGameID(AgiGame *state, uint8 *p) {
	if (state->_curLogic->texts && (p0 - 1) <= state->_curLogic->numTexts)
		strncpy(state->id, state->_curLogic->texts[p0 - 1], 8);
	else
		state->id[0] = 0;

	debug(0, "Game ID: \"%s\"", state->id);
}

void cmdPause(AgiGame *state, uint8 *p) {
	int tmp = state->clockEnabled;
	const char *b[] = { "Continue", NULL };
	const char *b_ru[] = { "\x8f\xe0\xae\xa4\xae\xab\xa6\xa8\xe2\xec", NULL };

	state->clockEnabled = false;

	switch (getLanguage()) {
	case Common::RU_RUS:
		state->_vm->selectionBox("  \x88\xa3\xe0\xa0 \xae\xe1\xe2\xa0\xad\xae\xa2\xab\xa5\xad\xa0.  \n\n\n", b_ru);
		break;
	default:
		state->_vm->selectionBox("  Game is paused.  \n\n\n", b);
		break;
	}
	state->clockEnabled = tmp;
}

void cmdSetMenu(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts, "text %02x of %02x", p0, state->_curLogic->numTexts);

	if (state->_curLogic->texts != NULL && p0 <= state->_curLogic->numTexts)
		state->_vm->_menu->add(state->_curLogic->texts[p0 - 1]);
}

void cmdSetMenuItem(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts, "text %02x of %02x", p0, state->_curLogic->numTexts);

	if (state->_curLogic->texts != NULL && p0 <= state->_curLogic->numTexts)
		state->_vm->_menu->addItem(state->_curLogic->texts[p0 - 1], p1);
}

void cmdVersion(AgiGame *state, uint8 *p) {
	char ver2Msg[] =
	    "\n"
	    "                               \n\n"
	    "  Emulating Sierra AGI v%x.%03x\n";
	char ver3Msg[] =
	    "\n"
	    "                             \n\n"
	    "  Emulating AGI v%x.002.%03x\n";
	// no Sierra as it wraps textbox

	Common::String verMsg = TITLE " v%s";

	int ver = getVersion();
	int maj = (ver >> 12) & 0xf;
	int min = ver & 0xfff;

	verMsg += (maj == 2 ? ver2Msg : ver3Msg);
	verMsg = Common::String::format(verMsg.c_str(), gScummVMVersion, maj, min);

	state->_vm->messageBox(verMsg.c_str());
}

void cmdConfigureScreen(AgiGame *state, uint8 *p) {
	state->lineMinPrint = p0;
	state->lineUserInput = p1;
	state->lineStatus = p2;
}

void cmdTextScreen(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts, "switching to text mode");
	state->gfxMode = false;

	// Simulates the "bright background bit" of the PC video
	// controller.
	if (state->colorBg)
		state->colorBg |= 0x08;

	state->_vm->_gfx->clearScreen(state->colorBg);
}

void cmdGraphics(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts, "switching to graphics mode");

	if (!state->gfxMode) {
		state->gfxMode = true;
		state->_vm->_gfx->clearScreen(0);
		state->_vm->_picture->showPic();
		state->_vm->writeStatus();
		state->_vm->writePrompt();
	}
}

void cmdSetTextAttribute(AgiGame *state, uint8 *p) {
	state->colorFg = p0;
	state->colorBg = p1;

	if (state->gfxMode) {
		if (state->colorBg != 0) {
			state->colorFg = 0;
			state->colorBg = 15;
		}
	}
}

void cmdStatus(AgiGame *state, uint8 *p) {
	state->_vm->inventory();
}

void cmdQuit(AgiGame *state, uint8 *p) {
	const char *buttons[] = { "Quit", "Continue", NULL };

	state->_vm->_sound->stopSound();
	if (p0) {
		state->_vm->quitGame();
	} else {
		if (state->_vm->selectionBox(" Quit the game, or continue? \n\n\n", buttons) == 0) {
			state->_vm->quitGame();
		}
	}
}

void cmdQuitV1(AgiGame *state, uint8 *p) {
	state->_vm->_sound->stopSound();
	state->_vm->quitGame();
}

void cmdRestartGame(AgiGame *state, uint8 *p) {
	const char *buttons[] = { "Restart", "Continue", NULL };
	int sel;

	state->_vm->_sound->stopSound();
	sel = getflag(fAutoRestart) ? 0 :
		state->_vm->selectionBox(" Restart game, or continue? \n\n\n", buttons);

	if (sel == 0) {
		state->_vm->_restartGame = true;
		setflag(fRestartGame, true);
		state->_vm->_menu->enableAll();
	}
}

void cmdDistance(AgiGame *state, uint8 *p) {
	int16 x1, y1, x2, y2, d;
	VtEntry *v0 = &state->viewTable[p0];
	VtEntry *v1 = &state->viewTable[p1];

	if (v0->flags & fDrawn && v1->flags & fDrawn) {
		x1 = v0->xPos + v0->xSize / 2;
		y1 = v0->yPos;
		x2 = v1->xPos + v1->xSize / 2;
		y2 = v1->yPos;
		d = ABS(x1 - x2) + ABS(y1 - y2);
		if (d > 0xfe)
			d = 0xfe;
	} else {
		d = 0xff;
	}

	// WORKAROUND: Fixes King's Quest IV's script bug #1660424 (KQ4: Zombie bug).
	// In the graveyard (Rooms 16 and 18) at night if you had the Obsidian Scarab (Item 4)
	// and you were very close to a spot where a zombie was going to rise up from the
	// ground you could reproduce the bug. Just standing there and letting the zombie
	// try to rise up the Obsidian Scarab would repel the zombie immediately and that
	// would make the script bug so that the zombie would still come up but it just
	// wouldn't chase Rosella around anymore. If it had worked correctly the zombie
	// wouldn't have come up at all or it would have come up and gone back down
	// immediately. The latter approach is the one implemented here.
	if (getGameID() == GID_KQ4 && (_v[vCurRoom] == 16 || _v[vCurRoom] == 18) && p2 >= 221 && p2 <= 223) {
		// Rooms 16 and 18 are graveyards where three zombies come up at night. They use logics 16 and 18.
		// Variables 221-223 are used to save the distance between each zombie and Rosella.
		// Variables 155, 156 and 162 are used to save the state of each zombie in room 16.
		// Variables 155, 156 and 158 are used to save the state of each zombie in room 18.
		// Rosella gets turned to a zombie only if any of the zombies is under 10 units away
		// from her and she doesn't have the Obsidian Scarab (Item 4). Likewise Rosella makes
		// a zombie go back into the ground if the zombie comes under 15 units away from her
		// and she has the Obsidian Scarab. To ensure a zombie always first rises up before
		// checking for either of the aforementioned conditions (Rosella getting turned to
		// a zombie or the zombie getting turned away by the scarab) we make it appear the
		// zombie is far away from Rosella if the zombie is not already up and chasing her.
		enum zombieStates {ZOMBIE_SET_TO_RISE_UP, ZOMBIE_RISING_UP, ZOMBIE_CHASING_EGO};
		uint8 zombieStateVarNumList[] = {155, 156, (_v[vCurRoom] == 16) ? 162 : 158};
		uint8 zombieNum         = p2 - 221;                         // Zombie's number (In range 0-2)
		uint8 zombieStateVarNum = zombieStateVarNumList[zombieNum]; // Number of the variable containing zombie's state
		uint8 zombieState       = _v[zombieStateVarNum];            // Zombie's state
		// If zombie is not chasing Rosella then set its distance from Rosella to the maximum
		if (zombieState != ZOMBIE_CHASING_EGO)
			d = 0xff;
	}

	_v[p2] = (unsigned char)d;
}

void cmdAcceptInput(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts | kDebugLevelInput, "input normal");

	state->_vm->newInputMode(INPUT_NORMAL);
	state->inputEnabled = true;
	state->_vm->writePrompt();
}

void cmdPreventInput(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts | kDebugLevelInput, "no input");

	state->_vm->newInputMode(INPUT_NONE);
	state->inputEnabled = false;

	// Always clear with black background. Fixes bug #3080041.
	state->_vm->clearPrompt(true);
}

void cmdGetString(AgiGame *state, uint8 *p) {
	int tex, row, col;

	debugC(4, kDebugLevelScripts, "%d %d %d %d %d", p0, p1, p2, p3, p4);

	tex = p1 - 1;
	row = p2;
	col = p3;

	// Workaround for SQLC bug.
	// See Sarien bug #792125 for details
	if (row > 24)
		row = 24;
	if (col > 39)
		col = 39;

	state->_vm->newInputMode(INPUT_GETSTRING);

	if (state->_curLogic->texts != NULL && state->_curLogic->numTexts >= tex) {
		int len = strlen(state->_curLogic->texts[tex]);

		state->_vm->printText(state->_curLogic->texts[tex], 0, col, row, len, state->colorFg, state->colorBg);
		state->_vm->getString(col + len - 1, row, p4, p0);

		// SGEO: display input char
		state->_vm->_gfx->printCharacter((col + len), row, state->cursorChar, state->colorFg, state->colorBg);
	}

	do {
		state->_vm->mainCycle();
	} while (state->inputMode == INPUT_GETSTRING && !(state->_vm->shouldQuit() || state->_vm->_restartGame));
}

void cmdGetNum(AgiGame *state, uint8 *p) {
	debugC(4, kDebugLevelScripts, "%d %d", p0, p1);

	state->_vm->newInputMode(INPUT_GETSTRING);

	if (state->_curLogic->texts != NULL && state->_curLogic->numTexts >= (p0 - 1)) {
		int len = strlen(state->_curLogic->texts[p0 - 1]);

		state->_vm->printText(state->_curLogic->texts[p0 - 1], 0, 0, 22, len, state->colorFg, state->colorBg);
		state->_vm->getString(len - 1, 22, 3, MAX_STRINGS);

		// CM: display input char
		state->_vm->_gfx->printCharacter((p3 + len), 22, state->cursorChar, state->colorFg, state->colorBg);
	}

	do {
		state->_vm->mainCycle();
	} while (state->inputMode == INPUT_GETSTRING && !(state->_vm->shouldQuit() || state->_vm->_restartGame));

	_v[p1] = atoi(state->strings[MAX_STRINGS]);

	debugC(4, kDebugLevelScripts, "[%s] -> %d", state->strings[MAX_STRINGS], _v[p1]);

	state->_vm->clearLines(22, 22, state->colorBg);
	state->_vm->flushLines(22, 22);
}

void cmdSetCursorChar(AgiGame *state, uint8 *p) {
	if (state->_curLogic->texts != NULL && (p0 - 1) <= state->_curLogic->numTexts) {
		state->cursorChar = *state->_curLogic->texts[p0 - 1];
	} else {
		// default
		state->cursorChar = '_';
	}
}

void cmdSetKey(AgiGame *state, uint8 *p) {
	int key = 256 * p1 + p0;
	int slot = -1;

	for (int i = 0; i < MAX_CONTROLLERS; i++) {
		if (slot == -1 && !state->controllers[i].keycode)
			slot = i;

		if (state->controllers[i].keycode == key && state->controllers[i].controller == p2)
			return;
	}

	if (slot == -1) {
		warning("Number of set.keys exceeded %d", MAX_CONTROLLERS);
		return;
	}

	debugC(4, kDebugLevelScripts, "cmdSetKey: %d %d %d", p0, p1, p2);
	state->controllers[slot].keycode = key;
	state->controllers[slot].controller = p2;

	state->controllerOccured[p2] = false;
}

void cmdSetString(AgiGame *state, uint8 *p) {
	// CM: to avoid crash in Groza (str = 150)
	if (p0 > MAX_STRINGS)
		return;
	strcpy(state->strings[p0], state->_curLogic->texts[p1 - 1]);
}

void cmdDisplay(AgiGame *state, uint8 *p) {
	// V1 has 4 args
	int t = (getVersion() >= 0x2000 ? p2 : p3);
	int len = 40;

	char *s = state->_vm->wordWrapString(state->_curLogic->texts[t - 1], &len);

	state->_vm->printText(s, p1, 0, p0, 40, state->colorFg, state->colorBg);

	free(s);
}

void cmdDisplayF(AgiGame *state, uint8 *p) {
	state->_vm->printText(state->_curLogic->texts[_v[p2] - 1], _v[p1], 0, _v[p0], 40, state->colorFg, state->colorBg);
}

void cmdClearTextRect(AgiGame *state, uint8 *p) {
	int c, x1, y1, x2, y2;

	if ((c = p4) != 0)
		c = 15;

	x1 = p1 * CHAR_COLS;
	y1 = p0 * CHAR_LINES;
	x2 = (p3 + 1) * CHAR_COLS - 1;
	y2 = (p2 + 1) * CHAR_LINES - 1;

	// Added to prevent crash with x2 = 40 in the iigs demo
	if (x1 > GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (x2 > GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y1 > GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (y2 > GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	state->_vm->_gfx->drawRectangle(x1, y1, x2, y2, c);
	state->_vm->_gfx->flushBlock(x1, y1, x2, y2);
}

void cmdToggleMonitor(AgiGame *state, uint8 *p) {
	debug(0, "toggle.monitor");
}

void cmdEchoLine(AgiGame *state, uint8 *p) {
	strcpy((char *)state->inputBuffer, (const char *)state->echoBuffer);
	state->cursorPos = strlen((char *)state->inputBuffer);
	state->hasPrompt = 0;
}

void cmdClearLines(AgiGame *state, uint8 *p) {
	uint8 l;

	// Residence 44 calls clear.lines(24,0,0), see Sarien bug #558423
	l = p1 ? p1 : p0;

	// Agent06 incorrectly calls clear.lines(1,150,0), see ScummVM bugs
	// #1935838 and #1935842
	l = (l <= 24) ? l : 24;

	state->_vm->clearLines(p0, l, p2);
	state->_vm->flushLines(p0, l);
}

void cmdPrint(AgiGame *state, uint8 *p) {
	int n = p0 < 1 ? 1 : p0;

	state->_vm->print(state->_curLogic->texts[n - 1], 0, 0, 0);
}

void cmdPrintF(AgiGame *state, uint8 *p) {
	int n = _v[p0] < 1 ? 1 : _v[p0];

	state->_vm->print(state->_curLogic->texts[n - 1], 0, 0, 0);
}

void cmdPrintAt(AgiGame *state, uint8 *p) {
	int n = p0 < 1 ? 1 : p0;

	debugC(4, kDebugLevelScripts, "%d %d %d %d", p0, p1, p2, p3);

	state->_vm->print(state->_curLogic->texts[n - 1], p1, p2, p3);
}

void cmdPrintAtV(AgiGame *state, uint8 *p) {
	int n = _v[p0] < 1 ? 1 : _v[p0];

	state->_vm->print(state->_curLogic->texts[n - 1], p1, p2, p3);
}

void cmdPushScript(AgiGame *state, uint8 *p) {
	// We run AGIMOUSE always as a side effect
	//if (getFeatures() & GF_AGIMOUSE || true) {
		state->vars[27] = state->_vm->_mouse.button;
		state->vars[28] = state->_vm->_mouse.x / 2;
		state->vars[29] = state->_vm->_mouse.y;
	/*} else {
		if (getVersion() >= 0x2915) {
			debug(0, "push.script");
		}
	}*/
}

void cmdSetPriBase(AgiGame *state, uint8 *p) {
	int i, x, pri;

	debug(0, "Priority base set to %d", p0);

	// state->alt_pri = true;
	x = (_HEIGHT - p0) * _HEIGHT / 10;

	for (i = 0; i < _HEIGHT; i++) {
		pri = (i - p0) < 0 ? 4 : (i - p0) * _HEIGHT / x + 5;
		if (pri > 15)
			pri = 15;
		state->priTable[i] = pri;
	}
}

void cmdMousePosn(AgiGame *state, uint8 *p) {
	_v[p0] = WIN_TO_PIC_X(state->_vm->_mouse.x);
	_v[p1] = WIN_TO_PIC_Y(state->_vm->_mouse.y);
}

void cmdShakeScreen(AgiGame *state, uint8 *p) {
	int i;

	// AGIPAL uses shake.screen values between 100 and 109 to set the palette
	// (Checked the original AGIPAL-hack's shake.screen-routine's disassembly).
	if (p0 >= 100 && p0 < 110) {
		if (getFeatures() & GF_AGIPAL) {
			state->_vm->_gfx->setAGIPal(p0);
			return;
		} else {
			warning("It looks like GF_AGIPAL flag is missing");
		}
	}

	// Disables input while shaking to prevent bug
	// #1678230: AGI: Entering text while screen is shaking
	bool originalValue = state->inputEnabled;
	state->inputEnabled = false;

	state->_vm->_gfx->shakeStart();

	state->_vm->_sprites->commitBoth();		// Fixes SQ1 demo
	for (i = 4 * p0; i; i--) {
		state->_vm->_gfx->shakeScreen(i & 1);
		state->_vm->_gfx->flushBlock(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
		state->_vm->mainCycle();
	}
	state->_vm->_gfx->shakeEnd();

	// Sets input back to what it was
	state->inputEnabled = originalValue;
}

void cmdSetSpeed(AgiGame *state, uint8 *p) {
	// V1 command
	(void)state;
	(void)p;
	// speed = _v[p0];
}

void cmdSetItemView(AgiGame *state, uint8 *p) {
	// V1 command
	(void)state;
	(void)p;
}

void cmdCallV1(AgiGame *state, uint8 *p) {
	state->_vm->agiLoadResource(rLOGIC, p0);
	// FIXME: The following instruction looks incomplete.
	// Maybe something is meant to be assigned to, or read from,
	// the logic_list entry?
	state->logic_list[++state->max_logics];
	_v[13] = 1;
}

void cmdNewRoomV1(AgiGame *state, uint8 *p) {
	warning("cmdNewRoomV1()");
	state->_vm->agiLoadResource(rLOGIC, p0);
	state->max_logics = 1;
	state->logic_list[1] = p0;
	_v[13] = 1;
}

void cmdNewRoomVV1(AgiGame *state, uint8 *p) {
	warning("cmdNewRoomVV1()");
	state->_vm->agiLoadResource(rLOGIC, _v[p0]);
	state->max_logics = 1;
	state->logic_list[1] = _v[p0];
	_v[13] = 1;
}

void cmdUnknown(AgiGame *state, uint8 *p) {
	warning("Skipping unknown opcode %2X", *(code + ip - 1));
}

/**
 * Execute a logic script
 * @param n  Number of the logic resource to execute
 */
int AgiEngine::runLogic(int n) {
	AgiGame *state = &_game;
	uint8 op = 0;
	uint8 p[CMD_BSIZE] = { 0 };
	int num = 0;
	ScriptPos sp;
	//int logic_index = 0;

	state->logic_list[0] = 0;
	state->max_logics = 0;

	debugC(2, kDebugLevelScripts, "=================");
	debugC(2, kDebugLevelScripts, "runLogic(%d)", n);

	sp.script = n;
	sp.curIP = 0;
	_game.execStack.push_back(sp);

	// If logic not loaded, load it
	if (~_game.dirLogic[n].flags & RES_LOADED) {
		debugC(4, kDebugLevelScripts, "logic %d not loaded!", n);
		agiLoadResource(rLOGIC, n);
	}

	_game.lognum = n;
	_game._curLogic = &_game.logics[_game.lognum];

	_game._curLogic->cIP = _game._curLogic->sIP;

	_timerHack = 0;
	while (ip < _game.logics[n].size && !(shouldQuit() || _restartGame)) {
		if (_debug.enabled) {
			if (_debug.steps > 0) {
				if (_debug.logic0 || n) {
					debugConsole(n, lCOMMAND_MODE, NULL);
					_debug.steps--;
				}
			} else {
				_sprites->blitBoth();
				_sprites->commitBoth();
				do {
					mainCycle();
				} while (!_debug.steps && _debug.enabled);
				_sprites->eraseBoth();
			}
		}

		_game.execStack.back().curIP = ip;

		char st[101];
		int sz = MIN(_game.execStack.size(), 100u);
		memset(st, '.', sz);
		st[sz] = 0;

		switch (op = *(code + ip++)) {
		case 0xff:	// if (open/close)
			testIfCode(n);
			break;
		case 0xfe:	// goto
			// +2 covers goto size
			ip += 2 + ((int16)READ_LE_UINT16(code + ip));

			// timer must keep running even in goto loops,
			// but AGI engine can't do that :(
			if (_timerHack > 20) {
				pollTimer();
				updateTimer();
				_timerHack = 0;
			}
			break;
		case 0x00:	// return
			debugC(2, kDebugLevelScripts, "%sreturn() // Logic %d", st, n);
			debugC(2, kDebugLevelScripts, "=================");

//			if (getVersion() < 0x2000) {
//				if (logic_index < state->max_logics) {
//					n = state->logic_list[++logic_index];
//					state->_curLogic = &state->logics[n];
//					state->lognum = n;
//					ip = 2;
//					warning("running logic %d\n", n);
//					break;
//				}
//				_v[13]=0;
//			}

			_game.execStack.pop_back();
			return 1;
		default:
			num = logicNamesCmd[op].argumentsLength();
			memmove(p, code + ip, num);
			memset(p + num, 0, CMD_BSIZE - num);

			debugC(2, kDebugLevelScripts, "%s%s(%d %d %d)", st, logicNamesCmd[op].name, p[0], p[1], p[2]);

			_agiCommands[op](&_game, p);
			ip += num;
		}

//		if ((op == 0x0B || op == 0x3F || op == 0x40) && logic_index < state->max_logics) {
//			n = state->logic_list[++logic_index];
//			state->_curLogic = &state->logics[n];
//			state->lognum = n;
//			ip = 2;
//			warning("running logic %d\n", n);
//		}

		if (_game.exitAllLogics)
			break;
	}

	_game.execStack.pop_back();

	return 0;		// after executing new.room()
}

void AgiEngine::executeAgiCommand(uint8 op, uint8 *p) {
	debugC(2, kDebugLevelScripts, "%s(%d %d %d)", logicNamesCmd[op].name, p[0], p[1], p[2]);

	_agiCommands[op](&_game, p);
}

} // End of namespace Agi
