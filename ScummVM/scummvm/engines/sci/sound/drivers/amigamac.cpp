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

#include "audio/softsynth/emumidi.h"
#include "sci/sound/drivers/mididriver.h"
#include "sci/resource.h"

#include "common/debug-channels.h"
#include "common/file.h"
#include "common/frac.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Sci {

class MidiDriver_AmigaMac : public MidiDriver_Emulated {
public:
	enum {
		kVoices = 4
	};

	MidiDriver_AmigaMac(Audio::Mixer *mixer) : MidiDriver_Emulated(mixer), _playSwitch(true), _masterVolume(15) { }
	virtual ~MidiDriver_AmigaMac() { }

	// MidiDriver
	int open();
	void close();
	void send(uint32 b);
	MidiChannel *allocateChannel() { return NULL; }
	MidiChannel *getPercussionChannel() { return NULL; }

	// AudioStream
	bool isStereo() const { return true; }
	int getRate() const { return _mixer->getOutputRate(); }

	// MidiDriver_Emulated
	void generateSamples(int16 *buf, int len);

	void setVolume(byte volume);
	void playSwitch(bool play);
	virtual uint32 property(int prop, uint32 param);

private:
	enum {
		kModeLoop = 1 << 0, // Instrument looping flag
		kModePitch = 1 << 1 // Instrument pitch changes flag
	};

	enum {
		kChannels = 10,
		kBaseFreq = 20000, // Samplerate of the instrument bank
		kPanLeft = 91,
		kPanRight = 164
	};

	struct Channel {
		int instrument;
		int volume;
		int pan;
		uint16 pitch;
	};

	struct Envelope {
		int length; // Phase period length in samples
		int delta; // Velocity delta per period
		int target; // Target velocity
	};

	struct Voice {
		int instrument;
		int note;
		int note_velocity;
		int velocity;
		int envelope;
		int envelope_samples; // Number of samples till next envelope event
		int decay;
		int looping;
		int hw_channel;
		frac_t offset;
		frac_t rate;
	};

	struct InstrumentSample {
		char name[30];
		int mode;
		int size; // Size of non-looping part in bytes
		int loop_size; // Starting offset and size of loop in bytes
		int transpose; // Transpose value in semitones
		Envelope envelope[4]; // Envelope
		int8 *samples;
		int8 *loop;
		int16 startNote;
		int16 endNote;
		bool isUnsigned;
		uint16 baseFreq;
		uint16 baseNote;
		int16 fixedNote;
	};

	class Instrument : public Common::Array<InstrumentSample *> {
	public:
		char name[30];
	};

	struct Bank {
		char name[30];
		uint size;
		Common::Array<Instrument> instruments;
	};

	bool _isSci1;
	bool _isSci1Early; // KQ1/MUMG Amiga, patch 5
	bool _playSwitch;
	int _masterVolume;
	int _frequency;
	Envelope _envDecay;
	Bank _bank; // Instrument bank
	double _freqTable[48];

	Channel _channels[MIDI_CHANNELS];
	/* Internal channels */
	Voice _voices[kChannels];

	void setEnvelope(Voice *channel, Envelope *envelope, int phase);
	void setOutputFrac(int voice);
	int interpolate(int8 *samples, frac_t offset, bool isUnsigned);
	void playInstrument(int16 *dest, Voice *channel, int count);
	void changeInstrument(int channel, int instrument);
	void stopChannel(int ch);
	void stopNote(int ch, int note);
	void startNote(int ch, int note, int velocity);
	InstrumentSample *findInstrument(int instrument, int note);
	void pitchWheel(int ch, uint16 pitch);

	bool loadInstrumentsSCI0(Common::File &file);
	bool loadInstrumentsSCI0Mac(Common::SeekableReadStream &file);
	InstrumentSample *readInstrumentSCI0(Common::SeekableReadStream &file, int *id);
	bool loadInstrumentsSCI1(Common::SeekableReadStream &file);
};

