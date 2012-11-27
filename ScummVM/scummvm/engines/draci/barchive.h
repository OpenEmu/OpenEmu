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

#ifndef DRACI_BARCHIVE_H
#define DRACI_BARCHIVE_H

#include "common/str.h"
#include "common/file.h"

namespace Draci {

/**
 *  Represents individual files inside the archive.
 */
struct BAFile {
	uint _compLength; ///< Compressed length (the same as _length if the file is uncompressed)
	uint _length;     ///< Uncompressed length
	uint32 _offset;   ///< Offset of file inside archive
	byte *_data;
	byte _crc;
	byte _stopper;    ///< Not used in BAR files, needed for DFW

	/** Releases the file data (for memory considerations) */
	void close() {
		delete[] _data;
		_data = NULL;
	}
};

class BArchive {
public:
	BArchive() : _files(NULL), _fileCount(0), _opened(false) {}

	BArchive(const Common::String &path) :
	_files(NULL), _fileCount(0), _opened(false) {
		openArchive(path);
	}

	~BArchive() { closeArchive(); }

	void openArchive(const Common::String &path);
	void closeArchive();
	uint size() const { return _fileCount; }

	/**
	 * Checks whether there is an archive opened. Should be called before reading
	 * from the archive to check whether openArchive() succeeded.
	 */
	bool isOpen() const { return _opened; }

	void clearCache();

	const BAFile *getFile(uint i);

private:
	// Archive header data
	static const char _magicNumber[];
	static const char _dfwMagicNumber[];
	static const uint _archiveHeaderSize = 10;

	// File stream header data
	static const uint _fileHeaderSize = 6;

	Common::String _path;    ///< Path to file
	BAFile *_files;          ///< Internal array of files
	uint _fileCount;         ///< Number of files in archive
	bool _isDFW;             ///< True if the archive is in DFW format, false otherwise
	bool _opened;            ///< True if the archive is opened, false otherwise
	Common::File _f;         ///< Opened file handle

	void openDFW(const Common::String &path);
	BAFile *loadFileDFW(uint i);
	BAFile *loadFileBAR(uint i);
};

} // End of namespace Draci

#endif // DRACI_BARCHIVE_H
