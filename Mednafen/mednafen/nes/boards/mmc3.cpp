/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1998 BERO
 *  Copyright (C) 2003 Xodnizel
 *  Mapper 12 code Copyright (C) 2003 CaH4e3
 *  UNL6035052 code Copyright (C) 2005 CaH4e3
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

/*  Code for emulating iNES mappers 4, 118,119 */

#include "mapinc.h"

static uint8 resetmode,MMC3_cmd,A000B,A001B;
static uint8 DRegBuf[8];

static uint8 *WRAM;
static uint8 *CHRRAM;
static uint32 CHRRAMSize;

static uint8 PPUCHRBus;
static uint8 TKSMIR[8];
static uint8 EXPREGS[8], EXPRCount, cmdin;	/* For bootleg games, mostly. */
static uint8 IRQCount,IRQLatch,IRQa;
static uint8 IRQReload;

static void (*pwrap)(uint32 A, uint8 V);
static void (*cwrap)(uint32 A, uint8 V);
static void (*mwrap)(uint8 V);

static int mmc3opts;

static void GenMMC3Close(void);
static int GenMMC3_Init(CartInfo *info, int prg, int chr, int wram, int battery);
static void GenMMC3Power(CartInfo *info);
static void FixMMC3PRG(int V);
static void FixMMC3CHR(int V);

static void FixMMC3PRG(int V)
{
          if(V&0x40)
          {
           pwrap(0xC000,DRegBuf[6]);
           pwrap(0x8000,~1);
          }
          else
          {
           pwrap(0x8000,DRegBuf[6]);
           pwrap(0xC000,~1);
          }
	  pwrap(0xA000,DRegBuf[7]);
	  pwrap(0xE000,~0);
}

static void FixMMC3CHR(int V)
{
           int cbase=(V&0x80)<<5;
           cwrap((cbase^0x000),DRegBuf[0]&(~1));
           cwrap((cbase^0x400),DRegBuf[0]|1);
           cwrap((cbase^0x800),DRegBuf[1]&(~1));
           cwrap((cbase^0xC00),DRegBuf[1]|1);

           cwrap(cbase^0x1000,DRegBuf[2]);
           cwrap(cbase^0x1400,DRegBuf[3]);
           cwrap(cbase^0x1800,DRegBuf[4]);
           cwrap(cbase^0x1c00,DRegBuf[5]);
}

static void MMC3RegReset(void)
{
 IRQCount=IRQLatch=IRQa=MMC3_cmd=0;

 DRegBuf[0]=0;
 DRegBuf[1]=2;
 DRegBuf[2]=4;
 DRegBuf[3]=5;
 DRegBuf[4]=6;
 DRegBuf[5]=7;
 DRegBuf[6]=0;
 DRegBuf[7]=1;

 FixMMC3PRG(0);
 FixMMC3CHR(0);
}

static void MMC3RegReset(CartInfo *info)
{
 MMC3RegReset();
}

static DECLFW(MMC3_CMDWrite)
{
        switch(A&0xE001)
	{
         case 0x8000:
          if((V&0x40) != (MMC3_cmd&0x40))
	   FixMMC3PRG(V);
          if((V&0x80) != (MMC3_cmd&0x80))
           FixMMC3CHR(V);
          MMC3_cmd = V;
          break;

        case 0x8001:
                {
                 int cbase=(MMC3_cmd&0x80)<<5;
                 DRegBuf[MMC3_cmd&0x7]=V;
                 switch(MMC3_cmd&0x07)
                 {
                  case 0: cwrap((cbase^0x000),V&(~1));
			  cwrap((cbase^0x400),V|1);
			  break;
                  case 1: cwrap((cbase^0x800),V&(~1));
			  cwrap((cbase^0xC00),V|1);
			  break;
                  case 2: cwrap(cbase^0x1000,V); break;
                  case 3: cwrap(cbase^0x1400,V); break;
                  case 4: cwrap(cbase^0x1800,V); break;
                  case 5: cwrap(cbase^0x1C00,V); break;
                  case 6: if (MMC3_cmd&0x40) pwrap(0xC000,V);
                          else pwrap(0x8000,V);
                          break;
                  case 7: pwrap(0xA000,V);
                          break;
                 }
                }
                break;

        case 0xA000:
	        if(mwrap) mwrap(V&1);
                break;
	case 0xA001:
		A001B=V;
		break;
 }
}

static DECLFW(MMC3_IRQWrite)
{
        switch(A&0xE001)
        {
         case 0xc000:IRQLatch=V;break;
         case 0xc001:IRQReload = 1;break;
         case 0xE000:X6502_IRQEnd(MDFN_IQEXT);IRQa=0;break;
         case 0xE001:IRQa=1;break;
        }
}



static void ClockMMC3Counter(void)
{
  int count = IRQCount;
  if(!count || IRQReload)
  {
   IRQCount = IRQLatch;
   IRQReload = 0;
  }
  else IRQCount--;
  if(count && !IRQCount)
  {
   if(IRQa)
   {
    X6502_IRQBegin(MDFN_IQEXT);
   }
  }
}

static void MMC3_hb(void)
{
      ClockMMC3Counter();
}

static void MMC3_hb_KickMasterHack(void)
{
	if(scanline==238)
	 ClockMMC3Counter();
	ClockMMC3Counter();
}

