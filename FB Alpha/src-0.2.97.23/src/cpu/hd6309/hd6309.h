/*** hd6309: Portable 6309 emulator ******************************************/

//#pragma once

#ifndef __HD6309_H__
#define __HD6309_H__

//#include "cpuintrf.h"

enum
{
	HD6309_PC=1, HD6309_S, HD6309_CC ,HD6309_A, HD6309_B, HD6309_U, HD6309_X, HD6309_Y, HD6309_DP,
	HD6309_E, HD6309_F, HD6309_V, HD6309_MD
};

#define HD6309_IRQ_LINE 0	/* IRQ line number */
#define HD6309_FIRQ_LINE 1	 /* FIRQ line number */
#define HD6309_INPUT_LINE_NMI	32

/* 6309 Registers */
typedef struct
{
	PAIR	pc; 		/* Program counter */
	PAIR	ppc;		/* Previous program counter */
	PAIR	d,w;		/* Accumlator d and w (ab = d, ef = w, abef = q) */
	PAIR	dp; 		/* Direct Page register (page in MSB) */
	PAIR	u, s;		/* Stack pointers */
	PAIR	x, y;		/* Index registers */
	PAIR	v;			/* New 6309 register */
	UINT8	cc;
	UINT8	md; 		/* Special mode register */
	UINT8	ireg;		/* First opcode */
	UINT8	irq_state[2];
	int 	extra_cycles; /* cycles used up by interrupts */
	int 	(*irq_callback)(int irqline);
	UINT8	int_state;	/* SYNC and CWAI flags */
	UINT8	nmi_state;
} hd6309_Regs;

unsigned char HD6309ReadByte(unsigned short Address);
void HD6309WriteByte(unsigned short Address, unsigned char Data);
unsigned char HD6309ReadOp(unsigned short Address);
unsigned char HD6309ReadOpArg(unsigned short Address);

void hd6309_init();
void hd6309_reset(void);
int hd6309_get_pc();
int hd6309_execute(int cycles);
void hd6309_set_irq_line(int irqline, int state);
void hd6309_get_context(void *dst);
void hd6309_set_context(void *src);


/* PUBLIC FUNCTIONS */
//void hd6309_get_info(UINT32 state, cpuinfo *info);

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
/* ASG 971005 -- changed to program_read_byte_8/cpu_writemem16 */
#define HD6309_RDMEM(Addr) ((unsigned)HD6309ReadByte(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define HD6309_WRMEM(Addr,Value) (HD6309WriteByte(Addr,Value))

/****************************************************************************/
/* Z80_RDOP() is identical to Z80_RDMEM() except it is used for reading     */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define HD6309_RDOP(Addr) ((unsigned)HD6309ReadOp(Addr))

/****************************************************************************/
/* Z80_RDOP_ARG() is identical to Z80_RDOP() except it is used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define HD6309_RDOP_ARG(Addr) ((unsigned)HD6309ReadOpArg(Addr))

#ifndef FALSE
#	 define FALSE 0
#endif
#ifndef TRUE
#	 define TRUE (!FALSE)
#endif

#endif /* __HD6309_H__ */
