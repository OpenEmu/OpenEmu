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

#include "agos/agos.h"
#include "agos/intern.h"

#include "graphics/surface.h"

namespace AGOS {

#ifdef ENABLE_AGOS2
void AGOSEngine_Feeble::doOutput(const byte *src, uint len) {
	if (_textWindow == NULL)
		return;

	while (len-- != 0 && !shouldQuit()) {
		if (getBitFlag(93)) {
			if (_curWindow == 3) {
				if ((_newLines >= _textWindow->scrollY) && (_newLines < (_textWindow->scrollY + 3)))
					sendWindow(*src);
				if (*src == '\n')		// Do two top lines of text only
					_newLines++;
				src++;
			}
		} else {
			if (getBitFlag(94)) {
				if (_curWindow == 3) {
					if (_newLines == (_textWindow->scrollY + 7))
						sendWindow(*src);
					if (*src == '\n')	// Do two top lines of text only
						_newLines++;
					src++;
				}
			} else {
				if (getBitFlag(92))
					delay(50);
				sendWindow(*src++);
			}
		}
	}
}
#endif

void AGOSEngine::doOutput(const byte *src, uint len) {
	uint idx;

	if (_textWindow == NULL)
		return;

	while (len-- != 0) {
		if (*src != 12 && _textWindow->iconPtr != NULL &&
				_fcsData1[idx = getWindowNum(_textWindow)] != 2) {

			_fcsData1[idx] = 2;
			_fcsData2[idx] = 1;
		}

		sendWindow(*src++);
	}
}

void AGOSEngine::clsCheck(WindowBlock *window) {
	uint index = getWindowNum(window);
	tidyIconArray(index);
	_fcsData1[index] = 0;
}

void AGOSEngine::tidyIconArray(uint i) {
	WindowBlock *window;

	if (_fcsData2[i]) {
		mouseOff();
		window = _windowArray[i];
		drawIconArray(i, window->iconPtr->itemRef, window->iconPtr->line, window->iconPtr->classMask);
		_fcsData2[i] = 0;
		mouseOn();
	}
}

void AGOSEngine::showMessageFormat(const char *s, ...) {
	char buf[STRINGBUFLEN];
	char *str;
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	if (!_fcsData1[_curWindow]) {
		if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
			if (_showMessageFlag) {
				if (_windowArray[_curWindow]->flags & 128) {
					haltAnimation();
				}
			}
		}
		openTextWindow();
		if (!_showMessageFlag) {
			_windowArray[0] = _textWindow;
			justifyStart();
		}
		_showMessageFlag = true;
		_fcsData1[_curWindow] = 1;
	}

	for (str = buf; *str; str++)
		justifyOutPut(*str);
}

void AGOSEngine::justifyStart() {
	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		_printCharCurPos = _textWindow->textColumn;
		_printCharMaxPos = _textWindow->width;
	} else {
		_printCharCurPos = _textWindow->textLength;
		_printCharMaxPos = _textWindow->textMaxLength;
	}
	_printCharPixelCount = 0;
	_numLettersToPrint = 0;
	_newLines = 0;
}

void AGOSEngine::justifyOutPut(byte chr) {
	if (chr == 12) {
		_numLettersToPrint = 0;
		_printCharCurPos = 0;
		_printCharPixelCount = 0;
		doOutput(&chr, 1);
		clsCheck(_textWindow);
	} else if (chr == 0 || chr == ' ' || chr == 10) {
		bool fit;

		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			fit = _printCharMaxPos - _printCharCurPos > _printCharPixelCount;
		} else {
			fit = _printCharMaxPos - _printCharCurPos >= _printCharPixelCount;
		}

		if (fit) {
			_printCharCurPos += _printCharPixelCount;
			doOutput(_lettersToPrintBuf, _numLettersToPrint);

			if (_printCharCurPos == _printCharMaxPos) {
				_printCharCurPos = 0;
			} else {
				if (chr)
					doOutput(&chr, 1);
				if (chr == 10)
					_printCharCurPos = 0;
				else if (chr != 0)
					_printCharCurPos += (getGameType() == GType_FF || getGameType() == GType_PP) ? getFeebleFontSize(chr) : 1;
			}
		} else {
			const byte newline_character = 10;
			_printCharCurPos = _printCharPixelCount;
			doOutput(&newline_character, 1);
			doOutput(_lettersToPrintBuf, _numLettersToPrint);
			if (chr == ' ') {
				doOutput(&chr, 1);
				_printCharCurPos += (getGameType() == GType_FF || getGameType() == GType_PP) ? getFeebleFontSize(chr) : 1;
			} else {
				doOutput(&chr, 1);
				_printCharCurPos = 0;
			}
		}
		_numLettersToPrint = 0;
		_printCharPixelCount = 0;
	} else {
		_lettersToPrintBuf[_numLettersToPrint++] = chr;
		_printCharPixelCount += (getGameType() == GType_FF || getGameType() == GType_PP) ? getFeebleFontSize(chr) : 1;
	}
}

