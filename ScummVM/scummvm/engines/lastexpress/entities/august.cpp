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

#include "lastexpress/entities/august.h"

#include "lastexpress/entities/alexei.h"
#include "lastexpress/entities/salko.h"
#include "lastexpress/entities/verges.h"

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

August::August(LastExpressEngine *engine) : Entity(engine, kEntityAugust) {
	ADD_CALLBACK_FUNCTION(August, reset);
	ADD_CALLBACK_FUNCTION(August, updateFromTime);
	ADD_CALLBACK_FUNCTION(August, draw);
	ADD_CALLBACK_FUNCTION(August, updatePosition);
	ADD_CALLBACK_FUNCTION(August, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(August, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION(August, enterExitCompartment3);
	ADD_CALLBACK_FUNCTION(August, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(August, callSavepoint);
	ADD_CALLBACK_FUNCTION(August, callSavepointNoDrawing);
	ADD_CALLBACK_FUNCTION(August, draw2);
	ADD_CALLBACK_FUNCTION(August, playSound);
	ADD_CALLBACK_FUNCTION(August, playSound16);
	ADD_CALLBACK_FUNCTION(August, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION(August, savegame);
	ADD_CALLBACK_FUNCTION(August, updateEntity);
	ADD_CALLBACK_FUNCTION(August, function17);
	ADD_CALLBACK_FUNCTION(August, updateEntity2);
	ADD_CALLBACK_FUNCTION(August, function19);
	ADD_CALLBACK_FUNCTION(August, function20);
	ADD_CALLBACK_FUNCTION(August, function21);
	ADD_CALLBACK_FUNCTION(August, chapter1);
	ADD_CALLBACK_FUNCTION(August, function23);
	ADD_CALLBACK_FUNCTION(August, dinner);
	ADD_CALLBACK_FUNCTION(August, chapter1Handler);
	ADD_CALLBACK_FUNCTION(August, function26);
	ADD_CALLBACK_FUNCTION(August, function27);
	ADD_CALLBACK_FUNCTION(August, function28);
	ADD_CALLBACK_FUNCTION(August, function29);
	ADD_CALLBACK_FUNCTION(August, restaurant);
	ADD_CALLBACK_FUNCTION(August, function31);
	ADD_CALLBACK_FUNCTION(August, function32);
	ADD_CALLBACK_FUNCTION(August, function33);
	ADD_CALLBACK_FUNCTION(August, function34);
	ADD_CALLBACK_FUNCTION(August, chapter2);
	ADD_CALLBACK_FUNCTION(August, chapter2Handler);
	ADD_CALLBACK_FUNCTION(August, function37);
	ADD_CALLBACK_FUNCTION(August, function38);
	ADD_CALLBACK_FUNCTION(August, function39);
	ADD_CALLBACK_FUNCTION(August, chapter3);
	ADD_CALLBACK_FUNCTION(August, function41);
	ADD_CALLBACK_FUNCTION(August, function42);
	ADD_CALLBACK_FUNCTION(August, chapter3Handler);
	ADD_CALLBACK_FUNCTION(August, function44);
	ADD_CALLBACK_FUNCTION(August, function45);
	ADD_CALLBACK_FUNCTION(August, function46);
	ADD_CALLBACK_FUNCTION(August, function47);
	ADD_CALLBACK_FUNCTION(August, function48);
	ADD_CALLBACK_FUNCTION(August, function49);
	ADD_CALLBACK_FUNCTION(August, function50);
	ADD_CALLBACK_FUNCTION(August, function51);
	ADD_CALLBACK_FUNCTION(August, function52);
	ADD_CALLBACK_FUNCTION(August, function53);
	ADD_CALLBACK_FUNCTION(August, function54);
	ADD_CALLBACK_FUNCTION(August, function55);
	ADD_CALLBACK_FUNCTION(August, function56);
	ADD_CALLBACK_FUNCTION(August, chapter4);
	ADD_CALLBACK_FUNCTION(August, chapter4Handler);
	ADD_CALLBACK_FUNCTION(August, function59);
	ADD_CALLBACK_FUNCTION(August, function60);
	ADD_CALLBACK_FUNCTION(August, function61);
	ADD_CALLBACK_FUNCTION(August, function62);
	ADD_CALLBACK_FUNCTION(August, function63);
	ADD_CALLBACK_FUNCTION(August, function64);
	ADD_CALLBACK_FUNCTION(August, function65);
	ADD_CALLBACK_FUNCTION(August, chapter5);
	ADD_CALLBACK_FUNCTION(August, chapter5Handler);
	ADD_CALLBACK_FUNCTION(August, function68);
	ADD_CALLBACK_FUNCTION(August, unhookCars);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, August, reset)
	Entity::reset(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(2, August, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, August, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SII(4, August, updatePosition, CarIndex, Position)
	Entity::updatePosition(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(5, August, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(6, August, enterExitCompartment2, ObjectIndex)
	Entity::enterExitCompartment(savepoint, kPosition_6470, kPosition_6130, kCarGreenSleeping, kObjectCompartment3, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(7, August, enterExitCompartment3, ObjectIndex)
	if (savepoint.action == kAction4) {
		getEntities()->exitCompartment(kEntityAugust, (ObjectIndex)params->param4);
		callbackAction();
		return;
	}

	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, August, callbackActionOnDirection)
	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SIIS(9, August, callSavepoint, EntityIndex, ActionIndex)
	Entity::callSavepoint(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_IIS(10, August, callSavepointNoDrawing, EntityIndex, ActionIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		if (!params->param6)
			getSavePoints()->call(kEntityAugust, (EntityIndex)params->param1, (ActionIndex)params->param2, (char *)&params->seq);

		callbackAction();
		break;

	case kAction10:
		if (!params->param6) {
			getSavePoints()->call(kEntityAugust, (EntityIndex)params->param1, (ActionIndex)params->param2, (char *)&params->seq);
			params->param6 = 1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SSI(11, August, draw2, EntityIndex)
	Entity::draw2(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(12, August, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(13, August, playSound16)
	Entity::playSound(savepoint, false, kFlagDefault);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, August, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(15, August, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(16, August, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kActionExcuseMeCath) {
		getProgress().eventMetAugust ? getSound()->playSound(kEntityPlayer, rnd(2) ? "CAT1002A" : "CAT1002") : getSound()->excuseMeCath();
		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(17, August, function17, TimeValue)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 < getState()->time && !params->param2) {
			params->param2 = 1;
			callbackAction();
			break;
		}

		if (getEntities()->isPlayerInCar(kCarGreenSleeping) || getEntities()->isPlayerInCar(kCarRedSleeping)) {
			if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping)) {
				setCallback(2);
				setup_updateEntity2(kCarGreenSleeping, kPosition_540);
			} else {
				setCallback(3);
				setup_updateEntity2(kCarRedSleeping, kPosition_9460);
			}
		}
		break;

	case kActionDefault:
		ENTITY_PARAM(0, 1) = 0;

		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_540);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (ENTITY_PARAM(0, 1)) {
				callbackAction();
				break;
			}

			getEntities()->clearSequences(kEntityAugust);
			break;

		case 2:
		case 3:
			if (ENTITY_PARAM(0, 1)) {
				callbackAction();
				break;
			}

			getEntities()->clearSequences(kEntityAugust);

			setCallback(4);
			setup_updateFromTime(450);
			break;

		case 4:
			setCallback(5);
			setup_updateEntity2(kCarRedSleeping, kPosition_540);
			break;

		case 5:
			if (ENTITY_PARAM(0, 1)) {
				callbackAction();
				break;
			}

			getEntities()->clearSequences(kEntityAugust);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(18, August, updateEntity2, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->updateEntity(_entityIndex, (CarIndex)params->param1, (EntityPosition)params->param2)) {
			callbackAction();
		} else if (getEntities()->isDistanceBetweenEntities(kEntityAugust, kEntityPlayer, 1000)
		        && !getEntities()->isInGreenCarEntrance(kEntityPlayer)
				&& !getEntities()->isInsideCompartments(kEntityPlayer)
				&& !getEntities()->checkFields10(kEntityPlayer)) {

			if (getData()->car == kCarGreenSleeping || getData()->car == kCarRedSleeping) {
				ENTITY_PARAM(0, 1) = 1;
				callbackAction();
			}
		}
		break;

	case kActionDefault:
		if (getEntities()->updateEntity(_entityIndex, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(19, August, function19, bool, bool)
	// Expose parameters as IISS and ignore the default exposed parameters
	EntityData::EntityParametersIISS *parameters = (EntityData::EntityParametersIISS*)_data->getCurrentParameters();

	switch (savepoint.action) {
	default:
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;
		getSound()->playSound(kEntityPlayer, "CAT1002");
		getSound()->playSound(kEntityAugust, "AUG3101", kFlagInvalid, 15);
		break;

	case kActionDefault:
		getData()->inventoryItem = kItemNone;

		switch (getProgress().chapter) {
		default:
			break;

		case kChapter1:
			strcpy((char *)&parameters->seq1, "626");
			break;

		case kChapter2:
		case kChapter3:
			if (getData()->clothes != kClothes2) {
				strcpy((char *)&parameters->seq1, "666");
				break;
			}
			// Fallback to next action

		case kChapter4:
		case kChapter5:
			strcpy((char *)&parameters->seq1, "696");
			break;
		}

		getSavePoints()->push(kEntityAugust, kEntityMertens, kAction303343617);

		strcpy((char *)&parameters->seq2, (char *)&parameters->seq1);
		strcat((char *)&parameters->seq2, "Pc");

		getEntities()->drawSequenceLeft(kEntityAugust, (char *)&parameters->seq2);
		getEntities()->enterCompartment(kEntityAugust, kObjectCompartment3, true);

		setCallback(1);
		setup_playSound("AUG2096");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			strcpy((char *)&parameters->seq2, (char *)&parameters->seq1);
			strcat((char *)&parameters->seq2, "Qc");

			getEntities()->drawSequenceLeft(kEntityAugust, (char *)&parameters->seq2);
			if (parameters->param2)
				getData()->inventoryItem = kItem147;
			break;

		case 2:
			strcpy((char *)&parameters->seq2, (char *)&parameters->seq1);
			strcat((char *)&parameters->seq2, parameters->param1 ? "Fc" : "Dc");

			setCallback(3);
			setup_enterExitCompartment((char *)&parameters->seq2, kObjectCompartment3);
			break;

		case 3:
			getEntities()->exitCompartment(kEntityAugust, kObjectCompartment3, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityAugust);

			callbackAction();
			break;
		}
		break;

	case kAction69239528:
		getData()->inventoryItem = kItemNone;

		setCallback(2);
		setup_updateFromTime(75);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(20, August, function20, bool)
	// Expose parameters as ISSI and ignore the default exposed parameters
	EntityData::EntityParametersISSI *parameters = (EntityData::EntityParametersISSI*)_data->getCurrentParameters();

	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		switch (getProgress().chapter) {
		default:
			break;

		case kChapter1:
			strcpy((char *)&parameters->seq1, "626");
			break;

		case kChapter2:
		case kChapter3:
			if (getData()->clothes != kClothes2) {
				strcpy((char *)&parameters->seq1, "666");
				break;
			}
			// Fallback to next case

		case kChapter4:
		case kChapter5:
			strcpy((char *)&parameters->seq1, "696");
			break;
		}

		if (params->param1) {
			strcpy((char *)&parameters->seq2, Common::String::format("%s%s", (char *)&parameters->seq1, "Gc").c_str());

			getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);
		} else {
			strcpy((char *)&parameters->seq2, Common::String::format("%s%s", (char *)&parameters->seq1, "Ec").c_str());
		}

		setCallback(1);
		setup_enterExitCompartment((char *)&parameters->seq2, kObjectCompartment3);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1: {
			getData()->location = kLocationOutsideCompartment;

			Common::String sequence2 = Common::String::format("%s%s", (char *)&parameters->seq2, "Pc");
			strcpy((char *)&parameters->seq2, (char *)&parameters->seq1);

			getEntities()->drawSequenceLeft(kEntityAugust, sequence2.c_str());
			getEntities()->enterCompartment(kEntityAugust, kObjectCompartment3, true);

			if (getProgress().chapter != kChapter3 || getState()->time >= kTime1998000) {
				setCallback(3);
				setup_playSound("AUG2095");
			} else {
				setCallback(2);
				setup_playSound("AUG2094");
			}
			}
			break;

		case 2:
		case 3:
			getSavePoints()->push(kEntityAugust, kEntityMertens, kAction269436673);
			strcpy((char *)&parameters->seq2, Common::String::format("%s%s", (char *)&parameters->seq1, "Qc").c_str());

			getEntities()->drawSequenceLeft(kEntityAugust, (char *)&parameters->seq2);
			break;
		}
		break;

	case kAction69239528:
		getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getEntities()->exitCompartment(kEntityAugust, kObjectCompartment3, true);

		callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(21, August, function21, TimeValue)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param4 && params->param1 < getState()->time && !params->param7) {
			params->param7 = 1;

			getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;
		}

		if (params->param2) {
			if (!Entity::updateParameter(params->param8, getState()->timeTicks, 75))
				goto label_continue;

			params->param2 = 0;
			params->param3 = 1;

			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorNormal, (getProgress().eventMetAugust || getProgress().jacket != kJacketGreen) ? kCursorNormal : kCursorHand);
		}

		params->param8 = 0;

label_continue:
		if (getProgress().chapter != kChapter1)
			break;

		if (params->param6) {
			if (Entity::updateParameter(CURRENT_PARAM(1, 1), getState()->time, 6300)) {
				params->param6 = 0;
				CURRENT_PARAM(1, 1) = 0;
			}
		}

		if (!params->param4
		 && !getProgress().eventMetAugust
		 && !params->param6
		 && (params->param1 - 4500) > getState()->time
		 && !getProgress().field_14) {
			getProgress().field_14 = 2;
			getData()->location = kLocationOutsideCompartment;
			getData()->entityPosition = kPosition_8200;

			setCallback(1);
			setup_function20(false);
		}
		break;

	case kActionOpenDoor:
		if (getProgress().chapter == kChapter1 && !getProgress().eventMetAugust && getProgress().jacket == kJacketGreen) {
			getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			getData()->location = kLocationInsideCompartment;

			setCallback(6);
			setup_savegame(kSavegameTypeEvent, kEventMeetAugustHisCompartment);
			break;
		}
		// Fallback to next case

	case kActionKnock:
		getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorNormal, kCursorNormal);

		if (params->param2) {
			if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(12);
				setup_playSound(rnd(2) ? getSound()->wrongDoorCath() : (rnd(2) ? "CAT1502" : "CAT1502A"));
			} else {
				setCallback(13);
				setup_playSound(getSound()->wrongDoorCath());
			}
		} else {
			setCallback(savepoint.action == kActionKnock ? 7 : 8);
			setup_playSound(savepoint.action == kActionKnock ?  "LIB012" : "LIB013");
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (params->param2 || params->param3) {
			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorHandKnock, kCursorHand);

			params->param2 = 0;
			params->param3 = 0;
			params->param5 = 0;
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_8200);
			break;

		case 2:
			setCallback(3);
			setup_function23((TimeValue)(params->param1 - 2700));
			break;

		case 3:
			setCallback(4);
			setup_updateEntity(kCarGreenSleeping, kPosition_6470);
			break;

		case 4:
			setCallback(5);
			setup_function19(false, false);
			break;

		case 5:
			if (getProgress().field_14 == 2)
				getProgress().field_14 = 0;

			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorHandKnock, kCursorHand);

			params->param2 = 0;
			params->param3 = 0;
			params->param5 = 0;
			params->param6 = 0;
			CURRENT_PARAM(1, 1) = 0;
			break;

		case 6:
			getAction()->playAnimation(getObjects()->get(kObjectCompartment3).location2 == kObjectLocation1 ? kEventMeetAugustHisCompartmentBed : kEventMeetAugustHisCompartment);
			getProgress().eventMetAugust = true;
			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorNormal, kCursorNormal);

			params->param2 = 0;
			params->param3 = 1;

			getScenes()->loadSceneFromObject(kObjectCompartment3, true);
			break;

		case 7:
		case 8:
			++params->param5;

			switch(params->param5) {
			default:
				// Fall to next case
				break;

			case 1:
				setCallback(9);
				setup_playSound(rnd(2) ? "AUG1128A" : "AUG1128B");
				return;

			case 2:
				setCallback(10);
				setup_playSound(getProgress().eventMetAugust ? "AUG1128E" : "AUG1128G");
				return;

			case 3:
				setCallback(11);
				setup_playSound(getProgress().eventMetAugust ? "AUG1128F" : "AUG1128H");
				return;
			}
			// Fallback to next case

		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorTalk, (getProgress().eventMetAugust || getProgress().jacket != kJacketGreen) ? kCursorNormal : kCursorHand);

			if (getCallback() == 12 || getCallback() == 13) {
				params->param2 = 0;
				params->param3 = 1;
			} else {
				params->param2= 1;
			}
			break;

		case 14:
			setCallback(15);
			setup_updateFromTime(75);
			break;

		case 15:
			setCallback(16);
			setup_playSound("AUG1128I");
			break;

		case 16:
			getSavePoints()->push(kEntityAugust, kEntityMertens, kAction100906246);
			break;

		case 17:
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityAugust, kEntityMertens, kAction156567128);
			getEntities()->drawSequenceLeft(kEntityAugust, "626Lc");
			getEntities()->enterCompartment(kEntityAugust, kObjectCompartment3, true);
			break;

		case 18:
			getEntities()->exitCompartment(kEntityAugust, kObjectCompartment3, true);
			getData()->location = kLocationInsideCompartment;  // BUG: in the original, this is set to 6470
			getEntities()->clearSequences(kEntityAugust);
			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorHandKnock, kCursorHand);

			params->param4 = 0;
			break;
		}
		break;

	case kAction124697504:
		getSound()->playSound(kEntityAugust, "CON1023A");

		setCallback(18);
		setup_enterExitCompartment("626Mc", kObjectCompartment3);
		break;

	case kAction192849856:
		setCallback(17);
		setup_enterExitCompartment("626Kc", kObjectCompartment3);
		break;

	case kAction221617184:
		params->param4 = 1;
		getSavePoints()->push(kEntityAugust, kEntityMertens, kAction102675536);

		setCallback(14);
		setup_playSound("CON1023");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, August, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(August, setup_chapter1Handler));
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject11, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		getData()->entityPosition = kPosition_4691;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;

		getProgress().eventMetAugust = false;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(23, August, function23, TimeValue)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getProgress().field_14 == 29 || getProgress().field_14 == 3) {
			if (params->param3) {
				getData()->location = kLocationOutsideCompartment;

				setCallback(2);
				setup_enterExitCompartment("626Ea", kObjectCompartment1);
			} else {
				getEntities()->exitCompartment(kEntityAugust, kObjectCompartment1, true);
				getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
				callbackAction();
			}
			break;
		}

		if (!params->param2) {

			if (!CURRENT_PARAM(1, 3))
				CURRENT_PARAM(1, 3) = getState()->timeTicks + 45;

			if (CURRENT_PARAM(1, 3) >= getState()->timeTicks)
				break;

			if (!params->param5) {
				setCallback(8);
				setup_playSound("AUG1002B");
				break;
			}

label_callback_8:
			if (Entity::updateParameter(CURRENT_PARAM(1, 4), getState()->timeTicks, 75)) {
				getEntities()->exitCompartment(kEntityAugust, kObjectCompartment1, true);

				if (getProgress().eventCorpseMovedFromFloor) {
					setCallback(9);
					setup_enterExitCompartment("626Da", kObjectCompartment1);
				} else if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping)) {
					setCallback(10);
					setup_enterExitCompartment3("626Da", kObjectCompartment1);
				} else {
					getScenes()->loadSceneFromPosition(kCarNone, 1);
					getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
					setCallback(11);
					setup_savegame(kSavegameTypeEvent, kEventAugustFindCorpse);
				}
				break;
			}