void MidiDriver_AmigaMac::setEnvelope(Voice *channel, Envelope *envelope, int phase) {
	channel->envelope = phase;
	channel->envelope_samples = envelope[phase].length;

	if (phase == 0)
		channel->velocity = channel->note_velocity / 2;
	else
		channel->velocity = envelope[phase - 1].target;
}

int MidiDriver_AmigaMac::interpolate(int8 *samples, frac_t offset, bool isUnsigned) {
	int x = fracToInt(offset);

	if (isUnsigned) {
		int s1 = (byte)samples[x] - 0x80;
		int s2 = (byte)samples[x + 1] - 0x80;
		int diff = (s2 - s1) << 8;
		return (s1 << 8) + fracToInt(diff * (offset & FRAC_LO_MASK));
	}

	int diff = (samples[x + 1] - samples[x]) << 8;
	return (samples[x] << 8) + fracToInt(diff * (offset & FRAC_LO_MASK));
}

void MidiDriver_AmigaMac::playInstrument(int16 *dest, Voice *channel, int count) {
	int index = 0;
	int vol = _channels[channel->hw_channel].volume;
	InstrumentSample *instrument = findInstrument(channel->instrument, channel->note);

	while (1) {
		/* Available source samples until end of segment */
		frac_t lin_avail;
		uint32 seg_end, rem, i, amount;
		int8 *samples;

		if (channel->looping && instrument->loop) {
			samples = instrument->loop;
			seg_end = instrument->loop_size;
		} else {
			samples = instrument->samples;
			seg_end = instrument->size;
		}

		lin_avail = intToFrac(seg_end) - channel->offset;

		rem = count - index;

		/* Amount of destination samples that we will compute this iteration */
		amount = lin_avail / channel->rate;

		if (lin_avail % channel->rate)
			amount++;

		if (amount > rem)
			amount = rem;

		/* Stop at next envelope event */
		if ((channel->envelope_samples != -1) && (amount > (uint32)channel->envelope_samples))
			amount = channel->envelope_samples;

		for (i = 0; i < amount; i++) {
			dest[index++] = interpolate(samples, channel->offset, instrument->isUnsigned) * channel->velocity / 64 * channel->note_velocity * vol / (127 * 127);
			channel->offset += channel->rate;
		}

		if (channel->envelope_samples != -1)
			channel->envelope_samples -= amount;

		if (channel->envelope_samples == 0) {
			Envelope *envelope;
			int delta, target, velocity;

			if (channel->decay)
				envelope = &_envDecay;
			else
				envelope = &instrument->envelope[channel->envelope];

			delta = envelope->delta;
			target = envelope->target;
			velocity = channel->velocity - envelope->delta;

			/* Check whether we have reached the velocity target for the current phase */
			if ((delta >= 0 && velocity <= target) || (delta < 0 && velocity >= target)) {
				channel->velocity = target;

				/* Stop note after velocity has dropped to 0 */
				if (target == 0) {
					channel->note = -1;
					break;
				} else
					switch (channel->envelope) {
					case 0:
					case 2:
						/* Go to next phase */
						setEnvelope(channel, instrument->envelope, channel->envelope + 1);
						break;
					case 1:
					case 3:
						/* Stop envelope */
						channel->envelope_samples = -1;
						break;
					}
			} else {
				/* We haven't reached the target yet */
				channel->envelope_samples = envelope->length;
				channel->velocity = velocity;
			}
		}

		if (index == count)
			break;

		if ((uint32)fracToInt(channel->offset) >= seg_end) {
			if (instrument->mode & kModeLoop) {
				/* Loop the samples */
				channel->offset -= intToFrac(seg_end);
				channel->looping = 1;
			} else {
				/* All samples have been played */
				channel->note = -1;
				break;
			}
		}
	}
}

void MidiDriver_AmigaMac::changeInstrument(int channel, int instrument) {
	if (((uint)instrument < _bank.instruments.size()) && (_bank.instruments[instrument].size() > 0))
		debugC(1, kDebugLevelSound, "Amiga/Mac driver: Setting channel %i to \"%s\" (%i)", channel, _bank.instruments[instrument].name, instrument);
	else
		debugC(kDebugLevelSound, "Amiga/Mac driver: instrument %i does not exist (channel %i)", instrument, channel);
	_channels[channel].instrument = instrument;
}

