//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------
//=========================================================================

	TLCS900h_disassemble.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

25 JUL 2002 - neopop_uk
=======================================
- Added missing registers to disassmbly table (unused, I hope!)
  
//---------------------------------------------------------------------------
*/

#ifdef NEOPOP_DEBUG
//=========================================================================

#include "neopop.h"
#include "TLCS900h_disassemble.h"
#include "TLCS900h_registers.h"
#include "TLCS900h_interpret.h"
#include "mem.h"

void TLCS900h_disassemble_extra(void);
void TLCS900h_disassemble_src(int size);
void TLCS900h_disassemble_dst(void);
void TLCS900h_disassemble_reg(int size);

//=========================================================================

char str_R[8];		//Big R
char str_r[8];		//Little R

//Control register names
char* crName[3][0x40] =
{
	{
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,

		0,0, "DMAM0", 0,
		0,0, "DMAM1", 0,
		0,0, "DMAM2", 0,
		0,0, "DMAM3", 0,
	},

	{
		0,0,
		0,0,
		0,0,
		0,0,
		0,0,
		0,0,
		0,0,
		0,0,

		"DMAC0", 0,
		"DMAC1", 0,
		"DMAC2", 0,
		"DMAC3", 0,

		0,0,		//30
		0,0,
		0,0,
		"NSP",0,	//3C
	},

	{
		"DMAS0",	//00
		"DMAS1",
		"DMAS2",
		"DMAS3",
		"DMAD0",	//10
		"DMAD1",
		"DMAD2",
		"DMAD3",
		0,			//20
		0,
		0,
		0,
		0,			//30
		0,
		0,
		"XNSP",		//3C
	}
};

//Register names
char* gprName[8][3] = 
{
	{"W", "WA", "XWA"},
	{"A", "BC", "XBC"},
	{"B", "DE", "XDE"},
	{"C", "HL", "XHL"},
	{"D", "IX", "XIX"},
	{"E", "IY", "XIY"},
	{"H", "IZ", "XIZ"},
	{"L", "SP", "XSP"}
};

//Condition Code names
char* ccName[] = 
{
	"F","LT","LE","ULE",
	"OV","MI","Z","C",
	"T","GE","GT","UGT",
	"NOV","PL","NZ","NC"
};

char* regCodeName[3][256] = 
{
	{
		"RA0","RW0","QA0","QW0","RC0","RB0","QC0","QB0",	//BANK 0
		"RE0","RD0","QE0","QD0","RL0","RH0","QL0","QH0",
		"RA1","RW1","QA1","QW1","RC1","RB1","QC1","QB1",	//BANK 1
		"RE1","RD1","QE1","QD1","RL1","RH1","QL1","QH1",
		"RA2","RW2","QA2","QW2","RC2","RB2","QC2","QB2",	//BANK 2
		"RE2","RD2","QE2","QD2","RL2","RH2","QL2","QH2",
		"RA3","RW3","QA3","QW3","RC3","RB3","QC3","QB3",	//BANK 3
		"RE3","RD3","QE3","QD3","RL3","RH3","QL3","QH3",

		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

		"A'","W'","QA'","QW'","C'","B'","QC'","QB'",		//Previous Bank
		"E'","D'","QE'","QD'","L'","H'","QL'","QH'",
		"A","W","QA","QW","C","B","QC","QB",				//Current Bank
		"E","D","QE","QD","L","H","QL","QH",
		
		"IXL","IXH","QIXL","QIXH","IYL","IYH","QIYL","QIYH",	
		"IZL","IZH","QIZL","QIZH","SPL","SPH","QSPL","QSPX"
	},

	{
		"RWA0","QWA0","RBC0","QBC0",	//BANK 0
		"RDE0","QDE0","RHL0","QHL0",
		"RWA1","QWA1","RBC1","QBC1",	//BANK 1
		"RDE1","QDE1","RHL1","QHL1",
		"RWA2","QWA2","RBC2","QBC2",	//BANK 2
		"RDE2","QDE2","RHL2","QHL2",
		"RWA3","QWA3","RBC3","QBC3",	//BANK 3
		"RDE3","QDE3","RHL3","QHL3",

		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,

		"WA'","QWA'","BC'","QBC'","DE'","QDE'","HL'","QHL'",//Previous Bank
		"WA","QWA","BC","QBC","DE","QDE","HL","QHL",		//Current Bank
		
		"IX","QIX","IY","QIY","IZ","QIZ","SP","QSP"
	},

	{
		"XWA0","XBC0","XDE0","XHL0",	//BANK 0
		"XWA1","XBC1","XDE1","XHL1",	//BANK 1
		"XWA2","XBC2","XDE2","XHL2",	//BANK 2
		"XWA3","XBC3","XDE3","XHL3",	//BANK 3

		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,

		"XWA'","XBC'","XDE'","XHL'",	//Previous Bank
		"XWA","XBC","XDE","XHL",		//Current Bank
		
		"XIX","XIY","XIZ","XSP"
	}
};

