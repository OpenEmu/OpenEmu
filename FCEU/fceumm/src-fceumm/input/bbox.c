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
/*
#include <string.h>
#include <stdlib.h>
#include "share.h"

#define DI      01
#define CLK     02
#define CS      04
#define OUT0    01
#define D3      01
#define D4      02

typedef struct
{
  uint8 state;
  uint8 cmd;
  uint8 addr;
  uint8 iswritable;
  uint16 acc;
  uint16 data[128];
} EEPROM;

EEPROM serialROM[2];
uint8 oldCLK, bankFlip, DIFlip, OUT0state;

uint8 serialROMautomat(uint8 chip, uint16 data)
{
  uint8 resp = 1;
  chip &= 1;
  if(!(data & CS))
  {
    if(!(data & CLK))
    {
      uint8 state = serialROM[chip].state;
      uint8 mask, i;
      switch (serialROM[chip].cmd)
      {
        case 0x00:
             mask = ~(1<<(state&7));
             if(state<8)
             {
               serialROM[chip].addr &= mask;
               serialROM[chip].addr |= ((data&1)<<(state&7));
             }
             else if(state<15)
             {
               serialROM[chip].acc &= mask;
               serialROM[chip].acc |= ((data&1)<<(state&7));
             }
             else
             {
               serialROM[chip].acc &= mask;
               serialROM[chip].acc |= ((data&1)<<(state&7));
               serialROM[chip].cmd = serialROM[chip].acc;
             }
             break;
        case 0x01:
             if(state<30)
               resp = (serialROM[chip].data[serialROM[chip].addr]>>(state&15))&1;
             else
             {
               resp = (serialROM[chip].data[serialROM[chip].addr]>>(state&15))&1;
               serialROM[chip].cmd = 0;
             }
             break;
        case 0x06:
             mask = ~(1<<(state&15));
             if(state<30)
             {
               serialROM[chip].acc &= mask;
               serialROM[chip].acc |= ((data&1)<<(state&15));
             }
             else
             {
               serialROM[chip].acc &= mask;
               serialROM[chip].acc |= ((data&1)<<(state&15));
               if(serialROM[chip].iswritable)
                 serialROM[chip].data[serialROM[chip].addr] = serialROM[chip].acc;
               serialROM[chip].cmd = 0;
             }
             break;
        case 0x0C:
             for(i=0;i<128;i++)
                serialROM[chip].data[i] = 0xFFFF;
             serialROM[chip].cmd = 0;
             resp = 1;
             break;
        case 0x0D:
             serialROM[chip].cmd = 0;
             resp = 1;
             break;
        case 0x09:
             serialROM[chip].cmd = 0;
             serialROM[chip].iswritable = 1;
             break;
        case 0x0B:
             serialROM[chip].cmd = 0;
             serialROM[chip].iswritable = 0;
             break;
        default:
             serialROM[chip].cmd = 0;
             serialROM[chip].state = 0;
             break;
      }
    }
    else
    {
      if(serialROM[chip].cmd == 0)
      {
        if(serialROM[chip].state>15)
          serialROM[chip].state = 0;
      }
      else
        serialROM[chip].cmd++;
    }
  }
  else
  {
    serialROM[chip].cmd = 0;
    serialROM[chip].state = 0;
  }
  return resp;
}

uint8 serialROMstate(uint8 linestate)
{
  uint8 answ = 0, newCLK = linestate & CLK;
  if((!oldCLK)&&newCLK)
  {
    DIFlip^=1;
    if(linestate&&OUT0)
    {
      serialROMautomat(bankFlip, 04+DIFlip);
      bankFlip^=1;
      serialROMautomat(bankFlip, 02+DIFlip);
    }    
  }
  answ = DIFlip^1;
  answ |= (serialROMautomat(bankFlip, newCLK+DIFlip)<<1);
  oldCLK = newCLK;
  return answ;
}

static uint8 BBRead(int w, uint8 ret)
{
  if(w)
  {
    serialROMstate(OUT0);
    ret |= serialROMstate(OUT0|CLK);
  }
  return(ret);
}

static void BBWrite(uint8 V)
{
  OUT0state = V;
  serialROMstate(OUT0state?OUT0:0);
}

static INPUTCFC BattleBox={BBRead,BBWrite,0,0,0,0};

INPUTCFC *FCEU_InitBattleBox(void)
{
  oldCLK = 1;
  bankFlip = 0;
  DIFlip = 0;
  serialROM[0].iswritable = 1;
  serialROMautomat(0, 4);
  serialROM[1].iswritable = 1;
  serialROMautomat(1, 4);
  serialROMautomat(bankFlip, 02+DIFlip);
  return(&BattleBox);
}

*/
