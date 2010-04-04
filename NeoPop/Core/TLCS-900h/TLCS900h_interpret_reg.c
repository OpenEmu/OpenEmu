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

	TLCS900h_interpret_reg.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

22 JUL 2002 - neopop_uk
=======================================
- Added ANDCF,ORCF and XORCF in A mode and improved the remaining ones.

22 JUL 2002 - neopop_uk
=======================================
- Added LINK and UNLK to fix "Rockman Battle and Fighters"

23 JUL 2002 - neopop_uk
=======================================
- Added MIRR to fix "Card Fighters 2"
- Added cycle counts for TSET.

28 JUL 2002 - neopop_uk
=======================================
- Converted DIV/DIVS to use the generic function

16 AUG 2002 - neopop_uk
=======================================
- Removed all of the 'second & 7' with R as it's pre-calculated anyway.
- Fixed V flag emulation of INC/DEC, fixes "Cotton" menus
- Fixed MINC4

21 AUG 2002 - neopop_uk
=======================================
- Added TSET and MULA, both untested.

04 SEP 2002 - neopop_uk
=======================================
- Fixed GCC compatibility.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "TLCS900h_interpret.h"
#include "TLCS900h_registers.h"
#include "mem.h"
#include "dma.h"

//=========================================================================

//===== LD r,#
void regLDi()
{
	switch(size)
	{
	case 0:	rCodeB(rCode) = FETCH8;		cycles = 4; break;
	case 1:	rCodeW(rCode) = fetch16();	cycles = 4;	break;
	case 2: rCodeL(rCode) = fetch32();	cycles = 6;	break;
	}
}

//===== PUSH r
void regPUSH()
{
	switch(size)
	{
	case 0:	push8(rCodeB(rCode));  cycles = 5;break;
	case 1:	push16(rCodeW(rCode)); cycles = 5;break;
	case 2: push32(rCodeL(rCode)); cycles = 7;break;
	}
}

//===== POP r
void regPOP()
{
	switch(size)
	{
	case 0:	rCodeB(rCode) = pop8();		cycles = 6;break;
	case 1:	rCodeW(rCode) = pop16();	cycles = 6;break;
	case 2: rCodeL(rCode) = pop32(); 	cycles = 8;break;
	}
}

//===== CPL r
void regCPL()
{
	switch(size)
	{
	case 0: rCodeB(rCode) = ~ rCodeB(rCode);	break;
	case 1: rCodeW(rCode) = ~ rCodeW(rCode);	break;
	}

	SETFLAG_H1;
	SETFLAG_N1;
	cycles = 4;
}

//===== NEG r
void regNEG()
{
	switch(size)
	{
	case 0: rCodeB(rCode) = generic_SUB_B(0, rCodeB(rCode)); break;
	case 1: rCodeW(rCode) = generic_SUB_W(0, rCodeW(rCode)); break;
	}
	cycles = 5;
}

//===== MUL rr,#
void regMULi()
{
	_u8 target = get_rr_Target();
	if (target == 0x80)
	{
#ifdef NEOPOP_DEBUG
		instruction_error("reg: MULi bad \'rr\' dst code");
#endif
		return;
	}

	switch(size)
	{
	case 0: rCodeW(target) = (rCodeW(target) & 0xFF) * FETCH8;
		cycles = 18; break;
	case 1: rCodeL(target) = (rCodeL(target) & 0xFFFF) * fetch16();
		cycles = 26; break;
	}
}

//===== MULS rr,#
void regMULSi()
{
	_u8 target = get_rr_Target();
	if (target == 0x80)
	{
		instruction_error("reg: MULSi bad \'rr\' dst code");
		return;
	}

	switch(size)
	{
	case 0: rCodeW(target) = (_s8)(rCodeW(target) & 0xFF) * (_s8)FETCH8;
		cycles = 18; break;
	case 1: rCodeL(target) = (_s16)(rCodeL(target) & 0xFFFF) * (_s16)fetch16();
		cycles = 26; break;
	}
}

//===== DIV rr,#
void regDIVi()
{
	_u8 target = get_rr_Target();
	if (target == 0x80)
	{
		instruction_error("reg: DIVi bad \'rr\' dst code");
		return;
	}

	switch(size)
	{
	case 0: {	rCodeW(target) =  generic_DIV_B(rCodeW(target), FETCH8);
				cycles = 22;
				break;	}

	case 1: {	rCodeL(target) =  generic_DIV_W(rCodeL(target), fetch16());
				cycles = 30;
				break;	}
	}
}

//===== DIVS rr,#
void regDIVSi()
{
	_u8 target = get_rr_Target();
	if (target == 0x80)
	{
		instruction_error("reg: DIVSi bad \'rr\' dst code");
		return;
	}

	switch(size)
	{
	case 0: {	rCodeW(target) =  generic_DIVS_B(rCodeW(target), FETCH8);
				cycles = 24;
				break;	}

	case 1: {	rCodeL(target) =  generic_DIVS_W(rCodeL(target), fetch16());
				cycles = 32;
				break;	}
	}
}

