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

#include "common/events.h"
#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/transitions.h"

namespace Sci {

//#define DISABLE_TRANSITIONS	// uncomment to disable room transitions (for development only! helps in testing games quickly)

GfxTransitions::GfxTransitions(GfxScreen *screen, GfxPalette *palette)
	: _screen(screen), _palette(palette) {
	init();
}

GfxTransitions::~GfxTransitions() {
	delete[] _oldScreen;
}

// This table contains a mapping between oldIDs (prior SCI1LATE) and newIDs
static const GfxTransitionTranslateEntry oldTransitionIDs[] = {
	{   0, SCI_TRANSITIONS_VERTICALROLL_FROMCENTER,		false },
	{   1, SCI_TRANSITIONS_HORIZONTALROLL_FROMCENTER,	false },
	{   2, SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT,			false },
	{   3, SCI_TRANSITIONS_STRAIGHT_FROM_LEFT,			false },
	{   4, SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM,		false },
	{   5, SCI_TRANSITIONS_STRAIGHT_FROM_TOP,			false },
	{   6, SCI_TRANSITIONS_DIAGONALROLL_TOCENTER,		false },
	{   7, SCI_TRANSITIONS_DIAGONALROLL_FROMCENTER,		false },
	{   8, SCI_TRANSITIONS_BLOCKS,						false },
	{   9, SCI_TRANSITIONS_VERTICALROLL_TOCENTER,		false },
	{  10, SCI_TRANSITIONS_HORIZONTALROLL_TOCENTER,		false },
	{  11, SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT,			true },
	{  12, SCI_TRANSITIONS_STRAIGHT_FROM_LEFT,			true },
	{  13, SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM,		true },
	{  14, SCI_TRANSITIONS_STRAIGHT_FROM_TOP,			true },
	{  15, SCI_TRANSITIONS_DIAGONALROLL_FROMCENTER,		true },
	{  16, SCI_TRANSITIONS_DIAGONALROLL_TOCENTER,		true },
	{  17, SCI_TRANSITIONS_BLOCKS,						true },
	{  18, SCI_TRANSITIONS_PIXELATION,					false },
	{  27, SCI_TRANSITIONS_PIXELATION	,				true },
	{  30, SCI_TRANSITIONS_FADEPALETTE,					false },
	{  40, SCI_TRANSITIONS_SCROLL_RIGHT,				false },
	{  41, SCI_TRANSITIONS_SCROLL_LEFT,					false },
	{  42, SCI_TRANSITIONS_SCROLL_UP,					false },
	{  43, SCI_TRANSITIONS_SCROLL_DOWN,					false },
	{ 100, SCI_TRANSITIONS_NONE,						false },
	{ 255, 255,											false }
};

// this table defines the blackout-transition that needs to be done prior doing the actual transition
static const GfxTransitionTranslateEntry blackoutTransitionIDs[] = {
	{ SCI_TRANSITIONS_VERTICALROLL_FROMCENTER,			SCI_TRANSITIONS_VERTICALROLL_TOCENTER,		true },
	{ SCI_TRANSITIONS_HORIZONTALROLL_FROMCENTER,		SCI_TRANSITIONS_HORIZONTALROLL_TOCENTER,	true },
	{ SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT,				SCI_TRANSITIONS_STRAIGHT_FROM_LEFT,			true },
	{ SCI_TRANSITIONS_STRAIGHT_FROM_LEFT,				SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT,		true },
	{ SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM,				SCI_TRANSITIONS_STRAIGHT_FROM_TOP,			true },
	{ SCI_TRANSITIONS_STRAIGHT_FROM_TOP,				SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM,		true },
	{ SCI_TRANSITIONS_DIAGONALROLL_FROMCENTER,			SCI_TRANSITIONS_DIAGONALROLL_TOCENTER,		true },
	{ SCI_TRANSITIONS_DIAGONALROLL_TOCENTER,			SCI_TRANSITIONS_DIAGONALROLL_FROMCENTER,	true },
	{ SCI_TRANSITIONS_BLOCKS,							SCI_TRANSITIONS_BLOCKS,						true },
	{ SCI_TRANSITIONS_PIXELATION,						SCI_TRANSITIONS_PIXELATION,					true },
	{ SCI_TRANSITIONS_FADEPALETTE,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_SCROLL_RIGHT,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_SCROLL_LEFT,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_SCROLL_UP,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_SCROLL_DOWN,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_NONE_LONGBOW,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_NONE,								SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_VERTICALROLL_TOCENTER,			SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_HORIZONTALROLL_TOCENTER,			SCI_TRANSITIONS_NONE,						true },
	{ 255,												255,										true }
};

void GfxTransitions::init() {
	_oldScreen = new byte[_screen->getDisplayHeight() * _screen->getDisplayWidth()];

	if (getSciVersion() >= SCI_VERSION_1_LATE)
		_translationTable = NULL;
	else
		_translationTable = oldTransitionIDs;

	// setup default transition
	_number = SCI_TRANSITIONS_HORIZONTALROLL_FROMCENTER;
	_blackoutFlag = false;
}

void GfxTransitions::setup(int16 number, bool blackoutFlag) {
	if (number != -1) {
#ifndef DISABLE_TRANSITIONS
		_number = number;
#else
		_number = SCI_TRANSITIONS_NONE;
#endif
		_blackoutFlag = blackoutFlag;
		debugC(kDebugLevelGraphics, "Transition %d, blackout %d", number, blackoutFlag);
	}
}

bool GfxTransitions::doCreateFrame(uint32 shouldBeAtMsec) {
	uint32 msecPos = g_system->getMillis() - _transitionStartTime;

	if (shouldBeAtMsec > msecPos)
		return true;
	return false;
}

void GfxTransitions::updateScreenAndWait(uint32 shouldBeAtMsec) {
	Common::Event ev;

	while (g_system->getEventManager()->pollEvent(ev)) {}	// discard all events

	g_system->updateScreen();
	// if we have still some time left, delay accordingly
	uint32 msecPos = g_system->getMillis() - _transitionStartTime;
	if (shouldBeAtMsec > msecPos)
		g_system->delayMillis(shouldBeAtMsec - msecPos);
}

// will translate a number and return corresponding translationEntry
const GfxTransitionTranslateEntry *GfxTransitions::translateNumber (int16 number, const GfxTransitionTranslateEntry *tablePtr) {
	while (1) {
		if (tablePtr->orgId == 255)
			return NULL;
		if (tablePtr->orgId == number)
			return tablePtr;
		tablePtr++;
	}
}

void GfxTransitions::doit(Common::Rect picRect) {
	const GfxTransitionTranslateEntry *translationEntry = _translationTable;

	_picRect = picRect;

	if (_translationTable) {
		// We need to translate the ID
		translationEntry = translateNumber(_number, _translationTable);
		if (translationEntry) {
			_number = translationEntry->newId;
			_blackoutFlag = translationEntry->blackoutFlag;
		} else {
			warning("Transitions: old ID %d not supported", _number);
			_number = SCI_TRANSITIONS_NONE;
			_blackoutFlag = false;
		}
	}

	if (_blackoutFlag) {
		// We need to find out what transition we are supposed to use for
		// blackout
		translationEntry = translateNumber(_number, blackoutTransitionIDs);
		if (translationEntry) {
			doTransition(translationEntry->newId, true);
		} else {
			warning("Transitions: ID %d not listed in blackoutTransitionIDs", _number);
		}
	}

	_palette->palVaryPrepareForTransition();

	// Now we do the actual transition to the new screen
	doTransition(_number, false);

	_screen->_picNotValid = 0;
}

// This may get called twice, if blackoutFlag is set. It will get once called
// with blackoutFlag set and another time with no blackoutFlag.
void GfxTransitions::doTransition(int16 number, bool blackoutFlag) {
	if (number != SCI_TRANSITIONS_FADEPALETTE) {
		setNewPalette(blackoutFlag);
	}

	_transitionStartTime = g_system->getMillis();
	switch (number) {
	case SCI_TRANSITIONS_VERTICALROLL_FROMCENTER:
		verticalRollFromCenter(blackoutFlag);
		break;
	case SCI_TRANSITIONS_VERTICALROLL_TOCENTER:
		verticalRollToCenter(blackoutFlag);
		break;
	case SCI_TRANSITIONS_HORIZONTALROLL_FROMCENTER:
		horizontalRollFromCenter(blackoutFlag);
		break;
	case SCI_TRANSITIONS_HORIZONTALROLL_TOCENTER:
		horizontalRollToCenter(blackoutFlag);
		break;
	case SCI_TRANSITIONS_DIAGONALROLL_TOCENTER:
		diagonalRollToCenter(blackoutFlag);
		break;
	case SCI_TRANSITIONS_DIAGONALROLL_FROMCENTER:
		diagonalRollFromCenter(blackoutFlag);
		break;

	case SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT:
	case SCI_TRANSITIONS_STRAIGHT_FROM_LEFT:
	case SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM:
	case SCI_TRANSITIONS_STRAIGHT_FROM_TOP:
		straight(number, blackoutFlag);
		break;

	case SCI_TRANSITIONS_PIXELATION:
		pixelation(blackoutFlag);
		break;

	case SCI_TRANSITIONS_BLOCKS:
		blocks(blackoutFlag);
		break;

	case SCI_TRANSITIONS_FADEPALETTE:
		if (!blackoutFlag) {
			fadeOut(); setNewScreen(blackoutFlag); fadeIn();
		}
		break;

	case SCI_TRANSITIONS_SCROLL_RIGHT:
	case SCI_TRANSITIONS_SCROLL_LEFT:
	case SCI_TRANSITIONS_SCROLL_UP:
	case SCI_TRANSITIONS_SCROLL_DOWN:
		scroll(number);
		break;

	case SCI_TRANSITIONS_NONE_LONGBOW:
	case SCI_TRANSITIONS_NONE:
		setNewScreen(blackoutFlag);
		break;

	default:
		warning("Transitions: ID %d not implemented", number);
		setNewScreen(blackoutFlag);
	}
}

void GfxTransitions::setNewPalette(bool blackoutFlag) {
	if (!blackoutFlag)
		_palette->setOnScreen();
}

void GfxTransitions::setNewScreen(bool blackoutFlag) {
	if (!blackoutFlag) {
		_screen->copyRectToScreen(_picRect);
		g_system->updateScreen();
	}
}

void GfxTransitions::copyRectToScreen(const Common::Rect rect, bool blackoutFlag) {
	if (!blackoutFlag) {
		_screen->copyRectToScreen(rect);
	} else {
		Graphics::Surface *surface = g_system->lockScreen();
		if (!_screen->getUpscaledHires()) {
			surface->fillRect(rect, 0);
		} else {
			Common::Rect upscaledRect = rect;
			_screen->adjustToUpscaledCoordinates(upscaledRect.top, upscaledRect.left);
			_screen->adjustToUpscaledCoordinates(upscaledRect.bottom, upscaledRect.right);
			surface->fillRect(upscaledRect, 0);
		}
		g_system->unlockScreen();
	}
}

// Note: don't do too many steps in here, otherwise cpu will crap out because of
// the load
void GfxTransitions::fadeOut() {
	byte oldPalette[3 * 256], workPalette[3 * 256];
	int16 stepNr, colorNr;
	// Sierra did not fade in/out color 255 for sci1.1, but they used it in
	//  several pictures (e.g. qfg3 demo/intro), so the fading looked weird
	int16 tillColorNr = getSciVersion() >= SCI_VERSION_1_1 ? 255 : 254;

	g_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);

