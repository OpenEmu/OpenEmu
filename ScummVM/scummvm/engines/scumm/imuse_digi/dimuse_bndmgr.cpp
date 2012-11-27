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


#include "common/scummsys.h"
#include "scumm/scumm.h"
#include "scumm/util.h"
#include "scumm/file.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_codecs.h"

namespace Scumm {

BundleDirCache::BundleDirCache() {
	for (int fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		_budleDirCache[fileId].bundleTable = NULL;
		_budleDirCache[fileId].fileName[0] = 0;
		_budleDirCache[fileId].numFiles = 0;
		_budleDirCache[fileId].isCompressed = false;
		_budleDirCache[fileId].indexTable = NULL;
	}
}

BundleDirCache::~BundleDirCache() {
	for (int fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		free(_budleDirCache[fileId].bundleTable);
		free(_budleDirCache[fileId].indexTable);
	}
}

BundleDirCache::AudioTable *BundleDirCache::getTable(int slot) {
	return _budleDirCache[slot].bundleTable;
}

int32 BundleDirCache::getNumFiles(int slot) {
	return _budleDirCache[slot].numFiles;
}

BundleDirCache::IndexNode *BundleDirCache::getIndexTable(int slot) {
	return _budleDirCache[slot].indexTable;
}

bool BundleDirCache::isSndDataExtComp(int slot) {
	return _budleDirCache[slot].isCompressed;
}

int BundleDirCache::matchFile(const char *filename) {
	int32 tag, offset;
	bool found = false;
	int freeSlot = -1;
	int fileId;

	for (fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		if ((_budleDirCache[fileId].bundleTable == NULL) && (freeSlot == -1)) {
			freeSlot = fileId;
		}
		if (scumm_stricmp(filename, _budleDirCache[fileId].fileName) == 0) {
			found = true;
			break;
		}
	}

	if (!found) {
		ScummFile file;

		if (g_scumm->openFile(file, filename) == false) {
			error("BundleDirCache::matchFile() Can't open bundle file: %s", filename);
			return false;
		}

		if (freeSlot == -1)
			error("BundleDirCache::matchFileFile() Can't find free slot for file bundle dir cache");

		tag = file.readUint32BE();
		if (tag == MKTAG('L','B','2','3'))
			_budleDirCache[freeSlot].isCompressed = true;
		offset = file.readUint32BE();

		strcpy(_budleDirCache[freeSlot].fileName, filename);
		_budleDirCache[freeSlot].numFiles = file.readUint32BE();
		_budleDirCache[freeSlot].bundleTable = (AudioTable *)malloc(_budleDirCache[freeSlot].numFiles * sizeof(AudioTable));
		assert(_budleDirCache[freeSlot].bundleTable);

		file.seek(offset, SEEK_SET);

		_budleDirCache[freeSlot].indexTable =
				(IndexNode *)calloc(_budleDirCache[freeSlot].numFiles, sizeof(IndexNode));
		assert(_budleDirCache[freeSlot].indexTable);

		for (int32 i = 0; i < _budleDirCache[freeSlot].numFiles; i++) {
			char name[24], c;
			int32 z = 0;
			int32 z2;

			if (tag == MKTAG('L','B','2','3')) {
				file.read(_budleDirCache[freeSlot].bundleTable[i].filename, 24);
			} else {
				for (z2 = 0; z2 < 8; z2++)
					if ((c = file.readByte()) != 0)
						name[z++] = c;
				name[z++] = '.';
				for (z2 = 0; z2 < 4; z2++)
					if ((c = file.readByte()) != 0)
						name[z++] = c;

				name[z] = '\0';
				strcpy(_budleDirCache[freeSlot].bundleTable[i].filename, name);
			}
			_budleDirCache[freeSlot].bundleTable[i].offset = file.readUint32BE();
			_budleDirCache[freeSlot].bundleTable[i].size = file.readUint32BE();
			strcpy(_budleDirCache[freeSlot].indexTable[i].filename,
					_budleDirCache[freeSlot].bundleTable[i].filename);
			_budleDirCache[freeSlot].indexTable[i].index = i;
		}
		qsort(_budleDirCache[freeSlot].indexTable, _budleDirCache[freeSlot].numFiles,
				sizeof(IndexNode), (int (*)(const void*, const void*))scumm_stricmp);
		return freeSlot;
	} else {
		return fileId;
	}
}

BundleMgr::BundleMgr(BundleDirCache *cache) {
	_cache = cache;
	_bundleTable = NULL;
	_compTable = NULL;
	_numFiles = 0;
	_numCompItems = 0;
	_curSampleId = -1;
	_fileBundleId = -1;
	_file = new ScummFile();
	_compInputBuff = NULL;
}

BundleMgr::~BundleMgr() {
	close();
	delete _file;
}

Common::SeekableReadStream *BundleMgr::getFile(const char *filename, int32 &offset, int32 &size) {
	BundleDirCache::IndexNode target;
	strcpy(target.filename, filename);
	BundleDirCache::IndexNode *found = (BundleDirCache::IndexNode *)bsearch(&target, _indexTable, _numFiles,
			sizeof(BundleDirCache::IndexNode), (int (*)(const void*, const void*))scumm_stricmp);
	if (found) {
		_file->seek(_bundleTable[found->index].offset, SEEK_SET);
		offset = _bundleTable[found->index].offset;
		size = _bundleTable[found->index].size;
		return _file;
	}

	return NULL;
}

bool BundleMgr::open(const char *filename, bool &compressed, bool errorFlag) {
	if (_file->isOpen())
		return true;

	if (g_scumm->openFile(*_file, filename) == false) {
		if (errorFlag) {
			error("BundleMgr::open() Can't open bundle file: %s", filename);
		} else {
			warning("BundleMgr::open() Can't open bundle file: %s", filename);
		}
		return false;
	}

	int slot = _cache->matchFile(filename);
	assert(slot != -1);
	compressed = _cache->isSndDataExtComp(slot);
	_numFiles = _cache->getNumFiles(slot);
	assert(_numFiles);
	_bundleTable = _cache->getTable(slot);
	_indexTable = _cache->getIndexTable(slot);
	assert(_bundleTable);
	_compTableLoaded = false;
	_outputSize = 0;
	_lastBlock = -1;

	return true;
}

void BundleMgr::close() {
	if (_file->isOpen()) {
		_file->close();
		_bundleTable = NULL;
		_numFiles = 0;
		_numCompItems = 0;
		_compTableLoaded = false;
		_lastBlock = -1;
		_outputSize = 0;
		_curSampleId = -1;
		free(_compTable);
		_compTable = NULL;
		free(_compInputBuff);
		_compInputBuff = NULL;
	}
}

bool BundleMgr::loadCompTable(int32 index) {
	_file->seek(_bundleTable[index].offset, SEEK_SET);
	uint32 tag = _file->readUint32BE();
	_numCompItems = _file->readUint32BE();
	assert(_numCompItems > 0);
	_file->seek(8, SEEK_CUR);

	if (tag != MKTAG('C','O','M','P')) {
		error("BundleMgr::loadCompTable() Compressed sound %d (%s:%d) invalid (%s)", index, _file->getName(), _bundleTable[index].offset, tag2str(tag));
		return false;
	}

	_compTable = (CompTable *)malloc(sizeof(CompTable) * _numCompItems);
	assert(_compTable);
	int32 maxSize = 0;
	for (int i = 0; i < _numCompItems; i++) {
		_compTable[i].offset = _file->readUint32BE();
		_compTable[i].size = _file->readUint32BE();
		_compTable[i].codec = _file->readUint32BE();
		_file->seek(4, SEEK_CUR);
		if (_compTable[i].size > maxSize)
			maxSize = _compTable[i].size;
	}
	// CMI hack: one more byte at the end of input buffer
	_compInputBuff = (byte *)malloc(maxSize + 1);
	assert(_compInputBuff);

	return true;
}

int32 BundleMgr::decompressSampleByCurIndex(int32 offset, int32 size, byte **compFinal, int headerSize, bool headerOutside) {
	return decompressSampleByIndex(_curSampleId, offset, size, compFinal, headerSize, headerOutside);
}

int32 BundleMgr::decompressSampleByIndex(int32 index, int32 offset, int32 size, byte **compFinal, int headerSize, bool headerOutside) {
	int32 i, finalSize, outputSize;
	int skip, firstBlock, lastBlock;

	assert(0 <= index && index < _numFiles);

	if (_file->isOpen() == false) {
		error("BundleMgr::decompressSampleByIndex() File is not open");
		return 0;
	}

	if (_curSampleId == -1)
		_curSampleId = index;

	assert(_curSampleId == index);

	if (!_compTableLoaded) {
		_compTableLoaded = loadCompTable(index);
		if (!_compTableLoaded)
			return 0;
	}

	firstBlock = (offset + headerSize) / 0x2000;
	lastBlock = (offset + headerSize + size - 1) / 0x2000;

	// Clip last_block by the total number of blocks (= "comp items")
	if ((lastBlock >= _numCompItems) && (_numCompItems > 0))
		lastBlock = _numCompItems - 1;

	int32 blocksFinalSize = 0x2000 * (1 + lastBlock - firstBlock);
	*compFinal = (byte *)malloc(blocksFinalSize);
	assert(*compFinal);
	finalSize = 0;

	skip = (offset + headerSize) % 0x2000;

	for (i = firstBlock; i <= lastBlock; i++) {
		if (_lastBlock != i) {
			// CMI hack: one more zero byte at the end of input buffer
			_compInputBuff[_compTable[i].size] = 0;
			_file->seek(_bundleTable[index].offset + _compTable[i].offset, SEEK_SET);
			_file->read(_compInputBuff, _compTable[i].size);
			_outputSize = BundleCodecs::decompressCodec(_compTable[i].codec, _compInputBuff, _compOutputBuff, _compTable[i].size);
			if (_outputSize > 0x2000) {
				error("_outputSize: %d", _outputSize);
			}
			_lastBlock = i;
		}

		outputSize = _outputSize;

		if (headerOutside) {
			outputSize -= skip;
		} else {
			if ((headerSize != 0) && (skip >= headerSize))
				outputSize -= skip;
		}

		if ((outputSize + skip) > 0x2000) // workaround
			outputSize -= (outputSize + skip) - 0x2000;

		if (outputSize > size)
			outputSize = size;

		assert(finalSize + outputSize <= blocksFinalSize);

		memcpy(*compFinal + finalSize, _compOutputBuff + skip, outputSize);
		finalSize += outputSize;

		size -= outputSize;
		assert(size >= 0);
		if (size == 0)
			break;

		skip = 0;
	}

	return finalSize;
}

int32 BundleMgr::decompressSampleByName(const char *name, int32 offset, int32 size, byte **comp_final, bool header_outside) {
	int32 final_size = 0;

	if (!_file->isOpen()) {
		error("BundleMgr::decompressSampleByName() File is not open");
		return 0;
	}

	BundleDirCache::IndexNode target;
	strcpy(target.filename, name);
	BundleDirCache::IndexNode *found = (BundleDirCache::IndexNode *)bsearch(&target, _indexTable, _numFiles,
			sizeof(BundleDirCache::IndexNode), (int (*)(const void*, const void*))scumm_stricmp);
	if (found) {
		final_size = decompressSampleByIndex(found->index, offset, size, comp_final, 0, header_outside);
		return final_size;
	}

	debug(2, "BundleMgr::decompressSampleByName() Failed finding sound %s", name);
	return final_size;
}

} // End of namespace Scumm
