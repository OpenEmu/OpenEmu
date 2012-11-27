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

#include "pegasus/cursor.h"
#include "pegasus/pegasus.h"
#include "pegasus/neighborhood/norad/constants.h"
#include "pegasus/neighborhood/norad/delta/globegame.h"
#include "pegasus/neighborhood/norad/delta/noraddelta.h"

namespace Pegasus {

static const TimeValue kDurationPerFrame = 600 / 15;
static const TimeValue kDurationPerRow = kNumLongSlices * kDurationPerFrame;
static const short kVerticalDuration = 16;

GlobeTracker::GlobeTracker(Movie *globeMovie, Picture *leftHighlight, Picture *rightHighlight,
		Picture *upHighlight, Picture *downHighlight) {
	_globeMovie = globeMovie;
	_leftHighlight = leftHighlight;
	_rightHighlight = rightHighlight;
	_upHighlight = upHighlight;
	_downHighlight = downHighlight;
}

void GlobeTracker::setTrackParameters(const Hotspot *trackSpot, GlobeTrackDirection direction) {
	_trackSpot = trackSpot;
	_trackDirection = direction;

	TimeValue time, newTime, start;

	switch (_trackDirection) {
	case kTrackLeft:
		time = _globeMovie->getTime();

		if (((time / kDurationPerRow) & 1) == 0) {
			start = (time / kDurationPerRow + 1) * kDurationPerRow;
			newTime = start + kDurationPerRow - time % kDurationPerRow;
		} else {
			start = (time / kDurationPerRow) * kDurationPerRow;
			newTime = time;
		}

		_globeMovie->setSegment(start, start + kDurationPerRow);

		if (newTime != time) {
			_globeMovie->setTime(newTime);
			_globeMovie->redrawMovieWorld();
		}

		_globeMovie->setFlags(kLoopTimeBase);
		break;
	case kTrackRight:
		time = _globeMovie->getTime();

		if (((time / kDurationPerRow) & 1) == 0) {
			start = (time / kDurationPerRow) * kDurationPerRow;
			newTime = time;
		} else {
			start = (time / kDurationPerRow - 1) * kDurationPerRow;
			newTime = start + kDurationPerRow - time % kDurationPerRow;
		}

		_globeMovie->setSegment(start, start + kDurationPerRow);

		if (newTime != time) {
			_globeMovie->setTime(newTime);
			_globeMovie->redrawMovieWorld();
		}

		_globeMovie->setFlags(kLoopTimeBase);
		break;
	case kTrackUp:
	case kTrackDown:
		_globeMovie->setSegment(0, _globeMovie->getDuration());
		_globeMovie->setFlags(0);
		break;
	}
}

void GlobeTracker::activateHotspots() {
	Tracker::activateHotspots();

	if (_trackSpot)
		g_allHotspots.activateOneHotspot(_trackSpot->getObjectID());
}

bool GlobeTracker::stopTrackingInput(const Input &input) {
	return !JMPPPInput::isPressingInput(input);
}

void GlobeTracker::continueTracking(const Input &input) {
	Common::Point where;
	input.getInputLocation(where);

	if (g_allHotspots.findHotspot(where) == _trackSpot)
		trackGlobeMovie();
	else
		stopGlobeMovie();
}

void GlobeTracker::startTracking(const Input &input) {
	Tracker::startTracking(input);
	trackGlobeMovie();
}

void GlobeTracker::stopTracking(const Input &input) {
	Tracker::stopTracking(input);
	stopGlobeMovie();
}

void GlobeTracker::trackGlobeMovie() {
	TimeValue time;

	switch (_trackDirection) {
	case kTrackLeft:
		if (!_globeMovie->isRunning())
			_globeMovie->start();

		_leftHighlight->show();
		break;
	case kTrackRight:
		if (!_globeMovie->isRunning())
			_globeMovie->start();

		_rightHighlight->show();
		break;
	case kTrackUp:
		time = _globeMovie->getTime();

		if (_trackTime == 0) {
			_trackTime = tickCount();
		} else if ((int)time - (int)kDurationPerRow * 2 >= 0 && (int)tickCount() >= _trackTime + kVerticalDuration) {
			_trackTime = tickCount();
			_globeMovie->setTime(time - kDurationPerRow * 2);
			_globeMovie->redrawMovieWorld();
		}

		_upHighlight->show();
		break;
	case kTrackDown:
		time = _globeMovie->getTime();

		if (_trackTime == 0) {
			_trackTime = tickCount();
		} else if (time + kDurationPerRow * 2 < _globeMovie->getDuration() && (int)tickCount() >= _trackTime + kVerticalDuration) {
			_trackTime = tickCount();
			_globeMovie->setTime(time + kDurationPerRow * 2);
			_globeMovie->redrawMovieWorld();
		}

		_downHighlight->show();
		break;
	}
}

void GlobeTracker::stopGlobeMovie() {
	switch (_trackDirection) {
	case kTrackLeft:
		_leftHighlight->hide();
		_globeMovie->stop();
		break;
	case kTrackRight:
		_rightHighlight->hide();
		_globeMovie->stop();
		break;
	case kTrackUp:
		_upHighlight->hide();
		_trackTime = tickCount() - kVerticalDuration;
		break;
	case kTrackDown:
		_downHighlight->hide();
		_trackTime = tickCount() - kVerticalDuration;
		break;
	}
}

// Globe game PICTs:
static const ResIDType kGlobeCircleLeftPICTID = 300;
static const ResIDType kGlobeCircleRightPICTID = 301;
static const ResIDType kGlobeCircleUpPICTID = 302;
static const ResIDType kGlobeCircleDownPICTID = 303;
static const ResIDType kTargetUpperLeftPICTID = 304;
static const ResIDType kTargetUpperRightPICTID = 305;
static const ResIDType kTargetLowerLeftPICTID = 306;
static const ResIDType kTargetLowerRightPICTID = 307;
static const ResIDType kMotionHiliteLeftPICTID = 308;
static const ResIDType kMotionHiliteRightPICTID = 309;
static const ResIDType kMotionHiliteUpPICTID = 310;
static const ResIDType kMotionHiliteDownPICTID = 311;

static const ResIDType kGlobeCountdownDigitsID = 350;

static const int kGlobeCountdownWidth = 28;
static const int kGlobeCountdownHeight = 12;
static const int kGlobeCountdownOffset1 = 12;
static const int kGlobeCountdownOffset2 = 20;

GlobeCountdown::GlobeCountdown(const DisplayElementID id) : IdlerAnimation(id) {
	_digits.getImageFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kGlobeCountdownDigitsID);

