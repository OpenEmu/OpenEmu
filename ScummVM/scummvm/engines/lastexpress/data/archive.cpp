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

// Based on the Xentax Wiki documentation:
// http://wiki.xentax.com/index.php/The_Last_Express_SND

#include "lastexpress/data/archive.h"

#include "lastexpress/debug.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"

namespace LastExpress {

HPFArchive::HPFArchive(const Common::String &path) {
	_filename = path;

	// Open a stream to the archive
	Common::SeekableReadStream *archive = SearchMan.createReadStreamForMember(_filename);
	if (!archive) {
		debugC(2, kLastExpressDebugResource, "Error opening file: %s", path.c_str());
		return;
	}

	debugC(2, kLastExpressDebugResource, "Opened archive: %s", path.c_str());

	// Read header to get the number of files
	uint32 numFiles = archive->readUint32LE();
	debugC(3, kLastExpressDebugResource, "Number of files in archive: %d", numFiles);

	// Read the list of files
	for (unsigned int i = 0; i < numFiles; ++i) {
		char name[13];
		HPFEntry entry;

		archive->read(&name, sizeof(char) * _archiveNameSize);
		entry.offset = archive->readUint32LE();
		entry.size = archive->readUint32LE();
		entry.isOnHD = archive->readUint16LE();

		// Terminate string
		name[12] = '\0';

		Common::String filename(name);
		filename.toLowercase();

		_files[filename] = entry;

		//debugC(9, kLastExpressDebugResource, "File entry: %s (offset:%d - Size: %d - HD: %u)", &name, entry.offset, entry.size, entry.isOnHD);
	}

	// Close stream
	delete archive;
}

bool HPFArchive::hasFile(const Common::String &name) const {
	return (_files.find(name) != _files.end());
}

int HPFArchive::listMembers(Common::ArchiveMemberList &list) const {
	int numMembers = 0;

	for (FileMap::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, this)));
		numMembers++;
	}

	return numMembers;
}

const Common::ArchiveMemberPtr HPFArchive::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *HPFArchive::createReadStreamForMember(const Common::String &name) const {
	FileMap::const_iterator fDesc = _files.find(name);
	if (fDesc == _files.end())
		return NULL;

	Common::File *archive = new Common::File();
	if (!archive->open(_filename)) {
		delete archive;
		return NULL;
	}

	return new Common::SeekableSubReadStream(archive, fDesc->_value.offset * _archiveSectorSize, fDesc->_value.offset * _archiveSectorSize + fDesc->_value.size * _archiveSectorSize, DisposeAfterUse::YES);
}

} // End of namespace LastExpress
