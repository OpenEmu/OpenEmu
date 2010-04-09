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

	TLCS900h_disassemble_src.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

28 JUL 2002 - neopop_uk
=======================================
- Better disassembly of MUL/MULS/DIV/DIVS - operand size is shown.
  
//---------------------------------------------------------------------------
*/

#ifdef NEOPOP_DEBUG
//=========================================================================

#include "neopop.h"
#include "TLCS900h_disassemble.h"
#include "TLCS900h_registers.h"
#include "TLCS900h_interpret.h"

//=========================================================================

static void PUSH()
{
	sprintf(instr, "PUSH (%s)", extra);
}

static void RLD()
{
	sprintf(instr, "RLD A,(%s)", extra);
}

static void RRD()
{
	sprintf(instr, "RRD A,(%s)", extra);
}

static void LDI()
{
	if ((first & 0xF) == 3)
	{		
		switch(size)
		{
		case 0:	sprintf(instr, "LDI.b (XDE+),(XHL+)");	break;
		case 1:	sprintf(instr, "LDI.w (XDE+),(XHL+)");	break;
		}
	}

	if ((first & 0xF) == 5)
	{
		switch(size)
		{
		case 0:	sprintf(instr, "LDI.b (XIX+),(XIY+)");	break;
		case 1:	sprintf(instr, "LDI.w (XIX+),(XIY+)");	break;
		}
	}
}

static void LDIR()
{
	if ((first & 0xF) == 3)
	{		
		switch(size)
		{
		case 0:	sprintf(instr, "LDIR.b (XDE+),(XHL+)");	break;
		case 1:	sprintf(instr, "LDIR.w (XDE+),(XHL+)");	break;
		}
	}

	if ((first & 0xF) == 5)
	{
		switch(size)
		{
		case 0:	sprintf(instr, "LDIR.b (XIX+),(XIY+)");	break;
		case 1:	sprintf(instr, "LDIR.w (XIX+),(XIY+)");	break;
		}
	}
}

static void LDD()
{
	if ((first & 0xF) == 3)
	{		
		switch(size)
		{
		case 0:	sprintf(instr, "LDD.b (XDE-),(XHL-)");	break;
		case 1:	sprintf(instr, "LDD.w (XDE-),(XHL-)");	break;
		}
	}

	if ((first & 0xF) == 5)
	{
		switch(size)
		{
		case 0:	sprintf(instr, "LDD.b (XIX-),(XIY-)");	break;
		case 1:	sprintf(instr, "LDD.w (XIX-),(XIY-)");	break;
		}
	}
}

static void LDDR()
{
	if ((first & 0xF) == 3)
	{		
		switch(size)
		{
		case 0:	sprintf(instr, "LDDR.b (XDE-),(XHL-)");	break;
		case 1:	sprintf(instr, "LDDR.w (XDE-),(XHL-)");	break;
		}
	}

	if ((first & 0xF) == 5)
	{
		switch(size)
		{
		case 0:	sprintf(instr, "LDDR.b (XIX-),(XIY-)");	break;
		case 1:	sprintf(instr, "LDDR.w (XIX-),(XIY-)");	break;
		}
	}
}

static void CPI()
{
	sprintf(instr, "CPI");
}

static void CPIR()
{
	switch(size)
	{
	case 0:	sprintf(instr, "CPIR.b A,(%s+)", gprName[first & 7][2]);	break;
	case 1:	sprintf(instr, "CPIR.w WA,(%s+)", gprName[first & 7][2]);	break;
	}
}

static void CPD()
{
	switch(size)
	{
	case 0:	sprintf(instr, "CPD.b A,(%s+)", gprName[first & 7][2]);	break;
	case 1:	sprintf(instr, "CPD.w WA,(%s+)", gprName[first & 7][2]);	break;
	}
}

static void CPDR()
{
	switch(size)
	{
	case 0:	sprintf(instr, "CPDR.b A,(%s+)", gprName[first & 7][2]);	break;
	case 1:	sprintf(instr, "CPDR.w WA,(%s+)", gprName[first & 7][2]);	break;
	}
}

static void LD16m()
{
	sprintf(instr, "LD (0x%04X),(%s)", get16_dis(), extra);
}

static void LD()
{
	sprintf(instr, "LD %s,(%s)", str_R, extra);
}

static void EX()
{
	switch(size)
	{
	case 0:	sprintf(instr, "EX.b (%s),%s", extra, str_R);	break;
	case 1:	sprintf(instr, "EX.w (%s),%s", extra, str_R);	break;
	}
	
}

static void ADDi()
{
	switch(size)
	{
	case 0: sprintf(instr, "ADD (%s), 0x%02X", extra, get8_dis());		break;
	case 1: sprintf(instr, "ADD (%s), 0x%04X", extra, get16_dis());		break;
	}
}

