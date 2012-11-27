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

void DreamWebEngine::showRain() {
	Common::List<Rain>::iterator i;

	// Do nothing if there's no rain at all
	if (_rainList.empty())
		return;

	const uint8 *frameData = _mainSprites.getFrameData(58);

	for (i = _rainList.begin(); i != _rainList.end(); ++i) {
		Rain &rain = *i;
		uint16 y = rain.y + _mapAdY + _mapYStart;
		uint16 x = rain.x + _mapAdX + _mapXStart;
		uint16 size = rain.size;
		uint16 offset = (rain.w3 - rain.b5) & 511;
		rain.w3 = offset;
		const uint8 *src = frameData + offset;
		uint8 *dst = workspace() + y * 320 + x;
		for (uint16 j = 0; j < size; ++j) {
			uint8 v = src[j];
			if (v != 0)
				*dst = v;
			dst += 320-1; // advance diagonally
		}
	}

	if (_sound->isChannel1Playing())
		return;
	if (_realLocation == 2 && _vars._beenMugged != 1)
		return;
	if (_realLocation == 55)
		return;

	if (randomNumber() >= 1) // play thunder with 1 in 256 chance
		return;

	uint8 soundIndex;
	if (_sound->getChannel0Playing() != 6)
		soundIndex = 4;
	else
		soundIndex = 7;
	_sound->playChannel1(soundIndex);
}

uint8 DreamWebEngine::getBlockOfPixel(uint8 x, uint8 y) {
	uint8 flag, flagEx, type, flagX, flagY;
	checkOne(x + _mapXStart, y + _mapYStart, &flag, &flagEx, &type, &flagX, &flagY);
	if (flag & 1)
		return 0;
	else
		return type;
}

void DreamWebEngine::splitIntoLines(uint8 x, uint8 y) {
	do {
		Rain rain;

		// Look for line start
		while (!getBlockOfPixel(x, y)) {
			--x;
			++y;
			if (x == 0 || y >= _mapYSize)
				return;
		}

		rain.x = x;
		rain.y = y;

		uint8 length = 1;

		// Look for line end
		while (getBlockOfPixel(x, y)) {
			--x;
			++y;
			if (x == 0 || y >= _mapYSize)
				break;
			++length;
		}

		rain.size = length;
		rain.w3 = _rnd.getRandomNumber(65535);
		rain.b5 = _rnd.getRandomNumberRng(4, 7);
		_rainList.push_back(rain);
	} while (x > 0 && y < _mapYSize);
}

struct RainLocation {
	uint8 location;
	uint8 x, y;
	uint8 rainSpacing;
};

static const RainLocation rainLocationList[] = {
	{ 1,44,10,16 },
	{ 4,11,30,14 },
	{ 4,22,30,14 },
	{ 3,33,10,14 },
	{ 10,33,30,14 },
	{ 10,22,30,24 },
	{ 9,22,10,14 },
	{ 2,33,0,14 },
	{ 2,22,0,14 },
	{ 6,11,30,14 },
	{ 7,11,20,18 },
	{ 7,0,20,18 },
	{ 7,0,30,18 },
	{ 55,44,0,14 },
	{ 5,22,30,14 },

	{ 8,0,10,18 },
	{ 8,11,10,18 },
	{ 8,22,10,18 },
	{ 8,33,10,18 },
	{ 8,33,20,18 },
	{ 8,33,30,18 },
	{ 8,33,40,18 },
	{ 8,22,40,18 },
	{ 8,11,40,18 },

	{ 21,44,20,18 },
	{ 255,0,0,0 }
};

void DreamWebEngine::initRain() {
	const RainLocation *r = rainLocationList;
	_rainList.clear();

	uint8 rainSpacing = 0;

	// look up location in rainLocationList to determine rainSpacing
	for (r = rainLocationList; r->location != 0xff; ++r) {
		if (r->location == _realLocation &&
		        r->x == _mapX && r->y == _mapY) {
			rainSpacing = r->rainSpacing;
			break;
		}
	}

	if (rainSpacing == 0) {
		// location not found in rainLocationList: no rain
		return;
	}

	// start lines of rain from top of screen
	uint8 x = 4;
	do {
		uint8 delta = _rnd.getRandomNumberRng(3, rainSpacing - 1);
		x += delta;
		if (x >= _mapXSize)
			break;

		splitIntoLines(x, 0);
	} while (true);

	// start lines of rain from side of screen
	uint8 y = 0;
	do {
		uint8 delta = _rnd.getRandomNumberRng(3, rainSpacing - 1);
		y += delta;
		if (y >= _mapYSize)
			break;

		splitIntoLines(_mapXSize - 1, y);
	} while (true);
}

} // End of namespace DreamWeb

