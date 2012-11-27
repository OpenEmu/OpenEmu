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

#include "common/events.h"
#include "video/qt_decoder.h"

#include "pegasus/cursor.h"
#include "pegasus/energymonitor.h"
#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/opticalchip.h"
#include "pegasus/items/biochips/shieldchip.h"
#include "pegasus/items/inventory/airmask.h"
#include "pegasus/neighborhood/mars/mars.h"

namespace Pegasus {

// This should really be 22.5.
// Probably no one will know the difference.
static const int16 kMarsShieldPanelOffsetAngle = 22;

static const CanMoveForwardReason kCantMoveRobotBlocking = kCantMoveLastReason + 1;

static const NotificationFlags kTimeForCanyonChaseFlag = kLastNeighborhoodNotificationFlag << 1;
static const NotificationFlags kExplosionFinishedFlag = kTimeForCanyonChaseFlag << 1;
static const NotificationFlags kTimeToTransportFlag = kExplosionFinishedFlag << 1;

static const NotificationFlags kMarsNotificationFlags = kTimeForCanyonChaseFlag |
													kExplosionFinishedFlag |
													kTimeToTransportFlag;

static const TimeValue kLittleExplosionStart = 0 * 40;
static const TimeValue kLittleExplosionStop = 24 * 40;

static const TimeValue kBigExplosionStart = 24 * 40;
static const TimeValue kBigExplosionStop = 62 * 40;

enum {
	kMaze007RobotLoopingEvent,
	kMaze015RobotLoopingEvent,
	kMaze101RobotLoopingEvent,
	kMaze104RobotLoopingEvent,
	kMaze133RobotLoopingEvent,
	kMaze136RobotLoopingEvent,
	kMaze184RobotLoopingEvent
};

enum {
	kMaze007RobotLoopingTime = (64 + 96) * kMarsFrameDuration,
	kMaze015RobotLoopingTime = (64 + 93) * kMarsFrameDuration,
	kMaze101RobotLoopingTime = (64 + 45) * kMarsFrameDuration,
	kMaze104RobotLoopingTime = 96 * kMarsFrameDuration,
	kMaze133RobotLoopingTime = (64 + 96) * kMarsFrameDuration,
	kMaze136RobotLoopingTime = (64 + 96) * kMarsFrameDuration,
	kMaze184RobotLoopingTime = 96 * kMarsFrameDuration
};

// I've made a couple macros for these rects so we don't
// have to globally construct them or whatnot
#define kShuttleEnergyBeamBounds Common::Rect(24, 27, 24 + 112, 27 + 46)
#define kShuttleGravitonBounds Common::Rect(24, 73, 24 + 112, 73 + 30)
#define kShuttleTractorBounds Common::Rect(24, 103, 24 + 112, 103 + 30)
#define kShuttleTransportBounds Common::Rect(484, 353, 89 + 484, 79 + 353)

void MarsTimerEvent::fire() {
	mars->marsTimerExpired(*this);
}

Mars::Mars(InputHandler *nextHandler, PegasusEngine *owner) : Neighborhood(nextHandler, owner, "Mars", kMarsID),
		_guessObject(kNoDisplayElement), _undoPict(kNoDisplayElement), _guessHistory(kNoDisplayElement),
		_choiceHighlight(kNoDisplayElement), _shuttleInterface1(kNoDisplayElement), _shuttleInterface2(kNoDisplayElement),
		_shuttleInterface3(kNoDisplayElement), _shuttleInterface4(kNoDisplayElement), _canyonChaseMovie(kNoDisplayElement),
		_leftShuttleMovie(kNoDisplayElement), _rightShuttleMovie(kNoDisplayElement), _lowerLeftShuttleMovie(kNoDisplayElement),
		_lowerRightShuttleMovie(kNoDisplayElement), _centerShuttleMovie(kNoDisplayElement),
		_upperLeftShuttleMovie(kNoDisplayElement), _upperRightShuttleMovie(kNoDisplayElement),
		_leftDamageShuttleMovie(kNoDisplayElement), _rightDamageShuttleMovie(kNoDisplayElement), _explosions(kNoDisplayElement),
		_planetMovie(kNoDisplayElement), _junk(kNoDisplayElement), _energyChoiceSpot(kShuttleEnergySpotID),
		_gravitonChoiceSpot(kShuttleGravitonSpotID), _tractorChoiceSpot(kShuttleTractorSpotID),
		_shuttleViewSpot(kShuttleViewSpotID), _shuttleTransportSpot(kShuttleTransportSpotID) {
	_noAirFuse.setFunctor(new Common::Functor0Mem<void, Mars>(this, &Mars::airStageExpired));
	setIsItemTaken(kMarsCard);
	setIsItemTaken(kAirMask);
	setIsItemTaken(kCrowbar);
	setIsItemTaken(kCardBomb);
}

Mars::~Mars() {
	_vm->getAllHotspots().remove(&_energyChoiceSpot);
	_vm->getAllHotspots().remove(&_gravitonChoiceSpot);
	_vm->getAllHotspots().remove(&_tractorChoiceSpot);
	_vm->getAllHotspots().remove(&_shuttleViewSpot);
	_vm->getAllHotspots().remove(&_shuttleTransportSpot);
}

void Mars::init() {
	Neighborhood::init();

	Hotspot *attackSpot = _vm->getAllHotspots().findHotspotByID(kAttackRobotHotSpotID);
	attackSpot->setMaskedHotspotFlags(kDropItemSpotFlag, kDropItemSpotFlag);
	_attackingItem = NULL;

	forceStridingStop(kMars08, kNorth, kAltMarsNormal);

	_neighborhoodNotification.notifyMe(this, kMarsNotificationFlags, kMarsNotificationFlags);

	_explosionCallBack.setNotification(&_neighborhoodNotification);
	_explosionCallBack.setCallBackFlag(kExplosionFinishedFlag);

	_weaponSelection = kNoWeapon;
}

void Mars::flushGameState() {
	g_energyMonitor->saveCurrentEnergyValue();
}

void Mars::start() {
	g_energyMonitor->stopEnergyDraining();
	g_energyMonitor->restoreLastEnergyValue();
	_vm->resetEnergyDeathReason();
	g_energyMonitor->startEnergyDraining();
	Neighborhood::start();
}

class AirMaskCondition : public AICondition {
public:
	AirMaskCondition(const uint32);

	virtual bool fireCondition();

protected:
	uint32 _airThreshold;
	uint32 _lastAirLevel;
};

AirMaskCondition::AirMaskCondition(const uint32 airThreshold) {
	_airThreshold = airThreshold;
	_lastAirLevel = g_airMask->getAirLeft();
}

bool AirMaskCondition::fireCondition() {
	bool result = g_airMask && g_airMask->isAirMaskOn() &&
			g_airMask->getAirLeft() <= _airThreshold && _lastAirLevel > _airThreshold;

	_lastAirLevel = g_airMask->getAirLeft();
	return result;
}

void Mars::setUpAIRules() {
	Neighborhood::setUpAIRules();

	// Don't add these rules if we're going to the robot's shuttle...
	if (g_AIArea && !GameState.getMarsReadyForShuttleTransport()) {
		AIPlayMessageAction *messageAction = new AIPlayMessageAction("Images/AI/Globals/XGLOB1E", false);
		AILocationCondition *locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars47, kSouth));
		AIRule *rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Mars/XM27NB", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars27, kNorth));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Mars/XM27NB", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars28, kNorth));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Mars/XM41ED", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars19, kEast));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		AIDeactivateRuleAction *deactivate = new AIDeactivateRuleAction(rule);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars35, kWest));
		rule = new AIRule(locCondition, deactivate);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Mars/XM41ED", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars48, kWest));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		AirMaskCondition *airMask50Condition = new AirMaskCondition(50);
		messageAction = new AIPlayMessageAction("Images/AI/Mars/XMMAZB1", false);
		AIRule *rule50 = new AIRule(airMask50Condition, messageAction);

		AirMaskCondition *airMask25Condition = new AirMaskCondition(25);
		AICompoundAction *compound = new AICompoundAction();
		messageAction = new AIPlayMessageAction("Images/AI/Mars/XMMAZB2", false);
		compound->addAction(messageAction);
		deactivate = new AIDeactivateRuleAction(rule50);
		compound->addAction(deactivate);
		AIRule *rule25 = new AIRule(airMask25Condition, compound);

		AirMaskCondition *airMask5Condition = new AirMaskCondition(5);
		compound = new AICompoundAction;
		messageAction = new AIPlayMessageAction("Images/AI/Mars/XMMAZB3", false);
		compound->addAction(messageAction);
		deactivate = new AIDeactivateRuleAction(rule50);
		compound->addAction(deactivate);
		deactivate = new AIDeactivateRuleAction(rule25);
		compound->addAction(deactivate);
		AIRule *rule5 = new AIRule(airMask5Condition, compound);

		g_AIArea->addAIRule(rule5);
		g_AIArea->addAIRule(rule25);
		g_AIArea->addAIRule(rule50);

		messageAction = new AIPlayMessageAction("Images/AI/Mars/XM51ND", false);
		AIDoorOpenedCondition *doorOpen = new AIDoorOpenedCondition(MakeRoomView(kMars51, kEast));
		rule = new AIRule(doorOpen, messageAction);
		g_AIArea->addAIRule(rule);
	}
}

uint16 Mars::getDateResID() const {
	return kDate2185ID;
}

TimeValue Mars::getViewTime(const RoomID room, const DirectionConstant direction) {
	ExtraTable::Entry extra;
	SpotTable::Entry spotEntry;
	uint32 extraID = 0xffffffff;

	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kMars0A, kNorth):
		if (!GameState.getMarsSeenTimeStream()) {
			getExtraEntry(kMarsArrivalFromTSA, extra);
			return extra.movieStart;
		}
		break;
	case MakeRoomView(kMars31South, kSouth):
		if (GameState.isTakenItemID(kMarsCard))
			extraID = kMars31SouthZoomViewNoCard;
		break;
	case MakeRoomView(kMars31, kSouth):
		if (GameState.isTakenItemID(kMarsCard))
			extraID = kMars31SouthViewNoCard;
		break;
	case MakeRoomView(kMars34, kSouth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			if (GameState.isTakenItemID(kCrowbar))
				extraID = kMars34ViewOpenNoBar;
			else
				extraID = kMars34ViewOpenWithBar;
		}
		break;
	case MakeRoomView(kMars36, kSouth):
	case MakeRoomView(kMars37, kSouth):
	case MakeRoomView(kMars38, kSouth):
		findSpotEntry(room, direction, kSpotOnTurnMask | kSpotLoopsMask, spotEntry);
		return spotEntry.movieStart;
	case MakeRoomView(kMars45, kNorth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			if (GameState.isTakenItemID(kCrowbar))
				extraID = kMars45ViewOpenNoBar;
			else
				extraID = kMars45ViewOpenWithBar;
		}
		break;
	case MakeRoomView(kMars48, kEast):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot())
			extraID = kMars48RobotView;
		break;
	case MakeRoomView(kMars56, kEast):
		if (_privateFlags.getFlag(kMarsPrivateBombExposedFlag)) {
			if (_privateFlags.getFlag(kMarsPrivateDraggingBombFlag))
				extraID = kMars57ViewOpenNoBomb;
			else
				extraID = kMars57ExposeBomb;
		} else if (GameState.getMarsLockBroken()) {
			extraID = kMars57OpenPanelChoices;
		} else if (GameState.getMarsLockFrozen()) {
			extraID = kMars57LockFrozenView;
		}
		break;
	case MakeRoomView(kMarsRobotShuttle, kEast):
		if (getCurrentActivation() == kActivationRobotHeadOpen) {
			extraID = kMarsRobotHead111;

			if (_privateFlags.getFlag(kMarsPrivateGotMapChipFlag))
				extraID -= 1;
			if (_privateFlags.getFlag(kMarsPrivateGotOpticalChipFlag))
				extraID -= 2;
			if (_privateFlags.getFlag(kMarsPrivateGotShieldChipFlag))
				extraID -= 4;
		}
		break;
	}

	if (extraID == 0xffffffff)
		return Neighborhood::getViewTime(room, direction);

	getExtraEntry(extraID, extra);
	return extra.movieEnd - 1;
}

void Mars::getZoomEntry(const HotSpotID spotID, ZoomTable::Entry &entry) {
	Neighborhood::getZoomEntry(spotID, entry);

	uint32 extraID = 0xffffffff;

	switch (spotID) {
	case kMars31SouthSpotID:
		if (GameState.getCurrentDirection() == kSouth && GameState.isTakenItemID(kMarsCard))
			extraID = kMars31SouthZoomInNoCard;
		break;
	case kMars31SouthOutSpotID:
		if (GameState.getCurrentDirection() == kSouth && GameState.isTakenItemID(kMarsCard))
			extraID = kMars31SouthZoomOutNoCard;
		break;
	}

	if (extraID != 0xffffffff) {
		ExtraTable::Entry extra;
		getExtraEntry(extraID, extra);
		entry.movieStart = extra.movieStart;
		entry.movieEnd = extra.movieEnd;
	}
}

void Mars::findSpotEntry(const RoomID room, const DirectionConstant direction, SpotFlags flags, SpotTable::Entry &entry) {
	Neighborhood::findSpotEntry(room, direction, flags, entry);

	if ((flags & (kSpotOnArrivalMask | kSpotOnTurnMask)) != 0) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kMars27, kNorth):
			if (GameState.getMarsSeenThermalScan())
				entry.clear();
			else
				GameState.setMarsSeenThermalScan(true);
			break;
		case MakeRoomView(kMars28, kNorth):
			if (GameState.getMarsSeenThermalScan())
				entry.clear();
			else
				GameState.setMarsSeenThermalScan(true);
			break;
		}
	}
}

CanMoveForwardReason Mars::canMoveForward(ExitTable::Entry &entry) {
	CanMoveForwardReason reason = Neighborhood::canMoveForward(entry);

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars48, kEast):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot())
			reason = kCantMoveRobotBlocking;
		break;
	case MakeRoomView(kMars48, kSouth):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot())
			_utilityFuse.stopFuse();
		break;
	}

	return reason;
}

void Mars::cantMoveThatWay(CanMoveForwardReason reason) {
	if (reason == kCantMoveRobotBlocking) {
		startExtraSequence(kMars48RobotKillsPlayer, kExtraCompletedFlag, kFilterNoInput);
		loadLoopSound2("");
	} else {
		Neighborhood::cantMoveThatWay(reason);
	}
}

void Mars::moveForward() {
	if (GameState.getCurrentRoom() == kMars02 || (GameState.getCurrentRoom() >= kMars05 && GameState.getCurrentRoom() <= kMars08))
		loadLoopSound2("");

	Neighborhood::moveForward();
}

void Mars::bumpIntoWall() {
	requestSpotSound(kMarsBumpIntoWallIn, kMarsBumpIntoWallOut, kFilterNoInput, 0);
	Neighborhood::bumpIntoWall();
}

CanOpenDoorReason Mars::canOpenDoor(DoorTable::Entry &entry) {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars05, kEast):
	case MakeRoomView(kMars06, kEast):
	case MakeRoomView(kMars07, kEast):
		if (!GameState.getMarsSecurityDown())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze037, kWest):
	case MakeRoomView(kMarsMaze038, kEast):
		if (GameState.getMarsMazeDoorPair1())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze050, kNorth):
	case MakeRoomView(kMarsMaze058, kSouth):
		if (!GameState.getMarsMazeDoorPair1())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze047, kNorth):
	case MakeRoomView(kMarsMaze142, kSouth):
		if (GameState.getMarsMazeDoorPair2())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze057, kNorth):
	case MakeRoomView(kMarsMaze136, kSouth):
		if (!GameState.getMarsMazeDoorPair2())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze120, kWest):
	case MakeRoomView(kMarsMaze121, kEast):
		if (GameState.getMarsMazeDoorPair3())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze081, kNorth):
	case MakeRoomView(kMarsMaze083, kSouth):
		if (!GameState.getMarsMazeDoorPair3())
			return kCantOpenLocked;
		break;
	}

	return Neighborhood::canOpenDoor(entry);
}

void Mars::cantOpenDoor(CanOpenDoorReason reason) {
	switch (GameState.getCurrentRoom()) {
	case kMars05:
	case kMars06:
	case kMars07:
		playSpotSoundSync(kMarsCantOpenShuttleIn, kMarsCantOpenShuttleOut);
		break;
	default:
		Neighborhood::cantOpenDoor(reason);
		break;
	}
}

void Mars::openDoor() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars06, kEast):
	case MakeRoomView(kMars07, kEast):
		if (GameState.getMarsSecurityDown())
			playSpotSoundSync(kMarsNoShuttleIn, kMarsNoShuttleOut);
		break;
	case MakeRoomView(kMars47, kSouth):
		if (GameState.isTakenItemID(kAirMask))
			setCurrentAlternate(kAltMarsTookMask);
		else
			setCurrentAlternate(kAltMarsNormal);
		break;
	case MakeRoomView(kMars48, kNorth):
		if (GameState.getMarsPodAtUpperPlatform())
			setCurrentAlternate(kAltMarsNormal);
		else
			setCurrentAlternate(kAltMarsPodAtMars45);
		break;
	case MakeRoomView(kMars48, kEast):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot()) {
			die(kDeathDidntGetOutOfWay);
			return;
		}
		break;
	}

	Neighborhood::openDoor();
}

void Mars::doorOpened() {
	switch (GameState.getCurrentRoom()) {
	case kMars27:
	case kMars28:
		if (GameState.getCurrentDirection() == kNorth)
			_vm->die(kDeathArrestedInMars);
		else
			Neighborhood::doorOpened();
		break;
	case kMars41:
	case kMars42:
		if (GameState.getCurrentDirection() == kEast)
			_vm->die(kDeathWrongShuttleLock);
		else
			Neighborhood::doorOpened();
		break;
	case kMars51:
		Neighborhood::doorOpened();
		setUpReactorEnergyDrain();

		if (g_AIArea)
			g_AIArea->checkRules();
		break;
	case kMars19:
		if (GameState.getCurrentDirection() == kEast)
			GameState.setMarsAirlockOpen(true);

		Neighborhood::doorOpened();
		break;
	case kMars48:
		if (GameState.getCurrentDirection() == kWest)
			GameState.setMarsAirlockOpen(true);

		Neighborhood::doorOpened();
		break;
	default:
		Neighborhood::doorOpened();
		break;
	}
}

