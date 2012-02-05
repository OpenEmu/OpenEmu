// SPC emulation support: init, sample buffering, reset, SPC loading

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


//// Init

blargg_err_t SNES_SPC::init()
{
	memset( &m, 0, sizeof m );
	dsp.init( RAM );
	
	m.tempo = tempo_unit;
	
	// Most SPC music doesn't need ROM, and almost all the rest only rely
	// on these two bytes
	m.rom [0x3E] = 0xFF;
	m.rom [0x3F] = 0xC0;
	
	static unsigned char const cycle_table [128] =
	{//   01   23   45   67   89   AB   CD   EF
	    0x28,0x47,0x34,0x36,0x26,0x54,0x54,0x68, // 0
	    0x48,0x47,0x45,0x56,0x55,0x65,0x22,0x46, // 1
	    0x28,0x47,0x34,0x36,0x26,0x54,0x54,0x74, // 2
	    0x48,0x47,0x45,0x56,0x55,0x65,0x22,0x38, // 3
	    0x28,0x47,0x34,0x36,0x26,0x44,0x54,0x66, // 4
	    0x48,0x47,0x45,0x56,0x55,0x45,0x22,0x43, // 5
	    0x28,0x47,0x34,0x36,0x26,0x44,0x54,0x75, // 6
	    0x48,0x47,0x45,0x56,0x55,0x55,0x22,0x36, // 7
	    0x28,0x47,0x34,0x36,0x26,0x54,0x52,0x45, // 8
	    0x48,0x47,0x45,0x56,0x55,0x55,0x22,0xC5, // 9
	    0x38,0x47,0x34,0x36,0x26,0x44,0x52,0x44, // A
	    0x48,0x47,0x45,0x56,0x55,0x55,0x22,0x34, // B
	    0x38,0x47,0x45,0x47,0x25,0x64,0x52,0x49, // C
	    0x48,0x47,0x56,0x67,0x45,0x55,0x22,0x83, // D
	    0x28,0x47,0x34,0x36,0x24,0x53,0x43,0x40, // E
	    0x48,0x47,0x45,0x56,0x34,0x54,0x22,0x60, // F
	};
	
	// unpack cycle table
	for ( int i = 0; i < 128; i++ )
	{
		int n = cycle_table [i];
		m.cycle_table [i * 2 + 0] = n >> 4;
		m.cycle_table [i * 2 + 1] = n & 0x0F;
	}

	allow_time_overflow = false;

	dsp.rom = m.rom;
	dsp.hi_ram = m.hi_ram;

#ifdef DEBUGGER
	apu_trace = NULL;
	debug_trace = false;
#endif

	#if SPC_LESS_ACCURATE
		memcpy( reg_times, reg_times_, sizeof reg_times );
	#endif
	
	reset();
	return 0;
}

void SNES_SPC::init_rom( uint8_t const in [rom_size] )
{
	memcpy( m.rom, in, sizeof m.rom );
}

void SNES_SPC::set_tempo( int t )
{
	m.tempo = t;
	int const timer2_shift = 4; // 64 kHz
	int const other_shift  = 3; //  8 kHz
	
	#if SPC_DISABLE_TEMPO
		m.timers [2].prescaler = timer2_shift;
		m.timers [1].prescaler = timer2_shift + other_shift;
		m.timers [0].prescaler = timer2_shift + other_shift;
	#else
		if ( !t )
			t = 1;
		int const timer2_rate  = 1 << timer2_shift;
		int rate = (timer2_rate * tempo_unit + (t >> 1)) / t;
		if ( rate < timer2_rate / 4 )
			rate = timer2_rate / 4; // max 4x tempo
		m.timers [2].prescaler = rate;
		m.timers [1].prescaler = rate << other_shift;
		m.timers [0].prescaler = rate << other_shift;
	#endif
}

// Timer registers have been loaded. Applies these to the timers. Does not
// reset timer prescalers or dividers.
void SNES_SPC::timers_loaded()
{
	int i;
	for ( i = 0; i < timer_count; i++ )
	{
		Timer* t = &m.timers [i];
		t->period  = IF_0_THEN_256( REGS [r_t0target + i] );
		t->enabled = REGS [r_control] >> i & 1;
		t->counter = REGS_IN [r_t0out + i] & 0x0F;
	}
	
	set_tempo( m.tempo );
}

