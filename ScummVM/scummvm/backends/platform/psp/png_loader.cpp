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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"
#include "common/stream.h"
#include "backends/platform/psp/psppixelformat.h"
#include "backends/platform/psp/display_client.h"
#include "backends/platform/psp/png_loader.h"

//#define __PSP_DEBUG_FUNCS__	/* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */

#include "backends/platform/psp/trace.h"

PngLoader::Status PngLoader::allocate() {
	DEBUG_ENTER_FUNC();

	if (!findImageDimensions()) {
		PSP_ERROR("failed to get image dimensions\n");
		return BAD_FILE;
	}

	_buffer->setSize(_width, _height, _sizeBy);

	uint32 bitsPerPixel = _bitDepth * _channels;

	if (_paletteSize) {	// 8 or 4-bit image
		if (bitsPerPixel == 4) {
			_buffer->setPixelFormat(PSPPixelFormat::Type_Palette_4bit);
			_palette->setPixelFormats(PSPPixelFormat::Type_4444, PSPPixelFormat::Type_Palette_4bit);
			_paletteSize = 16;	// round up
		} else if (bitsPerPixel == 8) {			// 8-bit image
			_buffer->setPixelFormat(PSPPixelFormat::Type_Palette_8bit);
			_palette->setPixelFormats(PSPPixelFormat::Type_4444, PSPPixelFormat::Type_Palette_8bit);
			_paletteSize = 256; // round up
		} else {
			PSP_ERROR("too many bits per pixel[%d] for a palette\n", bitsPerPixel);
			return BAD_FILE;
		}

	} else {	// 32-bit image
		_buffer->setPixelFormat(PSPPixelFormat::Type_8888);
	}

	if (!_buffer->allocate()) {
		PSP_ERROR("failed to allocate buffer\n");
		return OUT_OF_MEMORY;
	}
	if (_buffer->hasPalette() && !_palette->allocate()) {
		PSP_ERROR("failed to allocate palette\n");
		return OUT_OF_MEMORY;
	}
	return OK;
}

bool PngLoader::load() {
	DEBUG_ENTER_FUNC();
	// Try to load the image
	_file.seek(0);	// Go back to start

	if (!loadImageIntoBuffer()) {
		PSP_DEBUG_PRINT("failed to load image\n");
		return false;
	}

	PSP_DEBUG_PRINT("succeded in loading image\n");

	if (_paletteSize == 16)		// 4-bit
		_buffer->flipNibbles();	// required because of PNG 4-bit format
	return true;
}

void PngLoader::warningFn(png_structp png_ptr, png_const_charp warning_msg) {
	// ignore PNG warnings
}

// Read function for png library to be able to read from our SeekableReadStream
//
void PngLoader::libReadFunc(png_structp pngPtr, png_bytep data, png_size_t length) {
	Common::SeekableReadStream &file = *(Common::SeekableReadStream *)pngPtr->io_ptr;

	file.read(data, length);
}

bool PngLoader::basicImageLoad() {
	DEBUG_ENTER_FUNC();
	_pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!_pngPtr)
		return false;

	png_set_error_fn(_pngPtr, (png_voidp) NULL, (png_error_ptr) NULL, warningFn);

	_infoPtr = png_create_info_struct(_pngPtr);
	if (!_infoPtr) {
		png_destroy_read_struct(&_pngPtr, png_infopp_NULL, png_infopp_NULL);
		return false;
	}
	// Set the png lib to use our read function
	png_set_read_fn(_pngPtr, &_file, libReadFunc);

	unsigned int sig_read = 0;

	png_set_sig_bytes(_pngPtr, sig_read);
	png_read_info(_pngPtr, _infoPtr);
	int interlaceType;
	png_get_IHDR(_pngPtr, _infoPtr, (png_uint_32 *)&_width, (png_uint_32 *)&_height, &_bitDepth,
		&_colorType, &interlaceType, int_p_NULL, int_p_NULL);
	_channels = png_get_channels(_pngPtr, _infoPtr);

	if (_colorType & PNG_COLOR_MASK_PALETTE)
		_paletteSize = _infoPtr->num_palette;

	return true;
}

/* Get the width and height of a png image */
bool PngLoader::findImageDimensions() {
	DEBUG_ENTER_FUNC();

	bool status = basicImageLoad();

	PSP_DEBUG_PRINT("width[%d], height[%d], paletteSize[%d], bitDepth[%d], channels[%d], rowBytes[%d]\n", _width, _height, _paletteSize, _bitDepth, _channels, _infoPtr->rowbytes);
	png_destroy_read_struct(&_pngPtr, &_infoPtr, png_infopp_NULL);
	return status;
}

//
// Load a texture from a png image
//
bool PngLoader::loadImageIntoBuffer() {
	DEBUG_ENTER_FUNC();

	if (!basicImageLoad()) {
		png_destroy_read_struct(&_pngPtr, &_infoPtr, png_infopp_NULL);
		return false;
	}
	png_set_strip_16(_pngPtr);		// Strip off 16 bit channels in case they occur

	if (_paletteSize) {
		// Copy the palette
		png_colorp srcPal = _infoPtr->palette;
		for (int i = 0; i < _infoPtr->num_palette; i++) {
			unsigned char alphaVal = (i < _infoPtr->num_trans) ? _infoPtr->trans[i] : 0xFF;	// Load alpha if it's there
			_palette->setSingleColorRGBA(i, srcPal->red, srcPal->green, srcPal->blue, alphaVal);
			srcPal++;
		}
	} else {	// Not a palettized image
		if (_colorType == PNG_COLOR_TYPE_GRAY && _bitDepth < 8)
			png_set_gray_1_2_4_to_8(_pngPtr);	// Round up grayscale images
		if (png_get_valid(_pngPtr, _infoPtr, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(_pngPtr);		// Convert trans channel to alpha for 32 bits

		png_set_add_alpha(_pngPtr, 0xff, PNG_FILLER_AFTER);		// Filler for alpha if none exists
	}

	uint32 rowBytes = png_get_rowbytes(_pngPtr, _infoPtr);

	// there seems to be a bug in libpng where it doesn't increase the rowbytes or the
	// channel even after we add the alpha channel
	if (_channels == 3 && (rowBytes / _width) == 3) {
		_channels = 4;
		rowBytes = _width * _channels;
	}

	PSP_DEBUG_PRINT("rowBytes[%d], channels[%d]\n", rowBytes, _channels);

	unsigned char *line = (unsigned char*) malloc(rowBytes);
	if (!line) {
		png_destroy_read_struct(&_pngPtr, png_infopp_NULL, png_infopp_NULL);
		PSP_ERROR("Couldn't allocate line\n");
		return false;
	}

	for (size_t y = 0; y < _height; y++) {
		png_read_row(_pngPtr, line, png_bytep_NULL);
		_buffer->copyFromRect(line, rowBytes, 0, y, _width, 1);	// Copy into buffer
	}
	free(line);
	png_read_end(_pngPtr, _infoPtr);
	png_destroy_read_struct(&_pngPtr, &_infoPtr, png_infopp_NULL);

	return true;
}
