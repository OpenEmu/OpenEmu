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
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE_EVENTS_H
#define CGE_EVENTS_H

#include "common/events.h"
#include "cge/game.h"
#include "cge/talk.h"
#include "cge/vga13h.h"

namespace CGE {

/*----------------- KEYBOARD interface -----------------*/

#define kEventMax   256

enum EventMask {
	kMouseRoll      = 1 << 0,
	kMouseLeftDown  = 1 << 1,
	kMouseLeftUp    = 1 << 2,
	kMouseRightDown = 1 << 3,
	kMouseRightUp   = 1 << 4,
	kEventAttn      = 1 << 5,
	kEventKeyb      = 1 << 7
};

class Keyboard {
private:
	bool getKey(Common::Event &event);
	CGEEngine *_vm;
public:
	Sprite *_client;
	bool _keyAlt;

	void newKeyboard(Common::Event &event);
	Sprite *setClient(Sprite *spr);

	Keyboard(CGEEngine *vm);
	~Keyboard();
};

/*----------------- MOUSE interface -----------------*/

struct CGEEvent {
	uint16 _mask;
	uint16 _x;
	uint16 _y;
	Common::KeyCode _keyCode;
	Sprite *_spritePtr;
};

class Mouse : public Sprite {
public:
	Sprite *_hold;
	bool _active;
	int _hx;
	int _hy;
	bool _exist;
	int _buttons;
	Sprite *_busy;
	Mouse(CGEEngine *vm);
	~Mouse();
	void on();
	void off();
	void newMouse(Common::Event &event);
private:
	CGEEngine *_vm;
};

/*----------------- EventManager interface -----------------*/

class EventManager {
private:
	CGEEngine *_vm;
	Common::Event _event;
	CGEEvent _eventQueue[kEventMax];
	uint16 _eventQueueHead;
	uint16 _eventQueueTail;

	void handleEvents();
public:
	EventManager(CGEEngine *vm);
	void poll();
	void clearEvent(Sprite *spr);

	CGEEvent &getNextEvent();
};

} // End of namespace CGE

#endif