static void MMC3_hb_PALStarWarsHack(void)
{
        if(scanline==240)
         ClockMMC3Counter();
        ClockMMC3Counter();
}
/*
static void ClockMMC6Counter(void)
{
  unsigned int count = IRQCount;

  if(!count || IRQReload)
  {
   IRQCount = IRQLatch;
   IRQReload = 0;
  }
  else IRQCount--;
  if(!IRQCount)
  {
   if(IRQa)
    X6502_IRQBegin(MDFN_IQEXT);
  }
}
*/


/*
static uint32 lasta;
static void MMC3_PPUIRQ(uint32 A)
{
 if(A&0x2000) return;
 if((A&0x1000) && !(lasta&0x1000))
  ClockMMC3Counter();
 lasta = A;
}
*/

static void GENCWRAP(uint32 A, uint8 V)
{
 setchr1(A,V);
}

static void GENPWRAP(uint32 A, uint8 V)
{
 setprg8(A,V&0x3F);
}

static void GENMWRAP(uint8 V)
{
 A000B=V;
 setmirror(V^1);
}

static void GENNOMWRAP(uint8 V)
{
 A000B=V;
}



static void M12CW(uint32 A, uint8 V)
{
 setchr1(A,(EXPREGS[(A&0x1000)>>12]<<8)+V);
}

static DECLFW(M12Write)
{
 EXPREGS[0]= V&0x01;
 EXPREGS[1]= (V&0x10)>>4;
}

static void M12_Power(CartInfo *info)
{
 EXPREGS[0]=EXPREGS[1]=0;
 GenMMC3Power(info);
}

int Mapper12_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery))) return(0);

 SetWriteHandler(0x4100,0x5FFF,M12Write);

 cwrap=M12CW;
 info->Power=M12_Power;
 EXPRCount = 2;

 return(1);
}

static void M47PW(uint32 A, uint8 V)
{
 V&=0xF;
 V|=EXPREGS[0]<<4;
 setprg8(A,V);
}

static void M47CW(uint32 A, uint8 V)
{
 uint32 NV=V;
 NV&=0x7F;
 NV|=EXPREGS[0]<<7;
 setchr1(A,NV);
}

static DECLFW(M47Write)
{
 EXPREGS[0]=V&1;
 FixMMC3PRG(MMC3_cmd);
 FixMMC3CHR(MMC3_cmd); 
}

static void M47_Power(CartInfo *info)
{
 EXPREGS[0]=0;
 GenMMC3Power(info);
}

int Mapper47_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery))) return(0);

 SetWriteHandler(0x6000,0x7FFF,M47Write);
 SetReadHandler(0x6000,0x7FFF,0);

 pwrap=M47PW;
 cwrap=M47CW;
 info->Power=M47_Power;
 EXPRCount = 1;
 return(1);
}

static void M44PW(uint32 A, uint8 V)
{
 uint32 NV=V;
 if(EXPREGS[0]>=6) NV&=0x1F;
 else NV&=0x0F;
 NV|=EXPREGS[0]<<4;
 setprg8(A,NV);
}

static void M44CW(uint32 A, uint8 V)
{
 uint32 NV=V;
 if(EXPREGS[0]<6) NV&=0x7F;
 NV|=EXPREGS[0]<<7;
 setchr1(A,NV);
}

static DECLFW(Mapper44_write)
{
 if(A&1)
 {
  EXPREGS[0]=V&7;
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
 }
 else
  MMC3_CMDWrite(A,V);
}

static void M44_Power(CartInfo *info)
{
 EXPREGS[0]=0;
 GenMMC3Power(info);
}

int Mapper44_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery))) return(0);

 SetWriteHandler(0xA000,0xBFFF,Mapper44_write);

 cwrap=M44CW;
 pwrap=M44PW;
 info->Power=M44_Power;
 EXPRCount = 1;
 return(1);
}

static void M52PW(uint32 A, uint8 V)
{
 uint32 NV=V;
 NV&=0x1F^((EXPREGS[0]&8)<<1);
 NV|=((EXPREGS[0]&6)|((EXPREGS[0]>>3)&EXPREGS[0]&1))<<4;
 setprg8(A,NV);
}

static void M52CW(uint32 A, uint8 V)
{
 uint32 NV=V;
 NV&=0xFF^((EXPREGS[0]&0x40)<<1);
 NV|=(((EXPREGS[0]>>3)&4)|((EXPREGS[0]>>1)&2)|((EXPREGS[0]>>6)&(EXPREGS[0]>>4)&1))<<7;
 setchr1(A,NV);
}

static DECLFW(Mapper52_write)
{
 if(EXPREGS[1]) 
 {
  WRAM[A-0x6000]=V;
  return;
 }
 EXPREGS[1]=1;
 EXPREGS[0]=V;
 FixMMC3PRG(MMC3_cmd);
 FixMMC3CHR(MMC3_cmd);
}

static void M52Reset(CartInfo *info)
{
 EXPREGS[0]=EXPREGS[1]=0;
 MMC3RegReset(info); 
}

static void M52Power(CartInfo *info)
{
 M52Reset(info);
 GenMMC3Power(info);
}

int Mapper52_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery)))
  return(0);

 SetWriteHandler(0x6000,0x7FFF,Mapper52_write);

 cwrap=M52CW;
 pwrap=M52PW;
 info->Reset=M52Reset;
 info->Power=M52Power;
 EXPRCount = 2;
 return(1);
}

static void M45CW(uint32 A, uint8 V)
{
 uint32 NV=V;
 if(EXPREGS[2]&8)
  NV&=(1<<( (EXPREGS[2]&7)+1 ))-1;
 else
  NV&=0;
 NV|=EXPREGS[0]|((EXPREGS[2]&0xF0)<<4); // &0x10(not 0xf0) is valid given the original
					// description of mapper 45 by kevtris,
					// but this fixes Super 8 in 1.
 setchr1(A,NV);
}

