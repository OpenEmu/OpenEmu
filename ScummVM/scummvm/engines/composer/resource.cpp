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

#include "composer/resource.h"

#include "common/debug.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/util.h"
#include "common/textconsole.h"

namespace Composer {

// Base Archive code
// (copied from clone2727's mohawk code)

Archive::Archive() {
	_stream = 0;
}

Archive::~Archive() {
	close();
}

bool Archive::openFile(const Common::String &fileName) {
	Common::File *file = new Common::File();

	if (!file->open(fileName)) {
		delete file;
		return false;
	}

	if (!openStream(file)) {
		close();
		return false;
	}

	return true;
}

void Archive::close() {
	_types.clear();
	delete _stream; _stream = 0;
}

bool Archive::hasResource(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		return false;

	return _types[tag].contains(id);
}

bool Archive::hasResource(uint32 tag, const Common::String &resName) const {
	if (!_types.contains(tag) || resName.empty())
		return false;

	const ResourceMap &resMap = _types[tag];

	for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++)
		if (it->_value.name.matchString(resName))
			return true;

	return false;
}

Common::SeekableReadStream *Archive::getResource(uint32 tag, uint16 id) {
	if (!_types.contains(tag))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const Resource &res = resMap[id];

	return new Common::SeekableSubReadStream(_stream, res.offset, res.offset + res.size);
}

uint32 Archive::getResourceFlags(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const Resource &res = resMap[id];

	return res.flags;
}

uint32 Archive::getOffset(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	return resMap[id].offset;
}

uint16 Archive::findResourceID(uint32 tag, const Common::String &resName) const {
	if (!_types.contains(tag) || resName.empty())
		return 0xFFFF;

	const ResourceMap &resMap = _types[tag];

	for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++)
		if (it->_value.name.matchString(resName))
			return it->_key;

	return 0xFFFF;
}

Common::String Archive::getName(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	return resMap[id].name;
}

Common::Array<uint32> Archive::getResourceTypeList() const {
	Common::Array<uint32> typeList;

	for (TypeMap::const_iterator it = _types.begin(); it != _types.end(); it++)
		typeList.push_back(it->_key);

	return typeList;
}

Common::Array<uint16> Archive::getResourceIDList(uint32 type) const {
	Common::Array<uint16> idList;

	if (!_types.contains(type))
		return idList;

	const ResourceMap &resMap = _types[type];

	for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++)
		idList.push_back(it->_key);

	return idList;
}

// Composer Archive code

bool ComposerArchive::openStream(Common::SeekableReadStream *stream) {
	// Make sure no other file is open...
	close();

	bool newStyle = false;
	uint32 headerSize = stream->readUint32LE();
	if (headerSize == SWAP_CONSTANT_32(ID_LBRC)) {
		// new-style file
		newStyle = true;
		headerSize = stream->readUint32LE();
		uint32 zeros = stream->readUint32LE();
		if (zeros != 0)
			error("invalid LBRC header (%d instead of zeros)", zeros);
	}

	uint16 numResourceTypes = stream->readUint16LE();
	if (newStyle) {
		uint16 unknown = stream->readUint16LE();
		debug(4, "skipping unknown %04x", unknown);
	}

	debug(4, "Reading LBRC resource table with %d entries", numResourceTypes);
	for (uint i = 0; i < numResourceTypes; i++) {
		uint32 tag = stream->readUint32BE();
		uint32 tableOffset = stream->readUint32LE();
		debug(4, "Type '%s' at offset %d", tag2str(tag), tableOffset);
		// starting from the start of the resource table, which differs
		// according to whether we have the 10 extra bytes for newStyle
		if (newStyle)
			tableOffset += 16;
		else
			tableOffset += 6;

		ResourceMap &resMap = _types[tag];

		uint32 oldPos = stream->pos();
		stream->seek(tableOffset);

		while (true) {
			if (stream->eos())
				error("LBRC file ran out of stream");

			uint32 offset, size, id, flags;
			if (newStyle) {
				offset = stream->readUint32LE();
				if (!offset)
					break;
				size = stream->readUint32LE();
				id = stream->readUint16LE();
				flags = stream->readUint16LE(); // set to 1 for preload, otherwise no preload
				/*uint32 junk = */ stream->readUint32LE();
			} else {
				id = stream->readUint16LE();
				if (!id)
					break;
				offset = stream->readUint32LE();
				offset += headerSize;
				size = stream->readUint32LE();
				flags = stream->readUint16LE(); // FIXME

			}

			Resource &res = resMap[id];
			res.offset = offset;
			res.size = size;
			res.flags = flags;
			debug(4, "Id %d, offset %d, size %d, flags %08x", id, offset, size, flags);
		}

		stream->seek(oldPos);
	}

	_stream = stream;
	return true;
}

