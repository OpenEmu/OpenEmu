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
#include "pegasus/neighborhood/norad/constants.h"
#include "pegasus/neighborhood/norad/norad.h"
#include "pegasus/neighborhood/norad/subcontrolroom.h"
#include "pegasus/neighborhood/norad/delta/noraddelta.h"

namespace Pegasus {

// Right Monitor times

static const TimeValue kAlphaClawSplashStart = 0;
static const TimeValue kAlphaClawSplashStop = 4000;

static const TimeValue kDeltaClawSplashStart = 4000;
static const TimeValue kDeltaClawSplashStop = 8000;

static const TimeValue kClawAtATime = 8000;
static const TimeValue kClawAtAPinchedTime = 8600;
static const TimeValue kClawAtATurnedTime = 9200;
static const TimeValue kClawAtAWithRobotPinchedTime = 9800;

static const TimeValue kClawAtBTime = 10400;
static const TimeValue kClawAtBPinchedTime = 11000;
static const TimeValue kClawAtBTurnedTime = 11600;
static const TimeValue kClawAtBWithRobotTime = 12200;
static const TimeValue kClawAtBWithRobotPinchedTime = 12800;

static const TimeValue kClawAtCTime = 13400;
static const TimeValue kClawAtCPinchedTime = 14000;
static const TimeValue kClawAtCTurnedTime = 14600;

static const TimeValue kClawAtDTime = 15200;
static const TimeValue kClawAtDPinchedTime = 15800;
static const TimeValue kClawAtDTurnedTime = 16400;

static const TimeValue kAToBStart = 17000;
static const TimeValue kAToBStop = 18680;
static const TimeValue kAPinchStart = 18680;
static const TimeValue kAPinchStop = 20200;
static const TimeValue kACCWStart = 20200;
static const TimeValue kACCWStop = 21600;
static const TimeValue kACWStart = 21600;
static const TimeValue kACWStop = 23000;

static const TimeValue kBToAStart = 23000;
static const TimeValue kBToAStop = 24680;
static const TimeValue kBToCStart = 24680;
static const TimeValue kBToCStop = 26520;
static const TimeValue kBToDStart = 26520;
static const TimeValue kBToDStop = 28320;
static const TimeValue kBPinchStart = 28320;
static const TimeValue kBPinchStop = 29680;
static const TimeValue kBCCWStart = 29680;
static const TimeValue kBCCWStop = 31200;
static const TimeValue kBCWStart = 31200;
static const TimeValue kBCWStop = 32720;

static const TimeValue kCToBStart = 32720;
static const TimeValue kCToBStop = 34560;
static const TimeValue kCPinchStart = 34560;
static const TimeValue kCPinchStop = 36400;
static const TimeValue kCCCWStart = 36400;
static const TimeValue kCCCWStop = 37840;
static const TimeValue kCCWStart = 37840;
static const TimeValue kCCWStop = 39280;

static const TimeValue kDToBStart = 39280;
static const TimeValue kDToBStop = 41080;
static const TimeValue kDPinchStart = 41080;
static const TimeValue kDPinchStop = 42600;
static const TimeValue kDCCWStart = 42600;
static const TimeValue kDCCWStop = 44000;
static const TimeValue kDCWStart = 44000;
static const TimeValue kDCWStop = 45400;

static const TimeValue kRobotApproachStart = 45400;
static const TimeValue kRobotApproachStop = 56800;

static const TimeValue kCToBWithRobotStart = 56800;
static const TimeValue kCToBWithRobotStop = 58600;

static const TimeValue kBPinchWithRobotStart = 58600;
static const TimeValue kBPinchWithRobotStop = 60400;
static const TimeValue kBToAWithRobotStart = 60400;
static const TimeValue kBToAWithRobotStop = 62240;

// As usual, times here are in seconds.

// Left monitor times.

static const TimeValue kAlphaSplashStart = 0;
static const TimeValue kAlphaSplashStop = 2;

static const TimeValue kMainMenuTime = 2;
static const TimeValue kLaunchPrepRolloverTime = 3;
static const TimeValue kLaunchPrepHighlightStart = 4;
static const TimeValue kLaunchPrepHighlightStop = 5;
static const TimeValue kClawControlRolloverTime = 5;
static const TimeValue kClawControlHighlightStart = 6;
static const TimeValue kClawControlHighlightStop = 7;

static const TimeValue kAlphaLaunchPrepStart = 7;
static const TimeValue kAlphaLaunchPrepStop = 17;

static const TimeValue kClawMenuStart = 17;
static const TimeValue kClawMenuStop = 18;

static const TimeValue kClawMenuTime = 18;

static const TimeValue kDeltaSplashStart = 19;
static const TimeValue kDeltaSplashStop = 21;

static const TimeValue kDeltaLaunchPrepStart = 21;
static const TimeValue kDeltaLaunchPrepStop = 30;

// Right monitor times.

static const NotificationFlags kAlphaSplashFinished = 1;
static const NotificationFlags kAlphaPrepFinished = kAlphaSplashFinished << 1;
static const NotificationFlags kPrepHighlightFinished = kAlphaPrepFinished << 1;
static const NotificationFlags kClawHighlightFinished = kPrepHighlightFinished << 1;
static const NotificationFlags kClawMenuFinished = kClawHighlightFinished << 1;
static const NotificationFlags kDeltaSplashFinished = kClawMenuFinished << 1;
static const NotificationFlags kDeltaPrepFinished = kDeltaSplashFinished << 1;

static const NotificationFlags kSubControlNotificationFlags = kAlphaSplashFinished |
														kAlphaPrepFinished |
														kPrepHighlightFinished |
														kClawHighlightFinished |
														kClawMenuFinished |
														kDeltaSplashFinished |
														kDeltaPrepFinished;

static const NotificationFlags kOneSecondOfMoveFinished = 1;

static const NotificationFlags kGreenBallNotificationFlags = kOneSecondOfMoveFinished;

enum {
	kButtonDimFrame,
	kButtonActiveFrame,
	kButtonHighlightedFrame
};

enum {
	kAlphaSplash,
	kAlphaMainMenu,
	kDeltaSplash,
	kDeltaMainMenu,
	kClawMenu,
	kPlayingHighlight,
	kPuttingClawAway
};

// The owning neighborhood must provide an array of longs which hold the various
// extra IDs for moving the claw around. In addition, the owner must tell the sub
// control room interaction what position the claw starts out in (which is also the
// position the claw must be in before leaving).

// Standard array indices:
enum {
	kClawFromAToBIndex,
	kClawALoopIndex,
	kClawAPinchIndex,
	kClawACounterclockwiseIndex,
	kClawAClockwiseIndex,
	kClawFromBToAIndex,
	kClawFromBToCIndex,
	kClawFromBToDIndex,
	kClawBLoopIndex,
	kClawBPinchIndex,
	kClawBCounterclockwiseIndex,
	kClawBClockwiseIndex,
	kClawFromCToBIndex,
	kClawCLoopIndex,
	kClawCPinchIndex,
	kClawCCounterclockwiseIndex,
	kClawCClockwiseIndex,
	kClawFromDToBIndex,
	kClawDLoopIndex,
	kClawDPinchIndex,
	kClawDCounterclockwiseIndex,
	kClawDClockwiseIndex
};

// Action indices for s_clawStateTable:
// Can also be used as indices into _buttons (except for kNoActionIndex and kLoopActionIndex).
enum {
	kNoActionIndex = -1,
	kPinchActionIndex = 0,
	kMoveDownActionIndex,
	kMoveRightActionIndex,
	kMoveLeftActionIndex,
	kMoveUpActionIndex,
	kCCWActionIndex,
	kCWActionIndex,
	kLoopActionIndex
};

/*
	_currentAction and _nextAction:

	At any time, _currentAction contains an action index (defined above). The current
	action index is what the claw is doing right now. If the player presses a button
	before the current action completes, _nextAction saves the new action and input
	is disabled. This has the effect of implementing a queue of commands for the claw
	that can save at most one extra command.

	The general strategy for using _currentAction and _nextAction are:
	--	If the player presses a claw button and _currentAction is kNoActionIndex,
		do the command immediately and set _currentAction accordingly.
	--	If the player presses a claw button and _currentAction is not kNoActionIndex,
		save the appropriate action index in _nextAction.
	--	When a command animation completes, set _nextAction to kNoActionIndex, then
		check if _nextAction has a command waiting in it. If so, play the appriate
		animation, copy _nextAction into _currentAction and set _nextAction to
		kNoActionIndex.
	--	If the player tries to get up, disable input (including all claw buttons) until
		the player rises. Then, if the claw is in its original position, play the
		animation of the player rising.
	--	If the claw needs to be put back, play the first move required to put the
		claw back by setting _currentAction and playing the appropriate animation.
		Leave _nextAction alone. When the animation, completes, check to see if the
		claw is in its original position or not. If so, complete the player rising
		sequence by playing the rising animation. If not, repeat this whole step.

	Using this general strategy allows the use of a single function,
	DispatchClawAction, which can both cause the claw to perform a command and saving
	the next command in _nextAction.
*/

// Array indexed by [claw position] [action]
// array yields an index into the neighborhood's extra id table for claw animation or -1.
static const int s_clawStateTable[4][8] = {
	{
		kClawAPinchIndex,
		kNoActionIndex,
		kNoActionIndex,
		kClawFromAToBIndex,
		kNoActionIndex,
		kClawACounterclockwiseIndex,
		kClawAClockwiseIndex,
		kClawALoopIndex
	},
	{
		kClawBPinchIndex,
		kNoActionIndex,
		kClawFromBToAIndex,
		kClawFromBToDIndex,
		kClawFromBToCIndex,
		kClawBCounterclockwiseIndex,
		kClawBClockwiseIndex,
		kClawBLoopIndex
	},
	{
		kClawCPinchIndex,
		kClawFromCToBIndex,
		kNoActionIndex,
		kNoActionIndex,
		kNoActionIndex,
		kClawCCounterclockwiseIndex,
		kClawCClockwiseIndex,
		kClawCLoopIndex
	},
	{
		kClawDPinchIndex,
		kNoActionIndex,
		kClawFromDToBIndex,
		kNoActionIndex,
		kNoActionIndex,
		kClawDCounterclockwiseIndex,
		kClawDClockwiseIndex,
		kClawDLoopIndex
	}
};

// Move directions for s_clawMovieTable:
enum {
	kMoveClawDown,
	kMoveClawRight,
	kMoveClawLeft,
	kMoveClawUp
};

static const int kClawNowhere = -1;

// Array indexed by [claw position] [move direction]
// array yields new claw position or -1.
static const int s_clawMovieTable[4][4] = {
	{
		kClawNowhere,
		kClawNowhere,
		kClawAtB,
		kClawNowhere
	},
	{
		kClawNowhere,
		kClawAtA,
		kClawAtD,
		kClawAtC
	},
	{
		kClawAtB,
		kClawNowhere,
		kClawNowhere,
		kClawNowhere
	},
	{
		kClawNowhere,
		kClawAtB,
		kClawNowhere,
		kClawNowhere
	}
};

// Indexed by claw action index, claw position, plus 0 for start, 1 for stop.
// (Never indexed with kLoopActionIndex.)
static const TimeValue s_clawMonitorTable[7][4][2] = {
	{
		{ kAPinchStart, kAPinchStop },
		{ kBPinchStart, kBPinchStop },
		{ kCPinchStart, kCPinchStop },
		{ kDPinchStart, kDPinchStop }
	},
	{
		{ 0xffffffff, 0xffffffff },
		{ 0xffffffff, 0xffffffff },
		{ kCToBStart, kCToBStop },
		{ 0xffffffff, 0xffffffff }
	},
	{
		{ 0xffffffff, 0xffffffff },
		{ kBToAStart, kBToAStop },
		{ 0xffffffff, 0xffffffff },
		{ kDToBStart, kDToBStop }
	},
	{
		{ kAToBStart, kAToBStop },
		{ kBToDStart, kBToDStop },
		{ 0xffffffff, 0xffffffff },
		{ 0xffffffff, 0xffffffff }
	},
	{
		{ 0xffffffff, 0xffffffff },
		{ kBToCStart, kBToCStop },
		{ 0xffffffff, 0xffffffff },
		{ 0xffffffff, 0xffffffff }
	},
	{
		{ kACCWStart, kACCWStop },
		{ kBCCWStart, kBCCWStop },
		{ kCCCWStart, kCCCWStop },
		{ kDCCWStart, kDCCWStop }
	},
	{
		{ kACWStart, kACWStop },
		{ kBCWStart, kBCWStop },
		{ kCCWStart, kCCWStop },
		{ kDCWStart, kDCWStop }
	}
};

// Frame indices for the green ball sprite.
enum {
	kGreenBallAtA,
	kGreenBallAtAWithClaw,
	kGreenBallAtAWithClawAndRobot,
	kGreenBallAtB,
	kGreenBallAtBWithClaw,
	kGreenBallAtBWithClawAndRobot,
	kGreenBallAtCArmAtA,
	kGreenBallAtCArmAtB,
	kGreenBallAtCArmAtD,
	kGreenBallAtCWithClaw,
	kGreenBallAtD,
	kGreenBallAtDWithClaw,
	kNumClawGreenBalls
};

// State constants for _robotState.
enum {
	kNoRobot,
	kRobotApproaching,
	kPunchingOnce,
	kPunchingTwice,
	kPunchingThrice,
	kCarriedToDoor,
	kPlayerWon,
	kRobotWon
};

// Sub Control Room button PICTs:
static const ResIDType kSubControlButtonBaseID = 500;
static const ResIDType kClawMonitorGreenBallBaseID = 600;

// Constructor
SubControlRoom::SubControlRoom(Neighborhood *handler) : GameInteraction(kNoradSubControlRoomInteractionID, handler),
		_subControlMovie(kSubControlMonitorID), _subControlNotification(kSubControlNotificationID, (PegasusEngine *)g_engine),
		_clawMonitorMovie(kClawMonitorID), _pinchButton(kSubControlPinchID), _downButton(kSubControlDownID),
		_rightButton(kSubControlRightID), _leftButton(kSubControlLeftID), _upButton(kSubControlUpID),
		_ccwButton(kSubControlCCWID), _cwButton(kSubControlCWID), _greenBall(kClawMonitorGreenBallID),
		_greenBallNotification(kNoradGreenBallNotificationID, (PegasusEngine *)g_engine) {
	_neighborhoodNotification = handler->getNeighborhoodNotification();
	_playingAgainstRobot = false;
	_robotState = kNoRobot;
}

void SubControlRoom::playAgainstRobot() {
	_playingAgainstRobot = true;
}

void SubControlRoom::openInteraction() {
	_currentAction = kNoActionIndex;
	_nextAction = kNoActionIndex;

	Norad *owner = (Norad *)getOwner();
	owner->getClawInfo(_outSpotID, _prepSpotID, _clawControlSpotID, _clawButtonSpotIDs[0],
			_clawButtonSpotIDs[1], _clawButtonSpotIDs[2], _clawButtonSpotIDs[3],
			_clawButtonSpotIDs[4], _clawButtonSpotIDs[5], _clawButtonSpotIDs[6],
			_clawStartPosition, _clawExtraIDs);

	_clawPosition = _clawStartPosition;
	_clawNextPosition = _clawPosition;
	_subControlMovie.initFromMovieFile("Images/Norad Alpha/N22 Left Monitor Movie");
	_subControlMovie.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());
	_subControlMovie.moveElementTo(kNoradSubControlLeft, kNoradSubControlTop);
	_subControlScale = _subControlMovie.getScale();
	_subControlMovie.setDisplayOrder(kSubControlOrder);
	_subControlMovie.startDisplaying();
	_subControlCallBack.setNotification(&_subControlNotification);
	_subControlCallBack.initCallBack(&_subControlMovie, kCallBackAtExtremes);

