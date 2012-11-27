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

#include "backends/platform/linuxmoto/linuxmoto-sdl.h"
#include "backends/keymapper/keymapper.h"
#include "common/keyboard.h"

#ifdef ENABLE_KEYMAPPER

using namespace Common;

struct Key {
	const char *hwId;
	KeyCode keycode;
	uint16 ascii;
	const char *desc;
	bool shiftable;
};

static const Key keys[] = {
	{ "FIRE", KEYCODE_RETURN, ASCII_RETURN, "Fire", false },
	{ "CAMERA", KEYCODE_PAUSE, 0, "Camera", false },
	{ "HANGUP", KEYCODE_ESCAPE, ASCII_ESCAPE, "Hangup", false },
	{ "CALL", KEYCODE_SPACE, ASCII_SPACE, "Call", false },
	{ "PLUS", KEYCODE_PLUS, '+', "+", false },
	{ "MINUS", KEYCODE_MINUS, '-', "-", false },
#ifdef MOTOMAGX
	{"BACKSPACE", KEYCODE_BACKSPACE, ASCII_BACKSPACE, "Backspace", false},
	{"TAB", KEYCODE_TAB, ASCII_TAB, "Tab", false},
	{"HASH", KEYCODE_HASH, '#', "#", false},
	{"ASTERISK", KEYCODE_ASTERISK, '*', "*", false},
	{"LEFTSOFT", KEYCODE_F9, ASCII_F9, "LeftSoft", false},
	{"RIGHTSOFT", KEYCODE_F11, ASCII_F11, "RightSoft", false},
	{"0", KEYCODE_0, '0', "0", false},
	{"1", KEYCODE_1, '1', "1", false},
	{"2", KEYCODE_2, '2', "2", false},
	{"3", KEYCODE_3, '3', "3", false},
	{"4", KEYCODE_4, '4', "4", false},
	{"5", KEYCODE_5, '5', "5", false},
	{"6", KEYCODE_6, '6', "6", false},
	{"7", KEYCODE_7, '7', "7", false},
	{"8", KEYCODE_8, '8', "8", false},
	{"9", KEYCODE_9, '9', "9", false},
#endif
#ifdef MOTOEZX
	{ "a", KEYCODE_a, 'a', "a", true },
	{ "b", KEYCODE_b, 'b', "b", true },
	{ "c", KEYCODE_c, 'c', "c", true },
	{ "d", KEYCODE_d, 'd', "d", true },
	{ "e", KEYCODE_e, 'e', "e", true },
	{ "f", KEYCODE_f, 'f', "f", true },
	{ "g", KEYCODE_g, 'g', "g", true },
	{ "h", KEYCODE_h, 'h', "h", true },
	{ "i", KEYCODE_i, 'i', "i", true },
	{ "j", KEYCODE_j, 'j', "j", true },
#endif
	// Numeric keypad

	// Arrows + Home/End pad
	{"UP", KEYCODE_UP, 0, "Up", false},
	{"DOWN", KEYCODE_DOWN, 0, "Down", false},
	{"RIGHT", KEYCODE_RIGHT, 0, "Right", false},
	{"LEFT", KEYCODE_LEFT, 0, "Left", false},

	// Function keys

	// Miscellaneous function keys

	{0, KEYCODE_INVALID, 0, 0, false}
};

struct Mod {
	byte flag;
	const char *id;
	const char *desc;
	bool shiftable;
};

static const Mod modifiers[] = {
	{ 0, "", "", false },
	{ KBD_CTRL, "C+", "Ctrl+", false },
	{ KBD_ALT, "A+", "Alt+", false },
	{ KBD_SHIFT, "", "", true },
	{ KBD_CTRL | KBD_ALT, "C+A+", "Ctrl+Alt+", false },
	{ KBD_SHIFT | KBD_CTRL, "S+C+", "Shift+Ctrl+", true },
	{ KBD_SHIFT | KBD_CTRL | KBD_ALT, "C+A+", "Ctrl+Alt+", true },
	{ 0, 0, 0, false }
};

Common::HardwareInputSet *OSystem_LINUXMOTO::getHardwareInputSet() {
	return OSystem_SDL::getHardwareInputSet();
}
#endif
