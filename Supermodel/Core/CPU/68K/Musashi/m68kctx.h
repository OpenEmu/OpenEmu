/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson 
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * m68kctx.h
 *
 * Musashi CPU context. This was made a separate file to more easily facilitate
 * save state management. It is used internally by m68kcpu.h. 
 */
 
#include "Types.h"	// Supermodel types

typedef struct
{
	UINT32 cpu_type;     /* CPU Type: 68000, 68008, 68010, 68EC020, or 68020 */
	UINT32 dar[16];      /* Data and Address Registers */
	UINT32 ppc;		   	 /* Previous program counter */
	UINT32 pc;           /* Program Counter */
	UINT32 sp[7];        /* User, Interrupt, and Master Stack Pointers */
	UINT32 vbr;          /* Vector Base Register (m68010+) */
	UINT32 sfc;          /* Source Function Code Register (m68010+) */
	UINT32 dfc;          /* Destination Function Code Register (m68010+) */
	UINT32 cacr;         /* Cache Control Register (m68020, unemulated) */
	UINT32 caar;         /* Cache Address Register (m68020, unemulated) */
	UINT32 ir;           /* Instruction Register */
	UINT32 t1_flag;      /* Trace 1 */
	UINT32 t0_flag;      /* Trace 0 */
	UINT32 s_flag;       /* Supervisor */
	UINT32 m_flag;       /* Master/Interrupt state */
	UINT32 x_flag;       /* Extend */
	UINT32 n_flag;       /* Negative */
	UINT32 not_z_flag;   /* Zero, inverted for speedups */
	UINT32 v_flag;       /* Overflow */
	UINT32 c_flag;       /* Carry */
	UINT32 int_mask;     /* I0-I2 */
	UINT32 int_level;    /* State of interrupt pins IPL0-IPL2 -- ASG: changed from ints_pending */
	UINT32 int_cycles;   /* ASG: extra cycles from generated interrupts */
	UINT32 stopped;      /* Stopped state */
	UINT32 pref_addr;    /* Last prefetch address */
	UINT32 pref_data;    /* Data in the prefetch queue */
	UINT32 address_mask; /* Available address pins */
	UINT32 sr_mask;      /* Implemented status register bits */
	UINT32 instr_mode;   /* Stores whether we are in instruction mode or group 0/1 exception mode */
	UINT32 run_mode;     /* Stores whether we are processing a reset, bus error, address error, or something else */

	/* Clocks required for instructions / exceptions */
	UINT32 cyc_bcc_notake_b;
	UINT32 cyc_bcc_notake_w;
	UINT32 cyc_dbcc_f_noexp;
	UINT32 cyc_dbcc_f_exp;
	UINT32 cyc_scc_r_true;
	UINT32 cyc_movem_w;
	UINT32 cyc_movem_l;
	UINT32 cyc_shift;
	UINT32 cyc_reset;
	UINT8* cyc_instruction;
	UINT8* cyc_exception;

	/* Callbacks to host */
	int  (*int_ack_callback)(int int_line);           /* Interrupt Acknowledge */
	void (*bkpt_ack_callback)(unsigned int data);     /* Breakpoint Acknowledge */
	void (*reset_instr_callback)(void);               /* Called when a RESET instruction is encountered */
	void (*cmpild_instr_callback)(unsigned int, int); /* Called when a CMPI.L #v, Dn instruction is encountered */
	void (*rte_instr_callback)(void);                 /* Called when a RTE instruction is encountered */
	void (*pc_changed_callback)(unsigned int new_pc); /* Called when the PC changes by a large amount */
	void (*set_fc_callback)(unsigned int new_fc);     /* Called when the CPU function code changes */
	void (*instr_hook_callback)(void);                /* Called every instruction cycle prior to execution */

} m68ki_cpu_core;