// Loads registers from unified 16-byte format
void SNES_SPC::load_regs( uint8_t const in [reg_count] )
{
	memcpy( REGS, in, reg_count );
	memcpy( REGS_IN, REGS, reg_count );
	
	// These always read back as 0
	REGS_IN [r_test    ] = 0;
	REGS_IN [r_control ] = 0;
	REGS_IN [r_t0target] = 0;
	REGS_IN [r_t1target] = 0;
	REGS_IN [r_t2target] = 0;
}

// RAM was just loaded from SPC, with $F0-$FF containing SMP registers
// and timer counts. Copies these to proper registers.
void SNES_SPC::ram_loaded()
{
	m.rom_enabled = dsp.rom_enabled = 0;
	load_regs( &RAM [0xF0] );
	
	// Put STOP instruction around memory to catch PC underflow/overflow
	memset( m.ram.padding1, cpu_pad_fill, sizeof m.ram.padding1 );
	memset( m.ram.padding2, cpu_pad_fill, sizeof m.ram.padding2 );
}

// Registers were just loaded. Applies these new values.
void SNES_SPC::regs_loaded()
{
	enable_rom( REGS [r_control] & 0x80 );
	timers_loaded();
}

void SNES_SPC::reset_time_regs()
{
	m.cpu_error     = 0;
	m.echo_accessed = 0;
	m.spc_time      = 0;
	m.dsp_time      = 0;
	#if SPC_LESS_ACCURATE
		m.dsp_time = clocks_per_sample + 1;
	#endif
	
	for ( int i = 0; i < timer_count; i++ )
	{
		Timer* t = &m.timers [i];
		t->next_time = 1;
		t->divider   = 0;
	}
	
	regs_loaded();
	
	m.extra_clocks = 0;
	reset_buf();
}

void SNES_SPC::reset_common( int timer_counter_init )
{
	int i;
	for ( i = 0; i < timer_count; i++ )
		REGS_IN [r_t0out + i] = timer_counter_init;
	
	// Run IPL ROM
	memset( &m.cpu_regs, 0, sizeof m.cpu_regs );
	m.cpu_regs.pc = rom_addr;
	
	REGS [r_test   ] = 0x0A;
	REGS [r_control] = 0xB0; // ROM enabled, clear ports
	for ( i = 0; i < port_count; i++ )
		REGS_IN [r_cpuio0 + i] = 0;
	
	reset_time_regs();
}

void SNES_SPC::soft_reset()
{
	reset_common( 0 );
	dsp.soft_reset();
}

void SNES_SPC::reset()
{
	m.cpu_regs.pc  = 0xFFC0;
	m.cpu_regs.a   = 0x00;
	m.cpu_regs.x   = 0x00;
	m.cpu_regs.y   = 0x00;
	m.cpu_regs.psw = 0x02;
	m.cpu_regs.sp  = 0xEF;
	memset( RAM, 0x00, 0x10000 );
	ram_loaded();
	reset_common( 0x0F );
	dsp.reset();
}

char const SNES_SPC::signature [signature_size + 1] =
		"SNES-SPC700 Sound File Data v0.30\x1A\x1A";

blargg_err_t SNES_SPC::load_spc( void const* data, long size )
{
	spc_file_t const* const spc = (spc_file_t const*) data;
	
	// be sure compiler didn't insert any padding into fle_t
	assert( sizeof (spc_file_t) == spc_min_file_size + 0x80 );
	
	// Check signature and file size
	if ( size < signature_size || memcmp( spc, signature, 27 ) )
		return "Not an SPC file";
	
	if ( size < spc_min_file_size )
		return "Corrupt SPC file";
	
	// CPU registers
	m.cpu_regs.pc  = spc->pch * 0x100 + spc->pcl;
	m.cpu_regs.a   = spc->a;
	m.cpu_regs.x   = spc->x;
	m.cpu_regs.y   = spc->y;
	m.cpu_regs.psw = spc->psw;
	m.cpu_regs.sp  = spc->sp;
	
	// RAM and registers
	memcpy( RAM, spc->ram, 0x10000 );
	ram_loaded();
	
	// DSP registers
	dsp.load( spc->dsp );
	
	reset_time_regs();
	
	return 0;
}

