// Band-limited sound synthesis buffer

// Blip_Buffer 0.4.1
#ifndef BLIP_BUFFER_H
#define BLIP_BUFFER_H

	// internal
	#include <limits.h>
	#if INT_MAX < 0x7FFFFFFF || LONG_MAX == 0x7FFFFFFF
		typedef long blip_long;
		typedef unsigned long blip_ulong;
	#else
		typedef int blip_long;
		typedef unsigned blip_ulong;
	#endif

// Time unit at source clock rate
typedef blip_long blip_time_t;

// Output samples are 16-bit signed, with a range of -32768 to 32767
typedef short blip_sample_t;
enum { blip_sample_max = 32767 };

struct blip_buffer_state_t;

class Blip_Buffer {
public:
	typedef const char* blargg_err_t;

	// Sets output sample rate and buffer length in milliseconds (1/1000 sec, defaults
	// to 1/4 second) and clears buffer. If there isn't enough memory, leaves buffer
	// untouched and returns "Out of memory", otherwise returns NULL.
	blargg_err_t set_sample_rate( long samples_per_sec, int msec_length = 1000 / 4 );

	// Sets number of source time units per second
	void clock_rate( long clocks_per_sec );

	// Ends current time frame of specified duration and makes its samples available
	// (along with any still-unread samples) for reading with read_samples(). Begins
	// a new time frame at the end of the current frame.
	void end_frame( blip_time_t time );

	// Reads at most 'max_samples' out of buffer into 'dest', removing them from
	// the buffer. Returns number of samples actually read and removed. If stereo is
	// true, increments 'dest' one extra time after writing each sample, to allow
	// easy interleving of two channels into a stereo output buffer.
	long read_samples( blip_sample_t* dest, long max_samples, int stereo = 0 );

// Additional features

	// Removes all available samples and clear buffer to silence. If 'entire_buffer' is
	// false, just clears out any samples waiting rather than the entire buffer.
	void clear( int entire_buffer = 1 );

	// Number of samples available for reading with read_samples()
	long samples_avail() const;

	// Removes 'count' samples from those waiting to be read
	void remove_samples( long count );

	// Sets frequency high-pass filter frequency, where higher values reduce bass more
	void bass_freq( int frequency );

	// Current output sample rate
	long sample_rate() const;

	// Length of buffer in milliseconds
	int length() const;

	// Number of source time units per second
	long clock_rate() const;

// Experimental features

	// Saves state, including high-pass filter and tails of last deltas.
	// All samples must have been read from buffer before calling this.
	void save_state( blip_buffer_state_t* out );

	// Loads state. State must have been saved from Blip_Buffer with same
	// settings during same run of program. States can NOT be stored on disk.
	// Clears buffer before loading state.
	void load_state( blip_buffer_state_t const& in );

	// Number of samples delay from synthesis to samples read out
	int output_latency() const;

	// Counts number of clocks needed until 'count' samples will be available.
	// If buffer can't even hold 'count' samples, returns number of clocks until
	// buffer becomes full.
	blip_time_t count_clocks( long count ) const;

	// Number of raw samples that can be mixed within frame of specified duration.
	long count_samples( blip_time_t duration ) const;

	// Mixes in 'count' samples from 'buf_in'
	void mix_samples( blip_sample_t const* buf_in, long count );


	// Signals that sound has been added to buffer. Could be done automatically in
	// Blip_Synth, but that would affect performance more, as you can arrange that
	// this is called only once per time frame rather than for every delta.
	void set_modified() { modified_ = this; }

	// not documented yet
	blip_ulong unsettled() const;
	Blip_Buffer* clear_modified() { Blip_Buffer* b = modified_; modified_ = 0; return b; }
	void remove_silence( long count );
	typedef blip_ulong blip_resampled_time_t;
	blip_resampled_time_t resampled_duration( int t ) const     { return t * factor_; }
	blip_resampled_time_t resampled_time( blip_time_t t ) const { return t * factor_ + offset_; }
	blip_resampled_time_t clock_rate_factor( long clock_rate ) const;
public:
	Blip_Buffer();
	~Blip_Buffer();

