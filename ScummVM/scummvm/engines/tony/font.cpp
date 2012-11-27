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

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "common/textconsole.h"
#include "tony/mpal/mpalutils.h"
#include "tony/font.h"
#include "tony/input.h"
#include "tony/inventory.h"
#include "tony/loc.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*       RMFont Methods
\****************************************************************************/

RMFont::RMFont() {
	_letter = NULL;
	_nLetters = _fontDimx = _fontDimy = _dimx = _dimy = 0;
}

RMFont::~RMFont() {
	unload();
}

void RMFont::load(const byte *buf, int nChars, int dimx, int dimy, uint32 palResID) {
	_letter = new RMGfxSourceBuffer8RLEByte[nChars];

	// Initialize the fonts
	for (int i = 0; i < nChars; i++) {
		// Initialize the buffer with the letters
		_letter[i].init(buf + i * (dimx * dimy + 8) + 8, dimx, dimy);
		_letter[i].loadPaletteWA(palResID);
	}

	_fontDimx = dimx;
	_fontDimy = dimy;

	_nLetters = nChars;
}

void RMFont::load(uint32 resID, int nChars, int dimx, int dimy, uint32 palResID) {
	RMRes res(resID);

	if ((int)res.size() < nChars * (dimy * dimx + 8))
		nChars = res.size() / (dimy * dimx + 8);

	load(res, nChars, dimx, dimy, palResID);
}

void RMFont::unload() {
	if (_letter != NULL) {
		delete[] _letter;
		_letter = NULL;
	}
}

RMGfxPrimitive *RMFont::makeLetterPrimitive(byte bChar, int &nLength) {
	RMFontPrimitive *prim;

	// Convert from character to glyph index
	int nLett = convertToLetter(bChar);
	assert(nLett < _nLetters);

	// Create primitive font
	prim = new RMFontPrimitive(this);
	prim->_nChar = nLett;

	// Get the length of the character in pixels
	nLength = letterLength(bChar);

	return prim;
}

void RMFont::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim2) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	RMFontPrimitive *prim = (RMFontPrimitive *)prim2;

	CORO_BEGIN_CODE(_ctx);

	// Call the draw method of the letter assigned to the primitive
	if (prim->_nChar != -1)
		CORO_INVOKE_2(_letter[prim->_nChar].draw, bigBuf, prim);

	CORO_END_CODE;
}

void RMFont::close() {
	unload();
}

int RMFont::stringLen(const Common::String &text) {
	if (text.empty())
		return letterLength('\0');

	uint len = 0;
	uint i;
	for (i = 0; i < text.size() - 1; i++)
		len += letterLength(text[i], text[i + 1]);
	len += letterLength(text[i]);

	return len;
}

int RMFont::stringLen(char bChar, char bNext) {
	return letterLength(bChar, bNext);
}

/****************************************************************************\
*       RMFontColor Methods
\****************************************************************************/

RMFontColor::RMFontColor() : RMFont() {
	_fontR = _fontG = _fontB = 255;
}

RMFontColor::~RMFontColor() {
}

void RMFontColor::setBaseColor(byte r1, byte g1, byte b1) {
	int r = (int)r1 << 16;
	int g = (int)g1 << 16;
	int b = (int)b1 << 16;

	int rstep = r / 14;
	int gstep = g / 14;
	int bstep = b / 14;

	byte pal[768 * 3];

	// Check if we are already on the right color
	if (_fontR == r1 && _fontG == g1 && _fontB == b1)
		return;

	_fontR = r1;
	_fontG = g1;
	_fontB = b1;

	// Constructs a new palette for the font
	for (int i = 1; i < 16; i++) {
		pal[i * 3 + 0] = r >> 16;
		pal[i * 3 + 1] = g >> 16;
		pal[i * 3 + 2] = b >> 16;

		r -= rstep;
		g -= gstep;
		b -= bstep;
	}

	pal[15 * 3 + 0] += 8;
	pal[15 * 3 + 1] += 8;
	pal[15 * 3 + 2] += 8;

	// Puts in all the letters
	for (int i = 0; i < _nLetters; i++)
		_letter[i].loadPaletteWA(pal);
}

