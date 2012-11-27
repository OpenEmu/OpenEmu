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

#ifdef ENABLE_EOB

#include "kyra/eobcommon.h"
#include "kyra/screen_eob.h"
#include "kyra/script_eob.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {

void EoBCoreEngine::runLevelScript(int block, int flags) {
	_inf->run(block, flags);
}

void EoBCoreEngine::setScriptFlags(uint32 flags) {
	_inf->setFlags(flags);
}

void EoBCoreEngine::clearScriptFlags(uint32 flags) {
	_inf->clearFlags(flags);
}

bool EoBCoreEngine::checkScriptFlags(uint32 flags) {
	return _inf->checkFlags(flags);
}

const uint8 *EoBCoreEngine::initScriptTimers(const uint8 *pos) {
	_scriptTimersCount = 0;

	while (((int16)READ_LE_UINT16(pos)) != -1) {
		_scriptTimers[_scriptTimersCount].func = READ_LE_UINT16(pos);
		pos += 2;
		uint16 ticks = READ_LE_UINT16(pos) * 18;
		_scriptTimers[_scriptTimersCount].ticks = ticks;
		pos += 2;
		_scriptTimers[_scriptTimersCount++].next = _system->getMillis() + ticks * _tickLength;
	}

	return pos;
}

void EoBCoreEngine::updateScriptTimers() {
	bool timerUpdate = false;
	if ((_scriptTimersMode & 2) && _stepsUntilScriptCall && _stepCounter > _stepsUntilScriptCall) {
		_inf->run(0, 0x20);
		_stepCounter = 0;
		timerUpdate = true;
	}

	if (_scriptTimersMode & 1) {
		for (int i = 0; i < _scriptTimersCount; i++) {
			if (_scriptTimers[i].next < _system->getMillis()) {
				_inf->run(_scriptTimers[i].func, _flags.gameID == GI_EOB1 ? 0x20 : 0x80);
				_scriptTimers[i].next = _system->getMillis() + _scriptTimers[i].ticks * _tickLength;
				_sceneUpdateRequired = true;
				timerUpdate = true;
			}
		}
	}

	if (timerUpdate)
		updateScriptTimersExtra();
}

EoBInfProcessor::EoBInfProcessor(EoBCoreEngine *engine, Screen_EoB *screen) : _vm(engine), _screen(screen),
	_commandMin(engine->game() == GI_EOB1 ? -27 : -31) {

#define Opcode(x) _opcodes.push_back(new InfOpcode(new InfProc(this, &EoBInfProcessor::x), #x))
#define OpcodeAlt(x) if (_vm->game() == GI_EOB1) { Opcode(x##_v1); } else { Opcode(x##_v2); }
	Opcode(oeob_setWallType);
	Opcode(oeob_toggleWallState);
	Opcode(oeob_openDoor);
	Opcode(oeob_closeDoor);
	Opcode(oeob_replaceMonster);
	Opcode(oeob_movePartyOrObject);
	Opcode(oeob_moveInventoryItemToBlock);
	OpcodeAlt(oeob_printMessage);
	Opcode(oeob_setFlags);
	Opcode(oeob_playSoundEffect);
	Opcode(oeob_removeFlags);
	Opcode(oeob_modifyCharacterHitPoints);
	Opcode(oeob_calcAndInflictCharacterDamage);
	Opcode(oeob_jump);
	Opcode(oeob_end);
	Opcode(oeob_returnFromSubroutine);
	Opcode(oeob_callSubroutine);
	OpcodeAlt(oeob_eval);
	Opcode(oeob_deleteItem);
	Opcode(oeob_loadNewLevelOrMonsters);
	Opcode(oeob_increasePartyExperience);
	OpcodeAlt(oeob_createItem);
	Opcode(oeob_launchObject);
	Opcode(oeob_changeDirection);
	Opcode(oeob_identifyItems);
	Opcode(oeob_sequence);
	Opcode(oeob_delay);
	Opcode(oeob_drawScene);
	Opcode(oeob_dialogue);
	Opcode(oeob_specialEvent);
#undef Opcode
#undef OpcodeAlt

	_scriptData = 0;
	_scriptSize = 0;

	_abortScript = 0;
	_abortAfterSubroutine = 0;
	_dlgResult = 0;
	_preventRest = 0;

	_lastScriptFunc = 0;
	_lastScriptFlags = 0;

	_subroutineStack = new int8*[10];
	memset(_subroutineStack, 0, 10 * sizeof(int8 *));
	_subroutineStackPos = 0;

	_flagTable = new uint32[18];
	memset(_flagTable, 0, 18 * sizeof(uint32));

	_stack = new int16[30];
	memset(_stack, 0, 30 * sizeof(int16));
	_stackIndex = 0;

	_activeCharacter = -1;
}

EoBInfProcessor::~EoBInfProcessor() {
	delete[] _subroutineStack;
	delete[] _flagTable;
	delete[] _stack;
	delete[] _scriptData;

	for (Common::Array<const InfOpcode *>::const_iterator a = _opcodes.begin(); a != _opcodes.end(); ++a)
		delete *a;

	_opcodes.clear();
}

void EoBInfProcessor::loadData(const uint8 *data, uint32 dataSize) {
	delete[] _scriptData;
	_scriptSize = dataSize;
	_scriptData = new int8[_scriptSize];
	memcpy(_scriptData, data, _scriptSize);
}

void EoBInfProcessor::run(int func, int flags) {
	int o = _vm->_levelBlockProperties[func].assignedObjects;
	if (!o)
		return;

	uint16 f = _vm->_levelBlockProperties[func].flags;

	uint16 subFlags = ((f & 0xfff8) >> 3) | 0xe0;
	if (!(flags & subFlags))
		return;

	_abortScript = 0;
	_abortAfterSubroutine = 0;
	_dlgResult = 0;
	_activeCharacter = -1;

	_lastScriptFunc = func;
	_lastScriptFlags = flags;

	int8 *pos = (int8 *)(_scriptData + o);

	do {
		int8 cmd = *pos++;
		if (cmd <= _commandMin || cmd >= 0)
			continue;
		debugC(3, kDebugLevelScript, "[0x%.04X] EoBInfProcessor::%s()", (uint32)(pos - _scriptData), _opcodes[-(cmd + 1)]->desc.c_str());
		pos += (*_opcodes[-(cmd + 1)]->proc)(pos);
	} while (!_abortScript && !_abortAfterSubroutine);
}

