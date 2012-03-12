/*** hd6309: Portable 6309 emulator ******************************************

    Copyright John Butler
    Copyright Tim Lindner

    References:

        HD63B09EP Technical Refrence Guide, by Chet Simpson with addition
                            by Alan Dekok
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

    History:
070614 ZV:
    Fixed N flag setting in DIV overflow

991026 HJB:
    Fixed missing calls to cpu_changepc() for the TFR and EXG ocpodes.
    Replaced m6809_slapstic checks by a macro (CHANGE_PC). ESB still
    needs the tweaks.

991024 HJB:
    Tried to improve speed: Using bit7 of cycles1/2 as flag for multi
    byte opcodes is gone, those opcodes now call fetch_effective_address().
    Got rid of the slow/fast flags for stack (S and U) memory accesses.
    Minor changes to use 32 bit values as arguments to memory functions
    and added defines for that purpose (e.g. X = 16bit XD = 32bit).

990312 HJB:
    Added bugfixes according to Aaron's findings.
    Reset only sets CC_II and CC_IF, DP to zero and PC from reset vector.
990311 HJB:
    Added _info functions. Now uses static m6808_Regs struct instead
    of single statics. Changed the 16 bit registers to use the generic
    PAIR union. Registers defined using macros. Split the core into
    four execution loops for M6802, M6803, M6808 and HD63701.
    TST, TSTA and TSTB opcodes reset carry flag.
    Modified the read/write stack handlers to push LSB first then MSB
    and pull MSB first then LSB.

990228 HJB:
    Changed the interrupt handling again. Now interrupts are taken
    either right at the moment the lines are asserted or whenever
    an interrupt is enabled and the corresponding line is still
    asserted. That way the pending_interrupts checks are not
    needed anymore. However, the CWAI and SYNC flags still need
    some flags, so I changed the name to 'int_state'.
    This core also has the code for the old interrupt system removed.

990225 HJB:
    Cleaned up the code here and there, added some comments.
    Slightly changed the SAR opcodes (similiar to other CPU cores).
    Added symbolic names for the flag bits.
    Changed the way CWAI/Interrupt() handle CPU state saving.
    A new flag M6809_STATE in pending_interrupts is used to determine
    if a state save is needed on interrupt entry or already done by CWAI.
    Added M6809_IRQ_LINE and M6809_FIRQ_LINE defines to m6809.h
    Moved the internal interrupt_pending flags from m6809.h to m6809.c
    Changed CWAI cycles2[0x3c] to be 2 (plus all or at least 19 if
    CWAI actually pushes the entire state).
    Implemented undocumented TFR/EXG for undefined source and mixed 8/16
    bit transfers (they should transfer/exchange the constant $ff).
    Removed unused jmp/jsr _slap functions from 6809ops.c,
    m6809_slapstick check moved into the opcode functions.

000809 TJL:
    Started converting m6809 into hd6309

001217 TJL:
    Finished:
        All opcodes
        Dual Timing
    To Do:
        Verify new DIV opcodes.

070805 TJL:
    Fixed ADDR and ADCR opcodes not to clear the H condition code. Fixed ANDR,
    EORR, ORR, ADDR, ADCR, SBCR, and SUBR to evaluate condition codes after
    the destination register was set. Fixed BITMD opcode to only effect the Z
    condition code. Fixed BITMD opcode to clear only tested flags. Fixed EXG
    and TFR register promotion and demotion. Fixed illegal instruction handler
    to not set I and F condition codes. Credit to Darren Atkinson for the
    discovery of these bugs.

090907 TJL:
    The SEXW instruction is clearing the Overflow flag (V). It should not do
    that. When an invalid source or destination register is specified for
    the TFM instructions, real hardware invokes the Illegal Instruction
    trap, whereas the emulator simply ignores the instruction. Credit to
    Darren Atkinson for the discovery of these bugs.

*****************************************************************************/

//#include "debugger.h"
//#include "deprecat.h"
#include "burnint.h"
#include "hd6309.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define HD6309_INLINE		static
#define change_pc(newpc)	hd6309.pc.w.l = (newpc)
#define HD6309_CLEAR_LINE	0

//extern offs_t hd6309_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

/*#define BIG_SWITCH*/

static void CHECK_IRQ_LINES( void );
static void IIError(void);
static void DZError(void);

HD6309_INLINE void fetch_effective_address( void );

/* flag bits in the cc register */
#define CC_C	0x01		/* Carry */
#define CC_V	0x02		/* Overflow */
#define CC_Z	0x04		/* Zero */
#define CC_N	0x08		/* Negative */
#define CC_II	0x10		/* Inhibit IRQ */
#define CC_H	0x20		/* Half (auxiliary) carry */
#define CC_IF	0x40		/* Inhibit FIRQ */
#define CC_E	0x80		/* entire state pushed */

/* flag bits in the md register */
#define MD_EM	0x01		/* Execution mode */
#define MD_FM	0x02		/* FIRQ mode */
#define MD_II	0x40		/* Illegal instruction */
#define MD_DZ	0x80		/* Division by zero */

/* 6309 registers */
static hd6309_Regs hd6309;
//static int hd6309_slapstic = 0;

#define pPPC	hd6309.ppc
#define pPC 	hd6309.pc
#define pU		hd6309.u
#define pS		hd6309.s
#define pX		hd6309.x
#define pY		hd6309.y
#define pV		hd6309.v
/*#define pQ        hd6309.q*/
#define pD		hd6309.d
#define pW		hd6309.w
#define pZ		hd6309.z

#define PPC 	hd6309.ppc.w.l
#define PC		hd6309.pc.w.l
#define PCD 	hd6309.pc.d
#define U		hd6309.u.w.l
#define UD		hd6309.u.d
#define S		hd6309.s.w.l
#define SD		hd6309.s.d
#define X		hd6309.x.w.l
#define XD		hd6309.x.d
#define Y		hd6309.y.w.l
#define YD		hd6309.y.d
#define V		hd6309.v.w.l
#define VD		hd6309.v.d
#define D		hd6309.d.w.l
#define A		hd6309.d.b.h
#define B		hd6309.d.b.l
#define W		hd6309.w.w.l
#define E		hd6309.w.b.h
#define F		hd6309.w.b.l
#define DP		hd6309.dp.b.h
#define DPD 	hd6309.dp.d
#define CC		hd6309.cc
#define MD		hd6309.md

static PAIR ea; 		/* effective address */
#define EA	ea.w.l
#define EAD ea.d

#define CHANGE_PC change_pc(PCD)
#if 0
#define CHANGE_PC	{			\
	if( hd6309_slapstic )		\
		cpu_setOPbase16(PCD);	\
	else						\
		change_pc(PCD);		\
	}
#endif

#define HD6309_CWAI 	8	/* set when CWAI is waiting for an interrupt */
#define HD6309_SYNC 	16	/* set when SYNC is waiting for an interrupt */
#define HD6309_LDS		32	/* set when LDS occured at least once */

/* public globals */
static int hd6309_ICount;

