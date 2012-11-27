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

#include "common/debug-channels.h"
#include "common/file.h"
#include "common/str.h"
#include "common/system.h"
#include "common/util.h"

#include "scumm/actor.h"
#include "scumm/boxes.h"
#include "scumm/debugger.h"
#include "scumm/imuse/imuse.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"

namespace Scumm {

void debugC(int channel, const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	// FIXME: Still spew all debug at -d9, for crashes in startup etc.
	//	  Add setting from commandline ( / abstract channel interface)
	if (!DebugMan.isDebugChannelEnabled(channel) && (gDebugLevel < 9))
		return;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	debug("%s", buf);
}

ScummDebugger::ScummDebugger(ScummEngine *s)
	: GUI::Debugger() {
	_vm = s;

	// Register variables
	DVar_Register("scumm_speed", &_vm->_fastMode, DVAR_BYTE, 0);
	DVar_Register("scumm_room", &_vm->_currentRoom, DVAR_BYTE, 0);
	DVar_Register("scumm_roomresource", &_vm->_roomResource, DVAR_INT, 0);
	DVar_Register("scumm_vars", &_vm->_scummVars, DVAR_INTARRAY, _vm->_numVariables);

	// Register commands
	DCmd_Register("continue",  WRAP_METHOD(ScummDebugger, Cmd_Exit));
	DCmd_Register("restart",   WRAP_METHOD(ScummDebugger, Cmd_Restart));

	DCmd_Register("actor",     WRAP_METHOD(ScummDebugger, Cmd_Actor));
	DCmd_Register("actors",    WRAP_METHOD(ScummDebugger, Cmd_PrintActor));
	DCmd_Register("box",       WRAP_METHOD(ScummDebugger, Cmd_PrintBox));
	DCmd_Register("matrix",    WRAP_METHOD(ScummDebugger, Cmd_PrintBoxMatrix));
	DCmd_Register("camera",    WRAP_METHOD(ScummDebugger, Cmd_Camera));
	DCmd_Register("room",      WRAP_METHOD(ScummDebugger, Cmd_Room));
	DCmd_Register("objects",   WRAP_METHOD(ScummDebugger, Cmd_PrintObjects));
	DCmd_Register("object",    WRAP_METHOD(ScummDebugger, Cmd_Object));
	DCmd_Register("script",    WRAP_METHOD(ScummDebugger, Cmd_Script));
	DCmd_Register("scr",       WRAP_METHOD(ScummDebugger, Cmd_Script));
	DCmd_Register("scripts",   WRAP_METHOD(ScummDebugger, Cmd_PrintScript));
	DCmd_Register("importres", WRAP_METHOD(ScummDebugger, Cmd_ImportRes));

	if (_vm->_game.id == GID_LOOM)
		DCmd_Register("drafts",  WRAP_METHOD(ScummDebugger, Cmd_PrintDraft));

	if (_vm->_game.id == GID_MONKEY && _vm->_game.platform == Common::kPlatformSegaCD)
		DCmd_Register("passcode",  WRAP_METHOD(ScummDebugger, Cmd_Passcode));

	DCmd_Register("loadgame",  WRAP_METHOD(ScummDebugger, Cmd_LoadGame));
	DCmd_Register("savegame",  WRAP_METHOD(ScummDebugger, Cmd_SaveGame));

	DCmd_Register("level",     WRAP_METHOD(ScummDebugger, Cmd_DebugLevel));
	DCmd_Register("debug",     WRAP_METHOD(ScummDebugger, Cmd_Debug));

	DCmd_Register("show",      WRAP_METHOD(ScummDebugger, Cmd_Show));
	DCmd_Register("hide",      WRAP_METHOD(ScummDebugger, Cmd_Hide));

	DCmd_Register("imuse",     WRAP_METHOD(ScummDebugger, Cmd_IMuse));

	DCmd_Register("resetcursors",    WRAP_METHOD(ScummDebugger, Cmd_ResetCursors));
}

ScummDebugger::~ScummDebugger() {
	 // we need this destructor, even if it is empty, for __SYMBIAN32__
}

///////////////////////////////////////////////////
// Now the fun stuff:

// Commands
bool ScummDebugger::Cmd_Restart(int argc, const char **argv) {
	_vm->restart();

	detach();
	return false;
}

bool ScummDebugger::Cmd_IMuse(int argc, const char **argv) {
	if (!_vm->_imuse && !_vm->_musicEngine) {
		DebugPrintf("No iMuse engine is active.\n");
		return true;
	}

	if (argc > 1) {
		if (!strcmp(argv[1], "panic")) {
			_vm->_musicEngine->stopAllSounds();
			DebugPrintf("AAAIIIEEEEEE!\n");
			DebugPrintf("Shutting down all music tracks\n");
			return true;
		} else if (!strcmp(argv[1], "play")) {
			if (argc > 2 && (!strcmp(argv[2], "random") || atoi(argv[2]) != 0)) {
				int sound = atoi(argv[2]);
				if (!strcmp(argv[2], "random")) {
					DebugPrintf("Selecting from %d songs...\n", _vm->_numSounds);
					sound = _vm->_rnd.getRandomNumber(_vm->_numSounds);
				}
				_vm->ensureResourceLoaded(rtSound, sound);
				_vm->_musicEngine->startSound(sound);

				DebugPrintf("Attempted to start music %d.\n", sound);
			} else {
				DebugPrintf("Specify a music resource # from 1-255.\n");
			}
			return true;
		} else if (!strcmp(argv[1], "stop")) {
			if (argc > 2 && (!strcmp(argv[2], "all") || atoi(argv[2]) != 0)) {
				if (!strcmp(argv[2], "all")) {
					_vm->_musicEngine->stopAllSounds();
					DebugPrintf("Shutting down all music tracks.\n");
				} else {
					_vm->_musicEngine->stopSound(atoi(argv[2]));
					DebugPrintf("Attempted to stop music %d.\n", atoi(argv[2]));
				}
			} else {
				DebugPrintf("Specify a music resource # or \"all\".\n");
			}
			return true;
		}
	}

	DebugPrintf("Available iMuse commands:\n");
	DebugPrintf("  panic - Stop all music tracks\n");
	DebugPrintf("  play # - Play a music resource\n");
	DebugPrintf("  stop # - Stop a music resource\n");
	return true;
}

bool ScummDebugger::Cmd_Room(int argc, const char **argv) {
	if (argc > 1) {
		int room = atoi(argv[1]);
		_vm->_actors[_vm->VAR(_vm->VAR_EGO)]->_room = room;
		_vm->_sound->stopAllSounds();
		_vm->startScene(room, 0, 0);
		_vm->_fullRedraw = true;
		return false;
	} else {
		DebugPrintf("Current room: %d [%d] - use 'room <roomnum>' to switch\n", _vm->_currentRoom, _vm->_roomResource);
		return true;
	}
}

bool ScummDebugger::Cmd_LoadGame(int argc, const char **argv) {
	if (argc > 1) {
		int slot = atoi(argv[1]);

		_vm->requestLoad(slot);

		detach();
		return false;
	}

	DebugPrintf("Syntax: loadgame <slotnum>\n");
	return true;
}

bool ScummDebugger::Cmd_SaveGame(int argc, const char **argv) {
	if (argc > 2) {
		int slot = atoi(argv[1]);

		_vm->requestSave(slot, argv[2]);
	} else
		DebugPrintf("Syntax: savegame <slotnum> <name>\n");

	return true;
}

bool ScummDebugger::Cmd_Show(int argc, const char **argv) {

	if (argc != 2) {
		DebugPrintf("Syntax: show <parameter>\n");
		return true;
	}

	if (!strcmp(argv[1], "hex")) {
		_vm->_hexdumpScripts = true;
		DebugPrintf("Script hex dumping on\n");
	} else if (!strncmp(argv[1], "sta", 3)) {
		_vm->_showStack = 1;
		DebugPrintf("Stack tracing on\n");
	} else {
		DebugPrintf("Unknown show parameter '%s'\nParameters are 'hex' for hex dumping and 'sta' for stack tracing\n", argv[1]);
	}
	return true;
}

bool ScummDebugger::Cmd_Hide(int argc, const char **argv) {

	if (argc != 2) {
		DebugPrintf("Syntax: hide <parameter>\n");
		return true;
	}

	if (!strcmp(argv[1], "hex")) {
		_vm->_hexdumpScripts = false;
		DebugPrintf("Script hex dumping off\n");
	} else if (!strncmp(argv[1], "sta", 3)) {
		_vm->_showStack = 0;
		DebugPrintf("Stack tracing off\n");
	} else {
		DebugPrintf("Unknown hide parameter '%s'\nParameters are 'hex' to turn off hex dumping and 'sta' to turn off stack tracing\n", argv[1]);
	}
	return true;
}

bool ScummDebugger::Cmd_Script(int argc, const char** argv) {
	int scriptnum;

	if (argc < 2) {
		DebugPrintf("Syntax: script <scriptnum> <command>\n");
		return true;
	}

	scriptnum = atoi(argv[1]);

	// FIXME: what is the max range on these?
	// if (scriptnum >= _vm->_numScripts) {
	//	DebugPrintf("Script number %d is out of range (range: 1 - %d)\n", scriptnum, _vm->_numScripts);
	//	return true;
	//}

	if ((!strcmp(argv[2], "kill")) || (!strcmp(argv[2], "stop"))) {
		_vm->stopScript(scriptnum);
	} else if ((!strcmp(argv[2], "run")) || (!strcmp(argv[2], "start"))) {
		_vm->runScript(scriptnum, 0, 0, 0);
		return false;
	} else {
		DebugPrintf("Unknown script command '%s'\nUse <kill/stop | run/start> as command\n", argv[2]);
	}

	return true;
}

bool ScummDebugger::Cmd_ImportRes(int argc, const char** argv) {
	Common::File file;
	uint32 size;
	int resnum;

	if (argc != 4) {
		DebugPrintf("Syntax: importres <restype> <filename> <resnum>\n");
		return true;
	}

	resnum = atoi(argv[3]);
	// FIXME add bounds check

	if (!strncmp(argv[1], "scr", 3)) {
		file.open(argv[2]);
		if (file.isOpen() == false) {
			DebugPrintf("Could not open file %s\n", argv[2]);
			return true;
		}
		if (_vm->_game.features & GF_SMALL_HEADER) {
			size = file.readUint16LE();
			file.seek(-2, SEEK_CUR);
		} else if (_vm->_game.features & GF_SMALL_HEADER) { // FIXME: This never was executed
			if (_vm->_game.version == 4)
				file.seek(8, SEEK_CUR);
			size = file.readUint32LE();
			file.readUint16LE();
			file.seek(-6, SEEK_CUR);
		} else {
			file.readUint32BE();
			size = file.readUint32BE();
			file.seek(-8, SEEK_CUR);
		}

		file.read(_vm->_res->createResource(rtScript, resnum, size), size);

	} else
		DebugPrintf("Unknown importres type '%s'\n", argv[1]);
	return true;
}

bool ScummDebugger::Cmd_PrintScript(int argc, const char **argv) {
	int i;
	ScriptSlot *ss = _vm->vm.slot;
	DebugPrintf("+-----------------------------------+\n");
	DebugPrintf("|# | num|offst|sta|typ|fr|rec|fc|cut|\n");
	DebugPrintf("+--+----+-----+---+---+--+---+--+---+\n");
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (ss->number) {
			DebugPrintf("|%2d|%4d|%05x|%3d|%3d|%2d|%3d|%2d|%3d|\n",
					i, ss->number, ss->offs, ss->status, ss->where,
					ss->freezeResistant, ss->recursive,
					ss->freezeCount, ss->cutsceneOverride);
		}
	}
	DebugPrintf("+-----------------------------------+\n");

