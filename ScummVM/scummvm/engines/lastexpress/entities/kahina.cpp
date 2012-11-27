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

#include "lastexpress/entities/kahina.h"

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

Kahina::Kahina(LastExpressEngine *engine) : Entity(engine, kEntityKahina) {
	ADD_CALLBACK_FUNCTION(Kahina, reset);
	ADD_CALLBACK_FUNCTION(Kahina, playSound);
	ADD_CALLBACK_FUNCTION(Kahina, savegame);
	ADD_CALLBACK_FUNCTION(Kahina, updateFromTime);
	ADD_CALLBACK_FUNCTION(Kahina, updateFromTicks);
	ADD_CALLBACK_FUNCTION(Kahina, function6);
	ADD_CALLBACK_FUNCTION(Kahina, updateEntity2);
	ADD_CALLBACK_FUNCTION(Kahina, updateEntity);
	ADD_CALLBACK_FUNCTION(Kahina, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(Kahina, chapter1);
	ADD_CALLBACK_FUNCTION(Kahina, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Kahina, function12);
	ADD_CALLBACK_FUNCTION(Kahina, function13);
	ADD_CALLBACK_FUNCTION(Kahina, function14);
	ADD_CALLBACK_FUNCTION(Kahina, function15);
	ADD_CALLBACK_FUNCTION(Kahina, chapter2);
	ADD_CALLBACK_FUNCTION(Kahina, chapter2Handler);
	ADD_CALLBACK_FUNCTION(Kahina, chapter3);
	ADD_CALLBACK_FUNCTION(Kahina, function19);
	ADD_CALLBACK_FUNCTION(Kahina, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Kahina, function21);
	ADD_CALLBACK_FUNCTION(Kahina, function22);
	ADD_CALLBACK_FUNCTION(Kahina, function23);
	ADD_CALLBACK_FUNCTION(Kahina, function24);
	ADD_CALLBACK_FUNCTION(Kahina, function25);
	ADD_CALLBACK_FUNCTION(Kahina, function26);
	ADD_CALLBACK_FUNCTION(Kahina, function27);
	ADD_CALLBACK_FUNCTION(Kahina, chapter4);
	ADD_CALLBACK_FUNCTION(Kahina, chapter5);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Kahina, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Kahina, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(3, Kahina, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(4, Kahina, updateFromTime, uint32)
	if (savepoint.action == kAction137503360) {
		ENTITY_PARAM(0, 2) = 1;
		callbackAction();
	}

	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(5, Kahina, updateFromTicks)
	Entity::updateFromTicks(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(6, Kahina, function6, TimeValue)
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
		ENTITY_PARAM(0, 2) = 0;

		setCallback(1);
		setup_updateEntity2(kCarRedSleeping, kPosition_540);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (ENTITY_PARAM(0, 1) || ENTITY_PARAM(0, 2)) {
				callbackAction();
				break;
			}

			getEntities()->clearSequences(kEntityKahina);
			break;

		case 2:
		case 3:
			if (ENTITY_PARAM(0, 1) || ENTITY_PARAM(0, 2)) {
				callbackAction();
				break;
			}

			getEntities()->clearSequences(kEntityKahina);

			setCallback(4);
			setup_updateFromTime(450);
			break;

		case 4:
			if (ENTITY_PARAM(0, 2)) {
				callbackAction();
				break;
			}

			setCallback(5);
			setup_updateEntity2(kCarRedSleeping, kPosition_540);
			break;

		case 5:
			if (ENTITY_PARAM(0, 1) || ENTITY_PARAM(0, 2)) {
				callbackAction();
				break;
			}

			getEntities()->clearSequences(kEntityKahina);
			break;
		}
		break;

	case kAction137503360:
		ENTITY_PARAM(0, 2) = 1;

		callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(7, Kahina, updateEntity2, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->updateEntity(_entityIndex, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;

	case kActionDefault:
		if (getEntities()->updateEntity(_entityIndex, (CarIndex)params->param1, (EntityPosition)params->param2)) {
			callbackAction();
		} else if (getEntities()->isDistanceBetweenEntities(kEntityKahina, kEntityPlayer, 1000)
				&& !getEntities()->isInGreenCarEntrance(kEntityPlayer)
				&& !getEntities()->isInsideCompartments(kEntityPlayer)
				&& !getEntities()->checkFields10(kEntityPlayer)) {

			if (getData()->car == kCarGreenSleeping || getData()->car == kCarRedSleeping) {
				ENTITY_PARAM(0, 1) = 1;
				callbackAction();
			}
		}
		break;

	case kAction137503360:
		ENTITY_PARAM(0, 2) = 1;
		callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(8, Kahina, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kActionExcuseMeCath) {
		if (getEvent(kEventKronosConversation) || getEvent(kEventKronosConversationFirebird)) {
			getSound()->playSound(kEntityPlayer, rnd(2) ? "CAT1019" : "CAT1019A");
		} else {
			getSound()->excuseMeCath();
		}
		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(9, Kahina, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Kahina, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Kahina, setup_chapter1Handler));
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_5000;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarKronos;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Kahina, chapter1Handler)
	if (savepoint.action != kActionNone)
		return;

	if (getProgress().jacket != kJacketOriginal)
		Entity::timeCheckSavepoint(kTime1107000, params->param1, kEntityKahina, kEntityMertens, kAction238732837);

	if (getProgress().eventMertensKronosInvitation)
		setup_function12();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Kahina, function12)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTime1485000, params->param2, WRAP_SETUP_FUNCTION(Kahina, setup_function13));
		break;

	case kActionKnock:
		getSound()->playSound(kEntityPlayer, "LIB012");
		// Fallback to next action

	case kActionOpenDoor:
		if (!getEvent(kEventKronosGoingToInvitation)) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventKronosGoingToInvitation);
			break;
		}

		if (savepoint.action == kActionOpenDoor)
			getSound()->playSound(kEntityPlayer, "LIB014");

		getScenes()->loadSceneFromPosition(kCarKronos, 80);
		getSavePoints()->push(kEntityKahina, kEntityKronos, kAction171849314);
		params->param1 = 1;
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentKronos, kEntityKahina, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventKronosGoingToInvitation);
			getScenes()->loadSceneFromPosition(kCarKronos, 80);
			getSavePoints()->push(kEntityKahina, kEntityKronos, kAction171849314);
			params->param1 = 1;
		}
		break;

	case kAction137685712:
		setup_function13();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Kahina, function13)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getProgress().field_14 || getState()->time >= kTime1201500 || params->param2 == kTimeInvalid || params->param1 >= getState()->time)
			break;

		if (getState()->time <= kTime1197000) {
			if (!getEntities()->isPlayerInCar(kCarGreenSleeping) || !params->param2) {
				params->param2 = (uint)getState()->time;

				if (!getState()->time)
					goto label_callback;
			}

			if (params->param2 >= getState()->time)
				break;
		}

		params->param2 = kTimeInvalid;

