
// Gba_Snd_Emu 0.1.1. http://www.slack.net/~ant/nes-emu/

#include "Gba_Apu.h"

#include <string.h>
#include <stdio.h>

/* Library Copyright (C) 2003-2004 Shay Green. Gba_Snd_Emu is free
software; you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.
Gba_Snd_Emu is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
more details. You should have received a copy of the GNU General Public
License along with Gba_Snd_Emu; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */

const int trigger = 0x80;

// Gba_Osc

Gba_Osc::Gba_Osc() {
	output = NULL;
	output_select = 0;
	outputs [0] = NULL;
	outputs [1] = NULL;
	outputs [2] = NULL;
	outputs [3] = NULL;
        peradjust = 0;
        gbamode = 0;
}

void Gba_Osc::reset() 
{
	delay = 0;
	last_amp = 0;
	period = 2048;
	volume = 0;
	frequency = 0;
	length = 0;
	enabled = false;
	length_enabled = false;
}

void Gba_Osc::setME(int s)
{
 if(!s)
 {
  enabled = false;
  length = 0;
 }
 ME = s;
}

void Gba_Osc::clock_length() 
{
	if ( length_enabled && length )
		--length;
}

void Gba_Osc::write_register( int reg, int value ) {
	if ( reg == 4 )
		length_enabled = value & 0x40;
}

// Gba_Env

void Gba_Env::reset() {
	env_period = 0;
	env_dir = 0;
	env_delay = 0;
	new_env_period = 0;
	new_env_dir = 0;
	new_volume = 0;
	Gba_Osc::reset();
}

Gba_Env::Gba_Env() {
}

void Gba_Env::clock_envelope() {
	if ( env_delay && !--env_delay ) 
	{
		env_delay = env_period;
		if ( env_dir ) {
			if ( volume < 15 )
				++volume;
		}
		else if ( volume > 0 ) {
			--volume;
		}
	}
}

void Gba_Env::write_register( int reg, int value ) {
	if ( reg == 2 ) {
		env_period = new_env_period = value & 7;
		env_dir = new_env_dir = value & 8;

		new_volume = value >> 4;
		if(!gbamode)
			volume = new_volume;
		//enabled = new_volume != 0;
	}
	else if ( reg == 4 && value & trigger ) {
		env_period = new_env_period;
		env_delay = new_env_period;
		env_dir = new_env_dir;
		volume = new_volume;
	}
	Gba_Osc::write_register( reg, value );
}

// Gba_Square

void Gba_Square::reset() {
	phase = 1;
	duty = 1;
	
	sweep_period = 0;
	sweep_delay = 0;
	sweep_shift = 0;
	sweep_dir = 0;
	sweep_freq = 0;
	
	Gba_Env::reset();
}

Gba_Square::Gba_Square() {
	has_sweep = false;
}

void Gba_Square::clock_sweep() {
	if ( sweep_period && sweep_delay && !--sweep_delay ) {
		sweep_delay = sweep_period;
		frequency = sweep_freq;
		period = (2048 - frequency) * 4;
		
		int offset = sweep_freq >> sweep_shift;
		if ( sweep_dir )
			offset = -offset;
		sweep_freq += offset;
		if ( sweep_freq < 0 || sweep_freq >= 2048 ) {
			sweep_delay = 0;
			sweep_freq = 2048; // stop sound output
		}
	}
}

void Gba_Square::write_register( int reg, int value )
{
	//printf("Write: %d %02x\n",reg, value);
	switch ( reg ) {
		case 0:
			sweep_period = ((value >> 4) & 7);
			sweep_shift = value & 7;
			sweep_dir = value & 0x08;
			break;
		
		case 1:
			new_length = length = 64 - (value & 0x3f);
			duty = (value >> 5) & 6; // duty = { 1, 2, 4, 6 }
			if ( !duty )
				duty = 1;
			break;
		
		case 3:
			frequency = (frequency & ~0xFF) + value;
			break;
		
		case 4:
			frequency = (value & 7) * 0x100 + (frequency & 0xFF);
			length = new_length;
			if (( value & trigger) && ME) {
				sweep_freq = frequency;
				if ( has_sweep && sweep_period && sweep_shift ) {
					sweep_delay = 1;
					clock_sweep();
				}
				if(ME) enabled = true;
			}
			break;
	}
	
	period = (2048 - frequency) * 4;
	
	Gba_Env::write_register( reg, value );
}

void Gba_Square::run( gb_time_t time, gb_time_t end_time )
{
	if (!enabled || (!length && length_enabled) || !volume || sweep_freq == 2048 ) {
		if ( last_amp ) {
			synth->offset( time, -last_amp, output );
			last_amp = 0;
		}
		delay = 0;
	}
	else
	{
		int amp = (phase < duty) ? volume : -volume;

		if ( amp != last_amp ) {
			synth->offset( time, amp - last_amp, output );
			last_amp = amp;
		}
		
		time += delay;
		if ( time < end_time )
		{
			Blip_Buffer* const output = this->output;
			const int duty = this->duty;
			int phase = this->phase;
			amp *= 2;
			do {
				phase = (phase + 1) & 7;
				if ( phase == 0 || phase == duty ) {
					amp = -amp;
					synth->offset_inline( time, amp, output );
				}
				time += period << peradjust;
			}
			while ( time < end_time );
			
			this->phase = phase;
			last_amp = amp >> 1;
		}
		delay = time - end_time;
	}
}


