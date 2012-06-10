/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include        "../mednafen.h"
#include	"tblur.h"

typedef struct
{
 uint16 a, b, c, d;
} HQPixelEntry;

static uint32 *BlurBuf = NULL;
static uint32 AccumBlurAmount; // max of 16384, infinite blur!
static HQPixelEntry *AccumBlurBuf = NULL;

void TBlur_Init(void)
{
        std::string sn = MDFNGameInfo->shortname;

        if(MDFN_GetSettingB(std::string(sn + "." + std::string("tblur")).c_str()))
        {
         AccumBlurAmount = (uint32)(16384 * MDFN_GetSettingF(std::string(sn + "." + std::string("tblur.accum.amount")).c_str()) / 100);
         if(MDFN_GetSettingB(std::string(sn + "." + std::string("tblur.accum")).c_str()))
         {
          AccumBlurBuf = (HQPixelEntry *)calloc(sizeof(HQPixelEntry), MDFNGameInfo->fb_width * MDFNGameInfo->fb_height);
          MDFN_printf(_("Video temporal frame blur enabled with accumulation: %f.\n"), (double)AccumBlurAmount * 100 / 16384);
         }
         else
         {
          BlurBuf = (uint32 *)calloc(4, MDFNGameInfo->fb_width * MDFNGameInfo->fb_height);
          MDFN_printf(_("Video temporal frame blur enabled.\n"));
         }
        }
}

void TBlur_Run(EmulateSpecStruct *espec)
{
 MDFN_Surface *surface = espec->surface;
 uint32 *pXBuf = surface->pixels;
 const uint32 bb_pitch = MDFNGameInfo->fb_width;

 //printf("%d %d %d %d\n", espec->surface->format.Rshift, espec->surface->format.Gshift, espec->surface->format.Bshift, espec->surface->format.Ashift);
 if(AccumBlurBuf)
 {
  for(int y = 0; y < espec->DisplayRect.h; y++)
  {
   int xw = espec->DisplayRect.w;
   int xs = espec->DisplayRect.x;

   if(espec->LineWidths[0].w != ~0)
   {
    xw = espec->LineWidths[espec->DisplayRect.y + y].w;
    xs = espec->LineWidths[espec->DisplayRect.y + y].x;
   }

   if(AccumBlurAmount == 8192)
   {
    for(int x = 0; x < xw; x++)
    {
     uint32 color;
     HQPixelEntry mixcolor;

     color = pXBuf[(y + espec->DisplayRect.y) * surface->pitch32 + xs + x];
     mixcolor = AccumBlurBuf[y * bb_pitch + x];
     mixcolor.a = ((uint32)mixcolor.a + ((color & 0xFF) << 8)) >> 1;
     mixcolor.b = ((uint32)mixcolor.b + ((color & 0xFF00))) >> 1;
     mixcolor.c = ((uint32)mixcolor.c + ((color & 0xFF0000) >> 8)) >> 1;
     mixcolor.d = ((uint32)mixcolor.d + ((color & 0xFF000000) >> 16)) >> 1;

     AccumBlurBuf[y * bb_pitch + x] = mixcolor;

     pXBuf[(y + espec->DisplayRect.y) * surface->pitch32 + xs + x] = ((mixcolor.a >> 8) << 0) | ((mixcolor.b >> 8) << 8) |
        ((mixcolor.c >> 8) << 16) | ((mixcolor.d >> 8) << 24);
    }
   }
   else
   {
    uint32 InvAccumBlurAmount = 16384 - AccumBlurAmount;

    for(int x = 0; x < xw; x++)
    {
     uint32 color;
     HQPixelEntry mixcolor;
     color = pXBuf[(y + espec->DisplayRect.y) * surface->pitch32 + xs + x];

     mixcolor = AccumBlurBuf[y * bb_pitch + x];
     mixcolor.a = ((uint32)mixcolor.a * AccumBlurAmount + InvAccumBlurAmount * ((color & 0xFF) << 8)) >> 14;
     mixcolor.b = ((uint32)mixcolor.b * AccumBlurAmount + InvAccumBlurAmount * ((color & 0xFF00))) >> 14;
     mixcolor.c = ((uint32)mixcolor.c * AccumBlurAmount + InvAccumBlurAmount * ((color & 0xFF0000) >> 8)) >> 14;
     mixcolor.d = ((uint32)mixcolor.d * AccumBlurAmount + InvAccumBlurAmount * ((color & 0xFF000000) >> 16)) >> 14;
     AccumBlurBuf[y * bb_pitch + x] = mixcolor;

     pXBuf[(y + espec->DisplayRect.y) * surface->pitch32 + xs + x] = ((mixcolor.a >> 8) << 0) | ((mixcolor.b >> 8) << 8) |
        ((mixcolor.c >> 8) << 16) | ((mixcolor.d >> 8) << 24);
    }
   }

  }
 }
 else if(BlurBuf)
 {
  for(int y = 0; y < espec->DisplayRect.h; y++)
  {
   int xw = espec->DisplayRect.w;
   int xs = espec->DisplayRect.x;

   if(espec->LineWidths[0].w != ~0)
   {
    xw = espec->LineWidths[espec->DisplayRect.y + y].w;
    xs = espec->LineWidths[espec->DisplayRect.y + y].x;
   }

   for(int x = 0; x < xw; x++)
   {
    uint32 color, mixcolor;
    color = pXBuf[(y + espec->DisplayRect.y) * surface->pitch32 + xs + x];

    mixcolor = BlurBuf[y * bb_pitch + x];
    BlurBuf[y * bb_pitch + x] = color;

    // Needs 64-bit
    #ifdef HAVE_NATIVE64BIT
    color = ((((uint64)color + mixcolor) - ((color ^ mixcolor) & 0x01010101))) >> 1;
    #else
    color = ((((color & 0x00FF00FF) + (mixcolor & 0x00FF00FF)) >> 1) & 0x00FF00FF) | (((((color & 0xFF00FF00) >> 1) + ((mixcolor & 0xFF00FF00) >> 1))) & 0xFF00FF00);
    #endif

    //    color = (((color & 0xFF) + (mixcolor & 0xFF)) >> 1) | ((((color & 0xFF00) + (mixcolor & 0xFF00)) >> 1) & 0xFF00) |
    //       ((((color & 0xFF0000) + (mixcolor & 0xFF0000)) >> 1) & 0xFF0000) | ((((color >> 24) + (mixcolor >> 24)) >> 1) << 24);
    pXBuf[(y + espec->DisplayRect.y) * surface->pitch32 + xs + x] = color;
   }
  }
 }
}

void TBlur_Kill(void)
{
 if(BlurBuf)
 {
  free(BlurBuf);
  BlurBuf = NULL;
 }
 if(AccumBlurBuf)
 {
  free(AccumBlurBuf);
  AccumBlurBuf = NULL;
 }
}

bool TBlur_IsOn(void)
{
 return(BlurBuf || AccumBlurBuf);
}
