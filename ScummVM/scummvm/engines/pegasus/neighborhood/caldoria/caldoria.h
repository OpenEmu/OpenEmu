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

#ifndef PEGASUS_NEIGHBORHOOD_CALDORIA_CALDORIA_H
#define PEGASUS_NEIGHBORHOOD_CALDORIA_CALDORIA_H

#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

static const TimeScale kCaldoriaMovieScale = 600;
static const TimeScale kCaldoriaFramesPerSecond = 15;
static const TimeScale kCaldoriaFrameDuration = 40;

// Alternate IDs.

static const AlternateID kAltCaldoriaNormal = 0;
static const AlternateID kAltCaldoriaRoofDoorBlown = 2;
static const AlternateID kAltCaldoriaSinclairDown = 3;

// Room IDs.

static const RoomID kCaldoria00 = 1;
static const RoomID kCaldoria01 = 2;
static const RoomID kCaldoria02 = 3;
static const RoomID kCaldoria03 = 4;
static const RoomID kCaldoria04 = 5;
static const RoomID kCaldoria05 = 6;
static const RoomID kCaldoria06 = 7;
static const RoomID kCaldoria07 = 8;
static const RoomID kCaldoria08 = 9;
static const RoomID kCaldoria09 = 10;
static const RoomID kCaldoria10 = 11;
static const RoomID kCaldoriaToilet = 12;
static const RoomID kCaldoria11 = 13;
static const RoomID kCaldoria12 = 14;
static const RoomID kCaldoriaVidPhone = 15;
static const RoomID kCaldoriaReplicator = 16;
static const RoomID kCaldoriaDrawers = 17;
static const RoomID kCaldoria13 = 18;
static const RoomID kCaldoria14 = 19;
static const RoomID kCaldoria15 = 20;
static const RoomID kCaldoria16 = 21;
static const RoomID kCaldoria17 = 22;
static const RoomID kCaldoria18 = 23;
static const RoomID kCaldoria19 = 24;
static const RoomID kCaldoria20 = 25;
static const RoomID kCaldoria21 = 26;
static const RoomID kCaldoria22 = 27;
static const RoomID kCaldoria23 = 28;
static const RoomID kCaldoria24 = 29;
static const RoomID kCaldoria25 = 30;
static const RoomID kCaldoria26 = 31;
static const RoomID kCaldoria27 = 32;
static const RoomID kCaldoria28 = 33;
static const RoomID kCaldoria29 = 34;
static const RoomID kCaldoria30 = 35;
static const RoomID kCaldoria31 = 36;
static const RoomID kCaldoria32 = 37;
static const RoomID kCaldoria33 = 38;
static const RoomID kCaldoria34 = 39;
static const RoomID kCaldoria35 = 40;
static const RoomID kCaldoria36 = 41;
static const RoomID kCaldoria37 = 42;
static const RoomID kCaldoria38 = 43;
static const RoomID kCaldoria39 = 44;
static const RoomID kCaldoria40 = 45;
static const RoomID kCaldoria41 = 46;
static const RoomID kCaldoriaBinoculars = 47;
static const RoomID kCaldoria42 = 48;
static const RoomID kCaldoriaKiosk = 49;
static const RoomID kCaldoria44 = 50;
static const RoomID kCaldoria45 = 51;
static const RoomID kCaldoria46 = 52;
static const RoomID kCaldoria47 = 53;
static const RoomID kCaldoria48 = 54;
static const RoomID kCaldoria49 = 55;
static const RoomID kCaldoria50 = 56;
static const RoomID kCaldoria51 = 57;
static const RoomID kCaldoria52 = 58;
static const RoomID kCaldoria53 = 59;
static const RoomID kCaldoria54 = 60;
static const RoomID kCaldoria55 = 61;
static const RoomID kCaldoria56 = 62;
static const RoomID kCaldoriaDeathRoom = 0;

// Hot Spot Activation IDs.