	_clawMonitorMovie.initFromMovieFile("Images/Norad Alpha/N22:N60 Right Monitor");
	_clawMonitorMovie.moveElementTo(kNoradClawMonitorLeft, kNoradClawMonitorTop);
	_clawMonitorMovie.setDisplayOrder(kClawMonitorOrder);
	_clawMonitorMovie.startDisplaying();
	_clawMonitorCallBack.setNotification(&_subControlNotification);
	_clawMonitorCallBack.initCallBack(&_clawMonitorMovie, kCallBackAtExtremes);

	_subControlNotification.notifyMe(this, kSubControlNotificationFlags, kSubControlNotificationFlags);

	_neighborhoodNotification->notifyMe(this, kExtraCompletedFlag, kExtraCompletedFlag);

	_buttons[0] = &_pinchButton;
	_buttons[1] = &_downButton;
	_buttons[2] = &_rightButton;
	_buttons[3] = &_leftButton;
	_buttons[4] = &_upButton;
	_buttons[5] = &_ccwButton;
	_buttons[6] = &_cwButton;

	_pinchButton.setDisplayOrder(kSubControlPinchOrder);
	_downButton.setDisplayOrder(kSubControlDownOrder);
	_rightButton.setDisplayOrder(kSubControlRightOrder);
	_leftButton.setDisplayOrder(kSubControlLeftOrder);
	_upButton.setDisplayOrder(kSubControlUpOrder);
	_ccwButton.setDisplayOrder(kSubControlCCWOrder);
	_cwButton.setDisplayOrder(kSubControlCWOrder);

