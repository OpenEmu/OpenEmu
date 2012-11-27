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
#include "pegasus/neighborhood/norad/subplatform.h"
#include "pegasus/neighborhood/norad/alpha/noradalpha.h"

namespace Pegasus {

// As usual, times here are in seconds.

static const TimeValue kNormalSplashStart = 0;
static const TimeValue kNormalSplashStop = 5;

static const TimeValue kPrepSubStart = 5;
static const TimeValue kPrepSubStop = 15;

static const TimeValue kPrepIncompleteStart = 15;
static const TimeValue kPrepIncompleteStop = 19;

static const TimeValue kDamagedStart = 19;
static const TimeValue kDamagedStop = 28;

static const NotificationFlags kNormalSplashFinished = 1;
static const NotificationFlags kPrepSubFinished = kNormalSplashFinished << 1;
static const NotificationFlags kPrepIncompleteFinished = kPrepSubFinished << 1;
static const NotificationFlags kDamagedFinished = kPrepIncompleteFinished << 1;

static const NotificationFlags kPlatformNotificationFlags = kNormalSplashFinished |
														kPrepSubFinished |
														kPrepIncompleteFinished |
														kDamagedFinished;

static const uint16 kSubPreppedBit = (1 << 0);
static const uint16 kWaitingForPlayerBit = (1 << 1);

SubPlatform::SubPlatform(Neighborhood *handler) : GameInteraction(kNoradSubPlatformInteractionID, handler),
		_platformMovie(kPlatformMonitorID), _platformNotification(kNoradSubPlatformNotificationID, (PegasusEngine *)g_engine) {
	_neighborhoodNotification = handler->getNeighborhoodNotification();
}

void SubPlatform::openInteraction() {
	_stateBits = 0;

	// TODO: These next two lines seem unused?
	if (GameState.getNoradSubPrepState() == kSubPrepped)
		_stateBits |= kSubPreppedBit;

	_stateBits |= kWaitingForPlayerBit;
	_platformMovie.initFromMovieFile("Images/Norad Alpha/Platform Monitor Movie");
	_platformMovie.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());
	_platformMovie.moveElementTo(kNoradPlatformLeft, kNoradPlatformTop);
	_platformScale = _platformMovie.getScale();
	_platformMovie.setDisplayOrder(kPlatformOrder);
	_platformMovie.startDisplaying();
	_platformCallBack.setNotification(&_platformNotification);
	_platformCallBack.initCallBack(&_platformMovie, kCallBackAtExtremes);

	_platformNotification.notifyMe(this, kPlatformNotificationFlags, kPlatformNotificationFlags);
}

void SubPlatform::initInteraction() {
	_neighborhoodNotification->notifyMe(this, kExtraCompletedFlag, kExtraCompletedFlag);
}

void SubPlatform::closeInteraction() {
	_platformNotification.cancelNotification(this);
	_platformCallBack.releaseCallBack();
	_neighborhoodNotification->cancelNotification(this);
}

void SubPlatform::setSoundFXLevel(const uint16 fxLevel) {
	_platformMovie.setVolume(fxLevel);
}

void SubPlatform::receiveNotification(Notification *notification, const NotificationFlags flags) {
	FaderMoveSpec loop1Spec, loop2Spec;
	ExtraTable::Entry entry;

	Norad *owner = (Norad *)getOwner();

	if (notification == &_platformNotification) {
		switch (flags) {
		case kNormalSplashFinished:
			_platformMovie.stop();
			switch (GameState.getNoradSubPrepState()) {
			case kSubNotPrepped:
				_platformMovie.setSegment(kPrepIncompleteStart * _platformScale, kPrepIncompleteStop * _platformScale);
				_platformMovie.setTime(kPrepIncompleteStart * _platformScale);
				_platformCallBack.setCallBackFlag(kPrepIncompleteFinished);
				_platformCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
				_platformMovie.start();
				break;
			case kSubPrepped:
				_platformMovie.setSegment(kPrepSubStart * _platformScale, kPrepSubStop * _platformScale);
				_platformMovie.setTime(kPrepSubStart * _platformScale);
				_platformCallBack.setCallBackFlag(kPrepSubFinished);
				_platformCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
				owner->startExtraSequence(kNorad19PrepSub, 0, kFilterNoInput);
				_platformMovie.start();
				break;
			case kSubDamaged:
				// Shouldn't happen.
				break;
			}
			break;
		case kPrepSubFinished:
			_platformMovie.stop();
			_platformMovie.stopDisplaying();

			owner->getExtraEntry(kNorad19ExitToSub, entry);

			loop1Spec.makeTwoKnotFaderSpec(kNoradAlphaMovieScale, 0, kNoradWarningVolume,
					entry.movieEnd - entry.movieStart, 0);
			loop1Spec.insertFaderKnot(4560, kNoradWarningVolume);
			loop1Spec.insertFaderKnot(5080, 0);

			loop2Spec.makeTwoKnotFaderSpec(kNoradAlphaMovieScale, 0, kNoradSuckWindVolume,
					entry.movieEnd - entry.movieStart, 0);
			loop1Spec.insertFaderKnot(4560, kNoradSuckWindVolume);
			loop1Spec.insertFaderKnot(5080, 0);

			owner->startExtraSequence(kNorad19ExitToSub, kExtraCompletedFlag, kFilterNoInput);

			owner->startLoop1Fader(loop1Spec);
			owner->startLoop2Fader(loop2Spec);
			break;
		case kPrepIncompleteFinished:
			((NoradAlpha *)owner)->setSubPrepFailed(true);
			g_AIArea->checkMiddleArea();
			// Fall through...
		case kDamagedFinished:
			_platformMovie.stop();
			_platformMovie.hide();
			_stateBits |= kWaitingForPlayerBit;
			allowInput(true);
			break;
		}
	} else if (notification == _neighborhoodNotification) {
		allowInput(true);
		((PegasusEngine *)g_engine)->jumpToNewEnvironment(kNoradSubChaseID, kNoRoomID, kNoDirection);
		GameState.setScoringEnteredSub(true);
	}
}

void SubPlatform::activateHotspots() {
	if (_stateBits & kWaitingForPlayerBit)
		g_allHotspots.activateOneHotspot(kNorad19ActivateMonitorSpotID);

	GameInteraction::activateHotspots();
}

void SubPlatform::clickInHotspot(const Input &input, const Hotspot *spot) {
	if (spot->getObjectID() == kNorad19ActivateMonitorSpotID) {
		if (GameState.getNoradSubPrepState() == kSubDamaged) {
			_platformMovie.setSegment(kDamagedStart * _platformScale, kDamagedStop * _platformScale);
			_platformMovie.setTime(kDamagedStart * _platformScale);
			_platformCallBack.setCallBackFlag(kDamagedFinished);
		} else {
			_platformMovie.setSegment(kNormalSplashStart * _platformScale, kNormalSplashStop * _platformScale);
			_platformMovie.setTime(kNormalSplashStart * _platformScale);
			_platformCallBack.setCallBackFlag(kNormalSplashFinished);
		}

		_platformCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

		_platformMovie.show();
		_platformMovie.start();
		_platformMovie.redrawMovieWorld();

		_stateBits &= ~kWaitingForPlayerBit;

		allowInput(false);
	} else {
		GameInteraction::clickInHotspot(input, spot);
	}
}

} // End of namespace Pegasus
