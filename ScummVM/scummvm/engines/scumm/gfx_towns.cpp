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

#include "scumm/scumm.h"
#include "scumm/charset.h"
#include "scumm/util.h"
#include "scumm/resource.h"

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE

namespace Scumm {

void ScummEngine::towns_drawStripToScreen(VirtScreen *vs, int dstX, int dstY, int srcX, int srcY, int width, int height) {
	if (width <= 0 || height <= 0)
		return;

	assert(_textSurface.pixels);

	int m = _textSurfaceMultiplier;

	uint8 *src1 = vs->getPixels(srcX, srcY);
	uint8 *src2 = (uint8 *)_textSurface.getBasePtr(srcX * m, (srcY + vs->topline - _screenTop) * m);
	uint8 *dst1 = _townsScreen->getLayerPixels(0, dstX, dstY);
	uint8 *dst2 = _townsScreen->getLayerPixels(1, dstX * m, dstY * m);

	int dp1 = _townsScreen->getLayerPitch(0) - width * _townsScreen->getLayerBpp(0);
	int dp2 = _townsScreen->getLayerPitch(1) - width * m * _townsScreen->getLayerBpp(1);
	int sp1 = vs->pitch - (width * vs->format.bytesPerPixel);
	int sp2 = _textSurface.pitch - width * m;

	if (vs->number == kMainVirtScreen || _game.id == GID_INDY3 || _game.id == GID_ZAK) {
		for (int h = 0; h < height; ++h) {
			if (_outputPixelFormat.bytesPerPixel == 2) {
				for (int w = 0; w < width; ++w) {
					*(uint16 *)dst1 = _16BitPalette[*src1++];
					dst1 += _outputPixelFormat.bytesPerPixel;
				}

				src1 += sp1;
				dst1 += dp1;
			} else {
				memcpy(dst1, src1, width);
				src1 += vs->pitch;
				dst1 += _townsScreen->getLayerPitch(0);
			}

			for (int sH = 0; sH < m; ++sH) {
				memcpy(dst2, src2, width * m);
				src2 += _textSurface.pitch;
				dst2 += _townsScreen->getLayerPitch(1);
			}
		}
	} else {
		dst1 = dst2;
		for (int h = 0; h < height; ++h) {
			for (int w = 0; w < width; ++w) {
				uint8 t = (*src1++) & 0x0f;
				memset(dst1, (t << 4) | t, m);
				dst1 += m;
			}

			dst1 = dst2;
			uint8 *src3 = src2;

			if (m == 2) {
				dst2 += _townsScreen->getLayerPitch(1);
				src3 += _townsScreen->getLayerPitch(1);
			}

			for (int w = 0; w < width * m; ++w) {
				*dst2++ = (*src3 | (*dst1 & _townsLayer2Mask[*src3]));
				*dst1 = (*src2 | (*dst1 & _townsLayer2Mask[*src2]));
				src2++;
				src3++;
				dst1++;
			}

			src1 += sp1;
			src2 = src3 + sp2;
			dst1 = dst2 + dp2;
			dst2 += dp2;
		}
	}

	_townsScreen->addDirtyRect(dstX * m, dstY * m, width * m, height * m);
}

bool ScummEngine::towns_isRectInStringBox(int x1, int y1, int x2, int y2) {
	if (_game.platform == Common::kPlatformFMTowns && _charset->_hasMask && y1 <= _curStringRect.bottom && x1 <= _curStringRect.right && y2 >= _curStringRect.top && x2 >= _curStringRect.left)
		return true;
	return false;
}

void ScummEngine::towns_restoreCharsetBg() {
	if (_curStringRect.left != -1) {
		restoreBackground(_curStringRect, 0);
		_curStringRect.left = -1;
		_charset->_hasMask = false;
		_nextLeft = _string[0].xpos;
	}

	_nextLeft = _string[0].xpos;
	_nextTop = _string[0].ypos;
}

#ifdef USE_RGB_COLOR
void ScummEngine::towns_setPaletteFromPtr(const byte *ptr, int numcolor) {
	setPaletteFromPtr(ptr, numcolor);

	if (_game.version == 5)
		towns_setTextPaletteFromPtr(_currentPalette);

	_townsOverrideShadowColor = 1;
	int m = 48;
	for (int i = 1; i < 16; ++i) {
		int val = _currentPalette[i * 3] + _currentPalette[i * 3 + 1] + _currentPalette[i * 3 + 2];
		if (m > val) {
			_townsOverrideShadowColor = i;
			m = val;
		}
	}
}

void ScummEngine::towns_setTextPaletteFromPtr(const byte *ptr) {
	memcpy(_textPalette, ptr, 48);
}
#endif

void ScummEngine::towns_setupPalCycleField(int x1, int y1, int x2, int y2) {
	if (_numCyclRects >= 10)
		return;
	_cyclRects[_numCyclRects].left = x1;
	_cyclRects[_numCyclRects].top = y1;
	_cyclRects[_numCyclRects].right = x2;
	_cyclRects[_numCyclRects].bottom = y2;
	_numCyclRects++;
	_townsPaletteFlags |= 1;
}

void ScummEngine::towns_processPalCycleField() {
	for (int i = 0; i < _numCyclRects; i++) {
		int x1 = _cyclRects[i].left - _virtscr[kMainVirtScreen].xstart;
		int x2 = _cyclRects[i].right - _virtscr[kMainVirtScreen].xstart;
		if (x1 < 0)
			x1 = 0;
		if (x2 > 320)
			x2 = 320;
		if (x2 > 0)
			markRectAsDirty(kMainVirtScreen, x1, x2, _cyclRects[i].top, _cyclRects[i].bottom);
	}
}

void ScummEngine::towns_resetPalCycleFields() {
	_numCyclRects = 0;
	_townsPaletteFlags &= ~1;
}

const uint8 ScummEngine::_townsLayer2Mask[] = {
	0xFF, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define DIRTY_RECTS_MAX 20
#define FULL_REDRAW (DIRTY_RECTS_MAX + 1)

TownsScreen::TownsScreen(OSystem *system, int width, int height, Graphics::PixelFormat &format) :
	_system(system), _width(width), _height(height), _pixelFormat(format), _pitch(width * format.bytesPerPixel) {
	memset(&_layers[0], 0, sizeof(TownsScreenLayer));
	memset(&_layers[1], 0, sizeof(TownsScreenLayer));
	_outBuffer = new byte[_pitch * _height];
	memset(_outBuffer, 0, _pitch * _height);

	setupLayer(0, width, height, 256);
}

TownsScreen::~TownsScreen() {
	delete[] _layers[0].pixels;
	delete[] _layers[1].pixels;
	delete[] _layers[0].bltInternX;
	delete[] _layers[1].bltInternX;
	delete[] _layers[0].bltInternY;
	delete[] _layers[1].bltInternY;
	delete[] _layers[0].bltTmpPal;
	delete[] _layers[1].bltTmpPal;
	delete[] _outBuffer;
	_dirtyRects.clear();
}

void TownsScreen::setupLayer(int layer, int width, int height, int numCol, void *pal) {
	if (layer < 0 || layer > 1)
		return;

	TownsScreenLayer *l = &_layers[layer];

	if (numCol >> 15)
		error("TownsScreen::setupLayer(): No more than 32767 colors supported.");

	if (width > _width || height > _height)
		error("TownsScreen::setupLayer(): Layer width/height must be equal or less than screen width/height");

	l->scaleW = _width / width;
	l->scaleH = _height / height;

	if ((float)l->scaleW !=	((float)_width / (float)width) || (float)l->scaleH != ((float)_height / (float)height))
		error("TownsScreen::setupLayer(): Layer width/height must be equal or an EXACT half, third, etc. of screen width/height.\n More complex aspect ratio scaling is not supported.");

	if (width <= 0 || height <= 0 || numCol < 16)
		error("TownsScreen::setupLayer(): Invalid width/height/number of colors setting.");

	l->height = height;
	l->numCol = numCol;
	l->bpp = ((numCol - 1) & 0xff00) ? 2 : 1;
	l->pitch = width * l->bpp;
	l->palette = (uint8 *)pal;

	if (l->palette && _pixelFormat.bytesPerPixel == 1)
		warning("TownsScreen::setupLayer(): Layer palette usage requires 16 bit graphics setting.\nLayer palette will be ignored.");

	delete[] l->pixels;
	l->pixels = new uint8[l->pitch * l->height];
	assert(l->pixels);
	memset(l->pixels, 0, l->pitch * l->height);

	// build offset tables to speed up merging/scaling layers
	delete[] l->bltInternX;
	l->bltInternX = new uint16[_width];
	for (int i = 0; i < _width; ++i)
		l->bltInternX[i] = (i / l->scaleW) * l->bpp;

	delete[] l->bltInternY;
	l->bltInternY = new uint8*[_height];
	for (int i = 0; i < _height; ++i)
		l->bltInternY[i] = l->pixels + (i / l->scaleH) * l->pitch;

	delete[] l->bltTmpPal;
	l->bltTmpPal = (l->bpp == 1 && _pixelFormat.bytesPerPixel == 2) ? new uint16[l->numCol] : 0;

	l->enabled = true;
	_layers[0].onBottom = true;
	_layers[1].onBottom = !_layers[0].enabled;
	l->ready = true;
}

void TownsScreen::clearLayer(int layer) {
	if (layer < 0 || layer > 1)
		return;

	TownsScreenLayer *l = &_layers[layer];
	if (!l->ready)
		return;

	memset(l->pixels, 0, l->pitch * l->height);
	_dirtyRects.push_back(Common::Rect(_width - 1, _height - 1));
	_numDirtyRects = FULL_REDRAW;
}


void TownsScreen::fillLayerRect(int layer, int x, int y, int w, int h, int col) {
	if (layer < 0 || layer > 1 || w <= 0 || h <= 0)
		return;

	TownsScreenLayer *l = &_layers[layer];
	if (!l->ready)
		return;

	assert(x >= 0 && y >= 0 && ((x + w) * l->bpp) <= (l->pitch) && (y + h) <= (l->height));

	uint8 *pos = l->pixels + y * l->pitch + x * l->bpp;

	for (int i = 0; i < h; ++i) {
		if (l->bpp == 2) {
			for (int ii = 0; ii < w; ++ii) {
				*(uint16 *)pos = col;
				pos += 2;
			}
			pos += (l->pitch - w * 2);
		} else {
			memset(pos, col, w);
			pos += l->pitch;
		}
	}
	addDirtyRect(x * l->scaleW, y * l->scaleH, w * l->scaleW, h * l->scaleH);
}

uint8 *TownsScreen::getLayerPixels(int layer, int x, int y) {
	if (layer < 0 || layer > 1)
		return 0;

	TownsScreenLayer *l = &_layers[layer];
	if (!l->ready)
		return 0;

	return l->pixels + y * l->pitch + x * l->bpp;
}

int TownsScreen::getLayerPitch(int layer) {
	if (layer >= 0 && layer < 2)
		return _layers[layer].pitch;
	return 0;
}

int TownsScreen::getLayerHeight(int layer) {
	if (layer >= 0 && layer < 2)
		return _layers[layer].height;
	return 0;
}

int TownsScreen::getLayerBpp(int layer) {
	if (layer >= 0 && layer < 2)
		return _layers[layer].bpp;
	return 0;
}

int TownsScreen::getLayerScaleW(int layer) {
	if (layer >= 0 && layer < 2)
		return _layers[layer].scaleW;
	return 0;
}

int TownsScreen::getLayerScaleH(int layer) {
	if (layer >= 0 && layer < 2)
		return _layers[layer].scaleH;
	return 0;
}

void TownsScreen::addDirtyRect(int x, int y, int w, int h) {
	if (w <= 0 || h <= 0 || _numDirtyRects > DIRTY_RECTS_MAX)
		return;

	if (_numDirtyRects == DIRTY_RECTS_MAX) {
		// full redraw
		_dirtyRects.clear();
		_dirtyRects.push_back(Common::Rect(_width - 1, _height - 1));
		_numDirtyRects++;
		return;
	}

	int x2 = x + w - 1;
	int y2 = y + h - 1;

	assert(x >= 0 && y >= 0 && x2 <= _width && y2 <= _height);

	bool skip = false;
	for (Common::List<Common::Rect>::iterator r = _dirtyRects.begin(); r != _dirtyRects.end(); ++r) {
		// Try to merge new rect with an existing rect (only once, since trying to merge
		// more than one overlapping rect would be causing more overhead than doing any good).
		if (x > r->left && x < r->right && y > r->top && y < r->bottom) {
			x = r->left;
			y = r->top;
			skip = true;
		}

		if (x2 > r->left && x2 < r->right && y > r->top && y < r->bottom) {
			x2 = r->right;
			y = r->top;
			skip = true;
		}

		if (x2 > r->left && x2 < r->right && y2 > r->top && y2 < r->bottom) {
			x2 = r->right;
			y2 = r->bottom;
			skip = true;
		}

		if (x > r->left && x < r->right && y2 > r->top && y2 < r->bottom) {
			x = r->left;
			y2 = r->bottom;
			skip = true;
		}

		if (skip) {
			r->left = x;
			r->top = y;
			r->right = x2;
			r->bottom = y2;
			break;
		}
	}

	if (!skip) {
		_dirtyRects.push_back(Common::Rect(x, y, x2, y2));
		_numDirtyRects++;
	}
}

void TownsScreen::toggleLayers(int flag) {
	if (flag < 0 || flag > 3)
		return;

	_layers[0].enabled = (flag & 1) ? true : false;
	_layers[0].onBottom = true;
	_layers[1].enabled = (flag & 2) ? true : false;
	_layers[1].onBottom = !_layers[0].enabled;

	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(_width - 1, _height - 1));
	_numDirtyRects = FULL_REDRAW;

	memset(_outBuffer, 0, _pitch * _height);
	update();

	_system->updateScreen();
}

void TownsScreen::update() {
	updateOutputBuffer();
	outputToScreen();
}

void TownsScreen::updateOutputBuffer() {
	for (Common::List<Common::Rect>::iterator r = _dirtyRects.begin(); r != _dirtyRects.end(); ++r) {
		for (int i = 0; i < 2; i++) {

			TownsScreenLayer *l = &_layers[i];
			if (!l->enabled || !l->ready)
				continue;

			uint8 *dst = _outBuffer + r->top * _pitch + r->left * _pixelFormat.bytesPerPixel;
			int ptch = _pitch - (r->right - r->left + 1) * _pixelFormat.bytesPerPixel;

			if (_pixelFormat.bytesPerPixel == 2 && l->bpp == 1) {
				if (!l->palette)
					error("void TownsScreen::updateOutputBuffer(): No palette assigned to 8 bit layer %d", i);
				for (int ic = 0; ic < l->numCol; ic++)
					l->bltTmpPal[ic] = calc16BitColor(&l->palette[ic * 3]);
			}

			for (int y = r->top; y <= r->bottom; ++y) {
				if (l->bpp == _pixelFormat.bytesPerPixel && l->scaleW == 1 && l->onBottom && l->numCol & 0xff00) {
					memcpy(dst, &l->bltInternY[y][l->bltInternX[r->left]], (r->right + 1 - r->left) * _pixelFormat.bytesPerPixel);
					dst += _pitch;

				} else if (_pixelFormat.bytesPerPixel == 2) {
					for (int x = r->left; x <= r->right; ++x) {
						uint8 *src = &l->bltInternY[y][l->bltInternX[x]];
						if (l->bpp == 1) {
							uint8 col = *src;
							if (col || l->onBottom) {
								if (l->numCol == 16)
									col = (col >> 4) & (col & 0x0f);
								*(uint16 *)dst = l->bltTmpPal[col];
							}
						} else {
							*(uint16 *)dst = *(uint16 *)src;
						}
						dst += 2;
					}
					dst += ptch;

				} else {
					for (int x = r->left; x <= r->right; ++x) {
						uint8 col = l->bltInternY[y][l->bltInternX[x]];
						if (col || l->onBottom) {
							if (l->numCol == 16)
								col = (col >> 4) & (col & 0x0f);
							*dst = col;
						}
						dst++;
					}
					dst += ptch;
				}
			}
		}
	}
}

void TownsScreen::outputToScreen() {
	for (Common::List<Common::Rect>::iterator i = _dirtyRects.begin(); i != _dirtyRects.end(); ++i)
		_system->copyRectToScreen(_outBuffer + i->top * _pitch + i->left * _pixelFormat.bytesPerPixel, _pitch, i->left, i->top, i->right - i->left + 1, i->bottom - i->top + 1);
	_dirtyRects.clear();
	_numDirtyRects = 0;
}

uint16 TownsScreen::calc16BitColor(const uint8 *palEntry) {
	return _pixelFormat.RGBToColor(palEntry[0], palEntry[1], palEntry[2]);
}

#undef DIRTY_RECTS_MAX
#undef FULL_REDRAW

} // End of namespace Scumm

#endif // DISABLE_TOWNS_DUAL_LAYER_MODE