	for (int i = 0; i < kNumClawButtons; i++) {
		SpriteFrame *frame = new SpriteFrame();
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kSubControlButtonBaseID + i * 3, true);
		_buttons[i]->addFrame(frame, 0, 0);

		frame = new SpriteFrame();
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kSubControlButtonBaseID + i * 3 + 1, true);
		_buttons[i]->addFrame(frame, 0, 0);

		frame = new SpriteFrame();
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kSubControlButtonBaseID + i * 3 + 2, true);
		_buttons[i]->addFrame(frame, 0, 0);

		_buttons[i]->setCurrentFrameIndex(0);
		_buttons[i]->startDisplaying();
	}

	_pinchButton.moveElementTo(kNoradSubControlPinchLeft, kNoradSubControlPinchTop);
	_downButton.moveElementTo(kNoradSubControlDownLeft, kNoradSubControlDownTop);
	_rightButton.moveElementTo(kNoradSubControlRightLeft, kNoradSubControlRightTop);
	_leftButton.moveElementTo(kNoradSubControlLeftLeft, kNoradSubControlLeftTop);
	_upButton.moveElementTo(kNoradSubControlUpLeft, kNoradSubControlUpTop);
	_ccwButton.moveElementTo(kNoradSubControlCCWLeft, kNoradSubControlCCWTop);
	_cwButton.moveElementTo(kNoradSubControlCWLeft, kNoradSubControlCWTop);

	_greenBall.setDisplayOrder(kClawMonitorGreenBallOrder);

	for (int i = 0; i < kNumClawGreenBalls; i++) {
		SpriteFrame *frame = new SpriteFrame();
		frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kClawMonitorGreenBallBaseID + i);
		_greenBall.addFrame(frame, 0, 0);
	}

	_greenBall.setCurrentFrameIndex(0);
	_greenBall.startDisplaying();

	_greenBallTimer.setScale(owner->getNavMovie()->getScale());
	_greenBallCallBack.setNotification(&_greenBallNotification);
	_greenBallCallBack.initCallBack(&_greenBallTimer, kCallBackAtExtremes);
	_greenBallCallBack.setCallBackFlag(kOneSecondOfMoveFinished);
	_greenBallNotification.notifyMe(this, kGreenBallNotificationFlags, kGreenBallNotificationFlags);

	_subControlMovie.show();
	_clawMonitorMovie.show();
}

