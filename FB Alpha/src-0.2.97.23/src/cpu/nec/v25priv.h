/* Cpu types, steps of 8 to help the cycle count calculation */
#define V33_TYPE 0
#define V30_TYPE 8
#define V20_TYPE 16

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

/* interrupt vectors */
enum
{
	NEC_DIVIDE_VECTOR	= 0,
	NEC_TRAP_VECTOR		= 1,
	NEC_NMI_VECTOR		= 2,
	NEC_BRKV_VECTOR		= 4,
	NEC_CHKIND_VECTOR	= 5,
	NEC_IBRK_VECTOR		= 19,
	NEC_INTTU0_VECTOR	= 28,
	NEC_INTTU1_VECTOR	= 29,
	NEC_INTTU2_VECTOR	= 30,
	NEC_INTD0_VECTOR	= 20,
	NEC_INTD1_VECTOR	= 21,
	NEC_INTP0_VECTOR	= 24,
	NEC_INTP1_VECTOR	= 25,
	NEC_INTP2_VECTOR	= 26,
	NEC_INTSER0_VECTOR	= 12,
	NEC_INTSR0_VECTOR	= 13,
	NEC_INTST0_VECTOR	= 14,
	NEC_INTSER1_VECTOR	= 16,
	NEC_INTSR1_VECTOR	= 17,
	NEC_INTST1_VECTOR	= 18,
	NEC_INTTB_VECTOR	= 31
};

/* interrupt sources */
typedef enum
{
	BRK		= 0,
	INT_IRQ	= 1,
	NMI_IRQ	= 1 << 1,
	INTTU0	= 1 << 2,
	INTTU1	= 1 << 3,
	INTTU2	= 1 << 4,
	INTD0	= 1 << 5,
	INTD1	= 1 << 6,
	INTP0	= 1 << 7,
	INTP1	= 1 << 8,
	INTP2	= 1 << 9,
	INTSER0	= 1 << 10,
	INTSR0	= 1 << 11,
	INTST0	= 1 << 12,
	INTSER1	= 1 << 13,
	INTSR1	= 1 << 14,
	INTST1	= 1 << 15,
	INTTB	= 1 << 16,
	BRKN	= 1 << 17,
	BRKS	= 1 << 18
} INTSOURCES;

/* internal RAM and register banks */
typedef union
{
	UINT16 w[128];
	UINT8  b[256];
} internalram;

typedef struct _v25_state_t v25_state_t;
struct _v25_state_t
{
	internalram ram;
	offs_t	fetch_xor;

	UINT16	ip;

	/* PSW flags */
	INT32	SignVal;
	UINT32	AuxVal, OverVal, ZeroVal, CarryVal, ParityVal;	/* 0 or non-0 valued flags */
	UINT8	IBRK, F0, F1, TF, IF, DF, MF;	/* 0 or 1 valued flags */
	UINT8	RBW, RBB;	/* current register bank base, preshifted for word and byte registers */

	/* interrupt related */
	UINT32  vector;
	UINT32	pending_irq;
	UINT32	unmasked_irq;
	UINT32	bankswitch_irq;
	UINT8	priority_inttu, priority_intd, priority_intp, priority_ints0, priority_ints1;
	UINT8	IRQS, ISPR;
	UINT32	nmi_state;
	UINT32	irq_state;
	UINT32	poll_state;
	UINT32	mode_state;
	UINT32	intp_state[3];
	UINT8	no_interrupt;
	UINT8	halted;

	/* timer related */
	UINT16	TM0, MD0, TM1, MD1;
	UINT8	TMC0, TMC1;
//	emu_timer *timers[4];
	int timer_param[4];
	int timer_cycles_period[4];
	int timer_cycles_until_trigger[4];
	unsigned char timer_enabled[4];
	unsigned char timer_flags[4];
	unsigned int clock; // unscaled!

	/* system control */
	UINT8	RAMEN, TB, PCK;	/* PRC register */
	UINT32	IDB;

	int		icount;

	//const nec_config *config;
	unsigned char *decode;

	UINT8	prefetch_size;
	UINT8	prefetch_cycles;
	INT8	prefetch_count;
	UINT8	prefetch_reset;
	UINT32	chip_type;

	UINT32	prefix_base;	/* base address of the latest prefix segment */
	UINT8	seg_prefix;		/* prefix segment indicator */

	UINT32 cycles_total;
	UINT32 cycles_remaining;
	INT8	stop_run;
};

