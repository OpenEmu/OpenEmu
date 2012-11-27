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

#include "toon/resource.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "toon/toon.h"

namespace Toon {

Resources::Resources(ToonEngine *vm) : _vm(vm), _cacheSize(0) {
	_resourceCache.clear();
}

Resources::~Resources() {

	while (!_resourceCache.empty()) {
		CacheEntry *temp = _resourceCache.back();
		_resourceCache.pop_back();
		delete temp;
	}

	while (!_pakFiles.empty()) {
		PakFile *temp = _pakFiles.back();
		_pakFiles.pop_back();
		delete temp;
	}

	purgeFileData();
}

void Resources::removePackageFromCache(const Common::String &packName) {
	// I'm not sure what's a good strategy here. It seems unnecessary to
	// actually remove the cached resources, because the player may be
	// wandering back and forth between rooms. So for now, do nothing.
}

bool Resources::getFromCache(const Common::String &fileName, uint32 *fileSize, uint8 **fileData) {
	for (Common::Array<CacheEntry *>::iterator entry = _resourceCache.begin(); entry != _resourceCache.end(); ++entry) {
		if ((*entry)->_data && (*entry)->_fileName.compareToIgnoreCase(fileName) == 0) {
			debugC(5, kDebugResource, "getFromCache(%s) - Got %d bytes from %s", fileName.c_str(), (*entry)->_size, (*entry)->_packName.c_str());
			(*entry)->_age = 0;
			*fileSize = (*entry)->_size;
			*fileData = (*entry)->_data;
			return true;
			}
		}
	return false;
}

void Resources::addToCache(const Common::String &packName, const Common::String &fileName, uint32 fileSize, uint8 *fileData) {
	debugC(5, kDebugResource, "addToCache(%s, %s, %d) - Total Size: %d", packName.c_str(), fileName.c_str(), fileSize, _cacheSize + fileSize);
	for (Common::Array<CacheEntry *>::iterator entry = _resourceCache.begin(); entry != _resourceCache.end(); ++entry) {
		if ((*entry)->_data) {
			(*entry)->_age++;
		}
	}
	_cacheSize += fileSize;

	while (_cacheSize > MAX_CACHE_SIZE) {
		CacheEntry *bestEntry = 0;
		for (Common::Array<CacheEntry *>::iterator entry = _resourceCache.begin(); entry != _resourceCache.end(); ++entry) {
			if ((*entry)->_data) {
				if (!bestEntry || ((*entry)->_age >= bestEntry->_age && (*entry)->_size >= bestEntry->_size)) {
					bestEntry = *entry;
				}
			}
		}
		if (!bestEntry)
			break;

		free(bestEntry->_data);
		bestEntry->_data = 0;
		_cacheSize -= bestEntry->_size;
		debugC(5, kDebugResource, "Freed %s (%s) to reclaim %d bytes", bestEntry->_fileName.c_str(), bestEntry->_packName.c_str(), bestEntry->_size);
	}

	for (Common::Array<CacheEntry *>::iterator entry = _resourceCache.begin(); entry != _resourceCache.end(); ++entry) {
		if (!(*entry)->_data) {
			(*entry)->_packName = packName;
			(*entry)->_fileName = fileName;
			(*entry)->_age = 0;
			(*entry)->_size = fileSize;
			(*entry)->_data = fileData;
			return;
		}
	}

	CacheEntry *entry = new CacheEntry();
	entry->_packName = packName;
	entry->_fileName = fileName;
	entry->_size = fileSize;
	entry->_data = fileData;
	_resourceCache.push_back(entry);
}

void Resources::openPackage(const Common::String &fileName) {
	debugC(1, kDebugResource, "openPackage(%s)", fileName.c_str());

	Common::File file;
	bool opened = file.open(fileName);

	if (!opened)
		return;

	PakFile *pakFile = new PakFile();
	pakFile->open(&file, fileName);

	file.close();

	_pakFiles.push_back(pakFile);
}

void Resources::closePackage(const Common::String &fileName) {

	removePackageFromCache(fileName);
	for (uint32 i = 0; i < _pakFiles.size(); i++) {
		if (_pakFiles[i]->getPackName() == fileName) {
			delete _pakFiles[i];
			_pakFiles.remove_at(i);
			return;
		}
	}
}

uint8 *Resources::getFileData(const Common::String &fileName, uint32 *fileSize) {
	debugC(4, kDebugResource, "getFileData(%s, fileSize)", fileName.c_str());

	// first try to find files outside of .pak
	// some patched files have not been included in package.
	if (Common::File::exists(fileName)) {
		Common::File file;
		bool opened = file.open(fileName);
		if (!opened)
			return 0;

		*fileSize = file.size();
		uint8 *memory = (uint8 *)new uint8[*fileSize];
		file.read(memory, *fileSize);
		file.close();
		_allocatedFileData.push_back(memory);
		return memory;
	} else {

		uint32 locFileSize = 0;
		uint8 *locFileData = 0;

		if (getFromCache(fileName, &locFileSize, &locFileData)) {
			*fileSize = locFileSize;
			return locFileData;
		}

		for (uint32 i = 0; i < _pakFiles.size(); i++) {

			locFileData = _pakFiles[i]->getFileData(fileName, &locFileSize);
			if (locFileData) {
				*fileSize = locFileSize;
				addToCache(_pakFiles[i]->getPackName(), fileName, locFileSize, locFileData);
				return locFileData;
			}
		}
		return 0;
	}
}

Common::SeekableReadStream *Resources::openFile(const Common::String &fileName) {
	debugC(1, kDebugResource, "openFile(%s)", fileName.c_str());

	// first try to find files outside of .pak
	// some patched files have not been included in package.
	if (Common::File::exists(fileName)) {
		Common::File *file = new Common::File();
		bool opened = file->open(fileName);
		if (!opened) {
			delete file;
			return 0;
		}
		return file;
	} else {
		for (uint32 i = 0; i < _pakFiles.size(); i++) {
			Common::SeekableReadStream *stream = 0;
			stream = _pakFiles[i]->createReadStream(fileName);
			if (stream)
				return stream;
		}

		return 0;
	}
}

void Resources::purgeFileData() {
	for (uint32 i = 0; i < _allocatedFileData.size(); i++) {
		delete[] _allocatedFileData[i];
	}
	_allocatedFileData.clear();
}

Common::SeekableReadStream *PakFile::createReadStream(const Common::String &fileName) {
	debugC(1, kDebugResource, "createReadStream(%s)", fileName.c_str());

	uint32 fileSize = 0;
	uint8 *buffer = getFileData(fileName, &fileSize);
	if (buffer)
		return new Common::MemoryReadStream(buffer, fileSize, DisposeAfterUse::YES);
	else
		return 0;
}

uint8 *PakFile::getFileData(const Common::String &fileName, uint32 *fileSize) {
	debugC(4, kDebugResource, "getFileData(%s, fileSize)", fileName.c_str());

	for (uint32 i = 0; i < _numFiles; i++) {
		if (fileName.compareToIgnoreCase(_files[i]._name) == 0) {
			Common::File file;
			if (file.open(_packName)) {
					*fileSize = _files[i]._size;
					file.seek(_files[i]._offset);

					// Use malloc() because that's what MemoryReadStream
					// uses to dispose of the memory when it's done.
					uint8 *buffer = (uint8 *)malloc(*fileSize);
					file.read(buffer, *fileSize);
					file.close();
					return buffer;
			}
		}
	}

	return 0;
}

void PakFile::open(Common::SeekableReadStream *rs, const Common::String &packName) {
	debugC(1, kDebugResource, "open(rs)");

	char buffer[64];
	int32 currentPos = 0;
	_numFiles = 0;
	_packName = packName;

	while (1) {
		rs->seek(currentPos);
		rs->read(buffer, 64);

		int32 offset = READ_LE_UINT32(buffer);
		char *name = buffer + 4;

		if (!*name)
			break;

		int32 nameSize = strlen(name) + 1;
		int32 nextOffset = READ_LE_UINT32(buffer + 4 + nameSize);
		currentPos += 4 + nameSize;

		PakFile::File newFile;
		strcpy(newFile._name, name);
		newFile._offset = offset;
		newFile._size = nextOffset - offset;
		_numFiles++;
		_files.push_back(newFile);
	}
}

void PakFile::close() {
}

PakFile::PakFile() {
}

PakFile::~PakFile() {
	close();
}

} // End of namespace Toon