void SubControlRoom::initInteraction() {
	if (GameState.getNoradSubPrepState() == kSubDamaged) {
		playControlMonitorSection(kDeltaSplashStart * _subControlScale, kDeltaSplashStop * _subControlScale,
				0, kDeltaSplash, false);
		playClawMonitorSection(kDeltaClawSplashStart, kDeltaClawSplashStop, kDeltaSplashFinished, _gameState, false);
	} else {
		playControlMonitorSection(kAlphaSplashStart * _subControlScale, kAlphaSplashStop * _subControlScale,
				0, kAlphaSplash, false);
		playClawMonitorSection(kAlphaClawSplashStart, kAlphaClawSplashStop, kAlphaSplashFinished, _gameState, false);
	}

	_subControlMovie.redrawMovieWorld();
	_clawMonitorMovie.redrawMovieWorld();
}

void SubControlRoom::closeInteraction() {
	_subControlNotification.cancelNotification(this);
	_subControlCallBack.releaseCallBack();
	_greenBallNotification.cancelNotification(this);
	_greenBallCallBack.releaseCallBack();
	_neighborhoodNotification->cancelNotification(this);
}

void SubControlRoom::setSoundFXLevel(const uint16 fxLevel) {
	_subControlMovie.setVolume(fxLevel);
}

