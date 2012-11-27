/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_GLOBALS_H
#define TSAGE_GLOBALS_H

#include "common/random.h"
#include "tsage/core.h"
#include "tsage/dialogs.h"
#include "tsage/scenes.h"
#include "tsage/events.h"
#include "tsage/sound.h"
#include "tsage/saveload.h"
#include "tsage/user_interface.h"

namespace TsAGE {

class Globals : public SavedObject {
private:
	static void dispatchSound(ASound *obj);
public:
	GfxSurface _screenSurface;
	GfxManager _gfxManagerInstance;
	Common::List<GfxManager *> _gfxManagers;
	SceneHandler *_sceneHandler;
	Game *_game;
	EventsClass _events;
	SceneManager _sceneManager;
	ScenePalette _scenePalette;
	SceneRegions _sceneRegions;
	SceneItemList _sceneItems;
	SceneObjectList _sceneObjectsInstance;
	SceneObjectList *_sceneObjects;
	SynchronizedList<SceneObjectList *> _sceneObjects_queue;
	SceneText _sceneText;
	int _gfxFontNumber;
	GfxColors _gfxColors;
	GfxColors _fontColors;
	byte _color1, _color2, _color3;
	SoundManager _soundManager;
	Common::Point _dialogCenter;
	WalkRegions _walkRegions;
	SynchronizedList<ASound *> _sounds;
	bool _flags[256];
	Player _player;
	ASound _soundHandler;
	InvObjectList *_inventory;
	Region _paneRegions[2];
	int _paneRefreshFlag[2];
	Common::Point _sceneOffset;
	Common::Point _prevSceneOffset;
	SceneObject *_scrollFollower;
	SequenceManager _sequenceManager;
	Common::RandomSource _randomSource;
	int _stripNum;
	int _gfxEdgeAdjust;
public:
	Globals();
	~Globals();

	void setFlag(int flagNum) {
		assert((flagNum >= 0) && (flagNum < MAX_FLAGS));
		_flags[flagNum] = true;
	}
	void clearFlag(int flagNum) {
		assert((flagNum >= 0) && (flagNum < MAX_FLAGS));
		_flags[flagNum] = false;
	}
	bool getFlag(int flagNum) const {
		assert((flagNum >= 0) && (flagNum < MAX_FLAGS));
		return _flags[flagNum];
	}

	GfxManager &gfxManager() { return **_gfxManagers.begin(); }
	virtual Common::String getClassName() { return "Globals"; }
	virtual void synchronize(Serializer &s);
	virtual void reset();

	void dispatchSounds();
};

typedef bool (*SelectItemProc)(int objectNumber);

/**
 * The following class represents common globals that were introduced after the release of Ringworld.
 */
class TsAGE2Globals: public Globals {
public:
	UIElements _uiElements;
	SelectItemProc _onSelectItem;
	int _interfaceY;
	ASoundExt _inventorySound;

