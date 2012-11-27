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
#include "agos/vga.h"

namespace AGOS {

#define OPCODE(x)	_OPCODE(AGOSEngine_Elvira1, x)

void AGOSEngine_Elvira1::setupOpcodes() {
	static const OpcodeEntryElvira1 opcodes[] = {
		/* 00 */
		OPCODE(o_at),
		OPCODE(o_notAt),
		OPCODE(oe1_present),
		OPCODE(oe1_notPresent),
		/* 04 */
		OPCODE(oe1_worn),
		OPCODE(oe1_notWorn),
		OPCODE(o_carried),
		OPCODE(o_notCarried),
		/* 08 */
		OPCODE(o_isAt),
		OPCODE(oe1_isNotAt),
		OPCODE(oe1_sibling),
		OPCODE(oe1_notSibling),
		/* 12 */
		OPCODE(o_zero),
		OPCODE(o_notZero),
		OPCODE(o_eq),
		OPCODE(o_notEq),
		/* 16 */
		OPCODE(o_gt),
		OPCODE(o_lt),
		OPCODE(o_eqf),
		OPCODE(o_notEqf),
		/* 20 */
		OPCODE(o_ltf),
		OPCODE(o_gtf),
		OPCODE(oe1_isIn),
		OPCODE(oe1_isNotIn),
		/* 24 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 28 */
		OPCODE(o_invalid),
		OPCODE(o_chance),
		OPCODE(oe1_isPlayer),
		OPCODE(o_invalid),
		/* 32 */
		OPCODE(o_isRoom),
		OPCODE(o_isObject),
		OPCODE(o_state),
		OPCODE(o_invalid),
		/* 36 */
		OPCODE(o_oflag),
		OPCODE(oe1_canPut),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 40 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 44 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(oe1_create),
		/* 48 */
		OPCODE(o_destroy),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_place),
		/* 52 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(oe1_copyof),
		OPCODE(oe1_copyfo),
		/* 56 */
		OPCODE(o_copyff),
		OPCODE(oe1_whatO),
		OPCODE(o_invalid),
		OPCODE(oe1_weigh),
		/* 60 */
		OPCODE(oe1_setFF),
		OPCODE(o_clear),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 64 */
		OPCODE(o_let),
		OPCODE(o_add),
		OPCODE(o_sub),
		OPCODE(o_addf),
		/* 68 */
		OPCODE(o_subf),
		OPCODE(o_mul),
		OPCODE(o_div),
		OPCODE(o_mulf),
		/* 72 */
		OPCODE(o_divf),
		OPCODE(o_mod),
		OPCODE(o_modf),
		OPCODE(o_random),
		/* 76 */
		OPCODE(oe1_moveDirn),
		OPCODE(o_goto),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 80 */
		OPCODE(o_oset),
		OPCODE(o_oclear),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 84 */
		OPCODE(o_putBy),
		OPCODE(o_inc),
		OPCODE(o_dec),
		OPCODE(o_setState),
		/* 88 */
		OPCODE(o_invalid),
		OPCODE(o_print),
		OPCODE(oe1_score),
		OPCODE(o_message),
		/* 92 */
		OPCODE(o_msg),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 96 */
		OPCODE(oe1_look),
		OPCODE(o_end),
		OPCODE(o_done),
		OPCODE(o_invalid),
		/* 100 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 104 */
		OPCODE(o_invalid),
		OPCODE(o_process),
		OPCODE(oe1_doClass),
		OPCODE(o_invalid),
		/* 108 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 112*/
		OPCODE(oe1_pObj),
		OPCODE(o_invalid),
		OPCODE(oe1_pName),
		OPCODE(oe1_pcName),
		/* 116 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_when),
		/* 120 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 124 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 128 */
		OPCODE(o_if1),
		OPCODE(o_if2),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 132 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(oe1_isCalled),
		/* 136 */
		OPCODE(o_is),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 140 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 144 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 148 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 152 */
		OPCODE(o_debug),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 156 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 160 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(oe1_cFlag),
		OPCODE(o_invalid),
		/* 164 */
		OPCODE(oe1_rescan),
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
		OPCODE(o_invalid),
		/* 176 */
		OPCODE(oe1_setUserItem),
		OPCODE(oe1_getUserItem),
		OPCODE(oe1_clearUserItem),
		OPCODE(o_invalid),
		/* 180 */
		OPCODE(oe1_whereTo),
		OPCODE(oe1_doorExit),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 184 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 188 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 192 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 196 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_comment),
		OPCODE(o_invalid),
		/* 200 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(oe1_loadGame),
		OPCODE(o_invalid),
		/* 204 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_getParent),
		OPCODE(o_getNext),
		/* 208 */
		OPCODE(o_getChildren),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 212 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 216 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(oe1_findMaster),
		/* 220 */
		OPCODE(oe1_nextMaster),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 224 */
		OPCODE(o_picture),
		OPCODE(o_loadZone),
		OPCODE(oe1_animate),
		OPCODE(oe1_stopAnimate),
		/* 228 */
		OPCODE(o_killAnimate),
		OPCODE(o_defWindow),
		OPCODE(o_window),
		OPCODE(o_cls),
		/* 232 */
		OPCODE(o_closeWindow),
		OPCODE(oe1_menu),
		OPCODE(o_invalid),
		OPCODE(oe1_addBox),
		/* 236 */
		OPCODE(o_delBox),
		OPCODE(o_enableBox),
		OPCODE(o_disableBox),
		OPCODE(o_moveBox),
		/* 240 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_doIcons),
		OPCODE(o_isClass),
		/* 244 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 248 */
		OPCODE(o_invalid),
		OPCODE(o_setClass),
		OPCODE(o_unsetClass),
		OPCODE(oe1_bitClear),
		/* 252 */
		OPCODE(oe1_bitSet),
		OPCODE(oe1_bitTest),
		OPCODE(o_invalid),
		OPCODE(o_waitSync),
		/* 256 */
		OPCODE(o_sync),
		OPCODE(o_defObj),
		OPCODE(oe1_enableInput),
		OPCODE(oe1_setTime),
		/* 260 */
		OPCODE(oe1_ifTime),
		OPCODE(o_here),
		OPCODE(o_doClassIcons),
		OPCODE(oe1_playTune),
		/* 264 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_setAdjNoun),
		OPCODE(oe1_zoneDisk),
		/* 268 */
		OPCODE(o_saveUserGame),
		OPCODE(o_loadUserGame),
		OPCODE(oe1_printStats),
		OPCODE(oe1_stopTune),
		/* 272 */
		OPCODE(oe1_printPlayerDamage),
		OPCODE(oe1_printMonsterDamage),
		OPCODE(oe1_pauseGame),
		OPCODE(o_copysf),
		/* 276 */
		OPCODE(o_restoreIcons),
		OPCODE(oe1_printPlayerHit),
		OPCODE(oe1_printMonsterHit),
		OPCODE(o_freezeZones),
		/* 280 */
		OPCODE(o_placeNoIcons),
		OPCODE(o_clearTimers),
		OPCODE(o_setDollar),
		OPCODE(o_isBox)
	};

	_opcodesElvira1 = opcodes;
	_numOpcodes = 284;
}

