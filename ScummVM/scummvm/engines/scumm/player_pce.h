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

#ifndef SCUMM_PLAYER_PCE_H
#define SCUMM_PLAYER_PCE_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "scumm/music.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

// PCE sound engine is only used by Loom, which requires 16bit color support
#ifdef USE_RGB_COLOR

namespace Scumm {

class ScummEngine;
class PSG_HuC6280;

class Player_PCE : public Audio::AudioStream, public MusicEngine {
private:
	struct channel_t {
		int id;

		byte controlVec0;
		byte controlVec1;
		byte controlVec2;
		byte controlVec5;
		byte balance;
		byte balance2;
		byte controlVec8;
		byte controlVec9;
		byte controlVec10;
		byte controlVec11;
		int16 soundUpdateCounter;
		byte controlVec18;
		byte controlVec19;
		byte waveformCtrl;
		byte controlVec21;
		bool controlVec23;
		bool controlVec24;

		uint16 controlVecShort02;
		uint16 controlVecShort03;
		int16 controlVecShort06;
		uint16 freq;
		uint16 controlVecShort09;
		uint16 controlVecShort10;

		const byte* soundDataPtr;
		const byte* controlBufferPos;
	};

public:
	Player_PCE(ScummEngine *scumm, Audio::Mixer *mixer);
	virtual ~Player_PCE();

	virtual void setMusicVolume(int vol) { _maxvol = vol; }
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getSoundStatus(int sound) const;
	virtual int  getMusicTimer();

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return true; }
	bool endOfData() const { return false; }
	int getRate() const { return _sampleRate; }

private:
	ScummEngine *_vm;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	int _sampleRate;
	int _maxvol;

private:
	PSG_HuC6280 *_psg;
	channel_t channels[12];
	Common::Mutex _mutex;

	// number of samples per timer period
	int _samplesPerPeriod;
	int16* _sampleBuffer;
	int _sampleBufferCnt;

	void init();
	bool isPlaying();

	void PSG_Write(int reg, byte data);

	void setupWaveform(byte bank);
	void procA541(channel_t *channel);
	void updateSound();
	void procA731(channel_t *channel);
	void processSoundData(channel_t *channel);
	void procA9F3(int x);
	void procAA62(channel_t *channel, int a);
	uint16 procAAF6(int x);
	void procAB7F(channel_t *channel);
	void procAC24(channel_t *channel);
	void procACEA(channel_t *channel, int a);
	void procAD21(int a, int x);
	void procAD29(int value);
	void procAD3D(int a, int x);
};

} // End of namespace Scumm

#endif // USE_RGB_COLOR

#endif
