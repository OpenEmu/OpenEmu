// Gb_Snd_Emu 0.2.0. http://www.slack.net/~ant/

#include "Gb_Apu.h"

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

#include "blargg_source.h"

bool const cgb_02 = false; // enables bug in early CGB units that causes problems in some games
bool const cgb_05 = false; // enables CGB-05 zombie behavior

int const trigger_mask   = 0x80;
int const length_enabled = 0x40;

void Gb_Osc::reset()
{
	output   = 0;
	last_amp = 0;
	delay    = 0;
	phase    = 0;
	enabled  = false;
}

inline void Gb_Osc::update_amp( blip_time_t time, int new_amp )
{
	output->set_modified();
	int delta = new_amp - last_amp;
	if ( delta )
	{
		last_amp = new_amp;
		med_synth->offset( time, delta, output );
	}
}

// Units

void Gb_Osc::clock_length()
{
	if ( (regs [4] & length_enabled) && length_ctr )
	{
		if ( --length_ctr <= 0 )
			enabled = false;
	}
}

inline int Gb_Env::reload_env_timer()
{
	int raw = regs [2] & 7;
	env_delay = (raw ? raw : 8);
	return raw;
}

void Gb_Env::clock_envelope()
{
	if ( env_enabled && --env_delay <= 0 && reload_env_timer() )
	{
		int v = volume + (regs [2] & 0x08 ? +1 : -1);
		if ( 0 <= v && v <= 15 )
			volume = v;
		else
			env_enabled = false;
	}
}

inline void Gb_Sweep_Square::reload_sweep_timer()
{
	sweep_delay = (regs [0] & period_mask) >> 4;
	if ( !sweep_delay )
		sweep_delay = 8;
}

void Gb_Sweep_Square::calc_sweep( bool update )
{
	int const shift = regs [0] & shift_mask;
	int const delta = sweep_freq >> shift;
	sweep_neg = (regs [0] & 0x08) != 0;
	int const freq = sweep_freq + (sweep_neg ? -delta : delta);

	if ( freq > 0x7FF )
	{
		enabled = false;
	}
	else if ( shift && update )
	{
		sweep_freq = freq;

		regs [3] = freq & 0xFF;
		regs [4] = (regs [4] & ~0x07) | (freq >> 8 & 0x07);
	}
}

void Gb_Sweep_Square::clock_sweep()
{
	if ( --sweep_delay <= 0 )
	{
		reload_sweep_timer();
		if ( sweep_enabled && (regs [0] & period_mask) )
		{
			calc_sweep( true  );
			calc_sweep( false );
		}
	}
}

int Gb_Wave::access( unsigned addr ) const
{
	if ( enabled )
	{
		addr = phase & (bank_size - 1);
		if ( mode == Gb_Apu::mode_dmg )
		{
			addr++;
			if ( delay > clk_mul )
				return -1; // can only access within narrow time window while playing
		}
		addr >>= 1;
	}
	return addr & 0x0F;
}

// write_register

int Gb_Osc::write_trig( int frame_phase, int max_len, int old_data )
{
	int data = regs [4];

	if ( (frame_phase & 1) && !(old_data & length_enabled) && length_ctr )
	{
		if ( (data & length_enabled) || cgb_02 )
			length_ctr--;
	}

	if ( data & trigger_mask )
	{
		enabled = true;
		if ( !length_ctr )
		{
			length_ctr = max_len;
			if ( (frame_phase & 1) && (data & length_enabled) )
				length_ctr--;
		}
	}

	if ( !length_ctr )
		enabled = false;

	return data & trigger_mask;
}

inline void Gb_Env::zombie_volume( int old, int data )
{
	int v = volume;
	if ( mode == Gb_Apu::mode_agb || cgb_05 )
	{
		// CGB-05 behavior, very close to AGB behavior as well
		if ( (old ^ data) & 8 )
		{
			if ( !(old & 8) )
			{
				v++;
				if ( old & 7 )
					v++;
			}

			v = 16 - v;
		}
		else if ( (old & 0x0F) == 8 )
		{
			v++;
		}
	}
	else
	{
		// CGB-04&02 behavior, very close to MGB behavior as well
		if ( !(old & 7) && env_enabled )
			v++;
		else if ( !(old & 8) )
			v += 2;

		if ( (old ^ data) & 8 )
			v = 16 - v;
	}
	volume = v & 0x0F;
}

