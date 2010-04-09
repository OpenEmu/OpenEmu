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

	gfx.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

24 JUL 2002 - neopop_uk
=======================================
- Added delayed setting of the Negative RGB output switch.

16 AUG 2002 - neopop_uk
=======================================
- Removed data8, data16 and data32

//---------------------------------------------------------------------------
*/

#ifndef __GFX__
#define __GFX__
//=============================================================================

#define ZDEPTH_BACK_SPRITE			2
#define ZDEPTH_BACKGROUND_SCROLL	3
#define ZDEPTH_MIDDLE_SPRITE		4
#define ZDEPTH_FOREGROUND_SCROLL	5
#define ZDEPTH_FRONT_SPRITE			6

//=============================================================================

//---------------------------
// Common Graphics Variables
//---------------------------

extern _u8 zbuffer[256];	//Line z-buffer
extern _u16* cfb_scanline;	//set = cfb + (scanline * SCREEN_WIDTH)

extern _u8 scanline;		//Current scanline

extern _u8 winx, winw;
extern _u8 winy, winh;
extern _u8 scroll1x, scroll1y;
extern _u8 scroll2x, scroll2y;
extern _u8 scrollsprx, scrollspry;
extern _u8 planeSwap;
extern _u8 bgc, oowc, negative;

extern _u16 r,g,b;

void gfx_delayed_settings(void);

//=============================================================================

void gfx_draw_scanline_colour(void);
void gfx_draw_scanline_mono(void);

//=============================================================================
#endif