	for (stepNr = 100; stepNr >= 0; stepNr -= 10) {
		for (colorNr = 1; colorNr <= tillColorNr; colorNr++) {
			if (_palette->colorIsFromMacClut(colorNr)) {
				workPalette[colorNr * 3 + 0] = oldPalette[colorNr * 3];
				workPalette[colorNr * 3 + 1] = oldPalette[colorNr * 3 + 1];
				workPalette[colorNr * 3 + 2] = oldPalette[colorNr * 3 + 2];
			} else {
				workPalette[colorNr * 3 + 0] = oldPalette[colorNr * 3] * stepNr / 100;
				workPalette[colorNr * 3 + 1] = oldPalette[colorNr * 3 + 1] * stepNr / 100;
				workPalette[colorNr * 3 + 2] = oldPalette[colorNr * 3 + 2] * stepNr / 100;
			}
		}
		g_system->getPaletteManager()->setPalette(workPalette + 3, 1, tillColorNr);
		g_sci->getEngineState()->wait(2);
	}
}

// Note: don't do too many steps in here, otherwise cpu will crap out because of
// the load
void GfxTransitions::fadeIn() {
	int16 stepNr;
	// Sierra did not fade in/out color 255 for sci1.1, but they used it in
	//  several pictures (e.g. qfg3 demo/intro), so the fading looked weird
	int16 tillColorNr = getSciVersion() >= SCI_VERSION_1_1 ? 255 : 254;

	for (stepNr = 0; stepNr <= 100; stepNr += 10) {
		_palette->kernelSetIntensity(1, tillColorNr + 1, stepNr, true);
		g_sci->getEngineState()->wait(2);
	}
}

