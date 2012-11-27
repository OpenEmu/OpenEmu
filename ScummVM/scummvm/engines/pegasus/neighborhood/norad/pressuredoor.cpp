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

#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/neighborhood/norad/constants.h"
#include "pegasus/neighborhood/norad/norad.h"
#include "pegasus/neighborhood/norad/pressuredoor.h"
#include "pegasus/neighborhood/norad/delta/noraddelta.h"

namespace Pegasus {

static const TimeValue kLevelsSplashStart = 0;
static const TimeValue kLevelsSplashStop = 1;
static const TimeValue kPressureBase = 1;

static const TimeValue kDoorSealedTime = 0;
static const TimeValue kEqualizeTime = 1;
static const TimeValue kMaxPressureLoopStart = 2;
static const TimeValue kMaxPressureLoopStop = 3;
static const TimeValue kOpeningDoorLoopStart = 3;
static const TimeValue kOpeningDoorLoopStop = 4;
static const TimeValue kIncreasingPressureTime = 4;
static const TimeValue kDecreasingPressureTime = 5;
static const TimeValue kCautionLoopStart = 6;
static const TimeValue kCautionLoopStop = 7;

static const NotificationFlags kSplashFinished = 1;
static const NotificationFlags kPressureDroppingFlag = kSplashFinished << 1;

static const NotificationFlags kPressureNotificationFlags = kSplashFinished |
														kPressureDroppingFlag;

static const NotificationFlags kDoorJumpsUpFlag = 1;
static const NotificationFlags kDoorJumpsBackFlag = kDoorJumpsUpFlag << 1;
static const NotificationFlags kDoorCrushedFlag = kDoorJumpsBackFlag << 1;

static const NotificationFlags kUtilityNotificationFlags = kDoorJumpsUpFlag |
														kDoorJumpsBackFlag |
														kDoorCrushedFlag;

enum {
	kPlayingRobotApproaching,
	kRobotPunching,
	kRobotComingThrough,
	kRobotDying,
	kRobotDead
};

const short kMaxPunches = 5;

enum {
	kPlayingSplash,
	kPlayingPressureMessage,
	kPlayingEqualizeMessage,
	kWaitingForPlayer,
	kPlayingDoneMessage,
	kGameOver
};

// Pressure values range from 0 to 11.
static const short kMinPressure = 0;
static const short kMaxPressure = 11;

static const TimeScale kNavTimeScale = 600;
static const TimeValue kNavFrameRate = 15;
static const TimeValue kNavTimePerFrame = kNavTimeScale / kNavFrameRate;

static const TimeValue kApproachPunchInTime = 122 * kNavTimePerFrame;
static const TimeValue kLoopPunchInTime = 38 * kNavTimePerFrame;
static const TimeValue kPunchThroughTime = 38 * kNavTimePerFrame;

// Pressure door PICTs:
static const ResIDType kUpperPressureUpOffPICTID = 400;
static const ResIDType kUpperPressureUpOnPICTID = 401;
static const ResIDType kUpperPressureDownOffPICTID = 402;
static const ResIDType kUpperPressureDownOnPICTID = 403;

static const ResIDType kLowerPressureUpOffPICTID = 404;
static const ResIDType kLowerPressureUpOnPICTID = 405;
static const ResIDType kLowerPressureDownOffPICTID = 406;
static const ResIDType kLowerPressureDownOnPICTID = 407;

PressureDoor::PressureDoor(Neighborhood *handler, bool isUpperDoor, const HotSpotID upSpotID, const HotSpotID downSpotID,
		const HotSpotID outSpotID, TimeValue pressureSoundIn, TimeValue pressureSoundOut, TimeValue equalizeSoundIn,
		TimeValue equalizeSoundOut) : GameInteraction(kNoradPressureDoorInteractionID, handler),
		_levelsMovie(kPressureDoorLevelsID), _typeMovie(kPressureDoorTypeID), _upButton(kPressureDoorUpButtonID),
		_downButton(kPressureDoorDownButtonID), _pressureNotification(kNoradPressureNotificationID, ((PegasusEngine *)g_engine)),
		_doorTracker(this), _utilityNotification(kNoradUtilityNotificationID, ((PegasusEngine *)g_engine)) {
	_neighborhoodNotification = handler->getNeighborhoodNotification();
	_upHotspotID = upSpotID;
	_downHotspotID = downSpotID;
	_outHotspotID = outSpotID;
	_pressureSoundIn = pressureSoundIn;
	_pressureSoundOut = pressureSoundOut;
	_equalizeSoundIn = equalizeSoundIn;
	_equalizeSoundOut = equalizeSoundOut;
	_playingAgainstRobot = false;
	_isUpperDoor = isUpperDoor;
}

void PressureDoor::openInteraction() {
	if (_isUpperDoor) {
		_levelsMovie.initFromMovieFile("Images/Norad Alpha/Upper Levels Movie");
		_levelsMovie.moveElementTo(kNoradUpperLevelsLeft, kNoradUpperLevelsTop);
	} else {
		_levelsMovie.initFromMovieFile("Images/Norad Alpha/Lower Levels Movie");
		_levelsMovie.moveElementTo(kNoradLowerLevelsLeft, kNoradLowerLevelsTop);
	}

	_levelsScale = _levelsMovie.getScale();
	_levelsMovie.setDisplayOrder(kPressureLevelsOrder);
	_levelsMovie.startDisplaying();
	_levelsMovie.setSegment(kLevelsSplashStart * _levelsScale, kLevelsSplashStop * _levelsScale);
	_levelsMovie.setTime(kLevelsSplashStart * _levelsScale);
	_levelsMovie.redrawMovieWorld();
	_levelsMovie.show();

	_pressureCallBack.setNotification(&_pressureNotification);
	_pressureCallBack.initCallBack(&_levelsMovie, kCallBackAtExtremes);
	_pressureCallBack.setCallBackFlag(kSplashFinished);
	_pressureCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	_pressureNotification.notifyMe(this, kPressureNotificationFlags, kPressureNotificationFlags);

	if (_isUpperDoor) {
		_typeMovie.initFromMovieFile("Images/Norad Alpha/Upper Type Movie");
		_typeMovie.moveElementTo(kNoradUpperTypeLeft, kNoradUpperTypeTop);
	} else {
		_typeMovie.initFromMovieFile("Images/Norad Alpha/Lower Type Movie");
		_typeMovie.moveElementTo(kNoradLowerTypeLeft, kNoradLowerTypeTop);
	}

	_typeScale = _typeMovie.getScale();
	_typeMovie.setDisplayOrder(kPressureTypeOrder);
	_typeMovie.startDisplaying();
	_typeMovie.setTime(kDoorSealedTime * _typeScale);
	_typeMovie.redrawMovieWorld();

	SpriteFrame *frame = new SpriteFrame();
	if (_isUpperDoor)
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kLowerPressureUpOffPICTID);
	else
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kUpperPressureUpOffPICTID);
	_upButton.addFrame(frame, 0, 0);

	frame = new SpriteFrame();
	if (_isUpperDoor)
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kLowerPressureUpOnPICTID);
	else
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kUpperPressureUpOnPICTID);
	_upButton.addFrame(frame, 0, 0);

	_upButton.setCurrentFrameIndex(0);
	_upButton.setDisplayOrder(kPressureUpOrder);

	Common::Rect r;
	frame->getSurfaceBounds(r);
	if (_isUpperDoor)
		r.moveTo(kNoradUpperUpLeft, kNoradUpperUpTop);
	else
		r.moveTo(kNoradLowerUpLeft, kNoradLowerUpTop);

	_upButton.setBounds(r);
	_upButton.startDisplaying();
	_upButton.show();

	frame = new SpriteFrame();
	if (_isUpperDoor)
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kLowerPressureDownOffPICTID);
	else
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kUpperPressureDownOffPICTID);
	_downButton.addFrame(frame, 0, 0);

	frame = new SpriteFrame();
	if (_isUpperDoor)
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kLowerPressureDownOnPICTID);
	else
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kUpperPressureDownOnPICTID);
	_downButton.addFrame(frame, 0, 0);

	_downButton.setCurrentFrameIndex(0);
	_downButton.setDisplayOrder(kPressureDownOrder);

	frame->getSurfaceBounds(r);
	if (_isUpperDoor)
		r.moveTo(kNoradUpperDownLeft, kNoradUpperDownTop);
	else
		r.moveTo(kNoradLowerDownLeft, kNoradLowerDownTop);

	_downButton.setBounds(r);
	_downButton.startDisplaying();
	_downButton.show();

	_utilityCallBack.setNotification(&_utilityNotification);
	_utilityCallBack.initCallBack(&_utilityTimer, kCallBackAtTime);
	_utilityNotification.notifyMe(this, kUtilityNotificationFlags, kUtilityNotificationFlags);
	_utilityTimer.setMasterTimeBase(getOwner()->getNavMovie());

	if (_playingAgainstRobot)
		_neighborhoodNotification->notifyMe(this, kExtraCompletedFlag | kDelayCompletedFlag |
				kSpotSoundCompletedFlag, kExtraCompletedFlag | kDelayCompletedFlag | kSpotSoundCompletedFlag);
	else
		_neighborhoodNotification->notifyMe(this, kDelayCompletedFlag | kSpotSoundCompletedFlag,
				kDelayCompletedFlag | kSpotSoundCompletedFlag);

	_gameState = kPlayingSplash;
}

