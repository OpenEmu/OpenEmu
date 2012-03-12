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

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3
#define ESI 4
#define PC  ESI
#define EDI 5
#define EBP 6


#define NORMAL 0
#define PCREL  1

#ifdef __ELF__
#define PREF ""
#else
#define PREF "_"
#endif

/* Register Location Offsets */

#define ICOUNT				PREF "m68k_ICount"

#define REG_DAT				"R_D0"
#define REG_DAT_EBX			"[R_D0+ebx*4]"
#define REG_ADD				"R_A0"
#define REG_A7				"R_A7"
#define REG_USP				"R_USP"
#define REG_ISP				"R_ISP"
#define REG_SRH				"R_SR_H"
#define REG_CCR				"R_CCR"
#define REG_X				"R_XC"
#define REG_PC				"R_PC"
#define REG_IRQ				"R_IRQ"
#define REG_S				"R_SR"
#define REG_IRQ_CALLBACK	"R_IRQ_CALLBACK"
#define REG_RESET_CALLBACK	"R_RESET_CALLBACK"


/* 68010 Regs */

#define REG_VBR				"R_VBR"
#define REG_SFC				"R_SFC"
#define REG_DFC				"R_DFC"

#define FASTCALL_FIRST_REG	"ecx"
#define FASTCALL_SECOND_REG	"edx"



/*
 * Global Variables
 *
 */

FILE *fp			= NULL;

char *comptab		= NULL;
char *CPUtype		= NULL;

int  CPU			= 0;
int  FlagProcess	= 0;
int  CheckInterrupt = 0;
int  ExternalIO	  	= 0;
int  Opcount		= 0;
int  TimingCycles	= 0;
int  AddEACycles	= 0;
int  AccessType		= NORMAL;
int  ppro			= 0;



/* External register preservation */

#ifdef DOS

/* Registers normally saved around C routines anyway */
/* GCC 2.9.1 (dos) seems to preserve EBX,EDI and EBP */
static char SavedRegs[] = "-B--SDB";

#elif defined(WIN32)

/* visual C++, win32, says it preserves ebx, edi, esi, and ebp */
/* ---------- VC++ deosn't preserve EDI? (Kenjo, 110399) ---------- */
static char SavedRegs[] = "-B--S-B";

#else

/* Assume nothing preserved */
static char SavedRegs[] = "-------";

#endif



/* Jump Table */

int OpcodeArray[65536];

/* Lookup Arrays */

static char* regnameslong[] =
{ "EAX","EBX","ECX","EDX","ESI","EDI","EBP"};

static char* regnamesword[] =
{ "AX","BX","CX","DX", "SI", "DI", "BP"};

static char* regnamesshort[] =
{ "AL","BL","CL","DL"};



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
 *							 in order to read SR to AX, use READCCR
 * 12-15  INVALID
 *
 * 19  (A7)+
 * 20  -(A7)
 *
 */

int EAtoAMN(int EA, int Way)
{
	int Work;

	if (Way)
	{
		Work = (EA & 0x7);

		if (Work == 7) Work += ((EA & 0x38) >> 3);

		if (((Work == 3) || (Work == 4)) && (((EA & 0x38) >> 3) == 7))
		{
			Work += 16;
		}
	}
	else
	{
		Work = (EA & 0x38) >> 3;

		if (Work == 7) Work += (EA & 7);

		if (((Work == 3) || (Work == 4)) && ((EA & 7) == 7))
		{
			Work += 16;
		}
	}

	return Work;
}

/*
 * Generate Main or Sub label
 */

char *GenerateLabel(int ID,int Type)
{
	static int LabID,LabNum;
/*
	static char disasm[80];
	char	*dis = disasm;
*/
	if (Type == 0)
	{
		CheckInterrupt=0;			 /* No need to check for Interrupts */
		ExternalIO=0;				  /* Not left Assembler Yet */
		TimingCycles=0;				/* No timing info for this command */
		AddEACycles=1;				 /* default to add in EA timing */
		Opcount++;					  /* for screen display */

		DisOp = ID;
/*
		m68k_disassemble(dis,0);
		sprintf(codebuf, "OP%d_%4.4x:\t\t\t\t; %s", CPU,ID, dis);
*/
		sprintf(codebuf, "OP%d_%4.4x:\t\t\t\t;", CPU,ID);

		LabID  = ID;
		LabNum = 0;
	}
	else
	{
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
	fprintf(fp, "\t\t ALIGN %d\n\n",ALIGNMENT);
}

/*
 * Copy X into Carry
 *
 * There are several ways this could be done, this allows
 * us to easily change the way we are doing it!
 */

void CopyX(void)
{
	/* Copy bit 0 from X flag store into Carry */

	fprintf(fp, "\t\t bt    dword [%s],0\n",REG_X);
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

void ClearRegister(int regno)
{
#ifdef QUICKZERO
	fprintf(fp, "\t\t mov   %s,0\n",regnameslong[regno]);
#else
	fprintf(fp, "\t\t xor   %s,%s\n",regnameslong[regno],regnameslong[regno]);
#endif
}

void Immediate8(void)
{
	/* This takes 3 cycles, 5 bytes, no memory reads */

	fprintf(fp, "\t\t dec   ecx          ; Move range down\n");
	fprintf(fp, "\t\t and   ecx,byte 7   ; Mask out lower bits\n");
	fprintf(fp, "\t\t inc   ecx          ; correct range\n");


	/* This takes 2 cycles, 10 bytes but has a memory read */
	/* I don't know timing for the mov command - assumed 1 */

#if 0
	fprintf(fp, "\t\t and   ecx,byte 7\n");
	fprintf(fp, "\t\t mov   ecx,[ImmTable+ECX*4]\n");
#endif
}

/*
 * This will check for bank changes before
 * resorting to calling the C bank select code
 *
 * Most of the time it does not change!
 *
 */

/* forward used by MemoryBanking */
void Exception(int Number, int BaseCode) ;

void MemoryBanking(int BaseCode)
{
	/* check for odd address */
	fprintf(fp, "\t\t test  esi, dword 1\n");
	fprintf(fp, "\t\t jz    near OP%d_%5.5x\n",CPU,BaseCode);

	/* trying to run at an odd address */
	Exception(3,BaseCode);

	/* Keep Whole PC */

	fprintf(fp, "OP%d_%5.5x:\n",CPU,BaseCode);

/* ASG - always call to the changepc subroutine now, since the number of */
/* bits varies based on the memory model */
#ifdef KEEPHIGHPC
	fprintf(fp, "\t\t mov   [FullPC],ESI\n");
#endif

	/* Mask to n bits */
	fprintf(fp, "\t\t and   esi,[%smem_amask]\n", PREF);

#if 0
#ifdef KEEPHIGHPC
	fprintf(fp, "\t\t mov   [FullPC],ESI\n");
#endif

	/* Mask to 24 bits */
//	fprintf(fp, "\t\t and   esi,0ffffffh\n");

#ifdef ASMBANK
	/* Assembler bank switch */

	fprintf(fp, "\t\t mov   eax,esi\n");
	/* LVR - use a variable bank size */
	fprintf(fp, "\t\t shr   eax," ASMBANK "\n");
	fprintf(fp, "\t\t cmp   [asmbank],eax\n");
	fprintf(fp, "\t\t je    OP%d_%5.5x_Bank\n",CPU,BaseCode);

	fprintf(fp, "\t\t mov   [asmbank],eax\n");
#else
	/* This code is same as macro used by C core */

	fprintf(fp, "\t\t mov   ecx,esi\n");
	fprintf(fp, "\t\t mov   ebx,[_cur_mrhard]\n");
	fprintf(fp, "\t\t shr   ecx,9\n");
	fprintf(fp, "\t\t mov   al,byte [_opcode_entry]\n");
	fprintf(fp, "\t\t cmp   al,[ecx+ebx]\n");
	fprintf(fp, "\t\t je    OP%d_%5.5x_Bank\n",CPU,BaseCode);
#endif
#endif

	/* Call Banking Routine */

	if (SavedRegs[ESI] == '-')
	{
		fprintf(fp, "\t\t mov   [%s],ESI\n",REG_PC);
	}

	if (SavedRegs[EDX] == '-')
	{
		fprintf(fp, "\t\t mov   [%s],edx\n",REG_CCR);
	}

#ifdef FASTCALL
	fprintf(fp, "\t\t mov   %s,esi\n",FASTCALL_FIRST_REG);
#else
	fprintf(fp, "\t\t push  esi\n");
#endif

	fprintf(fp, "\t\t call  [%sa68k_memory_intf+28]\n", PREF);

#ifndef FASTCALL
	fprintf(fp, "\t\t lea   esp,[esp+4]\n");
#endif

	if (SavedRegs[EDX] == '-')
	{
		fprintf(fp, "\t\t mov   edx,[%s]\n",REG_CCR);
	}

	if (SavedRegs[ESI] == '-')
	{
		fprintf(fp, "\t\t mov   esi,[%s]\n",REG_PC);
	}

	/* Update our copy */

	fprintf(fp, "\t\t mov   ebp,dword [%sOP_ROM]\n", PREF);

	fprintf(fp, "OP%d_%5.5x_Bank:\n",CPU,BaseCode);
}

/*
 * Update Previous PC value
 *
 */

void SavePreviousPC(void)
{
#ifdef SAVEPPC
	fprintf(fp, "\t\t mov   [R_PPC],esi\t\t\t ; Keep Previous PC\n");
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

	if (TimingCycles != 0)
	{
		if (TimingCycles > 127)
			fprintf(fp, "\t\t sub   dword [%s],%d\n",ICOUNT,TimingCycles);
		else
		{
			if (TimingCycles != -1)
				fprintf(fp, "\t\t sub   dword [%s],byte %d\n",ICOUNT,TimingCycles);
		}

		if (FlagProcess > 0)
			fprintf(fp, "\t\t pop   EDX\n");
		if (FlagProcess == 2)
			fprintf(fp, "\t\t mov   [%s],edx\n",REG_X);

		fprintf(fp, "\t\t js    near MainExit\n\n");
	}
	else
	{
		fprintf(fp, "\t\t test  dword [%s],0xffffffff\n",ICOUNT);

		if (FlagProcess > 0)
			fprintf(fp, "\t\t pop   EDX\n");
		if (FlagProcess == 2)
			fprintf(fp, "\t\t mov   [%s],edx\n",REG_X);

		fprintf(fp, "\t\t jle   near MainExit\n\n");
	}
	FlagProcess = 0;

#ifdef MAME_DEBUG

	/* Check for Debug Active */

	fprintf(fp, "\n\t\t test    byte [%smame_debug],byte 0xff\n", PREF);
	fprintf(fp, "\t\t jnz   near MainExit\n\n");

#endif

#ifdef FBA_DEBUG

	fprintf(fp, "\n\t\t test    byte [%smame_debug],byte 0xff\n", PREF);
	fprintf(fp, "\t\t jns  near .NoDebug\n");
	fprintf(fp, "\t\t call  near FBADebugActive\n\n");
	fprintf(fp, ".NoDebug\n");

#endif

	if (CheckInterrupt)
	{
		fprintf(fp,"; Check for Interrupt waiting\n\n");
		fprintf(fp,"\t\t test  byte [%s],07H\n",REG_IRQ);
		fprintf(fp,"\t\t jne   near interrupt\n\n");
	}

	if(CPU==2)
	{
		/* 32 bit memory version */
		  fprintf(fp, "\t\t mov   eax,2\n");	/* ASG */
		fprintf(fp, "\t\t xor   eax,esi\n");	/* ASG */

#ifdef STALLCHECK
	  ClearRegister(ECX);
		fprintf(fp, "\t\t mov   cx,[eax+ebp]\n");
#else
		fprintf(fp, "\t\t movzx ecx,word [eax+ebp]\n");
#endif
	}
	else
	{
		/* 16 bit memory */
#ifdef STALLCHECK
	  ClearRegister(ECX);
		fprintf(fp, "\t\t mov   cx,[esi+ebp]\n");
#else
		fprintf(fp, "\t\t movzx ecx,word [esi+ebp]\n");
#endif
	}

	fprintf(fp, "\t\t jmp   [%s_OPCODETABLE+ecx*4]\n\n", CPUtype);
}

/*
 * Flag Routines
 *
 * Size	  = B,W or L
 * Sreg	  = Register to Test
 * TestReg  = Need to test register (false if flags already set up)
 * SetX	  = if C needs to be copied across to X register
 * Delayed  = Delays final processing to end of routine (Completed())
 *
 */

void TestFlags(char Size,int Sreg)
{
	char* Regname="";

	switch (Size)
	{
		case 66:
			Regname = regnamesshort[Sreg];
			break;

		case 87:
			Regname = regnamesword[Sreg];
			break;

		case 76:
			Regname = regnameslong[Sreg];
			break;
	}

	/* Test does not update register	 */
	/* so cannot generate partial stall */

	fprintf(fp, "\t\t test  %s,%s\n",Regname,Regname);
}

void SetFlags(char Size,int Sreg,int Testreg,int SetX,int Delayed)
{
	if (Testreg) TestFlags(Size,Sreg);

	fprintf(fp, "\t\t pushfd\n");

	if (Delayed)
	{
		/* Rest of code done by Completed routine */

		if (SetX) FlagProcess = 2;
		else FlagProcess = 1;
	}
	else
	{
		fprintf(fp, "\t\t pop   EDX\n");

		if (SetX) fprintf(fp, "\t\t mov   [%s],edx\n",REG_X);
	}
}

/******************/
/* Check CPU Type */
/******************/

void CheckCPUtype(int Minimum)
{
	if(CPU==2)
	{
		/* Only check for > 020 */

		if(Minimum>2)
		{
			fprintf(fp, "\t\t mov   eax,[CPUversion]\n");

			fprintf(fp, "\t\t cmp   al,%d\n",Minimum);
			fprintf(fp, "\t\t jb    near ILLEGAL\n\n");
		}
	}
	else
	{
		fprintf(fp, "\t\t mov   eax,[CPUversion]\n");

		if (Minimum == 1)
		{
			fprintf(fp, "\t\t test  eax,eax\n");
			fprintf(fp, "\t\t jz    near ILLEGAL\n\n");
		}
		else
		{
			fprintf(fp, "\t\t cmp   al,%d\n",Minimum);
			fprintf(fp, "\t\t jb    near ILLEGAL\n\n");
		}
	}
}

/************************************/
/* Pre-increment and Post-Decrement */
/************************************/

void IncrementEDI(int Size,int Rreg)
{
	switch (Size)
	{
		case 66:

#ifdef  A7ROUTINE

			/* Always does Byte Increment - A7 uses special routine */

			fprintf(fp, "\t\t inc   dword [%s+%s*4]\n",REG_ADD,regnameslong[Rreg]);

#else

			/* A7 uses same routines, so inc by 2 if A7 */

			fprintf(fp, "\t\t cmp   %s,7\n",regnamesshort[Rreg]);
			fprintf(fp, "\t\t cmc\n");
			fprintf(fp, "\t\t adc   dword [%s+%s*4],byte 1\n",REG_ADD,regnameslong[Rreg]);

#endif
			break;

		case 87:

			fprintf(fp, "\t\t add   dword [%s+%s*4],byte 2\n",REG_ADD,regnameslong[Rreg]);
			break;

		case 76:

			fprintf(fp, "\t\t add   dword [%s+%s*4],byte 4\n",REG_ADD,regnameslong[Rreg]);
			break;
	}
}

void DecrementEDI(int Size,int Rreg)
{
	switch (Size)
	{
		case 66:

#ifdef  A7ROUTINE

			/* Always does Byte Increment - A7 uses special routine */

			fprintf(fp, "\t\t dec   EDI\n");

#else

			/* A7 uses same routines, so dec by 2 if A7 */

			fprintf(fp, "\t\t cmp   %s,7\n",regnamesshort[Rreg]);
			fprintf(fp, "\t\t cmc\n");
			fprintf(fp, "\t\t sbb   dword edi,byte 1\n");

#endif
			break;

		case 87:

			fprintf(fp, "\t\t sub   EDI,byte 2\n");
			break;

		case 76:
			fprintf(fp, "\t\t sub   EDI,byte 4\n");
			break;
	}
}

/*
 * Generate an exception
 *
 * if Number = -1 then assume value in AL already
 *					 code must continue running afterwards
 *
 */

void Exception(int Number, int BaseCode)
{
	if (Number > -1)
	{
		fprintf(fp, "\t\t sub   esi,byte 2\n");
		fprintf(fp, "\t\t mov   al,%d\n",Number);
	}

	fprintf(fp, "\t\t call  Exception\n\n");

	if (Number > -1)
		Completed();
}


/********************/
/* Address Routines */
/********************/

/*
 * Decode Intel flags into AX as SR register
 *
 * Wreg = spare register to use (must not be EAX or EDX)
 */

void ReadCCR(char Size, int Wreg)
{
	fprintf(fp, "\t\t mov   eax,edx\n");
	fprintf(fp, "\t\t mov   ah,byte [%s]\n",REG_X);

	/* Partial stall so .. switch to new bit of processing */

	fprintf(fp, "\t\t mov   %s,edx\n",regnameslong[Wreg]);
	fprintf(fp, "\t\t and   %s,byte 1\n",regnameslong[Wreg]);

	/* Finish what we started */

	fprintf(fp, "\t\t shr   eax,4\n");
	fprintf(fp, "\t\t and   eax,byte 01Ch \t\t; X, N & Z\n\n");

	/* and complete second task */

	fprintf(fp, "\t\t or    eax,%s \t\t\t\t; C\n\n",regnameslong[Wreg]);

	/* and Finally */

	fprintf(fp, "\t\t mov   %s,edx\n",regnameslong[Wreg]);
	fprintf(fp, "\t\t shr   %s,10\n",regnameslong[Wreg]);
	fprintf(fp, "\t\t and   %s,byte 2\n",regnameslong[Wreg]);
	fprintf(fp, "\t\t or    eax,%s\t\t\t\t; O\n\n",regnameslong[Wreg]);

	if (Size == 'W')
	{
		fprintf(fp, "\t\t mov   ah,byte [%s] \t; T, S & I\n\n",REG_SRH);

#ifdef MASKCCR
		fprintf(fp, "\t\t and   ax,0A71Fh\t; Mask unused bits\n");
#endif
	}
}

/*
 * Convert SR into Intel flags
 *
 * Also handles change of mode from Supervisor to User
 *
 * n.b. This is also called by EffectiveAddressWrite
 */

void WriteCCR(char Size)
{
	if (Size == 'W')
	{
		/* Did we change from Supervisor to User mode ? */

		char *Label = GenerateLabel(0,1);

		fprintf(fp, "\t\t test  ah,20h \t\t\t; User Mode ?\n");
		fprintf(fp, "\t\t jne   short %s\n\n",Label);

		/* Mode Switch - Update A7 */

		fprintf(fp, "\t\t mov   edx,[%s]\n",REG_A7);
		fprintf(fp, "\t\t mov   [%s],edx\n",REG_ISP);
		fprintf(fp, "\t\t mov   edx,[%s]\n",REG_USP);
		fprintf(fp, "\t\t mov   [%s],edx\n",REG_A7);

		fprintf(fp, "%s:\n",Label);
		fprintf(fp, "\t\t mov   byte [%s],ah \t;T, S & I\n",REG_SRH);

		/* Mask may now allow Interrupt */

		CheckInterrupt += 1;
	}

	/* Flags */

	fprintf(fp, "\t\t and   eax,byte 1Fh\n");
	fprintf(fp, "\t\t mov   edx,[IntelFlag+eax*4]\n");
	fprintf(fp, "\t\t mov   [%s],dh\n",REG_X);
	fprintf(fp, "\t\t and   edx,0EFFh\n");
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

void Memory_Read(char Size,int AReg,char *Flags,int Mask)
{
	ExternalIO = 1;

	/* Save PC */

	fprintf(fp, "\t\t mov   [%s],ESI\n",REG_PC);

	/* Check for special mask condition */

	/* ASG - no longer need to mask addresses here */
/*	if (Mask == 2)
		fprintf(fp, "\t\t and   %s,0FFFFFFh\n",regnameslong[AReg]);*/

	/* Check to see if registers need saving */

	if ((Flags[EDX] != '-') && (SavedRegs[EDX] == '-'))
	{
		fprintf(fp, "\t\t mov   [%s],edx\n",REG_CCR);
	}

	if ((Flags[EBX] != '-') && (SavedRegs[EBX] == '-'))
	{
		fprintf(fp, "\t\t push  EBX\n");
	}

	if ((Flags[ECX] != '-') && (SavedRegs[ECX] == '-'))
	{
		fprintf(fp, "\t\t push  ECX\n");
	}

	if ((Flags[EDI] != '-') && (SavedRegs[EDI] == '-'))
	{
		fprintf(fp, "\t\t push  EDI\n");
	}

	/* Sort Address out */

#ifdef FASTCALL

	fprintf(fp, "\t\t mov   %s,%s\n",FASTCALL_FIRST_REG,regnameslong[AReg]);

	/* ASG - no longer need to mask addresses here */
/*	if (Mask == 1)
		fprintf(fp, "\t\t and   %s,0FFFFFFh\n",FASTCALL_FIRST_REG);*/

#else

	if (Mask == 1)
	{
		if ((Flags[AReg] != '-') && (SavedRegs[AReg] != '-'))
		{
			/* Don't trash a wanted safe register */

			fprintf(fp, "\t\t mov   EAX,%s\n",regnameslong[AReg]);
	/* ASG - no longer need to mask addresses here */
/*			fprintf(fp, "\t\t and   EAX,0FFFFFFh\n");*/
			fprintf(fp, "\t\t push  EAX\n");
		}
		else
		{
	/* ASG - no longer need to mask addresses here */
/*			fprintf(fp, "\t\t and   %s,0FFFFFFh\n",regnameslong[AReg]);*/
			fprintf(fp, "\t\t push  %s\n",regnameslong[AReg]);
		}
	}
	else
		fprintf(fp, "\t\t push  %s\n",regnameslong[AReg]);

#endif



	/* Call Mame memory routine */

	/* ASG - changed these to call through the function pointers */

#ifdef ENCRYPTED
	switch (AccessType)
	{
		  case NORMAL :
			 switch (Size)
			 {
				 case 66 :
					fprintf(fp, "\t\t call  [%sa68k_memory_intf+4]\n", PREF);
					break;

				 case 87 :
					fprintf(fp, "\t\t call  [%sa68k_memory_intf+8]\n", PREF);
					break;

				 case 76 :
					fprintf(fp, "\t\t call  [%sa68k_memory_intf+12]\n", PREF);
					break;
			 }
			 break;

		case PCREL :

			 switch (Size)
			 {
				 case 66 :
					 fprintf(fp, "\t\t call  [%sa68k_memory_intf+32]\n", PREF);
					 break;

				 case 87 :
					 fprintf(fp, "\t\t call  [%sa68k_memory_intf+36]\n", PREF);
					 break;

				 case 76 :
					 fprintf(fp, "\t\t call  [%sa68k_memory_intf+40]\n", PREF);
					 break;
			 }
			 break;
	}

// AccessType = NORMAL;

#else

	switch (Size)
	{
		case 66 :
			fprintf(fp, "\t\t call  [%sa68k_memory_intf+4]\n", PREF);
			break;

		case 87 :
			fprintf(fp, "\t\t call  [%sa68k_memory_intf+8]\n", PREF);
			break;

		case 76 :
			fprintf(fp, "\t\t call  [%sa68k_memory_intf+12]\n", PREF);
			break;
	}
#endif

	/* Correct Stack */

#ifndef FASTCALL
	fprintf(fp, "\t\t lea   esp,[esp+4]\n");
#endif



	/* Restore registers */

	/* Check to see if registers need restoring */

	if ((Flags[EDI] != '-') && (SavedRegs[EDI] == '-'))
	{
		fprintf(fp, "\t\t pop   EDI\n");
	}

	if ((Flags[ECX] != '-') && (SavedRegs[ECX] == '-'))
	{
		fprintf(fp, "\t\t pop   ECX\n");
	}

	if ((Flags[EBX] != '-') && (SavedRegs[EBX] == '-'))
	{
		fprintf(fp, "\t\t pop   EBX\n");
	}

	if ((Flags[ESI] != '-') && (SavedRegs[ESI] == '-'))
	{
		fprintf(fp, "\t\t mov   ESI,[%s]\n",REG_PC);
	}

	if ((Flags[EDX] != '-') && (SavedRegs[EDX] == '-'))
	{
		fprintf(fp, "\t\t mov   EDX,[%s]\n",REG_CCR);
	}

	if ((Flags[EBP] != '-') && (SavedRegs[EBP] == '-'))
	{
		fprintf(fp, "\t\t mov   ebp,dword [%sOP_ROM]\n", PREF);
	}
}

void Memory_Write(char Size,int AReg,int DReg,char *Flags,int Mask)
{
	ExternalIO = 1;

	/* Save PC */

	fprintf(fp, "\t\t mov   [%s],ESI\n",REG_PC);

	/* Check for special mask condition */

	/* ASG - no longer need to mask addresses here */
/*	if (Mask == 2)
		fprintf(fp, "\t\t and   %s,0FFFFFFh\n",regnameslong[AReg]);*/

	/* Check to see if registers need saving */

	if ((Flags[EDX] != '-') && (SavedRegs[EDX] == '-'))
	{
		fprintf(fp, "\t\t mov   [%s],edx\n",REG_CCR);
	}

	if ((Flags[EAX] != '-') && (SavedRegs[EAX] == '-'))
	{
		fprintf(fp, "\t\t push  EAX\n");
	}

	if ((Flags[EBX] != '-') && (SavedRegs[EBX] == '-'))
	{
		fprintf(fp, "\t\t push  EBX\n");
	}

	if ((Flags[ECX] != '-') && (SavedRegs[ECX] == '-'))
	{
		fprintf(fp, "\t\t push  ECX\n");
	}

	if ((Flags[EDI] != '-') && (SavedRegs[EDI] == '-'))
	{
		fprintf(fp, "\t\t push  EDI\n");
	}

#ifdef FASTCALL

	fprintf(fp, "\t\t mov   %s,%s\n",FASTCALL_SECOND_REG,regnameslong[DReg]);
	fprintf(fp, "\t\t mov   %s,%s\n",FASTCALL_FIRST_REG,regnameslong[AReg]);

	/* ASG - no longer need to mask addresses here */
/*	if (Mask == 1)
		fprintf(fp, "\t\t and   %s,0FFFFFFh\n",FASTCALL_FIRST_REG);*/

#else

	fprintf(fp, "\t\t push  %s\n",regnameslong[DReg]);

	if (Mask == 1)
	{
		if ((Flags[AReg] != '-') && (SavedRegs[AReg] != '-'))
		{
			/* Don't trash a wanted safe register */

			fprintf(fp, "\t\t mov   EAX,%s\n",regnameslong[AReg]);
	/* ASG - no longer need to mask addresses here */
/*			fprintf(fp, "\t\t and   EAX,0FFFFFFh\n");*/
			fprintf(fp, "\t\t push  EAX\n");
		}
		else
		{
	/* ASG - no longer need to mask addresses here */
/*			fprintf(fp, "\t\t and   %s,0FFFFFFh\n",regnameslong[AReg]);*/
			fprintf(fp, "\t\t push  %s\n",regnameslong[AReg]);
		}
	}
	else
		fprintf(fp, "\t\t push  %s\n",regnameslong[AReg]);

#endif



	/* Call Mame Routine */

	/* ASG - changed these to call through the function pointers */
	switch (Size)
	{
		case 66 :
			fprintf(fp, "\t\t call  [%sa68k_memory_intf+16]\n", PREF);
			break;

		case 87 :
			fprintf(fp, "\t\t call  [%sa68k_memory_intf+20]\n", PREF);
			break;

		case 76 :
			fprintf(fp, "\t\t call  [%sa68k_memory_intf+24]\n", PREF);
			break;
	}

	/* Correct Stack */

#ifndef FASTCALL
	fprintf(fp, "\t\t lea   esp,[esp+8]\n");
#endif



	/* Restore registers */

	/* Check to see if registers need restoring */

	if ((Flags[EDI] != '-') && (SavedRegs[EDI] == '-'))
	{
		fprintf(fp, "\t\t pop   EDI\n");
	}

	if ((Flags[ECX] != '-') && (SavedRegs[ECX] == '-'))
	{
		fprintf(fp, "\t\t pop   ECX\n");
	}

	if ((Flags[EBX] != '-') && (SavedRegs[EBX] == '-'))
	{
		fprintf(fp, "\t\t pop   EBX\n");
	}

	if ((Flags[EAX] != '-') && (SavedRegs[EAX] == '-'))
	{
		fprintf(fp, "\t\t pop   EAX\n");
	}

	if ((Flags[EDX] != '-') && (SavedRegs[EDX] == '-'))
	{
		fprintf(fp, "\t\t mov   EDX,[%s]\n",REG_CCR);
	}

	if ((Flags[ESI] != '-') && (SavedRegs[ESI] == '-'))
	{
		fprintf(fp, "\t\t mov   ESI,[%s]\n",REG_PC);
	}

	if ((Flags[EBP] != '-') && (SavedRegs[EBP] == '-'))
	{
		fprintf(fp, "\t\t mov   ebp,dword [%sOP_ROM]\n", PREF);
	}
}


/*
 * Fetch data from Code area
 *
 * Dreg	= Destination Register
 * Extend = Sign Extend Word to Long
 *
 */

void Memory_Fetch(char Size,int Dreg,int Extend)
{
	static int loopcount=0;

	/* Always goes via OP_ROM */

	if(CPU!=2)
	{
		/* 16 Bit version */

		if ((Extend == TRUE) & (Size == 'W'))
		  fprintf(fp, "\t\t movsx %s,word [esi+ebp]\n",regnameslong[Dreg]);
		else if (Size == 'W')
		  fprintf(fp, "\t\t movzx %s,word [esi+ebp]\n",regnameslong[Dreg]);
		else
		  fprintf(fp, "\t\t mov   %s,dword [esi+ebp]\n",regnameslong[Dreg]);

		if (Size == 'L')
	  	 fprintf(fp, "\t\t rol   %s,16\n",regnameslong[Dreg]);
	}
	else
	{
		/* 32 Bit version */

		if (Size == 'W')
		{
			fprintf(fp, "\t\t mov   %s,esi\n",regnameslong[Dreg]);
			fprintf(fp, "\t\t xor   %s,byte 2\n",regnameslong[Dreg]);

			if (Extend == TRUE)
				fprintf(fp, "\t\t movsx %s,word [%s+ebp]\n",regnameslong[Dreg],regnameslong[Dreg]);
			else
				fprintf(fp, "\t\t movzx %s,word [%s+ebp]\n",regnameslong[Dreg],regnameslong[Dreg]);
		}
		else
		{
			 // if address is exact multiple of 4, long read will work
			// otherwise we need to get words address-2 and address+4

			// Always read long
		 fprintf(fp, "\t\t test  esi,2\n");
if (!ppro)
{
			fprintf(fp, "\t\t mov   %s,dword [esi+ebp]\n",regnameslong[Dreg]);

			// OK ?
			fprintf(fp, "\t\t jz    short FL%3.3d\n",loopcount+1);

			// no, so get high word
			fprintf(fp, "\t\t mov   %s,dword [esi+ebp-4]\n",regnameslong[Dreg]);
			// and get low word
			fprintf(fp, "\t\t mov   %s,word [esi+ebp+4]\n",regnamesword[Dreg]);

			fprintf(fp, "FL%3.3d:\n",++loopcount);
}
else
{
			fprintf(fp, "\t\t cmovnz %s,dword [esi+ebp-4]\n",regnameslong[Dreg]);
			fprintf(fp, "\t\t cmovz %s,dword [esi+ebp]\n",regnameslong[Dreg]);
			fprintf(fp, "\t\t cmovnz %s,word [esi+ebp+4]\n",regnamesword[Dreg]);
}
		}
	}
}

/**********************/
/* Push PC onto Stack */
/**********************/

void PushPC(int Wreg,int Wreg2,char *Flags, int Mask)
{

	/* Wreg2 is only used when high byte is kept  */
	/* If it is EBP then the register is restored */

	fprintf(fp, "\t\t mov   %s,[%s]\t ; Push onto Stack\n",regnameslong[Wreg],REG_A7);
	fprintf(fp, "\t\t sub   %s,byte 4\n",regnameslong[Wreg]);
	fprintf(fp, "\t\t mov   [%s],%s\n",REG_A7,regnameslong[Wreg]);

#ifndef KEEPHIGHPC

	Memory_Write('L',Wreg,ESI,Flags,Mask);

#else

	fprintf(fp, "\t\t mov   %s,[FullPC]\n",regnameslong[Wreg2]);
	fprintf(fp, "\t\t and   %s,0xff000000\n",regnameslong[Wreg2]);
	fprintf(fp, "\t\t or    %s,ESI\n",regnameslong[Wreg2]);

	Memory_Write('L',Wreg,Wreg2,Flags,Mask);

	if (Wreg2 == EBP)
	{
		fprintf(fp, "\t\t mov   ebp,dword [%sOP_ROM]\n", PREF);
	}

#endif
}

void ExtensionDecode(int SaveEDX)
{
	char *Label = GenerateLabel(0,1);

	if (SaveEDX)
		fprintf(fp, "\t\t push  edx\n");

	Memory_Fetch('W',EAX,FALSE);
	fprintf(fp, "\t\t add   esi,byte 2\n");

	if(CPU!=2)
	{
		/* 68000 Extension */

		fprintf(fp, "\t\t mov   edx,eax\n");
		fprintf(fp, "\t\t shr   eax,12\n");
		fprintf(fp, "\t\t test  edx,0x0800\n");
		fprintf(fp, "\t\t mov   eax,[%s+eax*4]\n",REG_DAT);
		fprintf(fp, "\t\t jnz   short %s\n",Label);
		fprintf(fp, "\t\t cwde\n");
		fprintf(fp, "%s:\n",Label);
		fprintf(fp, "\t\t lea   edi,[edi+eax]\n");
		fprintf(fp, "\t\t movsx edx,dl\n");
		fprintf(fp, "\t\t lea   edi,[edi+edx]\n");
	}
	else
	{
		  /* 68020 Extension */

		// eax holds scaled index

		// might be faster just to push all regs
		fprintf(fp, "\t\t push  ebx\n");
		fprintf(fp, "\t\t push  ecx\n");

		// copies for later use
		fprintf(fp, "\t\t mov   ecx,eax\n");
		fprintf(fp, "\t\t mov   edx,eax\n");

		// check E bit to see if displacement or full format
		fprintf(fp, "\t\t test  edx,0x0100\n");
		fprintf(fp, "\t\t jz   short %s_a\n",Label);
		// full mode so check IS (index supress)
		fprintf(fp, "\t\t test  edx,0x0040\n");
		fprintf(fp, "\t\t jz   short %s_b\n",Label);
		// set index to 0, it's not added
		ClearRegister(EAX);
		fprintf(fp, "\t\t jmp  short %s_d\n",Label);  // near

		// add displacement
  fprintf(fp, "%s_a:\n",Label);
		fprintf(fp, "\t\t movsx eax,al\n");
		fprintf(fp, "\t\t lea   edi,[edi+eax]\n");

  fprintf(fp, "%s_b:\n",Label);
		// calc index always scale (68k will scale by 1)
		fprintf(fp, "\t\t mov   eax,edx\n");
		fprintf(fp, "\t\t shr   eax,12\n");
		fprintf(fp, "\t\t test  edx,0x0800\n");
		fprintf(fp, "\t\t mov   eax,[%s+eax*4]\n",REG_DAT);
		fprintf(fp, "\t\t jnz   short %s_c\n",Label);
		fprintf(fp, "\t\t cwde\n");

  fprintf(fp, "%s_c:\n",Label);
		fprintf(fp, "\t\t shr   ecx,byte 9\n");
		fprintf(fp, "\t\t and   ecx,byte 3\n");
		fprintf(fp, "\t\t shl   eax,cl\n");

		// if brief mode we can add index and exit
		fprintf(fp, "\t\t test  edx,0x0100\n");
		fprintf(fp, "\t\t jz    near %s_i2\n",Label);

  fprintf(fp, "%s_d:\n",Label);
		// check BS (base supress)
		// if BS is 1 then set edi to 0
		fprintf(fp, "\t\t test  edx,0x0080\n");
		fprintf(fp, "\t\t jz    short %s_4a\n",Label);
		ClearRegister(EDI);
		// if null displacement skip over
		fprintf(fp, "%s_4a:\n",Label);
		fprintf(fp, "\t\t test  edx,0x0020\n");
		fprintf(fp, "\t\t jz    short %s_f\n",Label);

		// **** calc base displacement ****
		// is it long
		fprintf(fp, "\t\t test  edx,0x0010\n");
		fprintf(fp, "\t\t jz    short %s_e\n",Label);
		// fetch long base
		Memory_Fetch('L',EBX,FALSE);
		fprintf(fp, "\t\t lea   edi,[edi+ebx]\n");
		fprintf(fp, "\t\t add   esi,byte 4\n");
		fprintf(fp, "\t\t jmp   short %s_f\n",Label);

		// fetch word base
  fprintf(fp, "%s_e:\n",Label);
		Memory_Fetch('W',EBX,TRUE);
		fprintf(fp, "\t\t lea   edi,[edi+ebx]\n");
		fprintf(fp, "\t\t add   esi,byte 2\n");

		// **** indirect? ****
  fprintf(fp, "%s_f:\n",Label);
		fprintf(fp, "\t\t test  edx,0x0003\n");
		fprintf(fp, "\t\t jz    near %s_7a\n",Label);
		// pre or post indirect
		fprintf(fp, "\t\t test  edx,0x0004\n");
		fprintf(fp, "\t\t jnz   short %s_g\n",Label);
		// do pre
		fprintf(fp, "\t\t lea   edi,[edi+eax]\n");
		Memory_Read('L',EDI,"ABCDSDB",2);
		fprintf(fp, "\t\t mov   edi,eax\n");
		fprintf(fp, "\t\t jmp   short %s_h\n",Label);

		// do post
 fprintf(fp, "%s_g:\n",Label);
		fprintf(fp, "\t\t push  eax\n");
		Memory_Read('L',EDI,"-B-DS-B",2);
		fprintf(fp, "\t\t pop   edi\n");

 fprintf(fp, "%s_7a:\n",Label);
		fprintf(fp, "\t\t lea   edi,[edi+eax]\n");

		// **** outer displacement ****
		// if null displacement skip over
 fprintf(fp, "%s_h:\n",Label);
		fprintf(fp, "\t\t test  edx,0x0002\n");
		fprintf(fp, "\t\t jz    short %s_j\n",Label);
		// word or long?
		fprintf(fp, "\t\t test  edx,0x0001\n");
		fprintf(fp, "\t\t jz    short %s_i\n",Label);
		// fetch long
		Memory_Fetch('L',EAX,FALSE);
		fprintf(fp, "\t\t add   esi,byte 4\n");
		fprintf(fp, "\t\t jmp   short %s_i2\n",Label);
		// fetch word
  fprintf(fp, "%s_i:\n",Label);
		Memory_Fetch('W',EAX,TRUE);
		fprintf(fp, "\t\t add   esi,byte 2\n");
  fprintf(fp, "%s_i2:\n",Label);
		fprintf(fp, "\t\t lea   edi,[edi+eax]\n");

		// **** exit ****
  fprintf(fp, "%s_j:\n",Label);
		fprintf(fp, "\t\t pop   ecx\n");
		fprintf(fp, "\t\t pop   ebx\n");
	}

	if (SaveEDX)
		fprintf(fp, "\t\t pop   edx\n");
}

/* Calculate Effective Address - Return address in EDI
 *
 * mode = Effective Address from Instruction
 * Size = Byte,Word or Long
 * Rreg = Register with Register Number in
 *
 * Only for modes 2 - 10 (5-10 clobber EAX)
 */

void EffectiveAddressCalculate(int mode,char Size,int Rreg,int SaveEDX)
{
	/* timing */

	if ((TimingCycles > 0) && (AddEACycles!=0))
	{
		switch (mode)
		{
			case 2:		/* (An) */
			case 3:		/* (An)+ */
			case 11:	/* #x,SR,CCR */
			case 19:	/* (A7)+ */
				TimingCycles += 4 ;
				break ;

			case 4:		/* -(An) */
			case 20:	/* -(A7) */
				TimingCycles += (CPU==2) ? 5 : 6 ;
				break ;

			case 5:		/* x(An) */
			case 9:		/* x(PC) */
				TimingCycles += (CPU==2) ? 5 : 8 ;
				break ;

			case 7:		/* x.w */
				TimingCycles += (CPU==2) ? 4 : 8 ;
				break ;

			case 6:		/* x(An,xr.s) */
			case 10:	/* x(PC,xr.s) */
				TimingCycles += (CPU==2) ? 7 : 10 ;
				break ;

			case 8:		/* x.l */
				TimingCycles += (CPU==2) ? 4 : 12 ;
				break ;
		}

		/* long w/r adds 4 cycles */

		if ((mode>1) && (Size == 'L') && (CPU != 2))
			TimingCycles += 4 ;
	}

	AccessType = NORMAL;

	switch (mode)
	{

		case 2:
			fprintf(fp, "\t\t mov   EDI,[%s+%s*4]\n",REG_ADD,regnameslong[Rreg]);
			break;

		case 3:
			fprintf(fp, "\t\t mov   EDI,[%s+%s*4]\n",REG_ADD,regnameslong[Rreg]);
			IncrementEDI(Size,Rreg);
			break;

		case 4:
			fprintf(fp, "\t\t mov   EDI,[%s+%s*4]\n",REG_ADD,regnameslong[Rreg]);
			DecrementEDI(Size,Rreg);
			fprintf(fp, "\t\t mov   [%s+%s*4],EDI\n",REG_ADD,regnameslong[Rreg]);
			break;

		case 5:
			Memory_Fetch('W',EAX,TRUE);
			fprintf(fp, "\t\t mov   EDI,[%s+%s*4]\n",REG_ADD,regnameslong[Rreg]);
			fprintf(fp, "\t\t add   esi,byte 2\n");
			fprintf(fp, "\t\t add   edi,eax\n");
			break;

		case 6:

			/* Get Address register Value */

			fprintf(fp, "\t\t mov   EDI,[%s+%s*4]\n",REG_ADD,regnameslong[Rreg]);

			/* Add Extension Details */

			ExtensionDecode(SaveEDX);
			break;

		case 7:

			/* Get Word */

			Memory_Fetch('W',EDI,TRUE);
//		 fprintf(fp, "\t\t movsx edi,di\n");
			fprintf(fp, "\t\t add   esi,byte 2\n");
			break;

		case 8:

			/* Get Long */

			Memory_Fetch('L',EDI,FALSE);
			fprintf(fp, "\t\t add   esi,byte 4\n");
			break;

		case 9:

			AccessType = PCREL;

			Memory_Fetch('W',EAX,TRUE);
//		 fprintf(fp, "\t\t movsx eax,ax\n");
			fprintf(fp, "\t\t mov   EDI,ESI           ; Get PC\n");
			fprintf(fp, "\t\t add   esi,byte 2\n");
			fprintf(fp, "\t\t add   edi,eax         ; Add Offset to PC\n");
			break;

		case 10:

			AccessType = PCREL;

			/* Get PC */

			fprintf(fp, "\t\t mov   edi,esi           ; Get PC\n");

			/* Add Extension Details */

			ExtensionDecode(SaveEDX);

			break;

		case 19:

			/* (A7)+ */

			fprintf(fp, "\t\t mov   edi,[%s]    ; Get A7\n",REG_A7);
			fprintf(fp, "\t\t add   dword [%s],byte 2\n",REG_A7);
			break;

		case 20:

			/* -(A7) */

			fprintf(fp, "\t\t mov   edi,[%s]    ; Get A7\n",REG_A7);
			fprintf(fp, "\t\t sub   edi,byte 2\n");
			fprintf(fp, "\t\t mov   [%s],edi\n",REG_A7);
			break;

	}
}

/* Read from Effective Address
 *
 * mode = Effective Address from Instruction
 * Size = Byte,Word or Long
 * Rreg = Register with Register Number in
 * Flag = Registers to preserve (EDX is handled by SaveEDX)
 *
 * Return
 * Dreg = Register to return result in (EAX is usually most efficient)
 * (modes 5 to 10) EDI  = Address of data read (masked with FFFFFF)
 */

void EffectiveAddressRead(int mode,char Size,int Rreg,int Dreg,const char *flags,int SaveEDX)
{
	char* Regname="";
	int	MaskMode;
	char Flags[8];

	AccessType = NORMAL;

	strcpy(Flags,flags);

	/* Which Masking to Use */

	if (Flags[5] != '-')
		MaskMode = 2;
	else
		MaskMode = 1;

	if (SaveEDX)
		Flags[3] = 'D';
	else
		Flags[3] = '-';

	switch (Size)
	{
		case 66:
			Regname = regnamesshort[Dreg];
			break;

		case 87:
			Regname = regnamesword[Dreg];
			break;

		case 76:
			Regname = regnameslong[Dreg];
			break;
	}

	switch (mode & 15)
	{

		case 0:

			/* Read 32 bits - No prefix */

			fprintf(fp, "\t\t mov   %s,[%s+%s*4]\n",regnameslong[Dreg],REG_DAT,regnameslong[Rreg]);
			break;

		case 1:

			/* Read 32 bits - No prefix */

			fprintf(fp, "\t\t mov   %s,[%s+%s*4]\n",regnameslong[Dreg],REG_ADD,regnameslong[Rreg]);
			break;

		case 2:
			EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);

			Memory_Read(Size,EDI,Flags,MaskMode);

			if (Dreg != EAX)
			{
				fprintf(fp, "\t\t mov   %s,EAX\n",regnameslong[Dreg]);
			}
			break;

		case 3:
			EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);

			Memory_Read(Size,EDI,Flags,MaskMode);

			if (Dreg != EAX)
			{
				fprintf(fp, "\t\t mov   %s,EAX\n",regnameslong[Dreg]);
			}
			break;

		case 4:
			EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);

			Memory_Read(Size,EDI,Flags,MaskMode);

			if (Dreg != EAX)
			{
				fprintf(fp, "\t\t mov   %s,EAX\n",regnameslong[Dreg]);
			}
			break;


		case 5:
			EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);

			Memory_Read(Size,EDI,Flags,MaskMode);

			if (Dreg != EAX)
			{
				fprintf(fp, "\t\t mov   %s,EAX\n",regnameslong[Dreg]);
			}
			break;

		case 6:
			EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);

			Memory_Read(Size,EDI,Flags,MaskMode);

			if (Dreg != EAX)
			{
				fprintf(fp, "\t\t mov   %s,EAX\n",regnameslong[Dreg]);
			}
			break;

		case 7:
			EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);

			Memory_Read(Size,EDI,Flags,MaskMode);

			if (Dreg != EAX)
			{
				fprintf(fp, "\t\t mov   %s,EAX\n",regnameslong[Dreg]);
			}
			break;

		case 8:
			EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);

			Memory_Read(Size,EDI,Flags,MaskMode);

			if (Dreg != EAX)
			{
				fprintf(fp, "\t\t mov   %s,EAX\n",regnameslong[Dreg]);
			}
			break;

		case 9:
			EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);

			Memory_Read(Size,EDI,Flags,MaskMode);

			if (Dreg != EAX)
			{
				fprintf(fp, "\t\t mov   %s,EAX\n",regnameslong[Dreg]);
			}
			break;

		case 10:
			EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);

			Memory_Read(Size,EDI,Flags,MaskMode);

			if (Dreg != EAX)
			{
				fprintf(fp, "\t\t mov   %s,EAX\n",regnameslong[Dreg]);
			}
			break;

		case 11:

			/* Immediate - for SR or CCR see ReadCCR() */

			if (Size == 'L')
			{
				Memory_Fetch('L',Dreg,FALSE);
				fprintf(fp, "\t\t add   esi,byte 4\n");
			}
			else
			{
				Memory_Fetch('W',Dreg,FALSE);
				fprintf(fp, "\t\t add   esi,byte 2\n");
			};
			break;
	}
}

