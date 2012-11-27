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

#include "lastexpress/entities/mertens.h"

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


Mertens::Mertens(LastExpressEngine *engine) : Entity(engine, kEntityMertens) {
	ADD_CALLBACK_FUNCTION(Mertens, reset);
	ADD_CALLBACK_FUNCTION(Mertens, bloodJacket);
	ADD_CALLBACK_FUNCTION(Mertens, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(Mertens, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION(Mertens, enterExitCompartment3);
	ADD_CALLBACK_FUNCTION(Mertens, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(Mertens, playSound);
	ADD_CALLBACK_FUNCTION(Mertens, playSound16);
	ADD_CALLBACK_FUNCTION(Mertens, savegame);
	ADD_CALLBACK_FUNCTION(Mertens, updateEntity);
	ADD_CALLBACK_FUNCTION(Mertens, function11);
	ADD_CALLBACK_FUNCTION(Mertens, bonsoir);
	ADD_CALLBACK_FUNCTION(Mertens, function13);
	ADD_CALLBACK_FUNCTION(Mertens, function14);
	ADD_CALLBACK_FUNCTION(Mertens, function15);
	ADD_CALLBACK_FUNCTION(Mertens, function16);
	ADD_CALLBACK_FUNCTION(Mertens, function17);
	ADD_CALLBACK_FUNCTION(Mertens, function18);
	ADD_CALLBACK_FUNCTION(Mertens, function19);
	ADD_CALLBACK_FUNCTION(Mertens, function20);
	ADD_CALLBACK_FUNCTION(Mertens, function21);
	ADD_CALLBACK_FUNCTION(Mertens, function22);
	ADD_CALLBACK_FUNCTION(Mertens, function23);
	ADD_CALLBACK_FUNCTION(Mertens, function24);
	ADD_CALLBACK_FUNCTION(Mertens, function25);
	ADD_CALLBACK_FUNCTION(Mertens, function26);
	ADD_CALLBACK_FUNCTION(Mertens, tylerCompartment);
	ADD_CALLBACK_FUNCTION(Mertens, function28);
	ADD_CALLBACK_FUNCTION(Mertens, function29);
	ADD_CALLBACK_FUNCTION(Mertens, function30);
	ADD_CALLBACK_FUNCTION(Mertens, function31);
	ADD_CALLBACK_FUNCTION(Mertens, function32);
	ADD_CALLBACK_FUNCTION(Mertens, function33);
	ADD_CALLBACK_FUNCTION(Mertens, chapter1);
	ADD_CALLBACK_FUNCTION(Mertens, function35);
	ADD_CALLBACK_FUNCTION(Mertens, function36);
	ADD_CALLBACK_FUNCTION(Mertens, function37);
	ADD_CALLBACK_FUNCTION(Mertens, function38);
	ADD_CALLBACK_FUNCTION(Mertens, function39);
	ADD_CALLBACK_FUNCTION(Mertens, function40);
	ADD_CALLBACK_FUNCTION(Mertens, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Mertens, function42);
	ADD_CALLBACK_FUNCTION(Mertens, chapter2);
	ADD_CALLBACK_FUNCTION(Mertens, function44);
	ADD_CALLBACK_FUNCTION(Mertens, chapter3);
	ADD_CALLBACK_FUNCTION(Mertens, function46);
	ADD_CALLBACK_FUNCTION(Mertens, chapter4);
	ADD_CALLBACK_FUNCTION(Mertens, function48);
	ADD_CALLBACK_FUNCTION(Mertens, function49);
	ADD_CALLBACK_FUNCTION(Mertens, chapter5);
	ADD_CALLBACK_FUNCTION(Mertens, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Mertens, function52);
	ADD_CALLBACK_FUNCTION(Mertens, function53);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Mertens, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Mertens, bloodJacket)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		break;

	case kActionExitCompartment:
		callbackAction();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(kEntityMertens, (char *)&params->seq1);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(3, Mertens, enterExitCompartment, ObjectIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		return;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		return;
	}

	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(4, Mertens, enterExitCompartment2, ObjectIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		return;

	case kAction4:
		getEntities()->exitCompartment(kEntityMertens, (ObjectIndex)params->param4);
		callbackAction();
		return;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		return;
	}

	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SIII(5, Mertens, enterExitCompartment3, ObjectIndex, EntityPosition, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		break;

	case kActionExitCompartment:
		getEntities()->exitCompartment(_entityIndex, (ObjectIndex)params->param4);
		getData()->entityPosition = (EntityPosition)params->param5;
		callbackAction();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(_entityIndex, (char *)&params->seq);
		getEntities()->enterCompartment(_entityIndex, (ObjectIndex)params->param4);
		getData()->entityPosition = (EntityPosition)params->param5;

		if (getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, (EntityPosition)params->param5) || getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, (EntityPosition)params->param6)) {
			getAction()->playAnimation(isNight() ? kEventCathTurningNight : kEventCathTurningDay);
			getSound()->playSound(kEntityPlayer, "BUMP");
			getScenes()->loadSceneFromObject((ObjectIndex)params->param4);
		}
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Mertens, callbackActionOnDirection)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getData()->direction != kDirectionRight) {
			callbackAction();
			break;
		}

		Entity::savegameBloodJacket();
		break;

	case kActionExitCompartment:
		callbackAction();
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(7, Mertens, playSound)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		break;

	case kActionEndSound:
		callbackAction();
		break;

	case kActionDefault:
		getSound()->playSound(kEntityMertens, (char *)&params->seq1);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(8, Mertens, playSound16)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		break;

	case kActionEndSound:
		callbackAction();
		break;

	case kActionDefault:
		getSound()->playSound(kEntityMertens, (char *)&params->seq1, kFlagDefault);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(9, Mertens, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(10, Mertens, updateEntity, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param3 && getEntities()->isDistanceBetweenEntities(kEntityMertens, kEntityPlayer, 2000))
			getData()->inventoryItem = (InventoryItem)(getData()->inventoryItem | kItemInvalid);
		else
			getData()->inventoryItem = (InventoryItem)(getData()->inventoryItem & kItemToggleHigh);

		if (!getEntities()->isDistanceBetweenEntities(kEntityMertens, kEntityPlayer, 1000)
		  || getEntities()->isInsideCompartments(kEntityPlayer)
		  || getEntities()->checkFields10(kEntityPlayer)) {
			if (getEntities()->updateEntity(kEntityMertens, (CarIndex)params->param1, (EntityPosition)params->param2)) {
				getData()->inventoryItem = kItemNone;
				callbackAction();
			}
			break;
		}

		if (getProgress().jacket == kJacketBlood) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventMertensBloodJacket);
			break;
		}

		if ((ENTITY_PARAM(0, 6) || ENTITY_PARAM(0, 7)) && (!getEvent(kEventKronosConversation) && getProgress().jacket == kJacketGreen)) {
			setCallback(2);
			setup_savegame(kSavegameTypeEvent, kEventMertensKronosInvitation);
			break;
		}

		if (ENTITY_PARAM(1, 2) && getProgress().jacket == kJacketGreen && !getProgress().eventMetAugust) {
			setCallback(3);
			setup_savegame(kSavegameTypeEvent, kEventMertensAugustWaiting);
			break;
		}

		if (ENTITY_PARAM(2, 4) && getState()->time < kTime2133000) {
			setCallback(4);
			setup_savegame(kSavegameTypeEvent, kEventMertensKronosConcertInvitation);
			break;
		}

		if (getEntities()->updateEntity(kEntityMertens, (CarIndex)params->param1, (EntityPosition)params->param2)) {
			getData()->inventoryItem = kItemNone;
			callbackAction();
		}
		break;

	case kAction1:
		params->param3 = 0;
		if (getProgress().eventCorpseFound || getEvent(kEventMertensAskTylerCompartment) || getEvent(kEventMertensAskTylerCompartmentD)) {
			if (ENTITY_PARAM(0, 4) && getProgress().jacket == kJacketGreen && !getEvent(kEventMertensDontMakeBed) && !getProgress().eventCorpseThrown) {
				setCallback(6);
				setup_savegame(kSavegameTypeEvent, kEventMertensDontMakeBed);
			}
		} else {
			setCallback(5);
			setup_savegame(kSavegameTypeEvent, kEventMertensAskTylerCompartment);
		}
		break;

	case kActionExcuseMeCath:
		getSound()->playSound(kEntityMertens, "CON1110B");
		break;

	case kActionExcuseMe:
		getSound()->excuseMe(kEntityMertens);
		break;

	case kActionDefault:
		if ((!getProgress().eventCorpseFound && !getEvent(kEventMertensAskTylerCompartment) && !getEvent(kEventMertensAskTylerCompartmentD))
		 || (ENTITY_PARAM(0, 4) && getProgress().jacket == kJacketGreen && !getEvent(kEventMertensDontMakeBed) && !getProgress().eventCorpseThrown))
			params->param3 = 1;

		if (getEntities()->updateEntity(kEntityMertens, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			break;

		case 2:
			getAction()->playAnimation(getData()->entityPosition < getEntityData(kEntityPlayer)->entityPosition ? kEventMertensKronosInvitation : kEventMertensKronosInvitationClosedWindows);
			getProgress().eventMertensKronosInvitation = true;

			ENTITY_PARAM(0, 6) = 0;
			ENTITY_PARAM(0, 7) = 0;

			if (params->param1 != 3 || (params->param2 != kPosition_8200 && params->param2 != kPosition_9510)) {
				loadSceneFromPosition();
				break;
			}

			getData()->inventoryItem = kItemNone;

			if (getData()->car == kCarGreenSleeping && getEntities()->checkDistanceFromPosition(kEntityMertens, kPosition_2000, 500))
				getData()->entityPosition = kPosition_2500;

			getEntities()->updateEntity(kEntityMertens, kCarGreenSleeping, kPosition_2000);
			getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + 750));

			callbackAction();
			break;

		case 3:
			getAction()->playAnimation(kEventMertensAugustWaiting);
			getProgress().eventMertensAugustWaiting = true;

			ENTITY_PARAM(1, 2) = 0;

			if (params->param1 == 3 && params->param2 == kPosition_8200) {
				if (getData()->car == kCarGreenSleeping && getEntities()->checkDistanceFromPosition(kEntityMertens, kPosition_2000, 500))
					getData()->entityPosition = kPosition_2500;

				getEntities()->updateEntity(kEntityMertens, kCarGreenSleeping, kPosition_2000);
				getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + 750));

				callbackAction();
				break;
			}

			loadSceneFromPosition();
			break;

		case 4:
			getAction()->playAnimation(kEventMertensKronosConcertInvitation);
			ENTITY_PARAM(2, 4) = 0;

			loadSceneFromPosition();
			break;

		case 5:
			getAction()->playAnimation(getData()->entityPosition < getEntityData(kEntityPlayer)->entityPosition ? kEventMertensAskTylerCompartmentD : kEventMertensAskTylerCompartment);
			loadSceneFromPosition();
			break;

		case 6:
			getAction()->playAnimation(kEventMertensDontMakeBed);
			loadSceneFromPosition();
			ENTITY_PARAM(0, 4) = 0;
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(11, Mertens, function11, uint32)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();

		if (!Entity::updateParameter(params->param2, getState()->time, params->param1))
			break;

		callbackAction();
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(12, Mertens, bonsoir, EntityIndex)
	EntityIndex entity = (EntityIndex)params->param1;

	if (savepoint.action == kActionDefault)
		return;

	if (getSoundQueue()->isBuffered(kEntityMertens)) {
		callbackAction();
		return;
	}

	if (isNight()) {
		if (Entities::isFemale(entity)) {
			getSound()->playSound(kEntityMertens, rnd(2) ? "CON1112" : "CON1112A");
		} else {
			if (entity || getProgress().field_18 != 2) {
				getSound()->playSound(kEntityMertens, "CON1112F");
			} else {
				switch (rnd(3)) {
				default:
				case 0:
					getSound()->playSound(kEntityMertens, "CON1061");
					break;

				case 1:
					getSound()->playSound(kEntityMertens, "CON1110G");
					break;

				case 2:
					getSound()->playSound(kEntityMertens, "CON1110H");
					break;
				}
			}
		}
	} else {
		if (Entities::isFemale(entity))
			getSound()->playSound(kEntityMertens, rnd(2) ? "CON1112B" : "CON1112C");
		else
			getSound()->playSound(kEntityMertens, "CON1112G");
	}

	callbackAction();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(13, Mertens, function13, bool, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();

		if (!params->param2 && !params->param3) {
			if (Entity::updateParameter(params->param4, getState()->timeTicks, 75)) {
				getData()->inventoryItem = kItemNone;
				setCallback(5);
				setup_function18();
				break;
			}
		}

		if (Entity::updateParameter(params->param5, getState()->timeTicks, 225)) {
			getData()->inventoryItem = kItemNone;
			setCallback(6);
			setup_function18();
			break;
		}

		getData()->inventoryItem = (getProgress().chapter == kChapter1
								 && !ENTITY_PARAM(2, 1)
								 && !getProgress().eventCorpseFound
								 && !getEvent(kEventMertensAskTylerCompartment)
								 && !getEvent(kEventMertensAskTylerCompartmentD)) ? kItemMatchBox : kItemNone;
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;
		setCallback(7);
		setup_savegame(kSavegameTypeEvent, kEventMertensAskTylerCompartmentD);
		break;

	case kAction11:
		params->param3++;
		setCallback(11);
		setup_bonsoir(savepoint.entity2);
		break;

	case kActionDefault:
		if (params->param2)
			params->param3 = 1;

		if (!getSoundQueue()->isBuffered(kEntityMertens)) {

		}

		setCallback(3);
		setup_function20();
		break;

	case kAction16:
		params->param3--;

		if (params->param2 && !params->param3) {
			getData()->inventoryItem = kItemNone;
			setCallback(10);
			setup_function18();
		}
		break;

	case kActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarGreenSleeping, 23) && ENTITY_PARAM(0, 7) && !getEvent(kEventKronosConversation)) {
			setCallback(8);
			setup_savegame(kSavegameTypeEvent, kEventMertensKronosInvitation);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_function20();
			break;

		case 3:
			getEntities()->drawSequenceLeft(kEntityMertens, params->param1 ? "601I" : "601H");
			break;

		case 4:
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			break;

		case 5:
		case 6:
		case 9:
		case 10:
			callbackAction();
			break;

		case 7:
			getAction()->playAnimation(kEventMertensAskTylerCompartmentD);
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 25);
			break;

		case 8:
			getAction()->playAnimation(kEventMertensKronosInvitation);
			ENTITY_PARAM(0, 6) = 0;
			ENTITY_PARAM(0, 7) = 0;
			getScenes()->processScene();

			if (!params->param3) {
				getData()->inventoryItem = kItemNone;
				setCallback(10);
				setup_function18();
			}
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(14, Mertens, function14, EntityIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		break;

	case kActionDefault:
		getData()->inventoryItem = kItemNone;

		if (ENTITY_PARAM(2, 1)) {
			ENTITY_PARAM(2, 1) = 0;

			setCallback(3);
			setup_updateEntity(kCarGreenSleeping, kPosition_1500);
		} else {
			setCallback(1);
			setup_function11(15);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityMertens, (EntityIndex)params->param1, kAction202558662);

			setCallback(2);
			setup_function20();
			break;

		case 2:
			getSavePoints()->push(kEntityMertens, (EntityIndex)params->param1, kAction155853632);
			getEntities()->drawSequenceLeft(kEntityMertens, "601K");
			break;

		case 3:
			getSavePoints()->push(kEntityMertens, (EntityIndex)params->param1, kAction202558662);
			getSavePoints()->push(kEntityMertens, (EntityIndex)params->param1, kAction155853632);
			getEntities()->drawSequenceLeft(kEntityMertens, "601K");
			getScenes()->loadSceneFromItemPosition(kItem7);
			break;

		case 4:
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			break;

		case 5:
			callbackAction();
			break;
		}
		break;

	case kAction125499160:
		if (params->param1 == kEntityVerges)
			ENTITY_PARAM(0, 8) = 0;

		setCallback(5);
		setup_function18();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(15, Mertens, function15, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		ENTITY_PARAM(1, 4) = 0;
		ENTITY_PARAM(1, 5) = 0;

		setCallback(1);
		setup_function19();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_4070);
			break;

		case 2:
			getSound()->playSound(kEntityMertens, params->param1 ? "CON1059A" : "CON1059");

			setCallback(3);
			setup_updateEntity(kCarGreenSleeping, kPosition_7500);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment("601Xb", kObjectCompartment2);
			break;

		case 4:
			getSavePoints()->push(kEntityMertens, kEntityAlexei, kAction135664192);

			setCallback(5);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 5:
			setCallback(6);
			setup_function17();
			break;

		case 6:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(16, Mertens, function16, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		ENTITY_PARAM(1, 6) = 0;
		ENTITY_PARAM(1, 7) = 0;

		setCallback(1);
		setup_function19();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_4070);
			break;

		case 2:
			switch (rnd(4)) {
			default:
				break;

			case 0:
				getSound()->playSound(kEntityMertens, "AUG2095A");
				break;

			case 1:
				getSound()->playSound(kEntityMertens, "AUG2096A");
				break;

			case 2:
				getSound()->playSound(kEntityMertens, "AUG2094B");
				break;

			case 3:
				getSound()->playSound(kEntityMertens, "AUG2094C");
				break;
			}

			setCallback(3);
			setup_updateEntity(kCarGreenSleeping, kPosition_6470);
			break;

		case 3:
			getSound()->playSound(kEntityMertens, params->param1 ? "AUG2097" : "AUG2098");

			setCallback(4);
			setup_enterExitCompartment("601Xc", kObjectCompartment3);
			break;

		case 4:
			getSavePoints()->push(kEntityMertens, kEntityAugust, kAction69239528);

			setCallback(5);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 5:
			setCallback(6);
			setup_function17();
			break;

		case 6:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Mertens, function17)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		// FIXME: Check that we are using the correct parameter struct
		if (ENTITY_PARAM(0, 6) || ((EntityData::EntityParametersIIII*)_data->getParameters(8, 1))->hasNonNullParameter()) {
			getInventory()->setLocationAndProcess(kItem7, kObjectLocation1);

			setCallback(1);
			setup_updateEntity(kCarGreenSleeping, kPosition_540);
			break;
		}

		if (ENTITY_PARAM(0, 8)) {
			getEntities()->drawSequenceLeft(kEntityMertens, "601K");
			getScenes()->loadSceneFromItemPosition(kItem7);
			ENTITY_PARAM(2, 1) = 1;

			callbackAction();
			break;
		}

		// Mertens sits on his chair at the back of the train
		if (!getInventory()->hasItem(kItemPassengerList) || ENTITY_PARAM(0, 2)) {
			getEntities()->drawSequenceRight(kEntityMertens, "601A");
		} else {
			// Got the passenger list, Mertens is looking for it before sitting
			ENTITY_PARAM(0, 2) = 1;
			getSound()->playSound(kEntityMertens, "CON1058", kFlagInvalid, 75);
			getEntities()->drawSequenceRight(kEntityMertens, "601D");
		}

		getScenes()->loadSceneFromItemPosition(kItem7);

		if (getEntities()->isPlayerPosition(kCarGreenSleeping, 68)) {
			getSound()->playSound(kEntityPlayer, "CON1110");
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 25);
		}

		setCallback(3);
		setup_callbackActionOnDirection();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->clearSequences(kEntityMertens);
			ENTITY_PARAM(2, 1) = 1;
			setCallback(2);
			setup_function11(75);
			break;

		case 2:
			callbackAction();
			break;

		case 3:
			if (!ENTITY_PARAM(0, 3)
			 && !getInventory()->hasItem(kItemPassengerList)
			 && ENTITY_PARAM(0, 2)) {
				 getSavePoints()->push(kEntityMertens, kEntityVerges, kAction158617345);
				 ENTITY_PARAM(0, 3) = 1;
			}

			getEntities()->drawSequenceLeft(kEntityMertens, "601B");

			ENTITY_PARAM(0, 1) = 0;
			getData()->inventoryItem = kItemNone;

			getSavePoints()->push(kEntityMertens, kEntityMertens, kActionDrawScene);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Mertens, function18)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (ENTITY_PARAM(0, 6)
		 || ENTITY_PARAM(1, 1)
		 || ENTITY_PARAM(1, 2)
		 || ENTITY_PARAM(1, 3)
		 || ENTITY_PARAM(1, 4)
		 || ENTITY_PARAM(1, 5)
		 || ENTITY_PARAM(1, 6)
		 || ENTITY_PARAM(1, 7)
		 || ENTITY_PARAM(1, 8)) {
			getInventory()->setLocationAndProcess(kItem7, kObjectLocation1);
			ENTITY_PARAM(2, 1) = 1;

			callbackAction();
			break;
		}

		if (ENTITY_PARAM(0, 8)) {
			getScenes()->loadSceneFromItemPosition(kItem7);
			ENTITY_PARAM(2, 1) = 1;

			callbackAction();
			break;
		}

		if (!getInventory()->hasItem(kItemPassengerList) || ENTITY_PARAM(0, 2)) {
			getEntities()->drawSequenceRight(kEntityMertens, "601A");
		} else {
			ENTITY_PARAM(0, 2) = 1;
			getSound()->playSound(kEntityMertens, "CON1058", kFlagInvalid, 75);
			getEntities()->drawSequenceRight(kEntityMertens, "601D");
		}

		getScenes()->loadSceneFromItemPosition(kItem7);

		setCallback(1);
		setup_callbackActionOnDirection();
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			if (!ENTITY_PARAM(0, 3)
			 && !getInventory()->hasItem(kItemPassengerList)
			 && ENTITY_PARAM(0, 2)) {
				getSavePoints()->push(kEntityMertens, kEntityVerges, kAction158617345);
				ENTITY_PARAM(0, 3) = 1;
			}

			getEntities()->drawSequenceLeft(kEntityMertens, "601B");
			ENTITY_PARAM(0, 1) = 0;
			getData()->inventoryItem = kItemNone;

			callbackAction();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Mertens, function19)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (ENTITY_PARAM(2, 1)) {
			getInventory()->setLocationAndProcess(kItem7, kObjectLocation1);
			ENTITY_PARAM(2, 1) = 0;
			callbackAction();
		} else {
			setCallback(1);
			setup_bloodJacket("601C");
		}
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getInventory()->setLocationAndProcess(kItem7, kObjectLocation1);

			if (!getEntities()->isPlayerPosition(kCarGreenSleeping, 2))
				getData()->entityPosition = kPosition_2088;

			callbackAction();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Mertens, function20)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getScenes()->loadSceneFromItemPosition(kItem7);

		if (ENTITY_PARAM(2, 1)) {
			ENTITY_PARAM(2, 1) = 0;

			callbackAction();
		} else {
			setCallback(1);
			setup_bloodJacket("601C");
		}
		break;

	case kActionCallback:
		if (getCallback() == 1)
			callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(21, Mertens, function21, ObjectIndex, ObjectIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::updateParameter(CURRENT_PARAM(1, 4), getState()->time, 300)) {
			getSound()->playSound(kEntityPlayer, "ZFX1004", getSound()->getSoundFlag(kEntityMertens));
		}

		if (!Entity::updateParameter(CURRENT_PARAM(1, 5), getState()->time, 900))
			break;

		// Update objects
		getObjects()->updateLocation2((ObjectIndex)params->param1, kObjectLocation1);
		if (params->param5 != kObjectLocation2)
			getObjects()->update((ObjectIndex)params->param1, (EntityIndex)params->param4, (ObjectLocation)params->param5, (CursorStyle)params->param6, (CursorStyle)params->param7);

		if (params->param2)
			getObjects()->update((ObjectIndex)params->param2, (EntityIndex)params->param8, (ObjectLocation)CURRENT_PARAM(1, 1), (CursorStyle)CURRENT_PARAM(1, 2), (CursorStyle)CURRENT_PARAM(1, 3));

		callbackAction();
		break;

	case kActionKnock:
	case kActionOpenDoor:
		getObjects()->update((ObjectIndex)params->param1, kEntityMertens, kObjectLocation1, kCursorNormal, kCursorNormal);
		if (params->param2)
			getObjects()->update((ObjectIndex)params->param2, kEntityMertens, kObjectLocation1, kCursorNormal, kCursorNormal);

		setCallback(savepoint.action == kActionKnock ? 1 : 2);
		setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		break;

	case kActionDefault:
		params->param3 = 1;
		params->param4 = getObjects()->get((ObjectIndex)params->param1).entity;
		params->param5 = getObjects()->get((ObjectIndex)params->param1).location;
		params->param6 = getObjects()->get((ObjectIndex)params->param1).cursor;
		params->param7 = getObjects()->get((ObjectIndex)params->param1).cursor2;

		if (params->param2) {
			params->param8       = getObjects()->get((ObjectIndex)params->param2).entity;
			CURRENT_PARAM(1, 1) = getObjects()->get((ObjectIndex)params->param2).location;
			CURRENT_PARAM(1, 2) = getObjects()->get((ObjectIndex)params->param2).cursor;
			CURRENT_PARAM(1, 3) = getObjects()->get((ObjectIndex)params->param2).cursor2;

			getObjects()->update((ObjectIndex)params->param2, kEntityMertens, kObjectLocation1, kCursorHandKnock, kCursorHand);
		}

		if (params->param5 != kObjectLocation2)
			getObjects()->update((ObjectIndex)params->param1, kEntityMertens, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(params->param3 ? 3 : 4);
			setup_playSound(params->param3 ? "Con1017" : "Con1017A");
			break;

		case 3:
		case 4:
			params->param3 = 0;
			getObjects()->update((ObjectIndex)params->param1, kEntityMertens, kObjectLocation1, kCursorHandKnock, kCursorHand);

			if (params->param2)
				getObjects()->update((ObjectIndex)params->param2, kEntityMertens, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Mertens, function22)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_2740);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("601Mh", kObjectCompartment8);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityMertens, "601Nh");
			getEntities()->enterCompartment(kEntityMertens, kObjectCompartment8, true);

			setCallback(3);
			setup_function11(150);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment("601Mh", kObjectCompartment8);
			break;

		case 4:
			getEntities()->drawSequenceLeft(kEntityMertens, "601Nh");
			getEntities()->enterCompartment(kEntityMertens, kObjectCompartment8);
			getSavePoints()->push(kEntityMertens, kEntityMahmud, kAction225563840);
			break;

		case 5:
			if (!getSoundQueue()->isBuffered(kEntityMertens))
				getSound()->playSound(kEntityMertens, "MAH1170I");

			setCallback(6);
			setup_enterExitCompartment("601Zd", kObjectCompartment4);
			break;

		case 6:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMertens);
			if (!getSoundQueue()->isBuffered(kEntityMertens))
				getSound()->playSound(kEntityMertens, "MAH1172", kFlagInvalid, 225);

			setCallback(7);
			setup_function21(kObjectCompartment4, kObject20);
			break;

		case 7:
			setCallback(8);
			setup_enterExitCompartment("671Ad", kObjectCompartment4);
			break;

		case 8:
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityMertens, kEntityMahmud, kAction123852928);

			setCallback(9);
			setup_updateEntity(kCarGreenSleeping, kPosition_540);

			break;

		case 9:
			callbackAction();
			break;
		}
		break;

	case kAction102227384:
		getEntities()->drawSequenceLeft(kEntityMertens, "671Dh");
		break;

	case kAction156567128:
		getEntities()->exitCompartment(kEntityMertens, kObjectCompartment8, true);

		setCallback(5);
		setup_updateEntity(kCarGreenSleeping, kPosition_5790);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Mertens, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_5790);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("601Vd", kObjectCompartment4);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityMertens, "601Wd");
			getEntities()->enterCompartment(kEntityMertens, kObjectCompartment4, true);

			setCallback(3);
			setup_function11(150);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment("601Zd", kObjectCompartment4);
			break;

		case 4:
			getEntities()->exitCompartment(kEntityMertens, kObjectCompartment4);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMertens);

			setCallback(5);
			setup_function21(kObjectCompartment4, kObject20);
			break;

		case 5:
			setCallback(6);
			setup_enterExitCompartment("671Ad", kObjectCompartment4);
			break;

		case 6:
			getData()->location = kLocationOutsideCompartment;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Mertens, function24)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1) {
			if (!Entity::updateParameter(params->param2, getState()->timeTicks, 75))
				break;

			setCallback(3);
			setup_enterExitCompartment3("601Rc", kObjectCompartment3, kPosition_6470, kPosition_6130);
		}
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
			setup_enterExitCompartment("601Mc", kObjectCompartment3);
			break;

		case 2:
			getSavePoints()->push(kEntityMertens, kEntityAugust, kAction221617184);
			getEntities()->drawSequenceLeft(kEntityMertens, "601Nc");
			getEntities()->enterCompartment(kEntityMertens, kObjectCompartment3, true);
			break;

		case 3:
			getEntities()->exitCompartment(kEntityMertens, kObjectCompartment3, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMertens);

			setCallback(4);
			setup_function21(kObjectCompartment3, kObjectKitchen);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("601Sc", kObjectCompartment3);
			break;

		case 5:
			getData()->location = kLocationOutsideCompartment;

			callbackAction();
			break;

		case 6:
			getEntities()->exitCompartment(kEntityMertens, kObjectCompartment3, true);
			getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMertens);

			setCallback(7);
			setup_function21(kObjectCompartment3, kObjectKitchen);
			break;

		case 7:
			getObjects()->update(kObjectCompartment3, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(8);
			setup_enterExitCompartment("601Uc", kObjectCompartment3);
			break;

		case 8:
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityMertens, kEntityAugust, kAction124697504);

			setCallback(9);
			setup_updateEntity(kCarGreenSleeping, kPosition_540);
			break;

		case 9:
			callbackAction();
			break;
		}
		break;

	case kAction100906246:
		getSavePoints()->push(kEntityMertens, kEntityAugust, kAction192849856);
		getEntities()->drawSequenceLeft(kEntityMertens, "601Qc");
		break;

	case kAction102675536:
		params->param1 = 1;
		break;

	case kAction156567128:
		setCallback(6);
		setup_enterExitCompartment("601Tc", kObjectCompartment3);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Mertens, function25)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1) {
			if (!Entity::updateParameter(params->param2, getState()->timeTicks, 75))
				break;

			setCallback(3);
			setup_enterExitCompartment3("601Zb", kObjectCompartment2, kPosition_7500, kPositionNone);
		}
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_7500);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("601Vb", kObjectCompartment2);
			break;

		case 2:
			getSavePoints()->push(kEntityMertens, kEntityAlexei, kAction221617184);
			getEntities()->drawSequenceLeft(kEntityMertens, "601Wb");
			getEntities()->enterCompartment(kEntityMertens, kObjectCompartment2, true);
			break;

		case 3:
			getEntities()->exitCompartment(kEntityMertens, kObjectCompartment2, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMertens);

			if (getProgress().chapter == kChapter1 && ENTITY_PARAM(0, 4))
				if (getProgress().field_14 != 29)
					getProgress().field_14 = 3;

			setCallback(4);
			setup_function21(kObjectCompartment2, kObjectHandleInsideBathroom);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("671Ab", kObjectCompartment2);
			break;

		case 5:
			getData()->location = kLocationOutsideCompartment;

			callbackAction();
			break;

		case 6:
			getEntities()->exitCompartment(kEntityMertens, kObjectCompartment2, true);
			getObjects()->update(kObjectCompartment2, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMertens);

			if (getProgress().chapter == kChapter1 && ENTITY_PARAM(0, 4))
				if (getProgress().field_14 != 29)
					getProgress().field_14 = 3;

			setCallback(7);
			setup_function21(kObjectCompartment2, kObjectHandleInsideBathroom);
			break;

		case 7:
			getSound()->playSound(kEntityMertens, "CON1024A");
			getObjects()->update(kObjectCompartment2, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(8);
			setup_enterExitCompartment("641Ub", kObjectCompartment2);
			break;

		case 8:
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityMertens, kEntityAlexei, kAction124697504);

			setCallback(9);
			setup_updateEntity(kCarGreenSleeping, kPosition_9460);
			break;

		case 9:
			callbackAction();
			break;
		}
		break;

	case kAction100906246:
		params->param1 = 1;
		break;

	case kAction156567128:
		setCallback(6);
		setup_enterExitCompartment("641Tb", kObjectCompartment2);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(26, Mertens, function26, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (getProgress().eventCorpseThrown
		 || !params->param1
		 || getProgress().chapter != kChapter1
		 || getProgress().jacket != kJacketGreen) {

			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMertens);
			getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorNormal);

			setCallback(3);
			setup_playSound16("ZNU1001");
		} else {
			setCallback(1);
			setup_savegame(kSavegameTypeTime, kTimeNone);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorNormal);

			setCallback(2);
			setup_playSound16("CON1062");
			break;

		case 2:
			getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;

		case 3:
			if (getProgress().jacket == kJacketBlood) {
				setCallback(4);
				setup_savegame(kSavegameTypeEvent, kEventMertensBloodJacket);
			} else if (getProgress().eventCorpseMovedFromFloor) {
				getEntities()->enterCompartment(kEntityMertens, kObjectCompartment1);
				getEntities()->drawSequenceRight(kEntityMertens, "601Ra");
				getScenes()->loadSceneFromPosition(kCarGreenSleeping, 16);

				setCallback(6);
				setup_callbackActionOnDirection();
			} else {
				setCallback(5);
				setup_savegame(kSavegameTypeEvent, kEventMertensCorpseFloor);
			}
			break;

		case 4:
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			break;

		case 5:
			getAction()->playAnimation(kEventMertensCorpseFloor);
			getLogic()->gameOver(kSavegameTypeIndex, 1, getProgress().eventCorpseFound ? kSceneGameOverStopPolice : kSceneGameOverPolice, true);
			break;

		case 6:
			getEntities()->exitCompartment(kEntityMertens, kObjectCompartment1);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMertens);

			setCallback(7);
			setup_function21(kObjectCompartment1, kObjectHandleBathroom);
			break;

		case 7:
			if (getProgress().eventCorpseThrown || getProgress().chapter != kChapter1) {
				if (getEntities()->isDistanceBetweenEntities(kEntityMertens, kEntityPlayer, 1000)) {
					if (!getEntities()->checkFields10(kEntityPlayer))
						getSound()->playSound(kEntityMertens, "CON1061");
				}

				setCallback(9);
				setup_enterExitCompartment("601Sa", kObjectCompartment1);
			} else {
				if (!getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping))
					getScenes()->loadSceneFromPosition(kCarNone, 1);

				setCallback(8);
				setup_savegame(kSavegameTypeEvent, kEventMertensCorpseBed);
			}
			break;

		case 8:
			getAction()->playAnimation(kEventMertensCorpseBed);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverPolice1, true);
			break;

		case 9:
			getData()->location = kLocationOutsideCompartment;
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(27, Mertens, tylerCompartment, MertensActionType)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getProgress().field_14 == 29) {
			callbackAction();
			break;
		}

		if (Entity::updateParameter(params->param2, getState()->timeTicks, 150)) {
			getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorNormal);

			setCallback(10);
			setup_playSound16("CON1018A");
			break;
		}

