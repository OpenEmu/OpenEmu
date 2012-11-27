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


#include "engines/engine.h"
#include "scumm/player_v1.h"
#include "scumm/scumm.h"

namespace Scumm {

#define FB_WNOISE 0x12000       /* feedback for white noise */
#define FB_PNOISE 0x08000       /* feedback for periodic noise */

#define TIMER_BASE_FREQ 1193000
#define FIXP_SHIFT  16

Player_V1::Player_V1(ScummEngine *scumm, Audio::Mixer *mixer, bool pcjr)
	: Player_V2(scumm, mixer, pcjr) {
	// Initialize channel code
	for (int i = 0; i < 4; ++i)
		clear_channel(i);

	_mplex_step = (_sampleRate << FIXP_SHIFT) / 1193000;
	_next_chunk = _repeat_chunk = 0;
	_forced_level = 0;
	_random_lsr = 0;
}

Player_V1::~Player_V1() {
}

void Player_V1::chainSound(int nr, byte *data) {
	uint i;
	for (i = 0; i < 4; ++i)
		clear_channel(i);

	_current_nr = nr;
	_current_data = data;
	_repeat_chunk = _next_chunk = data + (_pcjr ? 2 : 4);

	debug(4, "Chaining new sound %d", nr);
	if (_pcjr)
		parsePCjrChunk();
	else
		parseSpeakerChunk();
}

void Player_V1::startSound(int nr) {
	Common::StackLock lock(_mutex);

	byte *data = _vm->getResourceAddress(rtSound, nr);
	assert(data);

	int offset = _pcjr ? READ_LE_UINT16(data+4) : 6;
	int cprio = _current_data ? *(_current_data) & 0x7f : 0;
	int prio  = *(data + offset) & 0x7f;
	int restartable = *(data + offset) & 0x80;

	debug(4, "startSound %d: prio %d%s, cprio %d",
		  nr, prio, restartable ? " restartable" : "", cprio);

	if (!_current_nr || cprio <= prio) {
		if (_current_data && (*(_current_data) & 0x80)) {
			_next_nr = _current_nr;
			_next_data = _current_data;
		}

		chainSound(nr, data + offset);
	}
}

void Player_V1::stopAllSounds() {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < 4; i++)
		clear_channel(i);
	_repeat_chunk = _next_chunk = 0;
	_next_nr = _current_nr = 0;
	_next_data = _current_data = 0;
}

void Player_V1::stopSound(int nr) {
	Common::StackLock lock(_mutex);

	if (_next_nr == nr) {
		_next_nr = 0;
		_next_data = 0;
	}
	if (_current_nr == nr) {
		for (int i = 0; i < 4; i++) {
			clear_channel(i);
		}
		_repeat_chunk = _next_chunk = 0;
		_current_nr = 0;
		_current_data = 0;
		chainNextSound();
	}
}

void Player_V1::clear_channel(int i) {
	_channels[i].freq   = 0;
	_channels[i].volume = 15;
}

int Player_V1::getMusicTimer() {
	/* Do V1 games have a music timer? */
	return 0;
}

