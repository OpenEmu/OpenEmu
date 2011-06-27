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

#include "mednafen.h"
#include "endian.h"

void Endian_A16_Swap(void *src, uint32 nelements)
{
 uint32 i;
 uint8 *nsrc = (uint8 *)src;

 for(i = 0; i < nelements; i++)
 {
  uint8 tmp = nsrc[i * 2];

  nsrc[i * 2] = nsrc[i * 2 + 1];
  nsrc[i * 2 + 1] = tmp;
 }
}

void Endian_A32_Swap(void *src, uint32 nelements)
{
 uint32 i;
 uint8 *nsrc = (uint8 *)src;

 for(i = 0; i < nelements; i++)
 {
  uint8 tmp1 = nsrc[i * 4];
  uint8 tmp2 = nsrc[i * 4 + 1];

  nsrc[i * 4] = nsrc[i * 4 + 3];
  nsrc[i * 4 + 1] = nsrc[i * 4 + 2];

  nsrc[i * 4 + 2] = tmp2;
  nsrc[i * 4 + 3] = tmp1;
 }
}

void Endian_A64_Swap(void *src, uint32 nelements)
{
 uint32 i;
 uint8 *nsrc = (uint8 *)src;

 for(i = 0; i < nelements; i++)
 {
  uint8 *base = &nsrc[i * 8];

  for(int z = 0; z < 4; z++)
  {
   uint8 tmp = base[z];

   base[z] = base[7 - z];
   base[7 - z] = tmp;
  }
 }
}

void Endian_A16_NE_to_LE(void *src, uint32 nelements)
{
 #ifdef MSB_FIRST
 Endian_A16_Swap(src, nelements);
 #endif
}

void Endian_A32_NE_to_LE(void *src, uint32 nelements)
{
 #ifdef MSB_FIRST
 Endian_A32_Swap(src, nelements);
 #endif
}

void Endian_A64_NE_to_LE(void *src, uint32 nelements)
{
 #ifdef MSB_FIRST
 Endian_A64_Swap(src, nelements);
 #endif
}


void Endian_A16_LE_to_NE(void *src, uint32 nelements)
{
 #ifdef MSB_FIRST
 uint32 i;
 uint8 *nsrc = (uint8 *)src;

 for(i = 0; i < nelements; i++)
 {
  uint8 tmp = nsrc[i * 2];

  nsrc[i * 2] = nsrc[i * 2 + 1];
  nsrc[i * 2 + 1] = tmp;
 }
 #endif
}

void Endian_A16_BE_to_NE(void *src, uint32 nelements)
{
 #ifdef LSB_FIRST
 uint32 i;
 uint8 *nsrc = (uint8 *)src;

 for(i = 0; i < nelements; i++)
 {
  uint8 tmp = nsrc[i * 2];

  nsrc[i * 2] = nsrc[i * 2 + 1];
  nsrc[i * 2 + 1] = tmp;
 }
 #endif
}


void Endian_A32_LE_to_NE(void *src, uint32 nelements)
{
 #ifdef MSB_FIRST
 uint32 i;
 uint8 *nsrc = (uint8 *)src;

 for(i = 0; i < nelements; i++)
 {
  uint8 tmp1 = nsrc[i * 4];
  uint8 tmp2 = nsrc[i * 4 + 1];

  nsrc[i * 4] = nsrc[i * 4 + 3];
  nsrc[i * 4 + 1] = nsrc[i * 4 + 2];

  nsrc[i * 4 + 2] = tmp2;
  nsrc[i * 4 + 3] = tmp1;
 }
 #endif
}

void Endian_A64_LE_to_NE(void *src, uint32 nelements)
{
 #ifdef MSB_FIRST
 uint32 i;
 uint8 *nsrc = (uint8 *)src;

 for(i = 0; i < nelements; i++)
 {
  uint8 *base = &nsrc[i * 8];

  for(int z = 0; z < 4; z++)
  {
   uint8 tmp = base[z];

   base[z] = base[7 - z];
   base[7 - z] = tmp;
  }
 }
 #endif
}

void FlipByteOrder(uint8 *src, uint32 count)
{
 uint8 *start=src;
 uint8 *end=src+count-1;

 if((count&1) || !count)        return;         /* This shouldn't happen. */

 count >>= 1;

 while(count--)
 {
  uint8 tmp;

  tmp=*end;
  *end=*start;
  *start=tmp;
  end--;
  start++;
 }
}

void Endian_V_LE_to_NE(void *src, uint32 bytesize)
{
 #ifdef MSB_FIRST
 FlipByteOrder((uint8 *)src, bytesize);
 #endif
}

void Endian_V_NE_to_LE(void *src, uint32 bytesize)
{
 #ifdef MSB_FIRST
 FlipByteOrder((uint8 *)src, bytesize);
 #endif
}

int write16le(uint16 b, FILE *fp)
{
 uint8 s[2];
 s[0]=b;
 s[1]=b>>8;
 return((fwrite(s,1,2,fp)<2)?0:2);
}

int write32le(uint32 b, FILE *fp)
{
 uint8 s[4];
 s[0]=b;
 s[1]=b>>8;
 s[2]=b>>16;
 s[3]=b>>24;
 return((fwrite(s,1,4,fp)<4)?0:4);
}

int read32le(uint32 *Bufo, FILE *fp)
{
 uint32 buf;
 if(fread(&buf,1,4,fp)<4)
  return 0;
 #ifdef LSB_FIRST
 *(uint32*)Bufo=buf;
 #else
 *(uint32*)Bufo=((buf&0xFF)<<24)|((buf&0xFF00)<<8)|((buf&0xFF0000)>>8)|((buf&0xFF000000)>>24);
 #endif
 return 1;
}

int read16le(char *d, FILE *fp)
{
 #ifdef LSB_FIRST
 return((fread(d,1,2,fp)<2)?0:2);
 #else
 int ret;
 ret=fread(d+1,1,1,fp);
 ret+=fread(d,1,1,fp);
 return ret<2?0:2;
 #endif
}

