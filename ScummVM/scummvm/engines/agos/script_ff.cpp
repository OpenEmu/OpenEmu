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

#include "agos/animation.h"
#include "agos/agos.h"

namespace AGOS {

#define OPCODE(x)	_OPCODE(AGOSEngine_Feeble, x)

void AGOSEngine_Feeble::setupOpcodes() {
	static const OpcodeEntryFeeble opcodes[] = {
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
		OPCODE(o_invalid),
		OPCODE(o_destroy),
		/* 32 */
		OPCODE(o_invalid),
		OPCODE(o_place),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 36 */
		OPCODE(o_copyff),
		OPCODE(off_jumpOut),
		OPCODE(o_invalid),
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
		OPCODE(o_message),
		/* 64 */
		OPCODE(o_msg),
		OPCODE(off_addTextBox),
		OPCODE(oww_setShortText),
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
		OPCODE(o_invalid),
		OPCODE(o_invalid),
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
		OPCODE(o_sync),
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
		OPCODE(off_saveUserGame),
		OPCODE(off_loadUserGame),
		OPCODE(off_listSaveGames),
		OPCODE(off_checkCD),
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
		OPCODE(os1_screenTextMsg),
		OPCODE(o_invalid),
		/* 164 */
		OPCODE(oe2_getDollar2),
		OPCODE(off_isAdjNoun),
		OPCODE(oe2_b2Set),
		OPCODE(oe2_b2Clear),
		/* 168 */
		OPCODE(oe2_b2Zero),
		OPCODE(oe2_b2NotZero),
		OPCODE(o_invalid),
		OPCODE(off_hyperLinkOn),
		/* 172 */
		OPCODE(off_hyperLinkOff),
		OPCODE(off_checkPaths),
		OPCODE(o_invalid),
		OPCODE(oww_lockZones),
		/* 176 */
		OPCODE(oww_unlockZones),
		OPCODE(off_screenTextPObj),
		OPCODE(os1_getPathPosn),
		OPCODE(os1_scnTxtLongText),
		/* 180 */
		OPCODE(off_mouseOn),
		OPCODE(off_mouseOff),
		OPCODE(off_loadVideo),
		OPCODE(off_playVideo),
		/* 184 */
		OPCODE(os1_unloadZone),
		OPCODE(o_invalid),
		OPCODE(os1_unfreezeZones),
		OPCODE(off_centerScroll),
		/* 188 */
		OPCODE(os2_isShortText),
		OPCODE(os2_clearMarks),
		OPCODE(os2_waitMark),
		OPCODE(off_resetPVCount),
		/* 192 */
		OPCODE(off_setPathValues),
		OPCODE(off_stopClock),
		OPCODE(off_restartClock),
		OPCODE(off_setColor),
		/* 196 */
		OPCODE(off_b3Set),
		OPCODE(off_b3Clear),
		OPCODE(off_b3Zero),
		OPCODE(off_b3NotZero)
	};

	_opcodesFeeble = opcodes;
	_numOpcodes = 200;
}

void AGOSEngine_Feeble::executeOpcode(int opcode) {
	OpcodeProcFeeble op = _opcodesFeeble[opcode].proc;
	(this->*op) ();
}

// -----------------------------------------------------------------------
// Feeble Files Opcodes
// -----------------------------------------------------------------------

void AGOSEngine_Feeble::off_chance() {
	// 23
	uint16 a = getVarOrWord();

	if (a == 0) {
		setScriptCondition(false);
		return;
	}

	if (a == 100) {
		setScriptCondition(true);
		return;
	}

	if (_rnd.getRandomNumber(99) < a)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void AGOSEngine_Feeble::off_jumpOut() {
	// 37
	getVarOrByte();
	setScriptReturn(1);
}

void AGOSEngine_Feeble::off_addTextBox() {
	// 65: add hit area
	uint flags = kBFTextBox | kBFBoxItem;
	uint id = getVarOrWord();
	uint params = id / 1000;
	uint x, y, w, h, num;

	id %= 1000;

	if (params & 1)
		flags |= kBFInvertTouch;

	x = getVarOrWord();
	y = getVarOrWord();
	w = getVarOrWord();
	h = getVarOrWord();
	num = getVarOrByte();
	if (num < _numTextBoxes)
		defineBox(id, x, y, w, h, flags + (num << 8), 208, _dummyItem2);
}

void AGOSEngine_Feeble::off_printLongText() {
	// 70: show string from array
	int num = getVarOrByte();
	const char *str = (const char *)getStringPtrByID(_longText[num]);
	sendInteractText(num, "%d. %s\n", num, str);
}

void AGOSEngine_Feeble::off_addBox() {
	// 107: add item hitarea
	uint flags = 0;
	uint id = getVarOrWord();
	uint params = id / 1000;
	uint x, y, w, h, verb;
	Item *item;

	id = id % 1000;

	if (params & 1)
		flags |= kBFInvertTouch;
	if (params & 2)
		flags |= kBFNoTouchName;
	if (params & 4)
		flags |= kBFBoxItem;
	if (params & 8)
		flags |= kBFTextBox;
	if (params & 16)
		flags |= kBFHyperBox;

	x = getVarOrWord();
	y = getVarOrWord();
	w = getVarOrWord();
	h = getVarOrWord();
	item = getNextItemPtrStrange();
	verb = getVarOrWord();
	defineBox(id, x, y, w, h, flags, verb, item);
}

void AGOSEngine_Feeble::off_oracleTextDown() {
	// 122: oracle text down
	oracleTextDown();
}

void AGOSEngine_Feeble::off_oracleTextUp() {
	// 123: oracle text up
	oracleTextUp();
}

void AGOSEngine_Feeble::off_ifTime() {
	// 124: if time
	uint a = getVarOrWord();
	uint32 t = getTime() - _gameStoppedClock - a;
	if (t >= _timeStore)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void AGOSEngine_Feeble::off_setTime() {
	// 131
	_timeStore = getTime() - _gameStoppedClock;
}

void AGOSEngine_Feeble::off_saveUserGame() {
	// 132: save game
	_noOracleScroll = 0;
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	saveUserGame(countSaveGames() + 1 - readVariable(55));
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGOSEngine_Feeble::off_loadUserGame() {
	// 133: load game
	if (readVariable(55) == 999) {
		loadGame(getFileName(GAME_RESTFILE), true);
	} else {
		loadGame(genSaveName(readVariable(55)));
	}
}

void AGOSEngine_Feeble::off_listSaveGames() {
	listSaveGamesFeeble();
}

void AGOSEngine_Feeble::off_checkCD() {
	// 135: switch CD
	uint16 disc = readVariable(97);

	if (!strcmp(getExtra(), "4CD")) {
		_sound->switchVoiceFile(gss, disc);
	} else if (!strcmp(getExtra(), "2CD")) {
		if (disc == 1 || disc == 2)
			_sound->switchVoiceFile(gss, 1);
		else if (disc == 3 || disc == 4)
			_sound->switchVoiceFile(gss, 2);
	}

	debug(0, "Switch to CD number %d", disc);
}

void AGOSEngine_Feeble::off_screenTextBox() {
	// 161: setup text
	TextLocation *tl = getTextLocation(getVarOrByte());

	tl->x = getVarOrWord();
	tl->y = getVarOrWord();
	tl->width = getVarOrWord();
}

void AGOSEngine_Feeble::off_isAdjNoun() {
	// 165: item unk1 unk2 is
	Item *item = getNextItemPtr();
	int16 a = getNextWord(), b = getNextWord();
	if (item->adjective == a && item->noun == b)
		setScriptCondition(true);
	else if (a == -1 && item->noun == b)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void AGOSEngine_Feeble::off_hyperLinkOn() {
	// 171: oracle hyperlink on
	hyperLinkOn(getVarOrWord());
}

void AGOSEngine_Feeble::off_hyperLinkOff() {
	// 172: oracle hyperlink off
	hyperLinkOff();
}

void AGOSEngine_Feeble::off_checkPaths() {
	// 173 check paths
	int i, count;
	const uint8 *pathVal1 = _pathValues1;
	bool result = false;

	count = _variableArray2[38];
	for (i = 0; i < count; i++) {
		uint8 val = pathVal1[2];
		if (val == _variableArray2[50] ||
			val == _variableArray2[51] ||
			val == _variableArray2[201] ||
			val == _variableArray2[203] ||
			val == _variableArray2[205] ||
			val == _variableArray2[207] ||
			val == _variableArray2[209] ||
			val == _variableArray2[211] ||
			val == _variableArray2[213] ||
			val == _variableArray2[215] ||
			val == _variableArray2[219] ||
			val == _variableArray2[220] ||
			val == _variableArray2[221] ||
			val == _variableArray2[222] ||
			val == _variableArray2[223] ||
			val == _variableArray2[224] ||
			val == _variableArray2[225] ||
			val == _variableArray2[226]) {
				result = true;
				break;
		}
		pathVal1 += 4;
	}

	_variableArray2[52] = result;
}

void AGOSEngine_Feeble::off_screenTextPObj() {
	// 177: inventory descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	const char *string_ptr = NULL;
	TextLocation *tl = NULL;
	char buf[256];

	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), kObjectType);
	if (subObject != NULL && subObject->objectFlags & kOFText) {
		string_ptr = (const char *)getStringPtrByID(subObject->objectFlagValue[0]);
		tl = getTextLocation(vgaSpriteId);
	}

	if (subObject != NULL && subObject->objectFlags & kOFVoice) {
		uint offs = getOffsetOfChild2Param(subObject, kOFVoice);
		playSpeech(subObject->objectFlagValue[offs], vgaSpriteId);
	}

	if (subObject != NULL && (subObject->objectFlags & kOFText) && _subtitles) {
		if (subObject->objectFlags & kOFNumber) {
			sprintf(buf, "%d%s", subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)], string_ptr);
			string_ptr = buf;
		}
		if (string_ptr != NULL)
			printScreenText(vgaSpriteId, color, string_ptr, tl->x, tl->y, tl->width);
	}
}

void AGOSEngine_Feeble::off_mouseOn() {
	// 180: force mouseOn
	if (_mouseCursor != 5) {
		resetVerbs();
		_noRightClick = 0;
	}
	_mouseHideCount = 0;
}

void AGOSEngine_Feeble::off_mouseOff() {
	// 181: force mouseOff
	scriptMouseOff();
	clearName();
}

void AGOSEngine_Feeble::off_loadVideo() {
	// 182: load video file
	const byte *filename = getStringPtrByID(getNextStringID());

	_moviePlayer = makeMoviePlayer(this, (const char *)filename);

	assert(_moviePlayer);
	_moviePlayer->load();
}

void AGOSEngine_Feeble::off_playVideo() {
	// 183: play video
	if (getBitFlag(40)) {
		// Omni TV controls
		if (_moviePlayer) {
			setBitFlag(42, false);
			_interactiveVideo = MoviePlayer::TYPE_OMNITV;
			_moviePlayer->play();
		} else {
			_variableArray[254] = 6747;
		}
	} else {
		assert(_moviePlayer);
		_moviePlayer->play();

		delete _moviePlayer;
		_moviePlayer = NULL;
	}
}

void AGOSEngine_Feeble::off_centerScroll() {
	// 187
	centerScroll();
}

void AGOSEngine_Feeble::off_resetPVCount() {
	// 191
	if (getBitFlag(83)) {
		_PVCount1 = 0;
		_GPVCount1 = 0;
	} else {
		_PVCount = 0;
		_GPVCount = 0;
	}
}

void AGOSEngine_Feeble::off_setPathValues() {
	// 192
	uint8 a = getVarOrByte();
	uint8 b = getVarOrByte();
	uint8 c = getVarOrByte();
	uint8 d = getVarOrByte();
	if (getBitFlag(83)) {
		_pathValues1[_PVCount1++] = a;
		_pathValues1[_PVCount1++] = b;
		_pathValues1[_PVCount1++] = c;
		_pathValues1[_PVCount1++] = d;
	} else {
		_pathValues[_PVCount++] = a;
		_pathValues[_PVCount++] = b;
		_pathValues[_PVCount++] = c;
		_pathValues[_PVCount++] = d;
	}
}

void AGOSEngine_Feeble::off_stopClock() {
	// 193: pause clock
	_clockStopped = getTime();
}

void AGOSEngine_Feeble::off_restartClock() {
	// 194: resume clock
	if (_clockStopped != 0)
		_gameStoppedClock += getTime() - _clockStopped;
	_clockStopped = 0;
}

void AGOSEngine_Feeble::off_setColor() {
	// 195: set palette color
	uint16 c = getVarOrByte() * 3;
	uint8 r = getVarOrByte();
	uint8 g = getVarOrByte();
	uint8 b = getVarOrByte();

	_displayPalette[c + 0] = r;
	_displayPalette[c + 1] = g;
	_displayPalette[c + 2] = b;

	_paletteFlag = 2;
}

void AGOSEngine_Feeble::off_b3Set() {
	// 196: set bit3
	uint8 bit = getVarOrByte();
	_bitArrayThree[bit / 16] |= (1 << (bit & 15));
}

void AGOSEngine_Feeble::off_b3Clear() {
	// 197: clear bit3
	uint8 bit = getVarOrByte();
	_bitArrayThree[bit / 16] &= ~(1 << (bit & 15));
}

void AGOSEngine_Feeble::off_b3Zero() {
	// 198: is bit3 clear
	uint8 bit = getVarOrByte();
	setScriptCondition((_bitArrayThree[bit / 16] & (1 << (bit & 15))) == 0);
}

void AGOSEngine_Feeble::off_b3NotZero() {
	// 199: is bit3 set
	uint8 bit = getVarOrByte();
	setScriptCondition((_bitArrayThree[bit / 16] & (1 << (bit & 15))) != 0);
}

} // End of namespace AGOS

#endif // ENABLE_AGOS2
