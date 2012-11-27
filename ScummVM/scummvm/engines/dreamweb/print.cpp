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

#include "dreamweb/sound.h"
#include "dreamweb/dreamweb.h"

namespace DreamWeb {

void DreamWebEngine::printBoth(const GraphicsFile &charSet, uint16 *x, uint16 y, uint8 c, uint8 nextChar) {
	uint16 newX = *x;
	uint8 width, height;
	printChar(charSet, &newX, y, c, nextChar, &width, &height);
	multiDump(*x, y, width, height);
	*x = newX;
}

uint8 DreamWebEngine::getNextWord(const GraphicsFile &charSet, const uint8 *string, uint8 *totalWidth, uint8 *charCount) {
	*totalWidth = 0;
	*charCount = 0;
	while (true) {
		uint8 firstChar = *string;
		++string;
		++*charCount;
		if ((firstChar == ':') || (firstChar == 0)) { //endall
			*totalWidth += 6;
			return 1;
		}
		if (firstChar == 32) { //endword
			*totalWidth += 6;
			return 0;
		}
		firstChar = modifyChar(firstChar);
		// WORKAROUND: Also filter out invalid characters here (refer to the
		// workaround in printChar() below for more info).
		if (firstChar >= 32 && firstChar != 255) {
			uint8 secondChar = *string;
			uint8 width = charSet._frames[firstChar - 32 + _charShift].width;
			width = kernChars(firstChar, secondChar, width);
			*totalWidth += width;
		}
	}
}

void DreamWebEngine::printChar(const GraphicsFile &charSet, uint16* x, uint16 y, uint8 c, uint8 nextChar, uint8 *width, uint8 *height) {
	// WORKAROUND: Some texts contain leftover tab characters, which will cause
	// OOB memory access when showing a character, as all the printable ones are
	// from 32 onwards. We compensate for that here by ignoring all the invalid
	// characters (0 - 31).
	if (c < 32 || c == 255)
		return;
	
	uint8 dummyWidth, dummyHeight;
	if (width == NULL)
		width = &dummyWidth;
	if (height == NULL)
		height = &dummyHeight;
	if (_foreignRelease)
		y -= 3;
	uint16 tmp = c - 32 + _charShift;
	showFrame(charSet, *x, y, tmp & 0x1ff, (tmp >> 8) & 0xfe, width, height);
	if (_kerning == 0)
		*width = kernChars(c, nextChar, *width);
	(*x) += *width;
}

void DreamWebEngine::printChar(const GraphicsFile &charSet, uint16 x, uint16 y, uint8 c, uint8 nextChar, uint8 *width, uint8 *height) {
	printChar(charSet, &x, y, c, nextChar, width, height);
}

uint8 DreamWebEngine::printSlow(const uint8 *string, uint16 x, uint16 y, uint8 maxWidth, bool centered) {
	_pointerFrame = 1;
	_pointerMode = 3;
	do {
		uint16 offset = x;
		uint16 charCount = getNumber(_charset1, string, maxWidth, centered, &offset);
		do {
			uint8 c0 = string[0];
			uint8 c1 = string[1];
			uint8 c2 = string[2];
			c0 = modifyChar(c0);
			printBoth(_charset1, &offset, y, c0, c1);
			if ((c1 == 0) || (c1 == ':')) {
				return 0;
			}
			if (charCount != 1) {
				c1 = modifyChar(c1);
				_charShift = 91;
				uint16 offset2 = offset;
				printBoth(_charset1, &offset2, y, c1, c2);
				_charShift = 0;
				for (int i=0; i<2; ++i) {
					uint16 mouseState = waitFrames();
					if (_quitRequested)
						return 0;
					if (mouseState == 0)
						continue;
					if (mouseState != _oldButton) {
						return 1;
					}
				}
			}

			++string;
			--charCount;
		} while (charCount);
		y += 10;
	} while (true);
}

uint8 DreamWebEngine::printDirect(const uint8* string, uint16 x, uint16 y, uint8 maxWidth, bool centered) {
	return printDirect(&string, x, &y, maxWidth, centered);
}

uint8 DreamWebEngine::printDirect(const uint8** string, uint16 x, uint16 *y, uint8 maxWidth, bool centered) {
	_lastXPos = x;
	const GraphicsFile &charSet = *_currentCharset;
	while (true) {
		uint16 offset = x;
		uint8 charCount = getNumber(charSet, *string, maxWidth, centered, &offset);
		uint16 i = offset;
		do {
			uint8 c = (*string)[0];
			uint8 nextChar = (*string)[1];
			++(*string);
			if ((c == 0) || (c == ':')) {
				return c;
			}
			c = modifyChar(c);
			uint8 width, height;
			printChar(charSet, &i, *y, c, nextChar, &width, &height);
			_lastXPos = i;
			--charCount;
		} while (charCount);
		*y += _lineSpacing;
	}
}

uint8 DreamWebEngine::getNumber(const GraphicsFile &charSet, const uint8 *string, uint16 maxWidth, bool centered, uint16* offset) {
	uint8 totalWidth = 0;
	uint8 charCount = 0;
	while (true) {
		uint8 wordTotalWidth, wordCharCount;
		uint8 done = getNextWord(charSet, string, &wordTotalWidth, &wordCharCount);
		string += wordCharCount;

		uint16 tmp = totalWidth + wordTotalWidth - 10;
		if (done == 1) { //endoftext
			if (tmp < maxWidth) {
				totalWidth += wordTotalWidth;
				charCount += wordCharCount;
			}

			if (centered) {
				tmp = (maxWidth & 0xfe) + 2 + 20 - totalWidth;
				tmp /= 2;
			} else {
				tmp = 0;
			}
			*offset += tmp;
			return charCount;
		}
		if (tmp >= maxWidth) { //gotoverend
			if (centered) {
				tmp = (maxWidth & 0xfe) - totalWidth + 20;
				tmp /= 2;
			} else {
				tmp = 0;
			}
			*offset += tmp;
			return charCount;
		}
		totalWidth += wordTotalWidth;
		charCount += wordCharCount;
	}
}

uint8 DreamWebEngine::kernChars(uint8 firstChar, uint8 secondChar, uint8 width) {
	if ((firstChar == 'a') || (firstChar == 'u')) {
		if ((secondChar == 'n') || (secondChar == 't') || (secondChar == 'r') || (secondChar == 'i') || (secondChar == 'l'))
			return width-1;
	}
	return width;
}

uint16 DreamWebEngine::waitFrames() {
	readMouse();
	showPointer();
	waitForVSync();
	dumpPointer();
	delPointer();
	return _mouseButton;
}

const char *DreamWebEngine::monPrint(const char *string) {
	_kerning = 1;
	uint16 x = _monAdX;
	const char *iterator = string;
	bool done = false;
	while (!done) {

		uint16 count = getNumber(_monitorCharset, (const uint8 *)iterator, 166, false, &x);
		do {
			char c = *iterator++;
			if (c == ':')
				break;
			if ((c == 0) || (c == '"') || (c == '=')) {
				done = true;
				break;
			}
			if (c == '%') {
				_vars._lastTrigger = *iterator;
				iterator += 2;
				done = true;
				break;
			}
			c = modifyChar(c);
			printChar(_monitorCharset, &x, _monAdY, c, 0, NULL, NULL);
			_cursLocX = x;
			_cursLocY = _monAdY;
			_mainTimer = 1;
			printCurs();
			waitForVSync();
			lockMon();
			delCurs();
		} while (--count);

		x = _monAdX;
		scrollMonitor();
		_cursLocX = _monAdX;
	}

	_kerning = 0;
	return iterator;
}

void DreamWebEngine::rollEndCreditsGameWon() {
	_sound->playChannel0(16, 255);
	_sound->volumeSet(7);
	_sound->volumeChange(0, -1);

	multiGet(_mapStore, 75, 20, 160, 160);

	const uint8 *string = getTextInFile1(3);
	const int linespacing = _lineSpacing;

	for (int i = 0; i < 254; ++i) {
		// Output the text, initially with an offset of 10 pixels,
		// then move it up one pixel until we shifted it by a complete
		// line of text.
		for (int j = 0; j < linespacing; ++j) {
			waitForVSync();
			multiPut(_mapStore, 75, 20, 160, 160);
			waitForVSync();

			// Output up to 18 lines of text
			uint16 y = 10 - j;
			const uint8 *tmp_str = string;
			for (int k = 0; k < 18; ++k) {
				printDirect(&tmp_str, 75, &y, 160 + 1, true);
				y += linespacing;
			}

			waitForVSync();
			multiDump(75, 20, 160, 160);
		}

		// Skip to the next text line
		byte c;
		do {
			c = *string++;
		} while (c != ':' && c != 0);
	}

	hangOn(100);
	panelToMap();
	fadeScreenUpHalf();
}

void DreamWebEngine::rollEndCreditsGameLost() {
	multiGet(_mapStore, 25, 20, 160, 160);

	const uint8 *string = getTextInFile1(49);
	const int linespacing = _lineSpacing;

	for (int i = 0; i < 80; ++i) {
		// Output the text, initially with an offset of 10 pixels,
		// then move it up one pixel until we shifted it by a complete
		// line of text.
		for (int j = 0; j < linespacing; ++j) {
			waitForVSync();
			multiPut(_mapStore, 25, 20, 160, 160);
			waitForVSync();

			// Output up to 18 lines of text
			uint16 y = 10 - j;
			const uint8 *tmp_str = string;
			for (int k = 0; k < 18; ++k) {
				printDirect(&tmp_str, 25, &y, 160 + 1, true);
				y += linespacing;
			}

			waitForVSync();
			multiDump(25, 20, 160, 160);

			if (_lastHardKey == 1)
				return;
		}

		// Skip to the next text line
		byte c;
		do {
			c = *string++;
		} while (c != ':' && c != 0);

		if (_lastHardKey == 1)
			return;
	}

	hangOne(120);
}

} // End of namespace DreamWeb
