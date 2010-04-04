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

typedef struct
{
 uint16 a, b, c, d;
} HQPixelEntry;

static uint32 *BlurBuf = NULL;
static uint32 AccumBlurAmount; // max of 16384, infinite blur!
static HQPixelEntry *AccumBlurBuf = NULL;

void VBlur_Init(void)
{
        std::string sn = MDFNGameInfo->shortname;

        if(MDFN_GetSettingB(std::string(sn + "." + std::string("vblur")).c_str()))
        {
         AccumBlurAmount = (uint32)(16384 * MDFN_GetSettingF(std::string(sn + "." + std::string("vblur.accum.amount")).c_str()) / 100);
         if(MDFN_GetSettingB(std::string(sn + "." + std::string("vblur.accum")).c_str()))
         {
          AccumBlurBuf = (HQPixelEntry *)calloc(sizeof(HQPixelEntry), (MDFNGameInfo->pitch >> 2) * 256);
          MDFN_printf(_("Video frame blur enabled with accumulation: %f.\n"), (double)AccumBlurAmount * 100 / 16384);
         }
         else
         {
          BlurBuf = (uint32 *)calloc(4, (MDFNGameInfo->pitch >> 2) * 256);
          MDFN_printf(_("Video frame blur enabled.\n"));
         }
        }
}

void VBlur_Run(EmulateSpecStruct *espec)
{
 if(AccumBlurBuf)
 {
  uint32 *pXBuf = espec->pixels;
  uint32 pitchp = MDFNGameInfo->pitch >> 2;
  for(int y = 0; y < MDFNGameInfo->DisplayRect.h; y++)
  {
   int xw = MDFNGameInfo->DisplayRect.w;
   int xs = MDFNGameInfo->DisplayRect.x;

   if(espec->LineWidths[0].w != ~0)
    xw = espec->LineWidths[MDFNGameInfo->DisplayRect.y + y].w;

   if(AccumBlurAmount == 8192)
   {
    for(int x = 0; x < xw; x++)
    {
     uint32 color;
     HQPixelEntry mixcolor;
     color = pXBuf[(y + MDFNGameInfo->DisplayRect.y) * pitchp + xs + x];
     mixcolor = AccumBlurBuf[y * pitchp + x];
     mixcolor.a = ((uint32)mixcolor.a + ((color & 0xFF) << 8)) >> 1;
     mixcolor.b = ((uint32)mixcolor.b + ((color & 0xFF00))) >> 1;
     mixcolor.c = ((uint32)mixcolor.c + ((color & 0xFF0000) >> 8)) >> 1;
     mixcolor.d = ((uint32)mixcolor.d + ((color & 0xFF000000) >> 16)) >> 1;
    AccumBlurBuf[y * pitchp + x] = mixcolor;
    pXBuf[(y + MDFNGameInfo->DisplayRect.y) * pitchp + xs + x] = ((mixcolor.a >> 8) << 0) | ((mixcolor.b >> 8) << 8) |
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
     color = pXBuf[(y + MDFNGameInfo->DisplayRect.y) * pitchp + xs + x];
     mixcolor = AccumBlurBuf[y * pitchp + x];
     mixcolor.a = ((uint32)mixcolor.a * AccumBlurAmount + InvAccumBlurAmount * ((color & 0xFF) << 8)) >> 14;
     mixcolor.b = ((uint32)mixcolor.b * AccumBlurAmount + InvAccumBlurAmount * ((color & 0xFF00))) >> 14;
     mixcolor.c = ((uint32)mixcolor.c * AccumBlurAmount + InvAccumBlurAmount * ((color & 0xFF0000) >> 8)) >> 14;
     mixcolor.d = ((uint32)mixcolor.d * AccumBlurAmount + InvAccumBlurAmount * ((color & 0xFF000000) >> 16)) >> 14;
    AccumBlurBuf[y * pitchp + x] = mixcolor;
    pXBuf[(y + MDFNGameInfo->DisplayRect.y) * pitchp + xs + x] = ((mixcolor.a >> 8) << 0) | ((mixcolor.b >> 8) << 8) |
        ((mixcolor.c >> 8) << 16) | ((mixcolor.d >> 8) << 24);
    }
   }

  }
 }
 else if(BlurBuf)
 {
  uint32 *pXBuf = espec->pixels;
  uint32 pitchp = MDFNGameInfo->pitch >> 2;
  for(int y = 0; y < MDFNGameInfo->DisplayRect.h; y++)
  {
   int xw = MDFNGameInfo->DisplayRect.w;
   int xs = MDFNGameInfo->DisplayRect.x;

   if(espec->LineWidths[0].w != ~0)
    xw = espec->LineWidths[MDFNGameInfo->DisplayRect.y + y].w;

   for(int x = 0; x < xw; x++)
   {
    uint32 color, mixcolor;
    color = pXBuf[(y + MDFNGameInfo->DisplayRect.y) * pitchp + xs + x];
    mixcolor = BlurBuf[y * pitchp + x];
    BlurBuf[y * pitchp + x] = color;
    color = (((color & 0xFF) + (mixcolor & 0xFF)) >> 1) | ((((color & 0xFF00) + (mixcolor & 0xFF00)) >> 1) & 0xFF00) |
       ((((color & 0xFF0000) + (mixcolor & 0xFF0000)) >> 1) & 0xFF0000) | ((((color >> 24) + (mixcolor >> 24)) >> 1) << 24);
    pXBuf[(y + MDFNGameInfo->DisplayRect.y) * pitchp + xs + x] = color;
   }
  }
 }
}

void VBlur_Kill(void)
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

bool VBlur_IsOn(void)
{
 return(BlurBuf || AccumBlurBuf);
}
