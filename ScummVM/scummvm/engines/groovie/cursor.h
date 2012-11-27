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

#ifndef GROOVIE_CURSOR_H
#define GROOVIE_CURSOR_H

#include "common/array.h"
#include "common/system.h"

namespace Common {
class MacResManager;
}

namespace Groovie {

class Cursor {
public:
	virtual ~Cursor() {}
	uint16 getFrames() { return _numFrames; }
	virtual void enable() = 0;
	virtual void showFrame(uint16 frame) = 0;

protected:
	uint16 _width;
	uint16 _height;
	uint16 _numFrames;
};

class GrvCursorMan {
public:
	GrvCursorMan(OSystem *system);
	virtual ~GrvCursorMan();

	virtual void show(bool visible);
	virtual void animate();
	virtual void setStyle(uint8 newStyle);
	virtual uint8 getStyle();

protected:
	OSystem *_syst;

	// Animation variables
	uint8 _lastFrame;
	uint32 _lastTime;

	// Styles
	Common::Array<Cursor *> _cursors;
	uint8 _current;
	Cursor *_cursor;
};

class GrvCursorMan_t7g : public GrvCursorMan {
public:
	GrvCursorMan_t7g(OSystem *system, Common::MacResManager *macResFork = 0);
	~GrvCursorMan_t7g();

private:
	// Styles data
	static const uint _cursorImg[];
	static const uint _cursorPal[];

	// Cursors data
	Common::Array<byte *> _images;
	Common::Array<byte *> _palettes;

	// Loading functions
	byte *loadImage(Common::SeekableReadStream &file);
	byte *loadPalette(Common::SeekableReadStream &file);
};

class GrvCursorMan_v2 : public GrvCursorMan {
public:
	GrvCursorMan_v2(OSystem *system);
	~GrvCursorMan_v2();

	void setStyle(uint8 newStyle);
};

} // End of Groovie namespace

#endif // GROOVIE_CURSOR_H
