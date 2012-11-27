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

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/event.h"
#include "sci/graphics/maciconbar.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"

#include "common/memstream.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "graphics/decoders/pict.h"

namespace Sci {

GfxMacIconBar::GfxMacIconBar() {
	_lastX = 0;

	if (g_sci->getGameId() == GID_FREDDYPHARKAS)
		_inventoryIndex = 5;
	else
		_inventoryIndex = 4;

	_inventoryIcon = 0;
	_allDisabled = true;
}

GfxMacIconBar::~GfxMacIconBar() {
	if (_inventoryIcon) {
		_inventoryIcon->free();
		delete _inventoryIcon;
	}

	for (uint32 i = 0; i < _iconBarItems.size(); i++) {
		if (_iconBarItems[i].nonSelectedImage) {
			_iconBarItems[i].nonSelectedImage->free();
			delete _iconBarItems[i].nonSelectedImage;
		}

		if (_iconBarItems[i].selectedImage) {
			_iconBarItems[i].selectedImage->free();
			delete _iconBarItems[i].selectedImage;
		}
	}
}

void GfxMacIconBar::addIcon(reg_t obj) {
	IconBarItem item;
	uint32 iconIndex = readSelectorValue(g_sci->getEngineState()->_segMan, obj, SELECTOR(iconIndex));

	item.object = obj;
	item.nonSelectedImage = createImage(iconIndex, false);

	if (iconIndex != _inventoryIndex)
		item.selectedImage = createImage(iconIndex, true);
	else
		item.selectedImage = 0;

	item.enabled = true;

	// Start after the main viewing window and add a two pixel buffer
	uint16 y = g_sci->_gfxScreen->getHeight() + 2;

	if (item.nonSelectedImage)
		item.rect = Common::Rect(_lastX, y, MIN<uint32>(_lastX + item.nonSelectedImage->w, 320), y + item.nonSelectedImage->h);
	else
		error("Could not find a non-selected image for icon %d", iconIndex);

	_lastX += item.rect.width();

	_iconBarItems.push_back(item);
}

void GfxMacIconBar::drawIcons() {
	// Draw the icons to the bottom of the screen

	for (uint32 i = 0; i < _iconBarItems.size(); i++)
		drawIcon(i, false);
}

void GfxMacIconBar::drawIcon(uint16 iconIndex, bool selected) {
	if (iconIndex >= _iconBarItems.size())
		return;

	Common::Rect rect = _iconBarItems[iconIndex].rect;

	if (isIconEnabled(iconIndex)) {
		if (selected)
			drawEnabledImage(_iconBarItems[iconIndex].selectedImage, rect);
		else
			drawEnabledImage(_iconBarItems[iconIndex].nonSelectedImage, rect);
	} else
		drawDisabledImage(_iconBarItems[iconIndex].nonSelectedImage, rect);

	if ((iconIndex == _inventoryIndex) && _inventoryIcon) {
		Common::Rect invRect = Common::Rect(0, 0, _inventoryIcon->w, _inventoryIcon->h);
		invRect.moveTo(rect.left, rect.top);
		invRect.translate((rect.width() - invRect.width()) / 2, (rect.height() - invRect.height()) / 2);

		if (isIconEnabled(iconIndex))
			drawEnabledImage(_inventoryIcon, invRect);
		else
			drawDisabledImage(_inventoryIcon, invRect);
	}
}

void GfxMacIconBar::drawEnabledImage(Graphics::Surface *surface, const Common::Rect &rect) {
	if (surface)
		g_system->copyRectToScreen(surface->pixels, surface->pitch, rect.left, rect.top, rect.width(), rect.height());
}

void GfxMacIconBar::drawDisabledImage(Graphics::Surface *surface, const Common::Rect &rect) {
	if (!surface)
		return;

	// Add a black checkboard pattern to the image before copying it to the screen

	Graphics::Surface newSurf;
	newSurf.copyFrom(*surface);

	for (int i = 0; i < newSurf.h; i++) {
		// Start at the next four byte boundary
		int startX = 3 - ((rect.left + 3) & 3);

		// Start odd rows at two bytes past that (also properly aligned)
		if ((i + rect.top) & 1)
			startX = (startX + 2) & 3;

		for (int j = startX; j < newSurf.w; j += 4)
			*((byte *)newSurf.getBasePtr(j, i)) = 0;
	}

	g_system->copyRectToScreen(newSurf.pixels, newSurf.pitch, rect.left, rect.top, rect.width(), rect.height());
	newSurf.free();
}

void GfxMacIconBar::drawSelectedImage(uint16 iconIndex) {
	assert(iconIndex <= _iconBarItems.size());

	drawEnabledImage(_iconBarItems[iconIndex].selectedImage, _iconBarItems[iconIndex].rect);
}

bool GfxMacIconBar::isIconEnabled(uint16 iconIndex) const {
	if (iconIndex >= _iconBarItems.size())
		return false;

	return !_allDisabled && _iconBarItems[iconIndex].enabled;
}

void GfxMacIconBar::setIconEnabled(int16 iconIndex, bool enabled) {
	if (iconIndex < 0)
		_allDisabled = !enabled;
	else if (iconIndex < (int)_iconBarItems.size()) {
		_iconBarItems[iconIndex].enabled = enabled;
	}
}

void GfxMacIconBar::setInventoryIcon(int16 icon) {
	Graphics::Surface *surface = 0;

	if (icon >= 0)
		surface = loadPict(ResourceId(kResourceTypeMacPict, icon));

	if (_inventoryIcon) {
		// Free old inventory icon if we're removing the inventory icon
		// or setting a new one.
		if ((icon < 0) || surface) {
			_inventoryIcon->free();
			delete _inventoryIcon;
			_inventoryIcon = 0;
		}
	}

	if (surface)
		_inventoryIcon = surface;

	drawIcon(_inventoryIndex, false);
}

Graphics::Surface *GfxMacIconBar::loadPict(ResourceId id) {
	Graphics::PICTDecoder pictDecoder;
	Resource *res = g_sci->getResMan()->findResource(id, false);

	if (!res || res->size == 0)
		return 0;

	Common::MemoryReadStream stream(res->data, res->size);
	if (!pictDecoder.loadStream(stream))
		return 0;

	Graphics::Surface *surface = new Graphics::Surface();
	surface->copyFrom(*pictDecoder.getSurface());
	remapColors(surface, pictDecoder.getPalette());

	return surface;
}

Graphics::Surface *GfxMacIconBar::createImage(uint32 iconIndex, bool isSelected) {
	ResourceType type = isSelected ? kResourceTypeMacIconBarPictS : kResourceTypeMacIconBarPictN;
	return loadPict(ResourceId(type, iconIndex + 1));
}

void GfxMacIconBar::remapColors(Graphics::Surface *surf, const byte *palette) {
	byte *pixels = (byte *)surf->pixels;

	// Remap to the screen palette
	for (uint16 i = 0; i < surf->w * surf->h; i++) {
		byte color = *pixels;

		byte r = palette[color * 3];
		byte g = palette[color * 3 + 1];
		byte b = palette[color * 3 + 2];

		*pixels++ = g_sci->_gfxPalette->findMacIconBarColor(r, g, b);
	}
}

bool GfxMacIconBar::pointOnIcon(uint32 iconIndex, Common::Point point) {
	return _iconBarItems[iconIndex].rect.contains(point);
}

reg_t GfxMacIconBar::handleEvents() {
	// Peek event queue for a mouse button press
	EventManager *evtMgr = g_sci->getEventManager();
	SciEvent evt = evtMgr->getSciEvent(SCI_EVENT_MOUSE_PRESS | SCI_EVENT_PEEK);

	// No mouse press found
	if (evt.type == SCI_EVENT_NONE)
		return NULL_REG;

	// If the mouse is not over the icon bar, return
	if (evt.mousePos.y < g_sci->_gfxScreen->getHeight())
		return NULL_REG;

	// Remove event from queue
	evtMgr->getSciEvent(SCI_EVENT_MOUSE_PRESS);

	// Mouse press on the icon bar, check the icon rectangles
	uint iconNr;
	for (iconNr = 0; iconNr < _iconBarItems.size(); iconNr++) {
		if (pointOnIcon(iconNr, evt.mousePos) && isIconEnabled(iconNr))
			break;
	}

	// Mouse press not on an icon
	if (iconNr == _iconBarItems.size())
		return NULL_REG;

	drawIcon(iconNr, true);
	bool isSelected = true;

	// Wait for mouse release
	while (evt.type != SCI_EVENT_MOUSE_RELEASE) {
		// Mimic behavior of SSCI when moving mouse with button held down
		if (isSelected != pointOnIcon(iconNr, evt.mousePos)) {
			isSelected = !isSelected;
			drawIcon(iconNr, isSelected);
		}

		evt = evtMgr->getSciEvent(SCI_EVENT_MOUSE_RELEASE);
		g_system->delayMillis(10);
	}

	drawIcon(iconNr, false);

	// If user moved away from the icon, we do nothing
	if (pointOnIcon(iconNr, evt.mousePos))
		return _iconBarItems[iconNr].object;

	return NULL_REG;
}

} // End of namespace Sci
