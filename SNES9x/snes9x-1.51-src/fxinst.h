/**********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2007  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),
                             zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com)
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti


  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley,
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001-2006    byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight,

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound DSP emulator code is derived from SNEeSe and OpenSPC:
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2007  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
**********************************************************************************/



#ifndef _FXINST_H_
#define _FXINST_H_ 1

/*
 * FxChip(GSU) register space specification
 * (Register address space 3000->32ff)
 *
 * The 16 generic 16 bit registers:
 * (Some have a special function in special circumstances)
 * 3000 - R0	default source/destination register
 * 3002 - R1	pixel plot X position register
 * 3004 - R2	pixel plot Y position register
 * 3006 - R3
 * 3008 - R4	lower 16 bit result of lmult
 * 300a - R5
 * 300c - R6	multiplier for fmult and lmult
 * 300e - R7	fixed point texel X position for merge
 * 3010 - R8	fixed point texel Y position for merge
 * 3012 - R9
 * 3014 - R10
 * 3016 - R11	return address set by link
 * 3018 - R12	loop counter
 * 301a - R13	loop point address
 * 301c - R14	rom address for getb, getbh, getbl, getbs
 * 301e - R15	program counter
 *
 * 3020-302f -	unused
 *
 * Other internal registers
 * 3030 - SFR	status flag register (16bit)
 * 3032 -	unused
 * 3033 - BRAMR Backup RAM register (8bit)
 * 3034 - PBR	program bank register (8bit)
 * 3035 -	unused
 * 3036 - ROMBR	rom bank register (8bit)
 * 3037 - CFGR	control flags register (8bit)
 * 3038 - SCBR	screen base register (8bit)
 * 3039 - CLSR	clock speed register (8bit)
 * 303a - SCMR	screen mode register (8bit)
 * 303b - VCR	version code register (8bit) (read only)
 * 303c - RAMBR	ram bank register (8bit)
 * 303d -	unused
 * 303e - CBR	cache base register (16bit)
 *
 * 3040-30ff -	unused
 *
 * 3100-32ff -	CACHERAM 512 bytes of GSU cache memory
 *
 * SFR status flag register bits:
 *  0   -
 *  1   Z	Zero flag
 *  2   CY	Carry flag
 *  3   S	Sign flag
 *  4   OV	Overflow flag
 *  5   G	Go flag (set to 1 when the GSU is running)
 *  6   R	Set to 1 when reading ROM using R14 address
 *  7   -
 *  8   ALT1	Mode set-up flag for the next instruction
 *  9   ALT2	Mode set-up flag for the next instruction
 * 10   IL	Immediate lower 8-bit flag
 * 11   IH	Immediate higher 8-bit flag
 * 12   B	Set to 1 when the WITH instruction is executed
 * 13   -
 * 14   -
 * 15   IRQ	Set to 1 when GSU caused an interrupt
 *              Set to 0 when read by 658c16
 *
 * BRAMR = 0, BackupRAM is disabled
 * BRAMR = 1, BackupRAM is enabled
 *
 * CFGR control flags register bits:
 *  0   -
 *  1   -
 *  2   -
 *  3   -
 *  4   -
 *  5   MS0	Multiplier speed, 0=standard, 1=high speed
 *  6   -
 *  7   IRQ	Set to 1 when GSU interrupt request is masked
 *
 * CLSR clock speed register bits:
 *  0   CLSR	clock speed, 0 = 10.7Mhz, 1 = 21.4Mhz
 *
 * SCMR screen mode register bits:
 *  0	MD0	color depth mode bit 0
 *  1	MD1	color depth mode bit 1
 *  2	HT0	screen height bit 1
 *  3	RAN	RAM access control
 *  4	RON	ROM access control
 *  5	HT1	screen height bit 2
 *  6	-
 *  7	-
 *
 * RON = 0	SNES CPU has ROM access
 * RON = 1	GSU has ROM access
 *
 * RAN = 0	SNES has game pak RAM access
 * RAN = 1	GSU has game pak RAM access
 *
 * HT1  HT0  Screen height mode
 *  0    0   128 pixels high
 *  0    1   160 pixels high
 *  1    0   192 pixels high
 *  1    1   OBJ mode
 *
 * MD1  MD0  Color depth mode
 *  0    0   4 color mode
 *  0    1   16 color mode
 *  1    0   not used
 *  1    1   256 color mode
 *
 * CBR cache base register bits:
 * 15-4	     Specify base address for data to cache from ROM or RAM
 *  3-0	     Are 0 when address is read
 *
 * Write access to the program counter (301e) from
 * the SNES-CPU will start the GSU, and it will not
 * stop until it reaches a stop instruction.
 *
 */

