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

#ifndef GUI_KEY_H
#define GUI_KEY_H

#include "common/scummsys.h"
#include "common/system.h"

namespace GUI {

// TODO/FIXME: Make use of Common::KeyState from common/keyboard.h,
// or even better, just completely replace this by it.
// To be able to do that, though, the code using GUI::Key would need to
// be adopted -- right now it uses SDL keycodes, and uses SDL_PushEvent
// to generated fake events.

class Key {
public:
	Key(int ascii);
	Key(int ascii, int keycode, int flags = 0);
	Key();

	void setKey(int ascii);
	void setKey(int ascii, int keycode);
	void setKey(int ascii, int keycode, int flags);

	int ascii();
	int keycode();
	int flags();
private:
	int _ascii;
	int _keycode;
	int _flags;
};

} // namespace GUI

#endif
