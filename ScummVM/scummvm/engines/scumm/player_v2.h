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

#ifndef SCUMM_PLAYER_V2_H
#define SCUMM_PLAYER_V2_H

#include "scumm/player_v2base.h"

namespace Scumm {

/**
 * Scumm V2 PC-Speaker MIDI driver.
 * This simulates the pc speaker sound, which is driven  by the 8253 (square
 * wave generator) and a low-band filter.
 */
class Player_V2 : public Player_V2Base {
public:
	Player_V2(ScummEngine *scumm, Audio::Mixer *mixer, bool pcjr);
	virtual ~Player_V2();

	// MusicEngine API
	virtual void setMusicVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
//	virtual int  getMusicTimer();
	virtual int  getSoundStatus(int sound) const;

	// AudioStream API
	virtual int readBuffer(int16 *buffer, const int numSamples);

protected:
	unsigned int _update_step;
	unsigned int _decay;
	int _level;
	unsigned int _RNG;
	unsigned int _volumetable[16];

	int _timer_count[4];
	int _timer_output;

protected:
	virtual void generateSpkSamples(int16 *data, uint len);
	virtual void generatePCjrSamples(int16 *data, uint len);

	void lowPassFilter(int16 *data, uint len);
	void squareGenerator(int channel, int freq, int vol,
						int noiseFeedback, int16 *sample, uint len);
};

} // End of namespace Scumm

#endif
