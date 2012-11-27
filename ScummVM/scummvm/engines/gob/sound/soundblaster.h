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

#ifndef GOB_SOUND_SOUNDBLASTER_H
#define GOB_SOUND_SOUNDBLASTER_H

#include "common/mutex.h"
#include "audio/mixer.h"

#include "gob/sound/soundmixer.h"

namespace Gob {

class SoundDesc;

class SoundBlaster : public SoundMixer {
public:
	SoundBlaster(Audio::Mixer &mixer);
	~SoundBlaster();

	void playSample(SoundDesc &sndDesc, int16 repCount,
			int16 frequency, int16 fadeLength = 0);
	void stopSound(int16 fadeLength, SoundDesc *sndDesc = 0);

	void playComposition(const int16 *composition, int16 freqVal,
			SoundDesc *sndDescs = 0, int8 sndCount = 60);
	void stopComposition();
	void endComposition();

	void repeatComposition(int32 repCount);

protected:
	Common::Mutex _mutex;

	SoundDesc *_compositionSamples;
	int8 _compositionSampleCount;
	int16 _composition[50];
	int8 _compositionPos;

	int32 _compositionRepCount;

	SoundDesc *_curSoundDesc;

	void setSample(SoundDesc &sndDesc, int16 repCount,
			int16 frequency, int16 fadeLength);
	void checkEndSample();
	void endFade();

	void nextCompositionPos();
};

} // End of namespace Gob

#endif // GOB_SOUND_SOUNDBLASTER_H
