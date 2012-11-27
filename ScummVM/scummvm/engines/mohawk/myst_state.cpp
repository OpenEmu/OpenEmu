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
#include "mohawk/myst_state.h"

#include "common/debug.h"
#include "common/serializer.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Mohawk {

MystGameState::MystGameState(MohawkEngine_Myst *vm, Common::SaveFileManager *saveFileMan) : _vm(vm), _saveFileMan(saveFileMan) {
	// Most of the variables are zero at game start.
	memset(&_globals, 0, sizeof(_globals));
	memset(&_myst, 0, sizeof(_myst));
	memset(&_channelwood, 0, sizeof(_channelwood));
	memset(&_mechanical, 0, sizeof(_mechanical));
	memset(&_selenitic, 0, sizeof(_selenitic));
	memset(&_stoneship, 0, sizeof(_stoneship));
	memset(&_mystReachableZipDests, 0, sizeof(_mystReachableZipDests));
	memset(&_channelwoodReachableZipDests, 0, sizeof(_channelwoodReachableZipDests));
	memset(&_mechReachableZipDests, 0, sizeof(_mechReachableZipDests));
	memset(&_seleniticReachableZipDests, 0, sizeof(_seleniticReachableZipDests));
	memset(&_stoneshipReachableZipDests, 0, sizeof(_stoneshipReachableZipDests));

	// Unknown
	_globals.u0 = 2;
	// Current Age / Stack - Start in Myst
	_globals.currentAge = 7;
	_globals.u1 = 1;

	// Library Bookcase Door - Default to Up
	_myst.libraryBookcaseDoor = 1;
	// Dock Imager Numeric Selection - Default to 67
	_myst.imagerSelection = 67;
	// Dock Imager Active - Default to Active
	_myst.imagerActive = 1;
	// Stellar Observatory Lights - Default to On
	_myst.observatoryLights = 1;
	// First day of month
	_myst.observatoryDaySetting = 1;
	// Stellar Observatory sliders
	_myst.observatoryDaySlider = 90;
	_myst.observatoryMonthSlider = 90;
	_myst.observatoryYearSlider = 90;
	_myst.observatoryTimeSlider = 90;

	// Lighthouse Trapdoor State - Default to Locked
	_stoneship.trapdoorState = 2;
	// Lighthouse Chest Water State - Default to Full
	_stoneship.chestWaterState = 1;
}

MystGameState::~MystGameState() {
}

Common::StringArray MystGameState::generateSaveGameList() {
	return _saveFileMan->listSavefiles("*.mys");
}

bool MystGameState::load(const Common::String &filename) {
	Common::InSaveFile *loadFile = _saveFileMan->openForLoading(filename);
	if (!loadFile)
		return false;

	debugC(kDebugSaveLoad, "Loading game from '%s'", filename.c_str());

	// First, let's make sure we're using a saved game file from this version of Myst
	// By checking length of file...
	int32 size = loadFile->size();
	if (size != 664 && size != 601) {
		warning("Incompatible saved game version");
		delete loadFile;
		return false;
	}

	Common::Serializer s(loadFile, 0);
	syncGameState(s, size == 664);
	delete loadFile;

	// Switch us back to the intro stack, to the linking book
	_vm->changeToStack(kIntroStack, 5, 0, 0);

	// Set our default cursor
	if (_globals.heldPage == 0 || _globals.heldPage > 13)
		_vm->setMainCursor(kDefaultMystCursor);
	else if (_globals.heldPage < 7)
		_vm->setMainCursor(kBluePageCursor);
	else if (_globals.heldPage < 13)
		_vm->setMainCursor(kRedPageCursor);
	else // if (globals.heldPage == 13)
		_vm->setMainCursor(kWhitePageCursor);

	return true;
}