static void M45PW(uint32 A, uint8 V)
{
 //V=((V&(EXPREGS[3]^0xFF))&0x3f)|EXPREGS[1];
 V&=(EXPREGS[3]&0x3F)^0x3F;
 V|=EXPREGS[1];
 //printf("$%04x, $%02x\n",A,V);
 setprg8(A,V);
}

static DECLFW(Mapper45_write)
{
 //printf("$%02x, %d\n",V,EXPREGS[4]);
 if(EXPREGS[3]&0x40) 
 {
  WRAM[A-0x6000]=V;   
  return;
 }
 EXPREGS[EXPREGS[4]]=V;
 EXPREGS[4]=(EXPREGS[4]+1)&3;
 FixMMC3PRG(MMC3_cmd);
 FixMMC3CHR(MMC3_cmd);
}

static void M45Reset(CartInfo *info)
{
 memset(EXPREGS,0,5);
 MMC3RegReset(info);
}

static void M45Power(CartInfo *info)
{
 M45Reset(info);
 GenMMC3Power(info);
}

int Mapper45_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery)))
  return(0);

 SetWriteHandler(0x6000,0x7FFF,Mapper45_write);

 cwrap=M45CW;
 pwrap=M45PW;

 info->Reset=M45Reset;
 info->Power=M45Power;

 EXPRCount = 5;
 return(1);
}

static void M49PW(uint32 A, uint8 V)
{
 if(EXPREGS[0]&1)
 {
  V&=0xF;
  V|=(EXPREGS[0]&0xC0)>>2;
  setprg8(A,V);
 }
 else
  setprg32(0x8000,(EXPREGS[0]>>4)&3);
}

static void M49CW(uint32 A, uint8 V)
{
 uint32 NV=V;
 NV&=0x7F;
 NV|=(EXPREGS[0]&0xC0)<<1;
 setchr1(A,NV);
}

static DECLFW(M49Write)
{
 //printf("$%04x:$%02x\n",A,V);
 if(A001B&0x80)
 {
  EXPREGS[0]=V;
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
 }
}

static void M49Reset(CartInfo *info)
{
 EXPREGS[0]=0;
 MMC3RegReset(info);
}

static void M49Power(CartInfo *info)
{
 M49Reset(info);
 GenMMC3Power(info);
}

int Mapper49_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 0, 0)))
  return(0);

 SetWriteHandler(0x6000,0x7FFF,M49Write);
 SetReadHandler(0x6000,0x7FFF,0);

 cwrap=M49CW;
 pwrap=M49PW;
 info->Reset=M49Reset;
 info->Power=M49Power;

 EXPRCount = 1;
 return(1);
}

static void M115PW(uint32 A, uint8 V)
{
 if(EXPREGS[0]&0x80)
  setprg32(0x8000,(EXPREGS[0]&7) >> 1);
 else
  setprg8(A,V);
}

static void M115CW(uint32 A, uint8 V)
{
 setchr1(A,(uint32)V|((EXPREGS[1]&1)<<8));
}

static DECLFW(M115Write)
{
 if(A==0x6000)
  EXPREGS[0]=V;
 else if(A==0x6001)
  EXPREGS[1]=V;
 FixMMC3PRG(MMC3_cmd);
}

static void M115Power(CartInfo *info)
{
 GenMMC3Power(info);
}

int Mapper115_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 512, 0, 0)))
  return(0);

 SetWriteHandler(0x6000,0x7FFF,M115Write);
 SetReadHandler(0x6000,0x7FFF,0);

 cwrap=M115CW;
 pwrap=M115PW;
 info->Power=M115Power;
 EXPRCount = 2;

 return(1);
}

static void M116PW(uint32 A, uint8 V)
{
 // setprg8(A,(V&0xF)|((EXPREGS[1]&0x2)<<3));
 // setprg8(A,(V&7)|(<<3));
 setprg8(A,V);
 if(!(EXPREGS[1]&2))
 {
  setprg16(0x8000,1); //EXPREGS[2]);
 }
}

static void M116CW(uint32 A, uint8 V)
{
 //if(EXPREGS[1]&2)
  setchr1(A,V|((EXPREGS[0]&0x4)<<6));
 //else
 //{
 // setchr1(A,(V&7)|((EXPREGS[2])<<3));
 //}
}

static DECLFW(M116Write)
{
 if(A==0x4100) {EXPREGS[0]=V;setmirror(V&1);}
 else if(A==0x4141) EXPREGS[1]=V;
 else if(A==0xa000) EXPREGS[2]=V;
 else if(A==0x4106) EXPREGS[3]=V;
 FixMMC3PRG(MMC3_cmd);
 FixMMC3CHR(MMC3_cmd);
// MIRROR_SET(0);
 //printf("Wr: $%04x:$%02x, $%04x, %d, %d\n",A,V,X.PC,timestamp,scanline);
}
static DECLFW(M116Write2)
{
 //printf("Wr2: %04x:%02x\n",A,V);
}
static void M116Power(CartInfo *info)
{
 
 EXPREGS[0]=0;
 EXPREGS[1]=2;
 EXPREGS[2]=0;
 EXPREGS[3]=0;

 GenMMC3Power(info);
}

int Mapper116_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 0, 0)))
  return(0);

 SetWriteHandler(0x4020,0x7fff,M116Write);
 SetWriteHandler(0xa000,0xbfff,M116Write2);

 cwrap=M116CW;
 pwrap=M116PW;
 info->Power=M116Power;
 EXPRCount = 4;

 return(1);
}

