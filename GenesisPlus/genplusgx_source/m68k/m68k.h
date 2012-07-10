#ifndef M68K__HEADER
#define M68K__HEADER

/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */
/*
 *                                  MUSASHI
 *                                Version 3.32
 *
 * A portable Motorola M680x0 processor emulation engine.
 * Copyright Karl Stenerud.  All rights reserved.
 *
 * This code may be freely used for non-commercial purposes as long as this
 * copyright notice remains unaltered in the source code and any binary files
 * containing this code in compiled form.
 *
 * All other licensing terms must be negotiated with the author
 * (Karl Stenerud).
 *
 * The latest version of this code can be obtained at:
 * http://kstenerud.cjb.net
 */

 /* Modified by Eke-Eke for Genesis Plus GX:

    - removed unused stuff to reduce memory usage / optimize execution (multiple CPU types support, NMI support, ...)
    - moved stuff to compile statically in a single object file
    - implemented support for global cycle count (shared by 68k & Z80 CPU)
    - added support for interrupt latency (Sesame's Street Counting Cafe, Fatal Rewind)
    - added proper cycle use on reset
    - added cycle accurate timings for MUL/DIV instructions (thanks to Jorge Cwik !) 
    - fixed undocumented flags for DIV instructions (Blood Shot)
    - added MAIN-CPU & SUB-CPU support for Mega CD emulation
    
  */

/* ======================================================================== */
/* ================================ INCLUDES ============================== */
/* ======================================================================== */

#include "macros.h"


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

#define sint8  signed   char      /* ASG: changed from char to signed char */
#define sint16 signed   short
#define sint32 signed   int      /* AWJ: changed from long to int */
#define uint8  unsigned char
#define uint16 unsigned short
#define uint32 unsigned int      /* AWJ: changed from long to int */

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
/*#if UCHAR_MAX == 0xff*/
  #define MAKE_INT_8(A) (sint8)(A)
/*#else
  #undef  sint8
  #define sint8  signed   int
  #undef  uint8
  #define uint8  unsigned int
  INLINE sint MAKE_INT_8(uint value)
  {
    return (value & 0x80) ? value | ~0xff : value & 0xff;
  }*/
/*#endif *//* UCHAR_MAX == 0xff */


/* Allow for architectures that don't have 16-bit sizes */
/*#if USHRT_MAX == 0xffff*/
  #define MAKE_INT_16(A) (sint16)(A)
/*#else
  #undef  sint16
  #define sint16 signed   int
  #undef  uint16
  #define uint16 unsigned int
  INLINE sint MAKE_INT_16(uint value)
  {
    return (value & 0x8000) ? value | ~0xffff : value & 0xffff;
  }*/
/*#endif *//* USHRT_MAX == 0xffff */


/* Allow for architectures that don't have 32-bit sizes */
/*#if UINT_MAX == 0xffffffff*/
  #define MAKE_INT_32(A) (sint32)(A)
/*#else
  #undef  sint32
  #define sint32  signed   int
  #undef  uint32
  #define uint32  unsigned int
  INLINE sint MAKE_INT_32(uint value)
  {
    return (value & 0x80000000) ? value | ~0xffffffff : value & 0xffffffff;
  }*/
/*#endif *//* UINT_MAX == 0xffffffff */



/* ======================================================================== */
/* ============================ GENERAL DEFINES =========================== */

/* ======================================================================== */

/* There are 7 levels of interrupt to the 68K.
 * A transition from < 7 to 7 will cause a non-maskable interrupt (NMI).
 */
#define M68K_IRQ_NONE 0
#define M68K_IRQ_1    1
#define M68K_IRQ_2    2
#define M68K_IRQ_3    3
#define M68K_IRQ_4    4
#define M68K_IRQ_5    5
#define M68K_IRQ_6    6
#define M68K_IRQ_7    7


/* Special interrupt acknowledge values.
 * Use these as special returns from the interrupt acknowledge callback
 * (specified later in this header).
 */

/* Causes an interrupt autovector (0x18 + interrupt level) to be taken.
 * This happens in a real 68K if VPA or AVEC is asserted during an interrupt
 * acknowledge cycle instead of DTACK.
 */
#define M68K_INT_ACK_AUTOVECTOR    0xffffffff

/* Causes the spurious interrupt vector (0x18) to be taken
 * This happens in a real 68K if BERR is asserted during the interrupt
 * acknowledge cycle (i.e. no devices responded to the acknowledge).
 */
#define M68K_INT_ACK_SPURIOUS      0xfffffffe


