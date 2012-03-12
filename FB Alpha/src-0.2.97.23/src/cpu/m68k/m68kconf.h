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
#define M68K_EMULATE_010            OPT_ON
#define M68K_EMULATE_EC020          OPT_ON
#define M68K_EMULATE_020            OPT_OFF


/* If ON, the CPU will call m68k_read_immediate_xx() for immediate addressing
 * and m68k_read_pcrelative_xx() for PC-relative addressing.
 * If off, all read requests from the CPU will be redirected to m68k_read_xx()
 */
#define M68K_SEPARATE_READS         OPT_ON

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
#define M68K_INT_ACK_CALLBACK(A)    M68KIRQAcknowledge(A)


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
#define M68K_EMULATE_RESET          OPT_SPECIFY_HANDLER
#define M68K_RESET_CALLBACK()       M68KResetCallback()


/* If ON, CPU will call the callback when it encounters a cmpi.l #v, dn
 * instruction.
 */
#define M68K_CMPILD_HAS_CALLBACK     OPT_SPECIFY_HANDLER
//#define M68K_CMPILD_CALLBACK(v,r)    your_cmpild_handler_function(v,r)
#define M68K_CMPILD_CALLBACK(v, r)   M68KcmpildCallback(v, r)

/* If ON, CPU will call the callback when it encounters a rte
 * instruction.
 */
#define M68K_RTE_HAS_CALLBACK       OPT_SPECIFY_HANDLER
//#define M68K_RTE_CALLBACK()         your_rte_handler_function()
#define M68K_RTE_CALLBACK()	    M68KRTECallback()


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
#ifdef FBA_DEBUG
 #define M68K_INSTRUCTION_HOOK       OPT_ON
#else
 #define M68K_INSTRUCTION_HOOK       OPT_OFF
#endif
#define M68K_INSTRUCTION_CALLBACK() your_instruction_hook_function()


/* If ON, the CPU will emulate the 4-byte prefetch queue of a real 68000 */
#define M68K_EMULATE_PREFETCH       OPT_ON


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
#define M68K_USE_64_BIT  OPT_OFF


/* Set to your compiler's static inline keyword to enable it, or
 * set it to blank to disable it.
 * If you define INLINE in the makefile, it will override this value.
 * NOTE: not enabling inline functions will SEVERELY slow down emulation.
 */
#ifndef INLINE
#define INLINE static __inline__
#endif /* INLINE */

/* --------------------------- FB Alpha handlers -------------------------- */

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef FASTCALL
 #undef __fastcall
 #define __fastcall
#endif

/* Reset callback */
void M68KResetCallback(void);
int M68KIRQAcknowledge(int nIRQ);
void M68KRTECallback(void);
void M68KcmpildCallback(unsigned int val, int reg);

unsigned int __fastcall M68KFetchByte(unsigned int a);
unsigned int __fastcall M68KFetchWord(unsigned int a);
unsigned int __fastcall M68KFetchLong(unsigned int a);

extern unsigned int (*SekDbgFetchByteDisassembler)(unsigned int);
extern unsigned int (*SekDbgFetchWordDisassembler)(unsigned int);
extern unsigned int (*SekDbgFetchLongDisassembler)(unsigned int);

#if defined FBA_DEBUG
extern unsigned int (__fastcall *M68KReadByteDebug)(unsigned int);
extern unsigned int (__fastcall *M68KReadWordDebug)(unsigned int);
extern unsigned int (__fastcall *M68KReadLongDebug)(unsigned int);

extern void (__fastcall *M68KWriteByteDebug)(unsigned int, unsigned int);
extern void (__fastcall *M68KWriteWordDebug)(unsigned int, unsigned int);
extern void (__fastcall *M68KWriteLongDebug)(unsigned int, unsigned int);
#else
unsigned int __fastcall M68KReadByte(unsigned int a);
unsigned int __fastcall M68KReadWord(unsigned int a);
unsigned int __fastcall M68KReadLong(unsigned int a);

void __fastcall M68KWriteByte(unsigned int a, unsigned int d);
void __fastcall M68KWriteWord(unsigned int a, unsigned int d);
void __fastcall M68KWriteLong(unsigned int a, unsigned int d);
#endif

#ifdef __cplusplus
 }
#endif

#define m68ki_remaining_cycles m68k_ICount

/* Read data relative to the PC */
#define m68k_read_pcrelative_8(address) M68KFetchByte(address)
#define m68k_read_pcrelative_16(address) M68KFetchWord(address)
#define m68k_read_pcrelative_32(address) M68KFetchLong(address)

/* Read data immediately following the PC */
#define m68k_read_immediate_16(address) M68KFetchWord(address)
#define m68k_read_immediate_32(address) M68KFetchLong(address)

/* Memory access for the disassembler */
#define m68k_read_disassembler_8(address) SekDbgFetchByteDisassembler(address)
#define m68k_read_disassembler_16(address) SekDbgFetchWordDisassembler(address)
#define m68k_read_disassembler_32(address) SekDbgFetchLongDisassembler(address)

#if defined FBA_DEBUG
/* Read from anywhere */
#define m68k_read_memory_8(address) M68KReadByteDebug(address)
#define m68k_read_memory_16(address) M68KReadWordDebug(address)
#define m68k_read_memory_32(address) M68KReadLongDebug(address)

/* Write to anywhere */
#define m68k_write_memory_8(address, value) M68KWriteByteDebug(address, value)
#define m68k_write_memory_16(address, value) M68KWriteWordDebug(address, value)
#define m68k_write_memory_32(address, value) M68KWriteLongDebug(address, value)
#else
/* Read from anywhere */
#define m68k_read_memory_8(address) M68KReadByte(address)
#define m68k_read_memory_16(address) M68KReadWord(address)
#define m68k_read_memory_32(address) M68KReadLong(address)

/* Write to anywhere */
#define m68k_write_memory_8(address, value) M68KWriteByte(address, value)
#define m68k_write_memory_16(address, value) M68KWriteWord(address, value)
#define m68k_write_memory_32(address, value) M68KWriteLong(address, value)
#endif /* FBA_DEBUG */


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* M68KCONF__HEADER */
