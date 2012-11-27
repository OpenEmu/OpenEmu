/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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

// One feature still missing is the original's DipMusic() function which, as
// far as I can understand, softened the music volume when someone was
// speaking, but only (?) if the music was playing loudly at the time.
//
// All things considered, I think this is more bother than it's worth.


#include "common/file.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/decoders/mp3.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/xa.h"
#include "audio/rate.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/resman.h"
#include "sword2/sound.h"

namespace Sword2 {

static Audio::AudioStream *makeCLUStream(Common::File *fp, int size);
static Audio::AudioStream *makePSXCLUStream(Common::File *fp, int size);

static Audio::AudioStream *getAudioStream(SoundFileHandle *fh, const char *base, int cd, uint32 id, uint32 *numSamples) {
	bool alreadyOpen;

	if (!fh->file.isOpen()) {
		alreadyOpen = false;

		struct {
			const char *ext;
			int mode;
		} file_types[] = {
#ifdef USE_FLAC
			{ "clf", kFLACMode },
#endif
#ifdef USE_VORBIS
			{ "clg", kVorbisMode },
#endif
#ifdef USE_MAD
			{ "cl3", kMP3Mode },
#endif
			{ "clu", kCLUMode }
		};

		int soundMode = 0;
		char filename[20];

		for (int i = 0; i < ARRAYSIZE(file_types); i++) {
			sprintf(filename, "%s%d.%s", base, cd, file_types[i].ext);
			if (Common::File::exists(filename)) {
				soundMode = file_types[i].mode;
				break;
			}

			sprintf(filename, "%s.%s", base, file_types[i].ext);
			if (Common::File::exists(filename)) {
				soundMode = file_types[i].mode;
				break;
			}
		}

		if (soundMode == 0)
			return NULL;

		fh->file.open(filename);
		fh->fileType = soundMode;
		if (!fh->file.isOpen()) {
			warning("BS2 getAudioStream: Failed opening file '%s'", filename);
			return NULL;
		}
		if (fh->fileSize != fh->file.size()) {
			free(fh->idxTab);
			fh->idxTab = NULL;
		}
	} else
		alreadyOpen = true;

	uint32 entrySize = (fh->fileType == kCLUMode) ? 2 : 3;

	if (!fh->idxTab) {
		fh->file.seek(0);
		fh->idxLen = fh->file.readUint32LE();
		fh->file.seek(entrySize * 4);

		fh->idxTab = (uint32 *)malloc(fh->idxLen * 3 * sizeof(uint32));
		for (uint32 cnt = 0; cnt < fh->idxLen; cnt++) {
			fh->idxTab[cnt * 3 + 0] = fh->file.readUint32LE();
			fh->idxTab[cnt * 3 + 1] = fh->file.readUint32LE();
			if (fh->fileType == kCLUMode) {
				fh->idxTab[cnt * 3 + 2] = fh->idxTab[cnt * 3 + 1];
				fh->idxTab[cnt * 3 + 1]--;
			} else
				fh->idxTab[cnt * 3 + 2] = fh->file.readUint32LE();
		}
	}

	// FIXME: In the forest maze on Zombie Island, the scripts will often
	// try to play song 451, which doesn't exist. We could easily substitute
	// another for it here, but which one? There are roughly 250 musical
	// cues to choose from.

	uint32 pos = fh->idxTab[id * 3 + 0];
	uint32 len = fh->idxTab[id * 3 + 1];
	uint32 enc_len = fh->idxTab[id * 3 + 2];

	if (numSamples)
		*numSamples = len;

	if (!pos || !len) {
		// We couldn't find the sound. Possibly as a result of a bad
		// installation (e.g. using the music file from CD 2 as the
		// first music file). Don't close the file if it was already
		// open though, because something is playing from it.
		warning("getAudioStream: Could not find %s ID %d! Possibly the wrong file", base, id);
		if (!alreadyOpen)
			fh->file.close();
		return NULL;
	}

	fh->file.seek(pos, SEEK_SET);

	switch (fh->fileType) {
	case kCLUMode:
		if (Sword2Engine::isPsx())
			return makePSXCLUStream(&fh->file, enc_len);
		else
			return makeCLUStream(&fh->file, enc_len);
#ifdef USE_MAD
	case kMP3Mode: {
		Common::SafeSeekableSubReadStream *tmp = new Common::SafeSeekableSubReadStream(&fh->file, pos, pos + enc_len);
		return Audio::makeMP3Stream(tmp, DisposeAfterUse::YES);
		}
#endif
#ifdef USE_VORBIS
	case kVorbisMode: {
		Common::SafeSeekableSubReadStream *tmp = new Common::SafeSeekableSubReadStream(&fh->file, pos, pos + enc_len);
		return Audio::makeVorbisStream(tmp, DisposeAfterUse::YES);
		}
#endif
#ifdef USE_FLAC
	case kFLACMode: {
		Common::SafeSeekableSubReadStream *tmp = new Common::SafeSeekableSubReadStream(&fh->file, pos, pos + enc_len);
		return Audio::makeFLACStream(tmp, DisposeAfterUse::YES);
		}
#endif
	default:
		return NULL;
	}
}

// ----------------------------------------------------------------------------
// Custom AudioStream class to handle Broken Sword 2's audio compression.
// ----------------------------------------------------------------------------

#define GetCompressedShift(n)      (((n) >> 4) & 0x0F)
#define GetCompressedSign(n)       ((n) & 0x08)
#define GetCompressedAmplitude(n)  ((n) & 0x07)

CLUInputStream::CLUInputStream(Common::File *file, int size)
	: _file(file), _firstTime(true), _bufferEnd(_outbuf + BUFFER_SIZE) {

	// Determine the end position.
	_file_pos = _file->pos();
	_end_pos = _file_pos + size;

	// Read in initial data
	refill();
}

CLUInputStream::~CLUInputStream() {
}

int CLUInputStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples - samples, (int)(_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			refill();
		}
	}
	return samples;
}

