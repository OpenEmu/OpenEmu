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

#include "drascula/console.h"
#include "gui/debugger.h"
#include "drascula/drascula.h"

namespace Drascula {

Console::Console(DrasculaEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("room",			WRAP_METHOD(Console, Cmd_Room));
}

Console::~Console() {
}

bool Console::Cmd_Room(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: room <number>\n");
		return true;
	}

	int roomNum = atoi(argv[1]);

	_vm->loadedDifferentChapter = 0;
	_vm->enterRoom(roomNum);
	_vm->selectVerb(kVerbNone);
	_vm->clearRoom();
	_vm->loadPic(roomNum, _vm->bgSurface, HALF_PAL);
	_vm->selectionMade = 0;

	return false;
}

} // End of namespace Drascula
