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
 * m68kconf.h
 *
 * Musashi configuration. 
 *
 * Permission was obtained from Karl Stenerud to apply the GPL license to this
 * code.
 */

/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */
/*
 *                                  MUSASHI
 *                                Version 3.3
 *
 * A portable Motorola M680x0 processor emulation engine.
 * Copyright 1998-2001 Karl Stenerud.  All rights reserved.
 *
 * This code may be freely used for non-commercial purposes as long as this
 * copyright notice remains unaltered in the source code and any binary files
 * containing this code in compiled form.
 *
 * All other lisencing terms must be negotiated with the author
 * (Karl Stenerud).
 *
 * The latest version of this code can be obtained at:
 * http://kstenerud.cjb.net
 */



#ifndef M68KCONF__HEADER
#define M68KCONF__HEADER

#define M68K_COMPILE_FOR_MAME OPT_OFF

/* Configuration switches.
 * Use OPT_SPECIFY_HANDLER for configuration options that allow callbacks.
 * OPT_SPECIFY_HANDLER causes the core to link directly to the function
 * or macro you specify, rather than using callback functions whose pointer
 * must be passed in using m68k_set_xxx_callback().
 */
#define OPT_OFF             0
#define OPT_ON              1
#define OPT_SPECIFY_HANDLER 2


/* ======================================================================== */
/* ============================= CONFIGURATION ============================ */
/* ======================================================================== */

/* Turn ON if you want to use the following M68K variants */
#define M68K_EMULATE_008            OPT_OFF
#define M68K_EMULATE_010            OPT_OFF
#define M68K_EMULATE_EC020          OPT_OFF
#define M68K_EMULATE_020            OPT_OFF


/* If ON, the CPU will call m68k_read_immediate_xx() for immediate addressing
 * and m68k_read_pcrelative_xx() for PC-relative addressing.
 * If off, all read requests from the CPU will be redirected to m68k_read_xx()
 */
#define M68K_SEPARATE_READS         OPT_ON	// fixes Supermodel compilation on MacOS

/* If ON, the CPU will call m68k_write_32_pd() when it executes move.l with a
 * predecrement destination EA mode instead of m68k_write_32().
 * To simulate real 68k behavior, m68k_write_32_pd() must first write the high
 * word to [address+2], and then write the low word to [address].
 */
#define M68K_SIMULATE_PD_WRITES     OPT_OFF

/* If ON, CPU will call the interrupt acknowledge callback when it services an
 * interrupt.
 * If off, all interrupts will be autovectored and all interrupt requests will
 * auto-clear when the interrupt is serviced.
 */
#define M68K_EMULATE_INT_ACK        OPT_SPECIFY_HANDLER
#define M68K_INT_ACK_CALLBACK(A)    M68KIRQCallback(A)


/* If ON, CPU will call the breakpoint acknowledge callback when it encounters
 * a breakpoint instruction and it is running a 68010+.
 */
#define M68K_EMULATE_BKPT_ACK       OPT_OFF
#define M68K_BKPT_ACK_CALLBACK()    your_bkpt_ack_handler_function()


/* If ON, the CPU will monitor the trace flags and take trace exceptions
 */
#define M68K_EMULATE_TRACE          OPT_OFF


/* If ON, CPU will call the output reset callback when it encounters a reset
 * instruction.
 */
#define M68K_EMULATE_RESET          OPT_OFF
#define M68K_RESET_CALLBACK()       M68KResetCallback()


/* If ON, CPU will call the callback when it encounters a cmpi.l #v, dn
 * instruction.
 */
#define M68K_CMPILD_HAS_CALLBACK     OPT_OFF
#define M68K_CMPILD_CALLBACK(v,r)    your_cmpild_handler_function(v,r)

/* If ON, CPU will call the callback when it encounters a rte
 * instruction.
 */
#define M68K_RTE_HAS_CALLBACK       OPT_OFF
#define M68K_RTE_CALLBACK()         your_rte_handler_function()


/* If ON, CPU will call the set fc callback on every memory access to
 * differentiate between user/supervisor, program/data access like a real
 * 68000 would.  This should be enabled and the callback should be set if you
 * want to properly emulate the m68010 or higher. (moves uses function codes
 * to read/write data from different address spaces)
 */
#define M68K_EMULATE_FC             OPT_OFF
#define M68K_SET_FC_CALLBACK(A)     your_set_fc_handler_function(A)