void MidiDriver_AmigaMac::stopChannel(int ch) {
	int i;

	/* Start decay phase for note on this hw channel, if any */
	for (i = 0; i < kChannels; i++)
		if (_voices[i].note != -1 && _voices[i].hw_channel == ch && !_voices[i].decay) {
			/* Trigger fast decay envelope */
			_voices[i].decay = 1;
			_voices[i].envelope_samples = _envDecay.length;
			break;
		}
}

void MidiDriver_AmigaMac::pitchWheel(int ch, uint16 pitch) {
	_channels[ch].pitch = pitch;

	for (int i = 0; i < kChannels; i++)
		if (_voices[i].note != -1 && _voices[i].hw_channel == ch)
			setOutputFrac(i);
}

void MidiDriver_AmigaMac::stopNote(int ch, int note) {
	int channel;

	for (channel = 0; channel < kChannels; channel++)
		if (_voices[channel].note == note && _voices[channel].hw_channel == ch && !_voices[channel].decay)
			break;

	if (channel == kChannels) {
		debugC(1, kDebugLevelSound, "Amiga/Mac driver: cannot stop note %i on channel %i", note, ch);
		return;
	}

	InstrumentSample *instrument = findInstrument(_voices[channel].instrument, note);

	// FIXME: SCI1 envelope support is not perfect yet

	/* Start the envelope phases for note-off if looping is on and envelope is enabled */
	if ((instrument->mode & kModeLoop) && (instrument->envelope[0].length != 0))
		setEnvelope(&_voices[channel], instrument->envelope, 2);
}

MidiDriver_AmigaMac::InstrumentSample *MidiDriver_AmigaMac::findInstrument(int instrument, int note) {
	if ((uint)instrument >= _bank.instruments.size())
		return 0;

	for (uint32 i = 0; i < _bank.instruments[instrument].size(); i++) {
		InstrumentSample *sample = _bank.instruments[instrument][i];
		if (note >= sample->startNote && note <= sample->endNote)
			return sample;
	}

	return 0;
}

void MidiDriver_AmigaMac::setOutputFrac(int voice) {
	InstrumentSample *instrument = findInstrument(_voices[voice].instrument, _voices[voice].note);

	int fnote = 0;

	if (instrument->fixedNote == -1) {
		fnote = _voices[voice].note;

		// Handle SCI0-style transposing here
		if (!_isSci1)
			fnote += instrument->transpose;

		if (fnote < 0 || fnote > 127) {
			warning("Amiga/Mac driver: illegal note %i", fnote);
			return;
		}
	} else
		fnote = instrument->fixedNote;

	// Compute rate for note
	int mulFact = 1, divFact = 1;

	fnote -= instrument->baseNote;
	fnote *= 4;
	// FIXME: check how SSCI maps this
	fnote += (_channels[_voices[voice].hw_channel].pitch - 0x2000) / 169;

	while (fnote < 0) {
		divFact *= 2;
		fnote += 12 * 4;
	}

	while (fnote >= 12 * 4) {
		mulFact *= 2;
		fnote -= 12 * 4;
	}

	double freq = _freqTable[fnote] * instrument->baseFreq * mulFact / divFact;

	// Handle SCI1-style transposing here
	if (instrument->transpose && _isSci1)
		freq = freq + ((_freqTable[4] - 1.0) * freq * (double)instrument->transpose / (double)16);

	_voices[voice].rate = doubleToFrac(freq / _frequency);
}