static const HotSpotActivationID kActivate4DClosed = 1;
static const HotSpotActivationID kActivate4DOpen = 2;
static const HotSpotActivationID kActivateMirrorReady = 3;
static const HotSpotActivationID kActivateStylistReady = 4;
static const HotSpotActivationID kActivateReplicatorReady = 5;
static const HotSpotActivationID kActivateOJOnThePad = 6;
static const HotSpotActivationID kActivateDrawersClosed = 7;
static const HotSpotActivationID kActivateRightOpen = 8;
static const HotSpotActivationID kActivateLeftOpen = 9;
static const HotSpotActivationID kActivateFocusedOnShip = 10;
static const HotSpotActivationID kActivateNotFocusedOnShip = 11;
static const HotSpotActivationID kActivateReadyForCard = 12;
static const HotSpotActivationID kActivateReadyToTransport = 13;
static const HotSpotActivationID kActivateRoofSlotEmpty = 14;
static const HotSpotActivationID kActivateZoomedOnSinclair = 15;

// Hot Spot IDs.

static const HotSpotID kCa4DEnvironOpenSpotID = 5000;
static const HotSpotID kCa4DEnvironCloseSpotID = 5001;
static const HotSpotID kCa4DVisualSpotID = 5002;
static const HotSpotID kCa4DAudioSpotID = 5003;
static const HotSpotID kCa4DChoice1SpotID = 5004;
static const HotSpotID kCa4DChoice2SpotID = 5005;
static const HotSpotID kCa4DChoice3SpotID = 5006;
static const HotSpotID kCa4DChoice4SpotID = 5007;
static const HotSpotID kCaBathroomMirrorSpotID = 5008;
static const HotSpotID kCaHairStyle1SpotID = 5009;
static const HotSpotID kCaHairStyle2SpotID = 5010;
static const HotSpotID kCaHairStyle3SpotID = 5011;
static const HotSpotID kCaShowerSpotID = 5012;
static const HotSpotID kCaBathroomToiletSpotID = 5013;
static const HotSpotID kCaldoriaVidPhoneSpotID = 5014;
static const HotSpotID kCaldoriaReplicatorSpotID = 5015;
static const HotSpotID kCaldoriaDrawersSpotID = 5016;
static const HotSpotID kCaldoriaVidPhoneOutSpotID = 5017;
static const HotSpotID kCaBedroomVidPhoneActivationSpotID = 5018;
static const HotSpotID kCaldoriaReplicatorOutSpotID = 5019;
static const HotSpotID kCaldoriaMakeOJSpotID = 5020;
static const HotSpotID kCaldoriaMakeStickyBunsSpotID = 5021;
static const HotSpotID kCaldoriaOrangeJuiceSpotID = 5022;
static const HotSpotID kCaldoriaOrangeJuiceDropSpotID = 5023;
static const HotSpotID kCaldoriaDrawersOutSpotID = 5024;
static const HotSpotID kCaldoriaLeftDrawerOpenSpotID = 5025;
static const HotSpotID kCaldoriaRightDrawerOpenSpotID = 5026;
static const HotSpotID kCaldoriaKeyCardSpotID = 5027;
static const HotSpotID kCaldoriaLeftDrawerCloseSpotID = 5028;
static const HotSpotID kCaldoriaRightDrawerWithKeysCloseSpotID = 5029;
static const HotSpotID kCaldoriaRightDrawerNoKeysCloseSpotID = 5030;
static const HotSpotID kCaldoriaFourthFloorElevatorSpotID = 5031;
static const HotSpotID kCaldoria20DoorbellSpotID = 5032;
static const HotSpotID kCaldoria21DoorbellSpotID = 5033;
static const HotSpotID kCaldoria26DoorbellSpotID = 5034;
static const HotSpotID kCaldoriaFourthFloorElevator1 = 5035;
static const HotSpotID kCaldoriaFourthFloorElevator2 = 5036;
static const HotSpotID kCaldoriaFourthFloorElevator3 = 5037;
static const HotSpotID kCaldoriaFourthFloorElevator4 = 5038;
static const HotSpotID kCaldoriaFourthFloorElevator5 = 5039;
static const HotSpotID kCaldoriaGroundElevator1 = 5040;
static const HotSpotID kCaldoriaGroundElevator2 = 5041;
static const HotSpotID kCaldoriaGroundElevator3 = 5042;
static const HotSpotID kCaldoriaGroundElevator4 = 5043;
static const HotSpotID kCaldoriaGroundElevator5 = 5044;
static const HotSpotID kCaldoria29DoorbellSpotID = 5045;
static const HotSpotID kCaldoria34DoorbellSpotID = 5046;
static const HotSpotID kCaldoria35DoorbellSpotID = 5047;
static const HotSpotID kCaldoriaGroundElevatorSpotID = 5048;
static const HotSpotID kCaldoriaBinocularZoomInSpotID = 5049;
static const HotSpotID kCaldoriaBinocularsOutSpotID = 5050;
static const HotSpotID kCaldoriaZoomInOnShipSpotID = 5051;
static const HotSpotID kCaldoriaKioskSpotID = 5052;
static const HotSpotID kCaldoriaKioskOutSpotID = 5053;
static const HotSpotID kCaldoriaKioskInfoSpotID = 5054;
static const HotSpotID kCaldoriaGTCardDropSpotID = 5055;
static const HotSpotID kCaldoriaGTTokyoSpotID = 5056;
static const HotSpotID kCaldoriaGTTSASpotID = 5057;
static const HotSpotID kCaldoriaGTBeachSpotID = 5058;
static const HotSpotID kCaldoriaGTOtherSpotID = 5059;
static const HotSpotID kCaldoriaRoofElevator1 = 5060;
static const HotSpotID kCaldoriaRoofElevator2 = 5061;
static const HotSpotID kCaldoriaRoofElevator3 = 5062;
static const HotSpotID kCaldoriaRoofElevator4 = 5063;
static const HotSpotID kCaldoriaRoofElevator5 = 5064;
static const HotSpotID kCaldoriaRoofElevatorSpotID = 5065;
static const HotSpotID kCaldoriaRoofDoorSpotID = 5066;
static const HotSpotID kCaldoriaRoofCardDropSpotID = 5067;
static const HotSpotID kCaldoria53EastSinclairTargetSpotID = 5068;