bool Gb_Env::write_register( int frame_phase, int reg, int old, int data )
{
	int const max_len = 64;

	switch ( reg )
	{
	case 1:
		length_ctr = max_len - (data & (max_len - 1));
		break;

	case 2:
		if ( !dac_enabled() )
			enabled = false;

		zombie_volume( old, data );

		if ( (data & 7) && env_delay == 8 )
		{
			env_delay = 1;
			clock_envelope(); // TODO: really happens at next length clock
		}
		break;

	case 4:
		if ( write_trig( frame_phase, max_len, old ) )
		{
			volume = regs [2] >> 4;
			reload_env_timer();
			env_enabled = true;
			if ( frame_phase == 7 )
				env_delay++;
			if ( !dac_enabled() )
				enabled = false;
			return true;
		}
	}
	return false;
}

bool Gb_Square::write_register( int frame_phase, int reg, int old_data, int data )
{
	bool result = Gb_Env::write_register( frame_phase, reg, old_data, data );
	if ( result )
		delay = (delay & (4 * clk_mul - 1)) + period();
	return result;
}

inline void Gb_Noise::write_register( int frame_phase, int reg, int old_data, int data )
{
	if ( Gb_Env::write_register( frame_phase, reg, old_data, data ) )
	{
		phase = 0x7FFF;
		delay += 8 * clk_mul;
	}
}

inline void Gb_Sweep_Square::write_register( int frame_phase, int reg, int old_data, int data )
{
	if ( reg == 0 && sweep_enabled && sweep_neg && !(data & 0x08) )
		enabled = false; // sweep negate disabled after used

	if ( Gb_Square::write_register( frame_phase, reg, old_data, data ) )
	{
		sweep_freq = frequency();
		sweep_neg = false;
		reload_sweep_timer();
		sweep_enabled = (regs [0] & (period_mask | shift_mask)) != 0;
		if ( regs [0] & shift_mask )
			calc_sweep( false );
	}
}

void Gb_Wave::corrupt_wave()
{
	int pos = ((phase + 1) & (bank_size - 1)) >> 1;
	if ( pos < 4 )
		wave_ram [0] = wave_ram [pos];
	else
		for ( int i = 4; --i >= 0; )
			wave_ram [i] = wave_ram [(pos & ~3) + i];
}

inline void Gb_Wave::write_register( int frame_phase, int reg, int old_data, int data )
{
	int const max_len = 256;

	switch ( reg )
	{
	case 0:
		if ( !dac_enabled() )
			enabled = false;
		break;

	case 1:
		length_ctr = max_len - data;
		break;

	case 4:
		bool was_enabled = enabled;
		if ( write_trig( frame_phase, max_len, old_data ) )
		{
			if ( !dac_enabled() )
				enabled = false;
			else if ( mode == Gb_Apu::mode_dmg && was_enabled &&
					(unsigned) (delay - 2 * clk_mul) < 2 * clk_mul )
				corrupt_wave();

			phase = 0;
			delay    = period() + 6 * clk_mul;
		}
	}
}

void Gb_Apu::write_osc( int index, int reg, int old_data, int data )
{
	reg -= index * 5;
	switch ( index )
	{
	case 0: square1.write_register( frame_phase, reg, old_data, data ); break;
	case 1: square2.write_register( frame_phase, reg, old_data, data ); break;
	case 2: wave   .write_register( frame_phase, reg, old_data, data ); break;
	case 3: noise  .write_register( frame_phase, reg, old_data, data ); break;
	}
}

// Synthesis