label_callback10:
		if (!params->param3)
			params->param3 = getState()->timeTicks + 300;

		if (params->param3 >= getState()->timeTicks) {
label_callback11:
			if (!Entity::updateParameter(params->param4, getState()->timeTicks, 375))
				break;

			getSound()->playSound(kEntityPlayer, "LIB033");

			if (getProgress().eventCorpseMovedFromFloor) {

				if (getProgress().jacket == kJacketBlood) {
					setCallback(18);
					setup_savegame(kSavegameTypeEvent, kEventMertensBloodJacket);
					break;
				}

				if (params->param1) {
					getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

					switch (params->param1) {
					case 1:
						setCallback(20);
						setup_savegame(kSavegameTypeEvent, kEventMertensAugustWaitingCompartment);
						break;

					case 2:
						setCallback(21);
						setup_savegame(kSavegameTypeEvent, kEventMertensKronosInvitationCompartment);
						break;

					case 3:
						getAction()->playAnimation(isNight() ? kEventMertensPushCallNight : kEventMertensPushCall);
						// fallback to default case

					default:
						getSound()->playSound(kEntityPlayer, "LIB015");
						getScenes()->loadScene(kScene41);

						callbackAction();
						break;
					}
				} else {
					setCallback(26);
					setup_function26(false);
				}

			} else {
				if (!getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping))
					getScenes()->loadSceneFromPosition(kCarNone, 1);

				setCallback(17);
				setup_savegame(kSavegameTypeEvent, kEventMertensCorpseFloor);
			}
		} else {
			params->param3 = kTimeInvalid;

			if (getObjects()->get(kObjectCompartment1).location == kObjectLocation1) {
				getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);

				setCallback(11);
				setup_playSound16("CON1018B");
				break;
			}

			getSound()->playSound(kEntityPlayer, "LIB014");

			if (getProgress().eventCorpseMovedFromFloor) {

				if (getProgress().jacket == kJacketBlood) {
					setCallback(13);
					setup_savegame(kSavegameTypeEvent, kEventMertensBloodJacket);
					break;
				}

				if (params->param1) {
					getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

					switch (params->param1) {
					case 1:
						setCallback(15);
						setup_savegame(kSavegameTypeEvent, kEventMertensAugustWaitingCompartment);
						break;

					case 2:
						setCallback(16);
						setup_savegame(kSavegameTypeEvent, kEventMertensKronosInvitationCompartment);
						break;

					case 3:
						getAction()->playAnimation(isNight() ? kEventMertensPushCallNight : kEventMertensPushCall);
						// fallback to default case

					default:
						getSound()->playSound(kEntityPlayer, "LIB015");
						getScenes()->loadScene(kScene41);

						callbackAction();
						break;
					}
				} else {
					setCallback(14);
					setup_function26(false);
				}
			} else {
				if (!getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping))
					getScenes()->loadSceneFromPosition(kCarNone, 1);

				setCallback(12);
				setup_savegame(kSavegameTypeEvent, kEventMertensCorpseFloor);
			}
		}
		break;

	case kActionKnock:
		if (params->param1) {
			getObjects()->update(kObjectCompartment1, kEntityMertens, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorNormal);

			switch (params->param1) {
			default:
				getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorHandKnock, kCursorHand);

				callbackAction();
				break;

			case 1:
				setCallback(23);
				setup_playSound16("CON1018D");
				break;

			case 2:
				setCallback(24);
				setup_playSound16("CON1018E");
				break;

			case 3:
				setCallback(25);
				setup_playSound16("CON1025");
				break;
			}

		} else {
			setCallback(22);
			setup_function26(true);
		}
		break;

	case kActionOpenDoor:
		getSound()->playSound(kEntityPlayer, getObjects()->get(kObjectCompartment1).location == kObjectLocation1 ? "LIB012" : "LIB014");

		if (getProgress().eventCorpseMovedFromFloor) {

			if (getProgress().jacket == kJacketBlood) {
				setCallback(27);
				setup_savegame(kSavegameTypeEvent, kEventMertensBloodJacket);
				break;
			}

			if (params->param1) {
				getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

				switch (params->param1) {
				case 1:
					setCallback(29);
					setup_savegame(kSavegameTypeEvent, kEventMertensAugustWaitingCompartment);
					break;

				case 2:
					setCallback(30);
					setup_savegame(kSavegameTypeEvent, kEventMertensKronosInvitationCompartment);
					break;

				case 3:
					getAction()->playAnimation(isNight() ? kEventMertensPushCallNight : kEventMertensPushCall);
					// fallback to default case

				default:
					getSound()->playSound(kEntityPlayer, "LIB015");
					getScenes()->loadScene(kScene41);

					callbackAction();
					break;
				}
			} else {
				setCallback(28);
				setup_function26(false);
			}
		} else {
			if (!getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping))
				getScenes()->loadSceneFromPosition(kCarNone, 1);

			setCallback(26);
			setup_savegame(kSavegameTypeEvent, kEventMertensCorpseFloor);
		}
		break;

	case kActionDefault:
		getData()->inventoryItem = kItemNone;

		if (getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_8200)
		 || getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_7850)
		 || getEntities()->isOutsideAlexeiWindow()) {
			getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorNormal, kCursorNormal);

			if (getEntities()->isOutsideAlexeiWindow())
				getScenes()->loadSceneFromPosition(kCarGreenSleeping, 49);

			setCallback(params->param1 ? 9 : 8);
			setup_playSound16(params->param1 ? "CON1018" : "CON1060");
		} else {
			getSound()->playSound(kEntityMertens, "CON1019");

			setCallback(1);
			setup_enterExitCompartment("601Ma", kObjectCompartment1);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getProgress().eventCorpseMovedFromFloor) {
				setCallback(4);
				setup_enterExitCompartment("601Ra", kObjectCompartment1);
			} else {
				if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping)) {
					setCallback(2);
					setup_enterExitCompartment("601Ra", kObjectCompartment1);
				} else {
					getScenes()->loadSceneFromPosition(kCarNone, 1);

					setCallback(3);
					setup_savegame(kSavegameTypeEvent, kEventMertensCorpseFloor);
				}
			}
			break;

		case 2:
			setCallback(3);
			setup_savegame(kSavegameTypeEvent, kEventMertensCorpseFloor);
			break;

		case 3:
		case 12:
		case 17:
		case 26:
			getAction()->playAnimation(kEventMertensCorpseFloor);
			getLogic()->gameOver(kSavegameTypeIndex, 1, getProgress().eventCorpseFound ? kSceneGameOverStopPolice : kSceneGameOverPolice, true);
			break;

		case 4:
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityMertens);

			if (params->param1) {
				setCallback(7);
				setup_enterExitCompartment("601Sa", kObjectCompartment1);
				break;
			}

			if (getProgress().eventCorpseThrown || getProgress().chapter != kChapter1) {
				setCallback(6);
				setup_function21(kObjectCompartment1, kObjectHandleBathroom);
			} else {
				if (!getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping))
					getScenes()->loadSceneFromPosition(kCarNone, 1);

				setCallback(5);
				setup_savegame(kSavegameTypeEvent, kEventMertensCorpseBed);
			}
			break;

		case 5:
			getAction()->playAnimation(kEventMertensCorpseBed);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverPolice1, true);
			break;

		case 6:
			setCallback(7);
			setup_enterExitCompartment("601Sa", kObjectCompartment1);
			break;

		case 7:
			getData()->location = kLocationOutsideCompartment;
			getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;

		case 8:
		case 9:
			getObjects()->update(kObjectCompartment1, kEntityMertens, getObjects()->get(kObjectCompartment1).location, kCursorTalk, kCursorHand);
			break;

		case 10:
			getObjects()->update(kObjectCompartment1, kEntityMertens, getObjects()->get(kObjectCompartment1).location, kCursorTalk, kCursorHand);
			goto label_callback10;

		case 11:
			getObjects()->update(kObjectCompartment1, kEntityMertens, getObjects()->get(kObjectCompartment1).location, kCursorTalk, kCursorHand);
			goto label_callback11;

		case 13:
		case 18:
		case 27:
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			break;

		case 14:
		case 19:
		case 22:
		case 28:
			callbackAction();
			break;

		case 15:
		case 20:
		case 29:
			getAction()->playAnimation(kEventMertensAugustWaitingCompartment);
			getProgress().eventMertensAugustWaiting = true;

			getSound()->playSound(kEntityPlayer, "LIB015");
			getScenes()->loadScene(kScene41);

			callbackAction();
			break;

		case 16:
		case 21:
		case 30:
			getAction()->playAnimation(kEventMertensKronosInvitationCompartment);
			getProgress().eventMertensKronosInvitation = true;

			getSound()->playSound(kEntityPlayer, "LIB015");
			getScenes()->loadScene(kScene41);

			callbackAction();
			break;

		case 23:
			getProgress().eventMertensAugustWaiting = true;
			getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;

		case 24:
			getProgress().eventMertensKronosInvitation = true;
			getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;

		case 25:
			getObjects()->update(kObjectCompartment1, kEntityPlayer, getObjects()->get(kObjectCompartment1).location, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(28, Mertens, function28)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param4 && params->param5) {
			getSavePoints()->push(kEntityMertens, kEntityCoudert, kAction125499160);

			setCallback(3);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
		}
		break;

	case kActionEndSound:
		params->param4 = 1;
		break;

	case kActionDefault:
		setCallback(1);
		setup_function19();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_1500);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityMertens, "601O");
			getSavePoints()->push(kEntityMertens, kEntityCoudert, kAction154005632);
			break;

		case 3:
			setCallback(4);
			setup_function17();
			break;

		case 4:
			callbackAction();
			break;
		}
		break;

	case kAction155853632:
		params->param5 = 1;
		break;

	case kAction202558662:
		getEntities()->drawSequenceLeft(kEntityMertens, "601L");
		getSound()->playSound(kEntityMertens, (char *)&params->seq1);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SS(29, Mertens, function29)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param7 > 1 && params->param8) {
			getSavePoints()->push(kEntityMertens, kEntityCoudert, kAction125499160);

			setCallback(3);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
		}
		break;

	case kActionEndSound:
		params->param7++;
		if (params->param7 == 1)
			getSound()->playSound(kEntityMertens, (char *)&params->seq2);
		break;

	case kActionDefault:
		setCallback(1);
		setup_function19();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_1500);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityMertens, "601O");
			getSavePoints()->push(kEntityMertens, kEntityCoudert, kAction154005632);
			break;

		case 3:
			setCallback(4);
			setup_function17();
			break;

		case 4:
			callbackAction();
			break;
		}
		break;

	case kAction155853632:
		params->param8 = 1;
		break;

	case kAction202558662:
		getEntities()->drawSequenceLeft(kEntityMertens, "601L");
		getSound()->playSound(kEntityMertens, (char *)&params->seq1);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(30, Mertens, function30, MertensActionType)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		switch (params->param1) {
		default:
			callbackAction();
			return;

		case 1:
			params->param2 = kPosition_8200;

			if (getProgress().field_14) {
				callbackAction();
				return;
			}

			getProgress().field_14 = 3;
			break;

		case 2:
			params->param2 = kPosition_7500;
			break;

		case 3:
			params->param2 = kPosition_6470;
			break;
		}

		setCallback(1);
		setup_function19();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, (EntityPosition)params->param2);
			break;

		case 2:
			switch (params->param1) {
			default:
				if (getProgress().field_14 == 3)
					getProgress().field_14 = 0;

				setCallback(8);
				setup_updateEntity(kCarGreenSleeping, kPosition_2000);
				break;

			case 1:
				if (getProgress().chapter == kChapter4)
					getSavePoints()->push(kEntityMertens, kEntityTatiana, kAction238790488);

				setCallback(3);
				setup_tylerCompartment(kMertensAction3);
				break;

			case 2:
				if (getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_7500)) {
					getObjects()->update(kObjectCompartment2, kEntityPlayer, getObjects()->get(kObjectCompartment2).location, kCursorNormal, kCursorNormal);
					params->param3 = 1;
				}

				setCallback(4);
				setup_enterExitCompartment("601Vb", kObjectCompartment2);
				break;

			case 3:
				if (getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_6470)) {
					getObjects()->update(kObjectCompartment3, kEntityPlayer, getObjects()->get(kObjectCompartment3).location, kCursorNormal, kCursorNormal);
					params->param3 = 1;
				}

				setCallback(6);
				setup_enterExitCompartment("601Mc", kObjectCompartment3);
				break;
			}
			break;

		case 3:
			if (getProgress().field_14 == 3)
					getProgress().field_14 = 0;

			setCallback(8);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 4:
			getEntities()->drawSequenceLeft(kEntityMertens, "601Wb");
			getEntities()->enterCompartment(kEntityMertens, kObjectCompartment2, true);

			setCallback(5);
			setup_playSound("CON3020");
			break;

		case 5:
			if (params->param3)
				getObjects()->update(kObjectCompartment2, kEntityPlayer, getObjects()->get(kObjectCompartment2).location, kCursorHandKnock, kCursorHand);

			getEntities()->exitCompartment(kEntityMertens, kObjectCompartment2);

			if (getProgress().field_14 == 3)
					getProgress().field_14 = 0;

			setCallback(8);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 6:
			getEntities()->drawSequenceLeft(kEntityMertens, "601Nc");
			getEntities()->enterCompartment(kEntityMertens, kObjectCompartment3, true);

			setCallback(7);
			setup_playSound("CON3020");
			break;

		case 7:
			if (params->param3)
				getObjects()->update(kObjectCompartment3, kEntityPlayer, getObjects()->get(kObjectCompartment3).location, kCursorHandKnock, kCursorHand);

			getEntities()->exitCompartment(kEntityMertens, kObjectCompartment3);

			if (getProgress().field_14 == 3)
					getProgress().field_14 = 0;

			setCallback(8);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 8:
			setCallback(9);
			setup_function17();
			break;

		case 9:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(31, Mertens, function31, MertensActionType)
	switch (savepoint.action) {
	default:
		break;

	case kActionEndSound:
		setCallback(3);
		setup_function17();
		break;

	case kActionDefault:
		setCallback(1);
		setup_bloodJacket("601G");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getSoundQueue()->isBuffered(kEntityMertens)) {
				getEntities()->drawSequenceLeft(kEntityMertens, "601J");
			} else {
				setCallback(2);
				setup_function17();
			}
			break;

		case 2:
		case 3:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Mertens, function32)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function19();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_9510);
			break;

		case 2:
			if (getData()->entityPosition >= kPosition_9460) {
				getEntities()->clearSequences(kEntityMertens);
				setCallback(3);
				setup_function11(900);
				break;
			}
			// Fallback to next case

		case 3:
			setCallback(4);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 4:
			setCallback(5);
			setup_function17();
			break;

		case 5:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Mertens, function33)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (ENTITY_PARAM(0, 8) || ENTITY_PARAM(0, 6)
		 || ENTITY_PARAM(1, 1) || ENTITY_PARAM(1, 2) || ENTITY_PARAM(1, 3) || ENTITY_PARAM(1, 4) || ENTITY_PARAM(1, 5) || ENTITY_PARAM(1, 6) || ENTITY_PARAM(1, 7)
		 || ENTITY_PARAM(2, 2)) {
			ENTITY_PARAM(1, 8) = 1;

			setCallback(ENTITY_PARAM(0, 8) ? 1 : 3);
			setup_updateEntity(kCarGreenSleeping, ENTITY_PARAM(0, 8) ? kPosition_1500 : kPosition_540);
		} else {
			callbackAction();
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			ENTITY_PARAM(2, 1) = 1;

			setCallback(2);
			setup_function14(kEntityVerges);
			break;

		case 2:
			ENTITY_PARAM(1, 8) = 0;

			callbackAction();
			break;

		case 3:
			getEntities()->clearSequences(kEntityMertens);

			setCallback(4);
			setup_function11(75);
			break;

		case 4:
			if (ENTITY_PARAM(1, 6)) {
				setCallback(5);
				setup_function16(true);
				break;
			}
			// Fallback to next case

		case 5:
			if (ENTITY_PARAM(1, 7)) {
				setCallback(6);
				setup_function16(false);
				break;
			}
			// Fallback to next case

		case 6:
			if (ENTITY_PARAM(1, 5)) {
				setCallback(7);
				setup_function15(true);
				break;
			}
			// Fallback to next case

		case 7:
			if (ENTITY_PARAM(1, 4)) {
				setCallback(8);
				setup_function15(false);
				break;
			}
			// Fallback to next case

		case 8:
			if (ENTITY_PARAM(1, 2)) {
				setCallback(9);
				setup_function35();
				break;
			}
			// Fallback to next case

		case 9:
			if (ENTITY_PARAM(0, 6)) {
				setCallback(10);
				setup_function36();
				break;
			}
			// Fallback to next case

		case 10:
			if (ENTITY_PARAM(1, 3)) {
				setCallback(11);
				setup_function40();
				break;
			}
			// Fallback to next case

		case 11:
			if (ENTITY_PARAM(1, 1)) {
				setCallback(12);
				setup_function28("CON1200");
				break;
			}

			if (ENTITY_PARAM(2, 2)) {
				setCallback(13);
				setup_function37();
				break;
			}

			callbackAction();
			break;

		case 12:
			getSavePoints()->push(kEntityMertens, kEntityCoudert, kAction168254872);
			ENTITY_PARAM(1, 1) = 0;

			if (ENTITY_PARAM(2, 2)) {
				setCallback(13);
				setup_function37();
				break;
			}

			callbackAction();
			break;

		case 13:
			ENTITY_PARAM(2, 2) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(34, Mertens, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Mertens, setup_chapter1Handler));
		break;

	case kActionDefault:
		getSavePoints()->addData(kEntityMertens, kAction171394341, 7);
		getSavePoints()->addData(kEntityMertens, kAction169633856, 9);
		getSavePoints()->addData(kEntityMertens, kAction238732837, 10);
		getSavePoints()->addData(kEntityMertens, kAction269624833, 12);
		getSavePoints()->addData(kEntityMertens, kAction302614416, 11);
		getSavePoints()->addData(kEntityMertens, kAction190082817, 8);
		getSavePoints()->addData(kEntityMertens, kAction269436673, 13);
		getSavePoints()->addData(kEntityMertens, kAction303343617, 14);
		getSavePoints()->addData(kEntityMertens, kAction224122407, 17);
		getSavePoints()->addData(kEntityMertens, kAction201431954, 18);
		getSavePoints()->addData(kEntityMertens, kAction188635520, 19);
		getSavePoints()->addData(kEntityMertens, kAction204379649, 4);

		ENTITY_PARAM(0, 1) = 0;

		getData()->entityPosition = kPosition_9460;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(35, Mertens, function35)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (getProgress().field_14 == 29) {
			callbackAction();
			break;
		} else {
			getProgress().field_14 = 3;

			setCallback(1);
			setup_function19();
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
			if (!ENTITY_PARAM(1, 2) || getProgress().eventMetAugust) {
				ENTITY_PARAM(1, 2) = 0;

				if (getProgress().field_14 == 3)
					getProgress().field_14 = 0;

				setCallback(3);
				setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			} else {
				setCallback(5);
				setup_tylerCompartment(kMertensAction1);
			}
			break;

		case 3:
			setCallback(4);
			setup_function17();
			break;

		case 4:
			callbackAction();
			break;

		case 5:
			if (getProgress().field_14 == 3)
				getProgress().field_14 = 0;

			if (getProgress().eventMertensAugustWaiting)
				ENTITY_PARAM(1, 2) = 0;

			setCallback(6);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 6:
			ENTITY_PARAM(1, 2) = 0;

			setCallback(7);
			setup_function17();
			break;

		case 7:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(36, Mertens, function36)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (getProgress().field_14 == 29) {
			callbackAction();
		} else {
			getProgress().field_14 = 3;

			setCallback(1);
			setup_function19();
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
			if (ENTITY_PARAM(0, 6)) {
				if (getEntities()->isPlayerInCar(kCarGreenSleeping) && getData()->entityPosition < getEntityData(kEntityPlayer)->entityPosition) {
					setCallback(3);
					setup_updateEntity(kCarGreenSleeping, kPosition_9460);
				} else {
					setCallback(7);
					setup_tylerCompartment(kMertensAction2);
				}
			} else {
				ENTITY_PARAM(0, 6) = 0;
				ENTITY_PARAM(0, 7) = 0;

				if (getProgress().field_14 == 3)
					getProgress().field_14 = 0;

				setCallback(5);
				setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			}
			break;

		case 3:
			setCallback(4);
			setup_updateEntity(kCarGreenSleeping, kPosition_8200);
			break;

		case 4:
			if (ENTITY_PARAM(0, 6)) {
				setCallback(7);
				setup_tylerCompartment(kMertensAction2);
			} else {
				ENTITY_PARAM(0, 6) = 0;
				ENTITY_PARAM(0, 7) = 0;

				if (getProgress().field_14 == 3)
					getProgress().field_14 = 0;

				setCallback(5);
				setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			}
			break;

		case 5:
			setCallback(6);
			setup_function17();
			break;

		case 7:
			if (getProgress().field_14 == 3)
				getProgress().field_14 = 0;

			if (!getProgress().eventMertensKronosInvitation)
				ENTITY_PARAM(0, 7) = 1;

			ENTITY_PARAM(0, 6) = 0;

			setCallback(8);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 8:
			setCallback(9);
			setup_function17();
			break;

		case 6:
		case 9:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(37, Mertens, function37)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 >= 2 && params->param2) {
			getSavePoints()->push(kEntityMertens, kEntityCoudert, kAction125499160);

			setCallback(3);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
		}
		break;

	case kActionEndSound:
		++params->param6;

		if (params->param6 == 1)
			getSound()->playSound(kEntityMertens, getEntities()->isDistanceBetweenEntities(kEntityMertens, kEntityPlayer, 2000) ? "CON1152" : "CON1151");
		break;

	case kActionDefault:
		setCallback(1);
		setup_function19();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(1);
			setup_updateEntity(kCarRedSleeping, kPosition_1500);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityMertens, "601O");
			getSavePoints()->push(kEntityMertens, kEntityCoudert, kAction154005632);
			break;

		case 3:
			setCallback(4);
			setup_function17();
			break;

		case 4:
			callbackAction();
			break;
		}
		break;

	case kAction155853632:
		params->param2 = 1;
		break;

	case kAction202558662:
		getEntities()->drawSequenceLeft(kEntityMertens, "601L");
		getSound()->playSound(kEntityMertens, "CON1150");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(38, Mertens, function38)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (!ENTITY_PARAM(0, 4)) {
			callbackAction();
			break;
		}

		if (getProgress().field_14 == 29) {
			callbackAction();
		} else {
			setCallback(1);
			setup_updateEntity(kCarGreenSleeping, kPosition_8200);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (!ENTITY_PARAM(0, 4)) {
				callbackAction();
				break;
			}

			setCallback(2);
			setup_tylerCompartment(kMertensActionNone);
			break;

		case 2:
			ENTITY_PARAM(0, 4) = 0;
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(39, Mertens, function39)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		ENTITY_PARAM(0, 4) = 1;

		setCallback(1);
		setup_function19();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function22();
			break;

		case 2:
			setCallback(3);
			setup_function33();
			break;

		case 3:
			setCallback(4);
			setup_function24();
			break;

		case 4:
			setCallback(5);
			setup_function33();
			break;

		case 5:
			setCallback(6);
			setup_function25();
			break;

		case 6:
			setCallback(7);
			setup_function33();
			break;

		case 7:
			setCallback(8);
			setup_function38();
			break;

		case 8:
			if (getProgress().field_14 == 3)
				getProgress().field_14 = 0;

			setCallback(9);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 9:
			setCallback(10);
			setup_function17();
			break;

		case 10:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(40, Mertens, function40)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		ENTITY_PARAM(1, 3) = 0;
		setCallback(1);
		setup_function19();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarKronos, kPosition_9460);
			break;

		case 2:
			setCallback(3);
			setup_function11(1800);
			break;

		case 3:
			setCallback(4);
			setup_updateEntity(kCarGreenSleeping, kPosition_1500);
			break;

		case 4:
			setCallback(5);
			setup_function17();
			break;

		case 5:
			ENTITY_PARAM(0, 6) = 1;
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(41, Mertens, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_2000);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function17();
			break;

		case 2:
			setup_function42();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(42, Mertens, function42)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(2, 3)) {
			ENTITY_PARAM(0, 1) = 1;
			ENTITY_PARAM(0, 6) = 0;
			ENTITY_PARAM(0, 7) = 0;
			ENTITY_PARAM(0, 8) = 0;

			ENTITY_PARAM(1, 1) = 0;
			ENTITY_PARAM(1, 2) = 0;
			ENTITY_PARAM(1, 3) = 0;
			ENTITY_PARAM(1, 4) = 0;
			ENTITY_PARAM(1, 5) = 0;
			ENTITY_PARAM(1, 6) = 0;
			ENTITY_PARAM(1, 7) = 0;

			ENTITY_PARAM(2, 1) = 0; // BUG: is set twice. Maybe a bug?
			ENTITY_PARAM(2, 2) = 0;
			ENTITY_PARAM(2, 3) = 0;

			params->param1 = 1;
			params->param2 = 1;

			getEntities()->drawSequenceLeft(kEntityMertens, "601E");
		}

		if (ENTITY_PARAM(2, 1) || getProgress().eventCorpseFound || getEvent(kEventMertensAskTylerCompartmentD) || getEvent(kEventMertensAskTylerCompartment))
			getData()->inventoryItem = kItemNone;
		else
			getData()->inventoryItem = kItemInvalid;

		if (!params->param2) {
			Entity::timeCheckSavepoint(kTime1125000, params->param3, kEntityMertens, kEntityMahmud, kAction170483072);

			if (params->param4 != kTimeInvalid && getState()->time > kTimeCityChalons) {

				if (getState()->time <= kTime1188000) {
					if ((!getEntities()->isPlayerInCar(kCarGreenSleeping) && !getEntities()->isPlayerInCar(kCarRedSleeping))
					  || getSoundQueue()->isBuffered("REB1205")
					  || !getEntities()->isInsideCompartment(kEntityMmeBoutarel, kCarRedSleeping, kPosition_5790)
					  || !params->param4) {
						params->param4 = (uint)getState()->time;
					}

					if (params->param4 >= getState()->time)
						break;
				}

				ENTITY_PARAM(0, 4) = kTimeInvalid;
				getData()->inventoryItem = kItemNone;

				setCallback(8);
				setup_function29("CON1210", "CON1210A");
				break;
			}
		}

