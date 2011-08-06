/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2003 Xodnizel
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

#include "share.h"

static uint32 MReal,MRet;

static uint8 MJ_Read(int w, uint8 ret)
{
 if(w) 
 {
//  ret|=(MRet&1)<<1;
  ret|=((MRet&0x80)>>6)&2;
//  MRet>>=1;
 if(!fceuindbg)
  MRet<<=1;
 }
 return(ret);
}

static void MJ_Write(uint8 v)
{
 /* 1: I-D7, J-D6, K-D5, L-D4, M-D3, Big Red-D2
    2: A-D7, B-D6, C-D5, D-D4, E-D3, F-D2, G-D1, H-D0
    3: Sel-D6, Start-D7, D5, D4, D3, D2, D1
 */
 MRet=0;

 v>>=1;
 v&=3;

 if(v==3)
 {
  MRet=(MReal>>14)&0x7F;
  //MRet=((MRet&0x1F) |((MRet&0x40)>>1)|((MRet&0x20)<<1)) <<1; //(MReal>>13)&0x7F;
 }
 else if(v==2)
 {
  MRet=MReal&0xFF;
 }
 else if(v==1)
 {
  MRet=(MReal>>8)&0x3F;
 }
// HSValR=HSVal<<1;
}

static void MJ_Update(void *data)
{
 MReal=*(uint32*)data;
 //printf("%08x\n",MReal>>13);
 //HSVal=*(uint8*)data;
}

static int StateActionFC(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
   SFVAR(MReal),
   SFVAR(MRet),
   SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "INPF");
 if(load)
 {

 }
 return(ret);
}

static INPUTCFC Mahjong={MJ_Read,MJ_Write,0,MJ_Update,0,0, StateActionFC, 1, sizeof(uint32) };

INPUTCFC *MDFN_InitMahjong(void)
{
 MReal=MRet=0;
 return(&Mahjong);
}
