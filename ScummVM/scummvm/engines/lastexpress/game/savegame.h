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

#ifndef LASTEXPRESS_SAVELOAD_H
#define LASTEXPRESS_SAVELOAD_H

/*
	Savegame format
	---------------

	header: 32 bytes
	    uint32 {4}      - signature: 0x12001200
	    uint32 {4}      - chapter - needs to be [0; 5]
	    uint32 {4}      - time - needs to be >= 32 [1061100; timeMax]
	    uint32 {4}      - ?? needs to be >= 32
	    uint32 {4}      - ?? needs to be = 1
	    uint32 {4}      - Brightness (needs to be [0-6])
	    uint32 {4}      - Volume (needs to be [0-7])
	    uint32 {4}      - ?? needs to be = 9

	Game data Format
	-----------------

	uint32 {4}      - entity
	uint32 {4}      - current time
	uint32 {4}      - time delta (how much a tick is in "real" time)
	uint32 {4}      - time ticks
	uint32 {4}      - scene Index               max: 2500
	byte {1}        - use backup scene
	uint32 {4}      - backup Scene Index 1      max: 2500
	uint32 {4}      - backup Scene Index 2      max: 2500
	uint32 {4}      - selected inventory item   max: 32
	uint32 {4*100*10} - positions (by car)
	uint32 {4*16}   - compartments
	uint32 {4*16}   - compartments ??
	uint32 {4*128}  - game progress
	byte {512}      - game events
	byte {7*32}     - inventory
	byte {5*128}    - objects
	byte {1262*40}  - entities (characters and train entities)

	uint32 {4}      - sound queue state
	uint32 {4}      - ??
	uint32 {4}      - number of sound entries
	byte {count*68} - sound entries

	byte {16*128}   - save point data
	uint32 {4}      - number of save points (max: 128)
	byte {count*16} - save points

	... more unknown stuff

*/

#include "lastexpress/shared.h"

#include "common/savefile.h"
#include "common/serializer.h"
#include "common/memstream.h"

namespace LastExpress {

// Savegame signatures
#define SAVEGAME_SIGNATURE       0x12001200    // 301994496
#define SAVEGAME_ENTRY_SIGNATURE 0xE660E660    // 3865110112

#define WRAP_SYNC_FUNCTION(instance, className, method) \
	new Common::Functor1Mem<Common::Serializer &, void, className>(instance, &className::method)

class LastExpressEngine;

class SavegameStream : public Common::MemoryWriteStreamDynamic, public Common::SeekableReadStream {
public:
	SavegameStream() : MemoryWriteStreamDynamic(DisposeAfterUse::YES), _eos(false) {
		_enableCompression = false;
		_bufferOffset = -1;
		_valueCount = 0;
		_previousValue = 0;
		_repeatCount = 0;
		_offset = 0;
		_status = kStatusReady;

		memset(_buffer, 0, 256);
	}

	int32 pos() const { return MemoryWriteStreamDynamic::pos(); }
	int32 size() const { return MemoryWriteStreamDynamic::size(); }
	bool seek(int32 offset, int whence = SEEK_SET) { return MemoryWriteStreamDynamic::seek(offset, whence); }
	bool eos() const { return _eos; }
	uint32 read(void *dataPtr, uint32 dataSize);
	uint32 write(const void *dataPtr, uint32 dataSize);

	uint32 process();

private:
	enum CompressedStreamStatus {
		kStatusReady,
		kStatusReading,
		kStatusWriting
	};

	uint32 readUncompressed(void *dataPtr, uint32 dataSize);

	// Compressed data
	uint32 writeCompressed(const void *dataPtr, uint32 dataSize);
	uint32 readCompressed(void *dataPtr, uint32 dataSize);

	void writeBuffer(uint8 value, bool onlyValue = true);
	uint8 readBuffer();

private:
	bool _eos;

	// Compression handling
	bool                   _enableCompression;
	int16                  _bufferOffset;
	byte                   _valueCount;
	byte                   _previousValue;
	int16                  _repeatCount;
	uint32                 _offset;
	CompressedStreamStatus _status;

	byte _buffer[256];
};

class SaveLoad {
public:
	SaveLoad(LastExpressEngine *engine);
	~SaveLoad();

	// Init
	void create(GameId id);
	void clear(bool clearStream = false);
	uint32 init(GameId id, bool resetHeaders);

	// Save & Load
	void loadLastGame();
	void loadGame(uint32 index);
	void saveGame(SavegameType type, EntityIndex entity, uint32 value);

