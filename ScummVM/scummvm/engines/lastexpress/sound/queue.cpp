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

#include "lastexpress/sound/queue.h"

#include "lastexpress/game/logic.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/entry.h"
#include "lastexpress/sound/sound.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"

namespace LastExpress {

SoundQueue::SoundQueue(LastExpressEngine *engine) : _engine(engine) {
	 _state = 0;
	 _currentType = kSoundType16;
	 _flag = 0;

	_subtitlesFlag = 0;
	_currentSubtitle = NULL;
	//_soundCacheData = NULL;
}

SoundQueue::~SoundQueue() {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		SAFE_DELETE(*i);
	_soundList.clear();

	for (Common::List<SubtitleEntry *>::iterator i = _subtitles.begin(); i != _subtitles.end(); ++i)
		SAFE_DELETE(*i);
	_subtitles.clear();

	_currentSubtitle = NULL;
	//SAFE_DELETE(_soundCacheData);

	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Timer
//////////////////////////////////////////////////////////////////////////
void SoundQueue::handleTimer() {
	Common::StackLock locker(_mutex);

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		SoundEntry *entry = (*i);
		if (entry == NULL)
			error("[SoundQueue::handleTimer] Invalid entry found in sound queue");

		// When the entry has stopped playing, we remove his buffer
		if (entry->isFinished()) {
			entry->close();
			SAFE_DELETE(entry);
			i = _soundList.reverse_erase(i);
			continue;
		}

		// Queue the entry data, applying filtering
		entry->play();
	}
}

//////////////////////////////////////////////////////////////////////////
// Sound queue management
//////////////////////////////////////////////////////////////////////////
void SoundQueue::addToQueue(SoundEntry *entry) {
	_soundList.push_back(entry);
}

void SoundQueue::removeFromQueue(EntityIndex entity) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(entity);
	if (entry)
		entry->reset();
}

void SoundQueue::removeFromQueue(Common::String filename) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(filename);
	if (entry)
		entry->reset();
}

void SoundQueue::updateQueue() {
	Common::StackLock locker(_mutex);

	++_flag;

	if (getSoundState() & kSoundState1) {
		SoundEntry *entry = getEntry(kSoundType1);
		if (!entry || getFlags()->flag_3 || (entry && entry->getTime() > getSound()->getLoopingSoundDuration())) {
			getSound()->playLoopingSound(0x45);
		} else {
			if (getSound()->getData1() && getSound()->getData2() >= getSound()->getData1()) {
				entry->update(getSound()->getData0());
				getSound()->setData1(0);
			}
		}
	}

	for (Common::List<SoundEntry *>::iterator it = _soundList.begin(); it != _soundList.end(); ++it) {
		SoundEntry *entry = *it;
		if (entry == NULL)
			error("[SoundQueue::updateQueue] Invalid entry found in sound queue");

		// Original removes the entry data from the cache and sets the archive as not loaded
		// and if the sound data buffer is not full, loads a new entry to be played based on
		// its priority and filter id

		if (!entry->updateSound() && !(entry->getStatus().status3 & 0x8)) {
			entry->close();
			SAFE_DELETE(entry);
			it = _soundList.reverse_erase(it);
		}
	}

	// Original update the current entry, loading another set of samples to be decoded

	getFlags()->flag_3 = false;

	--_flag;
}

void SoundQueue::resetQueue() {
	Common::StackLock locker(_mutex);

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getType() == kSoundType1) {
			(*i)->reset();
			break;
		}
	}

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getType() == kSoundType2) {
			(*i)->reset();
			break;
		}
	}
}

void SoundQueue::resetQueue(SoundType type1, SoundType type2) {
	if (!type2)
		type2 = type1;

	Common::StackLock locker(_mutex);

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getType() != type1 && (*i)->getType() != type2)
			(*i)->reset();
	}
}

void SoundQueue::clearQueue() {
	Common::StackLock locker(_mutex);

	_flag |= 8;

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		SoundEntry *entry = (*i);
		if (entry == NULL)
			error("[SoundQueue::clearQueue] Invalid entry found in sound queue");

		// Delete entry
		entry->close();
		SAFE_DELETE(entry);

		i = _soundList.reverse_erase(i);
	}

	updateSubtitles();
}

//////////////////////////////////////////////////////////////////////////
// State
//////////////////////////////////////////////////////////////////////////
void SoundQueue::clearStatus() {
	Common::StackLock locker(_mutex);

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		(*i)->setStatus((*i)->getStatus().status | kSoundStatusClosed);
}

//////////////////////////////////////////////////////////////////////////
// Entry management
//////////////////////////////////////////////////////////////////////////
void SoundQueue::setupEntry(SoundType type, EntityIndex index) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(type);
	if (entry)
		entry->setEntity(index);
}

