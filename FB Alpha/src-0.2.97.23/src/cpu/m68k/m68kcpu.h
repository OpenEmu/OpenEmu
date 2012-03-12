#include <stdio.h>
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




#ifndef M68KCPU__HEADER
#define M68KCPU__HEADER

#include "m68k.h"
#include <limits.h>

#if M68K_EMULATE_ADDRESS_ERROR
#include <setjmp.h>
#endif /* M68K_EMULATE_ADDRESS_ERROR */

/* ======================================================================== */
/* ==================== ARCHITECTURE-DEPENDANT DEFINES ==================== */
/* ======================================================================== */

/* Check for > 32bit sizes */
#if UINT_MAX > 0xffffffff
	#define M68K_INT_GT_32_BIT  1
#else
	#define M68K_INT_GT_32_BIT  0
#endif

/* Data types used in this emulation core */
#undef sint8
#undef sint16
#undef sint32
#undef sint64
#undef uint8
#undef uint16
#undef uint32
#undef uint64
#undef sint
#undef uint

#define sint8  signed   char			/* ASG: changed from char to signed char */
#define sint16 signed   short
#define sint32 signed   long
#define uint8  unsigned char
#define uint16 unsigned short
#define uint32 unsigned long

/* signed and unsigned int must be at least 32 bits wide */
#define sint   signed   int
#define uint   unsigned int


#if M68K_USE_64_BIT
#define sint64 signed   long long
#define uint64 unsigned long long
#else
#define sint64 sint32
#define uint64 uint32
#endif /* M68K_USE_64_BIT */



/* Allow for architectures that don't have 8-bit sizes */
#if UCHAR_MAX == 0xff
	#define MAKE_INT_8(A) (sint8)(A)
#else
	#undef  sint8
	#define sint8  signed   int
	#undef  uint8
	#define uint8  unsigned int
	INLINE sint MAKE_INT_8(uint value)
	{
		return (value & 0x80) ? value | ~0xff : value & 0xff;
	}
#endif /* UCHAR_MAX == 0xff */


/* Allow for architectures that don't have 16-bit sizes */
#if USHRT_MAX == 0xffff
	#define MAKE_INT_16(A) (sint16)(A)
#else
	#undef  sint16
	#define sint16 signed   int
	#undef  uint16
	#define uint16 unsigned int
	INLINE sint MAKE_INT_16(uint value)
	{
		return (value & 0x8000) ? value | ~0xffff : value & 0xffff;
	}
#endif /* USHRT_MAX == 0xffff */


/* Allow for architectures that don't have 32-bit sizes */
#if ULONG_MAX == 0xffffffff
	#define MAKE_INT_32(A) (sint32)(A)
#else
	#undef  sint32
	#define sint32  signed   int
	#undef  uint32
	#define uint32  unsigned int
	INLINE sint MAKE_INT_32(uint value)
	{
		return (value & 0x80000000) ? value | ~0xffffffff : value & 0xffffffff;
	}
#endif /* ULONG_MAX == 0xffffffff */




/* ======================================================================== */
/* ============================ GENERAL DEFINES =========================== */
/* ======================================================================== */

/* Exception Vectors handled by emulation */
#define EXCEPTION_BUS_ERROR                2 /* This one is not emulated! */
#define EXCEPTION_ADDRESS_ERROR            3 /* This one is partially emulated (doesn't stack a proper frame yet) */
#define EXCEPTION_ILLEGAL_INSTRUCTION      4
#define EXCEPTION_ZERO_DIVIDE              5
#define EXCEPTION_CHK                      6
#define EXCEPTION_TRAPV                    7
#define EXCEPTION_PRIVILEGE_VIOLATION      8
#define EXCEPTION_TRACE                    9
#define EXCEPTION_1010                    10
#define EXCEPTION_1111                    11
#define EXCEPTION_FORMAT_ERROR            14
#define EXCEPTION_UNINITIALIZED_INTERRUPT 15
#define EXCEPTION_SPURIOUS_INTERRUPT      24
#define EXCEPTION_INTERRUPT_AUTOVECTOR    24
#define EXCEPTION_TRAP_BASE               32

/* Function codes set by CPU during data/address bus activity */
#define FUNCTION_CODE_USER_DATA          1
#define FUNCTION_CODE_USER_PROGRAM       2
#define FUNCTION_CODE_SUPERVISOR_DATA    5
#define FUNCTION_CODE_SUPERVISOR_PROGRAM 6
#define FUNCTION_CODE_CPU_SPACE          7

/* CPU types for deciding what to emulate */
#define CPU_TYPE_000   1
#define CPU_TYPE_008   2
#define CPU_TYPE_010   4
#define CPU_TYPE_EC020 8
#define CPU_TYPE_020   16

/* Different ways to stop the CPU */
#define STOP_LEVEL_STOP 1
#define STOP_LEVEL_HALT 2

/* Used for 68000 address error processing */
#define INSTRUCTION_YES 0
#define INSTRUCTION_NO  0x08
#define MODE_READ       0x10
#define MODE_WRITE      0

#define RUN_MODE_NORMAL          0
#define RUN_MODE_BERR_AERR_RESET 1

#ifndef NULL
#define NULL ((void*)0)
#endif

/* ======================================================================== */
/* ================================ MACROS ================================ */
/* ======================================================================== */


/* ---------------------------- General Macros ---------------------------- */

/* Bit Isolation Macros */
#define BIT_0(A)  ((A) & 0x00000001)
#define BIT_1(A)  ((A) & 0x00000002)
#define BIT_2(A)  ((A) & 0x00000004)
#define BIT_3(A)  ((A) & 0x00000008)
#define BIT_4(A)  ((A) & 0x00000010)
#define BIT_5(A)  ((A) & 0x00000020)
#define BIT_6(A)  ((A) & 0x00000040)
#define BIT_7(A)  ((A) & 0x00000080)
#define BIT_8(A)  ((A) & 0x00000100)
#define BIT_9(A)  ((A) & 0x00000200)
#define BIT_A(A)  ((A) & 0x00000400)
#define BIT_B(A)  ((A) & 0x00000800)
#define BIT_C(A)  ((A) & 0x00001000)
#define BIT_D(A)  ((A) & 0x00002000)
#define BIT_E(A)  ((A) & 0x00004000)
#define BIT_F(A)  ((A) & 0x00008000)
#define BIT_10(A) ((A) & 0x00010000)
#define BIT_11(A) ((A) & 0x00020000)
#define BIT_12(A) ((A) & 0x00040000)
#define BIT_13(A) ((A) & 0x00080000)
#define BIT_14(A) ((A) & 0x00100000)
#define BIT_15(A) ((A) & 0x00200000)
#define BIT_16(A) ((A) & 0x00400000)
#define BIT_17(A) ((A) & 0x00800000)
#define BIT_18(A) ((A) & 0x01000000)
#define BIT_19(A) ((A) & 0x02000000)
#define BIT_1A(A) ((A) & 0x04000000)
#define BIT_1B(A) ((A) & 0x08000000)
#define BIT_1C(A) ((A) & 0x10000000)
#define BIT_1D(A) ((A) & 0x20000000)
#define BIT_1E(A) ((A) & 0x40000000)
#define BIT_1F(A) ((A) & 0x80000000)

/* Get the most significant bit for specific sizes */
#define GET_MSB_8(A)  ((A) & 0x80)
#define GET_MSB_9(A)  ((A) & 0x100)
#define GET_MSB_16(A) ((A) & 0x8000)
#define GET_MSB_17(A) ((A) & 0x10000)
#define GET_MSB_32(A) ((A) & 0x80000000)
#if M68K_USE_64_BIT
#define GET_MSB_33(A) ((A) & 0x100000000)
#endif /* M68K_USE_64_BIT */

/* Isolate nibbles */
#define LOW_NIBBLE(A)  ((A) & 0x0f)
#define HIGH_NIBBLE(A) ((A) & 0xf0)

/* These are used to isolate 8, 16, and 32 bit sizes */
#define MASK_OUT_ABOVE_2(A)  ((A) & 3)
#define MASK_OUT_ABOVE_8(A)  ((A) & 0xff)
#define MASK_OUT_ABOVE_16(A) ((A) & 0xffff)
#define MASK_OUT_BELOW_2(A)  ((A) & ~3)
#define MASK_OUT_BELOW_8(A)  ((A) & ~0xff)
#define MASK_OUT_BELOW_16(A) ((A) & ~0xffff)

/* No need to mask if we are 32 bit */
#if M68K_INT_GT_32_BIT || M68K_USE_64_BIT
	#define MASK_OUT_ABOVE_32(A) ((A) & 0xffffffff)
	#define MASK_OUT_BELOW_32(A) ((A) & ~0xffffffff)
#else
	#define MASK_OUT_ABOVE_32(A) (A)
	#define MASK_OUT_BELOW_32(A) 0
#endif /* M68K_INT_GT_32_BIT || M68K_USE_64_BIT */

/* Simulate address lines of 68k family */
#define ADDRESS_68K(A) ((A)&CPU_ADDRESS_MASK)


/* Shift & Rotate Macros. */
#define LSL(A, C) ((A) << (C))
#define LSR(A, C) ((A) >> (C))

/* Some > 32-bit optimizations */
#if M68K_INT_GT_32_BIT
	/* Shift left and right */
	#define LSR_32(A, C) ((A) >> (C))
	#define LSL_32(A, C) ((A) << (C))
#else
	/* We have to do this because the morons at ANSI decided that shifts
	 * by >= data size are undefined.
	 */
	#define LSR_32(A, C) ((C) < 32 ? (A) >> (C) : 0)
	#define LSL_32(A, C) ((C) < 32 ? (A) << (C) : 0)
#endif /* M68K_INT_GT_32_BIT */

#if M68K_USE_64_BIT
	#define LSL_32_64(A, C) ((A) << (C))
	#define LSR_32_64(A, C) ((A) >> (C))
	#define ROL_33_64(A, C) (LSL_32_64(A, C) | LSR_32_64(A, 33-(C)))
	#define ROR_33_64(A, C) (LSR_32_64(A, C) | LSL_32_64(A, 33-(C)))
#endif /* M68K_USE_64_BIT */

