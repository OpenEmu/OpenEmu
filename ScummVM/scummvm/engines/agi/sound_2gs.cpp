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

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/archive.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/str-array.h"
#include "common/textconsole.h"

#include "agi/agi.h"
#include "agi/sound_2gs.h"

namespace Agi {

SoundGen2GS::SoundGen2GS(AgiBase *vm, Audio::Mixer *pMixer) : SoundGen(vm, pMixer) {
	// Allocate memory for the wavetable
	_wavetable = new int8[SIERRASTANDARD_SIZE];

	// Apple IIGS AGI MIDI player advances 60 ticks per second. Strategy
	// here is to first generate audio for a 1/60th of a second and then
	// advance the MIDI player by one tick. Thus, make the output buffer
	// to be a 1/60th of a second in length.
	_outSize = _sampleRate / 60;
	_out = new int16[2 * _outSize]; // stereo

	// Initialize player variables
	_nextGen = 0;
	_ticks = 0;

	// Not playing anything yet
	_playingSound = -1;
	_playing = false;

	// Load instruments
	_disableMidi = !loadInstruments();

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

SoundGen2GS::~SoundGen2GS() {
	_mixer->stopHandle(_soundHandle);
	delete[] _wavetable;
	delete[] _out;
}

int SoundGen2GS::readBuffer(int16 *buffer, const int numSamples) {
	static uint data_available = 0;
	static uint data_offset = 0;
	uint n = numSamples << 1;
	uint8 *p = (uint8 *)buffer;

	while (n > data_available) {
		memcpy(p, (uint8 *)_out + data_offset, data_available);
		p += data_available;
		n -= data_available;

		advancePlayer();

		data_available = generateOutput() << 1;
		data_offset = 0;
	}

	memcpy(p, (uint8 *)_out + data_offset, n);
	data_offset += n;
	data_available -= n;

	return numSamples;
}

/**
 * Initiate the playing of a sound resource.
 * @param resnum Resource number
 */
void SoundGen2GS::play(int resnum) {
	AgiSoundEmuType type;

	_playingSound = resnum;

	type = (AgiSoundEmuType)_vm->_game.sounds[resnum]->type();
	assert (type == AGI_SOUND_SAMPLE || type == AGI_SOUND_MIDI);

	if (_vm->_soundemu != SOUND_EMU_APPLE2GS) {
		warning("Trying to play sample or MIDI resource but not using Apple IIGS sound emulation mode");
		return;
	}

	haltGenerators();

	switch (type) {
	case AGI_SOUND_SAMPLE: {
		IIgsSample *sampleRes = (IIgsSample *) _vm->_game.sounds[_playingSound];
		const IIgsSampleHeader &header = sampleRes->getHeader();
		_channels[kSfxMidiChannel].setInstrument(&header.instrument);
		_channels[kSfxMidiChannel].setVolume(header.volume);
		midiNoteOn(kSfxMidiChannel, header.pitch, 127);
		break;
	}
	case AGI_SOUND_MIDI:
		((IIgsMidi *) _vm->_game.sounds[_playingSound])->rewind();
		_ticks = 0;
		break;
	default:
		break;
	}
}

void SoundGen2GS::stop() {
	haltGenerators();
	_playingSound = -1;
	_playing = 0;
}

/**
 * Fill output buffer by advancing the generators for a 1/60th of a second.
 * @return Number of generated samples
 */
uint SoundGen2GS::generateOutput() {
	memset(_out, 0, _outSize * 2 * 2);

	if (!_playing || _playingSound == -1)
		return _outSize * 2;

	int16 *p = _out;
	int n = _outSize;
	while (n--) {
		int outl = 0;
		int outr = 0;
		for (int k = 0; k < MAX_GENERATORS; k++) {
			IIgsGenerator *g = &_generators[k];
			if (!g->ins)
				continue;
			const IIgsInstrumentHeader *i = g->ins;

			// Advance envelope
			int vol = fracToInt(g->a);
			if (g->a <= i->env[g->seg].bp) {
				g->a += i->env[g->seg].inc * ENVELOPE_COEF;
				if (g->a > i->env[g->seg].bp) {
					g->a = i->env[g->seg].bp;
					g->seg++;
				}
			} else {
				g->a -= i->env[g->seg].inc * ENVELOPE_COEF;
				if (g->a < i->env[g->seg].bp) {
					g->a = i->env[g->seg].bp;
					g->seg++;
				}
			}

			// TODO: Advance vibrato here. The Apple IIGS uses a LFO with
			// triangle wave to modulate the frequency of both oscillators.
			// In Apple IIGS the vibrato and the envelope are updated at the
			// same time, so the vibrato speed depends on ENVELOPE_COEF.

			// Advance oscillators
			int s0 = 0;
			int s1 = 0;
			if (!g->osc[0].halt) {
				s0 = g->osc[0].base[fracToInt(g->osc[0].p)];
				g->osc[0].p += g->osc[0].pd;
				if ((uint)fracToInt(g->osc[0].p) >= g->osc[0].size) {
					g->osc[0].p -= intToFrac(g->osc[0].size);
					if (!g->osc[0].loop)
						g->osc[0].halt = 1;
					if (g->osc[0].swap) {
						g->osc[0].halt = 1;
						g->osc[1].halt = 0;
					}
				}
			}
			if (!g->osc[1].halt) {
				s1 = g->osc[1].base[fracToInt(g->osc[1].p)];
				g->osc[1].p += g->osc[1].pd;
				if ((uint)fracToInt(g->osc[1].p) >= g->osc[1].size) {
					g->osc[1].p -= intToFrac(g->osc[1].size);
					if (!g->osc[1].loop)
						g->osc[1].halt = 1;
					if (g->osc[1].swap) {
						g->osc[0].halt = 0;
						g->osc[1].halt = 1;
					}
				}
			}

			// Take envelope and MIDI volume information into account.
			// Also amplify.
			s0 *= vol * g->vel / 127 * 80 / 256;
			s1 *= vol * g->vel / 127 * 80 / 256;

			// Select output channel.
			if (g->osc[0].chn)
				outl += s0;
			else
				outr += s0;

			if (g->osc[1].chn)
				outl += s1;
			else
				outr += s1;
		}

		if (outl > 32768)
			outl = 32768;
		if (outl < -32767)
			outl = -32767;
		if (outr > 32768)
			outr = 32768;
		if (outr < -32767)
			outr = -32767;

		*p++ = outl;
		*p++ = outr;
	}

	return _outSize * 2;
}

void SoundGen2GS::advancePlayer() {
	if (_playingSound == -1)
		return;

	if (_vm->_game.sounds[_playingSound]->type() == AGI_SOUND_MIDI) {
		advanceMidiPlayer();
	} else if (_vm->_game.sounds[_playingSound]->type() == AGI_SOUND_SAMPLE) {
		_playing = activeGenerators() > 0;
	}

	if (!_playing) {
		_vm->_sound->soundIsFinished();
		_playingSound = -1;
	}
}

void SoundGen2GS::advanceMidiPlayer() {
	if (_disableMidi)
		return;

	const uint8 *p;
	uint8 parm1, parm2;
	static uint8 cmd, chn;

	if (_playingSound == -1 || _vm->_game.sounds[_playingSound] == NULL) {
		warning("Error playing Apple IIGS MIDI sound resource");
		_playing = false;
		return;
	}

	IIgsMidi *midiObj = (IIgsMidi *) _vm->_game.sounds[_playingSound];

	_ticks++;
	_playing = true;
	p = midiObj->getPtr();

	while (true) {
		// Check for end of MIDI sequence marker (Can also be here before delta-time)
		if (*p == MIDI_STOP_SEQUENCE) {
			debugC(3, kDebugLevelSound, "End of MIDI sequence (Before reading delta-time)");
			_playing = false;
			midiObj->rewind();
			return;
		}
		if (*p == MIDI_TIMER_SYNC) {
			debugC(3, kDebugLevelSound, "Timer sync");
			p++; // Jump over the timer sync byte as it's not needed
			continue;
		}

		// Check for delta time
		uint8 delta = *p;
		if (midiObj->_ticks + delta > _ticks)
			break;
		midiObj->_ticks += delta;
		p++;

		// Check for end of MIDI sequence marker (This time it after reading delta-time)
		if (*p == MIDI_STOP_SEQUENCE) {
			debugC(3, kDebugLevelSound, "End of MIDI sequence (After reading delta-time)");
			_playing = false;
			midiObj->rewind();
			return;
		}

		// Separate byte into command and channel if it's a command byte.
		// Otherwise use running status (i.e. previously set command and channel).
		if (*p & 0x80) {
			cmd = *p++;
			chn = cmd & 0x0f;
			cmd >>= 4;
		}

		switch (cmd) {
		case MIDI_NOTE_OFF:
			parm1 = *p++;
			parm2 = *p++;
			debugC(3, kDebugLevelSound, "channel %X: note off (key = %d, velocity = %d)", chn, parm1, parm2);
			midiNoteOff(chn, parm1, parm2);
			break;
		case MIDI_NOTE_ON:
			parm1 = *p++;
			parm2 = *p++;
			debugC(3, kDebugLevelSound, "channel %X: note on (key = %d, velocity = %d)", chn, parm1, parm2);
			midiNoteOn(chn, parm1, parm2);
			break;
		case MIDI_CONTROLLER:
			parm1 = *p++;
			parm2 = *p++;
			debugC(3, kDebugLevelSound, "channel %X: controller %02X = %02X", chn, parm1, parm2);
			// The tested Apple IIGS AGI MIDI resources only used
			// controllers 0 (Bank select?), 7 (Volume) and 64 (Sustain On/Off).
			// Controller 0's parameter was in range 94-127,
			// controller 7's parameter was in range 0-127 and
			// controller 64's parameter was always 0 (i.e. sustain off).
			switch (parm1) {
			case 7:
				_channels[chn].setVolume(parm2);
				break;
			}
			break;
		case MIDI_PROGRAM_CHANGE:
			parm1 = *p++;
			debugC(3, kDebugLevelSound, "channel %X: program change %02X", chn, parm1);
			_channels[chn].setInstrument(getInstrument(parm1));
			break;
		case MIDI_PITCH_WHEEL:
			parm1 = *p++;
			parm2 = *p++;
			debugC(3, kDebugLevelSound, "channel %X: pitch wheel (unimplemented)", chn);
			break;

		default:
			debugC(3, kDebugLevelSound, "channel %X: unimplemented command %02X", chn, cmd);
			break;
		}
	}

	midiObj->setPtr(p);
}

void SoundGen2GS::midiNoteOff(int channel, int note, int velocity) {
	// Release keys within the given MIDI channel
	for (int i = 0; i < MAX_GENERATORS; i++) {
		if (_generators[i].chn == channel && _generators[i].key == note)
			_generators[i].seg = _generators[i].ins->seg;
	}
}

void SoundGen2GS::midiNoteOn(int channel, int note, int velocity) {
	if (!_channels[channel].getInstrument()) {
		debugC(3, kDebugLevelSound, "midiNoteOn(): no instrument specified for channel %d", channel);
		return;
	}

	// Allocate a generator for the note.
	IIgsGenerator* g = allocateGenerator();
	g->ins = _channels[channel].getInstrument();
	const IIgsInstrumentHeader* i = g->ins;

	// Pass information from the MIDI channel to the generator. Take
	// velocity into account, although simplistically.
	velocity *= 5 / 3;
	if (velocity > 127)
		velocity = 127;

	g->key = note;
	g->vel = velocity * _channels[channel].getVolume() / 127;
	g->chn = channel;

	// Instruments can define different samples to be used based on
	// what the key is. Find the correct samples for our key.
	int wa = 0;
	int wb = 0;
	while (wa < i->waveCount[0] - 1 && note > i->wave[0][wa].key)
		wa++;
	while (wb < i->waveCount[1] - 1 && note > i->wave[1][wb].key)
		wb++;

	// Prepare the generator.
	g->osc[0].base	= i->base + i->wave[0][wa].offset;
	g->osc[0].size	= i->wave[0][wa].size;
	g->osc[0].pd	= doubleToFrac(midiKeyToFreq(note, (double)i->wave[0][wa].tune / 256.0) / (double)_sampleRate);
	g->osc[0].p		= 0;
	g->osc[0].halt	= i->wave[0][wa].halt;
	g->osc[0].loop	= i->wave[0][wa].loop;
	g->osc[0].swap	= i->wave[0][wa].swap;
	g->osc[0].chn	= i->wave[0][wa].chn;

	g->osc[1].base	= i->base + i->wave[1][wb].offset;
	g->osc[1].size	= i->wave[1][wb].size;
	g->osc[1].pd	= doubleToFrac(midiKeyToFreq(note, (double)i->wave[1][wb].tune / 256.0) / (double)_sampleRate);
	g->osc[1].p		= 0;
	g->osc[1].halt	= i->wave[1][wb].halt;
	g->osc[1].loop	= i->wave[1][wb].loop;
	g->osc[1].swap	= i->wave[1][wb].swap;
	g->osc[1].chn	= i->wave[1][wb].chn;

	g->seg	= 0;
	g->a	= 0;

	// Print debug messages for instruments with swap mode or vibrato enabled
	if (g->osc[0].swap || g->osc[1].swap)
		debugC(2, kDebugLevelSound, "Detected swap mode in a playing instrument. This is rare and is not tested well...");
	if (i->vibDepth > 0)
		debugC(2, kDebugLevelSound, "Detected vibrato in a playing instrument. Vibrato is not implemented, playing without...");
}

double SoundGen2GS::midiKeyToFreq(int key, double finetune) {
	return 440.0 * pow(2.0, (15.0 + (double)key + finetune) / 12.0);
}

void SoundGen2GS::haltGenerators() {
	for (int i = 0; i < MAX_GENERATORS; i++) {
		_generators[i].osc[0].halt = true;
		_generators[i].osc[1].halt = true;
	}
}

uint SoundGen2GS::activeGenerators() {
	int n = 0;
	for (int i = 0; i < MAX_GENERATORS; i++)
		if (!_generators[i].osc[0].halt || !_generators[i].osc[1].halt)
			n++;
	return n;
}

void SoundGen2GS::setProgramChangeMapping(const IIgsMidiProgramMapping *mapping) {
	_progToInst = mapping;
}

IIgsMidi::IIgsMidi(uint8 *data, uint32 len, int resnum, SoundMgr &manager) : AgiSound(manager) {
	_data = data; // Save the resource pointer
	_ptr = _data + 2; // Set current position to just after the header
	_len = len;  // Save the resource's length
	_type = READ_LE_UINT16(data); // Read sound resource's type
	_ticks = 0;
	_isValid = (_type == AGI_SOUND_MIDI) && (_data != NULL) && (_len >= 2);

	if (!_isValid) // Check for errors
		warning("Error creating Apple IIGS midi sound from resource %d (Type %d, length %d)", resnum, _type, len);
}

/**
 * Convert sample from 8-bit unsigned to 8-bit signed format.
 * @param source  Source stream containing the 8-bit unsigned sample data.
 * @param dest  Destination buffer for the 8-bit signed sample data.
 * @param length  Length of the sample data to be converted.
 */
static bool convertWave(Common::SeekableReadStream &source, int8 *dest, uint length) {
	// Convert the wave from 8-bit unsigned to 8-bit signed format
	for (uint i = 0; i < length; i++)
		dest[i] = (int8) ((int) source.readByte() - ZERO_OFFSET);
	return !(source.eos() || source.err());
}

IIgsSample::IIgsSample(uint8 *data, uint32 len, int resnum, SoundMgr &manager) : AgiSound(manager) {
	Common::MemoryReadStream stream(data, len, DisposeAfterUse::YES);

	// Check that the header was read ok and that it's of the correct type
	if (_header.read(stream) && _header.type == AGI_SOUND_SAMPLE) { // An Apple IIGS AGI sample resource
		uint32 sampleStartPos = stream.pos();
		uint32 tailLen = stream.size() - sampleStartPos;

		if (tailLen < _header.sampleSize) { // Check if there's no room for the sample data in the stream
			// Apple IIGS Manhunter I: Sound resource 16 has only 16074 bytes
			// of sample data although header says it should have 16384 bytes.
			warning("Apple IIGS sample (%d) too short (%d bytes. Should be %d bytes). Using the part that's left",
				resnum, tailLen, _header.sampleSize);

			_header.sampleSize = (uint16) tailLen; // Use the part that's left
		}

		if (_header.pitch > 0x7F) { // Check if the pitch is invalid
			warning("Apple IIGS sample (%d) has too high pitch (0x%02x)", resnum, _header.pitch);

			_header.pitch &= 0x7F; // Apple IIGS AGI probably did it this way too
		}

		// Convert sample data from 8-bit unsigned to 8-bit signed format
		stream.seek(sampleStartPos);
		_sample = new int8[_header.sampleSize];

		if (_sample != NULL) {
			_isValid = convertWave(stream, _sample, _header.sampleSize);
			// Finalize header info using sample data
			_header.finalize(_sample);
		}
	}

	if (!_isValid) // Check for errors
		warning("Error creating Apple IIGS sample from resource %d (Type %d, length %d)", resnum, _header.type, len);
}


bool IIgsInstrumentHeader::read(Common::SeekableReadStream &stream, bool ignoreAddr) {
	for (int i = 0; i < ENVELOPE_SEGMENT_COUNT; i++) {
		env[i].bp = intToFrac(stream.readByte());
		env[i].inc = intToFrac(stream.readUint16LE()) >> 8;
	}
	seg			= stream.readByte();
	/*priority	=*/ stream.readByte(); // Not needed. 32 in all tested data.
	bend		= stream.readByte();
	vibDepth	= stream.readByte();
	vibSpeed	= stream.readByte();
	stream.readByte(); // Not needed? 0 in all tested data.

	waveCount[0] = stream.readByte();
	waveCount[1] = stream.readByte();
	for (int i = 0; i < 2; i++)
	for (int k = 0; k < waveCount[i]; k++) {
		wave[i][k].key = stream.readByte();
		wave[i][k].offset = stream.readByte() << 8;
		wave[i][k].size = 0x100 << (stream.readByte() & 7);
		uint8 b = stream.readByte();
		wave[i][k].tune = stream.readUint16LE();

		// For sample resources we ignore the address.
		if (ignoreAddr)
			wave[i][k].offset = 0;

		// Check for samples that extend out of the wavetable.
		if (wave[i][k].offset + wave[i][k].size >= SIERRASTANDARD_SIZE) {
			warning("Invalid data detected in the instrument set of Apple IIGS AGI. Continuing anyway...");
			wave[i][k].size = SIERRASTANDARD_SIZE - wave[i][k].offset;
		}

		// Parse the generator mode byte to separate fields.
		wave[i][k].halt = b & 0x1;			// Bit 0     = HALT
		wave[i][k].loop = !(b & 0x2);		// Bit 1     =!LOOP
		wave[i][k].swap = (b & 0x6) == 0x6;	// Bit 1&2   = SWAP
		wave[k][k].chn = (b >> 4) > 0;		// Output channel (left or right)
	}

	return !(stream.eos() || stream.err());
}

bool IIgsInstrumentHeader::finalize(int8 *wavetable) {
	// Calculate final pointers to sample data and detect true sample size
	// in case the sample ends prematurely.
	for (int i = 0; i < 2; i++)
	for (int k = 0; k < waveCount[i]; k++) {
		base = wavetable;
		int8 *p = base + wave[i][k].offset;
		uint trueSize;
		for (trueSize = 0; trueSize < wave[i][k].size; trueSize++)
			if (p[trueSize] == -ZERO_OFFSET)
				break;
		wave[i][k].size = trueSize;
	}

	return true;
}

bool IIgsSampleHeader::read(Common::SeekableReadStream &stream) {
	type				= stream.readUint16LE();
	pitch				= stream.readByte();
	unknownByte_Ofs3	= stream.readByte();
	volume				= stream.readByte();
	unknownByte_Ofs5	= stream.readByte();
	instrumentSize		= stream.readUint16LE();
	sampleSize			= stream.readUint16LE();
	// Read the instrument header *ignoring* its wave address info
	return instrument.read(stream, true);
}

bool IIgsSampleHeader::finalize(int8 *sample) {
	return instrument.finalize(sample);
}

//###
//### LOADER METHODS
//###

bool SoundGen2GS::loadInstruments() {
	// Get info on the particular Apple IIGS AGI game's executable
	const IIgsExeInfo *exeInfo = getIIgsExeInfo((enum AgiGameID)_vm->getGameID());
	if (exeInfo == NULL) {
		warning("Unsupported Apple IIGS game, not loading instruments");
		return false;
	}

	// Find the executable file and the wavetable file
	Common::ArchiveMemberList exeNames, waveNames;
	SearchMan.listMatchingMembers(exeNames, "*.SYS16");
	SearchMan.listMatchingMembers(exeNames, "*.SYS");
	SearchMan.listMatchingMembers(waveNames, "SIERRASTANDARD");
	SearchMan.listMatchingMembers(waveNames, "SIERRAST");

	if (exeNames.empty()) {
		warning("Couldn't find Apple IIGS game executable (*.SYS16 or *.SYS), not loading instruments");
		return false;
	}
	if (waveNames.empty()) {
		warning("Couldn't find Apple IIGS wave file (SIERRASTANDARD or SIERRAST), not loading instruments");
		return false;
	}

	Common::String exeName  = exeNames.front()->getName();
	Common::String waveName = waveNames.front()->getName();

	// Set the MIDI program change to instrument number mapping and
	// load the instrument headers and their sample data.
	setProgramChangeMapping(exeInfo->instSet->progToInst);
	return loadWaveFile(waveName, *exeInfo) && loadInstrumentHeaders(exeName, *exeInfo);
}

/** Older Apple IIGS AGI MIDI program change to instrument number mapping. */
static const IIgsMidiProgramMapping progToInstMappingV1 = {
	{19, 20, 22, 23, 21, 24, 5, 5, 5, 5,
	6, 7, 10, 9, 11, 9, 15, 8, 5, 5,
	17, 16, 18, 12, 14, 5, 5, 5, 5, 5,
	0, 1, 2, 9, 3, 4, 15, 2, 2, 2,
	25, 13, 13, 25},
	5
};

/** Newer Apple IIGS AGI MIDI program change to instrument number mapping.
    FIXME: Some instrument choices sound wrong. */
static const IIgsMidiProgramMapping progToInstMappingV2 = {
	{21, 22, 24, 25, 23, 26, 6, 6, 6, 6,
	7, 9, 12, 8, 13, 11, 17, 10, 6, 6,
	19, 18, 20, 14, 16, 6, 6, 6, 6, 6,
	0, 1, 2, 4, 3, 5, 17, 2, 2, 2,
	27, 15, 15, 27},
	6
};

// Older Apple IIGS AGI instrument set. Used only by Space Quest I (AGI v1.002).
//
// Instrument 0 uses vibrato.
// Instrument 1 uses vibrato.
// Instrument 3 uses vibrato.
// Instrument 5 has swap mode enabled for the first oscillator.
// Instruemnt 9 uses vibrato.
// Instrument 10 uses vibrato.
// Instrument 12 uses vibrato.
// Instrument 15 uses vibrato.
// Instrument 16 uses vibrato.
// Instrument 18 uses vibrato.
static const IIgsInstrumentSetInfo instSetV1 = {
	1192, 26, "7ee16bbc135171ffd6b9120cc7ff1af2", "edd3bf8905d9c238e02832b732fb2e18", &progToInstMappingV1
};

// Newer Apple IIGS AGI instrument set (AGI v1.003+). Used by all others than Space Quest I.
//
// Instrument 0 uses vibrato.
// Instrument 1 uses vibrato.
// Instrument 3 uses vibrato.
// Instrument 6 has swap mode enabled for the first oscillator.
// Instrument 11 uses vibrato.
// Instrument 12 uses vibrato.
// Instrument 14 uses vibrato.
// Instrument 17 uses vibrato.
// Instrument 18 uses vibrato.
// Instrument 20 uses vibrato.
//
// In KQ1 intro and in LSL intro one (and the same, or at least similar)
// instrument is using vibrato. In PQ intro there is also one instrument
// using vibrato.
static const IIgsInstrumentSetInfo instSetV2 = {
	1292, 28, "b7d428955bb90721996de1cbca25e768", "c05fb0b0e11deefab58bc68fbd2a3d07", &progToInstMappingV2
};

/** Information about different Apple IIGS AGI executables. */
static const IIgsExeInfo IIgsExeInfos[] = {
	{GID_SQ1,      "SQ",   0x1002, 138496, 0x80AD, &instSetV1},
	{GID_LSL1,     "LL",   0x1003, 141003, 0x844E, &instSetV2},
	{GID_AGIDEMO,  "DEMO", 0x1005, 141884, 0x8469, &instSetV2},
	{GID_KQ1,      "KQ",   0x1006, 141894, 0x8469, &instSetV2},
	{GID_PQ1,      "PQ",   0x1007, 141882, 0x8469, &instSetV2},
	{GID_MIXEDUP,  "MG",   0x1013, 142552, 0x84B7, &instSetV2},
	{GID_KQ2,      "KQ2",  0x1013, 143775, 0x84B7, &instSetV2},
	{GID_KQ3,      "KQ3",  0x1014, 144312, 0x84B7, &instSetV2},
	{GID_SQ2,      "SQ2",  0x1014, 107882, 0x6563, &instSetV2},
	{GID_MH1,      "MH",   0x2004, 147678, 0x8979, &instSetV2},
	{GID_KQ4,      "KQ4",  0x2006, 147652, 0x8979, &instSetV2},
	{GID_BC,       "BC",   0x3001, 148192, 0x8979, &instSetV2},
	{GID_GOLDRUSH, "GR",   0x3003, 148268, 0x8979, &instSetV2}
};

/**
 * Finds information about an Apple IIGS AGI executable based on the game ID.
 * @return A non-null IIgsExeInfo pointer if successful, otherwise NULL.
 */
const IIgsExeInfo *SoundGen2GS::getIIgsExeInfo(enum AgiGameID gameid) const {
	for (int i = 0; i < ARRAYSIZE(IIgsExeInfos); i++)
		if (IIgsExeInfos[i].gameid == gameid)
			return &IIgsExeInfos[i];
	return NULL;
}

bool SoundGen2GS::loadInstrumentHeaders(Common::String &exePath, const IIgsExeInfo &exeInfo) {
	Common::File file;

	// Open the executable file and check that it has correct size
	file.open(exePath);
	if (file.size() != (int32)exeInfo.exeSize) {
		debugC(3, kDebugLevelSound, "Apple IIGS executable (%s) has wrong size (Is %d, should be %d)",
			exePath.c_str(), file.size(), exeInfo.exeSize);
	}

	// Read the whole executable file into memory
	// CHECKME: Why do we read the file into memory first? It does not seem to be
	// kept outside of this function. Is the processing of the data too slow
	// otherwise?
	Common::ScopedPtr<Common::SeekableReadStream> data(file.readStream(file.size()));
	file.close();

	// Check that we got enough data to be able to parse the instruments
	if (!data || data->size() < (int32)(exeInfo.instSetStart + exeInfo.instSet->byteCount)) {
		warning("Error loading instruments from Apple IIGS executable (%s)", exePath.c_str());
		return false;
	}

	// Check instrument set's length (The info's saved in the executable)
	data->seek(exeInfo.instSetStart - 4);
	uint16 instSetByteCount = data->readUint16LE();
	if (instSetByteCount != exeInfo.instSet->byteCount) {
		debugC(3, kDebugLevelSound, "Wrong instrument set size (Is %d, should be %d) in Apple IIGS executable (%s)",
			instSetByteCount, exeInfo.instSet->byteCount, exePath.c_str());
	}

	// Check instrument set's md5sum
	data->seek(exeInfo.instSetStart);
	Common::String md5str = Common::computeStreamMD5AsString(*data, exeInfo.instSet->byteCount);
	if (md5str != exeInfo.instSet->md5) {
		warning("Unknown Apple IIGS instrument set (md5: %s) in %s, trying to use it nonetheless",
			md5str.c_str(), exePath.c_str());
	}

	// Read in the instrument set one instrument at a time
	data->seek(exeInfo.instSetStart);

	_instruments.clear();
	_instruments.reserve(exeInfo.instSet->instCount);

	IIgsInstrumentHeader instrument;
	for (uint i = 0; i < exeInfo.instSet->instCount; i++) {
		if (!instrument.read(*data)) {
			warning("Error loading Apple IIGS instrument (%d. of %d) from %s, not loading more instruments",
					i + 1, exeInfo.instSet->instCount, exePath.c_str());
			break;
		}
		instrument.finalize(_wavetable);
		_instruments.push_back(instrument);
	}

	// Loading was successful only if all instruments were loaded successfully
	return (_instruments.size() == exeInfo.instSet->instCount);
}

bool SoundGen2GS::loadWaveFile(Common::String &wavePath, const IIgsExeInfo &exeInfo) {
	Common::File file;

	// Open the wave file and read it into memory
	// CHECKME: Why do we read the file into memory first? It does not seem to be
	// kept outside of this function. Is the processing of the data too slow
	// otherwise?
	file.open(wavePath);
	Common::ScopedPtr<Common::SeekableReadStream> uint8Wave(file.readStream(file.size()));
	file.close();

	// Check that we got the whole wave file
	if (!uint8Wave || (uint8Wave->size() != SIERRASTANDARD_SIZE)) {
		warning("Error loading Apple IIGS wave file (%s), not loading instruments", wavePath.c_str());
		return false;
	}

	// Check wave file's md5sum
	Common::String md5str = Common::computeStreamMD5AsString(*uint8Wave, SIERRASTANDARD_SIZE);
	if (md5str != exeInfo.instSet->waveFileMd5) {
		warning("Unknown Apple IIGS wave file (md5: %s, game: %s).\n" \
				"Please report the information on the previous line to the ScummVM team.\n" \
				"Using the wave file as it is - music may sound weird", md5str.c_str(), exeInfo.exePrefix);
	}

	// Convert the wave file to 8-bit signed and save the result
	uint8Wave->seek(0);
	return convertWave(*uint8Wave, _wavetable, SIERRASTANDARD_SIZE);
}

} // End of namespace Agi