	// Deprecated
	typedef blip_resampled_time_t resampled_time_t;
	blargg_err_t sample_rate( long r ) { return set_sample_rate( r ); }
	blargg_err_t sample_rate( long r, int msec ) { return set_sample_rate( r, msec ); }
private:
	// noncopyable
	Blip_Buffer( const Blip_Buffer& );
	Blip_Buffer& operator = ( const Blip_Buffer& );
public:
	typedef blip_long buf_t_;
	blip_ulong factor_;
	blip_resampled_time_t offset_;
	buf_t_* buffer_;
	blip_long buffer_size_;
	blip_long reader_accum_;
	int bass_shift_;
private:
	long sample_rate_;
	long clock_rate_;
	int bass_freq_;
	int length_;
	Blip_Buffer* modified_; // non-zero = true (more optimal than using bool, heh)
	friend class Blip_Reader;
};

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

// Number of bits in resample ratio fraction. Higher values give a more accurate ratio
// but reduce maximum buffer size.
#ifndef BLIP_BUFFER_ACCURACY
	#define BLIP_BUFFER_ACCURACY 16
#endif

// Number bits in phase offset. Fewer than 6 bits (64 phase offsets) results in
// noticeable broadband noise when synthesizing high frequency square waves.
// Affects size of Blip_Synth objects since they store the waveform directly.
#ifndef BLIP_PHASE_BITS
	#if BLIP_BUFFER_FAST
		#define BLIP_PHASE_BITS 8
	#else
		#define BLIP_PHASE_BITS 6
	#endif
#endif

	// Internal
	typedef blip_ulong blip_resampled_time_t;
	int const blip_widest_impulse_ = 16;
	int const blip_buffer_extra_ = blip_widest_impulse_ + 2;
	int const blip_res = 1 << BLIP_PHASE_BITS;
	class blip_eq_t;

	class Blip_Synth_Fast_ {
	public:
		Blip_Buffer* buf;
		int last_amp;
		int delta_factor;

		void volume_unit( double );
		Blip_Synth_Fast_();
		void treble_eq( blip_eq_t const& ) { }
	};

	class Blip_Synth_ {
	public:
		Blip_Buffer* buf;
		int last_amp;
		int delta_factor;

		void volume_unit( double );
		Blip_Synth_( short* impulses, int width );
		void treble_eq( blip_eq_t const& );
	private:
		double volume_unit_;
		short* const impulses;
		int const width;
		blip_long kernel_unit;
		int impulses_size() const { return blip_res / 2 * width + 1; }
		void adjust_impulse();
	};

// Quality level, better = slower. In general, use blip_good_quality.
const int blip_med_quality  = 8;
const int blip_good_quality = 12;
const int blip_high_quality = 16;

// Range specifies the greatest expected change in amplitude. Calculate it
// by finding the difference between the maximum and minimum expected
// amplitudes (max - min).
template<int quality,int range>
class Blip_Synth {
public:
	// Sets overall volume of waveform
	void volume( double v ) { impl.volume_unit( v * (1.0 / (range < 0 ? -range : range)) ); }

	// Configures low-pass filter (see blip_buffer.txt)
	void treble_eq( blip_eq_t const& eq )       { impl.treble_eq( eq ); }

	// Gets/sets Blip_Buffer used for output
	Blip_Buffer* output() const                 { return impl.buf; }
	void output( Blip_Buffer* b )               { impl.buf = b; impl.last_amp = 0; }

	// Updates amplitude of waveform at given time. Using this requires a separate
	// Blip_Synth for each waveform.
	void update( blip_time_t time, int amplitude );

// Low-level interface

	// Adds an amplitude transition of specified delta, optionally into specified buffer
	// rather than the one set with output(). Delta can be positive or negative.
	// The actual change in amplitude is delta * (volume / range)
	void offset( blip_time_t, int delta, Blip_Buffer* ) const;
	void offset( blip_time_t t, int delta ) const { offset( t, delta, impl.buf ); }

	// Works directly in terms of fractional output samples. Contact author for more info.
	void offset_resampled( blip_resampled_time_t, int delta, Blip_Buffer* ) const;

