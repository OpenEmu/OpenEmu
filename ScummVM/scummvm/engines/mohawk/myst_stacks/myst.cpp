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
#include "mohawk/myst_stacks/myst.h"

#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "gui/message.h"

namespace Mohawk {
namespace MystStacks {

Myst::Myst(MohawkEngine_Myst *vm) :
		MystScriptParser(vm), _state(_vm->_gameState->_myst) {
	setupOpcodes();

	// Card ID preinitialized by the engine for use by opcode 18
	// when linking back to Myst in the library
	_savedCardId = 4329;

	_towerRotationBlinkLabel = false;
	_libraryBookcaseChanged = false;
	_dockVaultState = 0;
	_cabinDoorOpened = 0;
	_cabinMatchState = 2;
	_matchBurning = false;
	_tree = 0;
	_treeAlcove = 0;
	_treeStopped = false;
	_treeMinPosition = 0;
	_imagerValidationStep = 0;
	_observatoryCurrentSlider = 0;
	_butterfliesMoviePlayed = false;
	_state.treeLastMoveTime = _vm->_system->getMillis();
}

Myst::~Myst() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &Myst::x, #x))

void Myst::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, NOP);
	OPCODE(101, o_libraryBookPageTurnLeft);
	OPCODE(102, o_libraryBookPageTurnRight);
	OPCODE(103, o_fireplaceToggleButton);
	OPCODE(104, o_fireplaceRotation);
	OPCODE(105, o_courtyardBoxesCheckSolution);
	OPCODE(106, o_towerRotationStart);
	OPCODE(107, NOP);
	OPCODE(108, o_towerRotationEnd);
	OPCODE(109, o_imagerChangeSelection);
	OPCODE(113, o_dockVaultOpen);
	OPCODE(114, o_dockVaultClose);
	OPCODE(115, o_bookGivePage);
	OPCODE(116, o_clockWheelsExecute);
	OPCODE(117, o_imagerPlayButton);
	OPCODE(118, o_imagerEraseButton);
	OPCODE(119, o_towerElevatorAnimation);
	OPCODE(120, o_generatorButtonPressed);
	OPCODE(121, o_cabinSafeChangeDigit);
	OPCODE(122, o_cabinSafeHandleStartMove);
	OPCODE(123, o_cabinSafeHandleMove);
	OPCODE(124, o_cabinSafeHandleEndMove);
	OPCODE(126, o_clockLeverStartMove);
	OPCODE(127, o_clockLeverEndMove);
	OPCODE(128, o_treePressureReleaseStart);
	if (!observatoryIsDDMMYYYY2400()) {
		OPCODE(129, o_observatoryMonthChangeStart);
		OPCODE(130, o_observatoryMonthChangeStart);
		OPCODE(131, o_observatoryDayChangeStart);
		OPCODE(132, o_observatoryDayChangeStart);
	} else {
		OPCODE(129, o_observatoryDayChangeStart);
		OPCODE(130, o_observatoryDayChangeStart);
		OPCODE(131, o_observatoryMonthChangeStart);
		OPCODE(132, o_observatoryMonthChangeStart);
	}
	OPCODE(133, o_observatoryGoButton);
	OPCODE(134, o_observatoryMonthSliderMove);
	OPCODE(135, o_observatoryDaySliderMove);
	OPCODE(136, o_observatoryYearSliderMove);
	OPCODE(137, o_observatoryTimeSliderMove);
	OPCODE(138, o_clockResetLeverStartMove);
	OPCODE(139, o_clockResetLeverMove);
	OPCODE(140, o_clockResetLeverEndMove);
	OPCODE(141, o_circuitBreakerStartMove);
	OPCODE(142, o_circuitBreakerMove);
	OPCODE(143, o_circuitBreakerEndMove);
	OPCODE(144, o_clockLeverMove);
	OPCODE(145, o_clockLeverMove);
	OPCODE(146, o_boilerIncreasePressureStart);
	OPCODE(147, o_boilerLightPilot);
	OPCODE(148, NOP);
	OPCODE(149, o_boilerIncreasePressureStop);
	OPCODE(150, o_boilerDecreasePressureStart);
	OPCODE(151, o_boilerDecreasePressureStop);
	OPCODE(152, NOP);
	OPCODE(153, o_basementIncreasePressureStart);
	OPCODE(154, o_basementIncreasePressureStop);
	OPCODE(155, o_basementDecreasePressureStart);
	OPCODE(156, o_basementDecreasePressureStop);
	OPCODE(157, o_rocketPianoMove);
	OPCODE(158, o_rocketSoundSliderStartMove);
	OPCODE(159, o_rocketSoundSliderMove);
	OPCODE(160, o_rocketSoundSliderEndMove);
	OPCODE(161, o_rocketPianoStart);
	OPCODE(162, o_rocketPianoStop);
	OPCODE(163, o_rocketLeverStartMove);
	OPCODE(164, o_rocketOpenBook);
	OPCODE(165, o_rocketLeverMove);
	OPCODE(166, o_rocketLeverEndMove);
	OPCODE(167, NOP);
	OPCODE(168, o_treePressureReleaseStop);
	OPCODE(169, o_cabinLeave);
	OPCODE(170, o_observatoryMonthSliderStartMove);
	OPCODE(171, o_observatoryMonthSliderEndMove);
	OPCODE(172, o_observatoryDaySliderStartMove);
	OPCODE(173, o_observatoryDaySliderEndMove);
	OPCODE(174, o_observatoryYearSliderStartMove);
	OPCODE(175, o_observatoryYearSliderEndMove);
	OPCODE(176, o_observatoryTimeSliderStartMove);
	OPCODE(177, o_observatoryTimeSliderEndMove);
	OPCODE(180, o_libraryCombinationBookStop);
	OPCODE(181, NOP);
	OPCODE(182, o_cabinMatchLight);
	OPCODE(183, o_courtyardBoxEnter);
	OPCODE(184, o_courtyardBoxLeave);
	OPCODE(185, NOP);
	OPCODE(186, o_clockMinuteWheelStartTurn);
	OPCODE(187, NOP);
	OPCODE(188, o_clockWheelEndTurn);
	OPCODE(189, o_clockHourWheelStartTurn);
	OPCODE(190, o_libraryCombinationBookStartRight);
	OPCODE(191, o_libraryCombinationBookStartLeft);
	OPCODE(192, o_observatoryTimeChangeStart);
	OPCODE(193, NOP);
	OPCODE(194, o_observatoryChangeSettingStop);
	OPCODE(195, o_observatoryTimeChangeStart);
	OPCODE(196, o_observatoryYearChangeStart);
	OPCODE(197, o_observatoryYearChangeStart);
	OPCODE(198, o_dockVaultForceClose);
	OPCODE(199, o_imagerEraseStop);

	// "Init" Opcodes
	OPCODE(200, o_libraryBook_init);
	OPCODE(201, o_courtyardBox_init);
	OPCODE(202, o_towerRotationMap_init);
	OPCODE(203, o_forechamberDoor_init);
	OPCODE(204, o_shipAccess_init);
	OPCODE(205, NOP);
	OPCODE(206, o_butterflies_init);
	OPCODE(208, o_imager_init);
	OPCODE(209, o_libraryBookcaseTransform_init);
	OPCODE(210, o_generatorControlRoom_init);
	OPCODE(211, o_fireplace_init);
	OPCODE(212, o_clockGears_init);
	OPCODE(213, o_gulls1_init);
	OPCODE(214, o_observatory_init);
	OPCODE(215, o_gulls2_init);
	OPCODE(216, o_treeCard_init);
	OPCODE(217, o_treeEntry_init);
	OPCODE(218, opcode_218);
	OPCODE(219, o_rocketSliders_init);
	OPCODE(220, o_rocketLinkVideo_init);
	OPCODE(221, o_greenBook_init);
	OPCODE(222, o_gulls3_init);

	// "Exit" Opcodes
	OPCODE(300, o_bookAddSpecialPage_exit);
	OPCODE(301, NOP);
	OPCODE(302, NOP);
	OPCODE(303, NOP);
	OPCODE(304, o_treeCard_exit);
	OPCODE(305, o_treeEntry_exit);
	OPCODE(306, NOP);
	OPCODE(307, o_generatorControlRoom_exit);
	OPCODE(308, NOP);
	OPCODE(309, NOP);
	OPCODE(312, NOP);
}

#undef OPCODE

void Myst::disablePersistentScripts() {
	_libraryBookcaseMoving = false;
	_generatorControlRoomRunning = false;
	_libraryCombinationBookPagesTurning = false;
	_clockTurningWheel = 0;
	_towerRotationMapRunning = false;
	_boilerPressureIncreasing = false;
	_boilerPressureDecreasing = false;
	_basementPressureIncreasing = false;
	_basementPressureDecreasing = false;
	_imagerValidationRunning = false;
	_imagerRunning = false;
	_observatoryRunning = false;
	_observatoryMonthChanging = false;
	_observatoryDayChanging = false;
	_observatoryYearChanging = false;
	_observatoryTimeChanging = false;
	_greenBookRunning = false;
	_clockLeverPulled = false;
	_gullsFlying1 = false;
	_gullsFlying2 = false;
	_gullsFlying3 = false;
}

void Myst::runPersistentScripts() {
	if (_towerRotationMapRunning)
		towerRotationMap_run();

	if (_generatorControlRoomRunning)
		generatorControlRoom_run();

	if (_libraryCombinationBookPagesTurning)
		libraryCombinationBook_run();

	if (_libraryBookcaseMoving)
		libraryBookcaseTransform_run();

	if (_clockTurningWheel)
		clockWheel_run();

	if (_matchBurning)
		matchBurn_run();

	if (_boilerPressureIncreasing)
		boilerPressureIncrease_run();

	if (_boilerPressureDecreasing)
		boilerPressureDecrease_run();

	if (_basementPressureIncreasing)
		basementPressureIncrease_run();

	if (_basementPressureDecreasing)
		basementPressureDecrease_run();

	if (!_treeStopped)
		tree_run();

	if (_imagerValidationRunning)
		imagerValidation_run();

	if (_imagerRunning)
		imager_run();

	if (_observatoryRunning)
		observatory_run();

	if (_observatoryMonthChanging)
		observatoryMonthChange_run();

	if (_observatoryDayChanging)
		observatoryDayChange_run();

	if (_observatoryYearChanging)
		observatoryYearChange_run();

	if (_observatoryTimeChanging)
		observatoryTimeChange_run();

	if (_greenBookRunning)
		greenBook_run();

	if (_clockLeverPulled)
		clockGears_run();

	if (_gullsFlying1)
		gullsFly1_run();

	if (_gullsFlying2)
		gullsFly2_run();

	if (_gullsFlying3)
		gullsFly3_run();
}

