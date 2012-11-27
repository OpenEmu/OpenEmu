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
#include "common/util.h"

#include "audio/decoders/flac.h"
#include "audio/decoders/voc.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/mp3.h"

#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/util.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_codecs.h"
#include "scumm/imuse_digi/dimuse_sndmgr.h"

namespace Scumm {

ImuseDigiSndMgr::ImuseDigiSndMgr(ScummEngine *scumm) {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		memset(&_sounds[l], 0, sizeof(SoundDesc));
	}
	_vm = scumm;
	_disk = 0;
	_cacheBundleDir = new BundleDirCache();
	assert(_cacheBundleDir);
	BundleCodecs::initializeImcTables();
}

ImuseDigiSndMgr::~ImuseDigiSndMgr() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		closeSound(&_sounds[l]);
	}

	delete _cacheBundleDir;
	BundleCodecs::releaseImcTables();
}

void ImuseDigiSndMgr::countElements(byte *ptr, int &numRegions, int &numJumps, int &numSyncs, int &numMarkers) {
	uint32 tag;
	int32 size = 0;

	do {
		tag = READ_BE_UINT32(ptr); ptr += 4;
		switch (tag) {
		case MKTAG('S','T','O','P'):
		case MKTAG('F','R','M','T'):
		case MKTAG('D','A','T','A'):
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		case MKTAG('T','E','X','T'):
			if (!scumm_stricmp((const char *)(ptr + 8), "exit"))
				numMarkers++;
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		case MKTAG('R','E','G','N'):
			numRegions++;
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		case MKTAG('J','U','M','P'):
			numJumps++;
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		case MKTAG('S','Y','N','C'):
			numSyncs++;
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		default:
			error("ImuseDigiSndMgr::countElements() Unknown sfx header '%s'", tag2str(tag));
		}
	} while (tag != MKTAG('D','A','T','A'));
}

void ImuseDigiSndMgr::prepareSoundFromRMAP(Common::SeekableReadStream *file, SoundDesc *sound, int32 offset, int32 size) {
	int l;

	file->seek(offset, SEEK_SET);
	uint32 tag = file->readUint32BE();
	assert(tag == MKTAG('R','M','A','P'));
	int32 version = file->readUint32BE();
	if (version != 3) {
		if (version == 2) {
			warning("ImuseDigiSndMgr::prepareSoundFromRMAP: Wrong version of compressed *.bun file, expected 3, but it's 2");
			warning("Suggested to recompress with latest tool from daily builds");
		} else
			error("ImuseDigiSndMgr::prepareSoundFromRMAP: Wrong version number, expected 3, but it's: %d", version);
	}
	sound->bits = file->readUint32BE();
	sound->freq = file->readUint32BE();
	sound->channels = file->readUint32BE();
	sound->numRegions = file->readUint32BE();
	sound->numJumps = file->readUint32BE();
	sound->numSyncs = file->readUint32BE();
	if (version >= 3)
		sound->numMarkers = file->readUint32BE();
	else
		sound->numMarkers = 0;

	sound->region = new Region[sound->numRegions];
	assert(sound->region);
	sound->jump = new Jump[sound->numJumps];
	assert(sound->jump);
	sound->sync = new Sync[sound->numSyncs];
	assert(sound->sync);
	sound->marker = new Marker[sound->numMarkers];
	assert(sound->marker);

	for (l = 0; l < sound->numRegions; l++) {
		sound->region[l].offset = file->readUint32BE();
		sound->region[l].length = file->readUint32BE();
	}
	for (l = 0; l < sound->numJumps; l++) {
		sound->jump[l].offset = file->readUint32BE();
		sound->jump[l].dest = file->readUint32BE();
		sound->jump[l].hookId = file->readUint32BE();
		sound->jump[l].fadeDelay = file->readUint32BE();
	}
	for (l = 0; l < sound->numSyncs; l++) {
		sound->sync[l].size = file->readUint32BE();
		sound->sync[l].ptr = new byte[sound->sync[l].size];
		file->read(sound->sync[l].ptr, sound->sync[l].size);
	}
	if (version >= 3) {
		for (l = 0; l < sound->numMarkers; l++) {
			sound->marker[l].pos = file->readUint32BE();
			sound->marker[l].length = file->readUint32BE();
			sound->marker[l].ptr = new char[sound->marker[l].length];
			file->read(sound->marker[l].ptr, sound->marker[l].length);
		}
	}
}

