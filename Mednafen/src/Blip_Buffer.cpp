
// Blip_Buffer 0.4.0. http://www.slack.net/~ant/

#include "blip/Blip_Buffer.h"

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

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

int const buffer_extra = blip_widest_impulse_ + 2;

Blip_Buffer::Blip_Buffer()
{
	factor_ = LONG_MAX;
	offset_ = 0;
	buffer_ = 0;
	buffer_size_ = 0;
	sample_rate_ = 0;
	reader_accum = 0;
	bass_shift = 0;
	clock_rate_ = 0;
	bass_freq_ = 16;
	length_ = 0;
	
	// assumptions code makes about implementation-defined features
	#ifndef NDEBUG
		// right shift of negative value preserves sign
		long i = LONG_MIN;
		assert( (i >> 1) == LONG_MIN / 2 );
		i = LONG_MIN;
		assert( (i >> (sizeof(long) * 8 - 1)) == -1);
		//assert( (i >> 31) == -1 );
		
		// casting to smaller signed type truncates bits and extends sign
		i = (SHRT_MAX + 1) * 5;
		assert( (short) i == SHRT_MIN );
	#endif
}

Blip_Buffer::~Blip_Buffer()
{
	free( buffer_ );
}

void Blip_Buffer::clear( int entire_buffer )
{
	offset_ = 0;
	reader_accum = 0;
	if ( buffer_ )
	{
		long count = (entire_buffer ? buffer_size_ : samples_avail());
		memset( buffer_, 0, (count + buffer_extra) * sizeof (buf_t_) );
	}
}

Blip_Buffer::blargg_err_t Blip_Buffer::set_sample_rate( long new_rate, int msec )
{
	// start with maximum length that resampled time can represent
	long new_size = (ULONG_MAX >> BLIP_BUFFER_ACCURACY) - buffer_extra - 64;
	if ( msec != blip_max_length )
	{
		long s = (new_rate * (msec + 1) + 999) / 1000;
		if ( s < new_size )
			new_size = s;
		else
			assert( 0 ); // fails if requested buffer length exceeds limit
	}
	
	if ( buffer_size_ != new_size )
	{
		void* p = realloc( buffer_, (new_size + buffer_extra) * sizeof *buffer_ );
		if ( !p )
			return "Out of memory";
		buffer_ = (buf_t_*) p;
	}
	
	buffer_size_ = new_size;
	
	// update things based on the sample rate
	sample_rate_ = new_rate;
	length_ = new_size * 1000 / new_rate - 1;
	if ( msec )
		assert( length_ == msec ); // ensure length is same as that passed in
	if ( clock_rate_ )
		clock_rate( clock_rate_ );
	bass_freq( bass_freq_ );
	
	clear();
	
	return 0; // success
}

blip_resampled_time_t Blip_Buffer::clock_rate_factor( long clock_rate ) const
{
	double ratio = (double) sample_rate_ / clock_rate;
	long factor = (long) floor( ratio * (1L << BLIP_BUFFER_ACCURACY) + 0.5 );
	assert( factor > 0 || !sample_rate_ ); // fails if clock/output ratio is too large
	return (blip_resampled_time_t) factor;
}

void Blip_Buffer::bass_freq( int freq )
{
	bass_freq_ = freq;
	int shift = 31;
	if ( freq > 0 )
	{
		shift = 13;
		long f = (freq << 16) / sample_rate_;
		while ( (f >>= 1) && --shift ) { }
	}
	bass_shift = shift;
}

void Blip_Buffer::end_frame( blip_time_t t )
{
	offset_ += t * factor_;
	assert( samples_avail() <= (long) buffer_size_ ); // time outside buffer length
}

void Blip_Buffer::remove_silence( long count )
{
	assert( count <= samples_avail() ); // tried to remove more samples than available
	offset_ -= (blip_resampled_time_t) count << BLIP_BUFFER_ACCURACY;
}

long Blip_Buffer::count_samples( blip_time_t t ) const
{
	unsigned long last_sample  = resampled_time( t ) >> BLIP_BUFFER_ACCURACY;
	unsigned long first_sample = offset_ >> BLIP_BUFFER_ACCURACY;
	return (long) (last_sample - first_sample);
}

blip_time_t Blip_Buffer::count_clocks( long count ) const
{
	if ( count > buffer_size_ )
		count = buffer_size_;
	blip_resampled_time_t time = (blip_resampled_time_t) count << BLIP_BUFFER_ACCURACY;
	return (blip_time_t) ((time - offset_ + factor_ - 1) / factor_);
}

