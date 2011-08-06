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

#include "neopop.h"
#include "mem.h"
#include "gfx.h"
#include "interrupt.h"
#include "dma.h"
#include "TLCS900h_registers.h"

NGPGFX_CLASS::NGPGFX_CLASS(void)
{
 layer_enable = 1 | 2 | 4;
}

NGPGFX_CLASS::~NGPGFX_CLASS()
{


}

void NGPGFX_CLASS::reset(void)
{
 memset(SPPLT, 0x7, sizeof(SPPLT));
 memset(SCRP1PLT, 0x7, sizeof(SCRP1PLT));
 memset(SCRP2PLT, 0x7, sizeof(SCRP2PLT));

 raster_line = 0;
 S1SO_H = 0;
 S1SO_V = 0;
 S2SO_H = 0;
 S2SO_V = 0;
 WBA_H = 0;
 WBA_V = 0;

 WSI_H = 0xFF;
 WSI_V = 0xFF;

 C_OVR = 0;
 BLNK = 0;

 PO_H = 0;
 PO_V = 0;
 P_F = 0;

 BG_COL = 0x7;
 CONTROL_2D = 0;
 CONTROL_INT = 0;
 SCREEN_PERIOD = 0xC6;
 K2GE_MODE = 0;

 delayed_settings();
}

void NGPGFX_CLASS::power(void)
{
 reset();

 memset(ScrollVRAM, 0, sizeof(ScrollVRAM));
 memset(CharacterRAM, 0, sizeof(CharacterRAM));
 memset(SpriteVRAM, 0, sizeof(SpriteVRAM));
 memset(SpriteVRAMColor, 0, sizeof(SpriteVRAMColor));
 memset(ColorPaletteRAM, 0, sizeof(ColorPaletteRAM));
}

void NGPGFX_CLASS::delayed_settings(void)
{
	//Window dimensions
	winx = WBA_H;
	winy = WBA_V;
	winw = WSI_H;
	winh = WSI_V;

	//Scroll Planes (Confirmed delayed)
	scroll1x = S1SO_H;
	scroll1y = S1SO_V;

	scroll2x = S2SO_H;
	scroll2y = S2SO_V;

	//Sprite offset (Confirmed delayed)
	scrollsprx = PO_H;
	scrollspry = PO_V;

	//Plane Priority (Confirmed delayed)
	planeSwap = P_F & 0x80;

	//Background colour register (Confirmed delayed)
	bgc = BG_COL;

	//2D Control register (Confirmed delayed)
	oowc = CONTROL_2D & 7;
	negative = CONTROL_2D & 0x80;
}

bool NGPGFX_CLASS::hint(void)
{
        //H_Int / Delayed settings
        if ((raster_line < SCREEN_HEIGHT-1 || raster_line == SCREEN_PERIOD))
        {
                delayed_settings(); //Get delayed settings

                //Allowed?
                if (CONTROL_INT & 0x40)
			return(1);
        }
	return(0);
}

void NGPGFX_CLASS::set_pixel_format(const MDFN_PixelFormat &format)
{
 for(int x = 0; x < 4096; x++)
 {
  int r = (x & 0xF) * 17;
  int g = ((x >> 4) & 0xF) * 17;
  int b = ((x >> 8) & 0xF) * 17;

  ColorMap[x] = format.MakeColor(r, g, b);
 }
}

bool NGPGFX_CLASS::draw(MDFN_Surface *surface, bool skip)
{
	bool ret = 0;

        //Draw the scanline
        if (raster_line < SCREEN_HEIGHT && !skip)
        {
                if (!K2GE_MODE)        draw_scanline_colour(layer_enable, raster_line);
                else                   draw_scanline_mono(layer_enable, raster_line);

                uint32 *dest = surface->pixels + surface->pitch32 * raster_line;
                for(int x = 0; x < SCREEN_WIDTH; x++)
                 dest[x] = ColorMap[cfb_scanline[x] & 4095];
        }
	raster_line++;

        //V_Int?
        if (raster_line == SCREEN_HEIGHT)
        {
                        BLNK = 1;
                        ret = 1;

			if(CONTROL_INT & 0x80) // (statusIFF() <= 4
			 TestIntHDMA(5, 0x0B);
        }

        //End of V_Int
        if(raster_line == SCREEN_PERIOD + 1)     //Last scanline + 1
        {
         raster_line = 0;
	 C_OVR = 0;
	 BLNK = 0;
        }

	return(ret);
}

