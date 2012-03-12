/*** m6805: Portable 6805 emulator ******************************************

    m6805.c (Also supports hd68705 and hd63705 variants)

    References:

        6809 Simulator V09, By L.C. Benschop, Eidnhoven The Netherlands.

        m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
            the 6809 Simulator V09)

        6809 Microcomputer Programming & Interfacing with Experiments"
            by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

    System dependencies:    UINT16 must be 16 bit unsigned int
                            UINT8 must be 8 bit unsigned int
                            UINT32 must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

  Additional Notes:

  K.Wilkins 18/03/99 - Added 63705 functonality and modified all CPU functions
                       necessary to support:
                           Variable width address bus
                           Different stack pointer
                           Alternate boot vectors
                           Alternate interrups vectors


*****************************************************************************/

#include "burnint.h"
#include "driver.h"
#include "m6805_intf.h"

#define change_pc(x)	PC = x

#define IRQ_LEVEL_DETECT 0

#define M6805_INLINE static

enum
{
	SUBTYPE_M6805,
	SUBTYPE_M68705,
	SUBTYPE_HD63705
};

/* 6805 Registers */
typedef struct
{
	int 	subtype;		/* Which sub-type is being emulated */
	UINT32	sp_mask;		/* Stack pointer address mask */
	UINT32	sp_low; 		/* Stack pointer low water mark (or floor) */
    PAIR    pc;             /* Program counter */
	PAIR	s;				/* Stack pointer */
	UINT8	a;				/* Accumulator */
	UINT8	x;				/* Index register */
	UINT8	cc; 			/* Condition codes */

	UINT16	pending_interrupts; /* MB */
	int 	(*irq_callback)(int irqline);
	int 	irq_state[9];		/* KW Additional lines for HD63705 */
	int		nmi_state;
	int	nTotalCycles;
} m6805_Regs;

/* 6805 registers */
static m6805_Regs m6805;

#define SUBTYPE	m6805.subtype	/* CPU Type */
#define SP_MASK m6805.sp_mask	/* stack pointer mask */
#define SP_LOW	m6805.sp_low	/* stack pointer low water mark */
#define pPC 	m6805.pc		/* program counter PAIR */
#define PC		m6805.pc.w.l	/* program counter lower word */
#define S		m6805.s.w.l 	/* stack pointer lower word */
#define A		m6805.a 		/* accumulator */
#define X		m6805.x 		/* index register */
#define CC		m6805.cc		/* condition codes */

static PAIR ea; 		/* effective address */
#define EAD ea.d
#define EA  ea.w.l

/* public globals */
static int m6805_ICount=50000;

/* DS -- THESE ARE RE-DEFINED IN m6805.h TO RAM, ROM or FUNCTIONS IN cpuintrf.c */
#define RM(Addr)			M6805_RDMEM(Addr)
#define WM(Addr,Value)		M6805_WRMEM(Addr,Value)
#define M_RDOP(Addr)        M6805_RDOP(Addr)
#define M_RDOP_ARG(Addr)	M6805_RDOP_ARG(Addr)

/* macros to tweak the PC and SP */
#define SP_INC	if( ++S > SP_MASK) S = SP_LOW
#define SP_DEC	if( --S < SP_LOW) S = SP_MASK
#define SP_ADJUST(s) ( ( (s) & SP_MASK ) | SP_LOW )

/* macros to access memory */
#define IMMBYTE(b) {b = M_RDOP_ARG(PC++);}
#define IMMWORD(w) {w.d = 0; w.b.h = M_RDOP_ARG(PC); w.b.l = M_RDOP_ARG(PC+1); PC+=2;}

#define PUSHBYTE(b) wr_s_handler_b(&b)
#define PUSHWORD(w) wr_s_handler_w(&w)
#define PULLBYTE(b) rd_s_handler_b(&b)
#define PULLWORD(w) rd_s_handler_w(&w)

/* CC masks      H INZC
              7654 3210 */
#define CFLAG 0x01
#define ZFLAG 0x02
#define NFLAG 0x04
#define IFLAG 0x08
#define HFLAG 0x10

