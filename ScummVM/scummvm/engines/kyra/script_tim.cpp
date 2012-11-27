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

#include "kyra/script_tim.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

#ifdef ENABLE_LOL
#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#endif // ENABLE_LOL

#include "common/iff_container.h"
#include "common/system.h"

namespace Kyra {

TIMInterpreter::TIMInterpreter(KyraEngine_v1 *engine, Screen_v2 *screen_v2, OSystem *system) : _vm(engine), _screen(screen_v2), _system(system), _currentTim(0) {
#define COMMAND(x) { &TIMInterpreter::x, #x }
#define COMMAND_UNIMPL() { 0, 0 }
#define cmd_return(n) cmd_return_##n
	static const CommandEntry commandProcs[] = {
		// 0x00
		COMMAND(cmd_initFunc0),
		COMMAND(cmd_stopCurFunc),
		COMMAND(cmd_initWSA),
		COMMAND(cmd_uninitWSA),
		// 0x04
		COMMAND(cmd_initFunc),
		COMMAND(cmd_stopFunc),
		COMMAND(cmd_wsaDisplayFrame),
		COMMAND(cmd_displayText),
		// 0x08
		COMMAND(cmd_loadVocFile),
		COMMAND(cmd_unloadVocFile),
		COMMAND(cmd_playVocFile),
		COMMAND_UNIMPL(),
		// 0x0C
		COMMAND(cmd_loadSoundFile),
		COMMAND(cmd_return(1)),
		COMMAND(cmd_playMusicTrack),
		COMMAND_UNIMPL(),
		// 0x10
		COMMAND(cmd_return(1)),
		COMMAND(cmd_return(1)),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		// 0x14
		COMMAND(cmd_setLoopIp),
		COMMAND(cmd_continueLoop),
		COMMAND(cmd_resetLoopIp),
		COMMAND(cmd_resetAllRuntimes),
		// 0x18
		COMMAND(cmd_return(1)),
		COMMAND(cmd_execOpcode),
		COMMAND(cmd_initFuncNow),
		COMMAND(cmd_stopFuncNow),
		// 0x1C
		COMMAND(cmd_return(1)),
		COMMAND(cmd_return(1)),
		COMMAND(cmd_return(n1))
	};
#undef cmd_return
#undef COMMAND_UNIMPL
#undef COMMAND

	_commands = commandProcs;
	_commandsSize = ARRAYSIZE(commandProcs);

	_langData = 0;
	_textDisplayed = false;
	_textAreaBuffer = new uint8[320*40];
	assert(_textAreaBuffer);
	if ((_vm->gameFlags().platform == Common::kPlatformPC98 || _vm->gameFlags().isDemo) && _vm->game() == GI_LOL)
		_drawPage2 = 0;
	else
		_drawPage2 = 8;

	_animator = new TimAnimator(0, screen_v2, 0, false);

	_palDelayInc = _palDiff = _palDelayAcc = 0;
	_abortFlag = 0;
	_tim = 0;
}

TIMInterpreter::~TIMInterpreter() {
	delete[] _langData;
	delete[] _textAreaBuffer;
	delete _animator;
}

bool TIMInterpreter::callback(Common::IFFChunk &chunk) {
	switch (chunk._type) {
	case MKTAG('T','E','X','T'):
		_tim->text = new byte[chunk._size];
		assert(_tim->text);
		if (chunk._stream->read(_tim->text, chunk._size) != chunk._size)
			error("Couldn't read TEXT chunk from file '%s'", _filename);
		break;

	case MKTAG('A','V','T','L'):
		_avtlChunkSize = chunk._size >> 1;
		_tim->avtl = new uint16[_avtlChunkSize];
		assert(_tim->avtl);
		if (chunk._stream->read(_tim->avtl, chunk._size) != chunk._size)
			error("Couldn't read AVTL chunk from file '%s'", _filename);

		for (int i = _avtlChunkSize - 1; i >= 0; --i)
			_tim->avtl[i] = READ_LE_UINT16(&_tim->avtl[i]);
		break;

	default:
		warning("Unexpected chunk '%s' of size %d found in file '%s'", tag2str(chunk._type), chunk._size, _filename);
	}

	return false;
}

TIM *TIMInterpreter::load(const char *filename, const Common::Array<const TIMOpcode *> *opcodes) {
	if (!_vm->resource()->exists(filename))
		return 0;

	Common::SeekableReadStream *stream = _vm->resource()->createReadStream(filename);
	if (!stream)
		error("Couldn't open TIM file '%s'", filename);

	_avtlChunkSize = 0;
	_filename = filename;

	_tim = new TIM;
	assert(_tim);
	memset(_tim, 0, sizeof(TIM));

	_tim->procFunc = -1;
	_tim->opcodes = opcodes;

	IFFParser iff(*stream);
	Common::Functor1Mem<Common::IFFChunk &, bool, TIMInterpreter> c(this, &TIMInterpreter::callback);
	iff.parse(c);

	if (!_tim->avtl)
		error("No AVTL chunk found in file: '%s'", filename);

	if (stream->err())
		error("Read error while parsing file '%s'", filename);

	delete stream;

	const int num = (_avtlChunkSize < TIM::kCountFuncs) ? _avtlChunkSize : (int)TIM::kCountFuncs;
	for (int i = 0; i < num; ++i)
		_tim->func[i].avtl = _tim->avtl + _tim->avtl[i];

	Common::strlcpy(_tim->filename, filename, 13);

	_tim->isLoLOutro = (_vm->game() == GI_LOL) && !scumm_stricmp(filename, "LOLFINAL.TIM");
	_tim->lolCharacter = 0;

	TIM *r = _tim;
	_tim = 0;
	return r;
}

void TIMInterpreter::unload(TIM *&tim) const {
	if (!tim)
		return;

	delete[] tim->text;
	delete[] tim->avtl;
	delete tim;
	tim = 0;
}

void TIMInterpreter::setLangData(const char *filename) {
	delete[] _langData;
	_langData = _vm->resource()->fileData(filename, 0);
}

int TIMInterpreter::exec(TIM *tim, bool loop) {
	if (!tim)
		return 0;

	_currentTim = tim;
	if (!_currentTim->func[0].ip) {
		_currentTim->func[0].ip = _currentTim->func[0].avtl;
		_currentTim->func[0].nextTime = _currentTim->func[0].lastTime = _system->getMillis();
	}

	do {
		update();

		for (_currentFunc = 0; _currentFunc < TIM::kCountFuncs; ++_currentFunc) {
			TIM::Function &cur = _currentTim->func[_currentFunc];

			if (_currentTim->procFunc != -1)
				execCommand(28, &_currentTim->procParam);

			update();
			checkSpeechProgress();

			bool running = true;
			int cnt = 0;
			while (cur.ip && cur.nextTime <= _system->getMillis() && running) {
				if (cnt++ > 0) {
					if (_currentTim->procFunc != -1)
						execCommand(28, &_currentTim->procParam);
					update();
				}

				int8 opcode = int8(cur.ip[2] & 0xFF);

				switch (execCommand(opcode, cur.ip + 3)) {
				case -1:
					loop = false;
					running = false;
					_currentFunc = 11;
					break;

				case -2:
					running = false;
					break;

				case -3:
					_currentTim->procFunc = _currentFunc;
					_currentTim->dlgFunc = -1;
					break;

				case 22:
					cur.loopIp = 0;
					break;

				default:
					break;
				}

				if (cur.ip) {
					cur.ip += cur.ip[0];
					cur.lastTime = cur.nextTime;
					cur.nextTime += cur.ip[1] * _vm->tickLength();
				}
			}
		}
	} while (loop && !_vm->shouldQuit());

	return _currentTim->clickedButton;
}

void TIMInterpreter::refreshTimersAfterPause(uint32 elapsedTime) {
	if (!_currentTim)
		return;

	for (int i = 0; i < TIM::kCountFuncs; i++) {
		if (_currentTim->func[i].lastTime)
			_currentTim->func[i].lastTime += elapsedTime;
		if (_currentTim->func[i].nextTime)
			_currentTim->func[i].nextTime += elapsedTime;
	}
}

void TIMInterpreter::displayText(uint16 textId, int16 flags) {
	char *text = getTableEntry(textId);

	if (_textDisplayed) {
		_screen->copyBlockToPage(0, 0, 160, 320, 40, _textAreaBuffer);
		_textDisplayed = false;
	}

	if (!text)
		return;
	if (!text[0])
		return;

	char filename[16];
	memset(filename, 0, sizeof(filename));

	if (text[0] == '$') {
		const char *end = strchr(text+1, '$');
		if (end)
			memcpy(filename, text+1, end-1-text);
	}

	const bool isPC98 = (_vm->gameFlags().platform == Common::kPlatformPC98);
	if (filename[0] && (_vm->speechEnabled() || !_vm->gameFlags().isTalkie))
		_vm->sound()->voicePlay(filename, 0, 255, 255, !_vm->gameFlags().isTalkie);

	if (text[0] == '$')
		text = strchr(text + 1, '$') + 1;

	if (!isPC98)
		setupTextPalette((flags < 0) ? 1 : flags, 0);

	if (flags < 0) {
		static const uint8 colorMap[] = { 0x00, 0xF0, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

		_screen->setFont(isPC98 ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);
		_screen->setTextColorMap(colorMap);
		_screen->_charWidth = -2;
	}

	_screen->_charOffset = -4;
	_screen->copyRegionToBuffer(0, 0, 160, 320, 40, _textAreaBuffer);
	_textDisplayed = true;

	char backupChar = 0;
	char *str = text;
	int heightAdd = 0;

	while (str[0] && _vm->textEnabled()) {
		char *nextLine = strchr(str, '\r');

		backupChar = 0;
		if (nextLine) {
			backupChar = nextLine[0];
			nextLine[0] = '\0';
		}

		int width = _screen->getTextWidth(str);

		if (flags >= 0) {
			if (isPC98) {
				static const uint8 colorMap[] = { 0xE1, 0xE1, 0xC1, 0xA1, 0x81, 0x61 };
				_screen->printText(str, (320 - width) >> 1, 160 + heightAdd, colorMap[flags], 0x00);
			} else {
				_screen->printText(str, (320 - width) >> 1, 160 + heightAdd, 0xF0, 0x00);
			}
		} else {
			_screen->printText(str, (320 - width) >> 1, 188, 0xF0, 0x00);
		}

		heightAdd += _screen->getFontHeight();
		str += strlen(str);

		if (backupChar) {
			nextLine[0] = backupChar;
			++str;
		}
	}

	_screen->_charOffset = 0;

	if (flags < 0) {
		static const uint8 colorMap[] = { 0x00, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0x00, 0x00, 0x00, 0x00 };

		_screen->setFont(isPC98 ? Screen::FID_SJIS_FNT : Screen::FID_INTRO_FNT);
		_screen->setTextColorMap(colorMap);
		_screen->_charWidth = 0;
	}
}

void TIMInterpreter::displayText(uint16 textId, int16 flags, uint8 color) {
	if (!_vm->textEnabled() && !(textId & 0x8000))
		return;

	char *text = getTableEntry(textId & 0x7FFF);

	if (flags > 0)
		_screen->copyBlockToPage(0, 0, 0, 320, 40, _textAreaBuffer);

	if (flags == 255)
		return;

	_screen->setFont(_vm->gameFlags().use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_INTRO_FNT);

	static const uint8 colorMap[] = { 0x00, 0xA0, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	_screen->setTextColorMap(colorMap);
	_screen->_charWidth = 0;
	if (!_vm->gameFlags().use16ColorMode)
		_screen->_charOffset = -4;

	if (!flags)
		_screen->copyRegionToBuffer(0, 0, 0, 320, 40, _textAreaBuffer);

	char backupChar = 0;
	char *str = text;
	int y = 0;

	if (_vm->gameFlags().use16ColorMode) {
		if (color == 0xda)
			color = 0xa1;
		else if (color == 0xf2)
			color = 0xe1;
		else if (flags < 0)
			color = 0xe1;
		else
			color = 0xc1;
	}

	while (str[0]) {
		char *nextLine = strchr(str, '\r');

		backupChar = 0;
		if (nextLine) {
			backupChar = nextLine[0];
			nextLine[0] = '\0';
		}

		int width = _screen->getTextWidth(str);

		if (flags >= 0)
			_screen->printText(str, (320 - width) >> 1, y, color, 0x00);
		else
			_screen->printText(str, 0, y, color, 0x00);

		y += (_vm->gameFlags().use16ColorMode ? 16 : (_screen->getFontHeight() - 4));
		str += strlen(str);

		if (backupChar) {
			nextLine[0] = backupChar;
			++str;
		}
	}
}

void TIMInterpreter::setupTextPalette(uint index, int fadePalette) {
	static const uint16 palTable[] = {
		0x00, 0x00, 0x00,
		0x64, 0x64, 0x64,
		0x61, 0x51, 0x30,
		0x29, 0x48, 0x64,
		0x00, 0x4B, 0x3B,
		0x64, 0x1E, 0x1E,
	};

	for (int i = 0; i < 15; ++i) {
		uint8 *palette = _screen->getPalette(0).getData() + (240 + i) * 3;

		uint8 c1 = (((15 - i) << 2) * palTable[index*3+0]) / 100;
		uint8 c2 = (((15 - i) << 2) * palTable[index*3+1]) / 100;
		uint8 c3 = (((15 - i) << 2) * palTable[index*3+2]) / 100;

		palette[0] = c1;
		palette[1] = c2;
		palette[2] = c3;
	}

	if (!fadePalette && !_palDiff) {
		_screen->setScreenPalette(_screen->getPalette(0));
	} else {
		_screen->getFadeParams(_screen->getPalette(0), fadePalette, _palDelayInc, _palDiff);
		_palDelayAcc = 0;
	}
}

int TIMInterpreter::initAnimStruct(int index, const char *filename, int x, int y, int, int offscreenBuffer, uint16 wsaFlags) {
	Movie *wsa = 0;

	const bool isLoLDemo = _vm->gameFlags().isDemo && _vm->game() == GI_LOL;

	if (isLoLDemo || _vm->gameFlags().platform == Common::kPlatformPC98 || _currentTim->isLoLOutro)
		_drawPage2 = 0;
	else
		_drawPage2 = 8;

	uint16 wsaOpenFlags = 0;
	if (isLoLDemo) {
		if (!(wsaFlags & 0x10))
			wsaOpenFlags |= 1;
	} else {
		if (wsaFlags & 0x10)
			wsaOpenFlags |= 2;
		wsaOpenFlags |= 1;

		if (offscreenBuffer == 2)
			wsaOpenFlags = 1;
	}

	Common::String file = Common::String::format("%s.WSA", filename);

	if (_vm->resource()->exists(file.c_str())) {
		if (isLoLDemo)
			wsa = new WSAMovie_v1(_vm);
		else
			wsa = new WSAMovie_v2(_vm);
		assert(wsa);

		wsa->open(file.c_str(), wsaOpenFlags, (index == 1) ? &_screen->getPalette(0) : 0);
	}

	if (wsa && wsa->opened()) {
		if (isLoLDemo) {
			if (x == -1) {
				int16 t = int8(320 - wsa->width());
				uint8 v = int8(t & 0x00FF) - int8((t & 0xFF00) >> 8);
				v >>= 1;
				x = v;
			}

			if (y == -1) {
				int16 t = int8(200 - wsa->height());
				uint8 v = int8(t & 0x00FF) - int8((t & 0xFF00) >> 8);
				v >>= 1;
				y = v;
			}
		} else {
			if (x == -1)
				x = 0;
			if (y == -1)
				y = 0;
		}

		if (wsaFlags & 2) {
			_screen->fadePalette(_screen->getPalette(1), 15, 0);
			_screen->clearPage(_drawPage2);
			if (_drawPage2)
				_screen->checkedPageUpdate(8, 4);
			_screen->updateScreen();
		}

		if (wsaFlags & 4) {
			file = Common::String::format("%s.CPS", filename);

			if (_vm->resource()->exists(file.c_str())) {
				_screen->loadBitmap(file.c_str(), 3, 3, &_screen->getPalette(0));
				_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, _drawPage2, Screen::CR_NO_P_CHECK);
				if (_drawPage2)
					_screen->checkedPageUpdate(8, 4);
				_screen->updateScreen();
			}

			wsa->displayFrame(0, 0, x, y, 0, 0, 0);
		}

		if (wsaFlags & 2)
			_screen->fadePalette(_screen->getPalette(0), 30, 0);
	} else {
		if (wsaFlags & 2) {
			_screen->fadePalette(_screen->getPalette(1), 15, 0);
			_screen->clearPage(_drawPage2);
			if (_drawPage2)
				_screen->checkedPageUpdate(8, 4);
			_screen->updateScreen();
		}

		file = Common::String::format("%s.CPS", filename);

		if (_vm->resource()->exists(file.c_str())) {
			_screen->loadBitmap(file.c_str(), 3, 3, &_screen->getPalette(0));
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, _drawPage2, Screen::CR_NO_P_CHECK);
			if (_drawPage2)
				_screen->checkedPageUpdate(8, 4);
			_screen->updateScreen();
		}

		if (wsaFlags & 2)
			_screen->fadePalette(_screen->getPalette(0), 30, 0);
	}

	_animator->init(index, wsa, x, y, wsaFlags, 0);

	return index + 1;
}

int TIMInterpreter::freeAnimStruct(int index) {
	_animator->reset(index, true);
	return 1;
}

char *TIMInterpreter::getTableEntry(uint idx) {
	if (!_langData)
		return 0;
	else
		return (char *)(_langData + READ_LE_UINT16(_langData + (idx<<1)));
}

const char *TIMInterpreter::getCTableEntry(uint idx) const {
	if (!_langData)
		return 0;
	else
		return (const char *)(_langData + READ_LE_UINT16(_langData + (idx<<1)));
}

int TIMInterpreter::execCommand(int cmd, const uint16 *param) {
	if (cmd < 0 || cmd >= _commandsSize) {
		warning("Calling unimplemented TIM command %d from file '%s'", cmd, _currentTim->filename);
		return 0;
	}

	if (_commands[cmd].proc == 0) {
		warning("Calling unimplemented TIM command %d from file '%s'", cmd, _currentTim->filename);
		return 0;
	}

	debugC(5, kDebugLevelScript, "TIMInterpreter::%s(%p)", _commands[cmd].desc, (const void *)param);
	return (this->*_commands[cmd].proc)(param);
}

int TIMInterpreter::cmd_initFunc0(const uint16 *param) {
	for (int i = 0; i < TIM::kWSASlots; ++i)
		memset(&_currentTim->wsa[i], 0, sizeof(TIM::WSASlot));

	_currentTim->func[0].ip = _currentTim->func[0].avtl;
	_currentTim->func[0].lastTime = _system->getMillis();
	return 1;
}

int TIMInterpreter::cmd_stopCurFunc(const uint16 *param) {
	if (_currentFunc < TIM::kCountFuncs)
		_currentTim->func[_currentFunc].ip = 0;
	if (!_currentFunc)
		_finished = true;
	return -2;
}

void TIMInterpreter::stopAllFuncs(TIM *tim) {
	for (int i = 0; i < TIM::kCountFuncs; ++i)
		tim->func[i].ip = 0;
}

int TIMInterpreter::cmd_initWSA(const uint16 *param) {
	const int index = param[0];

	TIM::WSASlot &slot = _currentTim->wsa[index];

	slot.x = int16(param[2]);
	slot.y = int16(param[3]);
	slot.offscreen = param[4];
	slot.wsaFlags = param[5];
	const char *filename = (const char *)(_currentTim->text + READ_LE_UINT16(_currentTim->text + (param[1]<<1)));

	slot.anim = initAnimStruct(index, filename, slot.x, slot.y, 10, slot.offscreen, slot.wsaFlags);
	return 1;
}

int TIMInterpreter::cmd_uninitWSA(const uint16 *param) {
	const int index = param[0];

	TIM::WSASlot &slot = _currentTim->wsa[index];

	if (!slot.anim)
		return 0;

	if (slot.offscreen) {
		_animator->reset(index, false);
		slot.anim = 0;
	} else {
		//XXX
		_animator->reset(index, true);
		memset(&slot, 0, sizeof(TIM::WSASlot));
	}

	return 1;
}

int TIMInterpreter::cmd_initFunc(const uint16 *param) {
	uint16 func = *param;
	assert(func < TIM::kCountFuncs);
	if (_currentTim->func[func].avtl)
		_currentTim->func[func].ip = _currentTim->func[func].avtl;
	else
		_currentTim->func[func].avtl = _currentTim->func[func].ip = _currentTim->avtl + _currentTim->avtl[func];
	return 1;
}

int TIMInterpreter::cmd_stopFunc(const uint16 *param) {
	uint16 func = *param;
	assert(func < TIM::kCountFuncs);
	_currentTim->func[func].ip = 0;
	return 1;
}

int TIMInterpreter::cmd_wsaDisplayFrame(const uint16 *param) {
	_animator->displayFrame(param[0], _drawPage2, param[1]);
	return 1;
}

int TIMInterpreter::cmd_displayText(const uint16 *param) {
	if (_currentTim->isLoLOutro)
		displayText(param[0], param[1], 0xF2);
	else
		displayText(param[0], param[1]);
	return 1;
}

int TIMInterpreter::cmd_loadVocFile(const uint16 *param) {
	const int stringId = param[0];
	const int index = param[1];

	_vocFiles[index] = (const char *)(_currentTim->text + READ_LE_UINT16(_currentTim->text + (stringId << 1)));

	if (index == 2 && _currentTim->isLoLOutro && _vm->gameFlags().isTalkie) {
		_vocFiles[index] = "CONGRATA.VOC";

		switch (_currentTim->lolCharacter) {
		case 0:
			_vocFiles[index].setChar('K', 7);
			break;

		case 1:
			_vocFiles[index].setChar('A', 7);
			break;

		case 2:
			_vocFiles[index].setChar('M', 7);
			break;

		case 3:
			_vocFiles[index].setChar('C', 7);
			break;

		default:
			break;
		}
	}

	for (int i = 0; i < 4; ++i)
		_vocFiles[index].deleteLastChar();
	return 1;
}

int TIMInterpreter::cmd_unloadVocFile(const uint16 *param) {
	const int index = param[0];
	_vocFiles[index].clear();
	return 1;
}

int TIMInterpreter::cmd_playVocFile(const uint16 *param) {
	const int index = param[0];
	const int volume = (param[1] * 255) / 100;

	if (index < ARRAYSIZE(_vocFiles) && !_vocFiles[index].empty())
		_vm->sound()->voicePlay(_vocFiles[index].c_str(), 0, volume, 255, true);
	else if (index == 7 && !_vm->gameFlags().isTalkie)
		_vm->sound()->playTrack(index);
	else
		_vm->sound()->playSoundEffect(index);

	return 1;
}

int TIMInterpreter::cmd_loadSoundFile(const uint16 *param) {
	const char *file = (const char *)(_currentTim->text + READ_LE_UINT16(_currentTim->text + (param[0]<<1)));

	_vm->sound()->loadSoundFile(file);
	if (_vm->game() == GI_LOL)
		_vm->sound()->loadSfxFile(file);

	return 1;
}

int TIMInterpreter::cmd_playMusicTrack(const uint16 *param) {
	_vm->sound()->playTrack(param[0]);
	return 1;
}

int TIMInterpreter::cmd_setLoopIp(const uint16 *param) {
	_currentTim->func[_currentFunc].loopIp = _currentTim->func[_currentFunc].ip;
	return 1;
}

int TIMInterpreter::cmd_continueLoop(const uint16 *param) {
	TIM::Function &func = _currentTim->func[_currentFunc];

	if (!func.loopIp)
		return -2;

	func.ip = func.loopIp;

	uint16 factor = param[0];
	if (factor) {
		const uint32 random = _vm->_rnd.getRandomNumberRng(0, 0x8000);
		uint32 waitTime = (random * factor) / 0x8000;
		func.nextTime += waitTime * _vm->tickLength();
	}

	return -2;
}

int TIMInterpreter::cmd_resetLoopIp(const uint16 *param) {
	_currentTim->func[_currentFunc].loopIp = 0;
	return 1;
}

int TIMInterpreter::cmd_resetAllRuntimes(const uint16 *param) {
	for (int i = 0; i < TIM::kCountFuncs; ++i) {
		if (_currentTim->func[i].ip)
			_currentTim->func[i].nextTime = _system->getMillis();
	}
	return 1;
}

int TIMInterpreter::cmd_execOpcode(const uint16 *param) {
	const uint16 opcode = *param++;

	if (!_currentTim->opcodes) {
		warning("Trying to execute TIM opcode %d without opcode list (file '%s')", opcode, _currentTim->filename);
		return 0;
	}

	if (opcode > _currentTim->opcodes->size()) {
		warning("Calling unimplemented TIM opcode(0x%.02X/%d) from file '%s'", opcode, opcode, _currentTim->filename);
		return 0;
	}

	if (!(*_currentTim->opcodes)[opcode]->isValid()) {
		warning("Calling unimplemented TIM opcode(0x%.02X/%d) from file '%s'", opcode, opcode, _currentTim->filename);
		return 0;
	}

	return (*(*_currentTim->opcodes)[opcode])(_currentTim, param);
}

int TIMInterpreter::cmd_initFuncNow(const uint16 *param) {
	uint16 func = *param;
	assert(func < TIM::kCountFuncs);
	_currentTim->func[func].ip = _currentTim->func[func].avtl;
	_currentTim->func[func].lastTime = _currentTim->func[func].nextTime = _system->getMillis();
	return 1;
}

int TIMInterpreter::cmd_stopFuncNow(const uint16 *param) {
	uint16 func = *param;
	assert(func < TIM::kCountFuncs);
	_currentTim->func[func].ip = 0;
	_currentTim->func[func].lastTime = _currentTim->func[func].nextTime = _system->getMillis();
	return 1;
}

// TODO: Consider moving to another file

#ifdef ENABLE_LOL
// LOL version of the TIM interpreter

TIMInterpreter_LoL::TIMInterpreter_LoL(LoLEngine *engine, Screen_v2 *screen_v2, OSystem *system) :
	TIMInterpreter(engine, screen_v2, system), _vm(engine)  {
	#define COMMAND(x) { &TIMInterpreter_LoL::x, #x }
	#define COMMAND_UNIMPL() { 0, 0 }
	#define cmd_return(n) cmd_return_##n
	static const CommandEntry commandProcs[] = {
		// 0x00
		COMMAND(cmd_initFunc0),
		COMMAND(cmd_stopAllFuncs),
		COMMAND(cmd_initWSA),
		COMMAND(cmd_uninitWSA),
		// 0x04
		COMMAND(cmd_initFunc),
		COMMAND(cmd_stopFunc),
		COMMAND(cmd_wsaDisplayFrame),
		COMMAND_UNIMPL(),
		// 0x08
		COMMAND(cmd_loadVocFile),
		COMMAND(cmd_unloadVocFile),
		COMMAND(cmd_playVocFile),
		COMMAND_UNIMPL(),
		// 0x0C
		COMMAND(cmd_loadSoundFile),
		COMMAND(cmd_return(1)),
		COMMAND(cmd_playMusicTrack),
		COMMAND_UNIMPL(),
		// 0x10
		COMMAND(cmd_return(1)),
		COMMAND(cmd_return(1)),
		COMMAND_UNIMPL(),
		COMMAND_UNIMPL(),
		// 0x14
		COMMAND(cmd_setLoopIp),
		COMMAND(cmd_continueLoop),
		COMMAND(cmd_resetLoopIp),
		COMMAND(cmd_resetAllRuntimes),
		// 0x18
		COMMAND(cmd_return(1)),
		COMMAND(cmd_execOpcode),
		COMMAND(cmd_initFuncNow),
		COMMAND(cmd_stopFuncNow),
		// 0x1C
		COMMAND(cmd_processDialogue),
		COMMAND(cmd_dialogueBox),
		COMMAND(cmd_return(n1))
	};
	#undef cmd_return
	#undef COMMAND_UNIMPL
	#undef COMMAND

	_commands = commandProcs;
	_commandsSize = ARRAYSIZE(commandProcs);

	_screen = engine->_screen;

	delete _animator;
	_animator = new TimAnimator(engine, screen_v2, system, true);

	_drawPage2 = 0;
}

int TIMInterpreter_LoL::initAnimStruct(int index, const char *filename, int x, int y, int frameDelay, int, uint16 wsaFlags) {
	Movie *wsa = 0;
	uint16 wsaOpenFlags = 0;
	if (wsaFlags & 0x10)
		wsaOpenFlags |= 2;
	if (wsaFlags & 8)
		wsaOpenFlags |= 1;

	Common::String file = Common::String::format("%s.WSA", filename);

	if (_vm->resource()->exists(file.c_str())) {
		wsa = new WSAMovie_v2(_vm);
		assert(wsa);
		wsa->open(file.c_str(), wsaOpenFlags, &_screen->getPalette(3));
	}

	if (!_vm->_flags.use16ColorMode) {
		if (wsaFlags & 1) {
			if (_screen->_fadeFlag != 1)
				_screen->fadeClearSceneWindow(10);
			_screen->getPalette(3).copy(_screen->getPalette(0), 128, 128);
		} else if (wsaFlags & 2) {
			_screen->fadeToBlack(10);
		}
	}

	if (wsa && (wsaFlags & 7))
		wsa->displayFrame(0, 0, x, y, 0, 0, 0);

	if (wsaFlags & 3) {
		if (_vm->_flags.use16ColorMode) {
			_vm->setPaletteBrightness(_screen->getPalette(0), _vm->_brightness, _vm->_lampEffect);
		} else {
			_screen->loadSpecialColors(_screen->getPalette(3));
			_screen->fadePalette(_screen->getPalette(3), 10);
		}
		_screen->_fadeFlag = 0;
	}

	_animator->init(index, wsa, x, y, wsaFlags, frameDelay);

	return index + 1;
}

int TIMInterpreter_LoL::freeAnimStruct(int index) {
	_animator->reset(index, true);
	return 1;
}

void TIMInterpreter_LoL::advanceToOpcode(int opcode) {
	TIM::Function *f = &_currentTim->func[_currentTim->dlgFunc];
	uint16 len = f->ip[0];

	while ((f->ip[2] & 0xFF) != opcode) {
		if ((f->ip[2] & 0xFF) == 1) {
			f->ip[0] = len;
			break;
		}
		len = f->ip[0];
		f->ip += len;
	}

	f->nextTime = _system->getMillis();
}

void TIMInterpreter_LoL::resetDialogueState(TIM *tim) {
	if (!tim)
		return;

	tim->procFunc = 0;
	tim->procParam = _vm->_dialogueNumButtons ? _vm->_dialogueNumButtons : 1;
	tim->clickedButton = 0;
	tim->dlgFunc = -1;
}

void TIMInterpreter_LoL::update() {
	_vm->update();
}

void TIMInterpreter_LoL::checkSpeechProgress() {
	if (_vm->speechEnabled() && _currentTim->procParam > 1 && _currentTim->func[_currentFunc].loopIp) {
		if (_vm->snd_updateCharacterSpeech() != 2) {
			_currentTim->func[_currentFunc].loopIp = 0;
			_currentTim->dlgFunc = _currentFunc;
			advanceToOpcode(21);
			_currentTim->dlgFunc = -1;
			_animator->reset(5, false);
		}
	}
}

char *TIMInterpreter_LoL::getTableString(int id) {
	return _vm->getLangString(id);
}

int TIMInterpreter_LoL::execCommand(int cmd, const uint16 *param) {
	if (cmd < 0 || cmd >= _commandsSize) {
		warning("Calling unimplemented TIM command %d from file '%s'", cmd, _currentTim->filename);
		return 0;
	}

	if (_commands[cmd].proc == 0) {
		warning("Calling unimplemented TIM command %d from file '%s'", cmd, _currentTim->filename);
		return 0;
	}

	debugC(5, kDebugLevelScript, "TIMInterpreter::%s(%p)", _commands[cmd].desc, (const void *)param);
	return (this->*_commands[cmd].proc)(param);
}

int TIMInterpreter_LoL::cmd_stopAllFuncs(const uint16 *param) {
	while (_currentTim->dlgFunc == -1 && _currentTim->clickedButton == 0 && !_vm->shouldQuit()) {
		update();
		_currentTim->clickedButton = _vm->processDialogue();
	}

	for (int i = 0; i < TIM::kCountFuncs; ++i)
		_currentTim->func[i].ip = 0;

	return -1;
}

int TIMInterpreter_LoL::cmd_setLoopIp(const uint16 *param) {
	if (_vm->speechEnabled()) {
		if (_vm->snd_updateCharacterSpeech() == 2)
			_currentTim->func[_currentFunc].loopIp = _currentTim->func[_currentFunc].ip;
		else
			advanceToOpcode(21);
	} else {
		_currentTim->func[_currentFunc].loopIp = _currentTim->func[_currentFunc].ip;
	}
	return 1;
}

int TIMInterpreter_LoL::cmd_continueLoop(const uint16 *param) {
	TIM::Function &func = _currentTim->func[_currentFunc];

	if (!func.loopIp)
		return -2;

	func.ip = func.loopIp;

	if (_vm->snd_updateCharacterSpeech() != 2) {
		uint16 factor = param[0];
		if (factor) {
			const uint32 random = _vm->_rnd.getRandomNumberRng(0, 0x8000);
			uint32 waitTime = (random * factor) / 0x8000;
			func.nextTime += waitTime * _vm->tickLength();
		}
	}

	return -2;
}

int TIMInterpreter_LoL::cmd_processDialogue(const uint16 *param) {
	int res = _vm->processDialogue();
	if (!res || !_currentTim->procParam)
		return res;

	_vm->snd_stopSpeech(false);

	_currentTim->func[_currentTim->procFunc].loopIp = 0;
	_currentTim->dlgFunc = _currentTim->procFunc;
	_currentTim->procFunc = -1;
	_currentTim->clickedButton = res;

	_animator->reset(5, false);

	if (_currentTim->procParam)
		advanceToOpcode(21);

	return res;
}

int TIMInterpreter_LoL::cmd_dialogueBox(const uint16 *param) {
	uint16 func = *param;
	assert(func < TIM::kCountFuncs);
	_currentTim->procParam = func;
	_currentTim->clickedButton = 0;

	const char *tmpStr[3];
	int cnt = 0;

	for (int i = 1; i < 4; i++) {
		if (param[i] != 0xffff) {
			tmpStr[i-1] = getTableString(param[i]);
			cnt++;
		} else {
			tmpStr[i-1] = 0;
		}
	}

	_vm->setupDialogueButtons(cnt, tmpStr[0], tmpStr[1], tmpStr[2]);
	_vm->gui_notifyButtonListChanged();

	return -3;
}
#endif // ENABLE_LOL

} // End of namespace Kyra
