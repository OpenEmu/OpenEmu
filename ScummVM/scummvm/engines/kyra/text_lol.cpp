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

#ifdef ENABLE_LOL

#include "kyra/text_lol.h"
#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/timer.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {

TextDisplayer_LoL::TextDisplayer_LoL(LoLEngine *engine, Screen_LoL *screenLoL) : TextDisplayer_rpg(engine, screenLoL),
	_vm(engine), _screen(screenLoL), _scriptTextParameter(0) {

	memset(_stringParameters, 0, 15 * sizeof(char *));
	_buffer = new char[600];
	memset(_buffer, 0, 600);

	_waitButtonSpace = 0;
}

TextDisplayer_LoL::~TextDisplayer_LoL() {
	delete[] _buffer;
}

void TextDisplayer_LoL::setupField(bool mode) {
	if (_vm->textEnabled()) {

		int y = 142;
		int h = 37;
		int stepY = 3;
		int stepH = 1;

		if (_vm->gameFlags().use16ColorMode) {
			y = 140;
			h = 39;
			stepY = 4;
			stepH = 2;
		}

		if (mode) {
			_screen->copyRegionToBuffer(3, 0, 0, 320, 40, _vm->_pageBuffer1);
			_screen->copyRegion(80, y, 0, 0, 240, h, 0, 3, Screen::CR_NO_P_CHECK);
			_screen->copyRegionToBuffer(3, 0, 0, 320, 40, _vm->_pageBuffer2);
			_screen->copyBlockToPage(3, 0, 0, 320, 40, _vm->_pageBuffer1);
		} else {
			_screen->setScreenDim(clearDim(4));
			int cp = _screen->setCurPage(2);
			_screen->copyRegionToBuffer(3, 0, 0, 320, 40, _vm->_pageBuffer1);
			_screen->copyBlockToPage(3, 0, 0, 320, 40, _vm->_pageBuffer2);
			_screen->copyRegion(0, 0, 80, y, 240, h, 3, _screen->_curPage, Screen::CR_NO_P_CHECK);

			for (int i = 177; i > 141; i--) {
				uint32 endTime = _vm->_system->getMillis() + _vm->_tickLength;
				_screen->copyRegion(83, i - stepH + 1, 83, i - stepH, 235, stepY, 0, 0, Screen::CR_NO_P_CHECK);
				_screen->copyRegion(83, i + 1, 83, i + 1, 235, 1, 2, 0, Screen::CR_NO_P_CHECK);
				_vm->updateInput();
				_screen->updateScreen();
				_vm->delayUntil(endTime);
			}

			_screen->copyBlockToPage(3, 0, 0, 320, 200, _vm->_pageBuffer1);
			_screen->setCurPage(cp);

			_vm->_updateFlags &= 0xfffd;
		}
	} else {
		if (!mode)
			_screen->setScreenDim(clearDim(4));
		_vm->toggleSelectedCharacterFrame(1);
	}
}

void TextDisplayer_LoL::expandField() {
	uint8 *tmp = _vm->_pageBuffer1 + 13000;

	if (_vm->textEnabled()) {
		_vm->_fadeText = false;
		_vm->_textColorFlag = 0;
		_vm->_timer->disable(11);
		_screen->setScreenDim(clearDim(3));
		_screen->copyRegionToBuffer(3, 0, 0, 320, 10, tmp);

		int y = 140;
		int h = 3;
		int stepH = 0;

		if (_vm->gameFlags().use16ColorMode) {
			y = 139;
			h = 4;
			stepH = 1;
		}

		_screen->copyRegion(83, y, 0, 0, 235, h, 0, 2, Screen::CR_NO_P_CHECK);

		for (int i = 140; i < 177; i++) {
			uint32 endTime = _vm->_system->getMillis() + _vm->_tickLength;
			_screen->copyRegion(0, 0, 83, i - stepH, 235, h, 2, 0, Screen::CR_NO_P_CHECK);
			_vm->updateInput();
			_screen->updateScreen();
			_vm->delayUntil(endTime);
		}

		_screen->copyBlockToPage(3, 0, 0, 320, 10, tmp);
		_vm->_updateFlags |= 2;

	} else {
		clearDim(3);
		_vm->toggleSelectedCharacterFrame(0);
	}
}

