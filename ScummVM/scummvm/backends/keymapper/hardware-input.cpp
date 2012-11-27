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
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "backends/keymapper/hardware-input.h"

#ifdef ENABLE_KEYMAPPER

#include "backends/keymapper/keymapper.h"

namespace Common {

static const KeyTableEntry defaultKeys[] = {
	{"BACKSPACE", KEYCODE_BACKSPACE, ASCII_BACKSPACE, "Backspace", false},
	{"TAB", KEYCODE_TAB, ASCII_TAB, "Tab", false},
	{"CLEAR", KEYCODE_CLEAR, 0, "Clear", false},
	{"RETURN", KEYCODE_RETURN, ASCII_RETURN, "Return", false},
	{"PAUSE", KEYCODE_PAUSE, 0, "Pause", false},
	{"ESCAPE", KEYCODE_ESCAPE, ASCII_ESCAPE, "Esc", false},
	{"SPACE", KEYCODE_SPACE, ASCII_SPACE, "Space", false},
	{"EXCLAIM", KEYCODE_EXCLAIM, '!', "!", false},
	{"QUOTEDBL", KEYCODE_QUOTEDBL, '"', "\"", false},
	{"HASH", KEYCODE_HASH, '#', "#", false},
	{"DOLLAR", KEYCODE_DOLLAR, '$', "$", false},
	{"AMPERSAND", KEYCODE_AMPERSAND, '&', "&", false},
	{"QUOTE", KEYCODE_QUOTE, '\'', "'", false},
	{"LEFTPAREN", KEYCODE_LEFTPAREN, '(', "(", false},
	{"RIGHTPAREN", KEYCODE_RIGHTPAREN, ')', ")", false},
	{"ASTERISK", KEYCODE_ASTERISK, '*', "*", false},
	{"PLUS", KEYCODE_PLUS, '+', "+", false},
	{"COMMA", KEYCODE_COMMA, ',', ",", false},
	{"MINUS", KEYCODE_MINUS, '-', "-", false},
	{"PERIOD", KEYCODE_PERIOD, '.', ".", false},
	{"SLASH", KEYCODE_SLASH, '/', "/", false},
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
	{"COLON", KEYCODE_COLON, ':', ":", false},
	{"SEMICOLON", KEYCODE_SEMICOLON, ';', ";", false},
	{"LESS", KEYCODE_LESS, '<', "<", false},
	{"EQUALS", KEYCODE_EQUALS, '=', "=", false},
	{"GREATER", KEYCODE_GREATER, '>', ">", false},
	{"QUESTION", KEYCODE_QUESTION, '?', "?", false},
	{"AT", KEYCODE_AT, '@', "@", false},

	{"LEFTBRACKET", KEYCODE_LEFTBRACKET, '[', "[", false},
	{"BACKSLASH", KEYCODE_BACKSLASH, '\\', "\\", false},
	{"RIGHTBRACKET", KEYCODE_RIGHTBRACKET, ']', "]", false},
	{"CARET", KEYCODE_CARET, '^', "^", false},
	{"UNDERSCORE", KEYCODE_UNDERSCORE, '_', "_", false},
	{"BACKQUOTE", KEYCODE_BACKQUOTE, '`', "`", false},
	{"a", KEYCODE_a, 'a', "a", true},
	{"b", KEYCODE_b, 'b', "b", true},
	{"c", KEYCODE_c, 'c', "c", true},
	{"d", KEYCODE_d, 'd', "d", true},
	{"e", KEYCODE_e, 'e', "e", true},
	{"f", KEYCODE_f, 'f', "f", true},
	{"g", KEYCODE_g, 'g', "g", true},
	{"h", KEYCODE_h, 'h', "h", true},
	{"i", KEYCODE_i, 'i', "i", true},
	{"j", KEYCODE_j, 'j', "j", true},
	{"k", KEYCODE_k, 'k', "k", true},
	{"l", KEYCODE_l, 'l', "l", true},
	{"m", KEYCODE_m, 'm', "m", true},
	{"n", KEYCODE_n, 'n', "n", true},
	{"o", KEYCODE_o, 'o', "o", true},
	{"p", KEYCODE_p, 'p', "p", true},
	{"q", KEYCODE_q, 'q', "q", true},
	{"r", KEYCODE_r, 'r', "r", true},
	{"s", KEYCODE_s, 's', "s", true},
	{"t", KEYCODE_t, 't', "t", true},
	{"u", KEYCODE_u, 'u', "u", true},
	{"v", KEYCODE_v, 'v', "v", true},
	{"w", KEYCODE_w, 'w', "w", true},
	{"x", KEYCODE_x, 'x', "x", true},
	{"y", KEYCODE_y, 'y', "y", true},
	{"z", KEYCODE_z, 'z', "z", true},
	{"DELETE", KEYCODE_DELETE, 0, "Del", false},

	// Numeric keypad
	{"KP0", KEYCODE_KP0, 0, "KP0", false},
	{"KP1", KEYCODE_KP1, 0, "KP1", false},
	{"KP2", KEYCODE_KP2, 0, "KP2", false},
	{"KP3", KEYCODE_KP3, 0, "KP3", false},
	{"KP4", KEYCODE_KP4, 0, "KP4", false},
	{"KP5", KEYCODE_KP5, 0, "KP5", false},
	{"KP6", KEYCODE_KP6, 0, "KP6", false},
	{"KP7", KEYCODE_KP7, 0, "KP7", false},
	{"KP8", KEYCODE_KP8, 0, "KP8", false},
	{"KP9", KEYCODE_KP9, 0, "KP9", false},
	{"KP_PERIOD", KEYCODE_KP_PERIOD, 0, "KP.", false},
	{"KP_DIVIDE", KEYCODE_KP_DIVIDE, 0, "KP/", false},
	{"KP_MULTIPLY", KEYCODE_KP_MULTIPLY, 0, "KP*", false},
	{"KP_MINUS", KEYCODE_KP_MINUS, 0, "KP-", false},
	{"KP_PLUS", KEYCODE_KP_PLUS, 0, "KP+", false},
	{"KP_ENTER", KEYCODE_KP_ENTER, 0, "KP Enter", false},
	{"KP_EQUALS", KEYCODE_KP_EQUALS, 0, "KP=", false},

	// Arrows + Home/End pad
	{"UP", KEYCODE_UP, 0, "Up", false},
	{"DOWN", KEYCODE_DOWN, 0, "Down", false},
	{"RIGHT", KEYCODE_RIGHT, 0, "Right", false},
	{"LEFT", KEYCODE_LEFT, 0, "Left", false},
	{"INSERT", KEYCODE_INSERT, 0, "Insert", false},
	{"HOME", KEYCODE_HOME, 0, "Home", false},
	{"END", KEYCODE_END, 0, "End", false},
	{"PAGEUP", KEYCODE_PAGEUP, 0, "PgUp", false},
	{"PAGEDOWN", KEYCODE_PAGEDOWN, 0, "PgDn", false},

	// Function keys
	{"F1", KEYCODE_F1, ASCII_F1, "F1", false},
	{"F2", KEYCODE_F2, ASCII_F2, "F2", false},
	{"F3", KEYCODE_F3, ASCII_F3, "F3", false},
	{"F4", KEYCODE_F4, ASCII_F4, "F4", false},
	{"F5", KEYCODE_F5, ASCII_F5, "F5", false},
	{"F6", KEYCODE_F6, ASCII_F6, "F6", false},
	{"F7", KEYCODE_F7, ASCII_F7, "F7", false},
	{"F8", KEYCODE_F8, ASCII_F8, "F8", false},
	{"F9", KEYCODE_F9, ASCII_F9, "F9", false},
	{"F10", KEYCODE_F10, ASCII_F10, "F10", false},
	{"F11", KEYCODE_F11, ASCII_F11, "F11", false},
	{"F12", KEYCODE_F12, ASCII_F12, "F12", false},
	{"F13", KEYCODE_F13, 0, "F13", false},
	{"F14", KEYCODE_F14, 0, "F14", false},
	{"F15", KEYCODE_F15, 0, "F15", false},

	// Miscellaneous function keys
	{"HELP", KEYCODE_HELP, 0, "Help", false},
	{"PRINT", KEYCODE_PRINT, 0, "Print", false},
	{"SYSREQ", KEYCODE_SYSREQ, 0, "SysRq", false},
	{"BREAK", KEYCODE_BREAK, 0, "Break", false},
	{"MENU", KEYCODE_MENU, 0, "Menu", false},
		// Power Macintosh power key
	{"POWER", KEYCODE_POWER, 0, "Power", false},
		// Some european keyboards
	{"EURO", KEYCODE_EURO, 0, "Euro", false},
		// Atari keyboard has Undo
	{"UNDO", KEYCODE_UNDO, 0, "Undo", false},
	{0, KEYCODE_INVALID, 0, 0, false}
};

static const ModifierTableEntry defaultModifiers[] = {
	{ 0, "", "", false },
	{ KBD_CTRL, "C+", "Ctrl+", false },
	{ KBD_ALT, "A+", "Alt+", false },
	{ KBD_SHIFT, "", "", true },
	{ KBD_CTRL | KBD_ALT, "C+A+", "Ctrl+Alt+", false },
	{ KBD_SHIFT | KBD_CTRL, "S+C+", "Shift+Ctrl+", true },
	{ KBD_SHIFT | KBD_CTRL | KBD_ALT, "C+A+", "Ctrl+Alt+", true },
	{ 0, 0, 0, false }
};

HardwareInputSet::HardwareInputSet(bool useDefault, const KeyTableEntry *keys, const ModifierTableEntry *modifiers) {
	if (useDefault)
		addHardwareInputs(defaultKeys, defaultModifiers);
	if (keys)
		addHardwareInputs(keys, modifiers ? modifiers : defaultModifiers);
}

HardwareInputSet::~HardwareInputSet() {
	List<const HardwareInput *>::const_iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); ++it)
		delete *it;
}

