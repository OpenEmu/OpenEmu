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
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_INPUT_H
#define TONY_INPUT_H

#include "common/events.h"
#include "common/rect.h"
#include "common/array.h"
#include "common/keyboard.h"
#include "tony/utils.h"

namespace Tony {

class RMInput {
private:
	Common::Event _event;

	// Mouse related fields
	Common::Point _mousePos;
	bool _leftClickMouse, _leftReleaseMouse, _rightClickMouse, _rightReleaseMouse;

	// Keyboard related fields
	Common::Array<Common::KeyCode> _keyDown;

public:
	RMInput();

	/**
	 * Polling (must be performed once per frame)
	 */
	void poll();

	/**
	 * Reading of the mouse
	 */
	RMPoint mousePos();

	/**
	 * Events of mouse clicks
	 */
	bool mouseLeftClicked();
	bool mouseRightClicked();
	bool mouseLeftReleased();
	bool mouseRightReleased();

	/**
	 * Returns true if the given key is pressed
	 */
	bool getAsyncKeyState(Common::KeyCode kc);
};

} // End of namespace Tony

#endif
