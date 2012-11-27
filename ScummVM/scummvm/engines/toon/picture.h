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

#ifndef TOON_PICTURE_H
#define TOON_PICTURE_H

#include "common/stream.h"
#include "common/array.h"
#include "common/func.h"
#include "common/str.h"

#include "toon/toon.h"

namespace Toon {

class ToonEngine;

class Picture {
public:
	Picture(ToonEngine *vm);
	~Picture();

	bool loadPicture(const Common::String &file);
	void setupPalette();
	void draw(Graphics::Surface &surface, int16 x, int16 y, int16 dx, int16 dy);
	void drawWithRectList(Graphics::Surface& surface, int16 x, int16 y, int16 dx, int16 dy, Common::Array<Common::Rect>& rectArray);
	void drawMask(Graphics::Surface &surface, int16 x, int16 y, int16 dx, int16 dy);
	void drawLineOnMask(int16 x, int16 y, int16 x2, int16 y2, bool walkable);
	void floodFillNotWalkableOnMask(int16 x, int16 y);
	uint8 getData(int16 x, int16 y);
	uint8 *getDataPtr() { return _data; }
	int16 getWidth() const { return _width; }
	int16 getHeight() const { return _height; }

protected:
	int16 _width;
	int16 _height;
	uint8 *_data;
	uint8 *_palette; // need to be copied at 3-387
	int32 _paletteEntries;
	bool _useFullPalette;

	ToonEngine *_vm;
};

} // End of namespace Toon

#endif