	return true;
}

bool ScummDebugger::Cmd_Actor(int argc, const char **argv) {
	Actor *a;
	int actnum;
	int value = 0, value2 = 0;

	if (argc < 3) {
		DebugPrintf("Syntax: actor <actornum> <command> <parameter>\n");
		return true;
	}

	actnum = atoi(argv[1]);
	if (actnum >= _vm->_numActors) {
		DebugPrintf("Actor %d is out of range (range: 1 - %d)\n", actnum, _vm->_numActors);
		return true;
	}

	a = _vm->_actors[actnum];
	if (argc > 3)
		value = atoi(argv[3]);
	if (argc > 4)
		value2 = atoi(argv[4]);

	if (!strcmp(argv[2], "animvar")) {
		a->setAnimVar(value, value2);
		DebugPrintf("Actor[%d].animVar[%d] = %d\n", actnum, value, a->getAnimVar(value));
	} else if (!strcmp(argv[2], "anim")) {
		a->animateActor(value);
		DebugPrintf("Actor[%d].animateActor(%d)\n", actnum, value);
	} else if (!strcmp(argv[2], "ignoreboxes")) {
		a->_ignoreBoxes = (value > 0);
		DebugPrintf("Actor[%d].ignoreBoxes = %d\n", actnum, a->_ignoreBoxes);
	} else if (!strcmp(argv[2], "x")) {
		a->putActor(value, a->getRealPos().y);
		DebugPrintf("Actor[%d].x = %d\n", actnum, a->getRealPos().x);
		_vm->_fullRedraw = true;
	} else if (!strcmp(argv[2], "y")) {
		a->putActor(a->getRealPos().x, value);
		DebugPrintf("Actor[%d].y = %d\n", actnum, a->getRealPos().y);
		_vm->_fullRedraw = true;
	} else if (!strcmp(argv[2], "_elevation")) {
		a->setElevation(value);
		DebugPrintf("Actor[%d]._elevation = %d\n", actnum, a->getElevation());
		_vm->_fullRedraw = true;
	} else if (!strcmp(argv[2], "costume")) {
		if (value >= (int)_vm->_res->_types[rtCostume].size())
			DebugPrintf("Costume not changed as %d exceeds max of %d\n", value, _vm->_res->_types[rtCostume].size());
		else {
			a->setActorCostume(value);
			_vm->_fullRedraw = true;
			DebugPrintf("Actor[%d].costume = %d\n", actnum, a->_costume);
		}
	} else if (!strcmp(argv[2], "name")) {
		DebugPrintf("Name of actor %d: %s\n", actnum,
			_vm->getObjOrActorName(_vm->actorToObj(actnum)));
	} else if (!strcmp(argv[2], "condmask")) {
		if (argc > 3) {
			a->_heCondMask = value;
		}
		DebugPrintf("Actor[%d]._heCondMask = 0x%X\n", actnum, a->_heCondMask);
	} else {
		DebugPrintf("Unknown actor command '%s'\nUse <ignoreboxes |costume> as command\n", argv[2]);
	}

	return true;

}
bool ScummDebugger::Cmd_PrintActor(int argc, const char **argv) {
	int i;
	Actor *a;

	DebugPrintf("+---------------------------------------------------------------+\n");
	DebugPrintf("|# |  x |  y | w | h |elev|cos|box|mov| zp|frm|scl|dir|   cls   |\n");
	DebugPrintf("+--+----+----+---+---+----+---+---+---+---+---+---+---+---------+\n");
	for (i = 1; i < _vm->_numActors; i++) {
		a = _vm->_actors[i];
		if (a->_visible)
			DebugPrintf("|%2d|%4d|%4d|%3d|%3d|%4d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|$%08x|\n",
						 a->_number, a->getRealPos().x, a->getRealPos().y, a->_width,  a->_bottom - a->_top,
						 a->getElevation(),
						 a->_costume, a->_walkbox, a->_moving, a->_forceClip, a->_frame,
						 a->_scalex, a->getFacing(), _vm->_classData[a->_number]);
	}
	DebugPrintf("\n");
	return true;
}

