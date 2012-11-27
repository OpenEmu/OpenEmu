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

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_stacks/demo.h"

#include "common/system.h"

namespace Mohawk {
namespace MystStacks {

Demo::Demo(MohawkEngine_Myst *vm) : Intro(vm) {
	setupOpcodes();

	_returnToMenuStep = 0;
}

Demo::~Demo() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &Demo::x, #x))

#define OVERRIDE_OPCODE(opcode, x) \
	for (uint32 i = 0; i < _opcodes.size(); i++) \
		if (_opcodes[i]->op == opcode) { \
			_opcodes[i]->proc = (OpcodeProcMyst) &Demo::x; \
			_opcodes[i]->desc = #x; \
			break; \
		}

void Demo::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OVERRIDE_OPCODE(100, o_stopIntro);
	OPCODE(101, o_fadeFromBlack);
	OPCODE(102, o_fadeToBlack);

	// "Init" Opcodes
	OVERRIDE_OPCODE(201, o_returnToMenu_init);
}

#undef OPCODE
#undef OVERRIDE_OPCODE

void Demo::disablePersistentScripts() {
	Intro::disablePersistentScripts();

	_returnToMenuRunning = false;
}

void Demo::runPersistentScripts() {
	Intro::runPersistentScripts();

	if (_returnToMenuRunning) {
		returnToMenu_run();
	}
}

void Demo::o_stopIntro(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Unk", op);
	// The original also seems to stop the movies. Not needed with this engine.
	_vm->_gfx->fadeToBlack();
}

void Demo::o_fadeFromBlack(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Fade from black", op);

	// FIXME: This glitches when enabled. The backbuffer is drawn to screen,
	// and then the fading occurs, causing the background to appear for one frame.
	// _vm->_gfx->fadeFromBlack();
}

void Demo::o_fadeToBlack(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Fade to black", op);
	_vm->_gfx->fadeToBlack();
}

void Demo::returnToMenu_run() {
	uint32 time = _vm->_system->getMillis();

	if (time < _returnToMenuNextTime)
		return;

	switch (_returnToMenuStep){
	case 0:
		_vm->_gfx->fadeToBlack();
		_vm->changeToCard(2003, false);
		_vm->_gfx->fadeFromBlack();

		_returnToMenuStep++;
		break;
	case 1:
		_vm->_gfx->fadeToBlack();
		_vm->changeToCard(2001, false);
		_vm->_gfx->fadeFromBlack();
		_vm->_cursor->showCursor();

		_returnToMenuStep++;
		break;
	default:
		break;
	}
}

void Demo::o_returnToMenu_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Return to menu init", op);

	// Used on Card 2001, 2002 and 2003
	_returnToMenuNextTime = _vm->_system->getMillis() + 5000;
	_returnToMenuRunning = true;
}

} // End of namespace MystStacks
} // End of namespace Mohawk
