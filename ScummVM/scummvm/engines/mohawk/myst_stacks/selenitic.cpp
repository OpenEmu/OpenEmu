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
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_state.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/selenitic.h"

#include "common/system.h"
#include "common/textconsole.h"
#include "gui/message.h"

namespace Mohawk {
namespace MystStacks {

Selenitic::Selenitic(MohawkEngine_Myst *vm) :
		MystScriptParser(vm), _state(vm->_gameState->_selenitic) {
	setupOpcodes();
	_invokingResource = NULL;
	_mazeRunnerPosition = 288;
	_mazeRunnerDirection = 8;
}

Selenitic::~Selenitic() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &Selenitic::x, #x))

void Selenitic::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, o_mazeRunnerMove);
	OPCODE(101, o_mazeRunnerSoundRepeat);
	OPCODE(102, o_soundReceiverSigma);
	OPCODE(103, o_soundReceiverRight);
	OPCODE(104, o_soundReceiverLeft);
	OPCODE(105, o_soundReceiverSource);
	OPCODE(106, o_soundReceiverSource);
	OPCODE(107, o_soundReceiverSource);
	OPCODE(108, o_soundReceiverSource);
	OPCODE(109, o_soundReceiverSource);
	OPCODE(110, o_mazeRunnerDoorButton);
	OPCODE(111, o_soundReceiverUpdateSound);
	OPCODE(112, o_soundLockMove);
	OPCODE(113, o_soundLockStartMove);
	OPCODE(114, o_soundLockEndMove);
	OPCODE(115, o_soundLockButton);
	OPCODE(116, NOP);
	OPCODE(117, o_soundReceiverEndMove);

	// "Init" Opcodes
	OPCODE(200, o_mazeRunnerCompass_init);
	OPCODE(201, o_mazeRunnerWindow_init);
	OPCODE(202, o_mazeRunnerLight_init);
	OPCODE(203, o_soundReceiver_init);
	OPCODE(204, o_soundLock_init);
	OPCODE(205, o_mazeRunnerRight_init);
	OPCODE(206, o_mazeRunnerLeft_init);
}

#undef OPCODE

void Selenitic::disablePersistentScripts() {
	_soundReceiverRunning = false;
}

void Selenitic::runPersistentScripts() {
	if (_soundReceiverRunning)
		soundReceiver_run();
}

uint16 Selenitic::getVar(uint16 var) {
	switch(var) {
	case 0: // Sound receiver emitters enabled
		return _state.emitterEnabledWind;
	case 1:
		return _state.emitterEnabledVolcano;
	case 2:
		return _state.emitterEnabledClock;
	case 3:
		return _state.emitterEnabledWater;
	case 4:
		return _state.emitterEnabledCrystal;
	case 5: // Sound receiver opened
		return _state.soundReceiverOpened;
	case 6: // Tunnel lights
		return _state.tunnelLightsSwitchedOn;
	case 7:// Maze runner display
		if (_mazeRunnerPosition == 288)
			return 0;
		else if (_mazeRunnerPosition == 289)
			return 1;
		else if (!mazeRunnerForwardAllowed(_mazeRunnerPosition))
			return 2;
		else
			return 3;
	case 8: // Viewer
		return 0;
	case 9: // Sound receiver selected source
		return _state.soundReceiverCurrentSource == 0;
	case 10:
		return _state.soundReceiverCurrentSource == 1;
	case 11:
		return _state.soundReceiverCurrentSource == 2;
	case 12:
		return _state.soundReceiverCurrentSource == 3;
	case 13:
		return _state.soundReceiverCurrentSource == 4;
	case 14: // Sound receiver position
		return (*_soundReceiverPosition) / 1000;
	case 15:
		return ((*_soundReceiverPosition) / 100) % 10;
	case 16:
		return ((*_soundReceiverPosition) / 10) % 10;
	case 17:
		return (*_soundReceiverPosition) % 10;
	case 20: // Sound lock sliders state
	case 21:
	case 22:
	case 23:
	case 24:
		return 1;
	case 25: // Maze runner direction
		return _mazeRunnerDirection;
	case 26:
		return _soundReceiverSigmaPressed;
	case 27:
	case 29: // Maze runner door
		return 0;
	case 30:
		return _mazeRunnerDoorOpened;
	case 31: // Maze runner forward allowed
		return mazeRunnerForwardAllowed(_mazeRunnerPosition) || _mazeRunnerPosition == 288;
	case 32: // Maze runner exit allowed
		return _mazeRunnerPosition != 288 && _mazeRunnerPosition != 289;
	case 33: // Maze runner at entry
		return _mazeRunnerPosition != 288;
	case 102: // Red page
		return !(_globals.redPagesInBook & 2) && (_globals.heldPage != 8);
	case 103: // Blue page
		return !(_globals.bluePagesInBook & 2) && (_globals.heldPage != 2);
	default:
		return MystScriptParser::getVar(var);
	}
}