void ImuseDigiSndMgr::prepareSound(byte *ptr, SoundDesc *sound) {
	if (READ_BE_UINT32(ptr) == MKTAG('C','r','e','a')) {
		bool quit = false;
		int len;

		int32 offset = READ_LE_UINT16(ptr + 20);
		int16 code = READ_LE_UINT16(ptr + 24);

		sound->numRegions = 0;
		sound->region = new Region[70];
		assert(sound->region);

		sound->numJumps = 0;
		sound->jump = new Jump[1];
		assert(sound->jump);

		sound->numSyncs = 0;

		sound->resPtr = ptr;
		sound->bits = 8;
		sound->channels = 1;

		while (!quit) {
			len = READ_LE_UINT32(ptr + offset);
			code = len & 0xFF;
			if ((code != 0) && (code != 1) && (code != 6) && (code != 7)) {
				// try again with 2 bytes forward (workaround for some FT sounds (ex.362, 363)
				offset += 2;
				len = READ_LE_UINT32(ptr + offset);
				code = len & 0xFF;
				if ((code != 0) && (code != 1) && (code != 6) && (code != 7)) {
					error("Invalid code in VOC file : %d", code);
				}
			}
			offset += 4;
			len >>= 8;
			switch (code) {
			case 0:
				quit = true;
				break;
			case 1:
				{
					int time_constant = ptr[offset];
					offset += 2;
					len -= 2;
					sound->freq = Audio::getSampleRateFromVOCRate(time_constant);
					sound->region[sound->numRegions].offset = offset;
					sound->region[sound->numRegions].length = len;
					sound->numRegions++;
				}
				break;
			case 6:	// begin of loop
				sound->jump[0].dest = offset + 8;
				sound->jump[0].hookId = 0;
				sound->jump[0].fadeDelay = 0;
				break;
			case 7:	// end of loop
				sound->jump[0].offset = offset - 4;
				sound->numJumps++;
				sound->region[sound->numRegions].offset = offset - 4;
				sound->region[sound->numRegions].length = 0;
				sound->numRegions++;
				break;
			default:
				error("Invalid code in VOC file : %d", code);
				quit = true;
				break;
			}
			offset += len;
		}
	} else if (READ_BE_UINT32(ptr) == MKTAG('i','M','U','S')) {
		uint32 tag;
		int32 size = 0;
		byte *s_ptr = ptr;
		ptr += 16;

		int curIndexRegion = 0;
		int curIndexJump = 0;
		int curIndexSync = 0;
		int curIndexMarker = 0;

		sound->numRegions = 0;
		sound->numJumps = 0;
		sound->numSyncs = 0;
		sound->numMarkers = 0;
		countElements(ptr, sound->numRegions, sound->numJumps, sound->numSyncs, sound->numMarkers);
		sound->region = new Region[sound->numRegions];
		assert(sound->region);
		sound->jump = new Jump[sound->numJumps];
		assert(sound->jump);
		sound->sync = new Sync[sound->numSyncs];
		assert(sound->sync);
		sound->marker = new Marker[sound->numMarkers];
		assert(sound->marker);

		do {
			tag = READ_BE_UINT32(ptr); ptr += 4;
			switch (tag) {
			case MKTAG('F','R','M','T'):
				ptr += 12;
				sound->bits = READ_BE_UINT32(ptr); ptr += 4;
				sound->freq = READ_BE_UINT32(ptr); ptr += 4;
				sound->channels = READ_BE_UINT32(ptr); ptr += 4;
				break;
			case MKTAG('T','E','X','T'):
				if (!scumm_stricmp((const char *)(ptr + 8), "exit")) {
					sound->marker[curIndexMarker].pos = READ_BE_UINT32(ptr + 4);
					sound->marker[curIndexMarker].length = strlen((const char *)(ptr + 8)) + 1;
					sound->marker[curIndexMarker].ptr = new char[sound->marker[curIndexMarker].length];
					assert(sound->marker[curIndexMarker].ptr);
					strcpy(sound->marker[curIndexMarker].ptr, (const char *)(ptr + 8));
					curIndexMarker++;
				}
				size = READ_BE_UINT32(ptr); ptr += size + 4;
				break;
			case MKTAG('S','T','O','P'):
				size = READ_BE_UINT32(ptr); ptr += size + 4;
				break;
			case MKTAG('R','E','G','N'):
				ptr += 4;
				sound->region[curIndexRegion].offset = READ_BE_UINT32(ptr); ptr += 4;
				sound->region[curIndexRegion].length = READ_BE_UINT32(ptr); ptr += 4;
				curIndexRegion++;
				break;
			case MKTAG('J','U','M','P'):
				ptr += 4;
				sound->jump[curIndexJump].offset = READ_BE_UINT32(ptr); ptr += 4;
				sound->jump[curIndexJump].dest = READ_BE_UINT32(ptr); ptr += 4;
				sound->jump[curIndexJump].hookId = READ_BE_UINT32(ptr); ptr += 4;
				sound->jump[curIndexJump].fadeDelay = READ_BE_UINT32(ptr); ptr += 4;
				curIndexJump++;
				break;
			case MKTAG('S','Y','N','C'):
				size = READ_BE_UINT32(ptr); ptr += 4;
				sound->sync[curIndexSync].size = size;
				sound->sync[curIndexSync].ptr = new byte[size];
				assert(sound->sync[curIndexSync].ptr);
				memcpy(sound->sync[curIndexSync].ptr, ptr, size);
				curIndexSync++;
				ptr += size;
				break;
			case MKTAG('D','A','T','A'):
				ptr += 4;
				break;
			default:
				error("ImuseDigiSndMgr::prepareSound(%d/%s) Unknown sfx header '%s'", sound->soundId, sound->name, tag2str(tag));
			}
		} while (tag != MKTAG('D','A','T','A'));
		sound->offsetData = ptr - s_ptr;
	} else {
		error("ImuseDigiSndMgr::prepareSound(): Unknown sound format");
	}
}

