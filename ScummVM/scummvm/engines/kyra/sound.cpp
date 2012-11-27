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


#include "kyra/sound.h"
#include "kyra/resource.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"

#include "audio/decoders/flac.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "audio/decoders/vorbis.h"

namespace Kyra {

Sound::Sound(KyraEngine_v1 *vm, Audio::Mixer *mixer)
	: _vm(vm), _mixer(mixer), _soundChannels(), _musicEnabled(1),
	_sfxEnabled(true) {
}

Sound::~Sound() {
}

Sound::kType Sound::getSfxType() const {
	return getMusicType();
}

bool Sound::isPlaying() const {
	return false;
}

bool Sound::isVoicePresent(const char *file) const {
	Common::String filename;

	for (int i = 0; _supportedCodecs[i].fileext; ++i) {
		filename = file;
		filename += _supportedCodecs[i].fileext;

		if (_vm->resource()->exists(filename.c_str()))
			return true;
	}

	return false;
}

int32 Sound::voicePlay(const char *file, Audio::SoundHandle *handle, uint8 volume, uint8 priority, bool isSfx) {
	Audio::SeekableAudioStream *audioStream = getVoiceStream(file);

	if (!audioStream) {
		return 0;
	}

	int playTime = audioStream->getLength().msecs();
	playVoiceStream(audioStream, handle, volume, priority, isSfx);
	return playTime;
}

Audio::SeekableAudioStream *Sound::getVoiceStream(const char *file) const {
	Common::String filename;

	Audio::SeekableAudioStream *audioStream = 0;
	for (int i = 0; _supportedCodecs[i].fileext; ++i) {
		filename = file;
		filename += _supportedCodecs[i].fileext;

		Common::SeekableReadStream *stream = _vm->resource()->createReadStream(filename);
		if (!stream)
			continue;

		audioStream = _supportedCodecs[i].streamFunc(stream, DisposeAfterUse::YES);
		break;
	}

	if (!audioStream) {
		warning("Couldn't load sound file '%s'", file);
		return 0;
	} else {
		return audioStream;
	}
}

bool Sound::playVoiceStream(Audio::AudioStream *stream, Audio::SoundHandle *handle, uint8 volume, uint8 priority, bool isSfx) {
	int h = 0;
	while (h < kNumChannelHandles && _mixer->isSoundHandleActive(_soundChannels[h].handle))
		++h;

	if (h >= kNumChannelHandles) {
		h = 0;
		while (h < kNumChannelHandles && _soundChannels[h].priority > priority)
			++h;
		if (h < kNumChannelHandles)
			voiceStop(&_soundChannels[h].handle);
	}

	if (h >= kNumChannelHandles) {
		// When we run out of handles we need to destroy the stream object,
		// this is to avoid memory leaks in some scenes where too many sfx
		// are started.
		// See bug #3427240 "LOL-CD: Memory leak in caves level 3".
		delete stream;
		return false;
	}

	_mixer->playStream(isSfx ? Audio::Mixer::kSFXSoundType : Audio::Mixer::kSpeechSoundType, &_soundChannels[h].handle, stream, -1, volume);
	_soundChannels[h].priority = priority;
	if (handle)
		*handle = _soundChannels[h].handle;

	return true;
}

void Sound::voiceStop(const Audio::SoundHandle *handle) {
	if (!handle) {
		for (int h = 0; h < kNumChannelHandles; ++h) {
			if (_mixer->isSoundHandleActive(_soundChannels[h].handle))
				_mixer->stopHandle(_soundChannels[h].handle);
		}
	} else {
		_mixer->stopHandle(*handle);
	}
}

bool Sound::voiceIsPlaying(const Audio::SoundHandle *handle) const {
	if (!handle) {
		for (int h = 0; h < kNumChannelHandles; ++h) {
			if (_mixer->isSoundHandleActive(_soundChannels[h].handle))
				return true;
		}
	} else {
		return _mixer->isSoundHandleActive(*handle);
	}

	return false;
}

bool Sound::allVoiceChannelsPlaying() const {
	for (int i = 0; i < kNumChannelHandles; ++i)
		if (!_mixer->isSoundHandleActive(_soundChannels[i].handle))
			return false;
	return true;
}

#pragma mark -

MixedSoundDriver::MixedSoundDriver(KyraEngine_v1 *vm, Audio::Mixer *mixer, Sound *music, Sound *sfx)
    : Sound(vm, mixer), _music(music), _sfx(sfx) {
}

MixedSoundDriver::~MixedSoundDriver() {
	delete _music;
	delete _sfx;
}

Sound::kType MixedSoundDriver::getMusicType() const {
	return _music->getMusicType();
}

Sound::kType MixedSoundDriver::getSfxType() const {
	return _sfx->getSfxType();
}

bool MixedSoundDriver::init() {
	return (_music->init() && _sfx->init());
}

void MixedSoundDriver::process() {
	_music->process();
	_sfx->process();
}

void MixedSoundDriver::updateVolumeSettings() {
	_music->updateVolumeSettings();
	_sfx->updateVolumeSettings();
}

void MixedSoundDriver::initAudioResourceInfo(int set, void *info) {
	_music->initAudioResourceInfo(set, info);
	_sfx->initAudioResourceInfo(set, info);
}

void MixedSoundDriver::selectAudioResourceSet(int set) {
	_music->selectAudioResourceSet(set);
	_sfx->selectAudioResourceSet(set);
}

bool MixedSoundDriver::hasSoundFile(uint file) const {
	return _music->hasSoundFile(file) && _sfx->hasSoundFile(file);
}

void MixedSoundDriver::loadSoundFile(uint file) {
	_music->loadSoundFile(file);
	_sfx->loadSoundFile(file);
}

void MixedSoundDriver::loadSoundFile(Common::String file) {
	_music->loadSoundFile(file);
	_sfx->loadSoundFile(file);
}

void MixedSoundDriver::loadSfxFile(Common::String file) {
	_sfx->loadSoundFile(file);
}

void MixedSoundDriver::playTrack(uint8 track) {
	_music->playTrack(track);
}

void MixedSoundDriver::haltTrack() {
	_music->haltTrack();
}

bool MixedSoundDriver::isPlaying() const {
	return _music->isPlaying() | _sfx->isPlaying();
}

void MixedSoundDriver::playSoundEffect(uint8 track, uint8 volume) {
	_sfx->playSoundEffect(track, volume);
}

void MixedSoundDriver::stopAllSoundEffects() {
	_sfx->stopAllSoundEffects();
}

void MixedSoundDriver::beginFadeOut() {
	_music->beginFadeOut();
}

void MixedSoundDriver::pause(bool paused) {
	_music->pause(paused);
	_sfx->pause(paused);
}

#pragma mark -

void KyraEngine_v1::snd_playTheme(int file, int track) {
	if (_curMusicTheme == file)
		return;

	_curSfxFile = _curMusicTheme = file;
	_sound->loadSoundFile(_curMusicTheme);

	// Kyrandia 2 uses a special file for
	// MIDI sound effects, so we load
	// this here
	if (_flags.gameID == GI_KYRA2)
		_sound->loadSfxFile("K2SFX");

	if (track != -1)
		_sound->playTrack(track);
}

void KyraEngine_v1::snd_playSoundEffect(int track, int volume) {
	_sound->playSoundEffect(track, volume);
}

void KyraEngine_v1::snd_playWanderScoreViaMap(int command, int restart) {
	if (restart)
		_lastMusicCommand = -1;

	// no track mapping given
	// so don't do anything here
	if (!_trackMap || !_trackMapSize)
		return;

	//if (!_disableSound) {
	//	XXX
	//}

	if (_flags.platform == Common::kPlatformPC || _flags.platform == Common::kPlatformMacintosh) {
		assert(command*2+1 < _trackMapSize);
		if (_curMusicTheme != _trackMap[command*2]) {
			if (_trackMap[command*2] != -1 && _trackMap[command*2] != -2)
				snd_playTheme(_trackMap[command*2], -1);
		}

		if (command != 1) {
			if (_lastMusicCommand != command) {
				_sound->haltTrack();
				_sound->playTrack(_trackMap[command*2+1]);
			}
		} else {
			_sound->beginFadeOut();
		}
	} else if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98) {
		if (command == -1) {
			_sound->haltTrack();
		} else {
			assert(command*2+1 < _trackMapSize);
			if (_trackMap[command*2] != -2 && command != _lastMusicCommand) {
				_sound->haltTrack();
				_sound->playTrack(command);
			}
		}
	} else if (_flags.platform == Common::kPlatformAmiga) {
		if (_curMusicTheme != 1)
			snd_playTheme(1, -1);

		assert(command < _trackMapSize);
		if (_trackMap[_lastMusicCommand] != _trackMap[command])
			_sound->playTrack(_trackMap[command]);
	}

