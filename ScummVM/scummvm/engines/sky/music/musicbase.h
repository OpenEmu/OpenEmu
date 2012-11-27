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

#ifndef SKY_MUSIC_MUSICBASE_H
#define SKY_MUSIC_MUSICBASE_H


#include "common/scummsys.h"
#include "common/mutex.h"

#include "audio/mixer.h"

namespace Sky {

class Disk;

#define FILES_PER_SECTION 4

typedef struct {
	uint8 musicToProcess;
} Actions;

class ChannelBase {
public:
	virtual ~ChannelBase() {}
	virtual uint8 process(uint16 aktTime) = 0;
	virtual void updateVolume(uint16 pVolume) = 0;
	virtual bool isActive() = 0;
private:
};

class MusicBase {
public:
	MusicBase(Audio::Mixer *pMixer, Disk *pDisk);
	virtual ~MusicBase();
	void loadSection(uint8 pSection);
	void startMusic(uint16 param);
	void stopMusic();
	bool musicIsPlaying();
	uint8 giveVolume();
	uint8 giveCurrentMusic();
	virtual void setVolume(uint16 param) = 0;

protected:

	Audio::Mixer *_mixer;
	Disk *_skyDisk;
	uint8 *_musicData;

	uint16 _musicDataLoc;
	uint16 _driverFileBase;

	uint16 _musicVolume, _numberOfChannels;
	uint8 _currentMusic, _currentSection;
	uint8 _musicTempo0; // can be changed by music stream
	uint8 _musicTempo1; // given once per music
	uint32 _tempo;      // calculated from musicTempo0 and musicTempo1
	uint32 _aktTime;
	Actions _onNextPoll;
	ChannelBase *_channels[10];
	Common::Mutex _mutex;
	Audio::SoundHandle _musicHandle;

	virtual void setupPointers() = 0;
	virtual void setupChannels(uint8 *channelData) = 0;
	virtual void startDriver() = 0;

	void updateTempo();
	void loadNewMusic();
	void pollMusic();

	void stopMusicInternal();
};

} // End of namespace Sky

#endif //MUSICBASE_H