label_callback_8:
		if (getState()->time > kTime1215000 && !ENTITY_PARAM(0, 1) && !ENTITY_PARAM(2, 1)) {
			if (Entity::updateParameter(params->param5, getState()->time, 2700)) {
				getEntities()->drawSequenceLeft(kEntityMertens, "601E");
				ENTITY_PARAM(0, 1) = 1;
				params->param5 = 0;
			}
		}

		if (ENTITY_PARAM(0, 8)) {
			getData()->inventoryItem = kItemNone;
			setCallback(9);
			setup_function14(kEntityVerges);
			break;
		}

		if (getProgress().field_14 == 29)
			goto label_callback_13;

label_callback_9:
		if (ENTITY_PARAM(1, 6)) {
			getData()->inventoryItem = kItemNone;
			setCallback(10);
			setup_function16(true);
			break;
		}

label_callback_10:
		if (ENTITY_PARAM(1, 7)) {
			getData()->inventoryItem = kItemNone;
			setCallback(11);
			setup_function16(false);
			break;
		}

label_callback_11:
		if (ENTITY_PARAM(1, 5)) {
			getData()->inventoryItem = kItemNone;
			setCallback(12);
			setup_function15(true);
			break;
		}

label_callback_12:
		if (ENTITY_PARAM(1, 4)) {
			getData()->inventoryItem = kItemNone;
			setCallback(13);
			setup_function15(false);
			break;
		}

