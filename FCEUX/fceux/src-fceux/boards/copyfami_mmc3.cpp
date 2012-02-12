/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2007 CaH4e3
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
 
 /*
0000 - 1FFF - RAM
2000 - 23FF - PPU
2400 - 27FF - Write: "PAG" / Read: --
2800 - 2BFF - Write: "BNK" / Read: "STS"
2C00 - 2FFF - Write: "UWR" / Read: "URD"
3000 - 3FFF - Small Flash Page (в регистре BNK)
4000 - 7FFF - Free
8000 - FFFF - Cart/Big Flash Page (смотри регистр PAG)
Биты:
Регистр [PAG], по сбросу должен быть = $00.
D3-D0 - Big Page High Address (D3,D2,D1,D0,A14,A13,A12,A11,A10,A9,A8,A7,A6,A5,A4,A3,A2,A1,A0)
 D4   - VMD бит. Если =0, то к PPU подключено внутренее ОЗу на 8Кб, если =1 - то картридж.
 D5   - STR бит. Если =0, то вместо карта по адресам 8000-FFFF внутренняя флэш, =1 - карт.
Регистр [BNK], не предустанавливается
D6-D0 - Small Page High Address (D6,D5,D4,D3,D2,D1,D0,A11,A10,A9,A8,A7,A6,A5,A4,A3,A2,A1,A0)
 D7   - S/W бит. Управляет USB мостом, в эмуле реализовывать не надо.
Регистры [UWR]/[URD], не предустанавливаются, в эмуле реализовывать не надо.
[UWR] - Регистр записи данных в USB мост.
[URD] - Регистр чтения из USB моста.
 */

#include "mapinc.h"
#include "mmc3.h"

static uint8 reg[3];

static uint8 *CHRRAM=NULL;   // there is no more extern CHRRAM in mmc3.h
							 // I need chrram here and local   static == local
static uint32 CHRRAMSIZE;

static SFORMAT StateRegs[]=
{
  {reg, 3, "REGS"},
  {0}
};

static void Sync(void)
{
  setprg4r(1,0x3000,reg[1]&0x7F);
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
}

static void MCopyFamiMMC3PW(uint32 A, uint8 V)
{
  if(reg[0]&0x20)
    setprg8r(0,A,V);
  else
    setprg32r(1,0x8000,reg[0]&0x0F);
}

static void MCopyFamiMMC3CW(uint32 A, uint8 V)
{
  if(reg[0]&0x20)
    setchr1r(0,A,V);
  else
    setchr8r(0x10,0);
}

static DECLFW(MCopyFamiMMC3WritePAG)
{
  reg[0]=V;
  Sync();
}

static DECLFW(MCopyFamiMMC3WriteBNK)
{
  reg[1]=V;
  Sync();
}

static DECLFW(MCopyFamiMMC3WriteUSB)
{
  reg[2]=V;
}

static void MCopyFamiMMC3Power(void)
{
  reg[0] = 0;
  GenMMC3Power();
  Sync();
  SetReadHandler(0x3000,0x3FFF,CartBR);
  SetWriteHandler(0x3000,0x3FFF,CartBW);

  SetWriteHandler(0x2400,0x27FF,MCopyFamiMMC3WritePAG);
  SetWriteHandler(0x2800,0x2BFF,MCopyFamiMMC3WriteBNK);
  SetWriteHandler(0x2C00,0x2FFF,MCopyFamiMMC3WriteUSB);
}

static void MCopyFamiMMC3Reset(void)
{
  reg[0] = 0;
  MMC3RegReset();
  Sync();
}

static void MCopyFamiMMC3Close(void)
{
  if(CHRRAM)
    free(CHRRAM);
  CHRRAM=NULL;
}

static void StateRestore(int version)
{
  Sync();
}

void MapperCopyFamiMMC3_Init(CartInfo *info)
{
  GenMMC3_Init(info, 512, 512, 0, 0);

  cwrap=MCopyFamiMMC3CW;
  pwrap=MCopyFamiMMC3PW;

  info->Reset=MCopyFamiMMC3Reset;
  info->Power=MCopyFamiMMC3Power;
  info->Close=MCopyFamiMMC3Close;
  GameStateRestore=StateRestore;

  CHRRAMSIZE=8192;
  CHRRAM=(uint8*)malloc(CHRRAMSIZE);
  SetupCartPRGMapping(0x10,CHRRAM,CHRRAMSIZE,1);
  AddExState(CHRRAM, CHRRAMSIZE, 0, "SRAM");

  AddExState(&StateRegs, ~0, 0, 0);
}
