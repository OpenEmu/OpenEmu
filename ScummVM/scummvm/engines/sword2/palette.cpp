/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/palette.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"
#include "sword2/resman.h"
#include "sword2/screen.h"

namespace Sword2 {

/**
 * Start layer palette fading up
 */

void Screen::startNewPalette() {
	// If the screen is still fading down then wait for black - could
	// happen when everythings cached into a large memory model
	waitForFade();

	byte *screenFile = _vm->_resman->openResource(_thisScreen.background_layer_id);

	// Don't fetch palette match table while using PSX version,
	// because it is not present.
	if (!Sword2Engine::isPsx())
		memcpy(_paletteMatch, _vm->fetchPaletteMatchTable(screenFile), PALTABLESIZE);

	_vm->fetchPalette(screenFile, _palette);
	setPalette(0, 256, _palette, RDPAL_FADE);

	// Indicating that it's a screen palette
	_lastPaletteRes = 0;

	_vm->_resman->closeResource(_thisScreen.background_layer_id);
	fadeUp();
	_thisScreen.new_palette = 0;
}

void Screen::setFullPalette(int32 palRes) {
	// fudge for hut interior
	// - unpausing should restore last palette as normal (could be screen
	// palette or 'dark_palette_13')
	// - but restoring the screen palette after 'dark_palette_13' should
	// now work properly too!

	// "Hut interior" refers to the watchman's hut in Marseille, and this
	// is apparently needed for the palette to be restored properly when
	// you turn the light off. (I didn't even notice the light switch!)

	if (_vm->_logic->readVar(LOCATION) == 13) {
		// unpausing
		if (palRes == -1) {
			// restore whatever palette was last set (screen
			// palette or 'dark_palette_13')
			palRes = _lastPaletteRes;
		}
	} else {
		// check if we're just restoring the current screen palette
		// because we might actually need to use a separate palette
		// file anyway eg. for pausing & unpausing during the eclipse

		// unpausing (fudged for location 13)
		if (palRes == -1) {
			// we really meant '0'
			palRes = 0;
		}

		if (palRes == 0 && _lastPaletteRes)
			palRes = _lastPaletteRes;
	}

	// If non-zero, set palette to this separate palette file. Otherwise,
	// set palette to current screen palette.

	if (palRes) {
		byte *pal = _vm->_resman->openResource(palRes);

		assert(_vm->_resman->fetchType(pal) == PALETTE_FILE);

		pal += ResHeader::size();

		// always set color 0 to black because most background screen
		// palettes have a bright color 0 although it should come out
		// as black in the game!

		_palette[0] = 0;
		_palette[1] = 0;
		_palette[2] = 0;

		for (uint i = 4, j = 3; i < 4 * 256; i += 4, j += 3) {
			_palette[j + 0] = pal[i + 0];
			_palette[j + 1] = pal[i + 1];
			_palette[j + 2] = pal[i + 2];
		}

		setPalette(0, 256, _palette, RDPAL_INSTANT);
		_vm->_resman->closeResource(palRes);
	} else {
		if (_thisScreen.background_layer_id) {
			byte *data = _vm->_resman->openResource(_thisScreen.background_layer_id);

			// Do not fetch palette match table when using PSX version,
			// because it is not present.
			if (!Sword2Engine::isPsx())
				memcpy(_paletteMatch, _vm->fetchPaletteMatchTable(data), PALTABLESIZE);

			_vm->fetchPalette(data, _palette);
			setPalette(0, 256, _palette, RDPAL_INSTANT);
			_vm->_resman->closeResource(_thisScreen.background_layer_id);
		} else
			error("setFullPalette(0) called, but no current screen available");
	}

	if (palRes != CONTROL_PANEL_PALETTE)
		_lastPaletteRes = palRes;
}

/**
 * Matches a color triplet to a palette index.
 * @param r red color component
 * @param g green color component
 * @param b blue color component
 * @return the palette index of the closest matching color in the palette
 */

// FIXME: This used to be inlined - probably a good idea - but the
// linker complained when I tried to use it in sprite.cpp.

uint8 Screen::quickMatch(uint8 r, uint8 g, uint8 b) {
	return _paletteMatch[((int32)(r >> 2) << 12) + ((int32)(g >> 2) << 6) + (b >> 2)];
}

/**
 * Sets the palette.
 * @param startEntry the first color entry to set
 * @param noEntries the number of color entries to set
 * @param colorTable the new color entries
 * @param fadeNow whether to perform the change immediately or delayed
 */

void Screen::setPalette(int16 startEntry, int16 noEntries, byte *colorTable, uint8 fadeNow) {
	assert(noEntries > 0);

	memmove(&_palette[3 * startEntry], colorTable, noEntries * 3);

	if (fadeNow == RDPAL_INSTANT) {
		setSystemPalette(_palette, startEntry, noEntries);
		setNeedFullRedraw();
	}
}

void Screen::dimPalette(bool dim) {
	if (getFadeStatus() != RDFADE_NONE)
		return;

	if (dim != _dimPalette) {
		_dimPalette = dim;
		setSystemPalette(_palette, 0, 256);
		setNeedFullRedraw();
	}
}

/**
 * Fades the palette up from black to the current palette.
 * @param time the time it will take the palette to fade up
 */

int32 Screen::fadeUp(float time) {
	if (getFadeStatus() != RDFADE_BLACK && getFadeStatus() != RDFADE_NONE)
		return RDERR_FADEINCOMPLETE;

	_fadeTotalTime = (int32)(time * 1000);
	_fadeStatus = RDFADE_UP;
	_fadeStartTime = getTick();

	return RD_OK;
}

/**
 * Fades the palette down to black from the current palette.
 * @param time the time it will take the palette to fade down
 */

int32 Screen::fadeDown(float time) {
	if (getFadeStatus() != RDFADE_BLACK && getFadeStatus() != RDFADE_NONE)
		return RDERR_FADEINCOMPLETE;

	_fadeTotalTime = (int32)(time * 1000);
	_fadeStatus = RDFADE_DOWN;
	_fadeStartTime = getTick();

	return RD_OK;
}

/**
 * Get the current fade status
 * @return RDFADE_UP (fading up), RDFADE_DOWN (fading down), RDFADE_NONE
 * (not faded), or RDFADE_BLACK (completely faded down)
 */

uint8 Screen::getFadeStatus() {
	return _fadeStatus;
}

void Screen::waitForFade() {
	while (getFadeStatus() != RDFADE_NONE && getFadeStatus() != RDFADE_BLACK && !_vm->shouldQuit()) {
		updateDisplay();
		_vm->_system->delayMillis(20);
	}
}

void Screen::fadeServer() {
	static int32 previousTime = 0;
	byte fadePalette[256 * 3];
	byte *newPalette = fadePalette;
	int32 currentTime;
	int16 fadeMultiplier;
	int16 i;

	// If we're not in the process of fading, do nothing.
	if (getFadeStatus() != RDFADE_UP && getFadeStatus() != RDFADE_DOWN)
		return;

	// I don't know if this is necessary, but let's limit how often the
	// palette is updated, just to be safe.
	currentTime = getTick();
	if (currentTime - previousTime <= 25)
		return;

	previousTime = currentTime;

	if (getFadeStatus() == RDFADE_UP) {
		if (currentTime >= _fadeStartTime + _fadeTotalTime) {
			_fadeStatus = RDFADE_NONE;
			newPalette = _palette;
		} else {
			fadeMultiplier = (int16)(((int32)(currentTime - _fadeStartTime) * 256) / _fadeTotalTime);
			for (i = 0; i < 256; i++) {
				newPalette[i * 3 + 0] = (_palette[i * 3 + 0] * fadeMultiplier) >> 8;
				newPalette[i * 3 + 1] = (_palette[i * 3 + 1] * fadeMultiplier) >> 8;
				newPalette[i * 3 + 2] = (_palette[i * 3 + 2] * fadeMultiplier) >> 8;
			}
		}
	} else {
		if (currentTime >= _fadeStartTime + _fadeTotalTime) {
			_fadeStatus = RDFADE_BLACK;
			memset(newPalette, 0, sizeof(fadePalette));
		} else {
			fadeMultiplier = (int16)(((int32)(_fadeTotalTime - (currentTime - _fadeStartTime)) * 256) / _fadeTotalTime);
			for (i = 0; i < 256; i++) {
				newPalette[i * 3 + 0] = (_palette[i * 3 + 0] * fadeMultiplier) >> 8;
				newPalette[i * 3 + 1] = (_palette[i * 3 + 1] * fadeMultiplier) >> 8;
				newPalette[i * 3 + 2] = (_palette[i * 3 + 2] * fadeMultiplier) >> 8;
			}
		}
	}

	setSystemPalette(newPalette, 0, 256);
	setNeedFullRedraw();
}

void Screen::setSystemPalette(const byte *colors, uint start, uint num) {
	const byte *palette;

	if (_dimPalette) {
		byte pal[256 * 3];

		for (uint i = start * 3; i < 3 * (start + num); i++)
			pal[i] = colors[i] / 2;

		palette = pal;
	} else
		palette = colors;

	_vm->_system->getPaletteManager()->setPalette(palette, start, num);
}

} // End of namespace Sword2
