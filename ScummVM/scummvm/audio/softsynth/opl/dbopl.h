/*
 *  Copyright (C) 2002-2011  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

// Last synch with DOSBox SVN trunk r3752

#ifndef AUDIO_SOFTSYNTH_OPL_DBOPL_H
#define AUDIO_SOFTSYNTH_OPL_DBOPL_H

#include "common/scummsys.h"

#ifndef DISABLE_DOSBOX_OPL

namespace OPL {
namespace DOSBox {

//Use 8 handlers based on a small logatirmic wavetabe and an exponential table for volume
#define WAVE_HANDLER	10
//Use a logarithmic wavetable with an exponential table for volume
#define WAVE_TABLELOG	11
//Use a linear wavetable with a multiply table for volume
#define WAVE_TABLEMUL	12

//Select the type of wave generator routine
#define DBOPL_WAVE WAVE_TABLEMUL

namespace DBOPL {

// Type aliases for the DBOPL code
typedef int Bits;
typedef uint Bitu;

typedef int8 Bit8s;
typedef uint8 Bit8u;

typedef int16 Bit16s;
typedef uint16 Bit16u;

typedef int32 Bit32s;
typedef uint32 Bit32u;

#define DB_FASTCALL
#define GCC_UNLIKELY(x) (x)
#define INLINE inline
// -------------------------------

struct Chip;
struct Operator;
struct Channel;

#if (DBOPL_WAVE == WAVE_HANDLER)
typedef Bits ( DB_FASTCALL *WaveHandler) ( Bitu i, Bitu volume );
#endif

typedef Bits ( DBOPL::Operator::*VolumeHandler) ( );
typedef Channel* ( DBOPL::Channel::*SynthHandler) ( Chip* chip, Bit32u samples, Bit32s* output );

//Different synth modes that can generate blocks of data
typedef enum {
	sm2AM,
	sm2FM,
	sm3AM,
	sm3FM,
	sm4Start,
	sm3FMFM,
	sm3AMFM,
	sm3FMAM,
	sm3AMAM,
	sm6Start,
	sm2Percussion,
	sm3Percussion
} SynthMode;

//Shifts for the values contained in chandata variable
enum {
	SHIFT_KSLBASE = 16,
	SHIFT_KEYCODE = 24
};

struct Operator {
public:
	//Masks for operator 20 values
	enum {
		MASK_KSR = 0x10,
		MASK_SUSTAIN = 0x20,
		MASK_VIBRATO = 0x40,
		MASK_TREMOLO = 0x80
	};

	typedef enum {
		OFF,
		RELEASE,
		SUSTAIN,
		DECAY,
		ATTACK
	} State;

	VolumeHandler volHandler;

#if (DBOPL_WAVE == WAVE_HANDLER)
	WaveHandler waveHandler;	//Routine that generate a wave
#else
	Bit16s* waveBase;
	Bit32u waveMask;
	Bit32u waveStart;
#endif
	Bit32u waveIndex;			//WAVE_BITS shifted counter of the frequency index
	Bit32u waveAdd;				//The base frequency without vibrato
	Bit32u waveCurrent;			//waveAdd + vibratao

	Bit32u chanData;			//Frequency/octave and derived data coming from whatever channel controls this
	Bit32u freqMul;				//Scale channel frequency with this, TODO maybe remove?
	Bit32u vibrato;				//Scaled up vibrato strength
	Bit32s sustainLevel;		//When stopping at sustain level stop here
	Bit32s totalLevel;			//totalLevel is added to every generated volume
	Bit32u currentLevel;		//totalLevel + tremolo
	Bit32s volume;				//The currently active volume

	Bit32u attackAdd;			//Timers for the different states of the envelope
	Bit32u decayAdd;
	Bit32u releaseAdd;
	Bit32u rateIndex;			//Current position of the evenlope

	Bit8u rateZero;				//Bits for the different states of the envelope having no changes
	Bit8u keyOn;				//Bitmask of different values that can generate keyon
	//Registers, also used to check for changes
	Bit8u reg20, reg40, reg60, reg80, regE0;
	//Active part of the envelope we're in
	Bit8u state;
	//0xff when tremolo is enabled
	Bit8u tremoloMask;
	//Strength of the vibrato
	Bit8u vibStrength;
	//Keep track of the calculated KSR so we can check for changes
	Bit8u ksr;
private:
	void SetState( Bit8u s );
	void UpdateAttack( const Chip* chip );
	void UpdateRelease( const Chip* chip );
	void UpdateDecay( const Chip* chip );
public:
	void UpdateAttenuation();
	void UpdateRates( const Chip* chip );
	void UpdateFrequency( );

	void Write20( const Chip* chip, Bit8u val );
	void Write40( const Chip* chip, Bit8u val );
	void Write60( const Chip* chip, Bit8u val );
	void Write80( const Chip* chip, Bit8u val );
	void WriteE0( const Chip* chip, Bit8u val );

	bool Silent() const;
	void Prepare( const Chip* chip );

	void KeyOn( Bit8u mask);
	void KeyOff( Bit8u mask);

	template< State state>
	Bits TemplateVolume( );

	Bit32s RateForward( Bit32u add );
	Bitu ForwardWave();
	Bitu ForwardVolume();

	Bits GetSample( Bits modulation );
	Bits GetWave( Bitu index, Bitu vol );
public:
	Operator();
};

struct Channel {
	Operator op[2];
	inline Operator* Op( Bitu index ) {
		return &( ( this + (index >> 1) )->op[ index & 1 ]);
	}
	SynthHandler synthHandler;
	Bit32u chanData;		//Frequency/octave and derived values
	Bit32s old[2];			//Old data for feedback

	Bit8u feedback;			//Feedback shift
	Bit8u regB0;			//Register values to check for changes
	Bit8u regC0;
	//This should correspond with reg104, bit 6 indicates a Percussion channel, bit 7 indicates a silent channel
	Bit8u fourMask;
	Bit8s maskLeft;		//Sign extended values for both channel's panning
	Bit8s maskRight;

	//Forward the channel data to the operators of the channel
	void SetChanData( const Chip* chip, Bit32u data );
	//Change in the chandata, check for new values and if we have to forward to operators
	void UpdateFrequency( const Chip* chip, Bit8u fourOp );
	void WriteA0( const Chip* chip, Bit8u val );
	void WriteB0( const Chip* chip, Bit8u val );
	void WriteC0( const Chip* chip, Bit8u val );
	void ResetC0( const Chip* chip );

	//call this for the first channel
	template< bool opl3Mode >
	void GeneratePercussion( Chip* chip, Bit32s* output );

	//Generate blocks of data in specific modes
	template<SynthMode mode>
	Channel* BlockTemplate( Chip* chip, Bit32u samples, Bit32s* output );
	Channel();
};

struct Chip {
	//This is used as the base counter for vibrato and tremolo
	Bit32u lfoCounter;
	Bit32u lfoAdd;


	Bit32u noiseCounter;
	Bit32u noiseAdd;
	Bit32u noiseValue;

	//Frequency scales for the different multiplications
	Bit32u freqMul[16];
	//Rates for decay and release for rate of this chip
	Bit32u linearRates[76];
	//Best match attack rates for the rate of this chip
	Bit32u attackRates[76];

	//18 channels with 2 operators each
	Channel chan[18];

	Bit8u reg104;
	Bit8u reg08;
	Bit8u reg04;
	Bit8u regBD;
	Bit8u vibratoIndex;
	Bit8u tremoloIndex;
	Bit8s vibratoSign;
	Bit8u vibratoShift;
	Bit8u tremoloValue;
	Bit8u vibratoStrength;
	Bit8u tremoloStrength;
	//Mask for allowed wave forms
	Bit8u waveFormMask;
	//0 or -1 when enabled
	Bit8s opl3Active;

	//Return the maximum amount of samples before and LFO change
	Bit32u ForwardLFO( Bit32u samples );
	Bit32u ForwardNoise();

	void WriteBD( Bit8u val );
	void WriteReg(Bit32u reg, Bit8u val );

	Bit32u WriteAddr( Bit32u port, Bit8u val );

	void GenerateBlock2( Bitu samples, Bit32s* output );
	void GenerateBlock3( Bitu samples, Bit32s* output );

	void Generate( Bit32u samples );
	void Setup( Bit32u r );

	Chip();
};

void InitTables();

}		//Namespace
} // End of namespace DOSBox
} // End of namespace OPL

#endif // !DISABLE_DOSBOX_OPL

#endif
