// Blip_Buffer 0.4.1. http://www.slack.net/~ant/

#include "Blip_Buffer.h"

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

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

// TODO: use scoped for variables in treble_eq()

#ifdef BLARGG_ENABLE_OPTIMIZER
	#include BLARGG_ENABLE_OPTIMIZER
#endif

int const silent_buf_size = 1; // size used for Silent_Blip_Buffer

Blip_Buffer::Blip_Buffer()
{
	factor_       = LONG_MAX;
	buffer_       = 0;
	buffer_size_  = 0;
	sample_rate_  = 0;
	bass_shift_   = 0;
	clock_rate_   = 0;
	bass_freq_    = 16;
	length_       = 0;

	// assumptions code makes about implementation-defined features
	#ifndef NDEBUG
		// right shift of negative value preserves sign
		buf_t_ i = -0x7FFFFFFE;
		assert( (i >> 1) == -0x3FFFFFFF );

		// casting to short truncates to 16 bits and sign-extends
		i = 0x18000;
		assert( (short) i == -0x8000 );
	#endif

	clear();
}

Blip_Buffer::~Blip_Buffer()
{
	if ( buffer_size_ != silent_buf_size )
		free( buffer_ );
}

Silent_Blip_Buffer::Silent_Blip_Buffer()
{
	factor_      = 0;
	buffer_      = buf;
	buffer_size_ = silent_buf_size;
	clear();
}

void Blip_Buffer::clear( int entire_buffer )
{
	offset_       = 0;
	reader_accum_ = 0;
	modified_     = 0;
	if ( buffer_ )
	{
		long count = (entire_buffer ? buffer_size_ : samples_avail());
		memset( buffer_, 0, (count + blip_buffer_extra_) * sizeof (buf_t_) );
	}
}

Blip_Buffer::blargg_err_t Blip_Buffer::set_sample_rate( long new_rate, int msec )
{
	if ( buffer_size_ == silent_buf_size )
	{
		assert( 0 );
		return "Internal (tried to resize Silent_Blip_Buffer)";
	}

	// start with maximum length that resampled time can represent
	long new_size = (ULONG_MAX >> BLIP_BUFFER_ACCURACY) - blip_buffer_extra_ - 64;
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
		void* p = realloc( buffer_, (new_size + blip_buffer_extra_) * sizeof *buffer_ );
		if ( !p )
			return "Out of memory";
		buffer_ = (buf_t_*) p;
	}

	buffer_size_ = new_size;
	assert( buffer_size_ != silent_buf_size ); // size should never happen to match this

	// update things based on the sample rate
	sample_rate_ = new_rate;
	length_ = new_size * 1000 / new_rate - 1;
	if ( msec )
		assert( length_ == msec ); // ensure length is same as that passed in

	// update these since they depend on sample rate
	if ( clock_rate_ )
		clock_rate( clock_rate_ );
	bass_freq( bass_freq_ );

	clear();

	return 0; // success
}

blip_resampled_time_t Blip_Buffer::clock_rate_factor( long rate ) const
{
	double ratio = (double) sample_rate_ / rate;
	blip_long factor = (blip_long) floor( ratio * (1L << BLIP_BUFFER_ACCURACY) + 0.5 );
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
	bass_shift_ = shift;
}

void Blip_Buffer::end_frame( blip_time_t t )
{
	offset_ += t * factor_;
	assert( samples_avail() <= (long) buffer_size_ ); // fails if time is past end of buffer
}

long Blip_Buffer::count_samples( blip_time_t t ) const
{
	blip_resampled_time_t last_sample  = resampled_time( t ) >> BLIP_BUFFER_ACCURACY;
	blip_resampled_time_t first_sample = offset_ >> BLIP_BUFFER_ACCURACY;
	return long (last_sample - first_sample);
}

blip_time_t Blip_Buffer::count_clocks( long count ) const
{
	if ( !factor_ )
	{
		assert( 0 ); // sample rate and clock rates must be set first
		return 0;
	}

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
		long remain = samples_avail() + blip_buffer_extra_;
		memmove( buffer_, buffer_ + count, remain * sizeof *buffer_ );
		memset( buffer_ + remain, 0, count * sizeof *buffer_ );
	}
}

// Blip_Synth_

Blip_Synth_Fast_::Blip_Synth_Fast_()
{
	buf          = 0;
	last_amp     = 0;
	delta_factor = 0;
}