// Extra sequence IDs.

static const ExtraID kCaldoriaWakeUpView1 = 0;
static const ExtraID kCaldoria00WakeUp1 = 1;
static const ExtraID kCaldoria00WakeUp2 = 2;
static const ExtraID kCaldoria00SitDown = 3;
static const ExtraID k4DEnvironOpenToINN = 4;
static const ExtraID k4DINNInterruption = 5;
static const ExtraID k4DINNIntro = 6;
static const ExtraID k4DINNMarkJohnson = 7;
static const ExtraID k4DINNMeganLove = 8;
static const ExtraID k4DINNFadeOut = 9;
static const ExtraID k4DEnvironOpenFromINN = 10;
static const ExtraID k4DEnvironOpen = 11;
static const ExtraID k4DEnvironOpenView = 12;
static const ExtraID k4DEnvironClose = 13;
static const ExtraID k4DIslandLoop = 14;
static const ExtraID k4DDesertLoop = 15;
static const ExtraID k4DMountainLoop = 16;
static const ExtraID k4DIsland1ToIsland0 = 17;
static const ExtraID k4DIsland2ToIsland0 = 18;
static const ExtraID k4DIsland0ToDesert0 = 19;
static const ExtraID k4DIsland1ToDesert0 = 20;
static const ExtraID k4DIsland2ToDesert0 = 21;
static const ExtraID k4DIsland0ToMountain0 = 22;
static const ExtraID k4DIsland1ToMountain0 = 23;
static const ExtraID k4DIsland2ToMountain0 = 24;
static const ExtraID k4DDesert0ToIsland0 = 25;
static const ExtraID k4DDesert1ToIsland0 = 26;
static const ExtraID k4DDesert2ToIsland0 = 27;
static const ExtraID k4DDesert0ToMountain0 = 28;
static const ExtraID k4DDesert1ToMountain0 = 29;
static const ExtraID k4DDesert2ToMountain0 = 30;
static const ExtraID k4DMountain0ToIsland0 = 31;
static const ExtraID k4DMountain1ToIsland0 = 32;
static const ExtraID k4DMountain2ToIsland0 = 33;
static const ExtraID k4DMountain0ToDesert0 = 34;
static const ExtraID k4DMountain1ToDesert0 = 35;
static const ExtraID k4DMountain2ToDesert0 = 36;
static const ExtraID kCaBathroomGreeting = 37;
static const ExtraID kCaBathroomBodyFat = 38;
static const ExtraID kCaBathroomStylistIntro = 39;
static const ExtraID kCaBathroomRetrothrash = 40;
static const ExtraID kCaBathroomRetrothrashReturn = 41;
static const ExtraID kCaBathroomGeoWave = 42;
static const ExtraID kCaBathroomGeoWaveReturn = 43;
static const ExtraID kCaBathroomAgencyStandard = 44;
static const ExtraID kCaldoriaShowerTitle = 45;
static const ExtraID kCaldoriaShowerButton = 46;
static const ExtraID kCaldoriaShowerDown = 47;
static const ExtraID kCaldoriaShowerUp = 48;
static const ExtraID kCaBedroomVidPhone = 49;
static const ExtraID kCaBedroomMessage1 = 50;
static const ExtraID kCaBedroomMessage2 = 51;
static const ExtraID kCreateOrangeJuice = 52;
static const ExtraID kDisposeOrangeJuice = 53;
static const ExtraID kReplicatorNorthViewWithOJ = 54;
static const ExtraID kLeftDrawerOpen = 55;
static const ExtraID kLeftDrawerClose = 56;
static const ExtraID kRightDrawerOpenWithKeys = 57;
static const ExtraID kRightDrawerCloseWithKeys = 58;
static const ExtraID kRightDrawerOpenNoKeys = 59;
static const ExtraID kRightDrawerCloseNoKeys = 60;
static const ExtraID kRightDrawerOpenViewWithKeys = 61;
static const ExtraID kRightDrawerOpenViewNoKeys = 62;
static const ExtraID kCaldoria16ElevatorUp = 63;
static const ExtraID kCaldoria16ElevatorDown = 64;
static const ExtraID kCaldoria16SouthViewWithElevator = 65;
static const ExtraID kCaldoria20Doorbell = 66;
static const ExtraID kCaldoria21Doorbell = 67;
static const ExtraID kCaldoria26Doorbell = 68;
static const ExtraID kCaldoriaFourthToGround = 69;
static const ExtraID kCaldoriaRoofToFourth = 70;
static const ExtraID kCaldoriaRoofToGround = 71;
static const ExtraID kCaldoriaGroundToFourth = 72;
static const ExtraID kCaldoriaGroundToRoof = 73;
static const ExtraID kCaldoriaFourthToRoof = 74;
static const ExtraID kCaldoria29Doorbell = 75;
static const ExtraID kCaldoria34Doorbell = 76;
static const ExtraID kCaldoria35Doorbell = 77;
static const ExtraID kBinocularsZoomInOnShip = 78;
static const ExtraID kCaldoriaKioskVideo = 79;
static const ExtraID kCaldoriaTransporterArrowLoop = 80;
static const ExtraID kArriveAtCaldoriaFromTSA = 81;
static const ExtraID kCaGTOtherChoice = 82;
static const ExtraID kCaGTCardSwipe = 83;
static const ExtraID kCaGTSelectTSA = 84;
static const ExtraID kCaGTFryTheFly = 85;
static const ExtraID kCaGTGoToTSA = 86;
static const ExtraID kCaGTSelectBeach = 87;
static const ExtraID kCaGTGoToBeach = 88;
static const ExtraID kCaGTArriveAtBeach = 89;
static const ExtraID kCaGTSelectTokyo = 90;
static const ExtraID kCaGTGoToTokyo = 91;
static const ExtraID kCaGTArriveAtTokyo = 92;
static const ExtraID kCa48NorthRooftopClosed = 93;
static const ExtraID kCa48NorthExplosion = 94;
static const ExtraID kCa48NorthExplosionDeath = 95;
static const ExtraID kCa49NorthVoiceAnalysis = 96;
static const ExtraID kCa50SinclairShoots = 97;
static const ExtraID kCa53EastZoomToSinclair = 98;
static const ExtraID kCa53EastDeath2 = 99;
static const ExtraID kCa53EastShootSinclair = 100;
static const ExtraID kCa53EastZoomOutFromSinclair = 101;
static const ExtraID kCa54SouthDeath = 102;
static const ExtraID kCaldoria56BombStage1 = 103;
static const ExtraID kCaldoria56BombStage2 = 104;
static const ExtraID kCaldoria56BombStage3 = 105;
static const ExtraID kCaldoria56BombStage4 = 106;
static const ExtraID kCaldoria56BombStage5 = 107;
static const ExtraID kCaldoria56BombStage6 = 108;
static const ExtraID kCaldoria56BombStage7 = 109;
static const ExtraID kCaldoria56BombExplodes = 110;

