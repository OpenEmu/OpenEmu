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

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/expression.h"
#include "gob/script.h"
#include "gob/hotspots.h"
#include "gob/scenery.h"
#include "gob/sound/sound.h"

namespace Gob {

Inter::Inter(GobEngine *vm) : _vm(vm), _varStack(600) {
	_terminate = 0;
	_break = false;

	for (int i = 0; i < 8; i++) {
		_animPalLowIndex[i] = 0;
		_animPalHighIndex[i] = 0;
		_animPalDir[i] = 0;
	}

	_breakFromLevel = 0;
	_nestLevel = 0;

	_soundEndTimeKey = 0;
	_soundStopVal = 0;

	_lastBusyWait = 0;
	_noBusyWait = false;

	_variables = 0;
}

Inter::~Inter() {
	delocateVars();
}

void Inter::setupOpcodes() {
	setupOpcodesDraw();
	setupOpcodesFunc();
	setupOpcodesGob();
}

void Inter::executeOpcodeDraw(byte i) {
	debugC(1, kDebugDrawOp, "opcodeDraw %d [0x%X] (%s)", i, i, getDescOpcodeDraw(i));

	if (_opcodesDraw[i].proc && _opcodesDraw[i].proc->isValid())
		(*_opcodesDraw[i].proc)();
	else
		warning("unimplemented opcodeDraw: %d [0x%X]", i, i);
}

void Inter::executeOpcodeFunc(byte i, byte j, OpFuncParams &params) {
	debugC(1, kDebugFuncOp, "opcodeFunc %d.%d [0x%X.0x%X] (%s)",
			i, j, i, j, getDescOpcodeFunc(i, j));

	int n = i * 16 + j;
	if ((i <= 4) && (j <= 15) && _opcodesFunc[n].proc && _opcodesFunc[n].proc->isValid())
		(*_opcodesFunc[n].proc)(params);
	else
		warning("unimplemented opcodeFunc: %d.%d [0x%X.0x%X]", i, j, i, j);
}

void Inter::executeOpcodeGob(int i, OpGobParams &params) {
	debugC(1, kDebugGobOp, "opcodeGoblin %d [0x%X] (%s)",
			i, i, getDescOpcodeGob(i));

	OpcodeEntry<OpcodeGob> *op = 0;

	if (_opcodesGob.contains(i))
		op = &_opcodesGob.getVal(i);

	if (op && op->proc && op->proc->isValid()) {
		(*op->proc)(params);
		return;
	}

	_vm->_game->_script->skip(params.paramCount << 1);
	warning("unimplemented opcodeGob: %d [0x%X]", i, i);
}

const char *Inter::getDescOpcodeDraw(byte i) {
	const char *desc = _opcodesDraw[i].desc;

	return ((desc) ? desc : "");
}

const char *Inter::getDescOpcodeFunc(byte i, byte j) {
	if ((i > 4) || (j > 15))
		return "";

	const char *desc = _opcodesFunc[i * 16 + j].desc;

	return ((desc) ? desc : "");
}

const char *Inter::getDescOpcodeGob(int i) {
	if (_opcodesGob.contains(i))
		return _opcodesGob.getVal(i).desc;

	return "";
}

void Inter::initControlVars(char full) {
	*_nestLevel = 0;
	*_breakFromLevel = -1;

	*_vm->_scenery->_pCaptureCounter = 0;

	_break = false;
	_terminate = 0;

	if (full == 1) {
		for (int i = 0; i < 8; i++)
			_animPalDir[i] = 0;
		_soundEndTimeKey = 0;
	}
}

void Inter::renewTimeInVars() {
	TimeDate t;
	_vm->_system->getTimeAndDate(t);

	WRITE_VAR(5, 1900 + t.tm_year);
	WRITE_VAR(6, t.tm_mon + 1);
	WRITE_VAR(7, 0);
	WRITE_VAR(8, t.tm_mday);
	WRITE_VAR(9, t.tm_hour);
	WRITE_VAR(10, t.tm_min);
	WRITE_VAR(11, t.tm_sec);
}

void Inter::storeMouse() {
	int16 x;
	int16 y;

	x = _vm->_global->_inter_mouseX;
	y = _vm->_global->_inter_mouseY;
	_vm->_draw->adjustCoords(1, &x, &y);

	WRITE_VAR(2, x);
	WRITE_VAR(3, y);
	WRITE_VAR(4, (uint32) _vm->_game->_mouseButtons);
}

void Inter::storeKey(int16 key) {
	WRITE_VAR(12, _vm->_util->getTimeKey() - _vm->_game->_startTimeKey);

	storeMouse();
	WRITE_VAR(1, _vm->_sound->blasterPlayingSound());

	if      (key == kKeyUp)
		key =    kShortKeyUp;
	else if (key == kKeyDown)
		key =    kShortKeyDown;
	else if (key == kKeyRight)
		key =    kShortKeyRight;
	else if (key == kKeyLeft)
		key =    kShortKeyLeft;
	else if (key == kKeyEscape)
		key =    kShortKeyEscape;
	else if (key == kKeyBackspace)
		key =    kShortKeyBackspace;
	else if (key == kKeyDelete)
		key =    kShortKeyDelete;
	else if ((key & 0xFF) != 0)
		key &= 0xFF;

	WRITE_VAR(0, key);

	if (key != 0)
		_vm->_util->clearKeyBuf();
}

void Inter::writeVar(uint32 offset, uint16 type, uint32 value) {
	switch (type) {
	case TYPE_VAR_INT8:
	case TYPE_ARRAY_INT8:
		WRITE_VARO_UINT8(offset, value);
		break;

	case TYPE_VAR_INT16:
	case TYPE_VAR_INT32_AS_INT16:
	case TYPE_ARRAY_INT16:
		WRITE_VARO_UINT16(offset, value);
		break;

	default:
		WRITE_VAR_OFFSET(offset, value);
		break;
	}
}

void Inter::funcBlock(int16 retFlag) {
	OpFuncParams params;
	byte cmd;
	byte cmd2;

	params.retFlag = retFlag;

	if (_vm->_game->_script->isFinished())
		return;

	_break = false;
	_vm->_game->_script->skip(1);
	params.cmdCount = _vm->_game->_script->readByte();
	_vm->_game->_script->skip(2);

	if (params.cmdCount == 0) {
		_vm->_game->_script->setFinished(true);
		return;
	}

	int startaddr = _vm->_game->_script->pos();

	params.counter = 0;
	do {
		if (_terminate)
			break;

		// WORKAROUND:
		// The EGA, Mac and Windows versions of gob1 doesn't add a delay after
		// showing images between levels. We manually add it here.
		if ((_vm->getGameType() == kGameTypeGob1) &&
		   (  _vm->isEGA() ||
		     (_vm->getPlatform() == Common::kPlatformMacintosh) ||
		     (_vm->getPlatform() == Common::kPlatformWindows))) {

			int addr = _vm->_game->_script->pos();

			if ((startaddr == 0x18B4 && addr == 0x1A7F && _vm->isCurrentTot("avt005.tot")) || // Zombie, EGA
			    (startaddr == 0x188D && addr == 0x1A58 && _vm->isCurrentTot("avt005.tot")) || // Zombie, Mac
			    (startaddr == 0x1299 && addr == 0x139A && _vm->isCurrentTot("avt006.tot")) || // Dungeon
			    (startaddr == 0x11C0 && addr == 0x12C9 && _vm->isCurrentTot("avt012.tot")) || // Cauldron, EGA
			    (startaddr == 0x11C8 && addr == 0x1341 && _vm->isCurrentTot("avt012.tot")) || // Cauldron, Mac
			    (startaddr == 0x09F2 && addr == 0x0AF3 && _vm->isCurrentTot("avt016.tot")) || // Statue
			    (startaddr == 0x0B92 && addr == 0x0C93 && _vm->isCurrentTot("avt019.tot")) || // Castle
			    (startaddr == 0x17D9 && addr == 0x18DA && _vm->isCurrentTot("avt022.tot")) || // Finale, EGA
			    (startaddr == 0x17E9 && addr == 0x19A8 && _vm->isCurrentTot("avt022.tot"))) { // Finale, Mac

				_vm->_util->longDelay(5000);
			}
		} // End of workaround

		// WORKAROUND:
		// Apart the CD version which is playing a speech in this room, all the versions
		// of Fascination have a too short delay between the storage room and the lab.
		// We manually add it here.
		if ((_vm->getGameType() == kGameTypeFascination) && _vm->isCurrentTot("PLANQUE.tot")) {
				int addr = _vm->_game->_script->pos();
				if ((startaddr == 0x0202 && addr == 0x0330) || // Before Lab, Amiga & Atari, English
				    (startaddr == 0x023D && addr == 0x032D) || // Before Lab, PC floppy, German
				    (startaddr == 0x02C2 && addr == 0x03C2)) { // Before Lab, PC floppy, Hebrew
					warning("Fascination - Adding delay");
					_vm->_util->longDelay(3000);
			}
		} // End of workaround

		cmd = _vm->_game->_script->readByte();

		// WORKAROUND:
		// A VGA version has some broken code in its scripts, this workaround skips the corrupted parts.
		if (_vm->getGameType() == kGameTypeFascination) {
			int addr = _vm->_game->_script->pos();
			if ((startaddr == 0x212D) && (addr == 0x290E) && (cmd == 0x90) && _vm->isCurrentTot("INTRO1.tot")) {
				_vm->_game->_script->skip(2);
				cmd = _vm->_game->_script->readByte();
			}
			if ((startaddr == 0x207D) && (addr == 0x22CE) && (cmd == 0x90) && _vm->isCurrentTot("INTRO2.tot")) {
				_vm->_game->_script->skip(2);
				cmd = _vm->_game->_script->readByte();
			}
		}

		if ((cmd >> 4) >= 12) {
			cmd2 = 16 - (cmd >> 4);
			cmd &= 0xF;
		} else
			cmd2 = 0;

		params.counter++;

		if (cmd2 == 0)
			cmd >>= 4;

		params.doReturn = false;
		executeOpcodeFunc(cmd2, cmd, params);

		if (params.doReturn)
			return;

		if (_vm->shouldQuit())
			break;

		if (_break) {
			if (params.retFlag != 2)
				break;

			if (*_breakFromLevel == -1)
				_break = false;
			break;
		}
	} while (params.counter != params.cmdCount);

	_vm->_game->_script->setFinished(true);
}

void Inter::callSub(int16 retFlag) {
	byte block;

	while (!_vm->shouldQuit() && !_vm->_game->_script->isFinished() &&
			(_vm->_game->_script->pos() != 0)) {

		block = _vm->_game->_script->peekByte();
		if (block == 1)
			funcBlock(retFlag);
		else if (block == 2)
			_vm->_game->_hotspots->evaluate();
		else
			error("Unknown block type %d in Inter::callSub()", block);
	}

	if (!_vm->_game->_script->isFinished() && (_vm->_game->_script->pos() == 0))
		_terminate = 1;
}

void Inter::allocateVars(uint32 count) {
	if (_vm->getEndianness() == kEndiannessBE)
		_variables = new VariablesBE(count * 4);
	else
		_variables = new VariablesLE(count * 4);
}

void Inter::delocateVars() {
	if (_vm->_game)
		_vm->_game->deletedVars(_variables);

	delete _variables;
	_variables = 0;
}

void Inter::storeValue(uint16 index, uint16 type, uint32 value) {
	switch (type) {
	case OP_ARRAY_INT8:
	case TYPE_VAR_INT8:
		WRITE_VARO_UINT8(index, value);
		break;

	case TYPE_VAR_INT16:
	case TYPE_VAR_INT32_AS_INT16:
	case TYPE_ARRAY_INT16:
		WRITE_VARO_UINT16(index, value);
		break;

	default:
		WRITE_VARO_UINT32(index, value);
	}
}

void Inter::storeValue(uint32 value) {
	uint16 type;
	uint16 index = _vm->_game->_script->readVarIndex(0, &type);

	storeValue(index, type, value);
}

void Inter::storeString(uint16 index, uint16 type, const char *value) {
	uint32 maxLength = _vm->_global->_inter_animDataSize * 4 - 1;
	char  *str       = GET_VARO_STR(index);

	switch (type) {
	case TYPE_VAR_STR:
		if (strlen(value) > maxLength)
			warning("Inter_v7::storeString(): String too long");

		Common::strlcpy(str, value, maxLength);
		break;

	case TYPE_IMM_INT8:
	case TYPE_VAR_INT8:
		strcpy(str, value);
		break;

	case TYPE_ARRAY_INT8:
		WRITE_VARO_UINT8(index, atoi(value));
		break;

	case TYPE_VAR_INT16:
	case TYPE_VAR_INT32_AS_INT16:
	case TYPE_ARRAY_INT16:
		WRITE_VARO_UINT16(index, atoi(value));
		break;

	case TYPE_VAR_INT32:
	case TYPE_ARRAY_INT32:
		WRITE_VARO_UINT32(index, atoi(value));
		break;

	default:
		warning("Inter_v7::storeString(): Requested to store a string into type %d", type);
		break;
	}
}

void Inter::storeString(const char *value) {
	uint16 type;
	uint16 varIndex = _vm->_game->_script->readVarIndex(0, &type);

	storeString(varIndex, type, value);
}

uint32 Inter::readValue(uint16 index, uint16 type) {
	switch (type) {
	case TYPE_IMM_INT8:
	case TYPE_VAR_INT8:
	case TYPE_ARRAY_INT8:
		return (uint32)(((int32)((int8)READ_VARO_UINT8(index))));
		break;

	case TYPE_VAR_INT16:
	case TYPE_VAR_INT32_AS_INT16:
	case TYPE_ARRAY_INT16:
		return (uint32)(((int32)((int16)READ_VARO_UINT16(index))));

	default:
		return READ_VARO_UINT32(index);
	}

	return 0;
}

void Inter::handleBusyWait() {
	uint32 now = _vm->_util->getTimeKey();

	if (!_noBusyWait)
		if ((now - _lastBusyWait) <= 20)
			_vm->_util->longDelay(1);

	_lastBusyWait = now;
	_noBusyWait   = false;
}

} // End of namespace Gob
