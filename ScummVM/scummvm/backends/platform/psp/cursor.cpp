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

#include "common/scummsys.h"
#include "backends/platform/psp/psppixelformat.h"
#include "backends/platform/psp/display_client.h"
#include "backends/platform/psp/default_display_client.h"
#include "backends/platform/psp/cursor.h"

//#define __PSP_DEBUG_FUNCS__	/* For debugging the stack */
//#define __PSP_DEBUG_PRINT__

#include "backends/platform/psp/trace.h"

void Cursor::init() {
	DEBUG_ENTER_FUNC();

	_renderer.setBuffer(&_buffer);			// We do this explicitly
	_renderer.setPalette(&_screenPalette);	// because we want to choose screenpalette by default
	_renderer.setUseGlobalScaler(true);
	setRendererModePalettized(true);		// Assume we start in 8bit mode

	// Default modes
	_palette.setPixelFormats(PSPPixelFormat::Type_5551, PSPPixelFormat::Type_Palette_8bit); // default
	_screenPalette.setPixelFormats(PSPPixelFormat::Type_5551, PSPPixelFormat::Type_Palette_8bit);
	_buffer.setPixelFormat(PSPPixelFormat::Type_5551);
}

void Cursor::deallocate() {
	DEBUG_ENTER_FUNC();

	_buffer.deallocate();
	_palette.deallocate();
	_screenPalette.deallocate();
}

void Cursor::setCursorPalette(const byte *colors, uint start, uint num) {
	DEBUG_ENTER_FUNC();

	if (!_palette.isAllocated()) {
		_palette.allocate();
	}

	// Workaround: This is wrong, but we seem to not be getting setScreenPalette
	if (!_screenPalette.isAllocated()) {
		_screenPalette.allocate();
	}

	_palette.setPartial(colors, start, num);
	setDirty();
}

void Cursor::setScreenPalette(const byte *colors, uint start, uint num) {
	DEBUG_ENTER_FUNC();

	if (!_screenPalette.isAllocated()) {
		_screenPalette.allocate();
	}

	_screenPalette.setPartial(colors, start, num);
	setDirty();
}

void Cursor::setKeyColor(uint32 color) {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("new color[%u], old color[%u]\n", color, _keyColor);

	// If it's a different color, undo the last keycolor
	if (_buffer.hasPalette() && color != _keyColor) {
		if (_screenPalette.isAllocated())
			_screenPalette.setColorPositionAlpha(_keyColor, true);
		if (_palette.isAllocated())
			_palette.setColorPositionAlpha(_keyColor, true);
	}
	// Don't need anything special for 16-bit
	_keyColor = color;
}

void Cursor::clearKeyColor() {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("keyColor[%d]\n", _keyColor);

	// We need 2 mechanisms: one for palettized and one for 16 bit
	if (_buffer.hasPalette()) {
		if (_screenPalette.isAllocated())
			_screenPalette.setColorPositionAlpha(_keyColor, false);		// set keycolor to 0
		if (_palette.isAllocated())
			_palette.setColorPositionAlpha(_keyColor, false);
	} else {	// 16bit
		_renderer.setKeyColor(_keyColor);
	}
	setDirty();
}

void Cursor::enableCursorPalette(bool enable) {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("enable[%s]\n", enable ? "true" : "false");

	_useCursorPalette = enable;
	if (enable)
		_renderer.setPalette(&_palette);	// very important that we do this switch
	else
		_renderer.setPalette(&_screenPalette);

	setDirty();
}

inline void Cursor::setSize(uint32 width, uint32 height) {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("width[%u], height[%u]\n", width, height);

	_buffer.setSize(width, height, Buffer::kSizeByTextureSize);	// we'll use texture size for mouse
	_renderer.setDrawWholeBuffer();		// We need to let the renderer know how much to draw
}

void Cursor::copyFromArray(const byte *array) {
	DEBUG_ENTER_FUNC();

	if (!_buffer.isAllocated())	{
		_buffer.allocate();
	}

	_buffer.copyFromArray(array, _buffer.getSourceWidthInBytes());	// pitch is source width
	setDirty();

	// debug
	//PSP_DEBUG_DO(_buffer.print(0xF));
}

void Cursor::setHotspot(int32 x, int32 y) {
	DEBUG_ENTER_FUNC();

	_hotspotX = x;
	_hotspotY = y;
	updateRendererOffset();	// Important

	PSP_DEBUG_PRINT("hotspotX[%d], hotspotY[%d]\n", x, y);
}

// Returns true if change in x or y
bool Cursor::increaseXY(int32 incX, int32 incY) {
	DEBUG_ENTER_FUNC();

	int32 oldX = _x, oldY = _y;

	// adjust for differences in X and Y
	adjustXYForScreenSize(incX, incY);

	_x += incX;
	_y += incY;

	// Clamp mouse
	if (_x < 0)
		_x = 0;
	if (_y < 0)
		_y = 0;
	if (_x >= (int)_mouseLimitWidth)
		_x = (int)_mouseLimitWidth - 1;
	if (_y >= (int)_mouseLimitHeight)
		_y = (int)_mouseLimitHeight - 1;

	PSP_DEBUG_PRINT("X[%d], Y[%d]\n", _x, _y);

	if (oldX != _x || oldY != _y) {
		updateRendererOffset();
		setDirty();
		return true;
	}

	return false;
}

// Set limits on the movement of the cursor ie. screen size
void Cursor::setLimits(uint32 width, uint32 height) {
#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272
	DEBUG_ENTER_FUNC();

	PSP_DEBUG_PRINT("width[%u], height[%u]\n", width, height);
	_mouseLimitWidth = width;
	_mouseLimitHeight = height;
}

