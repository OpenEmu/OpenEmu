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

#include "lastexpress/sound/entry.h"

#include "lastexpress/game/logic.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"
#include "lastexpress/sound/sound.h"

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"


namespace LastExpress {

#define SOUNDCACHE_ENTRY_SIZE 92160
#define FILTER_BUFFER_SIZE 2940

//////////////////////////////////////////////////////////////////////////
// SoundEntry
//////////////////////////////////////////////////////////////////////////
SoundEntry::SoundEntry(LastExpressEngine *engine) : _engine(engine) {
	_type = kSoundTypeNone;

	_currentDataPtr = NULL;

	_blockCount = 0;
	_time = 0;

	_stream = NULL;

	_field_34 = 0;
	_field_38 = 0;
	_field_3C = 0;
	_variant = 0;
	_entity = kEntityPlayer;
	_field_48 = 0;
	_priority = 0;

	_subtitle = NULL;

	_soundStream = NULL;

	_queued = false;
}

SoundEntry::~SoundEntry() {
	// Entries that have been queued will have their streamed disposed automatically
	if (!_soundStream)
		SAFE_DELETE(_stream);

	SAFE_DELETE(_soundStream);

	free(_currentDataPtr);

	_subtitle = NULL;
	_stream = NULL;

	// Zero passed pointers
	_engine = NULL;
}

void SoundEntry::open(Common::String name, SoundFlag flag, int priority) {
	_priority = priority;
	setType(flag);
	setupStatus(flag);
	loadStream(name);
}

void SoundEntry::close() {
	_status.status |= kSoundStatusClosed;

	// Loop until ready
	//while (!(_status.status1 & 4) && !(getSoundQueue()->getFlag() & 8) && (getSoundQueue()->getFlag() & 1))
	//	;	// empty loop body

	// The original game remove the entry from the cache here,
	// but since we are called from within an iterator loop
	// we will remove the entry there
	// removeFromCache(entry);

	if (_subtitle) {
		_subtitle->draw();
		SAFE_DELETE(_subtitle);
	}

	if (_entity) {
		if (_entity == kEntitySteam)
			getSound()->playLoopingSound(2);
		else if (_entity != kEntityTrain)
			getSavePoints()->push(kEntityPlayer, _entity, kActionEndSound);
	}
}

void SoundEntry::play() {
	if (!_stream)
		error("[SoundEntry::play] stream has been disposed");

	// Prepare sound stream
	if (!_soundStream)
		_soundStream = new StreamedSound();

	// Compute current filter id
	int32 filterId = _status.status & kSoundStatusFilter;
	// TODO adjust status (based on stepIndex)

	if (_queued) {
		_soundStream->setFilterId(filterId);
	} else {
		_stream->seek(0);

		// Load the stream and start playing
		_soundStream->load(_stream, filterId);

		_queued = true;
	}
}

bool SoundEntry::isFinished() {
	if (!_stream)
		return true;

	if (!_soundStream || !_queued)
		return false;

	// TODO check that all data has been queued
	return _soundStream->isFinished();
}

void SoundEntry::setType(SoundFlag flag) {
	switch (flag & kFlagType9) {
	default:
	case kFlagNone:
		_type = getSoundQueue()->getCurrentType();
		getSoundQueue()->setCurrentType((SoundType)(_type + 1));
		break;

	case kFlagType1_2: {
		SoundEntry *previous2 = getSoundQueue()->getEntry(kSoundType2);
		if (previous2)
			previous2->update(0);

		SoundEntry *previous = getSoundQueue()->getEntry(kSoundType1);
		if (previous) {
			previous->setType(kSoundType2);
			previous->update(0);
		}

		_type = kSoundType1;
		}
		break;

	case kFlagType3: {
		SoundEntry *previous = getSoundQueue()->getEntry(kSoundType3);
		if (previous) {
			previous->setType(kSoundType4);
			previous->update(0);
		}

		_type = kSoundType11;
		}
		break;

	case kFlagType7: {
		SoundEntry *previous = getSoundQueue()->getEntry(kSoundType7);
		if (previous)
			previous->setType(kSoundType8);

		_type = kSoundType7;
		}
		break;

	case kFlagType9: {
		SoundEntry *previous = getSoundQueue()->getEntry(kSoundType9);
		if (previous)
			previous->setType(kSoundType10);

		_type = kSoundType9;
		}
		break;

	case kFlagType11: {
		SoundEntry *previous = getSoundQueue()->getEntry(kSoundType11);
		if (previous)
			previous->setType(kSoundType14);

		_type = kSoundType11;
		}
		break;

	case kFlagType13: {
		SoundEntry *previous = getSoundQueue()->getEntry(kSoundType13);
		if (previous)
			previous->setType(kSoundType14);

		_type = kSoundType13;
		}
		break;
	}
}

void SoundEntry::setupStatus(SoundFlag flag) {
	SoundStatus statusFlag = (SoundStatus)flag;
	if (!((statusFlag & 0xFF) & kSoundStatusFilter))
		statusFlag = (SoundStatus)(statusFlag | kSoundStatusCached);

	if (((statusFlag & 0xFF00) >> 8) & kSoundStatusClear0)
		_status.status = (uint32)statusFlag;
	else
		_status.status = (statusFlag | kSoundStatusClear4);
}

void SoundEntry::loadStream(Common::String name) {
	_name2 = name;

	// Load sound data
	_stream = getArchive(name);

	if (!_stream)
		_stream = getArchive("DEFAULT.SND");

	if (!_stream)
		_status.status = kSoundStatusClosed;
}

void SoundEntry::update(uint val) {
	if (!(_status.status3 & 64)) {
		int value2 = val;

		_status.status |= kSoundStatus_100000;

		if (val) {
			if (getSoundQueue()->getFlag() & 32) {
				_variant = val;
				value2 = val * 2 + 1;
			}

			_field_3C = value2;
		} else {
			_field_3C = 0;
			_status.status |= kSoundStatus_40000000;
		}
	}
}

bool SoundEntry::updateSound() {
	bool result;
	char sub[16];

	if (_status.status2 & 4) {
		result = false;
	} else {
		if (_status.status2 & 0x80) {
			if (_field_48 <= getSound()->getData2()) {
				_status.status |= 0x20;
				_status.status &= ~0x8000;
				strcpy(sub, _name2.c_str());

				int l = strlen(sub) + 1;
				if (l - 1 > 4)
					sub[l - (1 + 4)] = 0;
				showSubtitle(sub);
			}
		} else {
			if (!(getSoundQueue()->getFlag() & 0x20)) {
				if (!(_status.status3 & 8)) {
					if (_entity) {
						if (_entity < 0x80) {
							updateEntryFlag(getSound()->getSoundFlag(_entity));
						}
					}
				}
			}
			//if (status.status2 & 0x40 && !((uint32)_status.status & 0x180) && v1->soundBuffer)
			//	Sound_FillSoundBuffer(v1);
		}
		result = true;
	}

	return result;
}

void SoundEntry::updateEntryFlag(SoundFlag flag) {
	if (flag) {
		if (getSoundQueue()->getFlag() & 0x20 && _type != kSoundType9 && _type != kSoundType7)
			update(flag);
		else
			_status.status = flag + (_status.status & ~0x1F);
	} else {
		_variant = 0;
		_status.status |= 0x80u;
		_status.status &= ~0x10001F;
	}
}

void SoundEntry::updateState() {
	if (getSoundQueue()->getFlag() & 32) {
		if (_type != kSoundType9 && _type != kSoundType7 && _type != kSoundType5) {
			uint32 variant = _status.status & kSoundStatusFilter;

			_status.status &= kSoundStatusClearAll;

			_variant = variant;
			_status.status |= variant * 2 + 1;
		}
	}

	_status.status |= kSoundStatus_20;
}

void SoundEntry::reset() {
	_status.status |= kSoundStatusClosed;
	_entity = kEntityPlayer;

	if (_stream) {
		if (!_soundStream) {
			SAFE_DELETE(_stream);
		} else {
			// the original stream will be disposed
			_soundStream->stop();
			SAFE_DELETE(_soundStream);
		}

		_stream = NULL;
	}
}

void SoundEntry::showSubtitle(Common::String filename) {
	_subtitle = new SubtitleEntry(_engine);
	_subtitle->load(filename, this);

	if (_subtitle->getStatus().status2 & 4) {
		_subtitle->draw();
		SAFE_DELETE(_subtitle);
	} else {
		_status.status |= kSoundStatus_20000;
	}
}

void SoundEntry::saveLoadWithSerializer(Common::Serializer &s) {
	if (_name2.matchString("NISSND?") && (_status.status & kFlagType7) != kFlag3) {
		s.syncAsUint32LE(_status.status);
		s.syncAsUint32LE(_type);
		s.syncAsUint32LE(_blockCount); // field_8;
		s.syncAsUint32LE(_time);
		s.syncAsUint32LE(_field_34); // field_10;
		s.syncAsUint32LE(_field_38); // field_14;
		s.syncAsUint32LE(_entity);

		uint32 delta = (uint32)_field_48 - getSound()->getData2();
		if (delta > kFlag8)
			delta = 0;
		s.syncAsUint32LE(delta);

		s.syncAsUint32LE(_priority);

		char name1[16];
		strcpy((char *)&name1, _name1.c_str());
		s.syncBytes((byte *)&name1, 16);

		char name2[16];
		strcpy((char *)&name2, _name2.c_str());
		s.syncBytes((byte *)&name2, 16);
	}
}

//////////////////////////////////////////////////////////////////////////
// SubtitleEntry
//////////////////////////////////////////////////////////////////////////
SubtitleEntry::SubtitleEntry(LastExpressEngine *engine) : _engine(engine) {
	_sound = NULL;
	_data = NULL;
}

SubtitleEntry::~SubtitleEntry() {
	SAFE_DELETE(_data);

	// Zero-out passed pointers
	_sound = NULL;
	_engine = NULL;
}

void SubtitleEntry::load(Common::String filename, SoundEntry *soundEntry) {
	// Add ourselves to the list of active subtitles
	getSoundQueue()->addSubtitle(this);

	// Set sound entry and filename
	_filename = filename + ".SBE";
	_sound = soundEntry;

	// Load subtitle data
	if (_engine->getResourceManager()->hasFile(_filename)) {
		if (getSoundQueue()->getSubtitleFlag() & 2)
			return;

		loadData();
	} else {
		_status.status = kSoundStatus_400;
	}
}

void SubtitleEntry::loadData() {
	_data = new SubtitleManager(_engine->getFont());
	_data->load(getArchive(_filename));

	getSoundQueue()->setSubtitleFlag(getSoundQueue()->getSubtitleFlag() | 2);
	getSoundQueue()->setCurrentSubtitle(this);
}

void SubtitleEntry::setupAndDraw() {
	if (!_sound)
		error("[SubtitleEntry::setupAndDraw] Sound entry not initialized");

	if (!_data) {
		_data = new SubtitleManager(_engine->getFont());
		_data->load(getArchive(_filename));
	}

	if (_data->getMaxTime() > _sound->getTime()) {
		_status.status = kSoundStatus_400;
	} else {
		_data->setTime((uint16)_sound->getTime());

		if (getSoundQueue()->getSubtitleFlag() & 1)
			drawOnScreen();
	}

	getSoundQueue()->setCurrentSubtitle(this);

	// TODO Missing code
}

void SubtitleEntry::draw() {
	// Remove ourselves from the queue
	getSoundQueue()->removeSubtitle(this);

	if (this == getSoundQueue()->getCurrentSubtitle()) {
		drawOnScreen();

		getSoundQueue()->setCurrentSubtitle(NULL);
		getSoundQueue()->setSubtitleFlag(0);
	}
}

void SubtitleEntry::drawOnScreen() {
	if (_data == NULL)
		return;

	getSoundQueue()->setSubtitleFlag(getSoundQueue()->getSubtitleFlag() & -2);
	_engine->getGraphicsManager()->draw(_data, GraphicsManager::kBackgroundOverlay);
}

} // End of namespace LastExpress