/***************************************************************************\
*       RMFontWithTables Methods
\****************************************************************************/
int RMFontWithTables::convertToLetter(byte nChar) {
	return _cTable[nChar];
}

int RMFontWithTables::letterLength(int nChar, int nNext) {
	return (nChar != -1 ? _lTable[(byte)nChar] + _l2Table[(byte)nChar][(byte)nNext] : _lDefault);
}

/***************************************************************************\
*       RMFontDialog Methods
\****************************************************************************/

void RMFontDialog::init() {
	// bernie: Number of characters in the font
	int nchars =
	    112    // base
	    + 18    // polish
	    + 66    // russian
	    + 30    // czech
	    +  8    // french
	    +  5;   // deutsch

	load(RES_F_PARL, nchars, 20, 20);

	// Initialize the font table
	_lDefault = 13;
	_hDefault = 18;
	Common::fill(&_l2Table[0][0], &_l2Table[0][0] + (256 * 256), '\0');

	for (int i = 0; i < 256; i++) {
		_cTable[i] = g_vm->_cTableDialog[i];
		_lTable[i] = g_vm->_lTableDialog[i];
	}
}

/***************************************************************************\
*       RMFontMacc Methods
\****************************************************************************/

void RMFontMacc::init() {
	// bernie: Number of characters in the font
	int nchars =
	    102    // base
	    + 18    // polish
	    + 66    // russian
	    + 30    // czech
	    +  8    // francais
	    +  5;   // deutsch

	load(RES_F_MACC, nchars, 11, 16);

	// Default
	_lDefault = 10;
	_hDefault = 17;
	Common::fill(&_l2Table[0][0], &_l2Table[0][0] + (256 * 256), '\0');

	for (int i = 0; i < 256; i++) {
		_cTable[i] = g_vm->_cTableMacc[i];
		_lTable[i] = g_vm->_lTableMacc[i];
	}
}

/***************************************************************************\
*       RMFontCredits Methods
\****************************************************************************/

void RMFontCredits::init() {
	// bernie: Number of characters in the font
	int nchars =
	    112    // base
	    + 18    // polish
	    + 66    // russian
	    + 30    // czech
	    +  8    // french
	    +  2;   // deutsch

	load(RES_F_CREDITS, nchars, 27, 28, RES_F_CPAL);

	// Default
	_lDefault = 10;
	_hDefault = 28;
	Common::fill(&_l2Table[0][0], &_l2Table[0][0] + (256 * 256), '\0');

	for (int i = 0; i < 256; i++) {
		_cTable[i] = g_vm->_cTableCred[i];
		_lTable[i] = g_vm->_lTableCred[i];
	}
}

/***************************************************************************\
*       RMFontObj Methods
\****************************************************************************/

#define TOUPPER(a)  ((a) >= 'a' && (a) <= 'z' ? (a) + 'A' - 'a' : (a))
#define TOLOWER(a)  ((a) >= 'A' && (a) <= 'Z' ? (a) + 'a' - 'A' : (a))

void RMFontObj::setBothCase(int nChar, int nNext, signed char spiazz) {
	_l2Table[TOUPPER(nChar)][TOUPPER(nNext)] = spiazz;
	_l2Table[TOUPPER(nChar)][TOLOWER(nNext)] = spiazz;
	_l2Table[TOLOWER(nChar)][TOUPPER(nNext)] = spiazz;
	_l2Table[TOLOWER(nChar)][TOLOWER(nNext)] = spiazz;
}