ImuseDigiSndMgr::SoundDesc *ImuseDigiSndMgr::allocSlot() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (!_sounds[l].inUse) {
			_sounds[l].inUse = true;
			return &_sounds[l];
		}
	}

	return NULL;
}

bool ImuseDigiSndMgr::openMusicBundle(SoundDesc *sound, int &disk) {
	bool result = false;

	sound->bundle = new BundleMgr(_cacheBundleDir);
	assert(sound->bundle);
	if (_vm->_game.id == GID_CMI) {
		if (_vm->_game.features & GF_DEMO) {
			result = sound->bundle->open("music.bun", sound->compressed);
		} else {
			char musicfile[20];
			if (disk == -1)
				disk = _vm->VAR(_vm->VAR_CURRENTDISK);
			sprintf(musicfile, "musdisk%d.bun", disk);
//			if (_disk != _vm->VAR(_vm->VAR_CURRENTDISK)) {
//				_vm->_imuseDigital->parseScriptCmds(0x1000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->parseScriptCmds(0x2000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->stopAllSounds();
//				sound->bundle->closeFile();
//			}

			result = sound->bundle->open(musicfile, sound->compressed, true);

			// FIXME: Shouldn't we only set _disk if result == true?
			_disk = (byte)_vm->VAR(_vm->VAR_CURRENTDISK);
		}
	} else if (_vm->_game.id == GID_DIG)
		result = sound->bundle->open("digmusic.bun", sound->compressed, true);
	else
		error("ImuseDigiSndMgr::openMusicBundle() Don't know which bundle file to load");

	_vm->VAR(_vm->VAR_MUSIC_BUNDLE_LOADED) = result ? 1 : 0;

	return result;
}