void SNES_SPC::clear_echo()
{
	if ( !(dsp.read( SPC_DSP::r_flg ) & 0x20) )
	{
		int addr = 0x100 * dsp.read( SPC_DSP::r_esa );
		int end  = addr + 0x800 * (dsp.read( SPC_DSP::r_edl ) & 0x0F);
		if ( end > 0x10000 )
			end = 0x10000;
		memset( &RAM [addr], 0xFF, end - addr );
	}
}


//// Sample output

void SNES_SPC::reset_buf()
{
	// Start with half extra buffer of silence
	sample_t* out = m.extra_buf;
	while ( out < &m.extra_buf [extra_size / 2] )
		*out++ = 0;
	
	m.extra_pos = out;
	m.buf_begin = 0;
	
	dsp.set_output( 0, 0 );
}

void SNES_SPC::set_output( sample_t* out, int size )
{
	require( (size & 1) == 0 ); // size must be even
	
	m.extra_clocks &= clocks_per_sample - 1;
	if ( out )
	{
		sample_t const* out_end = out + size;
		m.buf_begin = out;
		m.buf_end   = out_end;
		
		// Copy extra to output
		sample_t const* in = m.extra_buf;
		while ( in < m.extra_pos && out < out_end )
			*out++ = *in++;
		
		// Handle output being full already
		if ( out >= out_end )
		{
			// Have DSP write to remaining extra space
			out     = dsp.extra();
			out_end = &dsp.extra() [extra_size];
			
			// Copy any remaining extra samples as if DSP wrote them
			while ( in < m.extra_pos )
				*out++ = *in++;
			assert( out <= out_end );
		}
		
		dsp.set_output( out, out_end - out );
	}
	else
	{
		reset_buf();
	}
}

void SNES_SPC::save_extra()
{
	// Get end pointers
	sample_t const* main_end = m.buf_end;     // end of data written to buf
	sample_t const* dsp_end  = dsp.out_pos(); // end of data written to dsp.extra()
	if ( m.buf_begin <= dsp_end && dsp_end <= main_end )
	{
		main_end = dsp_end;
		dsp_end  = dsp.extra(); // nothing in DSP's extra
	}
	
	// Copy any extra samples at these ends into extra_buf
	sample_t* out = m.extra_buf;
	sample_t const* in;
	for ( in = m.buf_begin + sample_count(); in < main_end; in++ )
		*out++ = *in;
	for ( in = dsp.extra(); in < dsp_end ; in++ )
		*out++ = *in;
	
	m.extra_pos = out;
	assert( out <= &m.extra_buf [extra_size] );
}

blargg_err_t SNES_SPC::play( int count, sample_t* out )
{
	require( (count & 1) == 0 ); // must be even
	if ( count )
	{
		set_output( out, count );
		end_frame( count * (clocks_per_sample / 2) );
	}
	
	const char* err = m.cpu_error;
	m.cpu_error = 0;
	return err;
}

blargg_err_t SNES_SPC::skip( int count )
{
	#if SPC_LESS_ACCURATE
	if ( count > 2 * sample_rate * 2 )
	{
		set_output( 0, 0 );
		
		// Skip a multiple of 4 samples
		time_t end = count;
		count = (count & 3) + 1 * sample_rate * 2;
		end = (end - count) * (clocks_per_sample / 2);
		
		m.skipped_kon  = 0;
		m.skipped_koff = 0;
		
		// Preserve DSP and timer synchronization
		// TODO: verify that this really preserves it
		int old_dsp_time = m.dsp_time + m.spc_time;
		m.dsp_time = end - m.spc_time + skipping_time;
		end_frame( end );
		m.dsp_time = m.dsp_time - skipping_time + old_dsp_time;
		
		dsp.write( SPC_DSP::r_koff, m.skipped_koff & ~m.skipped_kon );
		dsp.write( SPC_DSP::r_kon , m.skipped_kon );
		clear_echo();
	}
	#endif
	
	return play( count, 0 );
}

