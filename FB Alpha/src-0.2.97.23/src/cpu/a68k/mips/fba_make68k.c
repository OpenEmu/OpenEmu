/*---------------------------------------------------------------
 * Motorola 68000 32 Bit emulator
 *
 * Copyright 1998-2001 Mike Coates, 	All rights reserved
 *							Darren Olafson
 *---------------------------------------------------------------
 *
 * Thanks to ...
 *
 * Neil Bradley	 (lots of optimisation help & ideas)
 *
 *---------------------------------------------------------------
 * History (so we know what bugs have been fixed)
 *
 * 02.11.98 MJC	- CMPM bug, overwriting first value
 * 04.11.98 MJC	- Debug timing - same as C core
 *						save PC on calls to C memory routines
 * 05.11.98 NS	 - Re-insert changes to make work on OS/2
 * 06.11.98 MJC	- Flags saved on ADDA commands
 *						X set on ADD commands
 * 23.11.98 MJC	- Alternate Memory Read/Write for non DOS ports
 * 24.11.98 CK	 - Add WIN32 specific stuff
 * 25.11.98 DEO	- ABCD Size not initialised
 * 21.12.98 MJC	- Change register saving on Memory Banking
 * 13.01.99 M/D	- Change to new C core disassembler
 * 19.01.99 MJC	- Proper? support for new Interrupt System
 * 17.02.99 MJC	- TRACE68K define added
 *                ABCD,SBCD not keeping Z flag
 *                JMP, JSR for some EA combo's damaging flags
 *                DIVU - Overflow not being set correctly
 *                ASL/ASR - Flag Handling now correct
 *                some minor optimisations
 * 13.03.99 DEO	- Added new cycle timing
 * 24.03.99 MJC	- TRACE68K define removed
 *                NEW INTERRUPT SYSTEM only
 *                interrupt check sped up (when not taken)
 *                STOP checks for interrupt
 * 01.04.99 MJC	- OS2 specifics added
 *                MOVEM reference point moved
 *                Data and Address register mode combined for :-
 *					 	movecodes
 *                dumpx
 * 04.05.99 MJC	- Add Previous PC support to MOVE.B #X,XXXXXX.L (F1 Dream)
 *                ABCD/SBCD could corrupt zero flag
 *                DIVS/DIVU overflow should not update register
 * 22.05.99 MJC	- Complete support of Previous PC on C calls
 *                Some optional bits now selected by DEFINES
 * 27.05.99 MJC	- Interrupt system changed
 * 28.05.99 MJC	- Use DEFINES in more places
 *                Interrupt check running one opcode when taken
 * 16.07.99 MJC	- Reset - Preload next opcode / external call
 *                68010 commands almost complete
 *                more compression on jump table (16k smaller)
 *                Some optimising
 *					 	  shl reg,1 -> add reg,reg
 *                  or ecx,ecx:jz -> jecxz
 * 22.08.99 DEO	- SBCD/ABCD sets N flag same as carry
 * 19.10.99 MJC	- Change DOS memory routines
 *                Change DOS Clobber flags (ESI no longer safe)
 *                Save EAX around memory write where needed
 *                bit commands optimised
 * 25.10.99  MJC  - Was keeping masked register on read/write
 *                if register was preserved over call
 *                ESI assumed 'safe' again
 * 25.10.99  MJC  - Bank ID moved to CPU context
 * 03.11.99 KENJO - VC++6.0 seems not to preserve EDI. Fixed "ABCD -(A0), -(A0)" crash / "roxr (A0)"-type shift crash
 * 13.11.99 KENJO - Fixed "NABC"
 *                Now Win32 uses FASTCALL type call for interrupt callback
 * 09.02.00  MJC  - Check CPU type before allowing 68010/68020 instructions
 *                remove routines for 5 non existant opcodes
 * 05.03.00  MJC  - not command decrement A7 by 1 for bytes
 * 10.03.00  MJC  - as did btst,cmpm and nbcd
 * 22.03.00  MJC  - Divide by zero should not decrement PC by 2 before push
 *                Move memory banking into exception routine
 * 14.04.00 Dave  - BTST missing Opcode
 *                ASL.L > 31 shift
 * 20.04.00  MJC  - TST.B Also missing A7 specific routine
 *                - Extra Define A7ROUTINE to switch between having seperate
 *                routines for +-(A7) address modes.
 * 24.06.00  MJC  - ADDX/SUBX +-(A7) routines added.
 *                TAS should not touch X flag
 *                LSL/LSR EA not clearing V flag
 *                CHK not all opcodes in jump table
 *                Add define to mask status register
 * 04.07.00  MJC  - Keep high byte of Program Counter on Bxx and Jxx
 *                Fix flag handling on NEGX
 *                Really fix ADDX/SUBX +-(A7)
 *                PC could be set wrong after CHK.W instruction
 *                DIVS/DIVU always clear C flag
 *                ABCD/SBCD missing +-(A7) Routine
 *                TAS missing +-(A7) Routine
 *                Bitwise Static missing +-(A7) Routine
 *                CMPM missing +-(A7) Routine
 * 30.09.00 DEO	- added mull, divl, bfextu
 *                added '020 addressing modes
 *                fixed $6xff branching
 * 23.01.01 MJC	- Spits out seperate code for 68000 & 68020
 *                allows seperate optimising!
 * 17.02.01 MJC	- Support for encrypted PC relative fetch calls
 * 11.03.01 GUTI  - change some cmp reg,0 and or reg,reg with test
 * 13.03.01 MJC	- Single Icount for Asm & C cores
 * 16.05.01 ASG	- use push/pop around mem calls instead of store to safe_REG
 *                optimized a bit the 020 extension word decoder
 *                removed lots of unnecessary code in branches
 *---------------------------------------------------------------
 * Known Problems / Bugs
 *
 * 68000
 * None - Let us know if you find any!
 *
 * 68010
 * Instructions that are supervisor only as per 68000 spec.
 * move address space not implemented.
 *
 * 68020
 * only long Bcc instruction implemented.
 *---------------------------------------------------------------
 * Notes
 *
 * STALLCHECK should be defined for Pentium Class
 *							 undefined for P2/Celerons
 *
 * ALIGNMENT is normally 4, but seems faster on my P2 as 0 !
 *
 *---------------------------------------------------------------
 *
 * Future Changes
 *
 * 68020 instructions to be completed
 * assembler memory routines												+
 *
 * and anything else that takes our fancy!
 *---------------------------------------------------------------*/

/* Specials - Switch what program allows/disallows */

#undef  STALLCHECK			/* Affects fetching of Opcode */
#undef  SAVEPPC				/* Save Previous PC (!!! FBA setting) */
#define ENCRYPTED			/* PC relative = decrypted */
#define ASMBANK		10		/* Memory banking algorithm to use LVR - (bank size = 1 << ASMBANK (8 = 16)) */
#define A7ROUTINE			/* Define to use separate routines for -(a7)/(a7)+ */
#define ALIGNMENT	4	  	/* Alignment to use for branches */
#undef  MASKCCR				/* Mask the status register to filter out unused bits */
#define KEEPHIGHPC			/* Keep or Ignore bits 24-31 */
#define QUICKZERO			/* selects XOR r,r or MOV r,0 */
#undef  RLETABLE			/* Don't use RLE table */
#undef  TESTTABLE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* New Disassembler */

char *	codebuf;
int		DisOp;

#define cpu_readmem24bew(addr)			(0)
#define cpu_readmem24bew_word(addr)	 	(DisOp)

#define MEMORY_H	/* so memory.h will not be included... */
/*
#include "d68k.c"
*/
#undef MEMORY_H

#undef cpu_readmem24bew
#undef cpu_readmem24bew_word


/*
 * Defines used by Program
 *
 */

#define VERSION	 "0.30"

#define TRUE -1
#define FALSE 0

#define ZERO		 0
#define AT			 1
#define V0			 2
#define V1			 3
#define A0			 4
#define TMPREG3		A0
#define A1			 5
#define TMPREG2		A1
#define A2			 6
#define TMPREG1		A2
#define A3			 7
#define TMPREG0		A3
#define T0			 8
#define T1			 9
#define T2			10
#define T3			11
#define T4			12
#define T5			13
#define T6			14
#define EFFADDR		T6
#define T7			15
#define ICNT		T7
#define S0			16
#define FLAG_C		S0
#define S1			17
#define FLAG_V		S1
#define S2			18
#define FLAG_Z		S2
#define S3			19
#define FLAG_N		S3
#define S4			20
#define FLAG_X		S4
#define S5			21
#define S6			22
#define BASEPC		S6
#define S7			23
#define PC			S7
#define T8			24
#define OPCODE		T8
#define T9			25
#define K0			26
#define K1			27
#define GP			28
#define SP			29
#define FP			30
#define S8			FP
#define OPCODETBL	S8
#define RA			31


#define NORMAL 0
#define PCREL  1

//#ifdef __ELF__
#define PREF ""
//#else
//#define PREF "_"
//#endif

/* Register Location Offsets */

#define MEMBASE		S5
#define MEMBASESTR	"$21"

#define ICOUNT				PREF "m68k_ICount"
#define OP_ROM				PREF "OP_ROM"
#define OP_RAM				PREF "OP_RAM"

#define MEMINTF_DBGCALLBACK	"0x78(" MEMBASESTR ")"
#define MEMINTF_READBYTE	"0x7C(" MEMBASESTR ")"
#define MEMINTF_READWORD	"0x80(" MEMBASESTR ")"
#define MEMINTF_READLONG	"0x84(" MEMBASESTR ")"
#define MEMINTF_WRITEBYTE	"0x88(" MEMBASESTR ")"
#define MEMINTF_WRITEWORD	"0x8C(" MEMBASESTR ")"
#define MEMINTF_WRITELONG	"0x90(" MEMBASESTR ")"
#define MEMINTF_CHANGEPC	"0x94(" MEMBASESTR ")"
#define MEMINTF_READBYTEPC	"0x98(" MEMBASESTR ")"
#define MEMINTF_READWORDPC	"0x9C(" MEMBASESTR ")"
#define MEMINTF_READLONGPC	"0xA0(" MEMBASESTR ")"
#define MEMINTF_DIR16		"0xA4(" MEMBASESTR ")"
#define MEMINTF_DIR32		"0xA8(" MEMBASESTR ")"

#define REG_A7				"0x3C(" MEMBASESTR ")"
#define REG_USP				"0x68(" MEMBASESTR ")"
#define REG_ISP				"0x40(" MEMBASESTR ")"
#define REG_SRH				"0x44(" MEMBASESTR ")"
#define REG_CCR				"0x48(" MEMBASESTR ")"
#define REG_PC				"0x4C(" MEMBASESTR ")"
#define REG_IRQ				"0x50(" MEMBASESTR ")"
#define REG_IRQ_CALLBACK	"0x54(" MEMBASESTR ")"
#define REG_RESET_CALLBACK	"0x5C(" MEMBASESTR ")"
#define FULLPC				"0x74(" MEMBASESTR ")"
#define CPUVERSION			"0x70(" MEMBASESTR ")"
#define PREVIOUSPC			"0x58(" MEMBASESTR ")"

/* 68010 Regs */

#define REG_SFC_BASE		0x60
#define REG_VBR				"0x6C(" MEMBASESTR ")"
#define REG_SFC				"0x60(" MEMBASESTR ")"
#define REG_DFC				"0x64(" MEMBASESTR ")"

/* 11 registers + space for 10 work registers + A0 - A3 : Stack must be kept on 16 bytes boundaries */
#define STACKFRAME_SIZE		(7 * (4 * 4))
#if (MEMBASE == GP)
#define LOCALVARBASE		(STACKFRAME_SIZE - 0x2C)
#else
#define LOCALVARBASE		(STACKFRAME_SIZE - 0x28)
#endif
#define MINSTACK_POS		(1 * (4 * 4))

#define FASTCALL_FIRST_REG	A0
#define FASTCALL_SECOND_REG	A1
#define FASTCALL_THIRD_REG	A2
#define FASTCALL_FOURTH_REG	A3


#define FLAGS_NONE		0
#define FLAGS_ADD		1
#define FLAGS_SUB		2
#define FLAGS_CMP		3
#define FLAGS_MOVE		4
#define FLAGS_BITOP		5
#define FLAGS_ADDX		6
#define FLAGS_SUBX		7

/*
 * Global Variables
 *
 */

FILE *fp			= NULL;

char *comptab		= NULL;
char *CPUtype		= NULL;

int  CPU			= 0;
int  CheckInterrupt = 0;
int  ExternalIO	  	= 0;
int  Opcount		= 0;
int  TimingCycles	= 0;
int  AddEACycles	= 0;
int  AccessType		= NORMAL;
int  LocalStack		= LOCALVARBASE;
int  PreDecLongMove = 0;

/* External register preservation */

static char SavedRegs[] = "ZA--------------SSSSSSSS----GSFR";



/* Jump Table */

int OpcodeArray[65536];

/* Lookup Arrays */

static char* regnameslong[] =
{ "$0","$1","$2","$3","$4","$5","$6","$7","$8","$9","$10","$11","$12","$13","$14","$15","$16","$17","$18","$19","$20","$21","$22","$23","$24","$25","$26","$27","$28","$29","$30","$31" };
//{ "$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3","$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7","$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7","$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra" };

//static char* regnamesword[] =
//{ "$0","$1","$2","$3","$4","$5","$6","$7","$8","$9","$10","$11","$12","$13","$14","$15","$16","$17","$18","$19","$20","$21","$22","$23","$24","$25","$26","$27","$28","$29","$30","$31" };
//
//static char* regnamesshort[] =
//{ "$0","$1","$2","$3","$4","$5","$6","$7","$8","$9","$10","$11","$12","$13","$14","$15","$16","$17","$18","$19","$20","$21","$22","$23","$24","$25","$26","$27","$28","$29","$30","$31" };


/*********************************/
/* Conversion / Utility Routines */
/*********************************/

/* Convert EA to Address Mode Number
 *
 * 0	Dn
 * 1	An
 * 2	(An)
 * 3	(An)+
 * 4	-(An)
 * 5	x(An)
 * 6	x(An,xr.s)
 * 7	x.w
 * 8	x.l
 * 9	x(PC)
 * 10  x(PC,xr.s)
 * 11  #x,SR,CCR		Read = Immediate, Write = SR or CCR
 *							 in order to read SR, use READCCR
 * 12-15  INVALID
 *
 * 19  (A7)+
 * 20  -(A7)
 *
 */

int EAtoAMN(int EA, int Way)
{
	int Work;

	if (Way) {
		Work = (EA & 0x7);

		if (Work == 7) Work += ((EA & 0x38) >> 3);

		if (((Work == 3) || (Work == 4)) && (((EA & 0x38) >> 3) == 7)) {
			Work += 16;
		}
	} else {
		Work = (EA & 0x38) >> 3;

		if (Work == 7) Work += (EA & 7);

		if (((Work == 3) || (Work == 4)) && ((EA & 7) == 7)) {
			Work += 16;
		}
	}

	return Work;
}

/*
 * Generate Main or Sub label
 */

char* GenerateLabel(int ID, int Type)
{
	static int LabID, LabNum;
/*
	static char disasm[80];
	char	*dis = disasm;
*/
	if (Type == 0) {
		CheckInterrupt = 0;			/* No need to check for Interrupts */
		ExternalIO = 0;				/* Not left Assembler Yet */
		TimingCycles = 0;			/* No timing info for this command */
		AddEACycles = 1;			/* default to add in EA timing */
		Opcount++;					/* for screen display */
		PreDecLongMove = 0;

		DisOp = ID;
/*
		m68k_disassemble(dis,0);
		sprintf(codebuf, "OP%d_%4.4x:\t\t\t\t; %s", CPU, ID, dis);
*/
		sprintf(codebuf, "OP%d_%4.4x:\t\t\t\t#", CPU, ID);

		LabID  = ID;
		LabNum = 0;
	} else {
		LabNum++;
		sprintf(codebuf, "OP%d_%4.4x_%1x", CPU,LabID, LabNum);
	}

	return codebuf;
}

/*
 * Generate Alignment Line
 */

void Align(void)
{
	//fprintf(fp, "\t\t .align %d\n\n", ALIGNMENT);
}

void ClearRegister(int regno)
{
	fprintf(fp, "\t\t and   %s,$0,$0\n", regnameslong[regno]);
}

/*
 * Immediate 3 bit data
 *
 * 0=8, anything else is itself
 *
 * Again, several ways to achieve this
 *
 * ECX contains data as 3 lowest bits
 *
 */

void Immediate8(void)
{
	fprintf(fp, "\t\t addiu %s,%s,-1       \t # Move range down\n", regnameslong[OPCODE], regnameslong[OPCODE]);
	fprintf(fp, "\t\t andi  %s,%s,0x07     \t # Mask out lower bits\n", regnameslong[OPCODE], regnameslong[OPCODE]);
	fprintf(fp, "\t\t addiu %s,%s,1        \t # correct range\n", regnameslong[OPCODE], regnameslong[OPCODE]);
}

/*
 * This will check for bank changes before
 * resorting to calling the C bank select code
 *
 * Most of the time it does not change!
 *
 */

/* forward used by MemoryBanking */
void Exception(int Number, char* DelaySlot);

void MemoryBanking(int pc, int LocalStack)
{
	/* check for odd address */
	fprintf(fp, "\t\t andi  %s,%s,0x01\n", regnameslong[T9], regnameslong[pc != 0 ? pc : PC]);
	fprintf(fp, "\t\t beq   %s,$0,2f\n", regnameslong[T9]);
	if (pc != 0) {
		fprintf(fp, "\t\t addu  %s,%s,%s     \t # Delay slot\n", regnameslong[PC], regnameslong[BASEPC], regnameslong[pc]);
	} else {
		pc = TMPREG2;
		fprintf(fp, "\t\t subu  %s,%s,%s     \t # Delay slot\n", regnameslong[pc], regnameslong[PC], regnameslong[BASEPC]);
	}

	/* trying to run at an odd address */
	Exception(3, NULL);

	/* Keep Whole PC */

	fprintf(fp, "\t2:\n");

/* ASG - always call to the changepc subroutine now, since the number of */
/* bits varies based on the memory model */
#ifdef KEEPHIGHPC
	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[pc], FULLPC);
#endif

	/* Mask to n bits */
	fprintf(fp, "\t\t lw    %s,%smem_amask\n", regnameslong[TMPREG1], PREF);
	//fprintf(fp, "\t\t la    %s,%smem_amask\n", regnameslong[TMPREG0], PREF);
	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_CHANGEPC);
	//fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[TMPREG1], regnameslong[TMPREG0]);
	fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[PC], regnameslong[pc], regnameslong[TMPREG1]);

	/* Call Banking Routine */

	if (SavedRegs[PC] == '-')	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[PC], REG_PC);

	if (SavedRegs[FLAG_C] == '-')	fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_C], --LocalStack, regnameslong[SP]);
	if (SavedRegs[FLAG_V] == '-')	fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_V], --LocalStack, regnameslong[SP]);
	if (SavedRegs[FLAG_Z] == '-')	fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_Z], --LocalStack, regnameslong[SP]);
	if (SavedRegs[FLAG_N] == '-')	fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_N], --LocalStack, regnameslong[SP]);
	if (SavedRegs[FLAG_X] == '-')	fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_X], --LocalStack, regnameslong[SP]);
	assert(LocalStack >= MINSTACK_POS);

	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[ICNT], ICOUNT);
	fprintf(fp, "\t\t jalr  %s\n", regnameslong[T9]);
	fprintf(fp, "\t\t or    %s,$0,%s   \t # Delay slot\n", regnameslong[FASTCALL_FIRST_REG], regnameslong[PC]);
	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[ICNT], ICOUNT);

	if (SavedRegs[FLAG_X] == '-')	fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_X], LocalStack++, regnameslong[SP]);
	if (SavedRegs[FLAG_N] == '-')	fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_N], LocalStack++, regnameslong[SP]);
	if (SavedRegs[FLAG_Z] == '-')	fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_Z], LocalStack++, regnameslong[SP]);
	if (SavedRegs[FLAG_V] == '-')	fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_V], LocalStack++, regnameslong[SP]);
	if (SavedRegs[FLAG_C] == '-')	fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_C], LocalStack++, regnameslong[SP]);

	if (SavedRegs[PC] == '-')	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[PC], REG_PC);

	/* Update our copy */

	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[BASEPC], OP_ROM);
	fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[PC], regnameslong[PC], regnameslong[BASEPC]);

	fprintf(fp, " # End of Banking code:\n");
}

/*
 * Update Previous PC value
 *
 */

void SavePreviousPC(void)
{
#ifdef SAVEPPC
	fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T9], regnameslong[PC], regnameslong[BASEPC]);
	fprintf(fp, "\t\t sw    %s,%s    \t # Keep Previous PC\n", regnameslong[T9], PREVIOUSPC);
#endif
}

/*
 * Complete Opcode handling
 *
 * Any tidying up, end code
 *
 */

void Completed(void)
{

	/* Flag Processing to be finished off ? */

	AccessType = NORMAL;

	/* Use assembler timing routines */

	if (TimingCycles != 0) {
		if (TimingCycles > 32767) {
			fprintf(fp, "\t\t li    %s,%d\n", regnameslong[T9], TimingCycles);
			fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[ICNT], regnameslong[ICNT], regnameslong[T9]);
		} else {
			if (TimingCycles != -1)
				fprintf(fp, "\t\t addiu %s,%s,%d\n", regnameslong[ICNT], regnameslong[ICNT], -TimingCycles);
		}
	}
	//fprintf(fp, "\t\t bltz  %s,MainExit\n", regnameslong[ICNT]);
	//fprintf(fp, "\t\t nop\n\n");

	if (CheckInterrupt) {
		fprintf(fp, "\t\t bgez  %s,3f\n", regnameslong[ICNT]);
		fprintf(fp, "\t\t lbu   %s,%s     \t # Delay slot\n", regnameslong[TMPREG0], REG_IRQ);
		fprintf(fp, "\t\t j     MainExit\n");
		fprintf(fp, "\t3:\n");
		fprintf(fp, " # Check for Interrupt waiting\n\n");
		fprintf(fp, "\t\t andi  %s,%s,0x07       \t # Delay slot\n", regnameslong[TMPREG0], regnameslong[TMPREG0]);
		//fprintf(fp, "\t\t bne   %s,$0,interrupt\n", regnameslong[TMPREG0]);
		fprintf(fp, "\t\t beq   %s,$0,3f\n", regnameslong[TMPREG0]);
		fprintf(fp, "\t\t nop                    \t # Delay slot\n");
		fprintf(fp, "\t\t j     interrupt\n");
		fprintf(fp, "\t3:\n");
		/* 16 bit memory */
		fprintf(fp, "\t\t lhu   %s,0x00(%s)    \t # Delay slot\n", regnameslong[OPCODE], regnameslong[PC]);

		//fprintf(fp, "\t\t jmp   [%s_OPCODETABLE+ecx*4]\n\n", CPUtype);
		fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[TMPREG0], regnameslong[OPCODE]);
		fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[TMPREG0], regnameslong[TMPREG0], regnameslong[OPCODETBL]);
		fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[TMPREG0], regnameslong[TMPREG0]);
		fprintf(fp, "\t\t jr    %s\n", regnameslong[TMPREG0]);
		fprintf(fp, "\t\t nop                    \t # Delay slot\n\n");
	} else {

		/* 16 bit memory */
		fprintf(fp, "\t\t bgez  %s,3f\n", regnameslong[ICNT]);
		fprintf(fp, "\t\t lhu   %s,0x00(%s)    \t # Delay slot\n", regnameslong[OPCODE], regnameslong[PC]);
		fprintf(fp, "\t\t j     MainExit\n");
		fprintf(fp, "\t3:\n");

		//fprintf(fp, "\t\t jmp   [%s_OPCODETABLE+ecx*4]\n\n", CPUtype);
		fprintf(fp, "\t\t sll   %s,%s,2         \t # Delay slot\n", regnameslong[TMPREG0], regnameslong[OPCODE]);
		fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[TMPREG0], regnameslong[TMPREG0], regnameslong[OPCODETBL]);
		fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[TMPREG0], regnameslong[TMPREG0]);
		fprintf(fp, "\t\t jr    %s\n", regnameslong[TMPREG0]);
		fprintf(fp, "\t\t nop                    \t # Delay slot\n\n");
	}
}
/*
 * Flag Routines
 */

void SetFlags(char Size, int Operand, int Rreg, int Sreg1, int Sreg2)
{
	char* Label;
	int Decal;

	assert((Size == 'B') || (Size == 'W') || (Size == 'L'));
	assert((Rreg  != FLAG_C) && (Rreg  != FLAG_V) && (Rreg  != FLAG_Z) && (Rreg  != FLAG_N) && (Rreg  != FLAG_X));
	assert((Sreg1 != FLAG_C) && (Sreg1 != FLAG_V) && (Sreg1 != FLAG_Z) && (Sreg1 != FLAG_N) && (Sreg1 != FLAG_X));
	assert((Sreg2 != FLAG_C) && (Sreg2 != FLAG_V) && (Sreg2 != FLAG_Z) && (Sreg2 != FLAG_N) && (Sreg2 != FLAG_X));

	Decal = Size == 'L' ? 31 : (Size == 'W' ? 15 : 7);

	switch (Operand) {

		case FLAGS_ADD:
			assert((Rreg  != T9) && (Rreg  != Sreg1) && (Rreg != Sreg2));
			assert((Sreg1 != T9) && (Sreg1 != Sreg2));
			assert(Sreg2 != T9);
			fprintf(fp, "\t\t sltu  %s,%s,%s       \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[Rreg], regnameslong[Sreg2]);
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[Sreg1], regnameslong[Sreg2]);
			fprintf(fp, "\t\t nor   %s,$0,%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[T9], regnameslong[Rreg], regnameslong[Sreg1]);
			fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], regnameslong[T9]);
			fprintf(fp, "\t\t srl   %s,%s,%d%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], Decal, Size == 'L' ? "        \t # Set Overflow" : "");
			if (Size != 'L') {
				fprintf(fp, "\t\t andi  %s,%s,0x01     \t # Set Overflow\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
				fprintf(fp, "\t\t se%c  %s,%s\n", Size == 'W' ? 'h' : 'b', regnameslong[T9], regnameslong[Rreg]);
				fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[T9]);
				fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[T9]);
			} else {
				fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[Rreg]);
				fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[Rreg]);
			}
			fprintf(fp, "\t\t or    %s,$0,%s      \t # Copy Carry to X\n", regnameslong[FLAG_X], regnameslong[FLAG_C]);
			break;

		case FLAGS_ADDX:
			assert((Rreg  != T9) && (Rreg  != Sreg1) && (Rreg != Sreg2));
			assert((Sreg1 != T9) && (Sreg1 != Sreg2));
			assert(Sreg2 != T9);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T9], regnameslong[Sreg2], regnameslong[FLAG_X]);
			fprintf(fp, "\t\t sltu  %s,%s,%s       \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[Rreg], regnameslong[T9]);
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[Sreg1], regnameslong[Sreg2]);
			fprintf(fp, "\t\t nor   %s,$0,%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[T9], regnameslong[Rreg], regnameslong[Sreg1]);
			fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], regnameslong[T9]);
			fprintf(fp, "\t\t srl   %s,%s,%d%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], Decal, Size == 'L' ? "        \t # Set Overflow" : "");
			if (Size != 'L') {
				fprintf(fp, "\t\t andi  %s,%s,0x01     \t # Set Overflow\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
				fprintf(fp, "\t\t se%c  %s,%s\n", Size == 'W' ? 'h' : 'b', regnameslong[T9], regnameslong[Rreg]);
				fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[T9]);
				fprintf(fp, "\t\t sltiu %s,%s,1\n", regnameslong[T9], regnameslong[T9]);
			} else {
				fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[Rreg]);
				fprintf(fp, "\t\t sltiu %s,%s,1\n", regnameslong[T9], regnameslong[Rreg]);
			}
			fprintf(fp, "\t\t and   %s,%s,%s      \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z], regnameslong[T9]);
			fprintf(fp, "\t\t or    %s,$0,%s      \t # Copy Carry to X\n", regnameslong[FLAG_X], regnameslong[FLAG_C]);
			break;

		case FLAGS_SUB:
		case FLAGS_CMP:
			assert((Rreg  != T9) && (Rreg  != Sreg1) && (Rreg != Sreg2));
			assert((Sreg1 != T9) && (Sreg1 != Sreg2));
			assert(Sreg2 != T9);
			fprintf(fp, "\t\t sltu  %s,%s,%s       \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[Sreg1], regnameslong[Sreg2]);
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[Sreg1], regnameslong[Sreg2]);
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[T9], regnameslong[Rreg], regnameslong[Sreg1]);
			fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], regnameslong[T9]);
			fprintf(fp, "\t\t srl   %s,%s,%d%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], Decal, Size == 'L' ? "        \t # Set Overflow" : "");
			if (Size != 'L') {
				fprintf(fp, "\t\t andi  %s,%s,0x01     \t # Set Overflow\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
				fprintf(fp, "\t\t se%c  %s,%s\n", Size == 'W' ? 'h' : 'b', regnameslong[T9], regnameslong[Rreg]);
				fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[T9]);
				fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[T9]);
			} else {
				fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[Rreg]);
				fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[Rreg]);
			}
			if (Operand == FLAGS_SUB)
				fprintf(fp, "\t\t or    %s,$0,%s      \t # Copy Carry to X\n", regnameslong[FLAG_X], regnameslong[FLAG_C]);
			break;

		case FLAGS_SUBX:
			assert((Rreg  != T9) && (Rreg  != Sreg1) && (Rreg != Sreg2));
			assert((Sreg1 != T9) && (Sreg1 != Sreg2));
			assert(Sreg2 != T9);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T9], regnameslong[Sreg2], regnameslong[FLAG_X]);
			fprintf(fp, "\t\t sltu  %s,%s,%s       \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[Sreg1], regnameslong[T9]);
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[Sreg1], regnameslong[Sreg2]);
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[T9], regnameslong[Rreg], regnameslong[Sreg1]);
			fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], regnameslong[T9]);
			fprintf(fp, "\t\t srl   %s,%s,%d%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], Decal, Size == 'L' ? "        \t # Set Overflow" : "");
			if (Size != 'L') {
				fprintf(fp, "\t\t andi  %s,%s,0x01     \t # Set Overflow\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
				fprintf(fp, "\t\t se%c  %s,%s\n", Size == 'W' ? 'h' : 'b', regnameslong[T9], regnameslong[Rreg]);
				fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[T9]);
				fprintf(fp, "\t\t sltiu %s,%s,1\n", regnameslong[T9], regnameslong[T9]);
			} else {
				fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[Rreg]);
				fprintf(fp, "\t\t sltiu %s,%s,1\n", regnameslong[T9], regnameslong[Rreg]);
			}
			fprintf(fp, "\t\t and   %s,%s,%s      \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z], regnameslong[T9]);
			fprintf(fp, "\t\t or    %s,$0,%s      \t # Copy Carry to X\n", regnameslong[FLAG_X], regnameslong[FLAG_C]);
			break;

		case FLAGS_BITOP:
			fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Carry\n", regnameslong[FLAG_C]);
			fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Overflow\n", regnameslong[FLAG_V]);
			fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[Rreg]);
			fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[Rreg]);
			break;

		case FLAGS_MOVE:
			fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Carry\n", regnameslong[FLAG_C]);
			fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Overflow\n", regnameslong[FLAG_V]);
			fprintf(fp, "\t\t srl   %s,%s,%d         \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[Rreg], Decal);
			fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[Rreg]);
			break;
	}
}

/******************/
/* Check CPU Type */
/******************/

void CheckCPUtype(int Minimum, char* DelaySlot1, char* DelaySlot2)
{
	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], CPUVERSION);

	if (Minimum == 1) {
		//fprintf(fp, "\t\t beq   %s,$0,ILLEGAL\n", regnameslong[T9]);
		fprintf(fp, "\t\t bne   %s,$0,0f\n", regnameslong[T9]);
		if (DelaySlot1 && (strlen(DelaySlot1) > 0))
			fprintf(fp, DelaySlot1);
		else
			fprintf(fp, "\t\t nop                    \t # Delay slot\n");
	} else {
		fprintf(fp, "\t\t addiu %s,%s,%d\n", regnameslong[T9], regnameslong[T9], -Minimum);
		//fprintf(fp, "\t\t bltz  %s,ILLEGAL\n", regnameslong[T9]);
		fprintf(fp, "\t\t bgez  %s,0f\n", regnameslong[T9]);
		if (DelaySlot1 && (strlen(DelaySlot1) > 0))
			fprintf(fp, DelaySlot1);
		else
			fprintf(fp, "\t\t nop                    \t # Delay slot\n");
	}
	fprintf(fp, "\t\t j     ILLEGAL\n");
	fprintf(fp, "\t0:\n");
	if (DelaySlot2 && (strlen(DelaySlot2) > 0))
		fprintf(fp, DelaySlot2);
	else
		fprintf(fp, "\t\t nop                    \t # Delay slot\n\n");
}

/************************************/
/* Pre-Decrement and Post-Increment */
/************************************/

