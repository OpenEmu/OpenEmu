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

#include "sci/sound/drivers/mididriver.h"

#include "audio/softsynth/emumidi.h"

#include "common/debug.h"
#include "common/system.h"

namespace Sci {

#define VOLUME_SHIFT 3

#define BASE_NOTE 129	// A10
#define BASE_OCTAVE 10	// A10, as I said

static const int freq_table[12] = { // A4 is 440Hz, halftone map is x |-> ** 2^(x/12)
	28160, // A10
	29834,
	31608,
	33488,
	35479,
	37589,
	39824,
	42192,
	44701,
	47359,
	50175,
	53159
};

static inline int get_freq(int note) {
	int halftone_delta = note - BASE_NOTE;
	int oct_diff = ((halftone_delta + BASE_OCTAVE * 12) / 12) - BASE_OCTAVE;
	int halftone_index = (halftone_delta + (12 * 100)) % 12;
	int freq = (!note) ? 0 : freq_table[halftone_index] / (1 << (-oct_diff));

	return freq;
}

class MidiDriver_PCJr : public MidiDriver_Emulated {
public:
	friend class MidiPlayer_PCJr;

	enum {
		kMaxChannels = 3
	};

	MidiDriver_PCJr(Audio::Mixer *mixer) : MidiDriver_Emulated(mixer) { }
	~MidiDriver_PCJr() { }

	// MidiDriver
	int open() { return open(kMaxChannels); }
	void close();
	void send(uint32 b);
	MidiChannel *allocateChannel() { return NULL; }
	MidiChannel *getPercussionChannel() { return NULL; }

	// AudioStream
	bool isStereo() const { return false; }
	int getRate() const { return _mixer->getOutputRate(); }

	// MidiDriver_Emulated
	void generateSamples(int16 *buf, int len);

	int open(int channels);
private:
	int _channels_nr;
	int _global_volume; // Base volume
	int _volumes[kMaxChannels];
	int _notes[kMaxChannels]; // Current halftone, or 0 if off
	int _freq_count[kMaxChannels];
	int _channel_assigner;
	int _channels_assigned;
	int _chan_nrs[kMaxChannels];
};

void MidiDriver_PCJr::send(uint32 b) {
	byte command = b & 0xff;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;
	int i;
	int mapped_chan = -1;
	int chan_nr = command & 0xf;

	// First, test for channel having been assigned already
	if (_channels_assigned & (1 << chan_nr)) {
		// Already assigned this channel number:
		for (i = 0; i < _channels_nr; i++)
			if (_chan_nrs[i] == chan_nr) {
				mapped_chan = i;
				break;
			}
	} else if ((command & 0xe0) == 0x80) {
		// Assign new channel round-robin

		// Mark channel as unused:
		if (_chan_nrs[_channel_assigner] >= 0)
			_channels_assigned &= ~(1 << _chan_nrs[_channel_assigner]);

		// Remember channel:
		_chan_nrs[_channel_assigner] = chan_nr;
		// Mark channel as used
		_channels_assigned |= (1 << _chan_nrs[_channel_assigner]);

		// Save channel for use later in this call:
		mapped_chan = _channel_assigner;
		// Round-ropin iterate channel assigner:
		_channel_assigner = (_channel_assigner + 1) % _channels_nr;
	}

	if (mapped_chan == -1)
		return;

	switch (command & 0xf0) {

	case 0x80:
		if (op1 == _notes[mapped_chan])
			_notes[mapped_chan] = 0;
		break;

	case 0x90:
		if (!op2) {
			if (op1 == _notes[mapped_chan])
				_notes[mapped_chan] = 0;
		} else {
			_notes[mapped_chan] = op1;
			_volumes[mapped_chan] = op2;
		}
		break;

	case 0xb0:
		if ((op1 == SCI_MIDI_CHANNEL_NOTES_OFF) || (op1 == SCI_MIDI_CHANNEL_SOUND_OFF))
			_notes[mapped_chan] = 0;
		break;

	default:
		debug(2, "Unused MIDI command %02x %02x %02x", command, op1, op2);
		break; /* ignore */
	}
}

void MidiDriver_PCJr::generateSamples(int16 *data, int len) {
	int i;
	int chan;
	int freq[kMaxChannels];
	int frequency = getRate();

	for (chan = 0; chan < _channels_nr; chan++)
		freq[chan] = get_freq(_notes[chan]);

	for (i = 0; i < len; i++) {
		int16 result = 0;

		for (chan = 0; chan < _channels_nr; chan++)
			if (_notes[chan]) {
				int volume = (_global_volume * _volumes[chan])
				             >> VOLUME_SHIFT;

				_freq_count[chan] += freq[chan];
				while (_freq_count[chan] >= (frequency << 1))
					_freq_count[chan] -= (frequency << 1);

				if (_freq_count[chan] - freq[chan] < 0) {
					/* Unclean rising edge */
					int l = volume << 1;
					result += -volume + (l * _freq_count[chan]) / freq[chan];
				} else if (_freq_count[chan] >= frequency
				           && _freq_count[chan] - freq[chan] < frequency) {
					/* Unclean falling edge */
					int l = volume << 1;
					result += volume - (l * (_freq_count[chan] - frequency)) / freq[chan];
				} else {
					if (_freq_count[chan] < frequency)
						result += volume;
					else
						result += -volume;
				}
			}
		data[i] = result;
	}
}

int MidiDriver_PCJr::open(int channels) {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	if (channels > kMaxChannels)
		return -1;

	_channels_nr = channels;
	_global_volume = 100;
	for (int i = 0; i < _channels_nr; i++) {
		_volumes[i] = 100;
		_notes[i] = 0;
		_freq_count[i] = 0;
		_chan_nrs[i] = -1;
	}
	_channel_assigner = 0;
	_channels_assigned = 0;

	MidiDriver_Emulated::open();

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO);

