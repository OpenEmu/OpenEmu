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

#include "kyra/kyra_v2.h"
#include "kyra/screen_v2.h"
#include "kyra/timer.h"

#include "common/system.h"

namespace Kyra {

int KyraEngine_v2::o2_getCharacterX(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getCharacterX(%p) ()", (const void *)script);
	return _mainCharacter.x1;
}

int KyraEngine_v2::o2_getCharacterY(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getCharacterY(%p) ()", (const void *)script);
	return _mainCharacter.y1;
}

int KyraEngine_v2::o2_getCharacterFacing(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getCharacterFacing(%p) ()", (const void *)script);
	return _mainCharacter.facing;
}

int KyraEngine_v2::o2_getCharacterScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getCharacterScene(%p) ()", (const void *)script);
	return _mainCharacter.sceneId;
}

int KyraEngine_v2::o2_setCharacterFacingOverwrite(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setCharacterFacingOverwrite(%p) (%d)", (const void *)script, stackPos(0));
	_mainCharacter.facing = stackPos(0);
	_overwriteSceneFacing = true;
	return 0;
}

int KyraEngine_v2::o2_trySceneChange(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_trySceneChange(%p) (%d, %d, %d, %d)", (const void *)script,
	       stackPos(0), stackPos(1), stackPos(2), stackPos(3));

	_unkHandleSceneChangeFlag = 1;
	int success = inputSceneChange(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_unkHandleSceneChangeFlag = 0;

	if (success) {
		_emc->init(script, script->dataPtr);
		_unk4 = 0;
		_savedMouseState = -1;
		_unk5 = 1;
		return 0;
	} else {
		return (_unk4 != 0) ? 1 : 0;
	}
}

int KyraEngine_v2::o2_moveCharacter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_moveCharacter(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	moveCharacter(stackPos(0), stackPos(1), stackPos(2));
	return 0;
}

int KyraEngine_v2::o2_checkForItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_checkForItem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return findItem(stackPos(0), stackPos(1)) == -1 ? 0 : 1;
}

int KyraEngine_v2::o2_defineItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_defineItem(%p) (%d, %d, %d, %d)", (const void *)script,
	       stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int freeItem = findFreeItem();

	if (freeItem >= 0) {
		_itemList[freeItem].id = stackPos(0);
		_itemList[freeItem].x = stackPos(1);
		_itemList[freeItem].y = stackPos(2);
		_itemList[freeItem].sceneId = stackPos(3);
	}

	return freeItem;
}

int KyraEngine_v2::o2_addSpecialExit(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_addSpecialExit(%p) (%d, %d, %d, %d, %d)", (const void *)script,
	       stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	if (_specialExitCount < 5) {
		_specialExitTable[_specialExitCount + 0] = stackPos(0);
		_specialExitTable[_specialExitCount + 5] = stackPos(1);
		_specialExitTable[_specialExitCount + 10] = stackPos(2) + stackPos(0) - 1;
		_specialExitTable[_specialExitCount + 15] = stackPos(3) + stackPos(1) - 1;
		_specialExitTable[_specialExitCount + 20] = stackPos(4);
		++_specialExitCount;
	}
	return 0;
}

int KyraEngine_v2::o2_delay(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_delay(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (stackPos(1)) {
		uint32 maxWaitTime = _system->getMillis() + stackPos(0) * _tickLength;
		while (_system->getMillis() < maxWaitTime) {
			int inputFlag = checkInput(0);
			removeInputTop();

			if (inputFlag == 198 || inputFlag == 199)
				return 1;

			if (_chatText)
				updateWithText();
			else
				update();
			_system->delayMillis(10);
		}
	} else {
		delay(stackPos(0) * _tickLength, true);
	}
	return 0;
}

int KyraEngine_v2::o2_update(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_update(%p) (%d)", (const void *)script, stackPos(0));
	for (int times = stackPos(0); times != 0; --times) {
		if (_chatText)
			updateWithText();
		else
			update();
	}
	return 0;
}

int KyraEngine_v2::o2_getShapeFlag1(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getShapeFlag1(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return screen()->getShapeFlag1(stackPos(0), stackPos(1));
}

int KyraEngine_v2::o2_waitForConfirmationClick(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_waitForConfirmationClick(%p) (%d)", (const void *)script, stackPos(0));
	resetSkipFlag();
	uint32 maxWaitTime = _system->getMillis() + stackPos(0) * _tickLength;

	while (_system->getMillis() < maxWaitTime) {
		int inputFlag = checkInput(0);
		removeInputTop();

		if (inputFlag == 198 || inputFlag == 199) {
			_sceneScriptState.regs[1] = _mouseX;
			_sceneScriptState.regs[2] = _mouseY;
			return 0;
		}

		update();
		_system->delayMillis(10);
	}

	_sceneScriptState.regs[1] = _mouseX;
	_sceneScriptState.regs[2] = _mouseY;
	return 1;
}

int KyraEngine_v2::o2_randomSceneChat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_randomSceneChat(%p)", (const void *)script);
	randomSceneChat();
	return 0;
}

int KyraEngine_v2::o2_setDlgIndex(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setDlgIndex(%p) (%d)", (const void *)script, stackPos(0));
	setDlgIndex(stackPos(0));
	return 0;
}

int KyraEngine_v2::o2_getDlgIndex(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getDlgIndex(%p) ()", (const void *)script);
	return _mainCharacter.dlgIndex;
}


int KyraEngine_v2::o2_defineRoomEntrance(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_defineRoomEntrance(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	switch (stackPos(0)) {
	case 0:
		_sceneEnterX1 = stackPos(1);
		_sceneEnterY1 = stackPos(2);
		break;

	case 1:
		_sceneEnterX2 = stackPos(1);
		_sceneEnterY2 = stackPos(2);
		break;

	case 2:
		_sceneEnterX3 = stackPos(1);
		_sceneEnterY3 = stackPos(2);
		break;

	case 3:
		_sceneEnterX4 = stackPos(1);
		_sceneEnterY4 = stackPos(2);
		break;

	default:
		break;
	}
	return 0;
}

int KyraEngine_v2::o2_runAnimationScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_runAnimationScript(%p) ('%s', %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1),
	       stackPos(2), stackPos(3));

	runAnimationScript(stackPosString(0), stackPos(3), stackPos(2) ? 1 : 0, stackPos(1), stackPos(2));
	return 0;
}

