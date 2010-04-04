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

	TLCS900h_interpret_src.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

#ifndef __TLCS900H_SRC__
#define __TLCS900H_SRC__
//=========================================================================

//===== PUSH (mem)
void srcPUSH(void);

//===== RLD A,(mem)
void srcRLD(void);

//===== RRD A,(mem)
void srcRRD(void);

//===== LDI
void srcLDI(void);

//===== LDIR
void srcLDIR(void);

//===== LDD
void srcLDD(void);

//===== LDDR
void srcLDDR(void);

//===== CPI
void srcCPI(void);

//===== CPIR
void srcCPIR(void);

//===== CPD
void srcCPD(void);

//===== CPDR
void srcCPDR(void);

//===== LD (nn),(mem)
void srcLD16m(void);

//===== LD R,(mem)
void srcLD(void);

//===== EX (mem),R
void srcEX(void);

//===== ADD (mem),#
void srcADDi(void);

//===== ADC (mem),#
void srcADCi(void);

//===== SUB (mem),#
void srcSUBi(void);

//===== SBC (mem),#
void srcSBCi(void);

//===== AND (mem),#
void srcANDi(void);

//===== OR (mem),#
void srcORi(void);

//===== XOR (mem),#
void srcXORi(void);

//===== CP (mem),#
void srcCPi(void);

//===== MUL RR,(mem)
void srcMUL(void);

//===== MULS RR,(mem)
void srcMULS(void);

//===== DIV RR,(mem)
void srcDIV(void);

//===== DIVS RR,(mem)
void srcDIVS(void);

//===== INC #3,(mem)
void srcINC(void);

//===== DEC #3,(mem)
void srcDEC(void);

//===== RLC (mem)
void srcRLC(void);

//===== RRC (mem)
void srcRRC(void);

//===== RL (mem)
void srcRL(void);

//===== RR (mem)
void srcRR(void);

//===== SLA (mem)
void srcSLA(void);

//===== SRA (mem)
void srcSRA(void);

//===== SLL (mem)
void srcSLL(void);

//===== SRL (mem)
void srcSRL(void);

//===== ADD R,(mem)
void srcADDRm(void);

//===== ADD (mem),R
void srcADDmR(void);

//===== ADC R,(mem)
void srcADCRm(void);

//===== ADC (mem),R
void srcADCmR(void);

//===== SUB R,(mem)
void srcSUBRm(void);

//===== SUB (mem),R
void srcSUBmR(void);

//===== SBC R,(mem)
void srcSBCRm(void);

//===== SBC (mem),R
void srcSBCmR(void);

//===== AND R,(mem)
void srcANDRm(void);

//===== AND (mem),R
void srcANDmR(void);

//===== XOR R,(mem)
void srcXORRm(void);

//===== XOR (mem),R
void srcXORmR(void);

//===== OR R,(mem)
void srcORRm(void);

//===== OR (mem),R
void srcORmR(void);

//===== CP R,(mem)
void srcCPRm(void);

//===== CP (mem),R
void srcCPmR(void);

//=============================================================================
#endif
