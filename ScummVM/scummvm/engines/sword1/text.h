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

#ifndef SWORD1_TEXT_H
#define SWORD1_TEXT_H

#include "sword1/object.h"
#include "sword1/sworddefs.h"

namespace Sword1 {

#define MAX_TEXT_OBS 3

#define BORDER_COL  200
#define BORDER_COL_PSX 199
#define LETTER_COL  193
#define NO_COL      0   // sprite background - 0 for transparency

class ObjectMan;
class ResMan;

struct LineInfo {
	uint16 width;  // width of line in pixels
	uint16 length; // length of line in characters
};

class Text {
public:
	Text(ObjectMan *pObjMan, ResMan *pResMan, bool czechVersion);
	~Text();
	FrameHeader *giveSpriteData(uint32 textTarget);
	uint32 lowTextManager(uint8 *text, int32 width, uint8 pen);
	void makeTextSprite(uint8 slot, const uint8 *text, uint16 maxWidth, uint8 pen);
	void releaseText(uint32 id, bool updateCount = true);

private:
	uint16 analyzeSentence(const uint8 *text, uint16 maxWidth, LineInfo *info);
	uint16 charWidth(uint8 ch);
	uint16 copyChar(uint8 ch, uint8 *sprPtr, uint16 sprWidth, uint8 pen);
	uint8 *_font;
	uint8 _textCount;
	uint16 _charHeight, _joinWidth;
	ObjectMan *_objMan;
	ResMan *_resMan;
	FrameHeader *_textBlocks[MAX_TEXT_OBS];
	uint32 _fontId;
};

} // End of namespace Sword1

#endif //BSTEXT_H
