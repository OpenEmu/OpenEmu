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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "dreamweb/sound.h"
#include "dreamweb/dreamweb.h"

namespace DreamWeb {


typedef void (DreamWebEngine::*UseCallback)(void);

// Note: The callback pointer has been placed before the
// ID to keep MSVC happy (otherwise, it throws warnings
// that alignment of a member was sensitive to packing)
struct UseListEntry {
	UseCallback callback;
	const char *id;
};

void DreamWebEngine::useRoutine() {

	static const UseListEntry kUseList[] = {
		{ &DreamWebEngine::useMon,                  "NETW" },
		{ &DreamWebEngine::useElevator1,            "ELVA" },
		{ &DreamWebEngine::useElevator2,            "ELVB" },
		{ &DreamWebEngine::useElevator3,            "ELVC" },
		{ &DreamWebEngine::useElevator4,            "ELVE" },
		{ &DreamWebEngine::useElevator5,            "ELVF" },
		{ &DreamWebEngine::useChurchGate,           "CGAT" },
		{ &DreamWebEngine::useStereo,               "REMO" },
		{ &DreamWebEngine::useButtonA,              "BUTA" },
		{ &DreamWebEngine::useWinch,                "CBOX" },
		{ &DreamWebEngine::useLighter,              "LITE" },
		{ &DreamWebEngine::usePlate,                "PLAT" },
		{ &DreamWebEngine::useControl,              "LIFT" },
		{ &DreamWebEngine::useWire,                 "WIRE" },
		{ &DreamWebEngine::useHandle,               "HNDL" },
		{ &DreamWebEngine::useHatch,                "HACH" },
		{ &DreamWebEngine::useElvDoor,              "DOOR" },
		{ &DreamWebEngine::useCashCard,             "CSHR" },
		{ &DreamWebEngine::useGun,                  "GUNA" },
		{ &DreamWebEngine::useCardReader1,          "CRAA" },
		{ &DreamWebEngine::useCardReader2,          "CRBB" },
		{ &DreamWebEngine::useCardReader3,          "CRCC" },
		{ &DreamWebEngine::sitDownInBar,            "SEAT" },
		{ &DreamWebEngine::useMenu,                 "MENU" },
		{ &DreamWebEngine::useCooker,               "COOK" },
		{ &DreamWebEngine::callHotelLift,           "ELCA" },
		{ &DreamWebEngine::callEdensLift,           "EDCA" },
		{ &DreamWebEngine::callEdensDLift,          "DDCA" },
		{ &DreamWebEngine::useAltar,                "ALTR" },
		{ &DreamWebEngine::openHotelDoor,           "LOKA" },
		{ &DreamWebEngine::openHotelDoor2,          "LOKB" },
		{ &DreamWebEngine::openLouis,               "ENTA" },
		{ &DreamWebEngine::openRyan,                "ENTB" },
		{ &DreamWebEngine::openPoolBoss,            "ENTE" },
		{ &DreamWebEngine::openYourNeighbour,       "ENTC" },
		{ &DreamWebEngine::openEden,                "ENTD" },
		{ &DreamWebEngine::openSarters,             "ENTH" },
		{ &DreamWebEngine::wearWatch,               "WWAT" },
		{ &DreamWebEngine::usePoolReader,           "POOL" },
		{ &DreamWebEngine::wearShades,              "WSHD" },
		{ &DreamWebEngine::grafittiDoor,            "GRAF" },
		{ &DreamWebEngine::trapDoor,                "TRAP" },
		{ &DreamWebEngine::edensCDPlayer,           "CDPE" },
		{ &DreamWebEngine::openTVDoor,              "DLOK" },
		{ &DreamWebEngine::useHole,                 "HOLE" },
		{ &DreamWebEngine::useDryer,                "DRYR" },
		{ &DreamWebEngine::useChurchHole,           "HOLY" },
		{ &DreamWebEngine::useWall,                 "WALL" },
		{ &DreamWebEngine::useDiary,                "BOOK" },
		{ &DreamWebEngine::useAxe,                  "AXED" },
		{ &DreamWebEngine::useShield,               "SHLD" },
		{ &DreamWebEngine::useRailing,              "BCNY" },
		{ &DreamWebEngine::useCoveredBox,           "LIDC" },
		{ &DreamWebEngine::useClearBox,             "LIDU" },
		{ &DreamWebEngine::useOpenBox,              "LIDO" },
		{ &DreamWebEngine::usePipe,                 "PIPE" },
		{ &DreamWebEngine::useBalcony,              "BALC" },
		{ &DreamWebEngine::useWindow,               "WIND" },
		{ &DreamWebEngine::viewFolder,              "PAPR" },
		{ &DreamWebEngine::useTrainer,              "UWTA" },
		{ &DreamWebEngine::useTrainer,              "UWTB" },
		{ &DreamWebEngine::enterSymbol,             "STAT" },
		{ &DreamWebEngine::openTomb,                "TLID" },
		{ &DreamWebEngine::useSlab,                 "SLAB" },
		{ &DreamWebEngine::useCart,                 "CART" },
		{ &DreamWebEngine::useFullCart,             "FCAR" },
		{ &DreamWebEngine::slabDoorA,               "SLBA" },
		{ &DreamWebEngine::slabDoorB,               "SLBB" },
		{ &DreamWebEngine::slabDoorC,               "SLBC" },
		{ &DreamWebEngine::slabDoorD,               "SLBD" },
		{ &DreamWebEngine::slabDoorE,               "SLBE" },
		{ &DreamWebEngine::slabDoorF,               "SLBF" },
		{ &DreamWebEngine::usePlinth,               "PLIN" },
		{ &DreamWebEngine::useLadder,               "LADD" },
		{ &DreamWebEngine::useLadderB,              "LADB" },
		{ &DreamWebEngine::chewy,                   "GUMA" },
		{ &DreamWebEngine::wheelSound,              "SQEE" },
		{ &DreamWebEngine::runTap,                  "TAPP" },
		{ &DreamWebEngine::playGuitar,              "GUIT" },
		{ &DreamWebEngine::hotelControl,            "CONT" },
		{ &DreamWebEngine::hotelBell,               "BELL" },
	};

	if (_realLocation >= 50) {
		if (_pointerPower == 0)
			return;
		_pointerPower = 0;
	}

	uint8 dummy;
	void *obj = getAnyAd(&dummy, &dummy);

	for (size_t i = 0; i < sizeof(kUseList)/sizeof(UseListEntry); ++i) {
		const UseListEntry &entry = kUseList[i];
		if (objectMatches(obj, entry.id)) {
			(this->*entry.callback)();
			return;
		}
	}

	delPointer();
	const uint8 *obText = getObTextStart();
	if (findNextColon(&obText) != 0) {
		if (findNextColon(&obText) != 0) {
			if (*obText != 0) {
				useText(obText);
				hangOnP(400);
				putBackObStuff();
				return;
			}
		}
	}

	createPanel();
	showPanel();
	showMan();
	showExit();
	obIcons();
	printMessage(33, 100, 63, 241, true);
	workToScreenM();
	hangOnP(50);
	putBackObStuff();
	_commandType = 255;
}

void DreamWebEngine::useText(const uint8 *string) {
	createPanel();
	showPanel();
	showMan();
	showExit();
	obIcons();
	printDirect(string, 36, 104, 241, true);
	workToScreenM();
}

void DreamWebEngine::showFirstUse() {
	const uint8 *obText = getObTextStart();
	findNextColon(&obText);
	findNextColon(&obText);
	useText(obText);
	hangOnP(400);
}

void DreamWebEngine::showSecondUse() {
	const uint8 *obText = getObTextStart();
	findNextColon(&obText);
	findNextColon(&obText);
	findNextColon(&obText);
	useText(obText);
	hangOnP(400);
}

void DreamWebEngine::edensCDPlayer() {
	showFirstUse();
	_vars._watchingTime = 18 * 2;
	_vars._reelToWatch = 25;
	_vars._endWatchReel = 42;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_getBack = 1;
}

void DreamWebEngine::hotelBell() {
	_sound->playChannel1(12);
	showFirstUse();
	putBackObStuff();
}

void DreamWebEngine::playGuitar() {
	_sound->playChannel1(14);
	showFirstUse();
	putBackObStuff();
}

void DreamWebEngine::useElevator1() {
	showFirstUse();
	selectLocation();
	_getBack = 1;
}

void DreamWebEngine::useElevator2() {
	showFirstUse();

	if (_vars._location == 23)	// In pool hall
		_newLocation = 31;
	else
		_newLocation = 23;

	_vars._countToClose = 20;
	_vars._countToOpen = 0;
	_vars._watchingTime = 80;
	_getBack = 1;
}

void DreamWebEngine::useElevator3() {
	showFirstUse();
	_vars._countToClose = 20;
	_newLocation = 34;
	_vars._reelToWatch = 46;
	_vars._endWatchReel = 63;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_vars._watchingTime = 80;
	_getBack = 1;
}

void DreamWebEngine::useElevator4() {
	showFirstUse();
	_vars._reelToWatch = 0;
	_vars._endWatchReel = 11;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_vars._countToClose = 20;
	_vars._watchingTime = 80;
	_getBack = 1;
	_newLocation = 24;
}

void DreamWebEngine::useElevator5() {
	placeSetObject(4);
	removeSetObject(0);
	_newLocation = 20;
	_vars._watchingTime = 80;
	_vars._liftFlag = 1;
	_vars._countToClose = 8;
	_getBack = 1;
}

void DreamWebEngine::useHatch() {
	showFirstUse();
	_newLocation = 40;
	_getBack = 1;
}

void DreamWebEngine::wheelSound() {
	_sound->playChannel1(17);
	showFirstUse();
	putBackObStuff();
}

void DreamWebEngine::callHotelLift() {
	_sound->playChannel1(12);
	showFirstUse();
	_vars._countToOpen = 8;
	_getBack = 1;
	_destination = 5;
	_finalDest = 5;
	autoSetWalk();
	turnPathOn(4);
}

void DreamWebEngine::useShield() {
	if (_realLocation != 20 || _vars._combatCount == 0) {
		// Not in Sart room
		showFirstUse();
		putBackObStuff();
	} else {
		_vars._lastWeapon = 3;
		showSecondUse();
		_getBack = 1;
		_vars._progressPoints++;
		removeObFromInv();
	}
}

void DreamWebEngine::useCoveredBox() {
	_vars._progressPoints++;
	showFirstUse();
	_vars._watchingTime = 50;
	_vars._reelToWatch = 41;
	_vars._endWatchReel = 66;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_getBack = 1;
}

void DreamWebEngine::useRailing() {
	showFirstUse();
	_vars._watchingTime = 80;
	_vars._reelToWatch = 0;
	_vars._endWatchReel = 30;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_getBack = 1;
	_vars._manDead = 4;
}

void DreamWebEngine::wearWatch() {
	if (_vars._watchOn == 1) {
		// Already wearing watch
		showSecondUse();
		putBackObStuff();
	} else {
		showFirstUse();
		_vars._watchOn = 1;
		_getBack = 1;
		uint8 dummy;
		makeWorn((DynObject *)getAnyAd(&dummy, &dummy));
	}
}

void DreamWebEngine::wearShades() {
	if (_vars._shadesOn == 1) {
		// Already wearing shades
		showSecondUse();
		putBackObStuff();
	} else {
		_vars._shadesOn = 1;
		showFirstUse();
		_getBack = 1;
		uint8 dummy;
		makeWorn((DynObject *)getAnyAd(&dummy, &dummy));
	}
}

void DreamWebEngine::useChurchHole() {
	showFirstUse();
	_getBack = 1;
	_vars._watchingTime = 28;
	_vars._reelToWatch = 13;
	_vars._endWatchReel = 26;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
}

void DreamWebEngine::sitDownInBar() {
	if (_vars._watchMode != 0xFF) {
		// Sat down
		showSecondUse();
		putBackObStuff();
	} else {
		showFirstUse();
		_vars._watchingTime = 50;
		_vars._reelToWatch = 55;
		_vars._endWatchReel = 71;
		_vars._reelToHold = 73;
		_vars._endOfHoldReel = 83;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		_getBack = 1;
	}
}

void DreamWebEngine::useDryer() {
	_sound->playChannel1(12);
	showFirstUse();
	_getBack = 1;
}

void DreamWebEngine::useBalcony() {
	showFirstUse();
	turnPathOn(6);
	turnPathOff(0);
	turnPathOff(1);
	turnPathOff(2);
	turnPathOff(3);
	turnPathOff(4);
	turnPathOff(5);
	_vars._progressPoints++;
	_mansPath = 6;
	_destination = 6;
	_finalDest = 6;
	findXYFromPath();
	switchRyanOff();
	_resetManXY = 1;
	_vars._watchingTime = 30 * 2;
	_vars._reelToWatch = 183;
	_vars._endWatchReel = 212;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_getBack = 1;
}

void DreamWebEngine::useWindow() {
	if (_mansPath != 6) {
		// Not on balcony
		showSecondUse();
		putBackObStuff();
	} else {
		_vars._progressPoints++;
		showFirstUse();
		_newLocation = 29;
		_getBack = 1;
	}
}

void DreamWebEngine::trapDoor() {
	_vars._progressPoints++;
	showFirstUse();
	switchRyanOff();
	_vars._watchingTime = 20 * 2;
	_vars._reelToWatch = 181;
	_vars._endWatchReel = 197;
	_newLocation = 26;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_getBack = 1;
}

void DreamWebEngine::callEdensLift() {
	showFirstUse();
	_vars._countToOpen = 8;
	_getBack = 1;
	turnPathOn(2);
}

void DreamWebEngine::callEdensDLift() {
	if (_vars._liftFlag == 1) {
		// Eden's D here
		showSecondUse();
		putBackObStuff();
	} else {
		showFirstUse();
		_vars._countToOpen = 8;
		_getBack = 1;
		turnPathOn(2);
	}
}

void DreamWebEngine::openYourNeighbour() {
	enterCode(255, 255, 255, 255);
	_getBack = 1;
}

void DreamWebEngine::openRyan() {
	enterCode(5, 1, 0, 6);
	_getBack = 1;
}

void DreamWebEngine::openPoolBoss() {
	enterCode(5, 2, 2, 2);
	_getBack = 1;
}

void DreamWebEngine::openEden() {
	enterCode(2, 8, 6, 5);
	_getBack = 1;
}

void DreamWebEngine::openSarters() {
	enterCode(7, 8, 3, 3);
	_getBack = 1;
}

void DreamWebEngine::openLouis() {
	enterCode(5, 2, 3, 8);
	_getBack = 1;
}


void DreamWebEngine::useWall() {
	showFirstUse();

	if (_mansPath != 3) {
		_vars._watchingTime = 30*2;
		_vars._reelToWatch = 2;
		_vars._endWatchReel = 31;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		_getBack = 1;
		turnPathOn(3);
		turnPathOn(4);
		turnPathOff(0);
		turnPathOff(1);
		turnPathOff(2);
		turnPathOff(5);
		_mansPath = 3;
		_finalDest = 3;
		findXYFromPath();
		_resetManXY = 1;
		switchRyanOff();
	} else {
		// Go back over
		_vars._watchingTime = 30 * 2;
		_vars._reelToWatch = 34;
		_vars._endWatchReel = 60;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		_getBack = 1;
		turnPathOff(3);
		turnPathOff(4);
		turnPathOn(0);
		turnPathOn(1);
		turnPathOn(2);
		turnPathOn(5);
		_mansPath = 5;
		_finalDest = 5;
		findXYFromPath();
		_resetManXY = 1;
		switchRyanOff();
	}
}

void DreamWebEngine::useLadder() {
	showFirstUse();
	_mapX = _mapX - 11;
	findRoomInLoc();
	_facing = 6;
	_turnToFace = 6;
	_mansPath = 0;
	_destination = 0;
	_finalDest = 0;
	findXYFromPath();
	_resetManXY = 1;
	_getBack = 1;
}

void DreamWebEngine::useLadderB() {
	showFirstUse();
	_mapX = _mapX + 11;
	findRoomInLoc();
	_facing = 2;
	_turnToFace = 2;
	_mansPath = 1;
	_destination = 1;
	_finalDest = 1;
	findXYFromPath();
	_resetManXY = 1;
	_getBack = 1;
}

void DreamWebEngine::slabDoorA() {
	showFirstUse();
	_getBack = 1;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_vars._reelToWatch = 13;
	if (_vars._dreamNumber != 3) {
		// Wrong
		_vars._watchingTime = 40;
		_vars._endWatchReel = 34;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
	} else {
		_vars._progressPoints++;
		_vars._watchingTime = 60;
		_vars._endWatchReel = 42;
		_newLocation = 47;
	}
}

void DreamWebEngine::slabDoorB() {
	if (_vars._dreamNumber != 1) {
		// Wrong
		showFirstUse();
		_getBack = 1;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		_vars._reelToWatch = 44;
		_vars._watchingTime = 40;
		_vars._endWatchReel = 63;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
	} else {
		if (!isRyanHolding("SHLD")) {
			// No crystal
			showPuzText(44, 200);
			putBackObStuff();
		} else {
			// Got crystal
			showFirstUse();
			_vars._progressPoints++;
			_getBack = 1;
			_vars._watchSpeed = 1;
			_vars._speedCount = 1;
			_vars._reelToWatch = 44;
			_vars._watchingTime = 60;
			_vars._endWatchReel = 71;
			_newLocation = 47;
		}
	}
}

void DreamWebEngine::slabDoorC() {
	showFirstUse();
	_getBack = 1;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_vars._reelToWatch = 108;
	if (_vars._dreamNumber != 4) {
		// Wrong
		_vars._watchingTime = 40;
		_vars._endWatchReel = 127;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
	} else {
		_vars._progressPoints++;
		_vars._watchingTime = 60;
		_vars._endWatchReel = 135;
		_newLocation = 47;
	}
}

void DreamWebEngine::slabDoorD() {
	showFirstUse();
	_getBack = 1;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_vars._reelToWatch = 75;
	if (_vars._dreamNumber != 0) {
		// Wrong
		_vars._watchingTime = 40;
		_vars._endWatchReel = 94;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
	} else {
		_vars._progressPoints++;
		_vars._watchingTime = 60;
		_vars._endWatchReel = 102;
		_newLocation = 47;
	}
}

void DreamWebEngine::slabDoorE() {
	showFirstUse();
	_getBack = 1;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_vars._reelToWatch = 141;
	if (_vars._dreamNumber != 5) {
		// Wrong
		_vars._watchingTime = 40;
		_vars._endWatchReel = 160;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
	} else {
		_vars._progressPoints++;
		_vars._watchingTime = 60;
		_vars._endWatchReel = 168;
		_newLocation = 47;
	}
}

void DreamWebEngine::slabDoorF() {
	showFirstUse();
	_getBack = 1;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_vars._reelToWatch = 171;
	if (_vars._dreamNumber != 2) {
		// Wrong
		_vars._watchingTime = 40;
		_vars._endWatchReel = 189;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
	} else {
		_vars._progressPoints++;
		_vars._watchingTime = 60;
		_vars._endWatchReel = 197;
		_newLocation = 47;
	}
}

bool DreamWebEngine::defaultUseHandler(const char *id) {
	if (_withObject == 255) {
		withWhat();
		return true;	// event handled
	}

	if (!compare(_withObject, _withType, id)) {
		// Wrong item
		showPuzText(14, 300);
		putBackObStuff();
		return true;	// event handled
	}

	return false;	// continue with the original event
}

void DreamWebEngine::useChurchGate() {
	if (defaultUseHandler("CUTT"))
		return;

	// Cut gate
	showFirstUse();
	_vars._watchingTime = 64 * 2;
	_vars._reelToWatch = 4;
	_vars._endWatchReel = 70;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_getBack = 1;
	_vars._progressPoints++;
	turnPathOn(3);
	if (_vars._aideDead != 0)
		turnPathOn(2);	// Open church
}

void DreamWebEngine::useGun() {

	if (_objectType != kExObjectType) {
		// gun is not taken
		showSecondUse();
		putBackObStuff();

	} else if (_realLocation == 22) {
		// in pool room
		showPuzText(34, 300);
		_vars._lastWeapon = 1;
		_vars._combatCount = 39;
		_getBack = 1;
		_vars._progressPoints++;

	} else if (_realLocation == 25) {
		// helicopter
		showPuzText(34, 300);
		_vars._lastWeapon = 1;
		_vars._combatCount = 19;
		_getBack = 1;
		_vars._dreamNumber = 2;
		_vars._roomAfterDream = 38;
		_vars._sartainDead = 1;
		_vars._progressPoints++;

	} else if (_realLocation == 27) {
		// in rock room
		showPuzText(46, 300);
		_pointerMode = 2;
		_vars._rockstarDead = 1;
		_vars._lastWeapon = 1;
		_vars._newsItem = 1;
		_getBack = 1;
		_vars._roomAfterDream = 32;
		_vars._dreamNumber = 0;
		_vars._progressPoints++;

	} else if (_realLocation == 8 && _mapX == 22 && _mapY == 40
	    && !isSetObOnMap(92) && _mansPath != 9) {
		// by studio
		_destination = 9;
		_finalDest = 9;
		autoSetWalk();
		_vars._lastWeapon = 1;
		_getBack = 1;
		_vars._progressPoints++;

	} else if (_realLocation == 6 && _mapX == 11 && _mapY == 20
	    && isSetObOnMap(5)) {
		// sarters
		_destination = 1;
		_finalDest = 1;
		autoSetWalk();
		removeSetObject(5);
		placeSetObject(6);
		turnAnyPathOn(1, _roomNum - 1);
		_vars._liftFlag = 1;
		_vars._watchingTime = 40*2;
		_vars._reelToWatch = 4;
		_vars._endWatchReel = 43;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		_getBack = 1;
		_vars._progressPoints++;

	} else if (_realLocation == 29) {
		// aide
		_getBack = 1;
		resetLocation(13);
		setLocation(12);
		_destPos = 12;
		_destination = 2;
		_finalDest = 2;
		autoSetWalk();
		_vars._watchingTime = 164*2;
		_vars._reelToWatch = 3;
		_vars._endWatchReel = 164;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		_vars._aideDead = 1;
		_vars._dreamNumber = 3;
		_vars._roomAfterDream = 33;
		_vars._progressPoints++;

	} else if (_realLocation == 23 && _mapX == 0 && _mapY == 50) {
		// with boss
		if (_mansPath != 5) {
			_destination = 5;
			_finalDest = 5;
			autoSetWalk();
		}
		_vars._lastWeapon = 1;
		_getBack = 1;

	} else if (_realLocation == 8 && _mapX == 11 && _mapY == 10) {
		// tv soldier
		if (_mansPath != 2) {
			_destination = 2;
			_finalDest = 2;
			autoSetWalk();
		}
		_vars._lastWeapon = 1;
		_getBack = 1;

	} else {
		showFirstUse();
		putBackObStuff();
	}
}

void DreamWebEngine::useFullCart() {
	_vars._progressPoints++;
	turnAnyPathOn(2, _roomNum + 6);
	_mansPath = 4;
	_facing = 4;
	_turnToFace = 4;
	_finalDest = 4;
	findXYFromPath();
	_resetManXY = 1;
	showFirstUse();
	_vars._watchingTime = 72 * 2;
	_vars._reelToWatch = 58;
	_vars._endWatchReel = 142;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_getBack = 1;
}

void DreamWebEngine::useClearBox() {
	if (defaultUseHandler("RAIL"))
		return;

	// Open box
	_vars._progressPoints++;
	showFirstUse();
	_vars._watchingTime = 80;
	_vars._reelToWatch = 67;
	_vars._endWatchReel = 105;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_getBack = 1;
}

void DreamWebEngine::openTVDoor() {
	if (defaultUseHandler("ULOK"))
		return;

	// Key on TV
	showFirstUse();
	_vars._lockStatus = 0;
	_getBack = 1;
}

void DreamWebEngine::usePlate() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "SCRW")) {
		// Unscrew plate
		_sound->playChannel1(20);
		showFirstUse();
		placeSetObject(28);
		placeSetObject(24);
		removeSetObject(25);
		placeFreeObject(0);
		_vars._progressPoints++;
		_getBack = 1;
	} else if (compare(_withObject, _withType, "KNFE")) {
		// Tried knife
		showPuzText(54, 300);
		putBackObStuff();
	} else {
		// Wrong item
		showPuzText(14, 300);
		putBackObStuff();
	}
}

