// Core SPC emulation: CPU, timers, SMP registers, memory

// snes_spc 0.9.0. http://www.slack.net/~ant/

#include "SNES_SPC.h"

#include <string.h>

/* Copyright (C) 2004-2007 Shay Green. This module is free software; you
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

#define RAM         (m.ram.ram)
#define REGS        (m.smp_regs [0])
#define REGS_IN     (m.smp_regs [1])

// (n ? n : 256)
#define IF_0_THEN_256( n ) ((uint8_t) ((n) - 1) + 1)

// Note: SPC_MORE_ACCURACY exists mainly so I can run my validation tests, which
// do crazy echo buffer accesses.
#ifndef SPC_MORE_ACCURACY
	#define SPC_MORE_ACCURACY 0
#endif

#ifdef BLARGG_ENABLE_OPTIMIZER
	#include BLARGG_ENABLE_OPTIMIZER
#endif


//// Timers

#if SPC_DISABLE_TEMPO
	#define TIMER_DIV( t, n ) ((n) >> t->prescaler)
	#define TIMER_MUL( t, n ) ((n) << t->prescaler)
#else
	#define TIMER_DIV( t, n ) ((n) / t->prescaler)
	#define TIMER_MUL( t, n ) ((n) * t->prescaler)
#endif

SNES_SPC::Timer* SNES_SPC::run_timer_( Timer* t, rel_time_t time )
{
	int elapsed = TIMER_DIV( t, time - t->next_time ) + 1;
	t->next_time += TIMER_MUL( t, elapsed );
	
	if ( t->enabled )
	{
		int remain = IF_0_THEN_256( t->period - t->divider );
		int divider = t->divider + elapsed;
		int over = elapsed - remain;
		if ( over >= 0 )
		{
			int n = over / t->period;
			t->counter = (t->counter + 1 + n) & 0x0F;
			divider = over - n * t->period;
		}
		t->divider = (uint8_t) divider;
	}
	return t;
}

inline SNES_SPC::Timer* SNES_SPC::run_timer( Timer* t, rel_time_t time )
{
	if ( time >= t->next_time )
		t = run_timer_( t, time );
	return t;
}


//// ROM

void SNES_SPC::enable_rom( int enable )
{
	if ( m.rom_enabled != enable )
	{
		m.rom_enabled = dsp.rom_enabled = enable;
		if ( enable )
			memcpy( m.hi_ram, &RAM [rom_addr], sizeof m.hi_ram );
		memcpy( &RAM [rom_addr], (enable ? m.rom : m.hi_ram), rom_size );
		// TODO: ROM can still get overwritten when DSP writes to echo buffer
	}
}


//// DSP

#if SPC_LESS_ACCURATE
	int const max_reg_time = 29;
	
	signed char const SNES_SPC::reg_times_ [256] =
	{
		 -1,  0,-11,-10,-15,-11, -2, -2,  4,  3, 14, 14, 26, 26, 14, 22,
		  2,  3,  0,  1,-12,  0,  1,  1,  7,  6, 14, 14, 27, 14, 14, 23,
		  5,  6,  3,  4, -1,  3,  4,  4, 10,  9, 14, 14, 26, -5, 14, 23,
		  8,  9,  6,  7,  2,  6,  7,  7, 13, 12, 14, 14, 27, -4, 14, 24,
		 11, 12,  9, 10,  5,  9, 10, 10, 16, 15, 14, 14, -2, -4, 14, 24,
		 14, 15, 12, 13,  8, 12, 13, 13, 19, 18, 14, 14, -2,-36, 14, 24,
		 17, 18, 15, 16, 11, 15, 16, 16, 22, 21, 14, 14, 28, -3, 14, 25,
		 20, 21, 18, 19, 14, 18, 19, 19, 25, 24, 14, 14, 14, 29, 14, 25,
		 
		 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
		 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
		 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
		 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
		 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
		 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
		 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
		 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	};
	
	#define RUN_DSP( time, offset ) \
		int count = (time) - (offset) - m.dsp_time;\
		if ( count >= 0 )\
		{\
			int clock_count = (count & ~(clocks_per_sample - 1)) + clocks_per_sample;\
			m.dsp_time += clock_count;\
			dsp.run( clock_count );\
		}
#else
	#define RUN_DSP( time, offset ) \
		{\
			int count = (time) - m.dsp_time;\
			if ( !SPC_MORE_ACCURACY || count )\
			{\
				assert( count > 0 );\
				m.dsp_time = (time);\
				dsp.run( count );\
			}\
		}
#endif

int SNES_SPC::dsp_read( rel_time_t time )
{
	RUN_DSP( time, reg_times [REGS [r_dspaddr] & 0x7F] );
	
	int result = dsp.read( REGS [r_dspaddr] & 0x7F );
	
	#ifdef SPC_DSP_READ_HOOK
		SPC_DSP_READ_HOOK( spc_time + time, (REGS [r_dspaddr] & 0x7F), result );
	#endif
	
	return result;
}

inline void SNES_SPC::dsp_write( int data, rel_time_t time )
{
	RUN_DSP( time, reg_times [REGS [r_dspaddr]] )
	#if SPC_LESS_ACCURATE
		else if ( m.dsp_time == skipping_time )
		{
			int r = REGS [r_dspaddr];
			if ( r == SPC_DSP::r_kon )
				m.skipped_kon |= data & ~dsp.read( SPC_DSP::r_koff );
			
			if ( r == SPC_DSP::r_koff )
			{
				m.skipped_koff |= data;
				m.skipped_kon &= ~data;
			}
		}
	#endif
	
	#ifdef SPC_DSP_WRITE_HOOK
		SPC_DSP_WRITE_HOOK( m.spc_time + time, REGS [r_dspaddr], (uint8_t) data );
	#endif
	
	if ( REGS [r_dspaddr] <= 0x7F )
		dsp.write( REGS [r_dspaddr], data );
	else if ( !SPC_MORE_ACCURACY )
		dprintf( "SPC wrote to DSP register > $7F\n" );
}


//// Memory access extras

#if SPC_MORE_ACCURACY
	#define MEM_ACCESS( time, addr ) \
	{\
		if ( time >= m.dsp_time )\
		{\
			RUN_DSP( time, max_reg_time );\
		}\
	}
#elif !defined (NDEBUG)
	// Debug-only check for read/write within echo buffer, since this might result in
	// inaccurate emulation due to the DSP not being caught up to the present.
	
	bool SNES_SPC::check_echo_access( int addr )
	{
		if ( !(dsp.read( SPC_DSP::r_flg ) & 0x20) )
		{
			int start = 0x100 * dsp.read( SPC_DSP::r_esa );
			int size  = 0x800 * (dsp.read( SPC_DSP::r_edl ) & 0x0F);
			int end   = start + (size ? size : 4);
			if ( start <= addr && addr < end )
			{
				if ( !m.echo_accessed )
				{
					m.echo_accessed = 1;
					return true;
				}
			}
		}
		return false;
	}
	
	#define MEM_ACCESS( time, addr ) check( !check_echo_access( (uint16_t) addr ) );
#else
	#define MEM_ACCESS( time, addr )
#endif


//// CPU write

#if SPC_MORE_ACCURACY
static unsigned char const glitch_probs [3] [256] =
{
	0xC3,0x92,0x5B,0x1C,0xD1,0x92,0x5B,0x1C,0xDB,0x9C,0x72,0x18,0xCD,0x5C,0x38,0x0B,
	0xE1,0x9C,0x74,0x17,0xCF,0x75,0x45,0x0C,0xCF,0x6E,0x4A,0x0D,0xA3,0x3A,0x1D,0x08,
	0xDB,0xA0,0x82,0x19,0xD9,0x73,0x3C,0x0E,0xCB,0x76,0x52,0x0B,0xA5,0x46,0x1D,0x09,
	0xDA,0x74,0x55,0x0F,0xA2,0x3F,0x21,0x05,0x9A,0x40,0x20,0x07,0x63,0x1E,0x10,0x01,
	0xDF,0xA9,0x85,0x1D,0xD3,0x84,0x4B,0x0E,0xCF,0x6F,0x49,0x0F,0xB3,0x48,0x1E,0x05,
	0xD8,0x77,0x52,0x12,0xB7,0x49,0x23,0x06,0xAA,0x45,0x28,0x07,0x7D,0x28,0x0F,0x07,
	0xCC,0x7B,0x4A,0x0E,0xB2,0x4F,0x24,0x07,0xAD,0x43,0x2C,0x06,0x86,0x29,0x11,0x07,
	0xAE,0x48,0x1F,0x0A,0x76,0x21,0x19,0x05,0x76,0x21,0x14,0x05,0x44,0x11,0x0B,0x01,
	0xE7,0xAD,0x96,0x23,0xDC,0x86,0x59,0x0E,0xDC,0x7C,0x5F,0x15,0xBB,0x53,0x2E,0x09,
	0xD6,0x7C,0x4A,0x16,0xBB,0x4A,0x25,0x08,0xB3,0x4F,0x28,0x0B,0x8E,0x23,0x15,0x08,
	0xCF,0x7F,0x57,0x11,0xB5,0x4A,0x23,0x0A,0xAA,0x42,0x28,0x05,0x7D,0x22,0x12,0x03,
	0xA6,0x49,0x28,0x09,0x82,0x2B,0x0D,0x04,0x7A,0x20,0x0F,0x04,0x3D,0x0F,0x09,0x03,
	0xD1,0x7C,0x4C,0x0F,0xAF,0x4E,0x21,0x09,0xA8,0x46,0x2A,0x07,0x85,0x1F,0x0E,0x07,
	0xA6,0x3F,0x26,0x07,0x7C,0x24,0x14,0x07,0x78,0x22,0x16,0x04,0x46,0x12,0x0A,0x02,
	0xA6,0x41,0x2C,0x0A,0x7E,0x28,0x11,0x05,0x73,0x1B,0x14,0x05,0x3D,0x11,0x0A,0x02,
	0x70,0x22,0x17,0x05,0x48,0x13,0x08,0x03,0x3C,0x07,0x0D,0x07,0x26,0x07,0x06,0x01,
	
	0xE0,0x9F,0xDA,0x7C,0x4F,0x18,0x28,0x0D,0xE9,0x9F,0xDA,0x7C,0x4F,0x18,0x1F,0x07,
	0xE6,0x97,0xD8,0x72,0x64,0x13,0x26,0x09,0xDC,0x67,0xA9,0x38,0x21,0x07,0x15,0x06,
	0xE9,0x91,0xD2,0x6B,0x63,0x14,0x2B,0x0E,0xD6,0x61,0xB7,0x41,0x2B,0x0E,0x10,0x09,
	0xCF,0x59,0xB0,0x2F,0x35,0x08,0x0F,0x07,0xB6,0x30,0x7A,0x21,0x17,0x07,0x09,0x03,
	0xE7,0xA3,0xE5,0x6B,0x65,0x1F,0x34,0x09,0xD8,0x6B,0xBE,0x45,0x27,0x07,0x10,0x07,
	0xDA,0x54,0xB1,0x39,0x2E,0x0E,0x17,0x08,0xA9,0x3C,0x86,0x22,0x16,0x06,0x07,0x03,
	0xD4,0x51,0xBC,0x3D,0x38,0x0A,0x13,0x06,0xB2,0x37,0x79,0x1C,0x17,0x05,0x0E,0x06,
	0xA7,0x31,0x74,0x1C,0x11,0x06,0x0C,0x02,0x6D,0x1A,0x38,0x10,0x0B,0x05,0x06,0x03,
	0xEB,0x9A,0xE1,0x7A,0x6F,0x13,0x34,0x0E,0xE6,0x75,0xC5,0x45,0x3E,0x0B,0x1A,0x05,
	0xD8,0x63,0xC1,0x40,0x3C,0x1B,0x19,0x06,0xB3,0x42,0x83,0x29,0x18,0x0A,0x08,0x04,
	0xD4,0x58,0xBA,0x43,0x3F,0x0A,0x1F,0x09,0xB1,0x33,0x8A,0x1F,0x1F,0x06,0x0D,0x05,
	0xAF,0x3C,0x7A,0x1F,0x16,0x08,0x0A,0x01,0x72,0x1B,0x52,0x0D,0x0B,0x09,0x06,0x01,
	0xCF,0x63,0xB7,0x47,0x40,0x10,0x14,0x06,0xC0,0x41,0x96,0x20,0x1C,0x09,0x10,0x05,
	0xA6,0x35,0x82,0x1A,0x20,0x0C,0x0E,0x04,0x80,0x1F,0x53,0x0F,0x0B,0x02,0x06,0x01,
	0xA6,0x31,0x81,0x1B,0x1D,0x01,0x08,0x08,0x7B,0x20,0x4D,0x19,0x0E,0x05,0x07,0x03,
	0x6B,0x17,0x49,0x07,0x0E,0x03,0x0A,0x05,0x37,0x0B,0x1F,0x06,0x04,0x02,0x07,0x01,
	
	0xF0,0xD6,0xED,0xAD,0xEC,0xB1,0xEB,0x79,0xAC,0x22,0x47,0x1E,0x6E,0x1B,0x32,0x0A,
	0xF0,0xD6,0xEA,0xA4,0xED,0xC4,0xDE,0x82,0x98,0x1F,0x50,0x13,0x52,0x15,0x2A,0x0A,
	0xF1,0xD1,0xEB,0xA2,0xEB,0xB7,0xD8,0x69,0xA2,0x1F,0x5B,0x18,0x55,0x18,0x2C,0x0A,
	0xED,0xB5,0xDE,0x7E,0xE6,0x85,0xD3,0x59,0x59,0x0F,0x2C,0x09,0x24,0x07,0x15,0x09,
	0xF1,0xD6,0xEA,0xA0,0xEC,0xBB,0xDA,0x77,0xA9,0x23,0x58,0x14,0x5D,0x12,0x2F,0x09,
	0xF1,0xC1,0xE3,0x86,0xE4,0x87,0xD2,0x4E,0x68,0x15,0x26,0x0B,0x27,0x09,0x15,0x02,
	0xEE,0xA6,0xE0,0x5C,0xE0,0x77,0xC3,0x41,0x67,0x1B,0x3C,0x07,0x2A,0x06,0x19,0x07,
	0xE4,0x75,0xC6,0x43,0xCC,0x50,0x95,0x23,0x35,0x09,0x14,0x04,0x15,0x05,0x0B,0x04,
	0xEE,0xD6,0xED,0xAD,0xEC,0xB1,0xEB,0x79,0xAC,0x22,0x56,0x14,0x5A,0x12,0x26,0x0A,
	0xEE,0xBB,0xE7,0x7E,0xE9,0x8D,0xCB,0x49,0x67,0x11,0x34,0x07,0x2B,0x0B,0x14,0x07,
	0xED,0xA7,0xE5,0x76,0xE3,0x7E,0xC4,0x4B,0x77,0x14,0x34,0x08,0x27,0x07,0x14,0x04,
	0xE7,0x8B,0xD2,0x4C,0xCA,0x56,0x9E,0x31,0x36,0x0C,0x11,0x07,0x14,0x04,0x0A,0x02,
	0xF0,0x9B,0xEA,0x6F,0xE5,0x81,0xC4,0x43,0x74,0x10,0x30,0x0B,0x2D,0x08,0x1B,0x06,
	0xE6,0x83,0xCA,0x48,0xD9,0x56,0xA7,0x23,0x3B,0x09,0x12,0x09,0x15,0x07,0x0A,0x03,
	0xE5,0x5F,0xCB,0x3C,0xCF,0x48,0x91,0x22,0x31,0x0A,0x17,0x08,0x15,0x04,0x0D,0x02,
	0xD1,0x43,0x91,0x20,0xA9,0x2D,0x54,0x12,0x17,0x07,0x09,0x02,0x0C,0x04,0x05,0x03,
};
#endif

// divided into multiple functions to keep rarely-used functionality separate
// so often-used functionality can be optimized better by compiler

// If write isn't preceded by read, data has this added to it
int const no_read_before_write = 0x2000;

void SNES_SPC::cpu_write_smp_reg_( int data, rel_time_t time, int addr )
{
	switch ( addr )
	{
	case r_t0target:
	case r_t1target:
	case r_t2target: {
		Timer* t = &m.timers [addr - r_t0target];
		int period = IF_0_THEN_256( data );
		if ( t->period != period )
		{
			t = run_timer( t, time );
			#if SPC_MORE_ACCURACY
				// Insane behavior when target is written just after counter is
				// clocked and counter matches new period and new period isn't 1, 2, 4, or 8
				if ( t->divider == (period & 0xFF) &&
						t->next_time == time + TIMER_MUL( t, 1 ) &&
						((period - 1) | ~0x0F) & period )
				{
					//dprintf( "SPC pathological timer target write\n" );
					
					// If the period is 3, 5, or 9, there's a probability this behavior won't occur,
					// based on the previous period
					int prob = 0xFF;
					int old_period = t->period & 0xFF;
					if ( period == 3 ) prob = glitch_probs [0] [old_period];
					if ( period == 5 ) prob = glitch_probs [1] [old_period];
					if ( period == 9 ) prob = glitch_probs [2] [old_period];
					
					// The glitch suppresses incrementing of one of the counter bits, based on
					// the lowest set bit in the new period
					int b = 1;
					while ( !(period & b) )
						b <<= 1;
					
					if ( (rand() >> 4 & 0xFF) <= prob )
						t->divider = (t->divider - b) & 0xFF;
				}
			#endif
			t->period = period;
		}
		break;
	}
	
	case r_t0out:
	case r_t1out:
	case r_t2out:
		if ( !SPC_MORE_ACCURACY )
			dprintf( "SPC wrote to counter %d\n", (int) addr - r_t0out );
		
		if ( data < no_read_before_write  / 2 )
			run_timer( &m.timers [addr - r_t0out], time - 1 )->counter = 0;
		break;
	
	// Registers that act like RAM
	case 0x8:
	case 0x9:
		REGS_IN [addr] = (uint8_t) data;
		break;
	
	case r_test:
		if ( (uint8_t) data != 0x0A )
			dprintf( "SPC wrote to test register\n" );
		break;
	
	case r_control:
		// port clears
		if ( data & 0x10 )
		{
			REGS_IN [r_cpuio0] = 0;
			REGS_IN [r_cpuio1] = 0;
		}
		if ( data & 0x20 )
		{
			REGS_IN [r_cpuio2] = 0;
			REGS_IN [r_cpuio3] = 0;
		}
		
		// timers
		{
			for ( int i = 0; i < timer_count; i++ )
			{
				Timer* t = &m.timers [i];
				int enabled = data >> i & 1;
				if ( t->enabled != enabled )
				{
					t = run_timer( t, time );
					t->enabled = enabled;
					if ( enabled )
					{
						t->divider = 0;
						t->counter = 0;
					}
				}
			}
		}
		enable_rom( data & 0x80 );
		break;
	}
}

void SNES_SPC::cpu_write_smp_reg( int data, rel_time_t time, int addr )
{
	if ( addr == r_dspdata ) // 99%
		dsp_write( data, time );
	else
		cpu_write_smp_reg_( data, time, addr );
}

void SNES_SPC::cpu_write_high( int data, int i, rel_time_t time )
{
	if ( i < rom_size )
	{
		m.hi_ram [i] = (uint8_t) data;
		if ( m.rom_enabled )
			RAM [i + rom_addr] = m.rom [i]; // restore overwritten ROM
	}
	else
	{
		assert( *(&(RAM [0]) + i + rom_addr) == (uint8_t) data );
		*(&(RAM [0]) + i + rom_addr) = cpu_pad_fill; // restore overwritten padding
		cpu_write( data, i + rom_addr - 0x10000, time );
	}
}

int const bits_in_int = CHAR_BIT * sizeof (int);

void SNES_SPC::cpu_write( int data, int addr, rel_time_t time )
{
	MEM_ACCESS( time, addr )
	
	// RAM
	RAM [addr] = (uint8_t) data;
	int reg = addr - 0xF0;
	if ( reg >= 0 ) // 64%
	{
		// $F0-$FF
		if ( reg < reg_count ) // 87%
		{
			REGS [reg] = (uint8_t) data;
			
			// Ports
			#ifdef SPC_PORT_WRITE_HOOK
				if ( (unsigned) (reg - r_cpuio0) < port_count )
					SPC_PORT_WRITE_HOOK( m.spc_time + time, (reg - r_cpuio0),
							(uint8_t) data, &REGS [r_cpuio0] );
			#endif
			
			// Registers other than $F2 and $F4-$F7
			//if ( reg != 2 && reg != 4 && reg != 5 && reg != 6 && reg != 7 )
			// TODO: this is a bit on the fragile side
			if ( ((~0x2F00 << (bits_in_int - 16)) << reg) < 0 ) // 36%
				cpu_write_smp_reg( data, time, reg );
		}
		// High mem/address wrap-around
		else
		{
			reg -= rom_addr - 0xF0;
			if ( reg >= 0 ) // 1% in IPL ROM area or address wrapped around
				cpu_write_high( data, reg, time );
		}
	}
}


//// CPU read

inline int SNES_SPC::cpu_read_smp_reg( int reg, rel_time_t time )
{
	int result = REGS_IN [reg];
	reg -= r_dspaddr;
	// DSP addr and data
	if ( (unsigned) reg <= 1 ) // 4% 0xF2 and 0xF3
	{
		result = REGS [r_dspaddr];
		if ( (unsigned) reg == 1 )
			result = dsp_read( time ); // 0xF3
	}
	return result;
}

int SNES_SPC::cpu_read( int addr, rel_time_t time )
{
	MEM_ACCESS( time, addr )
	
	// RAM
	int result = RAM [addr];
	int reg = addr - 0xF0;
	if ( reg >= 0 ) // 40%
	{
		reg -= 0x10;
		if ( (unsigned) reg >= 0xFF00 ) // 21%
		{
			reg += 0x10 - r_t0out;
			
			// Timers
			if ( (unsigned) reg < timer_count ) // 90%
			{
				Timer* t = &m.timers [reg];
				if ( time >= t->next_time )
					t = run_timer_( t, time );
				result = t->counter;
				t->counter = 0;
			}
			// Other registers
			else if ( reg < 0 ) // 10%
			{
				result = cpu_read_smp_reg( reg + r_t0out, time );
			}
			else // 1%
			{
				assert( reg + (r_t0out + 0xF0 - 0x10000) < 0x100 );
				result = cpu_read( reg + (r_t0out + 0xF0 - 0x10000), time );
			}
		}
	}
	
	return result;
}


//// Run

// Prefix and suffix for CPU emulator function
#define SPC_CPU_RUN_FUNC \
BOOST::uint8_t* SNES_SPC::run_until_( time_t end_time )\
{\
	rel_time_t rel_time = m.spc_time - end_time;\
	/*assert( rel_time <= 0 );*/\
	m.spc_time = end_time;\
	m.dsp_time += rel_time;\
	m.timers [0].next_time += rel_time;\
	m.timers [1].next_time += rel_time;\
	m.timers [2].next_time += rel_time;

