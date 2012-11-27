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

#ifndef GRAPHICS_DECODERS_IMAGEDECODER_H
#define GRAPHICS_DECODERS_IMAGEDECODER_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Common{
class SeekableReadStream;
}

namespace Graphics {

struct PixelFormat;
struct Surface;

/**
 * A representation of an image decoder that maintains ownership of the surface
 * and palette it decodes to.
 */
class ImageDecoder {
public:
	virtual ~ImageDecoder() {}

	/**
	 * Load an image from the specified stream
	 *
	 * @param stream the input stream
	 * @return whether loading the file succeeded
	 * @see getSurface
	 * @see getPalette
	 */
	virtual bool loadStream(Common::SeekableReadStream &stream) = 0;

	/**
	 * Destroy this decoder's surface and palette
	 */
	virtual void destroy() = 0;

	/**
	 * Get the decoded surface
	 *
	 * This surface is owned by this ImageDecoder and will remain valid
	 * until destroy() or loadStream() is called, or until this ImageDecoder's
	 * destructor is called.
	 *
	 * @return the decoded surface, or 0 if no surface is present
	 */
	virtual const Surface *getSurface() const = 0;

	/**
	 * Get the decoded palette
	 *
	 * This palette is owned by this ImageDecoder and will remain valid
	 * until destroy() or loadStream() is called, or until this ImageDecoder's
	 * destructor is called.
	 *
	 * The palette's format is the same as PaletteManager's palette
	 * (interleaved RGB values).
	 *
	 * @return the decoded palette, or undefined if no palette is present
	 */
	virtual const byte *getPalette() const { return 0; }

	/**
	 * Query if the decoded image has a palette.
	 */
	virtual bool hasPalette() const { return getPaletteColorCount() != 0; }

	/** Return the starting index of the palette. */
	virtual byte getPaletteStartIndex() const { return 0; }
	/** Return the number of colors in the palette. */
	virtual uint16 getPaletteColorCount() const { return 0; }
};

} // End of namespace Graphics

#endif
