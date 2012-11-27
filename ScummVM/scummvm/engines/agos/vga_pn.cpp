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

#include "agos/agos.h"
#include "agos/intern.h"
#include "agos/vga.h"

#include "common/system.h"

#include "graphics/surface.h"

namespace AGOS {

void AGOSEngine_PN::setupVideoOpcodes(VgaOpcodeProc *op) {
	op[1] = &AGOSEngine::vc1_fadeOut;
	op[2] = &AGOSEngine::vc2_call;
	op[3] = &AGOSEngine::vc3_loadSprite;
	op[4] = &AGOSEngine::vc4_fadeIn;
	op[5] = &AGOSEngine::vc5_ifEqual;
	op[6] = &AGOSEngine::vc6_ifObjectHere;
	op[7] = &AGOSEngine::vc7_ifObjectNotHere;
	op[8] = &AGOSEngine::vc8_ifObjectIsAt;
	op[9] = &AGOSEngine::vc9_ifObjectStateIs;
	op[10] = &AGOSEngine::vc10_draw;
	op[11] = &AGOSEngine::vc11_onStop;
	op[13] = &AGOSEngine::vc12_delay;
	op[14] = &AGOSEngine::vc13_addToSpriteX;
	op[15] = &AGOSEngine::vc14_addToSpriteY;
	op[16] = &AGOSEngine::vc15_sync;
	op[17] = &AGOSEngine::vc16_waitSync;
	op[18] = &AGOSEngine::vc17_waitEnd;
	op[19] = &AGOSEngine::vc18_jump;
	op[20] = &AGOSEngine::vc19_loop;
	op[21] = &AGOSEngine::vc20_setRepeat;
	op[22] = &AGOSEngine::vc21_endRepeat;
	op[23] = &AGOSEngine::vc22_setPalette;
	op[24] = &AGOSEngine::vc23_setPriority;
	op[25] = &AGOSEngine::vc24_setSpriteXY;
	op[26] = &AGOSEngine::vc25_halt_sprite;
	op[27] = &AGOSEngine::vc26_setSubWindow;
	op[28] = &AGOSEngine::vc27_resetSprite;
	op[29] = &AGOSEngine::vc28_playSFX;
	op[30] = &AGOSEngine::vc29_stopAllSounds;
	op[31] = &AGOSEngine::vc30_setFrameRate;
	op[32] = &AGOSEngine::vc31_setWindow;
	op[33] = &AGOSEngine::vc32_saveScreen;
	op[34] = &AGOSEngine::vc33_setMouseOn;
	op[35] = &AGOSEngine::vc34_setMouseOff;
	op[36] = &AGOSEngine::vc36_pause;
	op[38] = &AGOSEngine::vc35_clearWindow;
	op[39] = &AGOSEngine::vc39_volume;
	op[40] = &AGOSEngine::vc36_setWindowImage;
	op[41] = &AGOSEngine::vc37_pokePalette;
	op[44] = &AGOSEngine::vc44_enableBox;
	op[45] = &AGOSEngine::vc45_disableBox;
	op[46] = &AGOSEngine::vc46_maxBox;
	op[48] = &AGOSEngine::vc48_specialEffect;
	op[50] = &AGOSEngine::vc50_setBox;
	op[51] = &AGOSEngine::vc38_ifVarNotZero;
	op[52] = &AGOSEngine::vc39_setVar;
	op[53] = &AGOSEngine::vc40_scrollRight;
	op[54] = &AGOSEngine::vc41_scrollLeft;
	op[55] = &AGOSEngine::vc55_scanFlag;
}

bool AGOSEngine_PN::ifObjectHere(uint16 a) {
	if (getFeatures() & GF_DEMO)
		return false;

	return _variableArray[39] == getptr(_quickptr[11] + a * _quickshort[4] + 2);
}

bool AGOSEngine_PN::ifObjectAt(uint16 a, uint16 b) {
	if (getFeatures() & GF_DEMO)
		return false;

	return b == getptr(_quickptr[11] + a * _quickshort[4] + 2);
}

bool AGOSEngine_PN::ifObjectState(uint16 a, int16 b) {
	if (getFeatures() & GF_DEMO)
		return false;

	return b == getptr(_quickptr[0] + a * _quickshort[0] + 2);
}

void AGOSEngine::vc36_pause() {
	const char *message1 = "Press any key to continue";
	bool oldWiped = _wiped;
	_wiped = 0;

	_videoLockOut |= 8;

	windowPutChar(_windowArray[2], 13);

	for (; *message1; message1++)
		windowPutChar(_windowArray[2], *message1);

	while (!shouldQuit()) {
		if (_keyPressed.ascii != 0)
			break;
		delay(1);
	}

	_keyPressed.reset();

	windowPutChar(_windowArray[2], 13);
	_wiped = oldWiped;

	_videoLockOut &= ~8;
}

void AGOSEngine::vc39_volume() {
	_vcPtr += 2;
}

void AGOSEngine::vc44_enableBox() {
	HitArea *ha = _hitAreas + vcReadNextWord();
	ha->flags &= ~kOBFBoxDisabled;
}

void AGOSEngine::vc45_disableBox() {
	HitArea *ha = _hitAreas + vcReadNextWord();
	ha->flags |= kOBFBoxDisabled;
}

void AGOSEngine::vc46_maxBox() {
	HitArea *ha = _hitAreas + vcReadNextWord();
	ha->id = 0xFFFF;
}

void AGOSEngine::vc48_specialEffect() {
	uint16 num = vcReadNextWord();
	vcReadNextWord();

	if (getPlatform() == Common::kPlatformPC) {
		if (num == 1) {
			Graphics::Surface *screen = _system->lockScreen();
			byte *dst = (byte *)screen->pixels;

			for (uint h = 0; h < _screenHeight; h++) {
				for (uint w = 0; w < _screenWidth; w++) {
					if (dst[w] == 15)
						dst[w] = 4;
				}
				dst += screen->pitch;
			}
			_system->unlockScreen();
		} else if (num == 2) {
			const char *str = "There are gurgling noises from the sink.";
			for (; *str; str++)
				windowPutChar(_textWindow, *str);
		}
	}
}

void AGOSEngine::vc50_setBox() {
	uint16 id, x, y, w, h, msg1, msg2, flags;
	const uint16 *vlut;

	id = vcReadNextWord();
	vlut = &_videoWindows[vcReadNextWord() * 4];
	x = vlut[0] * 16 + vcReadNextWord();
	y = vlut[1] + vcReadNextWord();
	h = vcReadNextWord();
	w = vcReadNextWord();
	msg1 = vcReadNextWord();
	msg2 = vcReadNextWord();
	flags = vcReadNextWord();

	// Compressed string
	if (!(flags & kOBFUseMessageList)) {
		msg1 += 0x8000;
	}

	defineBox(id, x, y, h, w, msg1, msg2, flags);
}

void AGOSEngine::vc55_scanFlag() {
	_scanFlag = 1;
}

void AGOSEngine_PN::clearVideoWindow(uint16 num, uint16 color) {
	const uint16 *vlut = &_videoWindows[num * 4];
	uint16 xoffs = vlut[0] * 16;
	uint16 yoffs = vlut[1];

	Graphics::Surface *screen = _system->lockScreen();
	byte *dst = (byte *)screen->pixels + xoffs + yoffs * screen->pitch;
	for (uint h = 0; h < vlut[3]; h++) {
		memset(dst, color, vlut[2] * 16);
		dst += screen->pitch;
	}
	 _system->unlockScreen();
}

} // End of namespace AGOS
