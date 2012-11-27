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

#ifndef PSP_GRAPHICS_H
#define PSP_GRAPHICS_H

#include "common/singleton.h"
#include "graphics/surface.h"
#include "common/system.h"
#include "backends/platform/psp/memory.h"
#include "backends/platform/psp/psppixelformat.h"

#define MAX_TEXTURE_SIZE 512

class DisplayManager;
class GuRenderer;

/**
 *	Interface to inherit for all display clients
 *	We deliberately avoid virtual functions for speed.
 */
class DisplayClient {				// Abstract class
public:
	DisplayClient() {}
	bool isVisible() { return true; }
	bool isDirty() { return true; }
	void setClean() {}
	void render() {}
	virtual ~DisplayClient() {}
};

/**
 * Vertex used for GU rendering
 */
struct Vertex {
	float u, v;
	float x, y, z;
};

struct Point {
	int x;
	int y;
	Point() : x(0), y(0) {}
};

/**
 * Dimensions struct for simplification
 */
struct Dimensions {
	uint32 width;
	uint32 height;
	Dimensions() : width(0), height(0) {}
};

/**
 *	Universal PSP Palette class
 *	Use this in any class that wishes to draw to the PSP screen.
 *	Use together with GuRenderer
 */
class Palette {
public:
	Palette() : _values(0), _numOfEntries(0) {}
	virtual ~Palette() { deallocate(); }
	bool allocate();
	void deallocate();
	void clear();
	void setPixelFormats(PSPPixelFormat::Type paletteType, PSPPixelFormat::Type bufferType, bool swapRedBlue = false);
	void setNumOfEntries(uint32 num) {	_numOfEntries = num; }
	uint32 getNumOfEntries() { return _numOfEntries; }
	uint32 getSizeInBytes() { return _pixelFormat.pixelsToBytes(_numOfEntries); }
	void set(byte *values) { setPartial(values, 0, _numOfEntries); }
	void setPartial(const byte *colors, uint start, uint num, bool supportsAlpha = false);
	void getPartial(byte *colors, uint start, uint num);
	uint32 getRawColorAt(uint32 position);
	uint32 getRGBAColorAt(uint32 position);
	void setSingleColorRGBA(uint32 num, byte r, byte g, byte b, byte a);
	void setColorPositionAlpha(uint32 position, bool alpha);
	byte *getRawValues() { return _values; }
	bool isAllocated() { return (_values != 0); }
	PSPPixelFormat::Type getPixelFormat() { return _pixelFormat.format; }
	void print(uint32 numToPrint = 0);					// print to screen

protected:
	byte *_values;					///< array of palette data
	uint32 _numOfEntries;			///< number of palette entries
	PSPPixelFormat _pixelFormat;	///< pixel format of the palette data
};

/**
 *	Universal PSP buffer/texture object
 *	Use this in any class that wishes to draw to the PSP screen.
 *	Use together with GuRenderer
 */
class Buffer {
public:
	enum HowToSize {
		kSizeByTextureSize,	// buffer size is determined by power of 2 roundup for texture
		kSizeBySourceSize	// buffer size is determined by source size
	};

	Buffer() : _pixels(0), _width(0), _height(0)  {}
	virtual ~Buffer() { deallocate(); }

	// setters
	void setSize(uint32 width, uint32 height, HowToSize textureOrSource = kSizeByTextureSize);
	void setBitsPerPixel(uint32 bits) { _pixelFormat.bitsPerPixel = bits; }
	void setBytesPerPixel(uint32 bytes) { setBitsPerPixel(bytes << 3); }
	void setPixelFormat(PSPPixelFormat::Type type, bool swapRedBlue = false);

	// getters
	uint32 getWidth() { return _width; }
	uint32 getWidthInBytes() { return _pixelFormat.pixelsToBytes(getWidth()); }
	uint32 getHeight() { return _height; }
	uint32 getSourceWidth() { return _sourceSize.width; }
	uint32 getSourceWidthInBytes() { return _pixelFormat.pixelsToBytes(_sourceSize.width); }
	uint32 getSourceHeight() { return _sourceSize.height; }
	uint32 getTextureWidth() { return _textureSize.width; }
	uint32 getTextureHeight() { return _textureSize.height; }
	PSPPixelFormat::Type getPixelFormat() { return _pixelFormat.format; }
	uint32 getBitsPerPixel() { return _pixelFormat.bitsPerPixel; }
	uint32 getBytesPerPixel() { return getBitsPerPixel() >> 3; } /* won't work for 4-bit */
	byte *getPixels() { return _pixels; }
	uint32 getSizeInBytes() { return _pixelFormat.pixelsToBytes(_width * _height); }

