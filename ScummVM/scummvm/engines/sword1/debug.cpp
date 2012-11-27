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

#include "common/debug.h"
#include "common/util.h"
#include "sword1/debug.h"

namespace Sword1 {

void Debug::interpretScript(uint32 id, uint32 level, uint32 script, uint32 pc) {
	debug(8, "\nInterpreting %d@%d: script %X from %X", id, level, script, pc);
}

void Debug::callMCode(uint32 mcodeNum, uint32 paramCount, int32 a, int32 b, int32 c, int32 d, int32 e, int32 f) {
	debug(9, "mcode: %s(%d, %d, %d, %d, %d, %d) [%d]", _mCodeNames[mcodeNum], a, b, c, d, e, f, paramCount);
}

const char Debug::_mCodeNames[100][35] = {
	"fnBackground",
	"fnForeground",
	"fnSort",
	"fnNoSprite",
	"fnMegaSet",
	"fnAnim",
	"fnSetFrame",
	"fnFullAnim",
	"fnFullSetFrame",
	"fnFadeDown",
	"fnFadeUp",
	"fnCheckFade",
	"fnSetSpritePalette",
	"fnSetWholePalette",
	"fnSetFadeTargetPalette",
	"fnSetPaletteToFade",
	"fnSetPaletteToCut",
	"fnPlaySequence",
	"fnIdle",
	"fnPause",
	"fnPauseSeconds",
	"fnQuit",
	"fnKillId",
	"fnSuicide",
	"fnNewScript",
	"fnSubScript",
	"fnRestartScript",
	"fnSetBookmark",
	"fnGotoBookmark",
	"fnSendSync",
	"fnWaitSync",
	"cfnClickInteract",
	"cfnSetScript",
	"cfnPresetScript",
	"fnInteract",
	"fnIssueEvent",
	"fnCheckForEvent",
	"fnWipeHands",
	"fnISpeak",
	"fnTheyDo",
	"fnTheyDoWeWait",
	"fnWeWait",
	"fnChangeSpeechText",
	"fnTalkError",
	"fnStartTalk",
	"fnCheckForTextLine",
	"fnAddTalkWaitStatusBit",
	"fnRemoveTalkWaitStatusBit",
	"fnNoHuman",
	"fnAddHuman",
	"fnBlankMouse",
	"fnNormalMouse",
	"fnLockMouse",
	"fnUnlockMouse",
	"fnSetMousePointer",
	"fnSetMouseLuggage",
	"fnMouseOn",
	"fnMouseOff",
	"fnChooser",
	"fnEndChooser",
	"fnStartMenu",
	"fnEndMenu",
	"cfnReleaseMenu",
	"fnAddSubject",
	"fnAddObject",
	"fnRemoveObject",
	"fnEnterSection",
	"fnLeaveSection",
	"fnChangeFloor",
	"fnWalk",
	"fnTurn",
	"fnStand",
	"fnStandAt",
	"fnFace",
	"fnFaceXy",
	"fnIsFacing",
	"fnGetTo",
	"fnGetToError",
	"fnGetPos",
	"fnGetGamepadXy",
	"fnPlayFx",
	"fnStopFx",
	"fnPlayMusic",
	"fnStopMusic",
	"fnInnerSpace",
	"fnRandom",
	"fnSetScreen",
	"fnPreload",
	"fnCheckCD",
	"fnRestartGame",
	"fnQuitGame",
	"fnDeathScreen",
	"fnSetParallax",
	"fnTdebug",
	"fnRedFlash",
	"fnBlueFlash",
	"fnYellow",
	"fnGreen",
	"fnPurple",
	"fnBlack"
};

} // End of namespace Sword1
