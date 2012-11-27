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
#include "common/random.h"
#include "common/error.h"
#include "common/events.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

#include "hugo/hugo.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/mouse.h"
#include "hugo/inventory.h"
#include "hugo/parser.h"
#include "hugo/route.h"
#include "hugo/util.h"
#include "hugo/sound.h"
#include "hugo/intro.h"
#include "hugo/object.h"
#include "hugo/text.h"

#include "engines/util.h"

namespace Hugo {

HugoEngine *HugoEngine::s_Engine = 0;

HugoEngine::HugoEngine(OSystem *syst, const HugoGameDescription *gd) : Engine(syst), _gameDescription(gd),
	_hero(0), _heroImage(0), _defltTunes(0), _numScreens(0), _tunesNbr(0), _soundSilence(0), _soundTest(0),
	_screenStates(0), _numStates(0), _score(0), _maxscore(0), _lastTime(0), _curTime(0), _episode(0)
{
	_system = syst;
	DebugMan.addDebugChannel(kDebugSchedule, "Schedule", "Script Schedule debug level");
	DebugMan.addDebugChannel(kDebugEngine, "Engine", "Engine debug level");
	DebugMan.addDebugChannel(kDebugDisplay, "Display", "Display debug level");
	DebugMan.addDebugChannel(kDebugMouse, "Mouse", "Mouse debug level");
	DebugMan.addDebugChannel(kDebugParser, "Parser", "Parser debug level");
	DebugMan.addDebugChannel(kDebugFile, "File", "File IO debug level");
	DebugMan.addDebugChannel(kDebugRoute, "Route", "Route debug level");
	DebugMan.addDebugChannel(kDebugInventory, "Inventory", "Inventory debug level");
	DebugMan.addDebugChannel(kDebugObject, "Object", "Object debug level");
	DebugMan.addDebugChannel(kDebugMusic, "Music", "Music debug level");

	_console = new HugoConsole(this);
	_rnd = 0;
}

HugoEngine::~HugoEngine() {
	_file->closeDatabaseFiles();

	_intro->freeIntroData();
	_inventory->freeInvent();
	_mouse->freeHotspots();
	_object->freeObjects();
	_parser->freeParser();
	_scheduler->freeScheduler();
	_screen->freeScreen();
	_text->freeAllTexts();

	free(_defltTunes);
	free(_screenStates);


	delete _topMenu;
	delete _object;
	delete _sound;
	delete _route;
	delete _parser;
	delete _inventory;
	delete _mouse;
	delete _screen;
	delete _intro;
	delete _scheduler;
	delete _file;
	delete _text;

	DebugMan.clearAllDebugChannels();
	delete _console;
	delete _rnd;
}

GUI::Debugger *HugoEngine::getDebugger() {
	return _console;
}

Status &HugoEngine::getGameStatus() {
	return _status;
}

int HugoEngine::getScore() const {
	return _score;
}

void HugoEngine::setScore(const int newScore) {
	_score = newScore;
}

void HugoEngine::adjustScore(const int adjustment) {
	_score += adjustment;
}

int HugoEngine::getMaxScore() const {
	return _maxscore;
}

void HugoEngine::setMaxScore(const int newScore) {
	_maxscore = newScore;
}

Common::Error HugoEngine::saveGameState(int slot, const Common::String &desc) {
	return (_file->saveGame(slot, desc) ? Common::kWritingFailed : Common::kNoError);
}

Common::Error HugoEngine::loadGameState(int slot) {
	return (_file->restoreGame(slot) ? Common::kReadingFailed : Common::kNoError);
}

bool HugoEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsRTL) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *HugoEngine::getCopyrightString() const {
	return "Copyright 1989-1997 David P Gray, All Rights Reserved.";
}

GameType HugoEngine::getGameType() const {
	return _gameType;
}

Common::Platform HugoEngine::getPlatform() const {
	return _platform;
}

bool HugoEngine::isPacked() const {
	return _packedFl;
}

/**
 * Print options for user when dead
 */
void HugoEngine::gameOverMsg() {
	Utils::notifyBox(_text->getTextUtil(kGameOver));
}

