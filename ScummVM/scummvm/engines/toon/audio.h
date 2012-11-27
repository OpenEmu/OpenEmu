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

#ifndef TOON_AUDIO_H
#define TOON_AUDIO_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "toon/toon.h"

namespace Toon {

// used for music/voice/everything
class AudioManager;
class AudioStreamInstance : public Audio::AudioStream {

public:
	AudioStreamInstance(AudioManager *man, Audio::Mixer *mixer, Common::SeekableReadStream *stream, bool looping = false, bool deleteFileStreamAtEnd = false);
	~AudioStreamInstance();
	void play(bool fade = false, Audio::Mixer::SoundType soundType = Audio::Mixer::kMusicSoundType);
	void stop(bool fade = false);

	bool isPlaying() {
		return !_stopped;
	}
	bool isLooping() {
		return _looping;
	}
	bool isFading() {
		return _fadingIn || _fadingOut;
	}

	int32 getPlayedSampleCount() {
		return _playedSamples;
	}

	void setVolume(int32 volume);
protected:
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const {
		return false;
	}
	int getRate() const {
		return 22100;
	}
	bool endOfData() const {
		return _stopped;
	}
	void handleFade(int32 numSamples);
	void stopNow();

	bool readPacket();
	void decodeADPCM(uint8 *comp, int16 *dest, int32 packetSize);

	Common::SeekableReadStream *_file;
	bool _fadingIn;
	bool _fadingOut;
	int32 _fadeTime;
	uint8 *_compBuffer;
	int16 *_buffer;
	int32 _bufferSize;
	int32 _bufferMaxSize;
	int32 _bufferOffset;
	int32 _compBufferSize;
	Audio::SoundHandle _handle;
	Audio::Mixer::SoundType _soundType;
	Audio::Mixer *_mixer;
	int32 _lastSample;
	int32 _lastStepIndex;
	bool _stopped;
	AudioManager *_man;
	int32 _totalSize;
	int32 _currentReadSize;
	bool _looping;
	int32 _volume;
	int32 _musicAttenuation;
	bool _deleteFileStream;
	int32 _playedSamples;
};

class AudioStreamPackage {

public:
	AudioStreamPackage(ToonEngine *vm);
	~AudioStreamPackage();

	bool loadAudioPackage(const Common::String &indexFile, const Common::String &streamFile);
	void getInfo(int32 id, int32 *offset, int32 *size);
	Common::SeekableReadStream *getStream(int32 id, bool ownMemory = false);
protected:
	Common::SeekableReadStream *_file;
	uint32 *_indexBuffer;
	ToonEngine *_vm;
};

struct AudioAmbientSFX {
	int32 _id;
	int32 _volume;
	int32 _lastTimer;
	int32 _delay;
	int32 _mode;
	int32 _channel;
	bool _enabled;
};

class AudioManager {
public:
	void removeInstance(AudioStreamInstance *inst); // called by destructor

	AudioManager(ToonEngine *vm, Audio::Mixer *mixer);
	~AudioManager();

	bool voiceStillPlaying();

	void playMusic(const Common::String &dir, const Common::String &music);
	void playVoice(int32 id, bool genericVoice);
	int32 playSFX(int32 id, int volume, bool genericSFX);
	void stopCurrentVoice();
	void stopAllSfxs();
	void setMusicVolume(int32 volume);
	void stopMusic();
	void muteVoice(bool mute);
	void muteMusic(bool mute);
	void muteSfx(bool mute);
	bool isVoiceMuted() { return _voiceMuted; }
	bool isMusicMuted() { return _musicMuted; }
	bool isSfxMuted() { return _sfxMuted; }

	void startAmbientSFX(int32 id, int32 delay, int32 mode, int32 volume);
	void killAmbientSFX(int32 id);
	void killAllAmbientSFX();
	void updateAmbientSFX();
	void setAmbientSFXVolume(int32 id, int volume);

	void closeAudioPack(int32 id);
	bool loadAudioPack(int32 id, const Common::String &indexFile, const Common::String &packFile);

	AudioStreamInstance *_channels[16];  // 0-1 : music
	// 2 : voice
	// 3-16 : SFX

	AudioStreamPackage *_audioPacks[4];  // 0 : generic streams
	// 1 : local streams
	// 2 : generic SFX
	// 3 : local SFX
	uint32 _currentMusicChannel;
	Common::String _currentMusicName;
	ToonEngine *_vm;
	Audio::Mixer *_mixer;

protected:
	bool _voiceMuted;
	bool _musicMuted;
	bool _sfxMuted;

	AudioAmbientSFX _ambientSFXs[4];
};

} // End of namespace Toon

#endif
