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
#include "common/file.h"
#include "common/util.h"
#include "common/scummsys.h"

#include "lure/disk.h"
#include "lure/lure.h"
#include "lure/luredefs.h"
#include "lure/res.h"

namespace Lure {

static Disk *int_disk = NULL;

Disk &Disk::getReference() {
	return *int_disk;
}

Disk::Disk() {
	_fileNum = 0xff;
	_fileHandle = NULL;
	int_disk = this;
}

Disk::~Disk() {
	delete _fileHandle;
	int_disk = NULL;
}

uint8 Disk::indexOf(uint16 id, bool suppressError) {
	// Make sure the correct file is open - the upper two bits of the Id give the file number. Note
	// that an extra check is done for the upper byte of the Id being 0x3f, which is the Id range
	// I use for lure.dat resources, which are resources extracted from the lure.exe executable
	uint8 entryFileNum = ((id>>8) == 0x3f) ? 0 : ((id >> 14) & 3) + 1;
	openFile(entryFileNum);

	// Find the correct entry in the list based on the Id
	for (int entryIndex=0; entryIndex<NUM_ENTRIES_IN_HEADER; ++entryIndex) {
		if (_entries[entryIndex].id == HEADER_ENTRY_UNUSED_ID) break;
		else if (_entries[entryIndex].id == id) return entryIndex;
	}

	if (suppressError) return 0xff;
	if (_fileNum == 0)
		error("Could not find entry Id #%d in file %s", id, SUPPORT_FILENAME);
	else
		error("Could not find entry Id #%d in file disk%d.%s", id, _fileNum,
			LureEngine::getReference().isEGA() ? "ega" : "vga");
}

void Disk::openFile(uint8 fileNum) {
	// Validate that the file number is correct
	bool isEGA = LureEngine::getReference().isEGA();
	if (fileNum > 4)
		error("Invalid file number specified - %d", fileNum);

	// Only load up the new file if the current file number has changed
	if (fileNum == _fileNum) return;

	// Delete any existing open file handle
	if (_fileNum != 0xff) delete _fileHandle;
	_fileNum = fileNum;

	// Open up the the new file
	_fileHandle = new Common::File();

	char sFilename[10];
	if (_fileNum == 0)
		strcpy(sFilename, SUPPORT_FILENAME);
	else
		sprintf(sFilename, "disk%d.%s", _fileNum, isEGA ? "ega" : "vga");

	_fileHandle->open(sFilename);
	if (!_fileHandle->isOpen())
		error("Could not open %s", sFilename);

	char buffer[7];

	// If it's the support file, then move to the correct language area

	_dataOffset = 0;
	if (_fileNum == 0) {
		// Validate overall header
		_fileHandle->read(buffer, 6);
		buffer[4] = '\0';

		if (strcmp(buffer, SUPPORT_IDENT_STRING) != 0)
			error("The file %s is not a valid Lure support file", sFilename);

		// Scan for the correct language block
		LureLanguage language = LureEngine::getReference().getLureLanguage();
		bool foundFlag = false;

		while (!foundFlag) {
			_fileHandle->read(buffer, 5);
			if ((byte)buffer[0] == 0xff)
				error("Could not find language data in support file");

			if ((language == (LureLanguage)buffer[0]) || (language == LANG_UNKNOWN)) {
				foundFlag = true;
				_dataOffset = READ_LE_UINT32(&buffer[1]);
				_fileHandle->seek(_dataOffset);
			}
		}
	}

	// Validate the header

	_fileHandle->read(buffer, 6);
	buffer[6] = '\0';
	if (strcmp(buffer, HEADER_IDENT_STRING) != 0)
		error("The file %s was not a valid VGA file", sFilename);

	uint16 fileFileNum = _fileHandle->readUint16BE();
	if ((fileFileNum != 0) && (fileFileNum != (isEGA ? _fileNum + 4 : _fileNum)))
		error("The file %s was not the correct file number", sFilename);

	// Read in the header entries
	uint32 headerSize = sizeof(FileEntry) * NUM_ENTRIES_IN_HEADER;
	if (_fileHandle->read(_entries, headerSize) != headerSize)
		error("The file %s had a corrupted header", sFilename);

#ifdef SCUMM_BIG_ENDIAN
	// Process the read in header list to convert to big endian
	for (int i = 0; i < NUM_ENTRIES_IN_HEADER; ++i) {
		_entries[i].id = FROM_LE_16(_entries[i].id);
		_entries[i].size = FROM_LE_16(_entries[i].size);
		_entries[i].offset = FROM_LE_16(_entries[i].offset);
	}
#endif
}

uint32 Disk::getEntrySize(uint16 id) {
	// Special room area check
	uint16 tempId = id & 0x3fff;
	if ((tempId == 0x120) || (tempId == 0x311) || (tempId == 8) || (tempId == 0x410)) {
		ValueTableData &fieldList = Resources::getReference().fieldList();
		if (fieldList.getField(AREA_FLAG) != 0)
			id ^= 0x8000;
	}

	// Get the index of the resource, if necessary opening the correct file
	uint8 index = indexOf(id);

	// Calculate the offset and size of the entry
	uint32 size = (uint32) _entries[index].size;
	if (_entries[index].sizeExtension) size += 0x10000;

	return size;
}

MemoryBlock *Disk::getEntry(uint16 id) {
	// Special room area check
	uint16 tempId = id & 0x3fff;
	if ((tempId == 0x120) || (tempId == 0x311) || (tempId == 8) || (tempId == 0x410)) {
		ValueTableData &fieldList = Resources::getReference().fieldList();
		if (fieldList.getField(AREA_FLAG) != 0)
			id ^= 0x8000;
	}

	// Get the index of the resource, if necessary opening the correct file
	uint8 index = indexOf(id);

	// Calculate the offset and size of the entry
	uint32 size = (uint32) _entries[index].size;
	if (_entries[index].sizeExtension) size += 0x10000;
	uint32 offset = (uint32) _entries[index].offset * 0x20 + _dataOffset;

	MemoryBlock *result = Memory::allocate(size);
	_fileHandle->seek(offset, SEEK_SET);
	_fileHandle->read(result->data(), size);
	return result;
}

bool Disk::exists(uint16 id) {
	// Get the index of the resource, if necessary opening the correct file
	uint8 index = indexOf(id, true);
	return (index != 0xff);
}

uint8 Disk::numEntries() {
	if (_fileNum == 0)
		error("No file is currently open");

	// Figure out how many entries there are by count until an unused entry is found
	for (byte entryIndex = 0; entryIndex < NUM_ENTRIES_IN_HEADER; ++entryIndex)
		if (_entries[entryIndex].id == HEADER_ENTRY_UNUSED_ID) return entryIndex;

	return NUM_ENTRIES_IN_HEADER;
}

FileEntry *Disk::getIndex(uint8 entryIndex) {
	if (_fileNum == 0)
		error("No file is currently open");
	if ((entryIndex >= NUM_ENTRIES_IN_HEADER) || (_entries[entryIndex].id == HEADER_ENTRY_UNUSED_ID))
		error("There is no entry at the specified index");

	return &_entries[entryIndex];
}

} // End of namespace Lure