void MidiDriver_AmigaMac::startNote(int ch, int note, int velocity) {
	int channel;

	if (_channels[ch].instrument < 0 || _channels[ch].instrument > 255) {
		warning("Amiga/Mac driver: invalid instrument %i on channel %i", _channels[ch].instrument, ch);
		return;
	}

	InstrumentSample *instrument = findInstrument(_channels[ch].instrument, note);

	if (!instrument) {
		warning("Amiga/Mac driver: instrument %i does not exist", _channels[ch].instrument);
		return;
	}

	for (channel = 0; channel < kChannels; channel++)
		if (_voices[channel].note == -1)
			break;

	if (channel == kChannels) {
		warning("Amiga/Mac driver: could not find a free channel");
		return;
	}

	stopChannel(ch);

	_voices[channel].instrument = _channels[ch].instrument;
	_voices[channel].note = note;
	_voices[channel].note_velocity = velocity;

	if ((instrument->mode & kModeLoop) && (instrument->envelope[0].length != 0))
		setEnvelope(&_voices[channel], instrument->envelope, 0);
	else {
		_voices[channel].velocity = 64;
		_voices[channel].envelope_samples = -1;
	}

	_voices[channel].offset = 0;
	_voices[channel].hw_channel = ch;
	_voices[channel].decay = 0;
	_voices[channel].looping = 0;
	setOutputFrac(channel);
}

MidiDriver_AmigaMac::InstrumentSample *MidiDriver_AmigaMac::readInstrumentSCI0(Common::SeekableReadStream &file, int *id) {
	byte header[61];

	if (file.read(header, 61) < 61) {
		warning("Amiga/Mac driver: failed to read instrument header");
		return NULL;
	}

	int seg_size[3];
	seg_size[0] = (int16)READ_BE_UINT16(header + 35) * 2;
	seg_size[1] = (int16)READ_BE_UINT16(header + 41) * 2;
	seg_size[2] = (int16)READ_BE_UINT16(header + 47) * 2;

	InstrumentSample *instrument = new InstrumentSample;

	instrument->startNote = 0;
	instrument->endNote = 127;
	instrument->isUnsigned = false;
	instrument->baseFreq = kBaseFreq;
	instrument->baseNote = 101;
	instrument->fixedNote = 101;

	instrument->mode = header[33];
	instrument->transpose = (int8)header[34];
	for (int i = 0; i < 4; i++) {
		int length = (int8)header[49 + i];

		if (length == 0 && i > 0)
			length = 256;

		instrument->envelope[i].length = length * _frequency / 60;
		instrument->envelope[i].delta = (int8)header[53 + i];
		instrument->envelope[i].target = header[57 + i];
	}
	/* Final target must be 0 */
	instrument->envelope[3].target = 0;

	int loop_offset = READ_BE_UINT32(header + 37) & ~1;
	int size = seg_size[0] + seg_size[1] + seg_size[2];

	*id = READ_BE_UINT16(header);

	strncpy(instrument->name, (char *) header + 2, 29);
	instrument->name[29] = 0;

	if (DebugMan.isDebugChannelEnabled(kDebugLevelSound)) {
		debug("Amiga/Mac driver: Reading instrument %i: \"%s\" (%i bytes)",
		       *id, instrument->name, size);
		debugN("    Mode: %02x (", header[33]);
		debugN("looping: %s, ", header[33] & kModeLoop ? "on" : "off");
		debug("pitch changes: %s)", header[33] & kModePitch ? "on" : "off");
		debug("    Transpose: %i", (int8)header[34]);
		for (uint i = 0; i < 3; i++)
			debug("    Segment %i: %i words @ offset %i", i, (int16)READ_BE_UINT16(header + 35 + 6 * i), (i == 0 ? 0 : (int32)READ_BE_UINT32(header + 31 + 6 * i)));
		for (uint i = 0; i < 4; i++)
			debug("    Envelope %i: period %i / delta %i / target %i", i, header[49 + i], (int8)header[53 + i], header[57 + i]);
	}

	instrument->samples = (int8 *) malloc(size + 1);
	if (file.read(instrument->samples, size) < (unsigned int)size) {
		warning("Amiga/Mac driver: failed to read instrument samples");
		free(instrument->samples);
		delete instrument;
		return NULL;
	}

	if (instrument->mode & kModePitch)
		instrument->fixedNote = -1;

	if (instrument->mode & kModeLoop) {
		if (loop_offset + seg_size[1] > size) {
			debugC(kDebugLevelSound, "Amiga/Mac driver: looping samples extend %i bytes past end of sample block",
			       loop_offset + seg_size[1] - size);
			seg_size[1] = size - loop_offset;
		}

		if (seg_size[1] < 0) {
			warning("Amiga/Mac driver: invalid looping point");
			free(instrument->samples);
			delete instrument;
			return NULL;
		}

		instrument->size = seg_size[0];
		instrument->loop_size = seg_size[1];

		instrument->loop = (int8 *)malloc(instrument->loop_size + 1);
		memcpy(instrument->loop, instrument->samples + loop_offset, instrument->loop_size);

		instrument->samples[instrument->size] = instrument->loop[0];
		instrument->loop[instrument->loop_size] = instrument->loop[0];
	} else {
		instrument->loop = NULL;
		instrument->loop_size = 0;
		instrument->size = size;
		instrument->samples[instrument->size] = 0;
	}

	return instrument;
}

