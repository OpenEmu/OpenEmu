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

#ifndef CRUISE_MOUSE_H
#define CRUISE_MOUSE_H

namespace Cruise {

extern int16 main10;

enum CursorType {
	CURSOR_NORMAL = 0,
	CURSOR_DISK = 1,
	CURSOR_CROSS = 2,
	CURSOR_NOMOUSE = 3,
	CURSOR_WALK = 4,
	CURSOR_EXIT = 5,
	CURSOR_MAGNIFYING_GLASS = 6,

	CURSOR_MAX = 7
};

void changeCursor(CursorType eType);
bool isMouseOn();
void mouseOff();
void mouseOn();

extern CursorType currentCursor;

} // End of namespace Cruise

#endif