static void ADCi()
{
	switch(size)
	{
	case 0: sprintf(instr, "ADC (%s), 0x%02X", extra, get8_dis());		break;
	case 1: sprintf(instr, "ADC (%s), 0x%04X", extra, get16_dis());		break;
	}
}

static void SUBi()
{
	switch(size)
	{
	case 0: sprintf(instr, "SUB (%s), 0x%02X", extra, get8_dis());		break;
	case 1: sprintf(instr, "SUB (%s), 0x%04X", extra, get16_dis());		break;
	}
}

static void SBCi()
{
	switch(size)
	{
	case 0: sprintf(instr, "SBC (%s), 0x%02X", extra, get8_dis());		break;
	case 1: sprintf(instr, "SBC (%s), 0x%04X", extra, get16_dis());		break;
	}
}

static void ANDi()
{
	switch(size)
	{
	case 0: sprintf(instr, "AND (%s), 0x%02X", extra, get8_dis());		break;
	case 1: sprintf(instr, "AND (%s), 0x%04X", extra, get16_dis());		break;
	}
}

static void XORi()
{
	switch(size)
	{
	case 0: sprintf(instr, "XOR (%s), 0x%02X", extra, get8_dis());		break;
	case 1: sprintf(instr, "XOR (%s), 0x%04X", extra, get16_dis());		break;
	}
}

static void ORi()
{
	switch(size)
	{
	case 0: sprintf(instr, "OR (%s), 0x%02X", extra, get8_dis());		break;
	case 1: sprintf(instr, "OR (%s), 0x%04X", extra, get16_dis());		break;
	}
}

static void CPi()
{
	switch(size)
	{
	case 0:	sprintf(instr, "CP (%s),0x%02X", extra, get8_dis());	break;
	case 1:	sprintf(instr, "CP (%s),0x%04X", extra, get16_dis());	break;
	}
}

static void MUL()
{
	get_RR_Name();
	switch(size)
	{
	case 0:	sprintf(instr, "MUL.b (%s),(%s)",str_R, extra);	break;
	case 1:	sprintf(instr, "MUL.w (%s),(%s)",str_R, extra);	break;
	}
}

static void MULS()
{
	get_RR_Name();
	switch(size)
	{
	case 0:	sprintf(instr, "MULS.b (%s),(%s)",str_R, extra);	break;
	case 1:	sprintf(instr, "MULS.w (%s),(%s)",str_R, extra);	break;
	}
}

static void DIV()
{
	get_RR_Name();
	switch(size)
	{
	case 0:	sprintf(instr, "DIV.b (%s),(%s)",str_R, extra);	break;
	case 1:	sprintf(instr, "DIV.w (%s),(%s)",str_R, extra);	break;
	}
}

static void DIVS()
{
	get_RR_Name();
	switch(size)
	{
	case 0:	sprintf(instr, "DIVS.b (%s),(%s)",str_R, extra);	break;
	case 1:	sprintf(instr, "DIVS.w (%s),(%s)",str_R, extra);	break;
	}
}

static void INC()
{
	_u8 val = (second & 7);
	if (val == 0)
		val = 8;

	sprintf(instr, "INC %d,(%s)", val, extra);
}

static void DEC()
{
	_u8 val = (second & 7);
	if (val == 0)
		val = 8;

	sprintf(instr, "DEC %d,(%s)", val, extra);
}

static void RLC()
{
	sprintf(instr, "RLC (%s)", extra);
}

static void RRC()
{
	sprintf(instr, "RRC (%s)", extra);
}

static void RL()
{
	sprintf(instr, "RL (%s)", extra);
}

static void RR()
{
	sprintf(instr, "RR (%s)", extra);
}

static void SLA()
{
	sprintf(instr, "SLA (%s)", extra);
}

static void SRA()
{
	sprintf(instr, "SRA (%s)", extra);
}

static void SLL()
{
	sprintf(instr, "SLL (%s)", extra);
}

static void SRL()
{
	sprintf(instr, "SRL (%s)", extra);
}

static void ADDRm()
{
	sprintf(instr, "ADD %s,(%s)", str_R, extra);
}

static void ADDmR()
{
	sprintf(instr, "ADD (%s),%s", extra, str_R);
}

static void ADCRm()
{
	sprintf(instr, "ADC %s,(%s)", str_R, extra);
}

static void ADCmR()
{
	sprintf(instr, "ADC (%s),%s", extra, str_R);
}

static void SUBRm()
{
	sprintf(instr, "SUB %s,(%s)", str_R, extra);
}

static void SUBmR()
{
	sprintf(instr, "SUB (%s),%s", extra, str_R);
}

static void SBCRm()
{
	sprintf(instr, "SBC %s,(%s)", str_R, extra);
}

static void SBCmR()
{
	sprintf(instr, "SBC (%s),%s", extra, str_R);
}

static void ANDmR()
{
	sprintf(instr, "AND (%s),%s", extra, str_R);
}

static void ANDRm()
{
	sprintf(instr, "AND %s,(%s)", str_R, extra);
}