//// Snes9x Accessor

void SNES_SPC::dsp_set_spc_snapshot_callback( void (*callback) (void) )
{
	dsp.set_spc_snapshot_callback( callback );
}

void SNES_SPC::dsp_dump_spc_snapshot( void )
{
	dsp.dump_spc_snapshot();
}

void SNES_SPC::dsp_set_stereo_switch( int value )
{
	dsp.set_stereo_switch( value );
}

SNES_SPC::uint8_t SNES_SPC::dsp_reg_value( int ch, int addr )
{
	return dsp.reg_value( ch, addr );
}

int SNES_SPC::dsp_envx_value( int ch )
{
	return dsp.envx_value( ch );
}

//// Snes9x debugger

#ifdef DEBUGGER

void SNES_SPC::debug_toggle_trace( void )
{
	debug_trace = !debug_trace;

	if (debug_trace)
	{
		printf("APU tracing enabled.\n");
		ENSURE_TRACE_OPEN(apu_trace, "apu_trace.log", "wb")
	}
	else
	{
		printf("APU tracing disabled.\n");
		fclose(apu_trace);
		apu_trace = NULL;
	}
}

bool SNES_SPC::debug_is_enabled( void ) { return debug_trace; }

void SNES_SPC::debug_do_trace( int a, int x, int y, uint8_t const *pc, uint8_t *sp, int psw, int c, int nz, int dp )
{
	char	msg[512];

	ENSURE_TRACE_OPEN(apu_trace, "apu_trace.log", "a")

	debug_op_print(msg, a, x, y, pc, sp, psw, c, nz, dp);
	fprintf(apu_trace, "%s  ", msg);
	debug_io_print(msg);
	fprintf(apu_trace, "%s  ", msg);
	S9xPrintHVPosition(msg);
	fprintf(apu_trace, "%s\n", msg);
}