void AGOSEngine_Elvira1::executeOpcode(int opcode) {
	OpcodeProcElvira1 op = _opcodesElvira1[opcode].proc;
	(this->*op) ();
}

// -----------------------------------------------------------------------
// Elvira 1 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine_Elvira1::oe1_present() {
	// 2: present (here or carried)
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent == getItem1ID() || item->parent == me()->parent);
}

void AGOSEngine_Elvira1::oe1_notPresent() {
	// 3: not present (neither here nor carried)
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent != getItem1ID() && item->parent != me()->parent);
}

void AGOSEngine_Elvira1::oe1_worn() {
	// 4: worn
	Item *item = getNextItemPtr();
	SubObject *subObject = (SubObject *)findChildOfType(item, kObjectType);

	if (item->parent != getItem1ID() || subObject == NULL)
		setScriptCondition(false);
	else
		setScriptCondition((subObject->objectFlags & kOFWorn) != 0);
}

void AGOSEngine_Elvira1::oe1_notWorn() {
	// 5: not worn
	Item *item = getNextItemPtr();
	SubObject *subObject = (SubObject *)findChildOfType(item, kObjectType);

	if (item->parent != getItem1ID() || subObject == NULL)
		setScriptCondition(false);
	else
		setScriptCondition((subObject->objectFlags & kOFWorn) == 0);
}

