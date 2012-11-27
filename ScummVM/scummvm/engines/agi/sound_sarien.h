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

#ifndef AGI_SOUND_SARIEN_H
#define AGI_SOUND_SARIEN_H

#include "audio/audiostream.h"

namespace Agi {

#define BUFFER_SIZE	410

#define WAVEFORM_SIZE   64
#define ENV_ATTACK	10000		/**< envelope attack rate */
#define ENV_DECAY       1000		/**< envelope decay rate */
#define ENV_SUSTAIN     100		/**< envelope sustain level */
#define ENV_RELEASE	7500		/**< envelope release rate */
#define NUM_CHANNELS    7		/**< number of sound channels */

enum AgiSoundFlags {
	AGI_SOUND_LOOP		= 0x0001,
	AGI_SOUND_ENVELOPE	= 0x0002
};
enum AgiSoundEnv {
	AGI_SOUND_ENV_ATTACK	= 3,
	AGI_SOUND_ENV_DECAY		= 2,
	AGI_SOUND_ENV_SUSTAIN	= 1,
	AGI_SOUND_ENV_RELEASE	= 0
};


/**
 * AGI engine sound channel structure.
 */
struct ChannelInfo {
	AgiSoundEmuType type;
	const uint8 *ptr; // Pointer to the AgiNote data
	const int16 *ins;
	int32 size;
	uint32 phase;
	uint32 flags;	// ORs values from AgiSoundFlags
	AgiSoundEnv adsr;
	int32 timer;
	uint32 end;
	uint32 freq;
	uint32 vol;
	uint32 env;
};

class SoundGenSarien : public SoundGen, public Audio::AudioStream {
public:
	SoundGenSarien(AgiBase *vm, Audio::Mixer *pMixer);
	~SoundGenSarien();

	void play(int resnum);
	void stop(void);

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const {
		return false;
	}

	bool endOfData() const {
		return false;
	}

	int getRate() const {
		// FIXME: Ideally, we should use _sampleRate.
		return 22050;
	}

private:
	ChannelInfo _chn[NUM_CHANNELS];
	uint8 _env;

	int16 *_sndBuffer;
	const int16 *_waveform;

	bool _useChorus;

	bool _playing;
	int _playingSound;

private:
	void playSound();
	uint32 mixSound();
	void fillAudio(int16 *stream, uint len);

	void stopNote(int i);
	void playNote(int i, int freq, int vol);

};

} // End of namespace Agi

#endif /* AGI_SOUND_SARIEN_H */
