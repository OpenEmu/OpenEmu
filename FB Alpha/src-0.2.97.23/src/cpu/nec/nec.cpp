/****************************************************************************

    NEC V20/V30/V33 emulator

    ---------------------------------------------

    V20 = uPD70108 = 8-bit data bus @ 5MHz or 8MHz
    V20HL = uPD70108H = V20 with EMS support (24-bit address bus)

    V25 = uPD70320 = V20 with on-chip features:
            - 256 bytes on-chip RAM
            - 8 register banks
            - 4-bit input port
            - 20-bit I/O port
            - 2 channel serial interface
            - interrupt controller
            - 2 channel DMA controller
            - 2 channel 16-bit timer
            - new instructions: BTCLR, RETRBI, STOP, BRKCS, TSKSW,
                                MOVSPA, MOVSPB

    V25+ = uPD70325 = V25 @ 8MHz or 10MHz plus changes:
            - faster DMA
            - improved serial interface

    ---------------------------------------------

    V30 = uPD70116 = 16-bit data bus version of V20
    V30HL = uPD70116H = 16-bit data bus version of V20HL
    V30MX = V30HL with separate address and data busses

    V35 = uPD70330 = 16-bit data bus version of V25

    V35+ = uPD70335 = 16-bit data bus version of V25+

    ---------------------------------------------

    V40 = uPD70208 = 8-bit data bus @ 10MHz
    V40HL = uPD70208H = V40 with support up to 20Mhz

    ---------------------------------------------

    V50 = uPD70216 = 16-bit data bus version of V40
    V50HL = uPD70216H = 16-bit data bus version of V40HL

    ---------------------------------------------

    V41 = uPD70270

    V51 = uPD70280



    V33A = uPD70136A

    V53A = uPD70236A



    Instruction differences:
        V20, V30, V40, V50 have dedicated emulation instructions
            (BRKEM, RETEM, CALLN)

        V33A, V53A has dedicated address mode instructions
            (BRKXA, RETXA)



    (Re)Written June-September 2000 by Bryan McPhail (mish@tendril.co.uk) based
    on code by Oliver Bergmann (Raul_Bloodworth@hotmail.com) who based code
    on the i286 emulator by Fabrice Frances which had initial work based on
    David Hedley's pcemu(!).

    This new core features 99% accurate cycle counts for each processor,
    there are still some complex situations where cycle counts are wrong,
    typically where a few instructions have differing counts for odd/even
    source and odd/even destination memory operands.

    Flag settings are also correct for the NEC processors rather than the
    I86 versions.

    Changelist:

    22/02/2003:
        Removed cycle counts from memory accesses - they are certainly wrong,
        and there is already a memory access cycle penalty in the opcodes
        using them.

        Fixed save states.

        Fixed ADJBA/ADJBS/ADJ4A/ADJ4S flags/return values for all situations.
        (Fixes bugs in Geostorm and Thunderblaster)

        Fixed carry flag on NEG (I thought this had been fixed circa Mame 0.58,
        but it seems I never actually submitted the fix).

        Fixed many cycle counts in instructions and bug in cycle count
        macros (odd word cases were testing for odd instruction word address
        not data address).

    Todo!
        Double check cycle timing is 100%.

****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "driver.h"
#include "state.h"
#include "nec_intf.h"
#include "bitswap.h"
//#include "emu.h"
//#include "debugger.h"

#define offs_t int //?
//typedef UINT8 unsigned char
//typedef UINT16 unsigned short
//typedef UINT32 unsigned int
#define INT8 char
#define INT16 short
#define INT32 int
#define INPUT_LINE_NMI	0x20 // nmi

#define NEC_INLINE
#define NEC_CPU_CPP

unsigned char cpu_readmem20_op(unsigned int);
unsigned char cpu_readmem20_arg(unsigned int);
unsigned char cpu_readport(unsigned int);
void cpu_writeport(unsigned int,unsigned int);

static NEC_INLINE void write_mem_word(unsigned int a, unsigned short d)
{
	cpu_writemem20((a),(unsigned char)(d));
	cpu_writemem20((a)+1,(d)>>8);
}

static NEC_INLINE void write_port_word(unsigned int a, unsigned short d)
{
	cpu_writeport((a),(unsigned char)(d));
	cpu_writeport((a)+1,(d)>>8);
}

typedef UINT8 BOOLEAN;
typedef UINT8 BYTE;
typedef UINT16 WORD;
typedef UINT32 DWORD;

#include "nec.h"
#include "necpriv.h"

static nec_state_t sChips[4]; // 4 cpus should be plenty!
static nec_state_t *sChipsPtr;

NEC_INLINE void prefetch(nec_state_t *nec_state)
{
	nec_state->prefetch_count--;
}

static void do_prefetch(nec_state_t *nec_state, int previous_ICount)
{
	int diff = previous_ICount - (int) nec_state->icount;

	/* The implementation is not accurate, but comes close.
     * It does not respect that the V30 will fetch two bytes
     * at once directly, but instead uses only 2 cycles instead
     * of 4. There are however only very few sources publicly
     * available and they are vague.
     */
	while (nec_state->prefetch_count<0)
	{
		nec_state->prefetch_count++;
		if (diff>nec_state->prefetch_cycles)
			diff -= nec_state->prefetch_cycles;
		else
			nec_state->icount -= nec_state->prefetch_cycles;
	}

	if (nec_state->prefetch_reset)
	{
		nec_state->prefetch_count = 0;
		nec_state->prefetch_reset = 0;
		return;
	}

	while (diff>=nec_state->prefetch_cycles && nec_state->prefetch_count < nec_state->prefetch_size)
	{
		diff -= nec_state->prefetch_cycles;
		nec_state->prefetch_count++;
	}

}