#define CLR_NZ	  CC&=~(NFLAG|ZFLAG)
#define CLR_HNZC  CC&=~(HFLAG|NFLAG|ZFLAG|CFLAG)
#define CLR_Z	  CC&=~(ZFLAG)
#define CLR_NZC   CC&=~(NFLAG|ZFLAG|CFLAG)
#define CLR_ZC	  CC&=~(ZFLAG|CFLAG)

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)       if(!a)SEZ
#define SET_Z8(a)	   SET_Z((UINT8)a)
#define SET_N8(a)	   CC|=((a&0x80)>>5)
#define SET_H(a,b,r)   CC|=((a^b^r)&0x10)
#define SET_C8(a)	   CC|=((a&0x100)>>8)

static const UINT8 flags8i[256]=	 /* increment */
{
0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04
};
static const UINT8 flags8d[256]= /* decrement */
{
0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04
};
#define SET_FLAGS8I(a)		{CC|=flags8i[(a)&0xff];}
#define SET_FLAGS8D(a)		{CC|=flags8d[(a)&0xff];}

/* combos */
#define SET_NZ8(a)			{SET_N8(a);SET_Z(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r);SET_Z8(r);SET_C8(r);}

/* for treating an unsigned UINT8 as a signed INT16 */
#define SIGNED(b) ((INT16)(b&0x80?b|0xff00:b))

/* Macros for addressing modes */
#define DIRECT EAD=0;IMMBYTE(ea.b.l)
#define IMM8 EA=PC++
#define EXTENDED IMMWORD(ea)
#define INDEXED EA=X
#define INDEXED1 {EAD=0; IMMBYTE(ea.b.l); EA+=X;}
#define INDEXED2 {IMMWORD(ea); EA+=X;}

/* macros to set status flags */
#if defined(SEC)
#undef SEC
#endif
#define SEC CC|=CFLAG
#define CLC CC&=~CFLAG
#define SEZ CC|=ZFLAG
#define CLZ CC&=~ZFLAG
#define SEN CC|=NFLAG
#define CLN CC&=~NFLAG
#define SEH CC|=HFLAG
#define CLH CC&=~HFLAG
#define SEI CC|=IFLAG
#define CLI CC&=~IFLAG

/* macros for convenience */
#define DIRBYTE(b) {DIRECT;b=RM(EAD);}
#define EXTBYTE(b) {EXTENDED;b=RM(EAD);}
#define IDXBYTE(b) {INDEXED;b=RM(EAD);}
#define IDX1BYTE(b) {INDEXED1;b=RM(EAD);}
#define IDX2BYTE(b) {INDEXED2;b=RM(EAD);}
/* Macros for branch instructions */
#define BRANCH(f) { UINT8 t; IMMBYTE(t); if(f) { PC+=SIGNED(t); change_pc(PC); if (t==0xfe) { /* speed up busy loops */ if(m6805_ICount > 0) m6805_ICount = 0; } } }

/* what they say it is ... */
static const unsigned char cycles1[] =
{
      /* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
  /*0*/ 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
  /*1*/  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  /*2*/  4, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  /*3*/  6, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 6, 0, 6, 6, 0,
  /*4*/  4, 0, 0, 4, 4, 0, 4, 4, 4, 4, 4, 0, 4, 4, 0, 4,
  /*5*/  4, 0, 0, 4, 4, 0, 4, 4, 4, 4, 4, 0, 4, 4, 0, 4,
  /*6*/  7, 0, 0, 7, 7, 0, 7, 7, 7, 7, 7, 0, 7, 7, 0, 7,
  /*7*/  6, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 0, 6, 6, 0, 6,
  /*8*/  9, 6, 0,11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*9*/  0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 2,
  /*A*/  2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 0, 8, 2, 0,
  /*B*/  4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 3, 7, 4, 5,
  /*C*/  5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 4, 8, 5, 6,
  /*D*/  6, 6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 5, 9, 6, 7,
  /*E*/  5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 4, 8, 5, 6,
  /*F*/  4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 3, 7, 4, 5
};


/* pre-clear a PAIR union; clearing h2 and h3 only might be faster? */
#define CLEAR_PAIR(p)   p->d = 0

M6805_INLINE void rd_s_handler_b( UINT8 *b )
{
	SP_INC;
	*b = RM( S );
}

M6805_INLINE void rd_s_handler_w( PAIR *p )
{
	CLEAR_PAIR(p);
	SP_INC;
	p->b.h = RM( S );
	SP_INC;
	p->b.l = RM( S );
}

M6805_INLINE void wr_s_handler_b( UINT8 *b )
{
	WM( S, *b );
	SP_DEC;
}

