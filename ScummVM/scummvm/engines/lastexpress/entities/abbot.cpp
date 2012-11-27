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

#include "lastexpress/entities/abbot.h"

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

Abbot::Abbot(LastExpressEngine *engine) : Entity(engine, kEntityAbbot) {
	ADD_CALLBACK_FUNCTION(Abbot, reset);
	ADD_CALLBACK_FUNCTION(Abbot, draw);
	ADD_CALLBACK_FUNCTION(Abbot, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(Abbot, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION(Abbot, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(Abbot, draw2);
	ADD_CALLBACK_FUNCTION(Abbot, updateFromTime);
	ADD_CALLBACK_FUNCTION(Abbot, updateFromTicks);
	ADD_CALLBACK_FUNCTION(Abbot, playSound);
	ADD_CALLBACK_FUNCTION(Abbot, savegame);
	ADD_CALLBACK_FUNCTION(Abbot, updateEntity);
	ADD_CALLBACK_FUNCTION(Abbot, callSavepoint);
	ADD_CALLBACK_FUNCTION(Abbot, updatePosition);
	ADD_CALLBACK_FUNCTION(Abbot, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION(Abbot, chapter1);
	ADD_CALLBACK_FUNCTION(Abbot, chapter2);
	ADD_CALLBACK_FUNCTION(Abbot, chapter3);
	ADD_CALLBACK_FUNCTION(Abbot, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Abbot, conversationWithBoutarel);
	ADD_CALLBACK_FUNCTION(Abbot, readPaper);
	ADD_CALLBACK_FUNCTION(Abbot, goToLunch);
	ADD_CALLBACK_FUNCTION(Abbot, haveLunch);
	ADD_CALLBACK_FUNCTION(Abbot, function23);
	ADD_CALLBACK_FUNCTION(Abbot, function24);
	ADD_CALLBACK_FUNCTION(Abbot, function25);
	ADD_CALLBACK_FUNCTION(Abbot, function26);
	ADD_CALLBACK_FUNCTION(Abbot, function27);
	ADD_CALLBACK_FUNCTION(Abbot, function28);
	ADD_CALLBACK_FUNCTION(Abbot, function29);
	ADD_CALLBACK_FUNCTION(Abbot, function30);
	ADD_CALLBACK_FUNCTION(Abbot, function31);
	ADD_CALLBACK_FUNCTION(Abbot, function32);
	ADD_CALLBACK_FUNCTION(Abbot, function33);
	ADD_CALLBACK_FUNCTION(Abbot, function34);
	ADD_CALLBACK_FUNCTION(Abbot, function35);
	ADD_CALLBACK_FUNCTION(Abbot, function36);
	ADD_CALLBACK_FUNCTION(Abbot, function37);
	ADD_CALLBACK_FUNCTION(Abbot, function38);
	ADD_CALLBACK_FUNCTION(Abbot, chapter4);
	ADD_CALLBACK_FUNCTION(Abbot, function40);
	ADD_CALLBACK_FUNCTION(Abbot, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Abbot, function42);
	ADD_CALLBACK_FUNCTION(Abbot, function43);
	ADD_CALLBACK_FUNCTION(Abbot, function44);
	ADD_CALLBACK_FUNCTION(Abbot, function45);
	ADD_CALLBACK_FUNCTION(Abbot, function46);
	ADD_CALLBACK_FUNCTION(Abbot, drinkAfterDefuse);
	ADD_CALLBACK_FUNCTION(Abbot, function48);
	ADD_CALLBACK_FUNCTION(Abbot, pickBomb);
	ADD_CALLBACK_FUNCTION(Abbot, chapter5);
	ADD_CALLBACK_FUNCTION(Abbot, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Abbot, function52);
	ADD_CALLBACK_FUNCTION(Abbot, function53);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Abbot, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Abbot, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(3, Abbot, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(4, Abbot, enterExitCompartment2, ObjectIndex)
	Entity::enterExitCompartment(savepoint, kPosition_6470, kPosition_6130, kCarRedSleeping, kObjectCompartmentC, true, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(5, Abbot, callbackActionOnDirection)
	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SSI(6, Abbot, draw2, EntityIndex)
	Entity::draw2(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(7, Abbot, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(8, Abbot, updateFromTicks, uint32)
	Entity::updateFromTicks(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(9, Abbot, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(10, Abbot, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(11, Abbot, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kActionExcuseMeCath) {
		if (getEntities()->isPlayerPosition(kCarGreenSleeping, 18) || getEntities()->isPlayerPosition(kCarRedSleeping, 18)) {
			getSound()->excuseMe(kEntityAbbot);
		} else {
			if (getEvent(kEventAbbotIntroduction))
				getSound()->playSound(kEntityPlayer, "CAT1013");
			else
				getSound()->excuseMeCath();
		}
		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SIIS(12, Abbot, callSavepoint, EntityIndex, ActionIndex)
	Entity::callSavepoint(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SII(13, Abbot, updatePosition, CarIndex, Position)
	Entity::updatePosition(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Abbot, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Abbot, chapter1)
	if (savepoint.action == kActionDefault)
		getSavePoints()->addData(kEntityAbbot, kAction203073664, 0);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Abbot, chapter2)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityAbbot);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Abbot, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAbbot);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		getData()->clothes = kClothesDefault;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Abbot, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_draw("804DD");
			break;

		case 2:
			getSavePoints()->push(kEntityAbbot, kEntityCooks, kAction236976550);
			getEntities()->drawSequenceRight(kEntityAbbot, "804DS");

			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityAbbot);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			setCallback(4);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 4:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(5);
			setup_enterExitCompartment("617AC", kObjectCompartmentC);
			break;

		case 5:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getData()->entityPosition = kPosition_6470;
			getData()->location = kLocationInsideCompartment;

			setup_conversationWithBoutarel();
			break;
		}
		break;

	case kAction192054567:
		setCallback(1);
		setup_callbackActionRestaurantOrSalon();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Abbot, conversationWithBoutarel)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime1953000) {
			if (!params->param1) {
				params->param1 = 1;
				setCallback(3);
				setup_playSound("MrB3010");
			}
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityAbbot, "508A");
		getSavePoints()->push(kEntityAbbot, kEntityBoutarel, kAction122358304);

		setCallback(1);
		setup_playSound("Abb3010");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateFromTime(900);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityAbbot, "508B");
			break;

		case 3:
			getSavePoints()->push(kEntityAbbot, kEntityBoutarel, kAction122288808);
			setup_readPaper();
			break;
		}
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Abbot, readPaper)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime1966500 && getEntities()->isInRestaurant(kEntityBoutarel))
			setup_goToLunch();
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityAbbot, "509A");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Abbot, goToLunch)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_draw("509B");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject50, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			setCallback(2);
			setup_enterExitCompartment("617Mc", kObjectCompartmentC);
			break;

		case 2:
			getData()->location = kLocationOutsideCompartment;

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
			setup_draw("804US");
			break;

		case 5:
			getEntities()->drawSequenceRight(kEntityAbbot, "029J");
			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityAbbot);

			setCallback(6);
			setup_callbackActionOnDirection();
			break;

		case 6:
			getEntities()->drawSequenceLeft(kEntityAbbot, "029H");
			getSavePoints()->push(kEntityAbbot, kEntityPascale, kAction207769280);
			break;

		case 7:
			setup_haveLunch();
			break;
		}
		break;

	case kAction122288808:
		getSavePoints()->push(kEntityAbbot, kEntityTables4, kAction136455232);
		getData()->location = kLocationInsideCompartment;

		setCallback(7);
		setup_draw("029B");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Abbot, haveLunch)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheckSavepoint(kTime1971000, params->param1, kEntityAbbot, kEntityServers0, kAction218586752);

		if (getState()->time > kTime1989000 && getEntities()->isSomebodyInsideRestaurantOrSalon()) {
			getData()->inventoryItem = kItemNone;
			setup_function23();
		}
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventAbbotIntroduction);
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityAbbot, "029E");
		if (!getEvent(kEventAbbotIntroduction))
			getData()->inventoryItem = (InventoryItem)kCursorProcess;
		break;

	case kActionCallback:
		if (getCallback() != 1)
			break;

		getAction()->playAnimation(kEventAbbotIntroduction);
		getSound()->playSound(kEntityPlayer, "LIB036");
		getScenes()->loadSceneFromPosition(kCarRestaurant, 61);
		break;

	case kAction122288808:
		getEntities()->drawSequenceLeft(kEntityAbbot, "029E");
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityAbbot, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Abbot, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->location = kLocationOutsideCompartment;
		getEntities()->updatePositionEnter(kEntityAbbot, kCarRestaurant, 67);

		setCallback(1);
		setup_callSavepoint("029F", kEntityTables4, kActionDrawTablesWithChairs, "029G");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->updatePositionExit(kEntityAbbot, kCarRestaurant, 67);
			getSavePoints()->push(kEntityAbbot, kEntityServers0, kAction270068760);
			getSavePoints()->push(kEntityAbbot, kEntityAnna, kAction238936000);
			getEntities()->drawSequenceRight(kEntityAbbot, "804DS");

			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityAbbot);

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment2("617Cc", kObjectCompartmentC);
			break;

		case 4:
			getData()->location = kLocationInsideCompartment;

			setup_function24();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Abbot, function24)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param1, getState()->time, 900))
			break;

		setup_function25();
		break;

	case kActionKnock:
	case kActionOpenDoor:
		getObjects()->update(kObjectCompartmentC, kEntityAbbot, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject50, kEntityAbbot, kObjectLocation1, kCursorNormal, kCursorNormal);

		if (savepoint.action == kActionKnock) {
			setCallback(1);
			setup_playSound("LIB012");
		} else {
			setCallback(2);
			setup_playSound("LIB013");
		}
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAbbot);
		getObjects()->update(kObjectCompartmentC, kEntityAbbot, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kEntityAbbot, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_playSound("Abb3001");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentC, kEntityAbbot, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject50, kEntityAbbot, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Abbot, function25)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_enterExitCompartment("617Dc", kObjectCompartmentC);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject50, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			setCallback(2);
			setup_updateEntity(kCarRestaurant,  kPosition_850);
			break;

		case 2:
			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_updatePosition("115A", kCarRestaurant, 56);
			break;

		case 4:
			getData()->location = kLocationInsideCompartment;
			getScenes()->loadSceneFromItemPosition(kItem3);

			setup_function26();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Abbot, function26)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param2, getState()->time, 4500))
			break;

		if (getEntities()->isSomebodyInsideRestaurantOrSalon())
			setup_function27();
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityAbbot, kEntityKronos, kAction157159392);
		getEntities()->drawSequenceLeft(kEntityAbbot, "115B");
		break;

	case kAction101169422:
		params->param1 = 1;
		break;

	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Abbot, function27)
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
			setup_updatePosition("115C", kCarRestaurant, 56);
			break;

		case 2:
			getInventory()->setLocationAndProcess(kItem3, kObjectLocation1);

			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 3:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(4);
			setup_enterExitCompartment("617Ac", kObjectCompartmentC);
			break;

		case 4:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getData()->entityPosition = kPosition_6470;
			getData()->location = kLocationInsideCompartment;

			setup_function28();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Abbot, function28)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheckCallback(kTime2052000, params->param1, 1, WRAP_SETUP_FUNCTION(Abbot, setup_function29));
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityAbbot, kEntityBoutarel, kAction122358304);
		getEntities()->drawSequenceLeft(kEntityAbbot, "508A");

		setCallback(1);
		setup_playSound("abb3013");
		break;

	case kActionCallback:
		if (getCallback() == 1)
			getEntities()->drawSequenceLeft(kEntityAbbot, "508B");
		break;

	case kAction222609266:
		setup_function30();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Abbot, function29)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityAbbot, kEntityBoutarel, kAction122288808);
		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

		setCallback(1);
		setup_enterExitCompartment("617Bc", kObjectCompartmentC);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);

			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_9460);
			break;

		case 2:
			setCallback(3);
			setup_updateFromTicks(450);
			break;

		case 3:
			setCallback(4);
			setup_updateEntity(kCarGreenSleeping, kPosition_540);
			break;

		case 4:
			setCallback(5);
			setup_updateFromTime(225);
			break;

		case 5:
			setCallback(6);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 6:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(7);
			setup_enterExitCompartment("617Ac", kObjectCompartmentC);
			break;

		case 7:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getSavePoints()->push(kEntityAbbot, kEntityBoutarel, kAction122358304);
			getEntities()->drawSequenceLeft(kEntityAbbot, "508B");

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Abbot, function30)
switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_playSound("Abb3030");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityAbbot, kEntityBoutarel, kAction122288808);
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(2);
			setup_enterExitCompartment("617Bc", kObjectCompartmentC);
			break;

		case 2:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getData()->location = kLocationOutsideCompartment;

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
			setup_updatePosition("115A", kCarRestaurant, 56);
			break;

		case 5:
			getScenes()->loadSceneFromItemPosition(kItem3);
			getData()->location = kLocationInsideCompartment;

			setup_function31();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Abbot, function31)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param4 != kTimeInvalid && params->param2 < getState()->time) {
			if (params->param3 < getState()->time) {
				params->param4 = kTimeInvalid;

				setCallback(1);
				setup_callbackActionRestaurantOrSalon();
				break;
			} else {
				if (!getEntities()->isInSalon(kEntityPlayer) || !params->param4)
					params->param4 = (uint)getState()->time + 450;

				if (params->param4 < getState()->time) {
					params->param4 = kTimeInvalid;

					setCallback(1);
					setup_callbackActionRestaurantOrSalon();
					break;
				}
			}
		}

		if (!params->param1)
			break;

		if (!Entity::updateParameter(params->param5, getState()->time, 450))
			break;

		setCallback(6);
		setup_callbackActionRestaurantOrSalon();
		break;

	case kActionDefault:
		params->param2 = (uint)getState()->time + 4500;
		params->param3 = (uint)getState()->time + 18000;

		getEntities()->drawSequenceLeft(kEntityAbbot, "115B");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_updatePosition("115E", kCarRestaurant, 56);
			break;

		case 2:
			getInventory()->setLocationAndProcess(kItem3, kObjectLocation1);
			getSavePoints()->push(kEntityAbbot, kEntityAlexei, kAction122358304);
			getSound()->playSound(kEntityAbbot, "Abb3020");

			setCallback(3);
			setup_updatePosition("125A", kCarRestaurant, 52);
			break;

		case 3:
			getData()->location = kLocationInsideCompartment;
			getEntities()->drawSequenceLeft(kEntityAbbot, "125B");

			setCallback(4);
			setup_playSound("Abb3021");
			break;

		case 4:
			getSound()->playSound(kEntityAbbot, "Abb3023");
			getEntities()->updatePositionEnter(kEntityAbbot, kCarRestaurant, 52);

			setCallback(5);
			setup_draw2("125C1", "125C2", kEntityAlexei);
			break;

		case 5:
			getEntities()->updatePositionExit(kEntityAbbot, kCarRestaurant, 52);
			getEntities()->drawSequenceLeft(kEntityAbbot, "125D");
			getSavePoints()->push(kEntityAbbot, kEntityAlexei, kAction122288808);
			params->param1 = 1;

			if (!Entity::updateParameter(params->param5, getState()->time, 450))
				break;

			setCallback(6);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 6:
			getData()->location = kLocationOutsideCompartment;

			setCallback(7);
			setup_updatePosition("125E", kCarRestaurant, 52);
			break;

		case 7:
			setup_function32();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Abbot, function32)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_6470);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(1);
			setup_enterExitCompartment("617Ac", kObjectCompartmentC);
			break;

		case 2:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getData()->entityPosition = kPosition_6470;
			getData()->location = kLocationInsideCompartment;
			getSavePoints()->push(kEntityAbbot, kEntityBoutarel, kAction122358304);

			setup_function33();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Abbot, function33)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 != kTimeInvalid && getState()->time > kTime2115000) {
			if (getState()->time <= kTime2124000) {
				if (!getEntities()->isDistanceBetweenEntities(kEntityAbbot, kEntityPlayer, 2000) || !params->param1)
					params->param1 = (uint)getState()->time;

				if (params->param1 >= getState()->time)
					break;
			}

			params->param1 = kTimeInvalid;

			setCallback(1);
			setup_playSound("Abb3014");
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityAbbot, "508A");
		break;

	case kActionCallback:
		if (getCallback() == 1)
			getEntities()->drawSequenceLeft(kEntityAbbot, "508B");
		break;

	case kAction123712592:
		setup_function34();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(34, Abbot, function34)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_playSound("Abb3031");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityAbbot, kEntityBoutarel, kAction122288808);
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(2);
			setup_enterExitCompartment("617Bc", kObjectCompartmentC);
			break;

		case 2:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getData()->location = kLocationOutsideCompartment;

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
			setup_updatePosition("115A", kCarRestaurant, 56);
			break;

		case 5:
			getScenes()->loadSceneFromItemPosition(kItem3);

			getData()->location = kLocationInsideCompartment;
			setup_function35();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(35, Abbot, function35)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param2 == kTimeInvalid)
			break;

		if (params->param1 >= getState()->time) {
			if (!getEntities()->isInSalon(kEntityPlayer) || !params->param2)
				params->param2 = (uint)getState()->time + 450;

			if (params->param2 >= getState()->time)
				break;
		}

		params->param2 = kTimeInvalid;

		getSavePoints()->push(kEntityAbbot, kEntityAugust, kAction136196244);

		setCallback(1);
		setup_updateFromTime(0);
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityAbbot, "115B");
		params->param1 = (uint)getState()->time + 9000;
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
			getData()->location = kLocationOutsideCompartment;
			getSound()->playSound(kEntityAbbot, "Abb3040", kFlagInvalid, 45);
			getEntities()->updatePositionEnter(kEntityAbbot, kCarRestaurant, 57);

			setCallback(3);
			setup_callSavepoint("121A", kEntityAugust, kAction122358304, "BOGUS");
			break;

		case 3:
			getEntities()->updatePositionExit(kEntityAbbot, kCarRestaurant, 57);
			getInventory()->setLocationAndProcess(kItem3, kObjectLocation1);
			getData()->location = kLocationInsideCompartment;

			setup_function36();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(36, Abbot, function36)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		switch (params->param2) {
		default:
			break;

		case 1:
			if (params->param3 == kTimeInvalid)
				break;

			if (params->param1 >= getState()->time) {

				if (!getEntities()->isInSalon(kEntityPlayer) || !params->param3)
					params->param3 = (uint)getState()->time + 675;

				if (params->param3 >= getState()->time)
					break;
			}

			params->param3 = kTimeInvalid;

			getSound()->playSound(kEntityAbbot, "Abb3041");
			break;

		case 2:
			if (!Entity::updateParameter(params->param4, getState()->time, 900))
				break;

			getSound()->playSound(kEntityAbbot, "Abb3042");
			break;

		case 3:
			getSound()->playSound(kEntityAbbot, "Abb3043");
			getEntities()->updatePositionEnter(kEntityAbbot, kCarRestaurant, 57);

			setCallback(1);
			setup_callSavepoint("121D", kEntityAugust, kAction122288808, "BOGUS");
			break;
		}
		break;

	case kActionEndSound:
		++params->param2;
		break;

	case kActionDefault:
		params->param1 = (uint)getState()->time + 4500;
		getEntities()->drawSequenceLeft(kEntityAbbot, "121B");
		break;

	case kActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 57))
			getScenes()->loadSceneFromPosition(kCarRestaurant, 50);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getEntities()->updatePositionExit(kEntityAbbot, kCarRestaurant, 57);
			setup_function37();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(37, Abbot, function37)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_6470);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(2);
			setup_enterExitCompartment("617Ac", kObjectCompartmentC);
			break;

		case 2:
			getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getData()->entityPosition = kPosition_6470;
			getData()->location = kLocationInsideCompartment;
			getSavePoints()->push(kEntityAbbot, kEntityBoutarel, kAction122358304);

			setup_function38();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(38, Abbot, function38)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityAbbot, "508A");

		setCallback(1);
		setup_playSound("Abb3014A");
		break;

	case kActionCallback:
		if (getCallback() == 1)
			getEntities()->drawSequenceLeft(kEntityAbbot, "508B");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(39, Abbot, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAbbot);

		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 1) = 0;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(40, Abbot, function40, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->updateEntity(kEntityAbbot, (CarIndex)params->param1, (EntityPosition)params->param2)) {
			callbackAction();
		} else if (!getEvent(kEventAbbotInvitationDrink)
			    && getEntities()->isDistanceBetweenEntities(kEntityAbbot, kEntityPlayer, 1000)
			    && !getEntities()->isInsideCompartments(kEntityPlayer)
			    && !getEntities()->checkFields10(kEntityPlayer)) {

			if (getData()->car == kCarGreenSleeping || getData()->car == kCarRedSleeping) {
				setCallback(1);
				setup_savegame(kSavegameTypeEvent, kEventAbbotInvitationDrink);
			}
		}
		break;

	case kActionDefault:
		if (getEntities()->updateEntity(kEntityAbbot, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventAbbotInvitationDrink);
			getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + (750 * (getData()->direction == kDirectionUp ? -1 : 1))), getData()->direction == kDirectionUp);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(41, Abbot, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheckSavepoint(kTime2358000, params->param1, kEntityAbbot, kEntityServers0, kAction218128129);

		if (getState()->time > kTime2389500 && getEntities()->isSomebodyInsideRestaurantOrSalon())
			setup_function42();

		break;

	case kActionDefault:
		getSavePoints()->push(kEntityAbbot, kEntityTables4, kAction136455232);
		getEntities()->drawSequenceLeft(kEntityAbbot, "029E");
		getData()->location = kLocationInsideCompartment;
		break;

	case kAction122288808:
		getEntities()->drawSequenceLeft(kEntityAbbot, "029E");
		break;

	case kAction122358304:
		getEntities()->drawSequenceLeft(kEntityAbbot, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(42, Abbot, function42)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->location = kLocationOutsideCompartment;
		getEntities()->updatePositionExit(kEntityAbbot, kCarRestaurant, 67);

		setCallback(1);
		setup_callSavepoint("029F", kEntityTables4, kActionDrawTablesWithChairs, "029G");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->updatePositionExit(kEntityAbbot, kCarRestaurant, 67);
			getSavePoints()->push(kEntityAbbot, kEntityServers0, kAction270068760);
			getEntities()->drawSequenceRight(kEntityAbbot, "804DS");

			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityAbbot);

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment2("617Cc", kObjectCompartmentC);
			break;

		case 4:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityAbbot);

			setup_function43();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(43, Abbot, function43)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 && params->param4 != kTimeInvalid && params->param2 < getState()->time) {
			if (getState()->time < kTime2452500) {
				params->param4 = kTimeInvalid;

				setCallback(1);
				setup_playSound("Abb4002");
				break;
			} else {
				if (!getEntities()->isDistanceBetweenEntities(kEntityAbbot, kEntityPlayer, 1000) || getSoundQueue()->isBuffered(kEntityBoutarel) || !params->param4)
					params->param4 = (uint)getState()->time + 450;

				if (params->param4 < getState()->time) {
					params->param4 = kTimeInvalid;

					setCallback(1);
					setup_playSound("Abb4002");
					break;
				}
			}
		}