NEC_INLINE UINT8 fetch(nec_state_t *nec_state)
{
	prefetch(nec_state);
	return cpu_readop_arg(((Sreg(PS)<<4)+nec_state->ip++) ^ nec_state->fetch_xor); // ^ fetch_xor?
}

NEC_INLINE UINT16 fetchword(nec_state_t *nec_state)
{
	UINT16 r = FETCH();
	r |= (FETCH()<<8);
	return r;
}

#include "necinstr.h"
#include "necmacro.h"
#include "necea.h"
#include "necmodrm.h"

static UINT8 parity_table[256];

static UINT8 fetchop(nec_state_t *nec_state)
{
	prefetch(nec_state);
	return cpu_readop(((Sreg(PS)<<4)+nec_state->ip++) ^ nec_state->fetch_xor);
}


/***************************************************************************/

int nec_reset()
{
	nec_state_t *nec_state = sChipsPtr;

	memset( &nec_state->regs.w, 0, sizeof(nec_state->regs.w));

	nec_state->ip = 0;
	nec_state->TF = 0;
	nec_state->IF = 0;
	nec_state->DF = 0;
	nec_state->MF = 1;	// brkem should set to 0 when implemented
	nec_state->SignVal = 0;
	nec_state->AuxVal = 0;
	nec_state->OverVal = 0;
	nec_state->ZeroVal = 1;
	nec_state->CarryVal = 0;
	nec_state->ParityVal = 1;
	nec_state->pending_irq = 0;
	nec_state->nmi_state = 0;
	nec_state->irq_state = 0;
	nec_state->poll_state = 1;
	nec_state->halted = 0;
	nec_state->cycles_total = 0;
	nec_state->cycles_remaining = 0;
	nec_state->vector = 0xff; //default vector

	Sreg(PS) = 0xffff;
	Sreg(SS) = 0;
	Sreg(DS0) = 0;
	Sreg(DS1) = 0;

	CHANGE_PC;

	return 0;
}

static void nec_interrupt(nec_state_t *nec_state, unsigned int_num, INTSOURCES source)
{
	UINT32 dest_seg, dest_off;

	i_pushf(nec_state);
	nec_state->TF = nec_state->IF = 0;

	if (source == INT_IRQ)	/* get vector */
		int_num = nec_state->vector;

	dest_off = read_mem_word(int_num*4);
	dest_seg = read_mem_word(int_num*4+2);

	PUSH(Sreg(PS));
	PUSH(nec_state->ip);
	nec_state->ip = (WORD)dest_off;
	Sreg(PS) = (WORD)dest_seg;
	CHANGE_PC;
}

static void nec_trap(nec_state_t *nec_state)
{
	nec_instruction[fetchop(nec_state)](nec_state);
	nec_interrupt(nec_state, NEC_TRAP_VECTOR, BRK);
}

static void external_int(nec_state_t *nec_state)
{
	if (nec_state->pending_irq & NMI_IRQ)
	{
		nec_interrupt(nec_state, NEC_NMI_VECTOR, NMI_IRQ);
		nec_state->pending_irq &= ~NMI_IRQ;
	}
	else if (nec_state->pending_irq)
	{
		/* the actual vector is retrieved after pushing flags */
		/* and clearing the IF */
		nec_interrupt(nec_state, (UINT32)-1, INT_IRQ);
		nec_state->irq_state = CLEAR_LINE;
		nec_state->pending_irq &= ~INT_IRQ;
	}
}

