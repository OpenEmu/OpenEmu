// Gb_Snd_Emu 0.1.5. http://www.slack.net/~ant/

#include "Gb_Apu.h"

// Avoid any macros which evaluate their arguments multiple times
#undef min
#undef max

// using const references generates crappy code, and I am currenly only using these
// for built-in types, so they take arguments by value

template<class T>
inline T min( T x, T y )
{
        if ( x < y )
                return x;
        return y;
}

template<class T>
inline T max( T x, T y )
{
        if ( x < y )
                return y;
        return x;
}

#include <string.h>

/* Copyright (C) 2003-2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

//#include "blargg_source.h"

unsigned const vol_reg    = 0xFF24;
unsigned const status_reg = 0xFF26;

Gb_Apu::Gb_Apu()
{
	square1.synth = &square_synth;
	square2.synth = &square_synth;
	wave.synth  = &other_synth;
	noise.synth = &other_synth;
	
	oscs [0] = &square1;
	oscs [1] = &square2;
	oscs [2] = &wave;
	oscs [3] = &noise;
	
	for ( int i = 0; i < osc_count; i++ )
	{
		Gb_Osc& osc = *oscs [i];
		osc.regs = &regs [i * 5];
		osc.output = 0;
		osc.outputs [0] = 0;
		osc.outputs [1] = 0;
		osc.outputs [2] = 0;
		osc.outputs [3] = 0;
	}
	
	set_tempo( 1.0 );
	volume( 1.0 );
	reset();
}

void Gb_Apu::treble_eq( const blip_eq_t& eq )
{
	square_synth.treble_eq( eq );
	other_synth.treble_eq( eq );
}

void Gb_Apu::osc_output( int index, Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right )
{
	require( (unsigned) index < osc_count );
	require( (center && left && right) || (!center && !left && !right) );
	Gb_Osc& osc = *oscs [index];
	osc.outputs [1] = right;
	osc.outputs [2] = left;
	osc.outputs [3] = center;
	osc.output = osc.outputs [osc.output_select];
}

void Gb_Apu::output( Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right )
{
	for ( int i = 0; i < osc_count; i++ )
		osc_output( i, center, left, right );
}

void Gb_Apu::update_volume()
{
	// TODO: doesn't handle differing left/right global volume (support would
	// require modification to all oscillator code)
	int data = regs [vol_reg - start_addr];
	double vol = (max( data & 7, data >> 4 & 7 ) + 1) * volume_unit;
	square_synth.volume( vol );
	other_synth.volume( vol );
}

static unsigned char const powerup_regs [0x20] = {
	0x80,0x3F,0x00,0xFF,0xBF, // square 1
	0xFF,0x3F,0x00,0xFF,0xBF, // square 2
	0x7F,0xFF,0x9F,0xFF,0xBF, // wave
	0xFF,0xFF,0x00,0x00,0xBF, // noise
	0x00, // left/right enables
	0x77, // master volume
	0x80, // power
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

void Gb_Apu::set_tempo( double t )
{
	frame_period = 4194304 / 256; // 256 Hz
	if ( t != 1.0 )
		frame_period = blip_time_t (frame_period / t);
}

void Gb_Apu::reset()
{
	next_frame_time = 0;
	last_time       = 0;
	frame_count     = 0;
	
	square1.reset();
	square2.reset();
	wave.reset();
	noise.reset();
	noise.bits = 1;
	wave.wave_pos = 0;
	
	// avoid click at beginning
	regs [vol_reg - start_addr] = 0x77;
	update_volume();
	
	regs [status_reg - start_addr] = 0x01; // force power
	write_register( 0, status_reg, 0x00 );
	
	static unsigned char const initial_wave [] = {
		0x84,0x40,0x43,0xAA,0x2D,0x78,0x92,0x3C, // wave table
		0x60,0x59,0x59,0xB0,0x34,0xB8,0x2E,0xDA
	};
	memcpy( wave.wave, initial_wave, sizeof wave.wave );
}

void Gb_Apu::run_until( blip_time_t end_time )
{
	require( end_time >= last_time ); // end_time must not be before previous time
	if ( end_time == last_time )
		return;
	
	while ( true )
	{
		blip_time_t time = next_frame_time;
		if ( time > end_time )
			time = end_time;
		
		// run oscillators
		for ( int i = 0; i < osc_count; ++i )
		{
			Gb_Osc& osc = *oscs [i];
			if ( osc.output )
			{
				//osc.output->set_modified(); // TODO: misses optimization opportunities?
				int playing = false;
				if ( osc.enabled && osc.volume &&
						(!(osc.regs [4] & osc.len_enabled_mask) || osc.length) )
					playing = -1;
				switch ( i )
				{
				case 0: square1.run( last_time, time, playing ); break;
				case 1: square2.run( last_time, time, playing ); break;
				case 2: wave   .run( last_time, time, playing ); break;
				case 3: noise  .run( last_time, time, playing ); break;
				}
			}
		}
		last_time = time;
		
		if ( time == end_time )
			break;
		
		next_frame_time += frame_period;
		
		// 256 Hz actions
		square1.clock_length();
		square2.clock_length();
		wave.clock_length();
		noise.clock_length();
		
		frame_count = (frame_count + 1) & 3;
		if ( frame_count == 0 )
		{
			// 64 Hz actions
			square1.clock_envelope();
			square2.clock_envelope();
			noise.clock_envelope();
		}
		
		if ( frame_count & 1 )
			square1.clock_sweep(); // 128 Hz action
	}
}

void Gb_Apu::end_frame( blip_time_t end_time )
{
	if ( end_time > last_time )
		run_until( end_time );
	
	assert( next_frame_time >= end_time );
	next_frame_time -= end_time;
	
	assert( last_time >= end_time );
	last_time -= end_time;
}

void Gb_Apu::write_register( blip_time_t time, unsigned addr, int data )
{
	require( (unsigned) data < 0x100 );
	
	int reg = addr - start_addr;
	if ( (unsigned) reg >= register_count )
		return;
	
	run_until( time );
	
	int old_reg = regs [reg];
	regs [reg] = data;
	
	if ( addr < vol_reg )
	{
		write_osc( reg / 5, reg, data );
	}
	else if ( addr == vol_reg && data != old_reg ) // global volume
	{
		// return all oscs to 0
		for ( int i = 0; i < osc_count; i++ )
		{
			Gb_Osc& osc = *oscs [i];
			int amp = osc.last_amp;
			osc.last_amp = 0;
			if ( amp && osc.enabled && osc.output )
				other_synth.offset( time, -amp, osc.output );
		}
		
		if ( wave.outputs [3] )
			other_synth.offset( time, 30, wave.outputs [3] );
		
		update_volume();
		
		if ( wave.outputs [3] )
			other_synth.offset( time, -30, wave.outputs [3] );
		
		// oscs will update with new amplitude when next run
	}
	else if ( addr == 0xFF25 || addr == status_reg )
	{
		int mask = (regs [status_reg - start_addr] & 0x80) ? ~0 : 0;
		int flags = regs [0xFF25 - start_addr] & mask;
		
		// left/right assignments
		for ( int i = 0; i < osc_count; i++ )
		{
			Gb_Osc& osc = *oscs [i];
			osc.enabled &= mask;
			int bits = flags >> i;

			Blip_Buffer* old_output = osc.output;
			osc.output_select = (bits >> 3 & 2) | (bits & 1);
			osc.output = osc.outputs [osc.output_select];
			if ( osc.output != old_output )
			{
				int amp = osc.last_amp;
				osc.last_amp = 0;
				if ( amp && old_output )
					other_synth.offset( time, -amp, old_output );
			}
		}
		
		if ( addr == status_reg && data != old_reg )
		{
			if ( !(data & 0x80) )
			{
				for ( unsigned i = 0; i < sizeof powerup_regs; i++ )
				{
					if ( i != status_reg - start_addr )
						write_register( time, i + start_addr, powerup_regs [i] );
				}
			}
			else
			{
				//dprintf( "APU powered on\n" );
			}
		}
	}
	else if ( addr >= 0xFF30 )
	{
		int index = (addr & 0x0F) * 2;
		wave.wave [index] = data >> 4;
		wave.wave [index + 1] = data & 0x0F;
	}
}

int Gb_Apu::read_register( blip_time_t time, unsigned addr )
{
	run_until( time );
	
	int index = addr - start_addr;
	require( (unsigned) index < register_count );
	int data = regs [index];
	
	if ( addr == status_reg )
	{
		data = (data & 0x80) | 0x70;
		for ( int i = 0; i < osc_count; i++ )
		{
			const Gb_Osc& osc = *oscs [i];
			if ( osc.enabled && (osc.length || !(osc.regs [4] & osc.len_enabled_mask)) )
				data |= 1 << i;
		}
	}
	
	return data;
}

// square1, square2, wave, noise
void Gb_Apu::save_state(Gb_ApuState *state)
{
 memset(state, 0, sizeof(Gb_ApuState));

 memcpy(state->regs, regs, register_count);

 for(int x = 0; x < 4; x++)
 {
  state->length[x] = oscs[x]->length;
  state->volume[x] = oscs[x]->volume;
  state->enabled[x] = oscs[x]->enabled;
 }

 state->wave_pos = wave.wave_pos;
 memcpy(state->wave, wave.wave, 32);

 state->env_delay[0] = square1.env_delay;
 state->env_delay[1] = square2.env_delay;

 state->sq_phase[0] = square1.phase;
 state->sq_phase[1] = square2.phase;

 state->sq_sweep_delay[0] = square1.sweep_delay;
 state->sq_sweep_delay[1] = square2.sweep_delay;

 state->sq_sweep_freq[0] = square1.sweep_freq;
 state->sq_sweep_freq[1] = square2.sweep_freq;

 state->noise_bits = noise.bits;
 state->env_delay[2] = noise.env_delay;
}

void Gb_Apu::load_state(const Gb_ApuState *state)
{
 for(int i = 0; i < register_count; i++)
  write_register(0, start_addr + i, state->regs[i]);

 for(int x = 0; x < 4; x++)
 {
  oscs[x]->length = state->length[x];
  oscs[x]->volume = state->volume[x];
  oscs[x]->enabled = state->enabled[x];
 }

 wave.wave_pos = state->wave_pos;
 memcpy(wave.wave, state->wave, 32);

 square1.env_delay = state->env_delay[0];
 square2.env_delay = state->env_delay[1];

 square1.phase = state->sq_phase[0];
 square2.phase = state->sq_phase[1];

 square1.sweep_delay = state->sq_sweep_delay[0];
 square2.sweep_delay = state->sq_sweep_delay[1];

 square1.sweep_freq = state->sq_sweep_freq[0];
 square2.sweep_freq = state->sq_sweep_freq[1];

 noise.bits = state->noise_bits;
 noise.env_delay = state->env_delay[2];
}


