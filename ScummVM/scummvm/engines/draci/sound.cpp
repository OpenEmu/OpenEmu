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

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "common/memstream.h"
#include "common/unzip.h"

#include "draci/sound.h"
#include "draci/draci.h"
#include "draci/game.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/flac.h"

namespace Draci {

void LegacySoundArchive::openArchive(const char *path) {
	// Close previously opened archive (if any)
	closeArchive();

	debugCN(1, kDraciArchiverDebugLevel, "Loading samples %s: ", path);

	_f = new Common::File();
	_f->open(path);
	if (_f->isOpen()) {
		debugC(1, kDraciArchiverDebugLevel, "Success");
	} else {
		debugC(1, kDraciArchiverDebugLevel, "Error");
		delete _f;
		_f = NULL;
		return;
	}

	// Save path for reading in files later on
	_path = path;

	// Read archive header
	debugC(1, kDraciArchiverDebugLevel, "Loading header");

	uint totalLength = _f->readUint32LE();

	const uint kMaxSamples = 4095;	// The no-sound file is exactly 16K bytes long, so don't fail on short reads
	uint *sampleStarts = (uint *)malloc(kMaxSamples * sizeof(uint));
	if (!sampleStarts)
		error("[LegacySoundArchive::openArchive] Cannot allocate buffer for no-sound file");

	for (uint i = 0; i < kMaxSamples; ++i) {
		sampleStarts[i] = _f->readUint32LE();
	}

	// Fill the sample table
	for (_sampleCount = 0; _sampleCount < kMaxSamples - 1; ++_sampleCount) {
		int length = sampleStarts[_sampleCount + 1] - sampleStarts[_sampleCount];
		if (length <= 0 && sampleStarts[_sampleCount] >= totalLength)	// heuristics to detect the last sample
			break;
	}
	if (_sampleCount > 0) {
		debugC(1, kDraciArchiverDebugLevel, "Archive info: %d samples, %d total length",
			_sampleCount, totalLength);
		_samples = new SoundSample[_sampleCount];
		for (uint i = 0; i < _sampleCount; ++i) {
			_samples[i]._offset = sampleStarts[i];
			_samples[i]._length = sampleStarts[i+1] - sampleStarts[i];
			_samples[i]._frequency = 0;	// set in getSample()
		}
		if (_samples[_sampleCount-1]._offset + _samples[_sampleCount-1]._length != totalLength &&
		    _samples[_sampleCount-1]._offset + _samples[_sampleCount-1]._length - _samples[0]._offset != totalLength) {
			// WORKAROUND: the stored length is stored with the header for sounds and without the header for dubbing.  Crazy.
			debugC(1, kDraciArchiverDebugLevel, "Broken sound archive: %d != %d",
				_samples[_sampleCount-1]._offset + _samples[_sampleCount-1]._length,
				totalLength);
			closeArchive();

			free(sampleStarts);

			return;
		}
	} else {
		debugC(1, kDraciArchiverDebugLevel, "Archive info: empty");
	}

	free(sampleStarts);

	// Indicate that the archive has been successfully opened
	_opened = true;
}

/**
 * @brief LegacySoundArchive close method
 *
 * Closes the currently opened archive. It can be called explicitly to
 * free up memory.
 */
void LegacySoundArchive::closeArchive() {
	clearCache();
	delete _f;
	_f = NULL;
	delete[] _samples;
	_samples = NULL;
	_sampleCount = 0;
	_path = "";
	_opened = false;
}

/**
 * Clears the cache of the open files inside the archive without closing it.
 * If the files are subsequently accessed, they are read from the disk.
 */
void LegacySoundArchive::clearCache() {
	// Delete all cached data
	for (uint i = 0; i < _sampleCount; ++i) {
		_samples[i].close();
	}
}

/**
 * @brief On-demand sound sample loader
 * @param i Index of file inside an archive
 * @return Pointer to a SoundSample coresponding to the opened file or NULL (on failure)
 *
 * Loads individual samples from an archive to memory on demand.
 */
SoundSample *LegacySoundArchive::getSample(int i, uint freq) {
	// Check whether requested file exists
	if (i < 0 || i >= (int) _sampleCount) {
		return NULL;
	}

	debugCN(2, kDraciArchiverDebugLevel, "Accessing sample %d from archive %s... ",
		i, _path);

	// Check if file has already been opened and return that
	if (_samples[i]._data) {
		debugC(2, kDraciArchiverDebugLevel, "Cached");
	} else {
		// It would be nice to unify the approach with ZipSoundArchive
		// and allocate a MemoryReadStream with buffer stored inside it
		// that playSoundBuffer() would just play.  Unfortunately,
		// streams are not thread-safe and the same sample couldn't
		// thus be played more than once at the same time (this holds
		// even if we create a SeekableSubReadStream from it as this
		// just uses the parent).  The only thread-safe solution is to
		// share a read-only buffer and allocate separate
		// MemoryReadStream's on top of it.
		_samples[i]._data = new byte[_samples[i]._length];
		_samples[i]._format = RAW;

		// Read in the file (without the file header)
		_f->seek(_samples[i]._offset);
		_f->read(_samples[i]._data, _samples[i]._length);

		debugC(2, kDraciArchiverDebugLevel, "Read sample %d from archive %s",
			i, _path);
	}
	_samples[i]._frequency = freq ? freq : _defaultFreq;

	return _samples + i;
}

void ZipSoundArchive::openArchive(const char *path, const char *extension, SoundFormat format, int raw_frequency) {
	closeArchive();
	if ((format == RAW || format == RAW80) && !raw_frequency) {
		error("openArchive() expects frequency for RAW data");
		return;
	}

	debugCN(1, kDraciArchiverDebugLevel, "Trying to open ZIP archive %s: ", path);
	_archive = Common::makeZipArchive(path);
	_path = path;
	_extension = extension;
	_format = format;
	_defaultFreq = raw_frequency;

	if (_archive) {
		Common::ArchiveMemberList files;
		_archive->listMembers(files);
		_sampleCount = files.size();
		debugC(1, kDraciArchiverDebugLevel, "Capacity %d", _sampleCount);
	} else {
		debugC(1, kDraciArchiverDebugLevel, "Failed");
	}
}

void ZipSoundArchive::closeArchive() {
	clearCache();
	delete _archive;
	_archive = NULL;
	_path = _extension = NULL;
	_sampleCount = _defaultFreq = 0;
	_format = RAW;
}

void ZipSoundArchive::clearCache() {
	// Just deallocate the link-list of (very short) headers for each
	// dubbed sentence played in the current location.  If the callers have
	// not called .close() on any of the items, call them now.
	for (Common::List<SoundSample>::iterator it = _cache.begin(); it != _cache.end(); ++it) {
		it->close();
	}
	_cache.clear();
}

SoundSample *ZipSoundArchive::getSample(int i, uint freq) {
	if (i < 0 || i >= (int) _sampleCount) {
		return NULL;
	}
	debugCN(2, kDraciArchiverDebugLevel, "Accessing sample %d.%s from archive %s (format %d@%d, capacity %d): ",
		i, _extension, _path, static_cast<int> (_format), _defaultFreq, _sampleCount);
	if (freq != 0 && (_format != RAW && _format != RAW80)) {
		error("Cannot resample a sound in compressed format");
		return NULL;
	}

	// We cannot really cache anything, because createReadStreamForMember()
	// returns the data as a ReadStream, which is not thread-safe.  We thus
	// read it again each time even if it has possibly been already cached
	// a while ago.  This is not such a problem for dubbing as for regular
	// sound samples.
	SoundSample sample;
	sample._frequency = freq ? freq : _defaultFreq;
	sample._format = _format;
	// Read in the file (without the file header)
	Common::String filename = Common::String::format("%d.%s", i+1, _extension);
	sample._stream = _archive->createReadStreamForMember(filename);
	if (!sample._stream) {
		debugC(2, kDraciArchiverDebugLevel, "Doesn't exist");
		return NULL;
	} else {
		debugC(2, kDraciArchiverDebugLevel, "Read");
		_cache.push_back(sample);
		// Return a pointer that we own and which we will deallocate
		// including its contents.
		return &_cache.back();
	}
}

Sound::Sound(Audio::Mixer *mixer) : _mixer(mixer), _muteSound(false), _muteVoice(false),
	_showSubtitles(true), _talkSpeed(kStandardSpeed) {

	for (int i = 0; i < SOUND_HANDLES; i++)
		_handles[i].type = kFreeHandle;

	setVolume();
}

SndHandle *Sound::getHandle() {
	for (int i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type != kFreeHandle && !_mixer->isSoundHandleActive(_handles[i].handle)) {
			debugC(5, kDraciSoundDebugLevel, "Handle %d has finished playing", i);
			_handles[i].type = kFreeHandle;
		}
	}