label_callback:
		setCallback(1);
		setup_function15();
		break;

	case kActionDefault:
		getData()->car = kCarKronos;
		getData()->entityPosition = kPosition_5000;
		getData()->location = kLocationOutsideCompartment;

		getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		params->param1 = (uint)getState()->time + 1800;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Kahina, function14)
	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		getEntities()->exitCompartment(kEntityKahina, kObjectCompartmentF);
		callbackAction();
		break;

	case kAction4:
		getEntities()->exitCompartment(kEntityKahina, kObjectCompartmentF);
		callbackAction();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(kEntityKahina, "616Cf");
		getEntities()->enterCompartment(kEntityKahina, kObjectCompartmentF);
		getSavePoints()->push(kEntityKahina, kEntityMax, kAction158007856);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Kahina, function15)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param2 != kTimeInvalid) {
			if (Entity::updateParameterTime((TimeValue)params->param1, !getEntities()->isPlayerInCar(kCarRedSleeping), params->param2, 0)) {
				setCallback(9);
				setup_updateEntity(kCarRedSleeping, kPosition_4070);
			}
		}
		break;

	case kActionDefault:
		getProgress().field_14 = 19;

		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_8200);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getEntities()->hasValidFrame(kEntityKahina)) {
				setCallback(2);
				setup_updateEntity(kCarRedSleeping, kPosition_9460);
				break;
			}
			// Fallback to next case

		case 4:
			if (getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_8200)
			 || getEntities()->isOutsideAlexeiWindow()
			 || getEntities()->isDistanceBetweenEntities(kEntityKahina, kEntityPlayer, 2000)) {
				if (getProgress().field_14 == 19)
					getProgress().field_14 = 0;

				setCallback(8);
				setup_updateEntity(kCarGreenSleeping, kPosition_9460);
			} else {
				setCallback(5);
				setup_enterExitCompartment("616Aa", kObjectCompartment1);
			}
			break;

		case 2:
			setCallback(3);
			setup_updateFromTime(1800);
			break;

		case 3:
			setCallback(4);
			setup_updateEntity(kCarGreenSleeping, kPosition_8200);
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityKahina);
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorNormal);
			getObjects()->update(kObjectHandleBathroom, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorNormal);

			setCallback(6);
			setup_updateFromTime(900);
			break;

		case 6:
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectHandleBathroom, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			setCallback(7);
			setup_enterExitCompartment("616Ba", kObjectCompartment1);
			break;

		case 7:
			getData()->location = kLocationOutsideCompartment;

			if (getProgress().field_14 == 19)
				getProgress().field_14 = 0;

			setCallback(8);
			setup_updateEntity(kCarGreenSleeping, kPosition_9460);
			break;

		case 8:
			getEntities()->clearSequences(kEntityKahina);
			params->param1 = (uint)getState()->time + 4500;
			break;

		case 9:
			setCallback(10);
			setup_function14();
			break;

		case 10:
			setCallback(11);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 11:
			if (getEntities()->checkFields19(kEntityPlayer, kCarRedSleeping, kPosition_6130)) {
				setCallback(15);
				setup_updateEntity(kCarRedSleeping, kPosition_9460);
			} else {
				setCallback(12);
				setup_enterExitCompartment("616Ac", kObjectCompartmentC);
			}
			break;

		case 12:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityKahina);

			getObjects()->update(kObjectCompartmentC, kEntityPlayer, getObjects()->get(kObjectCompartmentC).location, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject50, kEntityPlayer, getObjects()->get(kObject50).location, kCursorNormal, kCursorNormal);

			setCallback(13);
			setup_updateFromTime(900);
			break;

		case 13:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, getObjects()->get(kObjectCompartmentC).location, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject50, kEntityPlayer, getObjects()->get(kObject50).location, kCursorHandKnock, kCursorHand);

			setCallback(14);
			setup_enterExitCompartment("616Bc", kObjectCompartmentC);
			break;

		case 14:
			getData()->location = kLocationOutsideCompartment;

			setCallback(15);
			setup_updateEntity(kCarRedSleeping, kPosition_9460);
			break;

		case 15:
			getEntities()->clearSequences(kEntityKahina);

			setCallback(16);
			setup_updateFromTime(900);
			break;

		case 16:
			setCallback(17);
			setup_updateEntity(kCarKronos, kPosition_9270);
			break;

		case 17:
			getEntities()->clearSequences(kEntityKahina);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Kahina, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter2Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityKahina);

		getData()->entityPosition = kPosition_6000;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarKronos;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentKronos, kEntityKahina, kObjectLocation1, kCursorHandKnock, kCursorHand);

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Kahina, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			if (Entity::updateParameter(params->param2, getState()->time, 9000)) {
				params->param1 = 1;
				params->param2 = 0;
			}
		}

		if (getEvent(kEventKahinaAskSpeakFirebird) && getEvent(kEventKronosConversationFirebird) && getEntities()->isInsideTrainCar(kEntityPlayer, kCarKronos)) {
			if (Entity::updateParameter(params->param3, getState()->time, 900)) {
				setCallback(1);
				setup_savegame(kSavegameTypeEvent, kEventKronosConversationFirebird);
				break;
			}
		}