uint16 Myst::getVar(uint16 var) {
	switch(var) {
	case 0: // Myst Library Bookcase Closed
		return _state.libraryBookcaseDoor;
	case 1:
		if (_globals.ending != 4)
			return _state.libraryBookcaseDoor != 1;
		else if (_state.libraryBookcaseDoor == 1)
			return 2;
		else
			return 3;
	case 2: // Marker Switch Near Cabin
		return _state.cabinMarkerSwitch;
	case 3: // Marker Switch Near Clock Tower
		return _state.clockTowerMarkerSwitch;
	case 4: // Marker Switch on Dock
		return _state.dockMarkerSwitch;
	case 5: // Marker Switch Near Ship Pool
		return _state.poolMarkerSwitch;
	case 6: // Marker Switch Near Cogs
		return _state.gearsMarkerSwitch;
	case 7: // Marker Switch Near Generator Room
		return _state.generatorMarkerSwitch;
	case 8: // Marker Switch Near Stellar Observatory
		return _state.observatoryMarkerSwitch;
	case 9: // Marker Switch Near Rocket Ship
		return _state.rocketshipMarkerSwitch;
	case 10: // Ship Floating State
		return _state.shipFloating;
	case 11: // Cabin Door Open State
		return _cabinDoorOpened;
	case 12: // Clock tower gears bridge
		return _state.clockTowerBridgeOpen;
	case 13: // Tower in right position
		return _state.towerRotationAngle == 271
				|| _state.towerRotationAngle == 83
				|| _state.towerRotationAngle == 129
				|| _state.towerRotationAngle == 152;
	case 14: // Tower Solution (Key) Plaque
		switch (_state.towerRotationAngle) {
		case 271:
			return 1;
		case 83:
			return 2;
		case 129:
			return 3;
		case 152:
			return 4;
		default:
			return 0;
		}
	case 15: // Tower Window (Book) View
		switch (_state.towerRotationAngle) {
		case 271:
			return 1;
		case 83:
			if (_state.gearsOpen)
				return 6;
			else
				return 2;
		case 129:
			if (_state.shipFloating)
				return 5;
			else
				return 3;
		case 152:
			return 4;
		default:
			return 0;
		}
	case 16: // Tower Window (Book) View From Ladder Top
		if (_state.towerRotationAngle != 271 && _state.towerRotationAngle != 83	&& _state.towerRotationAngle != 129) {
			if (_state.towerRotationAngle == 152)
				return 2;
			else
				return 0;
		} else
			return 1;
	case 23: // Fireplace Pattern Correct
		return _fireplaceLines[0] == 195
				&& _fireplaceLines[1] == 107
				&& _fireplaceLines[2] == 163
				&& _fireplaceLines[3] == 147
				&& _fireplaceLines[4] == 204
				&& _fireplaceLines[5] == 250;
	case 24: // Fireplace Blue Page Present
		if (_globals.ending != 4)
			return !(_globals.bluePagesInBook & 32) && (_globals.heldPage != 6);
		else
			return 0;
	case 25: // Fireplace Red Page Present
		if (_globals.ending != 4)
			return !(_globals.redPagesInBook & 32) && (_globals.heldPage != 12);
		else
			return 0;
	case 26: // Courtyard Image Box - Cross
	case 27: // Courtyard Image Box - Leaf
	case 28: // Courtyard Image Box - Arrow
	case 29: // Courtyard Image Box - Eye
	case 30: // Courtyard Image Box - Snake
	case 31: // Courtyard Image Box - Spider
	case 32: // Courtyard Image Box - Anchor
	case 33: // Courtyard Image Box - Ostrich
		if (!_tempVar)
			return 0;
		else if (_state.courtyardImageBoxes & (0x01 << (var - 26)))
			return 2;
		else
			return 1;
	case 34: // Sound Control In Dock forechamber
		if (_state.imagerActive) {
			if (_state.imagerSelection == 40 && !_state.imagerMountainErased)
				return 1;
			else if (_state.imagerSelection == 67 && !_state.imagerWaterErased)
				return 2;
			else
				return 0;
		}

		return 0;
	case 35: // Dock Forechamber Imager Control Left Digit
		if (_state.imagerSelection > 9)
			return _state.imagerSelection / 10 - 1;
		else
			return 9;
	case 36: // Dock Forechamber Imager Control Right Digit
		return (10 + _state.imagerSelection - 1) % 10;
	case 37: // Clock Tower Control Wheels Position
		return 3 * ((_state.clockTowerMinutePosition / 5) % 3) + _state.clockTowerHourPosition % 3;
	case 40: // Gears Open State
		return _state.gearsOpen;
	case 41: // Dock Marker Switch Vault State
		return _dockVaultState;
	case 43: // Clock Tower Time
		return _state.clockTowerHourPosition * 12 + _state.clockTowerMinutePosition / 5;
	case 44: // Rocket ship power state
		if (_state.generatorBreakers || _state.generatorVoltage == 0)
			return 0;
		else if (_state.generatorVoltage != 59)
			return 1;
		else
			return 2;
	case 45: // Dock Vault Imager Active On Water
		return _state.imagerActive && _state.imagerSelection == 67 && !_state.imagerWaterErased;
	case 46:
		return bookCountPages(100);
	case 47:
		return bookCountPages(101);
	case 48:
		if (_state.dockMarkerSwitch && !_state.shipFloating)
			return 1;
		else if (!_state.dockMarkerSwitch && _state.shipFloating)
			return 2;
		else
			return 0;
	case 49: // Generator running
		return _state.generatorVoltage > 0;
	case 51: // Forechamber Imager Movie Control
		if (_state.imagerSelection == 40 && !_state.imagerMountainErased)
			return 1;
		else if (_state.imagerSelection == 67 && !_state.imagerWaterErased)
			return 2;
		else if (_state.imagerSelection == 8 && !_state.imagerAtrusErased)
			return 3;
		else if (_state.imagerSelection == 47 && !_state.imagerMarkerErased)
			return 4;
		else
			return 0;
	case 52: // Generator Switch #1
	case 53: // Generator Switch #2
	case 54: // Generator Switch #3
	case 55: // Generator Switch #4
	case 56: // Generator Switch #5
	case 57: // Generator Switch #6
	case 58: // Generator Switch #7
	case 59: // Generator Switch #8
	case 60: // Generator Switch #9
	case 61: // Generator Switch #10
		return (_state.generatorButtons & (1 << (var - 52))) != 0;
	case 62: // Generator Power Dial Left LED Digit
		return _generatorVoltage / 10;
	case 63: // Generator Power Dial Right LED Digit
		return _generatorVoltage % 10;
	case 64: // Generator Power To Spaceship Dial Left LED Digit
		if (_state.generatorVoltage > 59 || _state.generatorBreakers)
			return 0;
		else
			return _state.generatorVoltage / 10;
	case 65: // Generator Power To Spaceship Dial Right LED Digit
		if (_state.generatorVoltage > 59 || _state.generatorBreakers)
			return 0;
		else
			return _state.generatorVoltage % 10;
	case 66: // Generators lights on
		return 0;
	case 67: // Cabin Safe Lock Number #1 - Left
		return _state.cabinSafeCombination / 100;
	case 68: // Cabin Safe Lock Number #2
		return (_state.cabinSafeCombination / 10) % 10;
	case 69: // Cabin Safe Lock Number #3 - Right
		return _state.cabinSafeCombination % 10;
	case 70: // Cabin Safe Matchbox State
		return _cabinMatchState;
	case 71: // Stellar Observatory Lights
		return _state.observatoryLights;
	case 72: // Channelwood tree position
		return _state.treePosition;
	case 73: // Stellar Observatory Date - Month
		return _state.observatoryMonthSetting;
	case 74: // Stellar Observatory Date - Day #1 (Left)
		if (_state.observatoryDaySetting / 10 == 0)
			return 10;
		else
			return _state.observatoryDaySetting / 10;
	case 75: // Stellar Observatory Date - Day #2 (Right)
		return _state.observatoryDaySetting % 10;
	case 76: // Stellar Observatory Date - Year #1 (Left)
		if (_state.observatoryYearSetting >= 1000)
			return (_state.observatoryYearSetting / 1000) % 10;
		else
			return 10;
	case 77: // Stellar Observatory Date - Year #2
		if (_state.observatoryYearSetting >= 100)
			return (_state.observatoryYearSetting / 100) % 10;
		else
			return 10;
	case 78: // Stellar Observatory Date - Year #3
		if (_state.observatoryYearSetting >= 10)
			return (_state.observatoryYearSetting / 10) % 10;
		else
			return 10;
	case 79: // Stellar Observatory Date - Year #4 (Right)
		return (_state.observatoryYearSetting / 1) % 10;
	case 80: // Stellar Observatory Hour #1 - Left ( Number 1 (0) or Blank (10))
		if (!observatoryIsDDMMYYYY2400()) {
			if (_state.observatoryTimeSetting % (12 * 60) < (10 * 60))
				return 10;
			else
				return 1;
		} else {
			if (_state.observatoryTimeSetting < (10 * 60))
				return 10;
			else if (_state.observatoryTimeSetting < (20 * 60))
				return 1;
			else
				return 2;
		}
	case 81: // Stellar Observatory Hour #2 - Right
		if (!observatoryIsDDMMYYYY2400())
			return ((_state.observatoryTimeSetting % (12 * 60)) / 60) % 10;
		else
			return (_state.observatoryTimeSetting / 60) % 10;
	case 82: // Stellar Observatory Minutes #1 - Left
		return (_state.observatoryTimeSetting % 60) / 10;
	case 83: // Stellar Observatory Minutes #2 - Right
		return (_state.observatoryTimeSetting % 60) % 10;
	case 88: // Stellar Observatory AM/PM
		if (_state.observatoryTimeSetting < (12 * 60))
			return 0; // AM
		else
			return 1; // PM
	case 89:
	case 90:
	case 91:
	case 92: // Stellar observatory sliders state
		return 1;
	case 93: // Breaker nearest Generator Room Blown
		return _state.generatorBreakers == 1;
	case 94: // Breaker nearest Rocket Ship Blown
		return _state.generatorBreakers == 2;
	case 95: // Going out of tree destination selection
		if (_state.treePosition == 0)
			return 0;
		else if (_state.treePosition == 4 || _state.treePosition == 5)
			return 1;
		else
			return 2;
	case 96: // Generator Power Dial Needle Position
		return _state.generatorVoltage / 4;
	case 97: // Generator Power To Spaceship Dial Needle Position
		if (_state.generatorVoltage > 59 || _state.generatorBreakers)
			return 0;
		else
			return _state.generatorVoltage / 4;
	case 98: // Cabin Boiler Pilot Light Lit
		return _state.cabinPilotLightLit;
	case 99: // Cabin Boiler Gas Valve Position
		return _state.cabinValvePosition % 6;
	case 102: // Red page
		if (_globals.ending != 4)
			return !(_globals.redPagesInBook & 1) && (_globals.heldPage != 7);
		else
			return 0;
	case 103: // Blue page
		if (_globals.ending != 4)
			return !(_globals.bluePagesInBook & 1) && (_globals.heldPage != 1);
		else
			return 0;
	case 300: // Rocket Ship Music Puzzle Slider State
		return 1;
	case 302: // Green Book Opened Before Flag
		return _state.greenBookOpenedBefore;
	case 304: // Tower Rotation Map Initialized
		return _towerRotationMapInitialized;
	case 305: // Cabin Boiler Lit
		return _state.cabinPilotLightLit == 1 && _state.cabinValvePosition > 0;
	case 306: // Cabin Boiler Steam Sound Control
		if (_state.cabinPilotLightLit == 1) {
			if (_state.cabinValvePosition <= 0)
				return 26;
			else
				return 27;
		}

		return _state.cabinValvePosition;
	case 307: // Cabin Boiler Fully Pressurized
		return _state.cabinPilotLightLit == 1 && _state.cabinValvePosition > 12;
	case 308: // Cabin handle position
		return 0; // Not implemented in the original
	default:
		return MystScriptParser::getVar(var);
	}
}

void Myst::toggleVar(uint16 var) {
	switch(var) {
	case 2: // Marker Switch Near Cabin
		_state.cabinMarkerSwitch = (_state.cabinMarkerSwitch + 1) % 2;
		break;
	case 3: // Marker Switch Near Clock Tower
		_state.clockTowerMarkerSwitch = (_state.clockTowerMarkerSwitch + 1) % 2;
		break;
	case 4: // Marker Switch on Dock
		_state.dockMarkerSwitch = (_state.dockMarkerSwitch + 1) % 2;
		break;
	case 5: // Marker Switch Near Ship Pool
		_state.poolMarkerSwitch = (_state.poolMarkerSwitch + 1) % 2;
		break;
	case 6: // Marker Switch Near Cogs
		_state.gearsMarkerSwitch = (_state.gearsMarkerSwitch + 1) % 2;
		break;
	case 7: // Marker Switch Near Generator Room
		_state.generatorMarkerSwitch = (_state.generatorMarkerSwitch + 1) % 2;
		break;
	case 8: // Marker Switch Near Stellar Observatory
		_state.observatoryMarkerSwitch = (_state.observatoryMarkerSwitch + 1) % 2;
		break;
	case 9: // Marker Switch Near Rocket Ship
		_state.rocketshipMarkerSwitch = (_state.rocketshipMarkerSwitch + 1) % 2;
		break;
	case 24: // Fireplace Blue Page
		if (_globals.ending != 4 && !(_globals.bluePagesInBook & 32)) {
			if (_globals.heldPage == 6)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 6;
		}
		break;
	case 25: // Fireplace Red page
		if (_globals.ending != 4 && !(_globals.redPagesInBook & 32)) {
			if (_globals.heldPage == 12)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 12;
		}
		break;
	case 26: // Courtyard Image Box - Cross
	case 27: // Courtyard Image Box - Leaf
	case 28: // Courtyard Image Box - Arrow
	case 29: // Courtyard Image Box - Eye
	case 30: // Courtyard Image Box - Snake
	case 31: // Courtyard Image Box - Spider
	case 32: // Courtyard Image Box - Anchor
	case 33: // Courtyard Image Box - Ostrich
		{
			uint16 mask = 0x01 << (var - 26);
			if (_state.courtyardImageBoxes & mask)
				_state.courtyardImageBoxes &= ~mask;
			else
				_state.courtyardImageBoxes |= mask;
		}
	case 41: // Vault white page
		if (_globals.ending != 4) {
			if (_dockVaultState == 1) {
				_dockVaultState = 2;
				_globals.heldPage = 0;
			} else if (_dockVaultState == 2) {
				_dockVaultState = 1;
				_globals.heldPage = 13;
			}
		}
		break;
	case 102: // Red page
		if (_globals.ending != 4 && !(_globals.redPagesInBook & 1)) {
			if (_globals.heldPage == 7)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 7;
		}
		break;
	case 103: // Blue page
		if (_globals.ending != 4 && !(_globals.bluePagesInBook & 1)) {
			if (_globals.heldPage == 1)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 1;
		}
		break;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool Myst::setVarValue(uint16 var, uint16 value) {
	bool refresh = false;

	switch (var) {
	case 0: // Myst Library Bookcase Closed
		if (_state.libraryBookcaseDoor != value) {
			_state.libraryBookcaseDoor = value;
			_tempVar = 0;
			refresh = true;
		}
		break;
	case 11: // Cabin Door Open State
		if (_cabinDoorOpened != value) {
			_cabinDoorOpened = value;
			refresh = true;
		}
		break;
	case 70: // Cabin Safe Matchbox State
		if (_cabinMatchState != value) {
			_cabinMatchState = value;
			refresh = true;
		}
		break;
	case 71: // Stellar Observatory Lights
		_state.observatoryLights = value;
		break;
	case 89:
	case 90:
	case 91:
	case 92:
	case 300: // Set slider value
		break; // Do nothing
	case 302: // Green Book Opened Before Flag
		_state.greenBookOpenedBefore = value;
		break;
	case 303: // Library Bookcase status changed
		_libraryBookcaseChanged = value;
		break;
	case 304: // Myst Library Image Present on Tower Rotation Map
		_towerRotationMapInitialized = value;
		break;
	case 309: // Tree stopped
		_treeStopped = value;
		break;
	case 310: // Imager validation step
		_imagerValidationStep = value;
		break;
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

uint16 Myst::bookCountPages(uint16 var) {
	uint16 pages = 0;
	uint16 cnt = 0;

	// Select book according to var
	if (var == 100)
		pages = _globals.redPagesInBook;
	else if (var == 101)
		pages = _globals.bluePagesInBook;

	// Special page present
	if (pages & 64)
		return 6;

	// Count pages
	if (pages & 1)
		cnt++;

	if (pages & 2)
		cnt++;

	if (pages & 4)
		cnt++;

	if (pages & 8)
		cnt++;

	if (pages & 16)
		cnt++;

	return cnt;
}

void Myst::o_libraryBookPageTurnLeft(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Turn book page left", op);

	if (_libraryBookPage - 1 >= 0) {
		_libraryBookPage--;

		Common::Rect rect = Common::Rect(0, 0, 544, 333);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->replaceSoundMyst(_libraryBookSound1);
		else
			_vm->_sound->replaceSoundMyst(_libraryBookSound2);

		_vm->_system->updateScreen();
	}
}

void Myst::o_libraryBookPageTurnRight(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Turn book page right", op);

	if (_libraryBookPage + 1 < _libraryBookNumPages) {
		_libraryBookPage++;

		Common::Rect rect = Common::Rect(0, 0, 544, 333);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->replaceSoundMyst(_libraryBookSound1);
		else
			_vm->_sound->replaceSoundMyst(_libraryBookSound2);

		_vm->_system->updateScreen();
	}
}

void Myst::o_fireplaceToggleButton(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Myst Card 4162 (Fireplace Grid)
	debugC(kDebugScript, "Opcode %d: Fireplace grid toggle button", op);

	uint16 bitmask = argv[0];
	uint16 line = _fireplaceLines[var - 17];

	debugC(kDebugScript, "\tvar: %d", var);
	debugC(kDebugScript, "\tbitmask: 0x%02X", bitmask);

	if (line & bitmask) {
		// Unset button
		for (uint i = 4795; i >= 4779; i--) {
			_vm->_gfx->copyImageToScreen(i, _invokingResource->getRect());
			_vm->_system->updateScreen();
		}
		_fireplaceLines[var - 17] &= ~bitmask;
	} else {
		// Set button
		for (uint i = 4779; i <= 4795; i++) {
			_vm->_gfx->copyImageToScreen(i, _invokingResource->getRect());
			_vm->_system->updateScreen();
		}
		_fireplaceLines[var - 17] |= bitmask;
	}
}

void Myst::o_fireplaceRotation(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Myst Card 4162 and 4166 (Fireplace Puzzle Rotation Movies)
	uint16 movieNum = argv[0];
	debugC(kDebugScript, "Opcode %d: Play Fireplace Puzzle Rotation Movies", op);
	debugC(kDebugScript, "\tmovieNum: %d", movieNum);

	if (movieNum)
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("fpout", kMystStack), 167, 4);
	else
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("fpin", kMystStack), 167, 4);
}

void Myst::o_courtyardBoxesCheckSolution(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 soundId = argv[0];

	debugC(kDebugScript, "Opcode %d: Ship Puzzle Logic", op);
	debugC(kDebugScript, "\tsoundId: %d", soundId);

	// Change ship state if the boxes are correctly enabled
	if (_state.courtyardImageBoxes == 50 && !_state.shipFloating) {
		_vm->_cursor->hideCursor();
		_state.shipFloating = 1;
		_vm->_sound->playSoundBlocking(soundId);
		_vm->_cursor->showCursor();
	} else if (_state.courtyardImageBoxes != 50 && _state.shipFloating) {
		_vm->_cursor->hideCursor();
		_state.shipFloating = 0;
		_vm->_sound->playSoundBlocking(soundId);
		_vm->_cursor->showCursor();
	}
}

void Myst::o_towerRotationStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_towerRotationBlinkLabel = false;
	_towerRotationMapClicked = true;
	_towerRotationSpeed = 0;

	_vm->_cursor->setCursor(700);

	const Common::Point center = Common::Point(383, 124);
	Common::Point end = towerRotationMapComputeCoords(center, _state.towerRotationAngle);
	towerRotationMapComputeAngle();
	towerRotationMapDrawLine(center, end);

	_vm->_sound->replaceSoundMyst(5378, Audio::Mixer::kMaxChannelVolume, true);
}

void Myst::o_towerRotationEnd(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_towerRotationMapClicked = false;

	// Set angle value to expected value
	if (_state.towerRotationAngle >= 265
			&& _state.towerRotationAngle <= 277
			&& _state.rocketshipMarkerSwitch) {
		_state.towerRotationAngle = 271;
	} else if (_state.towerRotationAngle >= 77
			&& _state.towerRotationAngle <= 89
			&& _state.gearsMarkerSwitch) {
		_state.towerRotationAngle = 83;
	} else if (_state.towerRotationAngle >= 123
			&& _state.towerRotationAngle <= 135
			&& _state.dockMarkerSwitch) {
		_state.towerRotationAngle = 129;
	} else if (_state.towerRotationAngle >= 146
			&& _state.towerRotationAngle <= 158
			&& _state.cabinMarkerSwitch) {
		_state.towerRotationAngle = 152;
	}

	_vm->_sound->replaceSoundMyst(6378);

	_towerRotationBlinkLabel = true;
	_towerRotationBlinkLabelCount = 0;
}

void Myst::o_imagerChangeSelection(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Dock imager change selection", op);

	if (_imagerValidationStep != 10) {
		_imagerValidationStep = 0;

		int16 signedValue = argv[0];
		uint16 d1 = (_state.imagerSelection / 10) % 10;
		uint16 d2 = _state.imagerSelection % 10;

		if (var == 35 && signedValue > 0 && d1 < 9)
			d1++;
		else if (var == 35 && signedValue < 0 && d1 > 0)
			d1--;
		else if (var == 36 && signedValue > 0 && d2 < 9)
			d2++;
		else if (var == 36 && signedValue < 0 && d2 > 0)
			d2--;

		_state.imagerSelection = 10 * d1 + d2;
		_state.imagerActive = 0;

		_vm->redrawArea(var);
	}
}

void Myst::o_dockVaultOpen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Myst 4143 (Dock near Marker Switch)
	uint16 soundId = argv[0];
	uint16 delay = argv[1];
	uint16 directionalUpdateDataSize = argv[2];

	debugC(kDebugScript, "Opcode %d: Vault Open Logic", op);
	debugC(kDebugScript, "\tsoundId: %d", soundId);
	debugC(kDebugScript, "\tdirectionalUpdateDataSize: %d", directionalUpdateDataSize);

	if ((_state.cabinMarkerSwitch == 1) &&
		(_state.clockTowerMarkerSwitch == 1) &&
		(_state.dockMarkerSwitch == 0) &&
		(_state.gearsMarkerSwitch == 1) &&
		(_state.generatorMarkerSwitch == 1) &&
		(_state.observatoryMarkerSwitch == 1) &&
		(_state.poolMarkerSwitch == 1) &&
		(_state.rocketshipMarkerSwitch == 1)) {
		if (_globals.heldPage != 13 && _globals.ending != 4)
			_dockVaultState = 2;
		else
			_dockVaultState = 1;

		_vm->_sound->replaceSoundMyst(soundId);
		_vm->redrawArea(41, false);
		animatedUpdate(directionalUpdateDataSize, &argv[3], delay);
	}
}