void Selenitic::toggleVar(uint16 var) {
	switch(var) {
	case 0: // Sound receiver emitters enabled
		_state.emitterEnabledWind = (_state.emitterEnabledWind + 1) % 2;
		break;
	case 1:
		_state.emitterEnabledVolcano = (_state.emitterEnabledVolcano + 1) % 2;
		break;
	case 2:
		_state.emitterEnabledClock = (_state.emitterEnabledClock + 1) % 2;
		break;
	case 3:
		_state.emitterEnabledWater = (_state.emitterEnabledWater + 1) % 2;
		break;
	case 4:
		_state.emitterEnabledCrystal = (_state.emitterEnabledCrystal + 1) % 2;
		break;
	case 5: // Sound receiver opened
		_state.soundReceiverOpened = (_state.soundReceiverOpened + 1) % 2;
		break;
	case 6: // Tunnel lights
		_state.tunnelLightsSwitchedOn = (_state.tunnelLightsSwitchedOn + 1) % 2;
		break;
	case 102: // Red page
		if (!(_globals.redPagesInBook & 2)) {
			if (_globals.heldPage == 8)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 8;
		}
		break;
	case 103: // Blue page
		if (!(_globals.bluePagesInBook & 2)) {
			if (_globals.heldPage == 2)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 2;
		}
		break;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool Selenitic::setVarValue(uint16 var, uint16 value) {
	bool refresh = false;

	switch (var) {
	case 0: // Sound receiver emitters enabled
		if (_state.emitterEnabledWind != value) {
			_state.emitterEnabledWind = value;
			refresh = true;
		}
		break;
	case 1:
		if (_state.emitterEnabledVolcano != value) {
			_state.emitterEnabledVolcano = value;
			refresh = true;
		}
		break;
	case 2:
		if (_state.emitterEnabledClock != value) {
			_state.emitterEnabledClock = value;
			refresh = true;
		}
		break;
	case 3:
		if (_state.emitterEnabledWater != value) {
			_state.emitterEnabledWater = value;
			refresh = true;
		}
		break;
	case 4:
		if (_state.emitterEnabledCrystal != value) {
			_state.emitterEnabledCrystal = value;
			refresh = true;
		}
		break;
	case 5: // Sound receiver opened
		if (_state.soundReceiverOpened != value) {
			_state.soundReceiverOpened = value;
			refresh = true;
		}
		break;
	case 6: // Tunnel lights
		if (_state.tunnelLightsSwitchedOn != value) {
			_state.tunnelLightsSwitchedOn = value;
			refresh = true;
		}
		break;
	case 20: // Sound lock sliders
		_state.soundLockSliderPositions[0] = value;
		break;
	case 21:
		_state.soundLockSliderPositions[1] = value;
		break;
	case 22:
		_state.soundLockSliderPositions[2] = value;
		break;
	case 23:
		_state.soundLockSliderPositions[3] = value;
		break;
	case 24:
		_state.soundLockSliderPositions[4] = value;
		break;
	case 30:
		_mazeRunnerDoorOpened = value;
		break;
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

void Selenitic::o_mazeRunnerMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 oldPosition = _mazeRunnerPosition;
	uint16 move = var;

	uint16 videoToNext = _mazeRunnerVideos[_mazeRunnerPosition][move];
	_mazeRunnerPosition = _mazeRunnerMap[_mazeRunnerPosition][move];

	if (videoToNext) {
		_mazeRunnerCompass->drawConditionalDataToScreen(8);

		if (move == 3)
			mazeRunnerBacktrack(oldPosition);

		mazeRunnerPlayVideo(videoToNext, oldPosition);
		mazeRunnerUpdateCompass();

		if (move == 0 || move == 3)
			mazeRunnerPlaySoundHelp();
	}
}

void Selenitic::mazeRunnerBacktrack(uint16 &oldPosition) {
	if (oldPosition == 289)
		_mazeRunnerDirection = 3;

	uint16 targetDirection = _mazeRunnerPosition % 8;

	if (_mazeRunnerPosition == 289)
		targetDirection = 3;
	else if (_mazeRunnerPosition == 288)
		targetDirection = 0;
	else if (_mazeRunnerPosition == 252)
		targetDirection = 6;
	else if (_mazeRunnerPosition == 212)
		targetDirection = 2;
	else if (_mazeRunnerPosition == 171)
		targetDirection = 7;
	else if (_mazeRunnerPosition == 150)
		targetDirection = 4;
	else if (_mazeRunnerPosition == 116)
		targetDirection = 2;

	uint16 moves = 0;
	if (targetDirection >= _mazeRunnerDirection)
		moves = targetDirection - _mazeRunnerDirection;
	else
		moves = targetDirection + 8 - _mazeRunnerDirection;

	bool goLeft = false;
	if (moves > 4)
		goLeft = true;

	while (targetDirection != _mazeRunnerDirection) {
		_mazeRunnerCompass->drawConditionalDataToScreen(8);

		if (goLeft) {
			_mazeRunnerLeftButton->drawConditionalDataToScreen(2);

			uint16 video = _mazeRunnerVideos[oldPosition][1];
			oldPosition = _mazeRunnerMap[oldPosition][1];
			_mazeRunnerDirection = (_mazeRunnerDirection + 7) % 8;

			mazeRunnerPlayVideo(video, oldPosition);

			_mazeRunnerLeftButton->drawConditionalDataToScreen(1);
		} else {
			_mazeRunnerRightButton->drawConditionalDataToScreen(2);

			uint16 video = _mazeRunnerVideos[oldPosition][2];
			oldPosition = _mazeRunnerMap[oldPosition][2];
			_mazeRunnerDirection = (_mazeRunnerDirection + 1) % 8;

			mazeRunnerPlayVideo(video, oldPosition);

			_mazeRunnerRightButton->drawConditionalDataToScreen(1);
		}

		_mazeRunnerCompass->drawConditionalDataToScreen(_mazeRunnerDirection);
		_vm->_system->delayMillis(150);
	}
}

void Selenitic::mazeRunnerPlayVideo(uint16 video, uint16 pos) {
	Common::String file;

	switch (video) {
	case 1:
		file = _vm->wrapMovieFilename("forwa1", kSeleniticStack);
		break;
	case 2:
		file = _vm->wrapMovieFilename("forwe0", kSeleniticStack);
		break;
	case 3:
		if (mazeRunnerForwardAllowed(_mazeRunnerPosition))
			file = _vm->wrapMovieFilename("forwf1", kSeleniticStack);
		else
			file = _vm->wrapMovieFilename("forwf0", kSeleniticStack);
		break;
	case 4:
		file = _vm->wrapMovieFilename("left00", kSeleniticStack);
		break;
	case 5:
		file = _vm->wrapMovieFilename("left01", kSeleniticStack);
		break;
	case 6:
		file = _vm->wrapMovieFilename("left10", kSeleniticStack);
		break;
	case 7:
		file = _vm->wrapMovieFilename("left11", kSeleniticStack);
		break;
	case 8:
		file = _vm->wrapMovieFilename("right00", kSeleniticStack);
		break;
	case 9:
		file = _vm->wrapMovieFilename("right01", kSeleniticStack);
		break;
	case 10:
		file = _vm->wrapMovieFilename("right10", kSeleniticStack);
		break;
	case 11:
		file = _vm->wrapMovieFilename("right11", kSeleniticStack);
		break;
	case 12:
		if (mazeRunnerForwardAllowed(_mazeRunnerPosition))
			file = _vm->wrapMovieFilename("forwo1", kSeleniticStack);
		else
			file = _vm->wrapMovieFilename("forwo0", kSeleniticStack);
		break;
	case 13:
		if (mazeRunnerForwardAllowed(_mazeRunnerPosition))
			file = _vm->wrapMovieFilename("forwp1", kSeleniticStack);
		else
			file = _vm->wrapMovieFilename("forwp0", kSeleniticStack);
		break;
	case 14:
		if (mazeRunnerForwardAllowed(_mazeRunnerPosition))
			file = _vm->wrapMovieFilename("forws1", kSeleniticStack);
		else
			file = _vm->wrapMovieFilename("forws0", kSeleniticStack);
		break;
	case 15:
		if (mazeRunnerForwardAllowed(_mazeRunnerPosition))
			file = _vm->wrapMovieFilename("forwr1", kSeleniticStack);
		else
			file = _vm->wrapMovieFilename("forwr0", kSeleniticStack);
		break;
	case 16:
		if (mazeRunnerForwardAllowed(_mazeRunnerPosition))
			file = _vm->wrapMovieFilename("forwl1", kSeleniticStack);
		else
			file = _vm->wrapMovieFilename("forwl0", kSeleniticStack);
		break;
	case 17:
		file = _vm->wrapMovieFilename("backa1", kSeleniticStack);
		break;
	case 18:
		file = _vm->wrapMovieFilename("backe1", kSeleniticStack);
		break;
	case 19:
		if (mazeRunnerForwardAllowed(pos))
			file = _vm->wrapMovieFilename("backf1", kSeleniticStack);
		else
			file = _vm->wrapMovieFilename("backf0", kSeleniticStack);
		break;
	case 20:
		if (mazeRunnerForwardAllowed(pos))
			file = _vm->wrapMovieFilename("backo1", kSeleniticStack);
		else
			file = _vm->wrapMovieFilename("backo0", kSeleniticStack);
		break;
	case 21:
		if (mazeRunnerForwardAllowed(pos))
			file = _vm->wrapMovieFilename("backp1", kSeleniticStack);
		else
			file = _vm->wrapMovieFilename("backp0", kSeleniticStack);
		break;
	case 22:
		if (mazeRunnerForwardAllowed(pos)) {
			file = _vm->wrapMovieFilename("backs1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("backs0", kSeleniticStack);
		}
		break;
	case 23:
		if (mazeRunnerForwardAllowed(pos))
			file = _vm->wrapMovieFilename("backr1", kSeleniticStack);
		else
			file = _vm->wrapMovieFilename("backr0", kSeleniticStack);
		break;
	case 24:
		if (mazeRunnerForwardAllowed(pos))
			file = _vm->wrapMovieFilename("backl1", kSeleniticStack);
		else
			file = _vm->wrapMovieFilename("backl0", kSeleniticStack);
		break;
	}

	if (!file.empty()) {
		const Common::Rect &dest = _mazeRunnerWindow->getRect();
		_vm->_video->playMovieBlocking(file, dest.left, dest.top, false);
	}
}

void Selenitic::mazeRunnerUpdateCompass() {
	if (_mazeRunnerPosition == 288 || _mazeRunnerPosition == 289)
		_mazeRunnerDirection = 8;
	else
		_mazeRunnerDirection = _mazeRunnerPosition % 8;

	_mazeRunnerCompass->drawConditionalDataToScreen(_mazeRunnerDirection);
}

bool Selenitic::mazeRunnerForwardAllowed(uint16 position) {
	uint16 move = _mazeRunnerVideos[position][1];
	return move == 6 || move == 7;
}

void Selenitic::mazeRunnerPlaySoundHelp() {
	uint16 soundId = 0;

	_mazeRunnerLight->drawConditionalDataToScreen(1);

	if (_mazeRunnerPosition >= 272)
		soundId = 0;
	else if (_mazeRunnerPosition >= 264)
		soundId =  8191;
	else if (_mazeRunnerPosition >= 256)
		soundId = 0;
	else if (_mazeRunnerPosition >= 248)
		soundId =  5191;
	else if (_mazeRunnerPosition >= 232)
		soundId = 0;
	else if (_mazeRunnerPosition >= 224)
		soundId =  5191;
	else if (_mazeRunnerPosition >= 216)
		soundId = 0;
	else if (_mazeRunnerPosition >= 208)
		soundId =  5191;
	else if (_mazeRunnerPosition >= 176)
		soundId = 0;
	else if (_mazeRunnerPosition >= 168)
		soundId =  7191;
	else if (_mazeRunnerPosition >= 152)
		soundId = 0;
	else if (_mazeRunnerPosition >= 144)
		soundId =  7191;
	else if (_mazeRunnerPosition >= 136)
		soundId =  2191;
	else if (_mazeRunnerPosition >= 112)
		soundId = 0;
	else if (_mazeRunnerPosition >= 104)
		soundId =  6191;
	else if (_mazeRunnerPosition >= 96)
		soundId =  2191;
	else if (_mazeRunnerPosition >= 88)
		soundId =  3191;
	else if (_mazeRunnerPosition >= 80)
		soundId =  4191;
	else if (_mazeRunnerPosition >= 72)
		soundId =  8191;
	else if (_mazeRunnerPosition >= 64)
		soundId =  7191;
	else if (_mazeRunnerPosition >= 56)
		soundId =  8191;
	else if (_mazeRunnerPosition >= 40)
		soundId =  5191;
	else if (_mazeRunnerPosition >= 24)
		soundId =  1191;
	else if (_mazeRunnerPosition >= 16)
		soundId =  2191;
	else if (_mazeRunnerPosition >= 8)
		soundId =  8191;
	else
		soundId =  2191;

	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);

	_mazeRunnerLight->drawConditionalDataToScreen(0);
}

void Selenitic::o_mazeRunnerSoundRepeat(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	mazeRunnerPlaySoundHelp();
}

/**
 * Sound receiver sigma button
 */
void Selenitic::o_soundReceiverSigma(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound receiver sigma button", op);

	_vm->_cursor->hideCursor();

	_soundReceiverCurrentSource->drawConditionalDataToScreen(0);

	uint16 *oldPosition = _soundReceiverPosition;
	uint16 source = 0;

	for (uint i = 0; i < 5; i++) {
		switch (i) {
		case 0:
			source = 3;
			break;
		case 1:
			source = 0;
			break;
		case 2:
			source = 4;
			break;
		case 3:
			source = 1;
			break;
		case 4:
			source = 2;
			break;
		}

		_soundReceiverPosition = &_state.soundReceiverPositions[source];
		_vm->_sound->stopBackgroundMyst();
		_vm->_sound->replaceSoundMyst(2287);
		soundReceiverDrawView();
		uint16 soundId = soundReceiverCurrentSound(source, *_soundReceiverPosition);
		_vm->_sound->replaceBackgroundMyst(soundId);
		_vm->_system->delayMillis(1000);
	}

	_soundReceiverPosition = oldPosition;
	_soundReceiverSigmaPressed = true;
	_vm->_sound->stopBackgroundMyst();

	_soundReceiverSources[_state.soundReceiverCurrentSource]->drawConditionalDataToScreen(1);

	soundReceiverDrawView();

	_vm->_cursor->showCursor();
}

/**
 * Sound receiver right button
 */
void Selenitic::o_soundReceiverRight(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound receiver right", op);

	soundReceiverLeftRight(1);
}

/**
 * Sound receiver left button
 */
void Selenitic::o_soundReceiverLeft(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound receiver left", op);

	soundReceiverLeftRight(2);
}

void Selenitic::soundReceiverLeftRight(uint direction) {

	if (_soundReceiverSigmaPressed) {
		_soundReceiverSigmaButton->drawConditionalDataToScreen(0);
		_soundReceiverSigmaPressed = false;
	}

	if (direction == 1)
		_soundReceiverRightButton->drawConditionalDataToScreen(1);
	else
		_soundReceiverLeftButton->drawConditionalDataToScreen(1);

	_vm->_sound->stopSound();

	_soundReceiverDirection = direction;
	_soundReceiverSpeed = 1;
	_soundReceiverStartTime = _vm->_system->getMillis();

	soundReceiverUpdate();
}

void Selenitic::soundReceiverUpdate() {
	if (_soundReceiverDirection == 1)
		*_soundReceiverPosition = ((*_soundReceiverPosition) + _soundReceiverSpeed) % 3600;
	else if (_soundReceiverDirection == 2)
		*_soundReceiverPosition = ((*_soundReceiverPosition) + 3600 - _soundReceiverSpeed) % 3600;

	soundReceiverDrawView();
}

void Selenitic::soundReceiverDrawView() {
	uint32 left = ((*_soundReceiverPosition) * 1800) / 3600;

	_soundReceiverViewer->_subImages->rect.left = left;
	_soundReceiverViewer->_subImages->rect.right = left + 136;

	_soundReceiverViewer->drawConditionalDataToScreen(0);

	soundReceiverDrawAngle();
}

void Selenitic::soundReceiverDrawAngle() {
	_vm->redrawResource(_soundReceiverAngle1);
	_vm->redrawResource(_soundReceiverAngle2);
	_vm->redrawResource(_soundReceiverAngle3);
	_vm->redrawResource(_soundReceiverAngle4);
}

/**
 * Sound receiver source selection buttons
 */
void Selenitic::o_soundReceiverSource(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound receiver source", op);

	if (_soundReceiverSigmaPressed) {
		_soundReceiverSigmaButton->drawConditionalDataToScreen(0);
		_soundReceiverSigmaPressed = false;
	}

	_vm->_cursor->hideCursor();

	uint pressedButton = var - 9;

	if (_state.soundReceiverCurrentSource != pressedButton) {
		_state.soundReceiverCurrentSource = pressedButton;

		_soundReceiverCurrentSource->drawConditionalDataToScreen(0);

		_soundReceiverPosition = &_state.soundReceiverPositions[pressedButton];
		_soundReceiverCurrentSource = _soundReceiverSources[pressedButton];

		_vm->_sound->stopSound();

		uint16 soundId = argv[0];
		_vm->_sound->stopBackgroundMyst();
		_vm->_sound->replaceSoundMyst(soundId);

		_soundReceiverCurrentSource->drawConditionalDataToScreen(1);

		soundReceiverDrawView();
	}

	_vm->_cursor->showCursor();
}

void Selenitic::o_mazeRunnerDoorButton(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Selenitic Maze Runner Exit Logic
	uint16 cardIdExit = argv[0];
	uint16 cardIdEntry = argv[1];

	if (_mazeRunnerPosition == 288) {
		_vm->changeToCard(cardIdEntry, false);
		_vm->_sound->replaceSoundMyst(cardIdEntry);
		animatedUpdate(argv[2], &argv[3], 10);
	} else if (_mazeRunnerPosition == 289) {
		_vm->changeToCard(cardIdExit, false);
		_vm->_sound->replaceSoundMyst(cardIdExit);
		animatedUpdate(argv[2], &argv[3], 10);
	}
}

void Selenitic::o_soundReceiverUpdateSound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound receiver update sound", op);

	soundReceiverUpdateSound();
}

uint16 Selenitic::soundLockCurrentSound(uint16 position, bool pixels) {
	if ((pixels && position < 96) || (!pixels && position == 0))
		return 289;
	else if ((pixels && position < 108) || (!pixels && position == 1))
		return 1289;
	else if ((pixels && position < 120) || (!pixels && position == 2))
		return 2289;
	else if ((pixels && position < 132) || (!pixels && position == 3))
		return 3289;
	else if ((pixels && position < 144) || (!pixels && position == 4))
		return 4289;
	else if ((pixels && position < 156) || (!pixels && position == 5))
		return 5289;
	else if ((pixels && position < 168) || (!pixels && position == 6))
		return 6289;
	else if ((pixels && position < 180) || (!pixels && position == 7))
		return 7289;
	else if ((pixels && position < 192) || (!pixels && position == 8))
		return 8289;
	else if (pixels || (!pixels && position == 9))
		return 9289;

	return 0;
}

MystResourceType10 *Selenitic::soundLockSliderFromVar(uint16 var) {
	switch (var) {
	case 20:
		return _soundLockSlider1;
	case 21:
		return _soundLockSlider2;
	case 22:
		return _soundLockSlider3;
	case 23:
		return _soundLockSlider4;
	case 24:
		return _soundLockSlider5;
	}

	return 0;
}

void Selenitic::o_soundLockMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound lock move", op);

	MystResourceType10 *slider = soundLockSliderFromVar(var);

	uint16 soundId = soundLockCurrentSound(slider->_pos.y, true);
	if (_soundLockSoundId != soundId) {
		_soundLockSoundId = soundId;
		_vm->_sound->replaceSoundMyst(soundId, Audio::Mixer::kMaxChannelVolume, true);
	}
}

void Selenitic::o_soundLockStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound lock start move", op);

	MystResourceType10 *slider = soundLockSliderFromVar(var);

	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackgroundMyst();

	_soundLockSoundId = soundLockCurrentSound(slider->_pos.y, true);
	_vm->_sound->replaceSoundMyst(_soundLockSoundId, Audio::Mixer::kMaxChannelVolume, true);
}