/*
 * EA	= Effective Address from Instruction
 * Size = Byte,Word or Long
 * Rreg = Register with Register Number in
 *
 * Writes from EAX
 */

void EffectiveAddressWrite(int mode,char Size,int Rreg,int CalcAddress,const char *flags,int SaveEDX)
{
	int	MaskMode;
	char* Regname="";
	char Flags[8];


	strcpy(Flags,flags);

	/* Which Masking to Use ? */

	if (CalcAddress)
	{
		if (Flags[5] != '-')
			MaskMode = 2;
		else
			MaskMode = 1;
	}
	else
		MaskMode = 0;

	if (SaveEDX)
		Flags[3] = 'D';
	else
		Flags[3] = '-';

	switch (Size)
	{
		case 66:
			Regname = regnamesshort[0];
			break;

		case 87:
			Regname = regnamesword[0];
			break;

		case 76:
			Regname = regnameslong[0];
			break;
	}

	switch (mode & 15)
	{

		case 0:
			fprintf(fp, "\t\t mov   [%s+%s*4],%s\n",REG_DAT,regnameslong[Rreg],Regname);
			break;

		case 1:
			if (Size == 66)
			{
				/* Not Allowed */

				fprintf(fp, "DUFF CODE!\n");
			}
			else
			{
				if (Size == 87)
				{
					fprintf(fp, "\t\t cwde\n");
				}

				fprintf(fp, "\t\t mov   [%s+%s*4],%s\n",REG_ADD,regnameslong[Rreg],regnameslong[0]);
			}
			break;

		case 2:
			if (CalcAddress) EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);
			Memory_Write(Size,EDI,EAX,Flags,MaskMode);
			break;

		case 3:
			if (CalcAddress) EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);
			Memory_Write(Size,EDI,EAX,Flags,MaskMode);
			break;

		case 4:
			if (CalcAddress) EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);
			Memory_Write(Size,EDI,EAX,Flags,MaskMode);
			break;

		case 5:
			if (CalcAddress)
			{
				fprintf(fp, "\t\t push  EAX\n");
				EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);
				fprintf(fp, "\t\t pop   EAX\n");
			}
			Memory_Write(Size,EDI,EAX,Flags,MaskMode);
			break;

		case 6:
			if (CalcAddress)
			{
				fprintf(fp, "\t\t push  EAX\n");
				EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);
				fprintf(fp, "\t\t pop   EAX\n");
			}
			Memory_Write(Size,EDI,EAX,Flags,MaskMode);
			break;

		case 7:
			if (CalcAddress)
			{
				fprintf(fp, "\t\t push  EAX\n");
				EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);
				fprintf(fp, "\t\t pop   EAX\n");
			}
			Memory_Write(Size,EDI,EAX,Flags,MaskMode);
			break;

		case 8:
			if (CalcAddress)
			{
				fprintf(fp, "\t\t push  EAX\n");
				EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);
				fprintf(fp, "\t\t pop   EAX\n");
			}
			Memory_Write(Size,EDI,EAX,Flags,MaskMode);
			break;

		case 9:
			if (CalcAddress)
			{
				fprintf(fp, "\t\t push  EAX\n");
				EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);
				fprintf(fp, "\t\t pop   EAX\n");
			}
			Memory_Write(Size,EDI,EAX,Flags,MaskMode);
			break;

		case 10:
			if (CalcAddress)
			{
				fprintf(fp, "\t\t push  EAX\n");
				EffectiveAddressCalculate(mode,Size,Rreg,SaveEDX);
				fprintf(fp, "\t\t pop   EAX\n");
			}
			Memory_Write(Size,EDI,EAX,Flags,MaskMode);
			break;

		case 11:

			/* SR, CCR - Chain to correct routine */

			WriteCCR(Size);
	}
}

/* Condition Decode Routines */

/*
 * mode = condition to check for
 *
 * Returns LABEL that is jumped to if condition is Condition
 *
 * Some conditions clobber AH
 */

char *ConditionDecode(int mode, int Condition)
{
	char *Label = GenerateLabel(0,1);

	switch (mode)
	{

		case 0:	/* A - Always */
			if (Condition)
			{
				fprintf(fp, "\t\t jmp   near %s\n",Label);
			}
			break;

		case 1:	/* F - Never */
			if (!Condition)
			{
				fprintf(fp, "\t\t jmp   near %s\n",Label);
			}
			break;

		case 2:	/* Hi */
			fprintf(fp, "\t\t mov   ah,dl\n");
			fprintf(fp, "\t\t sahf\n");

			if (Condition)
			{
				fprintf(fp, "\t\t ja    near %s\n",Label);
			}
			else
			{
				fprintf(fp, "\t\t jbe   near %s\n",Label);
			}
			break;

		case 3:	/* Ls */
			fprintf(fp, "\t\t mov   ah,dl\n");
			fprintf(fp, "\t\t sahf\n");

			if (Condition)
			{
				fprintf(fp, "\t\t jbe   near %s\n",Label);
			}
			else
			{
				fprintf(fp, "\t\t ja    near %s\n",Label);
			}
			break;

		case 4:	/* CC */
			fprintf(fp, "\t\t test  dl,1H\t\t;check carry\n");

			if (Condition)
			{
				fprintf(fp, "\t\t jz    near %s\n",Label);
			}
			else
			{
				fprintf(fp, "\t\t jnz   near %s\n",Label);
			}
			break;

		case 5:	/* CS */
			fprintf(fp,  "\t\t test  dl,1H\t\t;check carry\n");
			if (Condition)
			{
				fprintf(fp, "\t\t jnz   near %s\n",Label);
			}
			else
			{
				fprintf(fp, "\t\t jz    near %s\n",Label);
			}
			break;

		case 6:	/* NE */
			fprintf(fp, "\t\t test  dl,40H\t\t;Check zero\n");
			if (Condition)
			{
				fprintf(fp, "\t\t jz    near %s\n",Label);
			}
			else
			{
				fprintf(fp, "\t\t jnz   near %s\n",Label);
			}
			break;

		case 7:	/* EQ */
			fprintf(fp, "\t\t test  dl,40H\t\t;Check zero\n");
			if (Condition)
			{
				fprintf(fp, "\t\t jnz   near %s\n",Label);
			}
			else
			{
				fprintf(fp, "\t\t jz    near %s\n",Label);
			}
			break;

		case 8:	/* VC */
			fprintf(fp, "\t\t test  dh,8H\t\t;Check Overflow\n");
			if (Condition)
			{
				fprintf(fp, "\t\t jz    near %s\n", Label);
			}
			else
			{
				fprintf(fp, "\t\t jnz   near %s\n", Label);
			}
			break;

		case 9:	/* VS */
			fprintf(fp, "\t\t test  dh,8H\t\t;Check Overflow\n");
			if (Condition)
			{
				fprintf(fp, "\t\t jnz   near %s\n", Label);
			}
			else
			{
				fprintf(fp, "\t\t jz    near %s\n", Label);
			}
			break;

		case 10:	/* PL */
			fprintf(fp,"\t\t test  dl,80H\t\t;Check Sign\n");
			if (Condition)
			{
				fprintf(fp, "\t\t jz    near %s\n", Label);
			}
			else
			{
				fprintf(fp, "\t\t jnz   near %s\n", Label);
			}
			break;

		case 11:	/* MI */
			fprintf(fp,"\t\t test  dl,80H\t\t;Check Sign\n");
			if (Condition)
			{
				fprintf(fp, "\t\t jnz   near %s\n", Label);
			}
			else
			{
				fprintf(fp, "\t\t jz    near %s\n", Label);
			}
			break;

		case 12:	/* GE */
			fprintf(fp, "\t\t or    edx,200h\n");
			fprintf(fp, "\t\t push  edx\n");
			fprintf(fp, "\t\t popf\n");
			if (Condition)
			{
				fprintf(fp, "\t\t jge   near %s\n",Label);
			}
			else
			{
				fprintf(fp, "\t\t jl    near %s\n",Label);
			}
			break;

		case 13:	/* LT */
			fprintf(fp, "\t\t or    edx,200h\n");
			fprintf(fp, "\t\t push  edx\n");
			fprintf(fp, "\t\t popf\n");
			if (Condition)
			{
				fprintf(fp, "\t\t jl    near %s\n",Label);
			}
			else
			{
				fprintf(fp, "\t\t jge   near %s\n",Label);
			}
			break;

		case 14:	/* GT */
			fprintf(fp, "\t\t or    edx,200h\n");
			fprintf(fp, "\t\t push  edx\n");
			fprintf(fp, "\t\t popf\n");
			if (Condition)
			{
				fprintf(fp, "\t\t jg    near %s\n",Label);
			}
			else
			{
				fprintf(fp, "\t\t jle   near %s\n",Label);
			}
			break;

		case 15:	/* LE */
			fprintf(fp, "\t\t or    edx,200h\n");
			fprintf(fp, "\t\t push  edx\n");
			fprintf(fp, "\t\t popf\n");
			if (Condition)
			{
				fprintf(fp, "\t\t jle   near %s\n",Label);
			}
			else
			{
				fprintf(fp, "\t\t jg    near %s\n",Label);
			}
			break;
	}

	return Label;
}

/*
 * mode = condition to check for
 * SetWhat = text for assembler command (usually AL or address descriptor)
 *
 * Some conditions clobber AH
 */

