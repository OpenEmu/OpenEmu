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

#include "kyra/kyra_v1.h"
#include "kyra/sound_intern.h"
#include "kyra/resource.h"
#include "kyra/timer.h"
#include "kyra/debugger.h"

#include "common/error.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"

namespace Kyra {

KyraEngine_v1::KyraEngine_v1(OSystem *system, const GameFlags &flags)
	: Engine(system), _flags(flags), _rnd("kyra") {
	_res = 0;
	_sound = 0;
	_text = 0;
	_staticres = 0;
	_timer = 0;
	_emc = 0;
	_debugger = 0;

	_configRenderMode = Common::kRenderDefault;

	if (_flags.platform == Common::kPlatformAmiga)
		_gameSpeed = 50;
	else
		_gameSpeed = 60;
	_tickLength = (uint8)(1000.0 / _gameSpeed);

	_trackMap = 0;
	_trackMapSize = 0;
	_lastMusicCommand = -1;
	_curSfxFile = _curMusicTheme = -1;

	_gameToLoad = -1;

	_mouseState = -1;
	_deathHandler = -1;

	memset(_flagsTable, 0, sizeof(_flagsTable));

	_isSaveAllowed = false;

	_mouseX = _mouseY = 0;

	// sets up all engine specific debug levels
	DebugMan.addDebugChannel(kDebugLevelScriptFuncs, "ScriptFuncs", "Script function debug level");
	DebugMan.addDebugChannel(kDebugLevelScript, "Script", "Script interpreter debug level");
	DebugMan.addDebugChannel(kDebugLevelSprites, "Sprites", "Sprite debug level");
	DebugMan.addDebugChannel(kDebugLevelScreen, "Screen", "Screen debug level");
	DebugMan.addDebugChannel(kDebugLevelSound, "Sound", "Sound debug level");
	DebugMan.addDebugChannel(kDebugLevelAnimator, "Animator", "Animator debug level");
	DebugMan.addDebugChannel(kDebugLevelMain, "Main", "Generic debug level");
	DebugMan.addDebugChannel(kDebugLevelGUI, "GUI", "GUI debug level");
	DebugMan.addDebugChannel(kDebugLevelSequence, "Sequence", "Sequence debug level");
	DebugMan.addDebugChannel(kDebugLevelMovie, "Movie", "Movie debug level");
	DebugMan.addDebugChannel(kDebugLevelTimer, "Timer", "Timer debug level");
}

::GUI::Debugger *KyraEngine_v1::getDebugger() {
	return _debugger;
}

void KyraEngine_v1::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (_sound)
		_sound->pause(pause);
	if (_timer)
		_timer->pause(pause);
}