void Selenitic::o_soundLockEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound lock end move", op);

	MystResourceType10 *slider = soundLockSliderFromVar(var);
	uint16 *value = &_state.soundLockSliderPositions[0];

	switch (var) {
	case 20: // Sound lock sliders
		value = &_state.soundLockSliderPositions[0];
		break;
	case 21:
		value = &_state.soundLockSliderPositions[1];
		break;
	case 22:
		value = &_state.soundLockSliderPositions[2];
		break;
	case 23:
		value = &_state.soundLockSliderPositions[3];
		break;
	case 24:
		value = &_state.soundLockSliderPositions[4];
		break;
	default:
		error("Incorrect var value for Selenitic opcode 114");
		break;
	}

	uint16 stepped = 12 * (*value / 12) + 6;

	if (stepped == 6)
		stepped = 0;
	if (stepped == 114)
		stepped = 119;

	*value = stepped;

	slider->setStep(stepped);
	slider->restoreBackground();
	slider->drawConditionalDataToScreen(1);

	uint16 soundId = slider->getList3(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);

	_vm->_sound->resumeBackgroundMyst();
}

void Selenitic::soundLockCheckSolution(MystResourceType10 *slider, uint16 value, uint16 solution, bool &solved) {
	slider->drawConditionalDataToScreen(2);
	_vm->_sound->replaceSoundMyst(soundLockCurrentSound(value / 12, false));
	_vm->_system->delayMillis(1500);

	if (value / 12 != solution)
		solved = false;

	slider->drawConditionalDataToScreen(1);
	_vm->_sound->stopSound();
}

