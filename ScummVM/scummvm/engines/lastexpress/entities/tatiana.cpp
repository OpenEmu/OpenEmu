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

#include "lastexpress/entities/tatiana.h"

#include "lastexpress/entities/alexei.h"
#include "lastexpress/entities/coudert.h"

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

Tatiana::Tatiana(LastExpressEngine *engine) : Entity(engine, kEntityTatiana) {
	ADD_CALLBACK_FUNCTION(Tatiana, reset);
	ADD_CALLBACK_FUNCTION(Tatiana, playSound);
	ADD_CALLBACK_FUNCTION(Tatiana, draw);
	ADD_CALLBACK_FUNCTION(Tatiana, updatePosition);
	ADD_CALLBACK_FUNCTION(Tatiana, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(Tatiana, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION(Tatiana, callSavepoint);
	ADD_CALLBACK_FUNCTION(Tatiana, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(Tatiana, updateFromTicks);
	ADD_CALLBACK_FUNCTION(Tatiana, updateFromTime);
	ADD_CALLBACK_FUNCTION(Tatiana, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION(Tatiana, savegame);
	ADD_CALLBACK_FUNCTION(Tatiana, updateEntity);
	ADD_CALLBACK_FUNCTION(Tatiana, function14);
	ADD_CALLBACK_FUNCTION(Tatiana, function15);
	ADD_CALLBACK_FUNCTION(Tatiana, function16);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter1);
	ADD_CALLBACK_FUNCTION(Tatiana, function18);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Tatiana, function20);
	ADD_CALLBACK_FUNCTION(Tatiana, function21);
	ADD_CALLBACK_FUNCTION(Tatiana, function22);
	ADD_CALLBACK_FUNCTION(Tatiana, function23);
	ADD_CALLBACK_FUNCTION(Tatiana, function24);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter2);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter2Handler);
	ADD_CALLBACK_FUNCTION(Tatiana, function27);
	ADD_CALLBACK_FUNCTION(Tatiana, function28);
	ADD_CALLBACK_FUNCTION(Tatiana, function29);
	ADD_CALLBACK_FUNCTION(Tatiana, function30);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter3);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Tatiana, function33);
	ADD_CALLBACK_FUNCTION(Tatiana, function34);
	ADD_CALLBACK_FUNCTION(Tatiana, function35);
	ADD_CALLBACK_FUNCTION(Tatiana, function36);
	ADD_CALLBACK_FUNCTION(Tatiana, function37);
	ADD_CALLBACK_FUNCTION(Tatiana, function38);
	ADD_CALLBACK_FUNCTION(Tatiana, function39);
	ADD_CALLBACK_FUNCTION(Tatiana, function40);
	ADD_CALLBACK_FUNCTION(Tatiana, function41);
	ADD_CALLBACK_FUNCTION(Tatiana, function42);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter4);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Tatiana, function45);
	ADD_CALLBACK_FUNCTION(Tatiana, function46);
	ADD_CALLBACK_FUNCTION(Tatiana, function47);
	ADD_CALLBACK_FUNCTION(Tatiana, function48);
	ADD_CALLBACK_FUNCTION(Tatiana, function49);
	ADD_CALLBACK_FUNCTION(Tatiana, function50);
	ADD_CALLBACK_FUNCTION(Tatiana, function51);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter5);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Tatiana, function54);
	ADD_CALLBACK_FUNCTION(Tatiana, function55);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Tatiana, reset)
	Entity::reset(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Tatiana, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, Tatiana, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SII(4, Tatiana, updatePosition, CarIndex, Position)
	Entity::updatePosition(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(5, Tatiana, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(6, Tatiana, enterExitCompartment2, ObjectIndex)
	Entity::enterExitCompartment(savepoint, kPosition_7500, kPosition_7850, kCarRedSleeping, kObjectCompartmentB);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SIIS(7, Tatiana, callSavepoint, EntityIndex, ActionIndex)
	Entity::callSavepoint(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Tatiana, callbackActionOnDirection)
	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(9, Tatiana, updateFromTicks)
	Entity::updateFromTicks(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(10, Tatiana, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Tatiana, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(12, Tatiana, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(13, Tatiana, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kActionExcuseMeCath) {
		if (getEvent(kEventTatianaAskMatchSpeakRussian) || getEvent(kEventTatianaAskMatch) || getEvent(kEventVassiliSeizure)) {
			getSound()->playSound(kEntityPlayer, rnd(2) ? "CAT1010" : "CAT1010A");
		} else {
			getSound()->excuseMeCath();
		}
		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Tatiana, function14)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityTatiana, kEntityCoudert, kAction326348944);
		getEntities()->drawSequenceLeft(kEntityTatiana, getProgress().chapter == kChapter1 ? "603Fb" : "673Fb");
		getEntities()->enterCompartment(kEntityTatiana, kObjectCompartmentB, true);
		break;

	case kActionCallback:
		if (getCallback() == 1 || getCallback() == 2) {
			getEntities()->exitCompartment(kEntityTatiana, kObjectCompartmentB, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityTatiana);

			callbackAction();
		}
		break;

	case kAction69239528:
		setCallback(getProgress().chapter == kChapter1 ? 1 : 2);
		setup_enterExitCompartment2(getProgress().chapter == kChapter1 ? "603Db" : "673Db", kObjectCompartmentB);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Tatiana, function15)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(getProgress().chapter == kChapter1 ? 1 : 2);
		setup_enterExitCompartment2(getProgress().chapter == kChapter1 ? "603Bb" : "673Bb", kObjectCompartmentB);
		break;

	case kActionCallback:
		if (getCallback() == 1 || getCallback() == 2) {
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityTatiana, kEntityCoudert, kAction292048641);

			getEntities()->drawSequenceLeft(kEntityTatiana, getProgress().chapter == kChapter1 ? "603Fb" : "673Fb");
			getEntities()->enterCompartment(kEntityTatiana, kObjectCompartmentB, true);
		}
		break;

	case kAction69239528:
		getEntities()->exitCompartment(kEntityTatiana, kObjectCompartmentB, true);
		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(16, Tatiana, function16, uint32)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 < getState()->time && !params->param4) {
			params->param4 = 1;

			getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject49, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;
		}

		if (params->param2) {
			if (!Entity::updateParameter(params->param5, getState()->timeTicks, 75))
				break;

			params->param2 = 0;
			params->param3 = 1;

			getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param5 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (params->param2) {
			getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);

			if (savepoint.param.intValue == 49) {
				setCallback(4);
				setup_playSound(getSound()->justAMinuteCath());
				break;
			}

			if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(5);
				setup_playSound(rnd(2) ? "CAT1512" : getSound()->wrongDoorCath());
				break;
			}

			setCallback(6);
			setup_playSound(getSound()->wrongDoorCath());
		} else {
			getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);

			setCallback(savepoint.action == kActionKnock ? 1 : 2);
			setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (params->param2 || params->param3) {
			getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);

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
			setup_playSound(rnd(2) ? "TAT1133A" : "TAT1133B");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorTalk, kCursorNormal);
			params->param2 = 1;
			break;

		case 4:
		case 5:
		case 6:
			params->param2 = 0;
			params->param3 = 1;
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Tatiana, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Tatiana, setup_chapter1Handler));
		break;

	case kActionDefault:
		getSavePoints()->addData(kEntityTatiana, kAction191198209, 0);

		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject41, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		getData()->entityPosition = kPosition_5419;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Tatiana, function18)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1) {

			if (getState()->time > kTime1143000 && !params->param2) {
				params->param2 = 1;
				getEntities()->drawSequenceRight(kEntityTatiana, "806DS");
				params->param1 = 1;
			}

			if (!params->param1) {
				if (Entity::updateParameter(params->param3, getState()->time, 4500)) {
					getEntities()->drawSequenceRight(kEntityTatiana, "806DS");
					params->param1 = 1;
				}
			}
		}

		if (getData()->entityPosition <= kPosition_2330) {
			getSavePoints()->push(kEntityTatiana, kEntityAlexei, kAction157159392);
			getEntities()->clearSequences(kEntityTatiana);

			callbackAction();
		}
		break;

	case kActionExitCompartment:
		getSavePoints()->push(kEntityTatiana, kEntityAlexei, kAction188784532);

		callbackAction();
		break;

	case kActionDefault:
		if (getEntities()->isInSalon(kEntityPlayer)) {
			getEntities()->drawSequenceRight(kEntityTatiana, "806DS");
			params->param1 = 1;
		} else {
			getEntities()->clearSequences(kEntityTatiana);
		}
		break;

	case kActionDrawScene:
		if (!params->param1 && getEntities()->isInSalon(kEntityPlayer)) {
			getEntities()->drawSequenceRight(kEntityTatiana, "806DS");
			getEntities()->updateFrame(kEntityTatiana);
			params->param1 = 1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Tatiana, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getSoundQueue()->isBuffered(kEntityTatiana) || !params->param4 || params->param3 == 2 || getSoundQueue()->isBuffered("TAT1066"))
			goto label_tatiana_chapter1_2;

		if (Entity::updateParameter(params->param5, getState()->timeTicks, 450)) {
			getSound()->playSound(kEntityTatiana, params->param3 ? "TAT1069B" : "TAT1069A");
			getProgress().field_64 = 1;
			params->param3++;
			params->param5 = 0;
		}

		if (getEntities()->isPlayerPosition(kCarRestaurant, 71)) {
			if (Entity::updateParameter(params->param6, getState()->timeTicks, 75)) {
				getSound()->playSound(kEntityTatiana, params->param3 ? "TAT1069B" : "TAT1069A");
				getProgress().field_64 = 1;
				params->param3++;
				params->param6 = 0;
			}
		}