	TsAGE2Globals() { _onSelectItem = NULL; }
	virtual void reset();
	virtual void synchronize(Serializer &s);
};

extern Globals *g_globals;

#define GLOBALS (*::TsAGE::g_globals)
#define T2_GLOBALS (*((::TsAGE::TsAGE2Globals *)g_globals))
#define BF_GLOBALS (*((::TsAGE::BlueForce::BlueForceGlobals *)g_globals))
#define R2_GLOBALS (*((::TsAGE::Ringworld2::Ringworld2Globals *)g_globals))

// Note: Currently this can't be part of the g_globals structure, since it needs to be constructed
// prior to many of the fields in Globals execute their constructors
extern ResourceManager *g_resourceManager;


namespace BlueForce {

using namespace TsAGE;

enum Bookmark {
	bNone,
	bStartOfGame, bCalledToDomesticViolence, bArrestedGreen, bLauraToParamedics,
	bBookedGreen, bStoppedFrankie, bBookedFrankie, bBookedFrankieEvidence,
	bEndOfWorkDayOne, bTalkedToGrannyAboutSkipsCard, bLyleStoppedBy, bEndDayOne,
	bInspectionDone, bCalledToDrunkStop, bArrestedDrunk, bEndDayTwo,
	bFlashBackOne, bFlashBackTwo, bFlashBackThree, bDroppedOffLyle, bEndDayThree,
	bDoneWithIsland, bDoneAtLyles, bEndDayFour, bInvestigateBoat, bFinishedWGreen,
	bAmbushed, bAmbushOver, bEndOfGame
};

enum Flag {
	JAKE_FILE_COPIED, gunClean, onBike, onDuty, fShowedIdToKate, fLateToMarina,
	fCalledBackup, fWithLyle, gunDrawn, fBackupArrived340, fBriefedBackup,
	fGotAllSkip340, fToldToLeave340, fBackupIn350, fNetInBoat, fForbesWaiting,
	fWithCarter, fTalkedToTony, fMugOnKate, takenWeasel, gotTrailer450,
	showEugeneNapkin, showRapEugene, fMgrCallsWeasel, fCarterMetLyle,
	fGunLoaded, fLoadedSpare, showEugeneID, fRandomShot350, examinedFile810,
	shownLyleCrate1, shownLyleRapsheet, shownLyleDisk, shownLylePO,
	fCanDrawGun, fGotAutoWeapon, fGotBulletsFromDash, fShotSuttersDesk,
	greenTaken, fLateToDrunkStop, didDrunk, fSearchedTruck, seenFolder,
	showMugAround, frankInJail, fTalkedCarterDay3, fDecryptedBluePrints,
	fTalkedToDrunkInCar, fToldLyleOfSchedule, fTalkedShooterNoBkup,
	fTalkedDriverNoBkup, fDriverOutOfTruck, readGreenRights, readFrankRights,
	talkedToHarrisAboutDrunk, unlockBoat, fShootGoon, fBlowUpGoon,
	fTalkedToBarry, fTalkedToLarry, fLeftTraceIn920, fLeftTraceIn900,
	fBackupAt340, fShotNicoIn910, fGotPointsForTktBook, fGotPointsForMCard,
	fShowedBluePrint, fGotPointsForPunch, fGotPointsForBox, fGotPointsForBank,
	fGotPointsForCombo, fGotPointsForCoin, fGotPointsForCPU, fGotPointsForBoots,
	fGotPointsForCrate, fGotPointsForBlackCord, fGotPointsForGeneratorPlug,
	fGotPointsForFuseBoxPlug, fGotPointsForStartGenerator, fGotPointsForLightsOn,
	fGotPointsForOpeningDoor, fGotPointsForClosingDoor, fGotPointsForLightsOff,
	fGotPointsForGeneratorOff, fGotPointsForCordOnForklift, fGotPointsForCuffingNico,
	fGotPointsForCuffingDA, fGotPointsForSearchingNico, fGotPointsForSearchingDA,
	fLeftTraceIn910, fBookedGreenEvidence, fGotPointsForCleaningGun,
	fGotPointsForMemo, fGotPointsForFBI, fTookTrailerAmmo, fAlertedGreen355,
	fGotGreen355fTalkedToGrannyDay3, shownFax, beenToJRDay2, shownLyleCrate1Day1,
	fLyleOnIsland, iWasAmbushed, fGangInCar, fArrivedAtGangStop, ticketVW,
	f1015Marina, fCan1015Marina, f1015Frankie, fCan1015Frankie, f1015Drunk,
	fCan1015Drunk, f1027Marina, fCan1027Marina, f1027Frankie, fCan1027Frankie,
	f1027Drunk, fCan1027Drunk, f1035Marina, fCan1035Marina, f1035Frankie,
	fCan1035Frankie, f1035Drunk, fCan1035Drunk, f1097Marina, fCan1097Marina,
	f1097Frankie, fCan1097Frankie, f1097Drunk, fCan1097Drunk, f1098Marina,
	fCan1098Marina, f1098Frankie, fCan1098Frankie, f1098Drunk, fCan1098Drunk,
	fCuffedFrankie, fGotPointsForTrapDog, fGotPointsForUnlockGate,
	fGotPointsForUnlockWarehouse, fGotPointsForLockWarehouse, fGotPointsForLockGate,
	fGotPointsForFreeDog, fGotPointsForWhistleDog, fGivenNapkin, fCan1004Marina,
	fCan1004Drunk, fHasLeftDrunk, fHasDrivenFromDrunk, fCrateOpen, fSawGuns,
	hookPoints
};

class BlueForceGlobals: public TsAGE2Globals {
public:
	ASoundExt _sound1, _sound3;
	StripProxy _stripProxy;
	int _dayNumber;
	int _tonyDialogCtr;
	int _marinaWomanCtr;
	int _kateDialogCtr;
	int _v4CEB6;
	int _safeCombination;
	int _gateStatus;
	int _greenDay5TalkCtr;
	int _v4CEC4;
	int _v4CEC8;
	int _v4CECA;
	int _v4CECC;
	int8 _breakerBoxStatusArr[18];
	int _hiddenDoorStatus;
	int _nico910State;
	int _v4CEE4;
	int _v4CEE6;
	int _v4CEE8;
	int _deziTopic;
	int _deathReason;
	int _driveFromScene;
	int _driveToScene;
	int _v501F8;
	int _v501FA;
	int _v501FC;
	int _v5020C;
	int _v50696;
	uint8 _subFlagBitArr1;
	uint8 _subFlagBitArr2;
	int _v50CC2;
	int _v50CC4;
	int _v50CC6;
	int _v50CC8;
	int _v51C42;
	int _v51C44;
	Bookmark _bookmark;
	int _mapLocationId;
	int _clip1Bullets, _clip2Bullets;