	Common::Rect r;
	_digits.getSurfaceBounds(r);
	_digitOffset = r.width() / 10;
	setScale(1);
	sizeElement(kGlobeCountdownWidth, kGlobeCountdownHeight);
}

void GlobeCountdown::setDisplayOrder(const DisplayOrder order) {
	IdlerAnimation::setDisplayOrder(order);
}

void GlobeCountdown::show() {
	IdlerAnimation::show();
}

void GlobeCountdown::hide() {
	IdlerAnimation::hide();
}

void GlobeCountdown::moveElementTo(const CoordType x, const CoordType y) {
	IdlerAnimation::moveElementTo(x, y);
}

void GlobeCountdown::setCountdownTime(const int numSeconds) {
	stop();
	setSegment(0, numSeconds);
	setTime(numSeconds);
}

void GlobeCountdown::startCountdown() {
	setRate(-1);
}

void GlobeCountdown::stopCountdown() {
	stop();
}

void GlobeCountdown::draw(const Common::Rect &) {
	Common::Rect r1;
	_digits.getSurfaceBounds(r1);
	r1.right = r1.left + _digitOffset;
	Common::Rect r2 = r1;
	TimeValue time = getTime();

	Common::Rect bounds;
	getBounds(bounds);

	if (time > 60 * 9 + 59) {
		r2.moveTo(bounds.left, bounds.top);
		r1.moveTo(9 * _digitOffset, 0);
		_digits.copyToCurrentPort(r1, r2);

		r2.moveTo(bounds.left + kGlobeCountdownOffset1, bounds.top);
		r1.moveTo(5 * _digitOffset, 0);
		_digits.copyToCurrentPort(r1, r2);

		r2.moveTo(bounds.left + kGlobeCountdownOffset2, bounds.top);
		r1.moveTo(9 * _digitOffset, 0);
		_digits.copyToCurrentPort(r1, r2);
	} else {
		r2.moveTo(bounds.left, bounds.top);
		r1.moveTo((time / 60) * _digitOffset, 0);
		_digits.copyToCurrentPort(r1, r2);

		time %= 60;
		r2.moveTo(bounds.left + kGlobeCountdownOffset1, bounds.top);
		r1.moveTo((time / 10) * _digitOffset, 0);
		_digits.copyToCurrentPort(r1, r2);

		r2.moveTo(bounds.left + kGlobeCountdownOffset2, bounds.top);
		r1.moveTo((time % 10) * _digitOffset, 0);
		_digits.copyToCurrentPort(r1, r2);
	}
}

const int16 GlobeGame::_siloCoords[kNumAllSilos][2] = {
	{ 60, -151 }, // Anchorage, Alaska
	{ 6, 39 },    // Addis Ababa, Ethiopia
	{ -22, 44 },  // Antaro, Madagascar
	{ 30, -83 },  // Atlanta, Georgia
	{ -41, 173 }, // Auckland, New Zealand
	{ 39, -78 },  // Baltimore, Maryland
	{ 11, 101 },  // Bangkok, Thailand
	{ 2, -75 },   // Bogota, Colombia
	{ 46, 4 },    // Bonn, Germany
	{ 51, -7 },   // Dublin, Ireland
	{ 28, -1 },   // El Menia, Algeria
	{ 67, -111 }, // Ellesmere, Canada
	{ 43, -107 }, // Glasgow, Montana
	{ 61, -48 },  // Godthab, Greenland
	{ 19, -157 }, // Honolulu, Hawaii
	{ 6, 5 },     // Ibadan, Nigeria
	{ -29, 26 },  // Johannesburg, South Africa
	{ 46, 92 },   // Kobdo, Mongolia
	{ -15, -63 }, // La Paz, Bolivia
	{ -35, -61 }, // La Plata, Argentina
	{ -9, -76 },  // Lima, Peru
	{ 38, -4 },   // Madrid, Spain
	{ -8, -51 },  // Manaus, Brazil
	{ 13, 120 },  // Manila, Phillipines
	{ -35, 143 }, // Melbourne, Australia
	{ 60, -161 }, // Nome, Alaska
	{ -7, 142 },  // Papua, New Guinea
	{ -32, 117 }, // Perth, West Australia
	{ 34, -114 }, // Phoenix, Arizona
	{ 18, -71 },  // Port-Au-Prince, Haiti
	{ 42, -121 }, // Portland, Oregon
	{ 61, -20 },  // Reykjavik, Iceland
	{ -22, -46 }, // Rio de Janeiro
	{ 27, -101 }, // San Antonio, Texas
	{ 34, 126 },  // Seoul, Korea
	{ 37, -87 },  // Saint Louis, Missouri
	{ 60, 30 },   // Saint Petersberg, Russia
	{ 56, 12 },   // Stockholm, Sweden
	{ 51, 105 },  // Svortalsk, Siberia
	{ 36, -96 }   // Tulsa, Oklahoma
};

const int16 GlobeGame::_targetSilo[kNumTargetSilos] = {
	14, 9, 1, 33, 6, 8, 34, 31, 38, 21
};