	bool hasPalette();
	void copyFromArray(const byte *buffer, int pitch);
	void copyFromRect(const byte *buf, uint32 pitch, int destX, int destY, uint32 recWidth, uint32 recHeight);
	void copyToArray(byte *dst, int pitch);
	bool allocate(bool inVram = false);
	void deallocate();
	bool isAllocated() { return (_pixels != 0) ; }
	void clear();
	void flipNibbles();		// To handle peculiarities of PSP's 4 bit textures
	static uint32 scaleUpToPowerOfTwo(uint32 size);
	void print(uint32 mask, uint32 numToPrint = 0);

protected:
	friend class GuRenderer;
	byte *_pixels;
	uint32 _width;					///< True allocated width
	uint32 _height;					///< True allocated height
	Dimensions _textureSize;		///< Size rounded up to power of 2. Used for drawing
	Dimensions _sourceSize;			///< Original size of the buffer
	PSPPixelFormat _pixelFormat;	///< Format of the buffer
};

/**
 *	Universal rendering class for PSP
 *	Use this if you want to draw to the screen.
 *	Needs to be supplied with a Buffer and a Palette
 */
class GuRenderer {
public:
	// Constructors
	GuRenderer() : _useGlobalScaler(false), _buffer(0), _palette(0),
					_blending(false), _alphaReverse(false), _colorTest(false),
					_keyColor(0), _fullScreen(false), _stretch(false), _stretchX(1.0f), _stretchY(1.0f) {}
	GuRenderer(Buffer *buffer, Palette *palette) :
					_useGlobalScaler(false), _buffer(buffer), _palette(palette),
					_blending(false), _alphaReverse(false), _colorTest(false),
					_keyColor(0), _fullScreen(false), _stretch(false), _stretchX(1.0f), _stretchY(1.0f) {}
	static void setDisplayManager(DisplayManager *dm) { _displayManager = dm; } // Called by the Display Manager

	// Setters
	void setDrawSize(uint32 width, uint32 height) {	// How big of an area to draw
		_drawSize.width = width;
		_drawSize.height = height;
	}
	void setDrawWholeBuffer() {						// Draw the full size of the current buffer
		assert(_buffer);
		_drawSize.width = _buffer->getSourceWidth();
		_drawSize.height = _buffer->getSourceHeight();
	}
	void setBuffer(Buffer *buffer) { _buffer = buffer; }
	void setPalette(Palette *palette) { _palette = palette; }
	void setOffsetOnScreen(int x, int y) { _offsetOnScreen.x = x; _offsetOnScreen.y = y; }
	void setOffsetInBuffer(uint32 x, uint32 y) { _offsetInBuffer.x = x; _offsetInBuffer.y = y; }
	void setColorTest(bool value) { _colorTest = value; }
	void setKeyColor(uint32 value) { _keyColor = _buffer->_pixelFormat.convertTo32BitColor(value); }
	void setAlphaBlending(bool value) { _blending = value; }
	void setAlphaReverse(bool value) { _alphaReverse = value; }
	void setFullScreen(bool value) { _fullScreen = value; }		// Shortcut for rendering
	void setUseGlobalScaler(bool value) { _useGlobalScaler = value; }	// Scale to screen
	void setStretch(bool active) { _stretch = active; }
	void setStretchXY(float x, float y) { _stretchX = x; _stretchY = y; }

	static void cacheInvalidate(void *pointer, uint32 size);

	void render();							// Default rendering function. This should be good enough for most purposes

protected:
	// Gu functions
	void fillVertices(Vertex *vertices);	// Fill in vertices with coordinates
	void guProgramDrawBehavior();
	Vertex *guGetVertices();
	void guLoadTexture();
	void guLoadPalette();
	void guProgramTextureFormat();
	void guProgramTextureBitDepth();
	void guDrawVertices(Vertex *vertices);

	uint32 convertToGuPixelFormat(PSPPixelFormat::Type format);
	float scaleSourceToOutput(bool x, float offset);
	float stretch(bool x, float size);

	friend class MasterGuRenderer;
	Point _textureLoadOffset;		///> For rendering textures > 512 pixels
	Point _offsetOnScreen;			///> Where on screen to draw
	Point _offsetInBuffer;			///> Where in the texture to draw
	bool _useGlobalScaler;			///> Scale to the output size on screen
	Buffer *_buffer;
	Palette *_palette;
	static DisplayManager *_displayManager;
	Dimensions _drawSize;			///> Actual size to draw out of the Buffer
	bool _blending;
	bool _alphaReverse;				///> 0 counts as full alpha
	bool _colorTest;
	uint32 _keyColor;				///> Color to test against for color test. in 32 bits.
	bool _fullScreen;				///> Speeds up for fullscreen rendering
	bool _stretch;					///> Whether zooming is activated
	float _stretchX, _stretchY;
};

#endif /* PSP_SCREEN_H */
