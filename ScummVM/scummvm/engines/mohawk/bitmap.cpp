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

#include "mohawk/bitmap.h"

#include "common/debug.h"
#include "common/util.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/decoders/bmp.h"

namespace Mohawk {

#define PACK_COMPRESSION (_header.format & kPackMASK)
#define DRAW_COMPRESSION (_header.format & kDrawMASK)

MohawkBitmap::MohawkBitmap() {
	static const PackFunction packTable[] = {
		{ kPackNone, "Raw", &MohawkBitmap::unpackRaw },
		{ kPackLZ, "LZ", &MohawkBitmap::unpackLZ },
		{ kPackRiven, "Riven", &MohawkBitmap::unpackRiven }
	};

	_packTable = packTable;
	_packTableSize = ARRAYSIZE(packTable);

	static const DrawFunction drawTable[] = {
		{ kDrawRaw, "Raw", &MohawkBitmap::drawRaw },
		{ kDrawRLE8, "RLE8", &MohawkBitmap::drawRLE8 }
	};

	_drawTable = drawTable;
	_drawTableSize = ARRAYSIZE(drawTable);
}

MohawkBitmap::~MohawkBitmap() {
}

void MohawkBitmap::decodeImageData(Common::SeekableReadStream *stream) {
	_data = stream;
	_header.colorTable.palette = NULL;

	// NOTE: Only the bottom 12 bits of width/height/bytesPerRow are
	// considered valid and bytesPerRow has to be an even number.
	_header.width = _data->readUint16BE() & 0x3FFF;
	_header.height = _data->readUint16BE() & 0x3FFF;
	_header.bytesPerRow = _data->readSint16BE() & 0x3FFE;
	_header.format = _data->readUint16BE();

	debug (2, "Decoding Mohawk Bitmap (%dx%d, %dbpp, %s Packing + %s Drawing)", _header.width, _header.height, getBitsPerPixel(), getPackName(), getDrawName());

	if (getBitsPerPixel() != 8 && getBitsPerPixel() != 24)
		error ("Unhandled bpp %d", getBitsPerPixel());

	// Read in the palette if it's here.
	if (_header.format & kBitmapHasCLUT || (PACK_COMPRESSION == kPackRiven && getBitsPerPixel() == 8)) {
		_header.colorTable.tableSize = _data->readUint16BE();
		_header.colorTable.rgbBits = _data->readByte();
		_header.colorTable.colorCount = _data->readByte();
		_header.colorTable.palette = (byte *)malloc(256 * 3);

		for (uint16 i = 0; i < 256; i++) {
			_header.colorTable.palette[i * 3 + 2] = _data->readByte();
			_header.colorTable.palette[i * 3 + 1] = _data->readByte();
			_header.colorTable.palette[i * 3 + 0] = _data->readByte();
		}
	}

	unpackImage();
}

MohawkSurface *MohawkBitmap::decodeImage(Common::SeekableReadStream *stream) {
	decodeImageData(stream);

	Graphics::Surface *surface = createSurface(_header.width, _header.height);
	drawImage(surface);
	delete _data;

	return new MohawkSurface(surface, _header.colorTable.palette);
}

Common::Array<MohawkSurface *> MohawkBitmap::decodeImages(Common::SeekableReadStream *stream) {
	decodeImageData(stream);

	// Some Mohawk games (CSTime, Zoombinis) store 'compound shapes' by
	// packing several sub-images inside the data portion of an image.
	// We take a copy of what we need (since it will be overwritten),
	// and then decodeImage() all these sub-images.
	Common::SeekableReadStream *data = _data;
	int32 startPos = data->pos();
	uint16 count = _header.width;

	Common::Array<uint32> offsets;
	for (uint i = 0; i < count; i++)
		offsets.push_back(data->readUint32BE());

	Common::Array<MohawkSurface *> surfaces;
	for (uint i = 0; i < count; i++) {
		uint32 start = startPos + offsets[i] - 8;
		uint32 end;
		if (i != (uint)count - 1)
			end = startPos + offsets[i + 1] - 8;
		else
			end = data->size();
		Common::SeekableSubReadStream *substream = new Common::SeekableSubReadStream(data, start, end);
		surfaces.push_back(decodeImage(substream));
	}

	delete data;
	return surfaces;
}

Graphics::Surface *MohawkBitmap::createSurface(uint16 width, uint16 height) {
	Graphics::Surface *surface = new Graphics::Surface();
	Graphics::PixelFormat format;
	if (getBitsPerPixel() <= 8)
		format = Graphics::PixelFormat::createFormatCLUT8();
	else
		format = g_system->getScreenFormat();
	surface->create(width, height, format);
	return surface;
}

byte MohawkBitmap::getBitsPerPixel() {
	switch (_header.format & kBitsPerPixelMask) {
	case kBitsPerPixel1:
		return 1;
	case kBitsPerPixel4:
		return 4;
	case kBitsPerPixel8:
		return 8;
	case kBitsPerPixel16:
		return 16;
	case kBitsPerPixel24:
		return 24;
	default:
		error("Unknown bits per pixel");
	}

	return 0;
}

const char *MohawkBitmap::getPackName() {
	for (int i = 0; i < _packTableSize; i++)
		if (PACK_COMPRESSION == _packTable[i].flag)
			return _packTable[i].name;

	return "Unknown";
}

void MohawkBitmap::unpackImage() {
	for (int i = 0; i < _packTableSize; i++)
		if (PACK_COMPRESSION == _packTable[i].flag) {
			(this->*_packTable[i].func)();
			return;
		}

	error("Unknown Pack Compression");
}

const char *MohawkBitmap::getDrawName() {
	for (int i = 0; i < _drawTableSize; i++)
		if (DRAW_COMPRESSION == _drawTable[i].flag)
			return _drawTable[i].name;

	return "Unknown";
}

void MohawkBitmap::drawImage(Graphics::Surface *surface) {
	for (int i = 0; i < _drawTableSize; i++)
		if (DRAW_COMPRESSION == _drawTable[i].flag) {
			(this->*_drawTable[i].func)(surface);
			return;
		}

	error("Unknown Draw Compression");
}

//////////////////////////////////////////
// Raw "Unpacker"
//////////////////////////////////////////

void MohawkBitmap::unpackRaw() {
	// Do nothing :D
}

//////////////////////////////////////////
// LZ Unpacker
//////////////////////////////////////////

#define LEN_BITS		6
#define MIN_STRING		3									// lower limit for string length
#define POS_BITS		(16 - LEN_BITS)
#define MAX_STRING		((1 << LEN_BITS) + MIN_STRING - 1)	// upper limit for string length
#define CBUFFERSIZE		(1 << POS_BITS)						// size of the circular buffer
#define POS_MASK		(CBUFFERSIZE - 1)

Common::SeekableReadStream *MohawkBitmap::decompressLZ(Common::SeekableReadStream *stream, uint32 uncompressedSize) {
	uint16 flags = 0;
	uint32 bytesOut = 0;
	uint16 insertPos = 0;

	// Expand the output buffer to at least the ring buffer size
	uint32 outBufSize = MAX<int>(uncompressedSize, CBUFFERSIZE);

	byte *outputData = (byte *)malloc(outBufSize);
	byte *dst = outputData;
	byte *buf = dst;

	// Clear the buffer to all 0's
	memset(outputData, 0, outBufSize);

	while (stream->pos() < stream->size()) {
		flags >>= 1;

		if (!(flags & 0x100))
			flags = stream->readByte() | 0xff00;

		if (flags & 1) {
			if (++bytesOut > uncompressedSize)
				break;
			*dst++ = stream->readByte();
			if (++insertPos > POS_MASK) {
				insertPos = 0;
				buf += CBUFFERSIZE;
			}
		} else {
			uint16 offLen = stream->readUint16BE();
			uint16 stringLen = (offLen >> POS_BITS) + MIN_STRING;
			uint16 stringPos = (offLen + MAX_STRING) & POS_MASK;

			bytesOut += stringLen;
			if (bytesOut > uncompressedSize)
				stringLen -= bytesOut - uncompressedSize;

			byte *strPtr = buf + stringPos;
			if (stringPos > insertPos) {
				if (bytesOut >= CBUFFERSIZE)
					strPtr -= CBUFFERSIZE;
				else if (stringPos + stringLen > POS_MASK) {
					for (uint16 k = 0; k < stringLen; k++) {
						*dst++ = *strPtr++;
						if (++stringPos > POS_MASK) {
							stringPos = 0;
							strPtr = outputData;
						}
					}
					insertPos = (insertPos + stringLen) & POS_MASK;
					if (bytesOut >= uncompressedSize)
						break;
					continue;
				}
			}

			insertPos += stringLen;

			if (insertPos > POS_MASK) {
				insertPos &= POS_MASK;
				buf += CBUFFERSIZE;
			}

			for (uint16 k = 0; k < stringLen; k++)
				*dst++ = *strPtr++;

			if (bytesOut >= uncompressedSize)
				break;
		}
	}

	return new Common::MemoryReadStream(outputData, uncompressedSize, DisposeAfterUse::YES);
}

void MohawkBitmap::unpackLZ() {
	uint32 uncompressedSize = _data->readUint32BE();
	/* uint32 compressedSize = */ _data->readUint32BE();
	uint16 dictSize = _data->readUint16BE();

	// We only support the buffer size of 0x400
	if (dictSize != CBUFFERSIZE)
		error("Unsupported dictionary size of %04x", dictSize);

	// Now go and decompress the data
	Common::SeekableReadStream *decompressedData = decompressLZ(_data, uncompressedSize);
	delete _data;
	_data = decompressedData;
}

//////////////////////////////////////////
// Riven Unpacker
//////////////////////////////////////////

void MohawkBitmap::unpackRiven() {
	_data->readUint32BE(); // Unknown, the number is close to bytesPerRow * height. Could be bufSize.

	byte *uncompressedData = (byte *)malloc(_header.bytesPerRow * _header.height);
	byte *dst = uncompressedData;

	while (!_data->eos() && dst < (uncompressedData + _header.bytesPerRow * _header.height)) {
		byte cmd = _data->readByte();
		debug (8, "Riven Pack Command %02x", cmd);

		if (cmd == 0x00) {                       // End of stream
			break;
		} else if (cmd >= 0x01 && cmd <= 0x3f) { // Simple Pixel Duplet Output
			for (byte i = 0; i < cmd; i++) {
				*dst++ = _data->readByte();
				*dst++ = _data->readByte();
			}
		} else if (cmd >= 0x40 && cmd <= 0x7f) { // Simple Repetition of last 2 pixels (cmd - 0x40) times
			byte pixel[] = { *(dst - 2), *(dst - 1) };

			for (byte i = 0; i < (cmd - 0x40); i++) {
				*dst++ = pixel[0];
				*dst++ = pixel[1];
			}
		} else if (cmd >= 0x80 && cmd <= 0xbf) { // Simple Repetition of last 4 pixels (cmd - 0x80) times
			byte pixel[] = { *(dst - 4), *(dst - 3), *(dst - 2), *(dst - 1) };

			for (byte i = 0; i < (cmd - 0x80); i++) {
				*dst++ = pixel[0];
				*dst++ = pixel[1];
				*dst++ = pixel[2];
				*dst++ = pixel[3];
			}
		} else {                                 // Subcommand Stream of (cmd - 0xc0) subcommands
			handleRivenSubcommandStream(cmd - 0xc0, dst);
		}
	}

	delete _data;
	_data = new Common::MemoryReadStream(uncompressedData, _header.bytesPerRow * _header.height, DisposeAfterUse::YES);
}

static byte getLastTwoBits(byte c) {
	return (c & 0x03);
}

static byte getLastThreeBits(byte c) {
	return (c & 0x07);
}

static byte getLastFourBits(byte c) {
	return (c & 0x0f);
}

#define B_BYTE()				\
	*dst = _data->readByte();	\
	dst++

#define B_LASTDUPLET()			\
	*dst = *(dst - 2);			\
	dst++

#define B_LASTDUPLET_PLUS_M()	\
	*dst = *(dst - 2) + m;		\
	dst++

#define B_LASTDUPLET_MINUS_M()	\
	*dst = *(dst - 2) - m;		\
	dst++

#define B_LASTDUPLET_PLUS(m)	\
	*dst = *(dst - 2) + (m);	\
	dst++

#define B_LASTDUPLET_MINUS(m)	\
	*dst = *(dst - 2) - (m);	\
	dst++

#define B_PIXEL_MINUS(m)		\
	*dst = *(dst - (m));		\
	dst++

#define B_NDUPLETS(n)													\
	uint16 m1 = ((getLastTwoBits(cmd) << 8) + _data->readByte());		\
		for (uint16 j = 0; j < (n); j++) {								\
			*dst = *(dst - m1);											\
			dst++;														\
		}																\
		void dummyFuncToAllowTrailingSemicolon()



void MohawkBitmap::handleRivenSubcommandStream(byte count, byte *&dst) {
	for (byte i = 0; i < count; i++) {
		byte cmd = _data->readByte();
		uint16 m = getLastFourBits(cmd);
		debug (9, "Riven Pack Subcommand %02x", cmd);

		// Notes: p = value of the next byte, m = last four bits of the command

		// Arithmetic operations
		if (cmd >= 0x01 && cmd <= 0x0f) {
			// Repeat duplet at relative position of -m duplets
			B_PIXEL_MINUS(m * 2);
			B_PIXEL_MINUS(m * 2);
		} else if (cmd == 0x10) {
			// Repeat last duplet, but set the value of the second pixel to p
			B_LASTDUPLET();
			B_BYTE();
		} else if (cmd >= 0x11 && cmd <= 0x1f) {
			// Repeat last duplet, but set the value of the second pixel to the value of the -m pixel
			B_LASTDUPLET();
			B_PIXEL_MINUS(m);
		} else if (cmd >= 0x20 && cmd <= 0x2f) {
			// Repeat last duplet, but add x to second pixel
			B_LASTDUPLET();
			B_LASTDUPLET_PLUS_M();
		} else if (cmd >= 0x30 && cmd <= 0x3f) {
			// Repeat last duplet, but subtract x from second pixel
			B_LASTDUPLET();
			B_LASTDUPLET_MINUS_M();
		} else if (cmd == 0x40) {
			// Repeat last duplet, but set the value of the first pixel to p
			B_BYTE();
			B_LASTDUPLET();
		} else if (cmd >= 0x41 && cmd <= 0x4f) {
			// Output pixel at relative position -m, then second pixel of last duplet
			B_PIXEL_MINUS(m);
			B_LASTDUPLET();
		} else if (cmd == 0x50) {
			// Output two absolute pixel values, p1 and p2
			B_BYTE();
			B_BYTE();
		} else if (cmd >= 0x51 && cmd <= 0x57) {
			// Output pixel at relative position -m, then absolute pixel value p
			// m is the last 3 bits of cmd here, not last 4
			B_PIXEL_MINUS(getLastThreeBits(cmd));
			B_BYTE();
		} else if (cmd >= 0x59 && cmd <= 0x5f) {
			// Output absolute pixel value p, then pixel at relative position -m
			// m is the last 3 bits of cmd here, not last 4
			B_BYTE();
			B_PIXEL_MINUS(getLastThreeBits(cmd));
		} else if (cmd >= 0x60 && cmd <= 0x6f) {
			// Output absolute pixel value p, then (second pixel of last duplet) + x
			B_BYTE();
			B_LASTDUPLET_PLUS_M();
		} else if (cmd >= 0x70 && cmd <= 0x7f) {
			// Output absolute pixel value p, then (second pixel of last duplet) - x
			B_BYTE();
			B_LASTDUPLET_MINUS_M();
		} else if (cmd >= 0x80 && cmd <= 0x8f) {
			// Repeat last duplet adding x to the first pixel
			B_LASTDUPLET_PLUS_M();
			B_LASTDUPLET();
		} else if (cmd >= 0x90 && cmd <= 0x9f) {
			// Output (first pixel of last duplet) + x, then absolute pixel value p
			B_LASTDUPLET_PLUS_M();
			B_BYTE();
		} else if (cmd == 0xa0) {
			// Repeat last duplet, adding first 4 bits of the next byte
			// to first pixel and last 4 bits to second
			byte pattern = _data->readByte();
			B_LASTDUPLET_PLUS(pattern >> 4);
			B_LASTDUPLET_PLUS(getLastFourBits(pattern));
		} else if (cmd == 0xb0) {
			// Repeat last duplet, adding first 4 bits of the next byte
			// to first pixel and subtracting last 4 bits from second
			byte pattern = _data->readByte();
			B_LASTDUPLET_PLUS(pattern >> 4);
			B_LASTDUPLET_MINUS(getLastFourBits(pattern));
		} else if (cmd >= 0xc0 && cmd <= 0xcf) {
			// Repeat last duplet subtracting x from first pixel
			B_LASTDUPLET_MINUS_M();
			B_LASTDUPLET();
		} else if (cmd >= 0xd0 && cmd <= 0xdf) {
			// Output (first pixel of last duplet) - x, then absolute pixel value p
			B_LASTDUPLET_MINUS_M();
			B_BYTE();
		} else if (cmd == 0xe0) {
			// Repeat last duplet, subtracting first 4 bits of the next byte
			// to first pixel and adding last 4 bits to second
			byte pattern = _data->readByte();
			B_LASTDUPLET_MINUS(pattern >> 4);
			B_LASTDUPLET_PLUS(getLastFourBits(pattern));
		} else if (cmd == 0xf0 || cmd == 0xff) {
			// Repeat last duplet, subtracting first 4 bits from the next byte
			// to first pixel and last 4 bits from second
			byte pattern = _data->readByte();
			B_LASTDUPLET_MINUS(pattern >> 4);
			B_LASTDUPLET_MINUS(getLastFourBits(pattern));

		// Repeat operations
		// Repeat n duplets from relative position -m (given in pixels, not duplets).
		// If r is 0, another byte follows and the last pixel is set to that value
		} else if (cmd >= 0xa4 && cmd <= 0xa7) {
			B_NDUPLETS(3);
			B_BYTE();
		} else if (cmd >= 0xa8 && cmd <= 0xab) {
			B_NDUPLETS(4);
		} else if (cmd >= 0xac && cmd <= 0xaf) {
			B_NDUPLETS(5);
			B_BYTE();
		} else if (cmd >= 0xb4 && cmd <= 0xb7) {
			B_NDUPLETS(6);
		} else if (cmd >= 0xb8 && cmd <= 0xbb) {
			B_NDUPLETS(7);
			B_BYTE();
		} else if (cmd >= 0xbc && cmd <= 0xbf) {
			B_NDUPLETS(8);
		} else if (cmd >= 0xe4 && cmd <= 0xe7) {
			B_NDUPLETS(9);
			B_BYTE();
		} else if (cmd >= 0xe8 && cmd <= 0xeb) {
			B_NDUPLETS(10); // 5 duplets
		} else if (cmd >= 0xec && cmd <= 0xef) {
			B_NDUPLETS(11);
			B_BYTE();
		} else if (cmd >= 0xf4 && cmd <= 0xf7) {
			B_NDUPLETS(12);
		} else if (cmd >= 0xf8 && cmd <= 0xfb) {
			B_NDUPLETS(13);
			B_BYTE();
		} else if (cmd == 0xfc) {
			byte b1 = _data->readByte();
			byte b2 = _data->readByte();
			uint16 m1 = ((getLastTwoBits(b1) << 8) + b2);

			for (uint16 j = 0; j < ((b1 >> 3) + 1); j++) { // one less iteration
				B_PIXEL_MINUS(m1);
				B_PIXEL_MINUS(m1);
			}

			// last iteration
			B_PIXEL_MINUS(m1);

			if ((b1 & (1 << 2)) == 0) {
				B_BYTE();
			} else {
				B_PIXEL_MINUS(m1);
			}
		} else
			warning("Unknown Riven Pack Subcommand 0x%02x", cmd);
	}
}

//////////////////////////////////////////
// Raw Drawer
//////////////////////////////////////////

void MohawkBitmap::drawRaw(Graphics::Surface *surface) {
	assert(surface);

	for (uint16 y = 0; y < _header.height; y++) {
		if (getBitsPerPixel() == 24) {
			Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();

			for (uint16 x = 0; x < _header.width; x++) {
				byte b = _data->readByte();
				byte g = _data->readByte();
				byte r = _data->readByte();
				if (surface->format.bytesPerPixel == 2)
					*((uint16 *)surface->getBasePtr(x, y)) = pixelFormat.RGBToColor(r, g, b);
				else
					*((uint32 *)surface->getBasePtr(x, y)) = pixelFormat.RGBToColor(r, g, b);
			}

			_data->skip(_header.bytesPerRow - _header.width * 3);
		} else {
			_data->read((byte *)surface->pixels + y * _header.width, _header.width);
			_data->skip(_header.bytesPerRow - _header.width);
		}
	}
}

//////////////////////////////////////////
// RLE8 Drawer
//////////////////////////////////////////

void MohawkBitmap::drawRLE8(Graphics::Surface *surface, bool isLE) {
	// A very simple RLE8 scheme is used as a secondary compression on
	// most images in non-Riven tBMP's.

	assert(surface);

	for (uint16 i = 0; i < _header.height; i++) {
		uint16 rowByteCount = isLE ? _data->readUint16LE() : _data->readUint16BE();
		int32 startPos = _data->pos();
		byte *dst = (byte *)surface->pixels + i * _header.width;
		int16 remaining = _header.width;

		while (remaining > 0) {
			byte code = _data->readByte();
			uint16 runLen = (code & 0x7F) + 1;

			if (runLen > remaining)
				runLen = remaining;

			if (code & 0x80) {
				byte val = _data->readByte();
				memset(dst, val, runLen);
			} else {
				_data->read(dst, runLen);
			}

			dst += runLen;
			remaining -= runLen;
		}

		_data->seek(startPos + rowByteCount);
	}
}

#ifdef ENABLE_MYST

//////////////////////////////////////////
// Myst Bitmap Decoder
//////////////////////////////////////////

MohawkSurface *MystBitmap::decodeImage(Common::SeekableReadStream *stream) {
	uint32 uncompressedSize = stream->readUint32LE();
	Common::SeekableReadStream *bmpStream = decompressLZ(stream, uncompressedSize);
	delete stream;

	Graphics::BitmapDecoder bitmapDecoder;
	if (!bitmapDecoder.loadStream(*bmpStream))
		error("Could not decode Myst bitmap");

	const Graphics::Surface *bmpSurface = bitmapDecoder.getSurface();
	Graphics::Surface *newSurface = 0;

	if (bmpSurface->format.bytesPerPixel == 1) {
		_bitsPerPixel = 8;
		newSurface = new Graphics::Surface();
		newSurface->copyFrom(*bmpSurface);
	} else {
		_bitsPerPixel = 24;
		newSurface = bmpSurface->convertTo(g_system->getScreenFormat());
	}

	// Copy the palette to one of our own
	byte *newPal = 0;

	if (bitmapDecoder.hasPalette()) {
		const byte *palette = bitmapDecoder.getPalette();
		newPal = (byte *)malloc(256 * 3);
		memcpy(newPal, palette, 256 * 3);
	}

	delete bmpStream;

	return new MohawkSurface(newSurface, newPal);
}

#endif

MohawkSurface *LivingBooksBitmap_v1::decodeImage(Common::SeekableReadStream *stream) {
	Common::SeekableSubReadStreamEndian *endianStream = (Common::SeekableSubReadStreamEndian *)stream;

	// 12 bytes header for the image
	_header.format = endianStream->readUint16();
	_header.bytesPerRow = endianStream->readUint16();
	_header.width = endianStream->readUint16();
	_header.height = endianStream->readUint16();
	int offsetX = endianStream->readSint16();
	int offsetY = endianStream->readSint16();

	debug(7, "Decoding Old Mohawk Bitmap (%dx%d, %d bytesPerRow, %04x Format)", _header.width, _header.height, _header.bytesPerRow, _header.format);
	debug(7, "Offset X = %d, Y = %d", offsetX, offsetY);

	bool leRLE8 = false;

	if ((_header.format & 0xf0) == kOldPackLZ) {
		// 12 bytes header for the compressed data
		uint32 uncompressedSize = endianStream->readUint32();
		uint32 compressedSize = endianStream->readUint32();
		uint16 posBits = endianStream->readUint16();
		uint16 lengthBits = endianStream->readUint16();

		if (compressedSize != (uint32)endianStream->size() - 24)
			error("More bytes (%d) remaining in stream than header says there should be (%d)", endianStream->size() - 24, compressedSize);

		// These two errors are really just sanity checks and should never go off
		if (posBits != POS_BITS)
			error("Position bits modified to %d", posBits);
		if (lengthBits != LEN_BITS)
			error("Length bits modified to %d", lengthBits);

		_data = decompressLZ(stream, uncompressedSize);

		if (endianStream->pos() != endianStream->size())
			error("LivingBooksBitmap_v1 decompression failed");
	} else {
		if ((_header.format & 0xf0) != 0)
			error("Tried to use unknown LivingBooksBitmap_v1 compression (format %02x)", _header.format & 0xf0);

		// This is so nasty on so many levels. The original Windows LZ decompressor for the
		// Living Books v1 games had knowledge of the underlying RLE8 data. While going
		// through the LZ data, it would byte swap the RLE8 length fields to make them LE.
		// This is an extremely vile thing and there's no way in hell that I'm doing
		// anything similar. When no LZ compression is used, the underlying RLE8 fields
		// are LE, so we need to set a swap in this condition for LE vs. BE in the RLE8
		// decoder. *sigh*

		if (!endianStream->isBE())
			leRLE8 = true;

		_data = stream;
		stream = NULL;
	}

	Graphics::Surface *surface = createSurface(_header.width, _header.height);

	if ((_header.format & 0xf00) == kOldDrawRLE8)
		drawRLE8(surface, leRLE8);
	else
		drawRaw(surface);

	delete _data;
	delete stream;

	MohawkSurface *mhkSurface = new MohawkSurface(surface);
	mhkSurface->setOffsetX(offsetX);
	mhkSurface->setOffsetY(offsetY);

	return mhkSurface;
}

// Partially based on the Prince of Persia Format Specifications
// See http://sdfg.com.ar/git/?p=fp-git.git;a=blob;f=FP/doc/FormatSpecifications

MohawkSurface *DOSBitmap::decodeImage(Common::SeekableReadStream *stream) {
	_header.height = stream->readUint16LE();
	_header.width = stream->readUint16LE();
	stream->readByte(); // Always 0
	_header.format = stream->readByte();

	debug(2, "Decoding DOS Bitmap (%dx%d, %dbpp, Compression %d)", _header.width, _header.height, getBitsPerPixel(), _header.format & 0xf);

	// All the PoP games seem to have this flag, but at least CSWorld Deluxe doesn't...
	// Perhaps this differentiates between normal bitmap mode and planar mode?
	if (_header.format & 0x80)
		error("Unknown EGA flag?");

	// Calculate the bytes per row
	byte pixelsPerByte = 8 / getBitsPerPixel();
	_header.bytesPerRow = (_header.width + pixelsPerByte - 1) / pixelsPerByte;

	// Only Raw and LZ L/R are supported currently
	// Notice how Broderbund used their same LZ compression for every PC game possibly ever?
	switch (_header.format & 0xf) {
	case 0: // Raw
		_data = stream;
		break;
	case 3: // LZ Left/Right
		_data = decompressLZ(stream, _header.bytesPerRow * _header.height);
		delete stream;
		break;
	case 1: // RLE Left/Right (Used by PoP, haven't seen in a CS game)
	case 2: // RLE Up/Down (Used by PoP, haven't seen in a CS game)
	case 4: // LZ Up/Down (Used by CS America's Past and CS Space)
		error("Unhandled DOS bitmap compression %d", _header.format & 0xf);
		break;
	default:
		error("Unknown DOS bitmap compression %d", _header.format & 0xf);
	}

	Graphics::Surface *surface = createSurface(_header.width, _header.height);
	memset(surface->pixels, 0, _header.width * _header.height);

	// Expand the <8bpp data to one byte per pixel
	switch (getBitsPerPixel()) {
	case 1:
		expandMonochromePlane(surface, _data);
		break;
	case 4:
		expandEGAPlanes(surface, _data);
		break;
	default:
		error("Unhandled %dbpp", getBitsPerPixel());
	}

	delete _data;

	return new MohawkSurface(surface);
}

void DOSBitmap::expandMonochromePlane(Graphics::Surface *surface, Common::SeekableReadStream *rawStream) {
	assert(surface->format.bytesPerPixel == 1);

	byte *dst = (byte *)surface->pixels;

	// Expand the 8 pixels in a byte into a full byte per pixel

	for (uint32 i = 0; i < surface->h; i++) {
		for (uint x = 0; x < surface->w;) {
			byte temp = rawStream->readByte();

			for (int j = 7; j >= 0 && x < surface->w; j--) {
				if (temp & (1 << j))
					*dst++ = 0xf;
				else
					*dst++ = 0;

				x++;
			}
		}
	}
}

#define ADD_BIT(dstPixel, srcBit) \
	*(dst + j * 4 + dstPixel) = (*(dst + j * 4 + dstPixel) >> 1) | (((temp >> srcBit) & 1) << 3)

void DOSBitmap::expandEGAPlanes(Graphics::Surface *surface, Common::SeekableReadStream *rawStream) {
	assert(surface->format.bytesPerPixel == 1);

	// Note that the image is in EGA planar form and not just standard 4bpp
	// This seems to contradict the PoP specs which seem to do something else

	byte *dst = (byte *)surface->pixels;

	for (uint32 i = 0; i < surface->h; i++) {
		uint x = 0;

		for (int32 j = 0; j < surface->w / 4; j++) {
			byte temp = rawStream->readByte();
			ADD_BIT(3, 4);
			ADD_BIT(2, 5);
			ADD_BIT(1, 6);
			ADD_BIT(0, 7);
			j++;
			ADD_BIT(3, 0);
			ADD_BIT(2, 1);
			ADD_BIT(1, 2);
			ADD_BIT(0, 3);

			if (x < 3 && j + 1 >= surface->w / 4) {
				j = -1;
				x++;
			}
		}

		dst += surface->w;
	}
}

} // End of namespace Mohawk