/* these are re-defined in hd6309.h TO RAM, ROM or functions in cpuintrf.c */
#define RM(mAddr)		HD6309_RDMEM(mAddr)
#define WM(mAddr,Value) HD6309_WRMEM(mAddr,Value)
#define ROP(mAddr)		HD6309_RDOP(mAddr)
#define ROP_ARG(mAddr)	HD6309_RDOP_ARG(mAddr)

/* macros to access memory */
#define IMMBYTE(b)	b = ROP_ARG(PCD); PC++
#define IMMWORD(w)	w.d = (ROP_ARG(PCD)<<8) | ROP_ARG((PCD+1)&0xffff); PC+=2
#define IMMLONG(w)	w.d = (ROP_ARG(PCD)<<24) + (ROP_ARG(PCD+1)<<16) + (ROP_ARG(PCD+2)<<8) + (ROP_ARG(PCD+3)); PC+=4

#define PUSHBYTE(b) --S; WM(SD,b)
#define PUSHWORD(w) --S; WM(SD,w.b.l); --S; WM(SD,w.b.h)
#define PULLBYTE(b) b = RM(SD); S++
#define PULLWORD(w) w = RM(SD)<<8; S++; w |= RM(SD); S++

#define PSHUBYTE(b) --U; WM(UD,b);
#define PSHUWORD(w) --U; WM(UD,w.b.l); --U; WM(UD,w.b.h)
#define PULUBYTE(b) b = RM(UD); U++
#define PULUWORD(w) w = RM(UD)<<8; U++; w |= RM(UD); U++

#define CLR_HNZVC	CC&=~(CC_H|CC_N|CC_Z|CC_V|CC_C)
#define CLR_NZV 	CC&=~(CC_N|CC_Z|CC_V)
#define CLR_NZ	 	CC&=~(CC_N|CC_Z)
#define CLR_HNZC	CC&=~(CC_H|CC_N|CC_Z|CC_C)
#define CLR_NZVC	CC&=~(CC_N|CC_Z|CC_V|CC_C)
#define CLR_Z		CC&=~(CC_Z)
#define CLR_N		CC&=~(CC_N)
#define CLR_NZC 	CC&=~(CC_N|CC_Z|CC_C)
#define CLR_ZC		CC&=~(CC_Z|CC_C)

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)		if(!a)SEZ
#define SET_Z8(a)		SET_Z((UINT8)a)
#define SET_Z16(a)		SET_Z((UINT16)a)
#define SET_N8(a)		CC|=((a&0x80)>>4)
#define SET_N16(a)		CC|=((a&0x8000)>>12)
#define SET_N32(a)		CC|=((a&0x8000)>>20)
#define SET_H(a,b,r)	CC|=(((a^b^r)&0x10)<<1)
#define SET_C8(a)		CC|=((a&0x100)>>8)
#define SET_C16(a)		CC|=((a&0x10000)>>16)
#define SET_V8(a,b,r)	CC|=(((a^b^r^(r>>1))&0x80)>>6)
#define SET_V16(a,b,r)	CC|=(((a^b^r^(r>>1))&0x8000)>>14)

#define SET_FLAGS8I(a)		{CC|=flags8i[(a)&0xff];}
#define SET_FLAGS8D(a)		{CC|=flags8d[(a)&0xff];}

static UINT8 const *cycle_counts_page0;
static UINT8 const *cycle_counts_page01;
static UINT8 const *cycle_counts_page11;
static UINT8 const *index_cycle;

/* combos */
#define SET_NZ8(a)			{SET_N8(a);SET_Z(a);}
#define SET_NZ16(a) 		{SET_N16(a);SET_Z(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)	{SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

#define NXORV				((CC&CC_N)^((CC&CC_V)<<2))

/* for treating an unsigned byte as a signed word */
#define SIGNED(b) ((UINT16)(b&0x80?b|0xff00:b))
/* for treating an unsigned short as a signed long */
#define SIGNED_16(b) ((UINT32)(b&0x8000?b|0xffff0000:b))

/* macros for addressing modes (postbytes have their own code) */
#define DIRECT	EAD = DPD; IMMBYTE(ea.b.l)
#define IMM8	EAD = PCD; PC++
#define IMM16	EAD = PCD; PC+=2
#define EXTENDED IMMWORD(ea)

/* macros to set status flags */
#if defined(SEC)
#undef SEC
#endif
#define SEC CC|=CC_C
#define CLC CC&=~CC_C
#define SEZ CC|=CC_Z
#define CLZ CC&=~CC_Z
#define SEN CC|=CC_N
#define CLN CC&=~CC_N
#define SEV CC|=CC_V
#define CLV CC&=~CC_V
#define SEH CC|=CC_H
#define CLH CC&=~CC_H

/* Macros to set mode flags */
#define SEDZ MD|=MD_DZ
#define CLDZ MD&=~MD_DZ
#define SEII MD|=MD_II
#define CLII MD&=~MD_II
#define SEFM MD|=MD_FM
#define CLFM MD&=~MD_FM
#define SEEM MD|=MD_EM
#define CLEM MD&=~MD_EM

/* macros for convenience */
#define DIRBYTE(b) {DIRECT;b=RM(EAD);}
#define DIRWORD(w) {DIRECT;w.d=RM16(EAD);}
#define DIRLONG(lng) {DIRECT;lng.w.h=RM16(EAD);lng.w.l=RM16(EAD+2);}
#define EXTBYTE(b) {EXTENDED;b=RM(EAD);}
#define EXTWORD(w) {EXTENDED;w.d=RM16(EAD);}
#define EXTLONG(lng) {EXTENDED;lng.w.h=RM16(EAD);lng.w.l=RM16(EAD+2);}

/* includes the static function prototypes and other tables */
#include "6309tbl.c"

/* macros for branch instructions */
#define BRANCH(f) { 					\
	UINT8 t;							\
	IMMBYTE(t); 						\
	if( f ) 							\
	{									\
		PC += SIGNED(t);				\
		CHANGE_PC;						\
	}									\
}

#define LBRANCH(f) {					\
	PAIR t; 							\
	IMMWORD(t); 						\
	if( f ) 							\
	{									\
		if( !(MD & MD_EM) )				\
			hd6309_ICount -= 1;			\
		PC += t.w.l;					\
		CHANGE_PC;						\
	}									\
}

HD6309_INLINE UINT32 RM16( UINT32 mAddr );
HD6309_INLINE UINT32 RM16( UINT32 mAddr )
{
	UINT32 result = RM(mAddr) << 8;
	return result | RM((mAddr+1)&0xffff);
}

HD6309_INLINE UINT32 RM32( UINT32 mAddr );
HD6309_INLINE UINT32 RM32( UINT32 mAddr )
{
	UINT32 result = RM(mAddr) << 24;
	result += RM(mAddr+1) << 16;
	result += RM(mAddr+2) << 8;
	result += RM(mAddr+3);
	return result;
}

HD6309_INLINE void WM16( UINT32 mAddr, PAIR *p );
HD6309_INLINE void WM16( UINT32 mAddr, PAIR *p )
{
	WM( mAddr, p->b.h );
	WM( (mAddr+1)&0xffff, p->b.l );
}

