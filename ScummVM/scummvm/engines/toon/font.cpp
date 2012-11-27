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

#include "common/debug.h"
#include "common/rect.h"

#include "toon/font.h"

namespace Toon {

FontRenderer::FontRenderer(ToonEngine *vm) : _vm(vm) {
	_currentFontColor[0] = 0;
	_currentFontColor[1] = 0xc8;
	_currentFontColor[2] = 0xcb;
	_currentFontColor[3] = 0xce;
}

FontRenderer::~FontRenderer() {
}

// mapping extended characters required for foreign versions to font (animation)
static const byte map_textToFont[0x80] = {
	 '?',  '?',  '?',  '?', 0x03,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0x8x
	 '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0x9x
	 '?', 0x09,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0xAx
	 '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', 0x0a, // 0xBx
	 '?',  '?',  '?',  '?', 0x1d,  '?',  '?', 0x02,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0xCx
	 '?', 0x0b,  '?',  '?',  '?',  '?', 0x1e,  '?',  '?',  '?',  '?', 0x20, 0x1f,  '?',  '?', 0x19, // 0xDx
	0x0d, 0x04, 0x0e,  '?', 0x1a,  '?',  '?', 0x18, 0x10, 0x0f, 0x12, 0x11, 0x09, 0x05, 0x14, 0x13, // 0xEx
	0x23, 0x08, 0x23, 0x06, 0x15, 0x23, 0x1b, 0x23, 0x23, 0x16, 0x07, 0x17, 0x1c, 0x23, 0x23, 0x23  // 0xFx
};

byte FontRenderer::textToFont(byte c) {
	// No need to remap simple characters.
	if (c < 0x80)
		return c;

	// The Spanish version shows grave accent over the 'e' when it should
	// be acute. This happens both in the original interpreter and when
	// using the common map which works for other languages, so we add a
	// special case for it.
	if (_vm->_language == Common::ES_ESP && c == 0xe9)
		return 0x10;

	// Use the common map to convert the extended characters.
	return map_textToFont[c - 0x80];
}

void FontRenderer::renderText(int16 x, int16 y, const Common::String &origText, int32 mode) {
	debugC(5, kDebugFont, "renderText(%d, %d, %s, %d)", x, y, origText.c_str(), mode);

	int16 xx, yy;
	computeSize(origText, &xx, &yy);

	if (mode & 2) {
		y -= yy / 2;
	} else if (mode & 4) {
		y -= yy;
	}

	if (mode & 1) {
		x -= xx / 2;
	}

	_vm->addDirtyRect(x, y, x + xx, y + yy);

	int16 curX = x;
	int16 curY = y;
	int32 height = 0;

	const byte *text = (const byte *)origText.c_str();
	while (*text) {
		byte curChar = *text;
		if (curChar == 13) {
			curY = curY + height;
			height = 0;
			curX = x;
		} else {
			curChar = textToFont(curChar);
			_currentFont->drawFontFrame(_vm->getMainSurface(), curChar, curX, curY, _currentFontColor);
			curX = curX + _currentFont->getFrameWidth(curChar) - 1;
			height = MAX<int32>(height, _currentFont->getFrameHeight(curChar));
		}
		text++;
	}
}

void FontRenderer::computeSize(const Common::String &origText, int16 *retX, int16 *retY) {
	debugC(4, kDebugFont, "computeSize(%s, retX, retY)", origText.c_str());

	int16 lineWidth = 0;
	int16 lineHeight = 0;
	int16 totalHeight = 0;
	int16 totalWidth = 0;
	int16 lastLineHeight = 0;

	const byte *text = (const byte *)origText.c_str();
	while (*text) {
		byte curChar = *text;
		if (curChar < 32) {
			text++;
			continue;
		} else if (curChar == 13) {
			totalWidth = MAX(totalWidth, lineWidth);
			totalHeight += lineHeight;
			lineHeight = 0;
			lineWidth = 0;
			lastLineHeight = 0;
		} else {
			curChar = textToFont(curChar);
			int16 charWidth = _currentFont->getFrameWidth(curChar) - 1;
			int16 charHeight = _currentFont->getFrameHeight(curChar);
			lineWidth += charWidth;
			lineHeight = MAX(lineHeight, charHeight);

			// The character may be offset, so the height doesn't
			// really tell how far it will stick out. For now,
			// assume we only need to take the lower bound into
			// consideration.
			Common::Rect charRect = _currentFont->getFrameRect(curChar);
			lastLineHeight = MAX(lastLineHeight, charRect.bottom);
		}
		text++;
	}

	totalHeight += lastLineHeight;
	totalWidth = MAX(totalWidth, lineWidth);

	*retX = totalWidth;
	*retY = totalHeight;
}

void FontRenderer::setFont(Animation *font) {
	debugC(5, kDebugFont, "setFont(font)");

	_currentFont = font;
}

void FontRenderer::setFontColorByCharacter(int32 characterId) {
	debugC(5, kDebugFont, "setFontColorByCharacter(%d)", characterId);

	// unfortunately this table was hardcoded in the original executable
	static const byte colorsByCharacters[] = {
		0xe0, 0xdc, 0xc8,   0xd6, 0xc1, 0xc8,   0xe9, 0xde, 0xc8,   0xeb, 0xe8, 0xc8,
		0xd1, 0xcf, 0xc8,   0xdb, 0xd5, 0xc8,   0xfb, 0xfa, 0xc8,   0xd9, 0xd7, 0xc8,
		0xe8, 0xe4, 0xc8,   0xe9, 0xfa, 0xc8,   0xeb, 0xe4, 0xc8,   0xeb, 0xe4, 0xc8,
		0xd2, 0xea, 0xc8,   0xd3, 0xd0, 0xc8,   0xe1, 0xdd, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd2, 0xcf, 0xc8,   0xd1, 0xcf, 0xc8,   0xd9, 0xd7, 0xc8,   0xe3, 0xdd, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd9, 0xd7, 0xc8,   0xe6, 0xe4, 0xc8,   0xd9, 0xd7, 0xc8,   0xcd, 0xca, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xeb, 0xe8, 0xc8,   0xdb, 0xd5, 0xc8,
		0xe0, 0xdc, 0xc8,   0xd6, 0xc1, 0xc8,   0xd3, 0xd0, 0xc8,   0xd1, 0xcf, 0xc8,
		0xe6, 0xe4, 0xc8,   0xd1, 0xcf, 0xc8,   0xd2, 0xcf, 0xc8,   0xcc, 0xcb, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8
	};

	setFontColor(colorsByCharacters[characterId * 3 + 2], colorsByCharacters[characterId * 3 + 1], colorsByCharacters[characterId * 3 + 0]);
}

void FontRenderer::setFontColor(int32 fontColor1, int32 fontColor2, int32 fontColor3) {
	debugC(5, kDebugFont, "setFontColor(%d, %d, %d)", fontColor1, fontColor2, fontColor3);

	_currentFontColor[0] = 0;
	_currentFontColor[1] = fontColor1;
	_currentFontColor[2] = fontColor2;
	_currentFontColor[3] = fontColor3;
}

void FontRenderer::renderMultiLineText(int16 x, int16 y, const Common::String &origText, int32 mode) {
	debugC(5, kDebugFont, "renderMultiLineText(%d, %d, %s, %d)", x, y, origText.c_str(), mode);

	// divide the text in several lines
	// based on number of characters or size of lines.
	byte text[1024];
	strncpy((char *)text, origText.c_str(), 1023);
	text[1023] = 0;

	byte *lines[16];
	int32 lineSize[16];
	int32 numLines = 0;

	byte *it = text;

	int16 maxWidth = 0;
	int16 curWidth = 0;

	while (true) {
		byte *lastLine = it;
		byte *lastSpace = it;
		int32 lastSpaceX = 0;
		int32 curLetterNr = 0;
		curWidth = 0;

		while (*it && curLetterNr < 50 && curWidth < 580) {
			byte curChar = *it;
			if (curChar == 32) {
				lastSpace = it;
				lastSpaceX = curWidth;
			} else
				curChar = textToFont(curChar);

			int width = _currentFont->getFrameWidth(curChar);
			curWidth += MAX(width - 2, 0);
			it++;
			curLetterNr++;
		}

		if (*lastLine == 0)
			break;

		lines[numLines] = lastLine;

		if (*it == 0)
			lineSize[numLines] = curWidth;
		else
			lineSize[numLines] = lastSpaceX;

		if (lineSize[numLines] > maxWidth)
			maxWidth = lineSize[numLines];

		lastLine = lastSpace + 1;
		numLines++;

		if (*it == 0)
			break;

		it = lastLine;
		*lastSpace = 0;

		if (numLines >= 16)
			break;
	}

	if (curWidth > maxWidth) {
		maxWidth = curWidth;
	}
	//numLines++;

	// get font height (assumed to be constant)
	int16 height = _currentFont->getHeight();
	int32 textSize = (height - 2) * numLines;
	y = y - textSize;
	if (y < 30)
		y = 30;
	if (y + textSize > 370)
		y = 370 - textSize;

	x -= _vm->state()->_currentScrollValue;

	// adapt x
	if (x - 30 - maxWidth / 2 < 0)
		x = maxWidth / 2 + 30;

	if (x + 30 + (maxWidth / 2) > TOON_SCREEN_WIDTH)
		x = TOON_SCREEN_WIDTH - (maxWidth / 2) - 30;

	// we have good coordinates now, we can render the multi line
	int16 curX = x;
	int16 curY = y;

	for (int32 i = 0; i < numLines; i++) {
		const byte *line = lines[i];
		curX = x - lineSize[i] / 2;
		_vm->addDirtyRect(curX + _vm->state()->_currentScrollValue, curY, curX + lineSize[i] + _vm->state()->_currentScrollValue + 2, curY + height);

		while (*line) {
			byte curChar = textToFont(*line);
			if (curChar != 32) _currentFont->drawFontFrame(_vm->getMainSurface(), curChar, curX + _vm->state()->_currentScrollValue, curY, _currentFontColor);
			curX = curX + MAX<int32>(_currentFont->getFrameWidth(curChar) - 2, 0);
			//height = MAX(height, _currentFont->getFrameHeight(curChar));
			line++;
		}
		curY += height;
	}
}

} // End of namespace Toon
