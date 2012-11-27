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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_NORAD_H
#define PEGASUS_NEIGHBORHOOD_NORAD_NORAD_H

#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

// This is the code common to both Norad Alpha and Norad Delta

class Norad : public Neighborhood {
public:
	Norad(InputHandler *, PegasusEngine *owner, const Common::String &resName, const NeighborhoodID);
	virtual ~Norad() {}

	void flushGameState();

	virtual void start();

	virtual void getClawInfo(HotSpotID &outSpotID, HotSpotID &prepSpotID,
			HotSpotID &clawControlSpotID, HotSpotID &pinchClawSpotID,
			HotSpotID &moveClawDownSpotID, HotSpotID &moveClawRightSpotID,
			HotSpotID &moveClawLeftSpotID,HotSpotID &moveClawUpSpotID,
			HotSpotID &clawCCWSpotID, HotSpotID &clawCWSpotID, uint32 &, const uint32 *&) = 0;
	void checkAirMask();

	virtual uint16 getDateResID() const;

	virtual GameInteraction *makeInteraction(const InteractionID);

	Common::String getBriefingMovie();

	void pickedUpItem(Item *);

	virtual void playClawMonitorIntro() {}

	void doneWithPressureDoor();

protected:
	CanOpenDoorReason canOpenDoor(DoorTable::Entry &);
	void cantOpenDoor(CanOpenDoorReason);
	int16 getStaticCompassAngle(const RoomID, const DirectionConstant);
	virtual void startExitMovie(const ExitTable::Entry &);
	void startZoomMovie(const ZoomTable::Entry &);
	virtual void upButton(const Input &);
	virtual void activateHotspots();

	virtual void arriveAt(const RoomID, const DirectionConstant);
	virtual void arriveAtNoradElevator();
	virtual void arriveAtUpperPressureDoorRoom();
	virtual void arriveAtLowerPressureDoorRoom();
	virtual void arriveAtSubPlatformRoom();
	virtual void arriveAtSubControlRoom();
	void setUpAirMask();
	virtual void receiveNotification(Notification *, const NotificationFlags);
	virtual bool playingAgainstRobot() { return false; }

	Notification _noradNotification;
	bool _doneWithPressureDoor;

	RoomID _elevatorUpRoomID;
	RoomID _elevatorDownRoomID;
	HotSpotID _elevatorUpSpotID;
	HotSpotID _elevatorDownSpotID;

	TimeBase _airMaskTimer;
	NotificationCallBack _airMaskCallBack;

	RoomID _subRoomEntryRoom1;
	DirectionConstant _subRoomEntryDir1;
	RoomID _subRoomEntryRoom2;
	DirectionConstant _subRoomEntryDir2;
	RoomID _upperPressureDoorRoom;
	RoomID _lowerPressureDoorRoom;

	HotSpotID _upperPressureDoorUpSpotID;
	HotSpotID _upperPressureDoorDownSpotID;
	HotSpotID _upperPressureDoorAbortSpotID;

	HotSpotID _lowerPressureDoorUpSpotID;
	HotSpotID _lowerPressureDoorDownSpotID;
	HotSpotID _lowerPressureDoorAbortSpotID;

	TimeValue _pressureSoundIn;
	TimeValue _pressureSoundOut;
	TimeValue _equalizeSoundIn;
	TimeValue _equalizeSoundOut;
	TimeValue _accessDeniedIn;
	TimeValue _accessDeniedOut;

	RoomID	 _platformRoom;
	RoomID _subControlRoom;
};

} // End of namespace Pegasus

#endif
