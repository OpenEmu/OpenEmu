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

#include "cine/console.h"
#include "cine/cine.h"

namespace Cine {

bool labyrinthCheat;

CineConsole::CineConsole(CineEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("labyrinthCheat", WRAP_METHOD(CineConsole, Cmd_LabyrinthCheat));

	labyrinthCheat = false;
}

CineConsole::~CineConsole() {
}

// Activate Cheat during Scene 6 Labyrinth chased by Guards in Otto's Mansion
// This puzzle is hard, especially without save/load so this will aid playtesting.
bool CineConsole::Cmd_LabyrinthCheat(int argc, const char **argv) {
	labyrinthCheat = true;
	return true;
}

} // End of namespace Cine
