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

#include "kyra/screen.h"
#include "kyra/kyra_v1.h"
#include "kyra/resource.h"

#include "common/endian.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/config-manager.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/sjis.h"

namespace Kyra {

Screen::Screen(KyraEngine_v1 *vm, OSystem *system, const ScreenDim *dimTable, const int dimTableSize)
	: _system(system), _vm(vm), _sjisInvisibleColor(0), _dimTable(dimTable), _dimTableCount(dimTableSize),
	_cursorColorKey((vm->game() == GI_KYRA1 || vm->game() == GI_EOB1 || vm->game() == GI_EOB2) ? 0xFF : 0) {
	_debugEnabled = false;
	_maskMinY = _maskMaxY = -1;

	_drawShapeVar1 = 0;
	_drawShapeVar3 = 1;
	_drawShapeVar4 = 0;
	_drawShapeVar5 = 0;

	memset(_fonts, 0, sizeof(_fonts));

	memset(_pagePtrs, 0, sizeof(_pagePtrs));
	// In VGA mode the odd and even page pointers point to the same buffers.
	for (int i = 0; i < SCREEN_PAGE_NUM; i++)
		_pageMapping[i] = i & ~1;

	_renderMode = Common::kRenderDefault;

	_currentFont = FID_8_FNT;
	_paletteChanged = true;
	_curDim = 0;
}

Screen::~Screen() {
	for (int i = 0; i < SCREEN_OVLS_NUM; ++i)
		delete[] _sjisOverlayPtrs[i];

	delete[] _pagePtrs[0];

	for (int f = 0; f < ARRAYSIZE(_fonts); ++f)
		delete _fonts[f];

	delete _screenPalette;
	delete _internFadePalette;
	delete[] _decodeShapeBuffer;
	delete[] _animBlockPtr;

	for (uint i = 0; i < _palettes.size(); ++i)
		delete _palettes[i];

	for (int i = 0; i < _dimTableCount; ++i)
		delete _customDimTable[i];
	delete[] _customDimTable;
}

bool Screen::init() {
	_debugEnabled = false;

	memset(_sjisOverlayPtrs, 0, sizeof(_sjisOverlayPtrs));
	_useOverlays = false;
	_useSJIS = false;
	_use16ColorMode = _vm->gameFlags().use16ColorMode;
	_isAmiga = (_vm->gameFlags().platform == Common::kPlatformAmiga);

	// We only check the "render_mode" setting for both Eye of the Beholder
	// games here, since all the other games do not support the render_mode
	// setting or handle it differently, like Kyra 1 PC-98. This avoids
	// graphics glitches and crashes in other games, when the user sets his
	// global render_mode setting to EGA for example.
	// TODO/FIXME: It would be nice not to hardcode this. But there is no
	// trivial/non annoying way to do mode checks in an easy fashion right
	// now.
	// In a more general sense, we might want to think about a way to only
	// pass valid config values, as in values which the engine can work with,
	// to the engines. We already limit the selection via our GUIO flags in
	// the game specific settings, but this is not enough due to global
	// settings allowing everything.
	if (_vm->game() == GI_EOB1 || _vm->game() == GI_EOB2) {
		if (ConfMan.hasKey("render_mode"))
			_renderMode = Common::parseRenderMode(ConfMan.get("render_mode"));
	}

	// CGA and EGA modes use additional pages to do the CGA/EGA specific graphics conversions.
	if (_vm->game() == GI_EOB1 && (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderEGA)) {
		for (int i = 0; i < 8; i++)
			_pageMapping[i] = i;
	}

	memset(_fonts, 0, sizeof(_fonts));

	_useOverlays = (_vm->gameFlags().useHiRes && _renderMode != Common::kRenderEGA);

	if (_useOverlays) {
		_useSJIS = (_vm->gameFlags().lang == Common::JA_JPN);
		_sjisInvisibleColor = (_vm->game() == GI_KYRA1) ? 0x80 : 0xF6;

		for (int i = 0; i < SCREEN_OVLS_NUM; ++i) {
			if (!_sjisOverlayPtrs[i]) {
				_sjisOverlayPtrs[i] = new uint8[SCREEN_OVL_SJIS_SIZE];
				assert(_sjisOverlayPtrs[i]);
				memset(_sjisOverlayPtrs[i], _sjisInvisibleColor, SCREEN_OVL_SJIS_SIZE);
			}
		}

		if (_useSJIS) {
			Graphics::FontSJIS *font = Graphics::FontSJIS::createFont(_vm->gameFlags().platform);

			if (!font)
				error("Could not load any SJIS font, neither the original nor ScummVM's 'SJIS.FNT'");

			_fonts[FID_SJIS_FNT] = new SJISFont(font, _sjisInvisibleColor, _use16ColorMode, !_use16ColorMode);
		}
	}

	_curPage = 0;

	Common::Array<uint8> realPages;
	for (int i = 0; i < SCREEN_PAGE_NUM; i++) {
		if (Common::find(realPages.begin(), realPages.end(), _pageMapping[i]) == realPages.end())
			realPages.push_back(_pageMapping[i]);
	}

	int numPages = realPages.size();
	uint32 bufferSize = numPages * SCREEN_PAGE_SIZE;

	uint8 *pagePtr = new uint8[bufferSize];
	memset(pagePtr, 0, bufferSize);

	memset(_pagePtrs, 0, sizeof(_pagePtrs));
	for (int i = 0; i < SCREEN_PAGE_NUM; i++) {
		if (_pagePtrs[_pageMapping[i]]) {
			_pagePtrs[i] = _pagePtrs[_pageMapping[i]];
		} else {
			_pagePtrs[i] = pagePtr;
			pagePtr += SCREEN_PAGE_SIZE;
		}
	}

	memset(_shapePages, 0, sizeof(_shapePages));

	const int paletteCount = _isAmiga ? 13 : 4;
	// We allow 256 color palettes in EGA mode, since original EOB II code does the same and requires it
	const int numColors = _use16ColorMode ? 16 : (_isAmiga ? 32 : (_renderMode == Common::kRenderCGA ? 4 : 256));

	_interfacePaletteEnabled = false;

	_screenPalette = new Palette(numColors);
	assert(_screenPalette);

	_palettes.resize(paletteCount);
	for (int i = 0; i < paletteCount; ++i) {
		_palettes[i] = new Palette(numColors);
		assert(_palettes[i]);
	}

	// Setup CGA colors (if CGA mode is selected)
	if (_renderMode == Common::kRenderCGA) {
		Palette pal(5);
		pal.setCGAPalette(1, Palette::kIntensityHigh);
		// create additional black color 4 for use with the mouse cursor manager
		pal.fill(4, 1, 0);
		Screen::setScreenPalette(pal);
	}

	_internFadePalette = new Palette(numColors);
	assert(_internFadePalette);

	setScreenPalette(getPalette(0));

	// We setup the PC98 text mode palette at [16, 24], since that will be used
	// for KANJI characters in Lands of Lore.
	if (_use16ColorMode && _vm->gameFlags().platform == Common::kPlatformPC98) {
		uint8 palette[8 * 3];

		for (int i = 0; i < 8; ++i) {
			palette[i * 3 + 0] = ((i >> 1) & 1) * 0xFF;
			palette[i * 3 + 1] = ((i >> 2) & 1) * 0xFF;
			palette[i * 3 + 2] = ((i >> 0) & 1) * 0xFF;
		}

		_system->getPaletteManager()->setPalette(palette, 16, 8);
	}

	_customDimTable = new ScreenDim *[_dimTableCount];
	memset(_customDimTable, 0, sizeof(ScreenDim *) * _dimTableCount);

	_curDimIndex = -1;
	_curDim = 0;
	_charWidth = 0;
	_charOffset = 0;
	for (int i = 0; i < ARRAYSIZE(_textColorsMap); ++i)
		_textColorsMap[i] = i;
	_decodeShapeBuffer = NULL;
	_decodeShapeBufferSize = 0;
	_animBlockPtr = NULL;
	_animBlockSize = 0;
	_mouseLockCount = 1;
	CursorMan.showMouse(false);

	_forceFullUpdate = false;

	return true;
}

bool Screen::enableScreenDebug(bool enable) {
	bool temp = _debugEnabled;

	if (_debugEnabled != enable) {
		_debugEnabled = enable;
		setResolution();
		_forceFullUpdate = true;
		updateScreen();
	}

	return temp;
}

void Screen::setResolution() {
	byte palette[3*256];
	_system->getPaletteManager()->grabPalette(palette, 0, 256);

	int width = 320, height = 200;
	bool defaultTo1xScaler = false;

	if (_vm->gameFlags().useHiRes) {
		defaultTo1xScaler = true;
		height = 400;

		if (_debugEnabled)
			width = 960;
		else
			width = 640;
	} else {
		if (_debugEnabled)
			width = 640;
		else
			width = 320;
	}

	initGraphics(width, height, defaultTo1xScaler);

	_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void Screen::updateScreen() {
	bool needRealUpdate = _forceFullUpdate || !_dirtyRects.empty() || _paletteChanged;
	_paletteChanged = false;

	if (_useOverlays)
		updateDirtyRectsOvl();
	else if (_isAmiga && _interfacePaletteEnabled)
		updateDirtyRectsAmiga();
	else
		updateDirtyRects();

	if (_debugEnabled) {
		needRealUpdate = true;

		if (!_useOverlays)
			_system->copyRectToScreen(getPagePtr(2), SCREEN_W, 320, 0, SCREEN_W, SCREEN_H);
		else
			_system->copyRectToScreen(getPagePtr(2), SCREEN_W, 640, 0, SCREEN_W, SCREEN_H);
	}

	if (needRealUpdate)
		_system->updateScreen();
}

void Screen::updateDirtyRects() {
	if (_forceFullUpdate) {
		_system->copyRectToScreen(getCPagePtr(0), SCREEN_W, 0, 0, SCREEN_W, SCREEN_H);
	} else {
		const byte *page0 = getCPagePtr(0);
		Common::List<Common::Rect>::iterator it;
		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			_system->copyRectToScreen(page0 + it->top * SCREEN_W + it->left, SCREEN_W, it->left, it->top, it->width(), it->height());
		}
	}
	_forceFullUpdate = false;
	_dirtyRects.clear();
}

void Screen::updateDirtyRectsAmiga() {
	if (_forceFullUpdate) {
		_system->copyRectToScreen(getCPagePtr(0), SCREEN_W, 0, 0, SCREEN_W, 136);

		// Page 8 is not used by Kyra 1 AMIGA, thus we can use it to adjust the colors
		copyRegion(0, 136, 0, 0, 320, 64, 0, 8, CR_NO_P_CHECK);

		uint8 *dst = getPagePtr(8);
		for (int y = 0; y < 64; ++y)
			for (int x = 0; x < 320; ++x)
				*dst++ += 32;

		_system->copyRectToScreen(getCPagePtr(8), SCREEN_W, 0, 136, SCREEN_W, 64);
	} else {
		const byte *page0 = getCPagePtr(0);
		Common::List<Common::Rect>::iterator it;

		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			if (it->bottom <= 136) {
				_system->copyRectToScreen(page0 + it->top * SCREEN_W + it->left, SCREEN_W, it->left, it->top, it->width(), it->height());
			} else {
				// Check whether the rectangle is part of both the screen and the interface
				if (it->top < 136) {
					// The rectangle covers both screen part and interface part

					const int screenHeight = 136 - it->top;
					const int interfaceHeight = it->bottom - 136;

					const int width = it->width();
					const int lineAdd = SCREEN_W - width;

					// Copy the screen part verbatim
					_system->copyRectToScreen(page0 + it->top * SCREEN_W + it->left, SCREEN_W, it->left, it->top, width, screenHeight);

					// Adjust the interface part
					copyRegion(it->left, 136, 0, 0, width, interfaceHeight, 0, 8, Screen::CR_NO_P_CHECK);

					uint8 *dst = getPagePtr(8);
					for (int y = 0; y < interfaceHeight; ++y) {
						for (int x = 0; x < width; ++x)
							*dst++ += 32;
						dst += lineAdd;
					}

					_system->copyRectToScreen(getCPagePtr(8), SCREEN_W, it->left, 136, width, interfaceHeight);
				} else {
					// The rectangle only covers the interface part

					const int width = it->width();
					const int height = it->height();
					const int lineAdd = SCREEN_W - width;

					copyRegion(it->left, it->top, 0, 0, width, height, 0, 8, Screen::CR_NO_P_CHECK);

					uint8 *dst = getPagePtr(8);
					for (int y = 0; y < height; ++y) {
						for (int x = 0; x < width; ++x)
							*dst++ += 32;
						dst += lineAdd;
					}

					_system->copyRectToScreen(getCPagePtr(8), SCREEN_W, it->left, it->top, width, height);
				}
			}
		}
	}

	_forceFullUpdate = false;
	_dirtyRects.clear();
}

void Screen::updateDirtyRectsOvl() {
	if (_forceFullUpdate) {
		const byte *src = getCPagePtr(0);
		byte *dst = _sjisOverlayPtrs[0];

		scale2x(dst, 640, src, SCREEN_W, SCREEN_W, SCREEN_H);
		mergeOverlay(0, 0, 640, 400);
		_system->copyRectToScreen(dst, 640, 0, 0, 640, 400);
	} else {
		const byte *page0 = getCPagePtr(0);
		byte *ovl0 = _sjisOverlayPtrs[0];

		Common::List<Common::Rect>::iterator it;
		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			byte *dst = ovl0 + it->top * 1280 + (it->left<<1);
			const byte *src = page0 + it->top * SCREEN_W + it->left;

			scale2x(dst, 640, src, SCREEN_W, it->width(), it->height());
			mergeOverlay(it->left<<1, it->top<<1, it->width()<<1, it->height()<<1);
			_system->copyRectToScreen(dst, 640, it->left<<1, it->top<<1, it->width()<<1, it->height()<<1);
		}
	}

	_forceFullUpdate = false;
	_dirtyRects.clear();
}