void DreamWebEngine::usePlinth() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (!compare(_withObject, _withType, "DKEY")) {
		// Wrong key
		showFirstUse();
		putBackObStuff();
	} else {
		_vars._progressPoints++;
		showSecondUse();
		_vars._watchingTime = 220;
		_vars._reelToWatch = 0;
		_vars._endWatchReel = 104;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		_getBack = 1;
		_newLocation = _vars._roomAfterDream;
	}
}

void DreamWebEngine::useElvDoor() {
	if (defaultUseHandler("AXED"))
		return;

	// Axe on door
	showPuzText(15, 300);
	_vars._progressPoints++;
	_vars._watchingTime = 46 * 2;
	_vars._reelToWatch = 31;
	_vars._endWatchReel = 77;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_getBack = 1;
}

void DreamWebEngine::useObject() {
	_withObject = 255;

	if (_commandType != 229) {
		_commandType = 229;
		commandWithOb(51, _objectType, _command);
	}

	if (_mouseButton == _oldButton)
		return;	// nouse

	if (_mouseButton & 1)
		useRoutine();
}

void DreamWebEngine::useWinch() {
	uint16 contentIndex = checkInside(40, 1);
	if (contentIndex == kNumexobjects || !compare(contentIndex, kExObjectType, "FUSE")) {
		// No winch
		showFirstUse();
		putBackObStuff();
		return;
	}

	_vars._watchingTime = 217 * 2;
	_vars._reelToWatch = 0;
	_vars._endWatchReel = 217;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_destPos = 1;
	_newLocation = 45;
	_vars._dreamNumber = 1;
	_vars._roomAfterDream = 44;
	_vars._generalDead = 1;
	_vars._newsItem = 2;
	_getBack = 1;
	_vars._progressPoints++;
}

