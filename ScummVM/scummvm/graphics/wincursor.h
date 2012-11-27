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

#ifndef GRAPHICS_WINCURSOR_H
#define GRAPHICS_WINCURSOR_H

#include "common/array.h"
#include "common/winexe.h"

#include "graphics/cursor.h"

namespace Common {
class NEResources;
class PEResources;
class SeekableReadStream;
}

namespace Graphics {

/**
 * A structure holding an array of cursors from a single Windows Executable cursor group.
 *
 * Windows lumps different versions of the same cursors/icons together and decides which one
 * to use based on the screen's color depth and resolution. For instance, one cursor group
 * could hold a 1bpp 16x16 cursorand a 8bpp 16x16 cursor. This will hold all cursors in the
 * group. This class should be used to actually parse the cursors, whereas WinCursor is just
 * the representation used by this struct to store the cursors.
 */
struct WinCursorGroup {
	WinCursorGroup();
	~WinCursorGroup();

	struct CursorItem {
		Common::WinResourceID id;
		Cursor *cursor;
	};

	Common::Array<CursorItem> cursors;

	/** Create a cursor group from an NE EXE, returns 0 on failure */
	static WinCursorGroup *createCursorGroup(Common::NEResources &exe, const Common::WinResourceID &id);
	/** Create a cursor group from an PE EXE, returns 0 on failure */
	static WinCursorGroup *createCursorGroup(Common::PEResources &exe, const Common::WinResourceID &id);
};

/**
 * Create a Cursor for the default Windows cursor.
 *
 * @note The calling code is responsible for deleting the returned pointer.
 */
Cursor *makeDefaultWinCursor();

} // End of namespace Graphics

#endif