//=========================================================================

void get_rr_Name(void)
{
	sprintf(str_r, "???");

	if (size == 0 && first == 0xC7)
	{
		sprintf(str_r, extra);
		return;
	}

	switch(first & 7)
	{
	case 0: if (size == 1)	sprintf(str_r, "XWA");	break;
	case 1:	
		if (size == 0)	sprintf(str_r, "WA");
		if (size == 1)	sprintf(str_r, "XBC");
		break;
	case 2: if (size == 1)	sprintf(str_r, "XDE");	break;
	case 3:
		if (size == 0)	sprintf(str_r, "BC");
		if (size == 1)	sprintf(str_r, "XHL");
		break;
	case 4: if (size == 1)	sprintf(str_r, "XIX");	break;
	case 5:	
		if (size == 0)	sprintf(str_r, "DE");
		if (size == 1)	sprintf(str_r, "XIY");
		break;
	case 6: if (size == 1)	sprintf(str_r, "XIZ");	break;
	case 7:
		if (size == 0)	sprintf(str_r, "HL");
		if (size == 1)	sprintf(str_r, "XSP");
		break;
	}
}

void get_RR_Name(void)
{
	sprintf(str_R, "???");

	switch(second & 7)
	{
	case 0: if (size == 1)	sprintf(str_R, "XWA");	break;
	case 1:	
		if (size == 0)	sprintf(str_R, "WA");
		if (size == 1)	sprintf(str_R, "XBC");
		break;
	case 2: if (size == 1)	sprintf(str_R, "XDE");	break;
	case 3:
		if (size == 0)	sprintf(str_R, "BC");
		if (size == 1)	sprintf(str_R, "XHL");
		break;
	case 4: if (size == 1)	sprintf(str_R, "XIX");	break;
	case 5:	
		if (size == 0)	sprintf(str_R, "DE");
		if (size == 1)	sprintf(str_R, "XIY");
		break;
	case 6: if (size == 1)	sprintf(str_R, "XIZ");	break;
	case 7:
		if (size == 0)	sprintf(str_R, "HL");
		if (size == 1)	sprintf(str_R, "XSP");
		break;
	}
}

//=============================================================================

char instr[128];	//Print the disassembled instruction to this string
_u8 bytes[16];			//Stores the bytes used
_u8 bcnt;				//Byte Counter for above

//=============================================================================

_u8 get8_dis(void)
{
	_u8 a = bytes[bcnt++] = loadB(pc++);
	return a;
}

_u16 get16_dis(void)
{
	_u16 a = *(_u16*)(_u8*)(bytes + bcnt) = loadW(pc);
	pc += 2; bcnt += 2;
	return a;
}

_u32 get24_dis(void)
{
	_u8 b; _u16 a;
	a = *(_u16*)(_u8*)(bytes + bcnt) = loadW(pc);
	pc += 2; bcnt += 2;
	b = bytes[bcnt++] = loadB(pc++);
	return ((_u32)b << 16) | (_u32)a;
}

_u32 get32_dis(void)
{
	_u32 a = *(_u32*)(_u8*)(bytes + bcnt) = loadL(pc);
	pc += 4; bcnt += 4;
	return a;
}

//=========================================================================

static void src_B()	{ TLCS900h_disassemble_src(0); }
static void src_W()	{ TLCS900h_disassemble_src(1); }
static void src_L()	{ TLCS900h_disassemble_src(2); }
static void dst()	{ TLCS900h_disassemble_dst(); }
static void reg_B()	{ TLCS900h_disassemble_reg(0);}
static void reg_W()	{ TLCS900h_disassemble_reg(1);}
static void reg_L()	{ TLCS900h_disassemble_reg(2);}

//=========================================================================

//Single Byte Opcode

static void NOP()
{
	sprintf(instr, "NOP");
}

static void NORMAL()
{
	sprintf(instr, "NORMAL");
}