void DreamWebEngine::useCart() {
	if (defaultUseHandler("ROCK"))
		return;

	DynObject *exObject = getExAd(_withObject);
	exObject->mapad[0] = 0;
	removeSetObject(_command);
	placeSetObject(_command + 1);
	_vars._progressPoints++;
	_sound->playChannel1(17);
	showFirstUse();
	_getBack = 1;
}

void DreamWebEngine::useTrainer() {
	uint8 dummy;
	DynObject *object = (DynObject *)getAnyAd(&dummy, &dummy);
	if (object->mapad[0] != 4) {
		notHeldError();
	} else {
		_vars._progressPoints++;
		makeWorn(object);
		showSecondUse();
		putBackObStuff();
	}
}

void DreamWebEngine::chewy() {
	// Chewing a gum
	showFirstUse();
	uint8 dummy;
	DynObject *object = (DynObject *)getAnyAd(&dummy, &dummy);
	object->mapad[0] = 255;
	_getBack = 1;
}

void DreamWebEngine::useHole() {
	if (defaultUseHandler("HNDA"))
		return;

	showFirstUse();
	removeSetObject(86);
	DynObject *exObject = getExAd(_withObject);
	exObject->mapad[0] = 255;
	_vars._canMoveAltar = 1;
	_getBack = 1;
}

