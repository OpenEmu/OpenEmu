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

// RSC Resource file management module (SAGA 1, used in ITE)

#include "saga/saga.h"
#include "saga/resource.h"

namespace Saga {

#define ID_MIDI     MKTAG('M','i','d','i')

bool ResourceContext_RSC::loadMacMIDI() {
	// Sanity check
	if (_fileSize < RSC_MIN_FILESIZE + MAC_BINARY_HEADER_SIZE)
		return false;

	_file.seek(83);
	int macDataSize = _file.readSint32BE();
	int macResOffset = MAC_BINARY_HEADER_SIZE + (((macDataSize + 127) >> 7) << 7);

	_file.seek(macResOffset);
	uint32 macDataOffset = _file.readUint32BE() + macResOffset;
	uint32 macMapOffset = _file.readUint32BE() + macResOffset;

	_file.seek(macMapOffset + 22);
	_file.readUint16BE();	// resAttr
	int16 typeOffset = _file.readUint16BE();
	_file.readUint16BE();	// nameOffset
	uint16 numTypes = _file.readUint16BE() + 1;

	_file.seek(macMapOffset + typeOffset + 2);

	// Find the MIDI files
	for (uint16 i = 0; i < numTypes; i++) {
		uint32 id = _file.readUint32BE();
		uint16 items = _file.readUint16BE() + 1;
		uint16 offset = _file.readUint16BE();

		if (id == ID_MIDI) {
			for (uint16 curMidi = 0; curMidi < items; curMidi++) {
				// Jump to the header of the entry and read its fields
				_file.seek(offset + macMapOffset + typeOffset + curMidi * 12);
				uint16 midiID = _file.readUint16BE();
				_file.readUint16BE();	// nameOffset
				uint32 midiOffset = _file.readUint32BE() & 0xFFFFFF;
				_file.readUint32BE();	// macResSize

				// Jump to the actual data and read the file size
				_file.seek(macDataOffset + midiOffset);
				uint32 midiSize = _file.readUint32BE();

				// Add the entry
				if (_table.size() <= midiID)
					_table.resize(midiID + 1);
				_table[midiID].offset = macDataOffset + midiOffset + 4;
				_table[midiID].size = midiSize;
			}
		}
	}

	return true;
}

void ResourceContext_RSC::processPatches(Resource *resource, const GamePatchDescription *patchFiles) {
	const GamePatchDescription *patchDescription;
	ResourceData *resourceData;

	// Process external patch files
	for (patchDescription = patchFiles; patchDescription && patchDescription->fileName; ++patchDescription) {
		if ((patchDescription->fileType & _fileType) != 0) {
			if (patchDescription->resourceId < _table.size()) {
				resourceData = &_table[patchDescription->resourceId];
				// Check if we've already found a patch for this resource. One is enough.
				if (!resourceData->patchData) {
					resourceData->patchData = new PatchData(patchDescription->fileName);
					if (resourceData->patchData->_patchFile->open(patchDescription->fileName)) {
						resourceData->offset = 0;
						resourceData->size = resourceData->patchData->_patchFile->size();
						// The patched ITE file is in memory, so close the patch file
						resourceData->patchData->_patchFile->close();
					} else {
						delete resourceData->patchData;
						resourceData->patchData = NULL;
					}
				}
			}
		}
	}
}

} // End of namespace Saga
