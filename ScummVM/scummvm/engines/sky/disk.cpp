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


#include "common/debug.h"
#include "common/textconsole.h"
#include "common/endian.h"
#include "common/file.h"

#include "sky/disk.h"
#include "sky/sky.h"
#include "sky/struc.h"

namespace Sky {

static const char *const dataFilename = "sky.dsk";
static const char *const dinnerFilename = "sky.dnr";

Disk::Disk() {
	_dataDiskHandle = new Common::File();
	Common::File *dnrHandle = new Common::File();

	dnrHandle->open(dinnerFilename);
	if (!dnrHandle->isOpen())
		error("Could not open %s", dinnerFilename);

	if (!(_dinnerTableEntries = dnrHandle->readUint32LE()))
		error("Error reading from sky.dnr"); //even though it was opened correctly?!

	_dinnerTableArea = (uint8 *)malloc(_dinnerTableEntries * 8);
	uint32 entriesRead = dnrHandle->read(_dinnerTableArea, 8 * _dinnerTableEntries) / 8;

	if (entriesRead != _dinnerTableEntries)
		error("entriesRead != dinnerTableEntries. [%d/%d]", entriesRead, _dinnerTableEntries);

	_dataDiskHandle->open(dataFilename);
	if (!_dataDiskHandle->isOpen())
		error("Error opening %s", dataFilename);

	debug("Found BASS version v0.0%d (%d dnr entries)", determineGameVersion(), _dinnerTableEntries);

	memset(_buildList, 0, 60 * 2);
	memset(_loadedFilesList, 0, 60 * 4);

	dnrHandle->close();
	delete dnrHandle;
}

Disk::~Disk() {
	if (_dataDiskHandle->isOpen())
		_dataDiskHandle->close();
	fnFlushBuffers();
	free(_dinnerTableArea);
	delete _dataDiskHandle;
}

bool Disk::fileExists(uint16 fileNr) {
	return (getFileInfo(fileNr) != NULL);
}

// allocate memory, load the file and return a pointer
uint8 *Disk::loadFile(uint16 fileNr) {
	uint8 cflag;

	debug(3, "load file %d,%d (%d)", (fileNr >> 11), (fileNr & 2047), fileNr);

	uint8 *fileInfoPtr = getFileInfo(fileNr);
	if (fileInfoPtr == NULL) {
		debug(1, "File %d not found", fileNr);
		return NULL;
	}

	uint32 fileFlags = READ_LE_UINT24(fileInfoPtr + 5);
	uint32 fileSize = fileFlags & 0x03fffff;
	uint32 fileOffset = READ_LE_UINT32(fileInfoPtr + 2) & 0x0ffffff;

	_lastLoadedFileSize = fileSize;
	cflag = (uint8)((fileOffset >> 23) & 0x1);
	fileOffset &= 0x7FFFFF;

	if (cflag) {
		if (SkyEngine::_systemVars.gameVersion == 331)
			fileOffset <<= 3;
		else
			fileOffset <<= 4;
	}

	uint8 *fileDest = (uint8 *)malloc(fileSize + 4); // allocate memory for file

	_dataDiskHandle->seek(fileOffset, SEEK_SET);

	//now read in the data
	int32 bytesRead = _dataDiskHandle->read(fileDest, fileSize);

	if (bytesRead != (int32)fileSize)
		warning("Unable to read %d bytes from datadisk (%d bytes read)", fileSize, bytesRead);

	cflag = (uint8)((fileFlags >> 23) & 0x1);
	//if cflag == 0 then file is compressed, 1 == uncompressed

	DataFileHeader *fileHeader = (DataFileHeader *)fileDest;

	if ((!cflag) && ((FROM_LE_16(fileHeader->flag) >> 7) & 1)) {
		debug(4, "File is RNC compressed.");

		uint32 decompSize = (FROM_LE_16(fileHeader->flag) & ~0xFF) << 8;
		decompSize |= FROM_LE_16(fileHeader->s_tot_size);

		uint8 *uncompDest = (uint8 *)malloc(decompSize);

		int32 unpackLen;
		void *output, *input = fileDest + sizeof(DataFileHeader);

		if ((fileFlags >> 22) & 0x1) { //do we include the header?
			// don't return the file's header
			output = uncompDest;
			unpackLen = _rncDecoder.unpackM1(input, output, 0);
		} else {
#ifdef SCUMM_BIG_ENDIAN
			// Convert DataFileHeader to BE (it only consists of 16 bit words)
			uint16 *headPtr = (uint16 *)fileDest;
			for (uint i = 0; i < sizeof(DataFileHeader) / 2; i++)
				*(headPtr + i) = READ_LE_UINT16(headPtr + i);
#endif

			memcpy(uncompDest, fileDest, sizeof(DataFileHeader));
			output = uncompDest + sizeof(DataFileHeader);
			unpackLen = _rncDecoder.unpackM1(input, output, 0);
			if (unpackLen)
				unpackLen += sizeof(DataFileHeader);
		}

		debug(5, "UnpackM1 returned: %d", unpackLen);

		if (unpackLen == 0) { //Unpack returned 0: file was probably not packed.
			free(uncompDest);
			return fileDest;
		} else {
			if (unpackLen != (int32)decompSize)
				debug(1, "ERROR: File %d: invalid decomp size! (was: %d, should be: %d)", fileNr, unpackLen, decompSize);
			_lastLoadedFileSize = decompSize;

			free(fileDest);
			return uncompDest;
		}
	} else {
#ifdef SCUMM_BIG_ENDIAN
		if (!cflag) {
			uint16 *headPtr = (uint16 *)fileDest;
			for (uint i = 0; i < sizeof(DataFileHeader) / 2; i++)
				*(headPtr + i) = READ_LE_UINT16(headPtr + i);
		}
#endif
		return fileDest;
	}
}

uint16 *Disk::loadScriptFile(uint16 fileNr) {
	uint16 *buf = (uint16 *)loadFile(fileNr);
#ifdef SCUMM_BIG_ENDIAN
	for (uint i = 0; i < _lastLoadedFileSize / 2; i++)
		buf[i] = FROM_LE_16(buf[i]);
#endif
	return buf;
}

uint8 *Disk::getFileInfo(uint16 fileNr) {
	uint16 i;
	uint16 *dnrTbl16Ptr = (uint16 *)_dinnerTableArea;

	for (i = 0; i < _dinnerTableEntries; i++) {
		if (READ_LE_UINT16(dnrTbl16Ptr) == fileNr) {
			debug(4, "file %d found", fileNr);
			return (uint8 *)dnrTbl16Ptr;
		}
		dnrTbl16Ptr += 4;
	}

	return 0; //not found
}

void Disk::fnCacheChip(uint16 *fList) {
	// fnCacheChip is called after fnCacheFast
	uint16 cnt = 0;
	while (_buildList[cnt])
		cnt++;
	uint16 fCnt = 0;
	do {
		_buildList[cnt + fCnt] = fList[fCnt] & 0x7FFFU;
		fCnt++;
	} while (fList[fCnt-1]);
	fnCacheFiles();
}

void Disk::fnCacheFast(uint16 *fList) {
	if (fList != NULL) {
		uint8 cnt = 0;
		do {
			_buildList[cnt] = fList[cnt] & 0x7FFFU;
			cnt++;
		} while (fList[cnt-1]);
	}
}

void Disk::fnCacheFiles() {
	uint16 lCnt, bCnt, targCnt;
	targCnt = lCnt = 0;
	bool found;
	while (_loadedFilesList[lCnt]) {
		bCnt = 0;
		found = false;
		while (_buildList[bCnt] && (!found)) {
			if ((_buildList[bCnt] & 0x7FFFU) == _loadedFilesList[lCnt])
				found = true;
			else
				bCnt++;
		}
		if (found) {
			_loadedFilesList[targCnt] = _loadedFilesList[lCnt];
			targCnt++;
		} else {
			free(SkyEngine::_itemList[_loadedFilesList[lCnt] & 2047]);
			SkyEngine::_itemList[_loadedFilesList[lCnt] & 2047] = NULL;
		}
		lCnt++;
	}
	_loadedFilesList[targCnt] = 0; // mark end of list
	bCnt = 0;
	while (_buildList[bCnt]) {
		if ((_buildList[bCnt] & 0x7FF) == 0x7FF) {
			// amiga dummy files
			bCnt++;
			continue;
		}
		lCnt = 0;
		found = false;
		while (_loadedFilesList[lCnt] && (!found)) {
			if (_loadedFilesList[lCnt] == (_buildList[bCnt] & 0x7FFFU))
				found = true;
			lCnt++;
		}
		if (found) {
			bCnt++;
			continue;
		}
		// ok, we really have to load the file.
		_loadedFilesList[targCnt] = _buildList[bCnt] & 0x7FFFU;
		targCnt++;
		_loadedFilesList[targCnt] = 0;
		SkyEngine::_itemList[_buildList[bCnt] & 2047] = (void**)loadFile(_buildList[bCnt] & 0x7FFF);
		if (!SkyEngine::_itemList[_buildList[bCnt] & 2047])
			warning("fnCacheFiles: Disk::loadFile() returned NULL for file %d",_buildList[bCnt] & 0x7FFF);
		bCnt++;
	}
	_buildList[0] = 0;
}

void Disk::refreshFilesList(uint32 *list) {
	uint8 cnt = 0;
	while (_loadedFilesList[cnt]) {
		if (SkyEngine::_itemList[_loadedFilesList[cnt] & 2047])
			free(SkyEngine::_itemList[_loadedFilesList[cnt] & 2047]);
		SkyEngine::_itemList[_loadedFilesList[cnt] & 2047] = NULL;
		cnt++;
	}
	cnt = 0;
	while (list[cnt]) {
		_loadedFilesList[cnt] = list[cnt];
		SkyEngine::_itemList[_loadedFilesList[cnt] & 2047] = (void**)loadFile((uint16)(_loadedFilesList[cnt] & 0x7FFF));
		cnt++;
	}
	_loadedFilesList[cnt] = 0;
}

void Disk::fnMiniLoad(uint16 fileNum) {
	uint16 cnt = 0;
	while (_loadedFilesList[cnt]) {
		if (_loadedFilesList[cnt] == fileNum)
			return;
		cnt++;
	}
	_loadedFilesList[cnt] = fileNum & 0x7FFFU;
	_loadedFilesList[cnt + 1] = 0;
	SkyEngine::_itemList[fileNum & 2047] = (void**)loadFile(fileNum);
}

void Disk::fnFlushBuffers() {
	// dump all loaded sprites
	uint8 lCnt = 0;
	while (_loadedFilesList[lCnt]) {
		free(SkyEngine::_itemList[_loadedFilesList[lCnt] & 2047]);
		SkyEngine::_itemList[_loadedFilesList[lCnt] & 2047] = NULL;
		lCnt++;
	}
	_loadedFilesList[0] = 0;
}

void Disk::dumpFile(uint16 fileNr) {
	char buf[128];
	Common::DumpFile out;
	byte* filePtr;

	filePtr = loadFile(fileNr);
	sprintf(buf, "dumps/file-%d.dmp", fileNr);

	if (!Common::File::exists(buf)) {
		if (out.open(buf))
			out.write(filePtr, _lastLoadedFileSize);
	}
	free(filePtr);
}

uint32 Disk::determineGameVersion() {
	//determine game version based on number of entries in dinner table
	switch (_dinnerTableEntries) {
	case 232:
		// German floppy demo (v0.0272)
		return 272;
	case 243:
		// pc gamer demo (v0.0109)
		return 109;
	case 247:
		// English floppy demo (v0.0267)
		return 267;
	case 1404:
		//floppy (v0.0288)
		return 288;
	case 1413:
		//floppy (v0.0303)
		return 303;
	case 1445:
		//floppy (v0.0331 or v0.0348)
		if (_dataDiskHandle->size() == 8830435)
			return 348;
		else
			return 331;
	case 1711:
		//cd demo (v0.0365)
		return 365;
	case 5099:
		//cd (v0.0368)
		return 368;
	case 5097:
		//cd (v0.0372)
		return 372;
	default:
		//unknown version
		error("Unknown game version! %d dinner table entries", _dinnerTableEntries);
		break;
	}
}

} // End of namespace Sky
