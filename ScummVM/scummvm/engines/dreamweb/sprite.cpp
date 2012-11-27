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

void DreamWebEngine::printSprites() {
	for (size_t priority = 0; priority < 7; ++priority) {
		Common::List<Sprite>::const_iterator i;
		for (i = _spriteTable.begin(); i != _spriteTable.end(); ++i) {
			const Sprite &sprite = *i;
			if (priority != sprite.priority)
				continue;
			if (sprite.hidden == 1)
				continue;
			printASprite(&sprite);
		}
	}
}

void DreamWebEngine::printASprite(const Sprite *sprite) {
	uint16 x, y;
	if (sprite->y >= 220) {
		y = _mapAdY - (256 - sprite->y);
	} else {
		y = sprite->y + _mapAdY;
	}

	if (sprite->x >= 220) {
		x = _mapAdX - (256 - sprite->x);
	} else {
		x = sprite->x + _mapAdX;
	}

	uint8 c;
	if (sprite->walkFrame != 0)
		c = 8;
	else
		c = 0;
	showFrame(*sprite->_frameData, x, y, sprite->frameNumber, c);
}

void DreamWebEngine::clearSprites() {
	_spriteTable.clear();
}

Sprite *DreamWebEngine::makeSprite(uint8 x, uint8 y, bool mainManCallback, const GraphicsFile *frameData) {
	// Note: the original didn't append sprites here, but filled up the
	// first unused entry. This can change the order of entries, but since they
	// are drawn based on the priority field, this shouldn't matter.
	_spriteTable.push_back(Sprite());
	Sprite *sprite = &_spriteTable.back();

	memset(sprite, 0xff, sizeof(Sprite));

	sprite->_mainManCallback = mainManCallback;
	sprite->x = x;
	sprite->y = y;
	sprite->_frameData = frameData;
	sprite->frameNumber = 0;
	sprite->delay = 0;
	sprite->_objData = 0;
	return sprite;
}

void DreamWebEngine::spriteUpdate() {
	// During the intro the sprite table can be empty
	if (!_spriteTable.empty())
		_spriteTable.front().hidden = _vars._ryanOn;

	Common::List<Sprite>::iterator i;
	for (i = _spriteTable.begin(); i != _spriteTable.end(); ++i) {
		Sprite &sprite = *i;
		if (sprite._mainManCallback)
			mainMan(&sprite);
		else {
			backObject(&sprite);
		}

		if (_nowInNewRoom == 1)
			break;
	}
}

void DreamWebEngine::initMan() {
	Sprite *sprite = makeSprite(_ryanX, _ryanY, true, &_mainSprites);
	sprite->priority = 4;
	sprite->speed = 0;
	sprite->walkFrame = 0;
}

void DreamWebEngine::mainMan(Sprite *sprite) {
	if (_resetManXY == 1) {
		_resetManXY = 0;
		sprite->x = _ryanX;
		sprite->y = _ryanY;
		sprite->walkFrame = 0;
	}

	--sprite->speed;
	if (sprite->speed != 0xff)
		return;
	sprite->speed = 0;

	if (_turnToFace != _facing) {
		aboutTurn(sprite);
	} else {
		if ((_turnDirection != 0) && (_linePointer == 254)) {
			_reAssesChanges = 1;
			if (_facing == _leaveDirection)
				checkForExit(sprite);
		}
		_turnDirection = 0;
		if (_linePointer == 254) {
			sprite->walkFrame = 0;
		} else {
			++sprite->walkFrame;
			if (sprite->walkFrame == 11)
				sprite->walkFrame = 1;
			walking(sprite);
			if (_linePointer != 254) {
				if ((_facing & 1) == 0)
					walking(sprite);
				else if ((sprite->walkFrame != 2) && (sprite->walkFrame != 7))
					walking(sprite);
			}
			if (_linePointer == 254) {
				if (_turnToFace == _facing) {
					_reAssesChanges = 1;
					if (_facing == _leaveDirection)
						checkForExit(sprite);
				}
			}
		}
	}
	static const uint8 facelist[] = { 0,60,33,71,11,82,22,93 };
	sprite->frameNumber = sprite->walkFrame + facelist[_facing];
	_ryanX = sprite->x;
	_ryanY = sprite->y;
}

