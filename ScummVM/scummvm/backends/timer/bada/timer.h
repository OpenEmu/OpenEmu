/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BADA_TIMER_H
#define BADA_TIMER_H

#include <FBase.h>

#include "common/timer.h"
#include "common/list.h"

using namespace Osp::Base::Runtime;

struct TimerSlot: public ITimerEventListener, public Thread {
	TimerSlot(Common::TimerManager::TimerProc callback,
						uint32 interval,
						void *refCon);
	~TimerSlot();

	bool OnStart(void);
	void OnStop(void);
	void OnTimerExpired(Timer &timer);

	Timer *_timer;
	Common::TimerManager::TimerProc _callback;
	uint32 _interval;	// in microseconds
	void *_refCon;
};

class BadaTimerManager : public Common::TimerManager {
public:
	BadaTimerManager();
	~BadaTimerManager();

	bool installTimerProc(TimerProc proc, int32 interval, void *refCon,
												const Common::String &id);
	void removeTimerProc(TimerProc proc);

private:
	Common::List<TimerSlot> _timers;
};

#endif
