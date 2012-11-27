/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "pegasus/compass.h"
#include "pegasus/energymonitor.h"
#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_action.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/ai/ai_condition.h"
#include "pegasus/ai/ai_rule.h"
#include "pegasus/neighborhood/prehistoric/prehistoric.h"

namespace Pegasus {

static const int16 s_prehistoricCompass[kPrehistoric25 + 1][4] = {
	{ 0, 170, 90, 270 },   // kPrehistoric01
	{ 0, 180, 90, 270 },   // kPrehistoric02
	{ 10, 180, 90, 270 },  // kPrehistoric03
	{ 10, 190, 90, 270 },  // kPrehistoric04
	{ 10, 195, 90, 270 },  // kPrehistoric05
	{ 20, 210, 90, 270 },  // kPrehistoric06
	{ 20, 200, 130, 276 }, // kPrehistoric07
	{ 20, 176, 110, 260 }, // kPrehistoric08
	{ 20, 200, 100, 270 }, // kPrehistoric09
	{ 14, 186, 100, 280 }, // kPrehistoric10
	{ 26, 206, 116, 296 }, // kPrehistoric11
	{ 60, 226, 140, 320 }, // kPrehistoric12
	{ 0, 180, 80, 270 },   // kPrehistoric13
	{ 14, 200, 106, 286 }, // kPrehistoric14
	{ -10, 174, 80, 260 }, // kPrehistoric15
	{ 54, 236, 140, 210 }, // kPrehistoric16
	{ -24, 160, 70, 250 }, // kPrehistoric17
	{ 26, 206, 140, 296 }, // kPrehistoric18
	{ -16, 160, 70, 250 }, // kPrehistoric19
	{ -16, 160, 70, 250 }, // kPrehistoric20
	{ -10, 160, 90, 250 }, // kPrehistoric21
	{ -20, 160, 70, 244 }, // kPrehistoric22
	{ -20, 160, 70, 244 }, // kPrehistoric22North
	{ 60, 234, 150, 330 }, // kPrehistoric23
	{ 50, 230, 140, 320 }, // kPrehistoric24
	{ 60, 240, 140, 330 }  // kPrehistoric25
};

static const TimeValue kPrehistoricFlashlightClickIn = 0;
static const TimeValue kPrehistoricFlashlightClickOut = 138;

static const TimeValue kPrehistoricBumpIntoWallIn = 138;
static const TimeValue kPrehistoricBumpIntoWallOut = 291;

static const TimeValue kBridgeRetractIn = 291;
static const TimeValue kBridgeRetractOut = 1499;

static const TimeValue kPrehistoricWarningTimeLimit = kTenMinutes;

Prehistoric::Prehistoric(InputHandler *nextHandler, PegasusEngine *owner) : Neighborhood(nextHandler, owner, "Prehistoric", kPrehistoricID) {
	setIsItemTaken(kHistoricalLog);
}

uint16 Prehistoric::getDateResID() const {
	return kDatePrehistoricID;
}

void Prehistoric::init() {
	Neighborhood::init();

	// Forces a stop so the flashlight can turn off...
	forceStridingStop(kPrehistoric12, kSouth, kNoAlternateID);
}

void Prehistoric::start() {
	if (g_energyMonitor) {
		g_energyMonitor->stopEnergyDraining();
		g_energyMonitor->restoreLastEnergyValue();
		_vm->resetEnergyDeathReason();
		g_energyMonitor->startEnergyDraining();
	}

	Neighborhood::start();
}

class FinishPrehistoricAction : public AIPlayMessageAction {
public:
	FinishPrehistoricAction() : AIPlayMessageAction("Images/AI/Prehistoric/XP25W", false) {}
	~FinishPrehistoricAction() {}

