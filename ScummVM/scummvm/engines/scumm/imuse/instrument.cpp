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


#include "scumm/scumm.h"
#include "scumm/saveload.h"
#include "scumm/imuse/instrument.h"
#include "audio/mididrv.h"

namespace Scumm {

static bool _native_mt32 = false;

static struct {
	const char *name;
	byte program;
}

roland_to_gm_map[] = {
	// Monkey Island 2 instruments
	// TODO: Complete
	{ "badspit   ",  62 },
	{ "Big Drum  ", 116 },
	{ "burp      ",  58 },
//	{ "dinkfall  ", ??? },
//	{ "Fire Pit  ", ??? },
	{ "foghorn   ",  60 },
	{ "glop      ",  39 },
//	{ "jacob's la", ??? },
	{ "LeshBass  ",  33 },
//	{ "lowsnort  ", ??? },
	{ "ML explosn", 127 },
	{ "ReggaeBass",  32 },
//	{ "rope fall ", ??? },
	{ "rumble    ",  89 },
	{ "SdTrk Bend",  97 },
//	{ "snort     ", ??? },
	{ "spitting  ",  62 },
	{ "Swell 1   ",  95 },
	{ "Swell 2   ",  95 },
	{ "thnderclap", 127 }

	// Fate of Atlantis instruments
	// TODO: Build
//	{ "*aah!     ", ??? },
//	{ "*ooh!     ", ??? },
//	{ "*ShotFar4 ", ??? },
//	{ "*splash3  ", ??? },
//	{ "*torpedo5 ", ??? },
//	{ "*whip3    ", ??? },
//	{ "*woodknock", ??? },
//	{ "35 lavabub", ??? },
//	{ "49 bzzt!  ", ??? },
//	{ "applause  ", ??? },
//	{ "Arabongo  ", ??? },
//	{ "Big Drum  ", ??? }, // DUPLICATE (todo: confirm)
//	{ "bodythud1 ", ??? },
//	{ "boneKLOK2 ", ??? },
//	{ "boom10    ", ??? },
//	{ "boom11    ", ??? },
//	{ "boom15    ", ??? },
//	{ "boxclik1a ", ??? },
//	{ "brassbonk3", ??? },
//	{ "carstart  ", ??? },
//	{ "cb tpt 2  ", ??? },
//	{ "cell door ", ??? },
//	{ "chains    ", ??? },
//	{ "crash     ", ??? },
//	{ "crsrt/idl3", ??? },
//	{ "Fire Pit  ", ??? }, // DUPLICATE (todo: confirm)
//	{ "Fzooom    ", ??? },
//	{ "Fzooom 2  ", ??? },
//	{ "ghostwhosh", ??? },
//	{ "glasssmash", ??? },
//	{ "gloop2    ", ??? },
//	{ "gunShotNea", ??? },
//	{ "idoorclse ", ??? },
//	{ "knife     ", ??? },
//	{ "lavacmbl4 ", ??? },
//	{ "Mellow Str", ??? },
//	{ "mtlheater1", ??? },
//	{ "pachinko5 ", ??? },
//	{ "Ping1     ", ??? },
//	{ "rockcrunch", ??? },
//	{ "rumble    ", ??? }, // DUPLICATE (todo: confirm)
//	{ "runngwatr ", ??? },
//	{ "scrape2   ", ??? },
//	{ "snakeHiss ", ??? },
//	{ "snort     ", ??? }, // DUPLICATE (todo: confirm)
//	{ "spindle4  ", ??? },
//	{ "splash2   ", ??? },
//	{ "squirel   ", ??? },
//	{ "steam3    ", ??? },
//	{ "stonwheel6", ??? },
//	{ "street    ", ??? },
//	{ "trickle4  ", ??? }
};

// This emulates the percussion bank setup LEC used with the MT-32,
// where notes 24 - 34 were assigned instruments without reverb.
// It also fixes problems on GS devices that map sounds to these
// notes by default.
const byte Instrument::_gmRhythmMap[35] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0, 36, 37, 38, 39, 40, 41, 66, 47,
	65, 48, 56
};

