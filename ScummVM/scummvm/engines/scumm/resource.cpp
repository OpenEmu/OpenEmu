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

#include "common/str.h"
#ifndef MACOSX
#include "common/config-manager.h"
#endif

#include "scumm/charset.h"
#include "scumm/dialogs.h"
#include "scumm/file.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/scumm_v5.h"
#include "scumm/scumm_v8.h"
#include "scumm/sound.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

namespace Scumm {

enum {
	RF_LOCK = 0x80,
	RF_USAGE = 0x7F,
	RF_USAGE_MAX = RF_USAGE,

	RS_MODIFIED = 0x10,
	RF_OFFHEAP = 0x40
};



extern const char *nameOfResType(ResType type);

static uint16 newTag2Old(uint32 newTag);
static const byte *findResourceSmall(uint32 tag, const byte *searchin);

static bool checkTryMedia(BaseScummFile *handle);


/* Open a room */
void ScummEngine::openRoom(const int room) {
	bool result;
	byte encByte = 0;

	debugC(DEBUG_GENERAL, "openRoom(%d)", room);
	assert(room >= 0);

	/* Don't load the same room again */
	if (_lastLoadedRoom == room)
		return;
	_lastLoadedRoom = room;

	/* Room -1 means close file */
	if (room == -1) {
		deleteRoomOffsets();
		_fileHandle->close();
		return;
	}

	// Load the disk numer / room offs (special case for room 0 exists because
	// room 0 contains the data which is used to create the roomno / roomoffs
	// tables -- hence obviously we mustn't use those when loading room 0.
	const uint32 diskNumber = room ? _res->_types[rtRoom][room]._roomno : 0;
	const uint32 room_offs = room ? _res->_types[rtRoom][room]._roomoffs : 0;

	// FIXME: Since room_offs is const, clearly the following loop either
	// is never entered, or loops forever (if it wasn't for the return/error
	// statements in it, that is). -> This should be cleaned up!
	// Maybe we should re-enabled the looping properly, to deal with disc
	// changes in COMI ?
	while (room_offs != RES_INVALID_OFFSET) {

		if (room_offs != 0 && room != 0 && _game.heversion < 98) {
			_fileOffset = _res->_types[rtRoom][room]._roomoffs;
			return;
		}

		Common::String filename(generateFilename(room));

		// Determine the encryption, if any.
		if (_game.features & GF_USE_KEY) {
			if (_game.version <= 3)
				encByte = 0xFF;
			else if ((_game.version == 4) && (room == 0 || room >= 900))
				encByte = 0;
			else
				encByte = 0x69;
		} else
			encByte = 0;

		if (room > 0 && (_game.version == 8))
			VAR(VAR_CURRENTDISK) = diskNumber;

		// Try to open the file
		result = openResourceFile(filename, encByte);

		if (result) {
			if (room == 0)
				return;
			deleteRoomOffsets();
			readRoomsOffsets();
			_fileOffset = _res->_types[rtRoom][room]._roomoffs;

			if (_fileOffset != 8)
				return;

			error("Room %d not in %s", room, filename.c_str());
			return;
		}
		askForDisk(filename.c_str(), diskNumber);
	}

	do {
		char buf[16];
		snprintf(buf, sizeof(buf), "%.3d.lfl", room);
		encByte = 0;
		if (openResourceFile(buf, encByte))
			break;
		askForDisk(buf, diskNumber);
	} while (1);

	deleteRoomOffsets();
	_fileOffset = 0;		// start of file
}

void ScummEngine::closeRoom() {
	if (_lastLoadedRoom != -1) {
		_lastLoadedRoom = -1;
		deleteRoomOffsets();
		_fileHandle->close();
	}
}

/** Delete the currently loaded room offsets. */
void ScummEngine::deleteRoomOffsets() {
	for (int i = 0; i < _numRooms; i++) {
		if (_res->_types[rtRoom][i]._roomoffs != RES_INVALID_OFFSET)
			_res->_types[rtRoom][i]._roomoffs = 0;
	}
}

/** Read room offsets */
void ScummEngine::readRoomsOffsets() {
	if (_game.features & GF_SMALL_HEADER) {
		_fileHandle->seek(12, SEEK_SET);	// Directly searching for the room offset block would be more generic...
	} else {
		_fileHandle->seek(16, SEEK_SET);
	}

	int num = _fileHandle->readByte();
	while (num--) {
		int room = _fileHandle->readByte();
		int offset =  _fileHandle->readUint32LE();
		if (_res->_types[rtRoom][room]._roomoffs != RES_INVALID_OFFSET) {
			_res->_types[rtRoom][room]._roomoffs = offset;
		}
	}
}

bool ScummEngine::openFile(BaseScummFile &file, const Common::String &filename, bool resourceFile) {
	bool result = false;

	if (!_containerFile.empty()) {
		file.close();
		file.open(_containerFile);
		assert(file.isOpen());

		result = file.openSubFile(filename);
	}

	if (!result) {
		file.close();
		result = file.open(filename);
	}

	return result;
}

bool ScummEngine::openResourceFile(const Common::String &filename, byte encByte) {
	debugC(DEBUG_GENERAL, "openResourceFile(%s)", filename.c_str());

	if (openFile(*_fileHandle, filename, true)) {
		_fileHandle->setEnc(encByte);
		return true;
	}
	return false;
}

void ScummEngine::askForDisk(const char *filename, int disknum) {
	char buf[128];

	if (_game.version == 8) {
#ifdef ENABLE_SCUMM_7_8
		char result;

		_imuseDigital->stopAllSounds();

#ifdef MACOSX
		sprintf(buf, "Cannot find file: '%s'\nPlease insert disc %d.\nPress OK to retry, Quit to exit", filename, disknum);
#else
		sprintf(buf, "Cannot find file: '%s'\nInsert disc %d into drive %s\nPress OK to retry, Quit to exit", filename, disknum, ConfMan.get("path").c_str());
#endif

		result = displayMessage("Quit", "%s", buf);
		if (!result) {
			error("Cannot find file: '%s'", filename);
		}
#endif
	} else {
		sprintf(buf, "Cannot find file: '%s'", filename);
		InfoDialog dialog(this, (char *)buf);
		runDialog(dialog);
		error("Cannot find file: '%s'", filename);
	}
}

void ScummEngine::readIndexFile() {
	uint32 blocktype, itemsize;
	int numblock = 0;

	debugC(DEBUG_GENERAL, "readIndexFile()");

	closeRoom();
	openRoom(0);

	if (_game.version <= 5) {
		// Figure out the sizes of various resources
		while (true) {
			blocktype = _fileHandle->readUint32BE();
			itemsize = _fileHandle->readUint32BE();
			if (_fileHandle->eos() || _fileHandle->err())
				break;
			switch (blocktype) {
			case MKTAG('D','O','B','J'):
				_numGlobalObjects = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;
			case MKTAG('D','R','O','O'):
				_numRooms = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;

			case MKTAG('D','S','C','R'):
				_numScripts = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;

			case MKTAG('D','C','O','S'):
				_numCostumes = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;

			case MKTAG('D','S','O','U'):
				_numSounds = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;
			}
			_fileHandle->seek(itemsize - 8, SEEK_CUR);
		}
		_fileHandle->seek(0, SEEK_SET);
	}

	if (checkTryMedia(_fileHandle)) {
		displayMessage(NULL, "You're trying to run game encrypted by ActiveMark. This is not supported.");
		quitGame();

		return;
	}

	while (true) {
		blocktype = _fileHandle->readUint32BE();
		itemsize = _fileHandle->readUint32BE();

		if (_fileHandle->eos() || _fileHandle->err())
			break;

		numblock++;
		debug(2, "Reading index block of type '%s', size %d", tag2str(blocktype), itemsize);
		readIndexBlock(blocktype, itemsize);
	}

//  if (numblock!=9)
//    error("Not enough blocks read from directory");

	closeRoom();
}



#define TRYMEDIA_MARK_LEN 6

bool checkTryMedia(BaseScummFile *handle) {
	byte buf[TRYMEDIA_MARK_LEN];
	bool matched = true;
	const byte magic[2][TRYMEDIA_MARK_LEN] =
		{{ 0x00,  'T', 'M', 'S', 'A', 'M' },
		 { 'i',   '=', '$', ':', '(', '$' }};  // Same but 0x69 xored

	handle->read(buf, TRYMEDIA_MARK_LEN);

	for (int i = 0; i < 2; i++) {
		matched = true;
		for (int j = 0; j < TRYMEDIA_MARK_LEN; j++)
			if (buf[j] != magic[i][j]) {
				matched = false;
				break;
			}

		if (matched)
			break;
	}

	if (matched)
		return true;

	handle->seek(0, SEEK_SET);

	return false;
}


#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::readIndexBlock(uint32 blocktype, uint32 itemsize) {
	int num;
	char *ptr;
	switch (blocktype) {
	case MKTAG('A','N','A','M'):		// Used by: The Dig, FT
		num = _fileHandle->readUint16LE();
		ptr = (char *)malloc(num * 9);
		_fileHandle->read(ptr, num * 9);
		_imuseDigital->setAudioNames(num, ptr);
		break;

	case MKTAG('D','R','S','C'):		// Used by: COMI
		readResTypeList(rtRoomScripts);
		break;

	default:
		ScummEngine::readIndexBlock(blocktype, itemsize);
	}
}
#endif

