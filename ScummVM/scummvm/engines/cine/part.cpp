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
#include "common/endian.h"
#include "common/textconsole.h"

#include "cine/cine.h"
#include "cine/unpack.h"
#include "cine/various.h"

namespace Cine {

void loadPart(const char *partName) {
	g_cine->_partBuffer.clear();

	g_cine->_partFileHandle.close();

	checkDataDisk(-1);

	if (!g_cine->_partFileHandle.open(partName))
		error("loadPart(): Cannot open file %s", partName);

	setMouseCursor(MOUSE_CURSOR_DISK);

	uint16 numElementInPart = g_cine->_partFileHandle.readUint16BE();
	g_cine->_partBuffer.resize(numElementInPart);
	g_cine->_partFileHandle.readUint16BE(); // entry size

	if (currentPartName != partName)
		strcpy(currentPartName, partName);

	for (uint16 i = 0; i < g_cine->_partBuffer.size(); i++) {
		g_cine->_partFileHandle.read(g_cine->_partBuffer[i].partName, 14);
		g_cine->_partBuffer[i].offset = g_cine->_partFileHandle.readUint32BE();
		g_cine->_partBuffer[i].packedSize = g_cine->_partFileHandle.readUint32BE();
		g_cine->_partBuffer[i].unpackedSize = g_cine->_partFileHandle.readUint32BE();
		g_cine->_partFileHandle.readUint32BE(); // unused
	}

	if (g_cine->getGameType() == Cine::GType_FW && g_cine->getPlatform() == Common::kPlatformPC && strcmp(partName, "BASESON.SND") != 0)
		loadPal(partName);
}

void closePart() {
	// TODO
}

static Common::String fixVolCnfFileName(const uint8 *src, uint len) {
	assert(len == 11 || len == 13);
	// Copy source to a temporary buffer and force a trailing zero for string manipulation
	char tmp[14];
	memcpy(tmp, src, len);
	tmp[len] = 0;

	if (len == 11) {
		// Filenames of length 11 have no separation of the extension and the basename
		// so that's why we have to convert them first. There's no trailing zero in them
		// either and they're always of the full length 11 with padding spaces. Extension
		// can be always found at offset 8 onwards.
		//
		// Examples of filename mappings:
		// "AEROPORTMSG" -> "AEROPORT.MSG"
		// "MITRAILLHP " -> "MITRAILL.HP" (Notice the trailing space after the extension)
		// "BOND10     " -> "BOND10"
		// "GIRL    SET" -> "GIRL.SET"

		// Replace all space characters with zeroes
		for (uint i = 0; i < len; i++)
			if (tmp[i] == ' ')
				tmp[i] = 0;
		// Extract the filename's extension
		Common::String extension(tmp + 8);
		tmp[8] = 0; // Force separation of extension and basename
		Common::String basename(tmp);
		if (extension.empty()) {
			return basename;
		} else {
			return basename + "." + extension;
		}
	} else {
		// Filenames of length 13 are okay as they are, no need for conversion
		return Common::String(tmp);
	}
}

void CineEngine::readVolCnf() {
	Common::File f;
	if (!f.open("vol.cnf")) {
		error("Unable to open 'vol.cnf'");
	}
	uint32 unpackedSize, packedSize;
	char hdr[8];
	f.read(hdr, 8);
	bool compressed = (memcmp(hdr, "ABASECP", 7) == 0);
	if (compressed) {
		unpackedSize = f.readUint32BE();
		packedSize = f.readUint32BE();
	} else {
		f.seek(0);
		unpackedSize = packedSize = f.size();
	}
	uint8 *buf = new uint8[unpackedSize];
	uint8 *packedBuf = new uint8[packedSize];
	f.read(packedBuf, packedSize);
	CineUnpacker cineUnpacker;
	if (!cineUnpacker.unpack(packedBuf, packedSize, buf, unpackedSize)) {
		error("Error while unpacking 'vol.cnf' data");
	}
	delete[] packedBuf;
	uint8 *p = buf;
	int resourceFilesCount = READ_BE_UINT16(p); p += 2;
	int entrySize = READ_BE_UINT16(p); p += 2;
	for (int i = 0; i < resourceFilesCount; ++i) {
		char volumeResourceFile[9];
		memcpy(volumeResourceFile, p, 8);
		volumeResourceFile[8] = 0;
		_volumeResourceFiles.push_back(volumeResourceFile);
		p += entrySize;
	}

	// Check file name blocks' sizes
	bool fileNameLenMod11, fileNameLenMod13;
	fileNameLenMod11 = fileNameLenMod13 = true;
	for (int i = 0; i < resourceFilesCount; ++i) {
		int size = READ_BE_UINT32(p); p += 4;
		fileNameLenMod11 &= ((size % 11) == 0);
		fileNameLenMod13 &= ((size % 13) == 0);
		p += size;
	}
	// Make sure at least one of the candidates for file name length fits the data
	assert(fileNameLenMod11 || fileNameLenMod13);

	// File name length used to be deduced from the fact whether the file
	// was compressed or not. Compressed files used file name length 11,
	// uncompressed files used file name length 13. It worked almost always,
	// but not with the game entry that's detected as the Operation Stealth's
	// US Amiga release. It uses a compressed 'vol.cnf' file but still uses
	// file names of length 13. So we try to deduce the file name length from
	// the data in the 'vol.cnf' file.
	int fileNameLength;
	if (fileNameLenMod11 != fileNameLenMod13) {
		// All file name blocks' sizes were divisible by either 11 or 13, but not with both.
		fileNameLength = (fileNameLenMod11 ? 11 : 13);
	} else {
		warning("Couldn't deduce file name length from data in 'vol.cnf', using a backup deduction scheme");
		// Here we use the former file name length detection method
		// if we couldn't deduce the file name length from the data.
		fileNameLength = (compressed ? 11 : 13);
	}

	p = buf + 4 + resourceFilesCount * entrySize;
	for (int i = 0; i < resourceFilesCount; ++i) {
		int count = READ_BE_UINT32(p) / fileNameLength; p += 4;
		while (count--) {
			Common::String volumeEntryName = fixVolCnfFileName(p, fileNameLength);
			_volumeEntriesMap.setVal(volumeEntryName, _volumeResourceFiles[i].c_str());
			debugC(5, kCineDebugPart, "Added volume entry name '%s' resource file '%s'", volumeEntryName.c_str(), _volumeResourceFiles[i].c_str());
			p += fileNameLength;
		}
	}

	delete[] buf;
}

int16 findFileInBundle(const char *fileName) {
	if (g_cine->getGameType() == Cine::GType_OS) {
		// look first in currently loaded resource file
		for (uint i = 0; i < g_cine->_partBuffer.size(); i++) {
			if (!scumm_stricmp(fileName, g_cine->_partBuffer[i].partName)) {
				return i;
			}
		}
		// not found, open the required resource file
		StringPtrHashMap::const_iterator it = g_cine->_volumeEntriesMap.find(fileName);
		if (it == g_cine->_volumeEntriesMap.end()) {
			warning("Unable to find part file for filename '%s'", fileName);
			return -1;
		}
		const char *part = (*it)._value;
		loadPart(part);
	}
	for (uint i = 0; i < g_cine->_partBuffer.size(); i++) {
		if (!scumm_stricmp(fileName, g_cine->_partBuffer[i].partName)) {
			return i;
		}
	}
	return -1;
}

void readFromPart(int16 idx, byte *dataPtr, uint32 maxSize) {
	assert(maxSize >= g_cine->_partBuffer[idx].packedSize);
	setMouseCursor(MOUSE_CURSOR_DISK);

	g_cine->_partFileHandle.seek(g_cine->_partBuffer[idx].offset, SEEK_SET);
	g_cine->_partFileHandle.read(dataPtr, g_cine->_partBuffer[idx].packedSize);
}

byte *readBundleFile(int16 foundFileIdx, uint32 *size) {
	assert(foundFileIdx >= 0 && foundFileIdx < (int32)g_cine->_partBuffer.size());
	bool error = false;
	byte *dataPtr = (byte *)calloc(g_cine->_partBuffer[foundFileIdx].unpackedSize, 1);
	byte *packedData = (byte *)calloc(g_cine->_partBuffer[foundFileIdx].packedSize, 1);
	assert(dataPtr && packedData);
	readFromPart(foundFileIdx, packedData, g_cine->_partBuffer[foundFileIdx].packedSize);
	CineUnpacker cineUnpacker;
	error = !cineUnpacker.unpack(packedData, g_cine->_partBuffer[foundFileIdx].packedSize, dataPtr, g_cine->_partBuffer[foundFileIdx].unpackedSize);
	free(packedData);

	if (error) {
		warning("Error unpacking '%s' from bundle file '%s'", g_cine->_partBuffer[foundFileIdx].partName, currentPartName);
	}

	// Set the size variable if a pointer to it has been given
	if (size != NULL) {
		*size = g_cine->_partBuffer[foundFileIdx].unpackedSize;
	}

	return dataPtr;
}

byte *readBundleSoundFile(const char *entryName, uint32 *size) {
	int16 index;
	byte *data = 0;
	char previousPartName[15] = "";

	if (g_cine->getGameType() == Cine::GType_FW) {
		strcpy(previousPartName, currentPartName);
		loadPart("BASESON.SND");
	}
	index = findFileInBundle((const char *)entryName);
	if (index != -1) {
		data = readBundleFile(index);
		if (size) {
			*size = g_cine->_partBuffer[index].unpackedSize;
		}
	}
	if (g_cine->getGameType() == Cine::GType_FW) {
		loadPart(previousPartName);
	}
	return data;
}

/** Rotate byte value to the left by n bits */
byte rolByte(byte value, uint n) {
	n %= 8;
	return (byte)((value << n) | (value >> (8 - n)));
}

byte *readFile(const char *filename, bool crypted) {
	Common::File in;

	in.open(filename);

	if (!in.isOpen())
		error("readFile(): Cannot open file %s", filename);

	uint32 size = in.size();

	byte *dataPtr = (byte *)malloc(size);
	in.read(dataPtr, size);

	// The Sony published CD version of Future Wars has its
	// AUTO00.PRC file's bytes rotated to the right by one.
	// So we decode the so called crypting by rotating all
	// the bytes to the left by one.
	if (crypted) {
		for (uint index = 0; index < size; index++) {
			dataPtr[index] = rolByte(dataPtr[index], 1);
		}
	}

	return dataPtr;
}

void checkDataDisk(int16 param) {
}

void dumpBundle(const char *fileName) {
	char tmpPart[15];

	strcpy(tmpPart, currentPartName);

	loadPart(fileName);
	for (uint i = 0; i < g_cine->_partBuffer.size(); i++) {
		byte *data = readBundleFile(i);

		debug(0, "%s", g_cine->_partBuffer[i].partName);

		Common::DumpFile out;
		if (out.open(Common::String("dumps/") + g_cine->_partBuffer[i].partName)) {
			out.write(data, g_cine->_partBuffer[i].unpackedSize);
			out.close();
		}

		free(data);
	}

	loadPart(tmpPart);
}

} // End of namespace Cine