	void performAIAction(AIRule *);

};

void FinishPrehistoricAction::performAIAction(AIRule *rule) {
	AIPlayMessageAction::performAIAction(rule);
	((PegasusEngine *)g_engine)->die(kPlayerWonGame);
}

void Prehistoric::setUpAIRules() {
	Neighborhood::setUpAIRules();

	if (g_AIArea) {
		if (_vm->isDemo()) {
			FinishPrehistoricAction *doneAction = new FinishPrehistoricAction();
			AIHasItemCondition *hasLogCondition = new AIHasItemCondition(kHistoricalLog);
			AIRule *rule = new AIRule(hasLogCondition, doneAction);
			g_AIArea->addAIRule(rule);
		} else {
			AIPlayMessageAction *messageAction = new AIPlayMessageAction("Images/AI/Prehistoric/XP1NB", false);
			AILocationCondition *locCondition = new AILocationCondition(1);
			locCondition->addLocation(MakeRoomView(kPrehistoric16, kNorth));
			AIRule *rule = new AIRule(locCondition, messageAction);
			g_AIArea->addAIRule(rule);

			messageAction = new AIPlayMessageAction("Images/AI/Prehistoric/XP2SB", false);
			locCondition = new AILocationCondition(1);
			locCondition->addLocation(MakeRoomView(kPrehistoric01, kSouth));
			rule = new AIRule(locCondition, messageAction);
			g_AIArea->addAIRule(rule);

			messageAction = new  AIPlayMessageAction("Images/AI/Prehistoric/XP2SB", false);
			locCondition = new AILocationCondition(1);
			locCondition->addLocation(MakeRoomView(kPrehistoric08, kEast));
			rule = new AIRule(locCondition, messageAction);
			g_AIArea->addAIRule(rule);

			messageAction = new AIPlayMessageAction("Images/AI/Prehistoric/XP2SB", false);
			locCondition = new AILocationCondition(1);
			locCondition->addLocation(MakeRoomView(kPrehistoric25, kWest));
			rule = new AIRule(locCondition, messageAction);
			g_AIArea->addAIRule(rule);

			messageAction = new AIPlayMessageAction("Images/AI/Prehistoric/XP16NB", false);
			locCondition = new AILocationCondition(1);
			locCondition->addLocation(MakeRoomView(kPrehistoric23, kNorth));
			rule = new AIRule(locCondition, messageAction);
			g_AIArea->addAIRule(rule);

			messageAction = new AIPlayMessageAction("Images/AI/Prehistoric/XP18NB", false);
			AITimerCondition *timerCondition = new AITimerCondition(kPrehistoricWarningTimeLimit, 1, true);
			rule = new AIRule(timerCondition, messageAction);
			g_AIArea->addAIRule(rule);

			messageAction = new AIPlayMessageAction("Images/AI/Prehistoric/XP25W", false);
			AIHasItemCondition *hasLogCondition = new AIHasItemCondition(kHistoricalLog);
			rule = new AIRule(hasLogCondition, messageAction);
			g_AIArea->addAIRule(rule);
		}
	}
}

TimeValue Prehistoric::getViewTime(const RoomID room, const DirectionConstant direction) {
	ExtraTable::Entry extra;
	uint32 extraID = 0xffffffff;

	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kPrehistoric02, kSouth):
		if (!GameState.getPrehistoricSeenTimeStream()) {
			getExtraEntry(kPreArrivalFromTSA, extra);
			return extra.movieStart;
		}
		break;
	case MakeRoomView(kPrehistoric25, kEast):
		if (_privateFlags.getFlag(kPrehistoricPrivateVaultOpenFlag)) {
			if (_vm->itemInLocation(kHistoricalLog, kPrehistoricID, kPrehistoric25, kEast))
				extraID = kPre25EastViewWithLog;
			else
				extraID = kPre25EastViewNoLog;
		}
		break;
	}

	if (extraID == 0xffffffff)
		return Neighborhood::getViewTime(room, direction);

	getExtraEntry(extraID, extra);
	return extra.movieEnd - 1;
}


