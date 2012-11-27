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

static void (DreamWebEngine::*reelCallbacks[57])(ReelRoutine &) = {
	&DreamWebEngine::gamer, &DreamWebEngine::sparkyDrip,
	&DreamWebEngine::eden, &DreamWebEngine::edenInBath,
	&DreamWebEngine::sparky, &DreamWebEngine::smokeBloke,
	&DreamWebEngine::manAsleep, &DreamWebEngine::drunk,
	&DreamWebEngine::receptionist, &DreamWebEngine::genericPerson /*maleFan*/,
	&DreamWebEngine::genericPerson /*femaleFan*/, &DreamWebEngine::louis,
	&DreamWebEngine::louisChair, &DreamWebEngine::soldier1,
	&DreamWebEngine::bossMan, &DreamWebEngine::interviewer,
	&DreamWebEngine::heavy, &DreamWebEngine::manAsleep /*manAsleep2*/,
	&DreamWebEngine::genericPerson /*manSatStill*/, &DreamWebEngine::drinker,
	&DreamWebEngine::bartender, &DreamWebEngine::genericPerson /*otherSmoker*/,
	&DreamWebEngine::genericPerson /*tattooMan*/, &DreamWebEngine::attendant,
	&DreamWebEngine::keeper, &DreamWebEngine::candles1,
	&DreamWebEngine::smallCandle, &DreamWebEngine::security,
	&DreamWebEngine::copper, &DreamWebEngine::poolGuard,
	&DreamWebEngine::rockstar, &DreamWebEngine::businessMan,
	&DreamWebEngine::train, &DreamWebEngine::genericPerson /*aide*/,
	&DreamWebEngine::mugger, &DreamWebEngine::helicopter,
	&DreamWebEngine::introMagic1, &DreamWebEngine::introMusic,
	&DreamWebEngine::introMagic2, &DreamWebEngine::candles2,
	&DreamWebEngine::gates, &DreamWebEngine::introMagic3,
	&DreamWebEngine::introMonks1, &DreamWebEngine::candles,
	&DreamWebEngine::introMonks2, &DreamWebEngine::handClap,
	&DreamWebEngine::monkAndRyan, &DreamWebEngine::endGameSeq,
	&DreamWebEngine::priest, &DreamWebEngine::madman,
	&DreamWebEngine::madmansTelly, &DreamWebEngine::alleyBarkSound,
	&DreamWebEngine::foghornSound, &DreamWebEngine::carParkDrip,
	&DreamWebEngine::carParkDrip, &DreamWebEngine::carParkDrip,
	&DreamWebEngine::carParkDrip
};

