/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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
 */


#include "common/memstream.h"
#include "common/rect.h"
#include "common/system.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/console.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/memory.h"
#include "sword2/mouse.h"
#include "sword2/resman.h"
#include "sword2/screen.h"
#include "sword2/sound.h"

namespace Sword2 {

Debugger::Debugger(Sword2Engine *vm)
	: GUI::Debugger() {
	_vm = vm;

	memset(_debugTextBlocks, 0, sizeof(_debugTextBlocks));
	memset(_showVar, 0, sizeof(_showVar));

	_displayDebugText = false;	// "INFO"
	_displayWalkGrid = false;	// "WALKGRID"
	_displayMouseMarker = false;	// "MOUSE"
	_displayTime = false;		// "TIME"
	_displayPlayerMarker = false;	// "PLAYER"
	_displayTextNumbers = false;	// "TEXT"

	_definingRectangles = false;	// "RECT"
	_draggingRectangle = 0;		// 0 = waiting to start new rect
					// 1 = currently dragging a rectangle

	_rectX1 = _rectY1 = 0;
	_rectX2 = _rectY2 = 0;
	_rectFlicker = false;

	_testingSnR = false;		// "SAVEREST" - for system to kill all
					// object resources (except player) in
					// fnAddHuman()

	_speechScriptWaiting = 0;	// The id of whoever we're waiting for
					// in a speech script. See fnTheyDo(),
					// fnTheyDoWeWait(), fnWeWait(), and
					// fnTimedWait().

	_startTime = 0;			// "TIMEON" & "TIMEOFF" - system start
					// time

	_textNumber = 0;		// Current system text line number

	_graphNoFrames = 0;		// No. of frames in currently displayed
					// anim

	// Register commands

	DCmd_Register("continue", WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("q",        WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("mem",      WRAP_METHOD(Debugger, Cmd_Mem));
	DCmd_Register("tony",     WRAP_METHOD(Debugger, Cmd_Tony));
	DCmd_Register("res",      WRAP_METHOD(Debugger, Cmd_Res));
	DCmd_Register("reslist",  WRAP_METHOD(Debugger, Cmd_ResList));
	DCmd_Register("starts",   WRAP_METHOD(Debugger, Cmd_Starts));
	DCmd_Register("start",    WRAP_METHOD(Debugger, Cmd_Start));
	DCmd_Register("s",        WRAP_METHOD(Debugger, Cmd_Start));
	DCmd_Register("info",     WRAP_METHOD(Debugger, Cmd_Info));
	DCmd_Register("walkgrid", WRAP_METHOD(Debugger, Cmd_WalkGrid));
	DCmd_Register("mouse",    WRAP_METHOD(Debugger, Cmd_Mouse));
	DCmd_Register("player",   WRAP_METHOD(Debugger, Cmd_Player));
	DCmd_Register("reslook",  WRAP_METHOD(Debugger, Cmd_ResLook));
	DCmd_Register("cur",      WRAP_METHOD(Debugger, Cmd_CurrentInfo));
	DCmd_Register("runlist",  WRAP_METHOD(Debugger, Cmd_RunList));
	DCmd_Register("kill",     WRAP_METHOD(Debugger, Cmd_Kill));
	DCmd_Register("nuke",     WRAP_METHOD(Debugger, Cmd_Nuke));
	DCmd_Register("var",      WRAP_METHOD(Debugger, Cmd_Var));
	DCmd_Register("rect",     WRAP_METHOD(Debugger, Cmd_Rect));
	DCmd_Register("clear",    WRAP_METHOD(Debugger, Cmd_Clear));
	DCmd_Register("debugon",  WRAP_METHOD(Debugger, Cmd_DebugOn));
	DCmd_Register("debugoff", WRAP_METHOD(Debugger, Cmd_DebugOff));
	DCmd_Register("saverest", WRAP_METHOD(Debugger, Cmd_SaveRest));
	DCmd_Register("timeon",   WRAP_METHOD(Debugger, Cmd_TimeOn));
	DCmd_Register("timeoff",  WRAP_METHOD(Debugger, Cmd_TimeOff));
	DCmd_Register("text",     WRAP_METHOD(Debugger, Cmd_Text));
	DCmd_Register("showvar",  WRAP_METHOD(Debugger, Cmd_ShowVar));
	DCmd_Register("hidevar",  WRAP_METHOD(Debugger, Cmd_HideVar));
	DCmd_Register("version",  WRAP_METHOD(Debugger, Cmd_Version));
	DCmd_Register("animtest", WRAP_METHOD(Debugger, Cmd_AnimTest));
	DCmd_Register("texttest", WRAP_METHOD(Debugger, Cmd_TextTest));
	DCmd_Register("linetest", WRAP_METHOD(Debugger, Cmd_LineTest));
	DCmd_Register("events",   WRAP_METHOD(Debugger, Cmd_Events));
	DCmd_Register("sfx",      WRAP_METHOD(Debugger, Cmd_Sfx));
	DCmd_Register("english",  WRAP_METHOD(Debugger, Cmd_English));
	DCmd_Register("finnish",  WRAP_METHOD(Debugger, Cmd_Finnish));
	DCmd_Register("polish",   WRAP_METHOD(Debugger, Cmd_Polish));
	DCmd_Register("fxq",      WRAP_METHOD(Debugger, Cmd_FxQueue));
}

void Debugger::varGet(int var) {
	DebugPrintf("%d\n", _vm->_logic->readVar(var));
}

void Debugger::varSet(int var, int val) {
	DebugPrintf("was %d, ", _vm->_logic->readVar(var));
	_vm->_logic->writeVar(var, val);
	DebugPrintf("now %d\n", _vm->_logic->readVar(var));
}

void Debugger::preEnter() {
	// Pause sound output
	if (_vm->_sound) {
		_vm->_sound->pauseFx();
		_vm->_sound->pauseSpeech();
		_vm->_sound->pauseMusic();
	}
}

void Debugger::postEnter() {
	if (_vm->_sound) {
		// Resume previous sound state
		_vm->_sound->unpauseFx();
		_vm->_sound->unpauseSpeech();
		_vm->_sound->unpauseMusic();
	}

	if (_vm->_mouse) {
		// Restore old mouse cursor
		_vm->_mouse->drawMouse();
	}
}

// Now the fun stuff: Commands

static int compare_blocks(const void *p1, const void *p2) {
	const MemBlock *m1 = *(const MemBlock * const *)p1;
	const MemBlock *m2 = *(const MemBlock * const *)p2;

	if (m1->size < m2->size)
		return 1;
	if (m1->size > m2->size)
		return -1;
	return 0;
}

bool Debugger::Cmd_Mem(int argc, const char **argv) {
	int16 numBlocks = _vm->_memory->getNumBlocks();
	MemBlock *memBlocks = _vm->_memory->getMemBlocks();

	MemBlock **blocks = (MemBlock **)malloc(numBlocks * sizeof(MemBlock));

	int i, j;

	for (i = 0, j = 0; i < MAX_MEMORY_BLOCKS; i++) {
		if (memBlocks[i].ptr)
			blocks[j++] = &memBlocks[i];
	}

	qsort(blocks, numBlocks, sizeof(MemBlock *), compare_blocks);

	DebugPrintf("     size id  res  type                 name\n");
	DebugPrintf("---------------------------------------------------------------------------\n");

	for (i = 0; i < numBlocks; i++) {
		const char *type;

		switch (_vm->_resman->fetchType(blocks[i]->ptr)) {
		case ANIMATION_FILE:
			type = "ANIMATION_FILE";
			break;
		case SCREEN_FILE:
			type = "SCREEN_FILE";
			break;
		case GAME_OBJECT:
			type  = "GAME_OBJECT";
			break;
		case WALK_GRID_FILE:
			type = "WALK_GRID_FILE";
			break;
		case GLOBAL_VAR_FILE:
			type = "GLOBAL_VAR_FILE";
			break;
		case PARALLAX_FILE_null:
			type = "PARALLAX_FILE_null";
			break;
		case RUN_LIST:
			type = "RUN_LIST";
			break;
		case TEXT_FILE:
			type = "TEXT_FILE";
			break;
		case SCREEN_MANAGER:
			type = "SCREEN_MANAGER";
			break;
		case MOUSE_FILE:
			type = "MOUSE_FILE";
			break;
		case WAV_FILE:
			type = "WAV_FILE";
			break;
		case ICON_FILE:
			type = "ICON_FILE";
			break;
		case PALETTE_FILE:
			type = "PALETTE_FILE";
			break;
		default:
			type = "<unknown>";
			break;
		}

		DebugPrintf("%9d %-3d %-4d %-20s %s\n",
				blocks[i]->size, blocks[i]->id, blocks[i]->uid,
				type, _vm->_resman->fetchName(blocks[i]->ptr));
	}

	free(blocks);

	DebugPrintf("---------------------------------------------------------------------------\n");
	DebugPrintf("%9d\n", _vm->_memory->getTotAlloc());

	return true;
}

bool Debugger::Cmd_Tony(int argc, const char **argv) {
	DebugPrintf("What about him?\n");
	return true;
}

bool Debugger::Cmd_Res(int argc, const char **argv) {
	uint32 numClusters = _vm->_resman->getNumClusters();

	if (!numClusters) {
		DebugPrintf("Argh! No resources!\n");
		return true;
	}

	ResourceFile *resFiles = _vm->_resman->getResFiles();

	for (uint i = 0; i < numClusters; i++) {
		const char *locStr[3] = { "HDD", "CD1", "CD2" };

		DebugPrintf("%-20s %s\n", resFiles[i].fileName, locStr[resFiles[i].cd]);
	}

	DebugPrintf("%d resources\n", _vm->_resman->getNumResFiles());
	return true;
}

bool Debugger::Cmd_ResList(int argc, const char **argv) {
	// By default, list only resources that are being held open.
	uint32 minCount = 1;

	if (argc > 1)
		minCount = atoi(argv[1]);

	uint32 numResFiles = _vm->_resman->getNumResFiles();
	Resource *resList = _vm->_resman->getResList();

	for (uint i = 0; i < numResFiles; i++) {
		if (resList[i].ptr && resList[i].refCount >= minCount) {
			DebugPrintf("%-4d: %-35s refCount: %-3d\n", i, _vm->_resman->fetchName(resList[i].ptr), resList[i].refCount);
		}
	}

	return true;
}

bool Debugger::Cmd_Starts(int argc, const char **argv) {
	uint32 numStarts = _vm->getNumStarts();

	if (!numStarts) {
		DebugPrintf("Sorry - no startup positions registered?\n");

		uint32 numScreenManagers = _vm->getNumScreenManagers();

		if (!numScreenManagers)
			DebugPrintf("There is a problem with startup.inf\n");
		else
			DebugPrintf(" (%d screen managers found in startup.inf)\n", numScreenManagers);
		return true;
	}

	StartUp *startList = _vm->getStartList();

	for (uint i = 0; i < numStarts; i++)
		DebugPrintf("%d  (%s)\n", i, startList[i].description);

	return true;
}

bool Debugger::Cmd_Start(int argc, const char **argv) {
	uint8 pal[3] = { 255, 255, 255 };

	if (argc != 2) {
		DebugPrintf("Usage: %s number\n", argv[0]);
		return true;
	}

	uint32 numStarts = _vm->getNumStarts();

	if (!numStarts) {
		DebugPrintf("Sorry - there are no startups!\n");
		return true;
	}

	int start = atoi(argv[1]);

	if (start < 0 || start >= (int)numStarts) {
		DebugPrintf("Not a legal start position\n");
		return true;
	}

	DebugPrintf("Running start %d\n", start);

	_vm->runStart(start);
	_vm->_screen->setPalette(187, 1, pal, RDPAL_INSTANT);
	return true;
}

bool Debugger::Cmd_Info(int argc, const char **argv) {
	_displayDebugText = !_displayDebugText;

	if (_displayDebugText)
		DebugPrintf("Info text on\n");
	else
		DebugPrintf("Info Text off\n");

	return true;
}

bool Debugger::Cmd_WalkGrid(int argc, const char **argv) {
	_displayWalkGrid = !_displayWalkGrid;

	if (_displayWalkGrid)
		DebugPrintf("Walk-grid display on\n");
	else
		DebugPrintf("Walk-grid display off\n");

	return true;
}

bool Debugger::Cmd_Mouse(int argc, const char **argv) {
	_displayMouseMarker = !_displayMouseMarker;

	if (_displayMouseMarker)
		DebugPrintf("Mouse marker on\n");
	else
		DebugPrintf("Mouse marker off\n");

	return true;
}

bool Debugger::Cmd_Player(int argc, const char **argv) {
	_displayPlayerMarker = !_displayPlayerMarker;

	if (_displayPlayerMarker)
		DebugPrintf("Player feet marker on\n");
	else
		DebugPrintf("Player feet marker off\n");

	return true;
}

bool Debugger::Cmd_ResLook(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s number\n", argv[0]);
		return true;
	}

	int res = atoi(argv[1]);
	uint32 numResFiles = _vm->_resman->getNumResFiles();

	if (res < 0 || res >= (int)numResFiles) {
		DebugPrintf("Illegal resource %d. There are %d resources, 0-%d.\n",
			res, numResFiles, numResFiles - 1);
		return true;
	}

	if (!_vm->_resman->checkValid(res)) {
		DebugPrintf("%d is a null & void resource number\n", res);
		return true;
	}

	// Open up the resource and take a look inside!
	uint8 type = _vm->_resman->fetchType(res);

	switch (type) {
	case ANIMATION_FILE:
		DebugPrintf("<anim> %s\n", _vm->_resman->fetchName(res));
		break;
	case SCREEN_FILE:
		DebugPrintf("<layer> %s\n", _vm->_resman->fetchName(res));
		break;
	case GAME_OBJECT:
		DebugPrintf("<game object> %s\n", _vm->_resman->fetchName(res));
		break;
	case WALK_GRID_FILE:
		DebugPrintf("<walk grid> %s\n", _vm->_resman->fetchName(res));
		break;
	case GLOBAL_VAR_FILE:
		DebugPrintf("<global variables> %s\n", _vm->_resman->fetchName(res));
		break;
	case PARALLAX_FILE_null:
		DebugPrintf("<parallax file NOT USED!> %s\n", _vm->_resman->fetchName(res));
		break;
	case RUN_LIST:
		DebugPrintf("<run list> %s\n", _vm->_resman->fetchName(res));
		break;
	case TEXT_FILE:
		DebugPrintf("<text file> %s\n", _vm->_resman->fetchName(res));
		break;
	case SCREEN_MANAGER:
		DebugPrintf("<screen manager> %s\n", _vm->_resman->fetchName(res));
		break;
	case MOUSE_FILE:
		DebugPrintf("<mouse pointer> %s\n", _vm->_resman->fetchName(res));
		break;
	case ICON_FILE:
		DebugPrintf("<menu icon> %s\n", _vm->_resman->fetchName(res));
		break;
	default:
		DebugPrintf("unrecognized fileType %d\n", type);
		break;
	}

	return true;
}

bool Debugger::Cmd_CurrentInfo(int argc, const char **argv) {
	// prints general stuff about the screen, etc.
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	if (screenInfo->background_layer_id) {
		DebugPrintf("background layer id %d\n", screenInfo->background_layer_id);
		DebugPrintf("%d wide, %d high\n", screenInfo->screen_wide, screenInfo->screen_deep);
		DebugPrintf("%d normal layers\n", screenInfo->number_of_layers);

		Cmd_RunList(argc, argv);
	} else
		DebugPrintf("No screen\n");
	return true;
}

bool Debugger::Cmd_RunList(int argc, const char **argv) {
	uint32 runList = _vm->_logic->getRunList();

	if (runList) {
		Common::MemoryReadStream readS(_vm->_resman->openResource(runList), _vm->_resman->fetchLen(runList));

		readS.seek(ResHeader::size());

		DebugPrintf("Runlist number %d\n", runList);

		while (1) {
			uint32 res = readS.readUint32LE();
			if (!res)
				break;

			DebugPrintf("%d %s\n", res, _vm->_resman->fetchName(res));
		}

		_vm->_resman->closeResource(runList);
	} else
		DebugPrintf("No run list set\n");

	return true;
}

bool Debugger::Cmd_Kill(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s number\n", argv[0]);
		return true;
	}

	int res = atoi(argv[1]);
	uint32 numResFiles = _vm->_resman->getNumResFiles();

	if (res < 0 || res >= (int)numResFiles) {
		DebugPrintf("Illegal resource %d. There are %d resources, 0-%d.\n",
			res, numResFiles, numResFiles - 1);
		return true;
	}

	Resource *resList = _vm->_resman->getResList();

	if (!resList[res].ptr) {
		DebugPrintf("Resource %d is not in memory\n", res);
		return true;
	}

	if (resList[res].refCount) {
		DebugPrintf("Resource %d is open - cannot remove\n", res);
		return true;
	}

	_vm->_resman->remove(res);
	DebugPrintf("Trashed %d\n", res);
	return true;
}

bool Debugger::Cmd_Nuke(int argc, const char **argv) {
	DebugPrintf("Killing all resources except variable file and player object\n");
	_vm->_resman->killAll(true);
	return true;
}

bool Debugger::Cmd_Var(int argc, const char **argv) {
	switch (argc) {
	case 2:
		varGet(atoi(argv[1]));
		break;
	case 3:
		varSet(atoi(argv[1]), atoi(argv[2]));
		break;
	default:
		DebugPrintf("Usage: %s number value\n", argv[0]);
		break;
	}

	return true;
}

bool Debugger::Cmd_Rect(int argc, const char **argv) {
	uint32 filter = _vm->setInputEventFilter(0);

	_definingRectangles = !_definingRectangles;

	if (_definingRectangles) {
		_vm->setInputEventFilter(filter & ~(RD_LEFTBUTTONUP | RD_RIGHTBUTTONUP));
		DebugPrintf("Mouse rectangles enabled\n");
	} else {
		_vm->setInputEventFilter(filter | RD_LEFTBUTTONUP | RD_RIGHTBUTTONUP);
		DebugPrintf("Mouse rectangles disabled\n");
	}

	_draggingRectangle = 0;
	return true;
}

bool Debugger::Cmd_Clear(int argc, const char **argv) {
	_vm->_resman->killAllObjects(true);
	return true;
}

bool Debugger::Cmd_DebugOn(int argc, const char **argv) {
	_displayDebugText = true;
	_displayWalkGrid = true;
	_displayMouseMarker = true;
	_displayPlayerMarker = true;
	_displayTextNumbers = true;
	DebugPrintf("Enabled all on-screen debug info\n");
	return true;
}

bool Debugger::Cmd_DebugOff(int argc, const char **argv) {
	_displayDebugText = false;
	_displayWalkGrid = false;
	_displayMouseMarker = false;
	_displayPlayerMarker = false;
	_displayTextNumbers = false;
	DebugPrintf("Disabled all on-screen debug info\n");
	return true;
}

bool Debugger::Cmd_SaveRest(int argc, const char **argv) {
	_testingSnR = !_testingSnR;

	if (_testingSnR)
		DebugPrintf("Enabled S&R logic_script stability checking\n");
	else
		DebugPrintf("Disabled S&R logic_script stability checking\n");

	return true;
}

bool Debugger::Cmd_TimeOn(int argc, const char **argv) {
	if (argc == 2)
		_startTime = _vm->_system->getMillis() - atoi(argv[1]) * 1000;
	else if (_startTime == 0)
		_startTime = _vm->_system->getMillis();
	_displayTime = true;
	DebugPrintf("Timer display on\n");
	return true;
}

bool Debugger::Cmd_TimeOff(int argc, const char **argv) {
	_displayTime = false;
	DebugPrintf("Timer display off\n");
	return true;
}

bool Debugger::Cmd_Text(int argc, const char **argv) {
	_displayTextNumbers = !_displayTextNumbers;

	if (_displayTextNumbers)
		DebugPrintf("Text numbers on\n");
	else
		DebugPrintf("Text numbers off\n");

	return true;
}

bool Debugger::Cmd_ShowVar(int argc, const char **argv) {
	int32 showVarNo = 0;
	int32 varNo;

	if (argc != 2) {
		DebugPrintf("Usage: %s number\n", argv[0]);
		return true;
	}

	varNo = atoi(argv[1]);

	// search for a spare slot in the watch-list, but also watch out for
	// this variable already being in the list

	while (showVarNo < MAX_SHOWVARS && _showVar[showVarNo] != 0 && _showVar[showVarNo] != varNo)
		showVarNo++;

	// if we've found a spare slot or the variable's already there
	if (showVarNo < MAX_SHOWVARS) {
		if (_showVar[showVarNo] == 0) {
			// empty slot - add it to the list at this slot
			_showVar[showVarNo] = varNo;
			DebugPrintf("var(%d) added to the watch-list\n", varNo);
		} else
			DebugPrintf("var(%d) already in the watch-list!\n", varNo);
	} else
		DebugPrintf("Sorry - no more allowed - hide one or extend the system watch-list\n");

	return true;
}

bool Debugger::Cmd_HideVar(int argc, const char **argv) {
	int32 showVarNo = 0;
	int32 varNo;

	if (argc != 2) {
		DebugPrintf("Usage: %s number\n", argv[0]);
		return true;
	}

	varNo = atoi(argv[1]);

	// search for 'varNo' in the watch-list
	while (showVarNo < MAX_SHOWVARS && _showVar[showVarNo] != varNo)
		showVarNo++;

	if (showVarNo < MAX_SHOWVARS) {
		// We've found 'varNo' in the list - clear this slot
		_showVar[showVarNo] = 0;
		DebugPrintf("var(%d) removed from watch-list\n", varNo);
	} else
		DebugPrintf("Sorry - can't find var(%d) in the list\n", varNo);

	return true;
}

bool Debugger::Cmd_Version(int argc, const char **argv) {
	// This function used to print more information, but nothing we
	// particularly care about.

	DebugPrintf("\"Broken Sword II\" (c) Revolution Software 1997.\n");
	return true;
}

bool Debugger::Cmd_AnimTest(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s value\n", argv[0]);
		return true;
	}

