#ifndef _Z80_H_
#define _Z80_H_

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;

typedef signed char INT8;
typedef signed short INT16;
typedef signed int INT32;

#define TRUE (1 == 1)
#define FALSE (1 == 0)

#include <stdio.h>

#define INPUT_LINE_NMI 127
#define CLEAR_LINE 0

#define INLINE inline

enum {
	Z80_PC=1, Z80_SP,
	Z80_A, Z80_B, Z80_C, Z80_D, Z80_E, Z80_H, Z80_L,
	Z80_AF, Z80_BC, Z80_DE, Z80_HL,
	Z80_IX, Z80_IY,	Z80_AF2, Z80_BC2, Z80_DE2, Z80_HL2,
	Z80_R, Z80_I, Z80_IM, Z80_IFF1, Z80_IFF2, Z80_HALT,
	Z80_DC0, Z80_DC1, Z80_DC2, Z80_DC3
};

enum {
	Z80_TABLE_op,
	Z80_TABLE_cb,
	Z80_TABLE_ed,
	Z80_TABLE_xy,
	Z80_TABLE_xycb,
	Z80_TABLE_ex	/* cycles counts for taken jr/jp/call and interrupt latency (rst opcodes) */
};

#define LSB_FIRST

typedef union {
#ifdef LSB_FIRST
	struct { UINT8 l,h,h2,h3; } b;
	struct { UINT16 l,h; } w;
#else
	struct { UINT8 h3,h2,h,l; } b;
	struct { UINT16 h,l; } w;
#endif
	UINT32 d;
}	PAIR;

/****************************************************************************/
/* The Z80 registers. HALT is set to 1 when the CPU is halted, the refresh  */
/* register is calculated as follows: refresh=(Z80.r&127)|(Z80.r2&128)      */
/****************************************************************************/
typedef struct
{
	PAIR	prvpc,pc,sp,af,bc,de,hl,ix,iy;
	PAIR	af2,bc2,de2,hl2;
	UINT8	r,r2,iff1,iff2,halt,im,i;
	UINT8	nmi_state;			/* nmi line state */
	UINT8	nmi_pending;		/* nmi pending */
	UINT8	irq_state;			/* irq line state */
	UINT8	after_ei;			/* are we in the EI shadow? */
	const struct z80_irq_daisy_chain *daisy;
	int		(*irq_callback)(int irqline);
}	Z80_Regs;

void z80_init();
void z80_exit(void);
void z80_set_irq_line(int irqline, int state);
int z80_execute(int cycles);
void z80_reset(void);

#endif

