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
#ifndef WE_MASTER_CHECKER_H
#define WE_MASTER_CHECKER_H

template<typename T, class Comparer> class event_queue;
class Wy;
class SaveState;

#include "video_event.h"
#include "video_event_comparer.h"
#include "ly_counter.h"
#include "m3_extra_cycles.h"

class WeMasterChecker : public VideoEvent {
	event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue;
	Wy &wy;
	const LyCounter &lyCounter;
	M3ExtraCycles &m3ExtraCycles;
	
	bool weMaster_;
	
	void set(const bool value) {
		if (weMaster_ != value)
			m3ExtraCycles.invalidateCache();
		
		weMaster_ = value;
	}
	
public:
	WeMasterChecker(event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue_in,
	                Wy &wy_in,
	                const LyCounter &lyCounter_in,
	                M3ExtraCycles &m3ExtraCycles);
	
	void doEvent();
	
	static unsigned long schedule(const unsigned wySrc, const bool weSrc, const LyCounter &lyCounter, const unsigned long cycleCounter) {
		if (weSrc && wySrc < 143)
			return lyCounter.nextFrameCycle(wySrc * 456ul + 448 + lyCounter.isDoubleSpeed() * 4, cycleCounter);
		else
			return DISABLED_TIME;
	}
	
	void unset() {
		set(false);
	}
	
	bool weMaster() const {
		return weMaster_;
	}
	
	void saveState(SaveState &state) const;
	void loadState(const SaveState &state);
};

#endif