void RMFontObj::init() {
	//bernie: Number of characters in the font (solo maiuscolo)
	int nchars =
	    85    // base
	    +  9    // polish
	    + 33    // russian
	    + 15    // czech
	    +  0    // francais (no uppercase chars)
	    +  1;   // deutsch

	load(RES_F_OBJ, nchars, 25, 30);

	// Initialize the font table
	_lDefault = 26;
	_hDefault = 30;
	Common::fill(&_l2Table[0][0], &_l2Table[0][0] + (256 * 256), '\0');

	for (int i = 0; i < 256; i++) {
		_cTable[i] = g_vm->_cTableObj[i];
		_lTable[i] = g_vm->_lTableObj[i];
	}

	// Special case
	setBothCase('C', 'C', 2);
	setBothCase('A', 'T', -2);
	setBothCase('R', 'S', 2);
	setBothCase('H', 'I', -2);
	setBothCase('T', 'S', 2);
	setBothCase('O', 'R', 2);
	setBothCase('O', 'L', 2);
	setBothCase('O', 'G', 2);
	setBothCase('Z', 'A', -1);
	setBothCase('R', 'R', 1);
	setBothCase('R', 'U', 3);
}

/****************************************************************************\
*       RMText Methods
\****************************************************************************/

RMFontColor *RMText::_fonts[4] = { NULL, NULL, NULL, NULL };

void RMText::initStatics() {
	Common::fill(&_fonts[0], &_fonts[4], (RMFontColor *)NULL);
}

RMText::RMText() {
	// Default color: white
	_textR = _textG = _textB = 255;

	// Default length
	_maxLineLength = 350;

	_bTrasp0 = true;
	_aHorType = HCENTER;
	_aVerType = VTOP;
	setPriority(150);
}

RMText::~RMText() {
}

void RMText::unload() {
	if (_fonts[0] != NULL) {
		delete _fonts[0];
		delete _fonts[1];
		delete _fonts[2];
		delete _fonts[3];
		_fonts[0] =  _fonts[1] = _fonts[2] = _fonts[3] = 0;
	}
}

void RMText::setMaxLineLength(int max) {
	_maxLineLength = max;
}

void RMText::removeThis(CORO_PARAM, bool &result) {
	// Here we can do checks on the number of frames, time spent, etc.
	result = true;
}

void RMText::writeText(const Common::String &text, int nFont, int *time) {
	// Initializes the font (only once)
	if (_fonts[0] == NULL) {
		_fonts[0] = new RMFontDialog;
		_fonts[0]->init();
		_fonts[1] = new RMFontObj;
		_fonts[1]->init();
		_fonts[2] = new RMFontMacc;
		_fonts[2]->init();
		_fonts[3] = new RMFontCredits;
		_fonts[3]->init();
	}

	writeText(text, _fonts[nFont], time);
}

void RMText::writeText(Common::String text, RMFontColor *font, int *time) {
	RMGfxPrimitive *prim;

	// Set the base color
	font->setBaseColor(_textR, _textG, _textB);

	// Destroy the buffer before starting
	destroy();

	// If the string is empty, do nothing
	if (text.empty())
		return;

	// Divide the words into lines. In this cycle, X contains the maximum length reached by a line,
	// and the number of lines
	Common::Array<Common::String> lines;
	uint p = 0;
	int j = 0;
	int x = 0;
	while (p < text.size()) {
		j += font->stringLen(text[p]);
		if (j > (((_aHorType == HLEFTPAR) && (lines.size() > 0)) ? _maxLineLength - 25 : _maxLineLength)) {
			j -= font->stringLen(text[p], (p + 1 == text.size()) ? '\0' : text[p + 1]);
			if (j > x)
				x = j;

			// Back to the first usable space
			//
			// BERNIE: In the original, sentences containing words that exceed the
			// width of a line caused discontinuation of the whole sentence.
			// This workaround has the partial word broken up so it will still display
			//
			uint old_p = p;
			while (text[p] != ' ' && text[p] != '-' && p > 0)
				p--;

			if (p == 0)
				p = old_p;

			// Check if there are any blanks to end
			while ((text[p] == ' ' || text[p] == '-') && p + 1 < text.size())
				p++;
			if (p == text.size())
				break;
			lines.push_back(Common::String(text.c_str(), p));
			if (text[p] == ' ')
				p++;
			text = text.c_str() + p;
			p = 0;
			j = 0;
			continue;
		}
		p++;
	}

	if (j > x)
		x = j;

	// Add the last line of text.
	lines.push_back(text);

	x += 8;

	// Starting position for the surface: X1, Y
	int width = x;
	int height = (lines.size() - 1) * font->letterHeight() + font->_fontDimy;

	// Create the surface
	create(width, height);
	Common::fill(_buf, _buf + width * height * 2, 0);

	p = 0;

	int y = 0;
	int numchar = 0;
	for (uint i = 0; i < lines.size(); ++i) {
		const Common::String &line = lines[i];

		// Measure the length of the line
		x = 0;
		j = font->stringLen(line);

		switch (_aHorType) {
		case HLEFT:
			x = 0;
			break;

		case HLEFTPAR:
			if (i == 0)
				x = 0;
			else
				x = 25;
			break;

		case HCENTER:
			x = width / 2 - j / 2;
			break;

		case HRIGHT:
			x = width - j - 1;
			break;
		}

		p = 0;
		while (p < line.size()) {
			if (line[p] == ' ') {
				x += font->stringLen(line[p]);
				p++;
				continue;
			}

			int len;
			prim = font->makeLetterPrimitive(line[p], len);
			prim->getDst()._x1 = x;
			prim->getDst()._y1 = y;
			addPrim(prim);

			numchar++;

			x += font->stringLen(line[p], (p + 1 == line.size()) ? '\0' : line[p + 1]);
			p++;
		}
		p++;
		y += font->letterHeight();
	}

	if (time != NULL)
		*time = 1000 + numchar * (11 - GLOBALS._nCfgTextSpeed) * 14;
}

