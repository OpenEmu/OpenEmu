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

#ifndef TOON_FONT_H
#define TOON_FONT_H

#include "toon/toon.h"

namespace Toon {

class FontRenderer {
public:
	FontRenderer(ToonEngine *vm);
	~FontRenderer();

	void setFont(Animation *font);
	void computeSize(const Common::String &origText, int16 *retX, int16 *retY);
	void renderText(int16 x, int16 y, const Common::String &origText, int32 mode);
	void renderMultiLineText(int16 x, int16 y, const Common::String &origText, int32 mode);
	void setFontColorByCharacter(int32 characterId);
	void setFontColor(int32 fontColor1, int32 fontColor2, int32 fontColor3);
protected:
	Animation *_currentFont;
	ToonEngine *_vm;
	byte _currentFontColor[4];
	byte textToFont(byte c);
};

} // End of namespace Toon

#endif
