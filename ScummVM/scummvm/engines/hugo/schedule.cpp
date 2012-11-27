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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// This module contains all the scheduling and timing stuff

#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "hugo/hugo.h"
#include "hugo/schedule.h"
#include "hugo/file.h"
#include "hugo/display.h"
#include "hugo/util.h"
#include "hugo/object.h"
#include "hugo/sound.h"
#include "hugo/parser.h"
#include "hugo/text.h"
#include "hugo/route.h"
#include "hugo/mouse.h"

namespace Hugo {

Scheduler::Scheduler(HugoEngine *vm) : _vm(vm), _actListArr(0), _curTick(0), _oldTime(0), _refreshTimeout(0), _points(0), _screenActs(0) {
	memset(_events, 0, sizeof(_events));
	_numBonuses = 0;
	_screenActsSize = 0;
}

Scheduler::~Scheduler() {
}

void Scheduler::initCypher() {
	_cypher = getCypher();
}

/**
 * Initialize the timer event queue
 */
void Scheduler::initEventQueue() {
	debugC(1, kDebugSchedule, "initEventQueue");

	// Chain nextEvent from first to last
	for (int i = kMaxEvents; --i;)
		_events[i - 1]._nextEvent = &_events[i];
	_events[kMaxEvents - 1]._nextEvent = 0;

	// Chain prevEvent from last to first
	for (int i = 1; i < kMaxEvents; i++)
		_events[i]._prevEvent = &_events[i - 1];
	_events[0]._prevEvent = 0;

	_headEvent = _tailEvent = 0;                    // Event list is empty
	_freeEvent = _events;                           // Free list is full
}

/**
 * Return a ptr to an event structure from the free list
 */
Event *Scheduler::getQueue() {
	debugC(4, kDebugSchedule, "getQueue");

	if (!_freeEvent)                                // Error: no more events available
		error("An error has occurred: %s", "getQueue");
	Event *resEvent = _freeEvent;
	_freeEvent = _freeEvent->_nextEvent;
	resEvent->_nextEvent = 0;
	return resEvent;
}

/**
 * Call Insert_action for each action in the list supplied
 */
void Scheduler::insertActionList(const uint16 actIndex) {
	debugC(1, kDebugSchedule, "insertActionList(%d)", actIndex);

	if (_actListArr[actIndex]) {
		for (int i = 0; _actListArr[actIndex][i]._a0._actType != ANULL; i++)
			insertAction(&_actListArr[actIndex][i]);
	}
}

/**
 * Return system time in ticks.  A tick is 1/TICKS_PER_SEC mS
 */
uint32 Scheduler::getWinTicks() const {
	debugC(5, kDebugSchedule, "getWinTicks()");

	return _vm->getGameStatus()._tick;
}

/**
 * Return system time in ticks.  A tick is 1/TICKS_PER_SEC mS
 * If update FALSE, simply return last known time
 * Note that this is real time unless a processing cycle takes longer than
 * a real tick, in which case the system tick is simply incremented
 */
uint32 Scheduler::getDosTicks(const bool updateFl) {
	debugC(5, kDebugSchedule, "getDosTicks(%s)", (updateFl) ? "TRUE" : "FALSE");

	uint32 t_now;                                   // Current wall time in ticks

	if (!updateFl)
		return(_curTick);

	if (_oldTime == 0)
		_oldTime = (uint32) floor((double) (g_system->getMillis() * _vm->getTPS() / 1000));
	// Calculate current wall time in ticks
	t_now = g_system->getMillis() * _vm->getTPS() / 1000;

	if ((t_now - _oldTime) > 0) {
		_oldTime = t_now;
		_curTick++;
	}
	return(_curTick);
}

/**
 * Add indecated bonus to score if not added already
 */
void Scheduler::processBonus(const int bonusIndex) {
	debugC(1, kDebugSchedule, "processBonus(%d)", bonusIndex);

	if (!_points[bonusIndex]._scoredFl) {
		_vm->adjustScore(_points[bonusIndex]._score);
		_points[bonusIndex]._scoredFl = true;
	}
}

/**
 * Transition to a new screen as follows:
 * 1. Clear out all non-global events from event list.
 * 2. Set the new screen (in the hero object and any carried objects)
 * 3. Read in the screen files for the new screen
 * 4. Schedule action list for new screen
 * 5. Initialize prompt line and status line
 */
void Scheduler::newScreen(const int screenIndex) {
	debugC(1, kDebugSchedule, "newScreen(%d)", screenIndex);

	// Make sure the background file exists!
	if (!_vm->isPacked()) {
		Common::String filename = Common::String(_vm->_text->getScreenNames(screenIndex));
		if (!Common::File::exists(_vm->_picDir + filename + ".PCX") &&
			!Common::File::exists(filename + ".ART")) {
				error("Unable to find background file for %s", filename.c_str());
			return;
		}
	}

	// 1. Clear out all local events
	Event *curEvent = _headEvent;                 // The earliest event
	Event *wrkEvent;                              // Event ptr
	while (curEvent) {                              // While mature events found
		wrkEvent = curEvent->_nextEvent;             // Save p (becomes undefined after Del)
		if (curEvent->_localActionFl)
			delQueue(curEvent);                     // Return event to free list
		curEvent = wrkEvent;
	}

	// 2. Set the new screen in the hero object and any being carried
	_vm->setNewScreen(screenIndex);

	// 3. Read in new screen files
	_vm->readScreenFiles(screenIndex);

	// 4. Schedule action list for this screen
	_vm->_scheduler->screenActions(screenIndex);

	// 5. Initialize prompt line and status line
	_vm->_screen->initNewScreenDisplay();
}

/**
 * Transition to a new screen as follows:
 * 1. Set the new screen (in the hero object and any carried objects)
 * 2. Read in the screen files for the new screen
 * 3. Initialize prompt line and status line
 */
void Scheduler::restoreScreen(const int screenIndex) {
	debugC(1, kDebugSchedule, "restoreScreen(%d)", screenIndex);

	// 1. Set the new screen in the hero object and any being carried
	_vm->setNewScreen(screenIndex);

	// 2. Read in new screen files
	_vm->readScreenFiles(screenIndex);

	// 3. Initialize prompt line and status line
	_vm->_screen->initNewScreenDisplay();
}

/**
 * Wait (if necessary) for next synchronizing tick
 * Slow machines won't make it by the end of tick, so will just plod on
 * at their own speed, not waiting here, but free running.
 * Note: DOS Versions only
 */
void Scheduler::waitForRefresh() {
	debugC(5, kDebugSchedule, "waitForRefresh()");

	uint32 t;

	if (_refreshTimeout == 0)
		_refreshTimeout = getDosTicks(true);

	while ((t = getDosTicks(true)) < _refreshTimeout)
		;
	_refreshTimeout = ++t;
}

/**
 * Read kALnewscr used by maze (Hugo 2)
 */
void Scheduler::loadAlNewscrIndex(Common::ReadStream &in) {
	debugC(6, kDebugSchedule, "loadAlNewscrIndex(&in)");

	int numElem;
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant)
			_alNewscrIndex = numElem;
	}
}

/**
 * Load Points from Hugo.dat
 */
void Scheduler::loadPoints(Common::SeekableReadStream &in) {
	debugC(6, kDebugSchedule, "loadPoints(&in)");

	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant) {
			_numBonuses = numElem;
			_points = (Point *)malloc(sizeof(Point) * _numBonuses);
			for (int i = 0; i < _numBonuses; i++) {
				_points[i]._score = in.readByte();
				_points[i]._scoredFl = false;
			}
		} else {
			in.skip(numElem);
		}
	}
}

