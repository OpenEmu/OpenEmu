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
#include "mohawk/myst_state.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/channelwood.h"

#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Mohawk {
namespace MystStacks {

Channelwood::Channelwood(MohawkEngine_Myst *vm) :
		MystScriptParser(vm), _state(vm->_gameState->_channelwood) {
	setupOpcodes();
}

Channelwood::~Channelwood() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &Channelwood::x, #x))

void Channelwood::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, o_bridgeToggle);
	OPCODE(101, o_pipeExtend);
	OPCODE(102, o_drawImageChangeCardAndVolume);
	OPCODE(104, o_waterTankValveOpen);
	OPCODE(105, o_leverStartMove);
	OPCODE(106, o_leverEndMove);
	OPCODE(107, o_leverMoveFail);
	OPCODE(108, o_leverMove);
	OPCODE(109, o_stairsDoorToggle);
	OPCODE(110, o_valveHandleMove1);
	OPCODE(111, o_valveHandleMoveStart1);
	OPCODE(112, o_valveHandleMoveStop);
	OPCODE(113, o_valveHandleMove2);
	OPCODE(114, o_valveHandleMoveStart2);
	OPCODE(115, o_valveHandleMove3);
	OPCODE(116, o_valveHandleMoveStart3);
	OPCODE(117, o_hologramMonitor);
	OPCODE(118, o_drawerOpen);
	OPCODE(119, o_hologramTemple);
	OPCODE(120, o_leverElev3StartMove);
	OPCODE(121, o_leverElev3EndMove);
	OPCODE(122, o_waterTankValveClose);
	OPCODE(123, o_executeMouseUp);
	OPCODE(124, o_leverEndMoveWithSound);
	OPCODE(125, o_pumpLeverMove);
	OPCODE(126, o_pumpLeverEndMove);
	OPCODE(127, o_elevatorMovies);
	OPCODE(128, o_leverEndMoveResumeBackground);
	OPCODE(129, o_soundReplace);

	// "Init" Opcodes
	OPCODE(201, o_lever_init);
	OPCODE(202, o_pipeValve_init);
	OPCODE(203, o_drawer_init);

	// "Exit" Opcodes
	OPCODE(300, NOP);
}

#undef OPCODE

void Channelwood::disablePersistentScripts() {

}

void Channelwood::runPersistentScripts() {

}

