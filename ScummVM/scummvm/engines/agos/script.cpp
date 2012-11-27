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

// Item script opcodes for Simon1/Simon2

#include "common/endian.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "agos/animation.h"
#include "agos/agos.h"

#ifdef _WIN32_WCE
extern bool isSmartphone();
#endif

namespace AGOS {

void AGOSEngine::setupOpcodes() {
	error("setupOpcodes: Unknown game");
}

void AGOSEngine::setScriptCondition(bool cond) {
	_runScriptCondition[_recursionDepth] = cond;
}

bool AGOSEngine::getScriptCondition() {
	return _runScriptCondition[_recursionDepth];
}

void AGOSEngine::setScriptReturn(int ret) {
	_runScriptReturn[_recursionDepth] = ret;
}

int AGOSEngine::getScriptReturn() {
	return _runScriptReturn[_recursionDepth];
}

// -----------------------------------------------------------------------
// Common Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::o_invalid() {
	error("Invalid opcode %d", _opcode);
}

void AGOSEngine::o_at() {
	// 1: ptrA parent is
	setScriptCondition(me()->parent == getNextItemID());
}

void AGOSEngine::o_notAt() {
	// 2: ptrA parent is not
	setScriptCondition(me()->parent != getNextItemID());
}

void AGOSEngine::o_carried() {
	// 5: parent is 1
	setScriptCondition(getNextItemPtr()->parent == getItem1ID());
}

void AGOSEngine::o_notCarried() {
	// 6: parent isnot 1
	setScriptCondition(getNextItemPtr()->parent != getItem1ID());
}

void AGOSEngine::o_isAt() {
	// 7: parent is
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent == getNextItemID());
}

void AGOSEngine::o_zero() {
	// 11: is zero
	setScriptCondition(getNextVarContents() == 0);
}

void AGOSEngine::o_notZero() {
	// 12: isnot zero
	setScriptCondition(getNextVarContents() != 0);
}

void AGOSEngine::o_eq() {
	// 13: equal
	uint tmp = getNextVarContents();
	uint tmp2 = getVarOrWord();

#ifdef __DS__
	// HACK: Skip attempt to read Calypso's letter manually,
	// due to speech segment been too large to fit into memory
	if (getGameType() == GType_SIMON1 && (getFeatures() & GF_TALKIE) &&
		getPlatform() == Common::kPlatformWindows && _currentTable) {
		if (_currentTable->id == 71 && tmp == 1 && tmp2 == 1) {
			setScriptCondition(false);
			return;
		}
	}
#endif
	setScriptCondition(tmp == tmp2);
}

void AGOSEngine::o_notEq() {
	// 14: not equal
	uint tmp = getNextVarContents();
	setScriptCondition(tmp != getVarOrWord());
}

void AGOSEngine::o_gt() {
	// 15: is greater
	int16 tmp1 = getNextVarContents();
	int16 tmp2 = getVarOrWord();
	setScriptCondition(tmp1 > tmp2);
}

void AGOSEngine::o_lt() {
	// 16: is less
	int16 tmp1 = getNextVarContents();
	int16 tmp2 = getVarOrWord();
	setScriptCondition(tmp1 < tmp2);
}

void AGOSEngine::o_eqf() {
	// 17: is eq f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp == getNextVarContents());
}

void AGOSEngine::o_notEqf() {
	// 18: is not equal f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp != getNextVarContents());
}

void AGOSEngine::o_ltf() {
	// 19: is greater f
	int16 tmp1 = getNextVarContents();
	int16 tmp2 = getNextVarContents();
	setScriptCondition(tmp1 < tmp2);
}

void AGOSEngine::o_gtf() {
	// 20: is less f
	int16 tmp1 = getNextVarContents();
	int16 tmp2 = getNextVarContents();
	setScriptCondition(tmp1 > tmp2);
}

