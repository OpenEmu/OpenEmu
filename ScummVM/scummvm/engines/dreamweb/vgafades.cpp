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

#include "dreamweb/sound.h"
#include "dreamweb/dreamweb.h"

namespace DreamWeb {

void DreamWebEngine::clearStartPal() {
	memset(_startPal, 0, 256 * 3);
}

void DreamWebEngine::clearEndPal() {
	memset(_endPal, 0, 256 * 3);
}

void DreamWebEngine::palToStartPal() {
	memcpy(_startPal, _mainPal, 256 * 3);
}

void DreamWebEngine::endPalToStart() {
	memcpy(_startPal, _endPal, 256 * 3);
}

void DreamWebEngine::startPalToEnd() {
	memcpy(_endPal, _startPal, 256 * 3);
}

void DreamWebEngine::palToEndPal() {
	memcpy(_endPal, _mainPal, 256 * 3);
}

void DreamWebEngine::fadeDOS() {
	return; // FIXME later

	waitForVSync();
	//processEvents will be called from waitForVSync
	uint8 *dst = _startPal;
	getPalette(dst, 0, 64);
	for (int fade = 0; fade < 64; ++fade) {
		for (int c = 0; c < 768; ++c) { //original sources decrement 768 values -> 256 colors
			if (dst[c]) {
				--dst[c];
			}
		}
		setPalette(dst, 0, 64);
		waitForVSync();
	}
}

void DreamWebEngine::doFade() {
	if (_fadeDirection == 0)
		return;

	processEvents();
	uint8 *src = _startPal + 3 * _colourPos;
	setPalette(src, _colourPos, _numToFade);

	_colourPos += _numToFade;
	if (_colourPos == 0)
		fadeCalculation();
}

void DreamWebEngine::fadeCalculation() {
	if (_fadeCount == 0) {
		_fadeDirection = 0;
		return;
	}

	uint8 *startPal = _startPal;
	const uint8 *endPal = _endPal;
	for (size_t i = 0; i < 256 * 3; ++i) {
		uint8 s = startPal[i];
		uint8 e = endPal[i];
		if (s == e)
			continue;
		else if (s > e)
			--startPal[i];
		else {
			if (_fadeCount <= e)
				++startPal[i];
		}
	}
	--_fadeCount;
}

void DreamWebEngine::fadeUpYellows() {
	palToEndPal();
	memset(_endPal + 231 * 3, 0, 8 * 3);
	memset(_endPal + 246 * 3, 0, 1 * 3);
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 128;
	hangOn(128);
}

void DreamWebEngine::fadeUpMonFirst() {
	palToStartPal();
	palToEndPal();
	memset(_startPal + 231 * 3, 0, 8 * 3);
	memset(_startPal + 246 * 3, 0, 1 * 3);
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 128;
	hangOn(64);
	_sound->playChannel1(26);
	hangOn(64);
}


void DreamWebEngine::fadeDownMon() {
	palToStartPal();
	palToEndPal();
	memset(_endPal + 231 * 3, 0, 8 * 3);
	memset(_endPal + 246 * 3, 0, 1 * 3);
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 128;
	hangOn(64);
}

void DreamWebEngine::fadeUpMon() {
	palToStartPal();
	palToEndPal();
	memset(_startPal + 231 * 3, 0, 8 * 3);
	memset(_startPal + 246 * 3, 0, 1 * 3);
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 128;
	hangOn(128);
}

void DreamWebEngine::initialMonCols() {
	palToStartPal();
	memset(_startPal + 230 * 3, 0, 9 * 3);
	memset(_startPal + 246 * 3, 0, 1 * 3);
	processEvents();
	setPalette(_startPal + 230 * 3, 230, 18);
}

void DreamWebEngine::fadeScreenUp() {
	clearStartPal();
	palToEndPal();
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 128;
}

void DreamWebEngine::fadeScreenUps() {
	clearStartPal();
	palToEndPal();
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 64;
}

void DreamWebEngine::fadeScreenUpHalf() {
	endPalToStart();
	palToEndPal();
	_fadeDirection = 1;
	_fadeCount = 31;
	_colourPos = 0;
	_numToFade = 32;
}

void DreamWebEngine::fadeScreenDown() {
	palToStartPal();
	clearEndPal();
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 128;
}

void DreamWebEngine::fadeScreenDowns() {
	palToStartPal();
	clearEndPal();
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 64;
}

void DreamWebEngine::fadeScreenDownHalf() {
	palToStartPal();
	palToEndPal();

	const uint8 *startPal = _startPal;
	uint8 *endPal = _endPal;
	for (int i = 0; i < 256 * 3; ++i) {
		*endPal >>= 1;
		endPal++;
	}

	memcpy(endPal + (56*3), startPal + (56*3), 3*5);
	memcpy(endPal + (77*3), startPal + (77*3), 3*2);

	_fadeDirection = 1;
	_fadeCount = 31;
	_colourPos = 0;
	_numToFade = 32;
}


void DreamWebEngine::clearPalette() {
	_fadeDirection = 0;
	clearStartPal();
	dumpCurrent();
}

// Converts palette to grey scale, summed using formula
// .20xred + .59xGreen + .11xBlue
void DreamWebEngine::greyscaleSum() {
	byte *src = _mainPal;
	byte *dst = _endPal;

	for (int i = 0; i < 256; ++i) {
		const unsigned int r = 20 * *src++;
		const unsigned int g = 59 * *src++;
		const unsigned int b = 11 * *src++;
		const byte grey = (r + b + g) / 100;
		byte tmp;

		tmp = grey;
		//if (tmp != 0)	// FIXME: The assembler code has this check commented out. Bug or feature?
			tmp += _addToRed;
		*dst++ = tmp;

		tmp = grey;
		if (tmp != 0)
			tmp += _addToGreen;
		*dst++ = tmp;

		tmp = grey;
		if (tmp != 0)
			tmp += _addToBlue;
		*dst++ = tmp;
	}
}

void DreamWebEngine::allPalette() {
	memcpy(_startPal, _mainPal, 3 * 256);
	dumpCurrent();
}

void DreamWebEngine::dumpCurrent() {
	uint8 *pal = _startPal;

	waitForVSync();
	processEvents();
	setPalette(pal, 0, 128);

	pal += 128 * 3;

	waitForVSync();
	processEvents();
	setPalette(pal, 128, 128);
}

} // End of namespace DreamWeb