HD6309_INLINE void WM32( UINT32 mAddr, PAIR *p );
HD6309_INLINE void WM32( UINT32 mAddr, PAIR *p )
{
	WM( mAddr, p->b.h3 );
	WM( (mAddr+1)&0xffff, p->b.h2 );
	WM( (mAddr+2)&0xffff, p->b.h );
	WM( (mAddr+3)&0xffff, p->b.l );
}

static void UpdateState(void)
{
	if ( hd6309.md & MD_EM )
	{
		cycle_counts_page0  = ccounts_page0_na;
		cycle_counts_page01 = ccounts_page01_na;
		cycle_counts_page11 = ccounts_page11_na;
		index_cycle         = index_cycle_na;
	}
	else
	{
		cycle_counts_page0  = ccounts_page0_em;
		cycle_counts_page01 = ccounts_page01_em;
		cycle_counts_page11 = ccounts_page11_em;
		index_cycle         = index_cycle_em;
	}
}

static void CHECK_IRQ_LINES( void )
{
	if( hd6309.irq_state[HD6309_IRQ_LINE] != HD6309_CLEAR_LINE ||
		hd6309.irq_state[HD6309_FIRQ_LINE] != HD6309_CLEAR_LINE )
		hd6309.int_state &= ~HD6309_SYNC; /* clear SYNC flag */
	if( hd6309.irq_state[HD6309_FIRQ_LINE]!=HD6309_CLEAR_LINE && !(CC & CC_IF))
	{
		/* fast IRQ */
		/* HJB 990225: state already saved by CWAI? */
		if( hd6309.int_state & HD6309_CWAI )
		{
			hd6309.int_state &= ~HD6309_CWAI;
			hd6309.extra_cycles += 7;		 /* subtract +7 cycles */
		}
		else
		{
			if ( MD & MD_FM )
			{
				CC |= CC_E; 				/* save entire state */
				PUSHWORD(pPC);
				PUSHWORD(pU);
				PUSHWORD(pY);
				PUSHWORD(pX);
				PUSHBYTE(DP);
				if ( MD & MD_EM )
				{
					PUSHBYTE(F);
					PUSHBYTE(E);
					hd6309.extra_cycles += 2; /* subtract +2 cycles */
				}
				PUSHBYTE(B);
				PUSHBYTE(A);
				PUSHBYTE(CC);
				hd6309.extra_cycles += 19;	 /* subtract +19 cycles */
			}
			else
			{
				CC &= ~CC_E;				/* save 'short' state */
				PUSHWORD(pPC);
				PUSHBYTE(CC);
				hd6309.extra_cycles += 10;	/* subtract +10 cycles */
			}
		}
		CC |= CC_IF | CC_II;			/* inhibit FIRQ and IRQ */
		PCD=RM16(0xfff6);
		CHANGE_PC;
//		(void)(*hd6309.irq_callback)(HD6309_FIRQ_LINE);
	}
	else
	if( hd6309.irq_state[HD6309_IRQ_LINE]!=HD6309_CLEAR_LINE && !(CC & CC_II) )
	{
		/* standard IRQ */
		/* HJB 990225: state already saved by CWAI? */
		if( hd6309.int_state & HD6309_CWAI )
		{
			hd6309.int_state &= ~HD6309_CWAI;  /* clear CWAI flag */
			hd6309.extra_cycles += 7;		 /* subtract +7 cycles */
		}
		else
		{
			CC |= CC_E; 				/* save entire state */
			PUSHWORD(pPC);
			PUSHWORD(pU);
			PUSHWORD(pY);
			PUSHWORD(pX);
			PUSHBYTE(DP);
			if ( MD & MD_EM )
			{
				PUSHBYTE(F);
				PUSHBYTE(E);
				hd6309.extra_cycles += 2; /* subtract +2 cycles */
			}
			PUSHBYTE(B);
			PUSHBYTE(A);
			PUSHBYTE(CC);
			hd6309.extra_cycles += 19;	 /* subtract +19 cycles */
		}
		CC |= CC_II;					/* inhibit IRQ */
		PCD=RM16(0xfff8);
		CHANGE_PC;
//		(void)(*hd6309.irq_callback)(HD6309_IRQ_LINE);
	}
}

/****************************************************************************
 * Get all registers in given buffer
 ****************************************************************************/
void hd6309_get_context(void *dst)
{
	if( dst )
		*(hd6309_Regs*)dst = hd6309;
}

/****************************************************************************
 * Set all registers to given values
 ****************************************************************************/
void hd6309_set_context(void *src)
{
	if( src )
		hd6309 = *(hd6309_Regs*)src;
	CHANGE_PC;

	CHECK_IRQ_LINES();
	UpdateState();
}

//static STATE_POSTLOAD( hd6309_postload )
//{
//	UpdateState();
//}

void hd6309_init()
{
//	hd6309.irq_callback = irqcallback;

//	state_save_register_item("hd6309", index, PC);
//	state_save_register_item("hd6309", index, U);
//	state_save_register_item("hd6309", index, S);
//	state_save_register_item("hd6309", index, X);
//	state_save_register_item("hd6309", index, Y);
//	state_save_register_item("hd6309", index, V);
//	state_save_register_item("hd6309", index, DP);
//	state_save_register_item("hd6309", index, CC);
//	state_save_register_item("hd6309", index, MD);
//	state_save_register_postload(Machine, hd6309_postload, NULL);
//	state_save_register_item("hd6309", index, hd6309.int_state);
//	state_save_register_item("hd6309", index, hd6309.nmi_state);
//	state_save_register_item("hd6309", index, hd6309.irq_state[0]);
//	state_save_register_item("hd6309", index, hd6309.irq_state[1]);
}

/****************************************************************************/
/* Reset registers to their initial values                                  */
/****************************************************************************/
void hd6309_reset(void)
{
	hd6309.int_state = 0;
	hd6309.nmi_state = HD6309_CLEAR_LINE;
	hd6309.irq_state[0] = HD6309_CLEAR_LINE;
	hd6309.irq_state[0] = HD6309_CLEAR_LINE;

	DPD = 0;			/* Reset direct page register */

	MD = 0; 			/* Mode register gets reset */
	CC |= CC_II;		/* IRQ disabled */
	CC |= CC_IF;		/* FIRQ disabled */

	PCD = RM16(0xfffe);
	CHANGE_PC;
	UpdateState();
}

int hd6309_get_pc()
{
	return PC;
}

/*
static void hd6309_exit(void)
{

}*/