// Pixelates the new picture over the old one - works against the whole screen.
// TODO: it seems this needs to get applied on _picRect only if possible
void GfxTransitions::pixelation(bool blackoutFlag) {
	uint16 mask = 0x40, stepNr = 0;
	Common::Rect pixelRect;
	uint32 msecCount = 0;

	do {
		mask = (mask & 1) ? (mask >> 1) ^ 0xB400 : mask >> 1;
		if (mask >= _screen->getWidth() * _screen->getHeight())
			continue;
		pixelRect.left = mask % _screen->getWidth(); pixelRect.right = pixelRect.left + 1;
		pixelRect.top = mask / _screen->getWidth();	pixelRect.bottom = pixelRect.top + 1;
		pixelRect.clip(_picRect);
		if (!pixelRect.isEmpty())
			copyRectToScreen(pixelRect, blackoutFlag);
		if ((stepNr & 0x3FF) == 0) {
			msecCount += 9;
			updateScreenAndWait(msecCount);
		}
		stepNr++;
	} while (mask != 0x40);
}

// Like pixelation but uses 8x8 blocks - works against the whole screen.
// TODO: it seems this needs to get applied on _picRect only if possible
void GfxTransitions::blocks(bool blackoutFlag) {
	uint16 mask = 0x40, stepNr = 0;
	Common::Rect blockRect;
	uint32 msecCount = 0;

	do {
		mask = (mask & 1) ? (mask >> 1) ^ 0x240 : mask >> 1;
		if (mask >= 40 * 25)
			continue;
		blockRect.left = (mask % 40) << 3; blockRect.right = blockRect.left + 8;
		blockRect.top = (mask / 40) << 3; blockRect.bottom = blockRect.top + 8;
		blockRect.clip(_picRect);
		if (!blockRect.isEmpty())
			copyRectToScreen(blockRect, blackoutFlag);
		if ((stepNr & 7) == 0) {
			msecCount += 5;
			updateScreenAndWait(msecCount);
		}
		stepNr++;
	} while (mask != 0x40);
}

