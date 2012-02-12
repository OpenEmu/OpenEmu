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

#include "mapinc.h"

static uint8 is2kbank, isnot2kbank, dipreg;

/*void Mapper83_init(void)
{             
 
} 
*/
static void m83IRQHook(int a)
{
  if(IRQa)
  {
   IRQCount-=a;
   if(IRQCount<0)
   {
    X6502_IRQBegin(FCEU_IQEXT);
    IRQa=0;
    IRQCount=0xFFFF;
   }
  }
}

static DECLFW(wrlow)
{
 mapbyte4[A&3]=V;
}

static DECLFR(rdlow)
{
 return mapbyte4[A&3];
}

static DECLFR(rd5000)
{
 return dipreg;
}

static void m83prg(void)
{
  ROM_BANK16(0x8000,(mapbyte1[0]&0x3F)); 
  ROM_BANK16(0xC000,(mapbyte1[0]&0x30)|0xF);
}

static void m83chr(void)
{
  if(is2kbank&&!isnot2kbank)
  {
    VROM_BANK2(0x0000,mapbyte2[0]);
    VROM_BANK2(0x0800,mapbyte2[1]);
    VROM_BANK2(0x1000,mapbyte2[6]);
    VROM_BANK2(0x1800,mapbyte2[7]);
  }
  else
  {
    int x;
    for(x=0;x<8;x++)
       VROM_BANK1(x*0x400,mapbyte2[x]|((mapbyte1[0]&0x30)<<4));
  }
}

static DECLFW(Mapper83_write)
{
 //printf("$%04x:$%02x\n",A,V);
 switch(A)
 {
  case 0x8000: is2kbank = 1;
  case 0xB000:
  case 0xB0FF:
  case 0xB1FF:
       {
        mapbyte1[0]=V;
        m83prg();
        m83chr();
       }
       break;
  case 0x8100:
              mapbyte3[0]=V&0x80;
              switch(V&0x3)
              {
              case 0x00:MIRROR_SET2(1);break;
              case 0x01:MIRROR_SET2(0);break;
              case 0x02:onemir(0);break;
              case 0x03:onemir(1);break;
              }
              break;
  case 0x8200:IRQCount&=0xFF00;IRQCount|=V;X6502_IRQEnd(FCEU_IQEXT);break;
  case 0x8201:IRQa=mapbyte3[0];IRQCount&=0xFF;IRQCount|=V<<8;break;
  case 0x8300:ROM_BANK8(0x8000,V);break;
  case 0x8301:ROM_BANK8(0xA000,V);break;
  case 0x8302:ROM_BANK8(0xC000,V);break;
  case 0x8310:mapbyte2[0]=V;m83chr();break;
  case 0x8311:mapbyte2[1]=V;m83chr();break;
  case 0x8312:mapbyte2[2]=V;isnot2kbank = 1;m83chr();break; // 2 mappers in one :)
  case 0x8313:mapbyte2[3]=V;isnot2kbank = 1;m83chr();break;
  case 0x8314:mapbyte2[4]=V;isnot2kbank = 1;m83chr();break;
  case 0x8315:mapbyte2[5]=V;isnot2kbank = 1;m83chr();break;
  case 0x8316:mapbyte2[6]=V;m83chr();break;
  case 0x8317:mapbyte2[7]=V;m83chr();break;
  case 0x8318:mapbyte1[1]=V;m83prg();break;
 }
// printf("$%04x:$%02x, $%04x\n",A,V,X.PC.W);
 
}

static void m83Reset(void)
{
 dipreg^=1;
}

void Mapper83_init(void)
{
 is2kbank = 0;
 isnot2kbank = 0;
 dipreg = 0;
 ROM_BANK8(0xc000,0x1e);
 ROM_BANK8(0xe000,0x1f);

 MapIRQHook=m83IRQHook;
 MapperReset=m83Reset;

 SetReadHandler(0x5000,0x5000,rd5000); // title scren dip switch
 SetReadHandler(0x5100,0x5103,rdlow);
 SetWriteHandler(0x5100,0x5103,wrlow);
 SetWriteHandler(0x8000,0xffff,Mapper83_write);
 mapbyte1[1]=0xF;
}
