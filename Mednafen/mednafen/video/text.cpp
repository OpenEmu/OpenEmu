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

#include "video-common.h"
#include "../string/ConvertUTF.h"
#include "font-data.h"

typedef struct
{
        uint32 glyph_width;
        uint32 glyph_height;
        int extension;
} FontDescriptor_t;

static FontDescriptor_t FontDescriptors[_MDFN_FONT_COUNT] =
{
 #ifdef WANT_INTERNAL_CJK
 { 9, 18, MDFN_FONT_18x18 },
 { 5, 7, -1 },
 { 4, 5, -1 },
 { 6, 13, MDFN_FONT_12x13 },
 { 12, 13, -1 },
 { 18, 18, -1 },
 #else
 { 9, 18, -1 },
 { 5, 7, -1 },
 { 4, 5, -1 },
 { 6, 13, -1 },
 #endif
};

static const uint8 *FontDataCache[_MDFN_FONT_COUNT][65536];

void MDFN_InitFontData(void)
{
 unsigned int x;
 unsigned int inx;

 memset(FontDataCache, 0, sizeof(FontDataCache));

 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData4x5_Size / sizeof(font4x5)) && FontData4x5[inx].glyph_num == x)
  {
   FontDataCache[MDFN_FONT_4x5][x] = FontData4x5[inx].data;
   inx++;
  }
 }

 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData5x7_Size / sizeof(font5x7)) && FontData5x7[inx].glyph_num == x)
  {
   FontDataCache[MDFN_FONT_5x7][x] = FontData5x7[inx].data;
   inx++;
  }
 }

 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData6x13_Size / sizeof(font6x13)) && FontData6x13[inx].glyph_num == x)
  {
   FontDataCache[MDFN_FONT_6x13_12x13][x] = FontData6x13[inx].data;
   inx++;
  }
 }

 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData9x18_Size / sizeof(font9x18)) && FontData9x18[inx].glyph_num == x)
  {
   FontDataCache[MDFN_FONT_9x18_18x18][x] = FontData9x18[inx].data;
   inx++;
  }
 }

 #ifdef WANT_INTERNAL_CJK
 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData12x13_Size / sizeof(font12x13)) && FontData12x13[inx].glyph_num == x)
  {
   FontDataCache[MDFN_FONT_12x13][x] =  FontData12x13[inx].data;
   inx++;
  }
 }

 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData18x18_Size / sizeof(font18x18)) && FontData18x18[inx].glyph_num == x)
  {
   FontDataCache[MDFN_FONT_18x18][x] =  FontData18x18[inx].data;
   inx++;
  }
 }
 #endif
}

static long min(long a1, long a2)
{
 if(a1 < a2)
  return(a1);
 return(a2);
}

size_t utf32_strlen(UTF32 *s)
{
 size_t ret = 0;

 while(*s++) ret++;

 return(ret);
}

static void DrawTextSub(const UTF32 *utf32_buf, uint32 &slen, const uint8 **glyph_ptrs, uint8 *glyph_width, uint8 *glyph_ov_width, uint32 width, uint32 &pixwidth, uint32 which_font = 0)
{
 pixwidth = 0;

 for(uint32 x=0;x<slen;x++)
 {
  uint32 thisglyph = utf32_buf[x] & 0xFFFF;
  bool GlyphFound = FALSE;
  uint32 recurse_which_font = which_font;

  while(!GlyphFound)
  {
   if(FontDataCache[recurse_which_font][thisglyph])
   {
    glyph_ptrs[x] = FontDataCache[recurse_which_font][thisglyph];
    glyph_width[x] = FontDescriptors[recurse_which_font].glyph_width;
    GlyphFound = TRUE;
   }
   else if(FontDescriptors[recurse_which_font].extension != -1)
    recurse_which_font = FontDescriptors[recurse_which_font].extension;
   else
    break;
  }

  if(!GlyphFound)
  {
   glyph_ptrs[x] = FontDataCache[which_font][(unsigned char)'?'];
   glyph_width[x] = FontDescriptors[which_font].glyph_width;
  }

  pixwidth += glyph_width[x];

  if(thisglyph >= 0x0300 && thisglyph <= 0x036F)
  {
   if(x != (slen-1))
   {
    pixwidth -= min(glyph_width[x], glyph_width[x + 1]);
   }
   glyph_ov_width[x] = 0;
  }
  else
   glyph_ov_width[x] = glyph_width[x];

  if(pixwidth > width) // Oopsies, it's too big for the screen!  Just truncate it for now.
  {
   //printf("Oops: %d\n", x);
   slen = x;
   pixwidth -= glyph_width[x];
   break;
  }
 }
}

