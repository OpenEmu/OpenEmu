/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "scx_reader.h"

#include "../event_queue.h"
#include "m3_extra_cycles.h"
#include "../savestate.h"

ScxReader::ScxReader(event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue_in,
//                      VideoEvent &wyReader3_in,
                     VideoEvent &wxReader_in,
                     VideoEvent &weEnableChecker_in,
                     VideoEvent &weDisableChecker_in,
                     M3ExtraCycles &m3ExtraCycles) :
	VideoEvent(1),
	m3EventQueue(m3EventQueue_in),
// 	wyReader3(wyReader3_in),
	wxReader(wxReader_in),
	weEnableChecker(weEnableChecker_in),
	weDisableChecker(weDisableChecker_in),
	m3ExtraCycles(m3ExtraCycles)
{
	setDoubleSpeed(false);
	setSource(0);
	scxAnd7_ = src;
}

static void rescheduleEvent(event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue, VideoEvent& event, const unsigned long diff) {
	if (event.time() != VideoEvent::DISABLED_TIME) {
		event.setTime(event.time() + diff);
		(diff & 0x10) ? m3EventQueue.dec(&event, &event) : m3EventQueue.inc(&event, &event);
	}
}

void ScxReader::doEvent() {
	const unsigned long diff = (static_cast<unsigned long>(src) - static_cast<unsigned long>(scxAnd7_)) << dS;
	scxAnd7_ = src;
	
// 	rescheduleEvent(m3EventQueue, wyReader3, diff);
	rescheduleEvent(m3EventQueue, wxReader, diff);
	rescheduleEvent(m3EventQueue, weEnableChecker, diff);
	rescheduleEvent(m3EventQueue, weDisableChecker, diff);
	
	m3ExtraCycles.invalidateCache();
	
	setTime(DISABLED_TIME);
}

void ScxReader::saveState(SaveState &state) const {
	state.ppu.scxAnd7 = scxAnd7_;
}

void ScxReader::loadState(const SaveState &state) {
	scxAnd7_ = state.ppu.scxAnd7;
}