Common::Error HugoEngine::run() {
	s_Engine = this;
	initGraphics(320, 200, false);

	_mouse = new MouseHandler(this);
	_inventory = new InventoryHandler(this);
	_route = new Route(this);
	_sound = new SoundHandler(this);

	// Setup mixer
	syncSoundSettings();

	_text = new TextHandler(this);

	_topMenu = new TopMenu(this);

	switch (_gameVariant) {
	case kGameVariantH1Win: // H1 Win
		_file = new FileManager_v1w(this);
		_scheduler = new Scheduler_v1w(this);
		_intro = new intro_v1w(this);
		_screen = new Screen_v1w(this);
		_parser = new Parser_v1w(this);
		_object = new ObjectHandler_v1w(this);
		_normalTPS = 9;
		break;
	case kGameVariantH2Win:
		_file = new FileManager_v2w(this);
		_scheduler = new Scheduler_v1w(this);
		_intro = new intro_v2w(this);
		_screen = new Screen_v1w(this);
		_parser = new Parser_v1w(this);
		_object = new ObjectHandler_v1w(this);
		_normalTPS = 9;
		break;
	case kGameVariantH3Win:
		_file = new FileManager_v2w(this);
		_scheduler = new Scheduler_v1w(this);
		_intro = new intro_v3w(this);
		_screen = new Screen_v1w(this);
		_parser = new Parser_v1w(this);
		_object = new ObjectHandler_v1w(this);
		_normalTPS = 9;
		break;
	case kGameVariantH1Dos: // H1 DOS
		_file = new FileManager_v1d(this);
		_scheduler = new Scheduler_v1d(this);
		_intro = new intro_v1d(this);
		_screen = new Screen_v1d(this);
		_parser = new Parser_v1d(this);
		_object = new ObjectHandler_v1d(this);
		_normalTPS = 8;
		break;
	case kGameVariantH2Dos:
		_file = new FileManager_v2d(this);
		_scheduler = new Scheduler_v2d(this);
		_intro = new intro_v2d(this);
		_screen = new Screen_v1d(this);
		_parser = new Parser_v2d(this);
		_object = new ObjectHandler_v2d(this);
		_normalTPS = 8;
		break;
	case kGameVariantH3Dos:
		_file = new FileManager_v3d(this);
		_scheduler = new Scheduler_v3d(this);
		_intro = new intro_v3d(this);
		_screen = new Screen_v1d(this);
		_parser = new Parser_v3d(this);
		_object = new ObjectHandler_v3d(this);
		_normalTPS = 9;
		break;
	}

	if (!loadHugoDat())
		return Common::kUnknownError;

	// Use Windows-looking mouse cursor
	_screen->setCursorPal();
	_screen->resetInventoryObjId();

	_scheduler->initCypher();

	initStatus();                                   // Initialize game status
	initConfig();                                   // Initialize user's config
	if (!_status._doQuitFl) {
		initialize();
		resetConfig();                              // Reset user's config
		initMachine();

		// Start the state machine
		_status._viewState = kViewIntroInit;

		int16 loadSlot = Common::ConfigManager::instance().getInt("save_slot");
		if (loadSlot >= 0) {
			_status._skipIntroFl = true;
			_file->restoreGame(loadSlot);
		} else {
			_file->saveGame(0, "New Game");
		}
	}

	while (!_status._doQuitFl) {
		_screen->drawBoundaries();
		g_system->updateScreen();
		runMachine();

		// Handle input
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				_parser->keyHandler(event);
				break;
			case Common::EVENT_MOUSEMOVE:
				_mouse->setMouseX(event.mouse.x);
				_mouse->setMouseY(event.mouse.y);
				break;
			case Common::EVENT_LBUTTONUP:
				_mouse->setLeftButton();
				break;
			case Common::EVENT_RBUTTONUP:
				_mouse->setRightButton();
				break;
			case Common::EVENT_QUIT:
				_status._doQuitFl = true;
				break;
			default:
				break;
			}
		}
		if (_status._helpFl) {
			_status._helpFl = false;
			_file->instructions();
		}

		_mouse->mouseHandler();                     // Mouse activity - adds to display list
		_screen->displayList(kDisplayDisplay);      // Blit the display list to screen
		_status._doQuitFl |= shouldQuit();           // update game quit flag
	}
	return Common::kNoError;
}

