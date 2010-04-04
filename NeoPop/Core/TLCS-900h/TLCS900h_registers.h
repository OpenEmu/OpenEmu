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

	TLCS900h_registers.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 AUG 2002 - neopop_uk
=======================================
- Fixed potential precidence problems in regX and rCodeX by using ()'s

//---------------------------------------------------------------------------
*/

#ifndef __TLCS900H_REGISTERS__
#define __TLCS900H_REGISTERS__
//=============================================================================

void reset_registers(void);
void dump_registers_TLCS900h(void);

//The value read by bad rCodes, leave 0, improves "Gals Fighters"
#define RERR_VALUE		0

//=============================================================================

extern _u32 pc;
extern _u16	sr;
extern _u8 f_dash;

extern _u32 gprBank[4][4], gpr[4];

extern _u32 rErr;

extern _u8 statusRFP;

//GPR Access
extern _u8* gprMapB[4][8];
extern _u16* gprMapW[4][8];
extern _u32* gprMapL[4][8];

#define regB(x)	(*(gprMapB[statusRFP][(x)]))
#define regW(x)	(*(gprMapW[statusRFP][(x)]))
#define regL(x)	(*(gprMapL[statusRFP][(x)]))

//Reg.Code Access
extern _u8* regCodeMapB[4][256];
extern _u16* regCodeMapW[4][128];
extern _u32* regCodeMapL[4][64];

#define rCodeB(r)	(*(regCodeMapB[statusRFP][(r)]))
#define rCodeW(r)	(*(regCodeMapW[statusRFP][(r) >> 1]))
#define rCodeL(r)	(*(regCodeMapL[statusRFP][(r) >> 2]))

//Common Registers
#define REGA		(regB(1))
#define REGWA		(regW(0))
#define REGBC		(regW(1))
#define REGXSP		(gpr[3])

//=============================================================================

_u8 statusIFF(void);
void setStatusIFF(_u8 iff);

void setStatusRFP(_u8 rfp);
void changedSP(void);

#define FLAG_S ((sr & 0x0080) >> 7)
#define FLAG_Z ((sr & 0x0040) >> 6)
#define FLAG_H ((sr & 0x0010) >> 4)
#define FLAG_V ((sr & 0x0004) >> 2)
#define FLAG_N ((sr & 0x0002) >> 1)
#define FLAG_C (sr & 1)

#define SETFLAG_S(s) { _u16 sr1 = sr & 0xFF7F; if (s) sr1 |= 0x0080; sr = sr1; }
#define SETFLAG_Z(z) { _u16 sr1 = sr & 0xFFBF; if (z) sr1 |= 0x0040; sr = sr1; }
#define SETFLAG_H(h) { _u16 sr1 = sr & 0xFFEF; if (h) sr1 |= 0x0010; sr = sr1; }
#define SETFLAG_V(v) { _u16 sr1 = sr & 0xFFFB; if (v) sr1 |= 0x0004; sr = sr1; }
#define SETFLAG_N(n) { _u16 sr1 = sr & 0xFFFD; if (n) sr1 |= 0x0002; sr = sr1; }
#define SETFLAG_C(c) { _u16 sr1 = sr & 0xFFFE; if (c) sr1 |= 0x0001; sr = sr1; }

#define SETFLAG_S0		{ sr &= 0xFF7F;	}
#define SETFLAG_Z0		{ sr &= 0xFFBF;	}
#define SETFLAG_H0		{ sr &= 0xFFEF;	}
#define SETFLAG_V0		{ sr &= 0xFFFB;	}
#define SETFLAG_N0		{ sr &= 0xFFFD;	}
#define SETFLAG_C0		{ sr &= 0xFFFE;	}

#define SETFLAG_S1		{ sr |= 0x0080; }
#define SETFLAG_Z1		{ sr |= 0x0040; }
#define SETFLAG_H1		{ sr |= 0x0010; }
#define SETFLAG_V1		{ sr |= 0x0004; }
#define SETFLAG_N1		{ sr |= 0x0002; }
#define SETFLAG_C1		{ sr |= 0x0001; }

//=============================================================================
#endif