/* Number of banks in GSU RAM */
#define FX_RAM_BANKS 4

/* Emulate proper R14 ROM access (slower, but safer) */
/* #define FX_DO_ROMBUFFER */

/* Address checking (definately slow) */
/* #define FX_ADDRESS_CHECK */

struct FxRegs_s
{
    /* FxChip registers */
    uint32	avReg[16];		/* 16 Generic registers */
    uint32	vColorReg;		/* Internal color register */
    uint32	vPlotOptionReg;		/* Plot option register */
    uint32	vStatusReg;		/* Status register */
    uint32	vPrgBankReg;		/* Program bank index register */
    uint32	vRomBankReg;		/* Rom bank index register */
    uint32	vRamBankReg;		/* Ram bank index register */
    uint32	vCacheBaseReg;		/* Cache base address register */
    uint32	vCacheFlags;		/* Saying what parts of the cache was written to */
    uint32	vLastRamAdr;		/* Last RAM address accessed */
    uint32 *	pvDreg;			/* Pointer to current destination register */
    uint32 *	pvSreg;			/* Pointer to current source register */
    uint8	vRomBuffer;		/* Current byte read by R14 */
    uint8	vPipe;			/* Instructionset pipe */
    uint32	vPipeAdr;		/* The address of where the pipe was read from */

    /* status register optimization stuff */
    uint32	vSign;			/* v & 0x8000 */
    uint32	vZero;			/* v == 0 */
    uint32	vCarry;			/* a value of 1 or 0 */
    int32	vOverflow;		/* (v >= 0x8000 || v < -0x8000) */

    /* Other emulator variables */

    int32	vErrorCode;
    uint32	vIllegalAddress;

    uint8	bBreakPoint;
    uint32	vBreakPoint;
    uint32	vStepPoint;

    uint8 *	pvRegisters;	/* 768 bytes located in the memory at address 0x3000 */
    uint32	nRamBanks;	/* Number of 64kb-banks in FxRam (Don't confuse it with SNES-Ram!!!) */
    uint8 *	pvRam;		/* Pointer to FxRam */
    uint32	nRomBanks;	/* Number of 32kb-banks in Cart-ROM */
    uint8 *     pvRom;		/* Pointer to Cart-ROM */

    uint32	vMode;		/* Color depth/mode */
    uint32	vPrevMode;	/* Previous depth */
    uint8 *	pvScreenBase;
    uint8 *	apvScreen[32];		/* Pointer to each of the 32 screen colums */
    int32	x[32];
    uint32	vScreenHeight;		/* 128, 160, 192 or 256 (could be overriden by cmode) */
    uint32	vScreenRealHeight;	/* 128, 160, 192 or 256 */
    uint32	vPrevScreenHeight;
    uint32	vScreenSize;
    void	(*pfPlot)();
    void	(*pfRpix)();

    uint8 *	pvRamBank;		/* Pointer to current RAM-bank */
    uint8 *	pvRomBank;		/* Pointer to current ROM-bank */
    uint8 *	pvPrgBank;		/* Pointer to current program ROM-bank */

    uint8 *	apvRamBank[FX_RAM_BANKS];/* Ram bank table (max 256kb) */
    uint8 *	apvRomBank[256];	/* Rom bank table */