int NGPGFX_CLASS::StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(raster_line),
  SFVAR(S1SO_H), SFVAR(S1SO_V), SFVAR(S2SO_H), SFVAR(S2SO_V),
  SFVAR(WBA_H), SFVAR(WBA_V), SFVAR(WSI_H), SFVAR(WSI_V),
  SFVAR(C_OVR), SFVAR(BLNK),
  SFVAR(PO_H), SFVAR(PO_V),
  SFVAR(P_F),
  SFVAR(BG_COL),
  SFVAR(CONTROL_2D),
  SFVAR(CONTROL_INT),
  SFVAR(SCREEN_PERIOD),
  SFVAR(K2GE_MODE),

  SFARRAY(SPPLT, 6),
  SFARRAY(SCRP1PLT, 6),
  SFARRAY(SCRP2PLT, 6),

  SFVAR(winx), SFVAR(winw),
  SFVAR(winy), SFVAR(winh),
  SFVAR(scroll1x), SFVAR(scroll1y),
  SFVAR(scroll2x), SFVAR(scroll2y),
  SFVAR(scrollsprx), SFVAR(scrollspry),
  SFVAR(planeSwap),
  SFVAR(bgc), SFVAR(oowc),

  SFVAR(negative),

  SFARRAY(ScrollVRAM, 4096),
  SFARRAY(CharacterRAM, 8192),
  SFARRAY(SpriteVRAM, 256),
  SFARRAY(SpriteVRAMColor, 0x40),
  SFARRAY(ColorPaletteRAM, 0x200),

  SFEND
 };

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "GFX"))
  return(0);

 return(1);
}

bool NGPGFX_CLASS::ToggleLayer(int which)
{
 layer_enable ^= 1 << which;
 return(layer_enable & (1 << which));
}

//extern uint32 ngpc_soundTS;
void NGPGFX_CLASS::write8(uint32 address, uint8 data)
{
 //if(address >= 0x8032 && address <= 0x8035)
 // printf("%08x %02x %d\n", address, data, ngpc_soundTS);

 if(address >= 0x9000 && address <= 0x9fff)
  ScrollVRAM[address - 0x9000] = data;
 else if(address >= 0xa000 && address <= 0xbfff)
  CharacterRAM[address - 0xa000] = data;
 else if(address >= 0x8800 && address <= 0x88ff)
  SpriteVRAM[address - 0x8800] = data;
 else if(address >= 0x8c00 && address <= 0x8c3f)
  SpriteVRAMColor[address - 0x8c00] = data & 0x0f;
 else if(address >= 0x8200 && address <= 0x83ff)
  ColorPaletteRAM[address - 0x8200] = data;
 else switch(address)
 {
  //default: printf("HuhW: %08x\n", address); break;

  case 0x8000: CONTROL_INT = data & 0xC0; break;
  case 0x8002: WBA_H = data; break;
  case 0x8003: WBA_V = data; break;
  case 0x8004: WSI_H = data; break;
  case 0x8005: WSI_V = data; break;
  case 0x8006: SCREEN_PERIOD = data; break;
  case 0x8012: CONTROL_2D = data & 0x87; break;
  case 0x8020: PO_H = data; break;
  case 0x8021: PO_V = data; break;
  case 0x8030: P_F = data & 0x80; break;
  case 0x8032: S1SO_H = data; break;
  case 0x8033: S1SO_V = data; break;
  case 0x8034: S2SO_H = data; break;
  case 0x8035: S2SO_V = data; break;

  case 0x8101: SPPLT[0] = data & 0x7; break;
  case 0x8102: SPPLT[1] = data & 0x7; break;
  case 0x8103: SPPLT[2] = data & 0x7; break;

  case 0x8105: SPPLT[3] = data & 0x7; break;
  case 0x8106: SPPLT[4] = data & 0x7; break;
  case 0x8107: SPPLT[5] = data & 0x7; break;

  case 0x8109: SCRP1PLT[0] = data & 0x7; break;
  case 0x810a: SCRP1PLT[1] = data & 0x7; break;
  case 0x810b: SCRP1PLT[2] = data & 0x7; break;

  case 0x810d: SCRP1PLT[3] = data & 0x7; break;
  case 0x810e: SCRP1PLT[4] = data & 0x7; break;
  case 0x810f: SCRP1PLT[5] = data & 0x7; break;

  case 0x8111: SCRP2PLT[0] = data & 0x7; break;
  case 0x8112: SCRP2PLT[1] = data & 0x7; break;
  case 0x8113: SCRP2PLT[2] = data & 0x7; break;

  case 0x8115: SCRP2PLT[3] = data & 0x7; break;
  case 0x8116: SCRP2PLT[4] = data & 0x7; break;
  case 0x8117: SCRP2PLT[5] = data & 0x7; break;

  case 0x8118: BG_COL = data & 0xC7; break;

  case 0x87e0: if(data == 0x52) 
	       {
		puts("GEreset");
		reset(); 
	       }
	       break;
  case 0x87e2: K2GE_MODE = data & 0x80; break;
 }
}

