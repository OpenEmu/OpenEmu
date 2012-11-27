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
#include "common/events.h"
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/func.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/util.h"

#include "engines/advancedDetector.h"

#include "graphics/palette.h"
#include "graphics/surface.h"

#include "dreamweb/sound.h"
#include "dreamweb/dreamweb.h"

namespace DreamWeb {

DreamWebEngine::DreamWebEngine(OSystem *syst, const DreamWebGameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc), _rnd("dreamweb"),
	_exText(kNumExTexts),
	_setDesc(kNumSetTexts), _blockDesc(kNumBlockTexts),
	_roomDesc(kNumRoomTexts), _freeDesc(kNumFreeTexts),
	_personText(kNumPersonTexts) {

	_vSyncInterrupt = false;

	_console = 0;
	_sound = 0;
	DebugMan.addDebugChannel(kDebugAnimation, "Animation", "Animation Debug Flag");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
	_speed = 1;
	_turbo = false;
	_oldMouseState = 0;

	_datafilePrefix = "DREAMWEB.";
	_speechDirName = "SPEECH";
	// ES and FR CD release use a different data file prefix
	// and speech directory naming.
	if (isCD()) {
		switch(getLanguage()) {
		case Common::ES_ESP:
			_datafilePrefix = "DREAMWSP.";
			_speechDirName = "SPANISH";
			break;
		case Common::FR_FRA:
			_datafilePrefix = "DREAMWFR.";
			_speechDirName = "FRENCH";
			break;
		default:
			// Nothing to do
			break;
		}
	}

	_openChangeSize = kInventx+(4*kItempicsize);
	_quitRequested = false;

	_speechLoaded = false;

	_backdropBlocks = 0;
	_reelList = 0;

	_oldSubject._type = 0;
	_oldSubject._index = 0;

	// misc variables
	_speechCount = 0;
	_charShift = 0;
	_kerning = 0;
	_brightPalette = false;
	_roomLoaded = 0;
	_didZoom = 0;
	_lineSpacing = 10;
	_textAddressX = 13;
	_textAddressY = 182;
	_textLen = 0;
	_lastXPos = 0;
	_itemFrame = 0;
	_withObject = 0;
	_withType = 0;
	_lookCounter = 0;
	_command = 0;
	_commandType = 0;
	_objectType = 0;
	_getBack = 0;
	_invOpen = 0;
	_mainMode = 0;
	_pickUp = 0;
	_lastInvPos = 0;
	_examAgain = 0;
	_newTextLine = 0;
	_openedOb = 0;
	_openedType = 0;
	_mapAdX = 0;
	_mapAdY = 0;
	_mapOffsetX = 104;
	_mapOffsetY = 38;
	_mapXStart = 0;
	_mapYStart = 0;
	_mapXSize = 0;
	_mapYSize = 0;
	_haveDoneObs = 0;
	_manIsOffScreen = 0;
	_facing = 0;
	_leaveDirection = 0;
	_turnToFace = 0;
	_turnDirection = 0;
	_mainTimer = 0;
	_introCount = 0;
	_currentKey = 0;
	_timerCount = 0;
	_mapX = 0;
	_mapY = 0;
	_ryanX = 0;
	_ryanY = 0;
	_lastFlag = 0;
	_destPos = 0;
	_realLocation = 0;
	_roomNum = 0;
	_nowInNewRoom = 0;
	_resetManXY = 0;
	_newLocation = 0xFF;
	_autoLocation = 0xFF;
	_mouseX = 0;
	_mouseY = 0;
	_mouseButton = 0;
	_oldButton = 0;
	_oldX = 0;
	_oldY = 0;
	_oldPointerX = 0;
	_oldPointerY = 0;
	_delHereX = 0;
	_delHereY = 0;
	_pointerXS = 32;
	_pointerYS = 32;
	_delXS = 0;
	_delYS = 0;
	_pointerFrame = 0;
	_pointerPower = 0;
	_pointerMode = 0;
	_pointerSpeed = 0;
	_pointerCount = 0;
	_inMapArea = 0;
	_talkMode = 0;
	_talkPos = 0;
	_character = 0;
	_watchDump = 0;
	_logoNum = 0;
	_oldLogoNum = 0;
	_pressed = 0;
	_pressPointer = 0;
	_graphicPress = 0;
	_pressCount = 0;
	_lightCount = 0;
	_folderPage = 0;
	_diaryPage = 0;
	_menuCount = 0;
	_symbolTopX = 0;
	_symbolTopNum = 0;
	_symbolTopDir = 0;
	_symbolBotX = 0;
	_symbolBotNum = 0;
	_symbolBotDir = 0;
	_walkAndExam = 0;
	_walkExamType = 0;
	_walkExamNum = 0;
	_cursLocX = 0;
	_cursLocY = 0;
	_curPos = 0;
	_monAdX = 0;
	_monAdY = 0;
	_timeCount = 0;
	_needToDumpTimed = 0;
	_loadingOrSave = 0;
	_saveLoadPage = 0;
	_currentSlot = 0;
	_cursorPos = 0;
	_colourPos = 0;
	_fadeDirection = 0;
	_numToFade = 0;
	_fadeCount = 0;
	_addToGreen = 0;
	_addToRed = 0;
	_addToBlue = 0;
	_lastSoundReel = 0;
	_lastHardKey = 0;
	_bufferIn = 0;
	_bufferOut = 0;
	_blinkFrame = 23;
	_blinkCount = 0;
	_reAssesChanges = 0;
	_pointersPath = 0;
	_mansPath = 0;
	_pointerFirstPath = 0;
	_finalDest = 0;
	_destination = 0;
	_lineStartX = 0;
	_lineStartY = 0;
	_lineEndX = 0;
	_lineEndY = 0;
	_linePointer = 0;
	_lineDirection = 0;
	_lineLength = 0;
}

DreamWebEngine::~DreamWebEngine() {
	DebugMan.clearAllDebugChannels();
	delete _console;
	delete _sound;
}

static void vSyncInterrupt(void *refCon) {
	DreamWebEngine *vm = (DreamWebEngine *)refCon;

	if (!vm->isPaused()) {
		vm->setVSyncInterrupt(true);
	}
}

void DreamWebEngine::setVSyncInterrupt(bool flag) {
	_vSyncInterrupt = flag;
}

void DreamWebEngine::waitForVSync() {
	processEvents();

	if (!_turbo) {
		while (!_vSyncInterrupt) {
			_system->delayMillis(10);
		}
		setVSyncInterrupt(false);
	}

	doShake();
	doFade();
	_system->updateScreen();
}

void DreamWebEngine::quit() {
	_quitRequested = true;
	_lastHardKey = 1;
}

void DreamWebEngine::processEvents() {
	if (_eventMan->shouldQuit()) {
		quit();
		return;
	}

	_sound->soundHandler();
	Common::Event event;
	int softKey, hardKey;
	while (_eventMan->pollEvent(event)) {
		switch(event.type) {
		case Common::EVENT_RTL:
			quit();
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.flags & Common::KBD_CTRL) {
				switch (event.kbd.keycode) {

				case Common::KEYCODE_d:
					_console->attach();
					_console->onFrame();
					break;

				case Common::KEYCODE_f:
					setSpeed(_speed != 20? 20: 1);
					break;

				case Common::KEYCODE_g:
					_turbo = !_turbo;
					break;

				case Common::KEYCODE_c: //skip statue puzzle
					_symbolBotNum = 3;
					_symbolTopNum = 5;
					break;

				default:
					break;
				}

				return; //do not pass ctrl + key to the engine
			}

			// Some parts of the ASM code uses the hardware key
			// code directly. We don't have that code, so we fake
			// it for the keys where it's needed and assume it's
			// 0 (which is actually an invalid value, as far as I
			// know) otherwise.

			hardKey = 0;

			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				hardKey = 1;
				break;
			case Common::KEYCODE_SPACE:
				hardKey = 57;
				break;
			default:
				hardKey = 0;
				break;
			}

			_lastHardKey = hardKey;

			// The rest of the keys are converted to ASCII. This
			// is fairly restrictive, and eventually we may want
			// to let through more keys. I think this is mostly to
			// keep weird glyphs out of savegame names.

			softKey = 0;

			if (event.kbd.keycode >= Common::KEYCODE_a && event.kbd.keycode <= Common::KEYCODE_z) {
				softKey = event.kbd.ascii & ~0x20;
			} else if (event.kbd.keycode == Common::KEYCODE_MINUS ||
				event.kbd.keycode == Common::KEYCODE_SPACE ||
				(event.kbd.keycode >= Common::KEYCODE_0 && event.kbd.keycode <= Common::KEYCODE_9)) {
				softKey = event.kbd.ascii;
			} else if (event.kbd.keycode >= Common::KEYCODE_KP0 && event.kbd.keycode <= Common::KEYCODE_KP9) {
				softKey = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
			} else if (event.kbd.keycode == Common::KEYCODE_KP_MINUS) {
				softKey = '-';
			} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE ||
				event.kbd.keycode == Common::KEYCODE_DELETE) {
				softKey = 8;
			} else if (event.kbd.keycode == Common::KEYCODE_RETURN
				|| event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
				softKey = 13;
			}

