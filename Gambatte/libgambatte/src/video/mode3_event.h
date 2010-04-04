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
#ifndef MODE3_EVENT_H
#define MODE3_EVENT_H

class Mode0Irq;
class IrqEvent;

#include "video_event.h"
#include "video_event_comparer.h"
#include "../event_queue.h"

class Mode3Event : public VideoEvent {
	event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue;
	event_queue<VideoEvent*,VideoEventComparer> &vEventQueue;
	Mode0Irq &mode0Irq;
	IrqEvent &irqEvent;
	
public:
	Mode3Event(event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue_in,
	           event_queue<VideoEvent*,VideoEventComparer> &vEventQueue_in,
	           Mode0Irq &mode0Irq_in, IrqEvent &irqEvent_in);
	
	void doEvent();
	
	static unsigned long schedule(const event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue) {
		return m3EventQueue.empty() ? static_cast<unsigned long>(DISABLED_TIME) : m3EventQueue.top()->time();
	}
};

#endif