void DreamWebEngine::walking(Sprite *sprite) {
	uint8 comp;
	if (_lineDirection != 0) {
		--_linePointer;
		comp = 200;
	} else {
		++_linePointer;
		comp = _lineLength;
	}
	if (_linePointer < comp) {
		sprite->x = (uint8)_lineData[_linePointer].x;
		sprite->y = (uint8)_lineData[_linePointer].y;
		return;
	}

	_linePointer = 254;
	_mansPath = _destination;
	if (_destination == _finalDest) {
		faceRightWay();
		return;
	}
	_destination = _finalDest;
	autoSetWalk();
}

void DreamWebEngine::aboutTurn(Sprite *sprite) {
	bool incdir = true;

	if (_turnDirection == 1)
		incdir = true;
	else if ((int8)_turnDirection == -1)
		incdir = false;
	else {
		if (_facing < _turnToFace) {
			uint8 delta = _turnToFace - _facing;
			if (delta >= 4)
				incdir = false;
			else
				incdir = true;
		} else {
			uint8 delta = _facing - _turnToFace;
			if (delta >= 4)
				incdir = true;
			else
				incdir = false;
		}
	}

	if (incdir) {
		_turnDirection = 1;
		_facing = (_facing + 1) & 7;
		sprite->walkFrame = 0;
	} else {
		_turnDirection = (uint8)-1;
		_facing = (_facing - 1) & 7;
		sprite->walkFrame = 0;
	}
}

void DreamWebEngine::backObject(Sprite *sprite) {
	SetObject *objData = sprite->_objData;

	if (sprite->delay != 0) {
		--sprite->delay;
		return;
	}

	sprite->delay = objData->delay;
	switch (objData->type) {
	case 6:
		wideDoor(sprite, objData);
		break;
	case 5:
		randomSprite(sprite, objData);
		break;
	case 4:
		lockedDoorway(sprite, objData);
		break;
	case 3:
		liftSprite(sprite, objData);
		break;
	case 2:
		doorway(sprite, objData);
		break;
	case 1:
		constant(sprite, objData);
		break;
	default:
		steady(sprite, objData);
		break;
	}
}

void DreamWebEngine::constant(Sprite *sprite, SetObject *objData) {
	++sprite->animFrame;
	if (objData->frames[sprite->animFrame] == 255) {
		sprite->animFrame = 0;
	}
	uint8 frame = objData->frames[sprite->animFrame];
	objData->index = frame;
	sprite->frameNumber = frame;
}

void DreamWebEngine::randomSprite(Sprite *sprite, SetObject *objData) {
	uint8 r = _rnd.getRandomNumber(7);
	sprite->frameNumber = objData->frames[r];
}

void DreamWebEngine::doorway(Sprite *sprite, SetObject *objData) {
	Common::Rect check(-24, -30, 10, 10);
	doDoor(sprite, objData, check);
}

void DreamWebEngine::wideDoor(Sprite *sprite, SetObject *objData) {
	Common::Rect check(-24, -30, 24, 24);
	doDoor(sprite, objData, check);
}

void DreamWebEngine::doDoor(Sprite *sprite, SetObject *objData, Common::Rect check) {
	int ryanx = _ryanX;
	int ryany = _ryanY;

	// Automatically opening doors: check if Ryan is in range

	check.translate(sprite->x, sprite->y);
	bool openDoor = check.contains(ryanx, ryany);

	if (openDoor) {

		if ((_vars._throughDoor == 1) && (sprite->animFrame == 0))
			sprite->animFrame = 6;

		++sprite->animFrame;
		if (sprite->animFrame == 1) { // doorsound2
			uint8 soundIndex;
			if (_realLocation == 5) // hoteldoor2
				soundIndex = 13;
			else
				soundIndex = 0;
			_sound->playChannel1(soundIndex);
		}
		if (objData->frames[sprite->animFrame] == 255)
			--sprite->animFrame;

		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
		_vars._throughDoor = 1;

	} else {
		// shut door

		if (sprite->animFrame == 5) { // doorsound1;
			uint8 soundIndex;
			if (_realLocation == 5) // hoteldoor1
				soundIndex = 13;
			else
				soundIndex = 1;
			_sound->playChannel1(soundIndex);
		}
		if (sprite->animFrame != 0)
			--sprite->animFrame;

		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
		if (sprite->animFrame == 5) // nearly
			_vars._throughDoor = 0;
	}
}

