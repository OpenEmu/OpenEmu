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
#ifndef BASIC_ADD_EVENT_H
#define BASIC_ADD_EVENT_H

template<typename T, class Comparer> class event_queue;

#include "video_event.h"
#include "video_event_comparer.h"

/*template<class T>
static inline void addEvent(T &event, const LyCounter &lyCounter, const unsigned long cycleCounter, event_queue<VideoEvent*,VideoEventComparer> &queue) {
	if (event.time() == VideoEvent::DISABLED_TIME) {
		event.schedule(lyCounter, cycleCounter);
		queue.push(&event);
	}
}

template<class T>
static inline void addEvent(T &event, const unsigned data, const LyCounter &lyCounter, const unsigned long cycleCounter, event_queue<VideoEvent*,VideoEventComparer> &queue) {
	if (event.time() == VideoEvent::DISABLED_TIME) {
		event.schedule(data, lyCounter, cycleCounter);
		queue.push(&event);
	}
}

template<class T>
static inline void addEvent(T &event, const unsigned data1, const unsigned data2, const LyCounter &lyCounter, const unsigned long cycleCounter, event_queue<VideoEvent*,VideoEventComparer> &queue) {
	if (event.time() == VideoEvent::DISABLED_TIME) {
		event.schedule(data1, data2, lyCounter, cycleCounter);
		queue.push(&event);
	}
}*/

void addEvent(event_queue<VideoEvent*,VideoEventComparer> &q, VideoEvent *e, unsigned long newTime);
void addUnconditionalEvent(event_queue<VideoEvent*,VideoEventComparer> &q, VideoEvent *e, unsigned long newTime);
void addFixedtimeEvent(event_queue<VideoEvent*,VideoEventComparer> &q, VideoEvent *e, unsigned long newTime);
void addUnconditionalFixedtimeEvent(event_queue<VideoEvent*,VideoEventComparer> &q, VideoEvent *e, unsigned long newTime);

#endif