uint32 MidiDriver_AmigaMac::property(int prop, uint32 param) {
	switch(prop) {
	case MIDI_PROP_MASTER_VOLUME:
		if (param != 0xffff)
			_masterVolume = param;
		return _masterVolume;
	default:
		break;
	}
	return 0;
}

int MidiDriver_AmigaMac::open() {
	_isSci1 = false;
	_isSci1Early = false;

	for (int i = 0; i < 48; i++)
		_freqTable[i] = pow(2, i / (double)48);

	_frequency = _mixer->getOutputRate();
	_envDecay.length = _frequency / (32 * 64);
	_envDecay.delta = 1;
	_envDecay.target = 0;

	for (uint i = 0; i < kChannels; i++) {
		_voices[i].note = -1;
		_voices[i].hw_channel = 0;
	}

	for (uint i = 0; i < MIDI_CHANNELS; i++) {
		_channels[i].instrument = -1;
		_channels[i].volume = 127;
		_channels[i].pan = (i % 4 == 0 || i % 4 == 3 ? kPanLeft : kPanRight);
		_channels[i].pitch = 0x2000;
	}

	Common::File file;

	if (file.open("bank.001")) {
		if (!loadInstrumentsSCI0(file)) {
			file.close();
			return Common::kUnknownError;
		}
		file.close();
	} else {
		ResourceManager *resMan = g_sci->getResMan();

		Resource *resource = resMan->findResource(ResourceId(kResourceTypePatch, 7), false); // Mac
		if (!resource)
			resource = resMan->findResource(ResourceId(kResourceTypePatch, 9), false);       // Amiga

		if (!resource) {
			resource = resMan->findResource(ResourceId(kResourceTypePatch, 5), false);       // KQ1/MUMG Amiga
			if (resource)
				_isSci1Early = true;
		}

		// If we have a patch by this point, it's SCI1
		if (resource)
			_isSci1 = true;

		// Check for the SCI0 Mac patch
		if (!resource)
			resource = resMan->findResource(ResourceId(kResourceTypePatch, 200), false);

		if (!resource) {
			warning("Could not open patch for Amiga sound driver");
			return Common::kUnknownError;
		}

		Common::MemoryReadStream stream(resource->data, resource->size);

		if (_isSci1) {
			if (!loadInstrumentsSCI1(stream))
				return Common::kUnknownError;
		} else if (!loadInstrumentsSCI0Mac(stream))
			return Common::kUnknownError;
	}

	MidiDriver_Emulated::open();

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO);

	return Common::kNoError;
}

void MidiDriver_AmigaMac::close() {
	_mixer->stopHandle(_mixerSoundHandle);

	for (uint i = 0; i < _bank.size; i++) {
		for (uint32 j = 0; j < _bank.instruments[i].size(); j++) {
			if (_bank.instruments[i][j]) {
				if (_bank.instruments[i][j]->loop)
					free(_bank.instruments[i][j]->loop);
				free(_bank.instruments[i][j]->samples);
				delete _bank.instruments[i][j];
			}
		}
	}
}

void MidiDriver_AmigaMac::playSwitch(bool play) {
	_playSwitch = play;
}

void MidiDriver_AmigaMac::setVolume(byte volume_) {
	_masterVolume = volume_;
}