/* Generate interrupts */
/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
void hd6309_set_irq_line(int irqline, int state)
{
	if (irqline == HD6309_INPUT_LINE_NMI)
	{
		if (hd6309.nmi_state == state) return;
		hd6309.nmi_state = state;
//		LOG(("HD6309#%d set_irq_line (NMI) %d (PC=%4.4X)\n", cpu_getactivecpu(), state, pPC.d));
		if( state == HD6309_CLEAR_LINE ) return;

		/* if the stack was not yet initialized */
		if( !(hd6309.int_state & HD6309_LDS) ) return;

		hd6309.int_state &= ~HD6309_SYNC;
		/* HJB 990225: state already saved by CWAI? */
		if( hd6309.int_state & HD6309_CWAI )
		{
			hd6309.int_state &= ~HD6309_CWAI;
			hd6309.extra_cycles += 7;	/* subtract +7 cycles next time */
		}
		else
		{
			CC |= CC_E; 				/* save entire state */
			PUSHWORD(pPC);
			PUSHWORD(pU);
			PUSHWORD(pY);
			PUSHWORD(pX);
			PUSHBYTE(DP);
			if ( MD & MD_EM )
			{
				PUSHBYTE(F);
				PUSHBYTE(E);
				hd6309.extra_cycles += 2; /* subtract +2 cycles */
			}

			PUSHBYTE(B);
			PUSHBYTE(A);
			PUSHBYTE(CC);
			hd6309.extra_cycles += 19;	/* subtract +19 cycles next time */
		}
		CC |= CC_IF | CC_II;			/* inhibit FIRQ and IRQ */
		PCD = RM16(0xfffc);
		CHANGE_PC;
	}
	else if (irqline < 2)
	{
//		LOG(("HD6309#%d set_irq_line %d, %d (PC=%4.4X)\n", cpu_getactivecpu(), irqline, state, pPC.d));
		hd6309.irq_state[irqline] = state;
		if (state == HD6309_CLEAR_LINE) return;
		CHECK_IRQ_LINES();
	}
}

/* includes the actual opcode implementations */
#include "6309ops.c"