void DreamWebEngine::steady(Sprite *sprite, SetObject *objData) {
	uint8 frame = objData->frames[0];
	objData->index = frame;
	sprite->frameNumber = frame;
}

void DreamWebEngine::lockedDoorway(Sprite *sprite, SetObject *objData) {
	int ryanx = _ryanX;
	int ryany = _ryanY;

	Common::Rect check(-24, -30, 10, 12);
	check.translate(sprite->x, sprite->y);
	bool openDoor = check.contains(ryanx, ryany);

	if (_vars._throughDoor != 1 && _vars._lockStatus == 1)
		openDoor = false;

	if (openDoor) {

		if (sprite->animFrame == 1) {
			_sound->playChannel1(0);
		}

		if (sprite->animFrame == 6)
			turnPathOn(_vars._doorPath);

		if (_vars._throughDoor == 1 && sprite->animFrame == 0)
			sprite->animFrame = 6;

		++sprite->animFrame;
		if (objData->frames[sprite->animFrame] == 255)
			--sprite->animFrame;

		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
		if (sprite->animFrame == 5)
			_vars._throughDoor = 1;

	} else {
		// shut door

		if (sprite->animFrame == 5) {
			_sound->playChannel1(1);
		}

		if (sprite->animFrame != 0)
			--sprite->animFrame;

		_vars._throughDoor = 0;
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];

		if (sprite->animFrame == 0) {
			turnPathOff(_vars._doorPath);
			_vars._lockStatus = 1;
		}
	}
}

void DreamWebEngine::liftSprite(Sprite *sprite, SetObject *objData) {
	uint8 liftFlag = _vars._liftFlag;
	if (liftFlag == 0) { //liftclosed
		turnPathOff(_vars._liftPath);

		if (_vars._countToOpen != 0) {
			_vars._countToOpen--;
			if (_vars._countToOpen == 0)
				_vars._liftFlag = 3;
		}
		sprite->animFrame = 0;
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	}
	else if (liftFlag == 1) {  //liftopen
		turnPathOn(_vars._liftPath);

		if (_vars._countToClose != 0) {
			_vars._countToClose--;
			if (_vars._countToClose == 0)
				_vars._liftFlag = 2;
		}
		sprite->animFrame = 12;
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	}
	else if (liftFlag == 3) { //openlift
		if (sprite->animFrame == 12) {
			_vars._liftFlag = 1;
			return;
		}
		++sprite->animFrame;
		if (sprite->animFrame == 1) {
			liftNoise(2);
		}
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	} else { //closeLift
		assert(liftFlag == 2);
		if (sprite->animFrame == 0) {
			_vars._liftFlag = 0;
			return;
		}
		--sprite->animFrame;
		if (sprite->animFrame == 11) {
			liftNoise(3);
		}
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	}
}

Reel *DreamWebEngine::getReelStart(uint16 reelPointer) {
	Reel *reel = &_reelList[reelPointer * 8];
	return reel;
}

// Locate the reel segment (reel1, reel2, reel3) this frame is stored in,
// and adjust the frame number relative to this segment.
const GraphicsFile *DreamWebEngine::findSource(uint16 &frame) {
	if (frame < 160) {
		return &_reel1;
	} else if (frame < 320) {
		frame -= 160;
		return &_reel2;
	} else {
		frame -= 320;
		return &_reel3;
	}
}

void DreamWebEngine::showReelFrame(Reel *reel) {
	uint16 x = reel->x + _mapAdX;
	uint16 y = reel->y + _mapAdY;
	uint16 frame = reel->frame();
	const GraphicsFile *base = findSource(frame);
	showFrame(*base, x, y, frame, 8);
}

void DreamWebEngine::showGameReel(ReelRoutine *routine) {
	uint16 reelPointer = routine->reelPointer();
	if (reelPointer >= 512)
		return;
	plotReel(reelPointer);
	routine->setReelPointer(reelPointer);
}