// Caldoria interactions.

static const InteractionID kCaldoria4DInteractionID = 0;
static const InteractionID kCaldoriaBombInteractionID = 1;
static const InteractionID kCaldoriaMessagesInteractionID = 2;
static const InteractionID kCaldoriaMirrorInteractionID = 3;

// Caldoria:

static const DisplayOrder kVidPhoneOrder = kMonitorLayer;
static const DisplayOrder k4DSpritesOrder = kMonitorLayer;
static const DisplayOrder kCaldoriaMessagesOrder = kMonitorLayer;
static const DisplayOrder kCaldoriaElevatorOrder = kMonitorLayer;
static const DisplayOrder kCaldoriaA05LightLoopOrder = kMonitorLayer;
static const DisplayOrder kCaldoriaA07LightLoopOrder = kMonitorLayer;
static const DisplayOrder kCaldoriaBombGridOrder = kMonitorLayer;
static const DisplayOrder kCaldoriaBombTimerOrder = kCaldoriaBombGridOrder + 1;

/////////////////////////////////////////////
//
// Caldoria

static const CoordType kCaldoriaVidPhoneLeft = kNavAreaLeft + 105;
static const CoordType kCaldoriaVidPhoneTop = kNavAreaTop + 28;

static const CoordType kCaldoria4DSpritesLeft = kNavAreaLeft + 10;
static const CoordType kCaldoria4DSpritesTop = kNavAreaTop + 142;

