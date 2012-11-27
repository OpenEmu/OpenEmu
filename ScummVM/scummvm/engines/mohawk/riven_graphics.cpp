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

#include "mohawk/resource.h"
#include "mohawk/riven.h"
#include "mohawk/riven_graphics.h"

#include "common/system.h"
#include "engines/util.h"

namespace Mohawk {

RivenGraphics::RivenGraphics(MohawkEngine_Riven* vm) : GraphicsManager(), _vm(vm) {
	_bitmapDecoder = new MohawkBitmap();

	// Give me the best you've got!
	initGraphics(608, 436, true, NULL);
	_pixelFormat = _vm->_system->getScreenFormat();

	if (_pixelFormat.bytesPerPixel == 1)
		error("Riven requires greater than 256 colors to run");

	// The actual game graphics only take up the first 392 rows. The inventory
	// occupies the rest of the screen and we don't use the buffer to hold that.
	_mainScreen = new Graphics::Surface();
	_mainScreen->create(608, 392, _pixelFormat);

	_updatesEnabled = true;
	_scheduledTransition = -1;	// no transition
	_dirtyScreen = false;
	_inventoryDrawn = false;

	_creditsImage = 302;
	_creditsPos = 0;
}

RivenGraphics::~RivenGraphics() {
	_mainScreen->free();
	delete _mainScreen;
	delete _bitmapDecoder;
}

MohawkSurface *RivenGraphics::decodeImage(uint16 id) {
	MohawkSurface *surface = _bitmapDecoder->decodeImage(_vm->getResource(ID_TBMP, id));
	surface->convertToTrueColor();
	return surface;
}

void RivenGraphics::copyImageToScreen(uint16 image, uint32 left, uint32 top, uint32 right, uint32 bottom) {
	Graphics::Surface *surface = findImage(image)->getSurface();

	// Clip the width to fit on the screen. Fixes some images.
	if (left + surface->w > 608)
		surface->w = 608 - left;

	for (uint16 i = 0; i < surface->h; i++)
		memcpy(_mainScreen->getBasePtr(left, i + top), surface->getBasePtr(0, i), surface->w * surface->format.bytesPerPixel);

	_dirtyScreen = true;
}

void RivenGraphics::drawPLST(uint16 x) {
	Common::SeekableReadStream* plst = _vm->getResource(ID_PLST, _vm->getCurCard());
	uint16 recordCount = plst->readUint16BE();

	for (uint16 i = 0; i < recordCount; i++) {
		uint16 index = plst->readUint16BE();
		uint16 id = plst->readUint16BE();
		uint16 left = plst->readUint16BE();
		uint16 top = plst->readUint16BE();
		uint16 right = plst->readUint16BE();
		uint16 bottom = plst->readUint16BE();

		// We are also checking here to make sure we haven't drawn the image yet on screen.
		// This fixes problems with drawing PLST 1 twice and some other images twice. PLST
		// 1 is sometimes not called by the scripts, so some cards don't appear if we don't
		// draw PLST 1 each time. This "hack" is here to catch any PLST attempting to draw
		// twice. There should never be a problem with doing it this way.
		if (index == x && !(Common::find(_activatedPLSTs.begin(), _activatedPLSTs.end(), x) != _activatedPLSTs.end())) {
			debug(0, "Drawing image %d", id);
			copyImageToScreen(id, left, top, right, bottom);
			_activatedPLSTs.push_back(x);
			break;
		}
	}

	delete plst;
}

void RivenGraphics::updateScreen(Common::Rect updateRect) {
	if (_updatesEnabled) {
		_vm->runUpdateScreenScript();

		if (_dirtyScreen) {
			_activatedPLSTs.clear();

			// Copy to screen if there's no transition. Otherwise transition. ;)
			if (_scheduledTransition < 0)
				_vm->_system->copyRectToScreen(_mainScreen->getBasePtr(updateRect.left, updateRect.top), _mainScreen->pitch, updateRect.left, updateRect.top, updateRect.width(), updateRect.height());
			else
				runScheduledTransition();

			// Finally, update the screen.
			_vm->_system->updateScreen();
			_dirtyScreen = false;
		}
	}
}

void RivenGraphics::scheduleWaterEffect(uint16 sfxeID) {
	Common::SeekableReadStream *sfxeStream = _vm->getResource(ID_SFXE, sfxeID);

	if (sfxeStream->readUint16BE() != 'SL')
		error ("Unknown sfxe tag");

	// Read in header info
	SFXERecord sfxeRecord;
	sfxeRecord.frameCount = sfxeStream->readUint16BE();
	uint32 offsetTablePosition = sfxeStream->readUint32BE();
	sfxeRecord.rect.left = sfxeStream->readUint16BE();
	sfxeRecord.rect.top = sfxeStream->readUint16BE();
	sfxeRecord.rect.right = sfxeStream->readUint16BE();
	sfxeRecord.rect.bottom = sfxeStream->readUint16BE();
	sfxeRecord.speed = sfxeStream->readUint16BE();
	// Skip the rest of the fields...

	// Read in offsets
	sfxeStream->seek(offsetTablePosition);
	uint32 *frameOffsets = new uint32[sfxeRecord.frameCount];
	for (uint16 i = 0; i < sfxeRecord.frameCount; i++)
		frameOffsets[i] = sfxeStream->readUint32BE();
	sfxeStream->seek(frameOffsets[0]);

	// Read in the scripts
	for (uint16 i = 0; i < sfxeRecord.frameCount; i++)
		sfxeRecord.frameScripts.push_back(sfxeStream->readStream((i == sfxeRecord.frameCount - 1) ? sfxeStream->size() - frameOffsets[i] : frameOffsets[i + 1] - frameOffsets[i]));

	// Set it to the first frame
	sfxeRecord.curFrame = 0;
	sfxeRecord.lastFrameTime = 0;

	delete[] frameOffsets;
	delete sfxeStream;
	_waterEffects.push_back(sfxeRecord);
}

void RivenGraphics::clearWaterEffects() {
	_waterEffects.clear();
}

bool RivenGraphics::runScheduledWaterEffects() {
	// Don't run the effect if it's disabled
	if (_vm->_vars["waterenabled"] == 0)
		return false;

	Graphics::Surface *screen = NULL;

	for (uint16 i = 0; i < _waterEffects.size(); i++) {
		if (_vm->_system->getMillis() > _waterEffects[i].lastFrameTime + 1000 / _waterEffects[i].speed) {
			// Lock the screen!
			if (!screen)
				screen = _vm->_system->lockScreen();

			// Make sure the script is at the starting point
			Common::SeekableReadStream *script = _waterEffects[i].frameScripts[_waterEffects[i].curFrame];
			if (script->pos() != 0)
				script->seek(0);

			// Run script
			uint16 curRow = 0;
			for (uint16 op = script->readUint16BE(); op != 4; op = script->readUint16BE()) {
				if (op == 1) {        // Increment Row
					curRow++;
				} else if (op == 3) { // Copy Pixels
					uint16 dstLeft = script->readUint16BE();
					uint16 srcLeft = script->readUint16BE();
					uint16 srcTop = script->readUint16BE();
					uint16 rowWidth = script->readUint16BE();
					memcpy ((byte *)screen->getBasePtr(dstLeft, curRow + _waterEffects[i].rect.top), (byte *)_mainScreen->getBasePtr(srcLeft, srcTop), rowWidth * _pixelFormat.bytesPerPixel);
				} else if (op != 4) { // End of Script
					error ("Unknown SFXE opcode %d", op);
				}
			}

			// Increment frame
			_waterEffects[i].curFrame++;
			if (_waterEffects[i].curFrame == _waterEffects[i].frameCount)
				_waterEffects[i].curFrame = 0;

			// Set the new time
			_waterEffects[i].lastFrameTime = _vm->_system->getMillis();
		}
	}

	// Unlock the screen if it has been locked and return true to update the screen
	if (screen) {
		_vm->_system->unlockScreen();
		return true;
	}

	return false;
}

void RivenGraphics::scheduleTransition(uint16 id, Common::Rect rect) {
	_scheduledTransition = id;
	_transitionRect = rect;
}

void RivenGraphics::runScheduledTransition() {
	if (_scheduledTransition < 0) // No transition is scheduled
		return;

	// TODO: There's a lot to be done here...

	// Note: Transitions 0-11 are actual transitions, but none are used in-game.
	// There's no point in implementing them if they're not used. These extra
	// transitions were found by hacking scripts.

	switch (_scheduledTransition) {
	case 0:  // Swipe Left
	case 1:  // Swipe Right
	case 2:  // Swipe Up
	case 3:  // Swipe Down
	case 12: // Pan Left
	case 13: // Pan Right
	case 14: // Pan Up
	case 15: // Pan Down
	case 16: // Dissolve
	case 17: // Dissolve (tspit CARD 155)
		break;
	default:
		if (_scheduledTransition >= 4 && _scheduledTransition <= 11)
			error("Found unused transition %d", _scheduledTransition);
		else
			error("Found unknown transition %d", _scheduledTransition);
	}

	// For now, just copy the image to screen without doing any transition.
	_vm->_system->copyRectToScreen(_mainScreen->pixels, _mainScreen->pitch, 0, 0, _mainScreen->w, _mainScreen->h);
	_vm->_system->updateScreen();

	_scheduledTransition = -1; // Clear scheduled transition
}

void RivenGraphics::clearMainScreen() {
	_mainScreen->fillRect(Common::Rect(0, 0, 608, 392), _pixelFormat.RGBToColor(0, 0, 0));
}

void RivenGraphics::fadeToBlack() {
	// The transition speed is forced to best here
	setTransitionSpeed(kRivenTransitionSpeedBest);
	scheduleTransition(16);
	clearMainScreen();
	runScheduledTransition();
}

void RivenGraphics::showInventory() {
	// Don't redraw the inventory
	if (_inventoryDrawn)
		return;

	// Clear the inventory area
	clearInventoryArea();

	// Draw the demo's exit button
	if (_vm->getFeatures() & GF_DEMO) {
		// extras.mhk tBMP 101 contains "EXIT" instead of Atrus' journal in the demo!
		// The demo's extras.mhk contains all the other inventory/marble/credits image
		// but has hacked tBMP 101 with "EXIT". *sigh*
		drawInventoryImage(101, g_demoExitRect);
	} else {
		// We don't want to show the inventory on setup screens or in other journals.
		if (_vm->getCurStack() == aspit)
			return;

		// There are three books and three vars. We have three different
		// combinations. At the start you have just Atrus' journal. Later,
		// you get Catherine's journal and the trap book. Near the end,
		// you lose the trap book and have just the two journals.

		bool hasCathBook = _vm->_vars["acathbook"] != 0;
		bool hasTrapBook = _vm->_vars["atrapbook"] != 0;

		if (!hasCathBook) {
			drawInventoryImage(101, g_atrusJournalRect1);
		} else if (!hasTrapBook) {
			drawInventoryImage(101, g_atrusJournalRect2);
			drawInventoryImage(102, g_cathJournalRect2);
		} else {
			drawInventoryImage(101, g_atrusJournalRect3);
			drawInventoryImage(102, g_cathJournalRect3);
			drawInventoryImage(100, g_trapBookRect3);
		}
	}

	_vm->_system->updateScreen();
	_inventoryDrawn = true;
}

void RivenGraphics::hideInventory() {
	// Don't hide the inventory twice
	if (!_inventoryDrawn)
		return;

	// Clear the area
	clearInventoryArea();

	_inventoryDrawn = false;
}

void RivenGraphics::clearInventoryArea() {
	// Clear the inventory area
	static const Common::Rect inventoryRect = Common::Rect(0, 392, 608, 436);

	// Lock the screen
	Graphics::Surface *screen = _vm->_system->lockScreen();

	// Fill the inventory area with black
	screen->fillRect(inventoryRect, _pixelFormat.RGBToColor(0, 0, 0));

	_vm->_system->unlockScreen();
}

void RivenGraphics::drawInventoryImage(uint16 id, const Common::Rect *rect) {
	MohawkSurface *mhkSurface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, id));
	mhkSurface->convertToTrueColor();
	Graphics::Surface *surface = mhkSurface->getSurface();

	_vm->_system->copyRectToScreen(surface->pixels, surface->pitch, rect->left, rect->top, surface->w, surface->h);

	delete mhkSurface;
}