Pipe::Pipe(Common::SeekableReadStream *stream) {
	_offset = 0;
	_stream = stream;
	_anim = NULL;
}

Pipe::~Pipe() {
}

void Pipe::nextFrame() {
	if (_offset == (uint)_stream->size())
		return;

	_stream->seek(_offset, SEEK_SET);

	uint32 tagCount = _stream->readUint32LE();
	_offset += 4;
	for (uint i = 0; i < tagCount; i++) {
		uint32 tag = _stream->readUint32BE();
		uint32 count = _stream->readUint32LE();
		_offset += 8;

		ResourceMap &resMap = _types[tag];

		_offset += (12 * count);
		for (uint j = 0; j < count; j++) {
			uint32 offset = _stream->readUint32LE();
			uint32 size = _stream->readUint32LE();
			uint16 id = _stream->readUint16LE();
			uint32 unknown = _stream->readUint16LE(); // frame id?
			debug(9, "pipe: %s/%d: offset %d, size %d, unknown %d", tag2str(tag), id, offset, size, unknown);

			PipeResourceEntry entry;
			entry.size = size;
			entry.offset = _offset;
			resMap[id].entries.push_back(entry);

			_offset += size;
		}
		_stream->seek(_offset, SEEK_SET);
	}
}

bool Pipe::hasResource(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		return false;

	return _types[tag].contains(id);
}

Common::SeekableReadStream *Pipe::getResource(uint32 tag, uint16 id, bool buffering) {
	if (!_types.contains(tag))
		error("Pipe does not contain '%s' %04x", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive does not contain '%s' %04x", tag2str(tag), id);

	const PipeResource &res = resMap[id];

	if (res.entries.size() == 1) {
		Common::SeekableReadStream *stream = new Common::SeekableSubReadStream(_stream,
			res.entries[0].offset, res.entries[0].offset + res.entries[0].size);
		if (buffering)
			_types[tag].erase(id);
		return stream;
	}

	// If there are multiple entries in the pipe, we have to concaternate them together.

	uint32 size = 0;
	for (uint i = 0; i < res.entries.size(); i++)
		size += res.entries[i].size;

	byte *buffer = (byte *)malloc(size);
	uint32 offset = 0;
	for (uint i = 0; i < res.entries.size(); i++) {
		_stream->seek(res.entries[i].offset, SEEK_SET);
		_stream->read(buffer + offset, res.entries[i].size);
		offset += res.entries[i].size;
	}
	if (buffering)
		_types[tag].erase(id);
	return new Common::MemoryReadStream(buffer, size, DisposeAfterUse::YES);
}

OldPipe::OldPipe(Common::SeekableReadStream *stream) : Pipe(stream), _currFrame(0) {
	uint32 tag = _stream->readUint32BE();
	if (tag != ID_PIPE)
		error("invalid tag for pipe (%08x)", tag);

	_numFrames = _stream->readUint32LE();
	uint16 scriptCount = _stream->readUint16LE();
	_scripts.reserve(scriptCount);
	for (uint i = 0; i < scriptCount; i++)
		_scripts.push_back(_stream->readUint16LE());

	_offset = _stream->pos();
}

void OldPipe::nextFrame() {
	if (_currFrame >= _numFrames)
		return;

	_stream->seek(_offset, SEEK_SET);

	uint32 tag = _stream->readUint32BE();
	if (tag != ID_FRME)
		error("invalid tag for pipe (%08x)", tag);

	uint16 spriteCount = _stream->readUint16LE();
	uint32 spriteSize = _stream->readUint32LE();
	uint32 audioSize = _stream->readUint32LE();

	Common::Array<uint16> spriteIds;
	Common::Array<PipeResourceEntry> spriteEntries;
	for (uint i = 0; i < spriteCount; i++) {
		spriteIds.push_back(_stream->readUint16LE());
		PipeResourceEntry entry;
		entry.size = _stream->readUint32LE();
		entry.offset = _stream->readUint32LE();
		spriteEntries.push_back(entry);
	}

	uint32 spriteDataOffset = _stream->pos();
	_stream->skip(spriteSize);

	ResourceMap &spriteResMap = _types[ID_BMAP];
	spriteResMap.clear();
	for (uint i = 0; i < spriteIds.size(); i++) {
		PipeResourceEntry &entry = spriteEntries[i];
		entry.offset += spriteDataOffset;
		spriteResMap[spriteIds[i]].entries.push_back(entry);
	}

	ResourceMap &audioResMap = _types[ID_WAVE];
	audioResMap.clear();

	if (audioSize > 0) {
		PipeResourceEntry entry;
		entry.size = audioSize;
		entry.offset = _stream->pos();
		// we use 0xffff for per-frame pipe audio
		audioResMap[0xffff].entries.push_back(entry);
		_stream->skip(audioSize);
	}

	_offset = _stream->pos();
	_currFrame++;
}

}	// End of namespace Composer