static const CoordType kCaldoriaMessageLeft = kNavAreaLeft + 202;
static const CoordType kCaldoriaMessageTop = kNavAreaTop + 26;

static const CoordType kCaldoriaElevatorLeft = kNavAreaLeft + 407;
static const CoordType kCaldoriaElevatorTop = kNavAreaTop + 138;

static const CoordType kCaldoriaA05LightLoopLeft = kNavAreaLeft + 213;
static const CoordType kCaldoriaA05LightLoopTop = kNavAreaTop + 215;

static const CoordType kCaldoriaA07LightLoopLeft = kNavAreaLeft + 414;
static const CoordType kCaldoriaA07LightLoopTop = kNavAreaTop + 215;

static const CoordType kCaldoriaGunSpriteLeft = kNavAreaLeft + 276;
static const CoordType kCaldoriaGunSpriteTop = kNavAreaTop + 115;

static const CoordType kCaldoria11MessageLoopLeft = kNavAreaLeft + 135;
static const CoordType kCaldoria11MessageLoopTop = kNavAreaTop + 214;

static const CoordType kCaldoria12MessageLoopLeft = kNavAreaLeft + 209;
static const CoordType kCaldoria12MessageLoopTop = kNavAreaTop + 170;

static const CoordType kCaldoria13MessageLoopLeft = kNavAreaLeft + 480;
static const CoordType kCaldoria13MessageLoopTop = kNavAreaTop + 191;

static const CoordType kCaldoria14MessageLoopLeft = kNavAreaLeft + 248;
static const CoordType kCaldoria14MessageLoopTop = kNavAreaTop + 191;

static const CoordType kCaldoria48CardBombLoopLeft = kNavAreaLeft + 337;
static const CoordType kCaldoria48CardBombLoopTop = kNavAreaTop + 205;

static const CoordType kCaldoriaBombGridLeft = kNavAreaLeft + 290;
static const CoordType kCaldoriaBombGridTop = kNavAreaTop + 58;

static const CoordType kCaldoriaBombTimerLeft = kNavAreaLeft + 58;
static const CoordType kCaldoriaBombTimerTop = kNavAreaTop + 204;

// Caldoria display IDs.

static const DisplayElementID kCaldoriaVidPhoneID = kNeighborhoodDisplayID;
static const DisplayElementID kCaldoria4DSpritesID = kCaldoriaVidPhoneID + 1;
static const DisplayElementID kCaldoriaMessagesID = kCaldoria4DSpritesID + 1;
static const DisplayElementID kCaldoriaUtilityID = kCaldoriaMessagesID + 1;
static const DisplayElementID kCaldoriaBombGridID = kCaldoriaUtilityID + 1;
static const DisplayElementID kCaldoriaBombTimerID = kCaldoriaBombGridID + 1;

static const TimeValue kCaldoria4DBlankChoiceIn = 29730;
static const TimeValue kCaldoria4DBlankChoiceOut = 33910;

class Caldoria;

class SinclairCallBack : public TimeBaseCallBack {
public:
	SinclairCallBack(Caldoria *);
	~SinclairCallBack() {}

protected:
	virtual void callBack();

	Caldoria *_caldoria;
};

class Caldoria : public Neighborhood {
friend class SinclairCallBack;

public:
	Caldoria(InputHandler *, PegasusEngine *);
	virtual ~Caldoria();

