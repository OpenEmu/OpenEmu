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
 * MK4, MK6, A9711 board, MAPPER 187 the same!
 * UNL6035052_Init seems to be the same too, but with prot array in reverse
 */

#include "mapinc.h"
#include "mmc3.h"

static uint8 readbyte = 0;

static DECLFW(M121Write)
{
//  FCEU_printf("write: %04x:%04x\n",A&0xE003,V);
  if((A&0xF003)==0x8003)
  {
//    FCEU_printf("       prot write");
//    FCEU_printf("write: %04x:%04x\n",A,V);
    if     (V==0xAB) setprg8(0xE000,7);
    else if(V==0x26) setprg8(0xE000,8);
//    else if(V==0x26) setprg8(0xE000,1); // MK3
//    else if(V==0x26) setprg8(0xE000,0x15); // sonic 3D blast, 8003 - command (0x26), 8001 - data 0x2A (<<1 = 0x15)
    else if(V==0xFF) setprg8(0xE000,9);
    else if(V==0x28) setprg8(0xC000,0xC);
    else if(V==0xEC) setprg8(0xE000,0xD); 
//    else if(V==0xEC) setprg8(0xE000,0xC);//MK3
    else if(V==0xEF) setprg8(0xE000,0xD); // damn mess, need real hardware to figure out bankswitching
    else if(V==0x2A) setprg8(0xA000,0x0E);
//    else if(V==0x2A) setprg8(0xE000,0x0C); // MK3
    else if(V==0x20) setprg8(0xE000,0x13);
    else if(V==0x29) setprg8(0xE000,0x1B);
    else 
    {
//      FCEU_printf(" unknown");
      FixMMC3PRG(MMC3_cmd);
      MMC3_CMDWrite(A,V);
    }
//      FCEU_printf("\n");
  }
  else
  {
//    FixMMC3PRG(MMC3_cmd);
    MMC3_CMDWrite(A,V);
  }
}

static uint8 prot_array[16] = { 0x83, 0x83, 0x42, 0x00 };
static DECLFW(M121LoWrite)
{
  EXPREGS[0] = prot_array[V&3];  // 0x100 bit in address seems to be switch arrays 0, 2, 2, 3 (Contra Fighter)
//  FCEU_printf("write: %04x:%04x\n",A,V);
}

static DECLFR(M121Read)
{ 
//  FCEU_printf("read:  %04x\n",A);
  return EXPREGS[0];
}

static void M121Power(void)
{
  GenMMC3Power();
//  Write_IRQFM(0x4017,0x40);
  SetReadHandler(0x5000,0x5FFF,M121Read);
  SetWriteHandler(0x5000,0x5FFF,M121LoWrite);
  SetWriteHandler(0x8000,0x9FFF,M121Write);
}

void Mapper121_Init(CartInfo *info)
{
  GenMMC3_Init(info, 128, 256, 8, 0);
  info->Power=M121Power;
  AddExState(EXPREGS, 2, 0, "EXPR");
}
