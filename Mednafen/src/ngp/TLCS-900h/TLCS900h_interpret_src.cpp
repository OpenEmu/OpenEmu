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

	TLCS900h_interpret_src.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

24 JUL 2002 - neopop_uk
=======================================
- Fixed S flag in "RRC (mem)"

25 JUL 2002 - neopop_uk
=======================================
- Removed unneeded Long mode from EX.

28 JUL 2002 - neopop_uk
=======================================
- Improved the LDIR/LDDR/CPIR/CPDR instructions so that they
	finish with the correct register settings, even if there is
	a memory error.
- Converted DIV/DIVS to use the generic function

16 AUG 2002 - neopop_uk
=======================================
- Replaced 'second & 7' with 'R', clearer, faster - and for some reason
	more accurate... oh well!
- Fixed V flag emulation of INC/DEC, fixes "Cotton" menus

21 AUG 2002 - neopop_uk
=======================================
- Fixed "RR (mem)" - It was actually the [REG] version that hadn't been
	changed to use memory accesses!

30 AUG 2002 - neopop_uk
=======================================
- Fixed "DIV RR,(mem)" in long mode, wrong operand size.

04 SEP 2002 - neopop_uk
=======================================
- Fixed GCC compatibility.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "TLCS900h_interpret.h"
#include "TLCS900h_registers.h"
#include "mem.h"