/* Registers used by m68k_get_reg() and m68k_set_reg() */
typedef enum
{
  /* Real registers */
  M68K_REG_D0,    /* Data registers */
  M68K_REG_D1,
  M68K_REG_D2,
  M68K_REG_D3,
  M68K_REG_D4,
  M68K_REG_D5,
  M68K_REG_D6,
  M68K_REG_D7,
  M68K_REG_A0,    /* Address registers */
  M68K_REG_A1,
  M68K_REG_A2,
  M68K_REG_A3,
  M68K_REG_A4,
  M68K_REG_A5,
  M68K_REG_A6,
  M68K_REG_A7,
  M68K_REG_PC,    /* Program Counter */
  M68K_REG_SR,    /* Status Register */
  M68K_REG_SP,    /* The current Stack Pointer (located in A7) */
  M68K_REG_USP,   /* User Stack Pointer */
  M68K_REG_ISP,   /* Interrupt Stack Pointer */

#if M68K_EMULATE_PREFETCH
  /* Assumed registers */
  /* These are cheat registers which emulate the 1-longword prefetch
   * present in the 68000 and 68010.
   */
  M68K_REG_PREF_ADDR,  /* Last prefetch address */
  M68K_REG_PREF_DATA,  /* Last prefetch data */
#endif

  /* Convenience registers */
  M68K_REG_IR    /* Instruction register */
} m68k_register_t;


/* 68k memory map structure */
typedef struct 
{
  unsigned char *base;                             /* memory-based access (ROM, RAM) */
  unsigned int (*read8)(unsigned int address);               /* I/O byte read access */
  unsigned int (*read16)(unsigned int address);              /* I/O word read access */
  void (*write8)(unsigned int address, unsigned int data);  /* I/O byte write access */
  void (*write16)(unsigned int address, unsigned int data); /* I/O word write access */
} cpu_memory_map;

/* 68k idle loop detection */
typedef struct
{
  uint pc;
  uint cycle;
  uint detected;
} cpu_idle_t;

typedef struct
{
  cpu_memory_map memory_map[256]; /* memory mapping */

  cpu_idle_t poll;    /* polling detection */

  uint cycles;        /* current master cycle count */ 
  uint cycle_end;     /* aimed master cycle count for current execution frame */

  uint dar[16];      /* Data and Address Registers */
  uint pc;           /* Program Counter */
  uint sp[5];        /* User and Interrupt Stack Pointers */
  uint ir;           /* Instruction Register */
  uint t1_flag;      /* Trace 1 */
  uint s_flag;       /* Supervisor */
  uint x_flag;       /* Extend */
  uint n_flag;       /* Negative */
  uint not_z_flag;   /* Zero, inverted for speedups */
  uint v_flag;       /* Overflow */
  uint c_flag;       /* Carry */
  uint int_mask;     /* I0-I2 */
  uint int_level;    /* State of interrupt pins IPL0-IPL2 -- ASG: changed from ints_pending */
  uint stopped;      /* Stopped state */
#if M68K_EMULATE_PREFETCH
  uint pref_addr;    /* Last prefetch address */
  uint pref_data;    /* Data in the prefetch queue */
#endif
#if M68K_EMULATE_ADDRESS_ERROR
  uint instr_mode;      /* Stores whether we are in instruction mode or group 0/1 exception mode */
  uint run_mode;        /* Stores whether we are processing a reset, bus error, address error, or something else */
  uint aerr_enabled;    /* Enables/deisables address error checks at runtime */
  jmp_buf aerr_trap;    /* Address error jump */
  uint aerr_address;    /* Address error location */
  uint aerr_write_mode; /* Address error write mode */
  uint aerr_fc;         /* Address error FC code */
#endif
#if M68K_EMULATE_TRACE
  uint tracing;         /* Tracing enable flag */
#endif 
#if M68K_EMULATE_FC
  uint address_space;   /* Current FC code */
#endif

  /* Callbacks to host */
#if M68K_EMULATE_INT_ACK == OPT_ON
  int  (*int_ack_callback)(int int_line);           /* Interrupt Acknowledge */
#endif
#if M68K_EMULATE_RESET == OPT_ON
  void (*reset_instr_callback)(void);               /* Called when a RESET instruction is encountered */
#endif
#if M68K_TAS_HAS_CALLBACK == OPT_ON
  int  (*tas_instr_callback)(void);                 /* Called when a TAS instruction is encountered, allows / disallows writeback */
#endif
#if M68K_EMULATE_FC == OPT_ON
  void (*set_fc_callback)(unsigned int new_fc);     /* Called when the CPU function code changes */
#endif
} m68ki_cpu_core;

