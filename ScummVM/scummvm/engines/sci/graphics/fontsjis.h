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

#ifndef SCI_GRAPHICS_FONTSJIS_H
#define SCI_GRAPHICS_FONTSJIS_H

#include "sci/graphics/helpers.h"

namespace Graphics {
class FontSJIS;
}

namespace Sci {

/**
 * Special Font class, handles SJIS inside sci games, uses ScummVM SJIS support
 */
class GfxFontSjis : public GfxFont {
public:
	GfxFontSjis(GfxScreen *screen, GuiResourceId resourceId);
	~GfxFontSjis();

	GuiResourceId getResourceId();
	byte getHeight();
	bool isDoubleByte(uint16 chr);
	byte getCharWidth(uint16 chr);
	void draw(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput);

private:
	GfxScreen *_screen;
	GuiResourceId _resourceId;

	Graphics::FontSJIS *_commonFont;

	byte _lastForDoubleByteWidth;
	byte _lastForDoubleByteDraw;
};

} // End of namespace Sci

#endif
