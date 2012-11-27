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

#ifndef AGI_OPCODES_H
#define AGI_OPCODES_H

#include <string.h>

namespace Agi {

struct AgiInstruction {
	const char *name;
	const char *args;
	AgiCommand func;

	int argumentsLength() { return strlen(args); }
};

extern AgiInstruction *logicNamesTest;
extern AgiInstruction *logicNamesCmd;

void cmdIncrement(AgiGame *state, uint8 *p);
void cmdDecrement(AgiGame *state, uint8 *p);
void cmdAssignN(AgiGame *state, uint8 *p);
void cmdAssignV(AgiGame *state, uint8 *p);
void cmdAddN(AgiGame *state, uint8 *p);
void cmdAddV(AgiGame *state, uint8 *p);
void cmdSubN(AgiGame *state, uint8 *p);
void cmdSubV(AgiGame *state, uint8 *p);	// 0x08
void cmdLindirectV(AgiGame *state, uint8 *p);
void cmdRindirect(AgiGame *state, uint8 *p);
void cmdLindirectN(AgiGame *state, uint8 *p);
void cmdSet(AgiGame *state, uint8 *p);
void cmdReset(AgiGame *state, uint8 *p);
void cmdToggle(AgiGame *state, uint8 *p);
void cmdSetV(AgiGame *state, uint8 *p);
void cmdResetV(AgiGame *state, uint8 *p);	// 0x10
void cmdToggleV(AgiGame *state, uint8 *p);
void cmdNewRoom(AgiGame *state, uint8 *p);
void cmdNewRoomF(AgiGame *state, uint8 *p);
void cmdLoadLogic(AgiGame *state, uint8 *p);
void cmdLoadLogicF(AgiGame *state, uint8 *p);
void cmdCall(AgiGame *state, uint8 *p);
void cmdCallF(AgiGame *state, uint8 *p);
void cmdLoadPic(AgiGame *state, uint8 *p);	// 0x18
void cmdLoadPicV1(AgiGame *state, uint8 *p);
void cmdDrawPic(AgiGame *state, uint8 *p);
void cmdDrawPicV1(AgiGame *state, uint8 *p);
void cmdShowPic(AgiGame *state, uint8 *p);
void cmdDiscardPic(AgiGame *state, uint8 *p);
void cmdOverlayPic(AgiGame *state, uint8 *p);
void cmdShowPriScreen(AgiGame *state, uint8 *p);
void cmdLoadView(AgiGame *state, uint8 *p);
void cmdLoadViewF(AgiGame *state, uint8 *p);
void cmdDiscardView(AgiGame *state, uint8 *p);	// 0x20
void cmdAnimateObj(AgiGame *state, uint8 *p);
void cmdUnanimateAll(AgiGame *state, uint8 *p);
void cmdDraw(AgiGame *state, uint8 *p);
void cmdErase(AgiGame *state, uint8 *p);
void cmdPosition(AgiGame *state, uint8 *p);
void cmdPositionV1(AgiGame *state, uint8 *p);
void cmdPositionF(AgiGame *state, uint8 *p);
void cmdPositionFV1(AgiGame *state, uint8 *p);
void cmdGetPosn(AgiGame *state, uint8 *p);
void cmdReposition(AgiGame *state, uint8 *p);	// 0x28
void cmdRepositionV1(AgiGame *state, uint8 *p);	// 0x28
void cmdSetView(AgiGame *state, uint8 *p);
void cmdSetViewF(AgiGame *state, uint8 *p);
void cmdSetLoop(AgiGame *state, uint8 *p);
void cmdSetLoopF(AgiGame *state, uint8 *p);
void cmdFixLoop(AgiGame *state, uint8 *p);
void cmdReleaseLoop(AgiGame *state, uint8 *p);
void cmdSetCel(AgiGame *state, uint8 *p);
void cmdSetCelF(AgiGame *state, uint8 *p);	// 0x30
void cmdLastCel(AgiGame *state, uint8 *p);
void cmdCurrentCel(AgiGame *state, uint8 *p);
void cmdCurrentLoop(AgiGame *state, uint8 *p);
void cmdCurrentView(AgiGame *state, uint8 *p);
void cmdNumberOfLoops(AgiGame *state, uint8 *p);
void cmdSetPriority(AgiGame *state, uint8 *p);
void cmdSetPriorityF(AgiGame *state, uint8 *p);
void cmdReleasePriority(AgiGame *state, uint8 *p);	// 0x38
void cmdGetPriority(AgiGame *state, uint8 *p);
void cmdStopUpdate(AgiGame *state, uint8 *p);
void cmdStartUpdate(AgiGame *state, uint8 *p);
void cmdForceUpdate(AgiGame *state, uint8 *p);
void cmdIgnoreHorizon(AgiGame *state, uint8 *p);
void cmdObserveHorizon(AgiGame *state, uint8 *p);
void cmdSetHorizon(AgiGame *state, uint8 *p);
void cmdObjectOnWater(AgiGame *state, uint8 *p);	// 0x40
void cmdObjectOnLand(AgiGame *state, uint8 *p);
void cmdObjectOnAnything(AgiGame *state, uint8 *p);
void cmdIgnoreObjs(AgiGame *state, uint8 *p);
void cmdObserveObjs(AgiGame *state, uint8 *p);
void cmdDistance(AgiGame *state, uint8 *p);
void cmdStopCycling(AgiGame *state, uint8 *p);
void cmdStartCycling(AgiGame *state, uint8 *p);
void cmdNormalCycle(AgiGame *state, uint8 *p);	// 0x48
void cmdEndOfLoop(AgiGame *state, uint8 *p);
void cmdEndOfLoopV1(AgiGame *state, uint8 *p);
void cmdReverseCycle(AgiGame *state, uint8 *p);
void cmdReverseLoop(AgiGame *state, uint8 *p);
void cmdReverseLoopV1(AgiGame *state, uint8 *p);
void cmdCycleTime(AgiGame *state, uint8 *p);
void cmdStopMotion(AgiGame *state, uint8 *p);
void cmdStopMotionV1(AgiGame *state, uint8 *p);
void cmdStartMotion(AgiGame *state, uint8 *p);
void cmdStartMotionV1(AgiGame *state, uint8 *p);
void cmdStepSize(AgiGame *state, uint8 *p);
void cmdStepTime(AgiGame *state, uint8 *p);	// 0x50
void cmdMoveObj(AgiGame *state, uint8 *p);
void cmdMoveObjF(AgiGame *state, uint8 *p);
void cmdFollowEgo(AgiGame *state, uint8 *p);
void cmdWander(AgiGame *state, uint8 *p);
void cmdNormalMotion(AgiGame *state, uint8 *p);
void cmdSetDir(AgiGame *state, uint8 *p);
void cmdGetDir(AgiGame *state, uint8 *p);
void cmdIgnoreBlocks(AgiGame *state, uint8 *p);	// 0x58
void cmdObserveBlocks(AgiGame *state, uint8 *p);
void cmdBlock(AgiGame *state, uint8 *p);
void cmdUnblock(AgiGame *state, uint8 *p);
void cmdGet(AgiGame *state, uint8 *p);
void cmdGetV1(AgiGame *state, uint8 *p);
void cmdGetF(AgiGame *state, uint8 *p);
void cmdDrop(AgiGame *state, uint8 *p);
void cmdPut(AgiGame *state, uint8 *p);
void cmdPutF(AgiGame *state, uint8 *p);	// 0x60
void cmdGetRoomF(AgiGame *state, uint8 *p);
void cmdLoadSound(AgiGame *state, uint8 *p);
void cmdSound(AgiGame *state, uint8 *p);
void cmdStopSound(AgiGame *state, uint8 *p);
void cmdPrint(AgiGame *state, uint8 *p);
void cmdPrintF(AgiGame *state, uint8 *p);
void cmdDisplay(AgiGame *state, uint8 *p);
void cmdDisplayF(AgiGame *state, uint8 *p);	// 0x68
void cmdClearLines(AgiGame *state, uint8 *p);
void cmdTextScreen(AgiGame *state, uint8 *p);
void cmdGraphics(AgiGame *state, uint8 *p);
void cmdSetCursorChar(AgiGame *state, uint8 *p);
void cmdSetTextAttribute(AgiGame *state, uint8 *p);
void cmdShakeScreen(AgiGame *state, uint8 *p);
void cmdConfigureScreen(AgiGame *state, uint8 *p);
void cmdStatusLineOn(AgiGame *state, uint8 *p);	// 0x70
void cmdStatusLineOff(AgiGame *state, uint8 *p);
void cmdSetString(AgiGame *state, uint8 *p);
void cmdGetString(AgiGame *state, uint8 *p);
void cmdWordToString(AgiGame *state, uint8 *p);
void cmdParse(AgiGame *state, uint8 *p);
void cmdGetNum(AgiGame *state, uint8 *p);
void cmdPreventInput(AgiGame *state, uint8 *p);
void cmdAcceptInput(AgiGame *state, uint8 *p);	// 0x78
void cmdSetKey(AgiGame *state, uint8 *p);
void cmdAddToPic(AgiGame *state, uint8 *p);
void cmdAddToPicV1(AgiGame *state, uint8 *p);
void cmdAddToPicF(AgiGame *state, uint8 *p);
void cmdStatus(AgiGame *state, uint8 *p);
void cmdSaveGame(AgiGame *state, uint8 *p);
void cmdLoadGame(AgiGame *state, uint8 *p);
void cmdInitDisk(AgiGame *state, uint8 *p);
void cmdRestartGame(AgiGame *state, uint8 *p);	// 0x80
void cmdShowObj(AgiGame *state, uint8 *p);
void cmdRandom(AgiGame *state, uint8 *p);
void cmdRandomV1(AgiGame *state, uint8 *p);
void cmdProgramControl(AgiGame *state, uint8 *p);
void cmdPlayerControl(AgiGame *state, uint8 *p);
void cmdObjStatusF(AgiGame *state, uint8 *p);
void cmdQuit(AgiGame *state, uint8 *p);
void cmdQuitV1(AgiGame *state, uint8 *p);
void cmdShowMem(AgiGame *state, uint8 *p);
void cmdPause(AgiGame *state, uint8 *p);	// 0x88
void cmdEchoLine(AgiGame *state, uint8 *p);
void cmdCancelLine(AgiGame *state, uint8 *p);
void cmdInitJoy(AgiGame *state, uint8 *p);
void cmdToggleMonitor(AgiGame *state, uint8 *p);
void cmdVersion(AgiGame *state, uint8 *p);
void cmdScriptSize(AgiGame *state, uint8 *p);
void cmdSetGameID(AgiGame *state, uint8 *p);
void cmdLog(AgiGame *state, uint8 *p);	// 0x90
void cmdSetScanStart(AgiGame *state, uint8 *p);
void cmdResetScanStart(AgiGame *state, uint8 *p);
void cmdRepositionTo(AgiGame *state, uint8 *p);
void cmdRepositionToF(AgiGame *state, uint8 *p);
void cmdTraceOn(AgiGame *state, uint8 *p);
void cmdTraceInfo(AgiGame *state, uint8 *p);
void cmdPrintAt(AgiGame *state, uint8 *p);
void cmdPrintAtV(AgiGame *state, uint8 *p);	// 0x98
//void cmdDiscardView(AgiGame *state, uint8 *p);	// Opcode repeated from 0x20 ?
void cmdClearTextRect(AgiGame *state, uint8 *p);
void cmdSetUpperLeft(AgiGame *state, uint8 *p);
void cmdSetMenu(AgiGame *state, uint8 *p);
void cmdSetMenuItem(AgiGame *state, uint8 *p);
void cmdSubmitMenu(AgiGame *state, uint8 *p);
void cmdEnableItem(AgiGame *state, uint8 *p);
void cmdDisableItem(AgiGame *state, uint8 *p);	// 0xa0
void cmdMenuInput(AgiGame *state, uint8 *p);
void cmdShowObjV(AgiGame *state, uint8 *p);
void cmdOpenDialogue(AgiGame *state, uint8 *p);
void cmdCloseDialogue(AgiGame *state, uint8 *p);
void cmdMulN(AgiGame *state, uint8 *p);
void cmdMulV(AgiGame *state, uint8 *p);
void cmdDivN(AgiGame *state, uint8 *p);
void cmdDivV(AgiGame *state, uint8 *p);	// 0xa8
void cmdCloseWindow(AgiGame *state, uint8 *p);
void cmdSetSimple(AgiGame *state, uint8 *p);
void cmdPushScript(AgiGame *state, uint8 *p);
void cmdPopScript(AgiGame *state, uint8 *p);
void cmdHoldKey(AgiGame *state, uint8 *p);
void cmdSetPriBase(AgiGame *state, uint8 *p);
void cmdDiscardSound(AgiGame *state, uint8 *p);
void cmdHideMouse(AgiGame *state, uint8 *p);	// 0xb0
void cmdAllowMenu(AgiGame *state, uint8 *p);
void cmdShowMouse(AgiGame *state, uint8 *p);
void cmdFenceMouse(AgiGame *state, uint8 *p);
void cmdMousePosn(AgiGame *state, uint8 *p);
void cmdReleaseKey(AgiGame *state, uint8 *p);
void cmdAdjEgoMoveToXY(AgiGame *state, uint8 *p);

void cmdSetSpeed(AgiGame *state, uint8 *p);
void cmdSetItemView(AgiGame *state, uint8 *p);
void cmdCallV1(AgiGame *state, uint8 *p);
void cmdNewRoomV1(AgiGame *state, uint8 *p);
void cmdNewRoomVV1(AgiGame *state, uint8 *p);
void cmdUnknown(AgiGame *state, uint8 *p);

void condEqual(AgiGame *state, uint8 *p);
void condEqualV(AgiGame *state, uint8 *p);
void condLess(AgiGame *state, uint8 *p);
void condLessV(AgiGame *state, uint8 *p);
void condGreater(AgiGame *state, uint8 *p);
void condGreaterV(AgiGame *state, uint8 *p);
void condIsSet(AgiGame *state, uint8 *p);
void condIsSetV(AgiGame *state, uint8 *p);
void condHas(AgiGame *state, uint8 *p);
void condHasV1(AgiGame *state, uint8 *p);
void condObjInRoom(AgiGame *state, uint8 *p);
void condPosn(AgiGame *state, uint8 *p);
void condController(AgiGame *state, uint8 *p);
void condHaveKey(AgiGame *state, uint8 *p);
void condSaid(AgiGame *state, uint8 *p);
void condCompareStrings(AgiGame *state, uint8 *p);
void condObjInBox(AgiGame *state, uint8 *p);
void condCenterPosn(AgiGame *state, uint8 *p);
void condRightPosn(AgiGame *state, uint8 *p);
void condUnknown13(AgiGame *state, uint8 *p);
void condUnknown(AgiGame *state, uint8 *p);

void condIsSetV1(AgiGame *state, uint8 *p);
void condSaid1(AgiGame *state, uint8 *p);
void condSaid2(AgiGame *state, uint8 *p);
void condSaid3(AgiGame *state, uint8 *p);
void condBit(AgiGame *state, uint8 *p);

} // End of namespace Agi

#endif /* AGI_OPCODES_H */
