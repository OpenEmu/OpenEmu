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

#ifndef SCUMM_PLAYER_APPLEII_H
#define SCUMM_PLAYER_APPLEII_H

#include "common/mutex.h"
#include "common/scummsys.h"
#include "common/memstream.h"
#include "scumm/music.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/softsynth/sid.h"

namespace Scumm {

class ScummEngine;

/*
 * Optimized for use with periodical read/write phases when the buffer
 * is filled in a write phase and completely read in a read phase.
 * The growing strategy is optimized for repeated small (e.g. 2 bytes)
 * single writes resulting in large buffers
 * (avg.: 4KB, max: 18KB @ 16bit/22.050kHz (MM sound21)).
 */
class SampleBuffer {
public:
	SampleBuffer() : _data(0) {
		clear();
	}

	~SampleBuffer() {
		free(_data);
	}

	void clear() {
		free(_data);
		_data = 0;
		_capacity = 0;
		_writePos = 0;
		_readPos = 0;
	}

	void ensureFree(uint32 needed) {
		// if data was read completely, reset read/write pos to front
		if ((_writePos != 0) && (_writePos == _readPos)) {
			_writePos = 0;
			_readPos = 0;
		}

		// check for enough space at end of buffer
		uint32 freeEndCnt = _capacity - _writePos;
		if (needed <= freeEndCnt)
			return;

		uint32 avail = availableSize();

		// check for enough space at beginning and end of buffer
		if (needed <= _readPos + freeEndCnt) {
			// move unread data to front of buffer
			memmove(_data, _data + _readPos, avail);
			_writePos = avail;
			_readPos = 0;
		} else { // needs a grow
			byte *old_data = _data;
			uint32 new_len = avail + needed;

			_capacity = new_len + 2048;
			_data = (byte *)malloc(_capacity);

			if (old_data) {
				// copy old unread data to front of new buffer
				memcpy(_data, old_data + _readPos, avail);
				free(old_data);
				_writePos = avail;
				_readPos = 0;
			}
		}
	}

	uint32 availableSize() const {
		if (_readPos >= _writePos)
			return 0;
		return _writePos - _readPos;
	}

	uint32 write(const void *dataPtr, uint32 dataSize) {
		ensureFree(dataSize);
		memcpy(_data + _writePos, dataPtr, dataSize);
		_writePos += dataSize;
		return dataSize;
	}

	uint32 read(byte *dataPtr, uint32 dataSize) {
		uint32 avail = availableSize();
		if (avail == 0)
			return 0;
		if (dataSize > avail)
			dataSize = avail;
		memcpy(dataPtr, _data + _readPos, dataSize);
		_readPos += dataSize;
		return dataSize;
	}

private:
	uint32 _writePos;
	uint32 _readPos;
	uint32 _capacity;
	byte *_data;
};

// CPU_CLOCK according to AppleWin
static const double APPLEII_CPU_CLOCK = 1020484.5; // ~ 1.02 MHz

/*
 * Converts the 1-bit speaker state values into audio samples.
 * This is done by aggregation of the speaker states at each
 * CPU cycle in a sampling period into an audio sample.
 */
class SampleConverter {
private:
	void addSampleToBuffer(int sample) {
		int16 value = sample * _volume / _maxVolume;
		_buffer.write(&value, sizeof(value));
	}

public:
	SampleConverter() :
		_cyclesPerSampleFP(0),
		_missingCyclesFP(0),
		_sampleCyclesSumFP(0),
		_volume(_maxVolume)
	{}

	~SampleConverter() {}

	void reset() {
		_missingCyclesFP = 0;
		_sampleCyclesSumFP = 0;
		_buffer.clear();
	}

	uint32 availableSize() const {
		return _buffer.availableSize();
	}

	void setMusicVolume(int vol) {
		assert(vol >= 0 && vol <= _maxVolume);
		_volume = vol;
	}

	void setSampleRate(int rate) {
		/* ~46 CPU cycles per sample @ 22.05kHz */
		_cyclesPerSampleFP = int(APPLEII_CPU_CLOCK * (1 << PREC_SHIFT) / rate);
		reset();
	}

	void addCycles(byte level, const int cycles) {
		/* convert to fixed precision floats */
		int cyclesFP = cycles << PREC_SHIFT;

		// step 1: if cycles are left from the last call, process them first
		if (_missingCyclesFP > 0) {
			int n = (_missingCyclesFP < cyclesFP) ? _missingCyclesFP : cyclesFP;
			if (level)
				_sampleCyclesSumFP += n;
			cyclesFP -= n;
			_missingCyclesFP -= n;
			if (_missingCyclesFP == 0) {
				addSampleToBuffer(2*32767 * _sampleCyclesSumFP / _cyclesPerSampleFP - 32767);
			} else {
				return;
			}
		}

		_sampleCyclesSumFP = 0;

		// step 2: process blocks of cycles fitting into a whole sample
		while (cyclesFP >= _cyclesPerSampleFP) {
			addSampleToBuffer(level ? 32767 : -32767);
			cyclesFP -= _cyclesPerSampleFP;
		}

		// step 3: remember cycles left for next call
		if (cyclesFP > 0) {
			_missingCyclesFP = _cyclesPerSampleFP - cyclesFP;
			if (level)
				_sampleCyclesSumFP = cyclesFP;
		}
	}

	uint32 readSamples(void *buffer, int numSamples) {
		return _buffer.read((byte *)buffer, numSamples * 2) / 2;
	}

private:
	static const int PREC_SHIFT = 7;

private:
	int _cyclesPerSampleFP;   /* (fixed precision) */
	int _missingCyclesFP;     /* (fixed precision) */
	int _sampleCyclesSumFP;   /* (fixed precision) */
	int _volume; /* 0 - 256 */
	static const int _maxVolume = 256;
	SampleBuffer _buffer;
};

class Player_AppleII;

class AppleII_SoundFunction {
public:
	AppleII_SoundFunction() {}
	virtual ~AppleII_SoundFunction() {}
	virtual void init(Player_AppleII *player, const byte *params) = 0;
	/* returns true if finished */
	virtual bool update() = 0;
protected:
	Player_AppleII *_player;
};

class Player_AppleII : public Audio::AudioStream, public MusicEngine {
public:
	Player_AppleII(ScummEngine *scumm, Audio::Mixer *mixer);
	virtual ~Player_AppleII();

	virtual void setMusicVolume(int vol) { _sampleConverter.setMusicVolume(vol); }
	void setSampleRate(int rate) {
		_sampleRate = rate;
		_sampleConverter.setSampleRate(rate);
	}
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getSoundStatus(int sound) const;
	virtual int  getMusicTimer();

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return false; }
	int getRate() const { return _sampleRate; }

public:
	void speakerToggle();
	void generateSamples(int cycles);
	void wait(int interval, int count);

private:
	// sound number
	int _soundNr;
	// type of sound
	int _type;
	// number of loops left
	int _loop;
	// global sound param list
	const byte *_params;
	// speaker toggle state (0 / 1)
	byte _speakerState;
	// sound function
	AppleII_SoundFunction *_soundFunc;
	// cycle to sample converter
	SampleConverter _sampleConverter;

private:
	ScummEngine *_vm;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	int _sampleRate;
	Common::Mutex _mutex;

private:
	void resetState();
	bool updateSound();
};

} // End of namespace Scumm

#endif