void MidiDriver_AmigaMac::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	switch (command) {
	case 0x80:
		stopNote(channel, op1);
		break;
	case 0x90:
		if (op2 > 0)
			startNote(channel, op1, op2);
		else
			stopNote(channel, op1);
		break;
	case 0xb0:
		switch (op1) {
		case 0x07:
			_channels[channel].volume = op2;
			break;
		case 0x0a:	// pan
			// TODO
			debugC(1, kDebugLevelSound, "Amiga/Mac driver: ignoring pan 0x%02x event for channel %i", op2, channel);
			break;
		case 0x40:	// hold
			// TODO
			debugC(1, kDebugLevelSound, "Amiga/Mac driver: ignoring hold 0x%02x event for channel %i", op2, channel);
			break;
		case 0x4b:	// voice mapping
			break;
		case 0x4e:	// velocity
			break;
		case 0x7b:
			stopChannel(channel);
			break;
		default:
			//warning("Amiga/Mac driver: unknown control event 0x%02x", op1);
			break;
		}
		break;
	case 0xc0:
		changeInstrument(channel, op1);
		break;
	// The original MIDI driver from sierra ignores aftertouch completely, so should we
	case 0xa0: // Polyphonic key pressure (aftertouch)
	case 0xd0: // Channel pressure (aftertouch)
		break;
	case 0xe0:
		pitchWheel(channel, (op2 << 7) | op1);
		break;
	default:
		warning("Amiga/Mac driver: unknown event %02x", command);
	}
}

void MidiDriver_AmigaMac::generateSamples(int16 *data, int len) {
	if (len == 0)
		return;

	int16 *buffers = (int16 *)malloc(len * 2 * kChannels);

	memset(buffers, 0, len * 2 * kChannels);

	/* Generate samples for all notes */
	for (int i = 0; i < kChannels; i++)
		if (_voices[i].note >= 0)
			playInstrument(buffers + i * len, &_voices[i], len);

	if (isStereo()) {
		for (int j = 0; j < len; j++) {
			int mixedl = 0, mixedr = 0;

			/* Mix and pan */
			for (int i = 0; i < kChannels; i++) {
				mixedl += buffers[i * len + j] * (256 - _channels[_voices[i].hw_channel].pan);
				mixedr += buffers[i * len + j] * _channels[_voices[i].hw_channel].pan;
			}

			/* Adjust volume */
			data[2 * j] = mixedl * _masterVolume >> 13;
			data[2 * j + 1] = mixedr * _masterVolume >> 13;
		}
	} else {
		for (int j = 0; j < len; j++) {
			int mixed = 0;

			/* Mix */
			for (int i = 0; i < kChannels; i++)
				mixed += buffers[i * len + j];

			/* Adjust volume */
			data[j] = mixed * _masterVolume >> 6;
		}
	}

	free(buffers);
}

bool MidiDriver_AmigaMac::loadInstrumentsSCI0(Common::File &file) {
	_isSci1 = false;

	byte header[40];

	if (file.read(header, 40) < 40) {
		warning("Amiga/Mac driver: failed to read header of file bank.001");
		return false;
	}

	_bank.size = READ_BE_UINT16(header + 38);
	strncpy(_bank.name, (char *) header + 8, 29);
	_bank.name[29] = 0;
	debugC(kDebugLevelSound, "Amiga/Mac driver: Reading %i instruments from bank \"%s\"", _bank.size, _bank.name);

	for (uint i = 0; i < _bank.size; i++) {
		int id;
		InstrumentSample *instrument = readInstrumentSCI0(file, &id);

		if (!instrument) {
			warning("Amiga/Mac driver: failed to read bank.001");
			return false;
		}

		if (id < 0 || id > 255) {
			warning("Amiga/Mac driver: Error: instrument ID out of bounds");
			return false;
		}

		if ((uint)id >= _bank.instruments.size())
			_bank.instruments.resize(id + 1);

		_bank.instruments[id].push_back(instrument);
		memcpy(_bank.instruments[id].name, instrument->name, sizeof(instrument->name));
	}

	return true;
}