uint32 GetTextPixLength(const UTF8 *msg, uint32 which_font)
{
 uint32 slen;
 uint32 pixwidth;
 uint32 max_glyph_len = strlen((char *)msg);
 const uint8 *glyph_ptrs[max_glyph_len];
 uint8 glyph_width[max_glyph_len];
 uint8 glyph_ov_width[max_glyph_len];

 const UTF8 *src_begin = (UTF8 *)msg;
 UTF32 utf32_buf[max_glyph_len];
 UTF32 *tstart = utf32_buf;

 ConvertUTF8toUTF32(&src_begin, (UTF8*)msg + max_glyph_len, &tstart, &tstart[max_glyph_len], lenientConversion);
 slen = (tstart - utf32_buf);
 DrawTextSub(utf32_buf, slen, glyph_ptrs, glyph_width, glyph_ov_width, ~0, pixwidth, which_font);

 return(pixwidth);
}

uint32 GetTextPixLength(const UTF32 *msg, uint32 which_font)
{
 uint32 slen;
 uint32 pixwidth;
 uint32 max_glyph_len = utf32_strlen((UTF32 *)msg);
 const uint8 *glyph_ptrs[max_glyph_len];
 uint8 glyph_width[max_glyph_len];
 uint8 glyph_ov_width[max_glyph_len];

 slen = utf32_strlen((UTF32 *)msg);
 DrawTextSub((UTF32*)msg, slen, glyph_ptrs, glyph_width, glyph_ov_width, ~0, pixwidth, which_font);

 return(pixwidth);
}

static uint32 DoRealDraw(uint32 *dest, uint32 pitch, uint32 width, uint32 fgcolor, int centered, uint32 slen, const uint32 pixwidth,
		       uint32 glyph_height, const uint8 *glyph_ptrs[], const uint8 glyph_width[], const uint8 glyph_ov_width[])
{
 uint32 x;

 pitch /= sizeof(uint32);
 if(centered)
 {
  int32 poot = width - pixwidth;

  dest += poot / 2;
 }

 for(x=0;x<slen;x++)
 {
  unsigned int gx, gy;
  const uint8 *src_glyph = glyph_ptrs[x];
  unsigned int gy_mul = (glyph_width[x] >> 3) + 1;

  for(gy = 0; gy < glyph_height; gy++)
  {
   for(gx=0;gx<glyph_width[x];gx++)
   {
    if((src_glyph[gy * gy_mul + (gx >> 3)] << (gx & 0x7)) & 0x80)
     dest[gy * pitch + gx] = fgcolor;
   }
  }
  dest += glyph_ov_width[x];
 }

 return(pixwidth);
}

uint32 DrawTextTrans(uint32 *dest, int pitch, uint32 width, const UTF8 *msg, uint32 fgcolor, int centered, uint32 which_font)
{
 uint32 slen;
 uint32 pixwidth;
 uint32 max_glyph_len = strlen((char *)msg);
 const uint8 *glyph_ptrs[max_glyph_len];
 uint8 glyph_width[max_glyph_len];
 uint8 glyph_ov_width[max_glyph_len];

 const UTF8 *src_begin = (UTF8 *)msg;
 UTF32 utf32_buf[max_glyph_len];
 UTF32 *tstart = utf32_buf;

 ConvertUTF8toUTF32(&src_begin, (UTF8*)msg + max_glyph_len, &tstart, &tstart[max_glyph_len], lenientConversion);
 slen = (tstart - utf32_buf);
 DrawTextSub(utf32_buf, slen, glyph_ptrs, glyph_width, glyph_ov_width, width, pixwidth, which_font);

 return(DoRealDraw(dest, pitch, width, fgcolor, centered, slen, pixwidth, FontDescriptors[which_font].glyph_height, glyph_ptrs, glyph_width, glyph_ov_width));
}

uint32 DrawTextTrans(uint32 *dest, int pitch, uint32 width, const UTF32 *msg, uint32 fgcolor, int centered, uint32 which_font)
{
 uint32 slen;
 uint32 pixwidth;
 uint32 max_glyph_len = utf32_strlen((UTF32 *)msg);
 const uint8 *glyph_ptrs[max_glyph_len];
 uint8 glyph_width[max_glyph_len];
 uint8 glyph_ov_width[max_glyph_len];

 slen = utf32_strlen((UTF32 *)msg);
 DrawTextSub((UTF32*)msg, slen, glyph_ptrs, glyph_width, glyph_ov_width, ~0, pixwidth, which_font);

 return(DoRealDraw(dest, pitch, width, fgcolor, centered, slen, pixwidth, FontDescriptors[which_font].glyph_height, glyph_ptrs, glyph_width, glyph_ov_width));
}

uint32 DrawTextTransShadow(uint32 *dest, int pitch, uint32 width, const UTF8 *textmsg, uint32 fgcolor, uint32 shadcolor, int centered, uint32 which_font)
{
 DrawTextTrans(dest + 1 + (pitch >> 2), pitch, width, textmsg, shadcolor, centered, which_font);
 return(DrawTextTrans(dest, pitch, width, textmsg, fgcolor, centered, which_font));
}
