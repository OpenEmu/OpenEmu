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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef KYRA_TIMER_H
#define KYRA_TIMER_H

#include "kyra/kyra_v1.h"

#include "common/list.h"
#include "common/stream.h"
#include "common/func.h"

namespace Kyra {

typedef Common::Functor1<int, void> TimerFunc;

struct TimerEntry {
	uint8 id;
	int32 countdown;
	int8 enabled;

	int32 lastUpdate;
	uint32 nextRun;

	TimerFunc *func;

	uint32 pauseStartTime;
};

class TimerManager {
public:
	TimerManager(KyraEngine_v1 *vm, OSystem *sys) : _vm(vm), _system(sys), _timers(), _nextRun(0), _isPaused(0), _pauseStart(0) {}
	~TimerManager() { reset(); }

	void pause(bool p);

	void reset();

	void addTimer(uint8 id, TimerFunc *func, int countdown, bool enabled);

	int count() const { return _timers.size(); }

	void update();

	void resetNextRun();

	void setCountdown(uint8 id, int32 countdown);
	void setDelay(uint8 id, int32 countdown);
	int32 getDelay(uint8 id) const;
	void setNextRun(uint8 id, uint32 nextRun);
	uint32 getNextRun(uint8 id) const;

	void pauseSingleTimer(uint8 id, bool p);

	bool isEnabled(uint8 id) const;
	void enable(uint8 id);
	void disable(uint8 id);

	void loadDataFromFile(Common::SeekableReadStream &file, int version);
	void saveDataToFile(Common::WriteStream &file) const;

private:
	void resync();

	KyraEngine_v1 *_vm;
	OSystem *_system;
	Common::List<TimerEntry> _timers;
	uint32 _nextRun;

	uint _isPaused;
	uint32 _pauseStart;

	typedef Common::List<TimerEntry>::iterator Iterator;
	typedef Common::List<TimerEntry>::const_iterator CIterator;
};

class PauseTimer {
public:
	PauseTimer(TimerManager &timer) : _timer(timer) { _timer.pause(true); }
	~PauseTimer() { _timer.pause(false); }
private:
	TimerManager &_timer;
};

} // End of namespace Kyra

#endif
