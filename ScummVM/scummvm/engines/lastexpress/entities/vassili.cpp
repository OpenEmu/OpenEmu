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

#include "lastexpress/entities/vassili.h"

#include "lastexpress/entities/anna.h"
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

Vassili::Vassili(LastExpressEngine *engine) : Entity(engine, kEntityVassili) {
	ADD_CALLBACK_FUNCTION(Vassili, reset);
	ADD_CALLBACK_FUNCTION(Vassili, draw);
	ADD_CALLBACK_FUNCTION(Vassili, savegame);
	ADD_CALLBACK_FUNCTION(Vassili, chapter1);
	ADD_CALLBACK_FUNCTION(Vassili, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Vassili, function6);
	ADD_CALLBACK_FUNCTION(Vassili, function7);
	ADD_CALLBACK_FUNCTION(Vassili, function8);
	ADD_CALLBACK_FUNCTION(Vassili, function9);
	ADD_CALLBACK_FUNCTION(Vassili, seizure);
	ADD_CALLBACK_FUNCTION(Vassili, drawInBed);
	ADD_CALLBACK_FUNCTION(Vassili, chapter2);
	ADD_CALLBACK_FUNCTION(Vassili, sleeping);
	ADD_CALLBACK_FUNCTION(Vassili, chapter3);
	ADD_CALLBACK_FUNCTION(Vassili, stealEgg);
	ADD_CALLBACK_FUNCTION(Vassili, chapter4);
	ADD_CALLBACK_FUNCTION(Vassili, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Vassili, chapter5);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Vassili, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Vassili, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(3, Vassili, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(4, Vassili, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Vassili, setup_chapter1Handler));
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject40, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(5, Vassili, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			getData()->entityPosition = getEntityData(kEntityTatiana)->entityPosition;
			getData()->location = getEntityData(kEntityTatiana)->location;
		} else {
			if (params->param3 && params->param3 >= getState()->time) {
				break;
			}else {
				params->param3 = (uint)getState()->time + 450;
				if (params->param3 == 0)
					break;
			}

			if (!params->param2 && getObjects()->get(kObjectCompartmentA).location == kObjectLocation1) {
				params->param2 = 1;
				getEntities()->drawSequenceLeft(kEntityVassili, "303A");
				getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			}
			break;
		}
		break;

	case kActionDefault:
		params->param1 = 1;
		break;

	case kAction122732000:
		setup_function6();
		break;

	case kAction168459827:
		params->param1 = 0;
		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Vassili, function6)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_8200)) {
			if (!Entity::updateParameter(params->param3, getState()->timeTicks, params->param1))
				goto label_function7;

			setCallback(1);
			setup_draw("303B");
			break;
		}

		params->param3 = 0;

		if (params->param2)
			getEntities()->drawSequenceLeft(kEntityVassili, "303A");