void RivenGraphics::drawRect(Common::Rect rect, bool active) {
	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	Graphics::Surface *screen = _vm->_system->lockScreen();

	if (active)
		screen->frameRect(rect, _pixelFormat.RGBToColor(0, 255, 0));
	else
		screen->frameRect(rect, _pixelFormat.RGBToColor(255, 0, 0));

	_vm->_system->unlockScreen();
}

void RivenGraphics::drawImageRect(uint16 id, Common::Rect srcRect, Common::Rect dstRect) {
	// Draw tBMP id from srcRect to dstRect
	Graphics::Surface *surface = findImage(id)->getSurface();

	assert(srcRect.width() == dstRect.width() && srcRect.height() == dstRect.height());

	for (uint16 i = 0; i < srcRect.height(); i++)
		memcpy(_mainScreen->getBasePtr(dstRect.left, i + dstRect.top), surface->getBasePtr(srcRect.left, i + srcRect.top), srcRect.width() * surface->format.bytesPerPixel);

	_dirtyScreen = true;
}

void RivenGraphics::drawExtrasImage(uint16 id, Common::Rect dstRect) {
	MohawkSurface *mhkSurface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, id));
	mhkSurface->convertToTrueColor();
	Graphics::Surface *surface = mhkSurface->getSurface();

	assert(dstRect.width() == surface->w);

	for (uint16 i = 0; i < surface->h; i++)
		memcpy(_mainScreen->getBasePtr(dstRect.left, i + dstRect.top), surface->getBasePtr(0, i), surface->pitch);

	delete mhkSurface;
	_dirtyScreen = true;
}

