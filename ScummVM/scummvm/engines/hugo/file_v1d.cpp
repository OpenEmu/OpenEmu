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
FileManager_v1d::FileManager_v1d(HugoEngine *vm) : FileManager(vm) {
}

FileManager_v1d::~FileManager_v1d() {
}

void FileManager_v1d::openDatabaseFiles() {
	debugC(1, kDebugFile, "openDatabaseFiles");
}

void FileManager_v1d::closeDatabaseFiles() {
	debugC(1, kDebugFile, "closeDatabaseFiles");
}

/**
 * Open and read in an overlay file, close file
 */
void FileManager_v1d::readOverlay(const int screenNum, ImagePtr image, const OvlType overlayType) {
	debugC(1, kDebugFile, "readOverlay(%d, ...)", screenNum);

	const char *ovlExt[] = {".b", ".o", ".ob"};
	Common::String buf = Common::String(_vm->_text->getScreenNames(screenNum)) + Common::String(ovlExt[overlayType]);

	if (!Common::File::exists(buf)) {
		memset(image, 0, kOvlSize);
		warning("File not found: %s", buf.c_str());
		return;
	}

	if (!_sceneryArchive1.open(buf))
		error("File not found: %s", buf.c_str());

	ImagePtr tmpImage = image;                      // temp ptr to overlay file

	_sceneryArchive1.read(tmpImage, kOvlSize);
	_sceneryArchive1.close();
}

/**
 * Read a PCX image into dib_a
 */
void FileManager_v1d::readBackground(const int screenIndex) {
	debugC(1, kDebugFile, "readBackground(%d)", screenIndex);

	Common::String buf;
	buf = Common::String(_vm->_text->getScreenNames(screenIndex)) + ".ART";
	if (!_sceneryArchive1.open(buf))
		error("File not found: %s", buf.c_str());
	// Read the image into dummy seq and static dib_a
	Seq *dummySeq;                                // Image sequence structure for Read_pcx
	dummySeq = readPCX(_sceneryArchive1, 0, _vm->_screen->getFrontBuffer(), true, _vm->_text->getScreenNames(screenIndex));
	free(dummySeq);
	_sceneryArchive1.close();
}

const char *FileManager_v1d::fetchString(const int index) {
	debugC(1, kDebugFile, "fetchString(%d)", index);

	return _vm->_text->getStringtData(index);
}

/**
 * Simple instructions given when F1 pressed twice in a row
 * Only in DOS versions
 */
void FileManager_v1d::instructions() const {
	Common::File f;
	if (!f.open("help.dat")) {
		warning("help.dat not found");
		return;
	}

	char readBuf[2];
	while (f.read(readBuf, 1)) {
		char line[1024], *wrkLine;
		wrkLine = line;
		wrkLine[0] = readBuf[0];
		wrkLine++;
		do {
			f.read(wrkLine, 1);
		} while (*wrkLine++ != '#');                // '#' is EOP
		wrkLine[-2] = '\0';                         // Remove EOP and previous CR
		Utils::notifyBox(line);
		wrkLine = line;
		f.read(readBuf, 2);                         // Remove CRLF after EOP
	}
	f.close();
}

} // End of namespace Hugo
