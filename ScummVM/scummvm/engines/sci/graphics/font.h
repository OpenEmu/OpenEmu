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

#ifndef SCI_GRAPHICS_FONT_H
#define SCI_GRAPHICS_FONT_H

#include "sci/graphics/helpers.h"

namespace Sci {

class GfxFont {
public:
	GfxFont() {}
	virtual ~GfxFont() {}

	virtual GuiResourceId getResourceId() { return 0; }
	virtual byte getHeight() { return 0; }
	virtual bool isDoubleByte(uint16 chr) { return false; }
	virtual byte getCharWidth(uint16 chr) { return 0; }
	virtual void draw(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput) {}
	virtual void drawToBuffer(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput, byte *buffer, int16 width, int16 height) {}
};


/**
 * Font class, handles loading of font resources and drawing characters to screen
 *  every font resource has its own instance of this class
 */
class GfxFontFromResource : public GfxFont {
public:
	GfxFontFromResource(ResourceManager *resMan, GfxScreen *screen, GuiResourceId resourceId);
	~GfxFontFromResource();

	GuiResourceId getResourceId();
	byte getHeight();
	byte getCharWidth(uint16 chr);
	void draw(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput);
#ifdef ENABLE_SCI32
	// SCI2/2.1 equivalent
	void drawToBuffer(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput, byte *buffer, int16 width, int16 height);
#endif

private:
	byte getCharHeight(uint16 chr);
	byte *getCharData(uint16 chr);

	ResourceManager *_resMan;
	GfxScreen *_screen;

	Resource *_resource;
	GuiResourceId _resourceId;
	byte *_resourceData;

	struct Charinfo {
		byte w, h;
		int16 offset;
	};
	byte _fontHeight;
	uint16 _numChars;
	Charinfo *_chars;
};

} // End of namespace Sci

#endif