	for (int i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type == kFreeHandle) {
			debugC(5, kDraciSoundDebugLevel, "Allocated handle %d", i);
			return &_handles[i];
		}
	}

	error("Sound::getHandle(): Too many sound handles");

	return NULL;	// for compilers that don't support NORETURN
}

uint Sound::playSoundBuffer(Audio::SoundHandle *handle, const SoundSample &buffer, int volume,
				sndHandleType handleType, bool loop) {
	if (!buffer._stream && !buffer._data) {
		warning("Empty stream");
		return 0;
	}
	// Create a new SeekableReadStream which will be automatically disposed
	// after the sample stops playing.  Do not dispose the original
	// data/stream though.
	// Beware that if the sample comes from an archive (i.e., is stored in
	// buffer._stream), then you must NOT play it more than once at the
	// same time, because streams are not thread-safe.  Playing it
	// repeatedly is OK.  Currently this is ensured by that archives are
	// only used for dubbing, which is only played from one place in
	// script.cpp, which blocks until the dubbed sentence has finished
	// playing.
	Common::SeekableReadStream *stream;
	const int skip = buffer._format == RAW80 ? 80 : 0;
	if (buffer._stream) {
		stream = new Common::SeekableSubReadStream(
			buffer._stream, skip, buffer._stream->size() /* end */, DisposeAfterUse::NO);
	} else {
		stream = new Common::MemoryReadStream(
			buffer._data + skip, buffer._length - skip /* length */, DisposeAfterUse::NO);
	}

	Audio::SeekableAudioStream *reader = NULL;
	switch (buffer._format) {
	case RAW:
	case RAW80:
		reader = Audio::makeRawStream(stream, buffer._frequency, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
		break;
#ifdef USE_MAD
	case MP3:
		reader = Audio::makeMP3Stream(stream, DisposeAfterUse::YES);
		break;
#endif
#ifdef USE_VORBIS
	case OGG:
		reader = Audio::makeVorbisStream(stream, DisposeAfterUse::YES);
		break;
#endif
#ifdef USE_FLAC
	case FLAC:
		reader = Audio::makeFLACStream(stream, DisposeAfterUse::YES);
		break;
#endif
	default:
		error("Unsupported compression format %d", static_cast<int> (buffer._format));
		delete stream;
		return 0;
	}

	const uint length = reader->getLength().msecs();
	const Audio::Mixer::SoundType soundType = (handleType == kVoiceHandle) ?
				Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType;
	Audio::AudioStream *audio_stream = Audio::makeLoopingAudioStream(reader, loop ? 0 : 1);
	_mixer->playStream(soundType, handle, audio_stream, -1, volume);
	return length;
}

uint Sound::playSound(const SoundSample *buffer, int volume, bool loop) {
	if (!buffer || _muteSound)
		return 0;
	SndHandle *handle = getHandle();

	handle->type = kEffectHandle;
	return playSoundBuffer(&handle->handle, *buffer, 2 * volume, handle->type, loop);
}

void Sound::pauseSound() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kEffectHandle)
			_mixer->pauseHandle(_handles[i].handle, true);
}