static void M165CW(uint32 A, uint8 V)
{
 if(V==0)
  setchr4r(0x10,A,0);
 else
  setchr4(A,V>>2);
}

static void M165PPUSync(void)
{
 if(EXPREGS[0] == 0xFD)
  M165CW(0x0000, DRegBuf[0]);
 else
  M165CW(0x0000, DRegBuf[1]);

 if(EXPREGS[0] == 0xFD)
  M165CW(0x1000, DRegBuf[2]);
 else
  M165CW(0x1000, DRegBuf[3]);
}

static void M165PPU(uint32 A, uint8 V)
{
 M165PPUSync();
}

static void ppu165(uint32 VAddr)
{
     uint8 l,h;

     h=VAddr>>8;

     if(h>=0x20 || ((h&0xF)!=0xF))
      return;

     l=VAddr&0xF8;

     if(h<0x10)
     {
      if(l==0xD8)
      {
       EXPREGS[0]=0xFD;
	M165PPUSync();
      }
      else if(l==0xE8)
      {
       EXPREGS[0]=0xFE;
	M165PPUSync();
      }
     }
     else
     {
      if(l==0xD8)
      {
       EXPREGS[1]=0xFD;
	M165PPUSync();
      }
      else if(l==0xE8)
      {
       EXPREGS[1]=0xFE;
	M165PPUSync();
      }
     }
}


int Mapper165_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 128, 8, info->battery)))
  return(0);

 cwrap=M165PPU;
 if(!(CHRRAM = (uint8*)malloc(4096)))
 {
  GenMMC3Close();
  return(0);
 }

 EXPREGS[0]=0xFD;
 EXPREGS[1]=0xFE;
 PPU_hook=ppu165;

 CHRRAMSize = 4096;
 SetupCartCHRMapping(0x10, CHRRAM, 4096, 1);
 EXPRCount = 4;
 return(1);
}

// ---------------------------- Mapper 215 ------------------------------

static const uint8 m215_perm[8] = {0, 2, 5, 3, 6, 1, 7, 4};

static void M215CW(uint32 A, uint8 V)
{
 if(EXPREGS[1]&0x04)
   setchr1(A,V|0x100);
 else
   setchr1(A,(V&0x7F)|((EXPREGS[1]&0x10)<<3));
// setchr1(A,V|((EXPREGS[1]&0x4)<<6));
}

static void M215PW(uint32 A, uint8 V)
{
 if(EXPREGS[1]&0x08)
   setprg8(A,(V&0x1F)|0x20);
 else
   setprg8(A,(V&0x0F)|(EXPREGS[1]&0x10));
}

static DECLFW(M215Write)
{
 if(!(EXPREGS[2]))
 {
  if(A >= 0xc000)
   MMC3_IRQWrite(A, V);
  else
   MMC3_CMDWrite(A,V);
 }
 else switch(A&0xE001)
 {
  case 0xE000: X6502_IRQEnd(MDFN_IQEXT); IRQa=0; break;
  case 0xE001: IRQCount=V; break;
  case 0xC001: IRQa=1; break;
  case 0xC000: MMC3_CMDWrite(0xA000,V|(V>>7)); break;
  case 0xA000: MMC3_CMDWrite(0x8000,(V&0xC0)|(m215_perm[V&7])); cmdin=1; break;
  case 0x8001:
       if(!cmdin||((EXPREGS[0]&0x80)&&(((MMC3_cmd&7)==6)||((MMC3_cmd&7)==7)))) break;
       MMC3_CMDWrite(0x8001,V);
       cmdin=0;
       break;
 }
}

static DECLFW(M215ExWrite)
{
 switch(A)
 {
  case 0x5000:
       EXPREGS[0]=V;
       if(V&0x80)
       {
         setprg16(0x8000,(V&0x0F)|(EXPREGS[1]&0x10));
         setprg16(0xC000,(V&0x0F)|(EXPREGS[1]&0x10));
       }
       else
        FixMMC3PRG(MMC3_cmd);
       break;
  case 0x5001:
//       FCEU_printf("CMD: %04x:%04x\n",A,V);
       EXPREGS[1]=V;
       FixMMC3CHR(MMC3_cmd);
       break;
  case 0x5007:
       EXPREGS[2]=V;
       MMC3RegReset();
       break;
 }
}

static void M215Power(CartInfo *info)
{
 EXPREGS[0]=0;
 EXPREGS[1]=0xFF;
 EXPREGS[2]=4;
 GenMMC3Power(info);
}

int Mapper215_Init(CartInfo *info)
{
 GenMMC3_Init(info, 256, 256, 0, 0);

 SetWriteHandler(0x8000,0xFFFF,M215Write);
 SetWriteHandler(0x5000,0x7FFF,M215ExWrite);

 cwrap=M215CW;
 pwrap=M215PW;
 info->Power=M215Power;
 EXPRCount = 3;

 return(1);
}


// ---------------------------- Mapper 217 ------------------------------

static const uint8 m217_perm[8] = {0, 6, 3, 7, 5, 2, 4, 1};

static void M217CW(uint32 A, uint8 V)
{
 if(EXPREGS[1]&0x08)
   setchr1(A,V|((EXPREGS[1]&3)<<8));
 else
   setchr1(A,(V&0x7F)|((EXPREGS[1]&3)<<8)|((EXPREGS[1]&0x10)<<3));
}