void HugoEngine::initMachine() {
	if (_gameVariant == kGameVariantH1Dos)
		readScreenFiles(0);
	else
		_file->readBackground(_numScreens - 1);     // Splash screen
	_object->readObjectImages();                    // Read all object images
	if (_platform == Common::kPlatformWindows)
		_file->readUIFImages();                     // Read all uif images (only in Win versions)

	_sound->initPcspkrPlayer();
}

/**
 * Hugo game state machine - called during onIdle
 */
void HugoEngine::runMachine() {
	Status &gameStatus = getGameStatus();

	// Don't process if gameover
	if (gameStatus._gameOverFl)
		return;

	_curTime = g_system->getMillis();
	// Process machine once every tick
	while (_curTime - _lastTime < (uint32)(1000 / getTPS())) {
		g_system->delayMillis(5);
		_curTime = g_system->getMillis();
	}

	_lastTime = _curTime;

	switch (gameStatus._viewState) {
	case kViewIdle:                                 // Not processing state machine
		_screen->hideCursor();
		_intro->preNewGame();                       // Any processing before New Game selected
		break;
	case kViewIntroInit:                            // Initialization before intro begins
		_intro->introInit();
		gameStatus._viewState = kViewIntro;
		break;
	case kViewIntro:                                // Do any game-dependant preamble
		if (_intro->introPlay()) {                  // Process intro screen
			_scheduler->newScreen(0);               // Initialize first screen
			gameStatus._viewState = kViewPlay;
		}
		break;
	case kViewPlay:                                 // Playing game
		_screen->showCursor();
		_parser->charHandler();                     // Process user cmd input
		_object->moveObjects();                     // Process object movement
		_scheduler->runScheduler();                 // Process any actions
		_screen->displayList(kDisplayRestore);      // Restore previous background
		_object->updateImages();                    // Draw into _frontBuffer, compile display list
		_screen->drawStatusText();
		_screen->displayList(kDisplayDisplay);      // Blit the display list to screen
		_sound->checkMusic();
		break;
	case kViewInvent:                               // Accessing inventory
		_inventory->runInventory();                 // Process Inventory state machine
		break;
	case kViewExit:                                 // Game over or user exited
		gameStatus._viewState = kViewIdle;
		_status._doQuitFl = true;
		break;
	}
}

/**
 * Loads Hugo.dat file, which contains all the hardcoded data in the original executables
 */
bool HugoEngine::loadHugoDat() {
	Common::File in;
	in.open("hugo.dat");

	if (!in.isOpen()) {
		Common::String errorMessage = "You're missing the 'hugo.dat' file. Get it from the ScummVM website";
		GUIErrorMessage(errorMessage);
		warning("%s", errorMessage.c_str());
		return false;
	}

	// Read header
	char buf[4];
	in.read(buf, 4);

	if (memcmp(buf, "HUGO", 4)) {
		Common::String errorMessage = "File 'hugo.dat' is corrupt. Get it from the ScummVM website";
		GUIErrorMessage(errorMessage);
		return false;
	}

	int majVer = in.readByte();
	int minVer = in.readByte();

	if ((majVer != HUGO_DAT_VER_MAJ) || (minVer != HUGO_DAT_VER_MIN)) {
		Common::String errorMessage = Common::String::format("File 'hugo.dat' is wrong version. Expected %d.%d but got %d.%d. Get it from the ScummVM website", HUGO_DAT_VER_MAJ, HUGO_DAT_VER_MIN, majVer, minVer);
		GUIErrorMessage(errorMessage);
		return false;
	}

	_numVariant = in.readUint16BE();

	_screen->loadPalette(in);
	_screen->loadFontArr(in);
	_text->loadAllTexts(in);
	_intro->loadIntroData(in);
	_parser->loadArrayReqs(in);
	_parser->loadCatchallList(in);
	_parser->loadBackgroundObjects(in);
	_parser->loadCmdList(in);
	_mouse->loadHotspots(in);
	_inventory->loadInvent(in);
	_object->loadObjectUses(in);
	_object->loadObjectArr(in);
	_object->loadNumObj(in);
	_scheduler->loadPoints(in);
	_scheduler->loadScreenAct(in);
	_scheduler->loadActListArr(in);
	_scheduler->loadAlNewscrIndex(in);
	_hero = &_object->_objects[kHeroIndex];         // This always points to hero
	_screenPtr = &(_object->_objects[kHeroIndex]._screenIndex); // Current screen is hero's
	_heroImage = kHeroIndex;                        // Current in use hero image

	for (int varnt = 0; varnt < _numVariant; varnt++) {
		if (varnt == _gameVariant) {
			_tunesNbr     = in.readSByte();
			_soundSilence = in.readSByte();
			_soundTest    = in.readSByte();
		} else {
			in.readSByte();
			in.readSByte();
			in.readSByte();
		}
	}

	int numElem;

	//Read _defltTunes
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_defltTunes = (int16 *)malloc(sizeof(int16) * numElem);
			for (int i = 0; i < numElem; i++)
				_defltTunes[i] = in.readSint16BE();
		} else {
			for (int i = 0; i < numElem; i++)
				in.readSint16BE();
		}
	}

	//Read _screenStates size
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_numStates = numElem;
			_screenStates = (byte *)malloc(sizeof(byte) * numElem);
			memset(_screenStates, 0, sizeof(byte) * numElem);
		}
	}

	//Read look, take and drop special verbs indexes
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		if (varnt == _gameVariant) {
			_look = in.readUint16BE();
			_take = in.readUint16BE();
			_drop = in.readUint16BE();
		} else {
			in.readUint16BE();
			in.readUint16BE();
			in.readUint16BE();
		}
	}

	_sound->loadIntroSong(in);
	_topMenu->loadBmpArr(in);

	return true;
}

