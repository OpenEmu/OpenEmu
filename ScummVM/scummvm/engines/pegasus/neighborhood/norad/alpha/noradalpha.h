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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_NORADALPHA_H
#define PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_NORADALPHA_H

#include "pegasus/neighborhood/norad/norad.h"

namespace Pegasus {

class Item;

class NoradAlpha : public Norad {
public:
	NoradAlpha(InputHandler *, PegasusEngine *);
	virtual ~NoradAlpha() {}

	virtual void init();
	void start();

	virtual bool okayToJump();

	void playClawMonitorIntro();

	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &);

	void turnOnFillingStation();
	void turnOffFillingStation();
	Item *getFillingItem() { return _fillingStationItem; }
	bool gasCanisterIntake();

	virtual void takeItemFromRoom(Item *);
	virtual void dropItemIntoRoom(Item *, Hotspot *);

	virtual GameInteraction *makeInteraction(const InteractionID);

	virtual void getClawInfo(HotSpotID &outSpotID, HotSpotID &prepSpotID, HotSpotID &clawControlSpotID,
			HotSpotID &pinchClawSpotID, HotSpotID &moveClawDownSpotID, HotSpotID &moveClawRightSpotID,
			HotSpotID &moveClawLeftSpotID, HotSpotID &moveClawUpSpotID, HotSpotID &clawCCWSpotID,
			HotSpotID &clawCWSpotID, uint32 &, const uint32 *&);

	void loadAmbientLoops();

	Common::String getEnvScanMovie();
	uint getNumHints();
	Common::String getHintMovie(uint);
	void setUpAIRules();

	void setSubPrepFailed(bool value) { _subPrepFailed = value; }
	bool getSubPrepFailed() { return _subPrepFailed; }

	void closeDoorOffScreen(const RoomID, const DirectionConstant);
	void findSpotEntry(const RoomID, const DirectionConstant, SpotFlags, SpotTable::Entry &);
	void clickInHotspot(const Input &, const Hotspot *);

	void checkContinuePoint(const RoomID, const DirectionConstant);

	bool canSolve();
	void doSolve();

protected:
	static const uint32 _noradAlphaClawExtras[22];

	virtual void arriveAtNorad01();
	virtual void arriveAtNorad01East();
	virtual void arriveAtNorad01West();
	virtual void arriveAtNorad04();
	virtual void arriveAtNorad22();

	virtual void arriveAt(const RoomID, const DirectionConstant);

	virtual void getZoomEntry(const HotSpotID, ZoomTable::Entry &);
	virtual TimeValue getViewTime(const RoomID, const DirectionConstant);

	virtual void receiveNotification(Notification *, const NotificationFlags);

	virtual void activateHotspots();

	Hotspot *getItemScreenSpot(Item *, DisplayElement *);

	void bumpIntoWall();

	Item *_fillingStationItem;

	bool _subPrepFailed;

	Common::String getSoundSpotsName();
	Common::String getNavMovieName();
};

} // End of namespace Pegasus

#endif
