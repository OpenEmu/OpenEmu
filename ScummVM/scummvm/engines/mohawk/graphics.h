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

#ifndef MOHAWK_GRAPHICS_H
#define MOHAWK_GRAPHICS_H

#include "mohawk/bitmap.h"

#include "common/hashmap.h"
#include "common/rect.h"

namespace Graphics {
struct Surface;
}

namespace Mohawk {

class MohawkEngine;
class MohawkEngine_LivingBooks;
class MohawkBitmap;

class MohawkSurface {
public:
	MohawkSurface();
	MohawkSurface(Graphics::Surface *surface, byte *palette = NULL, int offsetX = 0, int offsetY = 0);
	~MohawkSurface();

	// getSurface() returns the surface in the current format
	// This will be the initial format unless convertToTrueColor() is called
	Graphics::Surface *getSurface() const { return _surface; }
	byte *getPalette() const { return _palette; }

	// Convert the 8bpp image to the current screen format
	// Does nothing if _surface is already >8bpp
	void convertToTrueColor();

	// Functions for OldMohawkBitmap offsets
	// They both default to 0
	int getOffsetX() const { return _offsetX; }
	int getOffsetY() const { return _offsetY; }
	void setOffsetX(int x) { _offsetX = x; }
	void setOffsetY(int y) { _offsetY = y; }

private:
	Graphics::Surface *_surface;
	byte *_palette;
	int _offsetX, _offsetY;
};

class GraphicsManager {
public:
	GraphicsManager();
	virtual ~GraphicsManager();

	// Free all surfaces in the cache
	void clearCache();

	void preloadImage(uint16 image);
	virtual void setPalette(uint16 id);
	void copyAnimImageToScreen(uint16 image, int left = 0, int top = 0);
	void copyAnimImageSectionToScreen(uint16 image, Common::Rect src, Common::Rect dest);
	void copyAnimSubImageToScreen(uint16 image, uint16 subimage, int left = 0, int top = 0);

	void getSubImageSize(uint16 image, uint16 subimage, uint16 &width, uint16 &height);

protected:
	void copyAnimImageSectionToScreen(MohawkSurface *image, Common::Rect src, Common::Rect dest);

	// findImage will search the cache to find the image.
	// If not found, it will call decodeImage to get a new one.
	MohawkSurface *findImage(uint16 id);

	// decodeImage will always return a new image.
	virtual MohawkSurface *decodeImage(uint16 id) = 0;
	virtual Common::Array<MohawkSurface *> decodeImages(uint16 id);

	virtual MohawkEngine *getVM() = 0;
	void addImageToCache(uint16 id, MohawkSurface *surface);

private:
	// An image cache that stores images until clearCache() is called
	Common::HashMap<uint16, MohawkSurface *> _cache;
	Common::HashMap<uint16, Common::Array<MohawkSurface *> > _subImageCache;
};

} // End of namespace Mohawk

#endif