void TextDisplayer_LoL::printDialogueText(int dim, char *str, EMCState *script, const uint16 *paramList, int16 paramIndex) {
	int oldDim = 0;

	const bool isPc98 = (_vm->gameFlags().platform == Common::kPlatformPC98);

	if (dim == 3) {
		if (_vm->_updateFlags & 2) {
			oldDim = clearDim(4);
			_textDimData[4].color1 = isPc98 ? 0x33 : 254;
			_textDimData[4].color2 = _screen->_curDim->unkA;
		} else {
			oldDim = clearDim(3);
			_textDimData[3].color1 = isPc98 ? 0x33 : 192;
			_textDimData[3].color2 = _screen->_curDim->unkA;
			if (!isPc98)
				_screen->copyColor(192, 254);
			_vm->enableTimer(11);
			_vm->_textColorFlag = 0;
			_vm->_fadeText = false;
		}
	} else {
		oldDim = _screen->curDimIndex();
		_screen->setScreenDim(dim);
		_lineCount = 0;
		_textDimData[dim].color1 = isPc98 ? 0x33 : 254;
		_textDimData[dim].color2 = _screen->_curDim->unkA;
	}

	int cp = _screen->setCurPage(0);
	Screen::FontId of = _screen->setFont(_vm->gameFlags().use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_9_FNT);

	preprocessString(str, script, paramList, paramIndex);
	_numCharsTotal = strlen(_dialogueBuffer);
	displayText(_dialogueBuffer);

	_screen->setScreenDim(oldDim);
	_screen->setCurPage(cp);
	_screen->setFont(of);
	_lineCount = 0;
	_vm->_fadeText = false;
}

void TextDisplayer_LoL::printMessage(uint16 type, const char *str, ...) {
	static const uint8 textColors256[] = { 0xfe, 0xa2, 0x84, 0x97, 0x9F };
	static const uint8 textColors16[] = { 0x33, 0xaa, 0x88, 0x55, 0x99 };
	static const uint8 soundEffect[] = { 0x0B, 0x00, 0x2B, 0x1B, 0x00 };

	const uint8 *textColors = _vm->gameFlags().use16ColorMode ? textColors16 : textColors256;

	if (type & 4)
		type ^= 4;
	else
		_vm->stopPortraitSpeechAnim();

	uint16 col = textColors[type & 0x7fff];

	int od = _screen->curDimIndex();

	if (_vm->_updateFlags & 2) {
		clearDim(4);
		_textDimData[4].color1 = col;
	} else {
		clearDim(3);
		if (_vm->gameFlags().use16ColorMode) {
			_textDimData[3].color1 = col;
		} else {
			_screen->copyColor(192, col);
			_textDimData[3].color1 = 192;
		}
		_vm->enableTimer(11);
	}

	va_list args;
	va_start(args, str);

	vsnprintf((char *)_buffer, 240, str, args);

	va_end(args);

	displayText(_buffer);

	_screen->setScreenDim(od);
	_lineCount = 0;

	if (!(type & 0x8000)) {
		if (soundEffect[type])
			_vm->sound()->playSoundEffect(soundEffect[type]);
	}

	_vm->_textColorFlag = type & 0x7fff;
	_vm->_fadeText = false;
}

void TextDisplayer_LoL::preprocessString(char *str, EMCState *script, const uint16 *paramList, int16 paramIndex) {
	char *dst = _dialogueBuffer;
	const bool isPc98 = (_vm->gameFlags().platform == Common::kPlatformPC98);

	for (char *s = str; *s;) {
		if (isPc98) {
			uint8 c = *s;
			if (c >= 0xE0 || (c > 0x80 && c < 0xA0)) {
				*dst++ = *s++;
				*dst++ = *s++;
				continue;
			}
		}

		if (*s != '%') {
			*dst++ = *s++;
			continue;
		}

		char para = *++s;
		bool eos = false;

		switch (para) {
		case '\0':
			eos = true;
			break;
		case '#':
			para = *++s;
			switch (para) {
			case 'E':
			case 'G':
			case 'X':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 's':
			case 'u':
			case 'x':
				break;
			default:
				eos = true;
			}
			break;
		case ' ':
		case '+':
		case '-':
			++s;
		default:
			break;
		}

		if (eos)
			continue;

		para = *s;

		switch (para) {
		case '\0':
			eos = true;
			break;
		case '0':
			++s;
			break;
		default:
			while (para && para > 47 && para < 58)
				para = *++s;
			break;
		}
		if (eos)
			continue;

		para = *s++;

		switch (para) {
		case 'a':
			strcpy(dst, Common::String::format("%d", _scriptTextParameter).c_str());
			dst += strlen(dst);
			break;

		case 'n':
			strcpy(dst, _vm->_characters[script ? script->stack[script->sp + paramIndex] : paramList[paramIndex]].name);
			dst += strlen(dst);
			break;

		case 's':
			strcpy(dst, _vm->getLangString(script ? script->stack[script->sp + paramIndex] : paramList[paramIndex]));
			dst += strlen(dst);
			break;

		case 'X':
		case 'd':
		case 'u':
		case 'x':
			strcpy(dst, Common::String::format("%d", script ? script->stack[script->sp + paramIndex] : paramList[paramIndex]).c_str());
			dst += strlen(dst);
			break;

		case '\0':
		default:
			continue;
		}
	}
	*dst = 0;
}

KyraRpgEngine *TextDisplayer_LoL::vm() {
	return _vm;
}

Screen *TextDisplayer_LoL::screen() {
	return _screen;
}

void TextDisplayer_LoL::textPageBreak() {
	strcpy(_pageBreakString, _vm->getLangString(0x4073));
	TextDisplayer_rpg::textPageBreak();
}

} // End of namespace Kyra

#endif // ENABLE_LOL