void Screen::scale2x(byte *dst, int dstPitch, const byte *src, int srcPitch, int w, int h) {
	byte *dstL1 = dst;
	byte *dstL2 = dst + dstPitch;

	int dstAdd = dstPitch * 2 - w * 2;
	int srcAdd = srcPitch - w;

	while (h--) {
		for (int x = 0; x < w; ++x, dstL1 += 2, dstL2 += 2) {
			uint16 col = *src++;
			col |= col << 8;
			*(uint16 *)(dstL1) = col;
			*(uint16 *)(dstL2) = col;
		}
		dstL1 += dstAdd; dstL2 += dstAdd;
		src += srcAdd;
	}
}

void Screen::mergeOverlay(int x, int y, int w, int h) {
	byte *dst = _sjisOverlayPtrs[0] + y * 640 + x;
	const byte *src = _sjisOverlayPtrs[1] + y * 640 + x;

	int add = 640 - w;

	while (h--) {
		for (x = 0; x < w; ++x, ++dst) {
			byte col = *src++;
			if (col != _sjisInvisibleColor)
				*dst = col;
		}
		dst += add;
		src += add;
	}
}

const ScreenDim *Screen::getScreenDim(int dim) const {
	assert(dim < _dimTableCount);
	return _customDimTable[dim] ? _customDimTable[dim] : &_dimTable[dim];
}

void Screen::modifyScreenDim(int dim, int x, int y, int w, int h) {
	if (!_customDimTable[dim])
		_customDimTable[dim] = new ScreenDim;

	memcpy(_customDimTable[dim], &_dimTable[dim], sizeof(ScreenDim));
	_customDimTable[dim]->sx = x;
	_customDimTable[dim]->sy = y;
	_customDimTable[dim]->w = w;
	_customDimTable[dim]->h = h;
	if (dim == _curDimIndex || _vm->game() == GI_LOL)
		setScreenDim(dim);
}

void Screen::setScreenDim(int dim) {
	_curDim = getScreenDim(dim);
	_curDimIndex = dim;
}

uint8 *Screen::getPagePtr(int pageNum) {
	assert(pageNum < SCREEN_PAGE_NUM);
	return _pagePtrs[pageNum];
}

const uint8 *Screen::getCPagePtr(int pageNum) const {
	assert(pageNum < SCREEN_PAGE_NUM);
	return _pagePtrs[pageNum];
}

uint8 *Screen::getPageRect(int pageNum, int x, int y, int w, int h) {
	assert(pageNum < SCREEN_PAGE_NUM);
	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, w, h);
	return _pagePtrs[pageNum] + y * SCREEN_W + x;
}

void Screen::clearPage(int pageNum) {
	assert(pageNum < SCREEN_PAGE_NUM);
	if (pageNum == 0 || pageNum == 1)
		_forceFullUpdate = true;
	memset(getPagePtr(pageNum), 0, SCREEN_PAGE_SIZE);
	clearOverlayPage(pageNum);
}

int Screen::setCurPage(int pageNum) {
	assert(pageNum < SCREEN_PAGE_NUM);
	int previousPage = _curPage;
	_curPage = pageNum;
	return previousPage;
}

void Screen::clearCurPage() {
	if (_curPage == 0 || _curPage == 1)
		_forceFullUpdate = true;
	memset(getPagePtr(_curPage), 0, SCREEN_PAGE_SIZE);
	clearOverlayPage(_curPage);
}

void Screen::copyWsaRect(int x, int y, int w, int h, int dimState, int plotFunc, const uint8 *src,
						int unk1, const uint8 *unkPtr1, const uint8 *unkPtr2) {
	uint8 *dstPtr = getPagePtr(_curPage);
	uint8 *origDst = dstPtr;

	const ScreenDim *dim = getScreenDim(dimState);
	int dimX1 = dim->sx << 3;
	int dimX2 = dim->w << 3;
	dimX2 += dimX1;

	int dimY1 = dim->sy;
	int dimY2 = dim->h;
	dimY2 += dimY1;

	int temp = y - dimY1;
	if (temp < 0) {
		if ((temp += h) <= 0)
			return;
		else {
			SWAP(temp, h);
			y += temp - h;
			src += (temp - h) * w;
		}
	}

	temp = dimY2 - y;
	if (temp <= 0)
		return;

	if (temp < h)
		h = temp;

	int srcOffset = 0;
	temp = x - dimX1;
	if (temp < 0) {
		temp = -temp;
		srcOffset = temp;
		x += temp;
		w -= temp;
	}

	int srcAdd = 0;

	temp = dimX2 - x;
	if (temp <= 0)
		return;

	if (temp < w) {
		SWAP(w, temp);
		temp -= w;
		srcAdd = temp;
	}

	dstPtr += y * SCREEN_W + x;
	uint8 *dst = dstPtr;

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x, y, w, h);

	if (!_use16ColorMode)
		clearOverlayRect(_curPage, x, y, w, h);

	temp = h;
	int curY = y;
	while (h--) {
		src += srcOffset;
		++curY;
		int cW = w;

		switch (plotFunc) {
		case 0:
			memcpy(dst, src, cW);
			dst += cW; src += cW;
			break;

		case 1:
			while (cW--) {
				uint8 d = *src++;
				uint8 t = unkPtr1[d];
				if (t != 0xFF)
					d = unkPtr2[*dst + (t << 8)];
				*dst++ = d;
			}
			break;

		case 4:
			while (cW--) {
				uint8 d = *src++;
				if (d)
					*dst = d;
				++dst;
			}
			break;

		case 5:
			while (cW--) {
				uint8 d = *src++;
				if (d) {
					uint8 t = unkPtr1[d];
					if (t != 0xFF)
						d = unkPtr2[*dst + (t << 8)];
					*dst = d;
				}
				++dst;
			}
			break;

		case 8:
		case 9:
			while (cW--) {
				uint8 d = *src++;
				uint8 t = _shapePages[0][dst - origDst] & 7;
				if (unk1 < t && (curY > _maskMinY && curY < _maskMaxY))
					d = _shapePages[1][dst - origDst];
				*dst++ = d;
			}
			break;

		case 12:
		case 13:
			while (cW--) {
				uint8 d = *src++;
				if (d) {
					uint8 t = _shapePages[0][dst - origDst] & 7;
					if (unk1 < t && (curY > _maskMinY && curY < _maskMaxY))
						d = _shapePages[1][dst - origDst];
					*dst++ = d;
				} else {
					d = _shapePages[1][dst - origDst];
					*dst++ = d;
				}
			}
			break;

		default:
			break;
		}

		dst = (dstPtr += SCREEN_W);
		src += srcAdd;
	}
}

uint8 Screen::getPagePixel(int pageNum, int x, int y) {
	assert(pageNum < SCREEN_PAGE_NUM);
	assert(x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H);
	return _pagePtrs[pageNum][y * SCREEN_W + x];
}

void Screen::setPagePixel(int pageNum, int x, int y, uint8 color) {
	assert(pageNum < SCREEN_PAGE_NUM);
	assert(x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H);

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, 1, 1);

	if (_use16ColorMode) {
		color &= 0x0F;
		color |= (color << 4);
	} else if (_renderMode == Common::kRenderCGA) {
		color &= 0x03;
	} else if (_renderMode == Common::kRenderEGA && !_useHiResEGADithering) {
		color &= 0x0F;
	}

	_pagePtrs[pageNum][y * SCREEN_W + x] = color;
}

void Screen::fadeFromBlack(int delay, const UpdateFunctor *upFunc) {
	fadePalette(getPalette(0), delay, upFunc);
}

void Screen::fadeToBlack(int delay, const UpdateFunctor *upFunc) {
	if (_renderMode == Common::kRenderEGA)
		return;

	Palette pal(getPalette(0).getNumColors());
	fadePalette(pal, delay, upFunc);
}

void Screen::fadePalette(const Palette &pal, int delay, const UpdateFunctor *upFunc) {
	if (_renderMode == Common::kRenderEGA)
		setScreenPalette(pal);

	updateScreen();

	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderEGA)
		return;

	int diff = 0, delayInc = 0;
	getFadeParams(pal, delay, delayInc, diff);

	int delayAcc = 0;
	while (!_vm->shouldQuit()) {
		delayAcc += delayInc;

		int refreshed = fadePalStep(pal, diff);

		if (upFunc && upFunc->isValid())
			(*upFunc)();
		else
			_system->updateScreen();

		if (!refreshed)
			break;

		_vm->delay((delayAcc >> 8) * 1000 / 60);
		delayAcc &= 0xFF;
	}
}

void Screen::getFadeParams(const Palette &pal, int delay, int &delayInc, int &diff) {
	uint8 maxDiff = 0;

	for (int i = 0; i < pal.getNumColors() * 3; ++i) {
		diff = ABS(pal[i] - (*_screenPalette)[i]);
		maxDiff = MAX<uint8>(maxDiff, diff);
	}

	delayInc = (delay << 8) & 0x7FFF;
	if (maxDiff != 0)
		delayInc /= maxDiff;

	delay = delayInc;
	for (diff = 1; diff <= maxDiff; ++diff) {
		if (delayInc >= 512)
			break;
		delayInc += delay;
	}
}

int Screen::fadePalStep(const Palette &pal, int diff) {
	_internFadePalette->copy(*_screenPalette);

	bool needRefresh = false;

	for (int i = 0; i < pal.getNumColors() * 3; ++i) {
		int c1 = pal[i];
		int c2 = (*_internFadePalette)[i];
		if (c1 != c2) {
			needRefresh = true;
			if (c1 > c2) {
				c2 += diff;
				if (c1 < c2)
					c2 = c1;
			}

			if (c1 < c2) {
				c2 -= diff;
				if (c1 > c2)
					c2 = c1;
			}

			(*_internFadePalette)[i] = (uint8)c2;
		}
	}

	if (needRefresh)
		setScreenPalette(*_internFadePalette);

	return needRefresh ? 1 : 0;
}

void Screen::setPaletteIndex(uint8 index, uint8 red, uint8 green, uint8 blue) {
	Palette &pal = getPalette(0);

	const int offset = index * 3;

	if (pal[offset + 0] == red && pal[offset + 1] == green && pal[offset + 2] == blue)
		return;

	pal[offset + 0] = red;
	pal[offset + 1] = green;
	pal[offset + 2] = blue;

	setScreenPalette(pal);
}

void Screen::getRealPalette(int num, uint8 *dst) {
	const int colors = _use16ColorMode ? 16 : (_isAmiga ? 32 : 256);
	const uint8 *palData = getPalette(num).getData();

	if (!palData) {
		memset(dst, 0, colors * 3);
		return;
	}

	for (int i = 0; i < colors; ++i) {
		dst[0] = (palData[0] * 0xFF) / 0x3F;
		dst[1] = (palData[1] * 0xFF) / 0x3F;
		dst[2] = (palData[2] * 0xFF) / 0x3F;
		dst += 3;
		palData += 3;
	}
}

void Screen::setScreenPalette(const Palette &pal) {
	uint8 screenPal[256 * 3];
	_screenPalette->copy(pal);

	for (int i = 0; i < pal.getNumColors(); ++i) {
		screenPal[3 * i + 0] = (pal[i * 3 + 0] * 0xFF) / 0x3F;
		screenPal[3 * i + 1] = (pal[i * 3 + 1] * 0xFF) / 0x3F;
		screenPal[3 * i + 2] = (pal[i * 3 + 2] * 0xFF) / 0x3F;
	}

	_paletteChanged = true;
	_system->getPaletteManager()->setPalette(screenPal, 0, pal.getNumColors());
}

void Screen::enableInterfacePalette(bool e) {
	_interfacePaletteEnabled = e;

	_forceFullUpdate = true;
	_dirtyRects.clear();

	// TODO: We might need to reset the mouse cursor

	updateScreen();
}

void Screen::setInterfacePalette(const Palette &pal, uint8 r, uint8 g, uint8 b) {
	if (!_isAmiga)
		return;

	uint8 screenPal[32 * 3];

	assert(32 <= pal.getNumColors());

	for (int i = 0; i < pal.getNumColors(); ++i) {
		if (i != 0x10) {
			screenPal[3 * i + 0] = (pal[i * 3 + 0] * 0xFF) / 0x3F;
			screenPal[3 * i + 1] = (pal[i * 3 + 1] * 0xFF) / 0x3F;
			screenPal[3 * i + 2] = (pal[i * 3 + 2] * 0xFF) / 0x3F;
		} else {
			screenPal[3 * i + 0] = (r * 0xFF) / 0x3F;
			screenPal[3 * i + 1] = (g * 0xFF) / 0x3F;
			screenPal[3 * i + 2] = (b * 0xFF) / 0x3F;
		}
	}

	_paletteChanged = true;
	_system->getPaletteManager()->setPalette(screenPal, 32, pal.getNumColors());
}

void Screen::copyToPage0(int y, int h, uint8 page, uint8 *seqBuf) {
	assert(y + h <= SCREEN_H);
	const uint8 *src = getPagePtr(page) + y * SCREEN_W;
	uint8 *dstPage = getPagePtr(0) + y * SCREEN_W;
	for (int i = 0; i < h; ++i) {
		for (int x = 0; x < SCREEN_W; ++x) {
			if (seqBuf[x] != src[x]) {
				seqBuf[x] = src[x];
				dstPage[x] = src[x];
			}
		}
		src += SCREEN_W;
		seqBuf += SCREEN_W;
		dstPage += SCREEN_W;
	}
	addDirtyRect(0, y, SCREEN_W, h);
	// This would remove the text in the end sequence of
	// the (Kyrandia 1) FM-TOWNS version.
	// Since this method is just used for the Seqplayer
	// this shouldn't be a problem anywhere else, so it's
	// safe to disable the call here.
	//clearOverlayRect(0, 0, y, SCREEN_W, h);
}

