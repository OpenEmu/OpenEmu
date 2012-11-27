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

#ifndef DREAMWEB_SOUND_H
#define DREAMWEB_SOUND_H

#include "common/array.h"
#include "common/str.h"
#include "audio/mixer.h"

namespace DreamWeb {

class DreamWebEngine;

class DreamWebSound {
public:
	DreamWebSound(DreamWebEngine *vm);
	~DreamWebSound();

	bool loadSpeech(byte type1, int idx1, byte type2, int idx2);
	void volumeSet(uint8 value) { _volume = value; }
	void volumeChange(uint8 value, int8 direction);
	void playChannel0(uint8 index, uint8 repeat);
	void playChannel1(uint8 index);
	uint8 getChannel0Playing() { return _channel0Playing; }
	bool isChannel1Playing() { return _channel1Playing != 255; }
	void cancelCh0();
	void cancelCh1();
	void loadRoomsSample(uint8 sample);
	void soundHandler();
	void loadSounds(uint bank, const Common::String &suffix);

private:
	DreamWebEngine *_vm;

	struct Sample {
		uint offset;
		uint size;
		Sample(): offset(), size() {}
	};

	struct SoundData {
		Common::Array<Sample> samples;
		Common::Array<uint8> data;
	};

	SoundData _soundData[2];
	Common::Array<uint8> _speechData;

	Audio::SoundHandle _channelHandle[2];

	uint8 _currentSample;
	uint8 _channel0Playing;
	uint8 _channel0Repeat;
	bool _channel0NewSound;
	uint8 _channel1Playing;
	bool _channel1NewSound;

	uint8 _volume;
	uint8 _volumeTo;
	int8 _volumeDirection;
	uint8 _volumeCount;

	void volumeAdjust();
	void playSound(uint8 channel, uint8 id, uint8 loops);
	void stopSound(uint8 channel);
	bool loadSpeech(const Common::String &filename);
};

} // End of namespace DreamWeb

#endif
