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

#ifndef LASTEXPRESS_SOUND_ENTRY_H
#define LASTEXPRESS_SOUND_ENTRY_H

/*
	Sound entry: 68 bytes (this is what appears in savegames)
	    uint32 {4}      - status
	    uint32 {4}      - type
	    uint32 {4}      - blockCount
	    uint32 {4}      - time
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - entity
	    uint32 {4}      - ??
	    uint32 {4}      - priority
	    char {16}       - name 1
	    char {16}       - name 2

	Sound queue entry: 120 bytes
	    uint16 {2}      - status
	    byte {1}        - type
	    byte {1}        - ??
	    uint32 {4}      - ??
	    uint32 {4}      - currentDataPtr
	    uint32 {4}      - soundData
	    uint32 {4}      - currentBufferPtr
	    uint32 {4}      - blockCount
	    uint32 {4}      - time
	    uint32 {4}      - size
	    uint32 {4}      - ??
	    uint32 {4}      - archive structure pointer
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - ??
	    uint32 {4}      - entity
	    uint32 {4}      - ??
	    uint32 {4}      - priority
	    char {16}       - name 1
	    char {16}       - name 2
	    uint32 {4}      - pointer to next entry in the queue
	    uint32 {4}      - subtitle data pointer
*/

#include "lastexpress/data/snd.h"
#include "lastexpress/data/subtitle.h"

#include "lastexpress/shared.h"

#include "common/serializer.h"

namespace LastExpress {

class LastExpressEngine;
class SubtitleEntry;

union SoundStatusUnion {
	uint32 status;
	byte status1;
	byte status2;
	byte status3;
	byte status4;

	SoundStatusUnion() {
		status = 0;
	}
};

//////////////////////////////////////////////////////////////////////////
// SoundEntry
//////////////////////////////////////////////////////////////////////////
class SoundEntry : Common::Serializable {
public:
	SoundEntry(LastExpressEngine *engine);
	~SoundEntry();

	void open(Common::String name, SoundFlag flag, int priority);
	void close();
	void play();
	void reset();
	bool isFinished();
	void update(uint val);
	bool updateSound();
	void updateState();
	void updateEntryFlag(SoundFlag flag);

	// Subtitles
	void showSubtitle(Common::String filename);

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &ser);

	// Accessors
	void setStatus(int status)         { _status.status = status; }
	void setType(SoundType type)       { _type = type; }
	void setEntity(EntityIndex entity) { _entity = entity; }
	void setField48(int val)           { _field_48 = val; }

	SoundStatusUnion getStatus()   { return _status; }
	SoundType        getType()     { return _type; }
	uint32           getTime()     { return _time; }
	EntityIndex      getEntity()   { return _entity; }
	uint32           getPriority() { return _priority; }
	Common::String   getName2()    { return _name2; }

	// Streams
	SimpleSound *getSoundStream() { return _soundStream; }

private:
	LastExpressEngine *_engine;

	SoundStatusUnion _status;
	SoundType _type;    // int
	//int _data;
	//int _endOffset;
	byte * _currentDataPtr;
	//int _currentBufferPtr;
	int _blockCount;
	uint32 _time;
	//int _size;
	//int _field_28;
	Common::SeekableReadStream *_stream;    // The file stream
	//int _archive;
	int _field_34;
	int _field_38;
	int _field_3C;
	int _variant;
	EntityIndex _entity;
	int _field_48;
	uint32 _priority;
	Common::String _name1;    //char[16];
	Common::String _name2;    //char[16];
	// original has pointer to the next structure in the list (not used)
	SubtitleEntry *_subtitle;

	// Sound buffer & stream
	bool _queued;
	StreamedSound *_soundStream;    // the filtered sound stream

	void setType(SoundFlag flag);
	void setupStatus(SoundFlag flag);
	void loadStream(Common::String name);
};

//////////////////////////////////////////////////////////////////////////
// SubtitleEntry
//////////////////////////////////////////////////////////////////////////
class SubtitleEntry {
public:
	SubtitleEntry(LastExpressEngine *engine);
	~SubtitleEntry();

	void load(Common::String filename, SoundEntry *soundEntry);
	void loadData();
	void draw();
	void setupAndDraw();
	void drawOnScreen();

	// Accessors
	SoundStatusUnion getStatus() { return _status; }
	SoundEntry *getSoundEntry()  { return _sound; }

private:
	LastExpressEngine *_engine;

	Common::String    _filename;
	SoundStatusUnion  _status;
	SoundEntry       *_sound;
	SubtitleManager  *_data;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SOUND_ENTRY_H
