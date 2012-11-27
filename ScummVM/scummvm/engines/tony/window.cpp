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

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "common/scummsys.h"
#include "graphics/surface.h"
#include "util.h"
#include "tony/window.h"
#include "tony/game.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*       RMWindow Methods
\****************************************************************************/

RMWindow::RMWindow() {
	_showDirtyRects = false;
}

RMWindow::~RMWindow() {
	close();
	RMText::unload();
	RMGfxTargetBuffer::freeBWPrecalcTable();
}

/**
 * Initializes the graphics window
 */
void RMWindow::init() {
	Graphics::PixelFormat pixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
	initGraphics(RM_SX, RM_SY, true, &pixelFormat);

	_bGrabScreenshot = false;
	_bGrabThumbnail = false;
	_bGrabMovie = false;
	_wiping = false;
}

void RMWindow::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	if (GLOBALS._bCfgAnni30) {
		if (!RMGfxTargetBuffer::_precalcTable) {
			RMGfxTargetBuffer::createBWPrecalcTable();
			g_vm->getEngine()->getPointer().updateCursor();
		}
		Graphics::Surface *screen = g_system->lockScreen();
		const uint16 *src = (const uint16 *)buf;
		for (int i = 0; i < h; i++) {
			uint16 *dst = (uint16 *)screen->getBasePtr(x, y + i);
			for (int j = 0; j < w; j++) {
				dst[j] = RMGfxTargetBuffer::_precalcTable[src[j] & 0x7FFF];
			}
			src += (pitch / 2);
		}
		g_system->unlockScreen();
	} else {
		if (RMGfxTargetBuffer::_precalcTable) {
			RMGfxTargetBuffer::freeBWPrecalcTable();
			g_vm->getEngine()->getPointer().updateCursor();
		}
		g_system->copyRectToScreen(buf, pitch, x, y, w, h);
	}
 }

/**
 * Close the window
 */
void RMWindow::close() {
}

void RMWindow::grabThumbnail(uint16 *thumbmem) {
	_bGrabThumbnail = true;
	_wThumbBuf = thumbmem;
}

/**
 * Repaint the screen
 */
void RMWindow::repaint() {
	g_system->updateScreen();
}

/**
 * Wipes an area of the screen
 */
void RMWindow::wipeEffect(Common::Rect &rcBoundEllipse) {
	if ((rcBoundEllipse.left == 0) && (rcBoundEllipse.top == 0) &&
	    (rcBoundEllipse.right == RM_SX) && (rcBoundEllipse.bottom == RM_SY)) {
		// Full screen clear wanted, so use shortcut method
		g_system->fillScreen(0);
	} else {
		// Clear the designated area a line at a time
		uint16 line[RM_SX];
		Common::fill(line, line + RM_SX, 0);

		// Loop through each line
		for (int yp = rcBoundEllipse.top; yp < rcBoundEllipse.bottom; ++yp) {
			copyRectToScreen((const byte *)&line[0], RM_SX * 2, rcBoundEllipse.left, yp, rcBoundEllipse.width(), 1);
		}
	}
}

void RMWindow::getNewFrame(RMGfxTargetBuffer &bigBuf, Common::Rect *rcBoundEllipse) {
	// Get a pointer to the bytes of the source buffer
	byte *lpBuf = bigBuf;

	if (rcBoundEllipse != NULL) {
		// Circular wipe effect
		getNewFrameWipe(lpBuf, *rcBoundEllipse);
		_wiping = true;
	} else if (_wiping) {
		// Just finished a wiping effect, so copy the full screen
		copyRectToScreen(lpBuf, RM_SX * 2, 0, 0, RM_SX, RM_SY);
		_wiping = false;

	} else {
		// Standard screen copy - iterate through the dirty rects
		Common::List<Common::Rect> dirtyRects = bigBuf.getDirtyRects();
		Common::List<Common::Rect>::iterator i;

		// If showing dirty rects, copy the entire screen background and set up a surface pointer
		Graphics::Surface *s = NULL;
		if (_showDirtyRects) {
			copyRectToScreen(lpBuf, RM_SX * 2, 0, 0, RM_SX, RM_SY);
			s = g_system->lockScreen();
		}

		for (i = dirtyRects.begin(); i != dirtyRects.end(); ++i) {
			Common::Rect &r = *i;
			const byte *lpSrc = lpBuf + (RM_SX * 2) * r.top + (r.left * 2);
			copyRectToScreen(lpSrc, RM_SX * 2, r.left, r.top, r.width(), r.height());
		}

		if (_showDirtyRects) {
			for (i = dirtyRects.begin(); i != dirtyRects.end(); ++i) {
				// Frame the copied area with a rectangle
				s->frameRect(*i, 0xffffff);
			}

			g_system->unlockScreen();
		}
	}

	if (_bGrabThumbnail) {
		// Need to generate a thumbnail
		RMSnapshot s;

		s.grabScreenshot(lpBuf, 4, _wThumbBuf);
		_bGrabThumbnail = false;
	}

	// Clear the dirty rect list
	bigBuf.clearDirtyRects();
}

/**
 * Copies a section of the game frame in a circle bounded by the specified rectangle
 */