/* If ON, CPU will call the pc changed callback when it changes the PC by a
 * large value.  This allows host programs to be nicer when it comes to
 * fetching immediate data and instructions on a banked memory system.
 */
#define M68K_MONITOR_PC             OPT_OFF
#define M68K_SET_PC_CALLBACK(A)     your_pc_changed_handler_function(A)


/* If ON, CPU will call the instruction hook callback before every
 * instruction.
 */
#ifdef SUPERMODEL_DEBUGGER
#define M68K_INSTRUCTION_HOOK       OPT_SPECIFY_HANDLER
#define M68K_INSTRUCTION_CALLBACK() M68KDebugCallback()
#else
#define M68K_INSTRUCTION_HOOK       OPT_OFF
#define M68K_INSTRUCTION_CALLBACK() your_instruction_hook_function()
#endif // SUPERMODEL_DEBUGGER

/* If ON, the CPU will emulate the 4-byte prefetch queue of a real 68000 */
#define M68K_EMULATE_PREFETCH       OPT_OFF


/* If ON, the CPU will generate address error exceptions if it tries to
 * access a word or longword at an odd address.
 * NOTE: This is only emulated properly for 68000 mode.
 */
#define M68K_EMULATE_ADDRESS_ERROR  OPT_OFF


/* Turn ON to enable logging of illegal instruction calls.
 * M68K_LOG_FILEHANDLE must be #defined to a stdio file stream.
 * Turn on M68K_LOG_1010_1111 to log all 1010 and 1111 calls.
 */
#define M68K_LOG_ENABLE             OPT_OFF
#define M68K_LOG_1010_1111          OPT_OFF
#define M68K_LOG_FILEHANDLE         some_file_handle


/* ----------------------------- COMPATIBILITY ---------------------------- */

/* The following options set optimizations that violate the current ANSI
 * standard, but will be compliant under the forthcoming C9X standard.
 */


/* If ON, the enulation core will use 64-bit integers to speed up some
 * operations.
*/
#define M68K_USE_64_BIT  OPT_ON


/* Set to your compiler's static inline keyword to enable it, or
 * set it to blank to disable it.
 * If you define INLINE in the makefile, it will override this value.
 * NOTE: not enabling inline functions will SEVERELY slow down emulation.
 */
#ifndef INLINE
#define INLINE static __inline__	// defined for GCC; if using MSVC, pass INLINE as "static __inline" from Makefile
#endif /* INLINE */

/******************************************************************************
 Supermodel Interface
******************************************************************************/

// Supermodel 68K interface (these functions defined in CPU/68K.cpp)
//#ifndef FASTCALL (this doesn't work for now (needs to be added to the prototypes in m68k.h for m68k_read_memory*)
	#undef FASTCALL
	#define FASTCALL
//#endif
unsigned int FASTCALL M68KFetch8(unsigned int a);
unsigned int FASTCALL M68KFetch16(unsigned int a);
unsigned int FASTCALL M68KFetch32(unsigned int a);
unsigned int FASTCALL M68KRead8(unsigned int a);
unsigned int FASTCALL M68KRead16(unsigned int a);
unsigned int FASTCALL M68KRead32(unsigned int a);
void FASTCALL M68KWrite8(unsigned int a, unsigned int d);
void FASTCALL M68KWrite16(unsigned int a, unsigned int d);
void FASTCALL M68KWrite32(unsigned int a, unsigned int d);

/* Read data relative to the PC */
#define m68k_read_pcrelative_8(address) M68KFetch8(address)
#define m68k_read_pcrelative_16(address) M68KFetch16(address)
#define m68k_read_pcrelative_32(address) M68KFetch32(address)

/* Read data immediately following the PC */
#define m68k_read_immediate_16(address) M68KFetch16(address)
#define m68k_read_immediate_32(address) M68KFetch32(address)

/* Memory access for the disassembler */
#define m68k_read_disassembler_8(address) M68KRead8(address)
#define m68k_read_disassembler_16(address) M68KRead16(address)
#define m68k_read_disassembler_32(address) M68KRead32(address)

/* Read from anywhere */
#define m68k_read_memory_8(address) M68KRead8(address)
#define m68k_read_memory_16(address) M68KRead16(address)
#define m68k_read_memory_32(address) M68KRead32(address)

/* Write to anywhere */
#define m68k_write_memory_8(address, value) M68KWrite8(address, value)
#define m68k_write_memory_16(address, value) M68KWrite16(address, value)
#define m68k_write_memory_32(address, value) M68KWrite32(address, value)


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* M68KCONF__HEADER */