static void XORmR()
{
	sprintf(instr, "XOR (%s),%s", extra, str_R);
}

static void XORRm()
{
	sprintf(instr, "XOR %s,(%s)", str_R, extra);
}

static void ORmR()
{
	sprintf(instr, "OR (%s),%s", extra, str_R);
}

static void ORRm()
{
	sprintf(instr, "OR %s,(%s)", str_R, extra);
}

static void CPmR()
{
	sprintf(instr, "CP (%s),%s", extra, str_R);
}

static void CPRm()
{
	sprintf(instr, "CP %s,(%s)", str_R, extra);
}

//=========================================================================

//Secondary (SRC) Instruction decode
static void (*decode[256])() = 
{
/*0*/	0,		0,		0,		0,		PUSH,	0,		RLD,	RRD,
		0,		0,		0,		0,		0,		0,		0,		0,
/*1*/	LDI,	LDIR,	LDD,	LDDR,	CPI,	CPIR,	CPD,	CPDR,
		0,		LD16m,	0,		0,		0,		0,		0,		0,
/*2*/	LD,		LD,		LD,		LD,		LD,		LD,		LD,		LD,
		0,		0,		0,		0,		0,		0,		0,		0,
/*3*/	EX,		EX,		EX,		EX,		EX,		EX,		EX,		EX,
		ADDi,	ADCi,	SUBi,	SBCi,	ANDi,	XORi,	ORi,	CPi,
/*4*/	MUL,	MUL,	MUL,	MUL,	MUL,	MUL,	MUL,	MUL,
		MULS,	MULS,	MULS,	MULS,	MULS,	MULS,	MULS,	MULS,
/*5*/	DIV,	DIV,	DIV,	DIV,	DIV,	DIV,	DIV,	DIV,
		DIVS,	DIVS,	DIVS,	DIVS,	DIVS,	DIVS,	DIVS,	DIVS,
/*6*/	INC,	INC,	INC,	INC,	INC,	INC,	INC,	INC,
		DEC,	DEC,	DEC,	DEC,	DEC,	DEC,	DEC,	DEC,
/*7*/	0,		0,		0,		0,		0,		0,		0,		0,
		RLC,	RRC,	RL,		RR,		SLA,	SRA,	SLL,	SRL,
/*8*/	ADDRm,	ADDRm,	ADDRm,	ADDRm,	ADDRm,	ADDRm,	ADDRm,	ADDRm,
		ADDmR,	ADDmR,	ADDmR,	ADDmR,	ADDmR,	ADDmR,	ADDmR,	ADDmR,
/*9*/	ADCRm,	ADCRm,	ADCRm,	ADCRm,	ADCRm,	ADCRm,	ADCRm,	ADCRm,
		ADCmR,	ADCmR,	ADCmR,	ADCmR,	ADCmR,	ADCmR,	ADCmR,	ADCmR,
/*A*/	SUBRm,	SUBRm,	SUBRm,	SUBRm,	SUBRm,	SUBRm,	SUBRm,	SUBRm,
		SUBmR,	SUBmR,	SUBmR,	SUBmR,	SUBmR,	SUBmR,	SUBmR,	SUBmR,
/*B*/	SBCRm,	SBCRm,	SBCRm,	SBCRm,	SBCRm,	SBCRm,	SBCRm,	SBCRm,
		SBCmR,	SBCmR,	SBCmR,	SBCmR,	SBCmR,	SBCmR,	SBCmR,	SBCmR,
/*C*/	ANDRm,	ANDRm,	ANDRm,	ANDRm,	ANDRm,	ANDRm,	ANDRm,	ANDRm,
		ANDmR,	ANDmR,	ANDmR,	ANDmR,	ANDmR,	ANDmR,	ANDmR,	ANDmR,
/*D*/	XORRm,	XORRm,	XORRm,	XORRm,	XORRm,	XORRm,	XORRm,	XORRm,
		XORmR,	XORmR,	XORmR,	XORmR,	XORmR,	XORmR,	XORmR,	XORmR,
/*E*/	ORRm,	ORRm,	ORRm,	ORRm,	ORRm,	ORRm,	ORRm,	ORRm,
		ORmR,	ORmR,	ORmR,	ORmR,	ORmR,	ORmR,	ORmR,	ORmR,
/*F*/	CPRm,	CPRm,	CPRm,	CPRm,	CPRm,	CPRm,	CPRm,	CPRm,
		CPmR,	CPmR,	CPmR,	CPmR,	CPmR,	CPmR,	CPmR,	CPmR
};

//=============================================================================

void TLCS900h_disassemble_src(int opsize)
{
	second = get8_dis();	//Get the second opcode
	size = opsize;

	//Prepare 'Big R'
	sprintf(str_R, "%s", gprName[second & 7][size]);

	if (decode[second])
		(*decode[second])();
	else
		sprintf(instr, "unknown src instr. %02X", second);
}

//=============================================================================
#endif
