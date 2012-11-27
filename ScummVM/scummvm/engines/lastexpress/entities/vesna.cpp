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

#include "lastexpress/entities/vesna.h"

#include "lastexpress/fight/fight.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Vesna::Vesna(LastExpressEngine *engine) : Entity(engine, kEntityVesna) {
	ADD_CALLBACK_FUNCTION(Vesna, reset);
	ADD_CALLBACK_FUNCTION(Vesna, playSound);
	ADD_CALLBACK_FUNCTION(Vesna, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(Vesna, draw);
	ADD_CALLBACK_FUNCTION(Vesna, updateEntity);
	ADD_CALLBACK_FUNCTION(Vesna, updateFromTime);
	ADD_CALLBACK_FUNCTION(Vesna, updateEntity2);
	ADD_CALLBACK_FUNCTION(Vesna, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION(Vesna, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(Vesna, savegame);
	ADD_CALLBACK_FUNCTION(Vesna, function11);
	ADD_CALLBACK_FUNCTION(Vesna, chapter1);
	ADD_CALLBACK_FUNCTION(Vesna, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Vesna, function14);
	ADD_CALLBACK_FUNCTION(Vesna, function15);
	ADD_CALLBACK_FUNCTION(Vesna, chapter2);
	ADD_CALLBACK_FUNCTION(Vesna, chapter2Handler);
	ADD_CALLBACK_FUNCTION(Vesna, function18);
	ADD_CALLBACK_FUNCTION(Vesna, chapter3);
	ADD_CALLBACK_FUNCTION(Vesna, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Vesna, function21);
	ADD_CALLBACK_FUNCTION(Vesna, function22);
	ADD_CALLBACK_FUNCTION(Vesna, function23);
	ADD_CALLBACK_FUNCTION(Vesna, chapter4);
	ADD_CALLBACK_FUNCTION(Vesna, function25);
	ADD_CALLBACK_FUNCTION(Vesna, function26);
	ADD_CALLBACK_FUNCTION(Vesna, function27);
	ADD_CALLBACK_FUNCTION(Vesna, chapter5);
	ADD_CALLBACK_FUNCTION(Vesna, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Vesna, function30);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Vesna, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Vesna, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(3, Vesna, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(4, Vesna, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(5, Vesna, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kActionExcuseMeCath) {
		getSound()->playSound(kEntityPlayer, rnd(2) ? "CAT10150" : "CAT1015A");

		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(6, Vesna, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(7, Vesna, updateEntity2, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		params->param3 = 0;

		if (getEntities()->isDistanceBetweenEntities(kEntityVesna, kEntityMilos, 500)
		 || (((getData()->direction == kDirectionUp && (getData()->car > getEntityData(kEntityMilos)->car)) || (getData()->car == getEntityData(kEntityMilos)->car && getData()->entityPosition > getEntityData(kEntityMilos)->entityPosition)))
		 || (((getData()->direction == kDirectionDown && (getData()->car < getEntityData(kEntityMilos)->car)) || (getData()->car == getEntityData(kEntityMilos)->car && getData()->entityPosition < getEntityData(kEntityMilos)->entityPosition)))) {
			getData()->field_49B = 0;
			params->param3 = 1;
		}

		if (!params->param3)
			getEntities()->updateEntity(kEntityVesna, (CarIndex)params->param1, (EntityPosition)params->param2);
		break;

	case kActionDefault:
		getEntities()->updateEntity(kEntityVesna, (CarIndex)params->param1, (EntityPosition)params->param2);
		break;

	case kAction123668192:
		callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Vesna, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Vesna, callbackActionOnDirection)
	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(10, Vesna, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Vesna, function11)
	// Expose parameters as IIIS and ignore the default exposed parameters
	EntityData::EntityParametersIIIS *parameters = (EntityData::EntityParametersIIIS*)_data->getCurrentParameters();

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (parameters->param3) {
			if (!Entity::updateParameter(parameters->param7, getState()->timeTicks, 75))
				break;

			parameters->param2 = 1;
			parameters->param3 = 0;
			getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		parameters->param7 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (parameters->param3) {
			getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorNormal, kCursorNormal);

			setCallback(4);
			setup_playSound(getSound()->wrongDoorCath());
			break;
		}

		parameters->param1++;
		switch (parameters->param1) {
		default:
			strcpy((char *)&parameters->seq, "VES1015C");
			parameters->param1 = 0;
			break;

		case 1:
			strcpy((char *)&parameters->seq, "VES1015A");
			break;

		case 2:
			strcpy((char *)&parameters->seq, "VES1015B");
			break;
		}

		getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorNormal, kCursorNormal);

		setCallback(savepoint.action == kActionKnock ? 2 : 1);
		setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (parameters->param2 || parameters->param3) {
			getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation1, kCursorHandKnock, kCursorHand);

			parameters->param2 = 0;
			parameters->param3 = 0;
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_playSound((char *)&parameters->seq);
			break;

		case 3:
			getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorTalk, kCursorNormal);
			parameters->param3 = 1;
			break;

		case 4:
			parameters->param2 = 1;
			parameters->param3 = 0;
			break;
		}
		break;

	case kAction55996766:
	case kAction101687594:
		callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Vesna, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Vesna, setup_chapter1Handler));
		break;

	case kActionDefault:
		getSavePoints()->addData(kEntityVesna, kAction124190740, 0);

		getData()->entityPosition = kPosition_4689;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Vesna, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		getData()->entityPosition = getEntityData(kEntityMilos)->entityPosition;
		getData()->location = getEntityData(kEntityMilos)->location;
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getEntities()->clearSequences(kEntityVesna);
			setup_function14();
		}
		break;

	case kAction204832737:
		setCallback(1);
		setup_updateEntity2(kCarRedSleeping, kPosition_3050);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Vesna, function14)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		break;

	case kAction190412928:
		setCallback(1);
		setup_function11();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Vesna, function15)
	if (savepoint.action == kActionDefault) {
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->clearSequences(kEntityVesna);
		getObjects()->update(kObjectCompartmentG, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Vesna, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter2Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityVesna);

		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Vesna, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kAction135024800:
		setCallback(2);
		setup_function18();
		break;

	case kAction137165825:
		setCallback(1);
		setup_function11();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Vesna, function18)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_enterExitCompartment("610Bg", kObjectCompartmentG);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			if (getData()->entityPosition < kPosition_2087)
				getData()->entityPosition = kPosition_2088;

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
			setup_draw("808US");
			break;

		case 4:
			getEntities()->drawSequenceRight(kEntityVesna, "808UD");
			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityVesna);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityVesna);

			setCallback(6);
			setup_updateFromTime(4500);
			break;

		case 6:
			setCallback(7);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 7:
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;

			setCallback(8);
			setup_draw("808DD");
			break;

		case 8:
			getEntities()->drawSequenceRight(kEntityVesna, "808DS");
			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityVesna);

			setCallback(9);
			setup_callbackActionOnDirection();
			break;

		case 9:
			setCallback(10);
			setup_updateEntity(kCarRedSleeping, kPosition_3050);
			break;

		case 10:
			setCallback(11);
			setup_enterExitCompartment("610Ag", kObjectCompartmentG);
			break;

		case 11:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityVesna);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Vesna, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityVesna);

		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Vesna, chapter3Handler)
	EntityData::EntityParametersIIIS *parameters = (EntityData::EntityParametersIIIS*)_data->getCurrentParameters();

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getProgress().field_54 && parameters->param7 != kTimeInvalid) {
			if (getState()->time > kTime2250000) {
				parameters->param7 = kTimeInvalid;
				setup_function22();
				break;
			}

			if (!getEntities()->isPlayerInCar(kCarRedSleeping) || !parameters->param7)
				parameters->param7 = (uint)getState()->time;

			if (parameters->param7 < getState()->time) {
				parameters->param7 = kTimeInvalid;
				setup_function22();
				break;
			}
		}

		if (parameters->param2) {
			if (!Entity::updateParameter(parameters->param8, getState()->timeTicks, 75))
				break;

			parameters->param1 = 1;
			parameters->param2 = 0;

			getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		parameters->param8 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (parameters->param2) {
			getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorNormal, kCursorNormal);

			setCallback(4);
			setup_playSound(getSound()->wrongDoorCath());
			break;
		}

		++parameters->param3;

		switch (parameters->param3) {
		default:
			break;

		case 1:
			strcpy((char *)&parameters->seq, "VES1015A");
			break;

		case 2:
			strcpy((char *)&parameters->seq, "VES1015B");
			break;

		case 3:
			strcpy((char *)&parameters->seq, "VES1015C");
			parameters->param3 = 0;
			break;
		}

		getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorNormal, kCursorNormal);

		setCallback(savepoint.action == kActionKnock ? 2 : 1);
		setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		break;

	case kActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getEntities()->clearSequences(kEntityVesna);
		break;

	case kActionDrawScene:
		if (parameters->param1 || parameters->param2) {
			getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation1, kCursorHandKnock, kCursorHand);
			parameters->param1 = 0;
			parameters->param2 = 0;
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_playSound((char *)&parameters->seq);
			break;

		case 3:
			getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorTalk, kCursorNormal);
			parameters->param2 = 1;
			break;

		case 4:
			parameters->param1 = 1;
			parameters->param2 = 0;
			break;
		}
		break;

	case kAction137165825:
		setCallback(5);
		setup_function11();
		break;

	case kAction155913424:
		setCallback(6);
		setup_function21();
		break;

	case kAction203663744:
		getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Vesna, function21)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_enterExitCompartment("610Bg", kObjectCompartmentG);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			if (getData()->entityPosition < kPosition_2087)
				getData()->entityPosition = kPosition_2088;

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
			setup_draw("808US");
			break;

		case 4:
			getEntities()->drawSequenceRight(kEntityVesna, "808UD");
			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityVesna);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getEntities()->clearSequences(kEntityVesna);
			getData()->entityPosition = kPosition_5900;
			getData()->location = kLocationInsideCompartment;

			setCallback(6);
			setup_updateFromTime(4500);
			break;

		case 6:
			setCallback(7);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 7:
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;

			setCallback(8);
			setup_draw("808DD");
			break;

		case 8:
			getEntities()->drawSequenceRight(kEntityVesna, "808DS");
			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityVesna);

			setCallback(9);
			setup_callbackActionOnDirection();
			break;

		case 9:
			setCallback(10);
			setup_updateEntity(kCarRedSleeping, kPosition_3050);
			break;

		case 10:
			setCallback(11);
			setup_enterExitCompartment("610Ag", kObjectCompartmentG); /* BUG the original engine passes 3050 here */
			break;

		case 11:
			getData()->entityPosition = kPosition_3050;
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityVesna);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Vesna, function22)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityVesna, kEntityMilos, kAction259125998);

		setCallback(1);
		setup_enterExitCompartment("610Bg", kObjectCompartmentG);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			if (getData()->entityPosition < kPosition_2087)
				getData()->entityPosition = kPosition_2088;

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
			setup_draw("808US");
			break;

		case 4:
			getEntities()->drawSequenceRight(kEntityVesna, "808UD");
			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityVesna);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getEntities()->clearSequences(kEntityVesna);
			getData()->car = kCarBaggage;
			getSavePoints()->push(kEntityVesna, kEntityAnna, kAction235856512);
			break;

		case 6:
			getData()->car = kCarRestaurant;
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;

			setCallback(7);
			setup_draw("808DD");
			break;

		case 7:
			getEntities()->drawSequenceRight(kEntityVesna, "808DS");
			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityVesna);

			setCallback(8);
			setup_callbackActionOnDirection();
			break;

		case 8:
			setCallback(9);
			setup_updateEntity(kCarRedSleeping, kPosition_3050);
			break;

		case 9:
			setCallback(10);
			setup_enterExitCompartment("610Ag", kObjectCompartmentG); /* BUG the original engine passes 3050 here */
			break;

		case 10:
			getData()->entityPosition = kPosition_3050;
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityVesna);

			setup_function23();
			break;
		}
		break;

	case kAction189299008:
		setCallback(6);
		setup_callbackActionRestaurantOrSalon();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Vesna, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionKnock:
	case kActionOpenDoor:
		getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorNormal, kCursorNormal);
		setCallback(savepoint.action == kActionKnock ? 1 : 2);
		setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		break;

	case kActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_playSound("VES1015A");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorHandKnock, kCursorHand);
			break;
		}
		break;

	case kAction203663744:
		getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Vesna, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setCallback(1);
		setup_function11();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityVesna);

		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentG, kEntityVesna, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function25();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Vesna, function25)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime2428200 && !params->param1) {
			params->param1 = 1;
			setup_function26();
		}
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityVesna, kEntityMilos, kAction135600432);

		setCallback(1);
		setup_enterExitCompartment("610BG", kObjectCompartmentG);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			if (getData()->entityPosition < kPosition_2087)
				getData()->entityPosition = kPosition_2088;

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
			setup_draw("808US");
			break;

		case 4:
			getEntities()->drawSequenceRight(kEntityVesna, "808UD");
			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityVesna);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getEntities()->clearSequences(kEntityVesna);
			getData()->entityPosition = kPosition_5900;
			getData()->location = kLocationInsideCompartment;

			// Original game calls clearSequences a second time
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Vesna, function26)
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
			getData()->car = kCarRestaurant;
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_draw("808DD");
			break;

		case 2:
			getEntities()->drawSequenceRight(kEntityVesna, "808DS");

			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityVesna);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			setCallback(4);
			setup_updateEntity(kCarRedSleeping, kPosition_3050);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("610AG", kObjectCompartmentG);
			break;

		case 5:
			setup_function27();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Vesna, function27)
	if (savepoint.action == kActionDefault) {
		getEntities()->clearSequences(kEntityVesna);
		getObjects()->update(kObjectCompartmentG, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->inventoryItem = kItemNone;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Vesna, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityVesna);

		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Vesna, chapter5Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionOpenDoor:
		setCallback(1);

		getData()->currentCall++;
		setup_savegame(kSavegameTypeEvent, kEventCathVesnaRestaurantKilled);
		break;

	case kActionDefault:
		getObjects()->update(kObject64, kEntityVesna, kObjectLocationNone, kCursorNormal, kCursorForward);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventCathVesnaRestaurantKilled);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneNone, true);
		}
		break;

	case kAction134427424:
		getObjects()->update(kObject64, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorForward);
		setup_function30();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Vesna, function30)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1) {
			if (Entity::updateParameter(params->param3, getState()->timeTicks, 120)) {
				getSound()->playSound(kEntityVesna, "Ves50001", kFlagDefault);
				params->param1 = 1;
			}
		}

		if (!Entity::updateParameter(params->param4, getState()->timeTicks, 180))
			break;

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventCathVesnaTrainTopKilled);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			getAction()->playAnimation(kEventCathVesnaTrainTopKilled);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneNone, true);
			break;

		case 3:
			getAction()->playAnimation(kEventCathVesnaTrainTopFight);

			setCallback(4);
			setup_savegame(kSavegameTypeTime, kTimeNone);
			break;

		case 4:
			params->param2 = getFight()->setup(kFightVesna);

			if (params->param2) {
				getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, params->param2 != Fight::kFightEndExit);
			} else {
				getSound()->playSound(kEntityPlayer, "TUNNEL");

				getState()->time = (TimeValue)(getState()->time + 1800);

				setCallback(5);
				setup_savegame(kSavegameTypeEvent, kEventCathVesnaTrainTopWin);
			}
			break;

		case 5:
			getAction()->playAnimation(kEventCathVesnaTrainTopWin);
			getScenes()->loadSceneFromPosition(kCarRestaurant, 11);

			setup_nullfunction();
			break;
		}
		break;

	case kAction167992577:
		setCallback(3);
		setup_savegame(kSavegameTypeEvent, kEventCathVesnaTrainTopFight);
		break;

	case kAction202884544:
		if (params->param1) {
			setCallback(2);
			setup_savegame(kSavegameTypeEvent, kEventCathVesnaTrainTopKilled);
		} else {
			getSound()->playSound(kEntityVesna, "Ves5001", kFlagDefault);
			params->param1 = 1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(31, Vesna)

} // End of namespace LastExpress