void ScummEngine_v70he::readIndexBlock(uint32 blocktype, uint32 itemsize) {
	int i;
	switch (blocktype) {
	case MKTAG('D','I','R','I'):
		readResTypeList(rtRoomImage);
		break;

	case MKTAG('D','I','R','M'):
		readResTypeList(rtImage);
		break;

	case MKTAG('D','I','R','T'):
		readResTypeList(rtTalkie);
		break;

	case MKTAG('D','L','F','L'):
		i = _fileHandle->readUint16LE();
		_fileHandle->seek(-2, SEEK_CUR);
		_heV7RoomOffsets = (byte *)calloc(2 + (i * 4), 1);
		_fileHandle->read(_heV7RoomOffsets, (2 + (i * 4)) );
		break;

	case MKTAG('D','I','S','K'):
		i = _fileHandle->readUint16LE();
		_heV7DiskOffsets = (byte *)calloc(i, 1);
		_fileHandle->read(_heV7DiskOffsets, i);
		break;

	case MKTAG('S','V','E','R'):
		// Index version number
		_fileHandle->seek(itemsize - 8, SEEK_CUR);
		break;

	case MKTAG('I','N','I','B'):
		_fileHandle->seek(itemsize - 8, SEEK_CUR);
		debug(2, "INIB index block not yet handled, skipping");
		break;

	default:
		ScummEngine::readIndexBlock(blocktype, itemsize);
	}
}

void ScummEngine::readIndexBlock(uint32 blocktype, uint32 itemsize) {
	int i;
	switch (blocktype) {
	case MKTAG('D','C','H','R'):
	case MKTAG('D','I','R','F'):
		readResTypeList(rtCharset);
		break;

	case MKTAG('D','O','B','J'):
		readGlobalObjects();
		break;

	case MKTAG('R','N','A','M'):
		// Names of rooms. Maybe we should put them into a table, for use by the debugger?
		if (_game.heversion >= 80) {
			for (int room; (room = _fileHandle->readUint16LE()); ) {
				char buf[100];
				i = 0;
				for (byte s; (s = _fileHandle->readByte()) && i < ARRAYSIZE(buf) - 1; ) {
					buf[i++] = s;
				}
				buf[i] = 0;
				debug(5, "Room %d: '%s'", room, buf);
			}
		} else {
			for (int room; (room = _fileHandle->readByte()); ) {
				char buf[10];
				_fileHandle->read(buf, 9);
				buf[9] = 0;
				for (i = 0; i < 9; i++)
					buf[i] ^= 0xFF;
				debug(5, "Room %d: '%s'", room, buf);
			}
		}
		break;

	case MKTAG('D','R','O','O'):
	case MKTAG('D','I','R','R'):
		readResTypeList(rtRoom);
		break;

	case MKTAG('D','S','C','R'):
	case MKTAG('D','I','R','S'):
		readResTypeList(rtScript);
		break;

	case MKTAG('D','C','O','S'):
	case MKTAG('D','I','R','C'):
		readResTypeList(rtCostume);
		break;

	case MKTAG('M','A','X','S'):
		readMAXS(itemsize);
		allocateArrays();
		break;

	case MKTAG('D','I','R','N'):
	case MKTAG('D','S','O','U'):
		readResTypeList(rtSound);
		break;

	case MKTAG('A','A','R','Y'):
		readArrayFromIndexFile();
		break;

	default:
		error("Bad ID %04X('%s') found in index file directory", blocktype,
				tag2str(blocktype));
	}
}