void Scheduler::readAct(Common::ReadStream &in, Act &curAct) {
	uint16 numSubAct;

	curAct._a0._actType = (Action) in.readByte();
	switch (curAct._a0._actType) {
	case ANULL:              // -1
		break;
	case ASCHEDULE:          // 0
		curAct._a0._timer = in.readSint16BE();
		curAct._a0._actIndex = in.readUint16BE();
		break;
	case START_OBJ:          // 1
		curAct._a1._timer = in.readSint16BE();
		curAct._a1._objIndex = in.readSint16BE();
		curAct._a1._cycleNumb = in.readSint16BE();
		curAct._a1._cycle = (Cycle) in.readByte();
		break;
	case INIT_OBJXY:         // 2
		curAct._a2._timer = in.readSint16BE();
		curAct._a2._objIndex = in.readSint16BE();
		curAct._a2._x = in.readSint16BE();
		curAct._a2._y = in.readSint16BE();
		break;
	case PROMPT:             // 3
		curAct._a3._timer = in.readSint16BE();
		curAct._a3._promptIndex = in.readSint16BE();
		numSubAct = in.readUint16BE();
		curAct._a3._responsePtr = (int *)malloc(sizeof(int) * numSubAct);
		for (int k = 0; k < numSubAct; k++)
			curAct._a3._responsePtr[k] = in.readSint16BE();
		curAct._a3._actPassIndex = in.readUint16BE();
		curAct._a3._actFailIndex = in.readUint16BE();
		curAct._a3._encodedFl = (in.readByte() == 1) ? true : false;
		break;
	case BKGD_COLOR:         // 4
		curAct._a4._timer = in.readSint16BE();
		curAct._a4._newBackgroundColor = in.readUint32BE();
		break;
	case INIT_OBJVXY:        // 5
		curAct._a5._timer = in.readSint16BE();
		curAct._a5._objIndex = in.readSint16BE();
		curAct._a5._vx = in.readSint16BE();
		curAct._a5._vy = in.readSint16BE();
		break;
	case INIT_CARRY:         // 6
		curAct._a6._timer = in.readSint16BE();
		curAct._a6._objIndex = in.readSint16BE();
		curAct._a6._carriedFl = (in.readByte() == 1) ? true : false;
		break;
	case INIT_HF_COORD:      // 7
		curAct._a7._timer = in.readSint16BE();
		curAct._a7._objIndex = in.readSint16BE();
		break;
	case NEW_SCREEN:         // 8
		curAct._a8._timer = in.readSint16BE();
		curAct._a8._screenIndex = in.readSint16BE();
		break;
	case INIT_OBJSTATE:      // 9
		curAct._a9._timer = in.readSint16BE();
		curAct._a9._objIndex = in.readSint16BE();
		curAct._a9._newState = in.readByte();
		break;
	case INIT_PATH:          // 10
		curAct._a10._timer = in.readSint16BE();
		curAct._a10._objIndex = in.readSint16BE();
		curAct._a10._newPathType = in.readSint16BE();
		curAct._a10._vxPath = in.readByte();
		curAct._a10._vyPath = in.readByte();
		break;
	case COND_R:             // 11
		curAct._a11._timer = in.readSint16BE();
		curAct._a11._objIndex = in.readSint16BE();
		curAct._a11._stateReq = in.readByte();
		curAct._a11._actPassIndex = in.readUint16BE();
		curAct._a11._actFailIndex = in.readUint16BE();
		break;
	case TEXT:               // 12
		curAct._a12._timer = in.readSint16BE();
		curAct._a12._stringIndex = in.readSint16BE();
		break;
	case SWAP_IMAGES:        // 13
		curAct._a13._timer = in.readSint16BE();
		curAct._a13._objIndex1 = in.readSint16BE();
		curAct._a13._objIndex2 = in.readSint16BE();
		break;
	case COND_SCR:           // 14
		curAct._a14._timer = in.readSint16BE();
		curAct._a14._objIndex = in.readSint16BE();
		curAct._a14._screenReq = in.readSint16BE();
		curAct._a14._actPassIndex = in.readUint16BE();
		curAct._a14._actFailIndex = in.readUint16BE();
		break;
	case AUTOPILOT:          // 15
		curAct._a15._timer = in.readSint16BE();
		curAct._a15._objIndex1 = in.readSint16BE();
		curAct._a15._objIndex2 = in.readSint16BE();
		curAct._a15._dx = in.readByte();
		curAct._a15._dy = in.readByte();
		break;
	case INIT_OBJ_SEQ:       // 16
		curAct._a16._timer = in.readSint16BE();
		curAct._a16._objIndex = in.readSint16BE();
		curAct._a16._seqIndex = in.readSint16BE();
		break;
	case SET_STATE_BITS:     // 17
		curAct._a17._timer = in.readSint16BE();
		curAct._a17._objIndex = in.readSint16BE();
		curAct._a17._stateMask = in.readSint16BE();
		break;
	case CLEAR_STATE_BITS:   // 18
		curAct._a18._timer = in.readSint16BE();
		curAct._a18._objIndex = in.readSint16BE();
		curAct._a18._stateMask = in.readSint16BE();
		break;
	case TEST_STATE_BITS:    // 19
		curAct._a19._timer = in.readSint16BE();
		curAct._a19._objIndex = in.readSint16BE();
		curAct._a19._stateMask = in.readSint16BE();
		curAct._a19._actPassIndex = in.readUint16BE();
		curAct._a19._actFailIndex = in.readUint16BE();
		break;
	case DEL_EVENTS:         // 20
		curAct._a20._timer = in.readSint16BE();
		curAct._a20._actTypeDel = (Action) in.readByte();
		break;
	case GAMEOVER:           // 21
		curAct._a21._timer = in.readSint16BE();
		break;
	case INIT_HH_COORD:      // 22
		curAct._a22._timer = in.readSint16BE();
		curAct._a22._objIndex = in.readSint16BE();
		break;
	case EXIT:               // 23
		curAct._a23._timer = in.readSint16BE();
		break;
	case BONUS:              // 24
		curAct._a24._timer = in.readSint16BE();
		curAct._a24._pointIndex = in.readSint16BE();
		break;
	case COND_BOX:           // 25
		curAct._a25._timer = in.readSint16BE();
		curAct._a25._objIndex = in.readSint16BE();
		curAct._a25._x1 = in.readSint16BE();
		curAct._a25._y1 = in.readSint16BE();
		curAct._a25._x2 = in.readSint16BE();
		curAct._a25._y2 = in.readSint16BE();
		curAct._a25._actPassIndex = in.readUint16BE();
		curAct._a25._actFailIndex = in.readUint16BE();
		break;
	case SOUND:              // 26
		curAct._a26._timer = in.readSint16BE();
		curAct._a26._soundIndex = in.readSint16BE();
		break;
	case ADD_SCORE:          // 27
		curAct._a27._timer = in.readSint16BE();
		curAct._a27._objIndex = in.readSint16BE();
		break;
	case SUB_SCORE:          // 28
		curAct._a28._timer = in.readSint16BE();
		curAct._a28._objIndex = in.readSint16BE();
		break;
	case COND_CARRY:         // 29
		curAct._a29._timer = in.readSint16BE();
		curAct._a29._objIndex = in.readSint16BE();
		curAct._a29._actPassIndex = in.readUint16BE();
		curAct._a29._actFailIndex = in.readUint16BE();
		break;
	case INIT_MAZE:          // 30
		curAct._a30._timer = in.readSint16BE();
		curAct._a30._mazeSize = in.readByte();
		curAct._a30._x1 = in.readSint16BE();
		curAct._a30._y1 = in.readSint16BE();
		curAct._a30._x2 = in.readSint16BE();
		curAct._a30._y2 = in.readSint16BE();
		curAct._a30._x3 = in.readSint16BE();
		curAct._a30._x4 = in.readSint16BE();
		curAct._a30._firstScreenIndex = in.readByte();
		break;
	case EXIT_MAZE:          // 31
		curAct._a31._timer = in.readSint16BE();
		break;
	case INIT_PRIORITY:      // 32
		curAct._a32._timer = in.readSint16BE();
		curAct._a32._objIndex = in.readSint16BE();
		curAct._a32._priority = in.readByte();
		break;
	case INIT_SCREEN:        // 33
		curAct._a33._timer = in.readSint16BE();
		curAct._a33._objIndex = in.readSint16BE();
		curAct._a33._screenIndex = in.readSint16BE();
		break;
	case AGSCHEDULE:         // 34
		curAct._a34._timer = in.readSint16BE();
		curAct._a34._actIndex = in.readUint16BE();
		break;
	case REMAPPAL:           // 35
		curAct._a35._timer = in.readSint16BE();
		curAct._a35._oldColorIndex = in.readSint16BE();
		curAct._a35._newColorIndex = in.readSint16BE();
		break;
	case COND_NOUN:          // 36
		curAct._a36._timer = in.readSint16BE();
		curAct._a36._nounIndex = in.readUint16BE();
		curAct._a36._actPassIndex = in.readUint16BE();
		curAct._a36._actFailIndex = in.readUint16BE();
		break;
	case SCREEN_STATE:       // 37
		curAct._a37._timer = in.readSint16BE();
		curAct._a37._screenIndex = in.readSint16BE();
		curAct._a37._newState = in.readByte();
		break;
	case INIT_LIPS:          // 38
		curAct._a38._timer = in.readSint16BE();
		curAct._a38._lipsObjIndex = in.readSint16BE();
		curAct._a38._objIndex = in.readSint16BE();
		curAct._a38._dxLips = in.readByte();
		curAct._a38._dyLips = in.readByte();
		break;
	case INIT_STORY_MODE:    // 39
		curAct._a39._timer = in.readSint16BE();
		curAct._a39._storyModeFl = (in.readByte() == 1);
		break;
	case WARN:               // 40
		curAct._a40._timer = in.readSint16BE();
		curAct._a40._stringIndex = in.readSint16BE();
		break;
	case COND_BONUS:         // 41
		curAct._a41._timer = in.readSint16BE();
		curAct._a41._bonusIndex = in.readSint16BE();
		curAct._a41._actPassIndex = in.readUint16BE();
		curAct._a41._actFailIndex = in.readUint16BE();
		break;
	case TEXT_TAKE:          // 42
		curAct._a42._timer = in.readSint16BE();
		curAct._a42._objIndex = in.readSint16BE();
		break;
	case YESNO:              // 43
		curAct._a43._timer = in.readSint16BE();
		curAct._a43._promptIndex = in.readSint16BE();
		curAct._a43._actYesIndex = in.readUint16BE();
		curAct._a43._actNoIndex = in.readUint16BE();
		break;
	case STOP_ROUTE:         // 44
		curAct._a44._timer = in.readSint16BE();
		break;
	case COND_ROUTE:         // 45
		curAct._a45._timer = in.readSint16BE();
		curAct._a45._routeIndex = in.readSint16BE();
		curAct._a45._actPassIndex = in.readUint16BE();
		curAct._a45._actFailIndex = in.readUint16BE();
		break;
	case INIT_JUMPEXIT:      // 46
		curAct._a46._timer = in.readSint16BE();
		curAct._a46._jumpExitFl = (in.readByte() == 1);
		break;
	case INIT_VIEW:          // 47
		curAct._a47._timer = in.readSint16BE();
		curAct._a47._objIndex = in.readSint16BE();
		curAct._a47._viewx = in.readSint16BE();
		curAct._a47._viewy = in.readSint16BE();
		curAct._a47._direction = in.readSint16BE();
		break;
	case INIT_OBJ_FRAME:     // 48
		curAct._a48._timer = in.readSint16BE();
		curAct._a48._objIndex = in.readSint16BE();
		curAct._a48._seqIndex = in.readSint16BE();
		curAct._a48._frameIndex = in.readSint16BE();
		break;
	case OLD_SONG:           //49
		curAct._a49._timer = in.readSint16BE();
		curAct._a49._songIndex = in.readUint16BE();
		break;
	default:
		error("Engine - Unknown action type encountered: %d", curAct._a0._actType);
	}
}

