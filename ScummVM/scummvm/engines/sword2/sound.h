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

/*****************************************************************************
 *		SOUND.H		Sound engine
 *
 *		SOUND.CPP Contains the sound engine, fx & music functions
 *		Some very 'sound' code in here ;)
 *
 *		(16Dec96 JEL)
 *
 ****************************************************************************/

#ifndef SWORD2_SOUND_H
#define SWORD2_SOUND_H

#include "common/file.h"
#include "common/mutex.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

// Max number of sound fx
#define MAXMUS 2

// Max number of fx in queue at once
#define FXQ_LENGTH 32

#define BUFFER_SIZE 4096

namespace Sword2 {

enum {
	kCLUMode = 1,
	kMP3Mode,
	kVorbisMode,
	kFLACMode
};

enum {
	kLeadInSound,
	kLeadOutSound
};

enum {
	// These three types correspond to types set by the scripts
	FX_SPOT		= 0,
	FX_LOOP		= 1,
	FX_RANDOM	= 2,

	// These are used for FX queue bookkeeping
	FX_SPOT2	= 3,
	FX_LOOPING	= 4
};

// Sound defines

enum {
	RDSE_SAMPLEFINISHED		= 0,
	RDSE_SAMPLEPLAYING		= 1,
	RDSE_FXTOCLEAR			= 0,		// Unused
	RDSE_FXCACHED			= 1,		// Unused
	RDSE_FXSPOT			= 0,
	RDSE_FXLOOP			= 1,
	RDSE_FXLEADIN			= 2,
	RDSE_FXLEADOUT			= 3,
	RDSE_QUIET			= 1,
	RDSE_SPEAKING			= 0
};

class CLUInputStream : public Audio::AudioStream {
private:
	Common::File *_file;
	bool _firstTime;
	uint32 _file_pos;
	uint32 _end_pos;
	int16 _outbuf[BUFFER_SIZE];
	byte _inbuf[BUFFER_SIZE];
	const int16 *_bufferEnd;
	const int16 *_pos;

	uint16 _prev;

	void refill();

	inline bool eosIntern() const {
		return !_file->isOpen() || _pos >= _bufferEnd;
	}

public:
	CLUInputStream(Common::File *file, int size);
	~CLUInputStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const	{ return eosIntern(); }
	bool isStereo() const	{ return false; }
	int getRate() const	{ return 22050; }
};

struct SoundFileHandle {
	Common::File file;
	uint32 *idxTab;
	uint32 idxLen;
	int32 fileSize;
	uint32 fileType;
	volatile bool inUse;
};

class MusicInputStream : public Audio::AudioStream {
private:
	int _cd;
	SoundFileHandle *_fh;
	uint32 _musicId;
	Audio::AudioStream *_decoder;
	int16 _buffer[BUFFER_SIZE];
	const int16 *_bufferEnd;
	const int16 *_pos;
	bool _remove;
	uint32 _numSamples;
	uint32 _samplesLeft;
	bool _looping;
	int32 _fading;
	int32 _fadeSamples;
	bool _paused;

	void refill();

	inline bool eosIntern() const {
		if (_looping)
			return false;
		return _remove || _pos >= _bufferEnd;
	}

public:
	MusicInputStream(int cd, SoundFileHandle *fh, uint32 musicId, bool looping);
	~MusicInputStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const	{ return eosIntern(); }
	bool isStereo() const	{ return _decoder->isStereo(); }
	int getRate() const	{ return _decoder->getRate(); }

	int getCD()		{ return _cd; }

	void fadeUp();
	void fadeDown();

	bool isReady()		{ return _decoder != NULL; }
	int32 isFading()	{ return _fading; }

	bool readyToRemove();
	int32 getTimeRemaining();
};

class Sound : public Audio::AudioStream {
private:
	Sword2Engine *_vm;

	Common::Mutex _mutex;

	Audio::SoundHandle _mixerSoundHandle;
	Audio::SoundHandle _leadInHandle;
	Audio::SoundHandle _leadOutHandle;

	struct FxQueueEntry {
		Audio::SoundHandle handle;	// sound handle
		uint32 resource;		// resource id of sample
		byte *data;			// pointer to WAV data
		uint32 len;			// WAV data length
		uint16 delay;			// cycles to wait before playing (or 'random chance' if FX_RANDOM)
		uint8 volume;			// sound volume
		int8 pan;			// sound panning
		uint8 type;			// FX_SPOT, FX_RANDOM, FX_LOOP
	};

	FxQueueEntry _fxQueue[FXQ_LENGTH];

	void triggerFx(uint8 i);

	bool _reverseStereo;

	bool _speechMuted;
	bool _fxMuted;
	bool _musicMuted;

	bool _speechPaused;
	bool _fxPaused;
	bool _musicPaused;

	int32 _loopingMusicId;

	Audio::SoundHandle _soundHandleSpeech;

	MusicInputStream *_music[MAXMUS];
	SoundFileHandle _musicFile[MAXMUS];
	SoundFileHandle _speechFile[MAXMUS];

	int16 *_mixBuffer;
	int _mixBufferLen;

public:
	Sound(Sword2Engine *vm);
	~Sound();

	// AudioStream API

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const;
	int getRate() const { return Sword2Engine::isPsx() ? 11025 : 22050; }

	// End of AudioStream API

	void clearFxQueue(bool killMovieSounds);
	void processFxQueue();

	void setReverseStereo(bool reverse);
	bool isReverseStereo() const { return _reverseStereo; }

	void muteSpeech(bool mute);
	bool isSpeechMute() const { return _speechMuted; }

	void muteFx(bool mute);
	bool isFxMute() const { return _fxMuted; }

	void muteMusic(bool mute) { _musicMuted = mute; }
	bool isMusicMute() const { return _musicMuted; }

	void setLoopingMusicId(int32 id) { _loopingMusicId = id; }
	int32 getLoopingMusicId() const { return _loopingMusicId; }

	void pauseSpeech();
	void unpauseSpeech();

	void pauseFx();
	void unpauseFx();

	void pauseMusic();
	void unpauseMusic();

	void playMovieSound(int32 res, int type);
	void stopMovieSounds();

	void queueFx(int32 res, int32 type, int32 delay, int32 volume, int32 pan);
	int32 playFx(FxQueueEntry *fx);
	int32 playFx(Audio::SoundHandle *handle, byte *data, uint32 len, uint8 vol, int8 pan, bool loop, Audio::Mixer::SoundType soundType);
	int32 stopFx(int32 i);
	int32 setFxIdVolumePan(int32 id, int vol, int pan = 255);

	int32 getSpeechStatus();
	int32 amISpeaking();
	int32 playCompSpeech(uint32 speechId, uint8 vol, int8 pan);
	int32 stopSpeech();

	int32 streamCompMusic(uint32 musicId, bool loop);
	void stopMusic(bool immediately);
	int32 musicTimeRemaining();

	void printFxQueue();
};

} // End of namespace Sword2

#endif
