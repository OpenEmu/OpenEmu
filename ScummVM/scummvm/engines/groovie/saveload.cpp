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

#include "groovie/saveload.h"

#include "common/system.h"
#include "common/substream.h"

#define SUPPORTED_SAVEFILE_VERSION 1
// 0 - Just script variables, compatible with the original
// 1 - Added one byte with version number at the beginning

namespace Groovie {

int SaveLoad::getMaximumSlot() {
	return 9;
}

bool SaveLoad::isSlotValid(int slot) {
	return slot >= 0 && slot <= getMaximumSlot();
}

Common::String SaveLoad::getSlotSaveName(const Common::String &target, int slot) {
	return target + ".00" + ('0' + slot);
}

SaveStateList SaveLoad::listValidSaves(const Common::String &target) {
	SaveStateList list;

	// Get the list of savefiles
	Common::String pattern = target + ".00?";
	Common::StringArray savefiles = g_system->getSavefileManager()->listSavefiles(pattern);

	// Sort the list of filenames
	sort(savefiles.begin(), savefiles.end());

	// Fill the information for the existing savegames
	Common::StringArray::iterator it = savefiles.begin();
	while (it != savefiles.end()) {
		int slot = it->lastChar() - '0';
		SaveStateDescriptor descriptor;
		Common::InSaveFile *file = SaveLoad::openForLoading(target, slot, &descriptor);
		if (file) {
			// It's a valid savefile, save the descriptor
			delete file;
			list.push_back(descriptor);
		}
		it++;
	}

	return list;
}

Common::InSaveFile *SaveLoad::openForLoading(const Common::String &target, int slot, SaveStateDescriptor *descriptor) {
	// Validate the slot number
	if (!isSlotValid(slot)) {
		return NULL;
	}

	// Open the savefile
	Common::String savename = getSlotSaveName(target, slot);
	Common::InSaveFile *savefile = g_system->getSavefileManager()->openForLoading(savename);
	if (!savefile) {
		return NULL;
	}

	// Read the savefile version
	uint8 version;
	if (savefile->size() == 1024) {
		version = 0;
	} else {
		version = savefile->readByte();
	}

	// Verify we can read this version
	if (version > SUPPORTED_SAVEFILE_VERSION) {
		//TODO: show the error about unsupported savefile version
	}

	// Save the current position as the start for the engine data
	int metaDataSize = savefile->pos();

	// Fill the SaveStateDescriptor if it was provided
	if (descriptor) {
		// Initialize the SaveStateDescriptor
		descriptor->setSaveSlot(slot);

		// TODO: Add extra information
		//setSaveDate(int year, int month, int day)
		//setSaveTime(int hour, int min)
		//setPlayTime(int hours, int minutes)

		// Read the savegame description
		Common::String description;
		unsigned char c = 1;
		for (int i = 0; (c != 0) && (i < 15); i++) {
			c = savefile->readByte();
			switch (c) {
				case 0:
					break;
				case 16: // @
					c = ' ';
					break;
				case 244: // $
					c = 0;
					break;
				default:
					c += 0x30;
			}
			if (c != 0) {
				description += c;
			}
		}
		descriptor->setDescription(description);
	}

	// Return a substream, skipping the metadata
	Common::SeekableSubReadStream *sub = new Common::SeekableSubReadStream(savefile, metaDataSize, savefile->size(), DisposeAfterUse::YES);

	// Move to the beginning of the substream
	sub->seek(0, SEEK_SET);

	return sub;
}

Common::OutSaveFile *SaveLoad::openForSaving(const Common::String &target, int slot) {
	// Validate the slot number
	if (!isSlotValid(slot)) {
		return NULL;
	}

	// Open the savefile
	Common::String savename = getSlotSaveName(target, slot);
	Common::OutSaveFile *savefile = g_system->getSavefileManager()->openForSaving(savename);
	if (!savefile) {
		return NULL;
	}

	// Write the savefile version
	savefile->writeByte(SUPPORTED_SAVEFILE_VERSION);

	return savefile;
}

} // End of Groovie namespace
