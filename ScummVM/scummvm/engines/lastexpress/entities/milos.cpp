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

#include "lastexpress/entities/milos.h"

#include "lastexpress/entities/vesna.h"

#include "lastexpress/fight/fight.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Milos::Milos(LastExpressEngine *engine) : Entity(engine, kEntityMilos) {
	ADD_CALLBACK_FUNCTION(Milos, reset);
	ADD_CALLBACK_FUNCTION(Milos, draw);
	ADD_CALLBACK_FUNCTION(Milos, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(Milos, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION(Milos, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(Milos, playSound);
	ADD_CALLBACK_FUNCTION(Milos, playSound16);
	ADD_CALLBACK_FUNCTION(Milos, savegame);
	ADD_CALLBACK_FUNCTION(Milos, updateFromTime);
	ADD_CALLBACK_FUNCTION(Milos, enterCompartmentDialog);
	ADD_CALLBACK_FUNCTION(Milos, function11);
	ADD_CALLBACK_FUNCTION(Milos, chapter1);
	ADD_CALLBACK_FUNCTION(Milos, function13);
	ADD_CALLBACK_FUNCTION(Milos, function14);
	ADD_CALLBACK_FUNCTION(Milos, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Milos, function16);
	ADD_CALLBACK_FUNCTION(Milos, function17);
	ADD_CALLBACK_FUNCTION(Milos, function18);
	ADD_CALLBACK_FUNCTION(Milos, chapter2);
	ADD_CALLBACK_FUNCTION(Milos, chapter2Handler);
	ADD_CALLBACK_FUNCTION(Milos, function21);
	ADD_CALLBACK_FUNCTION(Milos, chapter3);
	ADD_CALLBACK_FUNCTION(Milos, function23);
	ADD_CALLBACK_FUNCTION(Milos, function24);
	ADD_CALLBACK_FUNCTION(Milos, function25);
	ADD_CALLBACK_FUNCTION(Milos, function26);
	ADD_CALLBACK_FUNCTION(Milos, function27);
	ADD_CALLBACK_FUNCTION(Milos, chapter4);
	ADD_CALLBACK_FUNCTION(Milos, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Milos, function30);
	ADD_CALLBACK_FUNCTION(Milos, function31);
	ADD_CALLBACK_FUNCTION(Milos, function32);
	ADD_CALLBACK_FUNCTION(Milos, chapter5);
	ADD_CALLBACK_FUNCTION(Milos, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Milos, function35);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Milos, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Milos, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(3, Milos, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(4, Milos, enterExitCompartment2, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(5, Milos, callbackActionOnDirection)
	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(6, Milos, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(7, Milos, playSound16)
	Entity::playSound(savepoint, false, kFlagDefault);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(8, Milos, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(9, Milos, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(10, Milos, enterCompartmentDialog, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
	case kActionDefault:
		if (getEntities()->updateEntity(kEntityMilos, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;

	case kActionExcuseMeCath:
	case kActionExcuseMe:
		if (getEvent(kEventMilosTylerCompartmentDefeat)) {
			// Robert saying: "Milos"
			switch(rnd(3)) {
			default:
			case 0:
				getSound()->playSound(kEntityPlayer, "CAT1014");
				break;

			case 1:
				getSound()->playSound(kEntityPlayer, "CAT1014A");
				break;

			case 2:
				getSound()->playSound(kEntityPlayer, "CAT1014B");
				break;
			}
		} else {
			getSound()->excuseMeCath();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(11, Milos, function11, TimeValue)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param5 && params->param1 < getState()->time && !params->param7) {
			params->param7 = 1;

			callbackAction();
			break;
		}

		if (params->param2) {
			if (Entity::updateParameter(params->param8,  getState()->timeTicks, 75)) {
				params->param2 = 0;
				params->param3 = 1;
				getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation1, kCursorNormal, kCursorNormal);
			}
		}

		params->param8 = 0;

		if (getProgress().chapter != kChapter1 || params->param5)
			break;

		if (params->param6) {
			if (Entity::updateParameter(CURRENT_PARAM(1, 1), getState()->time, 4500)) {
				params->param6 = 0;
				CURRENT_PARAM(1, 1) = 0;
			}
		}

		if (!getProgress().field_CC) {

			if (ENTITY_PARAM(0, 3) && !getProgress().field_14 && !params->param6) {
				getProgress().field_14 = 14;
				getSavePoints()->push(kEntityMilos, kEntityVesna, kAction190412928);

				setCallback(1);
				setup_enterExitCompartment("609Cg", kObjectCompartmentG);
			}
			break;
		}

		if (!params->param4)
			params->param4 = (uint)getState()->time + 18000;

		if (CURRENT_PARAM(1, 2) != kTimeInvalid) {
			if (params->param4 >= getState()->time) {
				if (!getEntities()->isDistanceBetweenEntities(kEntityPlayer, kEntityMilos, 2000) || !CURRENT_PARAM(1, 2))
					CURRENT_PARAM(1, 2) = (uint)getState()->time + 150;

				if (CURRENT_PARAM(1, 2) >= getState()->time)
					break;
			}

			CURRENT_PARAM(1, 2) = kTimeInvalid;

			if (getEntities()->isDistanceBetweenEntities(kEntityPlayer, kEntityMilos, 2000))
				getProgress().field_98 = 1;

			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorNormal, kCursorNormal);

			setCallback(6);
			setup_playSound("MIL1012");
		}
		break;

	case kActionKnock:
	case kActionOpenDoor:
		getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorNormal, kCursorNormal);

		if (params->param2) {
			if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(10);
				setup_playSound((rnd(2) ? "CAT1504" : getSound()->wrongDoorCath()));
			} else {
				setCallback(11);
				setup_playSound(getSound()->wrongDoorCath());
			}
		} else {
			setCallback(savepoint.action == kActionKnock ? 7 : 8);
			setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (params->param3 || params->param2) {
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);
			params->param3 = 0;
			params->param2 = 0;
		}
		break;

	case kActionCallback:
		switch(getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			setCallback(2);
			setup_enterCompartmentDialog(kCarGreenSleeping, kPosition_8200);
			break;

		case 2:
			setCallback(3);
			setup_function14();
			break;

		case 3:
			if (getProgress().field_14 == 14)
				getProgress().field_14 = 0;

			params->param6 = 1;
			setCallback(4);
			setup_enterCompartmentDialog(kCarRedSleeping, kPosition_3050);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("609Bg", kObjectCompartmentG);
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMilos);
			getSavePoints()->push(kEntityMilos, kEntityVesna, kAction101687594);
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);
			break;

		case 6:
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);
			break;

		case 7:
		case 8:
			setCallback(9);
			// Milos asking: "Yeah? Who is it?"
			setup_playSound("MIL1117A");
			break;

		case 9:
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorTalk, kCursorNormal);
			params->param2 = 1;
			break;

		case 10:
		case 11:
			params->param2 = 0;
			params->param3 = 1;
			break;

		case 12:
			getEntities()->drawSequenceLeft(kEntityMilos, "611Cg");
			getEntities()->enterCompartment(kEntityMilos, kObjectCompartmentG, true);
			getSavePoints()->push(kEntityMilos, kEntityCoudert, kAction88652208);
			break;

		case 13:
			getEntities()->exitCompartment(kEntityMilos, kObjectCompartmentG, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMilos);
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);
			params->param5 = 0;
			break;

		}
		break;

	case kAction122865568:
		getData()->location = kLocationOutsideCompartment;
		setCallback(12);
		setup_enterExitCompartment("611Bg", kObjectCompartmentG);
		break;

	case kAction123852928:
		params->param1 = 13;
		setup_enterExitCompartment("611Dg", kObjectCompartmentG);
		break;

	case kAction221683008:
		params->param5 = 1;
		getSavePoints()->push(kEntityMilos, kEntityCoudert, kAction123199584);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Milos, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Milos, setup_chapter1Handler));
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentG, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject46, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		getData()->entityPosition = kPosition_4689;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;

		getSavePoints()->addData(kEntityMilos, kAction157691176, 0);
		getSavePoints()->addData(kEntityMilos, kAction208228224, 2);
		getSavePoints()->addData(kEntityMilos, kAction259125998, 3);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Milos, function13)
	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		getSavePoints()->push(kEntityMilos, kEntityTables2, kActionDrawTablesWithChairs, "009E");
		getEntities()->clearSequences(kEntityVesna);
		getEntities()->clearSequences(kEntityIvo);
		getEntities()->clearSequences(kEntitySalko);

		callbackAction();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(kEntitySalko, "009D5");
		getEntities()->drawSequenceRight(kEntityTables2, "009D4");
		getEntities()->drawSequenceRight(kEntityIvo, "009D3");
		getEntities()->drawSequenceRight(kEntityVesna, "009D2");
		getEntities()->drawSequenceRight(kEntityMilos, "009D1");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Milos, function14)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getProgress().field_14 == 29 || getProgress().field_14 == 3) {
			if (params->param2) {
				setCallback(1);
				setup_enterExitCompartment("609Ca", kObjectCompartment1);
			} else {
				getEntities()->exitCompartment(kEntityMilos, kObjectCompartment1, true);
				getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

				callbackAction();
			}
			break;
		}

		if (params->param1) {

			// TODO replace with UPDATE_PARAM_PROC (without the kTimeInvalid part)
			if (!CURRENT_PARAM(1, 1))
				CURRENT_PARAM(1, 1) = getState()->timeTicks + 45;

			if (CURRENT_PARAM(1, 1) < getState()->timeTicks) {

				if (getObjects()->get(kObjectCompartment1).location == kObjectLocation1) {
					if (!Entity::updateParameter(CURRENT_PARAM(1, 2), getState()->timeTicks, 75))
						break;

					getObjects()->update(kObjectCompartment1, kEntityMilos, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorNormal);

					++params->param5;
					switch (params->param5) {
					default:
						getObjects()->update(kObjectCompartment1, kEntityMilos, getObjects()->get(kObjectCompartment1).location, params->param3 < 1 ? kCursorTalk : kCursorNormal, kCursorHand);
						CURRENT_PARAM(1, 2) = 0;
						break;

					case 1:
						setCallback(6);
						setup_playSound("LIB013");
						break;

					case 2:
						setCallback(8);
						setup_playSound("LIB012");
						break;

					case 3:
						setCallback(10);
						setup_playSound("LIB012");
						break;

					case 4:
						++params->param7;

						if (params->param7 < 3) {
							params->param5 = 1;
							getObjects()->update(kObjectCompartment1, kEntityMilos, getObjects()->get(kObjectCompartment1).location, params->param3 < 1 ? kCursorTalk : kCursorNormal, kCursorHand);
							CURRENT_PARAM(1, 2) = 0;
							break;
						}

						getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorHandKnock, kCursorHand);

						callbackAction();
						break;
					}
				} else {
					if (getProgress().eventCorpseMovedFromFloor && getProgress().jacket != kJacketBlood) {
						params->param6 = (getObjects()->get(kObjectCompartment1).location2 == kObjectLocation1) ? kEventMilosTylerCompartmentBedVisit : kEventMilosTylerCompartmentVisit;

						setCallback(3);
						setup_savegame(kSavegameTypeEvent, kEventMilosTylerCompartmentVisit);
					} else {
						getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

						setCallback(2);
						setup_savegame(kSavegameTypeEvent, kEventMilosCorpseFloor);
					}
				}
			}
			break;
		}

		// TODO replace with UPDATE_PARAM_PROC (without the kTimeInvalid part)
		if (!CURRENT_PARAM(1, 3))
			CURRENT_PARAM(1, 3) = getState()->timeTicks + 75;

		if (CURRENT_PARAM(1, 3) < getState()->timeTicks) {

			if (!params->param4) {
				setCallback(12);
				setup_playSound("MIL1030C");
				break;
			}

label_callback_12:
			if (!Entity::updateParameter(CURRENT_PARAM(1, 4), getState()->timeTicks, 75))
				break;

			getEntities()->exitCompartment(kEntityMilos, kObjectCompartment1, true);

			if (getProgress().eventCorpseMovedFromFloor) {
				setCallback(13);
				setup_enterExitCompartment("609Ba", kObjectCompartment1);
				break;
			}

			if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping)) {
				setCallback(14);
				setup_enterExitCompartment2("609Ba", kObjectCompartment1);
				break;
			}

			getScenes()->loadSceneFromPosition(kCarNone, 1);
			getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

			setCallback(15);
			setup_savegame(kSavegameTypeEvent, kEventMilosCorpseFloor);
		}
		break;

	case kActionKnock:
		if (params->param2) {
			getObjects()->update(kObjectCompartment1, kEntityMilos, kObjectLocationNone, kCursorNormal, kCursorNormal);

			setCallback(20);
			setup_playSound("LIB012");
		} else if (!params->param3) {
			getObjects()->update(kObjectCompartment1, kEntityMilos, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorNormal);

			setCallback(22);
			setup_playSound16("MIL1032");
		}
		break;

	case kActionOpenDoor:
		if (getProgress().eventCorpseMovedFromFloor && getProgress().jacket != kJacketBlood) {
			if (params->param2) {
				getEntityData(kEntityPlayer)->location = kLocationInsideCompartment;
				params->param6 = (getObjects()->get(kObjectCompartment1).location2 == kObjectLocation1) ? kEventMilosTylerCompartmentBed : kEventMilosTylerCompartment;
			} else {
				params->param6 = (getObjects()->get(kObjectCompartment1).location2 == kObjectLocation1) ? kEventMilosTylerCompartmentBedVisit : kEventMilosTylerCompartmentVisit;
			}

			setCallback(17);
			setup_savegame(kSavegameTypeEvent, kEventMilosTylerCompartmentVisit);
		} else {
			getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

			setCallback(16);
			setup_savegame(kSavegameTypeEvent, kEventMilosCorpseFloor);
		}
		break;

	case kActionDefault:
		if (getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_8200)
		 || getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_7850)
		 || getEntities()->isOutsideAlexeiWindow()) {
			getObjects()->update(kObjectCompartment1, kEntityMilos, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorNormal);

			if (getEntities()->isOutsideAlexeiWindow())
				getScenes()->loadSceneFromPosition(kCarGreenSleeping, 49);

			getSound()->playSound(kEntityPlayer, "LIB012");

			getObjects()->update(kObjectCompartment1, kEntityMilos, getObjects()->get(kObjectCompartment1).location, kCursorTalk, kCursorHand);

			params->param1 = 1;
		} else {
			getEntities()->drawSequenceLeft(kEntityMilos, "609Aa");
			getEntities()->enterCompartment(kEntityMilos, kObjectCompartment1, true);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;

		case 2:
			getSound()->playSound(kEntityPlayer, "LIB014");
			getAction()->playAnimation(kEventMilosCorpseFloor);
			getLogic()->gameOver(kSavegameTypeIndex, 1, getProgress().eventCorpseMovedFromFloor ? kSceneGameOverBloodJacket : kSceneGameOverPolice1, true);
			break;

		case 3:
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			getSound()->playSound(kEntityPlayer, "LIB014");
			getAction()->playAnimation((EventIndex)params->param6);

			setCallback(4);
			setup_savegame(kSavegameTypeTime, kTimeNone);
			break;

		case 4:
		case 18:
			params->param8 = getFight()->setup(kFightMilos);
			if (params->param8) {
				getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, params->param8  == Fight::kFightEndLost);
			} else {
				getState()->time = (TimeValue)(getState()->time + 1800);
				getProgress().field_CC = 1;

				setCallback(getCallback() + 1);
				setup_savegame(kSavegameTypeEvent, kEventMilosTylerCompartmentDefeat);
			}
			break;

		case 5:
		case 19:
			getAction()->playAnimation(kEventMilosTylerCompartmentDefeat);
			getSound()->playSound(kEntityPlayer, "LIB015");
			getScenes()->loadScene(kScene41);
			getData()->location = kLocationOutsideCompartment;

			callbackAction();
			break;

		case 6:
			setCallback(7);
			setup_playSound16("MIL1031C");
			break;

		case 7:
		case 9:
		case 11:
			getObjects()->update(kObjectCompartment1, kEntityMilos, getObjects()->get(kObjectCompartment1).location, params->param3 < 1 ? kCursorTalk : kCursorNormal, kCursorHand);
			CURRENT_PARAM(1, 2) = 0;
			break;

		case 8:
			setCallback(9);
			setup_playSound16("MIL1031A");
			break;

		case 10:
			setCallback(11);
			setup_playSound16("MIL1031B");
			break;

		case 12:
			params->param4 = 1;
			goto label_callback_12;

		case 13:
			params->param2 = 1;
			getEntities()->clearSequences(kEntityMilos);
			getData()->location = kLocationInsideCompartment;
			getObjects()->update(kObjectCompartment1, kEntityMilos, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			break;

		case 14:
			getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

			setCallback(15);
			setup_savegame(kSavegameTypeEvent, kEventMilosCorpseFloor);
			break;

		case 15:
			getAction()->playAnimation(kEventMilosCorpseFloor);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverPolice1, true);
			break;

		case 16:
			getSound()->playSound(kEntityPlayer, getObjects()->get(kObjectCompartment1).location == kObjectLocation1 ? "LIB032" : "LIB014");
			getAction()->playAnimation(kEventMilosCorpseFloor);
			getLogic()->gameOver(kSavegameTypeIndex, 1, getProgress().eventCorpseMovedFromFloor ? kSceneGameOverBloodJacket : kSceneGameOverPolice1, true);
			break;

		case 17:
			getSound()->playSound(kEntityPlayer, getObjects()->get(kObjectCompartment1).location == kObjectLocation1 ? "LIB032" : "LIB014");
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			getAction()->playAnimation((EventIndex)params->param6);

			setCallback(18);
			setup_savegame(kSavegameTypeTime, kTimeNone);
			break;

		case 20:
			setCallback(21);
			setup_playSound("MIL1117A");
			break;

		case 21:
			getObjects()->update(kObjectCompartment1, kEntityMilos, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			break;

		case 22:
			params->param3 = 1;
			getObjects()->update(kObjectCompartment1, kEntityMilos, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorHand);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Milos, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheckSavepoint(kTime1071000, params->param3, kEntityMilos, kEntityServers1, kAction223002560);

		if (getState()->time > kTime1089000 && getEntities()->isSomebodyInsideRestaurantOrSalon()) {
			setup_function16();
			break;
		}

		if (getEntities()->isPlayerPosition(kCarRestaurant, 61) && !params->param1) {
			if (Entity::updateParameter(params->param4, getState()->timeTicks, 45)) {
				setCallback(1);
				setup_draw("009C");
				break;
			}
		}

		if (getEntities()->isPlayerPosition(kCarRestaurant, 70) && !params->param2) {
			if (!Entity::updateParameter(params->param5, getState()->timeTicks, 45))
				break;

			setCallback(2);
			setup_draw("009C");
		}
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityMilos, kEntityTables2, kAction136455232);
		getEntities()->drawSequenceLeft(kEntityMilos, "009A");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kEntityMilos, "009A");
			params->param1 = 1;
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityMilos, "009A");
			params->param2 = 1;
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Milos, function16)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			if (getEntities()->isDistanceBetweenEntities(kEntityMilos, kEntityVesna, 750)
			 || getEntities()->checkDistanceFromPosition(kEntityVesna, kPosition_3050, 500)) {
				getSavePoints()->push(kEntityMilos, kEntityVesna, kAction123668192);

				setCallback(5);
				setup_enterExitCompartment("611Ag", kObjectCompartmentG);
			}
		}
		break;

	case kActionDefault:
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_function13();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityMilos, kEntityServers1, kAction269485588);
			getSavePoints()->push(kEntityMilos, kEntityIvo, kAction125242096);
			getEntities()->drawSequenceRight(kEntityMilos, "807DS");
			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityMilos);

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			getEntities()->clearSequences(kEntityMilos);
			break;

		case 3:
			if (getEntities()->isDistanceBetweenEntities(kEntityMilos, kEntityVesna, 750)
			 || getEntities()->checkDistanceFromPosition(kEntityVesna, kPosition_3050, 500)) {
				getSavePoints()->push(kEntityMilos, kEntityVesna, kAction123668192);

				setCallback(4);
				setup_enterExitCompartment("611Ag", kObjectCompartmentG);
			} else {
				params->param1 = 1;

				getEntities()->drawSequenceLeft(kEntityMilos, "609Dg");
				getEntities()->enterCompartment(kEntityMilos, kObjectCompartmentG, true);
			}
			break;

		case 4:
			getData()->entityPosition = kPosition_3050;
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMilos);

			setup_function17();
			break;

		case 5:
			getEntities()->exitCompartment(kEntityMilos, kObjectCompartmentG, true);
			getData()->entityPosition = kPosition_3050;
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMilos);

			setup_function17();
			break;
		}
		break;

	case kAction135024800:
		getSavePoints()->push(kEntityMilos, kEntityVesna, kAction204832737);

		setCallback(3);
		setup_enterCompartmentDialog(kCarRedSleeping, kPosition_3050);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Milos, function17)
	if (savepoint.action == kActionDefault) {
		setCallback(1);
		setup_function11(kTimeBedTime);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Milos, function18)
	if (savepoint.action == kActionDefault) {
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->clearSequences(kEntityMilos);
		getObjects()->update(kObjectCompartmentG, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Milos, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter2Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMilos);

		getData()->entityPosition = kPosition_4689;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;

		getObjects()->update(kObjectCompartmentG, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject46, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Milos, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_540;
		getData()->location = kLocationOutsideCompartment;

		getSavePoints()->push(kEntityMilos, kEntityVesna, kAction137165825);
		break;

	case kActionDrawScene:
		if (getEntities()->isPlayerInCar(kCarRedSleeping) && !getEntities()->isPlayerPosition(kCarRedSleeping, 1)) {
			setCallback(1);
			setup_enterCompartmentDialog(kCarRedSleeping, kPosition_3050);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("609Bg", kObjectCompartmentG);
			break;

		case 2:
			getEntities()->clearSequences(kEntityMilos);

			getData()->entityPosition = kPosition_3050;
			getData()->location = kLocationInsideCompartment;

			getSavePoints()->push(kEntityMilos, kEntityVesna, kAction101687594);

			setup_function21();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Milos, function21)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param2, getState()->time, 4500))
			break;

		params->param1 = 1;
		break;

	case kActionKnock:
		getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorNormal, kCursorNormal);

		setCallback(1);
		setup_playSound("LIB012");
		break;

	case kActionOpenDoor:
		getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorNormal, kCursorNormal);

		setCallback(3);
		setup_savegame(kSavegameTypeEvent, kEventMilosCompartmentVisitAugust);
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (!getEvent(kEventMilosCompartmentVisitAugust)
		 && !getEntities()->isInsideTrainCar(kEntityPlayer, kCarRedSleeping)
		 && params->param1)
			setup_chapter2Handler();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_playSound("Mil1118");
			break;

		case 2:
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);
			break;

		case 3:
			getAction()->playAnimation(kEventMilosCompartmentVisitAugust);
			getScenes()->loadSceneFromPosition(kCarRedSleeping, 5);
			getSavePoints()->push(kEntityMilos, kEntityVesna, kAction135024800);

			setCallback(4);
			setup_function11(kTimeEnd);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Milos, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->events[kEventMilosCompartmentVisitAugust])
			setup_function24();
		else
			setup_function23();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMilos);

		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);

		ENTITY_PARAM(0, 1) = 0;
		ENTITY_PARAM(0, 4) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Milos, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime2106000 && !params->param1) {
			params->param1 = 1;

			setCallback(1);
			setup_enterCompartmentDialog(kCarRedSleeping, kPosition_3050);
		}
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_540;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		getSavePoints()->push(kEntityMilos, kEntityVesna, kAction137165825);
		break;

	case kActionDrawScene:
		if (getEntities()->isPlayerInCar(kCarRedSleeping)
		 && !getEntities()->isPlayerPosition(kCarRedSleeping, 1)) {
			setCallback(3);
			setup_enterCompartmentDialog(kCarRedSleeping, kPosition_3050);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("609Bg", kObjectCompartmentG);
			break;

		case 2:
		case 4:
			getEntities()->clearSequences(kEntityMilos);
			getData()->location = kLocationInsideCompartment;
			getSavePoints()->push(kEntityMilos, kEntityVesna, kAction101687594);

			setup_function24();
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment("609Bg", kObjectCompartmentG);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Milos, function24)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param4)
			params->param4 = (uint)getState()->time + 4500;

		if (params->param4 < getState()->time) {
			params->param4 = kTimeInvalid;
			params->param3 = 1;
		}

		if (ENTITY_PARAM(0, 1)) {
			setCallback(1);
			setup_enterExitCompartment("609Cg", kObjectCompartmentG);
			break;
		}

		if (params->param1) {
			if (!Entity::updateParameter(params->param5, getState()->timeTicks, 75))
				break;

			params->param1 = 0;
			params->param2 = 1;

			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param5 = 0;
		break;

	case kActionKnock:
		getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorNormal, kCursorNormal);

		if (params->param1) {
			if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(9);
				setup_playSound(rnd(2) ? "CAT1504" : getSound()->wrongDoorCath());
			} else {
				setCallback(10);
				setup_playSound(getSound()->wrongDoorCath());
			}
		} else {
			setCallback(6);
			setup_playSound("LIB012");
		}
		break;

	case kActionOpenDoor:
		getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorNormal, kCursorNormal);

		if (getEvent(kEventMilosCompartmentVisitAugust) || getState()->time >= kTime2106000) {
			setCallback(12);
			setup_playSound("LIB013");
		} else {
			getData()->location = kLocationInsideCompartment;

			setCallback(11);
			setup_savegame(kSavegameTypeEvent, kEventMilosCompartmentVisitAugust);
		}
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (getEvent(kEventMilosCompartmentVisitAugust)
		 || getEntities()->isInsideTrainCar(kEntityPlayer, kCarRedSleeping)
		 || !params->param3
		 || getState()->time >= kTime2106000) {
			if (params->param1 || params->param2) {
				getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);
				params->param1 = 0;
				params->param2 = 0;
			}
			break;
		}

		setup_function23();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentG, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
			getSavePoints()->push(kEntityMilos, kEntityVesna, kAction203663744);
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_function26(kTime2223000);
			break;

		case 2:
			if (ENTITY_PARAM(0, 2)) {
				setCallback(3);
				setup_savegame(kSavegameTypeEvent, kEventMilosCorridorThanksD);
			} else {
				setCallback(4);
				setup_enterCompartmentDialog(kCarRedSleeping, kPosition_3050);
			}
			break;

		case 3:
			getAction()->playAnimation((getData()->entityPosition < getEntityData(kEntityPlayer)->entityPosition) ? kEventMilosCorridorThanksD : kEventMilosCorridorThanks);

			if (getData()->car == kCarRedSleeping && getEntities()->checkDistanceFromPosition(kEntityMilos, kPosition_3050, 500))
				getData()->entityPosition = kPosition_3550;

			getEntities()->updateEntity(kEntityMilos, kCarRedSleeping, kPosition_3050);
			getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + (750 * (getData()->direction == kDirectionDown ? 1 : -1))), getData()->direction != kDirectionDown);

			setCallback(4);
			setup_enterCompartmentDialog(kCarRedSleeping, kPosition_3050);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("609BG", kObjectCompartmentG);
			break;

		case 5:
			getEntities()->clearSequences(kEntityMilos);
			getData()->location = kLocationInsideCompartment;
			ENTITY_PARAM(0, 1) = 0;

			setup_function25();
			break;

		case 6:
			if (getEvent(kEventMilosCompartmentVisitAugust) || getState()->time >= kTime2106000) {
				setCallback(8);
				setup_playSound("Mil1117A");
			} else {
				setCallback(7);
				setup_playSound("Mil1118");
			}
			break;

		case 7:
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);
			break;

		case 8:
		case 13:
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorTalk, kCursorNormal);
			params->param1 = 1;
			break;

		case 9:
		case 10:
			params->param1 = 0;
			params->param2 = 1;
			break;

		case 11:
			getAction()->playAnimation(kEventMilosCompartmentVisitAugust);
			getScenes()->loadSceneFromPosition(kCarRedSleeping, 5);
			getSavePoints()->push(kEntityMilos, kEntityVesna, kAction135024800);
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);
			break;

		case 12:
			setCallback(13);
			setup_playSound("MIL1117A");
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Milos, function25)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!getEvent(kEventMilosCompartmentVisitTyler) && !getProgress().field_54 && !ENTITY_PARAM(0, 4)) {
			if (Entity::updateParameter(params->param3, getState()->time, 13500)) {
				getSavePoints()->push(kEntityMilos, kEntityVesna, kAction155913424);
				params->param3 = 0;
			}
		}

		if (params->param1) {
			if (!Entity::updateParameter(params->param4, getState()->timeTicks, 75))
				break;

			params->param1 = 0;
			params->param2 = 1;
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param4 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		getObjects()->update(kObjectCompartmentG, kEntityMilos, params->param1 ? kObjectLocation3 : kObjectLocation1, kCursorNormal, kCursorNormal);

		if (params->param1) {
			setCallback(5);
			setup_playSound(rnd(2) ? "CAT1505" : "CAT1505A");
		} else {
			setCallback(savepoint.action == kActionKnock ? 1 : 2);
			setup_playSound(savepoint.action == kActionKnock ?  "LIB012" : "LIB013");
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);

		if (!getEvent(kEventMilosCompartmentVisitTyler) && !getProgress().field_54 && !ENTITY_PARAM(0, 4))
			getSavePoints()->push(kEntityMilos, kEntityVesna, kAction155913424);
		break;

	case kActionDrawScene:
		if (params->param1 || params->param2) {
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation3, kCursorHandKnock, kCursorHand);

			params->param1 = 0;
			params->param2 = 0;
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			if (getEntities()->isInsideCompartment(kEntityVesna, kCarRedSleeping, kPosition_3050)) {
				setCallback(3);
				setup_playSound("VES1015A");
				break;
			}

			if (getEvent(kEventMilosCompartmentVisitTyler) || ENTITY_PARAM(0, 4)) {
				getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation1, kCursorHandKnock, kCursorHand);
				break;
			}

			RESET_ENTITY_STATE(kEntityVesna, Vesna, setup_chapter3Handler);

			getData()->location = kLocationInsideCompartment;

			setCallback(4);
			setup_savegame(kSavegameTypeEvent, kEventMilosCompartmentVisitTyler);
			break;

		case 3:
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation1, kCursorHandKnock, kCursorHand);

			params->param1 = 1;
			break;

		case 4:
			getAction()->playAnimation(kEventMilosCompartmentVisitTyler);
			getScenes()->loadSceneFromPosition(kCarRestaurant, 5);
			getObjects()->update(kObjectCompartmentG, kEntityMilos, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;

		case 5:
			params->param1 = 0;
			params->param2 = 1;
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(26, Milos, function26, TimeValue)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 < getState()->time && !params->param2) {
			callbackAction();
			break;
		}

		if (getEntities()->isPlayerInCar(kCarGreenSleeping) || getEntities()->isPlayerInCar(kCarRedSleeping)) {
			if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping)) {
				setCallback(2);
				setup_function27(kCarGreenSleeping, kPosition_540);
			} else {
				setCallback(3);
				setup_function27(kCarRedSleeping, kPosition_9460);
			}
		}
		break;

	case kActionDefault:
		ENTITY_PARAM(0, 2) = 0;

		setCallback(1);
		setup_function27(kCarRedSleeping, kPosition_540);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (ENTITY_PARAM(0, 2)) {
				callbackAction();
				break;
			}

			getEntities()->clearSequences(kEntityMilos);
			break;

		case 2:
		case 3:
			if (ENTITY_PARAM(0, 2)) {
				callbackAction();
				break;
			}

			getEntities()->clearSequences(kEntityMilos);

			setCallback(4);
			setup_updateFromTime(450);
			break;

		case 4:
			setCallback(5);
			setup_function27(kCarRedSleeping, kPosition_540);
			break;

		case 5:
			if (ENTITY_PARAM(0, 2)) {
				callbackAction();
				break;
			}

			getEntities()->clearSequences(kEntityMilos);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(27, Milos, function27, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->updateEntity(kEntityMilos, (CarIndex)params->param1, (EntityPosition)params->param2)) {
			callbackAction();
			break;
		}

		if (getEntities()->isDistanceBetweenEntities(kEntityMilos, kEntityPlayer, 1000)
		 && !getEntities()->isInGreenCarEntrance(kEntityPlayer)
		 && !getEntities()->isInsideCompartments(kEntityPlayer)
		 && !getEntities()->checkFields10(kEntityPlayer)) {
			if (getData()->car == kCarRedSleeping || getData()->car == kCarGreenSleeping) {
				ENTITY_PARAM(0, 2) = 1;

				callbackAction();
			}
		}
		break;

	case kActionDefault:
		if (getEntities()->updateEntity(kEntityMilos, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Milos, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMilos);

		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Milos, chapter4Handler)
