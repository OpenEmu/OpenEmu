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
        uint8 glyph_width;
        uint8 glyph_height;
	int8 extension;
        uint8 entry_bsize;
	const uint8 *base_ptr;
} FontDescriptor_t;

static FontDescriptor_t FontDescriptors[_MDFN_FONT_COUNT] =
{
 #ifdef WANT_INTERNAL_CJK
 { 9, 18,	MDFN_FONT_18x18,	sizeof(FontData9x18[0]),	&FontData9x18[0].data[0] },
 { 5, 7, 	-1,			sizeof(FontData5x7[0]),		&FontData5x7[0].data[0] },
 { 4, 5, 	-1,			sizeof(FontData4x5[0]),		&FontData4x5[0].data[0] },
 { 6, 13,	MDFN_FONT_12x13,	sizeof(FontData6x13[0]),	&FontData6x13[0].data[0] },
 { 12, 13, 	-1,			sizeof(FontData12x13[0]),	&FontData12x13[0].data[0] },
 { 18, 18, 	-1,			sizeof(FontData18x18[0]),	&FontData18x18[0].data[0] },
 #else
 { 9, 18,	-1,			sizeof(FontData9x18[0]),	&FontData9x18[0].data[0] },
 { 5, 7, 	-1,			sizeof(FontData5x7[0]),		&FontData5x7[0].data[0] },
 { 4, 5,	-1,			sizeof(FontData4x5[0]),		&FontData4x5[0].data[0] },
 { 6, 13, 	-1,			sizeof(FontData6x13[0]),	&FontData6x13[0].data[0] },
 #endif
};

static uint16 FontDataIndexCache[_MDFN_FONT_COUNT][65536];

void MDFN_InitFontData(void)
{
 unsigned int x;
 unsigned int inx;

 memset(FontDataIndexCache, 0xFF, sizeof(FontDataIndexCache));

 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData4x5_Size / sizeof(font4x5)) && FontData4x5[inx].glyph_num == x)
  {
   FontDataIndexCache[MDFN_FONT_4x5][x] = inx;
   inx++;
  }
 }

 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData5x7_Size / sizeof(font5x7)) && FontData5x7[inx].glyph_num == x)
  {
   FontDataIndexCache[MDFN_FONT_5x7][x] = inx;
   inx++;
  }
 }

 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData6x13_Size / sizeof(font6x13)) && FontData6x13[inx].glyph_num == x)
  {
   FontDataIndexCache[MDFN_FONT_6x13_12x13][x] = inx;
   inx++;
  }
 }

 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData9x18_Size / sizeof(font9x18)) && FontData9x18[inx].glyph_num == x)
  {
   FontDataIndexCache[MDFN_FONT_9x18_18x18][x] = inx;
   inx++;
  }
 }

 #ifdef WANT_INTERNAL_CJK
 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData12x13_Size / sizeof(font12x13)) && FontData12x13[inx].glyph_num == x)
  {
   FontDataIndexCache[MDFN_FONT_12x13][x] = inx;
   inx++;
  }
 }

 for(inx=x=0;x<65536;x++)
 {
  if(inx < (FontData18x18_Size / sizeof(font18x18)) && FontData18x18[inx].glyph_num == x)
  {
   FontDataIndexCache[MDFN_FONT_18x18][x] = inx;
   inx++;
  }
 }
 #endif
}

size_t utf32_strlen(UTF32 *s)
{
 size_t ret = 0;

 while(*s++) ret++;

 return(ret);
}