// Directly shows new screen starting up/down/left/right and going to the
// opposite direction - works on _picRect area only
void GfxTransitions::straight(int16 number, bool blackoutFlag) {
	int16 stepNr = 0;
	Common::Rect newScreenRect = _picRect;
	uint32 msecCount = 0;

	switch (number) {
	case SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT:
		newScreenRect.left = newScreenRect.right - 1;
		while (newScreenRect.left >= _picRect.left) {
			copyRectToScreen(newScreenRect, blackoutFlag);
			if ((stepNr & 1) == 0) {
				msecCount += 2;
				updateScreenAndWait(msecCount);
			}
			stepNr++;
			newScreenRect.translate(-1, 0);
		}
		break;

	case SCI_TRANSITIONS_STRAIGHT_FROM_LEFT:
		newScreenRect.right = newScreenRect.left + 1;
		while (newScreenRect.right <= _picRect.right) {
			copyRectToScreen(newScreenRect, blackoutFlag);
			if ((stepNr & 1) == 0) {
				msecCount += 2;
				updateScreenAndWait(msecCount);
			}
			stepNr++;
			newScreenRect.translate(1, 0);
		}
		break;

	case SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM:
		newScreenRect.top = newScreenRect.bottom - 1;
		while (newScreenRect.top >= _picRect.top) {
			copyRectToScreen(newScreenRect, blackoutFlag);
			msecCount += 4;
			updateScreenAndWait(msecCount);
			stepNr++;
			newScreenRect.translate(0, -1);
		}
		break;

	case SCI_TRANSITIONS_STRAIGHT_FROM_TOP:
		newScreenRect.bottom = newScreenRect.top + 1;
		while (newScreenRect.bottom <= _picRect.bottom) {
			copyRectToScreen(newScreenRect, blackoutFlag);
			msecCount += 4;
			updateScreenAndWait(msecCount);
			stepNr++;
			newScreenRect.translate(0, 1);
		}
		break;
	}
}

