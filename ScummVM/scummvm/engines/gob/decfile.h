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

#ifndef GOB_DECFILE_H
#define GOB_DECFILE_H

#include "common/system.h"

namespace Common {
	class String;
	class SeekableSubReadStreamEndian;
}

namespace Gob {

class GobEngine;
class Surface;
class CMPFile;

/** A DEC file, describing a "decal" (background).
 *
 *  Used in hardcoded "actiony" parts of gob games.
 *  The principle is similar to a Static in Scenery (see scenery.cpp), but
 *  instead of referencing indices in the sprites array, DECs reference sprites
 *  directly by filename.
 */
class DECFile {
public:
	DECFile(GobEngine *vm, const Common::String &fileName,
	        uint16 width, uint16 height, uint8 bpp = 1);
	~DECFile();

	/** Draw the background, including all default layer parts. */
	void draw(Surface &dest) const;

	/** Explicitly draw the backdrop. */
	void drawBackdrop(Surface &dest) const;

	/** Explicitly draw a layer part. */
	void drawLayer(Surface &dest, uint16 layer, uint16 part,
	               uint16 x, uint16 y, int32 transp = -1) const;

private:
	struct Part {
		uint8 layer;
		uint8 part;

		uint16 x;
		uint16 y;
		bool transp;
	};

	typedef Common::Array<CMPFile *> LayerArray;
	typedef Common::Array<Part>      PartArray;

	GobEngine *_vm;

	uint16 _width;
	uint16 _height;
	uint8  _bpp;

	byte _hasPadding;

	CMPFile *_backdrop;

	LayerArray _layers;
	PartArray  _parts;


	void load(Common::SeekableSubReadStreamEndian &dec, const Common::String &fileName);

	void loadBackdrop(Common::SeekableSubReadStreamEndian &dec);

	CMPFile *loadLayer(Common::SeekableSubReadStreamEndian &dec);

	void loadParts(Common::SeekableSubReadStreamEndian &dec);
	void loadPart(Part &part, Common::SeekableSubReadStreamEndian &dec);
};

} // End of namespace Gob

#endif // GOB_DECFILE_H
