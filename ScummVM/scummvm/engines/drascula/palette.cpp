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

#include "graphics/palette.h"

#include "drascula/drascula.h"

namespace Drascula {

static const char colorTable[][3] = {
	{    0,    0,    0 }, { 0x10, 0x3E, 0x28 },
	{    0,    0,    0 },	// unused
	{ 0x16, 0x3F, 0x16 }, { 0x09, 0x3F, 0x12 },
	{ 0x3F, 0x3F, 0x15 },
	{    0,    0,    0 },	// unused
	{ 0x38,    0,    0 }, { 0x3F, 0x27, 0x0B },
	{ 0x2A,    0, 0x2A }, { 0x30, 0x30, 0x30 },
	{   98,   91,  100 }
};

void DrasculaEngine::setRGB(byte *pal, int colorCount) {
	int x, cnt = 0;

	for (x = 0; x < colorCount; x++) {
		gamePalette[x][0] = pal[cnt++] / 4;
		gamePalette[x][1] = pal[cnt++] / 4;
		gamePalette[x][2] = pal[cnt++] / 4;
	}
	setPalette((byte *)&gamePalette);
}

void DrasculaEngine::black() {
	int color, component;
	DacPalette256 blackPalette;

	for (color = 0; color < 256; color++)
		for (component = 0; component < 3; component++)
			blackPalette[color][component] = 0;

	blackPalette[254][0] = 0x3F;
	blackPalette[254][1] = 0x3F;
	blackPalette[254][2] = 0x15;

	setPalette((byte *)&blackPalette);
}

void DrasculaEngine::setPalette(byte *PalBuf) {
	byte pal[256 * 3];

	for (int i = 0; i < 3 * 256; ++i) {
		pal[i] = PalBuf[i] * 4;
	}

	_system->getPaletteManager()->setPalette(pal, 0, 256);
	_system->updateScreen();
}

void DrasculaEngine::color_abc(int cl) {
	_color = cl;

	for (int i = 0; i <= 2; i++)
		gamePalette[254][i] = colorTable[cl][i];

	setPalette((byte *)&gamePalette);
}

signed char DrasculaEngine::adjustToVGA(signed char value) {
	return (value & 0x3F) * (value > 0);
}

void DrasculaEngine::fadeToBlack(int fadeSpeed) {
	signed char fade;
	unsigned int color, component;

	DacPalette256 palFade;

	for (fade = 63; fade >= 0; fade--) {
		for (color = 0; color < 256; color++) {
			for (component = 0; component < 3; component++) {
				palFade[color][component] = adjustToVGA(gamePalette[color][component] - 63 + fade);
			}
		}
		pause(fadeSpeed);

		setPalette((byte *)&palFade);

		updateEvents();
	}
}

void DrasculaEngine::fadeFromBlack(int fadeSpeed) {
	signed char fade;
	unsigned int color, component;

	DacPalette256 palFade;

	for (fade = 0; fade < 64; fade++) {
		for (color = 0; color < 256; color++) {
			for (component = 0; component < 3; component++) {
				palFade[color][component] = adjustToVGA(gamePalette[color][component] - 63 + fade);
			}
		}
		pause(fadeSpeed);

		setPalette((byte *)&palFade);

		updateEvents();
	}
}

void DrasculaEngine::assignPalette(DacPalette256 pal) {
	int color, component;

	for (color = 235; color < 253; color++)
		for (component = 0; component < 3; component++)
			pal[color][component] = gamePalette[color][component];
}

void DrasculaEngine::setDefaultPalette(DacPalette256 pal) {
	int color, component;

	for (color = 235; color < 253; color++) {
		for (component = 0; component < 3; component++) {
			gamePalette[color][component] = pal[color][component];
		}
	}
	setPalette((byte *)&gamePalette);
}

void DrasculaEngine::setPaletteBase(int darkness) {
	signed char fade;
	unsigned int color, component;

	for (fade = darkness; fade >= 0; fade--) {
		for (color = 235; color < 253; color++) {
			for (component = 0; component < 3; component++)
				gamePalette[color][component] = adjustToVGA(gamePalette[color][component] - 8 + fade);
		}
	}

	setPalette((byte *)&gamePalette);
}

} // End of namespace Drascula