//===== LINK r,dd
void regLINK()
{
	_s16 d = (_s16)fetch16();
	push32(rCodeL(rCode));
	rCodeL(rCode) = REGXSP;
	REGXSP += d;
	cycles = 10;
}

//===== UNLK r
void regUNLK()
{
	REGXSP = rCodeL(rCode);
	rCodeL(rCode) = pop32();
	cycles = 8;
}

//===== BS1F A,r
void regBS1F()
{
	_u16 data = rCodeW(rCode), mask = 0x0001;
	_u8 i;
	
	SETFLAG_V0;
	for (i = 0; i < 15; i++)
	{
		if (data & mask)
		{
			REGA = i;
			return;
		}

		mask <<= 1;
	}

	SETFLAG_V1;
	cycles = 4;
}

//===== BS1B A,r
void regBS1B()
{
	_u16 data = rCodeW(rCode), mask = 0x8000;
	_u8 i;
	
	SETFLAG_V0;
	for (i = 0; i < 15; i++)
	{
		if (data & mask)
		{
			REGA = 15 - i;
			return;
		}

		mask >>= 1;
	}

	SETFLAG_V1;
	cycles = 4;
}

//===== DAA r
void regDAA()
{
	_u16 resultC;
	_u8 src = rCodeB(rCode), result, added, half;
	BOOL setC = FALSE;

	_u8 upper4 = (src & 0xF0);
	_u8 lower4 = (src & 0x0F);

	if (FLAG_C)	// {C = 1}
	{
		if (FLAG_H)	// {H = 1}
		{
			setC = TRUE;
			added = 0x66;
		}
		else		// {H = 0}
		{
			if      (lower4 < 0x0a)		{ added = 0x60; }
			else						{ added = 0x66; }
			setC = TRUE;
		}
	}
	else	// {C = 0}
	{
		if (FLAG_H)	// {H = 1}
		{
			if		(src < 0x9A)		{ added = 0x06; }
			else						{ added = 0x66; }
		}
		else		// {H = 0}
		{
			if		((upper4 < 0x90) && (lower4 > 0x9))	{ added = 0x06; }
			else if ((upper4 > 0x80) && (lower4 > 0x9))	{ added = 0x66; }
			else if ((upper4 > 0x90) && (lower4 < 0xa))	{ added = 0x60; }
		}
	}

	if (FLAG_N)
	{
		resultC = (_u16)src - (_u16)added;
		half = (src & 0xF) - (added & 0xF);
	}
	else
	{
		resultC = (_u16)src + (_u16)added;
		half = (src & 0xF) + (added & 0xF);
	}

	result = (_u8)(resultC & 0xFF);	

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if (FLAG_N)		SETFLAG_C(result > src || setC)
	else			SETFLAG_C(result < src || setC)
	
	parityB(result);
	rCodeB(rCode) = result;
	cycles = 6;
}

//===== EXTZ r
void regEXTZ()
{
	switch(size)
	{
	case 1:	rCodeW(rCode) &= 0xFF;	break;
	case 2: rCodeL(rCode) &= 0xFFFF;	break;
	}

	cycles = 4;
}

//===== EXTS r
void regEXTS()
{
	switch(size)
	{
	case 1:	if (rCodeW(rCode) & 0x0080) 
				{ rCodeW(rCode) |= 0xFF00; } else 
				{ rCodeW(rCode) &= 0x00FF; }
		break;
		
	case 2:	if (rCodeL(rCode) & 0x00008000) 
				{ rCodeL(rCode) |= 0xFFFF0000; } else 
				{ rCodeL(rCode) &= 0x0000FFFF; }
		break;
	}

	cycles = 5;
}

//===== PAA r
void regPAA()
{
	switch(size)
	{
	case 1:	if (rCodeW(rCode) & 0x1) rCodeW(rCode)++; break;
	case 2:	if (rCodeL(rCode) & 0x1) rCodeL(rCode)++; break;
	}
	cycles = 4;
}

//===== MIRR r
void regMIRR()
{
	_u16 src = rCodeW(rCode), dst = 0, bit;

	//Undocumented - see p165 of CPU .PDF
	//Seems to mirror bits completely, ie. 1234 -> 4321

	for (bit = 0; bit < 16; bit++)
		if (src & (1 << bit))
			dst |= (1 << (15 - bit));

	rCodeW(rCode) = dst;
	cycles = 4;
}

//===== MULA rr
void regMULA()
{
	_u32 src = (_s16)loadW(regL(2/*XDE*/)) * (_s16)loadW(regL(3/*XHL*/));
	_u32 dst = rCodeL(rCode);
	_u32 result = dst + src;

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((_s32)dst >= 0) && ((_s32)src >= 0) && ((_s32)result < 0)) || 
		(((_s32)dst < 0)  && ((_s32)src < 0) && ((_s32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	cycles = 31;
}

//===== DJNZ r,d
void regDJNZ()
{
	_s8 offset = FETCH8;

	cycles = 7;

	switch(size)
	{
	case 0: 
		rCodeB(rCode) --;
		if (rCodeB(rCode) != 0)
		{
			cycles = 11;
			pc = pc + offset;
		}
		break;

	case 1: 
		rCodeW(rCode) --;
		if (rCodeW(rCode) != 0)
		{
			cycles = 11;
			pc = pc + offset;
		}
		break;
	}
}

//===== ANDCF #,r
void regANDCFi()
{
	_u8 data, bit = FETCH8 & 0xF;
	switch(size)
	{
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C & data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C & data); 
				break; }
	}
	cycles = 4;
}