void Player_V1::parseSpeakerChunk() {
	set_mplex(3000);
	_forced_level = 0;

 parse_again:
	_chunk_type = READ_LE_UINT16(_next_chunk);
	debug(6, "parseSpeakerChunk: sound %d, offset %lx, chunk %x",
			_current_nr, (long)(_next_chunk - _current_data), _chunk_type);

	_next_chunk += 2;
	switch (_chunk_type) {
	case 0xffff:
		_current_nr = 0;
		_current_data = 0;
		_channels[0].freq = 0;
		_next_chunk = 0;
		chainNextSound();
		break;
	case 0xfffe:
		_repeat_chunk = _next_chunk;
		goto parse_again;

	case 0xfffd:
		_next_chunk = _repeat_chunk;
		goto parse_again;

	case 0xfffc:
		/* handle reset. We don't need this do we? */
		goto parse_again;

	case 0:
		_time_left = 1;
		set_mplex(READ_LE_UINT16(_next_chunk));
		_next_chunk += 2;
		break;
	case 1:
		set_mplex(READ_LE_UINT16(_next_chunk));
		_start = READ_LE_UINT16(_next_chunk + 2);
		_end   = READ_LE_UINT16(_next_chunk + 4);
		_delta = (int16) READ_LE_UINT16(_next_chunk + 6);
		_repeat_ctr = READ_LE_UINT16(_next_chunk + 8);
		_channels[0].freq = _start;
		_next_chunk += 10;
		debug(6, "chunk 1: mplex %d, freq %d -> %d step %d  x %d",
				_mplex, _start, _end, _delta, _repeat_ctr);
		break;
	case 2:
		_start = READ_LE_UINT16(_next_chunk);
		_end   = READ_LE_UINT16(_next_chunk + 2);
		_delta = (int16) READ_LE_UINT16(_next_chunk + 4);
		_channels[0].freq = 0;
		_next_chunk += 6;
		_forced_level = -1;
		debug(6, "chunk 2: %d -> %d step %d",
				_start, _end, _delta);
		break;
	case 3:
		_start = READ_LE_UINT16(_next_chunk);
		_end   = READ_LE_UINT16(_next_chunk + 2);
		_delta = (int16) READ_LE_UINT16(_next_chunk + 4);
		_channels[0].freq = 0;
		_next_chunk += 6;
		_forced_level = -1;
		debug(6, "chunk 3: %d -> %d step %d",
				_start, _end, _delta);
		break;
	}
}

void Player_V1::nextSpeakerCmd() {
	uint16 lsr;
	switch (_chunk_type) {
	case 0:
		if (--_time_left)
			return;
		_time_left = READ_LE_UINT16(_next_chunk);
		_next_chunk += 2;
		if (_time_left == 0xfffb) {
			/* handle 0xfffb?? */
			_time_left = READ_LE_UINT16(_next_chunk);
			_next_chunk += 2;
		}
		debug(7, "nextSpeakerCmd: chunk %d, offset %4lx: notelen %d",
				_chunk_type, (long)(_next_chunk - 2 - _current_data), _time_left);
		if (_time_left == 0) {
			parseSpeakerChunk();
		} else {
			_channels[0].freq = READ_LE_UINT16(_next_chunk);
			_next_chunk += 2;
			debug(7, "freq_current: %d", _channels[0].freq);
		}
		break;

	case 1:
		_channels[0].freq = (_channels[0].freq + _delta) & 0xffff;
		if (_channels[0].freq == _end) {
			if (!--_repeat_ctr) {
				parseSpeakerChunk();
				return;
			}
			_channels[0].freq = _start;
		}
		break;

	case 2:
		_start = (_start + _delta) & 0xffff;
		if (_start == _end) {
			parseSpeakerChunk();
			return;
		}
		set_mplex(_start);
		_forced_level = -_forced_level;
		break;
	case 3:
		_start = (_start + _delta) & 0xffff;
		if (_start == _end) {
			parseSpeakerChunk();
			return;
		}
		lsr = _random_lsr + 0x9248;
		lsr = (lsr >> 3) | (lsr << 13);
		_random_lsr = lsr;
		set_mplex((_start & lsr) | 0x180);
		_forced_level = -_forced_level;
		break;
	}
}