#define ROL_8(A, C)      MASK_OUT_ABOVE_8(LSL(A, C) | LSR(A, 8-(C)))
#define ROL_9(A, C)                      (LSL(A, C) | LSR(A, 9-(C)))
#define ROL_16(A, C)    MASK_OUT_ABOVE_16(LSL(A, C) | LSR(A, 16-(C)))
#define ROL_17(A, C)                     (LSL(A, C) | LSR(A, 17-(C)))
#define ROL_32(A, C)    MASK_OUT_ABOVE_32(LSL_32(A, C) | LSR_32(A, 32-(C)))
#define ROL_33(A, C)                     (LSL_32(A, C) | LSR_32(A, 33-(C)))

#define ROR_8(A, C)      MASK_OUT_ABOVE_8(LSR(A, C) | LSL(A, 8-(C)))
#define ROR_9(A, C)                      (LSR(A, C) | LSL(A, 9-(C)))
#define ROR_16(A, C)    MASK_OUT_ABOVE_16(LSR(A, C) | LSL(A, 16-(C)))
#define ROR_17(A, C)                     (LSR(A, C) | LSL(A, 17-(C)))
#define ROR_32(A, C)    MASK_OUT_ABOVE_32(LSR_32(A, C) | LSL_32(A, 32-(C)))
#define ROR_33(A, C)                     (LSR_32(A, C) | LSL_32(A, 33-(C)))



/* ------------------------------ CPU Access ------------------------------ */

/* Access the CPU registers */
#define CPU_TYPE         m68ki_cpu.cpu_type

#define REG_DA           m68ki_cpu.dar /* easy access to data and address regs */
#define REG_D            m68ki_cpu.dar
#define REG_A            (m68ki_cpu.dar+8)
#define REG_PPC 		 m68ki_cpu.ppc
#define REG_PC           m68ki_cpu.pc
#define REG_SP_BASE      m68ki_cpu.sp
#define REG_USP          m68ki_cpu.sp[0]
#define REG_ISP          m68ki_cpu.sp[4]
#define REG_MSP          m68ki_cpu.sp[6]
#define REG_SP           m68ki_cpu.dar[15]
#define REG_VBR          m68ki_cpu.vbr
#define REG_SFC          m68ki_cpu.sfc
#define REG_DFC          m68ki_cpu.dfc
#define REG_CACR         m68ki_cpu.cacr
#define REG_CAAR         m68ki_cpu.caar
#define REG_IR           m68ki_cpu.ir

#define FLAG_T1          m68ki_cpu.t1_flag
#define FLAG_T0          m68ki_cpu.t0_flag
#define FLAG_S           m68ki_cpu.s_flag
#define FLAG_M           m68ki_cpu.m_flag
#define FLAG_X           m68ki_cpu.x_flag
#define FLAG_N           m68ki_cpu.n_flag
#define FLAG_Z           m68ki_cpu.not_z_flag
#define FLAG_V           m68ki_cpu.v_flag
#define FLAG_C           m68ki_cpu.c_flag
#define FLAG_INT_MASK    m68ki_cpu.int_mask

#define CPU_INT_LEVEL    m68ki_cpu.int_level /* ASG: changed from CPU_INTS_PENDING */
#define CPU_INT_CYCLES   m68ki_cpu.int_cycles /* ASG */
#define CPU_STOPPED      m68ki_cpu.stopped
#define CPU_PREF_ADDR    m68ki_cpu.pref_addr
#define CPU_PREF_DATA    m68ki_cpu.pref_data
#define CPU_ADDRESS_MASK m68ki_cpu.address_mask
#define CPU_SR_MASK      m68ki_cpu.sr_mask
#define CPU_INSTR_MODE   m68ki_cpu.instr_mode
#define CPU_RUN_MODE     m68ki_cpu.run_mode

#define CYC_INSTRUCTION  m68ki_cpu.cyc_instruction
#define CYC_EXCEPTION    m68ki_cpu.cyc_exception
#define CYC_BCC_NOTAKE_B m68ki_cpu.cyc_bcc_notake_b
#define CYC_BCC_NOTAKE_W m68ki_cpu.cyc_bcc_notake_w
#define CYC_DBCC_F_NOEXP m68ki_cpu.cyc_dbcc_f_noexp
#define CYC_DBCC_F_EXP   m68ki_cpu.cyc_dbcc_f_exp
#define CYC_SCC_R_TRUE   m68ki_cpu.cyc_scc_r_true
#define CYC_MOVEM_W      m68ki_cpu.cyc_movem_w
#define CYC_MOVEM_L      m68ki_cpu.cyc_movem_l
#define CYC_SHIFT        m68ki_cpu.cyc_shift
#define CYC_RESET        m68ki_cpu.cyc_reset


#define CALLBACK_INT_ACK      m68ki_cpu.int_ack_callback
#define CALLBACK_BKPT_ACK     m68ki_cpu.bkpt_ack_callback
#define CALLBACK_RESET_INSTR  m68ki_cpu.reset_instr_callback
#define CALLBACK_CMPILD_INSTR m68ki_cpu.cmpild_instr_callback
#define CALLBACK_RTE_INSTR    m68ki_cpu.rte_instr_callback
#define CALLBACK_PC_CHANGED   m68ki_cpu.pc_changed_callback
#define CALLBACK_SET_FC       m68ki_cpu.set_fc_callback
#define CALLBACK_INSTR_HOOK   m68ki_cpu.instr_hook_callback



/* ----------------------------- Configuration ---------------------------- */

/* These defines are dependant on the configuration defines in m68kconf.h */

/* Disable certain comparisons if we're not using all CPU types */
#if M68K_EMULATE_020
	#define CPU_TYPE_IS_020_PLUS(A)    ((A) & CPU_TYPE_020)
	#define CPU_TYPE_IS_020_LESS(A)    1
#else
	#define CPU_TYPE_IS_020_PLUS(A)    0
	#define CPU_TYPE_IS_020_LESS(A)    1
#endif

#if M68K_EMULATE_EC020
	#define CPU_TYPE_IS_EC020_PLUS(A)  ((A) & (CPU_TYPE_EC020 | CPU_TYPE_020))
	#define CPU_TYPE_IS_EC020_LESS(A)  ((A) & (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_EC020))
#else
	#define CPU_TYPE_IS_EC020_PLUS(A)  CPU_TYPE_IS_020_PLUS(A)
	#define CPU_TYPE_IS_EC020_LESS(A)  CPU_TYPE_IS_020_LESS(A)
#endif

#if M68K_EMULATE_010
	#define CPU_TYPE_IS_010(A)         ((A) == CPU_TYPE_010)
	#define CPU_TYPE_IS_010_PLUS(A)    ((A) & (CPU_TYPE_010 | CPU_TYPE_EC020 | CPU_TYPE_020))
	#define CPU_TYPE_IS_010_LESS(A)    ((A) & (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010))
#else
	#define CPU_TYPE_IS_010(A)         0
	#define CPU_TYPE_IS_010_PLUS(A)    CPU_TYPE_IS_EC020_PLUS(A)
	#define CPU_TYPE_IS_010_LESS(A)    CPU_TYPE_IS_EC020_LESS(A)
#endif

#if M68K_EMULATE_020 || M68K_EMULATE_EC020
	#define CPU_TYPE_IS_020_VARIANT(A) ((A) & (CPU_TYPE_EC020 | CPU_TYPE_020))
#else
	#define CPU_TYPE_IS_020_VARIANT(A) 0
#endif

#if M68K_EMULATE_020 || M68K_EMULATE_EC020 || M68K_EMULATE_010
	#define CPU_TYPE_IS_000(A)         ((A) == CPU_TYPE_000 || (A) == CPU_TYPE_008)
#else
	#define CPU_TYPE_IS_000(A)         1
#endif


#if !M68K_SEPARATE_READS
#define m68k_read_immediate_16(A) m68ki_read_program_16(A)
#define m68k_read_immediate_32(A) m68ki_read_program_32(A)

#define m68k_read_pcrelative_8(A) m68ki_read_program_8(A)
#define m68k_read_pcrelative_16(A) m68ki_read_program_16(A)
#define m68k_read_pcrelative_32(A) m68ki_read_program_32(A)
#endif /* M68K_SEPARATE_READS */


/* Enable or disable callback functions */
#if M68K_EMULATE_INT_ACK
	#if M68K_EMULATE_INT_ACK == OPT_SPECIFY_HANDLER
		#define m68ki_int_ack(A) M68K_INT_ACK_CALLBACK(A)
	#else
		#define m68ki_int_ack(A) CALLBACK_INT_ACK(A)
	#endif
#else
	/* Default action is to used autovector mode, which is most common */
	#define m68ki_int_ack(A) M68K_INT_ACK_AUTOVECTOR
#endif /* M68K_EMULATE_INT_ACK */

#if M68K_EMULATE_BKPT_ACK
	#if M68K_EMULATE_BKPT_ACK == OPT_SPECIFY_HANDLER
		#define m68ki_bkpt_ack(A) M68K_BKPT_ACK_CALLBACK(A)
	#else
		#define m68ki_bkpt_ack(A) CALLBACK_BKPT_ACK(A)
	#endif
#else
	#define m68ki_bkpt_ack(A)
#endif /* M68K_EMULATE_BKPT_ACK */

#if M68K_EMULATE_RESET
	#if M68K_EMULATE_RESET == OPT_SPECIFY_HANDLER
		#define m68ki_output_reset() M68K_RESET_CALLBACK()
	#else
		#define m68ki_output_reset() CALLBACK_RESET_INSTR()
	#endif
#else
	#define m68ki_output_reset()
#endif /* M68K_EMULATE_RESET */

#if M68K_CMPILD_HAS_CALLBACK
	#if M68K_CMPILD_HAS_CALLBACK == OPT_SPECIFY_HANDLER
		#define m68ki_cmpild_callback(v,r) M68K_CMPILD_CALLBACK(v,r)
	#else
		#define m68ki_cmpild_callback(v,r) CALLBACK_CMPILD_INSTR(v,r)
	#endif
#else
	#define m68ki_cmpild_callback(v,r)
#endif /* M68K_CMPILD_HAS_CALLBACK */

#if M68K_RTE_HAS_CALLBACK
	#if M68K_RTE_HAS_CALLBACK == OPT_SPECIFY_HANDLER
		#define m68ki_rte_callback() M68K_RTE_CALLBACK()
	#else
		#define m68ki_rte_callback() CALLBACK_RTE_INSTR()
	#endif
