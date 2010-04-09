// T6W28_Snd_Emu

#include "../mednafen.h"
#include "T6W28_Apu.h"

/* Copyright (C) 2003-2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details. You should have received a copy of the GNU Lesser General
Public License along with this module; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */

// T6W28_Osc

T6W28_Osc::T6W28_Osc()
{
	outputs [0] = NULL; // always stays NULL
	outputs [1] = NULL;
	outputs [2] = NULL;
	outputs [3] = NULL;
}

void T6W28_Osc::reset()
{
	delay = 0;
	last_amp_left = 0;
	last_amp_right = 0;

	volume_left = 0;
	volume_right = 0;
}

// T6W28_Square

inline void T6W28_Square::reset()
{
	period = 0;
	phase = 0;
	T6W28_Osc::reset();
}

void T6W28_Square::run( sms_time_t time, sms_time_t end_time )
{
	if ((!volume_left && !volume_right) || period <= 128 )
	{
		// ignore 16kHz and higher
		if ( last_amp_left )
		{
			synth->offset( time, -last_amp_left, outputs[2] );
			last_amp_left = 0;
		}

                if ( last_amp_right )
                {
                        synth->offset( time, -last_amp_right, outputs[1] );
                        last_amp_right = 0;
                }

		time += delay;
		if ( !period )
		{
			time = end_time;
		}
		else if ( time < end_time )
		{
			// keep calculating phase
			int count = (end_time - time + period - 1) / period;
			phase = (phase + count) & 1;
			time += count * period;
		}
	}
	else
	{
		int amp_left = phase ? volume_left : -volume_left;
		int amp_right = phase ? volume_right : -volume_right;

		int delta_left  = amp_left - last_amp_left;
		int delta_right = amp_right - last_amp_right;

		if ( delta_left )
		{
			last_amp_left = amp_left;
			synth->offset( time, delta_left, outputs[2] );
		}
		
                if ( delta_right )
                {
                        last_amp_right = amp_right;
                        synth->offset( time, delta_right, outputs[1] );
                }

		time += delay;
		if ( time < end_time )
		{
			Blip_Buffer* const output_left = this->outputs[2];
			Blip_Buffer* const output_right = this->outputs[1];

			int delta_left = amp_left * 2;
			int delta_right = amp_right * 2;
			do
			{
				delta_left = -delta_left;
				delta_right = -delta_right;

				synth->offset_inline( time, delta_left, output_left );
				synth->offset_inline( time, delta_right, output_right );
				time += period;
				phase ^= 1;
			}
			while ( time < end_time );

			this->last_amp_left = phase ? volume_left : -volume_left;
			this->last_amp_right = phase ? volume_right : -volume_right;
		}
	}
	delay = time - end_time;
}

// T6W28_Noise

static const int noise_periods [3] = { 0x100, 0x200, 0x400 };

inline void T6W28_Noise::reset()
{
	period = &noise_periods [0];
	shifter = 0x8000;
	tap = 12;
	T6W28_Osc::reset();
}

void T6W28_Noise::run( sms_time_t time, sms_time_t end_time )
{
	int amp_left = volume_left;
	int amp_right = volume_right;

	if ( shifter & 1 )
	{
		amp_left = -amp_left;
		amp_right = -amp_right;
	}

	int delta_left = amp_left - last_amp_left;
	int delta_right = amp_right - last_amp_right;

	if ( delta_left )
	{
		last_amp_left = amp_left;
		synth.offset( time, delta_left, outputs[2] );
	}
	
        if ( delta_right )
        {
                last_amp_right = amp_right;
                synth.offset( time, delta_right, outputs[1] );
        }

	time += delay;

	if ( !volume_left && !volume_right )
		time = end_time;
	
	if ( time < end_time )
	{
		Blip_Buffer* const output_left = this->outputs[2];
		Blip_Buffer* const output_right = this->outputs[1];

		unsigned shifter = this->shifter;
		int delta_left = amp_left * 2;
		int delta_right = amp_right * 2;

		int period = *this->period * 2;
		if ( !period )
			period = 16;
		
		do
		{
			int changed = (shifter + 1) & 2; // set if prev and next bits differ
			shifter = (((shifter << 15) ^ (shifter << tap)) & 0x8000) | (shifter >> 1);
			if ( changed )
			{
				delta_left = -delta_left;
				synth.offset_inline( time, delta_left, output_left );

				delta_right = -delta_right;
				synth.offset_inline( time, delta_right, output_right );
			}
			time += period;
		}
		while ( time < end_time );
		
		this->shifter = shifter;
		this->last_amp_left = delta_left >> 1;
		this->last_amp_right = delta_right >> 1;
	}
	delay = time - end_time;
}

// T6W28_Apu

T6W28_Apu::T6W28_Apu()
{
	for ( int i = 0; i < 3; i++ )
	{
		squares [i].synth = &square_synth;
		oscs [i] = &squares [i];
	}
	oscs [3] = &noise;
	
	volume( 1.0 );
	reset();
}

T6W28_Apu::~T6W28_Apu()
{
}

void T6W28_Apu::volume( double vol )
{
	vol *= 0.85 / (osc_count * 64 * 2);
	square_synth.volume( vol );
	noise.synth.volume( vol );
}

