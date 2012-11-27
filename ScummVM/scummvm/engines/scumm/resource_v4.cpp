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


#include "scumm/scumm_v4.h"
#include "scumm/file.h"
#include "scumm/resource.h"
#include "scumm/util.h"

namespace Scumm {

extern const char *nameOfResType(ResType type);

int ScummEngine_v4::readResTypeList(ResType type) {
	uint num;

	debug(9, "readResTypeList(%s)", nameOfResType(type));

	num = _fileHandle->readUint16LE();

	if (num != _res->_types[type].size()) {
		error("Invalid number of %ss (%d) in directory", nameOfResType(type), num);
	}

	for (ResId idx = 0; idx < num; idx++) {
		_res->_types[type][idx]._roomno = _fileHandle->readByte();
		_res->_types[type][idx]._roomoffs = _fileHandle->readUint32LE();
	}

	return num;
}

void ScummEngine_v4::readIndexFile() {
	uint16 blocktype;
	uint32 itemsize;
	int numblock = 0;

	debug(9, "readIndexFile()");

	closeRoom();
	openRoom(0);

	while (true) {
		// Figure out the sizes of various resources
		itemsize = _fileHandle->readUint32LE();
		blocktype = _fileHandle->readUint16LE();
		if (_fileHandle->eos() || _fileHandle->err())
			break;

		switch (blocktype) {
		case 0x4E52:	// 'NR'
			_fileHandle->readUint16LE();
			break;
		case 0x5230:	// 'R0'
			_numRooms = _fileHandle->readUint16LE();
			break;
		case 0x5330:	// 'S0'
			_numScripts = _fileHandle->readUint16LE();
			break;
		case 0x4E30:	// 'N0'
			_numSounds = _fileHandle->readUint16LE();
			break;
		case 0x4330:	// 'C0'
			_numCostumes = _fileHandle->readUint16LE();
			break;
		case 0x4F30:	// 'O0'
			_numGlobalObjects = _fileHandle->readUint16LE();

			// Indy3 FM-TOWNS has 32 extra bytes of unknown meaning
			if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformFMTowns)
				itemsize += 32;
			break;
		}
		_fileHandle->seek(itemsize - 8, SEEK_CUR);
	}

	_fileHandle->seek(0, SEEK_SET);

	readMAXS(0);
	allocateArrays();

	while (true) {
		itemsize = _fileHandle->readUint32LE();

		if (_fileHandle->eos() || _fileHandle->err())
			break;

		blocktype = _fileHandle->readUint16LE();

		numblock++;

		switch (blocktype) {

		case 0x4E52:	// 'NR'
			// Names of rooms. Maybe we should put them into a table, for use by the debugger?
			for (int room; (room = _fileHandle->readByte()); ) {
				char buf[10];
				_fileHandle->read(buf, 9);
				buf[9] = 0;
				for (int i = 0; i < 9; i++)
					buf[i] ^= 0xFF;
				debug(5, "Room %d: '%s'", room, buf);
			}
			break;

		case 0x5230:	// 'R0'
			readResTypeList(rtRoom);
			break;

		case 0x5330:	// 'S0'
			readResTypeList(rtScript);
			break;

		case 0x4E30:	// 'N0'
			readResTypeList(rtSound);
			break;

		case 0x4330:	// 'C0'
			readResTypeList(rtCostume);
			break;

		case 0x4F30:	// 'O0'
			readGlobalObjects();
			break;

		default:
			error("Bad ID %c%c found in directory", blocktype & 0xFF, blocktype >> 8);
		}
	}
	closeRoom();
}

void ScummEngine_v4::loadCharset(int no) {
	uint32 size;
	memset(_charsetData, 0, sizeof(_charsetData));

	assertRange(0, no, 4, "charset");
	closeRoom();

	Common::File file;
	char buf[20];

	sprintf(buf, "%03d.LFL", 900 + no);
	file.open(buf);

	if (file.isOpen() == false) {
		error("loadCharset(%d): Missing file charset: %s", no, buf);
	}

	size = file.readUint32LE() + 11;
	file.read(_res->createResource(rtCharset, no, size), size);
}

void ScummEngine_v4::readMAXS(int blockSize) {
	// FIXME - I'm not sure for those values yet, they will have to be rechecked

	_numVariables = 800;				// 800
	_numBitVariables = 4096;			// 2048
	_numLocalObjects = 200;				// 200
	_numArray = 50;
	_numVerbs = 100;
	_numNewNames = 50;
	_objectRoomTable = NULL;
	_numCharsets = 9;					// 9
	_numInventory = 80;					// 80
	_numGlobalScripts = 200;
	_numFlObject = 50;

	_shadowPaletteSize = 256;

	_shadowPalette = (byte *) calloc(_shadowPaletteSize, 1);	// FIXME - needs to be removed later
}

void ScummEngine_v4::readGlobalObjects() {
	int i;
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);

	uint32 bits;
	byte tmp;
	if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine) {
		for (i = 0; i != num; i++) {
			bits = _fileHandle->readByte();
			bits |= _fileHandle->readByte() << 8;
			bits |= _fileHandle->readByte() << 16;
			_classData[i] = bits;
		}
		for (i = 0; i != num; i++) {
			tmp = _fileHandle->readByte();
			_objectOwnerTable[i] = tmp & OF_OWNER_MASK;
			_objectStateTable[i] = tmp >> OF_STATE_SHL;
		}
	} else {
		for (i = 0; i != num; i++) {
			bits = _fileHandle->readByte();
			bits |= _fileHandle->readByte() << 8;
			bits |= _fileHandle->readByte() << 16;
			_classData[i] = bits;
			tmp = _fileHandle->readByte();
			_objectOwnerTable[i] = tmp & OF_OWNER_MASK;
			_objectStateTable[i] = tmp >> OF_STATE_SHL;
		}
	}

	// FIXME: Indy3 FM-TOWNS has 32 extra bytes of unknown meaning
	if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformFMTowns)
		_fileHandle->seek(32, SEEK_CUR);
}


} // End of namespace Scumm