label_callback_13:
		if (ENTITY_PARAM(1, 2)) {
			getData()->inventoryItem = kItemNone;
			setCallback(14);
			setup_function35();
			break;
		}

label_callback_14:
		if (ENTITY_PARAM(0, 6)) {
			getData()->inventoryItem = kItemNone;
			setCallback(15);
			setup_function36();
			break;
		}

label_callback_15:
		if (ENTITY_PARAM(1, 3)) {
			getData()->inventoryItem = kItemNone;
			setCallback(16);
			setup_function40();
			break;
		}

label_callback_16:
		if (ENTITY_PARAM(1, 1)) {
			ENTITY_PARAM(1, 1) = 0;
			getData()->inventoryItem = kItemNone;
			setCallback(17);
			setup_function28("CON1200");
			break;
		}

label_callback_17:
		if (ENTITY_PARAM(2, 2)) {
			ENTITY_PARAM(2, 2) = 0;
			getData()->inventoryItem = kItemNone;
			setCallback(18);
			setup_function37();
			break;
		}

label_callback_18:
		if (!params->param1 && ENTITY_PARAM(0, 5)) {
			getData()->inventoryItem = kItemNone;
			setCallback(19);
			setup_function39();
			break;
		}

label_callback_19:
		if (ENTITY_PARAM(0, 1) && !getSoundQueue()->isBuffered(kEntityMertens)) {
			if (getProgress().field_18 != 4)
				getSound()->playSound(kEntityMertens, "CON1505");
		}
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;
		setCallback(21);
		setup_savegame(kSavegameTypeEvent, kEventMertensAskTylerCompartmentD);
		break;

	case kAction11:
		if (!ENTITY_PARAM(0, 1) && !ENTITY_PARAM(2, 1)) {
			getData()->inventoryItem = kItemNone;
			setCallback(20);
			setup_function13((bool)savepoint.param.intValue, (bool)savepoint.entity2);
		}
		break;

	case kActionDefault:
		getData()->car = kCarGreenSleeping;
		getData()->entityPosition = kPosition_1500;
		getData()->location = kLocationOutsideCompartment;
		getScenes()->loadSceneFromItemPosition(kItem7);
		break;

	case kActionDrawScene:
		if (ENTITY_PARAM(2, 1))
			break;

		if (getEntities()->isPlayerPosition(kCarGreenSleeping, 23) && ENTITY_PARAM(0, 7) && !getEvent(kEventKronosConversation)) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventMertensKronosInvitation);
			break;
		}

		if (getEntities()->isPlayerPosition(kCarGreenSleeping, 23) && !getProgress().eventMertensKronosInvitation && !getEvent(kEventMertensLastCar) && !getEvent(kEventMertensLastCarOriginalJacket)) {
			setCallback(3);
			setup_savegame(kSavegameTypeEvent, kEventMertensLastCar);
			break;
		}