#else
	#define m68ki_rte_callback()
#endif /* M68K_RTE_HAS_CALLBACK */

#if M68K_INSTRUCTION_HOOK
	#if M68K_INSTRUCTION_HOOK == OPT_SPECIFY_HANDLER
		#define m68ki_instr_hook() M68K_INSTRUCTION_CALLBACK()
	#else
		#define m68ki_instr_hook() CALLBACK_INSTR_HOOK()
	#endif
#else
	#define m68ki_instr_hook()
#endif /* M68K_INSTRUCTION_HOOK */

#if M68K_MONITOR_PC
	#if M68K_MONITOR_PC == OPT_SPECIFY_HANDLER
		#define m68ki_pc_changed(A) M68K_SET_PC_CALLBACK(ADDRESS_68K(A))
	#else
		#define m68ki_pc_changed(A) CALLBACK_PC_CHANGED(ADDRESS_68K(A))
	#endif
#else
	#define m68ki_pc_changed(A)
#endif /* M68K_MONITOR_PC */


/* Enable or disable function code emulation */
#if M68K_EMULATE_FC
	#if M68K_EMULATE_FC == OPT_SPECIFY_HANDLER
		#define m68ki_set_fc(A) M68K_SET_FC_CALLBACK(A)
	#else
		#define m68ki_set_fc(A) CALLBACK_SET_FC(A)
	#endif
	#define m68ki_use_data_space() m68ki_address_space = FUNCTION_CODE_USER_DATA
	#define m68ki_use_program_space() m68ki_address_space = FUNCTION_CODE_USER_PROGRAM
	#define m68ki_get_address_space() m68ki_address_space
#else
	#define m68ki_set_fc(A)
	#define m68ki_use_data_space()
	#define m68ki_use_program_space()
	#define m68ki_get_address_space() FUNCTION_CODE_USER_DATA
#endif /* M68K_EMULATE_FC */


/* Enable or disable trace emulation */
#if M68K_EMULATE_TRACE
	/* Initiates trace checking before each instruction (t1) */
	#define m68ki_trace_t1() m68ki_tracing = FLAG_T1
	/* adds t0 to trace checking if we encounter change of flow */
	#define m68ki_trace_t0() m68ki_tracing |= FLAG_T0
	/* Clear all tracing */
	#define m68ki_clear_trace() m68ki_tracing = 0
	/* Cause a trace exception if we are tracing */
	#define m68ki_exception_if_trace() if(m68ki_tracing) m68ki_exception_trace()
#else
	#define m68ki_trace_t1()
	#define m68ki_trace_t0()
	#define m68ki_clear_trace()
	#define m68ki_exception_if_trace()
#endif /* M68K_EMULATE_TRACE */



/* Address error */
#if M68K_EMULATE_ADDRESS_ERROR
	#include <setjmp.h>
	extern jmp_buf m68ki_aerr_trap;

	#define m68ki_set_address_error_trap() \
		if(setjmp(m68ki_aerr_trap) != 0) \
		{ \
			m68ki_exception_address_error(); \
			if(CPU_STOPPED) \
			{ \
				SET_CYCLES(0); \
				CPU_INT_CYCLES = 0; \
				return m68ki_initial_cycles; \
			} \
		}

	#define m68ki_check_address_error(ADDR, WRITE_MODE, FC) \
		if((ADDR)&1) \
		{ \
			m68ki_aerr_address = ADDR; \
			m68ki_aerr_write_mode = WRITE_MODE; \
			m68ki_aerr_fc = FC; \
			longjmp(m68ki_aerr_trap, 1); \
		}
#else
	#define m68ki_set_address_error_trap()
	#define m68ki_check_address_error(ADDR, WRITE_MODE, FC)
#endif /* M68K_ADDRESS_ERROR */

/* Logging */
#if M68K_LOG_ENABLE
	#include <stdio.h>
	extern FILE* M68K_LOG_FILEHANDLE
	extern char* m68ki_cpu_names[];

	#define M68K_DO_LOG(A) if(M68K_LOG_FILEHANDLE) fprintf A
	#if M68K_LOG_1010_1111
		#define M68K_DO_LOG_EMU(A) if(M68K_LOG_FILEHANDLE) fprintf A
	#else
		#define M68K_DO_LOG_EMU(A)
	#endif
#else
	#define M68K_DO_LOG(A)
	#define M68K_DO_LOG_EMU(A)
#endif



/* -------------------------- EA / Operand Access ------------------------- */

/*
 * The general instruction format follows this pattern:
 * .... XXX. .... .YYY
 * where XXX is register X and YYY is register Y
 */
/* Data Register Isolation */
#define DX (REG_D[(REG_IR >> 9) & 7])
#define DY (REG_D[REG_IR & 7])
/* Address Register Isolation */
#define AX (REG_A[(REG_IR >> 9) & 7])
#define AY (REG_A[REG_IR & 7])


/* Effective Address Calculations */
#define EA_AY_AI_8()   AY                                    /* address register indirect */
#define EA_AY_AI_16()  EA_AY_AI_8()
#define EA_AY_AI_32()  EA_AY_AI_8()
#define EA_AY_PI_8()   (AY++)                                /* postincrement (size = byte) */
#define EA_AY_PI_16()  ((AY+=2)-2)                           /* postincrement (size = word) */
#define EA_AY_PI_32()  ((AY+=4)-4)                           /* postincrement (size = long) */
#define EA_AY_PD_8()   (--AY)                                /* predecrement (size = byte) */
#define EA_AY_PD_16()  (AY-=2)                               /* predecrement (size = word) */
#define EA_AY_PD_32()  (AY-=4)                               /* predecrement (size = long) */
#define EA_AY_DI_8()   (AY+MAKE_INT_16(m68ki_read_imm_16())) /* displacement */
#define EA_AY_DI_16()  EA_AY_DI_8()
#define EA_AY_DI_32()  EA_AY_DI_8()
#define EA_AY_IX_8()   m68ki_get_ea_ix(AY)                   /* indirect + index */
#define EA_AY_IX_16()  EA_AY_IX_8()
#define EA_AY_IX_32()  EA_AY_IX_8()

#define EA_AX_AI_8()   AX
#define EA_AX_AI_16()  EA_AX_AI_8()
#define EA_AX_AI_32()  EA_AX_AI_8()
#define EA_AX_PI_8()   (AX++)
#define EA_AX_PI_16()  ((AX+=2)-2)
#define EA_AX_PI_32()  ((AX+=4)-4)
#define EA_AX_PD_8()   (--AX)
#define EA_AX_PD_16()  (AX-=2)
#define EA_AX_PD_32()  (AX-=4)
#define EA_AX_DI_8()   (AX+MAKE_INT_16(m68ki_read_imm_16()))
#define EA_AX_DI_16()  EA_AX_DI_8()
#define EA_AX_DI_32()  EA_AX_DI_8()
#define EA_AX_IX_8()   m68ki_get_ea_ix(AX)
#define EA_AX_IX_16()  EA_AX_IX_8()
#define EA_AX_IX_32()  EA_AX_IX_8()

#define EA_A7_PI_8()   ((REG_A[7]+=2)-2)
#define EA_A7_PD_8()   (REG_A[7]-=2)

#define EA_AW_8()      MAKE_INT_16(m68ki_read_imm_16())      /* absolute word */
#define EA_AW_16()     EA_AW_8()
#define EA_AW_32()     EA_AW_8()
#define EA_AL_8()      m68ki_read_imm_32()                   /* absolute long */
#define EA_AL_16()     EA_AL_8()
#define EA_AL_32()     EA_AL_8()
#define EA_PCDI_8()    m68ki_get_ea_pcdi()                   /* pc indirect + displacement */
#define EA_PCDI_16()   EA_PCDI_8()
#define EA_PCDI_32()   EA_PCDI_8()
#define EA_PCIX_8()    m68ki_get_ea_pcix()                   /* pc indirect + index */
#define EA_PCIX_16()   EA_PCIX_8()
#define EA_PCIX_32()   EA_PCIX_8()


#define OPER_I_8()     m68ki_read_imm_8()
#define OPER_I_16()    m68ki_read_imm_16()
#define OPER_I_32()    m68ki_read_imm_32()



/* --------------------------- Status Register ---------------------------- */

/* Flag Calculation Macros */
#define CFLAG_8(A) (A)
#define CFLAG_16(A) ((A)>>8)

#if M68K_INT_GT_32_BIT
	#define CFLAG_ADD_32(S, D, R) ((R)>>24)
	#define CFLAG_SUB_32(S, D, R) ((R)>>24)
#else
	#define CFLAG_ADD_32(S, D, R) (((S & D) | (~R & (S | D)))>>23)
	#define CFLAG_SUB_32(S, D, R) (((S & R) | (~D & (S | R)))>>23)
#endif /* M68K_INT_GT_32_BIT */

#define VFLAG_ADD_8(S, D, R) ((S^R) & (D^R))
#define VFLAG_ADD_16(S, D, R) (((S^R) & (D^R))>>8)
#define VFLAG_ADD_32(S, D, R) (((S^R) & (D^R))>>24)

#define VFLAG_SUB_8(S, D, R) ((S^D) & (R^D))
#define VFLAG_SUB_16(S, D, R) (((S^D) & (R^D))>>8)
#define VFLAG_SUB_32(S, D, R) (((S^D) & (R^D))>>24)

#define NFLAG_8(A) (A)
#define NFLAG_16(A) ((A)>>8)
#define NFLAG_32(A) ((A)>>24)
#define NFLAG_64(A) ((A)>>56)

#define ZFLAG_8(A) MASK_OUT_ABOVE_8(A)
#define ZFLAG_16(A) MASK_OUT_ABOVE_16(A)
#define ZFLAG_32(A) MASK_OUT_ABOVE_32(A)


/* Flag values */
#define NFLAG_SET   0x80
#define NFLAG_CLEAR 0
#define CFLAG_SET   0x100
#define CFLAG_CLEAR 0
#define XFLAG_SET   0x100
#define XFLAG_CLEAR 0
#define VFLAG_SET   0x80
#define VFLAG_CLEAR 0
#define ZFLAG_SET   0
#define ZFLAG_CLEAR 0xffffffff

