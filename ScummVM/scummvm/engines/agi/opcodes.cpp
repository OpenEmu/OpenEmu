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

#include "agi/agi.h"
#include "agi/opcodes.h"

namespace Agi {

AgiInstruction *logicNamesTest;
AgiInstruction *logicNamesCmd;

AgiInstruction insV1Test[] = {
	{ "",					"",			&condUnknown },		// 00
	{ "equaln",				"vn",		&condEqual },		// 01
	{ "equalv",				"vv",		&condEqualV },		// 02
	{ "lessn",				"vn",		&condLess },		// 03
	{ "lessv",				"vv",		&condLessV },		// 04
	{ "greatern",			"vn",		&condGreater },		// 05
	{ "greaterv",			"vv",		&condGreaterV },	// 06
	{ "isset",				"v",		&condIsSetV1 },		// 07
	{ "has",				"n",		&condHasV1 },		// 08
	{ "said",				"nnnn",		&condSaid2 },		// 09
	{ "posn",				"nnnnn",	&condPosn },		// 0A
	{ "controller",			"n",		&condController },	// 0B
	{ "obj.in.room",		"nv",		&condObjInRoom },	// 0C
	{ "said",				"nnnnnn",	&condSaid3 },		// 0D
	{ "have.key",			"",			&condHaveKey },		// 0E
	{ "said",				"nn",		&condSaid1 },		// 0F
	{ "bit",				"nv",		&condBit },			// 10
};

AgiInstruction insV1[] = {
	{ "return",				"",			NULL },					// 00
	{ "increment",			"v",		&cmdIncrement },		// 01
	{ "decrement",			"v",		&cmdDecrement },		// 02
	{ "assignn",			"vn",		&cmdAssignN },			// 03
	{ "assignv",			"vv",		&cmdAssignV },			// 04
	{ "addn",				"vn",		&cmdAddN },				// 05
	{ "addv",				"vv",		&cmdAddV },				// 06
	{ "subn",				"vn",		&cmdSubN },				// 07
	{ "subv",				"vv",		&cmdSubV },				// 08
	{ "load.view",			"n",		&cmdLoadView },			// 09
	{ "animate.obj",		"n",		&cmdAnimateObj },		// 0A
	{ "new.room",			"n",		&cmdNewRoomV1 },		// 0B
	{ "draw.pic",			"v",		&cmdDrawPicV1 },		// 0C
	{ "print",				"s",		&cmdPrint },			// 0D TODO
	{ "status",				"",			&cmdStatus },			// 0E TODO
	{ "save.game",			"",			&cmdSaveGame },			// 0F TODO
	{ "restore.game",		"",			&cmdLoadGame },			// 10 TODO
	{ "init.disk",			"",			&cmdInitDisk },			// 11 TODO
	{ "restart.game",		"",			&cmdRestartGame },		// 12 TODO
	{ "random",				"v",		&cmdRandomV1 },			// 13
	{ "get",				"n",		&cmdGetV1 },			// 14
	{ "drop",				"n",		&cmdDrop },				// 15
	{ "draw",				"n",		&cmdDraw },				// 16 TODO
	{ "erase",				"n",		&cmdErase },			// 17 TODO
	{ "position",			"nnn",		&cmdPositionV1 },		// 18
	{ "position.v",			"nvv",		&cmdPositionFV1 },		// 19
	{ "get.posn",			"nvv",		&cmdGetPosn },			// 1A
	{ "set.cel",			"nn",		&cmdSetCel },			// 1B
	{ "set.loop",			"nn",		&cmdSetLoop },			// 1C
	{ "end.of.loop",		"nn",		&cmdEndOfLoopV1 },		// 1D
	{ "reverse.loop",		"nn",		&cmdReverseLoopV1 },	// 1E
	{ "move.obj",			"nnnnn",	&cmdMoveObj },			// 1F
	{ "set.view",			"nn",		&cmdSetView },			// 20
	{ "follow.ego",			"nnn",		&cmdFollowEgo },		// 21
	{ "block",				"",			&cmdBlock },			// 22
	{ "unblock",			"",			&cmdUnblock },			// 23
	{ "ignore.blocks",		"n",		&cmdIgnoreBlocks },		// 24
	{ "observe.blocks",		"n",		&cmdObserveBlocks },	// 25
	{ "wander",				"n",		&cmdWander },			// 26
	{ "reposition",			"nvv",		&cmdRepositionV1 },		// 27
	{ "stop.motion",		"n",		&cmdStopMotionV1 },		// 28
	{ "start.motion",		"n",		&cmdStartMotionV1 },	// 29
	{ "stop.cycling",		"n",		&cmdStopCycling },		// 2A
	{ "start.cycling",		"n",		&cmdStartCycling },		// 2B
	{ "stop.update",		"n",		&cmdStopUpdate },		// 2C
	{ "start.update",		"n",		&cmdStartUpdate },		// 2D
	{ "program.control",	"",			&cmdProgramControl },	// 2E
	{ "player.control",		"",			&cmdPlayerControl },	// 2F
	{ "set.priority",		"nn",		&cmdSetPriority },		// 30
	{ "release.priority",	"n",		&cmdReleasePriority },	// 31
	{ "add.to.pic",			"nnnnnn",	&cmdAddToPicV1 },		// 32
	{ "set.horizon",		"n",		&cmdSetHorizon },		// 33
	{ "ignore.horizon",		"n",		&cmdIgnoreHorizon },	// 34
	{ "observe.horizon",	"n",		&cmdObserveHorizon },	// 35
	{ "load.logics",		"n",		&cmdLoadLogic },		// 36 TODO
	{ "object.on.water",	"n",		&cmdObjectOnWater },	// 37
	{ "load.pic",			"v",		&cmdLoadPicV1 },		// 38
	{ "load.sound",			"n",		&cmdLoadSound },		// 39
	{ "sound",				"nn",		&cmdSound },			// 3A
	{ "stop.sound",			"",			&cmdStopSound },		// 3B
	{ "set.v",				"v",		&cmdSetV },				// 3C
	{ "reset.v",			"v",		&cmdResetV },			// 3D
	{ "toggle.v",			"v",		&cmdToggleV },			// 3E
	{ "new.room.v",			"v",		&cmdNewRoomVV1 },		// 3F TODO
	{ "call",				"n",		&cmdCallV1 },			// 40 TODO
	{ "quit",				"",			&cmdQuitV1 },			// 41
	{ "set.speed",			"v",		&cmdSetSpeed },			// 42
	{ "move.obj.v",			"nvvvv",	&cmdMoveObjF },			// 43
	{ "...",				"nn",		&cmdUnknown },			// 44
	{ "get.v",				"v",		&cmdUnknown },			// 45
	{ "assign.v",			"vv",		&cmdUnknown },			// 46
	{ "...",				"n",		&cmdUnknown },			// 47 # printvar.v
	{ "get.priority",		"nv",		&cmdGetPriority },		// 48
	{ "ignore.objs",		"n",		&cmdIgnoreObjs },		// 49
	{ "observe.objs",		"n",		&cmdObserveObjs },		// 4A
	{ "distance",			"nnv",		&cmdDistance },			// 4B
	{ "object.on.land",		"n",		&cmdObjectOnLand },		// 4C
	{ "...",				"nv",		&cmdUnknown },			// 4D # set.priority.f
	{ "...",				"",			&cmdUnknown },			// 4E  # show.obj
	{ "load.logics",		"n",		&cmdLoadLogic },		// 4F # load.global.logics
	{ "display",			"nnns",		&cmdDisplay },			// 50 TODO: 4 vs 3 args
	{ "prevent.input???",	"",			&cmdUnknown },			// 51
	{ "...",				"",			&cmdUnknown },			// 52 # nop
	{ "...",				"n",		&cmdUnknown },			// 53 # text.screen
	{ "...",				"",			&cmdUnknown },			// 54 ???
	{ "stop.motion",		"",			&cmdStopMotion },		// 55 or force.update??
	{ "discard.view",		"n",		&cmdDiscardView },		// 56
	{ "discard.pic",		"v",		&cmdDiscardPic },		// 57
	{ "set.item.view",		"nn",		&cmdSetItemView },		// 58
	{ "...",				"",			&cmdUnknown },			// 59 # reverse.cycle
	{ "last.cel",			"nv",		&cmdLastCel },			// 5A
	{ "set.cel.v",			"nv",		&cmdSetCelF },			// 5B
	{ "...",				"",			&cmdUnknown },			// 5C # normal.cycle
	{ "load.view",			"n",		&cmdLoadView },			// 5D
	{ "...",				"",			&cmdUnknown },			// 5E
	{ "...",				"",			&cmdUnknown },			// 5F
	{ "setbit",				"nv",		&cmdUnknown },			// 60
	{ "...",				"nv",		&cmdUnknown },			// 61 # clearbit
};

AgiInstruction insV2Test[] = {
	{ "",					"",			&condUnknown },			// 00
	{ "equaln",				"vn",		&condEqual },			// 01
	{ "equalv",				"vv",		&condEqualV },			// 02
	{ "lessn",				"vn",		&condLess },			// 03
	{ "lessv",				"vv",		&condLessV },			// 04
	{ "greatern",			"vn",		&condGreater },			// 05
	{ "greaterv",			"vv",		&condGreaterV },		// 06
	{ "isset",				"n",		&condIsSet },			// 07
	{ "issetv",				"v",		&condIsSetV },			// 08
	{ "has",				"n",		&condHas },				// 09
	{ "obj.in.room",		"nv",		&condObjInRoom},		// 0A
	{ "posn",				"nnnnn",	&condPosn },			// 0B
	{ "controller",			"n",		&condController },		// 0C
	{ "have.key",			"",			&condHaveKey},			// 0D
	{ "said",				"",			&condSaid },			// 0E
	{ "compare.strings",	"ss",		&condCompareStrings },	// 0F
	{ "obj.in.box",			"nnnnn",	&condObjInBox },		// 10
	{ "center.posn",		"nnnnn",	&condCenterPosn },		// 11
	{ "right.posn",			"nnnnn",	&condRightPosn },		// 12
	{ "in.motion.using.mouse", "",		&condUnknown13 }		// 13
};

AgiInstruction insV2[] = {
	{ "return",				"",			NULL },
	{ "increment",			"v",		&cmdIncrement },
	{ "decrement",			"v",		&cmdDecrement },
	{ "assignn",			"vn",		&cmdAssignN },
	{ "assignv",			"vv",		&cmdAssignV },
	{ "addn",				"vn",		&cmdAddN },
	{ "addv",				"vv",		&cmdAddV },
	{ "subn",				"vn",		&cmdSubN },
	{ "subv",				"vv",		&cmdSubV },
	{ "lindirectv",			"vv",		&cmdLindirectV },
	{ "lindirect",			"vv",		&cmdRindirect },
	{ "lindirectn",			"vn",		&cmdLindirectN },
	{ "set",				"n",		&cmdSet },
	{ "reset",				"n",		&cmdReset },
	{ "toggle",				"n",		&cmdToggle },
	{ "set.v",				"v",		&cmdSetV },
	{ "reset.v",			"v",		&cmdResetV },
	{ "toggle.v",			"v",		&cmdToggleV },
	{ "new.room",			"n",		&cmdNewRoom },
	{ "new.room.v",			"v",		&cmdNewRoomF },
	{ "load.logics",		"n",		&cmdLoadLogic },
	{ "load.logics.v",		"v",		&cmdLoadLogicF },
	{ "call",				"n",		&cmdCall },
	{ "call.v",				"v",		&cmdCallF },
	{ "load.pic",			"v",		&cmdLoadPic },
	{ "draw.pic",			"v",		&cmdDrawPic },
	{ "show.pic",			"",			&cmdShowPic },
	{ "discard.pic",		"v",		&cmdDiscardPic },
	{ "overlay.pic",		"v",		&cmdOverlayPic },
	{ "show.pri.screen",	"",			&cmdShowPriScreen },
	{ "load.view",			"n",		&cmdLoadView },
	{ "load.view.v",		"v",		&cmdLoadViewF },
	{ "discard.view",		"n",		&cmdDiscardView },
	{ "animate.obj",		"n",		&cmdAnimateObj },
	{ "unanimate.all",		"",			&cmdUnanimateAll },
	{ "draw",				"n",		&cmdDraw },
	{ "erase",				"n",		&cmdErase },
	{ "position",			"nnn",		&cmdPosition },
	{ "position.v",			"nvv",		&cmdPositionF },
	{ "get.posn",			"nvv",		&cmdGetPosn },
	{ "reposition",			"nvv",		&cmdReposition },
	{ "set.view",			"nn",		&cmdSetView },
	{ "set.view.v",			"nv",		&cmdSetViewF },
	{ "set.loop",			"nn",		&cmdSetLoop },
	{ "set.loop.v",			"nv",		&cmdSetLoopF },
	{ "fix.loop",			"n",		&cmdFixLoop },
	{ "release.loop",		"n",		&cmdReleaseLoop },
	{ "set.cel",			"nn",		&cmdSetCel },
	{ "set.cel.v",			"nv",		&cmdSetCelF },
	{ "last.cel",			"nv",		&cmdLastCel },
	{ "current.cel",		"nv",		&cmdCurrentCel },
	{ "current.loop",		"nv",		&cmdCurrentLoop },
	{ "current.view",		"nv",		&cmdCurrentView },
	{ "number.of.loops",	"nv",		&cmdNumberOfLoops },
	{ "set.priority",		"nn",		&cmdSetPriority },
	{ "set.priority.v",		"nv",		&cmdSetPriorityF },
	{ "release.priority",	"n",		&cmdReleasePriority },
	{ "get.priority",		"nn",		&cmdGetPriority },
	{ "stop.update",		"n",		&cmdStopUpdate },
	{ "start.update",		"n",		&cmdStartUpdate },
	{ "force.update",		"n",		&cmdForceUpdate },
	{ "ignore.horizon",		"n",		&cmdIgnoreHorizon },
	{ "observe.horizon",	"n",		&cmdObserveHorizon },
	{ "set.horizon",		"n",		&cmdSetHorizon },
	{ "object.on.water",	"n",		&cmdObjectOnWater },
	{ "object.on.land",		"n",		&cmdObjectOnLand },
	{ "object.on.anything",	"n",		&cmdObjectOnAnything },
	{ "ignore.objs",		"n",		&cmdIgnoreObjs },
	{ "observe.objs",		"n",		&cmdObserveObjs },
	{ "distance",			"nnv",		&cmdDistance },
	{ "stop.cycling",		"n",		&cmdStopCycling },
	{ "start.cycling",		"n",		&cmdStartCycling },
	{ "normal.cycle",		"n",		&cmdNormalCycle },
	{ "end.of.loop",		"nn",		&cmdEndOfLoop },
	{ "reverse.cycle",		"n",		&cmdReverseCycle },
	{ "reverse.loop",		"nn",		&cmdReverseLoop },
	{ "cycle.time",			"nv",		&cmdCycleTime },
	{ "stop.motion",		"n",		&cmdStopMotion },
	{ "start.motion",		"n",		&cmdStartMotion },
	{ "step.size",			"nv",		&cmdStepSize },
	{ "step.time",			"nv",		&cmdStepTime },
	{ "move.obj",			"nnnnn",	&cmdMoveObj },
	{ "move.obj.v",			"nvvvv",	&cmdMoveObjF },
	{ "follow.ego",			"nnn",		&cmdFollowEgo },
	{ "wander",				"n",		&cmdWander },
	{ "normal.motion",		"n",		&cmdNormalMotion },
	{ "set.dir",			"nv",		&cmdSetDir },
	{ "get.dir",			"nv",		&cmdGetDir },
	{ "ignore.blocks",		"n",		&cmdIgnoreBlocks },
	{ "observe.blocks",		"n",		&cmdObserveBlocks },
	{ "block",				"nnnn",		&cmdBlock },
	{ "unblock",			"",			&cmdUnblock },
	{ "get",				"n",		&cmdGet },
	{ "get.v",				"v",		&cmdGetF },
	{ "drop",				"n",		&cmdDrop },
	{ "put",				"nn",		&cmdPut },
	{ "put.v",				"vv",		&cmdPutF },
	{ "get.room.v",			"vv",		&cmdGetRoomF },
	{ "load.sound",			"n",		&cmdLoadSound },
	{ "sound",				"nn",		&cmdSound },
	{ "stop.sound",			"",			&cmdStopSound },
	{ "print",				"s",		&cmdPrint },
	{ "print.v",			"v",		&cmdPrintF },
	{ "display",			"nns",		&cmdDisplay },
	{ "display.v",			"vvv",		&cmdDisplayF },
	{ "clear.lines",		"nns",		&cmdClearLines },
	{ "text.screen",		"",			&cmdTextScreen },
	{ "graphics",			"",			&cmdGraphics },
	{ "set.cursor.char",	"s",		&cmdSetCursorChar },
	{ "set.text.attribute",	"nn",		&cmdSetTextAttribute },
	{ "shake.screen",		"n",		&cmdShakeScreen },
	{ "configure.screen",	"nnn",		&cmdConfigureScreen },
	{ "status.line.on",		"",			&cmdStatusLineOn },
	{ "status.line.off",	"",			&cmdStatusLineOff },
	{ "set.string",			"ns",		&cmdSetString },
	{ "get.string",			"nsnnn",		&cmdGetString },
	{ "word.to.string",		"nn",		&cmdWordToString },
	{ "parse",				"n",		&cmdParse },
	{ "get.num",			"nv",		&cmdGetNum },
	{ "prevent.input",		"",			&cmdPreventInput },
	{ "accept.input",		"",			&cmdAcceptInput },
	{ "set.key",			"nnn",		&cmdSetKey },
	{ "add.to.pic",			"nnnnnnn",	&cmdAddToPic },
	{ "add.to.pic.v",		"vvvvvvv",	&cmdAddToPicF },
	{ "status",				"",			&cmdStatus },
	{ "save.game",			"",			&cmdSaveGame },
	{ "restore.game",		"",			&cmdLoadGame },
	{ "init.disk",			"",			&cmdInitDisk },
	{ "restart.game",		"",			&cmdRestartGame },
	{ "show.obj",			"n",		&cmdShowObj },
	{ "random",				"nnv",		&cmdRandom },
	{ "program.control",	"",			&cmdProgramControl },
	{ "player.control",		"",			&cmdPlayerControl },
	{ "obj.status.v",		"v",		&cmdObjStatusF },
	{ "quit",				"n",		&cmdQuit },  // 0 args for AGI version 2.089
	{ "show.mem",			"",			&cmdShowMem },
	{ "pause",				"",			&cmdPause },
	{ "echo.line",			"",			&cmdEchoLine },
	{ "cancel.line",		"",			&cmdCancelLine },
	{ "init.joy",			"",			&cmdInitJoy },
	{ "toggle.monitor",		"",			&cmdToggleMonitor },
	{ "version",			"",			&cmdVersion },
	{ "script.size",		"n",		&cmdScriptSize },
	{ "set.game.id",		"s",		&cmdSetGameID },
	{ "log",				"s",		&cmdLog },
	{ "set.scan.start",		"",			&cmdSetScanStart },
	{ "reset.scan.start",	"",			&cmdResetScanStart },
	{ "reposition.to",		"nnn",		&cmdRepositionTo },
	{ "reposition.to.v",	"nvv",		&cmdRepositionToF },
	{ "trace.on",			"",			&cmdTraceOn },
	{ "trace.info", 		"nnn",		&cmdTraceInfo },
	{ "print.at",			"snnn",		&cmdPrintAt }, // 3 args for AGI versions before 2.440
	{ "print.at.v",			"vnnn",		&cmdPrintAtV },
	{ "discard.view.v",		"v",		&cmdDiscardView},
	{ "clear.text.rect",	"nnnnn",	&cmdClearTextRect },
	{ "set.upper.left",		"nn",		&cmdSetUpperLeft },
	{ "set.menu",			"s",		&cmdSetMenu },
	{ "set.menu.item",		"sn",		&cmdSetMenuItem },
	{ "submit.menu",		"",			&cmdSubmitMenu },
	{ "enable.item",		"n",		&cmdEnableItem },
	{ "disable.item",		"n",		&cmdDisableItem },
	{ "menu.input",			"",			&cmdMenuInput },
	{ "show.obj.v",			"v",		&cmdShowObjV },
	{ "open.dialogue",		"",			&cmdOpenDialogue },
	{ "close.dialogue",		"",			&cmdCloseDialogue },
	{ "mul.n",				"vn",		&cmdMulN },
	{ "mul.v",				"vv",		&cmdMulV },
	{ "div.n",				"vn",		&cmdDivN },
	{ "div.v",				"vv",		&cmdDivV },
	{ "close.window",		"",			&cmdCloseWindow },
	{ "set.simple",			"n",		&cmdSetSimple },
	{ "push.script",		"",			&cmdPushScript },
	{ "pop.script",			"",			&cmdPopScript },
	{ "hold.key",			"",			&cmdHoldKey },
	{ "set.pri.base",		"n",		&cmdSetPriBase },
	{ "discard.sound",		"n",		&cmdDiscardSound },
	{ "hide.mouse",			"",			&cmdHideMouse }, // 1 arg for AGI version 3.002.086
	{ "allow.menu",			"n",		&cmdAllowMenu },
	{ "show.mouse",			"",			&cmdShowMouse },
	{ "fence.mouse",		"nnnn",		&cmdFenceMouse },
	{ "mouse.posn",			"vv",		&cmdMousePosn },
	{ "release.key",		"",			&cmdReleaseKey }, // 2 args for at least the Amiga GR (v2.05 1989-03-09) using AGI 2.316
	{ "adj.ego.move.to.xy",	"",			&cmdAdjEgoMoveToXY }
};

void AgiEngine::setupOpcodes() {
	if (getVersion() >= 0x2000) {
		for (int i = 0; i < ARRAYSIZE(insV2Test); ++i)
			_agiCondCommands[i] = insV2Test[i].func;
		for (int i = 0; i < ARRAYSIZE(insV2); ++i)
			_agiCommands[i] = insV2[i].func;

		logicNamesTest = insV2Test;
		logicNamesCmd = insV2;
	} else {
		for (int i = 0; i < ARRAYSIZE(insV1Test); ++i)
			_agiCondCommands[i] = insV1Test[i].func;
		for (int i = 0; i < ARRAYSIZE(insV1); ++i)
			_agiCommands[i] = insV1[i].func;

		logicNamesTest = insV1Test;
		logicNamesCmd = insV1;
	}

	// Alter opcode parameters for specific games
	// TODO: This could be either turned into a game feature, or a version
	// specific check, instead of a game version check

	// The Apple IIGS versions of MH1 and Goldrush both have a parameter for
	// show.mouse and hide.mouse. Fixes bugs #3577754 and #3426946.
	if ((getGameID() == GID_MH1 || getGameID() == GID_GOLDRUSH) &&
		getPlatform() == Common::kPlatformApple2GS) {
		logicNamesCmd[176].args = "n";	// hide.mouse
		logicNamesCmd[178].args = "n";	// show.mouse
	}
}

}