void CLUInputStream::refill() {
	byte *in = _inbuf;
	int16 *out = _outbuf;

	_file->seek(_file_pos, SEEK_SET);

	uint len_left = _file->read(in, MIN((uint32)BUFFER_SIZE, _end_pos - _file->pos()));

	_file_pos = _file->pos();

	while (len_left > 0) {
		uint16 sample;

		if (_firstTime) {
			_firstTime = false;
			_prev = READ_LE_UINT16(in);
			sample = _prev;
			len_left -= 2;
			in += 2;
		} else {
			uint16 delta = GetCompressedAmplitude(*in) << GetCompressedShift(*in);
			if (GetCompressedSign(*in))
				sample = _prev - delta;
			else
				sample = _prev + delta;

			_prev = sample;
			len_left--;
			in++;
		}

		*out++ = (int16)sample;
	}

	_pos = _outbuf;
	_bufferEnd = out;
}

Audio::AudioStream *makeCLUStream(Common::File *file, int size) {
	return new CLUInputStream(file, size);
}

Audio::AudioStream *makePSXCLUStream(Common::File *file, int size) {

	// Buffer audio file data, and ask MemoryReadStream to dispose of it
	// when not needed anymore.

	byte *buffer = (byte *)malloc(size);
	file->read(buffer, size);
	return Audio::makeXAStream(new Common::MemoryReadStream(buffer, size, DisposeAfterUse::YES), 11025);
}

// ----------------------------------------------------------------------------
// Another custom AudioStream class, to wrap around the various AudioStream
// classes used for music decompression, and to add looping, fading, etc.
// ----------------------------------------------------------------------------

// The length of a fade-in/out, in milliseconds.
#define FADE_LENGTH 3000

MusicInputStream::MusicInputStream(int cd, SoundFileHandle *fh, uint32 musicId, bool looping) {
	_cd = cd;
	_fh = fh;
	_musicId = musicId;
	_looping = looping;

	_bufferEnd = _buffer + BUFFER_SIZE;
	_remove = false;
	_fading = 0;

	_decoder = getAudioStream(_fh, "music", _cd, _musicId, &_numSamples);
	if (_decoder) {
		_samplesLeft = _numSamples;
		_fadeSamples = (getRate() * FADE_LENGTH) / 1000;
		fadeUp();

		// Read in initial data
		refill();
	}
}

MusicInputStream::~MusicInputStream() {
	delete _decoder;
	_decoder = NULL;
}

int MusicInputStream::readBuffer(int16 *buffer, const int numSamples) {
	if (!_decoder)
		return 0;

	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples - samples, (int)(_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			refill();
		}
	}
	return samples;
}