#define TIME_CHECK_PLAYSOUND_MILOS(timeValue, parameter, sound) \
	if (getState()->time > timeValue && !parameter) { \
		parameter = 1; \
		getSound()->playSound(kEntityMilos, sound); \
		if (getEntities()->isDistanceBetweenEntities(kEntityMilos, kEntityPlayer, 2000)) \
			getProgress().field_94 = 1; \
		break; \
	}

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1)
			break;

		if (params->param2) {
			setup_function30();
			break;
		}

		TIME_CHECK_PLAYSOUND_MILOS(kTime2356200, params->param3, "Mil4013");

		TIME_CHECK_PLAYSOUND_MILOS(kTime2360700, params->param4, "Mil4014");

		TIME_CHECK_PLAYSOUND_MILOS(kTime2370600, params->param5, "Mil4015");

		Entity::timeCheckSavepoint(kTime2407500, params->param6, kEntityMilos, kEntityVesna, kAction55996766);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			getEntities()->drawSequenceLeft(kEntityMilos, "611Cg");
			getEntities()->enterCompartment(kEntityMilos, kObjectCompartmentG, true);
			getSavePoints()->push(kEntityMilos, kEntityCoudert, kAction88652208);
			break;

		case 2:
			getEntities()->exitCompartment(kEntityMilos, kObjectCompartmentG);

			getData()->location = kLocationInsideCompartment;
			getData()->entityPosition = kPosition_3050;

			getEntities()->clearSequences(kEntityMilos);

			params->param1 = 0;
			break;
		}
		break;

	case kAction122865568:
		setCallback(1);
		setup_enterExitCompartment("611Bg", kObjectCompartmentG);
		break;

	case kAction123852928:
		setCallback(2);
		setup_enterExitCompartment("611Dg", kObjectCompartmentG);
		break;

	case kAction135600432:
		params->param2 = 1;
		break;

	case kAction221683008:
		if (getSoundQueue()->isBuffered(kEntityMilos))
			getSoundQueue()->processEntry(kEntityMilos);

		params->param1 = 1;
		getSavePoints()->push(kEntityMilos, kEntityCoudert, kAction123199584);
		break;
	}