	virtual uint16 getDateResID() const;

	void pickedUpItem(Item *);

	virtual GameInteraction *makeInteraction(const InteractionID);

	virtual Common::String getBriefingMovie();
	virtual Common::String getEnvScanMovie();
	virtual uint getNumHints();
	virtual Common::String getHintMovie(uint);
	void loadAmbientLoops();
	bool wantsCursor();
	void flushGameState();

	void checkContinuePoint(const RoomID, const DirectionConstant);

protected:
	enum {
		kCaldoriaPrivate4DSystemOpenFlag,
		kCaloriaPrivateLeftDrawerOpenFlag,
		kCaldoriaPrivateRightDrawerOpenFlag,
		kCaldoriaPrivateReadyToShootFlag,
		kCaldoriaPrivateZoomingToBombFlag,
		kCaldoriaPrivateCanOpenElevatorDoorFlag,
		kCaldoriaPrivateSinclairTimerExpiredFlag,
		kCaldoriaPrivateSeen13CarFlag,
		kCaldoriaPrivateSeen14CarFlag,
		kCaldoriaPrivateSeen18CarFlag,
		kCaldoriaPrivateSeen23CarFlag,
		kCaldoriaPrivateSeen33CarFlag,
		kCaldoriaPrivateSeen36CarFlag,
		kCaldoriaPrivateSeen41NorthCarFlag,
		kCaldoriaPrivateSeen41EastCarFlag,
		kCaldoriaPrivateSeen41WestCarFlag,
		kNumCaldoriaPrivateFlags
	};

	void init();
	void start();

	void setUpRoofTop();

	void setUpAIRules();
	void doAIRecalibration();
	TimeValue getViewTime(const RoomID, const DirectionConstant);
	void findSpotEntry(const RoomID, const DirectionConstant, SpotFlags, SpotTable::Entry &);
	void startSpotOnceOnly(TimeValue, TimeValue);
	void startExitMovie(const ExitTable::Entry &);
	void startZoomMovie(const ZoomTable::Entry &);
	void startDoorOpenMovie(const TimeValue, const TimeValue);
	void startTurnPush(const TurnDirection, const TimeValue, const DirectionConstant);
	void bumpIntoWall();
	int16 getStaticCompassAngle(const RoomID, const DirectionConstant);
	void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &);
	void getZoomCompassMove(const ZoomTable::Entry &, FaderMoveSpec &);
	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &);
	void spotCompleted();
	void arriveAt(const RoomID, const DirectionConstant);
	void arriveAtCaldoria00();
	void arriveAtCaldoriaToilet();
	void arriveAtCaldoria44();
	void arriveAtCaldoria49();
	void arriveAtCaldoria56();
	void arriveAtCaldoriaDeath();
	void turnTo(const DirectionConstant);
	void zoomTo(const Hotspot *);
	void downButton(const Input &);
	void receiveNotification(Notification *, const NotificationFlags);
	InputBits getInputFilter();
	void activateHotspots();
	void clickInHotspot(const Input &, const Hotspot *);
	void newInteraction(const InteractionID);

	void clickOnDoorbell(const HotSpotID);

	Hotspot *getItemScreenSpot(Item *, DisplayElement *);
	void dropItemIntoRoom(Item *, Hotspot *);
	void takeElevator(uint, uint);
	void updateElevatorMovie();
	void openElevatorMovie();
	void emptyOJGlass();
	void closeDoorOffScreen(const RoomID, const DirectionConstant);
	void doorBombTimerExpired();
	void sinclairTimerExpired();
	void checkSinclairShootsOS();
	void setUpSinclairLoops();
	void zoomToSinclair();
	void playEndMessage();
	void checkInterruptSinclair();

	CanOpenDoorReason canOpenDoor(DoorTable::Entry &);
	void doorOpened();

	void updateCursor(const Common::Point, const Hotspot *);

	FlagsArray<uint16, kNumCaldoriaPrivateFlags> _privateFlags;

	const Hotspot *_zoomOutSpot;

	FuseFunction _utilityFuse;

	long _sinclairLoopCount;
	long _numSinclairLoops;

	Sprite *_gunSprite;

	SinclairCallBack _sinclairInterrupt;

	Common::String getSoundSpotsName();
	Common::String getNavMovieName();
};

} // End of namespace Pegasus

#endif
