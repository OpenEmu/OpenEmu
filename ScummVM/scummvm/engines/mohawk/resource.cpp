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

#include "mohawk/resource.h"

#include "common/debug.h"
#include "common/substream.h"
#include "common/util.h"
#include "common/textconsole.h"

namespace Mohawk {

// Base Archive code

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

// Mohawk Archive code

struct FileTableEntry {
	uint32 offset;
	uint32 size;
	byte flags;
	uint16 unknown;
};

struct NameTableEntry {
	uint16 index;
	Common::String name;
};

bool MohawkArchive::openStream(Common::SeekableReadStream *stream) {
	// Make sure no other file is open...
	close();

	if (stream->readUint32BE() != ID_MHWK) {
		warning("Could not find tag 'MHWK'");
		return false;
	}

	/* uint32 fileSize = */ stream->readUint32BE();

	if (stream->readUint32BE() != ID_RSRC) {
		warning("Could not find tag \'RSRC\'");
		return false;
	}

	uint16 version = stream->readUint16BE();

	if (version != 0x100) {
		warning("Unsupported Mohawk resource version %d.%d", (version >> 8) & 0xff, version & 0xff);
		return false;
	}

	/* uint16 compaction = */ stream->readUint16BE(); // Only used in creation, not in reading
	/* uint32 rsrcSize = */ stream->readUint32BE();
	uint32 absOffset = stream->readUint32BE();
	uint16 fileTableOffset = stream->readUint16BE();
	/* uint16 fileTableSize = */ stream->readUint16BE();

	// First, read in the file table
	stream->seek(absOffset + fileTableOffset);
	Common::Array<FileTableEntry> fileTable;
	fileTable.resize(stream->readUint32BE());

	debug(4, "Reading file table with %d entries", fileTable.size());

	for (uint32 i = 0; i < fileTable.size(); i++) {
		fileTable[i].offset = stream->readUint32BE();
		fileTable[i].size = stream->readUint16BE();
		fileTable[i].size += stream->readByte() << 16; // Get bits 15-24 of size too
		fileTable[i].flags = stream->readByte();
		fileTable[i].unknown = stream->readUint16BE();

		// Add in another 3 bits for file size from the flags.
		// The flags are useless to us except for doing this ;)
		fileTable[i].size += (fileTable[i].flags & 7) << 24;

		debug(4, "File[%02x]: Offset = %08x  Size = %07x  Flags = %02x  Unknown = %04x", i, fileTable[i].offset, fileTable[i].size, fileTable[i].flags, fileTable[i].unknown);
	}

	// Now go in an read in each of the types
	stream->seek(absOffset);
	uint16 stringTableOffset = stream->readUint16BE();
	uint16 typeCount = stream->readUint16BE();

	debug(0, "Name List Offset = %04x  Number of Resource Types = %04x", stringTableOffset, typeCount);

	for (uint16 i = 0; i < typeCount; i++) {
		uint32 tag = stream->readUint32BE();
		uint16 resourceTableOffset = stream->readUint16BE();
		uint16 nameTableOffset = stream->readUint16BE();

		// HACK: Zoombini's SND resource starts will a NULL.
		if (tag == ID_SND)
			debug(3, "Type[%02d]: Tag = \'SND\' ResTable Offset = %04x  NameTable Offset = %04x", i, resourceTableOffset, nameTableOffset);
		else
			debug(3, "Type[%02d]: Tag = \'%s\' ResTable Offset = %04x  NameTable Offset = %04x", i, tag2str(tag), resourceTableOffset, nameTableOffset);

		// Name Table
		stream->seek(absOffset + nameTableOffset);
		Common::Array<NameTableEntry> nameTable;
		nameTable.resize(stream->readUint16BE());

		debug(3, "Names = %04x", nameTable.size());

		for (uint16 j = 0; j < nameTable.size(); j++) {
			uint16 offset = stream->readUint16BE();
			nameTable[j].index = stream->readUint16BE();

			debug(4, "Entry[%02x]: Name List Offset = %04x  Index = %04x", j, offset, nameTable[j].index);

			// Name List
			uint32 pos = stream->pos();
			stream->seek(absOffset + stringTableOffset + offset);
			char c = (char)stream->readByte();
			while (c != 0) {
				nameTable[j].name += c;
				c = (char)stream->readByte();
			}

			debug(3, "Name = \'%s\'", nameTable[j].name.c_str());

			// Get back to next entry
			stream->seek(pos);
		}

		// Resource Table
		stream->seek(absOffset + resourceTableOffset);
		uint16 resourceCount = stream->readUint16BE();

		debug(3, "Resource count = %04x", resourceCount);

		ResourceMap &resMap = _types[tag];

		for (uint16 j = 0; j < resourceCount; j++) {
			uint16 id = stream->readUint16BE();
			uint16 index = stream->readUint16BE();

			Resource &res = resMap[id];

			// Pull out the name from the name table
			for (uint32 k = 0; k < nameTable.size(); k++) {
				if (nameTable[k].index == index) {
					res.name = nameTable[k].name;
					break;
				}
			}

			// Pull out our offset/size too
			res.offset = fileTable[index - 1].offset;

			// WORKAROUND: tMOV resources pretty much ignore the size part of the file table,
			// as the original just passed the full Mohawk file to QuickTime and the offset.
			// We need to do this because of the way Mohawk is set up (this is much more "proper"
			// than passing _stream at the right offset). We may want to do that in the future, though.
			if (tag == ID_TMOV) {
				if (index == fileTable.size())
					res.size = stream->size() - fileTable[index - 1].offset;
				else
					res.size = fileTable[index].offset - fileTable[index - 1].offset;
			} else
				res.size = fileTable[index - 1].size;

			debug(4, "Entry[%02x]: ID = %04x (%d) Index = %04x", j, id, id, index);
		}

		// Return to next TypeTable entry
		stream->seek(absOffset + (i + 1) * 8 + 4);

		debug(3, "\n");
	}

	_stream = stream;
	return true;
}

// Living Books Archive code

bool LivingBooksArchive_v1::openStream(Common::SeekableReadStream *stream) {
	close();

	// This is for the "old" Mohawk resource format used in some older
	// Living Books. It is very similar, just missing the MHWK tag and
	// some other minor differences, especially with the file table
	// being merged into the resource table.

	uint32 headerSize = stream->readUint32BE();

	// NOTE: There are differences besides endianness! (Subtle changes,
	// but different).

	if (headerSize == 6) { // We're in Big Endian mode (Macintosh)
		stream->readUint16BE(); // Resource Table Size
		uint16 typeCount = stream->readUint16BE();

		debug(0, "Old Mohawk File (Macintosh): Number of Resource Types = %04x", typeCount);

		for (uint16 i = 0; i < typeCount; i++) {
			uint32 tag = stream->readUint32BE();
			uint32 resourceTableOffset = stream->readUint32BE() + 6;
			stream->readUint32BE(); // Unknown (always 0?)

			debug(3, "Type[%02d]: Tag = \'%s\'  ResTable Offset = %04x", i, tag2str(tag), resourceTableOffset);

			uint32 oldPos = stream->pos();

			// Resource Table/File Table
			stream->seek(resourceTableOffset);
			uint16 resourceCount = stream->readUint16BE();

			ResourceMap &resMap = _types[tag];

			for (uint16 j = 0; j < resourceCount; j++) {
				uint16 id = stream->readUint16BE();

				Resource &res = resMap[id];

				res.offset = stream->readUint32BE();
				res.size = stream->readByte() << 16;
				res.size |= stream->readUint16BE();
				stream->skip(5); // Unknown (always 0?)

				debug(4, "Entry[%02x]: ID = %04x (%d)\tOffset = %08x, Size = %08x", j, id, id, res.offset, res.size);
			}

			stream->seek(oldPos);
			debug(3, "\n");
		}
	} else if (SWAP_BYTES_32(headerSize) == 6) { // We're in Little Endian mode (Windows)
		stream->readUint16LE(); // Resource Table Size
		uint16 typeCount = stream->readUint16LE();

		debug(0, "Old Mohawk File (Windows): Number of Resource Types = %04x", typeCount);

		for (uint16 i = 0; i < typeCount; i++) {
			uint32 tag = stream->readUint32LE();
			uint16 resourceTableOffset = stream->readUint16LE() + 6;
			stream->readUint16LE(); // Unknown (always 0?)

			debug(3, "Type[%02d]: Tag = \'%s\'  ResTable Offset = %04x", i, tag2str(tag), resourceTableOffset);

			uint32 oldPos = stream->pos();

			// Resource Table/File Table
			stream->seek(resourceTableOffset);
			uint16 resourceCount = stream->readUint16LE();

			ResourceMap &resMap = _types[tag];

			for (uint16 j = 0; j < resourceCount; j++) {
				uint16 id = stream->readUint16LE();

				Resource &res = resMap[id];

				res.offset = stream->readUint32LE();
				res.size = stream->readUint32LE();
				stream->readUint16LE(); // Unknown (always 0?)

				debug(4, "Entry[%02x]: ID = %04x (%d)\tOffset = %08x, Size = %08x", j, id, id, res.offset, res.size);
			}

			stream->seek(oldPos);
			debug(3, "\n");
		}
	} else {
		// Not a valid Living Books Archive
		return false;
	}

	_stream = stream;
	return true;
}


// DOS Archive (v2) code
// Partially based on the Prince of Persia Format Specifications
// See http://sdfg.com.ar/git/?p=fp-git.git;a=blob;f=FP/doc/FormatSpecifications
// However, I'm keeping with the terminology we've been using with the
// later archive formats.

bool DOSArchive_v2::openStream(Common::SeekableReadStream *stream) {
	close();

	uint32 typeTableOffset = stream->readUint32LE();
	uint16 typeTableSize = stream->readUint16LE();

	if (typeTableOffset + typeTableSize != (uint32)stream->size())
		return false;

	stream->seek(typeTableOffset);

	uint16 typeCount = stream->readUint16LE();

	for (uint16 i = 0; i < typeCount; i++) {
		uint32 tag = stream->readUint32LE();
		uint16 resourceTableOffset = stream->readUint16LE();

		debug(3, "Type[%02d]: Tag = \'%s\'  ResTable Offset = %04x", i, tag2str(tag), resourceTableOffset);

		uint32 oldPos = stream->pos();

		// Resource Table/File Table
		stream->seek(resourceTableOffset + typeTableOffset);
		uint16 resourceCount = stream->readUint16LE();

		ResourceMap &resMap = _types[tag];

		for (uint16 j = 0; j < resourceCount; j++) {
			uint16 id = stream->readUint16LE();

			Resource &res = resMap[id];
			res.offset = stream->readUint32LE() + 1; // Need to add one to the offset to skip the checksum byte
			res.size = stream->readUint16LE();
			stream->skip(3); // Skip (useless) flags

			debug(4, "Entry[%02x]: ID = %04x (%d)\tOffset = %08x, Size = %08x", j, id, id, res.offset, res.size);
		}

		stream->seek(oldPos);
		debug(3, "\n");
	}

	_stream = stream;
	return true;
}

}	// End of namespace Mohawk
