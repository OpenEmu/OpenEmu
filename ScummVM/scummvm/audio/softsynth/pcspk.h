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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AUDIO_SOFTSYNTH_PCSPK_H
#define AUDIO_SOFTSYNTH_PCSPK_H

#include "audio/audiostream.h"
#include "common/mutex.h"

namespace Audio {

class PCSpeaker : public AudioStream {
public:
	enum WaveForm {
		kWaveFormSquare = 0,
		kWaveFormSine,
		kWaveFormSaw,
		kWaveFormTriangle
	};

	PCSpeaker(int rate = 44100);
	~PCSpeaker();

	/** Play a note for length ms.
	 *
	 *  If length is negative, play until told to stop.
	 */
	void play(WaveForm wave, int freq, int32 length);
	/** Stop the currently playing note after delay ms. */
	void stop(int32 delay = 0);
	/** Adjust the volume. */
	void setVolume(byte volume);

	bool isPlaying() const;

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const	{ return false; }
	bool endOfData() const	{ return false; }
	bool endOfStream() const { return false; }
	int getRate() const	{ return _rate; }

protected:
	Common::Mutex _mutex;

	int _rate;
	WaveForm _wave;
	bool _playForever;
	uint32 _oscLength;
	uint32 _oscSamples;
	uint32 _remainingSamples;
	uint32 _mixedSamples;
	byte _volume;

	typedef int8 (*generatorFunc)(uint32, uint32);
	static const generatorFunc generateWave[];

	static int8 generateSquare(uint32 x, uint32 oscLength);
	static int8 generateSine(uint32 x, uint32 oscLength);
	static int8 generateSaw(uint32 x, uint32 oscLength);
	static int8 generateTriangle(uint32 x, uint32 oscLength);
};

} // End of namespace Audio

#endif // AUDIO_SOFTSYNTH_PCSPEAKER_H