Common::Error KyraEngine_v1::init() {
	// Setup mixer
	syncSoundSettings();

	if (!_flags.useDigSound) {
		if (_flags.platform == Common::kPlatformFMTowns) {
			if (_flags.gameID == GI_KYRA1)
				_sound = new SoundTowns(this, _mixer);
			else
				_sound = new SoundTownsPC98_v2(this, _mixer);
		} else if (_flags.platform == Common::kPlatformPC98) {
			if (_flags.gameID == GI_KYRA1)
				_sound = new SoundPC98(this, _mixer);
			else
				_sound = new SoundTownsPC98_v2(this, _mixer);
		} else if (_flags.platform == Common::kPlatformAmiga) {
			_sound = new SoundAmiga(this, _mixer);
		} else {
			// In Kyra 1 users who have specified a default MT-32 device in the launcher settings
			// will get MT-32 music, otherwise AdLib. In Kyra 2 and LoL users who have specified a
			// default GM device in the launcher will get GM music, otherwise AdLib. Users who want
			// MT-32 music in Kyra2 or LoL have to select this individually (since we assume that
			// most users rather have a GM device than a MT-32 device).
			// Users who want PC speaker sound always have to select this individually for all
			// Kyra games.
			MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_PCSPK | MDT_MIDI | MDT_ADLIB | ((_flags.gameID == GI_KYRA2 || _flags.gameID == GI_LOL) ? MDT_PREFER_GM : MDT_PREFER_MT32));
			if (MidiDriver::getMusicType(dev) == MT_ADLIB) {
				_sound = new SoundAdLibPC(this, _mixer);
			} else {
				Sound::kType type;
				const MusicType midiType = MidiDriver::getMusicType(dev);

				if (midiType == MT_PCSPK || midiType == MT_NULL)
					type = Sound::kPCSpkr;
				else if (midiType == MT_MT32 || ConfMan.getBool("native_mt32"))
					type = Sound::kMidiMT32;
				else
					type = Sound::kMidiGM;

				MidiDriver *driver = 0;

				if (MidiDriver::getMusicType(dev) == MT_PCSPK) {
					driver = new MidiDriver_PCSpeaker(_mixer);
				} else {
					driver = MidiDriver::createMidi(dev);
					if (type == Sound::kMidiMT32)
						driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
				}

				assert(driver);

				SoundMidiPC *soundMidiPc = new SoundMidiPC(this, _mixer, driver, type);
				_sound = soundMidiPc;
				assert(_sound);

				// Unlike some SCUMM games, it's not that the MIDI sounds are
				// missing. It's just that at least at the time of writing they
				// are decidedly inferior to the AdLib ones.
				if (ConfMan.getBool("multi_midi")) {
					SoundAdLibPC *adlib = new SoundAdLibPC(this, _mixer);
					assert(adlib);

					_sound = new MixedSoundDriver(this, _mixer, soundMidiPc, adlib);
				}
			}
		}

		assert(_sound);
	}

	if (_sound)
		_sound->updateVolumeSettings();

	if (ConfMan.hasKey("render_mode"))
		_configRenderMode = Common::parseRenderMode(ConfMan.get("render_mode"));

	_res = new Resource(this);
	assert(_res);
	_res->reset();

	_staticres = new StaticResource(this);
	assert(_staticres);
	if (!_staticres->init())
		error("_staticres->init() failed");
	assert(screen());
	if (!screen()->init())
		error("screen()->init() failed");
	_timer = new TimerManager(this, _system);
	assert(_timer);
	setupTimers();
	_emc = new EMCInterpreter(this);
	assert(_emc);

	setupOpcodeTable();
	readSettings();

	if (ConfMan.hasKey("save_slot")) {
		_gameToLoad = ConfMan.getInt("save_slot");
		if (!saveFileLoadable(_gameToLoad))
			_gameToLoad = -1;
	}

	setupKeyMap();

	// Prevent autosave on game startup
	_lastAutosave = _system->getMillis();

	return Common::kNoError;
}

KyraEngine_v1::~KyraEngine_v1() {
	for (Common::Array<const Opcode *>::iterator i = _opcodes.begin(); i != _opcodes.end(); ++i)
		delete *i;
	_opcodes.clear();
	_keyMap.clear();

	delete _res;
	delete _staticres;
	delete _sound;
	delete _text;
	delete _timer;
	delete _emc;
	delete _debugger;
}

Common::Point KyraEngine_v1::getMousePos() {
	Common::Point mouse = _eventMan->getMousePos();

	if (_flags.useHiRes) {
		mouse.x >>= 1;
		mouse.y >>= 1;
	}

	return mouse;
}

void KyraEngine_v1::setMousePos(int x, int y) {
	if (_flags.useHiRes) {
		x <<= 1;
		y <<= 1;
	}
	_system->warpMouse(x, y);
}