	return 0;
}

void MidiDriver_PCJr::close() {
	_mixer->stopHandle(_mixerSoundHandle);
}

class MidiPlayer_PCJr : public MidiPlayer {
public:
	MidiPlayer_PCJr(SciVersion version) : MidiPlayer(version) { _driver = new MidiDriver_PCJr(g_system->getMixer()); }
	int open(ResourceManager *resMan) { return static_cast<MidiDriver_PCJr *>(_driver)->open(getPolyphony()); }
	byte getPlayId() const;
	int getPolyphony() const { return 3; }
	bool hasRhythmChannel() const { return false; }
	void setVolume(byte volume) { static_cast<MidiDriver_PCJr *>(_driver)->_global_volume = volume; }
};

byte MidiPlayer_PCJr::getPlayId() const {
	switch (_version) {
	case SCI_VERSION_0_EARLY:
		return 0x02;
	case SCI_VERSION_0_LATE:
		return 0x10;
	default:
		return 0x13;
	}
}

MidiPlayer *MidiPlayer_PCJr_create(SciVersion version) {
	return new MidiPlayer_PCJr(version);
}

class MidiPlayer_PCSpeaker : public MidiPlayer_PCJr {
public:
	MidiPlayer_PCSpeaker(SciVersion version) : MidiPlayer_PCJr(version) { }

	byte getPlayId() const;
	int getPolyphony() const { return 1; }
};

byte MidiPlayer_PCSpeaker::getPlayId() const {
	switch (_version) {
	case SCI_VERSION_0_EARLY:
		return 0x04;
	case SCI_VERSION_0_LATE:
		return 0x20;
	default:
		return 0x12;
	}
}

MidiPlayer *MidiPlayer_PCSpeaker_create(SciVersion version) {
	return new MidiPlayer_PCSpeaker(version);
}

} // End of namespace Sci