label_tatiana_chapter1_2:
		Entity::timeCheckSavepoint(kTime1084500, params->param7, kEntityTatiana, kEntityPascale, kAction257489762);

		if (params->param1) {
			if (!Entity::updateParameter(params->param8, getState()->timeTicks, 90))
				break;

			getScenes()->loadSceneFromPosition(kCarRestaurant, 65);
		} else {
			params->param8 = 0;
		}
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityTatiana, kEntityTables4, kAction136455232);
		getEntities()->drawSequenceLeft(kEntityTatiana, "014A");
		break;

	case kActionDrawScene:
		params->param1 = getEntities()->isPlayerPosition(kCarRestaurant, 67) ? 1 : 0;
		params->param4 = getEntities()->isPlayerPosition(kCarRestaurant, 69)
		              || getEntities()->isPlayerPosition(kCarRestaurant, 70)
		              || getEntities()->isPlayerPosition(kCarRestaurant, 71);
		break;

	case kAction122288808:
		getEntities()->drawSequenceLeft(kEntityTatiana, "014A");
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityTatiana, "BLANK");
		break;

	case kAction124973510:
		setup_function20();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Tatiana, function20)
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
			getSavePoints()->push(kEntityTatiana, kEntityAugust, kAction223183000);
			getEntities()->updatePositionEnter(kEntityTatiana, kCarRestaurant, 67);
			getSound()->playSound(kEntityTatiana, "TAT1070");

			setCallback(2);
			setup_callSavepoint("014C", kEntityTables4, kActionDrawTablesWithChairs, "014D");
			break;

		case 2:
			getEntities()->updatePositionExit(kEntityTatiana, kCarRestaurant, 67);
			getSavePoints()->push(kEntityTatiana, kEntityServers0, kAction188893625);

			setCallback(3);
			setup_function18();
			break;

		case 3:
			getSavePoints()->push(kEntityTatiana, kEntityAugust, kAction268620864);
			setup_function21();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Tatiana, function21)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->clothes = kClothes1;

		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_8513);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->clothes = kClothesDefault;

			getSound()->playSound(kEntityTatiana, "TAT1071");
			getEntities()->drawSequenceRight(kEntityTatiana, "604Aa");
			getEntities()->enterCompartment(kEntityTatiana, kObjectCompartmentA);

			getData()->location = kLocationInsideCompartment;

			if (getEntities()->checkFields19(kEntityPlayer, kCarRedSleeping, kPosition_7850)) {
				getAction()->playAnimation(isNight() ? kEventCathTurningNight : kEventCathTurningDay);
				getSound()->playSound(kEntityPlayer, "BUMP");
				getScenes()->loadSceneFromObject(kObjectCompartmentA, true);
			}

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			getEntities()->exitCompartment(kEntityTatiana, kObjectCompartmentA);

			getData()->location = kLocationInsideCompartment;

			getEntities()->clearSequences(kEntityTatiana);
			getSavePoints()->push(kEntityTatiana, kEntityAlexei, kAction135854208);
			getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);
			// Fallback to next case

		case 3:
			if (getSoundQueue()->isBuffered(kEntityTatiana)) {
				setCallback(3);
				setup_updateFromTime(75);
			} else {
				setCallback(4);
				setup_playSound("TAT1071A");
			}
			break;

		case 4:
			getData()->entityPosition = kPosition_7500;

			getSavePoints()->push(kEntityTatiana, kEntityVassili, kAction168459827);

			setCallback(5);
			setup_function16(kTime1156500);
			break;

		case 5:
		case 6:
			if (getProgress().field_14 == 29) {
				setCallback(6);
				setup_function16((uint)getState()->time + 900);
			} else {
				getObjects()->update(kObject49, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

				setup_function22();
			}
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Tatiana, function22)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 == kTimeInvalid || getState()->time <= kTime1179000)
			goto label_update;

		if (Entity::updateParameterTime(kTime1233000, ((!getEvent(kEventTatianaAskMatchSpeakRussian) && !getEvent(kEventTatianaAskMatch)) || getEntities()->isInGreenCarEntrance(kEntityPlayer)), params->param1, 0)) {
label_update:
			if (!getEvent(kEventTatianaAskMatchSpeakRussian)
			 && !getEvent(kEventTatianaAskMatch)
			 && getInventory()->hasItem(kItemMatchBox)
			 && getEntities()->isInGreenCarEntrance(kEntityPlayer)) {
				getObjects()->update(kObject25, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorForward);
				getObjects()->update(kObjectTrainTimeTable, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorForward);
			}
		}

		params->param1 = kTimeInvalid;

		getObjects()->update(kObject25, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		getObjects()->update(kObjectTrainTimeTable, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		getEntities()->updatePositionExit(kEntityTatiana, kCarGreenSleeping, 70);
		getEntities()->updatePositionExit(kEntityTatiana, kCarGreenSleeping, 71);

		if (getEntities()->isInGreenCarEntrance(kEntityPlayer)) {
			getSound()->excuseMe(kEntityTatiana);

			if (getEntities()->isPlayerPosition(kCarGreenSleeping, 62))
				getScenes()->loadSceneFromPosition(kCarGreenSleeping, 72);
		}

		getData()->inventoryItem = kItemNone;

		setup_function23();
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;

		setCallback(4);
		setup_savegame(kSavegameTypeEvent, kEventTatianaGivePoem);
		break;

	case kActionOpenDoor:
		setCallback(3);
		setup_savegame(kSavegameTypeEvent, kEventTatianaAskMatchSpeakRussian);
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityTatiana, kEntityVassili, kAction122732000);

		setCallback(1);
		setup_function15();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_540);
			break;

		case 2:
			if (getEntities()->isInGreenCarEntrance(kEntityPlayer)) {
				getSound()->excuseMe(kEntityTatiana);

				if (getEntities()->isPlayerPosition(kCarGreenSleeping, 62))
					getScenes()->loadSceneFromPosition(kCarGreenSleeping, 72);
			}

			getEntities()->drawSequenceLeft(kEntityTatiana, "306B");
			getEntities()->updatePositionEnter(kEntityTatiana, kCarGreenSleeping, 70);
			getEntities()->updatePositionEnter(kEntityTatiana, kCarGreenSleeping, 71);
			break;

		case 3:
			getAction()->playAnimation(getEvent(kEventAlexeiSalonVassili) ? kEventTatianaAskMatchSpeakRussian : kEventTatianaAskMatch);
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 62);
			getData()->inventoryItem = kItemParchemin;

			getObjects()->update(kObject25, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			getObjects()->update(kObjectTrainTimeTable, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			break;

		case 4:
			getAction()->playAnimation(kEventTatianaGivePoem);
			getInventory()->removeItem(kItemParchemin);
			getScenes()->processScene();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Tatiana, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_7500);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function14();
			break;

		case 2:
			setup_function24();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Tatiana, function24)
	if (savepoint.action == kActionDefault) {

		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObject25, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		getObjects()->update(kObjectTrainTimeTable, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		getEntities()->updatePositionExit(kEntityTatiana, kCarGreenSleeping, 70);
		getEntities()->updatePositionExit(kEntityTatiana, kCarGreenSleeping, 71);
		getEntities()->clearSequences(kEntityTatiana);

		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject41, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Tatiana, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter2Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityTatiana);

		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject41, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		getData()->entityPosition = kPosition_5420;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes2;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Tatiana, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime1800000 && params->param1 && getEntities()->isSomebodyInsideRestaurantOrSalon()) {
			getData()->inventoryItem = kItemNone;
			setup_function28();
		}
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;
		setup_function28();
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityTatiana, "024A");
		getSavePoints()->push(kEntityTatiana, kEntityTables5, kAction136455232);
		getData()->inventoryItem = kItemInvalid;
		break;

	case kActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 64) || getEntities()->isPlayerPosition(kCarRestaurant, 65)) {
			getData()->inventoryItem = kItemNone;
			setup_function27();
		}
		break;

	case kAction290869168:
		params->param1 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Tatiana, function27)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(getEvent(kEventTatianaGivePoem) ? 1 : 2);
		setup_savegame(kSavegameTypeEvent, getEvent(kEventTatianaGivePoem) ? kEventTatianaBreakfastAlexei : kEventTatianaBreakfast);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			RESET_ENTITY_STATE(kEntityAlexei, Alexei, setup_function30);
			getAction()->playAnimation(kEventTatianaBreakfastAlexei);
			getInventory()->addItem(kItemParchemin);
			getInventory()->setLocationAndProcess(kItem11, kObjectLocation1);
			setup_function28();
			break;

		case 2:
			RESET_ENTITY_STATE(kEntityAlexei, Alexei, setup_function30);
			getAction()->playAnimation(kEventTatianaBreakfast);
			if (getInventory()->hasItem(kItemParchemin)) {
				getAction()->playAnimation(kEventTatianaBreakfastGivePoem);
				getInventory()->removeItem(kItemParchemin);
			} else {
				getAction()->playAnimation(kEventTatianaAlexei);
			}
			setup_function28();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Tatiana, function28)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->inventoryItem = kItemNone;
		getData()->location = kLocationOutsideCompartment;

		getSavePoints()->push(kEntityTatiana, kEntityTables5, kActionDrawTablesWithChairs, "024D");
		getSavePoints()->push(kEntityTatiana, kEntityAlexei, kAction236053296, (getEvent(kEventTatianaBreakfastAlexei) || getEvent(kEventTatianaBreakfast)) ? 69 : 0);
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function29();
		break;

	case kAction123857088:
		getEntities()->drawSequenceLeft(kEntityTatiana, "018G");

		setCallback(1);
		setup_updateFromTime(1800);
		break;

	case kAction156444784:
		getData()->location = kLocationInsideCompartment;
		getEntities()->drawSequenceLeft(kEntityTatiana, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Tatiana, function29)
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
			getEntities()->updatePositionEnter(kEntityTatiana, kCarRestaurant, 63);

			setCallback(2);
			setup_callSavepoint("018H", kEntityTables1, kActionDrawTablesWithChairs, "018A");
			break;

		case 2:
			getEntities()->updatePositionExit(kEntityTatiana, kCarRestaurant, 63);
			getSavePoints()->push(kEntityTatiana, kEntityServers1, kAction302203328);
			getEntities()->drawSequenceRight(kEntityTatiana, "805DS");

			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityTatiana);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			setup_function30();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Tatiana, function30)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_7500);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function14();
			break;

		case 2:
			setCallback(3);
			setup_function16(kTimeEnd);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Tatiana, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityTatiana);

		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_1750;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes2;
		getData()->inventoryItem = kItemNone;

		// Update inventory
		getInventory()->get(kItemFirebird)->location = kObjectLocation2;

		if (getEvent(kEventTatianaBreakfastGivePoem) || (getEvent(kEventTatianaGivePoem) && !getEvent(kEventTatianaBreakfastAlexei)))
			getInventory()->get(kItemParchemin)->location = kObjectLocation2;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Tatiana, chapter3Handler)
	EntityData::EntityParametersI5S  *parameters = (EntityData::EntityParametersI5S*)_data->getCurrentParameters();
	EntityData::EntityParametersSIII *parameters1 = (EntityData::EntityParametersSIII*)_data->getCurrentParameters(1);

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!parameters->param2 && !parameters->param5) {
			parameters->param1 -= getState()->timeDelta;

			if (getState()->timeDelta > parameters->param1) {

				getEntities()->drawSequenceLeft(kEntityTatiana, (char *)&parameters1->seq);
				getSound()->playSound(kEntityTatiana, (char *)&parameters->seq);

				if (parameters->param3 == 4 && getEntities()->isInSalon(kEntityPlayer))
					getProgress().field_90 = 1;

				parameters->param2 = 1;
			}
		}

		if (parameters->param4 && parameters->param5) {
			if (Entity::updateParameterCheck(parameters->param4, getState()->time, 6300)) {
				if (getEntities()->isSomebodyInsideRestaurantOrSalon()) {
					getData()->location = kLocationOutsideCompartment;

					setCallback(1);
					setup_updatePosition("110E", kCarRestaurant, 52);
				}
			}
		}
		break;

	case  kActionEndSound:
		parameters->param2 = 0;
		++parameters->param3;

		switch (parameters->param3) {
		default:
			parameters->param5 = 1;
			break;

		case 1:
			parameters->param1 = 900;
			getEntities()->drawSequenceLeft(kEntityTatiana, "110A");
			strcpy((char *)&parameters->seq, "Tat3160B");
			strcpy((char *)&parameters1->seq, "110A");
			break;

		case 2:
			parameters->param1 = 9000;
			strcpy((char *)&parameters->seq, "Tat3160C");
			strcpy((char *)&parameters1->seq, "110C");
			break;

		case 3:
			parameters->param1 = 13500;
			getEntities()->drawSequenceLeft(kEntityTatiana, "110B");
			strcpy((char *)&parameters->seq, "Tat3160D");
			strcpy((char *)&parameters1->seq, "110D");
			break;

		case 4:
			parameters->param1 = 9000;
			getEntities()->drawSequenceLeft(kEntityTatiana, "110B");
			strcpy((char *)&parameters->seq, "Tat3160E");
			strcpy((char *)&parameters1->seq, "110D");
			break;

		case 5:
			parameters->param1 = 4500;
			getEntities()->drawSequenceLeft(kEntityTatiana, "110B");
			strcpy((char *)&parameters->seq, "Tat3160G");
			strcpy((char *)&parameters1->seq, "110D");
			break;

		case 6:
			parameters->param1 = 4500;
			getEntities()->drawSequenceLeft(kEntityTatiana, "110B");
			strcpy((char *)&parameters->seq, "Tat3160B");
			break;
		}
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityTatiana, kEntityAlexei, kAction122358304);
		getSavePoints()->push(kEntityTatiana, kEntityKronos, kAction157159392);
		getEntities()->drawSequenceLeft(kEntityTatiana, "110C");
		getSound()->playSound(kEntityTatiana, "Tat3160A");

		parameters->param2 = 1;
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getSavePoints()->push(kEntityTatiana, kEntityAlexei, kAction122288808);
			setup_function33();
		}
		break;

	case kAction101169422:
		parameters->param4 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Tatiana, function33)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityTatiana);
		setCallback(1);
		setup_updateFromTime(75);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_7500);
			break;

		case 2:
			setCallback(3);
			setup_function14();
			break;

		case 3:
			setup_function34();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(34, Tatiana, function34)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function16(kTime2097000);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getInventory()->get(kItemFirebird)->location = kObjectLocation1;
			if (getEntities()->checkFields19(kEntityPlayer, kCarRedSleeping, kPosition_7850))
				getScenes()->loadSceneFromObject(kObjectCompartmentB);

			getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject49, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			setup_function15();
			break;

		case 2:
			setCallback(3);
			setup_updateEntity(kCarKronos, kPosition_9270);
			break;

		case 3:
			setup_function35();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(35, Tatiana, function35)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1
		 && getInventory()->hasItem(kItemFirebird)
		 && getEntities()->checkFields19(kEntityPlayer, kCarRedSleeping, kPosition_7850)
		 && (getState()->time < kTime2133000 || getProgress().field_40)) {
			setCallback(1);
			setup_function41();
			break;
		}