bool MidiDriver_AmigaMac::loadInstrumentsSCI0Mac(Common::SeekableReadStream &file) {
	byte header[40];

	if (file.read(header, 40) < 40) {
		warning("Amiga/Mac driver: failed to read header of file patch.200");
		return false;
	}

	_bank.size = 128;
	strncpy(_bank.name, (char *) header + 8, 29);
	_bank.name[29] = 0;
	debugC(kDebugLevelSound, "Amiga/Mac driver: Reading %i instruments from bank \"%s\"", _bank.size, _bank.name);

	Common::Array<uint32> instrumentOffsets;
	instrumentOffsets.resize(_bank.size);
	_bank.instruments.resize(_bank.size);

	for (uint32 i = 0; i < _bank.size; i++)
		instrumentOffsets[i] = file.readUint32BE();

	for (uint i = 0; i < _bank.size; i++) {
		// 0 signifies it doesn't exist
		if (instrumentOffsets[i] == 0)
			continue;

		file.seek(instrumentOffsets[i]);

		uint16 id = file.readUint16BE();
		if (id != i)
			error("Instrument number mismatch");

		InstrumentSample *instrument = new InstrumentSample;

		instrument->startNote = 0;
		instrument->endNote = 127;
		instrument->isUnsigned = true;
		instrument->baseFreq = kBaseFreq;
		instrument->baseNote = 101;
		instrument->fixedNote = 101;
		instrument->mode = file.readUint16BE();

		// Read in the offsets
		int32 seg_size[3];
		seg_size[0] = file.readUint32BE();
		seg_size[1] = file.readUint32BE();
		seg_size[2] = file.readUint32BE();

		instrument->transpose = file.readUint16BE();

		for (byte j = 0; j < 4; j++) {
			int length = (int8)file.readByte();

			if (length == 0 && j > 0)
				length = 256;

			instrument->envelope[j].length = length * _frequency / 60;
			instrument->envelope[j].delta = (int8)file.readByte();
			instrument->envelope[j].target = file.readByte();
		}

		// Final target must be 0
		instrument->envelope[3].target = 0;

		file.read(instrument->name, 30);

		if (instrument->mode & kModePitch)
			instrument->fixedNote = -1;

		uint32 size = seg_size[2];
		uint32 loop_offset = seg_size[0];

		instrument->samples = (int8 *)malloc(size + 1);
		if (file.read(instrument->samples, size) < size) {
			warning("Amiga/Mac driver: failed to read instrument sample");
			free(instrument->samples);
			delete instrument;
			continue;
		}

		if (instrument->mode & kModeLoop) {
			instrument->size = seg_size[0];
			instrument->loop_size = seg_size[1] - seg_size[0];

			instrument->loop = (int8 *)malloc(instrument->loop_size + 1);
			memcpy(instrument->loop, instrument->samples + loop_offset, instrument->loop_size);

			instrument->samples[instrument->size] = instrument->loop[0];
			instrument->loop[instrument->loop_size] = instrument->loop[0];
		} else {
			instrument->loop = NULL;
			instrument->loop_size = 0;
			instrument->size = size;
			instrument->samples[instrument->size] = (int8)0x80;
		}

		_bank.instruments[id].push_back(instrument);
		memcpy(_bank.instruments[id].name, instrument->name, sizeof(instrument->name));
	}

	return true;
}