void EoBInfProcessor::setFlags(uint32 flags) {
	_flagTable[17] |= flags;
}

void EoBInfProcessor::clearFlags(uint32 flags) {
	_flagTable[17] &= ~flags;
}

bool EoBInfProcessor::checkFlags(uint32 flags) const {
	return ((_flagTable[17] & flags) == flags) ? true : false;
}

bool EoBInfProcessor::preventRest() const {
	return _preventRest ? true : false;
}

void EoBInfProcessor::loadState(Common::SeekableSubReadStreamEndian &in, bool origFile) {
	_preventRest = (_vm->game() == GI_EOB1 && origFile) ? 0 : in.readByte();
	int numFlags = (_vm->game() == GI_EOB1 && origFile) ? 13 : 18;
	for (int i = 0; i < numFlags; i++)
		_flagTable[i] = in.readUint32();
}

void EoBInfProcessor::saveState(Common::OutSaveFile *out) {
	out->writeByte(_preventRest);
	for (int i = 0; i < 18; i++)
		out->writeUint32BE(_flagTable[i]);
}

void EoBInfProcessor::reset() {
	_preventRest = 0;
	memset(_flagTable, 0, 18 * sizeof(uint32));
}

const char *EoBInfProcessor::getString(uint16 index) {
	if (index == 0xffff)
		return 0;

	int8 *res = _scriptData + READ_LE_UINT16(_scriptData);

	while (index) {
		if (*res++)
			continue;
		index--;
	}

	return (const char *)res;
}

int EoBInfProcessor::oeob_setWallType(int8 *data) {
	int8 *pos = data;

	uint16 block = 0;
	int8 dir = 0;

	switch (*pos++) {
	case -23:
		block = READ_LE_UINT16(pos);
		pos += 2;
		dir = *pos++;
		_vm->_levelBlockProperties[block].walls[dir] = *pos++;
		_vm->checkSceneUpdateNeed(block);
		break;

	case -19:
		_vm->_currentDirection = *pos++;
		break;

	case -9:
		block = READ_LE_UINT16(pos);
		pos += 2;
		dir = *pos++;
		memset(_vm->_levelBlockProperties[block].walls, dir, 4 * sizeof(uint8));
		_vm->checkSceneUpdateNeed(block);
		break;

	default:
		break;
	}

	return pos - data;
}

int EoBInfProcessor::oeob_toggleWallState(int8 *data) {
	int8 *pos = data;

	uint16 block = 0;
	int8 dir = 0;
	uint8 a = 0;
	uint8 b = 0;

	switch (*pos++) {
	case -23:
		block = READ_LE_UINT16(pos);
		pos += 2;
		dir = *pos++;
		a = (uint8)*pos++;
		b = (uint8)*pos++;
		a = (_vm->_levelBlockProperties[block].walls[dir] == a) ? b : a;
		_vm->_levelBlockProperties[block].walls[dir] = a;
		_vm->checkSceneUpdateNeed(block);
		break;

	case -22:
		_vm->processDoorSwitch(READ_LE_UINT16(pos), 0);
		pos += 2;
		break;

	case -9:
		block = READ_LE_UINT16(pos);
		pos += 2;
		a = (uint8)*pos++;
		b = (uint8)*pos++;
		a = (_vm->_levelBlockProperties[block].walls[dir] == a) ? b : a;
		memset(_vm->_levelBlockProperties[block].walls, a, 4 * sizeof(uint8));
		_vm->checkSceneUpdateNeed(block);
		break;

	default:
		break;
	}

	return pos - data;
}

int EoBInfProcessor::oeob_openDoor(int8 *data) {
	int8 *pos = data;
	_vm->openDoor(READ_LE_UINT16(pos));
	pos += 2;
	return pos - data;
}

int EoBInfProcessor::oeob_closeDoor(int8 *data) {
	int8 *pos = data;
	_vm->closeDoor(READ_LE_UINT16(pos));
	pos += 2;
	return pos - data;
}

int EoBInfProcessor::oeob_replaceMonster(int8 *data) {
	int8 *pos = data;
	_vm->replaceMonster(pos[1], READ_LE_UINT16(pos + 2), pos[4], pos[5], pos[6], pos[7], pos[8], pos[9], READ_LE_UINT16(pos + 10), READ_LE_UINT16(pos + 12));
	pos += 14;
	return pos - data;
}

