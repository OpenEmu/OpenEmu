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

#ifndef GOB_SOUND_SOUNDMIXER_H
#define GOB_SOUND_SOUNDMIXER_H

#include "common/mutex.h"
#include "common/frac.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Gob {

class SoundDesc;

class SoundMixer : public Audio::AudioStream {
public:
	SoundMixer(Audio::Mixer &mixer, Audio::Mixer::SoundType type);
	~SoundMixer();

	virtual void play(SoundDesc &sndDesc, int16 repCount,
			int16 frequency, int16 fadeLength = 0);
	virtual void stop(int16 fadeLength);

	bool isPlaying() const;
	char getPlayingSound() const;

	void setRepeating(int32 repCount);

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return _end; }
	bool endOfStream() const { return false; }
	int getRate() const { return _rate; }

protected:
	Audio::Mixer *_mixer;

	Audio::SoundHandle _handle;
	Common::Mutex _mutex;

	bool _16bit;

	bool _end;
	byte *_data;
	uint32 _length;
	uint32 _rate;
	int32 _freq;
	int32 _repCount;

	uint32 _offset;
	frac_t _offsetFrac;
	frac_t _offsetInc;

	int16 _cur;
	int16 _last;

	bool _fade;
	int32 _fadeVol;
	int32 _fadeVolStep;
	uint8 _fadeLength;
	uint32 _fadeSamples;
	uint32 _curFadeSamples;

	char _playingSound;

	virtual void setSample(SoundDesc &sndDesc, int16 repCount,
			int16 frequency, int16 fadeLength);
	virtual void checkEndSample();
	virtual void endFade();

private:
	inline int16 getData(int offset);
};

} // End of namespace Gob

#endif // GOB_SOUND_SOUNDMIXER_H
