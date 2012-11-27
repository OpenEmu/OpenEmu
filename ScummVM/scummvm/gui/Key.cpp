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

#include "gui/Key.h"

namespace GUI {

Key::Key() :
_ascii(0), _keycode(0), _flags(0) {
}

Key::Key(int ascii) :
_ascii(ascii), _keycode(ascii), _flags(0) {
}

Key::Key(int ascii, int keycode, int flags) :
_ascii(ascii), _keycode(keycode), _flags(flags) {
}

void Key::setKey(int ascii) {
	_ascii = ascii;
	_keycode = ascii;
}

void Key::setKey(int ascii, int keycode) {
	_ascii = ascii;
	_keycode = keycode;
}

void Key::setKey(int ascii, int keycode, int flags) {
	_ascii = ascii;
	_keycode = keycode;
	_flags = flags;
}

int Key::ascii() {
	return _ascii;
}

int Key::keycode() {
	return _keycode;
}

int Key::flags() {
	return _flags;
}

} // namespace GUI
