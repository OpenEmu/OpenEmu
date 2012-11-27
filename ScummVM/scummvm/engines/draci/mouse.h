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

#ifndef DRACI_MOUSE_H
#define DRACI_MOUSE_H

#include "common/events.h"

namespace Draci {

enum CursorType {
	kNormalCursor,
	kArrowCursor1,
	kArrowCursor2,
	kArrowCursor3,
	kArrowCursor4,
	kDialogueCursor,
	kHighlightedCursor,
	kMainMenuCursor,
	kUninitializedCursor = 100,
	kItemCursor			// + the index in the BArchive
};

class DraciEngine;
class GameItem;

class Mouse {
public:
	Mouse(DraciEngine *vm);
	~Mouse() {}

	void handleEvent(Common::Event event);
	void cursorOn();
	void cursorOff();
	bool isCursorOn() const;
	void setPosition(uint16 x, uint16 y);
	CursorType getCursorType() const { return _cursorType; }
	void setCursorType(CursorType cur);
	void loadItemCursor(const GameItem *item, bool highlighted);
	bool lButtonPressed() const { return _lButton; }
	bool rButtonPressed() const { return _rButton; }
	void lButtonSet(bool state) { _lButton = state; }
	void rButtonSet(bool state) { _rButton = state; }

	uint16 getPosX() const { return _x; }
	uint16 getPosY() const { return _y; }

private:
	uint16 _x, _y;
	bool _lButton, _rButton;
	CursorType _cursorType;
	DraciEngine *_vm;
};

} // End of namespace Draci

#endif // DRACI_MOUSE_H
