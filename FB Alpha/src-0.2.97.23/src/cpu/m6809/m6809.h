/*** m6809: Portable 6809 emulator ******************************************/

//#pragma once

#ifndef __M6809_H__
#define __M6809_H__

//#include "cpuintrf.h"

/* 6809 Registers */
typedef struct
{
	PAIR	pc; 		/* Program counter */
	PAIR	ppc;		/* Previous program counter */
	PAIR	d;			/* Accumulator a and b */
	PAIR	dp; 		/* Direct Page register (page in MSB) */
	PAIR	u, s;		/* Stack pointers */
	PAIR	x, y;		/* Index registers */
    UINT8   cc;
	UINT8	ireg;		/* First opcode */
	UINT8	irq_state[2];
    int     extra_cycles; /* cycles used up by interrupts */
    int     (*irq_callback)(int irqline);
    UINT8   int_state;  /* SYNC and CWAI flags */
    UINT8   nmi_state;
} m6809_Regs;

enum
{
	M6809_PC=1, M6809_S, M6809_CC ,M6809_A, M6809_B, M6809_U, M6809_X, M6809_Y,
	M6809_DP
};

#define M6809_IRQ_LINE	0	/* IRQ line number */
#define M6809_FIRQ_LINE 1   /* FIRQ line number */

//void m6809_get_info(UINT32 state, cpuinfo *info);
//void m6809e_get_info(UINT32 state, cpuinfo *info);

void m6809_init(int (*irqcallback)(int));
void m6809_reset(void);
int m6809_execute(int cycles);
void m6809_set_irq_line(int irqline, int state);
void m6809_get_context(void *dst);
void m6809_set_context(void *src);

unsigned char M6809ReadByte(unsigned short Address);
void M6809WriteByte(unsigned short Address, unsigned char Data);
unsigned char M6809ReadOp(unsigned short Address);
unsigned char M6809ReadOpArg(unsigned short Address);

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define M6809_RDMEM(Addr) ((unsigned)M6809ReadByte(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define M6809_WRMEM(Addr,Value) (M6809WriteByte(Addr,Value))

/****************************************************************************/
/* Z80_RDOP() is identical to Z80_RDMEM() except it is used for reading     */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define M6809_RDOP(Addr) ((unsigned)M6809ReadOp(Addr))

/****************************************************************************/
/* Z80_RDOP_ARG() is identical to Z80_RDOP() except it is used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define M6809_RDOP_ARG(Addr) ((unsigned)M6809ReadOpArg(Addr))

#ifndef FALSE
#    define FALSE 0
#endif
#ifndef TRUE
#    define TRUE (!FALSE)
#endif

#endif /* __M6809_H__ */