/* execute instructions on this CPU until icount expires */
int hd6309_execute(int cycles)	/* NS 970908 */
{
	hd6309_ICount = cycles - hd6309.extra_cycles;
	hd6309.extra_cycles = 0;

	if (hd6309.int_state & (HD6309_CWAI | HD6309_SYNC))
	{
//		debugger_instruction_hook(Machine, PCD);
		hd6309_ICount = 0;
	}
	else
	{
		do
		{
			pPPC = pPC;

//			debugger_instruction_hook(Machine, PCD);

			hd6309.ireg = ROP(PCD);
			PC++;

#ifdef BIG_SWITCH
			switch( hd6309.ireg )
			{
			case 0x00: neg_di();   				break;
			case 0x01: oim_di();   				break;
			case 0x02: aim_di();   				break;
			case 0x03: com_di();   				break;
			case 0x04: lsr_di();   				break;
			case 0x05: eim_di();   				break;
			case 0x06: ror_di();   				break;
			case 0x07: asr_di();   				break;
			case 0x08: asl_di();   				break;
			case 0x09: rol_di();   				break;
			case 0x0a: dec_di();   				break;
			case 0x0b: tim_di();   				break;
			case 0x0c: inc_di();   				break;
			case 0x0d: tst_di();   				break;
			case 0x0e: jmp_di();   				break;
			case 0x0f: clr_di();   				break;
			case 0x10: pref10();				break;
			case 0x11: pref11();				break;
			case 0x12: nop();	   				break;
			case 0x13: sync();	   				break;
			case 0x14: sexw();	   				break;
			case 0x15: IIError();				break;
			case 0x16: lbra();	   				break;
			case 0x17: lbsr();	   				break;
			case 0x18: IIError();				break;
			case 0x19: daa();	   				break;
			case 0x1a: orcc();	   				break;
			case 0x1b: IIError();				break;
			case 0x1c: andcc();    				break;
			case 0x1d: sex();	   				break;
			case 0x1e: exg();	   				break;
			case 0x1f: tfr();	   				break;
			case 0x20: bra();	   				break;
			case 0x21: brn();	   				break;
			case 0x22: bhi();	   				break;
			case 0x23: bls();	   				break;
			case 0x24: bcc();	   				break;
			case 0x25: bcs();	   				break;
			case 0x26: bne();	   				break;
			case 0x27: beq();	   				break;
			case 0x28: bvc();	   				break;
			case 0x29: bvs();	   				break;
			case 0x2a: bpl();	   				break;
			case 0x2b: bmi();	   				break;
			case 0x2c: bge();	   				break;
			case 0x2d: blt();	   				break;
			case 0x2e: bgt();	   				break;
			case 0x2f: ble();	   				break;
			case 0x30: leax();	   				break;
			case 0x31: leay();	   				break;
			case 0x32: leas();	   				break;
			case 0x33: leau();	   				break;
			case 0x34: pshs();	   				break;
			case 0x35: puls();	   				break;
			case 0x36: pshu();	   				break;
			case 0x37: pulu();	   				break;
			case 0x38: IIError();				break;
			case 0x39: rts();	   				break;
			case 0x3a: abx();	   				break;
			case 0x3b: rti();	   				break;
			case 0x3c: cwai();					break;
			case 0x3d: mul();					break;
			case 0x3e: IIError();				break;
			case 0x3f: swi();					break;
			case 0x40: nega();	   				break;
			case 0x41: IIError();				break;
			case 0x42: IIError();				break;
			case 0x43: coma();	   				break;
			case 0x44: lsra();	   				break;
			case 0x45: IIError();				break;
			case 0x46: rora();	   				break;
			case 0x47: asra();	   				break;
			case 0x48: asla();	   				break;
			case 0x49: rola();	   				break;
			case 0x4a: deca();	   				break;
			case 0x4b: IIError();				break;
			case 0x4c: inca();	   				break;
			case 0x4d: tsta();	   				break;
			case 0x4e: IIError();				break;
			case 0x4f: clra();	   				break;
			case 0x50: negb();	   				break;
			case 0x51: IIError();				break;
			case 0x52: IIError();				break;
			case 0x53: comb();	   				break;
			case 0x54: lsrb();	   				break;
			case 0x55: IIError();				break;
			case 0x56: rorb();	   				break;
			case 0x57: asrb();	   				break;
			case 0x58: aslb();	   				break;
			case 0x59: rolb();	   				break;
			case 0x5a: decb();	   				break;
			case 0x5b: IIError();				break;
			case 0x5c: incb();	   				break;
			case 0x5d: tstb();	   				break;
			case 0x5e: IIError();				break;
			case 0x5f: clrb();	   				break;
			case 0x60: neg_ix();   				break;
			case 0x61: oim_ix();   				break;
			case 0x62: aim_ix();   				break;
			case 0x63: com_ix();   				break;
			case 0x64: lsr_ix();   				break;
			case 0x65: eim_ix();   				break;
			case 0x66: ror_ix();   				break;
			case 0x67: asr_ix();   				break;
			case 0x68: asl_ix();   				break;
			case 0x69: rol_ix();   				break;
			case 0x6a: dec_ix();   				break;
			case 0x6b: tim_ix();   				break;
			case 0x6c: inc_ix();   				break;
			case 0x6d: tst_ix();   				break;
			case 0x6e: jmp_ix();   				break;
			case 0x6f: clr_ix();   				break;
			case 0x70: neg_ex();   				break;
			case 0x71: oim_ex();   				break;
			case 0x72: aim_ex();   				break;
			case 0x73: com_ex();   				break;
			case 0x74: lsr_ex();   				break;
			case 0x75: eim_ex();   				break;
			case 0x76: ror_ex();   				break;
			case 0x77: asr_ex();   				break;
			case 0x78: asl_ex();   				break;
			case 0x79: rol_ex();   				break;
			case 0x7a: dec_ex();   				break;
			case 0x7b: tim_ex();   				break;
			case 0x7c: inc_ex();   				break;
			case 0x7d: tst_ex();   				break;
			case 0x7e: jmp_ex();   				break;
			case 0x7f: clr_ex();   				break;
			case 0x80: suba_im();  				break;
			case 0x81: cmpa_im();  				break;
			case 0x82: sbca_im();  				break;
			case 0x83: subd_im();  				break;
			case 0x84: anda_im();  				break;
			case 0x85: bita_im();  				break;
			case 0x86: lda_im();   				break;
			case 0x87: IIError(); 				break;
			case 0x88: eora_im();  				break;
			case 0x89: adca_im();  				break;
			case 0x8a: ora_im();   				break;
			case 0x8b: adda_im();  				break;
			case 0x8c: cmpx_im();  				break;
			case 0x8d: bsr();	   				break;
			case 0x8e: ldx_im();   				break;
			case 0x8f: IIError();  				break;
			case 0x90: suba_di();  				break;
			case 0x91: cmpa_di();  				break;
			case 0x92: sbca_di();  				break;
			case 0x93: subd_di();  				break;
			case 0x94: anda_di();  				break;
			case 0x95: bita_di();  				break;
			case 0x96: lda_di();   				break;
			case 0x97: sta_di();   				break;
			case 0x98: eora_di();  				break;
			case 0x99: adca_di();  				break;
			case 0x9a: ora_di();   				break;
			case 0x9b: adda_di();  				break;
			case 0x9c: cmpx_di();  				break;
			case 0x9d: jsr_di();   				break;
			case 0x9e: ldx_di();   				break;
			case 0x9f: stx_di();   				break;
			case 0xa0: suba_ix();  				break;
			case 0xa1: cmpa_ix();  				break;
			case 0xa2: sbca_ix();  				break;
			case 0xa3: subd_ix();  				break;
			case 0xa4: anda_ix();  				break;
			case 0xa5: bita_ix();  				break;
			case 0xa6: lda_ix();   				break;
			case 0xa7: sta_ix();   				break;
			case 0xa8: eora_ix();  				break;
			case 0xa9: adca_ix();  				break;
			case 0xaa: ora_ix();   				break;
			case 0xab: adda_ix();  				break;
			case 0xac: cmpx_ix();  				break;
			case 0xad: jsr_ix();   				break;
			case 0xae: ldx_ix();   				break;
			case 0xaf: stx_ix();   				break;
			case 0xb0: suba_ex();  				break;
			case 0xb1: cmpa_ex();  				break;
			case 0xb2: sbca_ex();  				break;
			case 0xb3: subd_ex();  				break;
			case 0xb4: anda_ex();  				break;
			case 0xb5: bita_ex();  				break;
			case 0xb6: lda_ex();   				break;
			case 0xb7: sta_ex();   				break;
			case 0xb8: eora_ex();  				break;
			case 0xb9: adca_ex();  				break;
			case 0xba: ora_ex();   				break;
			case 0xbb: adda_ex();  				break;
			case 0xbc: cmpx_ex();  				break;
			case 0xbd: jsr_ex();   				break;
			case 0xbe: ldx_ex();   				break;
			case 0xbf: stx_ex();   				break;
			case 0xc0: subb_im();  				break;
			case 0xc1: cmpb_im();  				break;
			case 0xc2: sbcb_im();  				break;
			case 0xc3: addd_im();  				break;
			case 0xc4: andb_im();  				break;
			case 0xc5: bitb_im();  				break;
			case 0xc6: ldb_im();   				break;
			case 0xc7: IIError(); 				break;
			case 0xc8: eorb_im();  				break;
			case 0xc9: adcb_im();  				break;
			case 0xca: orb_im();   				break;
			case 0xcb: addb_im();  				break;
			case 0xcc: ldd_im();   				break;
			case 0xcd: ldq_im();   				break; /* in m6809 was std_im */
			case 0xce: ldu_im();   				break;
			case 0xcf: IIError();  				break;
			case 0xd0: subb_di();  				break;
			case 0xd1: cmpb_di();  				break;
			case 0xd2: sbcb_di();  				break;
			case 0xd3: addd_di();  				break;
			case 0xd4: andb_di();  				break;
			case 0xd5: bitb_di();  				break;
			case 0xd6: ldb_di();   				break;
			case 0xd7: stb_di();   				break;
			case 0xd8: eorb_di();  				break;
			case 0xd9: adcb_di();  				break;
			case 0xda: orb_di();   				break;
			case 0xdb: addb_di();  				break;
			case 0xdc: ldd_di();   				break;
			case 0xdd: std_di();   				break;
			case 0xde: ldu_di();   				break;
			case 0xdf: stu_di();   				break;
			case 0xe0: subb_ix();  				break;
			case 0xe1: cmpb_ix();  				break;
			case 0xe2: sbcb_ix();  				break;
			case 0xe3: addd_ix();  				break;
			case 0xe4: andb_ix();  				break;
			case 0xe5: bitb_ix();  				break;
			case 0xe6: ldb_ix();   				break;
			case 0xe7: stb_ix();   				break;
			case 0xe8: eorb_ix();  				break;
			case 0xe9: adcb_ix();  				break;
			case 0xea: orb_ix();   				break;
			case 0xeb: addb_ix();  				break;
			case 0xec: ldd_ix();   				break;
			case 0xed: std_ix();   				break;
			case 0xee: ldu_ix();   				break;
			case 0xef: stu_ix();   				break;
			case 0xf0: subb_ex();  				break;
			case 0xf1: cmpb_ex();  				break;
			case 0xf2: sbcb_ex();  				break;
			case 0xf3: addd_ex();  				break;
			case 0xf4: andb_ex();  				break;
			case 0xf5: bitb_ex();  				break;
			case 0xf6: ldb_ex();   				break;
			case 0xf7: stb_ex();   				break;
			case 0xf8: eorb_ex();  				break;
			case 0xf9: adcb_ex();  				break;
			case 0xfa: orb_ex();   				break;
			case 0xfb: addb_ex();  				break;
			case 0xfc: ldd_ex();   				break;
			case 0xfd: std_ex();   				break;
			case 0xfe: ldu_ex();   				break;
			case 0xff: stu_ex();   				break;
			}
#else
			(*hd6309_main[hd6309.ireg])();
#endif    /* BIG_SWITCH */

			hd6309_ICount -= cycle_counts_page0[hd6309.ireg];

		} while( hd6309_ICount > 0 );

		hd6309_ICount -= hd6309.extra_cycles;
		hd6309.extra_cycles = 0;
	}

	return cycles - hd6309_ICount;	 /* NS 970908 */
}

