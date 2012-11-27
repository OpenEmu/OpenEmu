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


#include "common/events.h"

#include "graphics/cursorman.h"

#include "lure/events.h"
#include "lure/lure.h"
#include "lure/res.h"

namespace Lure {

static Mouse *int_mouse = NULL;

Mouse &Mouse::getReference() {
	return *int_mouse;
}

Mouse::Mouse() {
	int_mouse = this;

	_lButton = false;
	_rButton = false;
	_mButton = false;
	_cursorNum = CURSOR_ARROW;
	_x = 0;
	_y = 0;
	setCursorNum(CURSOR_ARROW);
}

Mouse::~Mouse() {
}

void Mouse::handleEvent(Common::Event event) {
	_x = (int16) event.mouse.x;
	_y = (int16) event.mouse.y;

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		_lButton = true;
		break;
	case Common::EVENT_LBUTTONUP:
		_lButton = false;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_rButton = true;
		break;
	case Common::EVENT_RBUTTONUP:
		_rButton = false;
		break;
	case Common::EVENT_MBUTTONDOWN:
		_mButton = true;
		break;
	case Common::EVENT_MBUTTONUP:
		_mButton = false;
		break;
	default:
		break;
	}
}


void Mouse::cursorOn() {
	CursorMan.showMouse(true);
}

void Mouse::cursorOff() {
	CursorMan.showMouse(false);
}

void Mouse::setCursorNum(CursorType cursorNum) {
	int hotspotX = 7, hotspotY = 7;
	if ((cursorNum == CURSOR_ARROW) || (cursorNum == CURSOR_MENUBAR)) {
		hotspotX = 0;
		hotspotY = 0;
	}

	setCursorNum(cursorNum, hotspotX, hotspotY);
}

void Mouse::setCursorNum(CursorType cursorNum, int hotspotX, int hotspotY) {
	Resources &res = Resources::getReference();

	_cursorNum = cursorNum;
	byte *cursorAddr = res.getCursor(cursorNum);
	CursorMan.replaceCursor(cursorAddr, CURSOR_WIDTH, CURSOR_HEIGHT, hotspotX, hotspotY, 0);
}

void Mouse::pushCursorNum(CursorType cursorNum) {
	int hotspotX = 7, hotspotY = 7;
	if ((cursorNum == CURSOR_ARROW) || (cursorNum == CURSOR_MENUBAR)) {
		hotspotX = 0;
		hotspotY = 0;
	}

	pushCursorNum(cursorNum, hotspotX, hotspotY);
}

void Mouse::pushCursorNum(CursorType cursorNum, int hotspotX, int hotspotY) {
	Resources &res = Resources::getReference();

	_cursorNum = cursorNum;
	byte *cursorAddr = res.getCursor(cursorNum);
	CursorMan.pushCursor(cursorAddr, CURSOR_WIDTH, CURSOR_HEIGHT, hotspotX, hotspotY, 0);
}

void Mouse::popCursor() {
	CursorMan.popCursor();
}

void Mouse::setPosition(int newX, int newY) {
	g_system->warpMouse(newX, newY);
}

void Mouse::waitForRelease() {
	Events &e = Events::getReference();
	LureEngine &engine = LureEngine::getReference();

	do {
		while (e.pollEvent() && !engine.shouldQuit())
			;
		g_system->delayMillis(20);
	} while (!engine.shouldQuit() && (lButton() || rButton() || mButton()));
}

/*--------------------------------------------------------------------------*/

static Events *int_events = NULL;

Events::Events() {
	int_events = this;
}

Events &Events::getReference() {
	return *int_events;
}


bool Events::pollEvent() {
	if (!g_system->getEventManager()->pollEvent(_event)) return false;

	// Handle keypress
	switch (_event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_MBUTTONUP:
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
		Mouse::getReference().handleEvent(_event);
		break;

	default:
		break;
	}

	return true;
}

void Events::waitForPress() {
	bool keyButton = false;
	while (!keyButton) {
		while (pollEvent()) {
			if ((_event.type == Common::EVENT_QUIT) || (_event.type == Common::EVENT_RTL)) return;
			else if ((_event.type == Common::EVENT_KEYDOWN) && (_event.kbd.ascii != 0))
				keyButton = true;
			else if ((_event.type == Common::EVENT_LBUTTONDOWN) ||
				(_event.type == Common::EVENT_MBUTTONDOWN) ||
				(_event.type == Common::EVENT_RBUTTONDOWN)) {
				keyButton = true;
				Mouse::getReference().waitForRelease();
			}
		}
		g_system->delayMillis(20);
	}
}

// interruptableDelay
// Delays for a given number of milliseconds. If it returns true, it indicates that
// the Escape has been pressed to abort whatever sequence is being displayed

bool Events::interruptableDelay(uint32 milliseconds) {
	Events &events = Events::getReference();
	LureEngine &engine = LureEngine::getReference();
	uint32 delayCtr = g_system->getMillis() + milliseconds;

	while (g_system->getMillis() < delayCtr) {
		if (engine.shouldQuit()) return true;

		if (events.pollEvent()) {
			if (((events.type() == Common::EVENT_KEYDOWN) && (events.event().kbd.ascii != 0)) ||
				(events.type() == Common::EVENT_LBUTTONDOWN))
				return true;
		}

		uint32 delayAmount = delayCtr - g_system->getMillis();
		if (delayAmount > 10) delayAmount = 10;
		g_system->delayMillis(delayAmount);
	}
	return false;
}

} // End of namespace Lure
