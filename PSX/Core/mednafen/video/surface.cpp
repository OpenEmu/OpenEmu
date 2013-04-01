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

#include <mednafen/mednafen.h>
#include "surface.h"

MDFN_PixelFormat::MDFN_PixelFormat()
{
 bpp = 0;
 colorspace = 0;

 Rshift = 0;
 Gshift = 0;
 Bshift = 0;
 Ashift = 0;

 Rprec = 0;
 Gprec = 0;
 Bprec = 0;
 Aprec = 0;
}

MDFN_PixelFormat::MDFN_PixelFormat(const unsigned int p_colorspace, const uint8 p_rs, const uint8 p_gs, const uint8 p_bs, const uint8 p_as)
{
 bpp = 32;
 colorspace = p_colorspace;

 Rshift = p_rs;
 Gshift = p_gs;
 Bshift = p_bs;
 Ashift = p_as;

 Rprec = 8;
 Gprec = 8;
 Bprec = 8;
 Aprec = 8;
}

MDFN_Surface::MDFN_Surface()
{
 memset(&format, 0, sizeof(format));

 pixels = NULL;
 pixels8 = NULL;
 pixels16 = NULL;
 palette = NULL;
 pixels_is_external = false;
 pitchinpix = 0;
 w = 0;
 h = 0;
}

MDFN_Surface::MDFN_Surface(void *const p_pixels, const uint32 p_width, const uint32 p_height, const uint32 p_pitchinpix, const MDFN_PixelFormat &nf)
{
 Init(p_pixels, p_width, p_height, p_pitchinpix, nf);
}

#if 0
void MDFN_Surface::Resize(const uint32 p_width, const uint32 p_height, const uint32 p_pitchinpix)
{
 void *ptr = (format.bpp == 16) ? pixels16 : pixels;
 uint64 old_asize = ((uint64)pitchinpix * (format.bpp >> 3)) * h;
 uint64 new_asize = ((uint64)p_pitchinpix * (format.bpp >> 3)) * p_height;

 if(!(ptr = realloc(ptr, new_asize)))
  throw MDFN_Error(ErrnoHolder(errno));

 if(new_asize > old_asize)
  memset((uint8*)ptr + old_asize, 0x00, new_asize - old_asize);

 if(format.bpp == 16)
  pixels16 = (uint16*)ptr;
 else
  pixels = (uint32*)ptr;

 pitchinpix = p_pitchinpix;
 w = p_width;
 h = p_height;
}
#endif
void MDFN_Surface::Init(void *const p_pixels, const uint32 p_width, const uint32 p_height, const uint32 p_pitchinpix, const MDFN_PixelFormat &nf)
{
 void *rpix = NULL;
 assert(nf.bpp == 8 || nf.bpp == 16 || nf.bpp == 32);

 format = nf;

 if(nf.bpp == 8)
 {
  //assert(!nf.Rshift && !nf.Gshift && !nf.Bshift && !nf.Ashift);
  //assert(!nf.Rprec && !nf.Gprec && !nf.Bprec && !nf.Aprec);
 }
 else if(nf.bpp == 16)
 {
  assert(nf.Rprec && nf.Gprec && nf.Bprec && nf.Aprec);
 }
 else
 {
  assert((nf.Rshift + nf.Gshift + nf.Bshift + nf.Ashift) == 48);
  assert(!((nf.Rshift | nf.Gshift | nf.Bshift | nf.Ashift) & 0x7));

  format.Rprec = 8;
  format.Gprec = 8;
  format.Bprec = 8;
  format.Aprec = 8;
 }

 pixels16 = NULL;
 pixels8 = NULL;
 pixels = NULL;
 palette = NULL;

 pixels_is_external = false;

 if(p_pixels)
 {
  rpix = p_pixels;
  pixels_is_external = true;
 }
 else
 {
  if(!(rpix = calloc(1, p_pitchinpix * p_height * (nf.bpp / 8))))
  {
   ErrnoHolder ene(errno);

   throw(MDFN_Error(ene.Errno(), "%s", ene.StrError()));
  }
 }

 if(nf.bpp == 8)
 {
  if(!(palette = (MDFN_PaletteEntry*) calloc(sizeof(MDFN_PaletteEntry), 256)))
  {
   ErrnoHolder ene(errno);

   if(!pixels_is_external)
    free(rpix);

   throw(MDFN_Error(ene.Errno(), "%s", ene.StrError()));
  }
 }

 if(nf.bpp == 16)
  pixels16 = (uint16 *)rpix;
 else if(nf.bpp == 8)
  pixels8 = (uint8 *)rpix;
 else
  pixels = (uint32 *)rpix;

 w = p_width;
 h = p_height;

 pitchinpix = p_pitchinpix;
}