bool MystGameState::save(const Common::String &fname) {
	Common::String filename(fname);
	// Make sure we have the right extension
	if (!filename.hasSuffix(".mys") && !filename.hasSuffix(".MYS"))
		filename += ".mys";

	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(filename);
	if (!saveFile)
		return false;

	debugC(kDebugSaveLoad, "Saving game to '%s'", filename.c_str());

	Common::Serializer s(0, saveFile);
	syncGameState(s, _vm->getFeatures() & GF_ME);
	saveFile->finalize();
	delete saveFile;

	return true;
}

void MystGameState::syncGameState(Common::Serializer &s, bool isME) {
	// Globals first
	s.syncAsUint16LE(_globals.u0);
	s.syncAsUint16LE(_globals.currentAge);
	s.syncAsUint16LE(_globals.heldPage);
	s.syncAsUint16LE(_globals.u1);
	s.syncAsUint16LE(_globals.transitions);
	s.syncAsUint16LE(_globals.zipMode);
	s.syncAsUint16LE(_globals.redPagesInBook);
	s.syncAsUint16LE(_globals.bluePagesInBook);

	// Onto Myst
	if (isME) {
		s.syncAsUint32LE(_myst.cabinMarkerSwitch);
		s.syncAsUint32LE(_myst.clockTowerMarkerSwitch);
		s.syncAsUint32LE(_myst.dockMarkerSwitch);
		s.syncAsUint32LE(_myst.poolMarkerSwitch);
		s.syncAsUint32LE(_myst.gearsMarkerSwitch);
		s.syncAsUint32LE(_myst.generatorMarkerSwitch);
		s.syncAsUint32LE(_myst.observatoryMarkerSwitch);
		s.syncAsUint32LE(_myst.rocketshipMarkerSwitch);
	} else {
		s.syncAsByte(_myst.cabinMarkerSwitch);
		s.syncAsByte(_myst.clockTowerMarkerSwitch);
		s.syncAsByte(_myst.dockMarkerSwitch);
		s.syncAsByte(_myst.poolMarkerSwitch);
		s.syncAsByte(_myst.gearsMarkerSwitch);
		s.syncAsByte(_myst.generatorMarkerSwitch);
		s.syncAsByte(_myst.observatoryMarkerSwitch);
		s.syncAsByte(_myst.rocketshipMarkerSwitch);
	}

	s.syncAsUint16LE(_myst.greenBookOpenedBefore);
	s.syncAsUint16LE(_myst.shipFloating);
	s.syncAsUint16LE(_myst.cabinValvePosition);
	s.syncAsUint16LE(_myst.clockTowerHourPosition);
	s.syncAsUint16LE(_myst.clockTowerMinutePosition);
	s.syncAsUint16LE(_myst.gearsOpen);
	s.syncAsUint16LE(_myst.clockTowerBridgeOpen);
	s.syncAsUint16LE(_myst.generatorBreakers);
	s.syncAsUint16LE(_myst.generatorButtons);
	s.syncAsUint16LE(_myst.generatorVoltage);
	s.syncAsUint16LE(_myst.libraryBookcaseDoor);
	s.syncAsUint16LE(_myst.imagerSelection);
	s.syncAsUint16LE(_myst.imagerActive);
	s.syncAsUint16LE(_myst.imagerWaterErased);
	s.syncAsUint16LE(_myst.imagerMountainErased);
	s.syncAsUint16LE(_myst.imagerAtrusErased);
	s.syncAsUint16LE(_myst.imagerMarkerErased);
	s.syncAsUint16LE(_myst.towerRotationAngle);
	s.syncAsUint16LE(_myst.courtyardImageBoxes);
	s.syncAsUint16LE(_myst.cabinPilotLightLit);
	s.syncAsUint16LE(_myst.observatoryDaySetting);
	s.syncAsUint16LE(_myst.observatoryLights);
	s.syncAsUint16LE(_myst.observatoryMonthSetting);
	s.syncAsUint16LE(_myst.observatoryTimeSetting);
	s.syncAsUint16LE(_myst.observatoryYearSetting);
	s.syncAsUint16LE(_myst.observatoryDayTarget);
	s.syncAsUint16LE(_myst.observatoryMonthTarget);
	s.syncAsUint16LE(_myst.observatoryTimeTarget);
	s.syncAsUint16LE(_myst.observatoryYearTarget);
	s.syncAsUint16LE(_myst.cabinSafeCombination);
	s.syncAsUint16LE(_myst.treePosition);
	s.syncAsUint32LE(_myst.treeLastMoveTime);

	for (int i = 0; i < 5; i++)
		s.syncAsUint16LE(_myst.rocketSliderPosition[i]);

	s.syncAsUint16LE(_myst.observatoryDaySlider);
	s.syncAsUint16LE(_myst.observatoryMonthSlider);
	s.syncAsUint16LE(_myst.observatoryYearSlider);
	s.syncAsUint16LE(_myst.observatoryTimeSlider);

	// Channelwood
	if (isME) {
		s.syncAsUint32LE(_channelwood.waterPumpBridgeState);
		s.syncAsUint32LE(_channelwood.elevatorState);
		s.syncAsUint32LE(_channelwood.stairsLowerDoorState);
		s.syncAsUint32LE(_channelwood.pipeState);
	} else {
		s.syncAsByte(_channelwood.waterPumpBridgeState);
		s.syncAsByte(_channelwood.elevatorState);
		s.syncAsByte(_channelwood.stairsLowerDoorState);
		s.syncAsByte(_channelwood.pipeState);
	}

	s.syncAsUint16LE(_channelwood.waterValveStates);
	s.syncAsUint16LE(_channelwood.holoprojectorSelection);
	s.syncAsUint16LE(_channelwood.stairsUpperDoorState);

	// Mechanical

	if (isME)
		s.syncAsUint32LE(_mechanical.achenarCrateOpened);
	else
		s.syncAsByte(_mechanical.achenarCrateOpened);

	s.syncAsUint16LE(_mechanical.achenarPanelState);
	s.syncAsUint16LE(_mechanical.sirrusPanelState);
	s.syncAsUint16LE(_mechanical.staircaseState);
	s.syncAsUint16LE(_mechanical.elevatorRotation);

	for (int i = 0; i < 4; i++)
		s.syncAsUint16LE(_mechanical.codeShape[i]);

	// Selenitic

	if (isME) {
		s.syncAsUint32LE(_selenitic.emitterEnabledWater);
		s.syncAsUint32LE(_selenitic.emitterEnabledVolcano);
		s.syncAsUint32LE(_selenitic.emitterEnabledClock);
		s.syncAsUint32LE(_selenitic.emitterEnabledCrystal);
		s.syncAsUint32LE(_selenitic.emitterEnabledWind);
		s.syncAsUint32LE(_selenitic.soundReceiverOpened);
		s.syncAsUint32LE(_selenitic.tunnelLightsSwitchedOn);
	} else {
		s.syncAsByte(_selenitic.emitterEnabledWater);
		s.syncAsByte(_selenitic.emitterEnabledVolcano);
		s.syncAsByte(_selenitic.emitterEnabledClock);
		s.syncAsByte(_selenitic.emitterEnabledCrystal);
		s.syncAsByte(_selenitic.emitterEnabledWind);
		s.syncAsByte(_selenitic.soundReceiverOpened);
		s.syncAsByte(_selenitic.tunnelLightsSwitchedOn);
	}

	s.syncAsUint16LE(_selenitic.soundReceiverCurrentSource);

	for (byte i = 0; i < 5; i++)
		s.syncAsUint16LE(_selenitic.soundReceiverPositions[i]);

	for (byte i = 0; i < 5; i++)
		s.syncAsUint16LE(_selenitic.soundLockSliderPositions[i]);

	// Stoneship

	if (isME) {
		s.syncAsUint32LE(_stoneship.lightState);
	} else {
		s.syncAsByte(_stoneship.lightState);
	}

	s.syncAsUint16LE(_stoneship.sideDoorOpened);
	s.syncAsUint16LE(_stoneship.pumpState);
	s.syncAsUint16LE(_stoneship.trapdoorState);
	s.syncAsUint16LE(_stoneship.chestWaterState);
	s.syncAsUint16LE(_stoneship.chestValveState);
	s.syncAsUint16LE(_stoneship.chestOpenState);
	s.syncAsUint16LE(_stoneship.trapdoorKeyState);
	s.syncAsUint32LE(_stoneship.generatorDuration);
	s.syncAsUint16LE(_stoneship.generatorPowerAvailable);
	s.syncAsUint32LE(_stoneship.generatorDepletionTime);

	// D'ni
	s.syncAsUint16LE(_globals.ending);

	// Already visited zip destinations

	for (byte i = 0; i < 41; i++)
		s.syncAsUint16LE(_mystReachableZipDests[i]);

	for (byte i = 0; i < 41; i++)
		s.syncAsUint16LE(_channelwoodReachableZipDests[i]);

	for (byte i = 0; i < 41; i++)
		s.syncAsUint16LE(_mechReachableZipDests[i]);

	for (byte i = 0; i < 41; i++)
		s.syncAsUint16LE(_seleniticReachableZipDests[i]);

	for (byte i = 0; i < 41; i++)
		s.syncAsUint16LE(_stoneshipReachableZipDests[i]);

	if ((isME && s.bytesSynced() != 664) || (!isME && s.bytesSynced() != 601))
		warning("Unexpected File Position 0x%03X At End of Save/Load", s.bytesSynced());
}