void GfxTransitions::scrollCopyOldToScreen(Common::Rect screenRect, int16 x, int16 y) {
	byte *oldScreenPtr = _oldScreen;
	int16 screenWidth = _screen->getDisplayWidth();
	if (_screen->getUpscaledHires()) {
		_screen->adjustToUpscaledCoordinates(screenRect.top, screenRect.left);
		_screen->adjustToUpscaledCoordinates(screenRect.bottom, screenRect.right);
		_screen->adjustToUpscaledCoordinates(y, x);
	}
	oldScreenPtr += screenRect.left + screenRect.top * screenWidth;
	g_system->copyRectToScreen(oldScreenPtr, screenWidth, x, y, screenRect.width(), screenRect.height());
}

// Scroll old screen (up/down/left/right) and insert new screen that way - works
// on _picRect area only.
void GfxTransitions::scroll(int16 number) {
	int16 stepNr = 0;
	Common::Rect oldMoveRect = _picRect;
	Common::Rect oldScreenRect = _picRect;
	Common::Rect newMoveRect = _picRect;
	Common::Rect newScreenRect = _picRect;
	uint32 msecCount = 0;

	_screen->copyFromScreen(_oldScreen);

	switch (number) {
	case SCI_TRANSITIONS_SCROLL_LEFT:
		newScreenRect.right = newScreenRect.left;
		newMoveRect.left = newMoveRect.right;
		while (oldMoveRect.left < oldMoveRect.right) {
			oldMoveRect.right--; oldScreenRect.left++;
			newScreenRect.right++; newMoveRect.left--;
			if ((stepNr & 1) == 0) {
				msecCount += 5;
				if (doCreateFrame(msecCount)) {
					if (oldMoveRect.right > oldMoveRect.left)
						scrollCopyOldToScreen(oldScreenRect, oldMoveRect.left, oldMoveRect.top);
					_screen->copyRectToScreen(newScreenRect, newMoveRect.left, newMoveRect.top);
					updateScreenAndWait(msecCount);
				}
			}
			stepNr++;
		}
		break;

	case SCI_TRANSITIONS_SCROLL_RIGHT:
		newScreenRect.left = newScreenRect.right;
		while (oldMoveRect.left < oldMoveRect.right) {
			oldMoveRect.left++; oldScreenRect.right--;
			newScreenRect.left--;
			if ((stepNr & 1) == 0) {
				msecCount += 5;
				if (doCreateFrame(msecCount)) {
					if (oldMoveRect.right > oldMoveRect.left)
						scrollCopyOldToScreen(oldScreenRect, oldMoveRect.left, oldMoveRect.top);
					_screen->copyRectToScreen(newScreenRect, newMoveRect.left, newMoveRect.top);
					updateScreenAndWait(msecCount);
				}
			}
			stepNr++;
		}
		break;

	case SCI_TRANSITIONS_SCROLL_UP:
		newScreenRect.bottom = newScreenRect.top;
		newMoveRect.top = newMoveRect.bottom;
		while (oldMoveRect.top < oldMoveRect.bottom) {
			oldMoveRect.top++; oldScreenRect.top++;
			newScreenRect.bottom++;	newMoveRect.top--;

			msecCount += 5;
			if (doCreateFrame(msecCount)) {
				if (oldMoveRect.top < oldMoveRect.bottom)
					scrollCopyOldToScreen(oldScreenRect, _picRect.left, _picRect.top);
				_screen->copyRectToScreen(newScreenRect, newMoveRect.left, newMoveRect.top);
				updateScreenAndWait(msecCount);
			}
		}
		break;

	case SCI_TRANSITIONS_SCROLL_DOWN:
		newScreenRect.top = newScreenRect.bottom;
		while (oldMoveRect.top < oldMoveRect.bottom) {
			oldMoveRect.top++; oldScreenRect.bottom--;
			newScreenRect.top--;

			msecCount += 5;
			if (doCreateFrame(msecCount)) {
				if (oldMoveRect.top < oldMoveRect.bottom)
					scrollCopyOldToScreen(oldScreenRect, oldMoveRect.left, oldMoveRect.top);
				_screen->copyRectToScreen(newScreenRect, _picRect.left, _picRect.top);
				updateScreenAndWait(msecCount);
			}
		}
		break;
	}

	// Copy over final position just in case
	_screen->copyRectToScreen(newScreenRect);
	g_system->updateScreen();
}