void DreamWebEngine::openHotelDoor() {
	if (defaultUseHandler("KEYA"))
		return;

	_sound->playChannel1(16);
	showFirstUse();
	_vars._lockStatus = 0;
	_getBack = 1;
}

void DreamWebEngine::openHotelDoor2() {
	if (defaultUseHandler("KEYA"))
		return;

	_sound->playChannel1(16);
	showFirstUse();
	putBackObStuff();
}

void DreamWebEngine::grafittiDoor() {
	if (defaultUseHandler("APEN"))
		return;

	showFirstUse();
	putBackObStuff();
}

void DreamWebEngine::usePoolReader() {
	if (defaultUseHandler("MEMB"))
		return;

	if (_vars._talkedToAttendant != 1) {
		// Can't open pool
		showSecondUse();
		putBackObStuff();
	} else {
		_sound->playChannel1(17);
		showFirstUse();
		_vars._countToOpen = 6;
		_getBack = 1;
	}
}

void DreamWebEngine::useCardReader1() {
	if (defaultUseHandler("CSHR"))
		return;

	if (_vars._talkedToSparky == 0) {
		// Not yet
		showFirstUse();
		putBackObStuff();
	} else if (_vars._card1Money != 0) {
		// No cash
		showPuzText(17, 300);
		putBackObStuff();
	} else {
		// Get cash
		_sound->playChannel1(16);
		showPuzText(18, 300);
		_vars._progressPoints++;
		_vars._card1Money = 12432;
		_getBack = 1;
	}
}