void IncrementEFFADDR(int Size, int Rreg)
{
	assert((T9 != Rreg) && (EFFADDR != Rreg));

	switch (Size) {
		case 66:

			/* Always does Byte Increment - A7 uses special routine */

			fprintf(fp, "\t\t addiu %s,%s,1\n", regnameslong[T9], regnameslong[EFFADDR]);
			break;

		case 87:

			fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[T9], regnameslong[EFFADDR]);
			break;

		case 76:

			fprintf(fp, "\t\t addiu %s,%s,4\n", regnameslong[T9], regnameslong[EFFADDR]);
			break;
	}
	fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[T9], regnameslong[Rreg]);
}

void DecrementEFFADDR(int Size, int Rreg)
{
	switch (Size) {
		case 66:

			/* Always does Byte Increment - A7 uses special routine */

			fprintf(fp, "\t\t addiu %s,%s,-1\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
			break;

		case 87:

			fprintf(fp, "\t\t addiu %s,%s,-2\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
			break;

		case 76:

			fprintf(fp, "\t\t addiu %s,%s,-4\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
			break;
	}
}

/*
 * Generate an exception
 *
 * if Number = -1 then assume value in V0 already
 *					 code must continue running afterwards
 *
 */

void Exception(int Number, char* DelaySlot)
{
	if (Number > -1) {
		fprintf(fp, "\t\t addiu %s,%s,-2\n", regnameslong[PC], regnameslong[PC]);
		fprintf(fp, "\t\t jal   Exception\n");
		fprintf(fp, "\t\t ori   %s,$0,%d\n\n", regnameslong[V0], Number);
		Completed();
	} else {
		fprintf(fp, "\t\t jal   Exception\n");
		fprintf(fp, "\t\t %s\t # Delay slot\n\n", (DelaySlot != NULL) ? DelaySlot : "nop                    ");
	}
}

/********************/
/* Address Routines */
/********************/

/*
 * Decode flags into Rreg as SR register
 *
 */

void ReadCCR(char Size, int Rreg)
{
	assert((Rreg != FLAG_C) && (Rreg != FLAG_V) && (Rreg != FLAG_Z) && (Rreg != FLAG_N) && (Rreg != FLAG_X));

	if (Size == 'W') {
		fprintf(fp, "\t\t lbu   %s,%s\n", regnameslong[Rreg], REG_SRH);
		fprintf(fp, "\t\t sll   %s,%s,4\n", regnameslong[Rreg], regnameslong[Rreg]);
		fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[FLAG_X]);
	} else {
		fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[Rreg], regnameslong[FLAG_X]);
	}
	fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[Rreg], regnameslong[Rreg]);
	fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[FLAG_N]);
	fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[Rreg], regnameslong[Rreg]);
	fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[FLAG_Z]);
	fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[Rreg], regnameslong[Rreg]);
	fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[FLAG_V]);
	fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[Rreg], regnameslong[Rreg]);
	fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[FLAG_C]);
#ifdef MASKCCR
	fprintf(fp, "\t\t andi  %s,%s,0xA71F     \t # Mask unused bits\n", regnameslong[Rreg], regnameslong[Rreg]);
#endif
}

/*
 * Convert SR into flags
 *
 * Also handles change of mode from Supervisor to User
 *
 * n.b. This is also called by EffectiveAddressWrite
 */

void WriteCCR(char Size, int Sreg)
{
	assert((Sreg != FLAG_C) && (Sreg != FLAG_V) && (Sreg != FLAG_Z) && (Sreg != FLAG_N) && (Sreg != FLAG_X));

	if (Size == 'W') {
		assert(Sreg != T9);

		/* Did we change from Supervisor to User mode ? */

		fprintf(fp, "\t\t andi  %s,%s,0x2000    \t # User Mode ?\n", regnameslong[T9], regnameslong[Sreg]);
		fprintf(fp, "\t\t bne   %s,$0,0f\n", regnameslong[T9]);
		fprintf(fp, "\t\t or    %s,$0,%s       \t # Delay slot\n", regnameslong[FLAG_Z], regnameslong[Sreg]);

		/* Mode Switch - Update A7 */

		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[FLAG_C], REG_A7);
		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[FLAG_V], REG_USP);
		fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[FLAG_C], REG_ISP);
		fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[FLAG_V], REG_A7);

		fprintf(fp, "\t0:\n");
		fprintf(fp, "\t\t srl   %s,%s,8\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z]);
		fprintf(fp, "\t\t sb    %s,%s    \t # T, S & I\n", regnameslong[FLAG_Z], REG_SRH);

		/* Mask may now allow Interrupt */

		CheckInterrupt += 1;
	}

	/* Flags */

	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FLAG_X], regnameslong[Sreg]);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FLAG_N], regnameslong[Sreg]);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FLAG_Z], regnameslong[Sreg]);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FLAG_V], regnameslong[Sreg]);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FLAG_C], regnameslong[Sreg]);
	fprintf(fp, "\t\t andi  %s,%s,0x10\n", regnameslong[FLAG_X], regnameslong[FLAG_X]);
	fprintf(fp, "\t\t andi  %s,%s,0x08\n", regnameslong[FLAG_N], regnameslong[FLAG_N]);
	fprintf(fp, "\t\t andi  %s,%s,0x04\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z]);
	fprintf(fp, "\t\t andi  %s,%s,0x02\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
	fprintf(fp, "\t\t andi  %s,%s,0x01\n", regnameslong[FLAG_C], regnameslong[FLAG_C]);
	fprintf(fp, "\t\t srl   %s,%s,4\n", regnameslong[FLAG_X], regnameslong[FLAG_X]);
	fprintf(fp, "\t\t srl   %s,%s,3\n", regnameslong[FLAG_N], regnameslong[FLAG_N]);
	fprintf(fp, "\t\t srl   %s,%s,2\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z]);
	fprintf(fp, "\t\t srl   %s,%s,1\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
}


/*
 * Interface to Mame memory commands
 *
 * Flags = "ABCDSDB" - set to '-' if not required to preserve
 *			(order EAX,EBX,ECX,EDX,ESI,EDI,EBP)
 *
 * AReg	= Register containing Address
 *
 * Mask	0 : No Masking
 *		  1 : Mask top byte, but preserve register
 *		  2 : Mask top byte, preserve masked register
 */

void Memory_Read(char Size, int AReg, char *Flags, int LocalStack)
{
	int i;

	ExternalIO = 1;

	assert(strlen(Flags) == 32);
	assert(AReg != T9);

	/* Call Mame memory routine */

	/* ASG - changed these to call through the function pointers */

#ifdef ENCRYPTED
	switch (AccessType) {
		case NORMAL :
			switch (Size) {
				case 66 :
					fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_READBYTE);
					break;

				case 87 :
					fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_READWORD);
					break;

				case 76 :
					fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_READLONG);
					break;
			}
			break;

		case PCREL :
			switch (Size) {
				case 66 :
					fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_READBYTEPC);
					break;

				case 87 :
					fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_READWORDPC);
					break;

				case 76 :
					fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_READLONGPC);
					break;
			}
			break;
	}

#else

	switch (Size) {
		case 66 :
			fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_READBYTE);
			break;

		case 87 :
			fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_READWORD);
			break;

		case 76 :
			fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_READLONG);
			break;
	}
#endif

	/* Save ICount */
	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[ICNT], ICOUNT);

	/* Check to see if registers need saving */

	for (i = 1; i < sizeof(SavedRegs) - 1; i++) {
		if ((i == AT) || (i == T9) || (i == PC) || (i == ICNT) || (i == V0) || (i == BASEPC)) continue;
		if ((Flags[i] != '-') && (SavedRegs[i] == '-')) {
			LocalStack -= 4;
			fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[i], LocalStack, regnameslong[SP]);
		}
	}
	assert(LocalStack >= MINSTACK_POS);

	/* Sort Address out */
	if (AReg != FASTCALL_FIRST_REG)
		fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FASTCALL_FIRST_REG], regnameslong[AReg]);

	fprintf(fp, "\t\t jalr  %s\n", regnameslong[T9]);

	/* Save PC */
	fprintf(fp, "\t\t sw    %s,%s    \t # Delay slot\n", regnameslong[PC], REG_PC);

	/* Restore registers */

	/* Check to see if registers need restoring */

	for (i = sizeof(SavedRegs) - 2; i > 0; --i) {
		if ((i == AT) || (i == T9) || (i == PC) || (i == ICNT) || (i == V0) || (i == BASEPC)) continue;
		if ((Flags[i] != '-') && (SavedRegs[i] == '-')) {
			fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[i], LocalStack, regnameslong[SP]);
			LocalStack += 4;
		}
	}

	if ((Flags[PC] != '-') && (SavedRegs[PC] == '-')) {
		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[PC], REG_PC);
	}

	if ((Flags[BASEPC] != '-') && (SavedRegs[BASEPC] == '-')) {
		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[BASEPC], OP_ROM);
	}

	// Update ICount
	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[ICNT], ICOUNT);
}

void Memory_Write(char Size, int AReg, int DReg, char *Flags, int LocalStack)
{
	int i;

	ExternalIO = 1;

	assert(strlen(Flags) == 32);
	assert((AReg != T9) && (DReg != T9) && (AReg != DReg) && (AReg != FASTCALL_SECOND_REG) && (DReg != FASTCALL_FIRST_REG));

	/* ASG - changed these to call through the function pointers */
	switch (Size) {
		case 66 :
			fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_WRITEBYTE);
			break;

		case 87 :
			fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_WRITEWORD);
			break;

		case 76 :
			fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], PreDecLongMove ? MEMINTF_WRITEWORD : MEMINTF_WRITELONG);
			break;
	}

	/* Save ICount */
	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[ICNT], ICOUNT);

	/* Check to see if registers need saving */

	for (i = 1; i < sizeof(SavedRegs) - 1; i++) {
		if ((i == AT) || (i == T9) || (i == PC) || (i == ICNT) || (i == BASEPC)) continue;
		if ((Flags[i] != '-') && (SavedRegs[i] == '-')) {
			LocalStack -= 4;
			fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[i], LocalStack, regnameslong[SP]);
		}
	}

	if (PreDecLongMove) {
		LocalStack -= 4;
		fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[DReg], LocalStack, regnameslong[SP]);
		LocalStack -= 4;
		fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[AReg], LocalStack, regnameslong[SP]);
		//fprintf(fp, "\t\t andi  %s,%s,0xFFFF\n", regnameslong[DReg], regnameslong[DReg]);
		fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[AReg], regnameslong[AReg]);
	}
	assert(LocalStack >= MINSTACK_POS);

	/* Call Mame Routine */

	if (DReg != FASTCALL_SECOND_REG)
		fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[DReg]);
	if (AReg != FASTCALL_FIRST_REG)
		fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FASTCALL_FIRST_REG], regnameslong[AReg]);
	fprintf(fp, "\t\t jalr  %s\n", regnameslong[T9]);
	/* Save PC */
	fprintf(fp, "\t\t sw    %s,%s    \t # Delay slot\n", regnameslong[PC], REG_PC);

	if (PreDecLongMove) {
		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], MEMINTF_WRITEWORD);
		fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[FASTCALL_FIRST_REG], LocalStack, regnameslong[SP]);
		LocalStack += 4;
		fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[FASTCALL_SECOND_REG], LocalStack, regnameslong[SP]);
		LocalStack += 4;
		fprintf(fp, "\t\t jalr  %s\n", regnameslong[T9]);
		fprintf(fp, "\t\t srl   %s,%s,16         \t # Delay slot\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[FASTCALL_SECOND_REG]);
	}

	/* Restore registers */

	/* Check to see if registers need restoring */

	for (i = sizeof(SavedRegs) - 2; i > 0; --i) {
		if ((i == AT) || (i == T9) || (i == PC) || (i == ICNT) || (i == BASEPC)) continue;
		if ((Flags[i] != '-') && (SavedRegs[i] == '-')) {
			fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[i], LocalStack, regnameslong[SP]);
			LocalStack += 4;
		}
	}

	if ((Flags[PC] != '-') && (SavedRegs[PC] == '-')) {
		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[PC], REG_PC);
	}

	if ((Flags[BASEPC] != '-') && (SavedRegs[BASEPC] == '-')) {
		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[BASEPC], OP_ROM);
	}

	// Update ICount
	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[ICNT], ICOUNT);
}

/*
 * Fetch data from Code area
 *
 * Dreg	= Destination Register
 * Extend = Sign Extend Byte/Word to Long
 *
 */

void Memory_Fetch(char Size, int Dreg, int Extend)
{
	assert((Dreg != BASEPC) && (Dreg != PC));

	/* 16 Bit version */
	if (Size == 'B')
		fprintf(fp, "\t\t lb%c   %s,0x00(%s)\n", Extend ? ' ' : 'u', regnameslong[Dreg], regnameslong[PC]);
	else if (Size == 'W')
		fprintf(fp, "\t\t lh%c   %s,0x00(%s)\n", Extend ? ' ' : 'u', regnameslong[Dreg], regnameslong[PC]);
	else if (Size == 'L') {
		assert(Dreg != T9);
		fprintf(fp, "\t\t lhu   %s,0x00(%s)\n", regnameslong[Dreg], regnameslong[PC]);
		fprintf(fp, "\t\t lhu   %s,0x02(%s)\n", regnameslong[T9], regnameslong[PC]);
		fprintf(fp, "\t\t sll   %s,%s,16\n", regnameslong[Dreg], regnameslong[Dreg]);
		fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[Dreg], regnameslong[Dreg], regnameslong[T9]);
	} else {
		fprintf(fp, "DUFF CODE (BAD FETCH)!!!\n");
	}
}

/**********************/
/* Push PC onto Stack */
/**********************/

void PushPC(char *Flags)
{
#ifndef KEEPHIGHPC

	fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[PC], regnameslong[BASEPC]);
	fprintf(fp, "\t\t lw    %s,%s     \t # Push onto Stack\n", regnameslong[FASTCALL_FIRST_REG], REG_A7);
	fprintf(fp, "\t\t addiu %s,%s,-4\n", regnameslong[FASTCALL_FIRST_REG], regnameslong[FASTCALL_FIRST_REG]);
	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[FASTCALL_FIRST_REG], REG_A7);

	Memory_Write('L', FASTCALL_FIRST_REG, FASTCALL_SECOND_REG, Flags, LocalStack);

#else

	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[FASTCALL_SECOND_REG], FULLPC);
	fprintf(fp, "\t\t lui   %s,0xFF00\n", regnameslong[T9]);
	fprintf(fp, "\t\t lw    %s,%s     \t # Push onto Stack\n", regnameslong[FASTCALL_FIRST_REG], REG_A7);
	fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[FASTCALL_SECOND_REG], regnameslong[T9]);
	fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T9], regnameslong[PC], regnameslong[BASEPC]);
	fprintf(fp, "\t\t addiu %s,%s,-4\n", regnameslong[FASTCALL_FIRST_REG], regnameslong[FASTCALL_FIRST_REG]);
	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[FASTCALL_FIRST_REG], REG_A7);
	fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[FASTCALL_SECOND_REG], regnameslong[T9]);

	Memory_Write('L', FASTCALL_FIRST_REG, FASTCALL_SECOND_REG, Flags, LocalStack);

#endif
}

void ExtensionDecode()
{
	Memory_Fetch('W', TMPREG0, FALSE);
	fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);

	/* 68000 Extension */

	fprintf(fp, "\t\t seb   %s,%s\n", regnameslong[TMPREG1], regnameslong[TMPREG0]);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[T9], regnameslong[TMPREG0]);
	fprintf(fp, "\t\t srl   %s,%s,12\n", regnameslong[TMPREG0], regnameslong[TMPREG0]);
	fprintf(fp, "\t\t andi  %s,%s,0x0800\n", regnameslong[T9], regnameslong[T9]);
	fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[TMPREG0], regnameslong[TMPREG0]);
	fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[TMPREG0], regnameslong[TMPREG0], regnameslong[MEMBASE]);
	fprintf(fp, "\t\t bne   %s,$0,0f\n", regnameslong[T9]);
	fprintf(fp, "\t\t lw    %s,0x00(%s)      \t # Delay slot\n", regnameslong[T9], regnameslong[TMPREG0]);
	fprintf(fp, "\t\t seh   %s,%s\n", regnameslong[T9], regnameslong[T9]);
	fprintf(fp, "\t0:\n");
	fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T9], regnameslong[EFFADDR], regnameslong[T9]);
	fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[EFFADDR], regnameslong[T9], regnameslong[TMPREG1]);
}

/* Calculate Effective Address - Return address in T6
 *
 * mode = Effective Address from Instruction
 * Size = Byte,Word or Long
 * Rreg = Register with Register Number in
 *
 * Only for modes 2 - 10
 */

void EffectiveAddressCalculate(int mode, char Size, int Rreg, int Decal, int Add)
{
	assert((Rreg != MEMBASE) && (Rreg != EFFADDR));
	/* timing */

	if ((TimingCycles > 0) && (AddEACycles != 0)) {
		switch (mode) {
			case 2:		/* (An) */
			case 3:		/* (An)+ */
			case 11:	/* #x,SR,CCR */
			case 19:	/* (A7)+ */
				TimingCycles += 4;
				break;

			case 4:		/* -(An) */
			case 20:	/* -(A7) */
				TimingCycles += 6;
				break;

			case 5:		/* x(An) */
			case 9:		/* x(PC) */
				TimingCycles += 8;
				break;

			case 7:		/* x.w */
				TimingCycles += 8;
				break;

			case 6:		/* x(An,xr.s) */
			case 10:	/* x(PC,xr.s) */
				TimingCycles += 10;
				break;

			case 8:		/* x.l */
				TimingCycles += 12;
				break;
		}

		/* long w/r adds 4 cycles */

		if ((mode > 1) && (Size == 'L'))
			TimingCycles += 4;
	}

	AccessType = NORMAL;

	switch (mode) {

		case 2:
			if (Add) {
				if (Decal) fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[Rreg], regnameslong[Rreg]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[MEMBASE]);
			}
			fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[Rreg]);
			break;

		case 3:
			if (Add) {
				if (Decal) fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[Rreg], regnameslong[Rreg]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[MEMBASE]);
			}
			fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[Rreg]);
			IncrementEFFADDR(Size, Rreg);
			break;

		case 4:
			if (Add) {
				if (Decal) fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[Rreg], regnameslong[Rreg]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[MEMBASE]);
			}
			fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[Rreg]);
			DecrementEFFADDR(Size, Rreg);
			fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[Rreg]);
			break;

		case 5:
			assert((Rreg != PC) && (Rreg != TMPREG0));
			Memory_Fetch('W', TMPREG0, TRUE);
			if (Add) {
				if (Decal) fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[Rreg], regnameslong[Rreg]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[MEMBASE]);
			}
			fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[Rreg]);
			fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[EFFADDR], regnameslong[EFFADDR], regnameslong[TMPREG0]);
			break;

		case 6:

			/* Get Address register Value */

			if (Add) {
				if (Decal) fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[Rreg], regnameslong[Rreg]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[MEMBASE]);
			}
			fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[Rreg]);

			/* Add Extension Details */

			ExtensionDecode();
			break;

		case 7:

			/* Get Word */

			Memory_Fetch('W', EFFADDR, TRUE);
			fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);
			break;

		case 8:

			/* Get Long */

			Memory_Fetch('L', EFFADDR, FALSE);
			fprintf(fp, "\t\t addiu %s,%s,4\n", regnameslong[PC], regnameslong[PC]);
			break;

		case 9:

			AccessType = PCREL;

			Memory_Fetch('W', TMPREG0, TRUE);
			fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T9], regnameslong[PC], regnameslong[BASEPC]);
			fprintf(fp, "\t\t addu  %s,%s,%s       \t # Add Offset to PC\n", regnameslong[EFFADDR], regnameslong[T9], regnameslong[TMPREG0]);
			fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);
			break;

		case 10:

			AccessType = PCREL;

			/* Get PC */

			fprintf(fp, "\t\t subu  %s,%s,%s       \t # Get PC\n", regnameslong[EFFADDR], regnameslong[PC], regnameslong[BASEPC]);

			/* Add Extension Details */

			ExtensionDecode();

			break;

		case 19:

			/* (A7)+ */

			fprintf(fp, "\t\t lw    %s,%s    \t # Get A7\n", regnameslong[EFFADDR], REG_A7);
			fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[T9], regnameslong[EFFADDR]);
			fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[T9], REG_A7);
			break;

		case 20:

			/* -(A7) */

			fprintf(fp, "\t\t lw    %s,%s    \t # Get A7\n", regnameslong[EFFADDR], REG_A7);
			fprintf(fp, "\t\t addiu %s,%s,-2\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
			fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[EFFADDR], REG_A7);
			break;

	}
}

/* Read from Effective Address
 *
 * mode   = Effective Address from Instruction
 * Size   = Byte,Word or Long
 * Rreg   = Register with Register Number in
 * Flag   = Registers to preserve (EDX is handled by SaveEDX)
 * Extend = Force sign extension
 *
 * Return
 * Dreg = Register to return result in (V0 is usually most efficient)
 * (modes 5 to 10) EFFADDR = Address of data read (masked with FFFFFF)
 */

void EffectiveAddressRead(int mode, char Size, int Rreg, int Dreg, const char *flags, int Extend, int Decal, int Add)
{
	char* Opcode = "";
	char Flags[33];

	AccessType = NORMAL;

	strcpy(Flags, flags);

	switch (Size) {
		case 66:
			if (mode == 1)
				Opcode = "DUFF CODE (BAD READ)!!!";
			else if (Extend)
				Opcode = "lb ";
			else
				Opcode = "lbu";
			break;

		case 87:
			if (Extend || (mode == 1))
				Opcode = "lh ";
			else
				Opcode = "lhu";
			break;

		case 76:
			Opcode = "lw ";
			break;
	}

	switch (mode & 15) {

		case 0:

			/* Read 32 bits - No prefix */

			if (Add) {
				if (Decal) fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[Rreg], regnameslong[Rreg]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[MEMBASE]);
			}
			fprintf(fp, "\t\t %s   %s,0x00(%s)\n", Opcode, regnameslong[Dreg], regnameslong[Rreg]);
			break;

		case 1:

			/* Read 32 bits - No prefix */

			if (Add) {
				if (Decal) fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[Rreg], regnameslong[Rreg]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[MEMBASE]);
			}
			fprintf(fp, "\t\t %s   %s,0x20(%s)\n", Opcode, regnameslong[Dreg], regnameslong[Rreg]);
			break;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			EffectiveAddressCalculate(mode, Size, Rreg, Decal, Add);

			Memory_Read(Size, EFFADDR, Flags, LocalStack);

			if (Extend && (Size == 'B' || Size == 'W')) {
				fprintf(fp, "\t\t se%c   %s,%s\n", Size == 'W' ? 'h' : 'b', regnameslong[Dreg], regnameslong[V0]);
			} else if (Dreg != V0) {
				fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[Dreg], regnameslong[V0]);
			}
			break;

		case 11:

			/* Immediate - for SR or CCR see ReadCCR() */

			if (Size == 'L') {
				Memory_Fetch('L', Dreg, FALSE);
				fprintf(fp, "\t\t addiu %s,%s,4\n", regnameslong[PC], regnameslong[PC]);
			} else {
				Memory_Fetch(Size, Dreg, Extend);
				fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);
			}
			break;
	}
}

/*
 * EA	= Effective Address from Instruction
 * Size = Byte,Word or Long
 * Rreg = Register with Register Number in
 *
 * Writes from V0
 */

void EffectiveAddressWrite(int mode, char Size, int Rreg, int CalcAddress, const char *flags, int Decal, int Add)
{
	char* Opcode = "";
	char Flags[33];

	strcpy(Flags, flags);

	switch (Size) {
		case 66:
			Opcode = "sb";
			break;

		case 87:
			Opcode = "sh";
			break;

		case 76:
			Opcode = "sw";
			break;
	}

	switch (mode & 15) {

		case 0:
			if (Add) {
				if (Decal) fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[Rreg], regnameslong[Rreg]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[MEMBASE]);
			}
			fprintf(fp, "\t\t %s    %s,0x00(%s)\n", Opcode, regnameslong[V0], regnameslong[Rreg]);
			break;

		case 1:
			if (Size == 'B') {
				/* Not Allowed */

				fprintf(fp, "DUFF CODE (BAD WRITE)!\n");
			} else {
				if (Size == 'W') {
					fprintf(fp, "\t\t seh   %s,%s\n", regnameslong[V0], regnameslong[V0]);
				}

				if (Add) {
					if (Decal) fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[Rreg], regnameslong[Rreg]);
					fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[Rreg], regnameslong[Rreg], regnameslong[MEMBASE]);
				}
				fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[V0], regnameslong[Rreg]);
			}
			break;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			if (CalcAddress) EffectiveAddressCalculate(mode, Size, Rreg, Decal, Add);
			Memory_Write(Size, EFFADDR, V0, Flags, LocalStack);
			break;

		case 11:

			if (Size == 'L') fprintf(fp, "DUFF CODE (BAD WRITE)!\n");

			/* SR, CCR - Chain to correct routine */

			WriteCCR(Size, V0);
	}
}

/* Condition Decode Routines */

/*
 * mode = condition to check for
 *
 * Returns LABEL that is jumped to if condition is Condition
 *
 */