#define SFLAG_SET   4
#define SFLAG_CLEAR 0
#define MFLAG_SET   2
#define MFLAG_CLEAR 0

/* Turn flag values into 1 or 0 */
#define XFLAG_AS_1() ((FLAG_X>>8)&1)
#define NFLAG_AS_1() ((FLAG_N>>7)&1)
#define VFLAG_AS_1() ((FLAG_V>>7)&1)
#define ZFLAG_AS_1() (!FLAG_Z)
#define CFLAG_AS_1() ((FLAG_C>>8)&1)


/* Conditions */
#define COND_CS() (FLAG_C&0x100)
#define COND_CC() (!COND_CS())
#define COND_VS() (FLAG_V&0x80)
#define COND_VC() (!COND_VS())
#define COND_NE() FLAG_Z
#define COND_EQ() (!COND_NE())
#define COND_MI() (FLAG_N&0x80)
#define COND_PL() (!COND_MI())
#define COND_LT() ((FLAG_N^FLAG_V)&0x80)
#define COND_GE() (!COND_LT())
#define COND_HI() (COND_CC() && COND_NE())
#define COND_LS() (COND_CS() || COND_EQ())
#define COND_GT() (COND_GE() && COND_NE())
#define COND_LE() (COND_LT() || COND_EQ())

/* Reversed conditions */
#define COND_NOT_CS() COND_CC()
#define COND_NOT_CC() COND_CS()
#define COND_NOT_VS() COND_VC()
#define COND_NOT_VC() COND_VS()
#define COND_NOT_NE() COND_EQ()
#define COND_NOT_EQ() COND_NE()
#define COND_NOT_MI() COND_PL()
#define COND_NOT_PL() COND_MI()
#define COND_NOT_LT() COND_GE()
#define COND_NOT_GE() COND_LT()
#define COND_NOT_HI() COND_LS()
#define COND_NOT_LS() COND_HI()
#define COND_NOT_GT() COND_LE()
#define COND_NOT_LE() COND_GT()

/* Not real conditions, but here for convenience */
#define COND_XS() (FLAG_X&0x100)
#define COND_XC() (!COND_XS)


/* Get the condition code register */
#define m68ki_get_ccr() ((COND_XS() >> 4) | \
						 (COND_MI() >> 4) | \
						 (COND_EQ() << 2) | \
						 (COND_VS() >> 6) | \
						 (COND_CS() >> 8))

/* Get the status register */
#define m68ki_get_sr() ( FLAG_T1              | \
						 FLAG_T0              | \
						(FLAG_S        << 11) | \
						(FLAG_M        << 11) | \
						 FLAG_INT_MASK        | \
						 m68ki_get_ccr())



/* ---------------------------- Cycle Counting ---------------------------- */

#define ADD_CYCLES(A)    m68ki_remaining_cycles += (A)
#define USE_CYCLES(A)    m68ki_remaining_cycles -= (A)
#define SET_CYCLES(A)    m68ki_remaining_cycles = A
#define GET_CYCLES()     m68ki_remaining_cycles
#define USE_ALL_CYCLES() m68ki_remaining_cycles = 0



/* ----------------------------- Read / Write ----------------------------- */

/* Read from the current address space */
#define m68ki_read_8(A)  m68ki_read_8_fc (A, FLAG_S | m68ki_get_address_space())
#define m68ki_read_16(A) m68ki_read_16_fc(A, FLAG_S | m68ki_get_address_space())
#define m68ki_read_32(A) m68ki_read_32_fc(A, FLAG_S | m68ki_get_address_space())

/* Write to the current data space */
#define m68ki_write_8(A, V)  m68ki_write_8_fc (A, FLAG_S | FUNCTION_CODE_USER_DATA, V)
#define m68ki_write_16(A, V) m68ki_write_16_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA, V)
#define m68ki_write_32(A, V) m68ki_write_32_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA, V)

#if M68K_SIMULATE_PD_WRITES
#define m68ki_write_32_pd(A, V) m68ki_write_32_pd_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA, V)
#else
#define m68ki_write_32_pd(A, V) m68ki_write_32_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA, V)
#endif

/* map read immediate 8 to read immediate 16 */
#define m68ki_read_imm_8() MASK_OUT_ABOVE_8(m68ki_read_imm_16())

/* Map PC-relative reads */
#define m68ki_read_pcrel_8(A) m68k_read_pcrelative_8(A)
#define m68ki_read_pcrel_16(A) m68k_read_pcrelative_16(A)
#define m68ki_read_pcrel_32(A) m68k_read_pcrelative_32(A)

/* Read from the program space */
#define m68ki_read_program_8(A) 	m68ki_read_8_fc(A, FLAG_S | FUNCTION_CODE_USER_PROGRAM)
#define m68ki_read_program_16(A) 	m68ki_read_16_fc(A, FLAG_S | FUNCTION_CODE_USER_PROGRAM)
#define m68ki_read_program_32(A) 	m68ki_read_32_fc(A, FLAG_S | FUNCTION_CODE_USER_PROGRAM)

/* Read from the data space */
#define m68ki_read_data_8(A) 	m68ki_read_8_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA)
#define m68ki_read_data_16(A) 	m68ki_read_16_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA)
#define m68ki_read_data_32(A) 	m68ki_read_32_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA)



/* ======================================================================== */
/* =============================== PROTOTYPES ============================= */
/* ======================================================================== */

