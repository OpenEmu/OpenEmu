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

/*
	DOSBox implementation of a combined Yamaha YMF262 and Yamaha YM3812 emulator.
	Enabling the opl3 bit will switch the emulator to stereo opl3 output instead of regular mono opl2
	Except for the table generation it's all integer math
	Can choose different types of generators, using muls and bigger tables, try different ones for slower platforms
	The generation was based on the MAME implementation but tried to have it use less memory and be faster in general
	MAME uses much bigger envelope tables and this will be the biggest cause of it sounding different at times

	//TODO Don't delay first operator 1 sample in opl3 mode
	//TODO Maybe not use class method pointers but a regular function pointers with operator as first parameter
	//TODO Fix panning for the Percussion channels, would any opl3 player use it and actually really change it though?
	//TODO Check if having the same accuracy in all frequency multipliers sounds better or not

	//DUNNO Keyon in 4op, switch to 2op without keyoff.
*/

// Last synch with DOSBox SVN trunk r3752

#include "dbopl.h"

#ifndef DISABLE_DOSBOX_OPL

namespace OPL {
namespace DOSBox {

namespace DBOPL {

#define OPLRATE		((double)(14318180.0 / 288.0))
#define TREMOLO_TABLE 52

//Try to use most precision for frequencies
//Else try to keep different waves in synch
//#define WAVE_PRECISION	1
#ifndef WAVE_PRECISION
//Wave bits available in the top of the 32bit range
//Original adlib uses 10.10, we use 10.22
#define WAVE_BITS	10
#else
//Need some extra bits at the top to have room for octaves and frequency multiplier
//We support to 8 times lower rate
//128 * 15 * 8 = 15350, 2^13.9, so need 14 bits
#define WAVE_BITS	14
#endif
#define WAVE_SH		( 32 - WAVE_BITS )
#define WAVE_MASK	( ( 1 << WAVE_SH ) - 1 )

//Use the same accuracy as the waves
#define LFO_SH ( WAVE_SH - 10 )
//LFO is controlled by our tremolo 256 sample limit
#define LFO_MAX ( 256 << ( LFO_SH ) )


//Maximum amount of attenuation bits
//Envelope goes to 511, 9 bits
#if (DBOPL_WAVE == WAVE_TABLEMUL )
//Uses the value directly
#define ENV_BITS	( 9 )
#else
//Add 3 bits here for more accuracy and would have to be shifted up either way
#define ENV_BITS	( 9 )
#endif
//Limits of the envelope with those bits and when the envelope goes silent
#define ENV_MIN		0
#define ENV_EXTRA	( ENV_BITS - 9 )
#define ENV_MAX		( 511 << ENV_EXTRA )
#define ENV_LIMIT	( ( 12 * 256) >> ( 3 - ENV_EXTRA ) )
#define ENV_SILENT( _X_ ) ( (_X_) >= ENV_LIMIT )

//Attack/decay/release rate counter shift
#define RATE_SH		24
#define RATE_MASK	( ( 1 << RATE_SH ) - 1 )
//Has to fit within 16bit lookuptable
#define MUL_SH		16

//Check some ranges
#if ENV_EXTRA > 3
#error Too many envelope bits
#endif


//How much to substract from the base value for the final attenuation
static const Bit8u KslCreateTable[16] = {
	//0 will always be be lower than 7 * 8
	64, 32, 24, 19,
	16, 12, 11, 10,
	 8,  6,  5,  4,
	 3,  2,  1,  0,
};

#define M(_X_) ((Bit8u)( (_X_) * 2))
static const Bit8u FreqCreateTable[16] = {
	M(0.5), M(1 ), M(2 ), M(3 ), M(4 ), M(5 ), M(6 ), M(7 ),
	M(8  ), M(9 ), M(10), M(10), M(12), M(12), M(15), M(15)
};
#undef M

//We're not including the highest attack rate, that gets a special value
static const Bit8u AttackSamplesTable[13] = {
	69, 55, 46, 40,
	35, 29, 23, 20,
	19, 15, 11, 10,
	9
};
//On a real opl these values take 8 samples to reach and are based upon larger tables
static const Bit8u EnvelopeIncreaseTable[13] = {
	4,  5,  6,  7,
	8, 10, 12, 14,
	16, 20, 24, 28,
	32,
};

#if ( DBOPL_WAVE == WAVE_HANDLER ) || ( DBOPL_WAVE == WAVE_TABLELOG )
static Bit16u ExpTable[ 256 ];
#endif

#if ( DBOPL_WAVE == WAVE_HANDLER )
//PI table used by WAVEHANDLER
static Bit16u SinTable[ 512 ];
#endif

#if ( DBOPL_WAVE > WAVE_HANDLER )
//Layout of the waveform table in 512 entry intervals
//With overlapping waves we reduce the table to half it's size

//	|    |//\\|____|WAV7|//__|/\  |____|/\/\|
//	|\\//|    |    |WAV7|    |  \/|    |    |
//	|06  |0126|17  |7   |3   |4   |4 5 |5   |

//6 is just 0 shifted and masked

static Bit16s WaveTable[ 8 * 512 ];
//Distance into WaveTable the wave starts
static const Bit16u WaveBaseTable[8] = {
	0x000, 0x200, 0x200, 0x800,
	0xa00, 0xc00, 0x100, 0x400,

};
//Mask the counter with this
static const Bit16u WaveMaskTable[8] = {
	1023, 1023, 511, 511,
	1023, 1023, 512, 1023,
};

//Where to start the counter on at keyon
static const Bit16u WaveStartTable[8] = {
	512, 0, 0, 0,
	0, 512, 512, 256,
};
#endif

#if ( DBOPL_WAVE == WAVE_TABLEMUL )
static Bit16u MulTable[ 384 ];
#endif

static Bit8u KslTable[ 8 * 16 ];
static Bit8u TremoloTable[ TREMOLO_TABLE ];
//Start of a channel behind the chip struct start
static Bit16u ChanOffsetTable[32];
//Start of an operator behind the chip struct start
static Bit16u OpOffsetTable[64];

//The lower bits are the shift of the operator vibrato value
//The highest bit is right shifted to generate -1 or 0 for negation
//So taking the highest input value of 7 this gives 3, 7, 3, 0, -3, -7, -3, 0
static const Bit8s VibratoTable[ 8 ] = {
	1 - 0x00, 0 - 0x00, 1 - 0x00, 30 - 0x00,
	1 - 0x80, 0 - 0x80, 1 - 0x80, 30 - 0x80
};

//Shift strength for the ksl value determined by ksl strength
static const Bit8u KslShiftTable[4] = {
	31,1,2,0
};

//Generate a table index and table shift value using input value from a selected rate
static void EnvelopeSelect( Bit8u val, Bit8u& index, Bit8u& shift ) {
	if ( val < 13 * 4 ) {				//Rate 0 - 12
		shift = 12 - ( val >> 2 );
		index = val & 3;
	} else if ( val < 15 * 4 ) {		//rate 13 - 14
		shift = 0;
		index = val - 12 * 4;
	} else {							//rate 15 and up
		shift = 0;
		index = 12;
	}
}

#if ( DBOPL_WAVE == WAVE_HANDLER )
/*
	Generate the different waveforms out of the sine/exponetial table using handlers
*/
static inline Bits MakeVolume( Bitu wave, Bitu volume ) {
	Bitu total = wave + volume;
	Bitu index = total & 0xff;
	Bitu sig = ExpTable[ index ];
	Bitu exp = total >> 8;
#if 0
	//Check if we overflow the 31 shift limit
	if ( exp >= 32 ) {
		LOG_MSG( "WTF %d %d", total, exp );
	}
#endif
	return (sig >> exp);
}

static Bits DB_FASTCALL WaveForm0( Bitu i, Bitu volume ) {
	Bits neg = 0 - (( i >> 9) & 1);//Create ~0 or 0
	Bitu wave = SinTable[i & 511];
	return (MakeVolume( wave, volume ) ^ neg) - neg;
}
static Bits DB_FASTCALL WaveForm1( Bitu i, Bitu volume ) {
	Bit32u wave = SinTable[i & 511];
	wave |= ( ( (i ^ 512 ) & 512) - 1) >> ( 32 - 12 );
	return MakeVolume( wave, volume );
}
static Bits DB_FASTCALL WaveForm2( Bitu i, Bitu volume ) {
	Bitu wave = SinTable[i & 511];
	return MakeVolume( wave, volume );
}
static Bits DB_FASTCALL WaveForm3( Bitu i, Bitu volume ) {
	Bitu wave = SinTable[i & 255];
	wave |= ( ( (i ^ 256 ) & 256) - 1) >> ( 32 - 12 );
	return MakeVolume( wave, volume );
}
static Bits DB_FASTCALL WaveForm4( Bitu i, Bitu volume ) {
	//Twice as fast
	i <<= 1;
	Bits neg = 0 - (( i >> 9) & 1);//Create ~0 or 0
	Bitu wave = SinTable[i & 511];
	wave |= ( ( (i ^ 512 ) & 512) - 1) >> ( 32 - 12 );
	return (MakeVolume( wave, volume ) ^ neg) - neg;
}
static Bits DB_FASTCALL WaveForm5( Bitu i, Bitu volume ) {
	//Twice as fast
	i <<= 1;
	Bitu wave = SinTable[i & 511];
	wave |= ( ( (i ^ 512 ) & 512) - 1) >> ( 32 - 12 );
	return MakeVolume( wave, volume );
}
static Bits DB_FASTCALL WaveForm6( Bitu i, Bitu volume ) {
	Bits neg = 0 - (( i >> 9) & 1);//Create ~0 or 0
	return (MakeVolume( 0, volume ) ^ neg) - neg;
}
static Bits DB_FASTCALL WaveForm7( Bitu i, Bitu volume ) {
	//Negative is reversed here
	Bits neg = (( i >> 9) & 1) - 1;
	Bitu wave = (i << 3);
	//When negative the volume also runs backwards
	wave = ((wave ^ neg) - neg) & 4095;
	return (MakeVolume( wave, volume ) ^ neg) - neg;
}

static const WaveHandler WaveHandlerTable[8] = {
	WaveForm0, WaveForm1, WaveForm2, WaveForm3,
	WaveForm4, WaveForm5, WaveForm6, WaveForm7
};

#endif

/*
	Operator
*/

//We zero out when rate == 0
inline void Operator::UpdateAttack( const Chip* chip ) {
	Bit8u rate = reg60 >> 4;
	if ( rate ) {
		Bit8u val = (rate << 2) + ksr;
		attackAdd = chip->attackRates[ val ];
		rateZero &= ~(1 << ATTACK);
	} else {
		attackAdd = 0;
		rateZero |= (1 << ATTACK);
	}
}
inline void Operator::UpdateDecay( const Chip* chip ) {
	Bit8u rate = reg60 & 0xf;
	if ( rate ) {
		Bit8u val = (rate << 2) + ksr;
		decayAdd = chip->linearRates[ val ];
		rateZero &= ~(1 << DECAY);
	} else {
		decayAdd = 0;
		rateZero |= (1 << DECAY);
	}
}
inline void Operator::UpdateRelease( const Chip* chip ) {
	Bit8u rate = reg80 & 0xf;
	if ( rate ) {
		Bit8u val = (rate << 2) + ksr;
		releaseAdd = chip->linearRates[ val ];
		rateZero &= ~(1 << RELEASE);
		if ( !(reg20 & MASK_SUSTAIN ) ) {
			rateZero &= ~( 1 << SUSTAIN );
		}
	} else {
		rateZero |= (1 << RELEASE);
		releaseAdd = 0;
		if ( !(reg20 & MASK_SUSTAIN ) ) {
			rateZero |= ( 1 << SUSTAIN );
		}
	}
}

inline void Operator::UpdateAttenuation( ) {
	Bit8u kslBase = (Bit8u)((chanData >> SHIFT_KSLBASE) & 0xff);
	Bit32u tl = reg40 & 0x3f;
	Bit8u kslShift = KslShiftTable[ reg40 >> 6 ];
	//Make sure the attenuation goes to the right bits
	totalLevel = tl << ( ENV_BITS - 7 );	//Total level goes 2 bits below max
	totalLevel += ( kslBase << ENV_EXTRA ) >> kslShift;
}

void Operator::UpdateFrequency(  ) {
	Bit32u freq = chanData & (( 1 << 10 ) - 1);
	Bit32u block = (chanData >> 10) & 0xff;
#ifdef WAVE_PRECISION
	block = 7 - block;
	waveAdd = ( freq * freqMul ) >> block;
#else
	waveAdd = ( freq << block ) * freqMul;
#endif
	if ( reg20 & MASK_VIBRATO ) {
		vibStrength = (Bit8u)(freq >> 7);

#ifdef WAVE_PRECISION
		vibrato = ( vibStrength * freqMul ) >> block;
#else
		vibrato = ( vibStrength << block ) * freqMul;
#endif
	} else {
		vibStrength = 0;
		vibrato = 0;
	}
}

void Operator::UpdateRates( const Chip* chip ) {
	//Mame seems to reverse this where enabling ksr actually lowers
	//the rate, but pdf manuals says otherwise?
	Bit8u newKsr = (Bit8u)((chanData >> SHIFT_KEYCODE) & 0xff);
	if ( !( reg20 & MASK_KSR ) ) {
		newKsr >>= 2;
	}
	if ( ksr == newKsr )
		return;
	ksr = newKsr;
	UpdateAttack( chip );
	UpdateDecay( chip );
	UpdateRelease( chip );
}

INLINE Bit32s Operator::RateForward( Bit32u add ) {
	rateIndex += add;
	Bit32s ret = rateIndex >> RATE_SH;
	rateIndex = rateIndex & RATE_MASK;
	return ret;
}

template< Operator::State yes>
Bits Operator::TemplateVolume(  ) {
	Bit32s vol = volume;
	Bit32s change;
	switch ( yes ) {
	case OFF:
		return ENV_MAX;
	case ATTACK:
		change = RateForward( attackAdd );
		if ( !change )
			return vol;
		vol += ( (~vol) * change ) >> 3;
		if ( vol < ENV_MIN ) {
			volume = ENV_MIN;
			rateIndex = 0;
			SetState( DECAY );
			return ENV_MIN;
		}
		break;
	case DECAY:
		vol += RateForward( decayAdd );
		if ( GCC_UNLIKELY(vol >= sustainLevel) ) {
			//Check if we didn't overshoot max attenuation, then just go off
			if ( GCC_UNLIKELY(vol >= ENV_MAX) ) {
				volume = ENV_MAX;
				SetState( OFF );
				return ENV_MAX;
			}
			//Continue as sustain
			rateIndex = 0;
			SetState( SUSTAIN );
		}
		break;
	case SUSTAIN:
		if ( reg20 & MASK_SUSTAIN ) {
			return vol;
		}
		//In sustain phase, but not sustaining, do regular release
	case RELEASE:
		vol += RateForward( releaseAdd );
		if ( GCC_UNLIKELY(vol >= ENV_MAX) ) {
			volume = ENV_MAX;
			SetState( OFF );
			return ENV_MAX;
		}
		break;
	}
	volume = vol;
	return vol;
}

static const VolumeHandler VolumeHandlerTable[5] = {
	&Operator::TemplateVolume< Operator::OFF >,
	&Operator::TemplateVolume< Operator::RELEASE >,
	&Operator::TemplateVolume< Operator::SUSTAIN >,
	&Operator::TemplateVolume< Operator::DECAY >,
	&Operator::TemplateVolume< Operator::ATTACK >
};

INLINE Bitu Operator::ForwardVolume() {
	return currentLevel + (this->*volHandler)();
}


INLINE Bitu Operator::ForwardWave() {
	waveIndex += waveCurrent;
	return waveIndex >> WAVE_SH;
}

void Operator::Write20( const Chip* chip, Bit8u val ) {
	Bit8u change = (reg20 ^ val );
	if ( !change )
		return;
	reg20 = val;
	//Shift the tremolo bit over the entire register, saved a branch, YES!
	tremoloMask = (Bit8s)(val) >> 7;
	tremoloMask &= ~(( 1 << ENV_EXTRA ) -1);
	//Update specific features based on changes
	if ( change & MASK_KSR ) {
		UpdateRates( chip );
	}
	//With sustain enable the volume doesn't change
	if ( reg20 & MASK_SUSTAIN || ( !releaseAdd ) ) {
		rateZero |= ( 1 << SUSTAIN );
	} else {
		rateZero &= ~( 1 << SUSTAIN );
	}
	//Frequency multiplier or vibrato changed
	if ( change & (0xf | MASK_VIBRATO) ) {
		freqMul = chip->freqMul[ val & 0xf ];
		UpdateFrequency();
	}
}

void Operator::Write40( const Chip* /*chip*/, Bit8u val ) {
	if (!(reg40 ^ val ))
		return;
	reg40 = val;
	UpdateAttenuation( );
}

void Operator::Write60( const Chip* chip, Bit8u val ) {
	Bit8u change = reg60 ^ val;
	reg60 = val;
	if ( change & 0x0f ) {
		UpdateDecay( chip );
	}
	if ( change & 0xf0 ) {
		UpdateAttack( chip );
	}
}

void Operator::Write80( const Chip* chip, Bit8u val ) {
	Bit8u change = (reg80 ^ val );
	if ( !change )
		return;
	reg80 = val;
	Bit8u sustain = val >> 4;
	//Turn 0xf into 0x1f
	sustain |= ( sustain + 1) & 0x10;
	sustainLevel = sustain << ( ENV_BITS - 5 );
	if ( change & 0x0f ) {
		UpdateRelease( chip );
	}
}

void Operator::WriteE0( const Chip* chip, Bit8u val ) {
	if ( !(regE0 ^ val) )
		return;
	//in opl3 mode you can always selet 7 waveforms regardless of waveformselect
	Bit8u waveForm = val & ( ( 0x3 & chip->waveFormMask ) | (0x7 & chip->opl3Active ) );
	regE0 = val;
#if ( DBOPL_WAVE == WAVE_HANDLER )
	waveHandler = WaveHandlerTable[ waveForm ];
#else
	waveBase = WaveTable + WaveBaseTable[ waveForm ];
	waveStart = WaveStartTable[ waveForm ] << WAVE_SH;
	waveMask = WaveMaskTable[ waveForm ];
#endif
}

INLINE void Operator::SetState( Bit8u s ) {
	state = s;
	volHandler = VolumeHandlerTable[ s ];
}

INLINE bool Operator::Silent() const {
	if ( !ENV_SILENT( totalLevel + volume ) )
		return false;
	if ( !(rateZero & ( 1 << state ) ) )
		return false;
	return true;
}

INLINE void Operator::Prepare( const Chip* chip )  {
	currentLevel = totalLevel + (chip->tremoloValue & tremoloMask);
	waveCurrent = waveAdd;
	if ( vibStrength >> chip->vibratoShift ) {
		Bit32s add = vibrato >> chip->vibratoShift;
		//Sign extend over the shift value
		Bit32s neg = chip->vibratoSign;
		//Negate the add with -1 or 0
		add = ( add ^ neg ) - neg;
		waveCurrent += add;
	}
}

void Operator::KeyOn( Bit8u mask ) {
	if ( !keyOn ) {
		//Restart the frequency generator
#if ( DBOPL_WAVE > WAVE_HANDLER )
		waveIndex = waveStart;
#else
		waveIndex = 0;
#endif
		rateIndex = 0;
		SetState( ATTACK );
	}
	keyOn |= mask;
}

void Operator::KeyOff( Bit8u mask ) {
	keyOn &= ~mask;
	if ( !keyOn ) {
		if ( state != OFF ) {
			SetState( RELEASE );
		}
	}
}

INLINE Bits Operator::GetWave( Bitu index, Bitu vol ) {
#if ( DBOPL_WAVE == WAVE_HANDLER )
	return waveHandler( index, vol << ( 3 - ENV_EXTRA ) );
#elif ( DBOPL_WAVE == WAVE_TABLEMUL )
	return (waveBase[ index & waveMask ] * MulTable[ vol >> ENV_EXTRA ]) >> MUL_SH;
#elif ( DBOPL_WAVE == WAVE_TABLELOG )
	Bit32s wave = waveBase[ index & waveMask ];
	Bit32u total = ( wave & 0x7fff ) + vol << ( 3 - ENV_EXTRA );
	Bit32s sig = ExpTable[ total & 0xff ];
	Bit32u exp = total >> 8;
	Bit32s neg = wave >> 16;
	return ((sig ^ neg) - neg) >> exp;
#else
#error "No valid wave routine"
#endif
}

INLINE Bits Operator::GetSample( Bits modulation ) {
	Bitu vol = ForwardVolume();
	if ( ENV_SILENT( vol ) ) {
		//Simply forward the wave
		waveIndex += waveCurrent;
		return 0;
	} else {
		Bitu index = ForwardWave();
		index += modulation;
		return GetWave( index, vol );
	}
}

Operator::Operator() {
	chanData = 0;
	freqMul = 0;
	waveIndex = 0;
	waveAdd = 0;
	waveCurrent = 0;
	keyOn = 0;
	ksr = 0;
	reg20 = 0;
	reg40 = 0;
	reg60 = 0;
	reg80 = 0;
	regE0 = 0;
	SetState( OFF );
	rateZero = (1 << OFF);
	sustainLevel = ENV_MAX;
	currentLevel = ENV_MAX;
	totalLevel = ENV_MAX;
	volume = ENV_MAX;
	releaseAdd = 0;
}

/*
	Channel
*/

Channel::Channel() {
	old[0] = old[1] = 0;
	chanData = 0;
	regB0 = 0;
	regC0 = 0;
	maskLeft = -1;
	maskRight = -1;
	feedback = 31;
	fourMask = 0;
	synthHandler = &Channel::BlockTemplate< sm2FM >;
}

void Channel::SetChanData( const Chip* chip, Bit32u data ) {
	Bit32u change = chanData ^ data;
	chanData = data;
	Op( 0 )->chanData = data;
	Op( 1 )->chanData = data;
	//Since a frequency update triggered this, always update frequency
	Op( 0 )->UpdateFrequency();
	Op( 1 )->UpdateFrequency();
	if ( change & ( 0xff << SHIFT_KSLBASE ) ) {
		Op( 0 )->UpdateAttenuation();
		Op( 1 )->UpdateAttenuation();
	}
	if ( change & ( 0xff << SHIFT_KEYCODE ) ) {
		Op( 0 )->UpdateRates( chip );
		Op( 1 )->UpdateRates( chip );
	}
}

void Channel::UpdateFrequency( const Chip* chip, Bit8u fourOp ) {
	//Extrace the frequency bits
	Bit32u data = chanData & 0xffff;
	Bit32u kslBase = KslTable[ data >> 6 ];
	Bit32u keyCode = ( data & 0x1c00) >> 9;
	if ( chip->reg08 & 0x40 ) {
		keyCode |= ( data & 0x100)>>8;	/* notesel == 1 */
	} else {
		keyCode |= ( data & 0x200)>>9;	/* notesel == 0 */
	}
	//Add the keycode and ksl into the highest bits of chanData
	data |= (keyCode << SHIFT_KEYCODE) | ( kslBase << SHIFT_KSLBASE );
	( this + 0 )->SetChanData( chip, data );
	if ( fourOp & 0x3f ) {
		( this + 1 )->SetChanData( chip, data );
	}
}

void Channel::WriteA0( const Chip* chip, Bit8u val ) {
	Bit8u fourOp = chip->reg104 & chip->opl3Active & fourMask;
	//Don't handle writes to silent fourop channels
	if ( fourOp > 0x80 )
		return;
	Bit32u change = (chanData ^ val ) & 0xff;
	if ( change ) {
		chanData ^= change;
		UpdateFrequency( chip, fourOp );
	}
}

void Channel::WriteB0( const Chip* chip, Bit8u val ) {
	Bit8u fourOp = chip->reg104 & chip->opl3Active & fourMask;
	//Don't handle writes to silent fourop channels
	if ( fourOp > 0x80 )
		return;
	Bitu change = (chanData ^ ( val << 8 ) ) & 0x1f00;
	if ( change ) {
		chanData ^= change;
		UpdateFrequency( chip, fourOp );
	}
	//Check for a change in the keyon/off state
	if ( !(( val ^ regB0) & 0x20))
		return;
	regB0 = val;
	if ( val & 0x20 ) {
		Op(0)->KeyOn( 0x1 );
		Op(1)->KeyOn( 0x1 );
		if ( fourOp & 0x3f ) {
			( this + 1 )->Op(0)->KeyOn( 1 );
			( this + 1 )->Op(1)->KeyOn( 1 );
		}
	} else {
		Op(0)->KeyOff( 0x1 );
		Op(1)->KeyOff( 0x1 );
		if ( fourOp & 0x3f ) {
			( this + 1 )->Op(0)->KeyOff( 1 );
			( this + 1 )->Op(1)->KeyOff( 1 );
		}
	}
}

void Channel::WriteC0( const Chip* chip, Bit8u val ) {
	Bit8u change = val ^ regC0;
	if ( !change )
		return;
	regC0 = val;
	feedback = ( val >> 1 ) & 7;
	if ( feedback ) {
		//We shift the input to the right 10 bit wave index value
		feedback = 9 - feedback;
	} else {
		feedback = 31;
	}
	//Select the new synth mode
	if ( chip->opl3Active ) {
		//4-op mode enabled for this channel
		if ( (chip->reg104 & fourMask) & 0x3f ) {
			Channel* chan0, *chan1;
			//Check if it's the 2nd channel in a 4-op
			if ( !(fourMask & 0x80 ) ) {
				chan0 = this;
				chan1 = this + 1;
			} else {
				chan0 = this - 1;
				chan1 = this;
			}

			Bit8u synth = ( (chan0->regC0 & 1) << 0 )| (( chan1->regC0 & 1) << 1 );
			switch ( synth ) {
			case 0:
				chan0->synthHandler = &Channel::BlockTemplate< sm3FMFM >;
				break;
			case 1:
				chan0->synthHandler = &Channel::BlockTemplate< sm3AMFM >;
				break;
			case 2:
				chan0->synthHandler = &Channel::BlockTemplate< sm3FMAM >;
				break;
			case 3:
				chan0->synthHandler = &Channel::BlockTemplate< sm3AMAM >;
				break;
			}
		//Disable updating percussion channels
		} else if ((fourMask & 0x40) && ( chip->regBD & 0x20) ) {

		//Regular dual op, am or fm
		} else if ( val & 1 ) {
			synthHandler = &Channel::BlockTemplate< sm3AM >;
		} else {
			synthHandler = &Channel::BlockTemplate< sm3FM >;
		}
		maskLeft = ( val & 0x10 ) ? -1 : 0;
		maskRight = ( val & 0x20 ) ? -1 : 0;
	//opl2 active
	} else {
		//Disable updating percussion channels
		if ( (fourMask & 0x40) && ( chip->regBD & 0x20 ) ) {

		//Regular dual op, am or fm
		} else if ( val & 1 ) {
			synthHandler = &Channel::BlockTemplate< sm2AM >;
		} else {
			synthHandler = &Channel::BlockTemplate< sm2FM >;
		}
	}
}

void Channel::ResetC0( const Chip* chip ) {
	Bit8u val = regC0;
	regC0 ^= 0xff;
	WriteC0( chip, val );
}

template< bool opl3Mode>
INLINE void Channel::GeneratePercussion( Chip* chip, Bit32s* output ) {
	Channel* chan = this;

	//BassDrum
	Bit32s mod = (Bit32u)((old[0] + old[1])) >> feedback;
	old[0] = old[1];
	old[1] = Op(0)->GetSample( mod );

	//When bassdrum is in AM mode first operator is ignoed
	if ( chan->regC0 & 1 ) {
		mod = 0;
	} else {
		mod = old[0];
	}
	Bit32s sample = Op(1)->GetSample( mod );


	//Precalculate stuff used by other outputs
	Bit32u noiseBit = chip->ForwardNoise() & 0x1;
	Bit32u c2 = Op(2)->ForwardWave();
	Bit32u c5 = Op(5)->ForwardWave();
	Bit32u phaseBit = (((c2 & 0x88) ^ ((c2<<5) & 0x80)) | ((c5 ^ (c5<<2)) & 0x20)) ? 0x02 : 0x00;

	//Hi-Hat
	Bit32u hhVol = Op(2)->ForwardVolume();
	if ( !ENV_SILENT( hhVol ) ) {
		Bit32u hhIndex = (phaseBit<<8) | (0x34 << ( phaseBit ^ (noiseBit << 1 )));
		sample += Op(2)->GetWave( hhIndex, hhVol );
	}
	//Snare Drum
	Bit32u sdVol = Op(3)->ForwardVolume();
	if ( !ENV_SILENT( sdVol ) ) {
		Bit32u sdIndex = ( 0x100 + (c2 & 0x100) ) ^ ( noiseBit << 8 );
		sample += Op(3)->GetWave( sdIndex, sdVol );
	}
	//Tom-tom
	sample += Op(4)->GetSample( 0 );

	//Top-Cymbal
	Bit32u tcVol = Op(5)->ForwardVolume();
	if ( !ENV_SILENT( tcVol ) ) {
		Bit32u tcIndex = (1 + phaseBit) << 8;
		sample += Op(5)->GetWave( tcIndex, tcVol );
	}
	sample <<= 1;
	if ( opl3Mode ) {
		output[0] += sample;
		output[1] += sample;
	} else {
		output[0] += sample;
	}
}

template<SynthMode mode>
Channel* Channel::BlockTemplate( Chip* chip, Bit32u samples, Bit32s* output ) {
	switch( mode ) {
	case sm2AM:
	case sm3AM:
		if ( Op(0)->Silent() && Op(1)->Silent() ) {
			old[0] = old[1] = 0;
			return (this + 1);
		}
		break;
	case sm2FM:
	case sm3FM:
		if ( Op(1)->Silent() ) {
			old[0] = old[1] = 0;
			return (this + 1);
		}
		break;
	case sm3FMFM:
		if ( Op(3)->Silent() ) {
			old[0] = old[1] = 0;
			return (this + 2);
		}
		break;
	case sm3AMFM:
		if ( Op(0)->Silent() && Op(3)->Silent() ) {
			old[0] = old[1] = 0;
			return (this + 2);
		}
		break;
	case sm3FMAM:
		if ( Op(1)->Silent() && Op(3)->Silent() ) {
			old[0] = old[1] = 0;
			return (this + 2);
		}
		break;
	case sm3AMAM:
		if ( Op(0)->Silent() && Op(2)->Silent() && Op(3)->Silent() ) {
			old[0] = old[1] = 0;
			return (this + 2);
		}
		break;
	case sm2Percussion:
		// This case was not handled in the DOSBox code either
		// thus we leave this blank.
		// TODO: Consider checking this.
		break;
	case sm3Percussion:
		// This case was not handled in the DOSBox code either
		// thus we leave this blank.
		// TODO: Consider checking this.
		break;
	case sm4Start:
		// This case was not handled in the DOSBox code either
		// thus we leave this blank.
		// TODO: Consider checking this.
		break;
	case sm6Start:
		// This case was not handled in the DOSBox code either
		// thus we leave this blank.
		// TODO: Consider checking this.
		break;
	}
	//Init the operators with the the current vibrato and tremolo values
	Op( 0 )->Prepare( chip );
	Op( 1 )->Prepare( chip );
	if ( mode > sm4Start ) {
		Op( 2 )->Prepare( chip );
		Op( 3 )->Prepare( chip );
	}
	if ( mode > sm6Start ) {
		Op( 4 )->Prepare( chip );
		Op( 5 )->Prepare( chip );
	}
	for ( Bitu i = 0; i < samples; i++ ) {
		//Early out for percussion handlers
		if ( mode == sm2Percussion ) {
			GeneratePercussion<false>( chip, output + i );
			continue;	//Prevent some unitialized value bitching
		} else if ( mode == sm3Percussion ) {
			GeneratePercussion<true>( chip, output + i * 2 );
			continue;	//Prevent some unitialized value bitching
		}

		//Do unsigned shift so we can shift out all bits but still stay in 10 bit range otherwise
		Bit32s mod = (Bit32u)((old[0] + old[1])) >> feedback;
		old[0] = old[1];
		old[1] = Op(0)->GetSample( mod );
		Bit32s sample;
		Bit32s out0 = old[0];
		if ( mode == sm2AM || mode == sm3AM ) {
			sample = out0 + Op(1)->GetSample( 0 );
		} else if ( mode == sm2FM || mode == sm3FM ) {
			sample = Op(1)->GetSample( out0 );
		} else if ( mode == sm3FMFM ) {
			Bits next = Op(1)->GetSample( out0 );
			next = Op(2)->GetSample( next );
			sample = Op(3)->GetSample( next );
		} else if ( mode == sm3AMFM ) {
			sample = out0;
			Bits next = Op(1)->GetSample( 0 );
			next = Op(2)->GetSample( next );
			sample += Op(3)->GetSample( next );
		} else if ( mode == sm3FMAM ) {
			sample = Op(1)->GetSample( out0 );
			Bits next = Op(2)->GetSample( 0 );
			sample += Op(3)->GetSample( next );
		} else if ( mode == sm3AMAM ) {
			sample = out0;
			Bits next = Op(1)->GetSample( 0 );
			sample += Op(2)->GetSample( next );
			sample += Op(3)->GetSample( 0 );
		}
		switch( mode ) {
		case sm2AM:
		case sm2FM:
			output[ i ] += sample;
			break;
		case sm3AM:
		case sm3FM:
		case sm3FMFM:
		case sm3AMFM:
		case sm3FMAM:
		case sm3AMAM:
			output[ i * 2 + 0 ] += sample & maskLeft;
			output[ i * 2 + 1 ] += sample & maskRight;
			break;
		case sm2Percussion:
			// This case was not handled in the DOSBox code either
			// thus we leave this blank.
			// TODO: Consider checking this.
			break;
		case sm3Percussion:
			// This case was not handled in the DOSBox code either
			// thus we leave this blank.
			// TODO: Consider checking this.
			break;
		case sm4Start:
			// This case was not handled in the DOSBox code either
			// thus we leave this blank.
			// TODO: Consider checking this.
			break;
		case sm6Start:
			// This case was not handled in the DOSBox code either
			// thus we leave this blank.
			// TODO: Consider checking this.
			break;
		}
	}
	switch( mode ) {
	case sm2AM:
	case sm2FM:
	case sm3AM:
	case sm3FM:
		return ( this + 1 );
	case sm3FMFM:
	case sm3AMFM:
	case sm3FMAM:
	case sm3AMAM:
		return( this + 2 );
	case sm2Percussion:
	case sm3Percussion:
		return( this + 3 );
	case sm4Start:
		// This case was not handled in the DOSBox code either
		// thus we leave this blank.
		// TODO: Consider checking this.
		break;
	case sm6Start:
		// This case was not handled in the DOSBox code either
		// thus we leave this blank.
		// TODO: Consider checking this.
		break;
	}
	return 0;
}

/*
	Chip
*/

Chip::Chip() {
	reg08 = 0;
	reg04 = 0;
	regBD = 0;
	reg104 = 0;
	opl3Active = 0;
}

INLINE Bit32u Chip::ForwardNoise() {
	noiseCounter += noiseAdd;
	Bitu count = noiseCounter >> LFO_SH;
	noiseCounter &= WAVE_MASK;
	for ( ; count > 0; --count ) {
		//Noise calculation from mame
		noiseValue ^= ( 0x800302 ) & ( 0 - (noiseValue & 1 ) );
		noiseValue >>= 1;
	}
	return noiseValue;
}

INLINE Bit32u Chip::ForwardLFO( Bit32u samples ) {
	//Current vibrato value, runs 4x slower than tremolo
	vibratoSign = ( VibratoTable[ vibratoIndex >> 2] ) >> 7;
	vibratoShift = ( VibratoTable[ vibratoIndex >> 2] & 7) + vibratoStrength;
	tremoloValue = TremoloTable[ tremoloIndex ] >> tremoloStrength;

	//Check hom many samples there can be done before the value changes
	Bit32u todo = LFO_MAX - lfoCounter;
	Bit32u count = (todo + lfoAdd - 1) / lfoAdd;
	if ( count > samples ) {
		count = samples;
		lfoCounter += count * lfoAdd;
	} else {
		lfoCounter += count * lfoAdd;
		lfoCounter &= (LFO_MAX - 1);
		//Maximum of 7 vibrato value * 4
		vibratoIndex = ( vibratoIndex + 1 ) & 31;
		//Clip tremolo to the the table size
		if ( tremoloIndex + 1 < TREMOLO_TABLE  )
			++tremoloIndex;
		else
			tremoloIndex = 0;
	}
	return count;
}


void Chip::WriteBD( Bit8u val ) {
	Bit8u change = regBD ^ val;
	if ( !change )
		return;
	regBD = val;
	//TODO could do this with shift and xor?
	vibratoStrength = (val & 0x40) ? 0x00 : 0x01;
	tremoloStrength = (val & 0x80) ? 0x00 : 0x02;
	if ( val & 0x20 ) {
		//Drum was just enabled, make sure channel 6 has the right synth
		if ( change & 0x20 ) {
			if ( opl3Active ) {
				chan[6].synthHandler = &Channel::BlockTemplate< sm3Percussion >;
			} else {
				chan[6].synthHandler = &Channel::BlockTemplate< sm2Percussion >;
			}
		}
		//Bass Drum
		if ( val & 0x10 ) {
			chan[6].op[0].KeyOn( 0x2 );
			chan[6].op[1].KeyOn( 0x2 );
		} else {
			chan[6].op[0].KeyOff( 0x2 );
			chan[6].op[1].KeyOff( 0x2 );
		}
		//Hi-Hat
		if ( val & 0x1 ) {
			chan[7].op[0].KeyOn( 0x2 );
		} else {
			chan[7].op[0].KeyOff( 0x2 );
		}
		//Snare
		if ( val & 0x8 ) {
			chan[7].op[1].KeyOn( 0x2 );
		} else {
			chan[7].op[1].KeyOff( 0x2 );
		}
		//Tom-Tom
		if ( val & 0x4 ) {
			chan[8].op[0].KeyOn( 0x2 );
		} else {
			chan[8].op[0].KeyOff( 0x2 );
		}
		//Top Cymbal
		if ( val & 0x2 ) {
			chan[8].op[1].KeyOn( 0x2 );
		} else {
			chan[8].op[1].KeyOff( 0x2 );
		}
	//Toggle keyoffs when we turn off the percussion
	} else if ( change & 0x20 ) {
		//Trigger a reset to setup the original synth handler
		chan[6].ResetC0( this );
		chan[6].op[0].KeyOff( 0x2 );
		chan[6].op[1].KeyOff( 0x2 );
		chan[7].op[0].KeyOff( 0x2 );
		chan[7].op[1].KeyOff( 0x2 );
		chan[8].op[0].KeyOff( 0x2 );
		chan[8].op[1].KeyOff( 0x2 );
	}
}


#define REGOP( _FUNC_ )															\
	index = ( ( reg >> 3) & 0x20 ) | ( reg & 0x1f );								\
	if ( OpOffsetTable[ index ] ) {													\
		Operator* regOp = (Operator *)( ((char *)this ) + OpOffsetTable[ index ] );	\
		regOp->_FUNC_( this, val );													\
	}

#define REGCHAN( _FUNC_ )																\
	index = ( ( reg >> 4) & 0x10 ) | ( reg & 0xf );										\
	if ( ChanOffsetTable[ index ] ) {													\
		Channel* regChan = (Channel *)( ((char *)this ) + ChanOffsetTable[ index ] );	\
		regChan->_FUNC_( this, val );													\
	}

void Chip::WriteReg( Bit32u reg, Bit8u val ) {
	Bitu index;
	switch ( (reg & 0xf0) >> 4 ) {
	case 0x00 >> 4:
		if ( reg == 0x01 ) {
			waveFormMask = ( val & 0x20 ) ? 0x7 : 0x0;
		} else if ( reg == 0x104 ) {
			//Only detect changes in lowest 6 bits
			if ( !((reg104 ^ val) & 0x3f) )
				return;
			//Always keep the highest bit enabled, for checking > 0x80
			reg104 = 0x80 | ( val & 0x3f );
		} else if ( reg == 0x105 ) {
			//MAME says the real opl3 doesn't reset anything on opl3 disable/enable till the next write in another register
			if ( !((opl3Active ^ val) & 1 ) )
				return;
			opl3Active = ( val & 1 ) ? 0xff : 0;
			//Update the 0xc0 register for all channels to signal the switch to mono/stereo handlers
			for ( int i = 0; i < 18;i++ ) {
				chan[i].ResetC0( this );
			}
		} else if ( reg == 0x08 ) {
			reg08 = val;
		}
	case 0x10 >> 4:
		break;
	case 0x20 >> 4:
	case 0x30 >> 4:
		REGOP( Write20 );
		break;
	case 0x40 >> 4:
	case 0x50 >> 4:
		REGOP( Write40 );
		break;
	case 0x60 >> 4:
	case 0x70 >> 4:
		REGOP( Write60 );
		break;
	case 0x80 >> 4:
	case 0x90 >> 4:
		REGOP( Write80 );
		break;
	case 0xa0 >> 4:
		REGCHAN( WriteA0 );
		break;
	case 0xb0 >> 4:
		if ( reg == 0xbd ) {
			WriteBD( val );
		} else {
			REGCHAN( WriteB0 );
		}
		break;
	case 0xc0 >> 4:
		REGCHAN( WriteC0 );
	case 0xd0 >> 4:
		break;
	case 0xe0 >> 4:
	case 0xf0 >> 4:
		REGOP( WriteE0 );
		break;
	}
}


Bit32u Chip::WriteAddr( Bit32u port, Bit8u val ) {
	switch ( port & 3 ) {
	case 0:
		return val;
	case 2:
		if ( opl3Active || (val == 0x05) )
			return 0x100 | val;
		else
			return val;
	}
	return 0;
}

void Chip::GenerateBlock2( Bitu total, Bit32s* output ) {
	while ( total > 0 ) {
		Bit32u samples = ForwardLFO( total );
		memset(output, 0, sizeof(Bit32s) * samples);
		int count = 0;
		for( Channel* ch = chan; ch < chan + 9; ) {
			count++;
			ch = (ch->*(ch->synthHandler))( this, samples, output );
		}
		total -= samples;
		output += samples;
	}
}

void Chip::GenerateBlock3( Bitu total, Bit32s* output  ) {
	while ( total > 0 ) {
		Bit32u samples = ForwardLFO( total );
		memset(output, 0, sizeof(Bit32s) * samples * 2);
		int count = 0;
		for( Channel* ch = chan; ch < chan + 18; ) {
			count++;
			ch = (ch->*(ch->synthHandler))( this, samples, output );
		}
		total -= samples;
		output += samples * 2;
	}
}

void Chip::Setup( Bit32u rate ) {
	double scale = OPLRATE / (double)rate;

	//Noise counter is run at the same precision as general waves
	noiseAdd = (Bit32u)( 0.5 + scale * ( 1 << LFO_SH ) );
	noiseCounter = 0;
	noiseValue = 1;	//Make sure it triggers the noise xor the first time
	//The low frequency oscillation counter
	//Every time his overflows vibrato and tremoloindex are increased
	lfoAdd = (Bit32u)( 0.5 + scale * ( 1 << LFO_SH ) );
	lfoCounter = 0;
	vibratoIndex = 0;
	tremoloIndex = 0;

	//With higher octave this gets shifted up
	//-1 since the freqCreateTable = *2
#ifdef WAVE_PRECISION
	double freqScale = ( 1 << 7 ) * scale * ( 1 << ( WAVE_SH - 1 - 10));
	for ( int i = 0; i < 16; i++ ) {
		freqMul[i] = (Bit32u)( 0.5 + freqScale * FreqCreateTable[ i ] );
	}
#else
	Bit32u freqScale = (Bit32u)( 0.5 + scale * ( 1 << ( WAVE_SH - 1 - 10)));
	for ( int i = 0; i < 16; i++ ) {
		freqMul[i] = freqScale * FreqCreateTable[ i ];
	}
#endif

	//-3 since the real envelope takes 8 steps to reach the single value we supply
	for ( Bit8u i = 0; i < 76; i++ ) {
		Bit8u index, shift;
		EnvelopeSelect( i, index, shift );
		linearRates[i] = (Bit32u)( scale * (EnvelopeIncreaseTable[ index ] << ( RATE_SH + ENV_EXTRA - shift - 3 )));
	}
	//Generate the best matching attack rate
	for ( Bit8u i = 0; i < 62; i++ ) {
		Bit8u index, shift;
		EnvelopeSelect( i, index, shift );
		//Original amount of samples the attack would take
		Bit32s original = (Bit32u)( (AttackSamplesTable[ index ] << shift) / scale);

		Bit32s guessAdd = (Bit32u)( scale * (EnvelopeIncreaseTable[ index ] << ( RATE_SH - shift - 3 )));
		Bit32s bestAdd = guessAdd;
		Bit32u bestDiff = 1 << 30;
		for( Bit32u passes = 0; passes < 16; passes ++ ) {
			Bit32s volume = ENV_MAX;
			Bit32s samples = 0;
			Bit32u count = 0;
			while ( volume > 0 && samples < original * 2 ) {
				count += guessAdd;
				Bit32s change = count >> RATE_SH;
				count &= RATE_MASK;
				if ( GCC_UNLIKELY(change) ) { // less than 1 %
					volume += ( ~volume * change ) >> 3;
				}
				samples++;

			}
			Bit32s diff = original - samples;
			Bit32u lDiff = labs( diff );
			//Init last on first pass
			if ( lDiff < bestDiff ) {
				bestDiff = lDiff;
				bestAdd = guessAdd;
				if ( !bestDiff )
					break;
			}
			//Below our target
			if ( diff < 0 ) {
				//Better than the last time
				Bit32s mul = ((original - diff) << 12) / original;
				guessAdd = ((guessAdd * mul) >> 12);
				guessAdd++;
			} else if ( diff > 0 ) {
				Bit32s mul = ((original - diff) << 12) / original;
				guessAdd = (guessAdd * mul) >> 12;
				guessAdd--;
			}
		}
		attackRates[i] = bestAdd;
	}
	for ( Bit8u i = 62; i < 76; i++ ) {
		//This should provide instant volume maximizing
		attackRates[i] = 8 << RATE_SH;
	}
	//Setup the channels with the correct four op flags
	//Channels are accessed through a table so they appear linear here
	chan[ 0].fourMask = 0x00 | ( 1 << 0 );
	chan[ 1].fourMask = 0x80 | ( 1 << 0 );
	chan[ 2].fourMask = 0x00 | ( 1 << 1 );
	chan[ 3].fourMask = 0x80 | ( 1 << 1 );
	chan[ 4].fourMask = 0x00 | ( 1 << 2 );
	chan[ 5].fourMask = 0x80 | ( 1 << 2 );

	chan[ 9].fourMask = 0x00 | ( 1 << 3 );
	chan[10].fourMask = 0x80 | ( 1 << 3 );
	chan[11].fourMask = 0x00 | ( 1 << 4 );
	chan[12].fourMask = 0x80 | ( 1 << 4 );
	chan[13].fourMask = 0x00 | ( 1 << 5 );
	chan[14].fourMask = 0x80 | ( 1 << 5 );

	//mark the percussion channels
	chan[ 6].fourMask = 0x40;
	chan[ 7].fourMask = 0x40;
	chan[ 8].fourMask = 0x40;

	//Clear Everything in opl3 mode
	WriteReg( 0x105, 0x1 );
	for ( int i = 0; i < 512; i++ ) {
		if ( i == 0x105 )
			continue;
		WriteReg( i, 0xff );
		WriteReg( i, 0x0 );
	}
	WriteReg( 0x105, 0x0 );
	//Clear everything in opl2 mode
	for ( int i = 0; i < 255; i++ ) {
		WriteReg( i, 0xff );
		WriteReg( i, 0x0 );
	}
}

static bool doneTables = false;
void InitTables( void ) {
	if ( doneTables )
		return;
	doneTables = true;
#if ( DBOPL_WAVE == WAVE_HANDLER ) || ( DBOPL_WAVE == WAVE_TABLELOG )
	//Exponential volume table, same as the real adlib
	for ( int i = 0; i < 256; i++ ) {
		//Save them in reverse
		ExpTable[i] = (int)( 0.5 + ( pow(2.0, ( 255 - i) * ( 1.0 /256 ) )-1) * 1024 );
		ExpTable[i] += 1024; //or remove the -1 oh well :)
		//Preshift to the left once so the final volume can shift to the right
		ExpTable[i] *= 2;
	}
#endif
#if ( DBOPL_WAVE == WAVE_HANDLER )
	//Add 0.5 for the trunc rounding of the integer cast
	//Do a PI sinetable instead of the original 0.5 PI
	for ( int i = 0; i < 512; i++ ) {
		SinTable[i] = (Bit16s)( 0.5 - log10( sin( (i + 0.5) * (M_PI / 512.0) ) ) / log10(2.0)*256 );
	}
#endif
#if ( DBOPL_WAVE == WAVE_TABLEMUL )
	//Multiplication based tables
	for ( int i = 0; i < 384; i++ ) {
		int s = i * 8;
		//TODO maybe keep some of the precision errors of the original table?
		double val = ( 0.5 + ( pow(2.0, -1.0 + ( 255 - s) * ( 1.0 /256 ) )) * ( 1 << MUL_SH ));
		MulTable[i] = (Bit16u)(val);
	}

	//Sine Wave Base
	for ( int i = 0; i < 512; i++ ) {
		WaveTable[ 0x0200 + i ] = (Bit16s)(sin( (i + 0.5) * (M_PI / 512.0) ) * 4084);
		WaveTable[ 0x0000 + i ] = -WaveTable[ 0x200 + i ];
	}
	//Exponential wave
	for ( int i = 0; i < 256; i++ ) {
		WaveTable[ 0x700 + i ] = (Bit16s)( 0.5 + ( pow(2.0, -1.0 + ( 255 - i * 8) * ( 1.0 /256 ) ) ) * 4085 );
		WaveTable[ 0x6ff - i ] = -WaveTable[ 0x700 + i ];
	}
#endif
#if ( DBOPL_WAVE == WAVE_TABLELOG )
	//Sine Wave Base
	for ( int i = 0; i < 512; i++ ) {
		WaveTable[ 0x0200 + i ] = (Bit16s)( 0.5 - log10( sin( (i + 0.5) * (M_PI / 512.0) ) ) / log10(2.0)*256 );
		WaveTable[ 0x0000 + i ] = ((Bit16s)0x8000) | WaveTable[ 0x200 + i];
	}
	//Exponential wave
	for ( int i = 0; i < 256; i++ ) {
		WaveTable[ 0x700 + i ] = i * 8;
		WaveTable[ 0x6ff - i ] = ((Bit16s)0x8000) | i * 8;
	}
#endif

	//	|    |//\\|____|WAV7|//__|/\  |____|/\/\|
	//	|\\//|    |    |WAV7|    |  \/|    |    |
	//	|06  |0126|27  |7   |3   |4   |4 5 |5   |

#if (( DBOPL_WAVE == WAVE_TABLELOG ) || ( DBOPL_WAVE == WAVE_TABLEMUL ))
	for ( int i = 0; i < 256; i++ ) {
		//Fill silence gaps
		WaveTable[ 0x400 + i ] = WaveTable[0];
		WaveTable[ 0x500 + i ] = WaveTable[0];
		WaveTable[ 0x900 + i ] = WaveTable[0];
		WaveTable[ 0xc00 + i ] = WaveTable[0];
		WaveTable[ 0xd00 + i ] = WaveTable[0];
		//Replicate sines in other pieces
		WaveTable[ 0x800 + i ] = WaveTable[ 0x200 + i ];
		//double speed sines
		WaveTable[ 0xa00 + i ] = WaveTable[ 0x200 + i * 2 ];
		WaveTable[ 0xb00 + i ] = WaveTable[ 0x000 + i * 2 ];
		WaveTable[ 0xe00 + i ] = WaveTable[ 0x200 + i * 2 ];
		WaveTable[ 0xf00 + i ] = WaveTable[ 0x200 + i * 2 ];
	}
#endif

	//Create the ksl table
	for ( int oct = 0; oct < 8; oct++ ) {
		int base = oct * 8;
		for ( int i = 0; i < 16; i++ ) {
			int val = base - KslCreateTable[i];
			if ( val < 0 )
				val = 0;
			//*4 for the final range to match attenuation range
			KslTable[ oct * 16 + i ] = val * 4;
		}
	}
	//Create the Tremolo table, just increase and decrease a triangle wave
	for ( Bit8u i = 0; i < TREMOLO_TABLE / 2; i++ ) {
		Bit8u val = i << ENV_EXTRA;
		TremoloTable[i] = val;
		TremoloTable[TREMOLO_TABLE - 1 - i] = val;
	}
	//Create a table with offsets of the channels from the start of the chip
	DBOPL::Chip* chip = 0;
	for ( Bitu i = 0; i < 32; i++ ) {
		Bitu index = i & 0xf;
		if ( index >= 9 ) {
			ChanOffsetTable[i] = 0;
			continue;
		}
		//Make sure the four op channels follow eachother
		if ( index < 6 ) {
			index = (index % 3) * 2 + ( index / 3 );
		}
		//Add back the bits for highest ones
		if ( i >= 16 )
			index += 9;
		Bitu blah = reinterpret_cast<size_t>( &(chip->chan[ index ]) );
		ChanOffsetTable[i] = blah;
	}
	//Same for operators
	for ( Bitu i = 0; i < 64; i++ ) {
		if ( i % 8 >= 6 || ( (i / 8) % 4 == 3 ) ) {
			OpOffsetTable[i] = 0;
			continue;
		}
		Bitu chNum = (i / 8) * 3 + (i % 8) % 3;
		//Make sure we use 16 and up for the 2nd range to match the chanoffset gap
		if ( chNum >= 12 )
			chNum += 16 - 12;
		Bitu opNum = ( i % 8 ) / 3;
		DBOPL::Channel* chan = 0;
		Bitu blah = reinterpret_cast<size_t>( &(chan->op[opNum]) );
		OpOffsetTable[i] = ChanOffsetTable[ chNum ] + blah;
	}
#if 0
	//Stupid checks if table's are correct
	for ( Bitu i = 0; i < 18; i++ ) {
		Bit32u find = (Bit16u)( &(chip->chan[ i ]) );
		for ( Bitu c = 0; c < 32; c++ ) {
			if ( ChanOffsetTable[c] == find ) {
				find = 0;
				break;
			}
		}
		if ( find ) {
			find = find;
		}
	}
	for ( Bitu i = 0; i < 36; i++ ) {
		Bit32u find = (Bit16u)( &(chip->chan[ i / 2 ].op[i % 2]) );
		for ( Bitu c = 0; c < 64; c++ ) {
			if ( OpOffsetTable[c] == find ) {
				find = 0;
				break;
			}
		}
		if ( find ) {
			find = find;
		}
	}
#endif
}

}		//Namespace DBOPL
} // End of namespace DOSBox
} // End of namespace OPL

#endif // !DISABLE_DOSBOX_OPL