uint16 **HugoEngine::loadLongArray(Common::SeekableReadStream &in) {
	uint16 **resArray = 0;

	for (int varnt = 0; varnt < _numVariant; varnt++) {
		uint16 numRows = in.readUint16BE();
		if (varnt == _gameVariant) {
			resArray = (uint16 **)malloc(sizeof(uint16 *) * (numRows + 1));
			resArray[numRows] = 0;
		}
		for (int i = 0; i < numRows; i++) {
			uint16 numElems = in.readUint16BE();
			if (varnt == _gameVariant) {
				uint16 *resRow = (uint16 *)malloc(sizeof(uint16) * numElems);
				for (int j = 0; j < numElems; j++)
					resRow[j] = in.readUint16BE();
				resArray[i] = resRow;
			} else {
				in.skip(numElems * sizeof(uint16));
			}
		}
	}
	return resArray;
}

/**
 * Sets the playlist to be the default tune selection
 */
void HugoEngine::initPlaylist(bool playlist[kMaxTunes]) {
	debugC(1, kDebugEngine, "initPlaylist");

	for (int16 i = 0; i < kMaxTunes; i++)
		playlist[i] = false;
	for (int16 i = 0; _defltTunes[i] != -1; i++)
		playlist[_defltTunes[i]] = true;
}

/**
 * Initialize the dynamic game status
 */
void HugoEngine::initStatus() {
	debugC(1, kDebugEngine, "initStatus");
	_status._storyModeFl      = false;               // Not in story mode
	_status._gameOverFl       = false;               // Hero not knobbled yet
	_status._lookFl           = false;               // Toolbar "look" button
	_status._recallFl         = false;               // Toolbar "recall" button
	_status._newScreenFl      = false;               // Screen not just loaded
	_status._godModeFl        = false;               // No special cheats allowed
	_status._showBoundariesFl = false;               // Boundaries hidden by default
	_status._doQuitFl         = false;
	_status._skipIntroFl      = false;
	_status._helpFl           = false;

	// Initialize every start of new game
	_status._tick            = 0;                    // Tick count
	_status._viewState       = kViewIdle;            // View state
}

/**
 * Initialize default config values.  Must be done before Initialize().
 */
void HugoEngine::initConfig() {
	debugC(1, kDebugEngine, "initConfig()");

	_config._musicFl = true;                            // Music state initially on
	_config._soundFl = true;                            // Sound state initially on
	_config._turboFl = false;                           // Turbo state initially off
	initPlaylist(_config._playlist);                    // Initialize default tune playlist
	_file->readBootFile();                              // Read startup structure
}

/**
 * Reset config parts. Currently only reset music played based on playlist
 */
void HugoEngine::resetConfig() {
	debugC(1, kDebugEngine, "resetConfig()");

	// Find first tune and play it
	for (int16 i = 0; i < kMaxTunes; i++) {
		if (_config._playlist[i]) {
			_sound->playMusic(i);
			break;
		}
	}
}

