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

#ifndef SCI_GRAPHICS_PAINT32_H
#define SCI_GRAPHICS_PAINT32_H

#include "sci/graphics/paint.h"

namespace Sci {

class GfxPorts;

/**
 * Paint32 class, handles painting/drawing for SCI32 (SCI2+) games
 */
class GfxPaint32 : public GfxPaint {
public:
	GfxPaint32(ResourceManager *resMan, SegManager *segMan, Kernel *kernel, GfxCoordAdjuster *coordAdjuster, GfxCache *cache, GfxScreen *screen, GfxPalette *palette);
	~GfxPaint32();

	void fillRect(Common::Rect rect, byte color);

	void kernelDrawPicture(GuiResourceId pictureId, int16 animationNr, bool animationBlackoutFlag, bool mirroredFlag, bool addToFlag, int16 EGApaletteNo);
	void kernelGraphDrawLine(Common::Point startPoint, Common::Point endPoint, int16 color, int16 priority, int16 control);

private:
	ResourceManager *_resMan;
	SegManager *_segMan;
	Kernel *_kernel;
	GfxCoordAdjuster *_coordAdjuster;
	GfxCache *_cache;
	GfxScreen *_screen;
	GfxPalette *_palette;
};

} // End of namespace Sci

#endif