int KyraEngine_v1::checkInput(Button *buttonList, bool mainLoop, int eventFlag) {
	_isSaveAllowed = mainLoop;
	updateInput();
	_isSaveAllowed = false;

	if (mainLoop)
		checkAutosave();

	int keys = 0;
	int8 mouseWheel = 0;

	while (!_eventList.empty()) {
		Common::Event event = *_eventList.begin();
		bool breakLoop = false;

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode >= Common::KEYCODE_1 && event.kbd.keycode <= Common::KEYCODE_9 &&
			        (event.kbd.hasFlags(Common::KBD_CTRL) || event.kbd.hasFlags(Common::KBD_ALT)) && mainLoop) {
				int saveLoadSlot = 9 - (event.kbd.keycode - Common::KEYCODE_0) + 990;

				if (event.kbd.hasFlags(Common::KBD_CTRL)) {
					if (saveFileLoadable(saveLoadSlot))
						loadGameStateCheck(saveLoadSlot);
					_eventList.clear();
					breakLoop = true;
				} else {
					char savegameName[14];
					sprintf(savegameName, "Quicksave %d", event.kbd.keycode - Common::KEYCODE_0);
					saveGameStateIntern(saveLoadSlot, savegameName, 0);
				}
			} else if (event.kbd.hasFlags(Common::KBD_CTRL)) {
				if (event.kbd.keycode == Common::KEYCODE_d) {
					if (_debugger)
						_debugger->attach();
					breakLoop = true;
				} else if (event.kbd.keycode == Common::KEYCODE_q) {
					quitGame();
				}
			} else {
				KeyMap::const_iterator keycode = _keyMap.find(event.kbd.keycode);
				if (keycode != _keyMap.end()) {
					keys = keycode->_value;
					if (event.kbd.flags & Common::KBD_SHIFT)
						keys |= 0x100;
				} else {
					keys = 0;
				}

				// When we got an keypress, which we might need to handle,
				// break the event loop and pass it to GUI code.
				if (keys)
					breakLoop = true;
			}
			break;

		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP: {
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			if (_flags.useHiRes) {
				_mouseX >>= 1;
				_mouseY >>= 1;
			}
			keys = (event.type == Common::EVENT_LBUTTONDOWN ? 199 : (200 | 0x800));
			breakLoop = true;
			} break;

		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP: {
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			if (_flags.useHiRes) {
				_mouseX >>= 1;
				_mouseY >>= 1;
			}
			keys = (event.type == Common::EVENT_RBUTTONDOWN ? 201 : (202 | 0x800));
			breakLoop = true;
			} break;

		case Common::EVENT_WHEELUP:
			mouseWheel = -1;
			break;

		case Common::EVENT_WHEELDOWN:
			mouseWheel = 1;
			break;

		default:
			break;
		}

		if (_debugger)
			_debugger->onFrame();

		if (breakLoop)
			break;

		_eventList.erase(_eventList.begin());
	}

	GUI *guiInstance = gui();
	if (guiInstance) {
		if (keys)
			return guiInstance->processButtonList(buttonList, keys | eventFlag, mouseWheel);
		else
			return guiInstance->processButtonList(buttonList, 0, mouseWheel);
	} else {
		return keys;
	}
}

void KyraEngine_v1::setupKeyMap() {
	struct KeyCodeMapEntry {
		Common::KeyCode kcScummVM;
		int16 kcDOS;
		int16 kcPC98;
	};

#define KC(x) Common::KEYCODE_##x
	static const KeyCodeMapEntry keys[] = {
		{ KC(SPACE), 61, 53 },
		{ KC(RETURN), 43, 29 },
		{ KC(UP), 96, 68 },
		{ KC(KP8), 96, 68 },
		{ KC(RIGHT), 102, 73 },
		{ KC(KP6), 102, 73 },
		{ KC(DOWN), 98, 76 },
		{ KC(KP2), 98, 76 },
		{ KC(KP5), 97, 72 },
		{ KC(LEFT), 92, 71 },
		{ KC(KP4), 92, 71 },
		{ KC(HOME), 91, 67 },
		{ KC(KP7), 91, 67 },
		{ KC(PAGEUP), 101, 69 },
		{ KC(KP9), 101, 69 },
		{ KC(END), 93, 0/*unknown*/ },
		{ KC(KP1), 93, 0/*unknown*/ },
		{ KC(PAGEDOWN), 103, 0/*unknown*/ },
		{ KC(KP3), 103, 0/*unknown*/ },
		{ KC(F1), 112, 99 },
		{ KC(F2), 113, 100 },
		{ KC(F3), 114, 101 },
		{ KC(F4), 115, 102 },
		{ KC(F5), 116, 103 },
		{ KC(F6), 117, 104 },
		{ KC(a), 31, 31 },
		{ KC(b), 50, 50 },
		{ KC(c), 48, 48 },
		{ KC(d), 33, 33 },
		{ KC(e), 19, 19 },
		{ KC(f), 34, 34 },
		{ KC(i), 24, 24 },
		{ KC(k), 38, 38 },
		{ KC(m), 52, 52 },
		{ KC(n), 51, 51 },
		{ KC(o), 25, 25 },
		{ KC(p), 26, 26 },
		{ KC(r), 20, 20 },
		{ KC(s), 32, 32 },
		{ KC(w), 18, 18 },
		{ KC(y), 22, 22 },
		{ KC(z), 46, 46 },
		{ KC(1), 2, 0/*unknown*/ },
		{ KC(2), 3, 0/*unknown*/ },
		{ KC(3), 4, 0/*unknown*/ },
		{ KC(4), 5, 0/*unknown*/ },
		{ KC(5), 6, 0/*unknown*/ },
		{ KC(6), 7, 0/*unknown*/ },
		{ KC(7), 8, 0/*unknown*/ },
		{ KC(SLASH), 55, 55 },
		{ KC(ESCAPE), 110, 1 },
		{ KC(MINUS), 12, 0/*unknown*/ },
		{ KC(KP_MINUS), 105, 0/*unknown*/ },
		{ KC(PLUS), 13, 0/*unknown*/ },
		{ KC(KP_PLUS), 106, 0/*unknown*/ },

		// Multiple mappings for the keys to the right of the 'M' key,
		// since these are different for QWERTZ, QWERTY and AZERTY keyboards.
		// QWERTZ
		{ KC(COMMA), 53, 0/*unknown*/ },
		{ KC(PERIOD), 54, 0/*unknown*/ },
		// AZERTY
		{ KC(SEMICOLON), 53, 0/*unknown*/ },
		{ KC(COLON), 54, 0/*unknown*/ },
		// QWERTY
		{ KC(LESS), 53, 0/*unknown*/ },
		{ KC(GREATER), 54, 0/*unknown*/ }
	};
#undef KC

	_keyMap.clear();

	for (int i = 0; i < ARRAYSIZE(keys); i++)
		_keyMap[keys[i].kcScummVM] = (_flags.platform == Common::kPlatformPC98) ? keys[i].kcPC98 : keys[i].kcDOS;
}