void MystGameState::deleteSave(const Common::String &saveName) {
	debugC(kDebugSaveLoad, "Deleting save file \'%s\'", saveName.c_str());
	_saveFileMan->removeSavefile(saveName.c_str());
}

void MystGameState::addZipDest(uint16 stack, uint16 view) {
	ZipDests *zipDests = 0;

	// The demo has no zip dest storage
	if (_vm->getFeatures() & GF_DEMO)
		return;

	// Select stack
	switch (stack) {
	case kChannelwoodStack:
		zipDests = &_channelwoodReachableZipDests;
		break;
	case kMechanicalStack:
		zipDests = &_mechReachableZipDests;
		break;
	case kMystStack:
		zipDests = &_mystReachableZipDests;
		break;
	case kSeleniticStack:
		zipDests = &_seleniticReachableZipDests;
		break;
	case kStoneshipStack:
		zipDests = &_stoneshipReachableZipDests;
		break;
	default:
		error("Stack does not have zip destination storage");
	}

	// Check if not already in list
	int16 firstEmpty = -1;
	bool found = false;
	for (uint i = 0; i < ARRAYSIZE(*zipDests); i++) {
		if (firstEmpty == -1 && (*zipDests)[i] == 0)
			firstEmpty = i;

		if ((*zipDests)[i] == view)
			found = true;
	}

	// Add view to array
	if (!found && firstEmpty >= 0)
		(*zipDests)[firstEmpty] = view;
}

bool MystGameState::isReachableZipDest(uint16 stack, uint16 view) {
	// Zip mode enabled
	if (!_globals.zipMode)
		return false;

	// The demo has no zip dest storage
	if (_vm->getFeatures() & GF_DEMO)
		return false;

	// Select stack
	ZipDests *zipDests;
	switch (stack) {
	case kChannelwoodStack:
		zipDests = &_channelwoodReachableZipDests;
		break;
	case kMechanicalStack:
		zipDests = &_mechReachableZipDests;
		break;
	case kMystStack:
		zipDests = &_mystReachableZipDests;
		break;
	case kSeleniticStack:
		zipDests = &_seleniticReachableZipDests;
		break;
	case kStoneshipStack:
		zipDests = &_stoneshipReachableZipDests;
		break;
	default:
		error("Stack does not have zip destination storage");
	}

	// Check if in list
	for (uint i = 0; i < ARRAYSIZE(*zipDests); i++) {
		if ((*zipDests)[i] == view)
			return true;
	}

	return false;
}

} // End of namespace Mohawk
