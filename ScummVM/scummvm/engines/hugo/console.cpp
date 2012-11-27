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

#include "hugo/console.h"
#include "hugo/hugo.h"
#include "hugo/object.h"
#include "hugo/parser.h"
#include "hugo/schedule.h"
#include "hugo/text.h"

namespace Hugo {

HugoConsole::HugoConsole(HugoEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("listscreens",   WRAP_METHOD(HugoConsole, Cmd_listScreens));
	DCmd_Register("listobjects",   WRAP_METHOD(HugoConsole, Cmd_listObjects));
	DCmd_Register("getobject",     WRAP_METHOD(HugoConsole, Cmd_getObject));
	DCmd_Register("getallobjects", WRAP_METHOD(HugoConsole, Cmd_getAllObjects));
	DCmd_Register("gotoscreen",    WRAP_METHOD(HugoConsole, Cmd_gotoScreen));
	DCmd_Register("Boundaries",    WRAP_METHOD(HugoConsole, Cmd_boundaries));
}

HugoConsole::~HugoConsole() {
}

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

/**
 * This command loads up the specified screen number
 */
bool HugoConsole::Cmd_gotoScreen(int argc, const char **argv) {
	if ((argc != 2) || (strToInt(argv[1]) > _vm->_numScreens)){
		DebugPrintf("Usage: %s <screen number>\n", argv[0]);
		return true;
	}

	_vm->_scheduler->newScreen(strToInt(argv[1]));
	return false;
}

/**
 * This command lists all the screens available
 */
bool HugoConsole::Cmd_listScreens(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	DebugPrintf("Available screens for this game are:\n");
	for (int i = 0; i < _vm->_numScreens; i++)
		DebugPrintf("%2d - %s\n", i, _vm->_text->getScreenNames(i));
	return true;
}

/**
 * This command lists all the objects available
 */
bool HugoConsole::Cmd_listObjects(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	DebugPrintf("Available objects for this game are:\n");
	for (int i = 0; i < _vm->_object->_numObj; i++) {
		if (_vm->_object->_objects[i]._genericCmd & TAKE)
			DebugPrintf("%2d - %s\n", i, _vm->_text->getNoun(_vm->_object->_objects[i]._nounIndex, 2));
	}
	return true;
}

/**
 * This command puts an object in the inventory
 */
bool HugoConsole::Cmd_getObject(int argc, const char **argv) {
	if ((argc != 2) || (strToInt(argv[1]) > _vm->_object->_numObj)) {
		DebugPrintf("Usage: %s <object number>\n", argv[0]);
		return true;
	}

	if (_vm->_object->_objects[strToInt(argv[1])]._genericCmd & TAKE)
		_vm->_parser->takeObject(&_vm->_object->_objects[strToInt(argv[1])]);
	else
		DebugPrintf("Object not available\n");

	return true;
}

/**
 * This command puts all the available objects in the inventory
 */
bool HugoConsole::Cmd_getAllObjects(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	for (int i = 0; i < _vm->_object->_numObj; i++) {
		if (_vm->_object->_objects[i]._genericCmd & TAKE)
			_vm->_parser->takeObject(&_vm->_object->_objects[i]);
	}

	return false;
}

/**
 * This command shows and hides boundaries
 */
bool HugoConsole::Cmd_boundaries(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	_vm->getGameStatus()._showBoundariesFl = !_vm->getGameStatus()._showBoundariesFl;
	return false;
}

} // End of namespace Hugo