void AGOSEngine_Elvira1::oe1_isNotAt() {
	// 9: parent is not
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent != getNextItemID());
}

void AGOSEngine_Elvira1::oe1_sibling() {
	// 10: sibling
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(item1->parent == item2->parent);
}

void AGOSEngine_Elvira1::oe1_notSibling() {
	// 11: not sibling
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(item1->parent != item2->parent);
}

void AGOSEngine_Elvira1::oe1_isIn() {
	// 22: is in
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(contains(item1, item2) != 0);
}

void AGOSEngine_Elvira1::oe1_isNotIn() {
	// 23: is not in
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(contains(item1, item2) == 0);
}

void AGOSEngine_Elvira1::oe1_isPlayer() {
	// 30: is player
	setScriptCondition(isPlayer(getNextItemPtr()));
}

void AGOSEngine_Elvira1::oe1_canPut() {
	// 37: can put
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(canPlace(item1, item2) == 0);
}

void AGOSEngine_Elvira1::oe1_create() {
	// 47: create
	setItemParent(getNextItemPtr(), derefItem(me()->parent));
}

void AGOSEngine_Elvira1::oe1_copyof() {
	// 54: copy of
	Item *item = getNextItemPtr();
	uint tmp = getVarOrByte();
	writeNextVarContents(getUserFlag(item, tmp));
}

void AGOSEngine_Elvira1::oe1_copyfo() {
	// 55: copy fo
	uint tmp = getNextVarContents();
	Item *item = getNextItemPtr();
	setUserFlag(item, getVarOrByte(), tmp);
}

void AGOSEngine_Elvira1::oe1_whatO() {
	// 57: what o
	int a = getVarOrWord();

	if (a == 1)
		_subjectItem = findMaster(_scriptAdj1,_scriptNoun1);
	else
		_objectItem = findMaster(_scriptAdj2, _scriptNoun2);
}

void AGOSEngine_Elvira1::oe1_weigh() {
	// 59: weight
	Item *item = getNextItemPtr();
	writeNextVarContents(weighUp(item));
}

void AGOSEngine_Elvira1::oe1_setFF() {
	// 60: set FF
	writeNextVarContents(255);
}

void AGOSEngine_Elvira1::oe1_moveDirn() {
	// 54: move direction
	int16 d = readVariable(getVarOrWord());
	moveDirn(me(), d);
}

void AGOSEngine_Elvira1::oe1_score() {
	// 90: score
	SubPlayer *p = (SubPlayer *)findChildOfType(me(), kPlayerType);
	showMessageFormat("Your score is %d.\n", p->score);
}

void AGOSEngine_Elvira1::oe1_look() {
	// 96: look
	Item *i = derefItem(me()->parent);
	if (i == NULL)
		return;

	SubRoom *r = (SubRoom *)findChildOfType(i, kRoomType);
	SubObject *o = (SubObject *)findChildOfType(i, kObjectType);
	SubPlayer *p = (SubPlayer *)findChildOfType(i, kPlayerType);
	if (p == NULL)
		return;

	if ((o) && (!r)) {
		showMessageFormat("In the %s\n", (const char *)getStringPtrByID(i->itemName));
	} else if (p) {
		showMessageFormat("Carried by %s\n", (const char *)getStringPtrByID(i->itemName));
	}

	if (r) {
		showMessageFormat("%s", (const char *)getStringPtrByID(r->roomLong));
	}

	showMessageFormat("\n");

	Item *l = derefItem(i->child);
	if (l) {
		lobjFunc(l, "You can see ");	/* Show objects */
	}
}