/**
 * Load actListArr from Hugo.dat
 */
void Scheduler::loadActListArr(Common::ReadStream &in) {
	debugC(6, kDebugSchedule, "loadActListArr(&in)");

	Act tmpAct;

	int numElem, numSubElem;
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant) {
			_actListArrSize = numElem;
			_actListArr = (Act **)malloc(sizeof(Act *) * _actListArrSize);
		}

		for (int i = 0; i < numElem; i++) {
			numSubElem = in.readUint16BE();
			if (varnt == _vm->_gameVariant)
				_actListArr[i] = (Act *)malloc(sizeof(Act) * (numSubElem + 1));
			for (int j = 0; j < numSubElem; j++) {
				if (varnt == _vm->_gameVariant) {
					readAct(in, _actListArr[i][j]);
				} else {
					readAct(in, tmpAct);
					if (tmpAct._a0._actType == PROMPT)
						free(tmpAct._a3._responsePtr);
				}
			}

			if (varnt == _vm->_gameVariant)
				_actListArr[i][numSubElem]._a0._actType = ANULL;
		}
	}
}


/**
 * Read _screenActs
 */
void Scheduler::loadScreenAct(Common::SeekableReadStream &in) {
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numElem = in.readUint16BE();

		if (varnt == _vm->_gameVariant) {
			_screenActsSize = numElem;
			_screenActs = (uint16 **)malloc(sizeof(uint16 *) * numElem);
			for (int i = 0; i < numElem; i++) {
				uint16 numSubElem = in.readUint16BE();
				if (numSubElem == 0) {
					_screenActs[i] = 0;
				} else {
					_screenActs[i] = (uint16 *)malloc(sizeof(uint16) * numSubElem);
					for (int j = 0; j < numSubElem; j++)
						_screenActs[i][j] = in.readUint16BE();
				}
			}
		} else {
			for (int i = 0; i < numElem; i++) {
				uint16 numSubElem = in.readUint16BE();
				in.skip(numSubElem * sizeof(uint16));
			}
		}
	}
}

void Scheduler::freeScheduler() {
	debugC(6, kDebugSchedule, "freeActListArr()");

	free(_points);

	if (_screenActs) {
		for (int i = 0; i < _screenActsSize; i++)
			free(_screenActs[i]);
		free(_screenActs);
	}

	if (_actListArr) {
		for (int i = 0; i < _actListArrSize; i++) {
			for (int j = 0; _actListArr[i][j]._a0._actType != ANULL; j++) {
				if (_actListArr[i][j]._a0._actType == PROMPT)
					free(_actListArr[i][j]._a3._responsePtr);
			}
			free(_actListArr[i]);
		}
		free(_actListArr);
	}
}

/**
 * Add action lists for this screen to event queue
 */
void Scheduler::screenActions(const int screenNum) {
	debugC(1, kDebugEngine, "screenActions(%d)", screenNum);

	uint16 *screenAct = _screenActs[screenNum];
	if (screenAct) {
		for (int i = 0; screenAct[i]; i++)
			insertActionList(screenAct[i]);
	}
}

/**
 * Maze mode is enabled.  Check to see whether hero has crossed the maze
 * bounding box, if so, go to the next room
 */
void Scheduler::processMaze(const int x1, const int x2, const int y1, const int y2) {
	debugC(1, kDebugSchedule, "processMaze");

	if (x1 < _vm->_maze._x1) {
		// Exit west
		_actListArr[_alNewscrIndex][3]._a8._screenIndex = *_vm->_screenPtr - 1;
		_actListArr[_alNewscrIndex][0]._a2._x = _vm->_maze._x2 - kShiftSize - (x2 - x1);
		_actListArr[_alNewscrIndex][0]._a2._y = _vm->_hero->_y;
		_vm->_route->resetRoute();
		insertActionList(_alNewscrIndex);
	} else if (x2 > _vm->_maze._x2) {
		// Exit east
		_actListArr[_alNewscrIndex][3]._a8._screenIndex = *_vm->_screenPtr + 1;
		_actListArr[_alNewscrIndex][0]._a2._x = _vm->_maze._x1 + kShiftSize;
		_actListArr[_alNewscrIndex][0]._a2._y = _vm->_hero->_y;
		_vm->_route->resetRoute();
		insertActionList(_alNewscrIndex);
	} else if (y1 < _vm->_maze._y1 - kShiftSize) {
		// Exit north
		_actListArr[_alNewscrIndex][3]._a8._screenIndex = *_vm->_screenPtr - _vm->_maze._size;
		_actListArr[_alNewscrIndex][0]._a2._x = _vm->_maze._x3;
		_actListArr[_alNewscrIndex][0]._a2._y = _vm->_maze._y2 - kShiftSize - (y2 - y1);
		_vm->_route->resetRoute();
		insertActionList(_alNewscrIndex);
	} else if (y2 > _vm->_maze._y2 - kShiftSize / 2) {
		// Exit south
		_actListArr[_alNewscrIndex][3]._a8._screenIndex = *_vm->_screenPtr + _vm->_maze._size;
		_actListArr[_alNewscrIndex][0]._a2._x = _vm->_maze._x4;
		_actListArr[_alNewscrIndex][0]._a2._y = _vm->_maze._y1 + kShiftSize;
		_vm->_route->resetRoute();
		insertActionList(_alNewscrIndex);
	}
}

/**
 * Write the event queue to the file with handle f
 * Note that we convert all the event structure ptrs to indexes
 * using -1 for NULL.  We can't convert the action ptrs to indexes
 * so we save address of first dummy action ptr to compare on restore.
 */