void Screen::copyRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage, int flags) {
	if (x2 < 0) {
		if (x2  <= -w)
			return;
		w += x2;
		x1 -= x2;
		x2 = 0;
	} else if (x2 + w >= SCREEN_W) {
		if (x2 > SCREEN_W)
			return;
		w = SCREEN_W - x2;
	}

	if (y2 < 0) {
		if (y2 <= -h)
			return;
		h += y2;
		y1 -= y2;
		y2 = 0;
	} else if (y2 + h >= SCREEN_H) {
		if (y2 > SCREEN_H)
			return;
		h = SCREEN_H - y2;
	}

	const uint8 *src = getPagePtr(srcPage) + y1 * SCREEN_W + x1;
	uint8 *dst = getPagePtr(dstPage) + y2 * SCREEN_W + x2;

	if (src == dst)
		return;

	if (dstPage == 0 || dstPage == 1)
		addDirtyRect(x2, y2, w, h);

	copyOverlayRegion(x1, y1, x2, y2, w, h, srcPage, dstPage);

	if (flags & CR_NO_P_CHECK) {
		while (h--) {
			memmove(dst, src, w);
			src += SCREEN_W;
			dst += SCREEN_W;
		}
	} else {
		while (h--) {
			for (int i = 0; i < w; ++i) {
				if (src[i])
					dst[i] = src[i];
			}
			src += SCREEN_W;
			dst += SCREEN_W;
		}
	}
}

void Screen::copyRegionToBuffer(int pageNum, int x, int y, int w, int h, uint8 *dest) {
	if (y < 0) {
		dest += (-y) * w;
		h += y;
		y = 0;
	} else if (y + h > SCREEN_H) {
		h = SCREEN_H - y;
	}

	if (x < 0) {
		dest += -x;
		w += x;
		x = 0;
	} else if (x + w > SCREEN_W) {
		w = SCREEN_W - x;
	}

	if (w < 0 || h < 0)
		return;

	uint8 *pagePtr = getPagePtr(pageNum);

	for (int i = y; i < y + h; ++i)
		memcpy(dest + (i - y) * w, pagePtr + i * SCREEN_W + x, w);
}

void Screen::copyPage(uint8 srcPage, uint8 dstPage) {
	uint8 *src = getPagePtr(srcPage);
	uint8 *dst = getPagePtr(dstPage);
	if (src != dst)
		memcpy(dst, src, SCREEN_W * SCREEN_H);
	copyOverlayRegion(0, 0, 0, 0, SCREEN_W, SCREEN_H, srcPage, dstPage);

	if (dstPage == 0 || dstPage == 1)
		_forceFullUpdate = true;
}

void Screen::copyBlockToPage(int pageNum, int x, int y, int w, int h, const uint8 *src) {
	if (y < 0) {
		src += (-y) * w;
		h += y;
		y = 0;
	} else if (y + h > SCREEN_H) {
		h = SCREEN_H - y;
	}

	if (x < 0) {
		src += -x;
		w += x;
		x = 0;
	} else if (x + w > SCREEN_W) {
		w = SCREEN_W - x;
	}

	if (w < 0 || h < 0)
		return;

	uint8 *dst = getPagePtr(pageNum) + y * SCREEN_W + x;

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, w, h);

	clearOverlayRect(pageNum, x, y, w, h);

	while (h--) {
		memcpy(dst, src, w);
		dst += SCREEN_W;
		src += w;
	}
}

void Screen::shuffleScreen(int sx, int sy, int w, int h, int srcPage, int dstPage, int ticks, bool transparent) {
	assert(sx >= 0 && w <= SCREEN_W);
	int x;
	uint16 x_offs[SCREEN_W];
	for (x = 0; x < SCREEN_W; ++x)
		x_offs[x] = x;

	for (x = 0; x < w; ++x) {
		int i = _vm->_rnd.getRandomNumber(w - 1);
		SWAP(x_offs[x], x_offs[i]);
	}

	assert(sy >= 0 && h <= SCREEN_H);
	int y;
	uint8 y_offs[SCREEN_H];
	for (y = 0; y < SCREEN_H; ++y)
		y_offs[y] = y;

	for (y = 0; y < h; ++y) {
		int i = _vm->_rnd.getRandomNumber(h - 1);
		SWAP(y_offs[y], y_offs[i]);
	}

	int32 start, now;
	int wait;
	for (y = 0; y < h && !_vm->shouldQuit(); ++y) {
		start = (int32)_system->getMillis();
		int y_cur = y;
		for (x = 0; x < w; ++x) {
			int i = sx + x_offs[x];
			int j = sy + y_offs[y_cur];
			++y_cur;
			if (y_cur >= h)
				y_cur = 0;

			uint8 color = getPagePixel(srcPage, i, j);
			if (!transparent || color != 0)
				setPagePixel(dstPage, i, j, color);
		}
		// forcing full update for now
		_forceFullUpdate = true;
		updateScreen();
		now = (int32)_system->getMillis();
		wait = ticks * _vm->tickLength() - (now - start);
		if (wait > 0)
			_vm->delay(wait);
	}

	copyOverlayRegion(sx, sy, sx, sy, w, h, srcPage, dstPage);

	if (_vm->shouldQuit()) {
		copyRegion(sx, sy, sx, sy, w, h, srcPage, dstPage);
		_system->updateScreen();
	}
}

void Screen::fillRect(int x1, int y1, int x2, int y2, uint8 color, int pageNum, bool xored) {
	assert(x2 < SCREEN_W && y2 < SCREEN_H);
	if (pageNum == -1)
		pageNum = _curPage;

	uint8 *dst = getPagePtr(pageNum) + y1 * SCREEN_W + x1;

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x1, y1, x2-x1+1, y2-y1+1);

	clearOverlayRect(pageNum, x1, y1, x2-x1+1, y2-y1+1);

	if (_use16ColorMode) {
		color &= 0x0F;
		color |= (color << 4);
	} else if (_renderMode == Common::kRenderCGA) {
		color &= 0x03;
	} else if (_renderMode == Common::kRenderEGA && !_useHiResEGADithering) {
		color &= 0x0F;
	}

	if (xored) {
		for (; y1 <= y2; ++y1) {
			for (int x = x1; x <= x2; ++x)
				dst[x] ^= color;
			dst += SCREEN_W;
		}
	} else {
		for (; y1 <= y2; ++y1) {
			memset(dst, color, x2 - x1 + 1);
			dst += SCREEN_W;
		}
	}
}

void Screen::drawBox(int x1, int y1, int x2, int y2, int color) {
	drawClippedLine(x1, y1, x2, y1, color);
	drawClippedLine(x1, y1, x1, y2, color);
	drawClippedLine(x2, y1, x2, y2, color);
	drawClippedLine(x1, y2, x2, y2, color);
}

void Screen::drawShadedBox(int x1, int y1, int x2, int y2, int color1, int color2) {
	assert(x1 >= 0 && y1 >= 0);
	fillRect(x1, y1, x2, y1 + 1, color1);
	fillRect(x2 - 1, y1, x2, y2, color1);

	drawClippedLine(x1, y1, x1, y2, color2);
	drawClippedLine(x1 + 1, y1 + 1, x1 + 1, y2 - 1, color2);
	drawClippedLine(x1, y2 - 1, x2 - 1, y2 - 1, color2);
	drawClippedLine(x1, y2, x2, y2, color2);
}

void Screen::drawClippedLine(int x1, int y1, int x2, int y2, int color) {
	if (x1 < 0)
		x1 = 0;
	else if (x1 > 319)
		x1 = 319;

	if (x2 < 0)
		x2 = 0;
	else if (x2 > 319)
		x2 = 319;

	if (y1 < 0)
		y1 = 0;
	else if (y1 > 199)
		y1 = 199;

	if (y2 < 0)
		y2 = 0;
	else if (y2 > 199)
		y2 = 199;

	if (x1 == x2)
		if (y1 > y2)
			drawLine(true, x1, y2, y1 - y2 + 1, color);
		else
			drawLine(true, x1, y1, y2 - y1 + 1, color);
	else
		if (x1 > x2)
			drawLine(false, x2, y1, x1 - x2 + 1, color);
		else
			drawLine(false, x1, y1, x2 - x1 + 1, color);
}

void Screen::drawLine(bool vertical, int x, int y, int length, int color) {
	uint8 *ptr = getPagePtr(_curPage) + y * SCREEN_W + x;

	if (_use16ColorMode) {
		color &= 0x0F;
		color |= (color << 4);
	} else if (_renderMode == Common::kRenderCGA) {
		color &= 0x03;
	} else if (_renderMode == Common::kRenderEGA && !_useHiResEGADithering) {
		color &= 0x0F;
	}

	if (vertical) {
		assert((y + length) <= SCREEN_H);
		int currLine = 0;
		while (currLine < length) {
			*ptr = color;
			ptr += SCREEN_W;
			currLine++;
		}
	} else {
		assert((x + length) <= SCREEN_W);
		memset(ptr, color, length);
	}

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x, y, (vertical) ? 1 : length, (vertical) ? length : 1);

	clearOverlayRect(_curPage, x, y, (vertical) ? 1 : length, (vertical) ? length : 1);
}

void Screen::setAnimBlockPtr(int size) {
	delete[] _animBlockPtr;
	_animBlockPtr = new uint8[size];
	assert(_animBlockPtr);
	memset(_animBlockPtr, 0, size);
	_animBlockSize = size;
}

void Screen::setTextColor(const uint8 *cmap, int a, int b) {
	memcpy(&_textColorsMap[a], cmap, b-a+1);

	// We need to update the color tables of all fonts, we
	// setup so far here.
	for (int i = 0; i < FID_NUM; ++i) {
		if (_fonts[i])
			_fonts[i]->setColorMap(_textColorsMap);
	}
}

bool Screen::loadFont(FontId fontId, const char *filename) {
	if (fontId == FID_SJIS_FNT) {
		warning("Trying to replace system SJIS font");
		return true;
	}

	Font *&fnt = _fonts[fontId];

	if (!fnt) {
		if (_isAmiga)
			fnt = new AMIGAFont();
#ifdef ENABLE_EOB
		else if (_vm->game() == GI_EOB1 || _vm->game() == GI_EOB2)
			// We use normal VGA rendering in EOB II, since we do the complete EGA dithering in updateScreen().
			fnt = new OldDOSFont(_useHiResEGADithering ? Common::kRenderVGA : _renderMode);
#endif // ENABLE_EOB
		else
			fnt = new DOSFont();

		assert(fnt);
	}

	Common::SeekableReadStream *file = _vm->resource()->createReadStream(filename);
	if (!file)
		error("Font file '%s' is missing", filename);

	bool ret = fnt->load(*file);
	fnt->setColorMap(_textColorsMap);
	delete file;
	return ret;
}

Screen::FontId Screen::setFont(FontId fontId) {
	FontId prev = _currentFont;
	_currentFont = fontId;

	assert(_fonts[_currentFont]);
	return prev;
}

int Screen::getFontHeight() const {
	return _fonts[_currentFont]->getHeight();
}

int Screen::getFontWidth() const {
	return _fonts[_currentFont]->getWidth();
}

int Screen::getCharWidth(uint16 c) const {
	const int width = _fonts[_currentFont]->getCharWidth(c);
	return width + ((_currentFont != FID_SJIS_FNT) ? _charWidth : 0);
}

int Screen::getTextWidth(const char *str) const {
	int curLineLen = 0;
	int maxLineLen = 0;

	while (1) {
		uint c = fetchChar(str);

		if (c == 0) {
			break;
		} else if (c == '\r') {
			if (curLineLen > maxLineLen)
				maxLineLen = curLineLen;
			else
				curLineLen = 0;
		} else {
			curLineLen += getCharWidth(c);
		}
	}

	return MAX(curLineLen, maxLineLen);
}

void Screen::printText(const char *str, int x, int y, uint8 color1, uint8 color2) {
	uint8 cmap[2];
	cmap[0] = color2;
	cmap[1] = color1;
	setTextColor(cmap, 0, 1);

	const uint8 charHeightFnt = getFontHeight();

	if (x < 0)
		x = 0;
	else if (x >= SCREEN_W)
		return;

	int x_start = x;
	if (y < 0)
		y = 0;
	else if (y >= SCREEN_H)
		return;

	while (1) {
		uint c = fetchChar(str);

		if (c == 0) {
			break;
		} else if (c == '\r') {
			x = x_start;
			y += (charHeightFnt + _charOffset);
		} else {
			int charWidth = getCharWidth(c);
			if (x + charWidth > SCREEN_W) {
				x = x_start;
				y += (charHeightFnt + _charOffset);
				if (y >= SCREEN_H)
					break;
			}

			drawChar(c, x, y);
			x += charWidth;
		}
	}
}

uint16 Screen::fetchChar(const char *&s) const {
	if (_currentFont != FID_SJIS_FNT)
		return (uint8)*s++;

	uint16 ch = (uint8)*s++;

	if (ch <= 0x7F || (ch >= 0xA1 && ch <= 0xDF))
		return ch;

	ch |= (uint8)(*s++) << 8;
	return ch;
}

void Screen::drawChar(uint16 c, int x, int y) {
	Font *fnt = _fonts[_currentFont];
	assert(fnt);

	const bool useOverlay = fnt->usesOverlay();
	const int charWidth = fnt->getCharWidth(c);
	const int charHeight = fnt->getHeight();

	if (x < 0 || y < 0)
		return;
	if (x + charWidth > SCREEN_W || y + charHeight > SCREEN_H)
		return;

	if (useOverlay) {
		uint8 *destPage = getOverlayPtr(_curPage);
		if (!destPage) {
			warning("trying to draw SJIS char on unsupported page %d", _curPage);
			return;
		}

		destPage += (y * 2) * 640 + (x * 2);

		fnt->drawChar(c, destPage, 640);
	} else {
		fnt->drawChar(c, getPagePtr(_curPage) + y * SCREEN_W + x, SCREEN_W);
	}

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x, y, charWidth, charHeight);
}