void Myst::o_dockVaultClose(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Myst 4143 (Dock near Marker Switch)
	uint16 soundId = argv[0];
	uint16 delay = argv[1];
	uint16 directionalUpdateDataSize = argv[2];

	debugC(kDebugScript, "Opcode %d: Vault Close Logic", op);
	debugC(kDebugScript, "\tsoundId: %d", soundId);
	debugC(kDebugScript, "\tdirectionalUpdateDataSize: %d", directionalUpdateDataSize);

	if ((_state.cabinMarkerSwitch == 1) &&
		(_state.clockTowerMarkerSwitch == 1) &&
		(_state.dockMarkerSwitch == 1) &&
		(_state.gearsMarkerSwitch == 1) &&
		(_state.generatorMarkerSwitch == 1) &&
		(_state.observatoryMarkerSwitch == 1) &&
		(_state.poolMarkerSwitch == 1) &&
		(_state.rocketshipMarkerSwitch == 1)) {
		if (_dockVaultState == 1 || _dockVaultState == 2)
			_dockVaultState = 0;

		_vm->_sound->replaceSoundMyst(soundId);
		_vm->redrawArea(41, false);
		animatedUpdate(directionalUpdateDataSize, &argv[3], delay);
	}
}

void Myst::o_bookGivePage(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 cardIdLose = argv[0];
	uint16 cardIdBookCover = argv[1];
	uint16 soundIdAddPage = argv[2];

	debugC(kDebugScript, "Opcode %d: Red and Blue Book/Page Interaction", op);
	debugC(kDebugScript, "Var: %d", var);
	debugC(kDebugScript, "Card Id (Lose): %d", cardIdLose);
	debugC(kDebugScript, "Card Id (Book Cover): %d", cardIdBookCover);
	debugC(kDebugScript, "SoundId (Add Page): %d", soundIdAddPage);

	// No page or white page
	if (!_globals.heldPage || _globals.heldPage == 13) {
		_vm->changeToCard(cardIdBookCover, true);
		return;
	}

	uint16 bookVar = 101;
	uint16 mask = 0;

	switch (_globals.heldPage) {
	case 7:
		bookVar = 100;
	case 1:
		mask = 1;
		break;
	case 8:
		bookVar = 100;
	case 2:
		mask = 2;
		break;
	case 9:
		bookVar = 100;
	case 3:
		mask = 4;
		break;
	case 10:
		bookVar = 100;
	case 4:
		mask = 8;
		break;
	case 11:
		bookVar = 100;
	case 5:
		mask = 16;
		break;
	case 12:
		bookVar = 100;
	case 6:
		mask = 32;
		break;
	}

	// Wrong book
	if (bookVar != var) {
		_vm->changeToCard(cardIdBookCover, true);
		return;
	}

	_vm->_cursor->hideCursor();
	_vm->_sound->playSoundBlocking(soundIdAddPage);
	_vm->setMainCursor(kDefaultMystCursor);

	// Add page to book
	if (var == 100)
		_globals.redPagesInBook |= mask;
	else
		_globals.bluePagesInBook |= mask;

	// Remove page from hand
	_globals.heldPage = 0;

	_vm->_cursor->showCursor();

	if (mask == 32) {
		// You lose!
		if (var == 100)
			_globals.currentAge = 9;
		else
			_globals.currentAge = 10;

		_vm->changeToCard(cardIdLose, true);
	} else {
		_vm->changeToCard(cardIdBookCover, true);
	}
}

void Myst::o_clockWheelsExecute(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4006 (Clock Tower Time Controls)
	uint16 soundId = argv[0];

	debugC(kDebugScript, "Opcode %d: Clock Tower Bridge Puzzle Execute Button", op);

	// Correct time is 2:40
	bool correctTime = _state.clockTowerHourPosition == 2
						&& _state.clockTowerMinutePosition == 40;

	if (!_state.clockTowerBridgeOpen && correctTime) {
		_vm->_sound->replaceSoundMyst(soundId);
		_vm->_system->delayMillis(500);

		// Gears rise up
		VideoHandle gears = _vm->_video->playMovie(_vm->wrapMovieFilename("gears", kMystStack), 305, 33);
		_vm->_video->setVideoBounds(gears, Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 650, 600));
		_vm->_video->waitUntilMovieEnds(gears);


		_state.clockTowerBridgeOpen = 1;
		_vm->redrawArea(12);
	} else if (_state.clockTowerBridgeOpen && !correctTime) {
		_vm->_sound->replaceSoundMyst(soundId);
		_vm->_system->delayMillis(500);

		// Gears sink down
		VideoHandle gears = _vm->_video->playMovie(_vm->wrapMovieFilename("gears", kMystStack), 305, 33);
		_vm->_video->setVideoBounds(gears, Audio::Timestamp(0, 700, 600), Audio::Timestamp(0, 1300, 600));
		_vm->_video->waitUntilMovieEnds(gears);

		_state.clockTowerBridgeOpen = 0;
		_vm->redrawArea(12);
	}
}

void Myst::o_imagerPlayButton(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Imager play button", op);

	uint16 video = getVar(51);

	// Press button
	_vm->_sound->replaceSoundMyst(4698);

	Common::Rect src = Common::Rect(0, 0, 32, 75);
	Common::Rect dest = Common::Rect(261, 257, 293, 332);
	_vm->_gfx->copyImageSectionToScreen(4699, src, dest);
	_vm->_system->updateScreen();

	_vm->_system->delayMillis(200);

	_vm->_gfx->copyBackBufferToScreen(dest);
	_vm->_system->updateScreen();

	_vm->_cursor->hideCursor();


	// Play selected video
	if (!_state.imagerActive && video != 3)
		_vm->_sound->replaceSoundMyst(argv[0]);

	switch (video) {
	case 0: // Nothing
	case 3: // Atrus
	case 4: // Marker
		_imagerMovie->playMovie();
		break;
	case 1: // Mountain
		if (_state.imagerActive) {
			// Mountains disappearing
			Common::String file = _vm->wrapMovieFilename("vltmntn", kMystStack);
			VideoHandle mountain = _vm->_video->playMovie(file, 159, 96, false);
			_vm->_video->setVideoBounds(mountain, Audio::Timestamp(0, 11180, 600), Audio::Timestamp(0, 16800, 600));

			_state.imagerActive = 0;
		} else {
			// Mountains appearing
			Common::String file = _vm->wrapMovieFilename("vltmntn", kMystStack);
			VideoHandle mountain = _vm->_video->playMovie(file, 159, 96, false);
			_vm->_video->setVideoBounds(mountain, Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 11180, 600));

			_state.imagerActive = 1;
		}
		break;
	case 2: // Water
		_imagerMovie->setBlocking(false);

		if (_state.imagerActive) {
			_vm->_sound->replaceSoundMyst(argv[1]);

			// Water disappearing
			VideoHandle water = _imagerMovie->playMovie();
			_vm->_video->setVideoBounds(water, Audio::Timestamp(0, 4204, 600), Audio::Timestamp(0, 6040, 600));
			_vm->_video->setVideoLooping(water, false);

			_state.imagerActive = 0;
		} else {
			// Water appearing
			VideoHandle water = _imagerMovie->playMovie();
			_vm->_video->setVideoBounds(water, Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 1814, 600));
			_vm->_video->waitUntilMovieEnds(water);

			// Water looping
			water = _imagerMovie->playMovie();
			_vm->_video->setVideoBounds(water, Audio::Timestamp(0, 1814, 600), Audio::Timestamp(0, 4204, 600));
			_vm->_video->setVideoLooping(water, true);

			_state.imagerActive = 1;
		}
		break;
	}

	_vm->_cursor->showCursor();
}

void Myst::o_imagerEraseButton(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Imager erase button", op);

	_imagerRedButton = static_cast<MystResourceType8 *>(_invokingResource->_parent);
	for (uint i = 0; i < 4; i++)
		_imagerSound[i] = argv[i];
	_imagerValidationCard = argv[4];

	if (_imagerValidationStep == 0) {
		// Validation script is not running, run it
		_startTime = _vm->_system->getMillis() + 100;
		_imagerValidationRunning = true;
		return;
	} else if (_imagerValidationStep < 7) {
		// Too early
		_vm->_sound->playSoundBlocking(_imagerSound[2]);
		_imagerValidationStep = 0;
		return;
	} else if (_imagerValidationStep < 11) {
		_vm->_sound->playSoundBlocking(_imagerSound[3]);

		// Erase selected video from imager
		switch (_state.imagerSelection) {
		case 8:
			_state.imagerAtrusErased = 1;
			break;
		case 40:
			_state.imagerMountainErased = 1;
			break;
		case 47:
			_state.imagerMarkerErased = 1;
			break;
		case 67:
			_state.imagerWaterErased = 1;
			break;
		}

		_state.imagerActive = 0;
		_imagerValidationStep = 0;
		return;
	} else if (_imagerValidationStep == 11) {
		// Too late
		_imagerValidationStep = 0;
		return;
	}
}

void Myst::imagerValidation_run() {
	uint32 time = _vm->_system->getMillis();

	if (time > _startTime) {
		_imagerRedButton->drawConditionalDataToScreen(1);

		if (_imagerValidationStep < 6)
			_vm->_sound->replaceSoundMyst(_imagerSound[0]);
		else if (_imagerValidationStep < 10)
			_vm->_sound->replaceSoundMyst(_imagerSound[1]);
		else if (_imagerValidationStep == 10)
			_vm->_sound->replaceSoundMyst(_imagerSound[2]);

		_imagerValidationStep++;

		_vm->_system->delayMillis(50);

		_imagerRedButton->drawConditionalDataToScreen(0);

		if (_imagerValidationStep == 11) {
			_imagerValidationStep = 0;
			_vm->changeToCard(_imagerValidationCard, true);
		} else {
			_startTime = time + 100;
		}
	}
}