void HardwareInputSet::addHardwareInput(const HardwareInput *input) {
	assert(input);

	debug(8, "Adding hardware input [%s][%s]", input->id.c_str(), input->description.c_str());

	removeHardwareInput(input);

	_inputs.push_back(input);
}

const HardwareInput *HardwareInputSet::findHardwareInput(String id) const {
	List<const HardwareInput *>::const_iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); ++it) {
		if ((*it)->id == id)
			return (*it);
	}
	return 0;
}

const HardwareInput *HardwareInputSet::findHardwareInput(const HardwareInputCode code) const {
	List<const HardwareInput *>::const_iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); ++it) {
		const HardwareInput *entry = *it;
		if (entry->type == kHardwareInputTypeGeneric && entry->inputCode == code)
			return entry;
	}
	return 0;
}

const HardwareInput *HardwareInputSet::findHardwareInput(const KeyState& keystate) const {
	List<const HardwareInput *>::const_iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); ++it) {
		const HardwareInput *entry = *it;
		if (entry->type == kHardwareInputTypeKeyboard && entry->key == keystate)
			return entry;
	}
	return 0;
}

void HardwareInputSet::addHardwareInputs(const HardwareInputTableEntry inputs[]) {
	for (const HardwareInputTableEntry *entry = inputs; entry->hwId; ++entry)
		addHardwareInput(new HardwareInput(entry->hwId, entry->code, entry->desc));
}