void ScummEngine::readArrayFromIndexFile() {
	error("readArrayFromIndexFile() not supported in pre-V6 games");
}

int ScummEngine::readResTypeList(ResType type) {
	uint num;
	ResId idx;

	if (_game.version == 8)
		num = _fileHandle->readUint32LE();
	else
		num = _fileHandle->readUint16LE();

	if (num != _res->_types[type].size()) {
		error("Invalid number of %ss (%d) in directory", nameOfResType(type), num);
	}

	debug(2, "  readResTypeList(%s): %d entries", nameOfResType(type), num);


	for (idx = 0; idx < num; idx++) {
		_res->_types[type][idx]._roomno = _fileHandle->readByte();
	}
	for (idx = 0; idx < num; idx++) {
		_res->_types[type][idx]._roomoffs = _fileHandle->readUint32LE();
	}

	return num;
}

int ScummEngine_v70he::readResTypeList(ResType type) {
	uint num;
	ResId idx;

	num = ScummEngine::readResTypeList(type);

	if (type == rtRoom)
		for (idx = 0; idx < num; idx++) {
			_heV7RoomIntOffsets[idx] = _res->_types[rtRoom][idx]._roomoffs;
		}

	for (idx = 0; idx < num; idx++) {
		// The globsize is currently not being used
		/*_res->_types[type][idx]._globsize =*/ _fileHandle->readUint32LE();
	}

	return num;
}

void ResourceManager::allocResTypeData(ResType type, uint32 tag, int num, ResTypeMode mode) {
	debug(2, "allocResTypeData(%s,%s,%d,%d)", nameOfResType(type), tag2str(TO_BE_32(tag)), num, mode);
	assert(type >= 0 && type < (int)(ARRAYSIZE(_types)));

	if (num >= 8000)
		error("Too many %s resources (%d) in directory", nameOfResType(type), num);

	_types[type]._mode = mode;
	_types[type]._tag = tag;

	// If there was data in there, let's clear it out completely. This is important
	// in case we are restarting the game.
	_types[type].clear();
	_types[type].resize(num);

/*
	TODO: Use multiple Resource subclasses, one for each res mode; then,
	given them serializability.
	if (mode) {
		_types[type].roomno = (byte *)calloc(num, sizeof(byte));
		_types[type].roomoffs = (uint32 *)calloc(num, sizeof(uint32));
	}

	if (_vm->_game.heversion >= 70) {
		_types[type].globsize = (uint32 *)calloc(num, sizeof(uint32));
	}
*/
}

void ScummEngine::loadCharset(int no) {
	int i;
	byte *ptr;

	debugC(DEBUG_GENERAL, "loadCharset(%d)", no);

	/* FIXME - hack around crash in Indy4 (occurs if you try to load after dieing) */
	if (_game.id == GID_INDY4 && no == 0)
		no = 1;

	/* for Humongous catalogs */
	if (_game.heversion >= 70 && _numCharsets == 1) {
		debug(0, "Not loading charset as it doesn't seem to exist?");
		return;
	}

	assert(no < (int)sizeof(_charsetData) / 16);
	assertRange(1, no, _numCharsets - 1, "charset");

	ptr = getResourceAddress(rtCharset, no);

	for (i = 0; i < 15; i++) {
		_charsetData[no][i + 1] = ptr[i + 14];
	}
}

void ScummEngine::nukeCharset(int i) {
	assertRange(1, i, _numCharsets - 1, "charset");
	_res->nukeResource(rtCharset, i);
}

void ScummEngine::ensureResourceLoaded(ResType type, ResId idx) {
	debugC(DEBUG_RESOURCE, "ensureResourceLoaded(%s,%d)", nameOfResType(type), idx);

	if ((type == rtRoom) && idx > 0x7F && _game.version < 7 && _game.heversion <= 71) {
		idx = _resourceMapper[idx & 0x7F];
	}

	// FIXME: This check used to be "idx==0". However, that causes
	// problems when using this function to ensure charset 0 is loaded.
	// This is done for many games, e.g. Zak256 or Indy3 (EGA and VGA).
	// For now we restrict the check to anything which is not a charset.
	// Question: Why was this check like that in the first place?
	// Answer: costumes with an index of zero in the newer games at least.
	// TODO: determine why the heck anything would try to load a costume
	// with id 0. Is that "normal", or is it caused by yet another bug in
	// our code base? After all we also have to add special cases for many
	// of our script opcodes that check for the (invalid) actor 0... so
	// maybe both issues are related...
	if (type != rtCharset && idx == 0)
		return;

	if (idx <= _res->_types[type].size() && _res->_types[type][idx]._address)
		return;

	loadResource(type, idx);

	if (_game.version == 5 && type == rtRoom && (int)idx == _roomResource)
		VAR(VAR_ROOM_FLAG) = 1;
}

