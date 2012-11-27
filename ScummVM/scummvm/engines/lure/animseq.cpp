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

#include "lure/animseq.h"
#include "lure/decode.h"
#include "lure/events.h"
#include "lure/lure.h"
#include "lure/palette.h"
#include "lure/sound.h"
#include "common/endian.h"

namespace Lure {

// delay
// Delays for a given number of milliseconds. If it returns true, it indicates that
// Escape has been pressed, and the introduction should be aborted.

AnimAbortType AnimationSequence::delay(uint32 milliseconds) {
	Events &events = Events::getReference();
	uint32 delayCtr = g_system->getMillis() + milliseconds;

	while (g_system->getMillis() < delayCtr) {
		while (events.pollEvent()) {
			if ((events.type() == Common::EVENT_KEYDOWN) && (events.event().kbd.ascii != 0)) {
				if (events.event().kbd.keycode == Common::KEYCODE_ESCAPE)
					return ABORT_END_INTRO;
				else
					return ABORT_NEXT_SCENE;
			} else if (events.type() == Common::EVENT_LBUTTONDOWN) {
				return ABORT_NEXT_SCENE;
			} else if ((events.type() == Common::EVENT_QUIT) || (events.type() == Common::EVENT_RTL)) {
				return ABORT_END_INTRO;
			} else if (events.type() == Common::EVENT_MAINMENU) {
				return ABORT_NONE;
			}

		}

		uint32 delayAmount = delayCtr - g_system->getMillis();
		if (delayAmount > 10) delayAmount = 10;
		g_system->delayMillis(delayAmount);
	}
	return ABORT_NONE;
}

// egaDecodeFrame
// Decodes a single frame of a EGA animation sequence

void AnimationSequence::egaDecodeFrame(byte *&pPixels) {
	Screen &screen = Screen::getReference();
	byte *screenData = screen.screen_raw();

	// Skip over the list of blocks that are changed
	int numBlocks = *pPixels++;
	pPixels += numBlocks;

	// Loop through the list of same/changed pixel ranges
	int len = *pPixels++;
	int offset = MENUBAR_Y_SIZE * FULL_SCREEN_WIDTH *
		EGA_NUM_LAYERS / EGA_PIXELS_PER_BYTE;
	while ((offset += len) < FULL_SCREEN_WIDTH * FULL_SCREEN_HEIGHT / 2) {

		int repeatLen = *pPixels++;
		if (repeatLen > 0) {
			byte *pDest = screenData + (offset / EGA_NUM_LAYERS) * EGA_PIXELS_PER_BYTE;

			// Copy over the following bytes - each four bytes contain the four
			// planes worth of data for 8 sequential pixels
			while (repeatLen-- > 0) {
				int planeNum = offset % EGA_NUM_LAYERS;
				byte v = *pPixels++;
				for (int bitCtr = 0; bitCtr < 8; ++bitCtr, v <<= 1) {
					if ((v & 0x80) != 0)
						*(pDest + bitCtr) |= 1 << planeNum;
					else
						*(pDest + bitCtr) &= ~(1 << planeNum);
				}

				if ((++offset % EGA_NUM_LAYERS) == 0)
					pDest += EGA_PIXELS_PER_BYTE;
			}
		}

		// Get next skip bytes length
		len = *pPixels++;
	}
}

// vgaDecodeFrame
// Decodes a single frame of a VGA animation sequence

void AnimationSequence::vgaDecodeFrame(byte *&pPixels, byte *&pLines) {
	Screen &screen = Screen::getReference();
	byte *screenData = screen.screen_raw();
	uint16 screenPos = 0;
	uint16 len;

	while (screenPos < SCREEN_SIZE) {
		// Get line length
		len = (uint16) *pLines++;
		if (len == 0) {
			len = READ_LE_UINT16(pLines);
			pLines += 2;
		}

		// Move the splice over
		memcpy(screenData, pPixels, len);
		screenData += len;
		screenPos += len;
		pPixels += len;

		// Get the offset inc amount
		len = (uint16) *pLines++;
		if (len == 0) {
			len = READ_LE_UINT16(pLines);
			pLines += 2;
		}

		screenData += len;
		screenPos += len;
	}
}

AnimationSequence::AnimationSequence(uint16 screenId, Palette &palette,  bool fadeIn, int frameDelay,
					 const AnimSoundSequence *soundList): _screenId(screenId), _palette(palette),
					 _frameDelay(frameDelay), _soundList(soundList) {
	Screen &screen = Screen::getReference();
	PictureDecoder decoder;
	Disk &d = Disk::getReference();

	// Get the data and decode it. Note that VGA decompression is used
	// even if the decompressed contents is actually EGA data
	MemoryBlock *data = d.getEntry(_screenId);
	_decodedData = decoder.vgaDecode(data, MAX_ANIM_DECODER_BUFFER_SIZE);
	delete data;

	_isEGA = LureEngine::getReference().isEGA();
	if (_isEGA) {
		// Setup for EGA animation
		_lineRefs = NULL;

		// Reset the palette and clear the screen for EGA decoding
		screen.setPaletteEmpty(RES_PALETTE_ENTRIES);
		screen.screen().empty();

		// Load the screen - each four bytes contain the four planes
		// worth of data for 8 sequential pixels
		byte *pSrc = _decodedData->data();
		byte *pDest = screen.screen().data().data() +
			(FULL_SCREEN_WIDTH * MENUBAR_Y_SIZE);

		for (int ctr = 0; ctr < FULL_SCREEN_WIDTH * (FULL_SCREEN_HEIGHT -
				MENUBAR_Y_SIZE) / 8; ++ctr, pDest += EGA_PIXELS_PER_BYTE) {
			for (int planeCtr = 0; planeCtr < EGA_NUM_LAYERS; ++planeCtr, ++pSrc) {
				byte v = *pSrc;
				for (int bitCtr = 0; bitCtr < 8; ++bitCtr, v <<= 1) {
					if ((v & 0x80) != 0)
					*(pDest + bitCtr) |= 1 << planeCtr;
				}
			}
		}

		screen.update();
		screen.setPalette(&_palette, 0, _palette.numEntries());

		// Set pointers for animation
		_pPixels = pSrc;
		_pPixelsEnd = _decodedData->data() + _decodedData->size() - 1;
		_pLines = NULL;
		_pLinesEnd = NULL;

	} else {
		// Setup for VGA animation
		_lineRefs = d.getEntry(_screenId + 1);

		// Reset the palette and set the initial starting screen
		screen.setPaletteEmpty(RES_PALETTE_ENTRIES);
		screen.screen().data().copyFrom(_decodedData, 0, 0, FULL_SCREEN_HEIGHT * FULL_SCREEN_WIDTH);
		screen.update();

		// Set the palette
		if (fadeIn)	screen.paletteFadeIn(&_palette);
		else screen.setPalette(&_palette, 0, _palette.numEntries());

		// Set up frame pointers
		_pPixels = _decodedData->data() + SCREEN_SIZE;
		_pPixelsEnd = _decodedData->data() + _decodedData->size() - 1;
		_pLines = _lineRefs->data();
		_pLinesEnd = _lineRefs->data() + _lineRefs->size() - 1;
	}
}

AnimationSequence::~AnimationSequence() {
	delete _lineRefs;
	delete _decodedData;

	// Renable GMM saving/loading now that the animation is done
	LureEngine::getReference()._saveLoadAllowed = true;
}

// show
// Main method for displaying the animation

AnimAbortType AnimationSequence::show() {
	Screen &screen = Screen::getReference();
	AnimAbortType result;
	const AnimSoundSequence *soundFrame = _soundList;
	int frameCtr = 0;

	// Disable GMM saving/loading whilst animation is running
	LureEngine::getReference()._saveLoadAllowed = false;

	// Loop through displaying the animations
	while (_pPixels < _pPixelsEnd) {
		if ((soundFrame != NULL) && (frameCtr == 0))
			Sound.musicInterface_Play(
				Sound.isRoland() ? soundFrame->rolandSoundId : soundFrame->adlibSoundId,
				soundFrame->channelNum);

		if (_isEGA)
			egaDecodeFrame(_pPixels);
		else {
			if (_pLines >= _pLinesEnd) break;
			vgaDecodeFrame(_pPixels, _pLines);
		}

		// Make the decoded frame visible
		screen.update();

		result = delay(_frameDelay * 1000 / 50);
		if (result != ABORT_NONE) return result;

		if ((soundFrame != NULL) && (++frameCtr == soundFrame->numFrames)) {
			frameCtr = 0;
			++soundFrame;
			if (soundFrame->numFrames == 0) soundFrame = NULL;
		}
	}

	return ABORT_NONE;
}

bool AnimationSequence::step() {
	Screen &screen = Screen::getReference();
	if (_pPixels >= _pPixelsEnd) return false;

	if (_isEGA)
		egaDecodeFrame(_pPixels);
	else {
		if (_pLines >= _pLinesEnd) return false;
		vgaDecodeFrame(_pPixels, _pLines);
	}

	// Make the decoded frame visible
	screen.update();
	screen.setPalette(&_palette);

	return true;
}

} // End of namespace Lure