bool ScummDebugger::Cmd_PrintObjects(int argc, const char **argv) {
	int i;
	ObjectData *o;
	DebugPrintf("Objects in current room\n");
	DebugPrintf("+---------------------------------+------------+\n");
	DebugPrintf("|num |  x |  y |width|height|state|fl|   cls   |\n");
	DebugPrintf("+----+----+----+-----+------+-----+--+---------+\n");

	for (i = 1; i < _vm->_numLocalObjects; i++) {
		o = &(_vm->_objs[i]);
		if (o->obj_nr == 0)
			continue;
		int classData = (_vm->_game.version != 0 ? _vm->_classData[o->obj_nr] : 0);
		DebugPrintf("|%4d|%4d|%4d|%5d|%6d|%5d|%2d|$%08x|\n",
				o->obj_nr, o->x_pos, o->y_pos, o->width, o->height, o->state,
				o->fl_object_index, classData);
	}
	DebugPrintf("\n");

	return true;
}

bool ScummDebugger::Cmd_Object(int argc, const char **argv) {
	int i;
	int obj;

	if (argc < 3) {
		DebugPrintf("Syntax: object <objectnum> <command> <parameter>\n");
		return true;
	}

	obj = atoi(argv[1]);
	if (_vm->_game.version != 0 && obj >= _vm->_numGlobalObjects) {
		DebugPrintf("Object %d is out of range (range: 1 - %d)\n", obj, _vm->_numGlobalObjects);
		return true;
	}

	if (!strcmp(argv[2], "pickup")) {
		for (i = 0; i < _vm->_numInventory; i++) {
			if (_vm->_inventory[i] == (uint16)obj) {
				_vm->putOwner(obj, _vm->VAR(_vm->VAR_EGO));
				_vm->runInventoryScript(obj);
				return true;
			}
		}

		if (argc == 3)
			_vm->addObjectToInventory(obj, _vm->_currentRoom);
		else
			_vm->addObjectToInventory(obj, atoi(argv[3]));

		_vm->putOwner(obj, _vm->VAR(_vm->VAR_EGO));
		_vm->putClass(obj, kObjectClassUntouchable, 1);
		_vm->putState(obj, 1);
		_vm->markObjectRectAsDirty(obj);
		_vm->clearDrawObjectQueue();
		_vm->runInventoryScript(obj);
	} else if (!strcmp(argv[2], "state")) {
		if (argc == 4) {
			_vm->putState(obj, atoi(argv[3]));
			//is BgNeedsRedraw enough?
			_vm->_bgNeedsRedraw = true;
		} else {
			DebugPrintf("State of object %d: %d\n", obj, _vm->getState(obj));
		}
	} else if (!strcmp(argv[2], "name")) {
		DebugPrintf("Name of object %d: %s\n", obj, _vm->getObjOrActorName(obj));
	} else {
		DebugPrintf("Unknown object command '%s'\nUse <pickup | state | name> as command\n", argv[2]);
	}

	return true;
}

