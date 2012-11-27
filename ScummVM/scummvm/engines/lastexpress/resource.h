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

#ifndef LASTEXPRESS_RESOURCE_H
#define LASTEXPRESS_RESOURCE_H

#include "lastexpress/data/archive.h"
#include "lastexpress/shared.h"

#include "common/array.h"

namespace LastExpress {

class Background;
class Cursor;
class Font;

class ResourceManager : public Common::Archive {
public:
	ResourceManager(bool demo);
	~ResourceManager();

	// Loading
	bool loadArchive(ArchiveIndex type);
	static bool isArchivePresent(ArchiveIndex type);
	Common::SeekableReadStream *getFileStream(const Common::String &name) const;

	// Archive functions
	bool hasFile(const Common::String &name) const;
	int listMembers(Common::ArchiveMemberList &list) const;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;

	// Resource loading
	Background *loadBackground(const Common::String &name) const;
	Cursor *loadCursor() const;
	Font *loadFont() const;

private:
	bool _isDemo;

	bool loadArchive(const Common::String &name);
	void reset();

	Common::Array<HPFArchive *> _archives;

	friend class Debugger;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_RESOURCE_H
