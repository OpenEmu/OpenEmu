// Sms_Snd_Emu 0.1.4. http://www.slack.net/~ant/

#include "mednafen/mednafen.h"
#include "Sms_Apu.h"

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

// Sms_Osc

Sms_Osc::Sms_Osc()
{
	output = 0;
	outputs [0] = 0; // always stays NULL
	outputs [1] = 0;
	outputs [2] = 0;
	outputs [3] = 0;
}

void Sms_Osc::reset()
{
	delay = 0;
	last_amp = 0;
	volume = 0;
	output_select = 3;
	output = outputs [3];
}

// Sms_Square

blip_inline void Sms_Square::reset()
{
	period = 0;
	phase = 0;
	Sms_Osc::reset();
}

void Sms_Square::run( blip_time_t time, blip_time_t end_time )
{
    int amp = volume;
    if ( period > 128 )
        amp = amp << 1 & -phase;

    {
        int delta = amp - last_amp;
        if ( delta )
        {
            last_amp = amp;
            synth->offset( time, delta, output );
        }
    }

    time += delay;
    delay = 0;
    if ( period )
    {
        if ( time < end_time )
        {
            if ( !volume || period <= 128 ) // ignore 16kHz and higher
            {
                // keep calculating phase
                int count = (end_time - time + period - 1) / period;
                phase = (phase + count) & 1;
                time += count * period;
            }
            else
            {
                Blip_Buffer* const output = this->output;
                int delta = amp * 2 - volume * 2;
                do
                {
                    delta = -delta;
                    synth->offset_inline( time, delta, output );
                    time += period;
                }
                while ( time < end_time );

                last_amp = (delta >> 1) + volume;
                phase = (delta >= 0);
            }
        }
        delay = time - end_time;
    }
}

// Sms_Noise

static int const noise_periods [3] = { 0x100, 0x200, 0x400 };

blip_inline void Sms_Noise::reset()
{
	period = &noise_periods [0];
	shifter = 0x8000;
	feedback = 0x9000;
	Sms_Osc::reset();
}

void Sms_Noise::run( blip_time_t time, blip_time_t end_time )
{
	int amp = (shifter & 1) ? 0 : volume * 2;

	{
		int delta = amp - last_amp;
		if ( delta )
		{
			last_amp = amp;
			synth.offset( time, delta, output );
		}
	}
	
	time += delay;
	if ( !volume )
		time = end_time;

	if ( time < end_time )
	{
		Blip_Buffer* const output = this->output;
		unsigned shifter = this->shifter;
		int delta = (shifter & 1) ? (-volume * 2) : (volume * 2);
		int period = *this->period * 2;
		if ( !period )
			period = 16;

		do
		{
			int changed = shifter + 1;
			shifter = (feedback & -(shifter & 1)) ^ (shifter >> 1);
			if ( changed & 2 ) // true if bits 0 and 1 differ
			{
				amp = (shifter & 1) ? 0 : volume * 2;
				delta = -delta;
				synth.offset_inline( time, delta, output );
				last_amp = amp;
			}
			time += period;
		}
		while ( time < end_time );
		
		this->shifter = shifter;
		this->last_amp = (shifter & 1) ? 0 : volume * 2; //delta >> 1;
	}
	delay = time - end_time;
}

// Sms_Apu

Sms_Apu::Sms_Apu()
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

Sms_Apu::~Sms_Apu()
{
}

void Sms_Apu::volume( double vol )
{
	vol *= 0.85 / (osc_count * 64 * 2);
	square_synth.volume( vol );
	noise.synth.volume( vol );
}

void Sms_Apu::treble_eq( const blip_eq_t& eq )
{
	square_synth.treble_eq( eq );
	noise.synth.treble_eq( eq );
}

void Sms_Apu::osc_output( int index, Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right )
{
	require( (unsigned) index < osc_count );
	require( (center && left && right) || (!center && !left && !right) );
	Sms_Osc& osc = *oscs [index];
	osc.outputs [1] = right;
	osc.outputs [2] = left;
	osc.outputs [3] = center;
	osc.output = osc.outputs [osc.output_select];
}

void Sms_Apu::output( Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right )
{
	for ( int i = 0; i < osc_count; i++ )
		osc_output( i, center, left, right );
}