void Selenitic::o_soundLockButton(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound lock button", op);

	bool solved = true;

	_vm->_sound->pauseBackgroundMyst();
	_vm->_sound->replaceSoundMyst(1147);
	_soundLockButton->drawConditionalDataToScreen(1);
	_vm->_cursor->hideCursor();

	soundLockCheckSolution(_soundLockSlider1, _state.soundLockSliderPositions[0], 5, solved);
	soundLockCheckSolution(_soundLockSlider2, _state.soundLockSliderPositions[1], 9, solved);
	soundLockCheckSolution(_soundLockSlider3, _state.soundLockSliderPositions[2], 0, solved);
	soundLockCheckSolution(_soundLockSlider4, _state.soundLockSliderPositions[3], 6, solved);
	soundLockCheckSolution(_soundLockSlider5, _state.soundLockSliderPositions[4], 7, solved);

	_vm->_sound->replaceSoundMyst(1148);
	_vm->_sound->resumeBackgroundMyst();

	if (solved) {
		_soundLockButton->drawConditionalDataToScreen(2);

		uint16 cardIdClosed = argv[0];
		uint16 cardIdOpen = argv[1];

		_vm->changeToCard(cardIdClosed, true);

		_vm->changeToCard(cardIdOpen, false);
		_vm->_sound->replaceSoundMyst(argv[2]);

		animatedUpdate(argv[4], &argv[5], argv[3]);
	} else {
		_soundLockButton->drawConditionalDataToScreen(0);
	}

	_vm->_cursor->showCursor();
}

