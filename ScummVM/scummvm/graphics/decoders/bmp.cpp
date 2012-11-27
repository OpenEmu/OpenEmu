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

#include "common/stream.h"
#include "common/textconsole.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "graphics/decoders/bmp.h"

namespace Graphics {

BitmapDecoder::BitmapDecoder() {
	_surface = 0;
	_palette = 0;
	_paletteColorCount = 0;
}

BitmapDecoder::~BitmapDecoder() {
	destroy();
}

void BitmapDecoder::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface; _surface = 0;
	}

	delete[] _palette; _palette = 0;
	_paletteColorCount = 0;
}

bool BitmapDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	if (stream.readByte() != 'B')
		return false;

	if (stream.readByte() != 'M')
		return false;

	/* uint32 fileSize = */ stream.readUint32LE();
	/* uint16 res1 = */ stream.readUint16LE();
	/* uint16 res2 = */ stream.readUint16LE();
	uint32 imageOffset = stream.readUint32LE();

	uint32 infoSize = stream.readUint32LE();
	if (infoSize != 40) {
		warning("Only Windows v3 bitmaps are supported");
		return false;
	}

	uint32 width = stream.readUint32LE();
	int32 height = stream.readSint32LE();

	if (width == 0 || height == 0)
		return false;

	if (height < 0) {
		warning("Right-side up bitmaps not supported");
		return false;
	}

	/* uint16 planes = */ stream.readUint16LE();
	uint16 bitsPerPixel = stream.readUint16LE();

	if (bitsPerPixel != 8 && bitsPerPixel != 24 && bitsPerPixel != 32) {
		warning("%dbpp bitmaps not supported", bitsPerPixel);
		return false;
	}

	uint32 compression = stream.readUint32LE();

	if (compression != 0) {
		warning("Compressed bitmaps not supported");
		return false;
	}

	/* uint32 imageSize = */ stream.readUint32LE();
	/* uint32 pixelsPerMeterX = */ stream.readUint32LE();
	/* uint32 pixelsPerMeterY = */ stream.readUint32LE();
	_paletteColorCount = stream.readUint32LE();
	/* uint32 colorsImportant = */ stream.readUint32LE();

	if (bitsPerPixel == 8) {
		if (_paletteColorCount == 0)
			_paletteColorCount = 256;

		// Read the palette
		_palette = new byte[_paletteColorCount * 3];
		for (uint16 i = 0; i < _paletteColorCount; i++) {
			_palette[i * 3 + 2] = stream.readByte();
			_palette[i * 3 + 1] = stream.readByte();
			_palette[i * 3 + 0] = stream.readByte();
			stream.readByte();
		}
	}

	// Start us at the beginning of the image
	stream.seek(imageOffset);

	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();

	// BGRA for 24bpp and 32 bpp
	if (bitsPerPixel == 24 || bitsPerPixel == 32)
		format = Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0);

	_surface = new Graphics::Surface();
	_surface->create(width, height, format);

	int srcPitch = width * (bitsPerPixel >> 3);
	const int extraDataLength = (srcPitch % 4) ? 4 - (srcPitch % 4) : 0;

	if (bitsPerPixel == 8) {
		byte *dst = (byte *)_surface->pixels;

		for (int32 i = 0; i < height; i++) {
			stream.read(dst + (height - i - 1) * width, width);
			stream.skip(extraDataLength);
		}
	} else if (bitsPerPixel == 24) {
		byte *dst = (byte *)_surface->pixels + (height - 1) * _surface->pitch;

		for (int32 i = 0; i < height; i++) {
			for (uint32 j = 0; j < width; j++) {
				byte b = stream.readByte();
				byte g = stream.readByte();
				byte r = stream.readByte();
				uint32 color = format.RGBToColor(r, g, b);

				*((uint32 *)dst) = color;
				dst += format.bytesPerPixel;
			}

			stream.skip(extraDataLength);
			dst -= _surface->pitch * 2;
		}
	} else { // 32 bpp
		byte *dst = (byte *)_surface->pixels + (height - 1) * _surface->pitch;

		for (int32 i = 0; i < height; i++) {
			for (uint32 j = 0; j < width; j++) {
				byte b = stream.readByte();
				byte g = stream.readByte();
				byte r = stream.readByte();
				// Ignore the last byte, as in v3 it is unused
				// and should thus NOT be used as alpha.
				// ref: http://msdn.microsoft.com/en-us/library/windows/desktop/dd183376%28v=vs.85%29.aspx
				stream.readByte();
				uint32 color = format.RGBToColor(r, g, b);

				*((uint32 *)dst) = color;
				dst += format.bytesPerPixel;
			}

			stream.skip(extraDataLength);
			dst -= _surface->pitch * 2;
		}
	}

	return true;
}

} // End of namespace Graphics