void Mars::setUpReactorEnergyDrain() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars51, kEast):
		if (GameState.isCurrentDoorOpen()) {
			if (g_energyMonitor->getEnergyDrainRate() == kEnergyDrainNormal) {
				if (GameState.getShieldOn()) {
					g_shield->setItemState(kShieldRadiation);
					g_energyMonitor->setEnergyDrainRate(kMarsReactorEnergyDrainWithShield);
				} else {
					g_energyMonitor->setEnergyDrainRate(kMarsReactorEnergyDrainNoShield);
				}
				_vm->setEnergyDeathReason(kDeathReactorBurn);
			}
		} else {
			if (g_energyMonitor->getEnergyDrainRate() != kEnergyDrainNormal) {
				if (GameState.getShieldOn())
					g_shield->setItemState(kShieldNormal);
				g_energyMonitor->setEnergyDrainRate(kEnergyDrainNormal);
				_vm->resetEnergyDeathReason();
			}
		}
		break;
	case MakeRoomView(kMars52, kNorth):
	case MakeRoomView(kMars52, kSouth):
	case MakeRoomView(kMars52, kEast):
	case MakeRoomView(kMars52, kWest):
	case MakeRoomView(kMars54, kNorth):
	case MakeRoomView(kMars54, kSouth):
	case MakeRoomView(kMars54, kEast):
	case MakeRoomView(kMars54, kWest):
	case MakeRoomView(kMars56, kNorth):
	case MakeRoomView(kMars56, kSouth):
	case MakeRoomView(kMars56, kEast):
	case MakeRoomView(kMars56, kWest):
	case MakeRoomView(kMars58, kNorth):
	case MakeRoomView(kMars58, kSouth):
	case MakeRoomView(kMars58, kEast):
	case MakeRoomView(kMars58, kWest):
		if (g_energyMonitor->getEnergyDrainRate() == kEnergyDrainNormal) {
			if (GameState.getShieldOn()) {
				g_shield->setItemState(kShieldRadiation);
				g_energyMonitor->setEnergyDrainRate(kMarsReactorEnergyDrainWithShield);
			} else {
				g_energyMonitor->setEnergyDrainRate(kMarsReactorEnergyDrainNoShield);
			}
			_vm->setEnergyDeathReason(kDeathReactorBurn);
		}
		break;
	default:
		if (g_energyMonitor->getEnergyDrainRate() != kEnergyDrainNormal) {
			if (GameState.getShieldOn())
				g_shield->setItemState(kShieldNormal);
			g_energyMonitor->setEnergyDrainRate(kEnergyDrainNormal);
			_vm->resetEnergyDeathReason();
		}
		break;
	}
}

void Mars::closeDoorOffScreen(const RoomID room, const DirectionConstant direction) {
	switch (room) {
	case kMars51:
		playSpotSoundSync(kMarsGantryDoorCloseIn, kMarsGantryDoorCloseOut);
		if (GameState.getShieldOn())
			g_shield->setItemState(kShieldNormal);
		g_energyMonitor->setEnergyDrainRate(kEnergyDrainNormal);
		_vm->resetEnergyDeathReason();
		break;
	case kMars05:
	case kMars06:
	case kMars07:
	case kMars13:
	case kMars22:
	case kMars47:
	case kMars52:
		playSpotSoundSync(kMarsGantryDoorCloseIn, kMarsGantryDoorCloseOut);
		break;
	case kMars18:
	case kMars32:
		playSpotSoundSync(kMarsTransportDoorCloseIn, kMarsTransportDoorCloseOut);
		break;
	case kMars19:
		if (GameState.getCurrentRoom() != kMars35) {
			playSpotSoundSync(kMarsBigAirlockDoorCloseIn, kMarsBigAirlockDoorCloseOut);
			GameState.setMarsAirlockOpen(false);
		}
		break;
	case kMars36:
		if (GameState.getCurrentRoom() != kMars35)
			playSpotSoundSync(kMarsSmallAirlockDoorCloseIn, kMarsSmallAirlockDoorCloseOut);
		break;
	case kMars48:
		if (direction == kWest) {
			if (GameState.getCurrentRoom() != kMars60) {
				playSpotSoundSync(kMarsSmallAirlockDoorCloseIn, kMarsSmallAirlockDoorCloseOut);
				GameState.setMarsAirlockOpen(false);
			}
		} else {
			playSpotSoundSync(kMarsGantryDoorCloseIn, kMarsGantryDoorCloseOut);
		}
		break;
	case kMars41:
	case kMars42:
	case kMars43:
		if (direction == kWest)
			playSpotSoundSync(kMarsGantryDoorCloseIn, kMarsGantryDoorCloseOut);
		break;
	case kMarsMaze037:
	case kMarsMaze038:
	case kMarsMaze012:
	case kMarsMaze066:
	case kMarsMaze050:
	case kMarsMaze058:
	case kMarsMaze057:
	case kMarsMaze136:
	case kMarsMaze047:
	case kMarsMaze142:
	case kMarsMaze133:
	case kMarsMaze132:
	case kMarsMaze113:
	case kMarsMaze114:
	case kMarsMaze120:
	case kMarsMaze121:
	case kMarsMaze081:
	case kMarsMaze083:
	case kMarsMaze088:
	case kMarsMaze089:
	case kMarsMaze179:
	case kMarsMaze180:
		playSpotSoundSync(kMarsMazeDoorCloseIn, kMarsMazeDoorCloseOut);
		break;
	}
}

void Mars::checkAirlockDoors() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars19, kWest):
	case MakeRoomView(kMars18, kWest):
	case MakeRoomView(kMars17, kWest):
	case MakeRoomView(kMars16, kWest):
	case MakeRoomView(kMars15, kWest):
	case MakeRoomView(kMars14, kWest):
	case MakeRoomView(kMars12, kWest):
	case MakeRoomView(kMars11, kWest):
	case MakeRoomView(kMars10, kWest):
		if (GameState.getMarsInAirlock()) {
			playSpotSoundSync(kMarsBigAirlockDoorCloseIn, kMarsBigAirlockDoorCloseOut);
			GameState.setMarsInAirlock(false);
		}
		break;
	case MakeRoomView(kMars36, kEast):
	case MakeRoomView(kMars37, kEast):
	case MakeRoomView(kMars38, kEast):
	case MakeRoomView(kMars39, kEast):
	case MakeRoomView(kMars48, kEast):
	case MakeRoomView(kMars50, kEast):
	case MakeRoomView(kMars51, kEast):
	case MakeRoomView(kMars52, kEast):
		if (GameState.getMarsInAirlock()) {
			playSpotSoundSync(kMarsSmallAirlockDoorCloseIn, kMarsSmallAirlockDoorCloseOut);
			GameState.setMarsInAirlock(false);
		}
		break;
	case MakeRoomView(kMars35, kWest):
	case MakeRoomView(kMars35, kEast):
	case MakeRoomView(kMars60, kWest):
	case MakeRoomView(kMars60, kEast):
		GameState.setMarsInAirlock(true);
		break;
	default:
		GameState.setMarsInAirlock(false);
		break;
	}
}

int16 Mars::getStaticCompassAngle(const RoomID room, const DirectionConstant dir) {
	int16 angle = Neighborhood::getStaticCompassAngle(room, dir);

	switch (MakeRoomView(room, dir)) {
	case MakeRoomView(kMars0A, kNorth):
		angle -= 20;
		break;
	case MakeRoomView(kMars23, kNorth):
	case MakeRoomView(kMars23, kSouth):
	case MakeRoomView(kMars23, kEast):
	case MakeRoomView(kMars23, kWest):
	case MakeRoomView(kMars26, kNorth):
	case MakeRoomView(kMars26, kSouth):
	case MakeRoomView(kMars26, kEast):
	case MakeRoomView(kMars26, kWest):
		angle += 30;
		break;
	case MakeRoomView(kMars24, kNorth):
	case MakeRoomView(kMars24, kSouth):
	case MakeRoomView(kMars24, kEast):
	case MakeRoomView(kMars24, kWest):
	case MakeRoomView(kMars25, kNorth):
	case MakeRoomView(kMars25, kSouth):
	case MakeRoomView(kMars25, kEast):
	case MakeRoomView(kMars25, kWest):
		angle -= 30;
		break;
	case MakeRoomView(kMars54, kNorth):
	case MakeRoomView(kMars54, kSouth):
	case MakeRoomView(kMars54, kEast):
	case MakeRoomView(kMars54, kWest):
		angle += 90;
		break;
	case MakeRoomView(kMars56, kNorth):
	case MakeRoomView(kMars56, kSouth):
	case MakeRoomView(kMars56, kEast):
	case MakeRoomView(kMars56, kWest):
		angle += 180;
		break;
	case MakeRoomView(kMars58, kNorth):
	case MakeRoomView(kMars58, kSouth):
	case MakeRoomView(kMars58, kEast):
	case MakeRoomView(kMars58, kWest):
		angle -= 90;
		break;
	}

	return angle;
}

void Mars::getExitCompassMove(const ExitTable::Entry &exitEntry, FaderMoveSpec &compassMove) {
	Neighborhood::getExitCompassMove(exitEntry, compassMove);

	if (exitEntry.room == kMars43 && exitEntry.direction == kEast) {
		compassMove.insertFaderKnot(exitEntry.movieStart + 16 * kMarsFrameDuration, 90);
		compassMove.insertFaderKnot(exitEntry.movieStart + 32 * kMarsFrameDuration, 270);
	} else if (exitEntry.room == kMars46 && exitEntry.direction == kWest && exitEntry.altCode != kAltMarsPodAtMars45) {
		compassMove.makeTwoKnotFaderSpec(kMarsMovieScale, exitEntry.movieStart, 270, exitEntry.movieEnd, 360);
		compassMove.insertFaderKnot(exitEntry.movieStart + 43 * kMarsFrameDuration, 270);
		compassMove.insertFaderKnot(exitEntry.movieStart + 58 * kMarsFrameDuration, 360);
	}
}

void Mars::getExtraCompassMove(const ExtraTable::Entry &entry, FaderMoveSpec &compassMove) {
	switch (entry.extra) {
	case kMarsTakePodToMars45:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, 0, entry.movieEnd, 180);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 3), 30);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 11), 10);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 14), 40);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 16), 30);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 23), 100);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 31), 70);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 34), 100);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 37), 85);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 42), 135);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 44), 125);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 46), 145);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 49), 160);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 51), 180);
		break;
	case kMars35WestSpinAirlockToEast:
	case kMars60WestSpinAirlockToEast:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, 90, entry.movieEnd, 270);
		compassMove.insertFaderKnot(entry.movieStart + kMarsMovieScale, 90);
		compassMove.insertFaderKnot(entry.movieStart + kMarsMovieScale * 3, 270);
		break;
	case kMars35EastSpinAirlockToWest:
	case kMars60EastSpinAirlockToWest:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, 270, entry.movieEnd, 90);
		compassMove.insertFaderKnot(entry.movieStart + kMarsMovieScale, 270);
		compassMove.insertFaderKnot(entry.movieStart + kMarsMovieScale * 3, 90);
		break;
	case kMars52SpinLeft:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars52Compass, entry.movieEnd, kMars54Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars54Compass);
		break;
	case kMars52SpinRight:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars52Compass, entry.movieEnd, kMars58Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars58Compass);
		break;
	case kMars52Extend:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars52Compass,
				entry.movieEnd, kMars52Compass + kMarsShieldPanelOffsetAngle);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 60, kMars52Compass + kMarsShieldPanelOffsetAngle);
		break;
	case kMars53Retract:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart,
				kMars52Compass + kMarsShieldPanelOffsetAngle, entry.movieEnd, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars52Compass + kMarsShieldPanelOffsetAngle);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 60, kMars52Compass);
		break;
	case kMars56ExtendWithBomb:
	case kMars56ExtendNoBomb:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars56Compass,
				entry.movieEnd, kMars56Compass - kMarsShieldPanelOffsetAngle);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 60, kMars56Compass - kMarsShieldPanelOffsetAngle);
		break;
	case kMars57RetractWithBomb:
	case kMars57RetractNoBomb:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart,
				kMars56Compass - kMarsShieldPanelOffsetAngle, entry.movieEnd, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars56Compass - kMarsShieldPanelOffsetAngle);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 60, kMars56Compass);
		break;
	case kMars54SpinLeft:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars54Compass, entry.movieEnd, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars54Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars56Compass);
		break;
	case kMars54SpinRight:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars54Compass, entry.movieEnd, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars54Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars52Compass);
		break;
	case kMars56SpinLeft:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars56Compass,
				entry.movieEnd, kMars58Compass + 360);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars58Compass + 360);
		break;
	case kMars56SpinRight:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars56Compass, entry.movieEnd, kMars54Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars54Compass);
		break;
	case kMars58SpinLeft:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars58Compass,
				entry.movieEnd, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars58Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars52Compass);
		break;
	case kMars58SpinRight:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart,
				kMars58Compass + 360, entry.movieEnd, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars58Compass + 360);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars56Compass);
		break;
	default:
		Neighborhood::getExtraCompassMove(entry, compassMove);
	}
}

void Mars::loadAmbientLoops() {
	RoomID room = GameState.getCurrentRoom();

	if ((room >= kMars0A && room <= kMars21) || (room >= kMars41 && room <= kMars43)) {
		if (GameState.getMarsSeenTimeStream())
			loadLoopSound1("Sounds/Mars/Gantry Ambient.22K.8.AIFF");
	} else if (room >= kMars22 && room <= kMars31South) {
		loadLoopSound1("Sounds/Mars/Reception.02.22K.8.AIFF", 0x100 / 4);
	} else if (room >= kMars32 && room <= kMars34) {
		loadLoopSound1("Sounds/Mars/Pod Room Ambient.22K.8.AIFF");
	} else if (room == kMars35) {
		if (getAirQuality(room) == kAirQualityVacuum)
			loadLoopSound1("Sounds/Mars/Gear Room Ambient.22K.8.AIFF");
		else
			loadLoopSound1("Sounds/Mars/Gantry Ambient.22K.8.AIFF", 0x100 / 2);
	} else if (room >= kMars36 && room <= kMars39) {
		loadLoopSound1("Sounds/Mars/Gear Room Ambient.22K.8.AIFF");
	} else if (room >= kMars45 && room <= kMars51) {
		loadLoopSound1("Sounds/Mars/Lower Mars Ambient.22K.8.AIFF");
	} else if (room >= kMars52 && room <= kMars58) {
		loadLoopSound1("Sounds/Mars/ReactorLoop.22K.8.AIFF");
	} else if (room == kMars60) {
		if (getAirQuality(room) == kAirQualityVacuum)
			loadLoopSound1("Sounds/Mars/Mars Maze Ambient.22K.8.AIFF");
		else
			loadLoopSound1("Sounds/Mars/Lower Mars Ambient.22K.8.AIFF", 0x100 / 2);
	} else if (room >= kMarsMaze004 && room <= kMarsMaze200) {
		loadLoopSound1("Sounds/Mars/Mars Maze Ambient.22K.8.AIFF");
	} else if (room == kMarsRobotShuttle) {
		loadLoopSound1("Sounds/Mars/Robot Shuttle.22K.8.AIFF");
	}

	if (!_noAirFuse.isFuseLit()) {
		switch (room) {
		case kMars02:
		case kMars05:
		case kMars06:
		case kMars07:
		case kMars08:
			loadLoopSound2("Sounds/Mars/Gantry Loop.aiff", 0x100, 0, 0);
			break;
		// Robot at maze 48
		case kMarsMaze037:
			if (GameState.isCurrentDoorOpen())
				loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			else
				loadLoopSound2("");
			break;
		case kMarsMaze038:
		case kMarsMaze039:
		case kMarsMaze049:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100);
			break;
		case kMarsMaze050:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			break;
		case kMarsMaze051:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		case kMarsMaze052:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 4);
			break;
		case kMarsMaze042:
		case kMarsMaze053:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 8);
			break;
		case kMarsMaze058:
			if (GameState.isCurrentDoorOpen())
				loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 4);
			else
				loadLoopSound2("");
			break;
		// Robot at 151
		case kMarsMaze148:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100);
			break;
		case kMarsMaze147:
		case kMarsMaze149:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			break;
		case kMarsMaze146:
		case kMarsMaze152:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		case kMarsMaze145:
		case kMarsMaze153:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 4);
			break;
		// Robots at 80 and 82.
		case kMarsMaze079:
		case kMarsMaze081:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100);
			break;
		case kMarsMaze078:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			break;
		case kMarsMaze083:
			if (GameState.isCurrentDoorOpen())
				loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			else
				loadLoopSound2("");
			break;
		case kMarsMaze118:
		case kMarsMaze076:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		case kMarsMaze074:
		case kMarsMaze117:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 4);
			break;
		// Robot at 94
		case kMarsMaze093:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100);
			break;
		case kMarsMaze091:
		case kMarsMaze092:
		case kMarsMaze098:
		case kMarsMaze101:
		case kMarsMaze100:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			break;
		case kMarsMaze090:
		case kMarsMaze099:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		case kMarsMaze089:
			if (GameState.isCurrentDoorOpen())
				loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		case kMarsMaze178:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 4);
			break;
		// Robot at 197
		case kMarsMaze191:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100);
			break;
		case kMarsMaze190:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			break;
		case kMarsMaze198:
		case kMarsMaze189:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		default:
			loadLoopSound2("");
			break;
		}
	}
}

