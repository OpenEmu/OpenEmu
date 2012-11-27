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

#include "gob/gob.h"
#include "gob/global.h"

namespace Gob {

Global::Global(GobEngine *vm) : _vm(vm) {
	for (int i = 0; i < 128; i++)
		_pressedKeys[i] = 0;

	_presentCGA = UNDEF;
	_presentEGA = UNDEF;
	_presentVGA = UNDEF;
	_presentHER = UNDEF;

	_videoMode     = 0;
	_fakeVideoMode = 0;
	_oldMode       = 3;

	_soundFlags = 0;

	_language       = 0x8000;
	_languageWanted = 0x8000;
	_foundLanguage  = false;

	_useMouse     = UNDEF;
	_mousePresent = UNDEF;
	_mouseXShift  = 3;
	_mouseYShift  = 3;

	_mouseMinX =   0;
	_mouseMinY =   0;
	_mouseMaxX = 320;
	_mouseMaxY = 200;

	_useJoystick = 1;

	_primaryWidth  = 0;
	_primaryHeight = 0;

	_colorCount = 16;
	for (int i = 0; i < 256; i++) {
		_redPalette  [i] = 0;
		_greenPalette[i] = 0;
		_bluePalette [i] = 0;
	}

	_unusedPalette1[ 0] = (int16) 0x0000;
	_unusedPalette1[ 1] = (int16) 0x000B;
	_unusedPalette1[ 2] = (int16) 0x0000;
	_unusedPalette1[ 3] = (int16) 0x5555;
	_unusedPalette1[ 4] = (int16) 0xAAAA;
	_unusedPalette1[ 5] = (int16) 0xFFFF;
	_unusedPalette1[ 6] = (int16) 0x0000;
	_unusedPalette1[ 7] = (int16) 0x5555;
	_unusedPalette1[ 8] = (int16) 0xAAAA;
	_unusedPalette1[ 9] = (int16) 0xFFFF;
	_unusedPalette1[10] = (int16) 0x0000;
	_unusedPalette1[11] = (int16) 0x5555;
	_unusedPalette1[12] = (int16) 0xAAAA;
	_unusedPalette1[13] = (int16) 0xFFFF;
	_unusedPalette1[14] = (int16) 0x0000;
	_unusedPalette1[15] = (int16) 0x5555;
	_unusedPalette1[16] = (int16) 0xAAAA;
	_unusedPalette1[17] = (int16) 0xFFFF;

	for (int i = 0; i < 16; i++)
		_unusedPalette2[i] = i;

	_vgaPalette[ 0].red = 0x00; _vgaPalette[ 0].green = 0x00; _vgaPalette[ 0].blue = 0x00;
	_vgaPalette[ 1].red = 0x00; _vgaPalette[ 1].green = 0x00; _vgaPalette[ 1].blue = 0x2A;
	_vgaPalette[ 2].red = 0x00; _vgaPalette[ 2].green = 0x2A; _vgaPalette[ 2].blue = 0x00;
	_vgaPalette[ 3].red = 0x00; _vgaPalette[ 3].green = 0x2A; _vgaPalette[ 3].blue = 0x2A;
	_vgaPalette[ 4].red = 0x2A; _vgaPalette[ 4].green = 0x00; _vgaPalette[ 4].blue = 0x00;
	_vgaPalette[ 5].red = 0x2A; _vgaPalette[ 5].green = 0x00; _vgaPalette[ 5].blue = 0x2A;
	_vgaPalette[ 6].red = 0x2A; _vgaPalette[ 6].green = 0x15; _vgaPalette[ 6].blue = 0x00;
	_vgaPalette[ 7].red = 0x2A; _vgaPalette[ 7].green = 0x2A; _vgaPalette[ 7].blue = 0x2A;
	_vgaPalette[ 8].red = 0x15; _vgaPalette[ 8].green = 0x15; _vgaPalette[ 8].blue = 0x15;
	_vgaPalette[ 9].red = 0x15; _vgaPalette[ 9].green = 0x15; _vgaPalette[ 9].blue = 0x3F;
	_vgaPalette[10].red = 0x15; _vgaPalette[10].green = 0x3F; _vgaPalette[10].blue = 0x15;
	_vgaPalette[11].red = 0x15; _vgaPalette[11].green = 0x3F; _vgaPalette[11].blue = 0x3F;
	_vgaPalette[12].red = 0x3F; _vgaPalette[12].green = 0x15; _vgaPalette[12].blue = 0x15;
	_vgaPalette[13].red = 0x3F; _vgaPalette[13].green = 0x15; _vgaPalette[13].blue = 0x3F;
	_vgaPalette[14].red = 0x3F; _vgaPalette[14].green = 0x3F; _vgaPalette[14].blue = 0x15;
	_vgaPalette[15].red = 0x3F; _vgaPalette[15].green = 0x3F; _vgaPalette[15].blue = 0x3F;

	_pPaletteDesc = 0;

	_setAllPalette  = false;
	_dontSetPalette = false;

	_debugFlag = 0;

	_inter_animDataSize = 10;

	_inter_mouseX = 0;
	_inter_mouseY = 0;

	_speedFactor = 1;

	_doSubtitles = false;

	_noCd = false;

	_curWinId = 0;
}

Global::~Global() {
}

} // End of namespace Gob
