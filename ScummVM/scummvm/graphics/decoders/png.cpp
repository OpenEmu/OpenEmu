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

// Since we need to work with libpng here, we need to allow all symbols
// to avoid compilation issues.
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#ifdef USE_PNG
#include <png.h>
#endif

#include "graphics/decoders/png.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "common/stream.h"

namespace Graphics {

PNGDecoder::PNGDecoder() : _outputSurface(0), _palette(0), _paletteColorCount(0) {
}

PNGDecoder::~PNGDecoder() {
	destroy();
}

void PNGDecoder::destroy() {
	if (_outputSurface) {
		_outputSurface->free();
		delete _outputSurface;
		_outputSurface = 0;
	}
	delete[] _palette;
	_palette = NULL;
}

#ifdef USE_PNG
// libpng-error-handling:
void pngError(png_structp pngptr, png_const_charp errorMsg) {
	error("%s", errorMsg);
}

void pngWarning(png_structp pngptr, png_const_charp warningMsg) {
	warning("%s", warningMsg);
}

// libpng-I/O-helper:
void pngReadFromStream(png_structp pngPtr, png_bytep data, png_size_t length) {
	void *readIOptr = png_get_io_ptr(pngPtr);
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)readIOptr;
	stream->read(data, length);
}
#endif

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

bool PNGDecoder::loadStream(Common::SeekableReadStream &stream) {
#ifdef USE_PNG
	destroy();

	_stream = &stream;

	// First, check the PNG signature
	if (_stream->readUint32BE() != MKTAG(0x89, 'P', 'N', 'G')) {
		delete _stream;
		return false;
	}
	if (_stream->readUint32BE() != MKTAG(0x0d, 0x0a, 0x1a, 0x0a)) {
		delete _stream;
		return false;
	}

	// The following is based on the guide provided in:
	//http://www.libpng.org/pub/png/libpng-1.2.5-manual.html#section-3
	//http://www.libpng.org/pub/png/libpng-1.4.0-manual.pdf
	// along with the png-loading code used in the sword25-engine.
	png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pngPtr) {
		delete _stream;
		return false;
	}
	png_infop infoPtr = png_create_info_struct(pngPtr);
	if (!infoPtr) {
		png_destroy_read_struct(&pngPtr, NULL, NULL);
		delete _stream;
		return false;
	}
	png_infop endInfo = png_create_info_struct(pngPtr);
	if (!endInfo) {
		png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
		delete _stream;
		return false;
	}

	png_set_error_fn(pngPtr, NULL, pngError, pngWarning);
	// TODO: The manual says errors should be handled via setjmp

	png_set_read_fn(pngPtr, _stream, pngReadFromStream);
	png_set_crc_action(pngPtr, PNG_CRC_DEFAULT, PNG_CRC_WARN_USE);
	// We already verified the PNG-header
	png_set_sig_bytes(pngPtr, 8);

	// Read PNG header
	png_read_info(pngPtr, infoPtr);

	// No handling for unknown chunks yet.
	int bitDepth, colorType, width, height, interlaceType;
	png_uint_32 w, h;
	png_get_IHDR(pngPtr, infoPtr, &w, &h, &bitDepth, &colorType, &interlaceType, NULL, NULL);
	width = w;
	height = h;

	// Allocate memory for the final image data.
	// To keep memory framentation low this happens before allocating memory for temporary image data.
	_outputSurface = new Graphics::Surface();

	// Images of all color formats except PNG_COLOR_TYPE_PALETTE
	// will be transformed into ARGB images
	if (colorType == PNG_COLOR_TYPE_PALETTE && !png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS)) {
		int numPalette = 0;
		png_colorp palette = NULL;
		uint32 success = png_get_PLTE(pngPtr, infoPtr, &palette, &numPalette);
		if (success != PNG_INFO_PLTE) {
			png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
			return false;
		}
		_paletteColorCount = numPalette;
		_palette = new byte[_paletteColorCount * 3];
		for (int i = 0; i < _paletteColorCount; i++) {
			_palette[(i * 3)] = palette[i].red;
			_palette[(i * 3) + 1] = palette[i].green;
			_palette[(i * 3) + 2] = palette[i].blue;

		}
		_outputSurface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
		png_set_packing(pngPtr);
	} else {
		_outputSurface->create(width, height, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
		if (!_outputSurface->pixels) {
			error("Could not allocate memory for output image.");
		}
		if (bitDepth == 16)
			png_set_strip_16(pngPtr);
		if (bitDepth < 8)
			png_set_expand(pngPtr);
		if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
			png_set_expand(pngPtr);
		if (colorType == PNG_COLOR_TYPE_GRAY ||
			colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(pngPtr);

		// PNGs are Big-Endian:
#ifdef SCUMM_LITTLE_ENDIAN
		png_set_bgr(pngPtr);
		png_set_swap_alpha(pngPtr);
		if (colorType != PNG_COLOR_TYPE_RGB_ALPHA)
			png_set_filler(pngPtr, 0xff, PNG_FILLER_BEFORE);
#else
		if (colorType != PNG_COLOR_TYPE_RGB_ALPHA)
			png_set_filler(pngPtr, 0xff, PNG_FILLER_AFTER);
#endif

	}

	// After the transformations have been registered, the image data is read again.
	png_set_interlace_handling(pngPtr);
	png_read_update_info(pngPtr, infoPtr);
	png_get_IHDR(pngPtr, infoPtr, &w, &h, &bitDepth, &colorType, NULL, NULL, NULL);
	width = w;
	height = h;

	if (interlaceType == PNG_INTERLACE_NONE) {
		// PNGs without interlacing can simply be read row by row.
		for (int i = 0; i < height; i++) {
			png_read_row(pngPtr, (png_bytep)_outputSurface->getBasePtr(0, i), NULL);
		}
	} else {
		// PNGs with interlacing require us to allocate an auxillary
		// buffer with pointers to all row starts.

		// Allocate row pointer buffer
		png_bytep *rowPtr = new png_bytep[height];
		if (!rowPtr) {
			error("Could not allocate memory for row pointers.");
		}

		// Initialize row pointers
		for (int i = 0; i < height; i++)
			rowPtr[i] = (png_bytep)_outputSurface->getBasePtr(0, i);

		// Read image data
		png_read_image(pngPtr, rowPtr);

		// Free row pointer buffer
		delete[] rowPtr;
	}

	// Read additional data at the end.
	png_read_end(pngPtr, NULL);

	// Destroy libpng structures
	png_destroy_read_struct(&pngPtr, &infoPtr, NULL);

	// We no longer need the file stream, thus close it here
	_stream = 0;

	return true;
#else
	return false;
#endif
}

} // End of Graphics namespace
