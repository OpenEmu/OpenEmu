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

#ifndef SCI_GRAPHICS_CACHE_H
#define SCI_GRAPHICS_CACHE_H

#include "common/hashmap.h"

namespace Sci {

class GfxFont;
class GfxView;

typedef Common::HashMap<int, GfxFont *> FontCache;
typedef Common::HashMap<int, GfxView *> ViewCache;

/**
 * Cache class, handles caching of views/fonts
 */
class GfxCache {
public:
	GfxCache(ResourceManager *resMan, GfxScreen *screen, GfxPalette *palette);
	~GfxCache();

	GfxFont *getFont(GuiResourceId fontId);
	GfxView *getView(GuiResourceId viewId);

	int16 kernelViewGetCelWidth(GuiResourceId viewId, int16 loopNo, int16 celNo);
	int16 kernelViewGetCelHeight(GuiResourceId viewId, int16 loopNo, int16 celNo);
	int16 kernelViewGetLoopCount(GuiResourceId viewId);
	int16 kernelViewGetCelCount(GuiResourceId viewId, int16 loopNo);

	byte kernelViewGetColorAtCoordinate(GuiResourceId viewId, int16 loopNo, int16 celNo, int16 x, int16 y);

private:
	void purgeFontCache();
	void purgeViewCache();

	ResourceManager *_resMan;
	GfxScreen *_screen;
	GfxPalette *_palette;

	FontCache _cachedFonts;
	ViewCache _cachedViews;
};

} // End of namespace Sci

#endif
