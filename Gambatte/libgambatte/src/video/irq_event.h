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
#ifndef VIDEO_IRQ_EVENT_H
#define VIDEO_IRQ_EVENT_H

#include "../event_queue.h"
#include "video_event.h"
#include "video_event_comparer.h"
#include "basic_add_event.h"

class IrqEvent : public VideoEvent {
	event_queue<VideoEvent*,VideoEventComparer> &irqEventQueue;
	
public:
	IrqEvent(event_queue<VideoEvent*,VideoEventComparer> &irqEventQueue_in);
	
	void doEvent();
	
	static unsigned long schedule(const event_queue<VideoEvent*,VideoEventComparer> &irqEventQueue) {
		return irqEventQueue.top()->time();
	}
	
	void schedule() {
		setTime(irqEventQueue.top()->time());
	}
};

static inline void addEvent(event_queue<VideoEvent*,VideoEventComparer> &q, IrqEvent *const e, const unsigned long newTime) {
	addUnconditionalEvent(q, e, newTime);
}

static inline void addFixedtimeEvent(event_queue<VideoEvent*,VideoEventComparer> &q, IrqEvent *const e, const unsigned long newTime) {
	addUnconditionalFixedtimeEvent(q, e, newTime);
}

#endif
