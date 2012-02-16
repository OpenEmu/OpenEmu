/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2011 CaH4e3
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
 * FDS Conversion
 *
 */

#include "mapinc.h"

static uint8 reg[16], reset0 = 0, reset1 = 0;
static SFORMAT StateRegs[]=
{
  {reg, 16, "PRG"},
  {0}
};

static void Sync(void)
{
  setprg2(0x6000,reg[4]); /* A.0, 9.1, 9.2*/
  setprg2(0x6800,reg[5]); /* 8.0, 9.3*/
  setprg2(0x7000,reg[6]); /* 8.1*/
  setprg2(0x7800,reg[7]); /* 8.2*/
  setprg2(0x8000,reg[0]); /* 8.3*/
  setprg2(0x8800,reg[1]); /* 9.0*/
  setprg2(0x9000,reg[2]); /* C.2, 3.2, 7.3*/
  setprg2(0x9800,reg[3]); /* C.3*/
  setprg2(0xA000,0x34); /* D.0*/
  setprg2(0xA800,0x35); /* D.1*/
  setprg2(0xB000,0x36); /* D.2*/
  setprg2(0xB800,0x37); /* D.3*/
  setprg2(0xC000,0x28); /* WTF?*/
  setprg2(0xC800,0x29); /* A.1*/
  setprg2(0xD000,0x2A); /* A.2*/
  setprg2(0xD800,0x2B); /* A.3*/
  setprg2(0xE000,0x2C); /* B.0*/
  setprg2(0xE800,0x2D); /* B.1*/
  setprg2(0xF000,0x2E); /* B.2*/
  setprg2(0xF800,0x2F); /* B.3*/
  setchr8(0);
  setmirror(MI_V);
}

static DECLFW(UNLKS7057Write)
{
/*  FCEU_printf("%04X:%02X\n",A,V);*/
  switch(A) {
   case 0x9000: reg[16] = V; Sync(); break;
   case 0xB000: reg[0] = (reg[0] & 0xF0) | (V & 0x0F); Sync(); break;
   case 0xB001: reg[0] = (reg[0] & 0x0F) | (V << 4); Sync(); break;
   case 0xB002: reg[1] = (reg[1] & 0xF0) | (V & 0x0F); Sync(); break;
   case 0xB003: reg[1] = (reg[1] & 0x0F) | (V << 4); Sync(); break;
   case 0xC000: reg[2] = (reg[2] & 0xF0) | (V & 0x0F); Sync(); break;
   case 0xC001: reg[2] = (reg[2] & 0x0F) | (V << 4); Sync(); break;
   case 0xC002: reg[3] = (reg[3] & 0xF0) | (V & 0x0F); Sync(); break;
   case 0xC003: reg[3] = (reg[3] & 0x0F) | (V << 4); Sync(); break;
   case 0xD000: reg[4] = (reg[4] & 0xF0) | (V & 0x0F); Sync(); break;
   case 0xD001: reg[4] = (reg[4] & 0x0F) | (V << 4); Sync(); break;
   case 0xD002: reg[5] = (reg[5] & 0xF0) | (V & 0x0F); Sync(); break;
   case 0xD003: reg[5] = (reg[5] & 0x0F) | (V << 4); Sync(); break;
   case 0xE000: reg[6] = (reg[6] & 0xF0) | (V & 0x0F); Sync(); break;
   case 0xE001: reg[6] = (reg[6] & 0x0F) | (V << 4); Sync(); break;
   case 0xE002: reg[7] = (reg[7] & 0xF0) | (V & 0x0F); Sync(); break;
   case 0xE003: reg[7] = (reg[7] & 0x0F) | (V << 4); Sync(); break;
  }
}

static void UNLKS7057Power(void)
{
  reset1++;
  FCEU_printf("reset1 %02x\n",reset1);
  Sync();
  SetReadHandler(0x6000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,UNLKS7057Write);
}

static void UNLKS7057Reset(void)
{
  reset0++;
  FCEU_printf("reset0 %02x\n",reset0);
  Sync();
}

void UNLKS7057_Init(CartInfo *info)
{
  info->Power=UNLKS7057Power;
  info->Reset=UNLKS7057Reset;
  AddExState(&StateRegs, ~0, 0, 0);
}