uint16 Channelwood::getVar(uint16 var) {
	switch(var) {
	case 1: // Water Pump Bridge Raised
		return _state.waterPumpBridgeState;
	case 2: // Lower Walkway to Upper Walkway Elevator Raised
		return _state.elevatorState;
	case 3: // Water Flowing To Lower Walkway To Upper Walkway Elevator
		return (_state.waterValveStates & 0xc0) == 0xc0;
	case 4: // Water Flowing To Book Room Elevator
		return ((_state.waterValveStates & 0xf8) == 0xb0 && _state.pipeState) ? 1 : 0;
	case 5: // Lower Walkway to Upper Walkway Spiral Stair Lower Door State
		return _state.stairsLowerDoorState;
	case 6: // Pipe Bridge Extended
		return _state.pipeState;
	case 7: // Water Flowing To Water Pump For Bridge
		return ((_state.waterValveStates & 0xe2) == 0x82 || (_state.waterValveStates & 0xf4) == 0xa0) ? 1 : 0;
	case 8: // Water Tank Valve
		return (_state.waterValveStates & 0x80) ? 1 : 0;
	case 9: // State of First Water Valve
		return (_state.waterValveStates & 0x40) ? 1 : 0;
	case 10: // State of Second Water Valve
		return (_state.waterValveStates & 0x20) ? 1 : 0;
	case 11: // State of Right Third Water Valve
		return (_state.waterValveStates & 0x10) ? 1 : 0;
	case 12: // State of Right-Right Fourth Water Valve
		return (_state.waterValveStates & 0x08) ? 1 : 0;
	case 13: // State of Right-Left Fourth Water Valve
		return (_state.waterValveStates & 0x04) ? 1 : 0;
	case 14: // State of Left Third Water Valve
		return (_state.waterValveStates & 0x02) ? 1 : 0;
	case 15: // Water Flowing To Lower Walkway To Upper Walkway Elevator
		return ((_state.waterValveStates & 0xf8) == 0xb8) ? 1 : 0;
	case 16: // Channelwood Lower Walkway to Upper Walkway Spiral Stair Upper Door State
		return _state.stairsUpperDoorState;
	case 17: // Achenar's Holoprojector Selection
		return _state.holoprojectorSelection;
	case 18: // Sirrus's Room Bed Drawer Open
		return _siriusDrawerState;
	case 19: // Sound - Water Tank Valve
		return (_state.waterValveStates & 0x80) ? 1 : 0;
	case 20: // Sound - First Water Valve Water Flowing To Left
		return ((_state.waterValveStates & 0xc0) == 0x80) ? 1 : 0;
	case 21: // Sound - Second Water Valve Water Flowing To Right
		return ((_state.waterValveStates & 0xe0) == 0xa0) ? 1 : 0;
	case 22: // Sound - Right Third Water Valve Water Flowing To Right
		return ((_state.waterValveStates & 0xf0) == 0xb0) ? 1 : 0;
	case 23: // Sound - Right Third Water Valve Water Flowing To Left
		return ((_state.waterValveStates & 0xf0) == 0xa0) ? 1 : 0;
	case 24: // Sound - Second Water Valve Water Flowing To Left
		return ((_state.waterValveStates & 0xe0) == 0x80) ? 1 : 0;
	case 25: // Sound - Right-Right Fourth Valve Water Flowing To Left (To Pipe Bridge)
		return ((_state.waterValveStates & 0xf8) == 0xb0) ? 1 : 0;
	case 26: // Sound - Right-Left Fourth Valve Water Flowing To Right (To Pipe Down Tree)
		return ((_state.waterValveStates & 0xf4) == 0xa4) ? 1 : 0;
	case 27: // Sound - Right-Left Fourth Valve Water Flowing To Left (To Pipe Fork)
		return ((_state.waterValveStates & 0xf4) == 0xa0) ? 1 : 0;
	case 28: // Sound - Left Third Water Valve Flowing To Right (To Pipe Fork)
		return ((_state.waterValveStates & 0xe2) == 0x82) ? 1 : 0;
	case 29: // Sound - Left Third Water Valve Flowing To Left (To Pipe In Water)
		return ((_state.waterValveStates & 0xe2) == 0x80) ? 1 : 0;
	case 30: // Door State
		return _doorOpened;
	case 31: // Water flowing in pipe fork ?
		// 0 -> keep sound.
		// 1 -> not flowing.
		// 2 --> flowing.
		if ((_state.waterValveStates & 0xe2) == 0x82)	// From left.
			return 2;
		if ((_state.waterValveStates & 0xf4) == 0xa0)	// From right.
			return 1;

		return 0;
	case 32: // Sound - Water Flowing in Pipe to Book Room Elevator
		return ((_state.waterValveStates & 0xf8) == 0xb0 && _state.pipeState) ? 1 : 0;
	case 33: // Channelwood Lower Walkway to Upper Walkway Spiral Stair Upper Door State
		if (_state.stairsUpperDoorState) {
			if (_tempVar == 1)
				return 2;
			else
				return 1;
		} else {
			return 0;
		}
	case 102: // Sirrus's Desk Drawer / Red Page State
		if (_siriusDrawerState) {
			if(!(_globals.redPagesInBook & 16) && (_globals.heldPage != 11))
				return 2; // Drawer Open, Red Page Present
			else
				return 1; // Drawer Open, Red Page Taken
		} else {
			return 0; // Drawer Closed
		}
	case 103: // Blue Page Present
		return !(_globals.bluePagesInBook & 16) && (_globals.heldPage != 5);
	default:
		return MystScriptParser::getVar(var);
	}
}