void Screen::drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, ...) {
	if (!shapeData)
		return;

	if (_vm->gameFlags().useAltShapeHeader)
		shapeData += 2;

	if (*shapeData & 1)
		flags |= 0x400;

	va_list args;
	va_start(args, flags);

	static const int drawShapeVar2[] = {
		1, 3, 2, 5, 4, 3, 2, 1
	};

	_dsTable = 0;
	_dsTableLoopCount = 0;
	_dsTable2 = 0;
	_dsTable3 = 0;
	_dsTable4 = 0;
	_dsTable5 = 0;
	_dsDrawLayer = 0;

	if (flags & 0x8000) {
		_dsTable2 = va_arg(args, uint8 *);
	}

	if (flags & 0x100) {
		_dsTable = va_arg(args, uint8 *);
		_dsTableLoopCount = va_arg(args, int);
		if (!_dsTableLoopCount)
			flags &= ~0x100;
	}

	if (flags & 0x1000) {
		_dsTable3 = va_arg(args, uint8 *);
		_dsTable4 = va_arg(args, uint8 *);
	}

	if (flags & 0x200) {
		_drawShapeVar1 = (_drawShapeVar1 + 1) & 0x7;
		_drawShapeVar3 = drawShapeVar2[_drawShapeVar1];
		_drawShapeVar4 = 0;
		_drawShapeVar5 = 256;
	}

	if (flags & 0x4000)
		_drawShapeVar5 = va_arg(args, int);

	if (flags & 0x800)
		_dsDrawLayer = va_arg(args, int);

	if (flags & DSF_SCALE) {
		_dsScaleW = va_arg(args, int);
		_dsScaleH = va_arg(args, int);
	} else {
		_dsScaleW = 0x100;
		_dsScaleH = 0x100;
	}

	if ((flags & 0x2000) && _vm->game() != GI_KYRA1)
		_dsTable5 = va_arg(args, uint8 *);

	va_end(args);

	static const DsMarginSkipFunc dsMarginFunc[] = {
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeMarginScaleUpwind,
		&Screen::drawShapeMarginScaleDownwind,
		&Screen::drawShapeMarginScaleUpwind,
		&Screen::drawShapeMarginScaleDownwind
	};

	static const DsMarginSkipFunc dsSkipFunc[] = {
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeSkipScaleUpwind,
		&Screen::drawShapeSkipScaleDownwind,
		&Screen::drawShapeSkipScaleUpwind,
		&Screen::drawShapeSkipScaleDownwind
	};

	static const DsLineFunc dsLineFunc[] = {
		&Screen::drawShapeProcessLineNoScaleUpwind,
		&Screen::drawShapeProcessLineNoScaleDownwind,
		&Screen::drawShapeProcessLineNoScaleUpwind,
		&Screen::drawShapeProcessLineNoScaleDownwind,
		&Screen::drawShapeProcessLineScaleUpwind,
		&Screen::drawShapeProcessLineScaleDownwind,
		&Screen::drawShapeProcessLineScaleUpwind,
		&Screen::drawShapeProcessLineScaleDownwind
	};

	static const DsPlotFunc dsPlotFunc[] = {
		&Screen::drawShapePlotType0,		// used by Kyra 1 + 2
		&Screen::drawShapePlotType1,		// used by Kyra 3
		0,
		&Screen::drawShapePlotType3_7,		// used by Kyra 3 (shadow)
		&Screen::drawShapePlotType4,		// used by Kyra 1, 2 + 3
		&Screen::drawShapePlotType5,		// used by Kyra 1
		&Screen::drawShapePlotType6,		// used by Kyra 1 (invisibility)
		&Screen::drawShapePlotType3_7,		// used by Kyra 1 (invisibility)
		&Screen::drawShapePlotType8,		// used by Kyra 2
		&Screen::drawShapePlotType9,		// used by Kyra 1 + 3
		0,
		&Screen::drawShapePlotType11_15,	// used by Kyra 1 (invisibility) + Kyra 3 (shadow)
		&Screen::drawShapePlotType12,		// used by Kyra 2
		&Screen::drawShapePlotType13,		// used by Kyra 1
		&Screen::drawShapePlotType14,		// used by Kyra 1 (invisibility)
		&Screen::drawShapePlotType11_15,	// used by Kyra 1 (invisibility)
		&Screen::drawShapePlotType16,		// used by LoL PC-98/16 Colors (teleporters),
		0, 0, 0,
		&Screen::drawShapePlotType20,		// used by LoL (heal spell effect)
		&Screen::drawShapePlotType21,		// used by LoL (white tower spirits)
		0, 0, 0, 0,	0, 0, 0, 0, 0, 0,
		0,
		&Screen::drawShapePlotType33,		// used by LoL (blood spots on the floor)
		0, 0, 0,
		&Screen::drawShapePlotType37,		// used by LoL (monsters)
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		&Screen::drawShapePlotType48,		// used by LoL (slime spots on the floor)
		0, 0, 0,
		&Screen::drawShapePlotType52,		// used by LoL (projectiles)
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0
	};

	int scaleCounterV = 0;

	const int drawFunc = flags & 0x0f;
	_dsProcessMargin = dsMarginFunc[drawFunc];
	_dsScaleSkip = dsSkipFunc[drawFunc];
	_dsProcessLine = dsLineFunc[drawFunc];

	const int ppc = (flags >> 8) & 0x3F;
	_dsPlot = dsPlotFunc[ppc];
	DsPlotFunc dsPlot2 = dsPlotFunc[ppc], dsPlot3 = dsPlotFunc[ppc];
	if (flags & 0x800)
		dsPlot3 = dsPlotFunc[((flags >> 8) & 0xF7) & 0x3F];

	if (!_dsPlot || !dsPlot2 || !dsPlot3) {
		if (!dsPlot2)
			warning("Missing drawShape plotting method type %d", ppc);
		if (dsPlot3 != dsPlot2 && !dsPlot3)
			warning("Missing drawShape plotting method type %d", (((flags >> 8) & 0xF7) & 0x3F));
		return;
	}

	int curY = y;
	const uint8 *src = shapeData;
	uint8 *dst = _dsDstPage = getPagePtr(pageNum);

	const ScreenDim *dsDim = getScreenDim(sd);
	dst += (dsDim->sx << 3);

	if (!(flags & 0x10))
		x -= (dsDim->sx << 3);

	int x2 = (dsDim->w << 3);
	int y1 = dsDim->sy;
	if (flags & 0x10)
		y += y1;

	int y2 = y1 + dsDim->h;

	uint16 shapeFlags = READ_LE_UINT16(src); src += 2;

	int shapeHeight = *src++;
	uint16 shapeWidth = READ_LE_UINT16(src); src += 2;

	int shpWidthScaled1 = shapeWidth;
	int shpWidthScaled2 = shapeWidth;

	if (flags & DSF_SCALE) {
		shapeHeight = (shapeHeight * _dsScaleH) >> 8;
		shpWidthScaled1 = shpWidthScaled2 = (shapeWidth * _dsScaleW) >> 8;

		if (!shapeHeight || !shpWidthScaled1)
			return;
	}

	if (flags & DSF_CENTER) {
		x -= (shpWidthScaled1 >> 1);
		y -= (shapeHeight >> 1);
	}

	src += 3;

	uint16 frameSize = READ_LE_UINT16(src); src += 2;

	int colorTableColors = ((_vm->game() != GI_KYRA1) && (shapeFlags & 4)) ? *src++ : 16;

	if (!(flags & 0x8000) && (shapeFlags & 1))
		_dsTable2 = src;

	if (flags & 0x400)
		src += colorTableColors;

	if (!(shapeFlags & 2)) {
		decodeFrame4(src, _animBlockPtr, frameSize);
		src = _animBlockPtr;
	}

	int t = (flags & 2) ? y2 - y - shapeHeight : y - y1;

	if (t < 0) {
		shapeHeight += t;
		if (shapeHeight <= 0) {
			return;
		}

		t *= -1;
		const uint8 *srcBackUp = 0;

		do {
			_dsOffscreenScaleVal1 = 0;
			srcBackUp = src;
			_dsTmpWidth = shapeWidth;

			int cnt = shapeWidth;
			(this->*_dsScaleSkip)(dst, src, cnt);

			scaleCounterV += _dsScaleH;

			if (scaleCounterV & 0xFF00) {
				uint8 r = scaleCounterV >> 8;
				scaleCounterV &= 0xFF;
				t -= r;
			}
		} while (!(scaleCounterV & 0xFF00) && (t > 0));

		if (t < 0) {
			src = srcBackUp;
			scaleCounterV += (-t << 8);
		}

		if (!(flags & 2))
			y = y1;
	}

	t = (flags & 2) ? y + shapeHeight - y1 : y2 - y;
	if (t <= 0)
		return;

	if (t < shapeHeight) {
		shapeHeight = t;
		if (flags & 2)
			y = y1;
	}

	_dsOffscreenLeft = 0;
	if (x < 0) {
		shpWidthScaled1 += x;
		_dsOffscreenLeft = -x;
		if (_dsOffscreenLeft >= shpWidthScaled2)
			return;
		x = 0;
	}

	_dsOffscreenRight = 0;
	t = x2 - x;

	if (t <= 0)
		return;

	if (t < shpWidthScaled1) {
		shpWidthScaled1 = t;
		_dsOffscreenRight = shpWidthScaled2 - _dsOffscreenLeft - shpWidthScaled1;
	}

	int dsPitch = 320;
	int ty = y;

	if (flags & 2) {
		dsPitch *= -1;
		ty = ty - 1 + shapeHeight;
	}

	if (flags & DSF_X_FLIPPED) {
		SWAP(_dsOffscreenLeft, _dsOffscreenRight);
		dst += (shpWidthScaled1 - 1);
	}

	dst += (320 * ty + x);

	if (flags & DSF_SCALE) {
		_dsOffscreenRight = 0;
		_dsOffscreenScaleVal2 = _dsOffscreenLeft;
		_dsOffscreenLeft <<= 8;
		_dsOffscreenScaleVal1 = (_dsOffscreenLeft % _dsScaleW) * -1;
		_dsOffscreenLeft /= _dsScaleW;
	}

	if (shapeHeight <= 0 || shpWidthScaled1 <= 0)
		return;

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, shpWidthScaled1, shapeHeight);
	clearOverlayRect(pageNum, x, y, shpWidthScaled1, shapeHeight);

	uint8 *d = dst;

	bool normalPlot = true;
	while (true) {
		while (!(scaleCounterV & 0xFF00)) {
			scaleCounterV += _dsScaleH;
			if (!(scaleCounterV & 0xFF00)) {
				_dsTmpWidth = shapeWidth;
				int cnt = shapeWidth;
				(this->*_dsScaleSkip)(d, src, cnt);
			}
		}

		const uint8 *b_src = src;

		do {
			src = b_src;
			_dsTmpWidth = shapeWidth;
			int cnt = _dsOffscreenLeft;
			int scaleState = (this->*_dsProcessMargin)(d, src, cnt);

			if (_dsTmpWidth) {
				cnt += shpWidthScaled1;
				if (cnt > 0) {
					if (flags & 0x800)
						normalPlot = (curY > _maskMinY && curY < _maskMaxY);
					_dsPlot = normalPlot ? dsPlot2 : dsPlot3;
					(this->*_dsProcessLine)(d, src, cnt, scaleState);
				}
				cnt += _dsOffscreenRight;
				if (cnt)
					(this->*_dsScaleSkip)(d, src, cnt);
			}
			dst += dsPitch;
			d = dst;
			++curY;

			if (!--shapeHeight)
				return;

			scaleCounterV -= 0x100;
		} while (scaleCounterV & 0xFF00);
	}
}

int Screen::drawShapeMarginNoScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	while (cnt-- > 0) {
		if (*src++)
			continue;
		cnt = cnt + 1 - (*src++);
	}

	cnt++;
	dst -= cnt;
	return 0;
}

int Screen::drawShapeMarginNoScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	while (cnt-- > 0) {
		if (*src++)
			continue;
		cnt = cnt + 1 - (*src++);
	}

	cnt++;
	dst += cnt;
	return 0;
}

int Screen::drawShapeMarginScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	_dsTmpWidth -= cnt;

	while (cnt > 0) {
		--cnt;
		if (*src++)
			continue;

		cnt = cnt + 1 - (*src++);
	}

	if (!cnt)
		return _dsOffscreenScaleVal1;

	_dsTmpWidth += cnt;

	int i = (_dsOffscreenLeft - cnt) * _dsScaleW;
	int res = i & 0xff;
	i >>= 8;
	i -= _dsOffscreenScaleVal2;
	dst += i;
	cnt = -i;

	return res;
}

int Screen::drawShapeMarginScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	_dsTmpWidth -= cnt;

	while (cnt > 0) {
		--cnt;
		if (*src++)
			continue;

		cnt = cnt + 1 - (*src++);
	}

	if (!cnt)
		return _dsOffscreenScaleVal1;

	_dsTmpWidth += cnt;

	int i = (_dsOffscreenLeft - cnt) * _dsScaleW;
	int res = i & 0xff;
	i >>= 8;
	i -= _dsOffscreenScaleVal2;
	dst -= i;
	cnt = -i;

	return res;
}

int Screen::drawShapeSkipScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	cnt = _dsTmpWidth;

	if (cnt <= 0)
		return 0;

	do {
		--cnt;
		if (*src++)
			continue;
		cnt = cnt + 1 - (*src++);
	} while (cnt > 0);

	return 0;
}