void DreamWebEngine::useCardReader2() {
	if (defaultUseHandler("CSHR"))
		return;

	if (_vars._talkedToBoss == 0) {
		// Haven't talked to boss
		showFirstUse();
		putBackObStuff();
	} else if (_vars._card1Money == 0) {
		// No cash
		showPuzText(20, 300);
		putBackObStuff();
	} else if (_vars._gunPassFlag == 2) {
		// Already got new
		showPuzText(22, 300);
		putBackObStuff();
	} else {
		_sound->playChannel1(18);
		showPuzText(19, 300);
		placeSetObject(94);
		_vars._gunPassFlag = 1;
		_vars._card1Money -= 2000;
		_vars._progressPoints++;
		_getBack = 1;
	}
}

void DreamWebEngine::useCardReader3() {
	if (defaultUseHandler("CSHR"))
		return;

	if (_vars._talkedToRecep == 0) {
		// Haven't talked to receptionist
		showFirstUse();
		putBackObStuff();
	} else if (_vars._cardPassFlag != 0) {
		// Already used it
		showPuzText(26, 300);
		putBackObStuff();
	} else {
		_sound->playChannel1(16);
		showPuzText(25, 300);
		_vars._progressPoints++;
		_vars._card1Money -= 8300;
		_vars._cardPassFlag = 1;
		_getBack = 1;
	}
}

