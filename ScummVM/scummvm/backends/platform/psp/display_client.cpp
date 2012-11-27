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

// Disable printf override in common/forbidden.h to avoid
// clashes with pspdebug.h from the PSP SDK.
// That header file uses
//   __attribute__((format(printf,1,2)));
// which gets messed up by our override mechanism; this could
// be avoided by either changing the PSP SDK to use the equally
// legal and valid
//   __attribute__((format(__printf__,1,2)));
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the PSP port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <pspgu.h>
#include <pspkerneltypes.h>
#include <pspdisplay.h>
#include <psputils.h>

#include "common/scummsys.h"
#include "backends/platform/psp/psppixelformat.h"
#include "backends/platform/psp/display_client.h"
#include "backends/platform/psp/display_manager.h"
#define PSP_INCLUDE_SWAP
#include "backends/platform/psp/memory.h"

//#define __PSP_DEBUG_FUNCS__	/* For debugging the stack */
//#define __PSP_DEBUG_PRINT__
#include "backends/platform/psp/trace.h"

#define PSP_BUFFER_WIDTH (512)
#define	PSP_SCREEN_WIDTH	480
#define	PSP_SCREEN_HEIGHT	272
#define PSP_FRAME_SIZE (PSP_BUFFER_WIDTH * PSP_SCREEN_HEIGHT)

DisplayManager *GuRenderer::_displayManager = 0;


// class Palette ------------------------------------------------------------
//
void Palette::clear() {
	DEBUG_ENTER_FUNC();

	if (_values && _numOfEntries)
		memset(_values, 0, getSizeInBytes());

	PSP_DEBUG_PRINT("_values[%p]\n", _values);
}

// Used to clear the specific keycolor
//
void Palette::setColorPositionAlpha(uint32 position, bool alpha) {
	DEBUG_ENTER_FUNC();

	assert(_values);
	assert(position < _numOfEntries);

	PSP_DEBUG_PRINT("position[%d], numofEntries[%u], bpp[%u], values[%p]\n", position, _numOfEntries,
	                _pixelFormat.bitsPerPixel, _values);

	if (_numOfEntries <= 16)
		position &= 0xF;
	else if (_numOfEntries <= 256)
		position &= 0xFF;

	switch (_pixelFormat.bitsPerPixel) {
	case 16: {
		uint16 *shortVal = (uint16 *) & _values[_pixelFormat.pixelsToBytes(position)];
		*shortVal = _pixelFormat.setColorAlpha((uint32) * shortVal, alpha ? 255 : 0);
	}
	break;
	case 32: {
		uint32 *wordVal = (uint32 *) & _values[_pixelFormat.pixelsToBytes(position)];
		*wordVal = _pixelFormat.setColorAlpha((uint32) * wordVal, alpha ? 255 : 0);
	}
	break;
	default:
		PSP_ERROR("Incorrect bits per pixel value[%u]\n", _pixelFormat.bitsPerPixel);
	}
}

//	Set some of the palette to color values in array
//	By default, ScummVm doesn't support alpha values in palettes
void Palette::setPartial(const byte *colors, uint32 start, uint32 num, bool supportsAlpha /* = false */) {
	DEBUG_ENTER_FUNC();

	assert(_values);
	assert(_numOfEntries);

	const byte *src = colors;

	if (start + num > _numOfEntries)	// Check boundary
		num = _numOfEntries - start;

	if (_pixelFormat.bitsPerPixel == 16) {
		uint16 *palette = (uint16 *)_values;
		palette += start;

		for (uint32 i = 0; i < num; ++i) {
			byte alphaVal = supportsAlpha ? src[3] : 0xFF;
			*palette = (uint16)_pixelFormat.rgbaToColor(src[0], src[1], src[2], alphaVal);
			src += 3;
			palette++;
		}
	} else if (_pixelFormat.bitsPerPixel == 32) {
		uint32 *palette = (uint32 *)_values;
		palette += start;

		for (uint32 i = 0; i < num; ++i) {
			byte alphaVal = supportsAlpha ? src[3] : 0xFF;
			*palette = _pixelFormat.rgbaToColor(src[0], src[1], src[2], alphaVal);
			src += 3;
			palette++;
		}
	}
}

