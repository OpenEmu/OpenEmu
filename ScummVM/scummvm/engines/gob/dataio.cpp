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
#include "common/types.h"
#include "common/memstream.h"
#include "common/substream.h"

#include "gob/gob.h"
#include "gob/dataio.h"
#include "gob/global.h"
#include "gob/util.h"

namespace Gob {

DataIO::File::File() : size(0), offset(0), compression(0), archive(0) {
}

DataIO::File::File(const Common::String &n, uint32 s, uint32 o, uint8 c, Archive &a) :
	name(n), size(s), offset(o), compression(c), archive(&a) {
}


DataIO::DataIO() {
	// Reserve memory for the standard max amount of archives
	_archives.reserve(kMaxArchives);
	for (int i = 0; i < kMaxArchives; i++)
		_archives.push_back(0);
}

DataIO::~DataIO() {
	// Close all archives
	for (Common::Array<Archive *>::iterator it = _archives.begin(); it != _archives.end(); ++it) {
		if (!*it)
			continue;

		closeArchive(**it);
		delete *it;
	}
}

void DataIO::getArchiveInfo(Common::Array<ArchiveInfo> &info) const {
	info.resize(_archives.size());

	for (uint i = 0; i < _archives.size(); i++) {
		if (!_archives[i])
			continue;

		info[i].name      = _archives[i]->name;
		info[i].base      = _archives[i]->base;
		info[i].fileCount = _archives[i]->files.size();
	}
}

uint32 DataIO::getSizeChunks(Common::SeekableReadStream &src) {
	uint32 size = 0;

	uint32 chunkSize = 2, realSize;
	while (chunkSize != 0xFFFF) {
		src.skip(chunkSize - 2);

		chunkSize = src.readUint16LE();
		realSize  = src.readUint16LE();

		assert(chunkSize >= 4);

		size += realSize;
	}

	assert(!src.eos());

	src.seek(0);

	return size;
}

byte *DataIO::unpack(Common::SeekableReadStream &src, int32 &size, uint8 compression, bool useMalloc) {
	assert((compression == 1) || (compression == 2));

	if      (compression == 1)
		size = src.readUint32LE();
	else if (compression == 2)
		size = getSizeChunks(src);

	assert(size > 0);

	byte *data = 0;
	if (useMalloc)
		data = (byte *) malloc(size);
	else
		data = new byte[size];

	if      (compression == 1)
		unpackChunk(src, data, size);
	else if (compression == 2)
		unpackChunks(src, data, size);

	return data;
}

byte *DataIO::unpack(const byte *src, uint32 srcSize, int32 &size, uint8 compression) {
	Common::MemoryReadStream srcStream(src, srcSize);

	return unpack(srcStream, size, compression, false);
}

Common::SeekableReadStream *DataIO::unpack(Common::SeekableReadStream &src, uint8 compression) {
	int32 size;

	byte *data = unpack(src, size, compression, true);
	if (!data)
		return 0;

	return new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
}

void DataIO::unpackChunks(Common::SeekableReadStream &src, byte *dest, uint32 size) {
	uint32 chunkSize = 0, realSize;
	while (chunkSize != 0xFFFF) {
		uint32 pos = src.pos();

		chunkSize = src.readUint16LE();
		realSize  = src.readUint16LE();

		assert(chunkSize >= 4);
		assert(size >= realSize);

		src.skip(2);

		unpackChunk(src, dest, realSize);

		if (chunkSize != 0xFFFF)
			src.seek(pos + chunkSize + 2);

		size -= realSize;
		dest += realSize;
	}
}

void DataIO::unpackChunk(Common::SeekableReadStream &src, byte *dest, uint32 size) {
	byte *tmpBuf = new byte[4114];
	assert(tmpBuf);

	uint32 counter = size;

	for (int i = 0; i < 4078; i++)
		tmpBuf[i] = 0x20;
	uint16 tmpIndex = 4078;

	uint16 cmd = 0;
	while (1) {
		cmd >>= 1;
		if ((cmd & 0x0100) == 0)
			cmd = src.readByte() | 0xFF00;

		if ((cmd & 1) != 0) { /* copy */
			byte tmp = src.readByte();

			*dest++ = tmp;
			tmpBuf[tmpIndex] = tmp;

			tmpIndex++;
			tmpIndex %= 4096;
			counter--;
			if (counter == 0)
				break;
		} else { /* copy string */
			byte tmp1 = src.readByte();
			byte tmp2 = src.readByte();

			int16 off = tmp1 | ((tmp2 & 0xF0) << 4);
			byte  len =         (tmp2 & 0x0F) + 3;

			for (int i = 0; i < len; i++) {
				*dest++ = tmpBuf[(off + i) % 4096];
				counter--;
				if (counter == 0) {
					delete[] tmpBuf;
					return;
				}
				tmpBuf[tmpIndex] = tmpBuf[(off + i) % 4096];
				tmpIndex++;
				tmpIndex %= 4096;
			}

		}
	}

	delete[] tmpBuf;
}

bool DataIO::openArchive(Common::String name, bool base) {
	// Look for a free archive slot
	Archive **archive = 0;
	int i = 0;
	for (Common::Array<Archive *>::iterator it = _archives.begin(); it != _archives.end(); ++it, i++) {
		if (!*it) {
			archive = &*it;
			break;
		}
	}

	if (!archive) {
		// No free slot, create a new one

		warning("DataIO::openArchive(): Need to increase archive count to %d", _archives.size() + 1);
		_archives.push_back(0);

		Common::Array<Archive *>::iterator it = _archives.end();
		archive = &*(--it);
	}

	// Add extension if necessary
	if (!name.contains('.'))
		name += ".stk";

	// Try to open
	*archive = openArchive(name);
	if (!*archive)
		return false;

	(*archive)->base = base;
	return true;
}

DataIO::Archive *DataIO::openArchive(const Common::String &name) {
	Archive *archive = new Archive;
	if (!archive->file.open(name)) {
		delete archive;
		return 0;
	}

	archive->name = name;

	uint16 fileCount = archive->file.readUint16LE();
	for (uint16 i = 0; i < fileCount; i++) {
		File file;

		char fileName[14];

		archive->file.read(fileName, 13);
		fileName[13] = '\0';

		file.size        = archive->file.readUint32LE();
		file.offset      = archive->file.readUint32LE();
		file.compression = archive->file.readByte() != 0;

		// Replacing cyrillic characters
		Util::replaceChar(fileName, (char) 0x85, 'E');
		Util::replaceChar(fileName, (char) 0x8A, 'K');
		Util::replaceChar(fileName, (char) 0x8E, 'O');
		Util::replaceChar(fileName, (char) 0x91, 'C');
		Util::replaceChar(fileName, (char) 0x92, 'T');

		file.name = fileName;

		// Geisha use 0ot files, which are compressed TOT files without the packed byte set.
		if (file.name.hasSuffix(".0OT")) {
			file.name.setChar('T', file.name.size() - 3);
			file.compression = 2;
		}

		file.archive = archive;
		archive->files.setVal(file.name, file);
	}

	return archive;
}

bool DataIO::closeArchive(bool base) {
	// Look for a matching archive and close it
	for (int archive = _archives.size() - 1; archive >= 0; archive--) {
		if (_archives[archive] && (_archives[archive]->base == base)) {
			closeArchive(*_archives[archive]);
			delete _archives[archive];
			_archives[archive] = 0;

			return true;
		}
	}

	return false;
}

bool DataIO::closeArchive(Archive &archive) {
	archive.file.close();

	return true;
}

bool DataIO::hasFile(const Common::String &name){
	// Look up the files in the opened archives
	if (findFile(name))
		return true;

	// Else, look if a plain file that matches exists
	return Common::File::exists(name);
}

int32 DataIO::fileSize(const Common::String &name) {
	// Try to find the file in the archives
	File *file = findFile(name);
	if (file) {
		if (file->compression == 0)
			return file->size;

		// Sanity checks
		assert(file->size >= 4);
		assert(file->archive);
		assert(file->archive->file.isOpen());

		// Read the full, unpacked size
		file->archive->file.seek(file->offset);

		if (file->compression == 2)
			file->archive->file.skip(4);

		return file->archive->file.readUint32LE();
	}

	// Else, try to find a matching plain file
	Common::File f;
	if (!f.open(name))
		return -1;

	return f.size();
}

Common::SeekableReadStream *DataIO::getFile(const Common::String &name) {
	// Try to open the file in the archives
	File *file = findFile(name);
	if (file) {
		Common::SeekableReadStream *data = getFile(*file);
		if (data)
			return data;
	}

	// Else, try to open a matching plain file
	Common::File f;
	if (!f.open(name))
		return 0;

	return f.readStream(f.size());
}

byte *DataIO::getFile(const Common::String &name, int32 &size) {
	// Try to open the file in the archives
	File *file = findFile(name);
	if (file) {
		byte *data = getFile(*file, size);
		if (data)
			return data;
	}

	// Else, try to open a matching plain file
	Common::File f;
	if (!f.open(name))
		return 0;

	size = f.size();

	byte *data = new byte[size];
	if (f.read(data, size) != ((uint32) size)) {
		delete[] data;
		return 0;
	}

	return data;
}

DataIO::File *DataIO::findFile(const Common::String &name) {
	for (int i = _archives.size() - 1; i >= 0; i--) {
		Archive *archive = _archives[i];
		if (!archive)
			// Empty slot
			continue;

		// Look up the file in the file map
		FileMap::iterator file = archive->files.find(name);
		if (file != archive->files.end())
			return &file->_value;
	}

	return 0;
}

Common::SeekableReadStream *DataIO::getFile(File &file) {
	if (!file.archive)
		return 0;

	if (!file.archive->file.isOpen())
		return 0;

	if (!file.archive->file.seek(file.offset))
		return 0;

	Common::SeekableReadStream *rawData =
		new Common::SafeSeekableSubReadStream(&file.archive->file, file.offset, file.offset + file.size);

	if (file.compression == 0)
		return rawData;

	Common::SeekableReadStream *unpackedData = unpack(*rawData, file.compression);

	delete rawData;

	return unpackedData;
}

byte *DataIO::getFile(File &file, int32 &size) {
	if (!file.archive)
		return 0;

	if (!file.archive->file.isOpen())
		return 0;

	if (!file.archive->file.seek(file.offset))
		return 0;

	size = file.size;

	byte *rawData = new byte[file.size];
	if (file.archive->file.read(rawData, file.size) != file.size) {
		delete[] rawData;
		return 0;
	}

	if (file.compression == 0)
		return rawData;

	byte *unpackedData = unpack(rawData, file.size, size, file.compression);

	delete[] rawData;

	return unpackedData;
}

} // End of namespace Gob
