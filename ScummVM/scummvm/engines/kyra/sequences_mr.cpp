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

#include "kyra/kyra_mr.h"
#include "kyra/resource.h"

namespace Kyra {

void KyraEngine_MR::showBadConscience() {
	if (_badConscienceShown)
		return;

	_badConscienceShown = true;
	_badConscienceAnim = _rnd.getRandomNumberRng(0, 2);
	if (_currentChapter == 2)
		_badConscienceAnim = 5;
	else if (_currentChapter == 3)
		_badConscienceAnim = 3;
	else if (_currentChapter == 4 && _rnd.getRandomNumberRng(1, 100) <= 25)
		_badConscienceAnim = 6;
	else if (_currentChapter == 5 && _rnd.getRandomNumberRng(1, 100) <= 25)
		_badConscienceAnim = 7;

	if (_characterShapeFile == 9)
		_badConscienceAnim = 4;

	_badConsciencePosition = (_mainCharacter.x1 <= 160);

	if (_goodConscienceShown)
		_badConsciencePosition = !_goodConsciencePosition;

	int anim = _badConscienceAnim + (_badConsciencePosition ? 0 : 8);
	TalkObject &talkObject = _talkObjectList[1];

	if (_badConsciencePosition)
		talkObject.x = 290;
	else
		talkObject.x = 30;
	talkObject.y = 30;

	static const char *const animFilenames[] = {
		"GUNFL00.WSA", "GUNFL01.WSA", "GUNFL02.WSA", "GUNFL03.WSA", "GUNFL04.WSA", "GUNFL05.WSA", "GUNFL06.WSA", "GUNFL07.WSA",
		"GUNFR00.WSA", "GUNFR01.WSA", "GUNFR02.WSA", "GUNFR03.WSA", "GUNFR04.WSA", "GUNFR05.WSA", "GUNFR06.WSA", "GUNFR07.WSA"
	};

	setupSceneAnimObject(0x0E, 9, 0, 187, -1, -1, -1, -1, 0, 0, 0, -1, animFilenames[anim]);
	for (uint i = 0; i <= _badConscienceFrameTable[_badConscienceAnim]; ++i) {
		if (i == 8)
			snd_playSoundEffect(0x1B, 0xC8);
		updateSceneAnim(0x0E, i);
		delay(3*_tickLength, true);
	}

	if (_mainCharacter.animFrame < 50 || _mainCharacter.animFrame > 87)
		return;

	if (_mainCharacter.y1 == -1 || (_mainCharacter.x1 != -1 && _mainCharacter.animFrame == 87) || _mainCharacter.animFrame == 87) {
		_mainCharacter.animFrame = 87;
	} else {
		if (_badConsciencePosition)
			_mainCharacter.facing = 3;
		else
			_mainCharacter.facing = 5;
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	}

	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();
}

void KyraEngine_MR::hideBadConscience() {
	if (!_badConscienceShown)
		return;

	_badConscienceShown = false;
	for (int frame = _badConscienceFrameTable[_badConscienceAnim+8]; frame >= 0; --frame) {
		if (frame == 15)
			snd_playSoundEffect(0x31, 0xC8);
		updateSceneAnim(0x0E, frame);
		delay(1*_tickLength, true);
	}

	updateSceneAnim(0x0E, -1);
	update();
	removeSceneAnimObject(0x0E, 1);
	setNextIdleAnimTimer();
}

void KyraEngine_MR::showGoodConscience() {
	if (_goodConscienceShown)
		return;

	_goodConscienceShown = true;
	++_goodConscienceAnim;
	_goodConscienceAnim %= 5;

	setNextIdleAnimTimer();
	_goodConsciencePosition = (_mainCharacter.x1 <= 160);

	if (_badConscienceShown)
		_goodConsciencePosition = !_badConsciencePosition;

	int anim = _goodConscienceAnim + (_goodConsciencePosition ? 0 : 5);
	TalkObject &talkObject = _talkObjectList[87];

	if (_goodConsciencePosition)
		talkObject.x = 290;
	else
		talkObject.x = 30;
	talkObject.y = 30;

	static const char *const animFilenames[] = {
		"STUFL00.WSA", "STUFL02.WSA", "STUFL04.WSA", "STUFL03.WSA", "STUFL01.WSA",
		"STUFR00.WSA", "STUFR02.WSA", "STUFR04.WSA", "STUFR03.WSA", "STUFR01.WSA"
	};

	setupSceneAnimObject(0x0F, 9, 0, 187, -1, -1, -1, -1, 0, 0, 0, -1, animFilenames[anim]);
	for (uint i = 0; i <= _goodConscienceFrameTable[_goodConscienceAnim]; ++i) {
		if (i == 10)
			snd_playSoundEffect(0x7F, 0xC8);
		updateSceneAnim(0x0F, i);
		delay(2*_tickLength, true);
	}

	if (_mainCharacter.animFrame < 50 || _mainCharacter.animFrame > 87)
		return;

	if (_mainCharacter.y1 == -1 || (_mainCharacter.x1 != -1 && _mainCharacter.animFrame == 87) || _mainCharacter.animFrame == 87) {
		_mainCharacter.animFrame = 87;
	} else {
		if (_goodConsciencePosition)
			_mainCharacter.facing = 3;
		else
			_mainCharacter.facing = 5;
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	}

	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();
}

void KyraEngine_MR::hideGoodConscience() {
	if (!_goodConscienceShown)
		return;

	_goodConscienceShown = false;
	for (int frame = _goodConscienceFrameTable[_goodConscienceAnim+5]; frame >= 0; --frame) {
		if (frame == 17)
			snd_playSoundEffect(0x31, 0xC8);
		updateSceneAnim(0x0F, frame);
		delay(1*_tickLength, true);
	}

	updateSceneAnim(0x0F, -1);
	update();
	removeSceneAnimObject(0x0F, 1);
	setNextIdleAnimTimer();
}

void KyraEngine_MR::eelScript() {
	if (_chatText)
		return;
	_screen->hideMouse();

	if (_inventoryState)
		hideInventory();
	removeHandItem();

	objectChat((const char *)getTableEntry(_cCodeFile, 35), 0, 204, 35);
	objectChat((const char *)getTableEntry(_cCodeFile, 40), 0, 204, 40);

	setGameFlag(0xD1);

	snd_playSoundEffect(0x2A, 0xC8);

	setGameFlag(0x171);

	switch (_characterShapeFile-1) {
	case 0:
		runAnimationScript("EELS01.EMC", 0, 0, 1, 1);
		break;

	case 1:
		runAnimationScript("EELS02.EMC", 0, 0, 1, 1);
		break;

	case 2:
		runAnimationScript("EELS03.EMC", 0, 0, 1, 1);
		break;

	case 3:
		runAnimationScript("EELS04.EMC", 0, 0, 1, 1);
		break;

	default:
		resetGameFlag(0x171);
		runAnimationScript("EELS00.EMC", 0, 0, 1, 1);
	}

	changeChapter(2, 29, 0, 4);
	_screen->showMouse();
}

int KyraEngine_MR::initAnimationShapes(uint8 *filedata) {
	const int lastEntry = MIN(_animShapeLastEntry, 41);
	for (int i = 0; i < lastEntry; ++i)
		_gameShapes[9+i] = _screen->getPtrToShape(filedata, i);
	return lastEntry;
}

void KyraEngine_MR::uninitAnimationShapes(int count, uint8 *filedata) {
	for (int i = 0; i < count; ++i)
		_gameShapes[9+i] = 0;
	delete[] filedata;
	setNextIdleAnimTimer();
}

} // End of namespace Kyra
