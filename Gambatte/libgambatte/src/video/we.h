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
#ifndef WE_H
#define WE_H

class SaveState;

#include "video_event.h"
#include "ly_counter.h"
#include "m3_extra_cycles.h"
#include "basic_add_event.h"

class We {
public:
	class WeEnableChecker : public VideoEvent {
		We &we;
		
	public:
		WeEnableChecker(We &we);
		
		void doEvent();
		
		static unsigned long schedule(const unsigned scxAnd7, const unsigned wx, const LyCounter &lyCounter, const unsigned long cycleCounter) {
			return lyCounter.nextLineCycle(scxAnd7 + 82 + wx + lyCounter.isDoubleSpeed() * 3, cycleCounter);
		}
	};
	
	class WeDisableChecker : public VideoEvent {
		We &we;
		
	public:
		WeDisableChecker(We &we);
		
		void doEvent();
		
		static unsigned long schedule(const unsigned scxAnd7, const unsigned wx, const LyCounter &lyCounter, const unsigned long cycleCounter) {
			return lyCounter.nextLineCycle(scxAnd7 + 88 + wx + lyCounter.isDoubleSpeed() * 3, cycleCounter);
		}
	};
	
	friend class WeEnableChecker;
	friend class WeDisableChecker;
	
private:
	M3ExtraCycles &m3ExtraCycles_;
	WeEnableChecker enableChecker_;
	WeDisableChecker disableChecker_;
	
	bool we_;
	bool src_;
	
	void set(const bool value) {
		if (we_ != value)
			m3ExtraCycles_.invalidateCache();
		
		we_ = value;
	}
	
public:
	We(M3ExtraCycles &m3ExtraCycles);
	
	WeDisableChecker& disableChecker() {
		return disableChecker_;
	}
	
	WeEnableChecker& enableChecker() {
		return enableChecker_;
	}
	
	bool getSource() const {
		return src_;
	}
	
	void setSource(const bool src) {
		src_ = src;
	}
	
	bool value() const {
		return we_;
	}
	
	void saveState(SaveState &state) const;
	void loadState(const SaveState &state);
};

static inline void addEvent(event_queue<VideoEvent*,VideoEventComparer> &q, We::WeEnableChecker *const e, const unsigned long newTime) {
	addUnconditionalEvent(q, e, newTime);
}

static inline void addFixedtimeEvent(event_queue<VideoEvent*,VideoEventComparer> &q, We::WeEnableChecker *const e, const unsigned long newTime) {
	addUnconditionalFixedtimeEvent(q, e, newTime);
}

static inline void addEvent(event_queue<VideoEvent*,VideoEventComparer> &q, We::WeDisableChecker *const e, const unsigned long newTime) {
	addUnconditionalEvent(q, e, newTime);
}

static inline void addFixedtimeEvent(event_queue<VideoEvent*,VideoEventComparer> &q, We::WeDisableChecker *const e, const unsigned long newTime) {
	addUnconditionalFixedtimeEvent(q, e, newTime);
}

#endif