void Prehistoric::findSpotEntry(const RoomID room, const DirectionConstant direction, SpotFlags flags, SpotTable::Entry &entry) {
	Neighborhood::findSpotEntry(room, direction, flags, entry);

	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kPrehistoric01, kSouth):
	case MakeRoomView(kPrehistoric25, kSouth):
		entry.clear();
		break;
	case MakeRoomView(kPrehistoric01, kEast):
		if (GameState.getPrehistoricSeenFlyer1())
			entry.clear();
		else
			GameState.setPrehistoricSeenFlyer1(true);
		break;
	case MakeRoomView(kPrehistoric08, kEast):
		if (GameState.getPrehistoricSeenFlyer2())
			entry.clear();
		else
			GameState.setPrehistoricSeenFlyer2(true);
		break;
	}
}

int16 Prehistoric::getStaticCompassAngle(const RoomID room, const DirectionConstant dir) {
	if (room == kPrehistoricDeath)
		return g_compass->getFaderValue();

	return s_prehistoricCompass[room][dir];
}

void Prehistoric::getExitCompassMove(const ExitTable::Entry &exitEntry, FaderMoveSpec &compassMove) {
	uint32 angle;
	Neighborhood::getExitCompassMove(exitEntry, compassMove);

	switch (MakeRoomView(exitEntry.room, exitEntry.direction)) {
	case MakeRoomView(kPrehistoric01, kNorth):
		compassMove.insertFaderKnot(exitEntry.movieStart + (exitEntry.movieEnd - exitEntry.movieStart) / 2, -10);
		break;
	case MakeRoomView(kPrehistoric06, kEast):
		compassMove.insertFaderKnot(exitEntry.movieStart + (exitEntry.movieEnd - exitEntry.movieStart) / 4, 95);
		compassMove.insertFaderKnot(exitEntry.movieStart + (exitEntry.movieEnd - exitEntry.movieStart) / 4 * 1, 100);
		break;
	case MakeRoomView(kPrehistoric18, kEast):
		if (getCurrentAlternate() == kAltPrehistoricBridgeSet) {
			compassMove.insertFaderKnot(exitEntry.movieStart + kPrehistoricFrameDuration * 11, 145);
			compassMove.insertFaderKnot(exitEntry.movieStart + kPrehistoricFrameDuration * 26, 145);
			compassMove.insertFaderKnot(exitEntry.movieStart + kPrehistoricFrameDuration * 39, 148);
			compassMove.insertFaderKnot(exitEntry.movieStart + kPrehistoricFrameDuration * 114, 140);
		} else {
			compassMove.insertFaderKnot(exitEntry.movieStart + kPrehistoricFrameDuration * 10, 140);
			compassMove.insertFaderKnot(exitEntry.movieStart + kPrehistoricFrameDuration * 16, 145);
			compassMove.insertFaderKnot(exitEntry.movieEnd, 145);
		}
		break;
	case MakeRoomView(kPrehistoric23, kWest):
		angle = compassMove.getNthKnotValue(0);
		compassMove.insertFaderKnot(exitEntry.movieStart + kPrehistoricFrameDuration * 17, angle);
		compassMove.insertFaderKnot(exitEntry.movieStart + kPrehistoricFrameDuration * 32, angle - 90);
		compassMove.insertFaderKnot(exitEntry.movieEnd, angle - 90);
		break;
	}
}

