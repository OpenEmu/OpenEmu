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

/* Based on code from xoreos https://github.com/DrMcCoy/xoreos/
 * relicensed under GPLv2+ with permission from DrMcCoy and clone2727
 */

#include "common/util.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/error.h"

#include "graphics/decoders/tga.h"

namespace Graphics {

TGADecoder::TGADecoder() {
	_colorMapSize = 0;
	_colorMapOrigin = 0;
	_colorMapLength = 0;
	_colorMapEntryLength = 0;
	_colorMap = NULL;
}

TGADecoder::~TGADecoder() {
	destroy();
}

void TGADecoder::destroy() {
	_surface.free();
	delete[] _colorMap;
}

bool TGADecoder::loadStream(Common::SeekableReadStream &tga) {
	byte imageType, pixelDepth;
	bool success;
	success = readHeader(tga, imageType, pixelDepth);
	if (success) {
		switch (imageType) {
		case TYPE_BW:
		case TYPE_TRUECOLOR:
			success = readData(tga, imageType, pixelDepth);
			break;
		case TYPE_RLE_BW:
		case TYPE_RLE_TRUECOLOR:
		case TYPE_RLE_CMAP:
			success = readDataRLE(tga, imageType, pixelDepth);
			break;
		case TYPE_CMAP:
			success = readDataColorMapped(tga, imageType, pixelDepth);
			break;
		default:
			success = false;
			break;
		}
	}
	if (tga.err() || !success) {
		warning("Failed reading TGA-file");
		return false;
	}
	return success;
}

bool TGADecoder::readHeader(Common::SeekableReadStream &tga, byte &imageType, byte &pixelDepth) {
	if (!tga.seek(0)) {
		warning("Failed reading TGA-file");
		return false;
	}

	// TGAs have an optional "id" string in the header
	uint32 idLength = tga.readByte();

	// Number of colors in the color map / palette
	int hasColorMap = tga.readByte();

	// Image type. See header for numeric constants
	imageType = tga.readByte();

	switch (imageType) {
	case TYPE_CMAP:
	case TYPE_TRUECOLOR:
	case TYPE_BW:
	case TYPE_RLE_CMAP:
	case TYPE_RLE_TRUECOLOR:
	case TYPE_RLE_BW:
		break;
	default:
		warning("Unsupported image type: %d", imageType);
		return false;
	}

	// Color map specifications
	if (hasColorMap == 0) {
		tga.skip(5);
	} else {
		_colorMapOrigin = tga.readUint16LE();
		_colorMapLength = tga.readUint16LE();
		_colorMapEntryLength = tga.readByte();
	}
	// Origin-defintions
	tga.skip(2 + 2);

	// Image dimensions
	_surface.w = tga.readUint16LE();
	_surface.h = tga.readUint16LE();

	// Bits per pixel
	pixelDepth = tga.readByte();
	_surface.format.bytesPerPixel = pixelDepth / 8;

	// Image descriptor
	byte imgDesc = tga.readByte();
	int attributeBits = imgDesc & 0x0F;
	assert((imgDesc & 0x10) == 0);
	_originTop = (imgDesc & 0x20);

	// Interleaving is not handled at this point
	//int interleave = (imgDesc & 0xC);
	if (imageType == TYPE_CMAP || imageType == TYPE_RLE_CMAP) {
		if (pixelDepth == 8) {
			_format = PixelFormat::createFormatCLUT8();
		} else {
			warning("Unsupported index-depth: %d", pixelDepth);
			return false;
		}
	} else if (imageType == TYPE_TRUECOLOR || imageType == TYPE_RLE_TRUECOLOR) {
		if (pixelDepth == 24) {
			_format = PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0);
		} else if (pixelDepth == 32) {
			// HACK: According to the spec, attributeBits should determine the amount
			// of alpha-bits, however, as the game files that use this decoder seems
			// to ignore that fact, we force the amount to 8 for 32bpp files for now.
			_format = PixelFormat(4, 8, 8, 8, /* attributeBits */ 8, 16, 8, 0, 24);
		} else if (pixelDepth == 16 && imageType == TYPE_TRUECOLOR) {
			// 16bpp TGA is ARGB1555
			_format = PixelFormat(2, 5, 5, 5, attributeBits, 10, 5, 0, 15);
		} else {
			warning("Unsupported pixel depth: %d, %d", imageType, pixelDepth);
			return false;
		}
	} else if (imageType == TYPE_BW || TYPE_RLE_BW) {
		if (pixelDepth == 8) {
			_format = PixelFormat(4, 8, 8, 8, 0, 16, 8, 0, 0);
		} else {
			warning("Unsupported pixel depth: %d, %d", imageType, pixelDepth);
			return false;
		}

	} else {
		warning("Unsupported image type: %d", imageType);
		return false;
	}

