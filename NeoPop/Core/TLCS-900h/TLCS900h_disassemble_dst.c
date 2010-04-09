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

	TLCS900h_disassemble_dst.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

#ifdef NEOPOP_DEBUG
//=========================================================================

#include "neopop.h"
#include "TLCS900h_disassemble.h"
#include "TLCS900h_registers.h"
#include "TLCS900h_interpret.h"

//=========================================================================

static void LDBi()
{
	sprintf(instr, "LD (%s),0x%02X", extra, get8_dis());
}

static void LDWi()
{
	sprintf(instr, "LD (%s),0x%04X", extra, get16_dis());
}

static void POPB()
{
	sprintf(instr, "POP.b (%s)", extra);
}

static void POPW()
{
	sprintf(instr, "POP.w (%s)", extra);
}

static void LDBm16()
{
	sprintf(instr, "LD.b (%s),(0x%04X)", extra, get16_dis());
}

static void LDWm16()
{
	sprintf(instr, "LD.w (%s),(0x%04X)", extra, get16_dis());
}

static void LDAW()
{
	sprintf(instr, "LDA %s,%s", gprName[second & 7][1], extra);
}

static void LDAL()
{
	sprintf(instr, "LDA %s,%s", gprName[second & 7][2], extra);
}

static void ANDCFA()
{
	sprintf(instr, "ANDCF A,(%s)", extra);
}

static void ORCFA()
{
	sprintf(instr, "ORCF A,(%s)", extra);
}

static void XORCFA()
{
	sprintf(instr, "XORCF A,(%s)", extra);
}

static void LDCFA()
{
	sprintf(instr, "LDCF A,(%s)", extra);
}

static void STCFA()
{
	sprintf(instr, "STCF A,(%s)", extra);
}

static void LDBR()
{
	sprintf(instr, "LD (%s),%s", extra, gprName[second&7][0]);
}

static void LDWR()
{
	sprintf(instr, "LD (%s),%s", extra, gprName[second&7][1]);
}

static void LDLR()
{
	sprintf(instr, "LD (%s),%s", extra, gprName[second&7][2]);
}

static void ANDCF()
{
	sprintf(instr, "ANDCF %d,(%s)", second & 7, extra);
}

static void ORCF()
{
	sprintf(instr, "ORCF %d,(%s)", second & 7, extra);
}

static void XORCF()
{
	sprintf(instr, "XORCF %d,(%s)", second & 7, extra);
}

static void LDCF()
{
	sprintf(instr, "LDCF %d,(%s)", second & 7, extra);
}

static void STCF()
{
	sprintf(instr, "STCF %d,(%s)", second & 7, extra);
}

static void TSET()
{
	sprintf(instr, "TSET %d,(%s)", second & 7, extra);
}

static void RES()
{
	sprintf(instr, "RES %d,(%s)", second & 7, extra);
}

static void SET()
{
	sprintf(instr, "SET %d,(%s)", second & 7, extra);
}

static void CHG()
{
	sprintf(instr, "CHG %d,(%s)", second & 7, extra);
}

static void BIT()
{
	sprintf(instr, "BIT %d,(%s)", second & 7, extra);
}

static void JP()
{
	sprintf(instr, "JP %s,%s", ccName[second & 0xF], extra);
}

static void CALL()
{
	sprintf(instr, "CALL %s,%s", ccName[second & 0xF], extra);
}

static void RET()
{
	sprintf(instr, "RET %s", ccName[second & 0xF]);
}

//=========================================================================

//Secondary (DST) Instruction decode
static void (*decode[256])() = 
{
/*0*/	LDBi,	0,		LDWi,	0,		POPB,	0,		POPW,	0,
		0,		0,		0,		0,		0,		0,		0,		0,
/*1*/	0,		0,		0,		0,		LDBm16,	0,		LDWm16,	0,
		0,		0,		0,		0,		0,		0,		0,		0,
/*2*/	LDAW,	LDAW,	LDAW,	LDAW,	LDAW,	LDAW,	LDAW,	LDAW,
		ANDCFA,	ORCFA,	XORCFA,	LDCFA,	STCFA,	0,		0,		0,
/*3*/	LDAL,	LDAL,	LDAL,	LDAL,	LDAL,	LDAL,	LDAL,	LDAL,
		0,		0,		0,		0,		0,		0,		0,		0,
/*4*/	LDBR,	LDBR,	LDBR,	LDBR,	LDBR,	LDBR,	LDBR,	LDBR,
		0,		0,		0,		0,		0,		0,		0,		0,
/*5*/	LDWR,	LDWR,	LDWR,	LDWR,	LDWR,	LDWR,	LDWR,	LDWR,
		0,		0,		0,		0,		0,		0,		0,		0,
/*6*/	LDLR,	LDLR,	LDLR,	LDLR,	LDLR,	LDLR,	LDLR,	LDLR,
		0,		0,		0,		0,		0,		0,		0,		0,
/*7*/	0,		0,		0,		0,		0,		0,		0,		0,
		0,		0,		0,		0,		0,		0,		0,		0,
/*8*/	ANDCF,	ANDCF,	ANDCF,	ANDCF,	ANDCF,	ANDCF,	ANDCF,	ANDCF,
		ORCF,	ORCF,	ORCF,	ORCF,	ORCF,	ORCF,	ORCF,	ORCF,
/*9*/	XORCF,	XORCF,	XORCF,	XORCF,	XORCF,	XORCF,	XORCF,	XORCF,
		LDCF,	LDCF,	LDCF,	LDCF,	LDCF,	LDCF,	LDCF,	LDCF,
/*A*/	STCF,	STCF,	STCF,	STCF,	STCF,	STCF,	STCF,	STCF,	
		TSET,	TSET,	TSET,	TSET,	TSET,	TSET,	TSET,	TSET,
/*B*/	RES,	RES,	RES,	RES,	RES,	RES,	RES,	RES,
		SET,	SET,	SET,	SET,	SET,	SET,	SET,	SET,
/*C*/	CHG,	CHG,	CHG,	CHG,	CHG,	CHG,	CHG,	CHG,
		BIT,	BIT,	BIT,	BIT,	BIT,	BIT,	BIT,	BIT,
/*D*/	JP,		JP,		JP,		JP,		JP,		JP,		JP,		JP,
		JP,		JP,		JP,		JP,		JP,		JP,		JP,		JP,
/*E*/	CALL,	CALL,	CALL,	CALL,	CALL,	CALL,	CALL,	CALL,
		CALL,	CALL,	CALL,	CALL,	CALL,	CALL,	CALL,	CALL,
/*F*/	RET,	RET,	RET,	RET,	RET,	RET,	RET,	RET,
		RET,	RET,	RET,	RET,	RET,	RET,	RET,	RET
};

//=============================================================================

void TLCS900h_disassemble_dst(void)
{
	second = get8_dis();	//Get the second opcode

	if (decode[second])
		(*decode[second])();
	else
		sprintf(instr, "unknown dst instr. %02X", second);
}

//=============================================================================
#endif