void ConditionCheck(int mode, char *SetWhat)
{
	switch (mode)
	{

		case 0:	/* A - Always */
			fprintf(fp, "\t\t mov   %s,byte 0ffh\n",SetWhat);
			break;

		case 1:	/* F - Never */
			if (SetWhat[1] == 'L')
			{
				ClearRegister(EAX);
			}
			else
			{
				fprintf(fp, "\t\t mov   %s,byte 0h\n",SetWhat);
			}
			break;

		case 2:	/* Hi */
			fprintf(fp, "\t\t mov   ah,dl\n");
			fprintf(fp, "\t\t sahf\n");
			fprintf(fp, "\t\t seta  %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 3:	/* Ls */
			fprintf(fp, "\t\t mov   ah,dl\n");
			fprintf(fp, "\t\t sahf\n");
			fprintf(fp, "\t\t setbe %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 4:	/* CC */
			fprintf(fp, "\t\t test  dl,1\t\t;Check Carry\n");
			fprintf(fp, "\t\t setz  %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 5:	/* CS */
			fprintf(fp, "\t\t test  dl,1\t\t;Check Carry\n");
			fprintf(fp, "\t\t setnz %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 6:	/* NE */
			fprintf(fp, "\t\t test  dl,40H\t\t;Check Zero\n");
			fprintf(fp, "\t\t setz  %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 7:	/* EQ */
			fprintf(fp, "\t\t test  dl,40H\t\t;Check Zero\n");
			fprintf(fp, "\t\t setnz %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 8:	/* VC */
			fprintf(fp, "\t\t test  dh,8H\t\t;Check Overflow\n");
			fprintf(fp, "\t\t setz  %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 9:	/* VS */
			fprintf(fp, "\t\t test  dh,8H\t\t;Check Overflow\n");
			fprintf(fp, "\t\t setnz %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 10:	/* PL */
			fprintf(fp, "\t\t test  dl,80H\t\t;Check Sign\n");
			fprintf(fp, "\t\t setz  %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 11:	/* MI */
			fprintf(fp, "\t\t test  dl,80H\t\t;Check Sign\n");
			fprintf(fp, "\t\t setnz %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 12:	/* GE */
			fprintf(fp, "\t\t or    edx,200h\n");
			fprintf(fp, "\t\t push  edx\n");
			fprintf(fp, "\t\t popf\n");
			fprintf(fp, "\t\t setge %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 13:	/* LT */
			fprintf(fp, "\t\t or    edx,200h\n");
			fprintf(fp, "\t\t push  edx\n");
			fprintf(fp, "\t\t popf\n");
			fprintf(fp, "\t\t setl  %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 14:	/* GT */
			fprintf(fp, "\t\t or    edx,200h\n");
			fprintf(fp, "\t\t push  edx\n");
			fprintf(fp, "\t\t popf\n");
			fprintf(fp, "\t\t setg  %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
			break;

		case 15:	/* LE */
			fprintf(fp, "\t\t or    edx,200h\n");
			fprintf(fp, "\t\t push  edx\n");
			fprintf(fp, "\t\t popf\n");
			fprintf(fp, "\t\t setle %s\n",SetWhat);
			fprintf(fp, "\t\t neg   byte %s\n",SetWhat);
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

void dump_imm( int type, int leng, int mode, int sreg )
{
	int Opcode,BaseCode ;
	char Size=' ' ;
	char * RegnameEBX="" ;
	char * Regname="" ;
	char * OpcodeName[16] = {"or ", "and", "sub", "add",0,"xor","cmp",0} ;
	int allow[] = {1,0,1,1, 1,1,1,1, 1,0,0,0, 0,0,0,0, 0,0,0,1, 1} ;

	Opcode = (type << 9) | ( leng << 6 ) | ( mode << 3 ) | sreg;

	BaseCode = Opcode & 0xfff8;

	if (mode == 7) BaseCode |= sreg ;

#ifdef A7ROUTINE
	if ((leng == 0) && (sreg == 7) && (mode > 2) && (mode < 5))
	{
		BaseCode |= sreg ;
	}
#endif

	if (type != 4)	/* Not Valid (for this routine) */
	{
		int Dest = EAtoAMN(Opcode, FALSE);
		int SetX;

		/* ADDI & SUBI also set X flag */

		SetX = ((type == 2) || (type == 3));

		switch (leng)
		{
			case 0:
				Size = 'B';
				Regname = regnamesshort[0];
				RegnameEBX = regnamesshort[EBX];
				break;
			case 1:
				Size = 'W';
				Regname = regnamesword[0];
				RegnameEBX = regnamesword[EBX];
				break;
			case 2:
				Size = 'L';
				Regname = regnameslong[0];
				RegnameEBX = regnameslong[EBX];
				break;
		}

		if (allow[Dest])
		{
			if (OpcodeArray[BaseCode] == -2)
			{
				Align();
				fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

				/* Save Previous PC if Memory Access */

				if ((Dest >= 2) && (Dest <=10))
					SavePreviousPC();

				fprintf(fp, "\t\t add   esi,byte 2\n\n");

				if (mode < 2)
				{
					if (Size != 'L')
						TimingCycles += (CPU==2) ? 4 : 8;
					else
					{
						TimingCycles += (CPU==2) ? 8 : 14;
						if ((type != 1) && (type!=6))
							TimingCycles += 2 ;
					}
				}
				else
				{
					if (type != 6)
					{
						if (Size != 'L')
							TimingCycles += (CPU==2) ? 4 : 12 ;
						else
							TimingCycles += (CPU==2) ? 4 : 20 ;
					}
					else
					{
						if (Size != 'L')
							TimingCycles += (CPU==2) ? 4 : 8 ;
						else
							TimingCycles += (CPU==2) ? 4 : 12 ;
					}
				}

				fprintf(fp, "\t\t and   ecx,byte 7\n");

				/* Immediate Mode Data */
				EffectiveAddressRead(11,Size,EBX,EBX,"--C-S-B",FALSE);

				/* Source Data */
				EffectiveAddressRead(Dest,Size,ECX,EAX,"-BC-SDB",FALSE);

				/* The actual work */
				fprintf(fp, "\t\t %s   %s,%s\n", OpcodeName[type], Regname, RegnameEBX );

				SetFlags(Size,EAX,FALSE,SetX,TRUE);

				if (type != 6) /* CMP no update */
					EffectiveAddressWrite(Dest,Size,ECX,EAX,"---DS-B",FALSE);

				Completed();
			}
		}
		else
		{
			/* Logicals are allowed to alter SR/CCR */

			if ((!SetX) && (Dest == 11) && (Size != 'L') && (type != 6))
			{
				Align();

				fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
				fprintf(fp, "\t\t add   esi,byte 2\n\n");
				TimingCycles += 20 ;

				if (Size=='W')
				{
					/* If SR then must be in Supervisor Mode */

					char *Label = GenerateLabel(0,1);

					fprintf(fp, "\t\t test  byte [%s],20h \t\t\t; Supervisor Mode ?\n",REG_SRH);
					fprintf(fp, "\t\t jne   near %s\n\n",Label);

					/* User Mode - Exception */

					Exception(8,BaseCode);

					fprintf(fp, "%s:\n",Label);
				}

				/* Immediate Mode Data */
				EffectiveAddressRead(11,Size,EBX,EBX,"---DS-B",TRUE);

				ReadCCR(Size,ECX);

				fprintf(fp, "\t\t %s   %s,%s\n", OpcodeName[type], Regname, RegnameEBX );

				WriteCCR(Size);

				Completed();
			}
			else
			{

				/* Illegal Opcode */

				OpcodeArray[BaseCode] = -1;
				BaseCode = -1;
			}
		}
	}
	else
	{
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
	char *EAXReg,*ECXReg, *Label ;
	char allow[] = "0-2345678-------" ;
	int Dest ;

	/* BTST allows x(PC) and x(PC,xr.s) - others do not */

	if (type == 0)
	{
		allow[9]  = '9';
		allow[10] = 'a';
		allow[11] = 'b'; // dave fix to nhl
	}

	Opcode = 0x0100 | (sreg << 9) | (type<<6) | (mode<<3) | dreg ;

	BaseCode = Opcode & 0x01f8 ;
	if (mode == 7) BaseCode |= dreg ;


	// A7+, A7-

#ifdef  A7ROUTINE
	if ((mode > 2) && (mode < 5))
	{
		if (dreg == 7) BaseCode |= dreg;
	}
#endif

	Dest = EAtoAMN(Opcode, FALSE);

	if (allow[Dest&0xf] != '-')
	{
		if (mode == 0) /* long*/
		{
			/* Modify register memory directly */

			Size = 'L' ;
			EAXReg = REG_DAT_EBX;
			ECXReg = regnameslong[ECX];
		}
		else
		{
			Size = 'B' ;
			EAXReg = regnamesshort[EAX];
			ECXReg = regnamesshort[ECX];
		}

		if (OpcodeArray[BaseCode] == -2)
		{
			Align();
			fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

			/* Save Previous PC if Memory Access */

			if ((Dest >= 2) && (Dest <=10))
				SavePreviousPC();

			fprintf(fp, "\t\t add   esi,byte 2\n\n");

			if (mode<2)
			{
				switch (type)
				{
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
			}
			else
			{
				if (type==0)
					TimingCycles += 4;
				else
					TimingCycles += 8;
			}

			/* Only need this sorted out if a register is involved */

			if (Dest < 7)
			{
				fprintf(fp, "\t\t mov   ebx,ecx\n");
				fprintf(fp, "\t\t and   ebx,byte 7\n");
			}

			/* Get bit number and create mask in ECX */

			fprintf(fp, "\t\t shr   ecx, byte 9\n");
			fprintf(fp, "\t\t and   ecx, byte 7\n");
			fprintf(fp, "\t\t mov   ecx, [%s+ECX*4]\n",REG_DAT);

			if (Size == 'L')
				fprintf(fp, "\t\t and   ecx, byte 31\n");
			else
				fprintf(fp, "\t\t and   ecx, byte 7\n");

		 #ifdef QUICKZERO
		 	fprintf(fp,"\t\t mov   eax,1\n");
		 #else
				fprintf(fp,"\t\t xor   eax,eax\n");
				fprintf(fp,"\t\t inc   eax\n");
		 #endif

			fprintf(fp,"\t\t shl   eax,cl\n");
			fprintf(fp,"\t\t mov   ecx,eax\n");

			if (mode != 0)
				EffectiveAddressRead(Dest,Size,EBX,EAX,"-BCDSDB",TRUE);


			/* All commands copy existing bit to Zero Flag */

			Label = GenerateLabel(0,1);

			fprintf(fp,"\t\t or    edx,byte 40h\t; Set Zero Flag\n");
			fprintf(fp,"\t\t test  %s,%s\n",EAXReg,ECXReg);
			fprintf(fp,"\t\t jz    short %s\n",Label);
			fprintf(fp,"\t\t xor   edx,byte 40h\t; Clear Zero Flag\n");
			fprintf(fp,"%s:\n",Label);

			/* Some then modify the data */

			switch (type)
			{
				case 0: /* btst*/
					break;

				case 1: /* bchg*/
					fprintf(fp,"\t\t xor   %s,%s\n",EAXReg,ECXReg);
					break;

				case 2: /* bclr*/
					fprintf(fp,"\t\t not   ecx\n");
					fprintf(fp,"\t\t and   %s,%s\n",EAXReg,ECXReg);
					break;

				case 3: /* bset*/
					fprintf(fp,"\t\t or    %s,%s\n",EAXReg,ECXReg);
					break;
			}

			if ((mode !=0) && (type != 0))
				EffectiveAddressWrite(Dest,Size,EBX,FALSE,"---DS-B",TRUE);

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

void dump_bit_static(int type, int mode, int dreg )
{
	int  Opcode, BaseCode ;
	char Size ;
	char *EAXReg,*ECXReg, *Label ;
	char allow[] = "0-2345678-------" ;
	int Dest ;

	/* BTST allows x(PC) and x(PC,xr.s) - others do not */

	if (type == 0)
	{
		allow[9] = '9';
		allow[10] = 'a';
	}

	Opcode = 0x0800 | (type<<6) | (mode<<3) | dreg ;
	BaseCode = Opcode & 0x08f8 ;
	if (mode == 7) BaseCode |= dreg ;

	// A7+, A7-

#ifdef  A7ROUTINE
	if ((mode > 2) && (mode < 5))
	{
		if (dreg == 7) BaseCode |= dreg;
	}
#endif

	Dest = EAtoAMN(Opcode, FALSE);

	if (allow[Dest&0xf] != '-')
	{
		if (mode == 0) /* long*/
		{
			/* Modify register memory directly */

			Size = 'L' ;
			EAXReg = REG_DAT_EBX;
			ECXReg = regnameslong[ECX];
		}
		else
		{
			Size = 'B' ;
			EAXReg = regnamesshort[EAX];
			ECXReg = regnamesshort[ECX];
		}

		if (OpcodeArray[BaseCode] == -2)
		{
			Align();
			fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

			/* Save Previous PC if Memory Access */

			if ((Dest >= 2) && (Dest <=10))
				SavePreviousPC();

			fprintf(fp, "\t\t add   esi,byte 2\n\n");

			if (mode<2)
			{
				switch (type)
				{
					case 0:
						TimingCycles += 10 ;
						break ;
					case 1:
					case 3:
						TimingCycles += 12 ;
						break ;
					case 2:
						TimingCycles += 14 ;
						break ;
				}
			}
			else
			{
				if (type != 0)
					TimingCycles += 12 ;
				else
					TimingCycles += 8 ;
			}

			/* Only need this sorted out if a register is involved */

			if (Dest < 7)
			{
				fprintf(fp, "\t\t mov   ebx,ecx\n");
				fprintf(fp, "\t\t and   ebx, byte 7\n");
			}

			/* Get bit number and create mask in ECX */

			Memory_Fetch('W',ECX,FALSE);
			fprintf(fp, "\t\t add   esi,byte 2\n");

			if (Size == 'L')
				fprintf(fp, "\t\t and   ecx, byte 31\n");
			else
				fprintf(fp, "\t\t and   ecx, byte 7\n");

		 #ifdef QUICKZERO
		 	fprintf(fp,"\t\t mov   eax,1\n");
		 #else
				fprintf(fp,"\t\t xor   eax,eax\n");
				fprintf(fp,"\t\t inc   eax\n");
		 #endif

			fprintf(fp,"\t\t shl   eax,cl\n");
			fprintf(fp,"\t\t mov   ecx,eax\n");

			if (mode != 0)
				EffectiveAddressRead(Dest,Size,EBX,EAX,"-BCDSDB",TRUE);

			/* All commands copy existing bit to Zero Flag */

			Label = GenerateLabel(0,1);

			fprintf(fp,"\t\t or    edx,byte 40h\t; Set Zero Flag\n");
			fprintf(fp,"\t\t test  %s,%s\n",EAXReg,ECXReg);
			fprintf(fp,"\t\t jz    short %s\n",Label);
			fprintf(fp,"\t\t xor   edx,byte 40h\t; Clear Zero Flag\n");
			fprintf(fp,"%s:\n",Label);

			/* Some then modify the data */

			switch (type)
			{
				case 0: /* btst*/
					break;

				case 1: /* bchg*/
					fprintf(fp,"\t\t xor   %s,%s\n",EAXReg,ECXReg);
					break;

				case 2: /* bclr*/
					fprintf(fp,"\t\t not   ecx\n");
					fprintf(fp,"\t\t and   %s,%s\n",EAXReg,ECXReg);
					break;

				case 3: /* bset*/
					fprintf(fp,"\t\t or    %s,%s\n",EAXReg,ECXReg);
					break;
			}

			if ((mode !=0) && (type != 0))
				EffectiveAddressWrite(Dest,Size,EBX,FALSE,"---DS-B",TRUE);

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
	int sreg,dir,leng,dreg ;
	int	Opcode, BaseCode ;

	for (sreg = 0 ; sreg < 8 ; sreg++)
	{
		for (dir = 0 ; dir < 2 ; dir++)
		{
			for (leng = 0 ; leng < 2 ; leng++)
			{
				for (dreg = 0 ; dreg < 8 ; dreg++)
				{
					Opcode = 0x0108 | (sreg<<9) | (dir<<7) | (leng<<6) | dreg;
					BaseCode = Opcode & 0x01c8 ;
					if (OpcodeArray[BaseCode] == -2)
					{
						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
						SavePreviousPC();
						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						if (leng == 0) /* word */
							TimingCycles += 16 ;
						else
							TimingCycles += 24 ;

						/* Save Flags Register (so we only do it once) */

						fprintf(fp, "\t\t push  edx\n");

						fprintf(fp, "\t\t mov   ebx,ecx\n");
						fprintf(fp, "\t\t and   ebx,byte 7\n");

						/* Get Address to Read/Write in EDI */

						EffectiveAddressCalculate(5,'L',EBX,FALSE);

						fprintf(fp, "\t\t shr   ecx,byte 9\n");
						fprintf(fp, "\t\t and   ecx,byte 7\n");


						if (dir == 0) /* from memory to register*/
						{
							Memory_Read('B',EDI,"-BC-SDB",2);		/* mask first call */
							fprintf(fp,"\t\t mov   bh,al\n");
							fprintf(fp,"\t\t add   edi,byte 2\n");
							Memory_Read('B',EDI,"-BC-SDB",0);		/* not needed then */
							fprintf(fp,"\t\t mov   bl,al\n");

							if (leng == 0) /* word d(Ax) into Dx.W*/
							{
								fprintf(fp,"\t\t mov   [%s+ecx*4],bx\n",REG_DAT);
							}
							else /* long d(Ax) into Dx.L*/
							{
								fprintf(fp,"\t\t add   edi,byte 2\n");
								fprintf(fp,"\t\t shl   ebx,16\n");
								Memory_Read('B',EDI,"-BC-SDB",0);
								fprintf(fp,"\t\t mov   bh,al\n");
								fprintf(fp,"\t\t add   edi,byte 2\n");
								Memory_Read('B',EDI,"-BC-S-B",0);
								fprintf(fp,"\t\t mov   bl,al\n");
								fprintf(fp,"\t\t mov   [%s+ecx*4],ebx\n",REG_DAT);
							}
						}
						else /* Register to Memory*/
						{
							fprintf(fp,"\t\t mov   eax,[%s+ecx*4]\n",REG_DAT);

							/* Move bytes into Line */

							if (leng == 1)
								fprintf(fp,"\t\t rol   eax,byte 8\n");
							else
								fprintf(fp,"\t\t rol   eax,byte 24\n");

							Memory_Write('B',EDI,EAX,"A---SDB",2); /* Mask First */
							fprintf(fp,"\t\t add   edi,byte 2\n");
							fprintf(fp,"\t\t rol   eax,byte 8\n");

							if (leng == 1) /* long*/
							{
								Memory_Write('B',EDI,EAX,"A---SDB",0);
								fprintf(fp,"\t\t add   edi,byte 2\n");
								fprintf(fp,"\t\t rol   eax,byte 8\n");
								Memory_Write('B',EDI,EAX,"A---SDB",0);
								fprintf(fp,"\t\t add   edi,byte 2\n");
								fprintf(fp,"\t\t rol   eax,byte 8\n");
							}
							Memory_Write('B',EDI,EAX,"A---S-B",0);
						}

						fprintf(fp, "\t\t pop   edx\n");
						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
		}
	}
}

void movecodes(int allowfrom[],int allowto[],int Start,char Size) /* MJC */
{
	int Opcode;
	int Src,Dest;
	int SaveEDX;
	int BaseCode;

	for (Opcode=Start;Opcode<Start+0x1000;Opcode++)
	{
		/* Mask our Registers */

		BaseCode = Opcode & (Start + 0x1f8);

		/* Unless Mode = 7 */

		if ((BaseCode & 0x38)  == 0x38)  BaseCode |= (Opcode & 7);
		if ((BaseCode & 0x1c0) == 0x1c0) BaseCode |= (Opcode & 0xE00);

		/* If mode = 3 or 4 and Size = byte and register = A7 */
		/* then make it a separate code							  */

#ifdef  A7ROUTINE
		if (Size == 'B')
		{
			if (((Opcode & 0x3F) == 0x1F) || ((Opcode & 0x3F) == 0x27))
			{
				BaseCode |= 0x07;
			}

			if (((Opcode & 0xFC0) == 0xEC0) || ((Opcode & 0xFC0) == 0xF00))
			{
				BaseCode |= 0x0E00;
			}
		}
#endif

		/* If Source = Data or Address register - combine into same routine */

		if (((Opcode & 0x38) == 0x08) && (allowfrom[1]))
		{
			BaseCode &= 0xfff7;
		}

		if (OpcodeArray[BaseCode] == -2)
		{
			Src  = EAtoAMN(Opcode, FALSE);
			Dest = EAtoAMN(Opcode >> 6, TRUE);

			if ((allowfrom[(Src & 15)]) && (allowto[(Dest & 15)]))
			{
				/* If we are not going to calculate the flags */
				/* we need to preserve the existing ones		*/

				SaveEDX = (Dest == 1);

				Align();
				fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

				if (((Src >= 2) && (Src <= 10)) || ((Dest >= 2) && (Dest <=10)))
					SavePreviousPC();

				fprintf(fp, "\t\t add   esi,byte 2\n\n");


				TimingCycles += 4 ;

				if (Src < 7)
				{
					if (Dest < 7)
					{
						fprintf(fp, "\t\t mov   ebx,ecx\n");

						if ((Src == 0) && allowfrom[1])
							fprintf(fp, "\t\t and   ebx,byte 15\n");
						else
							fprintf(fp, "\t\t and   ebx,byte 7\n");

						EffectiveAddressRead(Src,Size,EBX,EAX,"--CDS-B",SaveEDX);
					}
					else
					{
						if ((Src == 0) && allowfrom[1])
							fprintf(fp, "\t\t and   ecx,byte 15\n");
						else
							fprintf(fp, "\t\t and   ecx,byte 7\n");

						EffectiveAddressRead(Src,Size,ECX,EAX,"---DS-B",SaveEDX);
					}
				}
				else
				{
					if (Dest < 7)
						EffectiveAddressRead(Src,Size,EBX,EAX,"--CDS-B",SaveEDX);
					else
						EffectiveAddressRead(Src,Size,EBX,EAX,"---DS-B",SaveEDX);
				}

				/* No flags if Destination Ax */

				if (!SaveEDX)
				{
					SetFlags(Size,EAX,TRUE,FALSE,TRUE);
				}

				if (Dest < 7)
				{
					fprintf(fp, "\t\t shr   ecx,9\n");
					fprintf(fp, "\t\t and   ecx,byte 7\n");
				}

				EffectiveAddressWrite(Dest,Size,ECX,TRUE,"---DS-B",SaveEDX);

				Completed();
			}
			else
			{
				BaseCode = -1; /* Invalid Code */
			}
		}
		else
		{
			BaseCode = OpcodeArray[BaseCode];
		}

		if (OpcodeArray[Opcode] < 0)
			OpcodeArray[Opcode] = BaseCode;
	}
}

void moveinstructions(void)
{
	int allowfrom[] = {1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0};
	int allowto[]	= {1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0};

	/* For Byte */

	movecodes(allowfrom,allowto,0x1000,'B');

	/* For Word & Long */

	allowto[1] = 1;
	movecodes(allowfrom,allowto,0x2000,'L');
	movecodes(allowfrom,allowto,0x3000,'W');
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

	int allowtoScc[]	= {1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0};
	int allowtoADDQ[]  = {1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0};
	int Opcode,BaseCode;
	char Label[32];
	char Label2[32];
	char Size=' ';
	char* Regname="";
	char* RegnameECX="";

	for (Opcode = 0x5000;Opcode < 0x6000;Opcode++)
	{
		if ((Opcode & 0xc0) == 0xc0)
		{
			/* Scc or DBcc */

			BaseCode = Opcode & 0x5FF8;
			if ((BaseCode & 0x38) == 0x38) BaseCode |= (Opcode & 7);

			/* If mode = 3 or 4 and register = A7 */
			/* then make it a separate code		 */

#ifdef  A7ROUTINE
			if (((Opcode & 0x3F) == 0x1F) || ((Opcode & 0x3F) == 0x27))
			{
				BaseCode |= 0x07;
			}
#endif

			if (OpcodeArray[BaseCode] == -2)
			{
				OpcodeArray[BaseCode] = BaseCode;

				if ((BaseCode & 0x38) == 0x8)
				{
					/* DBcc */

					Align();
					fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

					TimingCycles += 10 ;


					strcpy(Label,GenerateLabel(BaseCode,1)) ;
					strcpy(Label2,ConditionDecode((Opcode >> 8) & 0x0F,TRUE));

					/* False - Decrement Counter - Loop if not -1 */

					fprintf(fp, "\t\t and   ecx,byte 7\n");
					fprintf(fp, "\t\t mov   ax,[%s+ecx*4]\n",REG_DAT);
					fprintf(fp, "\t\t dec   ax\n");
					fprintf(fp, "\t\t mov   [%s+ecx*4],ax\n",REG_DAT);
					fprintf(fp, "\t\t inc   ax\t\t; Is it -1\n");
					fprintf(fp, "\t\t jz    short %s\n",Label);

					fprintf(fp, "\t\t add   esi,byte 2\n\n");
					Memory_Fetch('W',EAX,TRUE);
					fprintf(fp, "\t\t add   esi,eax\n");
					Completed();

					/* True - Exit Loop */
					fprintf(fp, "%s:\n",Label);

					fprintf(fp, "%s:\n",Label2);
					fprintf(fp, "\t\t add   esi,byte 4\n");
					TimingCycles += 2 ;

					Completed();
				}
				else
				{
					/* Scc */

					int  Dest = EAtoAMN(Opcode, FALSE);

					if (allowtoScc[(Dest & 15)])
					{
						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

						if ((Dest >= 2) && (Dest <=10))
							SavePreviousPC();

						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						if (Dest > 1)
							TimingCycles += 8 ;
						else
							TimingCycles += 4 ;

						if (Dest < 7)
						{
							fprintf(fp, "\t\t and   ecx,byte 7\n");
						}

						if (Dest > 1)
						{
							EffectiveAddressCalculate(Dest,'B',ECX,TRUE);
	/* ASG - no longer need to mask addresses here */
/*							fprintf(fp,"\t\t and   edi,0FFFFFFh\n");*/
						}

						ConditionCheck((Opcode >> 8) & 0x0F,"AL");

						EffectiveAddressWrite(Dest,'B',ECX,FALSE,"---DS-B",TRUE);

						/* take advantage of AL being 0 for false, 0xff for true */
						/* need to add 2 cycles if register and condition is true */

						if (Dest == 0)
						{
							fprintf(fp, "\t\t and   eax,byte 2\n");
							fprintf(fp, "\t\t add   eax,byte %d\n",TimingCycles);
							fprintf(fp, "\t\t sub   dword [%s],eax\n",ICOUNT);

							TimingCycles = -1;
						}
						Completed();
					}
					else
					{
						OpcodeArray[BaseCode] = -1;
						BaseCode = -1;
					}
				}
			}
			else
			{
				BaseCode = OpcodeArray[BaseCode];
			}

			OpcodeArray[Opcode] = BaseCode;
		}
		else
		{
			/* ADDQ or SUBQ */

			BaseCode = Opcode & 0x51F8;
			if ((BaseCode & 0x38) == 0x38) BaseCode |= (Opcode & 7);

			/* Special for Address Register Direct - Force LONG */

			if ((Opcode & 0x38) == 0x8) BaseCode = ((BaseCode & 0xFF3F) | 0x80);


			/* If mode = 3 or 4 and Size = byte and register = A7 */
			/* then make it a separate code							  */

#ifdef  A7ROUTINE
			if ((Opcode & 0xC0) == 0)
			{
				if (((Opcode & 0x3F) == 0x1F) || ((Opcode & 0x3F) == 0x27))
				{
					BaseCode |= 0x07;
				}
			}
#endif

			if (OpcodeArray[BaseCode] == -2)
			{
				char *Operation;
				int Dest = EAtoAMN(Opcode, FALSE);
				int SaveEDX = (Dest == 1);

				if (allowtoADDQ[(Dest & 15)])
				{
					switch (BaseCode & 0xC0)
					{
						case 0:
							Size = 'B';
							Regname = regnamesshort[0];
							RegnameECX = regnamesshort[ECX];
							break;

						case 0x40:
							Size = 'W';
							Regname = regnamesword[0];
							RegnameECX = regnamesword[ECX];
							break;

						case 0x80:
							Size = 'L';
							Regname = regnameslong[0];
							RegnameECX = regnameslong[ECX];
							break;
					}

					OpcodeArray[BaseCode] = BaseCode;

					Align();
					fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

					if ((Dest >= 2) && (Dest <=10))
						SavePreviousPC();

					fprintf(fp, "\t\t add   esi,byte 2\n\n");

					if (Dest == 0) /* write to Dx */
					{
						if (Size != 'L')
							TimingCycles += 4 ;
						else
							TimingCycles += 8 ;
					}

					if (Dest == 1)
					{
						if ((Size == 'L') || (Opcode & 0x100)) /* if long or SUBQ */
							TimingCycles += 8 ;
						else
							TimingCycles += 4 ;
					}

					if (Dest > 1) /* write to mem */
					{
						if (Size != 'L')
							TimingCycles += 8 ;
						else
							TimingCycles += 12 ;
					}

					if (Dest < 7)
					{
						fprintf(fp, "\t\t mov   ebx,ecx\n");
						fprintf(fp, "\t\t and   ebx,byte 7\n");
					}

					if (Dest > 1)
					{
						EffectiveAddressRead(Dest,Size,EBX,EAX,"-BCDSDB",SaveEDX);
					}

					/* Sub Immediate from Opcode */

					fprintf(fp, "\t\t shr   ecx,9\n");

					Immediate8();

					if (Opcode & 0x100)
					{
						/* SUBQ */
						Operation = "sub";
					}
					else
					{
						/* ADDQ */
						Operation = "add";
					}

					/* For Data or Address register, operate directly */
					/* on the memory location. Don't load into EAX	 */

					if (Dest < 2)
					{
						if (Dest == 0)
						{
							fprintf(fp, "\t\t %s   [%s+ebx*4],%s\n",Operation,REG_DAT,RegnameECX);
						}
						else
						{
							fprintf(fp, "\t\t %s   [%s+ebx*4],%s\n",Operation,REG_ADD,RegnameECX);
						}
					}
					else
					{
						fprintf(fp, "\t\t %s   %s,%s\n",Operation,Regname,RegnameECX);
					}

					/* No Flags for Address Direct */

					if (!SaveEDX)
					{
						/* Directly after ADD or SUB, so test not needed */

						SetFlags(Size,EAX,FALSE,TRUE,TRUE);
					}

					if (Dest > 1)
					{
						EffectiveAddressWrite(Dest,Size,EBX,FALSE,"---DS-B",FALSE);
					}

					Completed();
				}
				else
				{
					OpcodeArray[BaseCode] = -1;
					BaseCode = -1;
				}
			}
			else
			{
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
	char jmpLabel[40] ;

	for (Opcode = 0x60;Opcode < 0x70;Opcode++)
	{
		/* Displacement = 0 -> 16 Bit displacement */

		BaseCode = Opcode * 0x100;
		OpcodeArray[BaseCode] = BaseCode;

		Align();
		fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
		fprintf(fp, "\t\t add   esi,byte 2\n\n");

		TimingCycles += 10 ;

		if (Opcode == 0x60)
		{
			Memory_Fetch('W',EAX,TRUE);
			fprintf(fp, "\t\t add   esi,eax\n");
			MemoryBanking(BaseCode);
			Completed();
		}
		else
		{
			if (Opcode != 0x61)
			{
				Label = ConditionDecode(Opcode & 0x0F,TRUE);

				/* Code for Failed branch */

				fprintf(fp, "\t\t add   esi,byte 2\n");

				/* 2 less cycles for Failure */

				TimingCycles -= 2;
				Completed();
				TimingCycles += 2;

				/* Successful Branch */

				Align();
				fprintf(fp, "%s:\n",Label);

				Memory_Fetch('W',EAX,TRUE);
				fprintf(fp, "\t\t add   esi,eax\n");
				MemoryBanking(BaseCode+2);
				Completed();

			}
			else
			{
				/* BSR - Special Case */

				TimingCycles += 8 ;

				Memory_Fetch('W',EBX,TRUE);
				fprintf(fp, "\t\t add   ebx,esi\n");

				fprintf(fp, "\t\t add   esi,byte 2\n");
				PushPC(ECX,EAX,"-B-DS-B",1);

				fprintf(fp, "\t\t mov   esi,ebx\n");
				MemoryBanking(BaseCode+3);
				Completed();
			}
		}

		/* 8 Bit Displacement */

		Align();
		fprintf(fp, "%s:\n",GenerateLabel(BaseCode+1,0));
		fprintf(fp, "\t\t add   esi,byte 2\n\n");

		TimingCycles += 10 ;

		if (Opcode > 0x60)
		{
			if (Opcode != 0x61)
			{
				Label = ConditionDecode(Opcode & 0x0F,TRUE);

				/* Code for Failed branch */

				TimingCycles -= 2;
				Completed();
				TimingCycles += 2;

				/* Successful Branch */

				Align();
				fprintf(fp, "%s:\n",Label);
			}
			else
			{
				/* BSR - Special Case */

				TimingCycles += 8 ;

				PushPC(EDI,EBX,"--CDS-B",1);
			}
		}

		/* Common Ending */

		fprintf(fp, "\t\t movsx eax,cl               ; Sign Extend displacement\n");
		fprintf(fp, "\t\t add   esi,eax\n");
		MemoryBanking(BaseCode+5);
		Completed();

		/* Fill up Opcode Array */

		for (Counter=1;Counter<0x100;Counter++)
			OpcodeArray[BaseCode+Counter] = BaseCode+1;

		if(CPU==2)
		{

			/* 8 bit 0xff & 68020 instruction - 32 bit displacement */

		  Align();
		  fprintf(fp, "%s:\n",GenerateLabel(BaseCode+0xff,0));
		  sprintf( jmpLabel, GenerateLabel(BaseCode+0xff,1) ) ;
		  fprintf(fp, "\t\t add   esi,byte 2\n\n");

		  TimingCycles += 10 ;

		  if (Opcode == 0x60)
		  {
			  /* bra - always branch */
			  Memory_Fetch('L',EAX,FALSE);
			  fprintf(fp, "\t\t add   esi,eax\n");
			  MemoryBanking(BaseCode+6);
			  Completed();
		  }
		  else
		  {
			  if (Opcode != 0x61)
			  {
				  Label = ConditionDecode(Opcode & 0x0F,TRUE);

				  /* Code for Failed branch */
				  fprintf(fp, "\t\t add   esi,byte 4\n");

				  TimingCycles -= 2;
				  Completed();
				  TimingCycles += 2;

				  /* Successful Branch */
				  Align();
				  fprintf(fp, "%s:\n",Label);

				  Memory_Fetch('L',EAX,FALSE);
				  fprintf(fp, "\t\t add   esi,eax\n");
				  MemoryBanking(BaseCode+8);
				  Completed();
			  }
			  else
			  {
				  /* BSR - Special Case */

				  TimingCycles += 8 ;

				  Memory_Fetch('L',EBX,TRUE);
				  fprintf(fp, "\t\t add   ebx,esi\n");

				  fprintf(fp, "\t\t add   esi,byte 4\n");
				  PushPC(ECX,EAX,"-B-DS-B",1);

				  fprintf(fp, "\t\t mov   esi,ebx\n");
				  MemoryBanking(BaseCode+9);
				  Completed();
			  }
		  }

		  OpcodeArray[BaseCode+0xff] = BaseCode+0xff;
		}
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
	int Count;

	/* The Code */

	Align();
	fprintf(fp, "%s:\n",GenerateLabel(0x7000,0));
	fprintf(fp, "\t\t add   esi,byte 2\n\n");

	TimingCycles += 4 ;

	fprintf(fp, "\t\t movsx eax,cl\n");
	fprintf(fp, "\t\t shr   ecx,9\n");
	fprintf(fp, "\t\t and   ecx,byte 7\n");
	SetFlags('L',EAX,TRUE,FALSE,FALSE);
	EffectiveAddressWrite(0,'L',ECX,TRUE,"---DS-B",FALSE);
	Completed();

	/* Set OpcodeArray (Not strictly correct, since some are illegal!) */

	for (Count=0x7000;Count<0x8000;Count++)
	{
		OpcodeArray[Count] = 0x7000;
	}
}

/*
 * Extended version of Add & Sub commands
 *
 */

void addx_subx(void)
{
	int	Opcode, BaseCode ;
	int	regx,type,leng,rm,regy,mode ;
	int ModeModX;
	int ModeModY;
	char  Size=' ' ;
	char * Regname="" ;
	char * RegnameEBX="" ;
	char * Operand="";
	char * Label;

	for (type = 0 ; type < 2 ; type ++) /* 0=subx, 1=addx */
		for (regx = 0 ; regx < 8 ; regx++)
			for (leng = 0 ; leng < 3 ; leng++)
				for (rm = 0 ; rm < 2 ; rm++)
					for (regy = 0 ; regy < 8 ; regy++)
					{
						Opcode = 0x9100 | (type<<14) | (regx<<9) | (leng<<6) | (rm<<3) | regy ;

						BaseCode = Opcode & 0xd1c8 ;

						ModeModX = 0;
						ModeModY = 0;

#ifdef A7ROUTINE
						if ((rm == 1) && (leng == 0))
						{
							if (regx == 7)
							{
								BaseCode |= (regx << 9);
								ModeModY = 16;
							}
							if (regy == 7)
							{
								BaseCode |= regy;
								ModeModX = 16;
							}
						}
#endif

						if (rm == 0)
							mode = 0 ;
						else
							mode = 4 ;

						switch (leng)
						{
							case 0:
								Size = 'B';
								Regname = regnamesshort[0];
								RegnameEBX = regnamesshort[EBX];
								break;
							case 1:
								Size = 'W';
								Regname = regnamesword[0];
								RegnameEBX = regnamesword[EBX];
								break;
							case 2:
								Size = 'L';
								Regname = regnameslong[0];
								RegnameEBX = regnameslong[EBX];
								break;
						}

						if (OpcodeArray[BaseCode] == -2)
						{
							if (type == 0)
								Operand = "sbb";
							else
								Operand = "adc";

							Align();
							fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

							if (mode == 4)
								SavePreviousPC();

							fprintf(fp, "\t\t add   esi,byte 2\n\n");

							/* don't add in EA timing for ADDX,SUBX */

							AddEACycles = 0 ;

							if (rm == 0) /* reg to reg */
							{
								if (Size != 'L')
									TimingCycles += 4 ;
								else
									TimingCycles += 8 ;
							}
							else
							{
								if (Size != 'L')
									TimingCycles += 18 ;
								else
									TimingCycles += 30 ;
							}

							fprintf(fp, "\t\t mov   ebx,ecx\n");
							fprintf(fp, "\t\t and   ebx, byte 7\n");
							fprintf(fp, "\t\t shr   ecx, byte 9\n");
							fprintf(fp, "\t\t and   ecx, byte 7\n");

							/* Get Source */

							EffectiveAddressRead(mode+ModeModX,Size,EBX,EBX,"--CDS-B",FALSE);

							/* Get Destination (if needed) */

							if (mode == 4)
								EffectiveAddressRead(mode+ModeModY,Size,ECX,EAX,"-BCDSDB",FALSE);

							/* Copy the X flag into the Carry Flag */

							CopyX();

							/* Do the sums */

							if (mode == 0)
								fprintf(fp, "\t\t %s   [%s+ecx*4],%s\n",Operand,REG_DAT,RegnameEBX);
							else
								fprintf(fp, "\t\t %s   %s,%s\n",Operand,Regname,RegnameEBX);

							/* Preserve old Z flag */

							fprintf(fp, "\t\t mov   ebx,edx\n");

							/* Set the Flags */

							SetFlags(Size,EAX,FALSE,TRUE,FALSE);

							/* Handle the Z flag */

							Label = GenerateLabel(0,1);

							fprintf(fp, "\t\t jnz   short %s\n\n",Label);

							fprintf(fp, "\t\t and   dl,0BFh       ; Remove Z\n");
							fprintf(fp, "\t\t and   bl,40h        ; Mask out Old Z\n");
							fprintf(fp, "\t\t or    dl,bl         ; Copy across\n\n");
							fprintf(fp, "%s:\n",Label);

							/* Update the Data (if needed) */

							if (mode == 4)
								EffectiveAddressWrite(mode,Size,ECX,FALSE,"---DS-B",TRUE);

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

void dumpx( int start, int reg, int type, char * Op, int dir, int leng, int mode, int sreg )
{
	int Opcode,BaseCode ;
	char Size=' ' ;
	char * RegnameECX="" ;
	char * Regname="" ;
	int Dest ;
	int SaveEDX ;
	int SaveDir;
	char * allow="" ;
	char * allowtypes[] = { "0-23456789ab----", "--2345678-------",
		"0123456789ab----", "0-2345678-------"};

	SaveDir = dir;

	switch (type)
	{
		case 0: /* or and*/
			if (dir == 0)
				allow = allowtypes[0];
			else
				allow = allowtypes[1];
			break ;

		case 1: /* cmp*/
			allow = allowtypes[2] ;
			break ;

		case 2: /* eor*/
			allow = allowtypes[3] ;
			break ;

		case 3: /* adda suba cmpa*/
			allow = allowtypes[2] ;
			break ;

		case 4: /* sub add*/
			if (dir == 0)
				allow = allowtypes[0] ;
			else
				allow = allowtypes[1] ;
			break ;
	}

	if ((type == 4) && (dir == 0) && (leng > 0))
	{
		allow = allowtypes[2] ; /* word and long ok*/
	}

	Opcode = start | (reg << 9 ) | (dir<<8) | (leng<<6) | (mode<<3) | sreg;

	BaseCode = Opcode & 0xf1f8;

	if (mode == 7) BaseCode |= sreg ;

#ifdef A7ROUTINE
	if ((mode == 3 || mode == 4) && ( leng == 0 ) && (sreg == 7 ))
		BaseCode |= sreg ;
#endif



	/* If Source = Data or Address register - combine into same routine */

	if (((Opcode & 0x38) == 0x08) && (allow[1] != '-'))
	{
		BaseCode &= 0xfff7;
	}

	Dest = EAtoAMN(Opcode, FALSE);
	SaveEDX = (Dest == 1) || (type == 3);

	if (allow[Dest&0xf] != '-')
	{
		if (OpcodeArray[BaseCode] == -2)
		{
			switch (leng)
			{
				case 0:
					Size = 'B';
					Regname = regnamesshort[0];
					RegnameECX = regnamesshort[ECX];
					break;
				case 1:
					Size = 'W';
					Regname = regnamesword[0];
					RegnameECX = regnamesword[ECX];
					break;
				case 2:
					Size = 'L';
					Regname = regnameslong[0];
					RegnameECX = regnameslong[ECX];
					break;

				case 3: /* cmpa adda suba */
					if (dir == 0)
					{
						Size = 'W';
						Regname = regnamesword[0];
						RegnameECX = regnamesword[ECX];
					}
					else
					{
						Size = 'L';
						Regname = regnameslong[0];
						RegnameECX = regnameslong[ECX];
					}
					dir = 0 ;
					break ;
			}

			Align();
			fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

			if ((Dest >= 2) && (Dest <=10))
				SavePreviousPC();

			fprintf(fp, "\t\t add   esi,byte 2\n\n");

			if (dir==0)
			{
				if (Size != 'L')
					TimingCycles += 4;
				else
					TimingCycles += 6;
			}
			else
			{
				if (Size != 'L')
					TimingCycles += 8;
				else
					TimingCycles += 12;
			}

			if ((mode == 0) && (dir==0) && (Size == 'L'))
				TimingCycles += 2 ;

			if ((mode == 1) && (dir==0) && (Size != 'L'))
				TimingCycles += 4 ;

			if (Dest < 7)	 /* Others do not need reg.no. */
			{
				fprintf(fp, "\t\t mov   ebx,ecx\n");

				if ((Dest == 0) & (allow[1] != '-'))
					fprintf(fp, "\t\t and   ebx,byte 15\n");
				else
					fprintf(fp, "\t\t and   ebx,byte 7\n");
			}

			fprintf(fp, "\t\t shr   ecx,byte 9\n");
			fprintf(fp, "\t\t and   ecx,byte 7\n");

			EffectiveAddressRead(Dest,Size,EBX,EAX,"-BCDSDB",SaveEDX);

			if (dir == 0)
			{
				if (type != 3)
				{
					fprintf(fp, "\t\t %s   [%s+ECX*4],%s\n",Op ,REG_DAT ,Regname ) ;

					if (type == 4)
						SetFlags(Size,EAX,FALSE,TRUE,FALSE);
					else
						SetFlags(Size,EAX,FALSE,FALSE,FALSE);
				}
				else
				{
					if (Size == 'W')
						fprintf(fp, "\t\t cwde\n");

					fprintf(fp, "\t\t %s   [%s+ECX*4],EAX\n",Op ,REG_ADD);

					if (Op[0] == 'c')
					{
						SetFlags('L',EAX,FALSE,FALSE,FALSE);
					}
				}
			}
			else
			{
				fprintf(fp, "\t\t %s   %s,[%s+ECX*4]\n", Op, Regname ,REG_DAT ) ;

				if (type == 4)
					SetFlags(Size,EAX,FALSE,TRUE,TRUE);
				else
					SetFlags(Size,EAX,FALSE,FALSE,TRUE);

				EffectiveAddressWrite(Dest,Size,EBX,FALSE,"---DS-B",FALSE);
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
						dumpx( 0x9000, reg, 4, "sub", dir, leng, mode, sreg ) ;

		/* suba */

		for (dir = 0 ; dir < 2 ; dir++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
					dumpx( 0x9000, reg, 3, "sub", dir, 3, mode, sreg ) ;


		/* cmp */
		for (leng = 0 ; leng < 3; leng++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
					dumpx( 0xb000, reg, 1, "cmp", 0, leng, mode, sreg ) ;

		/* cmpa */

		for (dir = 0 ; dir < 2 ; dir++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
					dumpx( 0xb000, reg, 3, "cmp", dir, 3, mode, sreg ) ;

		/* adda */

		for (dir = 0 ; dir < 2 ; dir++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
					dumpx( 0xd000, reg, 3, "add", dir, 3, mode, sreg ) ;


		/* eor */
		for (leng = 0 ; leng < 3; leng++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
					dumpx( 0xb100, reg, 2, "xor", 1, leng, mode, sreg ) ;

		/* and */
		for (dir = 0 ; dir < 2 ; dir++)
			for (leng = 0 ; leng < 3; leng++)
				for (mode = 0 ; mode < 8 ; mode++)
					for (sreg = 0 ; sreg < 8 ; sreg++)
						dumpx( 0xc000, reg, 0, "and", dir, leng, mode, sreg ) ;

		/* add  */
		for (dir = 0 ; dir < 2 ; dir++)
			for (leng = 0 ; leng < 3; leng++)
				for (mode = 0 ; mode < 8 ; mode++)
					for (sreg = 0 ; sreg < 8 ; sreg++)
						dumpx( 0xd000, reg, 4, "add", dir, leng, mode, sreg ) ;
	}
}

/*
 * Single commands missed out by routines above
 *
 */

void mul(void)
{
	int dreg, type, mode, sreg ;
	int Opcode, BaseCode ;
	int Dest ;
	char allow[] = "0-23456789ab-----" ;

	for (dreg = 0 ; dreg < 8 ; dreg++)
		for (type = 0 ; type < 2 ; type++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
				{
					Opcode = 0xc0c0 | (dreg<<9) | (type<<8) | (mode<<3) | sreg ;
					BaseCode = Opcode & 0xc1f8 ;
					if (mode == 7)
					{
						BaseCode |= sreg ;
					}

					Dest = EAtoAMN(Opcode, FALSE);
					if (allow[Dest&0x0f] != '-')
					{
						if (OpcodeArray[ BaseCode ] == -2)
						{
							Align();
							fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

							if ((Dest >= 2) && (Dest <=10))
								SavePreviousPC();

							fprintf(fp, "\t\t add   esi,byte 2\n\n");

							TimingCycles += 54 ;

							if (mode < 7)
							{
								fprintf(fp, "\t\t mov   ebx,ecx\n");
								fprintf(fp, "\t\t and   ebx,byte 7\n");
							}

							fprintf(fp, "\t\t shr   ecx, byte 9\n");
							fprintf(fp, "\t\t and   ecx, byte 7\n");

							EffectiveAddressRead(Dest,'W',EBX,EAX,"ABCDSDB",FALSE);

							if (type == 0)
								fprintf(fp, "\t\t mul   word [%s+ECX*4]\n",REG_DAT);
							else
								fprintf(fp, "\t\t imul  word [%s+ECX*4]\n",REG_DAT);

							fprintf(fp, "\t\t shl   edx, byte 16\n");
							fprintf(fp, "\t\t mov   dx,ax\n");
							fprintf(fp, "\t\t mov   [%s+ECX*4],edx\n",REG_DAT);
							SetFlags('L',EDX,TRUE,FALSE,FALSE);
							Completed();
						}

						OpcodeArray[Opcode] = BaseCode ;
					}
				}
}

void mull(void)
{
	int mode, sreg ;
	int Opcode, BaseCode ;
	int Dest ;
	char allow[] = "0-23456789ab-----" ;
	char *Label = NULL ;

	for (mode = 0 ; mode < 8 ; mode++)
		for (sreg = 0 ; sreg < 8 ; sreg++)
		{
			Opcode = 0x4c00 | (mode<<3) | sreg ;
			BaseCode = Opcode & 0x4c38 ;
			if (mode == 7)
			{
				BaseCode |= sreg ;
			}

			Dest = EAtoAMN(Opcode, FALSE);
			if (allow[Dest&0x0f] != '-')
			{
				if (OpcodeArray[ BaseCode ] == -2)
				{
					TimingCycles += 70 ;
					Align();
					Label = GenerateLabel(BaseCode,0);
					fprintf(fp, "%s:\n",Label);
					if ((Dest >= 2) && (Dest <=10))
						SavePreviousPC();

					fprintf(fp, "\t\t add   esi,byte 2\n\n");

					if (mode < 7)
					{
						fprintf(fp, "\t\t and   ecx,byte 7\n");
					}

					Memory_Fetch('W', EBX, FALSE );							// fetch the next word
					fprintf(fp, "\t\t add   esi,byte 2\n\n");

					EffectiveAddressRead(Dest,'L',ECX,EAX,"ABCDSDB",FALSE);	// read from the EA

					fprintf(fp, "\t\t mov   ecx,ebx\n");					// save 2nd word in ecx
					fprintf(fp, "\t\t shr   ebx,12\n");						// ebx = Dl register
					fprintf(fp, "\t\t and   ebx,7\n");						// 0-7

					Label = GenerateLabel(BaseCode,1);

					fprintf(fp, "\t\t test  ecx,0x0800\n");					// signed/unsigned?
					fprintf(fp, "\t\t jz    short %s\n",Label);				// skip if unsigned

					fprintf(fp, "\t\t imul   dword [%s+EBX*4]\n",REG_DAT);	// signed 32x32->64
					fprintf(fp, "\t\t jmp   short %s_1\n",Label);			// skip

				fprintf(fp, "%s:\n",Label);
					fprintf(fp, "\t\t mul  dword [%s+EBX*4]\n",REG_DAT);	// unsigned 32x32->64

				fprintf(fp, "%s_1:\n",Label);
					fprintf(fp, "\t\t mov   [%s+EBX*4],eax\n",REG_DAT);		// store Dl back

					fprintf(fp, "\t\t test  ecx,0x0400\n");					// do we care?
					fprintf(fp, "\t\t jz    short %s_2\n",Label);			// if not, skip
					fprintf(fp, "\t\t and   ecx,7\n");						// get Dh register
					fprintf(fp, "\t\t mov   [%s+ECX*4],edx\n",REG_DAT);		// store upper 32 bits
					SetFlags('L',EDX,TRUE,FALSE,FALSE);						// set the flags
					fprintf(fp, "\t\t and   edx,~0x0800\n");				// clear the overflow
					fprintf(fp, "\t\t jmp   short %s_3\n",Label);			// skip

				fprintf(fp, "%s_2:\n",Label);
					fprintf(fp, "\t\t mov   ebx,edx\n");					// save upper 32 in ebx
					SetFlags('L',EAX,TRUE,FALSE,FALSE);						// set the flags
					fprintf(fp, "\t\t sar   eax,31\n");						// eax = sign-extended
					fprintf(fp, "\t\t test  ecx,0x0800\n");					// signed/unsigned?
					fprintf(fp, "\t\t jnz   short %s_4\n",Label);			// skip if signed
					fprintf(fp, "\t\t xor   eax,eax\n");					// always use 0 for unsigned
				fprintf(fp, "%s_4:\n",Label);
					fprintf(fp, "\t\t cmp   eax,ebx\n");					// compare upper 32 against eax
					fprintf(fp, "\t\t je    short %s_3\n",Label);			// if equal to sign extension, skip
					fprintf(fp, "\t\t or    edx,0x0800\n");					// set the overflow

				fprintf(fp, "%s_3:\n",Label);
					Completed();
				}

				OpcodeArray[Opcode] = BaseCode ;
			}
		}
}

void divl(void)
{
	int mode, sreg ;
	int Opcode, BaseCode ;
	int Dest ;
	char allow[] = "0-23456789ab-----" ;
	char *Label = NULL ;

	for (mode = 0 ; mode < 8 ; mode++)
		for (sreg = 0 ; sreg < 8 ; sreg++)
		{
			Opcode = 0x4c40 | (mode<<3) | sreg ;
			BaseCode = Opcode & 0x4c78 ;
			if (mode == 7)
			{
				BaseCode |= sreg ;
			}

			Dest = EAtoAMN(Opcode, FALSE);
			if (allow[Dest&0x0f] != '-')
			{
				if (OpcodeArray[ BaseCode ] == -2)
				{
					TimingCycles += 70 ;
					Align();
					Label = GenerateLabel(BaseCode,0);
					fprintf(fp, "%s:\n",Label);
					if ((Dest >= 2) && (Dest <=10))
						SavePreviousPC();

					fprintf(fp, "\t\t push  edx\n");					// save edx
					fprintf(fp, "\t\t add   esi,byte 2\n\n");
					fprintf(fp, "\t\t and   ecx,byte 7\n");				// read from ea

					Memory_Fetch('W', EDX, FALSE );						// fetch 2nd word in ecx
					fprintf(fp, "\t\t add   esi,byte 2\n\n");

					EffectiveAddressRead(Dest,'L',ECX,EBX,"---DSDB",FALSE);

					fprintf(fp, "\t\t push  esi\n");					// save and 0 esi
					ClearRegister(ESI);

					Label = GenerateLabel(BaseCode,1);

					fprintf(fp, "\t\t test  ebx,ebx\n");				// check divisor against 0
					fprintf(fp, "\t\t jz    near %s_ZERO\n",Label);		// handle divide-by-zero
// low part always used
					fprintf(fp, "\t\t mov   ecx,edx\n");				// ecx = extension word
					fprintf(fp, "\t\t shr   edx,12\n");					// edx = Dq register
					fprintf(fp, "\t\t and   edx,7\n");					// 0-7
					fprintf(fp, "\t\t mov   eax,[%s+edx*4]\n",REG_DAT);	// eax = Dq register value

					ClearRegister(EDX);									// edx = 0
					fprintf(fp, "\t\t test  ecx,0x0400\n");				// check size
					fprintf(fp, "\t\t jz    short %s_1\n",Label);		// skip if 32-bit dividend
// high longword (64bit)
					fprintf(fp, "\t\t mov   edx,ecx\n");				// edx = extension word
					fprintf(fp, "\t\t and   edx,7\n");					// 0-7
					fprintf(fp, "\t\t mov   edx,[%s+edx*4]\n",REG_DAT);	// fetch upper 32-bits

					fprintf(fp, "\t\t test  ecx,0x0800\n");				// signed?
					fprintf(fp, "\t\t jz    near %s_3\n",Label);		// if not, skip to unsigned 64-bit
					fprintf(fp, "\t\t jmp   near %s_2\n",Label);		// skip to signed 64-bit case

				fprintf(fp, "%s_1:\n",Label);							// short case
					ClearRegister(EDX);									// clear edx
					fprintf(fp, "\t\t test  ecx,0x0800\n");				// signed?
					fprintf(fp, "\t\t jz    short %s_3\n",Label);		// if not, don't convert
					fprintf(fp, "\t\t cdq\n");							// sign extend into edx
// signed
				fprintf(fp, "%s_2:\n",Label);							// signed 32/64-bit case
					fprintf(fp, "\t\t or    esi,1\n");					// esi |= 1 to indicate signed
					fprintf(fp, "\t\t test  ebx,ebx\n");				// check divisor sign
					fprintf(fp, "\t\t jge   short %s_2b\n",Label);		// if >= 0, don't set
					fprintf(fp, "\t\t or    esi,2\n");					// esi |= 2 to indicate negative divisor
					fprintf(fp, "\t\t neg   ebx\n");					// make positive
				fprintf(fp, "%s_2b:\n",Label);
					fprintf(fp, "\t\t test  edx,edx\n");				// check dividend sign
					fprintf(fp, "\t\t jge   short %s_3\n",Label);		// if >= 0, don't set
					fprintf(fp, "\t\t push  ebx\n");					// save ebx
					fprintf(fp, "\t\t push  ecx\n");					// save ecx
					ClearRegister(EBX);									// clear ebx
					ClearRegister(ECX);									// clear ecx
					fprintf(fp, "\t\t sub   ebx,eax\n");				// ebx = 0 - eax
					fprintf(fp, "\t\t sbb   ecx,edx\n");				// ecx = 0 - edx
					fprintf(fp, "\t\t mov   eax,ebx\n");				// eax = ebx
					fprintf(fp, "\t\t mov   edx,ecx\n");				// edx = ecx
					fprintf(fp, "\t\t pop   ecx\n");					// restore ecx
					fprintf(fp, "\t\t pop   ebx\n");					// restore ebx
					fprintf(fp, "\t\t or    esi,4\n");					// esi |= 4 to indicate negative dividend
// unsigned
				fprintf(fp, "%s_3:\n",Label);							// unsigned 32/64-bit case
					fprintf(fp, "\t\t cmp   ebx,edx\n");				// check ebx against upper 32 bits
					fprintf(fp, "\t\t jbe   near %s_OVERFLOW\n",Label);	// generate overflow
					fprintf(fp, "\t\t div   ebx\n");					// do the divide
					fprintf(fp, "\t\t test  esi,esi\n");				// see if we need to post process
					fprintf(fp, "\t\t jz    short %s_4\n",Label);		// if not, skip
					fprintf(fp, "\t\t jpo   short %s_4\n",Label);		// if PO (pos*pos or neg*neg), leave the result
					fprintf(fp, "\t\t neg   eax\n");					// negate the result

// store results
				fprintf(fp, "%s_4:\n",Label);
					fprintf(fp, "\t\t mov   ebx,ecx\n");				// ebx = extension word
					fprintf(fp, "\t\t and   ebx,7\n");					// get Dr in ebx
					fprintf(fp, "\t\t shr   ecx,12\n");					// ecx = Dq
					fprintf(fp, "\t\t and   ecx,7\n");					// 0-7
					fprintf(fp, "\t\t mov   [%s+ebx*4],edx\n",REG_DAT);	// store remainder first
					fprintf(fp, "\t\t mov   [%s+ecx*4],eax\n",REG_DAT);	// store quotient second
					fprintf(fp, "\t\t pop   esi\n");					// restore esi
					fprintf(fp, "\t\t pop   edx\n");					// restore edx
					SetFlags('L',EAX,TRUE,FALSE,FALSE);					// set the flags
				fprintf(fp, "%s_5:\n",Label);
					fprintf(fp, "\t\t and   edx,~1\n");					// clear the carry
					Completed();

					fprintf(fp, "%s_ZERO:\t\t ;Do divide by zero trap\n", Label);
					/* Correct cycle counter for error */
					fprintf(fp, "\t\t pop   esi\n");					// restore esi
					fprintf(fp, "\t\t pop   edx\n");					// restore edx
					fprintf(fp, "\t\t add   dword [%s],byte %d\n",ICOUNT,95);
				fprintf(fp,"\t\t jmp short %s_5\n",Label);
					Exception(5,BaseCode);

				fprintf(fp, "%s_OVERFLOW:\n",Label);
//set overflow
					fprintf(fp, "\t\t pop   esi\n");					// restore esi
					fprintf(fp, "\t\t pop   edx\n");					// restore edx
					fprintf(fp, "\t\t or    edx,0x0800\n");				// set the overflow bit
					fprintf(fp, "\t\t jmp   near %s_5\n",Label);		// done

				}

				OpcodeArray[Opcode] = BaseCode ;
			}
		}
}

void bfext(void)
{
// just bfextu/bfexts for now
	char allow[] = "0-2--56789a-----" ;
	char *Label = NULL ;
	int mode,dreg,sign,Opcode,BaseCode,Dest ;
	for (mode=0; mode<8; mode++)
		for (dreg=0; dreg<8; dreg++)
			for (sign=0; sign<2; sign++)
			{
				Opcode = 0xe9c0 | (sign<<9) | (mode<<3) | dreg ;
				BaseCode = Opcode & 0xebf8 ;
				if (mode == 7)
					BaseCode |= dreg ;
				Dest = EAtoAMN(Opcode, FALSE);
				if (allow[Dest&0xf] != '-')
				{
					if (OpcodeArray[BaseCode] == -2)
					{
						Align();
						Label = GenerateLabel(BaseCode,0);
						fprintf(fp, "%s:\n",Label);
						Label = GenerateLabel(BaseCode,1);
						if ((Dest >= 2) && (Dest <=10))
							SavePreviousPC();

						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						if (mode < 7)
						{
							fprintf(fp, "\t\t and   ecx,byte 7\n");
						}

						Memory_Fetch('W', EAX, FALSE ) ;
						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						EffectiveAddressRead(Dest,'L',ECX,EDX,"ABCDSDB",FALSE);		// edx = dword

						fprintf(fp, "\t\t mov   ecx,eax\n");
						fprintf(fp, "\t\t shr   ecx,byte 6\n");
						fprintf(fp, "\t\t test  eax,0x0800\n");
						fprintf(fp, "\t\t je    short %s_1\n",Label);
	//get offset from Dx
						fprintf(fp, "\t\t and   ecx,byte 7\n");
						fprintf(fp, "\t\t mov   ecx,[%s+ECX*4]\n",REG_DAT);
	//get offset from extension
						fprintf(fp, "%s_1:\n",Label);
						fprintf(fp, "\t\t and   ecx,31\n");							// ecx = offset
						fprintf(fp, "\t\t mov   ebx,eax\n");
						fprintf(fp, "\t\t test  eax,0x0020\n");
						fprintf(fp, "\t\t je    short %s_2\n",Label);
	//get width from Dy
						fprintf(fp, "\t\t and   ebx,byte 7\n");
						fprintf(fp, "\t\t mov   ebx,[%s+EBX*4]\n",REG_DAT);
	//get width from extension
						fprintf(fp, "%s_2:\n",Label);
	//fix 0=32
						fprintf(fp, "\t\t sub   ebx,byte 1\n");
						fprintf(fp, "\t\t and   ebx,byte 31\n");
						fprintf(fp, "\t\t add   ebx,byte 1\n");						// ebx = width
						fprintf(fp, "\t\t rol   edx,cl\n");
	// check for N
						fprintf(fp, "\t\t mov   ecx,32\n");
						fprintf(fp, "\t\t sub   ecx,ebx\n");
						fprintf(fp, "\t\t mov   ebx,edx\n");
						SetFlags('L',EBX,TRUE,FALSE,FALSE);
						if (sign)
							fprintf(fp, "\t\t sar   ebx,cl\n");
						else
							fprintf(fp, "\t\t shr   ebx,cl\n");
						fprintf(fp, "\t\t shr   eax,12\n");
						fprintf(fp, "\t\t and   eax,7\n");
						fprintf(fp, "\t\t mov   [%s+EAX*4],ebx\n",REG_DAT);
						fprintf(fp, "\t\t test  ebx,ebx\n");
						fprintf(fp, "\t\t jnz   short %s_3\n",Label);
	//zero flag
						fprintf(fp, "\t\t or    edx,40h\n");
						fprintf(fp, "%s_3:\n",Label);
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
	int	type,leng, mode, sreg ;
	int	Opcode, BaseCode ;
	int	Dest ;
	int SaveEDX=0;
	char Size=' ' ;
	char * Regname="" ;
	char * RegnameECX ;
	char * Label;

	char allow[] = "0-2345678-------" ;

	for (type = 0 ; type < 4 ; type++)
		for (leng = 0 ; leng < 3 ; leng++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
				{
					Opcode = 0x4000 | (type<<9) | (leng<<6) | (mode<<3) | sreg ;
					BaseCode = Opcode & 0x46f8 ;
					if (mode == 7)
					{
						BaseCode |= sreg ;
					}

					// A7+, A7-

#ifdef  A7ROUTINE
					if ((leng == 0) && (sreg == 7) && (mode > 2) && (mode < 5))
					{
						BaseCode |= sreg ;
					}
#endif

					Dest = EAtoAMN(Opcode, FALSE);

					if (allow[Dest&0x0f] != '-')
					{
						switch (leng)
						{
							case 0:
								Size = 'B';
								Regname = regnamesshort[0];
								RegnameECX = regnamesshort[ECX];
								break;
							case 1:
								Size = 'W';
								Regname = regnamesword[0];
								RegnameECX = regnamesword[ECX];
								break;
							case 2:
								Size = 'L';
								Regname = regnameslong[0];
								RegnameECX = regnameslong[ECX];
								break;
						}

						if (OpcodeArray[ BaseCode ] == -2)
						{
							Align();
							fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

							if ((Dest >= 2) && (Dest <=10))
								SavePreviousPC();

							fprintf(fp, "\t\t add   esi,byte 2\n\n");

							if (Size != 'L')
								TimingCycles += 4;
							else
								TimingCycles += 6;

							if (Dest < 7)
								fprintf(fp, "\t\t and   ecx,byte 7\n");

							if (type == 0) SaveEDX = TRUE;
							else SaveEDX = FALSE;

							/* CLR does not need to read source (although it does on a real 68000) */

							if (type != 1)
							{
								EffectiveAddressRead(Dest,Size,ECX,EAX,"A-CDS-B",SaveEDX);
							}

							switch (type)
							{
								case 0: /* negx */

									/* Preserve old Z flag */

									fprintf(fp, "\t\t mov   ebx,edx\n");

									CopyX();
									fprintf(fp, "\t\t adc   %s,byte 0\n", Regname ) ;
									fprintf(fp, "\t\t neg   %s\n", Regname ) ;

									/* Set the Flags */

									SetFlags(Size,EAX,FALSE,TRUE,FALSE);

									/* Handle the Z flag */

									Label = GenerateLabel(0,1);

									fprintf(fp, "\t\t jnz   short %s\n\n",Label);

									fprintf(fp, "\t\t and   edx,byte -65  ; Remove Z\n");
									fprintf(fp, "\t\t and   ebx,byte 40h  ; Mask out Old Z\n");
									fprintf(fp, "\t\t or    edx,ebx       ; Copy across\n\n");
									fprintf(fp, "%s:\n",Label);

									break;

								case 1: /* clr */
									ClearRegister(EAX);
									EffectiveAddressWrite(Dest,Size,ECX,TRUE,"----S-B",FALSE);
									fprintf(fp, "\t\t mov   edx,40H\n");
									break;

								case 2: /* neg */
									fprintf(fp, "\t\t neg   %s\n",Regname ) ;
									SetFlags(Size,EAX,FALSE,TRUE,TRUE);
									break;

								case 3: /* not */
									fprintf(fp, "\t\t xor   %s,-1\n",Regname ) ;
									SetFlags(Size,EAX,FALSE,FALSE,TRUE);
									break;
							}

							/* Update (unless CLR command) */

							if (type != 1)
								EffectiveAddressWrite(Dest,Size,ECX,FALSE,"---DS-B",TRUE);

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
		for (sreg = 0 ; sreg < 8 ; sreg++)
		{
			Opcode = 0x4e60 | ( dir << 3 ) | sreg ;
			BaseCode = Opcode & 0x4e68 ;

			if (OpcodeArray[BaseCode] == -2)
			{
				Align();
				Label = GenerateLabel(BaseCode,0);
				fprintf(fp, "%s\n", Label );
				fprintf(fp, "\t\t add   esi,byte 2\n\n");

				TimingCycles += 4;

				fprintf(fp, "\t\t test  byte [%s],20h \t\t\t; Supervisor Mode ?\n",REG_SRH);
				fprintf(fp, "\t\t jz    short OP%d_%4.4x_Trap\n",CPU,BaseCode);

				fprintf(fp, "\t\t and   ecx,7\n");

				if (dir == 0) /* reg 2 USP */
				{
					fprintf(fp, "\t\t mov   eax,[%s+ECX*4]\n",REG_ADD);
					fprintf(fp, "\t\t mov   [%s],eax\n",REG_USP);
				}
				else
				{
					fprintf(fp, "\t\t mov   eax,[%s]\n",REG_USP);
					fprintf(fp, "\t\t mov   [%s+ECX*4],eax\n",REG_ADD);
				}
				Completed();

				fprintf(fp, "OP%d_%4.4x_Trap:\n",CPU,BaseCode);
				Exception(8,BaseCode);
			}
			OpcodeArray[Opcode] = BaseCode ;
		}
}


/*
 * Check
 *
 */

void chk(void)
{
	int	dreg,mode,sreg,size ;
	int	Opcode, BaseCode ;
	int	Dest ;
	char * Label ;

	char  *allow = "0-23456789ab----" ;

	for (size = 0 ; size < (CPU==2 ? 2 : 1); size++)
		for (dreg = 0 ; dreg < 8; dreg++)
			for (mode = 0 ; mode < 8; mode++)
				for (sreg = 0 ; sreg < 8; sreg++)
				{
					if (size == 0)	/* word */
						Opcode = 0x4180 | (dreg<<9) | (mode<<3) | sreg ;
				  else			/* long */
						Opcode = 0x4100 | (dreg<<9) | (mode<<3) | sreg ;
					BaseCode = Opcode & 0x41f8 ;

					if (mode == 7)
					{
						BaseCode |= sreg ;
					}

					Dest = EAtoAMN(Opcode, FALSE);

					if (allow[Dest&0xf] != '-')
					{
						if (OpcodeArray[BaseCode] == -2)
						{
							Align();
							Label = GenerateLabel(BaseCode,0);
							fprintf(fp, "%s:\n", Label );
							fprintf(fp, "\t\t add   esi,byte 2\n\n");

							TimingCycles += 10;

							fprintf(fp, "\t\t mov   ebx,ecx\n");
							fprintf(fp, "\t\t shr   ebx,byte 9\n");
							fprintf(fp, "\t\t and   ebx,byte 7\n");

							if (Dest < 7)
								fprintf(fp, "\t\t and   ecx,byte 7\n");

							EffectiveAddressRead(Dest,(size == 0) ? 'W' : 'L',ECX,EAX,"----S-B",FALSE);

							if (size == 0)	/* word */
							{
								fprintf(fp, "\t\t movsx ebx,word [%s+EBX*4]\n",REG_DAT);
								fprintf(fp, "\t\t movsx eax,ax\n");
							}
							else			/* long */
								fprintf(fp, "\t\t mov   ebx,[%s+EBX*4]\n",REG_DAT);

							fprintf(fp, "\t\t test  ebx,ebx\n"); /* is word bx < 0 */
							fprintf(fp, "\t\t jl    near OP%d_%4.4x_Trap_minus\n",CPU,BaseCode);

							fprintf(fp, "\t\t cmp   ebx,eax\n");
							fprintf(fp, "\t\t jg    near OP%d_%4.4x_Trap_over\n",CPU,BaseCode);
							Completed();

							/* N is set if data less than zero */

							Align();
							fprintf(fp, "OP%d_%4.4x_Trap_minus:\n",CPU,BaseCode);
							fprintf(fp, "\t\t or    edx,0x0080\n");		/* N flag = 80H */
							fprintf(fp, "\t\t jmp   short OP%d_%4.4x_Trap_Exception\n",CPU,BaseCode);

							/* N is cleared if greated than compared number */

							Align();
							fprintf(fp, "OP%d_%4.4x_Trap_over:\n",CPU,BaseCode);
							fprintf(fp, "\t\t and   edx,0x007f\n");		/* N flag = 80H */

							fprintf(fp, "OP%d_%4.4x_Trap_Exception:\n",CPU,BaseCode);
							fprintf(fp, "\t\t mov   al,6\n");
							Exception(-1,0x10000+BaseCode);
							Completed();

						}

						OpcodeArray[Opcode] = BaseCode ;
					}
				}
}

void chk2(void)
{
#if 0
	int	mode,sreg,size ;
	int	Opcode, BaseCode ;
	int	Dest ;
	char * Label ;

	char  *allow = "--2--56789a-----" ;

	for (size = 0 ; size < 2; size++)
		for (mode = 0 ; mode < 8; mode++)
			for (sreg = 0 ; sreg < 8; sreg++)
			{
				Opcode = 0x00c0 | (size<<9) | (mode<<3) | sreg;
				BaseCode = Opcode & 0xfff8 ;

				if (mode == 7)
				{
					BaseCode |= sreg ;
				}

				Dest = EAtoAMN(Opcode, FALSE);

				if (allow[Dest&0xf] != '-')
				{
					if (OpcodeArray[BaseCode] == -2)
					{
						Align();
						Label = GenerateLabel(BaseCode,0);
						fprintf(fp, "%s:\n", Label );
						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						TimingCycles += 10;

						fprintf(fp, "\t\t mov   ebx,ecx\n");
						fprintf(fp, "\t\t shr   ebx,byte 9\n");
						fprintf(fp, "\t\t and   ebx,byte 7\n");

						if (Dest < 7)
							fprintf(fp, "\t\t and   ecx,byte 7\n");

						EffectiveAddressRead(Dest,'W',ECX,EAX,"----S-B",FALSE);

						if (size == 0)	/* word */
						{
							fprintf(fp, "\t\t movsx ebx,word [%s+EBX*4]\n",REG_DAT);
							fprintf(fp, "\t\t movsx eax,ax\n");
						 }
						 else			/* long */
							fprintf(fp, "\t\t mov   ebx,[%s+EBX*4]\n",REG_DAT);

						fprintf(fp, "\t\t test  ebx,ebx\n"); /* is word bx < 0 */
						fprintf(fp, "\t\t jl    near OP%d_%4.4x_Trap_minus\n",CPU,BaseCode);

						fprintf(fp, "\t\t cmp   ebx,eax\n");
						fprintf(fp, "\t\t jg    near OP%d_%4.4x_Trap_over\n",CPU,BaseCode);
						Completed();

						/* N is set if data less than zero */

						Align();
						fprintf(fp, "OP%d_%4.4x_Trap_minus:\n",CPU,BaseCode);
						fprintf(fp, "\t\t or    edx,0x0080\n");		/* N flag = 80H */
						fprintf(fp, "\t\t jmp   short OP%d_%4.4x_Trap_Exception\n",CPU,BaseCode);

						/* N is cleared if greated than compared number */

						Align();
						fprintf(fp, "OP%d_%4.4x_Trap_over:\n",CPU,BaseCode);
						fprintf(fp, "\t\t and   edx,0x007f\n");		/* N flag = 80H */

						fprintf(fp, "OP%d_%4.4x_Trap_Exception:\n",CPU,BaseCode);
						fprintf(fp, "\t\t mov   al,6\n");
						Exception(-1,0x10000+BaseCode);
						Completed();

					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
#endif
}

/*
 * Load Effective Address
 */

void LoadEffectiveAddress(void)
{
	int	Opcode, BaseCode ;
	int	sreg,mode,dreg ;
	int	Dest ;
	char allow[] = "--2--56789a-----" ;

	for (sreg = 0 ; sreg < 8 ; sreg++)
		for (mode = 0 ; mode < 8 ; mode++)
			for (dreg = 0 ; dreg < 8 ; dreg++)
			{
				Opcode = 0x41c0 | (sreg<<9) | (mode<<3) | dreg ;

				BaseCode = Opcode & 0x41f8 ;

				if (mode == 7)
					BaseCode = BaseCode | dreg ;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest&0x0f] != '-')
				{
					if (OpcodeArray[BaseCode] == -2)
					{
						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						switch (mode)
						{
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

						if (mode < 7)
						{
							fprintf(fp, "\t\t mov   ebx,ecx\n");
							fprintf(fp, "\t\t and   ebx,byte 7\n");
						}

						fprintf(fp, "\t\t shr   ecx,byte 9\n");
						fprintf(fp, "\t\t and   ecx,byte 7\n");

						EffectiveAddressCalculate(Dest,'L',EBX,TRUE);
						fprintf(fp, "\t\t mov   [%s+ECX*4],edi\n",REG_ADD);
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
	int	Opcode, BaseCode ;
	int	sreg,mode,Dest ;
	char allow[] = "0-2345678-------" ;

	for (mode = 0 ; mode < 8 ; mode++)
		for (sreg = 0 ; sreg < 8 ; sreg++)
		{
			Opcode = 0x4800 | (mode<<3) | sreg ;
			BaseCode = Opcode & 0x4838 ;

			if (mode == 7)
				BaseCode |= sreg ;

			// A7+, A7-

#ifdef  A7ROUTINE
			if ((sreg == 7) && (mode > 2) && (mode < 5))
			{
				BaseCode |= sreg;
			}
#endif

			Dest = EAtoAMN(BaseCode, FALSE);

			if (allow[Dest&0xf] != '-')
			{
				if (OpcodeArray[BaseCode] == -2)
				{
					Align();
					fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

					if ((Dest >= 2) && (Dest <=10))
						SavePreviousPC();

					fprintf(fp, "\t\t add   esi,byte 2\n\n");

					if (mode < 2)
						TimingCycles += 6;
					else
						TimingCycles += 8;

					fprintf(fp, "\t\t and   ecx, byte 7\n");

					EffectiveAddressRead(Dest,'B',ECX,EBX,"--C-SDB",FALSE);

					ClearRegister(EAX);
					CopyX();

					fprintf(fp, "\t\t sbb   al,bl\n");
					fprintf(fp, "\t\t das\n");

					SetFlags('B',EAX,FALSE,TRUE,TRUE);

					EffectiveAddressWrite(Dest,'B',ECX,EAX,"----S-B",FALSE);
					Completed();
				}
				OpcodeArray[Opcode] = BaseCode ;
			}
		}
}

void tas(void)
{
	int	Opcode, BaseCode ;
	int	sreg,mode,Dest ;
	char allow[] = "0-2345678-------" ;

	for (mode = 0 ; mode < 8 ; mode++)
		for (sreg = 0 ; sreg < 8 ; sreg++)
		{
			Opcode = 0x4ac0 | (mode<<3) | sreg ;
			BaseCode = Opcode & 0x4af8 ;

			if (mode == 7)
				BaseCode |= sreg ;

#ifdef A7ROUTINE
			if ((sreg == 7) && (mode > 2) && (mode < 5))
			{
				BaseCode |= sreg ;
			}
#endif

			Dest = EAtoAMN(BaseCode, FALSE);

			if (allow[Dest&0xf] != '-')
			{
				if (OpcodeArray[BaseCode] == -2)
				{
					Align();
					fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

					if ((Dest >= 2) && (Dest <=10))
						SavePreviousPC();

					fprintf(fp, "\t\t add   esi,byte 2\n\n");

					if (mode < 2)
						TimingCycles += 4;
					else
						TimingCycles += 14;

					fprintf(fp, "\t\t and   ecx, byte 7\n");

					EffectiveAddressRead(Dest,'B',ECX,EAX,"--C-SDB",FALSE);

					SetFlags('B',EAX,TRUE,FALSE,TRUE);
					fprintf(fp, "\t\t or    al,128\n");

					EffectiveAddressWrite(Dest,'B',ECX,EAX,"----S-B",FALSE);
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
	int	Opcode, BaseCode ;
	int	mode,dreg ;
	int	Dest ;
	char allow[] = "--2--56789a-----" ;

	for (mode = 0 ; mode < 8 ; mode++)
		for (dreg = 0 ; dreg < 8 ; dreg++)
		{
			Opcode = 0x4840 | (mode<<3) | dreg ;

			BaseCode = Opcode & 0x4878 ;

			if (mode == 7)
				BaseCode = BaseCode | dreg ;

			Dest = EAtoAMN(BaseCode, FALSE);

			if (allow[Dest&0x0f] != '-')
			{
				if (OpcodeArray[BaseCode] == -2)
				{
					Align();
					fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
					SavePreviousPC();
					fprintf(fp, "\t\t add   esi,byte 2\n\n");

					switch (mode)
					{
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

					if (mode < 7)
					{
						fprintf(fp, "\t\t and   ecx,byte 7\n");
					}

					EffectiveAddressCalculate(Dest,'L',ECX,TRUE);

					fprintf(fp, "\t\t mov   ecx,[%s]\t ; Push onto Stack\n",REG_A7);
					fprintf(fp, "\t\t sub   ecx,byte 4\n");
					fprintf(fp, "\t\t mov   [%s],ecx\n",REG_A7);
					Memory_Write('L',ECX,EDI,"---DS-B",2);
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
	int	leng, mode, sreg ;
	int	Opcode, BaseCode ;
	int	Dest ;
	char Size=' ' ;
	char * Regname ;
	char * RegnameECX ;

	char allow[] = "0-2345678-------" ;
	if (CPU==2)
		allow[1] = '1';

	for (leng = 0 ; leng < 3 ; leng++)
		for (mode = 0 ; mode < 8 ; mode++)
			for (sreg = 0 ; sreg < 8 ; sreg++)
			{
				Opcode = 0x4a00 | (leng<<6) | (mode<<3) | sreg ;
				BaseCode = Opcode & 0x4af8 ;
				if (mode == 7)
				{
					BaseCode |= sreg ;
				}

				// A7+, A7-

#ifdef  A7ROUTINE
				if ((leng == 0) && (sreg == 7) && (mode > 2) && (mode < 5))
				{
					BaseCode |= sreg ;
				}
#endif

				Dest = EAtoAMN(Opcode, FALSE);

				if ((allow[Dest&0x0f] != '-') || (( mode == 1 ) && (leng != 0)))
				{
					switch (leng)
					{
						case 0:
							Size = 'B';
							Regname = regnamesshort[0];
							RegnameECX = regnamesshort[ECX];
							break;
						case 1:
							Size = 'W';
							Regname = regnamesword[0];
							RegnameECX = regnamesword[ECX];
							break;
						case 2:
							Size = 'L';
							Regname = regnameslong[0];
							RegnameECX = regnameslong[ECX];
							break;
					}

					if (OpcodeArray[ BaseCode ] == -2)
					{
						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

						if ((Dest >= 2) && (Dest <=10))
							SavePreviousPC();

						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						TimingCycles += 4;

						if (Dest < 7)
							fprintf(fp, "\t\t and   ecx,byte 7\n");

						EffectiveAddressRead(Dest,Size,ECX,EAX,"----S-B",FALSE);

						SetFlags(Size,EAX,TRUE,FALSE,FALSE);
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
	int	leng,mode,sreg ;
	int	Opcode, BaseCode ;
	int	Dest ;
	char  Size ;
	char * Label ;

	char *allow = "--2-45678-------" ;

	for (leng = 0 ; leng < 2; leng++)
		for (mode = 0 ; mode < 8; mode++)
			for (sreg = 0 ; sreg < 8; sreg++)
			{
				Opcode = 0x4880 | ( leng<<6) | (mode<<3) | sreg ;
				BaseCode = Opcode & 0x4cf8 ;

				if (mode == 7)
				{
					BaseCode |= sreg ;
				}

				Dest = EAtoAMN(Opcode, FALSE);

				Size = "WL"[leng] ;

				if (allow[Dest&0xf] != '-')
				{
					if (OpcodeArray[BaseCode] == - 2)
					{
						Align();
						Label = GenerateLabel(BaseCode,0);
						fprintf(fp, "%s:\n",Label ) ;
						SavePreviousPC();
						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						switch (mode)
						{
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

						fprintf(fp, "\t\t push edx\n");

						Memory_Fetch('W',EDX,FALSE);
						fprintf(fp, "\t\t add   esi,byte 2\n");

						if (mode < 7)
						{
							fprintf(fp, "\t\t and   ecx,byte 7\n");
						}

						if (mode == 4)
						{
							fprintf(fp, "\t\t push  ecx\n");
							fprintf(fp, "\t\t mov   edi,[%s+ECX*4]\n",REG_ADD);
						}
						else
							EffectiveAddressCalculate(Dest,'L',ECX,TRUE);

						fprintf(fp, "\t\t mov   ebx,1\n");

						/* predecrement uses d0-d7..a0-a7  a7 first*/
						/* other modes use	a7-a0..d7-d0  d0 first*/

						if (Dest != 4)
							ClearRegister(ECX);
						else
							fprintf(fp, "\t\t mov   ecx,3Ch\n");

						fprintf(fp, "OP%d_%4.4x_Again:\n",CPU,BaseCode);
						fprintf(fp, "\t\t test  edx,ebx\n");
						fprintf(fp, "\t\t je    OP%d_%4.4x_Skip\n",CPU,BaseCode);

						fprintf(fp, "\t\t mov   eax,[%s+ecx]\n",REG_DAT);  /* load eax with current reg data */

						if (Dest == 4)
						{
							if (Size == 'W')						/* adjust pointer before write */
								fprintf(fp, "\t\t sub   edi,byte 2\n");
							else
								fprintf(fp, "\t\t sub   edi,byte 4\n");
						}

						Memory_Write(Size,EDI,EAX,"-BCDSDB",1);

						if (Dest != 4)
						{
							if (Size == 'W')					/* adjust pointer after write */
								fprintf(fp, "\t\t add   edi,byte 2\n");
							else
								fprintf(fp, "\t\t add   edi,byte 4\n");
						}

						/* Update Cycle Count */

						if (Size == 'W')
							fprintf(fp, "\t\t sub   dword [%s],byte 4\n",ICOUNT);
						else
							fprintf(fp, "\t\t sub   dword [%s],byte 8\n",ICOUNT);

						fprintf(fp, "OP%d_%4.4x_Skip:\n",CPU,BaseCode);

						if (Dest != 4)
							fprintf(fp, "\t\t add   ecx,byte 4h\n");
						else
							fprintf(fp, "\t\t sub   ecx,byte 4h\n");

						fprintf(fp, "\t\t add   ebx,ebx\n");		/* faster than shl ebx,1 */
						fprintf(fp, "\t\t test  bx,bx\n");		  /* check low 16 bits */
						fprintf(fp, "\t\t jnz   OP%d_%4.4x_Again\n",CPU,BaseCode);

						if (Dest == 4)
						{
							fprintf(fp, "\t\t pop   ecx\n");
							fprintf(fp, "\t\t mov   [%s+ECX*4],edi\n",REG_ADD);
						}

						fprintf(fp, "\t\t pop   edx\n");
						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

void movem_ea_reg(void)
{
	int	leng,mode,sreg ;
	int	Opcode, BaseCode ;
	int	Dest ;
	char  Size ;
	char * Label ;

	char  *allow = "--23-56789a-----" ;

	for (leng = 0 ; leng < 2; leng++)
		for (mode = 0 ; mode < 8; mode++)
			for (sreg = 0 ; sreg < 8; sreg++)
			{
				Opcode = 0x4c80 | ( leng<<6) | (mode<<3) | sreg ;
				BaseCode = Opcode & 0x4cf8 ;

				if (mode == 7)
				{
					BaseCode |= sreg ;
				}

				Dest = EAtoAMN(Opcode, FALSE);

				Size = "WL"[leng] ;

				if (allow[Dest&0xf] != '-')
				{
					if (OpcodeArray[BaseCode] == - 2)
					{
						Align();
						Label = GenerateLabel(BaseCode,0);

						fprintf(fp, "%s:\n",Label ) ;
						SavePreviousPC();
						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						switch (mode)
						{
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

						fprintf(fp, "\t\t push  edx\n");			 /* save edx because sr is unaffected */

						Memory_Fetch('W',EDX,FALSE);
						fprintf(fp, "\t\t add   esi,byte 2\n");

						if (mode < 7)
						{
							fprintf(fp, "\t\t and   ecx,byte 7\n");
						}

						if (mode == 3)
							fprintf(fp, "\t\t push   ecx\n");			/* if (An)+ then it needed later */

						EffectiveAddressCalculate(Dest,'L',ECX,TRUE);

						fprintf(fp, "\t\t mov   ebx,1\n");			  /* setup register list mask */

						/* predecrement uses d0-d7..a0-a7  a7 first*/
						/* other modes use	a7-a0..d7-d0  d0 first*/

						ClearRegister(ECX);								  /* always start with D0 */

						fprintf(fp, "OP%d_%4.4x_Again:\n",CPU,BaseCode);
						fprintf(fp, "\t\t test  edx,ebx\n");			/* is bit set for this register? */
						fprintf(fp, "\t\t je    OP%d_%4.4x_Skip\n",CPU,BaseCode);

						Memory_Read(Size,EDI,"-BCDSDB",1);

						if (Size == 'W')
							fprintf(fp, "\t\t cwde\n");				/* word size must be sign extended */

						fprintf(fp, "\t\t mov   [%s+ecx],eax\n",REG_DAT);  /* load current reg with eax */

						if (Size == 'W')						/* adjust pointer after write */
							fprintf(fp, "\t\t add   edi,byte 2\n");
						else
							fprintf(fp, "\t\t add   edi,byte 4\n");

						/* Update Cycle Count */

						if (Size == 'W')
							fprintf(fp, "\t\t sub   dword [%s],byte 4\n",ICOUNT);
						else
							fprintf(fp, "\t\t sub   dword [%s],byte 8\n",ICOUNT);

						fprintf(fp, "OP%d_%4.4x_Skip:\n",CPU,BaseCode);
						fprintf(fp, "\t\t add   ecx,byte 4\n");			/* adjust pointer to next reg */
						fprintf(fp, "\t\t add   ebx,ebx\n");			/* Faster than shl ebx,1 */
						fprintf(fp, "\t\t test  bx,bx\n");			  /* check low 16 bits */
						fprintf(fp, "\t\t jnz   OP%d_%4.4x_Again\n",CPU,BaseCode);

						if (mode == 3)
						{
							fprintf(fp, "\t\t pop   ecx\n");
							fprintf(fp, "\t\t mov   [%s+ECX*4],edi\n",REG_ADD);	/* reset Ax if mode = (Ax)+ */
						}

						fprintf(fp, "\t\t pop   edx\n");			 /* restore flags */
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

	for (sreg = 0 ; sreg < 8; sreg++)
	{
		Opcode = 0x4e50 | sreg ;
		BaseCode = 0x4e50 ;

		if (OpcodeArray[BaseCode] == - 2)
		{
			Align();
			fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
			SavePreviousPC();
			fprintf(fp, "\t\t add   esi,byte 2\n\n");

			TimingCycles += 16;

			fprintf(fp, "\t\t sub   dword [%s],byte 4\n",REG_A7);

			fprintf(fp, "\t\t and   ecx, byte 7\n");
			fprintf(fp, "\t\t mov   eax,[%s+ECX*4]\n",REG_ADD);
			fprintf(fp, "\t\t mov   edi,[%s]\n",REG_A7);
			fprintf(fp, "\t\t mov   [%s+ECX*4],edi\n",REG_ADD);

			Memory_Write('L',EDI,EAX,"---DS-B",1);

			Memory_Fetch('W',EAX,TRUE);
			fprintf(fp, "\t\t add   esi,byte 2\n");
			fprintf(fp, "\t\t add   [%s],eax\n",REG_A7);

			Completed();
		}

		OpcodeArray[Opcode] = BaseCode ;
	}
}

void unlinkasm(void)
{
	int	sreg ;
	int	Opcode, BaseCode ;

	for (sreg = 0 ; sreg < 8; sreg++)
	{
		Opcode = 0x4e58 | sreg ;
		BaseCode = 0x4e58 ;

		if (OpcodeArray[BaseCode] == - 2)
		{
			Align();
			fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
			SavePreviousPC();
			fprintf(fp, "\t\t add   esi,byte 2\n\n");

			TimingCycles += 12;

			fprintf(fp, "\t\t mov   ebx,ecx\n");
			fprintf(fp, "\t\t and   ebx, byte 7\n");
			fprintf(fp, "\t\t mov   edi,[%s+EBX*4]\n",REG_ADD);

			Memory_Read('L',EDI,"-B-DSDB",1);

			fprintf(fp, "\t\t mov   [%s+EBX*4],eax\n",REG_ADD);
			fprintf(fp, "\t\t add   edi,byte 4\n");
			fprintf(fp, "\t\t mov   dword [%s],EDI\n",REG_A7);
			Completed();
		}

		OpcodeArray[Opcode] = BaseCode ;
	}
}

void trap(void)
{
	int Count;
	int BaseCode = 0x4E40;

	if (OpcodeArray[BaseCode] == -2)
	{
		Align();
		fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
		fprintf(fp, "\t\t add   esi,byte 2\n\n");

		fprintf(fp, "\t\t mov   eax,ecx\n");
		fprintf(fp, "\t\t and   eax,byte 15\n");
		fprintf(fp, "\t\t or    eax,byte 32\n");
		Exception(-1,BaseCode);
		Completed();
	}

	for (Count=0;Count<=15;Count++)
		OpcodeArray[BaseCode+Count] = BaseCode;
}

void reset(void)
{
	int BaseCode = 0x4E70;
	char * Label;

	if (OpcodeArray[BaseCode] == -2)
	{
		Align();
		Label = GenerateLabel(BaseCode,0);

		TimingCycles += 132;

		fprintf(fp, "%s:\n", Label );
		SavePreviousPC();

		fprintf(fp, "\t\t add   esi,byte 2\n\n");
		fprintf(fp, "\t\t test  byte [%s],20h \t\t\t; Supervisor Mode ?\n",REG_SRH);
		fprintf(fp, "\t\t jnz   near OP%d_%4.4x_RESET\n",CPU,BaseCode);
		Exception(8,BaseCode);

		fprintf(fp, "\nOP%d_%4.4x_RESET:\n",CPU,BaseCode);

		/* Prefetch next instruction */

		if(CPU==2)
		{
			/* 32 bit memory version */

			fprintf(fp, "\t\t xor   esi,2\n");	/* ASG */
#ifdef STALLCHECK
		 ClearRegister(ECX);
			fprintf(fp, "\t\t mov   cx,[esi+ebp]\n");
#else
			fprintf(fp, "\t\t movzx ecx,word [esi+ebp]\n");
#endif
			fprintf(fp, "\t\t xor   esi,2\n");	/* ASG */
		}
		else
		{
			/* 16 bit memory */
#ifdef STALLCHECK
		 ClearRegister(ECX);
			fprintf(fp, "\t\t mov   cx,[esi+ebp]\n");
#else
			fprintf(fp, "\t\t movzx ecx,word [esi+ebp]\n");
#endif
		}

		fprintf(fp, "\t\t mov  eax,dword [%s]\n", REG_RESET_CALLBACK);
		fprintf(fp, "\t\t test eax,eax\n");
		fprintf(fp, "\t\t jz   near OP%d_%4.4x_END\n",CPU,BaseCode);

		/* Callback for Reset */

		fprintf(fp, "\t\t mov   [%s],ESI,\n",REG_PC);
		fprintf(fp, "\t\t mov   [%s],edx\n",REG_CCR);
		fprintf(fp, "\t\t push  ECX\n");

		fprintf(fp, "\t\t call  eax\n");

		fprintf(fp, "\t\t mov   ESI,[%s]\n",REG_PC);
		fprintf(fp, "\t\t mov   edx,[%s]\n",REG_CCR);
		fprintf(fp, "\t\t pop   ECX\n");
		fprintf(fp, "\t\t mov   ebp,dword [%sOP_ROM]\n", PREF);

		fprintf(fp, "OP%d_%4.4x_END:\n",CPU,BaseCode);
		fprintf(fp, "\t\t sub   dword [%s],%d\n",ICOUNT,TimingCycles);
		fprintf(fp, "\t\t jmp   [%s_OPCODETABLE+ecx*4]\n\n", CPUtype);
	}
	OpcodeArray[BaseCode] = BaseCode ;
}

void nop(void)
{
	int	BaseCode = 0x4e71 ;

	if (OpcodeArray[BaseCode] == -2)
	{
		Align();
		fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
		fprintf(fp, "\t\t add   esi,byte 2\n\n");

		TimingCycles += 4;

		Completed();
		OpcodeArray[BaseCode] = BaseCode ;
	}
}

void stop(void)
{
	char TrueLabel[16];
	int	 BaseCode = 0x4e72 ;

	if (OpcodeArray[BaseCode] == -2)
	{
		Align();
		fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
		fprintf(fp, "\t\t add   esi,byte 2\n\n");

		TimingCycles += 4;

		/* Must be in Supervisor Mode */

		sprintf(TrueLabel,GenerateLabel(0,1));

		fprintf(fp, "\t\t test  byte [%s],20h \t\t\t; Supervisor Mode ?\n",REG_SRH);
		fprintf(fp, "\t\t je    near %s\n\n",TrueLabel);

		/* Next WORD is new SR */

		Memory_Fetch('W',EAX,FALSE);
		fprintf(fp, "\t\t add   esi,byte 2\n");

		WriteCCR('W');

		/* See if Valid interrupt waiting */

		CheckInterrupt = 0;

		fprintf(fp, "\t\t mov   eax,[%s]\n",REG_IRQ);
		fprintf(fp, "\t\t and   eax,byte 07H\n");

		fprintf(fp, "\t\t cmp   al,7\t\t ; Always take 7\n");
		fprintf(fp, "\t\t je    near procint\n\n");

		fprintf(fp, "\t\t mov   ebx,[%s]\t\t; int mask\n",REG_SRH);
		fprintf(fp, "\t\t and   ebx,byte 07H\n");
		fprintf(fp, "\t\t cmp   eax,ebx\n");
		fprintf(fp, "\t\t jg    near procint\n\n");

		/* No int waiting - clear count, set stop */

		ClearRegister(ECX);
		fprintf(fp, "\t\t mov   [%s],ecx\n",ICOUNT);
		fprintf(fp, "\t\t or    byte [%s],80h\n",REG_IRQ);
		Completed();

		/* User Mode - Exception */

		Align();
		fprintf(fp, "%s:\n",TrueLabel);
		Exception(8,BaseCode);

		OpcodeArray[BaseCode] = BaseCode ;
	}
}

void ReturnFromException(void)
{
	char TrueLabel[16];

	int BaseCode = 0x4e73;

	Align();
	fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
	SavePreviousPC();
	fprintf(fp, "\t\t add   esi,byte 2\n\n");

	TimingCycles += 20;

	/* Check in Supervisor Mode */

	sprintf(TrueLabel,GenerateLabel(0,1));
	fprintf(fp, "\t\t test  byte [%s],20h \t\t\t; Supervisor Mode ?\n",REG_SRH);
	fprintf(fp, "\t\t je    near %s\n\n",TrueLabel);

	/* Get SR - Save in EBX */

	fprintf(fp, "\t\t mov   edi,[%s]\n",REG_A7);
	fprintf(fp, "\t\t add   dword [%s],byte 6\n",REG_A7);
	Memory_Read('W',EDI,"-----DB",2);
	fprintf(fp, "\t\t add   edi,byte 2\n");
	fprintf(fp, "\t\t mov   esi,eax\n");

	/* Get PC */

	Memory_Read('L',EDI,"----S-B",0);
	fprintf(fp, "\t\t xchg  esi,eax\n");

	/* Update CCR (and A7) */

	WriteCCR('W');

	MemoryBanking(BaseCode);
	Completed();

	fprintf(fp, "%s:\n",TrueLabel);
	Exception(8,0x10000+BaseCode);

	OpcodeArray[BaseCode] = BaseCode;
}

void trapv(void)
{
	int BaseCode = 0x4E76;
	char * Label;

	if (OpcodeArray[BaseCode] == -2)
	{
		Align();
		Label = GenerateLabel(BaseCode,0);
		fprintf(fp, "%s\n", Label );
		fprintf(fp, "\t\t add   esi,byte 2\n\n");

		TimingCycles += 4;

		fprintf(fp, "\t\t test  dh,08h\n");
		fprintf(fp, "\t\t jz    near OP%d_%4.4x_Clear\n",CPU,BaseCode);
		Exception(7,BaseCode);

		fprintf(fp, "OP%d_%4.4x_Clear:\n",CPU,BaseCode);
		Completed();
	}
	OpcodeArray[BaseCode] = BaseCode ;
}

void illegal_opcode(void)
{
	Align();
	fprintf(fp, "ILLEGAL:\n");
	fprintf(fp, "\t\t mov [%sillegal_op],ecx\n", PREF);
	fprintf(fp, "\t\t mov [%sillegal_pc],esi\n", PREF);

#if 0
#ifdef MAME_DEBUG
	fprintf(fp, "\t\t jmp ecx\n");
	fprintf(fp, "\t\t pushad\n");
	fprintf(fp, "\t\t call %sm68k_illegal_opcode\n", PREF);
	fprintf(fp, "\t\t popad\n");
#endif
#endif

	Exception(4,0xFFFE);
}

/*
 * Return from subroutine
 * restoring flags as well
 *
 */

void ReturnandRestore(void)
{
	int BaseCode = 0x4e77;

	Align();
	fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
	SavePreviousPC();
	fprintf(fp, "\t\t add   esi,byte 2\n\n");

	TimingCycles += 20;

	/* Get SR into ESI */

	fprintf(fp, "\t\t mov   edi,[%s]\n",REG_A7);
	fprintf(fp, "\t\t add   dword [%s],byte 6\n",REG_A7);

	Memory_Read('W',EDI,"-----DB",2);
	fprintf(fp, "\t\t add   edi,byte 2\n");
	fprintf(fp, "\t\t mov   esi,eax\n");

	/* Get PC */

	Memory_Read('L',EDI,"----SDB",0);
	fprintf(fp, "\t\t xchg  esi,eax\n");

	/* Update flags */

	WriteCCR('B');

	MemoryBanking(BaseCode);
	Completed();

	OpcodeArray[BaseCode] = BaseCode;
}

/*
 * Return from Subroutine
 *
 */

void rts(void)
{
	int	BaseCode = 0x4e75 ;

	if (OpcodeArray[BaseCode] == -2)
	{
		Align();
		fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
		SavePreviousPC();

		TimingCycles += 16;

		OpcodeArray[BaseCode] = BaseCode ;

		fprintf(fp, "\t\t mov   eax,[%s]\n",REG_A7);
		fprintf(fp, "\t\t add   dword [%s],byte 4\n",REG_A7);
		Memory_Read('L',EAX,"---D--B",1);
		fprintf(fp, "\t\t mov   esi,eax\n");
		MemoryBanking(BaseCode);
		Completed();
	}
}

void jmp_jsr(void)
{
	int	Opcode, BaseCode ;
	int	dreg,mode,type ;
	int	Dest ;
	char allow[] = "--2--56789a-----" ;

	for (type = 0 ; type < 2 ; type++)
		for (mode = 0 ; mode < 8 ; mode++)
			for (dreg = 0 ; dreg < 8 ; dreg++)
			{
				Opcode = 0x4e80 | (type<<6) | (mode<<3) | dreg ;
				BaseCode = Opcode & 0x4ef8 ;
				if (mode == 7)
					BaseCode = BaseCode | dreg ;

				Dest = EAtoAMN(BaseCode, FALSE);
				if (allow[Dest&0x0f] != '-')
				{
					if (OpcodeArray[BaseCode] == -2)
					{
						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
						SavePreviousPC();
						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						switch (mode)
						{
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

						if (mode < 7)
						{
							fprintf(fp, "\t\t and   ecx,byte 7\n");
						}
						EffectiveAddressCalculate(Dest,'L',ECX,TRUE);

						/* jsr needs to push PC onto stack */

						if (type == 0)
						{
							PushPC(EBX,EAX,"---D-DB",1);
						}

						fprintf(fp, "\t\t mov   esi,edi\n");
						MemoryBanking(BaseCode);
						Completed();
					}

					OpcodeArray[Opcode] = BaseCode ;
				}
			}
}

void cmpm(void)
{
	int	Opcode, BaseCode ;
	int	regx,leng,regy ;
	int ModeModX, ModeModY;
	char Size=' ' ;
	char * Regname="" ;
	char * RegnameEBX="" ;

	for (regx = 0 ; regx < 8 ; regx++)
		for (leng = 0 ; leng < 3 ; leng++)
			for (regy = 0 ; regy < 8 ; regy++)
			{
				Opcode = 0xb108 | (regx<<9) | (leng<<6) | regy ;
				BaseCode = Opcode & 0xb1c8 ;

				ModeModX = 0;
				ModeModY = 0;

#ifdef A7ROUTINE
				if (leng==0)
				{
					if (regx==7)
					{
						BaseCode |= (regx<<9);
						ModeModX = 16;
					}

					if (regy==7)
					{
						BaseCode |= regy;
						ModeModY = 16;
					}
				}
#endif

				switch (leng)
				{
					case 0:
						Size = 'B';
						Regname = regnamesshort[EAX];
						RegnameEBX = regnamesshort[EBX];
						break;
					case 1:
						Size = 'W';
						Regname = regnamesword[EAX];
						RegnameEBX = regnamesword[EBX];
						break;
					case 2:
						Size = 'L';
						Regname = regnameslong[EAX];
						RegnameEBX = regnameslong[EBX];
						break;
				}

				if (OpcodeArray[BaseCode] == -2)
				{
					Align();
					fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
					SavePreviousPC();
					fprintf(fp, "\t\t add   esi,byte 2\n\n");

					AddEACycles = 0 ;

					if (Size != 'L')
						TimingCycles += 12 ;
					else
						TimingCycles += 20 ;

					fprintf(fp, "\t\t mov   ebx,ecx\n");
					fprintf(fp, "\t\t and   ebx, byte 7\n");
					fprintf(fp, "\t\t shr   ecx, byte 9\n");
					fprintf(fp, "\t\t and   ecx, byte 7\n");

					EffectiveAddressRead(3+ModeModY,Size,EBX,EBX,"--C-S-B",FALSE);
					EffectiveAddressRead(3+ModeModX,Size,ECX,EAX,"-B--S-B",FALSE);

					fprintf(fp, "\t\t cmp   %s,%s\n",Regname,RegnameEBX);
					SetFlags(Size,EAX,FALSE,FALSE,FALSE);
					Completed();
				}

				OpcodeArray[Opcode] = BaseCode ;
			}
}

void exg(void)
{
	int	Opcode, BaseCode ;
	int	regx,type,regy ;
	int	opmask[3] = { 0x08, 0x09, 0x11} ;

	for (regx = 0 ; regx < 8 ; regx++)
		for (type = 0 ; type < 3 ; type++)
			for (regy = 0 ; regy < 8 ; regy++)
			{
				Opcode = 0xc100 | (regx<<9) | (opmask[type]<<3) | regy ;
				BaseCode = Opcode & 0xc1c8 ;

				if (OpcodeArray[BaseCode] == -2)
				{
					Align();
					fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
					fprintf(fp, "\t\t add   esi,byte 2\n\n");

					TimingCycles += 6 ;

					fprintf(fp, "\t\t mov   ebx,ecx\n");
					fprintf(fp, "\t\t and   ebx,byte 7\n");
					fprintf(fp, "\t\t shr   ecx,byte 9\n");
					fprintf(fp, "\t\t and   ecx,byte 7\n");

					if (type == 0)
					{
						fprintf(fp, "\t\t mov   eax,[%s+ECX*4]\n",REG_DAT);
						fprintf(fp, "\t\t mov   edi,[%s+EBX*4]\n",REG_DAT);
						fprintf(fp, "\t\t mov   [%s+ECX*4],edi\n",REG_DAT);
						fprintf(fp, "\t\t mov   [%s+EBX*4],eax\n",REG_DAT);
					}
					if (type == 1)
					{
						fprintf(fp, "\t\t mov   eax,[%s+ECX*4]\n",REG_ADD);
						fprintf(fp, "\t\t mov   edi,[%s+EBX*4]\n",REG_ADD);
						fprintf(fp, "\t\t mov   [%s+ECX*4],edi\n",REG_ADD);
						fprintf(fp, "\t\t mov   [%s+EBX*4],eax\n",REG_ADD);
					}
					if (type == 2)
					{
						fprintf(fp, "\t\t mov   eax,[%s+ECX*4]\n",REG_DAT);
						fprintf(fp, "\t\t mov   edi,[%s+EBX*4]\n",REG_ADD);
						fprintf(fp, "\t\t mov   [%s+ECX*4],edi\n",REG_DAT);
						fprintf(fp, "\t\t mov   [%s+EBX*4],eax\n",REG_ADD);
					}

					Completed();
				}

				OpcodeArray[Opcode] = BaseCode ;
			}
}

void ext(void)
{
	int	Opcode, BaseCode ;
	int	type,regy ;

	for (type = 2 ; type < 8 ; type++)
		for (regy = 0 ; regy < 8 ; regy++)
		{
			if (type > 3 && type < 7)
				continue ;
			Opcode = 0x4800 | (type<<6) | regy ;
			BaseCode = Opcode & 0x48c0 ;

			if (OpcodeArray[BaseCode] == -2)
			{
				Align();
				fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
				fprintf(fp, "\t\t add   esi,byte 2\n\n");

				TimingCycles += 4 ;

				fprintf(fp, "\t\t and   ecx, byte 7\n");

				if (type == 2) /* byte to word */
				{
					fprintf(fp, "\t\t movsx eax,byte [%s+ECX*4]\n",REG_DAT);
					fprintf(fp, "\t\t mov   [%s+ECX*4],ax\n",REG_DAT);
					SetFlags('W',EAX,TRUE,FALSE,FALSE);
				}
				if (type == 3) /* word to long */
				{
					fprintf(fp, "\t\t movsx eax,word [%s+ECX*4]\n",REG_DAT);
					fprintf(fp, "\t\t mov   [%s+ECX*4],eax\n",REG_DAT);
					SetFlags('L',EAX,TRUE,FALSE,FALSE);
				}
				if (type == 7) /* byte to long */
				{
					fprintf(fp, "\t\t movsx eax,byte [%s+ECX*4]\n",REG_DAT);
					fprintf(fp, "\t\t mov   [%s+ECX*4],eax\n",REG_DAT);
					SetFlags('L',EAX,TRUE,FALSE,FALSE);
				}
				Completed();
			}

			OpcodeArray[Opcode] = BaseCode ;
		}
}

void swap(void)
{
	int	Opcode, BaseCode ;
	int	sreg ;

	for (sreg = 0 ; sreg < 8 ; sreg++)
	{
		Opcode = 0x4840 | sreg ;
		BaseCode = Opcode & 0x4840;

		if (OpcodeArray[BaseCode] == -2)
		{
			Align();
			fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
			fprintf(fp, "\t\t add   esi,byte 2\n\n");

			TimingCycles += 4 ;

			fprintf(fp, "\t\t and   ecx, byte 7\n");
			fprintf(fp, "\t\t mov   eax, dword [%s+ECX*4]\n",REG_DAT);
			fprintf(fp, "\t\t ror   eax, 16\n");
			fprintf(fp, "\t\t test  eax,eax\n");
			fprintf(fp, "\t\t mov   dword [%s+ECX*4],eax\n",REG_DAT);
			SetFlags('L',EAX,FALSE,FALSE,FALSE);
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
	fprintf(fp, "%s:\n",GenerateLabel(0xA000,0));
	fprintf(fp, "\t\t add   esi,byte 2\n\n");
	Exception(0x0A,0xA000);

	for (Count=0xA000;Count<0xB000;Count++)
	{
		OpcodeArray[Count] = 0xA000;
	}
}

void LineF(void)
{
	int Count;

	/* Line F */

	Align();
	fprintf(fp, "%s:\n",GenerateLabel(0xF000,0));
	fprintf(fp, "\t\t add   esi,byte 2\n\n");
	Exception(0x0B,0xF000);

	for (Count=0xF000;Count<0x10000;Count++)
	{
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
	int Opcode, BaseCode ;
	int type, mode, sreg ;
	int Dest ;
	char allow[] = "0-2345678-------" ;
	char Size;

	for (type = 0 ; type < 4 ; type++)
		for (mode = 0 ; mode < 8 ; mode++)
			for (sreg = 0 ; sreg < 8 ; sreg++)
			{
				Opcode = 0x40c0 | (type << 9) | ( mode << 3 ) | sreg ;

				/* To has extra modes */

				if (type > 1)
				{
					allow[0x9] = '9';
					allow[0xa] = 'a';
					allow[0xb] = 'b' ;
				}

				if ((type == 0) | (type == 3))
					Size = 'W'; /* SR */
				else
					Size = 'B'; /* CCR */

				BaseCode = Opcode & 0x46f8 ;

				if (mode == 7)
					BaseCode |= sreg ;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest&0xf] != '-')
				{
					if (OpcodeArray[BaseCode] == -2)
					{
						char TrueLabel[16];

						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

						if ((Dest >= 2) && (Dest <=10))
							SavePreviousPC();

						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						if (type > 1) /* move to */
							TimingCycles += 12 ;
						else
						{
							if (mode < 2)
								TimingCycles += 6 ;
							else
								TimingCycles += 8 ;
						}

						/* If Move to SR then must be in Supervisor Mode */

						if (type == 3)
						{
							sprintf(TrueLabel,GenerateLabel(0,1));

							fprintf(fp, "\t\t test  byte [%s],20h \t\t\t; Supervisor Mode ?\n",REG_SRH);
							fprintf(fp, "\t\t je    near %s\n\n",TrueLabel);
						}

						/* 68010 Command ? */
						if (type==1) CheckCPUtype(1);


						if (mode < 7)
						{
							fprintf(fp, "\t\t and   ecx,byte 7\n");
						}

						/* Always read/write word 2 bytes */
						if (type < 2)
						{
							ReadCCR(Size,EBX);
							EffectiveAddressWrite(Dest & 15,'W',ECX,TRUE,"---DS-B",TRUE);
						}
						else
						{
							EffectiveAddressRead(Dest & 15,'W',ECX,EAX,"----S-B",FALSE);
							WriteCCR(Size);
						}
						Completed();

						/* Exception if not Supervisor Mode */

						if (type == 3)
						{
							/* Was in User Mode - Exception */

							fprintf(fp, "%s:\n",TrueLabel);
							Exception(8,BaseCode);
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
	int	Opcode, BaseCode ;
	int	regx,type,rm,regy,mode ;
	int ModeModX;
	int ModeModY;
	char *Label;

	for (type = 0 ; type < 2 ; type ++) /* 0=sbcd, 1=abcd */
		for (regx = 0 ; regx < 8 ; regx++)
			for (rm = 0 ; rm < 2 ; rm++)
				for (regy = 0 ; regy < 8 ; regy++)
				{
					Opcode = 0x8100 | (type<<14) | (regx<<9) | (rm<<3) | regy ;
					BaseCode = Opcode & 0xc108 ;

					ModeModX = 0;
					ModeModY = 0;

					if (rm == 0)
						mode = 0 ;
					else
					{
						mode = 4 ;

#ifdef A7ROUTINE

						if (regx == 7)
						{
							BaseCode |= (regx << 9);
							ModeModY = 16;
						}
						if (regy == 7)
						{
							BaseCode |= regy;
							ModeModX = 16;
						}

#endif
					}

					if (OpcodeArray[BaseCode] == -2)
					{
						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

						if (mode == 4)
							SavePreviousPC();

						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						AddEACycles = 0 ;

						if (rm == 0)
							TimingCycles += 6 ;
						else
							TimingCycles += 18 ;

						fprintf(fp, "\t\t mov   ebx,ecx\n");
						fprintf(fp, "\t\t and   ebx, byte 7\n");
						fprintf(fp, "\t\t shr   ecx, byte 9\n");
						fprintf(fp, "\t\t and   ecx, byte 7\n");

						EffectiveAddressRead(mode+ModeModX,'B',EBX,EBX,"--C-S-B",TRUE);
						EffectiveAddressRead(mode+ModeModY,'B',ECX,EAX,"-BC-SDB",TRUE);

						CopyX();

						if (type == 0)
						{
							fprintf(fp, "\t\t sbb   al,bl\n");
							fprintf(fp, "\t\t das\n");
						}
						else
						{
							fprintf(fp, "\t\t adc   al,bl\n");
							fprintf(fp, "\t\t daa\n");
						}

						/* Should only clear Zero flag if not zero */

						Label = GenerateLabel(0,1);

						fprintf(fp, "\t\t mov   ebx,edx\n");
						fprintf(fp, "\t\t setc  dl\n");

						fprintf(fp, "\t\t jnz   short %s\n\n",Label);

						/* Keep original Zero flag */
						fprintf(fp, "\t\t and   bl,40h        ; Mask out Old Z\n");
						fprintf(fp, "\t\t or    dl,bl         ; Copy across\n\n");

						fprintf(fp, "%s:\n",Label);

						fprintf(fp, "\t\t mov   bl,dl\n");  /* copy carry into sign */
						fprintf(fp, "\t\t and   bl,1\n");
						fprintf(fp, "\t\t shl   bl,7\n");
						fprintf(fp, "\t\t and   dl,7Fh\n");
						fprintf(fp, "\t\t or    dl,bl\n");

						fprintf(fp, "\t\t mov   [%s],edx\n",REG_X);

						EffectiveAddressWrite(mode,'B',ECX,EAX,"---DS-B",TRUE);
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
	int Opcode, BaseCode ;
	int dreg, dr, leng, ir, sreg ;
	char Size=' ';
	char * Label ;
	char * Regname="" ;
	char * RegnameECX ;

	for (dreg = 0 ; dreg < 8 ; dreg++)
		for (dr = 0 ; dr < 2 ; dr++)
			for (leng = 0 ; leng < 3 ; leng++)
				for (ir = 0 ; ir < 2 ; ir++)
					for (sreg = 0 ; sreg < 8 ; sreg++)
					{
						Opcode = 0xe018 | (dreg<<9) | (dr<<8) | (leng<<6) | (ir<<5) | sreg ;
						BaseCode = Opcode & 0xe1f8 ;

						switch (leng)
						{
							case 0:
								Size = 'B';
								Regname = regnamesshort[0];
								RegnameECX = regnamesshort[ECX];
								break;
							case 1:
								Size = 'W';
								Regname = regnamesword[0];
								RegnameECX = regnamesword[ECX];
								break;
							case 2:
								Size = 'L';
								Regname = regnameslong[0];
								RegnameECX = regnameslong[ECX];
								break;
						}

						if (OpcodeArray[ BaseCode ] == -2)
						{
							Align();
							fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
							fprintf(fp, "\t\t add   esi,byte 2\n\n");

							if (Size != 'L')
								TimingCycles += 6 ;
							else
								TimingCycles += 8 ;

							fprintf(fp, "\t\t mov   ebx,ecx\n");
							fprintf(fp, "\t\t and   ebx,byte 7\n");
							fprintf(fp, "\t\t shr   ecx,byte 9\n");

							if (ir == 0)
							{
								Immediate8();
							}
							else
							{
								fprintf(fp, "\t\t and   ecx,byte 7\n");
								EffectiveAddressRead(0,'L',ECX,ECX,"-B--S-B",FALSE);
								fprintf(fp, "\t\t and   ecx,byte 63\n");
							}

							EffectiveAddressRead(0,Size,EBX,EAX,"-BC-S-B",FALSE);

							/* shift 0 - no time, no shift and clear carry */

							Label = GenerateLabel(0,1);
							fprintf(fp, "\t\t jecxz %s\n",Label);

							/* allow 2 cycles per shift */

							fprintf(fp, "\t\t mov   edx,ecx\n");
							fprintf(fp, "\t\t add   edx,edx\n");
							fprintf(fp, "\t\t sub   dword [%s],edx\n",ICOUNT);

							if (dr == 0)
								fprintf(fp, "\t\t ror   %s,cl\n",Regname);
							else
								fprintf(fp, "\t\t rol   %s,cl\n",Regname);

							fprintf(fp, "\t\t setc  ch\n");

							fprintf(fp, "%s:\n",Label);

							SetFlags(Size,EAX,TRUE,FALSE,FALSE);
/*			fprintf(fp, "\t\t and   dl,254\n");  Test clears Carry */
							fprintf(fp, "\t\t or    dl,ch\n");

							EffectiveAddressWrite(0,Size,EBX,EAX,"--C-S-B",TRUE);

							Completed();
						}

						OpcodeArray[Opcode] = BaseCode ;
					}
}

void rol_ror_ea(void)
{
	int Opcode, BaseCode ;
	int dr, mode, sreg ;
	int Dest ;
	char allow[] = "--2345678-------" ;

	for (dr = 0 ; dr < 2 ; dr++)
		for (mode = 0 ; mode < 8 ; mode++)
			for (sreg = 0 ; sreg < 8 ; sreg++)
			{
				Opcode = 0xe6c0 | (dr<<8) | (mode<<3) | sreg ;
				BaseCode = Opcode & 0xfff8 ;

				if (mode == 7)
					BaseCode |= sreg ;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest&0xf] != '-')
				{
					if (OpcodeArray[ BaseCode ] == -2)
					{
						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

						if ((Dest >= 2) && (Dest <=10))
							SavePreviousPC();

						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						TimingCycles += 8 ;

						fprintf(fp, "\t\t and   ecx,byte 7\n");
						EffectiveAddressRead(Dest&0xf,'W',ECX,EAX,"--C-SDB",FALSE);

						if (dr == 0)
							fprintf(fp, "\t\t ror   ax,1\n");
						else
							fprintf(fp, "\t\t rol   ax,1\n");

						fprintf(fp, "\t\t setc  bl\n");
						SetFlags('W',EAX,TRUE,FALSE,FALSE);
/*				fprintf(fp, "\t\t and   dl,254\n");  Test clears Carry */
						fprintf(fp, "\t\t or    dl,bl\n");

						EffectiveAddressWrite(Dest&0xf,'W',ECX,EAX,"---DS-B",TRUE);

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
	int Opcode, BaseCode ;
	int dreg, dr, leng, ir, sreg ;
	char Size=' ';
	char * Regname="" ;
	char * RegnameECX="" ;
	char * RegnameEDX="" ;
	char * Label ;

	for (dreg = 0 ; dreg < 8 ; dreg++)
		for (dr = 0 ; dr < 2 ; dr++)
			for (leng = 0 ; leng < 3 ; leng++)
				for (ir = 0 ; ir < 2 ; ir++)
					for (sreg = 0 ; sreg < 8 ; sreg++)
					{
						Opcode = 0xe008 | (dreg<<9) | (dr<<8) | (leng<<6) | (ir<<5) | sreg ;
						BaseCode = Opcode & 0xe1f8 ;

						switch (leng)
						{
							case 0:
								Size = 'B';
								Regname = regnamesshort[0];
								RegnameECX = regnamesshort[ECX];
								RegnameEDX = regnamesshort[EDX];
								break;
							case 1:
								Size = 'W';
								Regname = regnamesword[0];
								RegnameECX = regnamesword[ECX];
								RegnameEDX = regnamesword[EDX];
								break;
							case 2:
								Size = 'L';
								Regname = regnameslong[0];
								RegnameECX = regnameslong[ECX];
								RegnameEDX = regnameslong[EDX];
								break;
						}

						if (OpcodeArray[ BaseCode ] == -2)
						{
							Align();
							fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
							fprintf(fp, "\t\t add   esi,byte 2\n\n");

							if (Size != 'L')
								TimingCycles += 6 ;
							else
								TimingCycles += 8 ;

							fprintf(fp, "\t\t mov   ebx,ecx\n");
							fprintf(fp, "\t\t and   ebx,byte 7\n");
							fprintf(fp, "\t\t shr   ecx,byte 9\n");

							if (ir == 0)
							{
								Immediate8();
							}
							else
							{
								fprintf(fp, "\t\t and   ecx,byte 7\n");
								EffectiveAddressRead(0,'L',ECX,ECX,"-B--S-B",FALSE);
								fprintf(fp, "\t\t and   ecx,byte 63\n");
							}

							/* and 2 cycles per shift */

							fprintf(fp, "\t\t mov   edx,ecx\n");
							fprintf(fp, "\t\t add   edx,edx\n");
							fprintf(fp, "\t\t sub   dword [%s],edx\n",ICOUNT);

							EffectiveAddressRead(0,Size,EBX,EAX,"-BC-S-B",FALSE);

							/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
							/* shift count is mod 32; we need to check for shifts of 32-63 */
							/* and produce zero */
							Label = GenerateLabel(0,1);
							fprintf(fp, "\t\t test  cl,0x20\n");
							fprintf(fp, "\t\t jnz   %s_BigShift\n",Label);

							fprintf(fp, "%s_Continue:\n",Label);
							if (dr == 0)
								fprintf(fp, "\t\t shr   %s,cl\n",Regname);
							else
								fprintf(fp, "\t\t shl   %s,cl\n",Regname);

							SetFlags(Size,EAX,FALSE,FALSE,FALSE);

							/* Clear Overflow flag */
							fprintf(fp, "\t\t xor   dh,dh\n");

							EffectiveAddressWrite(0,Size,EBX,EAX,"--CDS-B",TRUE);

							/* if shift count is zero clear carry */

							fprintf(fp, "\t\t jecxz %s\n",Label);

							fprintf(fp, "\t\t mov   [%s],edx\n",REG_X);
							Completed();

							Align();
							fprintf(fp, "%s:\n",Label);
							SetFlags(Size,EAX,TRUE,FALSE,FALSE);
							Completed();

							fprintf(fp, "%s_BigShift:\n",Label);
							if (dr == 0)
							{
								fprintf(fp, "\t\t shr   %s,16\n",Regname);
								fprintf(fp, "\t\t shr   %s,16\n",Regname);
							}
							else
							{
								fprintf(fp, "\t\t shl   %s,16\n",Regname);
								fprintf(fp, "\t\t shl   %s,16\n",Regname);
							}
							fprintf(fp, "\t\t jmp   %s_Continue\n",Label);
						}

						OpcodeArray[Opcode] = BaseCode ;
					}
}

void lsl_lsr_ea(void)
{
	int Opcode, BaseCode ;
	int dr, mode, sreg ;
	int Dest ;
	char allow[] = "--2345678-------" ;

	for (dr = 0 ; dr < 2 ; dr++)
		for (mode = 0 ; mode < 8 ; mode++)
			for (sreg = 0 ; sreg < 8 ; sreg++)
			{
				Opcode = 0xe2c0 | (dr<<8) | (mode<<3) | sreg ;
				BaseCode = Opcode & 0xfff8 ;

				if (mode == 7)
					BaseCode |= sreg ;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest&0xf] != '-')
				{
					if (OpcodeArray[ BaseCode ] == -2)
					{

						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

						if ((Dest >= 2) && (Dest <=10))
							SavePreviousPC();

						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						TimingCycles += 8 ;

						fprintf(fp, "\t\t and   ecx,byte 7\n");
						EffectiveAddressRead(Dest&0xf,'W',ECX,EAX,"--C-SDB",FALSE);

						if (dr == 0)
							fprintf(fp, "\t\t shr   ax,1\n");
						else
							fprintf(fp, "\t\t shl   ax,1\n");

						SetFlags('W',EAX,FALSE,TRUE,FALSE);

						/* Clear Overflow flag */

						fprintf(fp, "\t\t xor   dh,dh\n");

						EffectiveAddressWrite(Dest&0xf,'W',ECX,EAX,"---DS-B",TRUE);
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
	int Opcode, BaseCode ;
	int dreg, dr, leng, ir, sreg ;
	char Size=' ' ;
	char * Regname="" ;
	char * RegnameECX="" ;
	char * Label ;

	for (dreg = 0 ; dreg < 8 ; dreg++)
		for (dr = 0 ; dr < 2 ; dr++)
			for (leng = 0 ; leng < 3 ; leng++)
				for (ir = 0 ; ir < 2 ; ir++)
					for (sreg = 0 ; sreg < 8 ; sreg++)
					{
						Opcode = 0xe010 | (dreg<<9) | (dr<<8) | (leng<<6) | (ir<<5) | sreg ;
						BaseCode = Opcode & 0xe1f8 ;

						switch (leng)
						{
							case 0:
								Size = 'B';
								Regname = regnamesshort[0];
								RegnameECX = regnamesshort[ECX];
								break;
							case 1:
								Size = 'W';
								Regname = regnamesword[0];
								RegnameECX = regnamesword[ECX];
								break;
							case 2:
								Size = 'L';
								Regname = regnameslong[0];
								RegnameECX = regnameslong[ECX];
								break;
						}

						if (OpcodeArray[ BaseCode ] == -2)
						{
							Align();
							fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
							fprintf(fp, "\t\t add   esi,byte 2\n\n");

							if (Size != 'L')
								TimingCycles += 6 ;
							else
								TimingCycles += 8 ;

							fprintf(fp, "\t\t mov   ebx,ecx\n");
							fprintf(fp, "\t\t and   ebx,byte 7\n");
							fprintf(fp, "\t\t shr   ecx,byte 9\n");

							if (ir == 0)
							{
								Immediate8();
							}
							else
							{
								fprintf(fp, "\t\t and   ecx,byte 7\n");
								EffectiveAddressRead(0,'L',ECX,ECX,"-B--S-B",FALSE);
								fprintf(fp, "\t\t and   ecx,byte 63\n");
							}

							/* allow 2 cycles per shift */

							fprintf(fp, "\t\t mov   edx,ecx\n");
							fprintf(fp, "\t\t add   edx,edx\n");
							fprintf(fp, "\t\t sub   dword [%s],edx\n",ICOUNT);

							EffectiveAddressRead(0,Size,EBX,EAX,"-BC-SDB",FALSE);

							/* move X into C so RCR & RCL can be used */
							/* RCR & RCL only set the carry flag		*/

							CopyX();

							if (dr == 0)
								fprintf(fp, "\t\t rcr   %s,cl\n",Regname);
							else
								fprintf(fp, "\t\t rcl   %s,cl\n",Regname);

							fprintf(fp, "\t\t setc  ch\n");
							SetFlags(Size,EAX,TRUE,FALSE,FALSE);
/*			fprintf(fp, "\t\t and   dl,254\n"); Test Clears Carry */

							EffectiveAddressWrite(0,Size,EBX,EAX,"--CDS-B",TRUE);

							/* if shift count is zero clear carry */

							Label = GenerateLabel(0,1);
							fprintf(fp, "\t\t test  cl,cl\n");
							fprintf(fp, "\t\t jz    %s\n",Label);

							/* Add in Carry Flag */

							fprintf(fp, "\t\t or    dl,ch\n");
							fprintf(fp, "\t\t mov   [%s],dl\n",REG_X);
							Completed();


							/* copy X onto C when shift is zero */

							Align();
							fprintf(fp, "%s:\n",Label);
							fprintf(fp, "\t\t mov   ecx,[%s]\n",REG_X);
							fprintf(fp, "\t\t and   ecx,byte 1\n");
							fprintf(fp, "\t\t or    edx,ecx\n");
							Completed();
						}

						OpcodeArray[Opcode] = BaseCode ;
					}
}

void roxl_roxr_ea(void)
{
	int Opcode, BaseCode ;
	int dr, mode, sreg ;
	int Dest ;
	char allow[] = "--2345678-------" ;

	for (dr = 0 ; dr < 2 ; dr++)
		for (mode = 0 ; mode < 8 ; mode++)
			for (sreg = 0 ; sreg < 8 ; sreg++)
			{
				Opcode = 0xe4c0 | (dr<<8) | (mode<<3) | sreg ;
				BaseCode = Opcode & 0xfff8 ;

				if (mode == 7)
					BaseCode |= sreg ;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest&0xf] != '-')
				{
					if (OpcodeArray[ BaseCode ] == -2)
					{
						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

						if ((Dest >= 2) && (Dest <=10))
							SavePreviousPC();

						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						TimingCycles += 8 ;

						fprintf(fp, "\t\t and   ecx,byte 7\n");
						EffectiveAddressRead(Dest&0xf,'W',ECX,EAX,"--C-SDB",FALSE);

						/* move X into C so RCR & RCL can be used */
						/* RCR & RCL only set the carry flag		*/

						CopyX();

						if (dr == 0)
							fprintf(fp, "\t\t rcr   ax,1\n");
						else
							fprintf(fp, "\t\t rcl   ax,1\n");

						fprintf(fp, "\t\t setc  bl\n");
						SetFlags('W',EAX,TRUE,FALSE,FALSE);
/*				fprintf(fp, "\t\t and   dl,254\n"); - Intel Clears on Test */
						fprintf(fp, "\t\t or    dl,bl\n");

						EffectiveAddressWrite(Dest&0xf,'W',ECX,EAX,"---DS-B",TRUE);

						fprintf(fp, "\t\t mov   [%s],edx\n",REG_X);
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
	int Opcode, BaseCode ;
	int dreg, dr, leng, ir, sreg ;
	char Size=' ';
	char * Sizename="" ;
	char * Regname="" ;
	char * RegnameEDX="" ;
	char * RegnameECX="" ;
	char * Label;

	/* Normal routines for codes */

	for (dreg = 0 ; dreg < 8 ; dreg++)
		for (dr = 0 ; dr < 2 ; dr++)
			for (leng = 0 ; leng < 3 ; leng++)
				for (ir = 0 ; ir < 2 ; ir++)
					for (sreg = 0 ; sreg < 8 ; sreg++)
					{
						Opcode = 0xe000 | (dreg<<9) | (dr<<8) | (leng<<6) | (ir<<5) | sreg ;
						BaseCode = Opcode & 0xe1f8 ;

						switch (leng)
						{
							case 0:
								Size = 'B';
								Regname = regnamesshort[0];
								RegnameECX = regnamesshort[ECX];
								RegnameEDX = regnamesshort[EDX];
								break;
							case 1:
								Size = 'W';
								Regname = regnamesword[0];
								RegnameECX = regnamesword[ECX];
								RegnameEDX = regnamesword[EDX];
								break;
							case 2:
								Size = 'L';
								Regname = regnameslong[0];
								RegnameECX = regnameslong[ECX];
								RegnameEDX = regnameslong[EDX];
								break;
						}

						if (OpcodeArray[ BaseCode ] == -2)
						{
							Align();
							fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
							fprintf(fp, "\t\t add   esi,byte 2\n\n");

							Label = GenerateLabel(0,1);

							if (Size != 'L')
								TimingCycles += 6 ;
							else
								TimingCycles += 8 ;

							fprintf(fp, "\t\t mov   ebx,ecx\n");
							fprintf(fp, "\t\t and   ebx,byte 7\n");
							fprintf(fp, "\t\t shr   ecx,byte 9\n");

							EffectiveAddressRead(0,Size,EBX,EAX,"-BC-S-B",FALSE);

							if (ir == 0)
							{
								Immediate8();
							}
							else
							{
								fprintf(fp, "\t\t and   ecx,byte 7\n");
								EffectiveAddressRead(0,'L',ECX,ECX,"-B--S-B",FALSE);
								fprintf(fp, "\t\t and   ecx,byte 63\n");
								fprintf(fp, "\t\t jz   short %s\n",Label);
							}

							/* allow 2 cycles per shift */

							fprintf(fp, "\t\t mov   edx,ecx\n");
							fprintf(fp, "\t\t add   edx,edx\n");
							fprintf(fp, "\t\t sub   dword [%s],edx\n",ICOUNT);

							if (dr == 0)
							{
								/* ASR */

								/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
								/* shift count is mod 32; we need to check for shifts of 32-63 */
								/* and effectively shift 31 */
								fprintf(fp, "\t\t shrd  edx,ecx,6\n");
								fprintf(fp, "\t\t sar   edx,31\n");
								fprintf(fp, "\t\t and   edx,31\n");
								fprintf(fp, "\t\t or    ecx,edx\n");

								fprintf(fp, "\t\t sar   %s,cl\n",Regname);

								/* Mode 0 write does not affect Flags */
								EffectiveAddressWrite(0,Size,EBX,EAX,"---DS-B",TRUE);

								/* Update Flags */
								fprintf(fp, "\t\t lahf\n");

#ifdef STALLCHECK
						ClearRegister(EDX);
								fprintf(fp, "\t\t mov   dl,ah\n");
#else
								fprintf(fp, "\t\t movzx edx,ah\n");
#endif

								fprintf(fp, "\t\t mov   [%s],edx\n",REG_X);
							}
							else
							{
								/* ASL */

								/* Check to see if Overflow should be set */

								fprintf(fp,"\t\t mov   edi,eax\t\t; Save It\n");

								ClearRegister(EDX);
								fprintf(fp,"\t\t stc\n");
								fprintf(fp,"\t\t rcr   %s,1\t\t; d=1xxxx\n",RegnameEDX);
								fprintf(fp,"\t\t sar   %s,cl\t\t; d=1CCxx\n",RegnameEDX);
								fprintf(fp,"\t\t and   eax,edx\n");
								fprintf(fp,"\t\t jz    short %s_V\t\t; No Overflow\n",Label);
								fprintf(fp,"\t\t cmp   eax,edx\n");
								fprintf(fp,"\t\t je    short %s_V\t\t; No Overflow\n",Label);

								/* Set Overflow */
								fprintf(fp,"\t\t mov   edx,0x800\n");
								fprintf(fp,"\t\t jmp   short %s_OV\n",Label);

								fprintf(fp,"%s_V:\n",Label);
								ClearRegister(EDX);

								fprintf(fp,"%s_OV:\n",Label);

								/* more than 31 shifts and long */

								if ((ir==1) && (leng==2))
								{
									fprintf(fp,"\t\t test  cl,0x20\n");
									fprintf(fp,"\t\t jnz   short %s_32\n\n",Label);
								}

								fprintf(fp,"\t\t mov   eax,edi\t\t; Restore It\n");

								fprintf(fp, "\t\t sal   %s,cl\n",Regname);

								EffectiveAddressWrite(0,Size,EBX,EAX,"---DS-B",TRUE);
								fprintf(fp, "\t\t lahf\n");
								fprintf(fp, "\t\t mov   dl,ah\n");
								fprintf(fp, "\t\t mov   [%s],edx\n",REG_X);
							}
							Completed();

							if (ir != 0)
							{
								Align();
								fprintf(fp, "%s:\n",Label);


								if (dr == 0)
								{
									/* ASR - Test clears V and C */
									SetFlags(Size,EAX,TRUE,FALSE,FALSE);
								}
								else
								{
									/* ASL - Keep existing Carry flag, Clear V */
									fprintf(fp, "\t\t mov   ebx,edx\n");
									fprintf(fp, "\t\t and   ebx,byte 1\n");
									SetFlags(Size,EAX,TRUE,FALSE,FALSE);
									fprintf(fp, "\t\t or    edx,ebx\n");

									if (leng==2)
									{
										Completed();

										/* > 31 Shifts */

										fprintf(fp, "%s_32:\n",Label);
										fprintf(fp, "\t\t mov   dl,40h\n");	 // Zero flag
										ClearRegister(EAX);
										EffectiveAddressWrite(0,Size,EBX,EAX,"----S-B",TRUE);
									}
								}

								Completed();
							}
						}

						OpcodeArray[Opcode] = BaseCode ;
					}

	/* End with special routines for ASL.x #1,Dx  */
	/* To do correct V setting, ASL needs quite a */
	/* bit of additional code. A Shift of one has */
	/* correct flags on Intel, and is very common */
	/* in 68000 programs.								 */

	for (leng = 0 ; leng < 3 ; leng++)
		for (sreg = 0 ; sreg < 8 ; sreg++)
		{
			Opcode = 0xe300 | (leng<<6) | sreg ;
			BaseCode = Opcode & 0xe3c8 ;

			switch (leng)
			{
				case 0:
					Sizename = "byte";
					break;
				case 1:
					Sizename = "word";
					break;
				case 2:
					Sizename = "long";
					break;
			}

			if (sreg == 0)
			{
				Align();
				fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));
				fprintf(fp, "\t\t add   esi,byte 2\n\n");

				Label = GenerateLabel(0,1);

				if (Size != 'L')
					TimingCycles += 6 ;
				else
					TimingCycles += 8 ;

				fprintf(fp, "\t\t and   ecx,byte 7\n");
				fprintf(fp, "\t\t sal   %s [%s+ecx*4],1\n",Sizename,REG_DAT);
				SetFlags('L',EAX,FALSE,TRUE,FALSE);
				Completed();

			}

			OpcodeArray[Opcode] = BaseCode ;
		}
}

void asl_asr_ea(void)
{
	int Opcode, BaseCode ;
	int dr, mode, sreg ;
	int Dest ;
	char allow[] = "--2345678-------" ;

	for (dr = 0 ; dr < 2 ; dr++)
		for (mode = 0 ; mode < 8 ; mode++)
			for (sreg = 0 ; sreg < 8 ; sreg++)
			{
				Opcode = 0xe0c0 | (dr<<8) | (mode<<3) | sreg ;
				BaseCode = Opcode & 0xfff8 ;

				if (mode == 7)
					BaseCode |= sreg ;

				Dest = EAtoAMN(BaseCode, FALSE);

				if (allow[Dest&0xf] != '-')
				{
					if (OpcodeArray[ BaseCode ] == -2)
					{

						Align();
						fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

						if ((Dest >= 2) && (Dest <=10))
							SavePreviousPC();

						fprintf(fp, "\t\t add   esi,byte 2\n\n");

						TimingCycles += 8 ;

						fprintf(fp, "\t\t and   ecx,byte 7\n");
						EffectiveAddressRead(Dest&0xf,'W',ECX,EAX,"--C-SDB",FALSE);

						if (dr == 0)
							fprintf(fp, "\t\t sar   ax,1\n");
						else
							fprintf(fp, "\t\t sal   ax,1\n");

						SetFlags('W',EAX,FALSE,TRUE,TRUE);

						EffectiveAddressWrite(Dest&0xf,'W',ECX,EAX,"----S-B",FALSE);
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
	int dreg, type, mode, sreg ;
	int Opcode, BaseCode ;
	int Dest ;
	char allow[] = "0-23456789ab-----" ;
	char TrapLabel[16];
	int Cycles;

	int divide_cycles[12] =
	{
		38,0,42,42,44,46,50,46,50,46,48,42
	};

	for (dreg = 0 ; dreg < 8 ; dreg++)
		for (type = 0 ; type < 2 ; type++)
			for (mode = 0 ; mode < 8 ; mode++)
				for (sreg = 0 ; sreg < 8 ; sreg++)
				{
					Opcode = 0x80c0 | (dreg<<9) | (type<<8) | (mode<<3) | sreg ;
					BaseCode = Opcode & 0x81f8 ;
					if (mode == 7)
					{
						BaseCode |= sreg ;
					}

					Dest = EAtoAMN(Opcode, FALSE);
					if (allow[Dest&0x0f] != '-')
					{
						if (OpcodeArray[ BaseCode ] == -2)
						{
							Align();
							fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

							if ((Dest >= 2) && (Dest <=10))
								SavePreviousPC();

							fprintf(fp, "\t\t add   esi,byte 2\n\n");


							/* Save EDX (in case of overflow) */

							fprintf(fp, "\t\t and   edx,byte -2\n");
							fprintf(fp, "\t\t mov   [%s],edx\n",REG_CCR);


							/* Cycle Timing (if succeeds OK) */

							Cycles = divide_cycles[Dest & 0x0f] + 95 + (type * 17);

							if (Cycles > 127)
								fprintf(fp, "\t\t sub   dword [%s],%d\n",ICOUNT,Cycles);
							else
								fprintf(fp, "\t\t sub   dword [%s],byte %d\n",ICOUNT,Cycles);

							if (mode < 7)
							{
								fprintf(fp, "\t\t mov   ebx,ecx\n");
								fprintf(fp, "\t\t and   ebx,byte 7\n");
							}

							fprintf(fp, "\t\t shr   ecx, byte 9\n");
							fprintf(fp, "\t\t and   ecx, byte 7\n");

							sprintf(TrapLabel, "%s", GenerateLabel(0,1) ) ;

							EffectiveAddressRead(Dest,'W',EBX,EAX,"A-C-SDB",FALSE); /* source */

							fprintf(fp, "\t\t test  ax,ax\n");
							fprintf(fp, "\t\t je    near %s_ZERO\t\t;do div by zero trap\n", TrapLabel);

							if (type == 1) /* signed */
							{
								fprintf(fp, "\t\t movsx ebx,ax\n");
							}
							else
							{
								fprintf(fp, "\t\t movzx ebx,ax\n");
							}

							EffectiveAddressRead(0,'L',ECX,EAX,"ABC-SDB",FALSE); /* dest */

							if (type == 1) /* signed */
							{
								fprintf(fp, "\t\t cdq\n");			 /* EDX:EAX = 64 bit signed */
								fprintf(fp, "\t\t idiv  ebx\n");		 /* EBX = 32 bit */

								/* Check for Overflow */

								fprintf(fp, "\t\t movsx ebx,ax\n");
								fprintf(fp, "\t\t cmp   eax,ebx\n");
								fprintf(fp, "\t\t jne   short %s_OVER\n",TrapLabel);
							}
							else
							{
								ClearRegister(EDX);
								fprintf(fp, "\t\t div   ebx\n");

								/* Check for Overflow */

								fprintf(fp, "\t\t test  eax, 0FFFF0000H\n");
								fprintf(fp, "\t\t jnz   short %s_OVER\n",TrapLabel);
							}

							/* Sort out Result */

							fprintf(fp, "\t\t shl   edx, byte 16\n");
							fprintf(fp, "\t\t mov   dx,ax\n");
							fprintf(fp, "\t\t mov   [%s+ECX*4],edx\n",REG_DAT);
							SetFlags('W',EDX,TRUE,FALSE,FALSE);

							Completed();


							/* Overflow */

							Align();
							fprintf(fp, "%s_OVER:\n",TrapLabel);
							fprintf(fp, "\t\t mov   edx,[%s]\n",REG_CCR);
							fprintf(fp, "\t\t or    edx,0x0800\t\t;V flag\n");
							Completed();


							/* Division by Zero */

							Align();
							fprintf(fp, "%s_ZERO:\t\t ;Do divide by zero trap\n", TrapLabel);

							/* Correct cycle counter for error */

							fprintf(fp, "\t\t add   dword [%s],byte %d\n",ICOUNT,95 + (type * 17));
							fprintf(fp, "\t\t mov   al,5\n");
							Exception(-1,BaseCode);
							Completed();
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

	if (OpcodeArray[BaseCode] == -2)
	{
		Align();
		fprintf(fp, "%s:\n",GenerateLabel(BaseCode,0));

		CheckCPUtype(1);

		SavePreviousPC();

		TimingCycles += 16;

		OpcodeArray[BaseCode] = BaseCode ;

		/* Get Return Address */

		fprintf(fp, "\t\t mov   eax,[%s]\n",REG_A7);
		Memory_Read('L',EAX,"---D--B",1);


		/* Get Displacement */

		Memory_Fetch('W',EBX,TRUE);


		/* Set PC = New Address */

		fprintf(fp, "\t\t mov   esi,eax\n");


		/* Correct Stack for Return Address and Displacement */

		fprintf(fp, "\t\t add   ebx,byte 4\n");
		fprintf(fp, "\t\t add   dword [%s],ebx\n",REG_A7);

		MemoryBanking(BaseCode);
		Completed();
	}
}

void MoveControlRegister(void)
{
	int  Direction;
	int  BaseCode = 0x4e7a ;

	for (Direction=0;Direction<2;Direction++)
	{
		Align();
		fprintf(fp, "%s:\n",GenerateLabel(BaseCode+Direction,0));

		TimingCycles += 4; /* Assume same as move usp */

		CheckCPUtype(1);

		fprintf(fp, "\t\t test  byte [%s],20h \t\t\t; Supervisor Mode ?\n",REG_SRH);
		fprintf(fp, "\t\t jz    short OP%d_%4.4x_Trap\n",CPU,BaseCode+Direction);

		fprintf(fp, "\t\t add   esi,byte 2\n");
		if (CPU==2)
			fprintf(fp, "\t\t xor   esi,2\n");	/* ASG */
#ifdef STALLCHECK
		ClearRegister(EBX);
		fprintf(fp, "\t\t mov   bx,[esi+ebp]\n");
#else
		fprintf(fp, "\t\t movzx ebx,word [esi+ebp]\n");
#endif
		if (CPU==2)
			fprintf(fp, "\t\t xor   esi,2\n");	/* ASG */

		fprintf(fp, "\t\t add   esi,byte 2\n");
		fprintf(fp, "\t\t mov   eax,ebx\n");
		fprintf(fp, "\t\t mov   ecx,ebx\n");

		/* Sort out Register */

		fprintf(fp, "\t\t shr   ebx,12\n");

		/* Sort out Control Register ID */

		fprintf(fp, "\t\t and   eax,byte 1\n");
		fprintf(fp, "\t\t shr   ecx,10\n");
		fprintf(fp, "\t\t and   ecx,2\n");
		fprintf(fp, "\t\t or    ecx,eax\n");

		if (Direction==0)
		{
			/* from Control */

			fprintf(fp, "\t\t mov   eax,[%s+ecx*4]\n",REG_SFC);
			fprintf(fp, "\t\t mov   %s,eax\n",REG_DAT_EBX);
		}
		else
		{
			/* To Control */

			fprintf(fp, "\t\t mov   eax,%s\n",REG_DAT_EBX);

			/* Mask out for SFC & DFC */

			fprintf(fp, "\t\t test  cl,2\n");
			fprintf(fp, "\t\t jne   short OP%d_%4.4x_Mask\n",CPU,BaseCode+Direction);
			fprintf(fp, "\t\t and   eax,byte 7\n");
			fprintf(fp, "OP%d_%4.4x_Mask:\n",CPU,BaseCode+Direction);

			/* Write to control */

			fprintf(fp, "\t\t mov   [%s+ecx*4],eax\n",REG_SFC);
		}

		Completed();

		/* Not Supervisor Mode */

		Align();
		fprintf(fp, "OP%d_%4.4x_Trap:\n",CPU,BaseCode+Direction);
		Exception(8,BaseCode+Direction);

		OpcodeArray[BaseCode+Direction] = BaseCode+Direction;
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
	int Opcode,l,op;

	fprintf(fp, "DD OP%d_1000\n",CPU);

	l = 0 ;
	for (Opcode=0x0;Opcode<0x10000;)
	{

		op = OpcodeArray[Opcode];

		fprintf(fp, "DD ");

		l = 1 ;
		while (op == OpcodeArray[Opcode+l] && ((Opcode+l) & 0xfff) != 0)
		{
			l++ ;
		}

		Opcode += l ;

		if (l > 255)
		{
			if (op > -1)
				fprintf(fp, "OP%d_%4.4x - OP%d_1000\n",CPU,op,CPU);
			else
				fprintf(fp, "ILLEGAL - OP%d_1000\n",CPU);

			fprintf(fp, "DW %d\n", l);
		}
		else
		{
			if (op > -1)
				fprintf(fp, "(OP%d_%4.4x - OP%d_1000) + (%d * 1000000h)\n",CPU,op,CPU,l);
			else
				fprintf(fp, "(ILLEGAL - OP%d_1000) + (%d * 1000000h)\n",CPU,l);
		}
	}
}

void CodeSegmentBegin(void)
{
/* Messages */

	fprintf(fp, "; Make68K - V%s - Copyright 1998, Mike Coates (mame@btinternet.com)\n", VERSION);
	fprintf(fp, ";                               & Darren Olafson (deo@mail.island.net)\n\n");

/* Needed code to make it work! */

	fprintf(fp, "\t\t BITS 32\n\n");

	fprintf(fp, "\t\t GLOBAL %s_RUN\n",CPUtype);
	fprintf(fp, "\t\t GLOBAL %s_RESET\n",CPUtype);
	fprintf(fp, "\t\t GLOBAL %s_regs\n",CPUtype);
	fprintf(fp, "\t\t GLOBAL %s_COMPTABLE\n",CPUtype);
	fprintf(fp, "\t\t GLOBAL %s_OPCODETABLE\n",CPUtype);

	/* ASG - only one interface to memory now */
	fprintf(fp, "\t\t EXTERN %sm68k_ICount\n", PREF);
	fprintf(fp, "\t\t EXTERN %sa68k_memory_intf\n", PREF);
	fprintf(fp, "\t\t EXTERN %smem_amask\n", PREF);

	fprintf(fp, "; Vars Mame declares / needs access to\n\n");

	fprintf(fp, "\t\t EXTERN %smame_debug\n", PREF);
	fprintf(fp, "\t\t EXTERN %sillegal_op\n", PREF);
	fprintf(fp, "\t\t EXTERN %sillegal_pc\n", PREF);

	fprintf(fp, "\t\t EXTERN %sOP_ROM\n", PREF);
	fprintf(fp, "\t\t EXTERN %sOP_RAM\n", PREF);

	fprintf(fp, "\t\t EXTERN %sopcode_entry\n", PREF);
//	fprintf(fp, "\t\t EXTERN %scur_mrhard\n", PREF);

#ifdef MAME_DEBUG
	fprintf(fp, "\t\t EXTERN %sm68k_illegal_opcode\n", PREF);
#endif

#ifdef OS2
	fprintf(fp, "\t\t SECTION maincode USE32 FLAT CLASS=CODE\n\n");
#else
	fprintf(fp, "\t\t SECTION .text\n\n");
#endif



/* Reset routine */

	fprintf(fp, "%s_RESET:\n",CPUtype);

	fprintf(fp, "\t\t pushad\n\n");

	fprintf(fp, "; Build Jump Table (not optimised!)\n\n");

	fprintf(fp, "\t\t lea   edi,[%s_OPCODETABLE]\t\t; Jump Table\n", CPUtype);
	fprintf(fp, "\t\t lea   esi,[%s_COMPTABLE]\t\t; RLE Compressed Table\n", CPUtype);

	/* Reference Point in EBP */

	fprintf(fp, "\t\t mov   ebp,[esi]\n");
	fprintf(fp, "\t\t add   esi,byte 4\n");

	fprintf(fp, "RESET0:\n");
	fprintf(fp, "\t\t mov   eax,[esi]\n");
	fprintf(fp, "\t\t mov   ecx,eax\n");
	fprintf(fp, "\t\t and   eax,0xffffff\n");
	fprintf(fp, "\t\t add   eax,ebp\n");
	fprintf(fp, "\t\t add   esi,byte 4\n");

	/* if count is zero, then it's a word RLE length */

	fprintf(fp, "\t\t shr   ecx,24\n");
	fprintf(fp, "\t\t jne   short RESET1\n");

#ifdef STALLCHECK
	ClearRegister(ECX);
	fprintf(fp, "\t\t mov   cx,[esi]\t\t; Repeats\n");
#else
	fprintf(fp, "\t\t movzx ecx,word [esi]\t\t; Repeats\n");
#endif

	fprintf(fp, "\t\t add   esi,byte 2\n");
	fprintf(fp, "\t\t jecxz RESET2\t\t; Finished!\n");

	fprintf(fp, "RESET1:\n");
	fprintf(fp, "\t\t mov   [edi],eax\n");
	fprintf(fp, "\t\t add   edi,byte 4\n");
	fprintf(fp, "\t\t dec   ecx\n");
	fprintf(fp, "\t\t jnz   short RESET1\n");
	fprintf(fp, "\t\t jmp   short RESET0\n");

	fprintf(fp, "RESET2:\n");
	fprintf(fp, "\t\t popad\n");
	fprintf(fp, "\t\t ret\n\n");

/* Emulation Entry Point */

	Align();

	fprintf(fp, "%s_RUN:\n",CPUtype);

	fprintf(fp, "\t\t pushad\n");
	fprintf(fp, "\t\t mov   esi,[%s]\n",REG_PC);
	fprintf(fp, "\t\t mov   edx,[%s]\n",REG_CCR);
	fprintf(fp, "\t\t mov   ebp,dword [%sOP_ROM]\n", PREF);

	fprintf(fp,"; Check for Interrupt waiting\n\n");
	fprintf(fp,"\t\t test  [%s],byte 07H\n",REG_IRQ);
	fprintf(fp,"\t\t jne   near interrupt\n\n");

	fprintf(fp, "IntCont:\n");

	/* See if was only called to check for Interrupt */

	fprintf(fp, "\t\t test  dword [%s],-1\n",ICOUNT);
	fprintf(fp, "\t\t js    short MainExit\n\n");

#ifdef FBA_DEBUG

	fprintf(fp, "\n\t\t test    byte [%smame_debug],byte 0xff\n", PREF);
	fprintf(fp, "\t\t jns  near .NoDebug\n");
	fprintf(fp, "\t\t call  near FBADebugActive\n\n");
	fprintf(fp, ".NoDebug\n");

#endif

	if(CPU==2)
	{
		  /* 32 Bit */
		fprintf(fp, "\t\t mov   eax,2\n");		/* ASG */
		fprintf(fp, "\t\t xor   eax,esi\n");							/* ASG */
#ifdef STALLCHECK
		ClearRegister(ECX);
		fprintf(fp, "\t\t mov   cx,[eax+ebp]\n");
#else
		fprintf(fp, "\t\t movzx ecx,word [eax+ebp]\n");
#endif
	}
	else
	{
		  /* 16 Bit Fetch */
#ifdef STALLCHECK
		ClearRegister(ECX);
		fprintf(fp, "\t\t mov   cx,[esi+ebp]\n");
#else
		fprintf(fp, "\t\t movzx ecx,word [esi+ebp]\n");
#endif
	}
	fprintf(fp, "\t\t jmp   [%s_OPCODETABLE+ecx*4]\n", CPUtype);

	Align();

	fprintf(fp, "MainExit:\n");
	fprintf(fp, "\t\t mov   [%s],esi\t\t; Save PC\n",REG_PC);
	fprintf(fp, "\t\t mov   [%s],edx\n",REG_CCR);
	fprintf(fp, "\t\t test  byte [%s],20H\n",REG_SRH);
	fprintf(fp, "\t\t mov   eax,[%s]\t\t; Get A7\n",REG_A7);
	fprintf(fp, "\t\t jne   short ME1\t\t; Mode ?\n");
	fprintf(fp, "\t\t mov   [%s],eax\t\t;Save in USP\n",REG_USP);
	fprintf(fp, "\t\t jmp   short MC68Kexit\n");
	fprintf(fp, "ME1:\n");
	fprintf(fp, "\t\t mov   [%s],eax\n",REG_ISP);
	fprintf(fp, "MC68Kexit:\n");

	/* If in Debug mode make normal SR register */

#ifdef MAME_DEBUG

	ReadCCR('W', ECX);
	fprintf(fp, "\t\t mov   [%s],eax\n\n",REG_S);

#endif

	fprintf(fp, "\t\t popad\n");
	fprintf(fp, "\t\t ret\n");

/* Check for Pending Interrupts */

	Align();
	fprintf(fp, "; Interrupt check\n\n");

	fprintf(fp, "interrupt:\n");

	/* check to exclude interrupts */

	fprintf(fp, "\t\t mov   eax,[%s]\n",REG_IRQ);
	fprintf(fp, "\t\t and   eax,byte 07H\n");

	fprintf(fp, "\t\t cmp   al,7\t\t ; Always take 7\n");
	fprintf(fp, "\t\t je    short procint\n\n");

	fprintf(fp, "\t\t mov   ebx,[%s]\t\t; int mask\n",REG_SRH);
	fprintf(fp, "\t\t and   ebx,byte 07H\n");
	fprintf(fp, "\t\t cmp   eax,ebx\n");
	fprintf(fp, "\t\t jle   near IntCont\n\n");

	/* Take pending Interrupt */

	Align();
	fprintf(fp, "procint:\n");

	/* LVR - Enable proper IRQ handling (require explicit acknowledge) */
	/* fprintf(fp, "\t\t and   byte [%s],78h\t\t; remove interrupt & stop\n\n",REG_IRQ); */
	fprintf(fp, "\t\t and   byte [%s],7fh\t\t; remove stop\n\n",REG_IRQ);

	/* Get Interrupt Vector from callback */

	fprintf(fp, "\t\t push  eax\t\t; save level\n\n");

	if (SavedRegs[EBX] == '-')
	{
		fprintf(fp, "\t\t push  EBX\n");
	}
	else
	{
		fprintf(fp, "\t\t mov   ebx,eax\n");
	}

	if (SavedRegs[ESI] == '-')
	{
		fprintf(fp, "\t\t mov   [%s],ESI\n",REG_PC);
	}

	if (SavedRegs[EDX] == '-')
	{
		fprintf(fp, "\t\t mov   [%s],edx\n",REG_CCR);
	}

/* ----- Win32 uses FASTCALL (By Kenjo)----- */

#ifdef FASTCALL
	fprintf(fp, "\t\t mov   %s, eax\t\t; irq line #\n",FASTCALL_FIRST_REG);
	fprintf(fp, "\t\t call  dword [%s]\t; get the IRQ level\n", REG_IRQ_CALLBACK);
#else
	fprintf(fp, "\t\t push  eax\t\t; irq line #\n");
	fprintf(fp, "\t\t call  dword [%s]\t; get the IRQ level\n", REG_IRQ_CALLBACK);
	fprintf(fp, "\t\t lea   esp,[esp+4]\n");
#endif

	if (SavedRegs[EDX] == '-')
	{
		fprintf(fp, "\t\t mov   edx,[%s]\n",REG_CCR);
	}

	if (SavedRegs[ESI] == '-')
	{
		fprintf(fp, "\t\t mov   esi,[%s]\n",REG_PC);
	}

	/* Do we want to use normal vector number ? */


	fprintf(fp, "\t\t test  eax,eax\n");
	fprintf(fp, "\t\t jns   short AUTOVECTOR\n");

	/* Only need EBX restored if default vector to be used */

	if (SavedRegs[EBX] == '-')
	{
		fprintf(fp, "\t\t pop   EBX\n");
	}

	/* Just get default vector */

	fprintf(fp, "\t\t mov   eax,ebx\n");

	fprintf(fp, "\t\t add   eax,byte 24\t\t; Vector\n\n");

	fprintf(fp, "AUTOVECTOR:\n\n");

	Exception(-1,0xFFFF);

	fprintf(fp, "\t\t pop   eax\t\t; set Int mask\n");
	fprintf(fp, "\t\t mov   bl,byte [%s]\n",REG_SRH);
	fprintf(fp, "\t\t and   bl,0F8h\n");
	fprintf(fp, "\t\t or    bl,al\n");
	fprintf(fp, "\t\t mov   byte [%s],bl\n\n",REG_SRH);
	fprintf(fp, "\t\t jmp   IntCont\n\n");

/* Exception Routine */

	Align();
	fprintf(fp, "Exception:\n");
	fprintf(fp, "\t\t push  edx\t\t; Save flags\n");
	fprintf(fp, "\t\t and   eax,0FFH\t\t; Zero Extend IRQ Vector\n");

	fprintf(fp, "\t\t push  eax\t\t; Save for Later\n");

	/*  Update Cycle Count */

	fprintf(fp, "\t\t mov   al,[exception_cycles+eax]\t\t; Get Cycles\n");
	fprintf(fp, "\t\t sub   [%s],eax\t\t; Decrement ICount\n",ICOUNT);

	ReadCCR('W',ECX);

	fprintf(fp, "\t\t mov   edi,[%s]\t\t; Get A7\n",REG_A7);

	fprintf(fp, "\t\t test  ah,20H\t; Which Mode ?\n");
	fprintf(fp, "\t\t jne   short ExSuperMode\t\t; Supervisor\n");

	fprintf(fp, "\t\t or    byte [%s],20H\t; Set Supervisor Mode\n",REG_SRH);
	fprintf(fp, "\t\t mov   [%s],edi\t\t; Save in USP\n",REG_USP);
	fprintf(fp, "\t\t mov   edi,[%s]\t\t; Get ISP\n",REG_ISP);

	/* Write SR first (since it's in a register) */

	fprintf(fp, "ExSuperMode:\n");
	fprintf(fp, "\t\t sub   edi,byte 6\n");
	fprintf(fp, "\t\t mov   [%s],edi\t\t; Put in A7\n",REG_A7);
	Memory_Write('W',EDI,EAX,"----S-B",2);

	/* Then write PC */

	fprintf(fp, "\t\t add   edi,byte 2\n");
	Memory_Write('L',EDI,ESI,"------B",0);

	/* Get new PC */

	fprintf(fp, "\t\t pop   eax\t\t;Level\n");
	fprintf(fp, "\t\t shl   eax,2\n");
	fprintf(fp, "\t\t add   eax,[%s]\n",REG_VBR);	  /* 68010+ Vector Base */

	/* Direct Read */

	Memory_Read('L',EAX,"------B",0);

	fprintf(fp, "\t\t mov   esi,eax\t\t;Set PC\n");
	fprintf(fp, "\t\t pop   edx\t\t; Restore flags\n");

	/* Sort out any bank changes */
	MemoryBanking(1);

	fprintf(fp, "\t\t ret\n");

#ifdef FBA_DEBUG

	fprintf(fp,"\n; Call FBA debugging callback\n\n");
	fprintf(fp, "FBADebugActive:");
	fprintf(fp, "\t\t mov   [%s],ESI\n",REG_PC);
	fprintf(fp, "\t\t mov   [%s],EDX\n",REG_CCR);
	fprintf(fp, "\t\t call  [%sa68k_memory_intf+0]\n", PREF);
	if (SavedRegs[EDX] == '-')
	{
		fprintf(fp, "\t\t mov   EDX,[%s]\n",REG_CCR);
	}
	if (SavedRegs[ESI] == '-')
	{
		fprintf(fp, "\t\t mov   ESI,[%s]\n",REG_PC);
	}
	if (SavedRegs[EBP] == '-')
	{
		fprintf(fp, "\t\t mov   ebp,dword [%sOP_ROM]\n", PREF);
	}
	fprintf(fp,"\n; Now continue as usual\n\n");
	fprintf(fp, "\t\t ret\n");

#endif

}

void CodeSegmentEnd(void)
{
#ifdef OS2
	fprintf(fp, "\t\t SECTION maindata USE32 FLAT CLASS=DATA\n\n");
#else
	fprintf(fp, "\t\t SECTION .data\n");
#endif

	fprintf(fp, "\n\t\t align 16\n");
	fprintf(fp, "%s_ICount\n",CPUtype);
	fprintf(fp, "asm_count\t DD 0\n\n");

	/* Memory structure for 68000 registers  */
	/* Same layout as structure in CPUDEFS.H */

	fprintf(fp, "\n\n; Register Structure\n\n");
	fprintf(fp, "%s_regs\n",CPUtype);

	fprintf(fp, "R_D0\t DD 0\t\t\t ; Data Registers\n");
	fprintf(fp, "R_D1\t DD 0\n");
	fprintf(fp, "R_D2\t DD 0\n");
	fprintf(fp, "R_D3\t DD 0\n");
	fprintf(fp, "R_D4\t DD 0\n");
	fprintf(fp, "R_D5\t DD 0\n");
	fprintf(fp, "R_D6\t DD 0\n");
	fprintf(fp, "R_D7\t DD 0\n\n");

	fprintf(fp, "R_A0\t DD 0\t\t\t ; Address Registers\n");
	fprintf(fp, "R_A1\t DD 0\n");
	fprintf(fp, "R_A2\t DD 0\n");
	fprintf(fp, "R_A3\t DD 0\n");
	fprintf(fp, "R_A4\t DD 0\n");
	fprintf(fp, "R_A5\t DD 0\n");
	fprintf(fp, "R_A6\t DD 0\n");
	fprintf(fp, "R_A7\t DD 0\n\n");

	fprintf(fp, "R_ISP\t DD 0\t\t\t ; Supervisor Stack\n");
	fprintf(fp, "R_SR_H\t DD 0\t\t\t ; Status Register High TuSuuIII\n");
	fprintf(fp, "R_CCR\t DD 0\t\t\t ; CCR Register in Intel Format\n");
	fprintf(fp, "R_XC\t DD 0\t\t\t ; Extended Carry uuuuuuuX\n");

	fprintf(fp, "R_PC\t DD 0\t\t\t ; Program Counter\n");
	fprintf(fp, "R_IRQ\t DD 0\t\t\t ; IRQ Request Level\n\n");
	fprintf(fp, "R_SR\t DD 0\t\t\t ; Motorola Format SR\n\n");

	fprintf(fp, "R_IRQ_CALLBACK\t DD 0\t\t\t ; irq callback (get vector)\n\n");

	fprintf(fp, "R_PPC\t DD 0\t\t\t ; Previous Program Counter\n");

	fprintf(fp, "R_RESET_CALLBACK\t DD 0\t\t\t ; Reset Callback\n");

	fprintf(fp, "R_SFC\t DD 0\t\t\t ; Source Function Call\n");
	fprintf(fp, "R_DFC\t DD 0\t\t\t ; Destination Function Call\n");
	fprintf(fp, "R_USP\t DD 0\t\t\t ; User Stack\n");
	fprintf(fp, "R_VBR\t DD 0\t\t\t ; Vector Base\n");

	fprintf(fp, "asmbank\t DD 0\n\n");
	fprintf(fp, "CPUversion\t DD 0\n\n");
	fprintf(fp, "FullPC\t DD 0\n\n");

	/* Extra space for variables mame uses for debugger */

	fprintf(fp, "\t\t DD 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n");
	fprintf(fp, "\t\t DD 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n");
	fprintf(fp, "\t\t DD 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n");
	fprintf(fp, "\t\t DD 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n");
	fprintf(fp, "\t\t DD 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n");
	fprintf(fp, "\t\t DD 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n");
	fprintf(fp, "\t\t DD 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n");
	fprintf(fp, "\t\t DD 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n\n");

	/* Safe Memory Locations */

	fprintf(fp, "\t\t ALIGN 16\n");

	fprintf(fp, "\n\nIntelFlag\t\t\t\t; Intel Flag Lookup Table\n");
	fprintf(fp, "\t\t DD 0000h,0001h,0800h,0801h,0040h,0041h,0840h,0841h\n");
	fprintf(fp, "\t\t DD 0080h,0081h,0880h,0881h,00C0h,00C1h,08C0h,08C1h\n");
	fprintf(fp, "\t\t DD 0100h,0101h,0900h,0901h,0140h,0141h,0940h,0941h\n");
	fprintf(fp, "\t\t DD 0180h,0181h,0980h,0981h,01C0h,01C1h,09C0h,09C1h\n");

#if 0
	fprintf(fp, "\n\nImmTable\n");
	fprintf(fp, "\t\t DD 8,1,2,3,4,5,6,7\n\n");
#endif



	/* Exception Timing Table */

	fprintf(fp, "exception_cycles\n");
	fprintf(fp, "\t\t DB 0, 0, 0, 0, 38, 42, 44, 38, 38, 0, 38, 38, 0, 0, 0, 0\n");
	fprintf(fp, "\t\t DB 0, 0, 0, 0, 0, 0, 0, 0, 46, 46, 46, 46, 46, 46, 46, 46\n");
	fprintf(fp, "\t\t DB 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38\n\n");

	fprintf(fp, "; RLE Compressed Jump Table\n\n");

	fprintf(fp, "%s_COMPTABLE\n\n", CPUtype);

	fprintf(fp, "%cinclude '%s'\n\n",'%', comptab);

	fprintf(fp, "\t\tDW   0,0,0\n\n");


/* If using Windows, put the table area in .data section (Kenjo) */
#ifdef WIN32

	fprintf(fp, "%s_OPCODETABLE\tTIMES  65536  DD 0\n\n", CPUtype);

#else

#ifdef OS2
	fprintf(fp, "\t\t SECTION tempdata USE32 FLAT CLASS=BSS\n\n");
#else
	fprintf(fp, "\t\t SECTION .bss\n");
#endif

	fprintf(fp, "%s_OPCODETABLE\tRESD  65536\n\n", CPUtype);

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

	if(CPU==2)						/* 68020 Commands */
	{
		divl();
		mull();
		bfext();
	}

	CodeSegmentEnd();
}

int main(int argc, char **argv)
{
	int dwLoop;

	printf("\nMake68K - V%s - Copyright 1998, Mike Coates (mame@btinternet.com)\n", VERSION);
	printf("                            1999, & Darren Olafson (deo@mail.island.net)\n");
	printf("                            2000\n");

	if (argc != 4 && argc != 5)
	{
		printf("Usage: %s outfile jumptable-outfile type [ppro]\n", argv[0]);
		exit(1);
	}

	printf("Building 680%s              2001\n\n",argv[3]);

	for (dwLoop=0;dwLoop<65536;)  OpcodeArray[dwLoop++] = -2;

	codebuf=malloc(64);
	if (!codebuf)
	{
		printf ("Memory allocation error\n");
		exit(3);
	}

	/* Emit the code */
	fp = fopen(argv[1], "w");
	if (!fp)
	{
		fprintf(stderr, "Can't open %s for writing\n", argv[1]);
		exit(1);
	}

	comptab = argv[2];


	CPUtype = malloc(64);

	sprintf(CPUtype,"%sM680%s", PREF, argv[3]);

	if(argv[3][0]=='2') CPU = 2;
	if(argc > 4 && !strcmp(argv[4], "ppro"))
	{
		  ppro = 1;
		  printf("Generating ppro opcodes\n");
	}

	EmitCode();

	fclose(fp);

	printf("\n%d Unique Opcodes\n",Opcount);

	/* output Jump table to separate file */
	fp = fopen(argv[2], "w");
	if (!fp)
	{
		fprintf(stderr, "Can't open %s for writing\n", argv[2]);
		exit(1);
	}

	JumpTable();

	fclose(fp);

	exit(0);
}
