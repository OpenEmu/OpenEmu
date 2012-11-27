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
 * sound functionality
 */

#include "tinsel/sound.h"

#include "tinsel/adpcm.h"
#include "tinsel/dw.h"
#include "tinsel/config.h"
#include "tinsel/music.h"
#include "tinsel/strres.h"
#include "tinsel/tinsel.h"
#include "tinsel/sysvar.h"
#include "tinsel/background.h"

#include "common/endian.h"
#include "common/memstream.h"
#include "common/system.h"

#include "audio/mixer.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/xa.h"


#include "gui/message.h"

namespace Tinsel {

extern LANGUAGE g_sampleLanguage;

//--------------------------- General data ----------------------------------

SoundManager::SoundManager(TinselEngine *vm) :
	//_vm(vm),	// TODO: Enable this once global _vm var is gone
	_sampleIndex(0), _sampleIndexLen(0),
	_soundMode(kVOCMode) {

	for (int i = 0; i < kNumChannels; i++)
		_channels[i].sampleNum = _channels[i].subSample = -1;
}

SoundManager::~SoundManager() {
	free(_sampleIndex);
}

/**
 * Plays the specified sample through the sound driver.
 * @param id			Identifier of sample to be played
 * @param type			type of sound (voice or sfx)
 * @param handle		sound handle
 */
// playSample for DiscWorld 1
bool SoundManager::playSample(int id, Audio::Mixer::SoundType type, Audio::SoundHandle *handle) {
	// Floppy version has no sample file
	if (!_vm->isCD())
		return false;

	// no sample driver?
	if (!_vm->_mixer->isReady())
		return false;

	Channel &curChan = _channels[kChannelTinsel1];

	// stop any currently playing sample
	_vm->_mixer->stopHandle(curChan.handle);

	// make sure id is in range
	assert(id > 0 && id < _sampleIndexLen);

	curChan.sampleNum = id;
	curChan.subSample = 0;

	// get file offset for this sample
	uint32 dwSampleIndex = _sampleIndex[id];

	// move to correct position in the sample file
	_sampleStream.seek(dwSampleIndex);
	if (_sampleStream.eos() || _sampleStream.err() || (uint32)_sampleStream.pos() != dwSampleIndex)
		error(FILE_IS_CORRUPT, _vm->getSampleFile(g_sampleLanguage));

	// read the length of the sample
	uint32 sampleLen = _sampleStream.readUint32();
	if (_sampleStream.eos() || _sampleStream.err())
		error(FILE_IS_CORRUPT, _vm->getSampleFile(g_sampleLanguage));

	if (TinselV1PSX) {
		// Read the stream and create a XA ADPCM audio stream
		Audio::AudioStream *xaStream = Audio::makeXAStream(_sampleStream.readStream(sampleLen), 44100);

		// FIXME: Should set this in a different place ;)
		_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, _vm->_config->_soundVolume);
		//_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
		_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, _vm->_config->_voiceVolume);

		// Play the audio stream
		_vm->_mixer->playStream(type, &curChan.handle, xaStream);
	} else {
		// allocate a buffer
		byte *sampleBuf = (byte *)malloc(sampleLen);
		assert(sampleBuf);

		// read all of the sample
		if (_sampleStream.read(sampleBuf, sampleLen) != sampleLen)
			error(FILE_IS_CORRUPT, _vm->getSampleFile(g_sampleLanguage));

		// FIXME: Should set this in a different place ;)
		_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, _vm->_config->_soundVolume);
		//_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
		_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, _vm->_config->_voiceVolume);

		Audio::AudioStream *sampleStream = 0;

		// play it
		switch (_soundMode) {
		case kMP3Mode:
#ifdef USE_MAD
			{
			Common::MemoryReadStream *compressedStream =
				new Common::MemoryReadStream(sampleBuf, sampleLen, DisposeAfterUse::YES);
			sampleStream = Audio::makeMP3Stream(compressedStream, DisposeAfterUse::YES);
			}
#endif
			break;
		case kVorbisMode:
#ifdef USE_VORBIS
			{
			Common::MemoryReadStream *compressedStream =
				new Common::MemoryReadStream(sampleBuf, sampleLen, DisposeAfterUse::YES);
			sampleStream = Audio::makeVorbisStream(compressedStream, DisposeAfterUse::YES);
			}
#endif
			break;
		case kFLACMode:
#ifdef USE_FLAC
			{
			Common::MemoryReadStream *compressedStream =
				new Common::MemoryReadStream(sampleBuf, sampleLen, DisposeAfterUse::YES);
			sampleStream = Audio::makeFLACStream(compressedStream, DisposeAfterUse::YES);
			}
#endif
			break;
		default:
			sampleStream = Audio::makeRawStream(sampleBuf, sampleLen, 22050, Audio::FLAG_UNSIGNED);
			break;
		}
		if (sampleStream) {
			_vm->_mixer->playStream(type, &curChan.handle, sampleStream);
		}
	}

	if (handle)
		*handle = curChan.handle;

	return true;
}