HD6309_INLINE void fetch_effective_address( void )
{
	UINT8 postbyte = ROP_ARG(PCD);
	PC++;

	switch(postbyte)
	{
	case 0x00: EA=X;													break;
	case 0x01: EA=X+1;													break;
	case 0x02: EA=X+2;													break;
	case 0x03: EA=X+3;													break;
	case 0x04: EA=X+4;													break;
	case 0x05: EA=X+5;													break;
	case 0x06: EA=X+6;													break;
	case 0x07: EA=X+7;													break;
	case 0x08: EA=X+8;													break;
	case 0x09: EA=X+9;													break;
	case 0x0a: EA=X+10; 												break;
	case 0x0b: EA=X+11; 												break;
	case 0x0c: EA=X+12; 												break;
	case 0x0d: EA=X+13; 												break;
	case 0x0e: EA=X+14; 												break;
	case 0x0f: EA=X+15; 												break;

	case 0x10: EA=X-16; 												break;
	case 0x11: EA=X-15; 												break;
	case 0x12: EA=X-14; 												break;
	case 0x13: EA=X-13; 												break;
	case 0x14: EA=X-12; 												break;
	case 0x15: EA=X-11; 												break;
	case 0x16: EA=X-10; 												break;
	case 0x17: EA=X-9;													break;
	case 0x18: EA=X-8;													break;
	case 0x19: EA=X-7;													break;
	case 0x1a: EA=X-6;													break;
	case 0x1b: EA=X-5;													break;
	case 0x1c: EA=X-4;													break;
	case 0x1d: EA=X-3;													break;
	case 0x1e: EA=X-2;													break;
	case 0x1f: EA=X-1;													break;

	case 0x20: EA=Y;													break;
	case 0x21: EA=Y+1;													break;
	case 0x22: EA=Y+2;													break;
	case 0x23: EA=Y+3;													break;
	case 0x24: EA=Y+4;													break;
	case 0x25: EA=Y+5;													break;
	case 0x26: EA=Y+6;													break;
	case 0x27: EA=Y+7;													break;
	case 0x28: EA=Y+8;													break;
	case 0x29: EA=Y+9;													break;
	case 0x2a: EA=Y+10; 												break;
	case 0x2b: EA=Y+11; 												break;
	case 0x2c: EA=Y+12; 												break;
	case 0x2d: EA=Y+13; 												break;
	case 0x2e: EA=Y+14; 												break;
	case 0x2f: EA=Y+15; 												break;

	case 0x30: EA=Y-16; 												break;
	case 0x31: EA=Y-15; 												break;
	case 0x32: EA=Y-14; 												break;
	case 0x33: EA=Y-13; 												break;
	case 0x34: EA=Y-12; 												break;
	case 0x35: EA=Y-11; 												break;
	case 0x36: EA=Y-10; 												break;
	case 0x37: EA=Y-9;													break;
	case 0x38: EA=Y-8;													break;
	case 0x39: EA=Y-7;													break;
	case 0x3a: EA=Y-6;													break;
	case 0x3b: EA=Y-5;													break;
	case 0x3c: EA=Y-4;													break;
	case 0x3d: EA=Y-3;													break;
	case 0x3e: EA=Y-2;													break;
	case 0x3f: EA=Y-1;													break;

	case 0x40: EA=U;													break;
	case 0x41: EA=U+1;													break;
	case 0x42: EA=U+2;													break;
	case 0x43: EA=U+3;													break;
	case 0x44: EA=U+4;													break;
	case 0x45: EA=U+5;													break;
	case 0x46: EA=U+6;													break;
	case 0x47: EA=U+7;													break;
	case 0x48: EA=U+8;													break;
	case 0x49: EA=U+9;													break;
	case 0x4a: EA=U+10; 												break;
	case 0x4b: EA=U+11; 												break;
	case 0x4c: EA=U+12; 												break;
	case 0x4d: EA=U+13; 												break;
	case 0x4e: EA=U+14; 												break;
	case 0x4f: EA=U+15; 												break;

	case 0x50: EA=U-16; 												break;
	case 0x51: EA=U-15; 												break;
	case 0x52: EA=U-14; 												break;
	case 0x53: EA=U-13; 												break;
	case 0x54: EA=U-12; 												break;
	case 0x55: EA=U-11; 												break;
	case 0x56: EA=U-10; 												break;
	case 0x57: EA=U-9;													break;
	case 0x58: EA=U-8;													break;
	case 0x59: EA=U-7;													break;
	case 0x5a: EA=U-6;													break;
	case 0x5b: EA=U-5;													break;
	case 0x5c: EA=U-4;													break;
	case 0x5d: EA=U-3;													break;
	case 0x5e: EA=U-2;													break;
	case 0x5f: EA=U-1;													break;

	case 0x60: EA=S;													break;
	case 0x61: EA=S+1;													break;
	case 0x62: EA=S+2;													break;
	case 0x63: EA=S+3;													break;
	case 0x64: EA=S+4;													break;
	case 0x65: EA=S+5;													break;
	case 0x66: EA=S+6;													break;
	case 0x67: EA=S+7;													break;
	case 0x68: EA=S+8;													break;
	case 0x69: EA=S+9;													break;
	case 0x6a: EA=S+10; 												break;
	case 0x6b: EA=S+11; 												break;
	case 0x6c: EA=S+12; 												break;
	case 0x6d: EA=S+13; 												break;
	case 0x6e: EA=S+14; 												break;
	case 0x6f: EA=S+15; 												break;

	case 0x70: EA=S-16; 												break;
	case 0x71: EA=S-15; 												break;
	case 0x72: EA=S-14; 												break;
	case 0x73: EA=S-13; 												break;
	case 0x74: EA=S-12; 												break;
	case 0x75: EA=S-11; 												break;
	case 0x76: EA=S-10; 												break;
	case 0x77: EA=S-9;													break;
	case 0x78: EA=S-8;													break;
	case 0x79: EA=S-7;													break;
	case 0x7a: EA=S-6;													break;
	case 0x7b: EA=S-5;													break;
	case 0x7c: EA=S-4;													break;
	case 0x7d: EA=S-3;													break;
	case 0x7e: EA=S-2;													break;
	case 0x7f: EA=S-1;													break;

	case 0x80: EA=X;	X++;											break;
	case 0x81: EA=X;	X+=2;											break;
	case 0x82: X--; 	EA=X;											break;
	case 0x83: X-=2;	EA=X;											break;
	case 0x84: EA=X;													break;
	case 0x85: EA=X+SIGNED(B);											break;
	case 0x86: EA=X+SIGNED(A);											break;
	case 0x87: EA=X+SIGNED(E);											break;
	case 0x88: IMMBYTE(EA); 	EA=X+SIGNED(EA);						break;
	case 0x89: IMMWORD(ea); 	EA+=X;									break;
	case 0x8a: EA=X+SIGNED(F);											break;
	case 0x8b: EA=X+D;													break;
	case 0x8c: IMMBYTE(EA); 	EA=PC+SIGNED(EA);						break;
	case 0x8d: IMMWORD(ea); 	EA+=PC; 								break;
	case 0x8e: EA=X+W;													break;
	case 0x8f: EA=W;		 											break;

	case 0x90: EA=W;								EAD=RM16(EAD);		break;
	case 0x91: EA=X;	X+=2;						EAD=RM16(EAD);		break;
	case 0x92: IIError();												break;
	case 0x93: X-=2;	EA=X;						EAD=RM16(EAD);		break;
	case 0x94: EA=X;								EAD=RM16(EAD);		break;
	case 0x95: EA=X+SIGNED(B);						EAD=RM16(EAD);		break;
	case 0x96: EA=X+SIGNED(A);						EAD=RM16(EAD);		break;
	case 0x97: EA=X+SIGNED(E);						EAD=RM16(EAD);		break;
	case 0x98: IMMBYTE(EA); 	EA=X+SIGNED(EA);	EAD=RM16(EAD);		break;
	case 0x99: IMMWORD(ea); 	EA+=X;				EAD=RM16(EAD);		break;
	case 0x9a: EA=X+SIGNED(F);						EAD=RM16(EAD);		break;
	case 0x9b: EA=X+D;								EAD=RM16(EAD);		break;
	case 0x9c: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);		break;
	case 0x9d: IMMWORD(ea); 	EA+=PC; 			EAD=RM16(EAD);		break;
	case 0x9e: EA=X+W;								EAD=RM16(EAD);		break;
	case 0x9f: IMMWORD(ea); 						EAD=RM16(EAD);		break;

	case 0xa0: EA=Y;	Y++;											break;
	case 0xa1: EA=Y;	Y+=2;											break;
	case 0xa2: Y--; 	EA=Y;											break;
	case 0xa3: Y-=2;	EA=Y;											break;
	case 0xa4: EA=Y;													break;
	case 0xa5: EA=Y+SIGNED(B);											break;
	case 0xa6: EA=Y+SIGNED(A);											break;
	case 0xa7: EA=Y+SIGNED(E);											break;
	case 0xa8: IMMBYTE(EA); 	EA=Y+SIGNED(EA);						break;
	case 0xa9: IMMWORD(ea); 	EA+=Y;									break;
	case 0xaa: EA=Y+SIGNED(F);											break;
	case 0xab: EA=Y+D;													break;
	case 0xac: IMMBYTE(EA); 	EA=PC+SIGNED(EA);						break;
	case 0xad: IMMWORD(ea); 	EA+=PC; 								break;
	case 0xae: EA=Y+W;													break;
	case 0xaf: IMMWORD(ea);     EA+=W;									break;

	case 0xb0: IMMWORD(ea); 	EA+=W;				EAD=RM16(EAD);		break;
	case 0xb1: EA=Y;	Y+=2;						EAD=RM16(EAD);		break;
	case 0xb2: IIError();												break;
	case 0xb3: Y-=2;	EA=Y;						EAD=RM16(EAD);		break;
	case 0xb4: EA=Y;								EAD=RM16(EAD);		break;
	case 0xb5: EA=Y+SIGNED(B);						EAD=RM16(EAD);		break;
	case 0xb6: EA=Y+SIGNED(A);						EAD=RM16(EAD);		break;
	case 0xb7: EA=Y+SIGNED(E);						EAD=RM16(EAD);		break;
	case 0xb8: IMMBYTE(EA); 	EA=Y+SIGNED(EA);	EAD=RM16(EAD);		break;
	case 0xb9: IMMWORD(ea); 	EA+=Y;				EAD=RM16(EAD);		break;
	case 0xba: EA=Y+SIGNED(F);						EAD=RM16(EAD);		break;
	case 0xbb: EA=Y+D;								EAD=RM16(EAD);		break;
	case 0xbc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);		break;
	case 0xbd: IMMWORD(ea); 	EA+=PC; 			EAD=RM16(EAD);		break;
	case 0xbe: EA=Y+W;								EAD=RM16(EAD);		break;
	case 0xbf: IIError();												break;

	case 0xc0: EA=U;			U++;									break;
	case 0xc1: EA=U;			U+=2;									break;
	case 0xc2: U--; 			EA=U;									break;
	case 0xc3: U-=2;			EA=U;									break;
	case 0xc4: EA=U;													break;
	case 0xc5: EA=U+SIGNED(B);											break;
	case 0xc6: EA=U+SIGNED(A);											break;
	case 0xc7: EA=U+SIGNED(E);											break;
	case 0xc8: IMMBYTE(EA); 	EA=U+SIGNED(EA);						break;
	case 0xc9: IMMWORD(ea); 	EA+=U;									break;
	case 0xca: EA=U+SIGNED(F);											break;
	case 0xcb: EA=U+D;													break;
	case 0xcc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);						break;
	case 0xcd: IMMWORD(ea); 	EA+=PC; 								break;
	case 0xce: EA=U+W;													break;
	case 0xcf: EA=W;            W+=2;									break;

	case 0xd0: EA=W;	W+=2;						EAD=RM16(EAD);		break;
	case 0xd1: EA=U;	U+=2;						EAD=RM16(EAD);		break;
	case 0xd2: IIError();												break;
	case 0xd3: U-=2;	EA=U;						EAD=RM16(EAD);		break;
	case 0xd4: EA=U;								EAD=RM16(EAD);		break;
	case 0xd5: EA=U+SIGNED(B);						EAD=RM16(EAD);		break;
	case 0xd6: EA=U+SIGNED(A);						EAD=RM16(EAD);		break;
	case 0xd7: EA=U+SIGNED(E);						EAD=RM16(EAD);		break;
	case 0xd8: IMMBYTE(EA); 	EA=U+SIGNED(EA);	EAD=RM16(EAD);		break;
	case 0xd9: IMMWORD(ea); 	EA+=U;				EAD=RM16(EAD);		break;
	case 0xda: EA=U+SIGNED(F);						EAD=RM16(EAD);		break;
	case 0xdb: EA=U+D;								EAD=RM16(EAD);		break;
	case 0xdc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);		break;
	case 0xdd: IMMWORD(ea); 	EA+=PC; 			EAD=RM16(EAD);		break;
	case 0xde: EA=U+W;								EAD=RM16(EAD);		break;
	case 0xdf: IIError();												break;

	case 0xe0: EA=S;	S++;											break;
	case 0xe1: EA=S;	S+=2;											break;
	case 0xe2: S--; 	EA=S;											break;
	case 0xe3: S-=2;	EA=S;											break;
	case 0xe4: EA=S;													break;
	case 0xe5: EA=S+SIGNED(B);											break;
	case 0xe6: EA=S+SIGNED(A);											break;
	case 0xe7: EA=S+SIGNED(E);											break;
	case 0xe8: IMMBYTE(EA); 	EA=S+SIGNED(EA);						break;
	case 0xe9: IMMWORD(ea); 	EA+=S;									break;
	case 0xea: EA=S+SIGNED(F);											break;
	case 0xeb: EA=S+D;													break;
	case 0xec: IMMBYTE(EA); 	EA=PC+SIGNED(EA);						break;
	case 0xed: IMMWORD(ea); 	EA+=PC; 								break;
	case 0xee: EA=S+W;													break;
	case 0xef: W-=2;	EA=W;											break;

	case 0xf0: W-=2;	EA=W;						EAD=RM16(EAD);		break;
	case 0xf1: EA=S;	S+=2;						EAD=RM16(EAD);		break;
	case 0xf2: IIError();												break;
	case 0xf3: S-=2;	EA=S;						EAD=RM16(EAD);		break;
	case 0xf4: EA=S;								EAD=RM16(EAD);		break;
	case 0xf5: EA=S+SIGNED(B);						EAD=RM16(EAD);		break;
	case 0xf6: EA=S+SIGNED(A);						EAD=RM16(EAD);		break;
	case 0xf7: EA=S+SIGNED(E);						EAD=RM16(EAD);		break;
	case 0xf8: IMMBYTE(EA); 	EA=S+SIGNED(EA);	EAD=RM16(EAD);		break;
	case 0xf9: IMMWORD(ea); 	EA+=S;				EAD=RM16(EAD);		break;
	case 0xfa: EA=S+SIGNED(F);						EAD=RM16(EAD);		break;
	case 0xfb: EA=S+D;								EAD=RM16(EAD);		break;
	case 0xfc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);		break;
	case 0xfd: IMMWORD(ea); 	EA+=PC; 			EAD=RM16(EAD);		break;
	case 0xfe: EA=S+W;								EAD=RM16(EAD);		break;
	case 0xff: IIError();												break;
	}

	hd6309_ICount -= index_cycle[postbyte];
}

