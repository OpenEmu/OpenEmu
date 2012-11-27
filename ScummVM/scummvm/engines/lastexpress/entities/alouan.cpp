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

#include "lastexpress/entities/alouan.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Alouan::Alouan(LastExpressEngine *engine) : Entity(engine, kEntityAlouan) {
	ADD_CALLBACK_FUNCTION(Alouan, reset);
	ADD_CALLBACK_FUNCTION(Alouan, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(Alouan, playSound);
	ADD_CALLBACK_FUNCTION(Alouan, updateFromTime);
	ADD_CALLBACK_FUNCTION(Alouan, updateEntity);
	ADD_CALLBACK_FUNCTION(Alouan, compartment6);
	ADD_CALLBACK_FUNCTION(Alouan, compartment8);
	ADD_CALLBACK_FUNCTION(Alouan, compartment6to8);
	ADD_CALLBACK_FUNCTION(Alouan, compartment8to6);
	ADD_CALLBACK_FUNCTION(Alouan, chapter1);
	ADD_CALLBACK_FUNCTION(Alouan, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Alouan, function12);
	ADD_CALLBACK_FUNCTION(Alouan, chapter2);
	ADD_CALLBACK_FUNCTION(Alouan, chapter2Handler);
	ADD_CALLBACK_FUNCTION(Alouan, chapter3);
	ADD_CALLBACK_FUNCTION(Alouan, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Alouan, chapter4);
	ADD_CALLBACK_FUNCTION(Alouan, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Alouan, function19);
	ADD_CALLBACK_FUNCTION(Alouan, chapter5);
	ADD_CALLBACK_FUNCTION(Alouan, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Alouan, function22);
	ADD_CALLBACK_FUNCTION(Alouan, function23);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Alouan, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(2, Alouan, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, Alouan, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(4, Alouan, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(5, Alouan, updateEntity, CarIndex, EntityPosition)
	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Alouan, compartment6)
	Entity::goToCompartment(savepoint, kObjectCompartment6, kPosition_4070, "621Cf", "621Df");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Alouan, compartment8)
	Entity::goToCompartment(savepoint, kObjectCompartment8, kPosition_2740, "621Ch", "621Dh");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Alouan, compartment6to8)
	Entity::goToCompartmentFromCompartment(savepoint, kObjectCompartment6, kPosition_4070, "621Bf", kObjectCompartment8, kPosition_2740, "621Ah");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Alouan, compartment8to6)
	Entity::goToCompartmentFromCompartment(savepoint, kObjectCompartment8, kPosition_2740, "621Bh", kObjectCompartment6, kPosition_4070, "621Af");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Alouan, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Alouan, setup_chapter1Handler));
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Alouan, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:

		if (Entity::timeCheckCallback(kTime1096200, params->param1, 1, WRAP_SETUP_FUNCTION(Alouan, setup_compartment8to6)))
			break;

label_callback1:
		if (getState()->time > kTime1162800 && !params->param2) {
			params->param2 = 1;
			getSavePoints()->push(kEntityAlouan, kEntityTrain, kAction191070912, kPosition_4070);
			getData()->entityPosition = kPosition_4070;
		}

		if (getState()->time > kTime1179000 && !params->param3) {
			params->param3 = 1;
			getSavePoints()->push(kEntityAlouan, kEntityTrain, kAction191070912, kPosition_4840);

			setCallback(2);
			setup_compartment6to8();
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->entityPosition = kPosition_4840;
			goto label_callback1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Alouan, function12)
	if (savepoint.action == kActionDefault) {
		getObjects()->update(kObjectCompartment7, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartment5, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;

		getEntities()->clearSequences(kEntityAlouan);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Alouan, chapter2)
	if (savepoint.action != kActionDefault)
		return;

	getEntities()->clearSequences(kEntityAlouan);

	getData()->entityPosition = kPosition_2740;
	getData()->location = kLocationInsideCompartment;
	getData()->car = kCarGreenSleeping;
	getData()->clothes = kClothesDefault;
	getData()->inventoryItem = kItemNone;

	setup_chapter2Handler();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Alouan, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param2 == kTimeInvalid)
			break;

		if (getState()->time <= kTime1777500) {
			if (!getEntities()->isPlayerInCar(kCarGreenSleeping) || !params->param2)
				params->param2 = (uint)getState()->time + 75;

			if (params->param2 >= getState()->time)
				break;
		}

		params->param2 = kTimeInvalid;

		setCallback(params->param1 ? 1 : 2);
		if (params->param1)
			setup_compartment8();
		else
			setup_compartment6();
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityAlouan, kEntityTrain, kAction191070912, kPosition_4840);
		params->param1 = 1;
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 3:
			params->param1 = 0;
			setCallback(4);
			setup_playSound("Har2011");
			break;

		case 4:
			setCallback(5);
			setup_updateFromTime(900);
			break;

		case 5:
			getSavePoints()->push(kEntityAlouan, kEntityFrancois, kAction190219584);
			break;
		}
		break;

	case kAction189489753:
		setCallback(3);
		setup_compartment8to6();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Alouan, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAlouan);

		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Alouan, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::timeCheckCallback(kTimeCitySalzbourg, params->param1, 1, WRAP_SETUP_FUNCTION(Alouan, setup_compartment8to6)))
			break;

