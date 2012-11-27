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

#include "common/system.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "parallaction/parallaction.h"
#include "parallaction/exec.h"
#include "parallaction/input.h"
#include "parallaction/parser.h"
#include "parallaction/saveload.h"
#include "parallaction/sound.h"
#include "parallaction/walk.h"

namespace Parallaction {


const char *Parallaction_br::_partNames[] = {
	"PART0",
	"PART1",
	"PART2",
	"PART3",
	"PART4"
};

Parallaction_br::Parallaction_br(OSystem* syst, const PARALLACTIONGameDescription *gameDesc) : Parallaction(syst, gameDesc),
	_locationParser(0), _programParser(0), _soundManI(0) {
}

Common::Error Parallaction_br::init() {

	_screenWidth = 640;
	_screenHeight = 400;

	if (getPlatform() == Common::kPlatformPC) {
		if (getFeatures() & GF_DEMO) {
			_disk = new DosDemoDisk_br(this);
		} else {
			_disk = new DosDisk_br(this);
		}
		_disk->setLanguage(2);					// NOTE: language is now hardcoded to English. Original used command-line parameters.
		_soundManI = new DosSoundMan_br(this);
	} else {
		_disk = new AmigaDisk_br(this);
		_disk->setLanguage(2);					// NOTE: language is now hardcoded to English. Original used command-line parameters.
		_soundManI = new AmigaSoundMan_br(this);
	}

	_disk->init();
	_soundMan = new SoundMan(_soundManI);

	initResources();
	initFonts();
	_locationParser = new LocationParser_br(this);
	_locationParser->init();
	_programParser = new ProgramParser_br(this);
	_programParser->init();

	_cmdExec = new CommandExec_br(this);
	_programExec = new ProgramExec_br(this);

	_walker = new PathWalker_BR(this);

	_part = -1;
	_nextPart = -1;

	_subtitle[0] = 0;
	_subtitle[1] = 0;

	memset(_zoneFlags, 0, sizeof(_zoneFlags));

	_countersNames = 0;

	_saveLoad = new SaveLoad_br(this, _saveFileMan);

	initInventory();
	setupBalloonManager();

	Parallaction::init();

	return Common::kNoError;
}

Parallaction_br::~Parallaction_br() {
	freeFonts();
	freeCharacter();

	destroyInventory();

	delete _objects;

	delete _locationParser;
	delete _programParser;

	_location._animations.remove(_char._ani);

	delete _walker;
}

void Parallaction_br::callFunction(uint index, void* parm) {
	assert(index < 6);	// magic value 6 is maximum # of callables for Big Red Adventure

	(this->*_callables[index])(parm);
}

bool Parallaction_br::processGameEvent(int event) {
	if (event == kEvNone) {
		return true;
	}

	bool c = true;
	_input->stopHovering();

	switch (event) {
	case kEvIngameMenu:
		startIngameMenu();
		c = false;
		break;
	}

	_input->setArrowCursor();

	return c;
}

Common::Error Parallaction_br::go() {

	bool splash = true;

	while (!shouldQuit()) {

		if (getFeatures() & GF_DEMO) {
			scheduleLocationSwitch("camalb");
			_nextPart = 1;
			_input->_inputMode = Input::kInputModeGame;
		} else {
			startGui(splash);
			 // don't show splash after first time
			splash = false;
		}

//		initCharacter();

		while (((g_engineFlags & kEngineReturn) == 0) && (!shouldQuit())) {
			runGame();
		}
		g_engineFlags &= ~kEngineReturn;

		cleanupGame();
	}

	return Common::kNoError;
}


void Parallaction_br::freeFonts() {
	delete _menuFont;
	_menuFont  = 0;

	delete _dialogueFont;
	_dialogueFont = 0;

	// no need to delete _labelFont, since it is using the same buffer as _menuFont
	_labelFont = 0;
}


void Parallaction_br::runPendingZones() {
	ZonePtr z;

	_cmdExec->runSuspended();

	if (_activeZone) {
		z = _activeZone;	// speak Zone or sound
		_activeZone.reset();
		if (ACTIONTYPE(z) == kZoneSpeak && z->u._speakDialogue) {
			enterDialogueMode(z);
		} else {
			runZone(z);			// FIXME: BRA doesn't handle sound yet
		}
	}

	if (_activeZone2) {
		z = _activeZone2;	// speak Zone or sound
		_activeZone2.reset();
		if (ACTIONTYPE(z) == kZoneSpeak && z->u._speakDialogue) {
			enterDialogueMode(z);
		} else {
			runZone(z);			// FIXME: BRA doesn't handle sound yet
		}
	}
}

void Parallaction_br::freeCharacter() {
	_gfx->freeCharacterObjects();

	delete _char._talk;
	delete _char._ani->gfxobj;

	_char._talk = 0;
	_char._ani->gfxobj = 0;
}

void Parallaction_br::freeLocation(bool removeAll) {
	// free open location stuff
	clearSubtitles();

	_localFlagNames->clear();

	_gfx->freeLocationObjects();

	// save zone and animation flags
	ZoneList::iterator zit = _location._zones.begin();
	for ( ; zit != _location._zones.end(); ++zit) {
		restoreOrSaveZoneFlags(*zit, false);
	}
	AnimationList::iterator ait = _location._animations.begin();
	for ( ; ait != _location._animations.end(); ++ait) {
		restoreOrSaveZoneFlags(*ait, false);
	}

	_location._animations.remove(_char._ani);
	_location.cleanup(removeAll);
	_location._animations.push_front(_char._ani);

}

void Parallaction_br::cleanupGame() {
	freeLocation(true);

	freeCharacter();

	delete _globalFlagsNames;
	delete _objectsNames;
	delete _countersNames;

	_globalFlagsNames = 0;
	_objectsNames = 0;
	_countersNames = 0;

	_numLocations = 0;
	g_globalFlags = 0;
	memset(_localFlags, 0, sizeof(_localFlags));
	memset(_locationNames, 0, sizeof(_locationNames));
	memset(_zoneFlags, 0, sizeof(_zoneFlags));
}


void Parallaction_br::changeLocation() {
	if (_newLocationName.empty()) {
		return;
	}

	if (_nextPart != -1) {
		cleanupGame();

		// more cleanup needed for part changes (see also saveload)
		g_globalFlags = 0;
		cleanInventory(true);
		strcpy(_characterName1, "null");

		_part = _nextPart;

		if (getFeatures() & GF_DEMO) {
			assert(_part == 1);
		} else {
			assert(_part >= 0 && _part <= 4);
		}

		_disk->selectArchive(_partNames[_part]);

		memset(_counters, 0, sizeof(_counters));

		_globalFlagsNames = _disk->loadTable("global");
		_objectsNames = _disk->loadTable("objects");
		_countersNames = _disk->loadTable("counters");

		// TODO: maybe handle this into Disk
		delete _objects;
		if (getPlatform() == Common::kPlatformPC) {
			_objects = _disk->loadObjects("icone.ico");
		} else {
			_objects = _disk->loadObjects("icons.ico", _part);
		}

		parseLocation("common.slf");
	}

	freeLocation(false);
	// load new location
	strcpy(_location._name, _newLocationName.c_str());
	parseLocation(_location._name);

	if (_location._startPosition.x != -1000) {
		_char._ani->setFoot(_location._startPosition);
		_char._ani->setF(_location._startFrame);
	}

	// re-link the follower animation
	setFollower(_followerName);
	if (_follower) {
		Common::Point p = _location._followerStartPosition;
		if (p.x == -1000) {
			_char._ani->getFoot(p);
		}
		_follower->setFoot(p);
		_follower->setF(_location._followerStartFrame);
	}

	_location._startPosition.x = -1000;
	_location._startPosition.y = -1000;
	_location._followerStartPosition.x = -1000;
	_location._followerStartPosition.y = -1000;

	_gfx->setScrollPosX(0);
	_gfx->setScrollPosY(0);
	if (_char._ani->gfxobj) {
		Common::Point foot;
		_char._ani->getFoot(foot);

		if (foot.x > 550)
			_gfx->setScrollPosX(320);

		if (foot.y > 350)
			_gfx->setScrollPosY(foot.y - 350);
	}

	// kFlagsRemove is cleared because the character is visible by default.
	// Commands can hide the character, anyway.
	_char._ani->_flags &= ~kFlagsRemove;
	_cmdExec->run(_location._commands);

	doLocationEnterTransition();

	_cmdExec->run(_location._aCommands);

	// NOTE: music should not started here!
	// TODO: implement the music commands which control music execution
	_soundMan->execute(SC_PLAYMUSIC);

	g_engineFlags &= ~kEngineChangeLocation;
	_newLocationName.clear();
	_nextPart = -1;
}

// FIXME: Parallaction_br::parseLocation() is now a verbatim copy of the same routine from Parallaction_ns.
void Parallaction_br::parseLocation(const char *filename) {
	debugC(1, kDebugParser, "parseLocation('%s')", filename);

	// find a new available slot
	allocateLocationSlot(filename);
	Script *script = _disk->loadLocation(filename);

	// parse the text file
	LocationParserOutput_br out;
	_locationParser->parse(script, &out);
	assert(out._info);
	delete script;

	bool visited = getLocationFlags() & kFlagsVisited;

	// load background, mask and path
	_disk->loadScenery(*out._info,
		out._backgroundName.empty() ? 0 : out._backgroundName.c_str(),
		out._maskName.empty()       ? 0 : out._maskName.c_str(),
		out._pathName.empty()       ? 0 : out._pathName.c_str());
	// assign background
	_gfx->setBackground(kBackgroundLocation, out._info);


	// process zones
	ZoneList::iterator zit = _location._zones.begin();
	for ( ; zit != _location._zones.end(); ++zit) {
		ZonePtr z = *zit;
		// restore the flags if the location has already been visited
		restoreOrSaveZoneFlags(z, visited);

		// (re)link the bounding animation if needed
		if (z->_flags & kFlagsAnimLinked) {
			z->_linkedAnim = _location.findAnimation(z->_linkedName.c_str());
		}

		bool visible = (z->_flags & kFlagsRemove) == 0;
		if (visible) {
			showZone(z, visible);
		}
	}

	// load the character (must be done before animations are processed)
	if (!out._characterName.empty()) {
		changeCharacter(out._characterName.c_str());
	}

	// process animations
	AnimationList::iterator ait = _location._animations.begin();
	for ( ; ait != _location._animations.end(); ++ait) {
		// restore the flags if the location has already been visited
		restoreOrSaveZoneFlags(*ait, visited);

		// load the script
		if ((*ait)->_scriptName) {
			loadProgram(*ait, (*ait)->_scriptName);
		}
	}

	debugC(1, kDebugParser, "parseLocation('%s') done", filename);
	return;
}

void Parallaction_br::loadProgram(AnimationPtr a, const char *filename) {
	debugC(1, kDebugParser, "loadProgram(Animation: %s, script: %s)", a->_name, filename);

	Script *script = _disk->loadScript(filename);
	ProgramPtr program(new Program);
	program->_anim = a;

	_programParser->parse(script, program);

	delete script;

	_location._programs.push_back(program);

	debugC(1, kDebugParser, "loadProgram() done");

	return;
}



void Parallaction_br::changeCharacter(const char *name) {

	const char *charName = _char.getName();

	if (scumm_stricmp(charName, name)) {
		freeCharacter();

		debugC(1, kDebugExec, "changeCharacter(%s)", name);

		_char.setName(name);
		_char._ani->gfxobj = _gfx->loadCharacterAnim(name);
		_char._talk = _disk->loadTalk(name);

		/* TODO: adjust inventories as following
		 * 1) if not on game load, then copy _inventory to the right slot of _charInventories
		 * 2) copy the new inventory from the right slot of _charInventories
		 */
	}

	_char._ani->_flags |= kFlagsActive;
}

bool Parallaction_br::counterExists(const Common::String &name) {
	return Table::notFound != _countersNames->lookup(name.c_str());
}

int	Parallaction_br::getCounterValue(const Common::String &name) {
	int index = _countersNames->lookup(name.c_str());
	if (index != Table::notFound) {
		return _counters[index - 1];
	}
	return 0;
}

void Parallaction_br::setCounterValue(const Common::String &name, int value) {
	int index = _countersNames->lookup(name.c_str());
	if (index != Table::notFound) {
		_counters[index - 1] = value;
	}
}

void Parallaction_br::testCounterCondition(const Common::String &name, int op, int value) {
	int index = _countersNames->lookup(name.c_str());
	if (index == Table::notFound) {
		clearLocationFlags(kFlagsTestTrue);
		return;
	}

	int c = _counters[index - 1];

// these definitions must match those in parser_br.cpp
#define CMD_TEST		25
#define CMD_TEST_GT		26
#define CMD_TEST_LT		27

	bool res = false;
	switch (op) {
	case CMD_TEST:
		res = (c == value);
		break;

	case CMD_TEST_GT:
		res = (c > value);
		break;

	case CMD_TEST_LT:
		res = (c < value);
		break;

	default:
		error("unknown operator in testCounterCondition");
	}

	if (res) {
		setLocationFlags(kFlagsTestTrue);
	} else {
		clearLocationFlags(kFlagsTestTrue);
	}
}

void Parallaction_br::updateWalkers() {
	_walker->walk();
}

void Parallaction_br::scheduleWalk(int16 x, int16 y, bool fromUser) {
	AnimationPtr a = _char._ani;

	if ((a->_flags & kFlagsRemove) || (a->_flags & kFlagsActive) == 0) {
		return;
	}

	_walker->setCharacterPath(a, x, y);

	if (!fromUser) {
		_walker->stopFollower();
	} else {
		if (_follower) {
			_walker->setFollowerPath(_follower, x, y);
		}
	}

	g_engineFlags |= kEngineWalking;
}

void Parallaction_br::setFollower(const Common::String &name) {
	if (name.empty()) {
		_followerName.clear();
		_follower.reset();
	} else {
		_followerName = name;
		_follower = _location.findAnimation(name.c_str());
	}
}

void Parallaction_br::restoreOrSaveZoneFlags(ZonePtr z, bool restore) {
	if ((z->_locationIndex == INVALID_LOCATION_INDEX) || (z->_index == INVALID_ZONE_INDEX)) {
		return;
	}

	if (restore) {
		z->_flags = _zoneFlags[z->_locationIndex][z->_index];
	} else {
		_zoneFlags[z->_locationIndex][z->_index] = z->_flags;
	}
}

int Parallaction_br::getSfxStatus() {
	if (!_soundManI) {
		return -1;
	}
	return _soundManI->isSfxEnabled() ? 1 : 0;
}

int Parallaction_br::getMusicStatus() {
	if (!_soundManI) {
		return -1;
	}
	return _soundManI->isMusicEnabled() ? 1 : 0;
}

void Parallaction_br::enableSfx(bool enable) {
	if (_soundManI) {
		_soundManI->enableSfx(enable);
	}
}

void Parallaction_br::enableMusic(bool enable) {
	if (_soundManI) {
		_soundManI->enableMusic(enable);
	}
}

} // namespace Parallaction