void Channelwood::toggleVar(uint16 var) {
	switch(var) {
	case 1: // Water Pump Bridge Raised
		_state.waterPumpBridgeState ^= 1;
		break;
	case 6: // Pipe Bridge Extended
		_state.pipeState ^= 1;
		break;
	case 16: // Channelwood Lower Walkway to Upper Walkway Spiral Stair Upper Door State
		_state.stairsUpperDoorState ^= 1;
		break;
	case 102: // Red page
		if (!(_globals.redPagesInBook & 16)) {
			if (_globals.heldPage == 11)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 11;
		}
		break;
	case 103: // Blue page
		if (!(_globals.bluePagesInBook & 16)) {
			if (_globals.heldPage == 5)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 5;
		}
		break;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool Channelwood::setVarValue(uint16 var, uint16 value) {
	bool refresh = false;

	switch (var) {
	case 2: // Lower Walkway to Upper Walkway Elevator Raised
		if (_state.elevatorState != value) {
			_state.elevatorState = value;
			refresh = true;
		}
		break;
	case 5: // Lower Walkway to Upper Walkway Spiral Stair Lower Door State
		if (_state.stairsLowerDoorState != value) {
			_state.stairsLowerDoorState = value;
			refresh = true;
		}
		break;
	case 9:
		refresh = pipeChangeValve(value, 0x40);
		break;
	case 10:
		refresh = pipeChangeValve(value, 0x20);
		break;
	case 11:
		refresh = pipeChangeValve(value, 0x10);
		break;
	case 12:
		refresh = pipeChangeValve(value, 0x08);
		break;
	case 13:
		refresh = pipeChangeValve(value, 0x04);
		break;
	case 14:
		refresh = pipeChangeValve(value, 0x02);
		break;
	case 18: // Sirrus's Room Bed Drawer Open
		if (_siriusDrawerState != value) {
			_siriusDrawerState = value;
			refresh = true;
		}
		break;
	case 30: // Door opened
		_doorOpened = value;
		break;
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

bool Channelwood::pipeChangeValve(bool open, uint16 mask) {
	if (open) {
		if (!(_state.waterValveStates & mask)) {
			_state.waterValveStates |= mask;
			return true;
		}
	} else {
		if (_state.waterValveStates & mask) {
			_state.waterValveStates &= ~mask;
			return true;
		}
	}

	return false;
}

void Channelwood::o_bridgeToggle(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Bridge rise / skink video", op);

	VideoHandle bridge = _vm->_video->playMovie(_vm->wrapMovieFilename("bridge", kChannelwoodStack), 292, 203);

	// Toggle bridge state
	if (_state.waterPumpBridgeState)
		_vm->_video->setVideoBounds(bridge, Audio::Timestamp(0, 3050, 600), Audio::Timestamp(0, 6100, 600));
	else
		_vm->_video->setVideoBounds(bridge, Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 3050, 600));

	_vm->_video->waitUntilMovieEnds(bridge);
}

void Channelwood::o_pipeExtend(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play Pipe Movie and Sound", op);

	uint16 soundId = argv[0];
	debugC(kDebugScript, "\tsoundId: %d", soundId);

	_vm->_sound->replaceSoundMyst(soundId);
	VideoHandle pipe = _vm->_video->playMovie(_vm->wrapMovieFilename("pipebrid", kChannelwoodStack), 267, 170);

	// Toggle pipe state
	if (_state.pipeState)
		_vm->_video->setVideoBounds(pipe, Audio::Timestamp(0, 3040, 600), Audio::Timestamp(0, 6080, 600));
	else
		_vm->_video->setVideoBounds(pipe, Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 3040, 600));

	_vm->_video->waitUntilMovieEnds(pipe);
	_vm->_sound->resumeBackgroundMyst();
}

void Channelwood::o_drawImageChangeCardAndVolume(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Draw Full Screen Image, Change Card, and change volume", op);

	uint16 imageId = argv[0];
	uint16 cardId = argv[1];

	debugC(kDebugScript, "\timageId: %d", imageId);
	debugC(kDebugScript, "\tcardId: %d", cardId);

	_vm->_gfx->copyImageToScreen(imageId, Common::Rect(0, 0, 544, 333));
	_vm->_system->updateScreen();

	_vm->changeToCard(cardId, true);

	if (argc == 3) {
		uint16 volume = argv[2];
		_vm->_sound->changeBackgroundVolumeMyst(volume);
	}
}


void Channelwood::o_waterTankValveOpen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Do Water Tank Valve Open Animation", op);
	Common::Rect rect = _invokingResource->getRect();

	for (uint i = 0; i < 2; i++)
		for (uint16 imageId = 3601; imageId >= 3595; imageId--) {
			_vm->_gfx->copyImageToScreen(imageId, rect);
			_vm->_system->updateScreen();
		}

	pipeChangeValve(true, 0x80);
}

void Channelwood::o_leverStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generic lever start move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
	lever->drawFrame(0);
	_vm->_cursor->setCursor(700);
	_leverPulled = false;
}

void Channelwood::o_leverMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generic lever move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	if (lever->pullLeverV()) {
		if (!_leverPulled) {
			_leverPulled = true;
			_leverAction->handleMouseUp();
		}
	} else {
		_leverPulled = false;
	}
}

void Channelwood::o_leverMoveFail(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generic lever move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	if (lever->pullLeverV()) {
		if (!_leverPulled) {
			_leverPulled = true;
			uint16 soundId = lever->getList2(0);
			if (soundId)
				_vm->_sound->replaceSoundMyst(soundId);
		}
	} else {
		_leverPulled = false;
	}
}