void NGPGFX_CLASS::write16(uint32 address, uint16 data)
{
 write8(address, data & 0xFF);
 write8(address + 1, data >> 8);
}

uint8 NGPGFX_CLASS::read8(uint32 address)
{
 if(address >= 0x9000 && address <= 0x9fff)
  return(ScrollVRAM[address - 0x9000]);
 else if(address >= 0xa000 && address <= 0xbfff)
  return(CharacterRAM[address - 0xa000]);
 else if(address >= 0x8800 && address <= 0x88ff)
  return(SpriteVRAM[address - 0x8800]);
 else if(address >= 0x8c00 && address <= 0x8c3f)
  return(SpriteVRAMColor[address - 0x8c00]);
 else if(address >= 0x8200 && address <= 0x83ff)
  return(ColorPaletteRAM[address - 0x8200]);
 else switch(address)
 {
  //default: printf("Huh: %08x\n", address); break;
  case 0x8000: return(CONTROL_INT);
  case 0x8002: return(WBA_H);
  case 0x8003: return(WBA_V);
  case 0x8004: return(WSI_H);
  case 0x8005: return(WSI_V);
  case 0x8006: return(SCREEN_PERIOD);

  case 0x8008: return( (uint8)((abs(TIMER_HINT_RATE - (int)timer_hint)) >> 2) ); //RAS.H read (Simulated horizontal raster position)
  case 0x8009: return(raster_line);
  case 0x8010: return((C_OVR ? 0x80 : 0x00) | (BLNK ? 0x40 : 0x00));
  case 0x8012: return(CONTROL_2D);
  case 0x8020: return(PO_H);
  case 0x8021: return(PO_V);
  case 0x8030: return(P_F);
  case 0x8032: return(S1SO_H);
  case 0x8033: return(S1SO_V);
  case 0x8034: return(S2SO_H);
  case 0x8035: return(S2SO_V);

  case 0x8101: return(SPPLT[0]); break;
  case 0x8102: return(SPPLT[1]); break;
  case 0x8103: return(SPPLT[2]); break;

  case 0x8105: return(SPPLT[3]); break;
  case 0x8106: return(SPPLT[4]); break;
  case 0x8107: return(SPPLT[5]); break;

  case 0x8108: return(SCRP1PLT[0]); break;
  case 0x8109: return(SCRP1PLT[1]); break;
  case 0x810a: return(SCRP1PLT[2]); break;

  case 0x810d: return(SCRP1PLT[3]); break;
  case 0x810e: return(SCRP1PLT[4]); break;
  case 0x810f: return(SCRP1PLT[5]); break;

  case 0x8111: return(SCRP2PLT[0]); break;
  case 0x8112: return(SCRP2PLT[1]); break;
  case 0x8113: return(SCRP2PLT[2]); break;

  case 0x8115: return(SCRP2PLT[3]); break;
  case 0x8116: return(SCRP2PLT[4]); break;
  case 0x8117: return(SCRP2PLT[5]); break;

  case 0x8118: return(BG_COL);

  case 0x87e2: return(K2GE_MODE);
 }

 return(0);
}

uint16 NGPGFX_CLASS::read16(uint32 address)
{
 return(read8(address) | (read8(address + 1) << 8));
}