#undef TIME_CHECK_PLAYSOUND_MILOS
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Milos, function30)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setCallback(1);
		setup_function11(kTime2410200);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityMilos, kEntityIvo, kAction55996766);

			setCallback(2);
			setup_function11(kTime2412000);
			break;

		case 2:
			getSavePoints()->push(kEntityMilos, kEntitySalko, kAction55996766);

			setCallback(3);
			setup_function11(kTime2415600);
			break;

		case 3:
			setup_function31();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Milos, function31)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setCallback(1);
		setup_enterExitCompartment("609CG", kObjectCompartmentG);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			getObjects()->update(kObjectCompartmentG, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);

			setCallback(2);
			setup_enterCompartmentDialog(kCarGreenSleeping, kPosition_540);
			break;

		case 2:
			setup_function32();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Milos, function32)
	if (savepoint.action == kActionDefault) {
		getEntities()->clearSequences(kEntityMilos);
		getObjects()->update(kObjectCompartmentG, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_540;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarCoalTender;
		getData()->inventoryItem = kItemNone;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Milos, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMilos);

		getData()->entityPosition = kPosition_540;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarCoalTender;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(34, Milos, chapter5Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionEndSound:
		if (!getProgress().isNightTime) {
			setCallback(6);
			setup_savegame(kSavegameTypeEvent, kEventTrainStopped);
			break;
		}

		getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneGameOverTrainStopped2, true);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(isNight() ? kEventLocomotiveMilosShovelingNight : kEventLocomotiveMilosShovelingDay);
			getScenes()->processScene();
			break;

		case 2:
			if (getSoundQueue()->isBuffered("MUS050"))
				getSoundQueue()->processEntry("MUS050");

			if (getSoundQueue()->isBuffered("ARRIVE"))
				getSoundQueue()->removeFromQueue("ARRIVE");

			getSoundQueue()->processEntries();
			getAction()->playAnimation(isNight() ? kEventLocomotiveMilosNight : kEventLocomotiveMilosDay);
			getSoundQueue()->setupEntry(kSoundType7, kEntityMilos);
			getScenes()->loadSceneFromPosition(kCarCoalTender, 1);
			break;

		case 3:
			getAction()->playAnimation(kEventLocomotiveAnnaStopsTrain);
			getLogic()->gameOver(kSavegameTypeEvent2, kEventLocomotiveMilosDay, kSceneGameOverTrainStopped, true);
			break;

		case 4:
			getAction()->playAnimation(kEventLocomotiveRestartTrain);
			getAction()->playAnimation(kEventLocomotiveOldBridge);
			getSoundQueue()->resetState();
			getState()->time = kTime2983500;

			setCallback(5);
			setup_savegame(kSavegameTypeTime, kTimeNone);
			break;

		case 5:
			getScenes()->loadSceneFromPosition(kCarCoalTender, 2, 1);
			getSavePoints()->push(kEntityMilos, kEntityAbbot, kAction135600432);

			setup_function35();
			break;

		case 6:
			getAction()->playAnimation(kEventTrainStopped);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverTrainStopped, true);
			break;
		}
		break;

	case kAction168646401:
		if (!getEvent(kEventLocomotiveMilosShovelingDay) && !getEvent(kEventLocomotiveMilosShovelingNight)) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventLocomotiveMilosShovelingDay);
			break;
		}

		if (!getEvent(kEventLocomotiveMilosDay) && !getEvent(kEventLocomotiveMilosNight)) {
			if (getProgress().isNightTime && getState()->time < kTimeTrainStopped2)
				getState()->time = kTimeTrainStopped2;

			setCallback(2);
			setup_savegame(kSavegameTypeEvent, kEventLocomotiveMilosDay);
		}
		break;

	case kAction169773228:
		if (!getProgress().isNightTime) {
			setCallback(3);
			setup_savegame(kSavegameTypeEvent, kEventLocomotiveAnnaStopsTrain);
		}

		getSoundQueue()->processEntry(kEntityMilos);
		if (getState()->time < kTimeTrainStopped2)
			getState()->time = kTimeTrainStopped2;

		setCallback(4);
		setup_savegame(kSavegameTypeEvent, kEventLocomotiveRestartTrain);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(35, Milos, function35)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityMilos);
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
