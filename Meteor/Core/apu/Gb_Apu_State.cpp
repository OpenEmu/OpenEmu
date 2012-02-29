// Gb_Snd_Emu $vers. http://www.slack.net/~ant/

#include "Gb_Apu.h"

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

#if GB_APU_CUSTOM_STATE
	#define REFLECT( x, y ) (save ?       (io->y) = (x) :         (x) = (io->y)          )
#else
	#define REFLECT( x, y ) (save ? set_val( io->y, x ) : (void) ((x) = get_val( io->y )))

	static blargg_ulong get_val( byte const* p )
	{
		return  p [3] * 0x1000000 + p [2] * 0x10000 + p [1] * 0x100 + p [0];
	}

	static void set_val( byte* p, blargg_ulong n )
	{
		p [0] = (byte) (n      );
		p [1] = (byte) (n >>  8);
		p [2] = (byte) (n >> 16);
		p [3] = (byte) (n >> 24);
	}
#endif

inline const char* Gb_Apu::save_load( gb_apu_state_t* io, bool save )
{
	#if !GB_APU_CUSTOM_STATE
		assert( sizeof (gb_apu_state_t) == 256 );
	#endif

	int format = io->format0;
	REFLECT( format, format );
	if ( format != io->format0 )
		return "Unsupported sound save state format";

	int version = 0;
	REFLECT( version, version );

	// Registers and wave RAM
	assert( regs_size == sizeof io->regs );
	if ( save )
		memcpy( io->regs, regs, sizeof io->regs );
	else
		memcpy( regs, io->regs, sizeof     regs );

	// Frame sequencer
	REFLECT( frame_time,  frame_time  );
	REFLECT( frame_phase, frame_phase );

	REFLECT( square1.sweep_freq,    sweep_freq );
	REFLECT( square1.sweep_delay,   sweep_delay );
	REFLECT( square1.sweep_enabled, sweep_enabled );
	REFLECT( square1.sweep_neg,     sweep_neg );

	REFLECT( noise.divider,         noise_divider );
	REFLECT( wave.sample_buf,       wave_buf );

	return 0;
}

// second function to avoid inline limits of some compilers
inline void Gb_Apu::save_load2( gb_apu_state_t* io, bool save )
{
	for ( int i = osc_count; --i >= 0; )
	{
		Gb_Osc& osc = *oscs [i];
		REFLECT( osc.delay,      delay      [i] );
		REFLECT( osc.length_ctr, length_ctr [i] );
		REFLECT( osc.phase,      phase      [i] );
		REFLECT( osc.enabled,    enabled    [i] );

		if ( i != 2 )
		{
			int j = min( i, 2 );
			Gb_Env& env = STATIC_CAST(Gb_Env&,osc);
			REFLECT( env.env_delay,   env_delay   [j] );
			REFLECT( env.volume,      env_volume  [j] );
			REFLECT( env.env_enabled, env_enabled [j] );
		}
	}
}

void Gb_Apu::save_state( gb_apu_state_t* out )
{
	(void) save_load( out, true );
	save_load2( out, true );

	#if !GB_APU_CUSTOM_STATE
		memset( out->unused, 0, sizeof out->unused );
	#endif
}

blargg_err_t Gb_Apu::load_state( gb_apu_state_t const& in )
{
	RETURN_ERR( save_load( CONST_CAST(gb_apu_state_t*,&in), false ) );
	save_load2( CONST_CAST(gb_apu_state_t*,&in), false );

	apply_stereo();
	synth_volume( 0 );          // suppress output for the moment
	run_until_( last_time );    // get last_amp updated
	apply_volume();             // now use correct volume

	return 0;
}