// Sets pixel format and number of entries by the buffer's pixel format */
void Palette::setPixelFormats(PSPPixelFormat::Type paletteType, PSPPixelFormat::Type bufferType, bool swapRedBlue /* = false */) {
	DEBUG_ENTER_FUNC();

	if (paletteType == PSPPixelFormat::Type_Unknown)
		PSP_ERROR("Unknown paletteType[%u]\n", paletteType);

	switch (bufferType) {
	case PSPPixelFormat::Type_Palette_8bit:
		_numOfEntries = 256;
		break;
	case PSPPixelFormat::Type_Palette_4bit:
		_numOfEntries = 16;
		break;
	case PSPPixelFormat::Type_Unknown:
	case PSPPixelFormat::Type_None:
		PSP_ERROR("Unhandled bufferType[%u]\n", bufferType);
		break;
	default:		// No palette
		_numOfEntries = 0;
		break;
	}

	_pixelFormat.set(paletteType, swapRedBlue);
}

bool Palette::allocate() {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("_numOfEntries[%u]\n", _numOfEntries);
	PSP_DEBUG_PRINT("_pixelFormat: format[%u], bpp[%u]\n", _pixelFormat.format, _pixelFormat.bitsPerPixel);

	if (_values) {
		free(CACHED(_values));
		_values = 0;
	}

	// We allocate on 64bytes to get a cache line, and round up to 64bytes to get the full line
	uint32 amountInBytes = getSizeInBytes();
	if (amountInBytes < 64)
		amountInBytes = 64;
	_values = (byte *)memalign(64, amountInBytes);

	// Use uncached memory
	GuRenderer::cacheInvalidate(_values, amountInBytes);
	_values = UNCACHED(_values);

	if (!_values) {
		PSP_ERROR("Couldn't allocate palette.\n");
		return false;
	}

	PSP_DEBUG_PRINT("_values[%p]\n", _values);
	clear();

	return true;
}

void Palette::deallocate() {
	DEBUG_ENTER_FUNC();

	free(CACHED(_values));
	_values = 0;
	_numOfEntries = 0;
}

// Copy some of the palette to an array of colors
//
void Palette::getPartial(byte *colors, uint start, uint num) {
	DEBUG_ENTER_FUNC();

	assert(_values);
	assert(_numOfEntries);

	uint32 r, g, b, a;

	if (start + num > _numOfEntries)	// Check boundary
		num = _numOfEntries - start;

	if (_pixelFormat.bitsPerPixel == 16) {
		uint16 *palette = (uint16 *)_values;
		palette += start;

		for (uint32 i = start; i < start + num; i++) {
			_pixelFormat.colorToRgba(*palette, r, g, b, a);

			*colors++ = (byte)r;
			*colors++ = (byte)g;
			*colors++ = (byte)b;
			palette++;
		}
	} else if (_pixelFormat.bitsPerPixel == 32) {
		uint32 *palette = (uint32 *)_values;
		palette += start;

		for (uint32 i = start; i < start + num; i++) {
			_pixelFormat.colorToRgba(*palette, r, g, b, a);

			*colors++ = (byte)r;
			*colors++ = (byte)g;
			*colors++ = (byte)b;
			palette++;
		}
	}
}

void Palette::setSingleColorRGBA(uint32 num, byte r, byte g, byte b, byte a) {
	// DEBUG_ENTER_FUNC();
	uint16 *shortValues;
	uint32 *wordValues;

	assert(_values);
	assert(num < _numOfEntries);

	switch (_pixelFormat.bitsPerPixel) {
	case 16:
		shortValues = (uint16 *)_values;
		shortValues[num] = _pixelFormat.rgbaToColor(r, g, b, a);
		break;
	case 32:
		wordValues = (uint32 *)_values;
		wordValues[num] = _pixelFormat.rgbaToColor(r, g, b, a);
		break;
	default:
		PSP_ERROR("Incorrect bitsPerPixel[%d]\n", _pixelFormat.bitsPerPixel);
		break;
	}
}

