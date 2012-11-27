/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/textconsole.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"

namespace Sword2 {

void Logic::sendEvent(uint32 id, uint32 interact_id) {
	for (int i = 0; i < ARRAYSIZE(_eventList); i++) {
		if (_eventList[i].id == id || !_eventList[i].id) {
			_eventList[i].id = id;
			_eventList[i].interact_id = interact_id;
			return;
		}
	}

	error("sendEvent() ran out of event slots");
}

void Logic::setPlayerActionEvent(uint32 id, uint32 interact_id) {
	// Full script id of action script number 2
	sendEvent(id, (interact_id << 16) | 2);
}

int Logic::checkEventWaiting() {
	for (int i = 0; i < ARRAYSIZE(_eventList); i++) {
		if (_eventList[i].id == readVar(ID))
			return 1;
	}

	return 0;
}

void Logic::startEvent() {
	// call this from stuff like fnWalk
	// you must follow with a return IR_TERMINATE

	for (int i = 0; i < ARRAYSIZE(_eventList); i++) {
		if (_eventList[i].id == readVar(ID)) {
			logicOne(_eventList[i].interact_id);
			_eventList[i].id = 0;
			return;
		}
	}

	error("startEvent() can't find event for id %d", readVar(ID));
}

void Logic::clearEvent(uint32 id) {
	for (int i = 0; i < ARRAYSIZE(_eventList); i++) {
		if (_eventList[i].id == id) {
			_eventList[i].id = 0;
			return;
		}
	}
}

void Logic::killAllIdsEvents(uint32 id) {
	for (int i = 0; i < ARRAYSIZE(_eventList); i++) {
		if (_eventList[i].id == id)
			_eventList[i].id = 0;
	}
}

// For the debugger

uint32 Logic::countEvents() {
	uint32 count = 0;

	for (int i = 0; i < ARRAYSIZE(_eventList); i++) {
		if (_eventList[i].id)
			count++;
	}

	return count;
}

} // End of namespace Sword2
