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
#ifndef SCX_READER_H
#define SCX_READER_H

template<typename T, class Comparer> class event_queue;
class M3ExtraCycles;
class SaveState;

#include "video_event.h"
#include "video_event_comparer.h"
#include "ly_counter.h"
#include "basic_add_event.h"

class ScxReader : public VideoEvent {
	event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue;
// 	VideoEvent &wyReader3;
	VideoEvent &wxReader;
	VideoEvent &weEnableChecker;
	VideoEvent &weDisableChecker;
	M3ExtraCycles &m3ExtraCycles;
	
	unsigned char scxAnd7_;
	unsigned char src;
	bool dS;
	
public:
	ScxReader(event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue_in,
// 	          VideoEvent &wyReader3_in,
	          VideoEvent &wxReader_in,
	          VideoEvent &weEnableChecker_in,
	          VideoEvent &weDisableChecker_in,
	          M3ExtraCycles &m3ExtraCycles);
	
	void doEvent();
	
	unsigned getSource() const {
		return src;
	}
	
	static unsigned long schedule(const LyCounter &lyCounter, const unsigned long cycleCounter) {
		return lyCounter.nextLineCycle(82 + lyCounter.isDoubleSpeed() * 3, cycleCounter);
	}
	
	unsigned scxAnd7() const {
		return scxAnd7_;
	}
	
	void setDoubleSpeed(const bool dS_in) {
		dS = dS_in;
	}
	
	void setSource(const unsigned scxSrc) {
		src = scxSrc & 7;
	}
	
	void saveState(SaveState &state) const;
	void loadState(const SaveState &state);
};

static inline void addEvent(event_queue<VideoEvent*,VideoEventComparer> &q, ScxReader *const e, const unsigned long newTime) {
	addUnconditionalEvent(q, e, newTime);
}

static inline void addFixedtimeEvent(event_queue<VideoEvent*,VideoEventComparer> &q, ScxReader *const e, const unsigned long newTime) {
	addUnconditionalFixedtimeEvent(q, e, newTime);
}

#endif