int EoBInfProcessor::oeob_movePartyOrObject(int8 *data) {
	int8 *pos = data;

	int8 a = *pos++;
	uint16 b = 0xffff;
	uint16 c = 0;
	uint16 d = 0;

	if (_vm->game() == GI_EOB2 && a == -31) {
		b = READ_LE_UINT16(pos);
		pos += 2;
	}

	if (_vm->game() == GI_EOB1) {
		if (a != -15) {
			c = READ_LE_UINT16(pos);
			pos += 2;
		}
		d = READ_LE_UINT16(pos);
		pos += 2;
	}

	if (_vm->game() == GI_EOB2 && a != -31 && a != -11) {
		c = READ_LE_UINT16(pos);
		pos += 2;
		d = READ_LE_UINT16(pos);
		pos += 2;
	}

	if (a == -13) {
		// move monster from block c to block d
		for (int i = 0; i < 30; i++) {
			if (_vm->_monsters[i].block != c)
				continue;
			_vm->placeMonster(&_vm->_monsters[i], d, _vm->_monsters[i].pos);
		}
		debugC(5, kDebugLevelScript, "         - move monsters on block '0x%.04X' to block '0x%.04X'", c, d);

	} else if (a == -24) {
		// move party to block d
		int ba = _dlgResult;
		int bb = _lastScriptFunc;
		int bc = _lastScriptFlags;
		int bd = _abortScript;
		int be = _activeCharacter;
		int bf = _subroutineStackPos;

		_vm->moveParty(d);
		debugC(5, kDebugLevelScript, "         - move party to block '0x%.04X'", d);

		_dlgResult = ba;
		_lastScriptFunc = bb;
		_lastScriptFlags = bc;
		_abortScript = bd;
		_activeCharacter = be;
		if (!_abortAfterSubroutine)
			_subroutineStackPos = bf;
		_vm->_sceneDefaultUpdate = 0;

	} else if ((a == -31 && _vm->game() == GI_EOB2) || a == -11) {
		// move item
		int8 e = _vm->_currentLevel;
		int8 f = _vm->_currentLevel;

		if (_vm->game() == GI_EOB2) {
			e = (*pos++ == -21) ? _vm->_currentLevel : *pos++;
			c = READ_LE_UINT16(pos);
			pos += 2;
			f = (*pos++ == -21) ? _vm->_currentLevel : *pos++;
			d = READ_LE_UINT16(pos);
			pos += 2;
		}

		if (e == _vm->_currentLevel) {
			int i = _vm->countQueuedItems(_vm->_levelBlockProperties[c].drawObjects, -1, (int16)b, 0, 1);
			while (i) {
				int p = _vm->_items[i].pos;
				_vm->getQueuedItem((Item *)&_vm->_levelBlockProperties[c].drawObjects, 0, i);
				if (_vm->_currentLevel == f) {
					_vm->setItemPosition((Item *)&_vm->_levelBlockProperties[d].drawObjects, d, i, p);
				} else {
					_vm->_items[i].level = f;
					_vm->_items[i].block = d;
					if (p < 8)
						_vm->_items[i].pos = p & 3;
				}
				i = _vm->countQueuedItems(_vm->_levelBlockProperties[c].drawObjects, -1, (int16)b, 0, 1);
			}

			for (i = 0; i < 10; i++) {
				if (_vm->_flyingObjects[i].enable != 1 || _vm->_flyingObjects[i].curBlock != c)
					continue;
				if (f == _vm->_currentLevel || _vm->game() == GI_EOB1)
					_vm->_flyingObjects[i].curBlock = d;
				else
					_vm->_flyingObjects[i].enable = 0;
			}

		} else {
			for (int i = 0; i < 600; i++) {
				if (_vm->_items[i].level != e || _vm->_items[i].block != c)
					continue;
				_vm->_items[i].level = f;
				_vm->_items[i].block = d;
			}
		}
		debugC(5, kDebugLevelScript, "         - move items from level '%d', block '0x%.04X' to level '%d', block '0x%.04X'", c, e, d, f);
	}

	_vm->_sceneUpdateRequired = true;
	return pos - data;
}

int EoBInfProcessor::oeob_moveInventoryItemToBlock(int8 *data) {
	int8 *pos = data;
	int8 c = *pos++;
	uint16 block = READ_LE_UINT16(pos);
	pos += 2;
	int8 p = *pos++;

	if (c == -1)
		c = _vm->rollDice(1, 6, -1);

	while (!(_vm->_characters[c].flags & 1)) {
		if (++c == 5)
			c = 0;
	}

	if (_vm->_currentControlMode && (_vm->_updateCharNum == c))
		return pos - data;

	int slot = _vm->rollDice(1, 27, 0);
	int itm = 0;
	int i = 0;

	for (; i < 27; i++) {
		if ((!_vm->_currentControlMode && slot > 1) || slot == 16)
			continue;

		itm = _vm->_characters[c].inventory[slot];

		if (!itm)
			continue;

		if (_vm->_dscItemShapeMap[_vm->_items[itm].icon] >= 15)
			break;

		if (++slot == 27)
			slot = 0;
	}

	if (i < 27 && itm) {
		_vm->_characters[c].inventory[slot] = 0;
		_vm->setItemPosition((Item *)&_vm->_levelBlockProperties[block].drawObjects, block, itm, p);
	}

	return pos - data;
}

int EoBInfProcessor::oeob_printMessage_v1(int8 *data) {
	static const char colorConfig[] = "\x6\x21\x2\x21";
	char col[5];
	int8 *pos = data;

	strcpy(col, colorConfig);
	const char *str = (const char *)pos;
	pos += (strlen(str) + 1);

	col[1] = *pos++;
	col[3] = *pos++;
	_vm->txt()->printMessage(col);
	_vm->txt()->printMessage(str);

	col[1] = _screen->_curDim->unk8;
	col[3] = _screen->_curDim->unkA;
	_vm->txt()->printMessage(col);
	_vm->txt()->printMessage("\r");

	return pos - data;
}

int EoBInfProcessor::oeob_printMessage_v2(int8 *data) {
	int8 *pos = data;
	uint16 str = READ_LE_UINT16(pos);
	pos += 2;
	uint8 col = (uint8)*pos;
	pos += 2;

	int c = 0;
	if (_activeCharacter == -1) {
		c = _vm->rollDice(1, 6, -1);
		while (!_vm->testCharacter(c, 3))
			c = (c + 1) % 6;
	} else {
		c = _activeCharacter;
	}

	_vm->txt()->printMessage(getString(str), col, _vm->_characters[c].name);
	_vm->txt()->printMessage("\r");

	return pos - data;
}

int EoBInfProcessor::oeob_setFlags(int8 *data) {
	int8 *pos = data;
	int8 b = 0;

	switch (*pos++) {
	case -47:
		_preventRest = 0;
		debugC(5, kDebugLevelScript, "         - set preventRest to 0");
		break;

	case -28:
		_dlgResult = 1;
		debugC(5, kDebugLevelScript, "         - set dlgResult to 1");
		break;

	case -17:
		_flagTable[_vm->_currentLevel] |= (1 << (*pos++));
		debugC(5, kDebugLevelScript, "         - set level flag '%d' for current level (current level = '%d')", *(pos - 1), _vm->_currentLevel);
		break;

	case -16:
		_flagTable[17] |= (1 << (*pos++));
		debugC(5, kDebugLevelScript, "         - set global flag '%d'", *(pos - 1));
		break;

	case -13:
		b = *pos++;
		_vm->_monsters[b].flags |= (1 << (*pos++));
		_vm->_monsters[b].mode = 0;
		debugC(5, kDebugLevelScript, "         - set monster flag '%d' for monster '%d'", *(pos - 1), b);
		break;

	default:
		break;
	}

	return pos - data;
}

int EoBInfProcessor::oeob_playSoundEffect(int8 *data) {
	int8 *pos = data;
	uint16 block = READ_LE_UINT16(pos + 1);

	if (block) {
		_vm->snd_processEnvironmentalSoundEffect(pos[0], block);
	} else {
		_vm->snd_playSoundEffect(pos[0]);
	}

	pos += 3;
	return pos - data;
}

