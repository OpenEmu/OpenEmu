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

#include "made/made.h"
#include "made/console.h"
#include "made/pmvplayer.h"
#include "made/resource.h"
#include "made/screen.h"
#include "made/database.h"
#include "made/script.h"
#include "made/music.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/error.h"

#include "engines/util.h"

#include "backends/audiocd/audiocd.h"

namespace Made {

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

static const GameSettings madeSettings[] = {
	{"made", "Made game", 0, 0, 0},

	{NULL, NULL, 0, 0, NULL}
};

MadeEngine::MadeEngine(OSystem *syst, const MadeGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = madeSettings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;

	_rnd = new Common::RandomSource("made");

	_console = new MadeConsole(this);

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->getAudioCDManager()->openCD(cd_num);

	_pmvPlayer = new PmvPlayer(this, _mixer);
	_res = new ResourceReader();
	_screen = new Screen(this);

	if (getGameID() == GID_LGOP2 || getGameID() == GID_MANHOLE || getGameID() == GID_RODNEY) {
		_dat = new GameDatabaseV2(this);
	} else if (getGameID() == GID_RTZ) {
		_dat = new GameDatabaseV3(this);
	} else {
		error("Unknown GameID");
	}

	_script = new ScriptInterpreter(this);

	_music = new MusicPlayer();

	// Set default sound frequency
	switch (getGameID()) {
	case GID_RODNEY:
		_soundRate = 11025;
		break;
	case GID_MANHOLE:
		_soundRate = 11025;
		break;
	case GID_LGOP2:
		_soundRate = 8000;
		break;
	case GID_RTZ:
		// Return to Zork sets it itself via a script funtion
		break;
	}

	syncSoundSettings();
}

MadeEngine::~MadeEngine() {
	_system->getAudioCDManager()->stop();

	delete _rnd;
	delete _console;
	delete _pmvPlayer;
	delete _res;
	delete _screen;
	delete _dat;
	delete _script;
	delete _music;
}

void MadeEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_music->setVolume(mute ? 0 : ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType,
									mute ? 0 : ConfMan.getInt("sfx_volume"));
}

int16 MadeEngine::getTicks() {
	return g_system->getMillis() * 30 / 1000;
}

GUI::Debugger *MadeEngine::getDebugger() {
	return _console;
}

int16 MadeEngine::getTimer(int16 timerNum) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers) && _timers[timerNum - 1] != -1)
		return (getTicks() - _timers[timerNum - 1]);
	else
		return 32000;
}

void MadeEngine::setTimer(int16 timerNum, int16 value) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers))
		_timers[timerNum - 1] = value;
}

void MadeEngine::resetTimer(int16 timerNum) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers))
		_timers[timerNum - 1] = getTicks();
}

int16 MadeEngine::allocTimer() {
	for (int i = 0; i < ARRAYSIZE(_timers); i++) {
		if (_timers[i] == -1) {
			_timers[i] = getTicks();
			return i + 1;
		}
	}
	return 0;
}

void MadeEngine::freeTimer(int16 timerNum) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers))
		_timers[timerNum - 1] = -1;
}

void MadeEngine::resetAllTimers() {
	for (int i = 0; i < ARRAYSIZE(_timers); i++)
		_timers[i] = -1;
}

Common::String MadeEngine::getSavegameFilename(int16 saveNum) {
	return Common::String::format("%s.%03d", getTargetName().c_str(), saveNum);
}

void MadeEngine::handleEvents() {

	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	// NOTE: Don't reset _eventNum to 0 here or no events will get through to the scripts.

	while (eventMan->pollEvent(event)) {
		switch (event.type) {

		case Common::EVENT_MOUSEMOVE:
			_eventMouseX = event.mouse.x;
			_eventMouseY = event.mouse.y;
			break;

		case Common::EVENT_LBUTTONDOWN:
			_eventNum = 2;
			break;

		case Common::EVENT_LBUTTONUP:
			_eventNum = 1;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_eventNum = 4;
			break;

		case Common::EVENT_RBUTTONUP:
			_eventNum = 3;
			break;

		case Common::EVENT_KEYDOWN:
			// Handle any special keys here
			// Supported keys taken from http://www.allgame.com/game.php?id=13542&tab=controls

			switch (event.kbd.keycode) {
			case Common::KEYCODE_KP_PLUS:	// action (same as left mouse click)
				_eventNum = 1;		// left mouse button up
				break;
			case Common::KEYCODE_KP_MINUS:	// inventory (same as right mouse click)
				_eventNum = 3;		// right mouse button up
				break;
			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP8:
				_eventMouseY = MAX<int16>(0, _eventMouseY - 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP2:
				_eventMouseY = MIN<int16>(199, _eventMouseY + 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_KP4:
				_eventMouseX = MAX<int16>(0, _eventMouseX - 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP6:
				_eventMouseX = MIN<int16>(319, _eventMouseX + 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case Common::KEYCODE_F1:		// menu
			case Common::KEYCODE_F2:		// save game
			case Common::KEYCODE_F3:		// load game
			case Common::KEYCODE_F4:		// repeat last message
				_eventNum = 5;
				_eventKey = (event.kbd.keycode - Common::KEYCODE_F1) + 21;
				break;
			case Common::KEYCODE_BACKSPACE:
				_eventNum = 5;
				_eventKey = 9;
				break;
			default:
				_eventNum = 5;
				_eventKey = event.kbd.ascii;
				break;
			}

			// Check for Debugger Activation
			if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_d) {
				this->getDebugger()->attach();
				this->getDebugger()->onFrame();
			}
			break;

		default:
			break;

		}
	}

	_system->getAudioCDManager()->updateCD();

}

Common::Error MadeEngine::run() {

	// Initialize backend
	initGraphics(320, 200, false);

	resetAllTimers();

	if (getGameID() == GID_RTZ) {
		if (getFeatures() & GF_DEMO) {
			_dat->open("demo.dat");
			_res->open("demo.prj");
		} else if (getFeatures() & GF_CD) {
			_dat->open("rtzcd.dat");
			_res->open("rtzcd.prj");
		} else if (getFeatures() & GF_CD_COMPRESSED) {
			_dat->openFromRed("rtzcd.red", "rtzcd.dat");
			_res->open("rtzcd.prj");
		} else if (getFeatures() & GF_FLOPPY) {
			_dat->open("rtz.dat");
			_res->open("rtz.prj");
		} else {
			error("Unknown RTZ game features");
		}
	} else if (getGameID() == GID_MANHOLE) {
		_dat->open("manhole.dat");

		if (getVersion() == 2) {
			_res->open("manhole.prj");
		} else {
			_res->openResourceBlocks();
		}
	} else if (getGameID() == GID_LGOP2) {
		_dat->open("lgop2.dat");
		_res->open("lgop2.prj");
	} else if (getGameID() == GID_RODNEY) {
		_dat->open("rodneys.dat");
		_res->open("rodneys.prj");
	} else {
		error ("Unknown MADE game");
	}

	if ((getFeatures() & GF_CD) || (getFeatures() & GF_CD_COMPRESSED))
		checkCD();

	_autoStopSound = false;
	_eventNum = _eventKey = _eventMouseX = _eventMouseY = 0;

#ifdef DUMP_SCRIPTS
	_script->dumpAllScripts();
#else
	_screen->setDefaultMouseCursor();
	_script->runScript(_dat->getMainCodeObjectIndex());
#endif

	return Common::kNoError;
}

} // End of namespace Made
