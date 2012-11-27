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

#include "common/file.h"
#include "common/fs.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/events.h"

#include "audio/mixer.h"

#include "saga/saga.h"

#include "saga/resource.h"
#include "saga/gfx.h"
#include "saga/render.h"
#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/console.h"
#include "saga/events.h"
#include "saga/font.h"
#include "saga/interface.h"
#include "saga/isomap.h"
#include "saga/puzzle.h"
#include "saga/script.h"
#include "saga/scene.h"
#include "saga/sndres.h"
#include "saga/sprite.h"
#include "saga/sound.h"
#include "saga/music.h"
#include "saga/palanim.h"
#include "saga/objectmap.h"

namespace Saga {

#define MAX_TIME_DELTA 100

SagaEngine::SagaEngine(OSystem *syst, const SAGAGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc), _rnd("saga") {

	_framesEsc = 0;

	_globalFlags = 0;
	memset(_ethicsPoints, 0, sizeof(_ethicsPoints));
	_spiritualBarometer = 0;

	_soundVolume = 0;
	_musicVolume = 0;
	_speechVolume = 0;
	_subtitlesEnabled = false;
	_voicesEnabled = false;
	_voiceFilesExist = false;
	_readingSpeed = 0;

	_copyProtection = false;
	_musicWasPlaying = false;
	_hasITESceneSubstitutes = false;

	_sndRes = NULL;
	_sound = NULL;
	_music = NULL;
	_anim = NULL;
	_render = NULL;
	_isoMap = NULL;
	_gfx = NULL;
	_script = NULL;
	_actor = NULL;
	_font = NULL;
	_sprite = NULL;
	_scene = NULL;
	_interface = NULL;
	_console = NULL;
	_events = NULL;
	_palanim = NULL;
	_puzzle = NULL;
	_resource = NULL;

	_previousTicks = 0;

	_saveFilesCount = 0;

	_leftMouseButtonPressed = _rightMouseButtonPressed = false;
	_mouseClickCount = 0;

	_gameNumber = 0;

	_frameCount = 0;

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	// The Linux version of Inherit the Earth puts all data files in an
	// 'itedata' sub-directory, except for voices.rsc
	SearchMan.addSubDirectoryMatching(gameDataDir, "itedata");

	// The Windows version of Inherit the Earth puts various data files in
	// other subdirectories.
	SearchMan.addSubDirectoryMatching(gameDataDir, "graphics");
	SearchMan.addSubDirectoryMatching(gameDataDir, "music");
	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	// The Multi-OS version puts the voices file in the root directory of
	// the CD. The rest of the data files are in game/itedata
	SearchMan.addSubDirectoryMatching(gameDataDir, "game/itedata");

	// Mac CD Wyrmkeep
	SearchMan.addSubDirectoryMatching(gameDataDir, "patch");

	// Dinotopia
	SearchMan.addSubDirectoryMatching(gameDataDir, "smack");

	// FTA2
	SearchMan.addSubDirectoryMatching(gameDataDir, "video");

	_displayClip.left = _displayClip.top = 0;
}

SagaEngine::~SagaEngine() {
	if (_scene != NULL) {
		if (_scene->isSceneLoaded()) {
			_scene->endScene();
		}
	}

	if (getGameId() == GID_ITE) {
		delete _isoMap;
		_isoMap = NULL;

		delete _puzzle;
		_puzzle = NULL;
	}

	delete _sndRes;
	_sndRes = NULL;

	delete _events;
	_events = NULL;

	if (!isSaga2()) {
		delete _font;
		_font = NULL;

		delete _sprite;
		_sprite = NULL;
	}

	delete _anim;
	_anim = NULL;

	delete _script;
	_script = NULL;

	if (!isSaga2()) {
		delete _interface;
		_interface = NULL;
	}

	delete _actor;
	_actor = NULL;

	delete _palanim;
	_palanim = NULL;

	delete _scene;
	_scene = NULL;

	delete _render;
	_render = NULL;

	delete _music;
	_music = NULL;

	delete _sound;
	_sound = NULL;

	delete _gfx;
	_gfx = NULL;

	delete _console;
	_console = NULL;

	delete _resource;
	_resource = NULL;
}

Common::Error SagaEngine::run() {
	setTotalPlayTime(0);

	// Assign default values to the config manager, in case settings are missing
	ConfMan.registerDefault("talkspeed", "255");
	ConfMan.registerDefault("subtitles", "true");

	_musicVolume = ConfMan.getInt("music_volume");
	_subtitlesEnabled = ConfMan.getBool("subtitles");
	_readingSpeed = getTalkspeed();
	_copyProtection = ConfMan.getBool("copy_protection");
	_musicWasPlaying = false;
	_isIHNMDemo = Common::File::exists("music.res");
	_hasITESceneSubstitutes = Common::File::exists("boarhall.bbm");

	if (_readingSpeed > 3)
		_readingSpeed = 0;

	switch (getGameId()) {
		case GID_ITE:
			_resource = new Resource_RSC(this);
			break;
#ifdef ENABLE_IHNM
		case GID_IHNM:
			_resource = new Resource_RES(this);
			break;
#endif
#ifdef ENABLE_SAGA2
		case GID_DINO:
		case GID_FTA2:
			_resource = new Resource_HRS(this);
			break;
#endif
	}

	// Detect game and open resource files
	if (!initGame()) {
		GUIErrorMessage("Error loading game resources.");
		return Common::kUnknownError;
	}

	// Initialize engine modules
	// TODO: implement differences for SAGA2
	_sndRes = new SndRes(this);
	_events = new Events(this);

	if (!isSaga2()) {
		_font = new Font(this);
		_sprite = new Sprite(this);
		_script = new SAGA1Script(this);
	} else {
		_script = new SAGA2Script(this);
	}

	_anim = new Anim(this);
	_interface = new Interface(this); // requires script module
	_scene = new Scene(this);
	_actor = new Actor(this);
	_palanim = new PalAnim(this);

	if (getGameId() == GID_ITE) {
		_isoMap = new IsoMap(this);
		_puzzle = new Puzzle(this);
	}

	// System initialization

	_previousTicks = _system->getMillis();

	// Initialize graphics
	_gfx = new Gfx(this, _system, getDisplayInfo().width, getDisplayInfo().height);

	// Graphics driver should be initialized before console
	_console = new Console(this);

	// Graphics should be initialized before music
	_music = new Music(this, _mixer);
	_render = new Render(this, _system);
	if (!_render->initialized()) {
		return Common::kUnknownError;
	}

	// Initialize system specific sound
	_sound = new Sound(this, _mixer);

	if (!isSaga2()) {
		_interface->converseClear();
		_script->setVerb(_script->getVerbType(kVerbWalkTo));
	}

	_music->setVolume(_musicVolume, 1);

	if (!isSaga2()) {
		_gfx->initPalette();
	}

	if (_voiceFilesExist) {
		if (getGameId() == GID_IHNM) {
			if (!ConfMan.hasKey("voices")) {
				_voicesEnabled = true;
				ConfMan.setBool("voices", true);
			} else {
				_voicesEnabled = ConfMan.getBool("voices");
			}
		} else {
			_voicesEnabled = true;
		}
	}

	syncSoundSettings();

	int msec = 0;

	_previousTicks = _system->getMillis();

	if (ConfMan.hasKey("start_scene")) {
		_scene->changeScene(ConfMan.getInt("start_scene"), 0, kTransitionNoFade);
	} else if (ConfMan.hasKey("boot_param")) {
		if (getGameId() == GID_ITE)
			_interface->addToInventory(_actor->objIndexToId(0));	// Magic hat
		_scene->changeScene(ConfMan.getInt("boot_param"), 0, kTransitionNoFade);
	} else if (ConfMan.hasKey("save_slot")) {
		// Init the current chapter to 8 (character selection) for IHNM
		if (getGameId() == GID_IHNM)
			_scene->changeScene(-2, 0, kTransitionFade, 8);

		// First scene sets up palette
		_scene->changeScene(getStartSceneNumber(), 0, kTransitionNoFade);
		_events->handleEvents(0); // Process immediate events

		if (getGameId() == GID_ITE)
			_interface->setMode(kPanelMain);
		else
			_interface->setMode(kPanelChapterSelection);

		char *fileName = calcSaveFileName(ConfMan.getInt("save_slot"));
		load(fileName);
		syncSoundSettings();
	} else {
		_framesEsc = 0;
		//_sndRes->playVoice(0);    // SAGA 2 sound test
		_scene->startScene();
	}

	uint32 currentTicks;

	while (!shouldQuit()) {
		_console->onFrame();

		if (_render->getFlags() & RF_RENDERPAUSE) {
			// Freeze time while paused
			_previousTicks = _system->getMillis();
		} else {
			currentTicks = _system->getMillis();
			// Timer has rolled over after 49 days
			if (currentTicks < _previousTicks)
				msec = 0;
			else {
				msec = currentTicks - _previousTicks;
				_previousTicks = currentTicks;
			}
			if (msec > MAX_TIME_DELTA) {
				msec = MAX_TIME_DELTA;
			}

			// Since Puzzle and forced text are actorless, we do them here
			if ((getGameId() == GID_ITE && _puzzle->isActive()) || _actor->isForcedTextShown()) {
				_actor->handleSpeech(msec);
			} else if (!_scene->isInIntro()) {
				if (_interface->getMode() == kPanelMain ||
						_interface->getMode() == kPanelConverse ||
						_interface->getMode() == kPanelCutaway ||
						_interface->getMode() == kPanelNull ||
						_interface->getMode() == kPanelChapterSelection)
					_actor->direct(msec);
			}

			_events->handleEvents(msec);
			_script->executeThreads(msec);
		}
		// Per frame processing
		_render->drawScene();
		_system->delayMillis(10);
	}

	return Common::kNoError;
}

void SagaEngine::loadStrings(StringsTable &stringsTable, const ByteArray &stringsData) {
	uint16 stringsCount;
	size_t offset;
	size_t prevOffset = 0;
	Common::Array<size_t> tempOffsets;
	uint ui;

	if (stringsData.empty()) {
		error("SagaEngine::loadStrings() Error loading strings list resource");
	}


	ByteArrayReadStreamEndian scriptS(stringsData, isBigEndian()); //TODO: get endianess from context

	offset = scriptS.readUint16();
	stringsCount = offset / 2;
	ui = 0;
	scriptS.seek(0);
	tempOffsets.resize(stringsCount);
	while (ui < stringsCount) {
		offset = scriptS.readUint16();
		// In some rooms in IHNM, string offsets can be greater than the maximum value than a 16-bit integer can hold
		// We detect this by checking the previous offset, and if it was bigger than the current one, an overflow
		// occurred (since the string offsets are sequential), so we're adding the missing part of the number
		// Fixes bug #1895205 - "IHNM: end game text/caption error"
		if (prevOffset > offset)
			offset += 65536;
		prevOffset = offset;
		if (offset == stringsData.size()) {
			stringsCount = ui;
			tempOffsets.resize(stringsCount);
			break;
		}
		if (offset > stringsData.size()) {
			// This case should never occur, but apparently it does in the Italian fan
			// translation of IHNM
			warning("SagaEngine::loadStrings wrong strings table");
			stringsCount = ui;
			tempOffsets.resize(stringsCount);
			break;
		}
		tempOffsets[ui] = offset;
		ui++;
	}

	prevOffset = scriptS.pos();
	int32 left = scriptS.size() - prevOffset;
	if (left < 0) {
		error("SagaEngine::loadStrings() Error loading strings buffer");
	}

	stringsTable.buffer.resize(left);
	if (left > 0) {
		scriptS.read(&stringsTable.buffer.front(), left);
	}

	stringsTable.strings.resize(tempOffsets.size());
	for (ui = 0; ui < tempOffsets.size(); ui++) {
		offset = tempOffsets[ui] - prevOffset;
		if (offset >= stringsTable.buffer.size()) {
			error("SagaEngine::loadStrings() Wrong offset");
		}
		stringsTable.strings[ui] = &stringsTable.buffer[offset];

		debug(9, "string[%i]=%s", ui, stringsTable.strings[ui]);
	}
}

const char *SagaEngine::getObjectName(uint16 objectId) const {
	ActorData *actor;
	ObjectData *obj;
	const HitZone *hitZone;

	// Disable the object names in IHNM when the chapter is 8
	if (getGameId() == GID_IHNM && _scene->currentChapterNumber() == 8)
		return "";

	switch (objectTypeId(objectId)) {
	case kGameObjectObject:
		obj = _actor->getObj(objectId);
		if (getGameId() == GID_ITE)
			return _script->_mainStrings.getString(obj->_nameIndex);
		return _actor->_objectsStrings.getString(obj->_nameIndex);
	case kGameObjectActor:
		actor = _actor->getActor(objectId);
		return _actor->_actorsStrings.getString(actor->_nameIndex);
	case kGameObjectHitZone:
		hitZone = _scene->_objectMap->getHitZone(objectIdToIndex(objectId));

		if (hitZone == NULL)
			return "";

		return _scene->_sceneStrings.getString(hitZone->getNameIndex());
	}
	warning("SagaEngine::getObjectName name not found for 0x%X", objectId);
	return NULL;
}

const char *SagaEngine::getTextString(int textStringId) {
	const char *string;
	int lang = 0;

	switch (getLanguage()) {
		case Common::DE_DEU:
			lang = 1;
			break;
		case Common::IT_ITA:
			lang = 2;
			break;
		case Common::ES_ESP:
			lang = 3;
			break;
		default:
			lang = 0;
			break;
	}

	string = ITEinterfaceTextStrings[lang][textStringId];
	if (!string)
		string = ITEinterfaceTextStrings[0][textStringId];

	return string;
}

void SagaEngine::getExcuseInfo(int verb, const char *&textString, int &soundResourceId) {
	textString = NULL;

	if (verb == _script->getVerbType(kVerbOpen)) {
		textString = getTextString(kTextNoPlaceToOpen);
		soundResourceId = 239;		// Boar voice 0
	}
	if (verb == _script->getVerbType(kVerbClose)) {
		textString = getTextString(kTextNoOpening);
		soundResourceId = 241;		// Boar voice 2
	}
	if (verb == _script->getVerbType(kVerbUse)) {
		textString = getTextString(kTextDontKnow);
		soundResourceId = 244;		// Boar voice 5
	}
	if (verb == _script->getVerbType(kVerbLookAt)) {
		textString = getTextString(kTextNothingSpecial);
		soundResourceId = 245;		// Boar voice 6
	}
	if (verb == _script->getVerbType(kVerbPickUp)) {
		textString = getTextString(kTextICantPickup);
		soundResourceId = 246;		// Boar voice 7
	}
}

ColorId SagaEngine::KnownColor2ColorId(KnownColor knownColor) {
	ColorId colorId = kITEColorTransBlack;

	if (getGameId() == GID_ITE) {
		switch (knownColor) {
		case(kKnownColorTransparent):
			colorId = kITEColorTransBlack;
			break;
		case (kKnownColorBrightWhite):
			colorId = kITEColorBrightWhite;
			break;
		case (kKnownColorWhite):
			colorId = kITEColorWhite;
			break;
		case (kKnownColorBlack):
			colorId = kITEColorBlack;
			break;
		case (kKnownColorSubtitleTextColor):
			colorId = (ColorId)255;
			break;
		case (kKnownColorVerbText):
			colorId = kITEColorBlue;
			break;
		case (kKnownColorVerbTextShadow):
			colorId = kITEColorBlack;
			break;
		case (kKnownColorVerbTextActive):
			colorId = (ColorId)96;
			break;

		default:
			error("SagaEngine::KnownColor2ColorId unknown color %i", knownColor);
		}
#ifdef ENABLE_IHNM
	} else if (getGameId() == GID_IHNM) {
		// The default colors in the Spanish version of IHNM are shifted by one
		// Fixes bug #1848016 - "IHNM: Wrong Subtitles Color (Spanish)"
		int offset = (getLanguage() == Common::ES_ESP) ? 1 : 0;

		switch (knownColor) {
		case(kKnownColorTransparent):
			colorId = (ColorId)(249 - offset);
			break;
		case (kKnownColorBrightWhite):
			colorId = (ColorId)(251 - offset);
			break;
		case (kKnownColorWhite):
			colorId = (ColorId)(251 - offset);
			break;
		case (kKnownColorBlack):
			colorId = (ColorId)(249 - offset);
			break;
		case (kKnownColorVerbText):
			colorId = (ColorId)(253 - offset);
			break;
		case (kKnownColorVerbTextShadow):
			colorId = (ColorId)(15 - offset);
			break;
		case (kKnownColorVerbTextActive):
			colorId = (ColorId)(252 - offset);
			break;

		default:
			error("SagaEngine::KnownColor2ColorId unknown color %i", knownColor);
		}
#endif
	}
	return colorId;
}

void SagaEngine::setTalkspeed(int talkspeed) {
	ConfMan.setInt("talkspeed", (talkspeed * 255 + 3 / 2) / 3);
}

int SagaEngine::getTalkspeed() const {
	return (ConfMan.getInt("talkspeed") * 3 + 255 / 2) / 255;
}

GUI::Debugger *SagaEngine::getDebugger() {
	return _console;
}

void SagaEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_subtitlesEnabled = ConfMan.getBool("subtitles");
	_readingSpeed = getTalkspeed();

	if (_readingSpeed > 3)
		_readingSpeed = 0;

	_musicVolume = ConfMan.getInt("music_volume");
	_music->setVolume(_musicVolume, 1);
	_sound->setVolume();
}

void SagaEngine::pauseEngineIntern(bool pause) {
	bool engineIsPaused = (_render->getFlags() & RF_RENDERPAUSE);
	if (engineIsPaused == pause)
		return;

	if (pause) {
		_render->setFlag(RF_RENDERPAUSE);
		if (_music->isPlaying() && !_music->hasDigitalMusic()) {
			_music->pause();
			_musicWasPlaying = true;
		} else {
			_musicWasPlaying = false;
		}
	} else {
		_render->clearFlag(RF_RENDERPAUSE);
		if (_musicWasPlaying) {
			_music->resume();
		}
	}

	_mixer->pauseAll(pause);
}

} // End of namespace Saga
