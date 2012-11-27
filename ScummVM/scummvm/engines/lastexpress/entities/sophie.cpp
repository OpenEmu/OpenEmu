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

#include "lastexpress/entities/sophie.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Sophie::Sophie(LastExpressEngine *engine) : Entity(engine, kEntitySophie) {
	ADD_CALLBACK_FUNCTION(Sophie, reset);
	ADD_CALLBACK_FUNCTION(Sophie, updateEntity);
	ADD_CALLBACK_FUNCTION(Sophie, chaptersHandler);
	ADD_CALLBACK_FUNCTION(Sophie, chapter1);
	ADD_CALLBACK_FUNCTION(Sophie, function5);
	ADD_CALLBACK_FUNCTION(Sophie, chapter2);
	ADD_CALLBACK_FUNCTION(Sophie, chapter3);
	ADD_CALLBACK_FUNCTION(Sophie, chapter4);
	ADD_CALLBACK_FUNCTION(Sophie, function9);
	ADD_CALLBACK_FUNCTION(Sophie, chapter5);
	ADD_CALLBACK_FUNCTION(Sophie, chapter5Handler);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Sophie, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(2, Sophie, updateEntity, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone: {
		params->param3 = 0;

		// Sophie
		byte direction = getData()->direction;
		EntityPosition position = getData()->entityPosition;
		CarIndex car = getData()->car;

		// Rebecca
		EntityPosition rebecca_position = getEntityData(kEntityRebecca)->entityPosition;
		CarIndex rebeccaCar = getEntityData(kEntityRebecca)->car;

		if (getEntities()->isDistanceBetweenEntities(kEntitySophie, kEntityRebecca, 500)
		|| (direction == kDirectionUp && car >= rebeccaCar && position > rebecca_position)
		|| (direction == kDirectionDown && car <= rebeccaCar && position < rebecca_position)) {
			 getData()->field_49B = 0;
			 params->param3 = 1;
		}

		if (!params->param3)
			getEntities()->updateEntity(kEntitySophie, (CarIndex)params->param1, (EntityPosition)params->param2);

		break;
	}

	case kActionExcuseMeCath:
		getSound()->excuseMeCath();
		break;

	case kActionExcuseMe:
		getSound()->excuseMe(kEntitySophie);
		break;

	case kActionDefault:
		getEntities()->updateEntity(kEntitySophie, (CarIndex)params->param1, (EntityPosition)params->param2);
		break;

	case kAction123668192:
		callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(3, Sophie, chaptersHandler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		getData()->entityPosition = getEntityData(kEntityRebecca)->entityPosition;
		getData()->car = getEntityData(kEntityRebecca)->car;
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->clearSequences(kEntitySophie);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntitySophie, "BLANK");
			break;

		case 3:
			getEntities()->clearSequences(kEntitySophie);
			break;

		case 4:
			getEntities()->drawSequenceLeft(kEntitySophie, "BLANK");
			break;
		}
		break;

	case kAction125242096:
		getData()->entityPosition = (EntityPosition)(getEntityData(kEntityRebecca)->entityPosition - 100);
		getData()->location = getEntityData(kEntityRebecca)->location;
		getData()->car = getEntityData(kEntityRebecca)->car;

		setCallback(1);
		setup_updateEntity(kCarRestaurant, kPosition_850);
		break;

	case kAction136654208:
		getData()->entityPosition = (EntityPosition)(getEntityData(kEntityRebecca)->entityPosition + 100);
		getData()->location = getEntityData(kEntityRebecca)->location;
		getData()->car = getEntityData(kEntityRebecca)->car;

		setCallback(2);
		setup_updateEntity(kCarRedSleeping, kPosition_4840);
		break;

	case kAction259921280:
		getData()->entityPosition = (EntityPosition)(getEntityData(kEntityRebecca)->entityPosition + 100);
		getData()->location = getEntityData(kEntityRebecca)->location;
		getData()->car = getEntityData(kEntityRebecca)->car;

		setCallback(3);
		setup_updateEntity(kCarKronos, kPosition_9460);
		break;

	case kAction292775040:
		getData()->entityPosition = kPosition_9270;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarKronos;

		setCallback(4);
		setup_updateEntity(kCarRedSleeping, kPosition_4840);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(4, Sophie, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Sophie, setup_chaptersHandler));
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_4840;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(5, Sophie, function5)
	handleAction(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Sophie, chapter2)
	handleChapter(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Sophie, chapter3)
	handleChapter(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Sophie, chapter4)
	handleChapter(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Sophie, function9)
	handleAction(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Sophie, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntitySophie);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Sophie, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_nullfunction();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(12, Sophie)

//////////////////////////////////////////////////////////////////////////
// Helpers functions
//////////////////////////////////////////////////////////////////////////

void Sophie::handleAction(const SavePoint &savepoint) {
	if (savepoint.action == kActionDefault) {
		getData()->entityPosition = kPosition_4840;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getEntities()->clearSequences(kEntitySophie);
	}
}

void Sophie::handleChapter(const SavePoint &savepoint) {
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chaptersHandler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntitySophie);
		getData()->entityPosition = kPosition_4840;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
}

} // End of namespace LastExpress
