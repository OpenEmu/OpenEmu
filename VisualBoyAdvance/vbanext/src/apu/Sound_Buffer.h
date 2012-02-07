/* Multi-channel sound buffer interface, and basic mono and stereo buffers */

#ifndef MULTI_BUFFER_H
#define MULTI_BUFFER_H

#define STEREO 2
#define EXTRA_CHANS 4
#define BUFS_SIZE 3

#include "../System.h"

#include "blargg_common.h"

/* BLIP BUFFER */

struct blip_buffer_state_t;

class Blip_Buffer
{
	public:
	// Sets output sample rate and buffer length in milliseconds (1/1000 sec, defaults
	// to 1/4 second) and clears buffer. If there isn't enough memory, leaves buffer
	// untouched and returns "Out of memory", otherwise returns NULL.
	const char * set_sample_rate( long samples_per_sec, int msec_length = 1000 / 4 );

	// Reads at most 'max_samples' out of buffer into 'dest', removing them from
	// the buffer. Returns number of samples actually read and removed. If stereo is
	// true, increments 'dest' one extra time after writing each sample, to allow
	// easy interleving of two channels into a stereo output buffer.
	long read_samples( int16_t* dest, long count);

	/* Additional features*/

	/* Removes all available samples and clear buffer to silence.*/
	void clear( void);

	/* Removes 'count' samples from those waiting to be read*/
	void remove_samples( long count );

	/* Experimental features*/

	/* Saves state, including high-pass filter and tails of last deltas.*/
	/* All samples must have been read from buffer before calling this.*/
	void save_state( blip_buffer_state_t* out );

	// Loads state. State must have been saved from Blip_Buffer with same
	// settings during same run of program. States can NOT be stored on disk.
	// Clears buffer before loading state.
	void load_state( blip_buffer_state_t const& in );

	/* not documented yet*/
	uint32_t clock_rate_factor( long clock_rate ) const;
	long clock_rate_;

	int length_;		/* Length of buffer in milliseconds*/
	long sample_rate_;	/* Current output sample rate*/
	uint32_t factor_;
	uint32_t offset_;
	int32_t * buffer_;
	int32_t buffer_size_;
	int32_t reader_accum_;
	Blip_Buffer();
	~Blip_Buffer();
	private:
	Blip_Buffer( const Blip_Buffer& );
	Blip_Buffer& operator = ( const Blip_Buffer& );
};

/* Number of bits in resample ratio fraction. Higher values give a more accurate 
   ratio but reduce maximum buffer size.*/
#define BLIP_BUFFER_ACCURACY 16

/* Number bits in phase offset. Fewer than 6 bits (64 phase offsets) results in
   noticeable broadband noise when synthesizing high frequency square waves.
   Affects size of Blip_Synth objects since they store the waveform directly. */
#define BLIP_PHASE_BITS 8

/* Internal*/
#define BLIP_WIDEST_IMPULSE_ 16
#define BLIP_BUFFER_EXTRA_ 18
#define BLIP_RES 256
#define BLIP_RES_MIN_ONE 255

#define BLIP_SAMPLE_BITS 30

/* Constant value to use instead of BLIP_READER_BASS(), for slightly more optimal
   code at the cost of having no bass control */
#define BLIP_READER_DEFAULT_BASS 9

class Blip_Synth
{
	public:
	Blip_Buffer* buf;
	int delta_factor;

	Blip_Synth();

	/* Sets overall volume of waveform */
	void volume( double v ) { delta_factor = int ((v * 1.0) * (1L << BLIP_SAMPLE_BITS) + 0.5); }

	// Low-level interface
	// Adds an amplitude transition of specified delta, optionally into specified buffer
	// rather than the one set with output(). Delta can be positive or negative.
	// The actual change in amplitude is delta * (volume / range)
	void offset( int32_t, int delta, Blip_Buffer* ) const;

	/* Works directly in terms of fractional output samples. Contact author for more info.*/
	void offset_resampled( uint32_t, int delta, Blip_Buffer* ) const;

	/* Same as offset(), except code is inlined for higher performance*/
	void offset_inline( int32_t t, int delta, Blip_Buffer* buf ) const {
		offset_resampled( t * buf->factor_ + buf->offset_, delta, buf );
	}
	void offset_inline( int32_t t, int delta ) const {
		offset_resampled( t * buf->factor_ + buf->offset_, delta, buf );
	}
};