label_callback_1:
		if (getState()->time > kTime2133000) {
			if (getData()->car >= kCarRedSleeping || (getData()->car == kCarGreenSleeping && getData()->entityPosition > kPosition_5790))
				setup_function36();
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getEntities()->clearSequences(kEntityTatiana);

		getData()->car = kCarKronos;
		getData()->entityPosition = kPosition_6000;
		getData()->location = kLocationInsideCompartment;
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			params->param1 = 1;
			goto label_callback_1;
		}
		break;

	case kAction191668032:
		setup_function36();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(36, Tatiana, function36)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->car = kCarGreenSleeping;
		getData()->entityPosition = kPosition_850;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_7500);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (!getEntities()->checkFields19(kEntityPlayer, kCarGreenSleeping, kPosition_7850) || getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_8200)) {
				setCallback(2);
				setup_function14();
				break;
			}

			if (getInventory()->hasItem(kItemFirebird)) {
				getAction()->playAnimation(kEventTatianaCompartmentStealEgg);
				getInventory()->removeItem(kItemFirebird);
				getInventory()->get(kItemFirebird)->location = kObjectLocation2;
			} else {
				getAction()->playAnimation(kEventTatianaCompartment);
			}

			getScenes()->loadSceneFromObject(kObjectCompartmentB);
			break;

		case 2:
			setup_function37();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(37, Tatiana, function37)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getInventory()->get(kItemFirebird)->location != kObjectLocation1 && getInventory()->get(kItemFirebird)->location != kObjectLocation2) {
			if(!params->param3)
				params->param3 = (uint)getState()->time + 900;

			if (params->param4 != kTimeInvalid && params->param3 < getState()->time) {
				if (Entity::updateParameterTime(kTime2227500, !getEntities()->isPlayerInCar(kCarRedSleeping), params->param4, 450)) {
					getProgress().field_5C = 1;
					if (getEntities()->isInsideCompartment(kEntityAnna, kCarRedSleeping, kPosition_4070)) {
						setup_function38();
						break;
					}
				}
			}
		}

		if (params->param1) {
			if (!Entity::updateParameter(params->param5, getState()->timeTicks, 75))
				break;

			getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);

			params->param1 = 0;
			params->param2 = 1;
		}

		params->param5 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (params->param1) {
			getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);

			if (savepoint.param.intValue == 49) {
				setCallback(4);
				setup_playSound(getSound()->justAMinuteCath());
				break;
			}

			if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(5);
				setup_playSound(rnd(2) ? "CAT1512" : getSound()->wrongDoorCath());
				break;
			}

			setCallback(6);
			setup_playSound(getSound()->wrongDoorCath());
			break;
		}

		if (savepoint.param.intValue == 49) {

			if (getInventory()->hasItem(kItemFirebird)) {
				getAction()->playAnimation(kEventTatianaCompartmentStealEgg);
				getInventory()->removeItem(kItemFirebird);
				getInventory()->get(kItemFirebird)->location = kObjectLocation2;
			} else {
				getAction()->playAnimation(kEventTatianaCompartment);
			}

			getScenes()->loadSceneFromObject(kObjectCompartmentB);
			break;
		}

		getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);

		setCallback(savepoint.action == kActionKnock ? 1 : 2);
		setup_playSound(savepoint.action == kActionKnock ?  "LIB012" : "LIB013");
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorHand);

		getData()->location = kLocationInsideCompartment;
		getEntities()->clearSequences(kEntityTatiana);
		break;

	case kActionDrawScene:
		if (params->param1 || params->param2) {
			getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorNormal, kCursorHand);

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
			setCallback(3);
			setup_playSound(rnd(2) ? "TAT1133A" : "TAT1133B");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentB, kEntityTatiana, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject49, kEntityTatiana, kObjectLocation1, kCursorTalk, kCursorNormal);
			params->param1 = 1;
			break;

		case 4:
		case 5:
		case 6:
			params->param1 = 0;
			params->param2 = 1;
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(38, Tatiana, function38)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param1, getState()->time, 450))
			break;

		getEntities()->exitCompartment(kEntityTatiana, kObjectCompartmentF, true);

		setCallback(4);
		setup_function42(kCarRedSleeping, kPosition_7500);
		break;

	case kActionDefault:
		getData()->clothes = kClothes3;

		setCallback(1);
		setup_enterExitCompartment("673Jb", kObjectCompartmentB);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject49, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			setCallback(2);
			setup_function42(kCarRedSleeping, kPosition_4070);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityTatiana, "673Gf");
			getEntities()->enterCompartment(kEntityTatiana, kObjectCompartmentF, true);

			setCallback(3);
			setup_playSound("Tat3164");
			break;

		case 3:
			getSavePoints()->push(kEntityTatiana, kEntityAnna, kAction236241630);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment2("673Db", kObjectCompartmentB);
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityTatiana);

			setup_function39();
			break;

		case 6:
			getEntities()->exitCompartment(kEntityTatiana, kObjectCompartmentF, true);
			getEntities()->clearSequences(kEntityTatiana);
			getData()->location = kLocationInsideCompartment;

			setCallback(7);
			setup_playSound("ANN3011");
			break;

		case 7:
			setCallback(8);
			setup_updateFromTime(900);
			break;

		case 8:
			setCallback(9);
			setup_enterExitCompartment("673Jf", kObjectCompartmentF);
			break;

		case 9:
			getData()->location = kLocationOutsideCompartment;

			setCallback(10);
			setup_function42(kCarRedSleeping, kPosition_7500);
			break;

		case 10:
			getSavePoints()->push(kEntityTatiana, kEntityAnna, kAction236517970);

			setCallback(11);
			setup_enterExitCompartment2("673Db", kObjectCompartmentB);
			break;

		case 11:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityTatiana);

			setup_function39();
			break;
		}
		break;

	case kAction100906246:
		setCallback(6);
		setup_enterExitCompartment("673Df", kObjectCompartmentF);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(39, Tatiana, function39)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1 && getEntities()->isDistanceBetweenEntities(kEntityTatiana, kEntityPlayer, 1000)) {
			params->param1 = 1;
			getSound()->playSound(kEntityTatiana, "Tat3164");	// Tatiana weeping
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(40, Tatiana, function40)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarKronos)
		 || getData()->car != getEntityData(kEntityPlayer)->car
		 || getEntities()->updateEntity(kEntityTatiana, kCarKronos, kPosition_9270))
			callbackAction();
		break;

	case kActionExcuseMe:
		if (getEvent(kEventTatianaAskMatchSpeakRussian) || getEvent(kEventTatianaAskMatch) || getEvent(kEventVassiliSeizure))
			getSound()->playSound(kEntityPlayer, rnd(2) ? "CAT1001A" : "CAT1010");
		else
			getSound()->excuseMeCath();
		break;

	case kActionDefault:
		if (getEntities()->updateEntity(kEntityTatiana, kCarKronos, kPosition_9270))
			callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(41, Tatiana, function41)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1)
			break;

		if (getEntities()->checkFields19(kEntityPlayer, kCarRedSleeping, kPosition_7850)
		 && !getEvent(kEventVassiliCompartmentStealEgg)
		 && (getState()->time <= kTime2133000 || getProgress().field_40)) {
			if (getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_7500)) {

				getSavePoints()->push(kEntityTatiana, kEntityCoudert, kAction235061888);
				getEntities()->clearSequences(kEntityTatiana);
				getEntities()->exitCompartment(kEntityTatiana, kObjectCompartmentB, true);
				getData()->location = kLocationInsideCompartment;

				if (getInventory()->hasItem(kItemFirebird)) {
					getAction()->playAnimation(kEventTatianaCompartmentStealEgg);
					getInventory()->removeItem(kItemFirebird);
					getInventory()->get(kItemFirebird)->location = kObjectLocation2;
				} else {
					getAction()->playAnimation(kEventTatianaCompartment);
				}

				getScenes()->loadSceneFromObject(kObjectCompartmentB);

				setCallback(4);
				setup_updateFromTime(150);
			}
		} else {
			getEntities()->exitCompartment(kEntityTatiana, kObjectCompartmentB, true);

			if (getState()->time < kTime2133000 || getProgress().field_40) {
				setCallback(3);
				setup_function40();
				break;
			}

			getEntities()->clearSequences(kEntityTatiana);
			callbackAction();
		}
		break;

	case kActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationOutsideCompartment;

		RESET_ENTITY_STATE(kEntityCoudert, Coudert, setup_function51);

		getEntities()->drawSequenceLeft(kEntityTatiana, "673Fb");
		getEntities()->enterCompartment(kEntityTatiana, kObjectCompartmentB, true);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_playSound("Tat3161B");
			break;

		case 2:
			getSavePoints()->push(kEntityTatiana, kEntityCoudert, kAction168316032);
			params->param1 = 1;
			break;

		case 3:
		case 6:
			getEntities()->clearSequences(kEntityTatiana);

			callbackAction();
			break;

		case 4:
			setCallback(5);
			setup_function15();
			break;

		case 5:
			setCallback(6);
			setup_function40();
			break;
		}
		break;

	case kAction154071333:
		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);

		setCallback(1);
		setup_savegame(kSavegameTypeTime, kTimeNone);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(42, Tatiana, function42, CarIndex, EntityPosition)
	if (savepoint.action == kActionExcuseMeCath || savepoint.action == kActionExcuseMe) {
		getSound()->playSound(kEntityPlayer, "Tat3124", getSound()->getSoundFlag(kEntityTatiana));
		return;
	}

	Entity::updateEntity(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(43, Tatiana, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityTatiana);

		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothes2;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 1) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(44, Tatiana, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function16(kTime2362500);
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function45();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(45, Tatiana, function45)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_enterExitCompartment("673Bb", kObjectCompartmentB);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getData()->location = kLocationOutsideCompartment;

			setCallback(1);
			setup_updateEntity(kCarGreenSleeping, kPosition_540);
			break;

		case 2:
			if (getEntities()->isInGreenCarEntrance(kEntityPlayer)) {
				getSound()->excuseMe(kEntityTatiana);

				if (getEntities()->isPlayerPosition(kCarGreenSleeping, 62))
					getScenes()->loadSceneFromPosition(kCarGreenSleeping, 72);
			}

			getSavePoints()->push(kEntityTatiana, kEntityAlexei, kAction123712592);
			setup_function46();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(46, Tatiana, function46)
	// Expose parameters as IIIIIS and ignore the default exposed parameters
	EntityData::EntityParametersI5S  *parameters  = (EntityData::EntityParametersI5S*)_data->getCurrentParameters();

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!parameters->param2 && !parameters->param3) {
			parameters->param1 -= getState()->timeDelta;

			if (parameters->param1 < getState()->timeDelta) {
				getSound()->playSound(kEntityTatiana, (char *)&parameters->seq);

				if (getEntities()->isDistanceBetweenEntities(kEntityTatiana, kEntityPlayer, 2000)) {
					if (parameters->param4 == 4)
						getProgress().field_8C = 1;
					else if (parameters->param4 == 7)
						getProgress().field_88 = 1;
				}

				parameters->param2 = 1;
			}
		}

		if (CURRENT_PARAM(1, 1) == kTimeInvalid || getState()->time <= kTime2394000)
			break;

		if (getState()->time >= kTime2398500) {
			CURRENT_PARAM(1, 1) = kTimeInvalid;
		} else {
			if (getEntities()->isInGreenCarEntrance(kEntityPlayer) || !CURRENT_PARAM(1, 1))
				CURRENT_PARAM(1, 1) = (uint)getState()->time;

			if (CURRENT_PARAM(1, 1) >= getState()->time)
				break;

			CURRENT_PARAM(1, 1) = kTimeInvalid;
		}

		if (getEntities()->isInGreenCarEntrance(kEntityPlayer)) {
			getSound()->excuseMe(kEntityTatiana);

			if (getEntities()->isPlayerPosition(kCarGreenSleeping, 62))
				getScenes()->loadSceneFromPosition(kCarGreenSleeping, 72);
		}

		getSavePoints()->push(kEntityTatiana, kEntityAlexei, kAction123536024);

		setup_function47();
		break;

	case kActionEndSound:
		parameters->param2 = 0;
		++parameters->param4;

		switch(parameters->param4) {
		default:
			parameters->param1 = 162000;
			break;

		case 1:
			parameters->param1 = 900;
			strcpy((char *)&parameters->seq, "Tat4165F");
			break;

		case 2:
			parameters->param1 = 900;
			strcpy((char *)&parameters->seq, "Tat4165B");
			break;

		case 3:
			parameters->param1 = 1800;
			strcpy((char *)&parameters->seq, "Tat4165G");
			break;

		case 4:
			parameters->param1 = 900;
			strcpy((char *)&parameters->seq, "Tat4165H");
			break;

		case 5:
			parameters->param1 = 2700;
			strcpy((char *)&parameters->seq, "Tat4165C");
			break;

		case 6:
			parameters->param1 = 900;
			strcpy((char *)&parameters->seq, "Tat4165D");
			break;

		case 7:
			parameters->param1 = 900;
			strcpy((char *)&parameters->seq, "Tat4165E");
			break;
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityTatiana, "306E");
		parameters->param1 = 450;
		strcpy((char *)&parameters->seq, "Tat4165A");
		break;

	case kActionDrawScene:
		if (getEntities()->isInGreenCarEntrance(kEntityPlayer)) {
			parameters->param3 = 1;

			if (parameters->param2) {
				getSoundQueue()->removeFromQueue(kEntityTatiana);
				getSavePoints()->call(kEntityTatiana, kEntityTatiana, kActionEndSound);
			}
		} else {
			parameters->param3 = 0;
			parameters->param5 = 0;
		}

		if (getEntities()->isPlayerPosition(kCarGreenSleeping, 62) && !parameters->param5) {
			setCallback(1);
			setup_draw("306D");
		}
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getEntities()->drawSequenceLeft(kEntityTatiana, "306E");
			parameters->param5 = 1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(47, Tatiana, function47)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_7500);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment2("673Db", kObjectCompartmentB);
			break;

		case 2:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityTatiana);

			setCallback(3);
			setup_function16(kTime2407500);
			break;

		case 3:
		case 4:
			if (ENTITY_PARAM(0, 1) && getObjects()->get(kObjectCompartment1).location2 == kObjectLocation1) {
				setup_function48();
			} else {
				setCallback(4);
				setup_function16(900);
			}
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(48, Tatiana, function48)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1) {
			if (!getEvent(kEventTatianaTylerCompartment) && getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_8200)) {
				params->param1 = 1;
				getProgress().field_E4 = 1;
				getObjects()->update(kObjectCompartment1, kEntityTatiana, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorHand);
			}

			if (!params->param1)
				goto label_end;
		}

		if (!getEntities()->checkFields19(kEntityPlayer, kCarGreenSleeping, kPosition_7850)) {
			getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorHandKnock, kCursorHand);
			params->param1 = 0;
		}

		if (!params->param1 || getSoundQueue()->isBuffered(kEntityTatiana))
			goto label_end;

		if (!Entity::updateParameter(params->param2, getState()->timeTicks, 5 * (3 * rnd(5) + 30)))
			goto label_end;

		getSound()->playSound(kEntityTatiana, "LIB012", kFlagDefault);
		params->param2 = 0;

