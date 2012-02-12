/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2006 CaH4e3
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
 * 700in1 and 400in1 carts
 */
 

#include "mapinc.h"

static uint16 cmd, bank;

static SFORMAT StateRegs[]=
{
  {&cmd, 2, "CMD"},
  {&bank, 2, "BANK"},
  {0}
};

static void Sync(void)
{
  setmirror((cmd&1)^1);
  setchr8(0);
  if(cmd&2)
  {
    if(cmd&0x100)
    {
      setprg16(0x8000,((cmd&0xe0)>>2)|bank);
      setprg16(0xC000,((cmd&0xe0)>>2)|7);  
    }
    else
    {
      setprg16(0x8000,((cmd&0xe0)>>2)|(bank&6));
      setprg16(0xC000,((cmd&0xe0)>>2)|((bank&6)|1));  
    }
  }
  else
  {
    setprg16(0x8000,((cmd&0xe0)>>2)|bank);
    setprg16(0xC000,((cmd&0xe0)>>2)|bank);
  }  
}

static DECLFW(UNLN625092WriteCommand)
{
  cmd=A;  
  Sync();
}

static DECLFW(UNLN625092WriteBank)
{ 
  bank=A&7;
  Sync();
}

static void UNLN625092Power(void)
{
  cmd=0;
  bank=0;
  Sync();
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xBFFF,UNLN625092WriteCommand);
  SetWriteHandler(0xC000,0xFFFF,UNLN625092WriteBank);
}

static void UNLN625092Reset(void)
{
  cmd=0;
  bank=0;
}

static void StateRestore(int version)
{
  Sync();
}

void UNLN625092_Init(CartInfo *info)
{
  info->Reset=UNLN625092Reset;
  info->Power=UNLN625092Power;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