void HugoEngine::initialize() {
	debugC(1, kDebugEngine, "initialize");

	_maze._enabledFl = false;
	_line[0] = '\0';

	_sound->initSound();
	_scheduler->initEventQueue();                   // Init scheduler stuff
	_screen->initDisplay();                         // Create Dibs and palette
	_file->openDatabaseFiles();                     // Open database files
	calcMaxScore();                                 // Initialize maxscore

	_rnd = new Common::RandomSource("hugo");
	_rnd->setSeed(42);                              // Kick random number generator

	switch (_gameVariant) {
	case kGameVariantH1Dos:
		_episode = "\"Hugo's House of Horrors\"";
		_picDir = "";
		break;
	case kGameVariantH2Dos:
		_episode = "\"Hugo II: Whodunit?\"";
		_picDir = "";
		break;
	case kGameVariantH3Dos:
		_episode = "\"Hugo III: Jungle of Doom\"";
		_picDir = "pictures/";
		break;
	case kGameVariantH1Win:
		_episode = "\"Hugo's Horrific Adventure\"";
		_picDir = "hugo1/";
		break;
	case kGameVariantH2Win:
		_episode = "\"Hugo's Mystery Adventure\"";
		_picDir = "hugo2/";
		break;
	case kGameVariantH3Win:
		_episode = "\"Hugo's Amazon Adventure\"";
		_picDir = "hugo3/";
		break;
	default:
		error("Unknown game");
	}
}

/**
 * Read scenery, overlay files for given screen number
 */
void HugoEngine::readScreenFiles(const int screenNum) {
	debugC(1, kDebugEngine, "readScreenFiles(%d)", screenNum);

	_file->readBackground(screenNum);               // Scenery file
	memcpy(_screen->getBackBuffer(), _screen->getFrontBuffer(), sizeof(_screen->getFrontBuffer())); // Make a copy

	// Workaround for graphic glitches in DOS versions. Cleaning the overlays fix the problem
	memset(_object->_objBound, '\0', sizeof(Overlay));
	memset(_object->_boundary, '\0', sizeof(Overlay));
	memset(_object->_overlay,  '\0', sizeof(Overlay));
	memset(_object->_ovlBase,  '\0', sizeof(Overlay));

	_file->readOverlay(screenNum, _object->_boundary, kOvlBoundary); // Boundary file
	_file->readOverlay(screenNum, _object->_overlay, kOvlOverlay);   // Overlay file
	_file->readOverlay(screenNum, _object->_ovlBase, kOvlBase);      // Overlay base file

	// Suppress a boundary used in H3 DOS in 'Crash' screen, which blocks
	// pathfinding and is useless.
	if ((screenNum == 0) && (_gameVariant == kGameVariantH3Dos))
		_object->clearScreenBoundary(50, 311, 152);
}

/**
 * Set the new screen number into the hero object and any carried objects
 */
void HugoEngine::setNewScreen(const int screenNum) {
	debugC(1, kDebugEngine, "setNewScreen(%d)", screenNum);

	*_screenPtr = screenNum;                        // HERO object
	_object->setCarriedScreen(screenNum);           // Carried objects
}

/**
 * Add up all the object values and all the bonus points
 */
void HugoEngine::calcMaxScore() {
	debugC(1, kDebugEngine, "calcMaxScore");

	_maxscore = _object->calcMaxScore() + _scheduler->calcMaxPoints();
}

/**
 * Exit game, advertise trilogy, show copyright
 */
void HugoEngine::endGame() {
	debugC(1, kDebugEngine, "endGame");

	if (_boot._registered != kRegRegistered)
		Utils::notifyBox(_text->getTextEngine(kEsAdvertise));
	Utils::notifyBox(Common::String::format("%s\n%s", _episode, getCopyrightString()));
	_status._viewState = kViewExit;
}

bool HugoEngine::canLoadGameStateCurrently() {
	return true;
}

bool HugoEngine::canSaveGameStateCurrently() {
	return (_status._viewState == kViewPlay);
}

int8 HugoEngine::getTPS() const {
	return ((_config._turboFl) ? kTurboTps : _normalTPS);
}

void HugoEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_sound->syncVolume();
}

Common::String HugoEngine::getSavegameFilename(int slot) {
	return _targetName + Common::String::format("-%02d.SAV", slot);
}



} // End of namespace Hugo
