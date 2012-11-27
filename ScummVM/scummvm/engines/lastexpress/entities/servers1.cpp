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

#include "lastexpress/entities/servers1.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Servers1::Servers1(LastExpressEngine *engine) : Entity(engine, kEntityServers1) {
	ADD_CALLBACK_FUNCTION(Servers1, updateFromTime);
	ADD_CALLBACK_FUNCTION(Servers1, draw);
	ADD_CALLBACK_FUNCTION(Servers1, updatePosition);
	ADD_CALLBACK_FUNCTION(Servers1, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(Servers1, callSavepoint);
	ADD_CALLBACK_FUNCTION(Servers1, playSound);
	ADD_CALLBACK_FUNCTION(Servers1, function7);
	ADD_CALLBACK_FUNCTION(Servers1, chapter1);
	ADD_CALLBACK_FUNCTION(Servers1, function9);
	ADD_CALLBACK_FUNCTION(Servers1, function10);
	ADD_CALLBACK_FUNCTION(Servers1, function11);
	ADD_CALLBACK_FUNCTION(Servers1, function12);
	ADD_CALLBACK_FUNCTION(Servers1, function13);
	ADD_CALLBACK_FUNCTION(Servers1, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Servers1, function15);
	ADD_CALLBACK_FUNCTION(Servers1, function16);
	ADD_CALLBACK_FUNCTION(Servers1, chapter2);
	ADD_CALLBACK_FUNCTION(Servers1, chapter2Handler);
	ADD_CALLBACK_FUNCTION(Servers1, function19);
	ADD_CALLBACK_FUNCTION(Servers1, function20);
	ADD_CALLBACK_FUNCTION(Servers1, function21);
	ADD_CALLBACK_FUNCTION(Servers1, chapter3);
	ADD_CALLBACK_FUNCTION(Servers1, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Servers1, function24);
	ADD_CALLBACK_FUNCTION(Servers1, chapter4);
	ADD_CALLBACK_FUNCTION(Servers1, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Servers1, function27);
	ADD_CALLBACK_FUNCTION(Servers1, function28);
	ADD_CALLBACK_FUNCTION(Servers1, function29);
	ADD_CALLBACK_FUNCTION(Servers1, chapter5);
	ADD_CALLBACK_FUNCTION(Servers1, chapter5Handler);
	ADD_NULL_FUNCTION()
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(1, Servers1, updateFromTime)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Servers1, draw)
	Entity::draw(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SII(3, Servers1, updatePosition, CarIndex, Position)
	Entity::updatePosition(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(4, Servers1, callbackActionOnDirection)
	if (savepoint.action == kActionExcuseMeCath) {
		if (!params->param1) {
			getSound()->excuseMe(kEntityServers1);
			params->param1 = 1;
		}
	}

	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SIIS(5, Servers1, callSavepoint, EntityIndex, ActionIndex)
	Entity::callSavepoint(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(6, Servers1, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Servers1, function7)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("924");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityServers1, kEntityBoutarel, kAction122358304);
			setCallback(2);
			setup_draw("008C");
			break;

		case 2:
			getSavePoints()->push(kEntityServers1, kEntityBoutarel, kAction122288808);
			setCallback(2);
			setup_draw("926");
			break;

		case 3:
			getEntities()->clearSequences(kEntityServers1);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(1, 2) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Servers1, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter1Handler();
		break;

	case kActionDefault:
		getSavePoints()->addData(kEntityServers1, kAction223002560, 0);
		getSavePoints()->addData(kEntityServers1, kAction302996448, 2);
		getSavePoints()->addData(kEntityServers1, kAction269485588, 3);
		getSavePoints()->addData(kEntityServers1, kAction326144276, 4);
		getSavePoints()->addData(kEntityServers1, kAction302203328, 5);
		getSavePoints()->addData(kEntityServers1, kAction189688608, 6);
		getSavePoints()->addData(kEntityServers1, kAction236237423, 7);
		getSavePoints()->addData(kEntityServers1, kAction219377792, 8);
		getSavePoints()->addData(kEntityServers1, kAction256200848, 9);
		getSavePoints()->addData(kEntityServers1, kAction291721418, 10);
		getSavePoints()->addData(kEntityServers1, kAction258136010, 11);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Servers1, function9)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("924");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kEntityMilos, "BLANK");
			getEntities()->drawSequenceLeft(kEntityServers1, "009B");

			setCallback(2);
			setup_playSound("WAT1001");
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityMilos, "009A");

			setCallback(3);
			setup_draw("926");
			break;

		case 3:
			getEntities()->clearSequences(kEntityServers1);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 1) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Servers1, function10)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("924");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kEntityBoutarel, "BLANK");
			getEntities()->drawSequenceLeft(kEntityServers1, "008C");

			setCallback(2);
			setup_playSound("MRB1077");
			break;

		case 2:
			getSavePoints()->push(kEntityServers1, kEntityBoutarel, kAction168717392);

			setCallback(3);
			setup_draw("926");
			break;

		case 3:
			getEntities()->clearSequences(kEntityServers1);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 2) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Servers1, function11)
	serveTable(savepoint, "919", kEntityTables1, "005H", "005J", "921", &ENTITY_PARAM(0, 3), 63);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Servers1, function12)
	serveTable(savepoint, "923", kEntityTables2, "009F", "009G", "926", &ENTITY_PARAM(0, 4));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Servers1, function13)
	serveTable(savepoint, "923", kEntityTables2, "009F", "009G", "926", &ENTITY_PARAM(0, 5));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Servers1, chapter1Handler)
switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (!getEntities()->isInKitchen(kEntityServers1) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
			break;

		if (ENTITY_PARAM(0, 1)) {
			setCallback(1);
			setup_function9();
			break;
		}

		if (ENTITY_PARAM(1, 2)) {
			setCallback(2);
			setup_function10();
			break;
		}

		if (ENTITY_PARAM(0, 3)) {
			setCallback(3);
			setup_function11();
			break;
		}

		if (ENTITY_PARAM(0, 4)) {
			setCallback(4);
			setup_function12();
			break;
		}

		if (ENTITY_PARAM(0, 5)) {
			setCallback(5);
			setup_function13();
		}
		break;

	case kActionCallback:
		if (getCallback() == 5) {
			getSavePoints()->push(kEntityServers1, kEntityPascale, kAction352768896);
			setup_function15();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Servers1, function15)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5900;
		break;

	case kAction101632192:
		setup_function16();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Servers1, function16)
	if (savepoint.action == kActionDefault) {
		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;

		getEntities()->clearSequences(kEntityServers1);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Servers1, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter2Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityServers1);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes1;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 6) = 0;
		ENTITY_PARAM(0, 7) = 0;
		ENTITY_PARAM(0, 8) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Servers1, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!getEntities()->isInKitchen(kEntityServers1) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
			break;

		if (ENTITY_PARAM(0, 6)) {
			setCallback(1);
			setup_function19();
			break;
		}

label_callback_1:
		if (ENTITY_PARAM(0, 7)) {
			setCallback(2);
			setup_function20();
			break;
		}

