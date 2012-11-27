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
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "hugo/hugo.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/text.h"
#include "hugo/util.h"

namespace Hugo {
FileManager_v2d::FileManager_v2d(HugoEngine *vm) : FileManager_v1d(vm) {
	_fetchStringBuf = (char *)malloc(kMaxBoxChar);
}

FileManager_v2d::~FileManager_v2d() {
	free(_fetchStringBuf);
}

/**
 * Open "database" file (packed files)
 */
void FileManager_v2d::openDatabaseFiles() {
	debugC(1, kDebugFile, "openDatabaseFiles");

	if (!_stringArchive.open(getStringFilename()))
		error("File not found: %s", getStringFilename());
	if (!_sceneryArchive1.open(getSceneryFilename()))
		error("File not found: %s", getSceneryFilename());
	if (!_objectsArchive.open(getObjectFilename()))
		error("File not found: %s", getObjectFilename());
}

/**
 * Close "Database" files
 */
void FileManager_v2d::closeDatabaseFiles() {
	debugC(1, kDebugFile, "closeDatabaseFiles");

	_stringArchive.close();
	_sceneryArchive1.close();
	_objectsArchive.close();
}

/**
 * Read a PCX image into dib_a
 */
void FileManager_v2d::readBackground(const int screenIndex) {
	debugC(1, kDebugFile, "readBackground(%d)", screenIndex);

	_sceneryArchive1.seek((uint32) screenIndex * sizeof(SceneBlock), SEEK_SET);

	SceneBlock sceneBlock;                          // Read a database header entry
	sceneBlock._sceneOffset = _sceneryArchive1.readUint32LE();
	sceneBlock._sceneLength = _sceneryArchive1.readUint32LE();
	sceneBlock._boundaryOffset = _sceneryArchive1.readUint32LE();
	sceneBlock._boundaryLength = _sceneryArchive1.readUint32LE();
	sceneBlock._overlayOffset = _sceneryArchive1.readUint32LE();
	sceneBlock._overlayLength = _sceneryArchive1.readUint32LE();
	sceneBlock._baseOffset = _sceneryArchive1.readUint32LE();
	sceneBlock._baseLength = _sceneryArchive1.readUint32LE();

	_sceneryArchive1.seek(sceneBlock._sceneOffset, SEEK_SET);

	// Read the image into dummy seq and static dib_a
	Seq *dummySeq;                                  // Image sequence structure for Read_pcx
	dummySeq = readPCX(_sceneryArchive1, 0, _vm->_screen->getFrontBuffer(), true, _vm->_text->getScreenNames(screenIndex));
	free(dummySeq);
}

/**
 * Open and read in an overlay file, close file
 */
void FileManager_v2d::readOverlay(const int screenNum, ImagePtr image, OvlType overlayType) {
	debugC(1, kDebugFile, "readOverlay(%d, ...)", screenNum);

	ImagePtr tmpImage = image;                      // temp ptr to overlay file
	_sceneryArchive1.seek((uint32)screenNum * sizeof(SceneBlock), SEEK_SET);

	SceneBlock sceneBlock;                          // Database header entry
	sceneBlock._sceneOffset = _sceneryArchive1.readUint32LE();
	sceneBlock._sceneLength = _sceneryArchive1.readUint32LE();
	sceneBlock._boundaryOffset = _sceneryArchive1.readUint32LE();
	sceneBlock._boundaryLength = _sceneryArchive1.readUint32LE();
	sceneBlock._overlayOffset = _sceneryArchive1.readUint32LE();
	sceneBlock._overlayLength = _sceneryArchive1.readUint32LE();
	sceneBlock._baseOffset = _sceneryArchive1.readUint32LE();
	sceneBlock._baseLength = _sceneryArchive1.readUint32LE();

	uint32 i = 0;
	switch (overlayType) {
	case kOvlBoundary:
		_sceneryArchive1.seek(sceneBlock._boundaryOffset, SEEK_SET);
		i = sceneBlock._boundaryLength;
		break;
	case kOvlOverlay:
		_sceneryArchive1.seek(sceneBlock._overlayOffset, SEEK_SET);
		i = sceneBlock._overlayLength;
		break;
	case kOvlBase:
		_sceneryArchive1.seek(sceneBlock._baseOffset, SEEK_SET);
		i = sceneBlock._baseLength;
		break;
	default:
		error("Bad overlayType: %d", overlayType);
		break;
	}
	if (i == 0) {
		memset(image, 0, kOvlSize);
		return;
	}

	// Read in the overlay file using MAC Packbits.  (We're not proud!)
	int16 k = 0;                                    // byte count
	do {
		int8 data = _sceneryArchive1.readByte();    // Read a code byte
		if ((byte)data == 0x80)                     // Noop
			;
		else if (data >= 0) {                       // Copy next data+1 literally
			for (i = 0; i <= (byte)data; i++, k++)
				*tmpImage++ = _sceneryArchive1.readByte();
		} else {                                    // Repeat next byte -data+1 times
			int16 j = _sceneryArchive1.readByte();

			for (i = 0; i < (byte)(-data + 1); i++, k++)
				*tmpImage++ = j;
		}
	} while (k < kOvlSize);
}

/**
 * Fetch string from file, decode and return ptr to string in memory
 */
const char *FileManager_v2d::fetchString(const int index) {
	debugC(1, kDebugFile, "fetchString(%d)", index);

	// Get offset to string[index] (and next for length calculation)
	_stringArchive.seek((uint32)index * sizeof(uint32), SEEK_SET);

	uint32 off1 = _stringArchive.readUint32LE();
	uint32 off2 = _stringArchive.readUint32LE();
	if (!off1 || !off2)
		error("An error has occurred: bad String offset");

	// Check size of string
	if ((off2 - off1) >= (uint32) kMaxBoxChar)
		error("Fetched string too long!");

	// Position to string and read it into gen purpose _textBoxBuffer
	_stringArchive.seek(off1, SEEK_SET);
	if (_stringArchive.read(_fetchStringBuf, (uint16)(off2 - off1)) == 0)
		error("An error has occurred: fetchString");

	// Null terminate, decode and return it
	_fetchStringBuf[off2-off1] = '\0';
	_vm->_scheduler->decodeString(_fetchStringBuf);
	return _fetchStringBuf;
}
} // End of namespace Hugo