// Gba_Wave

void Gba_Wave::reset() {
	volume_shift = 0;
	wave_pos = 0;
	ws_offset = 0;
	ws_mask = 32;
	memset( wave, 0, sizeof wave );
	Gba_Osc::reset();
}

Gba_Wave::Gba_Wave() 
{
}

void Gba_Wave::write_register( int reg, int value )
{
	switch ( reg ) {
		case 0:
			if(ME) enabled = value & 0x80;
                if(gbamode)
                {
                 if((value>> 5) & 1)
                 {
                  ws_offset = 0;
                  ws_mask = 64;
                 }
                 else
                 {
                  ws_offset = ((value >> 6) & 1) * 0x10;
                  ws_mask = 32;
                 }
                }
                else
                {
                 ws_mask = 32;
                 ws_offset = 0;
                }
			break;
		
		case 1:
			new_length = length = 256 - value;
			break;
		
		case 2:
			volume = ((value >> 5) & 3);
			volume_shift = (volume - 1) & 7; // silence = 7
			//printf("%d\n", volume);
			break;
		
		case 3:
			frequency = (frequency & ~0xFF) + value;
			break;
		
		case 4:
			length = new_length;
			frequency = (value & 7) * 0x100 + (frequency & 0xFF);
			//if ( value & trigger )
			//  wave_pos = 0;
			break;
		
	}
	
	period = (2048 - frequency) * 2;
	
	Gba_Osc::write_register( reg, value );
}

void Gba_Wave::run( gb_time_t time, gb_time_t end_time )
{
	if ( !enabled || (!length && length_enabled) || !volume ) {
		if ( last_amp ) {
			synth.offset( time, -last_amp, output );
			last_amp = 0;
		}
		delay = 0;
	}
	else
	{
		// wave data or shift may have changed
		int diff = (wave [wave_pos] >> volume_shift) * 2 - last_amp;
		if ( diff ) {
			last_amp += diff;
			synth.offset( time, diff, output );
		}

		time += delay;
		if ( time < end_time )
		{
		 	unsigned wave_pos = this->wave_pos;
		 	
			do {
				wave_pos = (wave_pos + 1) % ws_mask; //wave_size;
				int amp = (wave [wave_pos + ws_offset] >> volume_shift) * 2;
				int diff = amp - last_amp;
				if ( diff ) {
					last_amp = amp;
					synth.offset_inline( time, diff, output );
				}
				time += period << peradjust;
			}
			while ( time < end_time );
			
			this->wave_pos = wave_pos;
		}
		delay = time - end_time;
	}
}


// Gba_Noise

void Gba_Noise::reset() {
	bits = 1;
	tap = 14;
	Gba_Env::reset();
}

Gba_Noise::Gba_Noise() {
}

void Gba_Noise::write_register( int reg, int value ) 
{
	if ( reg == 1 ) {
		new_length = length = 64 - (value & 0x3f);
	}
	else if ( reg == 3 ) {
		tap = 14 - (value & 8);

		// noise formula and frequency tested against Metroid 2 and Zelda LA
		int divisor = (value & 7) * 16;
		if ( !divisor )
			divisor = 8;
		period = divisor << (value >> 4);
	}
	else if ( reg == 4 && value & trigger ) {
		length = new_length;
		bits = ~0u;
		if(ME)
		 enabled = 1;
	}
	
	Gba_Env::write_register( reg, value );
}

void Gba_Noise::run( gb_time_t time, gb_time_t end_time )
{
	if ( !enabled || (!length && length_enabled) || !volume ) {
		if ( last_amp ) {
			synth.offset( time, -last_amp, output );
			last_amp = 0;
		}
		delay = 0;
	}
	else
	{
		int amp = bits & 1 ? -volume : volume;

		if ( amp != last_amp ) {
			synth.offset( time, amp - last_amp, output );
			last_amp = amp;
		}
		
		time += delay;
		if ( time < end_time )
		{
			Blip_Buffer* const output = this->output;
			// keep parallel resampled time to eliminate multiplication in the loop
			const Blip_Buffer::blip_resampled_time_t resampled_period =
					output->resampled_duration( period );
			Blip_Buffer::blip_resampled_time_t resampled_time = output->resampled_time( time );
			const unsigned mask = ~(1u << tap);
			unsigned bits = this->bits;
			amp *= 2;

			do {
				unsigned feedback = bits;
				bits >>= 1;
				feedback = 1 & (feedback ^ bits);
				time += period << peradjust;
				bits = (feedback << tap) | (bits & mask);
				// feedback just happens to be true only when the level needs to change
				// (the previous and current bits are different)
				if ( feedback ) {
					amp = -amp;
					synth.offset_resampled( resampled_time, amp, output );
				}
				resampled_time += resampled_period << peradjust;
			}
			while ( time < end_time );
			
			this->bits = bits;
			last_amp = amp >> 1;
		}
		delay = time - end_time;
	}
}