void AGOSEngine_Elvira1::oe1_doClass() {
	// 106: do class
	Item *i = getNextItemPtr();
	int16 cm = getVarOrWord();
	int16 num = getVarOrWord();

	_classMask = (cm != -1) ? 1 << cm : 0;
	_classLine = (SubroutineLine *)((byte *)_currentTable + _currentLine->next);
	if (num == 1) {
		_subjectItem = findInByClass(i, (1 << cm));
		if (_subjectItem)
			_classMode1 = 1;
		else
			_classMode1 = 0;
	} else {
		_objectItem = findInByClass(i, (1 << cm));
		if (_objectItem)
			_classMode2 = 1;
		else
			_classMode2 = 0;
	}
}

void AGOSEngine_Elvira1::oe1_pObj() {
	// 112: print object name
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), kObjectType);
	getVarOrWord();

	if (subObject != NULL)
		showMessageFormat("%s", (const char *)getStringPtrByID(subObject->objectName));
}

void AGOSEngine_Elvira1::oe1_pName() {
	// 114: print item name
	Item *i = getNextItemPtr();
	showMessageFormat("%s", (const char *)getStringPtrByID(i->itemName));
}

void AGOSEngine_Elvira1::oe1_pcName() {
	// 115: print item case (and change first letter to upper case)
	Item *i = getNextItemPtr();
	showMessageFormat("%s", (const byte *)getStringPtrByID(i->itemName, true));
}

void AGOSEngine_Elvira1::oe1_isCalled() {
	// 135: childstruct fr2 is
	Item *item = getNextItemPtr();
	uint stringId = getNextStringID();
	setScriptCondition(!scumm_stricmp((const char *)getStringPtrByID(item->itemName), (const char *)getStringPtrByID(stringId)));
}

void AGOSEngine_Elvira1::oe1_cFlag() {
	// 162: check container flag
	SubContainer *c = (SubContainer *)findChildOfType(getNextItemPtr(), kContainerType);
	uint bit = getVarOrWord();

	if (c == NULL)
		setScriptCondition(false);
	else
		setScriptCondition((c->flags & (1 << bit)) != 0);
}

void AGOSEngine_Elvira1::oe1_rescan() {
	// 164: restart subroutine
	setScriptReturn(-10);
}

void AGOSEngine_Elvira1::oe1_setUserItem() {
	// 176: set user item
	Item *i = getNextItemPtr();
	uint tmp = getVarOrWord();
	setUserItem(i, tmp, getNextItemID());
}

void AGOSEngine_Elvira1::oe1_getUserItem() {
	// 177: get user item
	Item *i = getNextItemPtr();
	int n = getVarOrWord();

	if (getVarOrWord() == 1)
		_subjectItem = derefItem(getUserItem(i, n));
	else
		_objectItem = derefItem(getUserItem(i, n));
}

void AGOSEngine_Elvira1::oe1_whereTo() {
	// 180: where to
	Item *i = getNextItemPtr();
	int16 d = getVarOrWord();
	int16 f = getVarOrWord();

	if (f == 1)
		_subjectItem = getExitOf_e1(i, d);
	else
		_objectItem = getExitOf_e1(i, d);
}

void AGOSEngine_Elvira1::oe1_doorExit() {
	// 181: door exit
	Item *x;
	Item *a = (Item *)-1;
	SubChain *c;
	Item *i = getNextItemPtr();
	Item *d = getNextItemPtr();
	int16 f = getVarOrWord();
	int16 ct = 0;

	c = (SubChain *)findChildOfType(d, kChainType);
	if (c)
		a = derefItem(c->chChained);
	while (ct < 6) {
		x = getDoorOf(i, ct);
		if ((x == d) | (x == a)) {
			writeVariable(f, ct);
			return;
		}
		ct++;
	}
	writeVariable(f, 255);
}

void AGOSEngine_Elvira1::oe1_loadGame() {
	// 202: load restart state
	uint16 stringId = getNextStringID();
	loadGame((const char *)getStringPtrByID(stringId), true);
}

void AGOSEngine_Elvira1::oe1_clearUserItem() {
	// 178: clear user item
	Item *i = getNextItemPtr();
	uint tmp = getVarOrWord();
	setUserItem(i, tmp, 0);
}