/****************************************************************************/
/*                             OPCODES                                      */
/****************************************************************************/

#include "necinstr.c"

/*****************************************************************************/

void nec_set_irq_line_and_vector(int irqline, int vector, int state)
{
	nec_state_t *nec_state = sChipsPtr;

	switch (irqline)
	{
		case 0:
			nec_state->irq_state = state;
			if (state == CLEAR_LINE)
				nec_state->pending_irq &= ~INT_IRQ;
			else
			{
				nec_state->vector = vector;
				nec_state->pending_irq |= INT_IRQ;
				nec_state->halted = 0;
			}
			break;
		case INPUT_LINE_NMI:
			if (nec_state->nmi_state == (unsigned int)state) return;
		    	nec_state->nmi_state = state;
			if (state != CLEAR_LINE)
			{
				nec_state->vector = vector;
				nec_state->pending_irq |= NMI_IRQ;
				nec_state->halted = 0;
			}
			break;
		case NEC_INPUT_LINE_POLL:
			nec_state->vector = vector;
			nec_state->poll_state = state;
			break;
	}
}

void nec_init(int cpu)
{
	nec_state_t *nec_state = &sChips[cpu];

	unsigned int i, j, c;

	static const WREGS wreg_name[8]={ AW, CW, DW, BW, SP, BP, IX, IY };
	static const BREGS breg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	for (i = 0; i < 256; i++)
	{
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1) c++;
		parity_table[i] = !(c & 1);
	}

	for (i = 0; i < 256; i++)
	{
		Mod_RM.reg.b[i] = breg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = wreg_name[(i & 0x38) >> 3];
	}

	for (i = 0xc0; i < 0x100; i++)
	{
		Mod_RM.RM.w[i] = wreg_name[i & 7];
		Mod_RM.RM.b[i] = breg_name[i & 7];
	}

	memset(nec_state, 0, sizeof(*nec_state));
}

void necCpuOpen(int cpu)
{
	sChipsPtr = &sChips[cpu];
}

void necCpuClose()
{
	sChipsPtr = NULL;
}

unsigned int nec_total_cycles()
{
	return sChipsPtr->cycles_total + (sChipsPtr->cycles_remaining - sChipsPtr->icount);
}

void nec_new_frame()
{
	for (int i = 0; i < 4; i++) {
		nec_state_t *nec_state = &sChips[i];
		nec_state->cycles_total = 0;
	}
}

void nec_set_vector(int vector)
{
	nec_state_t *nec_state = sChipsPtr;
	nec_state->vector = vector;
}

void necRunEnd()
{
	sChipsPtr->stop_run = 1;
}

void necIdle(int cycles)
{
	sChipsPtr->icount -= cycles;
}

int nec_execute(int cycles)
{
	nec_state_t *nec_state = sChipsPtr;
	int prev_ICount;

	nec_state->icount = cycles;
	nec_state->cycles_remaining = cycles;

	if (nec_state->halted)
	{
		nec_state->icount = 0;
		//debugger_instruction_hook(device, (Sreg(PS)<<4) + nec_state->ip);
		return cycles;
	}

	while((nec_state->icount>0) && (!nec_state->stop_run)) {
		/* Dispatch IRQ */
		if (nec_state->pending_irq && nec_state->no_interrupt==0)
		{
			if (nec_state->pending_irq & NMI_IRQ)
				external_int(nec_state);
			else if (nec_state->IF)
				external_int(nec_state);
		}

		/* No interrupt allowed between last instruction and this one */
		if (nec_state->no_interrupt)
			nec_state->no_interrupt--;

		//debugger_instruction_hook(device, (Sreg(PS)<<4) + nec_state->ip);
		prev_ICount = nec_state->icount;
		nec_instruction[fetchop(nec_state)](nec_state);
		do_prefetch(nec_state, prev_ICount);
	}

	nec_state->cycles_total += cycles - nec_state->icount;
	nec_state->cycles_remaining = 0;
	nec_state->stop_run = 0;

	return (cycles - nec_state->icount);
}

