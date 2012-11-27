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

// Disable symbol overrides for FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#include "gbampsave.h"
#include "fat/gba_nds_fat.h"
#include "backends/fs/ds/ds-fs.h"
#include "common/config-manager.h"
#include "common/bufferedstream.h"

#define SAVE_BUFFER_SIZE 100000

// This method copied from an old version of the savefile.cpp, since it's been removed from there and
// placed in default-saves.cpp, where I cannot call it.
// FIXME: Does it even make sense to change the "savepath" on the NDS? Considering
// that nothing sets a default value for the "savepath" either, wouldn't it better
// to return a fixed path here?
static Common::String getSavePath() {
	// Try to use game specific savepath from config
	return ConfMan.get("savepath");
}

//////////////////////////
// GBAMP Save File Manager
//////////////////////////

Common::OutSaveFile *GBAMPSaveFileManager::openForSaving(const Common::String &filename, bool compress) {
	Common::String fileSpec = getSavePath();
	if (fileSpec.lastChar() != '/')
		fileSpec += '/';
	fileSpec += filename;

//	consolePrintf("Opening the file: %s\n", fileSpec.c_str());

	Common::WriteStream *stream = DS::DSFileStream::makeFromPath(fileSpec, true);
	// Use a write buffer
	stream = Common::wrapBufferedWriteStream(stream, SAVE_BUFFER_SIZE);
	return stream;
}

Common::InSaveFile *GBAMPSaveFileManager::openForLoading(const Common::String &filename) {
	Common::String fileSpec = getSavePath();
	if (fileSpec.lastChar() != '/')
		fileSpec += '/';
	fileSpec += filename;

//	consolePrintf("Opening the file: %s\n", fileSpec.c_str());

	return DS::DSFileStream::makeFromPath(fileSpec, false);
}


bool GBAMPSaveFileManager::removeSavefile(const Common::String &filename) {
	return false; // TODO: Implement this
}


Common::StringArray GBAMPSaveFileManager::listSavefiles(const Common::String &pattern) {

	enum { TYPE_NO_MORE = 0, TYPE_FILE = 1, TYPE_DIR = 2 };
	char name[256];

	{
		char dir[128];
		strcpy(dir, getSavePath().c_str());
		char *realName = dir;

		if ((strlen(dir) >= 4) && (dir[0] == 'm') && (dir[1] == 'p') && (dir[2] == ':') && (dir[3] == '/')) {
			realName += 4;
		}

	//	consolePrintf("Real cwd:%d\n", realName);

		char *p = realName;
		while (*p) {
			if (*p == '\\') *p = '/';
			p++;
		}

	//	consolePrintf("Real cwd:%d\n", realName);
		FAT_chdir(realName);

	}

//	consolePrintf("Save path: '%s', pattern: '%s'\n", getSavePath(), pattern);


	int fileType = FAT_FindFirstFileLFN(name);

	Common::StringArray list;

	do {

		if (fileType == TYPE_FILE) {

			FAT_GetLongFilename(name);

			for (int r = 0; name[r] != 0; r++) {
				name[r] = tolower(name[r]);
			}


			if (Common::matchString(name, pattern.c_str())) {
				list.push_back(name);
			}
		}

	} while ((fileType = FAT_FindNextFileLFN(name)));

	FAT_chdir("/");

	return list;
}
