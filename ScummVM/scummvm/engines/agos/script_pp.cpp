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



#ifdef ENABLE_AGOS2

#include "common/system.h"

#include "agos/agos.h"

namespace AGOS {

#define OPCODE(x)	_OPCODE(AGOSEngine_PuzzlePack, x)

void AGOSEngine_PuzzlePack::setupOpcodes() {
	static const OpcodeEntryPuzzlePack opcodes[] = {
		/* 00 */
		OPCODE(o_invalid),
		OPCODE(o_at),
		OPCODE(o_notAt),
		OPCODE(o_invalid),
		/* 04 */
		OPCODE(o_invalid),
		OPCODE(o_carried),
		OPCODE(o_notCarried),
		OPCODE(o_isAt),
		/* 08 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_zero),
		/* 12 */
		OPCODE(o_notZero),
		OPCODE(o_eq),
		OPCODE(o_notEq),
		OPCODE(o_gt),
		/* 16 */
		OPCODE(o_lt),
		OPCODE(o_eqf),
		OPCODE(o_notEqf),
		OPCODE(o_ltf),
		/* 20 */
		OPCODE(o_gtf),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(off_chance),
		/* 24 */
		OPCODE(o_invalid),
		OPCODE(o_isRoom),
		OPCODE(o_isObject),
		OPCODE(o_state),
		/* 28 */
		OPCODE(o_oflag),
		OPCODE(o_invalid),
		OPCODE(opp_iconifyWindow),
		OPCODE(o_destroy),
		/* 32 */
		OPCODE(opp_restoreOopsPosition),
		OPCODE(o_place),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 36 */
		OPCODE(o_copyff),
		OPCODE(o_invalid),
		OPCODE(opp_loadMouseImage),
		OPCODE(o_invalid),
		/* 40 */
		OPCODE(o_invalid),
		OPCODE(o_clear),
		OPCODE(o_let),
		OPCODE(o_add),
		/* 44 */
		OPCODE(o_sub),
		OPCODE(o_addf),
		OPCODE(o_subf),
		OPCODE(o_mul),
		/* 48 */
		OPCODE(o_div),
		OPCODE(o_mulf),
		OPCODE(o_divf),
		OPCODE(o_mod),
		/* 52 */
		OPCODE(o_modf),
		OPCODE(o_random),
		OPCODE(o_invalid),
		OPCODE(o_goto),
		/* 56 */
		OPCODE(o_oset),
		OPCODE(o_oclear),
		OPCODE(o_putBy),
		OPCODE(o_inc),
		/* 60 */
		OPCODE(o_dec),
		OPCODE(o_setState),
		OPCODE(o_print),
		OPCODE(opp_message),
		/* 64 */
		OPCODE(o_msg),
		OPCODE(off_addTextBox),
		OPCODE(opp_setShortText),
		OPCODE(oww_setLongText),
		/* 68 */
		OPCODE(o_end),
		OPCODE(o_done),
		OPCODE(off_printLongText),
		OPCODE(o_process),
		/* 72 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 76 */
		OPCODE(o_when),
		OPCODE(o_if1),
		OPCODE(o_if2),
		OPCODE(o_isCalled),
		/* 80 */
		OPCODE(o_is),
		OPCODE(o_invalid),
		OPCODE(o_debug),
		OPCODE(os2_rescan),
		/* 84 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_comment),
		/* 88 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_getParent),
		OPCODE(o_getNext),
		/* 92 */
		OPCODE(o_getChildren),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 96 */
		OPCODE(o_picture),
		OPCODE(o_loadZone),
		OPCODE(os2_animate),
		OPCODE(os2_stopAnimate),
		/* 100 */
		OPCODE(o_killAnimate),
		OPCODE(o_defWindow),
		OPCODE(o_window),
		OPCODE(o_cls),
		/* 104 */
		OPCODE(o_closeWindow),
		OPCODE(opp_loadHiScores),
		OPCODE(opp_checkHiScores),
		OPCODE(off_addBox),
		/* 108 */
		OPCODE(o_delBox),
		OPCODE(o_enableBox),
		OPCODE(o_disableBox),
		OPCODE(o_moveBox),
		/* 112 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_doIcons),
		OPCODE(o_isClass),
		/* 116 */
		OPCODE(o_setClass),
		OPCODE(o_unsetClass),
		OPCODE(o_invalid),
		OPCODE(o_waitSync),
		/* 120 */
		OPCODE(opp_sync),
		OPCODE(o_defObj),
		OPCODE(off_oracleTextDown),
		OPCODE(off_oracleTextUp),
		/* 124 */
		OPCODE(off_ifTime),
		OPCODE(o_here),
		OPCODE(o_doClassIcons),
		OPCODE(o_invalid),
		/* 128 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_setAdjNoun),
		OPCODE(off_setTime),
		/* 132 */
		OPCODE(opp_saveUserGame),
		OPCODE(opp_loadUserGame),
		OPCODE(off_listSaveGames),
		OPCODE(o_invalid),
		/* 136 */
		OPCODE(o_copysf),
		OPCODE(o_restoreIcons),
		OPCODE(o_freezeZones),
		OPCODE(o_placeNoIcons),
		/* 140 */
		OPCODE(o_clearTimers),
		OPCODE(o_setDollar),
		OPCODE(o_isBox),
		OPCODE(oe2_doTable),
		/* 144 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 148 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(oe2_storeItem),
		/* 152 */
		OPCODE(oe2_getItem),
		OPCODE(oe2_bSet),
		OPCODE(oe2_bClear),
		OPCODE(oe2_bZero),
		/* 156 */
		OPCODE(oe2_bNotZero),
		OPCODE(oe2_getOValue),
		OPCODE(oe2_setOValue),
		OPCODE(o_invalid),
		/* 160 */
		OPCODE(oe2_ink),
		OPCODE(off_screenTextBox),
		OPCODE(opp_playTune),
		OPCODE(o_invalid),
		/* 164 */
		OPCODE(oe2_getDollar2),
		OPCODE(off_isAdjNoun),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 168 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(off_hyperLinkOn),
		/* 172 */
		OPCODE(off_hyperLinkOff),
		OPCODE(opp_saveOopsPosition),
		OPCODE(o_invalid),
		OPCODE(oww_lockZones),
		/* 176 */
		OPCODE(oww_unlockZones),
		OPCODE(off_screenTextPObj),
		OPCODE(os1_getPathPosn),
		OPCODE(os1_scnTxtLongText),
		/* 180 */
		OPCODE(os1_mouseOn),
		OPCODE(off_mouseOff),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 184 */
		OPCODE(os1_unloadZone),
		OPCODE(o_invalid),
		OPCODE(os1_unfreezeZones),
		OPCODE(opp_resetGameTime),
		/* 188 */
		OPCODE(os2_isShortText),
		OPCODE(os2_clearMarks),
		OPCODE(os2_waitMark),
		OPCODE(opp_resetPVCount),
		/* 192 */
		OPCODE(opp_setPathValues),
		OPCODE(off_restartClock),
		OPCODE(opp_pauseClock),
		OPCODE(off_setColor),
	};

	_opcodesPuzzlePack = opcodes;
	_numOpcodes = 196;
}

void AGOSEngine_PuzzlePack::executeOpcode(int opcode) {
	OpcodeProcPuzzlePack op = _opcodesPuzzlePack[opcode].proc;
	(this->*op) ();
}

// -----------------------------------------------------------------------
// Puzzle Pack Opcodes
// -----------------------------------------------------------------------

void AGOSEngine_PuzzlePack::opp_iconifyWindow() {
	// 30
	getNextWord();
	if (_clockStopped != 0)
		_gameTime += getTime() - _clockStopped;
	_clockStopped = 0;
	_system->setFeatureState(OSystem::kFeatureIconifyWindow, true);
}

void AGOSEngine_PuzzlePack::opp_restoreOopsPosition() {
	// 32: restore oops position
	uint i;

	getNextWord();
	getNextWord();

	if (_oopsValid) {
		for (i = 0; i < _numVars; i++) {
			_variableArray[i] = _variableArray2[i];
		}
		i = _variableArray[999] * 100 + 11;
		setWindowImage(4,i);
		_gameTime += 10;
		// Swampy adventures
		if (!getBitFlag(110))
			_gameTime += 20;
		_oopsValid = false;
	}
}

void AGOSEngine_PuzzlePack::opp_loadMouseImage() {
	// 38: load mouse image
	getNextWord();
	getVarOrByte();
	loadMouseImage();
}

void AGOSEngine_PuzzlePack::opp_message() {
	// 63: show string nl
	const byte *stringPtr = getStringPtrByID(getNextStringID());
	if (getBitFlag(105)) {
		// Swampy adventures
		printInfoText((const char *)stringPtr);
	} else {
		showMessageFormat("%s\n", stringPtr);
	}
}

void AGOSEngine_PuzzlePack::opp_setShortText() {
	// 66: set item name
	uint var = getVarOrByte();
	uint stringId = getNextStringID();
	if (var < _numTextBoxes) {
		_shortText[var] = stringId;
		_shortTextX[var] = getVarOrWord();
		_shortTextY[var] = getVarOrWord();
	}
}

void AGOSEngine_PuzzlePack::opp_loadHiScores() {
	// 105: load high scores
	getVarOrByte();
	//loadHiScores();
}

void AGOSEngine_PuzzlePack::opp_checkHiScores() {
	// 106: check high scores
	getVarOrByte();
	getVarOrByte();
	//checkHiScores();
}

void AGOSEngine_PuzzlePack::opp_sync() {
	// 120: sync
	uint a = getVarOrWord();
	if (a == 8001 || a == 8101 || a == 8201 || a == 8301 || a == 8401) {
		_marks &= ~(1 << 2);
	}
	sendSync(a);
}

void AGOSEngine_PuzzlePack::opp_saveUserGame() {
	// 132: save game
	if (_clockStopped != 0)
		_gameTime += getTime() - _clockStopped;
	_clockStopped = 0;

	if (!getBitFlag(110)) {
		// Swampy adventures
		saveGame(1, NULL);
	}
	//saveHiScores()
}

void AGOSEngine_PuzzlePack::opp_loadUserGame() {
	// 133: load usergame

	// NoPatience or Jumble
	if (getBitFlag(110)) {
		//getHiScoreName();
		return;
	}

	// XXX
	loadGame(genSaveName(1));
}

void AGOSEngine_PuzzlePack::opp_playTune() {
	// 162: play tune
	getVarOrByte();
	getVarOrByte();
	getNextWord();

	uint16 music = (uint16)getVarOrWord();
	if (music != _lastMusicPlayed) {
		_lastMusicPlayed = music;
		playSpeech(music, 1);
	}
}

void AGOSEngine_PuzzlePack::opp_saveOopsPosition() {
	// 173: save oops position
	if (!isVgaQueueEmpty()) {
		_oopsValid = true;
		for (uint i = 0; i < _numVars; i++) {
			_variableArray2[i] = _variableArray[i];
		}
	} else {
		_oopsValid = false;
	}
}

void AGOSEngine_PuzzlePack::opp_resetGameTime() {
	// 187: reset game time
	_gameTime = 0;
}

void AGOSEngine_PuzzlePack::opp_resetPVCount() {
	// 191
	_PVCount = 0;
	_GPVCount = 0;
}

void AGOSEngine_PuzzlePack::opp_setPathValues() {
	// 192
	_pathValues[_PVCount++] = getVarOrByte();
	_pathValues[_PVCount++] = getVarOrByte();
	_pathValues[_PVCount++] = getVarOrByte();
	_pathValues[_PVCount++] = getVarOrByte();
}

void AGOSEngine_PuzzlePack::opp_pauseClock() {
	// 194: pause clock
	if (_clockStopped == 0)
		_clockStopped = getTime();
}

} // End of namespace AGOS

#endif // ENABLE_AGOS2