void Scheduler::saveEvents(Common::WriteStream *f) {
	debugC(1, kDebugSchedule, "saveEvents()");

	f->writeUint32BE(getTicks());

	int16 freeIndex = (_freeEvent == 0) ? -1 : _freeEvent - _events;
	int16 headIndex = (_headEvent == 0) ? -1 : _headEvent - _events;
	int16 tailIndex = (_tailEvent == 0) ? -1 : _tailEvent - _events;

	f->writeSint16BE(freeIndex);
	f->writeSint16BE(headIndex);
	f->writeSint16BE(tailIndex);

	// Convert event ptrs to indexes
	for (int16 i = 0; i < kMaxEvents; i++) {
		Event *wrkEvent = &_events[i];

		// fix up action pointer (to do better)
		int16 index, subElem;
		findAction(wrkEvent->_action, &index, &subElem);
		f->writeSint16BE(index);
		f->writeSint16BE(subElem);
		f->writeByte((wrkEvent->_localActionFl) ? 1 : 0);
		f->writeUint32BE(wrkEvent->_time);
		f->writeSint16BE((wrkEvent->_prevEvent == 0) ? -1 : (wrkEvent->_prevEvent - _events));
		f->writeSint16BE((wrkEvent->_nextEvent == 0) ? -1 : (wrkEvent->_nextEvent - _events));
	}
}

/**
 * Restore the action data from file with handle f
 */

void Scheduler::restoreActions(Common::ReadStream *f) {
	for (int i = 0; i < _actListArrSize; i++) {
		uint16 numSubElem = f->readUint16BE();
		for (int j = 0; j < numSubElem; j++) {
			readAct(*f, _actListArr[i][j]);
		}
	}
}

int16 Scheduler::calcMaxPoints() const {
	int16 tmpScore = 0;
	for (int i = 0; i < _numBonuses; i++)
		tmpScore += _points[i]._score;
	return tmpScore;
}