int ScummEngine::loadResource(ResType type, ResId idx) {
	int roomNr;
	uint32 fileOffs;
	uint32 size, tag;

	debugC(DEBUG_RESOURCE, "loadResource(%s,%d)", nameOfResType(type), idx);

	if (type == rtCharset && (_game.features & GF_SMALL_HEADER)) {
		loadCharset(idx);
		return 1;
	}

	roomNr = getResourceRoomNr(type, idx);

	if (idx >= _res->_types[type].size())
		error("%s %d undefined %d %d", nameOfResType(type), idx, _res->_types[type].size(), roomNr);

	if (roomNr == 0)
		roomNr = _roomResource;

	fileOffs = getResourceRoomOffset(type, idx);
	if (fileOffs == RES_INVALID_OFFSET)
		return 0;

	openRoom(roomNr);

	_fileHandle->seek(fileOffs + _fileOffset, SEEK_SET);

	if (_game.features & GF_OLD_BUNDLE) {
		if ((_game.version == 3) && !(_game.platform == Common::kPlatformAmiga) && (type == rtSound)) {
			return readSoundResourceSmallHeader(idx);
		} else {
			size = _fileHandle->readUint16LE();
			_fileHandle->seek(-2, SEEK_CUR);
		}
	} else if (_game.features & GF_SMALL_HEADER) {
		if (_game.version == 4)
			_fileHandle->seek(8, SEEK_CUR);
		size = _fileHandle->readUint32LE();
		tag = _fileHandle->readUint16LE();
		_fileHandle->seek(-6, SEEK_CUR);
		if ((type == rtSound) && !(_game.platform == Common::kPlatformAmiga) && !(_game.platform == Common::kPlatformFMTowns)) {
			return readSoundResourceSmallHeader(idx);
		}
	} else {
		if (type == rtSound) {
			return readSoundResource(idx);
		}

		// Sanity check: Is this the right tag for this resource type?
		//
		// Currently disabled for newer HE games because they use different
		// tags. For example, for rtRoom, 'ROOM' changed to 'RMDA'; and for
		// rtImage, 'AWIZ' and 'MULT' can both occur simultaneously.
		// On the long run, it would be preferable to not turn this check off,
		// but instead to explicitly support the variations in the HE games.
		tag = _fileHandle->readUint32BE();
		if (tag != _res->_types[type]._tag && _game.heversion < 70) {
			error("Unknown res tag '%s' encountered (expected '%s') "
			        "while trying to load res (%s,%d) in room %d at %d+%d in file %s",
			        tag2str(tag), tag2str(_res->_types[type]._tag),
					nameOfResType(type), idx, roomNr,
					_fileOffset, fileOffs, _fileHandle->getName());
		}

		size = _fileHandle->readUint32BE();
		_fileHandle->seek(-8, SEEK_CUR);
	}
	_fileHandle->read(_res->createResource(type, idx, size), size);

	// dump the resource if requested
	if (_dumpScripts && type == rtScript) {
		dumpResource("script-", idx, getResourceAddress(rtScript, idx));
	}

	if (_fileHandle->err() || _fileHandle->eos()) {
		error("Cannot read resource");
	}

	return 1;
}

int ScummEngine::getResourceRoomNr(ResType type, ResId idx) {
	if (type == rtRoom && _game.heversion < 70)
		return idx;
	return _res->_types[type][idx]._roomno;
}

uint32 ScummEngine::getResourceRoomOffset(ResType type, ResId idx) {
	if (type == rtRoom) {
		return (_game.version == 8) ? 8 : 0;
	}
	return _res->_types[type][idx]._roomoffs;
}

uint32 ScummEngine_v70he::getResourceRoomOffset(ResType type, ResId idx) {
	if (type == rtRoom) {
		return _heV7RoomIntOffsets[idx];
	}
	return _res->_types[type][idx]._roomoffs;
}

int ScummEngine::getResourceSize(ResType type, ResId idx) {
	byte *ptr = getResourceAddress(type, idx);
	assert(ptr);
	return _res->_types[type][idx]._size;
}

byte *ScummEngine::getResourceAddress(ResType type, ResId idx) {
	byte *ptr;

	if (_game.heversion >= 80 && type == rtString)
		idx &= ~0x33539000;

	if (!_res->validateResource("getResourceAddress", type, idx))
		return NULL;

	// If the resource is missing, but loadable from the game data files, try to do so.
	if (!_res->_types[type][idx]._address && _res->_types[type]._mode != kDynamicResTypeMode) {
		ensureResourceLoaded(type, idx);
	}

	ptr = (byte *)_res->_types[type][idx]._address;
	if (!ptr) {
		debugC(DEBUG_RESOURCE, "getResourceAddress(%s,%d) == NULL", nameOfResType(type), idx);
		return NULL;
	}

	_res->setResourceCounter(type, idx, 1);

	debugC(DEBUG_RESOURCE, "getResourceAddress(%s,%d) == %p", nameOfResType(type), idx, ptr);
	return ptr;
}

byte *ScummEngine::getStringAddress(ResId idx) {
	byte *addr = getResourceAddress(rtString, idx);
	return addr;
}

byte *ScummEngine_v6::getStringAddress(ResId idx) {
	byte *addr = getResourceAddress(rtString, idx);
	if (addr == NULL)
		return NULL;
	// Skip over the ArrayHeader
	return addr + 6;
}

byte *ScummEngine::getStringAddressVar(int i) {
	return getStringAddress(_scummVars[i]);
}

void ResourceManager::increaseExpireCounter() {
	++_expireCounter;
	if (_expireCounter == 0) {	// overflow?
		increaseResourceCounters();
	}
}

void ResourceManager::increaseResourceCounters() {
	for (ResType type = rtFirst; type <= rtLast; type = ResType(type + 1)) {
		ResId idx = _types[type].size();
		while (idx-- > 0) {
			byte counter = _types[type][idx].getResourceCounter();
			if (counter && counter < RF_USAGE_MAX) {
				setResourceCounter(type, idx, counter + 1);
			}
		}
	}
}

void ResourceManager::setResourceCounter(ResType type, ResId idx, byte counter) {
	_types[type][idx].setResourceCounter(counter);
}

void ResourceManager::Resource::setResourceCounter(byte counter) {
	_flags &= RF_LOCK;	// Clear lower 7 bits, preserve the lock bit.
	_flags |= counter;	// Update the usage counter
}

byte ResourceManager::Resource::getResourceCounter() const {
	return _flags & RF_USAGE;
}

/* 2 bytes safety area to make "precaching" of bytes in the gdi drawer easier */
#define SAFETY_AREA 2

byte *ResourceManager::createResource(ResType type, ResId idx, uint32 size) {
	debugC(DEBUG_RESOURCE, "_res->createResource(%s,%d,%d)", nameOfResType(type), idx, size);

	if (!validateResource("allocating", type, idx))
		return NULL;

	if (_vm->_game.version <= 2) {
		// Nuking and reloading a resource can be harmful in some
		// cases. For instance, Zak tries to reload the intro music
		// while it's playing. See bug #1253171.

		if (_types[type][idx]._address && (type == rtSound || type == rtScript || type == rtCostume))
			return _types[type][idx]._address;
	}

	nukeResource(type, idx);

	expireResources(size);

	byte *ptr = new byte[size + SAFETY_AREA];
	if (ptr == NULL) {
		error("createResource(%s,%d): Out of memory while allocating %d", nameOfResType(type), idx, size);
	}

	memset(ptr, 0, size + SAFETY_AREA);
	_allocatedSize += size;

	_types[type][idx]._address = ptr;
	_types[type][idx]._size = size;
	setResourceCounter(type, idx, 1);
	return ptr;
}

