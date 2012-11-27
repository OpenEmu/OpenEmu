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

#include "lastexpress/entities/pascale.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Pascale::Pascale(LastExpressEngine *engine) : Entity(engine, kEntityPascale) {
	ADD_CALLBACK_FUNCTION(Pascale, draw);
	ADD_CALLBACK_FUNCTION(Pascale, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION(Pascale, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(Pascale, updateFromTime);
	ADD_CALLBACK_FUNCTION(Pascale, updatePosition);
	ADD_CALLBACK_FUNCTION(Pascale, playSound);
	ADD_CALLBACK_FUNCTION(Pascale, draw2);
	ADD_CALLBACK_FUNCTION(Pascale, welcomeSophieAndRebecca);
	ADD_CALLBACK_FUNCTION(Pascale, sitSophieAndRebecca);
	ADD_CALLBACK_FUNCTION(Pascale, welcomeCath);
	ADD_CALLBACK_FUNCTION(Pascale, function11);
	ADD_CALLBACK_FUNCTION(Pascale, chapter1);
	ADD_CALLBACK_FUNCTION(Pascale, getMessageFromAugustToTyler);
	ADD_CALLBACK_FUNCTION(Pascale, sitAnna);
	ADD_CALLBACK_FUNCTION(Pascale, welcomeAnna);
	ADD_CALLBACK_FUNCTION(Pascale, serveTatianaVassili);
	ADD_CALLBACK_FUNCTION(Pascale, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Pascale, function18);
	ADD_CALLBACK_FUNCTION(Pascale, function19);
	ADD_CALLBACK_FUNCTION(Pascale, chapter2);
	ADD_CALLBACK_FUNCTION(Pascale, chapter3);
	ADD_CALLBACK_FUNCTION(Pascale, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Pascale, function23);
	ADD_CALLBACK_FUNCTION(Pascale, welcomeAbbot);
	ADD_CALLBACK_FUNCTION(Pascale, chapter4);
	ADD_CALLBACK_FUNCTION(Pascale, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Pascale, function27);
	ADD_CALLBACK_FUNCTION(Pascale, messageFromAnna);
	ADD_CALLBACK_FUNCTION(Pascale, function29);
	ADD_CALLBACK_FUNCTION(Pascale, function30);
	ADD_CALLBACK_FUNCTION(Pascale, chapter5);
	ADD_CALLBACK_FUNCTION(Pascale, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Pascale, function33);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(1, Pascale, draw)
	Entity::draw(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(2, Pascale, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(3, Pascale, callbackActionOnDirection)
	if (savepoint.action == kActionExcuseMeCath) {
		if (!params->param1) {
			getSound()->excuseMe(kEntityPascale);
			params->param1 = 1;
		}

		return;
	}

	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(4, Pascale, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(5, Pascale, updatePosition)
	Entity::updatePosition(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(6, Pascale, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(7, Pascale, draw2)
	Entity::draw2(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Pascale, welcomeSophieAndRebecca)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_850;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("901");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			switch (getProgress().chapter) {
			default:
				break;

			case kChapter1:
				getSound()->playSound(kEntityPascale, "REB1198", kFlagInvalid, 30);
				break;

			case kChapter3:
				getSound()->playSound(kEntityPascale, "REB3001", kFlagInvalid, 30);
				break;

			case kChapter4:
				getSound()->playSound(kEntityPascale, "REB4001", kFlagInvalid, 30);
				break;
			}

			setCallback(2);
			setup_sitSophieAndRebecca();
			break;

		case 2:
			getSavePoints()->push(kEntityPascale, kEntityRebecca, kAction157370960);

			setCallback(3);
			setup_draw("905");
			break;

		case 3:
			getEntities()->clearSequences(kEntityPascale);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 4) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Pascale, sitSophieAndRebecca)
	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		callbackAction();
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityPascale, "012C1");
		getEntities()->drawSequenceLeft(kEntityRebecca, "012C2");
		getEntities()->drawSequenceLeft(kEntityTables3, "012C3");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Pascale, welcomeCath)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 && !getSoundQueue()->isBuffered(kEntityPascale))
			getEntities()->updatePositionExit(kEntityPascale, kCarRestaurant, 64);
		break;

	case kActionExitCompartment:
		if (!params->param2) {
			params->param2 = 1;

			getSound()->playSound(kEntityPascale, "HED1001A");
			getSound()->playSound(kEntityPlayer, "LIB004");

			getScenes()->loadSceneFromPosition(kCarRestaurant, 69);
		}

		callbackAction();
		break;

	case kAction4:
		if (!params->param1) {
			params->param1 = 1;
			getSound()->playSound(kEntityPascale, "HED1001");
		}
		break;

	case kActionDefault:
		getEntities()->updatePositionEnter(kEntityPascale, kCarRestaurant, 64);
		getEntities()->drawSequenceRight(kEntityPascale, "035A");
		break;

	case kActionDrawScene:
		if (params->param1 && getEntities()->isPlayerPosition(kCarRestaurant, 64)) {
			getSound()->playSound(kEntityPascale, "HED1001A");
			getSound()->playSound(kEntityPlayer, "LIB004");

			getScenes()->loadSceneFromPosition(kCarRestaurant, 69);

			callbackAction();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Pascale, function11)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		getSavePoints()->push(kEntityPascale, kEntityAugust, kAction168046720);
		getSavePoints()->push(kEntityPascale, kEntityAnna, kAction168046720);
		getSavePoints()->push(kEntityPascale, kEntityAlexei, kAction168046720);
		getEntities()->updatePositionEnter(kEntityPascale, kCarRestaurant, 55);

		setCallback(1);
		setup_welcomeCath();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityPascale, kEntityAugust, kAction168627977);
			getSavePoints()->push(kEntityPascale, kEntityAnna, kAction168627977);
			getSavePoints()->push(kEntityPascale, kEntityAlexei, kAction168627977);
			getEntities()->updatePositionExit(kEntityPascale, kCarRestaurant, 55);

			setCallback(2);
			setup_draw("905");
			break;

		case 2:
			getEntities()->clearSequences(kEntityPascale);
			getData()->entityPosition = kPosition_5900;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Pascale, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter1Handler();
		break;

	case kActionDefault:
		getSavePoints()->addData(kEntityPascale, kAction239072064, 0);
		getSavePoints()->addData(kEntityPascale, kAction257489762, 2);
		getSavePoints()->addData(kEntityPascale, kAction207769280, 6);
		getSavePoints()->addData(kEntityPascale, kAction101824388, 7);
		getSavePoints()->addData(kEntityPascale, kAction136059947, 8);
		getSavePoints()->addData(kEntityPascale, kAction223262556, 1);
		getSavePoints()->addData(kEntityPascale, kAction269479296, 3);
		getSavePoints()->addData(kEntityPascale, kAction352703104, 4);
		getSavePoints()->addData(kEntityPascale, kAction352768896, 5);
		getSavePoints()->addData(kEntityPascale, kAction191604416, 10);
		getSavePoints()->addData(kEntityPascale, kAction190605184, 11);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Pascale, getMessageFromAugustToTyler)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("902");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (!ENTITY_PARAM(1, 3)) {
				getEntities()->drawSequenceLeft(kEntityPascale, "010E");
				getEntities()->drawSequenceLeft(kEntityAugust, "BLANK");

				setCallback(2);
				setup_playSound("AUG1001");
				break;
			}

			setCallback(3);
			setup_draw("905");
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityPascale, "010B");

			setCallback(3);
			setup_draw("905");
			break;

		case 3:
			getData()->entityPosition = kPosition_5900;
			getEntities()->clearSequences(kEntityPascale);
			getSavePoints()->push(kEntityPascale, kEntityVerges, kActionDeliverMessageToTyler);
			ENTITY_PARAM(0, 1) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Pascale, sitAnna)
	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		getEntities()->updatePositionExit(kEntityPascale, kCarRestaurant, 62);

		callbackAction();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(kEntityTables0, "001C3");
		getEntities()->drawSequenceRight(kEntityAnna, "001C2");
		getEntities()->drawSequenceRight(kEntityPascale, "001C1");

		getEntities()->updatePositionEnter(kEntityPascale, kCarRestaurant, 62);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Pascale, welcomeAnna)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("901");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSound()->playSound(kEntityPascale, "ANN1047");

			setCallback(2);
			setup_sitAnna();
			break;

		case 2:
			getSavePoints()->push(kEntityPascale, kEntityAnna, kAction157370960);

			setCallback(3);
			setup_draw("904");
			break;

		case 3:
			getEntities()->clearSequences(kEntityPascale);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 2) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Pascale, serveTatianaVassili)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("903");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityPascale, kEntityTatiana, kAction122358304);
			getEntities()->drawSequenceLeft(kEntityPascale, "014B");
			getEntities()->updatePositionEnter(kEntityPascale, kCarRestaurant, 67);

			if (getSoundQueue()->isBuffered("TAT1069A"))
				getSoundQueue()->processEntry("TAT1069A");
			else if (getSoundQueue()->isBuffered("TAT1069B"))
				getSoundQueue()->processEntry("TAT1069B");

			setCallback(2);
			setup_playSound("TAT1066");
			break;

		case 2:
			getEntities()->updatePositionExit(kEntityPascale, kCarRestaurant, 67);
			getSavePoints()->push(kEntityPascale, kEntityTatiana, kAction122288808);

			setCallback(3);
			setup_draw("906");
			break;

		case 3:
			getEntities()->clearSequences(kEntityPascale);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 3) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Pascale, chapter1Handler)
switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param2) {
			if (getEntities()->isPlayerPosition(kCarRestaurant, 69)
			 || getEntities()->isPlayerPosition(kCarRestaurant, 70)
			 || getEntities()->isPlayerPosition(kCarRestaurant, 71))
				params->param2 = 1;

			if (!params->param2 && getEntities()->isPlayerPosition(kCarRestaurant, 61))
				params->param1 = 1;
		}

		if (!getEntities()->isInKitchen(kEntityPascale))
			break;

		if (ENTITY_PARAM(0, 5) && ENTITY_PARAM(0, 6)) {
			setup_function18();
			break;
		}

		if (!getEntities()->isSomebodyInsideRestaurantOrSalon())
			goto label_callback3;

		if (params->param1 && !params->param2 && getEntities()->isPlayerPosition(kCarRestaurant, 61)) {
			setCallback(1);
			setup_function11();
			break;
		}

label_callback1:
		if (ENTITY_PARAM(0, 1) && !ENTITY_PARAM(1, 3)) {
			setCallback(2);
			setup_getMessageFromAugustToTyler();
			break;
		}

label_callback2:
		if (ENTITY_PARAM(0, 3)) {
			setCallback(3);
			setup_serveTatianaVassili();
			break;
		}

label_callback3:
		if (ENTITY_PARAM(0, 2)) {
			setCallback(4);
			setup_welcomeAnna();
			break;
		}

