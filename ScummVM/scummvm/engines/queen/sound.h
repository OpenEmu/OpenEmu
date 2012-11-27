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

#ifndef QUEEN_SOUND_H
#define QUEEN_SOUND_H

#include "common/util.h"
#include "audio/mixer.h"
#include "audio/mods/rjp1.h"
#include "queen/defs.h"

namespace Common {
class File;
}

namespace Queen {

struct SongData {
	int16 tuneList[5];
	int16 volume;
	int16 tempo;
	int16 reverb;
	int16 override;
	int16 ignore;
};

struct TuneData {
	int16 tuneNum[9];
	int16 sfx[2];
	int16 mode;
	int16 delay;
};

class MidiMusic;
class QueenEngine;

class Sound {
public:
	Sound(Audio::Mixer *mixer, QueenEngine *vm);
	virtual ~Sound() {}

	/**
	 * Factory method for subclasses of class Sound.
	 */
	static Sound *makeSoundInstance(Audio::Mixer *mixer, QueenEngine *vm, uint8 compression);

	virtual void playSfx(uint16 sfx) {}
	virtual void playSong(int16 songNum) {}
	virtual void playSpeech(const char *base) {}

	virtual void stopSfx() {}
	virtual void stopSong() {}
	virtual void stopSpeech() {}

	virtual bool isSpeechActive() const	{ return false; }
	virtual bool isSfxActive() const { return false; }

	virtual void updateMusic() {}

	virtual void setVolume(int vol);
	virtual int getVolume()				{ return _musicVolume; }

	void playLastSong()		{ playSong(_lastOverride); }

	bool sfxOn() const			{ return _sfxToggle; }
	void sfxToggle(bool val)	{ _sfxToggle = val; }
	void toggleSfx()			{ _sfxToggle = !_sfxToggle; }

	bool speechOn()	const		{ return _speechToggle; }
	void speechToggle(bool val)	{ _speechToggle = val; }
	void toggleSpeech()			{ _speechToggle = !_speechToggle; }

	bool musicOn() const		{ return _musicToggle; }
	void musicToggle(bool val)	{ _musicToggle = val; }
	void toggleMusic()			{ _musicToggle = !_musicToggle; }

	bool speechSfxExists() const	{ return _speechSfxExists; }

	int16 lastOverride() const	{ return _lastOverride; }

	void saveState(byte *&ptr);
	void loadState(uint32 ver, byte *&ptr);

	static const SongData _songDemo[];
	static const SongData _song[];
	static const TuneData _tuneDemo[];
	static const TuneData _tune[];
	static const char *const _sfxName[];
	static const int16 _jungleList[];

protected:

	Audio::Mixer *_mixer;
	QueenEngine *_vm;

	bool _sfxToggle;
	bool _speechToggle;
	bool _musicToggle;
	bool _speechSfxExists;

	int16 _lastOverride;
	int _musicVolume;
};

class PCSound : public Sound {
public:
	PCSound(Audio::Mixer *mixer, QueenEngine *vm);
	~PCSound();

	void playSfx(uint16 sfx);
	void playSpeech(const char *base);
	void playSong(int16 songNum);

	void stopSfx()			{ _mixer->stopHandle(_sfxHandle); }
	void stopSong();
	void stopSpeech()		{ _mixer->stopHandle(_speechHandle); }

	bool isSpeechActive() const	{ return _mixer->isSoundHandleActive(_speechHandle); }
	bool isSfxActive() const	{ return _mixer->isSoundHandleActive(_sfxHandle); }

	void setVolume(int vol);

protected:
	void playSound(const char *base, bool isSpeech);

	virtual void playSoundData(Common::File *f, uint32 size, Audio::SoundHandle *soundHandle) = 0;

	Audio::SoundHandle _sfxHandle;
	Audio::SoundHandle _speechHandle;
	MidiMusic *_music;
};

class AmigaSound : public Sound {
public:
	AmigaSound(Audio::Mixer *mixer, QueenEngine *vm);

	void playSfx(uint16 sfx);
	void playSong(int16 song);

	void stopSfx();
	void stopSong();

	bool isSfxActive() const { return _mixer->isSoundHandleActive(_sfxHandle); }

	void updateMusic();

protected:

	void playSound(const char *base);
	Audio::AudioStream *loadModule(const char *base, int song);
	void playModule(const char *base, int song);
	void playPattern(const char *base, int pattern);
	bool playSpecialSfx(int16 sfx);

	int16 _fanfareRestore;
	int _fanfareCount, _fluteCount;
	Audio::SoundHandle _modHandle;
	Audio::SoundHandle _patHandle;
	Audio::SoundHandle _sfxHandle;
};

} // End of namespace Queen

#endif
