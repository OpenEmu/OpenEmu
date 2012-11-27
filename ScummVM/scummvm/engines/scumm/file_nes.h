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
 *
 */

#ifndef SCUMM_FILE_NES_H
#define SCUMM_FILE_NES_H

#include "scumm/file.h"

namespace Scumm {

class ScummNESFile : public BaseScummFile {
public:
	enum ROMset {
		kROMsetUSA,
		kROMsetEurope,
		kROMsetSweden,
		kROMsetFrance,
		kROMsetGermany,
		kROMsetSpain,
		kROMsetItaly,
		kROMsetNum
	};

	struct Resource;
	struct ResourceGroup;
	struct LFLEntry;
	struct LFL;

	enum ResType {
		NES_UNKNOWN,
		NES_GLOBDATA,
		NES_ROOM,
		NES_SCRIPT,
		NES_SOUND,
		NES_COSTUME,
		NES_ROOMGFX,
		NES_COSTUMEGFX,
		NES_SPRPALS,
		NES_SPRDESC,
		NES_SPRLENS,
		NES_SPROFFS,
		NES_SPRDATA,
		NES_CHARSET,
		NES_PREPLIST
	};


private:
	Common::SeekableReadStream *_stream;
	ROMset _ROMset;
	byte *_buf;

	bool generateIndex();
	bool generateResource(int res);
	uint16 extractResource(Common::WriteStream *out, const Resource *res, ResType type);

	byte fileReadByte();
	uint16 fileReadUint16LE();

public:
	ScummNESFile();

	bool open(const Common::String &filename);
	bool openSubFile(const Common::String &filename);

	void close();
	bool eos() const { return _stream->eos(); }
	int32 pos() const { return _stream->pos(); }
	int32 size() const { return _stream->size(); }
	bool seek(int32 offs, int whence = SEEK_SET) { return _stream->seek(offs, whence); }
	uint32 read(void *dataPtr, uint32 dataSize);
};

} // End of namespace Scumm

#endif
