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

#include "common/md5.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/random.h"
#include "common/textconsole.h"

#include "engines/util.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/cursorman.h"

#include "audio/mididrv.h"
#include "audio/mixer.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/sprite.h"
#include "agi/keyboard.h"
#include "agi/menu.h"

#include "gui/predictivedialog.h"

namespace Agi {

void AgiEngine::allowSynthetic(bool allow) {
	_allowSynthetic = allow;
}

void AgiEngine::processEvents() {
	Common::Event event;
	int key = 0;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_PREDICTIVE_DIALOG: {
			GUI::PredictiveDialog _predictiveDialog;
			_predictiveDialog.runModal();
			strcpy(_predictiveResult, _predictiveDialog.getResult());
			if (strcmp(_predictiveResult, "")) {
				if (_game.inputMode == INPUT_NORMAL) {
					strcpy((char *)_game.inputBuffer, _predictiveResult);
					handleKeys(KEY_ENTER);
				} else if (_game.inputMode == INPUT_GETSTRING) {
					strcpy(_game.strings[_stringdata.str], _predictiveResult);
					newInputMode(INPUT_NORMAL);
					_gfx->printCharacter(_stringdata.x + strlen(_game.strings[_stringdata.str]) + 1,
							_stringdata.y, ' ', _game.colorFg, _game.colorBg);
				} else if (_game.inputMode == INPUT_NONE) {
					for (int n = 0; _predictiveResult[n]; n++)
						keyEnqueue(_predictiveResult[n]);
				}
			}
			/*
			if (predictiveDialog()) {
				if (_game.inputMode == INPUT_NORMAL) {
					strcpy((char *)_game.inputBuffer, _predictiveResult);
					handleKeys(KEY_ENTER);
				} else if (_game.inputMode == INPUT_GETSTRING) {
					strcpy(_game.strings[_stringdata.str], _predictiveResult);
					newInputMode(INPUT_NORMAL);
					_gfx->printCharacter(_stringdata.x + strlen(_game.strings[_stringdata.str]) + 1,
							_stringdata.y, ' ', _game.colorFg, _game.colorBg);
				} else if (_game.inputMode == INPUT_NONE) {
					for (int n = 0; _predictiveResult[n]; n++)
						keyEnqueue(_predictiveResult[n]);
				}
			}
			*/
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			key = BUTTON_LEFT;
			_mouse.button = kAgiMouseButtonLeft;
			keyEnqueue(key);
			_mouse.x = event.mouse.x;
			_mouse.y = event.mouse.y;
			break;
		case Common::EVENT_RBUTTONDOWN:
			key = BUTTON_RIGHT;
			_mouse.button = kAgiMouseButtonRight;
			keyEnqueue(key);
			_mouse.x = event.mouse.x;
			_mouse.y = event.mouse.y;
			break;
		case Common::EVENT_WHEELUP:
			key = WHEEL_UP;
			keyEnqueue(key);
			break;
		case Common::EVENT_WHEELDOWN:
			key = WHEEL_DOWN;
			keyEnqueue(key);
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouse.x = event.mouse.x;
			_mouse.y = event.mouse.y;

			if (!_game.mouseFence.isEmpty()) {
				if (_mouse.x < _game.mouseFence.left)
					_mouse.x = _game.mouseFence.left;
				if (_mouse.x > _game.mouseFence.right)
					_mouse.x = _game.mouseFence.right;
				if (_mouse.y < _game.mouseFence.top)
					_mouse.y = _game.mouseFence.top;
				if (_mouse.y > _game.mouseFence.bottom)
					_mouse.y = _game.mouseFence.bottom;

				g_system->warpMouse(_mouse.x, _mouse.y);
			}

			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_mouse.button = kAgiMouseButtonUp;
			_mouse.x = event.mouse.x;
			_mouse.y = event.mouse.y;
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_d) {
				_console->attach();
				break;
			}

			switch (key = event.kbd.keycode) {
			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_KP4:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_LEFT;
				break;
			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP6:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_RIGHT;
				break;
			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP8:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_UP;
				break;
			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP2:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_DOWN;
				break;
			case Common::KEYCODE_PAGEUP:
			case Common::KEYCODE_KP9:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_UP_RIGHT;
				break;
			case Common::KEYCODE_PAGEDOWN:
			case Common::KEYCODE_KP3:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_DOWN_RIGHT;
				break;
			case Common::KEYCODE_HOME:
			case Common::KEYCODE_KP7:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_UP_LEFT;
				break;
			case Common::KEYCODE_END:
			case Common::KEYCODE_KP1:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_DOWN_LEFT;
				break;
			case Common::KEYCODE_KP5:
				key = KEY_STATIONARY;
				break;
			case Common::KEYCODE_PLUS:
				key = '+';
				break;
			case Common::KEYCODE_MINUS:
				key = '-';
				break;
			case Common::KEYCODE_TAB:
				key = 0x0009;
				break;
			case Common::KEYCODE_F1:
				key = 0x3b00;
				break;
			case Common::KEYCODE_F2:
				key = 0x3c00;
				break;
			case Common::KEYCODE_F3:
				key = 0x3d00;
				break;
			case Common::KEYCODE_F4:
				key = 0x3e00;
				break;
			case Common::KEYCODE_F5:
				key = 0x3f00;
				break;
			case Common::KEYCODE_F6:
				key = 0x4000;
				break;
			case Common::KEYCODE_F7:
				key = 0x4100;
				break;
			case Common::KEYCODE_F8:
				key = 0x4200;
				break;
			case Common::KEYCODE_F9:
				key = 0x4300;
				break;
			case Common::KEYCODE_F10:
				key = 0x4400;
				break;
			case Common::KEYCODE_F11:
				key = KEY_STATUSLN;
				break;
			case Common::KEYCODE_F12:
				key = KEY_PRIORITY;
				break;
			case Common::KEYCODE_ESCAPE:
				key = 0x1b;
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				key = KEY_ENTER;
				break;
			case Common::KEYCODE_BACKSPACE:
				key = KEY_BACKSPACE;
				break;
			default:
				// Not a special key, so get the ASCII code for it
				key = event.kbd.ascii;

				if (Common::isAlpha(key)) {
					// Key is A-Z.
					// Map Ctrl-A to 1, Ctrl-B to 2, etc.
					if (event.kbd.flags & Common::KBD_CTRL) {
						key = toupper(key) - 'A' + 1;
					} else if (event.kbd.flags & Common::KBD_ALT) {
						// Map Alt-A, Alt-B etc. to special scancode values according to an internal scancode table.
						key = scancodeTable[toupper(key) - 'A'] << 8;
					}
				}
				break;
			}
			if (key)
				keyEnqueue(key);
			break;

		case Common::EVENT_KEYUP:
			if (_egoHoldKey)
				_game.viewTable[0].direction = 0;

		default:
			break;
		}
	}
}