/*
* Save the action data in the file with handle f
*/
void Scheduler::saveActions(Common::WriteStream *f) const {
	byte  subElemType;
	int16 nbrCpt;
	uint16 nbrSubElem;

	for (int i = 0; i < _actListArrSize; i++) {
		// write all the sub elems data
		for (nbrSubElem = 1; _actListArr[i][nbrSubElem - 1]._a0._actType != ANULL; nbrSubElem++)
			;

		f->writeUint16BE(nbrSubElem);
		for (int j = 0; j < nbrSubElem; j++) {
			subElemType = _actListArr[i][j]._a0._actType;
			f->writeByte(subElemType);
			switch (subElemType) {
			case ANULL:              // -1
				break;
			case ASCHEDULE:          // 0
				f->writeSint16BE(_actListArr[i][j]._a0._timer);
				f->writeUint16BE(_actListArr[i][j]._a0._actIndex);
				break;
			case START_OBJ:          // 1
				f->writeSint16BE(_actListArr[i][j]._a1._timer);
				f->writeSint16BE(_actListArr[i][j]._a1._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a1._cycleNumb);
				f->writeByte(_actListArr[i][j]._a1._cycle);
				break;
			case INIT_OBJXY:         // 2
				f->writeSint16BE(_actListArr[i][j]._a2._timer);
				f->writeSint16BE(_actListArr[i][j]._a2._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a2._x);
				f->writeSint16BE(_actListArr[i][j]._a2._y);
				break;
			case PROMPT:             // 3
				f->writeSint16BE(_actListArr[i][j]._a3._timer);
				f->writeSint16BE(_actListArr[i][j]._a3._promptIndex);
				for (nbrCpt = 0; _actListArr[i][j]._a3._responsePtr[nbrCpt] != -1; nbrCpt++)
					;
				nbrCpt++;
				f->writeUint16BE(nbrCpt);
				for (int k = 0; k < nbrCpt; k++)
					f->writeSint16BE(_actListArr[i][j]._a3._responsePtr[k]);
				f->writeUint16BE(_actListArr[i][j]._a3._actPassIndex);
				f->writeUint16BE(_actListArr[i][j]._a3._actFailIndex);
				f->writeByte((_actListArr[i][j]._a3._encodedFl) ? 1 : 0);
				break;
			case BKGD_COLOR:         // 4
				f->writeSint16BE(_actListArr[i][j]._a4._timer);
				f->writeUint32BE(_actListArr[i][j]._a4._newBackgroundColor);
				break;
			case INIT_OBJVXY:        // 5
				f->writeSint16BE(_actListArr[i][j]._a5._timer);
				f->writeSint16BE(_actListArr[i][j]._a5._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a5._vx);
				f->writeSint16BE(_actListArr[i][j]._a5._vy);
				break;
			case INIT_CARRY:         // 6
				f->writeSint16BE(_actListArr[i][j]._a6._timer);
				f->writeSint16BE(_actListArr[i][j]._a6._objIndex);
				f->writeByte((_actListArr[i][j]._a6._carriedFl) ? 1 : 0);
				break;
			case INIT_HF_COORD:      // 7
				f->writeSint16BE(_actListArr[i][j]._a7._timer);
				f->writeSint16BE(_actListArr[i][j]._a7._objIndex);
				break;
			case NEW_SCREEN:         // 8
				f->writeSint16BE(_actListArr[i][j]._a8._timer);
				f->writeSint16BE(_actListArr[i][j]._a8._screenIndex);
				break;
			case INIT_OBJSTATE:      // 9
				f->writeSint16BE(_actListArr[i][j]._a9._timer);
				f->writeSint16BE(_actListArr[i][j]._a9._objIndex);
				f->writeByte(_actListArr[i][j]._a9._newState);
				break;
			case INIT_PATH:          // 10
				f->writeSint16BE(_actListArr[i][j]._a10._timer);
				f->writeSint16BE(_actListArr[i][j]._a10._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a10._newPathType);
				f->writeByte(_actListArr[i][j]._a10._vxPath);
				f->writeByte(_actListArr[i][j]._a10._vyPath);
				break;
			case COND_R:             // 11
				f->writeSint16BE(_actListArr[i][j]._a11._timer);
				f->writeSint16BE(_actListArr[i][j]._a11._objIndex);
				f->writeByte(_actListArr[i][j]._a11._stateReq);
				f->writeUint16BE(_actListArr[i][j]._a11._actPassIndex);
				f->writeUint16BE(_actListArr[i][j]._a11._actFailIndex);
				break;
			case TEXT:               // 12
				f->writeSint16BE(_actListArr[i][j]._a12._timer);
				f->writeSint16BE(_actListArr[i][j]._a12._stringIndex);
				break;
			case SWAP_IMAGES:        // 13
				f->writeSint16BE(_actListArr[i][j]._a13._timer);
				f->writeSint16BE(_actListArr[i][j]._a13._objIndex1);
				f->writeSint16BE(_actListArr[i][j]._a13._objIndex2);
				break;
			case COND_SCR:           // 14
				f->writeSint16BE(_actListArr[i][j]._a14._timer);
				f->writeSint16BE(_actListArr[i][j]._a14._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a14._screenReq);
				f->writeUint16BE(_actListArr[i][j]._a14._actPassIndex);
				f->writeUint16BE(_actListArr[i][j]._a14._actFailIndex);
				break;
			case AUTOPILOT:          // 15
				f->writeSint16BE(_actListArr[i][j]._a15._timer);
				f->writeSint16BE(_actListArr[i][j]._a15._objIndex1);
				f->writeSint16BE(_actListArr[i][j]._a15._objIndex2);
				f->writeByte(_actListArr[i][j]._a15._dx);
				f->writeByte(_actListArr[i][j]._a15._dy);
				break;
			case INIT_OBJ_SEQ:       // 16
				f->writeSint16BE(_actListArr[i][j]._a16._timer);
				f->writeSint16BE(_actListArr[i][j]._a16._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a16._seqIndex);
				break;
			case SET_STATE_BITS:     // 17
				f->writeSint16BE(_actListArr[i][j]._a17._timer);
				f->writeSint16BE(_actListArr[i][j]._a17._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a17._stateMask);
				break;
			case CLEAR_STATE_BITS:   // 18
				f->writeSint16BE(_actListArr[i][j]._a18._timer);
				f->writeSint16BE(_actListArr[i][j]._a18._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a18._stateMask);
				break;
			case TEST_STATE_BITS:    // 19
				f->writeSint16BE(_actListArr[i][j]._a19._timer);
				f->writeSint16BE(_actListArr[i][j]._a19._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a19._stateMask);
				f->writeUint16BE(_actListArr[i][j]._a19._actPassIndex);
				f->writeUint16BE(_actListArr[i][j]._a19._actFailIndex);
				break;
			case DEL_EVENTS:         // 20
				f->writeSint16BE(_actListArr[i][j]._a20._timer);
				f->writeByte(_actListArr[i][j]._a20._actTypeDel);
				break;
			case GAMEOVER:           // 21
				f->writeSint16BE(_actListArr[i][j]._a21._timer);
				break;
			case INIT_HH_COORD:      // 22
				f->writeSint16BE(_actListArr[i][j]._a22._timer);
				f->writeSint16BE(_actListArr[i][j]._a22._objIndex);
				break;
			case EXIT:               // 23
				f->writeSint16BE(_actListArr[i][j]._a23._timer);
				break;
			case BONUS:              // 24
				f->writeSint16BE(_actListArr[i][j]._a24._timer);
				f->writeSint16BE(_actListArr[i][j]._a24._pointIndex);
				break;
			case COND_BOX:           // 25
				f->writeSint16BE(_actListArr[i][j]._a25._timer);
				f->writeSint16BE(_actListArr[i][j]._a25._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a25._x1);
				f->writeSint16BE(_actListArr[i][j]._a25._y1);
				f->writeSint16BE(_actListArr[i][j]._a25._x2);
				f->writeSint16BE(_actListArr[i][j]._a25._y2);
				f->writeUint16BE(_actListArr[i][j]._a25._actPassIndex);
				f->writeUint16BE(_actListArr[i][j]._a25._actFailIndex);
				break;
			case SOUND:              // 26
				f->writeSint16BE(_actListArr[i][j]._a26._timer);
				f->writeSint16BE(_actListArr[i][j]._a26._soundIndex);
				break;
			case ADD_SCORE:          // 27
				f->writeSint16BE(_actListArr[i][j]._a27._timer);
				f->writeSint16BE(_actListArr[i][j]._a27._objIndex);
				break;
			case SUB_SCORE:          // 28
				f->writeSint16BE(_actListArr[i][j]._a28._timer);
				f->writeSint16BE(_actListArr[i][j]._a28._objIndex);
				break;
			case COND_CARRY:         // 29
				f->writeSint16BE(_actListArr[i][j]._a29._timer);
				f->writeSint16BE(_actListArr[i][j]._a29._objIndex);
				f->writeUint16BE(_actListArr[i][j]._a29._actPassIndex);
				f->writeUint16BE(_actListArr[i][j]._a29._actFailIndex);
				break;
			case INIT_MAZE:          // 30
				f->writeSint16BE(_actListArr[i][j]._a30._timer);
				f->writeByte(_actListArr[i][j]._a30._mazeSize);
				f->writeSint16BE(_actListArr[i][j]._a30._x1);
				f->writeSint16BE(_actListArr[i][j]._a30._y1);
				f->writeSint16BE(_actListArr[i][j]._a30._x2);
				f->writeSint16BE(_actListArr[i][j]._a30._y2);
				f->writeSint16BE(_actListArr[i][j]._a30._x3);
				f->writeSint16BE(_actListArr[i][j]._a30._x4);
				f->writeByte(_actListArr[i][j]._a30._firstScreenIndex);
				break;
			case EXIT_MAZE:          // 31
				f->writeSint16BE(_actListArr[i][j]._a31._timer);
				break;
			case INIT_PRIORITY:      // 32
				f->writeSint16BE(_actListArr[i][j]._a32._timer);
				f->writeSint16BE(_actListArr[i][j]._a32._objIndex);
				f->writeByte(_actListArr[i][j]._a32._priority);
				break;
			case INIT_SCREEN:        // 33
				f->writeSint16BE(_actListArr[i][j]._a33._timer);
				f->writeSint16BE(_actListArr[i][j]._a33._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a33._screenIndex);
				break;
			case AGSCHEDULE:         // 34
				f->writeSint16BE(_actListArr[i][j]._a34._timer);
				f->writeUint16BE(_actListArr[i][j]._a34._actIndex);
				break;
			case REMAPPAL:           // 35
				f->writeSint16BE(_actListArr[i][j]._a35._timer);
				f->writeSint16BE(_actListArr[i][j]._a35._oldColorIndex);
				f->writeSint16BE(_actListArr[i][j]._a35._newColorIndex);
				break;
			case COND_NOUN:          // 36
				f->writeSint16BE(_actListArr[i][j]._a36._timer);
				f->writeUint16BE(_actListArr[i][j]._a36._nounIndex);
				f->writeUint16BE(_actListArr[i][j]._a36._actPassIndex);
				f->writeUint16BE(_actListArr[i][j]._a36._actFailIndex);
				break;
			case SCREEN_STATE:       // 37
				f->writeSint16BE(_actListArr[i][j]._a37._timer);
				f->writeSint16BE(_actListArr[i][j]._a37._screenIndex);
				f->writeByte(_actListArr[i][j]._a37._newState);
				break;
			case INIT_LIPS:          // 38
				f->writeSint16BE(_actListArr[i][j]._a38._timer);
				f->writeSint16BE(_actListArr[i][j]._a38._lipsObjIndex);
				f->writeSint16BE(_actListArr[i][j]._a38._objIndex);
				f->writeByte(_actListArr[i][j]._a38._dxLips);
				f->writeByte(_actListArr[i][j]._a38._dyLips);
				break;
			case INIT_STORY_MODE:    // 39
				f->writeSint16BE(_actListArr[i][j]._a39._timer);
				f->writeByte((_actListArr[i][j]._a39._storyModeFl) ? 1 : 0);
				break;
			case WARN:               // 40
				f->writeSint16BE(_actListArr[i][j]._a40._timer);
				f->writeSint16BE(_actListArr[i][j]._a40._stringIndex);
				break;
			case COND_BONUS:         // 41
				f->writeSint16BE(_actListArr[i][j]._a41._timer);
				f->writeSint16BE(_actListArr[i][j]._a41._bonusIndex);
				f->writeUint16BE(_actListArr[i][j]._a41._actPassIndex);
				f->writeUint16BE(_actListArr[i][j]._a41._actFailIndex);
				break;
			case TEXT_TAKE:          // 42
				f->writeSint16BE(_actListArr[i][j]._a42._timer);
				f->writeSint16BE(_actListArr[i][j]._a42._objIndex);
				break;
			case YESNO:              // 43
				f->writeSint16BE(_actListArr[i][j]._a43._timer);
				f->writeSint16BE(_actListArr[i][j]._a43._promptIndex);
				f->writeUint16BE(_actListArr[i][j]._a43._actYesIndex);
				f->writeUint16BE(_actListArr[i][j]._a43._actNoIndex);
				break;
			case STOP_ROUTE:         // 44
				f->writeSint16BE(_actListArr[i][j]._a44._timer);
				break;
			case COND_ROUTE:         // 45
				f->writeSint16BE(_actListArr[i][j]._a45._timer);
				f->writeSint16BE(_actListArr[i][j]._a45._routeIndex);
				f->writeUint16BE(_actListArr[i][j]._a45._actPassIndex);
				f->writeUint16BE(_actListArr[i][j]._a45._actFailIndex);
				break;
			case INIT_JUMPEXIT:      // 46
				f->writeSint16BE(_actListArr[i][j]._a46._timer);
				f->writeByte((_actListArr[i][j]._a46._jumpExitFl) ? 1 : 0);
				break;
			case INIT_VIEW:          // 47
				f->writeSint16BE(_actListArr[i][j]._a47._timer);
				f->writeSint16BE(_actListArr[i][j]._a47._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a47._viewx);
				f->writeSint16BE(_actListArr[i][j]._a47._viewy);
				f->writeSint16BE(_actListArr[i][j]._a47._direction);
				break;
			case INIT_OBJ_FRAME:     // 48
				f->writeSint16BE(_actListArr[i][j]._a48._timer);
				f->writeSint16BE(_actListArr[i][j]._a48._objIndex);
				f->writeSint16BE(_actListArr[i][j]._a48._seqIndex);
				f->writeSint16BE(_actListArr[i][j]._a48._frameIndex);
				break;
			case OLD_SONG:           // 49, Added by Strangerke for DOS versions
				f->writeSint16BE(_actListArr[i][j]._a49._timer);
				f->writeUint16BE(_actListArr[i][j]._a49._songIndex);
				break;
			default:
				error("Unknown action %d", subElemType);
			}
		}
	}
}

/*
* Find the index in the action list to be able to serialize the action to save game
*/
void Scheduler::findAction(const Act *action, int16 *index, int16 *subElem) {

	assert(index && subElem);
	if (!action) {
		*index = -1;
		*subElem = -1;
		return;
	}

	for (int i = 0; i < _actListArrSize; i++) {
		int j = 0;
		do {
			if (action == &_actListArr[i][j]) {
				*index = i;
				*subElem = j;
				return;
			}
			j++;
		} while (_actListArr[i][j-1]._a0._actType != ANULL);
	}
	// action not found ??
	assert(0);
}

