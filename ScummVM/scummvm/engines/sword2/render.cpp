/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/endian.h"
#include "common/system.h"

#include "graphics/primitives.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/screen.h"

namespace Sword2 {

#define RENDERAVERAGETOTAL 4

void Screen::updateRect(Common::Rect *r) {
	_vm->_system->copyRectToScreen(_buffer + r->top * _screenWide + r->left,
		_screenWide, r->left, r->top, r->right - r->left,
		r->bottom - r->top);
}

void Screen::blitBlockSurface(BlockSurface *s, Common::Rect *r, Common::Rect *clipRect) {
	if (!r->intersects(*clipRect))
		return;

	byte *src = s->data;

	if (r->top < clipRect->top) {
		src -= BLOCKWIDTH * (r->top - clipRect->top);
		r->top = clipRect->top;
	}
	if (r->left < clipRect->left) {
		src -= (r->left - clipRect->left);
		r->left = clipRect->left;
	}
	if (r->bottom > clipRect->bottom)
		r->bottom = clipRect->bottom;
	if (r->right > clipRect->right)
		r->right = clipRect->right;

	byte *dst = _buffer + r->top * _screenWide + r->left;
	int i;

	if (s->transparent) {
		for (i = 0; i < r->bottom - r->top; i++) {
			for (int j = 0; j < r->right - r->left; j++) {
				if (src[j])
					dst[j] = src[j];
			}
			src += BLOCKWIDTH;
			dst += _screenWide;
		}
	} else {
		for (i = 0; i < r->bottom - r->top; i++) {
			memcpy(dst, src, r->right - r->left);
			src += BLOCKWIDTH;
			dst += _screenWide;
		}
	}
}

// There are two different separate functions for scaling the image - one fast
// and one good. Or at least that's the theory. I'm sure there are better ways
// to scale an image than this. The latter is used at the highest graphics
// quality setting. Note that the "good" scaler takes extra parameters so that
// it can use the background image when calculating the average pixel value.
//
// This code isn't quite like the original DrawSprite(), but the result should
// be close enough, I hope.

void Screen::scaleImageFast(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight) {
	int x, y;

	for (x = 0; x < dstWidth; x++)
		_xScale[x] = (x * srcWidth) / dstWidth;

	for (y = 0; y < dstHeight; y++)
		_yScale[y] = (y * srcHeight) / dstHeight;

	for (y = 0; y < dstHeight; y++) {
		for (x = 0; x < dstWidth; x++) {
			dst[x] = src[_yScale[y] * srcPitch + _xScale[x]];
		}
		dst += dstPitch;
	}
}

void Screen::scaleImageGood(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight, byte *backBuf, int16 bbXPos, int16 bbYPos) {
	for (int y = 0; y < dstHeight; y++) {
		for (int x = 0; x < dstWidth; x++) {
			uint8 c1, c2, c3, c4;

			uint32 xPos = (x * srcWidth) / dstWidth;
			uint32 yPos = (y * srcHeight) / dstHeight;
			uint32 xFrac = dstWidth - (x * srcWidth) % dstWidth;
			uint32 yFrac = dstHeight - (y * srcHeight) % dstHeight;

			byte *srcPtr = src + yPos * srcPitch + xPos;

			bool transparent = true;

			if (*srcPtr) {
				c1 = *srcPtr;
				transparent = false;
			} else {
				if (bbXPos + x >= 0 &&
				    bbXPos + x < RENDERWIDE &&
				    bbYPos + y >= MENUDEEP &&
				    bbYPos + y < MENUDEEP + RENDERDEEP) {
					c1 = *(backBuf + _screenWide * (bbYPos + y) + bbXPos + x);
				} else {
					c1 = 0;
				}
			}

			if (x < dstWidth - 1) {
				if (*(srcPtr + 1)) {
					c2 = *(srcPtr + 1);
					transparent = false;
				} else {
					if (bbXPos + x + 1 >= 0 &&
					    bbXPos + x + 1 < RENDERWIDE &&
					    bbYPos + y >= MENUDEEP &&
					    bbYPos + y + 1 < MENUDEEP + RENDERDEEP) {
						c2 = *(backBuf + _screenWide * (bbYPos + y) + bbXPos + x + 1);
					} else {
						c2 = c1;
					}
				}
			} else {
				c2 = c1;
			}

			if (y < dstHeight - 1) {
				if (*(srcPtr + srcPitch)) {
					c3 = *(srcPtr + srcPitch);
					transparent = false;
				} else {
					if (bbXPos + x >= 0 &&
					    bbXPos + x < RENDERWIDE &&
					    bbYPos + y + 1 >= MENUDEEP &&
					    bbYPos + y + 1 < MENUDEEP + RENDERDEEP) {
						c3 = *(backBuf + _screenWide * (bbYPos + y + 1) + bbXPos);
					} else {
						c3 = c1;
					}
				}
			} else {
				c3 = c1;
			}

			if (x < dstWidth - 1 && y < dstHeight - 1) {
				if (*(srcPtr + srcPitch + 1)) {
					c4 = *(srcPtr + srcPitch + 1);
					transparent = false;
				} else {
					if (bbXPos + x + 1 >= 0 &&
					    bbXPos + x + 1 < RENDERWIDE &&
					    bbYPos + y + 1 >= MENUDEEP &&
					    bbYPos + y + 1 < MENUDEEP + RENDERDEEP) {
						c4 = *(backBuf + _screenWide * (bbYPos + y + 1) + bbXPos + x + 1);
					} else {
						c4 = c3;
					}
				}
			} else {
				c4 = c3;
			}

			if (!transparent) {
				uint32 r1 = _palette[c1 * 3 + 0];
				uint32 g1 = _palette[c1 * 3 + 1];
				uint32 b1 = _palette[c1 * 3 + 2];

				uint32 r2 = _palette[c2 * 3 + 0];
				uint32 g2 = _palette[c2 * 3 + 1];
				uint32 b2 = _palette[c2 * 3 + 2];

				uint32 r3 = _palette[c3 * 3 + 0];
				uint32 g3 = _palette[c3 * 3 + 1];
				uint32 b3 = _palette[c3 * 3 + 2];

				uint32 r4 = _palette[c4 * 3 + 0];
				uint32 g4 = _palette[c4 * 3 + 1];
				uint32 b4 = _palette[c4 * 3 + 2];

				uint32 r5 = (r1 * xFrac + r2 * (dstWidth - xFrac)) / dstWidth;
				uint32 g5 = (g1 * xFrac + g2 * (dstWidth - xFrac)) / dstWidth;
				uint32 b5 = (b1 * xFrac + b2 * (dstWidth - xFrac)) / dstWidth;

				uint32 r6 = (r3 * xFrac + r4 * (dstWidth - xFrac)) / dstWidth;
				uint32 g6 = (g3 * xFrac + g4 * (dstWidth - xFrac)) / dstWidth;
				uint32 b6 = (b3 * xFrac + b4 * (dstWidth - xFrac)) / dstWidth;

				uint32 r = (r5 * yFrac + r6 * (dstHeight - yFrac)) / dstHeight;
				uint32 g = (g5 * yFrac + g6 * (dstHeight - yFrac)) / dstHeight;
				uint32 b = (b5 * yFrac + b6 * (dstHeight - yFrac)) / dstHeight;

				dst[y * dstWidth + x] = quickMatch(r, g, b);
			} else
				dst[y * dstWidth + x] = 0;
		}
	}
}

/**
 * Plots a point relative to the top left corner of the screen. This is only
 * used for debugging.
 * @param x x-coordinate of the point
 * @param y y-coordinate of the point
 * @param color color of the point
 */

void Screen::plotPoint(int x, int y, uint8 color) {
	byte *buf = _buffer + MENUDEEP * RENDERWIDE;

	x -= _scrollX;
	y -= _scrollY;

	if (x >= 0 && x < RENDERWIDE && y >= 0 && y < RENDERDEEP) {
		buf[y * RENDERWIDE + x] = color;
		markAsDirty(x, y + MENUDEEP, x, y + MENUDEEP);
	}
}

static void plot(int x, int y, int color, void *data) {
	Screen *screen = (Screen *)data;
	screen->plotPoint(x, y, (uint8) color);
}

/**
 * Draws a line from one point to another. This is only used for debugging.
 * @param x0 x-coordinate of the start point
 * @param y0 y-coordinate of the start point
 * @param x1 x-coordinate of the end point
 * @param y1 y-coordinate of the end point
 * @param color color of the line
 */

void Screen::drawLine(int x0, int y0, int x1, int y1, uint8 color) {
	Graphics::drawLine(x0, y0, x1, y1, color, &plot, this);
}

/**
 * This function tells the driver the size of the background screen for the
 * current location.
 * @param w width of the current location
 * @param h height of the current location
 */

void Screen::setLocationMetrics(uint16 w, uint16 h) {
	_locationWide = w;
	_locationDeep = h;
	setNeedFullRedraw();
}

/**
 * Draws a parallax layer at the current position determined by the scroll. A
 * parallax can be either foreground, background or the main screen.
 */

void Screen::renderParallax(byte *ptr, int16 l) {
	int16 x, y;
	uint16 xRes, yRes;
	Common::Rect r;

	if (!ptr)
		return;

	// Fetch resolution data from parallax

	if (Sword2Engine::isPsx()) {
		xRes = READ_LE_UINT16(ptr);
		yRes = READ_LE_UINT16(ptr + 2) * 2;
	} else {
		Parallax p;

		p.read(ptr);
		xRes = p.w;
		yRes = p.h;
	}

	if (_locationWide == _screenWide)
		x = 0;
	else
		x = ((int32)((xRes - _screenWide) * _scrollX) / (int32)(_locationWide - _screenWide));

	if (_locationDeep == _screenDeep - MENUDEEP * 2)
		y = 0;
	else
		y = ((int32)((yRes - (_screenDeep - MENUDEEP * 2)) * _scrollY) / (int32)(_locationDeep - (_screenDeep - MENUDEEP * 2)));

	Common::Rect clipRect;

	// Leave enough space for the top and bottom menues

	clipRect.left = 0;
	clipRect.right = _screenWide;
	clipRect.top = MENUDEEP;
	clipRect.bottom = _screenDeep - MENUDEEP;

	for (int j = 0; j < _yBlocks[l]; j++) {
		for (int i = 0; i < _xBlocks[l]; i++) {
			if (_blockSurfaces[l][i + j * _xBlocks[l]]) {
				r.left = i * BLOCKWIDTH - x;
				r.right = r.left + BLOCKWIDTH;
				r.top = j * BLOCKHEIGHT - y + MENUDEEP;
				r.bottom = r.top + BLOCKHEIGHT;
				blitBlockSurface(_blockSurfaces[l][i + j * _xBlocks[l]], &r, &clipRect);
			}
		}
	}

	_parallaxScrollX = _scrollX - x;
	_parallaxScrollY = _scrollY - y;
}

// Uncomment this when benchmarking the drawing routines.
#define LIMIT_FRAME_RATE

/**
 * Initializes the timers before the render loop is entered.
 */

void Screen::initializeRenderCycle() {
	_initialTime = _vm->_system->getMillis();
	_totalTime = _initialTime + (1000 / _vm->getFramesPerSecond());
}

/**
 * This function should be called when the game engine is ready to start the
 * render cycle.
 */

void Screen::startRenderCycle() {
	_scrollXOld = _scrollX;
	_scrollYOld = _scrollY;

	_startTime = _vm->_system->getMillis();

	if (_startTime + _renderAverageTime >= _totalTime)	{
		_scrollX = _scrollXTarget;
		_scrollY = _scrollYTarget;
		_renderTooSlow = true;
	} else {
		_scrollX = (int16)(_scrollXOld + ((_scrollXTarget - _scrollXOld) * (_startTime - _initialTime + _renderAverageTime)) / (_totalTime - _initialTime));
		_scrollY = (int16)(_scrollYOld + ((_scrollYTarget - _scrollYOld) * (_startTime - _initialTime + _renderAverageTime)) / (_totalTime - _initialTime));
		_renderTooSlow = false;
	}

	if (_scrollXOld != _scrollX || _scrollYOld != _scrollY)
		setNeedFullRedraw();

	_framesPerGameCycle = 0;
}

/**
 * This function should be called at the end of the render cycle.
 * @return true if the render cycle is to be terminated,
 *         or false if it should continue
 */

bool Screen::endRenderCycle() {
	static int32 renderTimeLog[4] = { 60, 60, 60, 60 };
	static int32 renderCountIndex = 0;
	int32 time;

	time = _vm->_system->getMillis();
	renderTimeLog[renderCountIndex] = time - _startTime;
	_startTime = time;
	_renderAverageTime = (renderTimeLog[0] + renderTimeLog[1] + renderTimeLog[2] + renderTimeLog[3]) >> 2;

	_framesPerGameCycle++;

	if (++renderCountIndex == RENDERAVERAGETOTAL)
		renderCountIndex = 0;

	if (_renderTooSlow) {
		initializeRenderCycle();
		return true;
	}

	if (_startTime + _renderAverageTime >= _totalTime) {
		_totalTime += (1000 / _vm->getFramesPerSecond());
		_initialTime = time;
		return true;
	}

#ifdef LIMIT_FRAME_RATE
	if (_scrollXTarget == _scrollX && _scrollYTarget == _scrollY) {
		// If we have already reached the scroll target sleep for the
		// rest of the render cycle.
		_vm->sleepUntil(_totalTime);
		_initialTime = _vm->_system->getMillis();
		_totalTime += (1000 / _vm->getFramesPerSecond());
		return true;
	}
#endif

	// This is an attempt to ensure that we always reach the scroll target.
	// Otherwise the game frequently tries to pump out new interpolation
	// frames without ever getting anywhere.

	if (ABS(_scrollX - _scrollXTarget) <= 1 && ABS(_scrollY - _scrollYTarget) <= 1) {
		_scrollX = _scrollXTarget;
		_scrollY = _scrollYTarget;
	} else {
		_scrollX = (int16)(_scrollXOld + ((_scrollXTarget - _scrollXOld) * (_startTime - _initialTime + _renderAverageTime)) / (_totalTime - _initialTime));
		_scrollY = (int16)(_scrollYOld + ((_scrollYTarget - _scrollYOld) * (_startTime - _initialTime + _renderAverageTime)) / (_totalTime - _initialTime));
	}

	if (_scrollX != _scrollXOld || _scrollY != _scrollYOld)
		setNeedFullRedraw();

#ifdef LIMIT_FRAME_RATE
	// Give the other threads some breathing space. This apparently helps
	// against bug #875683, though I was never able to reproduce it for
	// myself.
	_vm->_system->delayMillis(10);
#endif

	return false;
}

/**
 * Reset scrolling stuff. This function is called from initBackground()
 */

void Screen::resetRenderEngine() {
	_parallaxScrollX = 0;
	_parallaxScrollY = 0;
	_scrollX = 0;
	_scrollY = 0;
}

/**
 * This function should be called five times with either the parallax layer
 * or a NULL pointer in order of background parallax to foreground parallax.
 */

int32 Screen::initializeBackgroundLayer(byte *parallax) {
	Parallax p;
	uint16 i, j, k;
	byte *data;
	byte *dst;

	debug(2, "initializeBackgroundLayer");

	assert(_layer < MAXLAYERS);

	if (!parallax) {
		_layer++;
		return RD_OK;
	}

	p.read(parallax);

	_xBlocks[_layer] = (p.w + BLOCKWIDTH - 1) / BLOCKWIDTH;
	_yBlocks[_layer] = (p.h + BLOCKHEIGHT - 1) / BLOCKHEIGHT;

	_blockSurfaces[_layer] = (BlockSurface **)calloc(_xBlocks[_layer] * _yBlocks[_layer], sizeof(BlockSurface *));
	if (!_blockSurfaces[_layer])
		return RDERR_OUTOFMEMORY;

	// Decode the parallax layer into a large chunk of memory

	byte *memchunk = (byte *)calloc(_xBlocks[_layer] * _yBlocks[_layer], BLOCKWIDTH * BLOCKHEIGHT);
	if (!memchunk)
		return RDERR_OUTOFMEMORY;

	for (i = 0; i < p.h; i++) {
		uint32 p_offset = READ_LE_UINT32(parallax + Parallax::size() + 4 * i);

		if (!p_offset)
			continue;

		byte *pLine = parallax + p_offset;
		uint16 packets = READ_LE_UINT16(pLine);
		uint16 offset = READ_LE_UINT16(pLine + 2);

		data = pLine + 4;
		dst = memchunk + i * p.w + offset;

		if (!packets) {
			memcpy(dst, data, p.w);
			continue;
		}

		bool zeros = false;

		for (j = 0; j < packets; j++) {
			if (zeros) {
				dst += *data;
				offset += *data;
				data++;
				zeros = false;
			} else if (!*data) {
				data++;
				zeros = true;
			} else {
				uint16 count = *data++;
				memcpy(dst, data, count);
				data += count;
				dst += count;
				offset += count;
				zeros = true;
			}
		}
	}

	// The large memory chunk is now divided into a number of smaller
	// surfaces. For most parallax layers, we'll end up using less memory
	// this way, and it will be faster to draw since completely transparent
	// surfaces are discarded.

	for (i = 0; i < _xBlocks[_layer] * _yBlocks[_layer]; i++) {
		bool block_has_data = false;
		bool block_is_transparent = false;

		int x = BLOCKWIDTH * (i % _xBlocks[_layer]);
		int y = BLOCKHEIGHT * (i / _xBlocks[_layer]);

		data = memchunk + p.w * y + x;

		for (j = 0; j < BLOCKHEIGHT; j++) {
			for (k = 0; k < BLOCKWIDTH; k++) {
				if (x + k < p.w && y + j < p.h) {
					if (data[j * p.w + k])
						block_has_data = true;
					else
						block_is_transparent = true;
				}
			}
		}

		//  Only assign a surface to the block if it contains data.

		if (block_has_data) {
			_blockSurfaces[_layer][i] = (BlockSurface *)malloc(sizeof(BlockSurface));

			//  Copy the data into the surfaces.
			dst = _blockSurfaces[_layer][i]->data;
			for (j = 0; j < BLOCKHEIGHT; j++) {
				memcpy(dst, data, BLOCKWIDTH);
				data += p.w;
				dst += BLOCKWIDTH;
			}

			_blockSurfaces[_layer][i]->transparent = block_is_transparent;

		} else
			_blockSurfaces[_layer][i] = NULL;
	}

	free(memchunk);
	_layer++;

	return RD_OK;
}

/**
 * This converts PSX format background data into a format that
 * can be understood by renderParallax functions.
 * PSX Backgrounds are divided into tiles of 64x32 (with aspect
 * ratio correction), while PC backgrounds are in tiles of 64x64.
 */

int32 Screen::initializePsxBackgroundLayer(byte *parallax) {
	uint16 bgXres, bgYres;
	uint16 trueXres, stripeNumber, totStripes;
	uint32 baseAddress, stripePos;
	uint16 i, j;
	byte *dst;

	debug(2, "initializePsxBackgroundLayer");

	assert(_layer < MAXLAYERS);

	if (!parallax) {
		_layer++;
		return RD_OK;
	}

	// Fetch data from buffer

	bgXres = READ_LE_UINT16(parallax);
	bgYres = READ_LE_UINT16(parallax + 2) * 2;
	baseAddress = READ_LE_UINT32(parallax + 4);
	parallax += 8;

	// Calculate TRUE resolution of background, must be
	// a multiple of 64

	trueXres = (bgXres % 64) ? ((bgXres/64) + 1) * 64 : bgXres;
	totStripes = trueXres / 64;

	_xBlocks[_layer] = (bgXres + BLOCKWIDTH - 1) / BLOCKWIDTH;
	_yBlocks[_layer] = (bgYres + BLOCKHEIGHT - 1) / BLOCKHEIGHT;

	uint16 remLines = bgYres % 64;

	byte *tileChunk = (byte *)malloc(BLOCKHEIGHT * BLOCKWIDTH);
	if (!tileChunk)
		return RDERR_OUTOFMEMORY;

	_blockSurfaces[_layer] = (BlockSurface **)calloc(_xBlocks[_layer] * _yBlocks[_layer], sizeof(BlockSurface *));
	if (!_blockSurfaces[_layer]) {
		free(tileChunk);
		return RDERR_OUTOFMEMORY;
	}

	// Group PSX background (64x32, when stretched vertically) tiles together,
	// to make them compatible with pc version (composed by 64x64 tiles)

	stripeNumber = 0;
	stripePos = 0;
	for (i = 0; i < _xBlocks[_layer] * _yBlocks[_layer]; i++) {
		bool block_has_data = false;
		bool block_is_transparent = false;

		int posX = i / _yBlocks[_layer];
		int posY = i % _yBlocks[_layer];

		uint32 stripeOffset = READ_LE_UINT32(parallax + stripeNumber * 8 + 4) + stripePos - baseAddress;

		memset(tileChunk, 1, BLOCKHEIGHT * BLOCKWIDTH);

		if (!(remLines && posY == _yBlocks[_layer] - 1))
			remLines = 32;

		for (j = 0; j < remLines; j++) {
			memcpy(tileChunk + j * BLOCKWIDTH * 2, parallax + stripeOffset + j * BLOCKWIDTH, BLOCKWIDTH);
			memcpy(tileChunk + j * BLOCKWIDTH * 2 + BLOCKWIDTH, parallax + stripeOffset + j * BLOCKWIDTH, BLOCKWIDTH);
		}

		for (j = 0; j < BLOCKHEIGHT * BLOCKWIDTH; j++) {
			if (tileChunk[j])
				block_has_data = true;
			else
				block_is_transparent = true;
		}

		int tileIndex = totStripes * posY + posX;

		//  Only assign a surface to the block if it contains data.

		if (block_has_data) {
			_blockSurfaces[_layer][tileIndex] = (BlockSurface *)malloc(sizeof(BlockSurface));

			//  Copy the data into the surfaces.
			dst = _blockSurfaces[_layer][tileIndex]->data;
			memcpy(dst, tileChunk, BLOCKWIDTH * BLOCKHEIGHT);

			_blockSurfaces[_layer][tileIndex]->transparent = block_is_transparent;

		} else
			_blockSurfaces[_layer][tileIndex] = NULL;

		if (posY == _yBlocks[_layer] - 1) {
			stripeNumber++;
			stripePos = 0;
		} else {
			stripePos += 0x800;
		}
	}

	free(tileChunk);
	_layer++;

	return RD_OK;
}

/**
 * This converts PSX format parallax data into a format that
 * can be understood by renderParallax functions.
 */

int32 Screen::initializePsxParallaxLayer(byte *parallax) {
	uint16 i, j, k;
	byte *data;
	byte *dst;

	debug(2, "initializePsxParallaxLayer");

	assert(_layer < MAXLAYERS);

	if (!parallax) {
		_layer++;
		return RD_OK;
	}

	// uint16 plxXres = READ_LE_UINT16(parallax);
	// uint16 plxYres = READ_LE_UINT16(parallax + 2);
	uint16 xTiles = READ_LE_UINT16(parallax + 4);
	uint16 yTiles = READ_LE_UINT16(parallax + 6);

	// Beginning of parallax table composed by uint32,
	// if word is 0, corresponding tile contains no data and must be skipped,
	// if word is 0x400 tile contains data.
	parallax += 8;

	// Beginning if tiles data.
	data = parallax + xTiles * yTiles * 4;

	_xBlocks[_layer] = xTiles;
	_yBlocks[_layer] = (yTiles / 2) + ((yTiles % 2) ? 1 : 0);
	bool oddTiles = ((yTiles % 2) ? true : false);

	_blockSurfaces[_layer] = (BlockSurface **)calloc(_xBlocks[_layer] * _yBlocks[_layer], sizeof(BlockSurface *));
	if (!_blockSurfaces[_layer])
		return RDERR_OUTOFMEMORY;

	// We have to check two tiles for every block in PSX version, if one of those
	// has data in it, the whole block has data. Also, tiles must be doublelined to
	// get correct aspect ratio.
	for (i = 0; i < _xBlocks[_layer] * _yBlocks[_layer]; i++) {
		bool block_has_data = false;
		bool block_is_transparent = false;
		bool firstTilePresent, secondTilePresent;

		int posX = i / _yBlocks[_layer];
		int posY = i % _yBlocks[_layer];

		if (oddTiles && posY == _yBlocks[_layer] - 1) {
			firstTilePresent = READ_LE_UINT32(parallax) == 0x400;
			secondTilePresent = false;
			parallax += 4;
		} else {
			firstTilePresent = READ_LE_UINT32(parallax) == 0x400;
			secondTilePresent = READ_LE_UINT32(parallax + 4) == 0x400;
			parallax += 8;
		}

		// If one of the two grouped tiles has data, then the whole block has data
		if (firstTilePresent || secondTilePresent) {
			block_has_data = true;

			// If one of the two grouped blocks is without data, then we also have transparency
			if (!firstTilePresent || !secondTilePresent)
				block_is_transparent = true;
		}

		// Now do a second check to see if we have a partially transparent block
		if (block_has_data && !block_is_transparent) {
			byte *block = data;
			if (firstTilePresent) {
				for (k = 0; k < 0x400; k++) {
					if (*(block + k) == 0) {
						block_is_transparent = true;
						break;
					}
				}
				block += 0x400; // On to next block...
			}

			// If we didn't find transparency in first block and we have
			// a second tile, check it
			if (secondTilePresent && !block_is_transparent) {
				for (k = 0; k < 0x400; k++) {
					if (*(block + k) == 0) {
						block_is_transparent = true;
						break;
					}
				}
			}
		}

		int tileIndex = xTiles * posY + posX;

		//  Only assign a surface to the block if it contains data.

		if (block_has_data) {
			_blockSurfaces[_layer][tileIndex] = (BlockSurface *)malloc(sizeof(BlockSurface));
			memset(_blockSurfaces[_layer][tileIndex], 0, BLOCKHEIGHT * BLOCKWIDTH);

			//  Copy the data into the surfaces.
			dst = _blockSurfaces[_layer][tileIndex]->data;

			if (firstTilePresent) { //There is data in the first tile
				for (j = 0; j < 16; j++) {
					memcpy(dst, data, BLOCKWIDTH);
					dst += BLOCKWIDTH;
					memcpy(dst, data, BLOCKWIDTH);
					dst += BLOCKWIDTH;
					data += BLOCKWIDTH;
				}
			} else {
				dst += 0x800;
			}

			if (secondTilePresent) {
				for (j = 0; j < 16; j++) {
					memcpy(dst, data, BLOCKWIDTH);
					dst += BLOCKWIDTH;
					memcpy(dst, data, BLOCKWIDTH);
					dst += BLOCKWIDTH;
					data += BLOCKWIDTH;
				}
			}

			_blockSurfaces[_layer][tileIndex]->transparent = block_is_transparent;
		} else
			_blockSurfaces[_layer][tileIndex] = NULL;
	}

	_layer++;

	return RD_OK;
}

/**
 * Should be called once after leaving the room to free up memory.
 */

void Screen::closeBackgroundLayer() {
	debug(2, "CloseBackgroundLayer");

	if (Sword2Engine::isPsx())
		flushPsxScrCache();

	for (int i = 0; i < MAXLAYERS; i++) {
		if (_blockSurfaces[i]) {
			for (int j = 0; j < _xBlocks[i] * _yBlocks[i]; j++)
				if (_blockSurfaces[i][j])
					free(_blockSurfaces[i][j]);
			free(_blockSurfaces[i]);
			_blockSurfaces[i] = NULL;
		}
	}

	_layer = 0;
}

} // End of namespace Sword2