char* ConditionDecode(int mode, int Condition, int NOP)
{
	char *Label = GenerateLabel(0, 1);

	Label[0] = '1';
	Label[1] = 0;

	switch (mode) {

		case 0:	/* A - Always */
			if (Condition) {
				fprintf(fp, "\t\t bgez  $0,%sf\n", Label);
				if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			}
			break;

		case 1:	/* F - Never */
			if (!Condition) {
				fprintf(fp, "\t\t bgez  $0,%sf\n", Label);
				if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			}
			break;

		case 2:	/* Hi */
			fprintf(fp, "\t\t nor   %s,%s,%s\n", regnameslong[T9], regnameslong[FLAG_C], regnameslong[FLAG_Z]);
			fprintf(fp, "\t\t andi  %s,%s,1\n", regnameslong[T9], regnameslong[T9]);
			if (Condition) {
				fprintf(fp, "\t\t bne   %s,$0,%sf\n", regnameslong[T9], Label);
			} else {
				fprintf(fp, "\t\t beq   %s,$0,%sf\n", regnameslong[T9], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 3:	/* Ls */
			fprintf(fp, "\t\t or   %s,%s,%s\n", regnameslong[T9], regnameslong[FLAG_C], regnameslong[FLAG_Z]);
			if (Condition) {
				fprintf(fp, "\t\t bne   %s,$0,%sf\n", regnameslong[T9], Label);
			} else {
				fprintf(fp, "\t\t beq   %s,$0,%sf\n", regnameslong[T9], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 4:	/* CC */
			if (Condition) {
				fprintf(fp, "\t\t beq   %s,$0,%sf        \t # check carry\n", regnameslong[FLAG_C], Label);
			} else {
				fprintf(fp, "\t\t bne   %s,$0,%sf        \t # check carry\n", regnameslong[FLAG_C], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 5:	/* CS */
			if (Condition) {
				fprintf(fp, "\t\t bne   %s,$0,%sf        \t # check carry\n", regnameslong[FLAG_C], Label);
			} else {
				fprintf(fp, "\t\t beq   %s,$0,%sf        \t # check carry\n", regnameslong[FLAG_C], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 6:	/* NE */
			if (Condition) {
				fprintf(fp, "\t\t beq   %s,$0,%sf        \t # Check zero\n", regnameslong[FLAG_Z], Label);
			} else {
				fprintf(fp, "\t\t bne   %s,$0,%sf        \t # Check zero\n", regnameslong[FLAG_Z], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 7:	/* EQ */
			if (Condition) {
				fprintf(fp, "\t\t bne   %s,$0,%sf        \t # Check zero\n", regnameslong[FLAG_Z], Label);
			} else {
				fprintf(fp, "\t\t beq   %s,$0,%sf        \t # Check zero\n", regnameslong[FLAG_Z], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 8:	/* VC */
			if (Condition) {
				fprintf(fp, "\t\t beq   %s,$0,%sf        \t # Check Overflow\n", regnameslong[FLAG_V], Label);
			} else {
				fprintf(fp, "\t\t bne   %s,$0,%sf        \t # Check Overflow\n", regnameslong[FLAG_V], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 9:	/* VS */
			if (Condition) {
				fprintf(fp, "\t\t bne   %s,$0,%sf        \t # Check Overflow\n", regnameslong[FLAG_V], Label);
			} else {
				fprintf(fp, "\t\t beq   %s,$0,%sf        \t # Check Overflow\n", regnameslong[FLAG_V], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 10:	/* PL */
			if (Condition) {
				fprintf(fp, "\t\t beq   %s,$0,%sf        \t # Check Sign\n", regnameslong[FLAG_N], Label);
			} else {
				fprintf(fp, "\t\t bne   %s,$0,%sf        \t # Check Sign\n", regnameslong[FLAG_N], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 11:	/* MI */
			if (Condition) {
				fprintf(fp, "\t\t bne   %s,$0,%sf        \t # Check Sign\n", regnameslong[FLAG_N], Label);
			} else {
				fprintf(fp, "\t\t beq   %s,$0,%sf        \t # Check Sign\n", regnameslong[FLAG_N], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 12:	/* GE */
			if (Condition) {
				fprintf(fp, "\t\t beq   %s,%s,%sf\n", regnameslong[FLAG_N], regnameslong[FLAG_V], Label);
			} else {
				fprintf(fp, "\t\t bne   %s,%s,%sf\n", regnameslong[FLAG_N], regnameslong[FLAG_V], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 13:	/* LT */
			if (Condition) {
				fprintf(fp, "\t\t bne   %s,%s,%sf\n", regnameslong[FLAG_N], regnameslong[FLAG_V], Label);
			} else {
				fprintf(fp, "\t\t beq   %s,%s,%sf\n", regnameslong[FLAG_N], regnameslong[FLAG_V], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 14:	/* GT */
			fprintf(fp, "\t\t xor    %s,%s,%s\n", regnameslong[T9], regnameslong[FLAG_N], regnameslong[FLAG_V]);
			fprintf(fp, "\t\t or     %s,%s,%s\n", regnameslong[T9], regnameslong[T9], regnameslong[FLAG_Z]);
			if (Condition) {
				fprintf(fp, "\t\t beq   %s,$0,%sf\n", regnameslong[T9], Label);
			} else {
				fprintf(fp, "\t\t bne   %s,$0,%sf\n", regnameslong[T9], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;

		case 15:	/* LE */
			fprintf(fp, "\t\t xor    %s,%s,%s\n", regnameslong[T9], regnameslong[FLAG_N], regnameslong[FLAG_V]);
			fprintf(fp, "\t\t or     %s,%s,%s\n", regnameslong[T9], regnameslong[T9], regnameslong[FLAG_Z]);
			if (Condition) {
				fprintf(fp, "\t\t bne   %s,$0,%sf\n", regnameslong[T9], Label);
			} else {
				fprintf(fp, "\t\t beq   %s,$0,%sf\n", regnameslong[T9], Label);
			}
			if (NOP) fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			break;
	}

	return Label;
}

/*
 * mode = condition to check for
 *
 * result in V0
 */

void ConditionCheck(int mode, int SetWhat)
{
	switch (mode) {

		case 0:	/* A - Always */
			fprintf(fp, "\t\t addiu %s,$0,-1\n", regnameslong[SetWhat]);
			break;

		case 1:	/* F - Never */
			ClearRegister(SetWhat);
			break;

		case 2:	/* Hi */
			fprintf(fp, "\t\t nor   %s,%s,%s\n", regnameslong[SetWhat], regnameslong[FLAG_C], regnameslong[FLAG_Z]);
			fprintf(fp, "\t\t andi  %s,%s,1\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 3:	/* Ls */
			fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[SetWhat], regnameslong[FLAG_C], regnameslong[FLAG_Z]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 4:	/* CC */
			fprintf(fp, "\t\t xori  %s,%s,1         \t # Check Carry\n", regnameslong[SetWhat], regnameslong[FLAG_C]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 5:	/* CS */
			fprintf(fp, "\t\t or    %s,$0,%s        \t # Check Carry\n", regnameslong[SetWhat], regnameslong[FLAG_C]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 6:	/* NE */
			fprintf(fp, "\t\t xori  %s,%s,1         \t # Check Zero\n", regnameslong[SetWhat], regnameslong[FLAG_Z]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 7:	/* EQ */
			fprintf(fp, "\t\t or    %s,$0,%s        \t # Check Zero\n", regnameslong[SetWhat], regnameslong[FLAG_Z]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 8:	/* VC */
			fprintf(fp, "\t\t xori  %s,%s,1         \t # Check Overflow\n", regnameslong[SetWhat], regnameslong[FLAG_V]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 9:	/* VS */
			fprintf(fp, "\t\t or    %s,$0,%s        \t # Check Overflow\n", regnameslong[SetWhat], regnameslong[FLAG_V]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 10:	/* PL */
			fprintf(fp, "\t\t xori  %s,%s,1         \t # Check Sign\n", regnameslong[SetWhat], regnameslong[FLAG_N]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 11:	/* MI */
			fprintf(fp, "\t\t or    %s,$0,%s        \t # Check Sign\n", regnameslong[SetWhat], regnameslong[FLAG_N]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 12:	/* GE */
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[SetWhat], regnameslong[FLAG_N], regnameslong[FLAG_V]);
			fprintf(fp, "\t\t addiu %s,%s,-1\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 13:	/* LT */
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[SetWhat], regnameslong[FLAG_N], regnameslong[FLAG_V]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 14:	/* GT */
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[SetWhat], regnameslong[FLAG_N], regnameslong[FLAG_V]);
			fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[SetWhat], regnameslong[SetWhat], regnameslong[FLAG_Z]);
			fprintf(fp, "\t\t addiu %s,%s,-1\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;

		case 15:	/* LE */
			fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[SetWhat], regnameslong[FLAG_N], regnameslong[FLAG_V]);
			fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[SetWhat], regnameslong[SetWhat], regnameslong[FLAG_Z]);
			fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[SetWhat], regnameslong[SetWhat]);
			break;
	}
}

/**********************************************************************/
/* Instructions - Each routine generates a range of instruction codes */
/**********************************************************************/

/*
 * Immediate Commands
 *
 * ORI	00xx
 * ANDI	02xx
 * SUBI	04xx
 * ADDI	06xx
 * EORI	0axx
 * CMPI	0cxx
 *
 */

void dump_imm(int type, int leng, int mode, int sreg)
{
	int Opcode, BaseCode, OpType;
	char Size = ' ';
	char * OpcodeName[16] = {"or  ", "and ", "subu", "addu", 0, "xor ", "subu", 0} ;
	int allow[] = {1,0,1,1, 1,1,1,1, 1,0,0,0, 0,0,0,0, 0,0,0,1, 1} ;
	char flags[] = "---------------------SSS----G-F-";

	Opcode = (type << 9) | ( leng << 6 ) | ( mode << 3 ) | sreg;

	BaseCode = Opcode & 0xfff8;

	if (mode == 7) BaseCode |= sreg ;

	if ((leng == 0) && (sreg == 7) && (mode > 2) && (mode < 5)) {
		BaseCode |= sreg ;
	}

	if (type != 4) {	/* Not Valid (for this routine) */
		int Dest = EAtoAMN(Opcode, FALSE);
		int SetX;

		/* ADDI & SUBI also set X flag */

		SetX = ((type == 2) || (type == 3));

		switch (type) {
			case 0:
			case 1:
			case 5:
				OpType = FLAGS_BITOP;
				break;
			case 2:
				OpType = FLAGS_SUB;
				break;
			case 3:
				OpType = FLAGS_ADD;
				break;
			case 6:
				OpType = FLAGS_CMP;
				break;
		}

		switch (leng) {
			case 0:
				Size = 'B';
				break;
			case 1:
				Size = 'W';
				break;
			case 2:
				Size = 'L';
				break;
		}

		if (allow[Dest]) {
			if (OpcodeArray[BaseCode] == -2) {
				Align();
				fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

				if (!SetX) flags[FLAG_X] = 'S';

				/* Save Previous PC if Memory Access */

				if ((Dest >= 2) && (Dest <= 10))
					SavePreviousPC();

				fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

				if (mode < 2) {
					if (Size != 'L')
						TimingCycles += 8;
					else {
						TimingCycles += 14;
						if ((type != 1) && (type!=6))
							TimingCycles += 2;
					}
				} else {
					if (type != 6) {
						if (Size != 'L')
							TimingCycles += 12;
						else
							TimingCycles += 20;
					} else {
						if (Size != 'L')
							TimingCycles += 8;
						else
							TimingCycles += 12;
					}
				}

				if (Dest < 7) {
					fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
					if (Dest < 2) flags[OPCODE] = 'T';
				}

				/* Immediate Mode Data */
				EffectiveAddressRead(11, Size, T0, T0, flags, TRUE, FALSE, FALSE);

				/* Source Data */
				flags[T0] = 'T';
				if (Dest > 1 && type != 6) flags[EFFADDR] = 'T';
				EffectiveAddressRead(Dest, Size, OPCODE, SetX ? T1 : V0, flags, TRUE, TRUE, TRUE);

				/* The actual work */
				fprintf(fp, "\t\t %s  %s,%s,%s\n", OpcodeName[type], regnameslong[type == 6 ? T1 : V0], regnameslong[SetX ? T1 : V0], regnameslong[T0]);

				SetFlags(Size, OpType, type == 6 ? T1 : V0, SetX ? T1 : V0, T0);

				if (type != 6) { /* CMP no update */
					flags[OPCODE] = '-';
					flags[T0] = '-';
					flags[EFFADDR] = '-';
					flags[FLAG_C] = 'S';
					flags[FLAG_V] = 'S';
					flags[FLAG_Z] = 'S';
					flags[FLAG_N] = 'S';
					flags[FLAG_X] = 'S';
					EffectiveAddressWrite(Dest, Size, OPCODE, FALSE, flags, FALSE, FALSE);
				}

				Completed();
			}
		} else {
			/* Logicals are allowed to alter SR/CCR */

			if ((!SetX) && (Dest == 11) && (Size != 'L') && (type != 6)) {
				Align();

				fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
				if (Size != 'W') {
					// Will be added in the delay slot for 16bit access
					fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);
				}
				TimingCycles += 20 ;

				if (Size == 'W') {
					/* If SR then must be in Supervisor Mode */

					fprintf(fp, "\t\t lbu   %s,%s\n", regnameslong[T0], REG_SRH);
					fprintf(fp, "\t\t andi  %s,%s,0x20       \t # Supervisor Mode ?\n", regnameslong[T0], regnameslong[T0]);
					fprintf(fp, "\t\t bne   %s,$0,9f\n", regnameslong[T0]);
					fprintf(fp, "\t\t addiu %s,%s,2        \t # Delay slot\n\n", regnameslong[PC], regnameslong[PC]);

					/* User Mode - Exception */

					Exception(8, NULL);

					fprintf(fp, "\t9:\n");
				}

				/* Immediate Mode Data */
				EffectiveAddressRead(11, Size, T0, T0, flags, FALSE, FALSE, FALSE);

				ReadCCR(Size, V0);

				fprintf(fp, "\t\t %s  %s,%s,%s\n", OpcodeName[type], regnameslong[V0], regnameslong[V0], regnameslong[T0]);

				WriteCCR(Size, V0);

				Completed();
			} else {

				/* Illegal Opcode */

				OpcodeArray[BaseCode] = -1;
				BaseCode = -1;
			}
		}
	} else {
		BaseCode = -2;
	}

	OpcodeArray[Opcode] = BaseCode;
}

void immediate(void)
{
	int type, size, mode, sreg ;

	for (type = 0 ; type < 0x7; type++)
		for (size = 0 ; size < 3 ; size++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
					dump_imm( type, size, mode, sreg ) ;
}


/*
 * Bitwise Codes
 *
 */

void dump_bit_dynamic( int sreg, int type, int mode, int dreg )
{
	int  Opcode, BaseCode ;
	char Size ;
	char allow[] = "0-2345678-------" ;
	char flags[] = "----------------SSSSSSSS----G-F-";
	int Dest ;

	/* BTST allows x(PC) and x(PC,xr.s) - others do not */

	if (type == 0)
	{
		allow[9]  = '9';
		allow[10] = 'a';
		allow[11] = 'b'; // dave fix to nhl
	}

	Opcode = 0x0100 | (sreg << 9) | (type << 6) | (mode << 3) | dreg ;

	BaseCode = Opcode & 0x01f8 ;
	if (mode == 7) BaseCode |= dreg ;


	// A7+, A7-

	if ((mode > 2) && (mode < 5)) {
		if (dreg == 7) BaseCode |= dreg;
	}

	Dest = EAtoAMN(Opcode, FALSE);

	if (allow[Dest & 0xf] != '-') {
		if (mode == 0) {/* long*/
			Size = 'L' ;
		} else {
			Size = 'B' ;
		}

		if (OpcodeArray[BaseCode] == -2) {
			Align();
			fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

			/* Save Previous PC if Memory Access */

			if ((Dest >= 2) && (Dest <=10))
				SavePreviousPC();

			fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

			if (mode < 2) {
				switch (type) {
					case 0:
						TimingCycles += 6 ;
						break;
					case 1:
					case 3:
						TimingCycles += 8 ;
						break;
					case 2:
						TimingCycles += 10;
						break;
				}
			} else {
				if (type == 0)
					TimingCycles += 4;
				else
					TimingCycles += 8;
			}

			/* Only need this sorted out if a register is involved */

			if (Dest < 7) {
				fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
			}

			/* Get bit number and create mask in T1 */

			fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[OPCODE], regnameslong[OPCODE]);
			fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);
			fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[FLAG_Z], regnameslong[OPCODE]);

		 	fprintf(fp,"\t\t ori   %s,$0,1\n", regnameslong[T2]);

			if (Size != 'L')
				fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z]);
			//else
			//	fprintf(fp, "\t\t andi  %s,%s,0x1F\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z]);

			fprintf(fp,"\t\t sllv  %s,%s,%s\n", regnameslong[T1], regnameslong[T2], regnameslong[FLAG_Z]);

			if (mode != 0) {
				flags[T1] = 'T';
				flags[EFFADDR] = 'T';
				EffectiveAddressRead(Dest, Size, T0, V0, flags, FALSE, TRUE, TRUE);
			} else {
				fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[T0], regnameslong[T0]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T0], regnameslong[T0], regnameslong[MEMBASE]);
				fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[T0]);
			}


			/* All commands copy existing bit to Zero Flag */

			fprintf(fp,"\t\t and   %s,%s,%s\n", regnameslong[T2], regnameslong[T1], regnameslong[V0]);
			fprintf(fp,"\t\t srlv  %s,%s,%s\n", regnameslong[FLAG_Z], regnameslong[T2], regnameslong[FLAG_Z]);
			fprintf(fp,"\t\t xori  %s,%s,1        \t # Set Zero Flag\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z]);

			/* Some then modify the data */

			switch (type) {
				case 0: /* btst*/
					break;

				case 1: /* bchg*/
					fprintf(fp,"\t\t xor   %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T1]);
					break;

				case 2: /* bclr*/
					fprintf(fp,"\t\t nor   %s,$0,%s\n", regnameslong[T1], regnameslong[T1]);
					fprintf(fp,"\t\t and   %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T1]);
					break;

				case 3: /* bset*/
					fprintf(fp,"\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T1]);
					break;
			}

			if (type != 0) {
				if (mode != 0) {
					flags[T0] = '-';
					flags[T1] = '-';
					flags[EFFADDR] = '-';
					EffectiveAddressWrite(Dest, Size, T0, FALSE, flags, FALSE, FALSE);
				} else {
					fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[T0]);
				}
			}

			Completed();
		}

		OpcodeArray[Opcode] = BaseCode ;
	}
}

void bitdynamic(void) /* dynamic non-immediate bit operations*/
{
	int type, sreg, mode, dreg ;

	for (sreg = 0 ; sreg < 8 ; sreg++)
		for (type = 0 ; type < 4 ; type++)
			for (mode = 0 ; mode < 8 ;mode++)
				for (dreg = 0 ; dreg < 8 ;dreg++)
					dump_bit_dynamic( sreg, type, mode, dreg ) ;
}

void dump_bit_static(int type, int mode, int dreg)
{
	int  Opcode, BaseCode;
	char Size;
	char allow[] = "0-2345678-------";
	char flags[] = "----------------SSSSSSSS----G-F-";
	int Dest;

	/* BTST allows x(PC) and x(PC,xr.s) - others do not */

	if (type == 0) {
		allow[9] = '9';
		allow[10] = 'a';
	}

	Opcode = 0x0800 | (type << 6) | (mode << 3) | dreg ;
	BaseCode = Opcode & 0x08f8 ;
	if (mode == 7) BaseCode |= dreg ;

	// A7+, A7-

	if ((mode > 2) && (mode < 5)) {
		if (dreg == 7) BaseCode |= dreg;
	}

	Dest = EAtoAMN(Opcode, FALSE);

	if (allow[Dest & 0xf] != '-') {
		if (mode == 0) { /* long*/
			/* Modify register memory directly */
			Size = 'L' ;
		} else {
			Size = 'B' ;
		}

		if (OpcodeArray[BaseCode] == -2) {
			Align();
			fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

			/* Save Previous PC if Memory Access */

			if ((Dest >= 2) && (Dest <= 10))
				SavePreviousPC();

			fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

			if (mode < 2) {
				switch (type) {
					case 0:
						TimingCycles += 10;
						break ;
					case 1:
					case 3:
						TimingCycles += 12;
						break ;
					case 2:
						TimingCycles += 14;
						break ;
				}
			} else {
				if (type != 0)
					TimingCycles += 12;
				else
					TimingCycles += 8;
			}

			/* Only need this sorted out if a register is involved */

			if (Dest < 7) {
				fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
			}

			/* Get bit number and create mask in T1 */

			Memory_Fetch('W', FLAG_Z, FALSE);
			fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);

			if (Size != 'L')
				fprintf(fp, "\t\t and   %s,%s,0x07\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z]);
			//else
			//	fprintf(fp, "\t\t and   %s,%s,0x1F\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z]);

		 	fprintf(fp,"\t\t ori   %s,$0,1\n", regnameslong[T2]);

			fprintf(fp,"\t\t sllv  %s,%s,%s\n", regnameslong[T1], regnameslong[T2], regnameslong[FLAG_Z]);

			if (mode != 0) {
				flags[T1] = 'T';
				flags[EFFADDR] = 'T';
				EffectiveAddressRead(Dest, Size, T0, V0, flags, FALSE, TRUE, TRUE);
			} else {
				fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[T0], regnameslong[T0]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T0], regnameslong[T0], regnameslong[MEMBASE]);
				fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[T0]);
			}

			/* All commands copy existing bit to Zero Flag */

			fprintf(fp,"\t\t and   %s,%s,%s\n", regnameslong[T2], regnameslong[T1], regnameslong[V0]);
			fprintf(fp,"\t\t srlv  %s,%s,%s\n", regnameslong[FLAG_Z], regnameslong[T2], regnameslong[FLAG_Z]);
			fprintf(fp,"\t\t xori  %s,%s,1        \t # Set Zero Flag\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z]);

			/* Some then modify the data */

			switch (type) {
				case 0: /* btst*/
					break;

				case 1: /* bchg*/
					fprintf(fp,"\t\t xor   %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T1]);
					break;

				case 2: /* bclr*/
					fprintf(fp,"\t\t nor   %s,$0,%s\n", regnameslong[T1], regnameslong[T1]);
					fprintf(fp,"\t\t and   %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T1]);
					break;

				case 3: /* bset*/
					fprintf(fp,"\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T1]);
					break;
			}

			if (type != 0) {
				if (mode != 0) {
					flags[T0] = '-';
					flags[T1] = '-';
					flags[EFFADDR] = '-';
					EffectiveAddressWrite(Dest, Size, T0, FALSE, flags, FALSE, FALSE);
				} else {
					fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[T0]);
				}
			}

			Completed();
		}

		OpcodeArray[Opcode] = BaseCode ;
	}
}


void bitstatic(void) /* static non-immediate bit operations*/
{
	int type, mode, dreg ;

	for (type = 0 ; type < 4 ; type++)
		for (mode = 0 ; mode < 8 ;mode++)
			for (dreg = 0 ; dreg < 8 ;dreg++)
				dump_bit_static( type, mode, dreg ) ;
}

/*
 * Move Peripheral
 *
 */

void movep(void)
{
	int sreg, dir, leng, dreg;
	int	Opcode, BaseCode;
	char flags[] = "---------------------SSS----G-F-";

	for (sreg = 0 ; sreg < 8 ; sreg++) {
		for (dir = 0 ; dir < 2 ; dir++) {
			for (leng = 0 ; leng < 2 ; leng++) {
				for (dreg = 0 ; dreg < 8 ; dreg++) {
					Opcode = 0x0108 | (sreg << 9) | (dir << 7) | (leng << 6) | dreg;
					BaseCode = Opcode & 0x01c8;
					if (OpcodeArray[BaseCode] == -2) {
						Align();
						fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
						SavePreviousPC();
						fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						if (leng == 0) /* word */
							TimingCycles += 16 ;
						else
							TimingCycles += 24 ;

						/* Save Flags Registers (so we only do it once) */

						if (SavedRegs[FLAG_C] == '-') {
							LocalStack -= 4;
							fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_C], LocalStack, regnameslong[SP]);
						}
						if (SavedRegs[FLAG_V] == '-') {
							LocalStack -= 4;
							fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_V], LocalStack, regnameslong[SP]);
						}
						if (SavedRegs[FLAG_Z] == '-') {
							LocalStack -= 4;
							fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_Z], LocalStack, regnameslong[SP]);
						}
						if (SavedRegs[FLAG_N] == '-') {
							LocalStack -= 4;
							fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_N], LocalStack, regnameslong[SP]);
						}
						if (SavedRegs[FLAG_X] == '-') {
							LocalStack -= 4;
							fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_X], LocalStack, regnameslong[SP]);
						}
						assert(LocalStack >= MINSTACK_POS);

						fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);

						/* Get Address to Read/Write in EFFADDR */

						EffectiveAddressCalculate(5, 'L', T0, TRUE, TRUE);

						fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);

						if (dir == 0) { /* from memory to register*/
							flags[OPCODE] = 'T';
							flags[EFFADDR] = 'T';
							Memory_Read('B', EFFADDR, flags, LocalStack);
							fprintf(fp, "\t\t sll   %s,%s,8\n", regnameslong[T0], regnameslong[V0]);
							fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
							flags[T0] = 'T';
							if (leng == 0) flags[EFFADDR] = '-'; /* word d(Ax) into Dx.W*/
							Memory_Read('B', EFFADDR, flags, LocalStack);
							fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[T0], regnameslong[T0], regnameslong[V0]);

							if (leng == 0) { /* word d(Ax) into Dx.W*/
								fprintf(fp, "\t\t sh    %s,0x00(%s)\n", regnameslong[T0], regnameslong[OPCODE]);
							} else { /* long d(Ax) into Dx.L*/
								fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
								fprintf(fp, "\t\t sll   %s,%s,16\n", regnameslong[T0], regnameslong[V0]);
								Memory_Read('B', EFFADDR, flags, LocalStack);
								fprintf(fp, "\t\t sll   %s,%s,8\n", regnameslong[T1], regnameslong[V0]);
								fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
								flags[EFFADDR] = '-';
								Memory_Read('B', EFFADDR, flags, LocalStack);
								flags[OPCODE] = '-';
								fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[T0], regnameslong[T0], regnameslong[T1]);
								fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[T0], regnameslong[T0], regnameslong[V0]);
								fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[T0], regnameslong[OPCODE]);
							}
						} else { /* Register to Memory*/
							fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[OPCODE], regnameslong[OPCODE]);

							/* Move bytes into Line */

							if (leng == 1)
								fprintf(fp,"\t\t sll   %s,%s,24\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[OPCODE]);
							else
								fprintf(fp,"\t\t sll   %s,%s,8\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[OPCODE]);

							flags[OPCODE] = 'T';
							flags[EFFADDR] = 'T';
							Memory_Write('B', EFFADDR, FASTCALL_SECOND_REG, flags, LocalStack);
							fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
							if (leng == 1) /* long*/
								fprintf(fp, "\t\t sll   %s,%s,16\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[OPCODE]);
							else
								fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[OPCODE]);

							if (leng == 1) { /* long*/
								Memory_Write('B', EFFADDR, FASTCALL_SECOND_REG, flags, LocalStack);
								fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
								fprintf(fp, "\t\t sll   %s,%s,8\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[OPCODE]);
								Memory_Write('B', EFFADDR, FASTCALL_SECOND_REG, flags, LocalStack);
								fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
								fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[OPCODE]);
							}
							flags[OPCODE] = '-';
							flags[EFFADDR] = '-';
							Memory_Write('B', EFFADDR, FASTCALL_SECOND_REG, flags, LocalStack);
						}

						if (SavedRegs[FLAG_X] == '-') {
							fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_X], LocalStack, regnameslong[SP]);
							LocalStack += 4;
						}
						if (SavedRegs[FLAG_N] == '-') {
							fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_N], LocalStack, regnameslong[SP]);
							LocalStack += 4;
						}
						if (SavedRegs[FLAG_Z] == '-') {
							fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_Z], LocalStack, regnameslong[SP]);
							LocalStack += 4;
						}
						if (SavedRegs[FLAG_V] == '-') {
							fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_V], LocalStack, regnameslong[SP]);
							LocalStack += 4;
						}
						if (SavedRegs[FLAG_C] == '-') {
							fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_C], LocalStack, regnameslong[SP]);
							LocalStack += 4;
						}

						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
		}
	}
}

void movecodes(int allowfrom[], int allowto[], int Start, char Size) /* MJC */
{
	int Opcode;
	int Src, Dest;
	int BaseCode;
	char flags[] = "--------------------SSSS----G-F-";

	for (Opcode = Start; Opcode < Start + 0x1000; Opcode++) {
		/* Mask our Registers */

		BaseCode = Opcode & (Start + 0x1f8);

		/* Unless Mode = 7 */

		if ((BaseCode & 0x38)  == 0x38)  BaseCode |= (Opcode & 7);
		if ((BaseCode & 0x1c0) == 0x1c0) BaseCode |= (Opcode & 0xE00);

		/* If mode = 3 or 4 and Size = byte and register = A7 */
		/* then make it a separate code							  */

		if (Size == 'B') {
			if (((Opcode & 0x3F) == 0x1F) || ((Opcode & 0x3F) == 0x27)) {
				BaseCode |= 0x07;
			}

			if (((Opcode & 0xFC0) == 0xEC0) || ((Opcode & 0xFC0) == 0xF00)) {
				BaseCode |= 0x0E00;
			}
		}

		/* If Source = Data or Address register - combine into same routine */

		if (((Opcode & 0x38) == 0x08) && (allowfrom[1])) {
			BaseCode &= 0xfff7;
		}

		if (OpcodeArray[BaseCode] == -2) {
			Src  = EAtoAMN(Opcode, FALSE);
			Dest = EAtoAMN(Opcode >> 6, TRUE);

			if ((allowfrom[(Src & 15)]) && (allowto[(Dest & 15)])) {
				/* If we are not going to calculate the flags */
				/* we need to preserve the existing ones	*/

				if (Dest == 1) {
					flags[FLAG_C] = 'S';
					flags[FLAG_V] = 'S';
					flags[FLAG_Z] = 'S';
					flags[FLAG_N] = 'S';
				} else {
					flags[FLAG_C] = '-';
					flags[FLAG_V] = '-';
					flags[FLAG_Z] = '-';
					flags[FLAG_N] = '-';
				}

				Align();
				fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

				if ((Size == 'L') && ((Dest & 15) == 4))
					PreDecLongMove = 1;

				if (((Src >= 2) && (Src <= 10)) || ((Dest >= 2) && (Dest <= 10)))
					SavePreviousPC();

				fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);


				TimingCycles += 4 ;

				if (Dest < 7) flags[OPCODE] = 'T';
				if (Src < 7) {
					if ((Src == 0) && allowfrom[1])
						fprintf(fp, "\t\t and   %s,%s,0x0f\n", regnameslong[T0], regnameslong[OPCODE]);
					else
						fprintf(fp, "\t\t and   %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
				}
				EffectiveAddressRead(Src, Size, T0, V0, flags, FALSE, TRUE, TRUE);

				/* No flags if Destination Ax */

				if (Dest != 1) {
					SetFlags(Size, FLAGS_MOVE, V0, 0, 0);
				}

				if (Dest < 7) {
					flags[OPCODE] = '-';
					fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[OPCODE], regnameslong[OPCODE]);
					fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);
				}

				if (Dest != 1) {
					flags[FLAG_C] = 'S';
					flags[FLAG_V] = 'S';
					flags[FLAG_Z] = 'S';
					flags[FLAG_N] = 'S';
				}
				EffectiveAddressWrite(Dest, Size, OPCODE, TRUE, flags, FALSE, TRUE);
				flags[FLAG_C] = '-';
				flags[FLAG_V] = '-';
				flags[FLAG_Z] = '-';
				flags[FLAG_N] = '-';

				Completed();
			} else {
				BaseCode = -1; /* Invalid Code */
			}
		} else {
			BaseCode = OpcodeArray[BaseCode];
		}

		if (OpcodeArray[Opcode] < 0)
			OpcodeArray[Opcode] = BaseCode;
	}
}

void moveinstructions(void)
{
	int allowfrom[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0};
	int allowto[]	= {1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};

	/* For Byte */

	movecodes(allowfrom, allowto, 0x1000, 'B');

	/* For Word & Long */

	allowto[1] = 1;
	movecodes(allowfrom, allowto, 0x2000, 'L');
	movecodes(allowfrom, allowto, 0x3000, 'W');
}

/*
 *
 * Opcodes 5###
 *
 * ADDQ,SUBQ,Scc and DBcc
 *
 */

void opcode5(void)
{
	/* ADDQ,SUBQ,Scc and DBcc */

	int allowtoScc[]	= {1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
	int allowtoADDQ[]	= {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
	int Opcode, BaseCode;
	char Label[32];
	char Label2[32];
	char Size = ' ';
	char OpSize = ' ';
	char flags[] = "---------------------SSS----G-F-";

	for (Opcode = 0x5000; Opcode < 0x6000; Opcode++) {
		if ((Opcode & 0xc0) == 0xc0) {
			/* Scc or DBcc */

			BaseCode = Opcode & 0x5FF8;
			if ((BaseCode & 0x38) == 0x38) BaseCode |= (Opcode & 7);

			/* If mode = 3 or 4 and register = A7 */
			/* then make it a separate code		 */

			if (((Opcode & 0x3F) == 0x1F) || ((Opcode & 0x3F) == 0x27)) {
				BaseCode |= 0x07;
			}

			if (OpcodeArray[BaseCode] == -2) {
				OpcodeArray[BaseCode] = BaseCode;

				if ((BaseCode & 0x38) == 0x8) {
					/* DBcc */

					Align();
					fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

					TimingCycles += 10 ;


					strcpy(Label, GenerateLabel(BaseCode, 1));
					strcpy(Label2, ConditionDecode((Opcode >> 8) & 0x0F, TRUE, FALSE));

					/* False - Decrement Counter - Loop if not -1 */

					fprintf(fp, "\t\t andi  %s,%s,0x07%s\n", regnameslong[OPCODE], regnameslong[OPCODE], ((Opcode >> 8) & 0x0F) == 1 ? "" : "     \t # Delay slot");
					fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[OPCODE], regnameslong[OPCODE]);
					fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);
					fprintf(fp, "\t\t lhu   %s,0x00(%s)\n", regnameslong[T0], regnameslong[OPCODE]);
					fprintf(fp, "\t\t addiu %s,%s,-1\n", regnameslong[T1], regnameslong[T0]);
					fprintf(fp, "\t\t beq   %s,$0,9f         \t # Is it -1\n", regnameslong[T0]);
					fprintf(fp, "\t\t sh    %s,0x00(%s)     \t # Delay slot\n\n", regnameslong[T1], regnameslong[OPCODE]);

					fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);
					Memory_Fetch('W', V0, TRUE);
					fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[PC], regnameslong[PC], regnameslong[V0]);
					Completed();

					/* True - Exit Loop */
					fprintf(fp, "\t9:\n");

					fprintf(fp, "\t%s:\n", Label2);
					fprintf(fp, "\t\t addiu %s,%s,4\n\n", regnameslong[PC], regnameslong[PC]);
					TimingCycles += 2 ;

					Completed();
				} else {
					/* Scc */

					int  Dest = EAtoAMN(Opcode, FALSE);

					if (allowtoScc[(Dest & 15)]) {
						Align();
						fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

						if ((Dest >= 2) && (Dest <= 10))
							SavePreviousPC();

						fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						if (Dest > 1)
							TimingCycles += 8;
						else
							TimingCycles += 4;

						if (Dest < 7) {
							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						}

						if (Dest > 1) {
							EffectiveAddressCalculate(Dest, 'B', OPCODE, TRUE, TRUE);
						}

						ConditionCheck((Opcode >> 8) & 0x0F, V0);

						flags[FLAG_C] = 'S';
						flags[FLAG_V] = 'S';
						flags[FLAG_Z] = 'S';
						flags[FLAG_N] = 'S';
						flags[FLAG_X] = 'S';
						EffectiveAddressWrite(Dest, 'B', OPCODE, FALSE, flags, TRUE, TRUE);
						flags[FLAG_C] = '-';
						flags[FLAG_V] = '-';
						flags[FLAG_Z] = '-';
						flags[FLAG_N] = '-';
						flags[FLAG_X] = '-';

						/* take advantage of V0 being 0 for false, -1 for true */
						/* need to add 2 cycles if register and condition is true */

						if (Dest == 0) {
							fprintf(fp, "\t\t andi  %s,%s,0x02\n", regnameslong[V0], regnameslong[V0]);
							fprintf(fp, "\t\t addiu %s,%s,%d\n", regnameslong[ICNT], regnameslong[ICNT], -TimingCycles);
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[ICNT], regnameslong[ICNT], regnameslong[V0]);

							TimingCycles = -1;
						}
						Completed();
					} else {
						OpcodeArray[BaseCode] = -1;
						BaseCode = -1;
					}
				}
			} else {
				BaseCode = OpcodeArray[BaseCode];
			}

			OpcodeArray[Opcode] = BaseCode;
		} else {
			/* ADDQ or SUBQ */

			BaseCode = Opcode & 0x51F8;
			if ((BaseCode & 0x38) == 0x38) BaseCode |= (Opcode & 7);

			/* Special for Address Register Direct - Force LONG */

			if ((Opcode & 0x38) == 0x8) BaseCode = ((BaseCode & 0xFF3F) | 0x80);


			/* If mode = 3 or 4 and Size = byte and register = A7 */
			/* then make it a separate code							  */

			if ((Opcode & 0xC0) == 0) {
				if (((Opcode & 0x3F) == 0x1F) || ((Opcode & 0x3F) == 0x27)) {
					BaseCode |= 0x07;
				}
			}

			if (OpcodeArray[BaseCode] == -2) {
				char *Operation;
				int Dest = EAtoAMN(Opcode, FALSE);
				int SaveEDX = (Dest == 1);

				flags[FLAG_C] = 'S';
				if (SaveEDX) {
					flags[FLAG_V] = 'S';
					flags[FLAG_Z] = 'S';
					flags[FLAG_N] = 'S';
					flags[FLAG_X] = 'S';
				} else {
					flags[FLAG_V] = '-';
					flags[FLAG_Z] = '-';
					flags[FLAG_N] = '-';
					flags[FLAG_X] = '-';
				}

				if (allowtoADDQ[(Dest & 15)]) {
					switch (BaseCode & 0xC0) {
						case 0:
							Size = 'B';
							OpSize = 'b';
							break;

						case 0x40:
							Size = 'W';
							OpSize = 'h';
							break;

						case 0x80:
							Size = 'L';
							OpSize = 'w';
							break;
					}

					OpcodeArray[BaseCode] = BaseCode;

					Align();
					fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

					if ((Dest >= 2) && (Dest <= 10))
						SavePreviousPC();

					fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

					if (Dest == 0) { /* write to Dx */
						if (Size != 'L')
							TimingCycles += 4 ;
						else
							TimingCycles += 8 ;
					}

					if (Dest == 1) {
						if ((Size == 'L') || (Opcode & 0x100)) /* if long or SUBQ */
							TimingCycles += 8 ;
						else
							TimingCycles += 4 ;
					}

					if (Dest > 1) { /* write to mem */
						if (Size != 'L')
							TimingCycles += 8 ;
						else
							TimingCycles += 12 ;
					}

					if (Dest < 7) {
						fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[SaveEDX ? T0 : FLAG_C], regnameslong[OPCODE]);
					}

					if (Dest > 1) {
						flags[OPCODE] = 'T';
						flags[EFFADDR] = 'T';
						EffectiveAddressRead(Dest, Size, FLAG_C, T1, flags, TRUE, TRUE, TRUE);
					} else {
						fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[SaveEDX ? T0 : FLAG_C], regnameslong[SaveEDX ? T0 : FLAG_C]);
						fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[SaveEDX ? T0 : FLAG_C], regnameslong[SaveEDX ? T0 : FLAG_C], regnameslong[MEMBASE]);
						fprintf(fp, "\t\t l%c    %s,0x%2.2X(%s)\n", OpSize, regnameslong[T1], SaveEDX ? 32 : 0, regnameslong[SaveEDX ? T0 : FLAG_C]);
					}

					/* Sub Immediate from Opcode */

					fprintf(fp, "\t\t srl   %s,%s,9\n", regnameslong[OPCODE], regnameslong[OPCODE]);

					Immediate8();

					if (Opcode & 0x100) {
						/* SUBQ */
						Operation = "subu";
					} else {
						/* ADDQ */
						Operation = "addu";
					}

					/* For Data or Address register, operate directly */
					/* on the memory location. */

					fprintf(fp, "\t\t %s  %s,%s,%s\n", Operation, regnameslong[V0], regnameslong[T1], regnameslong[OPCODE]);
					if (Dest < 2) {
						fprintf(fp, "\t\t s%c    %s,0x%2.2X(%s)\n", OpSize, regnameslong[V0], SaveEDX ? 32 : 0, regnameslong[SaveEDX ? T0 : FLAG_C]);
					}

					/* No Flags for Address Direct */

					if (!SaveEDX) {
						SetFlags(Size, Opcode & 0x100 ? FLAGS_SUB : FLAGS_ADD, V0, T1, OPCODE);
					}

					if (Dest > 1) {
						flags[OPCODE] = '-';
						flags[EFFADDR] = '-';
						EffectiveAddressWrite(Dest, Size, 0, FALSE, flags, FALSE, FALSE);
					}

					Completed();
				} else {
					OpcodeArray[BaseCode] = -1;
					BaseCode = -1;
				}
				flags[FLAG_C] = '-';
			} else {
				BaseCode = OpcodeArray[BaseCode];
			}

			OpcodeArray[Opcode] = BaseCode;
		}
	}
}