int EoBInfProcessor::oeob_removeFlags(int8 *data) {
	int8 *pos = data;
	int8 a = *pos++;

	switch (a) {
	case -47:
		_preventRest = 1;
		debugC(5, kDebugLevelScript, "         - set preventRest to 1");
		break;

	case -28:
		_dlgResult = 0;
		debugC(5, kDebugLevelScript, "         - set dlgResult to 0");
		break;

	case -17:
		_flagTable[_vm->_currentLevel] &= ~(1 << (*pos++));
		debugC(5, kDebugLevelScript, "         - clear level flag '%d' for current level (current level = '%d')", *(pos - 1), _vm->_currentLevel);
		break;

	case -16:
		_flagTable[17] &= ~(1 << (*pos++));
		debugC(5, kDebugLevelScript, "         - clear global flag '%d'", *(pos - 1));
		break;

	default:
		break;
	}

	return pos - data;
}

int EoBInfProcessor::oeob_modifyCharacterHitPoints(int8 *data) {
	int8 *pos = data;
	int8 c = *pos++;
	int8 p = *pos++;

	if (c == -1) {
		for (c = 0; c < 6; c++)
			_vm->modifyCharacterHitpoints(c, p);
	} else {
		_vm->modifyCharacterHitpoints(c, p);
	}

	return pos - data;
}

int EoBInfProcessor::oeob_calcAndInflictCharacterDamage(int8 *data) {
	int8 *pos = data;
	int charIndex = *pos++;
	int times = *pos++;
	int itemOrPips = *pos++;
	int useStrModifierOrBase = *pos++;

	int flg = (charIndex == -1) ? 4 : 0;
	int savingThrowType = 5;
	int savingThrowEffect = 1;

	if (_vm->game() == GI_EOB2) {
		flg = *pos++;
		savingThrowType = *pos++;
		savingThrowEffect = *pos++;
	} else if (!itemOrPips) {
		useStrModifierOrBase = times;
		times = 0;
	}

	if (charIndex == -1) {
		for (int i = 0; i < 6; i++)
			_vm->calcAndInflictCharacterDamage(i, times, itemOrPips, useStrModifierOrBase, flg, savingThrowType, savingThrowEffect);
	} else {
		_vm->calcAndInflictCharacterDamage(charIndex, times, itemOrPips, useStrModifierOrBase, flg, savingThrowType, savingThrowEffect);
	}
	return pos - data;
}

int EoBInfProcessor::oeob_jump(int8 *data) {
	int8 *pos = data;
	pos = _scriptData + READ_LE_UINT16(pos);
	return pos - data;
}

int EoBInfProcessor::oeob_end(int8 *data) {
	_abortScript = 1;
	_subroutineStackPos = 0;
	return 0;
}

int EoBInfProcessor::oeob_returnFromSubroutine(int8 *data) {
	int8 *pos = data;

	if (_subroutineStackPos)
		pos = _subroutineStack[--_subroutineStackPos];
	else
		_abortScript = 1;

	return pos - data;
}

int EoBInfProcessor::oeob_callSubroutine(int8 *data) {
	int8 *pos = data;
	uint16 offs = READ_LE_UINT16(pos);
	assert(offs < _scriptSize);
	pos += 2;

	if (_subroutineStackPos < 10) {
		_subroutineStack[_subroutineStackPos++] = pos;
		pos = _scriptData + offs;
	}

	return pos - data;
}

