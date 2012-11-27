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

SaveConverter_v2::SaveConverter_v2(GobEngine *vm, const Common::String &fileName) :
	SaveConverter(vm, fileName) {
}

SaveConverter_v2::~SaveConverter_v2() {
}

int SaveConverter_v2::isOldSave(Common::InSaveFile **save) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);
	if (varSize == 0)
		return 0;

	uint32 saveSize = getActualSize(save);
	if (saveSize == 0)
		return 0;

	// The size of the old save always follows that rule
	if (saveSize == (varSize * 2 + kSlotNameLength * 2))
		return 1;

	// Not an old save, clean up
	if (save) {
		delete *save;
		*save = 0;
	}

	return 0;
}

char *SaveConverter_v2::getDescription(Common::SeekableReadStream &save) const {
	char *desc = new char[kSlotNameLength];

	// Read the description
	if (save.read(desc, kSlotNameLength) != kSlotNameLength) {
		delete[] desc;
		return 0;
	}

	return desc;
}

bool SaveConverter_v2::loadFail(SavePartInfo *info, SavePartVars *vars,
		Common::InSaveFile *save) {

	delete info;
	delete vars;
	delete save;

	clear();

	return false;
}

// Loads the old save by constructing a new save containing the old save's data
bool SaveConverter_v2::load() {
	clear();

	uint32 varSize = SaveHandler::getVarSize(_vm);
	if (varSize == 0)
		return false;

	Common::InSaveFile *save;

	// Test if it's an old savd
	if (!isOldSave(&save) || !save)
		return false;

	displayWarning();

	SaveWriter writer(2, 0);

	SavePartInfo *info = readInfo(*save, kSlotNameLength);
	if (!info)
		return loadFail(0, 0, save);

	SavePartVars *vars = readVars(*save, varSize, true);
	if (!vars)
		return loadFail(info, 0, save);

	// We don't need the save anymore
	delete save;

	// Write all parts
	if (!writer.writePart(0, info))
		return loadFail(info, vars, 0);
	if (!writer.writePart(1, vars))
		return loadFail(info, vars, 0);

	// We don't need those anymore
	delete info;
	delete vars;

	// Create the final read stream
	if (!createStream(writer))
		return loadFail(0, 0, 0);

	return true;
}

} // End of namespace Gob