void Channelwood::o_leverEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generic lever end move", op);

	// Get current lever frame
	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	// Release lever
	lever->releaseLeverV();

	uint16 soundId = lever->getList3(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);

	_vm->checkCursorHints();
}

void Channelwood::o_leverEndMoveResumeBackground(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_vm->_sound->resumeBackgroundMyst();
	o_leverEndMove(op, var, argc, argv);
}

void Channelwood::o_leverEndMoveWithSound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	o_leverEndMove(op, var, argc, argv);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
	uint16 soundId = lever->getList3(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);
}

void Channelwood::o_leverElev3StartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_vm->_gfx->copyImageToScreen(3970, Common::Rect(544, 333));
	_vm->_system->updateScreen();
	o_leverStartMove(op, var, argc, argv);
}

void Channelwood::o_leverElev3EndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	o_leverEndMove(op, var, argc, argv);
	_vm->_gfx->copyImageToScreen(3265, Common::Rect(544, 333));
	_vm->_system->updateScreen();
	_vm->_sound->replaceSoundMyst(5265);
}

void Channelwood::o_pumpLeverMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Pump lever move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	if (lever->pullLeverV()) {
		uint16 soundId = lever->getList2(0);
		_vm->_sound->replaceBackgroundMyst(soundId, 38400);
	} else {
		uint16 soundId = lever->getList2(1);
		_vm->_sound->replaceBackgroundMyst(soundId, 36864);
	}
}

void Channelwood::o_pumpLeverEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	o_leverEndMove(op, var, argc, argv);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
	uint16 soundId = lever->getList3(0);
	if (soundId)
		_vm->_sound->replaceBackgroundMyst(soundId, 36864);
}

void Channelwood::o_stairsDoorToggle(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play stairs door video", op);

	MystResourceType6 *movie = static_cast<MystResourceType6 *>(_invokingResource);

	if (_state.stairsUpperDoorState) {
		// TODO: Play backwards
		movie->playMovie();
	} else {
		movie->playMovie();
	}
}

void Channelwood::o_valveHandleMove1(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	if (handle->getRect().contains(mouse)) {
		// Compute frame to draw
		_tempVar = (mouse.x - 250) / 4;
		_tempVar = CLIP<int16>(_tempVar, 1, handle->getStepsH() - 2);

		// Draw frame
		handle->drawFrame(_tempVar);
	}
}

void Channelwood::o_valveHandleMoveStart1(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move start", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	uint16 soundId = handle->getList1(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);
	_vm->_cursor->setCursor(700);

	o_valveHandleMove1(op, var, argc, argv);
}

void Channelwood::o_valveHandleMoveStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move stop", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);

	// Update state with valve position
	if (_tempVar <= 5)
		setVarValue(_valveVar, 1);
	else
		setVarValue(_valveVar, 0);

	// Play release sound
	uint16 soundId = handle->getList3(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);

	// Redraw valve
	_vm->redrawArea(_valveVar);

	// Restore cursor
	_vm->checkCursorHints();
}

void Channelwood::o_valveHandleMove2(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	if (handle->getRect().contains(mouse)) {
		// Compute frame to draw
		_tempVar = handle->getStepsH() - (mouse.x - 234) / 4;
		_tempVar = CLIP<int16>(_tempVar, 1, handle->getStepsH() - 2);

		// Draw frame
		handle->drawFrame(_tempVar);
	}
}

void Channelwood::o_valveHandleMoveStart2(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move start", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	uint16 soundId = handle->getList1(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);
	_vm->_cursor->setCursor(700);

	o_valveHandleMove2(op, var, argc, argv);
}

void Channelwood::o_valveHandleMove3(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	if (handle->getRect().contains(mouse)) {
		// Compute frame to draw
		_tempVar = handle->getStepsH() - (mouse.x - 250) / 4;
		_tempVar = CLIP<int16>(_tempVar, 1, handle->getStepsH() - 2);

		// Draw frame
		handle->drawFrame(_tempVar);
	}
}

void Channelwood::o_valveHandleMoveStart3(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move start", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	uint16 soundId = handle->getList1(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);
	_vm->_cursor->setCursor(700);

	o_valveHandleMove3(op, var, argc, argv);
}

