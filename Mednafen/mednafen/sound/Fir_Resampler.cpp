// Game_Music_Emu 0.5.2. http://www.slack.net/~ant/

#include "Fir_Resampler.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Copyright (C) 2004-2006 Shay Green. This module is free software; you
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

#undef PI
#define PI 3.1415926535897932384626433832795029

static void gen_sinc( double rolloff, int width, double offset, double spacing, double scale,
		int count, short* out )
{
	double const maxh = 256;
	double const step = PI / maxh * spacing;
	double const to_w = maxh * 2 / width;
	double const pow_a_n = pow( rolloff, maxh );
	scale /= maxh * 2;
	
	double angle = (count / 2 - 1 + offset) * -step;
	while ( count-- )
	{
		*out++ = 0;
		double w = angle * to_w;
		if ( fabs( w ) < PI )
		{
			double rolloff_cos_a = rolloff * cos( angle );
			double num = 1 - rolloff_cos_a -
					pow_a_n * cos( maxh * angle ) +
					pow_a_n * rolloff * cos( (maxh - 1) * angle );
			double den = 1 - rolloff_cos_a - rolloff_cos_a + rolloff * rolloff;
			double sinc = scale * num / den - scale;
			
			out [-1] = (short) (cos( w ) * sinc + sinc);
		}
		angle += step;
	}
}

Fir_Resampler_::Fir_Resampler_( int width, sample_t* impulses_ ) :
	width_( width ),
	write_offset( width * stereo - stereo ),
	impulses( impulses_ )
{
	write_pos = 0;
	res       = 1;
	imp_phase = 0;
	skip_bits = 0;
	step      = stereo;
	ratio_    = 1.0;
}

Fir_Resampler_::~Fir_Resampler_() { }

void Fir_Resampler_::clear()
{
	imp_phase = 0;
	if ( buf.size() )
	{
		write_pos = &buf [write_offset];
		memset( buf.begin(), 0, write_offset * sizeof buf [0] );
	}
}

blargg_err_t Fir_Resampler_::buffer_size( int new_size )
{
	RETURN_ERR( buf.resize( new_size + write_offset ) );
	clear();
	return 0;
}
	
double Fir_Resampler_::time_ratio( double new_factor, double rolloff, double gain )
{
	ratio_ = new_factor;
	
	double fstep = 0.0;
	{
		double least_error = 2;
		double pos = 0;
		res = -1;
		for ( int r = 1; r <= max_res; r++ )
		{
			pos += ratio_;
			double nearest = floor( pos + 0.5 );
			double error = fabs( pos - nearest );
			if ( error < least_error )
			{
				res = r;
				fstep = nearest / res;
				least_error = error;
			}
		}
	}
	
	skip_bits = 0;
	
	step = stereo * (int) floor( fstep );
	
	ratio_ = fstep;
	fstep = fmod( fstep, 1.0 );
	
	double filter = (ratio_ < 1.0) ? 1.0 : 1.0 / ratio_;
	double pos = 0.0;
	input_per_cycle = 0;
	for ( int i = 0; i < res; i++ )
	{
		gen_sinc( rolloff, int (width_ * filter + 1) & ~1, pos, filter,
				double (0x7FFF * gain * filter),
				(int) width_, impulses + i * width_ );
		
		pos += fstep;
		input_per_cycle += step;
		if ( pos >= 0.9999999 )
		{
			pos -= 1.0;
			skip_bits |= 1 << i;
			input_per_cycle++;
		}
	}
	
	clear();
	
	return ratio_;
}

int Fir_Resampler_::input_needed( blargg_long output_count ) const
{
	blargg_long input_count = 0;
	
	unsigned long skip = skip_bits >> imp_phase;
	int remain = res - imp_phase;
	while ( (output_count -= 2) > 0 )
	{
		input_count += step + (skip & 1) * stereo;
		skip >>= 1;
		if ( !--remain )
		{
			skip = skip_bits;
			remain = res;
		}
		output_count -= 2;
	}
	
	long input_extra = input_count - (write_pos - &buf [(width_ - 1) * stereo]);
	if ( input_extra < 0 )
		input_extra = 0;
	return input_extra;
}

int Fir_Resampler_::avail_( blargg_long input_count ) const
{
	int cycle_count = input_count / input_per_cycle;
	int output_count = cycle_count * res * stereo;
	input_count -= cycle_count * input_per_cycle;
	
	blargg_ulong skip = skip_bits >> imp_phase;
	int remain = res - imp_phase;
	while ( input_count >= 0 )
	{
		input_count -= step + (skip & 1) * stereo;
		skip >>= 1;
		if ( !--remain )
		{
			skip = skip_bits;
			remain = res;
		}
		output_count += 2;
	}
	return output_count;
}

int Fir_Resampler_::skip_input( long count )
{
	int remain = write_pos - buf.begin();
	int max_count = remain - width_ * stereo;
	if ( count > max_count )
		count = max_count;
	
	remain -= count;
	write_pos = &buf [remain];
	memmove( buf.begin(), &buf [count], remain * sizeof buf [0] );
	
	return count;
}