int EoBInfProcessor::oeob_eval_v1(int8 *data) {
	int8 *pos = data;
	int8 cmd = *pos++;

	int a = 0;
	int b = 0;
	int i = 0;
	EoBItem *itm = &_vm->_items[_vm->_itemInHand];
	Common::String tempString1;
	Common::String tempString2;

	while (cmd != -18) {
		switch (cmd + 38) {
		case 0:
			a = 1;
			for (i = 0; i < 6; i++) {
				if (!(_vm->_characters[i].flags & 1))
					continue;
				if (_vm->_characters[i].effectFlags & 0x40)
					continue;
				a = 0;
				break;
			}
			_stack[_stackIndex++] = a;
			debugC(5, kDebugLevelScript, "         - check if whole party is invisible - PUSH result: '%d'", a);
			break;

		case 1:
			_stack[_stackIndex++] = _vm->rollDice(pos[0], pos[1], pos[2]);
			debugC(9, kDebugLevelScript, "         - throw dice(s): num = '%d', pips = '%d', offset = '%d' - PUSH result: '%d'", pos[0], pos[1], pos[2], _stack[_stackIndex - 1]);
			pos += 3;
			break;

		case 2:
			cmd = *pos++;
			b = 0;
			for (i = 0; i < 6; i++) {
				if (!(_vm->_characters[i].flags & 1))
					continue;
				if (_vm->_classModifierFlags[_vm->_characters[i].cClass] & cmd) {
					b = 1;
					break;
				}
			}
			_stack[_stackIndex++] = b;
			debugC(5, kDebugLevelScript, "         - check if character with class flags '0x%.02X' is present - PUSH result: '%d'", cmd, b);
			break;

		case 3:
			cmd = *pos++;
			b = 0;
			for (i = 0; i < 6; i++) {
				if (!(_vm->_characters[i].flags & 1))
					continue;
				if ((_vm->_characters[i].raceSex >> 1) == cmd) {
					b = 1;
					break;
				}
			}
			_stack[_stackIndex++] = b;
			debugC(5, kDebugLevelScript, "         - check if character with race '%d' is present - PUSH result: '%d'", cmd, b);
			break;

		case 6:
			_stack[_stackIndex++] = _lastScriptFlags;
			debugC(5, kDebugLevelScript, "         - get script execution flags - PUSH result: '%d'", _lastScriptFlags);
			break;

		case 13:
			itm = &_vm->_items[_vm->_itemInHand];
			switch (*pos++) {
			case -31:
				_stack[_stackIndex++] = itm->type;
				debugC(5, kDebugLevelScript, "         - get hand item type (hand item number = '%d') - PUSH result: '%d'", _vm->_itemInHand, itm->type);
				break;

			case -11:
				_stack[_stackIndex++] = _vm->_itemInHand;
				debugC(5, kDebugLevelScript, "         - get hand item number - PUSH result: '%d'", _vm->_itemInHand);
				break;

			default:
				_stack[_stackIndex++] = itm->value;
				debugC(5, kDebugLevelScript, "         - get hand item value (hand item number = '%d') - PUSH result: '%d'", _vm->_itemInHand, itm->value);
				break;
			}
			break;

		case 15:
			_stack[_stackIndex++] = _vm->_levelBlockProperties[READ_LE_UINT16(pos + 1)].walls[pos[0]];
			debugC(5, kDebugLevelScript, "         - get wall index for block '0x%.04X', direction '%d' - PUSH result: '%d'", READ_LE_UINT16(pos + 1), pos[0], _stack[_stackIndex - 1]);
			pos += 3;
			break;

		case 19:
			_stack[_stackIndex++] = _vm->_currentDirection;
			debugC(5, kDebugLevelScript, "         - get current direction - PUSH result: '%d'", _vm->_currentDirection);
			break;

		case 21:
			_stack[_stackIndex++] = (_flagTable[_vm->_currentLevel] & (1 << (*pos++))) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - test level flag '%d' (current level = '%d') - PUSH result: '%d'", *(pos - 1), _vm->_currentLevel, _stack[_stackIndex - 1]);
			break;

		case 22:
			_stack[_stackIndex++] = (_flagTable[17] & (1 << (*pos++))) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - test global flag '%d' - PUSH result: '%d'", *(pos - 1), _stack[_stackIndex - 1]);
			break;

		case 23:
			_stack[_stackIndex++] = (_vm->_currentBlock == READ_LE_UINT16(pos)) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - compare current block with block '0x%.04X' (current block = '0x%.04X') - PUSH result: '%d'", _vm->_currentBlock, READ_LE_UINT16(pos), _stack[_stackIndex - 1]);
			pos += 2;
			break;

		case 24:
			a = (int16)READ_LE_UINT16(pos);
			pos += 2;
			b = READ_LE_UINT16(pos);
			pos += 2;
			_stack[_stackIndex++] = _vm->countQueuedItems(_vm->_levelBlockProperties[b].drawObjects, a, -1, 0, 1);
			debugC(5, kDebugLevelScript, "         - find item number '%d' on block '0x%.04X' - PUSH result: '%d'", a, b, _stack[_stackIndex - 1]);
			break;

		case 25:
			_stack[_stackIndex++] = (_vm->_levelBlockProperties[READ_LE_UINT16(pos)].flags & 1) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - test block flag '1' for block '0x%.04X' - PUSH result: '%d'", READ_LE_UINT16(pos), _stack[_stackIndex - 1]);
			pos += 2;
			break;

		case 27:
			b = *pos++;
			i = READ_LE_UINT16(pos);
			pos += 2;
			_stack[_stackIndex++] = _vm->countQueuedItems(_vm->_levelBlockProperties[i].drawObjects, -1, b, 1, 1);
			debugC(5, kDebugLevelScript, "         - count items of type '%d' on block '0x%.04X' - PUSH result: '%d'", b, i, _stack[_stackIndex - 1]);
			break;

		case 29:
			_stack[_stackIndex++] = _vm->_levelBlockProperties[READ_LE_UINT16(pos)].walls[0];
			debugC(5, kDebugLevelScript, "         - get wall index 0 for block '0x%.04X' - PUSH result: '%d'", READ_LE_UINT16(pos), _stack[_stackIndex - 1]);
			pos += 2;
			break;

		case 30:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a || b) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - evaluate: POP('%d') || POP('%d') - PUSH result: '%d'", a, b, _stack[_stackIndex - 1]);
			break;

		case 31:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a && b) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - evaluate: POP('%d') && POP('%d') - PUSH result: '%d'", a, b, _stack[_stackIndex - 1]);
			break;

		case 32:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a <= b) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - evaluate: POP('%d') <= POP('%d') - PUSH result: '%d'", a, b, _stack[_stackIndex - 1]);
			break;

		case 33:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a < b) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - evaluate: POP('%d') < POP('%d') - PUSH result: '%d'", a, b, _stack[_stackIndex - 1]);
			break;

		case 34:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a >= b) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - evaluate: POP('%d') >= POP('%d') - PUSH result: '%d'", a, b, _stack[_stackIndex - 1]);
			break;

		case 35:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a > b) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - evaluate: POP('%d') > POP('%d') - PUSH result: '%d'", a, b, _stack[_stackIndex - 1]);
			break;

		case 36:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a != b) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - evaluate: POP('%d') != POP('%d') - PUSH result: '%d'", a, b, _stack[_stackIndex - 1]);
			break;

		case 37:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a == b) ? 1 : 0;
			debugC(5, kDebugLevelScript, "         - evaluate: POP('%d') == POP('%d') - PUSH result: '%d'", a, b, _stack[_stackIndex - 1]);
			break;

		default:
			a = cmd;
			if (a >= 0 && a < 128)
				_stack[_stackIndex++] = a;
			debugC(5, kDebugLevelScript, "         - PUSH value: '%d'", a);
			break;
		}
		cmd = *pos++;
	}

	cmd = _stack[--_stackIndex];
	if (cmd)
		pos += 2;
	else
		pos = _scriptData + READ_LE_UINT16(pos);
	debugC(5, kDebugLevelScript, "         - conditional jump depending on POP('%d')", cmd);

	return pos - data;
}

