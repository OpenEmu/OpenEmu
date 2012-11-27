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

#include "lastexpress/game/savegame.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/menu/menu.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/debug.h"
#include "lastexpress/lastexpress.h"

#include "common/file.h"

namespace LastExpress {

#define DISABLE_COMPRESSION 1

// Names of savegames
static const struct {
	const char *saveFile;
} gameInfo[6] = {
	{"lastexpress-blue.egg"},
	{"lastexpress-red.egg"},
	{"lastexpress-green.egg"},
	{"lastexpress-purple.egg"},
	{"lastexpress-teal.egg"},
	{"lastexpress-gold.egg"}
};

//////////////////////////////////////////////////////////////////////////
// SavegameStream
//////////////////////////////////////////////////////////////////////////

uint32 SavegameStream::write(const void *dataPtr, uint32 dataSize) {
#if !DISABLE_COMPRESSION
	if (_enableCompression)
		return writeCompressed(dataPtr, dataSize);
#endif

	return Common::MemoryWriteStreamDynamic::write(dataPtr, dataSize);
}

uint32 SavegameStream::read(void *dataPtr, uint32 dataSize) {
#if !DISABLE_COMPRESSION
	if (_enableCompression)
		return readCompressed(dataPtr, dataSize);
#endif

	return readUncompressed(dataPtr, dataSize);
}

uint32 SavegameStream::readUncompressed(void *dataPtr, uint32 dataSize) {
	if ((int32)dataSize > size() - pos()) {
		dataSize = (uint32)(size() - pos());
		_eos = true;
	}
	memcpy(dataPtr, getData() + pos(), dataSize);

	seek(dataSize, SEEK_CUR);

	return dataSize;
}

void SavegameStream::writeBuffer(uint8 value, bool onlyValue) {
	if (_bufferOffset == -1)
		_bufferOffset = 0;

	if (_bufferOffset == 256) {
		_bufferOffset = 0;
		Common::MemoryWriteStreamDynamic::write(_buffer, 256);
	}

	if (onlyValue || value < 0xFB)
		_buffer[_bufferOffset] = value;
	else
		_buffer[_bufferOffset] = 0xFE;

	_offset++;
	_bufferOffset++;

	if (!onlyValue && value >= 0xFB)
	{
		if (_bufferOffset == 256) {
			_bufferOffset = 0;
			Common::MemoryWriteStreamDynamic::write(_buffer, 256);
		}

		_buffer[_bufferOffset] = value;

		_bufferOffset++;
		_offset++;
	}
}

uint8 SavegameStream::readBuffer() {
	if (_bufferOffset == -1 || _bufferOffset >= 256) {
		readUncompressed(_buffer, 256);
		_bufferOffset = 0;
	}

	byte val = _buffer[_bufferOffset];
	_bufferOffset++;

	return val;
}

uint32 SavegameStream::process() {
	_enableCompression = !_enableCompression;

#if DISABLE_COMPRESSION
	return 0;
#else
	switch (_status) {
	default:
		break;

	case kStatusReading:
		_status = kStatusReady;
		if (_bufferOffset != -1 && _bufferOffset != 256) {
			seek(_bufferOffset - 256, SEEK_CUR);
			_bufferOffset = -1;
		}
		break;

	case kStatusWriting:
		switch (_valueCount) {
		default:
			break;

		case 1:
			writeBuffer(_previousValue, false);
			break;

		case 2:
			if (_previousValue) {
				writeBuffer(0xFF);
				writeBuffer(_repeatCount);
				writeBuffer(_previousValue);
				break;
			}

			if (_repeatCount == 3) {
				writeBuffer(0xFB);
				break;
			}

			if (_repeatCount == 255) {
				writeBuffer(0xFC);
				break;
			}

			writeBuffer(0xFD);
			writeBuffer(_repeatCount);
			break;
		}

		if (_bufferOffset != -1 && _bufferOffset != 0) {
			Common::MemoryWriteStreamDynamic::write(_buffer, _bufferOffset);
			_bufferOffset = -1;
		}
		break;
	}

	_status = kStatusReady;
	_valueCount = 0;
	uint32 offset = _offset;
	_offset = 0;

	return offset;
#endif
}

uint32 SavegameStream::writeCompressed(const void *dataPtr, uint32 dataSize) {
	if (_status == kStatusReading)
		error("[SavegameStream::writeCompressed] Error: Compression buffer is in read mode.");

	_status = kStatusWriting;
	const byte *data = (const byte *)dataPtr;

	while (dataSize) {
		switch (_valueCount) {
		default:
			error("[SavegameStream::writeCompressed] Invalid value count (%d)", _valueCount);

		case 0:
			_previousValue = *data++;
			_valueCount = 1;
			break;

		case 1:
			if (*data != _previousValue) {
				writeBuffer(_previousValue, false);
				_previousValue = *data;
			} else {
				_valueCount = 2;
				_repeatCount = 2;
			}

			++data;
			break;

		case 2:
			if (*data != _previousValue || _repeatCount >= 255) {
				if (_previousValue) {
					writeBuffer(0xFF, true);
					writeBuffer((uint8)_repeatCount, true);
					writeBuffer(_previousValue, true);

					_previousValue = *data++;
					_valueCount = 1;
					break;
				}

				if (_repeatCount == 3) {
					writeBuffer(0xFB, true);

					_previousValue = *data++;
					_valueCount = 1;
					break;
				}

				if (_repeatCount == -1) {
					writeBuffer(0xFC, true);

					_previousValue = *data++;
					_valueCount = 1;
					break;
				}

				writeBuffer(0xFD, true);
				writeBuffer((uint8)_repeatCount, true);

				_previousValue = *data++;
				_valueCount = 1;
			}

			++data;
			++_repeatCount;
			break;
		}

		--dataSize;
	}

	return _offset;
}

uint32 SavegameStream::readCompressed(void *dataPtr, uint32 dataSize) {
	if (_status == kStatusWriting)
		error("[SavegameStream::writeCompressed] Error: Compression buffer is in write mode.");

	_status = kStatusReady;
	byte *data = (byte *)dataPtr;

	while (dataSize) {
		switch (_valueCount) {
		default:
			error("[SavegameStream::readCompressed] Invalid value count (%d)", _valueCount);

		case 0:
		case 1: {
			// Read control code
			byte control = readBuffer();

			switch (control) {
			default:
				// Data value
				*data++ = control;
				break;

			case 0xFB:
				_repeatCount = 2;
				_previousValue = 0;
				*data++ = 0;
				_valueCount = 2;
				break;

			case 0xFC:
				_repeatCount = 254;
				_previousValue = 0;
				*data++ = 0;
				_valueCount = 2;
				break;

			case 0xFD:
				_repeatCount = readBuffer() - 1;
				_previousValue = 0;
				*data++ = 0;
				_valueCount = 2;
				break;

			case 0xFE:
				*data++ = readBuffer();
				break;

			case 0xFF:
				_repeatCount = readBuffer() - 1;
				_previousValue = readBuffer();
				*data++ = _previousValue;
				_valueCount = 2;
				break;
			}
			}
			break;

		case 2:
			*data++ = _previousValue;
			_repeatCount--;
			if (!_repeatCount)
				_valueCount = 1;
			break;
		}

		--dataSize;
	}

	return _offset;
}

//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////

SaveLoad::SaveLoad(LastExpressEngine *engine) : _engine(engine), _savegame(NULL), _gameTicksLastSavegame(0), _entity(kEntityPlayer) {
}

SaveLoad::~SaveLoad() {
	clear(true);
	_savegame = NULL;

	// Zero passed pointers
	_engine = NULL;
}

void SaveLoad::initStream() {
	delete _savegame;
	_savegame = new SavegameStream();
}

void SaveLoad::flushStream(GameId id) {
	Common::OutSaveFile *save = openForSaving(id);
	if (!save)
		error("[SaveLoad::flushStream] Cannot open savegame (%s)", getFilename(id).c_str());

	if (!_savegame)
		error("[SaveLoad::flushStream] Savegame stream is invalid");

	save->write(_savegame->getData(), (uint32)_savegame->size());
	save->finalize();

	delete save;
}

//////////////////////////////////////////////////////////////////////////
// Init
//////////////////////////////////////////////////////////////////////////
void SaveLoad::create(GameId id) {
	initStream();

	Common::Serializer ser(NULL, _savegame);
	SavegameMainHeader header;
	header.saveLoadWithSerializer(ser);

	flushStream(id);
}

uint32 SaveLoad::init(GameId id, bool resetHeaders) {
	initStream();

	// Load game data
	loadStream(id);

	// Get the main header
	Common::Serializer ser(_savegame, NULL);
	SavegameMainHeader mainHeader;
	mainHeader.saveLoadWithSerializer(ser);
	if (!mainHeader.isValid())
		error("[SaveLoad::init] Savegame seems to be corrupted (invalid header)");

	// Reset cached entry headers if needed
	if (resetHeaders) {
		clear();

		SavegameEntryHeader *entryHeader = new SavegameEntryHeader();
		entryHeader->time = kTimeCityParis;
		entryHeader->chapter = kChapter1;

		_gameHeaders.push_back(entryHeader);
	}

	// Read the list of entry headers
	if (_savegame->size() > 32) {
		while (_savegame->pos() < _savegame->size() && !_savegame->eos() && !_savegame->err()) {

			// Update sound queue while we go through the savegame
			getSoundQueue()->updateQueue();

			SavegameEntryHeader *entry = new SavegameEntryHeader();
			entry->saveLoadWithSerializer(ser);

			if (!entry->isValid())
				break;

			_gameHeaders.push_back(entry);

			_savegame->seek(entry->offset, SEEK_CUR);
		}
	}

	// return the index to the current save game entry (we store count + 1 entries, so we're good)
	return mainHeader.count;
}

void SaveLoad::loadStream(GameId id) {
	Common::InSaveFile *save = openForLoading(id);
	if (save->size() < 32)
		error("[SaveLoad::loadStream] Savegame seems to be corrupted (not enough data: %i bytes)", save->size());

	if (!_savegame)
		error("[SaveLoad::loadStream] Savegame stream is invalid");

	// Load all savegame data
	uint8 *buf = new uint8[8192];
	while (!save->eos() && !save->err()) {
		_engine->pollEvents();

		uint32 count = save->read(buf, 8192);
		if (count) {
			uint32 w = _savegame->write(buf, count);
			assert (w == count);
		}
	}

	if (save->err())
		error("SaveLoad::init - Error reading savegame");

	delete[] buf;
	delete save;

	// Move back to the beginning of the stream
	_savegame->seek(0);
}

void SaveLoad::clear(bool clearStream) {
	for (uint i = 0; i < _gameHeaders.size(); i++)
		SAFE_DELETE(_gameHeaders[i]);

	_gameHeaders.clear();

	if (clearStream)
		SAFE_DELETE(_savegame);
}

//////////////////////////////////////////////////////////////////////////
// Save & Load
//////////////////////////////////////////////////////////////////////////

// Load last saved game
void SaveLoad::loadLastGame() {
	if (!_savegame)
		error("[SaveLoad::loadLastGame] No savegame stream present");

	// Rewind current savegame
	_savegame->seek(0);

	// Validate main header
	SavegameMainHeader header;
	if (!loadMainHeader(_savegame, &header)) {
		debugC(2, kLastExpressDebugSavegame, "Cannot load main header: %s", getFilename(getMenu()->getGameId()).c_str());
		return;
	}

	if (!_savegame)
		error("[SaveLoad::loadGame] No savegame stream present");

	// Load the last entry
	_savegame->seek(header.offsetEntry);

	SavegameType type = kSavegameTypeIndex;
	EntityIndex entity = kEntityPlayer;
	uint32 val = 0;
	readEntry(&type, &entity, &val, header.keepIndex == 1);

	// Setup last loading time
	_gameTicksLastSavegame = getState()->timeTicks;

	if (header.keepIndex) {
		getSoundQueue()->clearQueue();

		readEntry(&type, &entity, &val, false);
	}

	getEntities()->reset();
	getEntities()->setup(false, entity);
}

// Load a specific game entry
void SaveLoad::loadGame(uint32 index) {
	if (!_savegame)
		error("[SaveLoad::loadLastGame] No savegame stream present");

	// Rewind current savegame
	_savegame->seek(0);

	// Write main header (with selected index)
	SavegameMainHeader header;
	header.count = index;
	header.brightness = getState()->brightness;
	header.volume = getState()->volume;

	Common::Serializer ser(NULL, _savegame);
	header.saveLoadWithSerializer(ser);

	// TODO
	// Go to the entry
	// Load the entry
	// Get offset (main and entry)
	// Write main header again with correct entry offset
	// Setup game and start

	error("[SaveLoad::loadGame] Not implemented! (only loading the last entry is working for now)");
}

// Save game
void SaveLoad::saveGame(SavegameType type, EntityIndex entity, uint32 value) {
	if (getState()->scene <= kSceneIntro)
		return;

	// Validate main header
	SavegameMainHeader header;
	if (!loadMainHeader(_savegame, &header)) {
		debugC(2, kLastExpressDebugSavegame, "Cannot load main header: %s", getFilename(getMenu()->getGameId()).c_str());
		return;
	}

	if (!_savegame)
		error("[SaveLoad::saveGame] Savegame stream is invalid");

	// Validate the current entry if it exists
	if (header.count > 0) {
		_savegame->seek(header.offsetEntry);

		// Load entry header
		SavegameEntryHeader entry;
		Common::Serializer ser(_savegame, NULL);
		entry.saveLoadWithSerializer(ser);

		if (!entry.isValid()) {
			warning("[SaveLoad::saveGame] Invalid entry. This savegame might be corrupted");
			_savegame->seek(header.offset);
		} else if (getState()->time < entry.time || (type == kSavegameTypeTickInterval && getState()->time == entry.time)) {
			// Not ready to save a game, skipping!
			return;
		} else if ((type == kSavegameTypeTime || type == kSavegameTypeEvent)
			&& (entry.type == kSavegameTypeTickInterval && (getState()->time - entry.time) < 450)) {
			_savegame->seek(header.offsetEntry);
			--header.count;
		} else {
			_savegame->seek(header.offset);
		}
	} else {
		// Seek to the next savegame entry
		_savegame->seek(header.offset);
	}

	if (type != kSavegameTypeEvent2 && type != kSavegameTypeAuto)
		header.offsetEntry = (uint32)_savegame->pos();

	// Write the savegame entry
	writeEntry(type, entity, value);

	if (!header.keepIndex)
		++header.count;

	if (type == kSavegameTypeEvent2 || type == kSavegameTypeAuto) {
		header.keepIndex = 1;
	} else {
		header.keepIndex = 0;
		header.offset = (uint32)_savegame->pos();

		// Save ticks
		_gameTicksLastSavegame = getState()->timeTicks;
	}

	// Validate the main header
	if (!header.isValid())
		error("[SaveLoad::saveGame] Main game header is invalid");

	// Write the main header
	_savegame->seek(0);
	Common::Serializer ser(NULL, _savegame);
	header.saveLoadWithSerializer(ser);

	flushStream(getMenu()->getGameId());
}

void SaveLoad::saveVolumeBrightness() {
	warning("[SaveLoad::saveVolumeBrightness] Not implemented");
}

//////////////////////////////////////////////////////////////////////////
// Headers
//////////////////////////////////////////////////////////////////////////
bool SaveLoad::loadMainHeader(Common::InSaveFile *stream, SavegameMainHeader *header) {
	if (!stream)
		return false;

	// Check there is enough data (32 bytes)
	if (stream->size() < 32) {
		debugC(2, kLastExpressDebugSavegame, "Savegame seems to be corrupted (not enough data: %i bytes)", stream->size());
		return false;
	}

	// Rewind stream
	stream->seek(0);

	Common::Serializer ser(stream, NULL);
	header->saveLoadWithSerializer(ser);

	// Validate the header
	if (!header->isValid()) {
		debugC(2, kLastExpressDebugSavegame, "Cannot validate main header");
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Entries
//////////////////////////////////////////////////////////////////////////
uint32 SaveLoad::writeValue(Common::Serializer &ser, const char *name, Common::Functor1<Common::Serializer &, void> *function, uint size) {
	if (!_savegame)
		error("[SaveLoad::writeValue] Stream not initialized properly");

	debugC(kLastExpressDebugSavegame, "Savegame: Writing %s: %u bytes", name, size);

	uint32 prevPosition = (uint32)_savegame->pos();

	// Serialize data into our buffer
	(*function)(ser);

	uint32 count = (uint32)_savegame->pos() - prevPosition;

#if DISABLE_COMPRESSION
	if (count != size)
		error("[SaveLoad::writeValue] %s - Number of bytes written (%d) differ from expected count (%d)", name, count, size);
#endif

	return count;
}

uint32 SaveLoad::readValue(Common::Serializer &ser, const char *name, Common::Functor1<Common::Serializer &, void> *function, uint size) {
	if (!_savegame)
		error("[SaveLoad::readValue] Stream not initialized properly");

	debugC(kLastExpressDebugSavegame, "Savegame: Reading %s: %u bytes", name, size);

	uint32 prevPosition = (uint32)_savegame->pos();

	(*function)(ser);

	uint32 count = (uint32)_savegame->pos() - prevPosition;

#if DISABLE_COMPRESSION
	if (size != 0 && count != size)
		error("[SaveLoad::readValue] %s - Number of bytes read (%d) differ from expected count (%d)", name, count, size);
#endif

	return count;
}

void SaveLoad::syncEntity(Common::Serializer &ser) {
	ser.syncAsUint32LE(_entity);
}

void SaveLoad::writeEntry(SavegameType type, EntityIndex entity, uint32 value) {
	if (!_savegame)
		error("[SaveLoad::writeEntry] Savegame stream is invalid");

	SavegameEntryHeader header;

	header.type = type;
	header.time = (uint32)getState()->time;
	header.chapter = getProgress().chapter;
	header.value = value;

	// Save position
	uint32 originalPosition = (uint32)_savegame->pos();

	// Write header
	Common::Serializer ser(NULL, _savegame);
	header.saveLoadWithSerializer(ser);

	// Write game data
	_entity = entity;

	_savegame->process();
	writeValue(ser, "entity index", WRAP_SYNC_FUNCTION(this, SaveLoad, syncEntity), 4);
	writeValue(ser, "state", WRAP_SYNC_FUNCTION(getState(), State::GameState, saveLoadWithSerializer), 4 + 4 + 4 + 4 + 1 + 4 + 4);
	writeValue(ser, "selected item", WRAP_SYNC_FUNCTION(getInventory(), Inventory, saveSelectedItem), 4);
	writeValue(ser, "positions", WRAP_SYNC_FUNCTION(getEntities(), Entities, savePositions), 4 * 1000);
	writeValue(ser, "compartments", WRAP_SYNC_FUNCTION(getEntities(), Entities, saveCompartments), 4 * 16 * 2);
	writeValue(ser, "progress", WRAP_SYNC_FUNCTION(&getProgress(), State::GameProgress, saveLoadWithSerializer), 4 * 128);
	writeValue(ser, "events", WRAP_SYNC_FUNCTION(getState(), State::GameState, syncEvents), 512);
	writeValue(ser, "inventory", WRAP_SYNC_FUNCTION(getInventory(), Inventory, saveLoadWithSerializer), 7 * 32);
	writeValue(ser, "objects", WRAP_SYNC_FUNCTION(getObjects(), Objects, saveLoadWithSerializer), 5 * 128);
	writeValue(ser, "entities", WRAP_SYNC_FUNCTION(getEntities(), Entities, saveLoadWithSerializer), 1262 * 40);
	writeValue(ser, "sound", WRAP_SYNC_FUNCTION(getSoundQueue(), SoundQueue, saveLoadWithSerializer), 3 * 4 + getSoundQueue()->count() * 64);
	writeValue(ser, "savepoints", WRAP_SYNC_FUNCTION(getSavePoints(), SavePoints, saveLoadWithSerializer), 128 * 16 + 4 + getSavePoints()->count() * 16);
	_savegame->process();

	header.offset = (uint32)_savegame->pos() - (originalPosition + 32);

	// Add padding if necessary
	while (header.offset & 0xF) {
		_savegame->writeByte(0);
		header.offset++;
	}

	// Save end position
	uint32 endPosition = (uint32)_savegame->pos();

	// Validate entry header
	if (!header.isValid())
		error("[SaveLoad::writeEntry] Entry header is invalid");

	// Save the header with the updated info
	_savegame->seek(originalPosition);
	header.saveLoadWithSerializer(ser);

	// Move back to the end of the entry
	_savegame->seek(endPosition);
}

void SaveLoad::readEntry(SavegameType *type, EntityIndex *entity, uint32 *val, bool keepIndex) {
	if (!type || !entity || !val)
		error("[SaveLoad::readEntry] Invalid parameters passed");

	if (!_savegame)
		error("[SaveLoad::readEntry] No savegame stream present");

	// Load entry header
	SavegameEntryHeader entry;
	Common::Serializer ser(_savegame, NULL);
	entry.saveLoadWithSerializer(ser);

	if (!entry.isValid())
		error("[SaveLoad::readEntry] Entry header is invalid");

	// Init type, entity & value
	*type = entry.type;
	*val = entry.value;

	// Save position
	uint32 originalPosition = (uint32)_savegame->pos();

	// Load game data
	_savegame->process();
	readValue(ser, "entity index", WRAP_SYNC_FUNCTION(this, SaveLoad, syncEntity), 4);
	readValue(ser, "state", WRAP_SYNC_FUNCTION(getState(), State::GameState, saveLoadWithSerializer), 4 + 4 + 4 + 4 + 1 + 4 + 4);
	readValue(ser, "selected item", WRAP_SYNC_FUNCTION(getInventory(), Inventory, saveSelectedItem), 4);
	readValue(ser, "positions", WRAP_SYNC_FUNCTION(getEntities(), Entities, savePositions), 4 * 1000);
	readValue(ser, "compartments", WRAP_SYNC_FUNCTION(getEntities(), Entities, saveCompartments), 4 * 16 * 2);
	readValue(ser, "progress", WRAP_SYNC_FUNCTION(&getProgress(), State::GameProgress, saveLoadWithSerializer), 4 * 128);
	readValue(ser, "events", WRAP_SYNC_FUNCTION(getState(), State::GameState, syncEvents), 512);
	readValue(ser, "inventory", WRAP_SYNC_FUNCTION(getInventory(), Inventory, saveLoadWithSerializer), 7 * 32);
	readValue(ser, "objects", WRAP_SYNC_FUNCTION(getObjects(), Objects, saveLoadWithSerializer), 5 * 128);
	readValue(ser, "entities", WRAP_SYNC_FUNCTION(getEntities(), Entities, saveLoadWithSerializer), 1262 * 40);
	readValue(ser, "sound", WRAP_SYNC_FUNCTION(getSoundQueue(), SoundQueue, saveLoadWithSerializer));
	readValue(ser, "savepoints", WRAP_SYNC_FUNCTION(getSavePoints(), SavePoints, saveLoadWithSerializer));
	_savegame->process();

	// Update chapter
	*entity = _entity;
	getProgress().chapter = entry.chapter;

	// Skip padding
	uint32 offset = (uint32)_savegame->pos() - originalPosition;
	if (offset & 0xF) {
		_savegame->seek((~offset & 0xF) + 1, SEEK_SET);
	}
}

SaveLoad::SavegameEntryHeader *SaveLoad::getEntry(uint32 index) {
	if (index >= _gameHeaders.size())
		error("[SaveLoad::getEntry] Invalid index (was:%d, max:%d)", index, _gameHeaders.size() - 1);

	return _gameHeaders[index];
}

//////////////////////////////////////////////////////////////////////////
// Checks
//////////////////////////////////////////////////////////////////////////

// Check if a specific savegame exists
bool SaveLoad::isSavegamePresent(GameId id) {
	if (g_system->getSavefileManager()->listSavefiles(getFilename(id)).size() == 0)
		return false;

	return true;
}

// Check if the game has been started in the specific savegame
bool SaveLoad::isSavegameValid(GameId id) {
	if (!isSavegamePresent(id)) {
		debugC(2, kLastExpressDebugSavegame, "Savegame does not exist: %s", getFilename(id).c_str());
		return false;
	}

	SavegameMainHeader header;

	Common::InSaveFile *save = openForLoading(id);
	bool isHeaderValid = loadMainHeader(save, &header);
	delete save;

	return isHeaderValid;
}

bool SaveLoad::isGameFinished(uint32 menuIndex, uint32 savegameIndex) {
	SavegameEntryHeader *data = getEntry(menuIndex);

	if (savegameIndex != menuIndex)
		return false;

	if (data->type != kSavegameTypeEvent)
		return false;

	return (data->value == kEventAnnaKilled
	     || data->value == kEventKronosHostageAnnaNoFirebird
	     || data->value == kEventKahinaPunchBaggageCarEntrance
	     || data->value == kEventKahinaPunchBlue
	     || data->value == kEventKahinaPunchYellow
	     || data->value == kEventKahinaPunchSalon
	     || data->value == kEventKahinaPunchKitchen
	     || data->value == kEventKahinaPunchBaggageCar
	     || data->value == kEventKahinaPunchCar
	     || data->value == kEventKahinaPunchSuite4
	     || data->value == kEventKahinaPunchRestaurant
	     || data->value == kEventKahinaPunch
	     || data->value == kEventKronosGiveFirebird
	     || data->value == kEventAugustFindCorpse
	     || data->value == kEventMertensBloodJacket
	     || data->value == kEventMertensCorpseFloor
	     || data->value == kEventMertensCorpseBed
	     || data->value == kEventCoudertBloodJacket
	     || data->value == kEventGendarmesArrestation
	     || data->value == kEventAbbotDrinkGiveDetonator
	     || data->value == kEventMilosCorpseFloor
	     || data->value == kEventLocomotiveAnnaStopsTrain
	     || data->value == kEventTrainStopped
	     || data->value == kEventCathVesnaRestaurantKilled
	     || data->value == kEventCathVesnaTrainTopKilled
	     || data->value == kEventLocomotiveConductorsDiscovered
	     || data->value == kEventViennaAugustUnloadGuns
	     || data->value == kEventViennaKronosFirebird
	     || data->value == kEventVergesAnnaDead
	     || data->value == kEventTrainExplosionBridge
	     || data->value == kEventKronosBringNothing);
}


//////////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////////

// Get the file name from the savegame ID
Common::String SaveLoad::getFilename(GameId id) {
	if (id >= 6)
		error("[SaveLoad::getFilename] Attempting to use an invalid game id. Valid values: 0 - 5, was %d", id);

	return gameInfo[id].saveFile;
}

Common::InSaveFile *SaveLoad::openForLoading(GameId id) {
	Common::InSaveFile *load = g_system->getSavefileManager()->openForLoading(getFilename(id));

	if (!load)
		debugC(2, kLastExpressDebugSavegame, "Cannot open savegame for loading: %s", getFilename(id).c_str());

	return load;
}

Common::OutSaveFile *SaveLoad::openForSaving(GameId id) {
	Common::OutSaveFile *save = g_system->getSavefileManager()->openForSaving(getFilename(id), false); // TODO Enable compression again

	if (!save)
		debugC(2, kLastExpressDebugSavegame, "Cannot open savegame for writing: %s", getFilename(id).c_str());

	return save;
}

} // End of namespace LastExpress
