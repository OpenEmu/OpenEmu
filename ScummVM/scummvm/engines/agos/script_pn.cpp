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

#include "common/endian.h"
#include "common/textconsole.h"

namespace AGOS {

enum {
	kJmpClassNum = -1
};

#define OPCODE(x)	_OPCODE(AGOSEngine_PN, x)

void AGOSEngine_PN::setupOpcodes() {
	static const OpcodeEntryPN opcodes[] = {
		/* 00 */
		OPCODE(opn_opcode00),
		OPCODE(opn_add),
		OPCODE(opn_sub),
		OPCODE(opn_mul),
		/* 04 */
		OPCODE(opn_div),
		OPCODE(opn_opcode05),
		OPCODE(opn_opcode06),
		OPCODE(opn_opcode07),
		/* 08 */
		OPCODE(opn_opcode08),
		OPCODE(opn_opcode09),
		OPCODE(opn_opcode10),
		OPCODE(opn_opcode11),
		/* 12 */
		OPCODE(opn_opcode12),
		OPCODE(opn_opcode13),
		OPCODE(opn_opcode14),
		OPCODE(opn_opcode15),
		/* 16 */
		OPCODE(opn_opcode16),
		OPCODE(opn_lt),
		OPCODE(opn_gt),
		OPCODE(opn_eq),
		/* 20 */
		OPCODE(opn_neq),
		OPCODE(opn_opcode21),
		OPCODE(opn_opcode22),
		OPCODE(opn_opcode23),
		/* 24 */
		OPCODE(opn_opcode24),
		OPCODE(opn_opcode25),
		OPCODE(opn_opcode26),
		OPCODE(opn_opcode27),
		/* 28 */
		OPCODE(opn_opcode28),
		OPCODE(opn_opcode29),
		OPCODE(opn_opcode30),
		OPCODE(opn_opcode31),
		/* 32 */
		OPCODE(opn_opcode32),
		OPCODE(opn_opcode33),
		OPCODE(opn_opcode34),
		OPCODE(opn_opcode35),
		/* 36 */
		OPCODE(opn_opcode36),
		OPCODE(opn_opcode37),
		OPCODE(opn_opcode38),
		OPCODE(opn_opcode39),
		/* 40 */
		OPCODE(opn_opcode40),
		OPCODE(opn_opcode41),
		OPCODE(opn_opcode42),
		OPCODE(opn_opcode43),
		/* 44 */
		OPCODE(opn_opcode44),
		OPCODE(opn_opcode45),
		OPCODE(opn_opcode46),
		OPCODE(opn_opcode47),
		/* 48 */
		OPCODE(opn_opcode48),
		OPCODE(opn_opcode49),
		OPCODE(opn_opcode50),
		OPCODE(opn_opcode51),
		/* 52 */
		OPCODE(opn_opcode52),
		OPCODE(opn_opcode53),
		OPCODE(opn_opcode54),
		OPCODE(opn_opcode55),
		/* 56 */
		OPCODE(opn_opcode56),
		OPCODE(opn_opcode57),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 60 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(opn_opcode62),
		OPCODE(opn_opcode63),
	};

	_opcodesPN = opcodes;
	_numOpcodes = 64;
}

void AGOSEngine_PN::executeOpcode(int opcode) {
	OpcodeProcPN op = _opcodesPN[opcode].proc;
	(this->*op)();
}

int AGOSEngine_PN::readfromline() {
	if (!_linct)
		error("readfromline: Internal Error - Line Over-run");
	_linct--;
	return *_workptr++;
}

// -----------------------------------------------------------------------
// Personal Nightmare Opcodes
// -----------------------------------------------------------------------

void AGOSEngine_PN::opn_opcode00() {
	uint8 *str = _workptr;
	varval();
	writeval(str, varval());
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_add() {
	uint8 *str = _workptr;
	int32 sp = varval() + varval();
	_variableArray[12] = sp % 65536;
	_variableArray[13] = sp / 65536;
	if (sp > 65535)
		sp = 65535;
	writeval(str, (int)sp);
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_sub() {
	uint8 *str = _workptr;
	int32 sp = varval();
	sp -= varval();
	_variableArray[12] = sp % 65536;
	_variableArray[13] = sp / 65536;
	if (sp < 0)
		sp = 0;
	writeval(str, (int)sp);
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_mul() {
	uint8 *str = _workptr;
	int32 sp = varval() * varval();
	_variableArray[12] = sp % 65536;
	_variableArray[13] = sp / 65536;
	if (sp > 65535)
		sp = 65535;
	writeval(str, (int)sp);
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_div() {
	uint8 *str = _workptr;
	int32 sp = varval();
	int32 sp2 = varval();
	if (sp2 == 0)
		error("opn_div: Division by 0");
	sp = sp / sp2;
	_variableArray[12] = sp % 65536;
	_variableArray[13] = sp / 65536;
	writeval(str, (int)sp);
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode05() {
	pcf((uint8)'\n');
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode06() {
	pmesd(varval());
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode07() {
	int32 sp = varval();
	plocd((int)sp, varval());
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode08() {
	int32 sp = varval();
	pobjd((int)sp, varval());
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode09() {
	pmesd(varval());
	pcf((uint8)'\n');
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode10() {
	int32 sp = varval();
	plocd((int)sp, varval());
	pcf((uint8)'\n');
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode11() {
	int32 sp = varval();
	pobjd((int)sp, varval());
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode12() {
	char bf[8];
	int a = 0;
	sprintf(bf,"%d", varval());
	while (bf[a])
		pcf(bf[a++]);
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode13() {
	char bf[8];
	int a = 0;
	sprintf(bf,"%d", varval());
	while (bf[a])
		pcf(bf[a++]);
	pcf((uint8)'\n');
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode14() {
	clearWindow(_windowArray[_curWindow]);
	pcf((uint8)255);
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode15() {
	int32 x = varval();
	if ((x < 0) || (x > 4))
		x = 0;

	pcf((unsigned char)254);
	_curWindow = x;
	_xofs = (8 * _windowArray[_curWindow]->textLength) / 6 + 1;
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode16() {
	int32 sp = varval();
	setScriptReturn((sp >= 0 && sp <= 4));
}

void AGOSEngine_PN::opn_lt() {
	int16 v1 = varval();
	int16 v2 = varval();
	setScriptReturn(v1 < v2);
}

void AGOSEngine_PN::opn_gt() {
	int16 v1 = varval();
	int16 v2 = varval();
	setScriptReturn(v1 > v2);
}

void AGOSEngine_PN::opn_eq() {
	int16 v1 = varval();
	int16 v2 = varval();
	setScriptReturn(v1 == v2);
}

void AGOSEngine_PN::opn_neq() {
	int16 v1 = varval();
	int16 v2 = varval();
	setScriptReturn(v1 != v2);
}

void AGOSEngine_PN::opn_opcode21() {
	setposition(_procnum, varval());
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode22() {
	int pf[8];
	int n = varval();
	funcentry(pf, n);
	funccpy(pf);
	setposition(n, 0);
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode23() {
	setScriptReturn(actCallD(varval()));
}

void AGOSEngine_PN::opn_opcode24() {
	popstack(kJmpClassNum);
	// Jump back to the last doline, which will return 2-1=1.
	// That value then is returned to actCallD, which once again
	// returns it. In the end, this amounts to a setScriptReturn(true)
	// (but possibly in a different level than the current one).
	_dolineReturnVal = 2;
	_tagOfActiveDoline = _stackbase->tagOfParentDoline;
}

void AGOSEngine_PN::opn_opcode25() {
	popstack(kJmpClassNum);
	// Jump back to the last doline, which will return 1-1=0.
	// That value then is returned to actCallD, which once again
	// returns it. In the end, this amounts to a setScriptReturn(false)
	// (but possibly in a different level than the current one).
	_dolineReturnVal = 1;
	_tagOfActiveDoline = _stackbase->tagOfParentDoline;
}

void AGOSEngine_PN::opn_opcode26() {
	while ((_stackbase != NULL) && (_stackbase->classnum != kJmpClassNum))
		dumpstack();
	dumpstack();
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode27() {
	quitGame();
	// Make sure the quit event is processed immediately.
	delay(0);
}

void AGOSEngine_PN::opn_opcode28() {
	addstack(varval());
	_stackbase->tagOfParentDoline = _tagOfActiveDoline;
	setScriptReturn(false);
}

void AGOSEngine_PN::opn_opcode29() {
	popstack(varval());
	// Jump back to the last doline indicated by the top stackframe.
	// The -1 tells it to simply go on with its business.
	_dolineReturnVal = -1;
	_tagOfActiveDoline = _stackbase->tagOfParentDoline;
}

void AGOSEngine_PN::opn_opcode30() {
	_variableArray[1] = varval();
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode31() {
	int a, slot = 0;
	char bf[60];

	if ((a = varval()) > 2) {
		setScriptReturn(false);
		return;
	}

	switch (a) {
		case 0:
			getFilename();
			slot = matchSaveGame(_saveFile, countSaveGames());
			strcpy(bf, genSaveName(slot));
			break;
		case 1:
			strcpy(bf, "pn.sav");
			break;
		case 2:
			// NOTE: Is this case ever used?
			error("opn_opcode31: case 2");
			break;
	}

	if (slot == -1) {
		setScriptReturn(false);
	} else {
		a = loadFile(bf);
		if (a)
			setScriptReturn(badload(a));
		else
			setScriptReturn(true);
	}
}

void AGOSEngine_PN::opn_opcode32() {
	char bf[60];
	int a, slot;

	a = varval();
	if (a > 2) {
		setScriptReturn(true);
		return;
	}

	uint16 curSlot = countSaveGames();
	switch (a) {
		case 0:
			getFilename();
			slot = matchSaveGame(_saveFile, curSlot);
			if (slot != -1)
				strcpy(bf, genSaveName(slot));
			else
				strcpy(bf, genSaveName(curSlot));
			break;
		case 1:
			strcpy(bf, "pn.sav");
			break;
		case 2:
			// NOTE: Is this case ever used?
			error("opn_opcode32: case 2");
			break;
	}

	a = saveFile(bf);
	setScriptReturn(a);
}

void AGOSEngine_PN::opn_opcode33() {
	setScriptReturn((varval() < 3) ? 1 : 0);
}

void AGOSEngine_PN::opn_opcode34() {
	uint16 msgNum1, msgNum2;
	varval();
	getResponse((int)_variableArray[166], (int)_variableArray[167], msgNum1, msgNum2);
	_variableArray[168]= msgNum1;
	_variableArray[169]= msgNum2;
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode35() {
	int a;
	uint8 *sav = _workptr;
	varval();
	a = varval();
	if ((a = gvwrd((uint8 *)_wordcp, a)) == -1) {
		setScriptReturn(false);
		return;
	}

	writeval(sav, a);
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode36() {
	for (int i = 0; i < _dataBase[57] + 1; ++i)
		_wordcp[i] = 0;
	if (Common::isSpace(*_inpp))
		while ((*_inpp) && (Common::isSpace(*_inpp)))
			_inpp++;
	if (*_inpp == 0) {
		setScriptReturn(false);
		return;
	}
	_curwrdptr = _inpp;
	_wordcp[0] = *_inpp++;
	if ((_wordcp[0] == '.') || (_wordcp[0] == ',') || (_wordcp[0] == '"')) {
		setScriptReturn(true);
		return;
	}

	int ct = 1;
	while ((*_inpp != '.') && (*_inpp != ',') && (!Common::isSpace(*_inpp)) && (*_inpp != '\0') &&
		(*_inpp!='"')) {
		if (ct < _dataBase[57])
			_wordcp[ct++] = *_inpp;
		_inpp++;
	}
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode37() {
	_curwrdptr = NULL;

	_inputReady = true;
	interact(_inputline, 49);

	if ((_inpp = strchr(_inputline,'\n')) != NULL)
		*_inpp = '\0';
	_inpp = _inputline;
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode38() {
	_noScanFlag = 1;
	clearInputLine();
	writeval(_workptr, _keyPressed.ascii);
	_keyPressed.reset();
	_noScanFlag = 0;
	varval();
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode39() {
	pcf((uint8)varval());
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode40() {
	int a = doaction();
	if (_dolineReturnVal != 0)
		return;
	int b = doaction();
	setScriptReturn(a | b);
}

void AGOSEngine_PN::opn_opcode41() {
	int a = doaction();
	if (_dolineReturnVal != 0)
		return;
	int b = doaction();
	setScriptReturn(a & b);
}

void AGOSEngine_PN::opn_opcode42() {
	int a = doaction();
	if (_dolineReturnVal != 0)
		return;
	int b = doaction();
	setScriptReturn(a ^ b);
}

void AGOSEngine_PN::opn_opcode43() {
	int a = doaction();
	setScriptReturn(!a);
}

void AGOSEngine_PN::opn_opcode44() {
	pcf((uint8)254);
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode45() {
	uint8 *myptr;
	int x;

	if (_havinit == 0) {
		_seed = (int16)getTime();
		_havinit = 1;
	}
	_seed = 1 + (75 * (_seed + 1) - 1) % 65537;
	myptr = _workptr;
	varval();
	x = varval();
	if (x == 0)
		error("Illegal range specified for RANDOM");
	writeval(myptr, (_seed % x));
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode46() {
	char *x = _curwrdptr;
	if (x == NULL) {
		setScriptReturn(true);
		return;
	}
	pcf(*x);
	if ((*x == '.') || (*x == '"') || (*x == ',')) {
		setScriptReturn(true);
		return;
	}
	x++;
	while ((*x != '.') && (*x != ',') && (*x != '"') && (!Common::isSpace(*x)) && (*x != '\0'))
		pcf(*x++);
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode47() {
	pmesd(varval() * 256 + varval());
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode48() {
	pmesd(varval() * 256 + varval());
	pcf((uint8)'\n');
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode49() {
	setScriptReturn(findentry());
}

void AGOSEngine_PN::opn_opcode50() {
	_fnst = 0;
	setScriptReturn(findset());
}

void AGOSEngine_PN::opn_opcode51() {
	_fnst = varval();
	setScriptReturn(findset());
}

void AGOSEngine_PN::opn_opcode52() {
	int32 mode = varval();
	if (mode == 1) {
		setWindowImage(mode, varval(), true);
	} else {
		setWindowImageEx(mode, varval());
	}

	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode53() {
	vc27_resetSprite();
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode54() {
	stopAnimate(varval());
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode55() {
	varval();
	varval();
	varval();
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode56() {
	varval();
	varval();
	varval();
	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode57() {
	uint16 windowNum = varval();
	uint16 vgaSpriteId = varval();
	int16 x = varval();
	int16 y = varval();
	uint16 palette = varval();

	_videoLockOut |= 0x40;
	animate(windowNum, 0, vgaSpriteId, x, y, palette);
	_videoLockOut &= ~0x40;

	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode62() {
	int32 zoneNum = varval();

	_videoLockOut |= 0x80;

	vc29_stopAllSounds();

	_hitCalled = 0;
	_oneClick = 0;

	loadZone(zoneNum);

	setWindowImage(2, 2);

	_copyScnFlag = 0;
	_vgaSpriteChanged = 0;

	_videoLockOut &= ~0x80;

	setScriptReturn(true);
}

void AGOSEngine_PN::opn_opcode63() {
	int a = readfromline();
	switch (a) {
		case 65:
			setScriptReturn(inventoryOn(varval()));
			break;
		case 64:
			setScriptReturn((_videoLockOut & 0x10) != 0);
			break;
		case 63:
			setScriptReturn(inventoryOff());
			break;
		default:
			error("opn_opcode63: unknown code %d", a);
	}
}

int AGOSEngine_PN::inventoryOn(int val) {
	writeVariable(210, val);
	if (_videoLockOut & 0x10) {
		iconPage();
	} else {
		_videoLockOut |= 0x10;
		_hitAreaList = _invHitAreas;

		_windowArray[2]->textColor = 0;
		windowPutChar(_windowArray[2], 13);

		clearVideoWindow(4, 0);
		drawIconHitBar();

		_objects = _variableArray[211];
		_objectCountS = -1;
		iconPage();
	}
	return 1;
}

int AGOSEngine_PN::inventoryOff() {
	if (_videoLockOut & 0x10) {
		_windowArray[2]->textColor = 15;

		restoreBlock(48, 2, 272, 130);

		_hitAreaList = _hitAreas;
		_videoLockOut &= ~0x10;
		_vgaSpriteChanged++;
	}
	return 1;
}

// -----------------------------------------------------------------------
// Personal Nightmare Script Code
// -----------------------------------------------------------------------


int AGOSEngine_PN::bitextract(uint32 ptr, int offs) {
	const byte mask = 0x80 >> (offs % 8);
	return ((mask & _dataBase[ptr + offs / 8]) != 0);
}

uint16 AGOSEngine_PN::getptr(uint32 pos) {
	if (pos > _dataBaseSize)
		error("getptr: Read beyond EOF (%d)", pos);
	return (int)READ_LE_UINT16(_dataBase + pos);
}

uint32 AGOSEngine_PN::getlong(uint32 pos) {
	// Only actually reads 24bit though
	if (pos > _dataBaseSize)
		error("getlong: Read beyond EOF (%d)", pos);
	return (uint32)READ_LE_UINT24(_dataBase + pos);
}

int AGOSEngine_PN::varval() {
	int a;
	int b;

	a = readfromline();
	if (a < 247) {
		return a;
	}

	switch (a) {
		case 249:
			b = readfromline();
			return (int)(b + 256 * readfromline());
		case 250:
			return readfromline();
		case 251:
			return (int)_variableArray[varval()];
		case 252:
			b = varval();
			return (int)_dataBase[_quickptr[0] + b * _quickshort[0] + varval()];
		case 254:
			b = varval();
			return (int)_dataBase[_quickptr[3] + b * _quickshort[2] + varval()];
		case 247:
			b = varval();
			return (int)getptr(_quickptr[11] + (b * _quickshort[4]) + (2 * varval()));
		case 248:
			b = varval();
			return (int)getptr(_quickptr[12] + (b * _quickshort[5]) + (2 * varval()));
		case 253:
			b = varval();
			return bitextract((int32)_quickptr[1] + b * _quickshort[1], varval());
		case 255:
			b = varval();
			return bitextract((int32)_quickptr[4] + b * _quickshort[3], varval());
		default:
			error("VARVAL : Illegal code %d encountered", a);
	}
}

void AGOSEngine_PN::writeval(uint8 *ptr, int val) {
	uint8 *savpt = _workptr;
	int lsav = _linct, a, b, x;
	_workptr = ptr;
	_linct = 255;

	if ((a = readfromline()) < 247)
		error("writeval: Write to constant (%d)", a);

	switch (a) {
		case 249:
			error("writeval: Write to constant (%d)", a);
			break;
		case 250:
			error("writeval: Write to constant (%d)", a);
			break;
		case 251:
			_variableArray[varval()] = val;
			break;
		case 252:
			b = varval();
			_dataBase[_quickptr[0] + b * _quickshort[0] + varval()] = val;
			break;
		case 254:
			b = varval();
			_dataBase[_quickptr[3] + b * _quickshort[2] + varval()] = val;
			break;
		case 247:
			b = varval();
			x = _quickptr[11] + b * _quickshort[4] + varval() * 2;
			WRITE_LE_UINT16(_dataBase + x, val);
			break;
		case 248:
			b = varval();
			x = _quickptr[12] + b * _quickshort[5] + varval() * 2;
			WRITE_LE_UINT16(_dataBase + x, val);
			break;
		case 253:
			b = varval();
			setbitf((uint32)_quickptr[1] + b * _quickshort[1], varval(), val);
			break;
		case 255:
			b = varval();
			setbitf((uint32)_quickptr[4] + b * _quickshort[3], varval(), val);
			break;
		default:
			error("WRITEVAL : undefined evaluation %d", a);
	}
	_linct = lsav;
	_workptr = savpt;
}

void AGOSEngine_PN::setbitf(uint32 ptr, int offs, int val) {
	ptr += offs / 8;
	const byte mask = 0x80 >> (offs % 8);
	if (val != 0)
		_dataBase[ptr] |= mask;
	else
		_dataBase[ptr] &= ~mask;
}

int AGOSEngine_PN::actCallD(int n) {
	int pf[8];
	funcentry(pf, n);
	addstack(kJmpClassNum);
	funccpy(pf);
	setposition(n, 0);
	return doline(1);
}

int AGOSEngine_PN::doaction() {
	if (_linct == 0)
		return 0;

	_opcode = readfromline();

	if (_opcode > 63) {
		return (actCallD(_opcode - 64));
	}

	setScriptReturn(0);
	executeOpcode(_opcode);
	delay(0);

	return getScriptReturn();
}

int AGOSEngine_PN::doline(int needsave) {
	assert(!_stackbase == !needsave);

	int x;
	int myTag = ++_tagOfActiveDoline;	// Obtain a unique tag for this doline invocation
	_dolineReturnVal = 0;

	if (_stackbase && needsave)
		_stackbase->tagOfParentDoline = myTag;

	do {
		_linct = ((*_linebase) & 127) - 1;
		_workptr = _linebase + 1;
		if (*_linebase > 127) {
			x = varval();
			if (x != (int)_variableArray[1])
				goto skipln;
		}

		do {
			x = doaction();

			if (_dolineReturnVal != 0) {
				if (_tagOfActiveDoline != myTag)
					return 0;

				x = _dolineReturnVal;
				_dolineReturnVal = 0;

				if (x > 0) {
					if (x != 3)
						dumpstack();
					// Restore the active jmpbuf to its previous value,
					// then return _dolineReturnVal-1 (will be 2-1=1 or 1-1=0).
					_tagOfActiveDoline = myTag - 1;
					return (x - 1);
				}
			}

		} while (x && !shouldQuit());

skipln:
		_linebase += 127 & *_linebase;
		_linembr++;
	} while (!shouldQuit());

	return 0;
}

int AGOSEngine_PN::findentry() {
	int stepmt;
	int curObj = 0;
	uint32 ofs = _quickptr[11];
	int c1, c2;

	c1 = varval();
	c2 = varval();
	stepmt = _quickshort[4];

	while (curObj < _quickshort[6]) {
		if (((c1 == 255) || (c1 == getptr(ofs))) &&
			(c2 == getptr(ofs + 2))) {
				_variableArray[23] = curObj;
				return 1;
		}
		curObj++;
		ofs += stepmt;
	}
	return 0;
}

int AGOSEngine_PN::findset() {
	int curObj = _fnst;
	int c1, c2, c3, c4;
	int stepmt = _quickshort[4];
	uint32 ofs = _quickptr[11] + stepmt * curObj;
	c1 = varval();
	c2 = varval();
	c3 = varval();
	c4 = varval();
	while (curObj < _quickshort[6]) {
		if (((c1 ==255) || (c1 == getptr(ofs))) &&
			((c2 == 255) || (c2 == getptr(ofs + 2))) &&
			((c3 == 255) || (c3 == getptr(ofs + 4))) &&
			((c4 == 255) || (c4 == getptr(ofs + 6)))) {
				_variableArray[23] = curObj;
				_fnst = curObj + 1;
				return 1;
		}
		curObj++;
		ofs += stepmt;
	}
	return 0;
}

void AGOSEngine_PN::funccpy(int *store) {
	for (int i = 24; i < 32; ++i) {
		_variableArray[i] = *store++;
	}
}

void AGOSEngine_PN::funcentry(int *store, int procn) {
	int numParams = _dataBase[getlong(_quickptr[6] + 3 * procn)];
	for (int i = 0; i < numParams; ++i) {
		*store++ = varval();
	}
}

int AGOSEngine_PN::gvwrd(uint8 *wptr, int mask) {
	int val = 0, code = 0, q = _dataBase[57];
	uint8 *vocbase = _dataBase + getlong(15);
	while (*vocbase != 255) {
		if (*vocbase < 0x80) {
			val = vocbase[q] + 256 * vocbase[q + 1];
			code = vocbase[q + 2];
		}
		if (wrdmatch(vocbase, mask, wptr, code))
			return val;
		vocbase += (*vocbase > 127) ? q : q + 3;
	}
	return -1;
}


int AGOSEngine_PN::setposition(int process, int line) {
	uint8 *ourptr;
	int np;
	int ct;

	ourptr = _dataBase + getlong(_quickptr[6] + 3 * process);
	np = *ourptr++;
	for (ct = 0; ct < line; ++ct) {
		ourptr += (127 & *ourptr);
	}

	while (true) {
		_linebase = ourptr;
		_linct = (127 & *ourptr) - 1;
		if (*ourptr++ <= 127)
			break;

		ct = varval();
		if (ct == (int)_variableArray[1])
			break;

		ourptr += _linct - 1;
		line++;
	}

	_linembr = line;
	_procnum = process;
	_variableArray[0] = process;
	_workptr = ourptr;
	return np;
}

int AGOSEngine_PN::wrdmatch(uint8 *word1, int mask1, uint8 *word2, int mask2) {
	uint8 sv;

	if ((mask1 & mask2) == 0)
		return 0;

	sv = *word1;
	*word1 &= 127;
	if (scumm_strnicmp((const char *)word1, (const char *)word2, _dataBase[57])) {
		*word1 = sv;
		return 0;
	}
	*word1 = sv;
	return 1;
}

// -----------------------------------------------------------------------
// Personal Nightmare Stack Code
// -----------------------------------------------------------------------

void AGOSEngine_PN::addstack(int type) {
	StackFrame *a;
	int i;

	a = (StackFrame *)calloc(1, sizeof(StackFrame));
	if (a == NULL)
		error("addstack: Out of memory - stack overflow");

	a->nextframe = _stackbase;
	_stackbase = a;

	for (i = 0; i < 6; ++i)
		a->flag[i] = _variableArray[i];
	for (i = 0; i < 8; ++i)
		a->param[i] = _variableArray[24 + i];
	a->classnum = type;
	a->ll = _linct;
	a->linenum = _linembr;
	a->linpos = _workptr;
	a->lbase = _linebase;
	a->process = _procnum;
}

void AGOSEngine_PN::dumpstack() {
	StackFrame *a;

	if (_stackbase == NULL)
		error("dumpstack: Stack underflow or unknown longjmp");

	a = _stackbase->nextframe;
	free((char *)_stackbase);
	_stackbase = a;
}

void AGOSEngine_PN::popstack(int type) {
	int i = 0;

	while ((_stackbase != NULL) && (_stackbase->classnum != type)) {
		dumpstack();
		++i;
	}

	if (_stackbase == NULL)
		error("popstack: Stack underflow or unknown longjmp");

	_linct = _stackbase->ll;
	_linebase = _stackbase->lbase;
	_workptr = _stackbase->linpos;
	_procnum = _stackbase->process;
	_linembr = _stackbase->linenum;
	for (i = 0; i < 6; ++i)
		_variableArray[i] = _stackbase->flag[i];
	for (i = 0; i < 8; ++i)
		_variableArray[24 + i] = _stackbase->param[i];
}

} // End of namespace AGOS
