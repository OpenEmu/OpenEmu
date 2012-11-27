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

#ifndef LURE_EVENTS_H
#define LURE_EVENTS_H


#include "common/events.h"
#include "common/str.h"
#include "lure/luredefs.h"
#include "lure/disk.h"

namespace Lure {

class Mouse {
private:
	CursorType _cursorNum;
	int16 _x, _y;
	bool _lButton, _rButton, _mButton;
public:
	Mouse();
	~Mouse();
	static Mouse &getReference();
	void handleEvent(Common::Event event);

	void cursorOn();
	void cursorOff();
	void setCursorNum(CursorType cursorNum);
	void setCursorNum(CursorType cursorNum, int hotspotX, int hotspotY);
	CursorType getCursorNum() { return _cursorNum; }
	void setPosition(int x, int y);
	int16 x() { return _x; }
	int16 y() { return _y; }
	bool lButton() { return _lButton; }
	bool rButton() { return _rButton; }
	bool mButton() { return _mButton; }
	void waitForRelease();
	void pushCursorNum(CursorType cursorNum);
	void pushCursorNum(CursorType cursorNum, int hotspotX, int hotspotY);
	void popCursor();
};

class Events {
private:
	Common::Event _event;
public:
	Events();
	static Events &getReference();

	bool pollEvent();
	void waitForPress();
	bool interruptableDelay(uint32 milliseconds);

	Common::Event event() { return _event; }
	Common::EventType type() { return _event.type; }
};

} // End of namespace Lure

#endif
