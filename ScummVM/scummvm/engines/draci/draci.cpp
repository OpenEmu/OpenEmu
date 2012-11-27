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
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/keyboard.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"

#include "draci/draci.h"
#include "draci/animation.h"
#include "draci/barchive.h"
#include "draci/font.h"
#include "draci/game.h"
#include "draci/mouse.h"
#include "draci/music.h"
#include "draci/saveload.h"
#include "draci/screen.h"
#include "draci/script.h"
#include "draci/sound.h"
#include "draci/sprite.h"

namespace Draci {

// Data file paths

const char *objectsPath = "OBJEKTY.DFW";
const char *palettePath = "PALETY.DFW";
const char *spritesPath = "OBR_AN.DFW";
const char *overlaysPath = "OBR_MAS.DFW";
const char *roomsPath = "MIST.DFW";
const char *animationsPath = "ANIM.DFW";
const char *iconsPath = "HRA.DFW";
const char *walkingMapsPath = "MAPY.DFW";
const char *itemsPath = "IKONY.DFW";
const char *itemImagesPath = "OBR_IK.DFW";
const char *initPath = "INIT.DFW";
const char *stringsPath = "RETEZCE.DFW";
const char *soundsPath = "CD2.SAM";
const char *dubbingPath = "CD.SAM";
const char *musicPathMask = "HUDBA%d.MID";

const uint kSoundsFrequency = 13000;
const uint kDubbingFrequency = 22050;

DraciEngine::DraciEngine(OSystem *syst, const ADGameDescription *gameDesc)
 : Engine(syst), _rnd("draci") {

	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from init().

	// Do not initialize graphics here

	// However this is the place to specify all default directories
	//const Common::FSNode gameDataDir(ConfMan.get("path"));
	//SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	// Here is the right place to set up the engine specific debug levels
	DebugMan.addDebugChannel(kDraciGeneralDebugLevel, "general", "Draci general debug info");
	DebugMan.addDebugChannel(kDraciBytecodeDebugLevel, "bytecode", "GPL bytecode instructions");
	DebugMan.addDebugChannel(kDraciArchiverDebugLevel, "archiver", "BAR archiver debug info");
	DebugMan.addDebugChannel(kDraciLogicDebugLevel, "logic", "Game logic debug info");
	DebugMan.addDebugChannel(kDraciAnimationDebugLevel, "animation", "Animation debug info");
	DebugMan.addDebugChannel(kDraciSoundDebugLevel, "sound", "Sound debug info");
	DebugMan.addDebugChannel(kDraciWalkingDebugLevel, "walking", "Walking debug info");

	_console = new DraciConsole(this);
}

bool DraciEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsSubtitleOptions) ||
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

static SoundArchive* openAnyPossibleDubbing() {
	debugC(1, kDraciGeneralDebugLevel, "Trying to find original dubbing");
	LegacySoundArchive *legacy = new LegacySoundArchive(dubbingPath, kDubbingFrequency);
	if (legacy->isOpen() && legacy->size()) {
		debugC(1, kDraciGeneralDebugLevel, "Found original dubbing");
		return legacy;
	}
	delete legacy;

	// The original uncompressed dubbing cannot be found.  Try to open the
	// newer compressed version.
	debugC(1, kDraciGeneralDebugLevel, "Trying to find compressed dubbing");
	ZipSoundArchive *zip = new ZipSoundArchive();

	zip->openArchive("dub-raw.zzz", "buf", RAW80, kDubbingFrequency);
	if (zip->isOpen() && zip->size()) return zip;
#ifdef USE_FLAC
	zip->openArchive("dub-flac.zzz", "flac", FLAC);
	if (zip->isOpen() && zip->size()) return zip;
#endif
#ifdef USE_VORBIS
	zip->openArchive("dub-ogg.zzz", "ogg", OGG);
	if (zip->isOpen() && zip->size()) return zip;
#endif
#ifdef USE_MAD
	zip->openArchive("dub-mp3.zzz", "mp3", MP3);
	if (zip->isOpen() && zip->size()) return zip;
#endif

	// Return an empty (but initialized) archive anyway.
	return zip;
}

