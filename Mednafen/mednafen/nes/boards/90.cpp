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

static int is209;

static uint8 IRQMode;	// from $c001
static uint8 IRQPre;	// from $c004
static uint8 IRQPreSize; // from $c007
static uint8 IRQCount;	// from $c005
static uint8 IRQXOR;	// Loaded from $C006
static uint8 IRQa;	// $c002, $c003, and $c000

static uint8 mul[2];
static uint8 regie;

static uint8 tkcom[4];
static uint8 prgb[4];
static uint8 chrlow[8];
static uint8 chrhigh[8];

static uint16 names[4];
static uint8 tekker;

static DECLFR(tekread)
{
 //MDFN_printf("READ READ READ: $%04x, $%04x\n",A,X.PC);
 switch(A)
 {
  case 0x5000:return(tekker);
  case 0x5800:return(mul[0]*mul[1]);
  case 0x5801:return((mul[0]*mul[1])>>8);
  case 0x5803:return(regie);
  default:break;
 } 
 return(X.DB);
}

static void mira(void)
{
 if(tkcom[0]&0x20 && is209)
 {
  int x;
  if(tkcom[0] & 0x40)	// Name tables are ROM-only
  {
   for(x=0;x<4;x++)
    setntamem(CHRptr[0]+(((names[x])&CHRmask1[0])<<10), 0, x);
  }
  else			// Name tables can be RAM or ROM.
  {
   for(x=0;x<4;x++)
   {
    if((tkcom[2]&0x80) == (names[x]&0x80))	// RAM selected.
     setntamem(NTARAM + ((names[x]&0x1)<<10),1,x);
    else
     setntamem(CHRptr[0]+(((names[x])&CHRmask1[0])<<10), 0, x);
   }
  }
 }
 else
 {
  switch(tkcom[1]&3){
                  case 0:setmirror(MI_V);break;
                  case 1:setmirror(MI_H);break;
                  case 2:setmirror(MI_0);break;
                  case 3:setmirror(MI_1);break;
               }
 }
}

static void tekprom(void)
{
 switch(tkcom[0]&3)
  {
   case 1:              // 16 KB
          setprg16(0x8000,prgb[0]);
          setprg16(0xC000,prgb[2]);
          break;
   case 2:              //2 = 8 KB ??
	  if(tkcom[0]&0x4)
	  {
           setprg8(0x8000,prgb[0]);
           setprg8(0xa000,prgb[1]);
           setprg8(0xc000,prgb[2]);
           setprg8(0xe000,prgb[3]);
	  }
	  else
	  {
	   if(tkcom[0]&0x80)
	    setprg8(0x6000,prgb[3]);
           setprg8(0x8000,prgb[0]);
           setprg8(0xa000,prgb[1]);
           setprg8(0xc000,prgb[2]);
           setprg8(0xe000,~0);
	  }
 	  break;
   case 0:
   case 3:
          setprg8(0x8000,prgb[0]);
          setprg8(0xa000,prgb[1]);
          setprg8(0xc000,prgb[2]);
          setprg8(0xe000,prgb[3]);
          break;
  }
}

static void tekvrom(void)
{
 int x;

 switch(tkcom[0]&0x18)
  {
   case 0x00:      // 8KB
           setchr8(chrlow[0]|(chrhigh[0]<<8));
	   break;
   case 0x08:      // 4KB
          for(x=0;x<8;x+=4)
           setchr4(x<<10,chrlow[x]|(chrhigh[x]<<8));
	  break;
   case 0x10:      // 2KB
	  for(x=0;x<8;x+=2)
           setchr2(x<<10,chrlow[x]|(chrhigh[x]<<8));
	  break;
   case 0x18:      // 1KB
	   for(x=0;x<8;x++)
	    setchr1(x<<10,(chrlow[x]|(chrhigh[x]<<8)));
	   break;
 }
}