// Print to screen
void Palette::print(uint32 numToPrint /* = 0 */) {
	if (_numOfEntries > 0) {
		assert(_values);

		if (numToPrint > _numOfEntries || numToPrint == 0)
			numToPrint = _numOfEntries;

		PSP_INFO_PRINT("cursor palette:\n");

		for (unsigned int i = 0; i < numToPrint; i++) {
			byte *pcolor = &_values[_pixelFormat.pixelsToBytes(i)];
			uint32 color = _pixelFormat.getColorValueAt(pcolor);

			PSP_INFO_PRINT("[%u=%x] ", i, color);
		}

		PSP_INFO_PRINT("\n");
	}
}

uint32 Palette::getRawColorAt(uint32 position) {
	byte *pcolor = &_values[_pixelFormat.pixelsToBytes(position)];
	uint32 color = _pixelFormat.getColorValueAt(pcolor);
	return color;
}

uint32 Palette::getRGBAColorAt(uint32 position) {
	uint32 color = getRawColorAt(position);
	uint32 r, g, b, a;
	_pixelFormat.colorToRgba(color, r, g, b, a);
	return (a << 24 | b << 16 | g << 8 |  r);
}

// class Buffer ---------------------------------------------------

void Buffer::setPixelFormat(PSPPixelFormat::Type type, bool swapRedBlue) {
	if (type == PSPPixelFormat::Type_None ||
	        type == PSPPixelFormat::Type_Unknown)
		PSP_ERROR("Unhandled buffer format[%u]\n", type);

	_pixelFormat.set(type, swapRedBlue);
}

bool Buffer::hasPalette() {
	if (_pixelFormat.format == PSPPixelFormat::Type_Palette_8bit ||
	        _pixelFormat.format == PSPPixelFormat::Type_Palette_4bit)
		return true;

	return false;
}

/* pitch is in bytes */
void Buffer::copyFromArray(const byte *buffer, int pitch) {
	DEBUG_ENTER_FUNC();

	// We use sourceSize because outside, they won't know what the true size is
	copyFromRect(buffer, pitch, 0, 0, _sourceSize.width, _sourceSize.height);
}

/* pitch is in bytes */
void Buffer::copyFromRect(const byte *buf, uint32 pitch, int destX, int destY, uint32 recWidth, uint32 recHeight) {
	// Removed silly clipping code
	DEBUG_ENTER_FUNC();
	assert(_pixels);

	if (recWidth > _sourceSize.width - destX) {
		recWidth = _sourceSize.width - destX;
	}

	if (recHeight > _sourceSize.height - destY) {
		recHeight = _sourceSize.height - destY;
	}

	if (recWidth <= 0 || recHeight <= 0) {
		return;
	}

	byte *dst = _pixels + _pixelFormat.pixelsToBytes((destY * _width) + destX);

	uint32 recWidthInBytes = _pixelFormat.pixelsToBytes(recWidth);
	uint32 realWidthInBytes = _pixelFormat.pixelsToBytes(_width);

	if (pitch == realWidthInBytes && pitch == recWidthInBytes) {
		//memcpy(dst, buf, _pixelFormat.pixelsToBytes(recHeight * recWidth));
		if (_pixelFormat.swapRB)
			PspMemorySwap::fastSwap(dst, buf, _pixelFormat.pixelsToBytes(recHeight * recWidth), _pixelFormat);
		else
			PspMemory::fastCopy(dst, buf, _pixelFormat.pixelsToBytes(recHeight * recWidth));
	} else {
		do {
			if (_pixelFormat.swapRB)
				PspMemorySwap::fastSwap(dst, buf, recWidthInBytes, _pixelFormat);
			else
				PspMemory::fastCopy(dst, buf, recWidthInBytes);
			buf += pitch;
			dst += realWidthInBytes;
		} while (--recHeight);
	}
}

