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

	dma.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

25 JUL 2002 - neopop_uk
=======================================
- Added function prototype for DMA_update

//---------------------------------------------------------------------------
*/

#ifndef __DMA__
#define __DMA__
//=============================================================================

void reset_dma(void);

void DMA_update(int channel);

extern _u32 dmaS[4], dmaD[4];
extern _u16 dmaC[4];
extern _u8 dmaM[4];

_u8  dmaLoadB(_u8 cr);
_u16 dmaLoadW(_u8 cr);
_u32 dmaLoadL(_u8 cr);

void dmaStoreB(_u8 cr, _u8 data);
void dmaStoreW(_u8 cr, _u16 data);
void dmaStoreL(_u8 cr, _u32 data);

//=============================================================================
#endif