label_callback_3:
		if (getState()->time > kTime1845000 && getEvent(kEventKronosConversationFirebird) && getEntities()->isInKronosSalon(kEntityPlayer)) {
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getScenes()->loadSceneFromPosition(kCarKronos, 87);
		}
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (getEvent(kEventKronosConversationFirebird))
			break;

		if (getEvent(kEventKahinaAskSpeakFirebird)) {
			if (getSoundQueue()->isBuffered(kEntityKahina))
				getSoundQueue()->processEntry(kEntityKahina);

			if (savepoint.action == kActionKnock)
				getSound()->playSound(kEntityPlayer, "LIB012");

			setCallback(4);
			setup_savegame(kSavegameTypeEvent, kEventKronosConversationFirebird);
			break;
		}

		if (getEvent(kEventMilosCompartmentVisitAugust) || getEvent(kEventTatianaGivePoem) || getEvent(kEventTatianaBreakfastGivePoem)) {
			if (savepoint.action == kActionKnock)
				getSound()->playSound(kEntityPlayer, "LIB012");

			setCallback(7);
			setup_savegame(kSavegameTypeEvent, kEventKahinaAskSpeakFirebird);
			break;
		}

		if (params->param1) {
			if (savepoint.action == kActionKnock)
				getSound()->playSound(kEntityPlayer, "LIB012");

			getAction()->playAnimation(kEventKahinaAskSpeak);
			getScenes()->processScene();

			getObjects()->update(kObjectCompartmentKronos, kEntityKahina, kObjectLocation1, kCursorNormal, kCursorNormal);

			setCallback(8);
			setup_playSound("KRO3003");
		} else {
			getObjects()->update(kObjectCompartmentKronos, kEntityKahina, kObjectLocation1, kCursorNormal, kCursorNormal);

			setCallback(savepoint.action == kActionKnock ? 9 : 10);
			setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kActionDefault:
		params->param1 = 1;
		getObjects()->update(kObjectCompartmentKronos, kEntityKahina, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 4:
			getAction()->playAnimation(kEventKronosConversationFirebird);
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getScenes()->loadSceneFromPosition(kCarKronos, 80, 1);

			setCallback(getCallback() + 1);
			setup_updateFromTime(900);
			break;

		case 2:
		case 5:
			setCallback(getCallback() + 1);
			setup_playSound("KRO3005");
			break;

		case 3:
			goto label_callback_3;

		case 7:
			getAction()->playAnimation(kEventKahinaAskSpeakFirebird);
			getScenes()->loadSceneFromPosition(kCarKronos, 81);
			getSound()->playSound(kEntityKahina, "KRO3004");
			break;

		case 8:
		case 9:
		case 10:
			getObjects()->update(kObjectCompartmentKronos, kEntityKahina, kObjectLocation1, kCursorHandKnock, kCursorHand);
			if (getCallback() == 8)
				params->param1 = 0;
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Kahina, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityKahina);

		getData()->entityPosition = kPosition_5000;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarKronos;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(19, Kahina, function19, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEvent(kEventAnnaBaggageArgument))
			RESET_ENTITY_STATE(kEntityKahina, Kahina, setup_function22);

		if (getEntities()->updateEntity(kEntityKahina, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;

	case kActionExcuseMeCath:
		if (getEvent(kEventKronosConversation) || getEvent(kEventKronosConversationFirebird))
			getSound()->playSound(kEntityPlayer, rnd(2) ? "CAT1019" : "CAT1019A");
		else
			getSound()->excuseMeCath();
		break;

	case kActionExcuseMe:
		getSound()->excuseMe(kEntityKahina);
		break;

	case kActionDefault:
		if (getEntities()->updateEntity(kEntityKahina, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Kahina, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEvent(kEventKronosVisit))
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);

		if (getEntities()->isInKronosSanctum(kEntityPlayer)) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventKahinaPunchSuite4);
			break;
		}

label_callback_1:
		if (getState()->time > kTime2079000 && !params->param2) {
			params->param2 = 1;

			if (getEvent(kEventKahinaAskSpeakFirebird)
			 && !getEvent(kEventKronosConversationFirebird)
			 && getEntities()->isInsideTrainCar(kEntityPlayer, kCarKronos)) {
				setCallback(2);
				setup_savegame(kSavegameTypeEvent, kEventKronosConversationFirebird);
				break;
			}

label_callback_2:
			if (getEntities()->isInKronosSalon(kEntityPlayer))
				getScenes()->loadSceneFromPosition(kCarKronos, 87);

			setup_function21();
			break;
		}

		if (!params->param1) {
			if (Entity::updateParameter(params->param3, getState()->time, 9000)) {
				params->param1 = 1;
				params->param3 = 0;
			}
		}

		if (getEvent(kEventKahinaAskSpeakFirebird)
		 && !getEvent(kEventKronosConversationFirebird)
		 && getEntities()->isInsideTrainCar(kEntityPlayer, kCarKronos)) {
			if (!Entity::updateParameter(params->param4, getState()->time, 900))
				break;

			setCallback(3);
			setup_savegame(kSavegameTypeEvent, kEventKronosConversationFirebird);
		}
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (!getEvent(kEventKronosConversationFirebird)) {

			if (getEvent(kEventKahinaAskSpeakFirebird)) {
				if (savepoint.action == kActionKnock)
					getSound()->playSound(kEntityPlayer, "LIB012");

				setCallback(6);
				setup_savegame(kSavegameTypeEvent, kEventKronosConversationFirebird);
				break;
			}

			if (getEvent(kEventMilosCompartmentVisitAugust) || getEvent(kEventTatianaGivePoem) || getEvent(kEventTatianaBreakfastGivePoem)) {
				if (savepoint.action == kActionKnock)
					getSound()->playSound(kEntityPlayer, "LIB012");

				setCallback(9);
				setup_savegame(kSavegameTypeEvent, kEventKahinaAskSpeakFirebird);
				break;
			}

			if (params->param1) {
				if (savepoint.action == kActionKnock)
					getSound()->playSound(kEntityPlayer, "LIB012");

				getAction()->playAnimation(kEventKahinaAskSpeak);
				getScenes()->processScene();
				getObjects()->update(kObjectCompartmentKronos, kEntityKahina, kObjectLocation1, kCursorNormal, kCursorNormal);

				setCallback(10);
				setup_playSound("KRO3003");
				break;
			}

			getObjects()->update(kObjectCompartmentKronos, kEntityKahina, kObjectLocation1, kCursorNormal, kCursorNormal);

			setCallback(savepoint.action == kActionKnock ? 11 : 12);
			setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kActionDefault:
		if (getEvent(kEventKronosConversationFirebird)) {
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
		} else {
			getObjects()->update(kObjectCompartmentKronos, kEntityKahina, kObjectLocation1, kCursorHandKnock, kCursorHand);
			params->param1 = 1;
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventKahinaPunchSuite4);
			getLogic()->gameOver(kSavegameTypeEvent2, kEventCathJumpDownCeiling, kSceneNone, false);
			goto label_callback_1;

		case 2:
			getAction()->playAnimation(kEventKronosConversationFirebird);
			getScenes()->loadSceneFromPosition(kCarKronos, 87);
			goto label_callback_2;

		case 3:
			getAction()->playAnimation(kEventKronosConversationFirebird);
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getScenes()->loadSceneFromPosition(kCarKronos, 80, 1);

			setCallback(4);
			setup_updateFromTime(900);
			break;

		case 4:
			setCallback(5);
			setup_playSound("KRO3005");
			break;

		case 6:
			getAction()->playAnimation(kEventKronosConversationFirebird);
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getScenes()->loadSceneFromPosition(kCarKronos, 80, 1);

			setCallback(7);
			setup_updateFromTime(900);
			break;

		case 7:
			setCallback(8);
			setup_playSound("KRO3005");
			break;

		case 9:
			getAction()->playAnimation(kEventKahinaAskSpeakFirebird);
			getScenes()->loadSceneFromPosition(kCarKronos, 81);
			getSound()->playSound(kEntityKahina, "KRO3004");
			break;

		case 10:
			params->param1 = 0;
			// Fallback to next case

		case 11:
		case 12:
			getObjects()->update(kObjectCompartmentKronos, kEntityKahina, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Kahina, function21)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			if (!params->param3)
				params->param3 = (uint)getState()->time + 4500;

			if (params->param6 != kTimeInvalid) {
				if (Entity::updateParameterTime((TimeValue)params->param3, (getEntities()->isPlayerPosition(kCarKronos, 80) || getEntities()->isPlayerPosition(kCarKronos, 88)), params->param5, 0)) {
					setCallback(2);
					setup_function23();
					break;
				}
			}
		}

label_callback_2:
		if (params->param2) {

			if (!params->param4)
				params->param4 = (uint)getState()->time + 4500;

			if (params->param6 != kTimeInvalid) {
				if (Entity::updateParameterTime((TimeValue)params->param3, (getEntities()->isPlayerPosition(kCarKronos, 80) || getEntities()->isPlayerPosition(kCarKronos, 88)), params->param6, 0)) {
					getSound()->playSound(kEntityPlayer, "LIB014", getSound()->getSoundFlag(kEntityKahina));
					getSound()->playSound(kEntityPlayer, "LIB015", getSound()->getSoundFlag(kEntityKahina));

					getEntities()->drawSequenceLeft(kEntityKahina, "202a");

					params->param2 = 0;
				}
			}
		}

		if (!getProgress().field_44
		 && getState()->time > kTime2214000) {

			ObjectLocation location = getInventory()->get(kItemFirebird)->location;

			if (location == kObjectLocation3 || location == kObjectLocation7) {
				setCallback(3);
				setup_function25();
			} else if (location == kObjectLocation1 || location == kObjectLocation2) {
				setCallback(4);
				setup_function26();
			}
		}
		break;

	case kActionDefault:
		getData()->car = kCarKronos;
		getData()->entityPosition = kPosition_5000;
		getData()->location = kLocationOutsideCompartment;

		getEntities()->drawSequenceLeft(kEntityKahina, "202a");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			params->param1 = 0;
			params->param2 = 1;
			break;

		case 2:
			params->param1 = 0;
			params->param2 = 1;
			goto label_callback_2;
		}
		break;

	case kAction92186062:
		if (params->param1) {
			setCallback(1);
			setup_function23();
		}
		break;

	case kAction134611040:
		if (getEvent(kEventConcertLeaveWithBriefcase))
			setup_function24();
		break;

	case kAction137503360:
		setup_function22();
		break;

	case kAction237555748:
		params->param1 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Kahina, function22)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			ObjectLocation location = getInventory()->get(kItemFirebird)->location;

			if (ENTITY_PARAM(0, 3) || location == kObjectLocation3 || location == kObjectLocation7) {
				setCallback(1);
				setup_function25();
			} else if (location == kObjectLocation2 || location == kObjectLocation1) {
				setCallback(2);
				setup_function26();
			}
		}
		break;

	case kActionDefault:
		getData()->car = kCarKronos;
		getData()->entityPosition = kPosition_5000;
		getData()->location = kLocationOutsideCompartment;
		break;

	case kActionDrawScene:
		if (getData()->car > kCarGreenSleeping || (getData()->car == kCarGreenSleeping && getData()->entityPosition > kPosition_2740))
			params->param1 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Kahina, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getSound()->playSound(kEntityPlayer, "LIB014", getSound()->getSoundFlag(kEntityKahina));
		getSound()->playSound(kEntityPlayer, "LIB015", getSound()->getSoundFlag(kEntityKahina), 15);

		getEntities()->clearSequences(kEntityKahina);

		getData()->car = kCarGreenSleeping;
		getData()->entityPosition = kPosition_540;

		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_4070);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getEntities()->checkFields19(kEntityPlayer, kCarRedSleeping, kPosition_4455) || getEntities()->isOutsideAnnaWindow()) {
				setCallback(5);
				setup_updateEntity(kCarRedSleeping, kPosition_9460);
				break;
			} else {
				setCallback(2);
				setup_enterExitCompartment("616Cf", kObjectCompartmentF);
			}
			break;

		case 2:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityKahina);

			getObjects()->update(kObjectCompartmentF, kEntityPlayer, getObjects()->get(kObjectCompartmentF).location, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject53, kEntityPlayer, getObjects()->get(kObject53).location, kCursorNormal, kCursorNormal);

			setCallback(3);
			setup_updateFromTime(900);
			break;

		case 3:
			getObjects()->update(kObjectCompartmentF, kEntityPlayer, getObjects()->get(kObjectCompartmentF).location, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject53, kEntityPlayer, getObjects()->get(kObject53).location, kCursorHandKnock, kCursorHand);

			setCallback(4);
			setup_enterExitCompartment("616Df", kObjectCompartmentF);
			break;

		case 4:
			getData()->location = kLocationOutsideCompartment;

			setCallback(5);
			setup_updateEntity(kCarRedSleeping, kPosition_9460);
			break;

		case 5:
			getEntities()->clearSequences(kEntityKahina);

			setCallback(6);
			setup_updateFromTime(900);
			break;

		case 6:
			setCallback(7);
			setup_updateEntity(kCarKronos, kPosition_9270);
			break;

		case 7:
			getEntities()->clearSequences(kEntityKahina);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Kahina, function24)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 && getEntities()->updateEntity(kEntityKahina, (CarIndex)params->param2, (EntityPosition)params->param3)) {
			getEntities()->clearSequences(kEntityKahina);
			params->param1 = 0;
		}
		break;

	case kActionEndSound:
		if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarKronos))
			getSavePoints()->push(kEntityKahina, kEntityKronos, kActionOpenDoor);
		else
			setup_function27();
		break;

	case kActionDefault:
		setCallback(1);
		setup_function6(kTime2241000);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (ENTITY_PARAM(0, 2)) {
				getEntities()->clearSequences(kEntityKahina);
				if (getSoundQueue()->isBuffered(kEntityKahina))
					getSoundQueue()->processEntry(kEntityKahina);

				getProgress().field_44 = 0;

				setup_function22();
			} else if (ENTITY_PARAM(0, 1)) {
				setCallback(2);
				setup_savegame(kSavegameTypeEvent, kEventKahinaGunYellow);
			} else {
				setup_function27();
			}
			break;

		case 2:
			if (getEntityData(kEntityPlayer)->entityPosition >= getData()->entityPosition)
				getAction()->playAnimation(getData()->car < kCarRedSleeping ? kEventKahinaGunYellow : kEventKahinaGunBlue);
			else
				getAction()->playAnimation(kEventKahinaGun);

			getEntities()->updateEntity(kEntityKahina, kCarKronos, kPosition_9270);
			getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + 750));
			getSavePoints()->push(kEntityKahina, kEntityKronos, kAction235599361);
			getSound()->playSound(kEntityKahina, "MUS016", kFlagDefault);
			getProgress().field_44 = 1;

			params->param1 = true;
			params->param2 = kCarKronos;
			params->param3 = kPosition_9270;
			break;
		}
		break;

	case kAction137503360:
		getEntities()->clearSequences(kEntityKahina);
		if (getSoundQueue()->isBuffered(kEntityKahina))
			getSoundQueue()->processEntry(kEntityKahina);

		getProgress().field_44 = 0;

		setup_function22();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Kahina, function25)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 == kTimeInvalid)
			break;

		if (getState()->time <= kTime2263500) {
			if (!getEntities()->isPlayerInCar(kCarGreenSleeping) || !params->param1)
				params->param1 = (uint)getState()->time;

			if (params->param1 >= getState()->time)
				break;
		}

		params->param1 = kTimeInvalid;

		setCallback(12);
		setup_enterExitCompartment("616Ba", kObjectCompartment1);
		break;

	case kActionDefault:
		if (!getEvent(kEventAnnaBaggageArgument)) {
			setCallback(1);
			setup_function19(kCarGreenSleeping, kPosition_8200);
			break;
		}

		switch (getInventory()->get(kItemFirebird)->location) {
		default:
			break;

		case kObjectLocation3:
		case kObjectLocation7:
			if (getInventory()->get(kItemFirebird)->location == kObjectLocation3)
				getProgress().field_7C = 1;
			else
				getProgress().field_80 = 1;

			getScenes()->loadSceneFromItemPosition(kItemFirebird);
			getInventory()->get(kItemFirebird)->location = kObjectLocation5;
			getSavePoints()->push(kEntityKahina, kEntityKronos, kAction138085344);
			break;
		}

		getInventory()->setLocationAndProcess(kItemBriefcase, kObjectLocation2);
		getProgress().field_78 = 1;
		ENTITY_PARAM(0, 3) = 0;

		callbackAction();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 4:
			if (getEntities()->isPlayerInCar(kCarGreenSleeping)) {
				setCallback(getCallback() + 1);
				setup_function19(getCallback() == 1 ? kCarGreenSleeping : kCarKronos, getCallback() == 1 ? kPosition_9460 : kPosition_9270);
				break;
			} else {
				if (getEntities()->checkFields19(kEntityPlayer, kCarGreenSleeping, kPosition_7850) || getEntities()->isOutsideAlexeiWindow()) {
					setCallback(6);
					setup_playSound("LIB013");
				} else {
					setCallback(8);
					setup_enterExitCompartment("616Aa", kObjectCompartment1);
				}
			}
			break;

		case 2:
			setCallback(3);
			setup_updateFromTime(1800);
			break;

		case 3:
			setCallback(4);
			setup_function19(kCarGreenSleeping, kPosition_8200);
			break;

		case 5:
		case 7:
		case 11:
		case 13:
			getEntities()->clearSequences(kEntityKahina);

			callbackAction();
			break;

		case 6:
			setCallback(7);
			setup_function19(kCarKronos, kPosition_9270);
			break;

		case 8:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityKahina);
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorNormal);
			getObjects()->update(kObjectHandleBathroom, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorNormal);

			setCallback(9);
			setup_updateFromTime(900);
			break;

		case 9:
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectHandleBathroom, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			switch (getInventory()->get(kItemFirebird)->location) {
			default:
				if (ENTITY_PARAM(0, 3))
					getInventory()->setLocationAndProcess(kItemBriefcase, kObjectLocation2);
				break;

			case kObjectLocation3:
			case kObjectLocation7:
				if (getInventory()->get(kItemFirebird)->location == kObjectLocation3)
					getProgress().field_7C = 1;
				else
					getProgress().field_80 = 1;

				getScenes()->loadSceneFromItemPosition(kItemFirebird);
				getInventory()->get(kItemFirebird)->location = kObjectLocation5;
				getSavePoints()->push(kEntityKahina, kEntityKronos, kAction138085344);
				getInventory()->setLocationAndProcess(kItemBriefcase, kObjectLocation2);
				getProgress().field_C0 = (uint)getState()->time;
				getProgress().field_78 = 1;
				break;
			}

			getProgress().field_78 = 1;
			ENTITY_PARAM(0, 3) = 0;

			if (getInventory()->get(kItemFirebird)->location != kObjectLocation18) {
				setCallback(10);
				setup_enterExitCompartment("616Ba", kObjectCompartment1);
			}
			break;

		case 10:
		case 12:
			getData()->location = kLocationOutsideCompartment;

			setCallback(getCallback() + 1);
			setup_updateEntity(kCarKronos, kPosition_9270);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Kahina, function26)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (!getEvent(kEventAnnaBaggageArgument)) {
			setCallback(1);
			setup_function19(kCarRedSleeping, kPosition_8200);
			break;
		}

		getScenes()->loadSceneFromItemPosition(kItemFirebird);
		getInventory()->get(kItemFirebird)->location = kObjectLocation5;
		getSavePoints()->push(kEntityKahina, kEntityKronos, kAction138085344);
		getInventory()->setLocationAndProcess(kItemBriefcase, kObjectLocation2);
		getProgress().field_78 = 1;

		callbackAction();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getEntities()->checkFields19(kEntityPlayer, kCarGreenSleeping, kPosition_7850)) {
				setCallback(2);
				setup_function19(kCarRedSleeping, kPosition_9460);
			} else {
				setCallback(6);
				setup_enterExitCompartment("616Aa", kObjectCompartmentA);
			}
			break;

		case 2:
			setCallback(3);
			setup_updateFromTime(1800);
			break;

		case 3:
			setCallback(4);
			setup_function19(kCarRedSleeping, kPosition_8200);
			break;

		case 4:
			if (getEntities()->checkFields19(kEntityPlayer, kCarGreenSleeping, kPosition_7850)) {
				setCallback(5);
				setup_function19(kCarRedSleeping, kPosition_9270);
			} else {
				setCallback(6);
				setup_enterExitCompartment("616Aa", kObjectCompartmentA);
			}
			break;

		case 5:
		case 9:
			getEntities()->clearSequences(kEntityKahina);

			callbackAction();
			break;

		case 6:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityKahina);

			getObjects()->update(kObjectCompartmentA, kEntityPlayer, getObjects()->get(kObjectCompartmentA).location, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject48, kEntityPlayer, getObjects()->get(kObject48).location, kCursorNormal, kCursorNormal);

			setCallback(7);
			setup_updateFromTime(900);
			break;

		case 7:
			getObjects()->update(kObjectCompartmentA, kEntityPlayer, getObjects()->get(kObjectCompartmentA).location, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject48, kEntityPlayer, getObjects()->get(kObject48).location, kCursorHandKnock, kCursorHand);

			if (getInventory()->get(kItemFirebird)->location == kObjectLocation1 || getInventory()->get(kItemFirebird)->location == kObjectLocation2) {
				getScenes()->loadSceneFromItemPosition(kItemFirebird);
				getInventory()->get(kItemFirebird)->location = kObjectLocation5;
				getSavePoints()->push(kEntityKahina, kEntityKronos, kAction138085344);
				ENTITY_PARAM(0, 3) = 1;
			}

			setCallback(8);
			setup_enterExitCompartment("616Ba", kObjectCompartmentA);
			break;

		case 8:
			getData()->location = kLocationOutsideCompartment;

			setCallback(9);
			setup_updateEntity(kCarKronos, kPosition_9270);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Kahina, function27)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInGreenCarEntrance(kEntityPlayer))
			params->param1 = kEventKahinaPunchCar;
		else if (getEntities()->isPlayerInCar(kCarGreenSleeping))
			params->param1 = kEventKahinaPunchBlue;
		else if (getEntities()->isPlayerInCar(kCarRedSleeping))
			params->param1 = kEventKahinaPunchYellow;
		else if (getEntities()->isInSalon(kEntityPlayer))
			params->param1 = kEventKahinaPunchSalon;
		else if (getEntities()->isInRestaurant(kEntityPlayer))
			params->param1 = kEventKahinaPunchRestaurant;
		else if (getEntities()->isInKitchen(kEntityPlayer))
			params->param1 = kEventKahinaPunchKitchen;
		else if (getEntities()->isInBaggageCarEntrance(kEntityPlayer))
			params->param1 = kEventKahinaPunchBaggageCarEntrance;
		else if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarBaggage))
			params->param1 = kEventKahinaPunchBaggageCar;

		if (params->param1) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kSceneGameOverAlarm2);
		}
		break;

	case kActionDefault:
		getState()->timeDelta = 0;
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation((EventIndex)params->param1);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneNone, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Kahina, chapter4)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityKahina);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Kahina, chapter5)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityKahina);
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