typedef struct
{
	uint cpu_type;     /* CPU Type: 68000, 68008, 68010, 68EC020, or 68020 */
	uint dar[16];      /* Data and Address Registers */
	uint ppc;		   /* Previous program counter */
	uint pc;           /* Program Counter */
	uint sp[7];        /* User, Interrupt, and Master Stack Pointers */
	uint vbr;          /* Vector Base Register (m68010+) */
	uint sfc;          /* Source Function Code Register (m68010+) */
	uint dfc;          /* Destination Function Code Register (m68010+) */
	uint cacr;         /* Cache Control Register (m68020, unemulated) */
	uint caar;         /* Cache Address Register (m68020, unemulated) */
	uint ir;           /* Instruction Register */
	uint t1_flag;      /* Trace 1 */
	uint t0_flag;      /* Trace 0 */
	uint s_flag;       /* Supervisor */
	uint m_flag;       /* Master/Interrupt state */
	uint x_flag;       /* Extend */
	uint n_flag;       /* Negative */
	uint not_z_flag;   /* Zero, inverted for speedups */
	uint v_flag;       /* Overflow */
	uint c_flag;       /* Carry */
	uint int_mask;     /* I0-I2 */
	uint int_level;    /* State of interrupt pins IPL0-IPL2 -- ASG: changed from ints_pending */
	uint int_cycles;   /* ASG: extra cycles from generated interrupts */
	uint stopped;      /* Stopped state */
	uint pref_addr;    /* Last prefetch address */
	uint pref_data;    /* Data in the prefetch queue */
	uint address_mask; /* Available address pins */
	uint sr_mask;      /* Implemented status register bits */
	uint instr_mode;   /* Stores whether we are in instruction mode or group 0/1 exception mode */
	uint run_mode;     /* Stores whether we are processing a reset, bus error, address error, or something else */

	/* Clocks required for instructions / exceptions */
	uint cyc_bcc_notake_b;
	uint cyc_bcc_notake_w;
	uint cyc_dbcc_f_noexp;
	uint cyc_dbcc_f_exp;
	uint cyc_scc_r_true;
	uint cyc_movem_w;
	uint cyc_movem_l;
	uint cyc_shift;
	uint cyc_reset;
	uint8* cyc_instruction;
	uint8* cyc_exception;

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


extern m68ki_cpu_core m68ki_cpu;
extern sint           m68ki_remaining_cycles;
extern uint           m68ki_tracing;
extern uint8          m68ki_shift_8_table[];
extern uint16         m68ki_shift_16_table[];
extern uint           m68ki_shift_32_table[];
extern uint8          m68ki_exception_cycle_table[][256];
extern uint           m68ki_address_space;
extern uint8          m68ki_ea_idx_cycle_table[];

extern uint           m68ki_aerr_address;
extern uint           m68ki_aerr_write_mode;
extern uint           m68ki_aerr_fc;

/* Read data immediately after the program counter */
INLINE uint m68ki_read_imm_16(void);
INLINE uint m68ki_read_imm_32(void);

/* Read data with specific function code */
INLINE uint m68ki_read_8_fc  (uint address, uint fc);
INLINE uint m68ki_read_16_fc (uint address, uint fc);
INLINE uint m68ki_read_32_fc (uint address, uint fc);

/* Write data with specific function code */
INLINE void m68ki_write_8_fc (uint address, uint fc, uint value);
INLINE void m68ki_write_16_fc(uint address, uint fc, uint value);
INLINE void m68ki_write_32_fc(uint address, uint fc, uint value);
#if M68K_SIMULATE_PD_WRITES
INLINE void m68ki_write_32_pd_fc(uint address, uint fc, uint value);
#endif /* M68K_SIMULATE_PD_WRITES */

/* Indexed and PC-relative ea fetching */
INLINE uint m68ki_get_ea_pcdi(void);
INLINE uint m68ki_get_ea_pcix(void);
INLINE uint m68ki_get_ea_ix(uint An);

/* Operand fetching */
INLINE uint OPER_AY_AI_8(void);
INLINE uint OPER_AY_AI_16(void);
INLINE uint OPER_AY_AI_32(void);
INLINE uint OPER_AY_PI_8(void);
INLINE uint OPER_AY_PI_16(void);
INLINE uint OPER_AY_PI_32(void);
INLINE uint OPER_AY_PD_8(void);
INLINE uint OPER_AY_PD_16(void);
INLINE uint OPER_AY_PD_32(void);
INLINE uint OPER_AY_DI_8(void);
INLINE uint OPER_AY_DI_16(void);
INLINE uint OPER_AY_DI_32(void);
INLINE uint OPER_AY_IX_8(void);
INLINE uint OPER_AY_IX_16(void);
INLINE uint OPER_AY_IX_32(void);

INLINE uint OPER_AX_AI_8(void);
INLINE uint OPER_AX_AI_16(void);
INLINE uint OPER_AX_AI_32(void);
INLINE uint OPER_AX_PI_8(void);
INLINE uint OPER_AX_PI_16(void);
INLINE uint OPER_AX_PI_32(void);
INLINE uint OPER_AX_PD_8(void);
INLINE uint OPER_AX_PD_16(void);
INLINE uint OPER_AX_PD_32(void);
INLINE uint OPER_AX_DI_8(void);
INLINE uint OPER_AX_DI_16(void);
INLINE uint OPER_AX_DI_32(void);
INLINE uint OPER_AX_IX_8(void);
INLINE uint OPER_AX_IX_16(void);
INLINE uint OPER_AX_IX_32(void);

INLINE uint OPER_A7_PI_8(void);
INLINE uint OPER_A7_PD_8(void);

INLINE uint OPER_AW_8(void);
INLINE uint OPER_AW_16(void);
INLINE uint OPER_AW_32(void);
INLINE uint OPER_AL_8(void);
INLINE uint OPER_AL_16(void);
INLINE uint OPER_AL_32(void);
INLINE uint OPER_PCDI_8(void);
INLINE uint OPER_PCDI_16(void);
INLINE uint OPER_PCDI_32(void);
INLINE uint OPER_PCIX_8(void);
INLINE uint OPER_PCIX_16(void);
INLINE uint OPER_PCIX_32(void);

/* Stack operations */
INLINE void m68ki_push_16(uint value);
INLINE void m68ki_push_32(uint value);
INLINE uint m68ki_pull_16(void);
INLINE uint m68ki_pull_32(void);

/* Program flow operations */
INLINE void m68ki_jump(uint new_pc);
INLINE void m68ki_jump_vector(uint vector);
INLINE void m68ki_branch_8(uint offset);
INLINE void m68ki_branch_16(uint offset);
INLINE void m68ki_branch_32(uint offset);

/* Status register operations. */
INLINE void m68ki_set_s_flag(uint value);            /* Only bit 2 of value should be set (i.e. 4 or 0) */
INLINE void m68ki_set_sm_flag(uint value);           /* only bits 1 and 2 of value should be set */
INLINE void m68ki_set_ccr(uint value);               /* set the condition code register */
INLINE void m68ki_set_sr(uint value);                /* set the status register */
INLINE void m68ki_set_sr_noint(uint value);          /* set the status register */

/* Exception processing */
INLINE uint m68ki_init_exception(void);              /* Initial exception processing */

INLINE void m68ki_stack_frame_3word(uint pc, uint sr); /* Stack various frame types */
INLINE void m68ki_stack_frame_buserr(uint sr);

INLINE void m68ki_stack_frame_0000(uint pc, uint sr, uint vector);
INLINE void m68ki_stack_frame_0001(uint pc, uint sr, uint vector);
INLINE void m68ki_stack_frame_0010(uint sr, uint vector);
INLINE void m68ki_stack_frame_1000(uint pc, uint sr, uint vector);
INLINE void m68ki_stack_frame_1010(uint sr, uint vector, uint pc);
INLINE void m68ki_stack_frame_1011(uint sr, uint vector, uint pc);

INLINE void m68ki_exception_trap(uint vector);
INLINE void m68ki_exception_trapN(uint vector);
INLINE void m68ki_exception_trace(void);
INLINE void m68ki_exception_privilege_violation(void);
INLINE void m68ki_exception_1010(void);
INLINE void m68ki_exception_1111(void);
INLINE void m68ki_exception_illegal(void);
INLINE void m68ki_exception_format_error(void);
INLINE void m68ki_exception_address_error(void);
INLINE void m68ki_exception_interrupt(uint int_level);
INLINE void m68ki_check_interrupts(void);            /* ASG: check for interrupts */

/* quick disassembly (used for logging) */
char* m68ki_disassemble_quick(unsigned int pc, unsigned int cpu_type);


/* ======================================================================== */
/* =========================== UTILITY FUNCTIONS ========================== */
/* ======================================================================== */


/* ---------------------------- Read Immediate ---------------------------- */

/* Handles all immediate reads, does address error check, function code setting,
 * and prefetching if they are enabled in m68kconf.h
 */
INLINE uint m68ki_read_imm_16(void)
{
	m68ki_set_fc(FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error(REG_PC, MODE_READ, FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */
#if M68K_EMULATE_PREFETCH
	if(MASK_OUT_BELOW_2(REG_PC) != CPU_PREF_ADDR)
	{
		CPU_PREF_ADDR = MASK_OUT_BELOW_2(REG_PC);
		CPU_PREF_DATA = m68k_read_immediate_32(ADDRESS_68K(CPU_PREF_ADDR));
	}
	REG_PC += 2;
	return MASK_OUT_ABOVE_16(CPU_PREF_DATA >> ((2-((REG_PC-2)&2))<<3));
#else
	REG_PC += 2;
	return m68k_read_immediate_16(ADDRESS_68K(REG_PC-2));
#endif /* M68K_EMULATE_PREFETCH */
}
INLINE uint m68ki_read_imm_32(void)
{
#if M68K_EMULATE_PREFETCH
	uint temp_val;

	m68ki_set_fc(FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error(REG_PC, MODE_READ, FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */
	if(MASK_OUT_BELOW_2(REG_PC) != CPU_PREF_ADDR)
	{
		CPU_PREF_ADDR = MASK_OUT_BELOW_2(REG_PC);
		CPU_PREF_DATA = m68k_read_immediate_32(ADDRESS_68K(CPU_PREF_ADDR));
	}
	temp_val = CPU_PREF_DATA;
	REG_PC += 2;
	if(MASK_OUT_BELOW_2(REG_PC) != CPU_PREF_ADDR)
	{
		CPU_PREF_ADDR = MASK_OUT_BELOW_2(REG_PC);
		CPU_PREF_DATA = m68k_read_immediate_32(ADDRESS_68K(CPU_PREF_ADDR));
		temp_val = MASK_OUT_ABOVE_32((temp_val << 16) | (CPU_PREF_DATA >> 16));
	}
	REG_PC += 2;

	return temp_val;
#else
	m68ki_set_fc(FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error(REG_PC, MODE_READ, FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */
	REG_PC += 4;
	return m68k_read_immediate_32(ADDRESS_68K(REG_PC-4));
#endif /* M68K_EMULATE_PREFETCH */
}



/* ------------------------- Top level read/write ------------------------- */

/* Handles all memory accesses (except for immediate reads if they are
 * configured to use separate functions in m68kconf.h).
 * All memory accesses must go through these top level functions.
 * These functions will also check for address error and set the function
 * code if they are enabled in m68kconf.h.
 */
INLINE uint m68ki_read_8_fc(uint address, uint fc)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	return m68k_read_memory_8(ADDRESS_68K(address));
}
INLINE uint m68ki_read_16_fc(uint address, uint fc)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error(address, MODE_READ, fc); /* auto-disable (see m68kcpu.h) */
	return m68k_read_memory_16(ADDRESS_68K(address));
}
INLINE uint m68ki_read_32_fc(uint address, uint fc)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error(address, MODE_READ, fc); /* auto-disable (see m68kcpu.h) */
	return m68k_read_memory_32(ADDRESS_68K(address));
}

INLINE void m68ki_write_8_fc(uint address, uint fc, uint value)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68k_write_memory_8(ADDRESS_68K(address), value);
}
INLINE void m68ki_write_16_fc(uint address, uint fc, uint value)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error(address, MODE_WRITE, fc); /* auto-disable (see m68kcpu.h) */
	m68k_write_memory_16(ADDRESS_68K(address), value);
}
INLINE void m68ki_write_32_fc(uint address, uint fc, uint value)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error(address, MODE_WRITE, fc); /* auto-disable (see m68kcpu.h) */
	m68k_write_memory_32(ADDRESS_68K(address), value);
}

#if M68K_SIMULATE_PD_WRITES
INLINE void m68ki_write_32_pd_fc(uint address, uint fc, uint value)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error(address, MODE_WRITE, fc); /* auto-disable (see m68kcpu.h) */
	m68k_write_memory_32_pd(ADDRESS_68K(address), value);
}
#endif


/* --------------------- Effective Address Calculation -------------------- */

/* The program counter relative addressing modes cause operands to be
 * retrieved from program space, not data space.
 */
INLINE uint m68ki_get_ea_pcdi(void)
{
	uint old_pc = REG_PC;
	m68ki_use_program_space(); /* auto-disable */
	return old_pc + MAKE_INT_16(m68ki_read_imm_16());
}


INLINE uint m68ki_get_ea_pcix(void)
{
	m68ki_use_program_space(); /* auto-disable */
	return m68ki_get_ea_ix(REG_PC);
}

/* Indexed addressing modes are encoded as follows:
 *
 * Base instruction format:
 * F E D C B A 9 8 7 6 | 5 4 3 | 2 1 0
 * x x x x x x x x x x | 1 1 0 | BASE REGISTER      (An)
 *
 * Base instruction format for destination EA in move instructions:
 * F E D C | B A 9    | 8 7 6 | 5 4 3 2 1 0
 * x x x x | BASE REG | 1 1 0 | X X X X X X       (An)
 *
 * Brief extension format:
 *  F  |  E D C   |  B  |  A 9  | 8 | 7 6 5 4 3 2 1 0
 * D/A | REGISTER | W/L | SCALE | 0 |  DISPLACEMENT
 *
 * Full extension format:
 *  F     E D C      B     A 9    8   7    6    5 4       3   2 1 0
 * D/A | REGISTER | W/L | SCALE | 1 | BS | IS | BD SIZE | 0 | I/IS
 * BASE DISPLACEMENT (0, 16, 32 bit)                (bd)
 * OUTER DISPLACEMENT (0, 16, 32 bit)               (od)
 *
 * D/A:     0 = Dn, 1 = An                          (Xn)
 * W/L:     0 = W (sign extend), 1 = L              (.SIZE)
 * SCALE:   00=1, 01=2, 10=4, 11=8                  (*SCALE)
 * BS:      0=add base reg, 1=suppress base reg     (An suppressed)
 * IS:      0=add index, 1=suppress index           (Xn suppressed)
 * BD SIZE: 00=reserved, 01=NULL, 10=Word, 11=Long  (size of bd)
 *
 * IS I/IS Operation
 * 0  000  No Memory Indirect
 * 0  001  indir prex with null outer
 * 0  010  indir prex with word outer
 * 0  011  indir prex with long outer
 * 0  100  reserved
 * 0  101  indir postx with null outer
 * 0  110  indir postx with word outer
 * 0  111  indir postx with long outer
 * 1  000  no memory indirect
 * 1  001  mem indir with null outer
 * 1  010  mem indir with word outer
 * 1  011  mem indir with long outer
 * 1  100-111  reserved
 */