void Myst::o_towerElevatorAnimation(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Tower elevator animation", op);

	_treeStopped = true;

	_vm->_cursor->hideCursor();
	_vm->_sound->stopSound();
	_vm->_sound->pauseBackgroundMyst();

	switch (argv[0]) {
	case 0:
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("libdown", kMystStack), 216, 78);
		break;
	case 1:
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("libup", kMystStack), 216, 78);
		break;
	default:
		break;
	}

	_vm->_sound->resumeBackgroundMyst();
	_vm->_cursor->showCursor();
	_treeStopped = false;
}

void Myst::o_generatorButtonPressed(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generator button pressed", op);

	MystResource *button = _invokingResource->_parent;

	generatorRedrawRocket();

	_generatorVoltage = _state.generatorVoltage;

	uint16 mask = 0;
	uint16 value = 0;
	generatorButtonValue(button, mask, value);

	// Button pressed
	if (_state.generatorButtons & mask) {
		_state.generatorButtons &= ~mask;
		_state.generatorVoltage -= value;

		if (_state.generatorVoltage)
			_vm->_sound->replaceSoundMyst(8297);
		else {
			_vm->_sound->replaceSoundMyst(9297);
			_vm->_sound->stopBackgroundMyst();
		}
	} else {
		if (_generatorVoltage)
			_vm->_sound->replaceSoundMyst(6297);
		else {
			_vm->_sound->replaceSoundMyst(7297); // TODO: Replace with play sound and replace background 4297
			_vm->_sound->replaceBackgroundMyst(4297);
		}

		_state.generatorButtons |= mask;
		_state.generatorVoltage += value;
	}

	// Redraw button
	_vm->redrawArea(button->getType8Var());

	// Blow breaker
	if (_state.generatorVoltage > 59)
		_state.generatorBreakers = _vm->_rnd->getRandomNumberRng(1, 2);
}

void Myst::generatorRedrawRocket() {
	_vm->redrawArea(64);
	_vm->redrawArea(65);
	_vm->redrawArea(97);
}

void Myst::generatorButtonValue(MystResource *button, uint16 &mask, uint16 &value) {
	switch (button->getType8Var()) {
	case 52: // Generator Switch #1
		mask = 1;
		value = 10;
		break;
	case 53: // Generator Switch #2
		mask = 2;
		value = 7;
		break;
	case 54: // Generator Switch #3
		mask = 4;
		value = 8;
		break;
	case 55: // Generator Switch #4
		mask = 8;
		value = 16;
		break;
	case 56: // Generator Switch #5
		mask = 16;
		value = 5;
		break;
	case 57: // Generator Switch #6
		mask = 32;
		value = 1;
		break;
	case 58: // Generator Switch #7
		mask = 64;
		value = 2;
		break;
	case 59: // Generator Switch #8
		mask = 128;
		value = 22;
		break;
	case 60: // Generator Switch #9
		mask = 256;
		value = 19;
		break;
	case 61: // Generator Switch #10
		mask = 512;
		value = 9;
		break;
	}
}

void Myst::o_cabinSafeChangeDigit(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Cabin safe change digit", op);

	uint16 d1 = _state.cabinSafeCombination / 100;
	uint16 d2 = (_state.cabinSafeCombination / 10) % 10;
	uint16 d3 = _state.cabinSafeCombination % 10;

	if (var == 67)
		d1 = (d1 + 1) % 10;
	else if (var == 68)
		d2 = (d2 + 1) % 10;
	else
		d3 = (d3 + 1) % 10;

	_state.cabinSafeCombination = 100 * d1 + 10 * d2 + d3;

	_vm->redrawArea(var);
}

void Myst::o_cabinSafeHandleStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Cabin safe handle start move", op);

	// Used on Card 4100
	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	handle->drawFrame(0);
	_vm->_cursor->setCursor(700);
	_tempVar = 0;
}

void Myst::o_cabinSafeHandleMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Cabin safe handle move", op);

	// Used on Card 4100
	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);

	if (handle->pullLeverV()) {
		// Sound not played yet
		if (_tempVar == 0) {
			uint16 soundId = handle->getList2(0);
			if (soundId)
				_vm->_sound->replaceSoundMyst(soundId);
		}
		// Combination is right
		if (_state.cabinSafeCombination == 724) {
			uint16 soundId = handle->getList2(1);
			if (soundId)
				_vm->_sound->replaceSoundMyst(soundId);

			_vm->changeToCard(4103, false);

			Common::Rect screenRect = Common::Rect(544, 333);
			_vm->_gfx->runTransition(0, screenRect, 2, 5);
		}
		_tempVar = 1;
	} else {
		_tempVar = 0;
	}
}

void Myst::o_cabinSafeHandleEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Cabin safe handle end move", op);

	// Used on Card 4100
	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	handle->drawFrame(0);
	_vm->checkCursorHints();
}

void Myst::o_observatoryMonthChangeStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Observatory month change start", op);

	_vm->_sound->pauseBackgroundMyst();

	if (op == 129 || op == 131) {
		// Increase
		if (observatoryIsDDMMYYYY2400())
			_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(36, 0, 48, 9), Common::Rect(351, 70, 363, 79));
		else
			_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(0, 0, 12, 9), Common::Rect(315, 70, 327, 79));

		_observatoryIncrement = -1;
	} else {
		// Decrease
		if (observatoryIsDDMMYYYY2400())
			_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(36, 0, 48, 9), Common::Rect(351, 204, 363, 213));
		else
			_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(0, 0, 12, 9), Common::Rect(315, 204, 327, 213));

		_observatoryIncrement = 1;
	}

	// Highlight slider
	_observatoryMonthSlider->drawConditionalDataToScreen(2);
	_observatoryCurrentSlider = _observatoryMonthSlider;

	// First increment
	observatoryIncrementMonth(_observatoryIncrement);

	// Start persistent script
	_startTime = _vm->_system->getMillis();
	_observatoryMonthChanging = true;
}

void Myst::observatoryIncrementMonth(int16 increment) {
	int16 newMonth = _state.observatoryMonthSetting + increment;

	if (newMonth >= 0 && newMonth <= 11) {
		_state.observatoryMonthSetting = newMonth;

		// Redraw digits
		_vm->redrawArea(73);

		// Update slider
		_observatoryMonthSlider->setPosition(94 + 94 * _state.observatoryMonthSetting / 11);
		_observatoryMonthSlider->restoreBackground();
		_observatoryMonthSlider->drawConditionalDataToScreen(2);
		_state.observatoryMonthSlider = _observatoryMonthSlider->_pos.y;
	}

	_vm->_sound->replaceSoundMyst(8500);
}

void Myst::observatoryMonthChange_run() {
	if (_startTime + 500 < _vm->_system->getMillis())
		observatoryIncrementMonth(_observatoryIncrement);
}

void Myst::o_observatoryDayChangeStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Observatory day change start", op);

	_vm->_sound->pauseBackgroundMyst();

	if (op == 129 || op == 131) {
		// Increase
		if (observatoryIsDDMMYYYY2400())
			_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(0, 0, 12, 9), Common::Rect(315, 70, 327, 79));
		else
			_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(36, 0, 48, 9), Common::Rect(351, 70, 363, 79));

		_observatoryIncrement = -1;
	} else {
		// Decrease
		if (observatoryIsDDMMYYYY2400())
			_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(0, 0, 12, 9), Common::Rect(315, 204, 327, 213));
		else
			_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(36, 0, 48, 9), Common::Rect(351, 204, 363, 213));

		_observatoryIncrement = 1;
	}

	// Highlight slider
	_observatoryDaySlider->drawConditionalDataToScreen(2);
	_observatoryCurrentSlider = _observatoryDaySlider;

	// First increment
	observatoryIncrementDay(_observatoryIncrement);

	// Start persistent script
	_startTime = _vm->_system->getMillis();
	_observatoryDayChanging = true;
}

void Myst::observatoryIncrementDay(int16 increment) {
	int16 newDay = _state.observatoryDaySetting + increment;

	if (newDay >= 1 && newDay <= 31) {
		_state.observatoryDaySetting = newDay;

		// Redraw digits
		_vm->redrawArea(75);
		_vm->redrawArea(74);

		// Update slider
		_observatoryDaySlider->setPosition(91 + 3 * _state.observatoryDaySetting);
		_observatoryDaySlider->restoreBackground();
		_observatoryDaySlider->drawConditionalDataToScreen(2);
		_state.observatoryDaySlider = _observatoryDaySlider->_pos.y;
	}

	_vm->_sound->replaceSoundMyst(8500);
}

void Myst::observatoryDayChange_run() {
	if (_startTime + 500 < _vm->_system->getMillis())
		observatoryIncrementDay(_observatoryIncrement);
}

void Myst::o_observatoryYearChangeStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Observatory year change start", op);

	_vm->_sound->pauseBackgroundMyst();

	if (op == 196) {
		// Increase
		_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(72, 0, 84, 9), Common::Rect(387, 70, 399, 79));
		_observatoryIncrement = -1;
	} else {
		// Decrease
		_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(72, 0, 84, 9), Common::Rect(387, 204, 399, 213));
		_observatoryIncrement = 1;
	}

	// Highlight slider
	_observatoryYearSlider->drawConditionalDataToScreen(2);
	_observatoryCurrentSlider = _observatoryYearSlider;

	// First increment
	observatoryIncrementYear(_observatoryIncrement);

	// Start persistent script
	_startTime = _vm->_system->getMillis();
	_observatoryYearChanging = true;
}

void Myst::observatoryIncrementYear(int16 increment) {
	int16 newYear = _state.observatoryYearSetting + increment;

	if (newYear >= 0 && newYear <= 9999) {
		_state.observatoryYearSetting = newYear;

		// Redraw digits
		_vm->redrawArea(79);
		_vm->redrawArea(78);
		_vm->redrawArea(77);
		_vm->redrawArea(76);

		// Update slider
		_observatoryYearSlider->setPosition(94 + 94 * _state.observatoryYearSetting / 9999);
		_observatoryYearSlider->restoreBackground();
		_observatoryYearSlider->drawConditionalDataToScreen(2);
		_state.observatoryYearSlider = _observatoryYearSlider->_pos.y;
	}

	_vm->_sound->replaceSoundMyst(8500);
}

void Myst::observatoryYearChange_run() {
	if (_startTime + 500 < _vm->_system->getMillis())
		observatoryIncrementYear(_observatoryIncrement);
}

void Myst::o_observatoryTimeChangeStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Observatory time change start", op);

	_vm->_sound->pauseBackgroundMyst();

	if (op == 192) {
		// Increase
		_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(109, 0, 121, 9), Common::Rect(424, 70, 436, 79));
		_observatoryIncrement = -1;
	} else {
		// Decrease
		_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(109, 0, 121, 9), Common::Rect(424, 204, 436, 213));
		_observatoryIncrement = 1;
	}

	// Highlight slider
	_observatoryTimeSlider->drawConditionalDataToScreen(2);
	_observatoryCurrentSlider = _observatoryTimeSlider;

	// First increment
	observatoryIncrementTime(_observatoryIncrement);

	// Start persistent script
	_startTime = _vm->_system->getMillis();
	_observatoryTimeChanging = true;
}

void Myst::observatoryIncrementTime(int16 increment) {
	int16 newTime = _state.observatoryTimeSetting + increment;

	if (newTime >= 0 && newTime <= 1439) {
		_state.observatoryTimeSetting = newTime;

		// Redraw digits
		_vm->redrawArea(80);
		_vm->redrawArea(81);
		_vm->redrawArea(82);
		_vm->redrawArea(83);

		// Draw AM/PM
		if (!observatoryIsDDMMYYYY2400()) {
			_vm->redrawArea(88);
		}

		// Update slider
		_observatoryTimeSlider->setPosition(94 + 94 * _state.observatoryTimeSetting / 1439);
		_observatoryTimeSlider->restoreBackground();
		_observatoryTimeSlider->drawConditionalDataToScreen(2);
		_state.observatoryTimeSlider = _observatoryTimeSlider->_pos.y;
	}

	_vm->_sound->replaceSoundMyst(8500);
}

void Myst::observatoryTimeChange_run() {
	if (_startTime + 500 < _vm->_system->getMillis())
		observatoryIncrementTime(_observatoryIncrement);
}

void Myst::o_observatoryGoButton(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Observatory go button", op);

	// Setting not at target
	if (_state.observatoryDayTarget != _state.observatoryDaySetting
			|| _state.observatoryMonthTarget != _state.observatoryMonthSetting
			|| _state.observatoryYearTarget != _state.observatoryYearSetting
			|| _state.observatoryTimeTarget != _state.observatoryTimeSetting) {
		uint16 soundId = argv[0];
		_vm->_sound->replaceSoundMyst(soundId);

		int16 distance = _state.observatoryYearTarget - _state.observatoryYearSetting;
		uint32 end = _vm->_system->getMillis() + 32 * ABS(distance) / 50 + 800;

		while (end > _vm->_system->getMillis()) {
			_vm->_system->delayMillis(50);

			observatoryUpdateVisualizer(_vm->_rnd->getRandomNumber(409), _vm->_rnd->getRandomNumber(409));

			_vm->redrawResource(_observatoryVisualizer);
		}

		_vm->_sound->resumeBackgroundMyst();

		// Redraw visualizer
		observatorySetTargetToSetting();
		_vm->redrawResource(_observatoryVisualizer);

		// Redraw button
		_tempVar = 0;
		_vm->redrawArea(105);
	}
}

void Myst::o_observatoryMonthSliderMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Month slider move", op);

	observatoryUpdateMonth();
}

void Myst::o_observatoryDaySliderMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Day slider move", op);

	observatoryUpdateDay();
}

void Myst::o_observatoryYearSliderMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Year slider move", op);

	observatoryUpdateYear();
}

void Myst::o_observatoryTimeSliderMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Time slider move", op);

	observatoryUpdateTime();
}

void Myst::o_circuitBreakerStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Circuit breaker start move", op);

	MystResourceType12 *breaker = static_cast<MystResourceType12 *>(_invokingResource);
	breaker->drawFrame(0);
	_vm->_cursor->setCursor(700);
	_tempVar = 0;
}