#if 0
/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void hd6309_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + HD6309_IRQ_LINE:	set_irq_line(HD6309_IRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + HD6309_FIRQ_LINE:set_irq_line(HD6309_FIRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + HD6309_INPUT_LINE_NMI:	set_irq_line(HD6309_INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + HD6309_PC:		PC = info->i; CHANGE_PC;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + HD6309_S:		S = info->i;								break;
		case CPUINFO_INT_REGISTER + HD6309_CC:		CC = info->i; CHECK_IRQ_LINES();			break;
		case CPUINFO_INT_REGISTER + HD6309_MD:		MD = info->i; UpdateState();				break;
		case CPUINFO_INT_REGISTER + HD6309_U: 		U = info->i;								break;
		case CPUINFO_INT_REGISTER + HD6309_A: 		A = info->i;								break;
		case CPUINFO_INT_REGISTER + HD6309_B: 		B = info->i;								break;
		case CPUINFO_INT_REGISTER + HD6309_E: 		E = info->i;								break;
		case CPUINFO_INT_REGISTER + HD6309_F: 		F = info->i;								break;
		case CPUINFO_INT_REGISTER + HD6309_X: 		X = info->i;								break;
		case CPUINFO_INT_REGISTER + HD6309_Y: 		Y = info->i;								break;
		case CPUINFO_INT_REGISTER + HD6309_V: 		V = info->i;								break;
		case CPUINFO_INT_REGISTER + HD6309_DP: 		DP = info->i;								break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void hd6309_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(hd6309);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 4;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 5;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 20;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + HD6309_IRQ_LINE:	info->i = hd6309.irq_state[HD6309_IRQ_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + HD6309_FIRQ_LINE:info->i = hd6309.irq_state[HD6309_FIRQ_LINE]; break;
		case CPUINFO_INT_INPUT_STATE + HD6309_INPUT_LINE_NMI:	info->i = hd6309.nmi_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + HD6309_PC:			info->i = PC;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + HD6309_S:			info->i = S;							break;
		case CPUINFO_INT_REGISTER + HD6309_CC:			info->i = CC;							break;
		case CPUINFO_INT_REGISTER + HD6309_MD:			info->i = MD;							break;
		case CPUINFO_INT_REGISTER + HD6309_U:			info->i = U;							break;
		case CPUINFO_INT_REGISTER + HD6309_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + HD6309_B:			info->i = B;							break;
		case CPUINFO_INT_REGISTER + HD6309_E:			info->i = E;							break;
		case CPUINFO_INT_REGISTER + HD6309_F:			info->i = F;							break;
		case CPUINFO_INT_REGISTER + HD6309_X:			info->i = X;							break;
		case CPUINFO_INT_REGISTER + HD6309_Y:			info->i = Y;							break;
		case CPUINFO_INT_REGISTER + HD6309_V:			info->i = V;							break;
		case CPUINFO_INT_REGISTER + HD6309_DP:			info->i = DP;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = hd6309_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = hd6309_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = hd6309_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = hd6309_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = hd6309_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = hd6309_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = hd6309_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = hd6309_dasm;		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &hd6309_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "HD6309");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Hitachi 6309");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.01");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright John Butler and Tim Lindner"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c (MD:%c%c%c%c)",
				hd6309.cc & 0x80 ? 'E':'.',
				hd6309.cc & 0x40 ? 'F':'.',
				hd6309.cc & 0x20 ? 'H':'.',
				hd6309.cc & 0x10 ? 'I':'.',
				hd6309.cc & 0x08 ? 'N':'.',
				hd6309.cc & 0x04 ? 'Z':'.',
				hd6309.cc & 0x02 ? 'V':'.',
				hd6309.cc & 0x01 ? 'C':'.',

				hd6309.md & 0x80 ? 'E':'e',
				hd6309.md & 0x40 ? 'F':'f',
				hd6309.md & 0x02 ? 'I':'i',
				hd6309.md & 0x01 ? 'Z':'z');
			break;

		case CPUINFO_STR_REGISTER + HD6309_PC:			sprintf(info->s, "PC:%04X", hd6309.pc.w.l); break;
		case CPUINFO_STR_REGISTER + HD6309_S:			sprintf(info->s, "S:%04X", hd6309.s.w.l); break;
		case CPUINFO_STR_REGISTER + HD6309_CC:			sprintf(info->s, "CC:%02X", hd6309.cc); break;
		case CPUINFO_STR_REGISTER + HD6309_MD:			sprintf(info->s, "MD:%02X", hd6309.md); break;
		case CPUINFO_STR_REGISTER + HD6309_U:			sprintf(info->s, "U:%04X", hd6309.u.w.l); break;
		case CPUINFO_STR_REGISTER + HD6309_A:			sprintf(info->s, "A:%02X", hd6309.d.b.h); break;
		case CPUINFO_STR_REGISTER + HD6309_B:			sprintf(info->s, "B:%02X", hd6309.d.b.l); break;
		case CPUINFO_STR_REGISTER + HD6309_E:			sprintf(info->s, "E:%02X", hd6309.w.b.h); break;
		case CPUINFO_STR_REGISTER + HD6309_F:			sprintf(info->s, "F:%02X", hd6309.w.b.l); break;
		case CPUINFO_STR_REGISTER + HD6309_X:			sprintf(info->s, "X:%04X", hd6309.x.w.l); break;
		case CPUINFO_STR_REGISTER + HD6309_Y:			sprintf(info->s, "Y:%04X", hd6309.y.w.l); break;
		case CPUINFO_STR_REGISTER + HD6309_V:			sprintf(info->s, "V:%04X", hd6309.v.w.l); break;
		case CPUINFO_STR_REGISTER + HD6309_DP:			sprintf(info->s, "DP:%02X", hd6309.dp.b.h); break;
	}
}
#endif
