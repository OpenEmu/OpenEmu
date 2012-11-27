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

// Sound resource management class

#include "saga/saga.h"

#include "saga/itedata.h"
#include "saga/resource.h"
#include "saga/sndres.h"
#include "saga/sound.h"

#include "common/file.h"
#include "common/substream.h"

#include "audio/audiostream.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/mac_snd.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"
#ifdef ENABLE_SAGA2
#include "saga/shorten.h"
#endif

namespace Saga {

#define RID_IHNM_SFX_LUT 265
#define RID_IHNMDEMO_SFX_LUT 222

SndRes::SndRes(SagaEngine *vm) : _vm(vm), _sfxContext(NULL), _voiceContext(NULL), _voiceSerial(-1) {

	// Load sound module resource file contexts
	_sfxContext = _vm->_resource->getContext(GAME_SOUNDFILE);
	if (_sfxContext == NULL) {
		error("SndRes::SndRes resource context not found");
	}

	setVoiceBank(0);

	if (_vm->getGameId() == GID_ITE) {
		_fxTable.resize(ITE_SFXCOUNT);
		for (uint i = 0; i < _fxTable.size(); i++) {
			_fxTable[i].res = ITE_SfxTable[i].res;
			_fxTable[i].vol = ITE_SfxTable[i].vol;
		}
#ifdef ENABLE_IHNM
	} else if (_vm->getGameId() == GID_IHNM) {
		ResourceContext *resourceContext;

		resourceContext = _vm->_resource->getContext(GAME_SOUNDFILE);
		if (resourceContext == NULL) {
			error("Resource::loadGlobalResources() resource context not found");
		}

		ByteArray resourceData;

		if (_vm->isIHNMDemo()) {
			_vm->_resource->loadResource(resourceContext, RID_IHNMDEMO_SFX_LUT, resourceData);
		} else {
			_vm->_resource->loadResource(resourceContext, RID_IHNM_SFX_LUT, resourceData);
		}

		if (resourceData.empty()) {
			error("Sndres::SndRes can't read SfxIDs table");
		}

		_fxTableIDs.resize(resourceData.size() / 2);

		ByteArrayReadStreamEndian metaS(resourceData);
		for (uint i = 0; i < _fxTableIDs.size(); i++) {
			_fxTableIDs[i] = metaS.readSint16LE();
		}
#endif
#ifdef ENABLE_SAGA2
	} else if (_vm->getGameId() == GID_DINO) {
		// TODO
	} else if (_vm->getGameId() == GID_FTA2) {
		// TODO
#endif
	}
}

void SndRes::setVoiceBank(int serial) {
	Common::File *file;
	if (_voiceSerial == serial)
		return;

#ifdef ENABLE_IHNM
	// If we got the Macintosh version of IHNM, just set the voice bank
	// so that we know which voices* subfolder to look for later
	if (_vm->getGameId() == GID_IHNM && _vm->isMacResources()) {
		_voiceSerial = serial;
		// Set a dummy voice context
		_voiceContext = new VoiceResourceContext_RES();
		return;
	}
#endif

	// If there are no voice files present, don't set the voice bank
	if (!_vm->_voiceFilesExist)
		return;

	// Close previous voice bank file
	if (_voiceContext != NULL) {
		file = _voiceContext->getFile(NULL);
		if (file->isOpen()) {
			file->close();
		}
	}

	_voiceSerial = serial;

	_voiceContext = _vm->_resource->getContext(GAME_VOICEFILE, _voiceSerial);
}

void SndRes::playSound(uint32 resourceId, int volume, bool loop) {
	SoundBuffer buffer;

	debug(4, "SndRes::playSound %i", resourceId);

	if (!load(_sfxContext, resourceId, buffer, false)) {
		warning("Failed to load sound");
		return;
	}

	_vm->_sound->playSound(buffer, volume, loop, resourceId);
}

void SndRes::playVoice(uint32 resourceId) {
	SoundBuffer buffer;

	if (!(_vm->_voiceFilesExist))
		return;

	if (_vm->getGameId() == GID_IHNM && !(_vm->_voicesEnabled))
		return;

	debug(4, "SndRes::playVoice %i", resourceId);

	if (!load(_voiceContext, resourceId, buffer, false)) {
		warning("Failed to load voice");
		return;
	}

	_vm->_sound->playVoice(buffer);
}

enum GameSoundType {
	kSoundPCM = 0,
	kSoundVOX = 1,
	kSoundVOC = 2,
	kSoundWAV = 3,
	kSoundMP3 = 4,
	kSoundOGG = 5,
	kSoundFLAC = 6,
	kSoundAIFF = 7,
	kSoundShorten = 8,
	kSoundMacSND = 9
};

// Use a macro to read in the sound data based on if we actually want to buffer it or not
#define READ_STREAM(streamSize) \
	(onlyHeader \
	? new Common::SeekableSubReadStream(&readS, readS.pos(), readS.pos() + (streamSize)) \
	: readS.readStream(streamSize))

bool SndRes::load(ResourceContext *context, uint32 resourceId, SoundBuffer &buffer, bool onlyHeader) {
	size_t soundResourceLength;
	bool result = false;
	GameSoundType resourceType = kSoundPCM;
	int rate = 0, size = 0;
	Common::File *file;

	if (resourceId == (uint32)-1) {
		return false;
	}

#ifdef ENABLE_IHNM
	//TODO: move to resource_res so we can use normal "getResourceData" and "getFile" methods
	if (_vm->getGameId() == GID_IHNM && _vm->isMacResources()) {
		char soundFileName[40];
		int dirIndex = resourceId / 64;

		if ((context->fileType() & GAME_VOICEFILE) != 0) {
			if (_voiceSerial == 0) {
				sprintf(soundFileName, "Voices/VoicesS/Voices%d/VoicesS%03x", dirIndex, resourceId);
			} else {
				sprintf(soundFileName, "Voices/Voices%d/Voices%d/Voices%d%03x", _voiceSerial, dirIndex, _voiceSerial, resourceId);
			}
		} else {
			sprintf(soundFileName, "SFX/SFX%d/SFX%03x", dirIndex, resourceId);
		}

		file = new Common::File();

		file->open(soundFileName);
		soundResourceLength = file->size();
	} else
#endif
	{
		ResourceData* resourceData = context->getResourceData(resourceId);
		file = context->getFile(resourceData);

		file->seek(resourceData->offset);
		soundResourceLength = resourceData->size;
	}

	Common::SeekableReadStream &readS = *file;
	bool uncompressedSound = false;

	if (soundResourceLength >= 8) {
		byte header[8];

		readS.read(&header, 8);
		readS.seek(readS.pos() - 8);

		if (!memcmp(header, "Creative", 8)) {
			resourceType = kSoundVOC;
		} else if (!memcmp(header, "RIFF", 4) != 0) {
			resourceType = kSoundWAV;
		} else if (!memcmp(header, "FORM", 4) != 0) {
			resourceType = kSoundAIFF;
		} else if (!memcmp(header, "ajkg", 4) != 0) {
			resourceType = kSoundShorten;
		}

		// If patch data exists for sound resource 4 (used in ITE intro), don't treat this sound as compressed
		// Patch data for this resource is in file p2_a.iaf or p2_a.voc
		if (_vm->getGameId() == GID_ITE && resourceId == 4 && context->getResourceData(resourceId)->patchData != NULL)
			uncompressedSound = true;

		// FIXME: Currently, the SFX.RES file in IHNM cannot be compressed
		if (_vm->getGameId() == GID_IHNM && (context->fileType() & GAME_SOUNDFILE))
			uncompressedSound = true;

		if (context->isCompressed() && !uncompressedSound) {
			if (header[0] == char(0)) {
				resourceType = kSoundMP3;
			} else if (header[0] == char(1)) {
				resourceType = kSoundOGG;
			} else if (header[0] == char(2)) {
				resourceType = kSoundFLAC;
			}
		}

	}

	// Default sound type is 16-bit signed PCM, used in ITE
	byte rawFlags = Audio::FLAG_16BITS;

	if (_vm->getGameId() == GID_ITE) {
		if (context->fileType() & GAME_MACBINARY) {
			// ITE Mac has sound in the Mac snd format
			resourceType = kSoundMacSND;
		} else if (_vm->getFeatures() & GF_8BIT_UNSIGNED_PCM) {	// older ITE demos
			rawFlags |= Audio::FLAG_UNSIGNED;
			rawFlags &= ~Audio::FLAG_16BITS;
		} else if (!uncompressedSound && !scumm_stricmp(context->fileName(), "voicesd.rsc")) {
			// Voice files in newer ITE demo versions are OKI ADPCM (VOX) encoded.
			resourceType = kSoundVOX;
		}
	}

	buffer.stream = 0;

	// Check for LE sounds
	if (!context->isBigEndian())
		rawFlags |= Audio::FLAG_LITTLE_ENDIAN;

	switch (resourceType) {
	case kSoundPCM: {
		// In ITE CD German, some voices are absent and contain just 5 zero bytes.
		// Round down to an even number when the audio is 16-bit so makeRawStream
		// will accept the data (needs to be an even size for 16-bit data).
		// See bug #1256701

		if ((soundResourceLength & 1) && (rawFlags & Audio::FLAG_16BITS))
			soundResourceLength &= ~1;

		Audio::SeekableAudioStream *audStream = Audio::makeRawStream(READ_STREAM(soundResourceLength), 22050, rawFlags);
		buffer.stream = audStream;
		buffer.streamLength = audStream->getLength();
		result = true;
		} break;
	case kSoundVOX:
		buffer.stream = Audio::makeADPCMStream(READ_STREAM(soundResourceLength), DisposeAfterUse::YES, soundResourceLength, Audio::kADPCMOki, 22050, 1);
		buffer.streamLength = Audio::Timestamp(0, soundResourceLength * 2, buffer.stream->getRate());
		result = true;
		break;
	case kSoundMacSND: {
		Audio::SeekableAudioStream *audStream = Audio::makeMacSndStream(READ_STREAM(soundResourceLength), DisposeAfterUse::YES);
		buffer.stream = audStream;
		buffer.streamLength = audStream->getLength();
		result = true;
		} break;
	case kSoundAIFF: {
		Audio::SeekableAudioStream *audStream = Audio::makeAIFFStream(READ_STREAM(soundResourceLength), DisposeAfterUse::YES);
		buffer.stream = audStream;
		buffer.streamLength = audStream->getLength();
		result = true;
		} break;
	case kSoundVOC: {
		Audio::SeekableAudioStream *audStream = Audio::makeVOCStream(READ_STREAM(soundResourceLength), Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
		buffer.stream = audStream;
		buffer.streamLength = audStream->getLength();
		result = true;
		} break;
	case kSoundWAV:
	case kSoundShorten:
		if (resourceType == kSoundWAV) {
			result = Audio::loadWAVFromStream(readS, size, rate, rawFlags);
#ifdef ENABLE_SAGA2
		} else if (resourceType == kSoundShorten) {
			result = loadShortenFromStream(readS, size, rate, rawFlags);
#endif
		}

		if (result) {
			Audio::SeekableAudioStream *audStream = Audio::makeRawStream(READ_STREAM(size), rate, rawFlags);
			buffer.stream = audStream;
			buffer.streamLength = audStream->getLength();
		}
		break;
	case kSoundMP3:
	case kSoundOGG:
	case kSoundFLAC: {
		readS.skip(9); // skip sfx header

		Audio::SeekableAudioStream *audStream = 0;
		Common::SeekableReadStream *memStream = READ_STREAM(soundResourceLength - 9);

		if (resourceType == kSoundMP3) {
#ifdef USE_MAD
			audStream = Audio::makeMP3Stream(memStream, DisposeAfterUse::YES);
#endif
		} else if (resourceType == kSoundOGG) {
#ifdef USE_VORBIS
			audStream = Audio::makeVorbisStream(memStream, DisposeAfterUse::YES);
#endif
		} else /* if (resourceType == kSoundFLAC) */ {
#ifdef USE_FLAC
			audStream = Audio::makeFLACStream(memStream, DisposeAfterUse::YES);
#endif
		}

		if (audStream) {
			buffer.stream = audStream;
			buffer.streamLength = audStream->getLength();
			result = true;
		} else {
			delete memStream;
		}

		} break;
	default:
		error("SndRes::load Unknown sound type");
	}

	if (_vm->getGameId() == GID_IHNM && _vm->isMacResources()) {
		delete file;
	}

	if (onlyHeader) {
		delete buffer.stream;
		buffer.stream = 0;
	}

	return result;
}

#undef READ_STREAM

int SndRes::getVoiceLength(uint32 resourceId) {
	SoundBuffer buffer;

	if (!(_vm->_voiceFilesExist))
		return -1;

	if (!load(_voiceContext, resourceId, buffer, true))
		return -1;

	return buffer.streamLength.msecs();
}

} // End of namespace Saga
