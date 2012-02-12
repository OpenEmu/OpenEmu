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
 */

#include "mapinc.h"

static uint16 latche, latcheinit;
static uint16 addrreg0, addrreg1;
static void(*WSync)(void);
static readfunc defread;

static DECLFW(LatchWrite)
{
  FCEU_printf("%04x:%02x\n",A,V);
  latche=A;
  WSync();
}

static void LatchReset(void)
{
  latche=latcheinit;
  WSync();
}

static void LatchPower(void)
{
  latche=latcheinit;
  WSync();
  SetReadHandler(0x8000,0xFFFF,defread);
  SetWriteHandler(addrreg0,addrreg1,LatchWrite);
}

static void StateRestore(int version)
{
  WSync();
}

static void Latch_Init(CartInfo *info, void (*proc)(void), readfunc func, uint16 init, uint16 adr0, uint16 adr1)
{
  latcheinit=init;
  addrreg0=adr0;
  addrreg1=adr1;
  WSync=proc;
  if(func)
    defread=func;
  else
    defread=CartBR;
  info->Power=LatchPower;
  info->Reset=LatchReset;
  GameStateRestore=StateRestore;
  AddExState(&latche, 2, 0, "LATC");
}

//------------------ UNLCC21 ---------------------------

static void UNLCC21Sync(void)
{
  setprg32(0x8000,0);
  setchr8(latche&1);
  setmirror(MI_0+((latche&2)>>1));
}

void UNLCC21_Init(CartInfo *info)
{ 
  Latch_Init(info, UNLCC21Sync, 0, 0, 0x8000, 0xFFFF);
}

//------------------ BMCD1038 ---------------------------

static uint8 dipswitch;
static void BMCD1038Sync(void)
{
  if(latche&0x80)
  {
    setprg16(0x8000,(latche&0x70)>>4);
    setprg16(0xC000,(latche&0x70)>>4);
  }
  else
    setprg32(0x8000,(latche&0x60)>>5);
  setchr8(latche&7);
  setmirror(((latche&8)>>3)^1);
}

static DECLFR(BMCD1038Read)
{
  if(latche&0x100)
    return dipswitch;
  else
    return CartBR(A);
}

static void BMCD1038Reset(void)
{
  dipswitch++;
  dipswitch&=3; 
}

void BMCD1038_Init(CartInfo *info)
{ 
  Latch_Init(info, BMCD1038Sync, BMCD1038Read, 0, 0x8000, 0xFFFF);
  info->Reset=BMCD1038Reset;
  AddExState(&dipswitch, 1, 0, "DIPSW");
}


//------------------ Map 058 ---------------------------

static void BMCGK192Sync(void)
{
  if(latche&0x40)
  {
    setprg16(0x8000,latche&7);
    setprg16(0xC000,latche&7);
  }
  else
    setprg32(0x8000,(latche>>1)&3);
  setchr8((latche>>3)&7);
  setmirror(((latche&0x80)>>7)^1);
}

void BMCGK192_Init(CartInfo *info)
{
  Latch_Init(info, BMCGK192Sync, 0, 0, 0x8000, 0xFFFF);
}

//------------------ Map 200 ---------------------------

static void M200Sync(void)
{
//  FCEU_printf("A\n");
  setprg16(0x8000,latche&7);
  setprg16(0xC000,latche&7);
  setchr8(latche&7);
  setmirror((latche&8)>>3);
}

void Mapper200_Init(CartInfo *info)
{ 
  Latch_Init(info, M200Sync, 0, 0xff, 0x8000, 0xFFFF);
}

//------------------ 190in1 ---------------------------

static void BMC190in1Sync(void)
{
  setprg16(0x8000,(latche>>2)&0x07);
  setprg16(0xC000,(latche>>2)&0x07);
  setchr8((latche>>2)&0x07);
  setmirror((latche&1)^1);
}

void BMC190in1_Init(CartInfo *info)
{ 
  Latch_Init(info, BMC190in1Sync, 0, 0, 0x8000, 0xFFFF);
}