/*
 * Branch Instructions
 *
 * BSR, Bcc
 *
 */

void branchinstructions(void)
{
	int Opcode,BaseCode;
	int Counter;
	char *Label;

	for (Opcode = 0x60; Opcode < 0x70; Opcode++) {
		/* Displacement = 0 -> 16 Bit displacement */

		BaseCode = Opcode * 0x100;
		OpcodeArray[BaseCode] = BaseCode;

		Align();
		fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

		TimingCycles += 10 ;

		if (Opcode == 0x60) {
			fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);
			Memory_Fetch('W', V0, TRUE);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[PC], regnameslong[PC], regnameslong[V0]);
			MemoryBanking(0, LocalStack);
			Completed();
		} else {
			if (Opcode != 0x61) {
				Label = ConditionDecode(Opcode & 0x0F, TRUE, FALSE);
				fprintf(fp, "\t\t addiu %s,%s,2        \t # Delay slot\n\n", regnameslong[PC], regnameslong[PC]);

				/* Code for Failed branch */

				fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);

				/* 2 less cycles for Failure */

				TimingCycles -= 2;
				Completed();
				TimingCycles += 2;

				/* Successful Branch */

				Align();
				fprintf(fp, "\t%s:\n", Label);

				Memory_Fetch('W', V0, TRUE);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[PC], regnameslong[PC], regnameslong[V0]);
				MemoryBanking(0, LocalStack);
				Completed();
			} else {
				/* BSR - Special Case */
				fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

				TimingCycles += 8 ;

				Memory_Fetch('W', OPCODE, TRUE);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[PC], regnameslong[OPCODE]);

				fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);
				PushPC("----------------SSSSSSSST---G-F-");

				fprintf(fp, "\t\t or    %s,$0,%s\n\n", regnameslong[PC], regnameslong[OPCODE]);
				MemoryBanking(0, LocalStack);
				Completed();
			}
		}

		/* 8 Bit Displacement */

		Align();
		fprintf(fp, "%s:\n", GenerateLabel(BaseCode + 1, 0));

		TimingCycles += 10 ;

		if (Opcode > 0x60) {
			if (Opcode != 0x61) {
				Label = ConditionDecode(Opcode & 0x0F, TRUE, FALSE);
				fprintf(fp, "\t\t addiu %s,%s,2        \t # Delay slot\n\n", regnameslong[PC], regnameslong[PC]);

				/* Code for Failed branch */

				TimingCycles -= 2;
				Completed();
				TimingCycles += 2;

				/* Successful Branch */

				Align();
				fprintf(fp, "\t%s:\n",Label);
			} else {
				/* BSR - Special Case */
				fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

				TimingCycles += 8 ;

				PushPC("----------------SSSSSSSST---G-F-");
			}
		} else {
			fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);
		}

		/* Common Ending */

		fprintf(fp, "\t\t seb   %s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE]);
		fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[PC], regnameslong[PC], regnameslong[OPCODE]);
		MemoryBanking(0, LocalStack);
		Completed();

		/* Fill up Opcode Array */

		for (Counter = 1; Counter < 0x100; Counter++)
			OpcodeArray[BaseCode + Counter] = BaseCode + 1;
	}
}

/*
 * Move Quick Commands
 *
 * Fairly simple, as only allowed to Data Registers
 *
 */

void moveq(void)
{
#if 0
	int Count;
#else
	int i, j;
#endif

	/* The Code */

	Align();
	fprintf(fp, "%s:\n", GenerateLabel(0x7000, 0));
	fprintf(fp, "\n\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

	TimingCycles += 4 ;

	fprintf(fp, "\t\t seb   %s,%s\n", regnameslong[V0], regnameslong[OPCODE]);
	fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[OPCODE], regnameslong[OPCODE]);
	fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);

	SetFlags('L', FLAGS_MOVE, V0, 0, 0);

	EffectiveAddressWrite(0, 'L', OPCODE, TRUE, "----------------SSSSSSSS----G-F-", FALSE, TRUE);
	Completed();

#if 0
	/* Set OpcodeArray (Not strictly correct, since some are illegal!) */

	for (Count = 0x7000; Count < 0x8000; Count++) {
		OpcodeArray[Count] = 0x7000;
	}
#else

	for(i = 0x0000; i <= 0x0E00; i += 0x0200)
		for(j = 0x0000; j <= 0x00FF; j += 0x0001)
				OpcodeArray[0x7000 + i + j] = 0x7000;
#endif
}

/*
 * Extended version of Add & Sub commands
 *
 */

void addx_subx(void)
{
	int	Opcode, BaseCode ;
	int	regx, type, leng, rm, regy, mode ;
	int ModeModX;
	int ModeModY;
	char  Size = ' ';
	char  OpSize = ' ';
	char * Operand = "";
	char flags[] = "------------------S-SSSS----G-F-";

	for (type = 0 ; type < 2 ; type ++) /* 0 = subx, 1 = addx */
		for (regx = 0 ; regx < 8 ; regx++)
			for (leng = 0 ; leng < 3 ; leng++)
				for (rm = 0 ; rm < 2 ; rm++)
					for (regy = 0 ; regy < 8 ; regy++) {
						Opcode = 0x9100 | (type << 14) | (regx << 9) | (leng << 6) | (rm << 3) | regy;

						BaseCode = Opcode & 0xd1c8;

						ModeModX = 0;
						ModeModY = 0;

						if ((rm == 1) && (leng == 0)) {
							if (regx == 7) {
								BaseCode |= (regx << 9);
								ModeModY = 16;
							}
							if (regy == 7) {
								BaseCode |= regy;
								ModeModX = 16;
							}
						}

						if (rm == 0)
							mode = 0;
						else
							mode = 4;

						switch (leng) {
							case 0:
								Size = 'B';
								OpSize = 'b';
								break;
							case 1:
								Size = 'W';
								OpSize = 'h';
								break;
							case 2:
								Size = 'L';
								OpSize = 'w';
								break;
						}

						if (OpcodeArray[BaseCode] == -2) {
							if (type == 0)
								Operand = "subu";
							else
								Operand = "addu";

							Align();
							fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

							if (mode == 4)
								SavePreviousPC();

							fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

							/* don't add in EA timing for ADDX,SUBX */

							AddEACycles = 0 ;

							if (rm == 0) { /* reg to reg */
								if (Size != 'L')
									TimingCycles += 4 ;
								else
									TimingCycles += 8 ;
							} else {
								if (Size != 'L')
									TimingCycles += 18 ;
								else
									TimingCycles += 30 ;
							}

							fprintf(fp, "\t\t andi  %s,%s,0x07\n\n", regnameslong[T0], regnameslong[OPCODE]);
							fprintf(fp, "\t\t srl   %s,%s,7\n\n", regnameslong[OPCODE], regnameslong[OPCODE]);
							fprintf(fp, "\t\t andi  %s,%s,0x1C\n\n", regnameslong[OPCODE], regnameslong[OPCODE]);

							/* Get Source */

							flags[OPCODE] = 'T';
							EffectiveAddressRead(mode + ModeModX, Size, T0, T0, flags, TRUE, TRUE, TRUE);
							flags[OPCODE] = '-';

							/* Get Destination (if needed) */

							if (mode == 4) {
								flags[T0] = 'T';
								//flags[OPCODE] = 'T';
								flags[EFFADDR] = 'T';
								EffectiveAddressRead(mode + ModeModY, Size, OPCODE, T1, flags, TRUE, FALSE, TRUE);
								flags[T0] = '-';
								//flags[OPCODE] = '-';
								flags[EFFADDR] = '-';
							} else {
								fprintf(fp, "\t\t addu   %s,%s,%s\n\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);
								fprintf(fp, "\t\t l%c    %s,0x00(%s)\n\n", OpSize, regnameslong[T1], regnameslong[OPCODE]);
							}

							/* Do the sums */
							fprintf(fp, "\t\t %s  %s,%s,%s\n", Operand, regnameslong[V0], regnameslong[T1], regnameslong[T0]);
							fprintf(fp, "\t\t %s  %s,%s,%s\n", Operand, regnameslong[V0], regnameslong[V0], regnameslong[FLAG_X]);

							/* Set the Flags */

							SetFlags(Size, type == 0 ? FLAGS_SUBX : FLAGS_ADDX, V0, T1, T0);

							/* Update the Data (if needed) */

							if (mode == 4) {
								flags[FLAG_C] = 'S';
								flags[FLAG_V] = 'S';
								flags[FLAG_N] = 'S';
								EffectiveAddressWrite(mode, Size, OPCODE, FALSE, flags, FALSE, FALSE);
								flags[FLAG_C] = '-';
								flags[FLAG_V] = '-';
								flags[FLAG_N] = '-';
							} else {
								fprintf(fp, "\t\t s%c    %s,0x00(%s)\n\n", OpSize, regnameslong[V0], regnameslong[OPCODE]);
							}

							Completed();
						}

						OpcodeArray[Opcode] = BaseCode ;
					}
}

/*
 * Logicals / Simple Maths (+ and -)
 *
 * OR,AND,CMP,EOR,ADD and SUB
 *
 */

void dumpx(int start, int reg, int type, char * Op, int dir, int leng, int mode, int sreg)
{
	int Opcode, BaseCode;
	char Size = ' ';
	char OpSize = ' ';
	int Dest;
	int SaveEDX;
	int SaveDir;
	char * allow = "";
	char * allowtypes[] = {"0-23456789ab----", "--2345678-------", "0123456789ab----", "0-2345678-------"};
	char flags[] = "---------------------SSS----G-F-";

	SaveDir = dir;

	switch (type) {
		case 0: /* or and*/
			flags[FLAG_X] = 'S';
			if (dir == 0)
				allow = allowtypes[0];
			else
				allow = allowtypes[1];
			break ;

		case 1: /* cmp*/
			flags[FLAG_X] = 'S';
			allow = allowtypes[2] ;
			break ;

		case 2: /* eor*/
			flags[FLAG_X] = 'S';
			allow = allowtypes[3] ;
			break ;

		case 3: /* adda suba cmpa*/
			if (start != 0xb000) {
				flags[FLAG_C] = 'S';
				flags[FLAG_V] = 'S';
				flags[FLAG_Z] = 'S';
				flags[FLAG_N] = 'S';
			}
			flags[FLAG_X] = 'S';
			allow = allowtypes[2] ;
			break ;

		case 4: /* sub add*/
			if (dir == 0)
				allow = allowtypes[0] ;
			else
				allow = allowtypes[1] ;
			break ;
	}

	if ((type == 4) && (dir == 0) && (leng > 0)) {
		allow = allowtypes[2] ; /* word and long ok*/
	}

	Opcode = start | (reg << 9 ) | (dir << 8) | (leng << 6) | (mode << 3) | sreg;

	BaseCode = Opcode & 0xf1f8;

	if (mode == 7) BaseCode |= sreg ;

	if ((mode == 3 || mode == 4) && ( leng == 0 ) && (sreg == 7 ))
		BaseCode |= sreg ;

	/* If Source = Data or Address register - combine into same routine */

	if (((Opcode & 0x38) == 0x08) && (allow[1] != '-')) {
		BaseCode &= 0xfff7;
	}

	Dest = EAtoAMN(Opcode, FALSE);
	SaveEDX = (Dest == 1) || (type == 3);

	if (allow[Dest & 0xf] != '-') {
		if (OpcodeArray[BaseCode] == -2) {
			switch (leng) {
				case 0:
					Size = 'B';
					OpSize = 'b';
					break;
				case 1:
					Size = 'W';
					OpSize = 'h';
					break;
				case 2:
					Size = 'L';
					OpSize = 'w';
					break;

				case 3: /* cmpa adda suba */
					if (dir == 0) {
						Size = 'W';
					} else {
						Size = 'L';
					}
					OpSize = 'w';
					dir = 0 ;
					break ;
			}

			Align();
			fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

			if ((Dest >= 2) && (Dest <= 10))
				SavePreviousPC();

			fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

			if (dir == 0) {
				if (Size != 'L')
					TimingCycles += 4;
				else
					TimingCycles += 6;
			} else {
				if (Size != 'L')
					TimingCycles += 8;
				else
					TimingCycles += 12;
			}

			if ((mode == 0) && (dir == 0) && (Size == 'L'))
				TimingCycles += 2 ;

			if ((mode == 1) && (dir == 0) && (Size != 'L'))
				TimingCycles += 4 ;

			if (Dest < 7) {	 /* Others do not need reg.no. */
				if ((Dest == 0) & (allow[1] != '-'))
					fprintf(fp, "\t\t andi  %s,%s,0x0f\n", regnameslong[T0], regnameslong[OPCODE]);
				else
					fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
			}

			fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[OPCODE], regnameslong[OPCODE]);
			fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);
			fprintf(fp, "\t\t l%c    %s,0x%2.2X(%s)\n", OpSize, regnameslong[T1], type != 3 ? 0 : 32, regnameslong[OPCODE]);

			flags[OPCODE] = 'T';
			flags[T1] = 'T';
			if (dir != 0) flags[Dest < 2 ? T0 : EFFADDR] = 'T';
			EffectiveAddressRead(Dest, Size, T0, dir == 0 ? V0 : T2, flags, TRUE, TRUE, TRUE);

			if (dir == 0) {
				fprintf(fp, "\t\t %s  %s,%s,%s\n", Op, regnameslong[T2], regnameslong[T1], regnameslong[V0]);
				if (type != 3) {
					if (start != 0xb000)
						fprintf(fp, "\t\t s%c    %s,0x00(%s)\n", OpSize, regnameslong[T2], regnameslong[OPCODE]);

					switch (start) {
						case 0x8000: /* OR  */
						case 0xc000: /* AND */
							SetFlags(Size, FLAGS_BITOP, T2, T1, V0);
							break;
						case 0xb000: /* CMP */
							SetFlags(Size, FLAGS_CMP, T2, T1, V0);
							break;
						case 0x9000: /* SUB */
							SetFlags(Size, FLAGS_SUB, T2, T1, V0);
							break;
						case 0xd000: /* ADD */
							SetFlags(Size, FLAGS_ADD, T2, T1, V0);
							break;
					}
				} else {
					if (start == 0xb000) {
						SetFlags('L', FLAGS_CMP, T2, T1, V0);
					} else {
						fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[T2], regnameslong[OPCODE]);
					}
				}
			} else {
				fprintf(fp, "\t\t %s  %s,%s,%s\n", Op, regnameslong[V0], regnameslong[T2], regnameslong[T1]);

				switch (start) {
					case 0x8000: /* OR  */
					case 0xb100: /* EOR */
					case 0xc000: /* AND */
						SetFlags(Size, FLAGS_BITOP, V0, T2, T1);
						break;
					case 0x9000: /* SUB */
						SetFlags(Size, FLAGS_SUB, V0, T2, T1);
						break;
					case 0xd000: /* ADD */
						SetFlags(Size, FLAGS_ADD, V0, T2, T1);
						break;
				}

				flags[FLAG_C] = 'S';
				flags[FLAG_V] = 'S';
				flags[FLAG_Z] = 'S';
				flags[FLAG_N] = 'S';
				flags[FLAG_X] = 'S';
				EffectiveAddressWrite(Dest, Size, T0, FALSE, flags, FALSE, FALSE);
			}
			Completed();
		}

		OpcodeArray[Opcode] = BaseCode;
	}

	dir = SaveDir;
}

void typelogicalmath(void)
{
	int dir, leng, mode, sreg ,reg ;

	for (reg = 0 ; reg < 8 ; reg++)
	{
		/* or */
		for (dir = 0 ; dir < 2 ; dir++)
			for (leng = 0 ; leng < 3; leng++)
				for (mode = 0 ; mode < 8 ; mode++)
					for (sreg = 0 ; sreg < 8 ; sreg++)
						dumpx( 0x8000, reg, 0, "or ", dir, leng, mode, sreg ) ;

		/* sub */
		for (dir = 0 ; dir < 2 ; dir++)
			for (leng = 0 ; leng < 3; leng++)
				for (mode = 0 ; mode < 8 ; mode++)
					for (sreg = 0 ; sreg < 8 ; sreg++)
						dumpx( 0x9000, reg, 4, "subu", dir, leng, mode, sreg ) ;

		/* suba */

		for (dir = 0 ; dir < 2 ; dir++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
					dumpx( 0x9000, reg, 3, "subu", dir, 3, mode, sreg ) ;


		/* cmp */
		for (leng = 0 ; leng < 3; leng++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
					dumpx( 0xb000, reg, 1, "subu", 0, leng, mode, sreg ) ;

		/* cmpa */

		for (dir = 0 ; dir < 2 ; dir++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
					dumpx( 0xb000, reg, 3, "subu", dir, 3, mode, sreg ) ;

		/* adda */

		for (dir = 0 ; dir < 2 ; dir++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
					dumpx( 0xd000, reg, 3, "addu", dir, 3, mode, sreg ) ;


		/* eor */
		for (leng = 0 ; leng < 3; leng++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
					dumpx( 0xb100, reg, 2, "xor ", 1, leng, mode, sreg ) ;

		/* and */
		for (dir = 0 ; dir < 2 ; dir++)
			for (leng = 0 ; leng < 3; leng++)
				for (mode = 0 ; mode < 8 ; mode++)
					for (sreg = 0 ; sreg < 8 ; sreg++)
						dumpx( 0xc000, reg, 0, "and ", dir, leng, mode, sreg ) ;

		/* add  */
		for (dir = 0 ; dir < 2 ; dir++)
			for (leng = 0 ; leng < 3; leng++)
				for (mode = 0 ; mode < 8 ; mode++)
					for (sreg = 0 ; sreg < 8 ; sreg++)
						dumpx( 0xd000, reg, 4, "addu", dir, leng, mode, sreg ) ;
	}
}

/*
 * Single commands missed out by routines above
 *
 */

void mul(void)
{
	int dreg, type, mode, sreg;
	int Opcode, BaseCode;
	int Dest;
	char allow[] = "0-23456789ab-----" ;
	char flags[] = "--------------------SSSST---G-F-";

	for (dreg = 0 ; dreg < 8 ; dreg++)
		for (type = 0 ; type < 2 ; type++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++) {
					Opcode = 0xc0c0 | (dreg << 9) | (type << 8) | (mode << 3) | sreg ;
					BaseCode = Opcode & 0xc1f8 ;
					if (mode == 7) {
						BaseCode |= sreg ;
					}

					Dest = EAtoAMN(Opcode, FALSE);
					if (allow[Dest & 0x0f] != '-') {
						if (OpcodeArray[ BaseCode ] == -2) {
							Align();
							fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

							if ((Dest >= 2) && (Dest <= 10))
								SavePreviousPC();

							fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

							TimingCycles += 54 ;

							if (mode < 7) {
								fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
							}

							fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[OPCODE], regnameslong[OPCODE]);
							fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);
							fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);
							fprintf(fp, "\t\t lh%c   %s,0x00(%s)\n", type == 0 ? 'u' : ' ', regnameslong[T1], regnameslong[OPCODE]);

							EffectiveAddressRead(Dest, 'W', T0, V0, flags, type == 0 ? FALSE : TRUE, TRUE, TRUE);

							fprintf(fp, "\t\t mult%c %s,%s\n", type == 0 ? 'u' : ' ', regnameslong[V0], regnameslong[T1]);

							fprintf(fp, "\t\t mflo  %s\n", regnameslong[T0]);
							fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[T0], regnameslong[OPCODE]);
							SetFlags('L', FLAGS_BITOP, T0, V0, T1);
							Completed();
						}

						OpcodeArray[Opcode] = BaseCode ;
					}
				}
}

/*
 * not
 * clr
 * neg
 * negx
 *
 */

void not(void)
{
	int	type, leng, mode, sreg;
	int	Opcode, BaseCode;
	int	Dest;
	int SaveEDX = 0;
	char Size = ' ';
	char OpSize = ' ';

	char allow[] = "0-2345678-------" ;
	char flags[] = "--------------------SSSS----G-F-";

	for (type = 0 ; type < 4 ; type++)
		for (leng = 0 ; leng < 3 ; leng++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++) {
					Opcode = 0x4000 | (type<<9) | (leng<<6) | (mode<<3) | sreg ;
					BaseCode = Opcode & 0x46f8 ;
					if (mode == 7) {
						BaseCode |= sreg ;
					}

					// A7+, A7-

					if ((leng == 0) && (sreg == 7) && (mode > 2) && (mode < 5)) {
						BaseCode |= sreg ;
					}

					Dest = EAtoAMN(Opcode, FALSE);

					if (allow[Dest & 0x0f] != '-') {
						switch (leng) {
							case 0:
								Size = 'B';
								OpSize = 'b';
								break;
							case 1:
								Size = 'W';
								OpSize = 'h';
								break;
							case 2:
								Size = 'L';
								OpSize = 'w';
								break;
						}

						if (OpcodeArray[BaseCode] == -2) {
							Align();
							fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

							if ((Dest >= 2) && (Dest <= 10))
								SavePreviousPC();

							fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

							if (Size != 'L')
								TimingCycles += 4;
							else
								TimingCycles += 6;

							if (Dest < 7)
								fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);

							/* CLR does not need to read source (although it does on a real 68000) */

							if (type != 1) {
								if (type == 0) flags[FLAG_Z] = 'S';
								if (type != 2) flags[FLAG_X] = 'S';
								flags[Dest < 2 ? OPCODE : EFFADDR] = 'T';
								EffectiveAddressRead(Dest, Size, OPCODE, type == 3 ? V0 : T0, flags, TRUE, TRUE, TRUE);
								flags[EFFADDR] = '-';
								flags[OPCODE] = '-';
								flags[FLAG_Z] = '-';
								flags[FLAG_X] = '-';
							}

							switch (type) {
								case 0: /* negx */

									fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[V0], regnameslong[T0], regnameslong[FLAG_X]);
									fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[V0], regnameslong[V0]);

									/* Set the Flags */

									SetFlags(Size, FLAGS_SUBX, V0, ZERO, T0);

									break;

								case 1: /* clr */
									ClearRegister(V0);
									EffectiveAddressWrite(Dest, Size, OPCODE, TRUE, flags, TRUE, TRUE);
									ClearRegister(FLAG_C);
									ClearRegister(FLAG_V);
									fprintf(fp, "\t\t ori   %s,$0,1\n", regnameslong[FLAG_Z]);
									ClearRegister(FLAG_N);
									break;

								case 2: /* neg */
									fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[V0], regnameslong[T0]);
									SetFlags(Size, FLAGS_SUB, V0, ZERO, T0);
									break;

								case 3: /* not */
									fprintf(fp, "\t\t nor   %s,$0,%s\n", regnameslong[V0], regnameslong[V0]);
									SetFlags(Size, FLAGS_BITOP, V0, ZERO, V0);
									break;
							}

							/* Update (unless CLR command) */

							if (type != 1) {
								flags[FLAG_C] = 'S';
								flags[FLAG_V] = 'S';
								flags[FLAG_Z] = 'S';
								flags[FLAG_N] = 'S';
								flags[FLAG_X] = 'S';
								EffectiveAddressWrite(Dest, Size, OPCODE, FALSE, flags, FALSE, FALSE);
								flags[FLAG_C] = '-';
								flags[FLAG_V] = '-';
								flags[FLAG_Z] = '-';
								flags[FLAG_N] = '-';
								flags[FLAG_X] = '-';
							}

							Completed();
						}

						OpcodeArray[Opcode] = BaseCode ;
					}
				}
}

/*
 * Move to/from USP
 *
 */

void moveusp(void)
{
	int Opcode, BaseCode ;
	int dir, sreg ;
	char * Label;

	for (dir = 0 ; dir < 2 ; dir++)
		for (sreg = 0 ; sreg < 8 ; sreg++) {
			Opcode = 0x4e60 | (dir << 3) | sreg ;
			BaseCode = Opcode & 0x4e68 ;

			if (OpcodeArray[BaseCode] == -2) {
				Align();
				Label = GenerateLabel(BaseCode, 0);
				fprintf(fp, "%s\n", Label);

				TimingCycles += 4;

				fprintf(fp, "\t\t lbu   %s,%s\n", regnameslong[T0], REG_SRH);
				fprintf(fp, "\t\t andi  %s,%s,0x20       \t # Supervisor Mode ?\n", regnameslong[T0], regnameslong[T0]);
				fprintf(fp, "\t\t beq   %s,$0,9f\n", regnameslong[T0]);
				fprintf(fp, "\t\t addiu %s,%s,2        \t # Delay slot\n\n", regnameslong[PC], regnameslong[PC]);

				fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
				fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[T0], regnameslong[T0]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T0], regnameslong[T0], regnameslong[MEMBASE]);

				if (dir == 0) { /* reg 2 USP */
					fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[V0], regnameslong[T0]);
					fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[V0], REG_USP);
				} else {
					fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[V0], REG_USP);
					fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[V0], regnameslong[T0]);
				}
				Completed();

				fprintf(fp, "\t9:\n");
				Exception(8, NULL);
			}
			OpcodeArray[Opcode] = BaseCode;
		}
}


/*
 * Check
 *
 */

void chk(void)
{
	int	dreg, mode, sreg;
	int	Opcode, BaseCode;
	int	Dest;
	char* Label;
	char DelaySlot[64] = "";

	char  *allow = "0-23456789ab----";
	char flags[] = "--------------------SSSS----G-F-";

	for (dreg = 0 ; dreg < 8; dreg++)
		for (mode = 0 ; mode < 8; mode++)
			for (sreg = 0 ; sreg < 8; sreg++) {
				Opcode = 0x4180 | (dreg << 9) | (mode << 3) | sreg ;
				BaseCode = Opcode & 0x41f8 ;

				if (mode == 7) {
					BaseCode |= sreg ;
				}

				Dest = EAtoAMN(Opcode, FALSE);

				if (allow[Dest & 0xf] != '-') {
					if (OpcodeArray[BaseCode] == -2) {
						Align();
						Label = GenerateLabel(BaseCode, 0);
						fprintf(fp, "%s:\n", Label);
						fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						TimingCycles += 10;

						fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[Dest < 7 ? T0 : OPCODE], regnameslong[OPCODE]);
						fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[Dest < 7 ? T0 : OPCODE], regnameslong[Dest < 7 ? T0 : OPCODE]);
						fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[Dest < 7 ? T0 : OPCODE], regnameslong[Dest < 7 ? T0 : OPCODE], regnameslong[MEMBASE]);

						if (Dest < 7) {
							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
							flags[T0] = 'T';
						}

						fprintf(fp, "\t\t lh    %s,0x00(%s)\n", regnameslong[Dest < 7 ? T0 : OPCODE], regnameslong[Dest < 7 ? T0 : OPCODE]);

						EffectiveAddressRead(Dest, 'W', OPCODE, V0, flags, TRUE, TRUE, TRUE);
						flags[T0] = '-';

						ClearRegister(FLAG_C);
						ClearRegister(FLAG_V);
						fprintf(fp, "\t\t sltiu %s,%s,1\n", regnameslong[FLAG_Z], regnameslong[Dest < 7 ? T0 : OPCODE]);

						/* N is set if data less than zero */

						fprintf(fp, "\t\t slt   %s,%s,$0\n", regnameslong[FLAG_N], regnameslong[Dest < 7 ? T0 : OPCODE]); /* is T0 < 0 */
						fprintf(fp, "\t\t bne   %s,$0,8f\n", regnameslong[FLAG_N]);

						fprintf(fp, "\t\t slt   %s,%s,%s         \t # Delay slot\n", regnameslong[T0], regnameslong[V0], regnameslong[Dest < 7 ? T0 : OPCODE]);
						fprintf(fp, "\t\t bne   %s,$0,9f\n", regnameslong[T0]);
						fprintf(fp, "\t\t nop                    \t # Delay slot\n");
						Completed();

						/* N is cleared if greated than compared number */

						Align();
						fprintf(fp, "\t9:\n");
						ClearRegister(FLAG_N);

						fprintf(fp, "\t8:\n");
						sprintf(DelaySlot, "ori   %s,$0,6          ", regnameslong[V0]);
						Exception(-1, DelaySlot);
						Completed();

					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

/*
 * Load Effective Address
 */

void LoadEffectiveAddress(void)
{
	int	Opcode, BaseCode;
	int	sreg, mode, dreg;
	int	Dest;
	char allow[] = "--2--56789a-----" ;

	for (sreg = 0 ; sreg < 8 ; sreg++)
		for (mode = 0 ; mode < 8 ; mode++)
			for (dreg = 0 ; dreg < 8 ; dreg++) {
				Opcode = 0x41c0 | (sreg << 9) | (mode << 3) | dreg;

				BaseCode = Opcode & 0x41f8 ;

				if (mode == 7)
					BaseCode = BaseCode | dreg;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest & 0x0f] != '-') {
					if (OpcodeArray[BaseCode] == -2) {
						Align();
						fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
						fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						switch (mode) {
							case 2:
								TimingCycles += 4;
								break;
							case 5:
							case 7:
							case 9:
								TimingCycles += 8;
								break;
							case 6:
							case 8:
							case 10:
								TimingCycles += 12;
								break;
						}

						if (mode < 7) {
							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
						}

						fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);

						EffectiveAddressCalculate(Dest, 'L', T0, TRUE, TRUE);
						fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[OPCODE]);
						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

/*
 * Negate BCD
 *
 */

void nbcd(void)
{
	int	Opcode, BaseCode;
	int	sreg, mode, Dest;
	char allow[] = "0-2345678-------" ;
	char flags[] = "------------------S-SSSS----G-F-";
	char Label0[64] = "";

	for (mode = 0 ; mode < 8 ; mode++)
		for (sreg = 0 ; sreg < 8 ; sreg++) {
			Opcode = 0x4800 | (mode << 3) | sreg ;
			BaseCode = Opcode & 0x4838 ;

			if (mode == 7)
				BaseCode |= sreg ;

			// A7+, A7-

			if ((sreg == 7) && (mode > 2) && (mode < 5)) {
				BaseCode |= sreg;
			}

			Dest = EAtoAMN(BaseCode, FALSE);

			if (allow[Dest & 0xf] != '-') {
				if (OpcodeArray[BaseCode] == -2) {
					Align();
					fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

					if ((Dest >= 2) && (Dest <= 10))
						SavePreviousPC();

					fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

					if (mode < 2)
						TimingCycles += 6;
					else
						TimingCycles += 8;

					fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);

					flags[Dest < 2 ? OPCODE : EFFADDR] = 'T';
					EffectiveAddressRead(Dest, 'B', OPCODE, V0, flags, FALSE, TRUE, TRUE);
					flags[OPCODE] = '-';
					flags[EFFADDR] = '-';

					fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[FLAG_V], regnameslong[V0]);
					fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], regnameslong[FLAG_X]);
					fprintf(fp, "\t\t andi  %s,%s,0xFF\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);

					fprintf(fp, "\t\t andi  %s,%s,0x0F\n", regnameslong[T1], regnameslong[V0]);
					fprintf(fp, "\t\t andi  %s,%s,0xF0\n", regnameslong[T0], regnameslong[V0]);

					fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[T1], regnameslong[T1]);
					fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T1], regnameslong[T1], regnameslong[FLAG_X]);
					fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[T0], regnameslong[T0]);
					fprintf(fp, "\t\t sltiu %s,%s,10\n", regnameslong[T2], regnameslong[T1]);
					fprintf(fp, "\t\t xori  %s,%s,1\n", regnameslong[T2], regnameslong[T2]);
					fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T2], regnameslong[T2]);
					fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T1], regnameslong[T1], regnameslong[T2]);
					fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T2], regnameslong[T2]);
					fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T1], regnameslong[T1], regnameslong[T2]);
					fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[V0], regnameslong[T0], regnameslong[T1]);
					fprintf(fp, "\t\t sltiu %s,%s,0x9a\n", regnameslong[FLAG_C], regnameslong[V0]);
					fprintf(fp, "\t\t xori  %s,%s,1        \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[FLAG_C]);
					fprintf(fp, "\t\t sll   %s,%s,5\n", regnameslong[T2], regnameslong[FLAG_C]);
					fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T2]);
					fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[T2], regnameslong[T2]);
					fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T2]);

					fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[FLAG_N], regnameslong[FLAG_V], regnameslong[V0]);
					fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], regnameslong[FLAG_N]);
					fprintf(fp, "\t\t srl   %s,%s,7        \t # Set Overflow\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);

					fprintf(fp, "\t\t sltiu %s,%s,1\n", regnameslong[T9], regnameslong[V0]);
					fprintf(fp, "\t\t and   %s,%s,%s       \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z], regnameslong[T9]);
					fprintf(fp, "\t\t srl   %s,%s,7         \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0]);
					fprintf(fp, "\t\t or    %s,$0,%s       \t # Copy Carry to X\n", regnameslong[FLAG_X], regnameslong[FLAG_C]);

					flags[FLAG_C] = 'S';
					flags[FLAG_V] = 'S';
					flags[FLAG_N] = 'S';
					EffectiveAddressWrite(Dest, 'B', OPCODE, FALSE, flags, FALSE, FALSE);
					flags[FLAG_C] = '-';
					flags[FLAG_V] = '-';
					flags[FLAG_N] = '-';
					Completed();
				}
				OpcodeArray[Opcode] = BaseCode;
			}
		}
}

