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

#include "mohawk/myst.h"
#include "mohawk/cursors.h"
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/dni.h"

#include "common/system.h"

namespace Mohawk {
namespace MystStacks {

Dni::Dni(MohawkEngine_Myst *vm) :
		MystScriptParser(vm) {
	setupOpcodes();
	_notSeenAtrus = true;
}

Dni::~Dni() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &Dni::x, #x))

void Dni::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, NOP);
	OPCODE(101, o_handPage);

	// "Init" Opcodes
	OPCODE(200, o_atrus_init);

	// "Exit" Opcodes
	OPCODE(300, NOP);
}

#undef OPCODE

void Dni::disablePersistentScripts() {
	_atrusRunning = false;
	_waitForLoop = false;
	_atrusLeft = false;
}

void Dni::runPersistentScripts() {
	if (_atrusRunning)
		atrus_run();

	if (_waitForLoop)
		loopVideo_run();

	if (_atrusLeft)
		atrusLeft_run();
}

uint16 Dni::getVar(uint16 var) {
	switch(var) {
	case 0: // Atrus Gone (from across room)
		return _globals.ending == 2;
	case 1: // Myst Book Status
		if (_globals.ending != 4)
			return _globals.ending == 3;
		else
			return 2; // Linkable
	case 2: // Music Type
		if (_notSeenAtrus) {
			_notSeenAtrus = false;
			return _globals.ending != 4 && _globals.heldPage != 13;
		} else
			return 2;
	default:
		return MystScriptParser::getVar(var);
	}
}

void Dni::o_handPage(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Hand page to Atrus", op);
	// Used in Card 5014 (Atrus)

	// Find Atrus movie
	VideoHandle atrus = _vm->_video->findVideoHandle(_video);

	// Good ending and Atrus asked to give page
	if (_globals.ending == 1 && _vm->_video->getTime(atrus) > (uint)Audio::Timestamp(0, 6801, 600).msecs()) {
		_globals.ending = 2;
		_globals.heldPage = 0;
		_vm->setMainCursor(kDefaultMystCursor);

		// Play movie end (atrus leaving)
		_vm->_video->setVideoBounds(atrus, Audio::Timestamp(0, 14813, 600), _vm->_video->getDuration(atrus));
		_vm->_video->setVideoLooping(atrus, false);

		_atrusLeft = true;
		_waitForLoop = false;
		_atrusLeftTime = _vm->_system->getMillis();
	}
}

void Dni::atrusLeft_run() {
	if (_vm->_system->getMillis() > _atrusLeftTime + 63333) {
		_video = _vm->wrapMovieFilename("atrus2", kDniStack);
		VideoHandle atrus = _vm->_video->playMovie(_video, 215, 77);
		_vm->_video->setVideoBounds(atrus, Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 98000, 600));

		_waitForLoop = true;
		_loopStart = 73095;
		_loopEnd = 98000;

		// Good ending
		_globals.ending = 4;
		_globals.bluePagesInBook = 63;
		_globals.redPagesInBook = 63;

		_atrusLeft = false;
	}
}

void Dni::loopVideo_run() {
	if (!_vm->_video->isVideoPlaying()) {
		VideoHandle atrus = _vm->_video->playMovie(_video, 215, 77);
		_vm->_video->setVideoBounds(atrus, Audio::Timestamp(0, _loopStart, 600), Audio::Timestamp(0, _loopEnd, 600));
		_vm->_video->setVideoLooping(atrus, true);

		_waitForLoop = false;
	}
}

void Dni::atrus_run() {
	if (_globals.ending == 2) {
		// Wait for atrus to come back
		_atrusLeft = true;
	} else if (_globals.ending == 1) {
		// Atrus asking for page
		if (!_vm->_video->isVideoPlaying()) {
			_video = _vm->wrapMovieFilename("atr1page", kDniStack);
			VideoHandle atrus = _vm->_video->playMovie(_video, 215, 77, true);
			_vm->_video->setVideoBounds(atrus, Audio::Timestamp(0, 7388, 600), Audio::Timestamp(0, 14700, 600));
		}
	} else if (_globals.ending != 3 && _globals.ending != 4) {
		if (_globals.heldPage == 13) {
			_video = _vm->wrapMovieFilename("atr1page", kDniStack);
			VideoHandle atrus = _vm->_video->playMovie(_video, 215, 77);
			_vm->_video->setVideoBounds(atrus, Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 14700, 600));

			_waitForLoop = true;
			_loopStart = 7388;
			_loopEnd = 14700;

			// Wait for page
			_globals.ending = 1;

		} else {
			_video = _vm->wrapMovieFilename("atr1nopg", kDniStack);
			VideoHandle atrus = _vm->_video->playMovie(_video, 215, 77);
			_vm->_video->setVideoBounds(atrus, Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 46175, 600));

			_waitForLoop = true;
			_loopStart = 30656;
			_loopEnd = 46175;

			// Bad ending
			_globals.ending = 3;
		}
	} else if (!_vm->_video->isVideoPlaying()) {
		_vm->_video->playMovie(_vm->wrapMovieFilename("atrwrite", kDniStack), 215, 77, true);
	}
}

void Dni::o_atrus_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Atrus init", op);

	_atrusRunning = true;
}

} // End of namespace MystStacks
} // End of namespace Mohawk