ResourceManager::Resource::Resource() {
	_address = 0;
	_size = 0;
	_flags = 0;
	_status = 0;
	_roomno = 0;
	_roomoffs = 0;
}

ResourceManager::Resource::~Resource() {
	delete[] _address;
	_address = 0;
}

void ResourceManager::Resource::nuke() {
	delete[] _address;
	_address = 0;
	_size = 0;
	_flags = 0;
	_status &= ~RS_MODIFIED;
}

ResourceManager::ResTypeData::ResTypeData() {
	_mode = kDynamicResTypeMode;
	_tag = 0;
}

ResourceManager::ResTypeData::~ResTypeData() {
}

ResourceManager::ResourceManager(ScummEngine *vm) : _vm(vm) {
	_allocatedSize = 0;
	_maxHeapThreshold = 0;
	_minHeapThreshold = 0;
	_expireCounter = 0;
}

ResourceManager::~ResourceManager() {
	freeResources();
}

void ResourceManager::setHeapThreshold(int min, int max) {
	assert(0 < max);
	assert(min <= max);
	_maxHeapThreshold = max;
	_minHeapThreshold = min;
}

bool ResourceManager::validateResource(const char *str, ResType type, ResId idx) const {
	if (type < rtFirst || type > rtLast || (uint)idx >= (uint)_types[type].size()) {
		error("%s Illegal Glob type %s (%d) num %d", str, nameOfResType(type), type, idx);
		return false;
	}
	return true;
}

void ResourceManager::nukeResource(ResType type, ResId idx) {
	byte *ptr = _types[type][idx]._address;
	if (ptr != NULL) {
		debugC(DEBUG_RESOURCE, "nukeResource(%s,%d)", nameOfResType(type), idx);
		_allocatedSize -= _types[type][idx]._size;
		_types[type][idx].nuke();
	}
}

const byte *ScummEngine::findResourceData(uint32 tag, const byte *ptr) {
	if (_game.features & GF_OLD_BUNDLE)
		error("findResourceData must not be used in GF_OLD_BUNDLE games");
	else if (_game.features & GF_SMALL_HEADER)
		ptr = findResourceSmall(tag, ptr);
	else
		ptr = findResource(tag, ptr);

	if (ptr == NULL)
		return NULL;
	return ptr + _resourceHeaderSize;
}

int ScummEngine::getResourceDataSize(const byte *ptr) const {
	if (ptr == NULL)
		return 0;

	if (_game.features & GF_OLD_BUNDLE)
		return READ_LE_UINT16(ptr) - _resourceHeaderSize;
	else if (_game.features & GF_SMALL_HEADER)
		return READ_LE_UINT32(ptr) - _resourceHeaderSize;
	else
		return READ_BE_UINT32(ptr - 4) - _resourceHeaderSize;
}

void ResourceManager::lock(ResType type, ResId idx) {
	if (!validateResource("Locking", type, idx))
		return;
	_types[type][idx].lock();
}

void ResourceManager::unlock(ResType type, ResId idx) {
	if (!validateResource("Unlocking", type, idx))
		return;
	_types[type][idx].unlock();
}

bool ResourceManager::isLocked(ResType type, ResId idx) const {
	if (!validateResource("isLocked", type, idx))
		return false;
	return _types[type][idx].isLocked();
}

void ResourceManager::Resource::lock() {
	_flags |= RF_LOCK;
}

void ResourceManager::Resource::unlock() {
	_flags &= ~RF_LOCK;
}

bool ResourceManager::Resource::isLocked() const {
	return (_flags & RF_LOCK) != 0;
}

bool ScummEngine::isResourceInUse(ResType type, ResId idx) const {
	if (!_res->validateResource("isResourceInUse", type, idx))
		return false;
	switch (type) {
	case rtRoom:
		return _roomResource == (byte)idx;
	case rtRoomImage:
		return _roomResource == (byte)idx;
	case rtRoomScripts:
		return _roomResource == (byte)idx;
	case rtScript:
		return isScriptInUse(idx);
	case rtCostume:
		return isCostumeInUse(idx);
	case rtSound:
		// Sound resource 1 is used for queued speech
		if (_game.heversion >= 60 && idx == 1)
			return true;
		else
			return _sound->isSoundInUse(idx);
	case rtCharset:
		return _charset->getCurID() == (int)idx;
	case rtImage:
		return _res->isModified(type, idx) != 0;
	case rtSpoolBuffer:
		return _sound->isSoundRunning(10000 + idx) != 0;
	default:
		return false;
	}
}

void ResourceManager::setModified(ResType type, ResId idx) {
	if (!validateResource("Modified", type, idx))
		return;
	_types[type][idx].setModified();
}

void ResourceManager::setOffHeap(ResType type, ResId idx) {
	if (!validateResource("setOffHeap", type, idx))
		return;
	_types[type][idx].setOffHeap();
}

void ResourceManager::setOnHeap(ResType type, ResId idx) {
	if (!validateResource("setOnHeap", type, idx))
		return;
	_types[type][idx].setOnHeap();
}

bool ResourceManager::isModified(ResType type, ResId idx) const {
	if (!validateResource("isModified", type, idx))
		return false;
	return _types[type][idx].isModified();
}

bool ResourceManager::Resource::isModified() const {
	return (_status & RS_MODIFIED) != 0;
}

bool ResourceManager::Resource::isOffHeap() const {
	return (_status & RF_OFFHEAP) != 0;
}

void ResourceManager::Resource::setModified() {
	_status |= RS_MODIFIED;
}

void ResourceManager::Resource::setOffHeap() {
	_status |= RF_OFFHEAP;
}

void ResourceManager::Resource::setOnHeap() {
	_status &= ~RF_OFFHEAP;
}

