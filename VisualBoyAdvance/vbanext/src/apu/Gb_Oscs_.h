/* Gb_Snd_Emu 0.2.0. http://www.slack.net/~ant/ */

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

/* enables bug in early CGB units that causes problems in some games*/
#define CGB_02 0

/* enables CGB-05 zombie behavior*/
#define CGB_05 0

#define TRIGGER_MASK 0x80
#define LENGTH_ENABLED 0x40

#define VOLUME_SHIFT_PLUS_FOUR	6
#define SIZE20_MASK 0x20

void Gb_Osc::reset()
{
        output   = 0;
        last_amp = 0;
        delay    = 0;
        phase    = 0;
        enabled  = false;
}

INLINE void Gb_Osc::update_amp( int32_t time, int new_amp )
{
	int delta = new_amp - last_amp;
        if ( delta )
        {
                last_amp = new_amp;
                med_synth->offset( time, delta, output );
        }
}

void Gb_Osc::clock_length()
{
        if ( (regs [4] & LENGTH_ENABLED) && length_ctr )
        {
                if ( --length_ctr <= 0 )
                        enabled = false;
        }
}

INLINE int Gb_Env::reload_env_timer()
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

#define reload_sweep_timer() \
        sweep_delay = (regs [0] & PERIOD_MASK) >> 4; \
        if ( !sweep_delay ) \
                sweep_delay = 8;

void Gb_Sweep_Square::calc_sweep( bool update )
{
	int shift, delta, freq;

        shift = regs [0] & SHIFT_MASK;
        delta = sweep_freq >> shift;
        sweep_neg = (regs [0] & 0x08) != 0;
        freq = sweep_freq + (sweep_neg ? -delta : delta);

        if ( freq > 0x7FF )
                enabled = false;
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
                if ( sweep_enabled && (regs [0] & PERIOD_MASK) )
                {
                        calc_sweep( true  );
                        calc_sweep( false );
                }
        }
}

int Gb_Wave::access( unsigned addr ) const
{
	addr = phase & BANK_SIZE_MIN_ONE;
	if ( mode == MODE_DMG )
	{
		addr++;
		if ( delay > CLK_MUL )
			return -1; // can only access within narrow time window while playing
	}

	addr >>= 1;
	return addr & 0x0F;
}

// write_register

int Gb_Osc::write_trig( int frame_phase, int max_len, int old_data )
{
        int data = regs [4];

        if ( (frame_phase & 1) && !(old_data & LENGTH_ENABLED) && length_ctr )
        {
                if ( (data & LENGTH_ENABLED))
                        length_ctr--;
        }

        if ( data & TRIGGER_MASK )
        {
                enabled = true;
                if ( !length_ctr )
                {
                        length_ctr = max_len;
                        if ( (frame_phase & 1) && (data & LENGTH_ENABLED) )
                                length_ctr--;
                }
        }

        if ( !length_ctr )
                enabled = false;

        return data & TRIGGER_MASK;
}

INLINE void Gb_Env::zombie_volume( int old, int data )
{
        int v = volume;
        if ( mode == MODE_AGB)
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
                        v++;
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
			if ( !GB_ENV_DAC_ENABLED() )
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
				if ( !GB_ENV_DAC_ENABLED() )
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
                delay = (delay & (CLK_MUL_MUL_4 - 1)) + period();
        return result;
}

INLINE void Gb_Noise::write_register( int frame_phase, int reg, int old_data, int data )
{
        if ( Gb_Env::write_register( frame_phase, reg, old_data, data ) )
        {
                phase = 0x7FFF;
                delay += CLK_MUL_MUL_8;
        }
}

INLINE void Gb_Sweep_Square::write_register( int frame_phase, int reg, int old_data, int data )
{
        if ( reg == 0 && sweep_enabled && sweep_neg && !(data & 0x08) )
                enabled = false; // sweep negate disabled after used

        if ( Gb_Square::write_register( frame_phase, reg, old_data, data ) )
        {
                sweep_freq = GB_OSC_FREQUENCY();
                sweep_neg = false;
                reload_sweep_timer();
                sweep_enabled = (regs [0] & (PERIOD_MASK | SHIFT_MASK)) != 0;
                if ( regs [0] & SHIFT_MASK )
                        calc_sweep( false );
        }
}

void Gb_Wave::corrupt_wave()
{
        int pos = ((phase + 1) & BANK_SIZE_MIN_ONE) >> 1;
        if ( pos < 4 )
                wave_ram [0] = wave_ram [pos];
        else
                for ( int i = 4; --i >= 0; )
                        wave_ram [i] = wave_ram [(pos & ~3) + i];
}

INLINE void Gb_Wave::write_register( int frame_phase, int reg, int old_data, int data )
{
        switch ( reg )
	{
		case 0:
			if ( !GB_WAVE_DAC_ENABLED() )
				enabled = false;
			break;

		case 1:
			length_ctr = 256 - data;
			break;

		case 4:
			bool was_enabled = enabled;
			if ( write_trig( frame_phase, 256, old_data ) )
			{
				if ( !GB_WAVE_DAC_ENABLED() )
					enabled = false;
				else if ( mode == MODE_DMG && was_enabled && (unsigned) (delay - CLK_MUL_MUL_2) < CLK_MUL_MUL_2 )
					corrupt_wave();
				phase = 0;
				delay    = period() + CLK_MUL_MUL_6;
			}
	}
}