	// Automatically do "s 32" to run the animation testing start script
	_vm->runStart(32);

	// Same as typing "VAR 912 <value>" at the console
	varSet(912, atoi(argv[1]));

	DebugPrintf("Setting flag 'system_testing_anims'\n");
	return true;
}

bool Debugger::Cmd_TextTest(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s value\n", argv[0]);
		return true;
	}

	// Automatically do "s 33" to run the text/speech testing start script
	_vm->runStart(33);

	// Same as typing "VAR 1230 <value>" at the console
	varSet(1230, atoi(argv[1]));

	_displayTextNumbers = true;

	DebugPrintf("Setting flag 'system_testing_text'\n");
	DebugPrintf("Text numbers on\n");
	return true;
}

bool Debugger::Cmd_LineTest(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: %s value1 value2\n", argv[0]);
		return true;
	}

	// Automatically do "s 33" to run the text/speech testing start script
	_vm->runStart(33);

	// Same as typing "VAR 1230 <value>" at the console
	varSet(1230, atoi(argv[1]));

	// Same as typing "VAR 1264 <value>" at the console
	varSet(1264, atoi(argv[2]));

	_displayTextNumbers = true;

	DebugPrintf("Setting flag 'system_testing_text'\n");
	DebugPrintf("Setting flag 'system_test_line_no'\n");
	DebugPrintf("Text numbers on\n");
	return true;
}