void SoundQueue::processEntry(EntityIndex entity) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(entity);
	if (entry) {
		entry->update(0);
		entry->setEntity(kEntityPlayer);
	}
}

void SoundQueue::processEntry(SoundType type) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(type);
	if (entry)
		entry->update(0);
}

void SoundQueue::processEntry(Common::String filename) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(filename);
	if (entry) {
		entry->update(0);
		entry->setEntity(kEntityPlayer);
	}
}

void SoundQueue::processEntries() {
	_state = 0;

	processEntry(kSoundType1);
	processEntry(kSoundType2);
}

SoundEntry *SoundQueue::getEntry(EntityIndex index) {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getEntity() == index)
			return *i;
	}

	return NULL;
}

SoundEntry *SoundQueue::getEntry(Common::String name) {
	if (!name.contains('.'))
		name += ".SND";

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getName2() == name)
			return *i;
	}

	return NULL;
}

SoundEntry *SoundQueue::getEntry(SoundType type) {
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i) {
		if ((*i)->getType() == type)
			return *i;
	}

	return NULL;
}

uint32 SoundQueue::getEntryTime(EntityIndex index) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(index);
	if (entry)
		return entry->getTime();

	return 0;
}

bool SoundQueue::isBuffered(EntityIndex entity) {
	Common::StackLock locker(_mutex);

	return (getEntry(entity) != NULL);
}

bool SoundQueue::isBuffered(Common::String filename, bool testForEntity) {
	Common::StackLock locker(_mutex);

	SoundEntry *entry = getEntry(filename);

	if (testForEntity)
		return entry != NULL && entry->getEntity() != kEntityPlayer;

	return (entry != NULL);
}

//////////////////////////////////////////////////////////////////////////
// Subtitles
//////////////////////////////////////////////////////////////////////////
void SoundQueue::updateSubtitles() {
	Common::StackLock locker(_mutex);

	uint32 index = 0;
	SubtitleEntry *subtitle = NULL;

	for (Common::List<SubtitleEntry *>::iterator i = _subtitles.begin(); i != _subtitles.end(); ++i) {
		uint32 current_index = 0;
		SoundEntry *soundEntry = (*i)->getSoundEntry();
		SoundStatus status = (SoundStatus)soundEntry->getStatus().status;

		if (!(status & kSoundStatus_40)
		 || status & kSoundStatus_180
		 || soundEntry->getTime() == 0
		 || (status & kSoundStatusFilter) < 6
		 || ((getFlags()->nis & 0x8000) && soundEntry->getPriority() < 90)) {
			 current_index = 0;
		} else {
			current_index = soundEntry->getPriority() + (status & kSoundStatusFilter);

			if (_currentSubtitle == (*i))
				current_index += 4;
		}

		if (index < current_index) {
			index = current_index;
			subtitle = (*i);
		}
	}

	if (_currentSubtitle == subtitle) {
		if (subtitle)
			subtitle->setupAndDraw();

		return;
	}

	if (!subtitle)
		return;

	if (_subtitlesFlag & 1)
		subtitle->drawOnScreen();

	subtitle->loadData();
	subtitle->setupAndDraw();
}

//////////////////////////////////////////////////////////////////////////
// Savegame
//////////////////////////////////////////////////////////////////////////
void SoundQueue::saveLoadWithSerializer(Common::Serializer &s) {
	Common::StackLock locker(_mutex);

	s.syncAsUint32LE(_state);
	s.syncAsUint32LE(_currentType);

	// Compute the number of entries to save
	uint32 numEntries = count();
	s.syncAsUint32LE(numEntries);

	// Save or load each entry data
	if (s.isSaving()) {
		for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
			(*i)->saveLoadWithSerializer(s);
	} else {
		warning("[Sound::saveLoadWithSerializer] Loading not implemented");

		uint32 unusedDataSize = numEntries * 64;
		if (s.isLoading()) {
			byte *empty = (byte *)malloc(unusedDataSize);
			s.syncBytes(empty, unusedDataSize);
			free(empty);
		} else {
			s.skip(unusedDataSize);
		}
	}
}


// FIXME: We probably need another mutex here to protect during the whole savegame process
// as we could have removed an entry between the time we check the count and the time we
// save the entries
uint32 SoundQueue::count() {
	Common::StackLock locker(_mutex);

	uint32 numEntries = 0;
	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		if ((*i)->getName2().matchString("NISSND?"))
			++numEntries;

	return numEntries;
}

//////////////////////////////////////////////////////////////////////////
// Debug
//////////////////////////////////////////////////////////////////////////
void SoundQueue::stopAllSound() {
	Common::StackLock locker(_mutex);

	for (Common::List<SoundEntry *>::iterator i = _soundList.begin(); i != _soundList.end(); ++i)
		(*i)->getSoundStream()->stop();
}

} // End of namespace LastExpress
