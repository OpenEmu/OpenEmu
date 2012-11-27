/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MT32EMU_STRUCTURES_H
#define MT32EMU_STRUCTURES_H

namespace MT32Emu {

// MT32EMU_MEMADDR() converts from sysex-padded, MT32EMU_SYSEXMEMADDR converts to it
// Roland provides documentation using the sysex-padded addresses, so we tend to use that in code and output
#define MT32EMU_MEMADDR(x) ((((x) & 0x7f0000) >> 2) | (((x) & 0x7f00) >> 1) | ((x) & 0x7f))
#define MT32EMU_SYSEXMEMADDR(x) ((((x) & 0x1FC000) << 2) | (((x) & 0x3F80) << 1) | ((x) & 0x7f))

#ifdef _MSC_VER
#define  MT32EMU_ALIGN_PACKED __declspec(align(1))
#else
#define MT32EMU_ALIGN_PACKED __attribute__((packed))
#endif

typedef unsigned int       Bit32u;
typedef   signed int       Bit32s;
typedef unsigned short int Bit16u;
typedef   signed short int Bit16s;
typedef unsigned char      Bit8u;
typedef   signed char      Bit8s;

// The following structures represent the MT-32's memory
// Since sysex allows this memory to be written to in blocks of bytes,
// we keep this packed so that we can copy data into the various
// banks directly
#if defined(_MSC_VER) || defined(__MINGW32__)
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

struct TimbreParam {
	struct CommonParam {
		char name[10];
		Bit8u partialStructure12;  // 1 & 2  0-12 (1-13)
		Bit8u partialStructure34;  // 3 & 4  0-12 (1-13)
		Bit8u partialMute;  // 0-15 (0000-1111)
		Bit8u noSustain; // ENV MODE 0-1 (Normal, No sustain)
	} MT32EMU_ALIGN_PACKED common;

	struct PartialParam {
		struct WGParam {
			Bit8u pitchCoarse;  // 0-96 (C1,C#1-C9)
			Bit8u pitchFine;  // 0-100 (-50 to +50 (cents - confirmed by Mok))
			Bit8u pitchKeyfollow;  // 0-16 (-1, -1/2, -1/4, 0, 1/8, 1/4, 3/8, 1/2, 5/8, 3/4, 7/8, 1, 5/4, 3/2, 2, s1, s2)
			Bit8u pitchBenderEnabled;  // 0-1 (OFF, ON)
			Bit8u waveform; // MT-32: 0-1 (SQU/SAW); LAPC-I: WG WAVEFORM/PCM BANK 0 - 3 (SQU/1, SAW/1, SQU/2, SAW/2)
			Bit8u pcmWave; // 0-127 (1-128)
			Bit8u pulseWidth; // 0-100
			Bit8u pulseWidthVeloSensitivity; // 0-14 (-7 - +7)
		} MT32EMU_ALIGN_PACKED wg;

		struct PitchEnvParam {
			Bit8u depth; // 0-10
			Bit8u veloSensitivity; // 0-100
			Bit8u timeKeyfollow; // 0-4
			Bit8u time[4]; // 0-100
			Bit8u level[5]; // 0-100 (-50 - +50) // [3]: SUSTAIN LEVEL, [4]: END LEVEL
		} MT32EMU_ALIGN_PACKED pitchEnv;

		struct PitchLFOParam {
			Bit8u rate; // 0-100
			Bit8u depth; // 0-100
			Bit8u modSensitivity; // 0-100
		} MT32EMU_ALIGN_PACKED pitchLFO;

		struct TVFParam {
			Bit8u cutoff; // 0-100
			Bit8u resonance; // 0-30
			Bit8u keyfollow; // -1, -1/2, -1/4, 0, 1/8, 1/4, 3/8, 1/2, 5/8, 3/4, 7/8, 1, 5/4, 3/2, 2
			Bit8u biasPoint; // 0-127 (<1A-<7C >1A-7C)
			Bit8u biasLevel; // 0-14 (-7 - +7)
			Bit8u envDepth; // 0-100
			Bit8u envVeloSensitivity; // 0-100
			Bit8u envDepthKeyfollow; // DEPTH KEY FOLL0W 0-4
			Bit8u envTimeKeyfollow; // TIME KEY FOLLOW 0-4
			Bit8u envTime[5]; // 0-100
			Bit8u envLevel[4]; // 0-100 // [3]: SUSTAIN LEVEL
		} MT32EMU_ALIGN_PACKED tvf;

