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

#include "groovie/graphics.h"
#include "groovie/groovie.h"

#include "common/rect.h"
#include "common/system.h"

#include "graphics/palette.h"

namespace Groovie {

GraphicsMan::GraphicsMan(GroovieEngine *vm) :
	_vm(vm), _changed(false), _fading(0) {
	// Create the game surfaces
	_foreground.create(640, 320, _vm->_pixelFormat);
	_background.create(640, 320, _vm->_pixelFormat);
}

GraphicsMan::~GraphicsMan() {
	// Free the game surfaces
	_foreground.free();
	_background.free();
}

void GraphicsMan::update() {
	if (_fading) {
		// Set the start time
		uint32 time = _vm->_system->getMillis() - _fadeStartTime;

		// Scale the time
		int step = (time * 15 << 3) / 1000;
		if (step > 256) {
			step = 256;
		}

		// Apply the current fading
		applyFading(step);

		// Check for the end
		if (step == 256) {
			_fading = 0;

			// Clear the buffer when ending the fade out
			if (_fading == 2)
				_foreground.fillRect(Common::Rect(640, 320), 0);
		}
	}

	// Update the screen if needed and reset the status
	if (_changed) {
		_vm->_system->updateScreen();
		_changed = false;
	}
}

void GraphicsMan::change() {
	_changed = true;
}

void GraphicsMan::mergeFgAndBg() {
	uint32 i;
	byte *countf, *countb;

	countf = (byte *)_foreground.getBasePtr(0, 0);
	countb = (byte *)_background.getBasePtr(0, 0);
	for (i = 640 * 320; i; i--) {
		if (255 == *(countf)) {
			*(countf) = *(countb);
		}
		countf++;
		countb++;
	}
}

void GraphicsMan::updateScreen(Graphics::Surface *source) {
	_vm->_system->copyRectToScreen(source->getBasePtr(0, 0), 640, 0, 80, 640, 320);
	change();
}

bool GraphicsMan::isFading() {
	return _fading;
}

void GraphicsMan::fadeIn(byte *pal) {
	// Set the start time
	_fadeStartTime = _vm->_system->getMillis();

	// Copy the target palette
	memcpy(_paletteFull, pal, 3 * 256);

	// Set the current fading
	_fading = 1;

	// Apply a black palette right now
	applyFading(0);
}

void GraphicsMan::fadeOut() {
	// Set the start time
	_fadeStartTime = _vm->_system->getMillis();

	// Get the current palette
	_vm->_system->getPaletteManager()->grabPalette(_paletteFull, 0, 256);

	// Set the current fading
	_fading = 2;
}

void GraphicsMan::applyFading(int step) {
	// Calculate the fade factor for the given step
	int factorR = 0, factorG = 0, factorB = 0;
	if (_fading == 1) {
		// Fading in
		factorR = (step << 2);
		factorG = (step << 1);
		factorB = step;
		if (factorR > 256) factorR = 256;
		if (factorG > 256) factorG = 256;
		if (factorB > 256) factorB = 256;
	} else if (_fading == 2) {
		// Fading out
		factorR = 256 - step;
		factorG = 256 - (step << 1);
		if (factorR < 0) factorR = 0;
		if (factorG < 0) factorG = 0;
		factorB = factorG;
	}

	// Calculate the new palette
	byte newpal[256 * 3];
	for (int i = 0; i < 256; i++) {
		newpal[(i * 3) + 0] = (_paletteFull[(i * 3) + 0] * factorR) / 256;
		newpal[(i * 3) + 1] = (_paletteFull[(i * 3) + 1] * factorG) / 256;
		newpal[(i * 3) + 2] = (_paletteFull[(i * 3) + 2] * factorB) / 256;
	}

	// Set the screen palette
	_vm->_system->getPaletteManager()->setPalette(newpal, 0, 256);

	// Request a screen update
	change();
}

} // End of Groovie namespace