/* pitch is in bytes */
void Buffer::copyToArray(byte *dst, int pitch) {
	DEBUG_ENTER_FUNC();
	assert(_pixels);

	uint32 h = _height;
	byte *src = _pixels;
	uint32 sourceWidthInBytes = _pixelFormat.pixelsToBytes(_sourceSize.width);
	uint32 realWidthInBytes = _pixelFormat.pixelsToBytes(_width);

	do {
		//memcpy(dst, src, sourceWidthInBytes);
		if (_pixelFormat.swapRB)
			PspMemorySwap::fastSwap(dst, src, sourceWidthInBytes, _pixelFormat);
		else
			PspMemory::fastCopy(dst, src, sourceWidthInBytes);
		src += realWidthInBytes;
		dst += pitch;
	} while (--h);
}

void Buffer::setSize(uint32 width, uint32 height, HowToSize textureOrSource/*=kSizeByTextureSize*/) {
	DEBUG_ENTER_FUNC();

	// We can size the buffer either by texture size (multiple of 2^n) or source size.
	// At higher sizes, increasing the texture size to 2^n is a waste of space. At these sizes kSizeBySourceSize should be used.

	_sourceSize.width = width;
	_sourceSize.height = height;

	_textureSize.width = scaleUpToPowerOfTwo(width);		// can only scale up to 512
	_textureSize.height = scaleUpToPowerOfTwo(height);

	if (textureOrSource == kSizeByTextureSize) {
		_width = _textureSize.width;
		_height = _textureSize.height;
	} else { // sizeBySourceSize
		_width =  _sourceSize.width;
		_height = _sourceSize.height;

		// adjust allocated width to be divisible by 32.
		// The GU can only handle multiples of 16 bytes. A 4 bit image x 32 will give us 16 bytes
		// We don't necessarily know the depth of the pixels here. So just make it divisible by 32.
		uint32 checkDiv = _width & 31;
		if (checkDiv)
			_width += 32 - checkDiv;
	}

	PSP_DEBUG_PRINT("width[%u], height[%u], texW[%u], texH[%u], sourceW[%d], sourceH[%d] %s\n", _width, _height, _textureSize.width, _textureSize.height, _sourceSize.width, _sourceSize.height, textureOrSource ? "size by source" : "size by texture");
}

// Scale a dimension (width/height) up to power of 2 for the texture
// Will only go up to 512 since that's the maximum PSP texture size
uint32 Buffer::scaleUpToPowerOfTwo(uint32 size) {

	uint32 textureDimension = 16;
	while (size > textureDimension && textureDimension < 512)
		textureDimension <<= 1;

	PSP_DEBUG_PRINT("size[%u]. power of 2[%u]\n", size, textureDimension);

	return textureDimension;
}

bool Buffer::allocate(bool inVram/*=false*/) {
	DEBUG_ENTER_FUNC();

	PSP_DEBUG_PRINT("_width[%u], _height[%u]\n", _width, _height);
	PSP_DEBUG_PRINT("_pixelFormat: format[%u], bpp[%u]\n", _pixelFormat.format, _pixelFormat.bitsPerPixel);

	if (_pixels) {
		if (VramAllocator::isAddressInVram(_pixels)) 	// Check if in VRAM
			VramAllocator::instance().deallocate(_pixels);
		else	// not in VRAM
			free(CACHED(_pixels));

		_pixels = 0;
	}

	uint32 size = getSizeInBytes();

	if (inVram) {
		_pixels = (byte *)VramAllocator::instance().allocate(size);
	}

	if (!_pixels) {	// Either we are not in vram or we didn't manage to allocate in vram
		// Align to 64 bytes. All normal buffer sizes are multiples of 64 anyway
		_pixels = (byte *)memalign(64, size);
	}

	if (!_pixels) {
		PSP_ERROR("couldn't allocate buffer.\n");
		return false;
	}

	// Use uncached memory
	GuRenderer::cacheInvalidate(_pixels, size);
	_pixels = UNCACHED(_pixels);

	clear();
	return true;
}

