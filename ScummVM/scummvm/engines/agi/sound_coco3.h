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

#ifndef AGI_SOUND_COCO3_H
#define AGI_SOUND_COCO3_H

#include "audio/audiostream.h"

namespace Agi {

struct CoCoNote {
	uint8  freq;
	uint8  volume;
	uint16 duration;    ///< Note duration

	/** Reads a CoCoNote through the given pointer. */
	void read(const uint8 *ptr) {
		freq = *ptr;
		volume = *(ptr + 1);
		duration = READ_LE_UINT16(ptr + 2);
	}
};

class SoundGenCoCo3 : public SoundGen, public Audio::AudioStream {
public:
	SoundGenCoCo3(AgiBase *vm, Audio::Mixer *pMixer);
	~SoundGenCoCo3();

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
};

} // End of namespace Agi

#endif /* AGI_SOUND_COCO3_H */