	void loadVolumeBrightness();
	void saveVolumeBrightness();

	// Getting information
	static bool isSavegamePresent(GameId id);
	static bool isSavegameValid(GameId id);

	bool isGameFinished(uint32 menuIndex, uint32 savegameIndex);

	// Accessors
 	uint32       getTime(uint32 index) { return getEntry(index)->time; }
	ChapterIndex getChapter(uint32 index) { return getEntry(index)->chapter; }
	uint32       getValue(uint32 index) { return getEntry(index)->value; }
	uint32       getLastSavegameTicks() const { return _gameTicksLastSavegame; }

private:
	LastExpressEngine *_engine;

	struct SavegameMainHeader : Common::Serializable {
		uint32 signature;
		uint32 count;
		uint32 offset;
		uint32 offsetEntry;
		uint32 keepIndex;
		int32 brightness;
		int32 volume;
		uint32 field_1C;

		SavegameMainHeader() {
			signature = SAVEGAME_SIGNATURE;
			count = 0;
			offset = 32;
			offsetEntry = 32;
			keepIndex = 0;
			brightness = 3;
			volume = 7;
			field_1C = 9;
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(signature);
			s.syncAsUint32LE(count);
			s.syncAsUint32LE(offset);
			s.syncAsUint32LE(offsetEntry);
			s.syncAsUint32LE(keepIndex);
			s.syncAsUint32LE(brightness);
			s.syncAsUint32LE(volume);
			s.syncAsUint32LE(field_1C);
		}

		bool isValid() {
			if (signature != SAVEGAME_SIGNATURE)
				return false;

			/* Check not needed as it can never be < 0
			if (header.chapter < 0)
				return false;*/

			if (offset < 32)
				return false;

			if (offsetEntry < 32)
				return false;

			if (keepIndex != 1 && keepIndex != 0)
				return false;

			if (brightness < 0 || brightness > 6)
				return false;

			if (volume < 0 || volume > 7)
				return false;

			if (field_1C != 9)
				return false;

			return true;
		}
	};

	struct SavegameEntryHeader : Common::Serializable {
		uint32 signature;
		SavegameType type;
		uint32 time;
		int offset;
		ChapterIndex chapter;
		uint32 value;
		int field_18;
		int field_1C;

		SavegameEntryHeader() {
			signature = SAVEGAME_ENTRY_SIGNATURE;
			type = kSavegameTypeIndex;
			time = kTimeNone;
			offset = 0;
			chapter = kChapterAll;
			value = 0;
			field_18 = 0;
			field_1C = 0;
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(signature);
			s.syncAsUint32LE(type);
			s.syncAsUint32LE(time);
			s.syncAsUint32LE(offset);
			s.syncAsUint32LE(chapter);
			s.syncAsUint32LE(value);
			s.syncAsUint32LE(field_18);
			s.syncAsUint32LE(field_1C);
		}

		bool isValid() {
			if (signature != SAVEGAME_ENTRY_SIGNATURE)
				return false;

			if (type < kSavegameTypeTime || type > kSavegameTypeTickInterval)
				return false;

			if (time < kTimeStartGame || time > kTimeCityConstantinople)
				return false;

			if (offset <= 0 || offset & 15)
				return false;

			/* No check for < 0, as it cannot happen normaly */
			if (chapter == 0)
				return false;

			return true;
		}
	};

	SavegameStream *_savegame;
	Common::Array<SavegameEntryHeader *> _gameHeaders;
	uint32 _gameTicksLastSavegame;

	// Headers
	static bool loadMainHeader(Common::InSaveFile *stream, SavegameMainHeader *header);

	// Entries
	void writeEntry(SavegameType type, EntityIndex entity, uint32 val);
	void readEntry(SavegameType *type, EntityIndex *entity, uint32 *val, bool keepIndex);

	uint32 writeValue(Common::Serializer &ser, const char *name, Common::Functor1<Common::Serializer &, void> *function, uint size);
	uint32 readValue(Common::Serializer &ser, const char *name, Common::Functor1<Common::Serializer &, void> *function, uint size = 0);

	SavegameEntryHeader *getEntry(uint32 index);

	// Opening save files
	static Common::String getFilename(GameId id);
	static Common::InSaveFile  *openForLoading(GameId id);
	static Common::OutSaveFile *openForSaving(GameId id);

	// Savegame stream
	void initStream();
	void loadStream(GameId id);
	void flushStream(GameId id);

	// Misc
	EntityIndex _entity;
	void syncEntity(Common::Serializer &ser);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SAVELOAD_H
