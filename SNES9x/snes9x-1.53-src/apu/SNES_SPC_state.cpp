// SPC emulation state save/load: copy_state(), save_spc()
// Separate file to avoid linking in unless needed

// snes_spc 0.9.0. http://www.slack.net/‾ant/

#include "SNES_SPC.h"

#if !SPC_NO_COPY_STATE_FUNCS

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

#include <stdio.h>
#include "blargg_source.h"

#define RAM         (m.ram.ram)
#define REGS        (m.smp_regs [0])
#define REGS_IN     (m.smp_regs [1])

void SNES_SPC::save_regs( uint8_t out [reg_count] )
{
	// Use current timer counter values
	for ( int i = 0; i < timer_count; i++ )
			out [r_t0out + i] = m.timers [i].counter;

	// Last written values
	memcpy( out, REGS, r_t0out );
}

void SNES_SPC::init_header( void* spc_out )
{
	spc_file_t* const spc = (spc_file_t*) spc_out;
	
	spc->has_id666 = 26; // has none
	spc->version   = 30;
	memcpy( spc, signature, sizeof spc->signature );
	memset( spc->text, 0, sizeof spc->text );
}

void SNES_SPC::save_spc( void* spc_out )
{
	spc_file_t* const spc = (spc_file_t*) spc_out;
	
	// CPU
	spc->pcl = (uint8_t) (m.cpu_regs.pc >> 0);
	spc->pch = (uint8_t) (m.cpu_regs.pc >> 8);
	spc->a   = m.cpu_regs.a;
	spc->x   = m.cpu_regs.x;
	spc->y   = m.cpu_regs.y;
	spc->psw = m.cpu_regs.psw;
	spc->sp  = m.cpu_regs.sp;
	
	// RAM, ROM
	memcpy( spc->ram, RAM, sizeof spc->ram );
	if ( m.rom_enabled )
		memcpy( spc->ram + rom_addr, m.hi_ram, sizeof m.hi_ram );
	memset( spc->unused, 0, sizeof spc->unused );
	memcpy( spc->ipl_rom, m.rom, sizeof spc->ipl_rom );
	
	// SMP registers
	save_regs( &spc->ram [0xF0] );
	int i;
	for ( i = 0; i < port_count; i++ )
		spc->ram [0xF0 + r_cpuio0 + i] = REGS_IN [r_cpuio0 + i];
	
	// DSP registers
	for ( i = 0; i < SPC_DSP::register_count; i++ )
		spc->dsp [i] = dsp.read( i );
}

#undef IF_0_THEN_256
#define IF_0_THEN_256( n ) ((uint8_t) ((n) - 1) + 1)
void SNES_SPC::copy_state( unsigned char** io, copy_func_t copy )
{
	SPC_State_Copier copier( io, copy );
	
	// Make state data more readable by putting 64K RAM, 16 SMP registers,
	// then DSP (with its 128 registers) first

	// RAM
	enable_rom( 0 ); // will get re-enabled if necessary in regs_loaded() below
	copier.copy( RAM, 0x10000 );
	
	{
		// SMP registers
		uint8_t regs [reg_count];
		uint8_t regs_in [reg_count];

		memcpy( regs, REGS, reg_count );
		memcpy( regs_in, REGS_IN, reg_count );

		copier.copy( regs, sizeof regs );
		copier.copy( regs_in, sizeof regs_in );

		memcpy( REGS, regs, reg_count);
		memcpy( REGS_IN, regs_in, reg_count );

		enable_rom( REGS [r_control] & 0x80 );
	}
	
	// CPU registers
	SPC_COPY( uint16_t, m.cpu_regs.pc );
	SPC_COPY(  uint8_t, m.cpu_regs.a );
	SPC_COPY(  uint8_t, m.cpu_regs.x );
	SPC_COPY(  uint8_t, m.cpu_regs.y );
	SPC_COPY(  uint8_t, m.cpu_regs.psw );
	SPC_COPY(  uint8_t, m.cpu_regs.sp );
	copier.extra();
	
	SPC_COPY( int16_t, m.spc_time );
	SPC_COPY( int16_t, m.dsp_time );

	// DSP
	dsp.copy_state( io, copy );
	
	// Timers
	for ( int i = 0; i < timer_count; i++ )
	{
		Timer* t = &m.timers [i];
		t->period  = IF_0_THEN_256( REGS [r_t0target + i] );
		t->enabled = REGS [r_control] >> i & 1;
		SPC_COPY( int16_t, t->next_time );
		SPC_COPY( uint8_t, t->divider );
		SPC_COPY( uint8_t, t->counter );
		copier.extra();
	}

	set_tempo( m.tempo );

	copier.extra();
}
#endif
