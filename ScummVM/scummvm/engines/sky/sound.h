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

#ifndef SKY_SOUND_H
#define SKY_SOUND_H


#include "common/scummsys.h"
#include "audio/mixer.h"

namespace Sky {

class Disk;

enum {
	SOUND_CH0    = 0,
	SOUND_CH1    = 1,
	SOUND_BG     = 2,
	SOUND_VOICE  = 3,
	SOUND_SPEECH = 4
};

struct SfxQueue {
	uint8 count, fxNo, chan, vol;
};

#define MAX_QUEUED_FX 4

class Sound {
protected:

public:

	Audio::Mixer *_mixer;
	Audio::SoundHandle _voiceHandle;
	Audio::SoundHandle _effectHandle;
	Audio::SoundHandle _bgSoundHandle;
	Audio::SoundHandle _ingameSound0, _ingameSound1, _ingameSpeech;

	uint16 _saveSounds[2];

protected:

	void playSound(uint32 id, byte *sound, uint32 size, Audio::SoundHandle *handle);

public:
	Sound(Audio::Mixer *mixer, Disk *pDisk, uint8 pVolume);
	~Sound();

	void loadSection(uint8 pSection);
	void playSound(uint16 sound, uint16 volume, uint8 channel);
	void fnStartFx(uint32 sound, uint8 channel);
	bool startSpeech(uint16 textNum);
	bool speechFinished() { return !_mixer->isSoundHandleActive(_ingameSpeech); }
	void fnPauseFx();
	void fnUnPauseFx();
	void fnStopFx();
	void stopSpeech();
	void checkFxQueue();
	void restoreSfx();
	uint8 _soundsTotal;

private:
	Disk *_skyDisk;
	uint16 _sfxBaseOfs;
	uint8 *_soundData;
	uint8 *_sampleRates, *_sfxInfo;
	uint8 _mainSfxVolume;

	bool _isPaused;

	static uint16 _speechConvertTable[8];
	static SfxQueue _sfxQueue[MAX_QUEUED_FX];
};

} // End of namespace Sky

#endif