void necInit(int cpu, int type)
{
	nec_state_t *nec_state = &sChips[cpu];

	nec_init(cpu);

	switch (type)
	{
		case V20_TYPE:
		{
			nec_state->fetch_xor = 0;
			nec_state->chip_type=V20_TYPE;
			nec_state->prefetch_size = 4;		/* 3 words */
			nec_state->prefetch_cycles = 4;		/* four cycles per byte */
		}
		break;

		case V30_TYPE:
		{
			nec_state->fetch_xor = 0;
			nec_state->chip_type=V30_TYPE;
			nec_state->prefetch_size = 6;		/* 3 words */
			nec_state->prefetch_cycles = 2;		/* two cycles per byte / four per word */
		}
		break;

		case V33_TYPE:
		{
			nec_state->fetch_xor = 0;
			nec_state->chip_type=V33_TYPE;
			nec_state->prefetch_size = 6;
			/* FIXME: Need information about prefetch size and cycles for V33.
		         * complete guess below, nbbatman will not work
		         * properly without. */
			nec_state->prefetch_cycles = 1;		/* two cycles per byte / four per word */
		}
		break;
	}
}

int necGetPC(int n)
{
	if (n == -1) { // current cpu
		return ((sChipsPtr->sregs[PS]<<4) + sChipsPtr->ip);
	} else {
		nec_state_t *nec_state = &sChips[n];
		return ((nec_state->sregs[PS]<<4) + nec_state->ip);
	}
}

void necScan(int cpu, int nAction)
{
	nec_state_t *nec_state = &sChips[cpu];

	if (nAction & ACB_DRIVER_DATA) {
		struct BurnArea ba;
		char szText[] = "NEC #0";
		szText[5] = '1' + cpu;

		memset(&ba, 0, sizeof(ba));

		ba.Data	  = (unsigned char*)nec_state;
		ba.nLen	  = sizeof(nec_state_t);
		ba.szName = szText;
		BurnAcb(&ba);
	}
}