static void PUSHSR()
{
	sprintf(instr, "PUSH SR");
}

static void POPSR()
{
	sprintf(instr, "POP SR");
}

static void MAX()
{
	sprintf(instr, "MAX");
}

static void HALT()
{
	sprintf(instr, "HALT");
}

static void EI()
{
	_u8 value = get8_dis();

	if (value == 7)
		sprintf(instr, "DI");
	else
		sprintf(instr, "EI %d", value);
}

static void RETI()
{
	sprintf(instr, "RETI");
}

static void LD8_8()
{
	_u8 dst = get8_dis(), src = get8_dis();
	sprintf(instr, "LD (0x%02X),0x%02X", dst, src);
}

static void PUSH8()
{
	sprintf(instr, "PUSH 0x%02X", get8_dis());
}

static void LD8_16()
{
	_u8 dst = get8_dis();
	_u16 src = get16_dis();
	sprintf(instr, "LD.w (0x%02X),0x%04X", dst, src);
}

static void PUSH16()
{
	sprintf(instr, "PUSH 0x%04X", get16_dis());
}

static void INCF()
{
	sprintf(instr, "INCF");
}

static void DECF()
{
	sprintf(instr, "DECF");
}

static void RET()
{
	sprintf(instr, "RET");
}

static void RETD()
{
	sprintf(instr, "RETD %d", get16_dis());
}

static void RCF()
{
	sprintf(instr, "RCF");
}

static void SCF()
{
	sprintf(instr, "SCF");
}

static void CCF()
{
	sprintf(instr, "CCF");
}

static void ZCF()
{
	sprintf(instr, "ZCF");
}

static void PUSHA()
{
	sprintf(instr, "PUSH A");
}

static void POPA()
{
	sprintf(instr, "POP A");
}

static void EX()
{
	sprintf(instr, "EX F,F'");
}

static void LDF()
{
	sprintf(instr, "LDF 0x%02X", get8_dis());
}

static void PUSHF()
{
	sprintf(instr, "PUSH F");
}

static void POPF()
{
	sprintf(instr, "POP F");
}

static void JP16()
{
	sprintf(instr, "JP 0x%04X", get16_dis());
}

static void JP24()
{
	sprintf(instr, "JP 0x%06X", get24_dis());
}

static void CALL16()
{
	sprintf(instr, "CALL 0x%04X", get16_dis());
}

static void CALL24()
{
	sprintf(instr, "CALL 0x%06X", get24_dis());
}

static void CALR()
{
	sprintf(instr, "CALR 0x%06X", (_s16)get16_dis() + pc);
}

static void LDB()
{
	sprintf(instr, "LD %s,0x%02X", gprName[first & 7][0], get8_dis());
}

static void PUSHW()
{
	sprintf(instr, "PUSH %s", gprName[first & 7][1]);
}

static void LDW()
{
	sprintf(instr, "LD %s,0x%04X", gprName[first & 7][1], get16_dis());
}

static void PUSHL()
{
	sprintf(instr, "PUSH %s", gprName[first & 7][2]);
}

static void LDL()
{
	sprintf(instr, "LD %s,0x%08X", gprName[first & 7][2], get32_dis());
}

static void POPW()
{
	sprintf(instr, "POP %s", gprName[first & 7][1]);
}

static void POPL()
{
	sprintf(instr, "POP %s", gprName[first & 7][2]);
}

static void JR()
{
	sprintf(instr, "JR %s,0x%06X", ccName[first & 0xF], (_s8)get8_dis() + pc);
}

static void JRL()
{
	sprintf(instr, "JRL %s,0x%06X", ccName[first & 0xF], (_s16)get16_dis() + pc);
}

static void LDX()
{
	_u8 dst, src;

	get8_dis();			//00
	dst = get8_dis();	//#8
	get8_dis();			//00
	src = get8_dis();	//#
	get8_dis();			//00
	sprintf(instr, "LDX (0x%02X),0x%02X", dst, src);
}

static void SWI()
{
	sprintf(instr, "SWI %d", first & 7);
}

//=========================================================================

static void dBIOSHLE()
{
	sprintf(instr, "BIOS-HLE");
}

//=========================================================================

