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


#include "common/file.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "sword1/sword1.h"
#include "sword1/music.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/xa.h"

#define SMP_BUFSIZE 8192

namespace Sword1 {

// This means fading takes 3 seconds.
#define FADE_LENGTH 3

// These functions are only called from Music, so I'm just going to
// assume that if locking is needed it has already been taken care of.

bool MusicHandle::play(const Common::String &filename, bool loop) {
	stop();

	// FIXME: How about using AudioStream::openStreamFile instead of the code below?
	// I.e.:
	//_audioSource = Audio::AudioStream::openStreamFile(fileBase, 0, 0, loop ? 0 : 1);

	Audio::RewindableAudioStream *stream = 0;

#ifdef USE_FLAC
	if (!stream) {
		if (_file.open(filename + ".flac")) {
			stream = Audio::makeFLACStream(&_file, DisposeAfterUse::NO);
			if (!stream)
				_file.close();
		}
	}

	if (!stream) {
		if (_file.open(filename + ".fla")) {
			stream = Audio::makeFLACStream(&_file, DisposeAfterUse::NO);
			if (!stream)
				_file.close();
		}
	}
#endif
#ifdef USE_VORBIS
	if (!stream) {
		if (_file.open(filename + ".ogg")) {
			stream = Audio::makeVorbisStream(&_file, DisposeAfterUse::NO);
			if (!stream)
				_file.close();
		}
	}
#endif
#ifdef USE_MAD
	if (!stream) {
		if (_file.open(filename + ".mp3")) {
			stream = Audio::makeMP3Stream(&_file, DisposeAfterUse::NO);
			if (!stream)
				_file.close();
		}
	}
#endif
	if (!stream) {
		if (_file.open(filename + ".wav"))
			stream = Audio::makeWAVStream(&_file, DisposeAfterUse::NO);
	}

	if (!stream) {
		if (_file.open(filename + ".aif"))
			stream = Audio::makeAIFFStream(&_file, DisposeAfterUse::NO);
	}

	if (!stream)
		return false;

	_audioSource = Audio::makeLoopingAudioStream(stream, loop ? 0 : 1);

	fadeUp();
	return true;
}

bool MusicHandle::playPSX(uint16 id) {
	stop();

	if (!_file.isOpen())
		if (!_file.open("tunes.dat"))
			return false;

	Common::File tableFile;
	if (!tableFile.open("tunes.tab"))
		return false;

	tableFile.seek((id - 1) * 8, SEEK_SET);
	uint32 offset = tableFile.readUint32LE() * 0x800;
	uint32 size = tableFile.readUint32LE();

	tableFile.close();

	// Because of broken tunes.dat/tab in psx demo, also check that tune offset is
	// not over file size
	if ((size != 0) && (size != 0xffffffff) && ((int32)(offset + size) <= _file.size())) {
		_file.seek(offset, SEEK_SET);
		_audioSource = Audio::makeXAStream(_file.readStream(size), 11025);
		fadeUp();
	} else {
		_audioSource = NULL;
		return false;
	}

	return true;
}

void MusicHandle::fadeDown() {
	if (streaming()) {
		if (_fading < 0)
			_fading = -_fading;
		else if (_fading == 0)
			_fading = FADE_LENGTH * getRate();
		_fadeSamples = FADE_LENGTH * getRate();
	}
}

void MusicHandle::fadeUp() {
	if (streaming()) {
		if (_fading > 0)
			_fading = -_fading;
		else if (_fading == 0)
			_fading = -1;
		_fadeSamples = FADE_LENGTH * getRate();
	}
}

bool MusicHandle::endOfData() const {
	return !streaming();
}

// if we don't have an audiosource, return some dummy values.
bool MusicHandle::streaming() const {
	return (_audioSource) ? (!_audioSource->endOfStream()) : false;
}

bool MusicHandle::isStereo() const {
	return (_audioSource) ? _audioSource->isStereo() : false;
}

int MusicHandle::getRate() const {
	return (_audioSource) ? _audioSource->getRate() : 11025;
}

int MusicHandle::readBuffer(int16 *buffer, const int numSamples) {
	int totalSamples = 0;
	int16 *bufStart = buffer;
	if (!_audioSource)
		return 0;
	int expectedSamples = numSamples;
	while ((expectedSamples > 0) && _audioSource) { // _audioSource becomes NULL if we reach EOF and aren't looping
		int samplesReturned = _audioSource->readBuffer(buffer, expectedSamples);
		buffer += samplesReturned;
		totalSamples += samplesReturned;
		expectedSamples -= samplesReturned;
		if ((expectedSamples > 0) && _audioSource->endOfData()) {
			debug(2, "Music reached EOF");
			stop();
		}
	}
	// buffer was filled, now do the fading (if necessary)
	int samplePos = 0;
	while ((_fading > 0) && (samplePos < totalSamples)) { // fade down
		--_fading;
		bufStart[samplePos] = (bufStart[samplePos] * _fading) / _fadeSamples;
		samplePos++;
		if (_fading == 0) {
			stop();
			// clear the rest of the buffer
			memset(bufStart + samplePos, 0, (totalSamples - samplePos) * 2);
			return samplePos;
		}
	}
	while ((_fading < 0) && (samplePos < totalSamples)) { // fade up
		bufStart[samplePos] = -(bufStart[samplePos] * --_fading) / _fadeSamples;
		if (_fading <= -_fadeSamples)
			_fading = 0;
	}
	return totalSamples;
}

void MusicHandle::stop() {
	delete _audioSource;
	_audioSource = NULL;
	_file.close();
	_fading = 0;
}

Music::Music(Audio::Mixer *pMixer) {
	_mixer = pMixer;
	_sampleRate = pMixer->getOutputRate();
	_converter[0] = NULL;
	_converter[1] = NULL;
	_volumeL = _volumeR = 192;
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Music::~Music() {
	_mixer->stopHandle(_soundHandle);
	delete _converter[0];
	delete _converter[1];
}

void Music::mixer(int16 *buf, uint32 len) {
	Common::StackLock lock(_mutex);
	memset(buf, 0, 2 * len * sizeof(int16));
	for (int i = 0; i < ARRAYSIZE(_handles); i++)
		if (_handles[i].streaming() && _converter[i])
			_converter[i]->flow(_handles[i], buf, len, _volumeL, _volumeR);
}

void Music::setVolume(uint8 volL, uint8 volR) {
	_volumeL = (Audio::st_volume_t)volL;
	_volumeR = (Audio::st_volume_t)volR;
}

void Music::giveVolume(uint8 *volL, uint8 *volR) {
	*volL = (uint8)_volumeL;
	*volR = (uint8)_volumeR;
}

void Music::startMusic(int32 tuneId, int32 loopFlag) {
	if (strlen(_tuneList[tuneId]) > 0) {
		int newStream = 0;
		_mutex.lock();
		if (_handles[0].streaming() && _handles[1].streaming()) {
			int streamToStop;
			// Both streams playing - one must be forced to stop.
			if (!_handles[0].fading() && !_handles[1].fading()) {
				// None of them are fading. Shouldn't happen,
				// so it doesn't matter which one we pick.
				streamToStop = 0;
			} else if (_handles[0].fading() && !_handles[1].fading()) {
				// Stream 0 is fading, so pick that one.
				streamToStop = 0;
			} else if (!_handles[0].fading() && _handles[1].fading()) {
				// Stream 1 is fading, so pick that one.
				streamToStop = 1;
			} else {
				// Both streams are fading. Pick the one that
				// is closest to silent.
				if (ABS(_handles[0].fading()) < ABS(_handles[1].fading()))
					streamToStop = 0;
				else
					streamToStop = 1;
			}
			_handles[streamToStop].stop();
		}
		if (_handles[0].streaming()) {
			_handles[0].fadeDown();
			newStream = 1;
		} else if (_handles[1].streaming()) {
			_handles[1].fadeDown();
			newStream = 0;
		}
		delete _converter[newStream];
		_converter[newStream] = NULL;
		_mutex.unlock();

		/* The handle will load the music file now. It can take a while, so unlock
		   the mutex before, to have the soundthread playing normally.
		   As the corresponding _converter is NULL, the handle will be ignored by the playing thread */
		if (SwordEngine::isPsx()) {
			if (_handles[newStream].playPSX(tuneId)) {
				_mutex.lock();
				_converter[newStream] = Audio::makeRateConverter(_handles[newStream].getRate(), _mixer->getOutputRate(), _handles[newStream].isStereo(), false);
				_mutex.unlock();
			}
		} else if (_handles[newStream].play(_tuneList[tuneId], loopFlag != 0)) {
			_mutex.lock();
			_converter[newStream] = Audio::makeRateConverter(_handles[newStream].getRate(), _mixer->getOutputRate(), _handles[newStream].isStereo(), false);
			_mutex.unlock();
		} else {
			if (tuneId != 81) // file 81 was apparently removed from BS.
				warning("Can't find music file %s", _tuneList[tuneId]);
		}
	} else {
		_mutex.lock();
		if (_handles[0].streaming())
			_handles[0].fadeDown();
		if (_handles[1].streaming())
			_handles[1].fadeDown();
		_mutex.unlock();
	}
}

void Music::fadeDown() {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < ARRAYSIZE(_handles); i++)
		if (_handles[i].streaming())
			_handles[i].fadeDown();
}

} // End of namespace Sword1
