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
#include "mohawk/myst_stacks/preview.h"

#include "common/system.h"
#include "gui/message.h"

namespace Mohawk {
namespace MystStacks {

Preview::Preview(MohawkEngine_Myst *vm) : Myst(vm) {
	setupOpcodes();
	_vm->_cursor->hideCursor();
}

Preview::~Preview() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &Preview::x, #x))

#define OVERRIDE_OPCODE(opcode, x) \
	for (uint32 i = 0; i < _opcodes.size(); i++) \
		if (_opcodes[i]->op == opcode) { \
			_opcodes[i]->proc = (OpcodeProcMyst) &Preview::x; \
			_opcodes[i]->desc = #x; \
			break; \
		}

void Preview::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OVERRIDE_OPCODE(196, o_fadeToBlack);
	OVERRIDE_OPCODE(197, o_fadeFromBlack);
	OVERRIDE_OPCODE(198, o_stayHere);
	OVERRIDE_OPCODE(199, o_speechStop);

	// "Init" Opcodes
	OPCODE(298, o_speech_init);
	OPCODE(299, o_library_init);
}

#undef OPCODE
#undef OVERRIDE_OPCODE

void Preview::disablePersistentScripts() {
	Myst::disablePersistentScripts();
}

void Preview::runPersistentScripts() {
	Myst::runPersistentScripts();

	if (_speechRunning)
		speech_run();
}

void Preview::o_fadeToBlack(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Fade to black", op);
	_vm->_gfx->fadeToBlack();
}

void Preview::o_fadeFromBlack(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Fade from black", op);

	// FIXME: This glitches when enabled. The backbuffer is drawn to screen,
	// and then the fading occurs, causing the background to appear for one frame.
	// _vm->_gfx->fadeFromBlack();
}

void Preview::o_stayHere(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Stay here dialog", op);

	// Nuh-uh! No leaving the library in the demo!
	GUI::MessageDialog dialog("You can't leave the library in the demo.");
	dialog.runModal();
}

void Preview::o_speechStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Speech stop", op);

	_vm->_sound->stopSound(3001);
	_speechRunning = false;
	_globals.currentAge = 2;
}

void Preview::speechUpdateCue() {
	// This is a callback in the original, handling audio events.
	if (!_vm->_sound->isPlaying(3001)) {
		return;
	}

	uint samples = _vm->_sound->getNumSamplesPlayed(3001);
	for (int16 i = 0; i < _cueList.pointCount; i++) {
		if (_cueList.points[i].sampleFrame > samples)
			return;
		if (i > _currentCue - 1) {
			_currentCue++;
			debugC(kDebugScript, "Sneak speech advanced to cue %d", _currentCue);
		}
	}
}

void Preview::speech_run() {
	uint32 time = _vm->_system->getMillis();

	// Update current speech sound cue
	speechUpdateCue();

	switch (_speechStep) {
	case 0: // Start Voice Over... which controls book opening
		_currentCue = 0;
		_vm->_sound->playSound(3001, Audio::Mixer::kMaxChannelVolume, false, &_cueList);

		_speechStep++;
		break;
	case 1: // Open book
		if (_currentCue >= 1) {
			_vm->changeToCard(3001, true);

			_speechStep++;
		}
		break;
	case 2: // Go to Myst
		if (_currentCue >= 2) {
			_vm->_gfx->fadeToBlack();
			_vm->changeToCard(3002, false);
			_vm->_gfx->fadeFromBlack();

			_speechStep++;
		}
		break;
	case 3: // Start blinking the library
		if (_currentCue >= 3) {
			_libraryState = 1;
			_speechNextTime = 0;
			_speechStep++;
		}
		break;
	case 4: // Library blinking, zoom in library
		if (_currentCue >= 4) {
			_library->drawConditionalDataToScreen(0);

			_vm->changeToCard(3003, true);

			_speechNextTime = time + 2000;
			_speechStep++;
		} else {
			if (time < _speechNextTime)
				break;

			_library->drawConditionalDataToScreen(_libraryState);
			_libraryState = (_libraryState + 1) % 2;
			_speechNextTime = time + 500;
		}
		break;
	case 5: // Go to library near view
		if (time < _speechNextTime)
			break;

		_vm->changeToCard(3004, true);
		_speechNextTime = time + 2000;
		_speechStep++;
		break;
	case 6: // Fade to courtyard
		if (time < _speechNextTime)
			break;

		_vm->_gfx->fadeToBlack();
		_vm->changeToCard(3005, false);
		_vm->_gfx->fadeFromBlack();
		_speechNextTime = time + 1000;
		_speechStep++;
		break;
	case 7: // Walk to library
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
		if (time < _speechNextTime)
			break;

		_vm->changeToCard(3006 + _speechStep - 7, true);
		_speechNextTime = time + 2000;
		_speechStep++;
		break;
	case 14: // Go to playable library card
		if (time < _speechNextTime)
			break;

		_vm->changeToCard(4329, true);

		_speechRunning = false;
		_globals.currentAge = 2;

		_vm->_cursor->showCursor();
		break;
	default:
		warning("Unknown speech step");
		break;
	}
}

void Preview::o_speech_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Speech init", op);

	// Used for Card 3000 (Closed Myst Book)
	_speechStep = 0;
	_speechRunning = true;
}

void Preview::o_library_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Library init", op);

	// Used for Card 3002 (Myst Island Overview)
	_library = static_cast<MystResourceType8 *>(_invokingResource);
}

} // End of namespace MystStacks
} // End of namespace Mohawk