label_callback_9:
			if (params->param3 && params->param1 < getState()->time && !CURRENT_PARAM(1, 5)) {
				CURRENT_PARAM(1, 5) = 1;
				getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

				setCallback(12);
				setup_enterExitCompartment("626Ea", kObjectCompartment1);
			}
			break;
		}

		if (!CURRENT_PARAM(1, 1))
			CURRENT_PARAM(1, 1) = getState()->timeTicks + 45;

		if (CURRENT_PARAM(1, 1) >= getState()->timeTicks)
			break;

		if (getObjects()->get(kObjectCompartment1).location == kObjectLocation1) {
			if (!Entity::updateParameter(CURRENT_PARAM(1, 2), getState()->timeTicks, 75))
				break;

			getObjects()->update(kObjectCompartment1, kEntityAugust, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorNormal);

			params->param6++;

			switch (params->param6) {
			default:
				break;

			case 1:
				setCallback(5);
				setup_playSound("LIB013");
				return;

			case 2:
				setCallback(7);
				setup_playSound("LIB012");
				return;

			case 3:
				params->param8++;

				if (params->param8 >= 3) {
					getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorHandKnock, kCursorHand);
					callbackAction();
					break;
				}

				params->param6 = 0;
			}

			getObjects()->update(kObjectCompartment1, kEntityAugust, getObjects()->get(kObjectCompartment1).location, params->param4 ? kCursorNormal : kCursorTalk, kCursorHand);
			CURRENT_PARAM(1, 2) = 0;
		} else {

			if (getProgress().eventCorpseMovedFromFloor && getProgress().jacket != kJacketBlood) {
				params->param7 = (getObjects()->get(kObjectCompartment1).location2 == kObjectLocation1) ? 8 : 7;
				getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

				setCallback(4);
				setup_savegame(kSavegameTypeEvent, kEventMeetAugustTylerCompartment);
			} else {
				getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

				setCallback(3);
				setup_savegame(kSavegameTypeEvent, kEventAugustFindCorpse);
			}
		}
		break;

	case kActionKnock:
		if (params->param3) {
			getObjects()->update(kObjectCompartment1, kEntityAugust, kObjectLocationNone, kCursorNormal, kCursorNormal);

			setCallback(15);
			setup_playSound("LIB012");
		} else if (!params->param4) {
			getObjects()->update(kObjectCompartment1, kEntityAugust, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorNormal);

			setCallback(17);
			setup_playSound("AUG1002A");
		}
		break;

	case kActionOpenDoor:
		if (getProgress().eventCorpseMovedFromFloor && getProgress().jacket != kJacketBlood) {
			if (params->param3) {
				getData()->location = kLocationInsideCompartment;

				params->param7 = (getObjects()->get(kObjectCompartment1).location2 == kObjectLocation1) ? kEventMeetAugustHisCompartmentBed : kEventMeetAugustHisCompartment;
			} else {
				params->param7 = (getObjects()->get(kObjectCompartment1).location2 == kObjectLocation1) ? kEventMeetAugustTylerCompartmentBed : kEventMeetAugustTylerCompartment;
			}

			setCallback(14);
			setup_savegame(kSavegameTypeEvent, kEventMeetAugustTylerCompartment);
		} else {
			getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

			setCallback(13);
			setup_savegame(kSavegameTypeEvent, kEventAugustFindCorpse);
		}
		break;

	case kActionDefault:
		if (getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_8200)
		 || getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_7850)
		 || getEntities()->isOutsideAlexeiWindow()) {
			getObjects()->update(kObjectCompartment1, kEntityAugust, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorNormal);

			if (getEntities()->isOutsideAlexeiWindow())
				getScenes()->loadSceneFromPosition(kCarGreenSleeping, 49);

			getSound()->playSound(kEntityPlayer, "LIB012");

			getObjects()->update(kObjectCompartment1, kEntityAugust, getObjects()->get(kObjectCompartment1).location, kCursorTalk, kCursorHand);

			params->param2 = 1;
		} else {
			setCallback(1);
			setup_enterExitCompartment("626Aa", kObjectCompartment1);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kEntityAugust, "626Ba");
			getEntities()->enterCompartment(kEntityAugust, kObjectCompartment1, true);
			break;

		case 2:
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			callbackAction();
			break;

		case 3:
			getSound()->playSound(kEntityPlayer, "LIB014");
			getAction()->playAnimation(kEventAugustFindCorpse);
			if (getEvent(kEventDinerAugustOriginalJacket))
				getLogic()->gameOver(kSavegameTypeEvent2, kEventDinerAugustOriginalJacket, getProgress().eventCorpseFound ? kSceneGameOverStopPolice : kSceneGameOverPolice, true);
			else if (getProgress().eventCorpseMovedFromFloor)
				getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			else
				getLogic()->gameOver(kSavegameTypeIndex, 1, getProgress().eventCorpseFound ? kSceneGameOverStopPolice : kSceneGameOverPolice, true);
			break;

		case 4:
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getSound()->playSound(kEntityPlayer, "LIB014");
			getEntities()->clearSequences(kEntityAugust);
			getData()->location = kLocationInsideCompartment;

			getAction()->playAnimation((EventIndex)params->param7);
			getSound()->playSound(kEntityPlayer, "LIB015");
			getProgress().eventMetAugust = true;
			getData()->location = kLocationOutsideCompartment;

			getScenes()->loadScene(kScene41);

			callbackAction();
			break;

		case 5:
			setCallback(6);
			setup_playSound16("AUG1002B");
			break;

		case 6:
		case 7:
			getObjects()->update(kObjectCompartment1, kEntityAugust, getObjects()->get(kObjectCompartment1).location, params->param4 ? kCursorNormal : kCursorTalk, kCursorHand);
			ENTITY_PARAM(1, 2) = 0;
			break;

		case 8:
			params->param5 = 1;
			goto label_callback_8;

		case 9:
			params->param3 = 1;
			getEntities()->clearSequences(kEntityAugust);
			getData()->location = kLocationInsideCompartment;
			getObjects()->update(kObjectCompartment1, kEntityAugust, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			goto label_callback_9;

		case 10:
			getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			setCallback(11);
			setup_savegame(kSavegameTypeEvent, kEventAugustFindCorpse);
			break;

		case 11:
			getAction()->playAnimation(kEventAugustFindCorpse);

			getLogic()->gameOver(getEvent(kEventDinerAugustOriginalJacket) ? kSavegameTypeEvent2 : kSavegameTypeIndex,
								 getEvent(kEventDinerAugustOriginalJacket) ? kEventDinerAugustOriginalJacket : 1,
								 getProgress().eventCorpseFound ? kSceneGameOverStopPolice : kSceneGameOverPolice,
								 true);
			break;

		case 12:
			getData()->location = kLocationOutsideCompartment;
			callbackAction();
			break;

		case 13:
			getSound()->playSound(kEntityPlayer, getObjects()->get(kObjectCompartment1).location == kObjectLocation1 ? "LIB032" : "LIB014");
			getAction()->playAnimation(kEventAugustFindCorpse);

			if (getEvent(kEventDinerAugustOriginalJacket))
				getLogic()->gameOver(kSavegameTypeEvent2, kEventDinerAugustOriginalJacket, getProgress().eventCorpseFound ? kSceneGameOverStopPolice : kSceneGameOverPolice, true);
			else if (getProgress().eventCorpseMovedFromFloor)
				getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			else
				getLogic()->gameOver(kSavegameTypeIndex, 1, getProgress().eventCorpseFound ? kSceneGameOverStopPolice : kSceneGameOverPolice, true);
			break;

		case 14:
			if (!params->param2)
				getSound()->playSound(kEntityPlayer, getObjects()->get(kObjectCompartment1).location == kObjectLocation1 ? "LIB032" : "LIB014");

			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

			getAction()->playAnimation((EventIndex)params->param7);
			getProgress().eventMetAugust = true;
			getData()->location = kLocationOutsideCompartment;

			getScenes()->loadScene(kScene41);

			callbackAction();
			break;

		case 15:
			setCallback(16);
			setup_playSound("AUG1128A");
			break;

		case 16:
			getObjects()->update(kObjectCompartment1, kEntityAugust, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			break;

		case 17:
			params->param4 = 1;
			getObjects()->update(kObjectCompartment1, kEntityAugust, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorHand);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, August, dinner)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventDinerAugust);
		break;

	case kActionCallback:
		if (getCallback() == 1) {

			getAction()->playAnimation(getEntities()->isInRestaurant(kEntityAlexei) ? kEventDinerAugustAlexeiBackground : kEventDinerAugust);
			getProgress().eventMetAugust = true;

			getScenes()->loadSceneFromPosition(kCarRestaurant, 61);

			callbackAction();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, August, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1 && getProgress().eventCorpseFound) {
			getSavePoints()->push(kEntityAugust, kEntityPascale, kAction239072064);
			params->param1 = 1;
		}

		if (getState()->time > kTime1080000 && !params->param3) {
			params->param3 = 1;

			if (!params->param1) {
				getSavePoints()->push(kEntityAugust, kEntityPascale, kAction239072064);
				params->param1 = 1;
			}
		}

		if (getState()->time > kTime1093500 && getEntities()->isSomebodyInsideRestaurantOrSalon()) {
			getData()->location = kLocationOutsideCompartment;
			getData()->inventoryItem = kItemNone;

			setCallback(1);
			setup_callSavepoint("010J", kEntityTables3, kActionDrawTablesWithChairs, "010K");
		}
		break;

	case kAction1:
		params->param2 = 0;
		getData()->inventoryItem = kItemNone;
		getSavePoints()->push(kEntityAugust, kEntityPascale, kAction191604416);

		if (getProgress().jacket == kJacketGreen) {
			setCallback(3);
			setup_dinner();
		} else {
			setCallback(4);
			setup_savegame(kSavegameTypeEvent, kEventDinerAugustOriginalJacket);
		}
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityAugust, kEntityTables3, kAction136455232);
		getEntities()->drawSequenceLeft(kEntityAugust, "010B");

		if (!getProgress().eventMetAugust)
			params->param2 = kItemInvalid;

		getData()->inventoryItem = (InventoryItem)params->param2;
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityAugust, kEntityServers0, kAction204704037);
			getEntities()->drawSequenceRight(kEntityAugust, "803DS");
			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityAugust);

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			setup_function26();
			break;

		case 3:
			setup_function28();
			break;

		case 4:
			getSavePoints()->push(kEntityAugust, kEntityAlexei, kAction225182640);
			getAction()->playAnimation(kEventDinerAugustOriginalJacket);
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocation3, kCursorNormal, kCursorNormal);

			getData()->location = kLocationOutsideCompartment;

			getSavePoints()->push(kEntityAugust, kEntityTables3, kActionDrawTablesWithChairs, "010K");
			getEntities()->drawSequenceRight(kEntityAugust, "010P");
			getScenes()->loadSceneFromPosition(kCarRestaurant, 65);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getSavePoints()->push(kEntityAugust, kEntityServers0, kAction204704037);
			getEntities()->drawSequenceRight(kEntityAugust, "803DS");
			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityAugust);

			setCallback(6);
			setup_callbackActionOnDirection();
			break;

		case 6:
			getProgress().field_14 = 2;

			setCallback(7);
			setup_updateEntity(kCarGreenSleeping, kPosition_8200);
			break;

		case 7:
			setCallback(8);
			setup_function23(kTimeNone);
			break;

		case 8:
			getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, true);
			break;
		}
		break;

	case kAction168046720:
		getData()->inventoryItem = kItemNone;
		break;

	case kAction168627977:
		getData()->inventoryItem = (InventoryItem)params->param2;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, August, function26)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (getProgress().eventMetAugust || getProgress().field_14) {
			setCallback(5);
			setup_updateEntity(kCarGreenSleeping, kPosition_6470);
		} else {
			getProgress().field_14 = 2;
			setCallback(1);
			setup_updateEntity(kCarGreenSleeping, kPosition_8200);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function23((TimeValue)(getState()->time + 13500));
			break;

		case 2:
			setCallback(3);
			setup_updateEntity(kCarGreenSleeping, kPosition_6470);
			break;

		case 3:
			setCallback(4);
			setup_function19(false, false);
			break;

		case 4:
			if (getProgress().field_14 == 2)
				getProgress().field_14 = 0;

			setCallback(7);
			setup_function21((TimeValue)(getState()->time + 900));
			break;

		case 5:
			setCallback(6);
			setup_function19(false, false);
			break;

		case 6:
			setCallback(7);
			setup_function21((TimeValue)(getState()->time + 900));
			break;

		case 7:
			setup_function27();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, August, function27)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function20(false);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_draw("803US");
			break;

		case 4:
			getEntities()->drawSequenceRight(kEntityAugust, "010A");
			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityAugust);

			setCallback(5);
			setup_callSavepointNoDrawing(kEntityTables3, kAction136455232, "BOGUS");
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			setup_function28();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, August, function28)
	switch (savepoint.action) {
	default:
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;
		params->param1 = 0;

		setCallback(3);
		setup_dinner();
		break;

	case kActionDefault:
		if (!getProgress().eventMetAugust && getProgress().jacket == kJacketGreen)
			params->param1 = kItemInvalid;

		getEntities()->drawSequenceLeft(kEntityAugust, "010B");
		getSavePoints()->push(kEntityAugust, kEntityServers0, kAction304061224);
		getData()->inventoryItem = (InventoryItem)params->param1;
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityAugust, kEntityServers0, kAction203859488);
			getData()->inventoryItem = (InventoryItem)params->param1;
			getEntities()->drawSequenceLeft(kEntityAugust, "010B");
			break;

		case 2:
			getSavePoints()->push(kEntityAugust, kEntityServers0, kAction136702400);
			getEntities()->drawSequenceLeft(kEntityAugust, "010B");
			setup_function29();
			break;
		}
		break;

	case kAction168046720:
		getData()->inventoryItem = kItemNone;
		break;

	case kAction168627977:
		getData()->inventoryItem = (InventoryItem)params->param1;
		break;

	case kAction170016384:
		getData()->inventoryItem = kItemNone;
		getEntities()->drawSequenceLeft(kEntityServers0, "BLANK");
		getEntities()->drawSequenceLeft(kEntityAugust, "010G");

		setCallback(2);
		setup_playSound("AUG1053");
		break;

	case kAction268773672:
		getData()->inventoryItem = kItemNone;
		getEntities()->drawSequenceLeft(kEntityAugust, "010D");

		setCallback(1);
		setup_playSound("AUG1052");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, August, function29)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!getProgress().field_28 || (params->param2 && params->param3 == kTimeInvalid))
			break;

		if (getState()->time < kTime1134000) {

			if (!getEntities()->isInRestaurant(kEntityPlayer)
			 || getSoundQueue()->isBuffered("MRB1076") || getSoundQueue()->isBuffered("MRB1078") || getSoundQueue()->isBuffered("MRB1078A"))
				params->param3 = (uint)getState()->time + 225;

			if (params->param3 > getState()->time)
				break;
		}

		params->param3 = kTimeInvalid;
		getData()->inventoryItem = kItemNone;
		getProgress().field_28 = 0;

		setup_restaurant();
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;
		params->param1 = kItemNone;

		setCallback(1);
		setup_dinner();
		break;

	case kActionDefault:
		if (!getProgress().eventMetAugust && getProgress().jacket == kJacketGreen)
			params->param1 = kItemInvalid;

		getData()->inventoryItem = (InventoryItem)LOW_BYTE(params->param1);

		getEntities()->drawSequenceLeft(kEntityAugust, "010H");
		break;

	case kAction168046720:
		getData()->inventoryItem = kItemNone;
		break;

	case kAction168627977:
		getData()->inventoryItem = (InventoryItem)LOW_BYTE(params->param1);
		break;

	case kAction189426612:
		params->param2 = 1;
		break;

	case kAction235257824:
		params->param2 = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, August, restaurant)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param3, getState()->timeTicks, 75))
			break;

		getData()->inventoryItem = kItemInvalid;
		break;

	case kAction1:
		params->param1 = 1;
		getData()->inventoryItem = kItemNone;
		getScenes()->loadSceneFromPosition(kCarRestaurant, 62);
		getEntities()->updatePositionEnter(kEntityAugust, kCarRestaurant, 61);
		getEntities()->updatePositionEnter(kEntityAugust, kCarRestaurant, 64);
		break;

	case kActionEndSound:
		if (params->param1) {
			getData()->inventoryItem = kItemNone;
			getEntities()->updatePositionExit(kEntityAugust, kCarRestaurant, 61);
			getEntities()->updatePositionExit(kEntityAugust, kCarRestaurant, 64);

			setCallback(4);
			setup_savegame(kSavegameTypeEvent, kEventAugustPresentAnna);
			break;
		}

		if (params->param2) {
			params->param2 = 0;
			if (getProgress().eventMetAugust)
				getData()->inventoryItem = kItemNone;

			getSound()->playSound(kEntityAugust, "Aug1003A");
		} else {
			getData()->inventoryItem = kItemNone;
			getSavePoints()->push(kEntityAugust, kEntityAnna, kAction201437056);

			setCallback(8);
			setup_draw("010P");
		}
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityAugust, kEntityBoutarel, kAction135854206);

		setCallback(1);
		setup_updateFromTime(450);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 2:
			getSavePoints()->push(kEntityAugust, kEntityAnna, kAction259136835);
			getData()->location = kLocationOutsideCompartment;

			setCallback(3);
			setup_callSavepoint("010N", kEntityTables3, kActionDrawTablesWithChairs, "010K");
			break;

		case 3:
			getSavePoints()->push(kEntityAugust, kEntityServers0, kAction292758554);
			getSavePoints()->push(kEntityAugust, kEntityAnna, kAction122358304);
			getEntities()->drawSequenceLeft(kEntityAugust, "001K");
			getSound()->playSound(kEntityAugust, "AUG1003");

			if (getEntities()->isInRestaurant(kEntityPlayer))
				getProgress().field_60 = 1;

			params->param2 = 1;
			break;

		case 4:

			break;

		case 5:
		case 7:
		case 9:
			getSavePoints()->push(kEntityAugust, kEntityBoutarel, kAction134466544);

			setup_function31();
			break;

		case 6:
		case 8:
			getEntities()->drawSequenceRight(kEntityAugust, "803DS");
			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityAugust);

			setCallback(getCallback() + 1);
			setup_callbackActionOnDirection();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, August, function31)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_6470);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function19(false, false);
			break;

		case 2:
			setCallback(2);
			setup_function21(kTime1161000);
			break;

		case 3:
		case 4:
			if (getProgress().field_14 == 29) {
				setCallback(4);
				setup_function21((TimeValue)(getState()->time + 900));
			} else {
				setup_function32();
			}
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, August, function32)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::updateParameterTime(kTime1179000, (!getEntities()->isInSalon(kEntityAnna) || getEntities()->isInSalon(kEntityPlayer)), params->param6, 0)) {
			getSavePoints()->push(kEntityAugust, kEntityAnna, kAction123712592);
		}

		if (params->param1 && getEntities()->isSomebodyInsideRestaurantOrSalon()) {
			if (!params->param4) {
				params->param4 = (uint)getState()->time + 1800;
				params->param5 = (uint)getState()->time + 9000;
			}

			if (params->param7 != kTimeInvalid && params->param4 < getState()->time) {
				if (Entity::updateParameterTime((TimeValue)params->param5, getEntities()->isInSalon(kEntityPlayer), params->param7, 0)) {
					getData()->location = kLocationOutsideCompartment;

					setCallback(5);
					setup_updatePosition("109D", kCarRestaurant, 56);
					break;
				}
			}
		}

		if (params->param3) {
			if (!Entity::updateParameter(params->param8, getState()->timeTicks, 90))
				break;

			getScenes()->loadSceneFromPosition(kCarRestaurant, 55);
		} else {
			params->param8 = 0;
		}
		break;

	case kActionDefault:
		setCallback(1);
		setup_function20(false);
		break;

	case kActionDrawScene:
		if (params->param2) {
			if (getEntities()->isPlayerPosition(kCarRestaurant, 57)) {
				getScenes()->loadSceneFromPosition(kCarRestaurant, 50);
				params->param3 = true;
			} else if (!getEntities()->isPlayerPosition(kCarRestaurant, 50)) {
				params->param3 = false;
			}
		} else {
			params->param3 = getEntities()->isPlayerPosition(kCarRestaurant, 56) && params->param1;
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_updatePosition("105A", kCarRestaurant, 57);
			break;

		case 4:
			getData()->location = kLocationInsideCompartment;
			getEntities()->drawSequenceLeft(kEntityAugust, "105B");
			params->param2 = 1;
			break;

		case 5:
			setCallback(6);
			setup_updateEntity(kCarGreenSleeping, kPosition_6470);
			break;

		case 6:
			setCallback(7);
			setup_function19(false, false);
			break;

		case 7:
			setup_function33();
			break;
		}
		break;

	case kAction122358304:
		params->param2 = 0;
		getEntities()->drawSequenceLeft(kEntityAugust, "BLANK");
		break;

	case kAction159332865:
		getEntities()->drawSequenceLeft(kEntityAugust, "106E");
		params->param1 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, August, function33)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(getProgress().eventMetAugust ? 1 : 2);
		setup_function21(getProgress().eventMetAugust ? (TimeValue)(getState()->time + 9000) : kTimeBedTime);
		break;

	case kActionCallback:
		if (getCallback() == 1 || getCallback() == 2)
			setup_function34();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(34, August, function34)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!getSoundQueue()->isBuffered(kEntityAugust) && getProgress().field_18 != 4)
			getSound()->playSound(kEntityAugust, "AUG1057");    // August snoring
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;

		getEntities()->clearSequences(kEntityAugust);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(35, August, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter2Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAugust);

		getData()->entityPosition = kPosition_3970;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes1;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject11, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(36, August, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheckSavepoint(kTime1755000, params->param2, kEntityAugust, kEntityServers0, kAction252568704);

		if (getState()->time > kTime1773000 && params->param1 && getEntities()->isSomebodyInsideRestaurantOrSalon()) {
			getData()->inventoryItem = kItemNone;
			getData()->location = kLocationOutsideCompartment;
			getEntities()->updatePositionEnter(kEntityAugust, kCarRestaurant, 62);

			setCallback(2);
			setup_callSavepoint("016C", kEntityTables0, kActionDrawTablesWithChairs, "016D");
		}
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventAugustGoodMorning);
		break;

	case kActionDefault:
		if (!getEvent(kEventAugustGoodMorning))
				getData()->inventoryItem = kItemInvalid;

		getSavePoints()->push(kEntityAugust, kEntityTables0, kAction136455232);
		getEntities()->drawSequenceLeft(kEntityAugust, "016B");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventAugustGoodMorning);
			getScenes()->loadSceneFromPosition(kCarRestaurant, 61);
			break;

		case 2:
			getEntities()->updatePositionExit(kEntityAugust, kCarRestaurant, 62);
			getEntities()->drawSequenceRight(kEntityAugust, "803ES");
			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityAugust);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			getSavePoints()->push(kEntityAugust, kEntityServers0, kAction286534136);

			setCallback(4);
			setup_updateEntity(kCarGreenSleeping, kPosition_6470);
			break;

		case 4:
			setCallback(5);
			setup_function19(true, false);
			break;

		case 5:
			setup_function37();
			break;

		case 6:
			if (!getEvent(kEventAugustGoodMorning))
				getData()->inventoryItem = kItemInvalid;

			getSavePoints()->push(kEntityAugust, kEntityServers0, kAction219522616);
			getEntities()->drawSequenceLeft(kEntityAugust, "016B");
			params->param1 = 1;
			break;
		}
		break;

	case kAction123712592:
		getEntities()->drawSequenceLeft(kEntityAugust, "016A");
		getData()->inventoryItem = kItemNone;

		setCallback(6);
		setup_playSound("AUG2113");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(37, August, function37)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheckCallback(kTime1791000, params->param2, 5, true, WRAP_SETUP_FUNCTION_B(August, setup_function20));
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
		getEntities()->drawSequenceLeft(kEntityAugust, "506A2");
		break;

	case kActionDrawScene:
		if (getState()->time > kTime1786500 && getEntities()->isPlayerPosition(kCarGreenSleeping, 43)) {
			if (params->param1) {
				setCallback(2);
				setup_draw("506C2");
			} else {
				params->param1 = 1;

				setCallback(1);
				setup_draw("506B2");
			}
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 16);
			break;

		case 2:
			setCallback(3);
			setup_function20(true);
			break;

		case 3:
		case 5:
			setCallback(getCallback() + 1);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 4:
		case 6:
			setup_function38();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(38, August, function38)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheckSavepoint(kTime1801800, params->param1, kEntityAugust, kEntityRebecca, kAction155980128);

		Entity::timeCheckCallback(kTime1820700, params->param2, 3, WRAP_SETUP_FUNCTION(August, setup_callbackActionRestaurantOrSalon));
		break;

	case kActionDefault:
		setCallback(1);
		setup_callbackActionRestaurantOrSalon();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_updatePosition("109A", kCarRestaurant, 56);
			break;

		case 2:
			getScenes()->loadSceneFromItemPosition(kItem3);
			getData()->location = kLocationInsideCompartment;
			break;

		case 3:
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_updatePosition("109D2", kCarRestaurant, 56);
			break;

		case 4:
			getInventory()->setLocationAndProcess(kItem3, kObjectLocation1);

			setCallback(5);
			setup_function17(kTime1849500);
			break;

		case 5:
			setup_function39();
			break;

		case 6:
			setCallback(7);
			setup_playSound("AUG2114");
			break;

		case 7:
			getEntities()->drawSequenceLeft(kEntityAugust, "108C");
			getEntities()->updatePositionEnter(kEntityAugust, kCarRestaurant, 56);
			getEntities()->updatePositionEnter(kEntityAugust, kCarRestaurant, 57);

			setCallback(8);
			setup_playSound("AUG2114A");
			break;

		case 8:
			setCallback(9);
			setup_playSound("AUG2115");
			break;

		case 9:
			setCallback(10);
			setup_draw2("108D1", "108D2", kEntityRebecca);
			break;

		case 10:
			getEntities()->drawSequenceLeft(kEntityAugust, "109B");
			getEntities()->updatePositionExit(kEntityAugust, kCarRestaurant, 56);
			getEntities()->updatePositionExit(kEntityAugust, kCarRestaurant, 57);
			getSavePoints()->push(kEntityAugust, kEntityRebecca, kAction125496184);
			break;
		}
		break;

	case kAction169358379:
		getSavePoints()->push(kEntityAugust, kEntityRebecca, kAction155465152);
		getEntities()->drawSequenceLeft(kEntityAugust, "108A");

		setCallback(6);
		setup_updateFromTime(900);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(39, August, function39)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (!ENTITY_PARAM(0, 1))
			getSound()->playSound(kEntityPlayer, "BUMP");

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventAugustArrivalInMunich);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventAugustArrivalInMunich);
			getSavePoints()->push(kEntityAugust, kEntityChapters, kActionChapter3);
			getEntities()->clearSequences(kEntityAugust);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(40, August, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAugust);

		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;
		getData()->clothes = kClothes1;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(41, August, function41, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param3 && getEntities()->isDistanceBetweenEntities(kEntityAugust, kEntityPlayer, 2000))
			getData()->inventoryItem = kItemInvalid;
		else
			getData()->inventoryItem = kItemNone;

		if (getEntities()->updateEntity(kEntityAugust, (CarIndex)params->param1, (EntityPosition)params->param2)) {
			callbackAction();
			break;
		}

		if (!getEvent(kEventAugustMerchandise)
		 && getEntities()->isDistanceBetweenEntities(kEntityAugust, kEntityPlayer, 1000)
		 && !getEntities()->isInsideCompartments(kEntityPlayer)
		 && !getEntities()->checkFields10(kEntityPlayer)) {
			if (getData()->car == kCarGreenSleeping || getData()->car == kCarRedSleeping) {
				getAction()->playAnimation(kEventAugustMerchandise);

				getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + (750 * (getData()->direction == kDirectionUp ? -1 : 1))), getData()->direction == kDirectionUp);
			}
		}
		break;

	case kAction1:
		params->param3 = kItemNone;
		getData()->inventoryItem = kItemNone;

		getAction()->playAnimation((getData()->entityPosition < getEntityData(kEntityPlayer)->entityPosition) ? kEventAugustTalkGoldDay : kEventAugustTalkGold);
		getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + (750 * (getData()->direction == kDirectionUp ? -1 : 1))), getData()->direction == kDirectionUp);
		break;

	case kActionExcuseMeCath:
		if (getProgress().eventMetAugust)
			getSound()->playSound(kEntityPlayer, rnd(2) ? "CAT1002" : "CAT1002A");
		else
			getSound()->excuseMeCath();
		break;

	case kActionExcuseMe:
		getSound()->excuseMe(kEntityAugust);
		break;

	case kActionDefault:
		if (getEntities()->updateEntity(kEntityAugust, (CarIndex)params->param1, (EntityPosition)params->param2)) {
			callbackAction();
			break;
		}

		if (getEvent(kEventAugustMerchandise) && !getEvent(kEventAugustTalkGold) && !getEvent(kEventAugustTalkGoldDay))
			params->param3 = kItemInvalid;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_III(42, August, function42, CarIndex, EntityPosition, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param4 && getEntities()->isDistanceBetweenEntities(kEntityAugust, kEntityPlayer, 2000))
			getData()->inventoryItem = kItemInvalid;
		else
			getData()->inventoryItem = kItemNone;

		if (getEntities()->updateEntity(kEntityAugust, (CarIndex)params->param1, (EntityPosition)params->param2)) {
			getData()->inventoryItem = kItemNone;

			callbackAction();
		}
		break;

	case kAction1:
		params->param4 = 0;
		getData()->inventoryItem = kItemNone;

		getSound()->playSound(kEntityPlayer, "CAT1002");
		getSound()->playSound(kEntityAugust, getEvent(kEventAugustBringBriefcase) ? "AUG3103" : "AUG3100", kFlagInvalid, 15);
		break;

	case kActionExcuseMe:
		if (!getSoundQueue()->isBuffered(kEntityAugust))
			getSound()->excuseMe(kEntityAugust);
		break;

	case kActionDefault:
		if (getEntities()->updateEntity(kEntityAugust, (CarIndex)params->param1, (EntityPosition)params->param2)) {
			callbackAction();
			break;
		}

		if (params->param3) {
			params->param4 = 128;

			if (!getEvent(kEventAugustBringBriefcase))
				params->param4 = 147;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(43, August, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheckSavepoint(kTime1953000, params->param2, kEntityAugust, kEntityAnna, kAction291662081);

		// Set as same position as Anna
		if (params->param1) {
			getData()->entityPosition = getEntityData(kEntityAnna)->entityPosition;
			getData()->car = getEntityData(kEntityAnna)->car;
		}

		if (getState()->time > kTime2016000 && !params->param1) {
			if (getEntities()->isSomebodyInsideRestaurantOrSalon()) {
				getData()->inventoryItem = kItemNone;
				setup_function44();
			}
		}
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;

		setCallback(6);
		setup_savegame(kSavegameTypeEvent, kEventAugustLunch);
		break;

	case kActionDefault:
		setCallback(1);
		setup_function20(true);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function41(kCarRestaurant, kPosition_850);
			break;

		case 2:
			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_draw("803VS");
			break;

		case 4:
			getEntities()->drawSequenceRight(kEntityAugust, "010A2");

			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityAugust);

			setCallback(5);
			setup_callSavepointNoDrawing(kEntityTables3, kAction136455232, "BOGUS");
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			getEntities()->drawSequenceLeft(kEntityAugust, "010B2");

			if (!getEvent(kEventAugustLunch))
				getData()->inventoryItem = kItemInvalid;
			break;

		case 6:
			getAction()->playAnimation(kEventAugustLunch);
			getScenes()->processScene();
			break;
		}
		break;

	case kAction122288808:
		params->param1 = 0;
		getData()->inventoryItem = kItemNone;
		getData()->location = kLocationInsideCompartment;

		getEntities()->drawSequenceLeft(kEntityAugust, "112G");
		break;

	case kAction122358304:
		params->param1 = 1;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(44, August, function44)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_updatePosition("122H", kCarRestaurant, 57);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getEvent(kEventAugustMerchandise)) {
				setCallback(4);
				setup_function41(kCarGreenSleeping, kPosition_6470);
			} else {
				setCallback(2);
				setup_function17(kTime2043000);
			}
			break;

		case 2:
			if (!ENTITY_PARAM(0, 1)) {
				setCallback(4);
				setup_function41(kCarGreenSleeping, kPosition_6470);
			} else {
				setCallback(3);
				setup_savegame(kSavegameTypeEvent, kEventAugustMerchandise);
			}
			break;

		case 3:
			getAction()->playAnimation(kEventAugustMerchandise);
			if (getData()->car == kCarGreenSleeping && getEntities()->checkDistanceFromPosition(kEntityAugust, kPosition_6470, 500))
				getData()->entityPosition = kPosition_5970;

			getEntities()->updateEntity(kEntityAugust, kCarGreenSleeping, kPosition_6470);

			getEntities()->loadSceneFromEntityPosition(getData()->car,
			                                           (EntityPosition)(getData()->entityPosition + 750 * (getData()->direction == kDirectionUp ? -1 : 1)),
													   getData()->direction == kDirectionUp);

			setCallback(4);
			setup_function41(kCarGreenSleeping, kPosition_6470);
			break;

		case 4:
			setCallback(5);
			setup_function19(true, false);
			break;

		case 5:
			setup_function45();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(45, August, function45)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime2061000 && !params->param1) {
			params->param1 = 1;
			getData()->inventoryItem = kItemNone;

			setup_function46();
		}
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;
		getSound()->playSound(kEntityPlayer, "CAT1002");
		getSound()->playSound(kEntityAugust, "AUG3102", kFlagInvalid, 15);
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
		getEntities()->drawSequenceLeft(kEntityAugust, "506A2");
		getData()->inventoryItem = kItem146;	// TODO which item is that?
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(46, August, function46)
	switch (savepoint.action) {
	default:
		Entity::timeCheckCallback(kTime2088000, params->param1, 1, WRAP_SETUP_FUNCTION(August, setup_function47));
		break;

	case kActionNone:
		break;

	case kActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarGreenSleeping, 43)) {
			setCallback(2);
			setup_draw("507B2");
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setup_function48();
			break;

		case 2:
			if (getEntities()->isPlayerPosition(kCarGreenSleeping, 43))
				getScenes()->loadSceneFromPosition(kCarGreenSleeping, 34);

			getEntities()->clearSequences(kEntityAugust);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(47, August, function47)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function20(true);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function41(kCarGreenSleeping, kPosition_9460);
			break;

		case 2:
			getEntities()->clearSequences(kEntityAugust);
			setCallback(3);
			setup_updateFromTime(2700);
			break;

		case 3:
			setCallback(4);
			setup_function41(kCarGreenSleeping, kPosition_6470);
			break;

		case 4:
			setCallback(5);
			setup_function19(false, false);
			break;

		case 5:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(48, August, function48)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeCityLinz, params->param1, WRAP_SETUP_FUNCTION(August, setup_function49));
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (!getEvent(kEventAugustTalkCompartmentDoor) && !getEvent(kEventAugustTalkCompartmentDoorBlueRedingote)
		 && !getEvent(kEventAugustBringEgg) && !getEvent(kEventAugustBringBriefcase)) {

			if (savepoint.action == kActionKnock)
				getSound()->playSound(kEntityPlayer, "LIB012");

			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventAugustTalkCompartmentDoor);
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getData()->clothes = kClothes2;
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventAugustTalkCompartmentDoor);
			getScenes()->processScene();

			setCallback(2);
			setup_function21(kTimeCityLinz);
			break;

		case 2:
			setup_function49();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(49, August, function49)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function20(false);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarKronos, kPosition_9270);
			break;

		case 2:
			setup_function50();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(50, August, function50)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getEntities()->clearSequences(kEntityAugust);

		getData()->entityPosition = kPosition_6000;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarKronos;
		break;

	case kAction191668032:
		setup_function51();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(51, August, function51)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->car = kCarGreenSleeping;
		getData()->entityPosition = kPosition_850;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_function42(kCarGreenSleeping, kPosition_5790, false);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityAugust, kEntityTatiana, kAction191668032);

			setCallback(2);
			setup_function42(kCarRedSleeping, kPosition_540, true);
			break;

		case 2:
			getEntities()->clearSequences(kEntityAugust);
			break;

		case 3:
			getEntities()->drawSequenceLeft(kEntityAugust, "BLANK");
			getSavePoints()->push(kEntityAugust, kEntityAnna, kAction123712592);
			break;
		}
		break;

	case kAction122288808:
		setup_function52();
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityAugust, "BLANK");
		break;

	case kAction169032608:
		setCallback(3);
		setup_function42(kCarRedSleeping, kPosition_3820, true);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(52, August, function52)
	switch (savepoint.action) {
	default:
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (getInventory()->hasItem(kItemBriefcase)) {
			getData()->location = kLocationInsideCompartment;
			if (savepoint.action == kActionKnock)
				getSound()->playSound(kEntityPlayer, "LIB012");

			setCallback(3);
			setup_savegame(kSavegameTypeEvent, kEventAugustBringBriefcase);
			break;
		}

		if (getInventory()->hasItem(kItemFirebird) && !getEvent(kEventAugustBringEgg)) {
			setCallback(4);
			setup_savegame(kSavegameTypeEvent, kEventAugustBringEgg);
			break;
		}

		if (!getEvent(kEventAugustTalkCompartmentDoorBlueRedingote) && !getEvent(kEventAugustBringEgg) && !getEvent(kEventAugustBringBriefcase)) {
			if (savepoint.action == kActionKnock)
				getSound()->playSound(kEntityPlayer, "LIB012");

			setCallback(5);
			setup_savegame(kSavegameTypeEvent, kEventAugustBringEgg);
			break;
		}

		getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorNormal, kCursorNormal);

		setCallback(savepoint.action == kActionKnock ? 6 : 7);
		setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		break;

	case kActionDefault:
		setCallback(1);
		setup_function42(kCarGreenSleeping, kPosition_6470, true);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function19(false, true);
			break;

		case 2:
			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getSavePoints()->push(kEntityAugust, kEntityKahina, kAction134611040);
			break;

		case 3:
			getAction()->playAnimation(kEventAugustBringBriefcase);
			getSound()->playSound(kEntityPlayer, "LIB015");
			RESET_ENTITY_STATE(kEntitySalko, Salko, setup_function17);
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 13);

			setup_function53();
			break;

		case 4:
			getAction()->playAnimation(kEventAugustBringEgg);
			getScenes()->processScene();
			break;

		case 5:
			getAction()->playAnimation(kEventAugustTalkCompartmentDoorBlueRedingote);
			getScenes()->processScene();
			break;

		case 6:
		case 7:
			setCallback(8);
			setup_playSound("AUG1128F");
			break;

		case 8:
			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(53, August, function53)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateFromTime(2700);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function20(false);
			break;

		case 2:
			setCallback(3);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 3:
			setup_function54();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(54, August, function54)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param4 || params->param2 || getProgress().field_44)
			getData()->inventoryItem = kItemNone;
		else
			getData()->inventoryItem = kItemInvalid;

		if (!params->param2 && params->param1) {
			if (!Entity::updateParameter(params->param5, getState()->time, params->param1))
				break;

			getData()->inventoryItem = kItemNone;
			setup_function55();
		}
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;

		setCallback(3);
		setup_savegame(kSavegameTypeEvent, kEventAugustTalkCigar);
		break;

	case kActionExitCompartment:
		getEntities()->updatePositionExit(kEntityAugust, kCarRestaurant, 57);
		getEntities()->drawSequenceLeft(kEntityAugust, "105B3");
		break;

	case kActionDefault:
		setCallback(1);
		setup_callbackActionRestaurantOrSalon();
		break;

	case kActionDrawScene:
		if (!getEntities()->isPlayerPosition(kCarRestaurant, 60) || params->param3) {
			if (!params->param2 && getEntities()->isPlayerPosition(kCarRestaurant, 57))
				getScenes()->loadSceneFromPosition(kCarRestaurant, 50);
		} else if (!params->param2) {
			getEntities()->updatePositionEnter(kEntityAugust, kCarRestaurant, 57);
			getEntities()->drawSequenceRight(kEntityAugust, "105C3");
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_updatePosition("105A3", kCarRestaurant, 57);
			break;

		case 2:
			getData()->location = kLocationInsideCompartment;
			getSavePoints()->push(kEntityAugust, kEntityAbbot, kAction123712592);
			getEntities()->drawSequenceLeft(kEntityAugust, "105B3");
			params->param4 = 1;
			break;

		case 3:
			getAction()->playAnimation(kEventAugustTalkCigar);
			getEntities()->drawSequenceLeft(kEntityAugust, params->param3 ? "122B" : "105B3");
			getScenes()->processScene();

			params->param1 = 9000;
			params->param4 = 0;
			break;
		}
		break;

	case kAction122288808:
		getEntities()->drawSequenceLeft(kEntityAugust, "122B");
		params->param2 = 0;

		if (getEvent(kEventAugustTalkCigar))
			params->param1 = 9000;
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityAugust, "BLANK");
		params->param2 = 1;
		params->param3 = 1;
		break;

	case kAction136196244:
		params->param2 = 1;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(55, August, function55)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_callbackActionRestaurantOrSalon();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_updatePosition("105D3", kCarRestaurant, 57);
			break;

		case 2:
			setCallback(3);
			setup_updateEntity(kCarGreenSleeping, kPosition_6470);
			break;

		case 3:
			setCallback(4);
			setup_function19(true, false);
			break;

		case 4:
			setup_function56();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(56, August, function56)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);
		getEntities()->drawSequenceLeft(kEntityAugust, "507A3");
		break;

	case kActionDrawScene:
		if (!params->param1 && getEntities()->isPlayerPosition(kCarGreenSleeping, 43)) {
			setCallback(1);
			setup_draw("507B3");
		}
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			params->param1 = 1;
			getEntities()->drawSequenceLeft(kEntityAugust, "507A3");
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(57, August, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAugust);

		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;
		getData()->clothes = kClothes2;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation2, kCursorNormal, kCursorNormal);

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(58, August, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function20(false);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_draw("803WS");
			break;

		case 4:
			getEntities()->drawSequenceRight(kEntityAugust, "010A3");
			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityAugust);

			setCallback(5);
			setup_callSavepointNoDrawing(kEntityTables3, kAction136455232, "BOGUS");
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			setup_function59();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(59, August, function59)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityAugust, "010B3");
		getSavePoints()->push(kEntityAugust, kEntityPascale, kAction190605184);
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityAugust, "BLANK");
		break;

	case kAction123793792:
		setup_function60();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(60, August, function60)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone: {
		bool pushSavepoint = false;
		if (!params->param2) {
			pushSavepoint = true;
			params->param2 = (uint)getState()->time + 450;
		}

		if (params->param2 < getState()->time) {
			pushSavepoint = true;
			params->param2 = kTimeInvalid;
		}

		if (pushSavepoint)
			getSavePoints()->push(kEntityAugust, kEntityServers0, kAction207330561);

		if (!params->param1)
			break;

		if (!Entity::updateParameter(params->param3, getState()->time, 9000))
			break;

		setCallback(1);
		setup_callbackActionRestaurantOrSalon();
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityAugust, "010B3");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_callSavepoint("010J3", kEntityTables3, kActionDrawTablesWithChairs, "010M");
			break;

		case 2:
			getSavePoints()->push(kEntityAugust, kEntityServers0, kAction286403504);
			setup_function61();
			break;
		}
		break;

	case kAction122288808:
		getEntities()->drawSequenceLeft(kEntityAugust, "010B3");
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityAugust, "BLANK");
		break;

	case kAction201964801:
		getEntities()->drawSequenceLeft(kEntityAugust, "010H3");
		params->param1 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(61, August, function61)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->location = kLocationOutsideCompartment;
		getEntities()->drawSequenceRight(kEntityAugust, "803FS");
		if (getEntities()->isInRestaurant(kEntityPlayer))
			getEntities()->updateFrame(kEntityAugust);

		setCallback(1);
		setup_callbackActionOnDirection();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_6470);
			break;

		case 2:
			setCallback(3);
			setup_function19(false, false);
			break;

		case 3:
			setCallback(4);
			setup_function21((TimeValue)(getState()->time + 4500));
			break;

		case 4:
			setup_function62();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(62, August, function62)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param1, getState()->time, 900))
			break;

		getSound()->playSound(kEntityAugust, "Aug4003A");

		setCallback(5);
		setup_updatePosition("122C", kCarRestaurant, 57);
		break;

	case kActionDefault:
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_enterExitCompartment("696Ec", kObjectCompartment3);
		break;

	case kActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 57))
			getScenes()->loadSceneFromPosition(kCarRestaurant, 50);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			setCallback(2);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_updatePosition("122A", kCarRestaurant, 57);
			break;

		case 4:
			getData()->location = kLocationInsideCompartment;
			getEntities()->drawSequenceLeft(kEntityAugust, "122B");
			break;

		case 5:
			getEntities()->drawSequenceLeft(kEntityAugust, "122B");
			getSavePoints()->push(kEntityAugust, kEntityServers1, kAction291721418);
			break;
		}
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityAugust, "BLANK");
		break;

	case kAction125826561:
		setup_function63();
		break;

	case kAction134486752:
		getEntities()->drawSequenceLeft(kEntityAugust, "122B");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(63, August, function63)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::updateParameter(params->param3, getState()->time, 1800)) {
			getData()->inventoryItem = kItemInvalid;
		}

		if (getState()->time > kTime2488500 && !params->param4) {
			params->param4 = 1;
			getData()->inventoryItem = kItemNone;
			setup_function64();
			break;
		}

		if (!Entity::updateParameter(params->param5, getState()->timeTicks, params->param1))
			break;

		params->param2 = (params->param6 < 1 ? 1 : 0);

		getEntities()->drawSequenceLeft(kEntityAugust, params->param2 ? "122H" : "122F");

		params->param1 = 5 * (3 * rnd(20) + 15);
		params->param5 = 0;
		break;

	case kAction1:
		if (getEntities()->isInSalon(kEntityAlexei))
			RESET_ENTITY_STATE(kEntityAlexei, Alexei, setup_function44);

		getData()->inventoryItem = kItemNone;

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventAugustDrink);
		break;

	case kActionDefault:
		params->param1 = 5 * (3 * rnd(20) + 15);
		getEntities()->drawSequenceLeft(kEntityAugust, "122F");
		break;

	case kActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 57))
			getScenes()->loadSceneFromPosition(kCarRestaurant, 50);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventAugustDrink);
			getScenes()->loadSceneFromPosition(kCarRestaurant, 55);

			setup_function64();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(64, August, function64)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1)
			params->param1 = (uint)getState()->time + 1800;

		if (params->param1 >= getState()->time)
			break;

		if (getState()->time > kTime2430000 && getEntities()->isSomebodyInsideRestaurantOrSalon())  {
			getData()->location = kLocationOutsideCompartment;

			setCallback(1);
			setup_updatePosition("122J", kCarRestaurant, 57);
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityAugust, "122H");
		break;

	case kActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 57))
			getScenes()->loadSceneFromPosition(kCarRestaurant, 50);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_6470);
			break;

		case 2:
			setCallback(3);
			setup_enterExitCompartment2("696Dc", kObjectCompartment3);
			break;

		case 3:
			getEntities()->clearSequences(kEntityAugust);
			setup_function65();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(65, August, function65)
	switch (savepoint.action) {
	default:
		break;

	case kActionEndSound:
		getSound()->playSound(kEntityAugust, "AUG1057");   // August snoring
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;

		getEntities()->clearSequences(kEntityAugust);

		getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		if (!getSoundQueue()->isBuffered(kEntityAugust))
			getSound()->playSound(kEntityAugust, "AUG1057");   // August snoring
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(66, August, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAugust);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes2;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(67, August, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_function68();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(68, August, function68)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			if (!Entity::updateParameter(params->param4, getState()->timeTicks, 75))
				break;

			params->param1 = 0;
			params->param2 = 1;

			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param4 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (params->param1) {
			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorNormal, kCursorNormal);

			setCallback(1);
			setup_playSound(getSound()->justCheckingCath());
		} else {
			setCallback(savepoint.action == kActionKnock ? 2 : 3);
			setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;

		getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (params->param1 || params->param2) {
			params->param1 = 0;
			params->param2 = 0;
			params->param3 = 0;

			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorHandKnock, kCursorHand);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			params->param1 = 0;
			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;

		case 2:
		case 3:
			++params->param3;

			switch (params->param3) {
			default:
				break;

			case 1:
				getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorNormal, kCursorNormal);

				setCallback(4);
				setup_playSound("Aug5002");
				break;

			case 2:
				getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorNormal, kCursorNormal);

				setCallback(5);
				setup_playSound("Aug5002A");
				break;

			case 3:
				getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorNormal, kCursorNormal);

				setCallback(6);
				setup_playSound("Aug5002B");
				break;
			}
			break;

		case 4:
			params->param1 = 1;
			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorTalk, kCursorNormal);
			break;

		case 5:
			getObjects()->update(kObjectCompartment3, kEntityAugust, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;

		case 6:
			params->param2 = 1;
			break;
		}
		break;

	case kAction203078272:
		getSavePoints()->push(kEntityAugust, kEntityTatiana, kAction203078272);

		setup_unhookCars();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(69, August, unhookCars)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		getSavePoints()->pushAll(kEntityAugust, kAction135800432);
		setup_nullfunction();
		break;

	case kActionDefault:
		getSoundQueue()->processEntries();
		if (getSoundQueue()->isBuffered("ARRIVE"))
			getSoundQueue()->removeFromQueue("ARRIVE");

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventAugustUnhookCarsBetrayal);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(getProgress().field_C ? kEventAugustUnhookCarsBetrayal : kEventAugustUnhookCars);
			getEntities()->clearSequences(kEntityAugust);
			getSoundQueue()->resetState();
			getSound()->playSound(kEntityPlayer, "MUS050");
			getScenes()->loadSceneFromPosition(kCarRestaurant, 85, 1);
			getSavePoints()->pushAll(kEntityAugust, kActionProceedChapter5);

			RESET_ENTITY_STATE(kEntityVerges, Verges, setup_end)
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(70, August)

} // End of namespace LastExpress