//===== ORCF #,r
void regORCFi()
{
	_u8 data, bit = FETCH8 & 0xF;
	switch(size)
	{
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C | data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C | data); 
				break; }
	}
	cycles = 4;
}

//===== XORCF #,r
void regXORCFi()
{
	_u8 data, bit = FETCH8 & 0xF;
	switch(size)
	{
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C ^ data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C ^ data); 
				break; }
	}
	cycles = 4;
}

//===== LDCF #,r
void regLDCFi()
{
	_u8 bit = FETCH8 & 0xF;
	switch(size)
	{
	case 0: {	_u8 mask = (1 << bit);
				if (bit < 8)
					SETFLAG_C(rCodeB(rCode) & mask);
				break; }


	case 1: {	_u16 mask = (1 << bit);
				SETFLAG_C(rCodeW(rCode) & mask);
				break; }
	}

	cycles = 4;
}

//===== STCF #,r
void regSTCFi()
{
	_u8 bit = FETCH8 & 0xF;
	switch(size)
	{
	case 0: {	_u8 cmask = ~(1 << bit);
				_u8 set = FLAG_C << bit;
				if (bit < 8) rCodeB(rCode) = (rCodeB(rCode) & cmask) | set;
				break;	}

	case 1: {	_u16 cmask = ~(1 << bit);
				_u16 set = FLAG_C << bit;
				rCodeW(rCode) = (rCodeW(rCode) & cmask) | set;
				break;	}
	}

	cycles = 4;
}

//===== ANDCF A,r
void regANDCFA()
{
	_u8 data, bit = REGA & 0xF;
	switch(size)
	{
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C & data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C & data); 
				break; }
	}
	cycles = 4;
}

//===== ORCF A,r
void regORCFA()
{
	_u8 data, bit = REGA & 0xF;
	switch(size)
	{
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C | data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C | data); 
				break; }
	}
	cycles = 4;
}

//===== XORCF A,r
void regXORCFA()
{
	_u8 data, bit = REGA & 0xF;
	switch(size)
	{
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C ^ data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C ^ data); 
				break; }
	}
	cycles = 4;
}

//===== LDCF A,r
void regLDCFA()
{
	_u8 bit = REGA & 0xF;
	_u32 mask = (1 << bit);

	switch(size)
	{
	case 0: if (bit < 8) SETFLAG_C(rCodeB(rCode) & mask); break;
	case 1: SETFLAG_C(rCodeW(rCode) & mask); break;
	}

	cycles = 4;
}

//===== STCF A,r
void regSTCFA()
{
	switch(size)
	{
	case 0: {	_u8 bit = REGA & 0xF;
				_u8 cmask = ~(1 << bit);
				_u8 set = FLAG_C << bit;
				if (bit < 8) rCodeB(rCode) = (rCodeB(rCode) & cmask) | set;
				break;	}

	case 1: {	_u8 bit = REGA & 0xF;
				_u16 cmask = ~(1 << bit);
				_u16 set = FLAG_C << bit;
				rCodeW(rCode) = (rCodeW(rCode) & cmask) | set;
				break;	}
	}

	cycles = 4;
}

//===== LDC cr,r
void regLDCcrr()
{
	_u8 cr = FETCH8;

	switch(size)
	{
	case 0: dmaStoreB(cr, rCodeB(rCode)); break;
	case 1: dmaStoreW(cr, rCodeW(rCode)); break;
	case 2: dmaStoreL(cr, rCodeL(rCode)); break;
	}

	cycles = 8;
}

//===== LDC r,cr
void regLDCrcr()
{
	_u8 cr = FETCH8;

	switch(size)
	{
	case 0: rCodeB(rCode) = dmaLoadB(cr); break;
	case 1: rCodeW(rCode) = dmaLoadW(cr); break;
	case 2: rCodeL(rCode) = dmaLoadL(cr); break;
	}

	cycles = 8;
}

//===== RES #,r
void regRES()
{
	_u8 b = FETCH8 & 0xF;

	switch(size)
	{
	case 0: rCodeB(rCode) &= ~(_u8)(1 << b); break;
	case 1: rCodeW(rCode) &= ~(_u16)(1 << b); break;
	}

	cycles = 4;
}

//===== SET #,r
void regSET()
{
	_u8 b = FETCH8 & 0xF;

	switch(size)
	{
	case 0: rCodeB(rCode) |= (1 << b); break;
	case 1: rCodeW(rCode) |= (1 << b); break;
	}

	cycles = 4;
}

//===== CHG #,r
void regCHG()
{
	_u8 b = FETCH8 & 0xF;

	switch(size)
	{
	case 0: rCodeB(rCode) ^= (1 << b); break;
	case 1: rCodeW(rCode) ^= (1 << b); break;
	}

	cycles = 4;
}

