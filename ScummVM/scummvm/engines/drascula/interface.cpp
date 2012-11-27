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

#include "drascula/drascula.h"
#include "graphics/cursorman.h"

namespace Drascula {

void DrasculaEngine::setCursor(int cursor) {
	switch (cursor) {
	case kCursorCrosshair:
		CursorMan.replaceCursor(crosshairCursor, 40, 25, 20, 17, 255);
		break;
	case kCursorCurrentItem:
		CursorMan.replaceCursor(mouseCursor, OBJWIDTH, OBJHEIGHT, 20, 17, 255);
	default:
		break;
	}
}

void DrasculaEngine::showCursor() {
	CursorMan.showMouse(true);
}

void DrasculaEngine::hideCursor() {
	CursorMan.showMouse(false);
}

bool DrasculaEngine::isCursorVisible() {
	return CursorMan.isVisible();
}

void DrasculaEngine::selectVerbFromBar() {
	for (int n = 0; n < 7; n++) {
		if (mouseX > _verbBarX[n] && mouseX < _verbBarX[n + 1] && n > 0) {
			selectVerb(n);
			return;
		}
	}

	// no verb selected
	selectVerb(kVerbNone);
}

void DrasculaEngine::selectVerb(int verb) {
	debug(4, "selectVerb(%d)", verb);

	int c = _menuScreen ? 0 : 171;

	if (currentChapter == 5) {
		if (takeObject == 1 && pickedObject != 16)
			addObject(pickedObject);
	} else {
		if (takeObject == 1)
			addObject(pickedObject);
	}

	for (int i = 0; i < OBJHEIGHT; i++)
		memcpy(mouseCursor + i * OBJWIDTH, cursorSurface + OBJWIDTH * verb + (c + i) * 320, OBJWIDTH);
	setCursor(kCursorCurrentItem);

	if (verb > 0) {
		takeObject = 1;
		pickedObject = verb;
	} else {
		takeObject = 0;
		_hasName = false;
	}
}

bool DrasculaEngine::confirmExit() {
	byte key = 0;

	color_abc(kColorRed);
	updateRoom();
	centerText(_textsys[1], 160, 87);
	updateScreen();

	delay(100);
	while (!shouldQuit()) {
		key = getScan();
		if (key != 0)
			break;

		// This gives a better feedback to the user when he is asked to
		// confirm whether he wants to quit. It now still updates the room and
		// shows mouse cursor movement. Hopefully it will work in all
		// locations of the game.
		updateRoom();
		color_abc(kColorRed);
		centerText(_textsys[1], 160, 87);
		updateScreen();
	}

	if (key == Common::KEYCODE_ESCAPE || shouldQuit()) {
		stopMusic();
		return false;
	}

	return true;
}

void DrasculaEngine::showMenu() {
	int h, n, x;
	char textIcon[13];
	byte *srcSurface = (currentChapter == 6) ? tableSurface : frontSurface;
	x = whichObject();
	strcpy(textIcon, iconName[x]);

	for (n = 1; n < ARRAYSIZE(inventoryObjects); n++) {
		h = inventoryObjects[n];

		if (h != 0) {
			copyBackground(_polX[n], _polY[n], _itemLocations[n].x, _itemLocations[n].y,
							OBJWIDTH, OBJHEIGHT, srcSurface, screenSurface);
		}
		copyRect(_x1d_menu[h], _y1d_menu[h], _itemLocations[n].x, _itemLocations[n].y,
				OBJWIDTH, OBJHEIGHT, cursorSurface, screenSurface);
	}

	if (x < 7)
		print_abc(textIcon, _itemLocations[x].x - 2, _itemLocations[x].y - 7);
}

void DrasculaEngine::clearMenu() {
	int n, verbActivated = 1;

	for (n = 0; n < 7; n++) {
		if (mouseX > _verbBarX[n] && mouseX < _verbBarX[n + 1])
			verbActivated = 0;
		copyRect(OBJWIDTH * n, OBJHEIGHT * verbActivated, _verbBarX[n], 2,
						OBJWIDTH, OBJHEIGHT, cursorSurface, screenSurface);
		verbActivated = 1;
	}
}

void DrasculaEngine::enterName() {
	Common::KeyCode key;
	flushKeyBuffer();
	int v = 0, h = 0;
	char select2[23];
	strcpy(select2, "                      ");
	while (!shouldQuit()) {
		select2[v] = '-';
		copyBackground(115, 14, 115, 14, 176, 9, bgSurface, screenSurface);
		print_abc(select2, 117, 15);
		updateScreen();

		key = getScan();

		if (key != 0) {
			if (key >= 0 && key <= 0xFF && isAlpha(key))
				select2[v] = tolower(key);
			else if ((key >= Common::KEYCODE_0 && key <= Common::KEYCODE_9) || key == Common::KEYCODE_SPACE)
				select2[v] = key;
			else if (key == Common::KEYCODE_ESCAPE)
				break;
			else if (key == Common::KEYCODE_RETURN) {
				select2[v] = '\0';
				h = 1;
				break;
			} else if (key == Common::KEYCODE_BACKSPACE)
				select2[v] = '\0';
			else
				v--;

			if (key == Common::KEYCODE_BACKSPACE)
				v--;
			else
				v++;
		}
		if (v == 22)
			v = 21;
		else if (v == -1)
			v = 0;
	}
	if (h == 1) {
		strcpy(select, select2);
		selectionMade = 1;
	}
}

bool DrasculaEngine::checkMenuFlags() {
	int n = whichObject();
	if (n != 0) {
		if (inventoryObjects[n] != 0 && checkAction(inventoryObjects[n]))
			return true;
	}

	return false;
}

void DrasculaEngine::showMap() {
	_hasName = false;

	for (int l = 0; l < numRoomObjs; l++) {
		if (mouseX > x1[l] && mouseY > y1[l]
				&& mouseX < x2[l] && mouseY < y2[l]
				&& visible[l] == 1) {
			strcpy(textName, objName[l]);
			_hasName = true;
		}
	}
}

} // End of namespace Drascula
