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

// Misc. graphics routines

#include "saga/saga.h"
#include "saga/gfx.h"
#include "saga/interface.h"
#include "saga/resource.h"
#include "saga/scene.h"
#include "saga/render.h"

#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "engines/util.h"

namespace Saga {

#define RID_IHNM_DEFAULT_PALETTE 1
#define RID_IHNM_HOURGLASS_CURSOR 11 // not in demo

Gfx::Gfx(SagaEngine *vm, OSystem *system, int width, int height) : _vm(vm), _system(system) {
	initGraphics(width, height, width > 320);

	debug(5, "Init screen %dx%d", width, height);
	// Convert surface data to R surface data
	_backBuffer.create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	// Start with the cursor shown. It will be hidden before the intro, if
	// there is an intro. (With boot params, there may not be.)
	setCursor(kCursorNormal);
	showCursor(true);
}

Gfx::~Gfx() {
	_backBuffer.free();
}

#ifdef SAGA_DEBUG
void Surface::drawPalette() {
	int x;
	int y;
	int color = 0;
	Rect palRect;

	for (y = 0; y < 16; y++) {
		palRect.top = (y * 8) + 4;
		palRect.bottom = palRect.top + 8;

		for (x = 0; x < 16; x++) {
			palRect.left = (x * 8) + 4;
			palRect.right = palRect.left + 8;

			drawRect(palRect, color);
			color++;
		}
	}
}
#endif

// * Copies a rectangle from a raw 8 bit pixel buffer to the specified surface.
// - The surface must match the logical dimensions of the buffer exactly.
void Surface::blit(const Common::Rect &destRect, const byte *sourceBuffer) {
	const byte *readPointer;
	byte *writePointer;
	int row;
	ClipData clipData;

	clipData.sourceRect.left = 0;
	clipData.sourceRect.top = 0;
	clipData.sourceRect.right = destRect.width();
	clipData.sourceRect.bottom = destRect.height();

	clipData.destPoint.x = destRect.left;
	clipData.destPoint.y = destRect.top;
	clipData.destRect.left = 0;
	clipData.destRect.right = w;
	clipData.destRect.top = 0;
	clipData.destRect.bottom = h;

	if (!clipData.calcClip()) {
		return;
	}

	// Transfer buffer data to surface
	readPointer = (sourceBuffer + clipData.drawSource.x) +
						(clipData.sourceRect.right * clipData.drawSource.y);

	writePointer = ((byte *)pixels + clipData.drawDest.x) + (pitch * clipData.drawDest.y);

	for (row = 0; row < clipData.drawHeight; row++) {
		memcpy(writePointer, readPointer, clipData.drawWidth);

		writePointer += pitch;
		readPointer += clipData.sourceRect.right;
	}
}

void Surface::drawPolyLine(const Point *points, int count, int color) {
	int i;
	if (count >= 3) {
		for (i = 1; i < count; i++) {
			drawLine(points[i].x, points[i].y, points[i - 1].x, points[i - 1].y, color);
		}

		drawLine(points[count - 1].x, points[count - 1].y, points[0].x, points[0].y, color);
	}
}

// Dissolve one image with another. If flags is set to 1, do zero masking.
void Surface::transitionDissolve(const byte *sourceBuffer, const Common::Rect &sourceRect, int flags, double percent) {
#define XOR_MASK 0xB400;
	int pixelcount = w * h;
	int seqlimit = (int)(65535 * percent);
	int seq = 1;
	int i, x1, y1;
	byte color;

	for (i = 0; i < seqlimit; i++) {
		if (seq & 1) {
			seq = (seq >> 1) ^ XOR_MASK;
		} else {
			seq = seq >> 1;
		}

		if (seq == 1) {
			return;
		}

		if (seq >= pixelcount) {
			continue;
		} else {
			x1 = seq % w;
			y1 = seq / w;

			if (sourceRect.contains(x1, y1)) {
				color = sourceBuffer[(x1-sourceRect.left) + sourceRect.width()*(y1-sourceRect.top)];
				if (flags == 0 || color)
					((byte *)pixels)[seq] = color;
			}
		}
	}
}

void Gfx::initPalette() {
	if (_vm->getGameId() == GID_ITE)
		return;

	ResourceContext *resourceContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (resourceContext == NULL) {
		error("Resource::loadGlobalResources() resource context not found");
	}

	ByteArray resourceData;

	_vm->_resource->loadResource(resourceContext, RID_IHNM_DEFAULT_PALETTE, resourceData);

	ByteArrayReadStreamEndian metaS(resourceData);

	for (int i = 0; i < 256; i++) {
		_globalPalette[i].red = metaS.readByte();
		_globalPalette[i].green = metaS.readByte();
		_globalPalette[i].blue = metaS.readByte();
	}

	setPalette(_globalPalette, true);
}

void Gfx::setPalette(const PalEntry *pal, bool full) {
	int i;
	byte *ppal;
	int from, numcolors;

	if (_vm->getGameId() == GID_ITE || full) {
		from = 0;
		numcolors = PAL_ENTRIES;
	} else {
		from = 0;
		numcolors = 248;
	}

	for (i = 0, ppal = &_currentPal[from * 3]; i < numcolors; i++, ppal += 3) {
		ppal[0] = _globalPalette[i].red = pal[i].red;
		ppal[1] = _globalPalette[i].green = pal[i].green;
		ppal[2] = _globalPalette[i].blue = pal[i].blue;
	}

	// Color 0 should always be black in IHNM
	if (_vm->getGameId() == GID_IHNM)
		memset(&_currentPal[0 * 3], 0, 3);

	// Make 256th color black. See bug #1256368
	if ((_vm->getPlatform() == Common::kPlatformMacintosh) && !_vm->_scene->isInIntro())
		memset(&_currentPal[255 * 3], 0, 3);

	_system->getPaletteManager()->setPalette(_currentPal, 0, PAL_ENTRIES);
}

void Gfx::setPaletteColor(int n, int r, int g, int b) {
	bool update = false;

	// This function may get called a lot. To avoid forcing full-screen
	// updates, only update the palette if the color actually changes.

	if (_currentPal[3 * n + 0] != r) {
		_currentPal[3 * n + 0] = _globalPalette[n].red = r;
		update = true;
	}
	if (_currentPal[3 * n + 1] != g) {
		_currentPal[3 * n + 1] = _globalPalette[n].green = g;
		update = true;
	}
	if (_currentPal[3 * n + 2] != b) {
		_currentPal[3 * n + 2] = _globalPalette[n].blue = b;
		update = true;
	}

	if (update)
		_system->getPaletteManager()->setPalette(_currentPal + n * 3, n, 1);
}

void Gfx::getCurrentPal(PalEntry *src_pal) {
	int i;
	byte *ppal;

	for (i = 0, ppal = _currentPal; i < PAL_ENTRIES; i++, ppal += 3) {
		src_pal[i].red = ppal[0];
		src_pal[i].green = ppal[1];
		src_pal[i].blue = ppal[2];
	}
}

void Gfx::palToBlack(PalEntry *srcPal, double percent) {
	int i;
	//int fade_max = 255;
	int new_entry;
	byte *ppal;
	PalEntry *palE;
	int from, numcolors;

	double fpercent;

	if (_vm->getGameId() == GID_ITE) {
		from = 0;
		numcolors = PAL_ENTRIES;
	} else {
		from = 0;
		numcolors = 248;
	}

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	// Use the correct percentage change per frame for each palette entry
	for (i = 0, ppal = _currentPal; i < PAL_ENTRIES; i++, ppal += 3) {
		if (i < from || i >= from + numcolors)
			palE = &_globalPalette[i];
		else
			palE = &srcPal[i];

		new_entry = (int)(palE->red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte) new_entry;
		}

		new_entry = (int)(palE->green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(palE->blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
	}

	// Color 0 should always be black in IHNM
	if (_vm->getGameId() == GID_IHNM)
		memset(&_currentPal[0 * 3], 0, 3);

	// Make 256th color black. See bug #1256368
	if ((_vm->getPlatform() == Common::kPlatformMacintosh) && !_vm->_scene->isInIntro())
		memset(&_currentPal[255 * 3], 0, 3);

	_system->getPaletteManager()->setPalette(_currentPal, 0, PAL_ENTRIES);
}

void Gfx::blackToPal(PalEntry *srcPal, double percent) {
	int new_entry;
	double fpercent;
	byte *ppal;
	int i;
	PalEntry *palE;
	int from, numcolors;

	if (_vm->getGameId() == GID_ITE) {
		from = 0;
		numcolors = PAL_ENTRIES;
	} else {
		from = 0;
		numcolors = 248;
	}

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	// Use the correct percentage change per frame for each palette entry
	for (i = 0, ppal = _currentPal; i < PAL_ENTRIES; i++, ppal += 3) {
		if (i < from || i >= from + numcolors)
			palE = &_globalPalette[i];
		else
			palE = &srcPal[i];

		new_entry = (int)(palE->red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte)new_entry;
		}

		new_entry = (int)(palE->green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(palE->blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
	}

	// Color 0 should always be black in IHNM
	if (_vm->getGameId() == GID_IHNM)
		memset(&_currentPal[0 * 3], 0, 3);

	// Make 256th color black. See bug #1256368
	if ((_vm->getPlatform() == Common::kPlatformMacintosh) && !_vm->_scene->isInIntro())
		memset(&_currentPal[255 * 3], 0, 3);

	_system->getPaletteManager()->setPalette(_currentPal, 0, PAL_ENTRIES);
}

#ifdef ENABLE_IHNM

// Used in IHNM only
void Gfx::palFade(PalEntry *srcPal, int16 from, int16 to, int16 start, int16 numColors, double percent) {
	int i;
	int new_entry;
	byte *ppal;
	PalEntry *palE;

	from = CLIP<int16>(from, 0, 256);
	to   = CLIP<int16>(to,   0, 256);

	if (from == 0 || to == 0) {
		// This case works like palToBlack or blackToPal, so no changes are needed
	} else {
		double x = from > to ? from / to : to / from;
		percent /= x;
		if (from < to)
			percent += 1 / x;
	}

	percent = percent > 1.0 ? 1.0 : percent;
	if (from > to)
		percent = 1.0 - percent;

	byte fadePal[PAL_ENTRIES * 3];

	// Use the correct percentage change per frame for each palette entry
	for (i = start, ppal = fadePal + start * 3; i < start + numColors; i++, ppal += 3) {
		palE = &srcPal[i];

		new_entry = (int)(palE->red * percent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte) new_entry;
		}

		new_entry = (int)(palE->green * percent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(palE->blue * percent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
	}

	// Color 0 should always be black in IHNM
	memset(&fadePal[0 * 3], 0, 3);

	_system->getPaletteManager()->setPalette(&fadePal[start * 3], start, numColors);
}

#endif

void Gfx::showCursor(bool state) {
	// Don't show the mouse cursor in the non-interactive part of the IHNM demo
	if (_vm->_scene->isNonInteractiveIHNMDemoPart())
		state = false;

	CursorMan.showMouse(state);
}

void Gfx::setCursor(CursorType cursorType) {
	if (_vm->getGameId() == GID_ITE) {
		// Set up the mouse cursor
		const byte A = kITEColorLightGrey;
		const byte B = kITEColorWhite;

		const byte cursor_img[CURSOR_W * CURSOR_H] = {
			0, 0, 0, A, 0, 0, 0,
			0, 0, 0, A, 0, 0, 0,
			0, 0, 0, A, 0, 0, 0,
			A, A, A, B, A, A, A,
			0, 0, 0, A, 0, 0, 0,
			0, 0, 0, A, 0, 0, 0,
			0, 0, 0, A, 0, 0, 0,
		};

		CursorMan.replaceCursor(cursor_img, CURSOR_W, CURSOR_H, 3, 3, 0);
	} else {
		uint32 resourceId;

		switch (cursorType) {
		case kCursorBusy:
			if (!_vm->isIHNMDemo())
				resourceId = RID_IHNM_HOURGLASS_CURSOR;
			else
				resourceId = (uint32)-1;
			break;
		default:
			resourceId = (uint32)-1;
			break;
		}

		ByteArray resourceData;
		ByteArray image;
		int width, height;

		if (resourceId != (uint32)-1) {
			ResourceContext *context = _vm->_resource->getContext(GAME_RESOURCEFILE);

			_vm->_resource->loadResource(context, resourceId, resourceData);

			_vm->decodeBGImage(resourceData, image, &width, &height);
		} else {
			width = height = 31;
			image.resize(width * height);

			for (int i = 0; i < 14; i++) {
				image[15 * 31 + i] = 1;
				image[15 * 31 + 30 - i] = 1;
				image[i * 31 + 15] = 1;
				image[(30 - i) * 31 + 15] = 1;
			}
		}

		// Note: Hard-coded hotspot
		CursorMan.replaceCursor(image.getBuffer(), width, height, 15, 15, 0);
	}
}

bool hitTestPoly(const Point *points, unsigned int npoints, const Point& test_point) {
	int yflag0;
	int yflag1;
	bool inside_flag = false;
	unsigned int pt;

	const Point *vtx0 = &points[npoints - 1];
	const Point *vtx1 = &points[0];

	yflag0 = (vtx0->y >= test_point.y);
	for (pt = 0; pt < npoints; pt++, vtx1++) {
		yflag1 = (vtx1->y >= test_point.y);
		if (yflag0 != yflag1) {
			if (((vtx1->y - test_point.y) * (vtx0->x - vtx1->x) >=
				(vtx1->x - test_point.x) * (vtx0->y - vtx1->y)) == yflag1) {
				inside_flag = !inside_flag;
			}
		}
		yflag0 = yflag1;
		vtx0 = vtx1;
	}

	return inside_flag;
}

// This method adds a dirty rectangle automatically
void Gfx::drawFrame(const Common::Point &p1, const Common::Point &p2, int color) {
	Common::Rect rect(MIN(p1.x, p2.x), MIN(p1.y, p2.y), MAX(p1.x, p2.x) + 1, MAX(p1.y, p2.y) + 1);
	_backBuffer.frameRect(rect, color);
	_vm->_render->addDirtyRect(rect);
}

// This method adds a dirty rectangle automatically
void Gfx::drawRect(const Common::Rect &destRect, int color) {
	_backBuffer.drawRect(destRect, color);
	_vm->_render->addDirtyRect(destRect);
}

// This method adds a dirty rectangle automatically
void Gfx::fillRect(const Common::Rect &destRect, uint32 color) {
	_backBuffer.fillRect(destRect, color);
	_vm->_render->addDirtyRect(destRect);
}

// This method adds a dirty rectangle automatically
void Gfx::drawRegion(const Common::Rect &destRect, const byte *sourceBuffer) {
	_backBuffer.blit(destRect, sourceBuffer);
	_vm->_render->addDirtyRect(destRect);
}

// This method does not add a dirty rectangle automatically
void Gfx::drawBgRegion(const Common::Rect &destRect, const byte *sourceBuffer) {
	_backBuffer.blit(destRect, sourceBuffer);
}


} // End of namespace Saga
