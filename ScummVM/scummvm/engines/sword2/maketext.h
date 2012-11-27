/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SWORD2_MAKETEXT_H
#define SWORD2_MAKETEXT_H

#include "sword2/debug.h"

namespace Sword2 {

// Output color for character border - should be be black but note that we
// have to use a different pen number during sequences

#define BORDER_PEN 194

// Usually the only texts on screen are the subtitles and the mouse-over text,
// but there can also be a considerable number of debugging messages...

#define MAX_text_blocs MAX_DEBUG_TEXTS + 1

enum {
	// Doesn't keep the text inside the screen - only for debug text!
	NO_JUSTIFICATION = 0,

	// These all force text inside the screen edge margin when necessary
	POSITION_AT_CENTER_OF_BASE = 1,
	POSITION_AT_CENTER_OF_TOP = 2,
	POSITION_AT_LEFT_OF_TOP = 3,
	POSITION_AT_RIGHT_OF_TOP = 4,
	POSITION_AT_LEFT_OF_BASE = 5,
	POSITION_AT_RIGHT_OF_BASE = 6,
	POSITION_AT_LEFT_OF_CENTER = 7,
	POSITION_AT_RIGHT_OF_CENTER = 8
};

enum {
	DEFAULT_TEXT = 0,
	FINNISH_TEXT = 1,
	POLISH_TEXT = 2
};

// Info about the text, used to create the SpriteInfo struct

 struct TextBloc {
	int16 x;
	int16 y;
	uint16 type;
	byte *text_mem;
};

// Info for each line of words in the output text sprite

struct LineInfo {
	uint16 width;	// Width in pixels
	uint16 length;	// Length in characters
};

class FontRenderer {
private:
	Sword2Engine *_vm;
	TextBloc _blocList[MAX_text_blocs];

	// Layout variables - these used to be defines, but now we're dealing
	// with three character sets

	int8 _lineSpacing;	// no. of pixels to separate lines of
				// characters in the output sprite - negative
				// for overlap
	int8 _charSpacing;	// no. of pixels to separate characters along
				// each line - negative for overlap
	uint8 _borderPen;	// output pen color of character borders

	uint16 analyzeSentence(byte *sentence, uint16 maxWidth, uint32 fontRes, LineInfo *line);
	byte *buildTextSprite(byte *sentence, uint32 fontRes, uint8 pen, LineInfo *line, uint16 noOfLines);
	uint16 charWidth(byte ch, uint32 fontRes);
	uint16 charHeight(uint32 fontRes);
	byte *findChar(byte ch, byte *charSet);
	void copyChar(byte *charPtr, byte *spritePtr, uint16 spriteWidth, uint8 pen);

public:
	FontRenderer(Sword2Engine *vm) : _vm(vm) {
		for (int i = 0; i < MAX_text_blocs; i++)
			_blocList[i].text_mem = NULL;
	}

	~FontRenderer() {
		for (int i = 0; i < MAX_text_blocs; i++)
			free(_blocList[i].text_mem);
	}

	byte *makeTextSprite(byte *sentence, uint16 maxWidth, uint8 pen, uint32 fontRes, uint8 border = BORDER_PEN);

	void killTextBloc(uint32 bloc_number);
	void printTextBlocs();

	uint32 buildNewBloc(byte *ascii, int16 x, int16 y, uint16 width, uint8 pen, uint32 type, uint32 fontRes, uint8 justification);
};

} // End of namespace Sword2

#endif