void tas(void)
{
	int	Opcode, BaseCode;
	int	sreg,mode,Dest;
	char allow[] = "0-2345678-------";
	char flags[] = "--------------------SSSS----G-F-";

	for (mode = 0 ; mode < 8 ; mode++)
		for (sreg = 0 ; sreg < 8 ; sreg++) {
			Opcode = 0x4ac0 | (mode << 3) | sreg;
			BaseCode = Opcode & 0x4af8;

			if (mode == 7)
				BaseCode |= sreg;

			if ((sreg == 7) && (mode > 2) && (mode < 5)) {
				BaseCode |= sreg ;
			}

			Dest = EAtoAMN(BaseCode, FALSE);

			if (allow[Dest & 0xf] != '-') {
				if (OpcodeArray[BaseCode] == -2) {
					Align();
					fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

					if ((Dest >= 2) && (Dest <= 10))
						SavePreviousPC();

					fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

					if (mode < 2)
						TimingCycles += 4;
					else
						TimingCycles += 14;

					if (Dest < 7) {
						fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
					}

					flags[Dest < 2 ? OPCODE : EFFADDR] = 'T';
					EffectiveAddressRead(Dest, 'B', OPCODE, V0, flags, FALSE, TRUE, TRUE);
					flags[EFFADDR] = '-';
					flags[OPCODE] = '-';

					SetFlags('B', FLAGS_MOVE, V0, 0, 0);
					fprintf(fp, "\t\t or    %s,%s,0x80\n", regnameslong[V0], regnameslong[V0]);

					flags[FLAG_C] = 'S';
					flags[FLAG_V] = 'S';
					flags[FLAG_Z] = 'S';
					flags[FLAG_N] = 'S';
					EffectiveAddressWrite(Dest, 'B', OPCODE, FALSE, flags, FALSE, FALSE);
					flags[FLAG_C] = '-';
					flags[FLAG_V] = '-';
					flags[FLAG_Z] = '-';
					flags[FLAG_N] = '-';
					Completed();
				}
				OpcodeArray[Opcode] = BaseCode ;
			}
		}
}

/*
 * push Effective Address
 */

void PushEffectiveAddress(void)
{
	int	Opcode, BaseCode;
	int	mode,dreg;
	int	Dest;
	char allow[] = "--2--56789a-----";

	for (mode = 0 ; mode < 8 ; mode++)
		for (dreg = 0 ; dreg < 8 ; dreg++) {
			Opcode = 0x4840 | (mode << 3) | dreg ;

			BaseCode = Opcode & 0x4878 ;

			if (mode == 7)
				BaseCode = BaseCode | dreg ;

			Dest = EAtoAMN(BaseCode, FALSE);

			if (allow[Dest & 0x0f] != '-') {
				if (OpcodeArray[BaseCode] == -2) {
					Align();
					fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
					SavePreviousPC();
					fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

					switch (mode) {
						case 2:
							TimingCycles += 12;
							break;
						case 5:
						case 7:
						case 9:
							TimingCycles += 16;
							break;
						case 6:
						case 8:
						case 10:
							TimingCycles += 20;
							break;
					}

					if (mode < 7) {
						fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
					}

					EffectiveAddressCalculate(Dest, 'L', OPCODE, TRUE, TRUE);

					fprintf(fp, "\t\t lw    %s,%s     \t # Push onto Stack\n", regnameslong[FASTCALL_FIRST_REG], REG_A7);
					fprintf(fp, "\t\t addiu %s,%s,-4\n", regnameslong[FASTCALL_FIRST_REG], regnameslong[FASTCALL_FIRST_REG]);
					fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[FASTCALL_FIRST_REG], REG_A7);
					Memory_Write('L', FASTCALL_FIRST_REG, EFFADDR, "----------------SSSSSSSS----G-F-", LocalStack);
					Completed();
				}

				OpcodeArray[Opcode] = BaseCode ;
			}
		}
}

/*
 * Test
 *
 */

void tst(void)
{
	int	leng, mode, sreg;
	int	Opcode, BaseCode;
	int	Dest;
	char Size = ' ';
	char OpSize = ' ';

	char allow[] = "0-2345678-------" ;

	for (leng = 0 ; leng < 3 ; leng++)
		for (mode = 0 ; mode < 8 ; mode++)
			for (sreg = 0 ; sreg < 8 ; sreg++) {
				Opcode = 0x4a00 | (leng << 6) | (mode << 3) | sreg ;
				BaseCode = Opcode & 0x4af8 ;
				if (mode == 7) {
					BaseCode |= sreg ;
				}

				// A7+, A7-

				if ((leng == 0) && (sreg == 7) && (mode > 2) && (mode < 5))
				{
					BaseCode |= sreg ;
				}

				Dest = EAtoAMN(Opcode, FALSE);

				if ((allow[Dest & 0x0f] != '-')/* || (( mode == 1 ) && (leng != 0))*/) {
					switch (leng) {
						case 0:
							Size = 'B';
							OpSize = 'b';
							break;
						case 1:
							Size = 'W';
							OpSize = 'h';
							break;
						case 2:
							Size = 'L';
							OpSize = 'w';
							break;
					}

					if (OpcodeArray[ BaseCode ] == -2) {
						Align();
						fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

						if ((Dest >= 2) && (Dest <= 10))
							SavePreviousPC();

						fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						TimingCycles += 4;

						if (Dest < 7)
							fprintf(fp, "\t\t andi  %s,%s,0x07\n\n", regnameslong[OPCODE], regnameslong[OPCODE]);

						EffectiveAddressRead(Dest, Size, OPCODE, V0, "--------------------SSSS----G-F-", FALSE, TRUE, TRUE);

						SetFlags(Size, FLAGS_MOVE, V0, 0, 0);
						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

/*
 * Move registers too / from memory
 *
 */

void movem_reg_ea(void)
{
	int	leng,mode,sreg;
	int	Opcode, BaseCode;
	int	Dest;
	char  Size;
	char * Label;

	char *allow = "--2-45678-------";
	char flags[] = "--------TT--T-T-SSSSSSSS----G-F-";

	for (leng = 0; leng < 2; leng++)
		for (mode = 0; mode < 8; mode++)
			for (sreg = 0; sreg < 8; sreg++) {
				Opcode = 0x4880 | (leng << 6) | (mode << 3) | sreg ;
				BaseCode = Opcode & 0x4cf8 ;

				if (mode == 7) {
					BaseCode |= sreg ;
				}

				Dest = EAtoAMN(Opcode, FALSE);

				Size = "WL"[leng];

				if (allow[Dest & 0xf] != '-') {
					if (OpcodeArray[BaseCode] == - 2) {
						Align();
						Label = GenerateLabel(BaseCode, 0);
						fprintf(fp, "%s:\n", Label);
						SavePreviousPC();
						fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						switch (mode) {
							case 2:
							case 4:
								TimingCycles += 8 ;
								break;
							case 5:
							case 7:
								TimingCycles += 12 ;
								break;
							case 6:
							case 8:
								TimingCycles += 14 ;
								break;
						}

						Memory_Fetch('W', T4, FALSE);
						fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);

						if (mode < 7) {
							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						}

						if (mode == 4) {
							fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[OPCODE], regnameslong[OPCODE]);
							fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);
							fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[OPCODE]);
						} else
							EffectiveAddressCalculate(Dest, 'L', OPCODE, TRUE, TRUE);

						fprintf(fp, "\t\t ori   %s,$0,1\n", regnameslong[T0]);

						/* predecrement uses d0-d7..a0-a7  a7 first*/
						/* other modes use	a7-a0..d7-d0  d0 first*/

						if (Dest != 4)
							fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[T1], regnameslong[MEMBASE]);
						else
							fprintf(fp, "\t\t addiu %s,%s,0x3C\n", regnameslong[T1], regnameslong[MEMBASE]);

						fprintf(fp, "\t9:\n");
						fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[T2], regnameslong[T4], regnameslong[T0]);
						fprintf(fp, "\t\t beq   %s,$0,8f\n", regnameslong[T2]);

						fprintf(fp, "\t\t lw    %s,0x00(%s)      \t # Delay slot\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[T1]);  /* load a1 with current reg data */

						if (Dest == 4) {
							if (Size == 'W')						/* adjust pointer before write */
								fprintf(fp, "\t\t addiu %s,%s,-2\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
							else
								fprintf(fp, "\t\t addiu %s,%s,-4\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
						}

						if (Dest == 4) flags[OPCODE] = 'T';
						Memory_Write(Size, EFFADDR, FASTCALL_SECOND_REG, flags, LocalStack);
						if (Dest == 4) flags[OPCODE] = '-';

						if (Dest != 4){
							if (Size == 'W')					/* adjust pointer after write */
								fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
							else
								fprintf(fp, "\t\t addiu %s,%s,4\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
						}

						/* Update Cycle Count */

						if (Size == 'W')
							fprintf(fp, "\t\t addiu %s,%s,-4\n", regnameslong[ICNT], regnameslong[ICNT]);
						else
							fprintf(fp, "\t\t addiu %s,%s,-8\n", regnameslong[ICNT], regnameslong[ICNT]);

						fprintf(fp, "\t8:\n");

						fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T0], regnameslong[T0]);
						fprintf(fp, "\t\t andi  %s,%s,0xffff\n", regnameslong[TMPREG0], regnameslong[T0]);  /* check low 16 bits */
						fprintf(fp, "\t\t bne   %s,$0,9b\n", regnameslong[TMPREG0]);

						if (Dest != 4)
							fprintf(fp, "\t\t addiu %s,%s,4          \t # Delay slot\n", regnameslong[T1], regnameslong[T1]);
						else
							fprintf(fp, "\t\t addiu %s,%s,-4         \t # Delay slot\n", regnameslong[T1], regnameslong[T1]);

						if (Dest == 4) {
							fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[OPCODE]);
						}

						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

void movem_ea_reg(void)
{
	int	leng, mode, sreg;
	int	Opcode, BaseCode;
	int	Dest;
	char  Size;
	char* Label;

	char* allow = "--23-56789a-----";
	char flags[] = "--------TT--T-T-SSSSSSSS----G-F-";

	for (leng = 0; leng < 2; leng++)
		for (mode = 0; mode < 8; mode++)
			for (sreg =0 ; sreg < 8; sreg++) {
				Opcode = 0x4c80 | ( leng<<6) | (mode<<3) | sreg ;
				BaseCode = Opcode & 0x4cf8 ;

				if (mode == 7) {
					BaseCode |= sreg ;
				}

				Dest = EAtoAMN(Opcode, FALSE);

				Size = "WL"[leng] ;

				if (allow[Dest & 0xf] != '-') {
					if (OpcodeArray[BaseCode] == - 2) {
						Align();
						Label = GenerateLabel(BaseCode, 0);

						fprintf(fp, "%s:\n", Label);
						SavePreviousPC();
						fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						switch (mode) {
							case 2:
							case 4:
								TimingCycles += 8 ;
								break;
							case 5:
							case 7:
								TimingCycles += 12 ;
								break;
							case 6:
							case 8:
								TimingCycles += 14 ;
								break;
						}

						Memory_Fetch('W', T4, FALSE);
						fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);

						if (mode < 7) {
							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						}

						EffectiveAddressCalculate(Dest, 'L' , OPCODE, TRUE, TRUE);

						fprintf(fp, "\t\t ori   %s,$0,1\n", regnameslong[T0]);	  /* setup register list mask */

						/* use	a7-a0..d7-d0  d0 first*/

						fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[T1], regnameslong[MEMBASE]);	/* always start with D0 */

						fprintf(fp, "\t9:\n");
						fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[T2], regnameslong[T4], regnameslong[T0]);	/* is bit set for this register? */
						fprintf(fp, "\t\t beq   %s,$0,8f\n", regnameslong[T2]);
						fprintf(fp, "\t\t nop                    \t # Delay slot\n");

						if (mode == 3) flags[OPCODE] = 'T';
						Memory_Read(Size, EFFADDR, flags, LocalStack);
						if (mode == 3) flags[OPCODE] = '-';
						if (Size == 'W') {
							fprintf(fp, "\t\t sll   %s,%s,16\n", regnameslong[V0], regnameslong[V0]);
							fprintf(fp, "\t\t sra   %s,%s,16\n", regnameslong[V0], regnameslong[V0]);
						}

						fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[T1]);  /* load current reg with v0 */

						if (Size == 'W')					/* adjust pointer after write */
							fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
						else
							fprintf(fp, "\t\t addiu %s,%s,4\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);

						/* Update Cycle Count */

						if (Size == 'W')
							fprintf(fp, "\t\t addiu %s,%s,-4\n", regnameslong[ICNT], regnameslong[ICNT]);
						else
							fprintf(fp, "\t\t addiu %s,%s,-8\n", regnameslong[ICNT], regnameslong[ICNT]);

						fprintf(fp, "\t8:\n");
						fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T0], regnameslong[T0]);
						fprintf(fp, "\t\t andi  %s,%s,0xffff\n", regnameslong[TMPREG0], regnameslong[T0]);	/* check low 16 bits */
						fprintf(fp, "\t\t bne   %s,$0,9b\n", regnameslong[TMPREG0]);
						fprintf(fp, "\t\t addiu %s,%s,4          \t # Delay slot\n", regnameslong[T1], regnameslong[T1]);	/* adjust pointer to next reg */

						if (mode == 3)
							fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[OPCODE]);	/* reset Ax if mode = (Ax)+ */

						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

/*
 * Link / Unlink
 *
 * Local stack space
 *
 */

void link(void)
{
	int	sreg ;
	int	Opcode, BaseCode ;

	for (sreg = 0; sreg < 8; sreg++) {
		Opcode = 0x4e50 | sreg ;
		BaseCode = 0x4e50 ;

		if (OpcodeArray[BaseCode] == - 2) {
			Align();
			fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
			SavePreviousPC();
			fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

			TimingCycles += 16;

			fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[EFFADDR], REG_A7);
			fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
			fprintf(fp, "\t\t addiu %s,%s,-4\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);

			fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[OPCODE], regnameslong[OPCODE]);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);
			fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[OPCODE]);
			fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[OPCODE]);

			Memory_Write('L',EFFADDR, FASTCALL_SECOND_REG, "--------------T-SSSSSSSS----G-F-", LocalStack);

			Memory_Fetch('W', V0, TRUE);
			fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[EFFADDR], regnameslong[EFFADDR], regnameslong[V0]);
			fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[EFFADDR], REG_A7);

			Completed();
		}

		OpcodeArray[Opcode] = BaseCode ;
	}
}

void unlinkasm(void)
{
	int	sreg ;
	int	Opcode, BaseCode ;

	for (sreg = 0; sreg < 8; sreg++) {
		Opcode = 0x4e58 | sreg ;
		BaseCode = 0x4e58 ;

		if (OpcodeArray[BaseCode] == - 2) {
			Align();
			fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
			SavePreviousPC();
			fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

			TimingCycles += 12;

			fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
			fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[T0], regnameslong[T0]);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T0], regnameslong[T0], regnameslong[MEMBASE]);
			fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[EFFADDR], regnameslong[T0]);

			Memory_Read('L', EFFADDR, "--------T-----T-SSSSSSSS----G-F-", LocalStack);

			fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[V0], regnameslong[T0]);
			fprintf(fp, "\t\t addiu %s,%s,4\n", regnameslong[EFFADDR], regnameslong[EFFADDR]);
			fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[EFFADDR], REG_A7);
			Completed();
		}

		OpcodeArray[Opcode] = BaseCode ;
	}
}

void trap(void)
{
	int Count;
	int BaseCode = 0x4E40;
	char DelaySlot[64] = "";

	if (OpcodeArray[BaseCode] == -2) {
		Align();
		fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
		fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

		fprintf(fp, "\t\t andi  %s,%s,0x0f\n", regnameslong[V0], regnameslong[OPCODE]);
		sprintf(DelaySlot, "ori   %s,%s,32         ", regnameslong[V0], regnameslong[V0]);
		Exception(-1, DelaySlot);
		Completed();
	}

	for (Count = 0; Count <= 15; Count++)
		OpcodeArray[BaseCode + Count] = BaseCode;
}

void reset(void)
{
	int BaseCode = 0x4E70;
	char * Label;

	if (OpcodeArray[BaseCode] == -2) {
		Align();
		Label = GenerateLabel(BaseCode, 0);

		TimingCycles += 132;

		fprintf(fp, "%s:\n", Label );
		SavePreviousPC();

		fprintf(fp, "\t\t lbu   %s,%s\n", regnameslong[T0], REG_SRH);
		fprintf(fp, "\t\t andi  %s,%s,0x20       \t # Supervisor Mode ?\n", regnameslong[T0], regnameslong[T0]);
		fprintf(fp, "\t\t bne   %s,$0,9f\n", regnameslong[T0]);
		fprintf(fp, "\t\t addiu %s,%s,2        \t # Delay slot\n\n", regnameslong[PC], regnameslong[PC]);

		Exception(8, NULL);

		fprintf(fp, "\n\t9:\n");

		/* Prefetch next instruction and check for a reset callback */

		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], REG_RESET_CALLBACK);
		fprintf(fp, "\t\t beq   %s,$0,9f\n", regnameslong[T9]);
		fprintf(fp, "\t\t lhu   %s,0x00(%s)    \t # Delay slot\n", regnameslong[OPCODE], regnameslong[PC]);

		/* Callback for Reset */

		fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[ICNT], ICOUNT);

		if (SavedRegs[FLAG_C] == '-') {
			LocalStack -= 4;
			fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_C], LocalStack, regnameslong[SP]);
		}
		if (SavedRegs[FLAG_V] == '-') {
			LocalStack -= 4;
			fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_V], LocalStack, regnameslong[SP]);
		}
		if (SavedRegs[FLAG_Z] == '-') {
			LocalStack -= 4;
			fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_Z], LocalStack, regnameslong[SP]);
		}
		if (SavedRegs[FLAG_N] == '-') {
			LocalStack -= 4;
			fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_N], LocalStack, regnameslong[SP]);
		}
		if (SavedRegs[FLAG_X] == '-') {
			LocalStack -= 4;
			fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_X], LocalStack, regnameslong[SP]);
		}
		assert(LocalStack >= MINSTACK_POS);

		fprintf(fp, "\t\t jalr  %s\n", regnameslong[T9]);
		fprintf(fp, "\t\t sw    %s,%s    \t # Delay slot\n", regnameslong[PC], REG_PC);

		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[ICNT], ICOUNT);
		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[PC], REG_PC);

		if (SavedRegs[FLAG_X] == '-') {
			fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_X], LocalStack, regnameslong[SP]);
			LocalStack += 4;
		}
		if (SavedRegs[FLAG_N] == '-') {
			fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_N], LocalStack, regnameslong[SP]);
			LocalStack += 4;
		}
		if (SavedRegs[FLAG_Z] == '-') {
			fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_Z], LocalStack, regnameslong[SP]);
			LocalStack += 4;
		}
		if (SavedRegs[FLAG_V] == '-') {
			fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_V], LocalStack, regnameslong[SP]);
			LocalStack += 4;
		}
		if (SavedRegs[FLAG_C] == '-') {
			fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_C], LocalStack, regnameslong[SP]);
			LocalStack += 4;
		}

		if (SavedRegs[BASEPC] == '-')
			fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[BASEPC], OP_ROM);

		fprintf(fp, "\t\t lhu   %s,0x00(%s)\n", regnameslong[OPCODE], regnameslong[PC]);

		fprintf(fp, "\t9:\n");
		//fprintf(fp, "\t\t jmp   [%s_OPCODETABLE+ecx*4]\n\n", CPUtype);
		fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[TMPREG0], regnameslong[OPCODE]);
		fprintf(fp, "\t\t addu  %s,%s,%s\n\n", regnameslong[TMPREG0], regnameslong[TMPREG0], regnameslong[OPCODETBL]);
		fprintf(fp, "\t\t lw    %s,0x00(%s)\n\n", regnameslong[TMPREG0], regnameslong[TMPREG0]);
		fprintf(fp, "\t\t jr    %s\n", regnameslong[TMPREG0]);
		fprintf(fp, "\t\t addiu %s,%s,%d      \t # Delay slot\n", regnameslong[ICNT], regnameslong[ICNT], -TimingCycles);
	}
	OpcodeArray[BaseCode] = BaseCode;
}

void nop(void)
{
	int	BaseCode = 0x4e71;

	if (OpcodeArray[BaseCode] == -2) {
		Align();
		fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
		fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

		TimingCycles += 4;

		Completed();
		OpcodeArray[BaseCode] = BaseCode ;
	}
}

void stop(void)
{
	char TrueLabel[16];
	int	 BaseCode = 0x4e72 ;

	if (OpcodeArray[BaseCode] == -2) {
		Align();
		fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

		TimingCycles += 4;

		/* Must be in Supervisor Mode */

		fprintf(fp, "\t\t lbu   %s,%s\n", regnameslong[T0], REG_SRH);
		fprintf(fp, "\t\t andi  %s,%s,0x20       \t # Supervisor Mode ?\n", regnameslong[T0], regnameslong[T0]);
		fprintf(fp, "\t\t beq   %s,$0,9f\n", regnameslong[T0]);
		fprintf(fp, "\t\t addiu %s,%s,2        \t # Delay slot\n\n", regnameslong[PC], regnameslong[PC]);

		/* Next WORD is new SR */

		Memory_Fetch('W', V0, FALSE);
		fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);

		WriteCCR('W', V0);

		/* See if Valid interrupt waiting */

		CheckInterrupt = 0;

		fprintf(fp, "\t\t lbu   %s,%s\n", regnameslong[V1], REG_IRQ);
		fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[V0], regnameslong[V1]);

		fprintf(fp, "\t\t xori  %s,%s,0x07       \t # Always take 7\n", regnameslong[T9], regnameslong[V0]);
		//fprintf(fp, "\t\t beq   %s,$0,procint\n", regnameslong[T9]);
		fprintf(fp, "\t\t bne   %s,$0,8f\n", regnameslong[T9]);
		fprintf(fp, "\t\t nop                    \t # Delay slot\n\n");
		fprintf(fp, "\t\t j     procint\n");
		fprintf(fp, "\t\t nop                    \t # Delay slot\n\n");
		fprintf(fp, "\t8:\n\n");

		fprintf(fp, "\t\t lbu   %s,%s     \t # int mask\n", regnameslong[T0], REG_SRH);
		fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[T0]);
		fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T9], regnameslong[T0], regnameslong[V0]);
		//fprintf(fp, "\t\t bne   %s,$0,procint\n\n", regnameslong[T9]);
		fprintf(fp, "\t\t bgez  %s,8f\n", regnameslong[T9]);
		fprintf(fp, "\t\t nop                    \t # Delay slot\n\n");
		fprintf(fp, "\t\t j     procint\n");
		fprintf(fp, "\t\t nop                    \t # Delay slot\n\n");
		fprintf(fp, "\t8:\n\n");

		/* No int waiting - clear count, set stop */

		ClearRegister(ICNT);
		fprintf(fp, "\t\t ori   %s,%s,0x80\n", regnameslong[V1], regnameslong[V1]);
		fprintf(fp, "\t\t sb    %s,%s\n", regnameslong[V1], REG_IRQ);
		Completed();

		/* User Mode - Exception */

		Align();
		fprintf(fp, "\t9:\n");
		Exception(8, NULL);

		OpcodeArray[BaseCode] = BaseCode ;
	}
}

void ReturnFromException(void)
{
	char TrueLabel[16];
	char flags[] = "----------------SS---SSS----G-F-";

	int BaseCode = 0x4e73;

	Align();
	fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
	SavePreviousPC();

	TimingCycles += 20;

	/* Check in Supervisor Mode */

	fprintf(fp, "\t\t lbu   %s,%s\n", regnameslong[T0], REG_SRH);
	fprintf(fp, "\t\t andi  %s,%s,0x20       \t # Supervisor Mode ?\n", regnameslong[T0], regnameslong[T0]);
	fprintf(fp, "\t\t beq   %s,$0,9f\n", regnameslong[T0]);
	fprintf(fp, "\t\t addiu %s,%s,2        \t # Delay slot\n\n", regnameslong[PC], regnameslong[PC]);

	/* Get SR */

	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[FLAG_C], REG_A7);
	Memory_Read('W', FLAG_C, flags, LocalStack);
	fprintf(fp, "\t\t addiu %s,%s,6\n", regnameslong[T0], regnameslong[FLAG_C]);
	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[T0], REG_A7);
	fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[FLAG_C], regnameslong[FLAG_C]);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FLAG_V], regnameslong[V0]);

	/* Get PC */

	Memory_Read('L', FLAG_C, flags, LocalStack);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[T0], regnameslong[V0]);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[V0], regnameslong[FLAG_V]);

	/* Update CCR (and A7) */

	WriteCCR('W', V0);

	MemoryBanking(T0, LocalStack);
	Completed();

	fprintf(fp, "\t9:\n");
	Exception(8, NULL);

	OpcodeArray[BaseCode] = BaseCode;
}

void trapv(void)
{
	int BaseCode = 0x4E76;
	char * Label;

	if (OpcodeArray[BaseCode] == -2) {
		Align();
		Label = GenerateLabel(BaseCode, 0);
		fprintf(fp, "%s\n", Label);

		TimingCycles += 4;

		fprintf(fp, "\t\t beq   %s,$0,9f\n", regnameslong[FLAG_V]);
		fprintf(fp, "\t\t addiu %s,%s,2        \t # Delay slot\n\n", regnameslong[PC], regnameslong[PC]);
		Exception(7, NULL);

		fprintf(fp, "\t9:\n");
		Completed();
	}
	OpcodeArray[BaseCode] = BaseCode ;
}

void illegal_opcode(void)
{
	Align();
	fprintf(fp, "ILLEGAL:\n");
	//fprintf(fp, "\t\t la    %s,%sillegal_op\n", regnameslong[T0], PREF);
	//fprintf(fp, "\t\t la    %s,%sillegal_pc\n", regnameslong[T1], PREF);
	fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T9], regnameslong[PC], regnameslong[BASEPC]);
	//fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[OPCODE], regnameslong[T0]);
	//fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[T9], regnameslong[T1]);
	fprintf(fp, "\t\t sw    %s,%sillegal_op\n", regnameslong[OPCODE], PREF);
	fprintf(fp, "\t\t sw    %s,%sillegal_pc\n", regnameslong[T9], PREF);

	Exception(4, NULL);
}

/*
 * Return from subroutine
 * restoring flags as well
 *
 */

void ReturnandRestore(void)
{
	int BaseCode = 0x4e77;
	char flags[] = "----------------SS---SSS----G-F-";

	Align();
	fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
	SavePreviousPC();
	//fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

	TimingCycles += 20;

	/* Get SR */

	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[FLAG_C], REG_A7);
	Memory_Read('W', FLAG_C, flags, LocalStack);
	fprintf(fp, "\t\t addiu %s,%s,6\n", regnameslong[T0], regnameslong[FLAG_C]);
	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[T0], REG_A7);
	fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[FLAG_C], regnameslong[FLAG_C]);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FLAG_V], regnameslong[V0]);

	/* Get PC */

	Memory_Read('L', FLAG_C, flags, LocalStack);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[T0], regnameslong[V0]);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[V0], regnameslong[FLAG_V]);

	/* Update flags */

	WriteCCR('B', V0);

	MemoryBanking(T0, LocalStack);
	Completed();

	OpcodeArray[BaseCode] = BaseCode;
}

/*
 * Return from Subroutine
 *
 */

void rts(void)
{
	int	BaseCode = 0x4e75;

	if (OpcodeArray[BaseCode] == -2)
	{
		Align();
		fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
		SavePreviousPC();

		TimingCycles += 16;

		OpcodeArray[BaseCode] = BaseCode ;

		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[FASTCALL_FIRST_REG], REG_A7);
		fprintf(fp, "\t\t addiu %s,%s,4\n", regnameslong[T0], regnameslong[FASTCALL_FIRST_REG]);
		fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[T0], REG_A7);
		Memory_Read('L', FASTCALL_FIRST_REG, "----------------SSSSSSS-----G-F-", LocalStack);
		fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[T0], regnameslong[V0]);
		MemoryBanking(T0, LocalStack);
		Completed();
	}
}

void jmp_jsr(void)
{
	int	Opcode, BaseCode;
	int	dreg, mode, type;
	int	Dest;
	char allow[] = "--2--56789a-----";

	for (type = 0 ; type < 2 ; type++)
		for (mode = 0 ; mode < 8 ; mode++)
			for (dreg = 0 ; dreg < 8 ; dreg++) {
				Opcode = 0x4e80 | (type << 6) | (mode << 3) | dreg;
				BaseCode = Opcode & 0x4ef8;
				if (mode == 7)
					BaseCode = BaseCode | dreg ;

				Dest = EAtoAMN(BaseCode, FALSE);
				if (allow[Dest&0x0f] != '-') {
					if (OpcodeArray[BaseCode] == -2) {
						Align();
						fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
						SavePreviousPC();
						if ((type == 0) || (mode != 2))
							fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						switch (mode) {
							case 2:
								TimingCycles += 8;
								break;
							case 5:
							case 7:
							case 9:
								TimingCycles += 10;
								break;
							case 8:
								TimingCycles += 12;
								break;
							case 6:
							case 10:
								TimingCycles += 14;
								break;
						}

						if (type == 0) /* jsr takes 8 more than jmp */
							TimingCycles += 8;

						if (mode < 7) {
							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						}
						EffectiveAddressCalculate(Dest, 'L', OPCODE, TRUE, TRUE);

						/* jsr needs to push PC onto stack */

						if (type == 0) {
							PushPC("--------------T-SSSSSSS-----G-F-");
						}

						MemoryBanking(EFFADDR, LocalStack);
						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

void cmpm(void)
{
	int	Opcode, BaseCode;
	int	regx,leng,regy;
	int ModeModX, ModeModY;
	char Size = ' ';

	for (regx = 0; regx < 8 ; regx++)
		for (leng = 0; leng < 3 ; leng++)
			for (regy = 0; regy < 8 ; regy++) {
				Opcode = 0xb108 | (regx << 9) | (leng << 6) | regy;
				BaseCode = Opcode & 0xb1c8 ;

				ModeModX = 0;
				ModeModY = 0;

				if (leng == 0) {
					if (regx == 7) {
						BaseCode |= (regx << 9);
						ModeModX = 16;
					}

					if (regy == 7) {
						BaseCode |= regy;
						ModeModY = 16;
					}
				}

				switch (leng) {
					case 0:
						Size = 'B';
						break;
					case 1:
						Size = 'W';
						break;
					case 2:
						Size = 'L';
						break;
				}

				if (OpcodeArray[BaseCode] == -2) {
					Align();
					fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
					SavePreviousPC();
					fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

					AddEACycles = 0 ;

					if (Size != 'L')
						TimingCycles += 12 ;
					else
						TimingCycles += 20 ;

					fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
					fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[OPCODE], regnameslong[OPCODE]);
					fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);

					EffectiveAddressRead(3 + ModeModY, Size, T0, T0, "--------------------SSSST---G-F-", TRUE, TRUE, TRUE);
					EffectiveAddressRead(3 + ModeModX, Size, OPCODE, V0, "--------T-----------SSSS----G-F-", TRUE, FALSE, TRUE);

					fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[V1], regnameslong[V0], regnameslong[T0]);
					SetFlags(Size, FLAGS_CMP, V1, V0, T0);
					Completed();
				}

				OpcodeArray[Opcode] = BaseCode ;
			}
}

void exg(void)
{
	int	Opcode, BaseCode;
	int	regx, type, regy;
	int	opmask[3] = {0x08, 0x09, 0x11};

	for (regx = 0; regx < 8; regx++)
		for (type = 0; type < 3; type++)
			for (regy = 0; regy < 8; regy++) {
				Opcode = 0xc100 | (regx << 9) | (opmask[type] << 3) | regy;
				BaseCode = Opcode & 0xc1c8;

				if (OpcodeArray[BaseCode] == -2) {
					Align();
					fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
					fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

					TimingCycles += 6 ;

					fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
					fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[OPCODE], regnameslong[OPCODE]);
					fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[T0], regnameslong[T0]);
					fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);
					fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T0], regnameslong[T0], regnameslong[MEMBASE]);
					fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);

					if (type == 0) {
						fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[T0]);
						fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[V1], regnameslong[OPCODE]);
						fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
						fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V1], regnameslong[T0]);
					}
					if (type == 1) {
						fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[V0], regnameslong[T0]);
						fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[V1], regnameslong[OPCODE]);
						fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
						fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[V1], regnameslong[T0]);
					}
					if (type == 2) {
						fprintf(fp, "\t\t lw    %s,0x20(%s)\n", regnameslong[V0], regnameslong[T0]);
						fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[V1], regnameslong[OPCODE]);
						fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
						fprintf(fp, "\t\t sw    %s,0x20(%s)\n", regnameslong[V1], regnameslong[T0]);
					}

					Completed();
				}

				OpcodeArray[Opcode] = BaseCode ;
			}
}

