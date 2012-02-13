/* FCE Ultra - NES/Famicom Emulator
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

static void AYSound(int Count);
static void AYSoundHQ(void);
static void DoAYSQ(int x);
static void DoAYSQHQ(int x);

#define sunselect mapbyte1[0]
#define sungah    mapbyte1[1]
static uint8 sunindex;

static DECLFW(SUN5BWRAM)
{
 if((sungah&0xC0)==0xC0)
  (WRAM-0x6000)[A]=V;
}

static DECLFR(SUN5AWRAM)
{
 if((sungah&0xC0)==0x40)
  return X.DB;
 return CartBROB(A);
}

static DECLFW(Mapper69_SWL)
{
  sunindex=V%14;
}

static DECLFW(Mapper69_SWH)
{
             int x;
             GameExpSound.Fill=AYSound;
             GameExpSound.HiFill=AYSoundHQ;
             if(FSettings.SndRate);
             switch(sunindex)
             {
              case 0:
              case 1:
              case 8:
#if SOUND_QUALITY == 1
		      DoAYSQHQ(0);
#else
		      DoAYSQ(0);
#endif
		      break;
              case 2:
              case 3:
              case 9:
#if SOUND_QUALITY == 1
		      DoAYSQHQ(1);
#else
		      DoAYSQ(1);
#endif
		      break;
              case 4:
              case 5:
              case 10:
#if SOUND_QUALITY == 1
		      DoAYSQHQ(2);
#else
		      DoAYSQ(2);
#endif
		      break;
              case 7:
                     for(x=0;x<2;x++)
#if SOUND_QUALITY == 1
		     DoAYSQHQ(x);
#else
		     DoAYSQ(x);
#endif
                     break;
             }
             MapperExRAM[sunindex]=V;
}

static DECLFW(Mapper69_write)
{
 switch(A&0xE000)
 {
  case 0x8000:sunselect=V;break;
  case 0xa000:
              sunselect&=0xF;
              if(sunselect<=7)
               VROM_BANK1(sunselect<<10,V);
              else
               switch(sunselect&0x0f)
               {
                case 8:
                       sungah=V;
                       if(V&0x40)
                        {
                         if(V&0x80) /* Select WRAM*/
                          setprg8r(0x10,0x6000,0);
                        }
                        else
                         setprg8(0x6000,V);
                        break;
                case 9:ROM_BANK8(0x8000,V);break;
                case 0xa:ROM_BANK8(0xa000,V);break;
                case 0xb:ROM_BANK8(0xc000,V);break;
                case 0xc:
                         switch(V&3)
                         {
                          case 0:MIRROR_SET2(1);break;
                          case 1:MIRROR_SET2(0);break;
                          case 2:onemir(0);break;
                          case 3:onemir(1);break;
                         }
                         break;
             case 0xd:IRQa=V;X6502_IRQEnd(FCEU_IQEXT);break;
             case 0xe:IRQCount&=0xFF00;IRQCount|=V;X6502_IRQEnd(FCEU_IQEXT);break;
             case 0xf:IRQCount&=0x00FF;IRQCount|=V<<8;X6502_IRQEnd(FCEU_IQEXT);break;
             }
             break;
 }
}

static int32 vcount[3];
static int32 dcount[3];
static int CAYBC[3];

static void DoAYSQ(int x)
{
    int32 freq=((MapperExRAM[x<<1]|((MapperExRAM[(x<<1)+1]&15)<<8))+1)<<(4+17);
    int32 amp=(MapperExRAM[0x8+x]&15)<<2;
    int32 start,end;
    int V;

    amp+=amp>>1;

    start=CAYBC[x];
    end=(SOUNDTS<<16)/soundtsinc;
    if(end<=start) return;
    CAYBC[x]=end;

    if(amp)
    for(V=start;V<end;V++)
    {
     if(dcount[x])
      Wave[V>>4]+=amp;
     vcount[x]-=nesincsize;
     while(vcount[x]<=0)
     {
      dcount[x]^=1;
      vcount[x]+=freq;
     }
    }
}

static void DoAYSQHQ(int x)
{
 int32 V;
 int32 freq=((MapperExRAM[x<<1]|((MapperExRAM[(x<<1)+1]&15)<<8))+1)<<4;
 int32 amp=(MapperExRAM[0x8+x]&15)<<6;

 amp+=amp>>1;

 if(!(MapperExRAM[0x7]&(1<<x)))
 {
  for(V=CAYBC[x];V<SOUNDTS;V++)
  {
   if(dcount[x])
    WaveHi[V]+=amp;
   vcount[x]--;
   if(vcount[x]<=0)
   {
    dcount[x]^=1;
    vcount[x]=freq;
   }
  }
 }
 CAYBC[x]=SOUNDTS;
}

static void AYSound(int Count)
{
    int x;
    DoAYSQ(0);
    DoAYSQ(1);
    DoAYSQ(2);
    for(x=0;x<3;x++)
     CAYBC[x]=Count;
}

static void AYSoundHQ(void)
{
    DoAYSQHQ(0);
    DoAYSQHQ(1);
    DoAYSQHQ(2);
}

static void AYHiSync(int32 ts)
{
 int x;

 for(x=0;x<3;x++)
  CAYBC[x]=ts;
}

static void SunIRQHook(int a)
{
  if(IRQa)
  {
   IRQCount-=a;
   if(IRQCount<=0)
   {X6502_IRQBegin(FCEU_IQEXT);IRQa=0;IRQCount=0xFFFF;}
  }
}

void Mapper69_StateRestore(int version)
{
   if(mapbyte1[1]&0x40)
   {
    if(mapbyte1[1]&0x80) /* Select WRAM*/
     setprg8r(0x10,0x6000,0);
   }
   else
    setprg8(0x6000,mapbyte1[1]);
}

void Mapper69_ESI(void)
{
 GameExpSound.RChange=Mapper69_ESI;
 GameExpSound.HiSync=AYHiSync;
 memset(dcount,0,sizeof(dcount));
 memset(vcount,0,sizeof(vcount));
 memset(CAYBC,0,sizeof(CAYBC));
}

void NSFAY_Init(void)
{
 sunindex=0;
 SetWriteHandler(0xc000,0xdfff,Mapper69_SWL);
 SetWriteHandler(0xe000,0xffff,Mapper69_SWH);
 Mapper69_ESI();
}

void Mapper69_init(void)
{
 sunindex=0;

 SetupCartPRGMapping(0x10,WRAM,8192,1);

 SetWriteHandler(0x8000,0xbfff,Mapper69_write);
 SetWriteHandler(0xc000,0xdfff,Mapper69_SWL);
 SetWriteHandler(0xe000,0xffff,Mapper69_SWH);
 SetWriteHandler(0x6000,0x7fff,SUN5BWRAM);
 SetReadHandler(0x6000,0x7fff,SUN5AWRAM);
 Mapper69_ESI();
 MapIRQHook=SunIRQHook;
 MapStateRestore=Mapper69_StateRestore;
}