	// Same as offset(), except code is inlined for higher performance
	void offset_inline( blip_time_t t, int delta, Blip_Buffer* buf ) const {
		offset_resampled( t * buf->factor_ + buf->offset_, delta, buf );
	}
	void offset_inline( blip_time_t t, int delta ) const {
		offset_resampled( t * impl.buf->factor_ + impl.buf->offset_, delta, impl.buf );
	}

private:
#if BLIP_BUFFER_FAST
	Blip_Synth_Fast_ impl;
#else
	Blip_Synth_ impl;
	typedef short imp_t;
	imp_t impulses [blip_res * (quality / 2) + 1];
public:
	Blip_Synth() : impl( impulses, quality ) { }
#endif
};

// Low-pass equalization parameters
class blip_eq_t {
public:
	// Logarithmic rolloff to treble dB at half sampling rate. Negative values reduce
	// treble, small positive values (0 to 5.0) increase treble.
	blip_eq_t( double treble_db = 0 );

	// See blip_buffer.txt
	blip_eq_t( double treble, long rolloff_freq, long sample_rate, long cutoff_freq = 0 );

private:
	double treble;
	long rolloff_freq;
	long sample_rate;
	long cutoff_freq;
	void generate( float* out, int count ) const;
	friend class Blip_Synth_;
};

int const blip_sample_bits = 30;

// Dummy Blip_Buffer to direct sound output to, for easy muting without
// having to stop sound code.
class Silent_Blip_Buffer : public Blip_Buffer {
	buf_t_ buf [blip_buffer_extra_ + 1];
public:
	// The following cannot be used (an assertion will fail if attempted):
	blargg_err_t set_sample_rate( long samples_per_sec, int msec_length );
	blip_time_t count_clocks( long count ) const;
	void mix_samples( blip_sample_t const* buf, long count );

	Silent_Blip_Buffer();
};

	#if __GNUC__ >= 3 || _MSC_VER >= 1100
		#define BLIP_RESTRICT __restrict
	#else
		#define BLIP_RESTRICT
	#endif

// Optimized reading from Blip_Buffer, for use in custom sample output

// Begins reading from buffer. Name should be unique to the current block.
#define BLIP_READER_BEGIN( name, blip_buffer ) \
	const Blip_Buffer::buf_t_* BLIP_RESTRICT name##_reader_buf = (blip_buffer).buffer_;\
	blip_long name##_reader_accum = (blip_buffer).reader_accum_

// Gets value to pass to BLIP_READER_NEXT()
#define BLIP_READER_BASS( blip_buffer ) ((blip_buffer).bass_shift_)

// Constant value to use instead of BLIP_READER_BASS(), for slightly more optimal
// code at the cost of having no bass control
int const blip_reader_default_bass = 9;

// Current sample
#define BLIP_READER_READ( name )        (name##_reader_accum >> (blip_sample_bits - 16))

// Current raw sample in full internal resolution
#define BLIP_READER_READ_RAW( name )    (name##_reader_accum)

