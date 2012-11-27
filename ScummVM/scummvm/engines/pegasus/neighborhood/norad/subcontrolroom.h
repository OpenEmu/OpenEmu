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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_SUBCONTROLROOM_H
#define PEGASUS_NEIGHBORHOOD_NORAD_SUBCONTROLROOM_H

#include "pegasus/interaction.h"
#include "pegasus/notification.h"

namespace Pegasus {

static const uint32 kClawAtA = 0;
static const uint32 kClawAtB = 1;
static const uint32 kClawAtC = 2;
static const uint32 kClawAtD = 3;

static const int kNumClawButtons = 7;

class Norad;

class SubControlRoom : public GameInteraction, public NotificationReceiver {
public:
	SubControlRoom(Neighborhood *);
	virtual ~SubControlRoom() {}

	void playAgainstRobot();

	virtual void setSoundFXLevel(const uint16);

	bool canSolve();
	void doSolve();

protected:
	virtual void openInteraction();
	virtual void initInteraction();
	virtual void closeInteraction();

	virtual void activateHotspots();
	virtual void clickInHotspot(const Input &, const Hotspot *);

	virtual void receiveNotification(Notification *, const NotificationFlags);

	void robotKillsPlayer(const uint32, Neighborhood *);
	InputBits getInputFilter();

	int findActionIndex(HotSpotID);
	void dispatchClawAction(const int);
	void performActionImmediately(const int, const uint32, Neighborhood *);

	void hideEverything();
	void showButtons();
	void hideButtons();

	void updateGreenBall();
	void moveGreenBallToA();
	void moveGreenBallToB();
	void moveGreenBallToC();
	void moveGreenBallToD();

	void setControlMonitorToTime(const TimeValue, const int, const bool);
	void playControlMonitorSection(const TimeValue, const TimeValue, const NotificationFlags,
			const int, const bool);

	void updateClawMonitor();
	void setClawMonitorToTime(const TimeValue);
	void playClawMonitorSection(const TimeValue, const TimeValue, const NotificationFlags,
			const int, const bool);

	Movie _subControlMovie;
	TimeScale _subControlScale;
	Notification _subControlNotification;
	NotificationCallBack _subControlCallBack;
	Movie _clawMonitorMovie;
	NotificationCallBack _clawMonitorCallBack;
	int _gameState;
	uint32 _clawStartPosition;
	uint32 _clawPosition;
	uint32 _clawNextPosition;
	const uint32 *_clawExtraIDs;

	int _currentAction;
	int _nextAction;

	Sprite *_buttons[kNumClawButtons];
	Sprite _pinchButton;
	Sprite _downButton;
	Sprite _rightButton;
	Sprite _leftButton;
	Sprite _upButton;
	Sprite _ccwButton;
	Sprite _cwButton;

	Sprite _greenBall;
	TimeBase _greenBallTimer;
	Notification _greenBallNotification;
	NotificationCallBack _greenBallCallBack;

	HotSpotID _outSpotID;
	HotSpotID _prepSpotID;
	HotSpotID _clawControlSpotID;
	HotSpotID _clawButtonSpotIDs[kNumClawButtons];

	Notification *_neighborhoodNotification;

	bool _playingAgainstRobot;
	int _robotState;
};

} // End of namespace Pegasus

#endif