void Selenitic::o_soundReceiverEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound receiver end move", op);

	uint16 oldDirection = _soundReceiverDirection;

	if (_soundReceiverDirection) {
		_soundReceiverDirection = 0;

		soundReceiverUpdateSound();

		if (oldDirection == 1)
			_soundReceiverRightButton->drawConditionalDataToScreen(0);
		else
			_soundReceiverLeftButton->drawConditionalDataToScreen(0);
	}
}

void Selenitic::o_mazeRunnerCompass_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_mazeRunnerCompass = static_cast<MystResourceType8 *>(_invokingResource);
}

void Selenitic::o_mazeRunnerWindow_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_mazeRunnerWindow = static_cast<MystResourceType8 *>(_invokingResource);
}

void Selenitic::o_mazeRunnerLight_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_mazeRunnerLight = static_cast<MystResourceType8 *>(_invokingResource);
}

void Selenitic::soundReceiver_run() {
	if (_soundReceiverStartTime) {
		if (_soundReceiverDirection) {
			uint32 currentTime = _vm->_system->getMillis();

			if (_soundReceiverSpeed == 50 && currentTime > _soundReceiverStartTime + 500)
					soundReceiverIncreaseSpeed();
			else if (currentTime > _soundReceiverStartTime + 1000)
					soundReceiverIncreaseSpeed();

			if (currentTime > _soundReceiverStartTime + 100)
				soundReceiverUpdate();
		} else if (!_soundReceiverSigmaPressed) {
			soundReceiverUpdateSound();
		}
	}
}

void Selenitic::soundReceiverIncreaseSpeed() {
	switch (_soundReceiverSpeed) {
	case 1:
		_soundReceiverSpeed = 10;
		break;
	case 10:
		_soundReceiverSpeed = 50;
		break;
	case 50:
		_soundReceiverSpeed = 100;
		break;
	}
}

void Selenitic::soundReceiverUpdateSound() {
	uint16 soundId = soundReceiverCurrentSound(_state.soundReceiverCurrentSource, *_soundReceiverPosition);
	_vm->_sound->replaceSoundMyst(soundId);
}

uint16 Selenitic::soundReceiverCurrentSound(uint16 source, uint16 position) {
	uint16 solution = 0;
	bool sourceEnabled = false;
	soundReceiverSolution(source, solution, sourceEnabled);

	uint16 soundIdGood = 0;
	uint16 soundIdNear = 0;
	uint16 soundId = 1245;

	switch (source) {
	case 0:
		soundIdNear = 3245;
		soundIdGood = 3093;
		break;
	case 1:
		soundIdNear = 5245;
		soundIdGood = 5093;
		break;
	case 2:
		soundIdNear = 6245;
		soundIdGood = 6093;
		break;
	case 3:
		soundIdNear = 2245;
		soundIdGood = 2093;
		break;
	case 4:
		soundIdNear = 4245;
		soundIdGood = 4093;
		break;
	default:
		error("MystScriptParser_Selenitic::soundReceiverCurrentSound(): Unknown source (%d)", source);
	}

	if (sourceEnabled) {
		if (position == solution) {
			soundId = soundIdGood;
		} else if (position > solution && position <= solution + 50) {
			_soundReceiverLeftButton->drawConditionalDataToScreen(2);
			_soundReceiverLeftButton->drawConditionalDataToScreen(0);
			soundId = soundIdNear;
		} else if (position < solution && position >= solution - 50) {
			_soundReceiverRightButton->drawConditionalDataToScreen(2);
			_soundReceiverRightButton->drawConditionalDataToScreen(0);
			soundId = soundIdNear;
		}
	}

	return soundId;
}

void Selenitic::soundReceiverSolution(uint16 source, uint16 &solution, bool &enabled) {
	switch (source) {
	case 0:
		enabled = _state.emitterEnabledWater;
		solution = 1534;
		break;
	case 1:
		enabled = _state.emitterEnabledVolcano;
		solution = 1303;
		break;
	case 2:
		enabled = _state.emitterEnabledClock;
		solution = 556;
		break;
	case 3:
		enabled = _state.emitterEnabledCrystal;
		solution = 150;
		break;
	case 4:
		enabled = _state.emitterEnabledWind;
		solution = 2122;
		break;
	}
}

