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

#include "kyra/kyra_v1.h"
#include "kyra/screen.h"

#include "common/system.h"

namespace Kyra {

int KyraEngine_v1::o1_queryGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_queryGameFlag(%p) (0x%X)", (const void *)script, stackPos(0));
	return queryGameFlag(stackPos(0));
}

int KyraEngine_v1::o1_setGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_setGameFlag(%p) (0x%X)", (const void *)script, stackPos(0));
	return setGameFlag(stackPos(0));
}

int KyraEngine_v1::o1_resetGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_resetGameFlag(%p) (0x%X)", (const void *)script, stackPos(0));
	return resetGameFlag(stackPos(0));
}

int KyraEngine_v1::o1_getRand(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_getRand(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	int min = stackPos(0);
	int max = stackPos(1);
	if (max < min)
		SWAP(min, max);
	return _rnd.getRandomNumberRng(min, max);
}

int KyraEngine_v1::o1_hideMouse(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_hideMouse(%p) ()", (const void *)script);
	screen()->hideMouse();
	return 0;
}

int KyraEngine_v1::o1_showMouse(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_showMouse(%p) ()", (const void *)script);
	screen()->showMouse();
	return 0;
}

int KyraEngine_v1::o1_setMousePos(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_setMousePos(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_system->warpMouse(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v1::o1_setHandItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_setHandItem(%p) (%d)", (const void *)script, stackPos(0));
	setHandItem(stackPos(0));
	return 0;
}

int KyraEngine_v1::o1_removeHandItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_removeHandItem(%p) ()", (const void *)script);
	removeHandItem();
	return 0;
}

int KyraEngine_v1::o1_getMouseState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_getMouseState(%p) ()", (const void *)script);
	return _mouseState;
}

int KyraEngine_v1::o1_setDeathHandler(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_setDeathHandler(%p) (%d)", (const void *)script, stackPos(0));
	_deathHandler = stackPos(0);
	return 0;
}

int KyraEngine_v1::o1_playWanderScoreViaMap(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_playWanderScoreViaMap(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	snd_playWanderScoreViaMap(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v1::o1_fillRect(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_fillRect(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	screen()->fillRect(stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(0));
	return 0;
}

int KyraEngine_v1::o1_blockInWalkableRegion(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_blockInWalkableRegion(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	screen()->blockInRegion(stackPos(0), stackPos(1), stackPos(2) - stackPos(0) + 1, stackPos(3) - stackPos(1) + 1);
	return 0;
}

int KyraEngine_v1::o1_blockOutWalkableRegion(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_blockOutWalkableRegion(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	screen()->blockOutRegion(stackPos(0), stackPos(1), stackPos(2) - stackPos(0) + 1, stackPos(3) - stackPos(1) + 1);
	return 0;
}

int KyraEngine_v1::o1_playSoundEffect(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v1::o1_playSoundEffect(%p) (%d)", (const void *)script, stackPos(0));
	snd_playSoundEffect(stackPos(0));
	return 0;
}

} // End of namespace Kyra