void Scheduler::saveSchedulerData(Common::WriteStream *out) {
	savePoints(out);

	// Now save current time and all current events in event queue
	saveEvents(out);

	// Now save current actions
	saveActions(out);
}

void Scheduler::restoreSchedulerData(Common::ReadStream *in) {
	restorePoints(in);
	_vm->_object->restoreAllSeq();

	// Now restore time of the save and the event queue
	restoreEvents(in);

	// Now restore actions
	restoreActions(in);
}

/**
 * Restore the event list from file with handle f
 */
void Scheduler::restoreEvents(Common::ReadStream *f) {
	debugC(1, kDebugSchedule, "restoreEvents");

	uint32 saveTime = f->readUint32BE();              // time of save
	int16 freeIndex = f->readSint16BE();              // Free list index
	int16 headIndex = f->readSint16BE();              // Head of list index
	int16 tailIndex = f->readSint16BE();              // Tail of list index

	// Restore events indexes to pointers
	for (int i = 0; i < kMaxEvents; i++) {
		int16 index = f->readSint16BE();
		int16 subElem = f->readSint16BE();

		// fix up action pointer (to do better)
		if ((index == -1) && (subElem == -1))
			_events[i]._action = 0;
		else
			_events[i]._action = (Act *)&_actListArr[index][subElem];

		_events[i]._localActionFl = (f->readByte() == 1) ? true : false;
		_events[i]._time = f->readUint32BE();

		int16 prevIndex = f->readSint16BE();
		int16 nextIndex = f->readSint16BE();

		_events[i]._prevEvent = (prevIndex == -1) ? (Event *)0 : &_events[prevIndex];
		_events[i]._nextEvent = (nextIndex == -1) ? (Event *)0 : &_events[nextIndex];
	}
	_freeEvent = (freeIndex == -1) ? 0 : &_events[freeIndex];
	_headEvent = (headIndex == -1) ? 0 : &_events[headIndex];
	_tailEvent = (tailIndex == -1) ? 0 : &_events[tailIndex];

	// Adjust times to fit our time
	uint32 curTime = getTicks();
	Event *wrkEvent = _headEvent;                     // The earliest event
	while (wrkEvent) {                                // While mature events found
		wrkEvent->_time = wrkEvent->_time - saveTime + curTime;
		wrkEvent = wrkEvent->_nextEvent;
	}
}

/**
 * Insert the action pointed to by p into the timer event queue
 * The queue goes from head (earliest) to tail (latest) timewise
 */
void Scheduler::insertAction(Act *action) {
	debugC(1, kDebugSchedule, "insertAction() - Action type A%d", action->_a0._actType);

	// First, get and initialize the event structure
	Event *curEvent = getQueue();
	curEvent->_action = action;
	switch (action->_a0._actType) {                   // Assign whether local or global
	case AGSCHEDULE:
		curEvent->_localActionFl = false;               // Lasts over a new screen
		break;
	// Workaround: When dying, switch to storyMode in order to block the keyboard.
	case GAMEOVER:
		_vm->getGameStatus()._storyModeFl = true;
	// No break on purpose
	default:
		curEvent->_localActionFl = true;                // Rest are for current screen only
		break;
	}

	curEvent->_time = action->_a0._timer + getTicks(); // Convert rel to abs time

	// Now find the place to insert the event
	if (!_tailEvent) {                                // Empty queue
		_tailEvent = _headEvent = curEvent;
		curEvent->_nextEvent = curEvent->_prevEvent = 0;
	} else {
		Event *wrkEvent = _tailEvent;                   // Search from latest time back
		bool found = false;

		while (wrkEvent && !found) {
			if (wrkEvent->_time <= curEvent->_time) {     // Found if new event later
				found = true;
				if (wrkEvent == _tailEvent)                 // New latest in list
					_tailEvent = curEvent;
				else
					wrkEvent->_nextEvent->_prevEvent = curEvent;
				curEvent->_nextEvent = wrkEvent->_nextEvent;
				wrkEvent->_nextEvent = curEvent;
				curEvent->_prevEvent = wrkEvent;
			}
			wrkEvent = wrkEvent->_prevEvent;
		}

		if (!found) {                                   // Must be earliest in list
			_headEvent->_prevEvent = curEvent;            // So insert as new head
			curEvent->_nextEvent = _headEvent;
			curEvent->_prevEvent = 0;
			_headEvent = curEvent;
		}
	}
}

/**
 * This function performs the action in the event structure pointed to by p
 * It dequeues the event and returns it to the free list.  It returns a ptr
 * to the next action in the list, except special case of NEW_SCREEN
 */
