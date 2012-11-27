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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "teenagent/pack.h"
#include "teenagent/teenagent.h"

#include "common/util.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/substream.h"

namespace TeenAgent {

FilePack::FilePack() : offsets(0) {}

FilePack::~FilePack() {
	close();
}

void FilePack::close() {
	delete[] offsets;
	offsets = NULL;
	file.close();
}

bool FilePack::open(const Common::String &filename) {
	if (!file.exists(filename) || !file.open(filename))
		return false;

	_fileCount = file.readUint32LE();
	debugC(0, kDebugPack, "opened %s, found %u entries", filename.c_str(), _fileCount);
	offsets = new uint32[_fileCount + 1];
	for (uint32 i = 0; i <= _fileCount; ++i) {
		offsets[i] = file.readUint32LE();
	}
	return true;
}

uint32 FilePack::getSize(uint32 id) const {
	if (id < 1 || id > _fileCount)
		return 0;
	return offsets[id] - offsets[id - 1];
}

uint32 FilePack::read(uint32 id, byte *dst, uint32 size) const {
	if (id < 1 || id > _fileCount)
		return 0;

	file.seek(offsets[id - 1]);
	uint32 rsize = offsets[id] - offsets[id - 1];
	uint32 r = file.read(dst, MIN(rsize, size));
	debugC(0, kDebugPack, "read(%u, %u) = %u", id, size, r);
	return r;
}

Common::SeekableReadStream *FilePack::getStream(uint32 id) const {
	if (id < 1 || id > _fileCount)
		return NULL;
	debugC(0, kDebugPack, "stream: %04x-%04x", offsets[id - 1], offsets[id]);
	return new Common::SeekableSubReadStream(&file, offsets[id - 1], offsets[id]);
}

TransientFilePack::TransientFilePack() : offsets(0) {}

TransientFilePack::~TransientFilePack() {
	close();
}

void TransientFilePack::close() {
	delete[] offsets;
	offsets = NULL;
	_filename.clear();
}

bool TransientFilePack::open(const Common::String &filename) {
	_filename = filename;

	Common::File file;
	if (!file.exists(filename) || !file.open(filename))
		return false;

	_fileCount = file.readUint32LE();
	debugC(0, kDebugPack, "opened %s, found %u entries", filename.c_str(), _fileCount);
	offsets = new uint32[_fileCount + 1];
	for (uint32 i = 0; i <= _fileCount; ++i) {
		offsets[i] = file.readUint32LE();
	}
	file.close();
	return true;
}

uint32 TransientFilePack::getSize(uint32 id) const {
	if (id < 1 || id > _fileCount)
		return 0;
	return offsets[id] - offsets[id - 1];
}

uint32 TransientFilePack::read(uint32 id, byte *dst, uint32 size) const {
	if (id < 1 || id > _fileCount)
		return 0;

	Common::File file;
	if (!file.open(_filename))
		return 0;

	file.seek(offsets[id - 1]);
	uint32 rsize = offsets[id] - offsets[id - 1];
	uint32 r = file.read(dst, MIN(rsize, size));
	file.close();
	debugC(0, kDebugPack, "read(%u, %u) = %u", id, size, r);
	return r;
}

Common::SeekableReadStream *TransientFilePack::getStream(uint32 id) const {
	if (id < 1 || id > _fileCount)
		return NULL;
	debugC(0, kDebugPack, "stream: %04x-%04x", offsets[id - 1], offsets[id]);
	Common::File file;
	if (!file.open(_filename))
		return NULL;

	file.seek(offsets[id - 1]);
	uint32 size = offsets[id] - offsets[id - 1];
	byte *ptr = (byte *)malloc(size);
	if (ptr == NULL)
		return NULL;
	uint32 r = file.read(ptr, size);
	file.close();
	return new Common::MemoryReadStream(ptr, r, DisposeAfterUse::YES);
}

void MemoryPack::close() {
	chunks.clear();
}

bool MemoryPack::open(const Common::String &filename) {
	Common::File file;
	if (!file.exists(filename) || !file.open(filename))
		return false;

	uint32 count = file.readUint32LE();
	debugC(0, kDebugPack, "opened %s, found %u entries [memory]", filename.c_str(), count);
	for (uint32 i = 0; i < count; ++i) {
		uint32 offset = file.readUint32LE();
		int32 pos = file.pos();
		uint32 next_offset = file.readUint32LE();
		uint32 size = next_offset - offset;
		Chunk chunk;
		if (size != 0) {
			file.seek(offset);
			chunk.data = new byte[size];
			chunk.size = size;
			file.read(chunk.data, size);
			file.seek(pos);
		}
		chunks.push_back(chunk);
	}
	file.close();
	return true;
}

uint32 MemoryPack::getSize(uint32 id) const {
	--id;
	return id < chunks.size() ? chunks[id].size : 0;
}

uint32 MemoryPack::read(uint32 id, byte *dst, uint32 size) const {
	--id;
	if (id >= chunks.size())
		return 0;
	const Chunk &c = chunks[id];
	memcpy(dst, c.data, c.size);
	return c.size;
}

Common::SeekableReadStream *MemoryPack::getStream(uint32 id) const {
	--id;
	if (id >= chunks.size())
		return 0;
	const Chunk &c = chunks[id];
	return new Common::MemoryReadStream(c.data, c.size, DisposeAfterUse::NO);
}

} // End of namespace TeenAgent