static const ReelRoutine g_initialReelRoutines[] = {
// Room number and x,y
// reel pointer
// speed,speed count,convers. no.
	{ 1,44,0, 20, 2,0,1 },
	{ 1,55,0, 0, 50,20,0 },
	{ 24,22,0, 74, 1,0,0 },
	{ 24,33,10, 75, 1,0,1 },
	{ 1,44,0, 27, 2,0,2 },
	{ 1,44,0, 96, 3,0,4 },
	{ 1,44,0, 118, 2,0,5 },
	{ 1,44,10, 0, 2,0,0 },
	{ 5,22,20, 53, 3,0,0 },
	{ 5,22,20, 40, 1,0,2 },
	{ 5,22,20, 50, 1,0,3 },
	{ 2,11,10, 192, 1,0,0 },
	{ 2,11,10, 182, 2,0,1 },
	{ 8,11,10, 0, 2,0,1 },
	{ 23,0,50, 0, 3,0,0 },
	{ 28,11,20, 250, 4,0,0 },
	{ 23,0,50, 43, 2,0,8 },
	{ 23,11,40, 130, 2,0,1 },
	{ 23,22,40, 122, 2,0,2 },
	{ 23,22,40, 105, 2,0,3 },
	{ 23,22,40, 81, 2,0,4 },
	{ 23,11,40, 135, 2,0,5 },
	{ 23,22,40, 145, 2,0,6 },
	{ 4,22,30, 0, 2,0,0 },
	{ 45,22,30, 200, 0,0,20 },
	{ 45,22,30, 39, 2,0,0 },
	{ 45,22,30, 25, 2,0,0 },
	{ 8,22,40, 32, 2,0,0 },
	{ 7,11,20, 64, 2,0,0 },
	{ 22,22,20, 82, 2,0,0 },
	{ 27,11,30, 0, 2,0,0 },
	{ 20,0,30, 0, 2,0,0 },
	{ 14,33,40, 21, 1,0,0 },
	{ 29,11,10, 0, 1,0,0 },
	{ 2,22,0, 2, 2,0,0 },
	{ 25,0,50, 4, 2,0,0 },
	{ 50,22,30, 121, 2,0,0 },
	{ 50,22,30, 0, 20,0,0 },
	{ 52,22,30, 192, 2,0,0 },
	{ 52,22,30, 233, 2,0,0 },
	{ 50,22,40, 104, 55,0,0 }, // ...., 65,0,0 for German CD
	{ 53,33,0, 99, 2,0,0 },
	{ 50,22,40, 0, 3,0,0 },
	{ 50,22,30, 162, 2,0,0 },
	{ 52,22,30, 57, 2,0,0 },
	{ 52,22,30, 0, 2,0,0 },
	{ 54,0,0, 72, 3,0,0 },
	{ 55,44,0, 0, 2,0,0 },
	{ 19,0,0, 0, 28,0,0 },
	{ 14,22,0, 2, 2,0,0 },
	{ 14,22,0, 300, 1,0,0 },
	{ 10,22,30, 174, 0,0,0 },
	{ 12,22,20, 0, 1,0,0 },
	{ 11,11,20, 0, 50,20,0 },
	{ 11,11,30, 0, 50,20,0 },
	{ 11,22,20, 0, 50,20,0 },
	{ 14,33,40, 0, 50,20,0 },
	{ 255,0,0, 0, 0,0,0 }
};

void DreamWebEngine::setupInitialReelRoutines() {
	for (unsigned int i = 0; i < kNumReelRoutines + 1; ++i) {
		_reelRoutines[i] = g_initialReelRoutines[i];
		if (_reelRoutines[i].period == 55 && hasSpeech() && getLanguage() == Common::DE_DEU)
			_reelRoutines[i].period = 65;
	}
}

void DreamWebEngine::updatePeople() {
	_peopleList.clear();
	++_mainTimer;

	for (int i = 0; _reelRoutines[i].reallocation != 255; ++i) {
		if (_reelRoutines[i].reallocation == _realLocation &&
		        _reelRoutines[i].mapX == _mapX &&
		        _reelRoutines[i].mapY == _mapY) {
			assert(reelCallbacks[i]);
			(this->*(reelCallbacks[i]))(_reelRoutines[i]);
		}
	}
}

void DreamWebEngine::madmanText() {
	byte origCount;

	uint16 length = 90;
	if (hasSpeech()) {
		if (_speechCount > 15)
			return;
		if (_sound->isChannel1Playing())
			return;
		origCount = _speechCount;
		++_speechCount;

		if (origCount != 15)
			length = 32000; // Set subtitle time very high to make it
			                // always wait for the next line, except for the
			                // last one, when there is no next line.
	} else {
		if (_vars._combatCount >= 61)
			return;
		if (_vars._combatCount & 3)
			return;
		origCount = _vars._combatCount / 4;
	}
	setupTimedTemp(47 + origCount, 82, 72, 80, length, 1);
}

void DreamWebEngine::madman(ReelRoutine &routine) {
	_vars._watchingTime = 2;
	if (checkSpeed(routine)) {
		uint16 newReelPointer = routine.reelPointer();
		if (newReelPointer >= 364) {
			_vars._manDead = 2;
			showGameReel(&routine);
			return;
		}
		if (newReelPointer == 10) {
			loadTempText("T82");
			_vars._combatCount = (uint8)-1;
			_speechCount = 0;
		}
		++newReelPointer;
		if (newReelPointer == 294) {
			if (!_wonGame) {
				_wonGame = true;
				getRidOfTempText();
			}
			return;
		}
		if (newReelPointer == 66) {
			++_vars._combatCount;

			if (_lastHardKey == 1)	// ESC pressed, skip the mad man's speech
				_vars._combatCount = _speechCount = (hasSpeech() ? 65 : 63);

			madmanText();
			newReelPointer = 53;

			if (_vars._combatCount >= (hasSpeech() ? 64 : 62)) {
				if (_vars._combatCount == (hasSpeech() ? 70 : 68))
					newReelPointer = 310;
				else {
					if (_vars._lastWeapon == 8) {
						_vars._combatCount = hasSpeech() ? 72 : 70;
						_vars._lastWeapon = (uint8)-1;
						_vars._madmanFlag = 1;
						newReelPointer = 67;
					}
				}
			}
		}
		routine.setReelPointer(newReelPointer);
	}
	showGameReel(&routine);
	routine.mapX = _mapX;
	madMode();
}