// Adjust X,Y movement for the screen size to keep it consistent
inline void Cursor::adjustXYForScreenSize(int32 &x, int32 &y) {
	DEBUG_ENTER_FUNC();
	// We have our speed calibrated for the y axis at 480x272. The idea is to adjust this for other
	// resolutions and for x, which is wider.
	int32 newX = x, newY = y;

	// adjust width movement to match height (usually around 1.5)
	if (_mouseLimitWidth >= _mouseLimitHeight + (_mouseLimitHeight >> 1))
		newX = newX + (newX >> 1);

	if (_mouseLimitWidth >= 600) {	// multiply by 2
		newX <<= 1;
		newY <<= 1;
	} else if (_mouseLimitWidth >= 480) {	// multiply by 1.5
		newX = newX + (newX >> 1);
		newY = newY + (newY >> 1);
	}

	// Divide all movements by 8
	newX >>= 3;
	newY >>= 3;

	// Make sure we didn't destroy minimum movement
	if (!((x && !newX) || (y && !newY))) {
		x = newX;
		y = newY;
	}
}

// This is only called when we have a new screen
void Cursor::setScreenPaletteScummvmPixelFormat(const Graphics::PixelFormat *format) {
	DEBUG_ENTER_FUNC();

	uint32 oldPaletteSize = 0;
	if (_screenPalette.isAllocated())
		oldPaletteSize = _screenPalette.getSizeInBytes();

	PSPPixelFormat::Type bufferType = PSPPixelFormat::Type_Unknown;
	PSPPixelFormat::Type paletteType = PSPPixelFormat::Type_Unknown;
	bool swapRedBlue = false;

	// Convert Scummvm Pixel Format to PSPPixelFormat
	PSPPixelFormat::convertFromScummvmPixelFormat(format, bufferType, paletteType, swapRedBlue);

	if (paletteType == PSPPixelFormat::Type_None) {
		//_screenPalette.deallocate();		// leave palette for default CLUT8
		setRendererModePalettized(false);	// use 16-bit mechanism
	} else {	// We have a palette
		_screenPalette.setPixelFormats(paletteType, bufferType);
		_palette.setPixelFormats(paletteType, bufferType);
		setRendererModePalettized(true);	// use palettized mechanism
	}
}

// This is called many many times
void Cursor::setSizeAndScummvmPixelFormat(uint32 width, uint32 height, const Graphics::PixelFormat *format) {
	DEBUG_ENTER_FUNC();

	PSP_DEBUG_PRINT("useCursorPalette[%s]\n", _useCursorPalette ? "true" : "false");

	uint32 oldBufferSize = 0, oldPaletteSize = 0;

	if (_buffer.isAllocated())
		oldBufferSize = _buffer.getSizeInBytes();

	if (_palette.isAllocated())
		oldPaletteSize = _palette.getSizeInBytes();

	setSize(width, height);

	PSPPixelFormat::Type bufferType = PSPPixelFormat::Type_Unknown;
	PSPPixelFormat::Type paletteType = PSPPixelFormat::Type_Unknown;
	bool swapRedBlue = false;

	PSPPixelFormat::convertFromScummvmPixelFormat(format, bufferType, paletteType, swapRedBlue);
	PSP_DEBUG_PRINT("bufferType[%u], paletteType[%u]\n", bufferType, paletteType);

	// Check if we need to set new pixel format
	if (_buffer.getPixelFormat() != bufferType) {
		PSP_DEBUG_PRINT("new buffer pixel format[%u] is different from [%u]. Setting it.\n", bufferType, _buffer.getPixelFormat());
		_buffer.setPixelFormat(bufferType);
	}

	// Check if we need to reallocate
	if (_buffer.getSizeInBytes() != oldBufferSize) {
		_buffer.allocate();
		PSP_DEBUG_PRINT("reallocating buffer. new size: width[%u], height[%u]\n", width, height);
	}

	PSP_DEBUG_PRINT("palette pixel format[%u]\n", paletteType);

	if (paletteType == PSPPixelFormat::Type_None) {
		setRendererModePalettized(false);	// use palettized mechanism
	} else {	// We have a palette
		_palette.setPixelFormats(paletteType, bufferType);
		setRendererModePalettized(true);	// use palettized mechanism
	}

	// debug
	// PSP_DEBUG_DO(_palette.print(10));
	// PSP_DEBUG_DO(_screenPalette.print(10));
}

void Cursor::setXY(int x, int y) {
	DEBUG_ENTER_FUNC();

	_x = x;
	_y = y;
	updateRendererOffset();	// Very important to let renderer know things changed
	setDirty();
}

inline void Cursor::updateRendererOffset() {
	DEBUG_ENTER_FUNC();
	_renderer.setOffsetOnScreen(_x - _hotspotX, _y - _hotspotY);
}

inline void Cursor::setRendererModePalettized(bool palettized) {
	if (palettized) {	// We have a palette. Use blending
		_renderer.setAlphaBlending(true);
		_renderer.setAlphaReverse(false);
		_renderer.setColorTest(false);
	} else {			// 16 bits, no palette
		// Color test is an easy way for the hardware to make our keycolor
		// transparent.
		_renderer.setColorTest(true);

		// Alpha blending is not strictly required, but makes the cursor look
		// much better
		_renderer.setAlphaBlending(true);

		// Pixel formats without alpha (5650) are considered to have their alpha set.
		// Since pixel formats with alpha don't have their alpha bits set, we reverse
		// the alpha format for them so that 0 alpha is 1.
		if (_buffer.getPixelFormat() != PSPPixelFormat::Type_5650)
			_renderer.setAlphaReverse(true);
		else
			_renderer.setAlphaReverse(false);
	}
}