void Channelwood::o_hologramMonitor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Hologram monitor", op);

	// Used on Card 3012 (Temple Hologram Monitor)
	uint16 button = argv[0]; // 0 to 3

	if (_state.holoprojectorSelection != button || !_vm->_video->isVideoPlaying()) {
		_state.holoprojectorSelection = button;
		_vm->redrawArea(17);

		_vm->_video->stopVideos();

		switch (button) {
		case 0:
			_vm->_video->playMovie(_vm->wrapMovieFilename("monalgh", kChannelwoodStack), 227, 70);
			break;
		case 1:
			_vm->_video->playMovie(_vm->wrapMovieFilename("monamth", kChannelwoodStack), 227, 70);
			break;
		case 2:
			_vm->_video->playMovie(_vm->wrapMovieFilename("monasirs", kChannelwoodStack), 227, 70);
			break;
		case 3:
			_vm->_video->playMovie(_vm->wrapMovieFilename("monsmsg", kChannelwoodStack), 227, 70);
			break;
		default:
			warning("Opcode %d Control Variable Out of Range", op);
			break;
		}
	}
}

void Channelwood::o_drawerOpen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Open Sirius drawer", op);

	_siriusDrawerState = 1;
	_vm->redrawArea(18, false);
	_vm->redrawArea(102, false);
}

void Channelwood::o_hologramTemple(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Temple hologram", op);

	_vm->_sound->pauseBackgroundMyst();

	// Used on Card 3333 (Temple Hologram)
	switch (_state.holoprojectorSelection) {
	case 0:
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("holoalgh", kChannelwoodStack), 139, 64);
		break;
	case 1:
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("holoamth", kChannelwoodStack), 127, 73);
		break;
	case 2:
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("holoasir", kChannelwoodStack), 139, 64);
		break;
	case 3:
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("holosmsg", kChannelwoodStack), 127, 45);
		break;
	default:
		warning("Opcode %d Control Variable Out of Range", op);
		break;
	}

	_vm->_sound->resumeBackgroundMyst();
}

void Channelwood::o_executeMouseUp(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Execute mouse up", op);

	MystResourceType5 *resource = static_cast<MystResourceType5 *>(_vm->_resources[argv[0]]);
	resource->handleMouseUp();
}

void Channelwood::o_waterTankValveClose(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Do Water Tank Valve Close Animation", op);
	Common::Rect rect = _invokingResource->getRect();

	for (uint i = 0; i < 2; i++)
		for (uint16 imageId = 3595; imageId <= 3601; imageId++) {
			_vm->_gfx->copyImageToScreen(imageId, rect);
			_vm->_system->updateScreen();
		}

	pipeChangeValve(false, 0x80);
}

void Channelwood::o_elevatorMovies(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used by Card 3262 (Elevator)
	debugC(kDebugScript, "Opcode %d: Elevator movie", op);

	uint16 elevator = argv[0];
	uint16 direction = argv[1];

	Common::String movie;
	uint16 x;
	uint16 y;

	switch (elevator) {
	case 1:
		x = 214;
		y = 106;
		if (direction == 1)
			movie = _vm->wrapMovieFilename("welev1up", kChannelwoodStack);
		else
			movie = _vm->wrapMovieFilename("welev1dn", kChannelwoodStack);
		break;
	case 2:
		x = 215;
		y = 117;
		if (direction == 1)
			movie = _vm->wrapMovieFilename("welev2up", kChannelwoodStack);
		else
			movie = _vm->wrapMovieFilename("welev2dn", kChannelwoodStack);
		break;
	case 3:
		x = 213;
		y = 98;
		if (direction == 1)
			movie = _vm->wrapMovieFilename("welev3up", kChannelwoodStack);
		else
			movie = _vm->wrapMovieFilename("welev3dn", kChannelwoodStack);
		break;
	default:
		error("Unknown elevator state %d in o_elevatorMovies", elevator);
	}

	_vm->_sound->pauseBackgroundMyst();
	_vm->_video->playMovieBlocking(movie, x, y);
	_vm->_sound->resumeBackgroundMyst();
}

void Channelwood::o_soundReplace(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play sound if not already playing", op);

	uint16 soundId = argv[0];

	// TODO: If is foreground playing
	_vm->_sound->replaceSoundMyst(soundId);
}

void Channelwood::o_lever_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generic lever init", op);
	_leverAction = static_cast<MystResourceType5 *>(_invokingResource);
}

void Channelwood::o_pipeValve_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Water valve init", op);
	_valveVar = var;
}

void Channelwood::o_drawer_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sirius's drawer init", op);
	_siriusDrawerState = 0;
}

} // End of namespace MystStacks
} // End of namespace Mohawk
