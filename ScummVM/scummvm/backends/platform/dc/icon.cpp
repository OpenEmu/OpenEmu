/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <ronin/ronin.h>
#include <string.h>

#include "icon.h"

void Icon::create_vmicon(void *buffer)
{
  unsigned short *pal = (unsigned short *)buffer;
  unsigned char *pix = ((unsigned char *)buffer)+32;

  for (int n = 0; n<16; n++) {
    int p = palette[n];
    pal[n] =
      ((p>>16)&0xf000)|
      ((p>>12)&0x0f00)|
      ((p>> 8)&0x00f0)|
      ((p>> 4)&0x000f);
  }

  for (int line = 0; line < 32; line++) {
    memcpy(pix, &bitmap[32/2*(31-line)], 32/2);
    pix += 32/2;
  }
}

void Icon::create_texture()
{
  static char tt[16] = { 0, 1, 4, 5, 16, 17, 20, 21,
			 64, 65, 68, 69, 80, 81, 84, 85 };
  unsigned short *tex = (unsigned short *)ta_txalloc(512);
  unsigned short *linebase;
  unsigned char *src = bitmap+sizeof(bitmap)-17;
  for (int y=0; y<16; y++) {
    linebase = tex + (tt[y]<<1);
    for (int x=0; x<16; x++, --src)
      linebase[tt[x]] = src[16]|(src[0]<<8);
    src -= 16;
  }
  texture = tex;
}

void Icon::setPalette(int pal)
{
  unsigned int (*hwpal)[64][16] = (unsigned int (*)[64][16])0xa05f9000;
  for (int n = 0; n<16; n++)
    (*hwpal)[pal][n] = palette[n];
}

void Icon::draw(float x1, float y1, float x2, float y2, int pal,
		unsigned int argb)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|TA_CMD_POLYGON_TEXTURED;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_ENABLE_ALPHA|
    TA_POLYMODE2_TEXTURE_MODULATE_ALPHA|TA_POLYMODE2_U_SIZE_32|
    TA_POLYMODE2_V_SIZE_32;
  mypoly.texture = TA_TEXTUREMODE_CLUT4|TA_TEXTUREMODE_CLUTBANK4(pal)|
    TA_TEXTUREMODE_ADDRESS(texture);

  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.ocolour = 0;
  myvertex.colour = argb;
  myvertex.z = 0.5;
  myvertex.u = 0.0;
  myvertex.v = 1.0;

  myvertex.x = x1;
  myvertex.y = y1;
  ta_commit_list(&myvertex);

  myvertex.x = x2;
  myvertex.v = 0.0;
  ta_commit_list(&myvertex);

  myvertex.x = x1;
  myvertex.y = y2;
  myvertex.u = 1.0;
  myvertex.v = 1.0;
  ta_commit_list(&myvertex);

  myvertex.x = x2;
  myvertex.v = 0.0;
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_list(&myvertex);
}

int Icon::find_unused_pixel(const unsigned char *mask)
{
  int use[16];
  memset(use, 0, sizeof(use));
  unsigned char *p = bitmap;
  for (int n=0; n<32*32/2/4; n++) {
    unsigned char mbits = ~*mask++;
    for (int i=0; i<4; i++) {
      unsigned char pix = *p++;
      if(mbits & 64)
	use[pix&0xf]++;
      if(mbits & 128)
	use[pix>>4]++;
      mbits <<= 2;
    }
  }
  for (int i=0; i<16; i++)
    if (!use[i])
      return i;
  return -1;
}

bool Icon::load_image2(const void *data, int len)
{
  struct {
    int size, w, h;
    short pla, bitcnt;
    int comp, sizeimg, xres, yres, used, imp;
  } hdr;
  if (len < 40)
    return false;
  memcpy(&hdr, data, 40);
  if (hdr.size != 40 || /* hdr.sizeimg<=0 || */ hdr.w<0 || hdr.h<0 ||
     hdr.bitcnt<0 || hdr.used<0)
    return false;
  if (!hdr.used)
    hdr.used = 1<<hdr.bitcnt;
  hdr.h >>= 1;
  /* Fix incorrect sizeimg (The Dig) */
  if (hdr.sizeimg < ((hdr.w*hdr.h*(1+hdr.bitcnt)+7)>>3))
    hdr.sizeimg = ((hdr.w*hdr.h*(1+hdr.bitcnt)+7)>>3);
  if (hdr.size + (hdr.used<<2) + hdr.sizeimg > len /* ||
     hdr.sizeimg < ((hdr.w*hdr.h*(1+hdr.bitcnt)+7)>>3) */)
    return false;
  if (hdr.w != 32 || hdr.h != 32 || hdr.bitcnt != 4 || hdr.used > 16)
    return false;
  memcpy(palette, ((const char *)data)+hdr.size, hdr.used<<2);
  memcpy(bitmap, ((const char *)data)+hdr.size+(hdr.used<<2), 32*32/2);
  for (int i=0; i<16; i++)
    palette[i] |= 0xff000000;
  for (int i=hdr.used; i<16; i++)
    palette[i] = 0;
  const unsigned char *mask =
    ((const unsigned char *)data)+hdr.size+(hdr.used<<2)+32*32/2;
  int unused = find_unused_pixel(mask);
  if (unused >= 0) {
    unsigned char *pix = bitmap;
    for (int y=0; y<32; y++)
      for (int x=0; x<32/8; x++) {
	unsigned char mbits = *mask++;
	for (int z=0; z<4; z++) {
	  unsigned char pbits = *pix;
	  if (mbits & 64) pbits = (pbits & ~0xf) | unused;
	  if (mbits & 128) pbits = (pbits & 0xf) | (unused << 4);
	  *pix++ = pbits;
	  mbits <<= 2;
	}
      }
    palette[unused] = 0;
  }
  return true;
}

bool Icon::load_image1(const void *data, int len, int offs)
{
  struct {
    char w, h, colors, rsrv;
    short pla, bitcnt;
    int bytes, offs;
  } hdr;
  if (len < offs+16)
    return false;
  memcpy(&hdr, ((const char *)data)+offs, 16);
  if (hdr.bytes > 0 && hdr.offs >= 0 && hdr.offs+hdr.bytes <= len)
    return load_image2(((const char *)data)+hdr.offs, hdr.bytes);
  else
    return false;
}

bool Icon::load(const void *data, int len, int offs)
{
  struct { short rsrv, type, cnt; } hdr;
  memset(bitmap, 0, sizeof(bitmap));
  memset(palette, 0, sizeof(palette));
  texture = NULL;
  if (len < offs+6)
    return false;
  memcpy(&hdr, ((const char *)data)+offs, 6);
  if (hdr.type != 1 || hdr.cnt < 1 || offs+6+(hdr.cnt<<4) > len)
    return false;
  for (int i=0; i<hdr.cnt; i++)
    if (load_image1(data, len, offs+6+(i<<4)))
      return true;
  return false;
}

bool Icon::load(const char *filename)
{
  char buf[2048];
  int fd;
  if ((fd = open(filename, O_RDONLY))>=0) {
    int sz;
    sz = read(fd, buf, sizeof(buf));
    close(fd);
    if (sz>0)
      return load(buf, sz);
  }
  return false;
}