label_callback_2_4:
		if ((getEntities()->isPlayerPosition(kCarGreenSleeping, 1) || getEntities()->isPlayerPosition(kCarGreenSleeping, 23)) && !ENTITY_PARAM(0, 1) && !ENTITY_PARAM(2, 1)) {
			getData()->inventoryItem = kItemNone;
			setCallback(getEntities()->isPlayerPosition(kCarGreenSleeping, 1) ? 5 : 6);
			setup_function13(getEntities()->isPlayerPosition(kCarGreenSleeping, 1), false);
			break;
		}

label_callback_5_6:
		if (getEntities()->isPlayerInCar(kCarGreenSleeping) && getData()->entityPosition < getEntityData(kEntityPlayer)->entityPosition) {
			if (getProgress().jacket == kJacketOriginal || ENTITY_PARAM(0, 7)) {
				getData()->inventoryItem = kItemNone;
				setCallback(7);
				setup_function32();
			}
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventMertensKronosInvitation);
			getProgress().eventMertensKronosInvitation = true;
			ENTITY_PARAM(0, 6) = 0;
			ENTITY_PARAM(0, 7) = 0;
			getEntities()->drawSequenceRight(kEntityMertens, "601A");
			getScenes()->loadSceneFromItemPosition(kItem7);
			ENTITY_PARAM(0, 1) = 0;
			getData()->inventoryItem = kItemNone;

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
		case 4:
			getEntities()->drawSequenceLeft(kEntityMertens, "601B");
			goto label_callback_2_4;

		case 3:
			getAction()->playAnimation(getProgress().jacket == kJacketOriginal ? kEventMertensLastCarOriginalJacket : kEventMertensLastCar);
			getEntities()->drawSequenceRight(kEntityMertens, "601A");
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 6);
			getScenes()->loadSceneFromItemPosition(kItem7);
			ENTITY_PARAM(0, 1) = 0;
			getData()->inventoryItem = kItemNone;

			setCallback(4);
			setup_callbackActionOnDirection();
			break;

		case 5:
		case 6:
			goto label_callback_5_6;

		case 8:
			goto label_callback_8;

		case 9:
			goto label_callback_9;

		case 10:
			goto label_callback_10;

		case 11:
			goto label_callback_11;

		case 12:
			goto label_callback_12;

		case 13:
			goto label_callback_13;

		case 14:
			goto label_callback_14;

		case 15:
			goto label_callback_15;

		case 16:
			goto label_callback_16;

		case 17:
			goto label_callback_17;

		case 18:
			goto label_callback_18;

		case 19:
			params->param1 = 1;
			goto label_callback_19;

		case 21:
			getAction()->playAnimation(kEventMertensAskTylerCompartmentD);
			getEntities()->drawSequenceRight(kEntityMertens, "601A");
			getInventory()->get(kItem7)->location = kObjectLocationNone;
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 25);

			setCallback(22);
			setup_callbackActionOnDirection();
			break;

		case 22:
			getEntities()->drawSequenceLeft(kEntityMertens, "601B");
			break;
		}
		break;

	case kAction225358684:
		if (!ENTITY_PARAM(0, 1)) {
			getData()->inventoryItem = kItemNone;
			setCallback(23);
			setup_function30((MertensActionType)savepoint.param.intValue);
		}
		break;

	case kAction225932896:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1))
			getSavePoints()->push(kEntityMertens, kEntityFrancois, kAction205346192);
		break;

	case kAction305159806:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			getData()->inventoryItem = kItemNone;
			setCallback(24);
			setup_function31((MertensActionType)savepoint.param.intValue);
		}
		break;

	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(43, Mertens, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setCallback(1);
		setup_function17();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMertens);

		getData()->entityPosition = kPosition_1500;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 6) = 0;
		ENTITY_PARAM(0, 8) = 0;

		ENTITY_PARAM(0, 1) = 0;
		ENTITY_PARAM(0, 2) = 0;
		ENTITY_PARAM(0, 3) = 0;
		ENTITY_PARAM(0, 4) = 0;
		ENTITY_PARAM(0, 5) = 0;
		ENTITY_PARAM(0, 6) = 0;
		ENTITY_PARAM(0, 7) = 0;
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function44();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(44, Mertens, function44)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(1, 6)) {
			setCallback(1);
			setup_function16(true);
			break;
		}