void Blip_Buffer::remove_samples( long count )
{
	if ( count )
	{
		remove_silence( count );
		
		// copy remaining samples to beginning and clear old samples
		long remain = samples_avail() + buffer_extra;
		memmove( buffer_, buffer_ + count, remain * sizeof *buffer_ );
		memset( buffer_ + remain, 0, count * sizeof *buffer_ );
	}
}

// Blip_Synth_

Blip_Synth_::Blip_Synth_( short* p, int w ) :
	impulses( p ),
	width( w )
{
	volume_unit_ = 0.0;
	kernel_unit = 0;
	buf = 0;
	last_amp = 0;
	delta_factor = 0;
}

static double const pi = 3.1415926535897932384626433832795029;

static void gen_sinc( float* out, int count, double oversample, double treble, double cutoff )
{
	if ( cutoff >= 0.999 )
		cutoff = 0.999;
	
	//if ( treble < -300.0 )
	//	treble = -300.0;
	if ( treble > 5.0 )
		treble = 5.0;

	double const maxh = 4096.0;
	double const rolloff = pow( 10.0, 1.0 / (maxh * 20.0) * treble / (1.0 - cutoff) );
	double const pow_a_n = pow( rolloff, maxh - maxh * cutoff );
	double const to_angle = pi / 2 / maxh / oversample;
	for ( int i = 0; i < count; i++ )
	{
		double angle = ((i - count) * 2 + 1) * to_angle;
		double c = rolloff * cos( (maxh - 1.0) * angle ) - cos( maxh * angle );
		double cos_nc_angle = cos( maxh * cutoff * angle );
		double cos_nc1_angle = cos( (maxh * cutoff - 1.0) * angle );
		double cos_angle = cos( angle );
		
		c = c * pow_a_n - rolloff * cos_nc1_angle + cos_nc_angle;
		double d = 1.0 + rolloff * (rolloff - cos_angle - cos_angle);
		double b = 2.0 - cos_angle - cos_angle;
		double a = 1.0 - cos_angle - cos_nc_angle + cos_nc1_angle;
		
		out [i] = (float) ((a * d + c * b) / (b * d)); // a / b + c / d
	}
}

void blip_eq_t::generate( float* out, int count ) const
{
	// lower cutoff freq for narrow kernels with their wider transition band
	// (8 points->1.49, 16 points->1.15)
	double oversample = blip_res * 2.25 / count + 0.85;
	double half_rate = sample_rate * 0.5;
	if ( cutoff_freq )
		oversample = half_rate / cutoff_freq;
	double cutoff = rolloff_freq * oversample / half_rate;
	
	gen_sinc( out, count, blip_res * oversample, treble, cutoff );
	
	// apply (half of) hamming window
	double to_fraction = pi / (count - 1);
	for ( int i = count; i--; )
		out [i] *= 0.54 - 0.46 * cos( i * to_fraction );
}

void Blip_Synth_::adjust_impulse()
{
	// sum pairs for each phase and add error correction to end of first half
	int const size = impulses_size();
	for ( int p = blip_res; p-- >= blip_res / 2; )
	{
		int p2 = blip_res - 2 - p;
		long error = kernel_unit;
		for ( int i = 1; i < size; i += blip_res )
		{
			error -= impulses [i + p ];
			error -= impulses [i + p2];
		}
		if ( p == p2 )
			error /= 2; // phase = 0.5 impulse uses same half for both sides
		impulses [size - blip_res + p] += error;
		//printf( "error: %ld\n", error );
	}
	
	//for ( int i = blip_res; i--; printf( "\n" ) )
	//  for ( int j = 0; j < width / 2; j++ )
	//      printf( "%5ld,", impulses [j * blip_res + i + 1] );
}

void Blip_Synth_::treble_eq( blip_eq_t const& eq )
{
	float fimpulse [blip_res / 2 * (blip_widest_impulse_ - 1) + blip_res * 2];
	
	int const half_size = blip_res / 2 * (width - 1);
	eq.generate( &fimpulse [blip_res], half_size );
	
	int i;
	
	// need mirror slightly past center for calculation
	for ( i = blip_res; i--; )
		fimpulse [blip_res + half_size + i] = fimpulse [blip_res + half_size - 1 - i];
	
	// starts at 0
	for ( i = 0; i < blip_res; i++ )
		fimpulse [i] = 0.0f;
	
	// find rescale factor
	double total = 0.0;
	for ( i = 0; i < half_size; i++ )
		total += fimpulse [blip_res + i];
	
	//double const base_unit = 44800.0 - 128 * 18; // allows treble up to +0 dB
	//double const base_unit = 37888.0; // allows treble to +5 dB
	double const base_unit = 32768.0; // necessary for blip_unscaled to work
	double rescale = base_unit / 2 / total;
	kernel_unit = (long) base_unit;
	
	// integrate, first difference, rescale, convert to int
	double sum = 0.0;
	double next = 0.0;
	int const impulses_size = this->impulses_size();
	for ( i = 0; i < impulses_size; i++ )
	{
		impulses [i] = (short) floor( (next - sum) * rescale + 0.5 );
		sum += fimpulse [i];
		next += fimpulse [i + blip_res];
	}
	adjust_impulse();
	
	// volume might require rescaling
	double vol = volume_unit_;
	if ( vol )
	{
		volume_unit_ = 0.0;
		volume_unit( vol );
	}
}