static void M217PW(uint32 A, uint8 V)
{
 if(EXPREGS[1]&0x08)
   setprg8(A,(V&0x1F)|((EXPREGS[1]&3)<<5));
 else
   setprg8(A,(V&0x0F)|((EXPREGS[1]&3)<<5)|(EXPREGS[1]&0x10));
}

static DECLFW(M217Write)
{
 if(!EXPREGS[2])
 {
  if(A >= 0xc000)
   MMC3_IRQWrite(A, V);
  else
   MMC3_CMDWrite(A,V);
 }
 else switch(A&0xE001)
 {
  case 0x8000: IRQCount=V; break;
  case 0xE000: X6502_IRQEnd(MDFN_IQEXT);IRQa=0; break;
  case 0xC001: IRQa=1; break;
  case 0xA001: MMC3_CMDWrite(0xA000,V); break;
  case 0x8001: MMC3_CMDWrite(0x8000,(V&0xC0)|(m217_perm[V&7])); cmdin=1; break;
  case 0xA000:
       if (!cmdin || ((EXPREGS[0]&0x80) && (((MMC3_cmd&7)==6)||((MMC3_cmd&7)==7)))) break;
       MMC3_CMDWrite(0x8001,V);
       cmdin=0;
       break;
 }
}

static DECLFW(M217ExWrite)
{
 switch(A)
 {
  case 0x5000:
       EXPREGS[0]=V;
       if(EXPREGS[0]&0x80)
       {
         setprg16(0x8000,(V&0x0F)|((EXPREGS[1]&3)<<4));
         setprg16(0xC000,(V&0x0F)|((EXPREGS[1]&3)<<4));
       }
       else
         FixMMC3PRG(MMC3_cmd);
       break;
  case 0x5001:
       EXPREGS[1]=V;
       FixMMC3PRG(MMC3_cmd);
       break;
  case 0x5007:
       EXPREGS[2]=V;
       break;
 }
}

static void M217Power(CartInfo *info)
{
 EXPREGS[0]=0;
 EXPREGS[1]=0xFF;
 EXPREGS[2]=3;
 GenMMC3Power(info);
}

int Mapper217_Init(CartInfo *info)
{
 EXPRCount = 3;
 GenMMC3_Init(info, 256, 256, 0, 0);
 cwrap=M217CW;
 pwrap=M217PW;
 info->Power=M217Power;

 SetWriteHandler(0x8000,0xFFFF,M217Write);
 SetWriteHandler(0x5000,0x7FFF,M217ExWrite);

 return(1);
}



static DECLFW(Mapper250_write)
{
	MMC3_CMDWrite((A&0xE000)|((A&0x400)>>10),A&0xFF);
}

static DECLFW(M250_IRQWrite)
{
	MMC3_IRQWrite((A&0xE000)|((A&0x400)>>10),A&0xFF);
}

static void M250_Power(CartInfo *info)
{
	GenMMC3Power(info);
}

int Mapper250_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery)))
  return(0);
 info->Power=M250_Power;

 SetWriteHandler(0x8000,0xBFFF,Mapper250_write);
 SetWriteHandler(0xC000,0xFFFF,M250_IRQWrite);

 return(1);
}

static void M249PW(uint32 A, uint8 V)
{
 if(EXPREGS[0]&0x2)
 {
  if(V<0x20)
   V=(V&1)|((V>>3)&2)|((V>>1)&4)|((V<<2)&8)|((V<<2)&0x10);
  else
  {
   V-=0x20;
   V=(V&3)|((V>>1)&4)|((V>>4)&8)|((V>>2)&0x10)|((V<<3)&0x20)|((V<<2)&0xC0);
  }
 }
 setprg8(A,V);
}

static void M249CW(uint32 A, uint8 V)
{
 if(EXPREGS[0]&0x2)
  V=(V&3)|((V>>1)&4)|((V>>4)&8)|((V>>2)&0x10)|((V<<3)&0x20)|((V<<2)&0xC0);
 setchr1(A,V);
}

static DECLFW(Mapper249_write)
{
 EXPREGS[0]=V;
 FixMMC3PRG(MMC3_cmd);
 FixMMC3CHR(MMC3_cmd);
}

static void M249Power(CartInfo *info)
{
 EXPREGS[0]=0;
 GenMMC3Power(info);
}

int Mapper249_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery)))
  return(0);
 cwrap=M249CW;
 pwrap=M249PW;
 info->Power=M249Power;
 EXPRCount = 1;

 SetWriteHandler(0x5000,0x5000,Mapper249_write);

 return(1);
}

static void M245CW(uint32 A, uint8 V)
{
 //printf("$%04x:$%02x\n",A,V);
 //setchr1(A,V);
 //	
 EXPREGS[0]=V;
 FixMMC3PRG(MMC3_cmd);
}

static void M245PW(uint32 A, uint8 V)
{
 setprg8(A,(V&0x3F)|((EXPREGS[0]&2)<<5));
 //printf("%d\n",(V&0x3F)|((EXPREGS[0]&2)<<5));
}

static void M245Power(CartInfo *info)
{
 EXPREGS[0]=0;
 GenMMC3Power(info);
}

int Mapper245_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery)))
  return(0);

 info->Power=M245Power;
 cwrap=M245CW;
 pwrap=M245PW;
 EXPRCount = 1;

 return(1);
}

void m74p(uint32 a, uint8 v)
{
 setprg8(a,v&0x3f);
}

void m74m(uint8 V)
{
 A000B = V;
 setmirror(V ^ 1);
}