void Prehistoric::turnTo(const DirectionConstant newDirection) {
	setCurrentAlternate(kAltPrehistoricNormal);
	_privateFlags.setFlag(kPrehistoricPrivateVaultOpenFlag, false);
	Neighborhood::turnTo(newDirection);

	Item *keyCard;

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kPrehistoric18, kEast):
		zoomToVault();
		break;
	case MakeRoomView(kPrehistoric18, kNorth):
	case MakeRoomView(kPrehistoric18, kSouth):
		if (_privateFlags.getFlag(kPrehistoricPrivateExtendedBridgeFlag)) {
			playSpotSoundSync(kBridgeRetractIn, kBridgeRetractOut);
			_privateFlags.setFlag(kPrehistoricPrivateExtendedBridgeFlag, false);
			loadAmbientLoops();
		}
		// fall through
	case MakeRoomView(kPrehistoric25, kEast):
		setCurrentActivation(kActivationVaultClosed);
		break;
	case MakeRoomView(kPrehistoric16, kNorth):
	case MakeRoomView(kPrehistoric21, kWest):
		keyCard = _vm->getAllItems().findItemByID(kKeyCard);
		if (keyCard->getItemState() == kFlashlightOff) {
			keyCard->setItemState(kFlashlightOn);
			playSpotSoundSync(kPrehistoricFlashlightClickIn, kPrehistoricFlashlightClickOut);
		}
		break;
	case MakeRoomView(kPrehistoric16, kEast):
	case MakeRoomView(kPrehistoric16, kWest):
	case MakeRoomView(kPrehistoric21, kNorth):
	case MakeRoomView(kPrehistoric21, kSouth):
		keyCard = _vm->getAllItems().findItemByID(kKeyCard);
		if (keyCard->getItemState() == kFlashlightOn) {
			keyCard->setItemState(kFlashlightOff);
			playSpotSoundSync(kPrehistoricFlashlightClickIn, kPrehistoricFlashlightClickOut);
		}
		break;
	}
}

void Prehistoric::zoomToVault() {
	if (!GameState.getPrehistoricSeenBridgeZoom())
		startExtraSequence(kPre18EastZoom, kExtraCompletedFlag, kFilterNoInput);
}

void Prehistoric::checkContinuePoint(const RoomID room, const DirectionConstant direction) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kPrehistoric08, kEast):
	case MakeRoomView(kPrehistoric18, kSouth):
	case MakeRoomView(kPrehistoric16, kNorth):
	case MakeRoomView(kPrehistoric21, kNorth):
	case MakeRoomView(kPrehistoric25, kNorth):
		makeContinuePoint();
		break;
	}
}

void Prehistoric::arriveAt(const RoomID room, const DirectionConstant direction) {
	Item *keyCard;

	if (MakeRoomView(room, direction) == MakeRoomView(kPrehistoric25, kEast) &&
			_privateFlags.getFlag(kPrehistoricPrivateExtendedBridgeFlag)) {
		_navMovie.stop();
		playSpotSoundSync(kBridgeRetractIn, kBridgeRetractOut);
		_privateFlags.setFlag(kPrehistoricPrivateExtendedBridgeFlag, false);
	}

	Neighborhood::arriveAt(room, direction);

	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kPrehistoricDeath, kNorth):
	case MakeRoomView(kPrehistoricDeath, kSouth):
	case MakeRoomView(kPrehistoricDeath, kEast):
	case MakeRoomView(kPrehistoricDeath, kWest):
		if (GameState.getLastRoom() == kPrehistoric23)
			die(kDeathEatenByDinosaur);
		else
			die(kDeathFallOffCliff);
		break;
	case MakeRoomView(kPrehistoric02, kSouth):
		if (!GameState.getPrehistoricSeenTimeStream()) {
			GameState.setPrehistoricTriedToExtendBridge(false);
			GameState.setPrehistoricSeenFlyer1(false);
			GameState.setPrehistoricSeenFlyer2(false);
			GameState.setPrehistoricSeenBridgeZoom(false);
			GameState.setPrehistoricBreakerThrown(false);
			startExtraSequence(kPreArrivalFromTSA, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case MakeRoomView(kPrehistoric18, kEast):
		zoomToVault();
		break;
	case MakeRoomView(kPrehistoric16, kNorth):
		keyCard = _vm->getAllItems().findItemByID(kKeyCard);

		if (keyCard->getItemState() == kFlashlightOff) {
			keyCard->setItemState(kFlashlightOn);
			playSpotSoundSync(kPrehistoricFlashlightClickIn, kPrehistoricFlashlightClickOut);
		}

		if (g_AIArea)
			g_AIArea->checkRules();
		break;
	case MakeRoomView(kPrehistoric01, kSouth):
	case MakeRoomView(kPrehistoric23, kNorth):
		if (g_AIArea)
			g_AIArea->checkRules();
		break;
	case MakeRoomView(kPrehistoric08, kSouth):
	case MakeRoomView(kPrehistoric10, kSouth):
	case MakeRoomView(kPrehistoric12, kSouth):
	case MakeRoomView(kPrehistoric13, kNorth):
	case MakeRoomView(kPrehistoric14, kSouth):
	case MakeRoomView(kPrehistoric15, kNorth):
	case MakeRoomView(kPrehistoric16, kSouth):
	case MakeRoomView(kPrehistoric17, kNorth):
	case MakeRoomView(kPrehistoric18, kSouth):
	case MakeRoomView(kPrehistoric19, kNorth):
	case MakeRoomView(kPrehistoric20, kNorth):
	case MakeRoomView(kPrehistoric21, kEast):
		keyCard = _vm->getAllItems().findItemByID(kKeyCard);

		if (keyCard->getItemState() == kFlashlightOn) {
			keyCard->setItemState(kFlashlightOff);
			playSpotSoundSync(kPrehistoricFlashlightClickIn, kPrehistoricFlashlightClickOut);
		}
		break;
	case MakeRoomView(kPrehistoric25, kEast):
		setCurrentActivation(kActivationVaultClosed);
		break;
	}
}