bool ImuseDigiSndMgr::openVoiceBundle(SoundDesc *sound, int &disk) {
	bool result = false;

	sound->bundle = new BundleMgr(_cacheBundleDir);
	assert(sound->bundle);
	if (_vm->_game.id == GID_CMI) {
		if (_vm->_game.features & GF_DEMO) {
			result = sound->bundle->open("voice.bun", sound->compressed);
		} else {
			char voxfile[20];
			if (disk == -1)
				disk = _vm->VAR(_vm->VAR_CURRENTDISK);
			sprintf(voxfile, "voxdisk%d.bun", disk);
//			if (_disk != _vm->VAR(_vm->VAR_CURRENTDISK)) {
//				_vm->_imuseDigital->parseScriptCmds(0x1000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->parseScriptCmds(0x2000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->stopAllSounds();
//				sound->bundle->closeFile();
//			}

			result = sound->bundle->open(voxfile, sound->compressed);

			// FIXME: Shouldn't we only set _disk if result == true?
			_disk = (byte)_vm->VAR(_vm->VAR_CURRENTDISK);
		}
	} else if (_vm->_game.id == GID_DIG)
		result = sound->bundle->open("digvoice.bun", sound->compressed);
	else
		error("ImuseDigiSndMgr::openVoiceBundle() Don't know which bundle file to load");

	_vm->VAR(_vm->VAR_VOICE_BUNDLE_LOADED) = result ? 1 : 0;

	return result;
}

ImuseDigiSndMgr::SoundDesc *ImuseDigiSndMgr::openSound(int32 soundId, const char *soundName, int soundType, int volGroupId, int disk) {
	assert(soundId >= 0);
	assert(soundType);

	SoundDesc *sound = allocSlot();
	if (!sound) {
		error("ImuseDigiSndMgr::openSound() can't alloc free sound slot");
	}

	const bool header_outside = ((_vm->_game.id == GID_CMI) && !(_vm->_game.features & GF_DEMO));
	bool result = false;
	byte *ptr = NULL;

	switch (soundType) {
	case IMUSE_RESOURCE:
		assert(soundName[0] == 0);	// Paranoia check

		_vm->ensureResourceLoaded(rtSound, soundId);
		_vm->_res->lock(rtSound, soundId);
		ptr = _vm->getResourceAddress(rtSound, soundId);
		if (ptr == NULL) {
			closeSound(sound);
			return NULL;
		}
		sound->resPtr = ptr;
		break;
	case IMUSE_BUNDLE:
		if (volGroupId == IMUSE_VOLGRP_VOICE)
			result = openVoiceBundle(sound, disk);
		else if (volGroupId == IMUSE_VOLGRP_MUSIC)
			result = openMusicBundle(sound, disk);
		else
			error("ImuseDigiSndMgr::openSound() Don't know how load sound: %d", soundId);
		if (!result) {
			closeSound(sound);
			return NULL;
		}
		if (sound->compressed) {
			char fileName[24];
			int32 offset = 0, size = 0;
			sprintf(fileName, "%s.map", soundName);
			Common::SeekableReadStream *rmapFile = sound->bundle->getFile(fileName, offset, size);
			if (!rmapFile) {
				closeSound(sound);
				return NULL;
			}
			prepareSoundFromRMAP(rmapFile, sound, offset, size);
			strcpy(sound->name, soundName);
			sound->soundId = soundId;
			sound->type = soundType;
			sound->volGroupId = volGroupId;
			sound->disk = disk;
			return sound;
		} else if (soundName[0] == 0) {
			if (sound->bundle->decompressSampleByIndex(soundId, 0, 0x2000, &ptr, 0, header_outside) == 0 || ptr == NULL) {
				closeSound(sound);
				return NULL;
			}
		} else {
			if (sound->bundle->decompressSampleByName(soundName, 0, 0x2000, &ptr, header_outside) == 0 || ptr == NULL) {
				closeSound(sound);
				return NULL;
			}
		}
		sound->resPtr = 0;
		break;
	default:
		error("ImuseDigiSndMgr::openSound() Unknown soundType %d (trying to load sound %d)", soundType, soundId);
	}

	strcpy(sound->name, soundName);
	sound->soundId = soundId;
	sound->type = soundType;
	sound->volGroupId = volGroupId;
	sound->disk = _disk;
	prepareSound(ptr, sound);
	if ((soundType == IMUSE_BUNDLE) && !sound->compressed) {
		free(ptr);
	}
	return sound;
}