void Myst::o_circuitBreakerMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Circuit breaker move", op);

	MystResourceType12 *breaker = static_cast<MystResourceType12 *>(_invokingResource);
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	int16 maxStep = breaker->getStepsV() - 1;
	int16 step = ((mouse.y - 80) * breaker->getStepsV()) / 65;
	step = CLIP<uint16>(step, 0, maxStep);

	breaker->drawFrame(step);

	if (_tempVar != step) {
		_tempVar = step;

		// Breaker switched
		if (step == maxStep) {
			// Choose breaker
			if (breaker->getType8Var() == 93) {
				// Voltage is still too high or not broken
				if (_state.generatorVoltage > 59 || _state.generatorBreakers != 1) {
					uint16 soundId = breaker->getList2(1);
					if (soundId)
						_vm->_sound->replaceSoundMyst(soundId);
				} else {
					uint16 soundId = breaker->getList2(0);
					if (soundId)
						_vm->_sound->replaceSoundMyst(soundId);

					// Reset breaker state
					_state.generatorBreakers = 0;
				}
			} else {
				// Voltage is still too high or not broken
				if (_state.generatorVoltage > 59 || _state.generatorBreakers != 2) {
					uint16 soundId = breaker->getList2(1);
					if (soundId)
						_vm->_sound->replaceSoundMyst(soundId);
				} else {
					uint16 soundId = breaker->getList2(0);
					if (soundId)
						_vm->_sound->replaceSoundMyst(soundId);

					// Reset breaker state
					_state.generatorBreakers = 0;
				}
			}
		}
	}
}

void Myst::o_circuitBreakerEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Circuit breaker end move", op);

	MystResourceType12 *breaker = static_cast<MystResourceType12 *>(_invokingResource);
	_vm->redrawArea(breaker->getType8Var());
	_vm->checkCursorHints();
}

void Myst::o_boilerIncreasePressureStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Boiler increase pressure start", op);

	_treeStopped = true;
	if (_state.cabinValvePosition < 25)
		_vm->_sound->stopBackgroundMyst();

	_boilerPressureIncreasing = true;
}

void Myst::o_boilerLightPilot(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Boiler light pilot", op);

	// Match is lit
	if (_cabinMatchState == 1) {
		_state.cabinPilotLightLit = 1;
		_vm->redrawArea(98);

		// Put out match
		_matchGoOutTime = _vm->_system->getMillis();

		if (_state.cabinValvePosition > 0)
			_vm->_sound->replaceBackgroundMyst(8098, 49152);

		if (_state.cabinValvePosition > 12)
			_state.treeLastMoveTime = _vm->_system->getMillis();

		// TODO: Complete. Play movies
	}
}

void Myst::o_boilerIncreasePressureStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Boiler increase pressure stop", op);

	_treeStopped = false;
	_boilerPressureIncreasing = false;
	_state.treeLastMoveTime = _vm->_system->getMillis();

	if (_state.cabinPilotLightLit == 1) {
		if (_state.cabinValvePosition > 0)
			_vm->_sound->replaceBackgroundMyst(8098, 49152);

		// TODO: Play movies
	} else if (_state.cabinValvePosition > 0)
		_vm->_sound->replaceBackgroundMyst(4098, _state.cabinValvePosition << 10);
}

void Myst::boilerPressureIncrease_run() {
	// Allow increasing pressure if sound has stopped
	if (!_vm->_sound->isPlaying(5098) && _state.cabinValvePosition < 25) {
		_state.cabinValvePosition++;
		if (_state.cabinValvePosition == 1) {
			// TODO: Play fire movie

			// Draw fire
			_vm->redrawArea(305);
		} else if (_state.cabinValvePosition == 25) {
			if (_state.cabinPilotLightLit == 1)
				_vm->_sound->replaceBackgroundMyst(8098, 49152);
			else
				_vm->_sound->replaceBackgroundMyst(4098, 25600);
		}

		// Pressure increasing sound
		_vm->_sound->replaceSoundMyst(5098);

		// Redraw wheel
		_vm->redrawArea(99);
	}
}

void Myst::boilerPressureDecrease_run() {
	// Allow decreasing pressure if sound has stopped
	if (!_vm->_sound->isPlaying(5098) && _state.cabinValvePosition > 0) {
		_state.cabinValvePosition--;
		if (_state.cabinValvePosition == 0) {
			// TODO: Play fire movie

			// Draw fire
			_vm->redrawArea(305);
		}

		// Pressure increasing sound
		_vm->_sound->replaceSoundMyst(5098);

		// Redraw wheel
		_vm->redrawArea(99);
	}
}

void Myst::o_boilerDecreasePressureStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Boiler decrease pressure start", op);

	_treeStopped = true;
	_vm->_sound->stopBackgroundMyst();

	_boilerPressureDecreasing = true;
}

void Myst::o_boilerDecreasePressureStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Boiler decrease pressure stop", op);

	_treeStopped = false;
	_boilerPressureDecreasing = false;
	_state.treeLastMoveTime = _vm->_system->getMillis();

	if (_state.cabinPilotLightLit == 1) {
		if (_state.cabinValvePosition > 0)
			_vm->_sound->replaceBackgroundMyst(8098, 49152);

		// TODO: Play movies
	} else {
		if (_state.cabinValvePosition > 0)
			_vm->_sound->replaceBackgroundMyst(4098, _state.cabinValvePosition << 10);
	}
}

void Myst::o_basementIncreasePressureStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Basement increase pressure start", op);

	_treeStopped = true;
	_basementPressureIncreasing = true;
}

void Myst::o_basementIncreasePressureStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Basement increase pressure stop", op);

	_treeStopped = false;
	_basementPressureIncreasing = false;
	_state.treeLastMoveTime = _vm->_system->getMillis();
}

void Myst::basementPressureIncrease_run() {
	// Allow increasing pressure if sound has stopped
	if (!_vm->_sound->isPlaying(4642) && _state.cabinValvePosition < 25) {
		_state.cabinValvePosition++;

		// Pressure increasing sound
		_vm->_sound->replaceSoundMyst(4642);

		// Redraw wheel
		_vm->redrawArea(99);
	}
}

void Myst::basementPressureDecrease_run() {
	// Allow decreasing pressure if sound has stopped
	if (!_vm->_sound->isPlaying(4642) && _state.cabinValvePosition > 0) {
		_state.cabinValvePosition--;

		// Pressure decreasing sound
		_vm->_sound->replaceSoundMyst(4642);

		// Redraw wheel
		_vm->redrawArea(99);
	}
}

void Myst::o_basementDecreasePressureStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Basement decrease pressure start", op);

	_treeStopped = true;
	_basementPressureDecreasing = true;
}

void Myst::o_basementDecreasePressureStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Basement decrease pressure stop", op);

	_treeStopped = false;
	_basementPressureDecreasing = false;
	_state.treeLastMoveTime = _vm->_system->getMillis();
}

void Myst::tree_run() {
	uint16 pressure;
	if (_state.cabinPilotLightLit)
		pressure = _state.cabinValvePosition;
	else
		pressure = 0;

	// 12 means tree is balanced
	if (pressure != 12) {
		bool goingDown = true;
		if (pressure >= 12)
			goingDown = false;

		// Tree is within bounds
		if ((_state.treePosition < 12 && !goingDown)
				|| (_state.treePosition > _treeMinPosition && goingDown)) {
			uint16 delay = treeNextMoveDelay(pressure);
			uint32 time = _vm->_system->getMillis();
			if (delay < time - _state.treeLastMoveTime) {

				// Tree movement
				if (goingDown) {
					_state.treePosition--;
					_vm->_sound->replaceSoundMyst(2);
				} else {
					_state.treePosition++;
					_vm->_sound->replaceSoundMyst(1);
				}

				// Stop background music if going up from book room
				if (_vm->getCurCard() == 4630) {
					if (_state.treePosition > 0)
						_vm->_sound->stopBackgroundMyst();
					else
						_vm->_sound->replaceBackgroundMyst(4630, 24576);
				}

				// Redraw tree
				_vm->redrawArea(72);

				// Check if alcove is accessible
				treeSetAlcoveAccessible();

				_state.treeLastMoveTime = time;
			}
		}
	}
}

void Myst::treeSetAlcoveAccessible() {
	if (_treeAlcove) {
		// Make alcove accessible if the tree is in the correct position
		_treeAlcove->setEnabled(_state.treePosition >= _treeMinAccessiblePosition
					&& _state.treePosition <= _treeMaxAccessiblePosition);
	}
}

uint32 Myst::treeNextMoveDelay(uint16 pressure) {
	if (pressure >= 12)
		return 25000 * (13 - (pressure - 12)) / 12 + 3000;
	else
		return 25000 * pressure / 13 + 3000;
}

void Myst::o_rocketSoundSliderStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket slider start move", op);

	_rocketSliderSound = 0;
	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackgroundMyst();
	rocketSliderMove();
}

void Myst::o_rocketSoundSliderMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket slider move", op);

	rocketSliderMove();
}

void Myst::o_rocketSoundSliderEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket slider end move", op);

	_vm->checkCursorHints();

	if (_state.generatorVoltage == 59 && !_state.generatorBreakers && _rocketSliderSound)
		_vm->_sound->stopSound();

	if (_invokingResource == _rocketSlider1)
		_state.rocketSliderPosition[0] = _rocketSlider1->_pos.y;
	else if (_invokingResource == _rocketSlider2)
		_state.rocketSliderPosition[1] = _rocketSlider2->_pos.y;
	else if (_invokingResource == _rocketSlider3)
		_state.rocketSliderPosition[2] = _rocketSlider3->_pos.y;
	else if (_invokingResource == _rocketSlider4)
		_state.rocketSliderPosition[3] = _rocketSlider4->_pos.y;
	else if (_invokingResource == _rocketSlider5)
		_state.rocketSliderPosition[4] = _rocketSlider5->_pos.y;

	_vm->_sound->resumeBackgroundMyst();
}

void Myst::rocketSliderMove() {
	MystResourceType10 *slider = static_cast<MystResourceType10 *>(_invokingResource);

	if (_state.generatorVoltage == 59 && !_state.generatorBreakers) {
		uint16 soundId = rocketSliderGetSound(slider->_pos.y);
		if (soundId != _rocketSliderSound) {
			_rocketSliderSound = soundId;
			_vm->_sound->replaceSoundMyst(soundId, Audio::Mixer::kMaxChannelVolume, true);
		}
	}
}

uint16 Myst::rocketSliderGetSound(uint16 pos) {
	return (uint16)(9530 + (pos - 216) * 35.0 / 61.0);
}

void Myst::rocketCheckSolution() {
	_vm->_cursor->hideCursor();

	uint16 soundId;
	bool solved = true;

	soundId = rocketSliderGetSound(_rocketSlider1->_pos.y);
	_vm->_sound->replaceSoundMyst(soundId);
	_rocketSlider1->drawConditionalDataToScreen(2);
	_vm->_system->delayMillis(250);
	if (soundId != 9558)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider2->_pos.y);
	_vm->_sound->replaceSoundMyst(soundId);
	_rocketSlider2->drawConditionalDataToScreen(2);
	_vm->_system->delayMillis(250);
	if (soundId != 9546)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider3->_pos.y);
	_vm->_sound->replaceSoundMyst(soundId);
	_rocketSlider3->drawConditionalDataToScreen(2);
	_vm->_system->delayMillis(250);
	if (soundId != 9543)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider4->_pos.y);
	_vm->_sound->replaceSoundMyst(soundId);
	_rocketSlider4->drawConditionalDataToScreen(2);
	_vm->_system->delayMillis(250);
	if (soundId != 9553)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider5->_pos.y);
	_vm->_sound->replaceSoundMyst(soundId);
	_rocketSlider5->drawConditionalDataToScreen(2);
	_vm->_system->delayMillis(250);
	if (soundId != 9560)
		solved = false;

	_vm->_sound->stopSound();

	if (solved) {
		// Reset lever position
		MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
		lever->drawFrame(0);

		// Book appearing
		Common::String movieFile = _vm->wrapMovieFilename("selenbok", kMystStack);
		_rocketLinkBook = _vm->_video->playMovie(movieFile, 224, 41);
		_vm->_video->setVideoBounds(_rocketLinkBook, Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 660, 600));
		_vm->_video->waitUntilMovieEnds(_rocketLinkBook);

		// Book looping closed
		_rocketLinkBook = _vm->_video->playMovie(movieFile, 224, 41, true);
		_vm->_video->setVideoBounds(_rocketLinkBook, Audio::Timestamp(0, 660, 600), Audio::Timestamp(0, 3500, 600));

		_tempVar = 1;
	}

	_rocketSlider1->drawConditionalDataToScreen(1);
	_rocketSlider2->drawConditionalDataToScreen(1);
	_rocketSlider3->drawConditionalDataToScreen(1);
	_rocketSlider4->drawConditionalDataToScreen(1);
	_rocketSlider5->drawConditionalDataToScreen(1);

	_vm->_cursor->showCursor();
}

void Myst::o_rocketPianoStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket piano start move", op);

	MystResourceType11 *key = static_cast<MystResourceType11 *>(_invokingResource);

	// What the hell??
	Common::Rect src = key->_subImages[1].rect;
	Common::Rect rect = key->_subImages[0].rect;
	Common::Rect dest = rect;
	dest.top = 332 - rect.bottom;
	dest.bottom = 332 - rect.top;

	// Draw pressed piano key
	_vm->_gfx->copyImageSectionToScreen(key->_subImages[1].wdib, src, dest);
	_vm->_system->updateScreen();

	// Play note
	if (_state.generatorVoltage == 59 && !_state.generatorBreakers) {
		uint16 soundId = key->getList1(0);
		_vm->_sound->replaceSoundMyst(soundId, Audio::Mixer::kMaxChannelVolume, true);
	}
}

void Myst::o_rocketPianoMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket piano move", op);

	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();
	Common::Rect piano = Common::Rect(85, 123, 460, 270);

	// Unpress previous key
	MystResourceType11 *key = static_cast<MystResourceType11 *>(_invokingResource);

	Common::Rect src = key->_subImages[0].rect;
	Common::Rect dest = src;
	dest.top = 332 - src.bottom;
	dest.bottom = 332 - src.top;

	// Draw unpressed piano key
	_vm->_gfx->copyImageSectionToScreen(key->_subImages[0].wdib, src, dest);

	if (piano.contains(mouse)) {
		MystResource *resource = _vm->updateCurrentResource();
		if (resource && resource->type == kMystDragArea) {
			// Press new key
			key = static_cast<MystResourceType11 *>(resource);
			src = key->_subImages[1].rect;
			Common::Rect rect = key->_subImages[0].rect;
			dest = rect;
			dest.top = 332 - rect.bottom;
			dest.bottom = 332 - rect.top;

			// Draw pressed piano key
			_vm->_gfx->copyImageSectionToScreen(key->_subImages[1].wdib, src, dest);

			// Play note
			if (_state.generatorVoltage == 59 && !_state.generatorBreakers) {
				uint16 soundId = key->getList1(0);
				_vm->_sound->replaceSoundMyst(soundId, Audio::Mixer::kMaxChannelVolume, true);
			}
		} else {
			// Not pressing a key anymore
			_vm->_sound->stopSound();
			_vm->_sound->resumeBackgroundMyst();
		}
	}

	_vm->_system->updateScreen();
}

