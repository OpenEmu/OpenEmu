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

namespace AGOS {

void AGOSEngine_Simon1::setupVideoOpcodes(VgaOpcodeProc *op) {
	AGOSEngine::setupVideoOpcodes(op);

	op[11] = &AGOSEngine::vc11_clearPathFinder;
	op[17] = &AGOSEngine::vc17_setPathfinderItem;
	op[22] = &AGOSEngine::vc22_setPalette;
	op[32] = &AGOSEngine::vc32_copyVar;
	op[37] = &AGOSEngine::vc37_addToSpriteY;
	op[48] = &AGOSEngine::vc48_setPathFinder;
	op[59] = &AGOSEngine::vc59_ifSpeech;
	op[60] = &AGOSEngine::vc60_stopAnimation;
	op[61] = &AGOSEngine::vc61_setMaskImage;
	op[62] = &AGOSEngine::vc62_fastFadeOut;
	op[63] = &AGOSEngine::vc63_fastFadeIn;
}

void AGOSEngine::vc11_clearPathFinder() {
	memset(&_pathFindArray, 0, sizeof(_pathFindArray));
}

void AGOSEngine::vc17_setPathfinderItem() {
	uint16 a = vcReadNextWord();
	_pathFindArray[a - 1] = (const uint16 *)_vcPtr;

	int end = (getGameType() == GType_FF || getGameType() == GType_PP) ? 9999 : 999;
	while (readUint16Wrapper(_vcPtr) != end)
		_vcPtr += 4;
	_vcPtr += 2;
}

static const uint8 customPalette[96] = {
	0x00, 0x00, 0x00,
	0x99, 0x22, 0xFF,
	0x66, 0xCC, 0xFF,
	0xFF, 0x99, 0xFF,
	0xFF, 0xFF, 0xFF,
	0x66, 0x44, 0xBB,
	0x77, 0x55, 0xCC,
	0x88, 0x77, 0xCC,
	0xCC, 0xAA, 0xDD,
	0x33, 0x00, 0x09,
	0x66, 0x44, 0xCC,
	0x88, 0x55, 0xCC,
	0xAA, 0x77, 0xEE,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF,
	0x33, 0x00, 0x00,
	0xCC, 0xCC, 0xDD,
	0x88, 0x99, 0xBB,
	0x44, 0x77, 0xAA,
	0x44, 0x44, 0x66,
	0x44, 0x44, 0x00,
	0x44, 0x66, 0x00,
	0x88, 0x99, 0x00,
	0x99, 0x44, 0x00,
	0xBB, 0x44, 0x22,
	0xFF, 0x55, 0x33,
	0xFF, 0x88, 0x88,
	0xFF, 0xBB, 0x33,
	0xFF, 0xFF, 0x77,
};

void AGOSEngine_Simon1::vc22_setPalette() {
	byte *offs, *palptr = 0, *src;
	uint16 a = 0, b, num, palSize = 0;

	a = vcReadNextWord();
	b = vcReadNextWord();

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		num = 256;
		palSize = 768;

		palptr = _displayPalette;
	} else {
		num = a == 0 ? 32 : 16;
		palSize = 96;

		palptr = &_displayPalette[(a * 3 * 16)];
	}

	offs = _curVgaFile1 + 6;
	src = offs + b * palSize;

	do {
		palptr[0] = src[0] * 4;
		palptr[1] = src[1] * 4;
		palptr[2] = src[2] * 4;

		palptr += 3;
		src += 3;
	} while (--num);

	if (getFeatures() & GF_32COLOR) {
		// Custom palette used for verb area
		palptr = &_displayPalette[(13 * 3 * 16)];
		for (uint8 c = 0; c < 32; c++) {
			palptr[0] = customPalette[c * 3 + 0];
			palptr[1] = customPalette[c * 3 + 1];
			palptr[2] = customPalette[c * 3 + 2];

			palptr += 3;
		};
	}

	_paletteFlag = 2;
	_vgaSpriteChanged++;
}

void AGOSEngine::vc32_copyVar() {
	uint16 a = vcReadVar(vcReadNextWord());
	vcWriteVar(vcReadNextWord(), a);
}

void AGOSEngine::vc37_addToSpriteY() {
	VgaSprite *vsp = findCurSprite();
	vsp->y += vcReadVar(vcReadNextWord());

	vsp->windowNum |= 0x8000;
	dirtyBackGround();
	_vgaSpriteChanged++;
}

void AGOSEngine::vc45_setSpriteX() {
	VgaSprite *vsp = findCurSprite();
	vsp->x = vcReadVar(vcReadNextWord());

	vsp->windowNum |= 0x8000;
	dirtyBackGround();
	_vgaSpriteChanged++;
}

void AGOSEngine::vc46_setSpriteY() {
	VgaSprite *vsp = findCurSprite();
	vsp->y = vcReadVar(vcReadNextWord());

	vsp->windowNum |= 0x8000;
	dirtyBackGround();
	_vgaSpriteChanged++;
}

void AGOSEngine::vc47_addToVar() {
	uint16 var = vcReadNextWord();
	vcWriteVar(var, vcReadVar(var) + vcReadVar(vcReadNextWord()));
}

void AGOSEngine::vc48_setPathFinder() {
	uint16 a = (uint16)_variableArrayPtr[12];
	const uint16 *p = _pathFindArray[a - 1];

	uint b = (uint16)_variableArray[13];
	p += b * 2 + 1;
	int c = _variableArray[14];

	int step;
	int y1, y2;
	int16 *vp;

	step = 2;
	if (c < 0) {
		c = -c;
		step = -2;
	}

	vp = &_variableArray[20];

	do {
		y2 = readUint16Wrapper(p);
		p += step;
		y1 = readUint16Wrapper(p) - y2;

		vp[0] = y1 / 2;
		vp[1] = y1 - (y1 / 2);

		vp += 2;
	} while (--c);
}

void AGOSEngine::vc59_ifSpeech() {
	if (!_sound->isVoiceActive())
		vcSkipNextInstruction();
}

void AGOSEngine::vc61_setMaskImage() {
	VgaSprite *vsp = findCurSprite();

	vsp->image = vcReadVarOrWord();
	vsp->x += vcReadNextWord();
	vsp->y += vcReadNextWord();
	vsp->flags = kDFMasked | kDFSkipStoreBG;

	vsp->windowNum |= 0x8000;
	dirtyBackGround();
	_vgaSpriteChanged++;
}

} // End of namespace AGOS