int EoBInfProcessor::oeob_eval_v2(int8 *data) {
	int8 *pos = data;
	int8 cmd = *pos++;

	int a = 0;
	int b = 0;
	int i = 0;
	EoBItem *itm = (_vm->_itemInHand != -1) ? &_vm->_items[_vm->_itemInHand] : 0;
	Common::String tempString1;
	Common::String tempString2;

	while (cmd != -18) {
		switch (cmd + 50) {
		case 0:
			a = 0;
			b = *pos++;

			for (i = 0; i < 6; i++) {
				if (!_vm->testCharacter(i, 5))
					continue;

				if (_vm->_characters[i].portrait != b) {
					a = 1;
					_activeCharacter = i;
					break;
				}
			}

			_stack[_stackIndex++] = a;
			break;

		case 4:
			_stack[_stackIndex++] = (int16)READ_LE_UINT16(pos);
			pos += 2;
			break;

		case 9:
			switch (*pos++) {
			case -36:
				_stack[_stackIndex++] = _vm->_itemTypes[_vm->_items[_vm->_lastUsedItem].type].extraProperties & 0x7f;
				break;
			case -31:
				_stack[_stackIndex++] = _vm->_items[_vm->_lastUsedItem].type;
				break;
			case -11:
				_stack[_stackIndex++] = _vm->_lastUsedItem;
				break;
			case -10:
				_stack[_stackIndex++] = _vm->_items[_vm->_lastUsedItem].value;
				break;
			default:
				break;
			}
			break;

		case 12:
			a = 1;
			for (i = 0; i < 6; i++) {
				if (!(_vm->_characters[i].flags & 1))
					continue;
				if (_vm->_characters[i].effectFlags & 0x40)
					continue;
				a = 0;
				break;
			}
			_stack[_stackIndex++] = a;
			break;

		case 13:
			_stack[_stackIndex++] = _vm->rollDice(pos[0], pos[1], pos[2]);
			pos += 3;
			break;

		case 14:
			cmd = *pos++;
			a = _vm->rollDice(1, 6);
			b = 0;
			for (i = 0; i < 6 && b == 0; i++) {
				if (++a > 5)
					a = 0;
				if (_vm->testCharacter(a, 5)) {
					if (_vm->_classModifierFlags[_vm->_characters[a].cClass] & cmd) {
						_activeCharacter = a;
						b = 1;
					}
				}
			}
			_stack[_stackIndex++] = b;
			break;

		case 15:
			cmd = *pos++;
			a = _vm->rollDice(1, 6);
			b = 0;
			for (i = 0; i < 6; i++) {
				if (++a > 5)
					a = 0;
				if (_vm->testCharacter(a, 5)) {
					if ((_vm->_characters[a].raceSex >> 1) == cmd) {
						_activeCharacter = a;
						b = 1;
					}
				}
			}
			_stack[_stackIndex++] = b;
			break;

		case 17:
			_stack[_stackIndex++] = _vm->_activeSpell;
			break;

		case 18:
			_stack[_stackIndex++] = _lastScriptFlags;
			break;

		case 22:
			_stack[_stackIndex++] = _dlgResult;
			break;

		case 25:
			itm = &_vm->_items[_vm->_itemInHand];

			switch (*pos++) {
			case -49:
				a = *pos++;
				tempString1 = _vm->_itemNames[itm->nameId];
				tempString1.toUppercase();
				tempString2 = (const char *)pos;
				tempString2.toUppercase();
				pos += a;
				_stack[_stackIndex++] = tempString1.contains(tempString2) ? 1 : 0;
				break;

			case -48:
				a = *pos++;
				tempString1 = _vm->_itemNames[itm->nameUnid];
				tempString1.toUppercase();
				tempString2 = (const char *)pos;
				tempString2.toUppercase();
				pos += a;
				_stack[_stackIndex++] = tempString1.contains(tempString2) ? 1 : 0;
				break;

			case -31:
				_stack[_stackIndex++] = itm->type;
				break;

			case -11:
				_stack[_stackIndex++] = _vm->_itemInHand;
				break;

			case -10:
				_stack[_stackIndex++] = itm->value;
				break;

			default:
				break;
			}

			break;

		case 26:
			a = 0;
			for (i = 0; i < 6; i++) {
				if (_vm->testCharacter(i, 0x0f))
					a++;
			}
			_stack[_stackIndex++] = a;
			break;

		case 27:
			_stack[_stackIndex++] = _vm->_levelBlockProperties[READ_LE_UINT16(pos + 1)].walls[pos[0]];
			pos += 3;
			break;

		case 31:
			_stack[_stackIndex++] = _vm->_currentDirection;
			break;

		case 33:
			_stack[_stackIndex++] = (_flagTable[_vm->_currentLevel] & (1 << (*pos++))) ? 1 : 0;
			break;

		case 34:
			_stack[_stackIndex++] = (_flagTable[17] & (1 << (*pos++))) ? 1 : 0;
			break;

		case 35:
			if (*pos++ == -11) {
				a = (int16)READ_LE_UINT16(pos);
				pos += 2;
				b = (int16)READ_LE_UINT16(pos);
				pos += 2;
				_stack[_stackIndex++] = _vm->countCharactersWithSpecificItems(a, b);
			} else {
				_stack[_stackIndex++] = (_vm->_currentBlock == READ_LE_UINT16(pos)) ? 1 : 0;
				pos += 2;
			}
			break;

		case 36:
			a = (int16)READ_LE_UINT16(pos);
			pos += 2;
			b = READ_LE_UINT16(pos);
			pos += 2;
			_stack[_stackIndex++] = _vm->countQueuedItems(_vm->_levelBlockProperties[b].drawObjects, a, -1, 0, 0);
			break;

		case 37:
			if (*pos++ == -1) {
				_stack[_stackIndex++] = _vm->_levelBlockProperties[READ_LE_UINT16(pos)].flags & 7;
				pos += 2;
			} else {
				do {
					a += _vm->countSpecificMonsters(*pos++);
				} while (*pos != -1);
				pos++;
				_stack[_stackIndex++] = a;
			}
			break;

		case 39:
			a = *pos++;
			b = *pos++;
			i = READ_LE_UINT16(pos);
			pos += 2;
			_stack[_stackIndex++] = _vm->countQueuedItems(_vm->_levelBlockProperties[i].drawObjects, -1, b, 1, a);
			break;

		case 41:
			_stack[_stackIndex++] = _vm->_levelBlockProperties[READ_LE_UINT16(pos)].walls[0];
			pos += 2;
			break;

		case 42:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a || b) ? 1 : 0;
			break;

		case 43:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a && b) ? 1 : 0;
			break;

		case 44:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a <= b) ? 1 : 0;
			break;

		case 45:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a < b) ? 1 : 0;
			break;

		case 46:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a >= b) ? 1 : 0;
			break;

		case 47:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a > b) ? 1 : 0;
			break;

		case 48:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a != b) ? 1 : 0;
			break;

		case 49:
			a = _stack[--_stackIndex];
			b = _stack[--_stackIndex];
			_stack[_stackIndex++] = (a == b) ? 1 : 0;
			break;

		default:
			break;
		}
		cmd = *pos++;
	}

	cmd = _stack[--_stackIndex];
	if (cmd)
		pos += 2;
	else
		pos = _scriptData + READ_LE_UINT16(pos);

	return pos - data;
}

int EoBInfProcessor::oeob_deleteItem(int8 *data) {
	int8 *pos = data;
	int8 c = *pos++;

	if (c == -1) {
		_vm->deleteInventoryItem(0, -1);
		debugC(5, kDebugLevelScript, "         - delete hand item");
	} else {
		_vm->deleteBlockItem(READ_LE_UINT16(pos), (c == -2) ? -1 : c);
		debugC(5, kDebugLevelScript, "         - delete item(s) of type '%d' on block '0x%.04X'", (c == -2) ? -1 : c, READ_LE_UINT16(pos));
		pos += 2;
	}

	return pos - data;
}

