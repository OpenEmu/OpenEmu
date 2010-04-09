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

static int WritePNGChunk(FILE *fp, uint32 size, const char *type, const uint8 *data)
{
 uint32 crc;

 uint8 tempo[4];

 tempo[0]=size>>24;
 tempo[1]=size>>16;
 tempo[2]=size>>8;
 tempo[3]=size;

 if(fwrite(tempo,4,1,fp)!=1)
  return 0;
 if(fwrite(type,4,1,fp)!=1)
  return 0;

 if(size)
  if(fwrite(data,1,size,fp)!=size)
   return 0;

 crc = crc32(0,(uint8 *)type,4);
 if(size)
  crc = crc32(crc,data,size);

 tempo[0]=crc>>24;
 tempo[1]=crc>>16;
 tempo[2]=crc>>8;
 tempo[3]=crc;

 if(fwrite(tempo,4,1,fp)!=1)
  return 0;
 return 1;
}

int MDFN_SavePNGSnapshot(const char *fname, uint32 *fb, const MDFN_Rect *rect, uint32 pitch)
{
 int x, y;
 FILE *pp=NULL;
 uint8 *compmem = NULL;
 uLongf compmemsize = (uLongf)( (rect->h * (rect->w + 1) * 3 * 1.001 + 1) + 12 );

 if(!(compmem=(uint8 *)MDFN_malloc(compmemsize, _("PNG compression buffer"))))
  return 0;

 if(!(pp=fopen(fname, "wb")))
 {
  return 0;
 }
 {
  static uint8 header[8]={137,80,78,71,13,10,26,10};
  if(fwrite(header,8,1,pp)!=1)
   goto PNGerr;
 }

 {
  uint8 chunko[13];

  chunko[0] = rect->w >> 24;		// Width
  chunko[1] = rect->w >> 16;
  chunko[2] = rect->w >> 8;
  chunko[3] = rect->w;

  chunko[4] = rect->h >> 24;		// Height
  chunko[5] = rect->h >> 16;
  chunko[6] = rect->h >> 8;
  chunko[7] = rect->h;

  chunko[8]=8;				// 8 bits per sample(24 bits per pixel)
  chunko[9]=2;				// Color type; RGB triplet
  chunko[10]=0;				// compression: deflate
  chunko[11]=0;				// Basic adapative filter set(though none are used).
  chunko[12]=0;				// No interlace.

  if(!WritePNGChunk(pp,13,"IHDR",chunko))
   goto PNGerr;
 }

 {
  uint8 *tmp_buffer;
  uint8 *tmp_inc;
  tmp_inc = tmp_buffer = (uint8 *)malloc((rect->w * 3 + 1) * rect->h);

  for(y=0;y<rect->h;y++)
  {
   *tmp_inc = 0;
   tmp_inc++;
   for(x=0;x<rect->w;x++)
   {
    int r,g,b;
    DECOMP_COLOR(*(uint32 *)((uint8 *)fb + (y + rect->y) * pitch + (x + rect->x) * 4), r, g, b);
    tmp_inc[0] = r;
    tmp_inc[1] = g;
    tmp_inc[2] = b;
    tmp_inc += 3;
   }
  }

  if(compress(compmem, &compmemsize, tmp_buffer, rect->h * (rect->w * 3 + 1))!=Z_OK)
  {
   if(tmp_buffer) free(tmp_buffer);
   goto PNGerr;
  }
  if(tmp_buffer) free(tmp_buffer);
  if(!WritePNGChunk(pp,compmemsize,"IDAT",compmem))
   goto PNGerr;
 }
 if(!WritePNGChunk(pp,0,"IEND",0))
  goto PNGerr;

 free(compmem);
 fclose(pp);

 return 1;

 PNGerr:
 if(compmem)
  free(compmem);
 if(pp)
  fclose(pp);
 return(0);
}
