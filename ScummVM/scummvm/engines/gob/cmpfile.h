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

#ifndef GOB_CMPFILE_H
#define GOB_CMPFILE_H

#include "common/system.h"
#include "common/array.h"

namespace Common {
	class String;
	class SeekableReadStream;
}

namespace Gob {

class GobEngine;
class Surface;
class RXYFile;

/** A CMP file, containing a sprite.
 *
 *  Used in hardcoded "actiony" parts of gob games.
 */
class CMPFile {
public:
	CMPFile(GobEngine *vm, const Common::String &baseName,
	        uint16 width, uint16 height, uint8 bpp = 1);
	CMPFile(GobEngine *vm, const Common::String &cmpFile, const Common::String &rxyFile,
	        uint16 width, uint16 height, uint8 bpp = 1);
	CMPFile(GobEngine *vm, Common::SeekableReadStream &cmp, Common::SeekableReadStream &rxy,
	        uint16 width, uint16 height, uint8 bpp = 1);
	CMPFile(GobEngine *vm, Common::SeekableReadStream &cmp,
	        uint16 width, uint16 height, uint8 bpp = 1);
	~CMPFile();

	bool empty() const;

	uint16 getSpriteCount() const;

	bool getCoordinates(uint16 sprite, uint16 &left, uint16 &top, uint16 &right, uint16 &bottom) const;

	uint16 getWidth (uint16 sprite) const;
	uint16 getHeight(uint16 sprite) const;

	void getMaxSize(uint16 &width, uint16 &height) const;

	void draw(Surface &dest, uint16 sprite, uint16 x, uint16 y, int32 transp = -1) const;
	void draw(Surface &dest, uint16 left, uint16 top, uint16 right, uint16 bottom,
	          uint16 x, uint16 y, int32 transp = -1) const;

	uint16 addSprite(uint16 left, uint16 top, uint16 right, uint16 bottom);

	void recolor(uint8 from, uint8 to);

private:
	GobEngine *_vm;

	uint16 _width;
	uint16 _height;
	uint16 _bpp;

	uint16 _maxWidth;
	uint16 _maxHeight;

	Surface *_surface;
	RXYFile *_coordinates;

	void loadCMP(const Common::String &cmp);
	void loadRXY(const Common::String &rxy);

	void loadCMP(Common::SeekableReadStream &cmp);
	void loadRXY(Common::SeekableReadStream &rxy);

	void createRXY();
	void createSurface();
};

} // End of namespace Gob

#endif // GOB_CMPFILE_H