const short GlobeGame::_timeLimit[kNumTargetSilos] = {
	120, 110, 100, 90, 80, 70, 60, 50, 40, 30
};

const TimeValue GlobeGame::_siloName[kNumTargetSilos][2] = {
	{ kHonoluluIn, kHonoluluOut },
	{ kDublinIn, kDublinOut },
	{ kAddisAbabaIn, kAddisAbabaOut },
	{ kSanAntonioIn, kSanAntonioOut },
	{ kBangkokIn, kBangkokOut },
	{ kBonnIn, kBonnOut },
	{ kSeoulIn, kSeoulOut },
	{ kReykjavikIn, kReykjavikOut },
	{ kSvortalskIn, kSvortalskOut },
	{ kMadridIn, kMadridOut }
};

// From globe room models

static const GlobeGame::Point3D kCameraLocation = { 0.53f, 4.4f, -0.86f };
static const GlobeGame::Point3D kGlobeCenter = { -31.5f, 8.0f, 0.0f };
static const float kGlobeRadius = 8.25f;
static const int16 kDegreesPerLongSlice = 360 / kNumLongSlices;
static const int16 kDegreesPerLatSlice = 25;
static const int16 kLongOrigin = -95;

// Other constants.

static const float kTanFieldOfView = 0.7082373180482f;
static const float kPicturePlaneDistance = 10.0f; // Completely arbitrary.
static const int16 kLatError = 2;
static const int16 kLongError = 2;
static const TimeValue kGlobeMovieStartTime = 2 * 2 * kNumLongSlices * 600 / 15;

static const TimeValue kTimePerGlobeFrame = 40;

static const NotificationFlags kGlobeSplash1Finished = 1;
static const NotificationFlags kGlobeTimerExpired = kGlobeSplash1Finished << 1;
static const NotificationFlags kMaxDeactivatedFinished = kGlobeTimerExpired << 1;

static const NotificationFlags kGlobeNotificationFlags = kGlobeSplash1Finished |
													kGlobeTimerExpired |
													kMaxDeactivatedFinished;

static const int16 kSplash1End = 4;
static const int16 kSplash2End = 5;
static const int16 kSplash3Start = 8;
static const int16 kSplash3Stop = 9;
static const int16 kSplash4Start = 9;
static const int16 kSplash4Stop = 10;
static const int16 kNewLaunchSiloTime = 10;
static const int16 kSiloDeactivatedTime = 11;
static const int16 kMissileLaunchedTime = 12;
static const int16 kMaxDeactivatedStart = 13;
static const int16 kMaxDeactivatedStop = 23;

static const int16 kGamePlaying = 1;
static const int16 kGameOver = 2;

enum {
	kGameIntro,
	kPlayingRobotIntro,
	kPlayingStrikeAuthorized,
	kPlayingPrimaryTarget,
	kPlayingNewSilo1,
	kPlayingNewSilo2,
	kPlayingNewSilo3,
	kPlayingTime,
	kPlayingInstructions,
	kWaitingForPlayer,
	kSiloDeactivated,
	kRobotTaunting,
	kDelayingPlayer,
	kPlayerWon1,
	kPlayerWon2,
	kPlayerLost1
};

// TODO: Use ScummVM equivalent
static const float kPI = 3.1415926535f;

float degreesToRadians(float angle) {
	return (angle * kPI) / 180;
}

float radiansToDegrees(float angle) {
	return (angle * 180) / kPI;
}

GlobeGame::GlobeGame(Neighborhood *handler) : GameInteraction(kNoradGlobeGameInteractionID, handler),
		_monitorMovie(kGlobeMonitorID), _globeMovie(kGlobeMovieID), _upperNamesMovie(kGlobeUpperNamesID),
		_lowerNamesMovie(kGlobeLowerNamesID), _globeNotification(kNoradGlobeNotificationID, (PegasusEngine *)g_engine),
		_globeCircleLeft(kGlobeCircleLeftID), _globeCircleRight(kGlobeCircleRightID),
		_globeCircleUp(kGlobeCircleUpID), _globeCircleDown(kGlobeCircleDownID),
		_motionHighlightLeft(kMotionHiliteLeftID), _motionHighlightRight(kMotionHiliteRightID),
		_motionHighlightUp(kMotionHiliteUpID), _motionHighlightDown(kMotionHiliteDownID),
		_targetHighlightUpperLeft(kTargetHiliteUpperLeftID), _targetHighlightUpperRight(kTargetHiliteUpperRightID),
		_targetHighlightLowerLeft(kTargetHiliteLowerLeftID), _targetHighlightLowerRight(kTargetHiliteLowerRightID),
		_globeTracker(&_globeMovie, &_motionHighlightLeft, &_motionHighlightRight, &_motionHighlightUp,
				&_motionHighlightDown), _countdown(kGlobeCountdownID) {
	_neighborhoodNotification = handler->getNeighborhoodNotification();
}