void Mars::checkContinuePoint(const RoomID room, const DirectionConstant direction) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kMars02, kSouth):
	case MakeRoomView(kMars19, kEast):
	case MakeRoomView(kMars22, kNorth):
	case MakeRoomView(kMars43, kEast):
	case MakeRoomView(kMars51, kEast):
	case MakeRoomView(kMars56, kEast):
	case MakeRoomView(kMars60, kWest):
	case MakeRoomView(kMarsMaze004, kWest):
	case MakeRoomView(kMarsMaze009, kWest):
	case MakeRoomView(kMarsMaze012, kWest):
	case MakeRoomView(kMarsMaze037, kWest):
	case MakeRoomView(kMarsMaze047, kNorth):
	case MakeRoomView(kMarsMaze052, kWest):
	case MakeRoomView(kMarsMaze057, kNorth):
	case MakeRoomView(kMarsMaze071, kWest):
	case MakeRoomView(kMarsMaze081, kNorth):
	case MakeRoomView(kMarsMaze088, kWest):
	case MakeRoomView(kMarsMaze093, kWest):
	case MakeRoomView(kMarsMaze115, kNorth):
	case MakeRoomView(kMarsMaze120, kWest):
	case MakeRoomView(kMarsMaze126, kEast):
	case MakeRoomView(kMarsMaze133, kNorth):
	case MakeRoomView(kMarsMaze144, kNorth):
	case MakeRoomView(kMarsMaze156, kEast):
	case MakeRoomView(kMarsMaze162, kNorth):
	case MakeRoomView(kMarsMaze177, kWest):
	case MakeRoomView(kMarsMaze180, kNorth):
	case MakeRoomView(kMarsMaze187, kWest):
	case MakeRoomView(kMarsMaze199, kWest):
		makeContinuePoint();
		break;
	case MakeRoomView(kMars05, kEast):
	case MakeRoomView(kMars06, kEast):
	case MakeRoomView(kMars07, kEast):
		if (GameState.getMarsSecurityDown())
			makeContinuePoint();
		break;
	case MakeRoomView(kMars46, kSouth):
		if (!GameState.getMarsSeenRobotAtReactor())
			makeContinuePoint();
		break;
	case MakeRoomView(kMars46, kWest):
		if (GameState.getMarsAvoidedReactorRobot())
			makeContinuePoint();
		break;
	}
}

void Mars::launchMaze007Robot() {
	startExtraLongSequence(kMarsMaze007RobotApproach, kMarsMaze007RobotDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze007RobotLoopingTime, kMarsMovieScale, kMaze007RobotLoopingEvent);
}

void Mars::launchMaze015Robot() {
	startExtraLongSequence(kMarsMaze015SouthRobotApproach, kMarsMaze015SouthRobotDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze015RobotLoopingTime, kMarsMovieScale, kMaze015RobotLoopingEvent);
}

void Mars::launchMaze101Robot() {
	startExtraLongSequence(kMarsMaze101EastRobotApproach, kMarsMaze101EastRobotDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze101RobotLoopingTime, kMarsMovieScale, kMaze101RobotLoopingEvent);
}

void Mars::launchMaze104Robot() {
	startExtraLongSequence(kMarsMaze104WestLoop, kMarsMaze104WestDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze104RobotLoopingTime, kMarsMovieScale, kMaze104RobotLoopingEvent);
}

void Mars::launchMaze133Robot() {
	startExtraLongSequence(kMarsMaze133SouthApproach, kMarsMaze133SouthDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze133RobotLoopingTime, kMarsMovieScale, kMaze133RobotLoopingEvent);
}

void Mars::launchMaze136Robot() {
	startExtraLongSequence(kMarsMaze136NorthApproach, kMarsMaze136NorthDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze136RobotLoopingTime, kMarsMovieScale, kMaze136RobotLoopingEvent);
}

void Mars::launchMaze184Robot() {
	startExtraLongSequence(kMarsMaze184WestLoop, kMarsMaze184WestDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze184RobotLoopingTime, kMarsMovieScale, kMaze184RobotLoopingEvent);
}

void Mars::timerExpired(const uint32 eventType) {
	switch (eventType) {
	case kMaze007RobotLoopingEvent:
	case kMaze015RobotLoopingEvent:
	case kMaze101RobotLoopingEvent:
	case kMaze104RobotLoopingEvent:
	case kMaze133RobotLoopingEvent:
	case kMaze136RobotLoopingEvent:
	case kMaze184RobotLoopingEvent:
		_interruptionFilter = kFilterNoInput;
		break;
	}
}

void Mars::arriveAt(const RoomID room, const DirectionConstant direction) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kMars18, kNorth):
		if (GameState.getMarsPodAtUpperPlatform())
			setCurrentAlternate(kAltMarsPodAtMars34);
		break;
	case MakeRoomView(kMars27, kEast):
	case MakeRoomView(kMars29, kEast):
		if (GameState.isTakenItemID(kMarsCard))
			setCurrentAlternate(kAltMarsTookCard);
		else
			setCurrentAlternate(kAltMarsNormal);
		break;
	case MakeRoomView(kMars35, kEast):
	case MakeRoomView(kMars35, kWest):
		if (GameState.getMarsAirlockOpen())
			setCurrentAlternate(kAltMars35AirlockWest);
		else
			setCurrentAlternate(kAltMars35AirlockEast);
		break;
	case MakeRoomView(kMars60, kEast):
	case MakeRoomView(kMars60, kWest):
		if (GameState.getMarsAirlockOpen())
			setCurrentAlternate(kAltMars60AirlockEast);
		else
			setCurrentAlternate(kAltMars60AirlockWest);
		break;
	case MakeRoomView(kMars45, kNorth):
	case MakeRoomView(kMars45, kSouth):
	case MakeRoomView(kMars45, kEast):
	case MakeRoomView(kMars45, kWest):
		GameState.setMarsPodAtUpperPlatform(false);
		setCurrentAlternate(kAltMarsPodAtMars45);
		break;
	case MakeRoomView(kMars46, kNorth):
	case MakeRoomView(kMars46, kSouth):
	case MakeRoomView(kMars46, kEast):
	case MakeRoomView(kMars46, kWest):
	case MakeRoomView(kMars47, kNorth):
	case MakeRoomView(kMars47, kSouth):
	case MakeRoomView(kMars47, kEast):
	case MakeRoomView(kMars47, kWest):
		if (GameState.getMarsPodAtUpperPlatform())
			setCurrentAlternate(kAltMarsNormal);
		else
			setCurrentAlternate(kAltMarsPodAtMars45);
		break;
	case MakeRoomView(kMars48, kNorth):
	case MakeRoomView(kMars48, kSouth):
	case MakeRoomView(kMars48, kEast):
	case MakeRoomView(kMars48, kWest):
	case MakeRoomView(kMars49, kNorth):
	case MakeRoomView(kMars49, kEast):
	case MakeRoomView(kMars49, kWest):
		if (GameState.isTakenItemID(kAirMask))
			setCurrentAlternate(kAltMarsTookMask);
		else
			setCurrentAlternate(kAltMarsNormal);
		break;
	case MakeRoomView(kMars49, kSouth):
		if (GameState.getMarsMaskOnFiller())
			setCurrentAlternate(kAltMarsMaskOnFiller);
		else if (GameState.isTakenItemID(kAirMask))
			setCurrentAlternate(kAltMarsTookMask);
		else
			setCurrentAlternate(kAltMarsNormal);
		break;
	}

	Neighborhood::arriveAt(room, direction);
	checkAirlockDoors();
	setUpReactorEnergyDrain();

	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kMars0A, kNorth):
		if (!GameState.getMarsSeenTimeStream())
			startExtraLongSequence(kMarsArrivalFromTSA, kMars0AWatchShuttleDepart, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kMars07, kSouth):
	case MakeRoomView(kMars13, kNorth):
		if (!GameState.getMarsHeardCheckInMessage()) {
			playSpotSoundSync(kMarsCheckInRequiredIn, kMarsCheckInRequiredOut);
			GameState.setMarsHeardCheckInMessage(true);
		}
		break;
	case MakeRoomView(kMars44, kWest):
		if (GameState.getMarsReadyForShuttleTransport())
			startUpFromFinishedSpaceChase();
		else if (GameState.getMarsFinishedCanyonChase())
			startUpFromSpaceChase();
		else
			_neighborhoodNotification.setNotificationFlags(kTimeForCanyonChaseFlag, kTimeForCanyonChaseFlag);
		break;
	case MakeRoomView(kMars10, kNorth):
		if (!GameState.getMarsRobotThrownPlayer())
			startExtraSequence(kRobotThrowsPlayer, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kMars11, kSouth):
	case MakeRoomView(kMars12, kSouth):
		setCurrentActivation(kActivationReadyForKiosk);
		break;
	case MakeRoomView(kMars15, kWest):
		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSecurityDown()) {
			playSpotSoundSync(kMarsShuttle2DepartedIn, kMarsShuttle2DepartedOut);
			restoreStriding(kMars17, kWest, kAltMarsNormal);
			GameState.setMarsSecurityDown(true);
		}
		break;
	case MakeRoomView(kMars17, kNorth):
	case MakeRoomView(kMars17, kSouth):
	case MakeRoomView(kMars17, kEast):
	case MakeRoomView(kMars17, kWest):
		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSecurityDown())
			forceStridingStop(kMars17, kWest, kAltMarsNormal);

		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSawRobotLeave()) {
			startExtraSequence(kRobotOnWayToShuttle, kExtraCompletedFlag, kFilterNoInput);
			restoreStriding(kMars19, kWest, kAltMarsNormal);
			GameState.setMarsSawRobotLeave(true);
		}
		break;
	case MakeRoomView(kMars19, kNorth):
	case MakeRoomView(kMars19, kSouth):
	case MakeRoomView(kMars19, kWest):
		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSawRobotLeave())
			forceStridingStop(kMars19, kWest, kAltMarsNormal);

		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSecurityDown())
			forceStridingStop(kMars17, kWest, kAltMarsNormal);
		break;
	case MakeRoomView(kMars19, kEast):
		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSawRobotLeave())
			forceStridingStop(kMars19, kWest, kAltMarsNormal);

		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSecurityDown())
			forceStridingStop(kMars17, kWest, kAltMarsNormal);
		break;
	case MakeRoomView(kMars32, kNorth):
		if (!GameState.getMarsPodAtUpperPlatform()) {
			playSpotSoundSync(kMarsPodArrivedUpperPlatformIn, kMarsPodArrivedUpperPlatformOut);
			GameState.setMarsPodAtUpperPlatform(true);
		}
		break;
	case MakeRoomView(kMars33North, kNorth):
		setCurrentActivation(kActivationTunnelMapReady);
		// Fall through...
	case MakeRoomView(kMars33, kSouth):
	case MakeRoomView(kMars33, kEast):
	case MakeRoomView(kMars33, kWest):
	case MakeRoomView(kMars32, kSouth):
	case MakeRoomView(kMars32, kEast):
	case MakeRoomView(kMars32, kWest):
		if (!GameState.getMarsPodAtUpperPlatform())
			GameState.setMarsPodAtUpperPlatform(true);
		break;
	case MakeRoomView(kMars34, kNorth):
		startExtraSequence(kMars34NorthPodGreeting, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kMars34, kSouth):
	case MakeRoomView(kMars45, kNorth):
		setCurrentActivation(kActivateMarsPodClosed);
		break;
	case MakeRoomView(kMars35, kWest):
		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSecurityDown())
			forceStridingStop(kMars19, kWest, kAltMarsNormal);
		// Fall through...
	case MakeRoomView(kMars60, kEast):
		if (!GameState.getMarsAirlockOpen())
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
		break;
	case MakeRoomView(kMars35, kEast):
	case MakeRoomView(kMars60, kWest):
		if (GameState.getMarsAirlockOpen())
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
		break;
	case MakeRoomView(kMars39, kWest):
		if (GameState.getLastRoom() == kMarsMaze200)
			GameState.setMarsPodAtUpperPlatform(false);
		break;
	case MakeRoomView(kMars45, kSouth):
		// Set up maze doors here.
		// Doing it here makes sure that it will be the same if the player comes
		// back out of the maze and goes back in, but will vary if
		// the player comes back down to the maze a second time.
		GameState.setMarsMazeDoorPair1(_vm->getRandomBit());
		GameState.setMarsMazeDoorPair2(_vm->getRandomBit());
		GameState.setMarsMazeDoorPair3(_vm->getRandomBit());
		GameState.setMarsArrivedBelow(true);
		break;
	case MakeRoomView(kMars48, kEast):
		if (!GameState.getMarsSeenRobotAtReactor()) {
			// Preload the looping sound...
			loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0, 0, 0);
			startExtraSequence(kMars48RobotApproaches, kExtraCompletedFlag, kFilterNoInput);
		} else if (!GameState.getMarsAvoidedReactorRobot()) {
			loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0x100, 0, 0);
			loopExtraSequence(kMars48RobotLoops);
			_utilityFuse.primeFuse(kMarsRobotPatienceLimit);
			_utilityFuse.setFunctor(new Common::Functor0Mem<void, Mars>(this, &Mars::robotTiredOfWaiting));
			_utilityFuse.lightFuse();
		}
		break;
	case MakeRoomView(kMars48, kSouth):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot()) {
			loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0x100, 0, 0);
			_utilityFuse.primeFuse(kMarsRobotPatienceLimit);
			_utilityFuse.setFunctor(new Common::Functor0Mem<void, Mars>(this, &Mars::robotTiredOfWaiting));
			_utilityFuse.lightFuse();
		}
		break;
	case MakeRoomView(kMars49, kSouth):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot()) {
			playSpotSoundSync(kMarsRobotTakesTransportIn, kMarsRobotTakesTransportOut);
			playSpotSoundSync(kMarsPodDepartedLowerPlatformIn, kMarsPodDepartedLowerPlatformOut);
			GameState.setMarsAvoidedReactorRobot(true);
			GameState.setMarsPodAtUpperPlatform(true);
			GameState.setScoringAvoidedRobot();
		}

		if (GameState.isTakenItemID(kAirMask))
			setCurrentActivation(kActivateHotSpotAlways);
		else if (GameState.getMarsMaskOnFiller())
			setCurrentActivation(kActivateMaskOnFiller);
		else
			setCurrentActivation(kActivateMaskOnHolder);
		break;
	case MakeRoomView(kMars51, kWest):
	case MakeRoomView(kMars50, kWest):
	case MakeRoomView(kMars48, kWest):
		if (GameState.getShieldOn())
			g_shield->setItemState(kShieldNormal);
		g_energyMonitor->setEnergyDrainRate(kEnergyDrainNormal);
		_vm->resetEnergyDeathReason();
		break;
	case MakeRoomView(kMars52, kNorth):
	case MakeRoomView(kMars52, kSouth):
	case MakeRoomView(kMars52, kEast):
	case MakeRoomView(kMars52, kWest):
	case MakeRoomView(kMars54, kNorth):
	case MakeRoomView(kMars54, kSouth):
	case MakeRoomView(kMars54, kEast):
	case MakeRoomView(kMars54, kWest):
	case MakeRoomView(kMars56, kNorth):
	case MakeRoomView(kMars56, kSouth):
	case MakeRoomView(kMars56, kWest):
	case MakeRoomView(kMars58, kNorth):
	case MakeRoomView(kMars58, kSouth):
	case MakeRoomView(kMars58, kEast):
	case MakeRoomView(kMars58, kWest):
		setCurrentActivation(kActivateReactorPlatformOut);
		break;
	case MakeRoomView(kMars56, kEast):
		if (GameState.getMarsLockBroken()) {
			setCurrentActivation(kActivateReactorAskOperation);
			_privateFlags.setFlag(kMarsPrivatePlatformZoomedInFlag, true);
		} else if (GameState.getMarsLockFrozen()) {
			setCurrentActivation(kActivateReactorReadyForCrowBar);
			_privateFlags.setFlag(kMarsPrivatePlatformZoomedInFlag, true);
			_utilityFuse.primeFuse(kLockFreezeTimeLmit);
			_utilityFuse.setFunctor(new Common::Functor0Mem<void, Mars>(this, &Mars::lockThawed));
			_utilityFuse.lightFuse();
		} else {
			setCurrentActivation(kActivateReactorPlatformOut);
		}
		break;
	case MakeRoomView(kMarsRobotShuttle, kEast):
		setCurrentActivation(kActivationRobotHeadClosed);
		break;
	case MakeRoomView(kMarsMaze007, kNorth):
		launchMaze007Robot();
		break;
	case MakeRoomView(kMarsMaze015, kSouth):
		launchMaze015Robot();
		break;
	case MakeRoomView(kMarsMaze101, kEast):
		launchMaze101Robot();
		break;
	case MakeRoomView(kMarsMaze104, kWest):
		launchMaze104Robot();
		break;
	case MakeRoomView(kMarsMaze133, kSouth):
		launchMaze133Robot();
		break;
	case MakeRoomView(kMarsMaze136, kNorth):
		launchMaze136Robot();
		break;
	case MakeRoomView(kMarsMaze184, kWest):
		launchMaze184Robot();
		break;
	case MakeRoomView(kMarsMaze199, kSouth):
		GameState.setScoringThreadedMaze();
		GameState.setMarsThreadedMaze(true);
		break;
	case MakeRoomView(kMarsDeathRoom, kNorth):
	case MakeRoomView(kMarsDeathRoom, kSouth):
	case MakeRoomView(kMarsDeathRoom, kEast):
	case MakeRoomView(kMarsDeathRoom, kWest):
		switch (GameState.getLastRoom()) {
		case kMars39:
			die(kDeathDidntLeaveBucket);
			break;
		case kMars46:
			die(kDeathRunOverByPod);
			break;
		}
		break;
	}

	checkAirMask();
}

void Mars::shieldOn() {
	setUpReactorEnergyDrain();
}

void Mars::shieldOff() {
	setUpReactorEnergyDrain();
}

