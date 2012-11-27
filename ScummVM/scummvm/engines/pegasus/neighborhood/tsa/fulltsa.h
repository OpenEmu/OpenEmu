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

#ifndef PEGASUS_NEIGHBORHOOD_TSA_FULLTSA_H
#define PEGASUS_NEIGHBORHOOD_TSA_FULLTSA_H

#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

class RipTimer : public IdlerAnimation {
public:
	RipTimer(const DisplayElementID id) : IdlerAnimation(id) {}
	virtual ~RipTimer() {}

	void initImage();
	void releaseImage();

	void draw(const Common::Rect &);

protected:
	void timeChanged(const TimeValue);

	CoordType _middle;
	Surface _timerImage;
};

//	Room IDs.

static const RoomID kTSA00 = 0;
static const RoomID kTSA22Red = 28;
static const RoomID kTSA37 = 42;

class FullTSA : public Neighborhood {
public:
	FullTSA(InputHandler *, PegasusEngine *);
	virtual ~FullTSA() {}

	virtual void init();

	void start();

	virtual uint16 getDateResID() const;

	void flushGameState();

	void checkContinuePoint(const RoomID, const DirectionConstant);

	bool canSolve();
	void doSolve();

	void updateCursor(const Common::Point, const Hotspot *);

protected:
	enum {
		kTSAPrivateLogReaderOpenFlag,
		kTSAPrivateKeyVaultOpenFlag,
		kTSAPrivateChipVaultOpenFlag,
		kTSAPrivatePlayingLeftComparisonFlag,
		kTSAPrivatePlayingRightComparisonFlag,
		kTSAPrivateSeenRobotWarningFlag,
		kNumTSAPrivateFlags
	};

	Common::String getBriefingMovie();
	Common::String getEnvScanMovie();
	uint getNumHints();
	Common::String getHintMovie(uint);
	void loadAmbientLoops();
	virtual void clickInHotspot(const Input &, const Hotspot *);

	virtual int16 getStaticCompassAngle(const RoomID, const DirectionConstant);
	void activateOneHotspot(HotspotInfoTable::Entry &, Hotspot *spot);
	virtual void activateHotspots();
	void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &);
	void dropItemIntoRoom(Item *, Hotspot *);
	void downButton(const Input &);
	void startDoorOpenMovie(const TimeValue, const TimeValue);
	TimeValue getViewTime(const RoomID, const DirectionConstant);
	void findSpotEntry(const RoomID, const DirectionConstant, SpotFlags, SpotTable::Entry &);
	void turnTo(const DirectionConstant);
	CanMoveForwardReason canMoveForward(ExitTable::Entry &);
	CanOpenDoorReason canOpenDoor(DoorTable::Entry &);
	void bumpIntoWall();
	void initializeTBPMonitor(const int, const ExtraID);
	void playTBPMonitor();
	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &);
	Hotspot *getItemScreenSpot(Item *, DisplayElement *);
	void openDoor();
	void turnRight();
	void turnLeft();
	void closeDoorOffScreen(const RoomID, const DirectionConstant);
	void playExtraMovie(const ExtraTable::Entry &, const NotificationFlags, const InputBits interruptionInput);
	void handleInput(const Input &, const Hotspot *);
	void arriveAtTSA25Red();
	void startUpComparisonMonitor();
	void shutDownComparisonMonitor();
	void initializeComparisonMonitor(const int, const ExtraID);
	void playLeftComparison();
	void playRightComparison();
	void startRobotGame();
	void setOffRipAlarm();
	uint getHistoricalLogIndex();
	void startUpRobotMonitor();
	void shutDownRobotMonitor();
	void pickedUpItem(Item *item);
	void arriveFromPrehistoric();

	void arriveFromNorad();
	void arriveFromMars();
	void arriveFromWSC();

	InputBits getInputFilter();
	void arriveAt(const RoomID, const DirectionConstant);
	void initializePegasusButtons(bool);
	void releaseSprites();
	void showMainJumpMenu();
	void arriveAtTSA37();
	void receiveNotification(Notification *, const NotificationFlags);
	void checkRobotLocations(const RoomID, const DirectionConstant);
	void getExtraEntry(const uint32, ExtraTable::Entry &);

	Sprite _sprite1, _sprite2, _sprite3;
	FuseFunction _utilityFuse;
	RipTimer _ripTimer;

	FlagsArray<byte, kNumTSAPrivateFlags> _privateFlags;

	Common::String getNavMovieName();
	Common::String getSoundSpotsName();

	void dieUncreatedInTSA();
};

} // End of namespace Pegasus

#endif
