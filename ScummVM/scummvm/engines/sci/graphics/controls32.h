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

#ifndef SCI_GRAPHICS_CONTROLS32_H
#define SCI_GRAPHICS_CONTROLS32_H

namespace Sci {

class GfxCache;
class GfxScreen;
class GfxText32;

/**
 * Controls class, handles drawing of controls in SCI32 (SCI2, SCI2.1, SCI3) games
 */
class GfxControls32 {
public:
	GfxControls32(SegManager *segMan, GfxCache *cache, GfxScreen *screen, GfxText32 *text);
	~GfxControls32();

	void kernelTexteditChange(reg_t controlObject);

private:
	SegManager *_segMan;
	GfxCache *_cache;
	GfxScreen *_screen;
	GfxText32 *_text;
};

} // End of namespace Sci

#endif