static void m74kie(uint32 a, uint8 v)
{
 if(v == 0x8 || v == 0x9) //((PRGsize[0]>>16)&0x08) || v==1+((PRGsize[0]>>16)&0x08))
  setchr1r(0x10,a, v & 0x1);
 else
  setchr1r(0x0,a,v);
}

static void M74_Power(CartInfo *info)
{
 GenMMC3Power(info);
}

int Mapper74_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery)))
  return(0);
 cwrap=m74kie;
 pwrap=m74p;
 mwrap = m74m;

 info->Power = M74_Power;

 if(!(CHRRAM=(uint8*)malloc(2048)))
 {
  GenMMC3Close();
  return(0);
 }
 CHRRAMSize=2048;
 SetupCartCHRMapping(0x10, CHRRAM, 2048, 1);
 return(1);
}

static void m148kie(uint32 a, uint8 v)
{
 if(v==128 || v==129)
  setchr1r(0x10,a,v);
 else
  setchr1r(0x0,a,v);
}

int Mapper148_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery)))
  return(0);
 cwrap=m148kie;
 pwrap=m74p;
 if(!(CHRRAM=(uint8*)malloc(2048)))
 {
  GenMMC3Close();
  return(0);
 }
 CHRRAMSize=2048;
 SetupCartCHRMapping(0x10, CHRRAM, 2048, 1);
 return(1);
}


static const uint8 UNL6035052_lut[4] = { 0x00, 0x02, 0x02, 0x03 };

static DECLFW(UNL6035052ProtWrite)
{
  EXPREGS[0]=UNL6035052_lut[V&3];
}

static DECLFR(UNL6035052ProtRead)
{
  return EXPREGS[0];
}

static void UNL6035052Power(CartInfo *info)
{
  GenMMC3Power(info);
}

int UNL6035052_Init(CartInfo *info)
{
  if(!GenMMC3_Init(info, 128, 256, 0, 0))
   return(0);

  SetWriteHandler(0x4020,0x7FFF,UNL6035052ProtWrite);
  SetReadHandler(0x4020,0x7FFF,UNL6035052ProtRead);

  info->Power=UNL6035052Power;
  EXPRCount = 1;

  return(1);
}



static void TKSPPU(uint32 A)
{
 //static uint8 z;
 //if(A>=0x2000 || type<0) return;
 //if(type<0) return;
 A&=0x1FFF;
 //if(scanline>=140 && scanline<=200) {setmirror(MI_1);return;}
 //if(scanline>=140 && scanline<=200)
 // if(scanline>=190 && scanline<=200) {setmirror(MI_1);return;}
 // setmirror(MI_1); 
 //printf("$%04x\n",A);

 A>>=10;
 PPUCHRBus=A;
 setmirror(MI_0+TKSMIR[A]);
}

static void TKSWRAP(uint32 A, uint8 V)
{
 TKSMIR[A>>10]=V>>7;
 setchr1(A,V&0x7F);
 if(PPUCHRBus==(A>>10))
  setmirror(MI_0+(V>>7));
}


static void TQWRAP(uint32 A, uint8 V)
{
 setchr1r((V&0x40)>>2,A,V&0x3F);
}

static int wrams;

static DECLFW(MBWRAM)
{
  WRAM[A-0x6000]=V;
}

static DECLFR(MAWRAM)
{
 return(WRAM[A-0x6000]);
}

static DECLFW(MBWRAMMMC6)
{
 WRAM[A&0x3ff]=V;
}

static DECLFR(MAWRAMMMC6)
{
 return(WRAM[A&0x3ff]);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(WRAM, wrams),
  SFARRAY(CHRRAM, CHRRAMSize),
  SFARRAYN(DRegBuf, 8, "REGS"),
  SFVARN(resetmode, "RMOD"),
  SFVARN(MMC3_cmd, "CMD"),
  SFVARN(A000B, "A000"),
  SFVARN(A001B, "A001"),
  SFVARN(IRQReload, "IRQR"),
  SFVARN(IRQCount, "IRQC"),
  SFVARN(IRQLatch, "IRQL"),
  SFVARN(IRQa, "IRQA"),
  SFVAR(PPUCHRBus),
  SFARRAY(EXPREGS, EXPRCount),
  SFVAR(cmdin),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  mwrap(A000B&1);
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
 }
 return(ret);
}

static void GenMMC3Power(CartInfo *info)
{
 A001B=A000B=0;
 setmirror((A000B&1)^1);
 if(mmc3opts&1)
 {
  if(!(mmc3opts&2))
   memset(WRAM, 0x00, wrams);
 }
 MMC3RegReset(info);
 if(CHRRAM)
  memset(CHRRAM,0,CHRRAMSize);
}

static void GenMMC3Close(void)
{
 if(CHRRAM)
  MDFN_free(CHRRAM);
 if(WRAM)
  MDFN_free(WRAM);
 CHRRAM=WRAM=NULL;
}

#ifdef WANT_DEBUGGER
enum
{
 MMC3_GSREG_CONTROL = 0,
 MMC3_GSREG_MIRRORING,
 MMC3_GSREG_WRAMPROTECT,
 MMC3_GSREG_CHRBANK0,
 MMC3_GSREG_CHRBANK1,
 MMC3_GSREG_CHRBANK2,
 MMC3_GSREG_CHRBANK3,
 MMC3_GSREG_CHRBANK4,
 MMC3_GSREG_CHRBANK5,
 MMC3_GSREG_PRGBANK0,
 MMC3_GSREG_PRGBANK1,
 MMC3_GSREG_IRQCOUNT,
 MMC3_GSREG_IRQLATCH,
 MMC3_GSREG_IRQENABLE
};