label_callback1:
		if (ENTITY_PARAM(1, 7)) {
			setCallback(2);
			setup_function16(false);
			break;
		}

label_callback2:
		if (ENTITY_PARAM(1, 5)) {
			setCallback(3);
			setup_function15(true);
			break;
		}

label_callback3:
		if (ENTITY_PARAM(1, 4)) {
			setCallback(4);
			setup_function15(false);
			break;
		}

		break;

	case kAction11:
		if (!ENTITY_PARAM(2, 1)) {
			setCallback(5);
			setup_function13((bool)savepoint.param.intValue, (bool)savepoint.entity2);
		}
		break;

	case kActionDrawScene:
		if (ENTITY_PARAM(2, 1))
			break;

		if (getEntities()->isPlayerPosition(kCarGreenSleeping, 1)) {
			setCallback(6);
			setup_function13(true, false);

		} else if (getEntities()->isPlayerPosition(kCarGreenSleeping, 23)) {
			setCallback(7);
			setup_function13(false, false);
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
			goto label_callback3;
		}
		break;

	case kAction225358684:
		if (!ENTITY_PARAM(0, 1)) {
			setCallback(9);
			setup_function30((MertensActionType)savepoint.param.intValue);
		}
		break;

	case kAction225932896:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1))
			getSavePoints()->push(kEntityMertens, kEntityFrancois, kAction205346192);
		break;

	case kAction226078300:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			setCallback(8);
			setup_playSound("CON2020");
		}
		break;

	case kAction305159806:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			setCallback(10);
			setup_function31((MertensActionType)savepoint.param.intValue);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(45, Mertens, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setCallback(1);
		setup_function17();
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_1500;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 6) = 0;
		ENTITY_PARAM(0, 8) = 0;

		ENTITY_PARAM(1, 1) = 0;
		ENTITY_PARAM(1, 2) = 0;
		ENTITY_PARAM(1, 3) = 0;
		ENTITY_PARAM(1, 4) = 0;
		ENTITY_PARAM(1, 5) = 0;
		ENTITY_PARAM(1, 6) = 0;
		ENTITY_PARAM(1, 7) = 0;

		ENTITY_PARAM(2, 3) = 0;
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function46();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(46, Mertens, function46)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(1, 6)) {
			setCallback(1);
			setup_function16(true);
			break;
		}

