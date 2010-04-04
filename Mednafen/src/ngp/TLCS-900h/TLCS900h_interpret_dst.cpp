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

	TLCS900h_interpret_dst.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

22 JUL 2002 - neopop_uk
=======================================
- Added ANDCF,ORCF and XORCF in # and A modes. These were being used
	by one of the obscure pachinko "games".

23 JUL 2002 - neopop_uk
=======================================
- Added cycle count for TSET.

16 AUG 2002 - neopop_uk
=======================================
- Replaced 'second & 7' with 'R', clearer, faster - and for some reason
	more accurate... oh well!

21 AUG 2002 - neopop_uk
=======================================
- Added TSET.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "TLCS900h_interpret.h"
#include "TLCS900h_registers.h"
#include "mem.h"

namespace TLCS900H
{

//=========================================================================

//===== LD (mem),#
void dstLDBi()
{
	storeB(mem, FETCH8);
	cycles = 5;
}

//===== LD (mem),#
void dstLDWi()
{
	storeW(mem, fetch16());
	cycles = 6;
}

//===== POP (mem)
void dstPOPB()
{
	storeB(mem, pop8());
	cycles = 6;
}

//===== POP (mem)
void dstPOPW()
{
	storeW(mem, pop16());
	cycles = 6;
}

//===== LD (mem),(nn)
void dstLDBm16()
{
	storeB(mem, loadB(fetch16()));
	cycles = 8;
}

//===== LD (mem),(nn)
void dstLDWm16()
{
	storeW(mem, loadW(fetch16()));
	cycles = 8;
}

//===== LDA R,mem
void dstLDAW()
{
	regW(R) = (uint16)mem;
	cycles = 4;
}

//===== LDA R,mem
void dstLDAL()
{
	regL(R) = (uint32)mem;
	cycles = 4;
}

//===== ANDCF A,(mem)
void dstANDCFA()
{
	uint8 bit = REGA & 0xF;
	uint8 mbit = (loadB(mem) >> bit) & 1;
	if (bit < 8) SETFLAG_C(mbit & FLAG_C);
	cycles = 8;
}

//===== ORCF A,(mem)
void dstORCFA()
{
	uint8 bit = REGA & 0xF;
	uint8 mbit = (loadB(mem) >> bit) & 1;
	if (bit < 8) SETFLAG_C(mbit | FLAG_C);
	cycles = 8;
}

//===== XORCF A,(mem)
void dstXORCFA()
{
	uint8 bit = REGA & 0xF;
	uint8 mbit = (loadB(mem) >> bit) & 1;
	if (bit < 8) SETFLAG_C(mbit ^ FLAG_C);
	cycles = 8;
}

//===== LDCF A,(mem)
void dstLDCFA()
{
	uint8 bit = REGA & 0xF;
	uint8 mask = (1 << bit);
	if (bit < 8) SETFLAG_C(loadB(mem) & mask);
	cycles = 8;
}

//===== STCF A,(mem)
void dstSTCFA()
{
	uint8 bit = REGA & 0xF;
	uint8 cmask = ~(1 << bit);
	uint8 set = FLAG_C << bit;
	if (bit < 8) storeB(mem, (loadB(mem) & cmask) | set); 
	cycles = 8;
}

//===== LD (mem),R
void dstLDBR()
{
	storeB(mem, regB(R));
	cycles = 4;
}

//===== LD (mem),R
void dstLDWR()
{
	storeW(mem, regW(R));
	cycles = 4;
}

//===== LD (mem),R
void dstLDLR()
{
	storeL(mem, regL(R));
	cycles = 6;
}

//===== ANDCF #3,(mem)
void dstANDCF()
{
	uint8 bit = R;
	uint8 mbit = (loadB(mem) >> bit) & 1;
	SETFLAG_C(mbit & FLAG_C);
	cycles = 8;
}

//===== ORCF #3,(mem)
void dstORCF()
{
	uint8 bit = R;
	uint8 mbit = (loadB(mem) >> bit) & 1;
	SETFLAG_C(mbit | FLAG_C);
	cycles = 8;
}

//===== XORCF #3,(mem)
void dstXORCF()
{
	uint8 bit = R;
	uint8 mbit = (loadB(mem) >> bit) & 1;
	SETFLAG_C(mbit ^ FLAG_C);
	cycles = 8;
}

//===== LDCF #3,(mem)
void dstLDCF()
{
	uint8 bit = R;
	uint32 mask = (1 << bit);
	SETFLAG_C(loadB(mem) & mask);
	cycles = 8;
}

//===== STCF #3,(mem)
void dstSTCF()
{
	uint8 bit = R;
	uint8 cmask = ~(1 << bit);
	uint8 set = FLAG_C << bit;
	storeB(mem, (loadB(mem) & cmask) | set); 
	cycles = 8;
}

//===== TSET #3,(mem)
void dstTSET()
{
	SETFLAG_Z(! (loadB(mem) & (1 << R)) );
	storeB(mem, loadB(mem) | (1 << R));

	SETFLAG_H1
	SETFLAG_N0
	cycles = 10;
}

//===== RES #3,(mem)
void dstRES()
{
	storeB(mem, loadB(mem) & (~(1 << R)));
	cycles = 8;
}

//===== SET #3,(mem)
void dstSET()
{
	storeB(mem, loadB(mem) | (1 << R));
	cycles = 8;
}

//===== CHG #3,(mem)
void dstCHG()
{
	storeB(mem, loadB(mem) ^ (1 << R));
	cycles = 8;
}

//===== BIT #3,(mem)
void dstBIT()
{
	SETFLAG_Z(! (loadB(mem) & (1 << R)) );
	SETFLAG_H1;
	SETFLAG_N0;
	cycles = 8;
}

//===== JP cc,mem
void dstJP()
{
	if (conditionCode(second & 0xF))
	{
		pc = mem;
		cycles = 9;
	}
	else
	{
		cycles = 6;
	}
}

//===== CALL cc,mem
void dstCALL()
{
	if (conditionCode(second & 0xF))
	{
		push32(pc);
		pc = mem;
		cycles = 12;
	}
	else
	{
		cycles = 6;
	}
}

//===== RET cc
void dstRET()
{
	if (conditionCode(second & 0xF))
	{
		pc = pop32();
		cycles = 12;
	}
	else
	{
		cycles = 6;
	}
}
};
//=============================================================================
