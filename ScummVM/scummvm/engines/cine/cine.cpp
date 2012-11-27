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

#include "engines/util.h"

#include "graphics/cursorman.h"

#include "cine/cine.h"
#include "cine/bg_list.h"
#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/texte.h"
#include "cine/sound.h"
#include "cine/various.h"

namespace Cine {

Sound *g_sound = 0;

CineEngine *g_cine = 0;

CineEngine::CineEngine(OSystem *syst, const CINEGameDescription *gameDesc)
	: Engine(syst),
	_gameDescription(gameDesc),
	_rnd("cine") {
	// Setup mixer
	syncSoundSettings();

	DebugMan.addDebugChannel(kCineDebugScript,    "Script",    "Script debug level");
	DebugMan.addDebugChannel(kCineDebugPart,      "Part",      "Part debug level");
	DebugMan.addDebugChannel(kCineDebugSound,     "Sound",     "Sound debug level");
	DebugMan.addDebugChannel(kCineDebugCollision, "Collision", "Collision debug level");
	_console = new CineConsole(this);

	g_cine = this;
}

CineEngine::~CineEngine() {
	if (getGameType() == Cine::GType_OS) {
		freeErrmessDat();
	}

	DebugMan.clearAllDebugChannels();
	delete _console;
}

void CineEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	// Use music volume for plain sound types (At least the AdLib player uses a plain sound type
	// so previously the music and sfx volume controls didn't affect it at all).
	// FIXME: Make AdLib player differentiate between playing sound effects and music and remove this.
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType,
									mute ? 0 : ConfMan.getInt("music_volume"));
}

Common::Error CineEngine::run() {
	// Initialize backend
	initGraphics(320, 200, false);

	if (getPlatform() == Common::kPlatformPC) {
		g_sound = new PCSound(_mixer, this);
	} else {
		// Paula chipset for Amiga and Atari versions
		g_sound = new PaulaSound(_mixer, this);
	}

	_restartRequested = false;

	do {
		initialize();

		_restartRequested = false;

		CursorMan.showMouse(true);
		mainLoop(1);

		delete renderer;
		delete[] collisionPage;
	} while (_restartRequested);

	delete g_sound;

	return Common::kNoError;
}

int CineEngine::getTimerDelay() const {
	return (10923000 * _timerDelayMultiplier) / 1193180;
}

/**
 * Modify game speed
 * @param speedChange Negative values slow game down, positive values speed it up, zero does nothing
 * @return Timer delay multiplier's value after the game speed change
 */
int CineEngine::modifyGameSpeed(int speedChange) {
	// If we want more speed we decrement the timer delay multiplier and vice versa.
	_timerDelayMultiplier = CLIP(_timerDelayMultiplier - speedChange, 1, 50);
	return _timerDelayMultiplier;
}

void CineEngine::initialize() {
	_globalVars.reinit(NUM_MAX_VAR + 1);

	// Initialize all savegames' descriptions to empty strings
	memset(currentSaveName, 0, sizeof(currentSaveName));

	// Resize object table to its correct size and reset all its elements
	g_cine->_objectTable.resize(NUM_MAX_OBJECT);
	resetObjectTable();

	// Resize animation data table to its correct size and reset all its elements
	g_cine->_animDataTable.resize(NUM_MAX_ANIMDATA);
	freeAnimDataTable();

	// Resize zone data table to its correct size and reset all its elements
	g_cine->_zoneData.resize(NUM_MAX_ZONE);
	Common::fill(g_cine->_zoneData.begin(), g_cine->_zoneData.end(), 0);

	// Resize zone query table to its correct size and reset all its elements
	g_cine->_zoneQuery.resize(NUM_MAX_ZONE);
	Common::fill(g_cine->_zoneQuery.begin(), g_cine->_zoneQuery.end(), 0);

	_timerDelayMultiplier = 12; // Set default speed
	setupOpcodes();

	initLanguage(getLanguage());

	if (getGameType() == Cine::GType_OS) {
		renderer = new OSRenderer;
	} else {
		renderer = new FWRenderer;
	}

	renderer->initialize();

	collisionPage = new byte[320 * 200];
	memset(collisionPage, 0, 320 * 200);

	// Clear part buffer as there's nothing loaded into it yet.
	// Its size will change when loading data into it with the loadPart function.
	g_cine->_partBuffer.clear();

	if (getGameType() == Cine::GType_OS) {
		readVolCnf();
	}

	loadTextData("texte.dat");

	if (getGameType() == Cine::GType_OS && !(getFeatures() & GF_DEMO)) {
		loadPoldatDat("poldat.dat");
		loadErrmessDat("errmess.dat");
	}

	// in case ScummVM engines can be restarted in the future
	g_cine->_scriptTable.clear();
	g_cine->_relTable.clear();
	g_cine->_objectScripts.clear();
	g_cine->_globalScripts.clear();
	g_cine->_bgIncrustList.clear();
	freeAnimDataTable();
	g_cine->_overlayList.clear();
	g_cine->_messageTable.clear();
	resetObjectTable();

	if (getGameType() == Cine::GType_OS) {
		disableSystemMenu = 1;
	} else {
		// WORKAROUND: We do not save this variable in FW's savegames.
		// Initializing this to 1, like we do it in the OS case, will
		// cause the menu disabled when loading from the launcher or
		// command line.
		// A proper fix here would be to save this variable in FW's saves.
		// Since it seems these are unversioned so far, there would be need
		// to properly add versioning to them first.
		disableSystemMenu = 0;
	}

	var8 = 0;

	var2 = var3 = var4 = var5 = 0;

	musicIsPlaying = 0;
	currentDatName[0] = 0;

	_preLoad = false;
	if (ConfMan.hasKey("save_slot") && !_restartRequested) {
		char saveNameBuffer[256];

		sprintf(saveNameBuffer, "%s.%1d", _targetName.c_str(), ConfMan.getInt("save_slot"));

		bool res = makeLoad(saveNameBuffer);

		if (res)
			_preLoad = true;
	}

	if (!_preLoad) {
		loadPrc(BOOT_PRC_NAME);
		strcpy(currentPrcName, BOOT_PRC_NAME);
		setMouseCursor(MOUSE_CURSOR_NORMAL);
	}
}

} // End of namespace Cine
