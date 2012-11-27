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



#include "common/util.h"
#include "engines/engine.h"

#include "scumm/imuse/imuse_internal.h"
#include "scumm/saveload.h"
#include "scumm/scumm.h"

#include "audio/midiparser.h"

namespace Scumm {

////////////////////////////////////////
//
//  Miscellaneous
//
////////////////////////////////////////

#define IMUSE_SYSEX_ID 0x7D
#define YM2612_SYSEX_ID 0x7C
#define ROLAND_SYSEX_ID 0x41
#define PERCUSSION_CHANNEL 9

extern MidiParser *MidiParser_createRO();

uint16 Player::_active_notes[128];



//////////////////////////////////////////////////
//
// IMuse Player implementation
//
//////////////////////////////////////////////////

Player::Player() :
	_midi(NULL),
	_parser(NULL),
	_parts(NULL),
	_active(false),
	_scanning(false),
	_id(0),
	_priority(0),
	_volume(0),
	_pan(0),
	_transpose(0),
	_detune(0),
	_note_offset(0),
	_vol_eff(0),
	_track_index(0),
	_loop_to_beat(0),
	_loop_from_beat(0),
	_loop_counter(0),
	_loop_to_tick(0),
	_loop_from_tick(0),
	_speed(128),
	_isMT32(false),
	_isMIDI(false),
	_supportsPercussion(false),
	_se(0),
	_vol_chan(0) {
}

Player::~Player() {
	if (_parser) {
		delete _parser;
		_parser = 0;
	}
}

bool Player::startSound(int sound, MidiDriver *midi) {
	byte *ptr;
	int i;

	// Not sure what the old code was doing,
	// but we'll go ahead and do a similar check.
	ptr = _se->findStartOfSound(sound);
	if (!ptr) {
		error("Player::startSound(): Couldn't find start of sound %d", sound);
		return false;
	}

	_isMT32 = _se->isMT32(sound);
	_isMIDI = _se->isMIDI(sound);
	_supportsPercussion = _se->supportsPercussion(sound);

	_parts = NULL;
	_active = true;
	_midi = midi;
	_id = sound;

	loadStartParameters(sound);

	for (i = 0; i < ARRAYSIZE(_parameterFaders); ++i)
		_parameterFaders[i].init();
	hook_clear();

	if (start_seq_sound(sound) != 0) {
		_active = false;
		_midi = NULL;
		return false;
	}

	debugC(DEBUG_IMUSE, "Starting music %d", sound);
	return true;
}

int Player::getMusicTimer() const {
	return _parser ? (_parser->getTick() * 2 / _parser->getPPQN()) : 0;
}

bool Player::isFadingOut() const {
	int i;
	for (i = 0; i < ARRAYSIZE(_parameterFaders); ++i) {
		if (_parameterFaders[i].param == ParameterFader::pfVolume &&
		        _parameterFaders[i].end == 0) {
			return true;
		}
	}
	return false;
}

void Player::clear() {
	if (!_active)
		return;
	debugC(DEBUG_IMUSE, "Stopping music %d", _id);

	if (_parser) {
		_parser->unloadMusic();
		delete _parser;
		_parser = 0;
	}
	uninit_parts();
	_se->ImFireAllTriggers(_id);
	_active = false;
	_midi = NULL;
	_id = 0;
	_note_offset = 0;
}

void Player::hook_clear() {
	memset(&_hook, 0, sizeof(_hook));
}

int Player::start_seq_sound(int sound, bool reset_vars) {
	byte *ptr;

	if (reset_vars) {
		_loop_to_beat = 1;
		_loop_from_beat = 1;
		_track_index = 0;
		_loop_counter = 0;
		_loop_to_tick = 0;
		_loop_from_tick = 0;
	}

	ptr = _se->findStartOfSound(sound);
	if (ptr == NULL)
		return -1;
	delete _parser;

	if (!memcmp(ptr, "RO", 2)) {
		// Old style 'RO' resource
		_parser = MidiParser_createRO();
	} else if (!memcmp(ptr, "FORM", 4)) {
		// Humongous Games XMIDI resource
		_parser = MidiParser::createParser_XMIDI();
	} else {
		// SCUMM SMF resource
		_parser = MidiParser::createParser_SMF();
	}

	_parser->setMidiDriver(this);
	_parser->property(MidiParser::mpSmartJump, 1);
	_parser->loadMusic(ptr, 0);
	_parser->setTrack(_track_index);

	ptr = _se->findStartOfSound(sound, IMuseInternal::kMDhd);
	setSpeed(reset_vars ? (ptr ? (READ_BE_UINT32(&ptr[4]) && ptr[15] ? ptr[15] : 128) : 128) : _speed);

	return 0;
}

void Player::loadStartParameters(int sound) {
	_priority = 0x80;
	_volume = 0x7F;
	_vol_chan = 0xFFFF;
	_vol_eff = (_se->get_channel_volume(0xFFFF) << 7) >> 7;
	_pan = 0;
	_transpose = 0;
	_detune = 0;

	byte *ptr = _se->findStartOfSound(sound, IMuseInternal::kMDhd);
	uint32 size = 0;

	if (ptr) {
		ptr += 4;
		size = READ_BE_UINT32(ptr);
		ptr += 4;

		// MDhd chunks don't get used in MI1 and contain only zeroes.
		// We check for volume, priority and speed settings of zero here.
		if (size && (ptr[2] | ptr[3] | ptr[7])) {
			_priority = ptr[2];
			_volume = ptr[3];
			_pan = ptr[4];
			_transpose = ptr[5];
			_detune = ptr[6];
			setSpeed(ptr[7]);
		}
	}
}

void Player::uninit_parts() {
	assert(!_parts || _parts->_player == this);

	while (_parts)
		_parts->uninit();

	// In case another player is waiting to allocate parts
	if (_midi)
		_se->reallocateMidiChannels(_midi);
}

void Player::setSpeed(byte speed) {
	_speed = speed;
	if (_parser)
		_parser->setTimerRate(((_midi->getBaseTempo() * speed) >> 7) * _se->_tempoFactor / 100);
}

void Player::send(uint32 b) {
	byte cmd = (byte)(b & 0xF0);
	byte chan = (byte)(b & 0x0F);
	byte param1 = (byte)((b >> 8) & 0xFF);
	byte param2 = (byte)((b >> 16) & 0xFF);
	Part *part;

	switch (cmd >> 4) {
	case 0x8: // Key Off
		if (!_scanning) {
			if ((part = getPart(chan)) != 0)
				part->noteOff(param1);
		} else {
			_active_notes[param1] &= ~(1 << chan);
		}
		break;

	case 0x9: // Key On
		param1 += _note_offset;
		if (!_scanning) {
			if (_isMT32 && !_se->isNativeMT32())
				param2 = (((param2 * 3) >> 2) + 32) & 0x7F;
			if ((part = getPart(chan)) != 0)
				part->noteOn(param1, param2);
		} else {
			_active_notes[param1] |= (1 << chan);
		}
		break;

	case 0xB: // Control Change
		part = (param1 == 123 ? getActivePart(chan) : getPart(chan));
		if (!part)
			break;

		switch (param1) {
		case 0: // Bank select. Not supported
			break;
		case 1: // Modulation Wheel
			part->modulationWheel(param2);
			break;
		case 7: // Volume
			part->volume(param2);
			break;
		case 10: // Pan Position
			part->set_pan(param2 - 0x40);
			break;
		case 16: // Pitchbend Factor(non-standard)
			part->pitchBendFactor(param2);
			break;
		case 17: // GP Slider 2
			part->set_detune(param2 - 0x40);
			break;
		case 18: // GP Slider 3
			part->set_pri(param2 - 0x40);
			_se->reallocateMidiChannels(_midi);
			break;
		case 64: // Sustain Pedal
			part->sustain(param2 != 0);
			break;
		case 91: // Effects Level
			part->effectLevel(param2);
			break;
		case 93: // Chorus Level
			part->chorusLevel(param2);
			break;
		case 116: // XMIDI For Loop. Not supported
			// Used in the ending sequence of puttputt
			break;
		case 117: // XMIDI Next/Break. Not supported
			// Used in the ending sequence of puttputt
			break;
		case 123: // All Notes Off
			part->allNotesOff();
			break;
		default:
			error("Player::send(): Invalid control change %d", param1);
		}
		break;

	case 0xC: // Program Change
		part = getPart(chan);
		if (part) {
			if (_isMIDI) {
				if (param1 < 128)
					part->programChange(param1);
			} else {
				if (param1 < 32)
					part->load_global_instrument(param1);
			}
		}
		break;

	case 0xE: // Pitch Bend
		part = getPart(chan);
		if (part)
			part->pitchBend(((param2 << 7) | param1) - 0x2000);
		break;

	case 0xA: // Aftertouch
	case 0xD: // Channel Pressure
	case 0xF: // Sequence Controls
		break;

	default:
		if (!_scanning) {
			error("Player::send(): Invalid command %d", cmd);
			clear();
		}
	}
	return;
}

void Player::sysEx(const byte *p, uint16 len) {
	byte a;
	byte buf[128];
	Part *part;

	// Check SysEx manufacturer.
	a = *p++;
	--len;
	if (a != IMUSE_SYSEX_ID) {
		if (a == ROLAND_SYSEX_ID) {
			// Roland custom instrument definition.
			if (_isMIDI || _isMT32) {
				part = getPart(p[0] & 0x0F);
				if (part) {
					part->_instrument.roland(p - 1);
					if (part->clearToTransmit())
						part->_instrument.send(part->_mc);
				}
			}
		} else if (a == YM2612_SYSEX_ID) {
			// FM-TOWNS custom instrument definition
			_midi->sysEx_customInstrument(p[0], 'EUP ', p + 1);
		} else {
			// SysEx manufacturer 0x97 has been spotted in the
			// Monkey Island 2 AdLib music, so don't make this a
			// fatal error. See bug #1481383.
			// The Macintosh version of Monkey Island 2 simply
			// ignores these SysEx events too.
			if (a == 0)
				warning("Unknown SysEx manufacturer 0x00 0x%02X 0x%02X", p[0], p[1]);
			else
				warning("Unknown SysEx manufacturer 0x%02X", (int)a);
		}
		return;
	}
	--len;

	// Too big?
	if (len >= sizeof(buf) * 2)
		return;

	if (!_scanning) {
		for (a = 0; a < len + 1 && a < 19; ++a) {
			sprintf((char *)&buf[a * 3], " %02X", p[a]);
		} // next for
		if (a < len + 1) {
			buf[a * 3] = buf[a * 3 + 1] = buf[a * 3 + 2] = '.';
			++a;
		} // end if
		buf[a * 3] = '\0';
		debugC(DEBUG_IMUSE, "[%02d] SysEx:%s", _id, buf);
	}

	if (_se->_sysex)
		(*_se->_sysex)(this, p, len);
}

void Player::decode_sysex_bytes(const byte *src, byte *dst, int len) {
	while (len >= 0) {
		*dst++ = ((src[0] << 4) & 0xFF) | (src[1] & 0xF);
		src += 2;
		len -= 2;
	}
}

void Player::maybe_jump(byte cmd, uint track, uint beat, uint tick) {
	// Is this the hook I'm waiting for?
	if (cmd && _hook._jump[0] != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80) {
		_hook._jump[0] = _hook._jump[1];
		_hook._jump[1] = 0;
	}

	jump(track, beat, tick);
}

void Player::maybe_set_transpose(byte *data) {
	byte cmd;

	cmd = data[0];

	// Is this the hook I'm waiting for?
	if (cmd && _hook._transpose != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80)
		_hook._transpose = 0;

	setTranspose(data[1], (int8)data[2]);
}

void Player::maybe_part_onoff(byte *data) {
	byte cmd, *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	p = &_hook._part_onoff[chan];

	// Is this the hook I'm waiting for?
	if (cmd && *p != cmd)
		return;

	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = getPart(chan);
	if (part)
		part->set_onoff(data[2] != 0);
}

void Player::maybe_set_volume(byte *data) {
	byte cmd;
	byte *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	p = &_hook._part_volume[chan];

	// Is this the hook I'm waiting for?
	if (cmd && *p != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = getPart(chan);
	if (part)
		part->volume(data[2]);
}

void Player::maybe_set_program(byte *data) {
	byte cmd;
	byte *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	// Is this the hook I'm waiting for?
	p = &_hook._part_program[chan];

	if (cmd && *p != cmd)
		return;

	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = getPart(chan);
	if (part)
		part->programChange(data[2]);
}

void Player::maybe_set_transpose_part(byte *data) {
	byte cmd;
	byte *p;
	uint chan;

	cmd = data[1];
	chan = data[0];

	// Is this the hook I'm waiting for?
	p = &_hook._part_transpose[chan];

	if (cmd && *p != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part_set_transpose(chan, data[2], (int8)data[3]);
}

int Player::setTranspose(byte relative, int b) {
	Part *part;

	if (b > 24 || b < -24 || relative > 1)
		return -1;
	if (relative)
		b = transpose_clamp(_transpose + b, -24, 24);

	_transpose = b;

	for (part = _parts; part; part = part->_next) {
		part->set_transpose(part->_transpose);
	}

	return 0;
}

void Player::part_set_transpose(uint8 chan, byte relative, int8 b) {
	Part *part;

	if (b > 24 || b < -24)
		return;

	part = getPart(chan);
	if (!part)
		return;
	if (relative)
		b = transpose_clamp(b + part->_transpose, -7, 7);
	part->set_transpose(b);
}

bool Player::jump(uint track, uint beat, uint tick) {
	if (!_parser)
		return false;
	if (_parser->setTrack(track))
		_track_index = track;
	if (!_parser->jumpToTick((beat - 1) * TICKS_PER_BEAT + tick))
		return false;
	turn_off_pedals();
	return true;
}

bool Player::setLoop(uint count, uint tobeat, uint totick, uint frombeat, uint fromtick) {
	if (tobeat + 1 >= frombeat)
		return false;

	if (tobeat == 0)
		tobeat = 1;

	// FIXME: Thread safety?
	_loop_counter = 0; // Because of possible interrupts
	_loop_to_beat = tobeat;
	_loop_to_tick = totick;
	_loop_from_beat = frombeat;
	_loop_from_tick = fromtick;
	_loop_counter = count;

	return true;
}

void Player::clearLoop() {
	_loop_counter = 0;
}

void Player::turn_off_pedals() {
	Part *part;

	for (part = _parts; part; part = part->_next) {
		if (part->_pedal)
			part->sustain(false);
	}
}

Part *Player::getActivePart(uint8 chan) {
	Part *part = _parts;
	while (part) {
		if (part->_chan == chan)
			return part;
		part = part->_next;
	}
	return 0;
}

Part *Player::getPart(uint8 chan) {
	Part *part = getActivePart(chan);
	if (part)
		return part;

	part = _se->allocate_part(_priority, _midi);
	if (!part) {
		debug(1, "No parts available");
		return NULL;
	}

	// Insert part into front of parts list
	part->_prev = NULL;
	part->_next = _parts;
	if (_parts)
		_parts->_prev = part;
	_parts = part;


	part->_chan = chan;
	part->setup(this);

	return part;
}

void Player::setPriority(int pri) {
	Part *part;

	_priority = pri;
	for (part = _parts; part; part = part->_next) {
		part->set_pri(part->_pri);
	}
	_se->reallocateMidiChannels(_midi);
}

void Player::setPan(int pan) {
	Part *part;

	_pan = pan;
	for (part = _parts; part; part = part->_next) {
		part->set_pan(part->_pan);
	}
}

void Player::setDetune(int detune) {
	Part *part;

	_detune = detune;
	for (part = _parts; part; part = part->_next) {
		part->set_detune(part->_detune);
	}
}

void Player::setOffsetNote(int offset) {
	_note_offset = offset;
}

int Player::scan(uint totrack, uint tobeat, uint totick) {
	if (!_active || !_parser)
		return -1;

	if (tobeat == 0)
		tobeat++;

	turn_off_parts();
	memset(_active_notes, 0, sizeof(_active_notes));
	_scanning = true;

	// If the scan involves a track switch, scan to the end of
	// the current track so that our state when starting the
	// new track is fully up to date.
	if (totrack != _track_index)
		_parser->jumpToTick((uint32)-1, true);
	_parser->setTrack(totrack);
	if (!_parser->jumpToTick((tobeat - 1) * TICKS_PER_BEAT + totick, true)) {
		_scanning = false;
		return -1;
	}

	_scanning = false;
	_se->reallocateMidiChannels(_midi);
	play_active_notes();

	if (_track_index != totrack) {
		_track_index = totrack;
		_loop_counter = 0;
	}
	return 0;
}

void Player::turn_off_parts() {
	Part *part;

	for (part = _parts; part; part = part->_next)
		part->off();
	_se->reallocateMidiChannels(_midi);
}

void Player::play_active_notes() {
	int i, j;
	uint mask;
	Part *part;

	for (i = 0; i < 16; ++i) {
		part = getPart(i);
		if (part) {
			mask = 1 << i;
			for (j = 0; j < 128; ++j) {
				if (_active_notes[j] & mask)
					part->noteOn(j, 80);
			}
		}
	}
}

int Player::setVolume(byte vol) {
	Part *part;

	if (vol > 127)
		return -1;

	_volume = vol;
	_vol_eff = _se->get_channel_volume(_vol_chan) * (vol + 1) >> 7;

	for (part = _parts; part; part = part->_next) {
		part->volume(part->_vol);
	}

	return 0;
}

int Player::getParam(int param, byte chan) {
	switch (param) {
	case 0:
		return (byte)_priority;
	case 1:
		return (byte)_volume;
	case 2:
		return (byte)_pan;
	case 3:
		return (byte)_transpose;
	case 4:
		return (byte)_detune;
	case 5:
		return _speed;
	case 6:
		return _track_index;
	case 7:
		return getBeatIndex();
	case 8:
		return (_parser ? _parser->getTick() % TICKS_PER_BEAT : 0); // _tick_index;
	case 9:
		return _loop_counter;
	case 10:
		return _loop_to_beat;
	case 11:
		return _loop_to_tick;
	case 12:
		return _loop_from_beat;
	case 13:
		return _loop_from_tick;
	case 14:
	case 15:
	case 16:
	case 17:
		return query_part_param(param, chan);
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
		return _hook.query_param(param, chan);
	default:
		return -1;
	}
}

int Player::query_part_param(int param, byte chan) {
	Part *part;

	part = _parts;
	while (part) {
		if (part->_chan == chan) {
			switch (param) {
			case 14:
				return part->_on;
			case 15:
				return part->_vol;
			case 16:
// FIXME: Need to know where this occurs...
				error("Trying to cast instrument (%d, %d) -- please tell Fingolfin", param, chan);
// In old versions of the code, this used to return part->_program.
// This was changed in revision 2.29 of imuse.cpp (where this code used
// to reside).
//				return (int)part->_instrument;
			case 17:
				return part->_transpose;
			default:
				return -1;
			}
		}
		part = part->_next;
	}
	return 129;
}

void Player::onTimer() {
	// First handle any parameter transitions
	// that are occuring.
	transitionParameters();

	// Since the volume parameter can cause
	// the player to be deactivated, check
	// to make sure we're still active.
	if (!_active || !_parser)
		return;

	uint32 target_tick = _parser->getTick();
	uint beat_index = target_tick / TICKS_PER_BEAT + 1;
	uint tick_index = target_tick % TICKS_PER_BEAT;

	if (_loop_counter && (beat_index > _loop_from_beat ||
	    (beat_index == _loop_from_beat && tick_index >= _loop_from_tick))) {
		_loop_counter--;
		jump(_track_index, _loop_to_beat, _loop_to_tick);
	}
	_parser->onTimer();
}

// "time" is referenced as hundredths of a second.
// IS THAT CORRECT??
// We convert it to microseconds before proceeding
int Player::addParameterFader(int param, int target, int time) {
	int start;

	switch (param) {
	case ParameterFader::pfVolume:
		// HACK: If volume is set to 0 with 0 time,
		// set it so immediately but DON'T clear
		// the player. This fixes a problem with
		// music being cleared inappropriately
		// in S&M when playing with the Dinosaur.
		if (!target && !time) {
			setVolume(0);
			return 0;
		}

		// Volume fades are handled differently.
		start = _volume;
		break;

	case ParameterFader::pfTranspose:
		// FIXME: Is this transpose? And what's the scale?
		// It's set to fade to -2400 in the tunnel of love.
//		debug(0, "parameterTransition(3) outside Tunnel of Love?");
		start = _transpose;
//		target /= 200;
		break;

	case ParameterFader::pfSpeed: // impSpeed
		// FIXME: Is the speed from 0-100?
		// Right now I convert it to 0-128.
		start = _speed;
//		target = target * 128 / 100;
		break;

	case 127: {
		// FIXME? I *think* this clears all parameter faders.
		ParameterFader *ptr = &_parameterFaders[0];
		int i;
		for (i = ARRAYSIZE(_parameterFaders); i; --i, ++ptr)
			ptr->param = 0;
		return 0;
	}
	break;

	default:
		debug(0, "Player::addParameterFader(%d, %d, %d): Unknown parameter", param, target, time);
		return 0; // Should be -1, but we'll let the script think it worked.
	}

	ParameterFader *ptr = &_parameterFaders[0];
	ParameterFader *best = 0;
	int i;
	for (i = ARRAYSIZE(_parameterFaders); i; --i, ++ptr) {
		if (ptr->param == param) {
			best = ptr;
			start = ptr->end;
			break;
		} else if (!ptr->param) {
			best = ptr;
		}
	}

	if (best) {
		best->param = param;
		best->start = start;
		best->end = target;
		if (!time)
			best->total_time = 1;
		else
			best->total_time = (uint32)time * 10000;
		best->current_time = 0;
	} else {
		debug(0, "IMuse Player %d: Out of parameter faders", _id);
		return -1;
	}

	return 0;
}

void Player::transitionParameters() {
	uint32 advance = _midi->getBaseTempo();
	int value;

	ParameterFader *ptr = &_parameterFaders[0];
	int i;
	for (i = ARRAYSIZE(_parameterFaders); i; --i, ++ptr) {
		if (!ptr->param)
			continue;

		ptr->current_time += advance;
		if (ptr->current_time > ptr->total_time)
			ptr->current_time = ptr->total_time;
		value = (int32)ptr->start + (int32)(ptr->end - ptr->start) * (int32)ptr->current_time / (int32)ptr->total_time;

		switch (ptr->param) {
		case ParameterFader::pfVolume:
			// Volume.
			if (!value && !ptr->end) {
				clear();
				return;
			}
			setVolume((byte)value);
			break;

		case ParameterFader::pfTranspose:
			// FIXME: Is this really transpose?
			setTranspose(0, value / 100);
			setDetune(value % 100);
			break;

		case ParameterFader::pfSpeed: // impSpeed:
			// Speed.
			setSpeed((byte)value);
			break;

		default:
			ptr->param = 0;
		}

		if (ptr->current_time >= ptr->total_time)
			ptr->param = 0;
	}
}

uint Player::getBeatIndex() {
	return (_parser ? (_parser->getTick() / TICKS_PER_BEAT + 1) : 0);
}

void Player::removePart(Part *part) {
	// Unlink
	if (part->_next)
		part->_next->_prev = part->_prev;
	if (part->_prev)
		part->_prev->_next = part->_next;
	else
		_parts = part->_next;
	part->_next = part->_prev = 0;
}

void Player::fixAfterLoad() {
	_midi = _se->getBestMidiDriver(_id);
	if (!_midi) {
		clear();
	} else {
		start_seq_sound(_id, false);
		setSpeed(_speed);
		if (_parser)
			_parser->jumpToTick(_music_tick); // start_seq_sound already switched tracks
		_isMT32 = _se->isMT32(_id);
		_isMIDI = _se->isMIDI(_id);
		_supportsPercussion = _se->supportsPercussion(_id);
	}
}

void Player::metaEvent(byte type, byte *msg, uint16 len) {
	if (type == 0x2F)
		clear();
}



////////////////////////////////////////
//
//  Player save/load functions
//
////////////////////////////////////////

void Player::saveLoadWithSerializer(Serializer *ser) {
	static const SaveLoadEntry playerEntries[] = {
		MKLINE(Player, _active, sleByte, VER(8)),
		MKLINE(Player, _id, sleUint16, VER(8)),
		MKLINE(Player, _priority, sleByte, VER(8)),
		MKLINE(Player, _volume, sleByte, VER(8)),
		MKLINE(Player, _pan, sleInt8, VER(8)),
		MKLINE(Player, _transpose, sleByte, VER(8)),
		MKLINE(Player, _detune, sleInt8, VER(8)),
		MKLINE(Player, _vol_chan, sleUint16, VER(8)),
		MKLINE(Player, _vol_eff, sleByte, VER(8)),
		MKLINE(Player, _speed, sleByte, VER(8)),
		MK_OBSOLETE(Player, _song_index, sleUint16, VER(8), VER(19)),
		MKLINE(Player, _track_index, sleUint16, VER(8)),
		MK_OBSOLETE(Player, _timer_counter, sleUint16, VER(8), VER(17)),
		MKLINE(Player, _loop_to_beat, sleUint16, VER(8)),
		MKLINE(Player, _loop_from_beat, sleUint16, VER(8)),
		MKLINE(Player, _loop_counter, sleUint16, VER(8)),
		MKLINE(Player, _loop_to_tick, sleUint16, VER(8)),
		MKLINE(Player, _loop_from_tick, sleUint16, VER(8)),
		MK_OBSOLETE(Player, _tempo, sleUint32, VER(8), VER(19)),
		MK_OBSOLETE(Player, _cur_pos, sleUint32, VER(8), VER(17)),
		MK_OBSOLETE(Player, _next_pos, sleUint32, VER(8), VER(17)),
		MK_OBSOLETE(Player, _song_offset, sleUint32, VER(8), VER(17)),
		MK_OBSOLETE(Player, _tick_index, sleUint16, VER(8), VER(17)),
		MK_OBSOLETE(Player, _beat_index, sleUint16, VER(8), VER(17)),
		MK_OBSOLETE(Player, _ticks_per_beat, sleUint16, VER(8), VER(17)),
		MKLINE(Player, _music_tick, sleUint32, VER(19)),
		MKLINE(Player, _hook._jump[0], sleByte, VER(8)),
		MKLINE(Player, _hook._transpose, sleByte, VER(8)),
		MKARRAY(Player, _hook._part_onoff[0], sleByte, 16, VER(8)),
		MKARRAY(Player, _hook._part_volume[0], sleByte, 16, VER(8)),
		MKARRAY(Player, _hook._part_program[0], sleByte, 16, VER(8)),
		MKARRAY(Player, _hook._part_transpose[0], sleByte, 16, VER(8)),
		MKEND()
	};

	const SaveLoadEntry parameterFaderEntries[] = {
		MKLINE(ParameterFader, param,        sleInt16,  VER(17)),
		MKLINE(ParameterFader, start,        sleInt16,  VER(17)),
		MKLINE(ParameterFader, end,          sleInt16,  VER(17)),
		MKLINE(ParameterFader, total_time,   sleUint32, VER(17)),
		MKLINE(ParameterFader, current_time, sleUint32, VER(17)),
		MKEND()
	};

	if (!ser->isSaving() && _parser) {
		delete _parser;
		_parser = 0;
	}
	_music_tick = _parser ? _parser->getTick() : 0;

	int num;
	if (ser->isSaving()) {
		num = (_parts ? (_parts - _se->_parts + 1) : 0);
		ser->saveUint16(num);
	} else {
		num = ser->loadUint16();
		_parts = (num ? &_se->_parts[num - 1] : 0);
	}
	ser->saveLoadEntries(this, playerEntries);
	ser->saveLoadArrayOf(_parameterFaders, ARRAYSIZE(_parameterFaders),
	                     sizeof(ParameterFader), parameterFaderEntries);
	return;
}

} // End of namespace Scumm
