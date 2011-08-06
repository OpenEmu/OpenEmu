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

	TLCS900h_disassemble_extra.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

02 SEP 2002 - neopop_uk
=======================================
- Added the undocumented type 0x13 R32 address mode.

//---------------------------------------------------------------------------
*/

//=========================================================================

#include "neopop.h"
#include "TLCS900h_disassemble.h"
#include "TLCS900h_interpret.h"

#define RCN_fetch(_a, _b) (regCodeName[(_a)][(_b)] ? regCodeName[(_a)][(_b)] : "-UNK-")

namespace TLCS900H
{

//=========================================================================

char extra[256];	//Print the mnemonic for the addressing mode here.

//=========================================================================

static void ExXWA()	{sprintf(extra, "XWA");}
static void ExXBC()	{sprintf(extra, "XBC");}
static void ExXDE()	{sprintf(extra, "XDE");}
static void ExXHL()	{sprintf(extra, "XHL");}
static void ExXIX()	{sprintf(extra, "XIX");}
static void ExXIY()	{sprintf(extra, "XIY");}
static void ExXIZ()	{sprintf(extra, "XIZ");}
static void ExXSP()	{sprintf(extra, "XSP");}

static void ExXWAd()	{sprintf(extra, "XWA %+d", (int8)get8_dis());}
static void ExXBCd()	{sprintf(extra, "XBC %+d", (int8)get8_dis());}
static void ExXDEd()	{sprintf(extra, "XDE %+d", (int8)get8_dis());}
static void ExXHLd()	{sprintf(extra, "XHL %+d", (int8)get8_dis());}
static void ExXIXd()	{sprintf(extra, "XIX %+d", (int8)get8_dis());}
static void ExXIYd()	{sprintf(extra, "XIY %+d", (int8)get8_dis());}
static void ExXIZd()	{sprintf(extra, "XIZ %+d", (int8)get8_dis());}
static void ExXSPd()	{sprintf(extra, "XSP %+d", (int8)get8_dis());}

static void Ex8()		{sprintf(extra, "0x%02X", get8_dis());}
static void Ex16()		{sprintf(extra, "0x%04X", get16_dis());}
static void Ex24()		{sprintf(extra, "0x%06X", get24_dis());}

static void ExR32()
{
	uint8 data = get8_dis();

	if (data == 0x03)
	{
		uint8 rIndex, r32;
		r32 = get8_dis();	//r32, upper 6 bits
		rIndex = get8_dis();	//r8 / r16
		sprintf(extra, "%s + %s", 
			RCN_fetch(2, r32 >> 2), RCN_fetch(0, rIndex >> 0));
		return;
	}

	if (data == 0x07)
	{
		uint8 rIndex, r32;
		r32 = get8_dis();	//r32, upper 6 bits
		rIndex = get8_dis();	//r8 / r16
		sprintf(extra, "%s + %s", 
			RCN_fetch(2, r32 >> 2), RCN_fetch(1, rIndex >> 1));
		return;
	}

	//Undocumented mode.
	if (data == 0x13)
	{
		sprintf(extra, "pc %+d", (int16)get16_dis()); 
		return;
	}

	if ((data & 3) == 1)
		sprintf(extra, "%s %+d", RCN_fetch(2, data >> 2), (int16)get16_dis()); 
	else
		sprintf(extra, "%s", RCN_fetch(2, data >> 2)); 
}

static void ExDec()
{
	uint8 data = get8_dis();
	uint8 r32 = data & 0xFC;

	switch(data & 3)
	{
	case 0:	sprintf(extra, "1--%s", RCN_fetch(2, r32 >> 2));	break;
	case 1:	sprintf(extra, "2--%s", RCN_fetch(2, r32 >> 2));	break;
	case 2:	sprintf(extra, "4--%s", RCN_fetch(2, r32 >> 2));	break;
	}
}

static void ExInc()
{
	uint8 data = get8_dis();
	uint8 r32 = data & 0xFC;

	switch(data & 3)
	{
	case 0:	sprintf(extra, "%s++1", RCN_fetch(2, r32 >> 2));	break;
	case 1:	sprintf(extra, "%s++2", RCN_fetch(2, r32 >> 2));	break;
	case 2:	sprintf(extra, "%s++4", RCN_fetch(2, r32 >> 2));	break;
	}
}

static void ExRCB()
{
	uint8 data = get8_dis();
	sprintf(extra, "%s", RCN_fetch(0, data >> 0));
	brCode = TRUE;
}

static void ExRCW()
{
	uint8 data = get8_dis();
	sprintf(extra, "%s", RCN_fetch(1, data >> 1));
	brCode = TRUE;
}

static void ExRCL()
{
	uint8 data = get8_dis();
	sprintf(extra, "%s", RCN_fetch(2, data >> 2));
	brCode = TRUE;
}

//=========================================================================

//Address Mode & Register Code
static void (*decodeExtra[256])() = 
{
/*0*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*1*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*2*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*3*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*4*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*5*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*6*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*7*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*8*/	ExXWA,	ExXBC,	ExXDE,	ExXHL,	ExXIX,	ExXIY,	ExXIZ,	ExXSP,
		ExXWAd,	ExXBCd,	ExXDEd,	ExXHLd,	ExXIXd,	ExXIYd,	ExXIZd,	ExXSPd,
/*9*/	ExXWA,	ExXBC,	ExXDE,	ExXHL,	ExXIX,	ExXIY,	ExXIZ,	ExXSP,
		ExXWAd,	ExXBCd,	ExXDEd,	ExXHLd,	ExXIXd,	ExXIYd,	ExXIZd,	ExXSPd,
/*A*/	ExXWA,	ExXBC,	ExXDE,	ExXHL,	ExXIX,	ExXIY,	ExXIZ,	ExXSP,
		ExXWAd,	ExXBCd,	ExXDEd,	ExXHLd,	ExXIXd,	ExXIYd,	ExXIZd,	ExXSPd,
/*B*/	ExXWA,	ExXBC,	ExXDE,	ExXHL,	ExXIX,	ExXIY,	ExXIZ,	ExXSP,
		ExXWAd,	ExXBCd,	ExXDEd,	ExXHLd,	ExXIXd,	ExXIYd,	ExXIZd,	ExXSPd,
/*C*/	Ex8,	Ex16,	Ex24,	ExR32,	ExDec,	ExInc,	0,		ExRCB,
		0,		0,		0,		0,		0,		0,		0,		0,
/*D*/	Ex8,	Ex16,	Ex24,	ExR32,	ExDec,	ExInc,	0,		ExRCW,
		0,		0,		0,		0,		0,		0,		0,		0,
/*E*/	Ex8,	Ex16,	Ex24,	ExR32,	ExDec,	ExInc,	0,		ExRCL,
		0,		0,		0,		0,		0,		0,		0,		0,
/*F*/	Ex8,	Ex16,	Ex24,	ExR32,	ExDec,	ExInc,	0,		0,
		0,		0,		0,		0,		0,		0,		0,		0
};

void TLCS900h_disassemble_extra(void)
{
	//Is any extra data used by this instruction?
	if (decodeExtra[first])
		(*decodeExtra[first])();
}

};