void GlobeGame::openInteraction() {
	_monitorMovie.initFromMovieFile("Images/Norad Delta/N79 Left Monitor");
	_monitorMovie.moveElementTo(kGlobeMonitorLeft, kGlobeMonitorTop);
	_monitorMovie.setDisplayOrder(kGlobeMonitorLayer);
	_monitorMovie.startDisplaying();
	_monitorMovie.setSegment(0, kSplash1End * _monitorMovie.getScale());
	_monitorMovie.show();

	_monitorCallBack.setNotification(&_globeNotification);
	_monitorCallBack.initCallBack(&_monitorMovie, kCallBackAtExtremes);
	_monitorCallBack.setCallBackFlag(kGlobeSplash1Finished);
	_monitorCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	_upperNamesMovie.initFromMovieFile("Images/Norad Delta/Upper Names");
	_upperNamesMovie.moveElementTo(kGlobeUpperNamesLeft, kGlobeUpperNamesTop);
	_upperNamesMovie.setDisplayOrder(kGlobeUpperNamesLayer);
	_upperNamesMovie.startDisplaying();

	_lowerNamesMovie.initFromMovieFile("Images/Norad Delta/Lower Names");
	_lowerNamesMovie.moveElementTo(kGlobeLowerNamesLeft, kGlobeLowerNamesTop);
	_lowerNamesMovie.setDisplayOrder(kGlobeLowerNamesLayer);
	_lowerNamesMovie.startDisplaying();

	_globeMovie.initFromMovieFile("Images/Norad Delta/Spinning Globe");
	_globeMovie.moveElementTo(kGlobeLeft, kGlobeTop);
	_globeMovie.setDisplayOrder(kGlobeMovieLayer);
	_globeMovie.startDisplaying();
	_globeMovie.setTime(kGlobeMovieStartTime);
	_globeMovie.redrawMovieWorld();

	_globeCircleLeft.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kGlobeCircleLeftPICTID, true);
	_globeCircleLeft.moveElementTo(kGlobeCircleLeftLeft, kGlobeCircleLeftTop);
	_globeCircleLeft.setDisplayOrder(kGlobeCircleLayer);
	_globeCircleLeft.startDisplaying();

	_globeCircleRight.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kGlobeCircleRightPICTID, true);
	_globeCircleRight.moveElementTo(kGlobeCircleRightLeft, kGlobeCircleRightTop);
	_globeCircleRight.setDisplayOrder(kGlobeCircleLayer);
	_globeCircleRight.startDisplaying();

	_globeCircleUp.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kGlobeCircleUpPICTID, true);
	_globeCircleUp.moveElementTo(kGlobeCircleUpLeft, kGlobeCircleUpTop);
	_globeCircleUp.setDisplayOrder(kGlobeCircleLayer);
	_globeCircleUp.startDisplaying();

	_globeCircleDown.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kGlobeCircleDownPICTID, true);
	_globeCircleDown.moveElementTo(kGlobeCircleDownLeft, kGlobeCircleDownTop);
	_globeCircleDown.setDisplayOrder(kGlobeCircleLayer);
	_globeCircleDown.startDisplaying();

	_motionHighlightLeft.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kMotionHiliteLeftPICTID, true);
	_motionHighlightLeft.moveElementTo(kGlobeLeftMotionHiliteLeft, kGlobeLeftMotionHiliteTop);
	_motionHighlightLeft.setDisplayOrder(kGlobeHilitesLayer);
	_motionHighlightLeft.startDisplaying();

	_motionHighlightRight.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kMotionHiliteRightPICTID, true);
	_motionHighlightRight.moveElementTo(kGlobeRightMotionHiliteLeft, kGlobeRightMotionHiliteTop);
	_motionHighlightRight.setDisplayOrder(kGlobeCircleLayer);
	_motionHighlightRight.startDisplaying();

	_motionHighlightUp.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kMotionHiliteUpPICTID, true);
	_motionHighlightUp.moveElementTo(kGlobeUpMotionHiliteLeft, kGlobeUpMotionHiliteTop);
	_motionHighlightUp.setDisplayOrder(kGlobeHilitesLayer);
	_motionHighlightUp.startDisplaying();

	_motionHighlightDown.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kMotionHiliteDownPICTID, true);
	_motionHighlightDown.moveElementTo(kGlobeDownMotionHiliteLeft, kGlobeDownMotionHiliteTop);
	_motionHighlightDown.setDisplayOrder(kGlobeHilitesLayer);
	_motionHighlightDown.startDisplaying();

	_targetHighlightUpperLeft.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kTargetUpperLeftPICTID, true);
	_targetHighlightUpperLeft.moveElementTo(kGlobeUpperLeftHiliteLeft, kGlobeUpperLeftHiliteTop);
	_targetHighlightUpperLeft.setDisplayOrder(kGlobeHilitesLayer);
	_targetHighlightUpperLeft.startDisplaying();

	_targetHighlightUpperRight.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kTargetUpperRightPICTID, true);
	_targetHighlightUpperRight.moveElementTo(kGlobeUpperRightHiliteLeft, kGlobeUpperRightHiliteTop);
	_targetHighlightUpperRight.setDisplayOrder(kGlobeHilitesLayer);
	_targetHighlightUpperRight.startDisplaying();

	_targetHighlightLowerLeft.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kTargetLowerLeftPICTID, true);
	_targetHighlightLowerLeft.moveElementTo(kGlobeLowerLeftHiliteLeft, kGlobeLowerLeftHiliteTop);
	_targetHighlightLowerLeft.setDisplayOrder(kGlobeHilitesLayer);
	_targetHighlightLowerLeft.startDisplaying();

	_targetHighlightLowerRight.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kTargetLowerRightPICTID, true);
	_targetHighlightLowerRight.moveElementTo(kGlobeLowerRightHiliteLeft, kGlobeLowerRightHiliteTop);
	_targetHighlightLowerRight.setDisplayOrder(kGlobeHilitesLayer);
	_targetHighlightLowerRight.startDisplaying();

	_countdown.setDisplayOrder(kGlobeCountdownLayer);
	_countdown.moveElementTo(kGlobeCountdownLeft, kGlobeCountdownTop);
	_countdown.startDisplaying();
	_countdown.setCountdownTime(_timeLimit[0]);

	_countdownCallBack.setNotification(&_globeNotification);
	_countdownCallBack.initCallBack(&_countdown, kCallBackAtExtremes);
	_countdownCallBack.setCallBackFlag(kGlobeTimerExpired);
	_countdownCallBack.scheduleCallBack(kTriggerAtStart, 0, 0);

	_globeNotification.notifyMe(this, kGlobeNotificationFlags, kGlobeNotificationFlags);

	_gameState = kGameIntro;
	_currentSiloIndex = 0;
	_playedInstructions = false;

	_neighborhoodNotification->notifyMe(this, kDelayCompletedFlag | kSpotSoundCompletedFlag, kDelayCompletedFlag | kSpotSoundCompletedFlag);
}