label_function7:
		if (params->param4 != kTimeInvalid && getState()->time > kTime1489500) {

			if (getState()->time <= kTime1503000) {

				if (getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_8200) || !params->param4) {

					params->param4 = (uint)getState()->time;
					if (!params->param4) {
						setup_function7();
						break;
					}
				}

				if (params->param4 >= getState()->time)
					break;
			}

			params->param4 = kTimeInvalid;
			setup_function7();
		}
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		params->param1 = 5 * (3 * rnd(25) + 15);

		getEntities()->drawSequenceLeft(kEntityVassili, "303A");
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getEntities()->drawSequenceLeft(kEntityVassili, "303C");
			params->param1 = 5 * (3 * rnd(25) + 15);
			params->param2 = 1;

			// Shared part with kActionNone
			goto label_function7;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Vassili, function7)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 != kTimeInvalid && getState()->time > kTime1503000) {

			 if (getState()->time <= kTime1512000) {
				 if (getEntities()->isPlayerInCar(kCarRedSleeping) || !params->param1) {
					 params->param1 = (uint)getState()->time + 150;
					 if (params->param1) {
						 setup_function8();
						 break;
					 }
				 }

				 if (params->param1 >= getState()->time)
					 break;
			 }

			 params->param1 = kTimeInvalid;
			 setup_function8();
		}
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->clearSequences(kEntityVassili);
		if (getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_8200))
			getScenes()->loadSceneFromObject(kObjectCompartmentA);

		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kAction339669520:
		setup_function9();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Vassili, function8)
	switch (savepoint.action) {
	default:
		break;

	case kActionEndSound:
		setup_function9();
		break;

	case kActionDefault:
		if (!getEntities()->isInsideTrainCar(kEntityPlayer, kCarRedSleeping)) {
			getSound()->playSound(kEntityPlayer, "BUMP");
			getScenes()->loadSceneFromPosition(kCarRedSleeping, (getEntityData(kEntityPlayer)->car <= kCarRedSleeping) ? 1 : 40);
		}

		getSavePoints()->push(kEntityVassili, kEntityAnna, kAction226031488);
		getSavePoints()->push(kEntityVassili, kEntityVerges, kAction226031488);
		getSavePoints()->push(kEntityVassili, kEntityCoudert, kAction226031488);
		getSound()->playSound(kEntityVassili, "VAS1027", kFlagDefault);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Vassili, function9)
	switch (savepoint.action) {
	default:
		break;

	case kActionEndSound:
		if (!getEntities()->isDistanceBetweenEntities(kEntityVassili, kEntityPlayer, 2500))
			getSound()->playSound(kEntityPlayer, "BUMP");

		setup_seizure();
		break;

	case kActionDefault:
	case kActionDrawScene:
		if ((getObjects()->get(kObjectCompartmentA).location == kObjectLocation2 && getEntities()->isPlayerPosition(kCarRedSleeping, 17))
		|| getEntities()->isPlayerPosition(kCarRedSleeping, 18)
		|| getEntities()->isPlayerPosition(kCarRedSleeping, 37)
		|| getEntities()->isPlayerPosition(kCarRedSleeping, 38)
		|| getEntities()->isPlayerPosition(kCarRedSleeping, 41)) {

			if (savepoint.action == kActionDrawScene)
				getSoundQueue()->processEntry(kEntityVassili);

			setup_seizure();
		} else {
			if (savepoint.action == kActionDefault)
				getSound()->playSound(kEntityVassili, "VAS1028", kFlagDefault);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Vassili, seizure)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		// Check that we have removed the body from the train and changed jacket
		if (!getProgress().eventCorpseMovedFromFloor) {
			getAction()->playAnimation(kEventMertensCorpseFloor);
			getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, false);
			break;
		}

		if (!getProgress().eventCorpseThrown) {
			getAction()->playAnimation(kEventMertensCorpseBed);
			getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, false);
			break;
		}

		if (getProgress().jacket == kJacketBlood) {
			getAction()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, false);
			break;
		}

		// Setup Anna & Coudert
		RESET_ENTITY_STATE(kEntityAnna, Anna, setup_function37);
		RESET_ENTITY_STATE(kEntityCoudert, Coudert, setup_function38);

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventVassiliSeizure);
		break;

	case kActionCallback:
		if (getCallback() != 1)
			break;

		getData()->location = kLocationInsideCompartment;
		getAction()->playAnimation(kEventVassiliSeizure);

		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getProgress().field_18 = 2;

		getSavePoints()->push(kEntityVassili, kEntityAnna, kAction191477936);
		getSavePoints()->push(kEntityVassili, kEntityVerges, kAction191477936);
		getSavePoints()->push(kEntityVassili, kEntityCoudert, kAction191477936);
		getScenes()->loadSceneFromObject(kObjectCompartmentA);

		setup_drawInBed();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Vassili, drawInBed)
	if (savepoint.action == kActionDefault)
		getEntities()->drawSequenceLeft(kEntityVassili, "303A");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Vassili, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_sleeping();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityVassili);

		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->updateLocation2(kObjectCompartmentA, kObjectLocation1);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Vassili, sleeping)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_8200)) {
			if (!Entity::updateParameter(params->param3, getState()->timeTicks, params->param1))
				break;

			setCallback(1);
			setup_draw("303B");
		} else {
			params->param3 = 0;
			if (params->param2)
				getEntities()->drawSequenceLeft(kEntityVassili, "303A");
		}
		break;

	case kActionDefault:
		params->param5 = 5 * (3 * rnd(25) + 15);
		getEntities()->drawSequenceLeft(kEntityVassili, "303A");
		break;

	case kActionCallback:
		if (getCallback() != 1)
			break;

		getEntities()->drawSequenceLeft(kEntityVassili, "303C");
		params->param1 = 5 * (3 * rnd(25) + 15);
		params->param2 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Vassili, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_stealEgg();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityVassili);

		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Vassili, stealEgg)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_8200)) {
			if (!Entity::updateParameter(params->param3, getState()->timeTicks, params->param1))
				break;

			setCallback(1);
			setup_draw("303B");
		} else {
			params->param3 = 0;
			if (params->param2)
				getEntities()->drawSequenceLeft(kEntityVassili, "303A");
		}
		break;

	case kActionOpenDoor:
		setCallback(2);
		setup_savegame(kSavegameTypeEvent, kEventVassiliCompartmentStealEgg);
		break;

	case kActionDefault:
		params->param5 = 5 * (3 * rnd(25) + 15);
		getEntities()->drawSequenceLeft(kEntityVassili, "303A");
		break;

	case kActionDrawScene:
		if (getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_7850)
		 && getInventory()->hasItem(kItemFirebird)
		 && !getEvent(kEventVassiliCompartmentStealEgg))
			getObjects()->update(kObject48, kEntityVassili, kObjectLocationNone, kCursorNormal, kCursorHand);
		else
			getObjects()->update(kObject48, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kEntityVassili, "303C");
			params->param1 = 5 * (3 * rnd(25) + 15);
			params->param2 = 1;
			break;

		case 2:
			getAction()->playAnimation(kEventVassiliCompartmentStealEgg);
			getScenes()->loadSceneFromPosition(kCarRedSleeping, 67);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Vassili, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityVassili);

		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->updateLocation2(kObjectCompartmentA, kObjectLocation1);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
// Looks identical to sleeping (#13)
IMPLEMENT_FUNCTION(17, Vassili, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_8200)) {
			if (!Entity::updateParameter(params->param3, getState()->timeTicks, params->param1))
				break;

			setCallback(1);
			setup_draw("303B");
		} else {
			params->param3 = 0;
			if (params->param2)
				getEntities()->drawSequenceLeft(kEntityVassili, "303A");
		}
		break;

	case kActionDefault:
		params->param5 = 5 * (3 * rnd(25) + 15);
		getEntities()->drawSequenceLeft(kEntityVassili, "303A");
		break;

	case kActionCallback:
		if (getCallback() != 1)
			break;

		getEntities()->drawSequenceLeft(kEntityVassili, "303C");
		params->param1 = 5 * (3 * rnd(25) + 15);
		params->param2 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Vassili, chapter5)
	if (savepoint.action == kActionDefault) {
		getEntities()->clearSequences(kEntityVassili);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
	}
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
