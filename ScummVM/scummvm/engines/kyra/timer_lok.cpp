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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "kyra/kyra_lok.h"
#include "kyra/animator_lok.h"
#include "kyra/timer.h"

namespace Kyra {

#define TimerV1(x) new Common::Functor1Mem<int, void, KyraEngine_LoK>(this, &KyraEngine_LoK::x)

void KyraEngine_LoK::setupTimers() {
	for (int i = 0; i <= 4; ++i)
		_timer->addTimer(i, 0, -1, 1);

	_timer->addTimer(5, 0,  5, 1);
	_timer->addTimer(6, 0,  7, 1);
	_timer->addTimer(7, 0,  8, 1);
	_timer->addTimer(8, 0,  9, 1);
	_timer->addTimer(9, 0,  7, 1);

	for (int i = 10; i <= 13; ++i)
		_timer->addTimer(i, 0, 420, 1);

	_timer->addTimer(14, TimerV1(timerAsWillowispTimeout), 600, 1);
	_timer->addTimer(15, TimerV1(timerUpdateHeadAnims), 11, 1);
	_timer->addTimer(16, TimerV1(timerTulipCreator), 7200, 1);
	_timer->addTimer(17, TimerV1(timerRubyCreator), 7200, 1);
	_timer->addTimer(18, TimerV1(timerAsInvisibleTimeout), 600, 1);
	_timer->addTimer(19, TimerV1(timerRedrawAmulet), 600, 1);

	_timer->addTimer(20, 0, 7200, 1);
	_timer->addTimer(21, TimerV1(timerLavenderRoseCreator), 18000, 1);
	_timer->addTimer(22, 0, 7200, 1);

	_timer->addTimer(23, 0, 10800, 1);
	_timer->addTimer(24, TimerV1(timerAcornCreator), 10800, 1);
	_timer->addTimer(25, 0, 10800, 1);
	_timer->addTimer(26, TimerV1(timerBlueberryCreator), 10800, 1);
	_timer->addTimer(27, 0, 10800, 1);

	_timer->addTimer(28, 0, 21600, 1);
	_timer->addTimer(29, 0, 7200, 1);
	_timer->addTimer(30, 0, 10800, 1);

	_timer->addTimer(31, TimerV1(timerFadeText), -1, 1);
	_timer->addTimer(32, TimerV1(timerWillowispFrameTimer), 9, 1);
	_timer->addTimer(33, TimerV1(timerInvisibleFrameTimer), 3, 1);
}

void KyraEngine_LoK::timerUpdateHeadAnims(int timerNum) {
	static const int8 frameTable[] = {
		4, 5, 4, 5, 4, 5, 0, 1,
		4, 5, 4, 4, 6, 4, 8, 1,
		9, 4, -1
	};

	if (_talkingCharNum < 0)
		return;

	_currHeadShape = frameTable[_currentHeadFrameTableIndex];
	++_currentHeadFrameTableIndex;

	if (frameTable[_currentHeadFrameTableIndex] == -1)
		_currentHeadFrameTableIndex = 0;

	_animator->animRefreshNPC(0);
	_animator->animRefreshNPC(_talkingCharNum);
}

void KyraEngine_LoK::timerTulipCreator(int timerNum) {
	if (_currentCharacter->sceneId == 0x1C)
		return;

	setItemCreationFlags(17, 3);
}

void KyraEngine_LoK::timerRubyCreator(int timerNum) {
	if (_currentCharacter->sceneId == 0x23)
		return;

	setItemCreationFlags(22, 4);
}

void KyraEngine_LoK::timerLavenderRoseCreator(int timerNum) {
	if (_currentCharacter->sceneId == 0x06)
		return;

	setItemCreationFlags(0, 4);
}

void KyraEngine_LoK::timerAcornCreator(int timerNum) {
	if (_currentCharacter->sceneId == 0x1F)
		return;

	setItemCreationFlags(72, 5);
}

void KyraEngine_LoK::timerBlueberryCreator(int timerNum) {
	if (_currentCharacter->sceneId == 0x28)
		return;

	setItemCreationFlags(26, 7);
}

void KyraEngine_LoK::setItemCreationFlags(int offset, int count) {
	int rndNr = _rnd.getRandomNumberRng(0, count);

	for (int i = 0; i <= count; i++) {
		if (!queryGameFlag(rndNr + offset)) {
			setGameFlag(rndNr + offset);
			break;
		} else {
			rndNr++;
			if (rndNr > count)
				rndNr = 0;
		}
	}
}

void KyraEngine_LoK::timerFadeText(int timerNum) {
	_fadeText = true;
}

void KyraEngine_LoK::timerWillowispFrameTimer(int timerNum) {
	if (_brandonStatusBit & 2)
		_brandonStatusBit0x02Flag = 1;
}

void KyraEngine_LoK::timerInvisibleFrameTimer(int timerNum) {
	if (_brandonStatusBit & 0x20)
		_brandonStatusBit0x20Flag = 1;
}

void KyraEngine_LoK::setTextFadeTimerCountdown(int16 countdown) {
	if (countdown == -1)
		countdown = 32000;

	_timer->setCountdown(31, countdown * 60);
}

void KyraEngine_LoK::timerAsInvisibleTimeout(int timerNum) {
	if (_brandonStatusBit & 0x20) {
		checkAmuletAnimFlags();
		_timer->setCountdown(18, -1);
	}
}

void KyraEngine_LoK::timerAsWillowispTimeout(int timerNum) {
	if (_brandonStatusBit & 0x2) {
		checkAmuletAnimFlags();
		_timer->setCountdown(14, -1);
	}
}

void KyraEngine_LoK::timerRedrawAmulet(int timerNum) {
	if (queryGameFlag(0xF1)) {
		drawAmulet();
		_timer->setCountdown(19, -1);
	}
}

void KyraEngine_LoK::setWalkspeed(uint8 newSpeed) {
	if (!_timer)
		return;

	static const uint8 speeds[] = { 11, 9, 6, 5, 3 };

	assert(newSpeed < ARRAYSIZE(speeds));
	_timer->setDelay(5, speeds[newSpeed]);
}

} // End of namespace Kyra
