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

#include "kyra/debugger.h"
#include "kyra/kyra_lok.h"
#include "kyra/kyra_hof.h"
#include "kyra/timer.h"
#include "kyra/resource.h"
#include "kyra/lol.h"
#include "kyra/eobcommon.h"

#include "common/system.h"
#include "common/config-manager.h"

#include "gui/message.h"

namespace Kyra {

Debugger::Debugger(KyraEngine_v1 *vm)
	: ::GUI::Debugger(), _vm(vm) {
}

void Debugger::initialize() {
	DCmd_Register("continue",           WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("screen_debug_mode",  WRAP_METHOD(Debugger, cmd_setScreenDebug));
	DCmd_Register("load_palette",       WRAP_METHOD(Debugger, cmd_loadPalette));
	DCmd_Register("facings",            WRAP_METHOD(Debugger, cmd_showFacings));
	DCmd_Register("gamespeed",          WRAP_METHOD(Debugger, cmd_gameSpeed));
	DCmd_Register("flags",              WRAP_METHOD(Debugger, cmd_listFlags));
	DCmd_Register("toggleflag",         WRAP_METHOD(Debugger, cmd_toggleFlag));
	DCmd_Register("queryflag",          WRAP_METHOD(Debugger, cmd_queryFlag));
	DCmd_Register("timers",             WRAP_METHOD(Debugger, cmd_listTimers));
	DCmd_Register("settimercountdown",  WRAP_METHOD(Debugger, cmd_setTimerCountdown));
}

bool Debugger::cmd_setScreenDebug(int argc, const char **argv) {
	if (argc > 1) {
		if (scumm_stricmp(argv[1], "enable") == 0)
			_vm->screen()->enableScreenDebug(true);
		else if (scumm_stricmp(argv[1], "disable") == 0)
			_vm->screen()->enableScreenDebug(false);
		else
			DebugPrintf("Use screen_debug_mode <enable/disable> to enable or disable it.\n");
	} else {
		DebugPrintf("Screen debug mode is %s.\n", (_vm->screen()->queryScreenDebug() ? "enabled" : "disabled"));
		DebugPrintf("Use screen_debug_mode <enable/disable> to enable or disable it.\n");
	}
	return true;
}

bool Debugger::cmd_loadPalette(int argc, const char **argv) {
	Palette palette(_vm->screen()->getPalette(0).getNumColors());

	if (argc <= 1) {
		DebugPrintf("Use load_palette <file> [start_col] [end_col]\n");
		return true;
	}

	if (_vm->game() != GI_KYRA1 && _vm->resource()->getFileSize(argv[1]) != 768) {
		uint8 *buffer = new uint8[320 * 200 * sizeof(uint8)];
		if (!buffer) {
			DebugPrintf("ERROR: Cannot allocate buffer for screen region!\n");
			return true;
		}

		_vm->screen()->copyRegionToBuffer(5, 0, 0, 320, 200, buffer);
		_vm->screen()->loadBitmap(argv[1], 5, 5, 0);
		palette.copy(_vm->screen()->getCPagePtr(5), 0, 256);
		_vm->screen()->copyBlockToPage(5, 0, 0, 320, 200, buffer);

		delete[] buffer;
	} else if (!_vm->screen()->loadPalette(argv[1], palette)) {
		DebugPrintf("ERROR: Palette '%s' not found!\n", argv[1]);
		return true;
	}

	int startCol = 0;
	int endCol = palette.getNumColors();
	if (argc > 2)
		startCol = MIN(palette.getNumColors(), MAX(0, atoi(argv[2])));
	if (argc > 3)
		endCol = MIN(palette.getNumColors(), MAX(0, atoi(argv[3])));

	if (startCol > 0)
		palette.copy(_vm->screen()->getPalette(0), 0, startCol);
	if (endCol < palette.getNumColors())
		palette.copy(_vm->screen()->getPalette(0), endCol);

	_vm->screen()->setScreenPalette(palette);
	_vm->screen()->updateScreen();

	return true;
}

bool Debugger::cmd_showFacings(int argc, const char **argv) {
	DebugPrintf("Facing directions:\n");
	DebugPrintf("7  0  1\n");
	DebugPrintf(" \\ | / \n");
	DebugPrintf("6--*--2\n");
	DebugPrintf(" / | \\\n");
	DebugPrintf("5  4  3\n");
	return true;
}

bool Debugger::cmd_gameSpeed(int argc, const char **argv) {
	if (argc == 2) {
		int val = atoi(argv[1]);

		if (val < 1 || val > 1000) {
			DebugPrintf("speed must lie between 1 and 1000 (default: 60)\n");
			return true;
		}

		_vm->_tickLength = (uint8)(1000.0 / val);
	} else {
		DebugPrintf("Syntax: gamespeed <value>\n");
	}

	return true;
}

bool Debugger::cmd_listFlags(int argc, const char **argv) {
	for (int i = 0, p = 0; i < (int)sizeof(_vm->_flagsTable) * 8; i++, ++p) {
		DebugPrintf("(%-3i): %-2i", i, _vm->queryGameFlag(i));
		if (p == 5) {
			DebugPrintf("\n");
			p -= 6;
		}
	}
	DebugPrintf("\n");
	return true;
}

bool Debugger::cmd_toggleFlag(int argc, const char **argv) {
	if (argc > 1) {
		uint flag = atoi(argv[1]);
		if (_vm->queryGameFlag(flag))
			_vm->resetGameFlag(flag);
		else
			_vm->setGameFlag(flag);
		DebugPrintf("Flag %i is now %i\n", flag, _vm->queryGameFlag(flag));
	} else {
		DebugPrintf("Syntax: toggleflag <flag>\n");
	}

	return true;
}

bool Debugger::cmd_queryFlag(int argc, const char **argv) {
	if (argc > 1) {
		uint flag = atoi(argv[1]);
		DebugPrintf("Flag %i is %i\n", flag, _vm->queryGameFlag(flag));
	} else {
		DebugPrintf("Syntax: queryflag <flag>\n");
	}

	return true;
}

bool Debugger::cmd_listTimers(int argc, const char **argv) {
	DebugPrintf("Current time: %-8u\n", g_system->getMillis());
	for (int i = 0; i < _vm->timer()->count(); i++)
		DebugPrintf("Timer %-2i: Active: %-3s Countdown: %-6i %-8u\n", i, _vm->timer()->isEnabled(i) ? "Yes" : "No", _vm->timer()->getDelay(i), _vm->timer()->getNextRun(i));

	return true;
}

bool Debugger::cmd_setTimerCountdown(int argc, const char **argv) {
	if (argc > 2) {
		uint timer = atoi(argv[1]);
		uint countdown = atoi(argv[2]);
		_vm->timer()->setCountdown(timer, countdown);
		DebugPrintf("Timer %i now has countdown %i\n", timer, _vm->timer()->getDelay(timer));
	} else {
		DebugPrintf("Syntax: settimercountdown <timer> <countdown>\n");
	}

	return true;
}

#pragma mark -

Debugger_LoK::Debugger_LoK(KyraEngine_LoK *vm)
	: Debugger(vm), _vm(vm) {
}

void Debugger_LoK::initialize() {
	DCmd_Register("enter",              WRAP_METHOD(Debugger_LoK, cmd_enterRoom));
	DCmd_Register("scenes",             WRAP_METHOD(Debugger_LoK, cmd_listScenes));
	DCmd_Register("give",               WRAP_METHOD(Debugger_LoK, cmd_giveItem));
	DCmd_Register("birthstones",        WRAP_METHOD(Debugger_LoK, cmd_listBirthstones));
	Debugger::initialize();
}

bool Debugger_LoK::cmd_enterRoom(int argc, const char **argv) {
	uint direction = 0;
	if (argc > 1) {
		int room = atoi(argv[1]);

		// game will crash if entering a non-existent room
		if (room >= _vm->_roomTableSize) {
			DebugPrintf("room number must be any value between (including) 0 and %d\n", _vm->_roomTableSize - 1);
			return true;
		}

		if (argc > 2) {
			direction = atoi(argv[2]);
		} else {
			if (_vm->_roomTable[room].northExit != 0xFFFF)
				direction = 3;
			else if (_vm->_roomTable[room].eastExit != 0xFFFF)
				direction = 4;
			else if (_vm->_roomTable[room].southExit != 0xFFFF)
				direction = 1;
			else if (_vm->_roomTable[room].westExit != 0xFFFF)
				direction = 2;
		}

		_vm->_system->hideOverlay();
		_vm->_currentCharacter->facing = direction;

		_vm->enterNewScene(room, _vm->_currentCharacter->facing, 0, 0, 1);
		while (!_vm->_screen->isMouseVisible())
			_vm->_screen->showMouse();

		detach();
		return false;
	}

	DebugPrintf("Syntax: room <roomnum> <direction>\n");
	return true;
}

bool Debugger_LoK::cmd_listScenes(int argc, const char **argv) {
	for (int i = 0; i < _vm->_roomTableSize; i++) {
		DebugPrintf("%-3i: %-10s", i, _vm->_roomFilenameTable[_vm->_roomTable[i].nameIndex]);
		if (!(i % 8))
			DebugPrintf("\n");
	}
	DebugPrintf("\n");
	DebugPrintf("Current room: %i\n", _vm->_currentRoom);
	return true;
}

bool Debugger_LoK::cmd_giveItem(int argc, const char **argv) {
	if (argc == 2) {
		int item = atoi(argv[1]);

		// Kyrandia 1 has only 108 items (-1 to 106), otherwise it will crash
		if (item < -1 || item > 106) {
			DebugPrintf("'itemid' must be any value between (including) -1 and 106\n");
			return true;
		}

		_vm->setMouseItem(item);
		_vm->_itemInHand = item;
	} else {
		DebugPrintf("Syntax: give <itemid>\n");
	}

	return true;
}

bool Debugger_LoK::cmd_listBirthstones(int argc, const char **argv) {
	DebugPrintf("Needed birthstone gems:\n");
	for (int i = 0; i < ARRAYSIZE(_vm->_birthstoneGemTable); ++i)
		DebugPrintf("%-3d '%s'\n", _vm->_birthstoneGemTable[i], _vm->_itemList[_vm->_birthstoneGemTable[i]]);
	return true;
}

#pragma mark -

Debugger_v2::Debugger_v2(KyraEngine_v2 *vm) : Debugger(vm), _vm(vm) {
}

void Debugger_v2::initialize() {
	DCmd_Register("character_info",     WRAP_METHOD(Debugger_v2, cmd_characterInfo));
	DCmd_Register("enter",              WRAP_METHOD(Debugger_v2, cmd_enterScene));
	DCmd_Register("scenes",             WRAP_METHOD(Debugger_v2, cmd_listScenes));
	DCmd_Register("scene_info",         WRAP_METHOD(Debugger_v2, cmd_sceneInfo));
	DCmd_Register("scene_to_facing",    WRAP_METHOD(Debugger_v2, cmd_sceneToFacing));
	DCmd_Register("give",               WRAP_METHOD(Debugger_v2, cmd_giveItem));
	Debugger::initialize();
}

bool Debugger_v2::cmd_enterScene(int argc, const char **argv) {
	uint direction = 0;
	if (argc > 1) {
		int scene = atoi(argv[1]);

		// game will crash if entering a non-existent scene
		if (scene >= _vm->_sceneListSize) {
			DebugPrintf("scene number must be any value between (including) 0 and %d\n", _vm->_sceneListSize - 1);
			return true;
		}

		if (argc > 2) {
			direction = atoi(argv[2]);
		} else {
			if (_vm->_sceneList[scene].exit1 != 0xFFFF)
				direction = 4;
			else if (_vm->_sceneList[scene].exit2 != 0xFFFF)
				direction = 6;
			else if (_vm->_sceneList[scene].exit3 != 0xFFFF)
				direction = 0;
			else if (_vm->_sceneList[scene].exit4 != 0xFFFF)
				direction = 2;
		}

		_vm->_system->hideOverlay();
		_vm->_mainCharacter.facing = direction;

		_vm->enterNewScene(scene, _vm->_mainCharacter.facing, 0, 0, 1);
		while (!_vm->screen_v2()->isMouseVisible())
			_vm->screen_v2()->showMouse();

		detach();
		return false;
	}

	DebugPrintf("Syntax: %s <scenenum> <direction>\n", argv[0]);
	return true;
}

bool Debugger_v2::cmd_listScenes(int argc, const char **argv) {
	int shown = 1;
	for (int i = 0; i < _vm->_sceneListSize; ++i) {
		if (_vm->_sceneList[i].filename1[0]) {
			DebugPrintf("%-2i: %-10s", i, _vm->_sceneList[i].filename1);
			if (!(shown % 5))
				DebugPrintf("\n");
			++shown;
		}
	}
	DebugPrintf("\n");
	DebugPrintf("Current scene: %i\n", _vm->_currentScene);
	return true;
}

bool Debugger_v2::cmd_sceneInfo(int argc, const char **argv) {
	DebugPrintf("Current scene: %d '%s'\n", _vm->_currentScene, _vm->_sceneList[_vm->_currentScene].filename1);
	DebugPrintf("\n");
	DebugPrintf("Exit information:\n");
	DebugPrintf("Exit1: leads to %d, position %dx%d\n", int16(_vm->_sceneExit1), _vm->_sceneEnterX1, _vm->_sceneEnterY1);
	DebugPrintf("Exit2: leads to %d, position %dx%d\n", int16(_vm->_sceneExit2), _vm->_sceneEnterX2, _vm->_sceneEnterY2);
	DebugPrintf("Exit3: leads to %d, position %dx%d\n", int16(_vm->_sceneExit3), _vm->_sceneEnterX3, _vm->_sceneEnterY3);
	DebugPrintf("Exit4: leads to %d, position %dx%d\n", int16(_vm->_sceneExit4), _vm->_sceneEnterX4, _vm->_sceneEnterY4);
	DebugPrintf("Special exit information:\n");
	if (!_vm->_specialExitCount) {
		DebugPrintf("No special exits.\n");
	} else {
		DebugPrintf("This scene has %d special exits.\n", _vm->_specialExitCount);
		for (int i = 0; i < _vm->_specialExitCount; ++i) {
			DebugPrintf("SpecialExit%d: facing %d, position (x1/y1/x2/y2): %d/%d/%d/%d\n", i,
			            _vm->_specialExitTable[20 + i], _vm->_specialExitTable[0 + i], _vm->_specialExitTable[5 + i],
			            _vm->_specialExitTable[10 + i], _vm->_specialExitTable[15 + i]);
		}
	}

	return true;
}

bool Debugger_v2::cmd_characterInfo(int argc, const char **argv) {
	DebugPrintf("Main character is in scene: %d '%s'\n", _vm->_mainCharacter.sceneId, _vm->_sceneList[_vm->_mainCharacter.sceneId].filename1);
	DebugPrintf("Position: %dx%d\n", _vm->_mainCharacter.x1, _vm->_mainCharacter.y1);
	DebugPrintf("Facing: %d\n", _vm->_mainCharacter.facing);
	DebugPrintf("Inventory:\n");
	for (int i = 0; i < 20; ++i) {
		DebugPrintf("%-2d ", int8(_vm->_mainCharacter.inventory[i]));
		if (i == 9 || i == 19)
			DebugPrintf("\n");
	}
	return true;
}

bool Debugger_v2::cmd_sceneToFacing(int argc, const char **argv) {
	if (argc == 2) {
		int facing = atoi(argv[1]);
		int16 exit = -1;

		switch (facing) {
		case 0: case 1: case 7:
			exit = _vm->_sceneList[_vm->_currentScene].exit1;
			break;

		case 6:
			exit = _vm->_sceneList[_vm->_currentScene].exit2;
			break;

		case 3: case 4: case 5:
			exit = _vm->_sceneList[_vm->_currentScene].exit3;
			break;

		case 2:
			exit = _vm->_sceneList[_vm->_currentScene].exit4;
			break;

		default:
			break;
		}

		DebugPrintf("Exit to facing %d leads to room %d.\n", facing, exit);
	} else {
		DebugPrintf("Usage: %s <facing>\n", argv[0]);
	}

	return true;
}

bool Debugger_v2::cmd_giveItem(int argc, const char **argv) {
	if (argc == 2) {
		int item = atoi(argv[1]);

		if (item < -1 || item > _vm->engineDesc().maxItemId) {
			DebugPrintf("itemid must be any value between (including) -1 and %d\n", _vm->engineDesc().maxItemId);
			return true;
		}

		_vm->setHandItem(item);
	} else {
		DebugPrintf("Syntax: give <itemid>\n");
	}

	return true;
}

#pragma mark -

Debugger_HoF::Debugger_HoF(KyraEngine_HoF *vm) : Debugger_v2(vm), _vm(vm) {
}

void Debugger_HoF::initialize() {
	DCmd_Register("pass_codes",         WRAP_METHOD(Debugger_HoF, cmd_passcodes));
	Debugger_v2::initialize();
}

bool Debugger_HoF::cmd_passcodes(int argc, const char **argv) {
	if (argc == 2) {
		int val = atoi(argv[1]);

		if (val < 0 || val > 1) {
			DebugPrintf("value must be either 1 (on) or 0 (off)\n");
			return true;
		}

		_vm->_dbgPass = val;
	} else {
		DebugPrintf("Syntax: pass_codes <0/1>\n");
	}

	return true;
}

#pragma mark -

#ifdef ENABLE_LOL
Debugger_LoL::Debugger_LoL(LoLEngine *vm) : Debugger(vm), _vm(vm) {
}
#endif // ENABLE_LOL

#ifdef ENABLE_EOB
Debugger_EoB::Debugger_EoB(EoBCoreEngine *vm) : Debugger(vm), _vm(vm) {
}

void Debugger_EoB::initialize() {
	DCmd_Register("import_savefile",         WRAP_METHOD(Debugger_EoB, cmd_importSaveFile));
}

bool Debugger_EoB::cmd_importSaveFile(int argc, const char **argv) {
	if (!_vm->_allowImport) {
		DebugPrintf("This command may only be used from the main menu.\n");
		return true;
	}

	if (argc == 3) {
		int slot = atoi(argv[1]);
		if (slot < -1 || slot > 989) {
			DebugPrintf("slot must be between (including) -1 and 989 \n");
			return true;
		}

		DebugPrintf(_vm->importOriginalSaveFile(slot, argv[2]) ? "Success.\n" : "Failure.\n");
		_vm->loadItemDefs();
	} else {
		DebugPrintf("Syntax:   import_savefile <dest slot> <source file>\n              (Imports source save game file to dest slot.)\n          import_savefile -1\n              (Imports all original save game files found and puts them into the first available slots.)\n\n");
	}

	return true;
}

#endif // ENABLE_EOB

} // End of namespace Kyra
