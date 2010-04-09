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
#ifndef WX_READER_H
#define WX_READER_H

template<typename T, class Comparer> class event_queue;
class M3ExtraCycles;
class SaveState;

#include "video_event.h"
#include "video_event_comparer.h"
#include "ly_counter.h"
#include "basic_add_event.h"
#include <algorithm>

class WxReader : public VideoEvent {
	event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue;
	VideoEvent &weEnableChecker;
	VideoEvent &weDisableChecker;
	M3ExtraCycles &m3ExtraCycles;

	unsigned char wx_;
	unsigned char src_;
	bool dS;
	
public:
	WxReader(event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue_in,
	         VideoEvent &weEnableChecker_in,
	         VideoEvent &weDisableChecker_in,
	         M3ExtraCycles &m3ExtraCycles);
	
	void doEvent();
	
	unsigned getSource() const {
		return src_;
	}
	
	unsigned wx() const {
		return wx_;
	}
	
	void setDoubleSpeed(const bool dS_in) {
		dS = dS_in;
	}
	
	void setSource(const unsigned src) {
		src_ = src;
	}
	
	static unsigned long schedule(const unsigned scxAnd7, const LyCounter &lyCounter, const WxReader &wxReader, const unsigned long cycleCounter) {
		return lyCounter.nextLineCycle(scxAnd7 + 82 + lyCounter.isDoubleSpeed() * 3 + std::min(wxReader.getSource(), wxReader.wx()), cycleCounter);
		//setTime(lyCounter.nextLineCycle(scxAnd7 + 89 + lyCounter.isDoubleSpeed() * 3, cycleCounter));
	}
	
	void saveState(SaveState &state) const;
	void loadState(const SaveState &state);
};

static inline void addEvent(event_queue<VideoEvent*,VideoEventComparer> &q, WxReader *const e, const unsigned long newTime) {
	addUnconditionalEvent(q, e, newTime);
}

static inline void addFixedtimeEvent(event_queue<VideoEvent*,VideoEventComparer> &q, WxReader *const e, const unsigned long newTime) {
	addUnconditionalFixedtimeEvent(q, e, newTime);
}

#endif