	_lastMusicCommand = command;
}

void KyraEngine_v1::snd_stopVoice() {
	_sound->voiceStop(&_speechHandle);
}

bool KyraEngine_v1::snd_voiceIsPlaying() {
	return _sound->voiceIsPlaying(&_speechHandle);
}

// static res

namespace {

// A simple wrapper to create VOC streams the way like creating MP3, OGG/Vorbis and FLAC streams.
// Possible TODO: Think of making this complete and moving it to sound/voc.cpp ?
Audio::SeekableAudioStream *makeVOCStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	Audio::SeekableAudioStream *as = Audio::makeVOCStream(stream, Audio::FLAG_UNSIGNED, disposeAfterUse);
	return as;
}

} // end of anonymous namespace

const Sound::SpeechCodecs Sound::_supportedCodecs[] = {
	{ ".VOC", makeVOCStream },
	{ "", makeVOCStream },

#ifdef USE_MAD
	{ ".VO3", Audio::makeMP3Stream },
	{ ".MP3", Audio::makeMP3Stream },
#endif // USE_MAD

#ifdef USE_VORBIS
	{ ".VOG", Audio::makeVorbisStream },
	{ ".OGG", Audio::makeVorbisStream },
#endif // USE_VORBIS

#ifdef USE_FLAC
	{ ".VOF", Audio::makeFLACStream },
	{ ".FLA", Audio::makeFLACStream },
#endif // USE_FLAC

	{ 0, 0 }
};

} // End of namespace Kyra
