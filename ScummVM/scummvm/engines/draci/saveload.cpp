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

#include "draci/draci.h"
#include "draci/game.h"
#include "draci/saveload.h"

#include "common/serializer.h"
#include "common/savefile.h"
#include "common/system.h"

#include "graphics/scaler.h"
#include "graphics/thumbnail.h"

namespace Draci {

static const char *const draciIdentString = "DRACI";

bool readSavegameHeader(Common::InSaveFile *in, DraciSavegameHeader &header) {
	char saveIdentBuffer[6];
	header.thumbnail = NULL;

	// Validate the header Id
	in->read(saveIdentBuffer, 6);
	if (strcmp(saveIdentBuffer, draciIdentString) != 0)
		return false;

	header.version = in->readByte();
	if (header.version != DRACI_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header.saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0') header.saveName += ch;

	header.date = in->readUint32LE();
	header.time = in->readUint16LE();
	header.playtime = in->readUint32LE();

	// Get the thumbnail
	header.thumbnail = Graphics::loadThumbnail(*in);
	if (!header.thumbnail)
		return false;

	return true;
}

void writeSavegameHeader(Common::OutSaveFile *out, const DraciSavegameHeader &header) {
	// Write out a savegame header
	out->write(draciIdentString, 6);
	out->writeByte(DRACI_SAVEGAME_VERSION);

	// Write savegame name
	out->write(header.saveName.c_str(), header.saveName.size() + 1);

	out->writeUint32LE(header.date);
	out->writeUint16LE(header.time);
	out->writeUint32LE(header.playtime);

	// Create a thumbnail and save it
	Graphics::saveThumbnail(*out);
}

Common::Error saveSavegameData(int saveGameIdx, const Common::String &saveName, DraciEngine &vm) {
	Common::String filename = vm.getSavegameFile(saveGameIdx);
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::OutSaveFile *f = saveMan->openForSaving(filename);
	if (f == NULL)
		return Common::kNoGameDataFoundError;

	TimeDate curTime;
	vm._system->getTimeAndDate(curTime);

	// Save the savegame header
	DraciSavegameHeader header;
	header.saveName = saveName;
	header.date = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	header.time = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);
	header.playtime = vm.getTotalPlayTime() / 1000;
	writeSavegameHeader(f, header);

	if (f->err()) {
		delete f;
		saveMan->removeSavefile(filename);
		return Common::kWritingFailed;
	} else {
		// Create the remainder of the savegame
		Common::Serializer s(NULL, f);
		vm._game->DoSync(s);

		f->finalize();
		delete f;
		return Common::kNoError;
	}
}

Common::Error loadSavegameData(int saveGameIdx, DraciEngine *vm) {
	Common::String saveName;

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *f = saveMan->openForLoading(vm->getSavegameFile(saveGameIdx));

	if (f == NULL) {
		return Common::kNoGameDataFoundError;
	}

	// Skip over the savegame header
	DraciSavegameHeader header;
	if (!readSavegameHeader(f, header)) {
		return Common::kNoGameDataFoundError;
	}
	if (header.thumbnail) {
		header.thumbnail->free();
		delete header.thumbnail;
	}

	// Pre-processing
	vm->_game->rememberRoomNumAsPrevious();
	vm->_game->deleteObjectAnimations();

	// Synchronise the remaining data of the savegame
	Common::Serializer s(f, NULL);
	vm->_game->DoSync(s);
	delete f;

	// Post-processing
	vm->_game->scheduleEnteringRoomUsingGate(vm->_game->getRoomNum(), 0);
	vm->_game->setExitLoop(true);
	vm->_game->setIsReloaded(true);

	vm->_game->inventoryReload();

	vm->setTotalPlayTime(header.playtime * 1000);

	return Common::kNoError;
}

} // End of namespace Draci
