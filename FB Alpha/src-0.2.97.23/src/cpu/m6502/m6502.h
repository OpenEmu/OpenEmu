/*****************************************************************************
 *
 *   m6502.h
 *   Portable 6502/65c02/65sc02/6510/n2a03 emulator interface
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   65sc02 core Copyright Peter Trauner.
 *   Deco16 portions Copyright Bryan McPhail.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/
/* 2.February 2000 PeT added 65sc02 subtype */

#pragma once

#ifndef __M6502_H__
#define __M6502_H__

//#include "cpuintrf.h"

#define HAS_M65C02		1
#define HAS_M6510		1
#define HAS_N2A03		1
#define HAS_M65SC02		1
#define HAS_DECO16		1
#define HAS_M6510T		1
#define HAS_M7501		1
#define HAS_M8502		1

typedef struct
{
	UINT8	subtype;		/* currently selected cpu sub type */
//	void	(*const *insn)(void); /* pointer to the function pointer table */
	PAIR	ppc;			/* previous program counter */
	PAIR	pc; 			/* program counter */
	PAIR	sp; 			/* stack pointer (always 100 - 1FF) */
	PAIR	zp; 			/* zero page address */
	PAIR	ea; 			/* effective address */
	UINT8	a;				/* Accumulator */
	UINT8	x;				/* X index register */
	UINT8	y;				/* Y index register */
	UINT8	p;				/* Processor status */
	UINT8	pending_irq;	/* nonzero if an IRQ is pending */
	UINT8	after_cli;		/* pending IRQ and last insn cleared I */
	UINT8	nmi_state;
	UINT8	irq_state;
	UINT8   so_state;
	int 	(*irq_callback)(int irqline);	/* IRQ callback */
//	read8_machine_func rdmem_id;					/* readmem callback for indexed instructions */
//	write8_machine_func wrmem_id;				/* writemem callback for indexed instructions */

#if (HAS_M6510) || (HAS_M6510T) || (HAS_M8502) || (HAS_M7501)
	UINT8    ddr;
	UINT8    port;
	UINT8	(*port_read)(UINT8 direction);
	void	(*port_write)(UINT8 direction, UINT8 data);
#endif

}	m6502_Regs;

#define M6502_CLEAR_LINE		0
#define M6502_ASSERT_LINE		1
#define M6502_INPUT_LINE_NMI	32

/* set to 1 to test cur_mrhard/cur_wmhard to avoid calls */
#define FAST_MEMORY 0

#define SUBTYPE_6502	0
#if (HAS_M65C02)
#define SUBTYPE_65C02	1
#endif
#if (HAS_M6510)
#define SUBTYPE_6510	2
#endif
#if (HAS_N2A03)
#define SUBTYPE_2A03	3
#endif
#if (HAS_M65SC02)
#define SUBTYPE_65SC02	4
#endif
#if (HAS_DECO16)
#define SUBTYPE_DECO16	5
#endif

enum
{
	M6502_PC=1, M6502_S, M6502_P, M6502_A, M6502_X, M6502_Y,
	M6502_EA, M6502_ZP,
	M6502_SUBTYPE
};

#define M6502_IRQ_LINE		0
/* use cpunum_set_input_line(machine, cpu, M6502_SET_OVERFLOW, level)
   to change level of the so input line
   positiv edge sets overflow flag */
#define M6502_SET_OVERFLOW	1

/*typedef unsigned char (*M6502ReadIoHandler)(unsigned short a);
typedef void (*M6502WriteIoHandler)(unsigned short a, unsigned char v);
typedef unsigned char (*M6502ReadProgHandler)(unsigned short a);
typedef void (*M6502WriteProgHandler)(unsigned short a, unsigned char v);
typedef unsigned char (*M6502ReadMemIndexHandler)(unsigned short a);
typedef void (*M6502WriteMemIndexHandler)(unsigned short a, unsigned char v);
typedef unsigned char (*M6502ReadOpHandler)(unsigned short a);
typedef unsigned char (*M6502ReadOpArgHandler)(unsigned short a);*/

