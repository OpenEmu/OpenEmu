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

#if !defined(SCUMM_NUT_RENDERER_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_NUT_RENDERER_H

#include "common/file.h"
#include "graphics/surface.h"

namespace Scumm {

class ScummEngine;

class NutRenderer {
protected:

	enum {
		kDefaultTransparentColor = 0,
		kSmush44TransparentColor = 2
	};

	ScummEngine *_vm;
	int _numChars;
	int _maxCharSize;
	byte *_charBuffer;
	byte *_decodedData;
	byte *_paletteMap;
	byte _bpp;
	byte _palette[16];
	struct {
		uint16 width;
		uint16 height;
		byte *src;
		byte transparency;
	} _chars[256];

	void codec1(byte *dst, const byte *src, int width, int height, int pitch);
	void codec21(byte *dst, const byte *src, int width, int height, int pitch);

	void loadFont(const char *filename);
	byte *unpackChar(byte c);

public:
	NutRenderer(ScummEngine *vm, const char *filename);
	virtual ~NutRenderer();
	int getNumChars() const { return _numChars; }

	void drawFrame(byte *dst, int c, int x, int y);
	void drawChar(const Graphics::Surface &s, byte c, int x, int y, byte color);
	void draw2byte(const Graphics::Surface &s, int c, int x, int y, byte color);

	int getCharWidth(byte c) const;
	int getCharHeight(byte c) const;
};

} // End of namespace Scumm

#endif