/* Optimized reading from Blip_Buffer, for use in custom sample output*/

/* Begins reading from buffer. Name should be unique to the current block.*/
#define BLIP_READER_BEGIN( name, blip_buffer ) \
        const int32_t * name##_reader_buf = (blip_buffer).buffer_;\
        int32_t name##_reader_accum = (blip_buffer).reader_accum_


/* Current sample */
#define BLIP_READER_READ( name )        (name##_reader_accum >> 14)

/* Advances to next sample*/
#define BLIP_READER_NEXT( name, bass ) \
        (void) (name##_reader_accum += *name##_reader_buf++ - (name##_reader_accum >> (bass)))

/* Ends reading samples from buffer. The number of samples read must now be removed
   using Blip_Buffer::remove_samples(). */
#define BLIP_READER_END( name, blip_buffer ) \
        (void) ((blip_buffer).reader_accum_ = name##_reader_accum)

#define BLIP_READER_ADJ_( name, offset ) (name##_reader_buf += offset)

#define BLIP_READER_NEXT_IDX_( name, idx ) {\
        name##_reader_accum -= name##_reader_accum >> BLIP_READER_DEFAULT_BASS;\
        name##_reader_accum += name##_reader_buf [(idx)];\
}

#define BLIP_READER_NEXT_RAW_IDX_( name, idx ) {\
        name##_reader_accum -= name##_reader_accum >> BLIP_READER_DEFAULT_BASS; \
        name##_reader_accum += *(int32_t const*) ((char const*) name##_reader_buf + (idx)); \
}

#if defined (_M_IX86) || defined (_M_IA64) || defined (__i486__) || \
                defined (__x86_64__) || defined (__ia64__) || defined (__i386__)
        #define BLIP_CLAMP_( in ) in < -0x8000 || 0x7FFF < in
#else
        #define BLIP_CLAMP_( in ) (int16_t) in != in
#endif

/* Clamp sample to int16_t range */
#define BLIP_CLAMP( sample, out ) { if ( BLIP_CLAMP_( (sample) ) ) (out) = ((sample) >> 24) ^ 0x7FFF; }

struct blip_buffer_state_t
{
        uint32_t offset_;
        int32_t reader_accum_;
        int32_t buf [BLIP_BUFFER_EXTRA_];
};

INLINE void Blip_Synth::offset_resampled( uint32_t time, int delta, Blip_Buffer* blip_buf ) const
{
	int32_t left, right, phase;
	int32_t *buf;

	delta *= delta_factor;
	buf = blip_buf->buffer_ + (time >> BLIP_BUFFER_ACCURACY);
	phase = (int) (time >> (BLIP_BUFFER_ACCURACY - BLIP_PHASE_BITS) & BLIP_RES_MIN_ONE);

	left = buf [0] + delta;

	/* Kind of crappy, but doing shift after multiply results in overflow.
	Alternate way of delaying multiply by delta_factor results in worse
	sub-sample resolution.*/
	right = (delta >> BLIP_PHASE_BITS) * phase;

	left  -= right;
	right += buf [1];

	buf [0] = left;
	buf [1] = right;
}

INLINE void Blip_Synth::offset( int32_t t, int delta, Blip_Buffer* buf ) const
{
        offset_resampled( t * buf->factor_ + buf->offset_, delta, buf );
}

/* 1/4th of a second */
#define BLIP_DEFAULT_LENGTH 250

#define	WAVE_TYPE	0x100
#define NOISE_TYPE	0x200
#define MIXED_TYPE	WAVE_TYPE | NOISE_TYPE
#define TYPE_INDEX_MASK	0xFF

struct channel_t {
	Blip_Buffer* center;
	Blip_Buffer* left;
	Blip_Buffer* right;
};

/* Uses three buffers (one for center) and outputs stereo sample pairs. */

#define STEREO_BUFFER_SAMPLES_AVAILABLE() ((long)(bufs_buffer[0].offset_ -  mixer_samples_read) << 1)

class Stereo_Buffer
{
	public:
		Stereo_Buffer();
		~Stereo_Buffer();
		const char * set_sample_rate( long, int msec = BLIP_DEFAULT_LENGTH);
		void clock_rate( long );
		void clear();
		long samples_avail() { return ((bufs_buffer [0].offset_ >> BLIP_BUFFER_ACCURACY) - mixer_samples_read) << 1; }
		long read_samples( int16_t*, long );
		void mixer_read_pairs( int16_t* out, int count );
		Blip_Buffer bufs_buffer [BUFS_SIZE];
	private:
		int mixer_samples_read;
};


