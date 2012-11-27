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

#include "gob/gob.h"
#include "gob/save/saveload.h"
#include "gob/global.h"
#include "gob/video.h"
#include "gob/draw.h"

namespace Gob {

SaveLoad::SaveLoad(GobEngine *vm) : _vm(vm) {
}

SaveLoad::~SaveLoad() {
}

const char *SaveLoad::stripPath(const char *fileName) {
	const char *backSlash;
	if ((backSlash = strrchr(fileName, '\\')))
		return backSlash + 1;

	return fileName;
}

int32 SaveLoad::getSize(const char *fileName) {
	debugC(3, kDebugSaveLoad, "Requested size of save file \"%s\"", fileName);

	SaveHandler *handler = getHandler(fileName);

	if (!handler) {
		warning("No save handler for \"%s\"", fileName);
		return -1;
	}

	int32 size = handler->getSize();

	debugC(4, kDebugSaveLoad, "Size is %d", size);

	return size;
}

bool SaveLoad::load(const char *fileName, int16 dataVar, int32 size, int32 offset) {
	debugC(3, kDebugSaveLoad, "Requested loading of save file \"%s\" - %d, %d, %d",
			fileName, dataVar, size, offset);

	SaveHandler *handler = getHandler(fileName);

	if (!handler) {
		warning("No save handler for \"%s\" (%d, %d, %d)", fileName, dataVar, size, offset);
		return false;
	}

	if (!handler->load(dataVar, size, offset)) {
		const char *desc = getDescription(fileName);

		if (!desc)
			desc = "Unknown";

		warning("Could not load %s (\"%s\" (%d, %d, %d))",
				desc, fileName, dataVar, size, offset);
		return false;
	}

	debugC(3, kDebugSaveLoad, "Successfully loaded game");
	return true;
}

bool SaveLoad::save(const char *fileName, int16 dataVar, int32 size, int32 offset) {
	debugC(3, kDebugSaveLoad, "Requested saving of save file \"%s\" - %d, %d, %d",
			fileName, dataVar, size, offset);

	SaveHandler *handler = getHandler(fileName);

	if (!handler) {
		warning("No save handler for \"%s\" (%d, %d, %d)", fileName, dataVar, size, offset);
		return false;
	}

	if (!handler->save(dataVar, size, offset)) {
		const char *desc = getDescription(fileName);

		if (!desc)
			desc = "Unknown";

		warning("Could not save %s (\"%s\" (%d, %d, %d))",
				desc, fileName, dataVar, size, offset);
		return false;
	}

	debugC(3, kDebugSaveLoad, "Successfully saved game");
	return true;
}

bool SaveLoad::deleteFile(const char *fileName) {
	debugC(3, kDebugSaveLoad, "Requested deletion save file \"%s\"", fileName);

	SaveHandler *handler = getHandler(fileName);

	if (!handler) {
		warning("No save handler for \"%s\"", fileName);
		return false;
	}

	if (!handler->deleteFile()) {
		const char *desc = getDescription(fileName);

		if (!desc)
			desc = "Unknown";

		warning("Could not delete %s (\"%s\")", desc, fileName);
		return false;
	}

	debugC(3, kDebugSaveLoad, "Successfully deleted file");
	return true;
}

SaveLoad::SaveMode SaveLoad::getSaveMode(const char *fileName) const {
	return kSaveModeNone;
}

SaveHandler *SaveLoad::getHandler(const char *fileName) const {
	return 0;
}

const char *SaveLoad::getDescription(const char *fileName) const {
	return 0;
}

} // End of namespace Gob