void T6W28_Apu::treble_eq( const blip_eq_t& eq )
{
	square_synth.treble_eq( eq );
	noise.synth.treble_eq( eq );
}

void T6W28_Apu::osc_output( int index, Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right )
{
	require( (unsigned) index < osc_count );
	require( (center && left && right) || (!center && !left && !right) );
	T6W28_Osc& osc = *oscs [index];
	osc.outputs [1] = right;
	osc.outputs [2] = left;
	osc.outputs [3] = center;
}

void T6W28_Apu::output( Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right )
{
	for ( int i = 0; i < osc_count; i++ )
		osc_output( i, center, left, right );
}

void T6W28_Apu::reset()
{
	last_time = 0;
	latch_left = 0;
	latch_right = 0;

	squares [0].reset();
	squares [1].reset();
	squares [2].reset();
	noise.reset();
}

void T6W28_Apu::run_until( sms_time_t end_time )
{
	require( end_time >= last_time ); // end_time must not be before previous time
	
	if ( end_time > last_time )
	{
		// run oscillators
		for ( int i = 0; i < osc_count; ++i )
		{
			T6W28_Osc& osc = *oscs [i];
			if ( osc.outputs[1] )
			{
				if ( i < 3 )
					squares [i].run( last_time, end_time );
				else
					noise.run( last_time, end_time );
			}
		}
		
		last_time = end_time;
	}
}

bool T6W28_Apu::end_frame( sms_time_t end_time )
{
	if ( end_time > last_time )
		run_until( end_time );
	
	assert( last_time >= end_time );
	last_time -= end_time;
	
	return(1);
}

static const unsigned char volumes [16] = {
	// volumes [i] = 64 * pow( 1.26, 15 - i ) / pow( 1.26, 15 )
	64, 50, 39, 31, 24, 19, 15, 12, 9, 7, 5, 4, 3, 2, 1, 0
};

void T6W28_Apu::write_data_left( sms_time_t time, int data )
{
	require( (unsigned) data <= 0xFF );
	
	run_until( time );
	
	if ( data & 0x80 )
		latch_left = data;
	
	int index = (latch_left >> 5) & 3;

	if ( latch_left & 0x10 )
	{
		oscs [index]->volume_left = volumes [data & 15];
	}
	else if ( index < 3 )
	{
		T6W28_Square& sq = squares [index];
		if ( data & 0x80 )
			sq.period = (sq.period & 0xFF00) | (data << 4 & 0x00FF);
		else
			sq.period = (sq.period & 0x00FF) | (data << 8 & 0x3F00);
	}
}

void T6W28_Apu::write_data_right( sms_time_t time, int data )
{
        require( (unsigned) data <= 0xFF );

        run_until( time );

        if ( data & 0x80 )
                latch_right = data;

        int index = (latch_right >> 5) & 3;
        //printf("%d\n", index);

        if ( latch_right & 0x10 )
        {
                oscs [index]->volume_right = volumes [data & 15];
        }
        else if ( index == 2 )
        {
                if ( data & 0x80 )
                        noise.period_extra = (noise.period_extra & 0xFF00) | (data << 4 & 0x00FF);
                else
                        noise.period_extra = (noise.period_extra & 0x00FF) | (data << 8 & 0x3F00);
        }
        else if(index == 3)
        {
                int select = data & 3;
                if ( select < 3 )
                        noise.period = &noise_periods [select];
                else
                        noise.period = &noise.period_extra;

                int const tap_disabled = 16;
                noise.tap = (data & 0x04) ? 12 : tap_disabled;
                noise.shifter = 0x8000;
        }
}


T6W28_ApuState *T6W28_Apu::save_state(void)
{
 T6W28_ApuState *ret = (T6W28_ApuState *)malloc(sizeof(T6W28_ApuState));

 for(int x = 0; x < 4; x++)
 {
  ret->volume_left[x] = oscs[x]->volume_left;
  ret->volume_right[x] = oscs[x]->volume_right;
 }
 for(int x = 0; x < 3; x++)
 {
  ret->sq_period[x] = squares[x].period;
  ret->sq_phase[x] = squares[x].phase;
 }
 ret->noise_shifter = noise.shifter;
 ret->noise_tap = noise.tap;

 if(noise.period == &noise_periods[0])
  ret->noise_period = 0;
 else if(noise.period == &noise_periods[1])
  ret->noise_period = 1;
 else if(noise.period == &noise_periods[2])
  ret->noise_period = 2;
 else ret->noise_period = 3;

 return(ret);
}

void T6W28_Apu::load_state(T6W28_ApuState *state)
{
 for(int x = 0; x < 4; x++)
 {
  oscs[x]->volume_left = state->volume_left[x];
  oscs[x]->volume_right = state->volume_right[x];
 }
 for(int x = 0; x < 3; x++)
 {
  squares[x].period = state->sq_period[x];
  squares[x].phase = state->sq_phase[x];
 }
 noise.shifter = state->noise_shifter;
 noise.tap = state->noise_tap = noise.tap;

 int select = state->noise_period;

 if ( select < 3 )
  noise.period = &noise_periods [select];
 else
  noise.period = &noise.period_extra;
}