void Gb_Square::run( blip_time_t time, blip_time_t end_time )
{
	// Calc duty and phase
	static byte const duty_offsets [4] = { 1, 1, 3, 7 };
	static byte const duties       [4] = { 1, 2, 4, 6 };
	int const duty_code = regs [1] >> 6;
	int duty_offset = duty_offsets [duty_code];
	int duty = duties [duty_code];
	if ( mode == Gb_Apu::mode_agb )
	{
		// AGB uses inverted duty
		duty_offset -= duty;
		duty = 8 - duty;
	}
	int ph = (this->phase + duty_offset) & 7;

	// Determine what will be generated
	int vol = 0;
	Blip_Buffer* const out = this->output;
	if ( out )
	{
		int amp = dac_off_amp;
		if ( dac_enabled() )
		{
			if ( enabled )
				vol = this->volume;

			amp = -dac_bias;
			if ( mode == Gb_Apu::mode_agb )
				amp = -(vol >> 1);

			// Play inaudible frequencies as constant amplitude
			if ( frequency() >= 0x7FA && delay < 32 * clk_mul )
			{
				amp += (vol * duty) >> 3;
				vol = 0;
			}

			if ( ph < duty )
			{
				amp += vol;
				vol = -vol;
			}
		}
		update_amp( time, amp );
	}

	// Generate wave
	time += delay;
	if ( time < end_time )
	{
		int const per = this->period();
		if ( !vol )
		{
			// Maintain phase when not playing
			int count = (end_time - time + per - 1) / per;
			ph += count; // will be masked below
			time += (blip_time_t) count * per;
		}
		else
		{
			// Output amplitude transitions
			int delta = vol;
			do
			{
				ph = (ph + 1) & 7;
				if ( ph == 0 || ph == duty )
				{
					good_synth->offset_inline( time, delta, out );
					delta = -delta;
				}
				time += per;
			}
			while ( time < end_time );

			if ( delta != vol )
				last_amp -= delta;
		}
		this->phase = (ph - duty_offset) & 7;
	}
	delay = time - end_time;
}

// Quickly runs LFSR for a large number of clocks. For use when noise is generating
// no sound.
static unsigned run_lfsr( unsigned s, unsigned mask, int count )
{
	bool const optimized = true; // set to false to use only unoptimized loop in middle

	// optimization used in several places:
	// ((s & (1 << b)) << n) ^ ((s & (1 << b)) << (n + 1)) = (s & (1 << b)) * (3 << n)

	if ( mask == 0x4000 && optimized )
	{
		if ( count >= 32767 )
			count %= 32767;

		// Convert from Fibonacci to Galois configuration,
		// shifted left 1 bit
		s ^= (s & 1) * 0x8000;

		// Each iteration is equivalent to clocking LFSR 255 times
		while ( (count -= 255) > 0 )
			s ^= ((s & 0xE) << 12) ^ ((s & 0xE) << 11) ^ (s >> 3);
		count += 255;

		// Each iteration is equivalent to clocking LFSR 15 times
		// (interesting similarity to single clocking below)
		while ( (count -= 15) > 0 )
			s ^= ((s & 2) * (3 << 13)) ^ (s >> 1);
		count += 15;

		// Remaining singles
		while ( --count >= 0 )
			s = ((s & 2) * (3 << 13)) ^ (s >> 1);

		// Convert back to Fibonacci configuration
		s &= 0x7FFF;
	}
	else if ( count < 8 || !optimized )
	{
		// won't fully replace upper 8 bits, so have to do the unoptimized way
		while ( --count >= 0 )
			s = (s >> 1 | mask) ^ (mask & -((s - 1) & 2));
	}
	else
	{
		if ( count > 127 )
		{
			count %= 127;
			if ( !count )
				count = 127; // must run at least once
		}

		// Need to keep one extra bit of history
		s = s << 1 & 0xFF;

		// Convert from Fibonacci to Galois configuration,
		// shifted left 2 bits
		s ^= (s & 2) * 0x80;

		// Each iteration is equivalent to clocking LFSR 7 times
		// (interesting similarity to single clocking below)
		while ( (count -= 7) > 0 )
			s ^= ((s & 4) * (3 << 5)) ^ (s >> 1);
		count += 7;

		// Remaining singles
		while ( --count >= 0 )
			s = ((s & 4) * (3 << 5)) ^ (s >> 1);

		// Convert back to Fibonacci configuration and
		// repeat last 8 bits above significant 7
		s = (s << 7 & 0x7F80) | (s >> 1 & 0x7F);
	}

	return s;
}

