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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_EVENTRECORDER_H
#define COMMON_EVENTRECORDER_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/singleton.h"
#include "common/mutex.h"
#include "common/array.h"

#define g_eventRec (Common::EventRecorder::instance())

namespace Common {

class RandomSource;
class SeekableReadStream;
class WriteStream;

/**
 * Our generic event recorder.
 *
 * TODO: Add more documentation.
 */
class EventRecorder : private EventSource, private EventObserver, public Singleton<EventRecorder> {
	friend class Singleton<SingletonBaseType>;
	EventRecorder();
	~EventRecorder();
public:
	void init();
	void deinit();

	/** Register random source so it can be serialized in game test purposes */
	void registerRandomSource(RandomSource &rnd, const String &name);

	/** TODO: Add documentation, this is only used by the backend */
	void processMillis(uint32 &millis);

	/** TODO: Add documentation, this is only used by the backend */
	bool processDelayMillis(uint &msecs);

private:
	bool notifyEvent(const Event &ev);
	bool notifyPoll();
	bool pollEvent(Event &ev);
	bool allowMapping() const { return false; }

	class RandomSourceRecord {
	public:
		String name;
		uint32 seed;
	};
	Array<RandomSourceRecord> _randomSourceRecords;

	bool _recordSubtitles;
	volatile uint32 _recordCount;
	volatile uint32 _lastRecordEvent;
	volatile uint32 _recordTimeCount;
	volatile uint32 _lastEventMillis;
	WriteStream *_recordFile;
	WriteStream *_recordTimeFile;
	MutexRef _timeMutex;
	MutexRef _recorderMutex;
	volatile uint32 _lastMillis;

	volatile uint32 _playbackCount;
	volatile uint32 _playbackDiff;
	volatile bool _hasPlaybackEvent;
	volatile uint32 _playbackTimeCount;
	Event _playbackEvent;
	SeekableReadStream *_playbackFile;
	SeekableReadStream *_playbackTimeFile;

	volatile uint32 _eventCount;
	volatile uint32 _lastEventCount;

	enum RecordMode {
		kPassthrough = 0,
		kRecorderRecord = 1,
		kRecorderPlayback = 2
	};
	volatile RecordMode _recordMode;
	String _recordFileName;
	String _recordTempFileName;
	String _recordTimeFileName;
};

} // End of namespace Common

#endif