//Primary Instruction decode
static void (*decode[256])() = 
{
/*0*/	NOP,	NORMAL,	PUSHSR,	POPSR,	MAX,	HALT,	EI,		RETI,
		LD8_8,	PUSH8,	LD8_16,	PUSH16,	INCF,	DECF,	RET,	RETD,
/*1*/	RCF,	SCF,	CCF,	ZCF,	PUSHA,	POPA,	EX,		LDF,
		PUSHF,	POPF,	JP16,	JP24,	CALL16,	CALL24,	CALR,	dBIOSHLE,
/*2*/	LDB,	LDB,	LDB,	LDB,	LDB,	LDB,	LDB,	LDB,
		PUSHW,	PUSHW,	PUSHW,	PUSHW,	PUSHW,	PUSHW,	PUSHW,	PUSHW,
/*3*/	LDW,	LDW,	LDW,	LDW,	LDW,	LDW,	LDW,	LDW,
		PUSHL,	PUSHL,	PUSHL,	PUSHL,	PUSHL,	PUSHL,	PUSHL,	PUSHL,
/*4*/	LDL,	LDL,	LDL,	LDL,	LDL,	LDL,	LDL,	LDL,
		POPW,	POPW,	POPW,	POPW,	POPW,	POPW,	POPW,	POPW,
/*5*/	0,		0,		0,		0,		0,		0,		0,		0,
		POPL,	POPL,	POPL,	POPL,	POPL,	POPL,	POPL,	POPL,
/*6*/	JR,		JR,		JR,		JR,		JR,		JR,		JR,		JR,
		JR,		JR,		JR,		JR,		JR,		JR,		JR,		JR,
/*7*/	JRL,	JRL,	JRL,	JRL,	JRL,	JRL,	JRL,	JRL,
		JRL,	JRL,	JRL,	JRL,	JRL,	JRL,	JRL,	JRL,
/*8*/	src_B,	src_B,	src_B,	src_B,	src_B,	src_B,	src_B,	src_B,
		src_B,	src_B,	src_B,	src_B,	src_B,	src_B,	src_B,	src_B,
/*9*/	src_W,	src_W,	src_W,	src_W,	src_W,	src_W,	src_W,	src_W,
		src_W,	src_W,	src_W,	src_W,	src_W,	src_W,	src_W,	src_W,
/*A*/	src_L,	src_L,	src_L,	src_L,	src_L,	src_L,	src_L,	src_L,
		src_L,	src_L,	src_L,	src_L,	src_L,	src_L,	src_L,	src_L,
/*B*/	dst,	dst,	dst,	dst,	dst,	dst,	dst,	dst,
		dst,	dst,	dst,	dst,	dst,	dst,	dst,	dst,
/*C*/	src_B,	src_B,	src_B,	src_B,	src_B,	src_B,	0,		reg_B,
		reg_B,	reg_B,	reg_B,	reg_B,	reg_B,	reg_B,	reg_B,	reg_B,
/*D*/	src_W,	src_W,	src_W,	src_W,	src_W,	src_W,	0,		reg_W,
		reg_W,	reg_W,	reg_W,	reg_W,	reg_W,	reg_W,	reg_W,	reg_W,
/*E*/	src_L,	src_L,	src_L,	src_L,	src_L,	src_L,	0,		reg_L,
		reg_L,	reg_L,	reg_L,	reg_L,	reg_L,	reg_L,	reg_L,	reg_L,
/*F*/	dst,	dst,	dst,	dst,	dst,	dst,	0,		LDX,
		SWI,	SWI,	SWI,	SWI,	SWI,	SWI,	SWI,	SWI
};

//=============================================================================

char* TLCS900h_disassemble(void)
{
	_u8 str[80], i;
	memset(str, 0, 80);
	
	//Reset
	bcnt = 0;
	brCode = FALSE;
	sprintf(instr, "unknown");
	sprintf(extra, "unknown");

	//Fix big addresses
	pc &= 0xFFFFFF;

	//Add the program counter
	sprintf(str, "%06X: ", pc);

	first = get8_dis();	//Get the first opcode

	//Disassemble
	if (decode[first])
	{
		//Decode any extra data
		TLCS900h_disassemble_extra();
		(*decode[first])();
	}

	//Add the instruction
	strcat(str, instr);

	//Add the bytes used
	for (i = strlen(str); i < 32; i++)
		str[i] = ' ';
	str[32] = '\"';
	for (i = 0; i < bcnt; i++)
	{
		_u8 tmp[80];
		sprintf(tmp, "%02X ", bytes[i]);
		strcat(str, tmp);
	}
	str[strlen(str) - 1] = '\"';

	return strdup(str);
}

//=============================================================================
#endif