void GlobeGame::initInteraction() {
	_monitorMovie.start();
	_monitorMovie.redrawMovieWorld();
}

void GlobeGame::closeInteraction() {
	_monitorMovie.stop();
	_monitorMovie.stopDisplaying();
	_monitorMovie.releaseMovie();
	_monitorCallBack.releaseCallBack();

	_globeMovie.stop();
	_globeMovie.stopDisplaying();
	_globeMovie.releaseMovie();
	_globeNotification.cancelNotification(this);

	_upperNamesMovie.stop();
	_upperNamesMovie.stopDisplaying();
	_upperNamesMovie.releaseMovie();

	_lowerNamesMovie.stop();
	_lowerNamesMovie.stopDisplaying();
	_lowerNamesMovie.releaseMovie();

	_countdown.hide();
	_countdown.stopDisplaying();
	_countdownCallBack.releaseCallBack();

	_globeCircleLeft.stopDisplaying();
	_globeCircleLeft.deallocateSurface();
	_globeCircleRight.stopDisplaying();
	_globeCircleRight.deallocateSurface();
	_globeCircleUp.stopDisplaying();
	_globeCircleUp.deallocateSurface();
	_globeCircleDown.stopDisplaying();
	_globeCircleDown.deallocateSurface();

	_motionHighlightLeft.stopDisplaying();
	_motionHighlightLeft.deallocateSurface();
	_motionHighlightRight.stopDisplaying();
	_motionHighlightRight.deallocateSurface();
	_motionHighlightUp.stopDisplaying();
	_motionHighlightUp.deallocateSurface();
	_motionHighlightDown.stopDisplaying();
	_motionHighlightDown.deallocateSurface();

	_targetHighlightUpperLeft.stopDisplaying();
	_targetHighlightUpperLeft.deallocateSurface();
	_targetHighlightUpperRight.stopDisplaying();
	_targetHighlightUpperRight.deallocateSurface();
	_targetHighlightLowerLeft.stopDisplaying();
	_targetHighlightLowerLeft.deallocateSurface();
	_targetHighlightLowerRight.stopDisplaying();
	_targetHighlightLowerRight.deallocateSurface();

	_neighborhoodNotification->cancelNotification(this);
}