void Gb_Noise::run( blip_time_t time, blip_time_t end_time )
{
	// Determine what will be generated
	int vol = 0;
	Blip_Buffer* const out = this->output;
	if ( out )
	{
		int amp = dac_off_amp;
		if ( dac_enabled() )
		{
			if ( enabled )
				vol = this->volume;

			amp = -dac_bias;
			if ( mode == Gb_Apu::mode_agb )
				amp = -(vol >> 1);

			if ( !(phase & 1) )
			{
				amp += vol;
				vol = -vol;
			}
		}

		// AGB negates final output
		if ( mode == Gb_Apu::mode_agb )
		{
			vol = -vol;
			amp    = -amp;
		}

		update_amp( time, amp );
	}

	// Run timer and calculate time of next LFSR clock
	static byte const period1s [8] = { 1, 2, 4, 6, 8, 10, 12, 14 };
	int const period1 = period1s [regs [3] & 7] * clk_mul;
	{
		int extra = (end_time - time) - delay;
		int const per2 = this->period2();
		time += delay + ((divider ^ (per2 >> 1)) & (per2 - 1)) * period1;

		int count = (extra < 0 ? 0 : (extra + period1 - 1) / period1);
		divider = (divider - count) & period2_mask;
		delay = count * period1 - extra;
	}

	// Generate wave
	if ( time < end_time )
	{
		unsigned const mask = this->lfsr_mask();
		unsigned bits = this->phase;

		int per = period2( period1 * 8 );
		if ( period2_index() >= 0xE )
		{
			time = end_time;
		}
		else if ( !vol )
		{
			// Maintain phase when not playing
			int count = (end_time - time + per - 1) / per;
			time += (blip_time_t) count * per;
			bits = run_lfsr( bits, ~mask, count );
		}
		else
		{
			// Output amplitude transitions
			int delta = -vol;
			do
			{
				unsigned changed = bits + 1;
				bits = bits >> 1 & mask;
				if ( changed & 2 )
				{
					bits |= ~mask;
					delta = -delta;
					med_synth->offset_inline( time, delta, out );
				}
				time += per;
			}
			while ( time < end_time );

			if ( delta == vol )
				last_amp += delta;
		}
		this->phase = bits;
	}
}

void Gb_Wave::run( blip_time_t time, blip_time_t end_time )
{
	// Calc volume
	static byte const volumes [8] = { 0, 4, 2, 1, 3, 3, 3, 3 };
	int const volume_shift = 2;
	int const volume_idx = regs [2] >> 5 & (agb_mask | 3); // 2 bits on DMG/CGB, 3 on AGB
	int const volume_mul = volumes [volume_idx];

	// Determine what will be generated
	int playing = false;
	Blip_Buffer* const out = this->output;
	if ( out )
	{
		int amp = dac_off_amp;
		if ( dac_enabled() )
		{
			// Play inaudible frequencies as constant amplitude
			amp = 8 << 4; // really depends on average of all samples in wave

			// if delay is larger, constant amplitude won't start yet
			if ( frequency() <= 0x7FB || delay > 15 * clk_mul )
			{
				if ( volume_mul )
					playing = (int) enabled;

				amp = (sample_buf << (phase << 2 & 4) & 0xF0) * playing;
			}

			amp = ((amp * volume_mul) >> (volume_shift + 4)) - dac_bias;
		}
		update_amp( time, amp );
	}

	// Generate wave
	time += delay;
	if ( time < end_time )
	{
		byte const* wave = this->wave_ram;

		// wave size and bank
		int const size20_mask = 0x20;
		int const flags = regs [0] & agb_mask;
		int const wave_mask = (flags & size20_mask) | 0x1F;
		int swap_banks = 0;
		if ( flags & bank40_mask )
		{
			swap_banks = flags & size20_mask;
			wave += bank_size/2 - (swap_banks >> 1);
		}

		int ph = this->phase ^ swap_banks;
		ph = (ph + 1) & wave_mask; // pre-advance

		int const per = this->period();
		if ( !playing )
		{
			// Maintain phase when not playing
			int count = (end_time - time + per - 1) / per;
			ph += count; // will be masked below
			time += (blip_time_t) count * per;
		}
		else
		{
			// Output amplitude transitions
			int lamp = this->last_amp + dac_bias;
			do
			{
				// Extract nybble
				int nybble = wave [ph >> 1] << (ph << 2 & 4) & 0xF0;
				ph = (ph + 1) & wave_mask;

				// Scale by volume
				int amp = (nybble * volume_mul) >> (volume_shift + 4);

				int delta = amp - lamp;
				if ( delta )
				{
					lamp = amp;
					med_synth->offset_inline( time, delta, out );
				}
				time += per;
			}
			while ( time < end_time );
			this->last_amp = lamp - dac_bias;
		}
		ph = (ph - 1) & wave_mask; // undo pre-advance and mask position

		// Keep track of last byte read
		if ( enabled )
			sample_buf = wave [ph >> 1];

		this->phase = ph ^ swap_banks; // undo swapped banks
	}
	delay = time - end_time;
}