/* See Simple_Effects_Buffer (below) for a simpler interface */

class Effects_Buffer {
	public:
		// To reduce memory usage, fewer buffers can be used (with a best-fit
		// approach if there are too few), and maximum echo delay can be reduced
		Effects_Buffer( int max_bufs = 32, long echo_size = 24 * 1024L );
		virtual ~Effects_Buffer();

		struct pan_vol_t
		{
			float vol; // 0.0 = silent, 0.5 = half volume, 1.0 = normal
			float pan; // -1.0 = left, 0.0 = center, +1.0 = right
		};

		// Global configuration
		struct config_t
		{
			bool enabled; // false = disable all effects

			// Current sound is echoed at adjustable left/right delay,
			// with reduced treble and volume (feedback).
			float treble;   // 1.0 = full treble, 0.1 = very little, 0.0 = silent
			int delay [2];  // left, right delays (msec)
			float feedback; // 0.0 = no echo, 0.5 = each echo half previous, 1.0 = cacophony
			pan_vol_t side_chans [2]; // left and right side channel volume and pan
		};
		config_t& config() { return config_; }

		// Per-channel configuration. Two or more channels with matching parameters are
		// optimized to internally use the same buffer.
		struct chan_config_t : pan_vol_t
		{
			bool surround;  // if true, negates left volume to put sound in back
			bool echo;      // false = channel doesn't have any echo
		};
		chan_config_t& chan_config( int i ) { return chans [i + EXTRA_CHANS].cfg; }

		// Apply any changes made to config() and chan_config()
		virtual void apply_config();
		void clear();
		const char * set_sample_rate( long samples_per_sec, int msec = BLIP_DEFAULT_LENGTH);

		/* Sets the number of channels available and optionally their types
		(type information used by Effects_Buffer) */

		const char * set_channel_count( int, int const* );
		void clock_rate( long );

		// Gets indexed channel, from 0 to channel count - 1
		channel_t channel( int i) { return chans[i + EXTRA_CHANS].channel; }
		void end_frame( int32_t );
		long read_samples( int16_t*, long );
		long samples_avail() const { return ((bufs_buffer [0].offset_ >> BLIP_BUFFER_ACCURACY) - mixer_samples_read) * 2; }
		void mixer_read_pairs( int16_t* out, int count );
		long sample_rate_;
		int length_;
		int channel_count_;
		int const* channel_types_;
	protected:
		void channels_changed() { channels_changed_count_++; }
	private:
		config_t config_;
		long clock_rate_;

		int echo_size;

		struct chan_t
		{
			int vol [STEREO];
			chan_config_t cfg;
			channel_t channel;
		};
		blargg_vector<chan_t> chans;

		struct buf_t : Blip_Buffer
		{
		int vol [STEREO];
		bool echo;

		void* operator new ( size_t, void* p ) { return p; }
		void operator delete ( void* ) { }

		~buf_t() { }
		};
		buf_t* bufs_buffer;
		int bufs_size;
		int bufs_max; // bufs_size <= bufs_max, to limit memory usage
		int mixer_samples_read;

		struct {
			long delay [STEREO];
			int treble;
			int feedback;
			int low_pass [STEREO];
		} s;

		blargg_vector<int> echo;
		int echo_pos;

		bool no_effects;
		bool no_echo;

		void mix_effects( int16_t* out, int pair_count );
		const char * new_bufs( int size );
		//from Multi_Buffer
		unsigned channels_changed_count_;
		int samples_per_frame_;
};

/* Simpler interface and lower memory usage */
class Simple_Effects_Buffer : public Effects_Buffer
{
	public:
	Simple_Effects_Buffer();
	struct config_t
	{
		bool enabled;   // false = disable all effects
		float echo;     // 0.0 = none, 1.0 = lots
		float stereo;   // 0.0 = channels in center, 1.0 = channels on left/right
		bool surround;  // true = put some channels in back
	};
	config_t& config() { return config_; }

	void apply_config();	// Apply any changes made to config()
	private:
	config_t config_;
	void chan_config();	// hide
};

#endif