void Selenitic::o_soundReceiver_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound receiver init", op);

	// Used for Card 1245 (Sound Receiver)
	_soundReceiverRunning = true;

	_soundReceiverRightButton = static_cast<MystResourceType8 *>(_vm->_resources[0]);
	_soundReceiverLeftButton = static_cast<MystResourceType8 *>(_vm->_resources[1]);
	_soundReceiverSigmaButton = static_cast<MystResourceType8 *>(_vm->_resources[2]);
	_soundReceiverSources[4] = static_cast<MystResourceType8 *>(_vm->_resources[3]);
	_soundReceiverSources[3] = static_cast<MystResourceType8 *>(_vm->_resources[4]);
	_soundReceiverSources[2] = static_cast<MystResourceType8 *>(_vm->_resources[5]);
	_soundReceiverSources[1] = static_cast<MystResourceType8 *>(_vm->_resources[6]);
	_soundReceiverSources[0] = static_cast<MystResourceType8 *>(_vm->_resources[7]);
	_soundReceiverViewer = static_cast<MystResourceType8 *>(_vm->_resources[8]);
	_soundReceiverAngle1 = static_cast<MystResourceType8 *>(_vm->_resources[10]);
	_soundReceiverAngle2 = static_cast<MystResourceType8 *>(_vm->_resources[11]);
	_soundReceiverAngle3 = static_cast<MystResourceType8 *>(_vm->_resources[12]);
	_soundReceiverAngle4 = static_cast<MystResourceType8 *>(_vm->_resources[13]);

	uint16 currentSource = _state.soundReceiverCurrentSource;
	_soundReceiverPosition = &_state.soundReceiverPositions[currentSource];
	_soundReceiverCurrentSource = _soundReceiverSources[currentSource];

	_soundReceiverSigmaPressed = false;
}

void Selenitic::o_soundLock_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound lock init", op);

	for (uint i = 0; i < _vm->_resources.size(); i++) {
		if (_vm->_resources[i]->type == kMystSlider) {
			switch (_vm->_resources[i]->getType8Var()) {
			case 20:
				_soundLockSlider1 = static_cast<MystResourceType10 *>(_vm->_resources[i]);
				_soundLockSlider1->setStep(_state.soundLockSliderPositions[0]);
				break;
			case 21:
				_soundLockSlider2 = static_cast<MystResourceType10 *>(_vm->_resources[i]);
				_soundLockSlider2->setStep(_state.soundLockSliderPositions[1]);
				break;
			case 22:
				_soundLockSlider3 = static_cast<MystResourceType10 *>(_vm->_resources[i]);
				_soundLockSlider3->setStep(_state.soundLockSliderPositions[2]);
				break;
			case 23:
				_soundLockSlider4 = static_cast<MystResourceType10 *>(_vm->_resources[i]);
				_soundLockSlider4->setStep(_state.soundLockSliderPositions[3]);
				break;
			case 24:
				_soundLockSlider5 = static_cast<MystResourceType10 *>(_vm->_resources[i]);
				_soundLockSlider5->setStep(_state.soundLockSliderPositions[4]);
				break;
			}
		} else if (_vm->_resources[i]->type == kMystConditionalImage && _vm->_resources[i]->getType8Var() == 28) {
			_soundLockButton = static_cast<MystResourceType8 *>(_vm->_resources[i]);
		}
	}

	_soundLockSoundId = 0;
}

void Selenitic::o_mazeRunnerRight_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_mazeRunnerRightButton = static_cast<MystResourceType8 *>(_invokingResource);
}

void Selenitic::o_mazeRunnerLeft_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_mazeRunnerLeftButton = static_cast<MystResourceType8 *>(_invokingResource);
}

