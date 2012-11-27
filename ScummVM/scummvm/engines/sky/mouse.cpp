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


#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"
#include "sky/disk.h"
#include "sky/logic.h"
#include "sky/mouse.h"
#include "sky/sky.h"
#include "sky/skydefs.h"
#include "sky/struc.h"
#include "sky/compact.h"

namespace Sky {

#define MICE_FILE	60300
#define NO_MAIN_OBJECTS	24
#define NO_LINC_OBJECTS	21

uint32 Mouse::_mouseMainObjects[24] = {
	65,
	9,
	66,
	64,
	8,
	63,
	10,
	11,
	71,
	76,
	37,
	36,
	42,
	75,
	79,
	6,
	74,
	39,
	49,
	43,
	34,
	35,
	77,
	38
};

uint32 Mouse::_mouseLincObjects[21] = {
	24625,
	24649,
	24827,
	24651,
	24583,
	24581,
	24582,
	24628,
	24650,
	24629,
	24732,
	24631,
	24584,
	24630,
	24626,
	24627,
	24632,
	24643,
	24828,
	24830,
	24829
};

Mouse::Mouse(OSystem *system, Disk *skyDisk, SkyCompact *skyCompact) {
	_skyDisk = skyDisk;
	_skyCompact = skyCompact;
	_system = system;
	_mouseB = 0;
	_currentCursor = 6;
	_mouseX = GAME_SCREEN_WIDTH / 2;
	_mouseY = GAME_SCREEN_HEIGHT / 2;

	_miceData = _skyDisk->loadFile(MICE_FILE);

	//load in the object mouse file
	_objectMouseData = _skyDisk->loadFile(MICE_FILE + 1);
}

Mouse::~Mouse( ){
	free (_miceData);
	free (_objectMouseData);
}

void Mouse::replaceMouseCursors(uint16 fileNo) {
	free(_objectMouseData);
	_objectMouseData = _skyDisk->loadFile(fileNo);
}

bool Mouse::fnAddHuman() {
	//reintroduce the mouse so that the human can control the player
	//could still be switched out at high-level

	if (!Logic::_scriptVariables[MOUSE_STOP]) {
		Logic::_scriptVariables[MOUSE_STATUS] |= 6;	//cursor & mouse

		if (_mouseY < 2) //stop mouse activating top line
			_mouseY = 2;

		_system->warpMouse(_mouseX, _mouseY);

		//force the pointer engine into running a get-off
		//even if it's over nothing

		//KWIK-FIX
		//get off may contain script to remove mouse pointer text
		//surely this script should be run just in case
		//I am going to try it anyway
		if (Logic::_scriptVariables[GET_OFF])
			_skyLogic->script((uint16)Logic::_scriptVariables[GET_OFF],(uint16)(Logic::_scriptVariables[GET_OFF] >> 16));

		Logic::_scriptVariables[SPECIAL_ITEM] = 0xFFFFFFFF;
		Logic::_scriptVariables[GET_OFF] = RESET_MOUSE;
	}

	return true;
}

void Mouse::fnSaveCoods() {
	Logic::_scriptVariables[SAFEX] = _mouseX + TOP_LEFT_X;
	Logic::_scriptVariables[SAFEY] = _mouseY + TOP_LEFT_Y;
}

void Mouse::lockMouse() {
	SkyEngine::_systemVars.systemFlags |= SF_MOUSE_LOCKED;
}

void Mouse::unlockMouse() {
	SkyEngine::_systemVars.systemFlags &= ~SF_MOUSE_LOCKED;
}

void Mouse::restoreMouseData(uint16 frameNum) {
	warning("Stub: Mouse::restoreMouseData");
}

void Mouse::drawNewMouse() {
	warning("Stub: Mouse::drawNewMouse");
	//calculateMouseValues();
	//saveMouseData();
	//drawMouse();
}

void Mouse::waitMouseNotPressed(int minDelay) {
	bool mousePressed = true;
	uint32 now = _system->getMillis();
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();
	while (mousePressed || _system->getMillis() < now + minDelay) {

		if (eventMan->shouldQuit()) {
			minDelay = 0;
			mousePressed = false;
		}

		if (!eventMan->getButtonState())
			mousePressed = false;

		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					minDelay = 0;
					mousePressed = false;
				}
				break;
			default:
				break;
			}
		}
		_system->updateScreen();
		_system->delayMillis(20);
	}
}

void Mouse::spriteMouse(uint16 frameNum, uint8 mouseX, uint8 mouseY) {
	_currentCursor = frameNum;

	byte *newCursor = _miceData;
	newCursor += ((DataFileHeader *)_miceData)->s_sp_size * frameNum;
	newCursor += sizeof(DataFileHeader);

	uint16 mouseWidth = ((DataFileHeader *)_miceData)->s_width;
	uint16 mouseHeight = ((DataFileHeader *)_miceData)->s_height;

	CursorMan.replaceCursor(newCursor, mouseWidth, mouseHeight, mouseX, mouseY, 0);
	if (frameNum == MOUSE_BLANK)
		CursorMan.showMouse(false);
	else
		CursorMan.showMouse(true);
}