class Instrument_Program : public InstrumentInternal {
private:
	byte _program;
	bool _mt32;

public:
	Instrument_Program(byte program, bool mt32);
	Instrument_Program(Serializer *s);
	void saveOrLoad(Serializer *s);
	void send(MidiChannel *mc);
	void copy_to(Instrument *dest) { dest->program(_program, _mt32); }
	bool is_valid() {
		return (_program < 128) &&
		       ((_native_mt32 == _mt32) || _native_mt32
		        ? (MidiDriver::_gmToMt32[_program] < 128)
		        : (MidiDriver::_mt32ToGm[_program] < 128));
	}
};

class Instrument_AdLib : public InstrumentInternal {
private:

#include "common/pack-start.h"  // START STRUCT PACKING

	struct AdLibInstrument {
		byte flags_1;
		byte oplvl_1;
		byte atdec_1;
		byte sustrel_1;
		byte waveform_1;
		byte flags_2;
		byte oplvl_2;
		byte atdec_2;
		byte sustrel_2;
		byte waveform_2;
		byte feedback;
		byte flags_a;
		struct {
			byte a, b, c, d, e, f, g, h;
		} extra_a;
		byte flags_b;
		struct {
			byte a, b, c, d, e, f, g, h;
		} extra_b;
		byte duration;
	} PACKED_STRUCT;

#include "common/pack-end.h"    // END STRUCT PACKING

	AdLibInstrument _instrument;

public:
	Instrument_AdLib(const byte *data);
	Instrument_AdLib(Serializer *s);
	void saveOrLoad(Serializer *s);
	void send(MidiChannel *mc);
	void copy_to(Instrument *dest) { dest->adlib((byte *)&_instrument); }
	bool is_valid() { return true; }
};

class Instrument_Roland : public InstrumentInternal {
private:

#include "common/pack-start.h"  // START STRUCT PACKING

	struct RolandInstrument {
		byte roland_id;
		byte device_id;
		byte model_id;
		byte command;
		byte address[3];
		struct {
			byte name[10];
			byte partial_struct12;
			byte partial_struct34;
			byte partial_mute;
			byte env_mode;
		} common;
		struct {
			byte wg_pitch_coarse;
			byte wg_pitch_fine;
			byte wg_pitch_keyfollow;
			byte wg_pitch_bender_sw;
			byte wg_waveform_pcm_bank;
			byte wg_pcm_wave_num;
			byte wg_pulse_width;
			byte wg_pw_velo_sens;
			byte p_env_depth;
			byte p_evn_velo_sens;
			byte p_env_time_keyf;
			byte p_env_time[4];
			byte p_env_level[3];
			byte p_env_sustain_level;
			byte end_level;
			byte p_lfo_rate;
			byte p_lfo_depth;
			byte p_lfo_mod_sens;
			byte tvf_cutoff_freq;
			byte tvf_resonance;
			byte tvf_keyfollow;
			byte tvf_bias_point_dir;
			byte tvf_bias_level;
			byte tvf_env_depth;
			byte tvf_env_velo_sens;
			byte tvf_env_depth_keyf;
			byte tvf_env_time_keyf;
			byte tvf_env_time[5];
			byte tvf_env_level[3];
			byte tvf_env_sustain_level;
			byte tva_level;
			byte tva_velo_sens;
			byte tva_bias_point_1;
			byte tva_bias_level_1;
			byte tva_bias_point_2;
			byte tva_bias_level_2;
			byte tva_env_time_keyf;
			byte tva_env_time_v_follow;
			byte tva_env_time[5];
			byte tva_env_level[3];
			byte tva_env_sustain_level;
		} partial[4];
		byte checksum;
	} PACKED_STRUCT;

#include "common/pack-end.h"    // END STRUCT PACKING

	RolandInstrument _instrument;

	char _instrument_name[11];

	uint8 getEquivalentGM();

public:
	Instrument_Roland(const byte *data);
	Instrument_Roland(Serializer *s);
	void saveOrLoad(Serializer *s);
	void send(MidiChannel *mc);
	void copy_to(Instrument *dest) { dest->roland((byte *)&_instrument); }
	bool is_valid() { return (_native_mt32 ? true : (_instrument_name[0] != '\0')); }
};

class Instrument_PcSpk : public InstrumentInternal {
public:
	Instrument_PcSpk(const byte *data);
	Instrument_PcSpk(Serializer *s);
	void saveOrLoad(Serializer *s);
	void send(MidiChannel *mc);
	void copy_to(Instrument *dest) { dest->pcspk((byte *)&_instrument); }
	bool is_valid() { return true; }

private:
	byte _instrument[23];
};

