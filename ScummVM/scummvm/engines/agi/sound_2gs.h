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

#ifndef AGI_SOUND_2GS_H
#define AGI_SOUND_2GS_H

#include "common/frac.h"
#include "audio/audiostream.h"

namespace Agi {

// Sample data in SIERRASTANDARD files is in unsigned 8-bit format. A zero
// occurring in the sample data causes the ES5503 wavetable sound chip in
// Apple IIGS to halt the corresponding oscillator immediately. We preprocess
// the sample data by converting it to signed values and the instruments by
// detecting prematurely stopping samples beforehand.
//
// Note: None of the tested SIERRASTANDARD files have zeroes in them. So in
// practice there is no need to check for them. However, they still do exist
// in the sample resources.
#define ZERO_OFFSET 0x80

// Apple IIGS envelope update frequency defaults to 100Hz. It can be changed,
// so there might be differences per game, for example.
#define ENVELOPE_COEF 100 / _sampleRate

// MIDI player commands
#define MIDI_NOTE_OFF		0x8
#define MIDI_NOTE_ON		0x9
#define MIDI_CONTROLLER		0xB
#define MIDI_PROGRAM_CHANGE	0xC
#define MIDI_PITCH_WHEEL	0xE

#define MIDI_STOP_SEQUENCE	0xFC
#define MIDI_TIMER_SYNC		0xF8

// Size of the SIERRASTANDARD file (i.e. the wave file i.e. the sample data used by the instruments).
#define SIERRASTANDARD_SIZE 65536

// Maximum number of instruments in an Apple IIGS instrument set.
// Chosen empirically based on Apple IIGS AGI game data, increase if needed.
#define MAX_INSTRUMENTS 28

// The MIDI player allocates one generator for each note it starts to play.
// Here the maximum number of generators is defined. Feel free to increase
// this if it does not seem to be enough.
#define MAX_GENERATORS 16

#define ENVELOPE_SEGMENT_COUNT 8
#define MAX_OSCILLATOR_WAVES 127  // Maximum is one for every MIDI key

struct IIgsInstrumentHeader {
	struct {
		frac_t bp;		///< Envelope segment breakpoint
		frac_t inc;		///< Envelope segment velocity
	} env[ENVELOPE_SEGMENT_COUNT];
	uint8 seg;			///< Envelope release segment
	uint8 bend;			///< Maximum range for pitch bend
	uint8 vibDepth;		///< Vibrato depth
	uint8 vibSpeed;		///< Vibrato speed
	uint8 waveCount[2];	///< Wave count for both generators
	struct {
		uint8 key;		///< Highest MIDI key to use this wave
		int offset;		///< Offset of wave data, relative to base
		uint size;		///< Wave size
		bool halt;		///< Oscillator halted?
		bool loop;		///< Loop mode?
		bool swap;		///< Swap mode?
		bool chn;		///< Output channel (left / right)
		int16 tune;		///< Fine tune in semitones (8.8 fixed point)
	} wave[2][MAX_OSCILLATOR_WAVES];

	int8* base; ///< Base of wave data

	/**
	 * Read an Apple IIGS instrument header from the given stream.
	 * @param stream The source stream from which to read the data.
	 * @param ignoreAddr Should we ignore wave infos' wave address variable's value?
	 * @return True if successful, false otherwise.
	 */
	bool read(Common::SeekableReadStream &stream, bool ignoreAddr = false);
	bool finalize(int8 *);
};

struct IIgsSampleHeader {
	uint16 type;
	uint8  pitch; ///< Logarithmic, base is 2**(1/12), unknown multiplier (Possibly in range 1040-1080)
	uint8  unknownByte_Ofs3; // 0x7F in Gold Rush's sound resource 60, 0 in all others.
	uint8  volume; ///< Current guess: Logarithmic in 6 dB steps
	uint8  unknownByte_Ofs5; ///< 0 in all tested samples.
	uint16 instrumentSize; ///< Little endian. 44 in all tested samples. A guess.
	uint16 sampleSize; ///< Little endian. Accurate in all tested samples excluding Manhunter I's sound resource 16.
	IIgsInstrumentHeader instrument;

	/**
	 * Read an Apple IIGS AGI sample header from the given stream.
	 * @param stream The source stream from which to read the data.
	 * @return True if successful, false otherwise.
	 */
	bool read(Common::SeekableReadStream &stream);
	bool finalize(int8 *sample);
};

class IIgsGenerator {
public:
	IIgsGenerator() : ins(NULL), key(-1), chn(-1) {}

