// Gb_Snd_Emu 0.1.5. http://www.slack.net/~ant/

#include "Gb_Apu.h"

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

// Gb_Osc

void Gb_Osc::reset()
{
	delay = 0;
	last_amp = 0;
	length = 0;
	output_select = 3;
	output = outputs [output_select];
}

void Gb_Osc::clock_length()
{
	if ( (regs [4] & len_enabled_mask) && length )
		length--;
}

// Gb_Env

void Gb_Env::clock_envelope()
{
	if ( env_delay && !--env_delay )
	{
		env_delay = regs [2] & 7;
		int v = volume - 1 + (regs [2] >> 2 & 2);
		if ( (unsigned) v < 15 )
			volume = v;
	}
}

bool Gb_Env::write_register( int reg, int data )
{
	switch ( reg )
	{
	case 1:
		length = 64 - (regs [1] & 0x3F);
		break;
	
	case 2:
		if ( !(data >> 4) )
			enabled = false;
		break;
	
	case 4:
		if ( data & trigger )
		{
			env_delay = regs [2] & 7;
			volume = regs [2] >> 4;
			enabled = true;
			if ( length == 0 )
				length = 64;
			return true;
		}
	}
	return false;
}

// Gb_Square

void Gb_Square::reset()
{
	phase = 0;
	sweep_freq = 0;
	sweep_delay = 0;
	Gb_Env::reset();
}

void Gb_Square::clock_sweep()
{
	int sweep_period = (regs [0] & period_mask) >> 4;
	if ( sweep_period && sweep_delay && !--sweep_delay )
	{
		sweep_delay = sweep_period;
		regs [3] = sweep_freq & 0xFF;
		regs [4] = (regs [4] & ~0x07) | (sweep_freq >> 8 & 0x07);
		
		int offset = sweep_freq >> (regs [0] & shift_mask);
		if ( regs [0] & 0x08 )
			offset = -offset;
		sweep_freq += offset;
		
		if ( sweep_freq < 0 )
		{
			sweep_freq = 0;
		}
		else if ( sweep_freq >= 2048 )
		{
			sweep_delay = 0; // don't modify channel frequency any further
			sweep_freq = 2048; // silence sound immediately
		}
	}
}

void Gb_Square::run( blip_time_t time, blip_time_t end_time, int playing )
{
	if ( sweep_freq == 2048 )
		playing = false;
	
	static unsigned char const table [4] = { 1, 2, 4, 6 };
	int const duty = table [regs [1] >> 6];
	int amp = volume & playing;
	if ( phase >= duty )
		amp = -amp;
	
	int frequency = this->frequency();
	if ( unsigned (frequency - 1) > 2040 ) // frequency < 1 || frequency > 2041
	{
		// really high frequency results in DC at half volume
		amp = volume >> 1;
		playing = false;
	}
	
	{
		int delta = amp - last_amp;
		if ( delta )
		{
			last_amp = amp;
			synth->offset( time, delta, output );
		}
	}
	
	time += delay;
	if ( !playing )
		time = end_time;
	
	if ( time < end_time )
	{
		int const period = (2048 - frequency) * 4;
		Blip_Buffer* const output = this->output;
		int phase = this->phase;
		int delta = amp * 2;
		do
		{
			phase = (phase + 1) & 7;
			if ( phase == 0 || phase == duty )
			{
				delta = -delta;
				synth->offset_inline( time, delta, output );
			}
			time += period;
		}
		while ( time < end_time );
		
		this->phase = phase;
		last_amp = delta >> 1;
	}
	delay = time - end_time;
}

// Gb_Noise

void Gb_Noise::run( blip_time_t time, blip_time_t end_time, int playing )
{
	int amp = volume & playing;
	int tap = 13 - (regs [3] & 8);
	if ( bits >> tap & 2 )
		amp = -amp;
	
	{
		int delta = amp - last_amp;
		if ( delta )
		{
			last_amp = amp;
			synth->offset( time, delta, output );
		}
	}
	
	time += delay;
	if ( !playing )
		time = end_time;
	
	if ( time < end_time )
	{
		static unsigned char const table [8] = { 8, 16, 32, 48, 64, 80, 96, 112 };
		int period = table [regs [3] & 7] << (regs [3] >> 4);
		
		// keep parallel resampled time to eliminate time conversion in the loop
		Blip_Buffer* const output = this->output;
		const blip_resampled_time_t resampled_period =
				output->resampled_duration( period );
		blip_resampled_time_t resampled_time = output->resampled_time( time );
		unsigned bits = this->bits;
		int delta = amp * 2;
		
		do
		{
			unsigned changed = (bits >> tap) + 1;
			time += period;
			bits <<= 1;
			if ( changed & 2 )
			{
				delta = -delta;
				bits |= 1;
				synth->offset_resampled( resampled_time, delta, output );
			}
			resampled_time += resampled_period;
		}
		while ( time < end_time );
		
		this->bits = bits;
		last_amp = delta >> 1;
	}
	delay = time - end_time;
}

// Gb_Wave

inline void Gb_Wave::write_register( int reg, int data )
{
	switch ( reg )
	{
	case 0:
		if ( !(data & 0x80) )
			enabled = false;
		break;
	
	case 1:
		length = 256 - regs [1];
		break;
	
	case 2:
		volume = data >> 5 & 3;
		break;
	
	case 4:
		if ( data & trigger & regs [0] )
		{
			wave_pos = 0;
			enabled = true;
			if ( length == 0 )
				length = 256;
		}
	}
}

void Gb_Wave::run( blip_time_t time, blip_time_t end_time, int playing )
{
	int volume_shift = (volume - 1) & 7; // volume = 0 causes shift = 7
	int frequency;
	{
		int amp = (wave [wave_pos] >> volume_shift & playing) * 2;
		
		frequency = this->frequency();
		if ( unsigned (frequency - 1) > 2044 ) // frequency < 1 || frequency > 2045
		{
			amp = 30 >> volume_shift & playing;
			playing = false;
		}
		
		int delta = amp - last_amp;
		if ( delta )
		{
			last_amp = amp;
			synth->offset( time, delta, output );
		}
	}
	
	time += delay;
	if ( !playing )
		time = end_time;
	
	if ( time < end_time )
	{
		Blip_Buffer* const output = this->output;
		int const period = (2048 - frequency) * 2;
	 	int wave_pos = (this->wave_pos + 1) & (wave_size - 1);
	 	
		do
		{
			int amp = (wave [wave_pos] >> volume_shift) * 2;
			wave_pos = (wave_pos + 1) & (wave_size - 1);
			int delta = amp - last_amp;
			if ( delta )
			{
				last_amp = amp;
				synth->offset_inline( time, delta, output );
			}
			time += period;
		}
		while ( time < end_time );
		
		this->wave_pos = (wave_pos - 1) & (wave_size - 1);
	}
	delay = time - end_time;
}

// Gb_Apu::write_osc

void Gb_Apu::write_osc( int index, int reg, int data )
{
	reg -= index * 5;
	Gb_Square* sq = &square2;
	switch ( index )
	{
	case 0:
		sq = &square1;
	case 1:
		if ( sq->write_register( reg, data ) && index == 0 )
		{
			square1.sweep_freq = square1.frequency();
			if ( (regs [0] & sq->period_mask) && (regs [0] & sq->shift_mask) )
			{
				square1.sweep_delay = 1; // cause sweep to recalculate now
				square1.clock_sweep();
			}
		}
		break;
	
	case 2:
		wave.write_register( reg, data );
		break;
	
	case 3:
		if ( noise.write_register( reg, data ) )
			noise.bits = 0x7FFF;
	}
}