void AGOSEngine_Elvira1::oe1_findMaster() {
	// 219: find master
	int16 ad, no;
	int16 d = getVarOrByte();

	ad = (d == 1) ? _scriptAdj1 : _scriptAdj2;
	no = (d == 1) ? _scriptNoun1 : _scriptNoun2;

	d = getVarOrByte();
	if (d == 1)
		_subjectItem = findMaster(ad, no);
	else
		_objectItem = findMaster(ad, no);
}

void AGOSEngine_Elvira1::oe1_nextMaster() {
	// 220: next master
	int16 ad, no;
	Item *item = getNextItemPtr();
	int16 d = getVarOrByte();

	ad = (d == 1) ? _scriptAdj1 : _scriptAdj2;
	no = (d == 1) ? _scriptNoun1 : _scriptNoun2;

	d = getVarOrByte();
	if (d == 1)
		_subjectItem = nextMaster(item, ad, no);
	else
		_objectItem = nextMaster(item, ad, no);
}

void AGOSEngine_Elvira1::oe1_animate() {
	// 226: animate
	uint16 vgaSpriteId = getVarOrWord();
	uint16 windowNum = getVarOrByte();
	int16 x = getVarOrWord();
	int16 y = getVarOrWord();
	uint16 palette = getVarOrWord();

	_videoLockOut |= 0x40;
	animate(windowNum, vgaSpriteId / 100, vgaSpriteId, x, y, palette);
	_videoLockOut &= ~0x40;
}

void AGOSEngine_Elvira1::oe1_stopAnimate() {
	// 227: stop animate
	stopAnimate(getVarOrWord());
}

void AGOSEngine_Elvira1::oe1_menu() {
	// 233: agos menu
	uint b = getVarOrWord();
	uint a = getVarOrWord();
	drawMenuStrip(a, b);
}

void AGOSEngine_Elvira1::oe1_addBox() {
	// 235: add item box
	uint flags = 0;
	uint id = getVarOrWord();
	uint params = id / 1000;
	uint x, y, w, h, verb;
	Item *item;

	id = id % 1000;

	if (params & 1)
		flags |= kBFInvertTouch;
	if (params & 2)
		flags |= kBFInvertSelect;
	if (params & 4)
		flags |= kBFBoxItem;
	if (params & 8)
		flags |= kBFToggleBox;
	if (params & 16)
		flags |= kBFDragBox;

	x = getVarOrWord();
	y = getVarOrWord();
	w = getVarOrWord();
	h = getVarOrWord();
	item = getNextItemPtrStrange();
	verb = getVarOrWord();
	if (x >= 1000) {
		verb += 0x4000;
		x -= 1000;
	}
	defineBox(id, x, y, w, h, flags, verb, item);
}

void AGOSEngine_Elvira1::oe1_bitClear() {
	// 251: set bit off
	int var = getVarOrWord();
	int bit = getVarOrWord();

	writeVariable(var, _variableArray[var] & ~(1 << bit));
}

void AGOSEngine_Elvira1::oe1_bitSet() {
	// 252: set bit on
	int var = getVarOrWord();
	int bit = getVarOrWord();

	writeVariable(var, _variableArray[var] | (1 << bit));
}

void AGOSEngine_Elvira1::oe1_bitTest() {
	// 253: bit test
	int var = getVarOrWord();
	int bit = getVarOrWord();

	setScriptCondition((_variableArray[var] & (1 << bit)) != 0);
}

void AGOSEngine_Elvira1::oe1_enableInput() {
	// 258: enable input
	_variableArray[500] = 0;

	for (int i = 120; i != 130; i++)
		disableBox(i);

	_verbHitArea = 0;
	_hitAreaSubjectItem = 0;
	_hitAreaObjectItem = 0;

	_dragFlag = false;
	_dragAccept = false;
	_dragCount = 0;
	_dragMode = false;

	_lastHitArea3 = 0;
	_lastHitArea = 0;

	_clickOnly = true;
}

void AGOSEngine_Elvira1::oe1_setTime() {
	// 259: set time
	_timeStore = getTime();
}