void DreamWebEngine::madMode() {
	_vars._watchingTime = 2;
	_pointerMode = 0;
	if (_vars._combatCount < (hasSpeech() ? 65 : 63))
		return;
	if (_vars._combatCount >= (hasSpeech() ? 70 : 68))
		return;
	_pointerMode = 2;
}

void DreamWebEngine::addToPeopleList(ReelRoutine *routine) {
	People people;
	people._reelPointer = routine->reelPointer();
	people._routinePointer = routine;
	people.b4 = routine->b7;

	_peopleList.push_back(people);
}

bool DreamWebEngine::checkSpeed(ReelRoutine &routine) {
	if (_vars._lastWeapon != (uint8)-1)
		return true;
	++routine.counter;
	if (routine.counter != routine.period)
		return false;
	routine.counter = 0;
	return true;
}

void DreamWebEngine::sparkyDrip(ReelRoutine &routine) {
	if (checkSpeed(routine))
		_sound->playChannel0(14, 0);
}

void DreamWebEngine::genericPerson(ReelRoutine &routine) {
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::gamer(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint8 v;
		do {
			v = _rnd.getRandomNumberRng(20, 24);
		} while (v == routine.reelPointer());
		routine.setReelPointer(v);
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::eden(ReelRoutine &routine) {
	if (_vars._generalDead)
		return;
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::sparky(ReelRoutine &routine) {
	if (_vars._card1Money)
		routine.b7 = 3;
	if (checkSpeed(routine)) {
		if (routine.reelPointer() == 34) {
			if (randomNumber() < 30)
				routine.incReelPointer();
			else
				routine.setReelPointer(27);
		} else {
			if (routine.reelPointer() != 48)
				routine.incReelPointer();
			else
				routine.setReelPointer(27);
		}
	}
	showGameReel(&routine);
	addToPeopleList(&routine);
	if (routine.b7 & 128)
		_vars._talkedToSparky = 1;
}

void DreamWebEngine::rockstar(ReelRoutine &routine) {
	if ((routine.reelPointer() == 303) || (routine.reelPointer() == 118)) {
		_newLocation = 45;
		showGameReel(&routine);
		return;
	}
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 118) {
			_vars._manDead = 2;
		} else if (nextReelPointer == 79) {
			--nextReelPointer;
			if (_vars._lastWeapon != 1) {
				++_vars._combatCount;
				if (_vars._combatCount == 40) {
					_vars._combatCount = 0;
					nextReelPointer = 79;
				}
			} else {
				_vars._lastWeapon = (uint8)-1;
				nextReelPointer = 123;
			}
		}
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
	if (routine.reelPointer() == 78) {
		addToPeopleList(&routine);
		_pointerMode = 2;
		_vars._watchingTime = 0;
	} else {
		_vars._watchingTime = 2;
		_pointerMode = 0;
		routine.mapY = _mapY;
	}
}

void DreamWebEngine::madmansTelly(ReelRoutine &routine) {
	uint16 nextReelPointer = routine.reelPointer() + 1;
	if (nextReelPointer == 307)
		nextReelPointer = 300;
	routine.setReelPointer(nextReelPointer);
	showGameReel(&routine);
}


void DreamWebEngine::smokeBloke(ReelRoutine &routine) {
	if (_vars._rockstarDead == 0) {
		if (routine.b7 & 128)
			setLocation(5);
	}
	if (checkSpeed(routine)) {
		if (routine.reelPointer() == 100) {
			if (randomNumber() < 30)
				routine.incReelPointer();
			else
				routine.setReelPointer(96);
		} else if (routine.reelPointer() == 117)
			routine.setReelPointer(96);
		else
			routine.incReelPointer();
	}
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::manAsleep(ReelRoutine &routine) {
	routine.b7 &= 127;
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::attendant(ReelRoutine &routine) {
	showGameReel(&routine);
	addToPeopleList(&routine);
	if (routine.b7 & 128)
		_vars._talkedToAttendant = 1;
}

void DreamWebEngine::keeper(ReelRoutine &routine) {
	if (_vars._keeperFlag != 0) {
		// Not waiting
		addToPeopleList(&routine);
		showGameReel(&routine);
		return;
	}

	if (_vars._reelToWatch < 190)
		return; // waiting

	_vars._keeperFlag++;

	if ((routine.b7 & 127) != _vars._dreamNumber)
		routine.b7 = _vars._dreamNumber;
}

void DreamWebEngine::drunk(ReelRoutine &routine) {
	if (_vars._generalDead)
		return;
	routine.b7 &= 127;
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::interviewer(ReelRoutine &routine) {
	if (_vars._reelToWatch == 68)
		routine.incReelPointer();

	if (routine.reelPointer() != 250 && routine.reelPointer() != 259 && checkSpeed(routine))
		routine.incReelPointer();

	showGameReel(&routine);
}

void DreamWebEngine::drinker(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		routine.incReelPointer();

		if ( routine.reelPointer() == 115 ||
			(routine.reelPointer() == 106 && randomNumber() >= 3))
			routine.setReelPointer(105);
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::alleyBarkSound(ReelRoutine &routine) {
	uint16 prevReelPointer = routine.reelPointer() - 1;
	if (prevReelPointer == 0) {
		_sound->playChannel1(14);
		routine.setReelPointer(1000);
	} else {
		routine.setReelPointer(prevReelPointer);
	}
}

void DreamWebEngine::introMagic1(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 145)
			nextReelPointer = 121;
		routine.setReelPointer(nextReelPointer);
		if (nextReelPointer == 121) {
			++_introCount;
			intro1Text();
			if (_introCount == 8) {
				_mapY += 10;
				_nowInNewRoom = 1;
			}
		}
	}
	showGameReel(&routine);
}

void DreamWebEngine::introMagic2(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 216)
			nextReelPointer = 192;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamWebEngine::introMagic3(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 218)
			_getBack = 1;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
	routine.mapX = _mapX;
}

void DreamWebEngine::candles1(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 44)
			nextReelPointer = 39;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamWebEngine::candles2(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 238)
			nextReelPointer = 233;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamWebEngine::smallCandle(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 37)
			nextReelPointer = 25;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamWebEngine::introMusic(ReelRoutine &routine) {
}

void DreamWebEngine::candles(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 167)
			nextReelPointer = 162;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamWebEngine::gates(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 116)
			_sound->playChannel1(17);
		if (nextReelPointer >= 110)
			routine.period = 2;
		if (nextReelPointer == 120) {
			_getBack = 1;
			nextReelPointer = 119;
		}
		routine.setReelPointer(nextReelPointer);
		intro3Text(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamWebEngine::security(ReelRoutine &routine) {
	if (routine.reelPointer() == 32) {
		if (_vars._lastWeapon == 1) {
			_vars._watchingTime = 10;
			if ((_mansPath == 9) && (_facing == 0)) {
				_vars._lastWeapon = (uint8)-1;
				routine.incReelPointer();
			}
		}
	} else if (routine.reelPointer() == 69)
		return;
	else {
		_vars._watchingTime = 10;
		if (checkSpeed(routine))
			routine.incReelPointer();
	}
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::edenInBath(ReelRoutine &routine) {
	if (_vars._generalDead == 0 || _vars._sartainDead != 0)
		return;

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::louis(ReelRoutine &routine) {
	if (_vars._rockstarDead != 0)
		return;

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::handClap(ReelRoutine &routine) {
}

void DreamWebEngine::carParkDrip(ReelRoutine &routine) {
	if (!checkSpeed(routine))
		return; // cantdrip2

	_sound->playChannel1(14);
}

void DreamWebEngine::foghornSound(ReelRoutine &routine) {
	if (randomNumber() == 198)
		_sound->playChannel1(13);
}

void DreamWebEngine::train(ReelRoutine &routine) {
	// The original code has logic for this, but it is disabled
}

void DreamWebEngine::louisChair(ReelRoutine &routine) {
	if (_vars._rockstarDead == 0)
		return; // notlouis2

	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 191) {
			routine.setReelPointer(182);	// Restart Louis
		} else if (nextReelPointer != 185) {
			routine.setReelPointer(nextReelPointer);
		} else {
			if (randomNumber() < 245)
				routine.setReelPointer(182);	// Restart Louis
			else
				routine.setReelPointer(nextReelPointer);
		}
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::bossMan(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;

		if (nextReelPointer == 4) {
			if (_vars._gunPassFlag != 1 && randomNumber() >= 10)
				nextReelPointer = 0;
		} else if (nextReelPointer == 20) {
			if (_vars._gunPassFlag != 1)
				nextReelPointer = 0;
		} else if (nextReelPointer == 41) {
			nextReelPointer = 0;
			_vars._gunPassFlag++;
			routine.b7 = 10;
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
	addToPeopleList(&routine);

	if (routine.b7 & 128)
		_vars._talkedToBoss = 1;
}

void DreamWebEngine::priest(ReelRoutine &routine) {
	if (routine.reelPointer() == 8)
		return; // priestspoken

	_pointerMode = 0;
	_vars._watchingTime = 2;

	if (checkSpeed(routine)) {
		routine.incReelPointer();
		priestText(routine);
	}
}

void DreamWebEngine::priestText(ReelRoutine &routine) {
	uint16 reel = routine.reelPointer();
	if (reel < 2 || reel >= 7 || (reel & 1))
		return; // nopriesttext

	setupTimedUse((reel >> 1) + 50, 54, 1, 72, 80);
}

void DreamWebEngine::monkAndRyan(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 83) {
			_introCount++;
			textForMonk();
			nextReelPointer = 77;

			if (_introCount == 57) {
				_getBack = 1;
				return;
			}
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
}

void DreamWebEngine::copper(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 94) {
			nextReelPointer = 64;
		} else if (nextReelPointer == 81 || nextReelPointer == 66) {
			// Might wait
			if (randomNumber() >= 7)
				nextReelPointer--;
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::introMonks1(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;

		if (nextReelPointer == 80) {
			_mapY += 10;
			_nowInNewRoom = 1;
			showGameReel(&routine);
			return;
		} else if (nextReelPointer == 30) {
			_mapY -= 10;
			_nowInNewRoom = 1;
			nextReelPointer = 51;
		}

		routine.setReelPointer(nextReelPointer);

		if (nextReelPointer ==  5 || nextReelPointer == 15 ||
			nextReelPointer == 25 || nextReelPointer == 61 ||
			nextReelPointer == 71) {
			// Wait step
			intro2Text(nextReelPointer);
			routine.counter = (uint8)-20;
		}
	}

	showGameReel(&routine);
	routine.mapY = _mapY;
}

void DreamWebEngine::introMonks2(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 87) {
			_introCount++;
			monks2text();

			if (_introCount == 19)
				nextReelPointer = 87;
			else
				nextReelPointer = 74;
		}

		if (nextReelPointer == 110) {
			_introCount++;
			monks2text();

			if (_introCount == 35)
				nextReelPointer = 111;
			else
				nextReelPointer = 98;
		} else if (nextReelPointer == 176) {
			_getBack = 1;
		} else if (nextReelPointer == 125) {
			nextReelPointer = 140;
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
}

void DreamWebEngine::soldier1(ReelRoutine &routine) {
	if (routine.reelPointer() != 0) {
		_vars._watchingTime = 10;
		if (routine.reelPointer() == 30) {
			_vars._combatCount++;
			if (_vars._combatCount == 40)
				_vars._manDead = 2;
		} else if (checkSpeed(routine)) {
			// Not after shot
			routine.incReelPointer();
		}
	} else if (_vars._lastWeapon == 1) {
		_vars._watchingTime = 10;
		if (_mansPath == 2 && _facing == 4)
			routine.incReelPointer();
		_vars._lastWeapon = 0xFF;
		_vars._combatCount = 0;
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::receptionist(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		if (_vars._cardPassFlag == 1) {
			// Set card
			_vars._cardPassFlag++;
			routine.b7 = 1;
			routine.setReelPointer(64);
		}

		if (routine.reelPointer() != 58) {
			// notdes1
			if (routine.reelPointer() != 60) {
				// notdes2
				if (routine.reelPointer() != 88)
					routine.incReelPointer();	// not end card
				else
					routine.setReelPointer(53);
			} else if (randomNumber() >= 240) {
				routine.setReelPointer(53);
			}
		} else if (randomNumber() >= 30) {
			routine.setReelPointer(55);
		} else {
			// notdes2
			if (routine.reelPointer() != 88)
				routine.incReelPointer();	// not end card
			else
				routine.setReelPointer(53);
		}
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
	if (routine.b7 & 128)
		_vars._talkedToRecep = 1;
}

void DreamWebEngine::bartender(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		if (routine.reelPointer() == 86) {
			if (randomNumber() >= 18)
				routine.setReelPointer(81);
			else
				routine.incReelPointer();	// notsmoket2
		} else if (routine.reelPointer() == 103) {
			routine.setReelPointer(81);	// notsmoket1
		} else {
			routine.incReelPointer();	// notsmoket2
		}
	}

	showGameReel(&routine);
	if (_vars._gunPassFlag == 1)
		routine.b7 = 9;	// got gun

	addToPeopleList(&routine);
}

void DreamWebEngine::heavy(ReelRoutine &routine) {
	routine.b7 &= 127;
	if (routine.reelPointer() != 43) {
		_vars._watchingTime = 10;
		if (routine.reelPointer() == 70) {
			// After shot
			_vars._combatCount++;
			if (_vars._combatCount == 80)
				_vars._manDead = 2;
		} else {
			if (checkSpeed(routine))
				routine.incReelPointer();
		}
	} else if (_vars._lastWeapon == 1 && _mansPath == 5 && _facing == 4) {
		// Heavy wait
		_vars._lastWeapon = (byte)-1;
		routine.incReelPointer();
		_vars._combatCount = 0;
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamWebEngine::helicopter(ReelRoutine &routine) {
	if (routine.reelPointer() == 203) {
		// Won helicopter
		_pointerMode = 0;
		return;
	}

	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 53) {
			// Before killing helicopter
			_vars._combatCount++;
			if (_vars._combatCount >= 8)
				_vars._manDead = 2;
			nextReelPointer = 49;
		} else if (nextReelPointer == 9) {
			nextReelPointer--;
			if (_vars._lastWeapon == 1) {
				_vars._lastWeapon = (byte)-1;
				nextReelPointer = 55;
			} else {
				nextReelPointer = 5;
				_vars._combatCount++;
				if (_vars._combatCount == 20) {
					_vars._combatCount = 0;
					nextReelPointer = 9;
				}
			}
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
	routine.mapX = _mapX;
	if (routine.reelPointer() < 9 && _vars._combatCount >= 7) {
		_pointerMode = 2;
		_vars._watchingTime = 0;
	} else {
		// Not waiting helicopter
		_pointerMode = 0;
		_vars._watchingTime = 2;
	}
}

void DreamWebEngine::mugger(ReelRoutine &routine) {
	if (routine.reelPointer() != 138) {
		if (routine.reelPointer() == 176)
			return; // endmugger2

		if (routine.reelPointer() == 2)
			_vars._watchingTime = 175 * 2;	// set watch

		if (checkSpeed(routine))
			routine.incReelPointer();

		showGameReel(&routine);
		routine.mapX = _mapX;
	} else {
		createPanel2();
		showIcon();

		const uint8 *string = (const uint8 *)_puzzleText.getString(41);
		uint16 y = 104;
		printDirect(&string, 33 + 20, &y, 241, 241 & 1);
		workToScreen();
		hangOn(300);
		routine.setReelPointer(140);
		_mansPath = 2;
		_finalDest = 2;
		findXYFromPath();
		_resetManXY = 1;
		_command = findExObject("WETA");
		_objectType = kExObjectType;
		removeObFromInv();
		_command = findExObject("WETB");
		_objectType = kExObjectType;
		removeObFromInv();
		makeMainScreen();
		setupTimedUse(48, 70, 10, 68 - 32, 54 + 64);
		_vars._beenMugged = 1;
	}
}

// Exiting the elevator of Sartain's industries, Sartain (the businessman) and
// two bodyguards are expecting Ryan.
void DreamWebEngine::businessMan(ReelRoutine &routine) {
	_pointerMode = 0;
	_vars._watchingTime = 2;
	if (routine.reelPointer() == 2)
		setupTimedUse(49, 30, 1, 68, 174);	// First

	if (routine.reelPointer() == 95) {
		// Businessman combat won - end
		_pointerMode = 0;
		_vars._watchingTime = 0;
		return;
	}

	if (routine.reelPointer() == 49)
		return; // Businessman combat end

	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 48) {
			_vars._manDead = 2;	// before dead body
		} else if (nextReelPointer == 15) {
			nextReelPointer--;
			if (_vars._lastWeapon == 3) {
				// Shield on bus
				_vars._lastWeapon = (byte)-1;
				_vars._combatCount = 0;
				nextReelPointer = 51;
			} else {
				// No shield on businessman
				_vars._combatCount++;
				if (_vars._combatCount == 20) {
					_vars._combatCount = 0;
					nextReelPointer = 15;
				}
			}
		} else {
			// Businessman combat won
			if (nextReelPointer == 91) {
				turnPathOn(0);
				turnPathOn(1);
				turnPathOn(2);
				turnPathOff(3);
				_mansPath = 5;
				_finalDest = 5;
				findXYFromPath();
				_resetManXY = 1;
				nextReelPointer = 92;
			}
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
	routine.mapY = _mapY;
	if (routine.reelPointer() == 14) {
		_vars._watchingTime = 0;
		_pointerMode = 2;
	}
}

void DreamWebEngine::endGameSeq(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 51 && _introCount != 140) {
			_introCount++;
			textForEnd();
			nextReelPointer = 50;
		}

		routine.setReelPointer(nextReelPointer);
		if (nextReelPointer == 134) {
			fadeScreenDownHalf();
		} else if (nextReelPointer == 324) {
			fadeScreenDowns();
			_sound->volumeChange(7, 1);
		}

		if (nextReelPointer == 340)
			_getBack = 1;
	}

	showGameReel(&routine);
	routine.mapY = _mapY;

	if (routine.reelPointer() == 145) {
		routine.setReelPointer(146);
		rollEndCreditsGameWon();
	}
}

void DreamWebEngine::poolGuard(ReelRoutine &routine) {
	if (routine.reelPointer() == 214 || routine.reelPointer() == 258) {
		// Combat over 2
		showGameReel(&routine);
		_vars._watchingTime = 2;
		_pointerMode = 0;
		_vars._combatCount++;
		if (_vars._combatCount < 100)
			return; // doneover2
		_vars._watchingTime = 0;
		_vars._manDead = 2;
		return;
	} else if (routine.reelPointer() == 185) {
		// Combat over 1
		_vars._watchingTime = 0;
		_pointerMode = 0;
		turnPathOn(0);
		turnPathOff(1);
		return;
	}

	if (routine.reelPointer() == 0)
		turnPathOn(0);	// first pool

	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;

		if (nextReelPointer != 122) {
			// Not end guard 1
			if (nextReelPointer == 147) {
				nextReelPointer--;
				if (_vars._lastWeapon == 1) {
					// Gun on pool
					_vars._lastWeapon = (byte)-1;
					nextReelPointer = 147;
				} else {
					// Gun not on pool
					_vars._combatCount++;
					if (_vars._combatCount == 40) {
						_vars._combatCount = 0;
						nextReelPointer = 220;
					}
				}
			}
		} else {
			nextReelPointer--;

			if (_vars._lastWeapon == 2) {
				// Axe on pool
				_vars._lastWeapon = (byte)-1;
				nextReelPointer = 122;
			} else {
				_vars._combatCount++;
				if (_vars._combatCount == 40) {
					_vars._combatCount = 0;
					nextReelPointer = 195;
				}
			}
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);

	if (routine.reelPointer() != 121 && routine.reelPointer() != 146) {
		_pointerMode = 0;
		_vars._watchingTime = 2;
	} else {
		_pointerMode = 2;
		_vars._watchingTime = 0;
	}
}

} // End of namespace DreamWeb