static RegType DBGMMC3Regs[] =
{
 { MMC3_GSREG_CONTROL, "Control", "Command/Control", 1 },
 { MMC3_GSREG_MIRRORING, "Mirroring", "Mirroring", 1 },
 { MMC3_GSREG_WRAMPROTECT, "WRAMProtect", "Cart WRAM Protect", 1 },
 { MMC3_GSREG_CHRBANK0, "CHRBank0", "CHR Bank(2KiB) Register 0", 1 },
 { MMC3_GSREG_CHRBANK1, "CHRBank1", "CHR Bank(2KiB) Register 1", 1 },
 { MMC3_GSREG_CHRBANK2, "CHRBank2", "CHR Bank(1KiB) Register 2", 1 },
 { MMC3_GSREG_CHRBANK3, "CHRBank3", "CHR Bank(1KiB) Register 3", 1 },
 { MMC3_GSREG_CHRBANK4, "CHRBank4", "CHR Bank(1KiB) Register 4", 1 },
 { MMC3_GSREG_CHRBANK5, "CHRBank5", "CHR Bank(1KiB) Register 5", 1 },
 { MMC3_GSREG_PRGBANK0, "PRGBank0", "PRG Bank(8KiB, @8000 or C000) Register 0", 1 },
 { MMC3_GSREG_PRGBANK1, "PRGBank1", "PRG Bank(8KiB, @A000) Register 1", 1 },

 { MMC3_GSREG_IRQCOUNT, "IRQCount", "IRQ Counter", 1 },
 { MMC3_GSREG_IRQLATCH, "IRQLatch", "IRQ Counter Latch", 1 },
 { MMC3_GSREG_IRQENABLE, "IRQEnable", "IRQ Counter Enable", 1 },

 { 0, "", "", 0 },
};

static uint32 MMC3DBG_GetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 uint32 value = 0xDEADBEEF;

 switch(id)
 {
  case MMC3_GSREG_CONTROL:
	value = MMC3_cmd;
	break;

  case MMC3_GSREG_CHRBANK0:
	value = DRegBuf[0];
	break;

  case MMC3_GSREG_CHRBANK1:
        value = DRegBuf[1];
	break;

  case MMC3_GSREG_CHRBANK2:
        value = DRegBuf[2];
	break;

  case MMC3_GSREG_CHRBANK3:
        value = DRegBuf[3];
	break;

  case MMC3_GSREG_CHRBANK4:
        value = DRegBuf[4];
	break;

  case MMC3_GSREG_CHRBANK5:
        value = DRegBuf[5];
	break;

  case MMC3_GSREG_PRGBANK0:
        value = DRegBuf[6];
        break;

  case MMC3_GSREG_PRGBANK1:
        value = DRegBuf[7];
        break;

  case MMC3_GSREG_IRQCOUNT:
	value = IRQCount;
	break;

  case MMC3_GSREG_IRQLATCH:
	value = IRQLatch;
	break;

  case MMC3_GSREG_IRQENABLE:
	value = IRQa;
	break;

  case MMC3_GSREG_MIRRORING:
	value = A000B;
	break;

  case MMC3_GSREG_WRAMPROTECT:
	value = A001B;
	break;
 }
 return(value);
}

static void MMC3DBG_SetRegister(const unsigned int id, uint32 value)
{
 switch(id)
 {
  case MMC3_GSREG_CONTROL:
        MMC3_cmd = value;
        break;

  case MMC3_GSREG_CHRBANK0:
        DRegBuf[0] = value;
        break;

  case MMC3_GSREG_CHRBANK1:
        DRegBuf[1] = value;
        break;

  case MMC3_GSREG_CHRBANK2:
        DRegBuf[2] = value;
        break;

  case MMC3_GSREG_CHRBANK3:
        DRegBuf[3] = value;
        break;

  case MMC3_GSREG_CHRBANK4:
        DRegBuf[4] = value;
        break;

  case MMC3_GSREG_CHRBANK5:
        DRegBuf[5] = value;
        break;

  case MMC3_GSREG_PRGBANK0:
        DRegBuf[6] = value;
        break;

  case MMC3_GSREG_PRGBANK1:
        DRegBuf[7] = value;
        break;

  case MMC3_GSREG_IRQCOUNT:
        IRQCount = value;
        break;

  case MMC3_GSREG_IRQLATCH:
        IRQLatch = value;
        break;

  case MMC3_GSREG_IRQENABLE:
        IRQa = value;
        break;

  case MMC3_GSREG_MIRRORING:
        A000B = value;
	mwrap(A000B&1);
        break;

  case MMC3_GSREG_WRAMPROTECT:
        A001B = value;
        break;
 }

 FixMMC3PRG(MMC3_cmd);
 FixMMC3CHR(MMC3_cmd);
}

static RegGroupType DBGMMC3RegsGroup =
{
 "MMC3",
 DBGMMC3Regs,
 MMC3DBG_GetRegister,
 MMC3DBG_SetRegister,
};

#endif