void ext(void)
{
	int	Opcode, BaseCode;
	int	type, regy;

	for (type = 2; type < 4/*8*/; type++)	// MC68000 and 68010 don't have EXTB.L (Byte --> Long)
		for (regy = 0; regy < 8; regy++) {
			if (type > 3 && type < 7)
				continue;
			Opcode = 0x4800 | (type<<6) | regy;
			BaseCode = Opcode & 0x48c0;

			if (OpcodeArray[BaseCode] == -2) {
				Align();
				fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
				fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

				TimingCycles += 4 ;

				fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
				fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[OPCODE], regnameslong[OPCODE]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);

				if (type == 2) { /* byte to word */
					fprintf(fp, "\t\t lb    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
					SetFlags('W', FLAGS_BITOP, V0, 0, 0);
					fprintf(fp, "\t\t sh    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
				}
				if (type == 3) { /* word to long */
					fprintf(fp, "\t\t lh    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
					SetFlags('L', FLAGS_BITOP, V0, 0, 0);
					fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
				}
				if (type == 7) { /* byte to long */
					fprintf(fp, "\t\t lb    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
					SetFlags('L', FLAGS_BITOP, V0, 0, 0);
					fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
				}
				Completed();
			}

			OpcodeArray[Opcode] = BaseCode ;
		}
}

void swap(void)
{
	int	Opcode, BaseCode;
	int	sreg;

	for (sreg = 0; sreg < 8 ; sreg++) {
		Opcode = 0x4840 | sreg;
		BaseCode = Opcode & 0x4840;

		if (OpcodeArray[BaseCode] == -2) {
			Align();
			fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
			fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

			TimingCycles += 4 ;

			fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
			fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[OPCODE], regnameslong[OPCODE]);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);

			fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
			fprintf(fp, "\t\t ror   %s,%s,16\n", regnameslong[V0], regnameslong[V0]);
			fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
			SetFlags('L', FLAGS_BITOP, V0, 0, 0);
			Completed();
		}

		OpcodeArray[Opcode] = BaseCode ;
	}
}

/*
 * Line A and Line F commands
 *
 */

void LineA(void)
{
	int Count;

	/* Line A */

	Align();
	fprintf(fp, "%s:\n", GenerateLabel(0xA000, 0));
	fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);
	Exception(0x0A, NULL);

	for (Count = 0xA000; Count < 0xB000; Count++) {
		OpcodeArray[Count] = 0xA000;
	}
}

void LineF(void)
{
	int Count;

	/* Line F */

	Align();
	fprintf(fp, "%s:\n", GenerateLabel(0xF000, 0));
	fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);
	Exception(0x0B, NULL);

	for (Count = 0xF000; Count < 0x10000; Count++) {
		OpcodeArray[Count] = 0xF000;
	}
}

/*
 * Moves To/From CCR and SR
 *
 * (Move from CCR is 68010 command)
 *
 */

void movesr(void)
{
	int Opcode, BaseCode;
	int type, mode, sreg;
	int Dest;
	char allow[] = "0-2345678-------";
	char Size;
	char DelaySlot[64] = "";

	for (type = 0; type < 4 ; type++)
		for (mode = 0; mode < 8 ; mode++)
			for (sreg = 0; sreg < 8 ; sreg++) {
				Opcode = 0x40c0 | (type << 9) | ( mode << 3 ) | sreg;

				/* To has extra modes */

				if (type > 1) {
					allow[0x9] = '9';
					allow[0xa] = 'a';
					allow[0xb] = 'b' ;
				}

				if ((type == 0) || (type == 3))
					Size = 'W'; /* SR */
				else
					Size = 'B'; /* CCR */

				BaseCode = Opcode & 0x46f8 ;

				if (mode == 7)
					BaseCode |= sreg ;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest & 0xf] != '-') {
					if (OpcodeArray[BaseCode] == -2) {
						char TrueLabel[16];

						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

						if ((Dest >= 2) && (Dest <= 10))
							SavePreviousPC();

						if (type != 3)
							fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						if (type > 1) /* move to */
							TimingCycles += 12 ;
						else {
							if (mode < 2)
								TimingCycles += 6 ;
							else
								TimingCycles += 8 ;
						}

						/* If Move to SR then must be in Supervisor Mode */

						if (type == 3) {

							fprintf(fp, "\t\t lbu   %s,%s\n", regnameslong[T0], REG_SRH);
							fprintf(fp, "\t\t andi  %s,%s,0x20       \t # Supervisor Mode ?\n", regnameslong[T0], regnameslong[T0]);
							fprintf(fp, "\t\t beq   %s,$0,9f\n", regnameslong[T0]);
							fprintf(fp, "\t\t addiu %s,%s,2        \t # Delay slot\n\n", regnameslong[PC], regnameslong[PC]);
						}

						/* 68010 Command ? */
						if ((type == 1) && (mode < 7)) {
							sprintf(DelaySlot, "\t\t andi  %s,%s,0x07     \t # Delay slot\n", regnameslong[OPCODE], regnameslong[OPCODE]);
							CheckCPUtype(1, DelaySlot, NULL);
						} else if (type == 1) {
							CheckCPUtype(1, NULL, NULL);
						} else if (mode < 7) {
							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						}

						/* Always read/write word 2 bytes */
						if (type < 2) {
							/*if (type == 0) {	// Move from SR first read the destionation before write
								EffectiveAddressRead(Dest & 15, 'W', OPCODE, V0, "-----------------SSSSSSSST---G-F-", FALSE, TRUE, TRUE);
								ReadCCR(Size, V0);
								EffectiveAddressWrite(Dest & 15, 'W', OPCODE, FALSE, "----------------SSSSSSSS----G-F-", FALSE, FALSE);
							} else*/ {
								ReadCCR(Size, V0);
								EffectiveAddressWrite(Dest & 15, 'W', OPCODE, TRUE, "----------------SSSSSSSS----G-F-", TRUE, TRUE);
							}
						} else {
							EffectiveAddressRead(Dest & 15, 'W', OPCODE, V0, "---------------------SSS----G-F-", FALSE, TRUE, TRUE);
							WriteCCR(Size, V0);
						}
						Completed();

						/* Exception if not Supervisor Mode */

						if (type == 3) {
							/* Was in User Mode - Exception */

							fprintf(fp, "\t9:\n");
							Exception(8, NULL);
						}
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

/*
 * Decimal mode Add / Subtracts
 *
 */

void abcd_sbcd(void)
{
	int	Opcode, BaseCode;
	int	regx, type, rm, regy, mode;
	int ModeModX;
	int ModeModY;
	char flags[] = "------------------S-SSSS----G-F-";

	for (type = 0; type < 2; type ++) /* 0 = sbcd, 1 = abcd */
		for (regx = 0; regx < 8; regx++)
			for (rm = 0; rm < 2; rm++)
				for (regy = 0; regy < 8; regy++) {
					Opcode = 0x8100 | (type << 14) | (regx << 9) | (rm << 3) | regy;
					BaseCode = Opcode & 0xc108;

					ModeModX = 0;
					ModeModY = 0;

					if (rm == 0)
						mode = 0;
					else {
						mode = 4;

						if (regx == 7) {
							BaseCode |= (regx << 9);
							ModeModY = 16;
						}
						if (regy == 7) {
							BaseCode |= regy;
							ModeModX = 16;
						}
					}

					if (OpcodeArray[BaseCode] == -2) {
						Align();
						fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

						if (mode == 4)
							SavePreviousPC();

						fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						AddEACycles = 0;

						if (rm == 0)
							TimingCycles += 6;
						else
							TimingCycles += 18;

						fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
						fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);

						flags[OPCODE] = 'T';
						EffectiveAddressRead(mode + ModeModX, 'B', T0, T0, flags, FALSE, TRUE, TRUE);
						flags[EFFADDR] = 'T';
						flags[T0] = 'T';
						EffectiveAddressRead(mode + ModeModY, 'B', OPCODE, T1, flags, FALSE, FALSE, TRUE);
						flags[OPCODE] = '-';
						flags[EFFADDR] = '-';
						flags[T0] = '-';

						fprintf(fp, "\t\t %s  %s,%s,%s\n", type == 0 ? "subu" : "addu", regnameslong[FLAG_V], regnameslong[T1], regnameslong[T0]);
						fprintf(fp, "\t\t %s  %s,%s,%s\n", type == 0 ? "subu" : "addu", regnameslong[FLAG_V], regnameslong[FLAG_V], regnameslong[FLAG_X]);
						fprintf(fp, "\t\t andi  %s,%s,0xFF\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);

						fprintf(fp, "\t\t andi  %s,%s,0x0F\n", regnameslong[T2], regnameslong[T0]);
						fprintf(fp, "\t\t andi  %s,%s,0x0F\n", regnameslong[T3], regnameslong[T1]);
						fprintf(fp, "\t\t andi  %s,%s,0xF0\n", regnameslong[T0], regnameslong[T0]);
						fprintf(fp, "\t\t andi  %s,%s,0xF0\n", regnameslong[T1], regnameslong[T1]);

						if (type == 0) {
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T3], regnameslong[T3], regnameslong[T2]);
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T3], regnameslong[T3], regnameslong[FLAG_X]);
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T1], regnameslong[T1], regnameslong[T0]);
							fprintf(fp, "\t\t sltiu %s,%s,10\n", regnameslong[T4], regnameslong[T3]);
							fprintf(fp, "\t\t xori  %s,%s,1\n", regnameslong[T4], regnameslong[T4]);
							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T4], regnameslong[T4]);
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T3], regnameslong[T3], regnameslong[T4]);
							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T4], regnameslong[T4]);
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T3], regnameslong[T3], regnameslong[T4]);
							fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[V0], regnameslong[T1], regnameslong[T3]);
							fprintf(fp, "\t\t sltiu %s,%s,0x9a\n", regnameslong[FLAG_C], regnameslong[V0]);
							fprintf(fp, "\t\t xori  %s,%s,1        \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[FLAG_C]);
							fprintf(fp, "\t\t sll   %s,%s,5\n", regnameslong[T4], regnameslong[FLAG_C]);
							fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T4]);
							fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[T4], regnameslong[T4]);
							fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T4]);

							fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[FLAG_N], regnameslong[FLAG_V], regnameslong[V0]);
							fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], regnameslong[FLAG_N]);
							fprintf(fp, "\t\t srl   %s,%s,7        \t # Set Overflow\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
						} else {
							fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T3], regnameslong[T3], regnameslong[T2]);
							fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T3], regnameslong[T3], regnameslong[FLAG_X]);
							fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T1], regnameslong[T1], regnameslong[T0]);
							fprintf(fp, "\t\t sltiu %s,%s,10\n", regnameslong[T4], regnameslong[T3]);
							fprintf(fp, "\t\t xori  %s,%s,1\n", regnameslong[T4], regnameslong[T4]);
							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T4], regnameslong[T4]);
							fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T3], regnameslong[T3], regnameslong[T4]);
							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T4], regnameslong[T4]);
							fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T3], regnameslong[T3], regnameslong[T4]);
							fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[V0], regnameslong[T1], regnameslong[T3]);
							fprintf(fp, "\t\t sltiu %s,%s,0x9a\n", regnameslong[FLAG_C], regnameslong[V0]);
							fprintf(fp, "\t\t xori  %s,%s,1        \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[FLAG_C]);
							fprintf(fp, "\t\t sll   %s,%s,5\n", regnameslong[T4], regnameslong[FLAG_C]);
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T4]);
							fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[T4], regnameslong[T4]);
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T4]);

							fprintf(fp, "\t\t nor   %s,$0,%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
							fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[FLAG_N], regnameslong[FLAG_V], regnameslong[V0]);
							fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[FLAG_V], regnameslong[FLAG_N]);
							fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
							fprintf(fp, "\t\t andi  %s,%s,0x01     \t # Set Overflow\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
						}

						/* Should only clear Zero flag if not zero */

						fprintf(fp, "\t\t sltiu %s,%s,1\n", regnameslong[T9], regnameslong[V0]);
						fprintf(fp, "\t\t and   %s,%s,%s       \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z], regnameslong[T9]);
						fprintf(fp, "\t\t srl   %s,%s,7         \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0]);
						fprintf(fp, "\t\t or    %s,$0,%s       \t # Copy Carry to X\n", regnameslong[FLAG_X], regnameslong[FLAG_C]);

						flags[FLAG_C] = 'S';
						EffectiveAddressWrite(mode, 'B', OPCODE, FALSE, flags, FALSE, FALSE);
						flags[FLAG_C] = '-';
						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
}

/*
 * Rotate Left / Right
 *
 */

void rol_ror(void)
{
	int Opcode, BaseCode;
	int dreg, dr, leng, ir, sreg;
	char Size = ' ';
	char * Label;
	char flags[] = "--------------------SSSS----G-F-";

	for (dreg = 0; dreg < 8; dreg++)
		for (dr = 0; dr < 2; dr++)
			for (leng = 0; leng < 3; leng++)
				for (ir = 0 ; ir < 2; ir++)
					for (sreg = 0; sreg < 8; sreg++) {
						Opcode = 0xe018 | (dreg << 9) | (dr << 8) | (leng << 6) | (ir << 5) | sreg ;
						BaseCode = Opcode & 0xe1f8 ;

						switch (leng) {
							case 0:
								Size = 'B';
								break;
							case 1:
								Size = 'W';
								break;
							case 2:
								Size = 'L';
								break;
						}

						if (OpcodeArray[ BaseCode ] == -2) {
							Align();
							fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
							fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

							if (Size != 'L')
								TimingCycles += 6 ;
							else
								TimingCycles += 8 ;

							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
							fprintf(fp, "\t\t srl   %s,%s,%d\n", regnameslong[OPCODE], regnameslong[OPCODE], ir == 0 ? 9 : 7);

							flags[T0] = 'T';
							if (ir == 0) {
								Immediate8();
							} else {
								fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);
								EffectiveAddressRead(0, 'L', OPCODE, OPCODE, flags, FALSE, FALSE, TRUE);
								fprintf(fp, "\t\t andi  %s,%s,0x3F\n", regnameslong[OPCODE], regnameslong[OPCODE]);
							}

							flags[OPCODE] = 'T';
							EffectiveAddressRead(0, Size, T0, V0, flags, FALSE, TRUE, TRUE);
							flags[OPCODE] = '-';
							flags[T0] = '-';

							/* shift 0 - no time, no shift and clear carry */

							fprintf(fp, "\t\t beq   %s,$0,9f\n", regnameslong[OPCODE]);
							fprintf(fp, "\t\t and   %s,$0,$0        \t # Delay slot - Clear Carry\n", regnameslong[FLAG_C]);

							/* allow 2 cycles per shift */

							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T9], regnameslong[OPCODE]);
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[ICNT], regnameslong[ICNT], regnameslong[T9]);

							if (Size != 'L')
								fprintf(fp, "\t\t andi  %s,%s,0x%2.2X\n", regnameslong[OPCODE], regnameslong[OPCODE], Size == 'W' ? 15 : 7);

							if (dr == 0) {
								if (Size != 'L') {
									fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[T9], regnameslong[OPCODE]);
									fprintf(fp, "\t\t andi  %s,%s,0x%2.2X\n", regnameslong[T9], regnameslong[T9], Size == 'W' ? 15 : 7);
									fprintf(fp, "\t\t sllv  %s,%s,%s\n", regnameslong[T9], regnameslong[V0], regnameslong[T9]);
									fprintf(fp, "\t\t srlv  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[OPCODE]);
									fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T9]);
									fprintf(fp, "\t\t srl   %s,%s,%d        \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[V0], Size == 'W' ? 15 : 7);
								} else {
									fprintf(fp, "\t\t rorv  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[OPCODE]);
									fprintf(fp, "\t\t srl   %s,%s,31        \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[V0]);
								}
							} else {
								fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[T9], regnameslong[OPCODE]);
								if (Size != 'L') {
									fprintf(fp, "\t\t andi  %s,%s,0x%2.2X\n", regnameslong[T9], regnameslong[T9], Size == 'W' ? 15 : 7);
									fprintf(fp, "\t\t srlv  %s,%s,%s\n", regnameslong[T9], regnameslong[V0], regnameslong[T9]);
									fprintf(fp, "\t\t sllv  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[OPCODE]);
									fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T9]);
									fprintf(fp, "\t\t andi  %s,%s,0x01      \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[V0]);
								} else {
									fprintf(fp, "\t\t rorv  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T9]);
									fprintf(fp, "\t\t andi  %s,%s,0x01      \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[V0]);
								}
							}

							fprintf(fp, "\t9:\n");

							if (Size != 'L')
								fprintf(fp, "\t\t andi  %s,%s,0x%2.2X\n", regnameslong[V0], regnameslong[V0], Size == 'W' ? 0xffff : 0xff);
							fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Overflow\n", regnameslong[FLAG_V]);
							fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[V0]);
							fprintf(fp, "\t\t srl   %s,%s,%d        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0], Size == 'L' ? 31 : (Size == 'W' ? 15 : 7));

							flags[FLAG_C] = 'S';
							flags[FLAG_V] = 'S';
							flags[FLAG_Z] = 'S';
							flags[FLAG_N] = 'S';
							EffectiveAddressWrite(0, Size, T0, FALSE, flags, FALSE, FALSE);
							flags[FLAG_C] = '-';
							flags[FLAG_V] = '-';
							flags[FLAG_Z] = '-';
							flags[FLAG_N] = '-';

							Completed();
						}

						OpcodeArray[Opcode] = BaseCode ;
					}
}

void rol_ror_ea(void)
{
	int Opcode, BaseCode;
	int dr, mode, sreg;
	int Dest;
	char allow[] = "--2345678-------";
	char flags[] = "--------------------SSSS----G-F-";

	for (dr = 0; dr < 2; dr++)
		for (mode = 0; mode < 8; mode++)
			for (sreg = 0 ; sreg < 8; sreg++) {
				Opcode = 0xe6c0 | (dr << 8) | (mode << 3) | sreg;
				BaseCode = Opcode & 0xfff8;

				if (mode == 7)
					BaseCode |= sreg ;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest & 0xf] != '-') {
					if (OpcodeArray[ BaseCode ] == -2) {
						Align();
						fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

						if ((Dest >= 2) && (Dest <= 10))
							SavePreviousPC();

						fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						TimingCycles += 8 ;

						if ((Dest & 0xf) < 7)
							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						flags[EFFADDR] = 'T';
						EffectiveAddressRead(Dest & 0xf, 'W', OPCODE, V0, flags, FALSE, TRUE, TRUE);
						flags[EFFADDR] = '-';

						if (dr == 0) {
							fprintf(fp, "\t\t sll   %s,%s,15\n", regnameslong[T9], regnameslong[V0]);
							fprintf(fp, "\t\t srl   %s,%s,1\n", regnameslong[V0], regnameslong[V0]);
							fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T9]);
							fprintf(fp, "\t\t srl   %s,%s,15        \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[V0]);
						} else {
							fprintf(fp, "\t\t srl   %s,%s,15\n", regnameslong[T9], regnameslong[V0]);
							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[V0], regnameslong[V0]);
							fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T9]);
							fprintf(fp, "\t\t andi  %s,%s,0x01      \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[V0]);
						}

						fprintf(fp, "\t\t andi  %s,%s,0xffff\n", regnameslong[V0], regnameslong[V0]);
						fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Overflow\n", regnameslong[FLAG_V]);
						fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[V0]);
						fprintf(fp, "\t\t srl   %s,%s,15        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0]);

						flags[FLAG_C] = 'S';
						flags[FLAG_V] = 'S';
						flags[FLAG_Z] = 'S';
						flags[FLAG_N] = 'S';
						EffectiveAddressWrite(Dest & 0xf, 'W', OPCODE, FALSE, flags, FALSE, FALSE);
						flags[FLAG_C] = '-';
						flags[FLAG_V] = '-';
						flags[FLAG_Z] = '-';
						flags[FLAG_N] = '-';

						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

/*
 * Logical Shift Left / Right
 *
 */

void lsl_lsr(void)
{
	int Opcode, BaseCode;
	int dreg, dr, leng, ir, sreg;
	char Size = ' ';
	char flags[] = "--------------------SSSS----G-F-";

	for (dreg = 0; dreg < 8; dreg++)
		for (dr = 0; dr < 2; dr++)
			for (leng = 0; leng < 3; leng++)
				for (ir = 0; ir < 2; ir++)
					for (sreg = 0; sreg < 8; sreg++) {
						Opcode = 0xe008 | (dreg << 9) | (dr << 8) | (leng << 6) | (ir << 5) | sreg;
						BaseCode = Opcode & 0xe1f8;

						switch (leng) {
							case 0:
								Size = 'B';
								break;
							case 1:
								Size = 'W';
								break;
							case 2:
								Size = 'L';
								break;
						}

						if (OpcodeArray[ BaseCode ] == -2) {
							Align();
							fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
							fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

							if (Size != 'L')
								TimingCycles += 6 ;
							else
								TimingCycles += 8 ;

							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
							fprintf(fp, "\t\t srl   %s,%s,%d\n", regnameslong[OPCODE], regnameslong[OPCODE], ir == 0 ? 9 : 7);

							flags[T0] = 'T';
							if (ir == 0) {
								Immediate8();
							} else {
								fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);
								EffectiveAddressRead(0, 'L', OPCODE, OPCODE, flags, FALSE, FALSE, TRUE);
								fprintf(fp, "\t\t andi  %s,%s,0x3F\n", regnameslong[OPCODE], regnameslong[OPCODE]);
							}

							/* and 2 cycles per shift */

							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T9], regnameslong[OPCODE]);
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[ICNT], regnameslong[ICNT], regnameslong[T9]);

							flags[OPCODE] = 'T';
							EffectiveAddressRead(0, Size, T0, V0, flags, FALSE, TRUE, TRUE);
							flags[OPCODE] = '-';
							flags[T0] = '-';

							/* ASG: on the 68k, the shift count is mod 64; on the MIPS, the */
							/* shift count is mod 32; we need to check for shifts of 32-63 */
							/* and produce zero */
							fprintf(fp, "\t\t andi  %s,%s,0x20\n", regnameslong[T1], regnameslong[OPCODE]);
							fprintf(fp, "\t\t addiu %s,%s,-1\n", regnameslong[T2], regnameslong[OPCODE]);
							fprintf(fp, "\t\t andi  %s,%s,0x20\n", regnameslong[T3], regnameslong[T2]);

							if (dr == 0) {
								fprintf(fp, "\t\t srlv  %s,%s,%s\n", regnameslong[FLAG_C], regnameslong[V0], regnameslong[T2]);
								fprintf(fp, "\t\t andi  %s,%s,0x01     \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[FLAG_C]);
								// The actual work
								fprintf(fp, "\t\t srlv  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[OPCODE]);
							} else {
								if (Size != 'L')
									fprintf(fp, "\t\t sll   %s,%s,%d\n", regnameslong[FLAG_C], regnameslong[V0], Size == 'W' ? 16 : 24);
								fprintf(fp, "\t\t sllv  %s,%s,%s\n", regnameslong[FLAG_C], regnameslong[FLAG_C], regnameslong[T2]);
								fprintf(fp, "\t\t srl   %s,%s,31       \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[FLAG_C]);
								// The actual work
								fprintf(fp, "\t\t sllv  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[OPCODE]);
							}

							if (Size != 'L')
								fprintf(fp, "\t\t andi  %s,%s,0x%2.2X\n", regnameslong[V0], regnameslong[V0], Size == 'W' ? 0xffff : 0xff);
							fprintf(fp, "\t\t movn  %s,$0,%s         \t # Return 0 if shift by 32-63\n", regnameslong[V0], regnameslong[T1]);
							fprintf(fp, "\t\t movn  %s,$0,%s       \t # Clear Carry if shift by >32\n", regnameslong[FLAG_C], regnameslong[T3]);
							fprintf(fp, "\t\t movz  %s,$0,%s       \t # Clear Carry if shift by 0\n", regnameslong[FLAG_C], regnameslong[OPCODE]);
							fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Overflow\n", regnameslong[FLAG_V]);
							if (Size == 'L') {
								fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0]);
							} else {
								fprintf(fp, "\t\t srl   %s,%s,%d        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0], Size == 'W' ? 15 : 7);
							}
							fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[V0]);
							fprintf(fp, "\t\t movn  %s,%s,%s      \t # Set X if not shift by 0\n", regnameslong[FLAG_X], regnameslong[FLAG_C], regnameslong[OPCODE]);

							flags[FLAG_C] = 'S';
							flags[FLAG_V] = 'S';
							flags[FLAG_Z] = 'S';
							flags[FLAG_N] = 'S';
							EffectiveAddressWrite(0, Size, T0, FALSE, flags, FALSE, FALSE);
							flags[FLAG_C] = '-';
							flags[FLAG_V] = '-';
							flags[FLAG_Z] = '-';
							flags[FLAG_N] = '-';

							Completed();
						}

						OpcodeArray[Opcode] = BaseCode ;
					}
}

void lsl_lsr_ea(void)
{
	int Opcode, BaseCode;
	int dr, mode, sreg;
	int Dest;
	char allow[] = "--2345678-------";
	char flags[] = "---------------------SSS----G-F-";

	for (dr = 0; dr < 2; dr++)
		for (mode = 0; mode < 8; mode++)
			for (sreg = 0; sreg < 8; sreg++) {
				Opcode = 0xe2c0 | (dr << 8) | (mode << 3) | sreg;
				BaseCode = Opcode & 0xfff8;

				if (mode == 7)
					BaseCode |= sreg ;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest & 0xf] != '-') {
					if (OpcodeArray[ BaseCode ] == -2) {

						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode, 0));

						if ((Dest >= 2) && (Dest <= 10))
							SavePreviousPC();

						fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						TimingCycles += 8 ;

						if ((Dest & 0xf) < 7)
							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						flags[EFFADDR] = 'T';
						EffectiveAddressRead(Dest & 0xf, 'W', OPCODE, V0, flags, FALSE, TRUE, TRUE);
						flags[EFFADDR] = '-';

						if (dr == 0) {
							fprintf(fp, "\t\t andi  %s,%s,0x01      \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[V0]);
							fprintf(fp, "\t\t srl   %s,%s,1\n", regnameslong[V0], regnameslong[V0]);
						} else {
							fprintf(fp, "\t\t srl   %s,%s,15        \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[V0]);
							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[V0], regnameslong[V0]);
						}

						fprintf(fp, "\t\t andi  %s,%s,0xffff\n", regnameslong[V0], regnameslong[V0]);
						fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Overflow\n", regnameslong[FLAG_V]);
						fprintf(fp, "\t\t srl   %s,%s,15        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0]);
						fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[V0]);
						fprintf(fp, "\t\t movn  %s,%s,%s      \t # Set X if not shift by 0\n", regnameslong[FLAG_X], regnameslong[FLAG_C], regnameslong[OPCODE]);

						flags[FLAG_C] = 'S';
						flags[FLAG_V] = 'S';
						flags[FLAG_Z] = 'S';
						flags[FLAG_N] = 'S';
						flags[FLAG_X] = 'S';
						EffectiveAddressWrite(Dest & 0xf, 'W', OPCODE, FALSE, flags, FALSE, FALSE);
						flags[FLAG_C] = '-';
						flags[FLAG_V] = '-';
						flags[FLAG_Z] = '-';
						flags[FLAG_N] = '-';
						flags[FLAG_X] = '-';
						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

/*
 * Rotate Left / Right though Extend
 *
 */

void roxl_roxr(void)
{
	int Opcode, BaseCode;
	int dreg, dr, leng, ir, sreg;
	char Size = ' ';
	char* Label;
	char flags[] = "--------------------SSSS----G-F-";

	for (dreg = 0; dreg < 8; dreg++)
		for (dr = 0; dr < 2; dr++)
			for (leng = 0; leng < 3; leng++)
				for (ir = 0; ir < 2; ir++)
					for (sreg = 0; sreg < 8; sreg++) {
						Opcode = 0xe010 | (dreg << 9) | (dr << 8) | (leng << 6) | (ir << 5) | sreg;
						BaseCode = Opcode & 0xe1f8;

						switch (leng) {
							case 0:
								Size = 'B';
								break;
							case 1:
								Size = 'W';
								break;
							case 2:
								Size = 'L';
								break;
						}

						if (OpcodeArray[ BaseCode ] == -2) {
							Align();
							fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
							fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

							if (Size != 'L')
								TimingCycles += 6 ;
							else
								TimingCycles += 8 ;

							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
							fprintf(fp, "\t\t srl   %s,%s,%d\n", regnameslong[OPCODE], regnameslong[OPCODE], ir == 0 ? 9 : 7);

							flags[T0] = 'T';
							if (ir == 0) {
								Immediate8();
							} else {
								fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);
								EffectiveAddressRead(0, 'L', OPCODE, OPCODE, flags, FALSE, FALSE, TRUE);
								fprintf(fp, "\t\t andi  %s,%s,0x3F\n", regnameslong[OPCODE], regnameslong[OPCODE]);
							}

							flags[OPCODE] = 'T';
							EffectiveAddressRead(0, Size, T0, V0, flags, FALSE, TRUE, TRUE);
							flags[OPCODE] = '-';
							flags[T0] = '-';

							/* shift 0 - no time, no shift and copy X to carry */

							fprintf(fp, "\t\t beq   %s,$0,9f\n", regnameslong[OPCODE]);
							fprintf(fp, "\t\t or    %s,$0,%s       \t # Delay slot - Copy X to Carry\n", regnameslong[FLAG_C], regnameslong[FLAG_X]);

							/* allow 2 cycles per shift */

							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T9], regnameslong[OPCODE]);
							fprintf(fp, "\t\t andi  %s,%s,1\n", regnameslong[T1], regnameslong[OPCODE]);
							fprintf(fp, "\t\t bne   %s,%s,7f\n", regnameslong[T1], regnameslong[OPCODE]);
							fprintf(fp, "\t\t subu  %s,%s,%s       \t # Delay slot\n", regnameslong[ICNT], regnameslong[ICNT], regnameslong[T9]);

							/* rotate by 1 */
							if (dr == 0) {
								fprintf(fp, "\t\t andi  %s,%s,0x01      \t # Set X\n", regnameslong[FLAG_X], regnameslong[V0]);
								fprintf(fp, "\t\t sll   %s,%s,%d\n", regnameslong[FLAG_C], regnameslong[FLAG_C], Size == 'L' ? 31 : (Size == 'W' ? 15 : 7));
								fprintf(fp, "\t\t srl   %s,%s,1\n", regnameslong[V0], regnameslong[V0]);
								fprintf(fp, "\t\t bgez  $0,6f\n");
								fprintf(fp, "\t\t or    %s,%s,%s        \t # Delay slot\n", regnameslong[V0], regnameslong[V0], regnameslong[FLAG_C]);
							} else {
								fprintf(fp, "\t\t srl   %s,%s,%d        \t # Set X\n", regnameslong[FLAG_X], regnameslong[V0], Size == 'L' ? 31 : (Size == 'W' ? 15 : 7));
								fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[V0], regnameslong[V0]);
								fprintf(fp, "\t\t bgez  $0,6f\n");
								fprintf(fp, "\t\t or    %s,%s,%s        \t # Delay slot\n", regnameslong[V0], regnameslong[V0], regnameslong[FLAG_C]);
							}

							/* rotate by n (n > 1) */

							fprintf(fp, "\t7:\n");
							fprintf(fp, "\t\t ori   %s,$0,%d\n", regnameslong[T3], Size == 'L' ? 33 : (Size == 'W' ? 17 : 9));
							fprintf(fp, "\t8:\n");
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T1], regnameslong[OPCODE], regnameslong[T3]);
							fprintf(fp, "\t\t addiu %s,%s,-1\n", regnameslong[T9], regnameslong[T1]);
							fprintf(fp, "\t\t slt   %s,%s,$0\n", regnameslong[T9], regnameslong[T9]);
							fprintf(fp, "\t\t beq   %s,$0,8b\n", regnameslong[T9]);
							fprintf(fp, "\t\t movz  %s,%s,%s        \t # Delay slot\n", regnameslong[OPCODE], regnameslong[T1], regnameslong[T9]);
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T3], regnameslong[T3], regnameslong[OPCODE]);
							fprintf(fp, "\t\t addiu %s,%s,-1\n", regnameslong[T1], regnameslong[OPCODE]);

							if (dr == 0) {
								fprintf(fp, "\t\t addiu %s,%s,-1\n", regnameslong[T2], regnameslong[T3]);
								fprintf(fp, "\t\t sllv  %s,%s,%s\n", regnameslong[FLAG_C], regnameslong[FLAG_C], regnameslong[T2]);
								fprintf(fp, "\t\t srlv  %s,%s,%s\n", regnameslong[FLAG_X], regnameslong[V0], regnameslong[T1]);
								fprintf(fp, "\t\t andi  %s,%s,0x01     \t # Set X\n", regnameslong[FLAG_X], regnameslong[FLAG_X]);
								fprintf(fp, "\t\t sllv  %s,%s,%s\n", regnameslong[T9], regnameslong[V0], regnameslong[T3]);
								fprintf(fp, "\t\t srlv  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[OPCODE]);
								fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T9]);
								fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[FLAG_C]);
							} else {
								fprintf(fp, "\t\t sllv  %s,%s,%s\n", regnameslong[FLAG_C], regnameslong[FLAG_C], regnameslong[T1]);
								fprintf(fp, "\t\t srlv  %s,%s,%s\n", regnameslong[FLAG_X], regnameslong[V0], regnameslong[T3]);
								fprintf(fp, "\t\t andi  %s,%s,0x01     \t # Set X\n", regnameslong[FLAG_X], regnameslong[FLAG_X]);
								fprintf(fp, "\t\t srlv  %s,%s,%s\n", regnameslong[T9], regnameslong[V0], regnameslong[T3]);
								fprintf(fp, "\t\t sllv  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[OPCODE]);
								fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T9]);
								fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[FLAG_C]);
							}

							fprintf(fp, "\t6:\n");
							fprintf(fp, "\t\t or    %s,$0,%s       \t # Copy X to Carry\n", regnameslong[FLAG_C], regnameslong[FLAG_X]);
							fprintf(fp, "\t9:\n");
							if (Size != 'L')
								fprintf(fp, "\t\t andi  %s,%s,0x%2.2X\n", regnameslong[V0], regnameslong[V0], Size == 'W' ? 0xffff : 0xff);
							fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Overflow\n", regnameslong[FLAG_V]);
							fprintf(fp, "\t\t srl   %s,%s,%d        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0], Size == 'L' ? 31 : (Size == 'W' ? 15 : 7));
							fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[V0]);

							flags[FLAG_C] = 'S';
							flags[FLAG_V] = 'S';
							flags[FLAG_Z] = 'S';
							flags[FLAG_N] = 'S';
							EffectiveAddressWrite(0, Size, T0, FALSE, flags, FALSE, FALSE);
							flags[FLAG_C] = '-';
							flags[FLAG_V] = '-';
							flags[FLAG_Z] = '-';
							flags[FLAG_N] = '-';

							Completed();
						}

						OpcodeArray[Opcode] = BaseCode ;
					}

	/* End with special routines for ROXx.x #1,Dx  */

	for (dr = 0; dr < 2; dr++)
		for (leng = 0; leng < 3; leng++)
			for (sreg = 0; sreg < 8; sreg++) {
				Opcode = 0xe210 | (dr << 8) | (leng << 6) | sreg;
				BaseCode = Opcode & 0xe3d0;

				switch (leng) {
					case 0:
						Size = 'B';
						break;
					case 1:
						Size = 'W';
						break;
					case 2:
						Size = 'L';
						break;
				}

				if (sreg == 0) {
					Align();
					fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
					fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

					if (Size != 'L')
						TimingCycles += 8;
					else
						TimingCycles += 10;

					fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
					flags[OPCODE] = 'T';
					EffectiveAddressRead(0, Size, OPCODE, V0, flags, FALSE, TRUE, TRUE);
					flags[OPCODE] = '-';

					if (dr == 0) {
						fprintf(fp, "\t\t sll   %s,%s,%d\n", regnameslong[T1], regnameslong[FLAG_X], Size == 'L' ? 31 : (Size == 'W' ? 15 : 7));
						fprintf(fp, "\t\t andi  %s,%s,0x01      \t # Set X\n", regnameslong[FLAG_X], regnameslong[V0]);
						fprintf(fp, "\t\t srl   %s,%s,1\n", regnameslong[V0], regnameslong[V0]);
						fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T1]);
					} else {
						fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[T1], regnameslong[FLAG_X]);
						fprintf(fp, "\t\t srl   %s,%s,%d        \t # Set X\n", regnameslong[FLAG_X], regnameslong[V0], Size == 'L' ? 31 : (Size == 'W' ? 15 : 7));
						fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[V0], regnameslong[V0]);
						fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T1]);
					}

					fprintf(fp, "\t\t or    %s,$0,%s       \t # Copy X to Carry\n", regnameslong[FLAG_C], regnameslong[FLAG_X]);
					if (Size != 'L')
						fprintf(fp, "\t\t andi  %s,%s,0x%2.2X\n", regnameslong[V0], regnameslong[V0], Size == 'W' ? 0xffff : 0xff);
					fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Overflow\n", regnameslong[FLAG_V]);
					fprintf(fp, "\t\t srl   %s,%s,%d        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0], Size == 'L' ? 31 : (Size == 'W' ? 15 : 7));
					fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[V0]);

					flags[FLAG_C] = 'S';
					flags[FLAG_V] = 'S';
					flags[FLAG_Z] = 'S';
					flags[FLAG_N] = 'S';
					EffectiveAddressWrite(0, Size, OPCODE, FALSE, flags, FALSE, FALSE);
					flags[FLAG_C] = '-';
					flags[FLAG_V] = '-';
					flags[FLAG_Z] = '-';
					flags[FLAG_N] = '-';

					Completed();
				}
				OpcodeArray[Opcode] = BaseCode;
			}
}


