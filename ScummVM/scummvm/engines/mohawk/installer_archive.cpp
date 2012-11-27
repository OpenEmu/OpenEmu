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

#include "mohawk/installer_archive.h"

#include "common/dcl.h"
#include "common/debug.h"

namespace Mohawk {

InstallerArchive::InstallerArchive() : Common::Archive() {
	_stream = 0;
}

InstallerArchive::~InstallerArchive() {
	close();
}

bool InstallerArchive::open(const Common::String &filename) {
	close();

	_stream = SearchMan.createReadStreamForMember(filename);

	if (!_stream)
		return false;

	// Check for the magic uint32
	// No idea what it means, but it's how "file" recognizes them
	if (_stream->readUint32BE() != 0x135D658C) {
		close();
		return false;
	}

	// Let's pull some relevant data from the header
	_stream->seek(41);
	uint32 directoryTableOffset = _stream->readUint32LE();
	/* uint32 directoryTableSize = */ _stream->readUint32LE();
	uint16 directoryCount = _stream->readUint16LE();
	uint32 fileTableOffset = _stream->readUint32LE();
	/* uint32 fileTableSize = */ _stream->readUint32LE();

	// We need to have at least one directory in order for the archive to be valid
	if (directoryCount == 0) {
		close();
		return false;
	}

	// TODO: Currently, we only support getting files from the first directory
	// To that end, get the number of files from that entry
	_stream->seek(directoryTableOffset);
	uint16 fileCount = _stream->readUint16LE();
	debug(2, "File count = %d", fileCount);

	// Following the directory table is the file table with files stored recursively
	// by directory. Since we're only using the first directory, we can just go
	// right to that one.
	_stream->seek(fileTableOffset);

	for (uint16 i = 0; i < fileCount; i++) {
		FileEntry entry;

		_stream->skip(3); // Unknown

		entry.uncompressedSize = _stream->readUint32LE();
		entry.compressedSize = _stream->readUint32LE();
		entry.offset = _stream->readUint32LE();

		_stream->skip(14); // Unknown

		byte nameLength = _stream->readByte();
		Common::String name;
		while (nameLength--)
			name += _stream->readByte();

		_stream->skip(13); // Unknown

		_map[name] = entry;

		debug(3, "Found file '%s' at 0x%08x (Comp: 0x%08x, Uncomp: 0x%08x)", name.c_str(),
				entry.offset, entry.compressedSize, entry.uncompressedSize);
	}

	return true;
}

void InstallerArchive::close() {
	delete _stream; _stream = 0;
	_map.clear();
}

bool InstallerArchive::hasFile(const Common::String &name) const {
	return _map.contains(name);
}

int InstallerArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (FileMap::const_iterator it = _map.begin(); it != _map.end(); it++)
		list.push_back(getMember(it->_key));

	return _map.size();
}

const Common::ArchiveMemberPtr InstallerArchive::getMember(const Common::String &name) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *InstallerArchive::createReadStreamForMember(const Common::String &name) const {
	if (!_stream || !_map.contains(name))
		return 0;

	const FileEntry &entry = _map[name];

	// Seek to our offset and then send it off to the decompressor
	_stream->seek(entry.offset);
	return Common::decompressDCL(_stream, entry.compressedSize, entry.uncompressedSize);
}

}	// End of namespace Mohawk