void GlobeGame::receiveNotification(Notification *notification, const NotificationFlags flags) {
	TimeScale scale = _monitorMovie.getScale();

	if (notification == _neighborhoodNotification) {
		switch (_gameState) {
		case kPlayingRobotIntro:
			_monitorMovie.stop();
			_monitorMovie.setSegment(0, _monitorMovie.getDuration());
			_monitorMovie.setTime(kSplash2End * scale - 1);
			_monitorMovie.setFlags(0);

			_owner->requestDelay(1, 2, kFilterNoInput, 0);
			_owner->requestSpotSound(kStrikeAuthorizedIn, kStrikeAuthorizedOut,
					kFilterNoInput, kSpotSoundCompletedFlag);
			_gameState = kPlayingStrikeAuthorized;
			break;
		case kPlayingStrikeAuthorized:
			_monitorMovie.setSegment(kSplash3Start * scale, kSplash3Stop * scale);
			_monitorMovie.setTime(kSplash3Start * scale);
			_monitorMovie.redrawMovieWorld();

			_owner->requestDelay(1, 3, kFilterNoInput, 0);
			_owner->requestSpotSound(kPrimaryTargetIn, kPrimaryTargetOut, kFilterNoInput, 0);
			_owner->requestDelay(1, 5, kFilterNoInput, kDelayCompletedFlag);
			_monitorMovie.start();
			_gameState = kPlayingPrimaryTarget;
			break;
		case kPlayingPrimaryTarget:
			_monitorMovie.stop();
			_monitorMovie.setSegment(0, _monitorMovie.getDuration());
			_monitorMovie.setTime(kNewLaunchSiloTime * scale);
			_owner->requestSpotSound(kNewLaunchSiloIn, kNewLaunchSiloOut, kFilterNoInput,
					kSpotSoundCompletedFlag);
			_gameState = kPlayingNewSilo1;
			break;
		case kPlayingNewSilo1:
			_monitorMovie.stop();
			_monitorMovie.setSegment(0, _monitorMovie.getDuration());
			_owner->requestDelay(1, 3, kFilterNoInput, kDelayCompletedFlag);
			_gameState = kPlayingNewSilo2;
			break;
		case kPlayingNewSilo2:
			_upperNamesMovie.show();
			_upperNamesMovie.setTime(_currentSiloIndex * _upperNamesMovie.getScale());
			_upperNamesMovie.redrawMovieWorld();
			_monitorMovie.setTime(kSplash4Stop * scale - 1);
			_monitorMovie.redrawMovieWorld();
			_owner->requestSpotSound(_siloName[_currentSiloIndex][0], _siloName[_currentSiloIndex][1], kFilterNoInput, 0);
			_owner->requestDelay(1, 3, kFilterNoInput, 0);
			_owner->requestSpotSound(kLaunchToProceedIn, kLaunchToProceedOut, kFilterNoInput, 0);
			_owner->requestDelay(1, 5, kFilterNoInput, kDelayCompletedFlag);
			_gameState = kPlayingNewSilo3;
			break;
		case kPlayingNewSilo3:
			_countdown.stopCountdown();
			_countdown.setCountdownTime(_timeLimit[_currentSiloIndex]);
			_countdown.show();
			_gameState = kPlayingTime;

			if (_timeLimit[_currentSiloIndex] >= 120)
				_owner->requestSpotSound(kTwoMinutesIn, kTwoMinutesOut, kFilterNoInput, 0);
			else if (_timeLimit[_currentSiloIndex] >= 60)
				_owner->requestSpotSound(kOneMinuteIn, kOneMinuteOut, kFilterNoInput, 0);

			switch (_timeLimit[_currentSiloIndex] % 60) {
			case 0:
				_owner->requestDelay(1, 5, kFilterNoInput, kDelayCompletedFlag);
				break;
			case 10:
				_owner->requestDelay(1, 5, kFilterNoInput, 0);
				_owner->requestSpotSound(kTenSecondsIn, kTenSecondsOut, kFilterNoInput,
						kSpotSoundCompletedFlag);
				break;
			case 20:
				_owner->requestDelay(1, 5, kFilterNoInput, 0);
				_owner->requestSpotSound(kTwentySecondsIn, kTwentySecondsOut,
						kFilterNoInput, kSpotSoundCompletedFlag);
				break;
			case 30:
				_owner->requestDelay(1, 5, kFilterNoInput, 0);
				_owner->requestSpotSound(kThirtySecondsIn, kThirtySecondsOut,
						kFilterNoInput, kSpotSoundCompletedFlag);
				break;
			case 40:
				_owner->requestDelay(1, 5, kFilterNoInput, 0);
				_owner->requestSpotSound(kFortySecondsIn, kFortySecondsOut,
						kFilterNoInput, kSpotSoundCompletedFlag);
				break;
			case 50:
				_owner->requestDelay(1, 5, kFilterNoInput, 0);
				_owner->requestSpotSound(kFiftySecondsIn, kFiftySecondsOut,
						kFilterNoInput, kSpotSoundCompletedFlag);
				break;
			}
		case kPlayingTime:
			_gameState = kPlayingInstructions;
			_globeMovie.show();
			_globeCircleLeft.show();
			_globeCircleRight.show();
			_globeCircleUp.show();
			_globeCircleDown.show();

			if (_playedInstructions) {
				receiveNotification(notification, flags);
			} else {
				_owner->requestSpotSound(kToDeactivateIn, kToDeactivateOut, kFilterNoInput,
						kSpotSoundCompletedFlag);
				_playedInstructions = true;
			}
			break;
		case kPlayingInstructions:
			_gameState = kWaitingForPlayer;
			_countdown.startCountdown();
			break;
		case kSiloDeactivated:
			_gameState = kRobotTaunting;

			switch (_currentSiloIndex) {
			case 3:
				_owner->requestSpotSound(kYouCannotPossiblyIn, kYouCannotPossiblyOut,
						kFilterNoInput, kSpotSoundCompletedFlag);
				break;
			case 5:
				_owner->requestSpotSound(kYouWillFailIn, kYouWillFailOut, kFilterNoInput,
						kSpotSoundCompletedFlag);
				break;
			case 7:
				_owner->requestSpotSound(kGiveUpHumanIn, kGiveUpHumanOut, kFilterNoInput,
						kSpotSoundCompletedFlag);
				break;
			case 9:
				_owner->requestSpotSound(kYouAreRunningIn, kYouAreRunningOut,
						kFilterNoInput, kSpotSoundCompletedFlag);
				break;
			default:
				_owner->requestSpotSound(kNewLaunchSiloIn, kNewLaunchSiloOut,
						kFilterNoInput, kSpotSoundCompletedFlag);
				_monitorMovie.setTime(kNewLaunchSiloTime * scale);
				_monitorMovie.redrawMovieWorld();
				_gameState = kPlayingNewSilo1;
				break;
			}
			break;
		case kRobotTaunting:
			_owner->requestDelay(1, 1, kFilterNoInput, 0);
			_owner->requestSpotSound(kNewLaunchSiloIn, kNewLaunchSiloOut, kFilterNoInput, kSpotSoundCompletedFlag);
			_monitorMovie.setTime(kNewLaunchSiloTime * scale);
			_monitorMovie.redrawMovieWorld();
			_gameState = kPlayingNewSilo1;
			break;
		case kDelayingPlayer:
			_gameState = kWaitingForPlayer;
			break;
		case kPlayerLost1:
			_owner->recallToTSAFailure();
			break;
		case kPlayerWon2:
			((NoradDelta *)_owner)->finishedGlobeGame();
			_owner->requestDeleteCurrentInteraction();
			break;
		default:
			break;
		}
	} else if (notification == &_globeNotification) {
		ExtraTable::Entry entry;

		switch (flags) {
		case kGlobeSplash1Finished:
			_owner->getExtraEntry(kN79BrightView, entry);
			_monitorMovie.stop();
			_monitorMovie.setSegment(kSplash1End * scale, kSplash2End * scale);
			_monitorMovie.setFlags(kLoopTimeBase);
			_monitorMovie.start();
			_owner->showViewFrame(entry.movieStart);
			_owner->requestSpotSound(kIJustBrokeIn, kIJustBrokeOut, kFilterNoInput, 0);
			_owner->requestDelay(1, 2, kFilterNoInput, kDelayCompletedFlag);
			_gameState = kPlayingRobotIntro;
			break;
		case kGlobeTimerExpired:
			// Missile launched, player loses.
			_owner->requestSpotSound(kMissileLaunchedIn, kMissileLaunchedOut, kFilterNoInput, kSpotSoundCompletedFlag);
			_gameState = kPlayerLost1;
			break;
		case kMaxDeactivatedFinished:
			_monitorMovie.stop();
			_monitorMovie.setSegment(0, _monitorMovie.getDuration());
			_owner->requestDelay(1, 2, kFilterNoInput, 0);
			_owner->requestSpotSound(kTheOnlyGoodHumanIn, kTheOnlyGoodHumanOut, kFilterNoInput, 0);
			_owner->requestDelay(1, 2, kFilterNoInput, kDelayCompletedFlag);
			_gameState = kPlayerWon2;
			break;
		default:
			break;
		}
	}
}