void roxl_roxr_ea(void)
{
	int Opcode, BaseCode;
	int dr, mode, sreg;
	int Dest;
	char allow[] = "--2345678-------";
	char flags[] = "--------------------SSSS----G-F-";

	for (dr = 0; dr < 2; dr++)
		for (mode = 0; mode < 8; mode++)
			for (sreg = 0; sreg < 8; sreg++) {
				Opcode = 0xe4c0 | (dr << 8) | (mode << 3) | sreg;
				BaseCode = Opcode & 0xfff8;

				if (mode == 7)
					BaseCode |= sreg;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest & 0xf] != '-') {
					if (OpcodeArray[ BaseCode ] == -2) {
						Align();
						fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

						if ((Dest >= 2) && (Dest <= 10))
							SavePreviousPC();

						fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						TimingCycles += 8 ;

						if ((Dest & 0xf) < 7)
							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						flags[EFFADDR] = 'T';
						EffectiveAddressRead(Dest & 0xf, 'W', OPCODE, V0, flags, FALSE, TRUE, TRUE);
						flags[EFFADDR] = '-';

						if (dr == 0) {
							fprintf(fp, "\t\t sll   %s,%s,15\n", regnameslong[T1], regnameslong[FLAG_X]);
							fprintf(fp, "\t\t andi  %s,%s,0x01      \t # Set X\n", regnameslong[FLAG_X], regnameslong[V0]);
							fprintf(fp, "\t\t srl   %s,%s,1\n", regnameslong[V0], regnameslong[V0]);
							fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T1]);
						} else {
							fprintf(fp, "\t\t andi  %s,%s,0x01\n", regnameslong[T1], regnameslong[FLAG_X]);
							fprintf(fp, "\t\t srl   %s,%s,15        \t # Set X\n", regnameslong[FLAG_X], regnameslong[V0]);
							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[V0], regnameslong[V0]);
							fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T1]);
						}

						fprintf(fp, "\t\t or    %s,$0,%s       \t # Copy X to Carry\n", regnameslong[FLAG_C], regnameslong[FLAG_X]);
						fprintf(fp, "\t\t andi  %s,%s,0xffff\n", regnameslong[V0], regnameslong[V0]);
						fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Overflow\n", regnameslong[FLAG_V]);
						fprintf(fp, "\t\t srl   %s,%s,15        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0]);
						fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[V0]);

						flags[FLAG_C] = 'S';
						flags[FLAG_V] = 'S';
						flags[FLAG_Z] = 'S';
						flags[FLAG_N] = 'S';
						EffectiveAddressWrite(Dest & 0xf, 'W', OPCODE, FALSE, flags, FALSE, FALSE);
						flags[FLAG_C] = '-';
						flags[FLAG_V] = '-';
						flags[FLAG_Z] = '-';
						flags[FLAG_N] = '-';

						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

/*
 * Arithmetic Shift Left / Right
 *
 */

void asl_asr(void)
{
	int Opcode, BaseCode;
	int dreg, dr, leng, ir, sreg;
	char Size = ' ';
	char* Label;
	char flags[] = "--------------------SSSS----G-F-";

	/* Normal routines for codes */

	for (dreg = 0; dreg < 8; dreg++)
		for (dr = 0; dr < 2; dr++)
			for (leng = 0; leng < 3; leng++)
				for (ir = 0; ir < 2; ir++)
					for (sreg = 0; sreg < 8; sreg++) {
						Opcode = 0xe000 | (dreg << 9) | (dr << 8) | (leng << 6) | (ir << 5) | sreg;
						BaseCode = Opcode & 0xe1f8;

						switch (leng) {
							case 0:
								Size = 'B';
								break;
							case 1:
								Size = 'W';
								break;
							case 2:
								Size = 'L';
								break;
						}

						if (OpcodeArray[ BaseCode ] == -2) {
							Align();
							fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));
							fprintf(fp, "\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

							Label = GenerateLabel(0, 1);

							if (Size != 'L')
								TimingCycles += 6;
							else
								TimingCycles += 8;

							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
							fprintf(fp, "\t\t srl   %s,%s,%d\n", regnameslong[OPCODE], regnameslong[OPCODE], ir == 0 ? 9 : 7);

							flags[T0] = 'T';
							if (ir == 0) {
								Immediate8();
							} else {
								fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);
								EffectiveAddressRead(0, 'L', OPCODE, OPCODE, flags, FALSE, FALSE, TRUE);
								fprintf(fp, "\t\t andi  %s,%s,0x3F\n", regnameslong[OPCODE], regnameslong[OPCODE]);
							}

							flags[OPCODE] = 'T';
							EffectiveAddressRead(0, Size, T0, V0, flags, TRUE, TRUE, TRUE);
							flags[OPCODE] = '-';
							flags[T0] = '-';

							fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Overflow\n", regnameslong[FLAG_V]);
							fprintf(fp, "\t\t beq   %s,$0,9f\n", regnameslong[OPCODE], Label);
							fprintf(fp, "\t\t and   %s,$0,$0        \t # Delay slot - Clear Carry\n", regnameslong[FLAG_C]);

							/* allow 2 cycles per shift */

							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[T9], regnameslong[OPCODE]);
							fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[ICNT], regnameslong[ICNT], regnameslong[T9]);

							if (dr == 0) {
								/* ASR */

								/* ASG: on the 68k, the shift count is mod 64; on the MIPS, the */
								/* shift count is mod 32; we need to check for shifts of 32-63 */
								/* and effectively shift 31 */
								fprintf(fp, "\t\t addiu %s,%s,-1\n", regnameslong[T1], regnameslong[OPCODE]);
								fprintf(fp, "\t\t sll   %s,%s,26\n", regnameslong[T9], regnameslong[OPCODE]);
								fprintf(fp, "\t\t sra   %s,%s,31\n", regnameslong[T9], regnameslong[T9]);
								fprintf(fp, "\t\t andi  %s,%s,0x1f\n", regnameslong[T9], regnameslong[T9]);
								fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[T9]);
								fprintf(fp, "\t\t sll   %s,%s,26\n", regnameslong[T9], regnameslong[T1]);
								fprintf(fp, "\t\t sra   %s,%s,31\n", regnameslong[T9], regnameslong[T9]);
								fprintf(fp, "\t\t andi  %s,%s,0x1f\n", regnameslong[T9], regnameslong[T9]);
								fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[T1], regnameslong[T1], regnameslong[T9]);

								fprintf(fp, "\t\t srlv  %s,%s,%s\n", regnameslong[FLAG_C], regnameslong[V0], regnameslong[T1]);
								fprintf(fp, "\t\t andi  %s,%s,0x01     \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[FLAG_C]);

								fprintf(fp, "\t\t srav  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[OPCODE]);

								/* Mode 0 write does not affect Flags */
								EffectiveAddressWrite(0, Size, T0, FALSE, flags, FALSE, FALSE);

								/* Update Flags */
								fprintf(fp, "\t\t or    %s,$0,%s       \t # Copy Carry to X\n", regnameslong[FLAG_X], regnameslong[FLAG_C]);
								fprintf(fp, "\t9:\n");

							} else {
								/* ASL */

								fprintf(fp, "\t\t sll   %s,%s,26\n", regnameslong[T9], regnameslong[OPCODE]);
								fprintf(fp, "\t\t sra   %s,%s,31\n", regnameslong[T9], regnameslong[T9]);
								fprintf(fp, "\t\t andi  %s,%s,0x1f\n", regnameslong[T9], regnameslong[T9]);
								fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[T9], regnameslong[OPCODE], regnameslong[T9]);

								/* Check to see if Overflow should be set */

								fprintf(fp,"\t\t clz   %s,%s\n", regnameslong[T1], regnameslong[V0]);
								fprintf(fp,"\t\t clo   %s,%s\n", regnameslong[T2], regnameslong[V0]);
								if (Size != 'L') {
									fprintf(fp,"\t\t addiu %s,%s,%d\n", regnameslong[T1], regnameslong[T1], Size == 'W' ? -16 : -24);
									fprintf(fp,"\t\t addiu %s,%s,%d\n", regnameslong[T2], regnameslong[T2], Size == 'W' ? -16 : -24);
								}
								fprintf(fp,"\t\t sltu  %s,%s,%s\n", regnameslong[T1], regnameslong[T1], regnameslong[T9]);
								fprintf(fp,"\t\t sltu  %s,%s,%s\n", regnameslong[T2], regnameslong[T2], regnameslong[T9]);
								fprintf(fp,"\t\t or    %s,%s,%s        \t # Set Overflow\n", regnameslong[FLAG_V], regnameslong[T1], regnameslong[T2]);

								/* more than 31 shifts and long */

								if ((ir == 1) && (leng == 2)) {
									fprintf(fp, "\t\t andi  %s,%s,0x20\n", regnameslong[T1], regnameslong[OPCODE]);
									fprintf(fp, "\t\t movn  %s,$0,%s\n", regnameslong[V0], regnameslong[T1]);
								}

								fprintf(fp, "\t\t subu  %s,$0,%s\n", regnameslong[T1], regnameslong[OPCODE]);
								if (leng != 2)
									fprintf(fp, "\t\t andi  %s,%s,0x%2.2X\n", regnameslong[T1], regnameslong[T1], leng == 1 ? 15 : 7);
								fprintf(fp, "\t\t srlv   %s,%s,%s\n", regnameslong[FLAG_C], regnameslong[V0], regnameslong[T1]);
								fprintf(fp, "\t\t andi  %s,%s,0x01     \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[FLAG_C]);

								fprintf(fp, "\t\t sllv   %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[OPCODE]);

								/* Mode 0 write does not affect Flags */
								EffectiveAddressWrite(0, Size, T0, FALSE, flags, FALSE, FALSE);
								fprintf(fp, "\t\t or    %s,$0,%s       \t # Copy Carry to X\n", regnameslong[FLAG_X], regnameslong[FLAG_C]);
								fprintf(fp,"\t9:\n");
							}
							fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0]);
							fprintf(fp, "\t\t sltiu %s,%s,1         \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[V0]);
							Completed();
						}

						OpcodeArray[Opcode] = BaseCode ;
					}

	/* End with special routines for ASL.x #1,Dx  */
	/* To do correct V setting, ASL needs quite a */
	/* bit of additional code. A Shift of one is  */
	/* easy to set on MIPS, and is very common	  */
	/* in 68000 programs.						  */

	for (leng = 0; leng < 3; leng++)
		for (sreg = 0; sreg < 8; sreg++) {
			Opcode = 0xe300 | (leng << 6) | sreg;
			BaseCode = Opcode & 0xe3c8;

			switch (leng) {
				case 0:
					Size = 'b';
					break;
				case 1:
					Size = 'h';
					break;
				case 2:
					Size = 'w';
					break;
			}

			if (sreg == 0) {
				Align();
				fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

				if (Size != 'w')
					TimingCycles += 6;
				else
					TimingCycles += 8;

				fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
				fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[OPCODE], regnameslong[OPCODE]);
				fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);
				fprintf(fp, "\t\t l%c%c   %s,0x00(%s)\n", Size, Size == 'w' ? ' ' : 'u', regnameslong[V1], regnameslong[OPCODE]);
				fprintf(fp, "\n\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);
				fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[V0], regnameslong[V1]);
				fprintf(fp, "\t\t s%c    %s,0x00(%s)\n", Size, regnameslong[V0], regnameslong[OPCODE]);

				fprintf(fp, "\t\t srl   %s,%s,%d        \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[V1], Size == 'w' ? 31 : (Size == 'h' ? 15 : 7));
				fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[V0], regnameslong[V1]);
				fprintf(fp, "\t\t srl   %s,%s,%d", regnameslong[FLAG_V], regnameslong[FLAG_V], Size == 'w' ? 31 : (Size == 'h' ? 15 : 7));
				if (Size != 'w')
					fprintf(fp, "\n\t\t andi  %s,%s,0x01     ", regnameslong[FLAG_V], regnameslong[FLAG_V]);
				else
					fprintf(fp, "       ");
				fprintf(fp, "\t # Set Overflow\n");
				if (Size == 'w')
					fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[FLAG_Z], regnameslong[V0]);
				else
					fprintf(fp, "\t\t andi  %s,%s,0x%2.2X\n", regnameslong[FLAG_Z], regnameslong[V0], Size == 'h' ? 0xffff : 0xff);
				fprintf(fp, "\t\t sltiu %s,%s,1        \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[FLAG_Z]);
				fprintf(fp, "\t\t srl   %s,%s,%d        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0], Size == 'w' ? 31 : (Size == 'h' ? 15 : 7));
				fprintf(fp, "\t\t or    %s,$0,%s       \t # Copy Carry to X\n", regnameslong[FLAG_X], regnameslong[FLAG_C]);

				Completed();

			}

			OpcodeArray[Opcode] = BaseCode ;
		}
}

void asl_asr_ea(void)
{
	int Opcode, BaseCode;
	int dr, mode, sreg;
	int Dest;
	char allow[] = "--2345678-------";
	char flags[] = "---------------------SSS----G-F-";

	for (dr = 0; dr < 2; dr++)
		for (mode = 0; mode < 8; mode++)
			for (sreg = 0; sreg < 8; sreg++) {
				Opcode = 0xe0c0 | (dr << 8) | (mode << 3) | sreg;
				BaseCode = Opcode & 0xfff8;

				if (mode == 7)
					BaseCode |= sreg;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest & 0xf] != '-') {
					if (OpcodeArray[BaseCode] == -2) {

						Align();
						fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

						if ((Dest >= 2) && (Dest <= 10))
							SavePreviousPC();

						fprintf(fp, "\n\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);

						TimingCycles += 8 ;

						if ((Dest & 0xf) < 7)
							fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[OPCODE], regnameslong[OPCODE]);
						flags[EFFADDR] = 'T';
						EffectiveAddressRead(Dest & 0xf, 'W', OPCODE, dr == 0 ? V0 : V1, flags, dr == 0, TRUE, TRUE);
						flags[EFFADDR] = '-';

						if (dr == 0) {
							fprintf(fp, "\t\t andi  %s,%s,0x01      \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[V0]);
							fprintf(fp, "\t\t and   %s,$0,$0        \t # Clear Overflow\n", regnameslong[FLAG_V]);
							fprintf(fp, "\t\t sra   %s,%s,1\n", regnameslong[V0], regnameslong[V0]);
							fprintf(fp, "\t\t slt   %s,%s,$0        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0]);
						} else {
							fprintf(fp, "\t\t sll   %s,%s,1\n", regnameslong[V0], regnameslong[V1]);

							fprintf(fp, "\t\t srl   %s,%s,15        \t # Set Carry\n", regnameslong[FLAG_C], regnameslong[V1]);
							fprintf(fp, "\t\t xor   %s,%s,%s\n", regnameslong[FLAG_V], regnameslong[V0], regnameslong[V1]);
							fprintf(fp, "\t\t srl   %s,%s,15\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
							fprintf(fp, "\t\t andi  %s,%s,0x01     \t # Set Overflow\n", regnameslong[FLAG_V], regnameslong[FLAG_V]);
							fprintf(fp, "\t\t andi  %s,%s,0xffff\n", regnameslong[V0], regnameslong[V0]);
							fprintf(fp, "\t\t srl   %s,%s,15        \t # Set Sign\n", regnameslong[FLAG_N], regnameslong[V0]);
						}

						fprintf(fp, "\t\t sltiu %s,%s,1        \t # Set Zero\n", regnameslong[FLAG_Z], regnameslong[V0]);
						fprintf(fp, "\t\t or    %s,$0,%s       \t # Copy Carry to X\n", regnameslong[FLAG_X], regnameslong[FLAG_C]);

						flags[FLAG_C] = 'S';
						flags[FLAG_V] = 'S';
						flags[FLAG_Z] = 'S';
						flags[FLAG_N] = 'S';
						flags[FLAG_X] = 'S';
						EffectiveAddressWrite(Dest & 0xf, 'W', OPCODE, FALSE, flags, FALSE, FALSE);
						flags[FLAG_C] = '-';
						flags[FLAG_V] = '-';
						flags[FLAG_Z] = '-';
						flags[FLAG_N] = '-';
						flags[FLAG_X] = '-';
						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

/*
 * Divide Commands
 */

void divides(void)
{
	int dreg, type, mode, sreg;
	int Opcode, BaseCode;
	int Dest;
	char allow[] = "0-23456789ab-----";
	char DelaySlot[32];
	int Cycles;
	char flags[] = "--------------------SSSS----G-F-";

	int divide_cycles[12] = {
		38, 0, 42, 42, 44, 46, 50, 46, 50, 46, 48, 42
	};

	for (dreg = 0; dreg < 8; dreg++)
		for (type = 0; type < 2; type++)
			for (mode = 0; mode < 8; mode++)
				for (sreg = 0; sreg < 8; sreg++) {
					Opcode = 0x80c0 | (dreg << 9) | (type << 8) | (mode << 3) | sreg;
					BaseCode = Opcode & 0x81f8;
					if (mode == 7) {
						BaseCode |= sreg ;
					}

					Dest = EAtoAMN(Opcode, FALSE);
					if (allow[Dest & 0x0f] != '-') {
						if (OpcodeArray[ BaseCode ] == -2) {
							Align();
							fprintf(fp, "%s:\n", GenerateLabel(BaseCode, 0));

							if ((Dest >= 2) && (Dest <= 10))
								SavePreviousPC();

							fprintf(fp, "\n\t\t addiu %s,%s,2\n\n", regnameslong[PC], regnameslong[PC]);


							/* Cycle Timing (if succeeds OK) */

							Cycles = divide_cycles[Dest & 0x0f] + 95 + (type * 17);

							if (Cycles > 32767) {
								fprintf(fp, "\t\t ori   %s,$0,%d\n", regnameslong[T9], Cycles);
								fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[ICNT], regnameslong[ICNT], regnameslong[T9]);
							} else {
								fprintf(fp, "\t\t addiu %s,%s,%d\n", regnameslong[ICNT], regnameslong[ICNT], -Cycles);
							}

							if (mode < 7) {
								fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[OPCODE]);
							}

							fprintf(fp, "\t\t srl   %s,%s,7\n", regnameslong[OPCODE], regnameslong[OPCODE]);
							fprintf(fp, "\t\t andi  %s,%s,0x1C\n", regnameslong[OPCODE], regnameslong[OPCODE]);

							flags[OPCODE] = 'T';
							flags[EFFADDR] = 'T';
							EffectiveAddressRead(Dest, 'W', T0, V0, flags, type == 1 /* signed */, TRUE, TRUE); /* source */

							fprintf(fp, "\t\t beq   %s,$0,9f         \t # do div by zero trap\n", regnameslong[V0]);
							fprintf(fp, "\t\t nop                    \t # Delay slot\n");

							EffectiveAddressRead(0, 'L', OPCODE, T0, flags, TRUE, FALSE, TRUE); /* dest */
							flags[OPCODE] = '-';
							flags[EFFADDR] = '-';

							if (type == 1) { /* signed */
								fprintf(fp, "\t\t div   $0,%s,%s\n", regnameslong[T0], regnameslong[V0]);

								/* Check for Overflow */

								fprintf(fp, "\t\t mflo  %s\n", regnameslong[T1]);
								fprintf(fp, "\t\t sll   %s,%s,16\n", regnameslong[T0], regnameslong[T1]);
								fprintf(fp, "\t\t sra   %s,%s,16\n", regnameslong[T0], regnameslong[T0]);
								fprintf(fp, "\t\t bne   %s,%s,8f\n", regnameslong[T0], regnameslong[T1]);
								fprintf(fp, "\t\t andi  %s,%s,0xffff     \t # Delay slot\n", regnameslong[T1], regnameslong[T1]);
							} else {
								fprintf(fp, "\t\t divu  $0,%s,%s\n", regnameslong[T0], regnameslong[V0]);

								/* Check for Overflow */

								fprintf(fp, "\t\t mflo  %s\n", regnameslong[T1]);
								fprintf(fp, "\t\t srl   %s,%s,16\n", regnameslong[T0], regnameslong[T1]);
								fprintf(fp, "\t\t bne   %s,$0,8f\n", regnameslong[T0]);
							}

							/* Sort out Result */

							fprintf(fp, "\t\t mfhi  %s%s\n", regnameslong[V1], type == 1 ? "" : "               \t # Delay slot");
							fprintf(fp, "\t\t sll   %s,%s,16\n", regnameslong[V1], regnameslong[V1]);
							fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V0], regnameslong[T1], regnameslong[V1]);
							fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);

							SetFlags('W', FLAGS_MOVE, T1, 0, 0);

							fprintf(fp, "\t7:\n");
							Completed();


							/* Overflow */

							Align();
							fprintf(fp, "\t8:\n");
							fprintf(fp, "\t\t bgez  $0,7b\n");
							fprintf(fp, "\t\t ori   %s,$0,1         \t # Selay slot - V flag\n", regnameslong[FLAG_V]);
							//Completed();


							/* Division by Zero */

							Align();
							fprintf(fp, "\t9:\t\t # Do divide by zero trap\n");

							/* Correct cycle counter for error */

							fprintf(fp, "\t\t addiu %s,%s,%d\n", regnameslong[ICNT], regnameslong[ICNT], 95 + (type * 17));
							sprintf(DelaySlot, "ori   %s,$0,5          ", regnameslong[V0]);
							Exception(-1, DelaySlot);
							fprintf(fp, "\t\t bgez  $0,7b\n");
							fprintf(fp, "\t\t nop                    \t # Delay slot\n");
							//Completed();
						}

						OpcodeArray[Opcode] = BaseCode ;
					}
				}
}


/*
 * 68010 Extra Opcodes
 *
 * move from CCR is done above
 *
 */

void ReturnandDeallocate(void)
{
	int	BaseCode = 0x4e74 ;

	if (OpcodeArray[BaseCode] == -2) {
		Align();
		fprintf(fp, "%s:\n",GenerateLabel(BaseCode, 0));

		CheckCPUtype(1, NULL, NULL);

		SavePreviousPC();

		TimingCycles += 16;

		OpcodeArray[BaseCode] = BaseCode;

		/* Get Return Address */

		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[FASTCALL_FIRST_REG], REG_A7);
		Memory_Read('L', FASTCALL_FIRST_REG, "----------------SSSSSSS-----G-F-", LocalStack);


		/* Get Displacement */

		Memory_Fetch('W' ,T0, TRUE);


		/* Set PC = New Address */

		//fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[PC], regnameslong[V0]);


		/* Correct Stack for Return Address and Displacement */

		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T1], REG_A7);
		fprintf(fp, "\t\t addiu %s,%s,4\n", regnameslong[T0], regnameslong[T0]);
		fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T1], regnameslong[T0], regnameslong[T1]);
		fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[T1], REG_A7);

		MemoryBanking(V0, LocalStack);
		Completed();
	}
}

void MoveControlRegister(void)
{
	int  Direction;
	int  BaseCode = 0x4e7a;

	for (Direction = 0; Direction < 2; Direction++) {
		Align();
		fprintf(fp, "%s:\n", GenerateLabel(BaseCode + Direction, 0));

		TimingCycles += 4; /* Assume same as move usp */

		CheckCPUtype(1, NULL, NULL);

		fprintf(fp, "\t\t lbu   %s,%s\n", regnameslong[T0], REG_SRH);
		fprintf(fp, "\t\t andi  %s,%s,0x20       \t # Supervisor Mode ?\n", regnameslong[T0], regnameslong[T0]);
		fprintf(fp, "\t\t beq   %s,$0,9f\n", regnameslong[T0]);
		fprintf(fp, "\t\t addiu %s,%s,2        \t # Delay slot\n", regnameslong[PC], regnameslong[PC]);

		fprintf(fp, "\t\t lhu   %s,0x00(%s)\n", regnameslong[T0], regnameslong[PC]);
		fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[PC], regnameslong[PC]);

		/* Sort out Control Register ID */

		fprintf(fp, "\t\t andi  %s,%s,0x01\n", regnameslong[V0], regnameslong[T0]);
		fprintf(fp, "\t\t srl   %s,%s,10\n", regnameslong[OPCODE], regnameslong[T0]);
		fprintf(fp, "\t\t andi  %s,%s,0x02\n", regnameslong[OPCODE], regnameslong[OPCODE]);
		fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[V0]);

		/* Sort out Register */

		fprintf(fp, "\t\t srl   %s,%s,12\n", regnameslong[T0], regnameslong[T0]);

		if (Direction == 0) {
			/* from Control */

			fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[OPCODE], regnameslong[OPCODE]);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);
			fprintf(fp, "\t\t addiu %s,%s,%d\n", regnameslong[OPCODE], regnameslong[OPCODE], REG_SFC_BASE);
			fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
			fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[T0], regnameslong[T0]);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T0], regnameslong[T0], regnameslong[MEMBASE]);
			fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[T0]);
		} else {
			/* To Control */

			fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[T0], regnameslong[T0]);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T0], regnameslong[T0], regnameslong[MEMBASE]);
			fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[T0]);

			/* Mask out for SFC & DFC */

			fprintf(fp, "\t\t andi  %s,%s,0x02\n", regnameslong[T1], regnameslong[OPCODE]);
			fprintf(fp, "\t\t bne   %s,$0,8f\n", regnameslong[T1]);
			fprintf(fp, "\t\t nop                    \t # Delay slot\n");
			fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[V0], regnameslong[V0]);
			fprintf(fp, "\t8:\n");

			/* Write to control */

			fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[OPCODE], regnameslong[OPCODE]);
			fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[OPCODE], regnameslong[OPCODE], regnameslong[MEMBASE]);
			fprintf(fp, "\t\t addiu %s,%s,%d\n", regnameslong[OPCODE], regnameslong[OPCODE], REG_SFC_BASE);
			fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[V0], regnameslong[OPCODE]);
		}

		Completed();

		/* Not Supervisor Mode */

		Align();
		fprintf(fp, "\t9:\n");
		Exception(8, NULL);

		OpcodeArray[BaseCode + Direction] = BaseCode + Direction;
	}
}

void MoveAddressSpace(void)
{
}

/*
 * Generate Jump Table
 *
 */

void JumpTable(void)
{
	int Opcode, l, op;

#ifdef RLETABLE
	fprintf(fp, ".word OP%d_1000\n", CPU);

	l = 0;
	for (Opcode = 0x0; Opcode < 0x10000;) {

		op = OpcodeArray[Opcode];

		fprintf(fp, ".word ");

		l = 1;
		if (op > -1) {
			while (op == OpcodeArray[Opcode + l] && ((Opcode + l) & 0xfff) != 0) {
				l++;
			}
		} else {
			while (OpcodeArray[Opcode + l] < 0 && ((Opcode + l) & 0xfff) != 0) {
				l++;
			}
		}

		Opcode += l;

		if (l > 255) {
			if (op > -1)
				fprintf(fp, "OP%d_%4.4x - OP%d_1000\n", CPU, op, CPU);
			else
				fprintf(fp, "ILLEGAL - OP%d_1000\n", CPU);

			fprintf(fp, ".word %d\n", l);
		} else {
			if (op > -1)
				fprintf(fp, "(OP%d_%4.4x - OP%d_1000) + (%d * 1000000h)\n", CPU, op, CPU, l);
			else
				fprintf(fp, "(ILLEGAL - OP%d_1000) + (%d * 1000000h)\n", CPU, l);
		}
	}
#else
	l = 0;
	for (Opcode = 0x0; Opcode < 0x10000;) {

		op = OpcodeArray[Opcode];

		l = 1;
		if (op > -1) {
			while (op == OpcodeArray[Opcode + l] && ((Opcode + l) & 0xfff) != 0) {
				l++;
			}
		} else {
			while (OpcodeArray[Opcode + l] < 0 && ((Opcode + l) & 0xfff) != 0) {
				l++;
			}
		}

		Opcode += l;

		if (l > 1) {
			fprintf(fp, ".rept %d\n", l);
			if (op > -1) {
				fprintf(fp, "\t.word\t  OP%d_%4.4x\n", CPU, op);
			} else {
				fprintf(fp, "\t.word\t  ILLEGAL\n");
			}
			fprintf(fp, ".endr\n");
		} else {
			if (op > -1)
				fprintf(fp, ".word\t\t  OP%d_%4.4x\n", CPU, op);
			else
				fprintf(fp, ".word\t\t  ILLEGAL\n");
		}
	}
#endif
}

#ifdef TESTTABLE
void TestTable(void)
{
	int Opcode, l, op, line = 0;

	fprintf(fp, "unsigned short OpTestTbl[%d] = {\n", Opcount);

	for (Opcode = 0x0; Opcode < 0x10000; Opcode++) {

		op = OpcodeArray[Opcode];

		if (op < 0) continue;

		for (l = Opcode + 1; l < 0x10000; l++)
			if (OpcodeArray[l] == op) OpcodeArray[l] = -1;

		fprintf(fp, " 0x%04X,", op);
		if (++line == 16) {
			fprintf(fp, "\n");
			line = 0;
		}
	}

	if (line) fprintf(fp, "\n");

	fprintf(fp, "};\n\n");
}
#endif

