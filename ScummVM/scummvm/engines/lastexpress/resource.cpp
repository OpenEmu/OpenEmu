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

#include "lastexpress/resource.h"

#include "lastexpress/data/background.h"
#include "lastexpress/data/cursor.h"
#include "lastexpress/data/font.h"

#include "lastexpress/debug.h"
#include "lastexpress/helpers.h"

#include "common/debug.h"
#include "common/file.h"

namespace LastExpress {

const char *archiveDemoPath = "demo.hpf";
const char *archiveHDPath = "hd.hpf";
const char *archiveCD1Path = "cd1.hpf";
const char *archiveCD2Path = "cd2.hpf";
const char *archiveCD3Path = "cd3.hpf";

ResourceManager::ResourceManager(bool isDemo) : _isDemo(isDemo) {
}

ResourceManager::~ResourceManager() {
	reset();
}

bool ResourceManager::isArchivePresent(ArchiveIndex type) {
	switch (type) {
	default:
	case kArchiveAll:
		error("[ResourceManager::isArchivePresent] Only checks for single CDs are valid");

	case kArchiveCd1:
		return Common::File::exists(archiveCD1Path);

	case kArchiveCd2:
		return Common::File::exists(archiveCD2Path);

	case kArchiveCd3:
		return Common::File::exists(archiveCD3Path);
	}
}

// Load a specific archive collection
//  - type is ignored in the demo version
//  - use ArchiveAll to load all three cds
//  - HD.hpf is always loaded along with the selected archive(s)
//  - will remove all other archives
bool ResourceManager::loadArchive(ArchiveIndex type) {
	// Unload all archives
	reset();

	// Demo version
	if (_isDemo)
		return loadArchive(archiveDemoPath);

	// Load HD
	if (!loadArchive(archiveHDPath))
		return false;

	switch(type) {
	case kArchiveCd1:
		return loadArchive(archiveCD1Path);

	case kArchiveCd2:
		return loadArchive(archiveCD2Path);

	case kArchiveCd3:
		return loadArchive(archiveCD3Path);

	case kArchiveAll:
	default:
		if (loadArchive(archiveCD1Path))
			if (loadArchive(archiveCD2Path))
				return loadArchive(archiveCD3Path);
		break;
	}

	return false;
}

void ResourceManager::reset() {
	// Free the loaded archives
	for (Common::Array<HPFArchive *>::iterator it = _archives.begin(); it != _archives.end(); ++it)
		SAFE_DELETE(*it);

	_archives.clear();
}

bool ResourceManager::loadArchive(const Common::String &name) {
	HPFArchive *archive = new HPFArchive(name);

	if (archive->count() == 0) {
		debugC(2, kLastExpressDebugResource, "Error opening archive: %s", name.c_str());

		delete archive;

		return false;
	}

	_archives.push_back(archive);

	return true;
}

// Get a stream to file in the archive
//  - same as createReadStreamForMember except it checks if the file exists and will assert / output a debug message if not
Common::SeekableReadStream *ResourceManager::getFileStream(const Common::String &name) const {

	// Check if the file exits in the archive
	if (!hasFile(name)) {
		debugC(2, kLastExpressDebugResource, "Error opening file: %s", name.c_str());
		return NULL;
	}

	debugC(2, kLastExpressDebugResource, "Opening file: %s", name.c_str());

	return createReadStreamForMember(name);
}

//////////////////////////////////////////////////////////////////////////
// Archive functions
//////////////////////////////////////////////////////////////////////////
bool ResourceManager::hasFile(const Common::String &name) const {
	for (Common::Array<HPFArchive *>::const_iterator it = _archives.begin(); it != _archives.end(); ++it) {
		if ((*it)->hasFile(name))
			return true;
	}

	return false;
}

int ResourceManager::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (Common::Array<HPFArchive *>::const_iterator it = _archives.begin(); it != _archives.end(); ++it) {

		Common::ArchiveMemberList members;
		count += (*it)->listMembers(members);

		list.insert(list.end(), members.begin(), members.end());
	}

	return count;
}

const Common::ArchiveMemberPtr ResourceManager::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *ResourceManager::createReadStreamForMember(const Common::String &name) const {
	for (Common::Array<HPFArchive *>::const_iterator it = _archives.begin(); it != _archives.end(); ++it) {

		Common::SeekableReadStream *stream = (*it)->createReadStreamForMember(name);

		if (stream)
			return stream;
	}

	return NULL;
}


// Resource loading

Background *ResourceManager::loadBackground(const Common::String &name) const {
	// Open the resource
	Common::SeekableReadStream *stream = createReadStreamForMember(name + ".bg");
	if (!stream)
		return NULL;

	// Create the new background & load the data
	Background *bg = new Background();
	if (!bg->load(stream)) {
		delete bg;
		// stream should be freed by the Background instance
		return NULL;
	}

	return bg;
}

Cursor *ResourceManager::loadCursor() const {
	// Open the resource
	Common::SeekableReadStream *stream = createReadStreamForMember("cursors.tbm");
	if (!stream)
		return NULL;

	// Create the new background
	Cursor *c = new Cursor();
	if (!c->load(stream)) {
		delete c;
		// stream should be freed by the Cursor instance
		return NULL;
	}

	return c;
}

Font *ResourceManager::loadFont() const {
	// Open the resource
	Common::SeekableReadStream *stream = createReadStreamForMember("font.dat");
	if (!stream)
		return NULL;

	// Create the new background
	Font *f = new Font();
	if (!f->load(stream)) {
		delete f;
		// stream should be freed by the Font instance
		return NULL;
	}

	return f;
}

} // End of namespace LastExpress