    uint8	bCacheActive;
    uint8 *	pvCache;		/* Pointer to the GSU cache */
    uint8 	avCacheBackup[512];	/* Backup of ROM when the cache has replaced it */
    uint32	vCounter;
    uint32	vInstCount;
    uint32	vSCBRDirty;		/* if SCBR is written, our cached screen pointers need updating */
};

#define  FxRegs_s_null { \
   {0},    0,      0,      0,      0,   0,    0,   0,    0,    0, \
  NULL, NULL,      0,      0,      0,   0,    0,   0,    0,    0, \
     0,    0,      0,      0,   NULL,   0, NULL,   0, NULL,    0, \
     0, NULL, {NULL},    {0},      0,   0,    0,   0, NULL, NULL, \
  NULL, NULL,   NULL, {NULL}, {NULL},   0, NULL, {0},    0,    0, \
  0, \
}

/* GSU registers */
#define GSU_R0 0x000
#define GSU_R1 0x002
#define GSU_R2 0x004
#define GSU_R3 0x006
#define GSU_R4 0x008
#define GSU_R5 0x00a
#define GSU_R6 0x00c
#define GSU_R7 0x00e
#define GSU_R8 0x010
#define GSU_R9 0x012
#define GSU_R10 0x014
#define GSU_R11 0x016
#define GSU_R12 0x018
#define GSU_R13 0x01a
#define GSU_R14 0x01c
#define GSU_R15 0x01e
#define GSU_SFR 0x030
#define GSU_BRAMR 0x033
#define GSU_PBR 0x034
#define GSU_ROMBR 0x036
#define GSU_CFGR 0x037
#define GSU_SCBR 0x038
#define GSU_CLSR 0x039
#define GSU_SCMR 0x03a
#define GSU_VCR 0x03b
#define GSU_RAMBR 0x03c
#define GSU_CBR 0x03e
#define GSU_CACHERAM 0x100

/* SFR flags */
#define FLG_Z (1<<1)
#define FLG_CY (1<<2)
#define FLG_S (1<<3)
#define FLG_OV (1<<4)
#define FLG_G (1<<5)
#define FLG_R (1<<6)
#define FLG_ALT1 (1<<8)
#define FLG_ALT2 (1<<9)
#define FLG_IL (1<<10)
#define FLG_IH (1<<11)
#define FLG_B (1<<12)
#define FLG_IRQ (1<<15)

/* Test flag */
#define TF(a) (GSU.vStatusReg & FLG_##a )
#define CF(a) (GSU.vStatusReg &= ~FLG_##a )
#define SF(a) (GSU.vStatusReg |= FLG_##a )

/* Test and set flag if condition, clear if not */
#define TS(a,b) GSU.vStatusReg = ( (GSU.vStatusReg & (~FLG_##a)) | ( (!!(##b)) * FLG_##a ) )

/* Testing ALT1 & ALT2 bits */
#define ALT0 (!TF(ALT1)&&!TF(ALT2))
#define ALT1 (TF(ALT1)&&!TF(ALT2))
#define ALT2 (!TF(ALT1)&&TF(ALT2))
#define ALT3 (TF(ALT1)&&TF(ALT2))

/* Sign extend from 8/16 bit to 32 bit */
#define SEX16(a) ((int32)((int16)(a)))
#define SEX8(a) ((int32)((int8)(a)))

/* Unsign extend from 8/16 bit to 32 bit */
#define USEX16(a) ((uint32)((uint16)(a)))
#define USEX8(a) ((uint32)((uint8)(a)))

#define SUSEX16(a) ((int32)((uint16)(a)))

/* Set/Clr Sign and Zero flag */
#define TSZ(num) TS(S, (num & 0x8000)); TS(Z, (!USEX16(num)) )

/* Clear flags */
#define CLRFLAGS GSU.vStatusReg &= ~(FLG_ALT1|FLG_ALT2|FLG_B); GSU.pvDreg = GSU.pvSreg = &R0;

/* Read current RAM-Bank */
#define RAM(adr) GSU.pvRamBank[USEX16(adr)]

/* Read current ROM-Bank */
#define ROM(idx) (GSU.pvRomBank[USEX16(idx)])

