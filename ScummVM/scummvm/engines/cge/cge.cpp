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
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/fs.h"
#include "engines/advancedDetector.h"
#include "engines/util.h"
#include "gui/message.h"

#include "cge/cge.h"
#include "cge/vga13h.h"
#include "cge/cge_main.h"
#include "cge/talk.h"
#include "cge/text.h"
#include "cge/walk.h"

namespace CGE {

const int CGEEngine::_maxSceneArr[5] = {1, 8, 16, 23, 24};

CGEEngine::CGEEngine(OSystem *syst, const ADGameDescription *gameDescription)
	: Engine(syst), _gameDescription(gameDescription), _randomSource("cge") {

	// Debug/console setup
	DebugMan.addDebugChannel(kCGEDebugBitmap, "bitmap", "CGE Bitmap debug channel");
	DebugMan.addDebugChannel(kCGEDebugFile, "file", "CGE IO debug channel");
	DebugMan.addDebugChannel(kCGEDebugEngine, "engine", "CGE Engine debug channel");

	_startupMode = 1;
	_oldLev      = 0;
	_pocPtr      = 0;
	_bitmapPalette = NULL;
	_quitFlag = false;
	_showBoundariesFl = false;
}

void CGEEngine::initSceneValues() {
	for (int i = 0; i < kSceneMax; i++) {
		_heroXY[i].x = 0;
		_heroXY[i].y = 0;
	}

	for (int i = 0; i < kSceneMax + 1; i++) {
		_barriers[i]._horz = 0xFF;
		_barriers[i]._vert = 0xFF;
	}
}

void CGEEngine::init() {
	debugC(1, kCGEDebugEngine, "CGEEngine::setup()");

	// Initialise fields
	_lastFrame = 0;
	_lastTick = 0;
	_hero = NULL;
	_shadow = NULL;
	_miniScene = NULL;
	_miniShp = NULL;
	_miniShpList = NULL;
	_sprite = NULL;
	_resman = new ResourceManager();

	// Create debugger console
	_console = new CGEConsole(this);

	// Initialise engine objects
	_font = new Font(this, "CGE");
	_text = new Text(this, "CGE");
	_talk = NULL;
	_vga = new Vga(this);
	_sys = new System(this);
	_pocLight = new PocLight(this);
	for (int i = 0; i < kPocketNX; i++)
		_pocket[i] = NULL;
	_horzLine = new HorizLine(this);
	_infoLine = new InfoLine(this, kInfoW);
	_sceneLight = new SceneLight(this);
	_debugLine = new InfoLine(this, kScrWidth);
	_commandHandler = new CommandHandler(this, false);
	_commandHandlerTurbo = new CommandHandler(this, true);
	_midiPlayer = new MusicPlayer(this);
	_mouse = new Mouse(this);
	_keyboard = new Keyboard(this);
	_eventManager = new EventManager(this);
	_fx = new Fx(this, 16);   // must precede SOUND!!
	_sound = new Sound(this);

	_offUseCount = atoi(_text->getText(kOffUseCount));
	_music = true;

	for (int i = 0; i < kPocketNX; i++)
		_pocref[i] = -1;
	_volume[0] = 0;
	_volume[1] = 0;

	initSceneValues();

	_maxScene   =  0;
	_dark       = false;
	_game       = false;
	_endGame    = false;
	_now        =  1;
	_lev        = -1;
	_recentStep = -2;

	for (int i = 0; i < 4; i++)
		_flag[i] = false;

	_mode = 0;
	_soundOk = 1;
	_sprTv = NULL;
	_gameCase2Cpt = 0;

	_startGameSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
}

void CGEEngine::deinit() {
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

	delete _console;

	// Delete engine objects
	delete _vga;
	delete _sys;
	delete _sprite;
	delete _miniScene;
	delete _shadow;
	delete _horzLine;
	delete _infoLine;
	delete _sceneLight;
	delete _debugLine;
	delete _text;
	delete _pocLight;
	delete _keyboard;
	delete _mouse;
	delete _eventManager;
	delete _sound;
	delete _fx;
	delete _midiPlayer;
	delete _font;
	delete _commandHandler;
	delete _commandHandlerTurbo;
	delete _hero;
	delete _resman;

	if (_miniShpList) {
		for (int i = 0; _miniShpList[i]; ++i)
			delete _miniShpList[i];
		delete[] _miniShpList;
	}
}

CGEEngine::~CGEEngine() {
	debugC(1, kCGEDebugEngine, "CGEEngine::~CGEEngine()");
}

Common::Error CGEEngine::run() {
	debugC(1, kCGEDebugEngine, "CGEEngine::run()");

	if (_gameDescription->flags & ADGF_DEMO) {
		warning("Demos of Soltys are not supported.\nPlease get a free version on ScummVM download page");
		return Common::kUnsupportedGameidError;
	}

	// Initialize graphics using following:
	initGraphics(320, 200, false);

	// Setup necessary game objects
	init();
	// Run the game
	cge_main();

	// If game is finished, display ending message
	if (_flag[3]) {
		Common::String msg = Common::String(_text->getText(kSayTheEnd));
		if (msg.size() != 0) {
			g_system->delayMillis(10);
			GUI::MessageDialog dialog(msg, "OK");
			dialog.runModal();
		}
	}

	// Remove game objects
	deinit();

	return Common::kNoError;
}

bool CGEEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool CGEEngine::canLoadGameStateCurrently() {
	return (_startupMode == 0) && _mouse->_active;
}

bool CGEEngine::canSaveGameStateCurrently() {
	return (_startupMode == 0) && _mouse->_active &&
				_commandHandler->idle() && !_hero->_flags._hide;
}

} // End of namespace CGE
