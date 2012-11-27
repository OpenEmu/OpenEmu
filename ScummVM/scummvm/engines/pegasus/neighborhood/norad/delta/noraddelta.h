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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_DELTA_NORADDELTA_H
#define PEGASUS_NEIGHBORHOOD_NORAD_DELTA_NORADDELTA_H

#include "pegasus/neighborhood/norad/norad.h"

namespace Pegasus {

class NoradDelta : public Norad {
public:
	NoradDelta(InputHandler *, PegasusEngine *);
	virtual ~NoradDelta() {}

	void init();

	void start();

	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &);

	void finishedGlobeGame();

	virtual GameInteraction *makeInteraction(const InteractionID);

	void playClawMonitorIntro();

	virtual void getClawInfo(HotSpotID &outSpotID, HotSpotID &prepSpotID, HotSpotID &clawControlSpotID,
			HotSpotID &pinchClawSpotID, HotSpotID &moveClawDownSpotID, HotSpotID &moveClawRightSpotID,
			HotSpotID &moveClawLeftSpotID, HotSpotID &moveClawUpSpotID, HotSpotID &clawCCWSpotID,
			HotSpotID &clawCWSpotID, uint32 &, const uint32 *&);

	void playerBeatRobotWithClaw();
	void playerBeatRobotWithDoor();

	void loadAmbientLoops();

	void setUpAIRules();
	Common::String getEnvScanMovie();
	uint getNumHints();
	Common::String getHintMovie(uint);
	void closeDoorOffScreen(const RoomID, const DirectionConstant);

	void checkContinuePoint(const RoomID, const DirectionConstant);

	bool canSolve();
	void doSolve();

	void doorOpened();

protected:
	enum {
		kNoradPrivateArrivedFromSubFlag,
		kNoradPrivateFinishedGlobeGameFlag,
		kNoradPrivateRobotHeadOpenFlag,
		kNoradPrivateGotShieldChipFlag,
		kNoradPrivateGotOpticalChipFlag,
		kNoradPrivateGotRetScanChipFlag,
		kNumNoradPrivateFlags
	};

	static const uint32 _noradDeltaClawExtras[22];

	void getExitEntry(const RoomID, const DirectionConstant, ExitTable::Entry &);
	void getZoomEntry(const HotSpotID, ZoomTable::Entry &);
	virtual void arriveAt(const RoomID, const DirectionConstant);
	void arriveAtNorad68West();
	void arriveAtNorad79West();
	TimeValue getViewTime(const RoomID, const DirectionConstant);
	void openDoor();
	void activateHotspots();
	void clickInHotspot(const Input &, const Hotspot *);
	void receiveNotification(Notification *, const NotificationFlags);
	void pickedUpItem(Item *item);
	void takeItemFromRoom(Item *item);
	void dropItemIntoRoom(Item *item, Hotspot *);
	Hotspot *getItemScreenSpot(Item *, DisplayElement *);

	virtual bool playingAgainstRobot();

	void failRetinalScan();
	void succeedRetinalScan();
	void getDoorEntry(const RoomID, const DirectionConstant, DoorTable::Entry &);

	void bumpIntoWall();

	FlagsArray<byte, kNumNoradPrivateFlags> _privateFlags;

	Common::String getSoundSpotsName();
	Common::String getNavMovieName();
};

} // End of namespace Pegasus

#endif
