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

#include "scumm/player_v2.h"
#include "scumm/scumm.h"

namespace Scumm {

#define SPK_DECAY   0xa000              /* Depends on sample rate */
#define PCJR_DECAY  0xa000              /* Depends on sample rate */

#define NG_PRESET 0x0f35        /* noise generator preset */
#define FB_WNOISE 0x12000       /* feedback for white noise */
#define FB_PNOISE 0x08000       /* feedback for periodic noise */


Player_V2::Player_V2(ScummEngine *scumm, Audio::Mixer *mixer, bool pcjr)
	: Player_V2Base(scumm, mixer, pcjr) {

	int i;

	// Initialize square generator
	_level = 0;

	_RNG = NG_PRESET;

	_pcjr = pcjr;

	if (_pcjr) {
		_decay = PCJR_DECAY;
		_update_step = (_sampleRate << FIXP_SHIFT) / (111860 * 2);
	} else {
		_decay = SPK_DECAY;
		_update_step = (_sampleRate << FIXP_SHIFT) / (1193000 * 2);
	}

	// Adapt _decay to sample rate.  It must be squared when
	// sample rate doubles.
	for (i = 0; (_sampleRate << i) < 30000; i++)
		_decay = _decay * _decay / 65536;

	_timer_output = 0;
	for (i = 0; i < 4; i++)
		_timer_count[i] = 0;

	setMusicVolume(255);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Player_V2::~Player_V2() {
	Common::StackLock lock(_mutex);
	_mixer->stopHandle(_soundHandle);
}

void Player_V2::setMusicVolume (int vol) {
	if (vol > 255)
		vol = 255;

	/* scale to int16, FIXME: find best value */
	double out = vol * 128 / 3;

	/* build volume table (2dB per step) */
	for (int i = 0; i < 15; i++) {
		/* limit volume to avoid clipping */
		if (out > 0xffff)
			_volumetable[i] = 0xffff;
		else
			_volumetable[i] = (int) out;

		out /= 1.258925412;         /* = 10 ^ (2/20) = 2dB */
	}
	_volumetable[15] = 0;
}

void Player_V2::stopAllSounds() {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < 4; i++) {
		clear_channel(i);
	}
	_next_nr = _current_nr = 0;
	_next_data = _current_data = 0;
}

void Player_V2::stopSound(int nr) {
	Common::StackLock lock(_mutex);

	if (_next_nr == nr) {
		_next_nr = 0;
		_next_data = 0;
	}
	if (_current_nr == nr) {
		for (int i = 0; i < 4; i++) {
			clear_channel(i);
		}
		_current_nr = 0;
		_current_data = 0;
		chainNextSound();
	}
}

void Player_V2::startSound(int nr) {
	Common::StackLock lock(_mutex);

	byte *data = _vm->getResourceAddress(rtSound, nr);
	assert(data);

	int cprio = _current_data ? *(_current_data + _header_len) : 0;
	int prio  = *(data + _header_len);
	int nprio = _next_data ? *(_next_data + _header_len) : 0;

	int restartable = *(data + _header_len + 1);

	if (!_current_nr || cprio <= prio) {
		int tnr = _current_nr;
		int tprio = cprio;
		byte *tdata  = _current_data;

		chainSound(nr, data);
		nr   = tnr;
		prio = tprio;
		data = tdata;
		restartable = data ? *(data + _header_len + 1) : 0;
	}

	if (!_current_nr) {
		nr = 0;
		_next_nr = 0;
		_next_data = 0;
	}

	if (nr != _current_nr
		&& restartable
		&& (!_next_nr
		|| nprio <= prio)) {

		_next_nr = nr;
		_next_data = data;
	}
}

int Player_V2::getSoundStatus(int nr) const {
	return _current_nr == nr || _next_nr == nr;
}

int Player_V2::readBuffer(int16 *data, const int numSamples) {
	Common::StackLock lock(_mutex);

	uint step;
	uint len = numSamples / 2;

	do {
		if (!(_next_tick >> FIXP_SHIFT)) {
			_next_tick += _tick_len;
			nextTick();
		}

		step = len;
		if (step > (_next_tick >> FIXP_SHIFT))
			step = (_next_tick >> FIXP_SHIFT);
		if (_pcjr)
			generatePCjrSamples(data, step);
		else
			generateSpkSamples(data, step);
		data += 2 * step;
		_next_tick -= step << FIXP_SHIFT;
	} while (len -= step);

	return numSamples;
}

void Player_V2::lowPassFilter(int16 *sample, uint len) {
	for (uint i = 0; i < len; i++) {
		_level = (int) (_level * _decay
				+ sample[0] * (0x10000 - _decay)) >> 16;
		sample[0] = sample[1] = _level;
		sample += 2;
	}
}

void Player_V2::squareGenerator(int channel, int freq, int vol,
								int noiseFeedback, int16 *sample, uint len) {
	int32 period = _update_step * freq;
	int32 nsample;
	if (period == 0)
		period = _update_step;

	for (uint i = 0; i < len; i++) {
		uint32 duration = 0;

		if (_timer_output & (1 << channel))
			duration += _timer_count[channel];

		_timer_count[channel] -= (1 << FIXP_SHIFT);
		while (_timer_count[channel] <= 0) {

			if (noiseFeedback) {
				if (_RNG & 1) {
					_RNG ^= noiseFeedback;
					_timer_output ^= (1 << channel);
				}
				_RNG >>= 1;
			} else {
				_timer_output ^= (1 << channel);
			}

			if (_timer_output & (1 << channel))
				duration += period;

			_timer_count[channel] += period;
		}

		if (_timer_output & (1 << channel))
			duration -= _timer_count[channel];

		nsample = *sample +
			(((int32) (duration - (1 << (FIXP_SHIFT - 1)))
				* (int32) _volumetable[vol]) >> FIXP_SHIFT);
		/* overflow: clip value */
		if (nsample > 0x7fff)
			nsample = 0x7fff;
		if (nsample < -0x8000)
			nsample = -0x8000;
		*sample = nsample;
		// The following write isn't necessary, because the lowPassFilter does it for us
		//sample[1] = sample[0];
		sample += 2;
	}
}

void Player_V2::generateSpkSamples(int16 *data, uint len) {
	int winning_channel = -1;
	for (int i = 0; i < 4; i++) {
		if (winning_channel == -1
			&& _channels[i].d.volume
			&& _channels[i].d.time_left) {
			winning_channel = i;
		}
	}

	memset(data, 0, 2 * sizeof(int16) * len);
	if (winning_channel != -1) {
		squareGenerator(0, _channels[winning_channel].d.freq, 0,
				0, data, len);
	} else if (_level == 0)
		/* shortcut: no sound is being played. */
		return;

	lowPassFilter(data, len);
}

void Player_V2::generatePCjrSamples(int16 *data, uint len) {
	int i, j;
	int freq, vol;

	memset(data, 0, 2 * sizeof(int16) * len);
	bool hasdata = false;

	for (i = 1; i < 3; i++) {
		freq = _channels[i].d.freq >> 6;
		if (_channels[i].d.volume && _channels[i].d.time_left) {
			for (j = 0; j < i; j++) {
				if (_channels[j].d.volume
					&& _channels[j].d.time_left
					&& freq == (_channels[j].d.freq >> 6)) {
					/* HACK: this channel is playing at
					 * the same frequency as another.
					 * Synchronize it to the same phase to
					 * prevent interference.
					 */
					_timer_count[i] = _timer_count[j];
					_timer_output ^= (1 << i) &
						(_timer_output ^ _timer_output << (i - j));
				}
			}
		}
	}

	for (i = 0; i < 4; i++) {
		freq = _channels[i].d.freq >> 6;
		vol  = (65535 - _channels[i].d.volume) >> 12;
		if (!_channels[i].d.volume || !_channels[i].d.time_left) {
			_timer_count[i] -= len << FIXP_SHIFT;
			if (_timer_count[i] < 0)
				_timer_count[i] = 0;
		} else if (i < 3) {
			hasdata = true;
			squareGenerator(i, freq, vol, 0, data, len);
		} else {
			int noiseFB = (freq & 4) ? FB_WNOISE : FB_PNOISE;
			int n = (freq & 3);

			freq = (n == 3) ? 2 * (_channels[2].d.freq>>6) : 1 << (5 + n);
			hasdata = true;
			squareGenerator(i, freq, vol, noiseFB, data, len);
		}
#if 0
		debug(9, "channel[%d]: freq %d %.1f ; volume %d",
				i, freq, 111860.0 / freq,  vol);
#endif
	}

	if (_level || hasdata)
		lowPassFilter(data, len);
}

} // End of namespace Scumm
