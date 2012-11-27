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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_PRESSUREDOOR_H
#define PEGASUS_NEIGHBORHOOD_NORAD_PRESSUREDOOR_H

#include "pegasus/interaction.h"
#include "pegasus/movie.h"
#include "pegasus/notification.h"
#include "pegasus/neighborhood/norad/pressuretracker.h"

namespace Pegasus {

static const short kNormalSubRoomPressure = 2;

class PressureDoor : public GameInteraction, public NotificationReceiver {
public:
	PressureDoor(Neighborhood *, bool isUpperDoor, const HotSpotID, const HotSpotID,
			const HotSpotID, TimeValue pressureSoundIn, TimeValue pressureSoundOut,
			TimeValue equalizeSoundIn, TimeValue equalizeSoundOut);
	virtual ~PressureDoor() {}

	void incrementPressure(const HotSpotID);
	void stopChangingPressure();

	void playAgainstRobot();

	bool canSolve();
	void doSolve();

protected:
	virtual void openInteraction();
	virtual void initInteraction();
	virtual void closeInteraction();

	virtual void activateHotspots();
	virtual void clickInHotspot(const Input &, const Hotspot *);

	virtual void receiveNotification(Notification *, const NotificationFlags);

	Movie _levelsMovie;
	TimeScale _levelsScale;
	Movie _typeMovie;
	TimeScale _typeScale;
	Sprite _upButton;
	Sprite _downButton;
	Notification _pressureNotification;
	NotificationCallBack _pressureCallBack;
	Notification *_neighborhoodNotification;
	int _gameState;
	HotSpotID _upHotspotID;
	HotSpotID _downHotspotID;
	HotSpotID _outHotspotID;
	PressureTracker _doorTracker;
	TimeValue _pressureSoundIn;
	TimeValue _pressureSoundOut;
	TimeValue _equalizeSoundIn;
	TimeValue _equalizeSoundOut;
	bool _isUpperDoor;

	bool _playingAgainstRobot, _typePunched;
	int _robotState, _punchCount;
	TimeBase _utilityTimer;
	Notification _utilityNotification;
	NotificationCallBack _utilityCallBack;
	TimeValue _punchInTime;
};

} // End of namespace Pegasus

#endif