void AGOSEngine_Elvira1::oe1_ifTime() {
	// 260: if time
	uint a = getVarOrWord();
	uint32 t = getTime() - a;
	if (t >= _timeStore)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void AGOSEngine_Elvira1::oe1_playTune() {
	// 264: play tune
	uint16 music = getVarOrWord();
	uint16 track = getVarOrWord();

	if (music != _lastMusicPlayed) {
		_lastMusicPlayed = music;
		// No tune under water
		if (music == 4) {
			stopMusic();
		} else {
			playMusic(music, track);
		}
	}
}

void AGOSEngine_Elvira1::oe1_zoneDisk() {
	// 267: set disk number of each zone
	getVarOrWord();
	getVarOrWord();
}

void AGOSEngine_Elvira1::oe1_printStats() {
	// 270: print stats
	printStats();
}

void AGOSEngine_Elvira1::oe1_stopTune() {
	// 271: stop tune
}

void AGOSEngine_Elvira1::oe1_printPlayerDamage() {
	// 272: print player damage
	WindowBlock *window = _dummyWindow;
	window->flags = 1;

	mouseOff();
	writeChar(window, 36, 38, 2, _variableArray[241]);
	mouseOn();
}

void AGOSEngine_Elvira1::oe1_printMonsterDamage() {
	// 273: print monster damage
	WindowBlock *window = _dummyWindow;
	window->flags = 1;

	mouseOff();
	writeChar(window, 36, 88, 2, _variableArray[242]);
	mouseOn();
}

void AGOSEngine_Elvira1::oe1_pauseGame() {
	// 274: pause game
	WindowBlock *window = _windowArray[4];
	const char *message1, *message2;

	uint32 pauseTime = getTime();
	haltAnimation();

restart:
	printScroll();
	window->textColumn = 0;
	window->textRow = 0;
	window->textColumnOffset = 0;
	window->textLength = 0;		// Difference

	switch (_language) {
	case Common::FR_FRA:
		message1 = "    Jeu interrompu.\r\r\r";
		message2 = " Reprendre    Quitter";
		break;
	case Common::DE_DEU:
		message1 = "         Pause.\r\r\r";
		message2 = "   Weiter      Ende";
		break;
	case Common::ES_ESP:
		message1 = "   Juego en Pausa\r\r\r";
		message2 = "Continuar      Salir";
		break;
	default:
		message1 = "     Game Paused\r\r\r";
		message2 = " Continue      Quit";
		break;
	}

	for (; *message1; message1++)
		windowPutChar(window, *message1);
	for (; *message2; message2++)
		windowPutChar(window, *message2);

	if (continueOrQuit() == 0x7FFE) {
		printScroll();
		window->textColumn = 0;
		window->textRow = 0;
		window->textColumnOffset = 0;
		window->textLength = 0;		// Difference

		switch (_language) {
		case Common::FR_FRA:
			message1 = "    Etes-vous s<r ?\r\r\r";
			message2 = "     Oui      Non";
			break;
		case Common::DE_DEU:
			message1 = "    Bist Du sicher ?\r\r\r";
			message2 = "     Ja        Nein";
			break;
		case Common::ES_ESP:
			message1 = "    Estas seguro ?\r\r\r";
			message2 = "    Si          No";
			break;
		default:
			message1 = "    Are you sure ?\r\r\r";
			message2 = "     Yes       No";
			break;
		}

		for (; *message1; message1++)
			windowPutChar(window, *message1);
		for (; *message2; message2++)
			windowPutChar(window, *message2);

		if (confirmYesOrNo(120, 62) == 0x7FFF) {
			quitGame();
			// Make sure the quit event is processed immediately.
			delay(0);
		} else {
			goto restart;
		}
	}

	restartAnimation();
	_gameStoppedClock = getTime() - pauseTime + _gameStoppedClock;
}

void AGOSEngine_Elvira1::oe1_printPlayerHit() {
	// 277: print player hit
	WindowBlock *window = _dummyWindow;
	window->flags = 1;

	mouseOff();
	writeChar(window, 3, 166, 0, _variableArray[414]);
	mouseOn();
}

void AGOSEngine_Elvira1::oe1_printMonsterHit() {
	// 278: print monster hit
	WindowBlock *window = _dummyWindow;
	window->flags = 1;

	mouseOff();
	writeChar(window, 35, 166, 4, _variableArray[415]);
	mouseOn();
}

int16 AGOSEngine::moreText(Item *i) {
	SubObject *o;
	i = derefItem(i->next);

	while (i) {
		o = (SubObject *)findChildOfType(i, kObjectType);
		if ((o) && (o->objectFlags & 1))
			goto l1;
		if (i != me())
			return 1;
	l1:	i = derefItem(i->next);
	}

	return 0;
}

void AGOSEngine::lobjFunc(Item *i, const char *f) {
	int n = 0;
	SubObject *o;

	while (i) {
		o = (SubObject *)findChildOfType(i, kObjectType);
		if ((o) && (o->objectFlags & 1))
			goto l1;
		if (i == me())
			goto l1;
		if (n == 0) {
			if (f)
				showMessageFormat("%s", f);
			n = 1;
		} else {
			if (moreText(i))
				showMessageFormat(", ");
			else
				showMessageFormat(" and ");
		}
		showMessageFormat("%s", (const char *)getStringPtrByID(i->itemName));
l1:		i = derefItem(i->next);
	}
	if (f) {
		if (n == 1)
			showMessageFormat(".\n");
	} else {
		if (n == 0)
			showMessageFormat("nothing");
	}
}

uint AGOSEngine::confirmYesOrNo(uint16 x, uint16 y) {
	HitArea *ha;

	ha = findEmptyHitArea();
	ha->x = x;
	ha->y = y;
	ha->width = 30;
	ha->height = 12;
	ha->flags = kBFBoxInUse;
	ha->id = 0x7FFF;
	ha->priority = 999;
	ha->window = 0;

	ha = findEmptyHitArea();
	ha->x = x + 60;
	ha->y = y;
	ha->width = 24;
	ha->height = 12;
	ha->flags = kBFBoxInUse;
	ha->id = 0x7FFE;
	ha->priority = 999;
	ha->window = 0;

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
		} else if (ha->id == 0x7FFE) {
			break;
		} else if (ha->id == 0x7FFF) {
			break;
		}
	}

	undefineBox(0x7FFF);
	undefineBox(0x7FFE);

	return ha->id;
}

