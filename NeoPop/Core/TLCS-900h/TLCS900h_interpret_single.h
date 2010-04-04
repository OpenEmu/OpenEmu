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

	TLCS900h_interpret_single.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

#ifndef __TLCS900H_SINGLE__
#define __TLCS900H_SINGLE__
//=========================================================================

//===== NOP
void sngNOP(void);

//===== NORMAL
void sngNORMAL(void);

//===== PUSH SR
void sngPUSHSR(void);

//===== POP SR
void sngPOPSR(void);

//===== MAX
void sngMAX(void);

//===== HALT
void sngHALT(void);

//===== EI #3
void sngEI(void);

//===== RETI
void sngRETI(void);

//===== LD (n), n
void sngLD8_8(void);

//===== PUSH n
void sngPUSH8(void);

//===== LD (n), nn
void sngLD8_16(void);

//===== PUSH nn
void sngPUSH16(void);

//===== INCF
void sngINCF(void);

//===== DECF
void sngDECF(void);

//===== RET condition
void sngRET(void);

//===== RETD dd
void sngRETD(void);

//===== RCF
void sngRCF(void);

//===== SCF
void sngSCF(void);

//===== CCF
void sngCCF(void);

//===== ZCF
void sngZCF(void);

//===== PUSH A
void sngPUSHA(void);

//===== POP A
void sngPOPA(void);

//===== EX F,F'
void sngEX(void);

//===== LDF #3
void sngLDF(void);

//===== PUSH F
void sngPUSHF(void);

//===== POP F
void sngPOPF(void);

//===== JP nn
void sngJP16(void);

//===== JP nnn
void sngJP24(void);

//===== CALL #16
void sngCALL16(void);

//===== CALL #24
void sngCALL24(void);

//===== CALR $+3+d16
void sngCALR(void);

//===== LD R, n
void sngLDB(void);

//===== PUSH RR
void sngPUSHW(void);

//===== LD RR, nn
void sngLDW(void);

//===== PUSH XRR
void sngPUSHL(void);

//===== LD XRR, nnnn
void sngLDL(void);

//===== POP RR
void sngPOPW(void);

//===== POP XRR
void sngPOPL(void);

//===== JR cc,PC + d
void sngJR(void);

//===== JR cc,PC + dd
void sngJRL(void);

//===== LDX dst,src
void sngLDX(void);

//===== SWI num
void sngSWI(void);

//=============================================================================
#endif