#define SPC_CPU_RUN_FUNC_END \
	m.spc_time += rel_time;\
	m.dsp_time -= rel_time;\
	m.timers [0].next_time -= rel_time;\
	m.timers [1].next_time -= rel_time;\
	m.timers [2].next_time -= rel_time;\
	/*assert( m.spc_time >= end_time );*/\
	return &REGS [r_cpuio0];\
}

int const cpu_lag_max = 12 - 1; // DIV YA,X takes 12 clocks

void SNES_SPC::end_frame( time_t end_time )
{
	// Catch CPU up to as close to end as possible. If final instruction
	// would exceed end, does NOT execute it and leaves m.spc_time < end.
	if ( end_time > m.spc_time )
		run_until_( end_time );
	
	m.spc_time     -= end_time;
	m.extra_clocks += end_time;
	
	// Greatest number of clocks early that emulation can stop early due to
	// not being able to execute current instruction without going over
	// allowed time.
	assert( -cpu_lag_max <= m.spc_time && m.spc_time <= cpu_lag_max );
	
	// Catch timers up to CPU
	for ( int i = 0; i < timer_count; i++ )
		run_timer( &m.timers [i], 0 );
	
	// Catch DSP up to CPU
	if ( m.dsp_time < 0 )
	{
		RUN_DSP( 0, max_reg_time );
	}
	
	// Save any extra samples beyond what should be generated
	if ( m.buf_begin )
		save_extra();
}

// Inclusion here allows static memory access functions and better optimization
#include "SPC_CPU.h"