void SubControlRoom::receiveNotification(Notification *notification, const NotificationFlags flags) {
	Norad *owner = (Norad *)getOwner();

	if (notification == &_subControlNotification) {
		switch (flags) {
		case kAlphaSplashFinished:
			setControlMonitorToTime(kMainMenuTime * _subControlScale, kAlphaMainMenu, true);
			break;
		case kPrepHighlightFinished:
			if (GameState.getNoradSubPrepState() == kSubDamaged)
				playControlMonitorSection(kDeltaLaunchPrepStart * _subControlScale,
						kDeltaLaunchPrepStop * _subControlScale, kDeltaPrepFinished, _gameState, false);
			else
				playControlMonitorSection(kAlphaLaunchPrepStart * _subControlScale,
						kAlphaLaunchPrepStop * _subControlScale, kAlphaPrepFinished, _gameState, false);
			break;
		case kAlphaPrepFinished:
			GameState.setNoradSubPrepState(kSubPrepped);
			GameState.setScoringPreppedSub(true);
			setControlMonitorToTime(kMainMenuTime * _subControlScale, kAlphaMainMenu, true);
			break;
		case kClawHighlightFinished:
			playControlMonitorSection(kClawMenuStart * _subControlScale, kClawMenuStop * _subControlScale,
					kClawMenuFinished, _gameState, false);
			break;
		case kClawMenuFinished:
			owner->playClawMonitorIntro();
			showButtons();
			setControlMonitorToTime(kClawMenuTime * _subControlScale, kClawMenu, true);

			if (!_playingAgainstRobot) {
				updateClawMonitor();
				owner->loopExtraSequence(_clawExtraIDs[s_clawStateTable[_clawPosition][kLoopActionIndex]]);
			}
			break;
		case kDeltaSplashFinished:
			setControlMonitorToTime(kMainMenuTime * _subControlScale, kDeltaMainMenu, true);

			if (_playingAgainstRobot) {
				_robotState = kRobotApproaching;
				playClawMonitorSection(kRobotApproachStart, kRobotApproachStop, 0, _gameState, true);
				owner->startExtraSequence(kN60RobotApproaches, kExtraCompletedFlag, kFilterAllInput);
			}
			break;
		case kDeltaPrepFinished:
			setControlMonitorToTime(kMainMenuTime * _subControlScale, kDeltaMainMenu, true);
			break;
		}
	} else if (notification == &_greenBallNotification) {
		if (_robotState == kRobotWon) {
			// We are using the green ball notification to hide stuff when the robot comes through
			// the glass.
			hideEverything();
		} else {
			// We are now midway through a move, time to update the claw's position and the green
			// ball.
			_clawPosition = _clawNextPosition;
			updateClawMonitor();
			updateGreenBall();
		}
	} else if (notification == _neighborhoodNotification) {
		_currentAction = kNoActionIndex;
		if (_playingAgainstRobot) {
			switch (_robotState) {
			case kRobotApproaching:
				if (_gameState == kClawMenu) {
					_robotState = kPunchingOnce;
					dispatchClawAction(kNoActionIndex);
				} else {
					robotKillsPlayer(kN60FirstMistake, owner);
				}
				break;
			case kPunchingOnce:
				if (_nextAction == kMoveDownActionIndex) {
					_robotState = kPunchingTwice;
					performActionImmediately(_nextAction, _clawExtraIDs[s_clawStateTable[_clawPosition][_nextAction]], owner);
				} else {
					robotKillsPlayer(kN60SecondMistake, owner);
				}
				break;
			case kPunchingTwice:
				if (_nextAction == kPinchActionIndex) {
					_robotState = kPunchingThrice;
					performActionImmediately(_nextAction, _clawExtraIDs[s_clawStateTable[_clawPosition][_nextAction]], owner);
				} else {
					robotKillsPlayer(kN60ThirdMistake, owner);
				}
				break;
			case kPunchingThrice:
				if (_nextAction == kMoveRightActionIndex) {
					_robotState = kCarriedToDoor;
					performActionImmediately(_nextAction, _clawExtraIDs[s_clawStateTable[_clawPosition][_nextAction]], owner);
				} else {
					robotKillsPlayer(kN60FourthMistake, owner);
				}
				break;
			case kCarriedToDoor:
				hideEverything();
				_robotState = kPlayerWon;
				owner->startExtraSequence(kN60PlayerFollowsRobotToDoor, kExtraCompletedFlag, kFilterAllInput);
				break;
			case kPlayerWon:
				((NoradDelta *)owner)->playerBeatRobotWithClaw();
				owner->requestDeleteCurrentInteraction();
				break;
			case kRobotWon:
				g_system->delayMillis(2 * 1000); // 120 ticks
				((PegasusEngine *)g_engine)->die(kDeathRobotSubControlRoom);
				break;
			}
		} else {
			if (_gameState == kPuttingClawAway && _nextAction == kNoActionIndex) {
				if (_clawPosition == _clawStartPosition) {
					Input scratch;
					GameInteraction::clickInHotspot(scratch, g_allHotspots.findHotspotByID(_outSpotID));
				} else {
					switch (_clawPosition) {
					case kClawAtA:
						dispatchClawAction(kMoveLeftActionIndex);
						break;
					case kClawAtB:
						if (_clawStartPosition == kClawAtD)             // Norad Alpha
							dispatchClawAction(kMoveLeftActionIndex);
						else if (_clawStartPosition == kClawAtC)        // Norad Delta
							dispatchClawAction(kMoveUpActionIndex);
						break;
					case kClawAtC:
						dispatchClawAction(kMoveDownActionIndex);
						break;
					case kClawAtD:
						dispatchClawAction(kMoveRightActionIndex);
						break;
					}
				}
			} else {
				dispatchClawAction(_nextAction);
			}
		}
	}
}

