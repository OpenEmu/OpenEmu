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

#ifndef LASTEXPRESS_HPFARCHIVE_H
#define LASTEXPRESS_HPFARCHIVE_H

/*
	Archive file format (.HPF)

	uint32 {4}   - number of files

	// for each file
	    char {12}    - name (zero-terminated)
	    uint32 {4}   - offset (expressed in sectors of 2048 bytes)
	    uint32 {4}   - size (expressed in sectors of 2048 bytes)
	    byte {2}     - file status: 1 = on disk (ie. in HD.HPF), 0 = on CD
*/

#include "common/archive.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"

namespace LastExpress {

class HPFArchive : public Common::Archive {
public:
	HPFArchive(const Common::String &path);

	bool hasFile(const Common::String &name) const;
	int listMembers(Common::ArchiveMemberList &list) const;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;

	int count() { return _files.size(); }

private:
	static const unsigned int _archiveNameSize = 12;
	static const unsigned int _archiveSectorSize = 2048;

	// File entry
	struct HPFEntry {
		uint32 offset;          ///< Offset (in sectors of 2048 bytes)
		uint32 size;            ///< Size (in sectors of 2048 bytes)
		uint16 isOnHD;          ///< File location (1: on HD; 0: on CD)
	};

	typedef Common::HashMap<Common::String, HPFEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

	FileMap _files;             ///< List of files
	Common::String _filename;   ///< Filename of the archive
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_HPFARCHIVE_H
