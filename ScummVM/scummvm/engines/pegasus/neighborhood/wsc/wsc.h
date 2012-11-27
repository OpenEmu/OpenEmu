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

#ifndef PEGASUS_NEIGHBORHOOD_WSC_WSC_H
#define PEGASUS_NEIGHBORHOOD_WSC_WSC_H

#include "pegasus/neighborhood/neighborhood.h"
#include "pegasus/neighborhood/wsc/moleculebin.h"

namespace Pegasus {

static const DisplayOrder kWSCMoleculeBinOrder = kMonitorLayer;
static const DisplayOrder kWSCMoleculesMovieOrder = kWSCMoleculeBinOrder + 1;

static const RoomID kWSC01 = 0;
static const RoomID kWSC02Morph = 2;
static const RoomID kWSC02Messages = 3;
static const RoomID kWSC62 = 62;

class WSC : public Neighborhood {
public:
	WSC(InputHandler *, PegasusEngine *);
	virtual ~WSC() {}

	void flushGameState();

	virtual uint16 getDateResID() const;

	bool okayToJump();

	void checkContinuePoint(const RoomID, const DirectionConstant);

	bool inSynthesizerGame();

	bool canSolve();
	void doSolve();

	virtual void prepareForAIHint(const Common::String &);
	virtual void cleanUpAfterAIHint(const Common::String &);

	void init();
	void start();

protected:
	enum {
		kWSCDraggingAntidoteFlag,

		kWSCPrivateLabMessagesOpenFlag,
		kWSCPrivateInterruptedMorphFlag,
		kWSCPrivateInMoleculeGameFlag,
		kWSCPrivateSinclairOfficeOpenFlag,
		kWSCPrivateOfficeLogOpenFlag,
		kWSCPrivate58SouthOpenFlag,
		kWSCPrivateClickedCatwalkCableFlag,
		kWSCPrivateRobotHeadOpenFlag,

		kWSCPrivateSeenPeopleAt17WestFlag,
		kWSCPrivateSeenPeopleAt19NorthFlag,
		kWSCPrivateSeenPeopleAt21SouthFlag,
		kWSCPrivateSeenPeopleAt24SouthFlag,
		kWSCPrivateSeenPeopleAt34EastFlag,
		kWSCPrivateSeenPeopleAt36WestFlag,
		kWSCPrivateSeenPeopleAt38NorthFlag,
		kWSCPrivateSeenPeopleAt46SouthFlag,
		kWSCPrivateSeenPeopleAt49NorthFlag,
		kWSCPrivateSeenPeopleAt73WestFlag,

		kWSCPrivateNeedPeopleAt17WestFlag,
		kWSCPrivateNeedPeopleAt21SouthFlag,
		kWSCPrivateNeedPeopleAt24SouthFlag,
		kWSCPrivateNeedPeopleAt34EastFlag,
		kWSCPrivateNeedPeopleAt36WestFlag,
		kWSCPrivateNeedPeopleAt38NorthFlag,
		kWSCPrivateNeedPeopleAt46SouthFlag,
		kWSCPrivateNeedPeopleAt49NorthFlag,
		kWSCPrivateNeedPeopleAt73WestFlag,

		kWSCPrivateGotRetScanChipFlag,
		kWSCPrivateGotMapChipFlag,
		kWSCPrivateGotOpticalChipFlag,

		kNumWSCPrivateFlags
	};

	void arriveAt(const RoomID, const DirectionConstant);
	void turnTo(const DirectionConstant);
	void receiveNotification(Notification *, const NotificationFlags);
	void dropItemIntoRoom(Item *, Hotspot *);
	void clickInHotspot(const Input &, const Hotspot *);
	TimeValue getViewTime(const RoomID, const DirectionConstant);
	void getZoomEntry(const HotSpotID, ZoomTable::Entry &);
	CanMoveForwardReason canMoveForward(ExitTable::Entry &entry);
	void cantMoveThatWay(CanMoveForwardReason reason);
	CanTurnReason canTurn(TurnDirection turn, DirectionConstant &nextDir);
	void zoomTo(const Hotspot *hotspot);
	void activateOneHotspot(HotspotInfoTable::Entry &, Hotspot *);
	void setUpMoleculeGame();
	void nextMoleculeGameLevel();
	void startMoleculeGameLevel();
	void moleculeGameClick(const HotSpotID);
	void loadAmbientLoops();
	CanOpenDoorReason canOpenDoor(DoorTable::Entry &);
	void cantOpenDoor(CanOpenDoorReason);
	void pickedUpItem(Item *);
	void doorOpened();
	void startExtraSequence(const ExtraID, const NotificationFlags, const InputBits);
	void getExtraEntry(const uint32, ExtraTable::Entry &);
	void takeItemFromRoom(Item *item);
	void checkPeopleCrossing();
	void turnLeft();
	void turnRight();
	void moveForward();
	Hotspot *getItemScreenSpot(Item *, DisplayElement *);
	int16 getStaticCompassAngle(const RoomID, const DirectionConstant);
	void getExitCompassMove(const ExitTable::Entry &exitEntry, FaderMoveSpec &compassMove);
	void getExtraCompassMove(const ExtraTable::Entry &entry, FaderMoveSpec &compassMove);
	void bumpIntoWall();
	void activateHotspots();
	void setUpAIRules();
	Common::String getBriefingMovie();
	Common::String getEnvScanMovie();
	uint getNumHints();
	Common::String getHintMovie(uint);
	void closeDoorOffScreen(const RoomID, const DirectionConstant);
	void setUpPoison();
	void findSpotEntry(const RoomID, const DirectionConstant, SpotFlags, SpotTable::Entry &);
	void timerExpired(const uint32);

	Common::String getSoundSpotsName();
	Common::String getNavMovieName();

	FlagsArray<byte, kNumWSCPrivateFlags> _privateFlags;
	const Hotspot *_cachedZoomSpot;
	MoleculeBin _moleculeBin;
	int32 _moleculeGameLevel, _numCorrect;
	Movie _moleculesMovie;
	uint32 _levelArray[6];
	Common::Rational _energyDrainRate;
	Sprite *_argonSprite;
};

} // End of namespace Pegasus

#endif
