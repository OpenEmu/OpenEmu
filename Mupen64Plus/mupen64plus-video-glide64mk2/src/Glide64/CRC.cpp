/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2002  Dave2001
* Copyright (c) 2003-2009  Sergey 'Gonetz' Lipski
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
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

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators
// Project started on December 29th, 2001
//
// Authors:
// Dave2001, original author, founded the project in 2001, left it in 2002
// Gugaman, joined the project in 2002, left it in 2002
// Sergey 'Gonetz' Lipski, joined the project in 2002, main author since fall of 2002
// Hiroshi 'KoolSmoky' Morii, joined the project in 2007
//
//****************************************************************
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
//****************************************************************
//
// CRC32 calculation functions 
//
// Created by Gonetz, 2004
//
//****************************************************************
//*
#define CRC32_POLYNOMIAL     0x04C11DB7

unsigned int CRCTable[ 256 ];

unsigned int Reflect( unsigned int ref, char ch )
{
     unsigned int value = 0;

     // Swap bit 0 for bit 7
     // bit 1 for bit 6, etc.
     for (char i = 1; i < (ch + 1); i++)
     {
          if(ref & 1)
               value |= 1 << (ch - i);
          ref >>= 1;
     }
     return value;
}

void CRC_BuildTable()
{
    unsigned int crc;

    for (unsigned i = 0; i <= 255; i++)
	{
        crc = Reflect( i, 8 ) << 24;
        for (unsigned j = 0; j < 8; j++)
			crc = (crc << 1) ^ (crc & (1 << 31) ? CRC32_POLYNOMIAL : 0);
        
        CRCTable[i] = Reflect( crc, 32 );
    }
}
//*/
//*
unsigned int CRC32( unsigned int crc, void *buffer, unsigned int count )
{
  unsigned int orig = crc;
  unsigned char * p = reinterpret_cast<unsigned char*>(buffer);
  while (count--)
    crc = (crc >> 8) ^ CRCTable[(crc & 0xFF) ^ *p++];
  return crc ^ orig;
}
//*/

/*
wxUint32 CRC_Calculate( wxUint32 crc, void *buffer, wxUint32 count )
{
  wxUint32 Crc32=crc;
  __asm {
			mov	esi, buffer
			mov	ecx, count
			mov	edx, crc
			xor eax, eax

loop1:
			inc	esi
			mov	al, dl
			xor	al, byte ptr [esi]
			shr	edx, 8
			mov	ebx, [CRCTable+eax*4]
			xor	edx, ebx

			loop loop1

			xor	Crc32, edx
   }
   return Crc32;
}
*/

/*
unsigned int CRC_Calculate( unsigned int crc, void *buffer, unsigned int count )
{
  unsigned int Crc32=crc;
  __asm {
			mov	esi, buffer
			mov	edx, count
			add	edx, esi
			mov	ecx, crc

loop1:
			mov	bl, byte ptr [esi]
			movzx	eax, cl
			inc	esi
			xor	al, bl
			shr	ecx, 8
			mov	ebx, [CRCTable+eax*4]
			xor	ecx, ebx

			cmp	edx, esi
			jne	loop1

			xor	Crc32, ecx
   }
   return Crc32;
}
//*/