void CodeSegmentBegin(void)
{
	int ExLocalStack;

/* Messages */

	fprintf(fp, " # MIPS Make68K - V%s - Copyright 2005, Manuel Geran (bdiamond@free.fr)\n", VERSION);
	fprintf(fp, " #   based on Make68K - Copyright 1998, Mike Coates (mame@btinternet.com)\n");
	fprintf(fp, " #                                    & Darren Olafson (deo@mail.island.net)\n");

/* Needed code to make it work! */

	fprintf(fp, "\t\t .set arch=allegrex\n\n");

	fprintf(fp, "\t\t .set noreorder\n");
	fprintf(fp, "\t\t .set nomacro\n");
	fprintf(fp, "\t\t .set nobopt\n");
	fprintf(fp, " #\t\t .set noat\n\n");

	fprintf(fp, "\t\t .globl %s_RUN\n", CPUtype);
	fprintf(fp, "\t\t .globl %s_RESET\n", CPUtype);
	fprintf(fp, "\t\t .globl %s_regs\n", CPUtype);
#ifdef RLETABLE
	fprintf(fp, "\t\t .globl %s_COMPTABLE\n", CPUtype);
#endif
	fprintf(fp, "\t\t .globl %s_OPCODETABLE\n", CPUtype);

	/* ASG - only one interface to memory now */
	fprintf(fp, "\t\t .globl %sa68k_memory_intf\n", PREF);

	fprintf(fp, "\n\n # Vars Mame declares / needs access to\n\n");

	fprintf(fp, "\t\t .extern %sm68k_ICount,4\n", PREF);
	fprintf(fp, "\t\t .extern %sOP_ROM,4\n", PREF);
	fprintf(fp, "\t\t .extern %sOP_RAM,4\n", PREF);

	fprintf(fp, "\t\t .extern %smem_amask,4\n", PREF);
	fprintf(fp, "\t\t .extern %smame_debug,4\n", PREF);
	fprintf(fp, "\t\t .extern %sillegal_op,4\n", PREF);
	fprintf(fp, "\t\t .extern %sillegal_pc,4\n", PREF);

	fprintf(fp, "\t\t .extern %sopcode_entry,4\n", PREF);
	fprintf(fp, "\t\t .extern %scur_mrhard,4\n", PREF);

#ifdef MAME_DEBUG
	fprintf(fp, "\t\t .extern %sm68k_illegal_opcode,4\n", PREF);
#endif

	fprintf(fp, "\n\n\n\t\t .text\n\n");


/* Reset routine */

	fprintf(fp, "\t .ent   %s_RESET\n", CPUtype);
	fprintf(fp, "%s_RESET:\n", CPUtype);
	fprintf(fp, "\t\t .frame %s, %d, %s\n", regnameslong[SP], 0, regnameslong[RA]);

#ifdef RLETABLE
	fprintf(fp, " # Build Jump Table (not optimised!)\n\n");

	fprintf(fp, "\t\t la    %s,%s_OPCODETABLE\t\t # Jump Table\n", regnameslong[T0], CPUtype);
	fprintf(fp, "\t\t la    %s,%s_COMPTABLE\t\t # RLE Compressed Table\n", regnameslong[T1], CPUtype);

	fprintf(fp, "\t\t lui   %s,0xff\n", regnameslong[V0]);
	fprintf(fp, "\t\t ori   %s,%s,0xffff\n", regnameslong[V0], regnameslong[V0]);

	/* Reference Point in T2 */

	fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[T2], regnameslong[T1]);
	fprintf(fp, "\t\t addiu %s,%s,4\n", regnameslong[T1], regnameslong[T1]);

	fprintf(fp, "RESET0:\n");
	fprintf(fp, "\t\t lw    %s,0x00(%s)\n", regnameslong[T3], regnameslong[T1]);
	fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[T4], regnameslong[T3]);
	fprintf(fp, "\t\t and   %s,%s,%s\n", regnameslong[T3], regnameslong[T3], regnameslong[V0]);
	fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[T3], regnameslong[T3], regnameslong[T2]);

	/* if count is zero, then it's a word RLE length */

	fprintf(fp, "\t\t srl   %s,%s,24\n", regnameslong[T4], regnameslong[T4]);
	fprintf(fp, "\t\t bne   %s,$0,RESET1\n", regnameslong[T4]);
	fprintf(fp, "\t\t addiu %s,%s,4\t\t # Delay slot\n", regnameslong[T1], regnameslong[T1]);

	fprintf(fp, "\t\t lw    %s,0x00(%s)\t\t # Repeats\n", regnameslong[T4], regnameslong[T4]);

	fprintf(fp, "\t\t beq	%s,$0,RESET2\t\t # Finished!\n", regnameslong[T4]);
	fprintf(fp, "\t\t addiu %s,%s,4\t\t # Delay slot\n", regnameslong[T1], regnameslong[T1]);

	fprintf(fp, "RESET1:\n");
	fprintf(fp, "\t\t sw    %s,0x00(%s)\n", regnameslong[T3], regnameslong[T0]);
	fprintf(fp, "\t\t addiu %s,%s,-1\n", regnameslong[T4], regnameslong[T4]);
	fprintf(fp, "\t\t bne   %s,$0,RESET1\n", regnameslong[T4]);
	fprintf(fp, "\t\t addiu %s,%s,4\t\t # Delay slot\n", regnameslong[T0], regnameslong[T0]);
	fprintf(fp, "\t\t bgez  $0,RESET0\n");
	fprintf(fp, "\t\t nop\t\t\t # Delay slot\n\n");

	fprintf(fp, "RESET2:\n");
#endif

	fprintf(fp, "\t\t jr    %s\n", regnameslong[RA]);
	fprintf(fp, "\t\t nop\t\t\t # Delay slot\n\n");
	fprintf(fp, "\t .end   %s_RESET\n\n\n", CPUtype);


/* Emulation Entry Point */

	Align();

	fprintf(fp, "\t .ent   %s_RUN\n", CPUtype);
	fprintf(fp, "%s_RUN:\n", CPUtype);

	fprintf(fp, "\t\t addiu %s,%s,%d\n", regnameslong[SP], regnameslong[SP], -STACKFRAME_SIZE);
	fprintf(fp, "\t\t .frame %s, %d, %s\n\n", regnameslong[SP], STACKFRAME_SIZE, regnameslong[RA]);
#if (MEMBASE == GP)
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[GP], STACKFRAME_SIZE - 0x2C, regnameslong[SP]);
#endif
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[S0], STACKFRAME_SIZE - 0x28, regnameslong[SP]);
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[S1], STACKFRAME_SIZE - 0x24, regnameslong[SP]);
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[S2], STACKFRAME_SIZE - 0x20, regnameslong[SP]);
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[S3], STACKFRAME_SIZE - 0x1C, regnameslong[SP]);
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[S4], STACKFRAME_SIZE - 0x18, regnameslong[SP]);
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[S5], STACKFRAME_SIZE - 0x14, regnameslong[SP]);
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[S6], STACKFRAME_SIZE - 0x10, regnameslong[SP]);
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[S7], STACKFRAME_SIZE - 0x0C, regnameslong[SP]);
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[S8], STACKFRAME_SIZE - 0x08, regnameslong[SP]);
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[RA], STACKFRAME_SIZE - 0x04, regnameslong[SP]);

	//fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[MEMBASE], regnameslong[A0]);
	fprintf(fp, "\t\t la    %s,%sM68000_regs\n", regnameslong[MEMBASE], PREF);
	fprintf(fp, "\t\t lbu   %s,%s\n", regnameslong[T0], REG_IRQ);
	fprintf(fp, "\t\t xori  %s,%s,0x80\n", regnameslong[T1], regnameslong[T0]);
	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[ICNT], ICOUNT);
	fprintf(fp, "\t\t beq   %s,$0,MC68Kexit\n", regnameslong[T1]);
	fprintf(fp, "\t\t movz  %s,$0,%s\t\t # Delay slot\n", regnameslong[ICNT], regnameslong[T1]);
	fprintf(fp, "\t\t la    %s,%sM68000_OPCODETABLE\n", regnameslong[OPCODETBL], PREF);
	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[BASEPC], OP_ROM);
	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[PC], REG_PC);
	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[V0], REG_CCR);
	fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[PC], regnameslong[BASEPC], regnameslong[PC]);

	WriteCCR('B', V0);

	fprintf(fp, " # Check for Interrupt waiting\n\n");
	//fprintf(fp, "\t\t lbu    %s,%s\n", regnameslong[T0], REG_IRQ);
	fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[T0], regnameslong[T0]);
	fprintf(fp, "\t\t bne   %s,$0,interrupt\n\n", regnameslong[T0]);
	fprintf(fp, "\t\t nop\t\t\t # Delay slot\n\n");

	fprintf(fp, "IntCont:\n");

	/* See if was only called to check for Interrupt */

	fprintf(fp, "\t\t bltz  %s,MainExit\n", regnameslong[ICNT]);

	/* 16 Bit Fetch */
	fprintf(fp, "\t\t lhu   %s,0x00(%s)\t\t # Delay slot\n\n", regnameslong[OPCODE], regnameslong[PC]);

	fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[TMPREG0], regnameslong[OPCODE]);
	fprintf(fp, "\t\t addu  %s,%s,%s\n\n", regnameslong[TMPREG0], regnameslong[TMPREG0], regnameslong[OPCODETBL]);
	fprintf(fp, "\t\t lw    %s,0x00(%s)\n\n", regnameslong[TMPREG0], regnameslong[TMPREG0]);
	fprintf(fp, "\t\t jr    %s\n", regnameslong[TMPREG0]);
	fprintf(fp,"\t\t nop\t\t\t # Delay slot\n\n");

	Align();

	fprintf(fp, "MainExit:\n");
	fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[PC], regnameslong[PC], regnameslong[BASEPC]);
	fprintf(fp, "\t\t sw    %s,%s\t\t # Save PC\n", regnameslong[PC], REG_PC);

	/* If in Debug mode make normal SR register */

#ifdef MAME_DEBUG

	ReadCCR('W', V0);

#else

	ReadCCR('B', V0);

#endif

	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T0], REG_SRH);
	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[V0], REG_CCR);
	fprintf(fp, "\t\t andi  %s,%s,0x20\n", regnameslong[T0], regnameslong[T0]);
	fprintf(fp, "\t\t bne   %s,$0,ME1\t\t # Mode ?\n", regnameslong[T0]);
	fprintf(fp, "\t\t lw    %s,%s\t\t # Delay slot - Get A7\n", regnameslong[V0], REG_A7);
	fprintf(fp, "\t\t bgez  $0,MC68Kexit\n");
	fprintf(fp, "\t\t sw    %s,%s\t\t # Delay slot - Save in USP\n", regnameslong[V0], REG_USP);
	fprintf(fp, "ME1:\n");
	fprintf(fp, "\t\t sw    %s,%s\t\t # Save in ISP\n", regnameslong[V0], REG_ISP);
	fprintf(fp, "MC68Kexit:\n");
	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[ICNT], ICOUNT);

#if (MEMBASE == GP)
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[GP], STACKFRAME_SIZE - 0x2C, regnameslong[SP]);
#endif
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[S0], STACKFRAME_SIZE - 0x28, regnameslong[SP]);
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[S1], STACKFRAME_SIZE - 0x24, regnameslong[SP]);
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[S2], STACKFRAME_SIZE - 0x20, regnameslong[SP]);
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[S3], STACKFRAME_SIZE - 0x1C, regnameslong[SP]);
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[S4], STACKFRAME_SIZE - 0x18, regnameslong[SP]);
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[S5], STACKFRAME_SIZE - 0x14, regnameslong[SP]);
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[S6], STACKFRAME_SIZE - 0x10, regnameslong[SP]);
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[S7], STACKFRAME_SIZE - 0x0C, regnameslong[SP]);
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[S8], STACKFRAME_SIZE - 0x08, regnameslong[SP]);
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[RA], STACKFRAME_SIZE - 0x04, regnameslong[SP]);

	fprintf(fp, "\t\t jr    %s\n", regnameslong[RA]);
	fprintf(fp, "\t\t addiu %s,%s,%d\t\t # Delay slot\n", regnameslong[SP], regnameslong[SP], STACKFRAME_SIZE);
	fprintf(fp, "\t .end   %s_RUN\n\n\n", CPUtype);

/* Check for Pending Interrupts */

	Align();
	fprintf(fp, " # Interrupt check\n\n");

	fprintf(fp, "interrupt:\n");

	/* check to exclude interrupts */

	fprintf(fp, "\t\t lbu   %s,%s\n", regnameslong[T1], REG_IRQ);
	fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[V0], regnameslong[T1]);

	fprintf(fp, "\t\t xori  %s,%s,0x07\t\t # Always take 7\n", regnameslong[T0], regnameslong[V0]);
	fprintf(fp, "\t\t beq   %s,$0,procint\n", regnameslong[T0]);
	fprintf(fp, "\t\t nop\t\t\t # Delay slot\n\n");

	fprintf(fp, "\t\t lbu   %s,%s\t\t # int mask\n", regnameslong[V1], REG_SRH);
	fprintf(fp, "\t\t andi  %s,%s,0x07\n", regnameslong[V1], regnameslong[V1]);
	fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[T0], regnameslong[V0], regnameslong[V1]);
	fprintf(fp, "\t\t blez  %s,IntCont\n", regnameslong[T0]);
	fprintf(fp, "\t\t nop\t\t\t # Delay slot\n\n");

	/* Take pending Interrupt */

	Align();
	fprintf(fp, "procint:\n");

	/* LVR - Enable proper IRQ handling (require explicit acknowledge) */
	/* fprintf(fp, "\t\t andi  %s,%s,0x78\t\t # remove interrupt & stop\n\n", regnameslong[T1], regnameslong[T1]); */
	fprintf(fp, "\t\t andi  %s,%s,0x7f\t\t # remove stop\n", regnameslong[T1], regnameslong[T1]);
	fprintf(fp, "\t\t sb    %s,%s\n\n", regnameslong[T1], REG_IRQ);

	/* Get Interrupt Vector from callback */

	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], REG_IRQ_CALLBACK);
	LocalStack -= 4;
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\t\t # save level\n\n", regnameslong[V0], LocalStack, regnameslong[SP]);

	if (SavedRegs[V1] == '-') {
		LocalStack -= 4;
		fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[V0], LocalStack, regnameslong[SP]);
	} else {
		fprintf(fp, "\t\t or    %s,$0,%s\n", regnameslong[V1], regnameslong[V0]);
	}

	if (SavedRegs[PC] == '-') {
		fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[PC], REG_PC);
	}

	if (SavedRegs[FLAG_C] == '-') {
		LocalStack -= 4;
		fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_C], LocalStack, regnameslong[SP]);
	}
	if (SavedRegs[FLAG_V] == '-') {
		LocalStack -= 4;
		fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_V], LocalStack, regnameslong[SP]);
	}
	if (SavedRegs[FLAG_Z] == '-') {
		LocalStack -= 4;
		fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_Z], LocalStack, regnameslong[SP]);
	}
	if (SavedRegs[FLAG_N] == '-') {
		LocalStack -= 4;
		fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_N], LocalStack, regnameslong[SP]);
	}
	if (SavedRegs[FLAG_X] == '-') {
		LocalStack -= 4;
		fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_X], LocalStack, regnameslong[SP]);
	}

	assert(LocalStack >= MINSTACK_POS);

	fprintf(fp, "\t\t sw    %s,%s\n", regnameslong[ICNT], ICOUNT);
	fprintf(fp, "\t\t jalr  %s\t # get the IRQ level\n", regnameslong[T9]);
	fprintf(fp, "\t\t or    %s,$0,%s\t\t # irq line #\t\t    Delay slot\n", regnameslong[FASTCALL_FIRST_REG], regnameslong[V0]);

	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[ICNT], ICOUNT);

	if (SavedRegs[FLAG_X] == '-') {
		fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_X], LocalStack, regnameslong[SP]);
		LocalStack += 4;
	}
	if (SavedRegs[FLAG_N] == '-') {
		fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_N], LocalStack, regnameslong[SP]);
		LocalStack += 4;
	}
	if (SavedRegs[FLAG_Z] == '-') {
		fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_Z], LocalStack, regnameslong[SP]);
		LocalStack += 4;
	}
	if (SavedRegs[FLAG_V] == '-') {
		fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_V], LocalStack, regnameslong[SP]);
		LocalStack += 4;
	}
	if (SavedRegs[FLAG_C] == '-') {
		fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_C], LocalStack, regnameslong[SP]);
		LocalStack += 4;
	}

	if (SavedRegs[PC] == '-') {
		fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[PC], REG_PC);
	}

	/* Do we want to use normal vector number ? */

	fprintf(fp, "\t\t bgez  %s,AUTOVECTOR\n", regnameslong[V0]);

	/* Only need V0 restored if default vector to be used */

	if (SavedRegs[V1] == '-') {
		fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\t\t # Delay slot\n", regnameslong[V1], LocalStack, regnameslong[SP]);
		LocalStack += 4;
	} else {
		fprintf(fp, "\t\t nop\t\t\t # Delay slot\n");
	}

		/* Just get default vector */

	fprintf(fp, "\t\t addiu %s,%s,24\t\t # Vector\n\n", regnameslong[V0], regnameslong[V1]);

	fprintf(fp, "AUTOVECTOR:\n\n");

	Exception(-1, NULL);

	fprintf(fp, "\t\t lbu   %s,%s\t\t # set Int mask\n", regnameslong[V1], REG_SRH);
	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[V0], LocalStack, regnameslong[SP]);
	LocalStack += 4;
	fprintf(fp, "\t\t andi  %s,%s,0xF8\n", regnameslong[V1], regnameslong[V1]);
	fprintf(fp, "\t\t or    %s,%s,%s\n", regnameslong[V1], regnameslong[V1], regnameslong[V0]);
	fprintf(fp, "\t\t bgez  $0,IntCont\n");
	fprintf(fp, "\t\t sb    %s,%s\t\t # Delay slot\n\n", regnameslong[V1], REG_SRH);

/* space 1 saved register and 7 work registers : Stack must be kept on 16 bytes boundaries */
#define EX_STACKFRAME_SIZE		(3 * (4 * 4))
#define EX_LOCALVARBASE			(EX_STACKFRAME_SIZE - 0x04)

	ExLocalStack = EX_LOCALVARBASE;

/* Exception Routine */

	Align();
	fprintf(fp, "\t .ent   Exception\n");
	fprintf(fp, "Exception:\n");
	fprintf(fp, "\t\t addiu %s,%s,%d\n", regnameslong[SP], regnameslong[SP], -EX_STACKFRAME_SIZE);
	fprintf(fp, "\t\t .frame %s, %d, %s\n\n", regnameslong[SP], EX_STACKFRAME_SIZE, regnameslong[RA]);
	fprintf(fp, "\t\t sw    %s,0x%2.2X(%s)\n", regnameslong[RA], EX_STACKFRAME_SIZE - 0x04, regnameslong[SP]);

	if (SavedRegs[FLAG_C] == '-') {
		ExLocalStack -= 4;
		fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_C], ExLocalStack, regnameslong[SP]);
	}
	if (SavedRegs[FLAG_V] == '-') {
		ExLocalStack -= 4;
		fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_V], ExLocalStack, regnameslong[SP]);
	}
	if (SavedRegs[FLAG_Z] == '-') {
		ExLocalStack -= 4;
		fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_Z], ExLocalStack, regnameslong[SP]);
	}
	if (SavedRegs[FLAG_N] == '-') {
		ExLocalStack -= 4;
		fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_N], ExLocalStack, regnameslong[SP]);
	}
	if (SavedRegs[FLAG_X] == '-') {
		ExLocalStack -= 4;
		fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\n", regnameslong[FLAG_X], ExLocalStack, regnameslong[SP]);
	}

	//fprintf(fp, "\t\t andi  %s,%s,0xFF\t\t # Zero Extend IRQ Vector\n", regnameslong[V0], regnameslong[V0]);

	ExLocalStack -= 4;
	fprintf(fp, "\t\t sb    %s,0x%2.2X(%s)\t\t # Save IRQ Vector for Later\n", regnameslong[V0], ExLocalStack, regnameslong[SP]);
	assert(ExLocalStack >= MINSTACK_POS);

	/*  Update Cycle Count */

	fprintf(fp, "\t\t la    %s,exception_cycles\n", regnameslong[T0]);
	fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[V0], regnameslong[V0], regnameslong[T0]);
	fprintf(fp, "\t\t lbu   %s,0x00(%s)\t\t # Get Cycles\n", regnameslong[V0], regnameslong[V0]);
	fprintf(fp, "\t\t subu  %s,%s,%s\t\t # Decrement ICount\n", regnameslong[ICNT], regnameslong[ICNT], regnameslong[V0]);

	ReadCCR('W', V0);

	fprintf(fp, "\t\t andi  %s,%s,0x2000\t\t\t # Which Mode ?\n", regnameslong[T9], regnameslong[V0]);
	fprintf(fp, "\t\t bne   %s,$0,ExSuperMode\t\t # Supervisor\n", regnameslong[T9]);
	fprintf(fp, "\t\t lw    %s,%s\t\t\t # Delay slot - Get A7\n", regnameslong[FASTCALL_FIRST_REG], REG_A7);

	fprintf(fp, "\t\t srl   %s,%s,8\n", regnameslong[T9], regnameslong[V0]);
	fprintf(fp, "\t\t ori   %s,%s,0x20\n", regnameslong[T9], regnameslong[T9]);
	fprintf(fp, "\t\t sb    %s,%s\t\t\t # Set Supervisor Mode\n", regnameslong[T9], REG_SRH);
	fprintf(fp, "\t\t sw    %s,%s\t\t\t # Save in USP\n", regnameslong[FASTCALL_FIRST_REG], REG_USP);
	fprintf(fp, "\t\t lw    %s,%s\t\t\t # Get ISP\n", regnameslong[FASTCALL_FIRST_REG], REG_ISP);

	/* Write SR first (since it's in a register) */

	fprintf(fp, "ExSuperMode:\n");
	fprintf(fp, "\t\t addiu %s,%s,-6\n", regnameslong[FASTCALL_FIRST_REG], regnameslong[FASTCALL_FIRST_REG]);
	fprintf(fp, "\t\t sw    %s,%s\t\t\t # Put in A7\n", regnameslong[FASTCALL_FIRST_REG], REG_A7);
	Memory_Write('W', FASTCALL_FIRST_REG, V0, "----A----------------SSS----G-F-", ExLocalStack);

	/* Then write PC */

	fprintf(fp, "\t\t subu  %s,%s,%s\n", regnameslong[FASTCALL_SECOND_REG], regnameslong[PC], regnameslong[BASEPC]);
	fprintf(fp, "\t\t addiu %s,%s,2\n", regnameslong[FASTCALL_FIRST_REG], regnameslong[FASTCALL_FIRST_REG]);
	Memory_Write('L', FASTCALL_FIRST_REG, FASTCALL_SECOND_REG, "---------------------SS-----G-F-", ExLocalStack);

	/* Get new PC */

	fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\t\t # Level\n", regnameslong[V0], ExLocalStack, regnameslong[SP]);
	ExLocalStack += 4;
	fprintf(fp, "\t\t lw    %s,%s\n", regnameslong[T9], REG_VBR);	  /* 68010+ Vector Base */
	fprintf(fp, "\t\t sll   %s,%s,2\n", regnameslong[V0], regnameslong[V0]);
	fprintf(fp, "\t\t addu  %s,%s,%s\n", regnameslong[FASTCALL_FIRST_REG], regnameslong[V0], regnameslong[T9]);

	/* Direct Read */

	Memory_Read('L', FASTCALL_FIRST_REG, "---------------------SS-----G-F-", ExLocalStack);

	//fprintf(fp, "\t\t or    %s,$0,%s\t\t # Set PC\n", regnameslong[PC], regnameslong[V0]);

	if (SavedRegs[FLAG_X] == '-') {
		fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_X], ExLocalStack, regnameslong[SP]);
		ExLocalStack += 4;
	}
	if (SavedRegs[FLAG_N] == '-') {
		fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_N], ExLocalStack, regnameslong[SP]);
		ExLocalStack += 4;
	}
	if (SavedRegs[FLAG_Z] == '-') {
		fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_Z], ExLocalStack, regnameslong[SP]);
		ExLocalStack += 4;
	}
	if (SavedRegs[FLAG_V] == '-') {
		fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_V], ExLocalStack, regnameslong[SP]);
		ExLocalStack += 4;
	}
	if (SavedRegs[FLAG_C] == '-') {
		fprintf(fp, "\t\t lbu   %s,0x%2.2X(%s)\n", regnameslong[FLAG_C], ExLocalStack, regnameslong[SP]);
		ExLocalStack += 4;
	}

	/* Sort out any bank changes */
	MemoryBanking(V0, ExLocalStack);

	fprintf(fp, "\t\t lw    %s,0x%2.2X(%s)\n", regnameslong[RA], EX_STACKFRAME_SIZE - 0x04, regnameslong[SP]);
	fprintf(fp, "\t\t jr    %s\n", regnameslong[RA]);
	fprintf(fp, "\t\t addiu %s,%s,%d\t\t # Delay slot\n\n", regnameslong[SP], regnameslong[SP], EX_STACKFRAME_SIZE);
	fprintf(fp, "\t .end   Exception\n");
	fprintf(fp, "\n\n\n\n");

}

void CodeSegmentEnd(void)
{
	fprintf(fp, "\t\t .data\n");

	/* Memory structure for 68000 registers  */
	/* Aligned on a cache line				 */

	fprintf(fp, "\n\t\t .align 6\n");

	/* Memory structure for 68000 registers  */
	/* Same layout as structure in CPUDEFS.H */

	fprintf(fp, "\n\n # Register Structure\n\n");
	fprintf(fp, "%s_regs:\n", CPUtype);

	fprintf(fp, "R_D0:\t .word 0\t\t\t # Data Registers\n");
	fprintf(fp, "R_D1:\t .word 0\n");
	fprintf(fp, "R_D2:\t .word 0\n");
	fprintf(fp, "R_D3:\t .word 0\n");
	fprintf(fp, "R_D4:\t .word 0\n");
	fprintf(fp, "R_D5:\t .word 0\n");
	fprintf(fp, "R_D6:\t .word 0\n");
	fprintf(fp, "R_D7:\t .word 0\n\n");

	fprintf(fp, "R_A0:\t .word 0\t\t\t # Address Registers\n");
	fprintf(fp, "R_A1:\t .word 0\n");
	fprintf(fp, "R_A2:\t .word 0\n");
	fprintf(fp, "R_A3:\t .word 0\n");
	fprintf(fp, "R_A4:\t .word 0\n");
	fprintf(fp, "R_A5:\t .word 0\n");
	fprintf(fp, "R_A6:\t .word 0\n");
	fprintf(fp, "R_A7:\t .word 0\n\n");

	fprintf(fp, "R_ISP:\t .word 0\t\t\t # Supervisor Stack\n");
	fprintf(fp, "R_SR_H:\t .word 0\t\t\t # Status Register High TuSuuIII\n");
	fprintf(fp, "R_SR:\t .word 0\t\t\t # Motorola Format SR\n\n");

	fprintf(fp, "R_PC:\t .word 0\t\t\t # Program Counter\n");
	fprintf(fp, "R_IRQ:\t .word 0\t\t\t # IRQ Request Level\n\n");

	fprintf(fp, "R_IRQ_CALLBACK:\t .word 0\t\t\t # irq callback (get vector)\n\n");

	fprintf(fp, "R_PPC:\t .word 0\t\t\t # Previous Program Counter\n");

	fprintf(fp, "R_RESET_CALLBACK:\t .word 0\t\t\t # Reset Callback\n");

	fprintf(fp, "R_SFC:\t .word 0\t\t\t # Source Function Call\n");
	fprintf(fp, "R_DFC:\t .word 0\t\t\t # Destination Function Call\n");
	fprintf(fp, "R_USP:\t .word 0\t\t\t # User Stack\n");
	fprintf(fp, "R_VBR:\t .word 0\t\t\t # Vector Base\n");

	fprintf(fp, "CPUversion:\t .word 0\n\n");
	fprintf(fp, "FullPC:\t .word 0\n\n");
	fprintf(fp, "%sa68k_memory_intf:\n", PREF);
	fprintf(fp, "\t.rept 13\n");
	fprintf(fp, "\t\t.word 0\n");
	fprintf(fp, "\t.endr\n");

	/* Safe Memory Locations */

	/* Exception Timing Table */

	fprintf(fp, "\n\nexception_cycles:\n");
	fprintf(fp, "\t\t .byte 0, 0, 0, 0, 38, 42, 44, 38, 38, 0, 38, 38, 0, 0, 0, 0\n");
	fprintf(fp, "\t\t .byte 0, 0, 0, 0, 0, 0, 0, 0, 46, 46, 46, 46, 46, 46, 46, 46\n");
	fprintf(fp, "\t\t .byte 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38\n\n");

	fprintf(fp, "\t\t .align 2\n");

#ifdef RLETABLE
	fprintf(fp, " # RLE Compressed Jump Table\n\n");
	fprintf(fp, "%s_COMPTABLE:\n\n", CPUtype);
#else
	fprintf(fp, " # Jump Table\n\n");
	fprintf(fp, "%s_OPCODETABLE:\n", CPUtype);
#endif

	fprintf(fp, "%cinclude \"%s\"\n\n", '.', comptab);

#ifdef RLETABLE
	fprintf(fp, "\t\t.word   0,0,0,0\n");
	fprintf(fp, "\n\n\n\n");


	fprintf(fp, "\t\t SECTION .bss\n");

	fprintf(fp, "\n\t\t .align 4\n\n");

	fprintf(fp, "%s_OPCODETABLE:\n", CPUtype);
	fprintf(fp, "\t.rept 65536\n");
	fprintf(fp, "\t\t.word 0\n");
	fprintf(fp, "\t.endr\n");
#endif
}

void EmitCode(void)
{
	CodeSegmentBegin();

	/* Instructions */

	moveinstructions();				  /* 1000 to 3FFF MOVE.X */
	immediate();						/* 0### XXX.I */
	bitdynamic();							  /* 0### dynamic bit operations */
	movep();									 /* 0### Move Peripheral */
	bitstatic();						/* 08## static bit operations */
	LoadEffectiveAddress();			 /* 4### */
	PushEffectiveAddress();			 /* ???? */
	movesr();							/* 4#C# */
	opcode5();								  /* 5000 to 5FFF ADDQ,SUBQ,Scc and DBcc */
	branchinstructions();				/* 6000 to 6FFF Bcc,BSR */
	moveq();						 /* 7000 to 7FFF MOVEQ */
	abcd_sbcd();						/* 8### Decimal Add/Sub */
	typelogicalmath();						/* Various ranges */
	addx_subx();
	divides();
	swap();
	not();								/* also neg negx clr */
	moveusp();
	chk();
	exg();
	cmpm();
	mul();
	ReturnandRestore();
	rts();
	jmp_jsr();
	nbcd();
	tas();
	trap();
	trapv();
	reset();
	nop();
	stop();
	ext();
	ReturnFromException();
	tst();
	movem_reg_ea();
	movem_ea_reg();
	link();
	unlinkasm();
	asl_asr();						  /* E### Shift Commands */
	asl_asr_ea();
	roxl_roxr();
	roxl_roxr_ea();
	lsl_lsr();
	lsl_lsr_ea();
	rol_ror();
	rol_ror_ea();
	LineA();						 /* A000 to AFFF Line A */
	LineF();							/* F000 to FFFF Line F */
	illegal_opcode();

	ReturnandDeallocate();			  /* 68010 Commands */
	MoveControlRegister();
	MoveAddressSpace();

	CodeSegmentEnd();
}

int main(int argc, char **argv)
{
	int dwLoop;

	printf("\nMIPS Make68K - V%s - Copyright 2005, Manuel Geran (bdiamond@free.fr)\n", VERSION);
	printf("  based on Make68K - Copyright 1998, Mike Coates (mame@btinternet.com)\n");
	printf("                               1999, & Darren Olafson (deo@mail.island.net)\n");
	printf("                               2000\n");

	if (argc != 3) {
		printf("Usage: %s outfile jumptable-outfile\n", argv[0]);
		exit(1);
	}

	printf("Building 68000/68010           2001\n\n");

	for (dwLoop = 0; dwLoop < 65536;)  OpcodeArray[dwLoop++] = -2;

	codebuf = malloc(64);
	if (!codebuf) {
		printf ("Memory allocation error\n");
		exit(3);
	}

	/* Emit the code */
	fp = fopen(argv[1], "w");
	if (!fp) {
		fprintf(stderr, "Can't open %s for writing\n", argv[1]);
		exit(1);
	}

	comptab = argv[2];


	CPUtype = malloc(64);

	sprintf(CPUtype, "%sM68000", PREF);

	EmitCode();

	fclose(fp);

	printf("\n%d Unique Opcodes\n", Opcount);

	/* output Jump table to separate file */
	fp = fopen(argv[2], "w");
	if (!fp) {
		fprintf(stderr, "Can't open %s for writing\n", argv[2]);
		exit(1);
	}

	JumpTable();

	fclose(fp);

#ifdef TESTTABLE
	fp = fopen("opcodes.h", "w");
	if (!fp) {
		fprintf(stderr, "Can't open %s for writing\n", argv[2]);
		exit(1);
	}

	TestTable();

	fclose(fp);
#endif

	exit(0);
}