// Prevent the player from getting up until the game is over.

void GlobeGame::handleInput(const Input &input, const Hotspot *cursorSpot) {
	Common::Point where;
	input.getInputLocation(where);
	Hotspot *spot = g_allHotspots.findHotspot(where);

	if (((PegasusEngine *)g_engine)->_cursor->isVisible() && spot != 0 &&
			spot->getObjectID() == kNorad79SiloAreaSpotID && findClickedSilo(input) != -1) {
		_targetHighlightUpperLeft.show();
		_targetHighlightUpperRight.show();
		_targetHighlightLowerLeft.show();
		_targetHighlightLowerRight.show();
	} else {
		_targetHighlightUpperLeft.hide();
		_targetHighlightUpperRight.hide();
		_targetHighlightLowerLeft.hide();
		_targetHighlightLowerRight.hide();
	}

	// Interrupt certain inputs to prevent player from switching modes.
	InputHandler::handleInput(input, cursorSpot);
}

int16 GlobeGame::findClickedSilo(const Input &input) {
	Common::Point screenPoint;
	input.getInputLocation(screenPoint);
	screenPoint.x -= kNavAreaLeft;
	screenPoint.y -= kNavAreaTop;

	Line3D ray;
	screenPointTo3DPoint(screenPoint.x, screenPoint.y, ray.pt2);
	ray.pt1 = kCameraLocation;

	Point3D globePoint;
	if (lineHitsGlobe(ray, globePoint)) {
		int16 latOrigin, longOrigin, latitude, longitude;
		globeMovieFrameToOrigin(_globeMovie.getTime() / kTimePerGlobeFrame, latOrigin, longOrigin);
		globePointToLatLong(globePoint, latOrigin, longOrigin, latitude, longitude);

		for (int16 i = 0; i < kNumAllSilos; i++)
			if (_siloCoords[i][0] >= latitude - kLatError && _siloCoords[i][0] <= latitude + kLatError &&
					_siloCoords[i][1] >= longitude - kLongError && _siloCoords[i][1] <= longitude + kLongError)
				return i;
	}

	return -1;
}

void GlobeGame::spinGlobe(const Input &input, const Hotspot *spot, GlobeTrackDirection trackDirection) {
	_globeTracker.setTrackParameters(spot, trackDirection);
	_globeTracker.startTracking(input);
}

void GlobeGame::clickGlobe(const Input &input) {
	int16 newSilo = findClickedSilo(input);

	if (newSilo != -1) {
		_targetHighlightUpperLeft.hide();
		_targetHighlightUpperRight.hide();
		_targetHighlightLowerLeft.hide();
		_targetHighlightLowerRight.hide();
		_lowerNamesMovie.show();
		_lowerNamesMovie.setTime(newSilo * _lowerNamesMovie.getScale());
		_lowerNamesMovie.redrawMovieWorld();
		_owner->requestSpotSound(kSiloBeepIn, kSiloBeepOut, kFilterNoInput, 0);

		if (newSilo == _targetSilo[_currentSiloIndex]) {
			_currentSiloIndex++;
			_countdown.stopCountdown();
			_owner->requestSpotSound(kSiloDeactivatedIn, kSiloDeactivatedOut, kFilterNoInput, 0);

			if (_currentSiloIndex == kNumTargetSilos) {
				// Player won.
				_owner->requestDelay(1, 2, kFilterNoInput, 0);
				_upperNamesMovie.hide();
				_lowerNamesMovie.hide();
				_countdown.hide();
				_monitorMovie.setSegment(kMaxDeactivatedStart * _monitorMovie.getScale(),
						kMaxDeactivatedStop * _monitorMovie.getScale());
				_monitorMovie.setTime(kMaxDeactivatedStart * _monitorMovie.getScale());
				_monitorCallBack.setCallBackFlag(kMaxDeactivatedFinished);
				_monitorCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
				_monitorMovie.start();
				_owner->requestSpotSound(kMaximumDeactivationIn, kMaximumDeactivationOut,
						kFilterNoInput, kSpotSoundCompletedFlag);
				_gameState = kPlayerWon1;
			} else {
				_owner->requestDelay(2, 1, kFilterNoInput, kDelayCompletedFlag);
				_upperNamesMovie.hide();
				_lowerNamesMovie.hide();
				_countdown.hide();
				_monitorMovie.setTime(kSiloDeactivatedTime * _monitorMovie.getScale());
				_monitorMovie.redrawMovieWorld();
				_gameState = kSiloDeactivated;
			}
		} else {
			_owner->requestDelay(5, 1, kFilterNoInput, kDelayCompletedFlag);
			_gameState = kDelayingPlayer;
			// Play "incorrect" sound?
		}
	}
}

void GlobeGame::clickInHotspot(const Input &input, const Hotspot *spot) {
	switch (spot->getObjectID()) {
	case kNorad79SpinLeftSpotID:
		spinGlobe(input, spot, kTrackLeft);
		break;
	case kNorad79SpinRightSpotID:
		spinGlobe(input, spot, kTrackRight);
		break;
	case kNorad79SpinUpSpotID:
		spinGlobe(input, spot, kTrackUp);
		break;
	case kNorad79SpinDownSpotID:
		spinGlobe(input, spot, kTrackDown);
		break;
	case kNorad79SiloAreaSpotID:
		clickGlobe(input);
		break;
	default:
		GameInteraction::clickInHotspot(input, spot);
		break;
	}
}