const uint16 Selenitic::_mazeRunnerMap[300][4] = {
	{8, 7, 1, 288},
	{1, 0, 2, 288},
	{2, 1, 3, 288},
	{3, 2, 4, 288},
	{4, 3, 5, 288},
	{5, 4, 6, 288},
	{6, 5, 7, 288},
	{7, 6, 0, 288},
	{8, 15, 9, 0},
	{9, 8, 10, 0},
	{10, 9, 11, 0},
	{11, 10, 12, 0},
	{4, 11, 13, 0},
	{13, 12, 14, 0},
	{22, 13, 15, 0},
	{15, 14, 8, 0},
	{24, 23, 17, 14},
	{17, 16, 18, 14},
	{10, 17, 19, 14},
	{19, 18, 20, 14},
	{20, 19, 21, 14},
	{21, 20, 22, 14},
	{22, 21, 23, 14},
	{23, 22, 16, 14},
	{112, 31, 25, 16},
	{25, 24, 26, 16},
	{34, 25, 27, 16},
	{27, 26, 28, 16},
	{20, 27, 29, 16},
	{29, 28, 30, 16},
	{30, 29, 31, 16},
	{31, 30, 24, 16},
	{120, 39, 33, 26},
	{33, 32, 34, 26},
	{42, 33, 35, 26},
	{35, 34, 36, 26},
	{36, 35, 37, 26},
	{37, 36, 38, 26},
	{30, 37, 39, 26},
	{39, 38, 32, 26},
	{40, 47, 41, 34},
	{143, 40, 42, 34},
	{42, 41, 43, 34},
	{43, 42, 44, 34},
	{52, 43, 45, 34},
	{45, 44, 46, 34},
	{38, 45, 47, 34},
	{47, 46, 40, 34},
	{40, 55, 49, 44},
	{49, 48, 50, 44},
	{146, 49, 51, 44},
	{51, 50, 52, 44},
	{60, 51, 53, 44},
	{53, 52, 54, 44},
	{54, 53, 55, 44},
	{55, 54, 48, 44},
	{48, 63, 57, 52},
	{57, 56, 58, 52},
	{58, 57, 59, 52},
	{171, 58, 60, 52},
	{60, 59, 61, 52},
	{61, 60, 62, 52},
	{68, 61, 63, 52},
	{63, 62, 56, 52},
	{58, 71, 65, 62},
	{65, 64, 66, 62},
	{66, 65, 67, 62},
	{67, 66, 68, 62},
	{68, 67, 69, 62},
	{77, 68, 70, 62},
	{70, 69, 71, 62},
	{71, 70, 64, 62},
	{72, 79, 73, 69},
	{65, 72, 74, 69},
	{186, 73, 75, 69},
	{75, 74, 76, 69},
	{196, 75, 77, 69},
	{77, 76, 78, 69},
	{86, 77, 79, 69},
	{79, 78, 72, 69},
	{80, 87, 81, 78},
	{81, 80, 82, 78},
	{74, 81, 83, 78},
	{83, 82, 84, 78},
	{212, 83, 85, 78},
	{229, 84, 86, 78},
	{86, 85, 87, 78},
	{95, 86, 80, 78},
	{88, 95, 89, 87},
	{97, 88, 90, 87},
	{90, 89, 91, 87},
	{83, 90, 92, 87},
	{92, 91, 93, 87},
	{93, 92, 94, 87},
	{254, 93, 95, 87},
	{95, 94, 88, 87},
	{106, 103, 97, 89},
	{97, 96, 98, 89},
	{98, 97, 99, 89},
	{99, 98, 100, 89},
	{100, 99, 101, 89},
	{93, 100, 102, 89},
	{102, 101, 103, 89},
	{271, 102, 96, 89},
	{104, 111, 105, 96},
	{105, 104, 106, 96},
	{106, 105, 107, 96},
	{289, 106, 108, 96},
	{108, 107, 109, 96},
	{109, 108, 110, 96},
	{100, 109, 111, 96},
	{111, 110, 104, 96},
	{112, 119, 113, 24},
	{113, 112, 114, 24},
	{114, 113, 115, 24},
	{115, 114, 116, 24},
	{28, 115, 117, 24},
	{117, 116, 118, 24},
	{118, 117, 119, 24},
	{119, 118, 112, 24},
	{120, 127, 121, 32},
	{121, 120, 122, 32},
	{122, 121, 123, 32},
	{123, 122, 124, 32},
	{36, 123, 125, 32},
	{125, 124, 126, 32},
	{126, 125, 127, 32},
	{127, 126, 120, 32},
	{128, 135, 129, 136},
	{129, 128, 130, 136},
	{130, 129, 131, 136},
	{131, 130, 132, 136},
	{140, 131, 133, 136},
	{133, 132, 134, 136},
	{134, 133, 135, 136},
	{135, 134, 128, 136},
	{128, 143, 137, 41},
	{137, 136, 138, 41},
	{138, 137, 139, 41},
	{45, 138, 140, 41},
	{140, 139, 141, 41},
	{141, 140, 142, 41},
	{142, 141, 143, 41},
	{143, 142, 136, 41},
	{144, 151, 145, 50},
	{145, 144, 146, 50},
	{146, 145, 147, 50},
	{147, 146, 148, 50},
	{148, 147, 149, 50},
	{157, 148, 150, 50},
	{54, 149, 151, 50},
	{151, 150, 144, 50},
	{152, 159, 153, 149},
	{145, 152, 154, 149},
	{154, 153, 155, 149},
	{155, 154, 156, 149},
	{156, 155, 157, 149},
	{157, 156, 158, 149},
	{158, 157, 159, 149},
	{159, 158, 152, 149},
	{160, 167, 161, 168},
	{161, 160, 162, 168},
	{162, 161, 163, 168},
	{163, 162, 164, 168},
	{172, 163, 165, 168},
	{165, 164, 166, 168},
	{166, 165, 167, 168},
	{167, 166, 160, 168},
	{160, 175, 169, 59},
	{169, 168, 170, 59},
	{170, 169, 171, 59},
	{171, 170, 172, 59},
	{172, 171, 173, 59},
	{181, 172, 174, 59},
	{174, 173, 175, 59},
	{63, 174, 168, 59},
	{176, 183, 177, 173},
	{169, 176, 178, 173},
	{178, 177, 179, 173},
	{179, 178, 180, 173},
	{180, 179, 181, 173},
	{181, 180, 182, 173},
	{182, 181, 183, 173},
	{183, 182, 176, 173},
	{184, 191, 185, 74},
	{185, 184, 186, 74},
	{186, 185, 187, 74},
	{187, 186, 188, 74},
	{188, 187, 189, 74},
	{189, 188, 190, 74},
	{78, 189, 191, 74},
	{191, 190, 184, 74},
	{72, 199, 193, 76},
	{193, 192, 194, 76},
	{194, 193, 195, 76},
	{195, 194, 196, 76},
	{196, 195, 197, 76},
	{197, 196, 198, 76},
	{198, 197, 199, 76},
	{199, 198, 192, 76},
	{200, 207, 201, 212},
	{201, 200, 202, 212},
	{202, 201, 203, 212},
	{203, 202, 204, 212},
	{204, 203, 205, 212},
	{205, 204, 206, 212},
	{208, 205, 207, 212},
	{207, 206, 200, 212},
	{80, 215, 209, 84},
	{209, 208, 210, 84},
	{210, 209, 211, 84},
	{211, 210, 212, 84},
	{202, 211, 213, 84},
	{213, 212, 214, 84},
	{208, 213, 215, 84},
	{215, 214, 208, 84},
	{224, 223, 217, 228},
	{217, 216, 218, 228},
	{218, 217, 219, 228},
	{219, 218, 220, 228},
	{220, 219, 221, 228},
	{221, 220, 222, 228},
	{216, 221, 223, 228},
	{223, 222, 216, 228},
	{224, 231, 225, 85},
	{81, 224, 226, 85},
	{226, 225, 227, 85},
	{227, 226, 228, 85},
	{220, 227, 229, 85},
	{229, 228, 230, 85},
	{238, 229, 231, 85},
	{231, 230, 224, 85},
	{232, 239, 233, 230},
	{233, 232, 234, 230},
	{226, 233, 235, 230},
	{235, 234, 236, 230},
	{236, 235, 237, 230},
	{237, 236, 238, 230},
	{232, 237, 239, 230},
	{239, 238, 232, 230},
	{240, 247, 241, 252},
	{241, 240, 242, 252},
	{248, 241, 243, 252},
	{243, 242, 244, 252},
	{244, 243, 245, 252},
	{245, 244, 246, 252},
	{240, 245, 247, 252},
	{247, 246, 240, 252},
	{256, 255, 249, 94},
	{249, 248, 250, 94},
	{90, 249, 251, 94},
	{251, 250, 252, 94},
	{246, 251, 253, 94},
	{253, 252, 254, 94},
	{248, 253, 255, 94},
	{255, 254, 248, 94},
	{256, 263, 257, 248},
	{257, 256, 258, 248},
	{258, 257, 259, 248},
	{259, 258, 260, 248},
	{252, 259, 261, 248},
	{261, 260, 262, 248},
	{256, 261, 263, 248},
	{263, 262, 256, 248},
	{280, 271, 265, 103},
	{265, 264, 266, 103},
	{266, 265, 267, 103},
	{99, 266, 268, 103},
	{268, 267, 269, 103},
	{269, 268, 270, 103},
	{278, 269, 271, 103},
	{271, 270, 264, 103},
	{272, 279, 273, 270},
	{273, 272, 274, 270},
	{266, 273, 275, 270},
	{275, 274, 276, 270},
	{276, 275, 277, 270},
	{277, 276, 278, 270},
	{272, 277, 279, 270},
	{279, 278, 272, 270},
	{280, 287, 281, 264},
	{281, 280, 282, 264},
	{282, 281, 283, 264},
	{283, 282, 284, 264},
	{268, 283, 285, 264},
	{285, 284, 286, 264},
	{280, 285, 287, 264},
	{287, 286, 280, 264},
	{0, 288, 288, 288},
	{289, 289, 289, 107},
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0},
	{ 0, 0, 0, 0}
};

