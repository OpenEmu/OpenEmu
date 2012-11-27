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

#include "kyra/kyra_mr.h"
#include "kyra/timer.h"

#include "common/system.h"

namespace Kyra {

#define TimerV3(x) new Common::Functor1Mem<int, void, KyraEngine_MR>(this, &KyraEngine_MR::x)

void KyraEngine_MR::setupTimers() {
	_timer->addTimer(0, TimerV3(timerRestoreCommandLine), -1, 1);
	for (int i = 1; i <= 3; ++i)
		_timer->addTimer(i, TimerV3(timerRunSceneScript7), -1, 0);
	_timer->addTimer(4, TimerV3(timerFleaDeath), -1, 0);
	for (int i = 5; i <= 11; ++i)
		_timer->addTimer(i, TimerV3(timerRunSceneScript7), -1, 0);
	for (int i = 12; i <= 13; ++i)
		_timer->addTimer(i, TimerV3(timerRunSceneScript7), 0, 0);
}

void KyraEngine_MR::timerRestoreCommandLine(int arg) {
	if (_shownMessage)
		_restoreCommandLine = true;
}

void KyraEngine_MR::timerRunSceneScript7(int arg) {
	_emc->init(&_sceneScriptState, &_sceneScriptData);
	_sceneScriptState.regs[1] = _mouseX;
	_sceneScriptState.regs[2] = _mouseY;
	_sceneScriptState.regs[3] = 0;
	_sceneScriptState.regs[4] = _itemInHand;
	_emc->start(&_sceneScriptState, 7);

	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);
}

void KyraEngine_MR::timerFleaDeath(int arg) {
	_timer->setCountdown(4, 5400);
	saveGameStateIntern(999, "Autosave", 0);
	_screen->hideMouse();
	_timer->disable(4);
	runAnimationScript("FLEADTH1.EMC", 0, 0, 1, 1);
	runAnimationScript("FLEADTH2.EMC", 0, 0, 1, 0);
	showBadConscience();
	delay(60, true);
	const char *str1 = (const char *)getTableEntry(_cCodeFile, 130);
	const char *str2 = (const char *)getTableEntry(_cCodeFile, 131);
	if (str1 && str2) {
		badConscienceChat(str1, 204, 130);
		badConscienceChat(str2, 204, 131);
	}
	delay(60, true);
	hideBadConscience();
	runAnimationScript("FLEADTH3.EMC", 0, 0, 0, 1);
	_deathHandler = 9;
	_screen->showMouse();
}

void KyraEngine_MR::setWalkspeed(uint8 speed) {
	if (speed < 5)
		speed = 3;
	else
		speed = 5;

	_mainCharacter.walkspeed = speed;
}

void KyraEngine_MR::setCommandLineRestoreTimer(int secs) {
	if (secs == -1)
		secs = 32000;
	_timer->setCountdown(0, secs*60);
}

void KyraEngine_MR::setNextIdleAnimTimer() {
	_nextIdleAnim = _system->getMillis() + _rnd.getRandomNumberRng(10, 15) * 1000;
}

} // End of namespace Kyra