#if 0

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( nec )
{
	nec_state_t *nec_state = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:					set_irq_line(nec_state, 0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		set_irq_line(nec_state, INPUT_LINE_NMI, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_POLL:	set_irq_line(nec_state, NEC_INPUT_LINE_POLL, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + NEC_PC:
			if( info->i - (Sreg(PS)<<4) < 0x10000 )
			{
				nec_state->ip = info->i - (Sreg(PS)<<4);
			}
			else
			{
				Sreg(PS) = info->i >> 4;
				nec_state->ip = info->i & 0x0000f;
			}
			break;
		case CPUINFO_INT_REGISTER + NEC_IP:				nec_state->ip = info->i;							break;
		case CPUINFO_INT_SP:
			if( info->i - (Sreg(SS)<<4) < 0x10000 )
			{
				Wreg(SP) = info->i - (Sreg(SS)<<4);
			}
			else
			{
				Sreg(SS) = info->i >> 4;
				Wreg(SP) = info->i & 0x0000f;
			}
			break;
		case CPUINFO_INT_REGISTER + NEC_SP:				Wreg(SP) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_FLAGS:			ExpandFlags(info->i);					break;
		case CPUINFO_INT_REGISTER + NEC_AW:				Wreg(AW) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_CW:				Wreg(CW) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_DW:				Wreg(DW) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_BW:				Wreg(BW) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_BP:				Wreg(BP) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_IX:				Wreg(IX) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_IY:				Wreg(IY) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_ES:				Sreg(DS1) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_CS:				Sreg(PS) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_SS:				Sreg(SS) = info->i;					break;
		case CPUINFO_INT_REGISTER + NEC_DS:				Sreg(DS0) = info->i;					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static CPU_GET_INFO( nec )
{
	nec_state_t *nec_state = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	int flags;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(nec_state_t);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 80;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 20;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:					info->i = (nec_state->pending_irq & INT_IRQ) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		info->i = nec_state->nmi_state;				break;
		case CPUINFO_INT_INPUT_STATE + NEC_INPUT_LINE_POLL:	info->i = nec_state->poll_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					/* not supported */						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + NEC_PC:				info->i = ((Sreg(PS)<<4) + nec_state->ip);	break;
		case CPUINFO_INT_REGISTER + NEC_IP:				info->i = nec_state->ip;							break;
		case CPUINFO_INT_SP:							info->i = (Sreg(SS)<<4) + Wreg(SP); break;
		case CPUINFO_INT_REGISTER + NEC_SP:				info->i = Wreg(SP);					break;
		case CPUINFO_INT_REGISTER + NEC_FLAGS:			info->i = CompressFlags();				break;
		case CPUINFO_INT_REGISTER + NEC_AW:				info->i = Wreg(AW);					break;
		case CPUINFO_INT_REGISTER + NEC_CW:				info->i = Wreg(CW);					break;
		case CPUINFO_INT_REGISTER + NEC_DW:				info->i = Wreg(DW);					break;
		case CPUINFO_INT_REGISTER + NEC_BW:				info->i = Wreg(BW);					break;
		case CPUINFO_INT_REGISTER + NEC_BP:				info->i = Wreg(BP);					break;
		case CPUINFO_INT_REGISTER + NEC_IX:				info->i = Wreg(IX);					break;
		case CPUINFO_INT_REGISTER + NEC_IY:				info->i = Wreg(IY);					break;
		case CPUINFO_INT_REGISTER + NEC_ES:				info->i = Sreg(DS1);					break;
		case CPUINFO_INT_REGISTER + NEC_CS:				info->i = Sreg(PS);					break;
		case CPUINFO_INT_REGISTER + NEC_SS:				info->i = Sreg(SS);					break;
		case CPUINFO_INT_REGISTER + NEC_DS:				info->i = Sreg(DS0);					break;
		case CPUINFO_INT_REGISTER + NEC_PENDING:		info->i = nec_state->pending_irq;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(nec);			break;
		case CPUINFO_FCT_INIT:							/* set per-CPU */						break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(nec);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(nec);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(necv);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(nec);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &nec_state->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "NEC");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "NEC V-Series");		break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "2.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Bryan McPhail (V25/V35 support added by Alex W. Jackson)"); break;

		case CPUINFO_STR_FLAGS:
            flags = CompressFlags();
            sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                flags & 0x8000 ? 'N':'E',
                flags & 0x4000 ? '?':'.',
                flags & 0x2000 ? '?':'.',
                flags & 0x1000 ? '?':'.',
                flags & 0x0800 ? 'O':'.',
                flags & 0x0400 ? 'D':'.',
                flags & 0x0200 ? 'I':'.',
                flags & 0x0100 ? 'T':'.',
                flags & 0x0080 ? 'S':'.',
                flags & 0x0040 ? 'Z':'.',
                flags & 0x0020 ? '?':'.',
                flags & 0x0010 ? 'A':'.',
                flags & 0x0008 ? '?':'.',
                flags & 0x0004 ? 'P':'.',
                flags & 0x0002 ? '.':'?',
                flags & 0x0001 ? 'C':'.');
            break;

        case CPUINFO_STR_REGISTER + NEC_PC:				sprintf(info->s, "PC:%05X", (Sreg(PS)<<4) + nec_state->ip); break;
        case CPUINFO_STR_REGISTER + NEC_IP:				sprintf(info->s, "IP:%04X", nec_state->ip); break;
        case CPUINFO_STR_REGISTER + NEC_SP:				sprintf(info->s, "SP:%04X", Wreg(SP)); break;
        case CPUINFO_STR_REGISTER + NEC_FLAGS:			sprintf(info->s, "F:%04X", CompressFlags()); break;
        case CPUINFO_STR_REGISTER + NEC_AW:				sprintf(info->s, "AW:%04X", Wreg(AW)); break;
        case CPUINFO_STR_REGISTER + NEC_CW:				sprintf(info->s, "CW:%04X", Wreg(CW)); break;
        case CPUINFO_STR_REGISTER + NEC_DW:				sprintf(info->s, "DW:%04X", Wreg(DW)); break;
        case CPUINFO_STR_REGISTER + NEC_BW:				sprintf(info->s, "BW:%04X", Wreg(BW)); break;
        case CPUINFO_STR_REGISTER + NEC_BP:				sprintf(info->s, "BP:%04X", Wreg(BP)); break;
        case CPUINFO_STR_REGISTER + NEC_IX:				sprintf(info->s, "IX:%04X", Wreg(IX)); break;
        case CPUINFO_STR_REGISTER + NEC_IY:				sprintf(info->s, "IY:%04X", Wreg(IY)); break;
        case CPUINFO_STR_REGISTER + NEC_ES:				sprintf(info->s, "DS1:%04X", Sreg(DS1)); break;
        case CPUINFO_STR_REGISTER + NEC_CS:				sprintf(info->s, "PS:%04X", Sreg(PS)); break;
        case CPUINFO_STR_REGISTER + NEC_SS:				sprintf(info->s, "SS:%04X", Sreg(SS)); break;
        case CPUINFO_STR_REGISTER + NEC_DS:				sprintf(info->s, "DS0:%04X", Sreg(DS0)); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( v20 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v20);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "V20");					break;

		default:										CPU_GET_INFO_CALL(nec);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( v30 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v30);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "V30");					break;

		default:										CPU_GET_INFO_CALL(nec);				break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( v33 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v33);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "V33");					break;

		default:										CPU_GET_INFO_CALL(nec);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(V20, v20);
DEFINE_LEGACY_CPU_DEVICE(V30, v30);
DEFINE_LEGACY_CPU_DEVICE(V33, v33);

#endif