INLINE uint m68ki_get_ea_ix(uint An)
{
	/* An = base register */
	uint extension = m68ki_read_imm_16();
	uint Xn = 0;                        /* Index register */
	uint bd = 0;                        /* Base Displacement */
	uint od = 0;                        /* Outer Displacement */

	if(CPU_TYPE_IS_010_LESS(CPU_TYPE))
	{
		/* Calculate index */
		Xn = REG_DA[extension>>12];     /* Xn */
		if(!BIT_B(extension))           /* W/L */
			Xn = MAKE_INT_16(Xn);

		/* Add base register and displacement and return */
		return An + Xn + MAKE_INT_8(extension);
	}

	/* Brief extension format */
	if(!BIT_8(extension))
	{
		/* Calculate index */
		Xn = REG_DA[extension>>12];     /* Xn */
		if(!BIT_B(extension))           /* W/L */
			Xn = MAKE_INT_16(Xn);
		/* Add scale if proper CPU type */
		if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
			Xn <<= (extension>>9) & 3;  /* SCALE */

		/* Add base register and displacement and return */
		return An + Xn + MAKE_INT_8(extension);
	}

	/* Full extension format */

	USE_CYCLES(m68ki_ea_idx_cycle_table[extension&0x3f]);

	/* Check if base register is present */
	if(BIT_7(extension))                /* BS */
		An = 0;                         /* An */

	/* Check if index is present */
	if(!BIT_6(extension))               /* IS */
	{
		Xn = REG_DA[extension>>12];     /* Xn */
		if(!BIT_B(extension))           /* W/L */
			Xn = MAKE_INT_16(Xn);
		Xn <<= (extension>>9) & 3;      /* SCALE */
	}

	/* Check if base displacement is present */
	if(BIT_5(extension))                /* BD SIZE */
		bd = BIT_4(extension) ? m68ki_read_imm_32() : MAKE_INT_16(m68ki_read_imm_16());

	/* If no indirect action, we are done */
	if(!(extension&7))                  /* No Memory Indirect */
		return An + bd + Xn;

	/* Check if outer displacement is present */
	if(BIT_1(extension))                /* I/IS:  od */
		od = BIT_0(extension) ? m68ki_read_imm_32() : MAKE_INT_16(m68ki_read_imm_16());

	/* Postindex */
	if(BIT_2(extension))                /* I/IS:  0 = preindex, 1 = postindex */
		return m68ki_read_32(An + bd) + Xn + od;

	/* Preindex */
	return m68ki_read_32(An + bd + Xn) + od;
}


/* Fetch operands */
INLINE uint OPER_AY_AI_8(void)  {uint ea = EA_AY_AI_8();  return m68ki_read_8(ea); }
INLINE uint OPER_AY_AI_16(void) {uint ea = EA_AY_AI_16(); return m68ki_read_16(ea);}
INLINE uint OPER_AY_AI_32(void) {uint ea = EA_AY_AI_32(); return m68ki_read_32(ea);}
INLINE uint OPER_AY_PI_8(void)  {uint ea = EA_AY_PI_8();  return m68ki_read_8(ea); }
INLINE uint OPER_AY_PI_16(void) {uint ea = EA_AY_PI_16(); return m68ki_read_16(ea);}
INLINE uint OPER_AY_PI_32(void) {uint ea = EA_AY_PI_32(); return m68ki_read_32(ea);}
INLINE uint OPER_AY_PD_8(void)  {uint ea = EA_AY_PD_8();  return m68ki_read_8(ea); }
INLINE uint OPER_AY_PD_16(void) {uint ea = EA_AY_PD_16(); return m68ki_read_16(ea);}
INLINE uint OPER_AY_PD_32(void) {uint ea = EA_AY_PD_32(); return m68ki_read_32(ea);}
INLINE uint OPER_AY_DI_8(void)  {uint ea = EA_AY_DI_8();  return m68ki_read_8(ea); }
INLINE uint OPER_AY_DI_16(void) {uint ea = EA_AY_DI_16(); return m68ki_read_16(ea);}
INLINE uint OPER_AY_DI_32(void) {uint ea = EA_AY_DI_32(); return m68ki_read_32(ea);}
INLINE uint OPER_AY_IX_8(void)  {uint ea = EA_AY_IX_8();  return m68ki_read_8(ea); }
INLINE uint OPER_AY_IX_16(void) {uint ea = EA_AY_IX_16(); return m68ki_read_16(ea);}
INLINE uint OPER_AY_IX_32(void) {uint ea = EA_AY_IX_32(); return m68ki_read_32(ea);}

INLINE uint OPER_AX_AI_8(void)  {uint ea = EA_AX_AI_8();  return m68ki_read_8(ea); }
INLINE uint OPER_AX_AI_16(void) {uint ea = EA_AX_AI_16(); return m68ki_read_16(ea);}
INLINE uint OPER_AX_AI_32(void) {uint ea = EA_AX_AI_32(); return m68ki_read_32(ea);}
INLINE uint OPER_AX_PI_8(void)  {uint ea = EA_AX_PI_8();  return m68ki_read_8(ea); }
INLINE uint OPER_AX_PI_16(void) {uint ea = EA_AX_PI_16(); return m68ki_read_16(ea);}
INLINE uint OPER_AX_PI_32(void) {uint ea = EA_AX_PI_32(); return m68ki_read_32(ea);}
INLINE uint OPER_AX_PD_8(void)  {uint ea = EA_AX_PD_8();  return m68ki_read_8(ea); }
INLINE uint OPER_AX_PD_16(void) {uint ea = EA_AX_PD_16(); return m68ki_read_16(ea);}
INLINE uint OPER_AX_PD_32(void) {uint ea = EA_AX_PD_32(); return m68ki_read_32(ea);}
INLINE uint OPER_AX_DI_8(void)  {uint ea = EA_AX_DI_8();  return m68ki_read_8(ea); }
INLINE uint OPER_AX_DI_16(void) {uint ea = EA_AX_DI_16(); return m68ki_read_16(ea);}
INLINE uint OPER_AX_DI_32(void) {uint ea = EA_AX_DI_32(); return m68ki_read_32(ea);}
INLINE uint OPER_AX_IX_8(void)  {uint ea = EA_AX_IX_8();  return m68ki_read_8(ea); }
INLINE uint OPER_AX_IX_16(void) {uint ea = EA_AX_IX_16(); return m68ki_read_16(ea);}
INLINE uint OPER_AX_IX_32(void) {uint ea = EA_AX_IX_32(); return m68ki_read_32(ea);}

INLINE uint OPER_A7_PI_8(void)  {uint ea = EA_A7_PI_8();  return m68ki_read_8(ea); }
INLINE uint OPER_A7_PD_8(void)  {uint ea = EA_A7_PD_8();  return m68ki_read_8(ea); }

INLINE uint OPER_AW_8(void)     {uint ea = EA_AW_8();     return m68ki_read_8(ea); }
INLINE uint OPER_AW_16(void)    {uint ea = EA_AW_16();    return m68ki_read_16(ea);}
INLINE uint OPER_AW_32(void)    {uint ea = EA_AW_32();    return m68ki_read_32(ea);}
INLINE uint OPER_AL_8(void)     {uint ea = EA_AL_8();     return m68ki_read_8(ea); }
INLINE uint OPER_AL_16(void)    {uint ea = EA_AL_16();    return m68ki_read_16(ea);}
INLINE uint OPER_AL_32(void)    {uint ea = EA_AL_32();    return m68ki_read_32(ea);}
INLINE uint OPER_PCDI_8(void)   {uint ea = EA_PCDI_8();   return m68ki_read_pcrel_8(ea); }
INLINE uint OPER_PCDI_16(void)  {uint ea = EA_PCDI_16();  return m68ki_read_pcrel_16(ea);}
INLINE uint OPER_PCDI_32(void)  {uint ea = EA_PCDI_32();  return m68ki_read_pcrel_32(ea);}
INLINE uint OPER_PCIX_8(void)   {uint ea = EA_PCIX_8();   return m68ki_read_pcrel_8(ea); }
INLINE uint OPER_PCIX_16(void)  {uint ea = EA_PCIX_16();  return m68ki_read_pcrel_16(ea);}
INLINE uint OPER_PCIX_32(void)  {uint ea = EA_PCIX_32();  return m68ki_read_pcrel_32(ea);}



/* ---------------------------- Stack Functions --------------------------- */

/* Push/pull data from the stack */
INLINE void m68ki_push_16(uint value)
{
	REG_SP = MASK_OUT_ABOVE_32(REG_SP - 2);
	m68ki_write_16(REG_SP, value);
}

INLINE void m68ki_push_32(uint value)
{
	REG_SP = MASK_OUT_ABOVE_32(REG_SP - 4);
	m68ki_write_32(REG_SP, value);
}

INLINE uint m68ki_pull_16(void)
{
	REG_SP = MASK_OUT_ABOVE_32(REG_SP + 2);
	return m68ki_read_16(REG_SP-2);
}

INLINE uint m68ki_pull_32(void)
{
	REG_SP = MASK_OUT_ABOVE_32(REG_SP + 4);
	return m68ki_read_32(REG_SP-4);
}


/* Increment/decrement the stack as if doing a push/pull but
 * don't do any memory access.
 */
INLINE void m68ki_fake_push_16(void)
{
	REG_SP = MASK_OUT_ABOVE_32(REG_SP - 2);
}

INLINE void m68ki_fake_push_32(void)
{
	REG_SP = MASK_OUT_ABOVE_32(REG_SP - 4);
}

INLINE void m68ki_fake_pull_16(void)
{
	REG_SP = MASK_OUT_ABOVE_32(REG_SP + 2);
}

INLINE void m68ki_fake_pull_32(void)
{
	REG_SP = MASK_OUT_ABOVE_32(REG_SP + 4);
}


/* ----------------------------- Program Flow ----------------------------- */