int DraciEngine::init() {
	// Initialize graphics using following:
	initGraphics(kScreenWidth, kScreenHeight, false);

	// Open game's archives
	_initArchive = new BArchive(initPath);
	_objectsArchive = new BArchive(objectsPath);
	_spritesArchive = new BArchive(spritesPath);
	_paletteArchive = new BArchive(palettePath);
	_roomsArchive = new BArchive(roomsPath);
	_overlaysArchive = new BArchive(overlaysPath);
	_animationsArchive = new BArchive(animationsPath);
	_iconsArchive = new BArchive(iconsPath);
	_walkingMapsArchive = new BArchive(walkingMapsPath);
	_itemsArchive = new BArchive(itemsPath);
	_itemImagesArchive = new BArchive(itemImagesPath);
	_stringsArchive = new BArchive(stringsPath);

	_soundsArchive = new LegacySoundArchive(soundsPath, kSoundsFrequency);
	_dubbingArchive = openAnyPossibleDubbing();
	_sound = new Sound(_mixer);

	_music = new MusicPlayer(musicPathMask);

	// Setup mixer
	syncSoundSettings();

	// Load the game's fonts
	_smallFont = new Font(kFontSmall);
	_bigFont = new Font(kFontBig);

	_screen = new Screen(this);
	_anims = new AnimationManager(this);
	_mouse = new Mouse(this);
	_script = new Script(this);
	_game = new Game(this);

	if (!_objectsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening objects archive failed");
		return Common::kUnknownError;
	}

	if (!_spritesArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening sprites archive failed");
		return Common::kUnknownError;
	}

	if (!_paletteArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening palette archive failed");
		return Common::kUnknownError;
	}

	if (!_roomsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening rooms archive failed");
		return Common::kUnknownError;
	}

	if (!_overlaysArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening overlays archive failed");
		return Common::kUnknownError;
	}

	if (!_animationsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening animations archive failed");
		return Common::kUnknownError;
	}

	if (!_iconsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening icons archive failed");
		return Common::kUnknownError;
	}

	if (!_walkingMapsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening walking maps archive failed");
		return Common::kUnknownError;
	}

	if (!_soundsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening sounds archive failed");
		return Common::kUnknownError;
	}

	if (!_dubbingArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "WARNING - Opening dubbing archive failed");
	}

	_showWalkingMap = false;

	// Basic archive test
	debugC(2, kDraciGeneralDebugLevel, "Running archive tests...");
	Common::String path("INIT.DFW");
	BArchive ar(path);
	const BAFile *f;
	debugC(3, kDraciGeneralDebugLevel, "Number of file streams in archive: %d", ar.size());

	if (ar.isOpen()) {
		f = ar.getFile(0);
	} else {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Archive not opened");
		return Common::kUnknownError;
	}

	debugC(3, kDraciGeneralDebugLevel, "First 10 bytes of file %d: ", 0);
	for (uint i = 0; i < 10; ++i) {
		debugC(3, kDraciGeneralDebugLevel, "0x%02x%c", f->_data[i], (i < 9) ? ' ' : '\n');
	}

	return Common::kNoError;
}

void DraciEngine::handleEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_RIGHT:
				if (gDebugLevel >= 0) {
					_game->scheduleEnteringRoomUsingGate(_game->nextRoomNum(), 0);
				}
				break;
			case Common::KEYCODE_LEFT:
				if (gDebugLevel >= 0) {
					_game->scheduleEnteringRoomUsingGate(_game->prevRoomNum(), 0);
				}
				break;
			case Common::KEYCODE_ESCAPE: {
				if (_game->getLoopStatus() == kStatusInventory &&
				   _game->getLoopSubstatus() == kOuterLoop) {
					_game->inventoryDone();
					break;
				}

				const int escRoom = _game->getRoomNum() != _game->getMapRoom()
					? _game->getEscRoom() : _game->getPreviousRoomNum();

				// Check if there is an escape room defined for the current room
				if (escRoom >= 0) {

					// Schedule room change
					// TODO: gate 0 (always present) is not always best for
					// returning from the map, e.g. in the starting location.
					// also, after loading the game, we shouldn't run any gate
					// program, but rather restore the state of all objects.
					_game->scheduleEnteringRoomUsingGate(escRoom, 0);

					// Immediately cancel any running animation or dubbing and
					// end any currently running GPL programs.  In the intro it
					// works as intended---skipping the rest of it.
					//
					// In the map, this causes that animation on newly
					// discovered locations will be re-run next time and
					// cut-scenes won't be played.
					_game->setExitLoop(true);
					_script->endCurrentProgram(true);
				}
				break;
			}
			case Common::KEYCODE_m:
				if (_game->getLoopStatus() == kStatusOrdinary) {
					const int new_room = _game->getRoomNum() != _game->getMapRoom()
						? _game->getMapRoom() : _game->getPreviousRoomNum();
					_game->scheduleEnteringRoomUsingGate(new_room, 0);
				}
				break;
			case Common::KEYCODE_w:
				// Show walking map toggle
				_showWalkingMap = !_showWalkingMap;
				_game->switchWalkingAnimations(_showWalkingMap);
				break;
			case Common::KEYCODE_q:
				_game->setWantQuickHero(!_game->getWantQuickHero());
				break;
			case Common::KEYCODE_i:
				if (_game->getRoomNum() == _game->getMapRoom() ||
				    _game->getLoopSubstatus() != kOuterLoop) {
					break;
				}
				if (_game->getLoopStatus() == kStatusInventory) {
					_game->inventoryDone();
				} else if (_game->getLoopStatus() == kStatusOrdinary) {
					_game->inventoryInit();
				}
				break;
			case Common::KEYCODE_F5:
				if (event.kbd.hasFlags(0)) {
					openMainMenuDialog();
				}
				break;
			case Common::KEYCODE_COMMA:
			case Common::KEYCODE_PERIOD:
			case Common::KEYCODE_SLASH:
				if ((_game->getLoopStatus() == kStatusOrdinary ||
				    _game->getLoopStatus() == kStatusInventory) &&
				   _game->getLoopSubstatus() == kOuterLoop &&
				   _game->getRoomNum() != _game->getMapRoom()) {
					_game->inventorySwitch(event.kbd.keycode);
				}
				break;
			case Common::KEYCODE_d:
				if (event.kbd.hasFlags(Common::KBD_CTRL)) {
					this->getDebugger()->attach();
					this->getDebugger()->onFrame();
				}
				break;
			default:
				break;
			}
			break;
		default:
			_mouse->handleEvent(event);
		}
	}

	// Handle EVENT_QUIT and EVENT_RTL.
	if (shouldQuit()) {
		_game->setQuit(true);
		_script->endCurrentProgram(true);
	}
}