const Frame *DreamWebEngine::getReelFrameAX(uint16 frame) {
	const GraphicsFile *base = findSource(frame);
	return &base->_frames[frame];
}

void DreamWebEngine::moveMap(uint8 param) {
	switch (param) {
	case 32:
		_mapY -= 20;
		break;
	case 16:
		_mapY -= 10;
		break;
	case 8:
		_mapY += 10;
		break;
	case 2:
		_mapX += 11;
		break;
	default:
		_mapX -= 11;
		break;
	}
	_nowInNewRoom = 1;
}

void DreamWebEngine::checkOne(uint8 x, uint8 y, uint8 *flag, uint8 *flagEx, uint8 *type, uint8 *flagX, uint8 *flagY) {
	*flagX = x / 16;
	*flagY = y / 16;
	const MapFlag &tileData = _mapFlags[*flagY * 11 + *flagX];
	*flag = tileData._flag;
	*flagEx = tileData._flagEx;
	*type = tileData._type;
}

void DreamWebEngine::intro1Text() {
	if (_introCount != 2 && _introCount != 4 && _introCount != 6)
		return;

	if (hasSpeech() && _sound->isChannel1Playing()) {
		_introCount--;
	} else {
		if (_introCount == 2)
			setupTimedTemp(40, 82, 34, 130, 90, 1);
		else if (_introCount == 4)
			setupTimedTemp(41, 82, 34, 130, 90, 1);
		else if (_introCount == 6)
			setupTimedTemp(42, 82, 34, 130, 90, 1);
	}
}

void DreamWebEngine::intro2Text(uint16 nextReelPointer) {
	if (nextReelPointer == 5)
		setupTimedTemp(43, 82, 34, 40, 90, 1);
	else if (nextReelPointer == 15)
		setupTimedTemp(44, 82, 34, 40, 90, 1);
}

void DreamWebEngine::intro3Text(uint16 nextReelPointer) {
	if (nextReelPointer == 107)
		setupTimedTemp(45, 82, 36, 56, 100, 1);
	else if (nextReelPointer == (hasSpeech() ? 108 : 109))
		setupTimedTemp(46, 82, 36, 56, 100, 1);
}

void DreamWebEngine::monks2text() {
	bool isGermanCD = hasSpeech() && getLanguage() == Common::DE_DEU;

	if (_introCount == 1)
		setupTimedTemp(8, 82, 36, 160, 120, 1);
	else if (_introCount == (isGermanCD ? 5 : 4))
		setupTimedTemp(9, 82, 36, 160, 120, 1);
	else if (_introCount == (isGermanCD ? 9 : 7))
		setupTimedTemp(10, 82, 36, 160, 120, 1);
	else if (_introCount == 10 && !isGermanCD) {
		if (hasSpeech())
			_introCount = 12;
		setupTimedTemp(11, 82, 0, 105, 120, 1);
	} else if (_introCount == 13 && isGermanCD) {
		_introCount = 14;
		setupTimedTemp(11, 82, 0, 105, 120, 1);
	} else if (_introCount == 13 && !isGermanCD) {
		if (hasSpeech())
			_introCount = 17;
		else
			setupTimedTemp(12, 82, 0, 120, 120, 1);
	} else if (_introCount == 16 && !isGermanCD)
		setupTimedTemp(13, 82, 0, 135, 120, 1);
	else if (_introCount == 19)
		setupTimedTemp(14, 82, 36, 160, 100, 1);
	else if (_introCount == (isGermanCD ? 23 : 22))
		setupTimedTemp(15, 82, 36, 160, 120, 1);
	else if (_introCount == (isGermanCD ? 27 : 25))
		setupTimedTemp(16, 82, 36, 160, 120, 1);
	else if (_introCount == (hasSpeech() ? 27 : 28) && !isGermanCD)
		setupTimedTemp(17, 82, 36, 160, 120, 1);
	else if (_introCount == 30 && isGermanCD)
		setupTimedTemp(17, 82, 36, 160, 120, 1);
	else if (_introCount == (isGermanCD ? 35 : 31))
		setupTimedTemp(18, 82, 36, 160, 120, 1);
}

