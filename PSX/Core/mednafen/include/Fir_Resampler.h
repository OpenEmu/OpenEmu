// Finite impulse response (FIR) resampler with adjustable FIR size

// Game_Music_Emu 0.5.2
#ifndef FIR_RESAMPLER_H
#define FIR_RESAMPLER_H

#include "blargg_common.h"
#include <string.h>

class Fir_Resampler_ {
public:
	
	// Use Fir_Resampler<width> (below)
	
	// Set input/output resampling ratio and optionally low-pass rolloff and gain.
	// Returns actual ratio used (rounded to internal precision).
	double time_ratio( double factor, double rolloff = 0.999, double gain = 1.0 );
	
	// Current input/output ratio
	double ratio() const { return ratio_; }
	
// Input
	
	typedef short sample_t;
	
	// Resize and clear input buffer
	blargg_err_t buffer_size( int );
	
	// Clear input buffer. At least two output samples will be available after
	// two input samples are written.
	void clear();
	
	// Number of input samples that can be written
	int max_write() const { return buf.end() - write_pos; }
	
	// Pointer to place to write input samples
	sample_t* buffer() { return write_pos; }
	
	// Notify resampler that 'count' input samples have been written
	void write( long count );
	
	// Number of input samples in buffer
	int written() const { return write_pos - &buf [write_offset]; }
	
	// Skip 'count' input samples. Returns number of samples actually skipped.
	int skip_input( long count );
	
// Output
	
	// Number of extra input samples needed until 'count' output samples are available
	int input_needed( blargg_long count ) const;
	
	// Number of output samples available
	int avail() const { return avail_( write_pos - &buf [width_ * stereo] ); }
	
public:
	~Fir_Resampler_();
protected:
	enum { stereo = 2 };
	enum { max_res = 32 };
	blargg_vector<sample_t> buf;
	sample_t* write_pos;
	int res;
	int imp_phase;
	int const width_;
	int const write_offset;
	blargg_ulong skip_bits;
	int step;
	int input_per_cycle;
	double ratio_;
	sample_t* impulses;
	
	Fir_Resampler_( int width, sample_t* );
	int avail_( blargg_long input_count ) const;
};

// Width is number of points in FIR. Must be even and 4 or more. More points give
// better quality and rolloff effectiveness, and take longer to calculate.
template<int width>
class Fir_Resampler : public Fir_Resampler_ {
	BOOST_STATIC_ASSERT( width >= 4 && width % 2 == 0 );
	short impulses [max_res] [width];
public:
	Fir_Resampler() : Fir_Resampler_( width, impulses [0] ) { }
	
	// Read at most 'count' samples. Returns number of samples actually read.
	typedef short sample_t;
	int read( sample_t* out, blargg_long count );

	int read_mono_hack( sample_t* out, blargg_long count );
};

// End of public interface

inline void Fir_Resampler_::write( long count )
{
	write_pos += count;
	assert( write_pos <= buf.end() );
}

template<int width>
int Fir_Resampler<width>::read( sample_t* out_begin, blargg_long count )
{
	sample_t* out = out_begin;
	const sample_t* in = buf.begin();
	sample_t* end_pos = write_pos;
	blargg_ulong skip = skip_bits >> imp_phase;
	sample_t const* imp = impulses [imp_phase];
	int remain = res - imp_phase;
	int const local_step = this->step;
	
	count >>= 1;
	
	if ( end_pos - in >= width * stereo )
	{
		end_pos -= width * stereo;
		do
		{
			count--;
			
			// accumulate in extended precision
			blargg_long l = 0;
			blargg_long r = 0;
			
			const sample_t* i = in;
			if ( count < 0 )
				break;
			
			for ( int n = width / 2; n; --n )
			{
				int pt0 = imp [0];
				l += pt0 * i [0];
				r += pt0 * i [1];
				int pt1 = imp [1];
				imp += 2;
				l += pt1 * i [2];
				r += pt1 * i [3];
				i += 4;
			}
			
			remain--;
			
			l >>= 15;
			r >>= 15;
			
			in += (skip * stereo) & stereo;
			skip >>= 1;
			in += local_step;
			
			if ( !remain )
			{
				imp = impulses [0];
				skip = skip_bits;
				remain = res;
			}
			
			out [0] = (sample_t) l;
			out [1] = (sample_t) r;
			out += 2;
		}
		while ( in <= end_pos );
	}
	
	imp_phase = res - remain;
	
	int left = write_pos - in;
	write_pos = &buf [left];
	memmove( buf.begin(), in, left * sizeof *in );
	
	return out - out_begin;
}
#include <stdio.h>
template<int width>
int Fir_Resampler<width>::read_mono_hack( sample_t* out_begin, blargg_long count )
{
	sample_t* out = out_begin;
	const sample_t* in = buf.begin();
	sample_t* end_pos = write_pos;
	blargg_ulong skip = skip_bits >> imp_phase;
	sample_t const* imp = impulses [imp_phase];
	int remain = res - imp_phase;
	int const local_step = this->step;
	
	count >>= 1;

	if ( end_pos - in >= width * stereo )
	{
		end_pos -= width * stereo;
		do
		{
			count--;
			
			// accumulate in extended precision
			blargg_long l = 0;
			
			const sample_t* i = in;
			if ( count < 0 )
				break;
			
			for ( int n = width / 2; n; --n )
			{
				int pt0 = imp [0];
				l += pt0 * i [0];

				int pt1 = imp [1];
				imp += 2;
				l += pt1 * i [2];

				i += 4;
			}
			
			remain--;
			
			l >>= 15;
			
			in += (skip * stereo) & stereo;
			skip >>= 1;
			in += local_step;
			
			if ( !remain )
			{
				imp = impulses [0];
				skip = skip_bits;
				remain = res;
			}
			
			*out = (sample_t) l;
			out ++;
		}
		while ( in <= end_pos );
	}
	
	imp_phase = res - remain;
	
	int left = write_pos - in;
	write_pos = &buf [left];
	memmove( buf.begin(), in, left * sizeof *in );
	
	return out - out_begin;
}


#endif