			if (softKey)
				keyPressed(softKey);
			break;
		default:
			break;
		}
	}
}

Common::Error DreamWebEngine::run() {
	syncSoundSettings();
	_console = new DreamWebConsole(this);
	_sound = new DreamWebSound(this);

	ConfMan.registerDefault("originalsaveload", "false");
	ConfMan.registerDefault("bright_palette", true);
	_hasSpeech = Common::File::exists(_speechDirName + "/r01c0000.raw") && !ConfMan.getBool("speech_mute");
	_brightPalette = ConfMan.getBool("bright_palette");
	_copyProtection = ConfMan.getBool("copy_protection");

	_timer->installTimerProc(vSyncInterrupt, 1000000 / 70, this, "dreamwebVSync");
	dreamweb();
	dreamwebFinalize();
	_quitRequested = false;

	_timer->removeTimerProc(vSyncInterrupt);

	return Common::kNoError;
}

void DreamWebEngine::setSpeed(uint speed) {
	debug(0, "setting speed %u", speed);
	_speed = speed;
	_timer->removeTimerProc(vSyncInterrupt);
	_timer->installTimerProc(vSyncInterrupt, 1000000 / 70 / speed, this, "dreamwebVSync");
}

Common::String DreamWebEngine::getSavegameFilename(int slot) const {
	// TODO: Are saves from all versions of Dreamweb compatible with each other?
	// Then we can can consider keeping the filenames as DREAMWEB.Dnn.
	// Otherwise, this must be changed to be target dependent.
	//Common::String filename = _targetName + Common::String::format(".d%02d", savegameId);
	Common::String filename = Common::String::format("DREAMWEB.D%02d", slot);
	return filename;
}