void AGOSEngine::o_chance() {
	// 23: chance
	int16 a = getVarOrWord();

	if (a == 0) {
		setScriptCondition(false);
		return;
	}

	if (a == 100) {
		setScriptCondition(true);
		return;
	}

	a += _chanceModifier;

	if (a <= 0) {
		_chanceModifier = 0;
		setScriptCondition(false);
	} else if ((int16)_rnd.getRandomNumber(99) < a) {
		if (_chanceModifier <= 0)
			_chanceModifier -= 5;
		else
			_chanceModifier = 0;
		setScriptCondition(true);
	} else {
		if (_chanceModifier >= 0)
			_chanceModifier += 5;
		else
			_chanceModifier = 0;
		setScriptCondition(false);
	}
}

void AGOSEngine::o_isRoom() {
	// 25: is room
	setScriptCondition(isRoom(getNextItemPtr()));
}

void AGOSEngine::o_isObject() {
	// 26: is object
	setScriptCondition(isObject(getNextItemPtr()));
}

void AGOSEngine::o_state() {
	// 27: item state is
	Item *item = getNextItemPtr();
	setScriptCondition((uint) item->state == getVarOrWord());
}

void AGOSEngine::o_oflag() {
	// 28: item has prop
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), kObjectType);
	uint num = getVarOrByte();
	setScriptCondition(subObject != NULL && (subObject->objectFlags & (1 << num)) != 0);
}

void AGOSEngine::o_destroy() {
	// 31: set no parent
	setItemParent(getNextItemPtr(), NULL);
}

void AGOSEngine::o_place() {
	// 33: set item parent
	Item *item = getNextItemPtr();
	setItemParent(item, getNextItemPtr());
}

void AGOSEngine::o_copyff() {
	// 36: copy var
	uint value = getNextVarContents();
	writeNextVarContents(value);
}

void AGOSEngine::o_clear() {
	// 41: zero var
	writeNextVarContents(0);
}

void AGOSEngine::o_let() {
	// 42: set var
	uint var = getVarWrapper();
	uint value = getVarOrWord();

	if (getGameType() == GType_FF && _currentTable) {
		// WORKAROUND: When the repair man comes to fix the car, the game doesn't
		// wait long enough for the screen to completely scroll to the left side.
		if (_currentTable->id == 20438 && var == 103 && value == 60) {
			value = 71;
		}
	}

	writeVariable(var, value);
}

void AGOSEngine::o_add() {
	// 43: add
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) + getVarOrWord());

	// WORKAROUND: The converation of the male in Vid-Phone Booth at Dave's Space Bar
	// is based on variable 116, but stops due to a missing option (37).
	if (getGameType() == GType_FF && _currentTable->id == 10538 && readVariable(116) == 37)
			writeVariable(116, 38);
}

void AGOSEngine::o_sub() {
	// 44: sub
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) - getVarOrWord());
}

void AGOSEngine::o_addf() {
	// 45: add f
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) + getNextVarContents());
}

void AGOSEngine::o_subf() {
	// 46: sub f
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) - getNextVarContents());
}

void AGOSEngine::o_mul() {
	// 47: mul
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) * getVarOrWord());
}

void AGOSEngine::o_div() {
	// 48: div
	uint var = getVarWrapper();
	int value = getVarOrWord();
	if (value == 0)
		error("o_div: Division by zero");
	writeVariable(var, readVariable(var) / value);
}

void AGOSEngine::o_mulf() {
	// 49: mul f
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) * getNextVarContents());
}

void AGOSEngine::o_divf() {
	// 50: div f
	uint var = getVarWrapper();
	int value = getNextVarContents();
	if (value == 0)
		error("o_divf: Division by zero");
	writeVariable(var, readVariable(var) / value);
}

void AGOSEngine::o_mod() {
	// 51: mod
	uint var = getVarWrapper();
	int value = getVarOrWord();
	if (value == 0)
		error("o_mod: Division by zero");
	writeVariable(var, readVariable(var) % value);
}

void AGOSEngine::o_modf() {
	// 52: mod f
	uint var = getVarWrapper();
	int value = getNextVarContents();
	if (value == 0)
		error("o_modf: Division by zero");
	writeVariable(var, readVariable(var) % value);
}