void DreamWebEngine::useLighter() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (!compare(_withObject, _withType, "SMKE")) {
		showFirstUse();
		putBackObStuff();
	} else {
		showPuzText(9, 300);
		DynObject *withObj = getExAd(_withObject);
		withObj->mapad[0] = 255;
		_getBack = 1;
	}
}

void DreamWebEngine::useWire() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "KNFE")) {
		removeSetObject(51);
		placeSetObject(52);
		showPuzText(11, 300);
		_vars._progressPoints++;
		_getBack = 1;
		return;
	}

	if (compare(_withObject, _withType, "AXED")) {
		showPuzText(16, 300);
		putBackObStuff();
		return;
	}

	showPuzText(14, 300);
	putBackObStuff();
}

void DreamWebEngine::openTomb() {
	_vars._progressPoints++;
	showFirstUse();
	_vars._watchingTime = 35 * 2;
	_vars._reelToWatch = 1;
	_vars._endWatchReel = 33;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	_getBack = 1;
}

void DreamWebEngine::hotelControl() {
	if (_realLocation != 21 || _mapX != 33)
		showSecondUse();	// Not right control
	else
		showFirstUse();

	putBackObStuff();
}

void DreamWebEngine::useCooker() {
	if (checkInside(_command, _objectType) == kNumexobjects)
		showFirstUse();
	else
		showSecondUse();	// Food inside

	putBackObStuff();
}