void Buffer::deallocate() {
	DEBUG_ENTER_FUNC();

	if (!_pixels)
		return;

	if (VramAllocator::isAddressInVram(_pixels)) 	// Check if in VRAM
		VramAllocator::instance().deallocate(_pixels);
	else
		free(CACHED(_pixels));

	_pixels = 0;
}

void Buffer::clear() {
	DEBUG_ENTER_FUNC();

	if (_pixels)
		memset(_pixels, 0, getSizeInBytes());
}

/* Convert 4 bit images to match weird PSP format */
void Buffer::flipNibbles() {
	DEBUG_ENTER_FUNC();

	if (_pixelFormat.bitsPerPixel != 4)
		return;

	assert(_pixels);

	uint32 *dest = (uint32 *)_pixels;

	for (uint32 i = 0; i < getSourceHeight(); i++) {
		for (uint32 j = 0; j < (getWidth() >> 3); j++) {	// /8 because we do it in 32bit chunks
			uint32 val = *dest;
			*dest++ = ((val >> 4) & 0x0F0F0F0F) | ((val << 4) & 0xF0F0F0F0);
		}
	}
}

// Print buffer contents to screen (only source size is printed out)
void Buffer::print(uint32 mask, uint32 numToPrint /*=0*/) {
	assert(_pixels);

	if (numToPrint > _sourceSize.width * _sourceSize.height || numToPrint == 0)
		numToPrint = _sourceSize.width * _sourceSize.height;

	PSP_INFO_PRINT("buffer: \n");
	PSP_INFO_PRINT("width[%u], height[%u]\n\n", _sourceSize.width, _sourceSize.height);

	for (unsigned int i = 0; i < _sourceSize.height; i++) {
		for (unsigned int j = 0; j < _sourceSize.width; j++) {
			if (numToPrint <= 0)	// check if done
				break;

			byte *pcolor = &_pixels[_pixelFormat.pixelsToBytes((i * _width) + j)];
			uint32 color = _pixelFormat.getColorValueAt(pcolor);

			//if (color != 0) PSP_INFO_PRINT("[%x] ", color);
			PSP_INFO_PRINT("[%x] ", mask & color);

			numToPrint--;
		}
		PSP_INFO_PRINT("\n");
	}
	PSP_INFO_PRINT("\n");
}

// class GuRenderer -------------------------------------------------
//#define __PSP_DEBUG_FUNCS__	/* For debugging the stack */
//#define __PSP_DEBUG_PRINT__

#include "backends/platform/psp/trace.h"


void GuRenderer::render() {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("Buffer[%p] Palette[%p]\n", _buffer->getPixels(), _palette->getRawValues());

	guProgramDrawBehavior();

	if (_buffer->hasPalette())
		guLoadPalette();

	guProgramTextureFormat();

	// Loop over patches of 512x512 pixel textures and draw them
	for (uint32 j = 0; j < _buffer->getSourceHeight(); j += 512) {
		_textureLoadOffset.y = j;

		for (uint32 i = 0; i < _buffer->getSourceWidth(); i += 512) {
			_textureLoadOffset.x = i;

			guLoadTexture();
			Vertex *vertices = guGetVertices();
			fillVertices(vertices);

			guDrawVertices(vertices);
		}
	}
}