void Sms_Apu::reset( unsigned feedback, int noise_width )
{
	last_time = 0;
	latch = 0;
	ggstereo_save = 0xFF;
	
	if ( !feedback || !noise_width )
	{
		feedback = 0x0009;
		noise_width = 16;
	}
	// convert to "Galios configuration"
	looped_feedback = 1 << (noise_width - 1);
	noise_feedback  = 0;
	while ( noise_width-- )
	{
		noise_feedback = (noise_feedback << 1) | (feedback & 1);
		feedback >>= 1;
	}
	
	squares [0].reset();
	squares [1].reset();
	squares [2].reset();
	noise.reset();
}

void Sms_Apu::run_until( blip_time_t end_time )
{
	require( end_time >= last_time ); // end_time must not be before previous time
	
	if ( end_time > last_time )
	{
		// run oscillators
		for ( int i = 0; i < osc_count; ++i )
		{
			Sms_Osc& osc = *oscs [i];
			if ( osc.output )
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

void Sms_Apu::end_frame( blip_time_t end_time )
{
	if ( end_time > last_time )
		run_until( end_time );
	
	assert( last_time >= end_time );
	last_time -= end_time;
}

void Sms_Apu::write_ggstereo( blip_time_t time, int data )
{
	require( (unsigned) data <= 0xFF );
	
	ggstereo_save = data;

	run_until( time );
	
	for ( int i = 0; i < osc_count; i++ )
	{
		Sms_Osc& osc = *oscs [i];
		int flags = data >> i;
		Blip_Buffer* old_output = osc.output;
		osc.output_select = (flags >> 3 & 2) | (flags & 1);
		osc.output = osc.outputs [osc.output_select];
		if ( osc.output != old_output && osc.last_amp )
		{
			if ( old_output )
			{
				square_synth.offset( time, -osc.last_amp, old_output );
			}
			osc.last_amp = 0;
		}
	}
}

// volumes [i] = 64 * pow( 1.26, 15 - i ) / pow( 1.26, 15 )
static unsigned char const volumes [16] = {
	64, 50, 39, 31, 24, 19, 15, 12, 9, 7, 5, 4, 3, 2, 1, 0
};

void Sms_Apu::write_data( blip_time_t time, int data )
{
	require( (unsigned) data <= 0xFF );
	
	run_until( time );
	
	if ( data & 0x80 )
		latch = data;
	
	int index = (latch >> 5) & 3;

	//if(index == 3)
        //printf("Write @ %d; Index: %d, %d, Value: %02x\n", time, index, latch, data);

	if ( latch & 0x10 )
	{
		oscs [index]->volume = volumes [data & 15];
	}
	else if ( index < 3 )
	{
		Sms_Square& sq = squares [index];
		if ( data & 0x80 )
			sq.period = (sq.period & 0xFF00) | (data << 4 & 0x00FF);
		else
			sq.period = (sq.period & 0x00FF) | (data << 8 & 0x3F00);
	}
	else
	{
		int select = data & 3;
		if ( select < 3 )
			noise.period = &noise_periods [select];
		else
			noise.period = &squares [2].period;
		
		noise.feedback = (data & 0x04) ? noise_feedback : looped_feedback;
		noise.shifter = 0x8000;
	}
}


void Sms_Apu::save_state(Sms_ApuState *ret)
{
 memset(ret, 0, sizeof(Sms_ApuState));

 ret->ggstereo = ggstereo_save;

 for(int x = 0; x < 4; x++)
 {
  ret->volume[x] = oscs[x]->volume;
 }

 for(int x = 0; x < 3; x++)
 {
  ret->sq_period[x] = squares[x].period;
  ret->sq_phase[x] = squares[x].phase;
 }

 ret->noise_shifter = noise.shifter;
 ret->noise_feedback = noise.feedback;

 if(noise.period == &noise_periods[0])
  ret->noise_period = 0;
 else if(noise.period == &noise_periods[1])
  ret->noise_period = 1;
 else if(noise.period == &noise_periods[2])
  ret->noise_period = 2;
 else ret->noise_period = 3;
}

void Sms_Apu::load_state(const Sms_ApuState *state)
{
 for(int x = 0; x < 4; x++)
 {
  oscs[x]->volume = state->volume[x];
 }

 for(int x = 0; x < 3; x++)
 {
  squares[x].period = state->sq_period[x];
  squares[x].phase = state->sq_phase[x];
 }
 noise.shifter = state->noise_shifter;
 noise.feedback = state->noise_feedback;

 int select = state->noise_period;

 if ( select < 3 )
  noise.period = &noise_periods [select];
 else 
  noise.period = &squares [2].period;

 write_ggstereo(0, state->ggstereo);
}

