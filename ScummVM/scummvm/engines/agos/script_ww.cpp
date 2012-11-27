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



#include "agos/agos.h"

namespace AGOS {

#define OPCODE(x)	_OPCODE(AGOSEngine_Waxworks, x)

void AGOSEngine_Waxworks::setupOpcodes() {
	static const OpcodeEntryWaxworks opcodes[] = {
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
		OPCODE(oe1_isNotAt),
		OPCODE(oe1_sibling),
		OPCODE(oe1_notSibling),
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
		OPCODE(oe1_isIn),
		OPCODE(oe1_isNotIn),
		OPCODE(o_chance),
		/* 24 */
		OPCODE(oe1_isPlayer),
		OPCODE(o_isRoom),
		OPCODE(o_isObject),
		OPCODE(o_state),
		/* 28 */
		OPCODE(o_oflag),
		OPCODE(oe1_canPut),
		OPCODE(o_invalid),
		OPCODE(o_destroy),
		/* 32 */
		OPCODE(o_invalid),
		OPCODE(o_place),
		OPCODE(oe1_copyof),
		OPCODE(oe1_copyfo),
		/* 36 */
		OPCODE(o_copyff),
		OPCODE(oe1_whatO),
		OPCODE(o_invalid),
		OPCODE(oe1_weigh),
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
		OPCODE(oe2_moveDirn),
		OPCODE(oww_goto),
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
		OPCODE(oww_addTextBox),
		OPCODE(oww_setShortText),
		OPCODE(oww_setLongText),
		/* 68 */
		OPCODE(o_end),
		OPCODE(o_done),
		OPCODE(o_invalid),
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
		OPCODE(oe1_rescan),
		/* 84 */
		OPCODE(o_invalid),
		OPCODE(oww_whereTo),
		OPCODE(o_invalid),
		OPCODE(o_comment),
		/* 88 */
		OPCODE(o_invalid),
		OPCODE(oe1_loadGame),
		OPCODE(o_getParent),
		OPCODE(o_getNext),
		/* 92 */
		OPCODE(o_getChildren),
		OPCODE(o_invalid),
		OPCODE(oe1_findMaster),
		OPCODE(oe1_nextMaster),
		/* 96 */
		OPCODE(o_picture),
		OPCODE(o_loadZone),
		OPCODE(oe1_animate),
		OPCODE(oe1_stopAnimate),
		/* 100 */
		OPCODE(o_killAnimate),
		OPCODE(o_defWindow),
		OPCODE(o_window),
		OPCODE(o_cls),
		/* 104 */
		OPCODE(o_closeWindow),
		OPCODE(oe2_menu),
		OPCODE(oww_textMenu),
		OPCODE(o_addBox),
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
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 124 */
		OPCODE(o_invalid),
		OPCODE(o_here),
		OPCODE(o_doClassIcons),
		OPCODE(o_playTune),
		/* 128 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_setAdjNoun),
		OPCODE(o_invalid),
		/* 132 */
		OPCODE(o_saveUserGame),
		OPCODE(o_loadUserGame),
		OPCODE(o_invalid),
		OPCODE(oww_pauseGame),
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
		OPCODE(oe2_setDoorOpen),
		OPCODE(oe2_setDoorClosed),
		OPCODE(oe2_setDoorLocked),
		OPCODE(oe2_setDoorClosed),
		/* 148 */
		OPCODE(oe2_ifDoorOpen),
		OPCODE(oe2_ifDoorClosed),
		OPCODE(oe2_ifDoorLocked),
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
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 164 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 168 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 172 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(oe2_getDollar2),
		/* 176 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(oe2_isAdjNoun),
		/* 180 */
		OPCODE(oe2_b2Set),
		OPCODE(oe2_b2Clear),
		OPCODE(oe2_b2Zero),
		OPCODE(oe2_b2NotZero),
		/* 184 */
		OPCODE(oww_boxMessage),
		OPCODE(oww_boxMsg),
		OPCODE(oww_boxLongText),
		OPCODE(oww_printBox),
		/* 188 */
		OPCODE(oww_boxPObj),
		OPCODE(oww_lockZones),
		OPCODE(oww_unlockZones),
	};

	_opcodesWaxworks = opcodes;
	_numOpcodes = 191;
}

void AGOSEngine_Waxworks::executeOpcode(int opcode) {
	OpcodeProcWaxworks op = _opcodesWaxworks[opcode].proc;
	(this->*op) ();
}

// -----------------------------------------------------------------------
// Waxworks Opcodes
// -----------------------------------------------------------------------

void AGOSEngine_Waxworks::oww_goto() {
	// 55: set itemA parent
	uint item = getNextItemID();
	if (derefItem(item) == NULL) {
		setItemParent(me(), NULL);
		loadRoomItems(item);
	}
	setItemParent(me(), derefItem(item));
}

void AGOSEngine_Waxworks::oww_addTextBox() {
	// 65: add hit area
	uint id = getVarOrWord();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint w = getVarOrWord();
	uint h = getVarOrWord();
	uint number = getVarOrByte();
	if (number < _numTextBoxes)
		defineBox(id, x, y, w, h, (number << 8) + 129, 208, _dummyItem2);
}

void AGOSEngine_Waxworks::oww_setShortText() {
	// 66: set item name
	uint var = getVarOrByte();
	uint stringId = getNextStringID();
	if (var < _numTextBoxes) {
		_shortText[var] = stringId;
	}
}

void AGOSEngine_Waxworks::oww_setLongText() {
	// 67: set item description
	uint var = getVarOrByte();
	uint stringId = getNextStringID();
	if (getFeatures() & GF_TALKIE) {
		uint speechId = getNextWord();
		if (var < _numTextBoxes) {
			_longText[var] = stringId;
			_longSound[var] = speechId;
		}
	} else {
		if (var < _numTextBoxes) {
			_longText[var] = stringId;
		}
	}
}

void AGOSEngine_Waxworks::oww_printLongText() {
	// 70: show string from array
	const char *str = (const char *)getStringPtrByID(_longText[getVarOrByte()]);
	showMessageFormat("%s\n", str);
}

void AGOSEngine_Waxworks::oww_whereTo() {
	// 85: where to
	Item *i = getNextItemPtr();
	int16 d = getVarOrByte();
	int16 f = getVarOrByte();

	if (f == 1)
		_subjectItem = derefItem(getExitOf(i, d));
	else
		_objectItem = derefItem(getExitOf(i, d));
}

void AGOSEngine_Waxworks::oww_textMenu() {
	// 106: set text menu
	byte slot = getVarOrByte();
	_textMenu[slot] = getVarOrByte();
}

void AGOSEngine_Waxworks::oww_pauseGame() {
	// 135: pause game
	HitArea *ha;

	uint32 pauseTime = getTime();
	haltAnimation();

	while (!shouldQuit()) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		while (!shouldQuit()) {
			if (_lastHitArea3 != 0)
				break;
			delay(1);
		}

		ha = _lastHitArea;

		if (ha == NULL) {
		} else if (ha->id == 200) {
			break;
		} else if (ha->id == 201) {
			break;
		}
	}

