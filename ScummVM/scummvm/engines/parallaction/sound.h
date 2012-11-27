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

#ifndef PARALLACTION_MUSIC_H
#define PARALLACTION_MUSIC_H

#include "common/util.h"
#include "common/mutex.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/iff_sound.h"

#define PATH_LEN 200

class MidiParser;
class MidiDriver;

namespace Parallaction {

class Parallaction_ns;
class MidiPlayer;
class Parallaction_br;
class MidiPlayer_MSC;

MidiDriver *createAdLibDriver();

class SoundManImpl {
public:
	virtual void execute(int command, const char *parm = 0) = 0;
	virtual ~SoundManImpl() { }
};

class SoundMan {
	SoundManImpl *_impl;
public:
	SoundMan(SoundManImpl *impl) : _impl(impl) { }
	virtual ~SoundMan() { delete _impl; }
	void execute(int command, int32 parm) {
		char n[12];
		sprintf(n, "%i", parm);
		execute(command, n);
	}
	void execute(int command, const char *parm = 0) {
		if (_impl) {
			_impl->execute(command, parm);
		}
	}
};

enum {
	// soundMan commands
	SC_PLAYMUSIC,
	SC_STOPMUSIC,
	SC_SETMUSICTYPE,
	SC_SETMUSICFILE,
	SC_PLAYSFX,
	SC_STOPSFX,
	SC_SETSFXCHANNEL,
	SC_SETSFXLOOPING,
	SC_SETSFXVOLUME,
	SC_SETSFXRATE,
	SC_PAUSE
};

struct Channel {
	Audio::AudioStream *stream;
	Audio::SoundHandle	handle;
	uint32				volume;
};



class SoundMan_ns : public SoundManImpl {
public:
	enum {
		MUSIC_ANY,
		MUSIC_CHARACTER,
		MUSIC_LOCATION
	};

protected:
	Parallaction_ns	*_vm;
	Audio::Mixer	*_mixer;
	char			_musicFile[PATH_LEN];

	bool 	_sfxLooping;
	int		_sfxVolume;
	int		_sfxRate;
	uint	_sfxChannel;

	int		_musicType;

public:
	SoundMan_ns(Parallaction_ns *vm);
	virtual ~SoundMan_ns() {}

	virtual void playSfx(const char *filename, uint channel, bool looping, int volume = -1) { }
	virtual void stopSfx(uint channel) { }

	void setMusicFile(const char *filename);
	virtual void playMusic() = 0;
	virtual void stopMusic() = 0;
	virtual void playCharacterMusic(const char *character) = 0;
	virtual void playLocationMusic(const char *location) = 0;
	virtual void pause(bool p) { }
	virtual void execute(int command, const char *parm);

	void setMusicVolume(int value);
};

class DosSoundMan_ns : public SoundMan_ns {

	MidiPlayer	*_midiPlayer;
	bool		_playing;

	bool isLocationSilent(const char *locationName);
	bool locationHasOwnSoftMusic(const char *locationName);


public:
	DosSoundMan_ns(Parallaction_ns *vm);
	~DosSoundMan_ns();
	void playMusic();
	void stopMusic();

	void playCharacterMusic(const char *character);
	void playLocationMusic(const char *location);

	void pause(bool p);
};

#define NUM_SFX_CHANNELS 4

class AmigaSoundMan_ns : public SoundMan_ns {

	Audio::AudioStream *_musicStream;
	Audio::SoundHandle	_musicHandle;

	uint32 	beepSoundBufferSize;
	int8	*beepSoundBuffer;

	Channel _channels[NUM_SFX_CHANNELS];

	Audio::AudioStream *loadChannelData(const char *filename, Channel *ch, bool looping);

public:
	AmigaSoundMan_ns(Parallaction_ns *vm);
	~AmigaSoundMan_ns();
	void playMusic();
	void stopMusic();

	void playSfx(const char *filename, uint channel, bool looping, int volume);
	void stopSfx(uint channel);

	void playCharacterMusic(const char *character);
	void playLocationMusic(const char *location);
};

class DummySoundMan : public SoundManImpl {
public:
	void execute(int command, const char *parm) { }
};

class SoundMan_br : public SoundManImpl {
protected:
	Parallaction_br	*_vm;
	Audio::Mixer	*_mixer;

	Common::String _musicFile;

	bool 	_sfxLooping;
	int		_sfxVolume;
	int		_sfxRate;
	uint	_sfxChannel;

	bool	_musicEnabled;
	bool	_sfxEnabled;

	Channel _channels[NUM_SFX_CHANNELS];

	virtual void playMusic() = 0;
	virtual void stopMusic() = 0;
	virtual void pause(bool p) = 0;

public:
	SoundMan_br(Parallaction_br *vm);
	~SoundMan_br();

	virtual void playSfx(const char *filename, uint channel, bool looping, int volume = -1) { }
	void stopSfx(uint channel);
	void stopAllSfx();

	virtual void execute(int command, const char *parm);
	void setMusicFile(const char *parm);

	void enableSfx(bool enable);
	void enableMusic(bool enable);
	bool isSfxEnabled() const;
	bool isMusicEnabled() const;
};

class DosSoundMan_br : public SoundMan_br {

	MidiPlayer_MSC	*_midiPlayer;

	Audio::AudioStream *loadChannelData(const char *filename, Channel *ch, bool looping);

public:
	DosSoundMan_br(Parallaction_br *vm);
	~DosSoundMan_br();

	void playMusic();
	void stopMusic();
	void pause(bool p);

	void playSfx(const char *filename, uint channel, bool looping, int volume);
};

class AmigaSoundMan_br : public SoundMan_br {

	Audio::AudioStream *_musicStream;
	Audio::SoundHandle	_musicHandle;

	Audio::AudioStream *loadChannelData(const char *filename, Channel *ch, bool looping);

public:
	AmigaSoundMan_br(Parallaction_br *vm);
	~AmigaSoundMan_br();

	void playMusic();
	void stopMusic();
	void pause(bool p);

	void playSfx(const char *filename, uint channel, bool looping, int volume);
};

} // namespace Parallaction

#endif
