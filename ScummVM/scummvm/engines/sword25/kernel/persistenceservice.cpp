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
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "common/fs.h"
#include "common/savefile.h"
#include "common/zlib.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/persistenceservice.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/filesystemutil.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/sfx/soundengine.h"
#include "sword25/input/inputengine.h"
#include "sword25/math/regionregistry.h"
#include "sword25/script/script.h"

namespace Sword25 {

//static const char *SAVEGAME_EXTENSION = ".b25s";
static const char *SAVEGAME_DIRECTORY = "saves";
static const char *FILE_MARKER = "BS25SAVEGAME";
static const uint  SLOT_COUNT = 18;
static const uint  FILE_COPY_BUFFER_SIZE = 1024 * 10;
static const char *VERSIONIDOLD = "SCUMMVM1";
static const char *VERSIONID = "SCUMMVM2";
static const int   VERSIONNUM = 2;

#define MAX_SAVEGAME_SIZE 100

char gameTarget[MAX_SAVEGAME_SIZE];

void setGameTarget(const char *target) {
	strncpy(gameTarget, target, MAX_SAVEGAME_SIZE);
}

static Common::String generateSavegameFilename(uint slotID) {
	char buffer[MAX_SAVEGAME_SIZE];
	snprintf(buffer, MAX_SAVEGAME_SIZE, "%s.%.3d", gameTarget, slotID);
	return Common::String(buffer);
}

static Common::String formatTimestamp(TimeDate time) {
	// In the original BS2.5 engine, this used a local object to show the date/time as as a string.
	// For now in ScummVM it's being hardcoded to 'dd-MON-yyyy hh:mm:ss'
	Common::String monthList[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	char buffer[100];
	snprintf(buffer, 100, "%.2d-%s-%.4d %.2d:%.2d:%.2d",
	         time.tm_mday, monthList[time.tm_mon].c_str(), 1900 + time.tm_year,
	         time.tm_hour, time.tm_min, time.tm_sec
	        );

	return Common::String(buffer);
}

static Common::String loadString(Common::InSaveFile *in, uint maxSize = 999) {
	Common::String result;

	char ch = (char)in->readByte();
	while (ch != '\0') {
		result += ch;
		if (result.size() >= maxSize)
			break;
		ch = (char)in->readByte();
	}

	return result;
}

struct SavegameInformation {
	bool isOccupied;
	bool isCompatible;
	Common::String description;
	int  version;
	uint gamedataLength;
	uint gamedataOffset;
	uint gamedataUncompressedLength;

	SavegameInformation() {
		clear();
	}

	void clear() {
		isOccupied = false;
		isCompatible = false;
		description = "";
		gamedataLength = 0;
		gamedataOffset = 0;
		gamedataUncompressedLength = 0;
	}
};

struct PersistenceService::Impl {
	SavegameInformation _savegameInformations[SLOT_COUNT];

	Impl() {
		reloadSlots();
	}

	void reloadSlots() {
		// Iterate through all the saved games, and read their thumbnails.
		for (uint i = 0; i < SLOT_COUNT; ++i) {
			readSlotSavegameInformation(i);
		}
	}

	void readSlotSavegameInformation(uint slotID) {
		// Get the information corresponding to the requested save slot.
		SavegameInformation &curSavegameInfo = _savegameInformations[slotID];
		curSavegameInfo.clear();

		// Generate the save slot file name.
		Common::String filename = generateSavegameFilename(slotID);

		// Try to open the savegame for loading
		Common::SaveFileManager *sfm = g_system->getSavefileManager();
		Common::InSaveFile *file = sfm->openForLoading(filename);

		if (file) {
			// Read in the header
			Common::String storedMarker = loadString(file);
			Common::String storedVersionID = loadString(file);
			if (storedVersionID == VERSIONIDOLD) {
				curSavegameInfo.version = 1;
			} else {
				Common::String versionNum = loadString(file);
				curSavegameInfo.version = atoi(versionNum.c_str());
			}
			Common::String gameDescription = loadString(file);
			Common::String gamedataLength = loadString(file);
			curSavegameInfo.gamedataLength = atoi(gamedataLength.c_str());
			Common::String gamedataUncompressedLength = loadString(file);
			curSavegameInfo.gamedataUncompressedLength = atoi(gamedataUncompressedLength.c_str());

			// If the header can be read in and is detected to be valid, we will have a valid file
			if (storedMarker == FILE_MARKER) {
				// The slot is marked as occupied.
				curSavegameInfo.isOccupied = true;
				// Check if the saved game is compatible with the current engine version.
				curSavegameInfo.isCompatible = (curSavegameInfo.version <= VERSIONNUM);
				// Load the save game description.
				curSavegameInfo.description = gameDescription;
				// The offset to the stored save game data within the file.
				// This reflects the current position, as the header information
				// is still followed by a space as separator.
				curSavegameInfo.gamedataOffset = static_cast<uint>(file->pos());
			}

			delete file;
		}
	}
};

PersistenceService &PersistenceService::getInstance() {
	static PersistenceService instance;
	return instance;
}

PersistenceService::PersistenceService() : _impl(new Impl) {
}

PersistenceService::~PersistenceService() {
	delete _impl;
}

void PersistenceService::reloadSlots() {
	_impl->reloadSlots();
}

uint PersistenceService::getSlotCount() {
	return SLOT_COUNT;
}

Common::String PersistenceService::getSavegameDirectory() {
	Common::FSNode node(FileSystemUtil::getUserdataDirectory());
	Common::FSNode childNode = node.getChild(SAVEGAME_DIRECTORY);

	// Try and return the path using the savegame subfolder. But if doesn't exist, fall back on the data directory
	if (childNode.exists())
		return childNode.getPath();

	return node.getPath();
}

namespace {
bool checkslotID(uint slotID) {
	// Überprüfen, ob die Slot-ID zulässig ist.
	if (slotID >= SLOT_COUNT) {
		error("Tried to access an invalid slot (%d). Only slot ids from 0 to %d are allowed.", slotID, SLOT_COUNT - 1);
		return false;
	} else {
		return true;
	}
}
}

bool PersistenceService::isSlotOccupied(uint slotID) {
	if (!checkslotID(slotID))
		return false;
	return _impl->_savegameInformations[slotID].isOccupied;
}

bool PersistenceService::isSavegameCompatible(uint slotID) {
	if (!checkslotID(slotID))
		return false;
	return _impl->_savegameInformations[slotID].isCompatible;
}

Common::String &PersistenceService::getSavegameDescription(uint slotID) {
	static Common::String emptyString;
	if (!checkslotID(slotID))
		return emptyString;
	return _impl->_savegameInformations[slotID].description;
}

Common::String &PersistenceService::getSavegameFilename(uint slotID) {
	static Common::String result;
	if (!checkslotID(slotID))
		return result;
	result = generateSavegameFilename(slotID);
	return result;
}

int PersistenceService::getSavegameVersion(uint slotID) {
	if (!checkslotID(slotID))
		return -1;
	return _impl->_savegameInformations[slotID].version;
}

bool PersistenceService::saveGame(uint slotID, const Common::String &screenshotFilename) {
	// FIXME: This code is a hack which bypasses the savefile API,
	// and should eventually be removed.

	// Überprüfen, ob die Slot-ID zulässig ist.
	if (slotID >= SLOT_COUNT) {
		error("Tried to save to an invalid slot (%d). Only slot ids form 0 to %d are allowed.", slotID, SLOT_COUNT - 1);
		return false;
	}

	// Dateinamen erzeugen.
	Common::String filename = generateSavegameFilename(slotID);

	// Spielstanddatei öffnen und die Headerdaten schreiben.
	Common::SaveFileManager *sfm = g_system->getSavefileManager();
	Common::OutSaveFile *file = sfm->openForSaving(filename);

	file->writeString(FILE_MARKER);
	file->writeByte(0);
	file->writeString(VERSIONID);
	file->writeByte(0);

	char buf[20];
	snprintf(buf, 20, "%d", VERSIONNUM);
	file->writeString(buf);
	file->writeByte(0);

	TimeDate dt;
	g_system->getTimeAndDate(dt);
	file->writeString(formatTimestamp(dt));
	file->writeByte(0);

	if (file->err()) {
		error("Unable to write header data to savegame file \"%s\".", filename.c_str());
	}

	// Alle notwendigen Module persistieren.
	OutputPersistenceBlock writer;
	bool success = true;
	success &= Kernel::getInstance()->getScript()->persist(writer);
	success &= RegionRegistry::instance().persist(writer);
	success &= Kernel::getInstance()->getGfx()->persist(writer);
	success &= Kernel::getInstance()->getSfx()->persist(writer);
	success &= Kernel::getInstance()->getInput()->persist(writer);
	if (!success) {
		error("Unable to persist modules for savegame file \"%s\".", filename.c_str());
	}

	// Write the save game data uncompressed, since the final saved game will be
	// compressed anyway.
	char sBuffer[10];
	snprintf(sBuffer, 10, "%u", writer.getDataSize());
	file->writeString(sBuffer);
	file->writeByte(0);
	snprintf(sBuffer, 10, "%u", writer.getDataSize());
	file->writeString(sBuffer);
	file->writeByte(0);
	file->write(writer.getData(), writer.getDataSize());

	// Get the screenshot
	Common::SeekableReadStream *thumbnail = Kernel::getInstance()->getGfx()->getThumbnail();

	if (thumbnail) {
		byte *buffer = new byte[FILE_COPY_BUFFER_SIZE];
		thumbnail->seek(0, SEEK_SET);
		while (!thumbnail->eos()) {
			int bytesRead = thumbnail->read(&buffer[0], FILE_COPY_BUFFER_SIZE);
			file->write(&buffer[0], bytesRead);
		}

		delete[] buffer;
	} else {
		warning("The screenshot file \"%s\" does not exist. Savegame is written without a screenshot.", filename.c_str());
	}

	file->finalize();
	delete file;

	// Savegameinformationen für diesen Slot aktualisieren.
	_impl->readSlotSavegameInformation(slotID);

	// Empty the cache, to remove old thumbnails
	Kernel::getInstance()->getResourceManager()->emptyThumbnailCache();

	// Erfolg signalisieren.
	return true;
}

bool PersistenceService::loadGame(uint slotID) {
	Common::SaveFileManager *sfm = g_system->getSavefileManager();
	Common::InSaveFile *file;

	// Überprüfen, ob die Slot-ID zulässig ist.
	if (slotID >= SLOT_COUNT) {
		error("Tried to load from an invalid slot (%d). Only slot ids form 0 to %d are allowed.", slotID, SLOT_COUNT - 1);
		return false;
	}

	SavegameInformation &curSavegameInfo = _impl->_savegameInformations[slotID];

	// Überprüfen, ob der Slot belegt ist.
	if (!curSavegameInfo.isOccupied) {
		error("Tried to load from an empty slot (%d).", slotID);
		return false;
	}

	// Überprüfen, ob der Spielstand im angegebenen Slot mit der aktuellen Engine-Version kompatibel ist.
	// Im Debug-Modus wird dieser Test übersprungen. Für das Testen ist es hinderlich auf die Einhaltung dieser strengen Bedingung zu bestehen,
	// da sich die Versions-ID bei jeder Codeänderung mitändert.
#ifndef DEBUG
	if (!curSavegameInfo.isCompatible) {
		error("Tried to load a savegame (%d) that is not compatible with this engine version.", slotID);
		return false;
	}
#endif

	byte *compressedDataBuffer = new byte[curSavegameInfo.gamedataLength];
	byte *uncompressedDataBuffer = new byte[curSavegameInfo.gamedataUncompressedLength];
	Common::String filename = generateSavegameFilename(slotID);
	file = sfm->openForLoading(filename);

	file->seek(curSavegameInfo.gamedataOffset);
	file->read(reinterpret_cast<char *>(&compressedDataBuffer[0]), curSavegameInfo.gamedataLength);
	if (file->err()) {
		error("Unable to load the gamedata from the savegame file \"%s\".", filename.c_str());
		delete[] compressedDataBuffer;
		delete[] uncompressedDataBuffer;
		return false;
	}

	// Uncompress game data, if needed.
	unsigned long uncompressedBufferSize = curSavegameInfo.gamedataUncompressedLength;

	if (uncompressedBufferSize > curSavegameInfo.gamedataLength) {
		// Older saved game, where the game data was compressed again.
		if (!Common::uncompress(reinterpret_cast<byte *>(&uncompressedDataBuffer[0]), &uncompressedBufferSize,
					   reinterpret_cast<byte *>(&compressedDataBuffer[0]), curSavegameInfo.gamedataLength)) {
			error("Unable to decompress the gamedata from savegame file \"%s\".", filename.c_str());
			delete[] uncompressedDataBuffer;
			delete[] compressedDataBuffer;
			delete file;
			return false;
		}
	} else {
		// Newer saved game with uncompressed game data, copy it as-is.
		memcpy(uncompressedDataBuffer, compressedDataBuffer, uncompressedBufferSize);
	}

	InputPersistenceBlock reader(&uncompressedDataBuffer[0], curSavegameInfo.gamedataUncompressedLength, curSavegameInfo.version);

	// Einzelne Engine-Module depersistieren.
	bool success = true;
	success &= Kernel::getInstance()->getScript()->unpersist(reader);
	// Muss unbedingt nach Script passieren. Da sonst die bereits wiederhergestellten Regions per Garbage-Collection gekillt werden.
	success &= RegionRegistry::instance().unpersist(reader);
	success &= Kernel::getInstance()->getGfx()->unpersist(reader);
	success &= Kernel::getInstance()->getSfx()->unpersist(reader);
	success &= Kernel::getInstance()->getInput()->unpersist(reader);

	delete[] compressedDataBuffer;
	delete[] uncompressedDataBuffer;
	delete file;

	if (!success) {
		error("Unable to unpersist the gamedata from savegame file \"%s\".", filename.c_str());
		return false;
	}

	return true;
}

} // End of namespace Sword25
