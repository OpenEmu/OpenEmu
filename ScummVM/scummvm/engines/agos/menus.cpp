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

#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/surface.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

void AGOSEngine::loadMenuFile() {
	Common::File in;

	in.open(getFileName(GAME_MENUFILE));
	if (in.isOpen() == false) {
		error("loadMenuFile: Can't load menus file '%s'", getFileName(GAME_MENUFILE));
	}

	uint fileSize = in.size();
	_menuBase = (byte *)malloc(fileSize);
	if (_menuBase == NULL)
		error("loadMenuFile: Out of memory for menu data");
	in.read(_menuBase, fileSize);
	in.close();
}

// Personal Nightmare specific
void AGOSEngine::restoreMenu() {
	_wiped = 0;

	_videoLockOut |= 0x80;

	clearVideoWindow(3, 0);

	uint16 oldWindowNum = _windowNum;

	setWindowImage(1, 1);
	setWindowImage(2, 2);

	drawEdging();

	_windowNum = oldWindowNum;

	_videoLockOut |= 0x20;
	_videoLockOut &= ~0x80;
}

// Elvira 1 specific
void AGOSEngine::drawMenuStrip(uint windowNum, uint menuNum) {
	WindowBlock *window = _windowArray[windowNum % 8];

	mouseOff();

	byte *srcPtr = _menuBase;
	int menu = (menuNum != 0) ? menuNum * 4 + 1 : 0;

	while (menu--) {
		if (READ_LE_UINT16(srcPtr) != 0xFFFF) {
			srcPtr += 2;
			while (*srcPtr != 0)
				srcPtr++;
			srcPtr++;
		} else {
			srcPtr += 2;
		}
	}

	clearWindow(window);

	int newline = 0;
	while (READ_LE_UINT16(srcPtr) != 0xFFFF) {
		byte *tmp = srcPtr;
		srcPtr += 2;

		if (newline != 0) {
			windowPutChar(window, 10);
		}

		uint len = 0;
		while (*srcPtr != 0 && *srcPtr != 1) {
			len++;
			srcPtr++;
		}
		if (*srcPtr == 1)
			srcPtr++;

		uint maxLen = window->textMaxLength - len;

		if (window->flags & 1)
			window->textColumnOffset += 4;

		maxLen /= 2;
		while (maxLen--)
			windowPutChar(window, 32);

		srcPtr = tmp;
		uint verb = READ_BE_UINT16(srcPtr); srcPtr += 2;

		while (*srcPtr != 0) {
			windowPutChar(window, *srcPtr++);
		}
		srcPtr++;

		if (verb != 0xFFFE) {
			HitArea *ha = findEmptyHitArea();
			ha->x = window->x * 8 + 3;
			ha->y = window->textRow * 8 + window->y;
			ha->data = menuNum;
			ha->width = window->width * 8 - 6;
			ha->height = 7;
			ha->flags = kBFBoxInUse | kBFInvertTouch;
			ha->id = 30000;
			ha->priority = 1;
			ha->verb = verb;
		}

		newline = 0xFFFF;
	}

	mouseOn();
}

void AGOSEngine::lightMenuStrip(int a) {
	mouseOff();
	unlightMenuStrip();

	for (int i = 0; i != 10; i++) {
		if (a & (1 << i)) {
			enableBox(120 + i);
			lightMenuBox(120 + i);
		}
	}

	mouseOn();
}

void AGOSEngine::unlightMenuStrip() {
	byte *src;
	int w, h, i;

	mouseOff();

	Graphics::Surface *screen = _system->lockScreen();
	src = (byte *)screen->pixels + 8 * screen->pitch + 272;
	w = 48;
	h = 82;

	do {
		for (i = 0; i != w; ++i) {
			if (src[i] != 0)
				src[i] = 14;
		}
		src += screen->pitch;
	} while (--h);

	for (i = 120; i != 130; i++)
		disableBox(i);

	_system->unlockScreen();

	mouseOn();
}

void AGOSEngine::lightMenuBox(uint hitarea) {
	HitArea *ha = findBox(hitarea);
	byte *src;
	int w, h, i;

	mouseOff();

	Graphics::Surface *screen = _system->lockScreen();
	src = (byte *)screen->pixels + ha->y * screen->pitch + ha->x;
	w = ha->width;
	h = ha->height;

	do {
		for (i = 0; i != w; ++i) {
			if (src[i] == 14)
				src[i] = 15;
		}
		src += screen->pitch;
	} while (--h);

	_system->unlockScreen();

	mouseOn();
}

// Elvira 2 specific
uint AGOSEngine::menuFor_e2(Item *item) {
	if (item == NULL || item == _dummyItem2 || item == _dummyItem3)
		return 0xFFFF;

	SubObject *subObject = (SubObject *)findChildOfType(item, kObjectType);
	if (subObject != NULL && subObject->objectFlags & kOFMenu) {
		uint offs = getOffsetOfChild2Param(subObject, kOFMenu);
		return subObject->objectFlagValue[offs];
	}

	return _agosMenu;
}

// Waxworks specific
uint AGOSEngine::menuFor_ww(Item *item, uint id) {
	if (id != 0xFFFF && id < 10 && _textMenu[id] != 0)
		return _textMenu[id];

	if (item == NULL || item == _dummyItem2 || item == _dummyItem3)
		return _agosMenu;

	SubObject *subObject = (SubObject *)findChildOfType(item, kObjectType);
	if (subObject != NULL && subObject->objectFlags & kOFMenu) {
		uint offs = getOffsetOfChild2Param(subObject, kOFMenu);
		return subObject->objectFlagValue[offs];
	}

	return _agosMenu;
}

void AGOSEngine::clearMenuStrip() {
	int i;

	for (i = 111; i != 115; i++)
		disableBox(i);

	if (getGameType() == GType_WW) {
		setWindowImageEx(2, 101);
	} else {
		setWindowImageEx(2, 102);
	}
}

// Elvira 2 and Waxworks specific
void AGOSEngine::doMenuStrip(uint menuNum) {
	uint i;
	const uint var = (getGameType() == GType_WW) ? 11 : 1;

	for (i = 111; i != 115; i++)
		disableBox(i);

	for (i = var; i != (var + 5); i++)
		_variableArray[i] = 0;

	byte *srcPtr = _menuBase;
	while (menuNum--) {
		while (READ_BE_UINT16(srcPtr) != 0)
			srcPtr += 2;
		srcPtr += 2;
	}

	uint id = 111;
	uint v = var;

	while (READ_BE_UINT16(srcPtr) != 0) {
		uint verb = READ_BE_UINT16(srcPtr);
		_variableArray[v] = verb;

		HitArea *ha = findBox(id);
		if (ha != NULL) {
			ha->flags &= ~kBFBoxDead;
			ha->verb = verb;
		}

		id++;
		srcPtr += 2;
		v++;
	}

	_variableArray[var + 4] = id - 111;
	if (getGameType() == GType_WW) {
		setWindowImageEx(2, 102);
	} else {
		setWindowImageEx(2, 103);
	}
}

} // End of namespace AGOS