const uint8 Selenitic::_mazeRunnerVideos[300][4] = {
	{3, 6, 10, 17},
	{0, 5, 8, 17},
	{0, 4, 8, 17},
	{0, 4, 8, 17},
	{0, 4, 8, 17},
	{0, 4, 8, 17},
	{0, 4, 8, 17},
	{0, 4, 9, 17},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 8, 19},
	{12, 6, 10, 19},
	{0, 5, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 9, 19},
	{14, 6, 10, 20},
	{0, 5, 9, 20},
	{13, 6, 10, 20},
	{0, 5, 9, 20},
	{12, 6, 10, 20},
	{0, 5, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 9, 20},
	{12, 6, 10, 21},
	{0, 5, 9, 21},
	{3, 6, 10, 21},
	{0, 5, 8, 21},
	{12, 4, 8, 21},
	{0, 4, 9, 21},
	{13, 6, 10, 21},
	{0, 5, 9, 21},
	{0, 4, 9, 19},
	{16, 6, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 9, 19},
	{12, 6, 10, 19},
	{0, 5, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 8, 19},
	{3, 6, 10, 20},
	{0, 5, 9, 20},
	{13, 6, 10, 20},
	{0, 5, 9, 20},
	{14, 6, 10, 20},
	{0, 5, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 9, 20},
	{14, 6, 10, 22},
	{0, 5, 8, 22},
	{0, 4, 9, 22},
	{3, 6, 10, 22},
	{0, 5, 8, 22},
	{0, 4, 9, 22},
	{16, 6, 10, 22},
	{0, 5, 9, 22},
	{15, 6, 10, 24},
	{0, 5, 8, 24},
	{0, 4, 8, 24},
	{0, 4, 8, 24},
	{0, 4, 9, 24},
	{14, 6, 10, 24},
	{0, 5, 8, 24},
	{0, 4, 9, 24},
	{0, 4, 9, 22},
	{14, 6, 11, 22},
	{12, 7, 10, 22},
	{0, 5, 9, 22},
	{13, 6, 10, 22},
	{0, 5, 9, 22},
	{3, 6, 10, 22},
	{0, 5, 8, 22},
	{0, 5, 8, 19},
	{0, 4, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 9, 19},
	{14, 6, 11, 19},
	{3, 7, 10, 19},
	{0, 5, 9, 19},
	{14, 6, 10, 19},
	{0, 4, 9, 22},
	{3, 6, 10, 22},
	{0, 5, 9, 22},
	{14, 6, 10, 22},
	{0, 5, 8, 22},
	{0, 4, 9, 22},
	{3, 6, 10, 22},
	{0, 5, 8, 22},
	{15, 7, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 9, 19},
	{14, 6, 11, 19},
	{0, 4, 8, 23},
	{0, 4, 8, 23},
	{0, 4, 9, 23},
	{2, 6, 10, 23},
	{0, 5, 8, 23},
	{0, 4, 9, 23},
	{16, 6, 10, 23},
	{0, 5, 8, 23},
	{0, 4, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 9, 22},
	{14, 6, 10, 22},
	{0, 5, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 9, 20},
	{12, 6, 10, 20},
	{0, 5, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{3, 6, 10, 24},
	{0, 5, 8, 24},
	{0, 4, 9, 24},
	{15, 6, 10, 24},
	{0, 5, 8, 24},
	{0, 4, 8, 24},
	{0, 4, 8, 24},
	{0, 4, 9, 24},
	{0, 4, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 9, 21},
	{3, 6, 11, 21},
	{13, 7, 10, 21},
	{0, 5, 8, 21},
	{0, 4, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 9, 22},
	{14, 6, 10, 22},
	{0, 5, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 8, 22},
	{14, 7, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 9, 19},
	{3, 6, 11, 19},
	{0, 4, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 9, 20},
	{12, 6, 10, 20},
	{0, 5, 8, 20},
	{13, 6, 10, 21},
	{0, 5, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 9, 21},
	{0, 4, 8, 24},
	{0, 4, 8, 24},
	{0, 4, 8, 24},
	{0, 4, 8, 24},
	{0, 4, 8, 24},
	{0, 4, 9, 24},
	{15, 6, 10, 24},
	{0, 5, 8, 24},
	{14, 6, 10, 22},
	{0, 5, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 9, 22},
	{16, 6, 10, 22},
	{0, 5, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 9, 22},
	{12, 6, 10, 20},
	{0, 5, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 8, 20},
	{0, 4, 9, 20},
	{0, 4, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 9, 19},
	{12, 6, 10, 19},
	{0, 5, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 23},
	{0, 4, 9, 23},
	{16, 6, 10, 23},
	{0, 5, 8, 23},
	{0, 4, 8, 23},
	{0, 4, 8, 23},
	{0, 4, 8, 23},
	{0, 4, 8, 23},
	{14, 6, 10, 19},
	{0, 5, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 9, 19},
	{15, 6, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 9, 19},
	{0, 4, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 9, 22},
	{14, 6, 10, 22},
	{0, 5, 8, 22},
	{0, 4, 8, 22},
	{0, 4, 8, 22},
	{3, 6, 10, 22},
	{0, 5, 8, 22},
	{0, 4, 9, 22},
	{14, 6, 10, 22},
	{0, 5, 8, 22},
	{0, 4, 9, 22},
	{13, 6, 10, 22},
	{0, 5, 9, 22},
	{0, 4, 8, 21},
	{0, 4, 9, 21},
	{12, 6, 10, 21},
	{0, 5, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 8, 21},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 9, 19},
	{3, 6, 10, 19},
	{0, 5, 8, 19},
	{0, 4, 8, 19},
	{0, 4, 8, 19},
	{1, 0, 0, 0},
	{0, 0, 0, 18},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

} // End of namespace MystStacks
} // End of namespace Mohawk