label_callback_1:
		if (ENTITY_PARAM(1, 7)) {
			setCallback(2);
			setup_function16(false);
			break;
		}

label_callback_2:
		if (ENTITY_PARAM(1, 5)) {
			setCallback(3);
			setup_function15(true);
			break;
		}

label_callback_3:
		if (ENTITY_PARAM(1, 4)) {
			setCallback(4);
			setup_function15(false);
			break;
		}

label_callback_4:
		if (ENTITY_PARAM(0, 8)) {
			setCallback(5);
			setup_function14(kEntityVerges);
			break;
		}

label_callback_5:
		if (ENTITY_PARAM(2, 4)
		 && (getEvent(kEventKronosVisit) || getState()->time > kTime2052000)
		 && getState()->time < kTime2133000
		 && getEntities()->isPlayerInCar(kCarGreenSleeping)) {
			setCallback(6);
			setup_function32();
			break;
		}

label_callback_6:
		if (Entity::timeCheckCallback(kTime1971000, params->param1, 7, "CON3012", WRAP_SETUP_FUNCTION_S(Mertens, setup_function28)))
			break;

label_callback_7:
		if (Entity::timeCheckCallback(kTime2117700, params->param2, 8, WRAP_SETUP_FUNCTION(Mertens, setup_function32)))
			break;

