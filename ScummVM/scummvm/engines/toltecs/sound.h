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
 *
 */

#ifndef TOLTECS_SOUND_H
#define TOLTECS_SOUND_H

#include "audio/mixer.h"	// for Audio::SoundHandle

#include "toltecs/toltecs.h"

namespace Toltecs {

// 0x1219

enum SoundChannelType {
	kChannelTypeEmpty		= 0,
	kChannelTypeBackground	= -1,
	kChannelTypeSfx			= -2,
	kChannelTypeSpeech		= -3
};

struct SoundChannel {
	int16 resIndex;
	int16 type;
	int16 volume;
	int16 panning;
	Audio::SoundHandle handle;
};

const int kMaxChannels = 4;

class Sound {
public:
	Sound(ToltecsEngine *vm);
	~Sound();

	void playSpeech(int16 resIndex);
	void playSound(int16 resIndex, int16 type, int16 volume);
	void playSoundAtPos(int16 resIndex, int16 x, int16 y);
	void updateSpeech();
	void stopSpeech();
	void stopAll();

	void saveState(Common::WriteStream *out);
	void loadState(Common::ReadStream *in, int version);

protected:
	ToltecsEngine *_vm;

	SoundChannel channels[kMaxChannels];

	void clearChannel(int channel);
	void internalPlaySound(int16 resIndex, int16 type, int16 volume, int16 panning);
	Audio::Mixer::SoundType getScummVMSoundType(SoundChannelType type) const;
};


} // End of namespace Toltecs

#endif /* TOLTECS_SOUND_H */