void Mars::turnTo(const DirectionConstant direction) {
	switch (MakeRoomView(GameState.getCurrentRoom(), direction)) {
	case MakeRoomView(kMars27, kNorth):
	case MakeRoomView(kMars27, kSouth):
	case MakeRoomView(kMars27, kEast):
	case MakeRoomView(kMars29, kNorth):
	case MakeRoomView(kMars29, kSouth):
	case MakeRoomView(kMars29, kEast):
		if (GameState.isTakenItemID(kMarsCard))
			setCurrentAlternate(kAltMarsTookCard);
		break;
	case MakeRoomView(kMars35, kNorth):
	case MakeRoomView(kMars35, kSouth):
	case MakeRoomView(kMars60, kNorth):
	case MakeRoomView(kMars60, kSouth):
		if (getCurrentActivation() == kActivateAirlockPressurized)
			playSpotSoundSync(kMarsAirlockPressurizeIn, kMarsAirlockPressurizeOut);
		break;
	}

	Neighborhood::turnTo(direction);

	switch (MakeRoomView(GameState.getCurrentRoom(), direction)) {
	case MakeRoomView(kMars11, kSouth):
	case MakeRoomView(kMars12, kSouth):
		setCurrentActivation(kActivationReadyForKiosk);
		break;
	case MakeRoomView(kMars18, kNorth):
		if (GameState.getMarsPodAtUpperPlatform())
			setCurrentAlternate(kAltMarsPodAtMars34);
		break;
	case MakeRoomView(kMars22, kSouth):
		if (!GameState.getMarsHeardCheckInMessage()) {
			playSpotSoundSync(kMarsCheckInRequiredIn, kMarsCheckInRequiredOut);
			GameState.setMarsHeardCheckInMessage(true);
		}
		break;
	case MakeRoomView(kMars34, kSouth):
	case MakeRoomView(kMars45, kNorth):
		setCurrentActivation(kActivateMarsPodClosed);
		break;
	case MakeRoomView(kMars34, kNorth):
		startExtraSequence(kMars34NorthPodGreeting, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kMars35, kEast):
	case MakeRoomView(kMars60, kWest):
		if (GameState.getMarsAirlockOpen())
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
		break;
	case MakeRoomView(kMars60, kEast):
		if (!GameState.getMarsAirlockOpen())
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
		break;
	case MakeRoomView(kMars35, kWest):
		if (!GameState.getMarsAirlockOpen())
			setCurrentActivation(kActivateReadyToPressurizeAirlock);

		// Do this here because this will be called after spinning the airlock after
		// going through the gear room.
		if (GameState.getMarsThreadedMaze())
			GameState.setScoringThreadedGearRoom();
		break;
	case MakeRoomView(kMars48, kNorth):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot())
			die(kDeathDidntGetOutOfWay);
		break;
	case MakeRoomView(kMars48, kEast):
		if (!GameState.getMarsSeenRobotAtReactor()) {
			// Preload the looping sound...
			loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0, 0, 0);
			startExtraSequence(kMars48RobotApproaches, kExtraCompletedFlag, kFilterNoInput);
		} else if (!GameState.getMarsAvoidedReactorRobot()) {
			loopExtraSequence(kMars48RobotLoops);
		} else if (GameState.isTakenItemID(kAirMask)) {
			setCurrentAlternate(kAltMarsTookMask);
		} else {
			setCurrentAlternate(kAltMarsNormal);
		}
		break;
	case MakeRoomView(kMars48, kWest):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot())
			die(kDeathDidntGetOutOfWay);
		else if (GameState.isTakenItemID(kAirMask))
			setCurrentAlternate(kAltMarsTookMask);
		else
			setCurrentAlternate(kAltMarsNormal);
		break;
	case MakeRoomView(kMars49, kSouth):
		if (GameState.isTakenItemID(kAirMask))
			setCurrentActivation(kActivateHotSpotAlways);
		else
			setCurrentActivation(kActivateMaskOnHolder);
		break;
	case MakeRoomView(kMars52, kNorth):
	case MakeRoomView(kMars52, kSouth):
	case MakeRoomView(kMars52, kEast):
	case MakeRoomView(kMars52, kWest):
	case MakeRoomView(kMars54, kNorth):
	case MakeRoomView(kMars54, kSouth):
	case MakeRoomView(kMars54, kEast):
	case MakeRoomView(kMars54, kWest):
	case MakeRoomView(kMars56, kNorth):
	case MakeRoomView(kMars56, kSouth):
	case MakeRoomView(kMars56, kEast):
	case MakeRoomView(kMars56, kWest):
	case MakeRoomView(kMars58, kNorth):
	case MakeRoomView(kMars58, kSouth):
	case MakeRoomView(kMars58, kEast):
	case MakeRoomView(kMars58, kWest):
		setCurrentActivation(kActivateReactorPlatformOut);
		break;
	case MakeRoomView(kMarsMaze007, kNorth):
		launchMaze007Robot();
		break;
	case MakeRoomView(kMarsMaze015, kSouth):
		launchMaze015Robot();
		break;
	case MakeRoomView(kMarsMaze101, kEast):
		launchMaze101Robot();
		break;
	case MakeRoomView(kMarsMaze104, kWest):
		launchMaze104Robot();
		break;
	case MakeRoomView(kMarsMaze133, kSouth):
		launchMaze133Robot();
		break;
	case MakeRoomView(kMarsMaze136, kNorth):
		launchMaze136Robot();
		break;
	case MakeRoomView(kMarsMaze184, kWest):
		launchMaze184Robot();
		break;
	}
}

void Mars::activateOneHotspot(HotspotInfoTable::Entry &entry, Hotspot *hotspot) {
	switch (hotspot->getObjectID()) {
	case kMars57RedMoveSpotID:
	case kMars57YellowMoveSpotID:
	case kMars57GreenMoveSpotID:
		if (!_choiceHighlight.choiceHighlighted(hotspot->getObjectID() - kMars57RedMoveSpotID))
			hotspot->setActive();
		break;
	case kMars57BlueMoveSpotID:
		if (_reactorStage >= 2 && !_choiceHighlight.choiceHighlighted(3))
			hotspot->setActive();
		break;
	case kMars57PurpleMoveSpotID:
		if (_reactorStage == 3 && !_choiceHighlight.choiceHighlighted(4))
			hotspot->setActive();
		break;
	default:
		Neighborhood::activateOneHotspot(entry, hotspot);
		break;
	}
}

void Mars::activateHotspots() {
	InventoryItem *item;

	Neighborhood::activateHotspots();

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars48, kEast):
		if ((_navMovie.getFlags() & kLoopTimeBase) != 0 && _vm->getDragType() == kDragInventoryUse)
			_vm->getAllHotspots().activateOneHotspot(kAttackRobotHotSpotID);
		break;
	case MakeRoomView(kMars56, kEast):
		switch (getCurrentActivation()) {
		case kActivateReactorReadyForNitrogen:
			item = (InventoryItem *)_vm->getAllItems().findItemByID(kNitrogenCanister);
			if (item->getItemState() != kNitrogenFull)
				_vm->getAllHotspots().deactivateOneHotspot(kMars57DropNitrogenSpotID);
			// Fall through...
		case kActivateReactorReadyForCrowBar:
			_vm->getAllHotspots().activateOneHotspot(kMars57CantOpenPanelSpotID);
			break;
		}
		break;
	case MakeRoomView(kMarsRobotShuttle, kEast):
		if (_privateFlags.getFlag(kMarsPrivateGotMapChipFlag))
			_vm->getAllHotspots().deactivateOneHotspot(kRobotShuttleMapChipSpotID);
		else
			_vm->getAllHotspots().activateOneHotspot(kRobotShuttleMapChipSpotID);

		if (_privateFlags.getFlag(kMarsPrivateGotOpticalChipFlag))
			_vm->getAllHotspots().deactivateOneHotspot(kRobotShuttleOpticalChipSpotID);
		else
			_vm->getAllHotspots().activateOneHotspot(kRobotShuttleOpticalChipSpotID);

		if (_privateFlags.getFlag(kMarsPrivateGotShieldChipFlag))
			_vm->getAllHotspots().deactivateOneHotspot(kRobotShuttleShieldChipSpotID);
		else
			_vm->getAllHotspots().activateOneHotspot(kRobotShuttleShieldChipSpotID);
		break;
	default:
		if (_privateFlags.getFlag(kMarsPrivateInSpaceChaseFlag)) {
			if (GameState.getMarsReadyForShuttleTransport()) {
				_shuttleTransportSpot.setActive();
			} else {
				_energyChoiceSpot.setActive();
				_gravitonChoiceSpot.setActive();
				_tractorChoiceSpot.setActive();
				if (_weaponSelection != kNoWeapon)
					_shuttleViewSpot.setActive();
			}
		}
		break;
	}
}