bool MidiDriver_AmigaMac::loadInstrumentsSCI1(Common::SeekableReadStream &file) {
	_bank.size = 128;

	if (_isSci1Early)
		file.readUint32BE(); // Skip size of bank

	Common::Array<uint32> instrumentOffsets;
	instrumentOffsets.resize(_bank.size);
	_bank.instruments.resize(_bank.size);

	for (uint32 i = 0; i < _bank.size; i++)
		instrumentOffsets[i] = file.readUint32BE();

	for (uint32 i = 0; i < _bank.size; i++) {
		// 0 signifies it doesn't exist
		if (instrumentOffsets[i] == 0)
			continue;

		file.seek(instrumentOffsets[i] + (_isSci1Early ? 4 : 0));

		// Read in the instrument name
		file.read(_bank.instruments[i].name, 10); // last two bytes are always 0

		for (uint32 j = 0; ; j++) {
			InstrumentSample *sample = new InstrumentSample;
			memset(sample, 0, sizeof(InstrumentSample));

			sample->startNote = file.readSint16BE();

			// startNote being -1 signifies we're done with this instrument
			if (sample->startNote == -1) {
				delete sample;
				break;
			}

			sample->endNote = file.readSint16BE();
			uint32 samplePtr = file.readUint32BE();
			sample->transpose = file.readSint16BE();
			for (int env = 0; env < 3; env++) {
				sample->envelope[env].length = file.readByte() * _frequency / 60;
				sample->envelope[env].delta = (env == 0 ? 10 : -10);
				sample->envelope[env].target = file.readByte();
			}

			sample->envelope[3].length = 0;
			sample->fixedNote = file.readSint16BE();
			int16 loop = file.readSint16BE();
			uint32 nextSamplePos = file.pos();

			file.seek(samplePtr + (_isSci1Early ? 4 : 0));
			file.read(sample->name, 8);

			uint16 phase1Offset, phase1End;
			uint16 phase2Offset, phase2End;

			if (_isSci1Early) {
				sample->isUnsigned = false;
				file.readUint32BE(); // skip total sample size
				phase2Offset = file.readUint16BE();
				phase2End = file.readUint16BE();
				sample->baseNote = file.readUint16BE();
				phase1Offset = file.readUint16BE();
				phase1End = file.readUint16BE();
			} else {
				sample->isUnsigned = file.readUint16BE() == 0;
				phase1Offset = file.readUint16BE();
				phase1End = file.readUint16BE();
				phase2Offset = file.readUint16BE();
				phase2End = file.readUint16BE();
				sample->baseNote = file.readUint16BE();
			}

			uint32 periodTableOffset = _isSci1Early ? 0 : file.readUint32BE();
			uint32 sampleDataPos = file.pos();

			sample->size = phase1End - phase1Offset + 1;
			sample->loop_size = phase2End - phase2Offset + 1;

			sample->samples = (int8 *)malloc(sample->size + 1);
			file.seek(phase1Offset + sampleDataPos);
			file.read(sample->samples, sample->size);
			sample->samples[sample->size] = (sample->isUnsigned ? (int8)0x80 : 0);

			if (loop == 0 && sample->loop_size > 1) {
				sample->loop = (int8 *)malloc(sample->loop_size + 1);
				file.seek(phase2Offset + sampleDataPos);
				file.read(sample->loop, sample->loop_size);
				sample->mode |= kModeLoop;
				sample->samples[sample->size] = sample->loop[0];
				sample->loop[sample->loop_size] = sample->loop[0];
			}

			_bank.instruments[i].push_back(sample);

			if (_isSci1Early) {
				// There's no frequency specified by the sample and is hardcoded like in SCI0
				sample->baseFreq = 11000;
			} else {
				file.seek(periodTableOffset + 0xe0);
				sample->baseFreq = file.readUint16BE();
			}

			file.seek(nextSamplePos);
		}
	}

	return true;
}

class MidiPlayer_AmigaMac : public MidiPlayer {
public:
	MidiPlayer_AmigaMac(SciVersion version) : MidiPlayer(version) { _driver = new MidiDriver_AmigaMac(g_system->getMixer()); }
	byte getPlayId() const;
	int getPolyphony() const { return MidiDriver_AmigaMac::kVoices; }
	bool hasRhythmChannel() const { return false; }
	void setVolume(byte volume) { static_cast<MidiDriver_AmigaMac *>(_driver)->setVolume(volume); }
	void playSwitch(bool play) { static_cast<MidiDriver_AmigaMac *>(_driver)->playSwitch(play); }
	void loadInstrument(int idx, byte *data);
};

MidiPlayer *MidiPlayer_AmigaMac_create(SciVersion version) {
	return new MidiPlayer_AmigaMac(version);
}

byte MidiPlayer_AmigaMac::getPlayId() const {
	if (_version > SCI_VERSION_0_LATE)
		return 0x06;

	return 0x40;
}

} // End of namespace Sci
