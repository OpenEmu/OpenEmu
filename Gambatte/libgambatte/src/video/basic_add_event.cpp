/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aamås                                    *
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
#include "basic_add_event.h"
#include "../event_queue.h"

void addEvent(event_queue<VideoEvent*,VideoEventComparer> &q, VideoEvent *const e, const unsigned long newTime) {
	const unsigned long oldTime = e->time();
	
	if (oldTime != newTime) {
		e->setTime(newTime);
		
		if (newTime < oldTime) {
			if (oldTime == VideoEvent::DISABLED_TIME)
				q.push(e);
			else
				q.dec(e, e);
		} else {
			if (newTime == VideoEvent::DISABLED_TIME)
				q.remove(e);
			else
				q.inc(e, e);
		}
	}
}

void addUnconditionalEvent(event_queue<VideoEvent*,VideoEventComparer> &q, VideoEvent *const e, const unsigned long newTime) {
	const unsigned long oldTime = e->time();
	
	e->setTime(newTime);
	
	if (newTime < oldTime) {
		if (oldTime == VideoEvent::DISABLED_TIME)
			q.push(e);
		else
			q.dec(e, e);
	} else if (oldTime != newTime) {
		q.inc(e, e);
	}
}

void addFixedtimeEvent(event_queue<VideoEvent*,VideoEventComparer> &q, VideoEvent *const e, const unsigned long newTime) {
	const unsigned long oldTime = e->time();
	
	if (oldTime != newTime) {
		e->setTime(newTime);
		
		if (oldTime == VideoEvent::DISABLED_TIME)
			q.push(e);
		else
			q.remove(e);
	}
}

void addUnconditionalFixedtimeEvent(event_queue<VideoEvent*,VideoEventComparer> &q, VideoEvent *const e, const unsigned long newTime) {
	if (e->time() == VideoEvent::DISABLED_TIME) {
		e->setTime(newTime);
		q.push(e);
	}
}
