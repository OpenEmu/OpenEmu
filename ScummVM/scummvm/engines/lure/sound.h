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

#ifndef LURE_SOUND_H
#define LURE_SOUND_H

#include "lure/luredefs.h"
#include "lure/disk.h"
#include "lure/memory.h"

#include "common/mutex.h"
#include "common/singleton.h"
#include "common/ptr.h"

#include "audio/mididrv.h"

class MidiParser;
class MidiChannel;

namespace Lure {

#define NUM_CHANNELS 16

struct ChannelEntry {
	MidiChannel *midiChannel;
	uint8 volume;
};

class MidiMusic: public MidiDriver_BASE {
private:
	uint8 _soundNumber;
	uint8 _channelNumber;
	uint8 _numChannels;
	byte _volume;
	MemoryBlock *_decompressedSound;
	uint8 *_soundData;
	uint8 _soundSize;
	MidiDriver *_driver;
	MidiParser *_parser;
	ChannelEntry *_channels;
	bool _isMusic;
	bool _isPlaying;

	void queueUpdatePos();
	uint8 randomQueuePos();
	uint32 songOffset(uint16 songNum) const;
	uint32 songLength(uint16 songNum) const;

public:
	MidiMusic(MidiDriver *driver, ChannelEntry channels[NUM_CHANNELS],
		 uint8 channelNum, uint8 soundNum, bool isMus, uint8 numChannels, void *soundData, uint32 size);
	~MidiMusic();
	void setVolume(int volume);
	int getVolume() const { return _volume; }

	void playSong(uint16 songNum);
	void stopSong() { stopMusic(); }
	void playMusic();
	void stopMusic();
	void queueTuneList(int16 tuneList);
	bool queueSong(uint16 songNum);
	void toggleVChange();

	// MidiDriver_BASE interface implementation
	virtual void send(uint32 b);
	virtual void metaEvent(byte type, byte *data, uint16 length);

	void onTimer();

	uint8 channelNumber() const { return _channelNumber; }
	uint8 soundNumber() const { return _soundNumber; }
	bool isPlaying() const { return _isPlaying; }
	bool isMusic() const { return _isMusic; }
};

class SoundManager : public Common::Singleton<SoundManager> {
private:
	// Outer sound interface properties
	MemoryBlock *_descs;
	MemoryBlock *_soundData;
	uint8 _soundsTotal;
	int _numDescs;
	SoundDescResource *soundDescs() { return (SoundDescResource *) _descs->data(); }
	MidiDriver *_driver;
	typedef Common::List<Common::SharedPtr<SoundDescResource> > SoundList;
	typedef SoundList::iterator SoundListIterator;
	SoundList _activeSounds;
	typedef Common::List<Common::SharedPtr<MidiMusic> > MusicList;
	typedef MusicList::iterator MusicListIterator;
	MusicList _playingSounds;
	ChannelEntry _channelsInner[NUM_CHANNELS];
	bool _channelsInUse[NUM_CHANNELS];
	bool _isPlaying;
	bool _nativeMT32;
	bool _isRoland;
	Common::MutexRef _soundMutex;
	bool _paused;

	uint _musicVolume;
	uint _sfxVolume;

	// Internal support methods
	void bellsBodge();
	void musicInterface_TidySounds();
	static void onTimer(void *data);
	void doTimer();

public:
	SoundManager();
	~SoundManager();

	void saveToStream(Common::WriteStream *stream);
	void loadFromStream(Common::ReadStream *stream);

	void loadSection(uint16 sectionId);
	void killSounds();
	void addSound(uint8 soundIndex, bool tidyFlag = true);
	void addSound2(uint8 soundIndex);
	void stopSound(uint8 soundIndex);
	void killSound(uint8 soundNumber);
	void setVolume(uint8 soundNumber, uint8 volume);
	void syncSounds();
	void tidySounds();
	uint8 descIndexOf(uint8 soundNumber);
	SoundDescResource *findSound(uint8 soundNumber);
	void removeSounds();
	void restoreSounds();
	void fadeOut();
	void pause() { _paused = true; }
	void resume() { _paused = false; }
	bool getPaused() const { return _paused; }
	bool hasNativeMT32() const { return _nativeMT32; }
	bool isRoland() const { return _isRoland; }
	uint musicVolume() const { return _musicVolume; }
	uint sfxVolume() const { return _sfxVolume; }

	// The following methods implement the external sound player module
	void musicInterface_Initialize();
	void musicInterface_Play(uint8 soundNumber, uint8 channelNumber, uint8 numChannels = 4);
	void musicInterface_Stop(uint8 soundNumber);
	bool musicInterface_CheckPlaying(uint8 soundNumber);
	void musicInterface_SetVolume(uint8 channelNum, uint8 volume);
	void musicInterface_KillAll();
	void musicInterface_ContinuePlaying();
	void musicInterface_TrashReverb();
};

} // End of namespace Lure

#define Sound (::Lure::SoundManager::instance())

#endif
