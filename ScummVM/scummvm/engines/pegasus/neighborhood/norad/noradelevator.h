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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_NORADELEVATOR_H
#define PEGASUS_NEIGHBORHOOD_NORAD_NORADELEVATOR_H

#include "pegasus/interaction.h"
#include "pegasus/notification.h"
#include "pegasus/surface.h"
#include "pegasus/timers.h"

namespace Pegasus {

class Neighborhood;

class NoradElevator : public GameInteraction, private NotificationReceiver {
public:
	NoradElevator(Neighborhood *, const RoomID, const RoomID, const HotSpotID, const HotSpotID);
	virtual ~NoradElevator() {}

protected:
	virtual void openInteraction();
	virtual void initInteraction();
	virtual void closeInteraction();
	virtual void resetInteraction();

	virtual void activateHotspots();
	virtual void clickInHotspot(const Input &, const Hotspot *);

	virtual void receiveNotification(Notification *, const NotificationFlags);

	RoomID _upRoom;
	RoomID _downRoom;
	HotSpotID _upHotspot;
	HotSpotID _downHotspot;
	Sprite _elevatorControls;
	TimeBase _elevatorTimer;
	NotificationCallBack _elevatorCallBack;
	Notification _elevatorNotification;
	bool _timerExpired;
};

} // End of namespace Pegasus

#endif
