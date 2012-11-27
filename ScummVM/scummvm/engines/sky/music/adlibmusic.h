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

#ifndef SKY_MUSIC_ADLIBMUSIC_H
#define SKY_MUSIC_ADLIBMUSIC_H

#include "sky/music/musicbase.h"
#include "audio/audiostream.h"
#include "audio/fmopl.h"

namespace Sky {

class AdLibMusic : public Audio::AudioStream, public MusicBase {
public:
	AdLibMusic(Audio::Mixer *pMixer, Disk *pDisk);
	~AdLibMusic();

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const;
	bool endOfData() const;
	int getRate() const;
	virtual void setVolume(uint16 param);

private:
	FM_OPL *_opl;
	Audio::SoundHandle _soundHandle;
	uint8 *_initSequence;
	uint32 _sampleRate, _nextMusicPoll;
	virtual void setupPointers();
	virtual void setupChannels(uint8 *channelData);
	virtual void startDriver();

	void premixerCall(int16 *buf, uint len);
};

} // End of namespace Sky

#endif //ADLIBMUSIC_H