void RMText::clipOnScreen(RMGfxPrimitive *prim) {
	// Don't let it go outside the screen
	if (prim->getDst()._x1 < 5)
		prim->getDst()._x1 = 5;
	if (prim->getDst()._y1 < 5)
		prim->getDst()._y1 = 5;
	if (prim->getDst()._x1 + _dimx > 635)
		prim->getDst()._x1 = 635 - _dimx;
	if (prim->getDst()._y1 + _dimy > 475)
		prim->getDst()._y1 = 475 - _dimy;
}

void RMText::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	// Horizontally
	if (_aHorType == HCENTER)
		prim->getDst().topLeft() -= RMPoint(_dimx / 2, 0);
	else if (_aHorType == HRIGHT)
		prim->getDst().topLeft() -= RMPoint(_dimx, 0);

	// Vertically
	if (_aVerType == VTOP) {

	} else if (_aVerType == VCENTER) {
		prim->getDst()._y1 -= _dimy / 2;

	} else if (_aVerType == VBOTTOM) {
		prim->getDst()._y1 -= _dimy;
	}

	clipOnScreen(prim);

	CORO_INVOKE_2(RMGfxWoodyBuffer::draw, bigBuf, prim);

	CORO_END_CODE;
}

/**
 * Set the alignment type
 */
void RMText::setAlignType(HorAlign aHor, VerAlign aVer) {
	_aHorType = aHor;
	_aVerType = aVer;
}

/**
 * Set the base color
 */
void RMText::setColor(byte r, byte g, byte b) {
	_textR = r;
	_textG = g;
	_textB = b;
}

/****************************************************************************\
*       RMTextDialog Methods
\****************************************************************************/

RMTextDialog::RMTextDialog() : RMText() {
	_time = _startTime = 0;
	_dst = RMPoint(0, 0);

	_bSkipStatus = true;
	_bShowed = true;
	_bForceTime = false;
	_bForceNoTime = false;
	_bAlwaysDisplay = false;
	_bNoTab = false;
	_hCustomSkip = CORO_INVALID_PID_VALUE;
	_hCustomSkip2 = CORO_INVALID_PID_VALUE;
	_input = NULL;

	// Create the event for displaying the end
	_hEndDisplay = CoroScheduler.createEvent(false, false);
}

RMTextDialog::~RMTextDialog() {
	CoroScheduler.closeEvent(_hEndDisplay);
}

void RMTextDialog::show() {
	_bShowed = true;
}