void Player_V1::parsePCjrChunk() {
	uint tmp;
	uint i;

	set_mplex(3000);
	_forced_level = 0;

parse_again:

	_chunk_type = READ_LE_UINT16(_next_chunk);
	debug(6, "parsePCjrChunk: sound %d, offset %4lx, chunk %x",
		  _current_nr, (long)(_next_chunk - _current_data), _chunk_type);

	_next_chunk += 2;
	switch (_chunk_type) {
	case 0xffff:
		for (i = 0; i < 4; ++i)
			clear_channel(i);
		_current_nr = 0;
		_current_data = 0;
		_repeat_chunk = _next_chunk = 0;
		chainNextSound();
		break;

	case 0xfffe:
		_repeat_chunk = _next_chunk;
		goto parse_again;

	case 0xfffd:
		_next_chunk = _repeat_chunk;
		goto parse_again;

	case 0xfffc:
		/* handle reset. We don't need this do we? */
		goto parse_again;

	case 0:
		set_mplex(READ_LE_UINT16(_next_chunk));
		_next_chunk += 2;
		for (i = 0; i < 4; i++) {
			tmp = READ_LE_UINT16(_next_chunk);
			_next_chunk += 2;
			if (tmp == 0xffff) {
				_channels[i].cmd_ptr = 0;
				continue;
			}
			_channels[i].attack    = READ_LE_UINT16(_current_data + tmp);
			_channels[i].decay     = READ_LE_UINT16(_current_data + tmp + 2);
			_channels[i].level     = READ_LE_UINT16(_current_data + tmp + 4);
			_channels[i].sustain_1 = READ_LE_UINT16(_current_data + tmp + 6);
			_channels[i].sustain_2 = READ_LE_UINT16(_current_data + tmp + 8);
			_channels[i].notelen   = 1;
			_channels[i].volume    = 15;
			_channels[i].cmd_ptr   = _current_data + tmp + 10;
		}
		break;

	case 1:
		set_mplex(READ_LE_UINT16(_next_chunk));
		tmp = READ_LE_UINT16(_next_chunk + 2);
		_channels[0].cmd_ptr = tmp != 0xffff ? _current_data + tmp : NULL;
		tmp = READ_LE_UINT16(_next_chunk + 4);
		_start = READ_LE_UINT16(_next_chunk + 6);
		_delta = (int16) READ_LE_UINT16(_next_chunk + 8);
		_time_left = READ_LE_UINT16(_next_chunk + 10);
		_next_chunk += 12;
		if (tmp >= 0xe0) {
			_channels[3].freq = tmp & 0xf;
			_value_ptr = &_channels[3].volume;
		} else {
			assert(!(tmp & 0x10));
			tmp = (tmp & 0x60) >> 5;
			_value_ptr = &_channels[tmp].freq;
			_channels[tmp].volume = 0;
		}
		*_value_ptr = _start;
		if (_channels[0].cmd_ptr) {
			tmp = READ_LE_UINT16(_channels[0].cmd_ptr);
			_start_2 = READ_LE_UINT16(_channels[0].cmd_ptr + 2);
			_delta_2 = (int16) READ_LE_UINT16(_channels[0].cmd_ptr + 4);
			_time_left_2 = READ_LE_UINT16(_channels[0].cmd_ptr + 6);
			_channels[0].cmd_ptr += 8;
			if (_value_ptr == &_channels[3].volume) {
				tmp = (tmp & 0x70) >> 4;
				if (tmp & 1)
					_value_ptr_2 = &_channels[tmp >> 1].volume;
				else
					_value_ptr_2 = &_channels[tmp >> 1].freq;
			} else {
				assert(!(tmp & 0x10));
				tmp = (tmp & 0x60) >> 5;
				_value_ptr_2 = &_channels[tmp].freq;
				_channels[tmp].volume = 0;
			}
			*_value_ptr_2 = _start_2;
		}
		debug(6, "chunk 1: %lu: %d step %d for %d, %lu: %d step %d for %d",
			  (long)(_value_ptr - (uint *)_channels), _start, _delta, _time_left,
			  (long)(_value_ptr_2 - (uint *)_channels), _start_2, _delta_2, _time_left_2);
		break;

	case 2:
		_start = READ_LE_UINT16(_next_chunk);
		_end   = READ_LE_UINT16(_next_chunk + 2);
		_delta = (int16) READ_LE_UINT16(_next_chunk + 4);
		_channels[0].freq = 0;
		_next_chunk += 6;
		_forced_level = -1;
		debug(6, "chunk 2: %d -> %d step %d",
			  _start, _end, _delta);
		break;
	case 3:
		set_mplex(READ_LE_UINT16(_next_chunk));
		tmp = READ_LE_UINT16(_next_chunk + 2);
		assert((tmp & 0xf0) == 0xe0);
		_channels[3].freq = tmp & 0xf;
		if ((tmp & 3) == 3) {
			_next_chunk += 2;
			_channels[2].freq = READ_LE_UINT16(_next_chunk + 2);
		}
		_channels[3].volume = READ_LE_UINT16(_next_chunk + 4);
		_repeat_ctr = READ_LE_UINT16(_next_chunk + 6);
		_delta = (int16) READ_LE_UINT16(_next_chunk + 8);
		_next_chunk += 10;
		break;
	}
}

