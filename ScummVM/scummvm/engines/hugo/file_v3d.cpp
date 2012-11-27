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
#include "hugo/display.h"
#include "hugo/text.h"
#include "hugo/util.h"

namespace Hugo {
FileManager_v3d::FileManager_v3d(HugoEngine *vm) : FileManager_v2d(vm) {
}

FileManager_v3d::~FileManager_v3d() {
}

/**
 * Read a PCX image into dib_a
 */
void FileManager_v3d::readBackground(const int screenIndex) {
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

	Seq *dummySeq;                                  // Image sequence structure for Read_pcx
	if (screenIndex < 20) {
		_sceneryArchive1.seek(sceneBlock._sceneOffset, SEEK_SET);
		// Read the image into dummy seq and static dib_a
		dummySeq = readPCX(_sceneryArchive1, 0, _vm->_screen->getFrontBuffer(), true, _vm->_text->getScreenNames(screenIndex));
	} else {
		_sceneryArchive2.seek(sceneBlock._sceneOffset, SEEK_SET);
		// Read the image into dummy seq and static dib_a
		dummySeq = readPCX(_sceneryArchive2, 0, _vm->_screen->getFrontBuffer(), true, _vm->_text->getScreenNames(screenIndex));
	}
	free(dummySeq);
}

/**
 * Open "database" file (packed files)
 */
void FileManager_v3d::openDatabaseFiles() {
	debugC(1, kDebugFile, "openDatabaseFiles");

	if (!_stringArchive.open(getStringFilename()))
		error("File not found: %s", getStringFilename());
	if (!_sceneryArchive1.open("scenery1.dat"))
		error("File not found: scenery1.dat");
	if (!_sceneryArchive2.open("scenery2.dat"))
		error("File not found: scenery2.dat");
	if (!_objectsArchive.open(getObjectFilename()))
		error("File not found: %s", getObjectFilename());
}

/**
 * Close "Database" files
 */
void FileManager_v3d::closeDatabaseFiles() {
	debugC(1, kDebugFile, "closeDatabaseFiles");

	_stringArchive.close();
	_sceneryArchive1.close();
	_sceneryArchive2.close();
	_objectsArchive.close();
}

/**
 * Open and read in an overlay file, close file
 */
void FileManager_v3d::readOverlay(const int screenNum, ImagePtr image, OvlType overlayType) {
	debugC(1, kDebugFile, "readOverlay(%d, ...)", screenNum);

	ImagePtr     tmpImage = image;                  // temp ptr to overlay file
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

	if (screenNum < 20) {
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
		int16 k = 0;                                // byte count
		do {
			int8 data = _sceneryArchive1.readByte();// Read a code byte
			if ((byte)data == 0x80)                 // Noop
				;
			else if (data >= 0) {                   // Copy next data+1 literally
				for (i = 0; i <= (byte)data; i++, k++)
					*tmpImage++ = _sceneryArchive1.readByte();
			} else {                                // Repeat next byte -data+1 times
				int16 j = _sceneryArchive1.readByte();

				for (i = 0; i < (byte)(-data + 1); i++, k++)
					*tmpImage++ = j;
			}
		} while (k < kOvlSize);
	} else {
		switch (overlayType) {
		case kOvlBoundary:
			_sceneryArchive2.seek(sceneBlock._boundaryOffset, SEEK_SET);
			i = sceneBlock._boundaryLength;
			break;
		case kOvlOverlay:
			_sceneryArchive2.seek(sceneBlock._overlayOffset, SEEK_SET);
			i = sceneBlock._overlayLength;
			break;
		case kOvlBase:
			_sceneryArchive2.seek(sceneBlock._baseOffset, SEEK_SET);
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
		int16 k = 0;                                // byte count
		do {
			int8 data = _sceneryArchive2.readByte();// Read a code byte
			if ((byte)data == 0x80)                 // Noop
				;
			else if (data >= 0) {                   // Copy next data+1 literally
				for (i = 0; i <= (byte)data; i++, k++)
					*tmpImage++ = _sceneryArchive2.readByte();
			} else {                                // Repeat next byte -data+1 times
				int16 j = _sceneryArchive2.readByte();

				for (i = 0; i < (byte)(-data + 1); i++, k++)
					*tmpImage++ = j;
			}
		} while (k < kOvlSize);
	}
}
} // End of namespace Hugo