void RMWindow::getNewFrameWipe(byte *lpBuf, Common::Rect &rcBoundEllipse) {
	// Clear the screen
	g_system->fillScreen(0);

	if (!rcBoundEllipse.isValidRect())
		return;

	Common::Point center(rcBoundEllipse.left + rcBoundEllipse.width() / 2,
	                     rcBoundEllipse.top + rcBoundEllipse.height() / 2);

	// The rectangle technically defines the area inside the ellipse, with the corners touching
	// the ellipse boundary. Since we're currently simulating the ellipse using a plain circle,
	// we need to calculate a necessary width using the hypotenuse of X/2 & Y/2
	int x2y2 = (rcBoundEllipse.width() / 2) * (rcBoundEllipse.width() / 2) +
	           (rcBoundEllipse.height() / 2) * (rcBoundEllipse.height() / 2);
	int radius = 0;
	while ((radius * radius) < x2y2)
		++radius;

	// Proceed copying a circular area of the frame with the calculated radius onto the screen
	int error = -radius;
	int x = radius;
	int y = 0;

	while (x >= y) {
		plotSplices(lpBuf, center, x, y);

		error += y;
		++y;
		error += y;

		if (error >= 0) {
			error -= x;
			--x;
			error -= x;
		}
	}
}

/**
 * Handles drawing the line splices for the circle of viewable area
 */
void RMWindow::plotSplices(const byte *lpBuf, const Common::Point &center, int x, int y) {
	plotLines(lpBuf, center, x, y);
	if (x != y)
		plotLines(lpBuf, center, y, x);
}

/**
 * Handles drawing the line splices for the circle of viewable area
 */
void RMWindow::plotLines(const byte *lpBuf, const Common::Point &center, int x, int y) {
	// Skips lines that have no width (i.e. at the top of the circle)
	if ((x == 0) || (y > center.y))
		return;

	const byte *pSrc;
	int xs = MAX(center.x - x, 0);
	int width = MIN(RM_SX - xs, x * 2);

	if ((center.y - y) >= 0) {
		// Draw line in top half of circle
		pSrc = lpBuf + ((center.y - y) * RM_SX * 2) + xs * 2;
		copyRectToScreen(pSrc, RM_SX * 2, xs, center.y - y, width, 1);
	}

	if ((center.y + y) < RM_SY) {
		// Draw line in bottom half of circle
		pSrc = lpBuf + ((center.y + y) * RM_SX * 2) + xs * 2;
		copyRectToScreen(pSrc, RM_SX * 2, xs, center.y + y, width, 1);
	}
}

void RMWindow::showDirtyRects(bool v) {
	_showDirtyRects = v;
}

/****************************************************************************\
*       RMSnapshot Methods
\****************************************************************************/

void RMSnapshot::grabScreenshot(byte *lpBuf, int dezoom, uint16 *lpDestBuf) {
	uint16 *src = (uint16 *)lpBuf;

	int dimx = RM_SX / dezoom;
	int dimy = RM_SY / dezoom;

	uint16 *cursrc;

	if (lpDestBuf == NULL)
		src += (RM_SY - 1) * RM_BBX;

	if (dezoom == 1 && 0) {
		byte *curOut = _rgb;

		for (int y = 0; y < dimy; y++) {
			for (int x = 0; x < dimx; x++) {
				cursrc = &src[RM_SKIPX + x];

				*curOut++ = ((*cursrc) & 0x1F) << 3;
				*curOut++ = (((*cursrc) >> 5) & 0x1F) << 3;
				*curOut++ = (((*cursrc) >> 10) & 0x1F) << 3;

				if (lpDestBuf)
					*lpDestBuf++ = *cursrc;
			}

			if (lpDestBuf == NULL)
				src -= RM_BBX;
			else
				src += RM_BBX;
		}
	} else {
		uint32 k = 0;
		for (int y = 0; y < dimy; y++) {
			for (int x = 0; x < dimx; x++) {
				cursrc = &src[RM_SKIPX + x * dezoom];
				int sommar, sommab, sommag, curv;
				sommar = sommab = sommag = 0;

				for (int v = 0; v < dezoom; v++) {
					for (int u = 0; u < dezoom; u++) {
						if (lpDestBuf == NULL)
							curv = -v;
						else
							curv = v;

						sommab += cursrc[curv * RM_BBX + u] & 0x1F;
						sommag += (cursrc[curv * RM_BBX + u] >> 5) & 0x1F;
						sommar += (cursrc[curv * RM_BBX + u] >> 10) & 0x1F;
					}
				}
				_rgb[k + 0] = (byte)(sommab * 8 / (dezoom * dezoom));
				_rgb[k + 1] = (byte)(sommag * 8 / (dezoom * dezoom));
				_rgb[k + 2] = (byte)(sommar * 8 / (dezoom * dezoom));

				if (lpDestBuf != NULL)
					lpDestBuf[k / 3] = ((int)_rgb[k + 0] >> 3) | (((int)_rgb[k + 1] >> 3) << 5) |
					                   (((int)_rgb[k + 2] >> 3) << 10);

				k += 3;
			}

			if (lpDestBuf == NULL)
				src -= RM_BBX * dezoom;
			else
				src += RM_BBX * dezoom;
		}
	}
}

} // End of namespace Tony