// Advances to next sample
#define BLIP_READER_NEXT( name, bass ) \
	(void) (name##_reader_accum += *name##_reader_buf++ - (name##_reader_accum >> (bass)))

// Ends reading samples from buffer. The number of samples read must now be removed
// using Blip_Buffer::remove_samples().
#define BLIP_READER_END( name, blip_buffer ) \
	(void) ((blip_buffer).reader_accum_ = name##_reader_accum)


// experimental
#define BLIP_READER_ADJ_( name, offset ) (name##_reader_buf += offset)

blip_long const blip_reader_idx_factor = sizeof (Blip_Buffer::buf_t_);

#define BLIP_READER_NEXT_IDX_( name, bass, idx ) {\
	name##_reader_accum -= name##_reader_accum >> (bass);\
	name##_reader_accum += name##_reader_buf [(idx)];\
}

#define BLIP_READER_NEXT_RAW_IDX_( name, bass, idx ) {\
	name##_reader_accum -= name##_reader_accum >> (bass);\
	name##_reader_accum +=\
			*(Blip_Buffer::buf_t_ const*) ((char const*) name##_reader_buf + (idx));\
}

// Compatibility with older version
const long blip_unscaled = 65535;
const int blip_low_quality  = blip_med_quality;
const int blip_best_quality = blip_high_quality;

// Deprecated; use BLIP_READER macros as follows:
// Blip_Reader r; r.begin( buf ); -> BLIP_READER_BEGIN( r, buf );
// int bass = r.begin( buf )      -> BLIP_READER_BEGIN( r, buf ); int bass = BLIP_READER_BASS( buf );
// r.read()                       -> BLIP_READER_READ( r )
// r.read_raw()                   -> BLIP_READER_READ_RAW( r )
// r.next( bass )                 -> BLIP_READER_NEXT( r, bass )
// r.next()                       -> BLIP_READER_NEXT( r, blip_reader_default_bass )
// r.end( buf )                   -> BLIP_READER_END( r, buf )
class Blip_Reader {
public:
	int begin( Blip_Buffer& );
	blip_long read() const          { return accum >> (blip_sample_bits - 16); }
	blip_long read_raw() const      { return accum; }
	void next( int bass_shift = 9 ) { accum += *buf++ - (accum >> bass_shift); }
	void end( Blip_Buffer& b )      { b.reader_accum_ = accum; }
private:
	const Blip_Buffer::buf_t_* buf;
	blip_long accum;
};

#if defined (_M_IX86) || defined (_M_IA64) || defined (__i486__) || \
		defined (__x86_64__) || defined (__ia64__) || defined (__i386__)
	#define BLIP_CLAMP_( in ) in < -0x8000 || 0x7FFF < in
#else
	#define BLIP_CLAMP_( in ) (blip_sample_t) in != in
#endif

// Clamp sample to blip_sample_t range
#define BLIP_CLAMP( sample, out )\
	{ if ( BLIP_CLAMP_( (sample) ) ) (out) = ((sample) >> 24) ^ 0x7FFF; }

struct blip_buffer_state_t
{
	blip_resampled_time_t offset_;
	blip_long reader_accum_;
	blip_long buf [blip_buffer_extra_];
};

// End of public interface

#ifndef assert
	#include <assert.h>
#endif

template<int quality,int range>
inline void Blip_Synth<quality,range>::offset_resampled( blip_resampled_time_t time,
		int delta, Blip_Buffer* blip_buf ) const
{
	// If this assertion fails, it means that an attempt was made to add a delta
	// at a negative time or past the end of the buffer.
	assert( (blip_long) (time >> BLIP_BUFFER_ACCURACY) < blip_buf->buffer_size_ );

	delta *= impl.delta_factor;
	blip_long* BLIP_RESTRICT buf = blip_buf->buffer_ + (time >> BLIP_BUFFER_ACCURACY);
	int phase = (int) (time >> (BLIP_BUFFER_ACCURACY - BLIP_PHASE_BITS) & (blip_res - 1));

#if BLIP_BUFFER_FAST
	blip_long left = buf [0] + delta;

	// Kind of crappy, but doing shift after multiply results in overflow.
	// Alternate way of delaying multiply by delta_factor results in worse
	// sub-sample resolution.
	blip_long right = (delta >> BLIP_PHASE_BITS) * phase;
	left  -= right;
	right += buf [1];

	buf [0] = left;
	buf [1] = right;
#else

	int const fwd = (blip_widest_impulse_ - quality) / 2;
	int const rev = fwd + quality - 2;
	int const mid = quality / 2 - 1;

	imp_t const* BLIP_RESTRICT imp = impulses + blip_res - phase;

	#if defined (_M_IX86) || defined (_M_IA64) || defined (__i486__) || \
			defined (__x86_64__) || defined (__ia64__) || defined (__i386__)

	// this straight forward version gave in better code on GCC for x86

	#define ADD_IMP( out, in ) \
		buf [out] += (blip_long) imp [blip_res * (in)] * delta

	#define BLIP_FWD( i ) {\
		ADD_IMP( fwd     + i, i     );\
		ADD_IMP( fwd + 1 + i, i + 1 );\
	}
	#define BLIP_REV( r ) {\
		ADD_IMP( rev     - r, r + 1 );\
		ADD_IMP( rev + 1 - r, r     );\
	}

		BLIP_FWD( 0 )
		if ( quality > 8  ) BLIP_FWD( 2 )
		if ( quality > 12 ) BLIP_FWD( 4 )
		{
			ADD_IMP( fwd + mid - 1, mid - 1 );
			ADD_IMP( fwd + mid    , mid     );
			imp = impulses + phase;
		}
		if ( quality > 12 ) BLIP_REV( 6 )
		if ( quality > 8  ) BLIP_REV( 4 )
		BLIP_REV( 2 )

		ADD_IMP( rev    , 1 );
		ADD_IMP( rev + 1, 0 );

	#undef ADD_IMP

	#else

	// for RISC processors, help compiler by reading ahead of writes

	#define BLIP_FWD( i ) {\
		blip_long t0 =                       i0 * delta + buf [fwd     + i];\
		blip_long t1 = imp [blip_res * (i + 1)] * delta + buf [fwd + 1 + i];\
		i0 =           imp [blip_res * (i + 2)];\
		buf [fwd     + i] = t0;\
		buf [fwd + 1 + i] = t1;\
	}
	#define BLIP_REV( r ) {\
		blip_long t0 =                 i0 * delta + buf [rev     - r];\
		blip_long t1 = imp [blip_res * r] * delta + buf [rev + 1 - r];\
		i0 =           imp [blip_res * (r - 1)];\
		buf [rev     - r] = t0;\
		buf [rev + 1 - r] = t1;\
	}

		blip_long i0 = *imp;
		BLIP_FWD( 0 )
		if ( quality > 8  ) BLIP_FWD( 2 )
		if ( quality > 12 ) BLIP_FWD( 4 )
		{
			blip_long t0 =                   i0 * delta + buf [fwd + mid - 1];
			blip_long t1 = imp [blip_res * mid] * delta + buf [fwd + mid    ];
			imp = impulses + phase;
			i0 = imp [blip_res * mid];
			buf [fwd + mid - 1] = t0;
			buf [fwd + mid    ] = t1;
		}
		if ( quality > 12 ) BLIP_REV( 6 )
		if ( quality > 8  ) BLIP_REV( 4 )
		BLIP_REV( 2 )

		blip_long t0 =   i0 * delta + buf [rev    ];
		blip_long t1 = *imp * delta + buf [rev + 1];
		buf [rev    ] = t0;
		buf [rev + 1] = t1;
	#endif