inline void GuRenderer::guProgramDrawBehavior() {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("blending[%s] colorTest[%s] reverseAlpha[%s] keyColor[%u]\n",
		_blending ? "on" : "off", _colorTest ? "on" : "off",
		_alphaReverse ? "on" : "off", _keyColor);

	if (_blending) {
		sceGuEnable(GU_BLEND);

		if (_alphaReverse)	// Reverse the alpha value (ie. 0 is 1) easier to do in some cases
			sceGuBlendFunc(GU_ADD, GU_ONE_MINUS_SRC_ALPHA, GU_SRC_ALPHA, 0, 0);
		else				// Normal alpha values
			sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

	} else
		sceGuDisable(GU_BLEND);

	if (_colorTest) {
		sceGuEnable(GU_COLOR_TEST);
		sceGuColorFunc(GU_NOTEQUAL, 	// show only colors not equal to this color
					   _keyColor,
					   0x00ffffff);		// match everything but alpha
	} else
		sceGuDisable(GU_COLOR_TEST);
}

inline void GuRenderer::guLoadPalette() {
	DEBUG_ENTER_FUNC();

	uint32 mask;

	if (_buffer->getBitsPerPixel() == 4)
		mask = 0x0F;
	else if (_buffer->getBitsPerPixel() == 8)
		mask = 0xFF;
	else
		assert(0);	/* error */

	PSP_DEBUG_PRINT("numOfEntries[%d]\n", _palette->getNumOfEntries());
	PSP_DEBUG_PRINT("bpp[%d], pixelformat[%d], mask[%x]\n", _buffer->getBitsPerPixel(), _palette->getPixelFormat(), mask);

	sceGuClutMode(convertToGuPixelFormat(_palette->getPixelFormat()), 0, mask, 0);
	sceGuClutLoad(_palette->getNumOfEntries() >> 3, 	// it's in batches of 8 for some reason
				  _palette->getRawValues());
}

inline void GuRenderer::guProgramTextureFormat() {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("pixelFormat[%d]\n", _buffer->getPixelFormat());

	sceGuTexMode(convertToGuPixelFormat(_buffer->getPixelFormat()), 0, 0, 0);
}

inline uint32 GuRenderer::convertToGuPixelFormat(PSPPixelFormat::Type format) {
	DEBUG_ENTER_FUNC();

	uint32 guFormat = 0;

	switch (format) {
	case PSPPixelFormat::Type_4444:
		guFormat = GU_PSM_4444;
		break;
	case PSPPixelFormat::Type_5551:
		guFormat = GU_PSM_5551;
		break;
	case PSPPixelFormat::Type_5650:
		guFormat = GU_PSM_5650;
		break;
	case PSPPixelFormat::Type_8888:
		guFormat = GU_PSM_8888;
		break;
	case PSPPixelFormat::Type_Palette_8bit:
		guFormat = GU_PSM_T8;
		break;
	case PSPPixelFormat::Type_Palette_4bit:
		guFormat = GU_PSM_T4;
		break;
	default:
		break;
	}

	PSP_DEBUG_PRINT("Pixelformat[%d], guFormat[%d]\n", format, guFormat);

	return guFormat;
}

inline void GuRenderer::guLoadTexture() {
	DEBUG_ENTER_FUNC();

	byte *startPoint = _buffer->getPixels();
	if (_textureLoadOffset.x)
		startPoint += _buffer->_pixelFormat.pixelsToBytes(_textureLoadOffset.x);
	if (_textureLoadOffset.y)
		startPoint += _buffer->getWidthInBytes() * _textureLoadOffset.y;

	sceGuTexImage(0,
				_buffer->getTextureWidth(), 	// texture width (must be power of 2)
				_buffer->getTextureHeight(), 	// texture height (must be power of 2)
				_buffer->getWidth(),			// width of a line of the image (to get to the next line)
				startPoint);					// where to start reading
}

inline Vertex *GuRenderer::guGetVertices() {
	DEBUG_ENTER_FUNC();

	Vertex *ret = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));

	return ret;
}