bool Debugger::Cmd_Events(int argc, const char **argv) {
	EventUnit *eventList = _vm->_logic->getEventList();

	DebugPrintf("EVENT LIST:\n");

	for (uint32 i = 0; i < MAX_events; i++) {
		if (eventList[i].id) {
			uint32 target = eventList[i].id;
			uint32 script = eventList[i].interact_id;

			DebugPrintf("slot %2d: id = %s (%d)\n", i, _vm->_resman->fetchName(target), target);
			DebugPrintf("         script = %s (%d) pos %d\n", _vm->_resman->fetchName(script / 65536), script / 65536, script % 65536);
		}
	}

	return true;
}

bool Debugger::Cmd_Sfx(int argc, const char **argv) {
	_vm->_wantSfxDebug = !_vm->_wantSfxDebug;

	if (_vm->_wantSfxDebug)
		DebugPrintf("SFX logging activated\n");
	else
		DebugPrintf("SFX logging deactivated\n");

	return true;
}

bool Debugger::Cmd_English(int argc, const char **argv) {
	_vm->initializeFontResourceFlags(DEFAULT_TEXT);
	DebugPrintf("Default fonts selected\n");
	return true;
}

bool Debugger::Cmd_Finnish(int argc, const char **argv) {
	_vm->initializeFontResourceFlags(FINNISH_TEXT);
	DebugPrintf("Finnish fonts selected\n");
	return true;
}

bool Debugger::Cmd_Polish(int argc, const char **argv) {
	_vm->initializeFontResourceFlags(POLISH_TEXT);
	DebugPrintf("Polish fonts selected\n");
	return true;
}

bool Debugger::Cmd_FxQueue(int argc, const char **argv) {
	_vm->_sound->printFxQueue();
	return true;
}

} // End of namespace Sword2
