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

#include "common/endian.h"
#include "common/savefile.h"

#include "gob/gob.h"
#include "gob/save/saveconverter.h"
#include "gob/save/savefile.h"
#include "gob/save/savehandler.h"

namespace Gob {

SaveConverter_v3::SaveConverter_v3(GobEngine *vm, const Common::String &fileName) :
	SaveConverter(vm, fileName) {
}

SaveConverter_v3::~SaveConverter_v3() {
}

int SaveConverter_v3::isOldSave(Common::InSaveFile **save) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);
	if (varSize == 0)
		return 0;

	uint32 saveSize = getActualSize(save);
	if (saveSize == 0)
		return 0;

	// The size of the old save always follows one of these rules
	if (saveSize == (varSize * 2 + kSlotNameLength + 1000))
		return 1; // No screenshot
	if (saveSize == (varSize * 2 + kSlotNameLength + 1000 + 19968))
		return 2; // Big screenshot, Goblins 3
	if (saveSize == (varSize * 2 + kSlotNameLength + 1000 + 4768))
		return 3; // Small screenshot, Lost in Time

	// Not an old save, clean up
	if (save) {
		delete *save;
		*save = 0;
	}

	return 0;
}

char *SaveConverter_v3::getDescription(Common::SeekableReadStream &save) const {
	// The description starts at 1000
	if (!save.seek(1000))
		return 0;

	char *desc = new char[kSlotNameLength];

	// Read the description
	if (save.read(desc, kSlotNameLength) != kSlotNameLength) {
		delete[] desc;
		return 0;
	}

	return desc;
}

bool SaveConverter_v3::loadFail(SavePartInfo *info, SavePartVars *vars,
		SavePartSprite *sprite, Common::InSaveFile *save) {

	delete info;
	delete vars;
	delete sprite;
	delete save;

	clear();

	return false;
}

void SaveConverter_v3::getScreenShotProps(int type,
		bool &used, uint32 &width, uint32 &height) {

	switch (type) {
	case 2:
		used   = true;
		width  = 120;
		height = 160;
		break;

	case 3:
		used   = true;
		width  = 80;
		height = 50;
		break;

	default:
		used   = false;
		width  = 0;
		height = 0;
		break;
	}
}

// Loads the old save by constructing a new save containing the old save's data
bool SaveConverter_v3::load() {
	clear();

	uint32 varSize = SaveHandler::getVarSize(_vm);
	if (varSize == 0)
		return false;

	Common::InSaveFile *save;

	int type = isOldSave(&save);

	// Test if it's an old savd
	if ((type == 0) || !save)
		return false;

	displayWarning();

	bool screenShot;
	uint32 screenShotWidth;
	uint32 screenShotHeight;

	getScreenShotProps(type, screenShot, screenShotWidth, screenShotHeight);

	SaveWriter writer(screenShot ? 3 : 2, 0);

	SavePartInfo *info = readInfo(*save, kSlotNameLength, false);
	if (!info)
		return loadFail(0, 0, 0, save);

	SavePartVars *vars = readVars(*save, varSize, true);
	if (!vars)
		return loadFail(info, 0, 0, save);

	if (screenShot) {
		SavePartSprite *sprite = readSprite(*save, screenShotWidth, screenShotHeight, true);

		if (!sprite)
			return loadFail(info, vars, 0, save);

		if (!writer.writePart(2, sprite))
			return loadFail(info, vars, sprite, save);

		delete sprite;
	}

	// We don't need the save anymore
	delete save;

	// Write all parts
	if (!writer.writePart(0, info))
		return loadFail(info, vars, 0, 0);
	if (!writer.writePart(1, vars))
		return loadFail(info, vars, 0, 0);

	// We don't need those anymore
	delete info;
	delete vars;

	// Create the final read stream
	if (!createStream(writer))
		return loadFail(0, 0, 0, 0);

	return true;
}

} // End of namespace Gob