int Screen::drawShapeSkipScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	cnt = _dsTmpWidth;
	bool found = false;

	if (cnt == 0)
		return 0;

	do {
		--cnt;
		if (*src++)
			continue;
		found = true;
		cnt = cnt + 1 - (*src++);
	} while (cnt > 0);

	return found ? 0 : _dsOffscreenScaleVal1;
}

void Screen::drawShapeProcessLineNoScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt, int16) {
	do {
		uint8 c = *src++;
		if (c) {
			uint8 *d = dst++;
			(this->*_dsPlot)(d, c);
			cnt--;
		} else {
			c = *src++;
			dst += c;
			cnt -= c;
		}
	} while (cnt > 0);
}

void Screen::drawShapeProcessLineNoScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt, int16) {
	do {
		uint8 c = *src++;
		if (c) {
			uint8 *d = dst--;
			(this->*_dsPlot)(d, c);
			cnt--;
		} else {
			c = *src++;
			dst -= c;
			cnt -= c;
		}
	} while (cnt > 0);
}

void Screen::drawShapeProcessLineScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt, int16 scaleState) {
	int c = 0;

	do {
		if ((scaleState & 0x8000) || !(scaleState & 0xFF00)) {
			c = *src++;
			_dsTmpWidth--;
			if (c) {
				scaleState += _dsScaleW;
			} else {
				_dsTmpWidth++;
				c = *src++;
				_dsTmpWidth -= c;
				int r = c * _dsScaleW + scaleState;
				dst += (r >> 8);
				cnt -= (r >> 8);
				scaleState = r & 0xff;
			}
		} else if (scaleState) {
			(this->*_dsPlot)(dst++, c);
			scaleState -= 0x100;
			cnt--;
		}
	} while (cnt > 0);

	cnt = -1;
}

void Screen::drawShapeProcessLineScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt, int16 scaleState) {
	int c = 0;

	do {
		if ((scaleState & 0x8000) || !(scaleState & 0xFF00)) {
			c = *src++;
			_dsTmpWidth--;
			if (c) {
				scaleState += _dsScaleW;
			} else {
				_dsTmpWidth++;
				c = *src++;
				_dsTmpWidth -= c;
				int r = c * _dsScaleW + scaleState;
				dst -= (r >> 8);
				cnt -= (r >> 8);
				scaleState = r & 0xff;
			}
		} else {
			(this->*_dsPlot)(dst--, c);
			scaleState -= 0x100;
			cnt--;
		}
	} while (cnt > 0);

	cnt = -1;
}

void Screen::drawShapePlotType0(uint8 *dst, uint8 cmd) {
	*dst = cmd;
}

