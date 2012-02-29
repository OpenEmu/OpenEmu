// Game_Music_Emu $vers. http://www.slack.net/~ant/

#include "Effects_Buffer.h"

#include <string.h>

/* Copyright (C) 2006-2007 Shay Green. This module is free software; you
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

int const fixed_shift = 12;
#define TO_FIXED( f )   fixed_t ((f) * ((fixed_t) 1 << fixed_shift))
#define FROM_FIXED( f ) ((f) >> fixed_shift)

int const max_read = 2560; // determines minimum delay

Effects_Buffer::Effects_Buffer( int max_bufs, long echo_size_ ) : Multi_Buffer( stereo )
{
	echo_size   = max( max_read * (long) stereo, echo_size_ & ~1 );
	clock_rate_ = 0;
	bass_freq_  = 90;
	bufs        = 0;
	bufs_size   = 0;
	bufs_max    = max( max_bufs, (int) extra_chans );
	no_echo     = true;
	no_effects  = true;

	// defaults
	config_.enabled   = false;
	config_.delay [0] = 120;
	config_.delay [1] = 122;
	config_.feedback  = 0.2f;
	config_.treble    = 0.4f;

	static float const sep = 0.8f;
	config_.side_chans [0].pan = -sep;
	config_.side_chans [1].pan = +sep;
	config_.side_chans [0].vol = 1.0f;
	config_.side_chans [1].vol = 1.0f;

	memset( &s, 0, sizeof s );
	clear();
}

Effects_Buffer::~Effects_Buffer()
{
	delete_bufs();
}

// avoid using new []
blargg_err_t Effects_Buffer::new_bufs( int size )
{
	bufs = (buf_t*) malloc( size * sizeof *bufs );
	CHECK_ALLOC( bufs );
	for ( int i = 0; i < size; i++ )
		new (bufs + i) buf_t;
	bufs_size = size;
	return 0;
}

void Effects_Buffer::delete_bufs()
{
	if ( bufs )
	{
		for ( int i = bufs_size; --i >= 0; )
			bufs [i].~buf_t();
		free( bufs );
		bufs = 0;
	}
	bufs_size = 0;
}

blargg_err_t Effects_Buffer::set_sample_rate( long rate, int msec )
{
	// extra to allow farther past-the-end pointers
	mixer.samples_read = 0;
	RETURN_ERR( echo.resize( echo_size + stereo ) );
	return Multi_Buffer::set_sample_rate( rate, msec );
}

void Effects_Buffer::clock_rate( long rate )
{
	clock_rate_ = rate;
	for ( int i = bufs_size; --i >= 0; )
		bufs [i].clock_rate( clock_rate_ );
}

void Effects_Buffer::bass_freq( int freq )
{
	bass_freq_ = freq;
	for ( int i = bufs_size; --i >= 0; )
		bufs [i].bass_freq( bass_freq_ );
}

blargg_err_t Effects_Buffer::set_channel_count( int count, int const* types )
{
	RETURN_ERR( Multi_Buffer::set_channel_count( count, types ) );

	delete_bufs();

	mixer.samples_read = 0;

	RETURN_ERR( chans.resize( count + extra_chans ) );

	RETURN_ERR( new_bufs( min( bufs_max, count + extra_chans ) ) );

	for ( int i = bufs_size; --i >= 0; )
		RETURN_ERR( bufs [i].set_sample_rate( sample_rate(), length() ) );

	for ( int i = chans.size(); --i >= 0; )
	{
		chan_t& ch = chans [i];
		ch.cfg.vol      = 1.0f;
		ch.cfg.pan      = 0.0f;
		ch.cfg.surround = false;
		ch.cfg.echo     = false;
	}
	// side channels with echo
	chans [2].cfg.echo = true;
	chans [3].cfg.echo = true;

	clock_rate( clock_rate_ );
	bass_freq( bass_freq_ );
	apply_config();
	clear();

	return 0;
}

void Effects_Buffer::clear_echo()
{
	if ( echo.size() )
		memset( echo.begin(), 0, echo.size() * sizeof echo [0] );
}

void Effects_Buffer::clear()
{
	echo_pos       = 0;
	s.low_pass [0] = 0;
	s.low_pass [1] = 0;
	mixer.samples_read = 0;

	for ( int i = bufs_size; --i >= 0; )
		bufs [i].clear();
	clear_echo();
}

Effects_Buffer::channel_t Effects_Buffer::channel( int i )
{
	i += extra_chans;
	require( extra_chans <= i && i < (int) chans.size() );
	return chans [i].channel;
}


// Configuration

// 3 wave positions with/without surround, 2 multi (one with same config as wave)
int const simple_bufs = 3 * 2 + 2 - 1;

Simple_Effects_Buffer::Simple_Effects_Buffer() :
	Effects_Buffer( extra_chans + simple_bufs, 18 * 1024L )
{
	config_.echo     = 0.20f;
	config_.stereo   = 0.20f;
	config_.surround = true;
	config_.enabled  = false;
}

void Simple_Effects_Buffer::apply_config()
{
	Effects_Buffer::config_t& c = Effects_Buffer::config();

	c.enabled = config_.enabled;
	if ( c.enabled )
	{
		c.delay [0] = 120;
		c.delay [1] = 122;
		c.feedback  = config_.echo * 0.7f;
		c.treble    = 0.6f - 0.3f * config_.echo;

		float sep = config_.stereo + 0.80f;
		if ( sep > 1.0f )
			sep = 1.0f;

		c.side_chans [0].pan = -sep;
		c.side_chans [1].pan = +sep;

		for ( int i = channel_count(); --i >= 0; )
		{
			chan_config_t& ch = Effects_Buffer::chan_config( i );

			ch.pan      = 0.0f;
			ch.surround = config_.surround;
			ch.echo     = false;

			int const type = (channel_types() ? channel_types() [i] : 0);
			if ( !(type & noise_type) )
			{
				int index = (type & type_index_mask) % 6 - 3;
				if ( index < 0 )
				{
					index += 3;
					ch.surround = false;
					ch.echo     = true;
				}
				if ( index >= 1 )
				{
					ch.pan = config_.stereo;
					if ( index == 1 )
						ch.pan = -ch.pan;
				}
			}
			else if ( type & 1 )
			{
				ch.surround = false;
			}
		}
	}

	Effects_Buffer::apply_config();
}

int Effects_Buffer::min_delay() const
{
	require( sample_rate() );
	return max_read * 1000L / sample_rate();
}

int Effects_Buffer::max_delay() const
{
	require( sample_rate() );
	return (echo_size / stereo - max_read) * 1000L / sample_rate();
}

void Effects_Buffer::apply_config()
{
	int i;

	if ( !bufs_size )
		return;

	s.treble = TO_FIXED( config_.treble );

	bool echo_dirty = false;

	fixed_t old_feedback = s.feedback;
	s.feedback = TO_FIXED( config_.feedback );
	if ( !old_feedback && s.feedback )
		echo_dirty = true;

	// delays
	for ( i = stereo; --i >= 0; )
	{
		long delay = config_.delay [i] * sample_rate() / 1000 * stereo;
		delay = max( delay, long (max_read * stereo) );
		delay = min( delay, long (echo_size - max_read * stereo) );
		if ( s.delay [i] != delay )
		{
			s.delay [i] = delay;
			echo_dirty = true;
		}
	}

	// side channels
	for ( i = 2; --i >= 0; )
	{
		chans [i+2].cfg.vol = chans [i].cfg.vol = config_.side_chans [i].vol * 0.5f;
		chans [i+2].cfg.pan = chans [i].cfg.pan = config_.side_chans [i].pan;
	}

	// convert volumes
	for ( i = chans.size(); --i >= 0; )
	{
		chan_t& ch = chans [i];
		ch.vol [0] = TO_FIXED( ch.cfg.vol - ch.cfg.vol * ch.cfg.pan );
		ch.vol [1] = TO_FIXED( ch.cfg.vol + ch.cfg.vol * ch.cfg.pan );
		if ( ch.cfg.surround )
			ch.vol [0] = -ch.vol [0];
	}

	assign_buffers();

	// set side channels
	for ( i = chans.size(); --i >= 0; )
	{
		chan_t& ch = chans [i];
		ch.channel.left  = chans [ch.cfg.echo*2  ].channel.center;
		ch.channel.right = chans [ch.cfg.echo*2+1].channel.center;
	}

	bool old_echo = !no_echo && !no_effects;

	// determine whether effects and echo are needed at all
	no_effects = true;
	no_echo    = true;
	for ( i = chans.size(); --i >= extra_chans; )
	{
		chan_t& ch = chans [i];
		if ( ch.cfg.echo && s.feedback )
			no_echo = false;

		if ( ch.vol [0] != TO_FIXED( 1 ) || ch.vol [1] != TO_FIXED( 1 ) )
			no_effects = false;
	}
	if ( !no_echo )
		no_effects = false;

	if (    chans [0].vol [0] != TO_FIXED( 1 ) ||
			chans [0].vol [1] != TO_FIXED( 0 ) ||
			chans [1].vol [0] != TO_FIXED( 0 ) ||
			chans [1].vol [1] != TO_FIXED( 1 ) )
		no_effects = false;

	if ( !config_.enabled )
		no_effects = true;

	if ( no_effects )
	{
		for ( i = chans.size(); --i >= 0; )
		{
			chan_t& ch = chans [i];
			ch.channel.center = &bufs [2];
			ch.channel.left   = &bufs [0];
			ch.channel.right  = &bufs [1];
		}
	}

	mixer.bufs [0] = &bufs [0];
	mixer.bufs [1] = &bufs [1];
	mixer.bufs [2] = &bufs [2];

	if ( echo_dirty || (!old_echo && (!no_echo && !no_effects)) )
		clear_echo();

	channels_changed();
}

void Effects_Buffer::assign_buffers()
{
	// assign channels to buffers
	int buf_count = 0;
	for ( int i = 0; i < (int) chans.size(); i++ )
	{
		// put second two side channels at end to give priority to main channels
		// in case closest matching is necessary
		int x = i;
		if ( i > 1 )
			x += 2;
		if ( x >= (int) chans.size() )
			x -= (chans.size() - 2);
		chan_t& ch = chans [x];

		int b = 0;
		for ( ; b < buf_count; b++ )
		{
			if (    ch.vol [0] == bufs [b].vol [0] &&
					ch.vol [1] == bufs [b].vol [1] &&
					(ch.cfg.echo == bufs [b].echo || !s.feedback) )
				break;
		}

		if ( b >= buf_count )
		{
			if ( buf_count < bufs_max )
			{
				bufs [b].vol [0] = ch.vol [0];
				bufs [b].vol [1] = ch.vol [1];
				bufs [b].echo    = ch.cfg.echo;
				buf_count++;
			}
			else
			{
				// TODO: this is a mess, needs refinement
				dprintf( "Effects_Buffer ran out of buffers; using closest match\n" );
				b = 0;
				fixed_t best_dist = TO_FIXED( 8 );
				for ( int h = buf_count; --h >= 0; )
				{
					#define CALC_LEVELS( vols, sum, diff, surround ) \
					fixed_t sum, diff;\
					bool surround = false;\
					{\
						fixed_t vol_0 = vols [0];\
						if ( vol_0 < 0 ) vol_0 = -vol_0, surround = true;\
						fixed_t vol_1 = vols [1];\
						if ( vol_1 < 0 ) vol_1 = -vol_1, surround = true;\
						sum  = vol_0 + vol_1;\
						diff = vol_0 - vol_1;\
					}
					CALC_LEVELS( ch.vol,       ch_sum,  ch_diff,  ch_surround );
					CALC_LEVELS( bufs [h].vol, buf_sum, buf_diff, buf_surround );

					fixed_t dist = abs( ch_sum - buf_sum ) + abs( ch_diff - buf_diff );

					if ( ch_surround != buf_surround )
						dist += TO_FIXED( 1 ) / 2;

					if ( s.feedback && ch.cfg.echo != bufs [h].echo )
						dist += TO_FIXED( 1 ) / 2;

					if ( best_dist > dist )
					{
						best_dist = dist;
						b = h;
					}
				}
			}
		}

		//dprintf( "ch %d->buf %d\n", x, b );
		ch.channel.center = &bufs [b];
	}
}


// Mixing

void Effects_Buffer::end_frame( blip_time_t time )
{
	for ( int i = bufs_size; --i >= 0; )
		bufs [i].end_frame( time );
}

long Effects_Buffer::read_samples( blip_sample_t* out, long out_size )
{
	out_size = min( out_size, samples_avail() );

	int pair_count = int (out_size >> 1);
	require( pair_count * stereo == out_size ); // must read an even number of samples
	if ( pair_count )
	{
		if ( no_effects )
		{
			mixer.read_pairs( out, pair_count );
		}
		else
		{
			int pairs_remain = pair_count;
			do
			{
				// mix at most max_read pairs at a time
				int count = max_read;
				if ( count > pairs_remain )
					count = pairs_remain;

				if ( no_echo )
				{
					// optimization: clear echo here to keep mix_effects() a leaf function
					echo_pos = 0;
					memset( echo.begin(), 0, count * stereo * sizeof echo [0] );
				}
				mix_effects( out, count );

				blargg_long new_echo_pos = echo_pos + count * stereo;
				if ( new_echo_pos >= echo_size )
					new_echo_pos -= echo_size;
				echo_pos = new_echo_pos;
				assert( echo_pos < echo_size );

				out += count * stereo;
				mixer.samples_read += count;
				pairs_remain -= count;
			}
			while ( pairs_remain );
		}

		if ( samples_avail() <= 0 || immediate_removal() )
		{
			for ( int i = bufs_size; --i >= 0; )
			{
				buf_t& b = bufs [i];
				// TODO: might miss non-silence settling since it checks END of last read
				if ( b.non_silent() )
					b.remove_samples( mixer.samples_read );
				else
					b.remove_silence( mixer.samples_read );
			}
			mixer.samples_read = 0;
		}
	}
	return out_size;
}

void Effects_Buffer::mix_effects( blip_sample_t* out_, int pair_count )
{
	typedef fixed_t stereo_fixed_t [stereo];

	// add channels with echo, do echo, add channels without echo, then convert to 16-bit and output
	int echo_phase = 1;
	do
	{
		// mix any modified buffers
		{
			buf_t* buf = bufs;
			int bufs_remain = bufs_size;
			do
			{
				if ( buf->non_silent() && ( buf->echo == !!echo_phase ) )
				{
					stereo_fixed_t* BLIP_RESTRICT out = (stereo_fixed_t*) &echo [echo_pos];
					int const bass = BLIP_READER_BASS( *buf );
					BLIP_READER_BEGIN( in, *buf );
					BLIP_READER_ADJ_( in, mixer.samples_read );
					fixed_t const vol_0 = buf->vol [0];
					fixed_t const vol_1 = buf->vol [1];

					int count = unsigned (echo_size - echo_pos) / stereo;
					int remain = pair_count;
					if ( count > remain )
						count = remain;
					do
					{
						remain -= count;
						BLIP_READER_ADJ_( in, count );

						out += count;
						int offset = -count;
						do
						{
							fixed_t s = BLIP_READER_READ( in );
							BLIP_READER_NEXT_IDX_( in, bass, offset );

							out [offset] [0] += s * vol_0;
							out [offset] [1] += s * vol_1;
						}
						while ( ++offset );

						out = (stereo_fixed_t*) echo.begin();
						count = remain;
					}
					while ( remain );

					BLIP_READER_END( in, *buf );
				}
				buf++;
			}
			while ( --bufs_remain );
		}

		// add echo
		if ( echo_phase && !no_echo )
		{
			fixed_t const feedback = s.feedback;
			fixed_t const treble   = s.treble;

			int i = 1;
			do
			{
				fixed_t low_pass = s.low_pass [i];

				fixed_t* echo_end = &echo [echo_size + i];
				fixed_t const* BLIP_RESTRICT in_pos = &echo [echo_pos + i];
				blargg_long out_offset = echo_pos + i + s.delay [i];
				if ( out_offset >= echo_size )
					out_offset -= echo_size;
				assert( out_offset < echo_size );
				fixed_t* BLIP_RESTRICT out_pos = &echo [out_offset];

				// break into up to three chunks to avoid having to handle wrap-around
				// in middle of core loop
				int remain = pair_count;
				do
				{
					fixed_t const* pos = in_pos;
					if ( pos < out_pos )
						pos = out_pos;
					int count = blargg_ulong ((char*) echo_end - (char const*) pos) /
							unsigned (stereo * sizeof (fixed_t));
					if ( count > remain )
						count = remain;
					remain -= count;

					in_pos  += count * stereo;
					out_pos += count * stereo;
					int offset = -count;
					do
					{
						low_pass += FROM_FIXED( in_pos [offset * stereo] - low_pass ) * treble;
						out_pos [offset * stereo] = FROM_FIXED( low_pass ) * feedback;
					}
					while ( ++offset );

					if (  in_pos >= echo_end )  in_pos -= echo_size;
					if ( out_pos >= echo_end ) out_pos -= echo_size;
				}
				while ( remain );

				s.low_pass [i] = low_pass;
			}
			while ( --i >= 0 );
		}
	}
	while ( --echo_phase >= 0 );

	// clamp to 16 bits
	{
		stereo_fixed_t const* BLIP_RESTRICT in = (stereo_fixed_t*) &echo [echo_pos];
		typedef blip_sample_t stereo_blip_sample_t [stereo];
		stereo_blip_sample_t* BLIP_RESTRICT out = (stereo_blip_sample_t*) out_;
		int count = unsigned (echo_size - echo_pos) / (unsigned) stereo;
		int remain = pair_count;
		if ( count > remain )
			count = remain;
		do
		{
			remain -= count;
			in  += count;
			out += count;
			int offset = -count;
			do
			{
				fixed_t in_0 = FROM_FIXED( in [offset] [0] );
				fixed_t in_1 = FROM_FIXED( in [offset] [1] );

				BLIP_CLAMP( in_0, in_0 );
				out [offset] [0] = (blip_sample_t) in_0;

				BLIP_CLAMP( in_1, in_1 );
				out [offset] [1] = (blip_sample_t) in_1;
			}
			while ( ++offset );

			in = (stereo_fixed_t*) echo.begin();
			count = remain;
		}
		while ( remain );
	}
}