void AGOSEngine::o_random() {
	// 53: random
	uint var = getVarWrapper();
	uint value = (uint16)getVarOrWord();
	writeVariable(var, _rnd.getRandomNumber(value - 1));
}

void AGOSEngine::o_goto() {
	// 55: set itemA parent
	uint item = getNextItemID();
	setItemParent(me(), _itemArrayPtr[item]);
}

void AGOSEngine::o_oset() {
	// 56: set child2 fr bit
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), kObjectType);
	int value = getVarOrByte();
	if (subObject != NULL && value >= 16)
		subObject->objectFlags |= (1 << value);
}

void AGOSEngine::o_oclear() {
	// 57: clear child2 fr bit
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), kObjectType);
	int value = getVarOrByte();
	if (subObject != NULL && value >= 16)
		subObject->objectFlags &= ~(1 << value);
}

void AGOSEngine::o_putBy() {
	// 58: make siblings
	Item *item = getNextItemPtr();
	setItemParent(item, derefItem(getNextItemPtr()->parent));
}

void AGOSEngine::o_inc() {
	// 59: item inc state
	Item *item = getNextItemPtr();
	if (item->state <= 30000) {
		setItemState(item, item->state + 1);
		synchChain(item);
	}
}

void AGOSEngine::o_dec() {
	// 60: item dec state
	Item *item = getNextItemPtr();
	if (item->state >= 0) {
		setItemState(item, item->state - 1);
		synchChain(item);
	}
}

void AGOSEngine::o_setState() {
	// 61: item set state
	Item *item = getNextItemPtr();
	int value = getVarOrWord();
	if (value < 0)
		value = 0;
	if (value > 30000)
		value = 30000;
	setItemState(item, value);
	synchChain(item);
}

void AGOSEngine::o_print() {
	// 62: show int
	showMessageFormat("%d", getNextVarContents());
}

void AGOSEngine::o_message() {
	// 63: show string nl
	showMessageFormat("%s\n", getStringPtrByID(getNextStringID()));
}

void AGOSEngine::o_msg() {
	// 64: show string
	showMessageFormat("%s", getStringPtrByID(getNextStringID()));
}

void AGOSEngine::o_end() {
	// 68: exit interpreter
	quitGame();
	// Make sure the quit event is processed immediately.
	delay(0);
}

void AGOSEngine::o_done() {
	// 69: return 1
	setScriptReturn(1);
}

void AGOSEngine::o_process() {
	// 71: start subroutine
	uint16 id = getVarOrWord();

	if (!_copyProtection && getGameType() == GType_WW && id == 71) {
		// Copy protection was disabled in Good Old Games release
		return;
	}

	Subroutine *sub = getSubroutineByID(id);
	if (sub != NULL) {
#ifdef __DS__
		// HACK: Skip scene of Simon reading letter from Calypso
		// due to speech segment been too large to fit into memory
		if (getGameType() == GType_SIMON1 && (getFeatures() & GF_TALKIE) &&
			getPlatform() == Common::kPlatformWindows && sub->id == 2922) {
			// set parent special
			_noParentNotify = true;
			setItemParent(derefItem(16), me());
			_noParentNotify = false;

			// set parent special
			_noParentNotify = true;
			setItemParent(derefItem(14), me());
			_noParentNotify = false;

			// set item parent
			setItemParent(derefItem(12), me());

			return;
		}
#endif
		startSubroutine(sub);
	}
}

void AGOSEngine::o_when() {
	// 76: add timeout
	uint16 timeout = getVarOrWord();
	addTimeEvent(timeout, getVarOrWord());
}

void AGOSEngine::o_if1() {
	// 77: has item minus 1
	setScriptCondition(_subjectItem != NULL);
}

void AGOSEngine::o_if2() {
	// 78: has item minus 3
	setScriptCondition(_objectItem != NULL);
}