// playSample for DiscWorld 2
bool SoundManager::playSample(int id, int sub, bool bLooped, int x, int y, int priority,
		Audio::Mixer::SoundType type, Audio::SoundHandle *handle) {

	// Floppy version has no sample file
	if (!_vm->isCD())
		return false;

	// no sample driver?
	if (!_vm->_mixer->isReady())
		return false;

	Channel *curChan;

	uint8 sndVol = 255;

	// Sample on screen?
	if (!offscreenChecks(x, y))
		return false;

	// If that sample is already playing, stop it
	stopSpecSample(id, sub);

	if (type == Audio::Mixer::kSpeechSoundType) {
		curChan = &_channels[kChannelTalk];
	} else if (type == Audio::Mixer::kSFXSoundType) {
		uint32 oldestTime = g_system->getMillis();
		int	oldestChan = kChannelSFX;

		int chan;
		for (chan = kChannelSFX; chan < kNumChannels; chan++) {
			if (!_vm->_mixer->isSoundHandleActive(_channels[chan].handle))
				break;

			if ((_channels[chan].lastStart <  oldestTime) &&
			    (_channels[chan].priority  <= priority)) {

				oldestTime = _channels[chan].lastStart;
				oldestChan = chan;
			}
		}

		if (chan == kNumChannels) {
			if (_channels[oldestChan].priority > priority) {
				warning("playSample: No free channel");
				return false;
			}

			chan = oldestChan;
		}

		if (_vm->_pcmMusic->isDimmed() && SysVar(SYS_SceneFxDimFactor))
			sndVol = 255 - 255/SysVar(SYS_SceneFxDimFactor);

		curChan = &_channels[chan];
	} else {
		warning("playSample: Unknown SoundType");
		return false;
	}

	// stop any currently playing sample
	_vm->_mixer->stopHandle(curChan->handle);

	// make sure id is in range
	assert(id > 0 && id < _sampleIndexLen);

	// get file offset for this sample
	uint32 dwSampleIndex = _sampleIndex[id];

	if (dwSampleIndex == 0) {
		warning("Tinsel2 playSample, non-existant sample %d", id);
		return false;
	}

	// move to correct position in the sample file
	_sampleStream.seek(dwSampleIndex);
	if (_sampleStream.eos() || _sampleStream.err() || (uint32)_sampleStream.pos() != dwSampleIndex)
		error(FILE_IS_CORRUPT, _vm->getSampleFile(g_sampleLanguage));

	// read the length of the sample
	uint32 sampleLen = _sampleStream.readUint32();
	if (_sampleStream.eos() || _sampleStream.err())
		error(FILE_IS_CORRUPT, _vm->getSampleFile(g_sampleLanguage));

	if (sampleLen & 0x80000000) {
		// Has sub samples

		int32 numSubs = sampleLen & ~0x80000000;

		assert(sub >= 0 && sub < numSubs);

		// Skipping
		for (int32 i = 0; i < sub; i++) {
			sampleLen = _sampleStream.readUint32();
			_sampleStream.skip(sampleLen);
			if (_sampleStream.eos() || _sampleStream.err())
				error(FILE_IS_CORRUPT, _vm->getSampleFile(g_sampleLanguage));
		}
		sampleLen = _sampleStream.readUint32();
		if (_sampleStream.eos() || _sampleStream.err())
			error(FILE_IS_CORRUPT, _vm->getSampleFile(g_sampleLanguage));
	}

	debugC(DEBUG_DETAILED, kTinselDebugSound, "Playing sound %d.%d, %d bytes at %d (pan %d)", id, sub, sampleLen,
			_sampleStream.pos(), getPan(x));

	// allocate a buffer
	byte *sampleBuf = (byte *) malloc(sampleLen);
	assert(sampleBuf);

	// read all of the sample
	if (_sampleStream.read(sampleBuf, sampleLen) != sampleLen)
		error(FILE_IS_CORRUPT, _vm->getSampleFile(g_sampleLanguage));

	Common::MemoryReadStream *compressedStream =
		new Common::MemoryReadStream(sampleBuf, sampleLen, DisposeAfterUse::YES);
	Audio::AudioStream *sampleStream = 0;

	switch (_soundMode) {
	case kMP3Mode:
#ifdef USE_MAD
		sampleStream = Audio::makeMP3Stream(compressedStream, DisposeAfterUse::YES);
#endif
		break;
	case kVorbisMode:
#ifdef USE_VORBIS
		sampleStream = Audio::makeVorbisStream(compressedStream, DisposeAfterUse::YES);
#endif
		break;
	case kFLACMode:
#ifdef USE_FLAC
		sampleStream = Audio::makeFLACStream(compressedStream, DisposeAfterUse::YES);
#endif
		break;
	default:
		sampleStream = new Tinsel6_ADPCMStream(compressedStream, DisposeAfterUse::YES, sampleLen, 22050, 1, 24);
		break;
	}

	// FIXME: Should set this in a different place ;)
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, _vm->_config->_soundVolume);
	//_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, _vm->_config->_voiceVolume);

	curChan->sampleNum = id;
	curChan->subSample = sub;
	curChan->looped = bLooped;
	curChan->x = x;
	curChan->y = y;
	curChan->priority = priority;
	curChan->lastStart = g_system->getMillis();
	//                         /---Compression----\    Milis   BytesPerSecond
	// not needed and won't work when using MP3/OGG/FLAC anyway
	//curChan->timeDuration = (((sampleLen * 64) / 25) * 1000) / (22050 * 2);

	// Play it
	_vm->_mixer->playStream(type, &curChan->handle, sampleStream);

	_vm->_mixer->setChannelVolume(curChan->handle, sndVol);
	_vm->_mixer->setChannelBalance(curChan->handle, getPan(x));

	if (handle)
		*handle = curChan->handle;

	return true;
}