bool ScummDebugger::Cmd_Debug(int argc, const char **argv) {
	const Common::DebugManager::DebugChannelList &lvls = DebugMan.listDebugChannels();

	// No parameters given: Print out a list of all channels and their status
	if (argc <= 1) {
		DebugPrintf("Available debug channels:\n");
		for (Common::DebugManager::DebugChannelList::const_iterator i = lvls.begin(); i != lvls.end(); ++i) {
			DebugPrintf("%c%s - %s (%s)\n", i->enabled ? '+' : ' ',
					i->name.c_str(), i->description.c_str(),
					i->enabled ? "enabled" : "disabled");
		}
		return true;
	}

	// Enable or disable channel?
	bool result = false;
	if (argv[1][0] == '+') {
		result = DebugMan.enableDebugChannel(argv[1] + 1);
	} else if (argv[1][0] == '-') {
		result = DebugMan.disableDebugChannel(argv[1] + 1);
	}

	if (result) {
		DebugPrintf("%s %s\n", (argv[1][0] == '+') ? "Enabled" : "Disabled", argv[1] + 1);
	} else {
		DebugPrintf("Usage: debug [+CHANNEL|-CHANNEL]\n");
		DebugPrintf("Enables or disables the given debug channel.\n");
		DebugPrintf("When used without parameters, lists all available debug channels and their status.\n");
	}

	return true;
}