void Mouse::mouseEngine() {
	_logicClick = (_mouseB > 0); // click signal is available for Logic for one gamecycle

	if (!Logic::_scriptVariables[MOUSE_STOP]) {
		if (Logic::_scriptVariables[MOUSE_STATUS] & (1 << 1)) {
			pointerEngine(_mouseX + TOP_LEFT_X, _mouseY + TOP_LEFT_Y);
			if (Logic::_scriptVariables[MOUSE_STATUS] & (1 << 2)) //buttons enabled?
				buttonEngine1();
		}
	}
	_mouseB = 0;	//don't save up buttons
}

void Mouse::pointerEngine(uint16 xPos, uint16 yPos) {
	uint32 currentListNum = Logic::_scriptVariables[MOUSE_LIST_NO];
	uint16 *currentList;
	do {
		currentList = (uint16 *)_skyCompact->fetchCpt(currentListNum);
		while ((*currentList != 0) && (*currentList != 0xFFFF)) {
			uint16 itemNum = *currentList;
			Compact *itemData = _skyCompact->fetchCpt(itemNum);
			currentList++;
			if ((itemData->screen == Logic::_scriptVariables[SCREEN]) && (itemData->status & 16)) {
				if (itemData->xcood + ((int16)itemData->mouseRelX) > xPos) continue;
				if (itemData->xcood + ((int16)itemData->mouseRelX) + itemData->mouseSizeX < xPos) continue;
				if (itemData->ycood + ((int16)itemData->mouseRelY) > yPos) continue;
				if (itemData->ycood + ((int16)itemData->mouseRelY) + itemData->mouseSizeY < yPos) continue;
				// we've hit the item
				if (Logic::_scriptVariables[SPECIAL_ITEM] == itemNum)
					return;
				Logic::_scriptVariables[SPECIAL_ITEM] = itemNum;
				if (Logic::_scriptVariables[GET_OFF])
					_skyLogic->mouseScript(Logic::_scriptVariables[GET_OFF], itemData);
				Logic::_scriptVariables[GET_OFF] = itemData->mouseOff;
				if (itemData->mouseOn)
					_skyLogic->mouseScript(itemData->mouseOn, itemData);
				return;
			}
		}
		if (*currentList == 0xFFFF)
			currentListNum = currentList[1];
	} while (*currentList != 0);
	if (Logic::_scriptVariables[SPECIAL_ITEM] != 0) {
		Logic::_scriptVariables[SPECIAL_ITEM] = 0;

		if (Logic::_scriptVariables[GET_OFF])
			_skyLogic->script((uint16)Logic::_scriptVariables[GET_OFF],(uint16)(Logic::_scriptVariables[GET_OFF] >> 16));
		Logic::_scriptVariables[GET_OFF] = 0;
	}
}

void Mouse::buttonPressed(uint8 button) {
	_mouseB = button;
}

void Mouse::mouseMoved(uint16 mouseX, uint16 mouseY) {
	_mouseX = mouseX;
	_mouseY = mouseY;
}

void Mouse::buttonEngine1() {
	//checks for clicking on special item
	//"compare the size of this routine to S1 mouse_button"

	if (_mouseB) {	//anything pressed?
		Logic::_scriptVariables[BUTTON] = _mouseB;
		if (Logic::_scriptVariables[SPECIAL_ITEM]) { //over anything?
			Compact *item = _skyCompact->fetchCpt(Logic::_scriptVariables[SPECIAL_ITEM]);
			if (item->mouseClick)
				_skyLogic->mouseScript(item->mouseClick, item);
		}
	}
}

void Mouse::resetCursor() {
	spriteMouse(_currentCursor, 0, 0);
}

uint16 Mouse::findMouseCursor(uint32 itemNum) {
	uint8 cnt;
	for (cnt = 0; cnt < NO_MAIN_OBJECTS; cnt++) {
		if (itemNum == _mouseMainObjects[cnt]) {
			return cnt;
		}
	}
	for (cnt = 0; cnt < NO_LINC_OBJECTS; cnt++) {
		if (itemNum == _mouseLincObjects[cnt]) {
			return cnt;
		}
	}
	return 0;
}

void Mouse::fnOpenCloseHand(bool open) {
	if ((!open) && (!Logic::_scriptVariables[OBJECT_HELD])) {
		spriteMouse(1, 0, 0);
		return;
	}
	uint16 cursor = findMouseCursor(Logic::_scriptVariables[OBJECT_HELD]) << 1;
	if (open)
		cursor++;

	uint32 size = ((DataFileHeader *)_objectMouseData)->s_sp_size;
	uint8 *srcData;
	uint8 *destData;

	srcData = (uint8 *)_objectMouseData + size * cursor + sizeof(DataFileHeader);
	destData = (uint8 *)_miceData + sizeof(DataFileHeader);
	memcpy(destData, srcData, size);
	spriteMouse(0, 5, 5);
}

bool Mouse::wasClicked() {
	if (_logicClick) {
		_logicClick = false;
		return true;
	} else
		return false;
}

} // End of namespace Sky