label_callback1:
		if (params->param2 != kTimeInvalid && getState()->time > kTime1989000) {
			if (Entity::timeCheckCar(kTime2119500, params->param5, 5, WRAP_SETUP_FUNCTION(Alouan, setup_compartment8)))
				break;
		}

label_callback2:
		if (Entity::timeCheckCallback(kTime2052000, params->param3, 3, "Har1005", WRAP_SETUP_FUNCTION_S(Alouan, setup_playSound)))
			break;

label_callback3:
		if (Entity::timeCheckCallback(kTime2133000, params->param4, 4, WRAP_SETUP_FUNCTION(Alouan, setup_compartment6to8)))
			break;

label_callback4:
		if (params->param5 != kTimeInvalid && getState()->time > kTime2151000) {
			if (Entity::timeCheckCar(kTime2241000, params->param5, 5, WRAP_SETUP_FUNCTION(Alouan, setup_compartment8)))
				break;
		}
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityAlouan, kEntityTrain, kAction191070912, kPosition_4840);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->entityPosition = kPosition_4840;
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
IMPLEMENT_FUNCTION(17, Alouan, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAlouan);

		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Alouan, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 != kTimeInvalid) {
			if (Entity::timeCheckCar(kTime2443500, params->param1, 1, WRAP_SETUP_FUNCTION(Alouan, setup_compartment8)))
				break;
		}

label_callback1:
		if (Entity::timeCheckCallback(kTime2455200, params->param2, 2, WRAP_SETUP_FUNCTION(Alouan, setup_compartment8to6)))
			break;

label_callback2:
		if (getState()->time > kTime2475000 && !params->param3) {
			params->param3 = 1;
			getSavePoints()->push(kEntityAlouan, kEntityTrain, kAction191070912, kPosition_4840);

			setCallback(3);
			setup_compartment6to8();
		}
		break;

	case kActionDefault:
		getSavePoints()->push(kEntityAlouan, kEntityTrain, kAction191070912, kPosition_4840);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback1;

		case 2:
			getSavePoints()->push(kEntityAlouan, kEntityTrain, kAction191070912, kPosition_4070);
			goto label_callback2;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Alouan, function19)
	if (savepoint.action == kActionDefault) {
		getObjects()->update(kObjectCompartment7, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartment5, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;

		getEntities()->clearSequences(kEntityAlouan);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Alouan, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityAlouan);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Alouan, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_function22();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Alouan, function22)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param1, getState()->time, 2700))
			break;

		setup_function23();
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5000;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;
		break;

	case kActionDrawScene:
		if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping))
			setup_function23();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Alouan, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_4070);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("619AF", kObjectCompartment5);
			break;

		case 2:
			getEntities()->clearSequences(kEntityAlouan);

			getData()->entityPosition = kPosition_4070;
			getData()->location = kLocationInsideCompartment;

			getObjects()->update(kObjectCompartment6, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;
		}
		break;

	case kAction135800432:
		setup_nullfunction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(24, Alouan)

} // End of namespace LastExpress