void SubControlRoom::hideEverything() {
	hideButtons();
	_subControlMovie.hide();
	_clawMonitorMovie.hide();
	_greenBall.hide();
}

void SubControlRoom::robotKillsPlayer(const uint32 extraID, Neighborhood *owner) {
	_robotState = kRobotWon;
	owner->startExtraSequence(extraID, kExtraCompletedFlag, kFilterAllInput);
	_greenBallTimer.stop();
	_greenBallTimer.setSegment(0, 32 * _greenBallTimer.getScale() / 15);
	_greenBallTimer.setTime(0);
	_greenBallCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	_greenBallTimer.start();
}

void SubControlRoom::activateHotspots() {
	if (_robotState == kRobotWon || _robotState == kPlayerWon)
		return;

	GameInteraction::activateHotspots();

	switch (_gameState) {
	case kAlphaMainMenu:
	case kDeltaMainMenu:
		g_allHotspots.activateOneHotspot(_prepSpotID);
		g_allHotspots.activateOneHotspot(_clawControlSpotID);
		break;
	case kClawMenu:
		// This could be called during a move, so use _clawNextPosition.
		if (_playingAgainstRobot) {
			g_allHotspots.deactivateOneHotspot(_outSpotID);
			if (_robotState != kRobotApproaching && _nextAction == kNoActionIndex)
				for (int i = 0; i < kNumClawButtons; i++)
					if (s_clawStateTable[_clawNextPosition][i] != kNoActionIndex)
						g_allHotspots.activateOneHotspot(_clawButtonSpotIDs[i]);
		} else if (_nextAction == kNoActionIndex) {
			for (int i = 0; i < kNumClawButtons; i++)
				if (s_clawStateTable[_clawNextPosition][i] != kNoActionIndex)
					g_allHotspots.activateOneHotspot(_clawButtonSpotIDs[i]);
		}
		break;
	default:
		break;
	}
}

void SubControlRoom::showButtons() {
	if (_playingAgainstRobot && _robotState == kRobotApproaching) {
		for (int i = 0; i < kNumClawButtons; i++) {
			_buttons[i]->show();
			_buttons[i]->setCurrentFrameIndex(kButtonDimFrame);
		}
	} else if (_nextAction != kNoActionIndex) {
		for (int i = 0; i < kNumClawButtons; i++) {
			_buttons[i]->show();
			if (i == _currentAction || i == _nextAction)
				_buttons[i]->setCurrentFrameIndex(kButtonHighlightedFrame);
			else
				_buttons[i]->setCurrentFrameIndex(kButtonDimFrame);
		}
	} else {
		for (int i = 0; i < kNumClawButtons; i++) {
			_buttons[i]->show();
			if (i == _currentAction)
				_buttons[i]->setCurrentFrameIndex(kButtonHighlightedFrame);
			else if (s_clawStateTable[_clawNextPosition][i] != kNoActionIndex &&
					_gameState != kPuttingClawAway) // this could be called during a move, so check _clawNextPosition
				_buttons[i]->setCurrentFrameIndex(kButtonActiveFrame);
			else
				_buttons[i]->setCurrentFrameIndex(kButtonDimFrame);
		}
	}
}

void SubControlRoom::hideButtons() {
	for (int i = 0; i < kNumClawButtons; i++)
		_buttons[i]->hide();
}

int SubControlRoom::findActionIndex(HotSpotID id) {
	for (int i = 0; i < kNumClawButtons; i++)
		if (id == _clawButtonSpotIDs[i])
			return i;

	return kNoActionIndex;
}

void SubControlRoom::clickInHotspot(const Input &input, const Hotspot *spot) {
	HotSpotID clickedID = spot->getObjectID();
	int actionIndex = findActionIndex(clickedID);

	if (actionIndex != kNoActionIndex) {
		dispatchClawAction(actionIndex);
	} else if (clickedID == _prepSpotID) {
		playControlMonitorSection(kLaunchPrepHighlightStart * _subControlScale,
				kLaunchPrepHighlightStop * _subControlScale,
				kPrepHighlightFinished, kPlayingHighlight, false);
	} else if (clickedID == _clawControlSpotID) {
		playControlMonitorSection(kClawControlHighlightStart * _subControlScale,
				kClawControlHighlightStop * _subControlScale,
				kClawHighlightFinished, kPlayingHighlight, false);
	} else if (clickedID == _outSpotID) {
		_gameState = kPuttingClawAway;

		if (_currentAction == kNoActionIndex) {
			if (_clawPosition == _clawStartPosition) {
				GameInteraction::clickInHotspot(input, spot);
			} else {
				switch (_clawPosition) {
				case kClawAtA:
					dispatchClawAction(kMoveLeftActionIndex);
					break;
				case kClawAtB:
					if (_clawStartPosition == kClawAtD)             // Norad Alpha
						dispatchClawAction(kMoveLeftActionIndex);
					else if (_clawStartPosition == kClawAtC)        // Norad Delta
						dispatchClawAction(kMoveUpActionIndex);
					break;
				case kClawAtC:
					dispatchClawAction(kMoveDownActionIndex);
					break;
				case kClawAtD:
					dispatchClawAction(kMoveRightActionIndex);
					break;
				}
			}
		}
	} else {
		GameInteraction::clickInHotspot(input, spot);
	}
}