	BlueForceGlobals();
	bool getHasBullets();

	virtual Common::String getClassName() { return "BFGlobals"; }
	virtual void reset();
	virtual void synchronize(Serializer &s);
	void set2Flags(int flagNum);
	bool removeFlag(int flagNum);
};

} // End of namespace BlueForce

namespace Ringworld2 {

#define SPEECH_TEXT 1
#define SPEECH_VOICE 2

#define k5A78C 15
#define k5A78D 16
#define k5A790 18
#define k5A791 17

class Ringworld2Globals: public TsAGE2Globals {
public:
	ASoundExt _sound1, _sound2, _sound3, _sound4;
	PlayStream _playStream;
	StripProxy _stripProxy;
	bool _v1000Flag;
	byte _v1000[0x1000];
	byte _palIndexList[10][256];
	int _insetUp;
	int _frameEdgeColour;	// _v421e
	Rect _v5589E;
	Rect _v558B6;
	int _v558C2;
	int _animationCtr;
	int _v565E1;
	int _v565E3;
	int _v565E5;
	int _v565E7;
	int _v565E9;
	int _v565EB;
	int _v565F5;
	int _v565F6;
	int _v565FA;
	int _v5657C;
	byte _v565AE;
	byte _v56605[14];
	int _v56613[76];
	byte _v566A4;
	byte _v566A5;
	int _v566A6;
	byte _v566A3;
	byte _v566A8;
	byte _v566A9;
	byte _v566AA;
	byte _v566AB[1000];
	int _v56A93;
	byte _v56A99;
	int _scene1925CurrLevel; //_v56A9C
	int _v56A9E;
	byte _v56AA0;
	byte _v56AA1;
	int _v56AA2;
	int _v56AA4;
	byte _v56AA6;
	byte _v56AA7;
	byte _v56AA8;
	int _v56AAB;
	int _scene180Mode;	// _v575f7
	int _v57709;
	int _v5780C;
	int _v5780E;
	int _v57810;
	int _v57C2C;
	int _speechSubtitles;
	byte _v565EC[5];
	byte _v565F1[4];
	byte _stripManager_lookupList[12];

	virtual void reset();
	virtual void synchronize(Serializer &s);
};

} // End of namespace Ringworld2

} // End of namespace TsAGE

#endif
