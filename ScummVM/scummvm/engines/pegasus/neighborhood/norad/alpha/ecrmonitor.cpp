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

#include "pegasus/pegasus.h"
#include "pegasus/neighborhood/norad/constants.h"
#include "pegasus/neighborhood/norad/norad.h"
#include "pegasus/neighborhood/norad/alpha/ecrmonitor.h"

namespace Pegasus {

static const NotificationFlags kECRSection1FinishedFlag = 1;
static const NotificationFlags kECRPanFinishedFlag = kECRSection1FinishedFlag << 1;
static const NotificationFlags kECRSection2FinishedFlag = kECRPanFinishedFlag << 1;
static const NotificationFlags kECRNotificationFlags = kECRSection1FinishedFlag |
													kECRPanFinishedFlag |
													kECRSection2FinishedFlag;

static const TimeValue kSection1Start = 0;
static const TimeValue kSection1Stop = 25;
static const TimeValue kPanStart = 0;
static const TimeValue kPanStop = 20;
static const TimeValue kSection2Start = 26;
static const TimeValue kSection2Stop = 1000;

// Seems to be a good value for a 20 second pan.
static const CoordType kPanPixelsPerFrame = 8;

// Interesting times are in seconds.
static const TimeValue s_ECRInterestingTimes[] = {
	0, 1, 2, 10, 25, 26, 56, 64, 72, 80, 88, 94, 102, 108, 116, 999
};

// Index into s_ECRInterestingTimes of interesting time before security pan.
static const int kBeforePanTime = 3;

// Index into s_ECRInterestingTimes of interesting time after security pan.
static const int kAfterPanTime = 5;

NoradAlphaECRMonitor::NoradAlphaECRMonitor(Neighborhood *nextHandler) : GameInteraction(kNoradECRMonitorInteractionID, nextHandler),
		_ecrSlideShowNotification(kNoradECRNotificationID, (PegasusEngine *)g_engine), _ecrMovie(kECRSlideShowMovieID),
		_ecrPan(kECRPanID) {
}

void NoradAlphaECRMonitor::receiveNotification(Notification *, const NotificationFlags flags) {
	if (flags & kECRSection1FinishedFlag)
		ecrSection1Finished();
	else if (flags & kECRPanFinishedFlag)
		ecrPanFinished();
	else if (flags & kECRSection2FinishedFlag)
		ecrSection2Finished();
}

int NoradAlphaECRMonitor::findCurrentInterestingTime() {
	TimeValue time = _ecrMovie.getTime();
	TimeScale scale = _ecrMovie.getScale();

	for (int i = ARRAYSIZE(s_ECRInterestingTimes) - 1; i >= 0; i--)
		if (time >= s_ECRInterestingTimes[i] * scale)
			return i;

	return 0;
}

void NoradAlphaECRMonitor::skipToNextInterestingTime() {
	if (_ecrMovie.isRunning()) {
		int interestingTime = findCurrentInterestingTime();
		_ecrMovie.setTime(s_ECRInterestingTimes[interestingTime + 1] * _ecrMovie.getScale());
		_ecrMovie.redrawMovieWorld();
	} else if (_ecrPan.isRunning()) {
		_ecrPanCallBack.cancelCallBack();
		ecrPanFinished();
	}
}

void NoradAlphaECRMonitor::skipToPreviousInterestingTime() {
	if (_ecrPan.isRunning()) {
		_ecrPan.stop();
		_ecrPan.stopDisplaying();
		_ecrPanCallBack.cancelCallBack();

		_ecrMovieCallBack.setCallBackFlag(kECRSection1FinishedFlag);
		_ecrMovieCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

		TimeScale scale = _ecrMovie.getScale();
		_ecrMovie.setSegment(kSection1Start * scale, kSection1Stop * scale + 1);
		_ecrMovie.setTime(s_ECRInterestingTimes[kBeforePanTime] * scale);
		_ecrMovie.start();
	} else {
		int interestingTime = findCurrentInterestingTime();

		if (interestingTime == kAfterPanTime) {
			_ecrMovieCallBack.cancelCallBack();
			TimeScale scale = _ecrMovie.getScale();
			_ecrMovie.setSegment(kSection1Start * scale, kSection1Stop * scale + 1);
			_ecrMovie.setTime(kSection1Stop * scale);
			ecrSection1Finished();
		} else if (interestingTime == 0) {
			_ecrMovie.setTime(kSection1Start * _ecrMovie.getScale());
			_ecrMovie.redrawMovieWorld();
		} else {
			_ecrMovie.setTime(s_ECRInterestingTimes[interestingTime - 1] * _ecrMovie.getScale());
			_ecrMovie.redrawMovieWorld();
		}
	}
}

void NoradAlphaECRMonitor::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (isInteracting()) {
		if (input.rightButtonDown())
			skipToNextInterestingTime();
		else if (input.leftButtonDown())
			skipToPreviousInterestingTime();
		else
			InputHandler::handleInput(input, cursorSpot);
	} else {
		InputHandler::handleInput(input, cursorSpot);
	}
}