// Vertically displays new screen starting from center - works on _picRect area
// only
void GfxTransitions::verticalRollFromCenter(bool blackoutFlag) {
	Common::Rect leftRect = Common::Rect(_picRect.left + (_picRect.width() / 2) -1, _picRect.top, _picRect.left + (_picRect.width() / 2), _picRect.bottom);
	Common::Rect rightRect = Common::Rect(leftRect.right, _picRect.top, leftRect.right + 1, _picRect.bottom);
	uint32 msecCount = 0;

	while ((leftRect.left >= _picRect.left) || (rightRect.right <= _picRect.right)) {
		if (leftRect.left < _picRect.left)
			leftRect.translate(1, 0);
		if (rightRect.right > _picRect.right)
			rightRect.translate(-1, 0);
		copyRectToScreen(leftRect, blackoutFlag); leftRect.translate(-1, 0);
		copyRectToScreen(rightRect, blackoutFlag); rightRect.translate(1, 0);
		msecCount += 3;
		updateScreenAndWait(msecCount);
	}
}

// Vertically displays new screen starting from edges - works on _picRect area
// only
void GfxTransitions::verticalRollToCenter(bool blackoutFlag) {
	Common::Rect leftRect = Common::Rect(_picRect.left, _picRect.top, _picRect.left + 1, _picRect.bottom);
	Common::Rect rightRect = Common::Rect(_picRect.right - 1, _picRect.top, _picRect.right, _picRect.bottom);
	uint32 msecCount = 0;

	while (leftRect.left < rightRect.right) {
		copyRectToScreen(leftRect, blackoutFlag); leftRect.translate(1, 0);
		copyRectToScreen(rightRect, blackoutFlag); rightRect.translate(-1, 0);
		msecCount += 3;
		updateScreenAndWait(msecCount);
	}
}

// Horizontally displays new screen starting from center - works on _picRect
// area only
void GfxTransitions::horizontalRollFromCenter(bool blackoutFlag) {
	Common::Rect upperRect = Common::Rect(_picRect.left, _picRect.top + (_picRect.height() / 2) - 1, _picRect.right, _picRect.top + (_picRect.height() / 2));
	Common::Rect lowerRect = Common::Rect(upperRect.left, upperRect.bottom, upperRect.right, upperRect.bottom + 1);
	uint32 msecCount = 0;

	while ((upperRect.top >= _picRect.top) || (lowerRect.bottom <= _picRect.bottom)) {
		if (upperRect.top < _picRect.top)
			upperRect.translate(0, 1);
		if (lowerRect.bottom > _picRect.bottom)
			lowerRect.translate(0, -1);
		copyRectToScreen(upperRect, blackoutFlag); upperRect.translate(0, -1);
		copyRectToScreen(lowerRect, blackoutFlag); lowerRect.translate(0, 1);
		msecCount += 4;
		updateScreenAndWait(msecCount);
	}
}

