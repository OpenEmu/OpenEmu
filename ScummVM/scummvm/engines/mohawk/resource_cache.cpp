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
#include "mohawk/myst.h"
#include "mohawk/resource_cache.h"

namespace Mohawk {

ResourceCache::ResourceCache() {
	enabled = true;
}

ResourceCache::~ResourceCache() {
	clear();
}

void ResourceCache::clear() {
	if (!enabled)
		return;

	debugC(kDebugCache, "Clearing Cache...");

	for (uint32 i = 0; i < _store.size(); i++)
		delete _store[i].data;

	_store.clear();
}

void ResourceCache::add(uint32 tag, uint16 id, Common::SeekableReadStream *data) {
	if (!enabled)
		return;

	debugC(kDebugCache, "Adding item %d - tag 0x%04X id %d", _store.size(), tag, id);

	DataObject current;
	current.tag = tag;
	current.id = id;
	uint32 dataCurPos = data->pos();
	current.data = data->readStream(data->size());
	data->seek(dataCurPos);
	_store.push_back(current);
}

// Returns NULL if not found
Common::SeekableReadStream *ResourceCache::search(uint32 tag, uint16 id) {
	if (!enabled)
		return NULL;

	debugC(kDebugCache, "Searching for tag 0x%04X id %d", tag, id);

	for (uint32 i = 0; i < _store.size(); i++) {
		if (tag == _store[i].tag && id == _store[i].id) {
			debugC(kDebugCache, "Found cached tag 0x%04X id %u", tag, id);
			uint32 dataCurPos  = _store[i].data->pos();
			Common::SeekableReadStream *ret = _store[i].data->readStream(_store[i].data->size());
			_store[i].data->seek(dataCurPos);
			return ret;
		}
	}

	debugC(kDebugCache, "tag 0x%04X id %d not found", tag, id);
	return NULL;
}

} // End of namespace Mohawk