void Blip_Synth_Fast_::volume_unit( double new_unit )
{
	delta_factor = int (new_unit * (1L << blip_sample_bits) + 0.5);
}

#if !BLIP_BUFFER_FAST

Blip_Synth_::Blip_Synth_( short* p, int w ) :
	impulses( p ),
	width( w )
{
	volume_unit_ = 0.0;
	kernel_unit  = 0;
	buf          = 0;
	last_amp     = 0;
	delta_factor = 0;
}

#undef PI
#define PI 3.1415926535897932384626433832795029

static void gen_sinc( float* out, int count, double oversample, double treble, double cutoff )
{
	if ( cutoff >= 0.999 )
		cutoff = 0.999;

	if ( treble < -300.0 )
		treble = -300.0;
	if ( treble > 5.0 )
		treble = 5.0;

	double const maxh = 4096.0;
	double const rolloff = pow( 10.0, 1.0 / (maxh * 20.0) * treble / (1.0 - cutoff) );
	double const pow_a_n = pow( rolloff, maxh - maxh * cutoff );
	double const to_angle = PI / 2 / maxh / oversample;
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
	double to_fraction = PI / (count - 1);
	for ( int i = count; i--; )
		out [i] *= 0.54f - 0.46f * (float) cos( i * to_fraction );
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
		impulses [size - blip_res + p] += (short) error;
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
	int const size = this->impulses_size();
	for ( i = 0; i < size; i++ )
	{
		impulses [i] = (short) (int) floor( (next - sum) * rescale + 0.5 );
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
					impulses [i] = (short) (int) (((impulses [i] + offset) >> shift) - offset2);
				adjust_impulse();
			}
		}
		delta_factor = (int) floor( factor + 0.5 );
		//printf( "delta_factor: %d, kernel_unit: %d\n", delta_factor, kernel_unit );
	}
}
#endif

long Blip_Buffer::read_samples( blip_sample_t* out_, long max_samples, int stereo )
{
	long count = samples_avail();
	if ( count > max_samples )
		count = max_samples;

	if ( count )
	{
		int const bass = BLIP_READER_BASS( *this );
		BLIP_READER_BEGIN( reader, *this );
		BLIP_READER_ADJ_( reader, count );
		blip_sample_t* BLIP_RESTRICT out = out_ + count;
		blip_long offset = (blip_long) -count;

		if ( !stereo )
		{
			do
			{
				blip_long s = BLIP_READER_READ( reader );
				BLIP_READER_NEXT_IDX_( reader, bass, offset );
				BLIP_CLAMP( s, s );
				out [offset] = (blip_sample_t) s;
			}
			while ( ++offset );
		}
		else
		{
			do
			{
				blip_long s = BLIP_READER_READ( reader );
				BLIP_READER_NEXT_IDX_( reader, bass, offset );
				BLIP_CLAMP( s, s );
				out [offset * 2] = (blip_sample_t) s;
			}
			while ( ++offset );
		}

		BLIP_READER_END( reader, *this );

		remove_samples( count );
	}
	return count;
}

void Blip_Buffer::mix_samples( blip_sample_t const* in, long count )
{
	if ( buffer_size_ == silent_buf_size )
	{
		assert( 0 );
		return;
	}

	buf_t_* out = buffer_ + (offset_ >> BLIP_BUFFER_ACCURACY) + blip_widest_impulse_ / 2;

	int const sample_shift = blip_sample_bits - 16;
	int prev = 0;
	while ( count-- )
	{
		blip_long s = (blip_long) *in++ << sample_shift;
		*out += s - prev;
		prev = s;
		++out;
	}
	*out -= prev;
}

blip_ulong const subsample_mask = (1L << BLIP_BUFFER_ACCURACY) - 1;

void Blip_Buffer::save_state( blip_buffer_state_t* out )
{
	assert( samples_avail() == 0 );
	out->offset_       = offset_;
	out->reader_accum_ = reader_accum_;
	memcpy( out->buf, &buffer_ [offset_ >> BLIP_BUFFER_ACCURACY], sizeof out->buf );
}

void Blip_Buffer::load_state( blip_buffer_state_t const& in )
{
	clear( false );

	offset_       = in.offset_;
	reader_accum_ = in.reader_accum_;
	memcpy( buffer_, in.buf, sizeof in.buf );
}
