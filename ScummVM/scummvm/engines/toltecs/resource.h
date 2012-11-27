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
 *
 */

#ifndef TOLTECS_RESOURCE_H
#define TOLTECS_RESOURCE_H

#include "common/file.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

#include "engines/engine.h"

namespace Toltecs {

const uint kMaxCacheItems = 1024;
const uint kMaxCacheSize = 8 * 1024 * 1024; // 8 MB


class ArchiveReader : public Common::File {
public:
	ArchiveReader();
	~ArchiveReader();

	void openArchive(const char *filename);

	// Returns the size of the opened resource
	uint32 openResource(uint resIndex);
	// Closes the resource
	void closeResource();
	// Returns the size of the resource
	uint32 getResourceSize(uint resIndex);

	void dump(uint resIndex, const char *prefix = NULL);

protected:
	uint32 *_offsets;

};

struct Resource {
	uint32 size;
	byte *data;
};

class ResourceCache {
public:
	ResourceCache(ToltecsEngine *vm);
	~ResourceCache();

	Resource *load(uint resIndex);
	void purgeCache();

protected:
	typedef Common::HashMap<uint, Resource *> ResourceMap;

	ToltecsEngine *_vm;

	ResourceMap _cache;

};


} // End of namespace Toltecs

#endif /* TOLTECS_H */
