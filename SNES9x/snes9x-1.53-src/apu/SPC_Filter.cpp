// snes_spc 0.9.0. http://www.slack.net/~ant/

#include "SPC_Filter.h"

#include <string.h>

/* Copyright (C) 2007 Shay Green. This module is free software; you
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

void SPC_Filter::clear() { memset( ch, 0, sizeof ch ); }

SPC_Filter::SPC_Filter()
{
	gain = gain_unit;
	bass = bass_norm;
	clear();
}

void SPC_Filter::run( short* io, int count )
{
	require( (count & 1) == 0 ); // must be even
	
	int const gain = this->gain;
	int const bass = this->bass;
	chan_t* c = &ch [2];
	do
	{
		// cache in registers
		int sum = (--c)->sum;
		int pp1 = c->pp1;
		int p1  = c->p1;
		
		for ( int i = 0; i < count; i += 2 )
		{
			// Low-pass filter (two point FIR with coeffs 0.25, 0.75)
			int f = io [i] + p1;
			p1 = io [i] * 3;
			
			// High-pass filter ("leaky integrator")
			int delta = f - pp1;
			pp1 = f;
			int s = sum >> (gain_bits + 2);
			sum += (delta * gain) - (sum >> bass);
			
			// Clamp to 16 bits
			if ( (short) s != s )
				s = (s >> 31) ^ 0x7FFF;
			
			io [i] = (short) s;
		}
		
		c->p1  = p1;
		c->pp1 = pp1;
		c->sum = sum;
		++io;
	}
	while ( c != ch );
}
