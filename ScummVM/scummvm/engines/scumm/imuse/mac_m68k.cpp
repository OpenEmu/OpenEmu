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

#include "scumm/imuse/mac_m68k.h"

#include "common/util.h"
#include "common/macresman.h"
#include "common/stream.h"

namespace Scumm {

MacM68kDriver::MacM68kDriver(Audio::Mixer *mixer)
	: MidiDriver_Emulated(mixer) {
}

MacM68kDriver::~MacM68kDriver() {
}

int MacM68kDriver::open() {
	if (_isOpen) {
		return MERR_ALREADY_OPEN;
	}

	const int error = MidiDriver_Emulated::open();
	if (error) {
		return error;
	}

	for (uint i = 0; i < ARRAYSIZE(_channels); ++i) {
		_channels[i].init(this, i);
	}

	memset(_voiceChannels, 0, sizeof(_voiceChannels));
	_lastUsedVoiceChannel = 0;

	loadAllInstruments();

	_pitchTable[116] = 1664510;
	_pitchTable[117] = 1763487;
	_pitchTable[118] = 1868350;
	_pitchTable[119] = 1979447;
	_pitchTable[120] = 2097152;
	_pitchTable[121] = 2221855;
	_pitchTable[122] = 2353973;
	_pitchTable[123] = 2493948;
	_pitchTable[124] = 2642246;
	_pitchTable[125] = 2799362;
	_pitchTable[126] = 2965820;
	_pitchTable[127] = 3142177;
	for (int i = 115; i >= 0; --i) {
		_pitchTable[i] = _pitchTable[i + 12] / 2;
	}

	_volumeTable = new byte[8192];
	for (int i = 0; i < 32; ++i) {
		for (int j = 0; j < 256; ++j) {
			_volumeTable[i * 256 + j] = ((-128 + j) * _volumeBaseTable[i]) / 127 - 128;
		}
	}

	_mixBuffer = 0;
	_mixBufferLength = 0;

	// We set the output sound type to music here to allow sound volume
	// adjustment. The drawback here is that we can not control the music and
	// sfx separately here. But the AdLib output has the same issue so it
	// should not be that bad.
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_mixerSoundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	return 0;
}

void MacM68kDriver::close() {
	if (!_isOpen) {
		return;
	}

	_mixer->stopHandle(_mixerSoundHandle);
	_isOpen = false;
	for (InstrumentMap::iterator i = _instruments.begin(); i != _instruments.end(); ++i) {
		delete[] i->_value.data;
	}
	_instruments.clear();
	delete[] _volumeTable;
	_volumeTable = 0;
	delete[] _mixBuffer;
	_mixBuffer = 0;
	_mixBufferLength = 0;
}

void MacM68kDriver::send(uint32 d) {
	assert(false);
}

void MacM68kDriver::sysEx_customInstrument(byte channel, uint32 type, const byte *instr) {
	assert(false);
}

MidiChannel *MacM68kDriver::allocateChannel() {
	for (uint i = 0; i < ARRAYSIZE(_channels); ++i) {
		if (_channels[i].allocate()) {
			return &_channels[i];
		}
	}

	return 0;
}

MacM68kDriver::Instrument MacM68kDriver::getInstrument(int idx) const {
	InstrumentMap::const_iterator i = _instruments.find(idx);
	if (i != _instruments.end()) {
		return i->_value;
	} else {
		return _defaultInstrument;
	}
}

void MacM68kDriver::generateSamples(int16 *buf, int len) {
	int silentChannels = 0;

	if (_mixBufferLength < len) {
		delete[] _mixBuffer;

		_mixBufferLength = len;
		_mixBuffer = new int[_mixBufferLength];
		assert(_mixBuffer);
	}
	memset(_mixBuffer, 0, sizeof(int) * _mixBufferLength);

	for (int i = 0; i < kChannelCount; ++i) {
		OutputChannel &out = _voiceChannels[i].out;
		if (out.isFinished) {
			++silentChannels;
			continue;
		}

		byte *volumeTable = &_volumeTable[(out.volume / 4) * 256];
		int *buffer = _mixBuffer;

		int samplesLeft = len;
		while (samplesLeft) {
			out.subPos += out.pitchModifier;
			while (out.subPos >= 0x10000) {
				out.subPos -= 0x10000;
				out.instrument++;
			}

			if (out.instrument >= out.end) {
				if (!out.start) {
					break;
				}

				out.instrument = out.start;
				out.subPos = 0;
			}

			*buffer++ += volumeTable[*out.instrument];
			--samplesLeft;
		}

		if (samplesLeft) {
			out.isFinished = true;
			while (samplesLeft--) {
				*buffer++ += 0x80;
			}
		}
	}

	const int *buffer = _mixBuffer;
	const int silenceAdd = silentChannels << 7;
	while (len--) {
		*buf++ = (((*buffer++ + silenceAdd) >> 3) << 8) ^ 0x8000;
	}
}

void MacM68kDriver::loadAllInstruments() {
	Common::MacResManager resource;
	if (resource.open("iMUSE Setups")) {
		if (!resource.hasResFork()) {
			error("MacM68kDriver::loadAllInstruments: \"iMUSE Setups\" loaded, but no resource fork present");
		}

		for (int i = 0x3E7; i < 0x468; ++i) {
			Common::SeekableReadStream *stream = resource.getResource(MKTAG('s', 'n', 'd', ' '), i);
			if (stream) {
				addInstrument(i, stream);
				delete stream;
			}
		}

		for (int i = 0x7D0; i < 0x8D0; ++i) {
			Common::SeekableReadStream *stream = resource.getResource(MKTAG('s', 'n', 'd', ' '), i);
			if (stream) {
				addInstrument(i, stream);
				delete stream;
			}
		}

		InstrumentMap::iterator inst = _instruments.find(kDefaultInstrument);
		if (inst != _instruments.end()) {
			_defaultInstrument = inst->_value;
		} else {
			error("MacM68kDriver::loadAllInstruments: Could not load default instrument");
		}
	} else {
		error("MacM68kDriver::loadAllInstruments: Could not load \"iMUSE Setups\"");
	}
}

void MacM68kDriver::addInstrument(int idx, Common::SeekableReadStream *data) {
	// We parse the "SND" files manually here, since we need special data
	// from their header and need to work on them raw while mixing.
	data->skip(2);
	int count = data->readUint16BE();
	data->skip(2 * (3 * count));
	count = data->readUint16BE();
	data->skip(2 * (4 * count));

	Instrument inst;
	// Skip (optional) pointer to data
	data->skip(4);
	inst.length        = data->readUint32BE();
	inst.sampleRate    = data->readUint32BE();
	inst.loopStart     = data->readUint32BE();
	inst.loopEnd       = data->readUint32BE();
	// Skip encoding
	data->skip(1);
	inst.baseFrequency = data->readByte();

	inst.data = new byte[inst.length];
	assert(inst.data);
	data->read(inst.data, inst.length);
	_instruments[idx] = inst;
}

void MacM68kDriver::setPitch(OutputChannel *out, int frequency) {
	out->frequency = frequency;
	out->isFinished = false;

	const int pitchIdx = (frequency >> 7) + 60 - out->baseFrequency;
	assert(pitchIdx >= 0);

	const int low7Bits = frequency & 0x7F;
	if (low7Bits) {
		out->pitchModifier = _pitchTable[pitchIdx] + (((_pitchTable[pitchIdx + 1] - _pitchTable[pitchIdx]) * low7Bits) >> 7);
	} else {
		out->pitchModifier = _pitchTable[pitchIdx];
	}
}

void MacM68kDriver::VoiceChannel::off() {
	if (out.start) {
		out.isFinished = true;
	}

	part->removeVoice(this);
	part = 0;
}

void MacM68kDriver::MidiChannel_MacM68k::release() {
	_allocated = false;
	while (_voice) {
		_voice->off();
	}
}

void MacM68kDriver::MidiChannel_MacM68k::send(uint32 b) {
	uint8 type = b & 0xF0;
	uint8 p1 = (b >> 8) & 0xFF;
	uint8 p2 = (b >> 16) & 0xFF;

	switch (type) {
	case 0x80:
		noteOff(p1);
		break;

	case 0x90:
		if (p2) {
			noteOn(p1, p2);
		} else {
			noteOff(p1);
		}
		break;

	case 0xB0:
		controlChange(p1, p2);
		break;

	case 0xE0:
		pitchBend((p1 | (p2 << 7)) - 0x2000);
		break;

	default:
		break;
	}
}

void MacM68kDriver::MidiChannel_MacM68k::noteOff(byte note) {
	for (VoiceChannel *i = _voice; i; i = i->next) {
		if (i->note == note) {
			if (_sustain) {
				i->sustainNoteOff = true;
			} else {
				i->off();
			}
		}
	}
}

void MacM68kDriver::MidiChannel_MacM68k::noteOn(byte note, byte velocity) {
	// Do not start a not unless there is an instrument set up
	if (!_instrument.data) {
		return;
	}

	// Allocate a voice channel
	VoiceChannel *voice = _owner->allocateVoice(_priority);
	if (!voice) {
		return;
	}
	addVoice(voice);

	voice->note = note;
	// This completly ignores the note's volume, but is in accordance
	// to the original.
	voice->out.volume = _volume;

	// Set up the instrument data
	voice->out.baseFrequency = _instrument.baseFrequency;
	voice->out.soundStart    = _instrument.data;
	voice->out.soundEnd      = _instrument.data + _instrument.length;
	if (_instrument.loopEnd && _instrument.loopEnd - 12 > _instrument.loopStart) {
		voice->out.loopStart = _instrument.data + _instrument.loopStart;
		voice->out.loopEnd   = _instrument.data + _instrument.loopEnd;
	} else {
		voice->out.loopStart = 0;
		voice->out.loopEnd   = voice->out.soundEnd;
	}

	voice->out.start = voice->out.loopStart;
	voice->out.end   = voice->out.loopEnd;

	// Set up the pitch
	_owner->setPitch(&voice->out, (note << 7) + _pitchBend);

	// Set up the sample position
	voice->out.instrument = voice->out.soundStart;
	voice->out.subPos = 0;
}

void MacM68kDriver::MidiChannel_MacM68k::programChange(byte program) {
	_instrument = _owner->getInstrument(program + kProgramChangeBase);
}

void MacM68kDriver::MidiChannel_MacM68k::pitchBend(int16 bend) {
	_pitchBend = (bend * _pitchBendFactor) >> 6;
	for (VoiceChannel *i = _voice; i; i = i->next) {
		_owner->setPitch(&i->out, (i->note << 7) + _pitchBend);
	}
}

void MacM68kDriver::MidiChannel_MacM68k::controlChange(byte control, byte value) {
	switch (control) {
	// volume change
	case 7:
		_volume = value;
		for (VoiceChannel *i = _voice; i; i = i->next) {
			i->out.volume = value;
			i->out.isFinished = false;
		}
		break;

	// sustain
	case 64:
		_sustain = value;
		if (!_sustain) {
			for (VoiceChannel *i = _voice; i; i = i->next) {
				if (i->sustainNoteOff) {
					i->off();
				}
			}
		}
		break;

	// all notes off
	case 123:
		for (VoiceChannel *i = _voice; i; i = i->next) {
			i->off();
		}
		break;

	default:
		break;
	}
}

void MacM68kDriver::MidiChannel_MacM68k::pitchBendFactor(byte value) {
	_pitchBendFactor = value;
}

void MacM68kDriver::MidiChannel_MacM68k::priority(byte value) {
	_priority = value;
}

void MacM68kDriver::MidiChannel_MacM68k::sysEx_customInstrument(uint32 type, const byte *instr) {
	assert(instr);
	if (type == 'MAC ') {
		_instrument = _owner->getInstrument(*instr + kSysExBase);
	}
}

void MacM68kDriver::MidiChannel_MacM68k::init(MacM68kDriver *owner, byte channel) {
	_owner = owner;
	_number = channel;
	_allocated = false;
}

bool MacM68kDriver::MidiChannel_MacM68k::allocate() {
	if (_allocated) {
		return false;
	}

	_allocated = true;
	_voice = 0;
	_priority = 0;
	memset(&_instrument, 0, sizeof(_instrument));
	_pitchBend = 0;
	_pitchBendFactor = 0;
	_volume = 0;
	return true;
}

void MacM68kDriver::MidiChannel_MacM68k::addVoice(VoiceChannel *voice) {
	voice->next = _voice;
	voice->prev = 0;
	voice->part = this;
	if (_voice) {
		_voice->prev = voice;
	}
	_voice = voice;
}

void MacM68kDriver::MidiChannel_MacM68k::removeVoice(VoiceChannel *voice) {
	VoiceChannel *i = _voice;
	while (i && i != voice) {
		i = i->next;
	}

	if (i) {
		if (i->next) {
			i->next->prev = i->prev;
		}

		if (i->prev) {
			i->prev->next = i->next;
		} else {
			_voice = i->next;
		}
	}
}

MacM68kDriver::VoiceChannel *MacM68kDriver::allocateVoice(int priority) {
	VoiceChannel *channel = 0;
	for (int i = 0; i < kChannelCount; ++i) {
		if (++_lastUsedVoiceChannel == kChannelCount) {
			_lastUsedVoiceChannel = 0;
		}

		VoiceChannel *cur = &_voiceChannels[_lastUsedVoiceChannel];
		if (!cur->part) {
			memset(cur, 0, sizeof(*cur));
			return cur;
		} else if (!cur->next) {
			if (cur->part->_priority <= priority) {
				priority = cur->part->_priority;
				channel = cur;
			}
		}
	}

	if (channel) {
		channel->off();
		memset(channel, 0, sizeof(*channel));
	}

	return channel;
}

const int MacM68kDriver::_volumeBaseTable[32] = {
	  0,   0,   1,   1,   2,   3,   5,   6,
	  8,  11,  13,  16,  19,  22,  26,  30,
	 34,  38,  43,  48,  53,  58,  64,  70,
	 76,  83,  89,  96, 104, 111, 119, 127
};

} // End of namespace Scumm