label_callback_2:
		if (ENTITY_PARAM(0, 8) || ENTITY_PARAM(0, 5)) {
			setCallback(3);
			setup_function21();
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

		case 4:
			getEntities()->clearSequences(kEntityServers1);
			getData()->entityPosition = kPosition_5900;
			break;
		}
		break;

	case kAction101106391:
		setCallback(4);
		setup_draw("975");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Servers1, function19)
	serveTable(savepoint, "969", kEntityTables1, "005H2", "018A", "971", &ENTITY_PARAM(0, 6), 63);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Servers1, function20)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("973");
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getSavePoints()->push(kEntityServers1, kEntityIvo, kAction123712592);
			getEntities()->drawSequenceLeft(kEntityServers1, "BLANK");
			ENTITY_PARAM(0, 7) = 0;

			callbackAction();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Servers1, function21)
	serveTable(savepoint, "974", kEntityTables2, "009F2", "009G", "976", &ENTITY_PARAM(0, 8), 0, true, &ENTITY_PARAM(0, 5));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Servers1, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityServers1);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes1;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(1, 1) = 0;
		ENTITY_PARAM(1, 2) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Servers1, chapter3Handler)
	if (savepoint.action != kActionNone)
		return;

	if (!getEntities()->isInKitchen(kEntityServers1) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
		return;

	if (ENTITY_PARAM(1, 1)) {
		setCallback(1);
		setup_function24();
		return;
	}

	if (ENTITY_PARAM(1, 2)) {
		setCallback(2);
		setup_function7();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Servers1, function24)
	serveSalon(savepoint, "927", "Ann3143A", kEntityAnna, "Ann31444", "112C", kAction122288808, "928", &ENTITY_PARAM(1, 1));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Servers1, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityServers1);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		getEntities()->clearSequences(kEntityServers1);

		ENTITY_PARAM(1, 2) = 0;
		ENTITY_PARAM(1, 3) = 0;
		ENTITY_PARAM(1, 4) = 0;
		ENTITY_PARAM(1, 5) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Servers1, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param2) {
			if (Entity::updateParameter(params->param2, getState()->time, 900)) {
				ENTITY_PARAM(1, 5) = 1;
				params->param1 = 0;
			}
		}

		if (!getEntities()->isInKitchen(kEntityServers1) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
			break;

		if (ENTITY_PARAM(1, 5)) {
			setCallback(2);
			setup_function28();
			break;
		}

		if (ENTITY_PARAM(1, 4)) {
			setCallback(3);
			setup_function29();
			break;
		}

		if (ENTITY_PARAM(1, 2)) {
			setCallback(4);
			setup_function7();
		}
		break;

	case kActionCallback:
		if (getCallback() == 1)
			params->param1 = 1;
		break;

	case kAction201431954:
		ENTITY_PARAM(1, 2) = 0;
		ENTITY_PARAM(1, 3) = 0;
		ENTITY_PARAM(1, 4) = 0;
		ENTITY_PARAM(1, 5) = 0;

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Servers1, function27)
	serveSalon(savepoint, "929", "", kEntityAugust, "Aug4003", "122D", kAction134486752, "930", &ENTITY_PARAM(1, 3));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Servers1, function28)
	serveSalon(savepoint, "931", "", kEntityAugust, "Aug4004", "122E", kAction125826561, "930", &ENTITY_PARAM(1, 5));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Servers1, function29)
	serveSalon(savepoint, "932", "", kEntityAnna, "Ann4151", "127D", kAction122288808, "930", &ENTITY_PARAM(1, 4));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Servers1, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityServers1);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Servers1, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_nullfunction();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(32, Servers1)


//////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////
void Servers1::serveTable(const SavePoint &savepoint, const char *seq1, EntityIndex entity, const char *seq2, const char *seq3, const char *seq4, uint *parameter, Position position, bool shouldUpdatePosition, uint *parameter2) {
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (shouldUpdatePosition) {
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;
		}

		setCallback(1);
		setup_draw(seq1);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (position)
				getEntities()->updatePositionEnter(kEntityServers1, kCarRestaurant, position);

			getSavePoints()->push(kEntityServers1, entity, kAction136455232);

			setCallback(2);
			setup_callSavepoint(seq2, entity, kActionDrawTablesWithChairs, seq3);
			break;

		case 2:
			if (position)
				getEntities()->updatePositionExit(kEntityServers1, kCarRestaurant, position);

			setCallback(3);
			setup_draw(seq4);
			break;

		case 3:
			getData()->entityPosition = kPosition_5900;
			getEntities()->clearSequences(kEntityServers1);
			*parameter = 0;

			if (parameter2 != NULL)
				*parameter2 = 0;

			callbackAction();
			break;
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void Servers1::serveSalon(const SavePoint &savepoint, const char *seq1, const char *snd1, EntityIndex entity, const char *snd2, const char *seq2, ActionIndex action, const char *seq3, uint *parameter) {
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("816DD");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceRight(kEntityServers1, seq1);

			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityServers1);

			if (!strcmp(snd1, ""))
				getSound()->playSound(kEntityServers1, snd1);

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			getSavePoints()->push(kEntityServers1, entity, kAction122358304);

			getSound()->playSound(kEntityServers1, snd2);

			setCallback(3);
			setup_updatePosition(seq2, kCarRestaurant, 57);
			break;

		case 3:
			getSavePoints()->push(kEntityServers1, entity, action);

			setCallback(4);
			setup_draw(seq3);
			break;

		case 4:
			getEntities()->drawSequenceRight(kEntityServers1, "816UD");

			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityServers1);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getEntities()->clearSequences(kEntityServers1);
			getData()->entityPosition = kPosition_5900;
			*parameter = 0;

			callbackAction();
			break;
		}
		break;
	}
}

} // End of namespace LastExpress