void PressureDoor::initInteraction() {
	_levelsMovie.start();

	if (_playingAgainstRobot) {
		ExtraTable::Entry entry;
		_owner->getExtraEntry(kN59RobotApproaches, entry);
		_utilityTimer.setSegment(entry.movieStart, entry.movieEnd);
		_utilityCallBack.setCallBackFlag(kDoorJumpsUpFlag);
		_punchInTime = kApproachPunchInTime + entry.movieStart;
		_utilityCallBack.scheduleCallBack(kTriggerTimeFwd, _punchInTime, kNavTimeScale);
		_utilityTimer.setTime(entry.movieStart);
		_owner->startExtraSequence(kN59RobotApproaches, kExtraCompletedFlag, kFilterAllInput);
		_utilityTimer.start();
		_robotState = kPlayingRobotApproaching;
	}

	_levelsMovie.redrawMovieWorld();
}

void PressureDoor::closeInteraction() {
	_pressureNotification.cancelNotification(this);
	_pressureCallBack.releaseCallBack();
	_utilityNotification.cancelNotification(this);
	_utilityCallBack.releaseCallBack();
	_neighborhoodNotification->cancelNotification(this);
}

void PressureDoor::playAgainstRobot() {
	_playingAgainstRobot = true;
}

void PressureDoor::receiveNotification(Notification *notification, const NotificationFlags flags) {
	Neighborhood *owner = getOwner();

	if (notification == _neighborhoodNotification) {
		if (_playingAgainstRobot && (flags & kExtraCompletedFlag) != 0) {
			ExtraTable::Entry entry;

			switch (_robotState) {
			case kPlayingRobotApproaching:
				_utilityTimer.stop();
				if (GameState.getNoradSubRoomPressure() == kMaxPressure) {
					owner->getExtraEntry(kN59PlayerWins1, entry);
					_utilityTimer.setSegment(entry.movieStart, entry.movieEnd);
					_utilityTimer.setTime(entry.movieStart);
					_utilityCallBack.setCallBackFlag(kDoorJumpsUpFlag);
					_punchInTime = kLoopPunchInTime + entry.movieStart;
					_utilityCallBack.scheduleCallBack(kTriggerTimeFwd, _punchInTime, kNavTimeScale);
					owner->startExtraSequence(kN59PlayerWins1, kExtraCompletedFlag, kFilterNoInput);
					_utilityTimer.start();
					_robotState = kRobotDying;
				} else {
					owner->getExtraEntry(kN59RobotPunchLoop, entry);
					_utilityTimer.setSegment(entry.movieStart, entry.movieEnd);
					_utilityTimer.setTime(entry.movieStart);
					_utilityCallBack.setCallBackFlag(kDoorJumpsUpFlag);
					_punchInTime = kLoopPunchInTime + entry.movieStart;
					_utilityCallBack.scheduleCallBack(kTriggerTimeFwd, _punchInTime, kNavTimeScale);
					owner->startSpotLoop(entry.movieStart, entry.movieEnd, kExtraCompletedFlag);
					_utilityTimer.start();
					_robotState = kRobotPunching;
					_punchCount = 1;
				}
				break;
			case kRobotPunching:
				if (GameState.getNoradSubRoomPressure() == kMaxPressure) {
					owner->startExtraSequence(kN59PlayerWins1, kExtraCompletedFlag, kFilterNoInput);
					_robotState = kRobotDying;
				} else if (++_punchCount >= kMaxPunches) {
					_robotState = kRobotComingThrough;
					owner->getExtraEntry(kN59RobotWins, entry);
					_utilityTimer.stop();
					_utilityTimer.setSegment(entry.movieStart, entry.movieEnd);
					_utilityTimer.setTime(entry.movieStart);
					_utilityCallBack.cancelCallBack();
					_utilityCallBack.setCallBackFlag(kDoorCrushedFlag);
					_utilityCallBack.scheduleCallBack(kTriggerTimeFwd, kPunchThroughTime + entry.movieStart, kNavTimeScale);
					owner->startExtraSequence(kN59RobotWins, kExtraCompletedFlag, kFilterNoInput);
					_utilityTimer.start();
				} else {
					_utilityCallBack.setCallBackFlag(kDoorJumpsUpFlag);
					_utilityCallBack.scheduleCallBack(kTriggerTimeFwd, _punchInTime, kNavTimeScale);
					owner->scheduleNavCallBack(kExtraCompletedFlag);
				}
				break;
			case kRobotComingThrough:
				g_system->delayMillis(2 * 1000);
				((PegasusEngine *)g_engine)->die(kDeathRobotThroughNoradDoor);
				break;
			case kRobotDying:
				_robotState = kRobotDead;
				_levelsMovie.stop();
				_levelsMovie.setSegment((kNormalSubRoomPressure + kPressureBase) * _levelsScale,
						(GameState.getNoradSubRoomPressure() + kPressureBase) * _levelsScale);
				_pressureCallBack.setCallBackFlag(kPressureDroppingFlag);
				_pressureCallBack.scheduleCallBack(kTriggerAtStart, 0, 0);
				_typeMovie.stop();
				_typeMovie.setSegment(0, _typeMovie.getDuration());
				_typeMovie.setTime(kDecreasingPressureTime * _typeScale);
				_typeMovie.redrawMovieWorld();
				_typeMovie.show();
				_downButton.show();
				_downButton.setCurrentFrameIndex(1);
				_gameState = kGameOver;
				allowInput(false);
				_levelsMovie.setRate(Common::Rational(0x5555, 0x10000) - 1); // Should match door tracker.
				break;
			case kRobotDead:
				allowInput(true);
				((NoradDelta *)owner)->playerBeatRobotWithDoor();
				owner->requestDeleteCurrentInteraction();
				break;
			}
		}

		if ((flags & (kDelayCompletedFlag | kSpotSoundCompletedFlag)) != 0) {
			switch (_gameState) {
			case kPlayingPressureMessage:
				_typeMovie.setTime(kEqualizeTime * _typeScale);
				_typeMovie.redrawMovieWorld();
				owner->requestDelay(1, 5, kFilterNoInput, 0);
				owner->requestSpotSound(_equalizeSoundIn, _equalizeSoundOut, kFilterNoInput, 0);
				owner->requestDelay(1, 5, kFilterNoInput, kDelayCompletedFlag);
				_gameState = kPlayingEqualizeMessage;
				break;
			case kPlayingEqualizeMessage:
				_gameState = kWaitingForPlayer;
				stopChangingPressure();
				break;
			case kPlayingDoneMessage:
				_gameState = kWaitingForPlayer;
				_typeMovie.stop();
				_typeMovie.setFlags(0);
				_typeMovie.hide();
				if (!_playingAgainstRobot)
					((Norad *)_owner)->doneWithPressureDoor();
				break;
			}
		}
	} else if (notification == &_pressureNotification) {
		switch (flags) {
		case kSplashFinished:
			_levelsMovie.stop();
			_levelsMovie.setSegment(0, _levelsMovie.getDuration());
			_levelsMovie.setTime((GameState.getNoradSubRoomPressure() + kPressureBase) * _levelsScale);
			_levelsMovie.redrawMovieWorld();

			if (GameState.getNoradSubRoomPressure() != kNormalSubRoomPressure) {
				_typeMovie.show();
				owner->requestDelay(1, 5, kFilterNoInput, 0);
				owner->requestSpotSound(_pressureSoundIn, _pressureSoundOut, kFilterNoInput, 0);
				owner->requestDelay(1, 5, kFilterNoInput, kDelayCompletedFlag);
				_gameState = kPlayingPressureMessage;
			} else {
				_gameState = kWaitingForPlayer;
			}
			break;
		case kPressureDroppingFlag:
			_levelsMovie.stop();
			_levelsMovie.hide();
			_typeMovie.stop();
			_typeMovie.hide();
			_upButton.hide();
			_downButton.hide();
			owner->startExtraSequence(kN59PlayerWins2, kExtraCompletedFlag, kFilterNoInput);
			break;
		}
	} else if (notification == &_utilityNotification) {
		switch (flags) {
		case kDoorJumpsUpFlag:
			_utilityCallBack.setCallBackFlag(kDoorJumpsBackFlag);
			_utilityCallBack.scheduleCallBack(kTriggerTimeFwd, _punchInTime + kNavTimePerFrame, kNavTimeScale);
			_levelsMovie.hide();
			_typePunched = _typeMovie.isVisible();
			if (_typePunched == true)
				_typeMovie.hide();
			_upButton.hide();
			_downButton.hide();
			break;
		case kDoorJumpsBackFlag:
			_levelsMovie.show();
			_upButton.show();
			_downButton.show();
			if (_typePunched)
				_typeMovie.show();
			break;
		case kDoorCrushedFlag:
			_levelsMovie.hide();
			_typeMovie.hide();
			_upButton.hide();
			_downButton.hide();
			break;
		}
	}
}

