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

uint8  dmaLoadB(uint8 cr);
uint16 dmaLoadW(uint8 cr);
uint32 dmaLoadL(uint8 cr);

void dmaStoreB(uint8 cr, uint8 data);
void dmaStoreW(uint8 cr, uint16 data);
void dmaStoreL(uint8 cr, uint32 data);

int MDFNNGPCDMA_StateAction(StateMem *sm, int load, int data_only);

//=============================================================================
#endif
