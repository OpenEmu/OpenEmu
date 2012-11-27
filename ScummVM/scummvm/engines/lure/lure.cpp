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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/system.h"
#include "common/savefile.h"

#include "engines/util.h"

#include "lure/luredefs.h"
#include "lure/surface.h"
#include "lure/lure.h"
#include "lure/intro.h"
#include "lure/game.h"
#include "lure/sound.h"

namespace Lure {

static LureEngine *int_engine = NULL;

LureEngine::LureEngine(OSystem *system, const LureGameDescription *gameDesc)
	: Engine(system), _gameDescription(gameDesc), _rnd("lure") {

	DebugMan.addDebugChannel(kLureDebugScripts, "scripts", "Scripts debugging");
	DebugMan.addDebugChannel(kLureDebugAnimations, "animations", "Animations debugging");
	DebugMan.addDebugChannel(kLureDebugHotspots, "hotspots", "Hotspots debugging");
	DebugMan.addDebugChannel(kLureDebugFights, "fights", "Fights debugging");
	DebugMan.addDebugChannel(kLureDebugSounds, "sounds", "Sounds debugging");
	DebugMan.addDebugChannel(kLureDebugStrings, "strings", "Strings debugging");
}

Common::Error LureEngine::init() {
	int_engine = this;
	_initialized = false;
	_saveLoadAllowed = false;

	initGraphics(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT, false);

	// Check the version of the lure.dat file
	Common::File f;
	VersionStructure version;
	if (!f.open(SUPPORT_FILENAME)) {
		GUIError("Could not locate Lure support file");
		return Common::kUnknownError;
	}

	f.seek(0xbf * 8);
	f.read(&version, sizeof(VersionStructure));
	f.close();

	if (READ_LE_UINT16(&version.id) != 0xffff) {
		GUIError("Error validating %s - file is invalid or out of date", SUPPORT_FILENAME);
		return Common::kUnknownError;
	} else if ((version.vMajor != LURE_DAT_MAJOR) || (version.vMinor != LURE_DAT_MINOR)) {
		GUIError("Incorrect version of %s file - expected %d.%d but got %d.%d",
			SUPPORT_FILENAME, LURE_DAT_MAJOR, LURE_DAT_MINOR,
			version.vMajor, version.vMinor);
		return Common::kUnknownError;
	}

	_disk = new Disk();
	_resources = new Resources();
	_strings = new StringData();
	_screen = new Screen(*_system);
	_mouse = new Mouse();
	_events = new Events();
	_menu = new Menu();
	Surface::initialize();
	_room = new Room();
	_fights = new FightsManager();

	_gameToLoad = -1;
	_initialized = true;

	// Setup mixer
	syncSoundSettings();

	return Common::kNoError;
}

LureEngine::~LureEngine() {
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

	if (_initialized) {
		// Delete and deinitialize subsystems
		Surface::deinitialize();
		Sound.destroy();
		delete _fights;
		delete _room;
		delete _menu;
		delete _events;
		delete _mouse;
		delete _screen;
		delete _strings;
		delete _resources;
		delete _disk;
	}
}

LureEngine &LureEngine::getReference() {
	return *int_engine;
}

Common::Error LureEngine::go() {
	Game *gameInstance = new Game();

	// If requested, load a savegame instead of showing the intro
	if (ConfMan.hasKey("save_slot")) {
		_gameToLoad = ConfMan.getInt("save_slot");
		if (_gameToLoad < 0 || _gameToLoad > 999)
			_gameToLoad = -1;
	}

	if (_gameToLoad == -1) {
		if (ConfMan.getBool("copy_protection")) {
			CopyProtectionDialog *dialog = new CopyProtectionDialog();
			bool result = dialog->show();
			delete dialog;
			if (shouldQuit()) {
				delete gameInstance;
				return Common::kNoError;
			}

			if (!result)
				error("Sorry - copy protection failed");
		}

		if (ConfMan.getInt("boot_param") == 0) {
			// Show the introduction
			Sound.loadSection(Sound.isRoland() ? ROLAND_INTRO_SOUND_RESOURCE_ID : ADLIB_INTRO_SOUND_RESOURCE_ID);
			Introduction *intro = new Introduction();
			intro->show();
			delete intro;
		}
	}

	// Play the game
	if (!shouldQuit()) {
		// Play the game
		_saveLoadAllowed = true;
		Sound.loadSection(Sound.isRoland() ? ROLAND_MAIN_SOUND_RESOURCE_ID : ADLIB_MAIN_SOUND_RESOURCE_ID);
		gameInstance->execute();
	}

	delete gameInstance;
	return Common::kNoError;
}

void LureEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);

	if (pause) {
		Sound.pause();
	} else {
		Sound.resume();
	}
}