DraciEngine::~DraciEngine() {
	// Dispose your resources here

	// If the common library supported Boost's scoped_ptr<>, then wrapping
	// all the following pointers and many more would be appropriate.  So
	// far, there is only SharedPtr, which I feel being an overkill for
	// easy deallocation.
	// TODO: We have ScopedPtr nowadays. Maybe should adapt this code then?
	delete _smallFont;
	delete _bigFont;

	delete _mouse;
	delete _script;
	delete _anims;
	delete _game;
	delete _screen;

	delete _initArchive;
	delete _paletteArchive;
	delete _objectsArchive;
	delete _spritesArchive;
	delete _roomsArchive;
	delete _overlaysArchive;
	delete _animationsArchive;
	delete _iconsArchive;
	delete _walkingMapsArchive;
	delete _itemsArchive;
	delete _itemImagesArchive;
	delete _stringsArchive;

	delete _sound;
	delete _music;
	delete _soundsArchive;
	delete _dubbingArchive;

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

	delete _console;
}

Common::Error DraciEngine::run() {
	init();
	setTotalPlayTime(0);
	_game->init();

	// Load game from specified slot, if any
	if (ConfMan.hasKey("save_slot")) {
		loadGameState(ConfMan.getInt("save_slot"));
	}

	_game->start();
	return Common::kNoError;
}

void DraciEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (pause) {
		_pauseStartTime = _system->getMillis();

		_anims->pauseAnimations();
		_sound->pauseSound();
		_sound->pauseVoice();
		_music->pause();
	} else {
		_anims->unpauseAnimations();
		_sound->resumeSound();
		_sound->resumeVoice();
		_music->resume();

		// Adjust engine start time
		const int delta = _system->getMillis() - _pauseStartTime;
		_game->shiftSpeechAndFadeTick(delta);
		_pauseStartTime = 0;
	}
}

void DraciEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_sound->setVolume();
	_music->syncVolume();
}

Common::String DraciEngine::getSavegameFile(int saveGameIdx) {
	return Common::String::format("draci.s%02d", saveGameIdx);
}

Common::Error DraciEngine::loadGameState(int slot) {
	// When called from run() using save_slot, the next operation is the
	// call to start() calling enterNewRoom().
	// When called from handleEvents() in the middle of the game, the next
	// operation after handleEvents() exits from loop(), and returns to
	// start() to the same place as above.
	// In both cases, we are safe to override the data structures right
	// here are now, without waiting for any other code to finish, thanks
	// to our constraint in canLoadGameStateCurrently() and to having
	// enterNewRoom() called right after we exit from here.
	return loadSavegameData(slot, this);
}

bool DraciEngine::canLoadGameStateCurrently() {
	return (_game->getLoopStatus() == kStatusOrdinary) &&
		(_game->getLoopSubstatus() == kOuterLoop);
}

Common::Error DraciEngine::saveGameState(int slot, const Common::String &desc) {
	return saveSavegameData(slot, desc, *this);
}

bool DraciEngine::canSaveGameStateCurrently() {
	return (_game->getLoopStatus() == kStatusOrdinary) &&
		(_game->getLoopSubstatus() == kOuterLoop);
}

} // End of namespace Draci