void Sound::resumeSound() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kEffectHandle)
			_mixer->pauseHandle(_handles[i].handle, false);
}

void Sound::stopSound() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kEffectHandle) {
			_mixer->stopHandle(_handles[i].handle);
			debugC(5, kDraciSoundDebugLevel, "Stopping effect handle %d", i);
			_handles[i].type = kFreeHandle;
		}
}

uint Sound::playVoice(const SoundSample *buffer) {
	if (!buffer || _muteVoice)
		return 0;
	SndHandle *handle = getHandle();

	handle->type = kVoiceHandle;
	return playSoundBuffer(&handle->handle, *buffer, Audio::Mixer::kMaxChannelVolume, handle->type, false);
}

void Sound::pauseVoice() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kVoiceHandle)
			_mixer->pauseHandle(_handles[i].handle, true);
}

void Sound::resumeVoice() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kVoiceHandle)
			_mixer->pauseHandle(_handles[i].handle, false);
}

void Sound::stopVoice() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kVoiceHandle) {
			_mixer->stopHandle(_handles[i].handle);
			debugC(5, kDraciSoundDebugLevel, "Stopping voice handle %d", i);
			_handles[i].type = kFreeHandle;
		}
}

void Sound::setVolume() {
	_showSubtitles = ConfMan.getBool("subtitles");
	_talkSpeed = ConfMan.getInt("talkspeed");

	if (_mixer->isReady()) {
		_muteSound = ConfMan.getBool("sfx_mute");
		_muteVoice = ConfMan.getBool("speech_mute");
	} else {
		_muteSound = _muteVoice = true;
	}

	if (ConfMan.getBool("mute")) {
		_muteSound = _muteVoice = true;
	}

	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, _muteSound);
	_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, _muteVoice);

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
}

} // End of namespace Draci
