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
#include "common/random.h"
#include "common/textconsole.h"

#include "agi/preagi.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"

namespace Agi {

PreAgiEngine::PreAgiEngine(OSystem *syst, const AGIGameDescription *gameDesc) : AgiBase(syst, gameDesc) {

	// Setup mixer
	syncSoundSettings();

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
}

void PreAgiEngine::initialize() {
	initRenderMode();

	_gfx = new GfxMgr(this);
	_picture = new PictureMgr(this, _gfx);

	_gfx->initMachine();

	_game.gameFlags = 0;

	_game.colorFg = 15;
	_game.colorBg = 0;

	_defaultColor = 0xF;

	_game.name[0] = '\0';

	_game.sbufOrig = (uint8 *)calloc(_WIDTH, _HEIGHT * 2); // Allocate space for two AGI screens vertically
	_game.sbuf16c  = _game.sbufOrig + SBUF16_OFFSET; // Make sbuf16c point to the 16 color (+control line & priority info) AGI screen
	_game.sbuf     = _game.sbuf16c; // Make sbuf point to the 16 color (+control line & priority info) AGI screen by default

	_game.lineMinPrint = 0; // hardcoded

	_gfx->initVideo();

	_speakerStream = new Audio::PCSpeaker(_mixer->getOutputRate());
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
							_speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	debugC(2, kDebugLevelMain, "Detect game");

	// clear all resources and events
	for (int i = 0; i < MAX_DIRS; i++) {
		memset(&_game.pictures[i], 0, sizeof(struct AgiPicture));
		memset(&_game.sounds[i], 0, sizeof(class AgiSound *)); // _game.sounds contains pointers now
		memset(&_game.dirPic[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirSound[i], 0, sizeof(struct AgiDir));
	}
}

PreAgiEngine::~PreAgiEngine() {
	_mixer->stopHandle(_speakerHandle);
	delete _speakerStream;
}

int PreAgiEngine::rnd(int hi) {
	return (_rnd->getRandomNumber(hi - 1) + 1);
}

// Screen functions
void PreAgiEngine::clearScreen(int attr, bool overrideDefault) {
	if (overrideDefault)
		_defaultColor = attr;

	_gfx->clearScreen((attr & 0xF0) / 0x10);
}

void PreAgiEngine::clearGfxScreen(int attr) {
	_gfx->drawRectangle(0, 0, GFX_WIDTH - 1, IDI_MAX_ROW_PIC * 8 -1, (attr & 0xF0) / 0x10);
}

// String functions

void PreAgiEngine::drawStr(int row, int col, int attr, const char *buffer) {
	int code;

	if (attr == kColorDefault)
		attr = _defaultColor;

	for (int iChar = 0; iChar < (int)strlen(buffer); iChar++) {
		code = buffer[iChar];

		switch (code) {
		case '\n':
		case 0x8D:
			if (++row == 200 / 8) return;
			col = 0;
			break;

		case '|':
			// swap attribute nibbles
			break;

		default:
			_gfx->putTextCharacter(1, col * 8 , row * 8, static_cast<char>(code), attr & 0x0f, (attr & 0xf0) / 0x10, false, getGameID() == GID_MICKEY ? mickey_fontdata : ibm_fontdata);

			if (++col == 320 / 8) {
				col = 0;
				if (++row == 200 / 8) return;
			}
		}
	}
}

void PreAgiEngine::drawStrMiddle(int row, int attr, const char *buffer) {
	int col = (25 / 2) - (strlen(buffer) / 2);	// 25 = 320 / 8 (maximum column)
	drawStr(row, col, attr, buffer);
}

void PreAgiEngine::clearTextArea() {
	int start = IDI_MAX_ROW_PIC;

	if (getGameID() == GID_TROLL)
		start = 21;

	for (int row = start; row < 200 / 8; row++) {
		clearRow(row);
	}
}

void PreAgiEngine::clearRow(int row) {
	drawStr(row, 0, IDA_DEFAULT, "                                        ");	// 40 spaces
}

void PreAgiEngine::printStr(const char* szMsg) {
	clearTextArea();
	drawStr(21, 0, IDA_DEFAULT, szMsg);
	_gfx->doUpdate();
}

void PreAgiEngine::XOR80(char *buffer) {
	for (size_t i = 0; i < strlen(buffer); i++)
		if (buffer[i] & 0x80)
			buffer[i] ^= 0x80;
}

void PreAgiEngine::printStrXOR(char *szMsg) {
	XOR80(szMsg);
	printStr(szMsg);
}

// Input functions

int PreAgiEngine::getSelection(SelectionTypes type) {
	Common::Event event;
	GUI::Debugger *console = getDebugger();

	while (!shouldQuit()) {
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
				return 0;
			case Common::EVENT_RBUTTONUP:
				return 0;
			case Common::EVENT_LBUTTONUP:
				if (type == kSelYesNo || type == kSelAnyKey)
					return 1;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_d && (event.kbd.flags & Common::KBD_CTRL) && console) {
					console->attach();
					console->onFrame();
					//FIXME: If not cleared, clicking again will start the console
					event.kbd.keycode = Common::KEYCODE_INVALID;
					event.kbd.flags = 0;
					continue;
				}
				switch (event.kbd.keycode) {
				case Common::KEYCODE_y:
					if (type == kSelYesNo)
						return 1;
				case Common::KEYCODE_n:
					if (type == kSelYesNo)
						return 0;
				case Common::KEYCODE_ESCAPE:
					if (type == kSelNumber || type == kSelAnyKey)
						return 0;
				case Common::KEYCODE_1:
				case Common::KEYCODE_2:
				case Common::KEYCODE_3:
				case Common::KEYCODE_4:
				case Common::KEYCODE_5:
				case Common::KEYCODE_6:
				case Common::KEYCODE_7:
				case Common::KEYCODE_8:
				case Common::KEYCODE_9:
					if (type == kSelNumber)
						return event.kbd.keycode - Common::KEYCODE_1 + 1;
				case Common::KEYCODE_SPACE:
					if (type == kSelSpace)
						return 1;
				case Common::KEYCODE_BACKSPACE:
					if (type == kSelBackspace)
						return 0;
				default:
					if (event.kbd.flags & Common::KBD_CTRL)
						break;
					if (type == kSelYesNo) {
						return 2;
					} else if (type == kSelNumber) {
						return 10;
					} else if (type == kSelAnyKey || type == kSelBackspace) {
						return 1;
					}
				}
				break;
			default:
				break;
			}
		}
		_system->updateScreen();
		_system->delayMillis(10);
	}
	return 0;
}

void PreAgiEngine::playNote(int16 frequency, int32 length) {
	_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, frequency, length);
	waitForTimer(length);
}

void PreAgiEngine::waitForTimer(int msec_delay) {
	uint32 start_time = _system->getMillis();

	while (_system->getMillis() < start_time + msec_delay) {
		_gfx->doUpdate();
		_system->delayMillis(10);
	}
}

} // End of namespace Agi