void AGOSEngine::o_isCalled() {
	// 79: childstruct fr2 is
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), kObjectType);
	uint stringId = getNextStringID();
	setScriptCondition((subObject != NULL) && subObject->objectName == stringId);
}

void AGOSEngine::o_is() {
	// 80: item equal
	setScriptCondition(getNextItemPtr() == getNextItemPtr());
}

void AGOSEngine::o_debug() {
	// 82: debug opcode
	getVarOrByte();
}

void AGOSEngine::o_comment() {
	// 87: comment
	getNextStringID();
}

void AGOSEngine::o_haltAnimation() {
	// 88: stop animation
	_videoLockOut |= 0x10;

	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		VgaTimerEntry *vte = _vgaTimerList;
		while (vte->delay) {
			if (vte->type == ANIMATE_EVENT)
				vte->delay += 10;
			vte++;
		}

		_scrollCount = 0;
		_scrollFlag = 0;
	}
}

void AGOSEngine::o_restartAnimation() {
	// 89: restart animation
	_videoLockOut &= ~0x10;
}

void AGOSEngine::o_getParent() {
	// 90: set minusitem to parent
	Item *i = getNextItemPtr();
	if (getVarOrByte() == 1)
		_subjectItem = derefItem(i->parent);
	else
		_objectItem = derefItem(i->parent);
}

void AGOSEngine::o_getNext() {
	// 91: set minusitem to next
	Item *i = getNextItemPtr();
	if (getVarOrByte() == 1)
		_subjectItem = derefItem(i->next);
	else
		_objectItem = derefItem(i->next);
}

void AGOSEngine::o_getChildren() {
	// 92: set minusitem to child
	Item *i = getNextItemPtr();
	if (getVarOrByte() == 1)
		_subjectItem = derefItem(i->child);
	else
		_objectItem = derefItem(i->child);
}

void AGOSEngine::o_picture() {
	// 96
	uint vga_res = getVarOrWord();
	uint mode = getVarOrByte();

	// WORKAROUND: For a script bug in the Amiga AGA/CD32 versions
	// When selecting locations on the magical map, the script looks
	// for vga_res 12701, but only vga_res 12700 exists.
	if (getGameType() == GType_SIMON1 && getPlatform() == Common::kPlatformAmiga &&
		vga_res == 12701) {
		return;
	}

	if (getGameType() == GType_PP && getGameId() != GID_DIMP) {
		if (vga_res == 8700 && getBitFlag(107)) {
			_vgaPeriod = 30;
		}

		_picture8600 = (vga_res == 8600);
	}

	setWindowImageEx(mode, vga_res);
}

void AGOSEngine::o_loadZone() {
	// 97: load zone
	uint vga_res = getVarOrWord();

	_videoLockOut |= 0x80;

	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 ||
		getGameType() == GType_WW) {
		vc27_resetSprite();
		vc29_stopAllSounds();
	}

	loadZone(vga_res);

	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 ||
		getGameType() == GType_WW) {
		_copyScnFlag = 0;
		_vgaSpriteChanged = 0;
	}

	_videoLockOut &= ~0x80;
}

void AGOSEngine::o_killAnimate() {
	// 100: kill animations
	_videoLockOut |= 0x8000;
	vc27_resetSprite();
	_videoLockOut &= ~0x8000;
}

void AGOSEngine::o_defWindow() {
	// 101: define window
	uint num = getVarOrByte();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint w = getVarOrWord();
	uint h = getVarOrWord();
	uint flags = getVarOrWord();
	uint color = getVarOrWord();

	uint fillColor, textColor;
	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 ||
		getGameType() == GType_WW) {
		fillColor = color % 100;
		textColor = color / 100;
	} else {
		fillColor = color;
		textColor = 0;
	}

	num &= 7;

	if (_windowArray[num])
		closeWindow(num);

	_windowArray[num] = openWindow(x, y, w, h, flags, fillColor, textColor);

	if (num == _curWindow) {
		_textWindow = _windowArray[num];
		justifyStart();
	}
}