void NoradAlphaECRMonitor::ecrSection1Finished() {
	_ecrMovie.stop();
	_ecrPanCallBack.setNotification(&_ecrSlideShowNotification);
	_ecrPanCallBack.initCallBack(&_ecrPan, kCallBackAtExtremes);
	_ecrPanCallBack.setCallBackFlag(kECRPanFinishedFlag);
	_ecrSlideShowNotification.notifyMe(this, kECRNotificationFlags, kECRNotificationFlags);
	_ecrPanCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	_ecrPan.startDisplaying();
	_ecrPan.show();

	TimeScale scale = _ecrPan.getScale();
	_ecrPan.setSegment(kPanStart * scale, kPanStop * scale);
	_ecrPan.setTime(0);
	_ecrPan.start();
}

void NoradAlphaECRMonitor::ecrPanFinished() {
	_ecrPan.stop();
	_ecrPan.stopDisplaying();
	_ecrMovieCallBack.setCallBackFlag(kECRSection2FinishedFlag);
	_ecrMovieCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	TimeScale scale = _ecrMovie.getScale();
	_ecrMovie.setSegment(kSection2Start * scale, kSection2Stop * scale);
	_ecrMovie.start();
}

void NoradAlphaECRMonitor::ecrSection2Finished() {
	_ecrMovie.stop();
	_ecrMovie.stopDisplaying();
}

void NoradAlphaECRMonitor::openInteraction() {
	// Initialize the security pan.
	_ecrPan.initFromMovieFile("Images/Norad Alpha/Security Pan.pano");
	_ecrPan.initMaskFromPICTFile("Images/Norad Alpha/Security Pan Mask");
	_ecrPan.setBounds(Common::Rect(kECRPanLeft, kECRPanTop, kECRPanRight, kECRPanBottom));
	_ecrPan.setDisplayOrder(kECRPanOrder);
	_ecrPan.setScale(15); // 15 fps.

	// Begin the lame ECR slide show.
	// clone2727: I didn't say it :P
	_ecrMovie.initFromMovieFile("Images/Norad Alpha/ECR Monitor Movie");

	_ecrMovieCallBack.setNotification(&_ecrSlideShowNotification);
	_ecrMovieCallBack.initCallBack(&_ecrMovie, kCallBackAtExtremes);
	_ecrMovieCallBack.setCallBackFlag(kECRSection1FinishedFlag);

	_ecrSlideShowNotification.notifyMe(this, kECRNotificationFlags, kECRNotificationFlags);
	_ecrMovieCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	_ecrMovie.moveElementTo(kECRSlideShowLeft, kECRSlideShowTop);
	_ecrMovie.setDisplayOrder(kECRMonitorOrder);
	_ecrMovie.startDisplaying();
	_ecrMovie.show();
	_ecrMovie.redrawMovieWorld();

	TimeScale scale = _ecrMovie.getScale();
	_ecrMovie.setSegment(kSection1Start * scale, kSection1Stop * scale + 1);

	_ecrMovie.start();
}

void NoradAlphaECRMonitor::closeInteraction() {
	_ecrMovieCallBack.releaseCallBack();
	_ecrMovie.stop();
	_ecrMovie.stopDisplaying();
	_ecrMovie.releaseMovie();
	_ecrMovieCallBack.releaseCallBack();

	_ecrPanCallBack.releaseCallBack();
	_ecrPan.stop();
	_ecrPan.stopDisplaying();
	_ecrPan.releasePanorama();
	_ecrPanCallBack.releaseCallBack();
}

} // End of namespace Pegasus