void MusicInputStream::refill() {
	int16 *buf = _buffer;
	uint32 numSamples = 0;
	uint32 len_left;
	bool endFade = false;

	len_left = BUFFER_SIZE;

	if (_fading > 0 && (uint32)_fading < len_left)
		len_left = _fading;

	if (_samplesLeft < len_left)
		len_left = _samplesLeft;

	if (!_looping) {
		// Non-looping music is faded out at the end. If this fade
		// out would have started somewhere within the len_left samples
		// to read, we only read up to that point. This way, we can
		// treat this fade as any other.

		if (!_fading) {
			uint32 currentlyAt = _numSamples - _samplesLeft;
			uint32 fadeOutAt = _numSamples - _fadeSamples;
			uint32 readTo = currentlyAt + len_left;

			if (fadeOutAt == currentlyAt)
				fadeDown();
			else if (fadeOutAt > currentlyAt && fadeOutAt <= readTo) {
				len_left = fadeOutAt - currentlyAt;
				endFade = true;
			}
		}
	}

	int desired = len_left - numSamples;
	int len = _decoder->readBuffer(buf, desired);

	// Shouldn't happen, but if it does it could cause an infinite loop.
	// Of course there were bugs that caused it to happen several times
	// during development. :-)

	if (len < desired) {
		warning("Expected %d samples, but got %d", desired, len);
		_samplesLeft = len;
	}

	buf += len;
	numSamples += len;
	len_left -= len;
	_samplesLeft -= len;

	int16 *ptr;

	if (_fading > 0) {
		// Fade down
		for (ptr = _buffer; ptr < buf; ptr++) {
			if (_fading > 0) {
				_fading--;
				*ptr = (*ptr * _fading) / _fadeSamples;
			}
			if (_fading == 0) {
				_looping = false;
				_remove = true;
				*ptr = 0;
			}
		}
	} else if (_fading < 0) {
		// Fade up
		for (ptr = _buffer; ptr < buf; ptr++) {
			_fading--;
			*ptr = -(*ptr * _fading) / _fadeSamples;
			if (_fading <= -_fadeSamples) {
				_fading = 0;
				break;
			}
		}
	}

	if (endFade)
		fadeDown();

	if (!_samplesLeft) {
		if (_looping) {
			delete _decoder;
			_decoder = getAudioStream(_fh, "music", _cd, _musicId, &_numSamples);
			_samplesLeft = _numSamples;
		} else
			_remove = true;
	}

	_pos = _buffer;
	_bufferEnd = buf;
}

void MusicInputStream::fadeUp() {
	if (_fading > 0)
		_fading = -_fading;
	else if (_fading == 0)
		_fading = -1;
}

void MusicInputStream::fadeDown() {
	if (_fading < 0)
		_fading = -_fading;
	else if (_fading == 0)
		_fading = _fadeSamples;
}

bool MusicInputStream::readyToRemove() {
	return _remove;
}

int32 MusicInputStream::getTimeRemaining() {
	// This is far from exact, but it doesn't have to be.
	return (_samplesLeft + BUFFER_SIZE) / getRate();
}

// ----------------------------------------------------------------------------
// Main sound class
// ----------------------------------------------------------------------------

// AudioStream API

int Sound::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);
	int i;

	if (_musicPaused)
		return 0;

	for (i = 0; i < MAXMUS; i++) {
		if (_music[i] && _music[i]->readyToRemove()) {
			delete _music[i];
			_music[i] = NULL;
		}
	}

	memset(buffer, 0, 2 * numSamples);

	if (!_mixBuffer || numSamples > _mixBufferLen) {
		if (_mixBuffer) {
			int16 *newBuffer = (int16 *)realloc(_mixBuffer, 2 * numSamples);
			if (newBuffer) {
				_mixBuffer = newBuffer;
			} else {
				// We can't use the old buffer any more. It's too small.
				free(_mixBuffer);
				_mixBuffer = 0;
			}
		} else
			_mixBuffer = (int16 *)malloc(2 * numSamples);

		_mixBufferLen = numSamples;
	}

	if (!_mixBuffer)
		return 0;

	for (i = 0; i < MAXMUS; i++) {
		if (!_music[i])
			continue;

		int len = _music[i]->readBuffer(_mixBuffer, numSamples);

		if (!_musicMuted) {
			for (int j = 0; j < len; j++) {
				Audio::clampedAdd(buffer[j], _mixBuffer[j]);
			}
		}
	}

	bool inUse[MAXMUS];

	for (i = 0; i < MAXMUS; i++)
		inUse[i] = false;

	for (i = 0; i < MAXMUS; i++) {
		if (_music[i]) {
			if (_music[i]->getCD() == 1)
				inUse[0] = true;
			else
				inUse[1] = true;
		}
	}

	for (i = 0; i < MAXMUS; i++) {
		if (!inUse[i] && !_musicFile[i].inUse && _musicFile[i].file.isOpen())
			_musicFile[i].file.close();
	}

	return numSamples;
}

