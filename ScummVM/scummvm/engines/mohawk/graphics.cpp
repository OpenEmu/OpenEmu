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

#include "mohawk/mohawk.h"
#include "mohawk/resource.h"
#include "mohawk/graphics.h"

#include "common/system.h"
#include "engines/util.h"
#include "graphics/palette.h"

namespace Mohawk {

MohawkSurface::MohawkSurface() : _surface(0), _palette(0) {
	_offsetX = 0;
	_offsetY = 0;
}

MohawkSurface::MohawkSurface(Graphics::Surface *surface, byte *palette, int offsetX, int offsetY) : _palette(palette), _offsetX(offsetX), _offsetY(offsetY) {
	assert(surface);

	_surface = surface;
}

MohawkSurface::~MohawkSurface() {
	free(_palette);

	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

void MohawkSurface::convertToTrueColor() {
	assert(_surface);

	if (_surface->format.bytesPerPixel > 1)
		return;

	assert(_palette);

	Graphics::Surface *surface = _surface->convertTo(g_system->getScreenFormat(), _palette);

	// Free everything and set the new surface as the converted surface
	_surface->free();
	delete _surface;
	free(_palette);
	_palette = 0;
	_surface = surface;
}

GraphicsManager::GraphicsManager() {
}

GraphicsManager::~GraphicsManager() {
	clearCache();
}

void GraphicsManager::clearCache() {
	for (Common::HashMap<uint16, MohawkSurface *>::iterator it = _cache.begin(); it != _cache.end(); it++)
		delete it->_value;
	for (Common::HashMap<uint16, Common::Array<MohawkSurface *> >::iterator it = _subImageCache.begin(); it != _subImageCache.end(); it++) {
		Common::Array<MohawkSurface *> &array = it->_value;
		for (uint i = 0; i < array.size(); i++)
			delete array[i];
	}

	_cache.clear();
	_subImageCache.clear();
}

MohawkSurface *GraphicsManager::findImage(uint16 id) {
	if (!_cache.contains(id))
		_cache[id] = decodeImage(id);

	// TODO: Probably would be nice to limit the size of the cache
	// Currently, this can't get large because it is freed on every
	// card/stack change in Myst/Riven so I'm not worried about it.
	// Doesn't mean this shouldn't be done in the future.

	return _cache[id];
}

Common::Array<MohawkSurface *> GraphicsManager::decodeImages(uint16 id) {
	error("decodeImages not implemented for this game");
}

void GraphicsManager::preloadImage(uint16 image) {
	findImage(image);
}

void GraphicsManager::setPalette(uint16 id) {
	Common::SeekableReadStream *tpalStream = getVM()->getResource(ID_TPAL, id);

	uint16 colorStart = tpalStream->readUint16BE();
	uint16 colorCount = tpalStream->readUint16BE();
	byte *palette = new byte[colorCount * 3];

	for (uint16 i = 0; i < colorCount; i++) {
		palette[i * 3 + 0] = tpalStream->readByte();
		palette[i * 3 + 1] = tpalStream->readByte();
		palette[i * 3 + 2] = tpalStream->readByte();
		tpalStream->readByte();
	}

	delete tpalStream;

	getVM()->_system->getPaletteManager()->setPalette(palette, colorStart, colorCount);
	delete[] palette;
}

void GraphicsManager::copyAnimImageToScreen(uint16 image, int left, int top) {
	Graphics::Surface *surface = findImage(image)->getSurface();

	Common::Rect srcRect(0, 0, surface->w, surface->h);
	Common::Rect dstRect(left, top, left + surface->w, top + surface->h);
	copyAnimImageSectionToScreen(image, srcRect, dstRect);
}

void GraphicsManager::copyAnimImageSectionToScreen(uint16 image, Common::Rect srcRect, Common::Rect dstRect) {
	copyAnimImageSectionToScreen(findImage(image), srcRect, dstRect);
}

void GraphicsManager::copyAnimSubImageToScreen(uint16 image, uint16 subimage, int left, int top) {
	if (!_subImageCache.contains(image))
		_subImageCache[image] = decodeImages(image);
	Common::Array<MohawkSurface *> &images = _subImageCache[image];

	Graphics::Surface *surface = images[subimage]->getSurface();

	Common::Rect srcRect(0, 0, surface->w, surface->h);
	Common::Rect dstRect(left, top, left + surface->w, top + surface->h);
	copyAnimImageSectionToScreen(images[subimage], srcRect, dstRect);
}

void GraphicsManager::getSubImageSize(uint16 image, uint16 subimage, uint16 &width, uint16 &height) {
	if (!_subImageCache.contains(image))
		_subImageCache[image] = decodeImages(image);
	Common::Array<MohawkSurface *> &images = _subImageCache[image];

	Graphics::Surface *surface = images[subimage]->getSurface();
	width = surface->w;
	height = surface->h;
}

void GraphicsManager::copyAnimImageSectionToScreen(MohawkSurface *image, Common::Rect srcRect, Common::Rect dstRect) {
	uint16 startX = 0;
	uint16 startY = 0;

	assert(srcRect.isValidRect() && dstRect.isValidRect());
	assert(srcRect.left >= 0 && srcRect.top >= 0);

	// TODO: clip rect
	if (dstRect.left < 0) {
		startX -= dstRect.left;
		dstRect.left = 0;
	}

	if (dstRect.top < 0) {
		startY -= dstRect.top;
		dstRect.top = 0;
	}

	if (dstRect.left >= getVM()->_system->getWidth())
		return;
	if (dstRect.top >= getVM()->_system->getHeight())
		return;

	Graphics::Surface *surface = image->getSurface();
	if (startX >= surface->w)
		return;
	if (startY >= surface->h)
		return;

	if (srcRect.left > surface->w)
		return;
	if (srcRect.top > surface->h)
		return;
	if (srcRect.right > surface->w)
		srcRect.right = surface->w;
	if (srcRect.bottom > surface->h)
		srcRect.bottom = surface->h;

	uint16 width = MIN<int>(srcRect.right - srcRect.left - startX, getVM()->_system->getWidth() - dstRect.left);
	uint16 height = MIN<int>(srcRect.bottom - srcRect.top - startY, getVM()->_system->getHeight() - dstRect.top);

	byte *surf = (byte *)surface->getBasePtr(0, srcRect.top + startY);
	Graphics::Surface *screen = getVM()->_system->lockScreen();

	// image and screen should always be 8bpp
	for (uint16 y = 0; y < height; y++) {
		byte *dest = (byte *)screen->getBasePtr(dstRect.left, dstRect.top + y);
		byte *src = surf + srcRect.left + startX;
		// blit, with 0 being transparent
		for (uint16 x = 0; x < width; x++) {
			if (*src)
				*dest = *src;
			src++;
			dest++;
		}
		surf += surface->pitch;
	}

	getVM()->_system->unlockScreen();
}

void GraphicsManager::addImageToCache(uint16 id, MohawkSurface *surface) {
	if (_cache.contains(id))
		error("Image %d already in cache", id);

	_cache[id] = surface;
}

} // End of namespace Mohawk