void HardwareInputSet::addHardwareInputs(const KeyTableEntry keys[], const ModifierTableEntry modifiers[]) {
	const KeyTableEntry *key;
	const ModifierTableEntry *mod;
	char fullKeyId[50];
	char fullKeyDesc[100];
	uint16 ascii;

	for (mod = modifiers; mod->id; mod++) {
		for (key = keys; key->hwId; key++) {
			ascii = key->ascii;

			if (mod->shiftable && key->shiftable) {
				snprintf(fullKeyId, 50, "%s%c", mod->id, toupper(key->hwId[0]));
				snprintf(fullKeyDesc, 100, "%s%c", mod->desc, toupper(key->desc[0]));
				ascii = toupper(key->ascii);
			} else if (mod->shiftable) {
				snprintf(fullKeyId, 50, "S+%s%s", mod->id, key->hwId);
				snprintf(fullKeyDesc, 100, "Shift+%s%s", mod->desc, key->desc);
			} else {
				snprintf(fullKeyId, 50, "%s%s", mod->id, key->hwId);
				snprintf(fullKeyDesc, 100, "%s%s", mod->desc, key->desc);
			}

			addHardwareInput(new HardwareInput(fullKeyId, KeyState(key->keycode, ascii, mod->flag), fullKeyDesc));
		}
	}
}

void HardwareInputSet::removeHardwareInput(const HardwareInput *input) {
	if (!input)
		return;

	List<const HardwareInput *>::iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); ++it) {
		const HardwareInput *entry = (*it);
		bool match = false;
		if (entry->id == input->id)
			match = true;
		else if (input->type == entry->type) {
			if (input->type == kHardwareInputTypeGeneric && input->inputCode == entry->inputCode)
				match = true;
			else if (input->type == kHardwareInputTypeKeyboard && input->key == entry->key)
				match = true;
		}
		if (match) {
			debug(7, "Removing hardware input [%s] (%s) because it matches [%s] (%s)", entry->id.c_str(), entry->description.c_str(), input->id.c_str(), input->description.c_str());
			delete entry;
			_inputs.erase(it);
		}
	}
}

} //namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