bool ScummDebugger::Cmd_DebugLevel(int argc, const char **argv) {
	if (argc == 1) {
		if (_vm->_debugMode == false)
			DebugPrintf("Debugging is not enabled at this time\n");
		else
			DebugPrintf("Debugging is currently set at level %d\n", gDebugLevel);
	} else { // set level
		gDebugLevel = atoi(argv[1]);
		if (gDebugLevel >= 0) {
			_vm->_debugMode = true;
			DebugPrintf("Debug level set to level %d\n", gDebugLevel);
		} else if (gDebugLevel < 0) {
			_vm->_debugMode = false;
			DebugPrintf("Debugging is now disabled\n");
		} else
			DebugPrintf("Not a valid debug level\n");
	}

	return true;
}

bool ScummDebugger::Cmd_Camera(int argc, const char **argv) {
	DebugPrintf("Camera: cur (%d,%d) - dest (%d,%d) - accel (%d,%d) -- last (%d,%d)\n",
		_vm->camera._cur.x, _vm->camera._cur.y, _vm->camera._dest.x, _vm->camera._dest.y,
		_vm->camera._accel.x, _vm->camera._accel.y, _vm->camera._last.x, _vm->camera._last.y);

	return true;
}

bool ScummDebugger::Cmd_PrintBox(int argc, const char **argv) {
	int num, i = 0;

	if (argc > 1) {
		for (i = 1; i < argc; i++)
			printBox(atoi(argv[i]));
	} else {
		num = _vm->getNumBoxes();
		DebugPrintf("\nWalk boxes:\n");
		for (i = 0; i < num; i++)
			printBox(i);
	}
	return true;
}