/* CPU cores */
extern m68ki_cpu_core m68k;
extern m68ki_cpu_core s68k;


/* ======================================================================== */
/* ============================== CALLBACKS =============================== */
/* ======================================================================== */

/* These functions allow you to set callbacks to the host when specific events
 * occur.  Note that you must enable the corresponding value in m68kconf.h
 * in order for these to do anything useful.
 * Note: I have defined default callbacks which are used if you have enabled
 * the corresponding #define in m68kconf.h but either haven't assigned a
 * callback or have assigned a callback of NULL.
 */

#if M68K_EMULATE_INT_ACK == OPT_ON
/* Set the callback for an interrupt acknowledge.
 * You must enable M68K_EMULATE_INT_ACK in m68kconf.h.
 * The CPU will call the callback with the interrupt level being acknowledged.
 * The host program must return either a vector from 0x02-0xff, or one of the
 * special interrupt acknowledge values specified earlier in this header.
 * If this is not implemented, the CPU will always assume an autovectored
 * interrupt, and will automatically clear the interrupt request when it
 * services the interrupt.
 * Default behavior: return M68K_INT_ACK_AUTOVECTOR.
 */
void m68k_set_int_ack_callback(int  (*callback)(int int_level));
#endif

#if M68K_EMULATE_RESET == OPT_ON
/* Set the callback for the RESET instruction.
 * You must enable M68K_EMULATE_RESET in m68kconf.h.
 * The CPU calls this callback every time it encounters a RESET instruction.
 * Default behavior: do nothing.
 */
void m68k_set_reset_instr_callback(void  (*callback)(void));
#endif

#if M68K_TAS_HAS_CALLBACK == OPT_ON
/* Set the callback for the TAS instruction.
 * You must enable M68K_TAS_HAS_CALLBACK in m68kconf.h.
 * The CPU calls this callback every time it encounters a TAS instruction.
 * Default behavior: return 1, allow writeback.
 */
void m68k_set_tas_instr_callback(int  (*callback)(void));
#endif

#if M68K_EMULATE_FC == OPT_ON
/* Set the callback for CPU function code changes.
 * You must enable M68K_EMULATE_FC in m68kconf.h.
 * The CPU calls this callback with the function code before every memory
 * access to set the CPU's function code according to what kind of memory
 * access it is (supervisor/user, program/data and such).
 * Default behavior: do nothing.
 */
void m68k_set_fc_callback(void  (*callback)(unsigned int new_fc));
#endif


/* ======================================================================== */
/* ====================== FUNCTIONS TO ACCESS THE CPU ===================== */
/* ======================================================================== */

/* Do whatever initialisations the core requires.  Should be called
 * at least once at init time.
 */
extern void m68k_init(void);
extern void s68k_init(void);

/* Pulse the RESET pin on the CPU.
 * You *MUST* reset the CPU at least once to initialize the emulation
 */
extern void m68k_pulse_reset(void);
extern void s68k_pulse_reset(void);

/* Run until given cycle count is reached */
extern void m68k_run(unsigned int cycles);
extern void s68k_run(unsigned int cycles);

/* Set the IPL0-IPL2 pins on the CPU (IRQ).
 * A transition from < 7 to 7 will cause a non-maskable interrupt (NMI).
 * Setting IRQ to 0 will clear an interrupt request.
 */
extern void m68k_set_irq(unsigned int int_level);
extern void m68k_set_irq_delay(unsigned int int_level);
extern void m68k_update_irq(unsigned int mask);
extern void s68k_update_irq(unsigned int mask);

/* Halt the CPU as if you pulsed the HALT pin. */
extern void m68k_pulse_halt(void);
extern void m68k_clear_halt(void);
extern void s68k_pulse_halt(void);
extern void s68k_clear_halt(void);


/* Peek at the internals of a CPU context.  This can either be a context
 * retrieved using m68k_get_context() or the currently running context.
 * If context is NULL, the currently running CPU context will be used.
 */
extern unsigned int m68k_get_reg(m68k_register_t reg);
extern unsigned int s68k_get_reg(m68k_register_t reg);

/* Poke values into the internals of the currently running CPU context */
extern void m68k_set_reg(m68k_register_t reg, unsigned int value);
extern void s68k_set_reg(m68k_register_t reg, unsigned int value);


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* M68K__HEADER */