static int GenMMC3_Init(CartInfo *info, int prg, int chr, int wram, int battery)
{
 mmc3opts = 0;

 pwrap=GENPWRAP;
 cwrap=GENCWRAP;
 mwrap=GENMWRAP;

 wrams=wram*1024;

 PRGmask8[0]&=(prg>>13)-1;
 CHRmask1[0]&=(chr>>10)-1;
 CHRmask2[0]&=(chr>>11)-1;

 if(wram)
 {
  mmc3opts|=1;
  if(!(WRAM=(uint8*)malloc(wram*1024)))
  {
   GenMMC3Close();
   return(0);
  }
 }

 if(battery)
 {
  mmc3opts|=2;

  info->SaveGame[0]=WRAM;
  info->SaveGameLen[0]=wram*1024;
 }

 if(!chr)
 {
  if(!(CHRRAM=(uint8*)malloc(8192)))
  {
   GenMMC3Close();
   return(0);
  }
  CHRRAMSize=8192;
  SetupCartCHRMapping(0, CHRRAM, 8192, 1);
 }

 info->StateAction = StateAction;
 info->Power=GenMMC3Power;
 info->Reset=MMC3RegReset;
 info->Close=GenMMC3Close;

 //PPU_hook=MMC3_PPUIRQ;

 if(info->CRC32 == 0x5104833e)		// Kick Master
  GameHBIRQHook = MMC3_hb_KickMasterHack;
 else if(info->CRC32 == 0x5a6860f1 || info->CRC32 == 0xae280e20) // Shougi Meikan '92/'93
  GameHBIRQHook = MMC3_hb_KickMasterHack;
 else if(info->CRC32 == 0xfcd772eb)	// PAL Star Wars, similar problem as Kick Master.
  GameHBIRQHook = MMC3_hb_PALStarWarsHack;
 else 
  GameHBIRQHook=MMC3_hb;

 #ifdef WANT_DEBUGGER
 MDFNDBG_AddRegGroup(&DBGMMC3RegsGroup);
 #endif

 SetWriteHandler(0x8000,0xBFFF,MMC3_CMDWrite);
 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0xC000,0xFFFF,MMC3_IRQWrite);

 if(mmc3opts&1)
 {
  if(wrams==1024)
  {
   MDFNMP_AddRAM(1024,0x7000,WRAM);
   SetReadHandler(0x7000,0x7FFF,MAWRAMMMC6);
   SetWriteHandler(0x7000,0x7FFF,MBWRAMMMC6);
  }
  else
  {
   MDFNMP_AddRAM(wrams, 0x6000, WRAM);
   SetReadHandler(0x6000,0x6000+wrams-1,MAWRAM);
   SetWriteHandler(0x6000,0x6000+wrams-1,MBWRAM);
  }
 }

 return(1);
}

static int hackm4;		/* For Karnov, maybe others.  BLAH.  Stupid iNES format.*/

static void Mapper4Power(CartInfo *info)
{
 GenMMC3Power(info);
 A000B=(hackm4^1)&1;
 setmirror(hackm4);
}

int Mapper4_Init(CartInfo *info)
{
 int ws=8;

 if((info->CRC32==0x93991433 || info->CRC32==0xaf65aa84))
 {
  MDFN_printf(_("Low-G-Man can not work normally in the iNES format.\nThis game has been recognized by its CRC32 value, and the appropriate changes will be made so it will run.\nIf you wish to hack this game, you should use the UNIF format for your hack.\n\n"));
  ws=0;
 }
 if(!(GenMMC3_Init(info,512,256,ws,info->battery)))
  return(0);
 info->Power=Mapper4Power;
 hackm4=info->mirror;
 return(1);
}

int Mapper118_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery)))
  return(0);
 cwrap=TKSWRAP;   
 mwrap=GENNOMWRAP;
 PPU_hook=TKSPPU;
 return(1);
}

int Mapper119_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 64, 0, 0)))
  return(0);
 cwrap=TQWRAP;
 if(!(CHRRAM=(uint8*)malloc(8192)))
 {
  GenMMC3Close();
  return(0);
 }
 CHRRAMSize=8192;
 SetupCartCHRMapping(0x10, CHRRAM, 8192, 1);
 return(1);
}

int TEROM_Init(CartInfo *info)
{
 return(GenMMC3_Init(info, 32, 32, 0, 0));
}

int TFROM_Init(CartInfo *info)
{
 return(GenMMC3_Init(info, 512, 64, 0, 0));
}

int TGROM_Init(CartInfo *info)
{
 return(GenMMC3_Init(info, 512, 0, 0, 0));
}

int TKROM_Init(CartInfo *info)
{
 return(GenMMC3_Init(info, 512, 256, 8, info->battery));
}

int TLROM_Init(CartInfo *info)
{
 return(GenMMC3_Init(info, 512, 256, 0, 0));
}

int TSROM_Init(CartInfo *info)
{
 return(GenMMC3_Init(info, 512, 256, 8, 0));
}

int TLSROM_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, 0)))
  return(0);

 cwrap=TKSWRAP;
 mwrap=GENNOMWRAP;
 PPU_hook=TKSPPU;
 return(1);
}

int TKSROM_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 256, 8, info->battery)))
  return(0);

 cwrap=TKSWRAP;
 mwrap=GENNOMWRAP;
 PPU_hook=TKSPPU;
 return(1);
}

int TQROM_Init(CartInfo *info)
{
 if(!(GenMMC3_Init(info, 512, 64, 0, 0)))
  return(0);
 cwrap=TQWRAP;
 if(!(CHRRAM=(uint8*)malloc(8192)))
 {
  GenMMC3Close();
  return(0);
 }
 CHRRAMSize=8192;
 SetupCartCHRMapping(0x10, CHRRAM, 8192, 1);

 return(0);
}

/* MMC6 board */
int HKROM_Init(CartInfo *info)
{
 return(GenMMC3_Init(info, 512, 512, 1, info->battery));
}