void AGOSEngine::openTextWindow() {
	if (_textWindow) {
		if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
			if (_textWindow->flags & 0x80)
				clearWindow(_textWindow);
		}
		return;
	}

	if (getGameType() == GType_FF || getGameType() == GType_PP)
		_textWindow = openWindow(64, 96, 384, 172, 1, 0, 15);
	else
		_textWindow = openWindow(8, 144, 24, 6, 1, 0, 15);
}

void AGOSEngine_PN::windowPutChar(WindowBlock *window, byte c, byte b) {
	if (_mousePrintFG || _wiped)
		return;
	AGOSEngine::windowPutChar(window, c, b);
}

void AGOSEngine::windowPutChar(WindowBlock *window, byte c, byte b) {
	byte width = 6;

	if (c == 12) {
		clearWindow(window);
	} else if (c == 13 || c == 10) {
		windowNewLine(window);
	} else if ((c == 1 && _language != Common::HE_ISR) || (c == 8)) {
		if (_language == Common::HE_ISR) {
			if (b >= 64 && b < 91)
				width = _hebrewCharWidths [b - 64];

			if (window->textLength != 0) {
				window->textLength--;
				window->textColumnOffset += width;
				if (window->textColumnOffset >= 8) {
					window->textColumnOffset -= 8;
					window->textColumn--;
				}
			}
		} else {
			int8 val = (c == 8) ? 6 : 4;

			if (window->textLength != 0) {
				window->textLength--;
				window->textColumnOffset -= val;
				if ((int8)window->textColumnOffset < val) {
					window->textColumnOffset += 8;
					window->textColumn--;
				}
			}
		}
	} else if (c >= 32) {
		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			// Ignore invalid characters
			if (c - 32 > 195)
				return;

			windowDrawChar(window, window->textColumn + window->x, window->textRow + window->y, c);
			window->textColumn += getFeebleFontSize(c);
			return;
		}

		// Ignore invalid characters
		if (c - 32 > 98)
			return;

		if (window->textLength == window->textMaxLength) {
			windowNewLine(window);
		} else if (window->textRow == window->height) {
			windowNewLine(window);
			window->textRow--;
		}

		if (_language == Common::HE_ISR) {
			if (c >= 64 && c < 91)
				width = _hebrewCharWidths [c - 64];
			window->textColumnOffset -= width;
			if (window->textColumnOffset >= width) {
				window->textColumnOffset += 8;
				window->textColumn++;
			}
			windowDrawChar(window, (window->width + window->x - window->textColumn) * 8, window->textRow * 8 + window->y, c);
			window->textLength++;
		} else {
			windowDrawChar(window, (window->textColumn + window->x) * 8, window->textRow * 8 + window->y, c);

			window->textLength++;
			window->textColumnOffset += 6;
			if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
				if (c == 'i' || c == 'l')
					window->textColumnOffset -= 2;
			}
			if (window->textColumnOffset >= 8) {
				window->textColumnOffset -= 8;
				window->textColumn++;
			}
		}
	}
}

#ifdef ENABLE_AGOS2
void AGOSEngine_Feeble::windowNewLine(WindowBlock *window) {
	if (_noOracleScroll == 0) {
		if (window->height < window->textRow + 30) {
			if (!getBitFlag(94)) {
				_noOracleScroll = 1;
				if (getBitFlag(92)) {
					_noOracleScroll = 0;
					checkLinkBox();
					scrollOracle();
					linksUp();
					window->scrollY++;
					_oracleMaxScrollY++;
				} else {
					_oracleMaxScrollY++;
					checkLinkBox();
				}
			}
		} else {
			window->textRow += 15;
			checkLinkBox();
		}
	} else {
		_oracleMaxScrollY++;
		checkLinkBox();
	}

	window->textColumn = 0;
	window->textColumnOffset = 0;
	window->textLength = 0;
}
#endif

void AGOSEngine::windowNewLine(WindowBlock *window) {
	window->textColumn = 0;
	window->textColumnOffset = (getGameType() == GType_ELVIRA2) ? 4 : 0;
	window->textLength = 0;

	if (getGameType() == GType_PN) {
		window->textRow++;
		if (window->textRow == window->height) {
			windowScroll(window);
			window->textRow--;
		}
	} else {
		if (window->textRow == window->height) {
			if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 ||
				getGameType() == GType_WW) {
				windowScroll(window);
			}
		} else {
			window->textRow++;
		}
	}
}

void AGOSEngine::windowScroll(WindowBlock *window) {
	_videoLockOut |= 0x8000;

	if (window->height != 1) {
		Graphics::Surface *screen = _system->lockScreen();

		byte *src, *dst;
		uint16 w, h;

		w = window->width * 8;
		h = (window->height -1) * 8;

		dst = (byte *)screen->pixels + window->y * screen->pitch + window->x * 8;
		src = dst + 8 * screen->pitch;

		do {
			memcpy(dst, src, w);
			src += screen->pitch;
			dst += screen->pitch;
		} while (--h);

		_system->unlockScreen();
	}

	colorBlock(window, window->x * 8, (window->height - 1) * 8 + window->y, window->width * 8, 8);

	_videoLockOut &= ~0x8000;
}
} // End of namespace AGOS