M6805_INLINE void wr_s_handler_w( PAIR *p )
{
	WM( S, p->b.l );
    SP_DEC;
	WM( S, p->b.h );
    SP_DEC;
}

M6805_INLINE void RM16( UINT32 Addr, PAIR *p )
{
	CLEAR_PAIR(p);
    p->b.h = RM(Addr);
    ++Addr;
//  if( ++Addr > AMASK ) Addr = 0;
	p->b.l = RM(Addr);
}

#ifdef UNUSED_FUNCTION
M6805_INLINE void WM16( UINT32 Addr, PAIR *p )
{
	WM( Addr, p->b.h );
    ++Addr;
//  if( ++Addr > AMASK ) Addr = 0;
	WM( Addr, p->b.l );
}
#endif


/* Generate interrupt - m68705 version */
static void m68705_Interrupt(void)
{
	if( (m6805.pending_interrupts & ((1<<M6805_IRQ_LINE)|M68705_INT_MASK)) != 0 )
	{
		if ( (CC & IFLAG) == 0 )
		{
			PUSHWORD(m6805.pc);
			PUSHBYTE(m6805.x);
			PUSHBYTE(m6805.a);
			PUSHBYTE(m6805.cc);
			SEI;
			if (m6805.irq_callback)
				(*m6805.irq_callback)(0);

			if ((m6805.pending_interrupts & (1<<M68705_IRQ_LINE)) != 0 )
			{
				m6805.pending_interrupts &= ~(1<<M68705_IRQ_LINE);
				RM16( 0xfffa, &pPC);
				change_pc(PC);
			}
			else if((m6805.pending_interrupts&(1<<M68705_INT_TIMER))!=0)
			{
				m6805.pending_interrupts &= ~(1<<M68705_INT_TIMER);
				RM16( 0xfff8, &pPC);
				change_pc(PC);
			}
		}
		m6805_ICount -= 11;
	}
}

