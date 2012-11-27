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

#include "dreamweb/sound.h"
#include "dreamweb/dreamweb.h"

namespace DreamWeb {

void DreamWebEngine::newPlace() {
	if (_vars._needToTravel == 1) {
		_vars._needToTravel = 0;
		selectLocation();
	} else if (_autoLocation != 0xFF) {
		_newLocation = _autoLocation;
		_autoLocation = 0xFF;
	}
}

void DreamWebEngine::selectLocation() {
	_inMapArea = 0;
	clearBeforeLoad();
	_getBack = 0;
	_pointerFrame = 22;
	readCityPic();
	showCity();
	_cityGraphics.clear();
	readDestIcon();
	loadTravelText();
	showPanel();
	showMan();
	showArrows();
	showExit();
	locationPic();
	underTextLine();
	_commandType = 255;
	readMouse();
	_pointerFrame = 0;
	showPointer();
	workToScreen();
	_sound->playChannel0(9, 255);
	_newLocation = 255;

	while (_newLocation == 255) {
		if (_quitRequested)
			break;

		delPointer();
		readMouse();
		showPointer();
		waitForVSync();
		dumpPointer();
		dumpTextLine();

		if (_getBack == 1)
			break;

		RectWithCallback destList[] = {
			{ 238,258,4,44,&DreamWebEngine::nextDest },
			{ 104,124,4,44,&DreamWebEngine::lastDest },
			{ 280,308,4,44,&DreamWebEngine::lookAtPlace },
			{ 104,216,138,192,&DreamWebEngine::destSelect },
			{ 273,320,157,198,&DreamWebEngine::getBack1 },
			{ 0,320,0,200,&DreamWebEngine::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(destList);
	}

	if (_quitRequested || _getBack == 1 || _newLocation == _vars._location) {
		_newLocation = _realLocation;
		_getBack = 0;
	}

	_newplaceGraphics.clear();
	_newplaceGraphics2.clear();
	_newplaceGraphics3.clear();

	_travelText.clear();
}

void DreamWebEngine::showCity() {
	clearWork();
	showFrame(_cityGraphics, 57, 32, 0, 0);
	showFrame(_cityGraphics, 120+57, 32, 1, 0);
}

void DreamWebEngine::lookAtPlace() {
	commandOnlyCond(27, 224);

	if (!(_mouseButton & 1) ||
		_mouseButton == _oldButton ||
		_destPos >= 15)
		return; // noinfo

	delPointer();
	delTextLine();
	getUnderCentre();
	showFrame(_newplaceGraphics3, 60, 72, 0, 0);
	showFrame(_newplaceGraphics3, 60, 72 + 55, 4, 0);
	if (_foreignRelease)
		showFrame(_newplaceGraphics3, 60, 72+55+21, 4, 0);

	const uint8 *string = (const uint8 *)_travelText.getString(_destPos);
	findNextColon(&string);
	uint16 y = (_foreignRelease) ? 84 + 4 : 84;
	printDirect(&string, 63, &y, 191, 191 & 1);
	workToScreenM();
	hangOnP(500);
	_pointerMode = 0;
	_pointerFrame = 0;
	putUnderCentre();
	workToScreenM();
}

void DreamWebEngine::getUnderCentre() {
	multiGet(_mapStore, 58, 72, 254, 110);
}

void DreamWebEngine::putUnderCentre() {
	multiPut(_mapStore, 58, 72, 254, 110);
}

void DreamWebEngine::locationPic() {
	const int roomPics[] = { 5, 0, 3, 2, 4, 1, 10, 9, 8, 6, 11, 4, 7, 7, 0 };
	byte picture = roomPics[_destPos];

	if (picture >= 6)
		showFrame(_newplaceGraphics2, 104, 138 + 14, picture - 6, 0);	// Second slot
	else
		showFrame(_newplaceGraphics,  104, 138 + 14, picture + 4, 0);

	if (_destPos == _realLocation)
		showFrame(_newplaceGraphics, 104, 140 + 14, 3, 0);	// Currently in this location

	const uint8 *string = (const uint8 *)_travelText.getString(_destPos);
	printDirect(string, 50, 20, 241, 241 & 1);
}

void DreamWebEngine::showArrows() {
	showFrame(_newplaceGraphics, 116 - 12, 16, 0, 0);
	showFrame(_newplaceGraphics, 226 + 12, 16, 1, 0);
	showFrame(_newplaceGraphics, 280, 14, 2, 0);
}

void DreamWebEngine::nextDest() {
	commandOnlyCond(28, 218);

	if (!(_mouseButton & 1) || _oldButton == 1)
		return;	// nodu

	do {
		_destPos++;
		if (_destPos == 15)
			_destPos = 0;	// last destination
	} while (!getLocation(_destPos));

	_newTextLine = 1;
	delTextLine();
	delPointer();
	showPanel();
	showMan();
	showArrows();
	locationPic();
	underTextLine();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamWebEngine::lastDest() {
	commandOnlyCond(29, 219);

	if (!(_mouseButton & 1) || _oldButton == 1)
		return;	// nodd

	do {
		_destPos--;
		if (_destPos == 0xFF)
			_destPos = 15;	// first destination
	} while (!getLocation(_destPos));

	_newTextLine = 1;
	delTextLine();
	delPointer();
	showPanel();
	showMan();
	showArrows();
	locationPic();
	underTextLine();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamWebEngine::destSelect() {
	commandOnlyCond(30, 222);

	if (!(_mouseButton & 1) || _oldButton == 1)
		return;	// notrav

	_newLocation = _destPos;
}

uint8 DreamWebEngine::getLocation(uint8 index) {
	return _roomsCanGo[index];
}

void DreamWebEngine::setLocation(uint8 index) {
	_roomsCanGo[index] = 1;
}

void DreamWebEngine::clearLocation(uint8 index) {
	_roomsCanGo[index] = 0;
}

void DreamWebEngine::resetLocation(uint8 index) {
	if (index == 5) {
		// delete hotel
		purgeALocation(5);
		purgeALocation(21);
		purgeALocation(22);
		purgeALocation(27);
	} else if (index == 8) {
		// delete TV studio
		purgeALocation(8);
		purgeALocation(28);
	} else if (index == 6) {
		// delete sarters
		purgeALocation(6);
		purgeALocation(20);
		purgeALocation(25);
	} else if (index == 13) {
		// delete boathouse
		purgeALocation(13);
		purgeALocation(29);
	}

	clearLocation(index);
}

void DreamWebEngine::readDestIcon() {
	loadGraphicsFile(_newplaceGraphics, "G05");
	loadGraphicsFile(_newplaceGraphics2, "G06");
	loadGraphicsFile(_newplaceGraphics3, "G08");
}

void DreamWebEngine::readCityPic() {
	loadGraphicsFile(_cityGraphics, "G04");
}

} // End of namespace DreamWeb