//===== BIT #,r
void regBIT()
{
	_u8 b = FETCH8 & 0xF;
	
	switch(size)
	{
	case 0:	SETFLAG_Z(! (rCodeB(rCode) & (1 << b))	);	break;
	case 1:	SETFLAG_Z(! (rCodeW(rCode) & (1 << b))	);	break;
	}

	SETFLAG_H1;
	SETFLAG_N0;
	cycles = 4;
}

//===== TSET #,r
void regTSET()
{
	_u8 b = FETCH8 & 0xF;
	
	switch(size)
	{
	case 0:	SETFLAG_Z(! (rCodeB(rCode) & (1 << b))	);	
			rCodeB(rCode) |= (1 << b);
			break;

	case 1:	SETFLAG_Z(! (rCodeW(rCode) & (1 << b))	);
			rCodeW(rCode) |= (1 << b);
			break;
	}

	SETFLAG_H1
	SETFLAG_N0
	cycles = 6;
}

//===== MINC1 #,r
void regMINC1()
{
	_u16 num = fetch16() + 1;

	if (size == 1)
	{
		if ((rCodeW(rCode) % num) == (num - 1))
			rCodeW(rCode) -= (num - 1);
		else
			rCodeW(rCode) += 1;
	}

	cycles = 8;
}

//===== MINC2 #,r
void regMINC2()
{
	_u16 num = fetch16() + 2;

	if (size == 1)
	{
		if ((rCodeW(rCode) % num) == (num - 2))
			rCodeW(rCode) -= (num - 2);
		else
			rCodeW(rCode) += 2;
	}

	cycles = 8;
}

//===== MINC4 #,r
void regMINC4()
{
	_u16 num = fetch16() + 4;

	if (size == 1)
	{
		if ((rCodeW(rCode) % num) == (num - 4))
			rCodeW(rCode) -= (num - 4);
		else
			rCodeW(rCode) += 4;
	}

	cycles = 8;
}

//===== MDEC1 #,r
void regMDEC1()
{
	_u16 num = fetch16() + 1;

	if (size == 1)
	{
		if ((rCodeW(rCode) % num) == 0)
			rCodeW(rCode) += (num - 1);
		else
			rCodeW(rCode) -= 1;
	}

	cycles = 7;
}

//===== MDEC2 #,r
void regMDEC2()
{
	_u16 num = fetch16() + 2;

	if (size == 1)
	{
		if ((rCodeW(rCode) % num) == 0)
			rCodeW(rCode) += (num - 2);
		else
			rCodeW(rCode) -= 2;
	}

	cycles = 7;
}

//===== MDEC4 #,r
void regMDEC4()
{
	_u16 num = fetch16() + 4;

	if (size == 1)
	{
		if ((rCodeW(rCode) % num) == 0)
			rCodeW(rCode) += (num - 4);
		else
			rCodeW(rCode) -= 4;
	}

	cycles = 7;
}

//===== MUL RR,r
void regMUL()
{
	_u8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("reg: MUL bad \'RR\' dst code");
		return;
	}

	switch(size)
	{
	case 0: rCodeW(target) = (rCodeW(target) & 0xFF) * rCodeB(rCode);
		cycles = 18; break;
	case 1: rCodeL(target) = (rCodeL(target) & 0xFFFF) * rCodeW(rCode);	
		cycles = 26; break;
	}
}

//===== MULS RR,r
void regMULS()
{
	_u8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("reg: MUL bad \'RR\' dst code");
		return;
	}

	switch(size)
	{
	case 0: rCodeW(target) = (_s8)(rCodeW(target) & 0xFF) * (_s8)rCodeB(rCode);	
		cycles = 18; break;
	case 1: rCodeL(target) = (_s16)(rCodeL(target) & 0xFFFF) * (_s16)rCodeW(rCode);	
		cycles = 26; break;
	}
}

//===== DIV RR,r
void regDIV()
{
	_u8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("reg: DIV bad \'RR\' dst code");
		return;
	}

	switch(size)
	{
	case 0: {	rCodeW(target) =  generic_DIV_B(rCodeW(target), rCodeB(rCode));
				cycles = 22;
				break;	}

	case 1: {	rCodeL(target) =  generic_DIV_W(rCodeL(target), rCodeW(rCode));
				cycles = 30;
				break;	}
	}
}

//===== DIVS RR,r
void regDIVS()
{
	_u8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("reg: DIVS bad \'RR\' dst code");
		return;
	}

	switch(size)
	{
	case 0: {	rCodeW(target) = generic_DIVS_B(rCodeW(target), rCodeB(rCode));
				cycles = 24;
				break;	}

	case 1: {	rCodeL(target) = generic_DIVS_W(rCodeL(target), rCodeW(rCode));
				cycles = 32;
				break;	}
	}
}