#endif
}

#undef BLIP_FWD
#undef BLIP_REV

template<int quality,int range>
#if BLIP_BUFFER_FAST
	inline
#endif
void Blip_Synth<quality,range>::offset( blip_time_t t, int delta, Blip_Buffer* buf ) const
{
	offset_resampled( t * buf->factor_ + buf->offset_, delta, buf );
}

template<int quality,int range>
#if BLIP_BUFFER_FAST
	inline
#endif
void Blip_Synth<quality,range>::update( blip_time_t t, int amp )
{
	int delta = amp - impl.last_amp;
	impl.last_amp = amp;
	offset_resampled( t * impl.buf->factor_ + impl.buf->offset_, delta, impl.buf );
}

inline blip_eq_t::blip_eq_t( double t ) :
		treble( t ), rolloff_freq( 0 ), sample_rate( 44100 ), cutoff_freq( 0 ) { }
inline blip_eq_t::blip_eq_t( double t, long rf, long sr, long cf ) :
		treble( t ), rolloff_freq( rf ), sample_rate( sr ), cutoff_freq( cf ) { }

inline int  Blip_Buffer::length() const         { return length_; }
inline long Blip_Buffer::samples_avail() const  { return (long) (offset_ >> BLIP_BUFFER_ACCURACY); }
inline long Blip_Buffer::sample_rate() const    { return sample_rate_; }
inline int  Blip_Buffer::output_latency() const { return blip_widest_impulse_ / 2; }
inline long Blip_Buffer::clock_rate() const     { return clock_rate_; }
inline void Blip_Buffer::clock_rate( long cps ) { factor_ = clock_rate_factor( clock_rate_ = cps ); }

inline int Blip_Reader::begin( Blip_Buffer& blip_buf )
{
	buf   = blip_buf.buffer_;
	accum = blip_buf.reader_accum_;
	return blip_buf.bass_shift_;
}

inline void Blip_Buffer::remove_silence( long count )
{
	// fails if you try to remove more samples than available
	assert( count <= samples_avail() );
	offset_ -= (blip_resampled_time_t) count << BLIP_BUFFER_ACCURACY;
}

inline blip_ulong Blip_Buffer::unsettled() const
{
	return reader_accum_ >> (blip_sample_bits - 16);
}

int const blip_max_length = 0;
int const blip_default_length = 250; // 1/4 second

#endif
