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

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/stack.h"
#include "common/util.h"
#include "tsage/resources.h"
#include "tsage/tsage.h"

namespace TsAGE {


MemoryManager::MemoryManager() {
	_memoryPool = new MemoryHeader*[MEMORY_POOL_SIZE];
	Common::fill(&_memoryPool[0], &_memoryPool[MEMORY_POOL_SIZE], (MemoryHeader *)NULL);
}

MemoryManager::~MemoryManager() {
	for (int i = 0; i < MEMORY_POOL_SIZE; ++i) {
		if (_memoryPool[i] != NULL)
			free(_memoryPool[i]);
	}
	delete[] _memoryPool;
}

uint16 MemoryManager::allocate(uint32 size) {
	int idx = 0;
	while ((idx < MEMORY_POOL_SIZE) && (_memoryPool[idx] != NULL))
		++idx;
	if (idx == MEMORY_POOL_SIZE)
		error("Out of memory handles");

	// Create the new entry
	_memoryPool[idx] = (MemoryHeader *)malloc(sizeof(MemoryHeader) + size);
	_memoryPool[idx]->id = MEMORY_ENTRY_ID;
	_memoryPool[idx]->index = idx;
	_memoryPool[idx]->lockCtr = 0;
	_memoryPool[idx]->criticalCtr = 0;
	_memoryPool[idx]->tag = 0;
	_memoryPool[idx]->size = size;

	// Return it's index
	return idx;
}

byte *MemoryManager::allocate2(uint32 size) {
	uint32 idx = allocate(size);
	byte *result = lock(idx);
	Common::fill(result, result + size, 0);
	return result;
}

byte *MemoryManager::lock(uint32 handle) {
	assert((int)handle < MEMORY_POOL_SIZE);
	return (byte *)_memoryPool[handle] + sizeof(MemoryHeader);
}

int MemoryManager::indexOf(const byte *p) {
	for (int idx = 0; idx < MEMORY_POOL_SIZE; ++idx) {
		if (((byte *)_memoryPool[idx] + sizeof(MemoryHeader)) == p)
			return idx;
	}

	return -1;
}

void MemoryManager::deallocate(const byte *p) {
	if (!p)
		return;

	int idx = indexOf(p);
	assert(idx != -1);
	if (_memoryPool[idx]->lockCtr-- == 0) {
		free(_memoryPool[idx]);
		_memoryPool[idx] = NULL;
	}
}

uint32 MemoryManager::getSize(const byte *p) {
	int idx = indexOf(p);
	assert(idx >= 0);
	return _memoryPool[idx]->size;
}

void MemoryManager::incLocks(const byte *p) {
	int idx = indexOf(p);
	assert(idx >= 0);
	_memoryPool[idx]->lockCtr++;
}

/*-------------------------------------------------------------------------*/

static uint16 bitMasks[4] = {0x1ff, 0x3ff, 0x7ff, 0xfff};

uint16 BitReader::readToken() {
	assert((numBits >= 9) && (numBits <= 12));
	uint16 result = _remainder;
	int bitsLeft = numBits - _bitsLeft;
	int bitOffset = _bitsLeft;
	_bitsLeft = 0;

	while (bitsLeft >= 0) {
		_remainder = readByte();
		result |= _remainder << bitOffset;
		bitsLeft -= 8;
		bitOffset += 8;
	}

	_bitsLeft = -bitsLeft;
	_remainder >>= 8 - _bitsLeft;
	return result & bitMasks[numBits - 9];
}

/*-------------------------------------------------------------------------*/

TLib::TLib(MemoryManager &memManager, const Common::String &filename) :
		_filename(filename), _memoryManager(memManager) {

	// If the resource strings list isn't yet loaded, load them
	if (_resStrings.size() == 0) {
		Common::File f;
		if (f.open("tsage.cfg")) {
			while (!f.eos()) {
				_resStrings.push_back(f.readLine());
			}
			f.close();
		}
	}

	if (!_file.open(filename))
		error("Missing file %s", filename.c_str());

	loadIndex();
}

TLib::~TLib() {
	_resStrings.clear();
}

/**
 * Load a section index from the given position in the file
 */
void TLib::loadSection(uint32 fileOffset) {
	_resources.clear();
	_file.seek(fileOffset);
	_sections.fileOffset = fileOffset;

	ResourceManager::loadSection(_file, _resources);
}

struct DecodeReference {
	uint16 vWord;
	uint8 vByte;
};

/**
 * Gets a resource from the currently loaded section
 */
byte *TLib::getResource(uint16 id, bool suppressErrors) {
	// Scan for an entry for the given Id
	ResourceEntry *re = NULL;
	ResourceList::iterator iter;
	for (iter = _resources.begin(); iter != _resources.end(); ++iter) {
		if ((*iter).id == id) {
			re = &(*iter);
			break;
		}
	}
	if (!re) {
		if (suppressErrors)
			return NULL;
		error("Could not find resource Id #%d", id);
	}

	if (!re->isCompressed) {
		// Read in the resource data and return it
		byte *dataP = _memoryManager.allocate2(re->size);
		_file.seek(_sections.fileOffset + re->fileOffset);
		_file.read(dataP, re->size);

		return dataP;
	}

	/*
	 * Decompress the data block
	 */

	_file.seek(_sections.fileOffset + re->fileOffset);
	Common::ReadStream *compStream = _file.readStream(re->size);
	BitReader bitReader(*compStream);

	byte *dataOut = _memoryManager.allocate2(re->uncompressedSize);
	byte *destP = dataOut;
	uint bytesWritten = 0;

	uint16 ctrCurrent = 0x102, ctrMax = 0x200;
	uint16 word_48050 = 0, currentToken = 0, word_48054 =0;
	byte byte_49068 = 0, byte_49069 = 0;

	const uint tableSize = 0x1000;
	DecodeReference *table = (DecodeReference *)malloc(tableSize * sizeof(DecodeReference));
	if (!table)
		error("[TLib::getResource] Cannot allocate table buffer");

	for (uint i = 0; i < tableSize; ++i) {
		table[i].vByte = table[i].vWord = 0;
	}
	Common::Stack<uint16> tokenList;

	for (;;) {
		// Get the next decode token
		uint16 token = bitReader.readToken();

		// Handle the token
		if (token == 0x101) {
			// End of compressed stream
			break;
		} else if (token == 0x100) {
			// Reset bit-rate
			bitReader.numBits = 9;
			ctrMax = 0x200;
			ctrCurrent = 0x102;

			// Set variables with next token
			currentToken = word_48050 = bitReader.readToken();
			byte_49069 = byte_49068 = (byte)currentToken;

			++bytesWritten;
			assert(bytesWritten <= re->uncompressedSize);
			*destP++ = byte_49069;
		} else {
			word_48054 = word_48050 = token;

			if (token >= ctrCurrent) {
				word_48050 = currentToken;
				tokenList.push(byte_49068);
			}

			while (word_48050 >= 0x100) {
				assert(word_48050 < 0x1000);
				tokenList.push(table[word_48050].vByte);
				word_48050 = table[word_48050].vWord;
			}

			byte_49069 = byte_49068 = (byte)word_48050;
			tokenList.push(word_48050);

			// Write out any cached tokens
			while (!tokenList.empty()) {
				++bytesWritten;
				assert(bytesWritten <= re->uncompressedSize);
				*destP++ = tokenList.pop();
			}

			assert(ctrCurrent < 0x1000);
			table[ctrCurrent].vByte = byte_49069;
			table[ctrCurrent].vWord = currentToken;
			++ctrCurrent;

			currentToken = word_48054;
			if ((ctrCurrent >= ctrMax) && (bitReader.numBits != 12)) {
				// Move to the next higher bit-rate
				++bitReader.numBits;
				ctrMax <<= 1;
			}
		}
	}

	free(table);

	assert(bytesWritten == re->uncompressedSize);
	delete compStream;
	return dataOut;
}

/**
 * Finds the correct section and loads the specified resource within it
 */
byte *TLib::getResource(ResourceType resType, uint16 resNum, uint16 rlbNum, bool suppressErrors) {
	SectionList::iterator i = _sections.begin();
	while ((i != _sections.end()) && ((*i).resType != resType || (*i).resNum != resNum))
		++i;
	if (i == _sections.end()) {
		if (suppressErrors)
			return NULL;
		error("Unknown resource type %d num %d", resType, resNum);
	}

	loadSection((*i).fileOffset);

	return getResource(rlbNum, suppressErrors);
}

/**
 * Gets the offset of the start of a resource in the resource file
 */
uint32 TLib::getResourceStart(ResourceType resType, uint16 resNum, uint16 rlbNum, ResourceEntry &entry) {
	// Find the correct section
	SectionList::iterator i = _sections.begin();
	while ((i != _sections.end()) && ((*i).resType != resType || (*i).resNum != resNum))
		++i;
	if (i == _sections.end()) {
		error("Unknown resource type %d num %d", resType, resNum);
	}

	// Load in the section index
	loadSection((*i).fileOffset);

	// Scan for an entry for the given Id
	ResourceEntry *re = NULL;
	ResourceList::iterator iter;
	for (iter = _resources.begin(); iter != _resources.end(); ++iter) {
		if ((*iter).id == rlbNum) {
			re = &(*iter);
			break;
		}
	}

	// Throw an error if no resource was found, or the resource is compressed
	if (!re || re->isCompressed)
		error("Invalid resource Id #%d", rlbNum);

	// Return the resource entry as well as the file offset
	entry = *re;
	return _sections.fileOffset + entry.fileOffset;
}

void TLib::loadIndex() {
	uint16 resNum, configId, fileOffset;

	// Load the root resources section
	loadSection(0);

	// Get the single resource from it
	const byte *pData = getResource(0);
	const byte *p = pData;

	_sections.clear();

	// Loop through reading the entries
	while ((resNum = READ_LE_UINT16(p)) != 0xffff) {
		configId = READ_LE_UINT16(p + 2);
		fileOffset = READ_LE_UINT16(p + 4);
		p += 6;

		SectionEntry se;
		se.resNum = resNum;
		se.resType = (ResourceType)(configId & 0x1f);
		se.fileOffset = (((configId >> 5) & 0x7ff) << 16) | fileOffset;
		if (g_vm->getGameID() == GType_Ringworld2)
			se.fileOffset <<= 4;

		_sections.push_back(se);
	}

	_memoryManager.deallocate(pData);
}

/**
 * Retrieves the specified palette resource and returns it's data
 *
 * @paletteNum Specefies the palette number
 */
bool TLib::getPalette(int paletteNum, byte *palData, uint *startNum, uint *numEntries) {
	// Get the specified palette
	byte *dataIn = getResource(RES_PALETTE, 0, paletteNum, true);
	if (!dataIn)
		return false;

	*startNum = READ_LE_UINT16(dataIn);
	*numEntries = READ_LE_UINT16(dataIn + 2);
	assert((*startNum < 256) && ((*startNum + *numEntries) <= 256));

	// Copy over the data
	Common::copy(&dataIn[6], &dataIn[6 + *numEntries * 3], palData);

	_memoryManager.deallocate(dataIn);
	return true;
}

byte *TLib::getSubResource(int resNum, int rlbNum, int index, uint *size, bool suppressErrors) {
	// Get the specified image set
	byte *dataIn = getResource(RES_VISAGE, resNum, rlbNum);
	if (!dataIn) {
		if (suppressErrors)
			return NULL;

		error("Unknown sub resource %d/%d index %d", resNum, rlbNum, index);
	}

	int numEntries = READ_LE_UINT16(dataIn);
	uint32 entryOffset = READ_LE_UINT32(dataIn + 2 + (index - 1) * 4);
	uint32 nextOffset = (index == numEntries) ?
			_memoryManager.getSize(dataIn) : READ_LE_UINT32(dataIn + 2 + index * 4);
	*size = nextOffset - entryOffset;
	assert(*size < (1024 * 1024));

	byte *entry = _memoryManager.allocate2(*size);
	Common::copy(&dataIn[entryOffset], &dataIn[nextOffset], entry);

	_memoryManager.deallocate(dataIn);
	return entry;
}

/**
 * Retrieves a given message resource, and returns the specified message number
 */
bool TLib::getMessage(int resNum, int lineNum, Common::String &result, bool suppressErrors) {
	byte *msgData = getResource(RES_MESSAGE, resNum, 0, true);
	if (!msgData || (lineNum < 0)) {
		if (suppressErrors)
			return false;

		error("Unknown message %d line %d", resNum, lineNum);
	}

	int msgSize = _memoryManager.getSize(msgData);
	const char *srcP = (const char *)msgData;
	const char *endP = srcP + msgSize;

	while (lineNum-- > 0) {
		srcP += strlen(srcP) + 1;

		if (srcP >= endP) {
			if (suppressErrors)
				return false;

			error("Unknown message %d line %d", resNum, lineNum);
		}
	}

	result = Common::String(srcP);
	_memoryManager.deallocate(msgData);
	return true;
}

/*--------------------------------------------------------------------------*/

ResourceManager::~ResourceManager() {
	for (uint idx = 0; idx < _libList.size(); ++idx)
		delete _libList[idx];
}

void ResourceManager::addLib(const Common::String &libName) {
	assert(_libList.size() < 5);

	_libList.push_back(new TLib(g_vm->_memoryManager, libName));
}

byte *ResourceManager::getResource(uint16 id, bool suppressErrors) {
	byte *result = NULL;
	for (uint idx = 0; idx < _libList.size(); ++idx) {
		result = _libList[idx]->getResource(id, true);
		if (result)
			return result;
	}

	if (!result && !suppressErrors)
		error("Could not find resource Id #%d", id);
	return NULL;
}

byte *ResourceManager::getResource(ResourceType resType, uint16 resNum, uint16 rlbNum, bool suppressErrors) {
	byte *result = NULL;
	for (uint idx = 0; idx < _libList.size(); ++idx) {
		result = _libList[idx]->getResource(resType, resNum, rlbNum, true);
		if (result)
			return result;
	}

	if (!result && !suppressErrors)
		error("Unknown resource type %d num %d", resType, resNum);
	return NULL;
}

void ResourceManager::getPalette(int paletteNum, byte *palData, uint *startNum, uint *numEntries, bool suppressErrors) {
	for (uint idx = 0; idx < _libList.size(); ++idx) {
		if (_libList[idx]->getPalette(paletteNum, palData, startNum, numEntries))
			return;
	}

	if (!suppressErrors)
		error("Unknown palette resource %d", paletteNum);
	*numEntries = 0;
}

byte *ResourceManager::getSubResource(int resNum, int rlbNum, int index, uint *size, bool suppressErrors) {
	byte *result = NULL;
	for (uint idx = 0; idx < _libList.size(); ++idx) {
		result = _libList[idx]->getSubResource(resNum, rlbNum, index, size, true);
		if (result)
			return result;
	}

	if (!result && !suppressErrors)
		error("Unknown resource %d/%d index %d", resNum, rlbNum, index);
	return NULL;
}

Common::String ResourceManager::getMessage(int resNum, int lineNum, bool suppressErrors) {
	Common::String result;

	for (uint idx = 0; idx < _libList.size(); ++idx) {
		if (_libList[idx]->getMessage(resNum, lineNum, result, true))
			return result;
	}

	if (!suppressErrors)
		error("Unknown message %d line %d", resNum, lineNum);
	return Common::String();
}

/*--------------------------------------------------------------------------*/

/**
 * Open up the given resource file using a passed file object. If the desired entry is found
 * in the index, return the index entry for it, and move the file to the start of the resource
 */
bool ResourceManager::scanIndex(Common::File &f, ResourceType resType, int rlbNum, int resNum,
									  ResourceEntry &resEntry) {
	// Load the root section index
	ResourceList resList;
	loadSection(f, resList);

	// Loop through the index for the desired entry
	ResourceList::iterator iter;
	for (iter = resList.begin(); iter != resList.end(); ++iter) {
		ResourceEntry &re = *iter;
		if (re.id == resNum) {
			// Found it, so exit
			resEntry = re;
			f.seek(re.fileOffset);
			return true;
		}
	}

	// No matching entry found
	return false;
}

/**
 * Inner logic for decoding a section index into a passed resource list object
 */
void ResourceManager::loadSection(Common::File &f, ResourceList &resources) {
	if (f.readUint32BE() != 0x544D492D)
		error("Data block is not valid Rlb data");

	/*uint8 unknown1 = */f.readByte();
	uint16 numEntries = f.readByte();

	for (uint i = 0; i < numEntries; ++i) {
		uint16 id = f.readUint16LE();
		uint16 size = f.readUint16LE();
		uint16 uncSize = f.readUint16LE();
		uint8 sizeHi = f.readByte();
		uint8 type = f.readByte() >> 5;
		assert(type <= 1);
		uint32 offset = f.readUint32LE();

		ResourceEntry re;
		re.id = id;
		re.fileOffset = offset;
		re.isCompressed = type != 0;
		re.size = ((sizeHi & 0xF) << 16) | size;
		re.uncompressedSize = ((sizeHi & 0xF0) << 12) | uncSize;

		resources.push_back(re);
	}
}

} // end of namespace TsAGE
