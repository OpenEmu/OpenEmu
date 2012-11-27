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

#include "lastexpress/entities/entity39.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Entity39::Entity39(LastExpressEngine *engine) : Entity(engine, kEntity39) {
	ADD_CALLBACK_FUNCTION(Entity39, chapter1);
	ADD_CALLBACK_FUNCTION(Entity39, chapter2);
	ADD_CALLBACK_FUNCTION(Entity39, chapter3);
	ADD_CALLBACK_FUNCTION(Entity39, chapter4);
	ADD_CALLBACK_FUNCTION(Entity39, chapter5);
	ADD_CALLBACK_FUNCTION(Entity39, process);

	memset(&_sequence, 0, 12);
	_counter = 0;
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Entity39, chapter1)
	if (savepoint.action == kActionDefault)
		setup_process();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(2, Entity39, chapter2)
	if (savepoint.action == kActionDefault)
		setup_process();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(3, Entity39, chapter3)
	if (savepoint.action == kActionDefault)
		setup_process();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(4, Entity39, chapter4)
	if (savepoint.action == kActionDefault)
		setup_process();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(5, Entity39, chapter5)
	if (savepoint.action == kActionDefault)
		setup_process();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Entity39, process)
// TODO: _sequence & counter do not seem to be touched anywhere else in the code :(
	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		getEntities()->drawSequenceRight(kEntity39, (char *)&_sequence);
		break;

	case kActionNone:
		getData()->car = getEntityData(kEntityPlayer)->car;

		if (*_sequence && !_counter) {
			_counter++;
			getEntities()->drawSequenceRight(kEntity39, (char *)&_sequence);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