/* Jump to a new program location or vector.
 * These functions will also call the pc_changed callback if it was enabled
 * in m68kconf.h.
 */
INLINE void m68ki_jump(uint new_pc)
{
	REG_PC = new_pc;
	m68ki_pc_changed(REG_PC);
}

INLINE void m68ki_jump_vector(uint vector)
{
	REG_PC = (vector<<2) + REG_VBR;
	REG_PC = m68ki_read_data_32(REG_PC);
	m68ki_pc_changed(REG_PC);
}


/* Branch to a new memory location.
 * The 32-bit branch will call pc_changed if it was enabled in m68kconf.h.
 * So far I've found no problems with not calling pc_changed for 8 or 16
 * bit branches.
 */
INLINE void m68ki_branch_8(uint offset)
{
	REG_PC += MAKE_INT_8(offset);
}

INLINE void m68ki_branch_16(uint offset)
{
	REG_PC += MAKE_INT_16(offset);
}

INLINE void m68ki_branch_32(uint offset)
{
	REG_PC += offset;
	m68ki_pc_changed(REG_PC);
}



/* ---------------------------- Status Register --------------------------- */

/* Set the S flag and change the active stack pointer.
 * Note that value MUST be 4 or 0.
 */
INLINE void m68ki_set_s_flag(uint value)
{
	/* Backup the old stack pointer */
	REG_SP_BASE[FLAG_S | ((FLAG_S>>1) & FLAG_M)] = REG_SP;
	/* Set the S flag */
	FLAG_S = value;
	/* Set the new stack pointer */
	REG_SP = REG_SP_BASE[FLAG_S | ((FLAG_S>>1) & FLAG_M)];
}

/* Set the S and M flags and change the active stack pointer.
 * Note that value MUST be 0, 2, 4, or 6 (bit2 = S, bit1 = M).
 */
INLINE void m68ki_set_sm_flag(uint value)
{
	/* Backup the old stack pointer */
	REG_SP_BASE[FLAG_S | ((FLAG_S>>1) & FLAG_M)] = REG_SP;
	/* Set the S and M flags */
	FLAG_S = value & SFLAG_SET;
	FLAG_M = value & MFLAG_SET;
	/* Set the new stack pointer */
	REG_SP = REG_SP_BASE[FLAG_S | ((FLAG_S>>1) & FLAG_M)];
}

/* Set the S and M flags.  Don't touch the stack pointer. */
INLINE void m68ki_set_sm_flag_nosp(uint value)
{
	/* Set the S and M flags */
	FLAG_S = value & SFLAG_SET;
	FLAG_M = value & MFLAG_SET;
}


/* Set the condition code register */
INLINE void m68ki_set_ccr(uint value)
{
	FLAG_X = BIT_4(value)  << 4;
	FLAG_N = BIT_3(value)  << 4;
	FLAG_Z = !BIT_2(value);
	FLAG_V = BIT_1(value)  << 6;
	FLAG_C = BIT_0(value)  << 8;
}

/* Set the status register but don't check for interrupts */
INLINE void m68ki_set_sr_noint(uint value)
{
	/* Mask out the "unimplemented" bits */
	value &= CPU_SR_MASK;

	/* Now set the status register */
	FLAG_T1 = BIT_F(value);
	FLAG_T0 = BIT_E(value);
	FLAG_INT_MASK = value & 0x0700;
	m68ki_set_ccr(value);
	m68ki_set_sm_flag((value >> 11) & 6);
}

/* Set the status register but don't check for interrupts nor
 * change the stack pointer
 */
INLINE void m68ki_set_sr_noint_nosp(uint value)
{
	/* Mask out the "unimplemented" bits */
	value &= CPU_SR_MASK;

	/* Now set the status register */
	FLAG_T1 = BIT_F(value);
	FLAG_T0 = BIT_E(value);
	FLAG_INT_MASK = value & 0x0700;
	m68ki_set_ccr(value);
	m68ki_set_sm_flag_nosp((value >> 11) & 6);
}

/* Set the status register and check for interrupts */
INLINE void m68ki_set_sr(uint value)
{
	m68ki_set_sr_noint(value);
	m68ki_check_interrupts();
}


/* ------------------------- Exception Processing ------------------------- */

/* Initiate exception processing */
INLINE uint m68ki_init_exception(void)
{
	/* Save the old status register */
	uint sr = m68ki_get_sr();

	/* Turn off trace flag, clear pending traces */
	FLAG_T1 = FLAG_T0 = 0;
	m68ki_clear_trace();
	/* Enter supervisor mode */
	m68ki_set_s_flag(SFLAG_SET);

	return sr;
}

/* 3 word stack frame (68000 only) */
INLINE void m68ki_stack_frame_3word(uint pc, uint sr)
{
	m68ki_push_32(pc);
	m68ki_push_16(sr);
}

/* Format 0 stack frame.
 * This is the standard stack frame for 68010+.
 */
INLINE void m68ki_stack_frame_0000(uint pc, uint sr, uint vector)
{
	/* Stack a 3-word frame if we are 68000 */
	if(CPU_TYPE == CPU_TYPE_000 || CPU_TYPE == CPU_TYPE_008)
	{
		m68ki_stack_frame_3word(pc, sr);
		return;
	}
	m68ki_push_16(vector<<2);
	m68ki_push_32(pc);
	m68ki_push_16(sr);
}

/* Format 1 stack frame (68020).
 * For 68020, this is the 4 word throwaway frame.
 */
INLINE void m68ki_stack_frame_0001(uint pc, uint sr, uint vector)
{
	m68ki_push_16(0x1000 | (vector<<2));
	m68ki_push_32(pc);
	m68ki_push_16(sr);
}

/* Format 2 stack frame.
 * This is used only by 68020 for trap exceptions.
 */
INLINE void m68ki_stack_frame_0010(uint sr, uint vector)
{
	m68ki_push_32(REG_PPC);
	m68ki_push_16(0x2000 | (vector<<2));
	m68ki_push_32(REG_PC);
	m68ki_push_16(sr);
}


/* Bus error stack frame (68000 only).
 */
INLINE void m68ki_stack_frame_buserr(uint sr)
{
	m68ki_push_32(REG_PC);
	m68ki_push_16(sr);
	m68ki_push_16(REG_IR);
	m68ki_push_32(m68ki_aerr_address);	/* access address */
	/* 0 0 0 0 0 0 0 0 0 0 0 R/W I/N FC
	 * R/W  0 = write, 1 = read
	 * I/N  0 = instruction, 1 = not
	 * FC   3-bit function code
	 */
	m68ki_push_16(m68ki_aerr_write_mode | CPU_INSTR_MODE | m68ki_aerr_fc);
}

/* Format 8 stack frame (68010).
 * 68010 only.  This is the 29 word bus/address error frame.
 */
void m68ki_stack_frame_1000(uint pc, uint sr, uint vector)
{
	/* VERSION
	 * NUMBER
	 * INTERNAL INFORMATION, 16 WORDS
	 */
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();
	m68ki_fake_push_32();

	/* INSTRUCTION INPUT BUFFER */
	m68ki_push_16(0);

	/* UNUSED, RESERVED (not written) */
	m68ki_fake_push_16();

	/* DATA INPUT BUFFER */
	m68ki_push_16(0);

	/* UNUSED, RESERVED (not written) */
	m68ki_fake_push_16();

	/* DATA OUTPUT BUFFER */
	m68ki_push_16(0);

	/* UNUSED, RESERVED (not written) */
	m68ki_fake_push_16();

	/* FAULT ADDRESS */
	m68ki_push_32(0);

	/* SPECIAL STATUS WORD */
	m68ki_push_16(0);

	/* 1000, VECTOR OFFSET */
	m68ki_push_16(0x8000 | (vector<<2));

	/* PROGRAM COUNTER */
	m68ki_push_32(pc);

	/* STATUS REGISTER */
	m68ki_push_16(sr);
}

/* Format A stack frame (short bus fault).
 * This is used only by 68020 for bus fault and address error
 * if the error happens at an instruction boundary.
 * PC stacked is address of next instruction.
 */