void SubControlRoom::dispatchClawAction(const int newAction) {
	GameState.setScoringPlayedWithClaw(true);

	Neighborhood *owner = getOwner();

	if (newAction == kNoActionIndex) {
		_currentAction = kNoActionIndex;
		_nextAction = kNoActionIndex;
		showButtons();
		updateGreenBall();

		if (_playingAgainstRobot)
			owner->startExtraSequence(kN60ArmActivated, kExtraCompletedFlag, kFilterAllInput);
		else
			owner->loopExtraSequence(_clawExtraIDs[s_clawStateTable[_clawPosition][kLoopActionIndex]]);
	} else {
		if (_currentAction == kNoActionIndex) {
			if (_playingAgainstRobot) {
				_nextAction = newAction;
				showButtons();
				updateGreenBall();
			} else {
				performActionImmediately(newAction, _clawExtraIDs[s_clawStateTable[_clawPosition][newAction]], owner);
			}
		} else if (_nextAction == kNoActionIndex) {
			_nextAction = newAction;
			showButtons();
			updateGreenBall();
		}
	}
}

void SubControlRoom::performActionImmediately(const int action, const uint32 extraID, Neighborhood *owner) {
	_currentAction = action;
	_nextAction = kNoActionIndex;
	ExtraTable::Entry entry;

	switch (action) {
	case kMoveDownActionIndex:
	case kMoveRightActionIndex:
	case kMoveLeftActionIndex:
	case kMoveUpActionIndex:
		// Set up green ball callback.
		owner->getExtraEntry(extraID, entry);
		_greenBallTimer.stop();
		_greenBallTimer.setSegment(entry.movieStart, entry.movieStart + owner->getNavMovie()->getScale());
		_greenBallTimer.setTime(entry.movieStart);
		_greenBallCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
		// Start move.
		_greenBallTimer.start();
		break;
	}

	if (_playingAgainstRobot) {
		switch (_robotState) {
		case kPunchingTwice:
			owner->startExtraSequence(kN60ArmToPositionB, kExtraCompletedFlag, kFilterAllInput);
			break;
		case kPunchingThrice:
			owner->startExtraSequence(kN60ArmGrabsRobot, kExtraCompletedFlag, kFilterAllInput);
			break;
		case kCarriedToDoor:
			owner->startExtraSequence(kN60ArmCarriesRobotToPositionA, kExtraCompletedFlag, kFilterAllInput);
			break;
		}
	} else {
		owner->startExtraSequence(extraID, kExtraCompletedFlag, kFilterAllInput);
	}

	switch (action) {
	case kMoveDownActionIndex:
		_clawNextPosition = s_clawMovieTable[_clawPosition][kMoveClawDown];
		break;
	case kMoveRightActionIndex:
		_clawNextPosition = s_clawMovieTable[_clawPosition][kMoveClawRight];
		break;
	case kMoveLeftActionIndex:
		_clawNextPosition = s_clawMovieTable[_clawPosition][kMoveClawLeft];
		break;
	case kMoveUpActionIndex:
		_clawNextPosition = s_clawMovieTable[_clawPosition][kMoveClawUp];
		break;
	case kLoopActionIndex:
		// Do nothing.
		break;
	default:
		playClawMonitorSection(s_clawMonitorTable[action][_clawPosition][0],
				s_clawMonitorTable[action][_clawPosition][1], 0, _gameState, true);
		break;
	}

	showButtons();
	updateGreenBall();
}

void SubControlRoom::setControlMonitorToTime(const TimeValue newTime, const int newState, const bool shouldAllowInput) {
	_subControlMovie.stop();
	_subControlMovie.setSegment(0, _subControlMovie.getDuration());
	_subControlMovie.setTime(newTime);
	_subControlMovie.redrawMovieWorld();
	_gameState = newState;
	allowInput(shouldAllowInput);
}

void SubControlRoom::playControlMonitorSection(const TimeValue in, const TimeValue out, const NotificationFlags flags,
		const int newState, const bool shouldAllowInput) {
	_subControlMovie.stop();
	_subControlMovie.setSegment(in, out);
	_subControlMovie.setTime(in);

	if (flags != 0) {
		_subControlCallBack.setCallBackFlag(flags);
		_subControlCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	}

	_gameState = newState;
	allowInput(shouldAllowInput);
	_subControlMovie.start();
}

void SubControlRoom::updateClawMonitor() {
	switch (_clawPosition) {
	case kClawAtA:
		setClawMonitorToTime(kClawAtATime);
		break;
	case kClawAtB:
		setClawMonitorToTime(kClawAtBTime);
		break;
	case kClawAtC:
		setClawMonitorToTime(kClawAtCTime);
		break;
	case kClawAtD:
		setClawMonitorToTime(kClawAtDTime);
		break;
	}
}

void SubControlRoom::setClawMonitorToTime(const TimeValue newTime) {
	_clawMonitorMovie.stop();
	_clawMonitorMovie.setSegment(0, _clawMonitorMovie.getDuration());
	_clawMonitorMovie.setTime(newTime);
	_clawMonitorMovie.redrawMovieWorld();
}

void SubControlRoom::playClawMonitorSection(const TimeValue in, const TimeValue out, const NotificationFlags flags,
		const int newState, const bool shouldAllowInput) {
	_clawMonitorMovie.stop();
	_clawMonitorMovie.setSegment(in, out);
	_clawMonitorMovie.setTime(in);

	if (flags != 0) {
		_clawMonitorCallBack.setCallBackFlag(flags);
		_clawMonitorCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	}

	_gameState = newState;
	allowInput(shouldAllowInput);
	_clawMonitorMovie.start();
}