void Prehistoric::loadAmbientLoops() {
	RoomID room = GameState.getCurrentRoom();

	switch (room) {
	case kPrehistoric02:
		// 1/4 volume.
		if (GameState.getPrehistoricSeenTimeStream())
			loadLoopSound1("Sounds/Prehistoric/P02SAL00.22k.AIFF", 64);
		break;
	case kPrehistoric01:
	case kPrehistoric03:
	case kPrehistoric04:
	case kPrehistoric05:
	case kPrehistoric06:
	case kPrehistoric07:
	case kPrehistoric09:
	case kPrehistoric11:
	case kPrehistoric13:
	case kPrehistoric15:
	case kPrehistoric17:
	case kPrehistoric19:
	case kPrehistoric20:
		// 1/4 volume.
		loadLoopSound1("Sounds/Prehistoric/P02SAL00.22k.AIFF", 64);
		break;
	case kPrehistoric08:
	case kPrehistoric10:
	case kPrehistoric12:
	case kPrehistoric14:
	case kPrehistoric16:
	case kPrehistoric18:
	case kPrehistoric21:
		// 3/16 volume.
		loadLoopSound1("Sounds/Prehistoric/P02SAL00.22k.AIFF", 48);
		break;
	case kPrehistoric25:
		// 1/8 volume.
		loadLoopSound1("Sounds/Prehistoric/P02SAL00.22k.AIFF", 32);
		break;
	case kPrehistoric22:
	case kPrehistoric22North:
	case kPrehistoric23:
	case kPrehistoric24:
	case kPrehistoricDeath:
		// 0 volume.
		loadLoopSound1("");
		break;
	}

	switch (room) {
	case kPrehistoric02:
	case kPrehistoric03:
	case kPrehistoric04:
	case kPrehistoric05:
	case kPrehistoric06:
	case kPrehistoric07:
	case kPrehistoric08:
	case kPrehistoric09:
	case kPrehistoric10:
	case kPrehistoric11:
	case kPrehistoric12:
	case kPrehistoric13:
	case kPrehistoric14:
	case kPrehistoric15:
	case kPrehistoric16:
	case kPrehistoric17:
	case kPrehistoric19:
	case kPrehistoric20:
	case kPrehistoric21:
	case kPrehistoricDeath:
		loadLoopSound2("");
		break;
	case kPrehistoric01:
	case kPrehistoric25:
		loadLoopSound2("Sounds/Prehistoric/VolcLoop.22K.AIFF", 64);
		break;
	case kPrehistoric18:
		if (_privateFlags.getFlag(kPrehistoricPrivateExtendedBridgeFlag))
			loadLoopSound2("Sounds/Prehistoric/P18EAL00.22k.AIFF", 0x100, 0, 0);
		else
			loadLoopSound2("");
		break;
	case kPrehistoric23:
	case kPrehistoric24:
	case kPrehistoric22:
	case kPrehistoric22North:
		loadLoopSound2("Sounds/Prehistoric/P24NAL00.22k.AIFF", 64);
		break;
	}
}