unsigned char M6502ReadPort(unsigned short Address);
void M6502WritePort(unsigned short Address, unsigned char Data);
unsigned char M6502ReadByte(unsigned short Address);
void M6502WriteByte(unsigned short Address, unsigned char Data);
unsigned char M6502ReadMemIndex(unsigned short Address);
void M6502WriteMemIndex(unsigned short Address, unsigned char Data);
unsigned char M6502ReadOp(unsigned short Address);
unsigned char M6502ReadOpArg(unsigned short Address);

void m6502_init();
void m6502_reset(void);
void m6502_exit(void);
void m6502_get_context (void *dst);
void m6502_set_context (void *src);
int m6502_execute(int cycles);
void m6502_set_irq_line(int irqline, int state);
UINT32 m6502_get_pc();

void m65c02_init();
void m65c02_reset();
int m65c02_execute(int cycles);
void m65c02_set_irq_line(int irqline, int state);

void m65sc02_init();

void n2a03_init();
void n2a03_irq(void); // used for PSG!!

void deco16_init();
void deco16_reset();
int deco16_execute(int cycles);
void deco16_set_irq_line(int irqline, int state);

void m6510_init();
void m6510_reset(void);

/*enum
{
	CPUINFO_PTR_M6502_READINDEXED_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC,
	CPUINFO_PTR_M6502_WRITEINDEXED_CALLBACK,
	CPUINFO_PTR_M6510_PORTREAD,
	CPUINFO_PTR_M6510_PORTWRITE,

	CPUINFO_INT_M6510_PORT = CPUINFO_INT_CPU_SPECIFIC
};*/

//extern void m6502_get_info(UINT32 state, cpuinfo *info);

//extern unsigned m6502_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

/****************************************************************************
 * The 6510
 ****************************************************************************/
#if (HAS_M6510)
#define M6510_A 						M6502_A
#define M6510_X 						M6502_X
#define M6510_Y 						M6502_Y
#define M6510_S 						M6502_S
#define M6510_PC						M6502_PC
#define M6510_P 						M6502_P
#define M6510_EA						M6502_EA
#define M6510_ZP						M6502_ZP
#define M6510_NMI_STATE 				M6502_NMI_STATE
#define M6510_IRQ_STATE 				M6502_IRQ_STATE

#define M6510_IRQ_LINE					M6502_IRQ_LINE

//extern void m6510_get_info(UINT32 state, cpuinfo *info);

//extern unsigned m6510_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#endif

#if (HAS_M6510T)
#define M6510T_A						M6502_A
#define M6510T_X						M6502_X
#define M6510T_Y						M6502_Y
#define M6510T_S						M6502_S
#define M6510T_PC						M6502_PC
#define M6510T_P						M6502_P
#define M6510T_EA						M6502_EA
#define M6510T_ZP						M6502_ZP
#define M6510T_NMI_STATE				M6502_NMI_STATE
#define M6510T_IRQ_STATE				M6502_IRQ_STATE

#define M6510T_IRQ_LINE					M6502_IRQ_LINE

//extern void m6510t_get_info(UINT32 state, cpuinfo *info);

#endif

#if (HAS_M7501)
#define M7501_A 						M6502_A
#define M7501_X 						M6502_X
#define M7501_Y 						M6502_Y
#define M7501_S 						M6502_S
#define M7501_PC						M6502_PC
#define M7501_P 						M6502_P
#define M7501_EA						M6502_EA
#define M7501_ZP						M6502_ZP
#define M7501_NMI_STATE 				M6502_NMI_STATE
#define M7501_IRQ_STATE 				M6502_IRQ_STATE

#define M7501_IRQ_LINE					M6502_IRQ_LINE

