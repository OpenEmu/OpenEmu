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

//=============================================================================

void NGPGFX_CLASS::MonoPlot(uint8 x, uint8* palette_ptr, uint16 pal_hi, uint8 index, uint8 depth)
{
	uint8 data8;

	//Clip
	if (index == 0 || x < winx || x >= (winw + winx) || x >= SCREEN_WIDTH)
		return;

	//Depth check, <= to stop later sprites overwriting pixels!
	if (depth <= zbuffer[x]) return;
	zbuffer[x] = depth;

	//Get the colour of the pixel
	if (pal_hi)
		data8 = palette_ptr[3 + index - 1];
	else
		data8 = palette_ptr[0 + index - 1];

	uint16 r = (data8 & 7) << 1;
	uint16 g = (data8 & 7) << 5;
	uint16 b = (data8 & 7) << 9;

	if (negative)
		cfb_scanline[x] = (r | g | b);
	else
		cfb_scanline[x] = ~(r | g | b);
}

void NGPGFX_CLASS::drawMonoPattern(uint8 screenx, uint16 tile, uint8 tiley, uint16 mirror, 
				 uint8* palette_ptr, uint16 pal, uint8 depth)
{
	//Get the data for th e "tiley'th" line of "tile".
	uint16 data = le16toh(*(uint16*)(CharacterRAM + (tile * 16) + (tiley * 2)));

	//Horizontal Flip
	if (mirror)
	{
		MonoPlot(screenx + 7, palette_ptr, pal, (data & 0xC000) >> 0xE, depth);
		MonoPlot(screenx + 6, palette_ptr, pal, (data & 0x3000) >> 0xC, depth);
		MonoPlot(screenx + 5, palette_ptr, pal, (data & 0x0C00) >> 0xA, depth);
		MonoPlot(screenx + 4, palette_ptr, pal, (data & 0x0300) >> 0x8, depth);
		MonoPlot(screenx + 3, palette_ptr, pal, (data & 0x00C0) >> 0x6, depth);
		MonoPlot(screenx + 2, palette_ptr, pal, (data & 0x0030) >> 0x4, depth);
		MonoPlot(screenx + 1, palette_ptr, pal, (data & 0x000C) >> 0x2, depth);
		MonoPlot(screenx + 0, palette_ptr, pal, (data & 0x0003) >> 0x0, depth);
	}
	else
	//Normal
	{
		MonoPlot(screenx + 0, palette_ptr, pal, (data & 0xC000) >> 0xE, depth);
		MonoPlot(screenx + 1, palette_ptr, pal, (data & 0x3000) >> 0xC, depth);
		MonoPlot(screenx + 2, palette_ptr, pal, (data & 0x0C00) >> 0xA, depth);
		MonoPlot(screenx + 3, palette_ptr, pal, (data & 0x0300) >> 0x8, depth);
		MonoPlot(screenx + 4, palette_ptr, pal, (data & 0x00C0) >> 0x6, depth);
		MonoPlot(screenx + 5, palette_ptr, pal, (data & 0x0030) >> 0x4, depth);
		MonoPlot(screenx + 6, palette_ptr, pal, (data & 0x000C) >> 0x2, depth);
		MonoPlot(screenx + 7, palette_ptr, pal, (data & 0x0003) >> 0x0, depth);
	}
}

void NGPGFX_CLASS::draw_mono_scroll1(uint8 depth, int ngpc_scanline)
{
	uint8 tx, row, line;
	uint16 data16;

	line = ngpc_scanline + scroll1y;
	row = line & 7;	//Which row?

	//Draw Foreground scroll plane (Scroll 1)
	for (tx = 0; tx < 32; tx++)
	{
		data16 = le16toh(*(uint16*)(ScrollVRAM + ((tx + ((line >> 3) << 5)) << 1)));
		
		//Draw the line of the tile
		drawMonoPattern((tx << 3) - scroll1x, data16 & 0x01FF, 
			(data16 & 0x4000) ? 7 - row : row, data16 & 0x8000, SCRP1PLT,
			data16 & 0x2000, depth);
	}
}

void NGPGFX_CLASS::draw_mono_scroll2(uint8 depth, int ngpc_scanline)
{
	uint8 tx, row, line;
	uint16 data16;

	line = ngpc_scanline + scroll2y;
	row = line & 7;	//Which row?

	//Draw Background scroll plane (Scroll 2)
	for (tx = 0; tx < 32; tx++)
	{
		data16 = le16toh(*(uint16*)(ScrollVRAM + 0x0800 + ((tx + ((line >> 3) << 5)) << 1)));
		
		//Draw the line of the tile
		drawMonoPattern((tx << 3) - scroll2x, data16 & 0x01FF, 
			(data16 & 0x4000) ? 7 - row : row, data16 & 0x8000, SCRP2PLT,
			data16 & 0x2000, depth);
	}
}