void SNES_SPC::debug_op_print( char *buffer, int a, int x, int y, uint8_t const *pc, uint8_t *sp, int psw, int c, int nz, int dp )
{
	static char	mnemonics[256][20] =
	{
		"NOP",
		"TCALL 0",
		"SET1 $%02X.0",
		"BBS $%02X.0,$%04X",
		"OR A,$%02X",
		"OR A,!$%04X",
		"OR A,(X)",
		"OR A,[$%02X+X]",
		"OR A,#$%02X",
		"OR $%02X,$%02X",
		"OR1 C,$%04X.%d",
		"ASL $%02X",
		"MOV !$%04X,Y",
		"PUSH PSW",
		"TSET1 !$%04X",
		"BRK",
		"BPL $%04X",
		"TCALL 1",
		"CLR1 $%02X.0",
		"BBC $%02X.0,$%04X",
		"OR A,$%02X+X",
		"OR A,!$%04X+X",
		"OR A,!$%04X+Y",
		"OR A,[$%02X]+Y",
		"OR $%02X,#$%02X",
		"OR (X),(Y)",
		"DECW $%02X",
		"ASL $%02X+X",
		"ASL A",
		"DEC X",
		"CMP X,!$%04X",
		"JMP [!$%04X+X]",
		"CLRP",
		"TCALL 2",
		"SET1 $%02X.1",
		"BBS $%02X.1,$%04X",
		"AND A,$%02X",
		"AND A,!$%04X",
		"AND A,(X)",
		"AND A,[$%02X+X]",
		"AND A,#$%02X",
		"AND $%02X,$%02X",
		"OR1 C,/$%04X.%d",
		"ROL $%02X",
		"ROL !$%04X",
		"PUSH A",
		"CBNE $%02X,$%04X",
		"BRA $%04X",
		"BMI $%04X",
		"TCALL 3",
		"CLR1 $%02X.1",
		"BBC $%02X.1,$%04X",
		"AND A,$%02X+X",
		"AND A,!$%04X+X",
		"AND A,!$%04X+Y",
		"AND A,[$%02X]+Y",
		"AND $%02X,#$%02X",
		"AND (X),(Y)",
		"INCW $%02X",
		"ROL $%02X+X",
		"ROL A",
		"INC X",
		"CMP X,$%02X",
		"CALL !$%04X",
		"SETP",
		"TCALL 4",
		"SET1 $%02X.2",
		"BBS $%02X.2,$%04X",
		"EOR A,$%02X",
		"EOR A,!$%04X",
		"EOR A,(X)",
		"EOR A,[$%02X+X]",
		"EOR A,#$%02X",
		"EOR $%02X,$%02X",
		"AND1 C,$%04X.%d",
		"LSR $%02X",
		"LSR !$%04X",
		"PUSH X",
		"TCLR1 !$%04X",
		"PCALL $%02X",
		"BVC $%04X",
		"TCALL 5",
		"CLR1 $%02X.2",
		"BBC $%02X.2,$%04X",
		"EOR A,$%02X+X",
		"EOR A,!$%04X+X",
		"EOR A,!$%04X+Y",
		"EOR A,[$%02X]+Y",
		"EOR $%02X,#$%02X",
		"EOR (X),(Y)",
		"CMPW YA,$%02X",
		"LSR $%02X+X",
		"LSR A",
		"MOV X,A",
		"CMP Y,!$%04X",
		"JMP !$%04X",
		"CLRC",
		"TCALL 6",
		"SET1 $%02X.3",
		"BBS $%02X.3,$%04X",
		"CMP A,$%02X",
		"CMP A,!$%04X",
		"CMP A,(X)",
		"CMP A,[$%02X+X]",
		"CMP A,#$%02X",
		"CMP $%02X,$%02X",
		"AND1 C,/$%04X.%d",
		"ROR $%02X",
		"ROR !$%04X",
		"PUSH Y",
		"DBNZ $%02X,$%04X",
		"RET",
		"BVS $%04X",
		"TCALL 7",
		"CLR1 $%02X.3",
		"BBC $%02X.3,$%04X",
		"CMP A,$%02X+X",
		"CMP A,!$%04X+X",
		"CMP A,!$%04X+Y",
		"CMP A,[$%02X]+Y",
		"CMP $%02X,#$%02X",
		"CMP (X),(Y)",
		"ADDW YA,$%02X",
		"ROR $%02X+X",
		"ROR A",
		"MOV A,X",
		"CMP Y,$%02X",
		"RET1",
		"SETC",
		"TCALL 8",
		"SET1 $%02X.4",
		"BBS $%02X.4,$%04X",
		"ADC A,$%02X",
		"ADC A,!$%04X",
		"ADC A,(X)",
		"ADC A,[$%02X+X]",
		"ADC A,#$%02X",
		"ADC $%02X,$%02X",
		"EOR1 C,$%04X.%d",
		"DEC $%02X",
		"DEC !$%04X",
		"MOV Y,#$%02X",
		"POP PSW",
		"MOV $%02X,#$%02X",
		"BCC $%04X",
		"TCALL 9",
		"CLR1 $%02X.4",
		"BBC $%02X.4,$%04X",
		"ADC A,$%02X+X",
		"ADC A,!$%04X+X",
		"ADC A,!$%04X+Y",
		"ADC A,[$%02X]+Y",
		"ADC $%02X,#$%02X",
		"ADC (X),(Y)",
		"SUBW YA,$%02X",
		"DEC $%02X+X",
		"DEC A",
		"MOV X,SP",
		"DIV YA,X",
		"XCN A",
		"EI",
		"TCALL 10",
		"SET1 $%02X.5",
		"BBS $%02X.5,$%04X",
		"SBC A,$%02X",
		"SBC A,!$%04X",
		"SBC A,(X)",
		"SBC A,[$%02X+X]",
		"SBC A,#$%02X",
		"SBC $%02X,$%02X",
		"MOV1 C,$%04X.%d",
		"INC $%02X",
		"INC !$%04X",
		"CMP Y,#$%02X",
		"POP A",
		"MOV (X)+,A",
		"BCS $%04X",
		"TCALL 11",
		"CLR1 $%02X.5",
		"BBC $%02X.5,$%04X",
		"SBC A,$%02X+X",
		"SBC A,!$%04X+X",
		"SBC A,!$%04X+Y",
		"SBC A,[$%02X]+Y",
		"SBC $%02X,#$%02X",
		"SBC (X),(Y)",
		"MOVW YA,$%02X",
		"INC $%02X+X",
		"INC A",
		"MOV SP,X",
		"DAS A",
		"MOV A,(X)+",
		"DI",
		"TCALL 12",
		"SET1 $%02X.6",
		"BBS $%02X.6,$%04X",
		"MOV $%02X,A",
		"MOV !$%04X,A",
		"MOV (X),A",
		"MOV [$%02X+X],A",
		"CMP X,#$%02X",
		"MOV !$%04X,X",
		"MOV1 $%04X.%d,C",
		"MOV $%02X,Y",
		"ASL !$%04X",
		"MOV X,#$%02X",
		"POP X",
		"MUL YA",
		"BNE $%04X",
		"TCALL 13",
		"CLR1 $%02X.6",
		"BBC $%02X.6,$%04X",
		"MOV $%02X+X,A",
		"MOV !$%04X+X,A",
		"MOV !$%04X+Y,A",
		"MOV [$%02X]+Y,A",
		"MOV $%02X,X",
		"MOV $%02X+Y,X",
		"MOVW $%02X,YA",
		"MOV $%02X+X,Y",
		"DEC Y",
		"MOV A,Y",
		"CBNE $%02X+X,$%04X",
		"DAA A",
		"CLRV",
		"TCALL 14",
		"SET1 $%02X.7",
		"BBS $%02X.7,$%04X",
		"MOV A,$%02X",
		"MOV A,!$%04X",
		"MOV A,(X)",
		"MOV A,[$%02X+X]",
		"MOV A,#$%02X",
		"MOV X,!$%04X",
		"NOT1 $%04X.%d",
		"MOV Y,$%02X",
		"MOV Y,!$%04X",
		"NOTC",
		"POP Y",
		"SLEEP",
		"BEQ $%04X",
		"TCALL 15",
		"CLR1 $%02X.7",
		"BBC $%02X.7,$%04X",
		"MOV A,$%02X+X",
		"MOV A,!$%04X+X",
		"MOV A,!$%04X+Y",
		"MOV A,[$%02X]+Y",
		"MOV X,$%02X",
		"MOV X,$%02X+Y",
		"MOV $%02X,$%02X",
		"MOV Y,$%02X+X",
		"INC Y",
		"MOV Y,A",
		"DBNZ Y,$%04X",
		"STOP"
	};

	static int modes[256] =
	{
		2, 2, 0, 5, 0, 1, 2, 0, 0, 3, 6, 0, 1, 2, 1, 2,
		7, 2, 0, 5, 0, 1, 1, 0, 4, 2, 0, 0, 2, 2, 1, 1,
		2, 2, 0, 5, 0, 1, 2, 0, 0, 3, 6, 0, 1, 2, 5, 7,
		7, 2, 0, 5, 0, 1, 1, 0, 4, 2, 0, 0, 2, 2, 0, 1,
		2, 2, 0, 5, 0, 1, 2, 0, 0, 3, 6, 0, 1, 2, 1, 0,
		7, 2, 0, 5, 0, 1, 1, 0, 4, 2, 0, 0, 2, 2, 1, 1,
		2, 2, 0, 5, 0, 1, 2, 0, 0, 3, 6, 0, 1, 2, 5, 2,
		7, 2, 0, 5, 0, 1, 1, 0, 4, 2, 0, 0, 2, 2, 0, 2,
		2, 2, 0, 5, 0, 1, 2, 0, 0, 3, 6, 0, 1, 0, 2, 4,
		7, 2, 0, 5, 0, 1, 1, 0, 4, 2, 0, 0, 2, 2, 2, 2,
		2, 2, 0, 5, 0, 1, 2, 0, 0, 3, 6, 0, 1, 0, 2, 2,
		7, 2, 0, 5, 0, 1, 1, 0, 4, 2, 0, 0, 2, 2, 2, 2,
		2, 2, 0, 5, 0, 1, 2, 0, 0, 1, 6, 0, 1, 0, 2, 2,
		7, 2, 0, 5, 0, 1, 1, 0, 0, 0, 0, 0, 2, 2, 5, 2,
		2, 2, 0, 5, 0, 1, 2, 0, 0, 1, 6, 0, 1, 2, 2, 2,
		7, 2, 0, 5, 0, 1, 1, 0, 0, 0, 3, 0, 2, 2, 7, 2
	};

	static int modesToBytes[] =
	{
		2, 3, 1, 3, 3, 3, 3, 2
	};

	int const	n80 = 0x80; // nz
	int const	p20 = 0x20; // dp
	int const	z02 = 0x02; // nz
	int const	c01 = 0x01; // c

	#define GET_PC()	(pc - ram)
	#define GET_SP()	(sp - 0x101 - ram)
	#define GET_PSW( out )\
	{\
		out = psw & ~(n80 | p20 | z02 | c01);\
		out |= c  >> 8 & c01;\
		out |= dp >> 3 & p20;\
		out |= ((nz >> 4) | nz) & n80;\
		if ( !(uint8_t) nz ) out |= z02;\
	}

	uint8_t const	*ram = RAM;

	int		addr;
	int		tsp, tpsw;
	uint8_t	d0, d1, d2;

	addr = GET_PC();
	tsp  = GET_SP();
	GET_PSW(tpsw);

	d0 = *pc;
	d1 = (addr < 0xffff) ? *(pc + 1) : 0;
	d2 = (addr < 0xfffe) ? *(pc + 2) : 0;

	int		mode  = modes[d0];
	int		bytes = modesToBytes[mode];
	char	mnem[100];

	switch (bytes)
	{
		case 1:
			sprintf(buffer, "%04X %02X       ",     addr, d0);
			break;

		case 2:
			sprintf(buffer, "%04X %02X %02X    ",   addr, d0, d1);
			break;

		case 3:
			sprintf(buffer, "%04X %02X %02X %02X ", addr, d0, d1, d2);
			break;
	}

	switch (mode)
	{
		case 0:
			sprintf(mnem, mnemonics[d0], d1);
			break;

		case 1:
			sprintf(mnem, mnemonics[d0], d1 + (d2 << 8));
			break;

		case 2:
			strcpy (mnem, mnemonics[d0]);
			break;

		case 3:
			sprintf(mnem, mnemonics[d0], d2, d1);
			break;

		case 4:
			sprintf(mnem, mnemonics[d0], d2, d1);
			break;

		case 5:
			sprintf(mnem, mnemonics[d0], d1, addr + 3 + (int8_t) d2);
			break;

		case 6:
			sprintf(mnem, mnemonics[d0], (d1 + (d2 << 8)) & 0x1fff, d2 >> 5);
			break;

		case 7:
			sprintf(mnem, mnemonics[d0], addr + 2 + (int8_t) d1);
			break;
	}

	sprintf(buffer, "%s %-20s A:%02X X:%02X Y:%02X S:%02X P:%c%c%c%c%c%c%c%c ROM:%d",
		buffer, mnem, a, x, y, tsp,
		(tpsw & 0x80) ? 'N' : 'n',
		(tpsw & 0x40) ? 'V' : 'v',
		(tpsw & 0x20) ? 'P' : 'p',
		(tpsw & 0x10) ? 'B' : 'b',
		(tpsw & 0x08) ? 'H' : 'h',
		(tpsw & 0x04) ? 'I' : 'i',
		(tpsw & 0x02) ? 'Z' : 'z',
		(tpsw & 0x01) ? 'C' : 'c',
		m.rom_enabled ? 1 : 0);
}

void SNES_SPC::debug_io_print( char *buffer )
{
	sprintf(buffer, "i/o %02X/%02X %02X/%02X %02X/%02X %02X/%02X",
		m.smp_regs[1][r_cpuio0], m.smp_regs[0][r_cpuio0],
		m.smp_regs[1][r_cpuio1], m.smp_regs[0][r_cpuio1],
		m.smp_regs[1][r_cpuio2], m.smp_regs[0][r_cpuio2],
		m.smp_regs[1][r_cpuio3], m.smp_regs[0][r_cpuio3]);
}

#endif