	// Skip the id string
	tga.skip(idLength);

	if (hasColorMap) {
		return readColorMap(tga, imageType, pixelDepth);
	}
	return true;
}

bool TGADecoder::readColorMap(Common::SeekableReadStream &tga, byte imageType, byte pixelDepth) {
	_colorMap = new byte[3 * _colorMapLength];
	for (int i = 0; i < _colorMapLength * 3; i += 3) {
		byte r, g, b;
		if (_colorMapEntryLength == 32) {
			byte a;
			PixelFormat format(4, 8, 8, 8, 0, 16, 8, 0, 24);
			uint32 color = tga.readUint32LE();
			format.colorToARGB(color, a, r, g, b);
		} else if (_colorMapEntryLength == 24) {
			r = tga.readByte();
			g = tga.readByte();
			b = tga.readByte();
		} else if (_colorMapEntryLength == 16) {
			byte a;
			PixelFormat format(2, 5, 5, 5, 0, 10, 5, 0, 15);
			uint16 color = tga.readUint16LE();
			format.colorToARGB(color, a, r, g, b);
		} else {
			warning("Unsupported image type: %d", imageType);
			r = g = b = 0;
		}
#ifdef SCUMM_LITTLE_ENDIAN
		_colorMap[i] = r;
		_colorMap[i + 1] = g;
		_colorMap[i + 2] = b;
#else
		_colorMap[i] = b;
		_colorMap[i + 1] = g;
		_colorMap[i + 2] = r;
#endif
	}
	return true;
}

// Additional information found from http://paulbourke.net/dataformats/tga/
// With some details from the link referenced in the header.
bool TGADecoder::readData(Common::SeekableReadStream &tga, byte imageType, byte pixelDepth) {
	// TrueColor
	if (imageType == TYPE_TRUECOLOR) {
		_surface.create(_surface.w, _surface.h, _format);

		if (pixelDepth == 16) {
			for (int i = 0; i < _surface.h; i++) {
				uint16 *dst;
				if (!_originTop) {
					dst = (uint16 *)_surface.getBasePtr(0, _surface.h - i - 1);
				} else {
					dst = (uint16 *)_surface.getBasePtr(0, i);
				}
				for (int j = 0; j < _surface.w; j++) {
					*dst++ = tga.readUint16LE();
				}
			}
		} else if (pixelDepth == 32) {
			for (int i = 0; i < _surface.h; i++) {
				uint32 *dst;
				if (!_originTop) {
					dst = (uint32 *)_surface.getBasePtr(0, _surface.h - i - 1);
				} else {
					dst = (uint32 *)_surface.getBasePtr(0, i);
				}
				for (int j = 0; j < _surface.w; j++) {
					*dst++ = tga.readUint32LE();
				}
			}
		} else if (pixelDepth == 24) {
			for (int i = 0; i < _surface.h; i++) {
				byte *dst;
				if (!_originTop) {
					dst = (byte *)_surface.getBasePtr(0, _surface.h - i - 1);
				} else {
					dst = (byte *)_surface.getBasePtr(0, i);
				}
				for (int j = 0; j < _surface.w; j++) {
					byte r = tga.readByte();
					byte g = tga.readByte();
					byte b = tga.readByte();
#ifdef SCUMM_LITTLE_ENDIAN
					*dst++ = r;
					*dst++ = g;
					*dst++ = b;
#else
					*dst++ = b;
					*dst++ = g;
					*dst++ = r;
#endif
				}
			}
		}
		// Black/White
	} else if (imageType == TYPE_BW) {
		_surface.create(_surface.w, _surface.h, _format);

		byte *data  = (byte *)_surface.pixels;
		uint32 count = _surface.w * _surface.h;

		while (count-- > 0) {
			byte g = tga.readByte();
			*data++ = g;
			*data++ = g;
			*data++ = g;
			*data++ = g;
		}
	}
	return true;
}

