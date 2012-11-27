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

#include "common/config-manager.h"

#include "agos/intern.h"
#include "agos/agos.h"

namespace AGOS {

AGOSEngine_PN::AGOSEngine_PN(OSystem *system, const AGOSGameDescription *gd)
	: AGOSEngine(system, gd) {

	_stackbase = 0;
	_tagOfActiveDoline = 0;
	_dolineReturnVal = 0;

	_dataBase = 0;
	_dataBaseSize = 0;
	_textBase = 0;
	_textBaseSize = 0;

	memset(_buffer, 0, sizeof(_buffer));
	memset(_inputline, 0, sizeof(_inputline));
	memset(_saveFile, 0, sizeof(_saveFile));
	memset(_sb, 0, sizeof(_sb));
	memset(_wordcp, 0, sizeof(_wordcp));

	memset(_objectName1, 0, sizeof(_objectName1));
	memset(_objectName2, 0, sizeof(_objectName2));

	_dragStore = 0;
	_hitCalled = 0;
	_inputReady = false;
	_inputting = false;
	_intputCounter = 0;
	_inputMax = 0;
	_mousePrintFG = 0;
	_mouseString = 0;
	_mouseString1 = 0;
	memset(_inMessage, 0, sizeof(_inMessage));
	memset(_placeMessage, 0, sizeof(_placeMessage));

	memset(_quickptr, 0, sizeof(_quickptr));
	memset(_quickshort, 0, sizeof(_quickshort));

	_noScanFlag = false;
	memset(_keyboardBuffer, 0, sizeof(_keyboardBuffer));

	_objects = 0;
	_objectCountS = 0;

	_bp = 0;
	_xofs = 0;
	_havinit = 0;
	_seed = 0;

	_curwrdptr = 0;
	_inpp = 0;
	_fnst = 0;
	_linembr = 0;
	_linct = 0;
	_procnum = 0;

	_linebase = 0;
	_workptr = 0;
}

AGOSEngine_PN::~AGOSEngine_PN() {
	free(_dataBase);
	free(_textBase);

}

const byte egaPalette[48] = {
	  0,   0,   0,
	  0,   0, 170,
	  0, 170,   0,
	  0, 170, 170,
	170,   0,   0,
	170,   0, 170,
	170,  85,   0,
	170, 170, 170,
	 85,  85,  85,
	 85,  85, 255,
	 85, 255,  85,
	 85, 255, 255,
	255,  85,  85,
	255,  85, 255,
	255, 255,  85,
	255, 255, 255
};

Common::Error AGOSEngine_PN::go() {
	loadGamePcFile();

	if (getFileName(GAME_ICONFILE) != NULL) {
		loadIconFile();
	}

	setupBoxes();

	vc34_setMouseOff();

	addVgaEvent(_frameCount, ANIMATE_INT, NULL, 0, 0);

	if (getFeatures() & GF_EGA) {
		// Set EGA Palette
		memcpy(_displayPalette, egaPalette, sizeof(egaPalette));
		_paletteFlag = 1;
	}

	_inputWindow = _windowArray[2] = openWindow(0, 192, 40, 1, 1, 0, 15);
	_textWindow = _windowArray[0] = openWindow(1, 136, 38, 6, 1, 0, 15);

	if (getFeatures() & GF_DEMO) {
		demoSeq();
	} else {
		introSeq();
		processor();
	}

	return Common::kNoError;
}

void AGOSEngine_PN::demoSeq() {
	while (!shouldQuit()) {
		loadZone(0);
		setWindowImage(3, 0);
		while (!shouldQuit() && _variableArray[228] != 1)
			delay(1);

		loadZone(1);
		setWindowImage(0, 0);
		while (!shouldQuit() && _variableArray[228] != 2)
			delay(1);

		loadZone(2);
		setWindowImage(0, 0);
		while (!shouldQuit() && _variableArray[228] != 3)
			delay(1);
	}
}

void AGOSEngine_PN::introSeq() {
	loadZone(25); // Zone 'I'
	setWindowImage(3, 0);

	_exitCutscene = false;
	while (!shouldQuit() && !_exitCutscene && _variableArray[228] != 1) {
		processSpecialKeys();
		delay(1);
	}

	setWindowImage(3, 3);
	delay(100);

	loadZone(27); // Zone 'K'
	setWindowImage(3, 0);

	_exitCutscene = false;
	while (!shouldQuit() && !_exitCutscene && _variableArray[228] != 2) {
		processSpecialKeys();
		delay(1);
	}
}

void AGOSEngine_PN::setupBoxes() {
	_hitAreaList = _invHitAreas;
	// Inventory box
	defineBox( 0,  11,  68, 16,  26, 25, 0, kOBFDraggable | kOBFUseEmptyLine | kOBFInventoryBox | kOBFNoShowName);
	// Room Box
	defineBox( 1,  11, 103, 16,  26, 26, 0, kOBFDraggable | kOBFUseEmptyLine | kOBFRoomBox | kOBFNoShowName);
	// Exit box
	defineBox( 2,  48,   2,  8,  28, 27, 0, kOBFUseEmptyLine | kOBFNoShowName);
	// More box
	defineBox( 3,  80,   2,  8,  26, 28, 0, kOBFUseEmptyLine | kOBFMoreBox | kOBFNoShowName);
	// Close box
	defineBox( 4, 110,   2,  8,  28, 29, 0, kOBFUseEmptyLine | kOBFNoShowName);

	// Icon boxes
	uint8 num = 5;
	for (uint8 r = 0; r < 5; r++) {
		for (uint8 i = 0; i < 7; i++) {
			defineBox(num, 96 + i * 24, 12 + r * 24, 24, 24, 0, 3, kOBFObject | kOBFDraggable);
			num++;
		}
	}

	// Mark the end of inventory boxes
	HitArea *ha = _hitAreaList + num;
	ha->id = 0xFFFF;

	_hitAreaList = _hitAreas;
	defineBox( 0,  0,    0, 200, 320, 0, 0, kOBFBoxDisabled | kOBFNoShowName);
	defineBox( 1, 273,   4,   5,  45, 1, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox( 2, 273,  12,   5,  45, 2, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox( 3, 273,  20,   5,  45, 3, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox( 4, 273,  28,   5,  45, 4, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox( 5, 273,  36,   5,  45, 5, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox( 6, 273,  44,   5,  45, 6, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox( 7, 273,  52,   5,  45, 7, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox( 8, 273,  60,   5,  45, 8, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox( 9, 273,  68,   5,  45, 9, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox(10, 273,  76,   5,  45, 10, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox(11, 273,  84,   5,  45, 11, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox(12, 273,  92,   5,  45, 12, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox(13, 273, 100,   5,  45, 13, 0, kOBFUseEmptyLine | kOBFBoxDisabled | kOBFNoShowName);
	defineBox(14, 273, 107,   5,  45, 14, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox(15, 273, 115,   5,  45, 15, 0, kOBFUseEmptyLine | kOBFNoShowName | kOBFInvertTouch);
	defineBox(16, 273, 123,   5,  45, 16, 0, kOBFUseEmptyLine | kOBFBoxDisabled | kOBFNoShowName);
	defineBox(17,  20,   5,   7,   7, 17, 0, kOBFUseEmptyLine | kOBFNoShowName);
	defineBox(18,  28,  11,   7,  13, 18, 0, kOBFUseEmptyLine | kOBFNoShowName);
	defineBox(19,  36,  21,   7,   7, 19, 0, kOBFUseEmptyLine | kOBFNoShowName);
	defineBox(20,  27,  31,   7,  13, 20, 0, kOBFUseEmptyLine | kOBFNoShowName);
	defineBox(21,  20,  37,   7,   7, 21, 0, kOBFUseEmptyLine | kOBFNoShowName);
	defineBox(22,   5,  31,   7,  13, 22, 0, kOBFUseEmptyLine | kOBFNoShowName);
	defineBox(23,   4,  21,   7,   7, 23, 0, kOBFUseEmptyLine | kOBFNoShowName);
	defineBox(24,   5,  11,   7,  13, 24, 0, kOBFUseEmptyLine | kOBFNoShowName);
	defineBox(25,  11,  68,  16,  26, 25, 0, kOBFDraggable | kOBFUseEmptyLine | kOBFInventoryBox | kOBFNoShowName);
	defineBox(26,  11, 103,  16,  26, 26, 0, kOBFDraggable | kOBFUseEmptyLine | kOBFRoomBox | kOBFNoShowName);
}

void AGOSEngine_PN::processor() {
	setqptrs();

	_tagOfActiveDoline = 0;
	int q = 0;
	do {
		assert(_tagOfActiveDoline == 0);
		_dolineReturnVal = 0;

		_variableArray[6] = 0;

		if (getPlatform() == Common::kPlatformAtariST) {
			_variableArray[21] = 2;
		} else if (getPlatform() == Common::kPlatformAmiga) {
			_variableArray[21] = 0;
		} else {
			_variableArray[21] = 1;
		}

		_variableArray[16] = _quickshort[6];
		_variableArray[17] = _quickshort[7];
		_variableArray[19] = getptr(55L);

		// q indicates the process to run and is 0 the first time,
		// but 1 later on (i.e., when we are "called" from badload()).
		setposition(0, 0);
		q = doline(0);
	} while (q);
}

void AGOSEngine_PN::setqptrs() {
	for (int i = 0; i < 11; ++i) {
		_quickptr[i] = getlong(3 * i);
	}
	_quickptr[11] = getlong(58L);
	_quickptr[12] = getlong(61L);
	_quickshort[0] = getptr(35L);
	_quickshort[1] = getptr(37L);
	_quickshort[2] = getptr(39L);
	_quickshort[3] = getptr(41L);
	_quickshort[4] = getptr(43L);
	_quickshort[5] = getptr(45L);
	_quickshort[6] = getptr(51L);
	_quickshort[7] = getptr(53L);
}

} // End of namespace AGOS
