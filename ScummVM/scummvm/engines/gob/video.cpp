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

#include "common/endian.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "gob/gob.h"
#include "gob/video.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"

namespace Gob {

Font::Font(const byte *data) : _dataPtr(data) {
	assert(data);

	bool hasWidths = _dataPtr[0] & 0x80;

	_data       = _dataPtr + 4;
	_itemWidth  = _dataPtr[0] & 0x7F;
	_itemHeight = _dataPtr[1];
	_startItem  = _dataPtr[2];
	_endItem    = _dataPtr[3];
	_charWidths = 0;

	uint8 rowAlignedBits = (_itemWidth - 1) / 8 + 1;

	_itemSize = rowAlignedBits * _itemHeight;
	_bitWidth = _itemWidth;

	if (hasWidths)
		_charWidths = _dataPtr + 4 + _itemSize * getCharCount();
}

Font::~Font() {
	delete[] _dataPtr;
}

uint8 Font::getCharWidth(uint8 c) const {
	if (!_charWidths || (_endItem == 0))
		return _itemWidth;

	if ((c < _startItem) || (c > _endItem))
		return _itemWidth;

	return _charWidths[c - _startItem];
}

uint8 Font::getCharWidth() const {
	return _itemWidth;
}

uint8 Font::getCharHeight() const {
	return _itemHeight;
}

uint16 Font::getCharCount() const {
	return _endItem - _startItem + 1;
}

bool Font::hasChar(uint8 c) const {
	return (c >= _startItem) && (c <= _endItem);
}

bool Font::isMonospaced() const {
	return _charWidths == 0;
}

void Font::drawLetter(Surface &surf, uint8 c, uint16 x, uint16 y,
		uint32 color1, uint32 color2, bool transp) const {

	uint16 data;

	const byte *src = getCharData(c);
	if (!src) {
		warning("Font::drawLetter(): getCharData() == 0");
		return;
	}

	Pixel dst = surf.get(x, y);

	int nWidth = _itemWidth;
	if (nWidth & 7)
		nWidth = (nWidth & 0xF8) + 8;

	nWidth >>= 3;

	for (int i = 0; (i < _itemHeight) && dst.isValid(); i++) {
		int width = _itemWidth;

		for (int k = 0; k < nWidth; k++) {

			data = *src++;
			for (int j = 0; j < MIN(8, width); j++) {

				if (dst.isValid()) {
					if (data & 0x80)
						dst.set(color1);
					else if (!transp)
						dst.set(color2);
				}

				dst++;
				data <<= 1;
			}

			width -= 8;

		}

		dst += surf.getWidth() - _itemWidth;
	}
}

void Font::drawString(const Common::String &str, int16 x, int16 y, int16 color1, int16 color2,
                      bool transp, Surface &dest) const {

	const char *s = str.c_str();

	while (*s != '\0') {
		const int16 charRight  = x + getCharWidth(*s);
		const int16 charBottom = y + getCharHeight();

		if ((x >= 0) && (y >= 0) && (charRight <= dest.getWidth()) && (charBottom <= dest.getHeight()))
			drawLetter(dest, *s, x, y, color1, color2, transp);

		x += getCharWidth(*s);
		s++;
	}
}

const byte *Font::getCharData(uint8 c) const {
	if (_endItem == 0) {
		warning("Font::getCharData(): _endItem == 0");
		return 0;
	}

	if ((c < _startItem) || (c > _endItem))
		return 0;

	return _data + (c - _startItem) * _itemSize;
}


Video::Video(GobEngine *vm) : _vm(vm) {
	_doRangeClamp = false;

	_surfWidth = 320;
	_surfHeight = 200;

	_scrollOffsetX = 0;
	_scrollOffsetY = 0;

	_splitHeight1 = 200;
	_splitHeight2 = 0;
	_splitStart = 0;

	_screenDeltaX = 0;
	_screenDeltaY = 0;

	_curSparse = 0;
	_lastSparse = 0xFFFFFFFF;

	_dirtyAll = false;
}

Video::~Video() {
}

void Video::initPrimary(int16 mode) {
	if ((mode != 3) && (mode != -1))
		_vm->validateVideoMode(mode);
	_vm->validateVideoMode(_vm->_global->_videoMode);

	if (mode == -1)
		mode = 3;
	_vm->_global->_oldMode = mode;

	if (mode != 3) {
		initSurfDesc(_surfWidth, _surfHeight, PRIMARY_SURFACE);

		if (!_vm->_global->_dontSetPalette)
			Video::setFullPalette(_vm->_global->_pPaletteDesc);
	}
}

SurfacePtr Video::initSurfDesc(int16 width, int16 height, int16 flags) {
	SurfacePtr descPtr;

	if (flags & PRIMARY_SURFACE)
		assert((width == _surfWidth) && (height == _surfHeight));

	if (flags & PRIMARY_SURFACE) {
		_vm->_global->_primaryWidth = width;
		_vm->_global->_primaryHeight = height;

		descPtr = _vm->_global->_primarySurfDesc;
		descPtr->resize(width, height);
	} else {
		assert(!(flags & DISABLE_SPR_ALLOC));

		if (!(flags & SCUMMVM_CURSOR))
			width = (width + 7) & 0xFFF8;

		descPtr = SurfacePtr(new Surface(width, height, _vm->getPixelFormat().bytesPerPixel));
	}
	return descPtr;
}

void Video::clearScreen() {
	g_system->fillScreen(0);
}

void Video::setSize(bool defaultTo1XScaler) {
	if (_vm->isTrueColor())
		initGraphics(_vm->_width, _vm->_height, defaultTo1XScaler, 0);
	else
		initGraphics(_vm->_width, _vm->_height, defaultTo1XScaler);
}

void Video::retrace(bool mouse) {
	if (mouse)
		CursorMan.showMouse((_vm->_draw->_showCursor & 6) != 0);

	if (_vm->_global->_primarySurfDesc) {
		int screenX = _screenDeltaX;
		int screenY = _screenDeltaY;
		int screenWidth = MIN<int>(_surfWidth - _scrollOffsetX, _vm->_width);
		int screenHeight = MIN<int>(_surfHeight - _splitHeight2 - _scrollOffsetY, _vm->_height - _splitHeight2);

		dirtyRectsApply(_scrollOffsetX, _scrollOffsetY, screenWidth, screenHeight,
				screenX, screenY);

		if (_splitSurf) {

			screenX = 0;
			screenY = _vm->_height - _splitSurf->getHeight();
			screenWidth = MIN<int>(_vm->_width, _splitSurf->getWidth());
			screenHeight = _splitSurf->getHeight();

			_splitSurf->blitToScreen(0, 0, screenWidth - 1, screenHeight - 1, screenX, screenY);

		} else if (_splitHeight2 > 0) {

			screenX = 0;
			screenY = _vm->_height - _splitHeight2;
			screenWidth = MIN<int>(_surfWidth, _vm->_width);
			screenHeight = _splitHeight2;

			dirtyRectsApply(0, _splitStart, screenWidth, screenHeight, screenX, screenY);
		}

		dirtyRectsClear();
		g_system->updateScreen();
	}

}

void Video::waitRetrace(bool mouse) {
	uint32 time = _vm->_util->getTimeKey();
	retrace(mouse);
	_vm->_util->delay(MAX(1, 10 - (int)(_vm->_util->getTimeKey() - time)));
}

void Video::sparseRetrace(int max) {
	uint32 timeKey = _vm->_util->getTimeKey();

	if ((_curSparse++ > max) || ((timeKey - _lastSparse) > 1000)) {
		_curSparse = 0;
		retrace(false);
	}

	_lastSparse = timeKey;
}

void Video::drawPacked(byte *sprBuf, int16 width, int16 height,
		int16 x, int16 y, byte transp, Surface &dest) {

	int destRight = x + width;
	int destBottom = y + height;

	Pixel dst = dest.get(x, y);

	int curx = x;
	int cury = y;

	while (1) {
		uint8 val = *sprBuf++;
		unsigned int repeat = val & 7;
		val &= 0xF8;

		if (!(val & 8)) {
			repeat <<= 8;
			repeat |= *sprBuf++;
		}
		repeat++;
		val >>= 4;

		for (unsigned int i = 0; i < repeat; ++i) {
			if (curx < dest.getWidth() && cury < dest.getHeight())
				if (!transp || val)
					dst.set(val);

			dst++;
			curx++;
			if (curx == destRight) {
				dst += dest.getWidth() + x - curx;
				curx = x;
				cury++;
				if (cury == destBottom)
					return;
			}
		}

	}
}

void Video::drawPackedSprite(byte *sprBuf, int16 width, int16 height,
		int16 x, int16 y, int16 transp, Surface &dest) {

	if (spriteUncompressor(sprBuf, width, height, x, y, transp, dest))
		return;

	drawPacked(sprBuf, width, height, x, y, transp, dest);
}

void Video::drawPackedSprite(const char *path, Surface &dest, int width) {
	int32 size;
	byte *data = _vm->_dataIO->getFile(path, size);
	if (!data) {
		warning("Video::drawPackedSprite(): Failed to open sprite \"%s\"", path);
		return;
	}

	drawPackedSprite(data, width, dest.getHeight(), 0, 0, 0, dest);
	delete[] data;
}

void Video::setPalElem(int16 index, char red, char green, char blue,
		int16 unused, int16 vidMode) {
	byte pal[3];

	_vm->validateVideoMode(vidMode);

	_vm->_global->_redPalette[index] = red;
	_vm->_global->_greenPalette[index] = green;
	_vm->_global->_bluePalette[index] = blue;
	setPalColor(pal, red, green, blue);

	if (_vm->getPixelFormat().bytesPerPixel == 1)
		g_system->getPaletteManager()->setPalette(pal, index, 1);
}

void Video::setPalette(PalDesc *palDesc) {
	byte pal[768];
	int16 numcolors;

	_vm->validateVideoMode(_vm->_global->_videoMode);

	numcolors = _vm->_global->_setAllPalette ? 256 : 16;
	for (int i = 0; i < numcolors; i++)
		setPalColor(pal + i * 3, palDesc->vgaPal[i]);

	if (_vm->getPixelFormat().bytesPerPixel == 1)
		g_system->getPaletteManager()->setPalette(pal, 0, numcolors);
}

void Video::setFullPalette(PalDesc *palDesc) {
	if (_vm->_global->_setAllPalette) {
		byte pal[768];
		Color *colors = palDesc->vgaPal;

		for (int i = 0; i < 256; i++) {
			_vm->_global->_redPalette[i] = colors[i].red;
			_vm->_global->_greenPalette[i] = colors[i].green;
			_vm->_global->_bluePalette[i] = colors[i].blue;
			setPalColor(pal + i * 3, colors[i]);
		}

		if (_vm->getPixelFormat().bytesPerPixel == 1)
			g_system->getPaletteManager()->setPalette(pal, 0, 256);
	} else
		Video::setPalette(palDesc);
}

void Video::setPalette(Color *palette) {
	Color *palBak;
	bool setAllPalBak;

	palBak = _vm->_global->_pPaletteDesc->vgaPal;
	setAllPalBak = _vm->_global->_setAllPalette;

	_vm->_global->_pPaletteDesc->vgaPal = palette;
	_vm->_global->_setAllPalette = true;
	setFullPalette(_vm->_global->_pPaletteDesc);

	_vm->_global->_setAllPalette = setAllPalBak;
	_vm->_global->_pPaletteDesc->vgaPal = palBak;
}

void Video::dirtyRectsClear() {
	_dirtyRects.clear();
	_dirtyAll = false;
}

void Video::dirtyRectsAll() {
	_dirtyRects.clear();
	_dirtyAll = true;
}

void Video::dirtyRectsAdd(int16 left, int16 top, int16 right, int16 bottom) {
	if (_dirtyAll)
		return;

	_dirtyRects.push_back(Common::Rect(left, top, right + 1, bottom + 1));
}

void Video::dirtyRectsApply(int left, int top, int width, int height, int x, int y) {
	if (_dirtyAll) {
		_vm->_global->_primarySurfDesc->blitToScreen(left, top, left + width - 1, top + height - 1, x, y);
		return;
	}

	int right  = left + width;
	int bottom = top  + height;

	Common::List<Common::Rect>::const_iterator it;
	for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
		int l = MAX<int>(left  , it->left);
		int t = MAX<int>(top   , it->top);
		int r = MIN<int>(right , it->right);
		int b = MIN<int>(bottom, it->bottom);

		if ((r <= l) || (b <= t))
			continue;

		_vm->_global->_primarySurfDesc->blitToScreen(l, t, r - 1, b - 1, x + (l - left), y + (t - top));
	}
}

} // End of namespace Gob