void Mars::clickInHotspot(const Input &input, const Hotspot *clickedSpot) {
	switch (clickedSpot->getObjectID()) {
	case kMars11NorthKioskSpotID:
	case kMars12NorthKioskSpotID:
		playSpotSoundSync(kMarsKioskBeepIn, kMarsKioskBeepOut);
		Neighborhood::clickInHotspot(input, clickedSpot);
		break;
	case kMars11NorthKioskSightsSpotID:
	case kMars12NorthKioskSightsSpotID:
		playSpotSoundSync(kMarsKioskBeepIn, kMarsKioskBeepOut);
		if (!startExtraSequenceSync(kMarsSightsInfo, kFilterAllInput))
			showExtraView(kMarsInfoKioskIntro);
		break;
	case kMars11NorthKioskColonySpotID:
	case kMars12NorthKioskColonySpotID:
		playSpotSoundSync(kMarsKioskBeepIn, kMarsKioskBeepOut);
		if (!startExtraSequenceSync(kMarsColonyInfo, kFilterAllInput))
			showExtraView(kMarsInfoKioskIntro);
		break;
	case kMars33NorthMonitorSpotID:
		switch (_lastExtra) {
		case kMars33SlideShow1:
			startExtraSequence(kMars33SlideShow2, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMars33SlideShow2:
			startExtraSequence(kMars33SlideShow3, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMars33SlideShow3:
			startExtraSequence(kMars33SlideShow4, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMars33SlideShow4:
			// Should never happen...
		default:
			startExtraSequence(kMars33SlideShow1, kExtraCompletedFlag, kFilterNoInput);
			break;
		}
		break;
	case kMars34SouthOpenStorageSpotID:
		if (GameState.isTakenItemID(kCrowbar))
			startExtraSequence(kMars34SpotOpenNoBar, kExtraCompletedFlag, kFilterNoInput);
		else
			startExtraSequence(kMars34SpotOpenWithBar, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kMars34SouthCloseStorageSpotID:
		if (GameState.isTakenItemID(kCrowbar))
			startExtraSequence(kMars34SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
		else
			startExtraSequence(kMars34SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kMars35WestPressurizeSpotID:
	case kMars35EastPressurizeSpotID:
	case kMars60WestPressurizeSpotID:
	case kMars60EastPressurizeSpotID:
		playSpotSoundSync(kMarsAirlockButtonBeepIn, kMarsAirlockButtonBeepOut);
		playSpotSoundSync(kMarsAirlockPressurizeIn, kMarsAirlockPressurizeOut);
		setCurrentActivation(kActivateAirlockPressurized);
		break;
	case kMars45NorthOpenStorageSpotID:
		if (GameState.isTakenItemID(kCrowbar))
			startExtraSequence(kMars45SpotOpenNoBar, kExtraCompletedFlag, kFilterNoInput);
		else
			startExtraSequence(kMars45SpotOpenWithBar, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kMars45NorthCloseStorageSpotID:
		if (GameState.isTakenItemID(kCrowbar))
			startExtraSequence(kMars45SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
		else
			startExtraSequence(kMars45SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kMars56ExtractSpotID:
		if (GameState.isTakenItemID(kCardBomb)) {
			startExtraSequence(kMars56ExtendNoBomb, kExtraCompletedFlag, kFilterNoInput);
			setCurrentActivation(kActivateReactorPlatformIn);
		} else {
			startExtraSequence(kMars56ExtendWithBomb, kExtraCompletedFlag, kFilterNoInput);
			setCurrentActivation(kActivateReactorAskLowerScreen);
		}
		break;
	case kMars57UndoMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doUndoOneGuess();
		break;
	case kMars57RedMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doReactorGuess(0);
		break;
	case kMars57YellowMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doReactorGuess(1);
		break;
	case kMars57GreenMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doReactorGuess(2);
		break;
	case kMars57BlueMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doReactorGuess(3);
		break;
	case kMars57PurpleMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doReactorGuess(4);
		break;
	case kShuttleEnergySpotID:
	case kShuttleGravitonSpotID:
	case kShuttleTractorSpotID:
	case kShuttleViewSpotID:
	case kShuttleTransportSpotID:
		spaceChaseClick(input, clickedSpot->getObjectID());
		break;
	default:
		Neighborhood::clickInHotspot(input, clickedSpot);
		break;
	}
}

InputBits Mars::getInputFilter() {
	InputBits result = Neighborhood::getInputFilter();

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars49, kSouth):
		if (GameState.getMarsMaskOnFiller())
			// Can't move when mask is on filler.
			result &= ~kFilterAllDirections;
		break;
	case MakeRoomView(kMars52, kNorth):
	case MakeRoomView(kMars52, kSouth):
	case MakeRoomView(kMars52, kEast):
	case MakeRoomView(kMars52, kWest):
	case MakeRoomView(kMars54, kNorth):
	case MakeRoomView(kMars54, kSouth):
	case MakeRoomView(kMars54, kEast):
	case MakeRoomView(kMars54, kWest):
	case MakeRoomView(kMars56, kNorth):
	case MakeRoomView(kMars56, kSouth):
	case MakeRoomView(kMars56, kEast):
	case MakeRoomView(kMars56, kWest):
	case MakeRoomView(kMars58, kNorth):
	case MakeRoomView(kMars58, kSouth):
	case MakeRoomView(kMars58, kEast):
	case MakeRoomView(kMars58, kWest):
		if (_privateFlags.getFlag(kMarsPrivatePlatformZoomedInFlag))
			// Can't move when platform is extended.
			result &= ~kFilterAllDirections;
		break;
	case MakeRoomView(kMars44, kWest):
		if (_canyonChaseMovie.isMovieValid() && _canyonChaseMovie.isRunning())
			result &= ~kFilterAllDirections;
		break;
	}

	return result;
}

// Only called when trying to pick up an item and the player can't (because
// the inventory is too full or because the player lets go of the item before
// dropping it into the inventory).
Hotspot *Mars::getItemScreenSpot(Item *item, DisplayElement *element) {
	HotSpotID destSpotID;

	switch (item->getObjectID()) {
	case kCardBomb:
		destSpotID = kMars57GrabBombSpotID;
		break;
	case kMarsCard:
		destSpotID = kMars31SouthCardSpotID;
		break;
	case kAirMask:
		if (GameState.getMarsMaskOnFiller())
			destSpotID = kMars49AirFillingDropSpotID;
		else
			destSpotID = kMars49AirMaskSpotID;
		break;
	case kCrowbar:
		if (GameState.getCurrentRoom() == kMars34)
			destSpotID = kMars34SouthCrowbarSpotID;
		else
			destSpotID = kMars45NorthCrowbarSpotID;
		break;
	case kMapBiochip:
		destSpotID = kRobotShuttleMapChipSpotID;
		break;
	case kOpticalBiochip:
		destSpotID = kRobotShuttleOpticalChipSpotID;
		break;
	case kShieldBiochip:
		destSpotID = kRobotShuttleShieldChipSpotID;
		break;
	default:
		destSpotID = kNoHotSpotID;
		break;
	}

	if (destSpotID == kNoHotSpotID)
		return Neighborhood::getItemScreenSpot(item, element);

	return _vm->getAllHotspots().findHotspotByID(destSpotID);
}

void Mars::takeItemFromRoom(Item *item) {
	switch (item->getObjectID()) {
	case kAirMask:
		setCurrentAlternate(kAltMarsTookMask);
		break;
	case kCardBomb:
		_privateFlags.setFlag(kMarsPrivateDraggingBombFlag, true);
		break;
	case kMapBiochip:
		_privateFlags.setFlag(kMarsPrivateGotMapChipFlag, true);
		break;
	case kShieldBiochip:
		_privateFlags.setFlag(kMarsPrivateGotShieldChipFlag, true);
		break;
	case kOpticalBiochip:
		_privateFlags.setFlag(kMarsPrivateGotOpticalChipFlag, true);
		break;
	}

	Neighborhood::takeItemFromRoom(item);
}

void Mars::pickedUpItem(Item *item) {
	switch (item->getObjectID()) {
	case kAirMask:
		setCurrentActivation(kActivateHotSpotAlways);
		if (!GameState.getScoringGotOxygenMask()) {
			g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Mars/XM48SB", false, kWarningInterruption);
			GameState.setScoringGotOxygenMask();
		}
		break;
	case kCrowbar:
		GameState.setScoringGotCrowBar();
		g_AIArea->checkMiddleArea();
		break;
	case kMarsCard:
		GameState.setScoringGotMarsCard();
		g_AIArea->checkMiddleArea();
		break;
	case kCardBomb:
		GameState.setScoringGotCardBomb();
		if (GameState.getMarsLockBroken()) {
			startExtraSequence(kMars57BackToNormal, kExtraCompletedFlag, kFilterNoInput);
			GameState.setMarsLockBroken(false);
		}

		_privateFlags.setFlag(kMarsPrivateDraggingBombFlag, false);
		break;
	case kMapBiochip:
		if (_privateFlags.getFlag(kMarsPrivateGotMapChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotShieldChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotOpticalChipFlag)) {
			GameState.setMarsFinished(true);
			GameState.setScoringMarsGandhi();
			startExtraSequence(kMarsRobotHeadClose, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kShieldBiochip:
		if (_privateFlags.getFlag(kMarsPrivateGotMapChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotShieldChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotOpticalChipFlag)) {
			GameState.setMarsFinished(true);
			GameState.setScoringMarsGandhi();
			startExtraSequence(kMarsRobotHeadClose, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kOpticalBiochip:
		g_opticalChip->addAries();
		GameState.setScoringGotMarsOpMemChip();

		if (_privateFlags.getFlag(kMarsPrivateGotMapChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotShieldChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotOpticalChipFlag)) {
			GameState.setMarsFinished(true);
			GameState.setScoringMarsGandhi();
			startExtraSequence(kMarsRobotHeadClose, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	}
}

void Mars::dropItemIntoRoom(Item *item, Hotspot *dropSpot) {
	if (dropSpot->getObjectID() == kAttackRobotHotSpotID) {
		_attackingItem = (InventoryItem *)item;
		startExtraSequence(kMars48RobotDefends, kExtraCompletedFlag, kFilterNoInput);
		loadLoopSound2("");
	} else {
		switch (item->getObjectID()) {
		case kMarsCard:
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			if (dropSpot && dropSpot->getObjectID() == kMars34NorthCardDropSpotID)
				startExtraSequence(kMarsTurnOnPod, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kNitrogenCanister:
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			if (dropSpot && dropSpot->getObjectID() == kMars57DropNitrogenSpotID)
				startExtraSequence(kMars57FreezeLock, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kCrowbar:
			_utilityFuse.stopFuse();
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			if (dropSpot && dropSpot->getObjectID() == kMars57DropCrowBarSpotID)
				startExtraSequence(kMars57BreakLock, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kAirMask:
			if (dropSpot) {
				if (dropSpot->getObjectID() == kMars49AirFillingDropSpotID) {
					if (!GameState.getMarsMaskOnFiller()) {
						Neighborhood::dropItemIntoRoom(item, dropSpot);
						startExtraSequence(kMars49SouthViewMaskFilling, kExtraCompletedFlag, kFilterNoInput);
					} else {
						setCurrentActivation(kActivateMaskOnFiller);
						setCurrentAlternate(kAltMarsMaskOnFiller);
						Neighborhood::dropItemIntoRoom(item, dropSpot);
					}
				} else if (dropSpot->getObjectID() == kMars49AirMaskSpotID) {
					setCurrentAlternate(kAltMarsNormal);
					setCurrentActivation(kActivateMaskOnHolder);
					Neighborhood::dropItemIntoRoom(item, dropSpot);
				}
			}
			break;
		case kCardBomb:
			_privateFlags.setFlag(kMarsPrivateDraggingBombFlag, false);
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			break;
		case kMapBiochip:
			_privateFlags.setFlag(kMarsPrivateGotMapChipFlag, false);
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			break;
		case kShieldBiochip:
			_privateFlags.setFlag(kMarsPrivateGotShieldChipFlag, false);
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			break;
		case kOpticalBiochip:
			_privateFlags.setFlag(kMarsPrivateGotOpticalChipFlag, false);
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			break;
		default:
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			break;
		}
	}
}

void Mars::robotTiredOfWaiting() {
	if (GameState.getCurrentRoomAndView() == MakeRoomView(kMars48, kEast)) {
		if (_attackingItem) {
			startExtraSequence(kMars48RobotKillsPlayer, kExtraCompletedFlag, kFilterNoInput);
			loadLoopSound2("");
		} else {
			_privateFlags.setFlag(kMarsPrivateRobotTiredOfWaitingFlag, true);
		}
	} else {
		die(kDeathDidntGetOutOfWay);
	}
}

void Mars::turnLeft() {
	if (isEventTimerRunning())
		cancelEvent();

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars34, kSouth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			_privateFlags.setFlag(kMarsPrivatePodTurnLeftFlag, true);
			if (GameState.isTakenItemID(kCrowbar))
				startExtraSequence(kMars34SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kMars34SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		} else {
			Neighborhood::turnLeft();
		}
		break;
	case MakeRoomView(kMars45, kNorth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			_privateFlags.setFlag(kMarsPrivatePodTurnLeftFlag, true);
			if (GameState.isTakenItemID(kCrowbar))
				startExtraSequence(kMars45SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kMars45SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		} else {
			Neighborhood::turnLeft();
		}
		break;
	default:
		Neighborhood::turnLeft();
		break;
	}
}

void Mars::turnRight() {
	if (isEventTimerRunning())
		cancelEvent();

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars34, kSouth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			_privateFlags.setFlag(kMarsPrivatePodTurnRightFlag, true);
			if (GameState.isTakenItemID(kCrowbar))
				startExtraSequence(kMars34SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kMars34SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		} else {
			Neighborhood::turnRight();
		}
		break;
	case MakeRoomView(kMars45, kNorth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			_privateFlags.setFlag(kMarsPrivatePodTurnRightFlag, true);
			if (GameState.isTakenItemID(kCrowbar))
				startExtraSequence(kMars45SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kMars45SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		} else {
			Neighborhood::turnRight();
		}
		break;
	default:
		Neighborhood::turnRight();
		break;
	}
}

void Mars::receiveNotification(Notification *notification, const NotificationFlags flag) {
	InventoryItem *item;

	Neighborhood::receiveNotification(notification, flag);

	if ((flag & kExtraCompletedFlag) != 0) {
		_interruptionFilter = kFilterAllInput;

		switch (_lastExtra) {
		case kMarsArrivalFromTSA:
			GameState.setMarsSeenTimeStream(true);
			loadAmbientLoops();
			playSpotSoundSync(kMarsShuttle1DepartedIn, kMarsShuttle1DepartedOut);
			makeContinuePoint();
			break;
		case kRobotThrowsPlayer:
			GameState.setMarsRobotThrownPlayer(true);
			GameState.setScoringThrownByRobot();
			restoreStriding(kMars08, kNorth, kAltMarsNormal);
			arriveAt(kMars08, kNorth);
			if (!GameState.getMarsHeardUpperPodMessage()) {
				playSpotSoundSync(kMarsPodDepartedUpperPlatformIn,
											kMarsPodDepartedUpperPlatformOut);
				GameState.setMarsHeardUpperPodMessage(true);
			}
			break;
		case kMarsInfoKioskIntro:
			GameState.setScoringSawMarsKiosk();
			setCurrentActivation(kActivationKioskChoice);
			break;
		case kMars33SlideShow4:
			GameState.setScoringSawTransportMap();
			setCurrentActivation(kActivateHotSpotAlways);
			break;
		case kMars34SpotOpenNoBar:
		case kMars34SpotOpenWithBar:
		case kMars45SpotOpenNoBar:
		case kMars45SpotOpenWithBar:
			_privateFlags.setFlag(kMarsPrivatePodStorageOpenFlag, true);
			setCurrentActivation(kActivateMarsPodOpen);
			break;
		case kMars34SpotCloseNoBar:
		case kMars34SpotCloseWithBar:
		case kMars45SpotCloseNoBar:
		case kMars45SpotCloseWithBar:
			_privateFlags.setFlag(kMarsPrivatePodStorageOpenFlag, false);
			setCurrentActivation(kActivateMarsPodClosed);
			if (_privateFlags.getFlag(kMarsPrivatePodTurnLeftFlag)) {
				_privateFlags.setFlag(kMarsPrivatePodTurnLeftFlag, false);
				turnLeft();
			} else if (_privateFlags.getFlag(kMarsPrivatePodTurnRightFlag)) {
				_privateFlags.setFlag(kMarsPrivatePodTurnRightFlag, false);
				turnRight();
			}
			break;
		case kMarsTurnOnPod:
			item = (InventoryItem *)_vm->getAllItems().findItemByID(kMarsCard);
			_vm->addItemToInventory(item);
			GameState.setScoringTurnedOnTransport();
			loadLoopSound1("");
			loadLoopSound2("");
			startExtraSequence(kMarsTakePodToMars45, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMarsTakePodToMars45:
			arriveAt(kMars45, kSouth);
			break;
		case kMars35WestSpinAirlockToEast:
			GameState.setMarsAirlockOpen(false);
			setCurrentAlternate(kAltMars35AirlockEast);
			turnTo(kWest);
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
			g_airMask->airQualityChanged();
			checkAirMask();
			loadAmbientLoops();
			break;
		case kMars35EastSpinAirlockToWest:
			GameState.setMarsAirlockOpen(true);
			setCurrentAlternate(kAltMars35AirlockWest);
			turnTo(kEast);
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
			g_airMask->airQualityChanged();
			checkAirMask();
			loadAmbientLoops();
			break;
		case kMars48RobotApproaches:
			loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0x100, 0, 0);
			GameState.setMarsSeenRobotAtReactor(true);
			loopExtraSequence(kMars48RobotLoops);
			_utilityFuse.primeFuse(kMarsRobotPatienceLimit);
			_utilityFuse.setFunctor(new Common::Functor0Mem<void, Mars>(this, &Mars::robotTiredOfWaiting));
			_utilityFuse.lightFuse();
			break;
		case kMars48RobotDefends:
			_vm->addItemToInventory(_attackingItem);
			_attackingItem = 0;
			if (_privateFlags.getFlag(kMarsPrivateRobotTiredOfWaitingFlag)) {
				startExtraSequence(kMars48RobotKillsPlayer, kExtraCompletedFlag, kFilterNoInput);
				loadLoopSound2("", 0x100, 0, 0);
			} else {
				loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0x100, 0, 0);
				loopExtraSequence(kMars48RobotLoops, kExtraCompletedFlag);
			}
			break;
		case kMars48RobotKillsPlayer:
			loadLoopSound2("");
			die(kDeathDidntGetOutOfWay);
			break;
		case kMars49SouthViewMaskFilling:
			setCurrentActivation(kActivateMaskOnFiller);
			setCurrentAlternate(kAltMarsMaskOnFiller);
			GameState.setMarsMaskOnFiller(true);
			break;
		case kMars58SpinLeft:
		case kMars54SpinRight:
			GameState.setScoringActivatedPlatform();
			arriveAt(kMars52, kEast);
			break;
		case kMars52SpinLeft:
		case kMars56SpinRight:
			GameState.setScoringActivatedPlatform();
			arriveAt(kMars54, kEast);
			break;
		case kMars54SpinLeft:
		case kMars58SpinRight:
			GameState.setScoringActivatedPlatform();
			arriveAt(kMars56, kEast);
			break;
		case kMars56SpinLeft:
		case kMars52SpinRight:
			GameState.setScoringActivatedPlatform();
			arriveAt(kMars58, kEast);
			break;
		case kMars52Extend:
		case kMars54Extend:
		case kMars56ExtendNoBomb:
		case kMars58Extend:
			GameState.setScoringActivatedPlatform();
			setCurrentActivation(kActivateReactorPlatformIn);
			_privateFlags.setFlag(kMarsPrivatePlatformZoomedInFlag, true);
			break;
		case kMars53Retract:
		case kMars55Retract:
		case kMars57RetractWithBomb:
		case kMars57RetractNoBomb:
		case kMars59Retract:
			GameState.setScoringActivatedPlatform();
			setCurrentActivation(kActivateReactorPlatformOut);
			_privateFlags.setFlag(kMarsPrivatePlatformZoomedInFlag, false);
			break;
		case kMars56ExtendWithBomb:
			playSpotSoundSync(kMustBeUnlockedIn, kMustBeUnlockedOut);
			GameState.setScoringActivatedPlatform();
			_privateFlags.setFlag(kMarsPrivatePlatformZoomedInFlag, true);
			break;
		case kMars57CantOpenPanel:
			GameState.setScoringActivatedPlatform();
			setCurrentActivation(kActivateReactorAskLowerScreen);
			break;
		case kMars57LowerScreenClosed:
		case kMars57ThawLock:
			setCurrentActivation(kActivateReactorReadyForNitrogen);
			GameState.setMarsLockFrozen(false);
			break;
		case kMars57FreezeLock:
			item = (InventoryItem *)_vm->getAllItems().findItemByID(kNitrogenCanister);
			item->setItemState(kNitrogenEmpty);
			_vm->addItemToInventory(item);
			setCurrentActivation(kActivateReactorReadyForCrowBar);
			GameState.setScoringUsedLiquidNitrogen();
			GameState.setMarsLockFrozen(true);
			showExtraView(kMars57LockFrozenView);
			_utilityFuse.primeFuse(kLockFreezeTimeLmit);
			_utilityFuse.setFunctor(new Common::Functor0Mem<void, Mars>(this, &Mars::lockThawed));
			_utilityFuse.lightFuse();
			break;
		case kMars57BreakLock:
			item = (InventoryItem *)_vm->getAllItems().findItemByID(kCrowbar);
			_vm->addItemToInventory(item);
			GameState.setScoringUsedCrowBar();
			GameState.setMarsLockBroken(true);
			GameState.setMarsLockFrozen(false);
			startExtraLongSequence(kMars57OpenPanel, kMars57OpenPanelChoices, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMars57OpenPanel:
		case kMars57OpenPanelChoices:
			setCurrentActivation(kActivateReactorAskOperation);
			break;
		case kMars57ShieldEvaluation:
		case kMars57MeasureOutput:
			setCurrentActivation(kActivateReactorRanEvaluation);
			loopExtraSequence(kMars57ShieldOkayLoop);
			break;
		case kMars57RunDiagnostics:
			setCurrentActivation(kActivateReactorRanDiagnostics);
			GameState.setScoringFoundCardBomb();
			break;
		case kMars57BombExplodes:
		case kMars57BombExplodesInGame:
			die(kDeathDidntDisarmMarsBomb);
			break;
		case kMars57BombAnalysis:
			setCurrentActivation(kActivateReactorAnalyzed);
			break;
		case kMars57DontLink:
			startExtraSequence(kMars57OpenPanelChoices, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMars57CircuitLink:
			setCurrentActivation(kActivateReactorInstructions);
			break;
		case kMars57GameLevel1:
			setUpReactorLevel1();
			break;
		case kMars57GameLevel2:
		case kMars57GameLevel3:
			setUpNextReactorLevel();
			break;
		case kMars57GameSolved:
			setCurrentActivation(kActivateReactorBombSafe);
			break;
		case kMars57ExposeBomb:
			setCurrentActivation(kActivateReactorBombExposed);
			_privateFlags.setFlag(kMarsPrivateBombExposedFlag, true);
			break;
		case kMars57BackToNormal:
			setCurrentActivation(kActivateReactorPlatformIn);
			_privateFlags.setFlag(kMarsPrivateBombExposedFlag, false);
			g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Mars/XM51SW", false, kWarningInterruption);
			break;
		case kMars60WestSpinAirlockToEast:
			GameState.setMarsAirlockOpen(true);
			setCurrentAlternate(kAltMars60AirlockEast);
			turnTo(kWest);
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
			g_airMask->airQualityChanged();
			checkAirMask();
			loadAmbientLoops();
			break;
		case kMars60EastSpinAirlockToWest:
			GameState.setMarsAirlockOpen(false);
			setCurrentAlternate(kAltMars60AirlockWest);
			turnTo(kEast);
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
			g_airMask->airQualityChanged();
			checkAirMask();
			loadAmbientLoops();
			break;
		case kMarsRobotHeadOpen:
			setCurrentActivation(kActivationRobotHeadOpen);
			break;
		case kMarsRobotHeadClose:
			recallToTSASuccess();
			break;
		case kMarsMaze007RobotApproach:
		case kMarsMaze015SouthRobotApproach:
		case kMarsMaze101EastRobotApproach:
		case kMarsMaze104WestLoop:
		case kMarsMaze133SouthApproach:
		case kMarsMaze136NorthApproach:
		case kMarsMaze184WestLoop:
			die(kDeathGroundByMazebot);
			break;
		}
	} else if ((flag & kTimeForCanyonChaseFlag) != 0) {
		doCanyonChase();
	} else if ((flag & kExplosionFinishedFlag) != 0) {
		_explosions.stop();
		_explosions.hide();
		if (g_robotShip->isDead()) {
			GameState.setMarsFinished(true);
			_centerShuttleMovie.hide();
			_upperRightShuttleMovie.show();
			_upperRightShuttleMovie.setTime(kShuttleUpperRightTargetDestroyedTime);
			_upperRightShuttleMovie.redrawMovieWorld();
			_rightDamageShuttleMovie.hide();
			playMovieSegment(&_rightShuttleMovie, kShuttleRightDestroyedStart, kShuttleRightDestroyedStop);
			playSpotSoundSync(kShuttleDestroyedIn, kShuttleDestroyedOut);
			throwAwayMarsShuttle();
			reinstateMonocleInterface();
			recallToTSASuccess();
		}
	} else if ((flag & kTimeToTransportFlag) != 0) {
		transportToRobotShip();
	}

	if (g_AIArea)
		g_AIArea->checkMiddleArea();
}

void Mars::spotCompleted() {
	Neighborhood::spotCompleted();

	if (GameState.getCurrentRoom() == kMarsRobotShuttle)
		g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Mars/XN59WD", false, kWarningInterruption);
}

void Mars::doCanyonChase() {
	GameState.setScoringEnteredShuttle();
	setNextHandler(_vm);
	throwAwayInterface();

	_vm->_cursor->hide();

	// Open the spot sounds movie again...
	_spotSounds.initFromQuickTime(getSoundSpotsName());
	_spotSounds.setVolume(_vm->getSoundFXLevel());

	Video::VideoDecoder *video = new Video::QuickTimeDecoder();
	if (!video->loadFile("Images/Mars/M44ESA.movie"))
		error("Could not load interface->shuttle transition video");

	video->start();

	while (!_vm->shouldQuit() && !video->endOfVideo()) {
		if (video->needsUpdate()) {
			const Graphics::Surface *frame = video->decodeNextFrame();

			if (frame)
				_vm->drawScaledFrame(frame, 0, 0);
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event))
			;

		g_system->delayMillis(10);
	}

	delete video;

	if (_vm->shouldQuit())
		return;

	initOnePicture(&_shuttleInterface1, "Images/Mars/MCmain1.pict", kShuttleBackgroundOrder, kShuttle1Left,
							kShuttle1Top, true);
	initOnePicture(&_shuttleInterface2, "Images/Mars/MCmain2.pict", kShuttleBackgroundOrder, kShuttle2Left,
							kShuttle2Top, true);
	initOnePicture(&_shuttleInterface3, "Images/Mars/MCmain3.pict", kShuttleBackgroundOrder, kShuttle3Left,
							kShuttle3Top, true);
	initOnePicture(&_shuttleInterface4, "Images/Mars/MCmain4.pict", kShuttleBackgroundOrder, kShuttle4Left,
							kShuttle4Top, true);

	initOneMovie(&_canyonChaseMovie, "Images/Mars/Canyon.movie",
						kShuttleMonitorOrder, kShuttleWindowLeft, kShuttleWindowTop, true);
	_canyonChaseMovie.setVolume(_vm->getSoundFXLevel());

	loadLoopSound1("Sounds/Mars/Inside Cockpit.22K.8.AIFF");

	// Swing shuttle around...
	playMovieSegment(&_canyonChaseMovie, kShuttleSwingStart, kShuttleSwingStop);

	initOneMovie(&_leftShuttleMovie, "Images/Mars/Left Shuttle.movie",
			kShuttleMonitorOrder, kShuttleLeftLeft, kShuttleLeftTop, false);

	initOneMovie(&_rightShuttleMovie, "Images/Mars/Right Shuttle.movie",
			kShuttleMonitorOrder, kShuttleRightLeft, kShuttleRightTop, false);

	initOneMovie(&_lowerLeftShuttleMovie, "Images/Mars/Lower Left Shuttle.movie", kShuttleMonitorOrder,
			kShuttleLowerLeftLeft, kShuttleLowerLeftTop, false);

	initOneMovie(&_lowerRightShuttleMovie, "Images/Mars/Lower Right Shuttle.movie", kShuttleMonitorOrder,
			kShuttleLowerRightLeft, kShuttleLowerRightTop, false);

	initOneMovie(&_centerShuttleMovie, "Images/Mars/Center Shuttle.movie",
			kShuttleMonitorOrder, kShuttleCenterLeft, kShuttleCenterTop, false);

	initOneMovie(&_upperLeftShuttleMovie, "Images/Mars/Upper Left Shuttle.movie", kShuttleMonitorOrder,
			kShuttleUpperLeftLeft, kShuttleUpperLeftTop, false);

	initOneMovie(&_upperRightShuttleMovie, "Images/Mars/Upper Right Shuttle.movie", kShuttleMonitorOrder,
			kShuttleUpperRightLeft, kShuttleUpperRightTop, false);

	initOneMovie(&_leftDamageShuttleMovie, "Images/Mars/Left Damage Shuttle.movie",
			kShuttleStatusOrder, kShuttleLeftEnergyLeft, kShuttleLeftEnergyTop, false);

	initOneMovie(&_rightDamageShuttleMovie, "Images/Mars/Right Damage Shuttle.movie",
			kShuttleStatusOrder, kShuttleRightEnergyLeft, kShuttleRightEnergyTop, false);

	_centerShuttleMovie.show();
	_centerShuttleMovie.setTime(kShuttleCenterBoardingTime);
	playSpotSoundSync(kShuttleCockpitIn, kShuttleCockpitOut);

	_centerShuttleMovie.setTime(kShuttleCenterCheckTime);
	playSpotSoundSync(kShuttleOnboardIn, kShuttleOnboardOut);

	_shuttleEnergyMeter.initShuttleEnergyMeter();
	_shuttleEnergyMeter.powerUpMeter();
	while (_shuttleEnergyMeter.isFading()) {
		_vm->checkCallBacks();
		_vm->refreshDisplay();
		g_system->updateScreen();
	}

	_leftShuttleMovie.show();
	playMovieSegment(&_leftShuttleMovie, kShuttleLeftIntroStart, kShuttleLeftIntroStop);

	_leftShuttleMovie.setTime(kShuttleLeftNormalTime);
	_leftShuttleMovie.redrawMovieWorld();

	_leftDamageShuttleMovie.show();
	playMovieSegment(&_leftDamageShuttleMovie);

	// Take it down a tick initially. This sets the time to the time of the last tick,
	// so that subsequence drops will drop it down a tick.
	_leftDamageShuttleMovie.setTime(_leftDamageShuttleMovie.getTime() - 40);
	_leftDamageShuttleMovie.redrawMovieWorld();

	_lowerRightShuttleMovie.show();
	_lowerRightShuttleMovie.setTime(kShuttleLowerRightOffTime);
	_lowerRightShuttleMovie.redrawMovieWorld();
	_centerShuttleMovie.setTime(kShuttleCenterNavCompTime);
	_centerShuttleMovie.redrawMovieWorld();
	playSpotSoundSync(kShuttleNavigationIn, kShuttleNavigationOut);

	_centerShuttleMovie.setTime(kShuttleCenterCommTime);
	_centerShuttleMovie.redrawMovieWorld();
	playSpotSoundSync(kShuttleCommunicationIn, kShuttleCommunicationOut);

	_centerShuttleMovie.setTime(kShuttleCenterAllSystemsTime);
	_centerShuttleMovie.redrawMovieWorld();
	playSpotSoundSync(kShuttleAllSystemsIn, kShuttleAllSystemsOut);

	_centerShuttleMovie.setTime(kShuttleCenterSecureLooseTime);
	_centerShuttleMovie.redrawMovieWorld();
	playSpotSoundSync(kShuttleSecureLooseIn, kShuttleSecureLooseOut);

	_centerShuttleMovie.setTime(kShuttleCenterAutoTestTime);
	_centerShuttleMovie.redrawMovieWorld();
	playSpotSoundSync(kShuttleAutoTestingIn, kShuttleAutoTestingOut);

	_leftShuttleMovie.setTime(kShuttleLeftAutoTestTime);
	_leftShuttleMovie.redrawMovieWorld();
	playSpotSoundSync(kMarsThrusterAutoTestIn, kMarsThrusterAutoTestOut);
	_leftShuttleMovie.setTime(kShuttleLeftNormalTime);
	_leftShuttleMovie.redrawMovieWorld();

	_centerShuttleMovie.setTime(kShuttleCenterLaunchTime);
	_centerShuttleMovie.redrawMovieWorld();
	playSpotSoundSync(kShuttlePrepareForDropIn, kShuttlePrepareForDropOut);

	playSpotSoundSync(kShuttleAllClearIn, kShuttleAllClearOut);

	_centerShuttleMovie.setTime(kShuttleCenterEnterTubeTime);
	_centerShuttleMovie.redrawMovieWorld();

	_lowerLeftShuttleMovie.show();
	_lowerLeftShuttleMovie.setTime(kShuttleLowerLeftCollisionTime);

	loadLoopSound1("");

	_canyonChaseMovie.setSegment(kCanyonChaseStart, kCanyonChaseStop);
	_canyonChaseMovie.start();

	startMarsTimer(kLaunchTubeReachedTime, kMovieTicksPerSecond, kMarsLaunchTubeReached);
}

void Mars::startUpFromFinishedSpaceChase() {
	setNextHandler(_vm);
	throwAwayInterface();

	initOnePicture(&_shuttleInterface1, "Images/Mars/MCmain1.pict", kShuttleBackgroundOrder, kShuttle1Left,
							kShuttle1Top, true);
	initOnePicture(&_shuttleInterface2, "Images/Mars/MCmain2.pict", kShuttleBackgroundOrder, kShuttle2Left,
							kShuttle2Top, true);
	initOnePicture(&_shuttleInterface3, "Images/Mars/MCmain3.pict", kShuttleBackgroundOrder, kShuttle3Left,
							kShuttle3Top, true);
	initOnePicture(&_shuttleInterface4, "Images/Mars/MCmain4.pict", kShuttleBackgroundOrder, kShuttle4Left,
							kShuttle4Top, true);

	initOneMovie(&_leftShuttleMovie, "Images/Mars/Left Shuttle.movie",
			kShuttleMonitorOrder, kShuttleLeftLeft, kShuttleLeftTop, false);

	initOneMovie(&_rightShuttleMovie, "Images/Mars/Right Shuttle.movie",
			kShuttleMonitorOrder, kShuttleRightLeft, kShuttleRightTop, false);

	initOneMovie(&_lowerLeftShuttleMovie, "Images/Mars/Lower Left Shuttle.movie", kShuttleMonitorOrder,
			kShuttleLowerLeftLeft, kShuttleLowerLeftTop, false);

	initOneMovie(&_lowerRightShuttleMovie, "Images/Mars/Lower Right Shuttle.movie", kShuttleMonitorOrder,
			kShuttleLowerRightLeft, kShuttleLowerRightTop, false);

	initOneMovie(&_centerShuttleMovie, "Images/Mars/Center Shuttle.movie",
			kShuttleMonitorOrder, kShuttleCenterLeft, kShuttleCenterTop, false);

	initOneMovie(&_upperLeftShuttleMovie, "Images/Mars/Upper Left Shuttle.movie", kShuttleMonitorOrder,
			kShuttleUpperLeftLeft, kShuttleUpperLeftTop, false);

	initOneMovie(&_upperRightShuttleMovie, "Images/Mars/Upper Right Shuttle.movie", kShuttleMonitorOrder,
			kShuttleUpperRightLeft, kShuttleUpperRightTop, false);

	initOneMovie(&_leftDamageShuttleMovie, "Images/Mars/Left Damage Shuttle.movie",
			kShuttleStatusOrder, kShuttleLeftEnergyLeft, kShuttleLeftEnergyTop, false);

	initOneMovie(&_rightDamageShuttleMovie, "Images/Mars/Right Damage Shuttle.movie",
			kShuttleStatusOrder, kShuttleRightEnergyLeft, kShuttleRightEnergyTop, false);

	_centerShuttleMovie.show();

	_shuttleEnergyMeter.initShuttleEnergyMeter();
	_shuttleEnergyMeter.setEnergyValue(kFullShuttleEnergy);

	_leftShuttleMovie.show();
	_leftShuttleMovie.setTime(kShuttleLeftNormalTime);
	_leftShuttleMovie.redrawMovieWorld();

	_leftDamageShuttleMovie.show();
	_leftDamageShuttleMovie.setTime(_leftDamageShuttleMovie.getDuration() - 40);
	_leftDamageShuttleMovie.redrawMovieWorld();

	_lowerRightShuttleMovie.show();

	_lowerLeftShuttleMovie.show();

	loadLoopSound1("Sounds/Mars/Space Ambient.22K.8.AIFF");

	initOneMovie(&_junk, "Images/Mars/Junk.movie", kShuttleJunkOrder, kShuttleJunkLeft,
			kShuttleJunkTop, false);

	initOneMovie(&_explosions, "Images/Mars/Explosions.movie", kShuttleWeaponFrontOrder, 0, 0, false);
	_explosionCallBack.initCallBack(&_explosions, kCallBackAtExtremes);

	_energyBeam.initShuttleWeapon();
	_gravitonCannon.initShuttleWeapon();

	_upperLeftShuttleMovie.show();
	_upperLeftShuttleMovie.setTime(kShuttleUpperLeftDimTime);
	_upperLeftShuttleMovie.redrawMovieWorld();

	_rightShuttleMovie.show();
	_rightShuttleMovie.setTime(kShuttleRightIntroStop - 1);
	_rightShuttleMovie.redrawMovieWorld();

	_rightDamageShuttleMovie.show();
	_rightDamageShuttleMovie.setTime(40);
	_rightDamageShuttleMovie.redrawMovieWorld();

	_lowerLeftShuttleMovie.setTime(kShuttleLowerLeftAutopilotTime);
	_lowerLeftShuttleMovie.redrawMovieWorld();

	_shuttleTransportSpot.setArea(kShuttleTransportBounds);
	_shuttleTransportSpot.setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
	_vm->getAllHotspots().push_back(&_shuttleTransportSpot);

	_privateFlags.setFlag(kMarsPrivateInSpaceChaseFlag, true);

	_upperRightShuttleMovie.show();
	_upperRightShuttleMovie.setTime(kShuttleUpperRightOverloadTime);
	_upperRightShuttleMovie.redrawMovieWorld();

	_centerShuttleMovie.setTime(kShuttleCenterSafeTime);
	_centerShuttleMovie.redrawMovieWorld();

	_lowerRightShuttleMovie.setTime(kShuttleLowerRightTransportTime);
	_lowerRightShuttleMovie.redrawMovieWorld();

	initOneMovie(&_canyonChaseMovie, "Images/Mars/M98EAS.movie", kShuttleTractorBeamMovieOrder,
			kShuttleWindowLeft, kShuttleWindowTop, true);
	_canyonChaseMovie.setTime(_canyonChaseMovie.getDuration());
	_canyonChaseMovie.redrawMovieWorld();
}

void Mars::startUpFromSpaceChase() {
	setNextHandler(_vm);
	throwAwayInterface();

	// Open the spot sounds movie again...
	_spotSounds.initFromQuickTime(getSoundSpotsName());
	_spotSounds.setVolume(_vm->getSoundFXLevel());;

	initOnePicture(&_shuttleInterface1, "Images/Mars/MCmain1.pict", kShuttleBackgroundOrder, kShuttle1Left,
							kShuttle1Top, true);
	initOnePicture(&_shuttleInterface2, "Images/Mars/MCmain2.pict", kShuttleBackgroundOrder, kShuttle2Left,
							kShuttle2Top, true);
	initOnePicture(&_shuttleInterface3, "Images/Mars/MCmain3.pict", kShuttleBackgroundOrder, kShuttle3Left,
							kShuttle3Top, true);
	initOnePicture(&_shuttleInterface4, "Images/Mars/MCmain4.pict", kShuttleBackgroundOrder, kShuttle4Left,
							kShuttle4Top, true);

	initOneMovie(&_leftShuttleMovie, "Images/Mars/Left Shuttle.movie",
			kShuttleMonitorOrder, kShuttleLeftLeft, kShuttleLeftTop, false);

	initOneMovie(&_rightShuttleMovie, "Images/Mars/Right Shuttle.movie",
			kShuttleMonitorOrder, kShuttleRightLeft, kShuttleRightTop, false);

	initOneMovie(&_lowerLeftShuttleMovie, "Images/Mars/Lower Left Shuttle.movie", kShuttleMonitorOrder,
			kShuttleLowerLeftLeft, kShuttleLowerLeftTop, false);

	initOneMovie(&_lowerRightShuttleMovie, "Images/Mars/Lower Right Shuttle.movie", kShuttleMonitorOrder,
			kShuttleLowerRightLeft, kShuttleLowerRightTop, false);

	initOneMovie(&_centerShuttleMovie, "Images/Mars/Center Shuttle.movie",
			kShuttleMonitorOrder, kShuttleCenterLeft, kShuttleCenterTop, false);

	initOneMovie(&_upperLeftShuttleMovie, "Images/Mars/Upper Left Shuttle.movie", kShuttleMonitorOrder,
			kShuttleUpperLeftLeft, kShuttleUpperLeftTop, false);

	initOneMovie(&_upperRightShuttleMovie, "Images/Mars/Upper Right Shuttle.movie", kShuttleMonitorOrder,
			kShuttleUpperRightLeft, kShuttleUpperRightTop, false);

	initOneMovie(&_leftDamageShuttleMovie, "Images/Mars/Left Damage Shuttle.movie",
			kShuttleStatusOrder, kShuttleLeftEnergyLeft, kShuttleLeftEnergyTop, false);

	initOneMovie(&_rightDamageShuttleMovie, "Images/Mars/Right Damage Shuttle.movie",
			kShuttleStatusOrder, kShuttleRightEnergyLeft, kShuttleRightEnergyTop, false);

	_centerShuttleMovie.show();

	_shuttleEnergyMeter.initShuttleEnergyMeter();
	_shuttleEnergyMeter.setEnergyValue(kFullShuttleEnergy);

	_leftShuttleMovie.show();
	_leftShuttleMovie.setTime(kShuttleLeftNormalTime);
	_leftShuttleMovie.redrawMovieWorld();

	_leftDamageShuttleMovie.show();
	_leftDamageShuttleMovie.setTime(_leftDamageShuttleMovie.getDuration() - 40);
	_leftDamageShuttleMovie.redrawMovieWorld();

	_lowerRightShuttleMovie.show();

	_lowerLeftShuttleMovie.show();

	loadLoopSound1("Sounds/Mars/Space Ambient.22K.8.AIFF");

	initOneMovie(&_planetMovie, "Images/Mars/Planet.movie", kShuttlePlanetOrder,
			kPlanetStartLeft, kPlanetStartTop, true);
	_planetMovie.setFlags(kLoopTimeBase);

	initOneMovie(&_junk, "Images/Mars/Junk.movie", kShuttleJunkOrder, kShuttleJunkLeft,
			kShuttleJunkTop, false);

	initOneMovie(&_explosions, "Images/Mars/Explosions.movie", kShuttleWeaponFrontOrder, 0, 0, false);
	_explosionCallBack.initCallBack(&_explosions, kCallBackAtExtremes);

	_energyBeam.initShuttleWeapon();
	_gravitonCannon.initShuttleWeapon();

	_upperLeftShuttleMovie.show();

	_robotShip.initRobotShip();

	_planetMovie.start();
	_planetMover.startMoving(&_planetMovie);

	_upperLeftShuttleMovie.setTime(kShuttleUpperLeftDimTime);
	_upperLeftShuttleMovie.redrawMovieWorld();

	_centerShuttleMovie.setTime(kShuttleCenterTargetSightedTime);
	_centerShuttleMovie.redrawMovieWorld();

	_lowerRightShuttleMovie.setTime(kShuttleLowerRightTrackingTime);
	_lowerRightShuttleMovie.redrawMovieWorld();

	_rightShuttleMovie.show();
	_rightShuttleMovie.setTime(kShuttleRightIntroStop - 1);
	_rightShuttleMovie.redrawMovieWorld();

	_rightDamageShuttleMovie.show();
	_rightDamageShuttleMovie.setTime(_rightDamageShuttleMovie.getDuration() - 40);
	_rightDamageShuttleMovie.redrawMovieWorld();

	_lowerLeftShuttleMovie.setTime(kShuttleLowerLeftAutopilotTime);
	_lowerLeftShuttleMovie.redrawMovieWorld();

	_robotShip.startMoving();

	_shuttleHUD.initShuttleHUD();

	_tractorBeam.startDisplaying();

	_energyChoiceSpot.setArea(kShuttleEnergyBeamBounds);
	_energyChoiceSpot.setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
	_vm->getAllHotspots().push_back(&_energyChoiceSpot);
	_gravitonChoiceSpot.setArea(kShuttleGravitonBounds);
	_gravitonChoiceSpot.setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
	_vm->getAllHotspots().push_back(&_gravitonChoiceSpot);
	_tractorChoiceSpot.setArea(kShuttleTractorBounds);
	_tractorChoiceSpot.setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
	_vm->getAllHotspots().push_back(&_tractorChoiceSpot);
	_shuttleViewSpot.setArea(kShuttleWindowLeft, kShuttleWindowTop,
			kShuttleWindowLeft + kShuttleWindowWidth, kShuttleWindowTop + kShuttleWindowHeight);
	_shuttleViewSpot.setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
	_vm->getAllHotspots().push_back(&_shuttleViewSpot);
	_shuttleTransportSpot.setArea(kShuttleTransportBounds);
	_shuttleTransportSpot.setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
	_vm->getAllHotspots().push_back(&_shuttleTransportSpot);

	_privateFlags.setFlag(kMarsPrivateInSpaceChaseFlag, true);

	startMarsTimer(kSpaceChaseTimeLimit, kOneTickPerSecond, kMarsSpaceChaseFinished);
}

void Mars::setSoundFXLevel(const uint16 level) {
	Neighborhood::setSoundFXLevel(level);

	if (_canyonChaseMovie.isMovieValid())
		_canyonChaseMovie.setVolume(level);

	if (_explosions.isMovieValid())
		_explosions.setVolume(level);
}

void Mars::startMarsTimer(TimeValue time, TimeScale scale, MarsTimerCode code) {
	_utilityFuse.primeFuse(time, scale);
	_marsEvent.mars = this;
	_marsEvent.event = code;
	_utilityFuse.setFunctor(new Common::Functor0Mem<void, MarsTimerEvent>(&_marsEvent, &MarsTimerEvent::fire));
	_utilityFuse.lightFuse();
}

void Mars::marsTimerExpired(MarsTimerEvent &event) {
	Common::Rect r;
	uint16 x, y;

	switch (event.event) {
	case kMarsLaunchTubeReached:
		_lowerLeftShuttleMovie.setTime(kShuttleLowerLeftTubeTime);
		_lowerLeftShuttleMovie.redrawMovieWorld();
		startMarsTimer(kCanyonChaseFinishedTime, kMovieTicksPerSecond, kMarsCanyonChaseFinished);
		break;
	case kMarsCanyonChaseFinished:
		GameState.setScoringEnteredLaunchTube();

		while (_canyonChaseMovie.isRunning()) {
			_vm->checkCallBacks();
			_vm->refreshDisplay();
			_vm->_system->delayMillis(10);
		}

		_canyonChaseMovie.stop();
		_canyonChaseMovie.stopDisplaying();
		_canyonChaseMovie.releaseMovie();

		_vm->_gfx->enableErase();

		loadLoopSound1("Sounds/Mars/Space Ambient.22K.8.AIFF");

		playSpotSoundSync(kShuttleConfiguringIn, kShuttleConfiguringOut);
		playSpotSoundSync(kShuttleGeneratingIn, kShuttleGeneratingOut);
		playSpotSoundSync(kShuttleBreakawayIn, kShuttleBreakawayOut);
		playSpotSoundSync(kMarsAtmosphericBreakawayIn, kMarsAtmosphericBreakawayOut);

		initOneMovie(&_planetMovie, "Images/Mars/Planet.movie", kShuttlePlanetOrder, kPlanetStartLeft, kPlanetStartTop, true);
		_planetMovie.setFlags(kLoopTimeBase);

		initOneMovie(&_junk, "Images/Mars/Junk.movie", kShuttleJunkOrder, kShuttleJunkLeft, kShuttleJunkTop, false);

		initOneMovie(&_explosions, "Images/Mars/Explosions.movie", kShuttleWeaponFrontOrder, 0, 0, false);
		_explosionCallBack.initCallBack(&_explosions, kCallBackAtExtremes);

		_energyBeam.initShuttleWeapon();
		_gravitonCannon.initShuttleWeapon();

		_centerShuttleMovie.setTime(kShuttleCenterWeaponsTime);
		_centerShuttleMovie.redrawMovieWorld();

		_upperLeftShuttleMovie.show();
		_upperLeftShuttleMovie.setTime(kShuttleUpperLeftDampingTime);
		_upperLeftShuttleMovie.redrawMovieWorld();

		_robotShip.initRobotShip();

		_planetMovie.start();
		_planetMover.startMoving(&_planetMovie);

		playSpotSoundSync(kShuttleDamperDescIn, kShuttleDamperDescOut);
		_upperLeftShuttleMovie.setTime(kShuttleUpperLeftGravitonTime);
		_upperLeftShuttleMovie.redrawMovieWorld();

		playSpotSoundSync(kShuttleGravitonDescIn, kShuttleGravitonDescOut);
		_upperLeftShuttleMovie.setTime(kShuttleUpperLeftTractorTime);
		_upperLeftShuttleMovie.redrawMovieWorld();

		playSpotSoundSync(kShuttleTractorDescIn, kShuttleTractorDescOut);
		_upperLeftShuttleMovie.setTime(kShuttleUpperLeftDimTime);
		_upperLeftShuttleMovie.redrawMovieWorld();

		_centerShuttleMovie.setTime(kShuttleCenterTargetSightedTime);
		_centerShuttleMovie.redrawMovieWorld();
		playSpotSoundSync(kShuttleTargetSightedIn, kShuttleTargetSightedOut);

		_lowerRightShuttleMovie.setTime(kShuttleLowerRightTrackingTime);
		_lowerRightShuttleMovie.redrawMovieWorld();
		_rightShuttleMovie.show();
		playMovieSegment(&_rightShuttleMovie, kShuttleRightIntroStart, kShuttleRightIntroStop);

		_rightDamageShuttleMovie.show();
		playMovieSegment(&_rightDamageShuttleMovie);

		// Take it down a tick initially. This sets the time to the time of the last tick,
		// so that subsequence drops will drop it down a tick.
		_rightDamageShuttleMovie.setTime(_rightDamageShuttleMovie.getTime() - 40);
		_rightDamageShuttleMovie.redrawMovieWorld();

		_lowerLeftShuttleMovie.setTime(kShuttleLowerLeftAutopilotTime);
		_lowerLeftShuttleMovie.redrawMovieWorld();
		playSpotSoundSync(kShuttleAutopilotEngagedIn, kShuttleAutopilotEngagedOut);

		_robotShip.startMoving();

		_shuttleHUD.initShuttleHUD();

		_tractorBeam.startDisplaying();

		_energyChoiceSpot.setArea(kShuttleEnergyBeamBounds);
		_energyChoiceSpot.setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
		_vm->getAllHotspots().push_back(&_energyChoiceSpot);
		_gravitonChoiceSpot.setArea(kShuttleGravitonBounds);
		_gravitonChoiceSpot.setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
		_vm->getAllHotspots().push_back(&_gravitonChoiceSpot);
		_tractorChoiceSpot.setArea(kShuttleTractorBounds);
		_tractorChoiceSpot.setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
		_vm->getAllHotspots().push_back(&_tractorChoiceSpot);
		_shuttleViewSpot.setArea(kShuttleWindowLeft, kShuttleWindowTop,
				kShuttleWindowLeft + kShuttleWindowWidth, kShuttleWindowTop + kShuttleWindowHeight);
		_shuttleViewSpot.setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
		_vm->getAllHotspots().push_back(&_shuttleViewSpot);
		_shuttleTransportSpot.setArea(kShuttleTransportBounds);
		_shuttleTransportSpot.setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
		_vm->getAllHotspots().push_back(&_shuttleTransportSpot);

		_privateFlags.setFlag(kMarsPrivateInSpaceChaseFlag, true);

		playSpotSoundSync(kMarsCockpitChatterIn, kMarsCockpitChatterOut);

		GameState.setMarsFinishedCanyonChase(true);

		startMarsTimer(kSpaceChaseTimeLimit, kOneTickPerSecond, kMarsSpaceChaseFinished);

		_vm->_cursor->hideUntilMoved();
		break;
	case kMarsSpaceChaseFinished:
		// Player failed to stop the robot in time...
		_interruptionFilter = kFilterNoInput;

		_rightShuttleMovie.setTime(kShuttleRightTargetLockTime);
		_rightShuttleMovie.redrawMovieWorld();

		_upperRightShuttleMovie.show();
		_upperRightShuttleMovie.setTime(kShuttleUpperRightLockedTime);
		_upperRightShuttleMovie.redrawMovieWorld();

		_rightShuttleMovie.setTime(kShuttleRightGravitonTime);
		_rightShuttleMovie.redrawMovieWorld();
		_upperRightShuttleMovie.setTime(kShuttleUpperRightArmedTime);
		_upperRightShuttleMovie.redrawMovieWorld();

		_vm->delayShell(3, 1);

		x = _vm->getRandomNumber(19);
		y = _vm->getRandomNumber(19);

		r = Common::Rect(kShuttleWindowMidH - x, kShuttleWindowMidV - y,
				kShuttleWindowMidH - x + 20, kShuttleWindowMidV - y + 20);
		showBigExplosion(r, kShuttleAlienShipOrder);

		while (_explosions.isRunning()) {
			_vm->checkCallBacks();
			_vm->refreshDisplay();
			g_system->delayMillis(10);
		}

		throwAwayMarsShuttle();
		reinstateMonocleInterface();
		recallToTSAFailure();
		break;
	default:
		break;
	}

	_interruptionFilter = kFilterAllInput;
}

void Mars::throwAwayMarsShuttle() {
	_shuttleInterface1.deallocateSurface();
	_shuttleInterface1.stopDisplaying();
	_shuttleInterface2.deallocateSurface();
	_shuttleInterface2.stopDisplaying();
	_shuttleInterface3.deallocateSurface();
	_shuttleInterface3.stopDisplaying();
	_shuttleInterface4.deallocateSurface();
	_shuttleInterface4.stopDisplaying();

	_spotSounds.disposeSound();

	_canyonChaseMovie.releaseMovie();
	_canyonChaseMovie.stopDisplaying();
	_leftShuttleMovie.releaseMovie();
	_leftShuttleMovie.stopDisplaying();
	_rightShuttleMovie.releaseMovie();
	_rightShuttleMovie.stopDisplaying();
	_lowerLeftShuttleMovie.releaseMovie();
	_lowerLeftShuttleMovie.stopDisplaying();
	_lowerRightShuttleMovie.releaseMovie();
	_lowerRightShuttleMovie.stopDisplaying();
	_centerShuttleMovie.releaseMovie();
	_centerShuttleMovie.stopDisplaying();
	_upperLeftShuttleMovie.releaseMovie();
	_upperLeftShuttleMovie.stopDisplaying();
	_upperRightShuttleMovie.releaseMovie();
	_upperRightShuttleMovie.stopDisplaying();
	_leftDamageShuttleMovie.releaseMovie();
	_leftDamageShuttleMovie.stopDisplaying();
	_rightDamageShuttleMovie.releaseMovie();
	_rightDamageShuttleMovie.stopDisplaying();

	_shuttleEnergyMeter.disposeShuttleEnergyMeter();
	_robotShip.cleanUpRobotShip();
	_shuttleHUD.cleanUpShuttleHUD();
	_tractorBeam.stopDisplaying();
	_junk.releaseMovie();
	_junk.stopDisplaying();
	_energyBeam.cleanUpShuttleWeapon();
	_gravitonCannon.cleanUpShuttleWeapon();
	_vm->getAllHotspots().remove(&_energyChoiceSpot);
	_vm->getAllHotspots().remove(&_gravitonChoiceSpot);
	_vm->getAllHotspots().remove(&_tractorChoiceSpot);
	_vm->getAllHotspots().remove(&_shuttleViewSpot);
	_vm->getAllHotspots().remove(&_shuttleTransportSpot);
	_explosions.releaseMovie();
	_explosions.stopDisplaying();

	loadLoopSound1("");
}

void Mars::transportToRobotShip() {
	throwAwayMarsShuttle();

	Video::VideoDecoder *video = new Video::QuickTimeDecoder();
	if (!video->loadFile("Images/Mars/M98EAE.movie"))
		error("Could not load shuttle->interface transition video");

	video->start();

	while (!_vm->shouldQuit() && !video->endOfVideo()) {
		if (video->needsUpdate()) {
			const Graphics::Surface *frame = video->decodeNextFrame();

			if (frame)
				_vm->drawScaledFrame(frame, 0, 0);
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event))
			;

		g_system->delayMillis(10);
	}

	delete video;

	if (_vm->shouldQuit())
		return;

	reinstateMonocleInterface();

	g_energyMonitor->stopEnergyDraining();
	g_energyMonitor->restoreLastEnergyValue();
	_vm->resetEnergyDeathReason();
	g_energyMonitor->startEnergyDraining();

	arriveAt(kMarsRobotShuttle, kEast);

	_navMovie.stop();
	_navMovie.setTime(_navMovie.getStart());
	_navMovie.start();
}

const int kRobotTooStrong = 1;
const int kTractorTooWeak = 2;
const int kCapturedRobotShip = 3;

void Mars::spaceChaseClick(const Input &input, const HotSpotID id) {
	Common::Point pt;

	switch (id) {
	case kShuttleEnergySpotID:
		_upperLeftShuttleMovie.setTime(kShuttleUpperLeftDampingTime);
		_upperLeftShuttleMovie.redrawMovieWorld();
		_leftShuttleMovie.setTime(kShuttleLeftDampingTime);
		_leftShuttleMovie.redrawMovieWorld();
		_shuttleHUD.hide();
		_weaponSelection = kEnergyBeam;
		playSpotSoundSync(kShuttleDampingBeamIn, kShuttleDampingBeamOut);
		break;
	case kShuttleGravitonSpotID:
		_upperLeftShuttleMovie.setTime(kShuttleUpperLeftGravitonTime);
		_upperLeftShuttleMovie.redrawMovieWorld();
		_leftShuttleMovie.setTime(kShuttleLeftGravitonTime);
		_leftShuttleMovie.redrawMovieWorld();
		_shuttleHUD.hide();
		_weaponSelection = kGravitonCannon;
		playSpotSoundSync(kShuttleGravitonIn, kShuttleGravitonOut);
		break;
	case kShuttleTractorSpotID:
		_upperLeftShuttleMovie.setTime(kShuttleUpperLeftTractorTime);
		_upperLeftShuttleMovie.redrawMovieWorld();
		_leftShuttleMovie.setTime(kShuttleLeftTractorTime);
		_leftShuttleMovie.redrawMovieWorld();
		_shuttleHUD.show();
		_weaponSelection = kTractorBeam;
		playSpotSoundSync(kShuttleTractorBeamIn, kShuttleTractorBeamOut);
		break;
	case kShuttleViewSpotID:
		switch (_weaponSelection) {
		case kEnergyBeam:
			if (_shuttleEnergyMeter.getEnergyValue() < kMinDampingEnergy) {
				playSpotSoundSync(kShuttleEnergyTooLowIn, kShuttleEnergyTooLowOut);
			} else {
				if (_energyBeam.canFireWeapon()) {
					_shuttleEnergyMeter.dropEnergyValue(kMinDampingEnergy);
					input.getInputLocation(pt);
					_energyBeam.fireWeapon(pt.x, pt.y);
					playSpotSoundSync(kMarsEDBBlastIn, kMarsEDBBlastOut);
				}
			}
			break;
		case kGravitonCannon:
			if (_shuttleEnergyMeter.getEnergyValue() < kMinGravitonEnergy) {
				playSpotSoundSync(kShuttleEnergyTooLowIn, kShuttleEnergyTooLowOut);
			} else {
				if (_gravitonCannon.canFireWeapon()) {
					_shuttleEnergyMeter.dropEnergyValue(kMinGravitonEnergy);
					input.getInputLocation(pt);
					_gravitonCannon.fireWeapon(pt.x, pt.y);
					playSpotSoundSync(kMarsGravitonBlastIn, kMarsGravitonBlastOut);
				}
			}
			break;
		case kTractorBeam:
			if (_shuttleHUD.isTargetLocked()) {
				// play tractor beam sound?
				_utilityFuse.stopFuse();

				_tractorBeam.show();

				int capture;
				if (_rightDamageShuttleMovie.getTime() > 40) {
					capture = kRobotTooStrong;
				} else if (!_shuttleEnergyMeter.enoughEnergyForTractorBeam()) {
					capture = kTractorTooWeak;
				} else {
					_robotShip.snareByTractorBeam();
					capture = kCapturedRobotShip;
					_planetMover.dropPlanetOutOfSight();
				}

				_shuttleEnergyMeter.drainForTractorBeam();

				while (_shuttleEnergyMeter.isFading()) {
					_vm->checkCallBacks();
					_vm->refreshDisplay();
					_vm->_system->delayMillis(10);
				}

				_shuttleEnergyMeter.setEnergyValue(_shuttleEnergyMeter.getEnergyValue());

				switch (capture) {
				case kRobotTooStrong:
					_tractorBeam.hide();
					playSpotSoundSync(kShuttleBrokeFreeIn, kShuttleBrokeFreeOut);
					_utilityFuse.lightFuse();
					break;
				case kTractorTooWeak:
					playSpotSoundSync(kShuttleCantHoldIn, kShuttleCantHoldOut);
					_tractorBeam.hide();
					_utilityFuse.lightFuse();
					break;
				case kCapturedRobotShip:
					_tractorBeam.hide();
					_shuttleHUD.hide();
					_robotShip.cleanUpRobotShip();
					_planetMovie.stop();
					_planetMovie.stopDisplaying();
					_planetMovie.releaseMovie();

					// Shameless reuse of a variable :P
					initOneMovie(&_canyonChaseMovie, "Images/Mars/M98EAS.movie", kShuttleTractorBeamMovieOrder,
							kShuttleWindowLeft, kShuttleWindowTop, true);
					_canyonChaseMovie.redrawMovieWorld();
					playMovieSegment(&_canyonChaseMovie, 0, _canyonChaseMovie.getDuration());

					// wait here until any junk clears...
					while (_junk.junkFlying()) {
						_vm->checkCallBacks();
						_vm->refreshDisplay();
						_vm->_system->delayMillis(10);
					}

					_upperRightShuttleMovie.show();
					_upperRightShuttleMovie.setTime(kShuttleUpperRightOverloadTime);
					_upperRightShuttleMovie.redrawMovieWorld();

					playSpotSoundSync(kShuttleOverloadedIn, kShuttleOverloadedOut);
					_centerShuttleMovie.setTime(kShuttleCenterVerifyingTime);
					_centerShuttleMovie.redrawMovieWorld();

					playSpotSoundSync(kShuttleCoordinatesIn, kShuttleCoordinatesOut);
					_centerShuttleMovie.setTime(kShuttleCenterScanningTime);
					_centerShuttleMovie.redrawMovieWorld();

					playSpotSoundSync(kShuttleScanningIn, kShuttleScanningOut);
					_centerShuttleMovie.setTime(kShuttleCenterSafeTime);
					_centerShuttleMovie.redrawMovieWorld();

					playSpotSoundSync(kShuttleSafeIn, kShuttleSafeOut);
					_lowerRightShuttleMovie.setTime(kShuttleLowerRightTransportTime);
					_lowerRightShuttleMovie.redrawMovieWorld();
					GameState.setMarsReadyForShuttleTransport(true);
					break;
				}
			} else {
				playSpotSoundSync(kShuttleTractorLimitedIn, kShuttleTractorLimitedOut);
			}
			break;
		default:
			break;
		}
		break;
	case kShuttleTransportSpotID:
		_lowerRightShuttleMovie.setTime(kShuttleLowerRightTransportHiliteTime);
		_lowerRightShuttleMovie.redrawMovieWorld();
		_neighborhoodNotification.setNotificationFlags(kTimeToTransportFlag, kTimeToTransportFlag);
		break;
	}
}

void Mars::showBigExplosion(const Common::Rect &r, const DisplayOrder order) {
	if (_explosions.isMovieValid()) {
		_explosions.setDisplayOrder(order);

		Common::Rect r2 = r;
		int dx = r.width() / 2;
		int dy = r.height() / 2;
		r2.left -= dx;
		r2.right += dx;
		r2.top -= dy;
		r2.bottom += dy;

		_explosions.setBounds(r2);
		_explosions.show();
		_explosions.stop();
		_explosions.setSegment(kBigExplosionStart, kBigExplosionStop);
		_explosions.setTime(kBigExplosionStart);
		_explosionCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
		_explosions.start();
	}
}

void Mars::showLittleExplosion(const Common::Rect &r, const DisplayOrder order) {
	if (_explosions.isMovieValid()) {
		_explosions.setDisplayOrder(order);

		Common::Rect r2 = r;
		int dx = r.width() / 2;
		int dy = r.height() / 2;
		r2.left -= dx;
		r2.right += dx;
		r2.top -= dy;
		r2.bottom += dy;
		_explosions.setBounds(r2);

		_explosions.show();
		_explosions.stop();
		_explosions.setSegment(kLittleExplosionStart, kLittleExplosionStop);
		_explosions.setTime(kLittleExplosionStart);
		_explosionCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
		_explosions.start();
	}
}

void Mars::hitByJunk() {
	_leftDamageShuttleMovie.setTime(_leftDamageShuttleMovie.getTime() - 40);
	_leftDamageShuttleMovie.redrawMovieWorld();

	playSpotSoundSync(kMarsJunkCollisionIn, kMarsJunkCollisionOut);

	if (_leftDamageShuttleMovie.getTime() == 0) {
		die(kDeathRanIntoSpaceJunk);
	} else {
		TimeValue t = _leftDamageShuttleMovie.getTime() / 40;

		if (t == 1)
			playSpotSoundSync(kShuttleHullBreachIn, kShuttleHullBreachOut);

		t = _leftShuttleMovie.getTime();
		_leftShuttleMovie.setTime(kShuttleLeftDamagedTime);
		_leftShuttleMovie.redrawMovieWorld();
		_vm->delayShell(1, 3);
		_leftShuttleMovie.setTime(t);
		_leftShuttleMovie.redrawMovieWorld();
	}
}

void Mars::setUpNextDropTime() {
	_robotShip.setUpNextDropTime();
}

void Mars::decreaseRobotShuttleEnergy(const int delta, Common::Point impactPoint) {
	_rightDamageShuttleMovie.setTime(_rightDamageShuttleMovie.getTime() - 40 * delta);
	_rightDamageShuttleMovie.redrawMovieWorld();

	if (_rightDamageShuttleMovie.getTime() == 0) {
		Common::Rect r;
		_robotShip.getShuttleBounds(r);
		int size = MAX(r.width(), r.height());
		r = Common::Rect::center(impactPoint.x, impactPoint.y, size, size);
		_robotShip.killRobotShip();
		showBigExplosion(r, kShuttleRobotShipOrder);
	} else if (delta > 1) {
		Common::Rect r;
		_robotShip.getShuttleBounds(r);
		int size = MIN(r.width(), r.height());
		r = Common::Rect::center(impactPoint.x, impactPoint.y, size, size);
		showLittleExplosion(r, kShuttleWeaponBackOrder);
		TimeValue t = _rightShuttleMovie.getTime();
		_rightShuttleMovie.setTime(kShuttleRightDamagedTime);
		_rightShuttleMovie.redrawMovieWorld();
		_vm->delayShell(1, 3);
		_rightShuttleMovie.setTime(t);
		_rightShuttleMovie.redrawMovieWorld();
	}

	if (_rightDamageShuttleMovie.getTime() <= 40) {
		GameState.setScoringStoppedRobotsShuttle();
		if (!GameState.getMarsHitRobotWithCannon())
			GameState.setScoringMarsGandhi();
	}
}

void Mars::updateCursor(const Common::Point cursorLocation, const Hotspot *cursorSpot) {
	if (cursorSpot && cursorSpot->getObjectID() == kShuttleViewSpotID) {
		if (_weaponSelection != kNoWeapon)
			_vm->_cursor->setCurrentFrameIndex(6);
		else
			_vm->_cursor->setCurrentFrameIndex(0);
	} else {
		Neighborhood::updateCursor(cursorLocation, cursorSpot);
	}
}

AirQuality Mars::getAirQuality(const RoomID room) {
	if ((room >= kMars36 && room <= kMars39) || (room >= kMarsMaze004 && room <= kMarsMaze200))
		return kAirQualityVacuum;
	if (room == kMars35 && !GameState.getMarsAirlockOpen())
		return kAirQualityVacuum;
	if (room == kMars60 && !GameState.getMarsAirlockOpen())
		return kAirQualityVacuum;

	return Neighborhood::getAirQuality(room);
}

// Start up panting sound if necessary.

void Mars::checkAirMask() {
	Neighborhood::checkAirMask();

	if (getAirQuality(GameState.getCurrentRoom()) == kAirQualityVacuum) {
		if (g_airMask->isAirMaskOn()) {
			if (_noAirFuse.isFuseLit()) {
				_noAirFuse.stopFuse();
				loadLoopSound2("");
				loadAmbientLoops();
				playSpotSoundSync(kMarsOxyMaskOnIn, kMarsOxyMaskOnOut);
			}
		} else {
			if (!_noAirFuse.isFuseLit()) {
				loadLoopSound2("Sounds/Mars/SukWind1.22K.AIFF");
				_noAirFuse.primeFuse(kVacuumSurvivalTimeLimit);
				_noAirFuse.lightFuse();
			}
		}
	} else {
		if (_noAirFuse.isFuseLit()) {
			_noAirFuse.stopFuse();
			loadLoopSound2("");
			loadAmbientLoops();
		}
	}
}

void Mars::airStageExpired() {
	if (((PegasusEngine *)g_engine)->playerHasItemID(kAirMask))
		die(kDeathNoAirInMaze);
	else
		die(kDeathNoMaskInMaze);
}

void Mars::lockThawed() {
	startExtraSequence(kMars57ThawLock, kExtraCompletedFlag, kFilterNoInput);
}

void Mars::setUpReactorLevel1() {
	_reactorStage = 1;
	makeColorSequence();
	_guessObject.initReactorGuess();
	_undoPict.initFromPICTResource(_vm->_resFork, kReactorUndoHilitePICTID);
	_undoPict.setDisplayOrder(kMonitorLayer);
	_undoPict.moveElementTo(kUndoHiliteLeft, kUndoHiliteTop);
	_undoPict.startDisplaying();
	_guessHistory.initReactorHistory();
	_choiceHighlight.initReactorChoiceHighlight();
	setCurrentActivation(kActivateReactorInGame);
	_bombFuse.primeFuse(kColorMatchingTimeLimit);
	_bombFuse.setFunctor(new Common::Functor0Mem<void, Mars>(this, &Mars::bombExplodesInGame));
	_bombFuse.lightFuse();
}

void Mars::setUpNextReactorLevel() {
	_guessObject.show();
	_guessHistory.show();
	_guessHistory.clearHistory();
	_choiceHighlight.show();
	_reactorStage++;
	makeColorSequence();
}

void Mars::makeColorSequence() {
	int32 code[5];
	int32 highest = _reactorStage + 2;

	for (int32 i = 0; i < highest; i++)
		code[i] = i;

	_vm->shuffleArray(code, highest);
	_currentGuess[0] = -1;
	_currentGuess[1] = -1;
	_currentGuess[2] = -1;
	_nextGuess = 0;
	_guessObject.setGuess(-1, -1, -1);
	_guessHistory.setAnswer(code[0], code[1], code[2]);
}

void Mars::doUndoOneGuess() {
	if (_nextGuess > 0) {
		_undoPict.show();
		_vm->delayShell(1, 2);
		_undoPict.hide();
		_nextGuess--;
		_currentGuess[_nextGuess] = -1;
		_guessObject.setGuess(_currentGuess[0], _currentGuess[1], _currentGuess[2]);
		_choiceHighlight.resetHighlight();

		if (_currentGuess[0] != -1) {
			_choiceHighlight.highlightChoice(_currentGuess[0]);

			if (_currentGuess[1] != -1) {
				_choiceHighlight.highlightChoice(_currentGuess[1]);

				if (_currentGuess[2] != -1)
					_choiceHighlight.highlightChoice(_currentGuess[2]);
			}
		}
	}
}

void Mars::doReactorGuess(int32 guess) {
	_choiceHighlight.highlightChoice(guess);
	_currentGuess[_nextGuess] = guess;
	_guessObject.setGuess(_currentGuess[0], _currentGuess[1], _currentGuess[2]);

	switch (guess) {
	case 0:
		playSpotSoundSync(kColorMatchRedIn, kColorMatchRedOut);
		break;
	case 1:
		playSpotSoundSync(kColorMatchYellowIn, kColorMatchYellowOut);
		break;
	case 2:
		playSpotSoundSync(kColorMatchGreenIn, kColorMatchGreenOut);
		break;
	case 3:
		playSpotSoundSync(kColorMatchBlueIn, kColorMatchBlueOut);
		break;
	case 4:
		playSpotSoundSync(kColorMatchPurpleIn, kColorMatchPurpleOut);
		break;
	}

	_nextGuess++;

	if (_nextGuess == 3) {
		_vm->delayShell(1, 2);
		_nextGuess = 0;
		_guessHistory.addGuess(_currentGuess[0], _currentGuess[1], _currentGuess[2]);

		switch (_guessHistory.getCurrentNumCorrect()) {
		case 0:
			playSpotSoundSync(kColorMatchZeroNodesIn, kColorMatchZeroNodesOut);
			break;
		case 1:
			playSpotSoundSync(kColorMatchOneNodeIn, kColorMatchOneNodeOut);
			break;
		case 2:
			playSpotSoundSync(kColorMatchTwoNodesIn, kColorMatchTwoNodesOut);
			break;
		case 3:
			playSpotSoundSync(kColorMatchThreeNodesIn, kColorMatchThreeNodesOut);
			break;
		}

		_currentGuess[0] = -1;
		_currentGuess[1] = -1;
		_currentGuess[2] = -1;
		_guessObject.setGuess(-1, -1, -1);
		_choiceHighlight.resetHighlight();

		if (_guessHistory.isSolved()) {
			_guessHistory.showAnswer();
			_vm->delayShell(1, 2);
			_guessObject.hide();
			_guessHistory.hide();
			_choiceHighlight.hide();

			switch (_reactorStage) {
			case 1:
				startExtraSequence(kMars57GameLevel2, kExtraCompletedFlag, kFilterNoInput);
				break;
			case 2:
				startExtraSequence(kMars57GameLevel3, kExtraCompletedFlag, kFilterNoInput);
				break;
			case 3:
				_bombFuse.stopFuse();
				_guessObject.disposeReactorGuess();
				_undoPict.deallocateSurface();
				_guessHistory.disposeReactorHistory();
				_choiceHighlight.disposeReactorChoiceHighlight();
				GameState.setScoringDisarmedCardBomb();
				startExtraSequence(kMars57GameSolved, kExtraCompletedFlag, kFilterNoInput);
				break;
			}
		} else if (_guessHistory.getNumGuesses() >= 5) {
			_vm->delayShell(2, 1);
			bombExplodesInGame();
		}
	}
}

void Mars::bombExplodesInGame() {
	_guessObject.disposeReactorGuess();
	_undoPict.deallocateSurface();
	_guessHistory.disposeReactorHistory();
	_choiceHighlight.disposeReactorChoiceHighlight();
	startExtraSequence(kMars57BombExplodesInGame, kExtraCompletedFlag, kFilterNoInput);
}

void Mars::didntFindBomb() {
	die(kDeathDidntFindMarsBomb);
}

Common::String Mars::getBriefingMovie() {
	Common::String movieName = Neighborhood::getBriefingMovie();

	if (!movieName.empty())
		return movieName;

	return "Images/AI/Mars/XM01";
}

Common::String Mars::getEnvScanMovie() {
	Common::String movieName = Neighborhood::getEnvScanMovie();

	if (movieName.empty()) {
		RoomID room = GameState.getCurrentRoom();

		if (room >= kMars0A && room <= kMars21)
			return "Images/AI/Mars/XME1";
		else if (room >= kMars22 && room <= kMars31South)
			return "Images/AI/Mars/XME2";
		else if (room >= kMars52 && room <= kMars58)
			return "Images/AI/Mars/XMREACE";

		return "Images/AI/Mars/XME3";
	}

	return movieName;
}

uint Mars::getNumHints() {
	uint numHints = Neighborhood::getNumHints();

	if (numHints == 0) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kMars27, kNorth):
		case MakeRoomView(kMars28, kNorth):
		case MakeRoomView(kMars49, kSouth):
			numHints = 1;
			break;
		case MakeRoomView(kMars31, kSouth):
		case MakeRoomView(kMars31South, kSouth):
			if (!GameState.isTakenItemID(kMarsCard))
				numHints = 1;
			break;
		case MakeRoomView(kMars34, kNorth):
			if (!GameState.isTakenItemID(kMarsCard))
				numHints = 2;
			break;
		case MakeRoomView(kMars34, kSouth):
		case MakeRoomView(kMars45, kNorth):
			if (!GameState.isTakenItemID(kCrowbar))
				numHints = 1;
			break;
		case MakeRoomView(kMars51, kEast):
			if (GameState.isCurrentDoorOpen() && !GameState.getShieldOn()) {
				if (GameState.isTakenItemID(kShieldBiochip))
					numHints = 1;
				else
					numHints = 2;
			}
			break;
		case MakeRoomView(kMars52, kNorth):
		case MakeRoomView(kMars52, kSouth):
		case MakeRoomView(kMars52, kEast):
		case MakeRoomView(kMars52, kWest):
		case MakeRoomView(kMars54, kNorth):
		case MakeRoomView(kMars54, kSouth):
		case MakeRoomView(kMars54, kEast):
		case MakeRoomView(kMars54, kWest):
		case MakeRoomView(kMars56, kNorth):
		case MakeRoomView(kMars56, kSouth):
		case MakeRoomView(kMars56, kWest):
		case MakeRoomView(kMars58, kNorth):
		case MakeRoomView(kMars58, kSouth):
		case MakeRoomView(kMars58, kEast):
		case MakeRoomView(kMars58, kWest):
			if (!GameState.getShieldOn()) {
				if (GameState.isTakenItemID(kShieldBiochip))
					numHints = 1;
				else
					numHints = 2;
			}
			break;
		case MakeRoomView(kMars56, kEast):
			if (getCurrentActivation() == kActivateReactorReadyForNitrogen) {
				if ((ExtraID)_lastExtra == kMars57LowerScreenClosed)
					numHints = 3;
			} else if (getCurrentActivation() == kActivateReactorPlatformOut) {
				if (!GameState.getShieldOn()) {
					if (GameState.isTakenItemID(kShieldBiochip))
						numHints = 1;
					else
						numHints = 2;
				}
			}
			break;
		}
	}

	return numHints;
}

Common::String Mars::getHintMovie(uint hintNum) {
	Common::String movieName = Neighborhood::getHintMovie(hintNum);

	if (movieName.empty()) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kMars27, kNorth):
		case MakeRoomView(kMars28, kNorth):
			return "Images/AI/Globals/XGLOB5C";
		case MakeRoomView(kMars31, kSouth):
		case MakeRoomView(kMars31South, kSouth):
		case MakeRoomView(kMars34, kSouth):
		case MakeRoomView(kMars45, kNorth):
			return "Images/AI/Globals/XGLOB1C";
		case MakeRoomView(kMars34, kNorth):
			if (hintNum == 1)
				return "Images/AI/Globals/XGLOB2C";

			return "Images/AI/Globals/XGLOB3G";
		case MakeRoomView(kMars49, kSouth):
			if (GameState.isTakenItemID(kAirMask))
				return "Images/AI/Globals/XGLOB3E";

			return "Images/AI/Globals/XGLOB1C";
		case MakeRoomView(kMars51, kEast):
			if (GameState.isTakenItemID(kShieldBiochip))
				return "Images/AI/Mars/XM52NW";

			if (hintNum == 1)
				return "Images/AI/Globals/XGLOB2D";

			return "Images/AI/Globals/XGLOB3F";
		case MakeRoomView(kMars52, kNorth):
		case MakeRoomView(kMars52, kSouth):
		case MakeRoomView(kMars52, kEast):
		case MakeRoomView(kMars52, kWest):
		case MakeRoomView(kMars54, kNorth):
		case MakeRoomView(kMars54, kSouth):
		case MakeRoomView(kMars54, kEast):
		case MakeRoomView(kMars54, kWest):
		case MakeRoomView(kMars56, kNorth):
		case MakeRoomView(kMars56, kSouth):
		case MakeRoomView(kMars56, kWest):
		case MakeRoomView(kMars58, kNorth):
		case MakeRoomView(kMars58, kSouth):
		case MakeRoomView(kMars58, kEast):
		case MakeRoomView(kMars58, kWest):
			if (hintNum == 1) {
				if (GameState.isTakenItemID(kShieldBiochip))
					return "Images/AI/Mars/XM52NW";

				return "Images/AI/Globals/XGLOB2D";
			}

			return "Images/AI/Globals/XGLOB3F";
		case MakeRoomView(kMars56, kEast):
			if (getCurrentActivation() == kActivateReactorReadyForNitrogen)
				return Common::String::format("Images/AI/Mars/XM57SD%d", hintNum);

			if (hintNum == 1) {
				if (GameState.isTakenItemID(kShieldBiochip))
					return "Images/AI/Mars/XM52NW";

				return "Images/AI/Globals/XGLOB2D";
			}

			return "Images/AI/Globals/XGLOB3F";
		}
	}

	return movieName;
}

bool Mars::inColorMatchingGame() {
	return _guessObject.isDisplaying();
}

bool Mars::canSolve() {
	return GameState.getCurrentRoomAndView() == MakeRoomView(kMars56, kEast) && (getCurrentActivation() == kActivateReactorReadyForNitrogen ||
			getCurrentActivation() == kActivateReactorReadyForCrowBar || inColorMatchingGame());
}

void Mars::doSolve() {
	if (getCurrentActivation() == kActivateReactorReadyForNitrogen || getCurrentActivation() == kActivateReactorReadyForCrowBar) {
		_utilityFuse.stopFuse();
		GameState.setMarsLockBroken(true);
		GameState.setMarsLockFrozen(false);
		startExtraLongSequence(kMars57OpenPanel, kMars57OpenPanelChoices, kExtraCompletedFlag, kFilterNoInput);
	} else if (inColorMatchingGame()) {
		_bombFuse.stopFuse();
		_guessObject.disposeReactorGuess();
		_undoPict.deallocateSurface();
		_guessHistory.disposeReactorHistory();
		_choiceHighlight.disposeReactorChoiceHighlight();
		startExtraSequence(kMars57GameSolved, kExtraCompletedFlag, kFilterNoInput);
	}
}

Common::String Mars::getSoundSpotsName() {
	return "Sounds/Mars/Mars Spots";
}

Common::String Mars::getNavMovieName() {
	return "Images/Mars/Mars.movie";
}

} // End of namespace Pegasus