/**
 * Returns FALSE if sample doesn't need playing
 */
bool SoundManager::offscreenChecks(int x, int &y) {
	// No action if no x specification
	if (x == -1)
		return true;

	// convert x to offset from screen center
	x -= PlayfieldGetCenterX(FIELD_WORLD);

	if (x < -SCREEN_WIDTH || x > SCREEN_WIDTH) {
		// A long way offscreen, ignore it
		return false;
	} else if (x < -SCREEN_WIDTH/2 || x > SCREEN_WIDTH/2) {
		// Off-screen, attennuate it

		y = (y > 0) ? (y / 2) : 50;

		return true;
	} else
		return true;
}

int8 SoundManager::getPan(int x) {

	if (x == -1)
		return 0;

	x -= PlayfieldGetCenterX(FIELD_WORLD);

	if (x == 0)
		return 0;

	if (x < 0) {
		if (x < (-SCREEN_WIDTH / 2))
			return -127;

		x = (-x * 127) / (SCREEN_WIDTH / 2);

		return 0 - x;
	}

	if (x > (SCREEN_WIDTH / 2))
		return 127;

	x = (x * 127) / (SCREEN_WIDTH / 2);

	return x;
}

/**
 * Returns TRUE if there is a sample for the specified sample identifier.
 * @param id			Identifier of sample to be checked
 */
bool SoundManager::sampleExists(int id) {
	if (_vm->_mixer->isReady())	{
		// make sure id is in range
		if (id > 0 && id < _sampleIndexLen) {
			// check for a sample index
			if (_sampleIndex[id])
				return true;
		}
	}

	// no sample driver or no sample
	return false;
}

/**
 * Returns true if a sample is currently playing.
 */
bool SoundManager::sampleIsPlaying(int id) {
	if (!TinselV2)
		return _vm->_mixer->isSoundHandleActive(_channels[kChannelTinsel1].handle);

	for (int i = 0; i < kNumChannels; i++)
		if (_channels[i].sampleNum == id)
			if (_vm->_mixer->isSoundHandleActive(_channels[i].handle))
				return true;

	return false;
}

/**
 * Stops any currently playing sample.
 */
void SoundManager::stopAllSamples() {
	// stop currently playing sample

	if (!TinselV2) {
		_vm->_mixer->stopHandle(_channels[kChannelTinsel1].handle);
		return;
	}

	for (int i = 0; i < kNumChannels; i++)
		_vm->_mixer->stopHandle(_channels[i].handle);
}

