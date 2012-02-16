/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
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

/*  Contains file I/O functions that write/read data    */
/*  LSB first.              */

#include <stdio.h>
#include "types.h"
#include "myendian.h"

#ifdef __SNC__
#include <ppu_intrinsics.h>
#endif

void FlipByteOrder(uint8 *src, uint32 count)
{
	uint8 *start=src;
	uint8 *end=src+count-1;

	if((count&1) || !count)  return;   /* This shouldn't happen. */

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

/*/writes a little endian 16bit value to the specified file*/
int write16le(uint16 b, FILE *fp)
{
	uint8 s[2];
	s[0]=(uint8)b;
	s[1]=(uint8)(b>>8);
	return((fwrite(s,1,2,fp)<2)?0:2);
}

/*/writes a little endian 32bit value to the specified file*/
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
#elif __SNC__
	*(uint32*)Bufo = __builtin_lwbrx(&buf, 0);
#else
	*(uint32*)Bufo=((buf&0xFF)<<24)|((buf&0xFF00)<<8)|((buf&0xFF0000)>>8)|((buf&0xFF000000)>>24);
#endif
	return 1;
}

#ifdef __LIBSNES__
int write32le_mem(uint32 b, memstream_t *mem)
{
 uint8 s[4];
 s[0]=b;
 s[1]=b>>8;
 s[2]=b>>16;
 s[3]=b>>24;
 return((memstream_write(mem, s, 4)<4)?0:4);
}

int read32le_mem(uint32 *Bufo, memstream_t *mem)
{
 uint32 buf;
 if(memstream_read(mem, &buf, 4)<4)
  return 0;
 #ifdef LSB_FIRST
 *(uint32*)Bufo=buf;
 #else
 *(uint32*)Bufo=((buf&0xFF)<<24)|((buf&0xFF00)<<8)|((buf&0xFF0000)>>8)|((buf&0xFF000000)>>24);
 #endif
 return 1;
}
#endif


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

void FCEU_en32lsb(uint8 *buf, uint32 morp)
{
	buf[0]=morp;
	buf[1]=morp>>8;
	buf[2]=morp>>16;
	buf[3]=morp>>24;
}

/*/unpacks a 32bit little endian value from the provided byte array into host byte order*/
uint32 FCEU_de32lsb(uint8 *morp)
{
	return(morp[0]|(morp[1]<<8)|(morp[2]<<16)|(morp[3]<<24));
}