void PressureDoor::activateHotspots() {
	GameInteraction::activateHotspots();

	switch (_gameState) {
	case kWaitingForPlayer:
		g_allHotspots.activateOneHotspot(_upHotspotID);
		g_allHotspots.activateOneHotspot(_downHotspotID);
		if (!_playingAgainstRobot)
			g_allHotspots.activateOneHotspot(_outHotspotID);
		break;
	default:
		break;
	}
}

void PressureDoor::clickInHotspot(const Input &input, const Hotspot *spot) {
	HotSpotID id = spot->getObjectID();

	if (id == _upHotspotID || id == _downHotspotID) {
		if (id == _upHotspotID)
			_doorTracker.setTrackParameters(spot, &_upButton);
		else
			_doorTracker.setTrackParameters(spot, &_downButton);

		_doorTracker.startTracking(input);
	} else {
		GameInteraction::clickInHotspot(input, spot);
	}
}

void PressureDoor::incrementPressure(const HotSpotID id) {
	_typeMovie.stop();
	_typeMovie.setSegment(0, _typeMovie.getDuration());
	_typeMovie.setFlags(0);

	if (id == _upHotspotID) {
		if (GameState.getNoradSubRoomPressure() < kMaxPressure) {
			GameState.setNoradSubRoomPressure(GameState.getNoradSubRoomPressure() + 1);
			_levelsMovie.setTime((GameState.getNoradSubRoomPressure() + kPressureBase) * _levelsScale);
			_levelsMovie.redrawMovieWorld();
			_typeMovie.setTime(kIncreasingPressureTime * _typeScale);
			_typeMovie.redrawMovieWorld();
			_typeMovie.show();
			g_AIArea->checkMiddleArea();
		} else {
			_typeMovie.hide();
		}
	} else if (id == _downHotspotID) {
		if (GameState.getNoradSubRoomPressure() > kMinPressure) {
			GameState.setNoradSubRoomPressure(GameState.getNoradSubRoomPressure() - 1);
			_levelsMovie.setTime((GameState.getNoradSubRoomPressure() + kPressureBase) * _levelsScale);
			_levelsMovie.redrawMovieWorld();
			_typeMovie.setTime(kDecreasingPressureTime * _typeScale);
			_typeMovie.redrawMovieWorld();
			_typeMovie.show();
			g_AIArea->checkMiddleArea();
		} else {
			_typeMovie.hide();
		}
	}
}

