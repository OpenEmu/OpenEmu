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

/*
 * Bitmap decoder used in engines:
 *  - parallaction
 *  - saga
 */

#ifndef GRAPHICS_IFF_H
#define GRAPHICS_IFF_H

#include "common/stream.h"

namespace Graphics {

struct Surface;


struct BMHD {
	uint16 width, height;
	uint16 x, y;
	byte depth;
	byte masking;
	byte pack;
	byte flags;
	uint16 transparentColor;
	byte xAspect, yAspect;
	uint16 pageWidth, pageHeight;

	BMHD() {
		memset(this, 0, sizeof(*this));
	}

	void load(Common::ReadStream *stream);
};


struct ILBMDecoder {
	/**
	 * ILBM header data, necessary for loadBitmap()
	 */
	Graphics::BMHD	_header;

	/**
	 * Available decoding modes for loadBitmap().
	 */
	enum {
		ILBM_UNPACK_PLANES = 0xFF,		///< Decode all bitplanes, and map 1 pixel to 1 byte.
		ILBM_PACK_PLANES   = 0x100,		///< Request unpacking, used as a mask with below options.

		ILBM_1_PLANES      = 1,									///< Decode only the first bitplane, don't pack.
		ILBM_1_PACK_PLANES = ILBM_1_PLANES | ILBM_PACK_PLANES, 	///< Decode only the first bitplane, pack 8 pixels in 1 byte.
		ILBM_2_PLANES      = 2,									///< Decode first 2 bitplanes, don't pack.
		ILBM_2_PACK_PLANES = ILBM_2_PLANES | ILBM_PACK_PLANES,	///< Decode first 2 bitplanes, pack 4 pixels in 1 byte.
		ILBM_3_PLANES      = 3,									///< Decode first 3 bitplanes, don't pack.
		ILBM_4_PLANES      = 4,									///< Decode first 4 bitplanes, don't pack.
		ILBM_4_PACK_PLANES = ILBM_4_PLANES | ILBM_PACK_PLANES,	///< Decode first 4 bitplanes, pack 2 pixels in 1 byte.
		ILBM_5_PLANES      = 5,									///< Decode first 5 bitplanes, don't pack.
		ILBM_8_PLANES      = 8									///< Decode all 8 bitplanes.
	};

	/**
	 * Fills the _header member from the given stream.
	 */
	void loadHeader(Common::ReadStream *stream);

	/**
	 * Loads and unpacks the ILBM bitmap data from the stream into the buffer.
	 * The functions assumes the buffer is large enough to contain all data.
	 * The caller controls how data should be packed by choosing mode from
	 * the enum above.
	 */
	void loadBitmap(uint32 mode, byte *buffer, Common::ReadStream *stream);

	/**
	 * Converts from bitplanar to chunky representation. Intended for internal
	 * usage, but you can be (ab)use it from client code if you know what you
	 * are doing.
	 */
	void planarToChunky(byte *out, uint32 width, byte *in, uint32 planeWidth, uint32 nPlanes, bool packPlanes);
};



/**
 * Handles PBM subtype of IFF FORM files
 */
void decodePBM(Common::ReadStream &input, Surface &surface, byte *colors);


/**
 * Decode a given PackBits encoded stream.
 *
 * PackBits is an RLE compression algorithm introduced by Apple. It is also
 * used to encode ILBM and PBM subtypes of IFF files, and some flavors of
 * TIFF.
 *
 * As there is no compression across row boundaries in the above formats,
 * read() will extract a *new* line on each call, discarding any alignment
 * or padding.
 */
class PackBitsReadStream : public Common::ReadStream {

protected:
	Common::ReadStream *_input;

public:
	PackBitsReadStream(Common::ReadStream &input);
	~PackBitsReadStream();

	virtual bool eos() const;

	uint32 read(void *dataPtr, uint32 dataSize);
};

} // End of namespace Graphics

#endif