bool ScummDebugger::Cmd_PrintBoxMatrix(int argc, const char **argv) {
	byte *boxm = _vm->getBoxMatrixBaseAddr();
	int num = _vm->getNumBoxes();
	int i, j;

	DebugPrintf("Walk matrix:\n");
	if (_vm->_game.version <= 2)
		boxm += num;
	for (i = 0; i < num; i++) {
		DebugPrintf("%d: ", i);
		if (_vm->_game.version <= 2) {
			for (j = 0; j < num; j++)
				DebugPrintf("[%d] ", *boxm++);
		} else {
			while (*boxm != 0xFF) {
				DebugPrintf("[%d-%d=>%d] ", boxm[0], boxm[1], boxm[2]);
				boxm += 3;
			}
			boxm++;
		}
		DebugPrintf("\n");
	}
	return true;
}

void ScummDebugger::printBox(int box) {
	if (box < 0 || box >= _vm->getNumBoxes()) {
		DebugPrintf("%d is not a valid box!\n", box);
		return;
	}
	BoxCoords coords;
	int flags = _vm->getBoxFlags(box);
	int mask = _vm->getMaskFromBox(box);
	int scale = _vm->getBoxScale(box);

	coords = _vm->getBoxCoordinates(box);

	// Print out coords, flags, zbuffer mask
	DebugPrintf("%d: [%d x %d] [%d x %d] [%d x %d] [%d x %d], flags=0x%02x, mask=%d, scale=%d\n",
								box,
								coords.ul.x, coords.ul.y, coords.ll.x, coords.ll.y,
								coords.ur.x, coords.ur.y, coords.lr.x, coords.lr.y,
								flags, mask, scale);

	// Draw the box
	drawBox(box);
}

