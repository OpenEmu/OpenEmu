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

#include "tony/gfxengine.h"
#include "tony/tony.h"

namespace Tony {

RMInput::RMInput() {
	_leftClickMouse = _leftReleaseMouse = false;
	_rightClickMouse = _rightReleaseMouse = false;
}

void RMInput::poll() {
	_leftClickMouse = _leftReleaseMouse = _rightClickMouse = _rightReleaseMouse = false;

	// Get pending events
	while (g_system->getEventManager()->pollEvent(_event) && !g_vm->shouldQuit()) {
		switch (_event.type) {
		case Common::EVENT_MOUSEMOVE:
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:
			_mousePos = _event.mouse;

			if (_event.type == Common::EVENT_LBUTTONDOWN) {
				_leftClickMouse = true;
			} else if (_event.type == Common::EVENT_LBUTTONUP) {
				_leftReleaseMouse = true;
			} else if (_event.type == Common::EVENT_RBUTTONDOWN) {
				_rightClickMouse = true;
			} else if (_event.type == Common::EVENT_RBUTTONUP) {
				_rightReleaseMouse = true;
			} else
				continue;

			// Since a mouse button has changed, don't do any further event processing this frame
			return;

		case Common::EVENT_KEYDOWN:
			// Check for debugger
			if ((_event.kbd.keycode == Common::KEYCODE_d) && (_event.kbd.flags & Common::KBD_CTRL)) {
				// Attach to the debugger
				g_vm->_debugger->attach();
				g_vm->_debugger->onFrame();
			} else {
				// Flag the given key as being down
				_keyDown.push_back(_event.kbd.keycode);
			}
			return;

		case Common::EVENT_KEYUP:
			for (uint i = 0; i < _keyDown.size(); i++) {
				if (_keyDown[i] == _event.kbd.keycode) {
					_keyDown.remove_at(i);
					break;
				}
			}
			return;

		default:
			break;
		}
	}
}

/**
 * Return true if a key has been pressed
 */
bool RMInput::getAsyncKeyState(Common::KeyCode kc) {
	// The act of testing for a particular key automatically clears the state, to prevent
	// the same key being registered in multiple different frames
	for (uint i = 0; i < _keyDown.size(); i++) {
		if (_keyDown[i] == kc) {
			_keyDown.remove_at(i);
			return true;
		}
	}
	return false;
}

/**
 * Reading of the mouse
 */
RMPoint RMInput::mousePos() {
	RMPoint p(_mousePos.x, _mousePos.y);
	return p;
}

/**
 * Events of mouse clicks
 */
bool RMInput::mouseLeftClicked() {
	return _leftClickMouse;
}

bool RMInput::mouseRightClicked() {
	return _rightClickMouse;
}

bool RMInput::mouseLeftReleased() {
	return _leftReleaseMouse;
}

bool RMInput::mouseRightReleased() {
	return _rightReleaseMouse;
}

} // End of namespace Tony
