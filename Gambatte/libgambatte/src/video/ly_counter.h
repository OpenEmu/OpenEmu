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
#ifndef LY_COUNTER_H
#define LY_COUNTER_H

class SaveState;

#include "video_event.h"
#include "basic_add_event.h"

class LyCounter : public VideoEvent {
	unsigned short lineTime_;
	unsigned char ly_;
	bool ds;
	
public:
	LyCounter();
	
	void doEvent();
	
	bool isDoubleSpeed() const {
		return ds;
	}
	
	unsigned lineCycles(const unsigned long cc) const {
		return 456u - ((time() - cc) >> isDoubleSpeed());
	}
	
	unsigned lineTime() const {
		return lineTime_;
	}
	
	unsigned ly() const {
		return ly_;
	}
	
	unsigned long nextLineCycle(unsigned lineCycle, unsigned long cycleCounter) const;
	unsigned long nextFrameCycle(unsigned long frameCycle, unsigned long cycleCounter) const;
	
	void reset(unsigned long videoCycles, unsigned long lastUpdate);
	
	void setDoubleSpeed(bool ds_in);
};

static inline void addEvent(event_queue<VideoEvent*,VideoEventComparer> &q, LyCounter *const e, const unsigned long newTime) {
	addUnconditionalEvent(q, e, newTime);
}

static inline void addFixedtimeEvent(event_queue<VideoEvent*,VideoEventComparer> &q, LyCounter *const e, const unsigned long newTime) {
	addUnconditionalFixedtimeEvent(q, e, newTime);
}

#endif
