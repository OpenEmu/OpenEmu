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

#include "kyra/timer.h"

#include "common/system.h"

namespace Kyra {

namespace {
struct TimerResync : public Common::UnaryFunction<TimerEntry&, void> {
	uint32 _tickLength, _curTime;
	TimerResync(KyraEngine_v1 *vm, uint32 curTime) : _tickLength(vm->tickLength()), _curTime(curTime) {}

	void operator()(TimerEntry &entry) const {
		if (entry.lastUpdate < 0) {
			if ((uint32)(ABS(entry.lastUpdate)) >= entry.countdown * _tickLength)
				entry.nextRun = 0;
			else
				entry.nextRun = _curTime + entry.lastUpdate + entry.countdown * _tickLength;
		} else {
			uint32 nextRun = entry.lastUpdate + entry.countdown * _tickLength;
			if (_curTime < nextRun)
				nextRun = 0;
			entry.nextRun = nextRun;
		}
	}
};

struct TimerEqual : public Common::UnaryFunction<const TimerEntry&, bool> {
	uint8 _id;

	TimerEqual(uint8 id) : _id(id) {}

	bool operator()(const TimerEntry &entry) const {
		return entry.id == _id;
	}
};
} // end of anonymous namespace

void TimerManager::pause(bool p) {
	if (p) {
		++_isPaused;

		if (_isPaused == 1) {
			_isPaused = true;
			_pauseStart = _system->getMillis();
		}
	} else if (!p && _isPaused > 0) {
		--_isPaused;

		if (_isPaused == 0) {
			const uint32 pausedTime = _system->getMillis() - _pauseStart;
			_nextRun += pausedTime;

			for (Iterator pos = _timers.begin(); pos != _timers.end(); ++pos) {
				pos->lastUpdate += pausedTime;
				pos->nextRun += pausedTime;
			}
		}
	}
}

void TimerManager::reset() {
	for (Iterator pos = _timers.begin(); pos != _timers.end(); ++pos)
		delete pos->func;

	_timers.clear();
}

void TimerManager::addTimer(uint8 id, TimerFunc *func, int countdown, bool enabled) {
	Iterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(id));
	if (timer != _timers.end()) {
		warning("Adding allready existing timer %d", id);
		return;
	}

	TimerEntry newTimer;

	newTimer.id = id;
	newTimer.countdown = countdown;
	newTimer.enabled = enabled ? 1 : 0;
	newTimer.lastUpdate = newTimer.nextRun = 0;
	newTimer.func = func;
	newTimer.pauseStartTime = 0;

	_timers.push_back(newTimer);
}

void TimerManager::update() {
	if (_system->getMillis() < _nextRun || _isPaused)
		return;

	_nextRun += 99999;

	for (Iterator pos = _timers.begin(); pos != _timers.end(); ++pos) {
		if (pos->enabled == 1 && pos->countdown >= 0) {
			if (pos->nextRun <= _system->getMillis()) {
				if (pos->func && pos->func->isValid()) {
					(*pos->func)(pos->id);
				}

				uint32 curTime = _system->getMillis();
				pos->lastUpdate = curTime;
				pos->nextRun = curTime + pos->countdown * _vm->tickLength();
			}

			_nextRun = MIN(_nextRun, pos->nextRun);
		}
	}
}

void TimerManager::resync() {
	const uint32 curTime = _isPaused ? _pauseStart : _system->getMillis();

	_nextRun = 0;	// force rerun
	Common::for_each(_timers.begin(), _timers.end(), TimerResync(_vm, curTime));
}

void TimerManager::resetNextRun() {
	_nextRun = 0;
}

void TimerManager::setCountdown(uint8 id, int32 countdown) {
	Iterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(id));
	if (timer != _timers.end()) {
		timer->countdown = countdown;

		if (countdown >= 0) {
			uint32 curTime = _system->getMillis();
			timer->lastUpdate = curTime;
			timer->nextRun = curTime + countdown * _vm->tickLength();
			if (timer->enabled & 2)
				timer->pauseStartTime = curTime;

			_nextRun = MIN(_nextRun, timer->nextRun);
		}
	} else {
		warning("TimerManager::setCountdown: No timer %d", id);
	}
}