void Prehistoric::activateHotspots() {
	Neighborhood::activateHotspots();

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kPrehistoric18, kEast):
		if (!_privateFlags.getFlag(kPrehistoricPrivateExtendedBridgeFlag))
			_vm->getAllHotspots().activateOneHotspot(kPre18EastSpotID);
		break;
	case MakeRoomView(kPrehistoric22North, kNorth):
		_vm->getAllHotspots().activateOneHotspot(kPre22NorthBreakerSpotID);
		break;
	}
}

void Prehistoric::clickInHotspot(const Input &input, const Hotspot *spot) {
	switch (spot->getObjectID()) {
	case kPre18EastSpotID:
		if (GameState.getPrehistoricBreakerThrown())
			startExtraSequence(kPre18EastBridgeOn, kExtraCompletedFlag, kFilterNoInput);
		else
			startExtraSequence(kPre18EastBridgeOut, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kPre22NorthBreakerSpotID:
		startExtraSequence(kPre22ThrowBreaker, kExtraCompletedFlag, kFilterNoInput);
		break;
	default:
		Neighborhood::clickInHotspot(input, spot);
		break;
	}
}

void Prehistoric::receiveNotification(Notification *notification, const NotificationFlags flags) {
	Neighborhood::receiveNotification(notification, flags);

	if ((flags & kExtraCompletedFlag) != 0) {
		_interruptionFilter = kFilterAllInput;

		switch (_lastExtra) {
		case kPreArrivalFromTSA:
			GameState.setPrehistoricSeenTimeStream(true);
			loadAmbientLoops();
			makeContinuePoint();
			break;
		case kPre18EastZoom:
			startExtraSequence(kPre18EastZoomOut, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kPre18EastZoomOut:
			GameState.setPrehistoricSeenBridgeZoom(true);
			break;
		case kPre18EastBridgeOn:
			_privateFlags.setFlag(kPrehistoricPrivateExtendedBridgeFlag, true);
			setCurrentAlternate(kAltPrehistoricBridgeSet);
			GameState.setPrehistoricTriedToExtendBridge(false);
			loadAmbientLoops();
			GameState.setScoringExtendedBridge(true);
			break;
		case kPre18EastBridgeOut:
			GameState.setPrehistoricTriedToExtendBridge(true);
			if (g_AIArea)
				g_AIArea->checkMiddleArea();
			break;
		case kPre22ThrowBreaker:
			GameState.setPrehistoricBreakerThrown(true);
			GameState.setScoringThrewBreaker(true);
			break;
		case kPre25EastUnlockingVaultNoLog:
		case kPre25EastUnlockingVaultWithLog:
			_vm->addItemToInventory((InventoryItem *)_vm->getAllItems().findItemByID(kJourneymanKey));
			break;
		}
	}

	g_AIArea->checkMiddleArea();
}

Common::String Prehistoric::getBriefingMovie() {
	Common::String movieName = Neighborhood::getBriefingMovie();

	if (movieName.empty())
		movieName = "Images/AI/Prehistoric/XPE";

	return movieName;
}

Common::String Prehistoric::getEnvScanMovie() {
	Common::String movieName = Neighborhood::getEnvScanMovie();

	if (movieName.empty()) {
		if (!_vm->isDemo()) {
			switch (GameState.getCurrentRoom()) {
			case kPrehistoric16:
			case kPrehistoric23:
			case kPrehistoric24:
				return "Images/AI/Prehistoric/XP7WB";
			}
		}

		return "Images/AI/Prehistoric/XP17NB";
	}

	return movieName;
}

uint Prehistoric::getNumHints() {
	uint numHints = Neighborhood::getNumHints();

	if (numHints == 0) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kPrehistoric18, kEast):
			if (!GameState.getPrehistoricBreakerThrown() && GameState.getPrehistoricTriedToExtendBridge() &&
					!_privateFlags.getFlag(kPrehistoricPrivateExtendedBridgeFlag))
				numHints = 1;
			break;
		case MakeRoomView(kPrehistoric25, kEast):
			if (!_privateFlags.getFlag(kPrehistoricPrivateVaultOpenFlag))
				numHints = 1;
			break;
		}
	}

	return numHints;
}

