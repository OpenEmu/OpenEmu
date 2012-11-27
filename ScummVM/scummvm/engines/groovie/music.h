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

#ifndef GROOVIE_MUSIC_H
#define GROOVIE_MUSIC_H

#include "common/array.h"
#include "common/mutex.h"

class MidiParser;

namespace Groovie {

class GroovieEngine;

class MusicPlayer {
public:
	MusicPlayer(GroovieEngine *vm);
	virtual ~MusicPlayer();

	void playSong(uint32 fileref);
	void setBackgroundSong(uint32 fileref);
	void playCD(uint8 track);
	void startBackground();

	void frameTick();
	void setBackgroundDelay(uint16 delay);

	// Volume
	void setUserVolume(uint16 volume);
	void setGameVolume(uint16 volume, uint16 time);

private:
	// Song playback
	bool play(uint32 fileref, bool loop);
	bool _isPlaying;
	uint32 _backgroundFileRef;
	uint8 _prevCDtrack;

	uint16 _backgroundDelay;

	// T7G iOS credits mp3 stream
	void playCreditsIOS();
	void stopCreditsIOS();
	Audio::SoundHandle _handleCreditsIOS;

	// Volume fading
	uint32 _fadingStartTime;
	uint16 _fadingStartVolume;
	uint16 _fadingEndVolume;
	uint16 _fadingDuration;
	void applyFading();

protected:
	GroovieEngine *_vm;

	// Callback
	static void onTimer(void *data);
	virtual void onTimerInternal() {}
	Common::Mutex _mutex;

	// User volume
	uint16 _userVolume;
	// Game volume
	uint16 _gameVolume;

	// These are specific for each type of music
	virtual void updateVolume() = 0;
	virtual bool load(uint32 fileref, bool loop) = 0;
	virtual void unload();
};

class MusicPlayerMidi : public MusicPlayer, public MidiDriver_BASE {
public:
	MusicPlayerMidi(GroovieEngine *vm);
	~MusicPlayerMidi();

	// MidiDriver_BASE interface
	virtual void send(uint32 b);
	virtual void metaEvent(byte type, byte *data, uint16 length);

private:
	// Channel volumes
	byte _chanVolumes[0x10];
	void updateChanVolume(byte channel);

	void endTrack();

protected:
	byte *_data;
	MidiParser *_midiParser;
	MidiDriver *_driver;

	virtual void onTimerInternal();
	void updateVolume();
	void unload();

	bool loadParser(Common::SeekableReadStream *stream, bool loop);
};

class MusicPlayerXMI : public MusicPlayerMidi {
public:
	MusicPlayerXMI(GroovieEngine *vm, const Common::String &gtlName);
	~MusicPlayerXMI();

	void send(uint32 b);

protected:
	bool load(uint32 fileref, bool loop);

private:
	// Channel banks
	byte _chanBanks[0x10];

	// Output music type
	uint8 _musicType;

	// Timbres
	class Timbre {
	public:
		Timbre() : data(NULL) {}
		byte patch;
		byte bank;
		uint32 size;
		byte *data;
	};
	Common::Array<Timbre> _timbres;
	void loadTimbres(const Common::String &filename);
	void clearTimbres();
	void setTimbreAD(byte channel, const Timbre &timbre);
	void setTimbreMT(byte channel, const Timbre &timbre);
};

class MusicPlayerMac : public MusicPlayerMidi {
public:
	MusicPlayerMac(GroovieEngine *vm);

protected:
	bool load(uint32 fileref, bool loop);

private:
	Common::SeekableReadStream *decompressMidi(Common::SeekableReadStream *stream);
};

class MusicPlayerIOS : public MusicPlayer {
public:
	MusicPlayerIOS(GroovieEngine *vm);
	~MusicPlayerIOS();

protected:
	void updateVolume();
	bool load(uint32 fileref, bool loop);
	void unload();

private:
	Audio::SoundHandle _handle;
};

} // End of Groovie namespace

#endif // GROOVIE_MUSIC_H
