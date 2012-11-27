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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_MOUSE_H
#define HUGO_MOUSE_H
namespace Hugo {

class MouseHandler {
public:
	MouseHandler(HugoEngine *vm);
	void mouseHandler();

	void resetLeftButton();
	void resetRightButton();
	void setLeftButton();
	void setRightButton();
	void setJumpExitFl(bool fl);
	void setMouseX(int x);
	void setMouseY(int y);
	void freeHotspots();

	bool getJumpExitFl() const;
	int  getMouseX() const;
	int  getMouseY() const;

	int16 getDirection(const int16 hotspotId) const;
	int16 getHotspotActIndex(const int16 hotspotId) const;

	void  drawHotspots() const;
	int16 findExit(const int16 cx, const int16 cy, byte screenId);
	void  loadHotspots(Common::ReadStream &in);

private:
	HugoEngine *_vm;

	static const char kCursorNochar = '~';              // Don't show name of object under cursor
	static const int  kExitHotspot = -4;                // Cursor over Exit hotspot
	static const int  kCursorNameIndex = 2;             // Index of name used under cursor
	static const int  kCursorNameOffX = 10;             // Cursor offset to name string
	static const int  kCursorNameOffY = -2;             // Cursor offset to name string

	enum seqTextMouse {
		kMsNoWayText = 0,
		kMsExit      = 1
	};

	Hotspot *_hotspots;
	bool  _leftButtonFl;                                // Left mouse button pressed
	bool  _rightButtonFl;                               // Right button pressed
	int   _mouseX;
	int   _mouseY;
	bool  _jumpExitFl;                                  // Allowed to jump to a screen exit

	void  cursorText(const char *buffer, const int16 cx, const int16 cy, const Uif fontId, const int16 color);
	void  processRightClick(const int16 objId, const int16 cx, const int16 cy);
	void  processLeftClick(const int16 objId, const int16 cx, const int16 cy);
	void  readHotspot(Common::ReadStream &in, Hotspot &hotspot);
};

} // End of namespace Hugo

#endif //HUGO_MOUSE_H