	const IIgsInstrumentHeader *ins; ///< Currently used instrument
	int key;		///< MIDI key
	int vel;		///< MIDI velocity (& channel volume)
	int chn;		///< MIDI channel
	struct {
		int8 *base;	///< Sample base pointer
		uint size;	///< Sample size
		frac_t p;	///< Sample pointer
		frac_t pd;	///< Sample pointer delta
		bool halt;	///< Is oscillator halted?
		bool loop;	///< Is looping enabled?
		bool swap;	///< Is swapping enabled?
		bool chn;	///< Output channel (left / right)
	} osc[2];
	int seg;		///< Current envelope segment
	frac_t a;		///< Current envelope amplitude
};

class IIgsMidi : public AgiSound {
public:
	IIgsMidi(uint8 *data, uint32 len, int resnum, SoundMgr &manager);
	~IIgsMidi() { if (_data != NULL) free(_data); }
	virtual uint16 type() { return _type; }
	virtual const uint8 *getPtr() { return _ptr; }
	virtual void setPtr(const uint8 *ptr) { _ptr = ptr; }
	virtual void rewind() { _ptr = _data + 2; _ticks = 0; }
protected:
	uint8 *_data; ///< Raw sound resource data
	const uint8 *_ptr; ///< Pointer to the current position in the MIDI data
	uint32 _len; ///< Length of the raw sound resource
	uint16 _type; ///< Sound resource type
public:
	uint _ticks; ///< MIDI song position in ticks (1/60ths of a second)
};

class IIgsSample : public AgiSound {
public:
	IIgsSample(uint8 *data, uint32 len, int resnum, SoundMgr &manager);
	~IIgsSample() { delete[] _sample; }
	virtual uint16 type() { return _header.type; }
	const IIgsSampleHeader &getHeader() const { return _header; }
	const int8 *getSample() const { return _sample; }
protected:
	IIgsSampleHeader _header;	///< Apple IIGS AGI sample header
	int8 *_sample;				///< Sample data (8-bit signed format)
};

/** Apple IIGS MIDI program change to instrument number mapping. */
struct IIgsMidiProgramMapping {
	byte midiProgToInst[44]; ///< Lookup table for the MIDI program number to instrument number mapping
	byte undefinedInst; ///< The undefined instrument number

	// Maps the MIDI program number to an instrument number
	byte map(uint midiProg) const {
		return midiProg < ARRAYSIZE(midiProgToInst) ? midiProgToInst[midiProg] : undefinedInst;
	}
};

/** Apple IIGS AGI instrument set information. */
struct IIgsInstrumentSetInfo {
	uint byteCount; ///< Length of the whole instrument set in bytes
	uint instCount; ///< Amount of instrument in the set
	const char *md5; ///< MD5 hex digest of the whole instrument set
	const char *waveFileMd5; ///< MD5 hex digest of the wave file (i.e. the sample data used by the instruments)
	const IIgsMidiProgramMapping *progToInst; ///< Program change to instrument number mapping
};

/** Apple IIGS AGI executable file information. */
struct IIgsExeInfo {
	enum AgiGameID gameid;	///< Game ID
	const char *exePrefix;	///< Prefix of the Apple IIGS AGI executable (e.g. "SQ", "PQ", "KQ4" etc)
	uint agiVer;			///< Apple IIGS AGI version number, not strictly needed
	uint exeSize;			///< Size of the Apple IIGS AGI executable file in bytes
	uint instSetStart;		///< Starting offset of the instrument set inside the executable file
	const IIgsInstrumentSetInfo *instSet; ///< Information about the used instrument set
};

class IIgsMidiChannel {
public:
	IIgsMidiChannel() : _instrument(NULL), _volume(127) {}
	void setInstrument(const IIgsInstrumentHeader *instrument) { _instrument = instrument; }
	const IIgsInstrumentHeader* getInstrument() { return _instrument; }
	void setVolume(int volume) { _volume = volume; }
	int getVolume() { return _volume; }
private:
	const IIgsInstrumentHeader *_instrument;	///< Instrument used on this MIDI channel
	int _volume;								///< MIDI controller number 7 (Volume)
};

class SoundGen2GS : public SoundGen, public Audio::AudioStream {
public:
	SoundGen2GS(AgiBase *vm, Audio::Mixer *pMixer);
	~SoundGen2GS();

	void play(int resnum);
	void stop(void);

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const { return true; }
	bool endOfData() const { return false; }
	int getRate() const { return _sampleRate; }

private:
	// Loader methods
	bool loadInstruments();
	bool loadInstrumentHeaders(Common::String &exePath, const IIgsExeInfo &exeInfo);
	bool loadWaveFile(Common::String &wavePath, const IIgsExeInfo &exeInfo);

	const IIgsExeInfo *getIIgsExeInfo(enum AgiGameID gameid) const;
	void setProgramChangeMapping(const IIgsMidiProgramMapping *mapping);

	// Player methods
	void advancePlayer();		///< Advance the player
	void advanceMidiPlayer();	///< Advance MIDI player
	uint generateOutput();		///< Fill the output buffer

	void haltGenerators();		///< Halt all generators
	uint activeGenerators();	///< How many generators are active?

	void midiNoteOff(int channel, int note, int velocity);
	void midiNoteOn(int channel, int note, int velocity);
	double midiKeyToFreq(int key, double finetune);
	IIgsInstrumentHeader* getInstrument(uint8 program) { return &_instruments[_progToInst->map(program)]; }
	IIgsGenerator* allocateGenerator() { IIgsGenerator* g = &_generators[_nextGen++]; _nextGen %= 16; return g; }

	bool _disableMidi;	///< Disable MIDI if loading instruments fail
	int _playingSound;	///< Resource number for the currently playing sound
	bool _playing;		///< True when the resource is still playing

	IIgsGenerator _generators[MAX_GENERATORS];			///< IIGS sound generators that are used to play single notes
	uint _nextGen;										///< Next generator available for allocation
	IIgsMidiChannel _channels[16];						///< MIDI channels
	Common::Array<IIgsInstrumentHeader> _instruments;	///< Instrument data
	const IIgsMidiProgramMapping *_progToInst;			///< MIDI program number to instrument mapping
	int8 *_wavetable;									///< Sample data used by the instruments
	uint _ticks;										///< MIDI ticks (60Hz)
	int16 *_out;										///< Output buffer
	uint _outSize;										///< Output buffer size

	static const int kSfxMidiChannel = 15; ///< MIDI channel used for playing sample resources
};

} // End of namespace Agi

#endif /* AGI_SOUND_2GS_H */
