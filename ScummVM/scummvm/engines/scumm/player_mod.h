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

#ifndef SCUMM_PLAYER_MOD_H
#define SCUMM_PLAYER_MOD_H

#include "scumm/scumm.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Audio {
class RateConverter;
}

namespace Scumm {

/**
 * Generic Amiga MOD mixer - provides a 60Hz 'update' routine.
 */
class Player_MOD : public Audio::AudioStream {
public:
	Player_MOD(Audio::Mixer *mixer);
	virtual ~Player_MOD();
	virtual void setMusicVolume(int vol);

	virtual void startChannel(int id, void *data, int size, int rate, uint8 vol, int loopStart = 0, int loopEnd = 0, int8 pan = 0);
	virtual void stopChannel(int id);
	virtual void setChannelVol(int id, uint8 vol);
	virtual void setChannelPan(int id, int8 pan);
	virtual void setChannelFreq(int id, int freq);

	typedef void ModUpdateProc(void *param);

	virtual void setUpdateProc(ModUpdateProc *proc, void *param, int freq);
	virtual void clearUpdateProc();

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) {
		do_mix(buffer, numSamples / 2);
		return numSamples;
	}
	bool isStereo() const { return true; }
	bool endOfData() const { return false; }
	int getRate() const { return _sampleRate; }

private:
	enum {
		MOD_MAXCHANS = 24
	};

	struct soundChan {
		int id;
		uint8 vol;
		int8 pan;
		uint16 freq;

		uint32 ctr;
		int16 pos;
		Audio::AudioStream *input;
	};

	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;

	uint32 _mixamt;
	uint32 _mixpos;
	const int _sampleRate;

	soundChan _channels[MOD_MAXCHANS];

	uint8 _maxvol;

	virtual void do_mix(int16 *buf, uint len);

	ModUpdateProc *_playproc;
	void *_playparam;
};

} // End of namespace Scumm

#endif