void SubControlRoom::updateGreenBall() {
	switch (_currentAction) {
	case kMoveDownActionIndex:
		switch (_nextAction) {
		case kMoveRightActionIndex:
			moveGreenBallToA();
			break;
		case kMoveLeftActionIndex:
			moveGreenBallToD();
			break;
		case kMoveUpActionIndex:
			moveGreenBallToC();
			break;
		default:
			moveGreenBallToB();
			break;
		}
		break;
	case kMoveRightActionIndex:
		if (_clawNextPosition == kClawAtA) {
			switch (_nextAction) {
			case kMoveLeftActionIndex:
				moveGreenBallToB();
				break;
			default:
				moveGreenBallToA();
				break;
			}
		} else {
			switch (_nextAction) {
			case kMoveRightActionIndex:
				moveGreenBallToA();
				break;
			case kMoveLeftActionIndex:
				moveGreenBallToD();
				break;
			case kMoveUpActionIndex:
				moveGreenBallToC();
				break;
			default:
				moveGreenBallToB();
				break;
			}
		}
		break;
	case kMoveLeftActionIndex:
		if (_clawNextPosition == kClawAtB) {
			switch (_nextAction) {
			case kMoveRightActionIndex:
				moveGreenBallToA();
				break;
			case kMoveLeftActionIndex:
				moveGreenBallToD();
				break;
			case kMoveUpActionIndex:
				moveGreenBallToC();
				break;
			default:
				moveGreenBallToB();
				break;
			}
		} else {
			switch (_nextAction) {
			case kMoveRightActionIndex:
				moveGreenBallToB();
				break;
			default:
				moveGreenBallToD();
				break;
			}
		}
		break;
	case kMoveUpActionIndex:
		switch (_nextAction) {
		case kMoveDownActionIndex:
			moveGreenBallToB();
			break;
		default:
			moveGreenBallToC();
			break;
		}
		break;
	default:
		switch (_nextAction) {
		case kMoveDownActionIndex:
			moveGreenBallToB();
			break;
		case kMoveRightActionIndex:
			if (_clawPosition == kClawAtB)
				moveGreenBallToA();
			else
				moveGreenBallToB();
			break;
		case kMoveLeftActionIndex:
			if (_clawPosition == kClawAtB)
				moveGreenBallToD();
			else
				moveGreenBallToB();
			break;
		case kMoveUpActionIndex:
			moveGreenBallToC();
			break;
		default:
			_greenBall.hide();
			break;
		}
		break;
	}
}

void SubControlRoom::moveGreenBallToA() {
	if (_clawPosition == kClawAtA) {
		if (_playingAgainstRobot)
			_greenBall.setCurrentFrameIndex(kGreenBallAtAWithClawAndRobot);
		else
			_greenBall.setCurrentFrameIndex(kGreenBallAtAWithClaw);
	} else {
		_greenBall.setCurrentFrameIndex(kGreenBallAtA);
	}

	_greenBall.moveElementTo(kNoradGreenBallAtALeft, kNoradGreenBallAtATop);
	_greenBall.show();
}

void SubControlRoom::moveGreenBallToB() {
	if (_clawPosition == kClawAtB) {
		if (_playingAgainstRobot)
			_greenBall.setCurrentFrameIndex(kGreenBallAtBWithClawAndRobot);
		else
			_greenBall.setCurrentFrameIndex(kGreenBallAtBWithClaw);
	} else {
		_greenBall.setCurrentFrameIndex(kGreenBallAtB);
	}

	_greenBall.moveElementTo(kNoradGreenBallAtBLeft, kNoradGreenBallAtBTop);
	_greenBall.show();
}

void SubControlRoom::moveGreenBallToC() {
	switch (_clawPosition) {
	case kClawAtA:
		_greenBall.setCurrentFrameIndex(kGreenBallAtCArmAtA);
		break;
	case kClawAtB:
		_greenBall.setCurrentFrameIndex(kGreenBallAtCArmAtB);
		break;
	case kClawAtC:
		_greenBall.setCurrentFrameIndex(kGreenBallAtCWithClaw);
		break;
	case kClawAtD:
		_greenBall.setCurrentFrameIndex(kGreenBallAtCArmAtD);
		break;
	}

	_greenBall.moveElementTo(kNoradGreenBallAtCLeft, kNoradGreenBallAtCTop);
	_greenBall.show();
}

void SubControlRoom::moveGreenBallToD() {
	if (_clawPosition == kClawAtD)
		_greenBall.setCurrentFrameIndex(kGreenBallAtDWithClaw);
	else
		_greenBall.setCurrentFrameIndex(kGreenBallAtD);

	_greenBall.moveElementTo(kNoradGreenBallAtDLeft, kNoradGreenBallAtDTop);
	_greenBall.show();
}

bool SubControlRoom::canSolve() {
	return _playingAgainstRobot && _robotState < kCarriedToDoor;
}

void SubControlRoom::doSolve() {
	_robotState = kCarriedToDoor;
	hideEverything();
	getOwner()->startExtraSequence(kN60ArmGrabsRobot, kExtraCompletedFlag, kFilterAllInput);
}

InputBits SubControlRoom::getInputFilter() {
	if (_playingAgainstRobot)
		return GameInteraction::getInputFilter() & ~kFilterDownButtonAny;

	return GameInteraction::getInputFilter();
}

} // End of namespace Pegasus