class Instrument_MacSfx : public InstrumentInternal {
private:
	byte _program;

public:
	Instrument_MacSfx(byte program);
	Instrument_MacSfx(Serializer *s);
	void saveOrLoad(Serializer *s);
	void send(MidiChannel *mc);
	void copy_to(Instrument *dest) { dest->macSfx(_program); }
	bool is_valid() {
		return (_program < 128);
	}
};

////////////////////////////////////////
//
// Instrument class members
//
////////////////////////////////////////

void Instrument::nativeMT32(bool native) {
	_native_mt32 = native;
}

void Instrument::clear() {
	delete _instrument;
	_instrument = NULL;
	_type = itNone;
}

void Instrument::program(byte prog, bool mt32) {
	clear();
	if (prog > 127)
		return;
	_type = itProgram;
	_instrument = new Instrument_Program(prog, mt32);
}

void Instrument::adlib(const byte *instrument) {
	clear();
	if (!instrument)
		return;
	_type = itAdLib;
	_instrument = new Instrument_AdLib(instrument);
}

void Instrument::roland(const byte *instrument) {
	clear();
	if (!instrument)
		return;
	_type = itRoland;
	_instrument = new Instrument_Roland(instrument);
}

void Instrument::pcspk(const byte *instrument) {
	clear();
	if (!instrument)
		return;
	_type = itPcSpk;
	_instrument = new Instrument_PcSpk(instrument);
}

void Instrument::macSfx(byte prog) {
	clear();
	if (prog > 127)
		return;
	_type = itMacSfx;
	_instrument = new Instrument_MacSfx(prog);
}

void Instrument::saveOrLoad(Serializer *s) {
	if (s->isSaving()) {
		s->saveByte(_type);
		if (_instrument)
			_instrument->saveOrLoad(s);
	} else {
		clear();
		_type = s->loadByte();
		switch (_type) {
		case itNone:
			break;
		case itProgram:
			_instrument = new Instrument_Program(s);
			break;
		case itAdLib:
			_instrument = new Instrument_AdLib(s);
			break;
		case itRoland:
			_instrument = new Instrument_Roland(s);
			break;
		case itPcSpk:
			_instrument = new Instrument_PcSpk(s);
			break;
		case itMacSfx:
			_instrument = new Instrument_MacSfx(s);
			break;
		default:
			warning("No known instrument classification #%d", (int)_type);
			_type = itNone;
		}
	}
}

////////////////////////////////////////
//
// Instrument_Program class members
//
////////////////////////////////////////

Instrument_Program::Instrument_Program(byte program, bool mt32) :
	_program(program),
	_mt32(mt32) {
	if (program > 127)
		_program = 255;
}

Instrument_Program::Instrument_Program(Serializer *s) {
	_program = 255;
	if (!s->isSaving())
		saveOrLoad(s);
}

void Instrument_Program::saveOrLoad(Serializer *s) {
	if (s->isSaving()) {
		s->saveByte(_program);
		s->saveByte(_mt32 ? 1 : 0);
	} else {
		_program = s->loadByte();
		_mt32 = (s->loadByte() > 0);
	}
}

void Instrument_Program::send(MidiChannel *mc) {
	if (_program > 127)
		return;

	byte program = _program;
	if (_native_mt32 != _mt32)
		program = _native_mt32 ? MidiDriver::_gmToMt32[program] : MidiDriver::_mt32ToGm[program];
	if (program < 128)
		mc->programChange(program);
}

////////////////////////////////////////
//
// Instrument_AdLib class members
//
////////////////////////////////////////

Instrument_AdLib::Instrument_AdLib(const byte *data) {
	memcpy(&_instrument, data, sizeof(_instrument));
}

Instrument_AdLib::Instrument_AdLib(Serializer *s) {
	if (!s->isSaving())
		saveOrLoad(s);
	else
		memset(&_instrument, 0, sizeof(_instrument));
}

void Instrument_AdLib::saveOrLoad(Serializer *s) {
	if (s->isSaving())
		s->saveBytes(&_instrument, sizeof(_instrument));
	else
		s->loadBytes(&_instrument, sizeof(_instrument));
}

void Instrument_AdLib::send(MidiChannel *mc) {
	mc->sysEx_customInstrument('ADL ', (byte *)&_instrument);
}

