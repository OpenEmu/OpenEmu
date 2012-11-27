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

//#define __PSP_DEBUG_FUNCS__	/* For debugging the stack */
//#define __PSP_DEBUG_PRINT__

#include "backends/platform/psp/trace.h"

// Class DefaultDisplayClient ---------------------------------------------

bool DefaultDisplayClient::allocate(bool bufferInVram /* = false */, bool paletteInVram /* = false */) {
	DEBUG_ENTER_FUNC();

	if (!_buffer.allocate(bufferInVram)) {
		PSP_ERROR("Couldn't allocate buffer.\n");
		return false;
	}

	if (_buffer.hasPalette()) {
		PSP_DEBUG_PRINT("_palette[%p]\n", &_palette);

		if (!_palette.allocate()) {
			PSP_ERROR("Couldn't allocate palette.\n");
			return false;
		}
	}

	return true;
}

void DefaultDisplayClient::deallocate() {
	_buffer.deallocate();
	if (_buffer.hasPalette())
		_palette.deallocate();
}


void DefaultDisplayClient::clearBuffer() {
	DEBUG_ENTER_FUNC();
	_buffer.clear();
	setDirty();
}

inline void DefaultDisplayClient::clearPalette() {
	DEBUG_ENTER_FUNC();
	_palette.clear();
	setDirty();
}

void DefaultDisplayClient::init() {
	DEBUG_ENTER_FUNC();
	_renderer.setBuffer(&_buffer);
	_renderer.setPalette(&_palette);
}

void DefaultDisplayClient::copyFromRect(const byte *buf, int pitch, int destX, int destY, int recWidth, int recHeight) {
	DEBUG_ENTER_FUNC();
	_buffer.copyFromRect(buf, pitch, destX, destY, recWidth, recHeight);
	setDirty();
}

void DefaultDisplayClient::copyToArray(byte *dst, int pitch) {
	DEBUG_ENTER_FUNC();
	_buffer.copyToArray(dst, pitch);
}

// Class Overlay -------------------------------------------------------

void Overlay::init() {
	DEBUG_ENTER_FUNC();

	DefaultDisplayClient::init();
	_renderer.setAlphaBlending(true);
	_renderer.setColorTest(false);
	_renderer.setUseGlobalScaler(false);
	_renderer.setFullScreen(true);	// speeds up render slightly
}

void Overlay::setBytesPerPixel(uint32 size) {
	DEBUG_ENTER_FUNC();

	switch (size) {
	case 1:
		_buffer.setPixelFormat(PSPPixelFormat::Type_Palette_8bit);
		_palette.setPixelFormats(PSPPixelFormat::Type_4444, PSPPixelFormat::Type_Palette_8bit);
		break;
	case 2:
		_buffer.setPixelFormat(PSPPixelFormat::Type_4444);
		break;
	case 4:
		_buffer.setPixelFormat(PSPPixelFormat::Type_8888);
		break;
	}
}

void Overlay::setSize(uint32 width, uint32 height) {
	DEBUG_ENTER_FUNC();
	_buffer.setSize(width, height, Buffer::kSizeBySourceSize);
	_renderer.setDrawWholeBuffer();	// We need to let the renderer know how much to draw
}

void Overlay::copyToArray(void *buf, int pitch) {
	DEBUG_ENTER_FUNC();
	_buffer.copyToArray((byte *)buf, pitch);	// Change to bytes
}

void Overlay::copyFromRect(const void *buf, int pitch, int x, int y, int w, int h) {
	DEBUG_ENTER_FUNC();

	_buffer.copyFromRect((byte *)buf, pitch, x, y, w, h);	// Change to bytes
	// debug
	//_buffer.print(0xFF);
	setDirty();
}

bool Overlay::allocate() {
	DEBUG_ENTER_FUNC();

	bool ret = DefaultDisplayClient::allocate(true, false);	// buffer in VRAM

	return ret;
}

// Class Screen -----------------------------------------------------------

void Screen::init() {
	DEBUG_ENTER_FUNC();

	DefaultDisplayClient::init();
	_renderer.setAlphaBlending(false);
	_renderer.setColorTest(false);
	_renderer.setUseGlobalScaler(true);
	_renderer.setFullScreen(true);
}

void Screen::setShakePos(int pos) {
	_shakePos = pos;
	_renderer.setOffsetOnScreen(0, pos);
	setDirty();
}

void Screen::setSize(uint32 width, uint32 height) {
	DEBUG_ENTER_FUNC();

	_buffer.setSize(width, height, Buffer::kSizeBySourceSize);
	_renderer.setDrawWholeBuffer();	// We need to let the renderer know how much to draw
}

void Screen::setScummvmPixelFormat(const Graphics::PixelFormat *format) {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("format[%p], _buffer[%p], _palette[%p]\n", format, &_buffer, &_palette);

	if (!format) {
		bzero(&_pixelFormat, sizeof(_pixelFormat));
		_pixelFormat.bytesPerPixel = 1;	// default
	} else {
		_pixelFormat = *format;
	}

	PSPPixelFormat::Type bufferFormat, paletteFormat;
	bool swapRedBlue = false;

	PSPPixelFormat::convertFromScummvmPixelFormat(format, bufferFormat, paletteFormat, swapRedBlue);
	_buffer.setPixelFormat(bufferFormat, swapRedBlue);
	_palette.setPixelFormats(paletteFormat, bufferFormat, swapRedBlue);
}

Graphics::Surface *Screen::lockAndGetForEditing() {
	DEBUG_ENTER_FUNC();

	_frameBuffer.pixels = _buffer.getPixels();
	_frameBuffer.w = _buffer.getSourceWidth();
	_frameBuffer.h = _buffer.getSourceHeight();
	_frameBuffer.pitch = _buffer.getBytesPerPixel() * _buffer.getWidth();
	_frameBuffer.format = _pixelFormat;
	// We'll set to dirty once we unlock the screen

	return &_frameBuffer;
}

bool Screen::allocate() {
	DEBUG_ENTER_FUNC();

	return DefaultDisplayClient::allocate(true, false);	// buffer in VRAM
}