void Gb_Apu::write_osc( int index, int reg, int old_data, int data )
{
        reg -= index * 5;
        switch ( index )
	{
		case 0:
			square1.write_register( frame_phase, reg, old_data, data );
			break;
		case 1:
			square2.write_register( frame_phase, reg, old_data, data );
			break;
		case 2:
			wave   .write_register( frame_phase, reg, old_data, data );
			break;
		case 3:
			noise  .write_register( frame_phase, reg, old_data, data );
			break;
	}
}

/* Synthesis*/

void Gb_Square::run( int32_t time, int32_t end_time )
{
        /* Calc duty and phase*/
        static unsigned char const duty_offsets [4] = { 1, 1, 3, 7 };
        static unsigned char const duties       [4] = { 1, 2, 4, 6 };
        int const duty_code = regs [1] >> 6;
        int32_t duty_offset = duty_offsets [duty_code];
        int32_t duty = duties [duty_code];
        if ( mode == MODE_AGB )
        {
                /* AGB uses inverted duty*/
                duty_offset -= duty;
                duty = 8 - duty;
        }
        int ph = (phase + duty_offset) & 7;

        /* Determine what will be generated*/
        int vol = 0;
        Blip_Buffer* const out = output;
        if ( out )
        {
                int amp = dac_off_amp;
                if ( GB_ENV_DAC_ENABLED() )
                {
                        if ( enabled )
                                vol = volume;

                        amp = -DAC_BIAS;
                        if ( mode == MODE_AGB )
                                amp = -(vol >> 1);

                        /* Play inaudible frequencies as constant amplitude*/
                        if ( GB_OSC_FREQUENCY() >= 0x7FA && delay < CLK_MUL_MUL_32 )
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

        /* Generate wave*/
        time += delay;
        if ( time < end_time )
        {
                int const per = period();
                if ( !vol )
                {
                        /* Maintain phase when not playing*/
                        int count = (end_time - time + per - 1) / per;
                        ph += count; /* will be masked below*/
                        time += (int32_t) count * per;
                }
                else
                {
                        /* Output amplitude transitions*/
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
                phase = (ph - duty_offset) & 7;
        }
        delay = time - end_time;
}

/* Quickly runs LFSR for a large number of clocks. For use when noise is generating*/
/* no sound.*/
static unsigned run_lfsr( unsigned s, unsigned mask, int count )
{
	/* optimization used in several places:*/
	/* ((s & (1 << b)) << n) ^ ((s & (1 << b)) << (n + 1)) = (s & (1 << b)) * (3 << n)*/

	if ( mask == 0x4000 )
	{
		if ( count >= 32767 )
			count %= 32767;

		/* Convert from Fibonacci to Galois configuration,*/
		/* shifted left 1 bit*/
		s ^= (s & 1) * 0x8000;

		/* Each iteration is equivalent to clocking LFSR 255 times*/
		while ( (count -= 255) > 0 )
			s ^= ((s & 0xE) << 12) ^ ((s & 0xE) << 11) ^ (s >> 3);
		count += 255;

		/* Each iteration is equivalent to clocking LFSR 15 times*/
		/* (interesting similarity to single clocking below)*/
		while ( (count -= 15) > 0 )
			s ^= ((s & 2) * (3 << 13)) ^ (s >> 1);
		count += 15;

		/* Remaining singles*/
		do{
			--count;
			s = ((s & 2) * (3 << 13)) ^ (s >> 1);
		}while(count >= 0);

		/* Convert back to Fibonacci configuration*/
		s &= 0x7FFF;
	}
	else if ( count < 8)
	{
		/* won't fully replace upper 8 bits, so have to do the unoptimized way*/
		do{
			--count;
			s = (s >> 1 | mask) ^ (mask & -((s - 1) & 2));
		}while(count >= 0);
	}
	else
	{
		if ( count > 127 )
		{
			count %= 127;
			if ( !count )
				count = 127; /* must run at least once*/
		}

		/* Need to keep one extra bit of history*/
		s = s << 1 & 0xFF;

		/* Convert from Fibonacci to Galois configuration,*/
		/* shifted left 2 bits*/
		s ^= (s & 2) << 7;

		/* Each iteration is equivalent to clocking LFSR 7 times*/
		/* (interesting similarity to single clocking below)*/
		while ( (count -= 7) > 0 )
			s ^= ((s & 4) * (3 << 5)) ^ (s >> 1);
		count += 7;

		/* Remaining singles*/
		while ( --count >= 0 )
			s = ((s & 4) * (3 << 5)) ^ (s >> 1);

		/* Convert back to Fibonacci configuration and*/
		/* repeat last 8 bits above significant 7*/
		s = (s << 7 & 0x7F80) | (s >> 1 & 0x7F);
	}

	return s;
}

void Gb_Noise::run( int32_t time, int32_t end_time )
{
        /* Determine what will be generated*/
        int vol = 0;
        Blip_Buffer* const out = output;
        if ( out )
        {
                int amp = dac_off_amp;
                if ( GB_ENV_DAC_ENABLED() )
                {
                        if ( enabled )
                                vol = volume;

                        amp = -DAC_BIAS;
                        if ( mode == MODE_AGB )
                                amp = -(vol >> 1);

                        if ( !(phase & 1) )
                        {
                                amp += vol;
                                vol = -vol;
                        }
                }

                /* AGB negates final output*/
                if ( mode == MODE_AGB )
                {
                        vol = -vol;
                        amp    = -amp;
                }

                update_amp( time, amp );
        }

        /* Run timer and calculate time of next LFSR clock*/
        static unsigned char const period1s [8] = { 1, 2, 4, 6, 8, 10, 12, 14 };
        int const period1 = period1s [regs [3] & 7] * CLK_MUL;
        {
                int extra = (end_time - time) - delay;
                int const per2 = GB_NOISE_PERIOD2(8);
                time += delay + ((divider ^ (per2 >> 1)) & (per2 - 1)) * period1;

                int count = (extra < 0 ? 0 : (extra + period1 - 1) / period1);
                divider = (divider - count) & PERIOD2_MASK;
                delay = count * period1 - extra;
        }

        /* Generate wave*/
        if ( time < end_time )
        {
                unsigned const mask = GB_NOISE_LFSR_MASK();
                unsigned bits = phase;

                int per = GB_NOISE_PERIOD2( period1 * 8 );
                if ( GB_NOISE_PERIOD2_INDEX() >= 0xE )
                {
                        time = end_time;
                }
                else if ( !vol )
                {
                        /* Maintain phase when not playing*/
                        int count = (end_time - time + per - 1) / per;
                        time += (int32_t) count * per;
                        bits = run_lfsr( bits, ~mask, count );
                }
                else
                {
                        /* Output amplitude transitions*/
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
                phase = bits;
        }
}


void Gb_Wave::run( int32_t time, int32_t end_time )
{
        /* Calc volume*/
        static unsigned char const volumes [8] = { 0, 4, 2, 1, 3, 3, 3, 3 };
        int const volume_idx = regs [2] >> 5 & (agb_mask | 3); /* 2 bits on DMG/CGB, 3 on AGB*/
        int const volume_mul = volumes [volume_idx];

        /* Determine what will be generated*/
        int playing = false;
        Blip_Buffer* const out = output;
        if ( out )
        {
                int amp = dac_off_amp;
                if ( GB_WAVE_DAC_ENABLED() )
                {
                        /* Play inaudible frequencies as constant amplitude*/
                        amp = 128; /* really depends on average of all samples in wave*/

                        /* if delay is larger, constant amplitude won't start yet*/
                        if ( GB_OSC_FREQUENCY() <= 0x7FB || delay > CLK_MUL_MUL_15 )
                        {
                                if ( volume_mul )
                                        playing = (int) enabled;

                                amp = (sample_buf << (phase << 2 & 4) & 0xF0) * playing;
                        }

                        amp = ((amp * volume_mul) >> VOLUME_SHIFT_PLUS_FOUR) - DAC_BIAS;
                }
                update_amp( time, amp );
        }

        /* Generate wave*/
        time += delay;
        if ( time < end_time )
        {
                unsigned char const* wave = wave_ram;

                /* wave size and bank*/
                int const flags = regs [0] & agb_mask;
                int const wave_mask = (flags & SIZE20_MASK) | 0x1F;
                int swap_banks = 0;
                if ( flags & BANK40_MASK)
                {
                        swap_banks = flags & SIZE20_MASK;
                        wave += BANK_SIZE_DIV_TWO - (swap_banks >> 1);
                }

                int ph = phase ^ swap_banks;
                ph = (ph + 1) & wave_mask; /* pre-advance*/

                int const per = period();
                if ( !playing )
                {
                        /* Maintain phase when not playing*/
                        int count = (end_time - time + per - 1) / per;
                        ph += count; /* will be masked below*/
                        time += (int32_t) count * per;
                }
                else
                {
                        /* Output amplitude transitions*/
                        int lamp = last_amp + DAC_BIAS;
                        do
                        {
                                /* Extract nybble*/
                                int nybble = wave [ph >> 1] << (ph << 2 & 4) & 0xF0;
                                ph = (ph + 1) & wave_mask;

                                /* Scale by volume*/
                                int amp = (nybble * volume_mul) >> VOLUME_SHIFT_PLUS_FOUR;

                                int delta = amp - lamp;
                                if ( delta )
                                {
                                        lamp = amp;
                                        med_synth->offset_inline( time, delta, out );
                                }
                                time += per;
                        }
                        while ( time < end_time );
                        last_amp = lamp - DAC_BIAS;
                }
                ph = (ph - 1) & wave_mask; /* undo pre-advance and mask position*/

                /* Keep track of last byte read*/
                if ( enabled )
                        sample_buf = wave [ph >> 1];

                phase = ph ^ swap_banks; /* undo swapped banks*/
        }
        delay = time - end_time;
}