void Myst::o_rocketPianoStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket piano end move", op);

	MystResourceType8 *key = static_cast<MystResourceType8 *>(_invokingResource);

	Common::Rect &src = key->_subImages[0].rect;
	Common::Rect dest = src;
	dest.top = 332 - src.bottom;
	dest.bottom = 332 - src.top;

	// Draw unpressed piano key
	_vm->_gfx->copyImageSectionToScreen(key->_subImages[0].wdib, src, dest);
	_vm->_system->updateScreen();

	_vm->_sound->stopSound();
	_vm->_sound->resumeBackgroundMyst();
}

void Myst::o_rocketLeverStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket lever start move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	_vm->_cursor->setCursor(700);
	_rocketLeverPosition = 0;
	lever->drawFrame(0);
}

void Myst::o_rocketOpenBook(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket open link book", op);

	// Flyby movie
	_vm->_video->setVideoBounds(_rocketLinkBook, Audio::Timestamp(0, 3500, 600), Audio::Timestamp(0, 13100, 600));

	// Set linkable
	_tempVar = 2;
}

void Myst::o_rocketLeverMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket lever move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	// Make the lever follow the mouse
	int16 maxStep = lever->getStepsV() - 1;
    Common::Rect rect = lever->getRect();
    int16 step = ((mouse.y - rect.top) * lever->getStepsV()) / rect.height();
	step = CLIP<uint16>(step, 0, maxStep);

	lever->drawFrame(step);

	// If lever pulled
	if (step == maxStep && step != _rocketLeverPosition) {
		uint16 soundId = lever->getList2(0);

		if (soundId)
			_vm->_sound->replaceSoundMyst(soundId);

		// If rocket correctly powered
		if (_state.generatorVoltage == 59 && !_state.generatorBreakers)
			rocketCheckSolution();
	}

	_rocketLeverPosition = step;
}

void Myst::o_rocketLeverEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket lever end move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	_vm->checkCursorHints();
	_rocketLeverPosition = 0;
	lever->drawFrame(0);
}

void Myst::o_cabinLeave(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Leave cabin", op);

	// If match is lit, put out
	if (_cabinMatchState == 1) {
		_matchGoOutTime = _vm->_system->getMillis();
	} else if (_cabinMatchState == 0) {
		_vm->setMainCursor(_savedCursorId);
		_cabinMatchState = 2;
	}
}

void Myst::o_treePressureReleaseStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Tree pressure release start", op);

	Common::Rect src = Common::Rect(0, 0, 49, 86);
	Common::Rect dest = Common::Rect(78, 46, 127, 132);
	_vm->_gfx->copyImageSectionToScreen(4631, src, dest);
	_vm->_system->updateScreen();

	_tempVar = _state.cabinValvePosition;

	if (_state.treePosition >= 4) {
		_state.cabinValvePosition = 0;
		_treeMinPosition = 4;
		_state.treeLastMoveTime = 0;
	}
}

void Myst::o_treePressureReleaseStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Tree pressure release stop", op);

	Common::Rect rect = Common::Rect(78, 46, 127, 132);
	_vm->_gfx->copyBackBufferToScreen(rect);
	_vm->_system->updateScreen();

	_state.cabinValvePosition = _tempVar;
	_treeMinPosition = 0;
}

void Myst::o_observatoryMonthSliderStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Month slider start move", op);

	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackgroundMyst();

	observatoryUpdateMonth();
}

void Myst::o_observatoryMonthSliderEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Month slider end move", op);

	_vm->checkCursorHints();
	_vm->_sound->resumeBackgroundMyst();

	observatoryUpdateMonth();
}

void Myst::observatoryUpdateMonth() {
	int16 month = (_observatoryMonthSlider->_pos.y - 94) / 8;

	if (month != _state.observatoryMonthSetting) {
		_state.observatoryMonthSetting = month;
		_state.observatoryMonthSlider = _observatoryMonthSlider->_pos.y;
		_vm->_sound->replaceSoundMyst(8500);

		// Redraw digits
		_vm->redrawArea(73);
	}
}

void Myst::o_observatoryDaySliderStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Day slider start move", op);

	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackgroundMyst();

	observatoryUpdateDay();
}

void Myst::o_observatoryDaySliderEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Day slider end move", op);

	_vm->checkCursorHints();
	_vm->_sound->resumeBackgroundMyst();

	observatoryUpdateDay();
}

void Myst::observatoryUpdateDay() {
	int16 day = (_observatoryDaySlider->_pos.y - 94) * 30 / 94 + 1;

	if (day != _state.observatoryDaySetting) {
		_state.observatoryDaySetting = day;
		_state.observatoryDaySlider = _observatoryDaySlider->_pos.y;
		_vm->_sound->replaceSoundMyst(8500);

		// Redraw digits
		_vm->redrawArea(75);
		_vm->redrawArea(74);
	}
}

void Myst::o_observatoryYearSliderStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Year slider start move", op);

	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackgroundMyst();

	observatoryUpdateYear();
}

void Myst::o_observatoryYearSliderEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Year slider end move", op);

	_vm->checkCursorHints();
	_vm->_sound->resumeBackgroundMyst();

	observatoryUpdateYear();
}

void Myst::observatoryUpdateYear() {
	int16 year = (_observatoryYearSlider->_pos.y - 94) * 9999 / 94;

	if (year != _state.observatoryYearSetting) {
		_state.observatoryYearSetting = year;
		_state.observatoryYearSlider = _observatoryYearSlider->_pos.y;
		_vm->_sound->replaceSoundMyst(8500);

		// Redraw digits
		_vm->redrawArea(79);
		_vm->redrawArea(78);
		_vm->redrawArea(77);
		_vm->redrawArea(76);
	}
}

void Myst::o_observatoryTimeSliderStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Time slider start move", op);

	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackgroundMyst();

	observatoryUpdateTime();
}

void Myst::o_observatoryTimeSliderEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Time slider end move", op);

	_vm->checkCursorHints();
	_vm->_sound->resumeBackgroundMyst();

	observatoryUpdateTime();
}

void Myst::observatoryUpdateTime() {
	int16 time = (_observatoryTimeSlider->_pos.y - 94) * 1439 / 94;

	if (time != _state.observatoryTimeSetting) {
		_state.observatoryTimeSetting = time;
		_state.observatoryTimeSlider = _observatoryTimeSlider->_pos.y;
		_vm->_sound->replaceSoundMyst(8500);

		// Redraw digits
		_vm->redrawArea(80);
		_vm->redrawArea(81);
		_vm->redrawArea(82);
		_vm->redrawArea(83);

		// Draw AM/PM
		if (!observatoryIsDDMMYYYY2400())
			_vm->redrawArea(88);
	}
}

void Myst::o_libraryCombinationBookStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Combination book stop turning pages", op);
	_libraryCombinationBookPagesTurning = false;
}

void Myst::o_cabinMatchLight(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (!_cabinMatchState) {
		_vm->_sound->replaceSoundMyst(4103);

		// Match is lit
		_cabinMatchState = 1;
		_matchBurning = true;
		_matchGoOutCnt = 0;
		_vm->setMainCursor(kLitMatchCursor);

		// Match will stay lit for one minute
		_matchGoOutTime = _vm->_system->getMillis() + 60 * 1000;
	}
}

void Myst::matchBurn_run() {
	uint32 time = _vm->_system->getMillis();

	if (time > _matchGoOutTime) {
		_matchGoOutTime = time + 150;

		// Switch between lit match and dead match every 150 ms when match is dying
		if (_matchGoOutCnt % 2)
			_vm->setMainCursor(kLitMatchCursor);
		else
			_vm->setMainCursor(kDeadMatchCursor);

		_matchGoOutCnt++;

		// Match is dead
		if (_matchGoOutCnt >= 5) {
			_matchBurning = false;
			_vm->setMainCursor(_savedCursorId);

			_cabinMatchState = 2;
		}
	}
}

void Myst::o_courtyardBoxEnter(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Mouse enters courtyard box", op);
	_tempVar = 1;
	_vm->_sound->playSound(_courtyardBoxSound);
	_vm->redrawArea(var);
}

void Myst::o_courtyardBoxLeave(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Mouse leaves courtyard box", op);
	_tempVar = 0;
	_vm->redrawArea(var);
}

void Myst::o_clockMinuteWheelStartTurn(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4006
	debugC(kDebugScript, "Opcode %d: Minute wheel start turn", op);

	clockWheelStartTurn(2);
}

void Myst::o_clockWheelEndTurn(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4006
	debugC(kDebugScript, "Opcode %d: Wheel end turn", op);

	_clockTurningWheel = 0;
}

void Myst::o_clockHourWheelStartTurn(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4006
	debugC(kDebugScript, "Opcode %d: Hour wheel start turn", op);

	clockWheelStartTurn(1);
}

void Myst::clockWheel_run() {
	// Turn wheel one step each second
	uint32 time = _vm->_system->getMillis();

	if (time > _startTime + 1000) {
		_startTime = time;

		if (_clockTurningWheel == 1)
			clockWheelTurn(39);
		else
			clockWheelTurn(38);

		_vm->redrawArea(37);
	}
}

void Myst::clockWheelStartTurn(uint16 wheel) {
	MystResourceType11 *resource = static_cast<MystResourceType11 *>(_invokingResource);
	uint16 soundId = resource->getList1(0);

	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);

	// Turn wheel one step
	if (wheel == 1)
		clockWheelTurn(39);
	else
		clockWheelTurn(38);

	_vm->redrawArea(37);

	// Continue turning wheel until mouse button is released
	_clockTurningWheel = wheel;
	_startTime = _vm->_system->getMillis();
}

void Myst::clockWheelTurn(uint16 var) {
	if (var == 38) {
		// Hours
		_state.clockTowerHourPosition = (_state.clockTowerHourPosition + 1) % 12;
	} else {
		// Minutes
		_state.clockTowerMinutePosition = (_state.clockTowerMinutePosition + 5) % 60;
	}
}

void Myst::o_libraryCombinationBookStartRight(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Combination book start turning pages right", op);

	_tempVar = 0;
	libraryCombinationBookTurnRight();
	_startTime = _vm->_system->getMillis();
	_libraryCombinationBookPagesTurning = true;
}

void Myst::o_libraryCombinationBookStartLeft(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Combination book start turning pages left", op);

	_tempVar = 0;
	libraryCombinationBookTurnLeft();
	_startTime = _vm->_system->getMillis();
	_libraryCombinationBookPagesTurning = true;
}

void Myst::libraryCombinationBookTurnLeft() {
	// Turn page left
	if (_libraryBookPage - 1 >= 0) {
		_tempVar--;

		if (_tempVar >= -5) {
			_libraryBookPage--;
		} else {
			_libraryBookPage -= 5;
			_tempVar = -5;
		}

		_libraryBookPage = CLIP<int16>(_libraryBookPage, 0, _libraryBookNumPages - 1);

		Common::Rect rect = Common::Rect(157, 113, 446, 220);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->replaceSoundMyst(_libraryBookSound1);
		else
			_vm->_sound->replaceSoundMyst(_libraryBookSound2);

		_vm->_system->updateScreen();
	}
}

void Myst::libraryCombinationBookTurnRight() {
	// Turn page right
	if (_libraryBookPage + 1 < _libraryBookNumPages) {
		_tempVar++;

		if (_tempVar <= 5) {
			_libraryBookPage++;
		} else {
			_libraryBookPage += 5;
			_tempVar = 5;
		}

		_libraryBookPage = CLIP<uint16>(_libraryBookPage, 0, _libraryBookNumPages - 1);

		Common::Rect rect = Common::Rect(157, 113, 446, 220);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->replaceSoundMyst(_libraryBookSound1);
		else
			_vm->_sound->replaceSoundMyst(_libraryBookSound2);

		_vm->_system->updateScreen();
	}
}

void Myst::libraryCombinationBook_run() {
	uint32 time = _vm->_system->getMillis();
	if (time >= _startTime + 500) {
		if (_tempVar > 0) {
			libraryCombinationBookTurnRight();
			_startTime = time;
		} else if (_tempVar < 0) {
			libraryCombinationBookTurnLeft();
			_startTime = time;
		}
	}
}

void Myst::o_observatoryChangeSettingStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Observatory change setting stop", op);

	// Stop persistent scripts
	_observatoryMonthChanging = false;
	_observatoryDayChanging = false;
	_observatoryYearChanging = false;
	_observatoryTimeChanging = false;
	_observatoryIncrement = 0;

	// Restore button and slider
	_vm->_gfx->copyBackBufferToScreen(_invokingResource->getRect());
	if (_observatoryCurrentSlider) {
		_vm->redrawResource(_observatoryCurrentSlider);
		_observatoryCurrentSlider = 0;
	}
	_vm->_sound->resumeBackgroundMyst();
}

void Myst::o_dockVaultForceClose(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Myst 4143 (Dock near Marker Switch)
	uint16 soundId = argv[0];
	uint16 delay = argv[1];
	uint16 directionalUpdateDataSize = argv[2];

	debugC(kDebugScript, "Opcode %d: Vault Force Close", op);
	debugC(kDebugScript, "\tsoundId: %d", soundId);
	debugC(kDebugScript, "\tdirectionalUpdateDataSize: %d", directionalUpdateDataSize);

	if (_dockVaultState) {
		// Open switch
		_state.dockMarkerSwitch = 1;
		_vm->_sound->replaceSoundMyst(4143);
		_vm->redrawArea(4);

		// Close vault
		_dockVaultState = 0;
		_vm->_sound->replaceSoundMyst(soundId);
		_vm->redrawArea(41, false);
		animatedUpdate(directionalUpdateDataSize, &argv[3], delay);
	}
}

void Myst::o_imagerEraseStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Imager stop erase", op);
	_imagerValidationRunning = false;
}

void Myst::o_clockLeverStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Clock lever start move", op);
	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
	lever->drawFrame(0);
	_vm->_cursor->setCursor(700);
	_clockMiddleGearMovedAlone = false;
	_clockLeverPulled = false;
}

void Myst::o_clockLeverMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Clock left lever move", op);

	if (!_clockLeverPulled) {
		MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

		// If lever pulled
		if (lever->pullLeverV()) {
			// Start videos for first step
			if (_clockWeightPosition < 2214) {
				_vm->_sound->replaceSoundMyst(5113);
				clockGearForwardOneStep(1);

				// Left lever
				if (op == 144)
					clockGearForwardOneStep(2);
				else // Right lever
					clockGearForwardOneStep(0);

				clockWeightDownOneStep();
			}
			_clockLeverPulled = true;
		}
	}
}

