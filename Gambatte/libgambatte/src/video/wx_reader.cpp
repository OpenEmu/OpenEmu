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
#include "wx_reader.h"

#include "../event_queue.h"
#include "m3_extra_cycles.h"
#include "../savestate.h"

WxReader::WxReader(event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue,
                   VideoEvent &weEnableChecker,
                   VideoEvent &weDisableChecker,
                   M3ExtraCycles &m3ExtraCycles) :
VideoEvent(7),
m3EventQueue(m3EventQueue),
weEnableChecker(weEnableChecker),
weDisableChecker(weDisableChecker),
m3ExtraCycles(m3ExtraCycles)
{
	setDoubleSpeed(false);
	setSource(0);
	wx_ = src_;
}

static void rescheduleEvent(event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue, VideoEvent& event, const unsigned long diff) {
	if (event.time() != VideoEvent::DISABLED_TIME) {
		event.setTime(event.time() + diff);
		(diff & 0x200) ? m3EventQueue.dec(&event, &event) : m3EventQueue.inc(&event, &event);
	}
}

void WxReader::doEvent() {
	const unsigned long diff = (static_cast<unsigned long>(src_) - static_cast<unsigned long>(wx_)) << dS;
	wx_ = src_;
	
	rescheduleEvent(m3EventQueue, weEnableChecker, diff);
	rescheduleEvent(m3EventQueue, weDisableChecker, diff);
	
	m3ExtraCycles.invalidateCache();
	
	setTime(DISABLED_TIME);
}

void WxReader::saveState(SaveState &state) const {
	state.ppu.wx = wx_;
}

void WxReader::loadState(const SaveState &state) {
	wx_ = state.ppu.wx;
}