void AgiEngine::pollTimer() {
	_lastTick += 50;

	while (_system->getMillis() < _lastTick) {
		processEvents();
		_console->onFrame();
		_system->delayMillis(10);
		_system->updateScreen();
	}

	_lastTick = _system->getMillis();
}

void AgiEngine::pause(uint32 msec) {
	uint32 endTime = _system->getMillis() + msec;

	_gfx->setCursor(_renderMode == Common::kRenderAmiga, true);

	while (_system->getMillis() < endTime) {
		processEvents();
		_system->updateScreen();
		_system->delayMillis(10);
	}
	_gfx->setCursor(_renderMode == Common::kRenderAmiga);
}

void AgiEngine::initPriTable() {
	int i, p, y = 0;

	for (p = 1; p < 15; p++) {
		for (i = 0; i < 12; i++) {
			_game.priTable[y++] = p < 4 ? 4 : p;
		}
	}
}

int AgiEngine::agiInit() {
	int ec, i;

	debug(2, "initializing");
	debug(2, "game version = 0x%x", getVersion());

	// initialize with adj.ego.move.to.x.y(0, 0) so to speak
	_game.adjMouseX = _game.adjMouseY = 0;

	// reset all flags to false and all variables to 0
	for (i = 0; i < MAX_FLAGS; i++)
		_game.flags[i] = 0;
	for (i = 0; i < MAX_VARS; i++)
		_game.vars[i] = 0;

	// clear all resources and events
	for (i = 0; i < MAX_DIRS; i++) {
		memset(&_game.views[i], 0, sizeof(struct AgiView));
		memset(&_game.pictures[i], 0, sizeof(struct AgiPicture));
		memset(&_game.logics[i], 0, sizeof(struct AgiLogic));
		memset(&_game.sounds[i], 0, sizeof(class AgiSound *)); // _game.sounds contains pointers now
		memset(&_game.dirView[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirPic[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirLogic[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirSound[i], 0, sizeof(struct AgiDir));
	}

	// clear view table
	for (i = 0; i < MAX_VIEWTABLE; i++)
		memset(&_game.viewTable[i], 0, sizeof(struct VtEntry));

	initWords();

	if (!_menu)
		_menu = new Menu(this, _gfx, _picture);

	initPriTable();

	// Clear the string buffer on startup, but not when the game restarts, as
	// some scripts expect that the game strings remain unaffected after a
	// restart. An example is script 98 in SQ2, which is not invoked on restart
	// to ask Ego's name again. The name is supposed to be maintained in string 1.
	// Fixes bug #3292784.
	if (!_restartGame) {
		for (i = 0; i < MAX_STRINGS; i++)
			_game.strings[i][0] = 0;
	}

	// setup emulation

	switch (getVersion() >> 12) {
	case 2:
		debug("Emulating Sierra AGI v%x.%03x",
				(int)(getVersion() >> 12) & 0xF,
				(int)(getVersion()) & 0xFFF);
		break;
	case 3:
		debug("Emulating Sierra AGI v%x.002.%03x",
				(int)(getVersion() >> 12) & 0xF,
				(int)(getVersion()) & 0xFFF);
		break;
	}

	if (getPlatform() == Common::kPlatformAmiga)
		_game.gameFlags |= ID_AMIGA;

	if (getFeatures() & GF_AGDS)
		_game.gameFlags |= ID_AGDS;

	// Make the 256 color AGI screen the default AGI screen when AGI256 or AGI256-2 is used
	if (getFeatures() & (GF_AGI256 | GF_AGI256_2))
		_game.sbuf = _game.sbuf256c;

	if (_game.gameFlags & ID_AMIGA)
		debug(1, "Amiga padded game detected.");

	if (_game.gameFlags & ID_AGDS)
		debug(1, "AGDS mode enabled.");

	ec = _loader->init();	// load vol files, etc

	if (ec == errOK)
		ec = _loader->loadObjects(OBJECTS);

	// note: demogs has no words.tok
	if (ec == errOK)
		ec = _loader->loadWords(WORDS);

	// FIXME: load IIgs instruments and samples
	// load_instruments("kq.sys16");

	// Load logic 0 into memory
	if (ec == errOK)
		ec = _loader->loadResource(rLOGIC, 0);

#ifdef __DS__
	// Normally, the engine loads the predictive text dictionary when the predictive dialog
	// is shown.  On the DS version, the word completion feature needs the dictionary too.

	// FIXME - loadDict() no long exists in AGI as this has been moved to within the
	// GUI Predictive Dialog, but DS Word Completion is probably broken due to this...
#endif

	_egoHoldKey = false;

	_game.mouseFence.setWidth(0); // Reset

	return ec;
}

/*
 * Public functions
 */

void AgiEngine::agiUnloadResources() {
	int i;

	// Make sure logic 0 is always loaded
	for (i = 1; i < MAX_DIRS; i++) {
		_loader->unloadResource(rLOGIC, i);
	}
	for (i = 0; i < MAX_DIRS; i++) {
		_loader->unloadResource(rVIEW, i);
		_loader->unloadResource(rPICTURE, i);
		_loader->unloadResource(rSOUND, i);
	}
}

int AgiEngine::agiDeinit() {
	int ec;

	cleanInput();		// remove all words from memory
	agiUnloadResources();	// unload resources in memory
	_loader->unloadResource(rLOGIC, 0);
	ec = _loader->deinit();
	unloadObjects();
	unloadWords();

	clearImageStack();

	return ec;
}

int AgiEngine::agiLoadResource(int r, int n) {
	int i;

	i = _loader->loadResource(r, n);

	// WORKAROUND: Patches broken picture 147 in a corrupted Amiga version of Gold Rush! (v2.05 1989-03-09).
	// The picture can be seen in room 147 after dropping through the outhouse's hole in room 146.
	if (i == errOK && getGameID() == GID_GOLDRUSH && r == rPICTURE && n == 147 && _game.dirPic[n].len == 1982) {
		uint8 *pic = _game.pictures[n].rdata;
		Common::MemoryReadStream picStream(pic, _game.dirPic[n].len);
		Common::String md5str = Common::computeStreamMD5AsString(picStream, _game.dirPic[n].len);
		if (md5str == "1c685eb048656cedcee4eb6eca2cecea") {
			pic[0x042] = 0x4B; // 0x49 -> 0x4B
			pic[0x043] = 0x66; // 0x26 -> 0x66
			pic[0x204] = 0x68; // 0x28 -> 0x68
			pic[0x6C0] = 0x2D; // 0x25 -> 0x2D
			pic[0x6F0] = 0xF0; // 0x70 -> 0xF0
			pic[0x734] = 0x6F; // 0x2F -> 0x6F
		}
	}

	return i;
}

int AgiEngine::agiUnloadResource(int r, int n) {
	return _loader->unloadResource(r, n);
}

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

static const GameSettings agiSettings[] = {
	{"agi", "AGI game", GID_AGI, MDT_ADLIB, "OBJECT"},
	{NULL, NULL, 0, 0, NULL}
};

AgiBase::AgiBase(OSystem *syst, const AGIGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	// Assign default values to the config manager, in case settings are missing
	ConfMan.registerDefault("originalsaveload", "false");

	_noSaveLoadAllowed = false;

	_rnd = new Common::RandomSource("agi");
	_sound = 0;

	initFeatures();
	initVersion();
}

AgiBase::~AgiBase() {
	delete _rnd;

	if (_sound) {
		_sound->deinitSound();
		delete _sound;
	}
}

void AgiBase::initRenderMode() {
	_renderMode = Common::kRenderEGA;

	if (ConfMan.hasKey("platform")) {
		Common::Platform platform = Common::parsePlatform(ConfMan.get("platform"));
		_renderMode = (platform == Common::kPlatformAmiga) ? Common::kRenderAmiga : Common::kRenderEGA;
	}

	if (ConfMan.hasKey("render_mode")) {
		Common::RenderMode tmpMode = Common::parseRenderMode(ConfMan.get("render_mode").c_str());
		if (tmpMode != Common::kRenderDefault)
			_renderMode = tmpMode;
	}
}

AgiEngine::AgiEngine(OSystem *syst, const AGIGameDescription *gameDesc) : AgiBase(syst, gameDesc) {

	// Setup mixer
	syncSoundSettings();

	parseFeatures();

	DebugMan.addDebugChannel(kDebugLevelMain, "Main", "Generic debug level");
	DebugMan.addDebugChannel(kDebugLevelResources, "Resources", "Resources debugging");
	DebugMan.addDebugChannel(kDebugLevelSprites, "Sprites", "Sprites debugging");
	DebugMan.addDebugChannel(kDebugLevelInventory, "Inventory", "Inventory debugging");
	DebugMan.addDebugChannel(kDebugLevelInput, "Input", "Input events debugging");
	DebugMan.addDebugChannel(kDebugLevelMenu, "Menu", "Menu debugging");
	DebugMan.addDebugChannel(kDebugLevelScripts, "Scripts", "Scripts debugging");
	DebugMan.addDebugChannel(kDebugLevelSound, "Sound", "Sound debugging");
	DebugMan.addDebugChannel(kDebugLevelText, "Text", "Text output debugging");
	DebugMan.addDebugChannel(kDebugLevelSavegame, "Savegame", "Saving & restoring game debugging");


	memset(&_game, 0, sizeof(struct AgiGame));
	memset(&_debug, 0, sizeof(struct AgiDebug));
	memset(&_mouse, 0, sizeof(struct Mouse));

	_game._vm = this;

	_game.clockEnabled = false;
	_game.state = STATE_INIT;

	_keyQueueStart = 0;
	_keyQueueEnd = 0;

	_allowSynthetic = false;

	_intobj = NULL;

	_menu = NULL;
	_menuSelected = false;

	_lastSentence[0] = 0;
	memset(&_stringdata, 0, sizeof(struct StringData));

	_objects = NULL;

	_restartGame = false;

	_oldMode = INPUT_NONE;

	_firstSlot = 0;

	resetControllers();

	setupOpcodes();
	_game._curLogic = NULL;
	_timerHack = 0;
}

void AgiEngine::initialize() {
	// TODO: Some sound emulation modes do not fit our current music
	//       drivers, and I'm not sure what they are. For now, they might
	//       as well be called "PC Speaker" and "Not PC Speaker".

	// If used platform is Apple IIGS then we must use Apple IIGS sound emulation
	// because Apple IIGS AGI games use only Apple IIGS specific sound resources.
	if (getPlatform() == Common::kPlatformApple2GS) {
		_soundemu = SOUND_EMU_APPLE2GS;
	} else if (getPlatform() == Common::kPlatformCoCo3) {
		_soundemu = SOUND_EMU_COCO3;
	} else if (ConfMan.get("music_driver") == "auto") {
		// Default sound is the proper PCJr emulation
		_soundemu = SOUND_EMU_PCJR;
	} else {
		switch (MidiDriver::getMusicType(MidiDriver::detectDevice(MDT_PCSPK|MDT_AMIGA|MDT_ADLIB|MDT_PCJR|MDT_MIDI))) {
		case MT_PCSPK:
			_soundemu = SOUND_EMU_PC;
			break;
		case MT_ADLIB:
			_soundemu = SOUND_EMU_NONE;
			break;
		case MT_PCJR:
			_soundemu = SOUND_EMU_PCJR;
			break;
		case MT_AMIGA:
			_soundemu = SOUND_EMU_AMIGA;
			break;
		default:
			debug(0, "DEF");
			_soundemu = SOUND_EMU_MIDI;
			break;
		}
	}

	initRenderMode();

	_buttonStyle = AgiButtonStyle(_renderMode);
	_defaultButtonStyle = AgiButtonStyle();
	_console = new Console(this);
	_gfx = new GfxMgr(this);
	_sound = new SoundMgr(this, _mixer);
	_picture = new PictureMgr(this, _gfx);
	_sprites = new SpritesMgr(this, _gfx);

	_gfx->initMachine();

	_game.gameFlags = 0;

	_game.colorFg = 15;
	_game.colorBg = 0;

	_game.name[0] = '\0';

	_game.sbufOrig = (uint8 *)calloc(_WIDTH, _HEIGHT * 2); // Allocate space for two AGI screens vertically
	_game.sbuf16c  = _game.sbufOrig + SBUF16_OFFSET; // Make sbuf16c point to the 16 color (+control line & priority info) AGI screen
	_game.sbuf256c = _game.sbufOrig + SBUF256_OFFSET; // Make sbuf256c point to the 256 color AGI screen
	_game.sbuf     = _game.sbuf16c; // Make sbuf point to the 16 color (+control line & priority info) AGI screen by default

	_gfx->initVideo();
	_sound->initSound();

	_lastSaveTime = 0;

	_lastTick = _system->getMillis();

	debugC(2, kDebugLevelMain, "Detect game");

	if (agiDetectGame() == errOK) {
		_game.state = STATE_LOADED;
		debugC(2, kDebugLevelMain, "game loaded");
	} else {
		warning("Could not open AGI game");
	}

	debugC(2, kDebugLevelMain, "Init sound");
}

AgiEngine::~AgiEngine() {
	// If the engine hasn't been initialized yet via
	// AgiEngine::initialize(), don't attempt to free any resources, as
	// they haven't been allocated. Fixes bug #1742432 - AGI: Engine
	// crashes if no game is detected
	if (_game.state == STATE_INIT) {
		return;
	}

	agiDeinit();
	delete _loader;
	_gfx->deinitVideo();
	delete _sprites;
	delete _picture;
	free(_game.sbufOrig);
	_gfx->deinitMachine();
	delete _gfx;
	delete _console;
}

Common::Error AgiBase::init() {

	// Initialize backend
	initGraphics(320, 200, false);

	initialize();

	_gfx->gfxSetPalette();

	return Common::kNoError;
}

Common::Error AgiEngine::go() {
	CursorMan.showMouse(true);
	setTotalPlayTime(0);

	if (_game.state < STATE_LOADED) {
		do {
			mainCycle();
		} while (_game.state < STATE_RUNNING);
	}

	runGame();

	return Common::kNoError;
}

void AgiEngine::parseFeatures() {

	/* FIXME: Seems this method doesn't really do anything. It might
	   be a leftover that could be removed, except that some of its
	   intended purpose may still need to be reimplemented.

	[0:29] <Fingolfin> can you tell me what the point behind AgiEngine::parseFeatures() is?
	[0:30] <_sev> when games are created with WAGI studio
	[0:31] <_sev> it creates .wag site with game-specific features such as full game title, whether to use AGIMOUSE etc
	[0:32] <Fingolfin> ... and the "features" config key is created by our detector based on the wag file, I guess?
	[0:33] <_sev> yes
	[0:33] <Fingolfin> it's just that I cant seem to find a place we do that
	[0:33] <_sev> it is used for fallback
	[0:34] <_sev> ah, perhaps it was not updated
	[0:34] <Fingolfin> I only see us check the value, but never set it
	[0:34] <Fingolfin> maybe I am grepping wrong, who knows :)
	[0:44] <Fingolfin> _sev: so, unless I miss something, it seem that function does nothing right now
	[0:45] <_sev> Fingolfin: it could be unfinished. It was part of GSoC 3 years ago
	[0:45] <Fingolfin> well
	[0:45] <_sev> I just don't remember
	[0:45] <Fingolfin> but don't we just re-parse the wag when the game is loaded anyway?
	[0:45] <_sev> but it documents the format
	[0:45] <Fingolfin> the advanced meta engine would re-run the detector, wouldn't it?
	[0:45] <_sev> yep
	[0:47] <Fingolfin> so... shouldn't we at least add a comment to the function explaining what it does and that it's unfinished etc.? maybe add a TODO to the wiki?
	[0:47] <Fingolfin> otherwise it might stay as it is for another 3 years :)
	*/

	if (!ConfMan.hasKey("features"))
		return;

	char *features = strdup(ConfMan.get("features").c_str());
	const char *feature[100];
	int numFeatures = 0;

	char *tok = strtok(features, " ");
	if (tok) {
		do {
			feature[numFeatures++] = tok;
		} while ((tok = strtok(NULL, " ")) != NULL);
	} else {
		feature[numFeatures++] = features;
	}

	const struct Flags {
		const char *name;
		uint32 flag;
	} flags[] = {
		{ "agimouse", GF_AGIMOUSE },
		{ "agds", GF_AGDS },
		{ "agi256", GF_AGI256 },
		{ "agi256-2", GF_AGI256_2 },
		{ "agipal", GF_AGIPAL },
		{ 0, 0 }
	};

	for (int i = 0; i < numFeatures; i++) {
		for (const Flags *flag = flags; flag->name; flag++) {
			if (!scumm_stricmp(feature[i], flag->name)) {
				debug(2, "Added feature: %s", flag->name);

				setFeature(flag->flag);
				break;
			}
		}
	}

	free(features);
}

} // End of namespace Agi