void KyraEngine_v1::updateInput() {
	Common::Event event;

	bool updateScreen = false;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_PERIOD || event.kbd.keycode == Common::KEYCODE_ESCAPE ||
			        event.kbd.keycode == Common::KEYCODE_SPACE || event.kbd.keycode == Common::KEYCODE_RETURN ||
			        event.kbd.keycode == Common::KEYCODE_UP || event.kbd.keycode == Common::KEYCODE_RIGHT ||
			        event.kbd.keycode == Common::KEYCODE_DOWN || event.kbd.keycode == Common::KEYCODE_LEFT)
				_eventList.push_back(Event(event, true));
			else if (event.kbd.keycode == Common::KEYCODE_q && event.kbd.hasFlags(Common::KBD_CTRL))
				quitGame();
			else
				_eventList.push_back(event);
			break;

		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			_eventList.push_back(Event(event, true));
			break;

		case Common::EVENT_MOUSEMOVE:
			if (screen()->isMouseVisible())
				updateScreen = true;
			break;

		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
		case Common::EVENT_WHEELUP:
		case Common::EVENT_WHEELDOWN:
			_eventList.push_back(event);
			break;

		default:
			break;
		}
	}

	if (updateScreen)
		_system->updateScreen();
}

void KyraEngine_v1::removeInputTop() {
	if (!_eventList.empty())
		_eventList.erase(_eventList.begin());
}

bool KyraEngine_v1::skipFlag() const {
	for (Common::List<Event>::const_iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip)
			return true;
	}
	return false;
}

void KyraEngine_v1::resetSkipFlag(bool removeEvent) {
	for (Common::List<Event>::iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip) {
			if (removeEvent)
				_eventList.erase(i);
			else
				i->causedSkip = false;
			return;
		}
	}
}


int KyraEngine_v1::setGameFlag(int flag) {
	assert((flag >> 3) >= 0 && (flag >> 3) <= ARRAYSIZE(_flagsTable));
	_flagsTable[flag >> 3] |= (1 << (flag & 7));
	return 1;
}

int KyraEngine_v1::queryGameFlag(int flag) const {
	assert((flag >> 3) >= 0 && (flag >> 3) <= ARRAYSIZE(_flagsTable));
	return ((_flagsTable[flag >> 3] >> (flag & 7)) & 1);
}

int KyraEngine_v1::resetGameFlag(int flag) {
	assert((flag >> 3) >= 0 && (flag >> 3) <= ARRAYSIZE(_flagsTable));
	_flagsTable[flag >> 3] &= ~(1 << (flag & 7));
	return 0;
}

void KyraEngine_v1::delayUntil(uint32 timestamp, bool updateTimers, bool update, bool isMainLoop) {
	const uint32 curTime = _system->getMillis();
	if (curTime > timestamp)
		return;

	uint32 del = timestamp - curTime;
	while (del && !shouldQuit()) {
		uint32 step = MIN<uint32>(del, _tickLength);
		delay(step, update, isMainLoop);
		del -= step;
	}
}

void KyraEngine_v1::delay(uint32 amount, bool update, bool isMainLoop) {
	_system->delayMillis(amount);
}

void KyraEngine_v1::delayWithTicks(int ticks) {
	delay(ticks * _tickLength);
}

