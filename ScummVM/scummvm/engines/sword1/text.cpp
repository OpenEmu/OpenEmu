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


#include "common/textconsole.h"

#include "sword1/text.h"
#include "sword1/resman.h"
#include "sword1/objectman.h"
#include "sword1/swordres.h"
#include "sword1/sworddefs.h"
#include "sword1/screen.h"
#include "sword1/sword1.h"

namespace Sword1 {

#define OVERLAP 3
#define SPACE ' '
#define MAX_LINES       30


Text::Text(ObjectMan *pObjMan, ResMan *pResMan, bool czechVersion) {
	_objMan = pObjMan;
	_resMan = pResMan;
	_textCount = 0;
	_fontId = (czechVersion) ? CZECH_GAME_FONT : GAME_FONT;
	_font = (uint8 *)_resMan->openFetchRes(_fontId);

	_joinWidth = charWidth(SPACE) - 2 * OVERLAP;
	_charHeight = _resMan->getUint16(_resMan->fetchFrame(_font, 0)->height); // all chars have the same height
	for (int i = 0; i < MAX_TEXT_OBS; i++)
		_textBlocks[i] = NULL;
}

Text::~Text() {
	for (int i = 0; i < MAX_TEXT_OBS; i++)
		free(_textBlocks[i]);
	//_resMan->resClose(_fontId); => wiped automatically by _resMan->flush();
}

uint32 Text::lowTextManager(uint8 *ascii, int32 width, uint8 pen) {
	_textCount++;
	if (_textCount > MAX_TEXT_OBS)
		error("Text::lowTextManager: MAX_TEXT_OBS exceeded");
	uint32 textObjId = (TEXT_sect * ITM_PER_SEC) - 1;
	do {
		textObjId++;
	} while (_objMan->fetchObject(textObjId)->o_status);
	// okay, found a free text object

	_objMan->fetchObject(textObjId)->o_status = STAT_FORE;
	makeTextSprite((uint8)textObjId, ascii, (uint16)width, pen);

	return textObjId;
}

void Text::makeTextSprite(uint8 slot, const uint8 *text, uint16 maxWidth, uint8 pen) {
	LineInfo lines[MAX_LINES];
	uint16 numLines = analyzeSentence(text, maxWidth, lines);

	uint16 sprWidth = 0;
	uint16 lineCnt;
	for (lineCnt = 0; lineCnt < numLines; lineCnt++)
		if (lines[lineCnt].width > sprWidth)
			sprWidth = lines[lineCnt].width;

	uint16 sprHeight = _charHeight * numLines;
	uint32 sprSize = sprWidth * sprHeight;
	assert(!_textBlocks[slot]); // if this triggers, the speechDriver failed to call Text::releaseText.
	_textBlocks[slot] = (FrameHeader *)malloc(sprSize + sizeof(FrameHeader));

	memcpy(_textBlocks[slot]->runTimeComp, "Nu  ", 4);
	_textBlocks[slot]->compSize = 0;
	_textBlocks[slot]->width    = _resMan->toUint16(sprWidth);
	_textBlocks[slot]->height   = _resMan->toUint16(sprHeight);
	_textBlocks[slot]->offsetX  = 0;
	_textBlocks[slot]->offsetY  = 0;

	uint8 *linePtr = ((uint8 *)_textBlocks[slot]) + sizeof(FrameHeader);
	memset(linePtr, NO_COL, sprSize);
	for (lineCnt = 0; lineCnt < numLines; lineCnt++) {
		uint8 *sprPtr = linePtr + (sprWidth - lines[lineCnt].width) / 2; // center the text
		for (uint16 pos = 0; pos < lines[lineCnt].length; pos++)
			sprPtr += copyChar(*text++, sprPtr, sprWidth, pen) - OVERLAP;
		text++; // skip space at the end of the line
		if (SwordEngine::isPsx()) //Chars are half height in psx version
			linePtr += (_charHeight / 2) * sprWidth;
		else
			linePtr += _charHeight * sprWidth;
	}
}

uint16 Text::charWidth(uint8 ch) {
	if (ch < SPACE)
		ch = 64;
	return _resMan->getUint16(_resMan->fetchFrame(_font, ch - SPACE)->width);
}

uint16 Text::analyzeSentence(const uint8 *text, uint16 maxWidth, LineInfo *line) {
	uint16 lineNo = 0;

	bool firstWord = true;
	while (*text) {
		uint16 wordWidth = 0;
		uint16 wordLength = 0;

		while ((*text != SPACE) && *text) {
			wordWidth += charWidth(*text) - OVERLAP;
			wordLength++;
			text++;
		}
		if (*text == SPACE)
			text++;

		wordWidth += OVERLAP; // no overlap on final letter of word!
		if (firstWord)  { // first word on first line, so no separating SPACE needed
			line[0].width = wordWidth;
			line[0].length = wordLength;
			firstWord = false;
		} else {
			// see how much extra space this word will need to fit on current line
			// (with a separating space character - also overlapped)
			uint16 spaceNeeded = _joinWidth + wordWidth;

			if (line[lineNo].width + spaceNeeded <= maxWidth) {
				line[lineNo].width += spaceNeeded;
				line[lineNo].length += 1 + wordLength; // NB. space+word characters
			} else {    // put word (without separating SPACE) at start of next line
				lineNo++;
				assert(lineNo < MAX_LINES);
				line[lineNo].width = wordWidth;
				line[lineNo].length = wordLength;
			}
		}
	}
	return lineNo + 1;  // return no of lines
}

uint16 Text::copyChar(uint8 ch, uint8 *sprPtr, uint16 sprWidth, uint8 pen) {
	if (ch < SPACE)
		ch = 64;
	FrameHeader *chFrame = _resMan->fetchFrame(_font, ch - SPACE);
	uint8 *chData = ((uint8 *)chFrame) + sizeof(FrameHeader);
	uint8 *dest = sprPtr;
	uint8 *decBuf = NULL;
	uint8 *decChr;
	uint16 frameHeight = 0;

	if (SwordEngine::isPsx()) {
		frameHeight = _resMan->getUint16(chFrame->height) / 2;
		if (_fontId == CZECH_GAME_FONT) { //Czech game fonts are compressed
			decBuf = (uint8 *)malloc((_resMan->getUint16(chFrame->width)) * (_resMan->getUint16(chFrame->height) / 2));
			Screen::decompressHIF(chData, decBuf);
			decChr = decBuf;
		} else //Normal game fonts are not compressed
			decChr = chData;
	} else {
		frameHeight =  _resMan->getUint16(chFrame->height);
		decChr = chData;
	}

	for (uint16 cnty = 0; cnty < frameHeight; cnty++) {
		for (uint16 cntx = 0; cntx < _resMan->getUint16(chFrame->width); cntx++) {
			if (*decChr == LETTER_COL)
				dest[cntx] = pen;
			else if (((*decChr == BORDER_COL) || (*decChr == BORDER_COL_PSX)) && (!dest[cntx])) // don't do a border if there's already a color underneath (chars can overlap)
				dest[cntx] = BORDER_COL;
			decChr++;
		}
		dest += sprWidth;
	}
	free(decBuf);
	return _resMan->getUint16(chFrame->width);
}

FrameHeader *Text::giveSpriteData(uint32 textTarget) {
	// textTarget is the resource ID of the Compact linking the textdata.
	// that's 0x950000 for slot 0 and 0x950001 for slot 1. easy, huh? :)
	textTarget &= ITM_ID;
	assert(textTarget < MAX_TEXT_OBS);

	return _textBlocks[textTarget];
}

void Text::releaseText(uint32 id, bool updateCount) {
	id &= ITM_ID;
	assert(id < MAX_TEXT_OBS);
	if (_textBlocks[id]) {
		free(_textBlocks[id]);
		_textBlocks[id] = NULL;
		if (updateCount)
			_textCount--;
	}
}

} // End of namespace Sword1
