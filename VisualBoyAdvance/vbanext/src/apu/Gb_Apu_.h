// Gb_Snd_Emu 0.2.0. http://www.slack.net/~ant/

#include "Gb_Apu.h"

#include "../System.h"

/* Copyright (C) 2003-2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

#include "blargg_source.h"

#define VOL_REG 0xFF24
#define STEREO_REG 0xFF25
#define STATUS_REG 0xFF26
#define WAVE_RAM 0xFF30
#define POWER_MASK 0x80

#define OSC_COUNT 4

INLINE int Gb_Apu::calc_output( int osc ) const
{
	int bits = regs [STEREO_REG - START_ADDR] >> osc;
	return (bits >> 3 & 2) | (bits & 1);
}

void Gb_Apu::set_output( Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right, int osc )
{
	int i;

	i = osc;
	do
	{
		Gb_Osc& o = *oscs [i];
		o.outputs [1] = right;
		o.outputs [2] = left;
		o.outputs [3] = center;
		o.output = o.outputs [calc_output( i )];
		++i;
	}
	while ( i < osc );
}

void Gb_Apu::synth_volume( int iv )
{
	double v = volume_ * 0.60 / OSC_COUNT / 15 /*steps*/ / 8 /*master vol range*/ * iv;
	good_synth.volume( v );
	med_synth .volume( v );
}

void Gb_Apu::apply_volume()
{
	int data, left, right, vol_tmp;
	data  = regs [VOL_REG - START_ADDR];
	left  = data >> 4 & 7;
	right = data & 7;
	vol_tmp = left < right ? right : left;
	synth_volume( vol_tmp + 1 );
}

void Gb_Apu::volume( double v )
{
	if ( volume_ != v )
	{
		volume_ = v;
		apply_volume();
	}
}

void Gb_Apu::reduce_clicks( bool reduce )
{
	reduce_clicks_ = reduce;

	/* Click reduction makes DAC off generate same output as volume 0*/
	int dac_off_amp = 0;
	if ( reduce && wave.mode != MODE_AGB ) /* AGB already eliminates clicks*/
		dac_off_amp = -DAC_BIAS;

	oscs [0]->dac_off_amp = dac_off_amp;
	oscs [1]->dac_off_amp = dac_off_amp;
	oscs [2]->dac_off_amp = dac_off_amp;
	oscs [3]->dac_off_amp = dac_off_amp;

	/* AGB always eliminates clicks on wave channel using same method*/
	if ( wave.mode == MODE_AGB )
		wave.dac_off_amp = -DAC_BIAS;
}

void Gb_Apu::reset( uint32_t mode, bool agb_wave )
{
	/* Hardware mode*/
	if ( agb_wave )
		mode = MODE_AGB; /* using AGB wave features implies AGB hardware*/
	wave.agb_mask = agb_wave ? 0xFF : 0;
	oscs [0]->mode = mode;
	oscs [1]->mode = mode;
	oscs [2]->mode = mode;
	oscs [3]->mode = mode;
	reduce_clicks( reduce_clicks_ );

	/* Reset state*/
	frame_time  = 0;
	last_time   = 0;
	frame_phase = 0;

	for ( int i = 0; i < 32; i++ )
		regs [i] = 0;

	square1.reset();
	square2.reset();
	wave   .reset();
	noise  .reset();

	apply_volume();

	square1.length_ctr = 64;
	square2.length_ctr = 64;
	wave   .length_ctr = 256;
	noise  .length_ctr = 64;

	/* Load initial wave RAM*/
	static unsigned char const initial_wave [2] [16] = {
		{0x84,0x40,0x43,0xAA,0x2D,0x78,0x92,0x3C,0x60,0x59,0x59,0xB0,0x34,0xB8,0x2E,0xDA},
		{0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF},
	};
	for ( int b = 2; --b >= 0; )
	{
		/* Init both banks (does nothing if not in AGB mode)*/
		write_register( 0, 0xFF1A, b * 0x40 );
		for ( unsigned i = 0; i < sizeof initial_wave [0]; i++ )
			write_register( 0, i + WAVE_RAM, initial_wave [(mode != MODE_DMG)] [i] );
	}
}

