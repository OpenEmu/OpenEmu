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

#ifndef GOB_DATAIO_H
#define GOB_DATAIO_H

#include "common/endian.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/array.h"
#include "common/file.h"

namespace Common {
class SeekableReadStream;
}

namespace Gob {

struct ArchiveInfo {
	Common::String name;
	bool base;
	uint32 fileCount;
};

class DataIO {
public:
	DataIO();
	~DataIO();

	void getArchiveInfo(Common::Array<ArchiveInfo> &info) const;

	bool openArchive(Common::String name, bool base);
	bool closeArchive(bool base);

	bool hasFile(const Common::String &name);

	int32 fileSize(const Common::String &name);

	Common::SeekableReadStream *getFile(const Common::String &name);
	byte *getFile(const Common::String &name, int32 &size);

	static byte *unpack(const byte *src, uint32 srcSize, int32 &size, uint8 compression = 1);
	static Common::SeekableReadStream *unpack(Common::SeekableReadStream &src, uint8 compression = 1);

private:
	static const int kMaxArchives = 8;

	struct Archive;

	struct File {
		Common::String name;
		uint32 size;
		uint32 offset;
		uint8  compression;

		Archive *archive;

		File();
		File(const Common::String &n, uint32 s, uint32 o, uint8 c, Archive &a);
	};

	typedef Common::HashMap<Common::String, File, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

	struct Archive {
		Common::String name;
		Common::File   file;

		FileMap files;

		bool base;
	};

	Common::Array<Archive *> _archives;

	Archive *openArchive(const Common::String &name);
	bool closeArchive(Archive &archive);

	File *findFile(const Common::String &name);

	Common::SeekableReadStream *getFile(File &file);
	byte *getFile(File &file, int32 &size);

	static byte *unpack(Common::SeekableReadStream &src, int32 &size, uint8 compression, bool useMalloc);

	static uint32 getSizeChunks(Common::SeekableReadStream &src);

	static void unpackChunks(Common::SeekableReadStream &src, byte *dest, uint32 size);
	static void unpackChunk (Common::SeekableReadStream &src, byte *dest, uint32 size);
};

} // End of namespace Gob

#endif // GOB_DATAIO_H