void ResourceManager::expireResources(uint32 size) {
	byte best_counter;
	ResType best_type;
	int best_res = 0;
	uint32 oldAllocatedSize;

	if (_expireCounter != 0xFF) {
		_expireCounter = 0xFF;
		increaseResourceCounters();
	}

	if (size + _allocatedSize < _maxHeapThreshold)
		return;

	oldAllocatedSize = _allocatedSize;

	do {
		best_type = rtInvalid;
		best_counter = 2;

		for (ResType type = rtFirst; type <= rtLast; type = ResType(type + 1)) {
			if (_types[type]._mode != kDynamicResTypeMode) {
				// Resources of this type can be reloaded from the data files,
				// so we can potentially unload them to free memory.
				ResId idx = _types[type].size();
				while (idx-- > 0) {
					Resource &tmp = _types[type][idx];
					byte counter = tmp.getResourceCounter();
					if (!tmp.isLocked() && counter >= best_counter && tmp._address && !_vm->isResourceInUse(type, idx) && !tmp.isOffHeap()) {
						best_counter = counter;
						best_type = type;
						best_res = idx;
					}
				}
			}
		}

		if (!best_type)
			break;
		nukeResource(best_type, best_res);
	} while (size + _allocatedSize > _minHeapThreshold);

	increaseResourceCounters();

	debugC(DEBUG_RESOURCE, "Expired resources, mem %d -> %d", oldAllocatedSize, _allocatedSize);
}

void ResourceManager::freeResources() {
	for (ResType type = rtFirst; type <= rtLast; type = ResType(type + 1)) {
		ResId idx = _types[type].size();
		while (idx-- > 0) {
			if (isResourceLoaded(type, idx))
				nukeResource(type, idx);
		}
		_types[type].clear();
	}
}

void ScummEngine::loadPtrToResource(ResType type, ResId idx, const byte *source) {
	byte *alloced;
	int len;

	_res->nukeResource(type, idx);

	len = resStrLen(source) + 1;
	if (len <= 0)
		return;

	alloced = _res->createResource(type, idx, len);

	if (!source) {
		// Need to refresh the script pointer, since createResource may
		// have caused the script resource to expire.
		refreshScriptPointer();
		memcpy(alloced, _scriptPointer, len);
		_scriptPointer += len;
	} else {
		memcpy(alloced, source, len);
	}
}

bool ResourceManager::isResourceLoaded(ResType type, ResId idx) const {
	if (!validateResource("isResourceLoaded", type, idx))
		return false;
	return _types[type][idx]._address != NULL;
}

void ResourceManager::resourceStats() {
	uint32 lockedSize = 0, lockedNum = 0;

	for (ResType type = rtFirst; type <= rtLast; type = ResType(type + 1)) {
		ResId idx = _types[type].size();
		while (idx-- > 0) {
			Resource &tmp = _types[type][idx];
			if (tmp.isLocked() && tmp._address) {
				lockedSize += tmp._size;
				lockedNum++;
			}
		}
	}

	debug(1, "Total allocated size=%d, locked=%d(%d)", _allocatedSize, lockedSize, lockedNum);
}

void ScummEngine_v5::readMAXS(int blockSize) {
	_numVariables = _fileHandle->readUint16LE();      // 800
	_fileHandle->readUint16LE();                      // 16
	_numBitVariables = _fileHandle->readUint16LE();   // 2048
	_numLocalObjects = _fileHandle->readUint16LE();   // 200
	_numArray = 50;
	_numVerbs = 100;
	// Used to be 50, which wasn't enough for MI2 and FOA. See bugs
	// #933610, #936323 and #941275.
	_numNewNames = 150;
	_objectRoomTable = NULL;

	_fileHandle->readUint16LE();                      // 50
	_numCharsets = _fileHandle->readUint16LE();       // 9
	_fileHandle->readUint16LE();                      // 100
	_fileHandle->readUint16LE();                      // 50
	_numInventory = _fileHandle->readUint16LE();      // 80
	_numGlobalScripts = 200;

	_shadowPaletteSize = 256;

	_numFlObject = 50;

	if (_shadowPaletteSize)
		_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v8::readMAXS(int blockSize) {
	_fileHandle->seek(50, SEEK_CUR);                 // Skip over SCUMM engine version
	_fileHandle->seek(50, SEEK_CUR);                 // Skip over data file version
	_numVariables = _fileHandle->readUint32LE();     // 1500
	_numBitVariables = _fileHandle->readUint32LE();  // 2048
	_fileHandle->readUint32LE();                     // 40
	_numScripts = _fileHandle->readUint32LE();       // 458
	_numSounds = _fileHandle->readUint32LE();        // 789
	_numCharsets = _fileHandle->readUint32LE();      // 1
	_numCostumes = _fileHandle->readUint32LE();      // 446
	_numRooms = _fileHandle->readUint32LE();         // 95
	_fileHandle->readUint32LE();                     // 80
	_numGlobalObjects = _fileHandle->readUint32LE(); // 1401
	_fileHandle->readUint32LE();                     // 60
	_numLocalObjects = _fileHandle->readUint32LE();  // 200
	_numNewNames = _fileHandle->readUint32LE();      // 100
	_numFlObject = _fileHandle->readUint32LE();      // 128
	_numInventory = _fileHandle->readUint32LE();     // 80
	_numArray = _fileHandle->readUint32LE();         // 200
	_numVerbs = _fileHandle->readUint32LE();         // 50

	_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
	_numGlobalScripts = 2000;

	_shadowPaletteSize = NUM_SHADOW_PALETTE * 256;
	_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);
}

void ScummEngine_v7::readMAXS(int blockSize) {
	_fileHandle->seek(50, SEEK_CUR);                 // Skip over SCUMM engine version
	_fileHandle->seek(50, SEEK_CUR);                 // Skip over data file version
	_numVariables = _fileHandle->readUint16LE();
	_numBitVariables = _fileHandle->readUint16LE();
	_fileHandle->readUint16LE();
	_numGlobalObjects = _fileHandle->readUint16LE();
	_numLocalObjects = _fileHandle->readUint16LE();
	_numNewNames = _fileHandle->readUint16LE();
	_numVerbs = _fileHandle->readUint16LE();
	_numFlObject = _fileHandle->readUint16LE();
	_numInventory = _fileHandle->readUint16LE();
	_numArray = _fileHandle->readUint16LE();
	_numRooms = _fileHandle->readUint16LE();
	_numScripts = _fileHandle->readUint16LE();
	_numSounds = _fileHandle->readUint16LE();
	_numCharsets = _fileHandle->readUint16LE();
	_numCostumes = _fileHandle->readUint16LE();

	_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);

	if ((_game.id == GID_FT) && (_game.features & GF_DEMO) &&
		(_game.platform == Common::kPlatformPC))
		_numGlobalScripts = 300;
	else
		_numGlobalScripts = 2000;

	_shadowPaletteSize = NUM_SHADOW_PALETTE * 256;
	_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);
}
#endif