label_callback4:
		if (ENTITY_PARAM(0, 4)) {
			setCallback(5);
			setup_welcomeSophieAndRebecca();
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			params->param1 = 0;
			params->param2 = 1;
			goto label_callback1;

		case 2:
			goto label_callback2;

		case 3:
			goto label_callback3;

		case 4:
			goto label_callback4;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Pascale, function18)
	if (savepoint.action != kActionNone)
		return;

	if (getState()->time > kTime1242000 && !params->param1) {
		params->param1 = 1;

		getSavePoints()->push(kEntityPascale, kEntityServers0, kAction101632192);
		getSavePoints()->push(kEntityPascale, kEntityServers1, kAction101632192);
		getSavePoints()->push(kEntityPascale, kEntityCooks, kAction101632192);
		getSavePoints()->push(kEntityPascale, kEntityVerges, kAction101632192);

		setup_function19();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Pascale, function19)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1 && getEntityData(kEntityPlayer)->entityPosition < kPosition_3650) {
			getObjects()->update(kObject65, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getSavePoints()->push(kEntityPascale, kEntityTables0, kActionDrawTablesWithChairs, "001P");
			getSavePoints()->push(kEntityPascale, kEntityTables1, kActionDrawTablesWithChairs, "005J");
			getSavePoints()->push(kEntityPascale, kEntityTables2, kActionDrawTablesWithChairs, "009G");
			getSavePoints()->push(kEntityPascale, kEntityTables3, kActionDrawTablesWithChairs, "010M");
			getSavePoints()->push(kEntityPascale, kEntityTables4, kActionDrawTablesWithChairs, "014F");
			getSavePoints()->push(kEntityPascale, kEntityTables5, kActionDrawTablesWithChairs, "024D");

			params->param1 = 1;
		}
		break;

	case kActionDefault:
		getData()->car = kCarRestaurant;
		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;

		getEntities()->clearSequences(kEntityPascale);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Pascale, chapter2)
	if (savepoint.action == kActionDefault) {
		getEntities()->clearSequences(kEntityPascale);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes1;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObject65, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorForward);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Pascale, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityPascale);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 4) = 0;
		ENTITY_PARAM(0, 7) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Pascale, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!getEntities()->isInKitchen(kEntityPascale))
			break;

		if (ENTITY_PARAM(0, 7)) {
			setCallback(1);
			setup_function23();
			break;
		}