const char *LureEngine::generateSaveName(int slotNumber) {
	static char buffer[15];

	sprintf(buffer, "lure.%.3d", slotNumber);
	return buffer;
}

bool LureEngine::saveGame(uint8 slotNumber, Common::String &caption) {
	Common::WriteStream *f = this->_saveFileMan->openForSaving(
		generateSaveName(slotNumber));
	if (f == NULL)
		return false;

	f->write("lure", 5);
	f->writeByte(getLureLanguage());
	f->writeByte(LURE_SAVEGAME_MINOR);
	f->writeString(caption);
	f->writeByte(0); // End of string terminator

	Resources::getReference().saveToStream(f);
	Game::getReference().saveToStream(f);
	Sound.saveToStream(f);
	Fights.saveToStream(f);
	Room::getReference().saveToStream(f);

	delete f;
	return true;
}

#define FAILED_MSG "loadGame: Failed to load slot %d"

bool LureEngine::loadGame(uint8 slotNumber) {
	Common::ReadStream *f = this->_saveFileMan->openForLoading(
		generateSaveName(slotNumber));
	if (f == NULL)
		return false;

	// Check for header
	char buffer[5];
	f->read(buffer, 5);
	if (memcmp(buffer, "lure", 5) != 0) {
		warning(FAILED_MSG, slotNumber);
		delete f;
		return false;
	}

	// Check language version
	uint8 language = f->readByte();
	_saveVersion = f->readByte();
	if ((language != getLureLanguage()) || (_saveVersion < LURE_MIN_SAVEGAME_MINOR)) {
		warning("loadGame: Failed to load slot %d - incorrect version", slotNumber);
		delete f;
		return false;
	}

	// Read in and discard the savegame caption
	while (f->readByte() != 0)
		;

	// Load in the data
	Resources::getReference().loadFromStream(f);
	Game::getReference().loadFromStream(f);
	Sound.loadFromStream(f);
	Fights.loadFromStream(f);
	Room::getReference().loadFromStream(f);

	delete f;
	return true;
}

void LureEngine::GUIError(const char *msg, ...) {
	char buffer[STRINGBUFLEN];
	va_list va;

	// Generate the full error message
	va_start(va, msg);
	vsnprintf(buffer, STRINGBUFLEN, msg, va);
	va_end(va);

	GUIErrorMessage(buffer);
}

GUI::Debugger *LureEngine::getDebugger() {
	return !Game::isCreated() ? NULL : &Game::getReference().debugger();
}

void LureEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	Sound.syncSounds();
}

Common::String *LureEngine::detectSave(int slotNumber) {
	Common::ReadStream *f = this->_saveFileMan->openForLoading(
		generateSaveName(slotNumber));
	if (f == NULL) return NULL;
	Common::String *result = NULL;

	// Check for header
	char buffer[5];
	f->read(&buffer[0], 5);
	if (memcmp(&buffer[0], "lure", 5) == 0) {
		// Check language version
		uint8 language = f->readByte();
		uint8 version = f->readByte();
		if ((language == getLureLanguage()) && (version >= LURE_MIN_SAVEGAME_MINOR)) {
			// Read in the savegame title
			char saveName[MAX_DESC_SIZE];
			char *p = saveName;
			int decCtr = MAX_DESC_SIZE - 1;
			while ((decCtr > 0) && ((*p++ = f->readByte()) != 0)) --decCtr;
			*p = '\0';
			result = new Common::String(saveName);
		}
	}

	delete f;
	return result;
}

Common::String getSaveName(Common::InSaveFile *in) {
	// Check for header
	char saveName[MAX_DESC_SIZE];
	char buffer[5];
	in->read(&buffer[0], 5);
	if (memcmp(&buffer[0], "lure", 5) == 0) {
		// Check language version
		in->readByte();
		in->readByte();
		char *p = saveName;
		int decCtr = MAX_DESC_SIZE - 1;
		while ((decCtr > 0) && ((*p++ = in->readByte()) != 0)) --decCtr;
		*p = '\0';

	}

	return Common::String(saveName);
}

} // End of namespace Lure
