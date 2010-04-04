// Multi-channel sound buffer interface, and basic mono and stereo buffers

// Blip_Buffer 0.4.1
#ifndef MULTI_BUFFER_H
#define MULTI_BUFFER_H

#include "blargg_common.h"
#include "Blip_Buffer.h"

// Interface to one or more Blip_Buffers mapped to one or more channels
// consisting of left, center, and right buffers.
class Multi_Buffer {
public:
	Multi_Buffer( int samples_per_frame );
	virtual ~Multi_Buffer() { }

	// Sets the number of channels available and optionally their types
	// (type information used by Effects_Buffer)
	enum { type_index_mask = 0xFF };
	enum { wave_type = 0x100, noise_type = 0x200, mixed_type = wave_type | noise_type };
	virtual blargg_err_t set_channel_count( int, int const* types = 0 );
	int channel_count() const { return channel_count_; }

	// Gets indexed channel, from 0 to channel count - 1
	struct channel_t {
		Blip_Buffer* center;
		Blip_Buffer* left;
		Blip_Buffer* right;
	};
	virtual channel_t channel( int index ) BLARGG_PURE( ; )

	// See Blip_Buffer.h
	virtual blargg_err_t set_sample_rate( long rate, int msec = blip_default_length ) BLARGG_PURE( ; )
	virtual void clock_rate( long ) BLARGG_PURE( { } )
	virtual void bass_freq( int ) BLARGG_PURE( { } )
	virtual void clear() BLARGG_PURE( { } )
	long sample_rate() const;

	// Length of buffer, in milliseconds
	int length() const;

	// See Blip_Buffer.h
	virtual void end_frame( blip_time_t ) BLARGG_PURE( { } )

	// Number of samples per output frame (1 = mono, 2 = stereo)
	int samples_per_frame() const;

	// Count of changes to channel configuration. Incremented whenever
	// a change is made to any of the Blip_Buffers for any channel.
	unsigned channels_changed_count() { return channels_changed_count_; }

	// See Blip_Buffer.h
	virtual long read_samples( blip_sample_t*, long ) BLARGG_PURE( { return 0; } )
	virtual long samples_avail() const BLARGG_PURE( { return 0; } )

public:
	BLARGG_DISABLE_NOTHROW
	void disable_immediate_removal() { immediate_removal_ = false; }
protected:
	bool immediate_removal() const { return immediate_removal_; }
	int const* channel_types() const { return channel_types_; }
	void channels_changed() { channels_changed_count_++; }
private:
	// noncopyable
	Multi_Buffer( const Multi_Buffer& );
	Multi_Buffer& operator = ( const Multi_Buffer& );

	unsigned channels_changed_count_;
	long sample_rate_;
	int length_;
	int channel_count_;
	int const samples_per_frame_;
	int const* channel_types_;
	bool immediate_removal_;
};

// Uses a single buffer and outputs mono samples.
class Mono_Buffer : public Multi_Buffer {
	Blip_Buffer buf;
	channel_t chan;
public:
	// Buffer used for all channels
	Blip_Buffer* center() { return &buf; }

public:
	Mono_Buffer();
	~Mono_Buffer();
	blargg_err_t set_sample_rate( long rate, int msec = blip_default_length );
	void clock_rate( long rate ) { buf.clock_rate( rate ); }
	void bass_freq( int freq ) { buf.bass_freq( freq ); }
	void clear() { buf.clear(); }
	long samples_avail() const { return buf.samples_avail(); }
	long read_samples( blip_sample_t* p, long s ) { return buf.read_samples( p, s ); }
	channel_t channel( int ) { return chan; }
	void end_frame( blip_time_t t ) { buf.end_frame( t ); }
};

	class Tracked_Blip_Buffer : public Blip_Buffer {
	public:
		// Non-zero if buffer still has non-silent samples in it. Requires that you call
		// set_modified() appropriately.
		blip_ulong non_silent() const;

		// remove_samples( samples_avail() )
		void remove_all_samples();

	public:
		BLARGG_DISABLE_NOTHROW

		long read_samples( blip_sample_t*, long );
		void remove_silence( long );
		void remove_samples( long );
		Tracked_Blip_Buffer();
		void clear();
		void end_frame( blip_time_t );
	private:
		blip_long last_non_silence;
		void remove_( long );
	};

	class Stereo_Mixer {
	public:
		Tracked_Blip_Buffer* bufs [3];
		blargg_long samples_read;

		Stereo_Mixer() : samples_read( 0 ) { }
		void read_pairs( blip_sample_t* out, int count );
	private:
		void mix_mono  ( blip_sample_t* out, int pair_count );
		void mix_stereo( blip_sample_t* out, int pair_count );
	};

// Uses three buffers (one for center) and outputs stereo sample pairs.
class Stereo_Buffer : public Multi_Buffer {
public:

	// Buffers used for all channels
	Blip_Buffer* center()   { return &bufs [2]; }
	Blip_Buffer* left()     { return &bufs [0]; }
	Blip_Buffer* right()    { return &bufs [1]; }

public:
	Stereo_Buffer();
	~Stereo_Buffer();
	blargg_err_t set_sample_rate( long, int msec = blip_default_length );
	void clock_rate( long );
	void bass_freq( int );
	void clear();
	channel_t channel( int ) { return chan; }
	void end_frame( blip_time_t );

	long samples_avail() const { return (bufs [0].samples_avail() - mixer.samples_read) * 2; }
	long read_samples( blip_sample_t*, long );

private:
	enum { bufs_size = 3 };
	typedef Tracked_Blip_Buffer buf_t;
	buf_t bufs [bufs_size];
	Stereo_Mixer mixer;
	channel_t chan;
	long samples_avail_;
};

// Silent_Buffer generates no samples, useful where no sound is wanted
class Silent_Buffer : public Multi_Buffer {
	channel_t chan;
public:
	Silent_Buffer();
	blargg_err_t set_sample_rate( long rate, int msec = blip_default_length );
	void clock_rate( long ) { }
	void bass_freq( int ) { }
	void clear() { }
	channel_t channel( int ) { return chan; }
	void end_frame( blip_time_t ) { }
	long samples_avail() const { return 0; }
	long read_samples( blip_sample_t*, long ) { return 0; }
};


inline blargg_err_t Multi_Buffer::set_sample_rate( long rate, int msec )
{
	sample_rate_ = rate;
	length_ = msec;
	return 0;
}

inline blargg_err_t Silent_Buffer::set_sample_rate( long rate, int msec )
{
	return Multi_Buffer::set_sample_rate( rate, msec );
}

inline int Multi_Buffer::samples_per_frame() const { return samples_per_frame_; }

inline long Multi_Buffer::sample_rate() const { return sample_rate_; }

inline int Multi_Buffer::length() const { return length_; }

inline blargg_err_t Multi_Buffer::set_channel_count( int n, int const* types )
{
	channel_count_ = n;
	channel_types_ = types;
	return 0;
}

#endif
