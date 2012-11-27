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

#include "common/events.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "engines/util.h"

#include "pegasus/elements.h"
#include "pegasus/graphics.h"
#include "pegasus/transition.h"

namespace Pegasus {

GraphicsManager::GraphicsManager(PegasusEngine *vm) : _vm(vm) {
	initGraphics(640, 480, true, NULL);

	if (_vm->_system->getScreenFormat().bytesPerPixel == 1)
		error("No true color mode available");

	_backLayer = kMinAvailableOrder;
	_frontLayer = kMaxAvailableOrder;
	_firstDisplayElement = _lastDisplayElement = 0;
	_workArea.create(640, 480, _vm->_system->getScreenFormat());
	_modifiedScreen = false;
	_curSurface = &_workArea;
	_erase = false;
	_updatesEnabled = true;
	_screenFader = new ScreenFader();
}

GraphicsManager::~GraphicsManager() {
	_workArea.free();
	delete _screenFader;
}

void GraphicsManager::invalRect(const Common::Rect &rect) {
	// We're using a simpler algorithm for dirty rect handling than the original
	// The original was way too overcomplicated for what we need here now.

	if (_dirtyRect.width() == 0 || _dirtyRect.height() == 0) {
		// We have no dirty rect, so this is now our dirty rect
		_dirtyRect = rect;
	} else {
		// Expand our dirty rect to include rect
		_dirtyRect.extend(rect);
	}

	// Sanity check: clip our rect to the screen
	_dirtyRect.right = MIN<int>(640, _dirtyRect.right);
	_dirtyRect.bottom = MIN<int>(480, _dirtyRect.bottom);
}

void GraphicsManager::addDisplayElement(DisplayElement *newElement) {
	newElement->_elementOrder = CLIP<int>(newElement->_elementOrder, kMinAvailableOrder, kMaxAvailableOrder);

	if (_firstDisplayElement) {
		DisplayElement *runner = _firstDisplayElement;
		DisplayElement *lastRunner = 0;

		// Search for first element whose display order is greater than
		// the new element's and add the new element just before it.
		while (runner) {
			if (newElement->_elementOrder < runner->_elementOrder) {
				if (lastRunner) {
					lastRunner->_nextElement = newElement;
					newElement->_nextElement = runner;
				} else {
					newElement->_nextElement = _firstDisplayElement;
					_firstDisplayElement = newElement;
				}
				break;
			}
			lastRunner = runner;
			runner = runner->_nextElement;
		}

		// If got here and runner == NULL, we ran through the whole list without
		// inserting, so add at the end.
		if (!runner) {
			_lastDisplayElement->_nextElement = newElement;
			_lastDisplayElement = newElement;
		}
	} else {
		_firstDisplayElement = newElement;
		_lastDisplayElement = newElement;
	}

	newElement->_elementIsDisplaying = true;
}

void GraphicsManager::removeDisplayElement(DisplayElement *oldElement) {
	if (!_firstDisplayElement)
		return;

	if (oldElement == _firstDisplayElement) {
		if (oldElement == _lastDisplayElement) {
			_firstDisplayElement = 0;
			_lastDisplayElement = 0;
		} else {
			_firstDisplayElement = oldElement->_nextElement;
		}

		invalRect(oldElement->_bounds);
	} else {
		// Scan list for element.
		// If we get here, we know that the list has at least one item, and it
		// is not the first item, so we can skip it.
		DisplayElement *runner = _firstDisplayElement->_nextElement;
		DisplayElement *lastRunner = _firstDisplayElement;

		while (runner) {
			if (runner == oldElement) {
				lastRunner->_nextElement = runner->_nextElement;

				if (oldElement == _lastDisplayElement)
					_lastDisplayElement = lastRunner;

				invalRect(oldElement->_bounds);
				break;
			}

			lastRunner = runner;
			runner = runner->_nextElement;
		}
	}

	oldElement->_nextElement = 0;
	oldElement->_elementIsDisplaying = false;
}

void GraphicsManager::updateDisplay() {
	bool screenDirty = false;

	if (!_dirtyRect.isEmpty()) {
		// Fill the dirty area with black if erase mode is enabled
		if (_erase)
			_workArea.fillRect(_dirtyRect, _workArea.format.RGBToColor(0, 0, 0));

		for (DisplayElement *runner = _firstDisplayElement; runner != 0; runner = runner->_nextElement) {
			Common::Rect bounds;
			runner->getBounds(bounds);

			// TODO: Better logic; it does a bit more work than it probably needs to
			// but it should work fine for now.
			if (bounds.intersects(_dirtyRect) && runner->validToDraw(_backLayer, _frontLayer)) {
				runner->draw(bounds);
				screenDirty = true;
			}
		}

		// Copy only the dirty rect to the screen
		if (screenDirty)
			g_system->copyRectToScreen((byte *)_workArea.getBasePtr(_dirtyRect.left, _dirtyRect.top), _workArea.pitch, _dirtyRect.left, _dirtyRect.top, _dirtyRect.width(), _dirtyRect.height());

		// Clear the dirty rect
		_dirtyRect = Common::Rect();
	}

	if (_updatesEnabled && (screenDirty || _modifiedScreen))
		g_system->updateScreen();

	_modifiedScreen = false;
}

void GraphicsManager::clearScreen() {
	Graphics::Surface *screen = g_system->lockScreen();
	screen->fillRect(Common::Rect(0, 0, 640, 480), g_system->getScreenFormat().RGBToColor(0, 0, 0));
	g_system->unlockScreen();
	_modifiedScreen = true;
}

DisplayElement *GraphicsManager::findDisplayElement(const DisplayElementID id) {
	DisplayElement *runner = _firstDisplayElement;

	while (runner) {
		if (runner->getObjectID() == id)
			return runner;
		runner = runner->_nextElement;
	}

	return 0;
}

void GraphicsManager::doFadeOutSync(const TimeValue time, const TimeScale scale, bool isBlack) {
	_updatesEnabled = false;
	_screenFader->doFadeOutSync(time, scale, isBlack);
}

void GraphicsManager::doFadeInSync(const TimeValue time, const TimeScale scale, bool isBlack) {
	_screenFader->doFadeInSync(time, scale, isBlack);
	_updatesEnabled = true;
}

void GraphicsManager::markCursorAsDirty() {
	_modifiedScreen = true;
}

void GraphicsManager::newShakePoint(int32 index1, int32 index2, int32 maxRadius) {
	int32 index3 = (index1 + index2) >> 1;

	if (maxRadius == 0) {
		_shakeOffsets[index3].x = ((_shakeOffsets[index1].x + _shakeOffsets[index2].x) >> 1);
		_shakeOffsets[index3].y = ((_shakeOffsets[index1].y + _shakeOffsets[index2].y) >> 1);
	} else {
		double angle = (int32)(_vm->getRandomNumber(360 - 1) * 3.1415926535 / 180);
		int32 radius = maxRadius;
		_shakeOffsets[index3].x = (int32)(((_shakeOffsets[index1].x + _shakeOffsets[index2].x) >> 1) +
				cos(angle) / 2 * radius);
		_shakeOffsets[index3].y = (int32)(((_shakeOffsets[index1].y + _shakeOffsets[index2].y) >> 1) +
				sin(angle) * radius);
	}

	if (index1 < index3 - 1)
		newShakePoint(index1, index3, maxRadius * 2 / 3);

	if (index3 < index2 - 1)
		newShakePoint(index3, index2, maxRadius * 2 / 3);
}

void GraphicsManager::shakeTheWorld(TimeValue duration, TimeScale scale) {
	if (duration == 0 || scale == 0)
		return;

	_shakeOffsets[0].x = 0;
	_shakeOffsets[0].y = 0;
	_shakeOffsets[(kMaxShakeOffsets - 1) / 4].x = 0;
	_shakeOffsets[(kMaxShakeOffsets - 1) / 4].y = 0;
	_shakeOffsets[(kMaxShakeOffsets - 1) / 2].x = 0;
	_shakeOffsets[(kMaxShakeOffsets - 1) / 2].y = 0;
	_shakeOffsets[(kMaxShakeOffsets - 1) * 3 / 4].x = 0;
	_shakeOffsets[(kMaxShakeOffsets - 1) * 3 / 4].y = 0;
	_shakeOffsets[kMaxShakeOffsets - 1].x = 0;
	_shakeOffsets[kMaxShakeOffsets - 1].y = 0;

	newShakePoint(0, (kMaxShakeOffsets - 1) / 4, 8);
	newShakePoint((kMaxShakeOffsets - 1) / 4, (kMaxShakeOffsets - 1) / 2, 6);
	newShakePoint((kMaxShakeOffsets - 1) / 2, (kMaxShakeOffsets - 1) * 3 / 4, 4);
	newShakePoint((kMaxShakeOffsets - 1) * 3 / 4, kMaxShakeOffsets - 1, 3);

	Common::Point lastOffset(0, 0);

	// Store the current screen for later use
	Graphics::Surface oldScreen;
	Graphics::Surface *curScreen = g_system->lockScreen();
	oldScreen.copyFrom(*curScreen);
	g_system->unlockScreen();

	// Convert to millis
	duration = duration * 1000 / scale;

	uint32 startTime = g_system->getMillis();

	while (g_system->getMillis() < startTime + duration) {
		Common::Point thisOffset = _shakeOffsets[(g_system->getMillis() - startTime) * (kMaxShakeOffsets - 1) / duration];
		if (thisOffset != lastOffset) {
			// Fill the screen with black
			Graphics::Surface *screen = g_system->lockScreen();
			screen->fillRect(Common::Rect(0, 0, 640, 480), g_system->getScreenFormat().RGBToColor(0, 0, 0));
			g_system->unlockScreen();

			// Calculate the src/dst offsets and the width/height
			int32 srcOffsetX, dstOffsetX, width;

			if (thisOffset.x > 0) {
				srcOffsetX = 0;
				dstOffsetX = thisOffset.x;
				width = 640 - dstOffsetX;
			} else {
				srcOffsetX = -thisOffset.x;
				dstOffsetX = 0;
				width = 640 - srcOffsetX;
			}

			int32 srcOffsetY, dstOffsetY, height;

			if (thisOffset.y > 0) {
				srcOffsetY = 0;
				dstOffsetY = thisOffset.y;
				height = 480 - dstOffsetY;
			} else {
				srcOffsetY = -thisOffset.y;
				dstOffsetY = 0;
				height = 480 - srcOffsetY;
			}

			// Now copy to the screen
			g_system->copyRectToScreen((byte *)oldScreen.getBasePtr(srcOffsetX, srcOffsetY), oldScreen.pitch,
					dstOffsetX, dstOffsetY, width, height);
			g_system->updateScreen();

			lastOffset = thisOffset;
		}

		g_system->delayMillis(10);
	}

	if (lastOffset.x != 0 || lastOffset.y != 0) {
		g_system->copyRectToScreen((byte *)oldScreen.pixels, oldScreen.pitch, 0, 0, 640, 480);
		g_system->updateScreen();
	}

	oldScreen.free();
}

void GraphicsManager::enableErase() {
	_erase = true;
}

void GraphicsManager::disableErase() {
	_erase = false;
}

void GraphicsManager::enableUpdates() {
	_updatesEnabled = true;
	_screenFader->setFaderValue(100);
}

void GraphicsManager::disableUpdates() {
	_updatesEnabled = false;
	_screenFader->setFaderValue(0);
}

} // End of namespace Pegasus