Event *Scheduler::doAction(Event *curEvent) {
	debugC(1, kDebugSchedule, "doAction - Event action type : %d", curEvent->_action->_a0._actType);

	Status &gameStatus = _vm->getGameStatus();
	Act    *action = curEvent->_action;
	Object *obj1;
	int     dx, dy;
	Event  *wrkEvent;                                 // Save ev_p->nextEvent for return

	switch (action->_a0._actType) {
	case ANULL:                                       // Big NOP from DEL_EVENTS
		break;
	case ASCHEDULE:                                   // act0: Schedule an action list
		insertActionList(action->_a0._actIndex);
		break;
	case START_OBJ:                                   // act1: Start an object cycling
		_vm->_object->_objects[action->_a1._objIndex]._cycleNumb = action->_a1._cycleNumb;
		_vm->_object->_objects[action->_a1._objIndex]._cycling = action->_a1._cycle;
		break;
	case INIT_OBJXY:                                  // act2: Initialize an object
		_vm->_object->_objects[action->_a2._objIndex]._x = action->_a2._x;          // Coordinates
		_vm->_object->_objects[action->_a2._objIndex]._y = action->_a2._y;
		break;
	case PROMPT:                                      // act3: Prompt user for key phrase
		promptAction(action);
		break;
	case BKGD_COLOR:                                  // act4: Set new background color
		_vm->_screen->setBackgroundColor(action->_a4._newBackgroundColor);
		break;
	case INIT_OBJVXY:                                 // act5: Initialize an object velocity
		_vm->_object->setVelocity(action->_a5._objIndex, action->_a5._vx, action->_a5._vy);
		break;
	case INIT_CARRY:                                  // act6: Initialize an object
		_vm->_object->setCarry(action->_a6._objIndex, action->_a6._carriedFl);  // carried status
		break;
	case INIT_HF_COORD:                               // act7: Initialize an object to hero's "feet" coords
		_vm->_object->_objects[action->_a7._objIndex]._x = _vm->_hero->_x - 1;
		_vm->_object->_objects[action->_a7._objIndex]._y = _vm->_hero->_y + _vm->_hero->_currImagePtr->_y2 - 1;
		_vm->_object->_objects[action->_a7._objIndex]._screenIndex = *_vm->_screenPtr;  // Don't forget screen!
		break;
	case NEW_SCREEN:                                  // act8: Start new screen
		newScreen(action->_a8._screenIndex);
		break;
	case INIT_OBJSTATE:                               // act9: Initialize an object state
		_vm->_object->_objects[action->_a9._objIndex]._state = action->_a9._newState;
		break;
	case INIT_PATH:                                   // act10: Initialize an object path and velocity
		_vm->_object->setPath(action->_a10._objIndex, (Path) action->_a10._newPathType, action->_a10._vxPath, action->_a10._vyPath);
		break;
	case COND_R:                                      // act11: action lists conditional on object state
		if (_vm->_object->_objects[action->_a11._objIndex]._state == action->_a11._stateReq)
			insertActionList(action->_a11._actPassIndex);
		else
			insertActionList(action->_a11._actFailIndex);
		break;
	case TEXT:                                        // act12: Text box (CF WARN)
		Utils::notifyBox(_vm->_file->fetchString(action->_a12._stringIndex));   // Fetch string from file
		break;
	case SWAP_IMAGES:                                 // act13: Swap 2 object images
		_vm->_object->swapImages(action->_a13._objIndex1, action->_a13._objIndex2);
		break;
	case COND_SCR:                                    // act14: Conditional on current screen
		if (_vm->_object->_objects[action->_a14._objIndex]._screenIndex == action->_a14._screenReq)
			insertActionList(action->_a14._actPassIndex);
		else
			insertActionList(action->_a14._actFailIndex);
		break;
	case AUTOPILOT:                                   // act15: Home in on a (stationary) object
		_vm->_object->homeIn(action->_a15._objIndex1, action->_a15._objIndex2, action->_a15._dx, action->_a15._dy);
		break;
	case INIT_OBJ_SEQ:                                // act16: Set sequence number to use
		// Note: Don't set a sequence at time 0 of a new screen, it causes
		// problems clearing the boundary bits of the object!  t>0 is safe
		_vm->_object->_objects[action->_a16._objIndex]._currImagePtr = _vm->_object->_objects[action->_a16._objIndex]._seqList[action->_a16._seqIndex]._seqPtr;
		break;
	case SET_STATE_BITS:                              // act17: OR mask with curr obj state
		_vm->_object->_objects[action->_a17._objIndex]._state |= action->_a17._stateMask;
		break;
	case CLEAR_STATE_BITS:                            // act18: AND ~mask with curr obj state
		_vm->_object->_objects[action->_a18._objIndex]._state &= ~action->_a18._stateMask;
		break;
	case TEST_STATE_BITS:                             // act19: If all bits set, do apass else afail
		if ((_vm->_object->_objects[action->_a19._objIndex]._state & action->_a19._stateMask) == action->_a19._stateMask)
			insertActionList(action->_a19._actPassIndex);
		else
			insertActionList(action->_a19._actFailIndex);
		break;
	case DEL_EVENTS:                                  // act20: Remove all events of this action type
		delEventType(action->_a20._actTypeDel);
		break;
	case GAMEOVER:                                    // act21: Game over!
		// NOTE: Must wait at least 1 tick before issuing this action if
		// any objects are to be made invisible!
		gameStatus._gameOverFl = true;
		break;
	case INIT_HH_COORD:                               // act22: Initialize an object to hero's actual coords
		_vm->_object->_objects[action->_a22._objIndex]._x = _vm->_hero->_x;
		_vm->_object->_objects[action->_a22._objIndex]._y = _vm->_hero->_y;
		_vm->_object->_objects[action->_a22._objIndex]._screenIndex = *_vm->_screenPtr;// Don't forget screen!
		break;
	case EXIT:                                        // act23: Exit game back to DOS
		_vm->endGame();
		break;
	case BONUS:                                       // act24: Get bonus score for action
		processBonus(action->_a24._pointIndex);
		break;
	case COND_BOX:                                    // act25: Conditional on bounding box
		obj1 = &_vm->_object->_objects[action->_a25._objIndex];
		dx = obj1->_x + obj1->_currImagePtr->_x1;
		dy = obj1->_y + obj1->_currImagePtr->_y2;
		if ((dx >= action->_a25._x1) && (dx <= action->_a25._x2) &&
		        (dy >= action->_a25._y1) && (dy <= action->_a25._y2))
			insertActionList(action->_a25._actPassIndex);
		else
			insertActionList(action->_a25._actFailIndex);
		break;
	case SOUND:                                       // act26: Play a sound (or tune)
		if (action->_a26._soundIndex < _vm->_tunesNbr)
			_vm->_sound->playMusic(action->_a26._soundIndex);
		else
			_vm->_sound->playSound(action->_a26._soundIndex, kSoundPriorityMedium);
		break;
	case ADD_SCORE:                                   // act27: Add object's value to score
		_vm->adjustScore(_vm->_object->_objects[action->_a27._objIndex]._objValue);
		break;
	case SUB_SCORE:                                   // act28: Subtract object's value from score
		_vm->adjustScore(-_vm->_object->_objects[action->_a28._objIndex]._objValue);
		break;
	case COND_CARRY:                                  // act29: Conditional on object being carried
		if (_vm->_object->isCarried(action->_a29._objIndex))
			insertActionList(action->_a29._actPassIndex);
		else
			insertActionList(action->_a29._actFailIndex);
		break;
	case INIT_MAZE:                                   // act30: Enable and init maze structure
		_vm->_maze._enabledFl = true;
		_vm->_maze._size = action->_a30._mazeSize;
		_vm->_maze._x1 = action->_a30._x1;
		_vm->_maze._y1 = action->_a30._y1;
		_vm->_maze._x2 = action->_a30._x2;
		_vm->_maze._y2 = action->_a30._y2;
		_vm->_maze._x3 = action->_a30._x3;
		_vm->_maze._x4 = action->_a30._x4;
		_vm->_maze._firstScreenIndex = action->_a30._firstScreenIndex;
		break;
	case EXIT_MAZE:                                   // act31: Disable maze mode
		_vm->_maze._enabledFl = false;
		break;
	case INIT_PRIORITY:
		_vm->_object->_objects[action->_a32._objIndex]._priority = action->_a32._priority;
		break;
	case INIT_SCREEN:
		_vm->_object->_objects[action->_a33._objIndex]._screenIndex = action->_a33._screenIndex;
		break;
	case AGSCHEDULE:                                  // act34: Schedule a (global) action list
		insertActionList(action->_a34._actIndex);
		break;
	case REMAPPAL:                                    // act35: Remap a palette color
		_vm->_screen->remapPal(action->_a35._oldColorIndex, action->_a35._newColorIndex);
		break;
	case COND_NOUN:                                   // act36: Conditional on noun mentioned
		if (_vm->_parser->isWordPresent(_vm->_text->getNounArray(action->_a36._nounIndex)))
			insertActionList(action->_a36._actPassIndex);
		else
			insertActionList(action->_a36._actFailIndex);
		break;
	case SCREEN_STATE:                                // act37: Set new screen state
		_vm->_screenStates[action->_a37._screenIndex] = action->_a37._newState;
		break;
	case INIT_LIPS:                                   // act38: Position lips on object
		_vm->_object->_objects[action->_a38._lipsObjIndex]._x = _vm->_object->_objects[action->_a38._objIndex]._x + action->_a38._dxLips;
		_vm->_object->_objects[action->_a38._lipsObjIndex]._y = _vm->_object->_objects[action->_a38._objIndex]._y + action->_a38._dyLips;
		_vm->_object->_objects[action->_a38._lipsObjIndex]._screenIndex = *_vm->_screenPtr; // Don't forget screen!
		_vm->_object->_objects[action->_a38._lipsObjIndex]._cycling = kCycleForward;
		break;
	case INIT_STORY_MODE:                             // act39: Init story_mode flag
		// This is similar to the QUIET path mode, except that it is
		// independant of it and it additionally disables the ">" prompt
		gameStatus._storyModeFl = action->_a39._storyModeFl;
		break;
	case WARN:                                        // act40: Text box (CF TEXT)
		Utils::notifyBox(_vm->_file->fetchString(action->_a40._stringIndex));
		break;
	case COND_BONUS:                                  // act41: Perform action if got bonus
		if (_points[action->_a41._bonusIndex]._scoredFl)
			insertActionList(action->_a41._actPassIndex);
		else
			insertActionList(action->_a41._actFailIndex);
		break;
	case TEXT_TAKE:                                   // act42: Text box with "take" message
		Utils::notifyBox(Common::String::format(TAKE_TEXT, _vm->_text->getNoun(_vm->_object->_objects[action->_a42._objIndex]._nounIndex, TAKE_NAME)));
		break;
	case YESNO:                                       // act43: Prompt user for Yes or No
		if (Utils::yesNoBox(_vm->_file->fetchString(action->_a43._promptIndex)))
			insertActionList(action->_a43._actYesIndex);
		else
			insertActionList(action->_a43._actNoIndex);
		break;
	case STOP_ROUTE:                                  // act44: Stop any route in progress
		_vm->_route->resetRoute();
		break;
	case COND_ROUTE:                                  // act45: Conditional on route in progress
		if (_vm->_route->getRouteIndex() >= action->_a45._routeIndex)
			insertActionList(action->_a45._actPassIndex);
		else
			insertActionList(action->_a45._actFailIndex);
		break;
	case INIT_JUMPEXIT:                               // act46: Init status.jumpexit flag
		// This is to allow left click on exit to get there immediately
		// For example the plane crash in Hugo2 where hero is invisible
		// Couldn't use INVISIBLE flag since conflicts with boat in Hugo1
		_vm->_mouse->setJumpExitFl(action->_a46._jumpExitFl);
		break;
	case INIT_VIEW:                                   // act47: Init object._viewx, viewy, dir
		_vm->_object->_objects[action->_a47._objIndex]._viewx = action->_a47._viewx;
		_vm->_object->_objects[action->_a47._objIndex]._viewy = action->_a47._viewy;
		_vm->_object->_objects[action->_a47._objIndex]._direction = action->_a47._direction;
		break;
	case INIT_OBJ_FRAME:                              // act48: Set seq,frame number to use
		// Note: Don't set a sequence at time 0 of a new screen, it causes
		// problems clearing the boundary bits of the object!  t>0 is safe
		_vm->_object->_objects[action->_a48._objIndex]._currImagePtr = _vm->_object->_objects[action->_a48._objIndex]._seqList[action->_a48._seqIndex]._seqPtr;
		for (dx = 0; dx < action->_a48._frameIndex; dx++)
			_vm->_object->_objects[action->_a48._objIndex]._currImagePtr = _vm->_object->_objects[action->_a48._objIndex]._currImagePtr->_nextSeqPtr;
		break;
	case OLD_SONG:
		// Replaces ACT26 for DOS games.
		_vm->_sound->_DOSSongPtr = _vm->_text->getTextData(action->_a49._songIndex);
		break;
	default:
		error("An error has occurred: %s", "doAction");
		break;
	}

	if (action->_a0._actType == NEW_SCREEN) {         // New_screen() deletes entire list
		return 0;                                       // nextEvent = 0 since list now empty
	} else {
		wrkEvent = curEvent->_nextEvent;
		delQueue(curEvent);                             // Return event to free list
		return wrkEvent;                                // Return next event ptr
	}
}

