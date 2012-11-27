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

#include "lastexpress/graphics.h"

#include "common/rect.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace LastExpress {

#define COLOR_KEY  0xFFFF

GraphicsManager::GraphicsManager() : _changed(false) {
	const Graphics::PixelFormat format(2, 5, 5, 5, 0, 10, 5, 0, 0);
	_screen.create(640, 480, format);

	// Create the game surfaces
	_backgroundA.create(640, 480, format);
	_backgroundC.create(640, 480, format);
	_overlay.create(640, 480, format);
	_inventory.create(640, 480, format);

	clear(kBackgroundAll);
}

GraphicsManager::~GraphicsManager() {
	// Free the game surfaces
	_screen.free();
	_backgroundA.free();
	_backgroundC.free();
	_overlay.free();
	_inventory.free();
}

void GraphicsManager::update() {
	// Update the screen if needed and reset the status
	if (_changed) {
		mergePlanes();
		updateScreen();
		_changed = false;
	}
}

void GraphicsManager::change() {
	_changed = true;
}

void GraphicsManager::clear(BackgroundType type) {
	clear(type, Common::Rect(640, 480));
}

void GraphicsManager::clear(BackgroundType type, const Common::Rect &rect) {
	switch (type) {
		default:
			error("[GraphicsManager::clear] Unknown background type: %d", type);
			break;

		case kBackgroundA:
		case kBackgroundC:
		case kBackgroundOverlay:
		case kBackgroundInventory:
			getSurface(type)->fillRect(rect, COLOR_KEY);
			break;

		case kBackgroundAll:
			_backgroundA.fillRect(rect, COLOR_KEY);
			_backgroundC.fillRect(rect, COLOR_KEY);
			_overlay.fillRect(rect, COLOR_KEY);
			_inventory.fillRect(rect, COLOR_KEY);
			break;
	}
}

bool GraphicsManager::draw(Drawable *drawable, BackgroundType type, bool transition) {
	// TODO handle transition properly
	if (transition)
		clear(type);

	// TODO store rect for later use
	Common::Rect rect = drawable->draw(getSurface(type));

	return (!rect.isEmpty());
}

Graphics::Surface *GraphicsManager::getSurface(BackgroundType type) {
	switch (type) {
		default:
			error("[GraphicsManager::getSurface] Unknown surface type: %d", type);
			break;

		case kBackgroundA:
			return &_backgroundA;

		case kBackgroundC:
			return &_backgroundC;

		case kBackgroundOverlay:
			return &_overlay;

		case kBackgroundInventory:
			return &_inventory;

		case kBackgroundAll:
			error("[GraphicsManager::getSurface] Cannot return a surface for kBackgroundAll");
			break;
	}
}

// TODO optimize to only merge dirty rects
void GraphicsManager::mergePlanes() {
	// Clear screen surface
	_screen.fillRect(Common::Rect(640, 480), 0);

	uint16 *screen = (uint16 *)_screen.pixels;
	uint16 *inventory = (uint16 *)_inventory.pixels;
	uint16 *overlay = (uint16 *)_overlay.pixels;
	uint16 *backgroundC = (uint16 *)_backgroundC.pixels;
	uint16 *backgroundA = (uint16 *)_backgroundA.pixels;

	for (int i = 0; i < 640 * 480; i++) {

		if (*inventory != COLOR_KEY)
			*screen = *inventory;
		else if (*overlay != COLOR_KEY)
			*screen = *overlay;
		else if (*backgroundA != COLOR_KEY)
			*screen = *backgroundA;
		else if (*backgroundC != COLOR_KEY)
			*screen = *backgroundC;
		else
			*screen = 0;

		inventory++;
		screen++;
		overlay++;
		backgroundA++;
		backgroundC++;
	}
}

void GraphicsManager::updateScreen() {
	g_system->fillScreen(0);
	g_system->copyRectToScreen(_screen.getBasePtr(0, 0), 640 * 2, 0, 0, 640, 480);
}

} // End of namespace LastExpress