bool TGADecoder::readDataColorMapped(Common::SeekableReadStream &tga, byte imageType, byte indexDepth) {
	// Color-mapped
	if (imageType == TYPE_CMAP) {
		_surface.create(_surface.w, _surface.h, _format);
		if (indexDepth == 8) {
			for (int i = 0; i < _surface.h; i++) {
				byte *dst;
				if (!_originTop) {
					dst = (byte *)_surface.getBasePtr(0, _surface.h - i - 1);
				} else {
					dst = (byte *)_surface.getBasePtr(0, i);
				}
				for (int j = 0; j < _surface.w; j++) {
					byte index = tga.readByte();
					*dst++ = index;
				}
			}
		} else if (indexDepth == 16) {
			warning("16 bit indexes not supported");
			return false;
		}
	} else {
		return false;
	}
	return true;
}

bool TGADecoder::readDataRLE(Common::SeekableReadStream &tga, byte imageType, byte pixelDepth) {
	// RLE-TrueColor / RLE-Black/White
	if (imageType == TYPE_RLE_TRUECOLOR || imageType == TYPE_RLE_BW || imageType == TYPE_RLE_CMAP) {
		_surface.create(_surface.w, _surface.h, _format);
		uint32 count = _surface.w * _surface.h;
		byte *data = (byte *)_surface.pixels;

		while (count > 0) {
			uint32 header = tga.readByte();
			byte type = (header & 0x80) >> 7;
			uint32 rleCount = (header & 0x7F) + 1;

			// RLE-packet
			if (type == 1) {
				if (pixelDepth == 32 && imageType == TYPE_RLE_TRUECOLOR) {
					uint32 color = tga.readUint32LE();
					while (rleCount-- > 0) {
						*((uint32 *)data) = color;
						data += 4;
						count--;
					}
				} else if (pixelDepth == 24 && imageType == TYPE_RLE_TRUECOLOR) {
					byte r = tga.readByte();
					byte g = tga.readByte();
					byte b = tga.readByte();
					while (rleCount-- > 0) {
#ifdef SCUMM_LITTLE_ENDIAN
						*data++ = r;
						*data++ = g;
						*data++ = b;
#else
						*data++ = b;
						*data++ = g;
						*data++ = r;
#endif
						count--;
					}
				} else if (pixelDepth == 8 && imageType == TYPE_RLE_BW) {
					byte color = tga.readByte();
					while (rleCount-- > 0) {
						*data++ = color;
						*data++ = color;
						*data++ = color;
						*data++ = color;
						count--;
					}
				} else if (pixelDepth == 8 && imageType == TYPE_RLE_CMAP) {
					byte index = tga.readByte();
					while (rleCount-- > 0) {
						*data++ = index;
						count--;
					}
				} else {
					warning("Unhandled pixel-depth for image-type 10");
					return false;
				}
				// Raw-packet
			} else if (type == 0) {
				if (pixelDepth == 32 && imageType == TYPE_RLE_TRUECOLOR) {
					while (rleCount-- > 0) {
						uint32 color = tga.readUint32LE();
						*((uint32 *)data) = color;
						data += 4;
						count--;
					}
				} else if (pixelDepth == 24 && imageType == TYPE_RLE_TRUECOLOR) {
					while (rleCount-- > 0) {
						byte r = tga.readByte();
						byte g = tga.readByte();
						byte b = tga.readByte();
#ifdef SCUMM_LITTLE_ENDIAN
						*data++ = r;
						*data++ = g;
						*data++ = b;
#else
						*data++ = b;
						*data++ = g;
						*data++ = r;
#endif
						count--;
					}
				} else if (pixelDepth == 8 && imageType == TYPE_RLE_BW) {
					while (rleCount-- > 0) {
						byte color = tga.readByte();
						*data++ = color;
						*data++ = color;
						*data++ = color;
						*data++ = color;
						count--;
					}
				} else if (pixelDepth == 8 && imageType == TYPE_RLE_CMAP) {
					while (rleCount-- > 0) {
						byte index = tga.readByte();
						*data++ = index;
						count--;
					}
				} else {
					warning("Unhandled pixel-depth for image-type 10");
					return false;
				}
			} else {
				warning("Unknown header for RLE-packet %d", type);
				return false;
			}
		}
	} else {
		return false;
	}
	return true;
}

} // End of namespace Graphics
