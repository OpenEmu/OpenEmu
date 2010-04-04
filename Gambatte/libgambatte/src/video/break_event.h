/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
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
#ifndef BREAK_EVENT_H
#define BREAK_EVENT_H

#include "video_event.h"
#include "ly_counter.h"
#include "basic_add_event.h"

class BreakEvent : public VideoEvent {
	unsigned char &drawStartCycle;
	unsigned char &scReadOffset;
	
	unsigned char scxSrc;
	unsigned char baseCycle;
	
public:
	BreakEvent(unsigned char &drawStartCycle_in, unsigned char &scReadOffset_in);
	
	void doEvent();
	
	static unsigned long schedule(const LyCounter &lyCounter) {
		return lyCounter.time();
	}
	
	void setDoubleSpeed(const bool dS) {
		baseCycle = 90 + dS * 4;
	}
	
	void setScxSource(const unsigned scxSrc_in) {
		scxSrc = scxSrc_in;
	}
};

static inline void addEvent(event_queue<VideoEvent*,VideoEventComparer> &q, BreakEvent *const e, const unsigned long newTime) {
	addUnconditionalEvent(q, e, newTime);
}

static inline void addFixedtimeEvent(event_queue<VideoEvent*,VideoEventComparer> &q, BreakEvent *const e, const unsigned long newTime) {
	addUnconditionalFixedtimeEvent(q, e, newTime);
}

#endif