// When we're converting, only convert the w*h area(AKA leave the last part of the line, pitch32 - w, alone),
// for places where we store auxillary information there(graphics viewer in the debugger), and it'll be faster
// to boot.
void MDFN_Surface::SetFormat(const MDFN_PixelFormat &nf, bool convert)
{
 if(format.bpp != 32 || nf.bpp != 32)
  printf("%u->%u\n",format.bpp, nf.bpp);

 assert(format.bpp == 8 || format.bpp == 16 || format.bpp == 32);
 assert((nf.bpp == 8 && !convert) || nf.bpp == 16 || nf.bpp == 32);

 if(nf.bpp == 8)
 {

 }
 else if(nf.bpp == 16)
 {

 }
 else
 {
  assert((nf.Rshift + nf.Gshift + nf.Bshift + nf.Ashift) == 48);
  assert(!((nf.Rshift | nf.Gshift | nf.Bshift | nf.Ashift) & 0x7));
 }

 if(nf.bpp != format.bpp)
 {
  void *rpix = calloc(1, pitchinpix * h * (nf.bpp / 8));
  void *oldpix;

  if(nf.bpp == 8)
  {
   assert(!convert);

   pixels8 = (uint8 *)rpix;
   palette = (MDFN_PaletteEntry*)calloc(sizeof(MDFN_PaletteEntry), 256);
  }
  else if(nf.bpp == 16)	// 32bpp or 8bpp to 16bpp
  {
   pixels16 = (uint16 *)rpix;

   if(convert)
   {
    if(format.bpp == 8)
    {
     puts("8bpp to 16bpp convert");
     for(int y = 0; y < h; y++)
     {
      uint8 *srow = &pixels8[y * pitchinpix];
      uint16 *drow = &pixels16[y * pitchinpix];

      for(int x = 0; x < w; x++)
      {
       const MDFN_PaletteEntry &p = palette[srow[x]];

       drow[x] = nf.MakeColor(p.r, p.g, p.b, 0);
      }
     }
    }
    else
    {
     puts("32bpp to 16bpp convert");
     for(int y = 0; y < h; y++)
     {
      uint32 *srow = &pixels[y * pitchinpix];
      uint16 *drow = &pixels16[y * pitchinpix];

      for(int x = 0; x < w; x++)
      {
       uint32 c = srow[x];
       int r, g, b, a;

       DecodeColor(c, r, g, b, a);
       drow[x] = nf.MakeColor(r, g, b, a);
      }
     }
    }
   }
  }
  else			// 16bpp or 8bpp to 32bpp
  {
   pixels = (uint32 *)rpix;

   if(convert)
   {
    if(format.bpp == 8)
    {
     puts("8bpp to 32bpp convert");
     for(int y = 0; y < h; y++)
     {
      uint8 *srow = &pixels8[y * pitchinpix];
      uint32 *drow = &pixels[y * pitchinpix];

      for(int x = 0; x < w; x++)
      {
       const MDFN_PaletteEntry &p = palette[srow[x]];

       drow[x] = nf.MakeColor(p.r, p.g, p.b, 0);
      }
     }
    }
    else
    {
     puts("16bpp to 32bpp convert");
     for(int y = 0; y < h; y++)
     {
      uint16 *srow = &pixels16[y * pitchinpix];
      uint32 *drow = &pixels[y * pitchinpix];

      for(int x = 0; x < w; x++)
      {
       uint32 c = srow[x];
       int r, g, b, a;

       DecodeColor(c, r, g, b, a);
       drow[x] = nf.MakeColor(r, g, b, a);
      }
     }
    }
   }
  }

  switch(format.bpp)
  {
   default:

   case 32: oldpix = pixels;
	    pixels = NULL;
	    break;

   case 16: oldpix = pixels16;
	    pixels16 = NULL;
	    break;

   case 8:  oldpix = pixels8;
	    pixels8 = NULL;
	    if(palette)
	    {
	     free(palette);
	     palette = NULL;
	    }
	    break;
  }

  if(oldpix && !pixels_is_external)
   free(oldpix);

  pixels_is_external = false;

  // We already handled surface conversion above.
  convert = false;
 }

 if(convert)
 {
  if(format.bpp == 16)
  {
   // We should assert that surface->pixels is non-NULL even if we don't need to convert the surface, to catch more insidious bugs.
   assert(pixels16);

   if(memcmp(&format, &nf, sizeof(MDFN_PixelFormat)))
   {
    //puts("Converting");
    for(int y = 0; y < h; y++)
    {
     uint16 *row = &pixels16[y * pitchinpix];

     for(int x = 0; x < w; x++)
     {
      uint32 c = row[x];
      int r, g, b, a;

      DecodeColor(c, r, g, b, a);
      row[x] = nf.MakeColor(r, g, b, a);
     }
    }
   }
  }
  else
  {
   // We should assert that surface->pixels is non-NULL even if we don't need to convert the surface, to catch more insidious bugs.
   assert(pixels);

   if(memcmp(&format, &nf, sizeof(MDFN_PixelFormat)))
   {
    //puts("Converting");
    for(int y = 0; y < h; y++)
    {
     uint32 *row = &pixels[y * pitchinpix];

     for(int x = 0; x < w; x++)
     {
      uint32 c = row[x];
      int r, g, b, a;

      DecodeColor(c, r, g, b, a);
      row[x] = nf.MakeColor(r, g, b, a);
     }
    }
   }
  }
 }
 format = nf;
}

void MDFN_Surface::Fill(uint8 r, uint8 g, uint8 b, uint8 a)
{
 uint32 color = MakeColor(r, g, b, a);

 if(format.bpp == 8)
 {
  assert(pixels8);

  for(int32 i = 0; i < pitchinpix * h; i++)
   pixels8[i] = color;
 }
 else if(format.bpp == 16)
 {
  assert(pixels16);

  for(int32 i = 0; i < pitchinpix * h; i++)
   pixels16[i] = color;
 }
 else
 {
  assert(pixels);

  for(int32 i = 0; i < pitchinpix * h; i++)
   pixels[i] = color;
 }
}

MDFN_Surface::~MDFN_Surface()
{
 if(!pixels_is_external)
 {
  if(pixels)
   free(pixels);
  if(pixels16)
   free(pixels16);
  if(pixels8)
   free(pixels8);
  if(palette)
   free(palette);
 }
}

