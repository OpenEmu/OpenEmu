/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2007-2008 Mad Dumper, CaH4e3
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
 * Panda prince pirate.
 * MK4, MK6, A9711 board
 * 6035052 seems to be the same too, but with prot array in reverse
 * A9746  seems to be the same too, check
 * 187 seems to be the same too, check (A98402 board)
 */

#include "mapinc.h"
#include "mmc3.h"

static void Sync()
{
  switch(EXPREGS[5]&0x3F)
  {
    case 0x20: EXPREGS[7] = 1; EXPREGS[0]=EXPREGS[6]; break;
    case 0x29: EXPREGS[7] = 1; EXPREGS[0]=EXPREGS[6]; break;
    case 0x26: EXPREGS[7] = 0; EXPREGS[0]=EXPREGS[6]; break;
    case 0x2B: EXPREGS[7] = 1; EXPREGS[0]=EXPREGS[6]; break;
    case 0x2C: EXPREGS[7] = 1; if(EXPREGS[6]) EXPREGS[0]=EXPREGS[6]; break;
    case 0x3F: EXPREGS[7] = 1; EXPREGS[0]=EXPREGS[6]; break;
    case 0x28: EXPREGS[7] = 0; EXPREGS[1]=EXPREGS[6]; break;
    case 0x2A: EXPREGS[7] = 0; EXPREGS[2]=EXPREGS[6]; break;
    case 0x2F: break;
    default:   EXPREGS[5] = 0; break;
  }
}

static void M121CW(uint32 A, uint8 V)
{
  if((A&0x1000)==((MMC3_cmd&0x80)<<5))
    setchr1(A,V|0x100);
  else
    setchr1(A,V);
}

static void M121PW(uint32 A, uint8 V)
{
  if(EXPREGS[5]&0x3F)
  {
/*    FCEU_printf("prot banks: %02x %02x %02x %02x\n",V,EXPREGS[2],EXPREGS[1],EXPREGS[0]);*/
    setprg8(A,V&0x3F);
    setprg8(0xE000,EXPREGS[0]);
    setprg8(0xC000,EXPREGS[1]);
    setprg8(0xA000,EXPREGS[2]);
  } 
  else
  {
/*    FCEU_printf("gen banks: %04x %02x\n",A,V);*/
    setprg8(A,V&0x3F);
  }
}

static DECLFW(M121Write)
{
/*  FCEU_printf("write: %04x:%04x\n",A&0xE003,V);*/
  switch(A&0xE003)
  {
    case 0x8000: /*EXPREGS[5] = 0;*/
/*                 FCEU_printf("gen: %02x\n",V);*/
                 MMC3_CMDWrite(A,V);
                 FixMMC3PRG(MMC3_cmd);
                 break;
    case 0x8001: EXPREGS[6] = ((V&1)<<5)|((V&2)<<3)|((V&4)<<1)|((V&8)>>1)|((V&0x10)>>3)|((V&0x20)>>5);
/*                 FCEU_printf("bank: %02x (%02x)\n",V,EXPREGS[6]);*/
                 if(!EXPREGS[7]) Sync();
                 MMC3_CMDWrite(A,V);
                 FixMMC3PRG(MMC3_cmd);
                 break;
    case 0x8003: EXPREGS[5] = V;
/*                 EXPREGS[7] = 0;*/
/*                 FCEU_printf("prot: %02x\n",EXPREGS[5]);*/
                 Sync();
                 MMC3_CMDWrite(0x8000,V);
                 FixMMC3PRG(MMC3_cmd);
                 break;
  }
}

static uint8 prot_array[16] = { 0x83, 0x83, 0x42, 0x00 };
static DECLFW(M121LoWrite)
{
  EXPREGS[4] = prot_array[V&3];  /* 0x100 bit in address seems to be switch arrays 0, 2, 2, 3 (Contra Fighter)*/
/*  FCEU_printf("write: %04x:%04x\n",A,V);*/
}

static DECLFR(M121Read)
{ 
/*  FCEU_printf("read:  %04x->\n",A,EXPREGS[0]);*/
  return EXPREGS[4];
}

static void M121Power(void)
{
  EXPREGS[5] = 0;
  GenMMC3Power();
  SetReadHandler(0x5000,0x5FFF,M121Read);
  SetWriteHandler(0x5000,0x5FFF,M121LoWrite);
  SetWriteHandler(0x8000,0x9FFF,M121Write);
}

void Mapper121_Init(CartInfo *info)
{
  GenMMC3_Init(info, 128, 256, 8, 0);
  pwrap=M121PW;
  cwrap=M121CW;
  info->Power=M121Power;
  AddExState(EXPREGS, 8, 0, "EXPR");
}