void Player_V1::nextPCjrCmd() {
	uint i;
	int dummy;
	switch (_chunk_type) {
	case 0:
		for (i = 0; i < 4; i++) {
			if (!_channels[i].cmd_ptr)
				continue;
			if (!--_channels[i].notelen) {
				dummy = READ_LE_UINT16(_channels[i].cmd_ptr);
				if (dummy >= 0xfffe) {
					if (dummy == 0xfffe)
						_next_chunk = _current_data + 2;
					parsePCjrChunk();
					return;
				}
				_channels[i].notelen = 4 * dummy;
				dummy = READ_LE_UINT16(_channels[i].cmd_ptr + 2);
				if (dummy == 0) {
					_channels[i].hull_counter = 4;
					_channels[i].sustctr = _channels[i].sustain_2;
				} else {
					_channels[i].hull_counter = 1;
					_channels[i].freq = dummy;
				}
				debug(7, "chunk 0: channel %d play %d for %d",
					  i, dummy, _channels[i].notelen);
				_channels[i].cmd_ptr += 4;
			}


			switch (_channels[i].hull_counter) {
			case 1:
				_channels[i].volume -= _channels[i].attack;
				if ((int) _channels[i].volume <= 0) {
					_channels[i].volume = 0;
					_channels[i].hull_counter++;
				}
				break;
			case 2:
				_channels[i].volume += _channels[i].decay;
				if (_channels[i].volume >= _channels[i].level) {
					_channels[i].volume = _channels[i].level;
					_channels[i].hull_counter++;
				}
				break;
			case 4:
				if (--_channels[i].sustctr < 0) {
					_channels[i].sustctr = _channels[i].sustain_2;
					_channels[i].volume += _channels[i].sustain_1;
					if ((int) _channels[i].volume >= 15) {
						_channels[i].volume = 15;
						_channels[i].hull_counter++;
					}
				}
				break;
			}
		}
		break;

	case 1:
		_start += _delta;
		*_value_ptr = _start;
		if (!--_time_left) {
			_start = READ_LE_UINT16(_next_chunk);
			_next_chunk += 2;
			if (_start == 0xffff) {
				parsePCjrChunk();
				return;
			}
			_delta = (int16) READ_LE_UINT16(_next_chunk);
			_time_left = READ_LE_UINT16(_next_chunk + 2);
			_next_chunk += 4;
			*_value_ptr = _start;
		}

		if (_channels[0].cmd_ptr) {
			_start_2 += _delta_2;
			*_value_ptr_2 = _start_2;
			if (!--_time_left_2) {
				_start_2 = READ_LE_UINT16(_channels[0].cmd_ptr);
				if (_start_2 == 0xffff) {
					_next_chunk = _channels[0].cmd_ptr + 2;
					parsePCjrChunk();
					return;
				}
				_delta_2 = (int16) READ_LE_UINT16(_channels[0].cmd_ptr + 2);
				_time_left_2 = READ_LE_UINT16(_channels[0].cmd_ptr + 4);
				_channels[0].cmd_ptr += 6;
			}
		}
		break;

	case 2:
		_start += _delta;
		if (_start == _end) {
			parsePCjrChunk();
			return;
		}
		set_mplex(_start);
		debug(7, "chunk 2: mplex %d  curve %d", _start, _forced_level);
		_forced_level = -_forced_level;
		break;
	case 3:
		dummy = _channels[3].volume + _delta;
		if (dummy >= 15) {
			_channels[3].volume = 15;
		} else if (dummy <= 0) {
			_channels[3].volume = 0;
		} else {
			_channels[3].volume = dummy;
			break;
		}

		if (!--_repeat_ctr) {
			parsePCjrChunk();
			return;
		}
		_delta = READ_LE_UINT16(_next_chunk);
		_next_chunk += 2;
		break;
	}
}

