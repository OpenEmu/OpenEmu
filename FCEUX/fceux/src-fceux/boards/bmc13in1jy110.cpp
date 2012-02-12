/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2005 CaH4e3
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
 *
 * BMC 42-in-1 reset switch
 */

#include "mapinc.h"

static uint8 bank_mode;
static uint8 bank_value;
static uint8 prgb[4];
static SFORMAT StateRegs[]=
{
  {0}
};

static void Sync(void)
{
  FCEU_printf("%02x: %02x %02x\n", bank_mode, bank_value, prgb[0]);  
  switch(bank_mode&7)
  {
    case 0:
         setprg32(0x8000,bank_value&7); break;
    case 1:
         setprg16(0x8000,((8+(bank_value&7))>>1)+prgb[1]);
         setprg16(0xC000,(bank_value&7)>>1);
    case 4:
         setprg32(0x8000,8+(bank_value&7)); break;
    case 5:
         setprg16(0x8000,((8+(bank_value&7))>>1)+prgb[1]);
         setprg16(0xC000,((8+(bank_value&7))>>1)+prgb[3]);
    case 2:
         setprg8(0x8000,prgb[0]>>2);
         setprg8(0xa000,prgb[1]);
         setprg8(0xc000,prgb[2]);
         setprg8(0xe000,~0);
         break;
    case 3:
         setprg8(0x8000,prgb[0]);
         setprg8(0xa000,prgb[1]);
         setprg8(0xc000,prgb[2]);
         setprg8(0xe000,prgb[3]);
         break;
  }
}

static DECLFW(BMC13in1JY110Write)
{
  FCEU_printf("%04x:%04x\n",A,V);
  switch(A)
  {
    case 0x8000:
    case 0x8001:
    case 0x8002:
    case 0x8003: prgb[A&3]=V; break;
    case 0xD000: bank_mode=V; break;
    case 0xD001: setmirror(V&3);
    case 0xD002: break;
    case 0xD003: bank_value=V; break;
  }
  Sync();
}

static void BMC13in1JY110Power(void)
{
  prgb[0]=prgb[1]=prgb[2]=prgb[3]=0;
  bank_mode=0;
  bank_value=0;
  setprg32(0x8000,0);
  setchr8(0);
  SetWriteHandler(0x8000,0xFFFF,BMC13in1JY110Write);
  SetReadHandler(0x8000,0xFFFF,CartBR);
}

static void StateRestore(int version)
{
  Sync();
}

void BMC13in1JY110_Init(CartInfo *info)
{
  info->Power=BMC13in1JY110Power;
  AddExState(&StateRegs, ~0, 0, 0);
  GameStateRestore=StateRestore;
}