bool Sound::endOfData() const {
	// The music never stops. It just goes quiet.
	return false;
}

// ----------------------------------------------------------------------------
// MUSIC
// ----------------------------------------------------------------------------

/**
 * Stops the music dead in its tracks. Any music that is currently being
 * streamed is paused.
 */

void Sound::pauseMusic() {
	Common::StackLock lock(_mutex);

	_musicPaused = true;
}

/**
 * Restarts the music from where it was stopped.
 */

void Sound::unpauseMusic() {
	Common::StackLock lock(_mutex);

	_musicPaused = false;
}

/**
 * Fades out and stops the music.
 */

void Sound::stopMusic(bool immediately) {
	Common::StackLock lock(_mutex);

	_loopingMusicId = 0;

	for (int i = 0; i < MAXMUS; i++) {
		if (_music[i]) {
			if (immediately) {
				delete _music[i];
				_music[i] = NULL;
			} else
				_music[i]->fadeDown();
		}
	}
}

/**
 * Streams music from a cluster file.
 * @param musicId the id of the music to stream
 * @param loop true if the music is to loop back to the start
 * @return RD_OK or an error code
 */
int32 Sound::streamCompMusic(uint32 musicId, bool loop) {
	Common::StackLock lock(_mutex);

	int cd = _vm->_resman->getCD();

	if (loop)
		_loopingMusicId = musicId;
	else
		_loopingMusicId = 0;

	int primary = -1;
	int secondary = -1;

	// If both music streams are active, one of them will have to go.

	if (_music[0] && _music[1]) {
		int32 fade0 = _music[0]->isFading();
		int32 fade1 = _music[1]->isFading();

		if (!fade0 && !fade1) {
			// Neither is fading. This shouldn't happen, so just
			// pick one and be done with it.
			primary = 0;
		} else if (fade0 && !fade1) {
			// Stream 0 is fading, so pick that one.
			primary = 0;
		} else if (!fade0 && fade1) {
			// Stream 1 is fading, so pick that one.
			primary = 1;
		} else {
			// Both streams are fading. Pick the one that is
			// closest to silent.
			if (ABS(fade0) < ABS(fade1))
				primary = 0;
			else
				primary = 1;
		}

		delete _music[primary];
		_music[primary] = NULL;
	}

	// Pick the available music stream. If no music is playing it doesn't
	// matter which we use.

	if (_music[0] || _music[1]) {
		if (_music[0]) {
			primary = 1;
			secondary = 0;
		} else {
			primary = 0;
			secondary = 1;
		}
	} else
		primary = 0;

	// Don't start streaming if the volume is off.
	if (isMusicMute()) {
		return RD_OK;
	}

	if (secondary != -1)
		_music[secondary]->fadeDown();
	SoundFileHandle *fh = (cd == 1) ? &_musicFile[0] : &_musicFile[1];
	fh->inUse = true;

	MusicInputStream *tmp = new MusicInputStream(cd, fh, musicId, loop);

	if (tmp->isReady()) {
		_music[primary] = tmp;
		fh->inUse = false;
		return RD_OK;
	} else {
		fh->inUse = false;
		delete tmp;
		return RDERR_INVALIDFILENAME;
	}
}

/**
 * @return the time left for the current music, in seconds.
 */

int32 Sound::musicTimeRemaining() {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < MAXMUS; i++) {
		if (_music[i] && _music[i]->isFading() <= 0)
			return _music[i]->getTimeRemaining();
	}

	return 0;
}

// ----------------------------------------------------------------------------
// SPEECH
// ----------------------------------------------------------------------------

/**
 * Mutes/Unmutes the speech.
 * @param mute If mute is false, restore the volume to the last set master
 * level. Otherwise the speech is muted (volume 0).
 */

void Sound::muteSpeech(bool mute) {
	_speechMuted = mute;

	if (_vm->_mixer->isSoundHandleActive(_soundHandleSpeech)) {
		uint volume = mute ? 0 : Audio::Mixer::kMaxChannelVolume;

		_vm->_mixer->setChannelVolume(_soundHandleSpeech, volume);
	}
}

/**
 * Stops the speech dead in its tracks.
 */

void Sound::pauseSpeech() {
	if (!_speechPaused) {
		_speechPaused = true;
		_vm->_mixer->pauseHandle(_soundHandleSpeech, true);
	}
}

/**
 * Restarts the speech from where it was stopped.
 */

void Sound::unpauseSpeech() {
	if (_speechPaused) {
		_speechPaused = false;
		_vm->_mixer->pauseHandle(_soundHandleSpeech, false);
	}
}