enum {
	VECTOR_PC = 0x02/2,
	PSW_SAVE  = 0x04/2,
	PC_SAVE   = 0x06/2
};

typedef enum {
	DS1 = 0x0E/2,
	PS  = 0x0C/2,
	SS  = 0x0A/2,
	DS0 = 0x08/2
} SREGS;

typedef enum {
	AW = 0x1E/2,
	CW = 0x1C/2,
	DW = 0x1A/2,
	BW = 0x18/2,
	SP = 0x16/2,
	BP = 0x14/2,
	IX = 0x12/2,
	IY = 0x10/2
} WREGS;

#ifdef LSB_FIRST
typedef enum {
   AL = 0x1e, //NATIVE_ENDIAN_VALUE_LE_BE(0x1E, 0x1F),
   AH = 0x1f, //NATIVE_ENDIAN_VALUE_LE_BE(0x1F, 0x1E),
   CL = 0x1c, //NATIVE_ENDIAN_VALUE_LE_BE(0x1C, 0x1D),
   CH = 0x1d, //NATIVE_ENDIAN_VALUE_LE_BE(0x1D, 0x1C),
   DL = 0x1a, //NATIVE_ENDIAN_VALUE_LE_BE(0x1A, 0x1B),
   DH = 0x1b, //NATIVE_ENDIAN_VALUE_LE_BE(0x1B, 0x1A),
   BL = 0x18, //NATIVE_ENDIAN_VALUE_LE_BE(0x18, 0x19),
   BH = 0x19  //NATIVE_ENDIAN_VALUE_LE_BE(0x19, 0x18)
} BREGS;
#else
typedef enum {
   AL = 0x1f, //NATIVE_ENDIAN_VALUE_LE_BE(0x1E, 0x1F),
   AH = 0x1e, //NATIVE_ENDIAN_VALUE_LE_BE(0x1F, 0x1E),
   CL = 0x1d, //NATIVE_ENDIAN_VALUE_LE_BE(0x1C, 0x1D),
   CH = 0x1c, //NATIVE_ENDIAN_VALUE_LE_BE(0x1D, 0x1C),
   DL = 0x1b, //NATIVE_ENDIAN_VALUE_LE_BE(0x1A, 0x1B),
   DH = 0x1a, //NATIVE_ENDIAN_VALUE_LE_BE(0x1B, 0x1A),
   BL = 0x19, //NATIVE_ENDIAN_VALUE_LE_BE(0x18, 0x19),
   BH = 0x18  //NATIVE_ENDIAN_VALUE_LE_BE(0x19, 0x18)
} BREGS;
#endif

#define SetRB(x)		do { nec_state->RBW = (x) << 4; nec_state->RBB = (x) << 5; } while (0)

#define Sreg(x)			nec_state->ram.w[nec_state->RBW + (x)]
#define Wreg(x)			nec_state->ram.w[nec_state->RBW + (x)]
#define Breg(x)			nec_state->ram.b[nec_state->RBB + (x)]

#define PC(n)		((Sreg(PS)<<4)+(n)->ip)

#define CF		(nec_state->CarryVal!=0)
#define SF		(nec_state->SignVal<0)
#define ZF		(nec_state->ZeroVal==0)
#define PF		parity_table[(BYTE)nec_state->ParityVal]
#define AF		(nec_state->AuxVal!=0)
#define OF		(nec_state->OverVal!=0)
#define RB		(nec_state->RBW >> 4)

/************************************************************************/

UINT8 v25_read_byte(v25_state_t *nec_state, unsigned a);
UINT16 v25_read_word(v25_state_t *nec_state, unsigned a);
void v25_write_byte(v25_state_t *nec_state, unsigned a, UINT8 d);
void v25_write_word(v25_state_t *nec_state, unsigned a, UINT16 d);

#define read_mem_byte(a)			v25_read_byte(nec_state,(a))
#define read_mem_word(a)			v25_read_word(nec_state,(a))
#define write_mem_byte(a,d)			v25_write_byte(nec_state,(a),(d))
#define write_mem_word(a,d)			v25_write_word(nec_state,(a),(d))

#define read_port_byte(a)		cpu_readport(a)
#define read_port_word(a)		(cpu_readport(a)+cpu_readport((a)+1)*256)
#define write_port_byte(a,d)	cpu_writeport((a),(d))
//#define write_port_word(a,d)	{ cpu_writeport((a),(unsigned char)(d)); (cpu_writeport((a)+1,(d)>>8); }

/************************************************************************/

#define CHANGE_PC do { EMPTY_PREFETCH(); } while (0)

