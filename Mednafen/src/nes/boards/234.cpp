/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
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

static uint8 r1, r2;

static void DoBS(void)
{
 if(r1&0x40)
 {
  setprg32(0x8000, (r1&0xE)|(r2&1));
  setchr8( ((r1&0xE)<<2) | ((r2>>4)&7) );
 }
 else
 {
  setprg32(0x8000, r1&0xF);
  setchr8( ((r1&0xF)<<2) | ((r2>>4)&3) );
 }
}

static void R1Set(uint8 V)
{
 if(r1) return;
 r1=V;
 setmirror(((V >> 7) & 1) ^ 1);
 DoBS();
}

static void R2Set(uint8 V)
{
 r2=V;
 DoBS();
}

static DECLFW(R1W)
{
 R1Set(V);
}

static DECLFR(R1R)
{
 uint8 r=CartBR(A);
 R1Set(r);
 return r;
}

static DECLFW(R2W)
{
 R2Set(V);
}

static DECLFR(R2R)
{
 uint8 r=CartBR(A);
 R2Set(r);
 return r;
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] = 
 { SFVAR(r1), SFVAR(r2), SFEND };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 
 if(load)
 {
  DoBS();
  setmirror(((r1 >> 7) & 1) ^ 1);
 }

 return(ret);
}

static void Power(CartInfo *info)
{
 r1=r2=0;
 DoBS();
 setmirror(MI_V);
}

int Mapper234_Init(CartInfo *info)
{
        SetReadHandler(0x8000, 0xFFFF, CartBR);

        SetWriteHandler(0xff80,0xff9f,R1W);
	SetReadHandler(0xff80,0xff9f,R1R);

	SetWriteHandler(0xffe8,0xfff7,R2W);
	SetReadHandler(0xffe8,0xfff7,R2R);

	info->Power = Power;
	info->StateAction = StateAction;

	return(1);
}