void ScummEngine_v6::readMAXS(int blockSize) {
	if (blockSize == 38) {
		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numBitVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numVerbs = _fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numNewNames = 50;

		_objectRoomTable = NULL;
		_numGlobalScripts = 200;

		if (_game.heversion >= 70) {
			_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		}

		if (_game.heversion <= 70) {
			_shadowPaletteSize = 256;
			_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);
		}
	} else
		error("readMAXS(%d) failed to read MAXS data", blockSize);
}

void ScummEngine::readGlobalObjects() {
	int i;
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);
	assert(_objectStateTable);
	assert(_objectOwnerTable);

	_fileHandle->read(_objectOwnerTable, num);
	for (i = 0; i < num; i++) {
		_objectStateTable[i] = _objectOwnerTable[i] >> OF_STATE_SHL;
		_objectOwnerTable[i] &= OF_OWNER_MASK;
	}

	_fileHandle->read(_classData, num * sizeof(uint32));

#if defined(SCUMM_BIG_ENDIAN)
	// Correct the endianess if necessary
	for (i = 0; i != num; i++)
		_classData[i] = FROM_LE_32(_classData[i]);
#endif
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v8::readGlobalObjects() {
	int i;
	int num = _fileHandle->readUint32LE();
	assert(num == _numGlobalObjects);
	assert(_objectStateTable);
	assert(_objectOwnerTable);

	_objectIDMap = new ObjectNameId[num];
	_objectIDMapSize = num;
	for (i = 0; i < num; i++) {
		// Add to object name-to-id map
		_fileHandle->read(_objectIDMap[i].name, 40);
		_objectIDMap[i].id = i;

		_objectStateTable[i] = _fileHandle->readByte();
		_objectRoomTable[i] = _fileHandle->readByte();
		_classData[i] = _fileHandle->readUint32LE();
	}
	memset(_objectOwnerTable, 0xFF, num);

	// Finally, sort the object name->ID map, so we can later use
	// bsearch on it. For this we (ab)use strcmp, which works fine
	// since the table entries start with a string.
	qsort(_objectIDMap, _objectIDMapSize, sizeof(ObjectNameId),
			(int (*)(const void*, const void*))strcmp);
}

void ScummEngine_v7::readGlobalObjects() {
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);
	assert(_objectStateTable);
	assert(_objectOwnerTable);

	_fileHandle->read(_objectStateTable, num);
	_fileHandle->read(_objectRoomTable, num);
	memset(_objectOwnerTable, 0xFF, num);

	_fileHandle->read(_classData, num * sizeof(uint32));

#if defined(SCUMM_BIG_ENDIAN)
	// Correct the endianess if necessary
	for (int i = 0; i != num; i++)
		_classData[i] = FROM_LE_32(_classData[i]);
#endif
}
#endif

void ScummEngine::allocateArrays() {
	// Note: Buffers are now allocated in scummMain to allow for
	//     early GUI init.

	_objectOwnerTable = (byte *)calloc(_numGlobalObjects, 1);
	_objectStateTable = (byte *)calloc(_numGlobalObjects, 1);
	_classData = (uint32 *)calloc(_numGlobalObjects, sizeof(uint32));
	_newNames = (uint16 *)calloc(_numNewNames, sizeof(uint16));

	_inventory = (uint16 *)calloc(_numInventory, sizeof(uint16));
	_verbs = (VerbSlot *)calloc(_numVerbs, sizeof(VerbSlot));
	_objs = (ObjectData *)calloc(_numLocalObjects, sizeof(ObjectData));
	_roomVars = (int32 *)calloc(_numRoomVariables, sizeof(int32));
	_scummVars = (int32 *)calloc(_numVariables, sizeof(int32));
	_bitVars = (byte *)calloc(_numBitVariables >> 3, 1);
	if (_game.heversion >= 60) {
		_arraySlot = (byte *)calloc(_numArray, 1);
	}

	_res->allocResTypeData(rtCostume, (_game.features & GF_NEW_COSTUMES) ? MKTAG('A','K','O','S') : MKTAG('C','O','S','T'),
								_numCostumes, kStaticResTypeMode);
	_res->allocResTypeData(rtRoom, MKTAG('R','O','O','M'), _numRooms, kStaticResTypeMode);
	_res->allocResTypeData(rtRoomImage, MKTAG('R','M','I','M'), _numRooms, kStaticResTypeMode);
	_res->allocResTypeData(rtRoomScripts, MKTAG('R','M','S','C'), _numRooms, kStaticResTypeMode);
	_res->allocResTypeData(rtSound, MKTAG('S','O','U','N'), _numSounds, kSoundResTypeMode);
	_res->allocResTypeData(rtScript, MKTAG('S','C','R','P'), _numScripts, kStaticResTypeMode);
	_res->allocResTypeData(rtCharset, MKTAG('C','H','A','R'), _numCharsets, kStaticResTypeMode);
	_res->allocResTypeData(rtObjectName, 0, _numNewNames, kDynamicResTypeMode);
	_res->allocResTypeData(rtInventory, 0, _numInventory, kDynamicResTypeMode);
	_res->allocResTypeData(rtTemp, 0, 10, kDynamicResTypeMode);
	_res->allocResTypeData(rtScaleTable, 0, 5, kDynamicResTypeMode);
	_res->allocResTypeData(rtActorName, 0, _numActors, kDynamicResTypeMode);
	_res->allocResTypeData(rtVerb, 0, _numVerbs, kDynamicResTypeMode);
	_res->allocResTypeData(rtString, 0, _numArray, kDynamicResTypeMode);
	_res->allocResTypeData(rtFlObject, 0, _numFlObject, kDynamicResTypeMode);
	_res->allocResTypeData(rtMatrix, 0, 10, kDynamicResTypeMode);
	_res->allocResTypeData(rtImage, MKTAG('A','W','I','Z'), _numImages, kStaticResTypeMode);
	_res->allocResTypeData(rtTalkie, MKTAG('T','L','K','E'), _numTalkies, kStaticResTypeMode);
}

void ScummEngine_v70he::allocateArrays() {
	ScummEngine::allocateArrays();

	_res->allocResTypeData(rtSpoolBuffer, 0, 9, kStaticResTypeMode);
	_heV7RoomIntOffsets = (uint32 *)calloc(_numRooms, sizeof(uint32));
}


