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
#include "common/str.h"
#include "common/memstream.h"

#include "draci/barchive.h"
#include "draci/draci.h"

namespace Draci {

const char BArchive::_magicNumber[] = "BAR!";
const char BArchive::_dfwMagicNumber[] = "BS";

/**
 * @brief Loads a DFW archive
 * @param path Path to input file
 *
 * Tries to load the file as a DFW archive if opening as BAR fails. Should only be called
 * from openArchive(). Only one of the game files appears to use this format (HRA.DFW)
 * and this file is compressed using a simple run-length scheme.
 *
 * archive format: header
 *                 index table
 *                 file0, file1, ...
 *
 * header format: [uint16LE] file count
 *                [uint16LE] index table size
 *                [2 bytes]  magic number "BS"
 *
 * index table format: entry0, entry1, ...
 *
 * entry<N> format: [uint16LE] compressed size (not including the 2 bytes for the
 *                             "uncompressed size" field)
 *                  [uint32LE] fileN offset from start of file
 *
 * file<N> format: [uint16LE] uncompressed size
 *                 [uint16LE] compressed size (the same as in the index table entry)
 *                 [byte] stopper mark (for run-length compression)
 *                 [multiple bytes] compressed data
 */

void BArchive::openDFW(const Common::String &path) {
	byte *table;
	uint16 tableSize;
	byte buf[2];

	_f.open(path);
	if (!_f.isOpen()) {
		debugC(2, kDraciArchiverDebugLevel, "Error opening file");
		return;
	}

	_fileCount = _f.readUint16LE();
	tableSize = _f.readUint16LE();

	_f.read(buf, 2);
	if (memcmp(buf, _dfwMagicNumber, 2) == 0) {
		debugC(2, kDraciArchiverDebugLevel, "Success");
		_isDFW = true;
	} else {
		debugC(2, kDraciArchiverDebugLevel, "Not a DFW archive");
		_f.close();
		return;
	}

	debugC(2, kDraciArchiverDebugLevel, "Archive info (DFW): %d files", _fileCount);

	// Read in index table
	table = new byte[tableSize];
	_f.read(table, tableSize);

	// Read in file headers, but do not read the actual data yet
	// The data will be read on demand to save memory
	_files = new BAFile[_fileCount];
	Common::MemoryReadStream tableReader(table, tableSize);
	for (uint i = 0; i < _fileCount; ++i) {
		_files[i]._compLength = tableReader.readUint16LE();
		_files[i]._offset = tableReader.readUint32LE();

		// Seek to the current file
		_f.seek(_files[i]._offset);

		_files[i]._length = _f.readUint16LE(); // Read in uncompressed length
		_f.readUint16LE(); // Compressed length again (already read from the index table)
		_files[i]._stopper = _f.readByte();

		_files[i]._data = NULL; // File data will be read in on demand
		_files[i]._crc = 0; // Dummy value; not used in DFW archives
	}

	// Indicate that the archive was successfully opened
	_opened = true;

	// Cleanup
	delete[] table;
}

/**
 * @brief BArchive open method
 * @param path Path to input file
 *
 * Opens a BAR (Bob's Archiver) archive, which is the game's archiving format.
 * BAR archives have a .DFW file extension, due to a historical interface.
 *
 * archive format: header,
 *                 file0, file1, ...
 *                 footer
 *
 * header format: [4 bytes] magic number "BAR!"
 *                [uint16LE] file count (number of archived streams),
 *                [uint32LE] footer offset from start of file
 *
 * file<N> format: [2 bytes] compressed length
 *                 [2 bytes] original length
 *                 [1 byte] compression type
 *                 [1 byte] CRC
 *                 [multiple bytes] actual data
 *
 * footer format: [array of uint32LE] offsets of individual files from start of archive
 *                (last entry is footer offset again)
 */

void BArchive::openArchive(const Common::String &path) {
	byte buf[4];
	byte *footer;
	uint32 footerOffset, footerSize;

	// Close previously opened archive (if any)
	closeArchive();

	debugCN(2, kDraciArchiverDebugLevel, "Loading archive %s: ", path.c_str());

	_f.open(path);
	if (_f.isOpen()) {
		debugC(2, kDraciArchiverDebugLevel, "Success");
	} else {
		debugC(2, kDraciArchiverDebugLevel, "Error");
		return;
	}

	// Save path for reading in files later on
	_path = path;

	// Read archive header
	debugCN(2, kDraciArchiverDebugLevel, "Checking for BAR magic number: ");

	_f.read(buf, 4);
	if (memcmp(buf, _magicNumber, 4) == 0) {
		debugC(2, kDraciArchiverDebugLevel, "Success");

		// Indicate this archive is a BAR
		_isDFW = false;
	} else {
		debugC(2, kDraciArchiverDebugLevel, "Not a BAR archive");
		debugCN(2, kDraciArchiverDebugLevel, "Retrying as DFW: ");
		_f.close();

		// Try to open as DFW
		openDFW(_path);

		return;
	}

	_fileCount = _f.readUint16LE();
	footerOffset = _f.readUint32LE();
	footerSize = _f.size() - footerOffset;

	debugC(2, kDraciArchiverDebugLevel, "Archive info: %d files, %d data bytes",
		_fileCount, footerOffset - _archiveHeaderSize);

	// Read in footer
	footer = new byte[footerSize];
	_f.seek(footerOffset);
	_f.read(footer, footerSize);
	Common::MemoryReadStream reader(footer, footerSize);

	// Read in file headers, but do not read the actual data yet
	// The data will be read on demand to save memory
	_files = new BAFile[_fileCount];

	for (uint i = 0; i < _fileCount; i++) {
		uint32 fileOffset;

		fileOffset = reader.readUint32LE();
		_f.seek(fileOffset); 						// Seek to next file in archive

		_files[i]._compLength = _f.readUint16LE(); 	// Compressed size
													// should be the same as uncompressed

		_files[i]._length = _f.readUint16LE(); 		// Original size

		_files[i]._offset = fileOffset;				// Offset of file from start

		byte compressionType = _f.readByte();
		assert(compressionType == 0 &&
			"Compression type flag is non-zero (file is compressed)");

		_files[i]._crc = _f.readByte(); 	// CRC checksum of the file
		_files[i]._data = NULL; 		// File data will be read in on demand
		_files[i]._stopper = 0; 		// Dummy value; not used in BAR files, needed in DFW
	}

	// Last footer item should be equal to footerOffset
	uint32 footerOffset2 = reader.readUint32LE();
	assert(footerOffset2 == footerOffset && "Footer offset mismatch");

	// Indicate that the archive has been successfully opened
	_opened = true;

	delete[] footer;
}

/**
 * @brief BArchive close method
 *
 * Closes the currently opened archive. It can be called explicitly to
 * free up memory.
 */
void BArchive::closeArchive() {
	if (!_opened) {
		return;
	}

	for (uint i = 0; i < _fileCount; ++i) {
		if (_files[i]._data) {
			delete[] _files[i]._data;
		}
	}

	delete[] _files;
	_f.close();

	_opened = false;
	_files = NULL;
	_fileCount = 0;
}

/**
 * @brief On-demand BAR file loader
 * @param i Index of file inside an archive
 * @return Pointer to a BAFile coresponding to the opened file or NULL (on failure)
 *
 * Loads individual BAR files from an archive to memory on demand.
 * Should not be called directly.
 */
BAFile *BArchive::loadFileBAR(uint i) {
	// Else open archive and read in requested file
	if (!_f.isOpen()) {
		debugC(2, kDraciArchiverDebugLevel, "Error");
		return NULL;
	}

	// Read in the file (without the file header)
	_f.seek(_files[i]._offset + _fileHeaderSize);
	_files[i]._data = new byte[_files[i]._length];
	_f.read(_files[i]._data, _files[i]._length);

	// Calculate CRC
	byte tmp = 0;
	for (uint j = 0; j < _files[i]._length; j++) {
		tmp ^= _files[i]._data[j];
	}

	debugC(2, kDraciArchiverDebugLevel, "Read %d bytes", _files[i]._length);
	assert(tmp == _files[i]._crc && "CRC checksum mismatch");

	return _files + i;
}

/**
 * @brief On-demand DFW file loader
 * @param i Index of file inside an archive
 * @return Pointer to a BAFile coresponding to the opened file or NULL (on failure)
 *
 * Loads individual DFW files from an archive to memory on demand.
 * Should not be called directly.
 */
BAFile *BArchive::loadFileDFW(uint i) {
	byte *buf;

	// Else open archive and read in requested file
	if (!_f.isOpen()) {
		debugC(2, kDraciArchiverDebugLevel, "Error");
		return NULL;
	}

	// Seek to raw data of the file
	// Five bytes are for the header (uncompressed and compressed length, stopper mark)
	_f.seek(_files[i]._offset + 5);

	// Since we are seeking directly to raw data, we subtract 3 bytes from the length
	// (to take account the compressed length and stopper mark)
	uint16 compressedLength = _files[i]._compLength - 3;
	uint16 uncompressedLength = _files[i]._length;

	debugC(2, kDraciArchiverDebugLevel,
		"File info (DFW): uncompressed %d bytes, compressed %d bytes",
		uncompressedLength, compressedLength);

	// Allocate a buffer for the file data
	buf = new byte[compressedLength];

	// Read in file data into the buffer
	_f.read(buf, compressedLength);

	// Allocate the space for the uncompressed file
	byte *dst;
	dst = _files[i]._data = new byte[uncompressedLength];

	Common::MemoryReadStream data(buf, compressedLength);

	// Uncompress file
	byte current, what;
	byte stopper = _files[i]._stopper;
	uint repeat;
	uint len = 0; // Sanity check (counts uncompressed bytes)

	current = data.readByte(); // Read initial byte
	while (!data.eos()) {
		if (current != stopper) {
			*dst++ = current;
			++len;
		} else {
			// Inflate block
			repeat = data.readByte();
			what = data.readByte();
			len += repeat;
			for (uint j = 0; j < repeat; ++j) {
				*dst++ = what;
			}
		}

		current = data.readByte();
	}

	assert(len == _files[i]._length && "Uncompressed file not of the expected length");

	delete[] buf;

	return _files + i;
}

/**
 * Clears the cache of the open files inside the archive without closing it.
 * If the files are subsequently accessed, they are read from the disk.
 */
void BArchive::clearCache() {
	// Delete all cached data
	for (uint i = 0; i < _fileCount; ++i) {
		_files[i].close();
	}
}

const BAFile *BArchive::getFile(uint i) {
	// Check whether requested file exists
	if (i >= _fileCount) {
		return NULL;
	}

	debugCN(2, kDraciArchiverDebugLevel, "Accessing file %d from archive %s... ",
		i, _path.c_str());

	// Check if file has already been opened and return that
	if (_files[i]._data) {
		debugC(2, kDraciArchiverDebugLevel, "Cached");
		return _files + i;
	}

	BAFile *file;

	// file will be NULL if something goes wrong
	if (_isDFW) {
		file = loadFileDFW(i);
	} else {
		file = loadFileBAR(i);
	}

	return file;
}

} // End of namespace Draci