void DreamWebEngine::keyPressed(uint16 ascii) {
	debug(2, "key pressed = %04x", ascii);
	uint16 in = (_bufferIn + 1) & 0x0f;
	uint16 out = _bufferOut;
	if (in == out) {
		warning("keyboard buffer is full");
		return;
	}
	_bufferIn = in;
	DreamWeb::g_keyBuffer[in] = ascii;
}

void DreamWebEngine::getPalette(uint8 *data, uint start, uint count) {
	_system->getPaletteManager()->grabPalette(data, start, count);
	while (count--)
		*data++ >>= 2;
}

void DreamWebEngine::setPalette(const uint8 *data, uint start, uint count) {
	assert(start + count <= 256);
	uint8 fixed[768];
	for (uint i = 0; i < count * 3; ++i) {
		fixed[i] = data[i] << 2;
	}
	_system->getPaletteManager()->setPalette(fixed, start, count);
}

void DreamWebEngine::blit(const uint8 *src, int pitch, int x, int y, int w, int h) {
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	if (h <= 0 || w <= 0)
		return;
	_system->copyRectToScreen(src, pitch, x, y, w, h);
}

void DreamWebEngine::printUnderMonitor() {
	uint8 *dst = workspace() + kScreenwidth * 43 + 76;

	Graphics::Surface *s = _system->lockScreen();
	if (!s)
		error("lockScreen failed");

	for (uint y = 0; y < 104; ++y) {
		uint8 *src = (uint8 *)s->getBasePtr(76, 43 + 8 + y);
		for (uint x = 0; x < 170; ++x) {
			if (*src < 231)
				*dst++ = *src++;
			else {
				++dst; ++src;
			}
		}
		dst += kScreenwidth - 170;
	}
	_system->unlockScreen();
}

void DreamWebEngine::cls() {
	_system->fillScreen(0);
}

uint8 DreamWebEngine::modifyChar(uint8 c) const {
	if (c < 128)
		return c;

	switch(getLanguage()) {
	case Common::DE_DEU:
		switch(c) {
		case 129:
			return 'Z' + 3;
		case 132:
			return 'Z' + 1;
		case 142:
			return 'Z' + 4;
		case 154:
			return 'Z' + 6;
		case 225:
			return 'A' - 1;
		case 153:
			return 'Z' + 5;
		case 148:
			return 'Z' + 2;
		default:
			return c;
		}
	case Common::ES_ESP:
		switch(c) {
		case 160:
			return 'Z' + 1;
		case 130:
			return 'Z' + 2;
		case 161:
			return 'Z' + 3;
		case 162:
			return 'Z' + 4;
		case 163:
			return 'Z' + 5;
		case 164:
			return 'Z' + 6;
		case 165:
			return ',' - 1;
		case 168:
			return 'A' - 1;
		case 173:
			return 'A' - 4;
		case 129:
			return 'A' - 5;
		default:
			return c;
		}
	case Common::FR_FRA:
	case Common::IT_ITA:
		switch(c) {
		case 133:
			return 'Z' + 1;
		case 130:
			return 'Z' + 2;
		case 138:
			return 'Z' + 3;
		case 136:
			return 'Z' + 4;
		case 140:
			return 'Z' + 5;
		case 135:
			return 'Z' + 6;
		case 149:
			return ',' - 1;
		case 131:
			return ',' - 2;
		case 141:
			return ',' - 3;
		case 139:
			return ',' - 4;
		case 151:
			return 'A' - 1;
		case 147:
			return 'A' - 3;
		case 150:
			return 'A' - 4;
		default:
			return c;
		}
	default:
		return c;
	}
}

Common::String DreamWebEngine::modifyFileName(const char *name) {
	Common::String fileName(name);

	// Sanity check
	if (!fileName.hasPrefix("DREAMWEB."))
		return fileName;

	// Make sure we use the correct file name as it differs depending on the game variant
	fileName = _datafilePrefix;
	fileName += name + 9;
	return fileName;
}

bool DreamWebEngine::hasSpeech() {
	return isCD() && _hasSpeech;
}

} // End of namespace DreamWeb