void AGOSEngine::o_window() {
	// 102
	changeWindow(getVarOrByte() & 7);
}

void AGOSEngine::o_cls() {
	// 103
	mouseOff();
	removeIconArray(_curWindow);
	showMessageFormat("\x0C");
	_oracleMaxScrollY = 0;
	_noOracleScroll = 0;
	mouseOn();
}

void AGOSEngine::o_closeWindow() {
	// 104
	closeWindow(getVarOrByte() & 7);
}

void AGOSEngine::o_addBox() {
	// 107: add item box
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

void AGOSEngine::o_delBox() {
	// 108: delete box
	undefineBox(getVarOrWord());
}

void AGOSEngine::o_enableBox() {
	// 109: enable box
	enableBox(getVarOrWord());
}

void AGOSEngine::o_disableBox() {
	// 110: set hitarea bit 0x40
	disableBox(getVarOrWord());
}

void AGOSEngine::o_moveBox() {
	// 111: set hitarea xy
	uint hitarea_id = getVarOrWord();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	moveBox(hitarea_id, x, y);
}

void AGOSEngine::o_doIcons() {
	// 114
	Item *item = getNextItemPtr();
	uint num = getVarOrByte();
	mouseOff();
	drawIconArray(num, item, 0, 0);
	mouseOn();
}

void AGOSEngine::o_isClass() {
	// 115: item has flag
	Item *item = getNextItemPtr();
	setScriptCondition((item->classFlags & (1 << getVarOrByte())) != 0);
}

void AGOSEngine::o_setClass() {
	// 116: item set flag
	Item *item = getNextItemPtr();
	item->classFlags |= (1 << getVarOrByte());
}

void AGOSEngine::o_unsetClass() {
	// 117: item clear flag
	Item *item = getNextItemPtr();
	item->classFlags &= ~(1 << getVarOrByte());
}

void AGOSEngine::o_waitSync() {
	// 119: wait vga
	uint var = getVarOrWord();
	_scriptVar2 = (var == 200);

	if (var != 200 || !_skipVgaWait)
		waitForSync(var);
	_skipVgaWait = false;
}

void AGOSEngine::o_sync() {
	// 120: sync
	sendSync(getVarOrWord());
}

void AGOSEngine::o_defObj() {
	// 121: set vga item
	uint slot = getVarOrByte();
	_objectArray[slot] = getNextItemPtr();
}

void AGOSEngine::o_here() {
	// 125: item is sibling with item 1
	Item *item = getNextItemPtr();
	setScriptCondition(me()->parent == item->parent);
}

void AGOSEngine::o_doClassIcons() {
	// 126: do class icons
	Item *item = getNextItemPtr();
	uint num = getVarOrByte();
	uint a = getVarOrByte();

	mouseOff();
	if (getGameType() == GType_ELVIRA1)
		drawIconArray(num, item, 0, a);
	else
		drawIconArray(num, item, 0, 1 << a);
	mouseOn();
}

void AGOSEngine::o_playTune() {
	// 127: play tune
	uint16 music = getVarOrWord();
	uint16 track = getVarOrWord();

	if (music != _lastMusicPlayed) {
		_lastMusicPlayed = music;
		playMusic(music, track);
	}
}

void AGOSEngine::o_setAdjNoun() {
	// 130: set adj noun
	uint var = getVarOrByte();
	if (var == 1) {
		_scriptAdj1 = getNextWord();
		_scriptNoun1 = getNextWord();
	} else {
		_scriptAdj2 = getNextWord();
		_scriptNoun2 = getNextWord();
	}
}

void AGOSEngine::o_saveUserGame() {
	// 132: save user game
	if (getGameId() == GID_SIMON1CD32) {
		// The Amiga CD32 version of Simon the Sorcerer 1 uses a single slot
		if (!saveGame(0, "Default Saved Game")) {
			vc33_setMouseOn();
			fileError(_windowArray[5], true);
		}
	} else {
		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
		userGame(false);
		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	}
}

void AGOSEngine::o_loadUserGame() {
	// 133: load user game
	if (getGameId() == GID_SIMON1CD32) {
		// The Amiga CD32 version of Simon the Sorcerer 1 uses a single slot
		if (!loadGame(genSaveName(0))) {
			vc33_setMouseOn();
			fileError(_windowArray[5], false);
		}
	} else {
		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
		userGame(true);
		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	}
}

void AGOSEngine::o_copysf() {
	// 136: set var to item unk3
	Item *item = getNextItemPtr();
	writeNextVarContents(item->state);
}

void AGOSEngine::o_restoreIcons() {
	// 137
	uint num = getVarOrByte();
	WindowBlock *window = _windowArray[num & 7];
	if (window->iconPtr)
		drawIconArray(num, window->iconPtr->itemRef, window->iconPtr->line, window->iconPtr->classMask);
}

void AGOSEngine::o_freezeZones() {
	// 138: freeze zones
	freezeBottom();

	if (!_copyProtection && !(getFeatures() & GF_TALKIE) && _currentTable) {
		if ((getGameType() == GType_SIMON1 && _currentTable->id == 2924) ||
			(getGameType() == GType_SIMON2 && _currentTable->id == 1322)) {
			_variableArray[134] = 3;
			_variableArray[135] = 3;
			setBitFlag(135, 1);
			setScriptCondition(0);
		}
	}
}

void AGOSEngine::o_placeNoIcons() {
	// 139: set parent special
	Item *item = getNextItemPtr();
	_noParentNotify = true;
	setItemParent(item, getNextItemPtr());
	_noParentNotify = false;
}

void AGOSEngine::o_clearTimers() {
	// 140: clear timers
	killAllTimers();

	if (getGameType() == GType_SIMON1)
		addTimeEvent(3, 160);
}

void AGOSEngine::o_setDollar() {
	// 141: set m1 to m3
	uint which = getVarOrByte();
	Item *item = getNextItemPtr();
	if (which == 1) {
		_subjectItem = item;
	} else {
		_objectItem = item;
	}
}

void AGOSEngine::o_isBox() {
	// 142: is box dead
	setScriptCondition(isBoxDead(getVarOrWord()));
}

// -----------------------------------------------------------------------

byte AGOSEngine::getByte() {
	return *_codePtr++;
}

int AGOSEngine::getNextWord() {
	int16 a = (int16)READ_BE_UINT16(_codePtr);
	_codePtr += 2;
	return a;
}

uint AGOSEngine::getNextStringID() {
	return (uint16)getNextWord();
}

uint AGOSEngine::getVarOrByte() {
	if (getGameType() == GType_ELVIRA1) {
		return getVarOrWord();
	} else {
		uint a = *_codePtr++;
		if (a != 255)
			return a;
		return readVariable(*_codePtr++);
	}
}

uint AGOSEngine::getVarOrWord() {
	uint a = READ_BE_UINT16(_codePtr);
	_codePtr += 2;
	if (getGameType() == GType_PP) {
		if (a >= 60000 && a < 62048) {
			return readVariable(a - 60000);
		}
	} else {
		if (a >= 30000 && a < 30512) {
			return readVariable(a - 30000);
		}
	}
	return a;
}

uint AGOSEngine::getVarWrapper() {
	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_PP)
		return getVarOrWord();
	else
		return getVarOrByte();
}

