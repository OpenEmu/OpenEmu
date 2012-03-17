/***************************************************************************
 * Gens: Miscellaneous Effects.                                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include <math.h>

#ifdef __linux__
#include <SDL/SDL.h>
#endif /* __linux__ */

#include "v_effects.hpp"
#include "gens_core/misc/misc.h"
#include "gens_core/gfx/fastblur.hpp"

#include "emulator/g_main.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"


/**
 * Update_Gens_Logo(): Update the Gens logo.
 * @return 1 on success.
 */
int Update_Gens_Logo(void)
{
	// TODO: 32-bit support.
	
	int i, j, m, n;
	static short tab[64000], Init = 0;
	static float renv = 0, /*ang = 0,*/ zoom_x = 0, zoom_y = 0, pas;
	unsigned short c;
	
	if (!Init)
	{
		// TODO: Don't use SDL for image loading.
		// TODO: Fix this for Win32.
#ifdef __linux__
		SDL_Surface* Logo;
		
		Logo = SDL_LoadBMP(GENS_DATADIR "/gens_big.bmp");
		
		SDL_LockSurface(Logo);
		memcpy(tab, Logo->pixels, 64000);
		SDL_UnlockSurface(Logo);
		
		pas = 0.05;
		Init = 1;
#endif
	}
	
	renv += pas;
	zoom_x = sin(renv);
	if (zoom_x == 0.0)
			zoom_x = 0.0000001;
	zoom_x = (1 / zoom_x) * 1;
	zoom_y = 1;
	
	if (VDP_Reg.Set4 & 0x1)
	{
		for(j = 0; j < 240; j++)
		{
			for(i = 0; i < 320; i++)
			{
				m = (float)(i - 160) * zoom_x;
				n = (float)(j - 120) * zoom_y;
				
				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5))
						MD_Screen[TAB336[j] + i + 8] = c;
				}
			}
		}
	}
	else
	{
		for(j = 0; j < 240; j++)
		{
			for(i = 0; i < 256; i++)
			{
				m = (float)(i - 128) * zoom_x;
				n = (float)(j - 120) * zoom_y;
				
				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5)) MD_Screen[TAB336[j] + i + 8] = c;
				}
			}
		}
	}
	
	Fast_Blur();
	//draw->Flip();
	
	return 1;
}


/**
 * Update_Crazy_Effect(): Update the "Crazy" Effect.
 * @return 1 on success.
 */
int Update_Crazy_Effect(unsigned char introEffectColor)
{
	int i, j, offset;
	int r = 0, v = 0, b = 0, prev_l, prev_p;
	int RB, G;
	
	for(offset = 336 * 240, j = 0; j < 240; j++)
	{
		for(i = 0; i < 336; i++, offset--)
		{
			prev_l = MD_Screen[offset - 336];
			prev_p = MD_Screen[offset - 1];
			
			if (bppMD == 15)
			{
				RB = ((prev_l & 0x7C1F) + (prev_p & 0x7C1F)) >> 1;
				G = ((prev_l & 0x03E0) + (prev_p & 0x03E0)) >> 1;
				
				if (introEffectColor & 0x4)
				{
					r = RB & 0x7C00;
					if (rand() > 0x2C00) r += 0x0400;
					else r -= 0x0400;
					if (r > 0x7C00) r = 0x7C00;
					else if (r < 0x0400) r = 0;
				}
				
				if (introEffectColor & 0x2)
				{
					v = G & 0x03E0;
					if (rand() > 0x2C00) v += 0x0020;
					else v -= 0x0020;
					if (v > 0x03E0) v = 0x03E0;
					else if (v < 0x0020) v = 0;
				}
				
				if (introEffectColor & 0x1)
				{
					b = RB & 0x001F;
					if (rand() > 0x2C00) b++;
					else b--;
					if (b > 0x1F) b = 0x1F;
					else if (b < 0) b = 0;
				}
			}
			else
			{
				RB = ((prev_l & 0xF81F) + (prev_p & 0xF81F)) >> 1;
				G = ((prev_l & 0x07C0) + (prev_p & 0x07C0)) >> 1;
				
				if (introEffectColor & 0x4)
				{
					r = RB & 0xF800;
					if (rand() > 0x2C00) r += 0x0800;
					else r -= 0x0800;
					if (r > 0xF800) r = 0xF800;
					else if (r < 0x0800) r = 0;
				}
				
				if (introEffectColor & 0x2)
				{
					v = G & 0x07C0;
					if (rand() > 0x2C00) v += 0x0040;
					else v -= 0x0040;
					if (v > 0x07C0) v = 0x07C0;
					else if (v < 0x0040) v = 0;
				}
				
				if (introEffectColor & 0x1)
				{
					b = RB & 0x001F;
					if (rand() > 0x2C00) b++;
					else b--;
					if (b > 0x1F) b = 0x1F;
					else if (b < 0) b = 0;
				}
			}
			
			MD_Screen[offset] = r + v + b;
		}
	}
	
	//draw->Flip();
	
	return 1;
}


/**
 * Pause_Screen_int(): Tint the screen a purple hue to indicate that emulation is paused.
 * @param screen Pointer to the MD screen buffer.
 * @param RMask Red component mask.
 * @param GMask Green component mask.
 * @param BMask Blue component mask.
 * @param RShift Red component shift.
 * @param GShift Green component shift.
 * @param BShift Blue component shift.
 */
template<typename pixel>
static void Pause_Screen_int(pixel *mdScreen, pixel RMask, pixel GMask, pixel BMask,
			     unsigned char RShift, unsigned char GShift, unsigned char BShift)
{
	unsigned char r, g, b, nr, ng, nb;
	unsigned short x, y;
	unsigned int offset = 0;
	unsigned short sum;
	
	for (y = 0; y < 240; y++)
	{
		for (x = 0; x < 336; x++, offset++)
		{
			r = (unsigned char)((mdScreen[offset] & RMask) >> RShift);
			g = (unsigned char)((mdScreen[offset] & GMask) >> GShift);
			b = (unsigned char)((mdScreen[offset] & BMask) >> BShift);
			
			sum = r + g + b;
			sum /= 3;
			nr = ng = nb = (unsigned char)sum;
			
			// L-shift the blue component to tint the image.
			nb <<= 1;
			if (nb > 0x1F)
				nb = 0x1F;
			
			// Mask off the LSB.
			nr &= 0x1E;
			ng &= 0x1E;
			nb &= 0x1E;
			
			mdScreen[offset] = (nr << RShift) | (ng << GShift) | (nb << BShift);
		}
	}
}


/**
 * Pause_Screen(): Tint the screen a purple hue to indicate that emulation is paused.
 */
void Pause_Screen(void)
{
	if (bppMD == 15)
		Pause_Screen_int(MD_Screen, (unsigned short)0x7C00, (unsigned short)0x03E0, (unsigned short)0x001F, 10, 5, 0);
	else if (bppMD == 16)
		Pause_Screen_int(MD_Screen, (unsigned short)0xF800, (unsigned short)0x07C0, (unsigned short)0x001F, 11, 6, 0);
	else //if (bppMD == 32)
		Pause_Screen_int(MD_Screen32, (unsigned int)0xFF0000, (unsigned int)0x00FF00, (unsigned int)0x0000FF, 16+3, 8+3, 0+3);
	
	if (ice == 2)
		ice = 3;
}