void Blip_Synth_::volume_unit( double new_unit )
{
	if ( new_unit != volume_unit_ )
	{
		// use default eq if it hasn't been set yet
		if ( !kernel_unit )
			treble_eq( -8.0 );
		
		volume_unit_ = new_unit;
		double factor = new_unit * (1L << blip_sample_bits) / kernel_unit;
		
		if ( factor > 0.0 )
		{
			int shift = 0;
			
			// if unit is really small, might need to attenuate kernel
			while ( factor < 2.0 )
			{
				shift++;
				factor *= 2.0;
			}
			
			if ( shift )
			{
				kernel_unit >>= shift;
				assert( kernel_unit > 0 ); // fails if volume unit is too low
				
				// keep values positive to avoid round-towards-zero of sign-preserving
				// right shift for negative values
				long offset = 0x8000 + (1 << (shift - 1));
				long offset2 = 0x8000 >> shift;
				for ( int i = impulses_size(); i--; )
					impulses [i] = (short) (((impulses [i] + offset) >> shift) - offset2);
				adjust_impulse();
			}
		}
		delta_factor = (int) floor( factor + 0.5 );
		//printf( "delta_factor: %d, kernel_unit: %d\n", delta_factor, kernel_unit );
	}
}

long Blip_Buffer::read_samples( blip_sample_t* out, long max_samples, int stereo )
{
	long count = samples_avail();
	if ( count > max_samples )
		count = max_samples;
	
	if ( count )
	{
		int const sample_shift = blip_sample_bits - 16;
		int const bass_shift = this->bass_shift;
		long accum = reader_accum;
		buf_t_* in = buffer_;
		
		if ( !stereo )
		{
			for ( long n = count; n--; )
			{
				long s = accum >> sample_shift;
				accum -= accum >> bass_shift;
				accum += *in++;
				*out++ = (blip_sample_t) s;
				
				// clamp sample
				if ( (blip_sample_t) s != s )
					out [-1] = (blip_sample_t) (0x7FFF - (s >> 24));
			}
		}
		else
		{
			for ( long n = count; n--; )
			{
				long s = accum >> sample_shift;
				accum -= accum >> bass_shift;
				accum += *in++;
				*out = (blip_sample_t) s;
				out += 2;
				
				// clamp sample
				if ( (blip_sample_t) s != s )
					out [-2] = (blip_sample_t) (0x7FFF - (s >> 24));
			}
		}
		
		reader_accum = accum;
		remove_samples( count );
	}
	return count;
}

long Blip_Buffer::read_samples_mix( float* out, long max_samples, int stereo )
{
        long count = samples_avail();
        if ( count > max_samples )
                count = max_samples;

        if ( count )
        {
                int const sample_shift = blip_sample_bits - 16;
                int const bass_shift = this->bass_shift;
                long accum = reader_accum;
                buf_t_* in = buffer_;

                if ( !stereo )
                {
                        for ( long n = count; n--; )
                        {
                                long s = accum >> sample_shift;
                                accum -= accum >> bass_shift;
                                accum += *in++;
                                *out += (float)((blip_sample_t) s) / 32768;

				if(*out >= 1.0f) *out = 1.0f;
				if(*out <= -1.0f) *out = -1.0f;
				out++;
                        }
                }
                else
                {
                        for ( long n = count; n--; )
                        {
                                long s = accum >> sample_shift;
                                accum -= accum >> bass_shift;
                                accum += *in++;
                                *out += (float)((blip_sample_t) s) / 32768;
                                if(*out >= 1.0f) *out = 1.0f;
                                if(*out <= -1.0f) *out = -1.0f;
                                out+=2;
                        }
                }

                reader_accum = accum;
                remove_samples( count );
        }
        return count;
}

void Blip_Buffer::mix_samples( blip_sample_t const* in, long count )
{
	buf_t_* out = buffer_ + (offset_ >> BLIP_BUFFER_ACCURACY) + blip_widest_impulse_ / 2;
	
	int const sample_shift = blip_sample_bits - 16;
	int prev = 0;
	while ( count-- )
	{
		long s = (long) *in++ << sample_shift;
		*out += s - prev;
		prev = s;
		++out;
	}
	*out -= prev;
}