/************ ENDER: Temporary debug code for boxen **************/

static int gfxPrimitivesCompareInt(const void *a, const void *b);


static void hlineColor(ScummEngine *scumm, int x1, int x2, int y, byte color) {
	VirtScreen *vs = &scumm->_virtscr[kMainVirtScreen];
	byte *ptr;

	// Clip y
	y += scumm->_screenTop;
	if (y < 0 || y >= scumm->_screenHeight)
		return;

	if (x2 < x1)
		SWAP(x2, x1);

	// Clip x1 / x2
	const int left = scumm->_screenStartStrip * 8;
	const int right = scumm->_screenEndStrip * 8;
	if (x1 < left)
		x1 = left;
	if (x2 >= right)
		x2 = right - 1;


	ptr = (byte *)vs->pixels + x1 + y * vs->pitch;

	while (x1++ <= x2) {
		*ptr++ = color;
	}
}

static int gfxPrimitivesCompareInt(const void *a, const void *b) {
	return (*(const int *)a) - (*(const int *)b);
}

static void fillQuad(ScummEngine *scumm, Common::Point v[4], int color) {
	const int N = 4;
	int i;
	int y;
	int miny, maxy;
	Common::Point pt1, pt2;

	int polyInts[N];


	// Determine Y maxima
	miny = maxy = v[0].y;
	for (i = 1; i < N; i++) {
		if (v[i].y < miny) {
			miny = v[i].y;
		} else if (v[i].y > maxy) {
			maxy = v[i].y;
		}
	}

	// Draw, scanning y
	for (y = miny; y <= maxy; y++) {
		int ints = 0;
		for (i = 0; i < N; i++) {
			int ind1 = i;
			int ind2 = (i + 1) % N;
			pt1 = v[ind1];
			pt2 = v[ind2];
			if (pt1.y > pt2.y) {
				SWAP(pt1, pt2);
			}

			if (pt1.y <= y && y <= pt2.y) {
				if (y == pt1.y && y == pt2.y) {
					hlineColor(scumm, pt1.x, pt2.x, y, color);
				} else if ((y >= pt1.y) && (y < pt2.y)) {
					polyInts[ints++] = (y - pt1.y) * (pt2.x - pt1.x) / (pt2.y - pt1.y) + pt1.x;
				} else if ((y == maxy) && (y > pt1.y) && (y <= pt2.y)) {
					polyInts[ints++] = (y - pt1.y) * (pt2.x - pt1.x) / (pt2.y - pt1.y) + pt1.x;
				}
			}
		}
		qsort(polyInts, ints, sizeof(int), gfxPrimitivesCompareInt);

		for (i = 0; i < ints; i += 2) {
			hlineColor(scumm, polyInts[i], polyInts[i + 1], y, color);
		}
	}

	return;
}

void ScummDebugger::drawBox(int box) {
	BoxCoords coords;
	Common::Point r[4];

	coords = _vm->getBoxCoordinates(box);

	r[0] = coords.ul;
	r[1] = coords.ur;
	r[2] = coords.lr;
	r[3] = coords.ll;

	if (_vm->_game.version <= 2) {
		for (int i = 0; i < 4; ++i) {
			r[i].x *= V12_X_MULTIPLIER;
			r[i].y *= V12_Y_MULTIPLIER;
		}
	}

	// TODO - maybe use different colors for each box, and/or print the box number inside it?
	fillQuad(_vm, r, 13);

	VirtScreen *vs = _vm->findVirtScreen(coords.ul.y);
	if (vs != NULL)
		_vm->markRectAsDirty(vs->number, 0, vs->w, 0, vs->h);
	_vm->drawDirtyScreenParts();
	_vm->_system->updateScreen();
}

