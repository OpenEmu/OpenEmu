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

#include "lure/screen.h"
#include "lure/luredefs.h"
#include "lure/memory.h"
#include "lure/disk.h"
#include "lure/decode.h"
#include "lure/events.h"

#include "graphics/palette.h"

namespace Lure {

static Screen *int_disk = NULL;

Screen &Screen::getReference() {
	return *int_disk;
}

Screen::Screen(OSystem &system): _system(system),
		_screen(new Surface(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT)),
		_disk(Disk::getReference()),
		_palette(new Palette(GAME_PALETTE_RESOURCE_ID, RGB64)) {
	int_disk = this;
	_screen->empty();
	_system.getPaletteManager()->setPalette(_palette->data(), 0, GAME_COLORS);
}

Screen::~Screen() {
	delete _screen;
	delete _palette;
}

void Screen::setSystemPalette(Palette *p, uint16 start, uint16 num) {
	byte pal[3 * 256];
	assert(start + num <= 256);

	const byte *rawData = p->data();
	for (uint i = 0; i < num; ++i) {
		pal[i * 3 + 0] = rawData[(i + start) * 4 + 0];
		pal[i * 3 + 1] = rawData[(i + start) * 4 + 1];
		pal[i * 3 + 2] = rawData[(i + start) * 4 + 2];
	}

	_system.getPaletteManager()->setPalette(pal, start, num);
}

// setPaletteEmpty
// Defaults the palette to an empty set

void Screen::setPaletteEmpty(int numEntries) {
	Palette emptyPalette(numEntries, NULL, RGB64);
	setSystemPalette(&emptyPalette, 0, numEntries);
	_palette->copyFrom(&emptyPalette);
/*
	delete _palette;
	_palette = new Palette();

	_system.getPaletteManager()->setPalette(_palette->data(), 0, numEntries);
*/
	_system.updateScreen();
}

// setPalette
// Sets the current palette to the passed palette

void Screen::setPalette(Palette *p) {
	_palette->copyFrom(p);
	setSystemPalette(_palette, 0, GAME_COLORS);
	_system.updateScreen();
}

// setPalette
// Variation that allows the specification of a subset of a palette passed in to be copied

void Screen::setPalette(Palette *p, uint16 start, uint16 num) {
	_palette->palette()->copyFrom(p->palette(), start * 4, start * 4, num * 4);
	setSystemPalette(_palette, start, num);
	_system.updateScreen();
}

// paletteFadeIn
// Fades in the palette. For proper operation, the palette should have been
// previously set to empty

void Screen::paletteFadeIn(Palette *p) {
	assert(p->numEntries() <= _palette->numEntries());
	Events &events = Events::getReference();
	bool changed;

	do {
		changed = false;
		byte *pFinal = p->data();
		byte *pCurrent = _palette->data();

		for (int palCtr = 0; palCtr < p->numEntries() * PALETTE_FADE_INC_SIZE; ++palCtr, ++pCurrent, ++pFinal) {
			if (palCtr % PALETTE_FADE_INC_SIZE == (PALETTE_FADE_INC_SIZE - 1)) continue;
			bool isDifferent = *pCurrent < *pFinal;

			if (isDifferent) {
				if ((*pFinal - *pCurrent) < PALETTE_FADE_INC_SIZE)
					*pCurrent = *pFinal;
				else
					*pCurrent += PALETTE_FADE_INC_SIZE;
				changed = true;
			}
		}

		if (changed) {
			setSystemPalette(_palette, 0, p->numEntries());
			_system.updateScreen();
			_system.delayMillis(20);
			while (events.pollEvent())
				;
		}
	} while (changed);
}

// paletteFadeOut
// Fades the screen to black by gradually decreasing the palette colors

void Screen::paletteFadeOut(int numEntries) {
	assert((uint32)numEntries <= _palette->palette()->size());
	Events &events = Events::getReference();
	bool changed;

	do {
		byte *pTemp = _palette->data();
		changed = false;

		for (uint32 palCtr = 0; palCtr < (uint32)(numEntries * PALETTE_FADE_INC_SIZE); ++palCtr, ++pTemp) {
			if (palCtr % PALETTE_FADE_INC_SIZE == (PALETTE_FADE_INC_SIZE - 1))
				continue;
			bool isDifferent = *pTemp > 0;
			if (isDifferent) {
				if (*pTemp < PALETTE_FADE_INC_SIZE) *pTemp = 0;
				else *pTemp -= PALETTE_FADE_INC_SIZE;
				changed = true;
			}
		}

		if (changed) {
			setSystemPalette(_palette, 0, numEntries);
			_system.updateScreen();
			_system.delayMillis(20);
			while (events.pollEvent())
				;
		}
	} while (changed);
}

void Screen::resetPalette() {
	Palette p(GAME_PALETTE_RESOURCE_ID);
	setPalette(&p);
}

void Screen::empty() {
	_screen->empty();
	update();
}

void Screen::update() {
	_system.copyRectToScreen(screen_raw(), FULL_SCREEN_WIDTH, 0, 0, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
	_system.updateScreen();
}

void Screen::updateArea(uint16 x, uint16 y, uint16 w, uint16 h) {
	_system.copyRectToScreen(screen_raw(), FULL_SCREEN_WIDTH, x, y, w, h);
	_system.updateScreen();
}

} // End of namespace Lure
