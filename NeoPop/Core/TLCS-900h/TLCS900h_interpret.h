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

	TLCS900h_interpret.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- Added the 'instruction_error' function declaration here.

28 JUL 2002 - neopop_uk
=======================================
- Removed CYCLE_WARNING as it is now obsolete.
- Added generic DIV prototypes.

//---------------------------------------------------------------------------
*/

#ifndef __TLCS900H_INTERPRET__
#define __TLCS900H_INTERPRET__
//=============================================================================

//Interprets a single instruction from 'pc', 
//pc is incremented to the start of the next instruction.
//Returns the number of cycles taken for this instruction
_u8 TLCS900h_interpret(void);

//=============================================================================

extern _u32 mem;	
extern int size;
extern _u8 first;			//First byte
extern _u8 second;			//Second byte
extern _u8 R;				//(second & 7)
extern _u8 rCode;
extern _u8 cycles;
extern BOOL brCode;

//=============================================================================

void __cdecl instruction_error(char* vaMessage,...);

//=============================================================================

#define FETCH8		loadB(pc++)

_u16 fetch16(void);
_u32 fetch24(void);
_u32 fetch32(void);

//=============================================================================

void parityB(_u8 value);
void parityW(_u16 value);

//=============================================================================

void push8(_u8 data);
void push16(_u16 data);
void push32(_u32 data);

_u8 pop8(void);
_u16 pop16(void);
_u32 pop32(void);

//=============================================================================

//DIV ===============
_u16 generic_DIV_B(_u16 val, _u8 div);
_u32 generic_DIV_W(_u32 val, _u16 div);

//DIVS ===============
_u16 generic_DIVS_B(_s16 val, _s8 div);
_u32 generic_DIVS_W(_s32 val, _s16 div);

//ADD ===============
_u8	generic_ADD_B(_u8 dst, _u8 src);
_u16 generic_ADD_W(_u16 dst, _u16 src);
_u32 generic_ADD_L(_u32 dst, _u32 src);

//ADC ===============
_u8	generic_ADC_B(_u8 dst, _u8 src);
_u16 generic_ADC_W(_u16 dst, _u16 src);
_u32 generic_ADC_L(_u32 dst, _u32 src);

//SUB ===============
_u8	generic_SUB_B(_u8 dst, _u8 src);
_u16 generic_SUB_W(_u16 dst, _u16 src);
_u32 generic_SUB_L(_u32 dst, _u32 src);

//SBC ===============
_u8	generic_SBC_B(_u8 dst, _u8 src);
_u16 generic_SBC_W(_u16 dst, _u16 src);
_u32 generic_SBC_L(_u32 dst, _u32 src);

//=============================================================================

//Confirms a condition code check
BOOL conditionCode(int cc);

//=============================================================================

//Translate an rr or RR value for MUL/MULS/DIV/DIVS
_u8 get_rr_Target(void);
_u8 get_RR_Target(void);

//=============================================================================
#endif
