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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef RING_RESOURCES_H
#define RING_RESOURCES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/file.h"
#include "common/list.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/util.h"
#include "graphics/surface.h"

namespace TsAGE {

// Magic number used by original game to identify valid memory blocks
const uint32 MEMORY_ENTRY_ID = 0xE11DA722;

const int MEMORY_POOL_SIZE = 1000;

enum ResourceType { RES_LIBRARY, RES_STRIP, RES_IMAGE, RES_PALETTE, RES_VISAGE, RES_SOUND, RES_MESSAGE,
		RES_FONT, RES_POINTER, RES_BANK, RES_SND_DRIVER, RES_PRIORITY, RES_CONTROL, RES_WALKRGNS,
		RES_BITMAP, RES_SAVE, RES_SEQUENCE,
		// Return to Ringworld specific resource types
		RT17, RT18, RT19, RT20, RT21, RT22, RT23, RT24, RT25, RT26, RT27, RT28, RT29, RT30, RT31
};

class MemoryHeader {
public:
	uint32 id;
	int16 index;
	int lockCtr;
	int criticalCtr;
	uint8 tag;
	uint32 size;

	MemoryHeader() {
		id = 0;
		index = 0;
		lockCtr = 0;
		criticalCtr = 0;
		tag = 0;
		size = 0;
	}
};

class SectionEntry {
public:
	ResourceType resType;
	uint16 resNum;
	uint32 fileOffset;

	SectionEntry() {
		resType = RES_LIBRARY;
		resNum = 0;
		fileOffset = 0;
	}
};

class ResourceEntry {
public:
	uint16 id;
	bool isCompressed;
	uint32 fileOffset;
	uint32 size;
	uint32 uncompressedSize;

	ResourceEntry() {
		id = 0;
		isCompressed = false;
		fileOffset = 0;
		size = 0;
		uncompressedSize = 0;
	}
};

typedef Common::List<ResourceEntry> ResourceList;

class SectionList : public Common::List<SectionEntry> {
public:
	uint32 fileOffset;

	SectionList() {
		fileOffset = 0;
	}
};

class MemoryManager {
private:
	MemoryHeader **_memoryPool;
public:
	MemoryManager();
	~MemoryManager();

	uint16 allocate(uint32 size);
	byte *allocate2(uint32 size);
	byte *lock(uint32 handle);
	int indexOf(const byte *p);
	void deallocate(const byte *p);
	void deallocate(uint16 handle) { warning("TODO: MemoryManager::deallocate(handle)"); }
	uint32 getSize(const byte *p);
	void incLocks(const byte *p);
};

class BitReader {
private:
	Common::ReadStream &_stream;
	uint8 _remainder, _bitsLeft;
	byte readByte() { return _stream.eos() ? 0 : _stream.readByte(); }
public:
	BitReader(Common::ReadStream &s) : _stream(s) {
		numBits = 9;
		_remainder = 0;
		_bitsLeft = 0;
	}
	uint16 readToken();

	int numBits;
};

class TLib {
private:
	Common::StringArray _resStrings;
	MemoryManager &_memoryManager;
private:
	Common::File _file;
	Common::String _filename;
	ResourceList _resources;
	SectionList _sections;

	void loadSection(uint32 fileOffset);
	void loadIndex();
public:
	TLib(MemoryManager &memManager, const Common::String &filename);
	~TLib();

	const Common::String &getFilename() { return _filename; }
	const SectionList &getSections() { return _sections; }
	byte *getResource(uint16 id, bool suppressErrors = false);
	byte *getResource(ResourceType resType, uint16 resNum, uint16 rlbNum, bool suppressErrors = false);
	uint32 getResourceStart(ResourceType resType, uint16 resNum, uint16 rlbNum, ResourceEntry &entry);
	bool getPalette(int paletteNum, byte *palData, uint *startNum, uint *numEntries);
	byte *getSubResource(int resNum, int rlbNum, int index, uint *size, bool suppressErrors = false);
	bool getMessage(int resNum, int lineNum, Common::String &result, bool suppressErrors = false);
};

class ResourceManager {
private:
	Common::Array<TLib *> _libList;
public:
	~ResourceManager();

	void addLib(const Common::String &libName);

	byte *getResource(uint16 id, bool suppressErrors = false);
	byte *getResource(ResourceType resType, uint16 resNum, uint16 rlbNum, bool suppressErrors = false);
	void getPalette(int paletteNum, byte *palData, uint *startNum, uint *numEntries, bool suppressErrors = false);
	byte *getSubResource(int resNum, int rlbNum, int index, uint *size, bool suppressErrors = false);
	Common::String getMessage(int resNum, int lineNum, bool suppressErrors = false);
	TLib &first() { return **_libList.begin(); }

	static bool scanIndex(Common::File &f, ResourceType resType, int rlbNum, int resNum, ResourceEntry &resEntry);
	static void loadSection(Common::File &f, ResourceList &resources);
};


} // end of namespace TsAGE

#endif