void ImuseDigiSndMgr::closeSound(SoundDesc *soundDesc) {
	assert(checkForProperHandle(soundDesc));

	if (soundDesc->resPtr) {
		bool found = false;
		for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
			if ((_sounds[l].soundId == soundDesc->soundId) && (&_sounds[l] != soundDesc))
				found = true;
		}
		if (!found)
			_vm->_res->unlock(rtSound, soundDesc->soundId);
	}

	delete soundDesc->compressedStream;
	delete soundDesc->bundle;

	for (int r = 0; r < soundDesc->numSyncs; r++)
		delete[] soundDesc->sync[r].ptr;
	for (int r = 0; r < soundDesc->numMarkers; r++)
		delete[] soundDesc->marker[r].ptr;
	delete[] soundDesc->region;
	delete[] soundDesc->jump;
	delete[] soundDesc->sync;
	delete[] soundDesc->marker;
	memset(soundDesc, 0, sizeof(SoundDesc));
}

ImuseDigiSndMgr::SoundDesc *ImuseDigiSndMgr::cloneSound(SoundDesc *soundDesc) {
	ImuseDigiSndMgr::SoundDesc *desc;
	assert(checkForProperHandle(soundDesc));

	desc = openSound(soundDesc->soundId, soundDesc->name, soundDesc->type, soundDesc->volGroupId, soundDesc->disk);
	if (!desc)
		desc = openSound(soundDesc->soundId, soundDesc->name, soundDesc->type, soundDesc->volGroupId, 1);
	if (!desc)
		desc = openSound(soundDesc->soundId, soundDesc->name, soundDesc->type, soundDesc->volGroupId, 2);
	return desc;
}

bool ImuseDigiSndMgr::checkForProperHandle(SoundDesc *soundDesc) {
	if (!soundDesc)
		return false;
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (soundDesc == &_sounds[l])
			return true;
	}
	return false;
}

bool ImuseDigiSndMgr::isSndDataExtComp(SoundDesc *soundDesc) {
	assert(checkForProperHandle(soundDesc));
	return soundDesc->compressed;
}

int ImuseDigiSndMgr::getFreq(SoundDesc *soundDesc) {
	assert(checkForProperHandle(soundDesc));
	return soundDesc->freq;
}

int ImuseDigiSndMgr::getBits(SoundDesc *soundDesc) {
	assert(checkForProperHandle(soundDesc));
	return soundDesc->bits;
}

int ImuseDigiSndMgr::getChannels(SoundDesc *soundDesc) {
	assert(checkForProperHandle(soundDesc));
	return soundDesc->channels;
}

bool ImuseDigiSndMgr::isEndOfRegion(SoundDesc *soundDesc, int region) {
	assert(checkForProperHandle(soundDesc));
	assert(region >= 0 && region < soundDesc->numRegions);
	return soundDesc->endFlag;
}

int ImuseDigiSndMgr::getNumRegions(SoundDesc *soundDesc) {
	assert(checkForProperHandle(soundDesc));
	return soundDesc->numRegions;
}