void GlobeGame::activateHotspots() {
	GameInteraction::activateHotspots();

	switch (_gameState) {
	case kWaitingForPlayer:
		g_allHotspots.deactivateOneHotspot(kNorad79WestOutSpotID);
		g_allHotspots.activateOneHotspot(kNorad79SpinLeftSpotID);
		g_allHotspots.activateOneHotspot(kNorad79SpinRightSpotID);
		g_allHotspots.activateOneHotspot(kNorad79SpinUpSpotID);
		g_allHotspots.activateOneHotspot(kNorad79SpinDownSpotID);
		g_allHotspots.activateOneHotspot(kNorad79SiloAreaSpotID);
		break;
	default:
		g_allHotspots.deactivateOneHotspot(kNorad79WestOutSpotID);
		break;
	}
}

void GlobeGame::globeMovieFrameToOrigin(int16 frameNum, int16 &latOrigin, int16 &longOrigin) {
	latOrigin = kDegreesPerLatSlice * 2 - (frameNum / (kNumLongSlices * 2)) * kDegreesPerLatSlice;
	frameNum %= kNumLongSlices * 2;

	if (frameNum >= kNumLongSlices)
		longOrigin = kLongOrigin + (kNumLongSlices * 2 - 1 - frameNum) * kDegreesPerLongSlice;
	else
		longOrigin = kLongOrigin + frameNum * kDegreesPerLongSlice;

	if (longOrigin > 180)
		longOrigin -= 360;
}

void GlobeGame::globePointToLatLong(const GlobeGame::Point3D &pt, int16 latOrigin, int16 longOrigin,
		int16 &latitude, int16 &longitude) {
	Point3D scratch = pt;

	// Translate globe center to origin.
	scratch.x -= kGlobeCenter.x;
	scratch.y -= kGlobeCenter.y;
	scratch.z -= kGlobeCenter.z;

	// Rotate around z axis latOrigin degrees to bring equator parallel with XZ plane
	float theta = degreesToRadians(latOrigin);
	float s = sin(theta);
	float c = cos(theta);
	float x = scratch.x * c - scratch.y * s;
	float y = scratch.y * c + scratch.x * s;
	scratch.x = x;
	scratch.y = y;

	// Calculate latitude
	latitude = (int16)radiansToDegrees(asin(scratch.y / kGlobeRadius));

	// Rotate around y axis longOrigin degrees to bring longitude 0 to positive X axis
	theta = degreesToRadians(longOrigin);
	s = sin(theta);
	c = cos(theta);
	x = scratch.x * c - scratch.z * s;
	float z = scratch.z * c + scratch.x * s;
	scratch.x = x;
	scratch.z = z;

	// Calculate longitude
	longitude = (int16)radiansToDegrees(acos(scratch.x / sqrt(scratch.x * scratch.x + scratch.z * scratch.z)));

	if (scratch.z < 0)
		longitude = -longitude;
}

// h, v in [0, 511][0, 255]
// Looking down negative x axis.
void GlobeGame::screenPointTo3DPoint(int16 h, int16 v, GlobeGame::Point3D &pt) {
	pt.x = kCameraLocation.x - kPicturePlaneDistance;
	pt.y = kCameraLocation.y + (128 - v) * kPicturePlaneDistance * kTanFieldOfView / 256;
	pt.z = kCameraLocation.z + (h - 256) * kPicturePlaneDistance * kTanFieldOfView / 256;
}

// Fundamentals of Three-Dimensional Graphics, by Alan Watt
// pp. 163-164
bool GlobeGame::lineHitsGlobe(const GlobeGame::Line3D &line, GlobeGame::Point3D &pt) {
	float i = line.pt2.x - line.pt1.x;
	float j = line.pt2.y - line.pt1.y;
	float k = line.pt2.z - line.pt1.z;
	float a = i * i + j * j + k * k;
	float b = 2 * i * (line.pt1.x - kGlobeCenter.x) + 2 * j * (line.pt1.y - kGlobeCenter.y) +
			2 * k * (line.pt1.z - kGlobeCenter.z);
	float c = kGlobeCenter.x * kGlobeCenter.x + kGlobeCenter.y * kGlobeCenter.y +
			kGlobeCenter.z * kGlobeCenter.z + line.pt1.x * line.pt1.x + line.pt1.y * line.pt1.y +
			line.pt1.z * line.pt1.z + -2 * (kGlobeCenter.x * line.pt1.x + kGlobeCenter.y * line.pt1.y +
			kGlobeCenter.z * line.pt1.z) - kGlobeRadius * kGlobeRadius;

	// Solve quadratic equation of a, b, c.
	float t = b * b - 4 * a * c;

	if (t >= 0.0f) {
		// Return smaller root, which corresponds to closest intersection point.
		t = (-b - sqrt(t)) / (2 * a);
		pt.x = i * t + line.pt1.x;
		pt.y = j * t + line.pt1.y;
		pt.z = k * t + line.pt1.z;
		return true;
	}

	return false;
}

bool GlobeGame::canSolve() {
	return	_gameState != kPlayerWon1 && _gameState != kPlayerWon2 && _gameState != kPlayerLost1;
}

void GlobeGame::doSolve() {
	_owner->requestDelay(1, 2, kFilterNoInput, 0);
	_upperNamesMovie.hide();
	_lowerNamesMovie.hide();
	_countdown.hide();
	_monitorMovie.setSegment(kMaxDeactivatedStart * _monitorMovie.getScale(), kMaxDeactivatedStop * _monitorMovie.getScale());
	_monitorMovie.setTime(kMaxDeactivatedStart * _monitorMovie.getScale());
	_monitorCallBack.setCallBackFlag(kMaxDeactivatedFinished);
	_monitorCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	_monitorMovie.start();
	_owner->requestSpotSound(kMaximumDeactivationIn, kMaximumDeactivationOut, kFilterNoInput, kSpotSoundCompletedFlag);
	_gameState = kPlayerWon1;
}

} // End of namespace Pegasus
