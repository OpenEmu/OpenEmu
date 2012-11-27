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

#ifndef MOHAWK_BITMAP_H
#define MOHAWK_BITMAP_H

#include "mohawk/graphics.h"

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/array.h"
#include "graphics/surface.h"

namespace Mohawk {

class MohawkSurface;

enum BitmapFormat {
	kBitsPerPixel1 = 0x0000,
	kBitsPerPixel4 = 0x0001,
	kBitsPerPixel8 = 0x0002,
	kBitsPerPixel16 = 0x0003,
	kBitsPerPixel24 = 0x0004,
	kBitsPerPixelMask = 0x0007,
	kBitmapHasCLUT = 0x0008,
	kDrawMASK = 0x00f0,
	kDrawRaw = 0x0000,
	kDrawRLE8 = 0x0010,
	kDrawMSRLE8 = 0x0020,
	kDrawRLE = 0x0030,
	kPackMASK = 0x0f00,
	kPackNone = 0x0000,
	kPackLZ = 0x0100,
	kPackLZ1 = 0x0200,
	kPackRiven = 0x0400,
	kPackXDec = 0x0f00,
	kFlagMASK = 0xf000,
	kFlag16_80X86 = 0x1000, // 16 bit pixel data has been converted to 80X86 format
	kFlag24_MAC = 0x1000 // 24 bit pixel data has been converted to MAC 32 bit format
};

enum OldBitmapFormat {
	kOldPackLZ = 0x0020,
	kOldDrawRLE8 = 0x0100
};

struct BitmapHeader {
	uint16 width;
	uint16 height;
	int16 bytesPerRow;
	uint16 format;

	struct ColorTable {
		uint16 tableSize;
		byte rgbBits;
		byte colorCount;
		byte* palette;   // In 8bpp only
	} colorTable;
};

class MohawkBitmap {
public:
	MohawkBitmap();
	virtual ~MohawkBitmap();

	virtual MohawkSurface *decodeImage(Common::SeekableReadStream *stream);
	Common::Array<MohawkSurface *> decodeImages(Common::SeekableReadStream *stream);

protected:
	BitmapHeader _header;
	virtual byte getBitsPerPixel();

	void decodeImageData(Common::SeekableReadStream *stream);

	// The actual LZ decoder
	static Common::SeekableReadStream *decompressLZ(Common::SeekableReadStream *stream, uint32 uncompressedSize);

	// The current data stream
	Common::SeekableReadStream *_data;

	// Create the output surface
	Graphics::Surface *createSurface(uint16 width, uint16 height);

	// Draw Functions
	void drawRLE8(Graphics::Surface *surface, bool isLE);
	void drawRaw(Graphics::Surface *surface);
	void drawRLE8(Graphics::Surface *surface) { return drawRLE8(surface, false); }

private:
	// Unpack Functions
	void unpackRaw();
	void unpackLZ();
	void unpackRiven();

	// An unpacker
	struct PackFunction {
		uint16 flag;
		const char *name;
		void (MohawkBitmap::*func)();
	};

	// A drawer
	struct DrawFunction {
		uint16 flag;
		const char *name;
		void (MohawkBitmap::*func)(Graphics::Surface *surface);
	};

	// Unpack/Draw maps
	const PackFunction *_packTable;
	int _packTableSize;
	const DrawFunction *_drawTable;
	int _drawTableSize;

	// Unpack/Draw helpers
	const char *getPackName();
	void unpackImage();
	const char *getDrawName();
	void drawImage(Graphics::Surface *surface);

	// Riven Decoding
	void handleRivenSubcommandStream(byte count, byte *&dst);
};

#ifdef ENABLE_MYST

// Myst uses a different image format than that of other Mohawk games.
// It essentially uses a Windows bitmap with the LZ encoding from the
// Mohawk Bitmap format.
class MystBitmap : public MohawkBitmap {
public:
	MystBitmap() : MohawkBitmap() {}
	~MystBitmap() {}

	MohawkSurface *decodeImage(Common::SeekableReadStream *stream);

protected:
	byte getBitsPerPixel() { return _bitsPerPixel; }

private:
	uint16 _bitsPerPixel;
};

#endif

class LivingBooksBitmap_v1 : public MohawkBitmap {
public:
	LivingBooksBitmap_v1() : MohawkBitmap() {}
	~LivingBooksBitmap_v1() {}

	MohawkSurface *decodeImage(Common::SeekableReadStream *stream);

protected:
	byte getBitsPerPixel() { return 8; }
};

class DOSBitmap : public MohawkBitmap {
public:
	DOSBitmap() : MohawkBitmap() {}
	~DOSBitmap() {}

	MohawkSurface *decodeImage(Common::SeekableReadStream *stream);

protected:
	byte getBitsPerPixel() { return ((_header.format & 0x30) >> 4) + 1; }

private:
	void expandMonochromePlane(Graphics::Surface *surface, Common::SeekableReadStream *rawStream);
	void expandEGAPlanes(Graphics::Surface *surface, Common::SeekableReadStream *rawStream);
};

} // End of namespace Mohawk

#endif
