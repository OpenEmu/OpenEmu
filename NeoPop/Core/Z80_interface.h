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

	Z80_interface.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

#ifndef __Z80_CONTROL__
#define __Z80_CONTROL__
//=============================================================================

#include "Z80.h"

void Z80_reset(void);	// z80 reset

void Z80_irq(void);		// Cause an interrupt
void Z80_nmi(void);		// Cause an NMI

#define Z80ACTIVE		(ram[0xb9] == 0x55)

//Emulate a z80 instruction
#define Z80EMULATE		{ ExecZ80(&Z80_regs); }

//Register status
#define Z80_REG_AF	0
#define Z80_REG_BC	1
#define Z80_REG_DE	2
#define Z80_REG_HL	3
#define Z80_REG_IX	4
#define Z80_REG_IY	5
#define Z80_REG_PC	6
#define Z80_REG_SP	7
#define Z80_REG_AF1	8
#define Z80_REG_BC1	9
#define Z80_REG_DE1	10
#define Z80_REG_HL1	11

_u16 Z80_getReg(_u8 reg);
void Z80_setReg(_u8 reg, _u16 value);

//Disassembles a single instruction from 'pc', 
char* Z80_disassemble(_u16* pc);

extern Z80 Z80_regs;

//=============================================================================
#endif