void DreamWebEngine::textForEnd() {
	if (_introCount == 20)
		setupTimedTemp(0, 83, 34, 20, 60, 1);
	else if (_introCount == (hasSpeech() ? 50 : 65))
		setupTimedTemp(1, 83, 34, 20, 60, 1);
	else if (_introCount == (hasSpeech() ? 85 : 110))
		setupTimedTemp(2, 83, 34, 20, 60, 1);
}

void DreamWebEngine::textForMonkHelper(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount) {
	if (hasSpeech() && _sound->isChannel1Playing())
		_introCount--;
	else
		setupTimedTemp(textIndex, voiceIndex, x, y, countToTimed, timeCount);
}

void DreamWebEngine::textForMonk() {
	if (_introCount == 1)
		textForMonkHelper(19, 82, 68, 154, 120, 1);
	else if (_introCount == 5)
		textForMonkHelper(20, 82, 68, 38, 120, 1);
	else if (_introCount == 9)
		textForMonkHelper(21, 82, 48, 154, 120, 1);
	else if (_introCount == 13)
		textForMonkHelper(22, 82, 68, 38, 120, 1);
	else if (_introCount == (hasSpeech() ? 15 : 17))
		textForMonkHelper(23, 82, 68, 154, 120, 1);
	else if (_introCount == 21)
		textForMonkHelper(24, 82, 68, 38, 120, 1);
	else if (_introCount == 25)
		textForMonkHelper(25, 82, 68, 154, 120, 1);
	else if (_introCount == 29)
		textForMonkHelper(26, 82, 68, 38, 120, 1);
	else if (_introCount == 33)
		textForMonkHelper(27, 82, 68, 154, 120, 1);
	else if (_introCount == 37)
		textForMonkHelper(28, 82, 68, 154, 120, 1);
	else if (_introCount == 41)
		textForMonkHelper(29, 82, 68, 38, 120, 1);
	else if (_introCount == 45)
		textForMonkHelper(30, 82, 68, 154, 120, 1);
	else if (_introCount == (hasSpeech() ? 52 : 49))
		textForMonkHelper(31, 82, 68, 154, 220, 1);
	else if (_introCount == 53) {
		fadeScreenDowns();
		if (hasSpeech()) {
			_sound->volumeChange(7, 1);
		}
	}
}

void DreamWebEngine::reelsOnScreen() {
	reconstruct();
	updatePeople();
	watchReel();
	showRain();
	useTimedText();
}

void DreamWebEngine::reconstruct() {
	if (_haveDoneObs == 0)
		return;
	_vars._newObs = 1;
	drawFloor();
	spriteUpdate();
	printSprites();
	if (_foreignRelease && (_realLocation == 20))
		underTextLine();
	_haveDoneObs = 0;
}



struct ReelSound {
	uint8 _sample;
	uint16 _reelPointer;
};

static const ReelSound g_roomSound0[] = {
	{ 255,0 }
};

static const ReelSound g_roomSound1[] = {
	{ 15, 257 },
	{ 255,0 }
};

static const ReelSound g_roomSound2[] = {
	{ 12, 5 },
	{ 13, 21 },
	{ 15, 35 }, // hitting floor?
	{ 17, 50 },
	{ 18, 103 },
	{ 19, 108 },
	{ 255,0 }
};

static const ReelSound g_roomSound6[] = {
	{ 18, 19 },
	{ 19, 23 },
	{ 255,0 }
};
static const ReelSound g_roomSound8[] = {

	{ 12, 51 },
	{ 13, 53 },
	{ 14, 14 },
	{ 15, 20 },
	{ 0, 78 },
	{ 255,0 }
};
static const ReelSound g_roomSound9[] = {

	{ 12, 119 },
	{ 12, 145 },
	{ 255,0 }
};

static const ReelSound g_roomSound10[] = {
	{ 13, 16 },
	{ 255,0 }
};

static const ReelSound g_roomSound11[] = {
	{ 13, 20 },
	{ 255,0 }
};

static const ReelSound g_roomSound12[] = {
	{ 14, 16 },
	{ 255,0 }
};

static const ReelSound g_roomSound13[] = {
	{ 15, 4 },
	{ 16, 8 },
	{ 17, 134 },
	{ 18, 153 },
	{ 255,0 }
};

