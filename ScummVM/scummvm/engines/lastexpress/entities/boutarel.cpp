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

#include "lastexpress/entities/boutarel.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Boutarel::Boutarel(LastExpressEngine *engine) : Entity(engine, kEntityBoutarel) {
	ADD_CALLBACK_FUNCTION(Boutarel, reset);
	ADD_CALLBACK_FUNCTION(Boutarel, playSound);
	ADD_CALLBACK_FUNCTION(Boutarel, draw);
	ADD_CALLBACK_FUNCTION(Boutarel, updateFromTime);
	ADD_CALLBACK_FUNCTION(Boutarel, updatePosition);
	ADD_CALLBACK_FUNCTION(Boutarel, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(Boutarel, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION(Boutarel, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(Boutarel, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION(Boutarel, updateEntity);
	ADD_CALLBACK_FUNCTION(Boutarel, function11);
	ADD_CALLBACK_FUNCTION(Boutarel, enterTableWithMmeBoutarel);
	ADD_CALLBACK_FUNCTION(Boutarel, leaveTableWithMmeBoutarel);
	ADD_CALLBACK_FUNCTION(Boutarel, function14);
	ADD_CALLBACK_FUNCTION(Boutarel, function15);
	ADD_CALLBACK_FUNCTION(Boutarel, function16);
	ADD_CALLBACK_FUNCTION(Boutarel, function17);
	ADD_CALLBACK_FUNCTION(Boutarel, function18);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter1);
	ADD_CALLBACK_FUNCTION(Boutarel, function20);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Boutarel, function22);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter2);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter2Handler);
	ADD_CALLBACK_FUNCTION(Boutarel, function25);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter3);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Boutarel, function28);
	ADD_CALLBACK_FUNCTION(Boutarel, function29);
	ADD_CALLBACK_FUNCTION(Boutarel, function30);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter4);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Boutarel, function33);
	ADD_CALLBACK_FUNCTION(Boutarel, function34);
	ADD_CALLBACK_FUNCTION(Boutarel, function35);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter5);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Boutarel, function38);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Boutarel, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Boutarel, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, Boutarel, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(4, Boutarel, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SII(5, Boutarel, updatePosition, CarIndex, Position)
	Entity::updatePosition(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(6, Boutarel, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(7, Boutarel, enterExitCompartment2, ObjectIndex)
	Entity::enterExitCompartment(savepoint, kPosition_6470, kPosition_6130, kCarRedSleeping, kObjectCompartmentC, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Boutarel, callbackActionOnDirection)
	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Boutarel, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(10, Boutarel, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kActionExcuseMeCath) {

		if (getInventory()->hasItem(kItemPassengerList) && getState()->time > kTime1089000)
			getSound()->playSound(kEntityPlayer, "CAT1022");
		else
			getSound()->excuseMeCath();

		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(11, Boutarel, function11, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(0, 1) && ENTITY_PARAM(0, 2)) {
			ENTITY_PARAM(0, 2) = 0;
			ENTITY_PARAM(0, 1) = 0;

			setCallback(5);
			setup_callbackActionRestaurantOrSalon();
		}
		break;

	case kActionDefault:
		if (params->param1) {
			if (getProgress().chapter == kChapter4) {
				getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

				setCallback(1);
				setup_enterExitCompartment("607Hc", kObjectCompartmentC);
			} else {
				getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

				setCallback(2);
				setup_enterExitCompartment("607Dc", kObjectCompartmentC);
			}
		} else {
			setCallback(3);
			setup_enterExitCompartment("607Bc", kObjectCompartmentC);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 2:
		case 3:
			if (getCallback() == 2)
				getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			else
				getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			// Fallback to next case

		case 1:
			getObjects()->update(kObject50, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityBoutarel, kEntityFrancois, kAction101107728);

			setCallback(4);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 4:
			getEntities()->clearSequences(kEntityBoutarel);
			break;

		case 5:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(6);
			setup_draw("812US");
			break;

		case 6:
			switch (getProgress().chapter) {
			default:
				break;

			case kChapter1:
				getSound()->playSound(kEntityBoutarel, "MRB1075", kFlagInvalid, 60);
				break;

			case kChapter3:
				getSound()->playSound(kEntityBoutarel, "MRB3101");
				break;
			}

			setCallback(7);
			setup_enterTableWithMmeBoutarel();
			break;

		case 7:
			getData()->location = kLocationInsideCompartment;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Boutarel, enterTableWithMmeBoutarel)
	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		getEntities()->clearSequences(kEntityMmeBoutarel);
		getSavePoints()->push(kEntityBoutarel, kEntityTables2, kAction136455232);
		getData()->location = kLocationInsideCompartment;

		callbackAction();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(kEntityTables2, "008A3");
		getEntities()->drawSequenceRight(kEntityMmeBoutarel, "008A2");
		getEntities()->drawSequenceRight(kEntityBoutarel, "008A1");

		if (getEntities()->isInSalon(kEntityPlayer)) {
			getEntities()->updateFrame(kEntityBoutarel);
			getEntityData(kEntityMmeBoutarel)->location = getData()->location;
			getEntityData(kEntityTables2)->location = getData()->location;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Boutarel, leaveTableWithMmeBoutarel)
	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		getSavePoints()->push(kEntityBoutarel, kEntityTables2, kActionDrawTablesWithChairs, "008F");
		getEntities()->clearSequences(kEntityMmeBoutarel);

		callbackAction();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(kEntityTables2, "008E3");
		getEntities()->drawSequenceRight(kEntityMmeBoutarel, "008E2");
		getEntities()->drawSequenceRight(kEntityBoutarel, "008E1");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(14, Boutarel, function14, bool)
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
			getSound()->playSound(kEntityBoutarel, "MRB1079");

			setCallback(2);
			setup_leaveTableWithMmeBoutarel();
			break;

		case 2:
			getSavePoints()->push(kEntityBoutarel, kEntityServers1, kAction326144276);
			getEntities()->drawSequenceRight(kEntityBoutarel, "812DS");
			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityBoutarel);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			getEntityData(kEntityFrancois)->entityPosition = kPosition_540;
			getEntityData(kEntityMmeBoutarel)->entityPosition = kPosition_540;
			getData()->entityPosition = kPosition_540;

			getEntityData(kEntityFrancois)->location = kLocationOutsideCompartment;
			getEntityData(kEntityMmeBoutarel)->location = kLocationOutsideCompartment;

			getEntities()->clearSequences(kEntityBoutarel);
			getSavePoints()->push(kEntityBoutarel, kEntityMmeBoutarel, kAction100901266);

			setCallback(4);
			setup_updateFromTime(450);
			break;

		case 4:
			getSavePoints()->push(kEntityBoutarel, kEntityFrancois, kAction100901266);

			setCallback(5);
			setup_updateFromTime(450);
			break;

		case 5:
			setCallback(6);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 6:
			setCallback(params->param1 ?  7: 8);
			setup_enterExitCompartment2(params->param1 ? "607Gc" : "607Ac", kObjectCompartmentC);
			break;

		case 7:
		case 8:
			getEntities()->clearSequences(kEntityBoutarel);
			getData()->location = kLocationInsideCompartment;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_IS(15, Boutarel, function15, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (params->param1)
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

		setCallback(params->param1 ? 1 : 2);
		setup_enterExitCompartment(params->param1 ? "607Dc" : "607Bc", kObjectCompartmentC);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getObjects()->update(kObject50, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			setCallback(3);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject50, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			setCallback(3);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 3:
			setCallback(4);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 4:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(5);
			setup_updatePosition(params->seq, kCarRestaurant, 52);
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
// Parameters:
// bool
// const char *
IMPLEMENT_FUNCTION_IS(16, Boutarel, function16, bool)
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
			setup_updatePosition((const char *)&params->seq, kCarRestaurant, 52);
			break;

		case 2:
			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 3:
			setCallback(params->param1 ? 4 : 5);
			setup_enterExitCompartment2(params->param1 ? "607Gc" : "607Ac", kObjectCompartmentC);
			break;

		case 4:
		case 5:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityBoutarel);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_IS(17, Boutarel, function17, TimeValue)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::timeCheckCallbackAction((TimeValue)params->param1, params->param6))
			break;

		if (params->param5) {
			if (!Entity::updateParameter(params->param7, getState()->timeTicks, 90))
				break;

			getScenes()->loadSceneFromPosition(kCarRestaurant, 51);
		} else {
			params->param7 = 0;
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityBoutarel, (char *)&params->seq);
		break;

	case kActionDrawScene:
		params->param5 = (getEntities()->isPlayerPosition(kCarRestaurant, 52) ? 1 : 0);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(18, Boutarel, function18, TimeValue)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 < getState()->time && !params->param4) {
			params->param4 = 1;

			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject50, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;
		}

		if (params->param2) {
			if (!Entity::updateParameter(params->param5, getState()->timeTicks, 75))
				break;

			params->param2 = 0;
			params->param3 = 1;

			getObjects()->update(kObjectCompartmentC, kEntityBoutarel, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject50, kEntityBoutarel, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param5 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		getObjects()->update(kObjectCompartmentC, kEntityBoutarel, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject50, kEntityBoutarel, kObjectLocation1, kCursorNormal, kCursorNormal);

		if (params->param2) {
			if (savepoint.param.intValue == 50) {
				setCallback(4);
				setup_playSound(getSound()->justAMinuteCath());
			} else if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(5);
				setup_playSound(rnd(2) ? "CAT1511" : getSound()->wrongDoorCath());
			} else {
				setCallback(6);
				setup_playSound(getSound()->wrongDoorCath());
			}
		} else {
			setCallback(savepoint.action == kActionKnock ? 1 : 2);
			setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentC, kEntityBoutarel, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kEntityBoutarel, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (params->param3 || params->param2) {
			getObjects()->update(kObjectCompartmentC, kEntityBoutarel, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject50, kEntityBoutarel, kObjectLocation1, kCursorHandKnock, kCursorHand);

			params->param2 = 0;
			params->param3 = 0;
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_playSound(rnd(2) ? "MRB1001" : "MRB1001A");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentC, kEntityBoutarel, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject50, kEntityBoutarel, kObjectLocation1, kCursorTalk, kCursorNormal);

			params->param2 = 1;
			break;

		case 4:
		case 5:
		case 6:
			params->param2 = 0;
			params->param3 = 1;
			break;

		case 7:
			getSavePoints()->push(kEntityBoutarel, kEntityCoudert, kAction123199584);
			break;

		}
		break;

	case kAction122865568:
		getSavePoints()->push(kEntityBoutarel, kEntityCoudert, kAction88652208);
		break;

	case kAction221683008:
		setCallback(7);
		setup_playSound("MRB1001");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Boutarel, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Boutarel, setup_chapter1Handler));
		break;

	case kActionDefault:
		getSavePoints()->addData(kEntityBoutarel, kAction203520448, 0);
		getSavePoints()->addData(kEntityBoutarel, kAction237889408, 1);

		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject42, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		getData()->entityPosition = kPosition_1750;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Boutarel, function20)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1)
			break;

		if (!params->param2) {
			if (Entity::updateParameter(params->param3, getState()->time, 4500)) {
				setCallback(3);
				setup_playSound("MRB1078A");
				break;
			}
		}

		Entity::timeCheckCallback(kTime1138500, params->param4, 4, false, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
		break;

	case kActionDefault:
		setCallback(1);
		setup_function11(false);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kEntityBoutarel, "008B");

			setCallback(2);
			setup_playSound("MRB1076");
			break;

		case 2:
			getSavePoints()->push(kEntityBoutarel, kEntityServers1, kAction256200848);
			break;

		case 3:
			Entity::timeCheckCallback(kTime1138500, params->param4, 4, false, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
			break;

		case 4:
			getSavePoints()->push(kEntityBoutarel, kEntityCooks, kAction224849280);

			callbackAction();
			break;
		}
		break;

	case kAction134466544:
		params->param2 = 0;
		break;

	case kAction135854206:
		params->param2 = 1;
		break;

	case kAction168717392:
		params->param1 = 1;
		getEntities()->drawSequenceLeft(kEntityBoutarel, "008D");

		if (!params->param2) {
			setCallback(5);
			setup_playSound("MRB1078");
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Boutarel, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function17(kTime1071000, "101A");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function16(false, "101B");
			break;

		case 2:
			setCallback(3);
			setup_function18(kTime1102500);
			break;

		case 3:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject50, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);

			if (getEntities()->isPlayerPosition(kCarRedSleeping, 54) || getEntities()->isPlayerPosition(kCarRedSleeping, 44))
				getScenes()->loadSceneFromPosition(kCarRedSleeping, 10);

			getEntities()->updatePositionExit(kEntityBoutarel, kCarRedSleeping, 54);
			getEntities()->updatePositionExit(kEntityBoutarel, kCarRedSleeping, 44);

			setCallback(4);
			setup_playSound("MRB1074");
			break;

		case 4:
			getEntities()->updatePositionExit(kEntityBoutarel, kCarRedSleeping, 54);
			getEntities()->updatePositionExit(kEntityBoutarel, kCarRedSleeping, 44);

			setCallback(5);
			setup_function20();
			break;

		case 5:
			setCallback(6);
			setup_function18(kTimeEnterChalons);
			break;

		case 6:
			setCallback(7);
			setup_function15(false, "102A");
			break;

		case 7:
			setCallback(8);
			setup_function17(kTime1183500, "102B");
			break;

		case 8:
			setCallback(9);
			setup_function16(false, "102C");
			break;

		case 9:
			setCallback(10);
			setup_function18(kTime1215000);
			break;

		case 10:
			setup_function22();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Boutarel, function22)
	if (savepoint.action == kActionDefault) {
		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		getEntities()->clearSequences(kEntityBoutarel);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Boutarel, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter2Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityBoutarel);

		getData()->entityPosition = kPosition_4689;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Boutarel, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheckCallback(kTime1759500, params->param2, 1, false, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityBoutarel, "008D");
		break;

	case kActionDrawScene:
		if (getEntities()->isInRestaurant(kEntityPlayer) && !params->param1) {
			getSound()->playSound(kEntityBoutarel, "MRB2001");
			params->param1 = 1;
		}
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function25();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Boutarel, function25)
	if (savepoint.action == kActionDefault) {
		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getEntities()->drawSequenceLeft(kEntityBoutarel, "510");
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Boutarel, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityBoutarel);

		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getObjects()->update(kObject50, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Boutarel, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getEntities()->drawSequenceLeft(kEntityBoutarel, "510");
		break;

	case kAction122288808:
		setup_function28();
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityBoutarel, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Boutarel, function28)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function11(true);
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function29();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Boutarel, function29)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::updateParameter(params->param2, getState()->time, 450)) {
			getSavePoints()->push(kEntityBoutarel, kEntityServers1, kAction256200848);
		}

		if (!params->param1)
			break;

		if (getEntities()->isInRestaurant(kEntityAnna)
		 && getEntities()->isInRestaurant(kEntityAugust)
		 && !getSoundQueue()->isBuffered(kEntityBoutarel)
		 && params->param3 != kTimeInvalid) {

			if (getState()->time <= kTime1998000)
				if (!getEntities()->isInRestaurant(kEntityPlayer) || !params->param3)
					params->param3 = (uint)(getState()->time + 450);

			if (params->param3 < getState()->time || getState()->time > kTime1998000) {
				params->param3 = kTimeInvalid;

				setCallback(1);
				setup_playSound("MRB3102");
				break;
			}
		}

		Entity::timeCheckCallback(kTime2002500, params->param4, 1, true, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityBoutarel, "008B");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			Entity::timeCheckCallback(kTime2002500, params->param4, 1, true, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
			break;

		case 2:
			setup_function30();
			break;
		}
		break;

	case kAction122288808:
		getEntities()->drawSequenceLeft(kEntityBoutarel, "008D");
		params->param1 = 1;
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityBoutarel, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Boutarel, function30)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getEntities()->drawSequenceLeft(kEntityBoutarel, "510");
		break;

	case kAction122288808:
		getEntities()->drawSequenceLeft(kEntityBoutarel, "510");
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityBoutarel, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Boutarel, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityBoutarel);

		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getObjects()->update(kObject50, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Boutarel, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTime2367000, params->param1, WRAP_SETUP_FUNCTION(Boutarel, setup_function33));
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getEntities()->drawSequenceLeft(kEntityBoutarel, "510");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Boutarel, function33)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1)
			Entity::timeCheckCallback(kTime2389500, params->param2, 3, false, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
		break;

	case kActionDefault:
		setCallback(1);
		setup_function11(true);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kEntityBoutarel, "008B");

			setCallback(2);
			setup_updateFromTime(450);
			break;

		case 2:
			getSavePoints()->push(kEntityBoutarel, kEntityServers1, kAction256200848);
			break;

		case 3:
			setup_function34();
			break;
		}
		break;

	case kAction122288808:
		params->param1 = 1;
		getEntities()->drawSequenceLeft(kEntityBoutarel, "008D");
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityBoutarel, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(34, Boutarel, function34)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::timeCheck(kTime2470500, params->param1, WRAP_SETUP_FUNCTION(Boutarel, setup_function35)))
			break;

		if (getState()->time > kTime2457000 && getEvent(kEventAugustDrink)) {
			getSavePoints()->push(kEntityBoutarel, kEntityAbbot, kAction159003408);

			setCallback(1);
			setup_function15(false, "102A");
		}
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityBoutarel, kEntityAbbot, kAction101687594);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function17(kTime2479500, "102B");
			break;

		case 2:
			setCallback(3);
			setup_function16(false, "102C");
			break;

		case 3:
		case 7:
			setup_function35();
			break;

		case 4:
		case 8:
			if (getState()->time >= kTime2470500) {
				setup_function35();
				break;
			}

			if (getEvent(kEventAugustDrink)) {
				setCallback(5);
				setup_function15(false, "102A");
			} else {
				setCallback(8);
				setup_function18((TimeValue)(getState()->time + 900));
			}
			break;

		case 5:
			setCallback(6);
			setup_function17(kTime2479500, "102B");
			break;

		case 6:
			setCallback(7);
			setup_function16(false, "102C");
			break;

		case 9:
			getSavePoints()->push(kEntityBoutarel, kEntityCoudert, kAction123199584);
			break;
		}
		break;

	case kAction122865568:
		getSavePoints()->push(kEntityBoutarel, kEntityCoudert, kAction88652208);
		break;

	case kAction125039808:
		setCallback(4);
		setup_function18(kTime2457000);
		break;

	case kAction221683008:
		setCallback(9);
		setup_playSound("Mrb1001");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(35, Boutarel, function35)
	if (savepoint.action == kActionDefault) {
		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->clearSequences(kEntityBoutarel);

		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(36, Boutarel, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityBoutarel);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(37, Boutarel, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_function38();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(38, Boutarel, function38)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		break;

	case kAction135800432:
		setup_nullfunction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(39, Boutarel)

} // End of namespace LastExpress