label_callback:
		if (ENTITY_PARAM(0, 4)) {
			setCallback(2);
			setup_welcomeSophieAndRebecca();
		}
		break;

	case kActionCallback:
		if (getCallback() == 1)
			goto label_callback;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Pascale, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;
		getEntities()->updatePositionEnter(kEntityPascale, kCarRestaurant, 67);

		setCallback(1);
		setup_welcomeAbbot();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->updatePositionExit(kEntityPascale, kCarRestaurant, 67);
			getSavePoints()->push(kEntityPascale, kEntityAbbot, kAction122288808);

			setCallback(2);
			setup_draw("906");
			break;

		case 2:
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 7) = 0;
			getEntities()->clearSequences(kEntityPascale);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Pascale, welcomeAbbot)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1) {
			getSound()->playSound(kEntityPascale, "ABB3015A");
			params->param1 = 1;
		}
		break;

	case kActionExitCompartment:
		callbackAction();
		break;

	case kAction10:
		getSavePoints()->push(kEntityPascale, kEntityTables4, kAction136455232);
		break;

	case kActionDefault:
		getSound()->playSound(kEntityPascale, "ABB3015", kFlagInvalid, 105);
		getEntities()->drawSequenceRight(kEntityPascale, "029A1");
		getEntities()->drawSequenceRight(kEntityAbbot, "029A2");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Pascale, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityPascale);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 4) = 0;
		ENTITY_PARAM(0, 8) = 0;

		ENTITY_PARAM(1, 1) = 0;
		ENTITY_PARAM(1, 2) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Pascale, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime2511000 && !params->param4) {
			params->param2 = 1;
			params->param4 = 1;
		}

		if (!getEntities()->isInKitchen(kEntityPascale))
			break;

		if (getEntities()->isSomebodyInsideRestaurantOrSalon()) {
			if (ENTITY_PARAM(0, 8)) {
				setCallback(1);
				setup_function27();
				break;
			}

label_callback1:
			if (ENTITY_PARAM(1, 2) && ENTITY_PARAM(1, 4)) {
				if (!params->param3)
					params->param3 = (uint)(getState()->time + 9000);

				if (params->param5 != kTimeInvalid) {

					if (params->param3 < getState()->time) {
						params->param5 = kTimeInvalid;
						setCallback(2);
						setup_messageFromAnna();
						break;
					}

					if (!getEntities()->isInRestaurant(kEntityPlayer) || !params->param5)
						params->param5 = (uint)getState()->time;

					if (params->param5 < getState()->time) {
						params->param5 = kTimeInvalid;
						setCallback(2);
						setup_messageFromAnna();
						break;
					}
				}
			}

label_callback2:
			if (params->param1 && !params->param2 && getEntities()->isPlayerPosition(kCarRestaurant, 61)) {
				setCallback(3);
				setup_function11();
				break;
			}
		}