/* Access the current value in the pipe */
#define PIPE GSU.vPipe

/* Access data in the current program bank */
#define PRGBANK(idx) GSU.pvPrgBank[USEX16(idx)]

/* Update pipe from ROM */
#if 0
#define FETCHPIPE { PIPE = PRGBANK(R15); GSU.vPipeAdr = (GSU.vPrgBankReg<<16) + R15; }
#else
#define FETCHPIPE { PIPE = PRGBANK(R15); }
#endif

/* ABS */
#define ABS(x) ((x)<0?-(x):(x))

/* Access source register */
#define SREG (*GSU.pvSreg)

/* Access destination register */
#define DREG (*GSU.pvDreg)

#ifndef FX_DO_ROMBUFFER

/* Don't read R14 */
#define READR14

/* Don't test and/or read R14 */
#define TESTR14

#else

/* Read R14 */
#define READR14 GSU.vRomBuffer = ROM(R14)

/* Test and/or read R14 */
#define TESTR14 if(GSU.pvDreg == &R14) READR14

#endif

/* Access to registers */
#define R0 GSU.avReg[0]
#define R1 GSU.avReg[1]
#define R2 GSU.avReg[2]
#define R3 GSU.avReg[3]
#define R4 GSU.avReg[4]
#define R5 GSU.avReg[5]
#define R6 GSU.avReg[6]
#define R7 GSU.avReg[7]
#define R8 GSU.avReg[8]
#define R9 GSU.avReg[9]
#define R10 GSU.avReg[10]
#define R11 GSU.avReg[11]
#define R12 GSU.avReg[12]
#define R13 GSU.avReg[13]
#define R14 GSU.avReg[14]
#define R15 GSU.avReg[15]
#define SFR GSU.vStatusReg
#define PBR GSU.vPrgBankReg
#define ROMBR GSU.vRomBankReg
#define RAMBR GSU.vRamBankReg
#define CBR GSU.vCacheBaseReg
#define SCBR USEX8(GSU.pvRegisters[GSU_SCBR])
#define SCMR USEX8(GSU.pvRegisters[GSU_SCMR])
#define COLR GSU.vColorReg
#define POR GSU.vPlotOptionReg
#define BRAMR USEX8(GSU.pvRegisters[GSU_BRAMR])
#define VCR USEX8(GSU.pvRegisters[GSU_VCR])
#define CFGR USEX8(GSU.pvRegisters[GSU_CFGR])
#define CLSR USEX8(GSU.pvRegisters[GSU_CLSR])

/* Execute instruction from the pipe, and fetch next byte to the pipe */
#define FX_STEP { uint32 vOpcode = (uint32)PIPE; FETCHPIPE; \
(*fx_ppfOpcodeTable[ (GSU.vStatusReg & 0x300) | vOpcode ])(); } \

#define FX_FUNCTION_RUN			0
#define FX_FUNCTION_RUN_TO_BREAKPOINT	1
#define FX_FUNCTION_STEP_OVER		2

extern uint32 (**fx_ppfFunctionTable)(uint32);
extern void (**fx_ppfPlotTable)();
extern void (**fx_ppfOpcodeTable)();

extern uint32 (*fx_apfFunctionTable[])(uint32);
extern void (*fx_apfOpcodeTable[])();
extern void (*fx_apfPlotTable[])();
extern uint32 (*fx_a_apfFunctionTable[])(uint32);
extern void (*fx_a_apfOpcodeTable[])();
extern void (*fx_a_apfPlotTable[])();
extern uint32 (*fx_r_apfFunctionTable[])(uint32);
extern void (*fx_r_apfOpcodeTable[])();
extern void (*fx_r_apfPlotTable[])();
extern uint32 (*fx_ar_apfFunctionTable[])(uint32);
extern void (*fx_ar_apfOpcodeTable[])();
extern void (*fx_ar_apfPlotTable[])();

/* Set this define if branches are relative to the instruction in the delay slot */
/* (I think they are) */
#define BRANCH_DELAY_RELATIVE

#endif

