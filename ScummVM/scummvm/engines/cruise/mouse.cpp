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

#include "cruise/cruise_main.h"
#include "cruise/staticres.h"

#include "common/system.h"
#include "graphics/cursorman.h"

namespace Cruise {

int16 main10;

struct MouseCursor {
	int hotspotX;
	int hotspotY;
	const byte *bitmap;
};

static const MouseCursor mouseCursors[] = {
	{ 1, 1, mouseCursorNormal },
	{ 0, 0, mouseCursorDisk },
	{ 7, 7, mouseCursorCross },
	{ 0, 0, mouseCursorNoMouse },
	{ 10, 6, mouseCursorWalk },
	{ 10, 6, mouseCursorExit },
	{ 10, 6, mouseCursorMagnifyingGlass }
};

CursorType currentCursor = CURSOR_NOMOUSE;

static const byte cursorPalette[] = {
	0, 0, 0,
	0xff, 0xff, 0xff
};

void changeCursor(CursorType eType) {
	assert(eType >= 0 && eType < CURSOR_MAX);
	if (currentCursor != eType) {
		byte mouseCursor[16 * 16];
		const MouseCursor *mc = &mouseCursors[eType];
		const byte *src = mc->bitmap;
		for (int i = 0; i < 32; ++i) {
			int offs = i * 8;
			for (byte mask = 0x80; mask != 0; mask >>= 1) {
				if (src[0] & mask) {
					mouseCursor[offs] = 1;
				} else if (src[32] & mask) {
					mouseCursor[offs] = 0;
				} else {
					mouseCursor[offs] = 0xFF;
				}
				++offs;
			}
			++src;
		}
		CursorMan.replaceCursor(mouseCursor, 16, 16, mc->hotspotX, mc->hotspotY, 0xFF);
		CursorMan.replaceCursorPalette(cursorPalette, 0, 2);
		currentCursor = eType;
	}
}

bool isMouseOn() {
	return (currentCursor != CURSOR_NOMOUSE) && CursorMan.isVisible();
}

void mouseOff() {
	CursorMan.showMouse(false);
	g_system->updateScreen();
}

void mouseOn() {
	CursorMan.showMouse(true);
	g_system->updateScreen();
}

} // End of namespace Cruise
