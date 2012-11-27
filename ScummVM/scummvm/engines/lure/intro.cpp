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

#include "lure/lure.h"
#include "lure/intro.h"
#include "lure/animseq.h"
#include "lure/events.h"
#include "lure/sound.h"

namespace Lure {

struct AnimRecord {
	uint16 resourceId;
	uint8 paletteIndex;
	uint16 initialPause;
	uint16 endingPause;
	uint8 soundNumber;
};

static const uint16 start_screens[] = {0x18, 0x1A, 0x1E, 0x1C, 0};
static const AnimRecord anim_screens[] = {
	{0x40, 0, 0x35A, 0x2C8, 0x80},		// The kingdom was at peace
	{0x42, 1, 0, 0x5FA, 0x81},			// Cliff overhang
	{0x44, 2, 0, 0, 0x82},				// Siluette in moonlight
	{0x24, 3, 0, 0x328 + 0x24, 0xff},	// Exposition of reaching town
	{0x46, 3, 0, 0, 0x83},				// Skorl approaches
	{0, 0, 0, 0, 0xff}};

// showScreen
// Shows a screen by loading it from the given resource, and then fading it in
// with a palette in the following resource. Returns true if the introduction
// should be aborted

bool Introduction::showScreen(uint16 screenId, uint16 paletteId, uint16 delaySize) {
	Screen &screen = Screen::getReference();
	bool isEGA = LureEngine::getReference().isEGA();
	screen.screen().loadScreen(screenId);
	screen.update();
	Palette p(paletteId);

	if (LureEngine::getReference().shouldQuit()) return true;

	if (isEGA) screen.setPalette(&p);
	else screen.paletteFadeIn(&p);

	bool result = interruptableDelay(delaySize);
	if (LureEngine::getReference().shouldQuit()) return true;

	if (!isEGA)
		screen.paletteFadeOut();

	return result;
}

// interruptableDelay
// Delays for a given number of milliseconds. If it returns true, it indicates that
// the Escape has been pressed to abort whatever sequence is being displayed

bool Introduction::interruptableDelay(uint32 milliseconds) {
	Events &events = Events::getReference();

	if (events.interruptableDelay(milliseconds)) {
		if (events.type() == Common::EVENT_KEYDOWN)
			return events.event().kbd.keycode == 27;
		else if (LureEngine::getReference().shouldQuit())
			return true;
		else if (events.type() == Common::EVENT_LBUTTONDOWN)
			return false;
	}

	return false;
}

// show
// Main method for the introduction sequence

bool Introduction::show() {
	Screen &screen = Screen::getReference();
	bool isEGA = LureEngine::getReference().isEGA();
	screen.setPaletteEmpty();

	// Initial game company and then game screen

	for (int ctr = 0; start_screens[ctr]; ++ctr)
		if (showScreen(start_screens[ctr], start_screens[ctr] + 1, 5000))
			return true;

	PaletteCollection coll(0x32);
	Palette EgaPalette(0x1D);

	// Animated screens

	AnimationSequence *anim;
	bool result;
	uint8 currentSound = 0xff;
	const AnimRecord *curr_anim = anim_screens;
	for (; curr_anim->resourceId; ++curr_anim) {
		// Handle sound selection
		if (curr_anim->soundNumber != 0xff) {
			if (currentSound != 0xff)
				// Stop the previous sound
				Sound.musicInterface_KillAll();

			currentSound = curr_anim->soundNumber;
			Sound.musicInterface_Play(currentSound, 0);
		}

		bool fadeIn = curr_anim == anim_screens;
		anim = new AnimationSequence(curr_anim->resourceId,
			isEGA ? EgaPalette : coll.getPalette(curr_anim->paletteIndex), fadeIn,
			(curr_anim->resourceId == 0x44) ? 4 : 7);

		if (curr_anim->initialPause != 0) {
			if (interruptableDelay(curr_anim->initialPause * 1000 / 50)) {
				delete anim;
				return true;
			}
		}

		result = false;
		switch (anim->show()) {
		case ABORT_NONE:
			if (curr_anim->endingPause != 0) {
				result = interruptableDelay(curr_anim->endingPause * 1000 / 50);
			}
			break;

		case ABORT_END_INTRO:
			result = true;
			break;

		case ABORT_NEXT_SCENE:
			break;
		}
		delete anim;

		if (result) {
			Sound.musicInterface_KillAll();
			return true;
		}
	}

	// Show battle pictures one frame at a time

	result = false;
	anim = new AnimationSequence(0x48, isEGA ? EgaPalette : coll.getPalette(4), false);
	do {
		result = interruptableDelay(2000);
		screen.paletteFadeOut();
		if (!result) result = interruptableDelay(500);
		if (result) break;
	} while (anim->step());
	delete anim;

	if (!result) {
		// Show final introduction animation
		if (!isEGA)
			showScreen(0x22, 0x21, 10000);
		else {
			Palette finalPalette(0x21);
			anim = new AnimationSequence(0x22, finalPalette, false);
			delete anim;
			interruptableDelay(10000);
		}
	}

	Sound.musicInterface_KillAll();
	return false;
}

} // End of namespace Lure
