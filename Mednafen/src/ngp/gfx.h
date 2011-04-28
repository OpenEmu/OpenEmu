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

#ifndef __GFX__
#define __GFX__
//=============================================================================

#define ZDEPTH_BACK_SPRITE			2
#define ZDEPTH_BACKGROUND_SCROLL	3
#define ZDEPTH_MIDDLE_SPRITE		4
#define ZDEPTH_FOREGROUND_SCROLL	5
#define ZDEPTH_FRONT_SPRITE			6

class NGPGFX_CLASS
{
 public:

 NGPGFX_CLASS();
 ~NGPGFX_CLASS();

 void write8(uint32 address, uint8 data);
 void write16(uint32 address, uint16 data);

 uint8 read8(uint32 address);
 uint16 read16(uint32 address);

 int StateAction(StateMem *sm, int load, int data_only);
 bool ToggleLayer(int which);
 void set_pixel_format(const MDFN_PixelFormat &format);

 bool draw(MDFN_Surface *surface, bool skip);
 bool hint(void);

 void power(void);

 private:

 // TODO: Alignment for faster memset
 uint8 zbuffer[256];		//  __attribute__ ((aligned (8)));	//Line z-buffer
 uint16 cfb_scanline[256];	// __attribute__ ((aligned (8)));

 uint8 winx, winw;
 uint8 winy, winh;
 uint8 scroll1x, scroll1y;
 uint8 scroll2x, scroll2y;
 uint8 scrollsprx, scrollspry;
 uint8 planeSwap;
 uint8 bgc, oowc, negative;

 void reset(void);
 void delayed_settings(void);

 void draw_scanline_colour(int, int);
 void drawColourPattern(uint8 screenx, uint16 tile, uint8 tiley, uint16 mirror,
                                 uint16* palette_ptr, uint8 pal, uint8 depth);
 void draw_colour_scroll1(uint8 depth, int ngpc_scanline);
 void draw_colour_scroll2(uint8 depth, int ngpc_scanline);

 void draw_scanline_mono(int, int);
 void MonoPlot(uint8 x, uint8* palette_ptr, uint16 pal_hi, uint8 index, uint8 depth);
 void drawMonoPattern(uint8 screenx, uint16 tile, uint8 tiley, uint16 mirror,
                                 uint8* palette_ptr, uint16 pal, uint8 depth);
 void draw_mono_scroll1(uint8 depth, int ngpc_scanline);
 void draw_mono_scroll2(uint8 depth, int ngpc_scanline);



 uint8 ScrollVRAM[4096];  // 9000-9fff
 uint8 CharacterRAM[8192]; // a000-bfff
 uint8 SpriteVRAM[256]; // 8800-88ff
 uint8 SpriteVRAMColor[0x40]; // 8C00-8C3F
 uint8 ColorPaletteRAM[0x200]; // 8200-83ff

 uint8 SPPLT[6];
 uint8 SCRP1PLT[6];
 uint8 SCRP2PLT[6];

 uint8 raster_line;
 uint8 S1SO_H, S1SO_V, S2SO_H, S2SO_V;
 uint8 WBA_H, WBA_V, WSI_H, WSI_V;
 bool C_OVR, BLNK;
 uint8 PO_H, PO_V;
 uint8 P_F;
 uint8 BG_COL;
 uint8 CONTROL_2D;
 uint8 CONTROL_INT;
 uint8 SCREEN_PERIOD;
 uint8 K2GE_MODE;

 uint32 ColorMap[4096];

 int layer_enable;
};

#endif