/* Generate interrupts */
static void Interrupt(void)
{
	/* the 6805 latches interrupt requests internally, so we don't clear */
	/* pending_interrupts until the interrupt is taken, no matter what the */
	/* external IRQ pin does. */

#if (1) //HAS_HD63705)
	if( (m6805.pending_interrupts & (1<<HD63705_INT_NMI)) != 0)
	{
		PUSHWORD(m6805.pc);
		PUSHBYTE(m6805.x);
		PUSHBYTE(m6805.a);
		PUSHBYTE(m6805.cc);
        SEI;
		/* no vectors supported, just do the callback to clear irq_state if needed */
		if (m6805.irq_callback)
			(*m6805.irq_callback)(0);

		RM16( 0x1ffc, &pPC);
		change_pc(PC);
		m6805.pending_interrupts &= ~(1<<HD63705_INT_NMI);

		m6805_ICount -= 11;

	}
	else if( (m6805.pending_interrupts & ((1<<M6805_IRQ_LINE)|HD63705_INT_MASK)) != 0 ) {
		if ( (CC & IFLAG) == 0 ) {
#else
	if( (m6805.pending_interrupts & (1<<M6805_IRQ_LINE)) != 0 ) {
		if ( (CC & IFLAG) == 0 ) {
#endif
	{
        /* standard IRQ */
//#if (HAS_HD63705)
//      if(SUBTYPE!=SUBTYPE_HD63705)
//#endif
//          PC |= ~AMASK;
		PUSHWORD(m6805.pc);
		PUSHBYTE(m6805.x);
		PUSHBYTE(m6805.a);
		PUSHBYTE(m6805.cc);
        SEI;
		/* no vectors supported, just do the callback to clear irq_state if needed */
		if (m6805.irq_callback)
			(*m6805.irq_callback)(0);


//#if (HAS_HD63705)
		if(SUBTYPE==SUBTYPE_HD63705)
		{
			/* Need to add emulation of other interrupt sources here KW-2/4/99 */
			/* This is just a quick patch for Namco System 2 operation         */

			if((m6805.pending_interrupts&(1<<HD63705_INT_IRQ1))!=0)
			{
				m6805.pending_interrupts &= ~(1<<HD63705_INT_IRQ1);
				RM16( 0x1ff8, &pPC);
				change_pc(PC);
			}
			else if((m6805.pending_interrupts&(1<<HD63705_INT_IRQ2))!=0)
			{
				m6805.pending_interrupts &= ~(1<<HD63705_INT_IRQ2);
				RM16( 0x1fec, &pPC);
				change_pc(PC);
			}
			else if((m6805.pending_interrupts&(1<<HD63705_INT_ADCONV))!=0)
			{
				m6805.pending_interrupts &= ~(1<<HD63705_INT_ADCONV);
				RM16( 0x1fea, &pPC);
				change_pc(PC);
			}
			else if((m6805.pending_interrupts&(1<<HD63705_INT_TIMER1))!=0)
			{
				m6805.pending_interrupts &= ~(1<<HD63705_INT_TIMER1);
				RM16( 0x1ff6, &pPC);
				change_pc(PC);
			}
			else if((m6805.pending_interrupts&(1<<HD63705_INT_TIMER2))!=0)
			{
				m6805.pending_interrupts &= ~(1<<HD63705_INT_TIMER2);
				RM16( 0x1ff4, &pPC);
				change_pc(PC);
			}
			else if((m6805.pending_interrupts&(1<<HD63705_INT_TIMER3))!=0)
			{
				m6805.pending_interrupts &= ~(1<<HD63705_INT_TIMER3);
				RM16( 0x1ff2, &pPC);
				change_pc(PC);
			}
			else if((m6805.pending_interrupts&(1<<HD63705_INT_PCI))!=0)
			{
				m6805.pending_interrupts &= ~(1<<HD63705_INT_PCI);
				RM16( 0x1ff0, &pPC);
				change_pc(PC);
			}
			else if((m6805.pending_interrupts&(1<<HD63705_INT_SCI))!=0)
			{
				m6805.pending_interrupts &= ~(1<<HD63705_INT_SCI);
				RM16( 0x1fee, &pPC);
				change_pc(PC);
			}
		}
		else
//#endif
		{
			RM16( 0xffff - 5, &pPC );
			change_pc(PC);
		}

		}	// CC & IFLAG
			m6805.pending_interrupts &= ~(1<<M6805_IRQ_LINE);
		}
		m6805_ICount -= 11;
	}
}

static void m6805_reset()
{
	int (*save_irqcallback)(int) = m6805.irq_callback;
	memset(&m6805, 0, sizeof(m6805));
	m6805.irq_callback = save_irqcallback;
	/* Force CPU sub-type and relevant masks */
	m6805.subtype	= SUBTYPE_M6805;
	SP_MASK = 0x07f;
	SP_LOW	= 0x060;
	/* Initial stack pointer */
	S = SP_MASK;
	/* IRQ disabled */
    SEI;
	RM16( 0xfffe , &pPC );
	change_pc(PC);
}

void m6805Reset() {
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805Reset called without init\n"));
#endif

	m6805_reset();
}

//static void m6805_init(int ) //int (*irqcallback)(int))
//{
//	m6805.irq_callback = irqcallback;
//}

//static void m6805_exit(void)
//{
//	/* nothing to do */
//}


void m6805SetIrqLine(int , int state)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805SetIrqLine called without init\n"));
#endif

	/* Basic 6805 only has one IRQ line */
	/* See HD63705 specific version     */
	if (m6805.irq_state[0] == state) return;

	m6805.irq_state[0] = state;
	if (state != CLEAR_LINE)
		m6805.pending_interrupts |= 1<<M6805_IRQ_LINE;
}


#include "6805ops.c"


/* execute instructions on this CPU until icount expires */
int m6805Run(int cycles)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805Run called without init\n"));
#endif

	UINT8 ireg;
	m6805_ICount = cycles;

	do
	{
		if (m6805.pending_interrupts != 0)
		{
			if (SUBTYPE==SUBTYPE_M68705)
			{
				m68705_Interrupt();
			}
			else
			{
				Interrupt();
			}
		}

		ireg=M_RDOP(PC++);

		switch( ireg )
		{
			case 0x00: brset(0x01); break;
			case 0x01: brclr(0x01); break;
			case 0x02: brset(0x02); break;
			case 0x03: brclr(0x02); break;
			case 0x04: brset(0x04); break;
			case 0x05: brclr(0x04); break;
			case 0x06: brset(0x08); break;
			case 0x07: brclr(0x08); break;
			case 0x08: brset(0x10); break;
			case 0x09: brclr(0x10); break;
			case 0x0A: brset(0x20); break;
			case 0x0B: brclr(0x20); break;
			case 0x0C: brset(0x40); break;
			case 0x0D: brclr(0x40); break;
			case 0x0E: brset(0x80); break;
			case 0x0F: brclr(0x80); break;
			case 0x10: bset(0x01); break;
			case 0x11: bclr(0x01); break;
			case 0x12: bset(0x02); break;
			case 0x13: bclr(0x02); break;
			case 0x14: bset(0x04); break;
			case 0x15: bclr(0x04); break;
			case 0x16: bset(0x08); break;
			case 0x17: bclr(0x08); break;
			case 0x18: bset(0x10); break;
			case 0x19: bclr(0x10); break;
			case 0x1a: bset(0x20); break;
			case 0x1b: bclr(0x20); break;
			case 0x1c: bset(0x40); break;
			case 0x1d: bclr(0x40); break;
			case 0x1e: bset(0x80); break;
			case 0x1f: bclr(0x80); break;
			case 0x20: bra(); break;
			case 0x21: brn(); break;
			case 0x22: bhi(); break;
			case 0x23: bls(); break;
			case 0x24: bcc(); break;
			case 0x25: bcs(); break;
			case 0x26: bne(); break;
			case 0x27: beq(); break;
			case 0x28: bhcc(); break;
			case 0x29: bhcs(); break;
			case 0x2a: bpl(); break;
			case 0x2b: bmi(); break;
			case 0x2c: bmc(); break;
			case 0x2d: bms(); break;
			case 0x2e: bil(); break;
			case 0x2f: bih(); break;
			case 0x30: neg_di(); break;
			case 0x31: illegal(); break;
			case 0x32: illegal(); break;
			case 0x33: com_di(); break;
			case 0x34: lsr_di(); break;
			case 0x35: illegal(); break;
			case 0x36: ror_di(); break;
			case 0x37: asr_di(); break;
			case 0x38: lsl_di(); break;
			case 0x39: rol_di(); break;
			case 0x3a: dec_di(); break;
			case 0x3b: illegal(); break;
			case 0x3c: inc_di(); break;
			case 0x3d: tst_di(); break;
			case 0x3e: illegal(); break;
			case 0x3f: clr_di(); break;
			case 0x40: nega(); break;
			case 0x41: illegal(); break;
			case 0x42: illegal(); break;
			case 0x43: coma(); break;
			case 0x44: lsra(); break;
			case 0x45: illegal(); break;
			case 0x46: rora(); break;
			case 0x47: asra(); break;
			case 0x48: lsla(); break;
			case 0x49: rola(); break;
			case 0x4a: deca(); break;
			case 0x4b: illegal(); break;
			case 0x4c: inca(); break;
			case 0x4d: tsta(); break;
			case 0x4e: illegal(); break;
			case 0x4f: clra(); break;
			case 0x50: negx(); break;
			case 0x51: illegal(); break;
			case 0x52: illegal(); break;
			case 0x53: comx(); break;
			case 0x54: lsrx(); break;
			case 0x55: illegal(); break;
			case 0x56: rorx(); break;
			case 0x57: asrx(); break;
			case 0x58: aslx(); break;
			case 0x59: rolx(); break;
			case 0x5a: decx(); break;
			case 0x5b: illegal(); break;
			case 0x5c: incx(); break;
			case 0x5d: tstx(); break;
			case 0x5e: illegal(); break;
			case 0x5f: clrx(); break;
			case 0x60: neg_ix1(); break;
			case 0x61: illegal(); break;
			case 0x62: illegal(); break;
			case 0x63: com_ix1(); break;
			case 0x64: lsr_ix1(); break;
			case 0x65: illegal(); break;
			case 0x66: ror_ix1(); break;
			case 0x67: asr_ix1(); break;
			case 0x68: lsl_ix1(); break;
			case 0x69: rol_ix1(); break;
			case 0x6a: dec_ix1(); break;
			case 0x6b: illegal(); break;
			case 0x6c: inc_ix1(); break;
			case 0x6d: tst_ix1(); break;
			case 0x6e: illegal(); break;
			case 0x6f: clr_ix1(); break;
			case 0x70: neg_ix(); break;
			case 0x71: illegal(); break;
			case 0x72: illegal(); break;
			case 0x73: com_ix(); break;
			case 0x74: lsr_ix(); break;
			case 0x75: illegal(); break;
			case 0x76: ror_ix(); break;
			case 0x77: asr_ix(); break;
			case 0x78: lsl_ix(); break;
			case 0x79: rol_ix(); break;
			case 0x7a: dec_ix(); break;
			case 0x7b: illegal(); break;
			case 0x7c: inc_ix(); break;
			case 0x7d: tst_ix(); break;
			case 0x7e: illegal(); break;
			case 0x7f: clr_ix(); break;
			case 0x80: rti(); break;
			case 0x81: rts(); break;
			case 0x82: illegal(); break;
			case 0x83: swi(); break;
			case 0x84: illegal(); break;
			case 0x85: illegal(); break;
			case 0x86: illegal(); break;
			case 0x87: illegal(); break;
			case 0x88: illegal(); break;
			case 0x89: illegal(); break;
			case 0x8a: illegal(); break;
			case 0x8b: illegal(); break;
			case 0x8c: illegal(); break;
			case 0x8d: illegal(); break;
			case 0x8e: illegal(); break;
			case 0x8f: illegal(); break;
			case 0x90: illegal(); break;
			case 0x91: illegal(); break;
			case 0x92: illegal(); break;
			case 0x93: illegal(); break;
			case 0x94: illegal(); break;
			case 0x95: illegal(); break;
			case 0x96: illegal(); break;
			case 0x97: tax(); break;
			case 0x98: CLC; break;
			case 0x99: SEC; break;
#if IRQ_LEVEL_DETECT
			case 0x9a: CLI; if (m6805.irq_state != CLEAR_LINE) m6805.pending_interrupts |= 1<<M6805_IRQ_LINE; break;
#else
			case 0x9a: CLI; break;
#endif
			case 0x9b: SEI; break;
			case 0x9c: rsp(); break;
			case 0x9d: nop(); break;
			case 0x9e: illegal(); break;
			case 0x9f: txa(); break;
			case 0xa0: suba_im(); break;
			case 0xa1: cmpa_im(); break;
			case 0xa2: sbca_im(); break;
			case 0xa3: cpx_im(); break;
			case 0xa4: anda_im(); break;
			case 0xa5: bita_im(); break;
			case 0xa6: lda_im(); break;
			case 0xa7: illegal(); break;
			case 0xa8: eora_im(); break;
			case 0xa9: adca_im(); break;
			case 0xaa: ora_im(); break;
			case 0xab: adda_im(); break;
			case 0xac: illegal(); break;
			case 0xad: bsr(); break;
			case 0xae: ldx_im(); break;
			case 0xaf: illegal(); break;
			case 0xb0: suba_di(); break;
			case 0xb1: cmpa_di(); break;
			case 0xb2: sbca_di(); break;
			case 0xb3: cpx_di(); break;
			case 0xb4: anda_di(); break;
			case 0xb5: bita_di(); break;
			case 0xb6: lda_di(); break;
			case 0xb7: sta_di(); break;
			case 0xb8: eora_di(); break;
			case 0xb9: adca_di(); break;
			case 0xba: ora_di(); break;
			case 0xbb: adda_di(); break;
			case 0xbc: jmp_di(); break;
			case 0xbd: jsr_di(); break;
			case 0xbe: ldx_di(); break;
			case 0xbf: stx_di(); break;
			case 0xc0: suba_ex(); break;
			case 0xc1: cmpa_ex(); break;
			case 0xc2: sbca_ex(); break;
			case 0xc3: cpx_ex(); break;
			case 0xc4: anda_ex(); break;
			case 0xc5: bita_ex(); break;
			case 0xc6: lda_ex(); break;
			case 0xc7: sta_ex(); break;
			case 0xc8: eora_ex(); break;
			case 0xc9: adca_ex(); break;
			case 0xca: ora_ex(); break;
			case 0xcb: adda_ex(); break;
			case 0xcc: jmp_ex(); break;
			case 0xcd: jsr_ex(); break;
			case 0xce: ldx_ex(); break;
			case 0xcf: stx_ex(); break;
			case 0xd0: suba_ix2(); break;
			case 0xd1: cmpa_ix2(); break;
			case 0xd2: sbca_ix2(); break;
			case 0xd3: cpx_ix2(); break;
			case 0xd4: anda_ix2(); break;
			case 0xd5: bita_ix2(); break;
			case 0xd6: lda_ix2(); break;
			case 0xd7: sta_ix2(); break;
			case 0xd8: eora_ix2(); break;
			case 0xd9: adca_ix2(); break;
			case 0xda: ora_ix2(); break;
			case 0xdb: adda_ix2(); break;
			case 0xdc: jmp_ix2(); break;
			case 0xdd: jsr_ix2(); break;
			case 0xde: ldx_ix2(); break;
			case 0xdf: stx_ix2(); break;
			case 0xe0: suba_ix1(); break;
			case 0xe1: cmpa_ix1(); break;
			case 0xe2: sbca_ix1(); break;
			case 0xe3: cpx_ix1(); break;
			case 0xe4: anda_ix1(); break;
			case 0xe5: bita_ix1(); break;
			case 0xe6: lda_ix1(); break;
			case 0xe7: sta_ix1(); break;
			case 0xe8: eora_ix1(); break;
			case 0xe9: adca_ix1(); break;
			case 0xea: ora_ix1(); break;
			case 0xeb: adda_ix1(); break;
			case 0xec: jmp_ix1(); break;
			case 0xed: jsr_ix1(); break;
			case 0xee: ldx_ix1(); break;
			case 0xef: stx_ix1(); break;
			case 0xf0: suba_ix(); break;
			case 0xf1: cmpa_ix(); break;
			case 0xf2: sbca_ix(); break;
			case 0xf3: cpx_ix(); break;
			case 0xf4: anda_ix(); break;
			case 0xf5: bita_ix(); break;
			case 0xf6: lda_ix(); break;
			case 0xf7: sta_ix(); break;
			case 0xf8: eora_ix(); break;
			case 0xf9: adca_ix(); break;
			case 0xfa: ora_ix(); break;
			case 0xfb: adda_ix(); break;
			case 0xfc: jmp_ix(); break;
			case 0xfd: jsr_ix(); break;
			case 0xfe: ldx_ix(); break;
			case 0xff: stx_ix(); break;
		}
		m6805_ICount -= cycles1[ireg];
		m6805.nTotalCycles += cycles1[ireg];
	} while( m6805_ICount > 0 );

	return cycles - m6805_ICount;
}

