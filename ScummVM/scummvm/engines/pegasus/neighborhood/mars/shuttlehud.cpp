/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "pegasus/pegasus.h"
#include "pegasus/neighborhood/mars/constants.h"
#include "pegasus/neighborhood/mars/robotship.h"
#include "pegasus/neighborhood/mars/shuttlehud.h"

namespace Pegasus {

static const CoordType kHUDTargetGridLeft = kShuttleWindowLeft + 16;
static const CoordType kHUDTargetGridTop = kShuttleWindowTop + 8;
static const CoordType kHUDTargetGridWidth = 328;
static const CoordType kHUDTargetGridHeight = 206;

static const CoordType kHUDRS232Left = kHUDTargetGridLeft + 264;
static const CoordType kHUDRS232Top = kHUDTargetGridTop + 2;

static const CoordType kHUDLockLeft = kShuttleWindowLeft + 101;
static const CoordType kHUDLockTop = kShuttleWindowTop + 49;
static const CoordType kHUDLockWidth = 145;
static const CoordType kHUDLockHeight = 124;

static const CoordType kTractorLockWidth = 50;
static const CoordType kTractorLockHeight = 30;

static const CoordType kTractorLockLeft = kShuttleWindowMidH - kTractorLockWidth / 2;
static const CoordType kTractorLockTop = kShuttleWindowMidV - kTractorLockHeight / 2;
static const CoordType kTractorLockRight = kTractorLockLeft + kTractorLockWidth;
static const CoordType kTractorLockBottom = kTractorLockTop + kTractorLockHeight;

static const uint16 s_RS232Data[] = {
	0xF0E1, 0xCE70,
	0xF9E1, 0xEF78,
	0x4900, 0x2108,
	0x79C0, 0xE738,
	0x70E1, 0xC770,
	0x5821, 0x0140,
	0x4DE1, 0xEF78,
	0x45C1, 0xEE78
};

static const uint16 s_lockData[] = {
	0xE007, 0xFE1F, 0xF8E0, 0x7000,
	0xE00F, 0xFF3F, 0xFCE0, 0xE000,
	0xE00E, 0x0738, 0x1CE1, 0xC000,
	0xE00E, 0x0738, 0x00FF, 0x8000,
	0xE00E, 0x0738, 0x00FF, 0x0000,
	0xE00E, 0x0738, 0x00E3, 0x8000,
	0xE00E, 0x0738, 0x1CE1, 0xC000,
	0xFFCF, 0xFF3F, 0xFCE0, 0xE000,
	0xFFC7, 0xFE1F, 0xF8E0, 0x7000
};

#define drawHUDLockLine(x1, y1, x2, y2, penX, penY, color) \
	screen->drawThickLine((x1) + kHUDLockLeft, (y1) + kHUDLockTop, \
			(x2) + kHUDLockLeft, (y2) + kHUDLockTop, penX, penY, color)

#define drawHUDLockArrows(offset, color) \
	drawHUDLockLine(63, 0 + (offset), 68, 5 + (offset), 1, 3, color); \
	drawHUDLockLine(71, 8 + (offset), 77, 14 + (offset), 1, 3, color); \
	drawHUDLockLine(78, 14 + (offset), 84, 8 + (offset), 1, 3, color); \
	drawHUDLockLine(87, 5 + (offset), 92, 0 + (offset), 1, 3, color); \
	drawHUDLockLine(63, 121 - (offset), 68, 116 - (offset), 1, 3, color); \
	drawHUDLockLine(71, 113 - (offset), 77, 107 - (offset), 1, 3, color); \
	drawHUDLockLine(78, 107 - (offset), 84, 113 - (offset), 1, 3, color); \
	drawHUDLockLine(87, 116 - (offset), 92, 121 - (offset), 1, 3, color); \
\
	drawHUDLockLine(13 + (offset), 47, 18 + (offset), 52, 3, 1, color); \
	drawHUDLockLine(21 + (offset), 55, 27 + (offset), 61, 3, 1, color); \
	drawHUDLockLine(27 + (offset), 62, 21 + (offset), 68, 3, 1, color); \
	drawHUDLockLine(18 + (offset), 71, 13 + (offset), 76, 3, 1, color); \
	drawHUDLockLine(142 - (offset), 47, 137 - (offset), 52, 3, 1, color); \
	drawHUDLockLine(134 - (offset), 55, 128 - (offset), 61, 3, 1, color); \
	drawHUDLockLine(128 - (offset), 62, 134 - (offset), 68, 3, 1, color); \
	drawHUDLockLine(137 - (offset), 71, 142 - (offset), 76, 3, 1, color)

ShuttleHUD::ShuttleHUD() : DisplayElement(kNoDisplayElement) {
	_lightGreen = g_system->getScreenFormat().RGBToColor(0, 204, 0);
	_gridDarkGreen = g_system->getScreenFormat().RGBToColor(0, 85, 0);
	_lockDarkGreen1 = g_system->getScreenFormat().RGBToColor(0, 68, 0);
	_lockDarkGreen2 = g_system->getScreenFormat().RGBToColor(0, 65, 0);

	_targetLocked = false;
	setBounds(kShuttleWindowLeft, kShuttleWindowTop, kShuttleWindowLeft + kShuttleWindowWidth,
			kShuttleWindowTop + kShuttleWindowHeight);
	setDisplayOrder(kShuttleHUDOrder);
}

void ShuttleHUD::initShuttleHUD() {
	startDisplaying();
	startIdling();
}

void ShuttleHUD::cleanUpShuttleHUD() {
	stopIdling();
	stopDisplaying();
}

void ShuttleHUD::showTargetGrid() {
	show();
}

void ShuttleHUD::hideTargetGrid() {
	hide();
	unlockOnTarget();
}

void ShuttleHUD::useIdleTime() {
	if (isVisible()) {
		Common::Rect r;
		g_robotShip->getShuttleBounds(r);
		if (r.left < kTractorLockRight && r.right > kTractorLockLeft && r.top < kTractorLockBottom && r.bottom > kTractorLockTop)
			lockOnTarget();
		else
			unlockOnTarget();
	}
}

void ShuttleHUD::lockOnTarget() {
	if (!_targetLocked) {
		_targetLocked = true;
		triggerRedraw();
	}
}

void ShuttleHUD::unlockOnTarget() {
	if (_targetLocked) {
		_targetLocked = false;
		triggerRedraw();
	}
}

void ShuttleHUD::draw(const Common::Rect &) {
	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getWorkArea();

	for (int y = 0; y < 35; y++) {
		Common::Rect r;

		if (y & 1) {
			if (y == 17) {
				r = Common::Rect(0, 0, 4, 2);
				r.moveTo(kHUDTargetGridLeft + 8, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _gridDarkGreen);
				r.moveTo(kHUDTargetGridLeft + kHUDTargetGridWidth - 12, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _gridDarkGreen);

				r = Common::Rect(0, 0, 6, 2);
				r.moveTo(kHUDTargetGridLeft + 2, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _lightGreen);
				r.moveTo(kHUDTargetGridLeft + kHUDTargetGridWidth - 8, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _lightGreen);

				r = Common::Rect(0, 0, 23, 2);
				r.moveTo(kHUDTargetGridLeft + 12, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _lightGreen);
				r.moveTo(kHUDTargetGridLeft + kHUDTargetGridWidth - 35, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _lightGreen);
			} else if (y == 1 || y == 15 || y == 19 || y == 33) {
				r = Common::Rect(0, 0, 4, 2);
				r.moveTo(kHUDTargetGridLeft + 2, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _gridDarkGreen);
				r.moveTo(kHUDTargetGridLeft + kHUDTargetGridWidth - 6, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _gridDarkGreen);

				r = Common::Rect(0, 0, 15, 2);
				r.moveTo(kHUDTargetGridLeft + 8, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _gridDarkGreen);
				r.moveTo(kHUDTargetGridLeft + kHUDTargetGridWidth - 23, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _gridDarkGreen);
			} else {
				r = Common::Rect(0, 0, 4, 2);
				r.moveTo(kHUDTargetGridLeft + 2, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _gridDarkGreen);
				r.moveTo(kHUDTargetGridLeft + kHUDTargetGridWidth - 6, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _gridDarkGreen);

				r = Common::Rect(0, 0, 10, 2);
				r.moveTo(kHUDTargetGridLeft + 8, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _gridDarkGreen);
				r.moveTo(kHUDTargetGridLeft + kHUDTargetGridWidth - 18, y * 6 + kHUDTargetGridTop);
				screen->fillRect(r, _gridDarkGreen);
			}
		} else {
			r = Common::Rect(0, 0, 2, 2);
			r.moveTo(kHUDTargetGridLeft, y * 6 + kHUDTargetGridTop);
			screen->fillRect(r, _gridDarkGreen);
			r.moveTo(kHUDTargetGridLeft + kHUDTargetGridWidth - 2, y * 6 + kHUDTargetGridTop);
			screen->fillRect(r, _gridDarkGreen);

			r = Common::Rect(0, 0, 4, 2);
			r.moveTo(kHUDTargetGridLeft + 8, y * 6 + kHUDTargetGridTop);
			screen->fillRect(r, _gridDarkGreen);
			r.moveTo(kHUDTargetGridLeft + kHUDTargetGridWidth - 12, y * 6 + kHUDTargetGridTop);
			screen->fillRect(r, _gridDarkGreen);
		}
	}

	drawOneBitImageOr(screen, s_RS232Data, 2, Common::Rect(kHUDRS232Left, kHUDRS232Top,
			kHUDRS232Left + 29, kHUDRS232Top + 8), _gridDarkGreen);

	if (_targetLocked) {
		drawHUDLockArrows(0, _lockDarkGreen2);
		drawHUDLockArrows(12, _lockDarkGreen1);
		drawHUDLockArrows(24, _lightGreen);
		drawOneBitImageOr(screen, s_lockData, 4, Common::Rect(kHUDLockLeft, kHUDLockTop + 115,
				kHUDLockLeft + 52, kHUDLockTop + 115 + 9), _lightGreen);
	}
}

void ShuttleHUD::drawOneBitImageOr(Graphics::Surface *screen, const uint16 *data, int pitch, const Common::Rect &bounds, uint32 color) {
	for (int y = 0; y < bounds.height(); y++) {
		for (int x = 0; x < bounds.width(); x++) {
			if ((data[y * pitch + x / 16] & (1 << (15 - (x % 16)))) != 0) {
				if (screen->format.bytesPerPixel == 2)
					WRITE_UINT16((byte *)screen->getBasePtr(x + bounds.left, y + bounds.top), color);
				else
					WRITE_UINT32((byte *)screen->getBasePtr(x + bounds.left, y + bounds.top), color);
			}
		}
	}
}

} // End of namespace Pegasus
