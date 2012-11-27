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
 * User events processing and utility functions
 */

#ifndef TINSEL_EVENTS_H
#define TINSEL_EVENTS_H

#include "common/coroutines.h"
#include "common/rect.h"
#include "tinsel/dw.h"

namespace Tinsel {

/*
enum BUTEVENT {
	PLR_NOEVENT, PLR_SLEFT, PLR_DLEFT, PLR_SRIGHT, PLR_DRIGHT,
	PLR_DRAG1_START, PLR_DRAG1_END, PLR_DRAG2_START, PLR_DRAG2_END,
	PLR_UNKNOWN
};

enum KEYEVENT {
	PLR_ESCAPE, PLR_QUIT, PLR_SAVE, PLR_LOAD, PLR_MENU,
	PLR_PGUP, PLR_PGDN, PLR_HOME, PLR_END,
	PLR_WALKTO, PLR_ACTION, PLR_LOOK,
	NOEVENT_KEY
};*/

enum PLR_EVENT {
	// action list
	PLR_PROV_WALKTO = 0,	// Provisional WALKTO !
	PLR_WALKTO = 1,
	PLR_LOOK = 2,
	PLR_ACTION = 3,
	PLR_ESCAPE = 4,
	PLR_MENU = 5,
	PLR_QUIT = 6,
	PLR_PGUP = 7,
	PLR_PGDN = 8,
	PLR_HOME = 9,
	PLR_END = 10,
	PLR_DRAG1_START = 11,
	PLR_DRAG1_END = 12,
	PLR_DRAG2_START = 13,
	PLR_DRAG2_END = 14,
	PLR_JUMP = 15,		// Call up scene hopper
	PLR_NOEVENT = 16,
	PLR_SAVE = 17,
	PLR_LOAD = 18,

	// Aliases used for DW1 actions
	PLR_SLEFT = PLR_WALKTO,
	PLR_DLEFT = PLR_ACTION,
	PLR_SRIGHT = PLR_LOOK,
	PLR_DRIGHT = PLR_NOEVENT,
	PLR_UNKNOWN = PLR_NOEVENT
};



/**
 * Reasons for running Glitter code.
 * Do not re-order these as equivalent CONSTs are defined in the master
 * scene Glitter source file for testing against the event() library function.
 *
 * Note: DW2 renames ENTER & LEAVE to WALKIN & WALKOUT, and has a new LEAVE event
 */
enum TINSEL_EVENT {
	NOEVENT, STARTUP, CLOSEDOWN, POINTED, UNPOINT, WALKIN, WALKOUT,
	PICKUP,	PUTDOWN, WALKTO, LOOK, ACTION, CONVERSE, SHOWEVENT,
	HIDEEVENT, TALKING, ENDTALK, LEAVE_T2, RESTORE, PROV_WALKTO
};

enum TINSEL1_EVENT {
	T1_POINTED, T1_WALKTO, T1_ACTION, T1_LOOK, T1_ENTER, T1_LEAVE, T1_STARTUP, T1_CONVERSE,
	T1_UNPOINT, T1_PUTDOWN, T1_NOEVENT
};

const TINSEL1_EVENT TINSEL1_EVENT_MAP[] = {
	T1_NOEVENT, T1_STARTUP, T1_NOEVENT, T1_POINTED, T1_UNPOINT, T1_ENTER, T1_LEAVE,
	T1_NOEVENT, T1_PUTDOWN, T1_WALKTO, T1_LOOK, T1_ACTION, T1_CONVERSE, T1_NOEVENT,
	T1_NOEVENT, T1_NOEVENT, T1_NOEVENT, T1_NOEVENT, T1_NOEVENT, T1_NOEVENT
};

void AllowDclick(CORO_PARAM, PLR_EVENT be);
bool GetControl(int param);
bool GetControl();
bool ControlIsOn();
void ControlOn();
void ControlOff();
void ControlStartOff();

void RunPolyTinselCode(HPOLYGON hPoly, TINSEL_EVENT event, PLR_EVENT be, bool tc);
void effRunPolyTinselCode(HPOLYGON hPoly, TINSEL_EVENT event, int actor);

void ProcessButEvent(PLR_EVENT be);
void ProcessKeyEvent(PLR_EVENT ke);


int GetEscEvents();
int GetLeftEvents();
bool LeftEventChange(int myleftEvent);

int getUserEvents();

uint32 getUserEventTime();
void resetUserEventTime();

void ResetEcount();

void PolygonEvent(CORO_PARAM, HPOLYGON hPoly, TINSEL_EVENT tEvent, int actor, bool bWait,
				int myEscape, bool *result = NULL);


void PlayerEvent(PLR_EVENT pEvent, const Common::Point &coOrds);

void ProcessedProvisional();
void ProvNotProcessed();
bool GetProvNotProcessed();

} // End of namespace Tinsel

#endif /* TINSEL_EVENTS_H */