void DreamWebEngine::placeFreeObject(uint8 index) {
	findOrMake(index, 0, 1);
	getFreeAd(index)->mapad[0] = 0;
}

void DreamWebEngine::removeFreeObject(uint8 index) {
	getFreeAd(index)->mapad[0] = 0xFF;
}

void DreamWebEngine::useControl() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "KEYA")) {	// Right key
		_sound->playChannel1(16);
		if (_vars._location == 21) {	// Going down
			showPuzText(3, 300);
			_newLocation = 30;
		} else {
			showPuzText(0, 300);
			_newLocation = 21;
		}

		_vars._countToClose = 8;
		_vars._countToOpen = 0;
		_vars._watchingTime = 80;
		_getBack = 1;
		return;
	}

	if (_realLocation == 21) {
		if (compare(_withObject, _withType, "KNFE")) {
			// Jimmy controls
			placeSetObject(50);
			placeSetObject(51);
			placeSetObject(26);
			placeSetObject(30);
			removeSetObject(16);
			removeSetObject(17);
			_sound->playChannel1(14);
			showPuzText(10, 300);
			_vars._progressPoints++;
			_getBack = 1;
		} else if (compare(_withObject, _withType, "AXED")) {
			// Axe on controls
			showPuzText(16, 300);
			_vars._progressPoints++;
			putBackObStuff();
		} else {
			// Balls
			showFirstUse();
			putBackObStuff();
		}
	} else {
		// Balls
		showFirstUse();
		putBackObStuff();
	}
}

void DreamWebEngine::useSlab() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (!compare(_withObject, _withType, "JEWL")) {
		showPuzText(14, 300);
		putBackObStuff();
		return;
	}

	DynObject *exObject = getExAd(_withObject);
	exObject->mapad[0] = 0;

	removeSetObject(_command);
	placeSetObject(_command + 1);
	if (_command + 1 == 54) {
		// Last slab
		turnPathOn(0);
		_vars._watchingTime = 22;
		_vars._reelToWatch = 35;
		_vars._endWatchReel = 48;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
	}

	_vars._progressPoints++;
	showFirstUse();
	_getBack = 1;
}

void DreamWebEngine::usePipe() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "CUPE")) {
		// Fill cup
		showPuzText(36, 300);
		putBackObStuff();
		DynObject *exObject = getExAd(_withObject);
		exObject->objId[3] = 'F'-'A';	// CUPE (empty cup) -> CUPF (full cup)
		return;
	} else if (compare(_withObject, _withType, "CUPF")) {
		// Already full
		showPuzText(35, 300);
		putBackObStuff();
	} else {
		showPuzText(14, 300);
		putBackObStuff();
	}
}

void DreamWebEngine::useOpenBox() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "CUPF")) {
		// Destroy open box
		_vars._progressPoints++;
		showPuzText(37, 300);
		DynObject *exObject = getExAd(_withObject);
		exObject->objId[3] = 'E'-'A';	// CUPF (full cup) -> CUPE (empty cup)
		_vars._watchingTime = 140;
		_vars._reelToWatch = 105;
		_vars._endWatchReel = 181;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		turnPathOn(4);
		_getBack = 1;
		return;
	}

	if (compare(_withObject, _withType, "CUPE")) {
		// Open box wrong
		showPuzText(38, 300);
		putBackObStuff();
		return;
	}

	showFirstUse();
}

void DreamWebEngine::runTap() {
	if (_withObject == 255) {
		withWhat();
		return;
	}

	if (compare(_withObject, _withType, "CUPE")) {
		// Fill cup from tap
		DynObject *exObject = getExAd(_withObject);
		exObject->objId[3] = 'F'-'A';	// CUPE (empty cup) -> CUPF (full cup)
		_sound->playChannel1(8);
		showPuzText(57, 300);
		putBackObStuff();
		return;
	}

	if (compare(_withObject, _withType, "CUPF")) {
		// Cup from tap full
		showPuzText(58, 300);
		putBackObStuff();
		return;
	}

	showPuzText(56, 300);
	putBackObStuff();
}

void DreamWebEngine::useAxe() {
	if (_realLocation != 22) {
		// Not in pool
		showFirstUse();
		return;
	}

	if (_mapY == 10) {
		// Axe on door
		showPuzText(15, 300);
		_vars._progressPoints++;
		_vars._watchingTime = 46*2;
		_vars._reelToWatch = 31;
		_vars._endWatchReel = 77;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		_getBack = 1;
		return;
	}

	showSecondUse();
	_vars._progressPoints++;
	_vars._lastWeapon = 2;
	_getBack = 1;
	removeObFromInv();
}