void RMTextDialog::hide(CORO_PARAM) {
	_bShowed = false;
}

void RMTextDialog::writeText(const Common::String &text, int font, int *time) {
	RMText::writeText(text, font, &_time);

	if (time != NULL)
		*time = _time;
}

void RMTextDialog::writeText(const Common::String &text, RMFontColor *font, int *time) {
	RMText::writeText(text, font, &_time);

	if (time != NULL)
		*time = _time;
}

void RMTextDialog::setSkipStatus(bool bEnabled) {
	_bSkipStatus = bEnabled;
}

void RMTextDialog::forceTime() {
	_bForceTime = true;
}

void RMTextDialog::forceNoTime() {
	_bForceNoTime = true;
}

void RMTextDialog::setNoTab() {
	_bNoTab = true;
}

void RMTextDialog::setForcedTime(uint32 dwTime) {
	_time = dwTime;
}

void RMTextDialog::setAlwaysDisplay() {
	_bAlwaysDisplay = true;
}

void RMTextDialog::removeThis(CORO_PARAM, bool &result) {
	CORO_BEGIN_CONTEXT;
	bool expired;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Presume successful result
	result = true;

	// Don't erase the background
	if (_bSkipStatus) {
		if (!(GLOBALS._bCfgDubbing && _hCustomSkip2 != CORO_INVALID_PID_VALUE)) {
			if (GLOBALS._bCfgTimerizedText) {
				if (!_bForceNoTime) {
					if (g_vm->getTime() > (uint32)_time + _startTime)
						return;
				}
			}
		}

		if (!_bNoTab) {
			if (g_vm->getEngine()->getInput().getAsyncKeyState(Common::KEYCODE_TAB))
				return;
		}

		if (!_bNoTab) {
			if (_input) {
				if (_input->mouseLeftClicked() || _input->mouseRightClicked())
					return;
			}
		}
	}

	// Erase the background
	else if (!(GLOBALS._bCfgDubbing && _hCustomSkip2 != CORO_INVALID_PID_VALUE)) {
		if (!_bForceNoTime) {
			if (g_vm->getTime() > (uint32)_time + _startTime)
				return;
		}
	}

	// If time is forced
	if (_bForceTime) {
		if (g_vm->getTime() > (uint32)_time + _startTime)
			return;
	}

	if (_hCustomSkip != CORO_INVALID_PID_VALUE) {
		CORO_INVOKE_3(CoroScheduler.waitForSingleObject, _hCustomSkip, 0, &_ctx->expired);
		// == WAIT_OBJECT_0
		if (!_ctx->expired)
			return;
	}

	if (GLOBALS._bCfgDubbing && _hCustomSkip2 != CORO_INVALID_PID_VALUE) {
		CORO_INVOKE_3(CoroScheduler.waitForSingleObject, _hCustomSkip2, 0, &_ctx->expired);
		// == WAIT_OBJECT_0
		if (!_ctx->expired)
			return;
	}

	result = false;

	CORO_END_CODE;
}

void RMTextDialog::unregister() {
	RMGfxTask::unregister();
	assert(_nInList == 0);
	CoroScheduler.setEvent(_hEndDisplay);
}

void RMTextDialog::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_startTime == 0)
		_startTime = g_vm->getTime();

	if (_bShowed) {
		if (GLOBALS._bShowSubtitles || _bAlwaysDisplay) {
			prim->getDst().topLeft() = _dst;
			CORO_INVOKE_2(RMText::draw, bigBuf, prim);
		}
	}

	CORO_END_CODE;
}

void RMTextDialog::setCustomSkipHandle(uint32 hCustom) {
	_hCustomSkip = hCustom;
}

void RMTextDialog::setCustomSkipHandle2(uint32 hCustom) {
	_hCustomSkip2 = hCustom;
}

void RMTextDialog::waitForEndDisplay(CORO_PARAM) {
	CoroScheduler.waitForSingleObject(coroParam, _hEndDisplay, CORO_INFINITE);
}

