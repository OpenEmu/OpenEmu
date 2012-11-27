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


#include "scumm/scumm_v3.h"
#include "scumm/file.h"
#include "scumm/resource.h"
#include "scumm/util.h"

namespace Scumm {

extern const char *nameOfResType(ResType type);

int ScummEngine_v3old::readResTypeList(ResType type) {
	uint num;
	ResId idx;

	debug(9, "readResTypeList(%s)", nameOfResType(type));

	num = _fileHandle->readByte();

	if (num >= 0xFF) {
		error("Too many %ss (%d) in directory", nameOfResType(type), num);
	}

	if (type == rtRoom) {
		for (idx = 0; idx < num; idx++)
			_res->_types[type][idx]._roomno = idx;
		_fileHandle->seek(num, SEEK_CUR);
	} else {
		for (idx = 0; idx < num; idx++)
			_res->_types[type][idx]._roomno = _fileHandle->readByte();
	}
	for (idx = 0; idx < num; idx++) {
		_res->_types[type][idx]._roomoffs = _fileHandle->readUint16LE();
		if (_res->_types[type][idx]._roomoffs == 0xFFFF)
			_res->_types[type][idx]._roomoffs = (uint32)RES_INVALID_OFFSET;
	}

	return num;
}

void ScummEngine_v3old::readIndexFile() {
	int magic = 0;
	debug(9, "readIndexFile()");

	closeRoom();
	openRoom(0);

	magic = _fileHandle->readUint16LE();
	if (magic != 0x0100)
		error("The magic id doesn't match (0x%X)", magic);

	_numGlobalObjects = _fileHandle->readUint16LE();
	_fileHandle->seek(_numGlobalObjects * 4, SEEK_CUR);
	_numRooms = _fileHandle->readByte();
	_fileHandle->seek(_numRooms * 3, SEEK_CUR);
	_numCostumes = _fileHandle->readByte();
	_fileHandle->seek(_numCostumes * 3, SEEK_CUR);
	_numScripts = _fileHandle->readByte();
	_fileHandle->seek(_numScripts * 3, SEEK_CUR);
	_numSounds = _fileHandle->readByte();

	_fileHandle->clearErr();
	_fileHandle->seek(0, SEEK_SET);

	readMAXS(0);
	allocateArrays();

	_fileHandle->readUint16LE(); /* version magic number */
	readGlobalObjects();
	readResTypeList(rtRoom);
	readResTypeList(rtCostume);
	readResTypeList(rtScript);
	readResTypeList(rtSound);

	closeRoom();
}

void ScummEngine_v3::readRoomsOffsets() {
}

void ScummEngine_v3::loadCharset(int no) {
	uint32 size;
	memset(_charsetData, 0, sizeof(_charsetData));

	assertRange(0, no, 2, "charset");
	closeRoom();

	Common::File file;
	char buf[20];

	sprintf(buf, "%02d.LFL", 99 - no);
	file.open(buf);

	if (file.isOpen() == false) {
		error("loadCharset(%d): Missing file charset: %s", no, buf);
	}

	size = file.readUint16LE();
	file.read(_res->createResource(rtCharset, no, size), size);
}

} // End of namespace Scumm
