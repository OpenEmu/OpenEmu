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

#include "common/system.h"

#include "graphics/cursorman.h"

#include "sword1/mouse.h"
#include "sword1/menu.h"
#include "sword1/screen.h"
#include "sword1/logic.h"
#include "sword1/resman.h"
#include "sword1/objectman.h"
#include "sword1/sworddefs.h"
#include "sword1/swordres.h"
#include "sword1/sword1.h"

namespace Sword1 {

Mouse::Mouse(OSystem *system, ResMan *pResMan, ObjectMan *pObjMan) {
	_resMan = pResMan;
	_objMan = pObjMan;
	_system = system;
	_currentPtr = NULL;
}

Mouse::~Mouse() {
	setLuggage(0, 0);
	setPointer(0, 0);

	for (uint8 cnt = 0; cnt < 17; cnt++)     // close mouse cursor resources
		_resMan->resClose(MSE_POINTER + cnt);
}

void Mouse::initialize() {
	_numObjs = 0;
	Logic::_scriptVars[MOUSE_STATUS] = 0; // mouse off and unlocked
	_getOff = 0;
	_inTopMenu = false;
	_lastState = 0;
	_mouseOverride = false;
	_currentPtrId = _currentLuggageId = 0;

	for (uint8 cnt = 0; cnt < 17; cnt++)     // force res manager to keep mouse
		_resMan->resOpen(MSE_POINTER + cnt); // cursors in memory all the time

	CursorMan.showMouse(false);
	createPointer(0, 0);
}

void Mouse::controlPanel(bool on) { // true on entering cpanel, false when leaving
	static uint32 savedPtrId = 0;
	if (on) {
		savedPtrId = _currentPtrId;
		_mouseOverride = true;
		setLuggage(0, 0);
		setPointer(MSE_POINTER, 0);
	} else {
		_currentPtrId = savedPtrId;
		_mouseOverride = false;
		setLuggage(_currentLuggageId, 0);
		setPointer(_currentPtrId, 0);
	}
}

void Mouse::useLogicAndMenu(Logic *pLogic, Menu *pMenu) {
	_logic = pLogic;
	_menu = pMenu;
}

void Mouse::addToList(int id, Object *compact) {
	_objList[_numObjs].id = id;
	_objList[_numObjs].compact = compact;
	_numObjs++;
}

void Mouse::engine(uint16 x, uint16 y, uint16 eventFlags) {
	_state = 0; // all mouse events are flushed after one cycle.
	if (_lastState) { // delay all events by one cycle to notice L_button + R_button clicks correctly.
		_state = _lastState | eventFlags;
		_lastState = 0;
	} else if (eventFlags)
		_lastState = eventFlags;

	// if we received both, mouse down and mouse up event in this cycle, resort them so that
	// we'll receive the up event in the next one.
	if ((_state & MOUSE_DOWN_MASK) && (_state & MOUSE_UP_MASK)) {
		_lastState = _state & MOUSE_UP_MASK;
		_state &= MOUSE_DOWN_MASK;
	}

	_mouse.x = x;
	_mouse.y = y;
	if (!(Logic::_scriptVars[MOUSE_STATUS] & 1)) {  // no human?
		_numObjs = 0;
		return; // no human, so we don't want the mouse engine
	}

	if (!Logic::_scriptVars[TOP_MENU_DISABLED]) {
		if (y < 40) { // okay, we are in the top menu.
			if (!_inTopMenu) { // are we just entering it?
				if (!Logic::_scriptVars[OBJECT_HELD])
					_menu->fnStartMenu();
				setPointer(MSE_POINTER, 0);
			}
			_menu->checkTopMenu();
			_inTopMenu = true;
		} else if (_inTopMenu) { // we're not in the menu. did we just leave it?
			if (!Logic::_scriptVars[OBJECT_HELD])
				_menu->fnEndMenu();
			_inTopMenu = false;
		}
	} else if (_inTopMenu) {
		_menu->fnEndMenu();
		_inTopMenu = false;
	}

	Logic::_scriptVars[MOUSE_X] = Logic::_scriptVars[SCROLL_OFFSET_X] + x + 128;
	Logic::_scriptVars[MOUSE_Y] = Logic::_scriptVars[SCROLL_OFFSET_Y] + y + 128 - 40;

	//-
	int32 touchedId = 0;
	uint16 clicked = 0;
	if (y > 40) {
		for (uint16 priority = 0; (priority < 10) && (!touchedId); priority++) {
			for (uint16 cnt = 0; (cnt < _numObjs) && (!touchedId); cnt++) {
				if ((_objList[cnt].compact->o_priority == priority) &&
				        (Logic::_scriptVars[MOUSE_X] >= (uint32)_objList[cnt].compact->o_mouse_x1) &&
				        (Logic::_scriptVars[MOUSE_X] <= (uint32)_objList[cnt].compact->o_mouse_x2) &&
				        (Logic::_scriptVars[MOUSE_Y] >= (uint32)_objList[cnt].compact->o_mouse_y1) &&
				        (Logic::_scriptVars[MOUSE_Y] <= (uint32)_objList[cnt].compact->o_mouse_y2)) {
					touchedId = _objList[cnt].id;
					clicked = cnt;
				}
			}
		}
		if (touchedId != (int)Logic::_scriptVars[SPECIAL_ITEM]) { //the mouse collision situation has changed in one way or another
			Logic::_scriptVars[SPECIAL_ITEM] = touchedId;
			if (_getOff) { // there was something else selected before, run its get-off script
				_logic->runMouseScript(NULL, _getOff);
				_getOff = 0;
			}
			if (touchedId) { // there's something new selected, now.
				if (_objList[clicked].compact->o_mouse_on)  //run its get on
					_logic->runMouseScript(_objList[clicked].compact, _objList[clicked].compact->o_mouse_on);

				_getOff = _objList[clicked].compact->o_mouse_off; //setup get-off for later
			}
		}
	} else
		Logic::_scriptVars[SPECIAL_ITEM] = 0;
	if (_state & MOUSE_DOWN_MASK) {
		if (_inTopMenu) {
			if (Logic::_scriptVars[SECOND_ITEM])
				_logic->runMouseScript(NULL, _menu->_objectDefs[Logic::_scriptVars[SECOND_ITEM]].useScript);
			if (Logic::_scriptVars[MENU_LOOKING])
				_logic->cfnPresetScript(NULL, -1, PLAYER, SCR_menu_look, 0, 0, 0, 0);
		}

		Logic::_scriptVars[MOUSE_BUTTON] = _state & MOUSE_DOWN_MASK;
		if (Logic::_scriptVars[SPECIAL_ITEM]) {
			Object *compact = _objMan->fetchObject(Logic::_scriptVars[SPECIAL_ITEM]);
			_logic->runMouseScript(compact, compact->o_mouse_click);
		}
	}
	_numObjs = 0;
}

uint16 Mouse::testEvent() {
	return _state;
}

void Mouse::createPointer(uint32 ptrId, uint32 luggageId) {
	if (_currentPtr) {
		free(_currentPtr);
		_currentPtr = NULL;
	}

	if (ptrId) {
		MousePtr *lugg = NULL;
		MousePtr *ptr = (MousePtr *)_resMan->openFetchRes(ptrId);
		uint16 noFrames = _resMan->getLEUint16(ptr->numFrames);
		uint16 ptrSizeX = _resMan->getLEUint16(ptr->sizeX);
		uint16 ptrSizeY = _resMan->getLEUint16(ptr->sizeY);
		uint16 luggSizeX = 0;
		uint16 luggSizeY = 0;
		uint16 resSizeX;
		uint16 resSizeY;

		if (SwordEngine::isPsx()) //PSX pointers are half height
			ptrSizeY *= 2;

		if (luggageId) {
			lugg = (MousePtr *)_resMan->openFetchRes(luggageId);
			luggSizeX = _resMan->getLEUint16(lugg->sizeX);
			luggSizeY = _resMan->getLEUint16(lugg->sizeY);

			if (SwordEngine::isPsx())
				luggSizeY *= 2;

			resSizeX = MAX(ptrSizeX, (uint16)((ptrSizeX / 2) + luggSizeX));
			resSizeY = MAX(ptrSizeY, (uint16)((ptrSizeY / 2) + luggSizeY));
		} else {
			resSizeX = ptrSizeX;
			resSizeY = ptrSizeY;
		}
		_currentPtr = (MousePtr *)malloc(sizeof(MousePtr) + resSizeX * resSizeY * noFrames);
		_currentPtr->hotSpotX = _resMan->getLEUint16(ptr->hotSpotX);
		_currentPtr->hotSpotY = _resMan->getLEUint16(ptr->hotSpotY);
		_currentPtr->numFrames = noFrames;
		_currentPtr->sizeX = resSizeX;
		_currentPtr->sizeY = resSizeY;
		uint8 *ptrData = (uint8 *)_currentPtr + sizeof(MousePtr);
		memset(ptrData, 255, resSizeX * resSizeY * noFrames);
		if (luggageId) {
			uint8 *dstData = ptrData + resSizeX - luggSizeX;
			for (uint32 frameCnt = 0; frameCnt < noFrames; frameCnt++) {
				uint8 *luggSrc = (uint8 *)lugg + sizeof(MousePtr);
				dstData += (resSizeY - luggSizeY) * resSizeX;
				for (uint32 cnty = 0; cnty < (uint32)(SwordEngine::isPsx() ? luggSizeY / 2 : luggSizeY); cnty++) {
					for (uint32 cntx = 0; cntx < luggSizeX; cntx++)
						if (luggSrc[cntx])
							dstData[cntx] = luggSrc[cntx];

					if (SwordEngine::isPsx()) {
						dstData += resSizeX;
						for (uint32 cntx = 0; cntx < luggSizeX; cntx++)
							if (luggSrc[cntx])
								dstData[cntx] = luggSrc[cntx];
					}

					dstData += resSizeX;
					luggSrc += luggSizeX;
				}
			}
			_resMan->resClose(luggageId);
		}

		uint8 *dstData = ptrData;
		uint8 *srcData = (uint8 *)ptr + sizeof(MousePtr);
		for (uint32 frameCnt = 0; frameCnt < noFrames; frameCnt++) {
			for (uint32 cnty = 0; cnty < (uint32)(SwordEngine::isPsx() ? ptrSizeY / 2 : ptrSizeY); cnty++) {
				for (uint32 cntx = 0; cntx < ptrSizeX; cntx++)
					if (srcData[cntx])
						dstData[cntx] = srcData[cntx];

				if (SwordEngine::isPsx()) {
					dstData += resSizeX;
					for (uint32 cntx = 0; cntx < ptrSizeX; cntx++)
						if (srcData[cntx])
							dstData[cntx] = srcData[cntx];
				}

				srcData += ptrSizeX;
				dstData += resSizeX;
			}
			dstData += (resSizeY - ptrSizeY) * resSizeX;
		}
		_resMan->resClose(ptrId);
	}
}

void Mouse::setPointer(uint32 resId, uint32 rate) {
	_currentPtrId = resId;
	_frame = 0;
	_activeFrame = -1;

	createPointer(resId, _currentLuggageId);

	if ((resId == 0) || (!(Logic::_scriptVars[MOUSE_STATUS] & 1) && (!_mouseOverride))) {
		CursorMan.showMouse(false);
	} else {
		animate();
		CursorMan.showMouse(true);
	}
}

void Mouse::setLuggage(uint32 resId, uint32 rate) {
	_currentLuggageId = resId;
	_frame = 0;
	_activeFrame = -1;

	createPointer(_currentPtrId, resId);
}

void Mouse::animate() {
	if ((Logic::_scriptVars[MOUSE_STATUS] == 1) || (_mouseOverride && _currentPtr)) {
		_frame = (_frame + 1) % _currentPtr->numFrames;

		if (_activeFrame == _frame)
			return;

		uint8 *ptrData = (uint8 *)_currentPtr + sizeof(MousePtr);
		ptrData += _frame * _currentPtr->sizeX * _currentPtr->sizeY;

		CursorMan.replaceCursor(ptrData, _currentPtr->sizeX, _currentPtr->sizeY, _currentPtr->hotSpotX, _currentPtr->hotSpotY, 255);

		_activeFrame = _frame;
	}
}

void Mouse::fnNoHuman() {
	if (Logic::_scriptVars[MOUSE_STATUS] & 2) // locked, can't do anything
		return;
	Logic::_scriptVars[MOUSE_STATUS] = 0; // off & unlocked
	setLuggage(0, 0);
	setPointer(0, 0);
}

void Mouse::fnAddHuman() {
	if (Logic::_scriptVars[MOUSE_STATUS] & 2) // locked, can't do anything
		return;
	Logic::_scriptVars[MOUSE_STATUS] = 1;
	Logic::_scriptVars[SPECIAL_ITEM] = 0;
	_getOff = SCR_std_off;
	setPointer(MSE_POINTER, 0);
}

void Mouse::fnBlankMouse() {
	setPointer(0, 0);
}

void Mouse::fnNormalMouse() {
	setPointer(MSE_POINTER, 0);
}

void Mouse::fnLockMouse() {
	Logic::_scriptVars[MOUSE_STATUS] |= 2;
}

void Mouse::fnUnlockMouse() {
	Logic::_scriptVars[MOUSE_STATUS] &= 1;
}

void Mouse::giveCoords(uint16 *x, uint16 *y) {
	*x = _mouse.x;
	*y = _mouse.y;
}

} // End of namespace Sword1