//extern void m7501_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_M8502)
#define M8502_A 						M6502_A
#define M8502_X 						M6502_X
#define M8502_Y 						M6502_Y
#define M8502_S 						M6502_S
#define M8502_PC						M6502_PC
#define M8502_P 						M6502_P
#define M8502_EA						M6502_EA
#define M8502_ZP						M6502_ZP
#define M8502_NMI_STATE 				M6502_NMI_STATE
#define M8502_IRQ_STATE 				M6502_IRQ_STATE

#define M8502_IRQ_LINE					M6502_IRQ_LINE

//extern void m8502_get_info(UINT32 state, cpuinfo *info);
#endif


/****************************************************************************
 * The 2A03 (NES 6502 without decimal mode ADC/SBC)
 ****************************************************************************/
#if (HAS_N2A03)
#define N2A03_A 						M6502_A
#define N2A03_X 						M6502_X
#define N2A03_Y 						M6502_Y
#define N2A03_S 						M6502_S
#define N2A03_PC						M6502_PC
#define N2A03_P 						M6502_P
#define N2A03_EA						M6502_EA
#define N2A03_ZP						M6502_ZP
#define N2A03_NMI_STATE 				M6502_NMI_STATE
#define N2A03_IRQ_STATE 				M6502_IRQ_STATE

#define N2A03_IRQ_LINE					M6502_IRQ_LINE

//extern void n2a03_get_info(UINT32 state, cpuinfo *info);

#define N2A03_DEFAULTCLOCK (21477272.724 / 12)

/* The N2A03 is integrally tied to its PSG (they're on the same die).
   Bit 7 of address $4011 (the PSG's DPCM control register), when set,
   causes an IRQ to be generated.  This function allows the IRQ to be called
   from the PSG core when such an occasion arises. */
//extern void n2a03_irq(void);
#endif


/****************************************************************************
 * The 65C02
 ****************************************************************************/
#if (HAS_M65C02)
#define M65C02_A						M6502_A
#define M65C02_X						M6502_X
#define M65C02_Y						M6502_Y
#define M65C02_S						M6502_S
#define M65C02_PC						M6502_PC
#define M65C02_P						M6502_P
#define M65C02_EA						M6502_EA
#define M65C02_ZP						M6502_ZP
#define M65C02_NMI_STATE				M6502_NMI_STATE
#define M65C02_IRQ_STATE				M6502_IRQ_STATE

#define M65C02_IRQ_LINE					M6502_IRQ_LINE

//extern void m65c02_get_info(UINT32 state, cpuinfo *info);

//extern unsigned m65c02_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#endif

/****************************************************************************
 * The 65SC02
 ****************************************************************************/
#if (HAS_M65SC02)
#define M65SC02_A						M6502_A
#define M65SC02_X						M6502_X
#define M65SC02_Y						M6502_Y
#define M65SC02_S						M6502_S
#define M65SC02_PC						M6502_PC
#define M65SC02_P						M6502_P
#define M65SC02_EA						M6502_EA
#define M65SC02_ZP						M6502_ZP
#define M65SC02_NMI_STATE				M6502_NMI_STATE
#define M65SC02_IRQ_STATE				M6502_IRQ_STATE

#define M65SC02_IRQ_LINE				M6502_IRQ_LINE

//extern void m65sc02_get_info(UINT32 state, cpuinfo *info);

//extern unsigned m65sc02_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

/****************************************************************************
 * The DECO CPU16
 ****************************************************************************/
#if (HAS_DECO16)
#define DECO16_A						M6502_A
#define DECO16_X						M6502_X
#define DECO16_Y						M6502_Y
#define DECO16_S						M6502_S
#define DECO16_PC						M6502_PC
#define DECO16_P						M6502_P
#define DECO16_EA						M6502_EA
#define DECO16_ZP						M6502_ZP
#define DECO16_NMI_STATE				M6502_NMI_STATE
#define DECO16_IRQ_STATE				M6502_IRQ_STATE

#define DECO16_IRQ_LINE					M6502_IRQ_LINE

//extern void deco16_get_info(UINT32 state, cpuinfo *info);
#endif

//extern unsigned deco16_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#endif /* __M6502_H__ */