namespace TLCS900H
{

//=========================================================================

//===== PUSH (mem)
void srcPUSH()
{
	switch(size)
	{
	case 0:	push8(loadB(mem));	break;
	case 1: push16(loadW(mem)); break;
	}
	cycles = 7;
}

//===== RLD A,(mem)
void srcRLD()
{
	uint8 al = REGA & 0xF, m, mh, ml;

	m = loadB(mem);
	mh = (m & 0xF0) >> 4;
	ml = (m & 0x0F) << 4;
	
	REGA = (REGA & 0xF0) | mh;
	storeB(mem, ml | al);

	SETFLAG_S(REGA & 0x80);
	SETFLAG_Z(REGA == 0);
	SETFLAG_H0
	SETFLAG_N0
	parityB(REGA);

	cycles = 12;
}

//===== RRD A,(mem)
void srcRRD()
{
	uint8 al = (REGA & 0xF) << 4, m, mh, ml;

	m = loadB(mem);
	mh = (m & 0xF0) >> 4;
	ml = m & 0x0F;
	
	REGA = (REGA & 0xF0) | ml;
	storeB(mem, al | mh);

	SETFLAG_S(REGA & 0x80);
	SETFLAG_Z(REGA == 0);
	SETFLAG_H0
	SETFLAG_N0
	parityB(REGA);

	cycles = 12;
}

//===== LDI
void srcLDI()
{
	uint8 dst = 2/*XDE*/, src = 3/*XHL*/;
	if ((first & 0xF) == 5) { dst = 4/*XIX*/; src = 5/*XIY*/; }

	switch(size)
	{
	case 0:
		storeB(regL(dst), loadB(regL(src)));
		regL(dst) += 1;
		regL(src) += 1;
		break;

	case 1:
		storeW(regL(dst), loadW(regL(src)));
		regL(dst) += 2;
		regL(src) += 2;
		break;
	}

	REGBC --;
	SETFLAG_V(REGBC);

	SETFLAG_H0;
	SETFLAG_N0;
	cycles = 10;
}

//===== LDIR
void srcLDIR()
{
	uint8 dst = 2/*XDE*/, src = 3/*XHL*/;
	if ((first & 0xF) == 5) { dst = 4/*XIX*/; src = 5/*XIY*/; }

	cycles = 10;

	do
	{
		switch(size)
		{
		case 0:	if (debug_abort_memory == FALSE)
					storeB(regL(dst), loadB(regL(src)));
			regL(dst) += 1;
			regL(src) += 1;
			break;

		case 1:	if (debug_abort_memory == FALSE)
					storeW(regL(dst), loadW(regL(src)));
			regL(dst) += 2;
			regL(src) += 2;
			break;
		}

		REGBC --;
		SETFLAG_V(REGBC);

		cycles += 14;
	}
	while (FLAG_V);

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== LDD
void srcLDD()
{
	uint8 dst = 2/*XDE*/, src = 3/*XHL*/;
	if ((first & 0xF) == 5) { dst = 4/*XIX*/; src = 5/*XIY*/; }

	switch(size)
	{
	case 0:
		storeB(regL(dst), loadB(regL(src)));
		regL(dst) -= 1;
		regL(src) -= 1;
		break;

	case 1:
		storeW(regL(dst), loadW(regL(src)));
		regL(dst) -= 2;
		regL(src) -= 2;
		break;
	}

	REGBC --;
	SETFLAG_V(REGBC);

	SETFLAG_H0;
	SETFLAG_N0;
	cycles = 10;
}

//===== LDDR
void srcLDDR()
{
	uint8 dst = 2/*XDE*/, src = 3/*XHL*/;
	if ((first & 0xF) == 5)	{ dst = 4/*XIX*/; src = 5/*XIY*/; }

	cycles = 10;

	do
	{
		switch(size)
		{
		case 0:
			if (debug_abort_memory == FALSE)
				storeB(regL(dst), loadB(regL(src)));
			regL(dst) -= 1;
			regL(src) -= 1;
			break;

		case 1:
			if (debug_abort_memory == FALSE)
				storeW(regL(dst), loadW(regL(src)));
			regL(dst) -= 2;
			regL(src) -= 2;
			break;
		}

		REGBC --;
		SETFLAG_V(REGBC);

		cycles += 14;
	}
	while (FLAG_V);

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== CPI
void srcCPI()
{
	uint8 R = first & 7;

	switch(size)
	{
	case 0: generic_SUB_B(REGA, loadB(regL(R)));
			regL(R) ++; break;

	case 1:	generic_SUB_W(REGWA, loadW(regL(R)));
			regL(R) += 2; break;
	}

	REGBC --;
	SETFLAG_V(REGBC);

	cycles = 8;
}

//===== CPIR
void srcCPIR()
{
	uint8 R = first & 7;

	cycles = 10;

	do
	{
		switch(size)
		{
		case 0:	if (debug_abort_memory == FALSE)
					generic_SUB_B(REGA, loadB(regL(R)));
				regL(R) ++; break;

		case 1:	if (debug_abort_memory == FALSE)
					generic_SUB_W(REGWA, loadW(regL(R)));
				regL(R) += 2; break;
		}

		REGBC --;
		SETFLAG_V(REGBC);

		cycles += 14;
	}
	while (FLAG_V && (FLAG_Z == FALSE));
}

//===== CPD
void srcCPD()
{
	uint8 R = first & 7;

	switch(size)
	{
	case 0:	generic_SUB_B(REGA, loadB(regL(R)));
			regL(R) --;	break;

	case 1:	generic_SUB_W(REGWA, loadW(regL(R)));
			regL(R) -= 2; break;
	}

	REGBC --;
	SETFLAG_V(REGBC);

	cycles = 8;
}

//===== CPDR
void srcCPDR()
{
	uint8 R = first & 7;

	cycles = 10;

	do
	{
		switch(size)
		{
		case 0:	if (debug_abort_memory == FALSE)
					generic_SUB_B(REGA, loadB(regL(R)));
				regL(R) -= 1; break;

		case 1: if (debug_abort_memory == FALSE)
					generic_SUB_W(REGWA, loadW(regL(R)));
				regL(R) -= 2; break;
		}

		REGBC --;
		SETFLAG_V(REGBC);

		cycles += 14;
	}
	while (FLAG_V && (FLAG_Z == FALSE));
}

//===== LD (nn),(mem)
void srcLD16m()
{
	switch(size)
	{
	case 0:	storeB(fetch16(), loadB(mem)); break;
	case 1: storeW(fetch16(), loadW(mem)); break;
	}

	cycles = 8;
}

//===== LD R,(mem)
void srcLD()
{
	switch(size)
	{
	case 0: regB(R) = loadB(mem); cycles = 4; break;
	case 1: regW(R) = loadW(mem); cycles = 4; break;
	case 2: regL(R) = loadL(mem); cycles = 6; break;
	}
}

//===== EX (mem),R
void srcEX()
{
	switch(size)
	{
	case 0:	{	uint8 temp = regB(R); 
				regB(R) = loadB(mem); 
				storeB(mem, temp); break;		}

	case 1:	{	uint16 temp = regW(R); 
				regW(R) = loadW(mem); 
				storeW(mem, temp); break;		}
	}

	cycles = 6;
}

//===== ADD (mem),#
void srcADDi()
{
	switch(size)
	{
	case 0:	storeB(mem, generic_ADD_B(loadB(mem), FETCH8)); cycles = 7;break;
	case 1:	storeW(mem, generic_ADD_W(loadW(mem), fetch16())); cycles = 8;break;
	}
}

//===== ADC (mem),#
void srcADCi()
{
	switch(size)
	{
	case 0:	storeB(mem, generic_ADC_B(loadB(mem), FETCH8)); cycles = 7;break;
	case 1:	storeW(mem, generic_ADC_W(loadW(mem), fetch16())); cycles = 8;break;
	}
}

//===== SUB (mem),#
void srcSUBi()
{
	switch(size)
	{
	case 0:	storeB(mem, generic_SUB_B(loadB(mem), FETCH8)); cycles = 7;break;
	case 1:	storeW(mem, generic_SUB_W(loadW(mem), fetch16())); cycles = 8;break;
	}
}

//===== SBC (mem),#
void srcSBCi()
{
	switch(size)
	{
	case 0:	storeB(mem, generic_SBC_B(loadB(mem), FETCH8)); cycles = 7;break;
	case 1:	storeW(mem, generic_SBC_W(loadW(mem), fetch16())); cycles = 8;break;
	}
}

//===== AND (mem),#
void srcANDi()
{
	switch(size)
	{
	case 0: {	uint8 result = loadB(mem) & FETCH8;
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 7;
				break; }

	case 1: {	uint16 result = loadW(mem) & fetch16();
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 8;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR (mem),#
void srcORi()
{
	switch(size)
	{
	case 0: {	uint8 result = loadB(mem) | FETCH8;
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 7;
				break; }

	case 1: {	uint16 result = loadW(mem) | fetch16();
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 8;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR (mem),#
void srcXORi()
{
	switch(size)
	{
	case 0: {	uint8 result = loadB(mem) ^ FETCH8;
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 7;
				break; }

	case 1: {	uint16 result = loadW(mem) ^ fetch16();
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 8;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== CP (mem),#
void srcCPi()
{
	switch(size)
	{
	case 0:	generic_SUB_B(loadB(mem), FETCH8);	break;
	case 1:	generic_SUB_W(loadW(mem), fetch16());	break;
	}
	
	cycles = 6;
}

//===== MUL RR,(mem)
void srcMUL()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("src: MUL bad \'RR\' dst code");
		return;
	}

	switch(size)
	{
	case 0: rCodeW(target) = (rCodeW(target) & 0xFF) * loadB(mem);
		cycles = 18; break;
	case 1: rCodeL(target) = (rCodeL(target) & 0xFFFF) * loadW(mem);
		cycles = 26; break;
	}
}

//===== MULS RR,(mem)
void srcMULS()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("src: MUL bad \'RR\' dst code");
		return;
	}

	switch(size)
	{
	case 0: rCodeW(target) = (int8)(rCodeW(target) & 0xFF) * (int8)loadB(mem);
		cycles = 18; break;
	case 1: rCodeL(target) = (int16)(rCodeL(target) & 0xFFFF) * (int16)loadW(mem);
		cycles = 26; break;
	}
}

//===== DIV RR,(mem)
void srcDIV()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("src: DIV bad \'RR\' dst code");
		return;
	}

	switch(size)
	{
	case 0: {	rCodeW(target) = generic_DIV_B(rCodeW(target), loadB(mem));
				cycles = 22;
				break;	}
				
	case 1: {	rCodeL(target) = generic_DIV_W(rCodeL(target), loadW(mem));
				cycles = 30;
				break;	}
	}
}

//===== DIVS RR,(mem)
void srcDIVS()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("src: DIVS bad \'RR\' dst code");
		return;
	}

	switch(size)
	{
	case 0: {	rCodeW(target) = generic_DIVS_B(rCodeW(target), loadB(mem));
				cycles = 24;
				break;	}

	case 1: {	rCodeL(target) = generic_DIVS_W(rCodeL(target), loadW(mem));
				cycles = 32;
				break;	}
	}
}

//===== INC #3,(mem)
void srcINC()
{
	uint8 val = R;
	if (val == 0)
		val = 8;

	switch(size)
	{
	case 0: {	uint8 dst = loadB(mem);
				uint32 resultC = dst + val;
				uint8 half = (dst & 0xF) + val;
				uint8 result = (uint8)(resultC & 0xFF);
				SETFLAG_Z(result == 0);
				SETFLAG_H(half > 0xF);
				SETFLAG_S(result & 0x80);
				SETFLAG_N0;

				if (((int8)dst >= 0) && ((int8)result < 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				storeB(mem, result);
				break; }

	case 1: {	uint16 dst = loadW(mem);
				uint32 resultC = dst + val;
				uint8 half = (dst & 0xF) + val;
				uint16 result = (uint16)(resultC & 0xFFFF);
				SETFLAG_Z(result == 0);
				SETFLAG_H(half > 0xF);
				SETFLAG_S(result & 0x8000);
				SETFLAG_N0;

				if (((int16)dst >= 0) && ((int16)result < 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				storeW(mem, result);
				break; }
	}

	cycles = 6;
}

//===== DEC #3,(mem)
void srcDEC()
{
	uint8 val = R;
	if (val == 0)
		val = 8;

	switch(size)
	{
	case 0: {	uint8 dst = loadB(mem);
				uint32 resultC = dst - val;
				uint8 half = (dst & 0xF) - val;
				uint8 result = (uint8)(resultC & 0xFF);
				SETFLAG_Z(result == 0);
				SETFLAG_H(half > 0xF);
				SETFLAG_S(result & 0x80);
				SETFLAG_N1;

				if (((int8)dst < 0) && ((int8)result >= 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				storeB(mem, result);
				break; }

	case 1: {	uint16 dst = loadW(mem);
				uint32 resultC = dst - val;
				uint8 half = (dst & 0xF) - val;
				uint16 result = (uint16)(resultC & 0xFFFF);
				SETFLAG_Z(result == 0);
				SETFLAG_H(half > 0xF);
				SETFLAG_S(result & 0x8000);
				SETFLAG_N1;

				if (((int16)dst < 0) && ((int16)result >= 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				storeW(mem, result);
				break; }
	}

	cycles = 6;
}

//===== RLC (mem)
void srcRLC()
{
	switch(size)
	{
	case 0:	{	uint8 result = loadB(mem);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				if (FLAG_C) result |= 1;
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				break; }
		
	case 1:	{	uint16 result = loadW(mem);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				if (FLAG_C) result |= 1;
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cycles = 8;
}

//===== RRC (mem)
void srcRRC()
{
	switch(size)
	{
	case 0:	{	uint8 data = loadB(mem), result;
				SETFLAG_C(data & 1);
				result = data >> 1;
				if (FLAG_C) result |= 0x80;
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				break; }
		
	case 1:	{	uint16 data = loadW(mem), result;
				SETFLAG_C(data & 1);
				result = data >> 1;
				if (FLAG_C) result |= 0x8000;
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cycles = 8;
}

//===== RL (mem)
void srcRL()
{
	bool tempC;

	switch(size)
	{
	case 0:	{	uint8 result = loadB(mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 0x80);
				result <<= 1;
				if (tempC) result |= 1;
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				break; }
		
	case 1:	{	uint16 result = loadW(mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				if (tempC) result |= 1;
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				break; }
	}

	cycles = 8;
}

//===== RR (mem)
void srcRR()
{
	bool tempC;

	switch(size)
	{
	case 0:	{	uint8 result = loadB(mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 1);
				result >>= 1;
				if (tempC) result |= 0x80;
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				break; }
		
	case 1:	{	uint16 result = loadW(mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 1);
				result >>= 1;
				if (tempC) result |= 0x8000;
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				break; }
	}

	cycles = 8;
}

//===== SLA (mem)
void srcSLA()
{
	switch(size)
	{
	case 0:	{	uint8 result, data = loadB(mem);
				SETFLAG_C(data & 0x80);
				result = ((int8)data << 1);
				SETFLAG_S(result & 0x80);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}

	case 1:	{	uint16 result, data = loadW(mem);
				SETFLAG_C(data & 0x8000);
				result = ((int16)data << 1);
				SETFLAG_S(result & 0x8000);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cycles = 8;
}

//===== SRA (mem)
void srcSRA()
{
	switch(size)
	{
	case 0:	{	uint8 result, data = loadB(mem);
				SETFLAG_C(data & 0x1);
				result = ((int8)data >> 1);
				SETFLAG_S(result & 0x80);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}
	
	case 1:	{	uint16 result, data = loadW(mem);
				SETFLAG_C(data & 0x1);
				result = ((int16)data >> 1);
				SETFLAG_S(result & 0x8000);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cycles = 8;
}

//===== SLL (mem)
void srcSLL()
{
	switch(size)
	{
	case 0:	{	uint8 result, data = loadB(mem);
				SETFLAG_C(data & 0x80);
				result = (data << 1);
				SETFLAG_S(result & 0x80);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}
	
	case 1:	{	uint16 result, data = loadW(mem);
				SETFLAG_C(data & 0x8000);
				result = (data << 1);
				SETFLAG_S(result & 0x8000);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cycles = 8;
}

//===== SRL (mem)
void srcSRL()
{
	switch(size)
	{
	case 0:	{	uint8 result, data = loadB(mem);
				SETFLAG_C(data & 0x01);
				result = (data >> 1);
				SETFLAG_S(result & 0x80);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}

	case 1:	{	uint16 result, data = loadW(mem);
				SETFLAG_C(data & 0x0001);
				result = (data >> 1);
				SETFLAG_S(result & 0x8000);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cycles = 8;
}

//===== ADD R,(mem)
void srcADDRm()
{
	switch(size)
	{
	case 0: regB(R) = generic_ADD_B(regB(R), loadB(mem)); cycles = 4;break;
	case 1: regW(R) = generic_ADD_W(regW(R), loadW(mem)); cycles = 4;break;
	case 2: regL(R) = generic_ADD_L(regL(R), loadL(mem)); cycles = 6;break;
	}
}

//===== ADD (mem),R
void srcADDmR()
{
	switch(size)
	{
	case 0:	storeB(mem, generic_ADD_B(loadB(mem), regB(R))); cycles = 6;break;
	case 1:	storeW(mem, generic_ADD_W(loadW(mem), regW(R))); cycles = 6;break;
	case 2:	storeL(mem, generic_ADD_L(loadL(mem), regL(R))); cycles = 10;break;
	}
}

//===== ADC R,(mem)
void srcADCRm()
{
	switch(size)
	{
	case 0: regB(R) = generic_ADC_B(regB(R), loadB(mem)); cycles = 4;break;
	case 1: regW(R) = generic_ADC_W(regW(R), loadW(mem)); cycles = 4;break;
	case 2: regL(R) = generic_ADC_L(regL(R), loadL(mem)); cycles = 6;break;
	}
}

//===== ADC (mem),R
void srcADCmR()
{
	switch(size)
	{
	case 0:	storeB(mem, generic_ADC_B(loadB(mem), regB(R))); cycles = 6;break;
	case 1:	storeW(mem, generic_ADC_W(loadW(mem), regW(R))); cycles = 6;break;
	case 2:	storeL(mem, generic_ADC_L(loadL(mem), regL(R))); cycles = 10;break;
	}
}

//===== SUB R,(mem)
void srcSUBRm()
{
	switch(size)
	{
	case 0: regB(R) = generic_SUB_B(regB(R), loadB(mem)); cycles = 4;break;
	case 1: regW(R) = generic_SUB_W(regW(R), loadW(mem)); cycles = 4;break;
	case 2: regL(R) = generic_SUB_L(regL(R), loadL(mem)); cycles = 6;break;
	}
}

//===== SUB (mem),R
void srcSUBmR()
{
	switch(size)
	{
	case 0:	storeB(mem, generic_SUB_B(loadB(mem), regB(R))); cycles = 6;break;
	case 1:	storeW(mem, generic_SUB_W(loadW(mem), regW(R))); cycles = 6;break;
	case 2:	storeL(mem, generic_SUB_L(loadL(mem), regL(R))); cycles = 10;break;
	}
}

//===== SBC R,(mem)
void srcSBCRm()
{
	switch(size)
	{
	case 0: regB(R) = generic_SBC_B(regB(R), loadB(mem)); cycles = 4;break;
	case 1: regW(R) = generic_SBC_W(regW(R), loadW(mem)); cycles = 4;break;
	case 2: regL(R) = generic_SBC_L(regL(R), loadL(mem)); cycles = 6;break;
	}
}

//===== SBC (mem),R
void srcSBCmR()
{
	switch(size)
	{
	case 0:	storeB(mem, generic_SBC_B(loadB(mem), regB(R))); cycles = 6;break;
	case 1:	storeW(mem, generic_SBC_W(loadW(mem), regW(R))); cycles = 6;break;
	case 2:	storeL(mem, generic_SBC_L(loadL(mem), regL(R))); cycles = 10;break;
	}
}

//===== AND R,(mem)
void srcANDRm()
{
	switch(size)
	{
	case 0:	{	uint8 result = regB(R) & loadB(mem);
				regB(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cycles = 4;
				break; }
	
	case 1: {	uint16 result = regW(R) & loadW(mem);
				regW(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cycles = 4;
				break; }

	case 2:	{	uint32 result = regL(R) & loadL(mem);
				regL(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cycles = 6;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== AND (mem),R
void srcANDmR()
{
	switch(size)
	{
	case 0:	{	uint8 result = regB(R) & loadB(mem);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cycles = 6;
				break; }
	
	case 1: {	uint16 result = regW(R) & loadW(mem);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cycles = 6;
				break; }

	case 2:	{	uint32 result = regL(R) & loadL(mem);
				storeL(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cycles = 10;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR R,(mem)
void srcXORRm()
{
	switch(size)
	{
	case 0:	{	uint8 result = regB(R) ^ loadB(mem);
				regB(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cycles = 4;
				break; }
	
	case 1: {	uint16 result = regW(R) ^ loadW(mem);
				regW(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cycles = 4;
				break; }

	case 2:	{	uint32 result = regL(R) ^ loadL(mem);
				regL(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cycles = 6;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR (mem),R
void srcXORmR()
{
	switch(size)
	{
	case 0:	{	uint8 result = regB(R) ^ loadB(mem);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cycles = 6;
				break; }
	
	case 1: {	uint16 result = regW(R) ^ loadW(mem);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cycles = 6;
				break; }

	case 2:	{	uint32 result = regL(R) ^ loadL(mem);
				storeL(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cycles = 10;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR R,(mem)
void srcORRm()
{
	switch(size)
	{
	case 0:	{	uint8 result = regB(R) | loadB(mem);
				regB(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cycles = 4;
				break; }
	
	case 1: {	uint16 result = regW(R) | loadW(mem);
				regW(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cycles = 4;
				break; }

	case 2:	{	uint32 result = regL(R) | loadL(mem);
				regL(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cycles = 6;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR (mem),R
void srcORmR()
{
	switch(size)
	{
	case 0:	{	uint8 result = regB(R) | loadB(mem);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cycles = 6;
				break; }
	
	case 1: {	uint16 result = regW(R) | loadW(mem);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cycles = 6;
				break; }

	case 2:	{	uint32 result = regL(R) | loadL(mem);
				storeL(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cycles = 10;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== CP R,(mem)
void srcCPRm()
{
	switch(size)
	{
	case 0: generic_SUB_B(regB(R), loadB(mem)); cycles = 4; break;
	case 1: generic_SUB_W(regW(R), loadW(mem)); cycles = 4; break;
	case 2: generic_SUB_L(regL(R), loadL(mem)); cycles = 6; break;
	}
}

//===== CP (mem),R
void srcCPmR()
{
	switch(size)
	{
	case 0: generic_SUB_B(loadB(mem), regB(R));		break;
	case 1: generic_SUB_W(loadW(mem), regW(R));		break;
	case 2: generic_SUB_L(loadL(mem), regL(R));		break;
	}
	
	cycles = 6;
}
};
//=============================================================================
