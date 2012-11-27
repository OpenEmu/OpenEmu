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

#ifndef LASTEXPRESS_SOUND_QUEUE_H
#define LASTEXPRESS_SOUND_QUEUE_H

#include "lastexpress/shared.h"

#include "common/array.h"
#include "common/mutex.h"
#include "common/serializer.h"

namespace LastExpress {

class LastExpressEngine;
class SoundEntry;
class SubtitleEntry;

class SoundQueue : Common::Serializable {
public:
	SoundQueue(LastExpressEngine *engine);
	~SoundQueue();

	// Timer
	void handleTimer();

	// Queue
	void addToQueue(SoundEntry *entry);
	void removeFromQueue(Common::String filename);
	void removeFromQueue(EntityIndex entity);
	void updateQueue();
	void resetQueue();
	void resetQueue(SoundType type1, SoundType type2 = kSoundTypeNone);
	void clearQueue();

	// State
	void clearStatus();
	int getSoundState() { return _state; }
	void resetState() { resetState(kSoundState1); }
	void resetState(SoundState state) { _state |= state; }

	// Entries
	void setupEntry(SoundType type, EntityIndex index);
	void processEntry(EntityIndex entity);
	void processEntry(SoundType type);
	void processEntry(Common::String filename);
	void processEntries();
	SoundEntry *getEntry(SoundType type);
	SoundEntry *getEntry(EntityIndex index);
	SoundEntry *getEntry(Common::String name);
	uint32 getEntryTime(EntityIndex index);
	bool isBuffered(Common::String filename, bool testForEntity = false);
	bool isBuffered(EntityIndex entity);

	// Subtitles
	void updateSubtitles();
	void addSubtitle(SubtitleEntry *entry) { _subtitles.push_back(entry); }
	void removeSubtitle(SubtitleEntry *entry) { _subtitles.remove(entry); }
	void setCurrentSubtitle(SubtitleEntry *entry) { _currentSubtitle = entry; }
	SubtitleEntry *getCurrentSubtitle() { return _currentSubtitle; }

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &ser);
	uint32 count();

	// Accessors
	uint32 getFlag() { return _flag; }
	int getSubtitleFlag() { return _subtitlesFlag; }
	void setSubtitleFlag(int flag) { _subtitlesFlag = flag; }
	SoundType getCurrentType() { return _currentType; }
	void setCurrentType(SoundType type) { _currentType = type; }

protected:
	// Debug
	void stopAllSound();

private:
	LastExpressEngine *_engine;

	Common::Mutex _mutex;

	// State & shared data
	int _state;
	SoundType _currentType;
	// TODO: this seems to be a synchronization flag for the sound timer
	uint32 _flag;

	// Entries
	Common::List<SoundEntry *> _soundList;    ///< List of all sound entries
	//void *_soundCacheData;

	// Subtitles
	int _subtitlesFlag;
	Common::List<SubtitleEntry *> _subtitles;
	SubtitleEntry *_currentSubtitle;

	friend class Debugger;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SOUND_QUEUE_H
