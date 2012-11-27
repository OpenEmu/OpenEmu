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

#ifndef AUDIO_MODS_PAULA_H
#define AUDIO_MODS_PAULA_H

#include "audio/audiostream.h"
#include "common/frac.h"
#include "common/mutex.h"

namespace Audio {

/**
 * Emulation of the "Paula" Amiga music chip
 * The interrupt frequency specifies the number of mixed wavesamples between
 * calls of the interrupt method
 */
class Paula : public AudioStream {
public:
	static const int NUM_VOICES = 4;
	enum {
		kPalSystemClock  = 7093790,
		kNtscSystemClock = 7159090,
		kPalCiaClock     = kPalSystemClock / 10,
		kNtscCiaClock    = kNtscSystemClock / 10,
		kPalPaulaClock   = kPalSystemClock / 2,
		kNtscPauleClock  = kNtscSystemClock / 2
	};

	/* TODO: Document this */
	struct Offset {
		uint	int_off;	// integral part of the offset
		frac_t	rem_off;	// fractional part of the offset, at least 0 and less than 1

		explicit Offset(int off = 0) : int_off(off), rem_off(0) {}
	};

	Paula(bool stereo = false, int rate = 44100, uint interruptFreq = 0);
	~Paula();

	bool playing() const { return _playing; }
	void setTimerBaseValue( uint32 ticksPerSecond ) { _timerBase = ticksPerSecond; }
	uint32 getTimerBaseValue() { return _timerBase; }
	void setSingleInterrupt(uint sampleDelay) { assert(sampleDelay < _intFreq); _curInt = sampleDelay; }
	void setSingleInterruptUnscaled(uint timerDelay) {
		setSingleInterrupt((uint)(((double)timerDelay * getRate()) / _timerBase));
	}
	void setInterruptFreq(uint sampleDelay) { _intFreq = sampleDelay; _curInt = 0; }
	void setInterruptFreqUnscaled(uint timerDelay) {
		setInterruptFreq((uint)(((double)timerDelay * getRate()) / _timerBase));
	}
	void clearVoice(byte voice);
	void clearVoices() { for (int i = 0; i < NUM_VOICES; ++i) clearVoice(i); }
	void startPlay() { _playing = true; }
	void stopPlay() { _playing = false; }
	void pausePlay(bool pause) { _playing = !pause; }

// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return _stereo; }
	bool endOfData() const { return _end; }
	int getRate() const { return _rate; }

protected:
	struct Channel {
		const int8 *data;
		const int8 *dataRepeat;
		uint32 length;
		uint32 lengthRepeat;
		int16 period;
		byte volume;
		Offset offset;
		byte panning; // For stereo mixing: 0 = far left, 255 = far right
		int dmaCount;
	};

	bool _end;
	Common::Mutex _mutex;

	virtual void interrupt() = 0;

	void startPaula() {
		_playing = true;
		_end = false;
	}

	void stopPaula() {
		_playing = false;
		_end = true;
	}

	void setChannelPanning(byte channel, byte panning) {
		assert(channel < NUM_VOICES);
		_voice[channel].panning = panning;
	}

	void disableChannel(byte channel) {
		assert(channel < NUM_VOICES);
		_voice[channel].data = 0;
	}

	void enableChannel(byte channel) {
		assert(channel < NUM_VOICES);
		Channel &ch = _voice[channel];
		ch.data = ch.dataRepeat;
		ch.length = ch.lengthRepeat;
		// actually first 2 bytes are dropped?
		ch.offset = Offset(0);
		// ch.period = ch.periodRepeat;
	}

	void setChannelPeriod(byte channel, int16 period) {
		assert(channel < NUM_VOICES);
		_voice[channel].period = period;
	}

	void setChannelVolume(byte channel, byte volume) {
		assert(channel < NUM_VOICES);
		_voice[channel].volume = volume;
	}

	void setChannelSampleStart(byte channel, const int8 *data) {
		assert(channel < NUM_VOICES);
		_voice[channel].dataRepeat = data;
	}

	void setChannelSampleLen(byte channel, uint32 length) {
		assert(channel < NUM_VOICES);
		assert(length < 32768/2);
		_voice[channel].lengthRepeat = 2 * length;
	}

	void setChannelData(uint8 channel, const int8 *data, const int8 *dataRepeat, uint32 length, uint32 lengthRepeat, int32 offset = 0) {
		assert(channel < NUM_VOICES);

		Channel &ch = _voice[channel];

		ch.dataRepeat = data;
		ch.lengthRepeat = length;
		enableChannel(channel);
		ch.offset = Offset(offset);

		ch.dataRepeat = dataRepeat;
		ch.lengthRepeat = lengthRepeat;
	}

	void setChannelOffset(byte channel, Offset offset) {
		assert(channel < NUM_VOICES);
		_voice[channel].offset = offset;
	}

	Offset getChannelOffset(byte channel) {
		assert(channel < NUM_VOICES);
		return _voice[channel].offset;
	}

	int getChannelDmaCount(byte channel) {
		assert(channel < NUM_VOICES);
		return _voice[channel].dmaCount;
	}

	void setChannelDmaCount(byte channel, int dmaVal = 0) {
		assert(channel < NUM_VOICES);
		_voice[channel].dmaCount = dmaVal;
	}

	void setAudioFilter(bool enable) {
		// TODO: implement
	}

private:
	Channel _voice[NUM_VOICES];

	const bool _stereo;
	const int _rate;
	const double _periodScale;
	uint _intFreq;
	uint _curInt;
	uint32 _timerBase;
	bool _playing;

	template<bool stereo>
	int readBufferIntern(int16 *buffer, const int numSamples);
};

} // End of namespace Audio

#endif