uint AGOSEngine::continueOrQuit() {
	HitArea *ha;

	ha = findEmptyHitArea();
	ha->x = 96;
	ha->y = 62;
	ha->width = 60;
	ha->height = 12;
	ha->flags = kBFBoxInUse;
	ha->id = 0x7FFF;
	ha->priority = 999;
	ha->window = 0;

	ha = findEmptyHitArea();
	ha->x = 180;
	ha->y = 62;
	ha->width = 36;
	ha->height = 12;
	ha->flags = kBFBoxInUse;
	ha->id = 0x7FFE;
	ha->priority = 999;
	ha->window = 0;

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
		} else if (ha->id == 0x7FFE) {
			break;
		} else if (ha->id == 0x7FFF) {
			break;
		}
	}

	undefineBox(0x7FFF);
	undefineBox(0x7FFE);

	return ha->id;
}

void AGOSEngine::printScroll() {
	VgaPointersEntry *vpe = &_vgaBufferPointers[1];
	byte *curVgaFile2Orig = _curVgaFile2;

	_windowNum = 3;
	_curVgaFile2 = vpe->vgaFile2;
	drawImage_init(9, 0, 10, 32, 0);

	_curVgaFile2 = curVgaFile2Orig;
}

void AGOSEngine::printStats() {
	WindowBlock *window = _dummyWindow;
	int val;

	window->flags = 1;

	mouseOff();

	// Strength
	val = _variableArray[0];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;
	writeChar(window, 5, 133, 6, val);

	// Resolution
	val = _variableArray[1];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;
	writeChar(window, 11, 133, 6, val);

	// Dexterity
	val = _variableArray[2];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;
	writeChar(window, 18, 133, 0, val);

	// Skill
	val = _variableArray[3];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;
	writeChar(window, 24, 133, 0, val);

	// Life
	val = _variableArray[5];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;
	writeChar(window, 30, 133, 2, val);

	// Experience
	val = _variableArray[6];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;
	writeChar(window, 36, 133, 4, val);

	mouseOn();
}

} // End of namespace AGOS