		struct TVAParam {
			Bit8u level; // 0-100
			Bit8u veloSensitivity; // 0-100
			Bit8u biasPoint1; // 0-127 (<1A-<7C >1A-7C)
			Bit8u biasLevel1; // 0-12 (-12 - 0)
			Bit8u biasPoint2; // 0-127 (<1A-<7C >1A-7C)
			Bit8u biasLevel2; // 0-12 (-12 - 0)
			Bit8u envTimeKeyfollow; // TIME KEY FOLLOW 0-4
			Bit8u envTimeVeloSensitivity; // VELOS KEY FOLL0W 0-4
			Bit8u envTime[5]; // 0-100
			Bit8u envLevel[4]; // 0-100 // [3]: SUSTAIN LEVEL
		} MT32EMU_ALIGN_PACKED tva;
	} MT32EMU_ALIGN_PACKED partial[4];
} MT32EMU_ALIGN_PACKED;

struct PatchParam {
	Bit8u timbreGroup; // TIMBRE GROUP  0-3 (group A, group B, Memory, Rhythm)
	Bit8u timbreNum; // TIMBRE NUMBER 0-63
	Bit8u keyShift; // KEY SHIFT 0-48 (-24 - +24 semitones)
	Bit8u fineTune; // FINE TUNE 0-100 (-50 - +50 cents)
	Bit8u benderRange; // BENDER RANGE 0-24
	Bit8u assignMode;  // ASSIGN MODE 0-3 (POLY1, POLY2, POLY3, POLY4)
	Bit8u reverbSwitch;  // REVERB SWITCH 0-1 (OFF,ON)
	Bit8u dummy; // (DUMMY)
} MT32EMU_ALIGN_PACKED;

const unsigned int SYSTEM_MASTER_TUNE_OFF = 0;
const unsigned int SYSTEM_REVERB_MODE_OFF = 1;
const unsigned int SYSTEM_REVERB_TIME_OFF = 2;
const unsigned int SYSTEM_REVERB_LEVEL_OFF = 3;
const unsigned int SYSTEM_RESERVE_SETTINGS_START_OFF = 4;
const unsigned int SYSTEM_RESERVE_SETTINGS_END_OFF = 12;
const unsigned int SYSTEM_CHAN_ASSIGN_START_OFF = 13;
const unsigned int SYSTEM_CHAN_ASSIGN_END_OFF = 21;
const unsigned int SYSTEM_MASTER_VOL_OFF = 22;

struct MemParams {
	// NOTE: The MT-32 documentation only specifies PatchTemp areas for parts 1-8.
	// The LAPC-I documentation specified an additional area for rhythm at the end,
	// where all parameters but fine tune, assign mode and output level are ignored
	struct PatchTemp {
		PatchParam patch;
		Bit8u outputLevel; // OUTPUT LEVEL 0-100
		Bit8u panpot; // PANPOT 0-14 (R-L)
		Bit8u dummyv[6];
	} MT32EMU_ALIGN_PACKED patchTemp[9];

	struct RhythmTemp {
		Bit8u timbre; // TIMBRE  0-94 (M1-M64,R1-30,OFF); LAPC-I: 0-127 (M01-M64,R01-R63)
		Bit8u outputLevel; // OUTPUT LEVEL 0-100
		Bit8u panpot; // PANPOT 0-14 (R-L)
		Bit8u reverbSwitch;  // REVERB SWITCH 0-1 (OFF,ON)
	} MT32EMU_ALIGN_PACKED rhythmTemp[85];

	TimbreParam timbreTemp[8];

	PatchParam patches[128];

	// NOTE: There are only 30 timbres in the "rhythm" bank for MT-32; the additional 34 are for LAPC-I and above
	struct PaddedTimbre {
		TimbreParam timbre;
		Bit8u padding[10];
	} MT32EMU_ALIGN_PACKED timbres[64 + 64 + 64 + 64]; // Group A, Group B, Memory, Rhythm

	struct System {
		Bit8u masterTune; // MASTER TUNE 0-127 432.1-457.6Hz
		Bit8u reverbMode; // REVERB MODE 0-3 (room, hall, plate, tap delay)
		Bit8u reverbTime; // REVERB TIME 0-7 (1-8)
		Bit8u reverbLevel; // REVERB LEVEL 0-7 (1-8)
		Bit8u reserveSettings[9]; // PARTIAL RESERVE (PART 1) 0-32
		Bit8u chanAssign[9]; // MIDI CHANNEL (PART1) 0-16 (1-16,OFF)
		Bit8u masterVol; // MASTER VOLUME 0-100
	} MT32EMU_ALIGN_PACKED system;
};

#if defined(_MSC_VER) || defined(__MINGW32__)
#pragma pack(pop)
#else
#pragma pack()
#endif

struct ControlROMPCMStruct;

struct PCMWaveEntry {
	Bit32u addr;
	Bit32u len;
	bool loop;
	ControlROMPCMStruct *controlROMPCMStruct;
};

// This is basically a per-partial, pre-processed combination of timbre and patch/rhythm settings
struct PatchCache {
	bool playPartial;
	bool PCMPartial;
	int pcm;
	char waveform;

	Bit32u structureMix;
	int structurePosition;
	int structurePair;

	// The following fields are actually common to all partials in the timbre
	bool dirty;
	Bit32u partialCount;
	bool sustain;
	bool reverb;

	TimbreParam::PartialParam srcPartial;

	// The following directly points into live sysex-addressable memory
	const TimbreParam::PartialParam *partialParam;
};

class Partial; // Forward reference for class defined in partial.h

}

#endif
