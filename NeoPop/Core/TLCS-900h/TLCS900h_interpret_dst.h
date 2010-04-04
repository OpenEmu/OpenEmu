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

	TLCS900h_interpret_dst.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

#ifndef __TLCS900H_DST__
#define __TLCS900H_DST__
//=========================================================================

//===== LD (mem),#
void dstLDBi(void);

//===== LD (mem),#
void dstLDWi(void);

//===== POP (mem)
void dstPOPB(void);

//===== POP (mem)
void dstPOPW(void);

//===== LD (mem),(nn)
void dstLDBm16(void);

//===== LD (mem),(nn)
void dstLDWm16(void);

//===== LDA R,mem
void dstLDAW(void);

//===== LDA R,mem
void dstLDAL(void);

//===== ANDCF A,(mem)
void dstANDCFA(void);

//===== ORCF A,(mem)
void dstORCFA(void);

//===== XORCF A,(mem)
void dstXORCFA(void);

//===== LDCF A,(mem)
void dstLDCFA(void);

//===== STCF A,(mem)
void dstSTCFA(void);

//===== LD (mem),R
void dstLDBR(void);

//===== LD (mem),R
void dstLDWR(void);

//===== LD (mem),R
void dstLDLR(void);

//===== ANDCF #3,(mem)
void dstANDCF(void);

//===== ORCF #3,(mem)
void dstORCF(void);

//===== XORCF #3,(mem)
void dstXORCF(void);

//===== LDCF #3,(mem)
void dstLDCF(void);

//===== STCF #3,(mem)
void dstSTCF(void);

//===== TSET #3,(mem)
void dstTSET(void);

//===== RES #3,(mem)
void dstRES(void);

//===== SET #3,(mem)
void dstSET(void);

//===== CHG #3,(mem)
void dstCHG(void);

//===== BIT #3,(mem)
void dstBIT(void);

//===== JP cc,mem
void dstJP(void);

//===== CALL cc,mem
void dstCALL(void);

//===== RET cc
void dstRET(void);

//=========================================================================
#endif