static DECLFW(Mapper90_write)
{
 //printf("$%04x:$%02x\n",A,V); 

 if(A==0x5800) mul[0]=V;
 else if(A==0x5801) mul[1]=V;
 else if(A==0x5803) regie=V;
 

 //if(A>=0xc000 && A<=0xc007) 
 // MDFN_printf("$%04x:$%02x $%04x, %d, %d\n",A,V,X.PC,scanline,timestamp);

 A&=0xF007;
 if(A>=0x8000 && A<=0x8003)
 {
  prgb[A&3]=V;
  tekprom();
 }
 else if(A>=0x9000 && A<=0x9007)
 {
  chrlow[A&7]=V;
  tekvrom();
 }
 else if(A>=0xa000 && A<=0xa007)
 {
  chrhigh[A&7]=V;
  tekvrom();
 }
 else if(A>=0xb000 && A<=0xb007)
 {
  //printf("$%04x:$%02x\n",A,V);
  if(A&4)
  {
   names[A&3]&=0x00FF;
   names[A&3]|=V<<8;
  }
  else
  {
   names[A&3]&=0xFF00;
   names[A&3]|=V;
  }
  mira();
 }
 else if(A>=0xd000 && A<=0xdfff)
 {
  tkcom[A&3]=V;
  tekprom();
  tekvrom();
  mira();
 }
 else switch(A)
 {
  case 0xc000: IRQa=V&1;if(!(V&1)) X6502_IRQEnd(MDFN_IQEXT);break;
  case 0xc002: IRQa=0;X6502_IRQEnd(MDFN_IQEXT);break;
  case 0xc003: IRQa=1;break;

  case 0xc001: IRQMode=V;break;
  case 0xc004: IRQPre=V^IRQXOR;break;
  case 0xc005: IRQCount=V^IRQXOR;break;
  case 0xc006: IRQXOR=V;break;
  case 0xc007: IRQPreSize=V;break;
 }

}

static void CCL(void)
{
 if((IRQMode>>6) == 1) // Count Up
 {
  IRQCount++;
  if((IRQCount == 0) && IRQa)
   X6502_IRQBegin(MDFN_IQEXT);
 } 
 else if((IRQMode>>6) == 2) // Count down
 { 
  IRQCount--;
  if((IRQCount == 0xFF) && IRQa)
   X6502_IRQBegin(MDFN_IQEXT);
 }
}

static void ClockCounter(void)
{
 uint8 premask;

 if(IRQMode & 0x4) premask = 0x7;
 else premask = 0xFF;

 if((IRQMode>>6) == 1) // Count up
 {
  IRQPre++;

  if((IRQPre & premask) == 0)
   CCL();
 }
 else if((IRQMode>>6) == 2) // Count down
 {
  IRQPre--;
  //printf("%d\n",IRQPre);
  if((IRQPre & premask) == premask)
   CCL();
 }
}

static void SLWrap(void)
{
 int x;
 for(x=0;x<8;x++) ClockCounter();	// 8 PPU A10 0->1 transitions per scanline(usually)
}
/*
static uint32 lasta;
static void YARGH(uint32 A)
{
 if((A&0x1000) && !(lasta & 0x1000))
  ClockCounter();

 lasta = A;
}
*/

static void togglie(CartInfo *info)
{
 tekker^=0xFF;
}

static void M90Power(CartInfo *info)
{
  mul[0]=mul[1]=regie=0xFF;

  tkcom[0]=tkcom[1]=tkcom[2]=tkcom[3]=0xFF;

  memset(prgb,0xff,sizeof(prgb));
  memset(chrlow,0xff,sizeof(chrlow));
  memset(chrhigh,0xff,sizeof(chrhigh));
  memset(names,0x00,sizeof(names));

  tekker=0;

  prgb[0]=0xFF;
  prgb[1]=0xFF;
  prgb[2]=0xFF;
  prgb[3]=0xFF;
  tekprom();
  tekvrom();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[]={
        SFVARN(IRQCount, "IRQC"),
        SFVARN(IRQa, "IRQa"),
        SFARRAYN(mul, 2, "MUL"),
        SFVARN(regie, "REGI"),
        SFARRAYN(tkcom, 4, "TKCO"),
        SFARRAYN(prgb, 4, "PRGB"),
        SFARRAYN(chrlow, 4, "CHRL"),
        SFARRAYN(chrhigh, 8, "CHRH"),
        SFVARN(names[0], "NMS0"),
        SFVARN(names[1], "NMS1"),
        SFVARN(names[2], "NMS2"),
        SFVARN(names[3], "NMS3"),
        SFVARN(tekker, "TEKR"),
        SFEND
};

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  tekprom();
  tekvrom();
  mira();
 }

 return(ret);
}

int Mapper90_Init(CartInfo *info)
{
  is209=0;
  info->Reset=togglie;
  info->Power=M90Power;
  //PPU_hook = YARGH;
  info->StateAction = StateAction;
  GameHBIRQHook2 = SLWrap; 

  SetWriteHandler(0x5000,0xffff,Mapper90_write);
  SetReadHandler(0x5000,0x5fff,tekread);

  SetReadHandler(0x6000,0xffff,CartBR);

  return(1);
}

int Mapper209_Init(CartInfo *info)
{ 
  is209=1;
  info->Reset=togglie;
  info->Power=M90Power;
  //PPU_hook = YARGH;
  GameHBIRQHook2 = SLWrap; 
  info->StateAction = StateAction;

  SetWriteHandler(0x5000,0xffff,Mapper90_write);
  SetReadHandler(0x5000,0x5fff,tekread);

  SetReadHandler(0x6000,0xffff,CartBR);

  return(1);
} 

