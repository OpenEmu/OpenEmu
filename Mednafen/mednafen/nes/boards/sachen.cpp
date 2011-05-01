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

static uint8 cmd;
static uint8 latch[8];
static int cmd_save, latch_save;
static void (*SachSync)(int version);

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  // FIXME to not abuse SFARRAYN:
  SFARRAYN(&cmd, cmd_save, "cmd"),
  SFARRAY(latch, latch_save),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  SachSync(load);
 return(ret);
}

static void StateInit(CartInfo *info, void (*ss)(int), int nc, int nl)
{
 info->StateAction = StateAction;
 SachSync = ss;
 cmd_save = nc;
 latch_save = nl;
}

static void S74LS374NSynco(void)
{
 setprg32(0x8000,latch[0]);
 setchr8(latch[1]);
 setmirror(latch[2]&1);
// setchr8(6);
}

static DECLFW(S74LS374NWrite)
{
 //printf("$%04x:$%02x\n",A,V);
 A&=0x4101;
 if(A==0x4100)
  cmd=V&7;
 else 
 {
  switch(cmd)
  {
   case 0:latch[0]=0;latch[1]=3;break;
   case 4:latch[1]&=3;latch[1]|=(V<<2);break;
   case 5:latch[0]=V&0x7;break;
   case 6:latch[1]&=0x1C;latch[1]|=V&3;break;
   case 7:latch[2]=V&1;break;
  }
  S74LS374NSynco();
 }
}

static void S74LS374NReset(CartInfo *info)
{
 latch[0]=latch[2]=0;
 latch[1]=3;
 S74LS374NSynco();
}

static void S74LS374NRestore(int version)
{
 S74LS374NSynco();
}

int S74LS374N_Init(CartInfo *info)
{
 info->Power=S74LS374NReset;
 StateInit(info, S74LS374NRestore, 1, 3);

 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0x4100,0x7FFF,S74LS374NWrite);

 return(1);
}

static int type;
static void S8259Synco(void)
{
 int x;

 setprg32(0x8000,latch[5]&7);

 if(!UNIFchrrama)	// No CHR RAM?  Then BS'ing is ok.
 {
  if(!type)
  {
   for(x=0;x<4;x++)
    setchr2(0x800*x,(x&1)|((latch[x]&7)<<1)|((latch[4]&7)<<4));
  }
  else
  {
   for(x=0;x<4;x++)
    setchr2(0x800*x,(latch[x]&0x7)|((latch[4]&7)<<3));
  }
 }
 switch((latch[7]>>1)&3)
 {
  case 0:setmirrorw(0,0,0,1);break;
  case 1:setmirror(MI_H);break;
  case 2:setmirror(MI_V);break;
  case 3:setmirror(MI_0);break;
 }
}

static DECLFW(S8259Write)
{
 A&=0x4101;
 if(A==0x4100) cmd=V;
 else 
 {
  latch[cmd&7]=V;
  S8259Synco();
 }
}

static void S8259Reset(CartInfo *info)
{
 int x;
 cmd=0;

 for(x=0;x<8;x++) latch[x]=0;
 if(UNIFchrrama) setchr8(0);

 S8259Synco();
}

static void S8259Restore(int version)
{
 S8259Synco();
}

int S8259A_Init(CartInfo *info)
{
 info->Power=S8259Reset;
 StateInit(info, S8259Restore, 1, 8);
 type=0;

 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0x4100,0x7FFF,S8259Write);

 return(1);
}

int S8259B_Init(CartInfo *info)
{
 info->Power=S8259Reset;
 StateInit(info, S8259Restore, 1, 8);
 type=1;

 return(1);
}

static void(*WSync)(void);

static void SA0161MSynco()
{
 setprg32(0x8000,(latch[0]>>3)&1); 
 setchr8(latch[0]&7);
}

static DECLFW(SAWrite)
{
 if(A&0x100)
 {
  latch[0]=V;
  WSync();
 }
}

static void SAReset(CartInfo *info)
{
 latch[0]=0;
 WSync();
}

static void SA0161MRestore(int version)
{
 SA0161MSynco();
}

int SA0161M_Init(CartInfo *info)
{
 WSync=SA0161MSynco;
 StateInit(info, SA0161MRestore, 0, 1);
 info->Power=SAReset;

 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0x4100,0x5FFF,SAWrite);

 return(1);
}

static void SA72007Synco()
{
 setprg32(0x8000,0);
 setchr8(latch[0]>>7);
}

static void SA72007Restore(int version)
{
 SA72007Synco();
}

int SA72007_Init(CartInfo *info)
{
 WSync=SA72007Synco;
 StateInit(info, SA72007Restore, 0, 1);
 info->Power=SAReset;

 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0x4100,0x5FFF,SAWrite);

 return(1);
}

static void SA72008Synco()
{
 setprg32(0x8000,(latch[0]>>2)&1);
 setchr8(latch[0]&3);
}

static void SA72008Restore(int version)
{
 SA72008Synco();
}

int SA72008_Init(CartInfo *info)
{
 WSync=SA72008Synco;
 StateInit(info, SA72008Restore, 0, 1);
 info->Power=SAReset;

 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0x4100,0x5FFF,SAWrite);

 return(1);
}

static DECLFW(SADWrite)
{
 latch[0]=V;
 WSync();
}

static void SADReset(CartInfo *info)
{
 latch[0]=0;
 WSync();
}

static void SA0036Synco()
{
 setprg32(0x8000,0);
 setchr8(latch[0]>>7);
}

static void SA0036Restore(int version)
{
 SA0036Synco();
}

static void SA0037Synco()
{
 setprg32(0x8000,(latch[0]>>3)&1);
 setchr8(latch[0]&7);
}

static void SA0037Restore(int version)
{
 SA0037Synco();
}

int SA0036_Init(CartInfo *info)
{
 WSync=SA0036Synco;
 StateInit(info, SA0036Restore, 0, 1);
 info->Power=SADReset;

 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0x8000,0xFFFF,SADWrite);

 return(1);
}

int SA0037_Init(CartInfo *info)
{
 WSync=SA0037Synco;
 StateInit(info, SA0037Restore, 0, 1);
 info->Power=SADReset;

 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0x8000,0xFFFF,SADWrite);

 return(1);
}

static void TCU01Synco()
{
 setprg32(0x8000,(latch[0]>>2)&1);
 setchr8((latch[0]>>3)&0xF);
}

static DECLFW(TCWrite)
{
 if((A&0x103)==0x102)
  latch[0]=V;
 TCU01Synco();
}

static void TCU01Reset(CartInfo *info)
{
 latch[0]=0;
 TCU01Synco();
}

static void TCU01Restore(int version)
{
 TCU01Synco();
}

int TCU01_Init(CartInfo *info)
{
 StateInit(info, TCU01Restore, 0, 1);
 info->Power=TCU01Reset;
 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0x4100,0xFFFF,TCWrite);
 return(1);
}

