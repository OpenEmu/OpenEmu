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

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/hotspots.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODEVER Inter_LittleRed
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_LittleRed::Inter_LittleRed(GobEngine *vm) : Inter_v2(vm) {
}

void Inter_LittleRed::setupOpcodesDraw() {
	Inter_v2::setupOpcodesDraw();
}

void Inter_LittleRed::setupOpcodesFunc() {
	Inter_v2::setupOpcodesFunc();

	OPCODEFUNC(0x14, oLittleRed_keyFunc);

	OPCODEFUNC(0x3D, oLittleRed_playComposition);
}

void Inter_LittleRed::setupOpcodesGob() {
	OPCODEGOB(1, o_gobNOP); // Sets some sound timer interrupt
	OPCODEGOB(2, o_gobNOP); // Sets some sound timer interrupt

	OPCODEGOB(500, o2_playProtracker);
	OPCODEGOB(501, o2_stopProtracker);
}

void Inter_LittleRed::oLittleRed_keyFunc(OpFuncParams &params) {
	animPalette();
	_vm->_draw->blitInvalidated();

	handleBusyWait();

	int16 cmd = _vm->_game->_script->readInt16();
	int16 key;
	uint32 keyState;

	switch (cmd) {
	case -1:
		break;

	case 0:
		_vm->_draw->_showCursor &= ~2;
		_vm->_util->longDelay(1);
		key = _vm->_game->_hotspots->check(0, 0);
		storeKey(key);

		_vm->_util->clearKeyBuf();
		break;

	case 1:
		_vm->_util->forceMouseUp(true);
		key = _vm->_game->checkKeys(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons, 0);
		storeKey(key);
		break;

	case 2:
		_vm->_util->processInput(true);
		keyState = _vm->_util->getKeyState();

		WRITE_VAR(0, keyState);
		_vm->_util->clearKeyBuf();
		break;

	default:
		_vm->_sound->speakerOnUpdate(cmd);
		if (cmd < 20) {
			_vm->_util->delay(cmd);
			_noBusyWait = true;
		} else
			_vm->_util->longDelay(cmd);
		break;
	}
}

void Inter_LittleRed::oLittleRed_playComposition(OpFuncParams &params) {
	o1_playComposition(params);

	_vm->_sound->blasterRepeatComposition(-1);
}

} // End of namespace Gob