void PressureDoor::stopChangingPressure() {
	Neighborhood *owner;

	switch (GameState.getNoradSubRoomPressure()) {
	case 11:
		_typeMovie.setSegment(kMaxPressureLoopStart * _typeScale, kMaxPressureLoopStop * _typeScale);
		_typeMovie.setFlags(kLoopTimeBase);
		_typeMovie.show();
		_typeMovie.start();
		break;
	case 10:
		_typeMovie.setSegment(kCautionLoopStart * _typeScale, kCautionLoopStop * _typeScale);
		_typeMovie.setFlags(kLoopTimeBase);
		_typeMovie.show();
		_typeMovie.start();
		break;
	case kNormalSubRoomPressure:
		owner = getOwner();
		_typeMovie.setSegment(kOpeningDoorLoopStart * _typeScale, kOpeningDoorLoopStop * _typeScale);
		_typeMovie.setFlags(kLoopTimeBase);
		_typeMovie.show();
		_gameState = kPlayingDoneMessage;
		owner->requestDelay(2, 1, kFilterNoInput, kDelayCompletedFlag);
		_typeMovie.start();
		break;
	default:
		_typeMovie.hide();
		break;
	}
}

bool PressureDoor::canSolve() {
	if (_playingAgainstRobot)
		return GameState.getNoradSubRoomPressure() < 11;

	return GameState.getNoradSubRoomPressure() != kNormalSubRoomPressure;
}

