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

#include "mapinc.h"
#include "mmc3.h"

static DECLFW(UNLA9746Write)
{
/*   FCEU_printf("write raw %04x:%02x\n",A,V);*/
   switch (A&0xE003)
   {
     case 0x8000: EXPREGS[1]=V; EXPREGS[0]=0; break;
     case 0x8002: EXPREGS[0]=V; EXPREGS[1]=0; break;
     case 0x8001: {
                    uint8 bits_rev = ((V&0x20)>>5)|((V&0x10)>>3)|((V&0x08)>>1)|((V&0x04)<<1);
                    switch(EXPREGS[0])
                    {
                      case 0x26: setprg8(0x8000, bits_rev); break;
                      case 0x25: setprg8(0xA000, bits_rev); break; 
                      case 0x24: setprg8(0xC000, bits_rev); break; 
                      case 0x23: setprg8(0xE000, bits_rev); break;
                    }
                    switch(EXPREGS[1])
                    {
                      case 0x0a:
                      case 0x08: EXPREGS[2] = (V << 4); break;
                      case 0x09: setchr1(0x0000, EXPREGS[2]|(V >> 1)); break;
                      case 0x0b: setchr1(0x0400, EXPREGS[2]|(V >> 1)|1);  break;
                      case 0x0c:
                      case 0x0e: EXPREGS[2] = (V << 4);  break;
                      case 0x0d: setchr1(0x0800, EXPREGS[2]|(V >> 1));  break;
                      case 0x0f: setchr1(0x0c00, EXPREGS[2]|(V >> 1)|1);  break;
                      case 0x10:
                      case 0x12: EXPREGS[2] = (V << 4);  break;
                      case 0x11: setchr1(0x1000, EXPREGS[2]|(V >> 1)); break;
                      case 0x14:
                      case 0x16: EXPREGS[2] = (V << 4);  break;
                      case 0x15: setchr1(0x1400, EXPREGS[2]|(V >> 1));  break;
                      case 0x18:
                      case 0x1a: EXPREGS[2] = (V << 4);  break;
                      case 0x19: setchr1(0x1800, EXPREGS[2]|(V >> 1));  break;
                      case 0x1c:
                      case 0x1e: EXPREGS[2] = (V << 4);  break;
                      case 0x1d: setchr1(0x1c00, EXPREGS[2]|(V >> 1));  break;
                    }
                  }
                  break;
   }
}

static void UNLA9746Power(void)
{
  GenMMC3Power();
  SetWriteHandler(0x8000,0xbfff,UNLA9746Write);
}

void UNLA9746_Init(CartInfo *info)
{
  GenMMC3_Init(info, 128, 256, 0, 0);
  info->Power=UNLA9746Power;
  AddExState(EXPREGS, 6, 0, "EXPR");
}