void m6805RunEnd()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805RunEnd called without init\n"));
#endif

	m6805_ICount = 0;
}

void m6805NewFrame()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805NewFrame called without init\n"));
#endif

	m6805.nTotalCycles = 0;
}

int m6805TotalCycles()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805TotalCycles called without init\n"));
#endif

	return m6805.nTotalCycles;
}

int m6805Scan(int nAction, int *)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805Scan called without init\n"));
#endif

	struct BurnArea ba;

	int (*save_irqcallback)(int) = m6805.irq_callback;

	if (nAction & ACB_DRIVER_DATA) {
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = &m6805;
		ba.nLen	  = sizeof(m6805);
		ba.szName = "m6805 Registers";
		BurnAcb(&ba);
	}

	m6805.irq_callback = save_irqcallback;

	return 0;
}


/****************************************************************************
 * M68705 section
 ****************************************************************************/

void m68705Reset()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m68705Reset called without init\n"));
#endif

	m6805_reset();
	/* Overide default 6805 type */
	m6805.subtype = SUBTYPE_M68705;
	RM16( 0xfffe, &m6805.pc );
}

void m68705SetIrqLine(int irqline, int state)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m68705SetIrqLine called without init\n"));
#endif

	if (m6805.irq_state[irqline] == state ) return;
	m6805.irq_state[irqline] = state;
	if (state != CLEAR_LINE) m6805.pending_interrupts |= 1<<irqline;
}


/****************************************************************************
 * HD63705 section
 ****************************************************************************/

//	m6805.irq_callback = irqcallback;

void hd63705Reset(void)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("hd63705Reset called without init\n"));
#endif

	m6805_reset();

	/* Overide default 6805 types */
	m6805.subtype	= SUBTYPE_HD63705;
	SP_MASK = 0x17f;
	SP_LOW	= 0x100;
	RM16( 0x1ffe, &m6805.pc );
	S = 0x17f;
}

void hd63705SetIrqLine(int irqline, int state)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("hd63705SetIrqLine called without init\n"));
#endif

	if (irqline == 0x20) //INPUT_LINE_NMI)
	{
		if (m6805.nmi_state == state) return;

		m6805.nmi_state = state;
		if (state != CLEAR_LINE)
			m6805.pending_interrupts |= 1<<HD63705_INT_NMI;
	}
	else if (irqline <= HD63705_INT_ADCONV)
	{
		if (m6805.irq_state[irqline] == state) return;
		m6805.irq_state[irqline] = state;
		if (state != CLEAR_LINE) m6805.pending_interrupts |= 1<<irqline;
	}
}