static const ReelSound g_roomSound14[] = {
	{ 13, 108 },
	{ 15, 326 },
	{ 15, 331 },
	{ 15, 336 },
	{ 15, 342 },
	{ 15, 348 },
	{ 15, 354 },
	{ 18, 159 },
	{ 18, 178 },
	{ 19+128, 217 },
	{ 20+64, 228 },
	{ 255,0 }
};

static const ReelSound g_roomSound20[] = {
	{ 13, 20 },
	{ 13, 21 },
	{ 15, 34 },
	{ 13, 52 },
	{ 13, 55 },
	{ 25, 57 },
	{ 21, 73 },
	{ 255,0 }
};

static const ReelSound g_roomSound22[] = {
	{ 13, 196 },
	{ 13, 234 },
	{ 13, 156 },
	{ 14, 129 },
	{ 13, 124 },
	{ 15, 162 },
	{ 15, 200 },
	{ 15, 239 },
	{ 17, 99 },
	{ 12, 52 },
	{ 255,0 }
};

static const ReelSound g_roomSound23[] = {
	{ 15, 56 },
	{ 16, 64 },
	{ 19, 22 },
	{ 20, 33 },
	{ 255,0 }
};

static const ReelSound g_roomSound25[] = {
	{ 20, 11 },
	{ 20, 15 },
	{ 15, 28 },
	{ 13, 80 },
	{ 21, 82 },
	{ 19+128, 87 },
	{ 23+64, 128 },
	{ 255,0 }
};

static const ReelSound g_roomSound26[] = {
	{ 12, 13 },
	{ 14, 39 },
	{ 12, 67 },
	{ 12, 75 },
	{ 12, 83 },
	{ 12, 91 },
	{ 15, 102 }, // was 90, should be mine cart
	{ 255,0 }
};

static const ReelSound g_roomSound27[] = {
	{ 22, 36 },
	{ 13, 125 },
	{ 18, 88 },
	{ 15, 107 },
	{ 14, 127 },
	{ 14, 154 },
	{ 19+128, 170 },
	{ 23+64, 232 },
	{ 255,0 }
};

static const ReelSound g_roomSound28[] = {
	{ 21, 16 },
	{ 21, 72 },
	{ 21, 205 },
	{ 22, 63 }, // 65
	{ 23+128, 99 },
	{ 24+64, 158 },
	{ 255,0 }
};

static const ReelSound g_roomSound29[] = {
	{ 13, 21 },
	{ 14, 24 },
	{ 19+128, 50 },
	{ 23+64, 75 },
	{ 24, 128 },
	{ 255,0 }
};

static const ReelSound g_roomSound29_German[] = {
	{ 13, 21 },
	{ 14, 24 },
	{ 19+128, 50 },
	{ 23+64, 75 },
	{ 255,0 }
};


static const ReelSound g_roomSound45[] = {
	{ 19+64, 46 },
	{ 16, 167 },
	{ 255,0 }
};

static const ReelSound g_roomSound46[] = {
	{ 16, 19 },
	{ 14, 36 },
	{ 16, 50 },
	{ 14, 65 },
	{ 16, 81 },
	{ 14, 96 },
	{ 16, 114 },
	{ 14, 129 },
	{ 16, 147 },
	{ 14, 162 },
	{ 16, 177 },
	{ 14, 191 },
	{ 255,0 }
};

static const ReelSound g_roomSound47[] = {
	{ 13, 48 },
	{ 14, 41 },
	{ 15, 78 },
	{ 16, 92 },
	{ 255,0 }
};

static const ReelSound g_roomSound52[] = {
	{ 16, 115 },
	{ 255,0 }
};

static const ReelSound g_roomSound53[] = {
	{ 21, 103 },
	{ 20, 199 },
	{ 255,0 }
};

static const ReelSound g_roomSound55[] = {
	{ 17, 53 },
	{ 17, 54 },
	{ 17, 55 },
	{ 17, 56 },
	{ 17, 57 },
	{ 17, 58 },
	{ 17, 59 },
	{ 17, 61 },
	{ 17, 63 },
	{ 17, 64 },
	{ 17, 65 },
	{ 255,0 }
};