void Screen::drawShapePlotType1(uint8 *dst, uint8 cmd) {
	for (int i = 0; i < _dsTableLoopCount; ++i)
		cmd = _dsTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType3_7(uint8 *dst, uint8 cmd) {
	cmd = *dst;
	for (int i = 0; i < _dsTableLoopCount; ++i)
		cmd = _dsTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType4(uint8 *dst, uint8 cmd) {
	*dst = _dsTable2[cmd];
}

void Screen::drawShapePlotType5(uint8 *dst, uint8 cmd) {
	cmd = _dsTable2[cmd];
	for (int i = 0; i < _dsTableLoopCount; ++i)
		cmd = _dsTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType6(uint8 *dst, uint8 cmd) {
	int t = _drawShapeVar4 + _drawShapeVar5;
	if (t & 0xff00) {
		cmd = dst[_drawShapeVar3];
		t &= 0xff;
	} else {
		cmd = _dsTable2[cmd];
	}

	_drawShapeVar4 = t;
	*dst = cmd;
}

void Screen::drawShapePlotType8(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;
	if (_dsDrawLayer < t)
		cmd = _shapePages[1][relOffs];

	*dst = cmd;
}

void Screen::drawShapePlotType9(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		for (int i = 0; i < _dsTableLoopCount; ++i)
			cmd = _dsTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType11_15(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;

	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		cmd = *dst;
		for (int i = 0; i < _dsTableLoopCount; ++i)
			cmd = _dsTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType12(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		cmd = _dsTable2[cmd];
	}

	*dst = cmd;
}

void Screen::drawShapePlotType13(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		cmd = _dsTable2[cmd];
		for (int i = 0; i < _dsTableLoopCount; ++i)
			cmd = _dsTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType14(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		t = _drawShapeVar4 + _drawShapeVar5;
		if (t & 0xff00) {
			cmd = dst[_drawShapeVar3];
			t &= 0xff;
		} else {
			cmd = _dsTable2[cmd];
		}
	}

	_drawShapeVar4 = t;
	*dst = cmd;
}

void Screen::drawShapePlotType16(uint8 *dst, uint8 cmd) {
	uint8 tOffs = _dsTable3[cmd];
	if (!(tOffs & 0x80))
		cmd = _dsTable4[tOffs << 8 | *dst];
	*dst = cmd;
}

void Screen::drawShapePlotType20(uint8 *dst, uint8 cmd) {
	cmd = _dsTable2[cmd];
	uint8 tOffs = _dsTable3[cmd];
	if (!(tOffs & 0x80))
		cmd = _dsTable4[tOffs << 8 | *dst];

	*dst = cmd;
}

void Screen::drawShapePlotType21(uint8 *dst, uint8 cmd) {
	cmd = _dsTable2[cmd];
	uint8 tOffs = _dsTable3[cmd];
	if (!(tOffs & 0x80))
		cmd = _dsTable4[tOffs << 8 | *dst];

	for (int i = 0; i < _dsTableLoopCount; ++i)
		cmd = _dsTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType33(uint8 *dst, uint8 cmd) {
	if (cmd == 255) {
		*dst = _dsTable5[*dst];
	} else {
		for (int i = 0; i < _dsTableLoopCount; ++i)
			cmd = _dsTable[cmd];
		if (cmd)
			*dst = cmd;
	}
}

void Screen::drawShapePlotType37(uint8 *dst, uint8 cmd) {
	cmd = _dsTable2[cmd];

	if (cmd == 255) {
		cmd = _dsTable5[*dst];
	} else {
		for (int i = 0; i < _dsTableLoopCount; ++i)
			cmd = _dsTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType48(uint8 *dst, uint8 cmd) {
	uint8 offs = _dsTable3[cmd];
	if (!(offs & 0x80))
		cmd = _dsTable4[(offs << 8) | *dst];
	*dst = cmd;
}

void Screen::drawShapePlotType52(uint8 *dst, uint8 cmd) {
	cmd = _dsTable2[cmd];
	uint8 offs = _dsTable3[cmd];

	if (!(offs & 0x80))
		cmd = _dsTable4[(offs << 8) | *dst];

	*dst = cmd;
}

void Screen::decodeFrame1(const uint8 *src, uint8 *dst, uint32 size) {
	const uint8 *dstEnd = dst + size;

	struct Pattern {
		const uint8 *pos;
		uint16 len;
	};

	Pattern *patterns = new Pattern[3840];
	uint16 numPatterns = 0;
	uint8 nib = 0;

	uint16 code = decodeEGAGetCode(src, nib);
	uint8 last = code & 0xff;

	uint8 *dstPrev = dst;
	uint16 count = 1;
	uint16 countPrev = 1;

	*dst++ = last;

	while (dst < dstEnd) {
		code = decodeEGAGetCode(src, nib);
		uint8 cmd = code >> 8;

		if (cmd--) {
			code = (cmd << 8) | (code & 0xff);
			uint8 *tmpDst = dst;

			if (code < numPatterns) {
				const uint8 *tmpSrc = patterns[code].pos;
				countPrev = patterns[code].len;
				last = *tmpSrc;
				for (int i = 0; i < countPrev; i++)
					*dst++ = *tmpSrc++;

			} else {
				const uint8 *tmpSrc = dstPrev;
				count = countPrev;
				for (int i = 0; i < countPrev; i++)
					*dst++ = *tmpSrc++;
				*dst++ = last;
				countPrev++;
			}

			if (numPatterns < 3840) {
				patterns[numPatterns].pos = dstPrev;
				patterns[numPatterns++].len = ++count;
			}

			dstPrev = tmpDst;
			count = countPrev;

		} else {
			*dst++ = last = (code & 0xff);

			if (numPatterns < 3840) {
				patterns[numPatterns].pos = dstPrev;
				patterns[numPatterns++].len = ++count;
			}

			dstPrev = dst - 1;
			count = 1;
			countPrev = 1;
		}
	}
	delete[] patterns;
}

uint16 Screen::decodeEGAGetCode(const uint8 *&pos, uint8 &nib) {
	uint16 res = READ_BE_UINT16(pos++);
	if ((++nib) & 1) {
		res >>= 4;
	} else {
		pos++;
		res &= 0xfff;
	}
	return res;
}

void Screen::decodeFrame3(const uint8 *src, uint8 *dst, uint32 size) {
	const uint8 *dstEnd = dst + size;
	while (dst < dstEnd) {
		int8 code = *src++;
		if (code == 0) {
			uint16 sz = READ_BE_UINT16(src);
			src += 2;
			memset(dst, *src++, sz);
			dst += sz;
		} else if (code < 0) {
			memset(dst, *src++, -code);
			dst -= code;
		} else {
			memcpy(dst, src, code);
			dst += code;
			src += code;
		}
	}
}

uint Screen::decodeFrame4(const uint8 *src, uint8 *dst, uint32 dstSize) {
	uint8 *dstOrig = dst;
	uint8 *dstEnd = dst + dstSize;
	while (1) {
		int count = dstEnd - dst;
		if (count == 0)
			break;

		uint8 code = *src++;
		if (!(code & 0x80)) { // 8th bit isn't set
			int len = MIN(count, (code >> 4) + 3); //upper half of code is the length
			int offs = ((code & 0xF) << 8) | *src++; //lower half of code as byte 2 of offset.
			const uint8 *dstOffs = dst - offs;
			while (len--)
				*dst++ = *dstOffs++;
		} else if (code & 0x40) { // 7th bit is set
			int len = (code & 0x3F) + 3;
			if (code == 0xFE) {
				len = READ_LE_UINT16(src); src += 2;
				if (len > count)
					len = count;

				memset(dst, *src++, len); dst += len;
			} else {
				if (code == 0xFF) {
					len = READ_LE_UINT16(src);
					src += 2;
				}

				int offs = READ_LE_UINT16(src); src += 2;
				if (len > count)
					len = count;

				const uint8 *dstOffs = dstOrig + offs;
				while (len--)
					*dst++ = *dstOffs++;
			}
		} else if (code != 0x80) { // not just the 8th bit set.
			//Copy some bytes from source to dest.
			int len = MIN(count, code & 0x3F);
			while (len--)
				*dst++ = *src++;
		} else {
			break;
		}
	}
	return dst - dstOrig;
}

void Screen::decodeFrameDelta(uint8 *dst, const uint8 *src, bool noXor) {
	if (noXor)
		wrapped_decodeFrameDelta<true>(dst, src);
	else
		wrapped_decodeFrameDelta<false>(dst, src);
}

template<bool noXor>
void Screen::wrapped_decodeFrameDelta(uint8 *dst, const uint8 *src) {
	while (1) {
		uint8 code = *src++;
		if (code == 0) {
			uint8 len = *src++;
			code = *src++;
			while (len--) {
				if (noXor)
					*dst++ = code;
				else
					*dst++ ^= code;
			}
		} else if (code & 0x80) {
			code -= 0x80;
			if (code != 0) {
				dst += code;
			} else {
				uint16 subcode = READ_LE_UINT16(src); src += 2;
				if (subcode == 0) {
					break;
				} else if (subcode & 0x8000) {
					subcode -= 0x8000;
					if (subcode & 0x4000) {
						uint16 len = subcode - 0x4000;
						code = *src++;
						while (len--) {
							if (noXor)
								*dst++ = code;
							else
								*dst++ ^= code;
						}
					} else {
						while (subcode--) {
							if (noXor)
								*dst++ = *src++;
							else
								*dst++ ^= *src++;
						}
					}
				} else {
					dst += subcode;
				}
			}
		} else {
			while (code--) {
				if (noXor)
					*dst++ = *src++;
				else
					*dst++ ^= *src++;
			}
		}
	}
}

void Screen::decodeFrameDeltaPage(uint8 *dst, const uint8 *src, int pitch, bool noXor) {
	if (noXor)
		wrapped_decodeFrameDeltaPage<true>(dst, src, pitch);
	else
		wrapped_decodeFrameDeltaPage<false>(dst, src, pitch);
}

void Screen::convertAmigaGfx(uint8 *data, int w, int h, int depth, bool wsa, int bytesPerPlane) {
	const int planeWidth = (bytesPerPlane == -1) ? (w + 7) / 8 : bytesPerPlane;
	const int planeSize = planeWidth * h;
	const uint imageSize = planeSize * depth;

	// Our static buffer which holds the plane data. We need this
	// because the "data" pointer is both source and destination pointer.
	// The buffer has enough space to fit the AMIGA MSC files, which are
	// the biggest graphics files found in the AMIGA version.
	static uint8 temp[40320];
	assert(imageSize <= sizeof(temp));

	// WSA files store their graphics data in a little different format, than
	// the usual AMIGA graphics format used in BitMaps. Thus we need to do
	// some special handling for them here. Means we convert them into
	// the usual format.
	//
	// TODO: We might think of moving this conversion into the WSAMovieAmiga
	// class.
	if (wsa) {
		const byte *src = data;
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < planeWidth; ++x)
				for (int i = 0; i < depth; ++i)
					temp[y * planeWidth + x + planeSize * i] = *src++;
		}
	} else {
		memcpy(temp, data, imageSize);
	}

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			const int bytePos = x / 8 + y * planeWidth;
			const int bitPos = 7 - (x & 7); // x & 7 == x % 8

			byte col = 0;

			for (int i = 0; i < depth; ++i)
				col |= ((temp[bytePos + planeSize * i] >> bitPos) & 1) << i;

			*data++ = col;
		}
	}
}

void Screen::convertAmigaMsc(uint8 *data) {
	// MSC files are always 320x144, thus we can safely assume
	// this to be correct. Also they contain 7 planes instead
	// of the normal 5 planes, which is used in 32 color mode.
	// The need for 7 planes can be explained, because the MSC
	// files have 6 bits for the layer number (bits 1 to 6)
	// and one bit for the "blocked" flag (bit 0), and every
	// plane contains one bit per pixel.
	convertAmigaGfx(data, 320, 144, 7);

	// We need to do some post conversion, since
	// the AMIGA MSC format is different from the DOS
	// one we use internally for our code.That is even
	// after converting it from the AMIGA plane based
	// approach to one byte per pixel approach.
	for (int i = 0; i < 320 * 144; ++i) {
		// The lowest bit indicates, whether the position
		// is walkable or not. If the bit is set, the
		// position is walkable, elsewise it is blocked.
		if (data[i] & 1)
			data[i] &= 0xFE;
		else
			data[i] |= 0x80;

		// The graphics layer for the pixel is saved
		// in the following format:
		// The highest bit set indicates the number of
		// the graphics layer. We count the first
		// bit as 0 here, thus we need to add one,
		// to get the correct number.
		//
		// Funnily since the first bit (bit 0) is
		// resevered for testing whether the position
		// is walkable or not, there is no possibility
		// for layer 1 to be present.
		int layer = 0;
		for (int k = 0; k < 7; ++k)
			if (data[i] & (1 << k))
				layer = k + 1;

		data[i] &= 0x80;
		data[i] |= layer;
	}
}

template<bool noXor>
void Screen::wrapped_decodeFrameDeltaPage(uint8 *dst, const uint8 *src, int pitch) {
	int count = 0;
	uint8 *dstNext = dst;
	while (1) {
		uint8 code = *src++;
		if (code == 0) {
			uint8 len = *src++;
			code = *src++;
			while (len--) {
				if (noXor)
					*dst++ = code;
				else
					*dst++ ^= code;

				if (++count == pitch) {
					count = 0;
					dstNext += SCREEN_W;
					dst = dstNext;
				}
			}
		} else if (code & 0x80) {
			code -= 0x80;
			if (code != 0) {
				dst += code;

				count += code;
				while (count >= pitch) {
					count -= pitch;
					dstNext += SCREEN_W;
					dst = dstNext + count;
				}
			} else {
				uint16 subcode = READ_LE_UINT16(src); src += 2;
				if (subcode == 0) {
					break;
				} else if (subcode & 0x8000) {
					subcode -= 0x8000;
					if (subcode & 0x4000) {
						uint16 len = subcode - 0x4000;
						code = *src++;
						while (len--) {
							if (noXor)
								*dst++ = code;
							else
								*dst++ ^= code;

							if (++count == pitch) {
								count = 0;
								dstNext += SCREEN_W;
								dst = dstNext;
							}
						}
					} else {
						while (subcode--) {
							if (noXor)
								*dst++ = *src++;
							else
								*dst++ ^= *src++;

							if (++count == pitch) {
								count = 0;
								dstNext += SCREEN_W;
								dst = dstNext;
							}
						}
					}
				} else {
					dst += subcode;

					count += subcode;
					while (count >= pitch) {
						count -= pitch;
						dstNext += SCREEN_W;
						dst = dstNext + count;
					}

				}
			}
		} else {
			while (code--) {
				if (noXor)
					*dst++ = *src++;
				else
					*dst++ ^= *src++;

				if (++count == pitch) {
					count = 0;
					dstNext += SCREEN_W;
					dst = dstNext;
				}
			}
		}
	}
}

uint8 *Screen::encodeShape(int x, int y, int w, int h, int flags) {
	uint8 *srcPtr = &_pagePtrs[_curPage][y * SCREEN_W + x];
	int16 shapeSize = 0;
	uint8 *tmp = srcPtr;
	int xpos = w;

	for (int i = h; i > 0; --i) {
		uint8 *start = tmp;
		shapeSize += w;
		xpos = w;
		while (xpos) {
			uint8 value = *tmp++;
			--xpos;

			if (!value) {
				shapeSize += 2;
				int16 curX = xpos;
				bool skip = false;

				while (xpos) {
					value = *tmp++;
					--xpos;

					if (value) {
						skip = true;
						break;
					}
				}

				if (!skip)
					++curX;

				curX -= xpos;
				shapeSize -= curX;

				while (curX > 0xFF) {
					curX -= 0xFF;
					shapeSize += 2;
				}
			}
		}

		tmp = start + SCREEN_W;
	}

	int16 shapeSize2 = shapeSize;
	if (_vm->gameFlags().useAltShapeHeader)
		shapeSize += 12;
	else
		shapeSize += 10;

	if (flags & 1)
		shapeSize += 16;

	uint8 table[274];
	int tableIndex = 0;

	uint8 *newShape = 0;
	newShape = new uint8[shapeSize+16];
	assert(newShape);

	byte *dst = newShape;

	if (_vm->gameFlags().useAltShapeHeader)
		dst += 2;

	WRITE_LE_UINT16(dst, (flags & 3)); dst += 2;
	*dst = h; dst += 1;
	WRITE_LE_UINT16(dst, w); dst += 2;
	*dst = h; dst += 1;
	WRITE_LE_UINT16(dst, shapeSize); dst += 2;
	WRITE_LE_UINT16(dst, shapeSize2); dst += 2;

	byte *src = srcPtr;
	if (flags & 1) {
		dst += 16;
		memset(table, 0, sizeof(table));
		tableIndex = 1;
	}

	for (int ypos = h; ypos > 0; --ypos) {
		uint8 *srcBackUp = src;
		xpos = w;
		while (xpos) {
			uint8 value = *src++;
			if (value) {
				if (flags & 1) {
					if (!table[value]) {
						if (tableIndex == 16) {
							value = 1;
						} else {
							table[0x100+tableIndex] = value;
							table[value] = tableIndex;
							++tableIndex;
							value = table[value];
						}
					} else {
						value = table[value];
					}
				}
				--xpos;
				*dst++ = value;
			} else {
				int16 temp = 1;
				--xpos;

				while (xpos) {
					if (*src)
						break;
					++src;
					++temp;
					--xpos;
				}

				while (temp > 0xFF) {
					*dst++ = 0;
					*dst++ = 0xFF;
					temp -= 0xFF;
				}

				if (temp & 0xFF) {
					*dst++ = 0;
					*dst++ = temp & 0xFF;
				}
			}
		}
		src = srcBackUp + SCREEN_W;
	}

	if (!(flags & 2)) {
		if (shapeSize > _animBlockSize) {
			dst = newShape;
			if (_vm->gameFlags().useAltShapeHeader)
				dst += 2;

			flags = READ_LE_UINT16(dst);
			flags |= 2;
			WRITE_LE_UINT16(dst, flags);
		} else {
			src = newShape;
			if (_vm->gameFlags().useAltShapeHeader)
				src += 2;
			if (flags & 1)
				src += 16;

			src += 10;
			uint8 *shapePtrBackUp = src;
			dst = _animBlockPtr;
			memcpy(dst, src, shapeSize2);

			int16 size = encodeShapeAndCalculateSize(_animBlockPtr, shapePtrBackUp, shapeSize2);
			if (size > shapeSize2) {
				shapeSize -= shapeSize2 - size;
				uint8 *newShape2 = new uint8[shapeSize];
				assert(newShape2);
				memcpy(newShape2, newShape, shapeSize);
				delete[] newShape;
				newShape = newShape2;
			} else {
				dst = shapePtrBackUp;
				src = _animBlockPtr;
				memcpy(dst, src, shapeSize2);
				dst = newShape;
				if (_vm->gameFlags().useAltShapeHeader)
					dst += 2;
				flags = READ_LE_UINT16(dst);
				flags |= 2;
				WRITE_LE_UINT16(dst, flags);
			}
		}
	}

	dst = newShape;
	if (_vm->gameFlags().useAltShapeHeader)
		dst += 2;
	WRITE_LE_UINT16((dst + 6), shapeSize);

	if (flags & 1) {
		dst = newShape + 10;
		if (_vm->gameFlags().useAltShapeHeader)
			dst += 2;
		src = &table[0x100];
		memcpy(dst, src, sizeof(uint8)*16);
	}

	return newShape;
}

int16 Screen::encodeShapeAndCalculateSize(uint8 *from, uint8 *to, int size_to) {
	byte *fromPtrEnd = from + size_to;
	bool skipPixel = true;
	byte *tempPtr = 0;
	byte *toPtr = to;
	byte *fromPtr = from;
	byte *toPtr2 = to;

	*to++ = 0x81;
	*to++ = *from++;

	while (from < fromPtrEnd) {
		byte *curToPtr = to;
		to = fromPtr;
		int size = 1;

		while (true) {
			byte curPixel = *from;
			if (curPixel == *(from+0x40)) {
				byte *toBackUp = to;
				to = from;

				for (int i = 0; i < (fromPtrEnd - from); ++i) {
					if (*to++ != curPixel)
						break;
				}
				--to;
				uint16 diffSize = (to - from);
				if (diffSize >= 0x41) {
					skipPixel = false;
					from = to;
					to = curToPtr;
					*to++ = 0xFE;
					WRITE_LE_UINT16(to, diffSize); to += 2;
					*to++ = curPixel;
					curToPtr = to;
					to = toBackUp;
					continue;
				} else {
					to = toBackUp;
				}
			}

			bool breakLoop = false;
			while (true) {
				if ((from - to) == 0) {
					breakLoop = true;
					break;
				}
				for (int i = 0; i < (from - to); ++i) {
					if (*to++ == curPixel)
						break;
				}
				if (*(to-1) == curPixel) {
					if (*(from+size-1) != *(to+size-2))
						continue;

					byte *fromBackUp = from;
					byte *toBackUp = to;
					--to;
					const int checkSize = fromPtrEnd - from;
					for (int i = 0; i < checkSize; ++i) {
						if (*from++ != *to++)
							break;
					}
					if (*(from - 1) == *(to - 1))
						++to;
					from = fromBackUp;
					int temp = to - toBackUp;
					to = toBackUp;
					if (temp >= size) {
						size = temp;
						tempPtr = toBackUp - 1;
					}
					break;
				} else {
					breakLoop = true;
					break;
				}
			}

			if (breakLoop)
				break;
		}

		to = curToPtr;
		if (size > 2) {
			uint16 word = 0;
			if (size <= 0x0A) {
				uint16 diffSize = from - tempPtr;
				if (diffSize <= 0x0FFF) {
					byte highByte = ((diffSize & 0xFF00) >> 8) + (((size & 0xFF) - 3) << 4);
					word = ((diffSize & 0xFF) << 8) | highByte;
					WRITE_LE_UINT16(to, word); to += 2;
					from += size;
					skipPixel = false;
					continue;
				}
			}

			if (size > 0x40) {
				*to++ = 0xFF;
				WRITE_LE_UINT16(to, size); to += 2;
			} else {
				*to++ = ((size & 0xFF) - 3) | 0xC0;
			}

			word = tempPtr - fromPtr;
			WRITE_LE_UINT16(to, word); to += 2;
			from += size;
			skipPixel = false;
		} else {
			if (!skipPixel) {
				toPtr2 = to;
				*to++ = 0x80;
			}

			if (*toPtr2 == 0xBF) {
				toPtr2 = to;
				*to++ = 0x80;
			}

			++(*toPtr2);
			*to++ = *from++;
			skipPixel = true;
		}
	}
	*to++ = 0x80;

	return (to - toPtr);
}

void Screen::hideMouse() {
	++_mouseLockCount;
	CursorMan.showMouse(false);
}

void Screen::showMouse() {
	if (_mouseLockCount == 1) {
		CursorMan.showMouse(true);

		// We need to call OSystem::updateScreen here, else the mouse cursor
		// will only be visible on mouse movment.
		_system->updateScreen();
	}

	if (_mouseLockCount > 0)
		_mouseLockCount--;
}


bool Screen::isMouseVisible() const {
	return _mouseLockCount == 0;
}

void Screen::setShapePages(int page1, int page2, int minY, int maxY) {
	_shapePages[0] = _pagePtrs[page1];
	_shapePages[1] = _pagePtrs[page2];
	_maskMinY = minY;
	_maskMaxY = maxY;
}

void Screen::setMouseCursor(int x, int y, const byte *shape) {
	if (!shape)
		return;

	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;

	int mouseHeight = *(shape + 2);
	int mouseWidth = (READ_LE_UINT16(shape + 3)) + 2;

	if (_vm->gameFlags().useAltShapeHeader)
		shape -= 2;

	if (_vm->gameFlags().useHiRes) {
		x <<= 1;
		y <<= 1;
		mouseWidth <<= 1;
		mouseHeight <<= 1;
	}

	uint8 *cursor = new uint8[mouseHeight * mouseWidth];
	fillRect(0, 0, mouseWidth, mouseHeight, _cursorColorKey, 8);
	drawShape(8, shape, 0, 0, 0, 0);

	int xOffset = 0;

	if (_vm->gameFlags().useHiRes) {
		xOffset = mouseWidth;
		scale2x(getPagePtr(8) + mouseWidth, SCREEN_W, getPagePtr(8), SCREEN_W, mouseWidth, mouseHeight);
		postProcessCursor(getPagePtr(8) + mouseWidth, mouseWidth, mouseHeight, SCREEN_W);
	} else {
		postProcessCursor(getPagePtr(8), mouseWidth, mouseHeight, SCREEN_W);
	}

	CursorMan.showMouse(false);
	copyRegionToBuffer(8, xOffset, 0, mouseWidth, mouseHeight, cursor);
	CursorMan.replaceCursor(cursor, mouseWidth, mouseHeight, x, y, _cursorColorKey);
	if (isMouseVisible())
		CursorMan.showMouse(true);
	delete[] cursor;

	// makes sure that the cursor is drawn
	// we do not use Screen::updateScreen here
	// so we can be sure that changes to page 0
	// are NOT updated on the real screen here
	_system->updateScreen();
}

Palette &Screen::getPalette(int num) {
	assert(num >= 0 && (uint)num < _palettes.size());
	return *_palettes[num];
}

void Screen::copyPalette(const int dst, const int src) {
	getPalette(dst).copy(getPalette(src));
}

byte Screen::getShapeFlag1(int x, int y) {
	uint8 color = _shapePages[0][y * SCREEN_W + x];
	color &= 0x80;
	color ^= 0x80;

	if (color & 0x80)
		return 1;
	return 0;
}

byte Screen::getShapeFlag2(int x, int y) {
	uint8 color = _shapePages[0][y * SCREEN_W + x];
	color &= 0x7F;
	color &= 0x87;
	return color;
}

int Screen::getDrawLayer(int x, int y) {
	int xpos = x - 8;
	int ypos = y - 1;
	int layer = 1;

	for (int curX = xpos; curX < xpos + 16; ++curX) {
		int tempLayer = getShapeFlag2(curX, ypos);

		if (layer < tempLayer)
			layer = tempLayer;

		if (layer >= 7)
			return 7;
	}
	return layer;
}

int Screen::getDrawLayer2(int x, int y, int height) {
	int xpos = x - 8;
	int ypos = y - 1;
	int layer = 1;

	for (int useX = xpos; useX < xpos + 16; ++useX) {
		for (int useY = ypos - height; useY < ypos; ++useY) {
			int tempLayer = getShapeFlag2(useX, useY);

			if (tempLayer > layer)
				layer = tempLayer;

			if (tempLayer >= 7)
				return 7;
		}
	}
	return layer;
}


int Screen::setNewShapeHeight(uint8 *shape, int height) {
	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;

	int oldHeight = shape[2];
	shape[2] = height;
	return oldHeight;
}

int Screen::resetShapeHeight(uint8 *shape) {
	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;

	int oldHeight = shape[2];
	shape[2] = shape[5];
	return oldHeight;
}

void Screen::blockInRegion(int x, int y, int width, int height) {
	assert(_shapePages[0]);
	byte *toPtr = _shapePages[0] + (y * 320 + x);
	for (int i = 0; i < height; ++i) {
		byte *backUpTo = toPtr;
		for (int i2 = 0; i2 < width; ++i2)
			*toPtr++ &= 0x7F;
		toPtr = (backUpTo + 320);
	}
}

void Screen::blockOutRegion(int x, int y, int width, int height) {
	assert(_shapePages[0]);
	byte *toPtr = _shapePages[0] + (y * 320 + x);
	for (int i = 0; i < height; ++i) {
		byte *backUpTo = toPtr;
		for (int i2 = 0; i2 < width; ++i2)
			*toPtr++ |= 0x80;
		toPtr = (backUpTo + 320);
	}
}

void Screen::rectClip(int &x, int &y, int w, int h) {
	if (x < 0)
		x = 0;
	else if (x + w >= 320)
		x = 320 - w;

	if (y < 0)
		y = 0;
	else if (y + h >= 200)
		y = 200 - h;
}

void Screen::shakeScreen(int times) {
	while (times--) {
		// seems to be 1 line (320 pixels) offset in the original
		// 4 looks more like dosbox though, maybe check this again
		_system->setShakePos(4);
		_system->updateScreen();
		_system->setShakePos(0);
		_system->updateScreen();
	}
}

void Screen::loadBitmap(const char *filename, int tempPage, int dstPage, Palette *pal, bool skip) {
	uint32 fileSize;
	uint8 *srcData = _vm->resource()->fileData(filename, &fileSize);

	if (!srcData) {
		warning("couldn't load bitmap: '%s'", filename);
		return;
	}

	if (skip)
		srcData += 4;

	const char *ext = filename + strlen(filename) - 3;
	uint8 compType = srcData[2];
	uint32 imgSize = (_vm->game() == GI_KYRA2 && !scumm_stricmp(ext, "CMP")) ? READ_LE_UINT16(srcData) : READ_LE_UINT32(srcData + 4);
	uint16 palSize = READ_LE_UINT16(srcData + 8);

	if (pal && palSize)
		loadPalette(srcData + 10, *pal, palSize);

	uint8 *srcPtr = srcData + 10 + palSize;
	uint8 *dstData = getPagePtr(dstPage);
	memset(dstData, 0, SCREEN_PAGE_SIZE);
	if (dstPage == 0 || tempPage == 0)
		_forceFullUpdate = true;

	switch (compType) {
	case 0:
		memcpy(dstData, srcPtr, imgSize);
		break;
	case 1:
		Screen::decodeFrame1(srcPtr, dstData, imgSize);
		break;
	case 3:
		Screen::decodeFrame3(srcPtr, dstData, imgSize);
		break;
	case 4:
		Screen::decodeFrame4(srcPtr, dstData, imgSize);
		break;
	default:
		error("Unhandled bitmap compression %d", compType);
	}

	if (_isAmiga) {
		if (!scumm_stricmp(ext, "MSC"))
			Screen::convertAmigaMsc(dstData);
		else
			Screen::convertAmigaGfx(dstData, 320, 200);
	}

	if (skip)
		srcData -= 4;

	delete[] srcData;
}

bool Screen::loadPalette(const char *filename, Palette &pal) {
	if (_renderMode == Common::kRenderCGA)
		return true;

	Common::SeekableReadStream *stream = _vm->resource()->createReadStream(filename);

	if (!stream)
		return false;

	debugC(3, kDebugLevelScreen, "Screen::loadPalette('%s', %p)", filename, (const void *)&pal);

	const int maxCols = pal.getNumColors();
	int numCols = 0;

	if (_isAmiga) {
		numCols = stream->size() / Palette::kAmigaBytesPerColor;
		pal.loadAmigaPalette(*stream, 0, MIN(maxCols, numCols));
	} else if (_vm->gameFlags().platform == Common::kPlatformPC98 && _use16ColorMode) {
		numCols = stream->size() / Palette::kPC98BytesPerColor;
		pal.loadPC98Palette(*stream, 0, MIN(maxCols, numCols));
	} else if (_renderMode == Common::kRenderEGA) {
		numCols = stream->size();
		// There aren't any 16 color EGA palette files. So this shouldn't ever get triggered.
		assert (numCols != 16);
		numCols /= Palette::kVGABytesPerColor;
		pal.loadVGAPalette(*stream, 0, numCols);
	} else {
		numCols = stream->size() / Palette::kVGABytesPerColor;
		pal.loadVGAPalette(*stream, 0, MIN(maxCols, numCols));
	}

	if (numCols > maxCols)
		warning("Palette file '%s' includes %d colors, but the target palette only support %d colors", filename, numCols, maxCols);

	delete stream;
	return true;
}

bool Screen::loadPaletteTable(const char *filename, int firstPalette) {
	Common::SeekableReadStream *stream = _vm->resource()->createReadStream(filename);

	if (!stream)
		return false;

	debugC(3, kDebugLevelScreen, "Screen::loadPaletteTable('%s', %d)", filename, firstPalette);

	if (_isAmiga) {
		const int numColors = getPalette(firstPalette).getNumColors();
		const int palSize = getPalette(firstPalette).getNumColors() * Palette::kAmigaBytesPerColor;
		const int numPals = stream->size() / palSize;

		for (int i = 0; i < numPals; ++i)
			getPalette(i + firstPalette).loadAmigaPalette(*stream, 0, numColors);
	} else {
		const int numColors = getPalette(firstPalette).getNumColors();
		const int palSize = getPalette(firstPalette).getNumColors() * Palette::kVGABytesPerColor;
		const int numPals = stream->size() / palSize;

		for (int i = 0; i < numPals; ++i)
			getPalette(i + firstPalette).loadVGAPalette(*stream, 0, numColors);
	}

	delete stream;
	return true;
}

void Screen::loadPalette(const byte *data, Palette &pal, int bytes) {
	Common::MemoryReadStream stream(data, bytes, DisposeAfterUse::NO);

	if (_isAmiga)
		pal.loadAmigaPalette(stream, 0, stream.size() / Palette::kAmigaBytesPerColor);
	else if (_vm->gameFlags().platform == Common::kPlatformPC98 && _use16ColorMode)
		pal.loadPC98Palette(stream, 0, stream.size() / Palette::kPC98BytesPerColor);
	else if (_renderMode == Common::kRenderEGA) {
		// EOB II checks the number of palette bytes to distinguish between real EGA palettes
		// and normal palettes (which are used to generate a color map).
		if (stream.size() == 16)
			pal.loadEGAPalette(stream, 0, stream.size());
		else
			pal.loadVGAPalette(stream, 0, stream.size() / Palette::kVGABytesPerColor);
	} else
		pal.loadVGAPalette(stream, 0, stream.size() / Palette::kVGABytesPerColor);
}

// dirty rect handling

void Screen::addDirtyRect(int x, int y, int w, int h) {
	if (_dirtyRects.size() >= kMaxDirtyRects || _forceFullUpdate) {
		_forceFullUpdate = true;
		return;
	}

	Common::Rect r(x, y, x + w, y + h);

	// Clip rectangle
	r.clip(SCREEN_W, SCREEN_H);

	// If it is empty after clipping, we are done
	if (r.isEmpty())
		return;

	// Check if the new rectangle is contained within another in the list
	Common::List<Common::Rect>::iterator it;
	for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ) {
		// If we find a rectangle which fully contains the new one,
		// we can abort the search.
		if (it->contains(r))
			return;

		// Conversely, if we find rectangles which are contained in
		// the new one, we can remove them
		if (r.contains(*it))
			it = _dirtyRects.erase(it);
		else
			++it;
	}

	// If we got here, we can safely add r to the list of dirty rects.
	_dirtyRects.push_back(r);
}

// overlay functions

byte *Screen::getOverlayPtr(int page) {
	if (page == 0 || page == 1)
		return _sjisOverlayPtrs[1];
	else if (page == 2 || page == 3)
		return _sjisOverlayPtrs[2];

	if (_vm->game() == GI_KYRA2) {
		if (page == 12 || page == 13)
			return _sjisOverlayPtrs[3];
	} else if (_vm->game() == GI_LOL) {
		if (page == 4 || page == 5)
			return _sjisOverlayPtrs[3];
		if (page == 6 || page == 7)
			return _sjisOverlayPtrs[4];
		if (page == 12 || page == 13)
			return _sjisOverlayPtrs[5];
	}

	return 0;
}

void Screen::clearOverlayPage(int page) {
	byte *dst = getOverlayPtr(page);
	if (!dst)
		return;
	memset(dst, _sjisInvisibleColor, SCREEN_OVL_SJIS_SIZE);
}

void Screen::clearOverlayRect(int page, int x, int y, int w, int h) {
	byte *dst = getOverlayPtr(page);

	if (!dst || w < 0 || h < 0)
		return;

	x <<= 1; y <<= 1;
	w <<= 1; h <<= 1;

	dst += y * 640 + x;

	if (w == 640 && h == 400) {
		memset(dst, _sjisInvisibleColor, SCREEN_OVL_SJIS_SIZE);
	} else {
		while (h--) {
			memset(dst, _sjisInvisibleColor, w);
			dst += 640;
		}
	}
}

void Screen::copyOverlayRegion(int x, int y, int x2, int y2, int w, int h, int srcPage, int dstPage) {
	byte *dst = getOverlayPtr(dstPage);
	const byte *src = getOverlayPtr(srcPage);

	if (!dst || !src)
		return;

	x <<= 1; x2 <<= 1;
	y <<= 1; y2 <<= 1;
	w <<= 1; h <<= 1;

	if (w == 640 && h == 400) {
		memcpy(dst, src, SCREEN_OVL_SJIS_SIZE);
	} else {
		dst += y2 * 640 + x2;
		src += y * 640 + x;

		while (h--) {
			for (x = 0; x < w; ++x)
				memcpy(dst, src, w);
			dst += 640;
			src += 640;
		}
	}
}

void Screen::crossFadeRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage) {
	if (srcPage > 13 || dstPage > 13)
		error("Screen::crossFadeRegion(): attempting to use temp page as source or dest page.");

	hideMouse();

	uint16 *wB = (uint16 *)_pagePtrs[14];
	uint8 *hB = _pagePtrs[14] + 640;

	for (int i = 0; i < w; i++)
		wB[i] = i;

	for (int i = 0; i < h; i++)
		hB[i] = i;

	for (int i = 0; i < w; i++)
		SWAP(wB[_vm->_rnd.getRandomNumberRng(0, w - 1)], wB[i]);

	for (int i = 0; i < h; i++)
		SWAP(hB[_vm->_rnd.getRandomNumberRng(0, h - 1)], hB[i]);

	uint8 *s = _pagePtrs[srcPage];
	uint8 *d = _pagePtrs[dstPage];

	for (int i = 0; i < h; i++) {
		int iH = i;
		uint32 end = _system->getMillis() + 3;
		for (int ii = 0; ii < w; ii++) {
			int sX = (x1 + wB[ii]);
			int sY = (y1 + hB[iH]);
			int dX = (x2 + wB[ii]);
			int dY = (y2 + hB[iH]);

			if (++iH >= h)
				iH = 0;

			d[dY * 320 + dX] = s[sY * 320 + sX];
			addDirtyRect(dX, dY, 1, 1);
		}

		// This tries to speed things up, to get similiar speeds as in DOSBox etc.
		// We can't write single pixels directly into the video memory like the original did.
		// We also (unlike the original) want to aim at similiar speeds for all platforms.
		if (!(i % 10))
			updateScreen();

		uint32 cur = _system->getMillis();
		if (end > cur)
			_system->delayMillis(end - cur);
	}

	updateScreen();
	showMouse();
}

#pragma mark -

DOSFont::DOSFont() {
	_data = _widthTable = _heightTable = 0;
	_colorMap = 0;
	_width = _height = _numGlyphs = 0;
	_bitmapOffsets = 0;
}

bool DOSFont::load(Common::SeekableReadStream &file) {
	unload();

	_data = new uint8[file.size()];
	assert(_data);

	file.read(_data, file.size());
	if (file.err())
		return false;

	const uint16 fontSig = READ_LE_UINT16(_data + 2);

	if (fontSig != 0x0500) {
		warning("DOSFont: invalid font: %.04X)", fontSig);
		return false;
	}

	const uint16 descOffset = READ_LE_UINT16(_data + 4);

	_width = _data[descOffset + 5];
	_height = _data[descOffset + 4];
	_numGlyphs = _data[descOffset + 3] + 1;

	_bitmapOffsets = (uint16 *)(_data + READ_LE_UINT16(_data + 6));
	_widthTable = _data + READ_LE_UINT16(_data + 8);
	_heightTable = _data + READ_LE_UINT16(_data + 12);

	for (int i = 0; i < _numGlyphs; ++i)
		_bitmapOffsets[i] = READ_LE_UINT16(&_bitmapOffsets[i]);

	return true;
}

int DOSFont::getCharWidth(uint16 c) const {
	if (c >= _numGlyphs)
		return 0;
	return _widthTable[c];
}

void DOSFont::drawChar(uint16 c, byte *dst, int pitch) const {
	if (c >= _numGlyphs)
		return;

	if (!_bitmapOffsets[c])
		return;

	const uint8 *src = _data + _bitmapOffsets[c];
	const uint8 charWidth = _widthTable[c];

	if (!charWidth)
		return;

	pitch -= charWidth;

	uint8 charH1 = _heightTable[c * 2 + 0];
	uint8 charH2 = _heightTable[c * 2 + 1];
	uint8 charH0 = _height - (charH1 + charH2);

	while (charH1--) {
		uint8 col = _colorMap[0];
		for (int i = 0; i < charWidth; ++i) {
			if (col != 0)
				*dst = col;
			++dst;
		}
		dst += pitch;
	}

	while (charH2--) {
		uint8 b = 0;
		for (int i = 0; i < charWidth; ++i) {
			uint8 col;
			if (i & 1) {
				col = _colorMap[b >> 4];
			} else {
				b = *src++;
				col = _colorMap[b & 0xF];
			}
			if (col != 0) {
				*dst = col;
			}
			++dst;
		}
		dst += pitch;
	}

	while (charH0--) {
		uint8 col = _colorMap[0];
		for (int i = 0; i < charWidth; ++i) {
			if (col != 0)
				*dst = col;
			++dst;
		}
		dst += pitch;
	}
}

void DOSFont::unload() {
	delete[] _data;
	_data = _widthTable = _heightTable = 0;
	_colorMap = 0;
	_width = _height = _numGlyphs = 0;
	_bitmapOffsets = 0;
}


AMIGAFont::AMIGAFont() {
	_width = _height = 0;
	memset(_chars, 0, sizeof(_chars));
}

bool AMIGAFont::load(Common::SeekableReadStream &file) {
	const uint16 dataSize = file.readUint16BE();
	if (dataSize + 2 != file.size())
		return false;

	_width = file.readByte();
	_height = file.readByte();

	// Read the character definition offset table
	uint16 offsets[ARRAYSIZE(_chars)];
	for (int i = 0; i < ARRAYSIZE(_chars); ++i)
		offsets[i] = file.readUint16BE() + 4;

	if (file.err())
		return false;

	for (int i = 0; i < ARRAYSIZE(_chars); ++i) {
		file.seek(offsets[i], SEEK_SET);

		_chars[i].yOffset = file.readByte();
		_chars[i].xOffset = file.readByte();
		_chars[i].width = file.readByte();
		file.readByte(); // unused

		// If the y offset is 255, then the character
		// does not have any bitmap representation
		if (_chars[i].yOffset != 255) {
			Character::Graphics &g = _chars[i].graphics;

			g.width = file.readUint16BE();
			g.height = file.readUint16BE();

			int depth = file.readByte();
			int specialWidth = file.readByte();
			int flags = file.readByte();
			int bytesPerPlane = file.readByte();

			assert(depth != 0 && specialWidth == 0 && flags == 0 && bytesPerPlane != 0);

			// Allocate a temporary buffer to store the plane data
			const int planesSize = bytesPerPlane * g.height * depth;
			uint8 *tempData = new uint8[MAX(g.width * g.height, planesSize)];
			assert(tempData);

			file.read(tempData, planesSize);

			// Convert the plane based graphics to our graphic format
			Screen::convertAmigaGfx(tempData, g.width, g.height, depth, false, bytesPerPlane);

			// Create a buffer perfectly fitting the character
			g.bitmap = new uint8[g.width * g.height];
			assert(g.bitmap);

			memcpy(g.bitmap, tempData, g.width * g.height);
			delete[] tempData;
		}

		if (file.err())
			return false;
	}

	return !file.err();
}

int AMIGAFont::getCharWidth(uint16 c) const {
	if (c >= 255)
		return 0;
	return _chars[c].width;
}

void AMIGAFont::drawChar(uint16 c, byte *dst, int pitch) const {
	if (c >= 255)
		return;

	if (_chars[c].yOffset == 255)
		return;

	dst += _chars[c].yOffset * pitch;
	dst += _chars[c].xOffset;

	pitch -= _chars[c].graphics.width;

	const uint8 *src = _chars[c].graphics.bitmap;
	assert(src);

	for (int y = 0; y < _chars[c].graphics.height; ++y) {
		for (int x = 0; x < _chars[c].graphics.width; ++x) {
			if (*src)
				*dst = *src;
			++src;
			++dst;
		}

		dst += pitch;
	}
}

void AMIGAFont::unload() {
	_width = _height = 0;
	for (int i = 0; i < ARRAYSIZE(_chars); ++i)
		delete[] _chars[i].graphics.bitmap;
	memset(_chars, 0, sizeof(_chars));
}

SJISFont::SJISFont(Graphics::FontSJIS *font, const uint8 invisColor, bool is16Color, bool outlineSize)
    : _colorMap(0), _font(font), _invisColor(invisColor), _is16Color(is16Color) {
	assert(_font);

	_font->setDrawingMode(outlineSize ? Graphics::FontSJIS::kOutlineMode : Graphics::FontSJIS::kDefaultMode);

	_sjisWidth = _font->getMaxFontWidth() >> 1;
	_fontHeight = _font->getFontHeight() >> 1;
	_asciiWidth = _font->getCharWidth('a') >> 1;
}

void SJISFont::unload() {
	delete _font;
	_font = 0;
}

int SJISFont::getHeight() const {
	return _fontHeight;
}

int SJISFont::getWidth() const {
	return _sjisWidth;
}

int SJISFont::getCharWidth(uint16 c) const {
	if (c <= 0x7F || (c >= 0xA1 && c <= 0xDF))
		return _asciiWidth;
	else
		return _sjisWidth;
}

void SJISFont::setColorMap(const uint8 *src) {
	_colorMap = src;

	if (!_is16Color) {
		if (_colorMap[0] == _invisColor)
			_font->setDrawingMode(Graphics::FontSJIS::kDefaultMode);
		else
			_font->setDrawingMode(Graphics::FontSJIS::kOutlineMode);
	}
}

void SJISFont::drawChar(uint16 c, byte *dst, int pitch) const {
	uint8 color1, color2;

	if (_is16Color) {
		// PC98 16 color games specify a color value which is for the
		// PC98 text mode palette, thus we need to remap it.
		color1 = ((_colorMap[1] >> 5) & 0x7) + 16;
		color2 = ((_colorMap[0] >> 5) & 0x7) + 16;
	} else {
		color1 = _colorMap[1];
		color2 = _colorMap[0];
	}

	_font->drawChar(dst, c, 640, 1, color1, color2, 640, 400);
}

#pragma mark -

Palette::Palette(const int numColors) : _palData(0), _numColors(numColors) {
	_palData = new uint8[numColors * 3];
	assert(_palData);

	memset(_palData, 0, numColors * 3);
}

Palette::~Palette() {
	delete[] _palData;
	_palData = 0;
}

void Palette::loadVGAPalette(Common::ReadStream &stream, int startIndex, int colors) {
	assert(startIndex + colors <= _numColors);

	uint8 *pos = _palData + startIndex * 3;
	for (int i = 0 ; i < colors * 3; i++)
		*pos++ = stream.readByte() & 0x3f;
}

void Palette::loadEGAPalette(Common::ReadStream &stream, int startIndex, int colors) {
	assert(startIndex + colors <= 16);

	uint8 *dst = _palData + startIndex * 3;
	for (int i = 0; i < colors; i++) {
		uint8 index = stream.readByte();
		assert(index < _egaNumColors);
		memcpy(dst, &_egaColors[index * 3], 3);
		dst += 3;
	}
}

void Palette::setCGAPalette(int palIndex, CGAIntensity intensity) {
	assert(_numColors >= _cgaNumColors);
	assert(!(palIndex & ~1));
	memcpy(_palData, _cgaColors[palIndex * 2 + intensity], _numColors * 3);
}

void Palette::loadAmigaPalette(Common::ReadStream &stream, int startIndex, int colors) {
	assert(startIndex + colors <= _numColors);

	for (int i = 0; i < colors; ++i) {
		uint16 col = stream.readUint16BE();
		_palData[(i + startIndex) * 3 + 2] = ((col & 0xF) * 0x3F) / 0xF; col >>= 4;
		_palData[(i + startIndex) * 3 + 1] = ((col & 0xF) * 0x3F) / 0xF; col >>= 4;
		_palData[(i + startIndex) * 3 + 0] = ((col & 0xF) * 0x3F) / 0xF; col >>= 4;
	}
}

void Palette::loadPC98Palette(Common::ReadStream &stream, int startIndex, int colors) {
	assert(startIndex + colors <= _numColors);

	for (int i = 0; i < colors; ++i) {
		const byte g = stream.readByte(), r = stream.readByte(), b = stream.readByte();

		_palData[(i + startIndex) * 3 + 0] = ((r & 0xF) * 0x3F) / 0xF;
		_palData[(i + startIndex) * 3 + 1] = ((g & 0xF) * 0x3F) / 0xF;
		_palData[(i + startIndex) * 3 + 2] = ((b & 0xF) * 0x3F) / 0xF;
	}
}

void Palette::clear() {
	memset(_palData, 0, _numColors * 3);
}

void Palette::fill(int firstCol, int numCols, uint8 value) {
	assert(firstCol >= 0 && firstCol + numCols <= _numColors);

	memset(_palData + firstCol * 3, CLIP<int>(value, 0, 63), numCols * 3);
}

void Palette::copy(const Palette &source, int firstCol, int numCols, int dstStart) {
	if (numCols == -1)
		numCols = MIN(source.getNumColors(), _numColors) - firstCol;
	if (dstStart == -1)
		dstStart = firstCol;

	assert(numCols >= 0 && numCols <= _numColors);
	assert(firstCol >= 0 && firstCol <= source.getNumColors());
	assert(dstStart >= 0 && dstStart + numCols <= _numColors);

	memmove(_palData + dstStart * 3, source._palData + firstCol * 3, numCols * 3);
}

void Palette::copy(const uint8 *source, int firstCol, int numCols, int dstStart) {
	if (dstStart == -1)
		dstStart = firstCol;

	assert(numCols >= 0 && numCols <= _numColors);
	assert(firstCol >= 0);
	assert(dstStart >= 0 && dstStart + numCols <= _numColors);

	memmove(_palData + dstStart * 3, source + firstCol * 3, numCols * 3);
}

uint8 *Palette::fetchRealPalette() const {
	uint8 *buffer = new uint8[_numColors * 3];
	assert(buffer);

	uint8 *dst = buffer;
	const uint8 *palData = _palData;

	for (int i = 0; i < _numColors; ++i) {
		dst[0] = (palData[0] << 2) | (palData[0] & 3);
		dst[1] = (palData[1] << 2) | (palData[1] & 3);
		dst[2] = (palData[2] << 2) | (palData[2] & 3);

		dst += 3;
		palData += 3;
	}

	return buffer;
}

const uint8 Palette::_egaColors[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0x00, 0xAA, 0xAA,
	0xAA, 0x00, 0x00, 0xAA,	0x00, 0xAA, 0xAA, 0x55, 0x00, 0xAA, 0xAA, 0xAA,
	0x55, 0x55, 0x55, 0x55, 0x55, 0xFF, 0x55, 0xFF,	0x55, 0x55, 0xFF, 0xFF,
	0xFF, 0x55, 0x55, 0xFF, 0x55, 0xFF, 0xFF, 0xFF, 0x55, 0xFF, 0xFF, 0xFF
};

const int Palette::_egaNumColors = ARRAYSIZE(_egaColors) / 3;

const uint8 Palette::_cgaColors[4][12] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x2A, 0x00, 0x00, 0x2A, 0x15, 0x00 },
	{ 0x00, 0x00, 0x00, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x15, 0x3F, 0x3F, 0x15 },
	{ 0x00, 0x00, 0x00, 0x00, 0x2A, 0x2A, 0x2A, 0x00, 0x2A, 0x2A, 0x2A, 0x2A },
	{ 0x00, 0x00, 0x00, 0x15, 0x3F, 0x3F, 0x3F, 0x15, 0x3F, 0x3F, 0x3F, 0x3F }
};

const int Palette::_cgaNumColors = ARRAYSIZE(_cgaColors[0]) / 3;

} // End of namespace Kyra