bool ScummDebugger::Cmd_PrintDraft(int argc, const char **argv) {
	const char *names[] = {
		"Opening",      "Straw to Gold", "Dyeing",
		"Night Vision",	"Twisting",      "Sleep",
		"Emptying",     "Invisibility",  "Terror",
		"Sharpening",   "Reflection",    "Healing",
		"Silence",      "Shaping",       "Unmaking",
		"Transcendence"
	};

	const char *notes = "cdefgabC";
	int i, base, draft;

	if (_vm->_game.id != GID_LOOM) {
		DebugPrintf("Command only works with Loom/LoomCD\n");
		return true;
	}

	// There are 16 drafts, stored from variable 50, 55 or 100 and upwards.
	// Each draft occupies two variables, the first of which contains the
	// notes for the draft and a number of flags.
	//
	// +---+---+---+---+-----+-----+-----+-----+
	// | A | B | C | D | 444 | 333 | 222 | 111 |
	// +---+---+---+---+-----+-----+-----+-----+
	//
	// A   Unknown
	// B   The player has used the draft successfully at least once
	// C   The player has knowledge of the draft
	// D   Unknown
	// 444 The fourth note
	// 333 The third note
	// 222 The second note
	// 111 The first note
	//
	// I don't yet know what the second variable is used for. Possibly to
	// store information on where and/or how the draft can be used. They
	// appear to remain constant throughout the game.

	if (_vm->_game.version == 4 || _vm->_game.platform == Common::kPlatformPCEngine) {
		// DOS CD version / PC-Engine version
		base = 100;
	} else if (_vm->_game.platform == Common::kPlatformMacintosh) {
		// Macintosh version
		base = 55;
	} else {
		// All (?) other versions
		base = 50;
	}

	if (argc == 2) {
		// We had to debug a problem at the end of the game that only
		// happened if you interrupted the intro at a specific point.
		// That made it useful with a command to learn all the drafts
		// and notes.

		if (strcmp(argv[1], "learn") == 0) {
			for (i = 0; i < 16; i++)
				_vm->_scummVars[base + 2 * i] |= 0x2000;
			_vm->_scummVars[base + 72] = 8;

			// In theory, we could run script 18 here to redraw
			// the distaff, but I don't know if that's a safe
			// thing to do.

			DebugPrintf("Learned all drafts and notes.\n");
			return true;
		}
	}

	// Probably the most useful command for ordinary use: list the drafts.

	for (i = 0; i < 16; i++) {
		draft = _vm->_scummVars[base + i * 2];
		DebugPrintf("%d %-13s %c%c%c%c %c%c\n",
			base + 2 * i,
			names[i],
			notes[draft & 0x0007],
			notes[(draft & 0x0038) >> 3],
			notes[(draft & 0x01c0) >> 6],
			notes[(draft & 0x0e00) >> 9],
			(draft & 0x2000) ? 'K' : ' ',
			(draft & 0x4000) ? 'U' : ' ');
	}

	return true;
}

bool ScummDebugger::Cmd_Passcode(int argc, const char **argv) {
	if (argc > 1) {
		_vm->_bootParam = atoi(argv[1]);
		int args[16];
		memset(args, 0, sizeof(args));
		args[0] = _vm->_bootParam;

		_vm->runScript(61, 0, 0, args);

		if (_vm->_bootParam != _vm->_scummVars[411]){
			DebugPrintf("Invalid Passcode\n");
			return true;
		}

		_vm->_bootParam = 0;
		detach();

	} else {
		DebugPrintf("Current Passcode is %d \nUse 'passcode <SEGA CD Passcode>'\n",_vm->_scummVars[411]);
		return true;
	}
	return false;
}

bool ScummDebugger::Cmd_ResetCursors(int argc, const char **argv) {
	_vm->resetCursors();
	detach();
	return false;
}

} // End of namespace Scumm