void KyraEngine_v1::registerDefaultSettings() {
	if (_flags.platform == Common::kPlatformFMTowns)
		ConfMan.registerDefault("cdaudio", true);
	if (_flags.fanLang != Common::UNK_LANG) {
		// HACK/WORKAROUND: Since we can't use registerDefault here to overwrite
		// the global subtitles settings, we're using this hack to enable subtitles
		// for fan translations
		const Common::ConfigManager::Domain *cur = ConfMan.getActiveDomain();
		if (!cur || (cur && cur->getVal("subtitles").empty()))
			ConfMan.setBool("subtitles", true);
	}
}

void KyraEngine_v1::readSettings() {
	_configWalkspeed = ConfMan.getInt("walkspeed");
	_configMusic = 0;

	if (!ConfMan.getBool("music_mute")) {
		if (_flags.platform == Common::kPlatformFMTowns)
			_configMusic = ConfMan.getBool("cdaudio") ? 2 : 1;
		else
			_configMusic = 1;
	}
	_configSounds = ConfMan.getBool("sfx_mute") ? 0 : 1;

	if (_sound) {
		_sound->enableMusic(_configMusic);
		_sound->enableSFX(_configSounds);
	}

	bool speechMute = ConfMan.getBool("speech_mute");
	bool subtitles = ConfMan.getBool("subtitles");

	if (!speechMute && subtitles)
		_configVoice = 2;   // Voice & Text
	else if (!speechMute && !subtitles)
		_configVoice = 1;   // Voice only
	else
		_configVoice = 0;   // Text only

	setWalkspeed(_configWalkspeed);
}

void KyraEngine_v1::writeSettings() {
	bool speechMute, subtitles;

	ConfMan.setInt("walkspeed", _configWalkspeed);
	ConfMan.setBool("music_mute", _configMusic == 0);
	if (_flags.platform == Common::kPlatformFMTowns)
		ConfMan.setBool("cdaudio", _configMusic == 2);
	ConfMan.setBool("sfx_mute", _configSounds == 0);

	switch (_configVoice) {
	case 0:     // Text only
		speechMute = true;
		subtitles = true;
		break;
	case 1:     // Voice only
		speechMute = false;
		subtitles = false;
		break;
	default:    // Voice & Text
		speechMute = false;
		subtitles = true;
	}

	if (_sound) {
		if (!_configMusic)
			_sound->beginFadeOut();
		_sound->enableMusic(_configMusic);
		_sound->enableSFX(_configSounds);
	}

	ConfMan.setBool("speech_mute", speechMute);
	ConfMan.setBool("subtitles", subtitles);

	ConfMan.flushToDisk();
}

bool KyraEngine_v1::speechEnabled() {
	return _flags.isTalkie && (_configVoice == 1 || _configVoice == 2);
}

bool KyraEngine_v1::textEnabled() {
	return !_flags.isTalkie || (_configVoice == 0 || _configVoice == 2);
}

int KyraEngine_v1::convertVolumeToMixer(int value) {
	value -= 2;
	return (value * Audio::Mixer::kMaxMixerVolume) / 95;
}

int KyraEngine_v1::convertVolumeFromMixer(int value) {
	return (value * 95) / Audio::Mixer::kMaxMixerVolume + 2;
}

void KyraEngine_v1::setVolume(kVolumeEntry vol, uint8 value) {
	switch (vol) {
	case kVolumeMusic:
		ConfMan.setInt("music_volume", convertVolumeToMixer(value));
		break;

	case kVolumeSfx:
		ConfMan.setInt("sfx_volume", convertVolumeToMixer(value));
		break;

	case kVolumeSpeech:
		ConfMan.setInt("speech_volume", convertVolumeToMixer(value));
		break;
	}

	// Resetup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	if (_sound)
		_sound->updateVolumeSettings();
}

uint8 KyraEngine_v1::getVolume(kVolumeEntry vol) {
	switch (vol) {
	case kVolumeMusic:
		return convertVolumeFromMixer(ConfMan.getInt("music_volume"));

	case kVolumeSfx:
		return convertVolumeFromMixer(ConfMan.getInt("sfx_volume"));

	case kVolumeSpeech:
		if (speechEnabled())
			return convertVolumeFromMixer(ConfMan.getInt("speech_volume"));
		else
			return 2;
		break;
	}

	return 2;
}

void KyraEngine_v1::syncSoundSettings() {
	Engine::syncSoundSettings();

	// We need to use this here to allow the subtitle options to be changed
	// through the GMM's options dialog.
	readSettings();

	if (_sound)
		_sound->updateVolumeSettings();
}

} // End of namespace Kyra
