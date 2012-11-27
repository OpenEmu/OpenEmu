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

/* Heavily based on code from NAGI
 *
 * COPYRIGHT AND PERMISSION NOTICE
 *
 * Copyright (c) 2001, 2001, 2002 Nick Sonneveld
 *
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
 * OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 *
 */

#include "agi/agi.h"
#include "agi/sound.h"
#include "agi/sound_pcjr.h"

namespace Agi {

// "fade out" or possibly "dissolve"
// v2.9xx
const int8 dissolveDataV2[] = {
	-2, -3, -2, -1,
	0x00, 0x00,
	0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08,
	0x09, 0x09, 0x09, 0x09,
	0x0A, 0x0A, 0x0A, 0x0A,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0D,
	-100
};

// v3
const int8 dissolveDataV3[] = {
	-2, -3, -2, -1,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08,
	0x09, 0x09, 0x09, 0x09,
	0x0A, 0x0A, 0x0A, 0x0A,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0D,
	-100
};


SoundGenPCJr::SoundGenPCJr(AgiBase *vm, Audio::Mixer *pMixer) : SoundGen(vm, pMixer) {
	_chanAllocated = 10240; // preallocate something which will most likely fit
	_chanData = (int16 *)malloc(_chanAllocated << 1);

	// Pick dissolve method
	//
	// 0 = no dissolve.. just play for as long as it's meant to be played
	// this was used in older v2.4 and under games i THINK
	// 1 = not used
	// 2 = v2.9+ games used a shorter dissolve
	// 3 (default) = v3 games used this dissolve pattern.. slightly longer
	if (_vm->getVersion() >= 0x3000)
		_dissolveMethod = 3;
	else if (_vm->getVersion() >= 0x2900)
		_dissolveMethod = 2;
	else
		_dissolveMethod = 0;

	_dissolveMethod = 3;

	memset(_channel, 0, sizeof(_channel));
	memset(_tchannel, 0, sizeof(_tchannel));

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_v1data = NULL;
	_v1size = 0;
}

SoundGenPCJr::~SoundGenPCJr() {
	free(_chanData);

	_mixer->stopHandle(_soundHandle);
}

void SoundGenPCJr::play(int resnum) {
	PCjrSound *pcjrSound = (PCjrSound *)_vm->_game.sounds[resnum];

	for (int i = 0; i < CHAN_MAX; i++) {
		_channel[i].data = pcjrSound->getVoicePointer(i % 4);
		_channel[i].duration = 0;
		_channel[i].avail = 0xffff;
		_channel[i].dissolveCount = 0xFFFF;
		_channel[i].attenuation = 0;
		_channel[i].attenuationCopy = 0;

		_tchannel[i].avail = 1;
		_tchannel[i].noteCount = 0;
		_tchannel[i].freqCount = 250;
		_tchannel[i].freqCountPrev = -1;
		_tchannel[i].atten = 0xF;	// silence
		_tchannel[i].genType = kGenTone;
		_tchannel[i].genTypePrev = -1;
	}

	_v1data = pcjrSound->getData() + 1;
	_v1size = pcjrSound->getLength() - 1;
}

void SoundGenPCJr::stop(void) {
	int i;

	for (i = 0; i < CHAN_MAX; i++) {
		_channel[i].avail = 0;
		_tchannel[i].avail = 0;
	}
}

int SoundGenPCJr::volumeCalc(SndGenChan *chan) {
	int8 attenuation, dissolveValue;

	const int8 *dissolveData;

	switch (_dissolveMethod) {
	case 2:
		dissolveData = dissolveDataV2;
		break;
	case 3:
	default:
		dissolveData = dissolveDataV3;
		break;
	}

	assert(chan);

	attenuation = chan->attenuation;
	if (attenuation != 0x0F) {	// != silence
		if (chan->dissolveCount != 0xFFFF) {
			dissolveValue = dissolveData[chan->dissolveCount];
			if (dissolveValue == -100) {	// if at end of list
				chan->dissolveCount = 0xFFFF;
				chan->attenuation = chan->attenuationCopy;
				attenuation = chan->attenuation;
			} else {
				chan->dissolveCount++;

				attenuation += dissolveValue;
				if (attenuation < 0)
					attenuation = 0;
				if (attenuation > 0x0F)
					attenuation = 0x0F;

				chan->attenuationCopy = attenuation;

				attenuation &= 0x0F;
				attenuation += _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 17;
				if (attenuation > 0x0F)
					attenuation = 0x0F;
			}
		}
		//if (computer_type == 2) && (attenuation < 8)
		if (attenuation < 8)
			attenuation += 2;
	}

	return attenuation;
}

int SoundGenPCJr::getNextNote(int ch)
{
	if (_vm->getVersion() > 0x2001)
		return getNextNote_v2(ch);
	else
		return getNextNote_v1(ch);

	return -1;
}

// read the next channel data.. fill it in *tone
// if tone isn't touched.. it should be inited so it just plays silence
// return 0 if it's passing more data
// return -1 if it's passing nothing (end of data)
int SoundGenPCJr::getNextNote_v2(int ch) {
	ToneChan *tpcm;
	SndGenChan *chan;
	const byte *data;

	assert(ch < CHAN_MAX);

	if (!_vm->getflag(fSoundOn))
		return -1;

	tpcm = &_tchannel[ch];
	chan = &_channel[ch];
	if (!chan->avail)
		return -1;

	while (chan->duration <= 0) {
		data = chan->data;

		// read the duration of the note
		chan->duration = READ_LE_UINT16(data);	// duration

		// if it's 0 then it's not going to be played
		// if it's 0xFFFF then the channel data has finished.
		if ((chan->duration == 0) || (chan->duration == 0xFFFF)) {
			tpcm->genTypePrev = -1;
			tpcm->freqCountPrev = -1;

			break;
		}

		_tchannel[ch].genTypePrev = -1;
		_tchannel[ch].freqCountPrev = -1;

		// only tone channels dissolve
		if ((ch != 3) && (_dissolveMethod != 0))	// != noise??
			chan->dissolveCount = 0;

		// attenuation (volume)
		writeData(data[4]);

		// frequency
		writeData(data[3]);
		writeData(data[2]);

		// data now points to the next data seg-a-ment
		chan->data += 5;
	}

	if (chan->duration == 0xFFFF) {
		// kill channel
		chan->avail = 0;
		chan->attenuation = 0x0F;	// silent
		chan->attenuationCopy = 0x0F;	// dunno really

		return -1;
	}

	chan->duration--;

	return 0;
}

int SoundGenPCJr::getNextNote_v1(int ch) {
	static int duration = 0;

	byte *data = _v1data;
	uint32 len = _v1size;

	if (len <= 0 || data == NULL) {
		_channel[ch].avail = 0;
		_channel[ch].attenuation = 0x0F;
		_channel[ch].attenuationCopy = 0x0F;
		return -1;
	}

	// In the V1 player the default duration for a row is 3 ticks
	if (duration > 0) {
		duration--;
		return 0;
	}
	duration = 3 * CHAN_MAX;

	// Otherwise fetch a row of data for all channels
	while (*data) {
		writeData(*data);
		data++;
		len--;
	}
	data++;
	len--;

	_v1data = data;
	_v1size = len;

	return 0;
}

void SoundGenPCJr::writeData(uint8 val) {
	static int reg = 0;

	debugC(5, kDebugLevelSound, "writeData(%.2X)", val);

	if ((val & 0x90) == 0x90) {
		reg = (val >> 5) & 0x3;
		_channel[reg].attenuation = val & 0xF;
	} else if ((val & 0xF0) == 0xE0) {
		_channel[3].genType = (val & 0x4) ? kGenWhite : kGenPeriod;
		int noiseFreq = val & 0x03;
		switch (noiseFreq) {
		case 0:
			_channel[3].freqCount = 32;
			break;
		case 1:
			_channel[3].freqCount = 64;
			break;
		case 2:
			_channel[3].freqCount = 128;
			break;
		case 3:
			_channel[3].freqCount = _channel[2].freqCount * 2;
			break;
		}
	} else if (val & 0x80) {
		reg = (val >> 5) & 0x3;
		_channel[reg].freqCount = val & 0xF;
		_channel[reg].genType = kGenTone;
	} else {
		_channel[reg].freqCount |= (val & 0x3F) << 4;
	}
}

// Formulas for noise generator
// bit0 = output

// noise feedback for white noise mode
#define FB_WNOISE 0x12000	// bit15.d(16bits) = bit0(out) ^ bit2
//#define FB_WNOISE 0x14000	// bit15.d(16bits) = bit0(out) ^ bit1
//#define FB_WNOISE 0x28000	// bit16.d(17bits) = bit0(out) ^ bit2 (same to AY-3-8910)
//#define FB_WNOISE 0x50000	// bit17.d(18bits) = bit0(out) ^ bit2

// noise feedback for periodic noise mode
// it is correct maybe (it was in the Megadrive sound manual)
//#define FB_PNOISE 0x10000	// 16bit rorate
#define FB_PNOISE 0x08000

// noise generator start preset (for periodic noise)
#define NG_PRESET 0x0f35

//#define WAVE_HEIGHT (0x7FFF)

// Volume table.
//
// 2dB = 20*log(a/b)
// 10^(2/20)*b = a;
// value = 0x7fff;
// value /= 1.258925411794;
const int16 volTable[16] = {
	32767, 26027, 20674, 16422, 13044, 10361, 8230, 6537, 5193, 4125, 3276, 2602, 2067, 1642, 1304, 0
};

#define FREQ_DIV 111844
#define MULT FREQ_DIV

// fill buff
int SoundGenPCJr::chanGen(int chan, int16 *stream, int len) {
	ToneChan *tpcm;
	int fillSize;
	int retVal;

	tpcm = &_tchannel[chan];

	retVal = -1;

	while (len > 0) {
		if (tpcm->noteCount <= 0) {
			// get new tone data
			if ((tpcm->avail) && (getNextNote(chan) == 0)) {
				tpcm->atten = _channel[chan].attenuation;
				tpcm->freqCount = _channel[chan].freqCount;
				tpcm->genType = _channel[chan].genType;

				// setup counters 'n stuff
				// SAMPLE_RATE samples per sec.. tone changes 60 times per sec
				tpcm->noteCount = SAMPLE_RATE / 60;
				retVal = 0;
			} else {
				// if it doesn't return an
				tpcm->genType = kGenSilence;
				tpcm->noteCount = len;
				tpcm->avail = 0;
			}
		}

		// write nothing
		if ((tpcm->freqCount == 0) || (tpcm->atten == 0xf)) {
			tpcm->genType = kGenSilence;
		}

		// find which is smaller.. the buffer or the
		fillSize = (tpcm->noteCount <= len) ? tpcm->noteCount : len;

		switch (tpcm->genType) {
			case kGenTone:
				fillSize = fillSquare(tpcm, stream, fillSize);
				break;
			case kGenPeriod:
			case kGenWhite:
				fillSize = fillNoise(tpcm, stream, fillSize);
				break;
			case kGenSilence:
			default:
				// fill with whitespace
				memset(stream, 0, fillSize * sizeof(int16));
				break;
		}

		tpcm->noteCount -= fillSize;
		stream += fillSize;
		len -= fillSize;
	}

	return retVal;
}

int SoundGenPCJr::fillSquare(ToneChan *t, int16 *buf, int len) {
	int count;

	if (t->genType != t->genTypePrev) {
		// make sure the freqCount is checked
		t->freqCountPrev = -1;
		t->sign = 1;
		t->genTypePrev = t->genType;
	}

	if (t->freqCount != t->freqCountPrev) {
		//t->scale = (int)( (double)t->samp->freq*t->freqCount/FREQ_DIV * MULT + 0.5);
		t->scale = (SAMPLE_RATE / 2) * t->freqCount;
		t->count = t->scale;
		t->freqCountPrev = t->freqCount;
	}

	count = len;

	while (count > 0) {
		*(buf++) = t->sign ? volTable[t->atten] : -volTable[t->atten];
		count--;

		// get next sample
		t->count -= MULT;
		while (t->count <= 0) {
			t->sign ^= 1;
			t->count += t->scale;
		}
	}

	return len;
}

int SoundGenPCJr::fillNoise(ToneChan *t, int16 *buf, int len) {
	int count;

	if (t->genType != t->genTypePrev) {
		// make sure the freqCount is checked
		t->freqCountPrev = -1;
		t->genTypePrev = t->genType;
	}

	if (t->freqCount != t->freqCountPrev) {
		//t->scale = (int)( (double)t->samp->freq*t->freqCount/FREQ_DIV * MULT + 0.5);
		t->scale = (SAMPLE_RATE / 2) * t->freqCount;
		t->count = t->scale;
		t->freqCountPrev = t->freqCount;

		t->feedback = (t->genType == kGenWhite) ? FB_WNOISE : FB_PNOISE;
		// reset noise shifter
		t->noiseState = NG_PRESET;
		t->sign = t->noiseState & 1;
	}

	count = len;

	while (count > 0) {
		*(buf++) = t->sign ? volTable[t->atten] : -volTable[t->atten];
		count--;

		// get next sample
		t->count -= MULT;
		while (t->count <= 0) {
			if (t->noiseState & 1)
				t->noiseState ^= t->feedback;

			t->noiseState >>= 1;
			t->sign = t->noiseState & 1;
			t->count += t->scale;
		}
	}

	return len;
}

int SoundGenPCJr::readBuffer(int16 *stream, const int len) {
	int streamCount;
	int16 *sPtr, *cPtr;

	if (_chanAllocated < len) {
		free(_chanData);
		_chanData = (int16 *)malloc(len << 1);
		_chanAllocated = len;
	}
	memset(stream, 0, len << 1);

	assert(stream);

	bool finished = true;

	for (int i = 0; i < CHAN_MAX; i++) {
		// get channel data(chan.userdata)
		if (chanGen(i, _chanData, len) == 0) {
			// divide by number of channels then add to stream
			streamCount = len;
			sPtr = stream;
			cPtr = _chanData;

			while (streamCount--)
				*(sPtr++) += *(cPtr++) / CHAN_MAX;

			finished = false;
		}
	}

	if (finished)
		_vm->_sound->soundIsFinished();

	return len;
}

} // End of namespace Agi