void Myst::clockGearForwardOneStep(uint16 gear) {
	static const uint16 startTime[] = { 0, 324, 618 };
	static const uint16 endTime[] = { 324, 618, 950 };
	static const char *videos[] = { "cl1wg1", "cl1wg2", "cl1wg3" };
	static const uint16 x[] = { 224, 224, 224 };
	static const uint16 y[] = { 49, 82, 109 };

	// Increment value by one
	_clockGearsPositions[gear] = _clockGearsPositions[gear] % 3 + 1;

	// Set video bounds
	uint16 gearPosition = _clockGearsPositions[gear] - 1;
	_clockGearsVideos[gear] = _vm->_video->playMovie(_vm->wrapMovieFilename(videos[gear], kMystStack), x[gear], y[gear]);
	_vm->_video->setVideoBounds(_clockGearsVideos[gear],
			Audio::Timestamp(0, startTime[gearPosition], 600),
			Audio::Timestamp(0, endTime[gearPosition], 600));
}

void Myst::clockWeightDownOneStep() {
	// The Myst ME version of this video is encoded faster than the original
	// The weight goes on the floor one step too early. Original ME engine also has this behavior.
	bool updateVideo = !(_vm->getFeatures() & GF_ME) || _clockWeightPosition < (2214 - 246);

	// Set video bounds
	if (updateVideo) {
		_clockWeightVideo = _vm->_video->playMovie(_vm->wrapMovieFilename("cl1wlfch", kMystStack) , 124, 0);
		_vm->_video->setVideoBounds(_clockWeightVideo,
				Audio::Timestamp(0, _clockWeightPosition, 600),
				Audio::Timestamp(0, _clockWeightPosition + 246, 600));
	}

	// Increment value by one step
	_clockWeightPosition += 246;
}

void Myst::clockGears_run() {
	if (!_vm->_video->isVideoPlaying() && _clockWeightPosition < 2214) {
		_clockMiddleGearMovedAlone = true;
		_vm->_sound->replaceSoundMyst(5113);
		clockGearForwardOneStep(1);
		clockWeightDownOneStep();
	}
}

void Myst::o_clockLeverEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Clock lever end move", op);
	static const char *videos[] = { "cl1wg1", "cl1wg2", "cl1wg3", "cl1wlfch" };

	_vm->_cursor->hideCursor();
	_clockLeverPulled = false;

	// Let movies stop playing
	for (uint i = 0; i < ARRAYSIZE(videos); i++) {
		VideoHandle handle = _vm->_video->findVideoHandle(_vm->wrapMovieFilename(videos[i], kMystStack));
		if (handle != NULL_VID_HANDLE)
			_vm->_video->delayUntilMovieEnds(handle);
	}

	if (_clockMiddleGearMovedAlone)
		_vm->_sound->replaceSoundMyst(8113);

	// Release lever
	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
	lever->releaseLeverV();

	// Check if puzzle is solved
	clockGearsCheckSolution();

	_vm->_cursor->showCursor();
}

void Myst::clockGearsCheckSolution() {
	if (_clockGearsPositions[0] == 2
			&& _clockGearsPositions[1] == 2
			&& _clockGearsPositions[2] == 1
			&& !_state.gearsOpen) {

		// Make weight go down
		_vm->_sound->replaceSoundMyst(9113);
		_clockWeightVideo = _vm->_video->playMovie(_vm->wrapMovieFilename("cl1wlfch", kMystStack) , 124, 0);
		_vm->_video->setVideoBounds(_clockWeightVideo,
				Audio::Timestamp(0, _clockWeightPosition, 600),
				Audio::Timestamp(0, 2214, 600));
		_vm->_video->waitUntilMovieEnds(_clockWeightVideo);
		_clockWeightPosition = 2214;

		_vm->_sound->replaceSoundMyst(6113);
		_vm->_system->delayMillis(1000);
		_vm->_sound->replaceSoundMyst(7113);

		// Gear opening video
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("cl1wggat", kMystStack) , 195, 225);
		_state.gearsOpen = 1;
		_vm->redrawArea(40);

		_vm->_sound->replaceBackgroundMyst(4113, 16384);
	}
}

void Myst::o_clockResetLeverStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Clock reset lever start move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
	lever->drawFrame(0);
	_vm->_cursor->setCursor(700);
}

void Myst::o_clockResetLeverMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Clock reset lever move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	// If pulled
	if (lever->pullLeverV() && _clockWeightPosition != 0)
		clockReset();
}

void Myst::clockReset() {
	static const char *videos[] = { "cl1wg1", "cl1wg2", "cl1wg3", "cl1wlfch" };

	_vm->_cursor->hideCursor();

	_vm->_sound->stopBackgroundMyst();
	_vm->_sound->replaceSoundMyst(5113);

	// Play reset videos
	clockResetWeight();
	clockResetGear(0);
	clockResetGear(1);
	clockResetGear(2);

	// Let movies stop playing
	for (uint i = 0; i < ARRAYSIZE(videos); i++) {
		VideoHandle handle = _vm->_video->findVideoHandle(_vm->wrapMovieFilename(videos[i], kMystStack));
		if (handle != NULL_VID_HANDLE)
			_vm->_video->delayUntilMovieEnds(handle);
	}

	_vm->_sound->replaceSoundMyst(10113);

	// Close gear
	if (_state.gearsOpen) {
		_vm->_sound->replaceSoundMyst(6113);
		_vm->_system->delayMillis(1000);
		_vm->_sound->replaceSoundMyst(7113);

		// TODO: Play cl1wggat backwards
		// Redraw gear
		_state.gearsOpen = 0;
		_vm->redrawArea(40);
	}

	_vm->_cursor->showCursor();
}

void Myst::clockResetWeight() {
	_clockWeightVideo = _vm->_video->playMovie(_vm->wrapMovieFilename("cl1wlfch", kMystStack) , 124, 0);

	if (!(_vm->getFeatures() & GF_ME)) {
		// Set video bounds, weight going up
		_vm->_video->setVideoBounds(_clockWeightVideo,
				Audio::Timestamp(0, 2214 * 2 - _clockWeightPosition, 600),
				Audio::Timestamp(0, 2214 * 2, 600));
	} else {
		//FIXME: Needs QT backwards playing, for now just display the weight up
		warning("Weight going back up not implemented");
		_vm->_video->drawVideoFrame(_clockWeightVideo, Audio::Timestamp(0, 0, 600));
	}

	// Reset position
	_clockWeightPosition = 0;
}

void Myst::clockResetGear(uint16 gear) {
	static const uint16 time[] = { 324, 618, 950 };
	static const char *videos[] = { "cl1wg1", "cl1wg2", "cl1wg3" };
	static const uint16 x[] = { 224, 224, 224 };
	static const uint16 y[] = { 49, 82, 109 };

	// Set video bounds, gears going to 3
	uint16 gearPosition = _clockGearsPositions[gear] - 1;
	if (gearPosition != 2) {
		_clockGearsVideos[gear] = _vm->_video->playMovie(_vm->wrapMovieFilename(videos[gear], kMystStack), x[gear], y[gear]);
		_vm->_video->setVideoBounds(_clockGearsVideos[gear],
				Audio::Timestamp(0, time[gearPosition], 600),
				Audio::Timestamp(0, time[2], 600));
	}

	// Reset gear position
	_clockGearsPositions[gear] = 3;
}

void Myst::o_clockResetLeverEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Clock reset lever end move", op);

	// Get current lever frame
	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	lever->releaseLeverV();

	_vm->checkCursorHints();
}

void Myst::o_libraryBook_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_libraryBookPage = 0;
	_libraryBookNumPages = argv[0];
	_libraryBookBaseImage = argv[1];
	_libraryBookSound1 = argv[2];
	_libraryBookSound2 = argv[3];
}

void Myst::o_courtyardBox_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Courtyard box init", op);

	_courtyardBoxSound = argv[0];
}

void Myst::towerRotationMap_run() {
	if (!_towerRotationMapInitialized) {
		_towerRotationMapInitialized = true;
		_vm->_sound->replaceSoundMyst(4378);

		towerRotationDrawBuildings();

		// Draw to screen
		_vm->_gfx->copyBackBufferToScreen(Common::Rect(106, 42, 459, 273));
		_vm->_system->updateScreen();
	}

	uint32 time = _vm->_system->getMillis();
	if (time > _startTime) {
		if (_towerRotationMapClicked) {
			towerRotationMapRotate();
			_startTime = time + 100;
		} else if (_towerRotationBlinkLabel
				&& _vm->_sound->isPlaying(6378)) {
			// Blink tower rotation label while sound is playing
			_towerRotationBlinkLabelCount = (_towerRotationBlinkLabelCount + 1) % 14;

			if (_towerRotationBlinkLabelCount == 7)
				_towerRotationMapLabel->drawConditionalDataToScreen(0);
			else if (_towerRotationBlinkLabelCount == 0)
				_towerRotationMapLabel->drawConditionalDataToScreen(1);

			_startTime = time + 100;
		} else {
			// Stop blinking label
			_towerRotationBlinkLabel = false;
			_towerRotationMapLabel->drawConditionalDataToScreen(0);

			// Blink tower
			_startTime = time + 500;
			_tempVar = (_tempVar + 1) % 2;
			_towerRotationMapTower->drawConditionalDataToScreen(_tempVar);
		}
	}
}

void Myst::o_towerRotationMap_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_towerRotationMapRunning = true;
	_towerRotationMapTower = static_cast<MystResourceType11 *>(_invokingResource);
	_towerRotationMapLabel = static_cast<MystResourceType8 *>(_vm->_resources[argv[0]]);
	_tempVar = 0;
	_startTime = 0;
	_towerRotationMapClicked = false;
}

void Myst::towerRotationDrawBuildings() {
	// Draw library
	_vm->redrawArea(304, false);

	// Draw other resources
	for (uint i = 1; i <= 10; i++) {
		MystResourceType8 *resource = static_cast<MystResourceType8 *>(_vm->_resources[i]);
		_vm->redrawResource(resource, false);
	}
}

uint16 Myst::towerRotationMapComputeAngle() {
	_towerRotationSpeed++;
	if (_towerRotationSpeed >= 7)
		_towerRotationSpeed = 7;
	else
		_towerRotationSpeed++;

	_state.towerRotationAngle = (_state.towerRotationAngle + _towerRotationSpeed) % 360;
	uint16 angle = _state.towerRotationAngle;
	_towerRotationOverSpot = false;

	if (angle >= 265 && angle <= 277
			&& _state.rocketshipMarkerSwitch) {
		angle = 271;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	} else if (angle >= 77 && angle <= 89
			&& _state.gearsMarkerSwitch) {
		angle = 83;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	} else if (angle >= 123 && angle <= 135
			&& _state.dockMarkerSwitch) {
		angle = 129;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	} else if (angle >= 146 && angle <= 158
			&& _state.cabinMarkerSwitch) {
		angle = 152;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	}

	return angle;
}

Common::Point Myst::towerRotationMapComputeCoords(const Common::Point &center, uint16 angle) {
	Common::Point end;

	// Polar to rect coords
	double radians = angle * M_PI / 180.0;
	end.x = (int16)(center.x + cos(radians) * 310.0);
	end.y = (int16)(center.y + sin(radians) * 310.0);

	return end;
}

void Myst::towerRotationMapDrawLine(const Common::Point &center, const Common::Point &end) {
	Graphics::PixelFormat pf = _vm->_system->getScreenFormat();
	uint32 color = 0;

	if (!_towerRotationOverSpot)
		color = pf.RGBToColor(0xFF, 0xFF, 0xFF); // White
	else
		color = pf.RGBToColor(0xFF, 0, 0); // Red

	const Common::Rect rect = Common::Rect(106, 42, 459, 273);

	Common::Rect src;
	src.left = rect.left;
	src.top = 333 - rect.bottom;
	src.right = rect.right;
	src.bottom = 333 - rect.top;

	// Redraw background
	_vm->_gfx->copyImageSectionToBackBuffer(_vm->getCardBackgroundId(), src, rect);

	// Draw buildings
	towerRotationDrawBuildings();

	// Draw tower
	_towerRotationMapTower->drawConditionalDataToScreen(0, false);

	// Draw label
	_towerRotationMapLabel->drawConditionalDataToScreen(1, false);

	// Draw line
	_vm->_gfx->drawLine(center, end, color);
	_vm->_gfx->copyBackBufferToScreen(rect);
	_vm->_system->updateScreen();
}

void Myst::towerRotationMapRotate() {
	const Common::Point center = Common::Point(383, 124);
	uint16 angle = towerRotationMapComputeAngle();
	Common::Point end = towerRotationMapComputeCoords(center, angle);
	towerRotationMapDrawLine(center, end);
}

void Myst::o_forechamberDoor_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 4138 (Dock Forechamber Door)
	// Set forechamber door to closed
	_tempVar = 0;
}

void Myst::o_shipAccess_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Enable acces to the ship
	if (_state.shipFloating) {
		_invokingResource->setEnabled(true);
	}
}

void Myst::o_butterflies_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Butterflies movie init", op);

	// Used for Card 4256 (Butterfly Movie Activation)
	if (!_butterfliesMoviePlayed) {
		MystResourceType6 *butterflies = static_cast<MystResourceType6 *>(_invokingResource);
		butterflies->playMovie();

		_butterfliesMoviePlayed = true;
	}
}

void Myst::o_imager_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Imager init", op);
	debugC(kDebugScript, "Var: %d", var);

	MystResourceType7 *select = static_cast<MystResourceType7 *>(_invokingResource);
	_imagerMovie = static_cast<MystResourceType6 *>(select->getSubResource(getVar(var)));
	_imagerRunning = true;
}

void Myst::imager_run() {
	_imagerRunning = false;

	if (_state.imagerActive && _state.imagerSelection == 67) {
		VideoHandle water = _imagerMovie->playMovie();
		_vm->_video->setVideoBounds(water, Audio::Timestamp(0, 1814, 600), Audio::Timestamp(0, 4204, 600));
		_vm->_video->setVideoLooping(water, true);
	}
}