void RivenGraphics::beginCredits() {
	// Clear the old cache
	clearCache();

	// Now cache all the credits images
	for (uint16 i = 302; i <= 320; i++) {
		MohawkSurface *surface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, i));
		surface->convertToTrueColor();
		addImageToCache(i, surface);
	}

	// And clear our screen too
	clearMainScreen();
}

void RivenGraphics::updateCredits() {
	if ((_creditsImage == 303 || _creditsImage == 304) && _creditsPos == 0)
		fadeToBlack();

	if (_creditsImage < 304) {
		// For the first two credit images, they are faded from black to the image and then out again
		scheduleTransition(16);

		Graphics::Surface *frame = findImage(_creditsImage++)->getSurface();

		for (int y = 0; y < frame->h; y++)
			memcpy(_mainScreen->getBasePtr(124, y), frame->getBasePtr(0, y), frame->pitch);

		runScheduledTransition();
	} else {
		// Otheriwse, we're scrolling
		// Move the screen up one row
		memmove(_mainScreen->pixels, _mainScreen->getBasePtr(0, 1), _mainScreen->pitch * (_mainScreen->h - 1));

		// Only update as long as we're not before the last frame
		// Otherwise, we're just moving up a row (which we already did)
		if (_creditsImage <= 320) {
			// Copy the next row to the bottom of the screen
			Graphics::Surface *frame = findImage(_creditsImage)->getSurface();
			memcpy(_mainScreen->getBasePtr(124, _mainScreen->h - 1), frame->getBasePtr(0, _creditsPos), frame->pitch);
			_creditsPos++;

			if (_creditsPos == _mainScreen->h) {
				_creditsImage++;
				_creditsPos = 0;
			}
		}

		// Now flush the new screen
		_vm->_system->copyRectToScreen(_mainScreen->pixels, _mainScreen->pitch, 0, 0, _mainScreen->w, _mainScreen->h);
		_vm->_system->updateScreen();
	}
}

} // End of namespace Mohawk
