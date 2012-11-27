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


#include "scumm/player_mod.h"
#include "audio/mixer.h"
#include "audio/rate.h"
#include "audio/decoders/raw.h"

namespace Scumm {

Player_MOD::Player_MOD(Audio::Mixer *mixer)
	: _mixer(mixer), _sampleRate(mixer->getOutputRate()) {
	int i;
	_mixamt = 0;
	_mixpos = 0;

	for (i = 0; i < MOD_MAXCHANS; i++) {
		_channels[i].id = 0;
		_channels[i].vol = 0;
		_channels[i].freq = 0;
		_channels[i].input = NULL;
		_channels[i].ctr = 0;
		_channels[i].pos = 0;
	}

	_playproc = NULL;
	_playparam = NULL;

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Player_MOD::~Player_MOD() {
	_mixer->stopHandle(_soundHandle);
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (!_channels[i].id)
			continue;
		delete _channels[i].input;
	}
}

void Player_MOD::setMusicVolume(int vol) {
	_maxvol = vol;
}

void Player_MOD::setUpdateProc(ModUpdateProc *proc, void *param, int freq) {
	_playproc = proc;
	_playparam = param;
	_mixamt = _sampleRate / freq;
}
void Player_MOD::clearUpdateProc() {
	_playproc = NULL;
	_playparam = NULL;
	_mixamt = 0;
}

void Player_MOD::startChannel(int id, void *data, int size, int rate, uint8 vol, int loopStart, int loopEnd, int8 pan) {
	int i;
	if (id == 0)
		error("player_mod - attempted to start channel id 0");

	for (i = 0; i < MOD_MAXCHANS; i++) {
		if (!_channels[i].id)
			break;
	}
	if (i == MOD_MAXCHANS) {
		warning("player_mod - too many music channels playing (%i max)",MOD_MAXCHANS);
		return;
	}
	_channels[i].id = id;
	_channels[i].vol = vol;
	_channels[i].pan = pan;
	_channels[i].freq = rate;
	_channels[i].ctr = 0;

	Audio::SeekableAudioStream *stream = Audio::makeRawStream((const byte *)data, size, rate, 0);
	if (loopStart != loopEnd) {
		_channels[i].input = new Audio::SubLoopingAudioStream(stream, 0, Audio::Timestamp(0, loopStart, rate), Audio::Timestamp(0, loopEnd, rate));
	} else {
		_channels[i].input = stream;
	}

	// read the first sample
	_channels[i].input->readBuffer(&_channels[i].pos, 1);
}

void Player_MOD::stopChannel(int id) {
	if (id == 0)
		error("player_mod - attempted to stop channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			delete _channels[i].input;
			_channels[i].input = NULL;
			_channels[i].id = 0;
			_channels[i].vol = 0;
			_channels[i].freq = 0;
			_channels[i].ctr = 0;
			_channels[i].pos = 0;
		}
	}
}
void Player_MOD::setChannelVol(int id, uint8 vol) {
	if (id == 0)
		error("player_mod - attempted to set volume for channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			_channels[i].vol = vol;
			break;
		}
	}
}

void Player_MOD::setChannelPan(int id, int8 pan) {
	if (id == 0)
		error("player_mod - attempted to set pan for channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			_channels[i].pan = pan;
			break;
		}
	}
}

void Player_MOD::setChannelFreq(int id, int freq) {
	if (id == 0)
		error("player_mod - attempted to set frequency for channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			if (freq > 31400)	// this is about as high as WinUAE goes
				freq = 31400;	// can't easily verify on my own Amiga
			_channels[i].freq = freq;
			break;
		}
	}
}

void Player_MOD::do_mix(int16 *data, uint len) {
	int i;
	int dpos = 0;
	uint dlen = 0;
	memset(data, 0, 2 * len * sizeof(int16));
	while (len) {
		if (_playproc) {
			dlen = _mixamt - _mixpos;
			if (!_mixpos)
				_playproc(_playparam);
			if (dlen <= len) {
				_mixpos = 0;
				len -= dlen;
			} else {
				_mixpos = len;
				dlen = len;
				len = 0;
			}
		} else {
			dlen = len;
			len = 0;
		}
		for (i = 0; i < MOD_MAXCHANS; i++) {
			if (_channels[i].id) {
				Audio::st_volume_t vol_l = (127 - _channels[i].pan) * _channels[i].vol / 127;
				Audio::st_volume_t vol_r = (127 + _channels[i].pan) * _channels[i].vol / 127;
				for (uint j = 0; j < dlen; j++) {
					// simple linear resample, unbuffered
					int delta = (uint32)(_channels[i].freq * 0x10000) / _sampleRate;
					uint16 cfrac = ~_channels[i].ctr & 0xFFFF;
					if (_channels[i].ctr + delta < 0x10000)
						cfrac = delta;
					_channels[i].ctr += delta;
					int32 cpos = _channels[i].pos * cfrac / 0x10000;
					while (_channels[i].ctr >= 0x10000) {
						if (_channels[i].input->readBuffer(&_channels[i].pos, 1) != 1) {	// out of data
							stopChannel(_channels[i].id);
							goto skipchan;	// exit 2 loops at once
						}
						_channels[i].ctr -= 0x10000;
						if (_channels[i].ctr > 0x10000)
							cpos += _channels[i].pos;
						else
							cpos += (int32)(_channels[i].pos * (_channels[i].ctr & 0xFFFF)) / 0x10000;
					}
					int16 pos = 0;
					// if too many samples play in a row, the calculation below will overflow and clip
					// so try and split it up into pieces it can manage comfortably
					while (cpos < -0x8000) {
						pos -= 0x80000000 / delta;
						cpos += 0x8000;
					}
					while (cpos > 0x7FFF) {
						pos += 0x7FFF0000 / delta;
						cpos -= 0x7FFF;
					}
					pos += cpos * 0x10000 / delta;
					Audio::clampedAdd(data[(dpos + j) * 2 + 0], pos * vol_l / Audio::Mixer::kMaxMixerVolume);
					Audio::clampedAdd(data[(dpos + j) * 2 + 1], pos * vol_r / Audio::Mixer::kMaxMixerVolume);
				}
			}
skipchan:		;	// channel ran out of data
		}
		dpos += dlen;
	}
}

} // End of namespace Scumm