	restartAnimation();
	_gameStoppedClock = getTime() - pauseTime + _gameStoppedClock;
}

void AGOSEngine_Waxworks::oww_boxMessage() {
	// 184: print message to box
	boxTextMessage((const char *)getStringPtrByID(getNextStringID()));
}

void AGOSEngine_Waxworks::oww_boxMsg() {
	// 185: print msg to box
	boxTextMsg((const char *)getStringPtrByID(getNextStringID()));
}

void AGOSEngine_Waxworks::oww_boxLongText() {
	// 186: print long text to box
	boxTextMsg((const char *)getStringPtrByID(_longText[getVarOrByte()]));
}

void AGOSEngine_Waxworks::oww_printBox() {
	// 187: print box
	printBox();
}

void AGOSEngine_Waxworks::oww_boxPObj() {
	// 188: print object name to box
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), kObjectType);

	if (subObject != NULL && subObject->objectFlags & kOFText)
		boxTextMsg((const char *)getStringPtrByID(subObject->objectFlagValue[0]));
}

void AGOSEngine_Waxworks::oww_lockZones() {
	// 189: lock zone
	_vgaMemBase = _vgaMemPtr;
}

void AGOSEngine_Waxworks::oww_unlockZones() {
	// 190: unlock zone
	_vgaMemPtr = _vgaFrozenBase;
	_vgaMemBase = _vgaFrozenBase;
}

} // End of namespace AGOS
