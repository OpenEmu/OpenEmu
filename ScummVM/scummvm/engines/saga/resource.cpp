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

// RSC Resource file management module

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/interface.h"
#include "saga/music.h"
#include "saga/resource.h"
#include "saga/scene.h"
#include "saga/sndres.h"

#include "engines/advancedDetector.h"

namespace Saga {

bool ResourceContext::loadResV1(uint32 contextOffset, uint32 contextSize) {
	size_t i;
	bool result;
	byte tableInfo[RSC_TABLEINFO_SIZE];
	ByteArray tableBuffer;
	uint32 count;
	uint32 resourceTableOffset;
	ResourceData *resourceData;

	if (contextSize < RSC_MIN_FILESIZE) {
		return false;
	}

	_file.seek(contextOffset + contextSize - RSC_TABLEINFO_SIZE);

	if (_file.read(tableInfo, RSC_TABLEINFO_SIZE) != RSC_TABLEINFO_SIZE) {
		return false;
	}

	Common::MemoryReadStreamEndian readS(tableInfo, RSC_TABLEINFO_SIZE, _isBigEndian);

	resourceTableOffset = readS.readUint32();
	count = readS.readUint32();

	// Check for sane table offset
	if (resourceTableOffset != contextSize - RSC_TABLEINFO_SIZE - RSC_TABLEENTRY_SIZE * count) {
		return false;
	}

	// Load resource table
	tableBuffer.resize(RSC_TABLEENTRY_SIZE * count);

	_file.seek(resourceTableOffset + contextOffset, SEEK_SET);

	result = (_file.read(tableBuffer.getBuffer(), tableBuffer.size()) == tableBuffer.size());
	if (result) {
		_table.resize(count);

		Common::MemoryReadStreamEndian readS1(tableBuffer.getBuffer(), tableBuffer.size(), _isBigEndian);

		for (i = 0; i < count; i++) {
			resourceData = &_table[i];
			resourceData->offset = contextOffset + readS1.readUint32();
			resourceData->size = readS1.readUint32();
			// Sanity check
			if ((resourceData->offset > (uint)_fileSize) || (resourceData->size > contextSize)) {
				result = false;
				break;
			}
		}
	}

	return result;
}

bool ResourceContext::load(SagaEngine *vm, Resource *resource) {
	if (_fileName == NULL) // IHNM special case
		return true;

	if (!_file.open(_fileName))
		return false;

	_fileSize = _file.size();
	_isBigEndian = vm->isBigEndian();

	if (_fileType & GAME_SWAPENDIAN)
		_isBigEndian = !_isBigEndian;

	if (_fileType & GAME_MACBINARY) {
		// Special case for the MacBinary packed files in the old Mac ITE
		// release. There are no patch files in this case.
		if (!(_fileType & GAME_MUSICFILE_GM)) {
			// Find the actual size, as there may be padded data in the end.
			_file.seek(83);
			uint32 macDataSize = _file.readSint32BE();
			// Skip the MacBinary headers, and read the resource data.
			return loadRes(MAC_BINARY_HEADER_SIZE, macDataSize);
		} else {
			// Unpack MacBinary packed MIDI files
			return loadMacMIDI();
		}
	}

	if (!loadRes(0, _fileSize))
		return false;

	processPatches(resource, vm->getPatchDescriptions());

	// Close the file if it's part of a series of files.
	// This prevents having all voice files open in IHNM for no reason, as each chapter uses
	// a different voice file.
	if (_serial > 0)
		_file.close();

	return true;
}

Resource::Resource(SagaEngine *vm): _vm(vm) {
}

Resource::~Resource() {
	clearContexts();
}

void Resource::addContext(const char *fileName, uint16 fileType, bool isCompressed, int serial) {
	ResourceContext *context;
	context = createContext();
	context->_fileName = fileName;
	context->_fileType = fileType;
	context->_isCompressed = isCompressed;
	context->_serial = serial;
	_contexts.push_back(context);
}

bool Resource::createContexts() {
	bool soundFileInArray = false;

	_vm->_voiceFilesExist = true;

	struct SoundFileInfo {
		int gameId;
		char fileName[40];
		bool isCompressed;
		uint16 voiceFileAddType;
	};

	for (const ADGameFileDescription *gameFileDescription = _vm->getFilesDescriptions();
		gameFileDescription->fileName; gameFileDescription++) {
		addContext(gameFileDescription->fileName, gameFileDescription->fileType);
		if (gameFileDescription->fileType == GAME_SOUNDFILE) {
			soundFileInArray = true;
		}
	}

	//// Detect and add SFX files ////////////////////////////////////////////////
	SoundFileInfo sfxFiles[] = {
		{	GID_ITE,	"sounds.rsc",		false,	0	},
		{	GID_ITE,	"sounds.cmp",		true,	0	},
		{	GID_ITE,	"soundsd.rsc",		false,	0	},
		{	GID_ITE,	"soundsd.cmp",		true,	0	},
#ifdef ENABLE_IHNM
		{	GID_IHNM,	"sfx.res",			false,	0	},
		{	GID_IHNM,	"sfx.cmp",			true,	0	},
#endif
#ifdef ENABLE_SAGA2
		{	GID_FTA2,	"ftasound.hrs",		false,	0	},
		{	GID_DINO,	"dinosnd.hrs",		false,	0	},
#endif
		{	-1,			"",				false,	0	}
	};

	_soundFileName[0] = 0;
	if (!soundFileInArray) {
		for (SoundFileInfo *curSoundFile = sfxFiles; (curSoundFile->gameId != -1); curSoundFile++) {
			if (curSoundFile->gameId != _vm->getGameId()) continue;
			if (!Common::File::exists(curSoundFile->fileName)) continue;
			strcpy(_soundFileName, curSoundFile->fileName);
			addContext(_soundFileName, GAME_SOUNDFILE, curSoundFile->isCompressed);
			break;
		}
	}

	//// Detect and add voice files /////////////////////////////////////////////
	SoundFileInfo voiceFiles[] = {
		{	GID_ITE,	"voices.rsc",					false	,	(_soundFileName[0] == 0) ? GAME_SOUNDFILE : 0},
		{	GID_ITE,	"voices.cmp",					true	,	(_soundFileName[0] == 0) ? GAME_SOUNDFILE : 0},
		{	GID_ITE,	"voicesd.rsc",					false	,	(_soundFileName[0] == 0) ? GAME_SOUNDFILE : 0},
		{	GID_ITE,	"voicesd.cmp",					true	,	(_soundFileName[0] == 0) ? GAME_SOUNDFILE : 0},
		// The resources in the Wyrmkeep combined Windows/Mac/Linux CD version are little endian, but
		// the voice file is big endian. If we got such a version with mixed files, mark this voice file
		// as big endian
		{	GID_ITE,	"inherit the earth voices",		false	,	_vm->isBigEndian() ? 0 : GAME_SWAPENDIAN},
		{	GID_ITE,	"inherit the earth voices.cmp",	true	,	_vm->isBigEndian() ? 0 : GAME_SWAPENDIAN},
		{	GID_ITE,	"ite voices.bin",				false	,	GAME_MACBINARY},
#ifdef ENABLE_IHNM
		{	GID_IHNM,	"voicess.res",					false	,	0},
		{	GID_IHNM,	"voicess.cmp",					true	,	0},
		{	GID_IHNM,	"voicesd.res",					false	,	0},
		{	GID_IHNM,	"voicesd.cmp",					true	,	0},
#endif
#ifdef ENABLE_SAGA2
		{	GID_FTA2,	"ftavoice.hrs",					false	,	0},
#endif
		{	-1,			"",							false	,	0}
	};

	// Detect and add voice files
	_voicesFileName[0][0] = 0;
	for (SoundFileInfo *curSoundFile = voiceFiles; (curSoundFile->gameId != -1); curSoundFile++) {
		if (curSoundFile->gameId != _vm->getGameId()) continue;
		if (!Common::File::exists(curSoundFile->fileName)) continue;

		strcpy(_voicesFileName[0], curSoundFile->fileName);
		addContext(_voicesFileName[0], GAME_VOICEFILE | curSoundFile->voiceFileAddType, curSoundFile->isCompressed);

		// Special cases
		if (!scumm_stricmp(curSoundFile->fileName, "voicess.res") ||
			!scumm_stricmp(curSoundFile->fileName, "voicess.cmp")) {
				// IHNM has multiple voice files
				for (size_t i = 1; i <= 6; i++) { // voices1-voices6
					sprintf(_voicesFileName[i], "voices%i.%s", (uint)i, curSoundFile->isCompressed ? "cmp" : "res");
					if (i == 4) {
						// The German and French versions of IHNM don't have Nimdok's chapter,
						// therefore the voices file for that chapter is missing
						if (!Common::File::exists(_voicesFileName[i])) {
							continue;
						}
					}
					addContext(_voicesFileName[i], GAME_VOICEFILE, curSoundFile->isCompressed, i);
				}
		}
		break;
	}

	if (_voicesFileName[0][0] == 0) {
#ifdef ENABLE_IHNM
		if (_vm->getGameId() == GID_IHNM && _vm->isMacResources()) {
			// The Macintosh version of IHNM has no voices.res, and it has all
			// its voice files in subdirectories, so don't do anything here
			_contexts.push_back(new VoiceResourceContext_RES());
		} else {
#endif
			warning("No voice file found, voices will be disabled");
			_vm->_voicesEnabled = false;
			_vm->_subtitlesEnabled = true;
			_vm->_voiceFilesExist = false;
#ifdef ENABLE_IHNM
		}
#endif
	}

	//// Detect and add music files /////////////////////////////////////////
	SoundFileInfo musicFiles[] = {
		{	GID_ITE,	"music.rsc",	false,	0	},
		{	GID_ITE,	"music.cmp",	true,	0	},
		{	GID_ITE,	"musicd.rsc",	false,	0	},
		{	GID_ITE,	"musicd.cmp",	true,	0	},
		{	-1,			"",			false	,	0}
	};

	// Check for digital music in ITE

	for (SoundFileInfo *curSoundFile = musicFiles; (curSoundFile->gameId != -1); curSoundFile++) {
		if (curSoundFile->gameId != _vm->getGameId()) continue;
		if (!Common::File::exists(curSoundFile->fileName)) continue;
		strcpy(_musicFileName, curSoundFile->fileName);
		addContext(_musicFileName, GAME_DIGITALMUSICFILE, curSoundFile->isCompressed);
		break;
	}

	for (ResourceContextList::iterator i = _contexts.begin(); i != _contexts.end(); ++i) {
		if (!(*i)->load(_vm, this)) {
			return false;
		}
	}
	return true;
}

void Resource::clearContexts() {
	ResourceContextList::iterator i = _contexts.begin();
	while (i != _contexts.end()) {
		ResourceContext * context = *i;
		i = _contexts.erase(i);
		delete context;
	}
}

void Resource::loadResource(ResourceContext *context, uint32 resourceId, ByteArray &resourceBuffer) {
	Common::File *file;
	uint32 resourceOffset;
	ResourceData *resourceData;


	resourceData = context->getResourceData(resourceId);

	file = context->getFile(resourceData);

	resourceOffset = resourceData->offset;

	debug(8, "loadResource %d 0x%X:0x%X", resourceId, resourceOffset, uint(resourceData->size));
	resourceBuffer.resize(resourceData->size);


	file->seek((long)resourceOffset, SEEK_SET);

	if (file->read(resourceBuffer.getBuffer(), resourceBuffer.size()) != resourceBuffer.size()) {
		error("Resource::loadResource() failed to read");
	}

	// ITE uses several patch files which are loaded and then not needed
	// anymore (as they're in memory), so close them here. IHNM uses only
	// 1 patch file, which is reused, so don't close it
	if (resourceData->patchData != NULL && _vm->getGameId() == GID_ITE)
		file->close();
}

ResourceContext *Resource::getContext(uint16 fileType, int serial) {
	for (ResourceContextList::const_iterator i = _contexts.begin(); i != _contexts.end(); ++i) {
		ResourceContext * context = *i;
		if ((context->fileType() & fileType) && (context->serial() == serial)) {
			return context;
		}
	}
	return NULL;
}

} // End of namespace Saga