uint AGOSEngine::getNextVarContents() {
	return (uint16)readVariable(getVarWrapper());
}

uint AGOSEngine::readVariable(uint16 variable) {
	if (variable >= _numVars)
		error("readVariable: Variable %d out of range", variable);

	if (getGameType() == GType_PP) {
		return (uint16)_variableArray[variable];
	} else if (getGameType() == GType_FF) {
		if (getBitFlag(83))
			return (uint16)_variableArray2[variable];
		else
			return (uint16)_variableArray[variable];
	} else {
			return _variableArray[variable];
	}
}

void AGOSEngine::writeNextVarContents(uint16 contents) {
	writeVariable(getVarWrapper(), contents);
}

void AGOSEngine::writeVariable(uint16 variable, uint16 contents) {
	if (variable >= _numVars)
		error("writeVariable: Variable %d out of range", variable);

	if (getGameType() == GType_FF && getBitFlag(83))
		_variableArray2[variable] = contents;
	else
		_variableArray[variable] = contents;
}

int AGOSEngine::runScript() {
	bool flag;

	if (shouldQuit())
		return 1;

	do {
		if (_dumpOpcodes)
			dumpOpcode(_codePtr);

		if (getGameType() == GType_ELVIRA1) {
			_opcode = getVarOrWord();
			if (_opcode == 10000)
				return 0;
		} else {
			_opcode = getByte();
			if (_opcode == 0xFF)
				return 0;
		}

		if (_runScriptReturn1)
			return 1;

		/* Invert condition? */
		flag = false;
		if (getGameType() == GType_ELVIRA1) {
			if (_opcode == 203) {
				flag = true;
				_opcode = getVarOrWord();
				if (_opcode == 10000)
					return 0;
			}
		} else {
			if (_opcode == 0) {
				flag = true;
				_opcode = getByte();
				if (_opcode == 0xFF)
					return 0;
			}
		}

		setScriptCondition(true);
		setScriptReturn(0);

		if (_opcode > _numOpcodes)
			error("Invalid opcode '%d' encountered", _opcode);

		executeOpcode(_opcode);
	} while (getScriptCondition() != flag && !getScriptReturn() && !shouldQuit());

	return (shouldQuit()) ? 1 : getScriptReturn();
}