int EoBInfProcessor::oeob_loadNewLevelOrMonsters(int8 *data) {
	int8 *pos = data;
	_vm->gui_updateControls();

	int8 cmd = *pos++;
	int8 index = *pos++;
	int res = 0;

	if (cmd == -27 || _vm->game() == GI_EOB1) {
		cmd = _vm->game() == GI_EOB2 ? *pos++ : 0;
		_vm->_currentBlock = READ_LE_UINT16(pos);
		pos += 2;
		uint8 dir = (uint8)*pos++;

		if (dir != 0xff)
			_vm->_currentDirection = dir;

		for (int i = 0; i < 30; i++)
			_vm->_monsters[i].curAttackFrame = 0;

		for (int i = 0; i < 10; i++) {
			EoBFlyingObject *fo = &_vm->_flyingObjects[i];
			if (fo->enable == 1) {
				_vm->_items[fo->item].pos &= 3;
				run(_vm->_items[fo->item].block, 4);
			}
			fo->enable = 0;
		}

		_vm->completeDoorOperations();

		_vm->generateTempData();
		_vm->txt()->removePageBreakFlag();
		_screen->setScreenDim(7);

		_vm->loadLevel(index, cmd);
		debugC(5, kDebugLevelScript, "         - entering level '%d', sub level '%d', start block '0x%.04X', start direction '%d'", index, cmd, _vm->_currentBlock, _vm->_currentDirection);

		if (_vm->_dialogueField)
			_vm->restoreAfterDialogueSequence();

		_vm->moveParty(_vm->_currentBlock);

		_abortScript = 1;
		_abortAfterSubroutine = 1;
		_vm->_sceneUpdateRequired = true;

		_vm->gui_drawAllCharPortraitsWithStats();
		_subroutineStackPos = 0;

	} else {
		cmd = *pos++;
		_vm->releaseMonsterShapes(cmd * 18, 18);
		_vm->loadMonsterShapes((const char *)pos, cmd * 18, true, index * 18);
		debugC(5, kDebugLevelScript, "         - loading monster shapes '%s', monster number '%d', encode type '%d'", (const char *)pos, cmd, index);
		pos += 13;
		_vm->gui_restorePlayField();
		res = pos - data;
	}

	return res;
}

int EoBInfProcessor::oeob_increasePartyExperience(int8 *data) {
	int8 *pos = data;
	if (*pos++ == -30) {
		_vm->increasePartyExperience((int16)READ_LE_UINT16(pos));
		debugC(5, kDebugLevelScript, "         - award '%d' experience points", READ_LE_UINT16(pos));
		pos += 2;
	}
	return pos - data;
}

int EoBInfProcessor::oeob_createItem_v1(int8 *data) {
	int8 *pos = data;
	uint16 itm = _vm->duplicateItem(READ_LE_UINT16(pos));
	pos += 2;
	uint16 block = READ_LE_UINT16(pos);
	pos += 2;
	uint8 itmPos = *pos++;

	if (itm) {
		if (block == 0xffff && !_vm->_itemInHand) {
			_vm->setHandItem(itm);
			debugC(5, kDebugLevelScript, "         - create hand item '%d'", itm);
		} else if (block != 0xffff) {
			_vm->setItemPosition((Item *)&_vm->_levelBlockProperties[block & 0x3ff].drawObjects, block, itm, itmPos);
			debugC(5, kDebugLevelScript, "         - create item '%d' on block '0x%.04X', position '%d'", itm, block, itmPos);
		}
	}

	return pos - data;
}

int EoBInfProcessor::oeob_createItem_v2(int8 *data) {
	static const uint8 _itemPos[] = { 0, 1, 2, 3, 1, 3, 0, 2, 3, 2, 1, 0, 2, 0, 3, 1 };
	int8 *pos = data;

	uint16 itm = _vm->duplicateItem(READ_LE_UINT16(pos));
	pos += 2;
	uint16 block = READ_LE_UINT16(pos);
	pos += 2;
	uint8 itmPos = *pos++;
	uint8 flg = *pos++;

	if (flg & 1)
		_vm->_items[itm].value = *pos++;

	if (flg & 2)
		_vm->_items[itm].flags = *pos++;

	if (flg & 4)
		_vm->_items[itm].icon = *pos++;

	if (!itm)
		return pos - data;

	if (block == 0xffff) {
		if (!_vm->_itemInHand) {
			_vm->setHandItem(itm);
			debugC(5, kDebugLevelScript, "         - create hand item '%d' (value '%d', flags '0x%X', icon number '%d')", itm, _vm->_items[itm].value, _vm->_items[itm].flags, _vm->_items[itm].icon);
		} else {
			_vm->setItemPosition((Item *)&_vm->_levelBlockProperties[_vm->_currentBlock & 0x3ff].drawObjects, _vm->_currentBlock, itm, _itemPos[_vm->rollDice(1, 2, -1)]);
			debugC(5, kDebugLevelScript, "         - create item '%d' (value '%d', flags '0x%X', icon number '%d') on current block", itm, _vm->_items[itm].value, _vm->_items[itm].flags, _vm->_items[itm].icon);
		}
	} else if (block == 0xfffe) {
		_vm->setItemPosition((Item *)&_vm->_levelBlockProperties[_vm->_currentBlock & 0x3ff].drawObjects, _vm->_currentBlock, itm, _itemPos[(_vm->_currentDirection << 2) + _vm->rollDice(1, 2, -1)]);
		debugC(5, kDebugLevelScript, "         - create item '%d' (value '%d', flags '0x%X', icon number '%d') on current block", itm, _vm->_items[itm].value, _vm->_items[itm].flags, _vm->_items[itm].icon);
	} else {
		_vm->setItemPosition((Item *)&_vm->_levelBlockProperties[block & 0x3ff].drawObjects, block, itm, itmPos);
		debugC(5, kDebugLevelScript, "         - create item '%d' (value '%d', flags '0x%X', icon number '%d') on block '0x%.04X', position '%d'", itm, _vm->_items[itm].value, _vm->_items[itm].flags, _vm->_items[itm].icon, block, itmPos);
	}

	return pos - data;
}

