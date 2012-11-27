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


#include "sword1/eventman.h"
#include "sword1/sworddefs.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Sword1 {

EventManager::EventManager() {
	for (uint8 cnt = 0; cnt < TOTAL_EVENT_SLOTS; cnt++)
		_eventPendingList[cnt].delay = _eventPendingList[cnt].eventNumber = 0;
}

void EventManager::serviceGlobalEventList() {
	for (uint8 slot = 0; slot < TOTAL_EVENT_SLOTS; slot++)
		if (_eventPendingList[slot].delay)
			_eventPendingList[slot].delay--;
}

void EventManager::checkForEvent(Object *compact) {
	for (uint8 objCnt = 0; objCnt < O_TOTAL_EVENTS; objCnt++) {
		if (compact->o_event_list[objCnt].o_event)
			for (uint8 globCnt = 0; globCnt < TOTAL_EVENT_SLOTS; globCnt++) {
				if (_eventPendingList[globCnt].delay &&
				        (_eventPendingList[globCnt].eventNumber == compact->o_event_list[objCnt].o_event)) {
					compact->o_logic = LOGIC_script;      //force into script mode
					_eventPendingList[globCnt].delay = 0; //started, so remove from queue
					compact->o_tree.o_script_level++;
					compact->o_tree.o_script_id[compact->o_tree.o_script_level] =
					    compact->o_event_list[objCnt].o_event_script;
					compact->o_tree.o_script_pc[compact->o_tree.o_script_level] =
					    compact->o_event_list[objCnt].o_event_script;
				}
			}
	}
}

bool EventManager::eventValid(int32 event) {
	for (uint8 slot = 0; slot < TOTAL_EVENT_SLOTS; slot++)
		if ((_eventPendingList[slot].eventNumber == event) &&
		        (_eventPendingList[slot].delay))
			return true;
	return false;
}

int EventManager::fnCheckForEvent(Object *cpt, int32 id, int32 pause) {
	if (pause) {
		cpt->o_pause = pause;
		cpt->o_logic = LOGIC_pause_for_event;
		return SCRIPT_STOP;
	}

	for (uint8 objCnt = 0; objCnt < O_TOTAL_EVENTS; objCnt++) {
		if (cpt->o_event_list[objCnt].o_event)
			for (uint8 globCnt = 0; globCnt < TOTAL_EVENT_SLOTS; globCnt++) {
				if (_eventPendingList[globCnt].delay &&
				        (_eventPendingList[globCnt].eventNumber == cpt->o_event_list[objCnt].o_event)) {
					cpt->o_logic = LOGIC_script;      //force into script mode
					_eventPendingList[globCnt].delay = 0; //started, so remove from queue
					cpt->o_tree.o_script_level++;
					cpt->o_tree.o_script_id[cpt->o_tree.o_script_level] =
					    cpt->o_event_list[objCnt].o_event_script;
					cpt->o_tree.o_script_pc[cpt->o_tree.o_script_level] =
					    cpt->o_event_list[objCnt].o_event_script;
					return SCRIPT_STOP;
				}
			}
	}
	return SCRIPT_CONT;
}

void EventManager::fnIssueEvent(Object *compact, int32 id, int32 event, int32 delay) {
	uint8 evSlot = 0;
	while (_eventPendingList[evSlot].delay)
		evSlot++;
	if (evSlot >= TOTAL_EVENT_SLOTS)
		error("EventManager ran out of event slots");
	_eventPendingList[evSlot].delay = delay;
	_eventPendingList[evSlot].eventNumber = event;
}

} // End of namespace Sword1