void m68ki_stack_frame_1010(uint sr, uint vector, uint pc)
{
	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* DATA OUTPUT BUFFER (2 words) */
	m68ki_push_32(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* DATA CYCLE FAULT ADDRESS (2 words) */
	m68ki_push_32(0);

	/* INSTRUCTION PIPE STAGE B */
	m68ki_push_16(0);

	/* INSTRUCTION PIPE STAGE C */
	m68ki_push_16(0);

	/* SPECIAL STATUS REGISTER */
	m68ki_push_16(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* 1010, VECTOR OFFSET */
	m68ki_push_16(0xa000 | (vector<<2));

	/* PROGRAM COUNTER */
	m68ki_push_32(pc);

	/* STATUS REGISTER */
	m68ki_push_16(sr);
}

/* Format B stack frame (long bus fault).
 * This is used only by 68020 for bus fault and address error
 * if the error happens during instruction execution.
 * PC stacked is address of instruction in progress.
 */
void m68ki_stack_frame_1011(uint sr, uint vector, uint pc)
{
	/* INTERNAL REGISTERS (18 words) */
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);
	m68ki_push_32(0);

	/* VERSION# (4 bits), INTERNAL INFORMATION */
	m68ki_push_16(0);

	/* INTERNAL REGISTERS (3 words) */
	m68ki_push_32(0);
	m68ki_push_16(0);

	/* DATA INTPUT BUFFER (2 words) */
	m68ki_push_32(0);

	/* INTERNAL REGISTERS (2 words) */
	m68ki_push_32(0);

	/* STAGE B ADDRESS (2 words) */
	m68ki_push_32(0);

	/* INTERNAL REGISTER (4 words) */
	m68ki_push_32(0);
	m68ki_push_32(0);

	/* DATA OUTPUT BUFFER (2 words) */
	m68ki_push_32(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* DATA CYCLE FAULT ADDRESS (2 words) */
	m68ki_push_32(0);

	/* INSTRUCTION PIPE STAGE B */
	m68ki_push_16(0);

	/* INSTRUCTION PIPE STAGE C */
	m68ki_push_16(0);

	/* SPECIAL STATUS REGISTER */
	m68ki_push_16(0);

	/* INTERNAL REGISTER */
	m68ki_push_16(0);

	/* 1011, VECTOR OFFSET */
	m68ki_push_16(0xb000 | (vector<<2));

	/* PROGRAM COUNTER */
	m68ki_push_32(pc);

	/* STATUS REGISTER */
	m68ki_push_16(sr);
}


/* Used for Group 2 exceptions.
 * These stack a type 2 frame on the 020.
 */
INLINE void m68ki_exception_trap(uint vector)
{
	uint sr = m68ki_init_exception();

	if(CPU_TYPE_IS_010_LESS(CPU_TYPE))
		m68ki_stack_frame_0000(REG_PC, sr, vector);
	else
		m68ki_stack_frame_0010(sr, vector);

	m68ki_jump_vector(vector);

	/* Use up some clock cycles */
	USE_CYCLES(CYC_EXCEPTION[vector]);
}

/* Trap#n stacks a 0 frame but behaves like group2 otherwise */
INLINE void m68ki_exception_trapN(uint vector)
{
	uint sr = m68ki_init_exception();
	m68ki_stack_frame_0000(REG_PC, sr, vector);
	m68ki_jump_vector(vector);

	/* Use up some clock cycles */
	USE_CYCLES(CYC_EXCEPTION[vector]);
}

/* Exception for trace mode */
INLINE void m68ki_exception_trace(void)
{
	uint sr = m68ki_init_exception();

	if(CPU_TYPE_IS_010_LESS(CPU_TYPE))
	{
		#if M68K_EMULATE_ADDRESS_ERROR == OPT_ON
		if(CPU_TYPE_IS_000(CPU_TYPE))
		{
			CPU_INSTR_MODE = INSTRUCTION_NO;
		}
		#endif /* M68K_EMULATE_ADDRESS_ERROR */
		m68ki_stack_frame_0000(REG_PC, sr, EXCEPTION_TRACE);
	}
	else
		m68ki_stack_frame_0010(sr, EXCEPTION_TRACE);

	m68ki_jump_vector(EXCEPTION_TRACE);

	/* Trace nullifies a STOP instruction */
	CPU_STOPPED &= ~STOP_LEVEL_STOP;

	/* Use up some clock cycles */
	USE_CYCLES(CYC_EXCEPTION[EXCEPTION_TRACE]);
}

/* Exception for privilege violation */
INLINE void m68ki_exception_privilege_violation(void)
{
	uint sr = m68ki_init_exception();

	#if M68K_EMULATE_ADDRESS_ERROR == OPT_ON
	if(CPU_TYPE_IS_000(CPU_TYPE))
	{
		CPU_INSTR_MODE = INSTRUCTION_NO;
	}
	#endif /* M68K_EMULATE_ADDRESS_ERROR */

	m68ki_stack_frame_0000(REG_PPC, sr, EXCEPTION_PRIVILEGE_VIOLATION);
	m68ki_jump_vector(EXCEPTION_PRIVILEGE_VIOLATION);

	/* Use up some clock cycles and undo the instruction's cycles */
	USE_CYCLES(CYC_EXCEPTION[EXCEPTION_PRIVILEGE_VIOLATION] - CYC_INSTRUCTION[REG_IR]);
}

/* Exception for A-Line instructions */
INLINE void m68ki_exception_1010(void)
{
	uint sr;
#if M68K_LOG_1010_1111 == OPT_ON
	M68K_DO_LOG_EMU((M68K_LOG_FILEHANDLE "%s at %08x: called 1010 instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PPC), REG_IR,
					 m68ki_disassemble_quick(ADDRESS_68K(REG_PPC))));
#endif

	sr = m68ki_init_exception();
	m68ki_stack_frame_0000(REG_PPC, sr, EXCEPTION_1010);
	m68ki_jump_vector(EXCEPTION_1010);

	/* Use up some clock cycles and undo the instruction's cycles */
	USE_CYCLES(CYC_EXCEPTION[EXCEPTION_1010] - CYC_INSTRUCTION[REG_IR]);
}

/* Exception for F-Line instructions */
INLINE void m68ki_exception_1111(void)
{
	uint sr;

#if M68K_LOG_1010_1111 == OPT_ON
	M68K_DO_LOG_EMU((M68K_LOG_FILEHANDLE "%s at %08x: called 1111 instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PPC), REG_IR,
					 m68ki_disassemble_quick(ADDRESS_68K(REG_PPC))));
#endif

	sr = m68ki_init_exception();
	m68ki_stack_frame_0000(REG_PPC, sr, EXCEPTION_1111);
	m68ki_jump_vector(EXCEPTION_1111);

	/* Use up some clock cycles and undo the instruction's cycles */
	USE_CYCLES(CYC_EXCEPTION[EXCEPTION_1111] - CYC_INSTRUCTION[REG_IR]);
}

/* Exception for illegal instructions */
INLINE void m68ki_exception_illegal(void)
{
	uint sr;

	M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: illegal instruction %04x (%s)\n",
				 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PPC), REG_IR,
				 m68ki_disassemble_quick(ADDRESS_68K(REG_PPC))));

	sr = m68ki_init_exception();

	#if M68K_EMULATE_ADDRESS_ERROR == OPT_ON
	if(CPU_TYPE_IS_000(CPU_TYPE))
	{
		CPU_INSTR_MODE = INSTRUCTION_NO;
	}
	#endif /* M68K_EMULATE_ADDRESS_ERROR */

	m68ki_stack_frame_0000(REG_PPC, sr, EXCEPTION_ILLEGAL_INSTRUCTION);
	m68ki_jump_vector(EXCEPTION_ILLEGAL_INSTRUCTION);

	/* Use up some clock cycles and undo the instruction's cycles */
	USE_CYCLES(CYC_EXCEPTION[EXCEPTION_ILLEGAL_INSTRUCTION] - CYC_INSTRUCTION[REG_IR]);
}

/* Exception for format errror in RTE */
INLINE void m68ki_exception_format_error(void)
{
	uint sr = m68ki_init_exception();
	m68ki_stack_frame_0000(REG_PC, sr, EXCEPTION_FORMAT_ERROR);
	m68ki_jump_vector(EXCEPTION_FORMAT_ERROR);

	/* Use up some clock cycles and undo the instruction's cycles */
	USE_CYCLES(CYC_EXCEPTION[EXCEPTION_FORMAT_ERROR] - CYC_INSTRUCTION[REG_IR]);
}

/* Exception for address error */
INLINE void m68ki_exception_address_error(void)
{
	uint sr = m68ki_init_exception();

	/* If we were processing a bus error, address error, or reset,
	 * this is a catastrophic failure.
	 * Halt the CPU
	 */
	if(CPU_RUN_MODE == RUN_MODE_BERR_AERR_RESET)
	{
m68k_read_memory_8(0x00ffff01);
		CPU_STOPPED = STOP_LEVEL_HALT;
		return;
	}
	CPU_RUN_MODE = RUN_MODE_BERR_AERR_RESET;

	/* Note: This is implemented for 68000 only! */
	m68ki_stack_frame_buserr(sr);

	m68ki_jump_vector(EXCEPTION_ADDRESS_ERROR);

	/* Use up some clock cycles and undo the instruction's cycles */
	USE_CYCLES(CYC_EXCEPTION[EXCEPTION_ADDRESS_ERROR] - CYC_INSTRUCTION[REG_IR]);
}


/* Service an interrupt request and start exception processing */
void m68ki_exception_interrupt(uint int_level)
{
	uint vector;
	uint sr;
	uint new_pc;

	#if M68K_EMULATE_ADDRESS_ERROR == OPT_ON
	if(CPU_TYPE_IS_000(CPU_TYPE))
	{
		CPU_INSTR_MODE = INSTRUCTION_NO;
	}
	#endif /* M68K_EMULATE_ADDRESS_ERROR */

	/* Turn off the stopped state */
	CPU_STOPPED &= ~STOP_LEVEL_STOP;

	/* If we are halted, don't do anything */
	if(CPU_STOPPED)
		return;

	/* Acknowledge the interrupt */
	vector = m68ki_int_ack(int_level);

	/* Get the interrupt vector */
	if(vector == M68K_INT_ACK_AUTOVECTOR)
		/* Use the autovectors.  This is the most commonly used implementation */
		vector = EXCEPTION_INTERRUPT_AUTOVECTOR+int_level;
	else if(vector == M68K_INT_ACK_SPURIOUS)
		/* Called if no devices respond to the interrupt acknowledge */
		vector = EXCEPTION_SPURIOUS_INTERRUPT;
	else if(vector > 255)
	{
		M68K_DO_LOG_EMU((M68K_LOG_FILEHANDLE "%s at %08x: Interrupt acknowledge returned invalid vector $%x\n",
				 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC), vector));
		return;
	}

	/* Start exception processing */
	sr = m68ki_init_exception();

	/* Set the interrupt mask to the level of the one being serviced */
	FLAG_INT_MASK = int_level<<8;

	/* Get the new PC */
	new_pc = m68ki_read_data_32((vector<<2) + REG_VBR);

	/* If vector is uninitialized, call the uninitialized interrupt vector */
	if(new_pc == 0)
		new_pc = m68ki_read_data_32((EXCEPTION_UNINITIALIZED_INTERRUPT<<2) + REG_VBR);

	/* Generate a stack frame */
	m68ki_stack_frame_0000(REG_PC, sr, vector);
	if(FLAG_M && CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		/* Create throwaway frame */
		m68ki_set_sm_flag(FLAG_S);	/* clear M */
		sr |= 0x2000; /* Same as SR in master stack frame except S is forced high */
		m68ki_stack_frame_0001(REG_PC, sr, vector);
	}

	m68ki_jump(new_pc);

	/* Defer cycle counting until later */
	CPU_INT_CYCLES += CYC_EXCEPTION[vector];

#if !M68K_EMULATE_INT_ACK
	/* Automatically clear IRQ if we are not using an acknowledge scheme */
	CPU_INT_LEVEL = 0;
#endif /* M68K_EMULATE_INT_ACK */
}


/* ASG: Check for interrupts */
INLINE void m68ki_check_interrupts(void)
{
	if(CPU_INT_LEVEL > FLAG_INT_MASK)
		m68ki_exception_interrupt(CPU_INT_LEVEL>>8);
}



/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* M68KCPU__HEADER */