void TimerManager::setDelay(uint8 id, int32 countdown) {
	Iterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(id));
	if (timer != _timers.end())
		timer->countdown = countdown;
	else
		warning("TimerManager::setDelay: No timer %d", id);
}

int32 TimerManager::getDelay(uint8 id) const {
	CIterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(id));
	if (timer != _timers.end())
		return timer->countdown;

	warning("TimerManager::getDelay: No timer %d", id);
	return -1;
}

void TimerManager::setNextRun(uint8 id, uint32 nextRun) {
	Iterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(id));
	if (timer != _timers.end()) {
		if (timer->enabled & 2)
			timer->pauseStartTime = _system->getMillis();
		timer->nextRun = nextRun;
		return;
	}

	warning("TimerManager::setNextRun: No timer %d", id);
}

uint32 TimerManager::getNextRun(uint8 id) const {
	CIterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(id));
	if (timer != _timers.end())
		return timer->nextRun;

	warning("TimerManager::getNextRun: No timer %d", id);
	return 0xFFFFFFFF;
}

void TimerManager::pauseSingleTimer(uint8 id, bool p) {
	Iterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(id));

	if (timer == _timers.end()) {
		warning("TimerManager::pauseSingleTimer: No timer %d", id);
		return;
	}

	if (p) {
		timer->pauseStartTime = _system->getMillis();
		timer->enabled |= 2;
	} else if (timer->pauseStartTime) {
		int32 elapsedTime = _system->getMillis() - timer->pauseStartTime;
		timer->enabled &= (~2);
		timer->lastUpdate += elapsedTime;
		timer->nextRun += elapsedTime;
		resetNextRun();
		timer->pauseStartTime = 0;
	}
}

bool TimerManager::isEnabled(uint8 id) const {
	CIterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(id));
	if (timer != _timers.end())
		return (timer->enabled & 1);

	warning("TimerManager::isEnabled: No timer %d", id);
	return false;
}

void TimerManager::enable(uint8 id) {
	Iterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(id));
	if (timer != _timers.end())
		timer->enabled |= 1;
	else
		warning("TimerManager::enable: No timer %d", id);
}

void TimerManager::disable(uint8 id) {
	Iterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(id));
	if (timer != _timers.end())
		timer->enabled &= (~1);
	else
		warning("TimerManager::disable: No timer %d", id);
}

void TimerManager::loadDataFromFile(Common::SeekableReadStream &file, int version) {
	const uint32 loadTime = _isPaused ? _pauseStart : _system->getMillis();

	if (version <= 7) {
		_nextRun = 0;
		for (int i = 0; i < 32; ++i) {
			uint8 enabled = file.readByte();
			int32 countdown = file.readSint32BE();
			uint32 nextRun = file.readUint32BE();

			Iterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(i));
			if (timer != _timers.end()) {
				timer->enabled = enabled;
				timer->countdown = countdown;

				if (nextRun) {
					timer->nextRun = nextRun + loadTime;
					timer->lastUpdate = timer->nextRun - countdown * _vm->tickLength();
				} else {
					timer->nextRun = loadTime;
					timer->lastUpdate = loadTime - countdown * _vm->tickLength();
				}
			} else {
				warning("Loading timer data for non existing timer %d", i);
			}
		}
	} else {
		int entries = file.readByte();
		for (int i = 0; i < entries; ++i) {
			uint8 id = file.readByte();

			Iterator timer = Common::find_if(_timers.begin(), _timers.end(), TimerEqual(id));
			if (timer != _timers.end()) {
				timer->enabled = file.readByte();
				timer->countdown = file.readSint32BE();
				timer->lastUpdate = file.readSint32BE();
			} else {
				warning("Loading timer data for non existing timer %d", id);
				file.seek(7, SEEK_CUR);
			}
		}

		resync();
	}
}

void TimerManager::saveDataToFile(Common::WriteStream &file) const {
	const uint32 saveTime = _isPaused ? _pauseStart : _system->getMillis();

	file.writeByte(count());
	for (CIterator pos = _timers.begin(); pos != _timers.end(); ++pos) {
		file.writeByte(pos->id);
		file.writeByte(pos->enabled);
		file.writeSint32BE(pos->countdown);
		file.writeSint32BE(pos->lastUpdate - saveTime);
	}
}

} // End of namespace Kyra