Common::String Prehistoric::getHintMovie(uint hintNum) {
	Common::String movieName = Neighborhood::getHintMovie(hintNum);

	if (movieName.empty()) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kPrehistoric18, kEast):
			return "Images/AI/Prehistoric/XP18WD";
		case MakeRoomView(kPrehistoric25, kEast):
			return "Images/AI/Globals/XGLOB1A";
		}
	}

	return movieName;
}

bool Prehistoric::canSolve() {
	return	GameState.getCurrentRoomAndView() == MakeRoomView(kPrehistoric18, kEast) &&
			!GameState.getPrehistoricBreakerThrown() &&
			GameState.getPrehistoricTriedToExtendBridge() &&
			!_privateFlags.getFlag(kPrehistoricPrivateExtendedBridgeFlag);
}

void Prehistoric::doSolve() {
	GameState.setPrehistoricBreakerThrown(true);
	startExtraSequence(kPre18EastBridgeOn, kExtraCompletedFlag, kFilterNoInput);
}

Hotspot *Prehistoric::getItemScreenSpot(Item *item, DisplayElement *element) {
	if (item->getObjectID() == kHistoricalLog)
		return _vm->getAllHotspots().findHotspotByID(kPrehistoricHistoricalLogSpotID);

	return Neighborhood::getItemScreenSpot(item, element);
}

void Prehistoric::pickedUpItem(Item *item) {
	switch (item->getObjectID()) {
	case kHistoricalLog:
		GameState.setScoringGotHistoricalLog(true);
		break;
	}

	Neighborhood::pickedUpItem(item);
}

void Prehistoric::dropItemIntoRoom(Item *item, Hotspot *dropSpot) {
	switch (item->getObjectID()) {
	case kJourneymanKey:
		Neighborhood::dropItemIntoRoom(item, dropSpot);

		if (GameState.isTakenItemID(kHistoricalLog))
			startExtraLongSequence(kPre25EastUnlockingVaultNoLog, kPre25EastVaultOpenNoLog, kExtraCompletedFlag, kFilterNoInput);
		else
			startExtraLongSequence(kPre25EastUnlockingVaultWithLog, kPre25EastVaultOpenWithLog, kExtraCompletedFlag, kFilterNoInput);

		_privateFlags.setFlag(kPrehistoricPrivateVaultOpenFlag, true);
		setCurrentActivation(kActivationVaultOpen);
		break;
	default:
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		break;
	}
}

void Prehistoric::bumpIntoWall() {
	requestSpotSound(kPrehistoricBumpIntoWallIn, kPrehistoricBumpIntoWallOut, kFilterAllInput, 0);
	Neighborhood::bumpIntoWall();
}

Common::String Prehistoric::getNavMovieName() {
	return "Images/Prehistoric/Prehistoric.movie";
}

Common::String Prehistoric::getSoundSpotsName() {
	return "Sounds/Prehistoric/Prehistoric Spots";
}

} // End of namespace Pegasus