void RMTextDialog::setInput(RMInput *input) {
	_input = input;
}

/**
 * Set the position
 */
void RMTextDialog::setPosition(const RMPoint &pt) {
	_dst = pt;
}

/****************************************************************************\
*       RMTextDialogScrolling Methods
\****************************************************************************/

RMTextDialogScrolling::RMTextDialogScrolling() {
	_curLoc = NULL;
}

RMTextDialogScrolling::RMTextDialogScrolling(RMLocation *loc) {
	_curLoc = loc;
	_startScroll = loc->scrollPosition();
}

RMTextDialogScrolling::~RMTextDialogScrolling() {
}

void RMTextDialogScrolling::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	RMPoint curDst;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curDst = _dst;

	if (_curLoc != NULL)
		_dst -= _curLoc->scrollPosition() - _startScroll;

	CORO_INVOKE_2(RMTextDialog::draw, bigBuf, prim);

	_dst = _ctx->curDst;

	CORO_END_CODE;
}

void RMTextDialogScrolling::clipOnScreen(RMGfxPrimitive *prim) {
	// We must not do anything!
}

/****************************************************************************\
*       RMTextItemName Methods
\****************************************************************************/

RMTextItemName::RMTextItemName() : RMText() {
	_item = NULL;
	setPriority(220);
}

RMTextItemName::~RMTextItemName() {
}

void RMTextItemName::doFrame(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMLocation &loc, RMPointer &ptr, RMInventory &inv) {
	CORO_BEGIN_CONTEXT;
	RMItem *lastItem;
	uint32 hThread;
	CORO_END_CONTEXT(_ctx);

	Common::String itemName;

	CORO_BEGIN_CODE(_ctx);

	_ctx->lastItem = _item;

	// Adds to the list if there is need
	if (!_nInList)
		bigBuf.addPrim(new RMGfxPrimitive(this));

	// Update the scrolling co-ordinates
	_curscroll = loc.scrollPosition();

	// Check if we are on the inventory
	if (inv.itemInFocus(_mpos))
		_item = inv.whichItemIsIn(_mpos);
	else
		_item = loc.whichItemIsIn(_mpos);

	// If there an item, get its name
	if (_item != NULL)
		_item->getName(itemName);

	// Write it
	writeText(itemName, 1);

	// Handle the change If the selected item is different from the previous one
	if (_ctx->lastItem != _item) {
		if (_item == NULL)
			ptr.setSpecialPointer(RMPointer::PTR_NONE);
		else {
			_ctx->hThread = mpalQueryDoAction(20, _item->mpalCode(), 0);
			if (_ctx->hThread == CORO_INVALID_PID_VALUE)
				ptr.setSpecialPointer(RMPointer::PTR_NONE);
			else
				CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->hThread, CORO_INFINITE);
		}
	}

	CORO_END_CODE;
}

void RMTextItemName::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// If there is no text, it's pointless to continue
	if (_buf == NULL)
		return;

	// Set the destination coordinates of the mouse
	prim->getDst().topLeft() = _mpos - RMPoint(0, 30);

	CORO_INVOKE_2(RMText::draw, bigBuf, prim);

	CORO_END_CODE;
}

RMPoint RMTextItemName::getHotspot() {
	if (_item == NULL)
		return _mpos + _curscroll;
	else
		return _item->getHotspot();
}

RMItem *RMTextItemName::getSelectedItem() {
	return _item;
}

bool RMTextItemName::isItemSelected() {
	return _item != NULL;
}

void RMTextItemName::setMouseCoord(const RMPoint &m) {
	_mpos = m;
}

void RMTextItemName::removeThis(CORO_PARAM, bool &result) {
	result = true;
}

/****************************************************************************\
*       RMDialogChoice Methods
\****************************************************************************/

