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

#ifndef SWORD1_MOUSE_H
#define SWORD1_MOUSE_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "sword1/sworddefs.h"
#include "sword1/object.h"

class OSystem;

namespace Sword1 {

#define MAX_MOUSE 30

#define BS1L_BUTTON_DOWN        2
#define BS1L_BUTTON_UP          4
#define BS1R_BUTTON_DOWN        8
#define BS1R_BUTTON_UP          16
#define BS1_WHEEL_UP            32
#define BS1_WHEEL_DOWN          64
#define MOUSE_BOTH_BUTTONS      (BS1L_BUTTON_DOWN | BS1R_BUTTON_DOWN)
#define MOUSE_DOWN_MASK         (BS1L_BUTTON_DOWN | BS1R_BUTTON_DOWN)
#define MOUSE_UP_MASK           (BS1L_BUTTON_UP | BS1R_BUTTON_UP)

struct MouseObj {
	int id;
	Object *compact;
};

#include "common/pack-start.h"  // START STRUCT PACKING

struct MousePtr {
	uint16 numFrames;
	uint16 sizeX;
	uint16 sizeY;
	uint16 hotSpotX;
	uint16 hotSpotY;
	uint8  dummyData[0x30];
} PACKED_STRUCT;

#include "common/pack-end.h"    // END STRUCT PACKING

class Logic;
class Menu;
class ResMan;
class ObjectMan;

class Mouse {
public:
	Mouse(OSystem *system, ResMan *pResMan, ObjectMan *pObjMan);
	~Mouse();
	void initialize();
	void addToList(int id, Object *compact);
	void useLogicAndMenu(Logic *pLogic, Menu *pMenu);
	void setLuggage(uint32 resID, uint32 rate);
	void setPointer(uint32 resID, uint32 rate);
	void animate();
	void engine(uint16 x, uint16 y, uint16 eventFlags);
	uint16 testEvent();
	void giveCoords(uint16 *x, uint16 *y);
	void fnNoHuman();
	void fnAddHuman();
	void fnBlankMouse();
	void fnNormalMouse();
	void fnLockMouse();
	void fnUnlockMouse();
	void controlPanel(bool on);
private:
	void createPointer(uint32 ptrId, uint32 luggageId);
	OSystem *_system;
	Logic *_logic;
	Menu *_menu;
	MouseObj _objList[MAX_MOUSE];
	ResMan *_resMan;
	ObjectMan *_objMan;
	Common::Point _mouse;

	uint32 _currentPtrId, _currentLuggageId;
	MousePtr *_currentPtr;
	int _frame, _activeFrame;
	uint16 _numObjs;
	uint16 _lastState, _state;
	uint32 _getOff;
	bool _inTopMenu, _mouseOverride;
};

} // End of namespace Sword1

#endif //BSMOUSE_H
