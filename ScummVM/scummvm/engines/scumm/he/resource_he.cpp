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

#include "scumm/scumm.h"
#include "scumm/file.h"
#include "scumm/he/intern_he.h"
#include "scumm/resource.h"
#include "scumm/he/resource_he.h"
#include "scumm/he/sound_he.h"

#include "audio/decoders/wave.h"
#include "graphics/cursorman.h"
#include "graphics/maccursor.h"
#include "graphics/wincursor.h"

#include "common/archive.h"
#include "common/memstream.h"
#include "common/system.h"

namespace Scumm {

ResExtractor::ResExtractor(ScummEngine_v70he *scumm)
	: _vm(scumm) {

	memset(_cursorCache, 0, sizeof(_cursorCache));
}

ResExtractor::~ResExtractor() {
	for (int i = 0; i < MAX_CACHED_CURSORS; ++i) {
		CachedCursor *cc = &_cursorCache[i];
		if (cc->valid) {
			free(cc->bitmap);
			free(cc->palette);
		}
	}

	memset(_cursorCache, 0, sizeof(_cursorCache));
}

ResExtractor::CachedCursor *ResExtractor::findCachedCursor(int id) {
	for (int i = 0; i < MAX_CACHED_CURSORS; ++i)
		if (_cursorCache[i].valid && _cursorCache[i].id == id)
			return &_cursorCache[i];

	return NULL;
}

ResExtractor::CachedCursor *ResExtractor::getCachedCursorSlot() {
	uint32 minLastUsed = 0;
	CachedCursor *r = NULL;

	for (int i = 0; i < MAX_CACHED_CURSORS; ++i) {
		CachedCursor *cc = &_cursorCache[i];
		if (!cc->valid)
			return cc;

		if (minLastUsed == 0 || cc->lastUsed < minLastUsed) {
			minLastUsed = cc->lastUsed;
			r = cc;
		}
	}

	assert(r);
	delete[] r->bitmap;
	delete[] r->palette;
	memset(r, 0, sizeof(CachedCursor));
	return r;
}

void ResExtractor::setCursor(int id) {
	CachedCursor *cc = findCachedCursor(id);

	if (cc != NULL) {
		debug(7, "Found cursor %d in cache slot %lu", id, (long)(cc - _cursorCache));
	} else {
		cc = getCachedCursorSlot();
		assert(cc && !cc->valid);

		if (!extractResource(id, cc))
			error("Could not extract cursor %d", id);

		debug(7, "Adding cursor %d to cache slot %lu", id, (long)(cc - _cursorCache));

		cc->valid = true;
		cc->id = id;
		cc->lastUsed = g_system->getMillis();
	}

	if (cc->palette)
		CursorMan.replaceCursorPalette(cc->palette, 0, cc->palSize);

	_vm->setCursorHotspot(cc->hotspotX, cc->hotspotY);
	_vm->setCursorFromBuffer(cc->bitmap, cc->width, cc->height, cc->width);
}


Win32ResExtractor::Win32ResExtractor(ScummEngine_v70he *scumm) : ResExtractor(scumm) {
}

bool Win32ResExtractor::extractResource(int id, CachedCursor *cc) {
	if (_fileName.empty()) { // We are running for the first time
		_fileName = _vm->generateFilename(-3);

		if (!_exe.loadFromEXE(_fileName))
			error("Cannot open file %s", _fileName.c_str());
	}

	Graphics::WinCursorGroup *group = Graphics::WinCursorGroup::createCursorGroup(_exe, id);

	if (!group)
		return false;

	Graphics::Cursor *cursor = group->cursors[0].cursor;

	cc->bitmap = new byte[cursor->getWidth() * cursor->getHeight()];
	cc->width = cursor->getWidth();
	cc->height = cursor->getHeight();
	cc->hotspotX = cursor->getHotspotX();
	cc->hotspotY = cursor->getHotspotY();

	// Convert from the paletted format to the SCUMM palette
	const byte *srcBitmap = cursor->getSurface();

	for (int i = 0; i < cursor->getWidth() * cursor->getHeight(); i++) {
		if (srcBitmap[i] == cursor->getKeyColor()) // Transparent
			cc->bitmap[i] = 255;
		else if (srcBitmap[i] == 0)                // Black
			cc->bitmap[i] = 253;
		else                                       // White
			cc->bitmap[i] = 254;
	}

	delete group;
	return true;
}

MacResExtractor::MacResExtractor(ScummEngine_v70he *scumm) : ResExtractor(scumm) {
	_resMgr = NULL;
}

bool MacResExtractor::extractResource(int id, CachedCursor *cc) {
	// Create the MacResManager if not created already
	if (_resMgr == NULL) {
		_resMgr = new Common::MacResManager();
		if (!_resMgr->open(_vm->generateFilename(-3)))
			error("Cannot open file %s", _fileName.c_str());
	}

	Common::SeekableReadStream *dataStream = _resMgr->getResource('crsr', id + 1000);

	if (!dataStream)
		return false;

	// If we don't have a cursor palette, force monochrome cursors
	bool forceMonochrome = !_vm->_system->hasFeature(OSystem::kFeatureCursorPalette);

	Graphics::MacCursor *macCursor = new Graphics::MacCursor();

	if (!macCursor->readFromStream(*dataStream, forceMonochrome)) {
		delete dataStream;
		delete macCursor;
		return false;
	}

	cc->bitmap = new byte[macCursor->getWidth() * macCursor->getHeight()];
	cc->width = macCursor->getWidth();
	cc->height = macCursor->getHeight();
	cc->hotspotX = macCursor->getHotspotX();
	cc->hotspotY = macCursor->getHotspotY();

	if (forceMonochrome) {
		// Convert to the SCUMM palette
		const byte *srcBitmap = macCursor->getSurface();

		for (int i = 0; i < macCursor->getWidth() * macCursor->getHeight(); i++) {
			if (srcBitmap[i] == macCursor->getKeyColor()) // Transparent
				cc->bitmap[i] = 255;
			else if (srcBitmap[i] == 0)                // Black
				cc->bitmap[i] = 253;
			else                                       // White
				cc->bitmap[i] = 254;
		}
	} else {
		// Copy data and palette

		// Sanity check. This code assumes that the key color is the same
		assert(macCursor->getKeyColor() == 255);

		memcpy(cc->bitmap, macCursor->getSurface(), macCursor->getWidth() * macCursor->getHeight());

		cc->palette = new byte[256 * 3];
		cc->palSize = 256;
		memcpy(cc->palette, macCursor->getPalette(), 256 * 3);
	}

	delete macCursor;
	delete dataStream;
	return true;
}

void ScummEngine_v70he::readRoomsOffsets() {
	int num, i;
	byte *ptr;

	debug(9, "readRoomOffsets()");

	num = READ_LE_UINT16(_heV7RoomOffsets);
	ptr = _heV7RoomOffsets + 2;
	for (i = 0; i < num; i++) {
		_res->_types[rtRoom][i]._roomoffs = READ_LE_UINT32(ptr);
		ptr += 4;
	}
}

void ScummEngine_v70he::readGlobalObjects() {
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);
	assert(_objectStateTable);
	assert(_objectOwnerTable);

