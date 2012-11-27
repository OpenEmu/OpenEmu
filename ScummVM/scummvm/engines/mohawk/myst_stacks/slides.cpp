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
#include "mohawk/myst_areas.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/slides.h"

#include "common/system.h"
#include "gui/message.h"

namespace Mohawk {
namespace MystStacks {

Slides::Slides(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();
	_vm->_cursor->hideCursor();
}

Slides::~Slides() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &Slides::x, #x))

void Slides::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, o_returnToMenu);

	// "Init" Opcodes
	OPCODE(200, o_setCardSwap);
}

#undef OPCODE

void Slides::disablePersistentScripts() {
	_cardSwapEnabled = false;
}

void Slides::runPersistentScripts() {
	if (_cardSwapEnabled) {
		// Used on Cards...
		if (_vm->_system->getMillis() > _nextCardTime) {
			_vm->_gfx->fadeToBlack();
			_vm->changeToCard(_nextCardID, false);
			_vm->_gfx->fadeFromBlack();
		}
	}
}

void Slides::o_returnToMenu(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Return to menu", op);

	// Go to the information screens of the menu
	_vm->changeToStack(kDemoStack, 2002, 0, 0);
}

void Slides::o_setCardSwap(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_nextCardID = argv[0];

	debugC(kDebugScript, "Opcode %d: Set next card %d", op, _nextCardID);

	_nextCardTime = _vm->_system->getMillis() + 5000;
	_cardSwapEnabled = true;
}

} // End of namespace MystStacks
} // End of namespace Mohawk