int EoBInfProcessor::oeob_launchObject(int8 *data) {
	static const uint8 startPos[] = { 2, 3, 0, 2, 1, 0, 3, 1 };

	int8 *pos = data;
	bool m = (*pos++ == -33);
	int i = READ_LE_UINT16(pos);
	pos += 2;
	uint16 block = READ_LE_UINT16(pos);
	pos += 2;
	int dir = *pos++;
	int dirOffs =  *pos++;

	if (m) {
		uint8 openBookType = _vm->_openBookType;
		_vm->_openBookType = 0;
		_vm->launchMagicObject(-1, i, block, startPos[dir * 2 + dirOffs], dir);
		_vm->_openBookType = openBookType;
	} else {
		Item itm = _vm->duplicateItem(i);
		if (itm) {
			if (!_vm->launchObject(-1, itm, block, startPos[dir * 2 + dirOffs], dir, _vm->_items[itm].type))
				_vm->_items[itm].block = -1;
		}
	}

	return pos - data;
}

int EoBInfProcessor::oeob_changeDirection(int8 *data) {
	int8 *pos = data;

	int8 cmd = *pos++;
	int8 dir = *pos++;

	if (cmd == -15) {
		_vm->_currentDirection = (_vm->_currentDirection + dir) & 3;
		//_vm->_keybControlUnk = -1;
		_vm->_sceneUpdateRequired = true;

	} else if (cmd == -11) {
		for (int i = 0; i < 10; i++) {
			if (_vm->_flyingObjects[i].enable)
				_vm->_flyingObjects[i].direction = (_vm->_flyingObjects[i].direction + dir) & 3;
		}
	}

	return pos - data;
}

int EoBInfProcessor::oeob_identifyItems(int8 *data) {
	int8 *pos = data;
	uint16 block = READ_LE_UINT16(pos);

	if (block == _vm->_currentBlock) {
		for (int i = 0; i < 6; i++) {
			if (!(_vm->_characters[i].flags & 1))
				continue;

			for (int ii = 0; ii < 27; ii++) {
				int inv = _vm->_characters[i].inventory[ii];
				if (inv)
					_vm->_items[inv].flags |= 0x40;
			}

			_vm->identifyQueuedItems(_vm->_characters[i].inventory[16]);
		}
	}

	_vm->identifyQueuedItems(_vm->_levelBlockProperties[block].drawObjects);
	return pos - data;
}

int EoBInfProcessor::oeob_sequence(int8 *data) {
	int8 *pos = data;
	_vm->_npcSequenceSub = -1;
	_vm->txt()->setWaitButtonMode(0);
	_vm->gui_updateControls();
	_vm->drawScene(1);

	int cmd = *pos++;

	if (_vm->game() == GI_EOB1) {
		if (cmd == 10)
			cmd = -1;
		else if (cmd == 9)
			cmd = -3;
		else if (cmd == 8)
			cmd = -2;
	}

	switch (cmd) {
	case -3:
		_vm->seq_xdeath();
		_vm->_runFlag = false;
		_vm->_playFinale = true;
		_abortScript = 1;
		return 0;

	case -2:
		_vm->seq_portal();
		break;

	case -1:
		_vm->_runFlag = _vm->checkPassword();
		break;

	default:
		_vm->npcSequence(cmd);
		break;
	}
	_vm->screen()->setScreenDim(7);
	return pos - data;
}

int EoBInfProcessor::oeob_delay(int8 *data) {
	int8 *pos = data;
	_vm->delay(READ_LE_UINT16(pos) * _vm->tickLength());
	pos += 2;
	return pos - data;
}

int EoBInfProcessor::oeob_drawScene(int8 *data) {
	_vm->drawScene(1);
	return 0;
}

int EoBInfProcessor::oeob_dialogue(int8 *data) {
	int8 *pos = data;

	switch (*pos++) {
	case -45:
		_vm->drawSequenceBitmap((const char *)pos, pos[13], READ_LE_UINT16(pos + 14), READ_LE_UINT16(pos + 16), READ_LE_UINT16(pos + 18));
		pos += 20;
		break;

	case -44:
		_vm->restoreAfterDialogueSequence();
		break;

	case -43:
		_vm->initDialogueSequence();
		break;

	case -42:
		_vm->gui_drawDialogueBox();
		break;

	case -40:
		_dlgResult = _vm->runDialogue(READ_LE_UINT16(pos), READ_LE_UINT16(pos + 6) == 0xffff ? 2 : 3, getString(READ_LE_UINT16(pos + 2)), getString(READ_LE_UINT16(pos + 4)), getString(READ_LE_UINT16(pos + 6)));
		pos += 8;
		break;

	case -8:
		_vm->txt()->printDialogueText(READ_LE_UINT16(pos), getString(READ_LE_UINT16(pos + 2)));
		pos += 4;
		break;

	default:
		break;
	}

	return pos - data;
}

int EoBInfProcessor::oeob_specialEvent(int8 *data) {
	int8 *pos = data;
	uint16 cmd = READ_LE_UINT16(pos);
	pos += 2;

	uint32 endTime = 0;
	int i = 0;

	switch (cmd) {
	case 0:
		_vm->drawScene(1);
		_screen->_curPage = 2;
		_screen->copyRegion(72, 0, 0, 0, 32, 120, 2, 12, Screen::CR_NO_P_CHECK);

		for (; i < 4; i++) {
			endTime = _vm->_system->getMillis() + _vm->_tickLength;
			_vm->drawLightningColumn();
			_screen->copyRegion(72, 0, 72, 0, 32, 120, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
			_screen->copyRegion(0, 0, 72, 0, 32, 120, 12, 2, Screen::CR_NO_P_CHECK);
			_vm->delayUntil(endTime);
		}

		_screen->_curPage = 0;
		_vm->_sceneUpdateRequired = true;
		break;

	case 1:
		_dlgResult = _vm->charSelectDialogue();
		break;

	case 2:
		_vm->characterLevelGain(_dlgResult);
		break;

	case 3:
		_dlgResult = _vm->resurrectionSelectDialogue();
		break;

	case 4:
		if (_vm->prepareForNewPartyMember(33, 5))
			_vm->initNpc(4);
		break;

	case 5:
		_vm->deletePartyItems(46, 5);
		_vm->deletePartyItems(46, 6);
		break;

	case 6:
		_vm->loadVcnData(0, 0);
		break;

	default:
		break;
	}

	return pos - data;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