void DreamWebEngine::useHandle() {
	SetObject *object = getSetAd(findSetObject("CUTW"));
	if (object->mapad[0] == 255) {
		// Wire not cut
		showPuzText(12, 300);
	} else {
		// Wire has been cut
		showPuzText(13, 300);
		_newLocation = 22;
	}

	_getBack = 1;
}

void DreamWebEngine::useAltar() {
	if (findExObject("CNDA") == 114 || findExObject("CNDB") == 114) {
		// Things on altar
		showFirstUse();
		_getBack = 1;
		return;
	}

	if (_vars._canMoveAltar == 1) {
		// Move altar
		_vars._progressPoints++;
		showSecondUse();
		_vars._watchingTime = 160;
		_vars._reelToWatch = 81;
		_vars._endWatchReel = 174;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		setupTimedUse(47, 32, 98, 52, 76);
		_getBack = 1;
	} else {
		showPuzText(23, 300);
		_getBack = 1;
	}
}

void DreamWebEngine::withWhat() {
	uint8 commandLine[64] = "OBJECT NAME ONE                         ";

	createPanel();
	showPanel();
	showMan();
	showExit();

	copyName(_objectType, _command, commandLine);
	printMessage2(100, 21, 63, 200, false, 2);
	uint16 x = _lastXPos + 5;
	printDirect(commandLine, x, 21, 220, false);
	printMessage2(_lastXPos + 5, 21, 63, 200, false, 3);

	fillRyan();
	_commandType = 255;
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	_invOpen = 2;
}

void DreamWebEngine::notHeldError() {
	createPanel();
	showPanel();
	showMan();
	showExit();
	obIcons();
	printMessage2(64, 100, 63, 200 + 1, true, 1);
	workToScreenM();
	hangOnP(50);
	putBackObStuff();
}

void DreamWebEngine::useCashCard() {
	getRidOfReels();
	loadKeypad();
	createPanel();
	showPanel();
	showExit();
	showMan();
	uint16 y = (!_foreignRelease) ? 120 : 120 - 3;
	showFrame(_keypadGraphics, 114, y, 39, 0);
	const uint8 *obText = getObTextStart();
	findNextColon(&obText);
	findNextColon(&obText);
	y = 98;
	printDirect(&obText, 36, &y, 36, 36 & 1);
	char amountStr[10];
	sprintf(amountStr, "%04d", _vars._card1Money / 10);
	_charShift = 91 * 2 + 75;
	printDirect((const uint8 *)amountStr, 160, 155, 240, 240 & 1);
	sprintf(amountStr, "%02d", (_vars._card1Money % 10) * 10);
	_charShift = 91 * 2 + 85;
	printDirect((const uint8 *)amountStr, 187, 155, 240, 240 & 1);
	_charShift = 0;
	workToScreenM();
	hangOnP(400);
	_keypadGraphics.clear();
	restoreReels();
	putBackObStuff();
}

void DreamWebEngine::useStereo() {
	// Handles the stereo in Ryan's apartment (accessible from the remote on
	// the couch)

	if (_vars._location != 0) {
		showPuzText(4, 400);
		putBackObStuff();
	} else if (_mapX != 11) {
		showPuzText(5, 400);
		putBackObStuff();
	} else if (checkInside(findSetObject("CDPL"), 1) == kNumexobjects) {
		// No CD inside
		showPuzText(6, 400);
		putBackObStuff();
		uint8 dummy;
		DynObject *object = (DynObject *)getAnyAd(&dummy, &dummy);
		object->turnedOn = 255;
	} else {
		// CD inside
		uint8 dummy;
		DynObject *object = (DynObject *)getAnyAd(&dummy, &dummy);
		object->turnedOn ^= 1;
		if (object->turnedOn != 255)
			showPuzText(7, 400);	// Stereo off
		else
			showPuzText(8, 400);	// Stereo on

		putBackObStuff();
	}
}

uint16 DreamWebEngine::checkInside(uint16 command, uint16 type) {
	for (uint16 index = 0; index < kNumexobjects; index++) {
		DynObject *object = getExAd(index);
		if (object->mapad[1] == command && object->mapad[0] == type)
			return index;
	}

	return kNumexobjects;
}

void DreamWebEngine::showPuzText(uint16 command, uint16 count) {
	createPanel();
	showPanel();
	showMan();
	showExit();
	obIcons();
	const uint8 *string = (const uint8 *)_puzzleText.getString(command);
	printDirect(string, 36, 104, 241, 241 & 1);
	workToScreenM();
	hangOnP(count);
}

void DreamWebEngine::useButtonA() {
	if (!isSetObOnMap(95)) {
		showFirstUse();
		turnAnyPathOn(0, _roomNum - 1);
		removeSetObject(9);
		placeSetObject(95);
		_vars._watchingTime = 15 * 2;
		_vars._reelToWatch = 71;
		_vars._endWatchReel = 85;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		_getBack = 1;
		_vars._progressPoints++;
	} else {
		// Done this bit
		showSecondUse();
		putBackObStuff();
	}
}


} // End of namespace DreamWeb
