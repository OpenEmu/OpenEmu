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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "pak.h"

bool PAKFile::loadFile(const char *file, const bool isAmiga) {
	_isAmiga = isAmiga;
	if (!file)
		return true;

	delete _fileList;
	_fileList = 0;

	FILE *pakfile = fopen(file, "rb");
	if (!pakfile)
		return false;

	uint32 filesize = fileSize(pakfile);

	// TODO: get rid of temp. buffer
	uint8 *buffer = new uint8[filesize];
	assert(buffer);

	fread(buffer, filesize, 1, pakfile);

	fclose(pakfile);

	const char *currentName = 0;

	uint32 startoffset = _isAmiga ? READ_BE_UINT32(buffer) : READ_LE_UINT32(buffer);
	uint32 endoffset = 0;
	uint8* position = buffer + 4;

	while (true) {
		uint32 strlgt = strlen((const char*)position);
		currentName = (const char*)position;

		if (!(*currentName))
			break;

		position += strlgt + 1;
		endoffset = _isAmiga ? READ_BE_UINT32(position) : READ_LE_UINT32(position);
		if (endoffset > filesize) {
			endoffset = filesize;
		} else if (endoffset == 0) {
			endoffset = filesize;
		}
		position += 4;

		uint8 *data = new uint8[endoffset - startoffset];
		assert(data);
		memcpy(data, buffer + startoffset, endoffset - startoffset);
		addFile(currentName, data, endoffset - startoffset);
		data = 0;

		if (endoffset == filesize)
			break;

		startoffset = endoffset;
	}

	delete[] buffer;
	return true;
}

bool PAKFile::saveFile(const char *file) {
	if (!_fileList)
		return true;

	FILE *f = fopen(file, "wb");
	if (!f) {
		error("couldn't open file '%s' for writing", file);
		return false;
	}

	// TODO: implement error handling
	uint32 startAddr = _fileList->getTableSize()+5+4;
	static const char *zeroName = "\0\0\0\0\0";

	uint32 curAddr = startAddr;
	for (FileList *cur = _fileList; cur; cur = cur->next) {
		if (_isAmiga)
			writeUint32BE(f, curAddr);
		else
			writeUint32LE(f, curAddr);
		fwrite(cur->filename, 1, strlen(cur->filename) + 1, f);
		curAddr += cur->size;
	}
	if (_isAmiga)
		writeUint32BE(f, curAddr);
	else
		writeUint32LE(f, curAddr);
	fwrite(zeroName, 1, 5, f);

	for (FileList *cur = _fileList; cur; cur = cur->next)
		fwrite(cur->data, 1, cur->size, f);

	fclose(f);
	return true;
}

void PAKFile::drawFileList() {
	FileList *cur = _fileList;
	while (cur) {
		printf("Filename: '%s' size: %d\n", cur->filename, cur->size);
		cur = cur->next;
	}
}

bool PAKFile::outputAllFiles() {
	FileList *cur = _fileList;
	while (cur) {
		FILE *file = fopen(cur->filename, "wb");
		if (!file) {
			error("couldn't open file '%s' for writing", cur->filename);
			return false;
		}
		printf("Exracting file '%s'...", cur->filename);
		if (fwrite(cur->data, 1, cur->size, file) == cur->size) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			return false;
		}
		fclose(file);
		cur = cur->next;
	}
	return true;
}

bool PAKFile::outputFileAs(const char *f, const char *fn) {
	FileList *cur = (_fileList != 0) ? _fileList->findEntry(f) : 0;

	if (!cur) {
		error("file '%s' not found", f);
		return false;
	}

	FILE *file = fopen(fn, "wb");
	if (!file) {
		error("couldn't open file '%s' in write mode", fn);
		return false;
	}
	printf("Exracting file '%s' to file '%s'...", cur->filename, fn);
	if (fwrite(cur->data, 1, cur->size, file) == cur->size) {
		printf("OK\n");
	} else {
		printf("FAILED\n");
		return false;
	}
	fclose(file);
	return true;
}

const uint8 *PAKFile::getFileData(const char *file, uint32 *size) {
	FileList *cur = (_fileList != 0) ? _fileList->findEntry(file) : 0;

	if (!cur)
		return 0;

	if (size)
		*size = cur->size;
	return cur->data;
}

bool PAKFile::addFile(const char *name, const char *file) {
	if (_fileList && _fileList->findEntry(name)) {
		error("entry '%s' already exists", name);
		return false;
	}

	FILE *f = fopen(file, "rb");
	if (!f) {
		error("couldn't open file '%s'", file);
		return false;
	}

	uint32 filesize = fileSize(f);
	uint8 *data = new uint8[filesize];
	assert(data);
	if (fread(data, 1, filesize, f) != filesize) {
		error("couldn't read from file '%s'", file);
		return false;
	}
	fclose(f);
	return addFile(name, data, filesize);
}

bool PAKFile::addFile(const char *name, uint8 *data, uint32 size) {
	if (_fileList && _fileList->findEntry(name)) {
		error("entry '%s' already exists", name);
		return false;
	}

	FileList *newEntry = new FileList;
	assert(newEntry);
	newEntry->filename = new char[strlen(name)+1];
	assert(newEntry->filename);
	strncpy(newEntry->filename, name, strlen(name)+1);
	newEntry->size = size;
	newEntry->data = data;

	if (_fileList) {
		_fileList->addEntry(newEntry);
	} else {
		_fileList = newEntry;
	}
	return true;
}

bool PAKFile::removeFile(const char *name) {
	for (FileList *cur = _fileList, *last = 0; cur; last = cur, cur = cur->next) {
		if (scumm_stricmp(cur->filename, name) == 0) {
			FileList *next = cur->next;
			cur->next = 0;
			delete cur;
			if (last)
				last->next = next;
			else
				_fileList = next;
			return true;
		}
	}
	return false;
}