void PressureDoor::doSolve() {
	if (_playingAgainstRobot) {
		GameState.setNoradSubRoomPressure(11);
		_levelsMovie.setTime((11 + kPressureBase) * _levelsScale);
		_levelsMovie.redrawMovieWorld();
		_typeMovie.setSegment(kMaxPressureLoopStart * _typeScale, kMaxPressureLoopStop * _typeScale);
		_typeMovie.setFlags(kLoopTimeBase);
		_typeMovie.show();
		_typeMovie.start();
		g_AIArea->checkMiddleArea();
	} else {
		GameState.setNoradSubRoomPressure(kNormalSubRoomPressure);
		_levelsMovie.setTime((kNormalSubRoomPressure + kPressureBase) * _levelsScale);
		_levelsMovie.redrawMovieWorld();
		_typeMovie.setSegment(kOpeningDoorLoopStart * _typeScale, kOpeningDoorLoopStop * _typeScale);
		_typeMovie.setFlags(kLoopTimeBase);
		_typeMovie.show();
		Neighborhood *owner = getOwner();
		owner->requestDelay(2, 1, kFilterNoInput, kDelayCompletedFlag);
		_gameState = kPlayingDoneMessage;
		_typeMovie.start();
		g_AIArea->checkMiddleArea();
	}
}

} // End of namespace Pegasus