static void DrawTextSub(const UTF32 *utf32_buf, uint32 &slen, const uint8 **glyph_ptrs, uint8 *glyph_width, uint8 *glyph_ov_width, uint32 &pixwidth, uint32 which_font)
{
 pixwidth = 0;

 for(uint32 x = 0; x < slen; x++)
 {
  uint32 thisglyph = utf32_buf[x] & 0xFFFF;
  bool GlyphFound = FALSE;
  uint32 recurse_which_font = which_font;

  while(!GlyphFound)
  {
   if(FontDataIndexCache[recurse_which_font][thisglyph] != 0xFFFF)
   {
    glyph_ptrs[x] = FontDescriptors[recurse_which_font].base_ptr + (FontDescriptors[recurse_which_font].entry_bsize * FontDataIndexCache[recurse_which_font][thisglyph]);
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
   glyph_ptrs[x] = FontDescriptors[which_font].base_ptr + (FontDescriptors[which_font].entry_bsize * FontDataIndexCache[which_font][(unsigned char)'?']);
   glyph_width[x] = FontDescriptors[which_font].glyph_width;
  }

  if((thisglyph >= 0x0300 && thisglyph <= 0x036F) || (thisglyph >= 0xFE20 && thisglyph <= 0xFE2F))
   glyph_ov_width[x] = 0;
  else
   glyph_ov_width[x] = glyph_width[x];

  pixwidth += (((x + 1) == slen) ? glyph_width[x] : glyph_ov_width[x]);
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
 DrawTextSub(utf32_buf, slen, glyph_ptrs, glyph_width, glyph_ov_width, pixwidth, which_font);

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
 DrawTextSub((UTF32*)msg, slen, glyph_ptrs, glyph_width, glyph_ov_width, pixwidth, which_font);

 return(pixwidth);
}

static uint32 DoRealDraw(uint32 *dest, uint32 pitch, uint32 width_limit, uint32 fgcolor, int centered, uint32 slen, uint32 pixwidth,
		       uint32 glyph_height, const uint8 *glyph_ptrs[], const uint8 glyph_width[], const uint8 glyph_ov_width[], const uint32 ex_offset = 0)
{
 pitch /= sizeof(uint32);
 if(centered)
 {
  int32 poot = width_limit - pixwidth;

  if(poot < 0)
   poot = 0;

  dest += poot / 2;
 }

#if 0
 // TODO to prevent writing past width
 // Shadow support kludge
 //
 if(width_limit < ex_offset)
  return(0);
 dest += ex_offset + pitch;
 width_limit -= ex_offset;
 //
 //
#endif

 pixwidth = 0;

 for(uint32 n = 0; n < slen; n++)
 {
  const uint8 *src_glyph = glyph_ptrs[n];
  uint32 gy_mul = (glyph_width[n] >> 3) + 1;
  uint32 gw = glyph_width[n];

  if((pixwidth + gw) > width_limit)
  {
   if(pixwidth > width_limit)	// Prooooobably shouldn't happen, but just in case.
    gw = 0;
   else
    gw = width_limit - pixwidth;
  }

  if((pixwidth + glyph_ov_width[n]) > width_limit)
   slen = n + 1;       // Break out


  for(uint32 gy = 0; gy < glyph_height; gy++)
  {
   for(uint32 gx = 0; gx < gw; gx++)
   {
    if((src_glyph[gy * gy_mul + (gx >> 3)] << (gx & 0x7)) & 0x80)
     dest[gy * pitch + gx] = fgcolor;
   }
  }
  dest += glyph_ov_width[n];
  pixwidth += ((n + 1) == slen) ? gw : glyph_ov_width[n];
 }

 if(pixwidth > width_limit)
  pixwidth = width_limit;

 return(pixwidth);
}

// FIXME/TODO: maximum text pixel width should be signed, not unsigned.
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
 DrawTextSub(utf32_buf, slen, glyph_ptrs, glyph_width, glyph_ov_width, pixwidth, which_font);

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
 DrawTextSub((UTF32*)msg, slen, glyph_ptrs, glyph_width, glyph_ov_width, pixwidth, which_font);

 return(DoRealDraw(dest, pitch, width, fgcolor, centered, slen, pixwidth, FontDescriptors[which_font].glyph_height, glyph_ptrs, glyph_width, glyph_ov_width));
}

uint32 DrawTextTransShadow(uint32 *dest, int pitch, uint32 width, const UTF8 *textmsg, uint32 fgcolor, uint32 shadcolor, int centered, uint32 which_font)
{
 DrawTextTrans(dest + 1 + (pitch >> 2), pitch, width, textmsg, shadcolor, centered, which_font);
 return(DrawTextTrans(dest, pitch, width, textmsg, fgcolor, centered, which_font));
}

uint32 DrawTextTransShadow(uint32 *dest, int pitch, uint32 width, const std::string &textmsg, uint32 fgcolor, uint32 shadcolor, int centered, uint32 which_font)
{
 const char *tmp = textmsg.c_str();

 DrawTextTrans(dest + 1 + (pitch >> 2), pitch, width, (const UTF8 *)tmp, shadcolor, centered, which_font);
 return(DrawTextTrans(dest, pitch, width, (const UTF8 *)tmp, fgcolor, centered, which_font));
}

#if 0
uint32 DrawText(MDFN_Surface *surface, const MDFN_Rect &rect, const char *textmsg, uint32 color,
		bool centered, uint32 which_font)
{
 MDFN_Rect tr = rect;

 if(tr.w < 0 || tr.h < 0)
  return;

 if((tr.x + tr.w) > surface->w)
  tr.w = surface->w - tr.x;

 if((tr.y + tr.h) > surface->h)
  tr.h = surface->h - tr.y;

}
#endif