int ImuseDigiSndMgr::getNumJumps(SoundDesc *soundDesc) {
	assert(checkForProperHandle(soundDesc));
	return soundDesc->numJumps;
}

int ImuseDigiSndMgr::getRegionOffset(SoundDesc *soundDesc, int region) {
	debug(5, "getRegionOffset() region:%d", region);
	assert(checkForProperHandle(soundDesc));
	assert(region >= 0 && region < soundDesc->numRegions);
	return soundDesc->region[region].offset;
}

int ImuseDigiSndMgr::getJumpIdByRegionAndHookId(SoundDesc *soundDesc, int region, int hookId) {
	debug(5, "getJumpIdByRegionAndHookId() region:%d, hookId:%d", region, hookId);
	assert(checkForProperHandle(soundDesc));
	assert(region >= 0 && region < soundDesc->numRegions);
	int32 offset = soundDesc->region[region].offset;
	for (int l = 0; l < soundDesc->numJumps; l++) {
		if (offset == soundDesc->jump[l].offset) {
			if (soundDesc->jump[l].hookId == hookId)
				return l;
		}
	}

	return -1;
}

bool ImuseDigiSndMgr::checkForTriggerByRegionAndMarker(SoundDesc *soundDesc, int region, const char *marker) {
	debug(5, "checkForTriggerByRegionAndMarker() region:%d, marker:%s", region, marker);
	assert(checkForProperHandle(soundDesc));
	assert(region >= 0 && region < soundDesc->numRegions);
	assert(marker);
	int32 offset = soundDesc->region[region].offset;
	for (int l = 0; l < soundDesc->numMarkers; l++) {
		if (offset == soundDesc->marker[l].pos) {
			if (!scumm_stricmp(soundDesc->marker[l].ptr, marker))
				return true;
		}
	}

	return false;
}

void ImuseDigiSndMgr::getSyncSizeAndPtrById(SoundDesc *soundDesc, int number, int32 &sync_size, byte **sync_ptr) {
	assert(checkForProperHandle(soundDesc));
	assert(number >= 0);
	if (number < soundDesc->numSyncs) {
		sync_size = soundDesc->sync[number].size;
		*sync_ptr = soundDesc->sync[number].ptr;
	} else {
		sync_size = 0;
		*sync_ptr = NULL;
	}
}

int ImuseDigiSndMgr::getRegionIdByJumpId(SoundDesc *soundDesc, int jumpId) {
	debug(5, "getRegionIdByJumpId() jumpId:%d", jumpId);
	assert(checkForProperHandle(soundDesc));
	assert(jumpId >= 0 && jumpId < soundDesc->numJumps);
	int32 dest = soundDesc->jump[jumpId].dest;
	for (int l = 0; l < soundDesc->numRegions; l++) {
		if (dest == soundDesc->region[l].offset) {
			return l;
		}
	}

	return -1;
}

int ImuseDigiSndMgr::getJumpHookId(SoundDesc *soundDesc, int number) {
	debug(5, "getJumpHookId() number:%d", number);
	assert(checkForProperHandle(soundDesc));
	assert(number >= 0 && number < soundDesc->numJumps);
	return soundDesc->jump[number].hookId;
}

int ImuseDigiSndMgr::getJumpFade(SoundDesc *soundDesc, int number) {
	debug(5, "getJumpFade() number:%d", number);
	assert(checkForProperHandle(soundDesc));
	assert(number >= 0 && number < soundDesc->numJumps);
	return soundDesc->jump[number].fadeDelay;
}