// Horizontally displays new screen starting from upper and lower edge - works
// on _picRect area only
void GfxTransitions::horizontalRollToCenter(bool blackoutFlag) {
	Common::Rect upperRect = Common::Rect(_picRect.left, _picRect.top, _picRect.right, _picRect.top + 1);
	Common::Rect lowerRect = Common::Rect(upperRect.left, _picRect.bottom - 1, upperRect.right, _picRect.bottom);
	uint32 msecCount = 0;

	while (upperRect.top < lowerRect.bottom) {
		copyRectToScreen(upperRect, blackoutFlag); upperRect.translate(0, 1);
		copyRectToScreen(lowerRect, blackoutFlag); lowerRect.translate(0, -1);
		msecCount += 4;
		updateScreenAndWait(msecCount);
	}
}

// Diagonally displays new screen starting from center - works on _picRect area
// only. Assumes that height of rect is larger than width.
void GfxTransitions::diagonalRollFromCenter(bool blackoutFlag) {
	int16 halfHeight = _picRect.height() / 2;
	Common::Rect upperRect(_picRect.left + halfHeight - 2, _picRect.top + halfHeight, _picRect.right - halfHeight + 1, _picRect.top + halfHeight + 1);
	Common::Rect lowerRect(upperRect.left, upperRect.top, upperRect.right, upperRect.bottom);
	Common::Rect leftRect(upperRect.left, upperRect.top, upperRect.left + 1, lowerRect.bottom);
	Common::Rect rightRect(upperRect.right, upperRect.top, upperRect.right + 1, lowerRect.bottom);
	uint32 msecCount = 0;

	while ((upperRect.top >= _picRect.top) || (lowerRect.bottom <= _picRect.bottom)) {
		if (upperRect.top < _picRect.top) {
			upperRect.translate(0, 1); leftRect.top++; rightRect.top++;
		}
		if (lowerRect.bottom > _picRect.bottom) {
			lowerRect.translate(0, -1); leftRect.bottom--; rightRect.bottom--;
		}
		if (leftRect.left < _picRect.left) {
			leftRect.translate(1, 0); upperRect.left++; lowerRect.left++;
		}
		if (rightRect.right > _picRect.right) {
			rightRect.translate(-1, 0); upperRect.right--; lowerRect.right--;
		}
		copyRectToScreen(upperRect, blackoutFlag); upperRect.translate(0, -1); upperRect.left--; upperRect.right++;
		copyRectToScreen(lowerRect, blackoutFlag); lowerRect.translate(0, 1); lowerRect.left--; lowerRect.right++;
		copyRectToScreen(leftRect, blackoutFlag); leftRect.translate(-1, 0);	leftRect.top--; leftRect.bottom++;
		copyRectToScreen(rightRect, blackoutFlag); rightRect.translate(1, 0); rightRect.top--; rightRect.bottom++;
		msecCount += 4;
		updateScreenAndWait(msecCount);
	}
}

// Diagonally displays new screen starting from edges - works on _picRect area
// only. Assumes that height of rect is larger than width.
void GfxTransitions::diagonalRollToCenter(bool blackoutFlag) {
	Common::Rect upperRect(_picRect.left, _picRect.top, _picRect.right, _picRect.top + 1);
	Common::Rect lowerRect(_picRect.left, _picRect.bottom - 1, _picRect.right, _picRect.bottom);
	Common::Rect leftRect(_picRect.left, _picRect.top, _picRect.left + 1, _picRect.bottom);
	Common::Rect rightRect(_picRect.right - 1, _picRect.top, _picRect.right, _picRect.bottom);
	uint32 msecCount = 0;

	while (upperRect.top < lowerRect.bottom) {
		copyRectToScreen(upperRect, blackoutFlag); upperRect.translate(0, 1); upperRect.left++; upperRect.right--;
		copyRectToScreen(lowerRect, blackoutFlag); lowerRect.translate(0, -1); lowerRect.left++; lowerRect.right--;
		copyRectToScreen(leftRect, blackoutFlag); leftRect.translate(1, 0);
		copyRectToScreen(rightRect, blackoutFlag); rightRect.translate(-1, 0);
		msecCount += 4;
		updateScreenAndWait(msecCount);
	}
}

} // End of namespace Sci