void ScummEngine::dumpResource(const char *tag, int id, const byte *ptr, int length) {
	char buf[256];
	Common::DumpFile out;

	uint32 size;
	if (length >= 0)
		size = length;
	else if (_game.features & GF_OLD_BUNDLE)
		size = READ_LE_UINT16(ptr);
	else if (_game.features & GF_SMALL_HEADER)
		size = READ_LE_UINT32(ptr);
	else
		size = READ_BE_UINT32(ptr + 4);

	sprintf(buf, "dumps/%s%d.dmp", tag, id);

	out.open(buf);
	if (out.isOpen() == false)
		return;
	out.write(ptr, size);
	out.close();
}

ResourceIterator::ResourceIterator(const byte *searchin, bool smallHeader)
	: _ptr(searchin), _smallHeader(smallHeader) {
	assert(searchin);
	if (_smallHeader) {
		_size = READ_LE_UINT32(searchin);
		_pos = 6;
		_ptr = searchin + 6;
	} else {
		_size = READ_BE_UINT32(searchin + 4);
		_pos = 8;
		_ptr = searchin + 8;
	}

}

const byte *ResourceIterator::findNext(uint32 tag) {
	uint32 size = 0;
	const byte *result = 0;

	if (_smallHeader) {
		uint16 smallTag = newTag2Old(tag);
		do {
			if (_pos >= _size)
				return 0;

			result = _ptr;
			size = READ_LE_UINT32(result);
			if ((int32)size <= 0)
				return 0;	// Avoid endless loop

			_pos += size;
			_ptr += size;
		} while (READ_LE_UINT16(result + 4) != smallTag);
	} else {
		do {
			if (_pos >= _size)
				return 0;

			result = _ptr;
			size = READ_BE_UINT32(result + 4);
			if ((int32)size <= 0)
				return 0;	// Avoid endless loop

			_pos += size;
			_ptr += size;
		} while (READ_BE_UINT32(result) != tag);
	}

	return result;
}

const byte *ScummEngine::findResource(uint32 tag, const byte *searchin) {
	uint32 curpos, totalsize, size;

	debugC(DEBUG_RESOURCE, "findResource(%s, %p)", tag2str(tag), (const void *)searchin);

	if (!searchin) {
		if (_game.heversion >= 70) {
			searchin = _resourceLastSearchBuf;
			totalsize = _resourceLastSearchSize;
			curpos = 0;
		} else {
			assert(searchin);
			return NULL;
		}
	} else {
		searchin += 4;
		_resourceLastSearchSize = totalsize = READ_BE_UINT32(searchin);
		curpos = 8;
		searchin += 4;
	}

	while (curpos < totalsize) {
		if (READ_BE_UINT32(searchin) == tag) {
			_resourceLastSearchBuf = searchin;
			return searchin;
		}

		size = READ_BE_UINT32(searchin + 4);
		if ((int32)size <= 0) {
			error("(%s) Not found in %d... illegal block len %d", tag2str(tag), 0, size);
			return NULL;
		}

		curpos += size;
		searchin += size;
	}

	return NULL;
}

const byte *findResourceSmall(uint32 tag, const byte *searchin) {
	uint32 curpos, totalsize, size;
	uint16 smallTag;

	smallTag = newTag2Old(tag);
	if (smallTag == 0)
		return NULL;

	assert(searchin);

	totalsize = READ_LE_UINT32(searchin);
	searchin += 6;
	curpos = 6;

	while (curpos < totalsize) {
		size = READ_LE_UINT32(searchin);

		if (READ_LE_UINT16(searchin + 4) == smallTag)
			return searchin;

		if ((int32)size <= 0) {
			error("(%s) Not found in %d... illegal block len %d", tag2str(tag), 0, size);
			return NULL;
		}

		curpos += size;
		searchin += size;
	}

	return NULL;
}

uint16 newTag2Old(uint32 newTag) {
	switch (newTag) {
	case (MKTAG('R','M','H','D')):
		return (0x4448);	// HD
	case (MKTAG('I','M','0','0')):
		return (0x4D42);	// BM
	case (MKTAG('E','X','C','D')):
		return (0x5845);	// EX
	case (MKTAG('E','N','C','D')):
		return (0x4E45);	// EN
	case (MKTAG('S','C','A','L')):
		return (0x4153);	// SA
	case (MKTAG('L','S','C','R')):
		return (0x534C);	// LS
	case (MKTAG('O','B','C','D')):
		return (0x434F);	// OC
	case (MKTAG('O','B','I','M')):
		return (0x494F);	// OI
	case (MKTAG('S','M','A','P')):
		return (0x4D42);	// BM
	case (MKTAG('C','L','U','T')):
		return (0x4150);	// PA
	case (MKTAG('B','O','X','D')):
		return (0x5842);	// BX
	case (MKTAG('C','Y','C','L')):
		return (0x4343);	// CC
	case (MKTAG('E','P','A','L')):
		return (0x5053);	// SP
	case (MKTAG('T','I','L','E')):
		return (0x4C54);	// TL
	case (MKTAG('Z','P','0','0')):
		return (0x505A);	// ZP
	default:
		return (0);
	}
}

const char *nameOfResType(ResType type) {
	static char buf[100];

	switch (type) {
	case rtRoom:
		return "Room";
	case rtScript:
		return "Script";
	case rtCostume:
		return "Costume";
	case rtSound:
		return "Sound";
	case rtInventory:
		return "Inventory";
	case rtCharset:
		return "Charset";
	case rtString:
		return "String";
	case rtVerb:
		return "Verb";
	case rtActorName:
		return "ActorName";
	case rtBuffer:
		return "Buffer";
	case rtScaleTable:
		return "ScaleTable";
	case rtTemp:
		return "Temp";
	case rtFlObject:
		return "FlObject";
	case rtMatrix:
		return "Matrix";
	case rtBox:
		return "Box";
	case rtObjectName:
		return "ObjectName";
	case rtRoomScripts:
		return "RoomScripts";
	case rtRoomImage:
		return "RoomImage";
	case rtImage:
		return "Image";
	case rtTalkie:
		return "Talkie";
	case rtSpoolBuffer:
		return "SpoolBuffer";
	default:
		sprintf(buf, "rt%d", type);
		return buf;
	}
}

} // End of namespace Scumm