RMDialogChoice::RMDialogChoice() {
	RMResRaw dlg1(RES_I_DLGTEXT);
	RMResRaw dlg2(RES_I_DLGTEXTLINE);
	RMRes dlgpal(RES_I_DLGTEXTPAL);

	_dlgText.init(dlg1, dlg1.width(), dlg1.height());
	_dlgTextLine.init(dlg2, dlg2.width(), dlg2.height());

	_dlgText.loadPaletteWA(dlgpal);
	_dlgTextLine.loadPaletteWA(dlgpal);

	_hUnreg = CoroScheduler.createEvent(false, false);
	_bRemoveFromOT = false;

	_curAdded = 0;
	_bShow = false;
}

RMDialogChoice::~RMDialogChoice() {
	CoroScheduler.closeEvent(_hUnreg);
}

void RMDialogChoice::unregister() {
	RMGfxWoodyBuffer::unregister();
	assert(!_nInList);
	CoroScheduler.pulseEvent(_hUnreg);

	_bRemoveFromOT = false;
}

void RMDialogChoice::init() {
	_numChoices = 0;
	_drawedStrings = NULL;
	_ptDrawStrings = NULL;
	_curSelection = -1;

	create(640, 477);
	setPriority(140);
}

void RMDialogChoice::close() {
	if (_drawedStrings != NULL) {
		delete[] _drawedStrings;
		_drawedStrings = NULL;
	}

	if (_ptDrawStrings != NULL) {
		delete[] _ptDrawStrings;
		_ptDrawStrings = NULL;
	}

	destroy();
}

void RMDialogChoice::setNumChoices(int num) {
	_numChoices = num;
	_curAdded = 0;

	// Allocate space for drawn strings
	_drawedStrings = new RMText[num];
	_ptDrawStrings = new RMPoint[num];

	// Initialization
	for (int i = 0; i < _numChoices; i++) {
		_drawedStrings[i].setColor(0, 255, 0);
		_drawedStrings[i].setAlignType(RMText::HLEFTPAR, RMText::VTOP);
		_drawedStrings[i].setMaxLineLength(600);
		_drawedStrings[i].setPriority(10);
	}
}

void RMDialogChoice::addChoice(const Common::String &string) {
	// Draw the string
	assert(_curAdded < _numChoices);
	_drawedStrings[_curAdded++].writeText(string, 0);
}

void RMDialogChoice::prepare(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	int i;
	RMPoint ptPos;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	addPrim(new RMGfxPrimitive(&_dlgText, RMPoint(0, 0)));
	addPrim(new RMGfxPrimitive(&_dlgTextLine, RMPoint(0, 155)));
	addPrim(new RMGfxPrimitive(&_dlgTextLine, RMPoint(0, 155 + 83)));
	addPrim(new RMGfxPrimitive(&_dlgTextLine, RMPoint(0, 155 + 83 + 83)));
	addPrim(new RMGfxPrimitive(&_dlgTextLine, RMPoint(0, 155 + 83 + 83 + 83)));

	_ctx->ptPos.set(20, 90);

	for (_ctx->i = 0; _ctx->i < _numChoices; _ctx->i++) {
		addPrim(new RMGfxPrimitive(&_drawedStrings[_ctx->i], _ctx->ptPos));
		_ptDrawStrings[_ctx->i] = _ctx->ptPos;
		_ctx->ptPos.offset(0, _drawedStrings[_ctx->i].getDimy() + 15);
	}

	CORO_INVOKE_0(drawOT);
	clearOT();

	_ptDrawPos.set(0, 480 - _ctx->ptPos._y);

	CORO_END_CODE;
}