int KyraEngine_v2::o2_setSpecialSceneScriptRunTime(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setSpecialSceneScriptRunTime(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) >= 0 && stackPos(0) < 10);
	_sceneSpecialScriptsTimer[stackPos(0)] = _system->getMillis() + stackPos(1) * _tickLength;
	return 0;
}

int KyraEngine_v2::o2_defineScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_defineScene(%p) (%d, '%s', %d, %d, %d, %d, %d, %d)",
	       (const void *)script, stackPos(0), stackPosString(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	const int scene = stackPos(0);
	strcpy(_sceneList[scene].filename1, stackPosString(1));
	strcpy(_sceneList[scene].filename2, stackPosString(1));

	_sceneList[scene].exit1 = stackPos(2);
	_sceneList[scene].exit2 = stackPos(3);
	_sceneList[scene].exit3 = stackPos(4);
	_sceneList[scene].exit4 = stackPos(5);
	_sceneList[scene].flags = stackPos(6);
	_sceneList[scene].sound = stackPos(7);

	if (_mainCharacter.sceneId == scene) {
		_sceneExit1 = _sceneList[scene].exit1;
		_sceneExit2 = _sceneList[scene].exit2;
		_sceneExit3 = _sceneList[scene].exit3;
		_sceneExit4 = _sceneList[scene].exit4;
	}

	return 0;
}

int KyraEngine_v2::o2_setSpecialSceneScriptState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setSpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	_specialSceneScriptState[stackPos(0)] = 1;
	return 1;
}

int KyraEngine_v2::o2_clearSpecialSceneScriptState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_clearSpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	_specialSceneScriptState[stackPos(0)] = 0;
	return 0;
}

int KyraEngine_v2::o2_querySpecialSceneScriptState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_querySpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	return _specialSceneScriptState[stackPos(0)];
}

int KyraEngine_v2::o2_setHiddenItemsEntry(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setHiddenItemsEntry(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return (_hiddenItems[stackPos(0)] = stackPos(1));
}

int KyraEngine_v2::o2_getHiddenItemsEntry(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getHiddenItemsEntry(%p) (%d)", (const void *)script, stackPos(0));
	return (int16)_hiddenItems[stackPos(0)];
}

int KyraEngine_v2::o2_disableTimer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_disableTimer(%p) (%d)", (const void *)script, stackPos(0));
	_timer->disable(stackPos(0));
	return 0;
}

int KyraEngine_v2::o2_enableTimer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_enableTimer(%p) (%d)", (const void *)script, stackPos(0));
	_timer->enable(stackPos(0));
	return 0;
}

int KyraEngine_v2::o2_setTimerCountdown(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setTimerCountdown(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_timer->setCountdown(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v2::o2_setVocHigh(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setVocHigh(%p) (%d)", (const void *)script, stackPos(0));
	_vocHigh = stackPos(0);
	return _vocHigh;
}

int KyraEngine_v2::o2_getVocHigh(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getVocHigh(%p) ()", (const void *)script);
	return _vocHigh;
}

#pragma mark -

int KyraEngine_v2::o2a_setAnimationShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2a_setAnimationShapes(%p) ('%s', %d, %d, %d, %d, %d)", (const void *)script,
	       stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	strcpy(_animShapeFilename, stackPosString(0));
	_animShapeLastEntry = stackPos(1);
	_animShapeWidth = stackPos(2);
	_animShapeHeight = stackPos(3);
	_animShapeXAdd = stackPos(4);
	_animShapeYAdd = stackPos(5);
	return 0;
}

int KyraEngine_v2::o2a_setResetFrame(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3t_setResetFrame(%p) (%d)", (const void *)script, stackPos(0));
	_animResetFrame = stackPos(0);
	return 0;
}

} // End of namespace Kyra