Gb_Apu::Gb_Apu()
{
	int i;

	wave.wave_ram = &regs [WAVE_RAM - START_ADDR];

	oscs [0] = &square1;
	oscs [1] = &square2;
	oscs [2] = &wave;
	oscs [3] = &noise;

	for ( i = OSC_COUNT; --i >= 0; )
	{
		Gb_Osc& o = *oscs [i];
		o.regs        = &regs [i * 5];
		o.output      = 0;
		o.outputs [0] = 0;
		o.outputs [1] = 0;
		o.outputs [2] = 0;
		o.outputs [3] = 0;
		o.good_synth  = &good_synth;
		o.med_synth   = &med_synth;
	}

	reduce_clicks_ = false;
	frame_period = 4194304 / 512; /* 512 Hz*/

	volume_ = 1.0;
	reset(MODE_CGB, false);
}

void Gb_Apu::run_until_( int32_t end_time )
{
	int32_t time;

	do{
		/* run oscillators*/
		time = end_time;
		if ( time > frame_time )
			time = frame_time;

		square1.run( last_time, time );
		square2.run( last_time, time );
		wave   .run( last_time, time );
		noise  .run( last_time, time );
		last_time = time;

		if ( time == end_time )
			break;

		/* run frame sequencer*/
		frame_time += frame_period * CLK_MUL;
		switch ( frame_phase++ )
		{
			case 2:
			case 6:
				/* 128 Hz*/
				square1.clock_sweep();
			case 0:
			case 4:
				/* 256 Hz*/
				square1.clock_length();
				square2.clock_length();
				wave   .clock_length();
				noise  .clock_length();
				break;

			case 7:
				/* 64 Hz*/
				frame_phase = 0;
				square1.clock_envelope();
				square2.clock_envelope();
				noise  .clock_envelope();
		}
	}while(1);
}

void Gb_Apu::silence_osc( Gb_Osc& o )
{
	int delta;

	delta = -o.last_amp;
	if ( delta )
	{
		o.last_amp = 0;
		if ( o.output )
		{
			med_synth.offset( last_time, delta, o.output );
		}
	}
}

void Gb_Apu::write_register( int32_t time, unsigned addr, int data )
{
	int reg = addr - START_ADDR;
	if ( (unsigned) reg >= REGISTER_COUNT )
		return;

	if ( addr < STATUS_REG && !(regs [STATUS_REG - START_ADDR] & POWER_MASK) )
	{
		/* Power is off*/

		/* length counters can only be written in DMG mode*/
		if ( wave.mode != MODE_DMG || (reg != 1 && reg != 5+1 && reg != 10+1 && reg != 15+1) )
			return;

		if ( reg < 10 )
			data &= 0x3F; /* clear square duty*/
	}

	if ( time > last_time )
		run_until_( time );

	if ( addr >= WAVE_RAM )
	{
		wave.write( addr, data );
	}
	else
	{
		int old_data = regs [reg];
		regs [reg] = data;

		if ( addr < VOL_REG )
		{
			/* Oscillator*/
			write_osc( reg / 5, reg, old_data, data );
		}
		else if ( addr == VOL_REG && data != old_data )
		{
			/* Master volume*/
			for ( int i = OSC_COUNT; --i >= 0; )
				silence_osc( *oscs [i] );

			apply_volume();
		}
		else if ( addr == STEREO_REG )
		{
			/* Stereo panning*/
			for ( int i = OSC_COUNT; --i >= 0; )
			{
				Gb_Osc& o = *oscs [i];
				Blip_Buffer* out = o.outputs [calc_output( i )];
				if ( o.output != out )
				{
					silence_osc( o );
					o.output = out;
				}
			} }
		else if ( addr == STATUS_REG && (data ^ old_data) & POWER_MASK )
		{
			/* Power control*/
			frame_phase = 0;
			for ( int i = OSC_COUNT; --i >= 0; )
				silence_osc( *oscs [i] );

			for ( int i = 0; i < 32; i++ )
				regs [i] = 0;

			square1.reset();
			square2.reset();
			wave   .reset();
			noise  .reset();

			apply_volume();

			if ( wave.mode != MODE_DMG )
			{
				square1.length_ctr = 64;
				square2.length_ctr = 64;
				wave   .length_ctr = 256;
				noise  .length_ctr = 64;
			}

			regs [STATUS_REG - START_ADDR] = data;
		}
	}
}