static const ReelSound *g_roomByRoom[] = {
	g_roomSound0,g_roomSound1,g_roomSound2,g_roomSound0,g_roomSound0,
	g_roomSound0,g_roomSound6,g_roomSound0,g_roomSound8,g_roomSound9,
	g_roomSound10,g_roomSound11,g_roomSound12,g_roomSound13,g_roomSound14,
	g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,
	g_roomSound20,g_roomSound0,g_roomSound22,g_roomSound23,g_roomSound0,
	g_roomSound25,g_roomSound26,g_roomSound27,g_roomSound28,g_roomSound29,
	g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,
	g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,
	g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,
	g_roomSound45,g_roomSound46,g_roomSound47,g_roomSound0,g_roomSound0,
	g_roomSound0,g_roomSound0,g_roomSound52,g_roomSound53,g_roomSound0,
	g_roomSound55
};


void DreamWebEngine::soundOnReels(uint16 reelPointer) {
	const ReelSound *r = g_roomByRoom[_realLocation];

	if (getLanguage() == Common::DE_DEU && r == g_roomSound29)
		r = g_roomSound29_German;

	for (; r->_sample != 255; ++r) {
		if (r->_reelPointer != reelPointer)
			continue;
		if (r->_reelPointer == _lastSoundReel)
			continue;
		_lastSoundReel = r->_reelPointer;
		if (r->_sample < 64) {
			_sound->playChannel1(r->_sample);
			return;
		}
		if (r->_sample < 128) {
			_sound->playChannel0(r->_sample & 63, 0);
			return;
		}
		_sound->playChannel0(r->_sample & 63, 255);
	}

	if (_lastSoundReel != reelPointer)
		_lastSoundReel = (uint16)-1;
}

void DreamWebEngine::clearBeforeLoad() {
	if (_roomLoaded != 1)
		return; // noclear

	clearReels();

	//clearRest
	memset(_mapData, 0, kLengthOfMap);
	delete[] _backdropBlocks;
	_backdropBlocks = 0;
	_setFrames.clear();
	delete[] _reelList;
	_reelList = 0;
	_personText.clear();
	_setDesc.clear();
	_blockDesc.clear();
	_roomDesc.clear();
	_freeFrames.clear();
	_freeDesc.clear();

	_roomLoaded = 0;
}

void DreamWebEngine::clearReels() {
	_reel1.clear();
	_reel2.clear();
	_reel3.clear();
}

void DreamWebEngine::getRidOfReels() {
	if (_roomLoaded)
		clearReels();
}

void DreamWebEngine::liftNoise(uint8 index) {
	if (_realLocation == 5 || _realLocation == 21)
		_sound->playChannel1(13);	// hiss noise
	else
		_sound->playChannel1(index);
}

void DreamWebEngine::checkForExit(Sprite *sprite) {
	uint8 flag, flagEx, type, flagX, flagY;
	checkOne(_ryanX + 12, _ryanY + 12, &flag, &flagEx, &type, &flagX, &flagY);
	_lastFlag = flag;

	if (flag & 64) {
		_autoLocation = flagEx;
		return;
	}

	if (!(flag & 32)) {
		if (flag & 4) {
			// adjust left
			_lastFlag = 0;
			_mapX -= 11;
			sprite->x = 16 * flagEx;
			_nowInNewRoom = 1;
		} else if (flag & 2) {
			// adjust right
			_mapX += 11;
			sprite->x = 16 * flagEx - 2;
			_nowInNewRoom = 1;
		} else if (flag & 8) {
			// adjust down
			_mapY += 10;
			sprite->y = 16 * flagEx;
			_nowInNewRoom = 1;
		} else if (flag & 16) {
			// adjust up
			_mapY -= 10;
			sprite->y = 16 * flagEx;
			_nowInNewRoom = 1;
		}

		return;
	}

	if (_realLocation == 2) {
		// Can't leave Louis' until you found shoes

		int shoeCount = 0;
		if (isRyanHolding("WETA")) shoeCount++;
		if (isRyanHolding("WETB")) shoeCount++;

		if (shoeCount < 2) {
			uint8 text = shoeCount ? 43 : 42;
			setupTimedUse(text, 80, 10, 68, 64);

			_turnToFace = (_facing + 4) & 7;
			return;
		}

	}

	_vars._needToTravel = 1;
}

} // End of namespace DreamWeb

