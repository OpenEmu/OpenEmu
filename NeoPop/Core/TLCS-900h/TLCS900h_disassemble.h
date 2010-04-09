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

	TLCS900h_disassemble.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

#ifndef __TLCS900H_DISASSEMBLE__
#define __TLCS900H_DISASSEMBLE__
//=============================================================================

//Disassembles a single instruction from 'pc', 
//pc is incremented to the start of the next instruction.
char* TLCS900h_disassemble(void);

//Print to this string the disassembled instruction
extern char instr[128];

//Print the mnemonic for the addressing mode / reg code.
extern char extra[32];

//=============================================================================

extern char str_R[8];		//Big R
extern char str_r[8];		//Little R

//Translate an rr or RR value for MUL/MULS/DIV/DIVS
void get_rr_Name(void);
void get_RR_Name(void);

extern _u8 bytes[16];		//Stores the bytes used
extern _u8 bcnt;			//Byte Counter for above

extern char* gprName[8][3];		//8 regs * 3 names (byte, word, long)
extern char* regCodeName[3][256];
extern char* crName[3][0x40];

extern char* ccName[];

_u8 get8_dis(void);
_u16 get16_dis(void);
_u32 get24_dis(void);
_u32 get32_dis(void);

//=============================================================================
#endif