	_fileHandle->read(_objectStateTable, num);
	_fileHandle->read(_objectOwnerTable, num);
	_fileHandle->read(_objectRoomTable, num);

	_fileHandle->read(_classData, num * sizeof(uint32));

#if defined(SCUMM_BIG_ENDIAN)
	// Correct the endianess if necessary
	for (int i = 0; i != num; i++)
		_classData[i] = FROM_LE_32(_classData[i]);
#endif
}

#ifdef ENABLE_HE
void ScummEngine_v99he::readMAXS(int blockSize) {
	if (blockSize == 52) {
		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numSprites = _fileHandle->readUint16LE();
		_numLocalScripts = _fileHandle->readUint16LE();
		_HEHeapSize = _fileHandle->readUint16LE();
		_numPalettes = _fileHandle->readUint16LE();
		_numUnk = _fileHandle->readUint16LE();
		_numTalkies = _fileHandle->readUint16LE();
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		_numGlobalScripts = 2048;
	} else
		ScummEngine_v90he::readMAXS(blockSize);
}

void ScummEngine_v90he::readMAXS(int blockSize) {
	if (blockSize == 46) {
		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numSprites = _fileHandle->readUint16LE();
		_numLocalScripts = _fileHandle->readUint16LE();
		_HEHeapSize = _fileHandle->readUint16LE();
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		if (_game.features & GF_HE_985)
			_numGlobalScripts = 2048;
		else
			_numGlobalScripts = 200;
	} else
		ScummEngine_v72he::readMAXS(blockSize);
}

void ScummEngine_v72he::readMAXS(int blockSize) {
	if (blockSize == 40) {
		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numBitVariables = _numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numVerbs = _fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		_numGlobalScripts = 200;
	} else
		ScummEngine_v6::readMAXS(blockSize);
}

byte *ScummEngine_v72he::getStringAddress(ResId idx) {
	byte *addr = getResourceAddress(rtString, idx);
	if (addr == NULL)
		return NULL;
	return ((ScummEngine_v72he::ArrayHeader *)addr)->data;
}

int ScummEngine_v72he::getSoundResourceSize(ResId id) {
	const byte *ptr;
	int offs, size;

	if (id > _numSounds) {
		if (!((SoundHE *)_sound)->getHEMusicDetails(id, offs, size)) {
			debug(0, "getSoundResourceSize: musicID %d not found", id);
			return 0;
		}
	} else {
		ptr = getResourceAddress(rtSound, id);
		if (!ptr)
			return 0;

		if (READ_BE_UINT32(ptr) == MKTAG('R','I','F','F')) {
			byte flags;
			int rate;

			size = READ_BE_UINT32(ptr + 4);
			Common::MemoryReadStream stream(ptr, size);

			if (!Audio::loadWAVFromStream(stream, size, rate, flags)) {
				error("getSoundResourceSize: Not a valid WAV file");
			}
		} else {
			ptr += 8 + READ_BE_UINT32(ptr + 12);
			if (READ_BE_UINT32(ptr) == MKTAG('S','B','N','G')) {
				ptr += READ_BE_UINT32(ptr + 4);
			}

			assert(READ_BE_UINT32(ptr) == MKTAG('S','D','A','T'));
			size = READ_BE_UINT32(ptr + 4) - 8;
		}
	}

	return size;
}

void ScummEngine_v90he::setResourceOffHeap(int typeId, int resId, int val) {
	debug(0, "setResourceOffHeap: type %d resId %d toggle %d", typeId, resId, val);
	ResType type;

	switch (typeId) {
	case 1:
		type = rtRoom;
		break;
	case 2:
		type = rtScript;
		break;
	case 3:
		type = rtCostume;
		break;
	case 4:
		type = rtSound;
		break;
	case 6:
		type = rtCharset;
		break;
	case 19:
		type = rtImage;
		break;
	default:
		error("setResourceOffHeap: default case %d", typeId);
	}

	if (val == 1) {
		_res->setOffHeap(type, resId);
	} else {
		_res->setOnHeap(type, resId);
	}
}

#endif

} // End of namespace Scumm
