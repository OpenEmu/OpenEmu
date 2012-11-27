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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_MARS_H
#define PEGASUS_NEIGHBORHOOD_MARS_MARS_H

#include "pegasus/neighborhood/neighborhood.h"
#include "pegasus/neighborhood/mars/constants.h"
#include "pegasus/neighborhood/mars/energybeam.h"
#include "pegasus/neighborhood/mars/gravitoncannon.h"
#include "pegasus/neighborhood/mars/planetmover.h"
#include "pegasus/neighborhood/mars/reactor.h"
#include "pegasus/neighborhood/mars/robotship.h"
#include "pegasus/neighborhood/mars/shuttleenergymeter.h"
#include "pegasus/neighborhood/mars/shuttlehud.h"
#include "pegasus/neighborhood/mars/spacejunk.h"
#include "pegasus/neighborhood/mars/tractorbeam.h"

namespace Pegasus {

class InventoryItem;
class Mars;

enum MarsTimerCode {
	kMarsLaunchTubeReached,
	kMarsCanyonChaseFinished,
	kMarsSpaceChaseFinished // Player ran out of time...
};

struct MarsTimerEvent {
	Mars *mars;
	MarsTimerCode event;

	void fire();
};

enum ShuttleWeaponSelection {
	kNoWeapon,
	kEnergyBeam,
	kGravitonCannon,
	kTractorBeam
};

class Mars : public Neighborhood {
friend struct MarsTimerEvent;
public:
	Mars(InputHandler *, PegasusEngine *);
	virtual ~Mars();

	void flushGameState();

	virtual uint16 getDateResID() const;

	virtual AirQuality getAirQuality(const RoomID);

	void checkAirMask();

	void showBigExplosion(const Common::Rect &, const DisplayOrder);
	void showLittleExplosion(const Common::Rect &, const DisplayOrder);
	void hitByJunk();
	void decreaseRobotShuttleEnergy(const int, Common::Point impactPoint);
	void setUpNextDropTime();

	Common::String getBriefingMovie();
	Common::String getEnvScanMovie();
	uint getNumHints();
	Common::String getHintMovie(uint);

	virtual void shieldOn();
	virtual void shieldOff();

	void checkContinuePoint(const RoomID, const DirectionConstant);

	void setSoundFXLevel(const uint16);

	bool canSolve();
	void doSolve();

	bool inColorMatchingGame();

protected:
	enum {
		kMarsPrivatePodStorageOpenFlag,
		kMarsPrivatePodTurnLeftFlag,
		kMarsPrivatePodTurnRightFlag,
		kMarsPrivateRobotTiredOfWaitingFlag,
		kMarsPrivatePlatformZoomedInFlag,
		kMarsPrivateBombExposedFlag,
		kMarsPrivateDraggingBombFlag,
		kMarsPrivateInSpaceChaseFlag,
		kMarsPrivateGotMapChipFlag,
		kMarsPrivateGotOpticalChipFlag,
		kMarsPrivateGotShieldChipFlag,
		kNumMarsPrivateFlags
	};

	void init();
	void start();
	void setUpAIRules();
	void arriveAt(const RoomID, const DirectionConstant);
	void takeItemFromRoom(Item *);
	void dropItemIntoRoom(Item *, Hotspot *);
	void activateHotspots();
	void activateOneHotspot(HotspotInfoTable::Entry &, Hotspot *);
	void clickInHotspot(const Input &, const Hotspot *);
	InputBits getInputFilter();

	TimeValue getViewTime(const RoomID, const DirectionConstant);
	void getZoomEntry(const HotSpotID, ZoomTable::Entry &);
	void findSpotEntry(const RoomID, const DirectionConstant, SpotFlags, SpotTable::Entry &);
	CanOpenDoorReason canOpenDoor(DoorTable::Entry &);
	void openDoor();
	void closeDoorOffScreen(const RoomID, const DirectionConstant);
	int16 getStaticCompassAngle(const RoomID, const DirectionConstant);
	void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &);
	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &);
	void turnTo(const DirectionConstant);
	void receiveNotification(Notification *, const NotificationFlags);
	void doorOpened();
	void setUpReactorEnergyDrain();
	Hotspot *getItemScreenSpot(Item *, DisplayElement *);
	void lockThawed();
	void robotTiredOfWaiting();

	void setUpReactorLevel1();
	void setUpNextReactorLevel();
	void makeColorSequence();
	void doUndoOneGuess();
	void doReactorGuess(int32 guess);
	void bombExplodesInGame();
	void didntFindBomb();
	CanMoveForwardReason canMoveForward(ExitTable::Entry &);
	void cantMoveThatWay(CanMoveForwardReason);
	void moveForward();
	void bumpIntoWall();
	void turnLeft();
	void turnRight();
	void airStageExpired();
	void loadAmbientLoops();
	void checkAirlockDoors();
	void pickedUpItem(Item *item);
	void cantOpenDoor(CanOpenDoorReason);
	void launchMaze007Robot();
	void launchMaze015Robot();
	void launchMaze101Robot();
	void launchMaze104Robot();
	void launchMaze133Robot();
	void launchMaze136Robot();
	void launchMaze184Robot();
	void timerExpired(const uint32);
	void spotCompleted();

	void doCanyonChase(void);
	void startMarsTimer(TimeValue, TimeScale, MarsTimerCode);
	void marsTimerExpired(MarsTimerEvent &);
	void throwAwayMarsShuttle();
	void startUpFromFinishedSpaceChase();
	void startUpFromSpaceChase();
	void transportToRobotShip();
	void spaceChaseClick(const Input &, const HotSpotID);
	void updateCursor(const Common::Point, const Hotspot *);

	Common::String getSoundSpotsName();
	Common::String getNavMovieName();

	InventoryItem *_attackingItem;
	FuseFunction _bombFuse;
	FuseFunction _noAirFuse;
	FuseFunction _utilityFuse;
	FlagsArray<byte, kNumMarsPrivateFlags> _privateFlags;
	uint _reactorStage, _nextGuess;
	int32 _currentGuess[3];
	ReactorGuess _guessObject;
	Picture _undoPict;
	ReactorHistory _guessHistory;
	ReactorChoiceHighlight _choiceHighlight;

	Picture _shuttleInterface1;
	Picture _shuttleInterface2;
	Picture _shuttleInterface3;
	Picture _shuttleInterface4;
	Movie _canyonChaseMovie;

	MarsTimerEvent _marsEvent;

	Movie _leftShuttleMovie;
	Movie _rightShuttleMovie;
	Movie _lowerLeftShuttleMovie;
	Movie _lowerRightShuttleMovie;
	Movie _centerShuttleMovie;
	Movie _upperLeftShuttleMovie;
	Movie _upperRightShuttleMovie;
	Movie _leftDamageShuttleMovie;
	Movie _rightDamageShuttleMovie;
	ShuttleEnergyMeter _shuttleEnergyMeter;
	Movie _planetMovie;
	PlanetMover _planetMover;
	RobotShip _robotShip;
	ShuttleHUD _shuttleHUD;
	TractorBeam _tractorBeam;
	SpaceJunk _junk;
	EnergyBeam _energyBeam;
	GravitonCannon _gravitonCannon;
	Hotspot _energyChoiceSpot;
	Hotspot _gravitonChoiceSpot;
	Hotspot _tractorChoiceSpot;
	Hotspot _shuttleViewSpot;
	Hotspot _shuttleTransportSpot;
	ShuttleWeaponSelection _weaponSelection;
	ScalingMovie _explosions;
	NotificationCallBack _explosionCallBack;
};

} // End of namespace Pegasus

#endif