void Gb_Apu::apply_stereo()
{
	int i;

	for ( i = OSC_COUNT; --i >= 0; )
	{
		Gb_Osc& o = *oscs [i];
		Blip_Buffer* out = o.outputs [calc_output( i )];
		if ( o.output != out )
		{
			silence_osc( o );
			o.output = out;
		}
	}
}


int Gb_Apu::read_register( int32_t time, unsigned addr )
{
	int reg, mask, data;

	if ( time > last_time )
		run_until_( time );

	reg = addr - START_ADDR;
	if ( (unsigned) reg >= REGISTER_COUNT )
		return 0;

	if ( addr >= WAVE_RAM )
		return wave.read( addr );

	/* Value read back has some bits always set*/
	static unsigned char const masks [] = {
		0x80,0x3F,0x00,0xFF,0xBF,
		0xFF,0x3F,0x00,0xFF,0xBF,
		0x7F,0xFF,0x9F,0xFF,0xBF,
		0xFF,0xFF,0x00,0x00,0xBF,
		0x00,0x00,0x70,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
	};

	mask = masks [reg];

	if ( wave.agb_mask && (reg == 10 || reg == 12) )
		mask = 0x1F; /* extra implemented bits in wave regs on AGB*/

	data = regs [reg] | mask;

	/* Status register*/
	if ( addr == STATUS_REG )
	{
		data &= 0xF0;
		data |= (int) square1.enabled << 0;
		data |= (int) square2.enabled << 1;
		data |= (int) wave   .enabled << 2;
		data |= (int) noise  .enabled << 3;
	}

	return data;
}

#define REFLECT( x, y ) (save ?       (io->y) = (x) :         (x) = (io->y)          )

INLINE const char* Gb_Apu::save_load( gb_apu_state_t* io, bool save )
{
	int format, version;

	format = io->format0;

	REFLECT( format, format );
	if ( format != io->format0 )
		return "Unsupported sound save state format";

	version = 0;
	REFLECT( version, version );

	/* Registers and wave RAM*/
	if ( save )
		memcpy( io->regs, regs, sizeof io->regs );
	else
		memcpy( regs, io->regs, sizeof     regs );

	/* Frame sequencer*/
	REFLECT( frame_time,  frame_time  );
	REFLECT( frame_phase, frame_phase );

	REFLECT( square1.sweep_freq,    sweep_freq );
	REFLECT( square1.sweep_delay,   sweep_delay );
	REFLECT( square1.sweep_enabled, sweep_enabled );
	REFLECT( square1.sweep_neg,     sweep_neg );

	REFLECT( noise.divider,         noise_divider );
	REFLECT( wave.sample_buf,       wave_buf );

	return 0;
}

/* second function to avoid inline limits of some compilers*/
INLINE void Gb_Apu::save_load2( gb_apu_state_t* io, bool save )
{
	int i;
	for ( i = OSC_COUNT; --i >= 0; )
	{
		Gb_Osc& osc = *oscs [i];
		REFLECT( osc.delay,      delay      [i] );
		REFLECT( osc.length_ctr, length_ctr [i] );
		REFLECT( osc.phase,      phase      [i] );
		REFLECT( osc.enabled,    enabled    [i] );

		if ( i != 2 )
		{
			int j = 2 < i ? 2 : i;
			Gb_Env& env = STATIC_CAST(Gb_Env&,osc);
			REFLECT( env.env_delay,   env_delay   [j] );
			REFLECT( env.volume,      env_volume  [j] );
			REFLECT( env.env_enabled, env_enabled [j] );
		}
	}
}

void Gb_Apu::save_state( gb_apu_state_t* out )
{
	(void) save_load( out, true );
	save_load2( out, true );
}

const char * Gb_Apu::load_state( gb_apu_state_t const& in )
{
	RETURN_ERR( save_load( CONST_CAST(gb_apu_state_t*,&in), false));
	save_load2( CONST_CAST(gb_apu_state_t*,&in), false );

	apply_stereo();
	synth_volume( 0 );          /* suppress output for the moment*/
	run_until_( last_time );    /* get last_amp updated*/
	apply_volume();             /* now use correct volume*/

	return 0;
}