void NGPGFX_CLASS::draw_scanline_mono(int layer_enable, int ngpc_scanline)
{
	int16 lastSpriteX;
	int16 lastSpriteY;
	int spr;
	uint16 data16;

	memset(cfb_scanline, 0, SCREEN_WIDTH * sizeof(uint16));
	memset(zbuffer, 0, SCREEN_WIDTH);

	//Window colour
	uint16 r = (uint16)oowc << 1;
	uint16 g = (uint16)oowc << 5;
	uint16 b = (uint16)oowc << 9;
	
	if (negative)
		data16 = (r | g | b);
	else
		data16 = ~(r | g | b);

	//Top
	if (ngpc_scanline < winy)
	{
		for (int x = 0; x < SCREEN_WIDTH; x++)
			cfb_scanline[x] = data16;
	}
	else
	{
		//Middle
		if (ngpc_scanline < winy + winh)
		{
			for (int x = 0; x < min(winx, SCREEN_WIDTH); x++)
				cfb_scanline[x] = data16;
			for (int x = min(winx + winw, SCREEN_WIDTH); x < SCREEN_WIDTH; x++)
				cfb_scanline[x] = data16;
		}
		else	//Bottom
		{
			for (int x = 0; x < SCREEN_WIDTH; x++)
				cfb_scanline[x] = data16;
		}
	}

	//Ignore above and below the window's top and bottom
	if (ngpc_scanline >= winy && ngpc_scanline < winy + winh)
	{
		//Background colour Enabled?
		if ((bgc & 0xC0) == 0x80)
		{
			r = (uint16)(bgc & 7) << 1;
			g = (uint16)(bgc & 7) << 5;
			b = (uint16)(bgc & 7) << 9;
			data16 = ~(r | g | b);
		}
		else data16 = 0x0FFF;

		if (negative) data16 = ~data16;
		
		//Draw background!
		for (int x = winx; x < min(winx + winw, SCREEN_WIDTH); x++)	
			cfb_scanline[x] = data16;

		//Swap Front/Back scroll planes?
		if (planeSwap)
		{
			if(layer_enable & 1)
			 draw_mono_scroll1(ZDEPTH_BACKGROUND_SCROLL, ngpc_scanline);		//Swap
			if(layer_enable & 2)
			 draw_mono_scroll2(ZDEPTH_FOREGROUND_SCROLL, ngpc_scanline);
		}
		else
		{
			if(layer_enable & 1)
			 draw_mono_scroll2(ZDEPTH_BACKGROUND_SCROLL, ngpc_scanline);		//Normal
			if(layer_enable & 2)
			 draw_mono_scroll1(ZDEPTH_FOREGROUND_SCROLL, ngpc_scanline);
		}

		//Draw Sprites
		//Last sprite position, (defaults to top-left, sure?)
		lastSpriteX = 0;
		lastSpriteY = 0;
		if(layer_enable & 4)
		for (spr = 0; spr < 64; spr++)
		{
			uint8 priority, row;
			uint8 sx = SpriteVRAM[(spr * 4) + 2];	//X position
			uint8 sy = SpriteVRAM[(spr * 4) + 3];	//Y position
			int16 x = sx;
			int16 y = sy;
			
			data16 = le16toh(*(uint16*)(SpriteVRAM + (spr * 4)));
			priority = (data16 & 0x1800) >> 11;

			if (data16 & 0x0400) x = lastSpriteX + sx;	//Horizontal chain?
			if (data16 & 0x0200) y = lastSpriteY + sy;	//Vertical chain?

			//Store the position for chaining
			lastSpriteX = x;
			lastSpriteY = y;
			
			//Visible?
			if (priority == 0)	continue;

			//Scroll the sprite
			x += scrollsprx;
			y += scrollspry;

			//Off-screen?
			if (x > 248 && x < 256)	x = x - 256; else x &= 0xFF;
			if (y > 248 && y < 256)	y = y - 256; else y &= 0xFF;

			//In range?
			if (ngpc_scanline >= y && ngpc_scanline <= y + 7)
			{
				row = (ngpc_scanline - y) & 7;	//Which row?
				drawMonoPattern((uint8)x, data16 & 0x01FF, 
					(data16 & 0x4000) ? 7 - row : row, data16 & 0x8000,
					SPPLT, data16 & 0x2000, priority << 1); 
			}
		}

	}

	//==========
}

//=============================================================================