void Myst::libraryBookcaseTransform_run(void) {
	if (_libraryBookcaseChanged) {
		_libraryBookcaseChanged = false;
		_libraryBookcaseMoving = false;

		_vm->_cursor->hideCursor();

		// Play transform sound and video
		_vm->_sound->replaceSoundMyst(_libraryBookcaseSoundId);
		_libraryBookcaseMovie->playMovie();

		if (_state.libraryBookcaseDoor) {
			_vm->_gfx->copyImageSectionToBackBuffer(11179, Common::Rect(0, 0, 106, 81), Common::Rect(0, 72, 106, 153));
			_vm->_gfx->runTransition(6, Common::Rect(0, 72, 106, 153), 5, 10);
			_vm->_sound->playSoundBlocking(7348);
			_vm->_sound->replaceBackgroundMyst(4348, 16384);
		} else {
			_vm->_gfx->copyImageSectionToBackBuffer(11178, Common::Rect(0, 0, 107, 67), Common::Rect(437, 84, 544, 151));
			_vm->_gfx->copyBackBufferToScreen(Common::Rect(437, 84, 544, 151));
			_vm->_sound->playSoundBlocking(7348);
			_vm->_sound->replaceBackgroundMyst(4334, 16384);
		}

		_vm->_cursor->showCursor();
	}
}

void Myst::o_libraryBookcaseTransform_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (_libraryBookcaseChanged) {
		MystResourceType7 *resource = static_cast<MystResourceType7 *>(_invokingResource);
		_libraryBookcaseMovie = static_cast<MystResourceType6 *>(resource->getSubResource(getVar(0)));
		_libraryBookcaseSoundId = argv[0];
		_libraryBookcaseMoving = true;
	}
}

void Myst::generatorControlRoom_run(void) {
	if (_generatorVoltage == _state.generatorVoltage) {
		generatorRedrawRocket();
	} else {
		// Animate generator gauge
		if (_generatorVoltage > _state.generatorVoltage)
			_generatorVoltage--;
		else
			_generatorVoltage++;

		// Redraw generator gauge
		_vm->redrawArea(62);
		_vm->redrawArea(63);
		_vm->redrawArea(96);
	}
}

void Myst::o_generatorControlRoom_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generator control room init", op);

	_generatorVoltage = _state.generatorVoltage;
	_generatorControlRoomRunning = true;
}

void Myst::o_fireplace_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Fireplace grid init", op);

	// Clear fireplace grid
	for (uint i = 0; i < 6; i++)
		_fireplaceLines[i] = 0;
}

void Myst::o_clockGears_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 4113 (Clock Tower Cog Puzzle)
	debugC(kDebugScript, "Opcode %d: Gears puzzle init", op);

	// Set gears position
	if (_state.gearsOpen) {
		_clockGearsPositions[0] = 2;
		_clockGearsPositions[1] = 2;
		_clockGearsPositions[2] = 1;
		_clockWeightPosition = 2214;
	} else {
		_clockGearsPositions[0] = 3;
		_clockGearsPositions[1] = 3;
		_clockGearsPositions[2] = 3;
		_clockWeightPosition = 0;
	}
}

void Myst::o_gulls1_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Gulls init", op);

	if (!_state.shipFloating) {
		_gullsNextTime = _vm->_system->getMillis() + 2000;
		_gullsFlying1 = true;
	}
}

void Myst::gullsFly1_run() {
	static const char* gulls[] = { "birds1", "birds2", "birds3" };
	uint32 time = _vm->_system->getMillis();

	if (time > _gullsNextTime) {
		uint16 video = _vm->_rnd->getRandomNumber(3);
		if (video != 3) {
			uint16 x = 0;
			if (_vm->_rnd->getRandomBit())
				x = _vm->_rnd->getRandomNumber(110);
			else
				x = _vm->_rnd->getRandomNumber(160) + 260;

			_vm->_video->playMovie(_vm->wrapMovieFilename(gulls[video], kMystStack), x, 0);

			_gullsNextTime = time + _vm->_rnd->getRandomNumber(16667) + 13334;
		}
	}
}

void Myst::o_observatory_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Stellar observatory init", op);

	_tempVar = 0;
	_observatoryNotInitialized = true;
	_observatoryVisualizer = static_cast<MystResourceType8 *>(_invokingResource);
	_observatoryGoButton = static_cast<MystResourceType8 *>(_vm->_resources[argv[0]]);
	if (observatoryIsDDMMYYYY2400()) {
		_observatoryDaySlider = static_cast<MystResourceType10 *>(_vm->_resources[argv[1]]);
		_observatoryMonthSlider = static_cast<MystResourceType10 *>(_vm->_resources[argv[2]]);
	} else {
		_observatoryMonthSlider = static_cast<MystResourceType10 *>(_vm->_resources[argv[1]]);
		_observatoryDaySlider = static_cast<MystResourceType10 *>(_vm->_resources[argv[2]]);
	}
	_observatoryYearSlider = static_cast<MystResourceType10 *>(_vm->_resources[argv[3]]);
	_observatoryTimeSlider = static_cast<MystResourceType10 *>(_vm->_resources[argv[4]]);

	// Set date selection sliders position
	_observatoryDaySlider->setPosition(_state.observatoryDaySlider);
	_observatoryMonthSlider->setPosition(_state.observatoryMonthSlider);
	_observatoryYearSlider->setPosition(_state.observatoryYearSlider);
	_observatoryTimeSlider->setPosition(_state.observatoryTimeSlider);

	_observatoryLastTime = _vm->_system->getMillis();

	observatorySetTargetToSetting();

	_observatoryRunning = true;
}

bool Myst::observatoryIsDDMMYYYY2400() {
	// TODO: Auto-detect based on the month rect position
	return !(_vm->getFeatures() & GF_ME) && (_vm->getLanguage() == Common::FR_FRA
			|| _vm->getLanguage() == Common::DE_DEU);
}

void Myst::observatoryUpdateVisualizer(uint16 x, uint16 y) {
	Common::Rect &visu0 = _observatoryVisualizer->_subImages[0].rect;
	Common::Rect &visu1 = _observatoryVisualizer->_subImages[1].rect;

	visu0.left = x;
	visu0.right = visu0.left + 105;
	visu0.bottom = 512 - y;
	visu0.top = visu0.bottom - 106;

	visu1.left = visu0.left;
	visu1.top = visu0.top;
	visu1.right = visu0.right;
	visu1.bottom = visu0.bottom;
}

void Myst::observatorySetTargetToSetting() {
	uint32 visuX = _state.observatoryTimeSetting * 7 / 25;
	uint32 visuY = 250 * _state.observatoryYearSetting
			+ 65 * (_state.observatoryMonthSetting + 1)
			+ 20 * _state.observatoryDaySetting;

	observatoryUpdateVisualizer(visuX % 407, visuY % 407);

	_state.observatoryDayTarget = _state.observatoryDaySetting;
	_state.observatoryMonthTarget = _state.observatoryMonthSetting;
	_state.observatoryYearTarget = _state.observatoryYearSetting;
	_state.observatoryTimeTarget = _state.observatoryTimeSetting;
}

void Myst::observatory_run() {
	if (_observatoryNotInitialized) {
		_observatoryNotInitialized = false;

		_vm->_cursor->hideCursor();

		// Make sliders "initialize"
		if (observatoryIsDDMMYYYY2400()) {
			_vm->_sound->replaceSoundMyst(8500);
			_observatoryDaySlider->drawConditionalDataToScreen(2);
			_vm->_system->delayMillis(200);
			_vm->redrawResource(_observatoryDaySlider);

			_vm->_sound->replaceSoundMyst(8500);
			_observatoryMonthSlider->drawConditionalDataToScreen(2);
			_vm->_system->delayMillis(200);
			_vm->redrawResource(_observatoryMonthSlider);
		} else {
			_vm->_sound->replaceSoundMyst(8500);
			_observatoryMonthSlider->drawConditionalDataToScreen(2);
			_vm->_system->delayMillis(200);
			_vm->redrawResource(_observatoryMonthSlider);

			_vm->_sound->replaceSoundMyst(8500);
			_observatoryDaySlider->drawConditionalDataToScreen(2);
			_vm->_system->delayMillis(200);
			_vm->redrawResource(_observatoryDaySlider);
		}

		_vm->_sound->replaceSoundMyst(8500);
		_observatoryYearSlider->drawConditionalDataToScreen(2);
		_vm->_system->delayMillis(200);
		_vm->redrawResource(_observatoryYearSlider);

		_vm->_sound->replaceSoundMyst(8500);
		_observatoryTimeSlider->drawConditionalDataToScreen(2);
		_vm->_system->delayMillis(200);
		_vm->redrawResource(_observatoryTimeSlider);

		_vm->_cursor->showCursor();
	}

	// Setting not at target
	if (_state.observatoryDayTarget != _state.observatoryDaySetting
			|| _state.observatoryMonthTarget != _state.observatoryMonthSetting
			|| _state.observatoryYearTarget != _state.observatoryYearSetting
			|| _state.observatoryTimeTarget != _state.observatoryTimeSetting) {

		// Blink the go button
		uint32 time = _vm->_system->getMillis();
		if (time > _observatoryLastTime + 250) {
			_tempVar = (_tempVar + 1) % 2;
			_observatoryGoButton->drawConditionalDataToScreen(_tempVar);
			_observatoryLastTime = time;
		}
	}
}

void Myst::o_gulls2_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Gulls init", op);

	if (!_state.shipFloating) {
		_gullsNextTime = _vm->_system->getMillis() + 2000;
		_gullsFlying2 = true;
	}
}

void Myst::gullsFly2_run() {
	static const char* gulls[] = { "birds1", "birds2", "birds3" };
	uint32 time = _vm->_system->getMillis();

	if (time > _gullsNextTime) {
		uint16 video = _vm->_rnd->getRandomNumber(3);
		if (video != 3) {
			_vm->_video->playMovie(_vm->wrapMovieFilename(gulls[video], kMystStack), 424, 0);

			_gullsNextTime = time + _vm->_rnd->getRandomNumber(16667) + 13334;
		}
	}
}

void Myst::o_treeCard_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Enter tree card", op);

	_tree = static_cast<MystResourceType8 *>(_invokingResource);
}

void Myst::o_treeEntry_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Enter tree card with entry", op);

	_treeAlcove = static_cast<MystResourceType5 *>(_invokingResource);
	_treeMinAccessiblePosition = argv[0];
	_treeMaxAccessiblePosition = argv[1];

	treeSetAlcoveAccessible();
}

void Myst::opcode_218(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4097 (Cabin Boiler)
	// TODO: Fill in logic
	if (false) {
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("cabfirfr", kMystStack), 254, 244);
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("cabcgfar", kMystStack), 254, 138);
	}

	// Used for Card 4098 (Cabin Boiler)
	// TODO: Fill in logic
	if (false) {
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("cabfire", kMystStack), 240, 279);
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("cabingau", kMystStack), 243, 97);
	}
}

void Myst::o_rocketSliders_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket sliders init", op);

	_rocketSlider1 = static_cast<MystResourceType10 *>(_vm->_resources[argv[0]]);
	_rocketSlider2 = static_cast<MystResourceType10 *>(_vm->_resources[argv[1]]);
	_rocketSlider3 = static_cast<MystResourceType10 *>(_vm->_resources[argv[2]]);
	_rocketSlider4 = static_cast<MystResourceType10 *>(_vm->_resources[argv[3]]);
	_rocketSlider5 = static_cast<MystResourceType10 *>(_vm->_resources[argv[4]]);

	// Initialize sliders position
	for (uint i = 0; i < 5; i++)
		if (!_state.rocketSliderPosition[i])
			_state.rocketSliderPosition[i] = 277;

	_rocketSlider1->setPosition(_state.rocketSliderPosition[0]);
	_rocketSlider2->setPosition(_state.rocketSliderPosition[1]);
	_rocketSlider3->setPosition(_state.rocketSliderPosition[2]);
	_rocketSlider4->setPosition(_state.rocketSliderPosition[3]);
	_rocketSlider5->setPosition(_state.rocketSliderPosition[4]);
}

void Myst::o_rocketLinkVideo_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket link video init", op);
	_tempVar = 0;
}

void Myst::o_greenBook_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 4168 (Green Book Movies)
	debugC(kDebugScript, "Opcode %d: Green book init", op);

	_greenBookRunning = true;
	_tempVar = 1;
}

void Myst::greenBook_run() {
	uint loopStart = 0;
	uint loopEnd = 0;
	Common::String file;

	if (!_state.greenBookOpenedBefore) {
		loopStart = 113200;
		loopEnd = 116400;
		file = _vm->wrapMovieFilename("atrusbk1", kMystStack);
	} else {
		loopStart = 8800;
		loopEnd = 9700;
		file = _vm->wrapMovieFilename("atrusbk2", kMystStack);
	}

	if (_tempVar == 1) {
		_vm->_sound->stopSound();
		_vm->_sound->pauseBackgroundMyst();

		if (_globals.ending != 4) {
			_tempVar = 2;
			_vm->_video->playMovie(file, 314, 76);
		} else {
			VideoHandle book = _vm->_video->playMovie(file, 314, 76, true);
			_vm->_video->setVideoBounds(book, Audio::Timestamp(0, loopStart, 600), Audio::Timestamp(0, loopEnd, 600));
			_tempVar = 0;
		}
	} else if (_tempVar == 2 && !_vm->_video->isVideoPlaying()) {
		VideoHandle book = _vm->_video->playMovie(file, 314, 76);
		_vm->_video->setVideoBounds(book, Audio::Timestamp(0, loopStart, 600), Audio::Timestamp(0, loopEnd, 600));
		_vm->_video->setVideoLooping(book, true);
		_tempVar = 0;
	}
}

void Myst::o_gulls3_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Gulls init", op);

	if (!_state.shipFloating) {
		_gullsNextTime = _vm->_system->getMillis() + 2000;
		_gullsFlying3 = true;
	}
}

void Myst::gullsFly3_run() {
	static const char* gulls[] = { "birds1", "birds2", "birds3" };
	uint32 time = _vm->_system->getMillis();

	if (time > _gullsNextTime) {
		uint16 video = _vm->_rnd->getRandomNumber(3);
		if (video != 3) {
			uint16 x = _vm->_rnd->getRandomNumber(280) + 135;

			_vm->_video->playMovie(_vm->wrapMovieFilename(gulls[video], kMystStack), x, 0);

			_gullsNextTime = time + _vm->_rnd->getRandomNumber(16667) + 13334;
		}
	}
}

void Myst::o_bookAddSpecialPage_exit(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Book Exit Function", op);

	uint16 numPages = bookCountPages(var);

	// Add special page
	if (numPages == 5) {
		if (var == 100)
			_globals.redPagesInBook |= 64;
		else
			_globals.bluePagesInBook |= 64;
	}
}

void Myst::o_treeCard_exit(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Exit tree card", op);

	_tree = 0;
}

void Myst::o_treeEntry_exit(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Exit tree card with entry", op);

	_treeAlcove = 0;
}

void Myst::o_generatorControlRoom_exit(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generator room exit", op);

	_generatorVoltage = _state.generatorVoltage;
}

} // End of namespace MystStacks
} // End of namespace Mohawk