label_callback3:
		if (ENTITY_PARAM(0, 4)) {
			setCallback(4);
			setup_welcomeSophieAndRebecca();
		}
		break;

	case kActionDefault:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 69)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 70)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 71))
			params->param2 = 1;
		break;

	case kActionDrawScene:
		if (!params->param2) {
			if (getEntities()->isPlayerPosition(kCarRestaurant, 69)
			 || getEntities()->isPlayerPosition(kCarRestaurant, 70)
			 || getEntities()->isPlayerPosition(kCarRestaurant, 71))
				params->param2 = 1;

			if (!params->param2 && getEntities()->isPlayerPosition(kCarRestaurant, 61))
				params->param1 = 1;
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback1;

		case 2:
			goto label_callback2;

		case 3:
			params->param1 = 0;
			params->param2 = 1;
			goto label_callback3;
		}
		break;

	case kAction201431954:
		ENTITY_PARAM(0, 4) = 0;
		ENTITY_PARAM(0, 8) = 0;

		getSavePoints()->push(kEntityPascale, kEntityTables0, kActionDrawTablesWithChairs, "001P");
		getSavePoints()->push(kEntityPascale, kEntityTables1, kActionDrawTablesWithChairs, "005J");
		getSavePoints()->push(kEntityPascale, kEntityTables2, kActionDrawTablesWithChairs, "009G");
		getSavePoints()->push(kEntityPascale, kEntityTables3, kActionDrawTablesWithChairs, "010M");
		getSavePoints()->push(kEntityPascale, kEntityTables4, kActionDrawTablesWithChairs, "014F");
		getSavePoints()->push(kEntityPascale, kEntityTables5, kActionDrawTablesWithChairs, "024D");

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Pascale, function27)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(1, 1)) {
			setCallback(2);
			setup_updateFromTime(450);
		}
		break;

	case kActionDefault:
		setCallback(1);
		setup_function29();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->clearSequences(kEntityPascale);
			break;

		case 2:
			getSavePoints()->push(kEntityPascale, kEntityCoudert, kAction123712592);

			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			setCallback(4);
			setup_function30();
			break;

		case 4:
			getEntities()->clearSequences(kEntityPascale);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 8) = 0;
			ENTITY_PARAM(1, 1) = 0;
			ENTITY_PARAM(1, 2) = 1;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Pascale, messageFromAnna)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("902");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityPascale, kEntityAugust, kAction122358304);
			getEntities()->drawSequenceLeft(kEntityPascale, "010E2");

			setCallback(2);
			setup_playSound("Aug4001");
			break;

		case 2:
			getSavePoints()->push(kEntityPascale, kEntityAugust, kAction123793792);

			setCallback(3);
			setup_draw("905");
			break;

		case 3:
			getEntities()->clearSequences(kEntityPascale);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(1, 2) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Pascale, function29)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_1540;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("817DD");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceRight(kEntityPascale, "817DS");
			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityPascale);

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			getData()->entityPosition = kPosition_850;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Pascale, function30)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_9270;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("817US");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceRight(kEntityPascale, "817UD");
			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityPascale);

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			getData()->entityPosition = kPosition_5900;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Pascale, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityPascale);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Pascale, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_function33();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Pascale, function33)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param4) {
			if (Entity::updateParameter(params->param5, getState()->time, 4500)) {
				getObjects()->update(kObjectCompartmentG, kEntityPascale, kObjectLocation1, kCursorNormal, kCursorNormal);

				setCallback(1);
				setup_playSound("Wat5010");
				break;
			}
		}

label_callback1:
		if (params->param1) {
			if (!Entity::updateParameter(params->param6, getState()->timeTicks, 75))
				break;

			params->param1 = 0;
			params->param2 = 2;

			getObjects()->update(kObjectCompartmentG, kEntityPascale, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param6 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (params->param1) {
			getObjects()->update(kObjectCompartmentG, kEntityPascale, kObjectLocation1, kCursorNormal, kCursorNormal);
			params->param1 = 0;

			setCallback(2);
			setup_playSound(getSound()->justCheckingCath());
		} else {
			setCallback(savepoint.action == kActionKnock ? 3 : 4);
			setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;

		getObjects()->update(kObjectCompartmentG, kEntityPascale, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (params->param2 || params->param1) {
			params->param1 = 0;
			params->param2 = 0;
			params->param3 = 0;

			getObjects()->update(kObjectCompartmentG, kEntityPascale, kObjectLocation1, kCursorHandKnock, kCursorHand);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentG, kEntityPascale, kObjectLocation1, kCursorHandKnock, kCursorHand);
			goto label_callback1;

		case 2:
			getObjects()->update(kObjectCompartmentG, kEntityPascale, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;

		case 3:
		case 4:
			params->param3++;

			if (params->param3 == 1 || params->param3 == 2) {
				getObjects()->update(kObjectCompartmentG, kEntityPascale, kObjectLocation1, kCursorNormal, kCursorNormal);
				setCallback(params->param3 == 1 ? 5 : 6);
				setup_playSound(params->param3 == 1 ? "Wat5001" : "Wat5002");
			}
			break;

		case 5:
			params->param1 = 1;
			getObjects()->update(kObjectCompartmentG, kEntityPascale, kObjectLocation1, kCursorTalk, kCursorNormal);
			break;

		case 6:
			params->param2 = 1;
			break;

		case 7:
			params->param4 = 1;
			break;
		}
		break;

	case kAction135800432:
		setup_nullfunction();
		break;

	case kAction169750080:
		if (getSoundQueue()->isBuffered(kEntityPascale)) {
			params->param4 = 1;
		} else {
			setCallback(7);
			setup_playSound("Wat5002");
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(34, Pascale)

} // End of namespace LastExpress