#define SegBase(Seg) (Sreg(Seg) << 4)

#define DefaultBase(Seg) ((nec_state->seg_prefix && (Seg==DS0 || Seg==SS)) ? nec_state->prefix_base : Sreg(Seg) << 4)

#define GetMemB(Seg,Off) (read_mem_byte(DefaultBase(Seg) + (Off)))
#define GetMemW(Seg,Off) (read_mem_word(DefaultBase(Seg) + (Off)))

#define PutMemB(Seg,Off,x) { write_mem_byte(DefaultBase(Seg) + (Off), (x)); }
#define PutMemW(Seg,Off,x) { write_mem_word(DefaultBase(Seg) + (Off), (x)); }

/* prefetch timing */

#define FETCH() 			fetch(nec_state)
#define FETCHWORD()			fetchword(nec_state)
#define EMPTY_PREFETCH()	nec_state->prefetch_reset = 1


#define PUSH(val) { Wreg(SP) -= 2; write_mem_word(((Sreg(SS)<<4)+Wreg(SP)), val); }
#define POP(var) { Wreg(SP) += 2; var = read_mem_word(((Sreg(SS)<<4) + ((Wreg(SP)-2) & 0xffff))); }

#define GetModRM UINT32 ModRM=FETCH()

/* Cycle count macros:
    CLK  - cycle count is the same on all processors
    CLKS - cycle count differs between processors, list all counts
    CLKW - cycle count for word read/write differs for odd/even source/destination address
    CLKM - cycle count for reg/mem instructions
    CLKR - cycle count for reg/mem instructions with different counts for odd/even addresses


    Prefetch & buswait time is not emulated.
    Extra cycles for PUSH'ing or POP'ing registers to odd addresses is not emulated.
*/

#define CLK(all) nec_state->icount-=all
#define CLKS(v20,v30,v33) { const UINT32 ccount=(v20<<16)|(v30<<8)|v33; nec_state->icount-=(ccount>>nec_state->chip_type)&0x7f; }
#define CLKW(v20o,v30o,v33o,v20e,v30e,v33e,addr) { const UINT32 ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; nec_state->icount-=(addr&1)?((ocount>>nec_state->chip_type)&0x7f):((ecount>>nec_state->chip_type)&0x7f); }
#define CLKM(v20,v30,v33,v20m,v30m,v33m) { const UINT32 ccount=(v20<<16)|(v30<<8)|v33, mcount=(v20m<<16)|(v30m<<8)|v33m; nec_state->icount-=( ModRM >=0xc0 )?((ccount>>nec_state->chip_type)&0x7f):((mcount>>nec_state->chip_type)&0x7f); }
#define CLKR(v20o,v30o,v33o,v20e,v30e,v33e,vall,addr) { const UINT32 ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; if (ModRM >=0xc0) nec_state->icount-=vall; else nec_state->icount-=(addr&1)?((ocount>>nec_state->chip_type)&0x7f):((ecount>>nec_state->chip_type)&0x7f); }

/************************************************************************/
#define CompressFlags() (WORD)(CF | (nec_state->IBRK << 1) | (PF << 2) | (nec_state->F0 << 3) | (AF << 4) \
				| (nec_state->F1 << 5) | (ZF << 6) | (SF << 7) | (nec_state->TF << 8) | (nec_state->IF << 9) \
				| (nec_state->DF << 10) | (OF << 11) | (RB << 12) | (nec_state->MF << 15))

#define ExpandFlags(f) \
{ \
	nec_state->CarryVal = (f) & 0x0001; \
	nec_state->IBRK = ((f) & 0x0002) == 0x0002; \
	nec_state->ParityVal = !((f) & 0x0004); \
	nec_state->F0 = ((f) & 0x0008) == 0x0008; \
	nec_state->AuxVal = (f) & 0x0010; \
	nec_state->F1 = ((f) & 0x0020) == 0x0020; \
	nec_state->ZeroVal = !((f) & 0x0040); \
	nec_state->SignVal = (f) & 0x0080 ? -1 : 0; \
	nec_state->TF = ((f) & 0x0100) == 0x0100; \
	nec_state->IF = ((f) & 0x0200) == 0x0200; \
	nec_state->DF = ((f) & 0x0400) == 0x0400; \
	nec_state->OverVal = (f) & 0x0800; \
	/* RB only changes on BRKCS/RETRBI/TSKSW, so skip it */ \
	nec_state->MF = ((f) & 0x8000) == 0x8000; \
}
