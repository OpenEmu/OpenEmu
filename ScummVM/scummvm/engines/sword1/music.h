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

#ifndef SWORD1_MUSIC_H
#define SWORD1_MUSIC_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "common/file.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/rate.h"

namespace Sword1 {

#define TOTAL_TUNES 270

class MusicHandle : public Audio::AudioStream {
private:
	Common::File _file;
	int32 _fading;
	int32 _fadeSamples;
	Audio::AudioStream *_audioSource;
public:
	MusicHandle() : _fading(0), _audioSource(NULL) {}
	virtual int readBuffer(int16 *buffer, const int numSamples);
	bool play(const Common::String &filename, bool loop);
	bool playPSX(uint16 id);
	void stop();
	void fadeUp();
	void fadeDown();
	bool streaming() const;
	int32 fading() { return _fading; }
	bool endOfData() const;
	bool endOfStream() const { return false; }
	bool isStereo() const;
	int getRate() const;
};

class Music : public Audio::AudioStream {
public:
	Music(Audio::Mixer *pMixer);
	~Music();
	void startMusic(int32 tuneId, int32 loopFlag);
	void fadeDown();
	void setVolume(uint8 volL, uint8 volR);
	void giveVolume(uint8 *volL, uint8 *volR);

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) {
		mixer(buffer, numSamples / 2);
		return numSamples;
	}
	bool isStereo() const { return true; }
	bool endOfData() const { return false; }
	int getRate() const { return _sampleRate; }

private:
	Audio::st_volume_t _volumeL, _volumeR;
	MusicHandle _handles[2];
	Audio::RateConverter *_converter[2];
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	uint32 _sampleRate;
	Common::Mutex _mutex;

	static void passMixerFunc(void *param, int16 *buf, uint len);
	void mixer(int16 *buf, uint32 len);

	static const char _tuneList[TOTAL_TUNES][8]; // in staticres.cpp
};

} // End of namespace Sword1

#endif // BSMUSIC_H
