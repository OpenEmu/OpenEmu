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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/font.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text16.h"

namespace Sci {

GfxText16::GfxText16(ResourceManager *resMan, GfxCache *cache, GfxPorts *ports, GfxPaint16 *paint16, GfxScreen *screen)
	: _resMan(resMan), _cache(cache), _ports(ports), _paint16(paint16), _screen(screen) {
	init();
}

GfxText16::~GfxText16() {
}

void GfxText16::init() {
	_font = NULL;
	_codeFonts = NULL;
	_codeFontsCount = 0;
	_codeColors = NULL;
	_codeColorsCount = 0;
}

GuiResourceId GfxText16::GetFontId() {
	return _ports->_curPort->fontId;
}

GfxFont *GfxText16::GetFont() {
	if ((_font == NULL) || (_font->getResourceId() != _ports->_curPort->fontId))
		_font = _cache->getFont(_ports->_curPort->fontId);

	return _font;
}

void GfxText16::SetFont(GuiResourceId fontId) {
	if ((_font == NULL) || (_font->getResourceId() != fontId))
		_font = _cache->getFont(fontId);

	_ports->_curPort->fontId = _font->getResourceId();
	_ports->_curPort->fontHeight = _font->getHeight();
}

void GfxText16::ClearChar(int16 chr) {
	if (_ports->_curPort->penMode != 1)
		return;
	Common::Rect rect;
	rect.top = _ports->_curPort->curTop;
	rect.bottom = rect.top + _ports->_curPort->fontHeight;
	rect.left = _ports->_curPort->curLeft;
	rect.right = rect.left + GetFont()->getCharWidth(chr);
	_paint16->eraseRect(rect);
}

// This internal function gets called as soon as a '|' is found in a text. It
// will process the encountered code and set new font/set color. We only support
// one-digit codes currently, don't know if multi-digit codes are possible.
// Returns textcode character count.
int16 GfxText16::CodeProcessing(const char *&text, GuiResourceId orgFontId, int16 orgPenColor, bool doingDrawing) {
	const char *textCode = text;
	int16 textCodeSize = 0;
	char curCode;
	signed char curCodeParm;

	// Find the end of the textcode
	while ((++textCodeSize) && (*text != 0) && (*text++ != 0x7C)) { }

	// possible TextCodes:
	//  c -> sets textColor to current port pen color
	//  cX -> sets textColor to _textColors[X-1]
	curCode = textCode[0];
	curCodeParm = textCode[1];
	if (Common::isDigit(curCodeParm)) {
		curCodeParm -= '0';
	} else {
		curCodeParm = -1;
	}
	switch (curCode) {
	case 'c': // set text color
		if (curCodeParm == -1) {
			_ports->_curPort->penClr = orgPenColor;
		} else {
			if (curCodeParm < _codeColorsCount) {
				_ports->_curPort->penClr = _codeColors[curCodeParm];
			}
		}
		break;
	case 'f': // set text font
		if (curCodeParm == -1) {
			SetFont(orgFontId);
		} else {
			if (curCodeParm < _codeFontsCount) {
				SetFont(_codeFonts[curCodeParm]);
			}
		}
		break;
	case 'r': // reference (used in pepper)
		if (doingDrawing) {
			if (_codeRefTempRect.top == -1) {
				// Starting point
				_codeRefTempRect.top = _ports->_curPort->curTop;
				_codeRefTempRect.left = _ports->_curPort->curLeft;
			} else {
				// End point reached
				_codeRefTempRect.bottom = _ports->_curPort->curTop + _ports->_curPort->fontHeight;
				_codeRefTempRect.right = _ports->_curPort->curLeft;
				_codeRefRects.push_back(_codeRefTempRect);
				_codeRefTempRect.left = _codeRefTempRect.top = -1;
			}
		}
		break;
	}
	return textCodeSize;
}

static const uint16 text16_punctuationSjis[] = {
	0x9F82, 0xA182, 0xA382, 0xA582, 0xA782, 0xC182, 0xA782, 0xC182, 0xE182, 0xE382, 0xE582, 0xEC82,
	0x4083, 0x4283, 0x4483, 0x4683, 0x4883, 0x6283, 0x8383, 0x8583, 0x8783, 0x8E83, 0x9583, 0x9683,
	0x5B81, 0x4181, 0x4281, 0x7681, 0x7881, 0x4981, 0x4881, 0
};

// return max # of chars to fit maxwidth with full words, does not include
// breaking space
int16 GfxText16::GetLongest(const char *text, int16 maxWidth, GuiResourceId orgFontId) {
	uint16 curChar = 0;
	int16 maxChars = 0, curCharCount = 0;
	uint16 width = 0;
	GuiResourceId previousFontId = GetFontId();
	int16 previousPenColor = _ports->_curPort->penClr;

	GetFont();
	if (!_font)
		return 0;

	while (width <= maxWidth) {
		curChar = (*(const byte *)text++);
		if (_font->isDoubleByte(curChar)) {
			curChar |= (*(const byte *)text++) << 8;
			curCharCount++;
		}
		switch (curChar) {
		case 0x7C:
			if (getSciVersion() >= SCI_VERSION_1_1) {
				curCharCount++;
				curCharCount += CodeProcessing(text, orgFontId, previousPenColor, false);
				continue;
			}
			break;

		// We need to add 0xD, 0xA and 0xD 0xA to curCharCount and then exit
		//  which means, we split text like
		//  'Mature, experienced software analyst available.' 0xD 0xA
		//  'Bug installation a proven speciality. "No version too clean."' (normal game text, this is from lsl2)
		//   and 0xA '-------' 0xA (which is the official sierra subtitle separator)
		//  Sierra did it the same way.
		case 0xD:
			// Check, if 0xA is following, if so include it as well
			if ((*(const unsigned char *)text) == 0xA)
				curCharCount++;
			// it's meant to pass through here
		case 0xA:
		case 0x9781: // this one is used by SQ4/japanese as line break as well
			curCharCount++;
			// and it's also meant to pass through here
		case 0:
			SetFont(previousFontId);
			_ports->penColor(previousPenColor);
			return curCharCount;

		case ' ':
			maxChars = curCharCount; // return count up to (but not including) breaking space
			break;
		}
		// Sometimes this can go off the screen, like for example bug #3040161.
		// However, we only perform this for non-Japanese games, as these require
		// special handling, done after this loop.
		if (width + _font->getCharWidth(curChar) > maxWidth && g_sci->getLanguage() != Common::JA_JPN)
			break;
		width += _font->getCharWidth(curChar);
		curCharCount++;
	}

	// Text without spaces, probably Kanji/Japanese
	if (maxChars == 0) {
		maxChars = curCharCount;

		uint16 nextChar;

		// We remove the last char only, if maxWidth was actually equal width
		// before adding the last char. Otherwise we won't get the same cutting
		// as in sierra pc98 sci.
		if (maxWidth == (width - _font->getCharWidth(curChar))) {
			maxChars--;
			if (curChar > 0xFF)
				maxChars--;
			nextChar = curChar;
		} else {
			nextChar = (*(const byte *)text++);
			if (_font->isDoubleByte(nextChar))
				nextChar |= (*(const byte *)text++) << 8;
		}
		// sierra checked the following character against a punctuation kanji table
		if (nextChar > 0xFF) {
			// if the character is punctuation, we go back one character
			uint nonBreakingNr = 0;
			while (text16_punctuationSjis[nonBreakingNr]) {
				if (text16_punctuationSjis[nonBreakingNr] == nextChar) {
					maxChars--;
					if (curChar > 0xFF)
						maxChars--; // go back 2 chars, when last char was double byte
					break;
				}
				nonBreakingNr++;
			}
		}
	}
	SetFont(previousFontId);
	_ports->penColor(previousPenColor);
	return maxChars;
}

void GfxText16::Width(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight, bool restoreFont) {
	uint16 curChar;
	GuiResourceId previousFontId = GetFontId();
	int16 previousPenColor = _ports->_curPort->penClr;

	textWidth = 0; textHeight = 0;

	GetFont();
	if (_font) {
		text += from;
		while (len--) {
			curChar = (*(const byte *)text++);
			if (_font->isDoubleByte(curChar)) {
				curChar |= (*(const byte *)text++) << 8;
				len--;
			}
			switch (curChar) {
			case 0x0A:
			case 0x0D:
			case 0x9781: // this one is used by SQ4/japanese as line break as well
				textHeight = MAX<int16> (textHeight, _ports->_curPort->fontHeight);
				break;
			case 0x7C:
				if (getSciVersion() >= SCI_VERSION_1_1) {
					len -= CodeProcessing(text, orgFontId, 0, false);
					break;
				}
			default:
				textHeight = MAX<int16> (textHeight, _ports->_curPort->fontHeight);
				textWidth += _font->getCharWidth(curChar);
			}
		}
	}
	// When calculating size, we do not restore font because we need the current (code modified) font active
	//  If we are drawing this is called inbetween, so font needs to get restored
	//  If we are calculating size of just one fixed string (::StringWidth), then we need to restore
	if (restoreFont) {
		SetFont(previousFontId);
		_ports->penColor(previousPenColor);
	}
	return;
}

void GfxText16::StringWidth(const char *str, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight) {
	Width(str, 0, (int16)strlen(str), orgFontId, textWidth, textHeight, true);
}

void GfxText16::ShowString(const char *str, GuiResourceId orgFontId, int16 orgPenColor) {
	Show(str, 0, (int16)strlen(str), orgFontId, orgPenColor);
}
void GfxText16::DrawString(const char *str, GuiResourceId orgFontId, int16 orgPenColor) {
	Draw(str, 0, (int16)strlen(str), orgFontId, orgPenColor);
}

int16 GfxText16::Size(Common::Rect &rect, const char *text, GuiResourceId fontId, int16 maxWidth) {
	GuiResourceId previousFontId = GetFontId();
	int16 previousPenColor = _ports->_curPort->penClr;
	int16 charCount;
	int16 maxTextWidth = 0, textWidth;
	int16 totalHeight = 0, textHeight;

	if (fontId != -1)
		SetFont(fontId);
	else
		fontId = previousFontId;

	if (g_sci->getLanguage() == Common::JA_JPN)
		SwitchToFont900OnSjis(text);

	rect.top = rect.left = 0;

	if (maxWidth < 0) { // force output as single line
		StringWidth(text, fontId, textWidth, textHeight);
		rect.bottom = textHeight;
		rect.right = textWidth;
	} else {
		// rect.right=found widest line with RTextWidth and GetLongest
		// rect.bottom=num. lines * GetPointSize
		rect.right = (maxWidth ? maxWidth : 192);
		const char *curPos = text;
		while (*curPos) {
			charCount = GetLongest(curPos, rect.right, fontId);
			if (charCount == 0)
				break;
			Width(curPos, 0, charCount, fontId, textWidth, textHeight, false);
			maxTextWidth = MAX(textWidth, maxTextWidth);
			totalHeight += textHeight;
			curPos += charCount;
			while (*curPos == ' ')
				curPos++; // skip over breaking spaces
		}
		rect.bottom = totalHeight;
		rect.right = maxWidth ? maxWidth : MIN(rect.right, maxTextWidth);
	}
	SetFont(previousFontId);
	_ports->penColor(previousPenColor);
	return rect.right;
}

// returns maximum font height used
void GfxText16::Draw(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor) {
	uint16 curChar, charWidth;
	Common::Rect rect;

	GetFont();
	if (!_font)
		return;

	rect.top = _ports->_curPort->curTop;
	rect.bottom = rect.top + _ports->_curPort->fontHeight;
	text += from;
	while (len--) {
		curChar = (*(const byte *)text++);
		if (_font->isDoubleByte(curChar)) {
			curChar |= (*(const byte *)text++) << 8;
			len--;
		}
		switch (curChar) {
		case 0x0A:
		case 0x0D:
		case 0:
		case 0x9781: // this one is used by SQ4/japanese as line break as well
			break;
		case 0x7C:
			if (getSciVersion() >= SCI_VERSION_1_1) {
				len -= CodeProcessing(text, orgFontId, orgPenColor, true);
				break;
			}
		default:
			charWidth = _font->getCharWidth(curChar);
			// clear char
			if (_ports->_curPort->penMode == 1) {
				rect.left = _ports->_curPort->curLeft;
				rect.right = rect.left + charWidth;
				_paint16->eraseRect(rect);
			}
			// CharStd
			_font->draw(curChar, _ports->_curPort->top + _ports->_curPort->curTop, _ports->_curPort->left + _ports->_curPort->curLeft, _ports->_curPort->penClr, _ports->_curPort->greyedOutput);
			_ports->_curPort->curLeft += charWidth;
		}
	}
}

// returns maximum font height used
void GfxText16::Show(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor) {
	Common::Rect rect;

	rect.top = _ports->_curPort->curTop;
	rect.bottom = rect.top + _ports->getPointSize();
	rect.left = _ports->_curPort->curLeft;
	Draw(text, from, len, orgFontId, orgPenColor);
	rect.right = _ports->_curPort->curLeft;
	_paint16->bitsShow(rect);
}

// Draws a text in rect.
void GfxText16::Box(const char *text, bool show, const Common::Rect &rect, TextAlignment alignment, GuiResourceId fontId) {
	int16 textWidth, maxTextWidth, textHeight, charCount;
	int16 offset = 0;
	int16 hline = 0;
	GuiResourceId previousFontId = GetFontId();
	int16 previousPenColor = _ports->_curPort->penClr;
	bool doubleByteMode = false;

	if (fontId != -1)
		SetFont(fontId);
	else
		fontId = previousFontId;

	if (g_sci->getLanguage() == Common::JA_JPN) {
		if (SwitchToFont900OnSjis(text))
			doubleByteMode = true;
	}

	// Reset reference code rects
	_codeRefRects.clear();
	_codeRefTempRect.left = _codeRefTempRect.top = -1;

	maxTextWidth = 0;
	while (*text) {
		charCount = GetLongest(text, rect.width(), fontId);
		if (charCount == 0)
			break;
		Width(text, 0, charCount, fontId, textWidth, textHeight, true);
		maxTextWidth = MAX<int16>(maxTextWidth, textWidth);
		switch (alignment) {
		case SCI_TEXT16_ALIGNMENT_RIGHT:
			offset = rect.width() - textWidth;
			break;
		case SCI_TEXT16_ALIGNMENT_CENTER:
			offset = (rect.width() - textWidth) / 2;
			break;
		case SCI_TEXT16_ALIGNMENT_LEFT:
			offset = 0;
			break;

		default:
			warning("Invalid alignment %d used in TextBox()", alignment);
		}
		_ports->moveTo(rect.left + offset, rect.top + hline);

		if (show) {
			Show(text, 0, charCount, fontId, previousPenColor);
		} else {
			Draw(text, 0, charCount, fontId, previousPenColor);
		}

		hline += textHeight;
		text += charCount;
		while (*text == ' ')
			text++; // skip over breaking spaces
	}
	SetFont(previousFontId);
	_ports->penColor(previousPenColor);

	if (doubleByteMode) {
		// Kanji is written by pc98 rom to screen directly. Because of
		// GetLongest() behavior (not cutting off the last char, that causes a
		// new line), results in the script thinking that the text would need
		// less space. The coordinate adjustment in fontsjis.cpp handles the
		// incorrect centering because of that and this code actually shows all
		// of the chars - if we don't do this, the scripts will only show most
		// of the chars, but the last few pixels won't get shown most of the
		// time.
		Common::Rect kanjiRect = rect;
		_ports->offsetRect(kanjiRect);
		kanjiRect.left &= 0xFFC;
		kanjiRect.right = kanjiRect.left + maxTextWidth;
		kanjiRect.bottom = kanjiRect.top + hline;
		kanjiRect.left *= 2; kanjiRect.right *= 2;
		kanjiRect.top *= 2; kanjiRect.bottom *= 2;
		_screen->copyDisplayRectToScreen(kanjiRect);
	}
}

void GfxText16::DrawString(const char *text) {
	GuiResourceId previousFontId = GetFontId();
	int16 previousPenColor = _ports->_curPort->penClr;

	Draw(text, 0, strlen(text), previousFontId, previousPenColor);
	SetFont(previousFontId);
	_ports->penColor(previousPenColor);
}

// we need to have a separate status drawing code
//  In KQ4 the IV char is actually 0xA, which would otherwise get considered as linebreak and not printed
void GfxText16::DrawStatus(const char *text) {
	uint16 curChar, charWidth;
	uint16 textLen = strlen(text);
	Common::Rect rect;

	GetFont();
	if (!_font)
		return;

	rect.top = _ports->_curPort->curTop;
	rect.bottom = rect.top + _ports->_curPort->fontHeight;
	while (textLen--) {
		curChar = (*(const byte *)text++);
		switch (curChar) {
		case 0:
			break;
		default:
			charWidth = _font->getCharWidth(curChar);
			_font->draw(curChar, _ports->_curPort->top + _ports->_curPort->curTop, _ports->_curPort->left + _ports->_curPort->curLeft, _ports->_curPort->penClr, _ports->_curPort->greyedOutput);
			_ports->_curPort->curLeft += charWidth;
		}
	}
}

// Sierra did this in their PC98 interpreter only, they identify a text as being
// sjis and then switch to font 900
bool GfxText16::SwitchToFont900OnSjis(const char *text) {
	byte firstChar = (*(const byte *)text++);
	if (((firstChar >= 0x81) && (firstChar <= 0x9F)) || ((firstChar >= 0xE0) && (firstChar <= 0xEF))) {
		SetFont(900);
		return true;
	}
	return false;
}

reg_t GfxText16::allocAndFillReferenceRectArray() {
	uint rectCount = _codeRefRects.size();
	if (rectCount) {
		reg_t rectArray;
		byte *rectArrayPtr = g_sci->getEngineState()->_segMan->allocDynmem(4 * 2 * (rectCount + 1), "text code reference rects", &rectArray);
		GfxCoordAdjuster *coordAdjuster = g_sci->_gfxCoordAdjuster;
		for (uint curRect = 0; curRect < rectCount; curRect++) {
			coordAdjuster->kernelLocalToGlobal(_codeRefRects[curRect].left, _codeRefRects[curRect].top);
			coordAdjuster->kernelLocalToGlobal(_codeRefRects[curRect].right, _codeRefRects[curRect].bottom);
			WRITE_LE_UINT16(rectArrayPtr + 0, _codeRefRects[curRect].left);
			WRITE_LE_UINT16(rectArrayPtr + 2, _codeRefRects[curRect].top);
			WRITE_LE_UINT16(rectArrayPtr + 4, _codeRefRects[curRect].right);
			WRITE_LE_UINT16(rectArrayPtr + 6, _codeRefRects[curRect].bottom);
			rectArrayPtr += 8;
		}
		WRITE_LE_UINT16(rectArrayPtr + 0, 0x7777);
		WRITE_LE_UINT16(rectArrayPtr + 2, 0x7777);
		WRITE_LE_UINT16(rectArrayPtr + 4, 0x7777);
		WRITE_LE_UINT16(rectArrayPtr + 6, 0x7777);
		return rectArray;
	}
	return NULL_REG;
}

void GfxText16::kernelTextSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight) {
	Common::Rect rect(0, 0, 0, 0);
	Size(rect, text, font, maxWidth);
	*textWidth = rect.width();
	*textHeight = rect.height();
}

// Used SCI1+ for text codes
void GfxText16::kernelTextFonts(int argc, reg_t *argv) {
	int i;

	delete _codeFonts;
	_codeFontsCount = argc;
	_codeFonts = new GuiResourceId[argc];
	for (i = 0; i < argc; i++) {
		_codeFonts[i] = (GuiResourceId)argv[i].toUint16();
	}
}

// Used SCI1+ for text codes
void GfxText16::kernelTextColors(int argc, reg_t *argv) {
	int i;

	delete _codeColors;
	_codeColorsCount = argc;
	_codeColors = new uint16[argc];
	for (i = 0; i < argc; i++) {
		_codeColors[i] = argv[i].toUint16();
	}
}

} // End of namespace Sci