label_callback_1:
		if (Entity::timeCheck(kTime2466000, params->param5, WRAP_SETUP_FUNCTION(Abbot, setup_function44)))
			break;

		if (params->param3) {
			if (!Entity::updateParameter(params->param6, getState()->timeTicks, 75))
				break;

			params->param2 = 1;
			params->param3 = 0;

			getObjects()->update(kObjectCompartmentC, kEntityAbbot, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject50, kEntityAbbot, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param6 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		getObjects()->update(kObjectCompartmentC, kEntityAbbot, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject50, kEntityAbbot, kObjectLocation1, kCursorNormal, kCursorNormal);

		if (params->param3) {
			setCallback(savepoint.param.intValue == 50 ? 5 : 6);
			setup_playSound(savepoint.param.intValue == 50 ? getSound()->justAMinuteCath() : getSound()->wrongDoorCath());
		} else {
			setCallback(savepoint.action == kActionKnock ? 2 : 3);
			setup_playSound(savepoint.action == kActionKnock ?  "LIB012" : "LIB013");
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentC, kEntityAbbot, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kEntityAbbot, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (params->param2 || params->param3) {
			getObjects()->update(kObjectCompartmentC, kEntityAbbot, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject50, kEntityAbbot, kObjectLocation1, kCursorHandKnock, kCursorHand);

			params->param2 = 0;
			params->param3 = 0;
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback_1;

		case 2:
		case 3:
			setCallback(4);
			setup_playSound("Abb3001");
			break;

		case 4:
			getObjects()->update(kObjectCompartmentC, kEntityAbbot, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject50, kEntityAbbot, kObjectLocation1, kCursorTalk, kCursorNormal);

			params->param3 = 1;
			break;

		case 5:
		case 6:
			params->param2 = 1;
			params->param3 = 0;
			break;
		}
		break;

	case kAction101687594:
		params->param1 = 1;
		break;

	case kAction159003408:
		params->param1 = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(44, Abbot, function44)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->clearSequences(kEntityAbbot);

		getObjects()->update(kObjectCompartmentC, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kAction104060776:
		setup_function45();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(45, Abbot, function45)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6471;
		getData()->car = kCarRedSleeping;
		getData()->location = kLocationOutsideCompartment;

		RESET_ENTITY_STATE(kEntityVerges, Verges, setup_resetState);

		getEntities()->drawSequenceLeft(kEntityAbbot, "617Ec");
		getEntities()->enterCompartment(kEntityAbbot, kObjectCompartmentC, true);

		setCallback(1);
		setup_playSound("Abb4010");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("617Kc", kObjectCompartmentC);
			break;

		case 2:
			getEntities()->exitCompartment(kEntityAbbot, kObjectCompartmentC, true);
			getSavePoints()->push(kEntityAbbot, kEntityVerges, kAction125233040);

			setup_function46();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(46, Abbot, function46)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6471;

		setCallback(1);
		setup_function40(kCarRestaurant, kPosition_850);
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_drinkAfterDefuse();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(47, Abbot, drinkAfterDefuse)
	switch (savepoint.action) {
	default:
		break;

	case kAction1:
		setCallback(3);
		setup_savegame(kSavegameTypeEvent, kEventAbbotDrinkGiveDetonator);
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
			setup_draw("126A");
			break;

		case 2:
			getData()->location = kLocationInsideCompartment;
			getEntities()->drawSequenceLeft(kEntityAbbot, "126B");
			getData()->inventoryItem = kItemBomb;
			break;

		case 3:
			getAction()->playAnimation(kEventAbbotDrinkGiveDetonator);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneNone, true);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(48, Abbot, function48)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(0, 1))
			getData()->inventoryItem = kItemInvalid;

		if (Entity::updateParameter(params->param1, getState()->time, 1800)) {
			getData()->inventoryItem = kItemNone;

			setCallback(4);
			setup_updatePosition("126C", kCarRedSleeping, 52);
		}

		Entity::timeCheckCallbackInventory(kTime2533500, params->param2, 5, WRAP_SETUP_FUNCTION(Abbot, setup_callbackActionRestaurantOrSalon));
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;

		setCallback(9);
		setup_savegame(kSavegameTypeEvent, kEventAbbotDrinkDefuse);
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_850;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->inventoryItem = kItemNone;

		getSavePoints()->push(kEntityAbbot, kEntityVerges, kAction125233040);

		setCallback(1);
		setup_updateEntity(kCarRestaurant, kPosition_850);
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
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(3);
			setup_updatePosition("126A", kCarRestaurant, 52);
			break;

		case 3:
			getData()->location = kLocationInsideCompartment;
			getEntities()->drawSequenceLeft(kEntityAbbot, "126B");
			break;

		case 4:
			if (!getEvent(kEventAbbotDrinkDefuse) && ENTITY_PARAM(0, 1))
				getData()->inventoryItem = kItemInvalid;

			getEntities()->drawSequenceLeft(kEntityAbbot, "126B");
			params->param1 = 0;

			Entity::timeCheckCallbackInventory(kTime2533500, params->param2, 5, WRAP_SETUP_FUNCTION(Abbot, setup_callbackActionRestaurantOrSalon));
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;

			setCallback(6);
			setup_updatePosition("126D", kCarRestaurant, 52);
			break;

		case 6:
			setCallback(7);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 7:
			setCallback(8);
			setup_enterExitCompartment2("617Cc", kObjectCompartmentC);
			break;

		case 8:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityAbbot);

			setup_function44();
			break;

		case 9:
			getAction()->playAnimation(kEventAbbotDrinkDefuse);
			getEntities()->drawSequenceLeft(kEntityAbbot, "126B");
			getSavePoints()->push(kEntityAbbot, kEntityAnna, kAction100969180);
			getScenes()->loadSceneFromPosition(kCarRestaurant, 58);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(49, Abbot, pickBomb)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param1, getState()->timeTicks, 150))
			break;

		getSavePoints()->push(kEntityAbbot, kEntityAbbot, kAction157489665);
		break;

	case kActionKnock:
		if (!getSoundQueue()->isBuffered("LIB012", true))
			getSound()->playSound(kEntityPlayer, "LIB012");
		break;

	case kActionOpenDoor:
	case kAction157489665:
		getSavePoints()->push(kEntityAbbot, kEntityTatiana, kAction238790488);
		getObjects()->update(kObjectCompartment2, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectHandleInsideBathroom, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		break;

	case kActionDefault:
		getData()->car = kCarGreenSleeping;
		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationInsideCompartment;

		getSavePoints()->call(kEntityAbbot, kEntityTables4, kActionDrawTablesWithChairs, "029G");
		getSavePoints()->push(kEntityAbbot, kEntityServers0, kAction270068760);
		getSavePoints()->push(kEntityAbbot, kEntityBoutarel, kAction125039808);
		getObjects()->update(kObjectCompartment2, kEntityAbbot, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectHandleInsideBathroom, kEntityAbbot, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(getObjects()->get(kObjectCompartment2).location2 < kObjectLocation2 ? kEventAbbotWrongCompartmentBed : kEventAbbotWrongCompartment);
			getEntities()->updateEntity(kEntityAbbot, kCarRedSleeping, kPosition_6470);
			getSound()->playSound(kEntityPlayer, "LIB015");
			getScenes()->loadSceneFromObject(kObjectCompartment2, true);

			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 2:
			setCallback(3);
			setup_enterExitCompartment2("617Cc", kObjectCompartmentC);
			break;

		case 3:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityAbbot);
			getObjects()->update(kObjectCompartmentC, kEntityAbbot, kObjectLocation1, kCursorHandKnock, kCursorHand);

			setup_function43();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(50, Abbot, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAbbot);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		getData()->clothes = kClothesDefault;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(51, Abbot, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_function52();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(52, Abbot, function52)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAbbot);

		getData()->entityPosition = kPositionNone;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarNone;
		break;

	case kAction135600432:
		setup_function53();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(53, Abbot, function53)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getInventory()->setLocationAndProcess(kItem25, kObjectLocation1);
		getSavePoints()->push(kEntityAbbot, kEntityAnna, kAction158480160);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventLocomotiveAbbotGetSomeRest);
			getScenes()->processScene();
			break;

		case 2:
			getAction()->playAnimation(kEventLocomotiveAbbotShoveling);
			getScenes()->processScene();
			break;
		}
		break;

	case kAction168646401:
		if (!getEvent(kEventLocomotiveAbbotGetSomeRest)) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventLocomotiveAbbotGetSomeRest);
			break;
		}

		if (!getEvent(kEventLocomotiveAbbotShoveling)) {
			setCallback(2);
			setup_savegame(kSavegameTypeEvent, kEventLocomotiveAbbotShoveling);
			break;
		}

		getAction()->playAnimation(kEventLocomotiveAbbotShoveling);
		getScenes()->processScene();
		break;
	}
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