label_end:
		if (getEvent(kEventTatianaTylerCompartment) || getState()->time > kTime2475000) {
			if (params->param1)
				getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorHandKnock, kCursorHand);

			getProgress().field_E4 = 0;
			getEntities()->exitCompartment(kEntityTatiana, kObjectCompartment2, true);

			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_7500);
		}
		break;

	case kActionOpenDoor:
		params->param1 = 0;

		getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		setCallback(5);
		setup_savegame(kSavegameTypeEvent, kEventTatianaTylerCompartment);
		break;

	case kActionDefault:
		setCallback(1);
		setup_enterExitCompartment("673Bb", kObjectCompartmentB);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_7500);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityTatiana, "673Fb");
			getEntities()->enterCompartment(kEntityTatiana, kObjectCompartment2, true);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment2("673Db", kObjectCompartmentB);
			break;

		case 4:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityTatiana);

			setup_function49();
			break;

		case 5:
			getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			getAction()->playAnimation(kEventTatianaTylerCompartment);
			getSound()->playSound(kEntityPlayer, "LIB015");
			getScenes()->loadScene(kScene41);
			break;

		case 6:
			setCallback(7);
			setup_updateEntity(kCarGreenSleeping, kPosition_7500);
			break;

		case 7:
			getEntities()->drawSequenceLeft(kEntityTatiana, "673Fb");
			getEntities()->enterCompartment(kEntityTatiana, kObjectCompartment2, true);
			break;
		}
		break;

	case kAction238790488:
		params->param1 = 0;

		getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorHandKnock, kCursorHand);
		getEntities()->exitCompartment(kEntityTatiana, kObjectCompartment2, true);
		getEntities()->clearSequences(kEntityTatiana);

		getData()->car = kCarGreenSleeping;
		getData()->entityPosition = kPosition_9460;

		setCallback(6);
		setup_updateFromTime(1800);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(49, Tatiana, function49)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kAction169360385:
		setup_function50();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(50, Tatiana, function50)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime2520000 && !params->param1) {
			params->param1 = 1;
			setup_function51();
		}
		break;

	case kActionEndSound:
		getSound()->playSound(kEntityTatiana, "Tat4166");
		break;

	case kActionKnock:
		if (!getSoundQueue()->isBuffered("LIB012", true))
			getSound()->playSound(kEntityPlayer, "LIB012");
		break;

	case kActionOpenDoor:
		getSound()->playSound(kEntityPlayer, "LIB014");

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventVassiliDeadAlexei);
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject48, kEntityTatiana, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartmentA, kEntityTatiana, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		if (!getSoundQueue()->isBuffered(kEntityTatiana))
			getSound()->playSound(kEntityTatiana, "Tat4166");
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			if (getSoundQueue()->isBuffered("MUS013"))
				getSoundQueue()->processEntry("MUS013");

			getAction()->playAnimation(kEventVassiliDeadAlexei);
			getSavePoints()->push(kEntityTatiana, kEntityAbbot, kAction104060776);
			getScenes()->loadSceneFromPosition(kCarRedSleeping, 38);

			setup_function51();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(51, Tatiana, function51)
	if (savepoint.action == kActionDefault) {
		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject48, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(52, Tatiana, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityTatiana);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(53, Tatiana, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_function54();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(54, Tatiana, function54)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param2) {
			switch (params->param1) {
			default:
				break;

			case 0:
				getSound()->playSound(kEntityTatiana, "Tat5167A");
				params->param2 = 1;
				break;

			case 1:
				getSound()->playSound(kEntityTatiana, "Tat5167B");
				params->param2 = 1;
				break;

			case 2:
				getSound()->playSound(kEntityTatiana, "Tat5167C");
				params->param2 = 1;
				break;

			case 3:
				getSound()->playSound(kEntityTatiana, "Tat5167D");
				params->param2 = 1;
				break;
			}
		}

		if (params->param1 > 3) {
			if (!Entity::updateParameter(params->param3, getState()->timeTicks, 225))
				break;

			params->param1 = 0;
			params->param3 = 0;
		}
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventTatianaVassiliTalk);
		break;

	case kActionEndSound:
		++params->param1;
		params->param2 = 0;
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityTatiana, "033A");
		getData()->inventoryItem = kItemInvalid;
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			if (getSoundQueue()->isBuffered("MUS050"))
				getSoundQueue()->processEntry("MUS050");

			if (getSoundQueue()->isBuffered(kEntityTatiana))
				getSoundQueue()->processEntry(kEntityTatiana);

			getAction()->playAnimation(isNight() ? kEventTatianaVassiliTalkNight : kEventTatianaVassiliTalk);
			getScenes()->processScene();

			params->param1 = 4;
			params->param2 = 0;
			params->param3 = 0;
		}
		break;

	case kAction203078272:
		getEntities()->drawSequenceLeft(kEntityTatiana, "033E");
		break;

	case kAction236060709:
		getData()->inventoryItem = kItemNone;
		setup_function55();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(55, Tatiana, function55)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityTatiana);
		// fall back to next action

	case kActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 72))
			getScenes()->loadSceneFromPosition(kCarRestaurant, 86);
		break;
	}
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
