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

#include "groovie/debug.h"
#include "groovie/graphics.h"
#include "groovie/groovie.h"
#include "groovie/script.h"

#include "common/debug-channels.h"
#include "common/system.h"

#include "graphics/palette.h"

namespace Groovie {

Debugger::Debugger(GroovieEngine *vm) :
	_vm(vm), _script(_vm->_script) {

	// Register the debugger comands
	DCmd_Register("step", WRAP_METHOD(Debugger, cmd_step));
	DCmd_Register("go", WRAP_METHOD(Debugger, cmd_go));
	DCmd_Register("pc", WRAP_METHOD(Debugger, cmd_pc));
	DCmd_Register("fg", WRAP_METHOD(Debugger, cmd_fg));
	DCmd_Register("bg", WRAP_METHOD(Debugger, cmd_bg));
	DCmd_Register("mem", WRAP_METHOD(Debugger, cmd_mem));
	DCmd_Register("load", WRAP_METHOD(Debugger, cmd_loadgame));
	DCmd_Register("save", WRAP_METHOD(Debugger, cmd_savegame));
	DCmd_Register("playref", WRAP_METHOD(Debugger, cmd_playref));
	DCmd_Register("dumppal", WRAP_METHOD(Debugger, cmd_dumppal));
}

Debugger::~Debugger() {
	DebugMan.clearAllDebugChannels();
}

int Debugger::getNumber(const char *arg) {
	return strtol(arg, (char **)NULL, 0);
}

bool Debugger::cmd_step(int argc, const char **argv) {
	_script->step();
	return true;
}

bool Debugger::cmd_go(int argc, const char **argv) {
	_script->step();
	return false;
}

bool Debugger::cmd_fg(int argc, const char **argv) {
	_vm->_graphicsMan->updateScreen(&_vm->_graphicsMan->_foreground);
	return false;
}

bool Debugger::cmd_bg(int argc, const char **argv) {
	_vm->_graphicsMan->updateScreen(&_vm->_graphicsMan->_background);
	return false;
}

bool Debugger::cmd_pc(int argc, const char **argv) {
	if (argc == 2) {
		int val = getNumber(argv[1]);
		_script->_currentInstruction = val;
	}
	DebugPrintf("pc = 0x%04X (%d)\n", _script->_currentInstruction, _script->_currentInstruction);
	return true;
}

bool Debugger::cmd_mem(int argc, const char **argv) {
	if (argc >= 2) {
		int pos = getNumber(argv[1]);
		uint8 val;
		if (argc >= 3) {
			// Set
			val = getNumber(argv[2]);
			_script->_variables[pos] = val;
		} else {
			// Get
			val = _script->_variables[pos];
		}
		DebugPrintf("mem[0x%04X] = 0x%02X\n", pos, val);
	} else {
		DebugPrintf("Syntax: mem <addr> [<val>]\n");
	}
	return true;
}

bool Debugger::cmd_loadgame(int argc, const char **argv) {
	if (argc == 2) {
		int slot = getNumber(argv[1]);
		_script->loadgame(slot);
	} else {
		DebugPrintf("Syntax: load <slot>\n");
	}
	return true;
}

bool Debugger::cmd_savegame(int argc, const char **argv) {
	if (argc == 2) {
		int slot = getNumber(argv[1]);
		_script->savegame(slot);
	} else {
		DebugPrintf("Syntax: save <slot>\n");
	}
	return true;
}

bool Debugger::cmd_playref(int argc, const char **argv) {
	if (argc == 2) {
		int ref = getNumber(argv[1]);
		_script->playvideofromref(ref);
	} else {
		DebugPrintf("Syntax: playref <videorefnum>\n");
	}
	return true;
}

bool Debugger::cmd_dumppal(int argc, const char **argv) {
	uint16 i;
	byte palettedump[256 * 3];
	_vm->_system->getPaletteManager()->grabPalette(palettedump, 0, 256);

	for (i = 0; i < 256; i++) {
		DebugPrintf("%3d: %3d,%3d,%3d\n", i, palettedump[(i * 3)], palettedump[(i * 3) + 1], palettedump[(i * 3) + 2]);
	}
	return true;
}

} // End of Groovie namespace