label_callback_8:
		if (Entity::timeCheckCallback(kTime2124000, params->param3, 9, "CON2010", WRAP_SETUP_FUNCTION_S(Mertens, setup_function28)))
			break;

label_callback_9:
		if (Entity::timeCheckCallback(kTime2146500, params->param4, 10, WRAP_SETUP_FUNCTION(Mertens, setup_function32)))
			break;

label_callback_10:
		Entity::timeCheckCallback(kTime2169000, params->param5, 11, WRAP_SETUP_FUNCTION(Mertens, setup_function32));
		break;

	case kAction11:
		if (!ENTITY_PARAM(2, 1)) {
			setCallback(12);
			setup_function13((bool)savepoint.param.intValue, savepoint.entity2 != kEntityPlayer);
		}
		break;

	case kActionDefault:
		break;

	case kActionDrawScene:
		if (!ENTITY_PARAM(2, 1)) {
			if (getEntities()->isPlayerPosition(kCarGreenSleeping, 1)) {
				setCallback(13);
				setup_function13(true, false);
			} else if (getEntities()->isPlayerPosition(kCarGreenSleeping, 23)) {
				setCallback(14);
				setup_function13(false, false);
			}
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback_1;

		case 2:
			goto label_callback_2;

		case 3:
			goto label_callback_3;

		case 4:
			goto label_callback_4;

		case 5:
			goto label_callback_5;

		case 6:
			goto label_callback_6;

		case 7:
			goto label_callback_7;

		case 8:
			goto label_callback_8;

		case 9:
			goto label_callback_9;

		case 10:
			goto label_callback_10;
		}
		break;

	case kAction225358684:
		if (!ENTITY_PARAM(0, 1)) {
			setCallback(16);
			setup_function30((MertensActionType)savepoint.param.intValue);
		}
		break;

	case kAction225932896:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1))
			getSavePoints()->push(kEntityMertens, kEntityFrancois, kAction205346192);
		break;

	case kAction226078300:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			setCallback(15);
			setup_playSound("CON2020");
		}
		break;

	case kAction305159806:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			setCallback(17);
			setup_function31((MertensActionType)savepoint.param.intValue);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(47, Mertens, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setCallback(1);
		setup_function17();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMertens);

		getData()->entityPosition = kPosition_1500;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 6) = 0;
		ENTITY_PARAM(0, 8) = 0;

		ENTITY_PARAM(1, 1) = 0;
		ENTITY_PARAM(1, 2) = 0;
		ENTITY_PARAM(1, 3) = 0;
		ENTITY_PARAM(1, 4) = 0;
		ENTITY_PARAM(1, 5) = 0;
		ENTITY_PARAM(1, 6) = 0;
		ENTITY_PARAM(1, 7) = 0;

		ENTITY_PARAM(2, 4) = 0;
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function48();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(48, Mertens, function48)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(2, 3)) {
			params->param1 = 1;

			getObjects()->updateLocation2(kObjectCompartment2, kObjectLocation1);
			getObjects()->updateLocation2(kObjectCompartment3, kObjectLocation1);
			getObjects()->updateLocation2(kObjectCompartment4, kObjectLocation1);

			ENTITY_PARAM(1, 4) = 0;
			ENTITY_PARAM(1, 5) = 0;
			ENTITY_PARAM(1, 6) = 0;
			ENTITY_PARAM(1, 7) = 0;

			getEntities()->drawSequenceLeft(kEntityMertens, "601E");

			ENTITY_PARAM(2, 3) = 0;
		}

		if (ENTITY_PARAM(1, 6)) {
			setCallback(1);
			setup_function16(true);
			break;
		}

label_callback_1:
		if (ENTITY_PARAM(1, 7)) {
			setCallback(2);
			setup_function16(false);
			break;
		}

label_callback_2:
		if (ENTITY_PARAM(1, 5)) {
			setCallback(3);
			setup_function15(true);
			break;
		}

label_callback_3:
		if (ENTITY_PARAM(1, 4)) {
			setCallback(4);
			setup_function15(false);
			break;
		}

label_callback_4:
		if (!params->param1) {
			if (Entity::timeCheckCallback(kTime2403000, params->param2, 5, WRAP_SETUP_FUNCTION(Mertens, setup_function49)))
				break;

label_callback_5:
			if (Entity::timeCheckCallback(kTime2430000, params->param3, 6, WRAP_SETUP_FUNCTION(Mertens, setup_function32)))
				break;

label_callback_6:
			if (Entity::timeCheckCallback(kTime2439000, params->param4, 7, WRAP_SETUP_FUNCTION(Mertens, setup_function32)))
				break;

label_callback_7:
			if (Entity::timeCheckCallback(kTime2448000, params->param5, 8, WRAP_SETUP_FUNCTION(Mertens, setup_function32)))
				break;
		}

label_callback_8:
		if (getState()->time > kTime2538000 && !ENTITY_PARAM(0, 1) && !ENTITY_PARAM(2, 1)) {
			if (!Entity::updateParameter(params->param6, getState()->time, 2700))
				break;

			getEntities()->drawSequenceLeft(kEntityMertens, "601E");

			ENTITY_PARAM(0, 1) = 1;
			params->param6 = 0;
		}
		break;

	case kAction11:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			setCallback(9);
			setup_function13((bool)savepoint.param.intValue, savepoint.entity2 != kEntityPlayer);
		}
		break;

	case kActionDefault:
		getData()->car = kCarGreenSleeping;
		getData()->entityPosition = kPosition_1500;
		getData()->location = kLocationOutsideCompartment;

		getScenes()->loadSceneFromItemPosition(kItem7);
		break;

	case kActionDrawScene:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			if (getEntities()->isPlayerPosition(kCarGreenSleeping, 1)) {
				setCallback(10);
				setup_function13(true, false);
			} else if (getEntities()->isPlayerPosition(kCarGreenSleeping, 23)) {
				setCallback(11);
				setup_function13(false, false);
			}
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback_1;

		case 2:
			goto label_callback_2;

		case 3:
			goto label_callback_3;

		case 4:
			goto label_callback_4;

		case 5:
			goto label_callback_5;

		case 6:
			goto label_callback_6;

		case 7:
			goto label_callback_7;

		case 8:
			goto label_callback_8;
		}
		break;

	case kAction225358684:
		if (!ENTITY_PARAM(0, 1)) {
			setCallback(13);
			setup_function30((MertensActionType)savepoint.param.intValue);
		}
		break;

	case kAction226078300:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			setCallback(12);
			setup_playSound("CON2020");
		}
		break;

	case kAction305159806:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			setCallback(14);
			setup_function31((MertensActionType)savepoint.param.intValue);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(49, Mertens, function49)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function19();
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
			setup_tylerCompartment(kMertensActionNone);
			break;

		case 3:
			setCallback(4);
			setup_function33();
			break;

		case 4:
			setCallback(5);
			setup_function25();
			break;

		case 5:
			setCallback(6);
			setup_function33();
			break;

		case 6:
			setCallback(7);
			setup_function24();
			break;

		case 7:
			setCallback(8);
			setup_function33();
			break;

		case 8:
			setCallback(9);
			setup_function23();
			break;

		case 9:
			setCallback(10);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 10:
			setCallback(11);
			setup_function17();
			break;

		case 11:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(50, Mertens, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMertens);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(51, Mertens, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_function52();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(52, Mertens, function52)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param2 == kTimeInvalid)
			break;

		if (params->param1 >= getState()->time) {

			if (!getEntities()->isPlayerInCar(kCarRedSleeping) || !params->param2)
				params->param2 = (uint)getState()->time;

			if (params->param2 >= getState()->time)
				break;
		}

		params->param2 = kTimeInvalid;

		setCallback(1);
		setup_playSound("Mme5010");
		break;

	case kActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;

		getObjects()->update(kObjectCompartmentD, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);

		params->param1 = (uint)(getState()->time + 4500);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("671Ad", kObjectCompartmentD);
			break;

		case 2:
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityMertens, kEntityMmeBoutarel, kAction155604840);
			setup_function53();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(53, Mertens, function53)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			if (!Entity::updateParameter(params->param4, getState()->timeTicks, 75))
				break;

			params->param1 = 0;
			params->param2 = 0;

			getObjects()->update(kObjectCompartment4, kEntityMertens, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param4 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (params->param1) {
			getObjects()->update(kObjectCompartment4, kEntityMertens, kObjectLocation1, kCursorNormal, kCursorNormal);
			params->param1 = 0;

			setCallback(3);
			setup_playSound(getSound()->justCheckingCath());
		}

		setCallback(savepoint.action == kActionKnock ? 4 : 5);
		setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_5790);
		break;

	case kActionDrawScene:
		if (params->param2 || params->param1) {
			params->param1 = 0;
			params->param2 = 0;
			params->param3 = 0;

			getObjects()->update(kObjectCompartment4, kEntityMertens, kObjectLocation1, kCursorHandKnock, kCursorHand);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("601ZD", kObjectCompartment4);
			break;

		case 2:
			getEntities()->clearSequences(kEntityMertens);
			getData()->location = kLocationInsideCompartment;
			getData()->entityPosition = kPosition_5790;
			// Fallback to next case

		case 3:
			getObjects()->update(kObjectCompartment4, kEntityMertens, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;

		case 4:
		case 5:
			params->param3++;

			if (params->param3 == 1) {
				getObjects()->update(kObjectCompartment4, kEntityMertens, kObjectLocation1, kCursorNormal, kCursorNormal);

				setCallback(6);
				setup_playSound("Con5002");

			} else if (params->param3 == 2) {
				getObjects()->update(kObjectCompartment4, kEntityMertens, kObjectLocation1, kCursorNormal, kCursorNormal);

				setCallback(7);
				setup_playSound("Con5002A");
			}
			break;

		case 6:
			params->param1 = 1;
			getObjects()->update(kObjectCompartment4, kEntityMertens, kObjectLocation1, kCursorTalk, kCursorNormal);
			break;

		case 7:
			params->param2 = 1;
			break;
		}
		break;

	case kAction135800432:
		setup_nullfunction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(54, Mertens)

//////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////

void Mertens::loadSceneFromPosition() {
	if (getData()->direction != kDirectionUp)
		getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + 750));
	else
		getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition - 750), true);
}

} // End of namespace LastExpress
