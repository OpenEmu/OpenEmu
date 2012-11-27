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

#ifndef SKY_MOUSE_H
#define SKY_MOUSE_H


#include "common/scummsys.h"

class OSystem;

namespace Sky {

class Disk;
class Logic;
class SkyCompact;

class Mouse {

public:

	Mouse(OSystem *system, Disk *skyDisk, SkyCompact *skyCompact);
	~Mouse();

	void mouseEngine();
	void replaceMouseCursors(uint16 fileNo);
	bool fnAddHuman();
	void fnSaveCoods();
	void fnOpenCloseHand(bool open);
	uint16 findMouseCursor(uint32 itemNum);
	void lockMouse();
	void unlockMouse();
	void restoreMouseData(uint16 frameNum);
	void drawNewMouse();
	void spriteMouse(uint16 frameNum, uint8 mouseX, uint8 mouseY);
	void useLogicInstance(Logic *skyLogic) { _skyLogic = skyLogic; }
	void buttonPressed(uint8 button);
	void mouseMoved(uint16 mouseX, uint16 mouseY);
	void waitMouseNotPressed(int minDelay = 0);
	uint16 giveMouseX() { return _mouseX; }
	uint16 giveMouseY() { return _mouseY; }
	uint16 giveCurrentMouseType() { return _currentCursor; }
	bool wasClicked();
	void logicClick() { _logicClick = true; }
	void resetCursor();

protected:

	void pointerEngine(uint16 xPos, uint16 yPos);
	void buttonEngine1();

	bool _logicClick;

	uint16 _mouseB;	//mouse button
	uint16 _mouseX;	//actual mouse coordinates
	uint16 _mouseY;

	uint16 _currentCursor;

	byte *_miceData;	//address of mouse sprites
	byte *_objectMouseData;	//address of object mouse sprites

	static uint32 _mouseMainObjects[24];
	static uint32 _mouseLincObjects[21];

	OSystem *_system;
	Disk *_skyDisk;
	Logic *_skyLogic;
	SkyCompact *_skyCompact;
};

} // End of namespace Sky

#endif //SKYMOUSE_H
