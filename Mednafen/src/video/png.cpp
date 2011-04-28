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

#include <zlib.h>
#include "png.h"
#include "../endian.h"

void PNGWrite::WriteChunk(FileWrapper &pngfile, uint32 size, const char *type, const uint8 *data)
{
 uint32 crc;
 uint8 tempo[4];

 MDFN_en32msb(tempo, size);

 pngfile.write(tempo, 4);
 pngfile.write(type, 4);

 if(size)
  pngfile.write(data, size);

 crc = crc32(0, (uint8 *)type, 4);
 if(size)
  crc = crc32(crc,data,size);

 MDFN_en32msb(tempo, crc);

 pngfile.write(tempo, 4);
}

// int MDFN_SavePNGSnapshot(const char *fname, const MDFN_Surface *src, const MDFN_Rect *rect, const MDFN_Rect *LineWidths)

PNGWrite::~PNGWrite()
{

}

PNGWrite::PNGWrite(const char *path, const MDFN_Surface *src, const MDFN_Rect &rect, const MDFN_Rect *LineWidths) : ownfile(path, FileWrapper::MODE_WRITE_SAFE)
{
 WriteIt(ownfile, src, rect, LineWidths);
}

#if 0
PNGWrite::PNGWrite(FileWrapper &pngfile, const MDFN_Surface *src, const MDFN_Rect &rect, const MDFN_Rect *LineWidths)
{
 WriteIt(pngfile, src, rect, LineWidths);
}
#endif

void PNGWrite::WriteIt(FileWrapper &pngfile, const MDFN_Surface *src, const MDFN_Rect &rect, const MDFN_Rect *LineWidths)

{
 uLongf compmemsize;
 int png_width;

 if(LineWidths[0].w != ~0)
 {
  png_width = 0;

  for(int y = 0; y < rect.h; y++)
  {
   if(LineWidths[rect.y + y].w > png_width)
    png_width = LineWidths[rect.y + y].w;
  }
 }
 else
  png_width = rect.w;

 if(!rect.h)
  throw(MDFN_Error(0, "Refusing to save a zero-height PNG."));

 if(!png_width)
  throw(MDFN_Error(0, "Refusing to save a zero-width PNG."));

 compmemsize = (uLongf)( (rect.h * (png_width + 1) * 3 * 1.001 + 1) + 12 );

 compmem.resize(compmemsize);

 {
  static uint8 header[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
  pngfile.write(header, 8);
 }

 {
  uint8 chunko[13];

  MDFN_en32msb(&chunko[0], png_width);	// Width

  MDFN_en32msb(&chunko[4], rect.h);	// Height

  chunko[8]=8;				// 8 bits per sample(24 bits per pixel)
  chunko[9]=2;				// Color type; RGB triplet
  chunko[10]=0;				// compression: deflate
  chunko[11]=0;				// Basic adapative filter set(though none are used).
  chunko[12]=0;				// No interlace.

  WriteChunk(pngfile, 13, "IHDR", chunko);
 }

 // pHYs chunk
 #if 0
 {
  uint8 chunko[9];
  uint32 ppx, ppy;

  //ppx = png_width / MDFNGameInfo->nominal_width;
  //ppy = 1;	//rect->h / rect->h

  ppx = png_width;
  ppy = MDFNGameInfo->nominal_width;

  MDFN_en32msb(&chunko[0], ppx);
  MDFN_en32msb(&chunko[4], ppy);

  //printf("%08x %08x, %04x %04x\n", ppx, ppy, *(uint32 *)&chunko[0], *(uint32 *)&chunko[4]);

  chunko[8] = 0;

  WriteChunk(pngfile, 9, "pHYs", chunko);
 }
 #endif

 {
  uint8 *tmp_inc;

  tmp_buffer.resize((png_width * 3 + 1) * rect.h);

  tmp_inc = &tmp_buffer[0];

  for(int y = 0; y < rect.h; y++)
  {
   *tmp_inc = 0;
   tmp_inc++;
   int line_width = rect.w;
   int x_base = rect.x;

   if(LineWidths[0].w != ~0)
   {
    line_width = LineWidths[y + rect.y].w;
    x_base = LineWidths[y + rect.y].x;
   }

   for(int x = 0; x < line_width; x++)
   {
    int r, g, b;

    if(src->format.bpp == 16)
     src->DecodeColor(src->pixels16[(y + rect.y) * src->pitchinpix + (x + x_base)], r, g, b);
    else
     src->DecodeColor(src->pixels[(y + rect.y) * src->pitchinpix + (x + x_base)], r, g, b);

    tmp_inc[0] = r;
    tmp_inc[1] = g;
    tmp_inc[2] = b;
    tmp_inc += 3;
   }

   for(int x = line_width; x < png_width; x++)
   {
    tmp_inc[0] = tmp_inc[1] = tmp_inc[2] = 0;
    tmp_inc += 3;
   }
  }

  if(compress(&compmem[0], &compmemsize, &tmp_buffer[0], rect.h * (png_width * 3 + 1)) != Z_OK)
  {
   throw(MDFN_Error(0, "zlib error"));	// TODO: verbosify
  }

  WriteChunk(pngfile, compmemsize, "IDAT", &compmem[0]);
 }

 WriteChunk(pngfile, 0, "IEND", 0);
}