Child *nextSub(Child *sub, int16 key) {
	Child *a = sub->next;
	while (a) {
		if (a->type == key)
			return a;
		a = a->next;
	}
	return NULL;
}

void AGOSEngine::synchChain(Item *i) {
	SubChain *c = (SubChain *)findChildOfType(i, kChainType);
	while (c) {
		setItemState(derefItem(c->chChained), i->state);
		c = (SubChain *)nextSub((Child *)c, kChainType);
	}
}

void AGOSEngine::sendSync(uint a) {
	uint16 id = to16Wrapper(a);
	_videoLockOut |= 0x8000;
	_vcPtr = (byte *)&id;
	vc15_sync();
	_videoLockOut &= ~0x8000;
}

void AGOSEngine::stopAnimate(uint16 a) {
	uint16 b = to16Wrapper(a);
	_videoLockOut |= 0x8000;
	_vcPtr = (byte *)&b;
	vc60_stopAnimation();
	_videoLockOut &= ~0x8000;
}

void AGOSEngine::waitForSync(uint a) {
	const uint maxCount = (getGameType() == GType_SIMON1) ? 1000 : 2500;

	if (getGameType() == GType_SIMON1 && (getFeatures() & GF_TALKIE)) {
		if (a != 200) {
			uint16 tmp = _lastVgaWaitFor;
			_lastVgaWaitFor = 0;
			if (tmp == a)
				return;
		}
	}

	_vgaWaitFor = a;
	_syncCount = 0;
	_exitCutscene = false;
	_rightButtonDown = false;

	while (_vgaWaitFor != 0 && !shouldQuit()) {
		if (_rightButtonDown) {
			if (_vgaWaitFor == 200 && (getGameType() == GType_FF || !getBitFlag(14))) {
				skipSpeech();
				break;
			}
		}
		if (_exitCutscene) {
			if (getGameType() == GType_ELVIRA1) {
				if (_variableArray[105] == 0) {
					_variableArray[105] = 255;
					break;
				}
			} else if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
				if (_vgaWaitFor == 51) {
					setBitFlag(244, 1);
					break;
				}
			} else {
				if (getBitFlag(9)) {
					endCutscene();
					break;
				}
			}
		}
		processSpecialKeys();

		if (_syncCount >= maxCount) {
			warning("waitForSync: wait timed out");
			break;
		}

		delay(1);
	}
}

} // End of namespace AGOS
