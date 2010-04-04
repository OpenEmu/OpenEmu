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

	TLCS900h_interpret_reg.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

#ifndef __TLCS900H_REG__
#define __TLCS900H_REG__
//=========================================================================

//===== LD r,#
void regLDi(void);

//===== PUSH r
void regPUSH(void);

//===== POP r
void regPOP(void);

//===== CPL r
void regCPL(void);

//===== NEG r
void regNEG(void);

//===== MUL rr,#
void regMULi(void);

//===== MULS rr,#
void regMULSi(void);

//===== DIV rr,#
void regDIVi(void);

//===== DIVS rr,#
void regDIVSi(void);

//===== LINK r,dd
void regLINK(void);

//===== UNLK r
void regUNLK(void);

//===== BS1F A,r
void regBS1F(void);

//===== BS1B A,r
void regBS1B(void);

//===== DAA r
void regDAA(void);

//===== EXTZ r
void regEXTZ(void);

//===== EXTS r
void regEXTS(void);

//===== PAA r
void regPAA(void);

//===== MIRR r
void regMIRR(void);

//===== MULA r
void regMULA(void);

//===== DJNZ r,d
void regDJNZ(void);

//===== ANDCF #,r
void regANDCFi(void);

//===== ORCF #,r
void regORCFi(void);

//===== XORCF #,r
void regXORCFi(void);

//===== LDCF #,r
void regLDCFi(void);

//===== STCF #,r
void regSTCFi(void);

//===== ANDCF A,r
void regANDCFA(void);

//===== ORCF A,r
void regORCFA(void);

//===== XORCF A,r
void regXORCFA(void);

//===== LDCF A,r
void regLDCFA(void);

//===== STCF A,r
void regSTCFA(void);

//===== LDC cr,r
void regLDCcrr(void);

//===== LDC r,cr
void regLDCrcr(void);

//===== RES #,r
void regRES(void);

//===== SET #,r
void regSET(void);

//===== CHG #,r
void regCHG(void);

//===== BIT #,r
void regBIT(void);

//===== TSET #,r
void regTSET(void);

//===== MINC1 #,r
void regMINC1(void);

//===== MINC2 #,r
void regMINC2(void);

//===== MINC4 #,r
void regMINC4(void);

//===== MDEC1 #,r
void regMDEC1(void);

//===== MDEC2 #,r
void regMDEC2(void);

//===== MDEC4 #,r
void regMDEC4(void);

//===== MUL RR,r
void regMUL(void);

//===== MULS RR,r
void regMULS(void);

//===== DIV RR,r
void regDIV(void);

//===== DIVS RR,r
void regDIVS(void);

//===== INC #3,r
void regINC(void);

//===== DEC #3,r
void regDEC(void);

//===== SCC cc,r
void regSCC(void);

//===== LD R,r
void regLDRr(void);

//===== LD r,R
void regLDrR(void);

//===== ADD R,r
void regADD(void);

//===== ADC R,r
void regADC(void);

//===== SUB R,r
void regSUB(void);

//===== SBC R,r
void regSBC(void);

//===== LD r,#3
void regLDr3(void);

//===== EX R,r
void regEX(void);

//===== ADD r,#
void regADDi(void);

//===== ADC r,#
void regADCi(void);

//===== SUB r,#
void regSUBi(void);

//===== SBC r,#
void regSBCi(void);

//===== CP r,#
void regCPi(void);

//===== AND r,#
void regANDi(void);

//===== OR r,#
void regORi(void);

//===== XOR r,#
void regXORi(void);

//===== AND R,r
void regAND(void);

//===== OR R,r
void regOR(void);

//===== XOR R,r
void regXOR(void);

//===== CP r,#3
void regCPr3(void);

//===== CP R,r
void regCP(void);

//===== RLC #,r
void regRLCi(void);

//===== RRC #,r
void regRRCi(void);

//===== RL #,r
void regRLi(void);

//===== RR #,r
void regRRi(void);

//===== SLA #,r
void regSLAi(void);

//===== SRA #,r
void regSRAi(void);

//===== SLL #,r
void regSLLi(void);

//===== SRL #,r
void regSRLi(void);

//===== RLC A,r
void regRLCA(void);

//===== RRC A,r
void regRRCA(void);

//===== RL A,r
void regRLA(void);

//===== RR A,r
void regRRA(void);

//===== SLA A,r
void regSLAA(void);

//===== SRA A,r
void regSRAA(void);

//===== SLL A,r
void regSLLA(void);

//===== SRL A,r
void regSRLA(void);

//=========================================================================
#endif