int32 ImuseDigiSndMgr::getDataFromRegion(SoundDesc *soundDesc, int region, byte **buf, int32 offset, int32 size) {
	debug(6, "getDataFromRegion() region:%d, offset:%d, size:%d, numRegions:%d", region, offset, size, soundDesc->numRegions);
	assert(checkForProperHandle(soundDesc));
	assert(buf && offset >= 0 && size >= 0);
	assert(region >= 0 && region < soundDesc->numRegions);

	int32 region_offset = soundDesc->region[region].offset;
	int32 region_length = soundDesc->region[region].length;
	int32 offset_data = soundDesc->offsetData;
	int32 start = region_offset - offset_data;

	if (offset + size + offset_data > region_length) {
		size = region_length - offset;
		soundDesc->endFlag = true;
	} else {
		soundDesc->endFlag = false;
	}

	int header_size = soundDesc->offsetData;
	bool header_outside = ((_vm->_game.id == GID_CMI) && !(_vm->_game.features & GF_DEMO));
	if ((soundDesc->bundle) && (!soundDesc->compressed)) {
		size = soundDesc->bundle->decompressSampleByCurIndex(start + offset, size, buf, header_size, header_outside);
	} else if (soundDesc->resPtr) {
		*buf = (byte *)malloc(size);
		assert(*buf);
		memcpy(*buf, soundDesc->resPtr + start + offset + header_size, size);
	} else if ((soundDesc->bundle) && (soundDesc->compressed)) {
		*buf = (byte *)malloc(size);
		assert(*buf);
		char fileName[24];
		int offsetMs = (((offset * 8 * 10) / soundDesc->bits) / (soundDesc->channels * soundDesc->freq)) * 100;
		sprintf(fileName, "%s_reg%03d", soundDesc->name, region);
		if (scumm_stricmp(fileName, soundDesc->lastFileName) != 0) {
			int32 offs = 0, len = 0;
			Common::SeekableReadStream *cmpFile;
			uint8 soundMode = 0;

			sprintf(fileName, "%s_reg%03d.fla", soundDesc->name, region);
			cmpFile = soundDesc->bundle->getFile(fileName, offs, len);
			if (len) {
#ifndef USE_FLAC
				error("FLAC library compiled support needed");
#endif
				soundMode = 3;
			}
			if (!len) {
				sprintf(fileName, "%s_reg%03d.ogg", soundDesc->name, region);
				cmpFile = soundDesc->bundle->getFile(fileName, offs, len);
				if (len) {
#ifndef USE_VORBIS
					error("Vorbis library compiled support needed");
#endif
					soundMode = 2;
				}
			}
			if (!len) {
				sprintf(fileName, "%s_reg%03d.mp3", soundDesc->name, region);
				cmpFile = soundDesc->bundle->getFile(fileName, offs, len);
				if (len) {
#ifndef USE_MAD
					error("Mad library compiled support needed");
#endif
					soundMode = 1;
				}
			}
			assert(len);

			if (!soundDesc->compressedStream) {
				Common::SeekableReadStream *tmp = cmpFile->readStream(len);
				assert(tmp);
#ifdef USE_FLAC
				if (soundMode == 3)
					soundDesc->compressedStream = Audio::makeFLACStream(tmp, DisposeAfterUse::YES);
#endif
#ifdef USE_VORBIS
				if (soundMode == 2)
					soundDesc->compressedStream = Audio::makeVorbisStream(tmp, DisposeAfterUse::YES);
#endif
#ifdef USE_MAD
				if (soundMode == 1)
					soundDesc->compressedStream = Audio::makeMP3Stream(tmp, DisposeAfterUse::YES);
#endif
				assert(soundDesc->compressedStream);
				soundDesc->compressedStream->seek(offsetMs);
			}
			strcpy(soundDesc->lastFileName, fileName);
		}
		size = soundDesc->compressedStream->readBuffer((int16 *)*buf, size / 2) * 2;
		if (soundDesc->compressedStream->endOfData() || soundDesc->endFlag) {
			delete soundDesc->compressedStream;
			soundDesc->compressedStream = NULL;
			soundDesc->lastFileName[0] = 0;
			soundDesc->endFlag = true;
		}
	}

	return size;
}

} // End of namespace Scumm