void SoundManager::stopSpecSample(int id, int sub) {
	debugC(DEBUG_DETAILED, kTinselDebugSound, "stopSpecSample(%d, %d)", id, sub);

	if (!TinselV2) {
		if (_channels[kChannelTinsel1].sampleNum == id)
			_vm->_mixer->stopHandle(_channels[kChannelTinsel1].handle);
		return;
	}

	for (int i = kChannelTalk; i < kNumChannels; i++) {
		if ((_channels[i].sampleNum == id) && (_channels[i].subSample == sub))
			_vm->_mixer->stopHandle(_channels[i].handle);
	}
}

void SoundManager::setSFXVolumes(uint8 volume) {
	if (!TinselV2)
		return;

	for (int i = kChannelSFX; i < kNumChannels; i++)
		_vm->_mixer->setChannelVolume(_channels[i].handle, volume);
}

/**
 * Opens and inits all sound sample files.
 */
void SoundManager::openSampleFiles() {
	// Floppy and demo versions have no sample files, except for the Discworld 2 demo
	if (!_vm->isCD() || TinselV0)
		return;

	TinselFile f;

	if (_sampleIndex)
		// already allocated
		return;

	// open sample index file in binary mode
	if (f.open(_vm->getSampleIndex(g_sampleLanguage)))	{
		// get length of index file
		f.seek(0, SEEK_END);		// move to end of file
		_sampleIndexLen = f.pos();	// get file pointer
		f.seek(0, SEEK_SET);		// back to beginning

		if (_sampleIndex == NULL) {
			// allocate a buffer for the indices
			_sampleIndex = (uint32 *)malloc(_sampleIndexLen);

			// make sure memory allocated
			if (_sampleIndex == NULL) {
				// disable samples if cannot alloc buffer for indices
				// TODO: Disabled sound if we can't load the sample index?
				return;
			}
		}

		// load data
		if (f.read(_sampleIndex, _sampleIndexLen) != (uint32)_sampleIndexLen)
			// file must be corrupt if we get to here
			error(FILE_IS_CORRUPT, _vm->getSampleFile(g_sampleLanguage));

		// close the file
		f.close();

		// convert file size to size in DWORDs
		_sampleIndexLen /= sizeof(uint32);

#ifdef SCUMM_BIG_ENDIAN
		// Convert all ids from LE to native format
		for (int i = 0; i < _sampleIndexLen; ++i) {
			_sampleIndex[i] = SWAP_BYTES_32(_sampleIndex[i]);
		}
#endif

		// Detect format of soundfile by looking at 1st sample-index
		switch (TO_BE_32(_sampleIndex[0])) {
		case MKTAG('M','P','3',' '):
			debugC(DEBUG_DETAILED, kTinselDebugSound, "Detected MP3 sound-data");
			_soundMode = kMP3Mode;
			break;

		case MKTAG('O','G','G',' '):
			debugC(DEBUG_DETAILED, kTinselDebugSound, "Detected OGG sound-data");
			_soundMode = kVorbisMode;
			break;

		case MKTAG('F','L','A','C'):
			debugC(DEBUG_DETAILED, kTinselDebugSound, "Detected FLAC sound-data");
			_soundMode = kFLACMode;
			break;

		default:
			debugC(DEBUG_DETAILED, kTinselDebugSound, "Detected original sound-data");
			break;
		}
		// Normally the 1st sample-index points to nothing at all
		_sampleIndex[0] = 0;
	} else {
		char buf[50];
		sprintf(buf, CANNOT_FIND_FILE, _vm->getSampleIndex(g_sampleLanguage));
		GUI::MessageDialog dialog(buf, "OK");
		dialog.runModal();

		error(CANNOT_FIND_FILE, _vm->getSampleIndex(g_sampleLanguage));
	}

	// open sample file in binary mode
	if (!_sampleStream.open(_vm->getSampleFile(g_sampleLanguage))) {
		char buf[50];
		sprintf(buf, CANNOT_FIND_FILE, _vm->getSampleFile(g_sampleLanguage));
		GUI::MessageDialog dialog(buf, "OK");
		dialog.runModal();

		error(CANNOT_FIND_FILE, _vm->getSampleFile(g_sampleLanguage));
	}

/*
	// gen length of the largest sample
	sampleBuffer.size = _sampleStream.readUint32LE();
	if (_sampleStream.eos() || _sampleStream.err())
		error(FILE_IS_CORRUPT, _vm->getSampleFile(g_sampleLanguage));
*/
}

void SoundManager::closeSampleStream() {
	_sampleStream.close();
	free(_sampleIndex);
	_sampleIndex = 0;
	_sampleIndexLen = 0;
}

} // End of namespace Tinsel
