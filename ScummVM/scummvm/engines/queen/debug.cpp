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

#include "common/scummsys.h"

#include "queen/debug.h"

#include "queen/graphics.h"
#include "queen/logic.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"

namespace Queen {

Debugger::Debugger(QueenEngine *vm)
	: _vm(vm), _flags(0) {

	DCmd_Register("areas", WRAP_METHOD(Debugger, Cmd_Areas));
	DCmd_Register("asm",   WRAP_METHOD(Debugger, Cmd_Asm));
	DCmd_Register("bob",   WRAP_METHOD(Debugger, Cmd_Bob));
	DCmd_Register("bobs",  WRAP_METHOD(Debugger, Cmd_PrintBobs));
	DCmd_Register("gs",    WRAP_METHOD(Debugger, Cmd_GameState));
	DCmd_Register("info",  WRAP_METHOD(Debugger, Cmd_Info));
	DCmd_Register("items", WRAP_METHOD(Debugger, Cmd_Items));
	DCmd_Register("room",  WRAP_METHOD(Debugger, Cmd_Room));
	DCmd_Register("song",  WRAP_METHOD(Debugger, Cmd_Song));
}

Debugger::~Debugger() {} // we need this here for __SYMBIAN32__

void Debugger::preEnter() {
	_vm->pauseEngine(true);
}

void Debugger::postEnter() {
	_vm->pauseEngine(false);
	_vm->graphics()->setupMouseCursor();
}

bool Debugger::Cmd_Asm(int argc, const char **argv) {
	if (argc == 2) {
		uint16 sm = atoi(argv[1]);
		_vm->logic()->executeSpecialMove(sm);
		return false;
	} else {
		DebugPrintf("Usage: %s smnum\n", argv[0]);
	}
	return true;
}

bool Debugger::Cmd_Areas(int argc, const char **argv) {
	_flags ^= DF_DRAW_AREAS;
	DebugPrintf("Room areas display %s\n", (_flags & DF_DRAW_AREAS) != 0 ? "on" : "off");
	return true;
}

bool Debugger::Cmd_Bob(int argc, const char **argv) {
	if (argc >= 3) {
		int bobNum = atoi(argv[1]);
		if (bobNum >= Graphics::MAX_BOBS_NUMBER) {
			DebugPrintf("Bob %d is out of range (range: 0 - %d)\n", bobNum, Graphics::MAX_BOBS_NUMBER);
		} else {
			int param = (argc > 3) ? atoi(argv[3]) : 0;
			BobSlot *bob = _vm->graphics()->bob(bobNum);
			if (!strcmp(argv[2], "toggle")) {
				bob->active = !bob->active;
				DebugPrintf("bob[%d].active = %d\n", bobNum, bob->active);
			} else if (!strcmp(argv[2], "x")) {
				bob->x = param;
				DebugPrintf("bob[%d].x = %d\n", bobNum, bob->x);
			} else if (!strcmp(argv[2], "y")) {
				bob->y = param;
				DebugPrintf("bob[%d].y = %d\n", bobNum, bob->y);
			} else if (!strcmp(argv[2], "frame")) {
				bob->frameNum = param;
				DebugPrintf("bob[%d].frameNum = %d\n", bobNum, bob->frameNum);
			} else if (!strcmp(argv[2], "speed")) {
				bob->speed = param;
				DebugPrintf("bob[%d].speed = %d\n", bobNum, bob->speed);
			} else {
				DebugPrintf("Unknown bob command '%s'\n", argv[2]);
			}
		}
	} else {
		DebugPrintf("Usage: %s bobnum command parameter\n", argv[0]);
	}
	return true;
}

bool Debugger::Cmd_GameState(int argc, const char **argv) {
	uint16 slot;
	switch (argc) {
	case 2:
		slot = atoi(argv[1]);
		DebugPrintf("GAMESTATE[%d] ", slot);
		DebugPrintf("is %d\n", _vm->logic()->gameState(slot));
		break;
	case 3:
		slot = atoi(argv[1]);
		DebugPrintf("GAMESTATE[%d] ", slot);
		DebugPrintf("was %d ", _vm->logic()->gameState(slot));
		_vm->logic()->gameState(slot, atoi(argv[2]));
		DebugPrintf("now %d\n", _vm->logic()->gameState(slot));
		break;
	default:
		DebugPrintf("Usage: %s slotnum value\n", argv[0]);
		break;
	}
	return true;
}

bool Debugger::Cmd_Info(int argc, const char **argv) {
	DebugPrintf("Version: %s\n", _vm->resource()->getJASVersion());
	DebugPrintf("Audio compression: %d\n", _vm->resource()->getCompression());
	return true;
}

bool Debugger::Cmd_Items(int argc, const char **argv) {
	int n = _vm->logic()->itemDataCount();
	ItemData *item = _vm->logic()->itemData(1);
	while (n--) {
		item->name = ABS(item->name);
		++item;
	}
	DebugPrintf("Enabled all inventory items\n");
	return true;
}

bool Debugger::Cmd_PrintBobs(int argc, const char**argv) {
	int i;
	BobSlot *bob = _vm->graphics()->bob(0);
	DebugPrintf("+------------------------------------+\n");
	DebugPrintf("|# |  x|  y|f|scl|frm|a|m|spd| ex| ey|\n");
	DebugPrintf("+--+---+---+-+---+---+-+-+---+---+---+\n");
	for (i = 0; i < Graphics::MAX_BOBS_NUMBER; ++i, ++bob) {
		if (bob->active) {
			DebugPrintf("|%2d|%3d|%3d|%1d|%3d|%3d|%1d|%1d|%3d|%3d|%3d|\n",
				i, bob->x, bob->y, bob->xflip, bob->scale, bob->frameNum,
				bob->animating, bob->moving, bob->speed, bob->endx, bob->endy);
		}
	}
	DebugPrintf("+--------------------------------+\n");
	return true;
}

bool Debugger::Cmd_Room(int argc, const char **argv) {
	if (argc == 2) {
		uint16 roomNum = atoi(argv[1]);
		_vm->logic()->joePos(0, 0);
		_vm->logic()->newRoom(roomNum);
		_vm->logic()->entryObj(_vm->logic()->roomData(roomNum) + 1);
		return false;
	} else {
		DebugPrintf("Current room: %d (%s), use '%s <roomnum>' to switch\n",
			_vm->logic()->currentRoom(),
			_vm->logic()->roomName(_vm->logic()->currentRoom()),
			argv[0]);
	}
	return true;
}

bool Debugger::Cmd_Song(int argc, const char **argv) {
	if (argc == 2) {
		int16 songNum = atoi(argv[1]);
		_vm->sound()->playSong(songNum);
		DebugPrintf("Playing song %d\n", songNum);
	} else {
		DebugPrintf("Usage: %s songnum\n", argv[0]);
	}
	return true;
}

} // End of namespace Queen
