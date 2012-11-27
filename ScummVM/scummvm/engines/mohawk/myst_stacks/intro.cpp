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
#include "mohawk/myst_areas.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_state.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/intro.h"

#include "gui/message.h"

namespace Mohawk {
namespace MystStacks {

Intro::Intro(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();
}

Intro::~Intro() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &Intro::x, #x))

void Intro::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, o_useLinkBook);

	// "Init" Opcodes
	OPCODE(200, o_playIntroMovies);
	OPCODE(201, o_mystLinkBook_init);

	// "Exit" Opcodes
	OPCODE(300, NOP);
}

#undef OPCODE

void Intro::disablePersistentScripts() {
	_introMoviesRunning = false;
	_linkBookRunning = false;
}

void Intro::runPersistentScripts() {
	if (_introMoviesRunning)
		introMovies_run();

	if (_linkBookRunning)
		mystLinkBook_run();
}

uint16 Intro::getVar(uint16 var) {
	switch(var) {
	case 0:
		if (_globals.currentAge == 9 || _globals.currentAge == 10)
			return 2;
		else
			return _globals.currentAge;
	default:
		return MystScriptParser::getVar(var);
	}
}

void Intro::o_useLinkBook(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Hard coded SoundId valid only for Intro Stack.
	// Other stacks use Opcode 40, which takes SoundId values as arguments.
	const uint16 soundIdLinkSrc = 5;
	const uint16 soundIdLinkDst[] = { 2282, 3029, 6396, 7122, 3137, 0, 9038, 5134, 0, 4739, 4741 };

	debugC(kDebugScript, "Opcode %d: o_useLinkBook", op);
	debugC(kDebugScript, "\tvar: %d", var);

	// Change to dest stack
	_vm->changeToStack(_stackMap[_globals.currentAge], _startCard[_globals.currentAge], soundIdLinkSrc, soundIdLinkDst[_globals.currentAge]);
}

void Intro::introMovies_run() {
	// Play Intro Movies
	// This is all quite messy...

	switch (_introStep) {
	case 0:
		_introStep = 1;
		_vm->_video->playMovie(_vm->wrapMovieFilename("broder", kIntroStack));
		break;
	case 1:
		if (!_vm->_video->isVideoPlaying())
			_introStep = 2;
		break;
	case 2:
		_introStep = 3;
		_vm->_video->playMovie(_vm->wrapMovieFilename("cyanlogo", kIntroStack));
		break;
	case 3:
		if (!_vm->_video->isVideoPlaying())
			_introStep = 4;
		break;
	case 4:
		_introStep = 5;

		if (!(_vm->getFeatures() & GF_DEMO)) // The demo doesn't have the intro video
			_vm->_video->playMovie(_vm->wrapMovieFilename("intro", kIntroStack));
		break;
	case 5:
		if (!_vm->_video->isVideoPlaying())
			_introStep = 6;
		break;
	default:
		if (_vm->getFeatures() & GF_DEMO)
			_vm->changeToCard(2001, true);
		else
			_vm->changeToCard(2, true);
	}
}

void Intro::o_playIntroMovies(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_introMoviesRunning = true;
	_introStep = 0;
}

void Intro::mystLinkBook_run() {
	if (_startTime == 1) {
		_startTime = 0;

		if (!_vm->skippableWait(5000)) {
			_linkBookMovie->playMovie();
			_vm->_gfx->copyImageToBackBuffer(4, Common::Rect(544, 333));
			_vm->_gfx->copyBackBufferToScreen(Common::Rect(544, 333));
		}
	} else if (!_linkBookMovie->isPlaying()) {
		_vm->changeToCard(5, true);
	}
}

void Intro::o_mystLinkBook_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Myst link book init", op);

	_linkBookMovie = static_cast<MystResourceType6 *>(_invokingResource);
	_startTime = 1;
	_linkBookRunning = true;
}

} // End of namespace MystStacks
} // End of namespace Mohawk
