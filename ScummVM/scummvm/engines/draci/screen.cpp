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

#include "common/memstream.h"
#include "common/system.h"

#include "graphics/palette.h"

#include "draci/draci.h"
#include "draci/screen.h"
#include "draci/surface.h"
#include "draci/sprite.h"


namespace Draci {

Screen::Screen(DraciEngine *vm) : _vm(vm) {
	_surface = new Surface(kScreenWidth, kScreenHeight);
	_palette = new byte[3 * kNumColors];
	_blackPalette = new byte[3 * kNumColors];
	for (int i = 0; i < 3 * kNumColors; ++i) {
		_blackPalette[i] = 0;
	}
	setPalette(NULL, 0, kNumColors);
	this->clearScreen();
}

Screen::~Screen() {
	delete _surface;
	delete[] _palette;
	delete[] _blackPalette;
}

/**
 * @brief Sets a part of the palette
 * @param data Pointer to a buffer containing new palette data
 *        start Index of the color where replacement should start
 *        num Number of colors to replace
 */
void Screen::setPalette(const byte *data, uint16 start, uint16 num) {
	Common::MemoryReadStream pal(data ? data : _blackPalette, 3 * kNumColors);
	pal.seek(start * 3);

	// Copy the palette
	for (uint16 i = start; i < start + num; ++i) {
		_palette[i * 3] = pal.readByte();
		_palette[i * 3 + 1] = pal.readByte();
		_palette[i * 3 + 2] = pal.readByte();
	}

	// Shift the palette two bits to the left to make it brighter.  The
	// original game only uses 6-bit colors 0..63.
	for (int i = start * 3; i < (start + num) * 3; ++i) {
		_palette[i] <<= 2;
	}

	_vm->_system->getPaletteManager()->setPalette(_palette, start, num);
}

void Screen::interpolatePalettes(const byte *first, const byte *second, uint16 start, uint16 num, int index, int number) {
	Common::MemoryReadStream firstPal(first ? first : _blackPalette, 3 * kNumColors);
	Common::MemoryReadStream secondPal(second ? second : _blackPalette, 3 * kNumColors);
	firstPal.seek(start * 3);
	secondPal.seek(start * 3);

	// Interpolate the palettes
	for (uint16 i = start; i < start + num; ++i) {
		_palette[i * 3] = interpolate(firstPal.readByte(), secondPal.readByte(), index, number);
		_palette[i * 3 + 1] = interpolate(firstPal.readByte(), secondPal.readByte(), index, number);
		_palette[i * 3 + 2] = interpolate(firstPal.readByte(), secondPal.readByte(), index, number);
	}

	// Shift the palette two bits to the left to make it brighter
	for (int i = start * 3; i < (start + num) * 3; ++i) {
		_palette[i] <<= 2;
	}

	_vm->_system->getPaletteManager()->setPalette(_palette, start, num);
}

int Screen::interpolate(int first, int second, int index, int number) {
	return (second * index + first * (number - index)) / number;
}

/**
 * @brief Copies the current memory screen buffer to the real screen
 */
void Screen::copyToScreen() {
	const Common::List<Common::Rect> *dirtyRects = _surface->getDirtyRects();
	Common::List<Common::Rect>::const_iterator it;

	// If a full update is needed, update the whole screen
	if (_surface->needsFullUpdate()) {
		byte *ptr = (byte *)_surface->getBasePtr(0, 0);

		_vm->_system->copyRectToScreen(ptr, kScreenWidth,
			0, 0, kScreenWidth, kScreenHeight);
	} else {
		// Otherwise, update only the dirty rectangles

		for (it = dirtyRects->begin(); it != dirtyRects->end(); ++it) {

			// Pointer to the upper left corner of the rectangle
			byte *ptr = (byte *)_surface->getBasePtr(it->left, it->top);

			_vm->_system->copyRectToScreen(ptr, kScreenWidth,
				it->left, it->top, it->width(), it->height());
		}
	}

	// Call the "real" updateScreen and mark the surface clean
	_vm->_system->updateScreen();
	_surface->markClean();
}

/**
 * @brief Clears the screen
 *
 * Clears the screen and marks the whole screen dirty.
 */
void Screen::clearScreen() {
	byte *ptr = (byte *)_surface->getBasePtr(0, 0);

	_surface->markDirty();

	memset(ptr, 0, kScreenWidth * kScreenHeight);
}

} // End of namespace Draci