/**
 * Stops the speech from playing.
 */

int32 Sound::stopSpeech() {
	if (_vm->_mixer->isSoundHandleActive(_soundHandleSpeech)) {
		_vm->_mixer->stopHandle(_soundHandleSpeech);
		return RD_OK;
	}

	return RDERR_SPEECHNOTPLAYING;
}

/**
 * @return Either RDSE_SAMPLEPLAYING or RDSE_SAMPLEFINISHED
 */

int32 Sound::getSpeechStatus() {
	return _vm->_mixer->isSoundHandleActive(_soundHandleSpeech) ? RDSE_SAMPLEPLAYING : RDSE_SAMPLEFINISHED;
}

/**
 * Returns either RDSE_QUIET or RDSE_SPEAKING
 */

int32 Sound::amISpeaking() {
	if (!_speechMuted && !_speechPaused && _vm->_mixer->isSoundHandleActive(_soundHandleSpeech))
		return RDSE_SPEAKING;

	return RDSE_QUIET;
}

/**
 * This function loads, decompresses and plays a line of speech. An error
 * occurs if speech is already playing.
 * @param speechId the text line id used to reference the speech
 * @param vol volume, 0 (minimum) to 16 (maximum)
 * @param pan panning, -16 (full left) to 16 (full right)
 */

int32 Sound::playCompSpeech(uint32 speechId, uint8 vol, int8 pan) {
	if (_speechMuted)
		return RD_OK;

	if (getSpeechStatus() == RDERR_SPEECHPLAYING)
		return RDERR_SPEECHPLAYING;

	int cd = _vm->_resman->getCD();
	SoundFileHandle *fh = (cd == 1) ? &_speechFile[0] : &_speechFile[1];

	Audio::AudioStream *input = getAudioStream(fh, "speech", cd, speechId, NULL);

	if (!input)
		return RDERR_INVALIDID;

	// Modify the volume according to the master volume

	byte volume = _speechMuted ? 0 : vol * Audio::Mixer::kMaxChannelVolume / 16;
	int8 p = (pan * 127) / 16;

	if (isReverseStereo())
		p = -p;

	// Start the speech playing
	_vm->_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandleSpeech, input, -1, volume, p);
	return RD_OK;
}

// ----------------------------------------------------------------------------
// SOUND EFFECTS
// ----------------------------------------------------------------------------

/**
 * Mutes/Unmutes the sound effects.
 * @param mute If mute is false, restore the volume to the last set master
 * level. Otherwise the sound effects are muted (volume 0).
 */

void Sound::muteFx(bool mute) {
	_fxMuted = mute;

	// Now update the volume of any fxs playing
	for (int i = 0; i < FXQ_LENGTH; i++) {
		if (_fxQueue[i].resource) {
			_vm->_mixer->setChannelVolume(_fxQueue[i].handle, mute ? 0 : _fxQueue[i].volume);
		}
	}
}

/**
 * Sets the volume and pan of the sample which is currently playing
 * @param id the id of the sample
 * @param vol volume
 * @param pan panning
 */

int32 Sound::setFxIdVolumePan(int32 id, int vol, int pan) {
	if (!_fxQueue[id].resource)
		return RDERR_FXNOTOPEN;

	if (vol > 16)
		vol = 16;

	_fxQueue[id].volume = (vol * Audio::Mixer::kMaxChannelVolume) / 16;

	if (pan != 255) {
		if (isReverseStereo())
			pan = -pan;
		_fxQueue[id].pan = (pan * 127) / 16;
	}

	if (!_fxMuted && _vm->_mixer->isSoundHandleActive(_fxQueue[id].handle)) {
		_vm->_mixer->setChannelVolume(_fxQueue[id].handle, _fxQueue[id].volume);
		if (pan != -1)
			_vm->_mixer->setChannelBalance(_fxQueue[id].handle, _fxQueue[id].pan);
	}

	return RD_OK;
}

void Sound::pauseFx() {
	if (!_fxPaused) {
		for (int i = 0; i < FXQ_LENGTH; i++) {
			if (_fxQueue[i].resource)
				_vm->_mixer->pauseHandle(_fxQueue[i].handle, true);
		}
		_fxPaused = true;
	}
}

void Sound::unpauseFx() {
	if (_fxPaused) {
		for (int i = 0; i < FXQ_LENGTH; i++)
			if (_fxQueue[i].resource)
				_vm->_mixer->pauseHandle(_fxQueue[i].handle, false);
		_fxPaused = false;
	}
}

} // End of namespace Sword2
