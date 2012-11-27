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

#ifndef PSP_PNG_IMAGE_H
#define PSP_PNG_IMAGE_H

#include <png.h>

class PngLoader {
private:
	bool basicImageLoad();		// common operation
	bool findImageDimensions();	// find dimensions of a given PNG file
	bool loadImageIntoBuffer();

	static void warningFn(png_structp png_ptr, png_const_charp warning_msg);
	static void libReadFunc(png_structp pngPtr, png_bytep data, png_size_t length);

	Common::SeekableReadStream &_file;
	Buffer *_buffer;
	Palette *_palette;

	uint32 _width;
	uint32 _height;
	uint32 _paletteSize;
	Buffer::HowToSize _sizeBy;

	// PNG lib values
	int _bitDepth;
	png_structp _pngPtr;
	png_infop _infoPtr;
	int _colorType;
	uint32 _channels;

public:
	enum Status {
		OK,
		OUT_OF_MEMORY,
		BAD_FILE
	};

	PngLoader(Common::SeekableReadStream &file, Buffer &buffer, Palette &palette,
		Buffer::HowToSize sizeBy = Buffer::kSizeByTextureSize) :
			_file(file), _buffer(&buffer), _palette(&palette),
			_width(0), _height(0), _paletteSize(0),
			_bitDepth(0), _sizeBy(sizeBy), _pngPtr(0),
			_infoPtr(0), _colorType(0), _channels(0) {}

	PngLoader::Status allocate();
	bool load();
};

#endif /* PSP_PNG_IMAGE_H */