/**
 * Delete an event structure (i.e. return it to the free list)
 * Historical note:  Originally event p was assumed to be at head of queue
 * (i.e. earliest) since all events were deleted in order when proceeding to
 * a new screen.  To delete an event from the middle of the queue, the action
 * was overwritten to be ANULL.  With the advent of GLOBAL events, delQueue
 * was modified to allow deletes anywhere in the list, and the DEL_EVENT
 * action was modified to perform the actual delete.
 */
void Scheduler::delQueue(Event *curEvent) {
	debugC(4, kDebugSchedule, "delQueue()");

	if (curEvent == _headEvent) {                     // If p was the head ptr
		_headEvent = curEvent->_nextEvent;              // then make new head_p
	} else {                                          // Unlink p
		curEvent->_prevEvent->_nextEvent = curEvent->_nextEvent;
		if (curEvent->_nextEvent)
			curEvent->_nextEvent->_prevEvent = curEvent->_prevEvent;
		else
			_tailEvent = curEvent->_prevEvent;
	}

	if (_headEvent)
		_headEvent->_prevEvent = 0;                     // Mark end of list
	else
		_tailEvent = 0;                                 // Empty queue

	curEvent->_nextEvent = _freeEvent;                // Return p to free list
	if (_freeEvent)                                   // Special case, if free list was empty
		_freeEvent->_prevEvent = curEvent;
	_freeEvent = curEvent;
}

/**
 * Delete all the active events of a given type
 */
void Scheduler::delEventType(const Action _actTypeDel) {
	// Note: actions are not deleted here, simply turned into NOPs!
	Event *wrkEvent = _headEvent;                     // The earliest event
	Event *saveEvent;

	while (wrkEvent) {                                // While events found in list
		saveEvent = wrkEvent->_nextEvent;
		if (wrkEvent->_action->_a20._actType == _actTypeDel)
			delQueue(wrkEvent);
		wrkEvent = saveEvent;
	}
}

/**
 * Save the points table
 */
void Scheduler::savePoints(Common::WriteStream *out) const {
	for (int i = 0; i < _numBonuses; i++) {
		out->writeByte(_points[i]._score);
		out->writeByte((_points[i]._scoredFl) ? 1 : 0);
	}
}

/**
 * Restore the points table
 */
void Scheduler::restorePoints(Common::ReadStream *in) {
	// Restore points table
	for (int i = 0; i < _numBonuses; i++) {
		_points[i]._score = in->readByte();
		_points[i]._scoredFl = (in->readByte() == 1);
	}
}

Scheduler_v1d::Scheduler_v1d(HugoEngine *vm) : Scheduler(vm) {
}

Scheduler_v1d::~Scheduler_v1d() {
}

const char *Scheduler_v1d::getCypher() const {
	return "Copyright (c) 1990, Gray Design Associates";
}

uint32 Scheduler_v1d::getTicks() {
	return getDosTicks(false);
}

/**
 * This is the scheduler which runs every tick.  It examines the event queue
 * for any events whose time has come.  It dequeues these events and performs
 * the action associated with the event, returning it to the free queue
 */
void Scheduler_v1d::runScheduler() {
	debugC(6, kDebugSchedule, "runScheduler");

	uint32 ticker = getTicks();                       // The time now, in ticks
	Event *curEvent = _headEvent;                     // The earliest event

	while (curEvent && (curEvent->_time <= ticker))   // While mature events found
		curEvent = doAction(curEvent);                  // Perform the action (returns nextEvent)
}

void Scheduler_v1d::promptAction(Act *action) {
	Common::String response;

	response = Utils::promptBox(_vm->_file->fetchString(action->_a3._promptIndex));

	response.toLowercase();

	char resp[256];
	Common::strlcpy(resp, response.c_str(), 256);

	if (action->_a3._encodedFl)
		decodeString(resp);

	if (strstr(resp, _vm->_file->fetchString(action->_a3._responsePtr[0])))
		insertActionList(action->_a3._actPassIndex);
	else
		insertActionList(action->_a3._actFailIndex);
}

/**
 * Decode a response to a prompt
 */
void Scheduler_v1d::decodeString(char *line) {
	debugC(1, kDebugSchedule, "decodeString(%s)", line);

	uint16 linelength = strlen(line);
	for (uint16 i = 0; i < linelength; i++) {
		line[i] = (line[i] + _cypher.c_str()[i % _cypher.size()]) % '~';
		if (line[i] < ' ')
			line[i] += ' ';
	}
}

Scheduler_v2d::Scheduler_v2d(HugoEngine *vm) : Scheduler_v1d(vm) {
}

Scheduler_v2d::~Scheduler_v2d() {
}

const char *Scheduler_v2d::getCypher() const {
	return "Copyright 1991, Gray Design Associates";
}

void Scheduler_v2d::promptAction(Act *action) {
	Common::String response;

	response = Utils::promptBox(_vm->_file->fetchString(action->_a3._promptIndex));
	response.toLowercase();

	debug(1, "doAction(act3), expecting answer %s", _vm->_file->fetchString(action->_a3._responsePtr[0]));

	bool  found = false;
	const char *tmpStr;                               // General purpose string ptr

	for (int dx = 0; !found && (action->_a3._responsePtr[dx] != -1); dx++) {
		tmpStr = _vm->_file->fetchString(action->_a3._responsePtr[dx]);
		if (response.contains(tmpStr))
			found = true;
	}

	if (found)
		insertActionList(action->_a3._actPassIndex);
	else
		insertActionList(action->_a3._actFailIndex);
}

/**
 * Decode a string
 */
void Scheduler_v2d::decodeString(char *line) {
	debugC(1, kDebugSchedule, "decodeString(%s)", line);

	int16 lineLength = strlen(line);
	for (uint16 i = 0; i < lineLength; i++)
		line[i] -= _cypher.c_str()[i % _cypher.size()];

	debugC(1, kDebugSchedule, "result : %s", line);
}

Scheduler_v3d::Scheduler_v3d(HugoEngine *vm) : Scheduler_v2d(vm) {
}

Scheduler_v3d::~Scheduler_v3d() {
}

const char *Scheduler_v3d::getCypher() const {
	return "Copyright 1992, Gray Design Associates";
}

Scheduler_v1w::Scheduler_v1w(HugoEngine *vm) : Scheduler_v3d(vm) {
}

Scheduler_v1w::~Scheduler_v1w() {
}

uint32 Scheduler_v1w::getTicks() {
	return getWinTicks();
}

/**
 * This is the scheduler which runs every tick.  It examines the event queue
 * for any events whose time has come.  It dequeues these events and performs
 * the action associated with the event, returning it to the free queue
 */
void Scheduler_v1w::runScheduler() {
	debugC(6, kDebugSchedule, "runScheduler");

	uint32 ticker = getTicks();                       // The time now, in ticks
	Event *curEvent = _headEvent;                     // The earliest event

	while (curEvent && (curEvent->_time <= ticker))   // While mature events found
		curEvent = doAction(curEvent);                  // Perform the action (returns nextEvent)

	_vm->getGameStatus()._tick++;                     // Accessed elsewhere via getTicks()
}
} // End of namespace Hugo
