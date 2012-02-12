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
 * (VRC4 mapper)
 */

#include "mapinc.h"

static uint8 IRQCount;
static uint8 IRQa;
static uint8 prg_reg[2];
static uint8 chr_reg[8];
static uint8 mirr;

static SFORMAT StateRegs[]=
{
  {&IRQCount, 1, "IRQC"},
  {&IRQa, 1, "IRQA"},
  {prg_reg, 2, "PRG"},
  {chr_reg, 8, "CHR"},
  {&mirr, 1, "MIRR"},
  {0}
};

static void M222IRQ(void)
{
  if(IRQa)
  {
    IRQCount++;
    if(IRQCount>=238)
    {
      X6502_IRQBegin(FCEU_IQEXT);
//      IRQa=0;
    }
  }
}

static void Sync(void)
{
  int i;
  setprg8(0x8000,prg_reg[0]);
  setprg8(0xA000,prg_reg[1]);
  for(i=0; i<8; i++)
     setchr1(i<<10,chr_reg[i]);
  setmirror(mirr^1);
}

static DECLFW(M222Write)
{
  switch(A&0xF003)
  {
    case 0x8000: prg_reg[0]=V; break;
    case 0x9000: mirr=V&1;     break;
    case 0xA000: prg_reg[1]=V; break;
    case 0xB000: chr_reg[0]=V; break;
    case 0xB002: chr_reg[1]=V; break;
    case 0xC000: chr_reg[2]=V; break;
    case 0xC002: chr_reg[3]=V; break;
    case 0xD000: chr_reg[4]=V; break;
    case 0xD002: chr_reg[5]=V; break;
    case 0xE000: chr_reg[6]=V; break;
    case 0xE002: chr_reg[7]=V; break;
//    case 0xF000: FCEU_printf("%04x:%02x %d\n",A,V,scanline); IRQa=V; if(!V)IRQPre=0; X6502_IRQEnd(FCEU_IQEXT); break;
//  /  case 0xF001: FCEU_printf("%04x:%02x %d\n",A,V,scanline); IRQCount=V; break;
//    case 0xF002: FCEU_printf("%04x:%02x %d\n",A,V,scanline); break;
//    case 0xD001: IRQa=V; X6502_IRQEnd(FCEU_IQEXT); FCEU_printf("%04x:%02x %d\n",A,V,scanline); break;
//    case 0xC001: IRQPre=16; FCEU_printf("%04x:%02x %d\n",A,V,scanline); break;
    case 0xF000: IRQa=IRQCount=V; if(scanline<240) IRQCount-=8; else IRQCount+=4; X6502_IRQEnd(FCEU_IQEXT); break;
  }
  Sync();
}

static void M222Power(void)
{
  setprg16(0xC000,~0);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,M222Write);
}

static void StateRestore(int version)
{
  Sync();
}

void Mapper222_Init(CartInfo *info)
{
  info->Power=M222Power;
  GameHBIRQHook=M222IRQ;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