//===== INC #3,r
void regINC()
{
	_u8 val = R;
	if (val == 0)
		val = 8;

	switch(size)
	{
	case 0: {	_u8 dst = rCodeB(rCode);
				_u8 half = (dst & 0xF) + val;
				_u32 resultC = dst + val;
				_u8 result = (_u8)(resultC & 0xFF);
				SETFLAG_S(result & 0x80);

				if (((_s8)dst >= 0) && ((_s8)result < 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				SETFLAG_H(half > 0xF);
				SETFLAG_Z(result == 0);
				SETFLAG_N0;
				rCodeB(rCode) = result;
				break;	}

	case 1: {	rCodeW(rCode) += val; break; }

	case 2: {	rCodeL(rCode) += val; break; }
	}

	cycles = 4;
}

//===== DEC #3,r
void regDEC()
{
	_u8 val = R;
	if (val == 0)
		val = 8;

	switch(size)
	{
	case 0: {	_u8 dst = rCodeB(rCode);
				_u8 half = (dst & 0xF) - val;
				_u32 resultC = dst - val;
				_u8 result = (_u8)(resultC & 0xFF);
				SETFLAG_S(result & 0x80);

				if (((_s8)dst < 0) && ((_s8)result >= 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				SETFLAG_H(half > 0xF);
				SETFLAG_Z(result == 0);
				SETFLAG_N1;
				rCodeB(rCode) = result;
				cycles = 4;
				break;	}

	case 1: {	rCodeW(rCode) -= val; cycles = 4; break; }

	case 2: {	rCodeL(rCode) -= val; cycles = 5; break; }
	}
}

//===== SCC cc,r
void regSCC()
{
	_u32 result;

	if (conditionCode(second & 0xF))
		result = 1;
	else
		result = 0;

	switch(size)
	{
	case 0: rCodeB(rCode) = (_u8)result; break;
	case 1: rCodeW(rCode) = (_u16)result; break;
	}

	cycles = 6;
}

//===== LD R,r
void regLDRr()
{
	switch(size)
	{
	case 0:	regB(R) = rCodeB(rCode);	break;
	case 1:	regW(R) = rCodeW(rCode);	break;
	case 2: regL(R) = rCodeL(rCode);	break;
	}

	cycles = 4;
}

//===== LD r,R
void regLDrR()
{
	switch(size)
	{
	case 0:	rCodeB(rCode) = regB(R);	break;
	case 1:	rCodeW(rCode) = regW(R);	break;
	case 2: rCodeL(rCode) = regL(R);	break;
	}

	cycles = 4;
}

//===== ADD R,r
void regADD()
{
	switch(size)
	{
	case 0: regB(R) = generic_ADD_B(regB(R), rCodeB(rCode)); cycles = 4; break;
	case 1: regW(R) = generic_ADD_W(regW(R), rCodeW(rCode)); cycles = 4; break;
	case 2: regL(R) = generic_ADD_L(regL(R), rCodeL(rCode)); cycles = 7; break;
	}
}

//===== ADC R,r
void regADC()
{
	switch(size)
	{
	case 0: regB(R) = generic_ADC_B(regB(R), rCodeB(rCode)); cycles = 4; break;
	case 1: regW(R) = generic_ADC_W(regW(R), rCodeW(rCode)); cycles = 4; break;
	case 2: regL(R) = generic_ADC_L(regL(R), rCodeL(rCode)); cycles = 7; break;
	}
}

//===== SUB R,r
void regSUB()
{
	switch(size)
	{
	case 0: regB(R) = generic_SUB_B(regB(R), rCodeB(rCode)); cycles = 4; break;
	case 1: regW(R) = generic_SUB_W(regW(R), rCodeW(rCode)); cycles = 4; break;
	case 2: regL(R) = generic_SUB_L(regL(R), rCodeL(rCode)); cycles = 7; break;
	}
}

//===== SBC R,r
void regSBC()
{
	switch(size)
	{
	case 0: regB(R) = generic_SBC_B(regB(R), rCodeB(rCode)); cycles = 4; break;
	case 1: regW(R) = generic_SBC_W(regW(R), rCodeW(rCode)); cycles = 4; break;
	case 2: regL(R) = generic_SBC_L(regL(R), rCodeL(rCode)); cycles = 7; break;
	}
}

//===== LD r,#3
void regLDr3()
{
	switch(size)
	{
	case 0:	rCodeB(rCode) = R;	break;
	case 1:	rCodeW(rCode) = R;	break;
	case 2:	rCodeL(rCode) = R;	break;
	}

	cycles = 4;
}

//===== EX R,r
void regEX()
{
	switch(size)
	{
	case 0:	{ _u8  temp = regB(R); regB(R) = rCodeB(rCode); rCodeB(rCode) = temp; break;}
	case 1:	{ _u16 temp = regW(R); regW(R) = rCodeW(rCode); rCodeW(rCode) = temp; break;}
	case 2:	{ _u32 temp = regL(R); regL(R) = rCodeL(rCode); rCodeL(rCode) = temp; break;}
	}

	cycles = 5;
}

//===== ADD r,#
void regADDi()
{
	switch(size)
	{
	case 0: rCodeB(rCode) = generic_ADD_B(rCodeB(rCode), FETCH8); cycles = 4;break;
	case 1: rCodeW(rCode) = generic_ADD_W(rCodeW(rCode), fetch16()); cycles = 4;break;
	case 2: rCodeL(rCode) = generic_ADD_L(rCodeL(rCode), fetch32()); cycles = 7;break;
	}
}

//===== ADC r,#
void regADCi()
{
	switch(size)
	{
	case 0: rCodeB(rCode) = generic_ADC_B(rCodeB(rCode), FETCH8); cycles = 4;break;
	case 1: rCodeW(rCode) = generic_ADC_W(rCodeW(rCode), fetch16()); cycles = 4;break;
	case 2: rCodeL(rCode) = generic_ADC_L(rCodeL(rCode), fetch32()); cycles = 7;break;
	}
}

//===== SUB r,#
void regSUBi()
{
	switch(size)
	{
	case 0: rCodeB(rCode) = generic_SUB_B(rCodeB(rCode), FETCH8); cycles = 4;break;
	case 1: rCodeW(rCode) = generic_SUB_W(rCodeW(rCode), fetch16()); cycles = 4;break;
	case 2: rCodeL(rCode) = generic_SUB_L(rCodeL(rCode), fetch32()); cycles = 7;break;
	}
}

//===== SBC r,#
void regSBCi()
{
	switch(size)
	{
	case 0: rCodeB(rCode) = generic_SBC_B(rCodeB(rCode), FETCH8); cycles = 4;break;
	case 1: rCodeW(rCode) = generic_SBC_W(rCodeW(rCode), fetch16()); cycles = 4;break;
	case 2: rCodeL(rCode) = generic_SBC_L(rCodeL(rCode), fetch32()); cycles = 7;break;
	}
}

//===== CP r,#
void regCPi()
{
	switch(size)
	{
	case 0:	generic_SUB_B(rCodeB(rCode), FETCH8);	cycles = 4;break;
	case 1:	generic_SUB_W(rCodeW(rCode), fetch16());cycles = 4;	break;
	case 2:	generic_SUB_L(rCodeL(rCode), fetch32());cycles = 7;	break;
	}
}

//===== AND r,#
void regANDi()
{
	switch(size)
	{
	case 0:	{	_u8 result = rCodeB(rCode) & FETCH8;
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cycles = 4;
				break; }
	
	case 1: {	_u16 result = rCodeW(rCode) & fetch16();
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cycles = 4;
				break; }

	case 2:	{	_u32 result = rCodeL(rCode) & fetch32();
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cycles = 7;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR r,#
void regORi()
{
	switch(size)
	{
	case 0: {	_u8 result = rCodeB(rCode) | FETCH8;
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				rCodeB(rCode) = result;
				parityB(result);
				cycles = 4;
				break; }

	case 1: {	_u16 result = rCodeW(rCode) | fetch16();
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				rCodeW(rCode) = result;
				parityW(result);
				cycles = 4;
				break; }

	case 2: {	_u32 result = rCodeL(rCode) | fetch32();
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				rCodeL(rCode) = result;
				cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR r,#
void regXORi()
{
	switch(size)
	{
	case 0: {	_u8 result = rCodeB(rCode) ^ FETCH8;
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				rCodeB(rCode) = result;
				parityB(result);
				cycles = 4;
				break; }

	case 1: {	_u16 result = rCodeW(rCode) ^ fetch16();
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				rCodeW(rCode) = result;
				parityW(result);
				cycles = 4;
				break; }

	case 2: {	_u32 result = rCodeL(rCode) ^ fetch32();
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				rCodeL(rCode) = result;
				cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== AND R,r
void regAND()
{
	switch(size)
	{
	case 0: {	_u8 result = regB(R) & rCodeB(rCode);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				regB(R) = result;
				parityB(result);
				cycles = 4;
				break; }

	case 1: {	_u16 result = regW(R) & rCodeW(rCode);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				regW(R) = result;
				parityW(result);
				cycles = 4;
				break; }

	case 2: {	_u32 result = regL(R) & rCodeL(rCode);
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				regL(R) = result;
				cycles = 7;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR R,r
void regOR()
{
	switch(size)
	{
	case 0: {	_u8 result = regB(R) | rCodeB(rCode);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				regB(R) = result;
				parityB(result);
				cycles = 4;
				break; }

	case 1: {	_u16 result = regW(R) | rCodeW(rCode);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				regW(R) = result;
				parityW(result);
				cycles = 4;
				break; }

	case 2: {	_u32 result = regL(R) | rCodeL(rCode);
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				regL(R) = result;
				cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR R,r
void regXOR()
{
	switch(size)
	{
	case 0: {	_u8 result = regB(R) ^ rCodeB(rCode);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				regB(R) = result;
				parityB(result);
				cycles = 4;
				break; }

	case 1: {	_u16 result = regW(R) ^ rCodeW(rCode);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				regW(R) = result;
				parityW(result);
				cycles = 4;
				break; }

	case 2: {	_u32 result = regL(R) ^ rCodeL(rCode);
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				regL(R) = result;
				cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== CP r,#3
void regCPr3()
{	
	switch(size)
	{
	case 0:	generic_SUB_B(rCodeB(rCode), R);	break;
	case 1:	generic_SUB_W(rCodeW(rCode), R);	break;
	}

	cycles = 4;
}

//===== CP R,r
void regCP()
{
	switch(size)
	{
	case 0:	generic_SUB_B(regB(R), rCodeB(rCode));cycles = 4;	break;
	case 1:	generic_SUB_W(regW(R), rCodeW(rCode));cycles = 4;	break;
	case 2:	generic_SUB_L(regL(R), rCodeL(rCode));cycles = 7;	break;
	}
}

//===== RLC #,r
void regRLCi()
{
	int i;
	_u8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;

	switch(size)
	{
	case 0:		for (i = 0; i < sa; i++) 
				{
					SETFLAG_C(rCodeB(rCode) & 0x80);
					rCodeB(rCode) <<= 1;
					if (FLAG_C) rCodeB(rCode) |= 1;
				}
				SETFLAG_S(rCodeB(rCode) & 0x80);
				SETFLAG_Z(rCodeB(rCode) == 0);
				parityB(rCodeB(rCode));
				cycles = 6 + (2*sa);
				break;
		
	case 1:		for (i = 0; i < sa; i++) 
				{	
					SETFLAG_C(rCodeW(rCode) & 0x8000); 
					rCodeW(rCode) <<= 1;
					if (FLAG_C) rCodeW(rCode) |= 1;
				}
				SETFLAG_S(rCodeW(rCode) & 0x8000);
				SETFLAG_Z(rCodeW(rCode) == 0);
				parityW(rCodeW(rCode));
				cycles = 6 + (2*sa);
				break;

	case 2:		for (i = 0; i < sa; i++) 
				{
					SETFLAG_C(rCodeL(rCode) & 0x80000000);
					rCodeL(rCode) <<= 1;
					if (FLAG_C) rCodeL(rCode) |= 1;
				}
				SETFLAG_S(rCodeL(rCode) & 0x80000000);
				SETFLAG_Z(rCodeL(rCode) == 0);
				cycles = 8 + (2*sa);
				break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RRC #,r
void regRRCi()
{
	int i;
	_u8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;

	switch(size)
	{
	case 0:		for (i = 0; i < sa; i++) 
				{
					SETFLAG_C(rCodeB(rCode) & 1);
					rCodeB(rCode) >>= 1;
					if (FLAG_C) rCodeB(rCode) |= 0x80;
				}
				SETFLAG_S(rCodeB(rCode) & 0x80);
				SETFLAG_Z(rCodeB(rCode) == 0);
				parityB(rCodeB(rCode));
				cycles = 6 + (2*sa);
				break;
		
	case 1:		for (i = 0; i < sa; i++) 
				{
					SETFLAG_C(rCodeW(rCode) & 1);
					rCodeW(rCode) >>= 1;
					if (FLAG_C) rCodeW(rCode) |= 0x8000;
				}
				SETFLAG_S(rCodeW(rCode) & 0x8000);
				SETFLAG_Z(rCodeW(rCode) == 0);
				parityW(rCodeW(rCode));
				cycles = 6 + (2*sa);
				break;

	case 2:		for (i = 0; i < sa; i++) 
				{
					SETFLAG_C(rCodeL(rCode) & 1);
					rCodeL(rCode) >>= 1;
					if (FLAG_C) rCodeL(rCode) |= 0x80000000;
				}
				SETFLAG_S(rCodeL(rCode) & 0x80000000);
				SETFLAG_Z(rCodeL(rCode) == 0);
				cycles = 8 + (2*sa);
				break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RL #,r
void regRLi()
{
	int i;
	BOOL tempC;
	_u8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;

	switch(size)
	{
	case 0:	{	_u8 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeB(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x80);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeB(rCode) = result;
				}
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + (2*sa);
				break; }
		
	case 1:	{	_u16 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeW(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x8000);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeW(rCode) = result;
				}
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + (2*sa);
				break; }

	case 2:	{	_u32 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeL(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x80000000);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeL(rCode) = result;
				}
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				cycles = 8 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RR #,r
void regRRi()
{
	int i;
	BOOL tempC;
	_u8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;

	switch(size)
	{
	case 0:	{	_u8 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeB(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x01);
					result >>= 1;
					if (tempC) result |= 0x80;
					rCodeB(rCode) = result;
				}
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				cycles = 6 + (2*sa);
				parityB(result);
				break; }
		
	case 1:	{	_u16 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeW(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x01);
					result >>= 1;
					if (tempC) result |= 0x8000;
					rCodeW(rCode) = result;
				}
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				cycles = 6 + (2*sa);
				parityW(result);
				break; }

	case 2:	{	_u32 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeL(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x01);
					result >>= 1;
					if (tempC) result |= 0x80000000;
					rCodeL(rCode) = result;
				}
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				cycles = 8 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SLA #,r
void regSLAi()
{
	_s8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size)
	{
	case 0:	{	_s8 result, data = (_s8)rCodeB(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break;	}
	
	case 1:	{	_s16 result, data = (_s16)rCodeW(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	_s32 result, data = (_s32)rCodeL(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRA #,r
void regSRAi()
{
	_s8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size)
	{
	case 0:	{	_s8 data = (_s8)rCodeB(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	_s16 data = (_s16)rCodeW(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	_s32 data = (_s32)rCodeL(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SLL #,r
void regSLLi()
{
	_u8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size)
	{
	case 0:	{	_u8 result, data = rCodeB(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break;	}
	
	case 1:	{	_u16 result, data = rCodeW(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	_u32 result, data = rCodeL(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRL #,r
void regSRLi()
{
	_u8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size)
	{
	case 0:	{	_u8 data = rCodeB(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	_u16 data = rCodeW(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	_u32 data = rCodeL(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RLC A,r
void regRLCA()
{
	int i;
	_u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;

	switch(size)
	{
	case 0:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeB(rCode) & 0x80); 
				rCodeB(rCode) <<= 1;
				if (FLAG_C) rCodeB(rCode) |= 1;	
			}
			SETFLAG_S(rCodeB(rCode) & 0x80);
			SETFLAG_Z(rCodeB(rCode) == 0);
			cycles = 6 + (2*sa);
			parityB(rCodeB(rCode));
			break;
		
	case 1:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeW(rCode) & 0x8000);
				rCodeW(rCode) <<= 1;
				if (FLAG_C) rCodeW(rCode) |= 1;
			}
			SETFLAG_S(rCodeW(rCode) & 0x8000);
			SETFLAG_Z(rCodeW(rCode) == 0);
			cycles = 6 + (2*sa);
			parityW(rCodeW(rCode));
			break;

	case 2:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeL(rCode) & 0x80000000);
				rCodeL(rCode) <<= 1;
				if (FLAG_C) rCodeL(rCode) |= 1;
			}
			SETFLAG_S(rCodeL(rCode) & 0x80000000);
			SETFLAG_Z(rCodeL(rCode) == 0);
			cycles = 8 + (2*sa);
			break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RRC A,r
void regRRCA()
{
	int i;
	_u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;

	switch(size)
	{
	case 0:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeB(rCode) & 1);
				rCodeB(rCode) >>= 1;
				if (FLAG_C) rCodeB(rCode) |= 0x80;	
			}
			SETFLAG_S(rCodeB(rCode) & 0x80);
			SETFLAG_Z(rCodeB(rCode) == 0);
			parityB(rCodeB(rCode));
			cycles = 6 + (2*sa);
			break;
		
	case 1:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeW(rCode) & 1); 
				rCodeW(rCode) >>= 1;
				if (FLAG_C) rCodeW(rCode) |= 0x8000;
			}
			SETFLAG_S(rCodeW(rCode) & 0x8000);
			SETFLAG_Z(rCodeW(rCode) == 0);
			parityW(rCodeW(rCode));
			cycles = 6 + (2*sa);
			break;

	case 2:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeL(rCode) & 1);
				rCodeL(rCode) >>= 1;
				if (FLAG_C) rCodeL(rCode) |= 0x80000000;
			}
			SETFLAG_S(rCodeL(rCode) & 0x80000000);
			SETFLAG_Z(rCodeL(rCode) == 0);
			cycles = 8 + (2*sa);
			break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RL A,r
void regRLA()
{
	int i;
	BOOL tempC;
	_u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;

	switch(size)
	{
	case 0:	{	_u8 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeB(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x80);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeB(rCode) = result;
				}
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				cycles = 6 + (2*sa);
				parityB(result);
				break; }
		
	case 1:	{	_u16 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeW(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x8000);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeW(rCode) = result;
				}
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				cycles = 6 + (2*sa);
				parityW(result);
				break; }

	case 2:	{	_u32 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeL(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x80000000);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeL(rCode) = result;
				}
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				cycles = 8 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RR A,r
void regRRA()
{
	int i;
	BOOL tempC;
	_u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;

	switch(size)
	{
	case 0:	{	_u8 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeB(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x01);
					result >>= 1;
					if (tempC) result |= 0x80;
					rCodeB(rCode) = result;
				}
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				cycles = 6 + (2*sa);
				parityB(result);
				break; }
		
	case 1:	{	_u16 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeW(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x01);
					result >>= 1;
					if (tempC) result |= 0x8000;
					rCodeW(rCode) = result;
				}
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				cycles = 6 + (2*sa);
				parityW(result);
				break; }

	case 2:	{	_u32 result;
				for (i = 0; i < sa; i++) 
				{
					result = rCodeL(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x01);
					result >>= 1;
					if (tempC) result |= 0x80000000;
					rCodeL(rCode) = result;
				}
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				cycles = 8 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SLA A,r
void regSLAA()
{
	_s8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size)
	{
	case 0:	{	_s8 result, data = (_s8)rCodeB(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break;	}
	
	case 1:	{	_s16 result, data = (_s16)rCodeW(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	_s32 result, data = (_s32)rCodeL(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRA A,r
void regSRAA()
{
	_s8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size)
	{
	case 0:	{	_s8 data = (_s8)rCodeB(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	_s16 data = (_s16)rCodeW(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	_s32 data = (_s32)rCodeL(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SLL A,r
void regSLLA()
{
	_u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size)
	{
	case 0:	{	_u8 result, data = rCodeB(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break;	}
	
	case 1:	{	_u16 result, data = rCodeW(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	_u32 result, data = rCodeL(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRL A,r
void regSRLA()
{
	_u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size)
	{
	case 0:	{	_u8 data = rCodeB(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	_u16 data = rCodeW(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	_u32 data = rCodeL(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//=============================================================================
