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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/base/base_save_thumb_helper.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/math/vector2.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/wintermute.h"
#include "graphics/decoders/bmp.h"
#include "common/memstream.h"
#include "common/str.h"
#include "common/system.h"
#include "common/savefile.h"

namespace Wintermute {

#define SAVE_BUFFER_INIT_SIZE 100000
#define SAVE_BUFFER_GROW_BY   50000

#define SAVE_MAGIC      0x45564153
#define SAVE_MAGIC_2    0x32564153

//////////////////////////////////////////////////////////////////////////
BasePersistenceManager::BasePersistenceManager(const char *savePrefix, bool deleteSingleton) {
	_saving = false;
//	_buffer = NULL;
//	_bufferSize = 0;
	_offset = 0;
	_saveStream = NULL;
	_loadStream = NULL;
	_deleteSingleton = deleteSingleton;
	if (BaseEngine::instance().getGameRef()) {
		_gameRef = BaseEngine::instance().getGameRef();
	} else {
		_gameRef = NULL;
	}

	_richBuffer = NULL;
	_richBufferSize = 0;

	_savedDescription = NULL;
//	_savedTimestamp = 0;
	_savedVerMajor = _savedVerMinor = _savedVerBuild = 0;
	_savedExtMajor = _savedExtMinor = 0;

	_thumbnailDataSize = 0;
	_thumbnailData = NULL;
	if (savePrefix) {
		_savePrefix = savePrefix;
	} else if (_gameRef) {
		_savePrefix = _gameRef->getGameId();
	} else {
		_savePrefix = "wmesav";
	}
}


//////////////////////////////////////////////////////////////////////////
BasePersistenceManager::~BasePersistenceManager() {
	cleanup();
	if (_deleteSingleton && BaseEngine::instance().getGameRef() == NULL)
		BaseEngine::destroy();
}


//////////////////////////////////////////////////////////////////////////
void BasePersistenceManager::cleanup() {
	/*  if (_buffer) {
	        if (_saving) free(_buffer);
	        else delete[] _buffer; // allocated by file manager
	    }
	    _buffer = NULL;

	    _bufferSize = 0;*/
	_offset = 0;

	delete[] _richBuffer;
	_richBuffer = NULL;
	_richBufferSize = 0;

	delete[] _savedDescription;
	_savedDescription = NULL; // ref to buffer
//	_savedTimestamp = 0;
	_savedVerMajor = _savedVerMinor = _savedVerBuild = 0;
	_savedExtMajor = _savedExtMinor = 0;

	_thumbnailDataSize = 0;
	if (_thumbnailData) {
		delete[] _thumbnailData;
		_thumbnailData = NULL;
	}

	delete _loadStream;
	delete _saveStream;
	_loadStream = NULL;
	_saveStream = NULL;
}

Common::String BasePersistenceManager::getFilenameForSlot(int slot) const {
	// 3 Digits, to allow for one save-slot for autosave + slot 1 - 100 (which will be numbered 0-99 filename-wise)
	return Common::String::format("%s-save%03d.wsv", _savePrefix.c_str(), slot);
}

void BasePersistenceManager::getSaveStateDesc(int slot, SaveStateDescriptor &desc) {
	Common::String filename = getFilenameForSlot(slot);
	debugC(kWintermuteDebugSaveGame, "Trying to list savegame %s in slot %d", filename.c_str(), slot);
	if (DID_FAIL(readHeader(filename))) {
		warning("getSavedDesc(%d) - Failed for %s", slot, filename.c_str());
		return;
	}
	desc.setSaveSlot(slot);
	desc.setDescription(_savedDescription);
	desc.setDeletableFlag(true);
	desc.setWriteProtectedFlag(false);

	if (_thumbnailDataSize > 0) {
		Common::MemoryReadStream thumbStream(_thumbnailData, _thumbnailDataSize);
		Graphics::BitmapDecoder bmpDecoder;
		if (bmpDecoder.loadStream(thumbStream)) {
			Graphics::Surface *surf = new Graphics::Surface;
			surf = bmpDecoder.getSurface()->convertTo(g_system->getOverlayFormat());
			desc.setThumbnail(surf);
		}
	}

	desc.setSaveDate(_savedTimestamp.tm_year, _savedTimestamp.tm_mon, _savedTimestamp.tm_mday);
	desc.setSaveTime(_savedTimestamp.tm_hour, _savedTimestamp.tm_min);
	desc.setPlayTime(0);
}

void BasePersistenceManager::deleteSaveSlot(int slot) {
	Common::String filename = getFilenameForSlot(slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

uint32 BasePersistenceManager::getMaxUsedSlot() {
	Common::String saveMask = Common::String::format("%s-save???.wsv", _savePrefix.c_str());
	Common::StringArray saves = g_system->getSavefileManager()->listSavefiles(saveMask);
	Common::StringArray::iterator it = saves.begin();
	int ret = -1;
	for (; it != saves.end(); ++it) {
		int num = -1;
		sscanf(it->c_str(), "save%d", &num);
		ret = MAX(ret, num);
	}
	return ret;
}

bool BasePersistenceManager::getSaveExists(int slot) {
	Common::String filename = getFilenameForSlot(slot);
	if (DID_FAIL(readHeader(filename))) {
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::initSave(const char *desc) {
	if (!desc) {
		return STATUS_FAILED;
	}

	cleanup();
	_saving = true;

	_saveStream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);

	if (_saveStream) {
		// get thumbnails
		if (!_gameRef->_cachedThumbnail) {
			_gameRef->_cachedThumbnail = new BaseSaveThumbHelper(_gameRef);
			if (DID_FAIL(_gameRef->_cachedThumbnail->storeThumbnail(true))) {
				delete _gameRef->_cachedThumbnail;
				_gameRef->_cachedThumbnail = NULL;
			}
		}

		uint32 magic = DCGF_MAGIC;
		putDWORD(magic);

		magic = SAVE_MAGIC_2;
		putDWORD(magic);

		byte verMajor, verMinor, extMajor, extMinor;
		_gameRef->getVersion(&verMajor, &verMinor, &extMajor, &extMinor);
		//uint32 version = MAKELONG(MAKEWORD(VerMajor, VerMinor), MAKEWORD(ExtMajor, ExtMinor));
		_saveStream->writeByte(verMajor);
		_saveStream->writeByte(verMinor);
		_saveStream->writeByte(extMajor);
		_saveStream->writeByte(extMinor);

		// new in ver 2
		putDWORD((uint32)DCGF_VER_BUILD);
		putString(_gameRef->getName());

		// thumbnail data size
		bool thumbnailOK = false;

		if (_gameRef->_cachedThumbnail) {
			if (_gameRef->_cachedThumbnail->_thumbnail) {
				Common::MemoryWriteStreamDynamic thumbStream(DisposeAfterUse::YES);
				if (_gameRef->_cachedThumbnail->_thumbnail->writeBMPToStream(&thumbStream)) {
					_saveStream->writeUint32LE(thumbStream.size());
					_saveStream->write(thumbStream.getData(), thumbStream.size());
				} else {
					_saveStream->writeUint32LE(0);
				}

				thumbnailOK = true;
			}
		}
		if (!thumbnailOK) {
			putDWORD(0);
		}

		// in any case, destroy the cached thumbnail once used
		delete _gameRef->_cachedThumbnail;
		_gameRef->_cachedThumbnail = NULL;

		uint32 dataOffset = _offset +
		                    sizeof(uint32) + // data offset
		                    sizeof(uint32) + strlen(desc) + 1 + // description
		                    sizeof(uint32); // timestamp

		putDWORD(dataOffset);
		putString(desc);

		g_system->getTimeAndDate(_savedTimestamp);
		putTimeDate(_savedTimestamp);
		_savedPlayTime = g_system->getMillis();
		_saveStream->writeUint32LE(_savedPlayTime);
	}
	return STATUS_OK;
}

bool BasePersistenceManager::readHeader(const Common::String &filename) {
	cleanup();

	_saving = false;

	_loadStream = g_system->getSavefileManager()->openForLoading(filename);
	//_buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename, &_bufferSize);
	if (_loadStream) {
		uint32 magic;
		magic = getDWORD();

		if (magic != DCGF_MAGIC) {
			cleanup();
			return STATUS_FAILED;
		}

		magic = getDWORD();

		if (magic == SAVE_MAGIC || magic == SAVE_MAGIC_2) {
			_savedVerMajor = _loadStream->readByte();
			_savedVerMinor = _loadStream->readByte();
			_savedExtMajor = _loadStream->readByte();
			_savedExtMinor = _loadStream->readByte();

			if (magic == SAVE_MAGIC_2) {
				_savedVerBuild = (byte)getDWORD();
				_savedName = getStringObj();

				// load thumbnail
				_thumbnailDataSize = getDWORD();
				if (_thumbnailDataSize > 0) {
					_thumbnailData = new byte[_thumbnailDataSize];
					if (_thumbnailData) {
						getBytes(_thumbnailData, _thumbnailDataSize);
					} else {
						_thumbnailDataSize = 0;
					}
				}
			} else {
				_savedVerBuild = 35;    // last build with ver1 savegames
			}

			uint32 dataOffset = getDWORD();

			_savedDescription = getString();
			_savedTimestamp = getTimeDate();
			_savedPlayTime = _loadStream->readUint32LE();

			_offset = dataOffset;

			return STATUS_OK;
		}
	}

	cleanup();
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::initLoad(const Common::String &filename) {
	if (DID_FAIL(readHeader(filename))) {
		cleanup();
		return STATUS_FAILED;
	}
	_saving = false;

	if (_savedName == "" || scumm_stricmp(_savedName.c_str(), _gameRef->getName()) != 0) {
		debugC(kWintermuteDebugSaveGame, "ERROR: Saved game name doesn't match current game");
		cleanup();
		return STATUS_FAILED;
	}

	// if save is newer version than we are, fail
	if (_savedVerMajor >  DCGF_VER_MAJOR ||
	        (_savedVerMajor == DCGF_VER_MAJOR && _savedVerMinor >  DCGF_VER_MINOR) ||
	        (_savedVerMajor == DCGF_VER_MAJOR && _savedVerMinor == DCGF_VER_MINOR && _savedVerBuild > DCGF_VER_BUILD)
	   ) {

		debugC(kWintermuteDebugSaveGame, "ERROR: Saved game version is newer than current game");
		debugC(kWintermuteDebugSaveGame, "ERROR: Expected %d.%d.%d got %d.%d.%d", DCGF_VER_MAJOR, DCGF_VER_MINOR, DCGF_VER_BUILD, _savedVerMajor, _savedVerMinor, _savedVerBuild);
		cleanup();
		return STATUS_FAILED;
	}

	// if save is older than the minimal version we support
	if (_savedVerMajor <  SAVEGAME_VER_MAJOR ||
	        (_savedVerMajor == SAVEGAME_VER_MAJOR && _savedVerMinor <  SAVEGAME_VER_MINOR) ||
	        (_savedVerMajor == SAVEGAME_VER_MAJOR && _savedVerMinor == SAVEGAME_VER_MINOR && _savedVerBuild < SAVEGAME_VER_BUILD)
	   ) {
		debugC(kWintermuteDebugSaveGame, "ERROR: Saved game is too old and cannot be used by this version of game engine");
		debugC(kWintermuteDebugSaveGame, "ERROR: Expected %d.%d.%d got %d.%d.%d", DCGF_VER_MAJOR, DCGF_VER_MINOR, DCGF_VER_BUILD, _savedVerMajor, _savedVerMinor, _savedVerBuild);
		cleanup();
		return STATUS_FAILED;

	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::saveFile(const Common::String &filename) {
	byte *prefixBuffer = _richBuffer;
	uint32 prefixSize = _richBufferSize;
	byte *buffer = ((Common::MemoryWriteStreamDynamic *)_saveStream)->getData();
	uint32 bufferSize = ((Common::MemoryWriteStreamDynamic *)_saveStream)->size();

	Common::SaveFileManager *saveMan = ((WintermuteEngine *)g_engine)->getSaveFileMan();
	Common::OutSaveFile *file = saveMan->openForSaving(filename);
	file->write(prefixBuffer, prefixSize);
	file->write(buffer, bufferSize);
	bool retVal = !file->err();
	file->finalize();
	delete file;
	return retVal;
}


//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::putBytes(byte *buffer, uint32 size) {
	_saveStream->write(buffer, size);
	if (_saveStream->err()) {
		return STATUS_FAILED;
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::getBytes(byte *buffer, uint32 size) {
	_loadStream->read(buffer, size);
	if (_loadStream->err()) {
		return STATUS_FAILED;
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BasePersistenceManager::putDWORD(uint32 val) {
	_saveStream->writeUint32LE(val);
}


//////////////////////////////////////////////////////////////////////////
uint32 BasePersistenceManager::getDWORD() {
	uint32 ret = _loadStream->readUint32LE();
	return ret;
}


//////////////////////////////////////////////////////////////////////////
void BasePersistenceManager::putString(const Common::String &val) {
	if (!val.size()) {
		putString("(null)");
	} else {
		_saveStream->writeUint32LE(val.size());
		_saveStream->writeString(val);
	}
}

Common::String BasePersistenceManager::getStringObj() {
	uint32 len = _loadStream->readUint32LE();
	char *ret = new char[len + 1];
	_loadStream->read(ret, len);
	ret[len] = '\0';

	Common::String retString = ret;
	delete[] ret;

	if (retString == "(null)") {
		retString = "";
	}

	return retString;
}

//////////////////////////////////////////////////////////////////////////
char *BasePersistenceManager::getString() {
	uint32 len = _loadStream->readUint32LE();
	char *ret = new char[len + 1];
	_loadStream->read(ret, len);
	ret[len] = '\0';

	if (!strcmp(ret, "(null)")) {
		delete[] ret;
		return NULL;
	} else {
		return ret;
	}
}

bool BasePersistenceManager::putTimeDate(const TimeDate &t) {
	_saveStream->writeSint32LE(t.tm_sec);
	_saveStream->writeSint32LE(t.tm_min);
	_saveStream->writeSint32LE(t.tm_hour);
	_saveStream->writeSint32LE(t.tm_mday);
	_saveStream->writeSint32LE(t.tm_mon);
	_saveStream->writeSint32LE(t.tm_year);
	// _saveStream->writeSint32LE(t.tm_wday); //TODO: Add this in when merging next

	if (_saveStream->err()) {
		return STATUS_FAILED;
	}
	return STATUS_OK;
}

TimeDate BasePersistenceManager::getTimeDate() {
	TimeDate t;
	t.tm_sec = _loadStream->readSint32LE();
	t.tm_min = _loadStream->readSint32LE();
	t.tm_hour = _loadStream->readSint32LE();
	t.tm_mday = _loadStream->readSint32LE();
	t.tm_mon = _loadStream->readSint32LE();
	t.tm_year = _loadStream->readSint32LE();
	// t.tm_wday = _loadStream->readSint32LE(); //TODO: Add this in when merging next
	return t;
}

void BasePersistenceManager::putFloat(float val) {
	Common::String str = Common::String::format("F%f", val);
	_saveStream->writeUint32LE(str.size());
	_saveStream->writeString(str);
}

float BasePersistenceManager::getFloat() {
	char *str = getString();
	float value = 0.0f;
	int ret = sscanf(str, "F%f", &value);
	if (ret != 1) {
		warning("%s not parsed as float", str);
	}
	delete[] str;
	return value;
}

void BasePersistenceManager::putDouble(double val) {
	Common::String str = Common::String::format("D%f", val);
	str.format("D%f", val);
	_saveStream->writeUint32LE(str.size());
	_saveStream->writeString(str);
}

double BasePersistenceManager::getDouble() {
	char *str = getString();
	float value = 0.0f; // TODO: Do we ever really need to carry a full double-precision number?
	int ret = sscanf(str, "D%f", &value);
	if (ret != 1) {
		warning("%s not parsed as double", str);
	}
	delete[] str;
	return value;
}

//////////////////////////////////////////////////////////////////////////
// bool
bool BasePersistenceManager::transfer(const char *name, bool *val) {
	if (_saving) {
		_saveStream->writeByte(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = _loadStream->readByte();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// int
bool BasePersistenceManager::transfer(const char *name, int *val) {
	if (_saving) {
		_saveStream->writeSint32LE(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = _loadStream->readSint32LE();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// DWORD
bool BasePersistenceManager::transfer(const char *name, uint32 *val) {
	if (_saving) {
		_saveStream->writeUint32LE(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = _loadStream->readUint32LE();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// float
bool BasePersistenceManager::transfer(const char *name, float *val) {
	if (_saving) {
		putFloat(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = getFloat();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// double
bool BasePersistenceManager::transfer(const char *name, double *val) {
	if (_saving) {
		putDouble(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = getDouble();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// char*
bool BasePersistenceManager::transfer(const char *name, char **val) {
	if (_saving) {
		putString(*val);
		return STATUS_OK;
	} else {
		char *str = getString();
		if (_loadStream->err()) {
			delete[] str;
			return STATUS_FAILED;
		}
		*val = str;
		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// const char*
bool BasePersistenceManager::transfer(const char *name, const char **val) {
	if (_saving) {
		putString(*val);
		return STATUS_OK;
	} else {
		char *str = getString();
		if (_loadStream->err()) {
			delete[] str;
			return STATUS_FAILED;
		}
		*val = str;
		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// Common::String
bool BasePersistenceManager::transfer(const char *name, Common::String *val) {
	if (_saving) {
		putString(*val);
		return STATUS_OK;
	} else {
		char *str = getString();
		if (_loadStream->err()) {
			delete[] str;
			return STATUS_FAILED;
		}
		if (str) {
			*val = str;
			delete[] str;
		} else {
			*val = "";
		}
		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::transfer(const char *name, AnsiStringArray &val) {
	size_t size;

	if (_saving) {
		size = val.size();
		_saveStream->writeUint32LE(size);

		for (AnsiStringArray::iterator it = val.begin(); it != val.end(); ++it) {
			putString((*it).c_str());
		}
	} else {
		val.clear();
		size = _loadStream->readUint32LE();

		for (size_t i = 0; i < size; i++) {
			char *str = getString();
			if (_loadStream->err()) {
				delete[] str;
				return STATUS_FAILED;
			}
			if (str) {
				val.push_back(str);
			}
			delete[] str;
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
// BYTE
bool BasePersistenceManager::transfer(const char *name, byte *val) {
	if (_saving) {
		_saveStream->writeByte(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = _loadStream->readByte();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// RECT
bool BasePersistenceManager::transfer(const char *name, Rect32 *val) {
	if (_saving) {
		_saveStream->writeSint32LE(val->left);
		_saveStream->writeSint32LE(val->top);
		_saveStream->writeSint32LE(val->right);
		_saveStream->writeSint32LE(val->bottom);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		val->left = _loadStream->readSint32LE();
		val->top = _loadStream->readSint32LE();
		val->right = _loadStream->readSint32LE();
		val->bottom = _loadStream->readSint32LE();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// POINT
bool BasePersistenceManager::transfer(const char *name, Point32 *val) {
	if (_saving) {
		_saveStream->writeSint32LE(val->x);
		_saveStream->writeSint32LE(val->y);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		val->x = _loadStream->readSint32LE();
		val->y = _loadStream->readSint32LE();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// Vector2
bool BasePersistenceManager::transfer(const char *name, Vector2 *val) {
	if (_saving) {
		putFloat(val->x);
		putFloat(val->y);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		val->x = getFloat();
		val->y = getFloat();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// generic pointer
bool BasePersistenceManager::transfer(const char *name, void *val) {
	int classID = -1, instanceID = -1;

	if (_saving) {
		SystemClassRegistry::getInstance()->getPointerID(*(void **)val, &classID, &instanceID);
		if (*(void **)val != NULL && (classID == -1 || instanceID == -1)) {
			debugC(kWintermuteDebugSaveGame, "Warning: invalid instance '%s'", name);
		}

		_saveStream->writeUint32LE(classID);
		_saveStream->writeUint32LE(instanceID);
	} else {
		classID = _loadStream->readUint32LE();
		instanceID = _loadStream->readUint32LE();

		*(void **)val = SystemClassRegistry::getInstance()->idToPointer(classID, instanceID);
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::checkVersion(byte verMajor, byte verMinor, byte verBuild) {
	if (_saving) {
		return true;
	}

	// it's ok if we are same or newer than the saved game
	if (verMajor >  _savedVerMajor ||
	        (verMajor == _savedVerMajor && verMinor >  _savedVerMinor) ||
	        (verMajor == _savedVerMajor && verMinor == _savedVerMinor && verBuild > _savedVerBuild)
	   ) {
		return false;
	}

	return true;
}

} // end of namespace Wintermute