////////////////////////////////////////
//
// Instrument_Roland class members
//
////////////////////////////////////////

Instrument_Roland::Instrument_Roland(const byte *data) {
	memcpy(&_instrument, data, sizeof(_instrument));
	memcpy(&_instrument_name, &_instrument.common.name, sizeof(_instrument.common.name));
	_instrument_name[10] = '\0';
	if (!_native_mt32 && getEquivalentGM() >= 128) {
		debug(0, "MT-32 instrument \"%s\" not supported yet", _instrument_name);
		_instrument_name[0] = '\0';
	}
}

Instrument_Roland::Instrument_Roland(Serializer *s) {
	_instrument_name[0] = '\0';
	if (!s->isSaving())
		saveOrLoad(s);
	else
		memset(&_instrument, 0, sizeof(_instrument));
}

void Instrument_Roland::saveOrLoad(Serializer *s) {
	if (s->isSaving()) {
		s->saveBytes(&_instrument, sizeof(_instrument));
	} else {
		s->loadBytes(&_instrument, sizeof(_instrument));
		memcpy(&_instrument_name, &_instrument.common.name, sizeof(_instrument.common.name));
		_instrument_name[10] = '\0';
		if (!_native_mt32 && getEquivalentGM() >= 128) {
			debug(2, "MT-32 custom instrument \"%s\" not supported", _instrument_name);
			_instrument_name[0] = '\0';
		}
	} // end if
}

void Instrument_Roland::send(MidiChannel *mc) {
	if (_native_mt32) {
		if (mc->getNumber() > 8)
			return;
		_instrument.device_id = mc->getNumber();

		// Remap instrument to appropriate address space.
		int address = 0x008000;
		_instrument.address[0] = (address >> 14) & 0x7F;
		_instrument.address[1] = (address >>  7) & 0x7F;
		_instrument.address[2] = (address      ) & 0x7F;

		// Recompute the checksum.
		byte checksum = 0;
		byte *ptr = (byte *)&_instrument + 4;
		int i;
		for (i = 4; i < (int)sizeof(_instrument) - 1; ++i)
			checksum -= *ptr++;
		_instrument.checksum = checksum & 0x7F;

		mc->device()->sysEx((byte *)&_instrument, sizeof(_instrument));
	} else {
		// Convert to a GM program change.
		byte program = getEquivalentGM();
		if (program < 128)
			mc->programChange(program);
	}
}

uint8 Instrument_Roland::getEquivalentGM() {
	byte i;
	for (i = 0; i != ARRAYSIZE(roland_to_gm_map); ++i) {
		if (!memcmp(roland_to_gm_map[i].name, _instrument.common.name, 10))
			return roland_to_gm_map[i].program;
	}
	return 255;
}

////////////////////////////////////////
//
// Instrument_PcSpk class members
//
////////////////////////////////////////

Instrument_PcSpk::Instrument_PcSpk(const byte *data) {
	memcpy(_instrument, data, sizeof(_instrument));
}

Instrument_PcSpk::Instrument_PcSpk(Serializer *s) {
	if (!s->isSaving())
		saveOrLoad(s);
	else
		memset(_instrument, 0, sizeof(_instrument));
}

void Instrument_PcSpk::saveOrLoad(Serializer *s) {
	if (s->isSaving())
		s->saveBytes(_instrument, sizeof(_instrument));
	else
		s->loadBytes(_instrument, sizeof(_instrument));
}

void Instrument_PcSpk::send(MidiChannel *mc) {
	mc->sysEx_customInstrument('SPK ', (byte *)&_instrument);
}

////////////////////////////////////////
//
// Instrument_MacSfx class members
//
////////////////////////////////////////

Instrument_MacSfx::Instrument_MacSfx(byte program) :
	_program(program) {
	if (program > 127) {
		_program = 255;
	}
}

Instrument_MacSfx::Instrument_MacSfx(Serializer *s) {
	_program = 255;
	if (!s->isSaving()) {
		saveOrLoad(s);
	}
}

void Instrument_MacSfx::saveOrLoad(Serializer *s) {
	if (s->isSaving()) {
		s->saveByte(_program);
	} else {
		_program = s->loadByte();
	}
}

void Instrument_MacSfx::send(MidiChannel *mc) {
	if (_program > 127) {
		return;
	}
	mc->sysEx_customInstrument('MAC ', &_program);
}
} // End of namespace Scumm