void RMDialogChoice::setSelected(CORO_PARAM, int pos) {
	CORO_BEGIN_CONTEXT;
	RMGfxBox box;
	RMRect rc;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (pos == _curSelection)
		return;

	_ctx->box.setPriority(5);

	if (_curSelection != -1) {
		_ctx->box.setColor(0xCC, 0xCC, 0xFF);
		_ctx->rc.topLeft() = RMPoint(18, _ptDrawStrings[_curSelection]._y);
		_ctx->rc.bottomRight() = _ctx->rc.topLeft() + RMPoint(597, _drawedStrings[_curSelection].getDimy());
		addPrim(new RMGfxPrimitive(&_ctx->box, _ctx->rc));

		addPrim(new RMGfxPrimitive(&_drawedStrings[_curSelection], _ptDrawStrings[_curSelection]));
		CORO_INVOKE_0(drawOT);
		clearOT();
	}

	if (pos != -1) {
		_ctx->box.setColor(100, 100, 100);
		_ctx->rc.topLeft() = RMPoint(18, _ptDrawStrings[pos]._y);
		_ctx->rc.bottomRight() = _ctx->rc.topLeft() + RMPoint(597, _drawedStrings[pos].getDimy());
		addPrim(new RMGfxPrimitive(&_ctx->box, _ctx->rc));
		addPrim(new RMGfxPrimitive(&_drawedStrings[pos], _ptDrawStrings[pos]));
	}

	CORO_INVOKE_0(drawOT);
	clearOT();

	_curSelection = pos;

	CORO_END_CODE;
}

void RMDialogChoice::show(CORO_PARAM, RMGfxTargetBuffer *bigBuf) {
	CORO_BEGIN_CONTEXT;
	RMPoint destpt;
	int deltay;
	int starttime;
	int elaps;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_0(prepare);
	_bShow = false;

	if (!_nInList && bigBuf != NULL)
		bigBuf->addPrim(new RMGfxPrimitive(this));

	if (0) {
		_bShow = true;
	} else {
		_ctx->starttime = g_vm->getTime();
		_ctx->deltay = 480 - _ptDrawPos._y;
		_ctx->destpt = _ptDrawPos;
		_ptDrawPos.set(0, 480);

		if (!_nInList && bigBuf != NULL)
			bigBuf->addPrim(new RMGfxPrimitive(this));
		_bShow = true;

		_ctx->elaps = 0;
		while (_ctx->elaps < 700) {
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);
			_ctx->elaps = g_vm->getTime() - _ctx->starttime;
			_ptDrawPos._y = 480 - ((_ctx->deltay * 100) / 700 * _ctx->elaps) / 100;
		}

		_ptDrawPos._y = _ctx->destpt._y;
	}

	CORO_END_CODE;
}

void RMDialogChoice::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_bShow == false)
		return;

	prim->setDst(_ptDrawPos);
	CORO_INVOKE_2(RMGfxSourceBuffer16::draw, bigBuf, prim);

	CORO_END_CODE;
}

void RMDialogChoice::hide(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	int deltay;
	int starttime;
	int elaps;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (1) {
		_ctx->starttime = g_vm->getTime();

		_ctx->deltay = 480 - _ptDrawPos._y;
		_ctx->elaps = 0;
		while (_ctx->elaps < 700) {
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);
			_ctx->elaps = g_vm->getTime() - _ctx->starttime;
			_ptDrawPos._y = 480 - ((_ctx->deltay * 100) / 700 * (700 - _ctx->elaps)) / 100;
		}
	}

	_bShow = false;
	_bRemoveFromOT = true;
	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _hUnreg, CORO_INFINITE);

	CORO_END_CODE;
}

void RMDialogChoice::removeThis(CORO_PARAM, bool &result) {
	result = _bRemoveFromOT;
}

void RMDialogChoice::doFrame(CORO_PARAM, RMPoint ptMousePos) {
	CORO_BEGIN_CONTEXT;
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (ptMousePos._y > _ptDrawPos._y) {
		for (_ctx->i = 0; _ctx->i < _numChoices; _ctx->i++) {
			if ((ptMousePos._y >= _ptDrawPos._y + _ptDrawStrings[_ctx->i]._y) && (ptMousePos._y < _ptDrawPos._y + _ptDrawStrings[_ctx->i]._y + _drawedStrings[_ctx->i].getDimy())) {
				CORO_INVOKE_1(setSelected, _ctx->i);
				break;
			}
		}

		if (_ctx->i == _numChoices)
			CORO_INVOKE_1(setSelected, -1);
	}

	CORO_END_CODE;
}

int RMDialogChoice::getSelection() {
	return _curSelection;
}

} // End of namespace Tony
