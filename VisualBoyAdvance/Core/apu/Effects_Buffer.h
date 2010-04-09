// Multi-channel effects buffer with echo and individual panning for each channel

// Game_Music_Emu $vers
#ifndef EFFECTS_BUFFER_H
#define EFFECTS_BUFFER_H

#include "Multi_Buffer.h"

// See Simple_Effects_Buffer (below) for a simpler interface

class Effects_Buffer : public Multi_Buffer {
public:
	// To reduce memory usage, fewer buffers can be used (with a best-fit
	// approach if there are too few), and maximum echo delay can be reduced
	Effects_Buffer( int max_bufs = 32, long echo_size = 24 * 1024L );

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

	// Limits of delay (msec)
	int min_delay() const;
	int max_delay() const;

	// Per-channel configuration. Two or more channels with matching parameters are
	// optimized to internally use the same buffer.
	struct chan_config_t : pan_vol_t
	{
		// (inherited from pan_vol_t)
		//float vol;        // these only affect center channel
		//float pan;
		bool surround;  // if true, negates left volume to put sound in back
		bool echo;      // false = channel doesn't have any echo
	};
	chan_config_t& chan_config( int i ) { return chans [i + extra_chans].cfg; }

	// Apply any changes made to config() and chan_config()
	virtual void apply_config();

public:
	~Effects_Buffer();
	blargg_err_t set_sample_rate( long samples_per_sec, int msec = blip_default_length );
	blargg_err_t set_channel_count( int, int const* = 0 );
	void clock_rate( long );
	void bass_freq( int );
	void clear();
	channel_t channel( int );
	void end_frame( blip_time_t );
	long read_samples( blip_sample_t*, long );
	long samples_avail() const { return (bufs [0].samples_avail() - mixer.samples_read) * 2; }
	enum { stereo = 2 };
	typedef blargg_long fixed_t;
protected:
	enum { extra_chans = stereo * stereo };
private:
	config_t config_;
	long clock_rate_;
	int bass_freq_;

	blargg_long echo_size;

	struct chan_t
	{
		fixed_t vol [stereo];
		chan_config_t cfg;
		channel_t channel;
	};
	blargg_vector<chan_t> chans;

	struct buf_t : Tracked_Blip_Buffer
	{
		fixed_t vol [stereo];
		bool echo;

		void* operator new ( size_t, void* p ) { return p; }
		void operator delete ( void* ) { }

		~buf_t() { }
	};
	buf_t* bufs;
	int bufs_size;
	int bufs_max; // bufs_size <= bufs_max, to limit memory usage
	Stereo_Mixer mixer;

	struct {
		long delay [stereo];
		fixed_t treble;
		fixed_t feedback;
		fixed_t low_pass [stereo];
	} s;

	blargg_vector<fixed_t> echo;
	blargg_long echo_pos;

	bool no_effects;
	bool no_echo;

	void assign_buffers();
	void clear_echo();
	void mix_effects( blip_sample_t* out, int pair_count );
	blargg_err_t new_bufs( int size );
	void delete_bufs();
};

// Simpler interface and lower memory usage
class Simple_Effects_Buffer : public Effects_Buffer {
public:
	struct config_t
	{
		bool enabled;   // false = disable all effects
		float echo;     // 0.0 = none, 1.0 = lots
		float stereo;   // 0.0 = channels in center, 1.0 = channels on left/right
		bool surround;  // true = put some channels in back
	};
	config_t& config() { return config_; }

	// Apply any changes made to config()
	void apply_config();

public:
	Simple_Effects_Buffer();
private:
	config_t config_;
	void chan_config(); // hide
};

#endif