void Player_V1::set_mplex(uint mplex) {
	if (mplex == 0)
		mplex = 65536;
	_mplex = mplex;
	_tick_len = _mplex_step * mplex;
}

void Player_V1::nextTick() {
	if (_next_chunk) {
		if (_pcjr)
			nextPCjrCmd();
		else
			nextSpeakerCmd();
	}
}

void Player_V1::generateSpkSamples(int16 *data, uint len) {
	uint i;

	memset(data, 0, 2 * sizeof(int16) * len);
	if (_channels[0].freq == 0) {
		if (_forced_level) {
			int sample = _forced_level * _volumetable[0];
			for (i = 0; i < len; i++)
				data[2*i] = data[2*i+1] = sample;
			debug(9, "speaker: %8x: forced one", _tick_len);
		} else if (!_level) {
			return;
		}
	} else {
		squareGenerator(0, _channels[0].freq, 0, 0, data, len);
		debug(9, "speaker: %8x: freq %d %.1f", _tick_len,
				_channels[0].freq, 1193000.0 / _channels[0].freq);
	}
	lowPassFilter(data, len);
}

void Player_V1::generatePCjrSamples(int16 *data, uint len) {
	uint i, j;
	uint freq, vol;
	bool hasdata = false;

	memset(data, 0, 2 * sizeof(int16) * len);

	if (_forced_level) {
		int sample = _forced_level * _volumetable[0];
		for (i = 0; i < len; i++)
			data[2*i] = data[2*i+1] = sample;
		hasdata = true;
		debug(9, "channel[4]: %8x: forced one", _tick_len);
	}

	for (i = 1; i < 3; i++) {
		freq = _channels[i].freq;
		if (freq) {
			for (j = 0; j < i; j++) {
				if (freq == _channels[j].freq) {
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
		freq = _channels[i].freq;
		vol  = _channels[i].volume;
		if (!_volumetable[_channels[i].volume]) {
			_timer_count[i] -= len << FIXP_SHIFT;
			if (_timer_count[i] < 0)
				_timer_count[i] = 0;
		} else if (i < 3) {
			hasdata = true;
			squareGenerator(i, freq, vol, 0, data, len);
			debug(9, "channel[%d]: %8x: freq %d %.1f ; volume %d",
				  i, _tick_len, freq, 111860.0 / freq,  vol);
		} else {
			int noiseFB = (freq & 4) ? FB_WNOISE : FB_PNOISE;
			int n = (freq & 3);

			freq = (n == 3) ? 2 * (_channels[2].freq) : 1 << (5 + n);
			hasdata = true;
			squareGenerator(i, freq, vol, noiseFB, data, len);
			debug(9, "channel[%d]: %x: noise freq %d %.1f ; volume %d",
				  i, _tick_len, freq, 111860.0 / freq,  vol);
		}
	}

	if (_level || hasdata)
		lowPassFilter(data, len);
}

} // End of namespace Scumm