// Fills the vertices. Most of the logic is here.
void GuRenderer::fillVertices(Vertex *vertices) {
	DEBUG_ENTER_FUNC();

	uint32 outputWidth = _displayManager->getOutputWidth();
	uint32 outputHeight = _displayManager->getOutputHeight();

	// Texture adjustments for eliminating half-pixel artifacts from scaling
	// Not necessary if we don't scale
	float textureFix = 0.0f;
	if (_useGlobalScaler &&
	        (_displayManager->getScaleX() != 1.0f || _displayManager->getScaleY() != 1.0f))
			textureFix = 0.5f;

	// These coordinates describe an area within the texture. ie. we already loaded a square of texture,
	// now the coordinates within it are 0 to the edge of the area of the texture we want to draw
	float textureStartX = textureFix + _offsetInBuffer.x;
	float textureStartY = textureFix + _offsetInBuffer.y;

	int textureLeftX = _drawSize.width - _textureLoadOffset.x;
	if (textureLeftX > 512)
		textureLeftX = 512;
	int textureLeftY = _drawSize.height - _textureLoadOffset.y;
	if (textureLeftY > 512)
		textureLeftY = 512;
	float textureEndX = -textureFix + _offsetInBuffer.x + textureLeftX;
	float textureEndY = -textureFix + _offsetInBuffer.y + textureLeftY;
	// For scaling to the final image size, calculate the gaps on both sides
	uint32 gapX = _useGlobalScaler ? (PSP_SCREEN_WIDTH - outputWidth) >> 1 : 0;
	uint32 gapY = _useGlobalScaler ? (PSP_SCREEN_HEIGHT - outputHeight) >> 1 : 0;

	// Save scaled offset on screen
	float scaledOffsetOnScreenX = scaleSourceToOutput(true, _offsetOnScreen.x);
	float scaledOffsetOnScreenY = scaleSourceToOutput(false, _offsetOnScreen.y);

	float imageStartX = gapX + scaledOffsetOnScreenX + (scaleSourceToOutput(true, stretch(true, _textureLoadOffset.x)));
	float imageStartY = gapY + scaledOffsetOnScreenY + (scaleSourceToOutput(false, stretch(false, _textureLoadOffset.y)));

	float imageEndX, imageEndY;

	imageEndX = imageStartX + scaleSourceToOutput(true, stretch(true, textureLeftX));
	imageEndY = imageStartY + scaleSourceToOutput(false, stretch(false, textureLeftY));

	vertices[0].u = textureStartX;
	vertices[0].v = textureStartY;
	vertices[1].u = textureEndX;
	vertices[1].v = textureEndY;

	vertices[0].x = imageStartX;
	vertices[0].y = imageStartY;
	vertices[0].z = 0;
	vertices[1].x = imageEndX;
	vertices[1].y = imageEndY;
	vertices[1].z = 0;

	PSP_DEBUG_PRINT("TextureStart: X[%f] Y[%f] TextureEnd: X[%.1f] Y[%.1f]\n", textureStartX, textureStartY, textureEndX, textureEndY);
	PSP_DEBUG_PRINT("ImageStart:   X[%f] Y[%f] ImageEnd:   X[%.1f] Y[%.1f]\n", imageStartX, imageStartY, imageEndX, imageEndY);
}

/* Scale the input X/Y offset to appear in proper position on the screen */
inline float GuRenderer::scaleSourceToOutput(bool x, float offset) {
	float result;

	if (!_useGlobalScaler)
		result = offset;
	else if (!offset)
		result = 0.0f;
	else
		result = x ? offset * _displayManager->getScaleX() : offset * _displayManager->getScaleY();

	return result;
}

/* Scale the input X/Y offset to appear in proper position on the screen */
inline float GuRenderer::stretch(bool x, float size) {
	if (!_stretch)
		return size;
	return (x ? size * _stretchX : size * _stretchY);
}

inline void GuRenderer::guDrawVertices(Vertex *vertices) {
	DEBUG_ENTER_FUNC();

	// This function shouldn't need changing. The '32' here refers to floating point vertices.
	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, vertices);
}

void GuRenderer::cacheInvalidate(void *pointer, uint32 size) {
	sceKernelDcacheWritebackInvalidateRange(pointer, size);
}
