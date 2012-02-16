/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1998 BERO
 *  Copyright (C) 2003 Xodnizel
 *  Mapper 12 code Copyright (C) 2003 CaH4e3
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

/*  Code for emulating iNES mappers 4,12,44,45,47,49,52,74,114,115,116,118,
    119,165,205,245,249,250,254
*/

#include "mapinc.h"
#include "mmc3.h"

uint8 MMC3_cmd;
uint8 *WRAM;
uint8 *CHRRAM;
uint32 CHRRAMSize;
uint8 DRegBuf[8];
uint8 EXPREGS[8];    /* For bootleg games, mostly. */
uint8 A000B,A001B;

#undef IRQCount
#undef IRQLatch
#undef IRQa
uint8 IRQCount,IRQLatch,IRQa;
uint8 IRQReload;

static SFORMAT MMC3_StateRegs[]=
{
 {DRegBuf, 8, "REGS"},
 {&MMC3_cmd, 1, "CMD"},
 {&A000B, 1, "A000"},
 {&A001B, 1, "A001"},
 {&IRQReload, 1, "IRQR"},
 {&IRQCount, 1, "IRQC"},
 {&IRQLatch, 1, "IRQL"},
 {&IRQa, 1, "IRQA"},
 {0}
};

static int mmc3opts=0;
static int wrams;
static int isRevB=1;

void (*pwrap)(uint32 A, uint8 V);
void (*cwrap)(uint32 A, uint8 V);
void (*mwrap)(uint8 V);

void GenMMC3Power(void);
void FixMMC3PRG(int V);
void FixMMC3CHR(int V);

void GenMMC3_Init(CartInfo *info, int prg, int chr, int wram, int battery);

/* ----------------------------------------------------------------------*/
/* ------------------------- Generic MM3 Code ---------------------------*/
/* ----------------------------------------------------------------------*/

void FixMMC3PRG(int V)
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

void FixMMC3CHR(int V)
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

 if(mwrap) mwrap(A000B);
}

void MMC3RegReset(void)
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

DECLFW(MMC3_CMDWrite)
{
/* FCEU_printf("bs %04x %02x\n",A,V);*/
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
         case 2: cwrap(cbase^0x1000,V);
                 break;
         case 3: cwrap(cbase^0x1400,V);
                 break;
         case 4: cwrap(cbase^0x1800,V);
                 break;
         case 5: cwrap(cbase^0x1C00,V);
                 break;
         case 6:
                 if(MMC3_cmd&0x40)
                    pwrap(0xC000,V);
                 else
                    pwrap(0x8000,V);
                 break;
         case 7:
                 pwrap(0xA000,V);
                 break;
        }
       }
       break;
  case 0xA000:
       if(mwrap) mwrap(V);
       break;
  case 0xA001:
       A001B=V;
/*       setprg8r(0x10,0x6000,A001B&3);*/
       break;
 }
}

DECLFW(MMC3_IRQWrite)
{
/* FCEU_printf("%04x:%04x\n",A,V);*/
 switch(A&0xE001)
 {
  case 0xC000:IRQLatch=V;break;
  case 0xC001:IRQReload=1;break;
  case 0xE000:X6502_IRQEnd(FCEU_IQEXT);IRQa=0;break;
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
 else
    IRQCount--;
 if((count|isRevB) && !IRQCount)
 {
    if(IRQa)
    {
       X6502_IRQBegin(FCEU_IQEXT);
    }
 }
}

static void MMC3_hb(void)
{
 ClockMMC3Counter();
}

static void MMC3_hb_KickMasterHack(void)
{
 if(scanline==238) ClockMMC3Counter();
 ClockMMC3Counter();
}

static void MMC3_hb_PALStarWarsHack(void)
{
 if(scanline==240) ClockMMC3Counter();
 ClockMMC3Counter();
}

void GenMMC3Restore(int version)
{
 FixMMC3PRG(MMC3_cmd);
 FixMMC3CHR(MMC3_cmd);
}

static void GENCWRAP(uint32 A, uint8 V)
{
   setchr1(A,V);    /* Business Wars NEEDS THIS for 8K CHR-RAM*/
}

static void GENPWRAP(uint32 A, uint8 V)
{
 setprg8(A,V&0x7F); /* [NJ102] Mo Dao Jie (C) has 1024Mb MMC3 BOARD, maybe something other will be broken*/
}

static void GENMWRAP(uint8 V)
{
 A000B=V;
 setmirror((V&1)^1);
}

static void GENNOMWRAP(uint8 V)
{
 A000B=V;
}

static DECLFW(MBWRAMMMC6)
{
 WRAM[A&0x3ff]=V;
}

static DECLFR(MAWRAMMMC6)
{
 return(WRAM[A&0x3ff]);
}

void GenMMC3Power(void)
{
 if(UNIFchrrama) setchr8(0);

 SetWriteHandler(0x8000,0xBFFF,MMC3_CMDWrite);
 SetWriteHandler(0xC000,0xFFFF,MMC3_IRQWrite);
 SetReadHandler(0x8000,0xFFFF,CartBR);
 A001B=A000B=0;
 setmirror(1);
 if(mmc3opts&1)
 {
  if(wrams==1024)
  {
   FCEU_CheatAddRAM(1,0x7000,WRAM);
   SetReadHandler(0x7000,0x7FFF,MAWRAMMMC6);
   SetWriteHandler(0x7000,0x7FFF,MBWRAMMMC6);
  }
  else
  {
   FCEU_CheatAddRAM((wrams&0x1fff)>>10,0x6000,WRAM);
   SetWriteHandler(0x6000,0x6000 + ((wrams - 1) & 0x1fff),CartBW);
   SetReadHandler(0x6000,0x6000 + ((wrams - 1) & 0x1fff),CartBR);
   setprg8r(0x10,0x6000,0);
  }
  if(!(mmc3opts&2))
     FCEU_dwmemset(WRAM,0,wrams);
 }
 MMC3RegReset();
 if(CHRRAM)
  FCEU_dwmemset(CHRRAM,0,CHRRAMSize);
}

static void GenMMC3Close(void)
{
 if(CHRRAM)
    free(CHRRAM);
 if(WRAM)
    free(WRAM);
 CHRRAM=WRAM=NULL;
}

void GenMMC3_Init(CartInfo *info, int prg, int chr, int wram, int battery)
{
 pwrap=GENPWRAP;
 cwrap=GENCWRAP;
 mwrap=GENMWRAP;

 wrams=wram<<10;

 PRGmask8[0]&=(prg>>13)-1;
 CHRmask1[0]&=(chr>>10)-1;
 CHRmask2[0]&=(chr>>11)-1;

 if(wram)
 {
  mmc3opts|=1;
  WRAM=(uint8*)FCEU_gmalloc(wrams);
  SetupCartPRGMapping(0x10,WRAM,wrams,1);
  AddExState(WRAM, wrams, 0, "MMC3WRAM");
 }

 if(battery)
 {
  mmc3opts|=2;
  info->SaveGame[0]=WRAM;
  info->SaveGameLen[0]=wrams;
 }

 AddExState(MMC3_StateRegs, ~0, 0, 0);

 info->Power=GenMMC3Power;
 info->Reset=MMC3RegReset;
 info->Close=GenMMC3Close;

 if(info->CRC32 == 0x5104833e)        /* Kick Master*/
  GameHBIRQHook = MMC3_hb_KickMasterHack;
 else if(info->CRC32 == 0x5a6860f1 || info->CRC32 == 0xae280e20) /* Shougi Meikan '92/'93*/
  GameHBIRQHook = MMC3_hb_KickMasterHack;
 else if(info->CRC32 == 0xfcd772eb)    /* PAL Star Wars, similar problem as Kick Master.*/
  GameHBIRQHook = MMC3_hb_PALStarWarsHack;
 else
  GameHBIRQHook=MMC3_hb;
 GameStateRestore=GenMMC3Restore;
}

/* ----------------------------------------------------------------------*/
/* -------------------------- MMC3 Based Code ---------------------------*/
/* ----------------------------------------------------------------------*/

/* ---------------------------- Mapper 4 --------------------------------*/

static int hackm4=0;/* For Karnov, maybe others.  BLAH.  Stupid iNES format.*/

static void M4Power(void)
{
 GenMMC3Power();
 A000B=(hackm4^1)&1;
 setmirror(hackm4);
}

void Mapper4_Init(CartInfo *info)
{
 int ws=8;

 if((info->CRC32==0x93991433 || info->CRC32==0xaf65aa84))
 {
  FCEU_printf("Low-G-Man can not work normally in the iNES format.\nThis game has been recognized by its CRC32 value, and the appropriate changes will be made so it will run.\nIf you wish to hack this game, you should use the UNIF format for your hack.\n\n");
  ws=0;
 }
 GenMMC3_Init(info,512,256,ws,info->battery);
 info->Power=M4Power;
 hackm4=info->mirror;
}

/* ---------------------------- Mapper 12 -------------------------------*/

static void M12CW(uint32 A, uint8 V)
{
 setchr1(A,(EXPREGS[(A&0x1000)>>12]<<8)+V);
}

static DECLFW(M12Write)
{
 EXPREGS[0]=V&0x01;
 EXPREGS[1]=(V&0x10)>>4;
}

static void M12Power(void)
{
 EXPREGS[0]=EXPREGS[1]=0;
 GenMMC3Power();
 SetWriteHandler(0x4100,0x5FFF,M12Write);
}

void Mapper12_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 cwrap=M12CW;
 info->Power=M12Power;
 AddExState(EXPREGS, 2, 0, "EXPR");
}

/* ---------------------------- Mapper 37 -------------------------------*/

static void M37PW(uint32 A, uint8 V)
{
  if(EXPREGS[0]!=2)
    V&=0x7;
  else
    V&=0xF;
  V|=EXPREGS[0]<<3;
  setprg8(A,V);
}

static void M37CW(uint32 A, uint8 V)
{
  uint32 NV=V;
  NV&=0x7F;
  NV|=EXPREGS[0]<<6;
  setchr1(A,NV);
}

static DECLFW(M37Write)
{
  EXPREGS[0]=(V&6)>>1;
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
}

static void M37Reset(void)
{
  EXPREGS[0]=0;
  MMC3RegReset();
}

static void M37Power(void)
{
  EXPREGS[0]=0;
  GenMMC3Power();
  SetWriteHandler(0x6000,0x7FFF,M37Write);
}

void Mapper37_Init(CartInfo *info)
{
  GenMMC3_Init(info, 512, 256, 8, info->battery);
  pwrap=M37PW;
  cwrap=M37CW;
  info->Power=M37Power;
  info->Reset=M37Reset;
  AddExState(EXPREGS, 1, 0, "EXPR");
}

/* ---------------------------- Mapper 44 -------------------------------*/

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

static DECLFW(M44Write)
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

static void M44Power(void)
{
 EXPREGS[0]=0;
 GenMMC3Power();
 SetWriteHandler(0xA000,0xBFFF,M44Write);
}

void Mapper44_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 cwrap=M44CW;
 pwrap=M44PW;
 info->Power=M44Power;
 AddExState(EXPREGS, 1, 0, "EXPR");
}

/* ---------------------------- Mapper 45 -------------------------------*/

static void M45CW(uint32 A, uint8 V)
{
 if(!UNIFchrrama)
 {
   uint32 NV=V;
   if(EXPREGS[2]&8)
      NV&=(1<<((EXPREGS[2]&7)+1))-1;
   else
      if(EXPREGS[2])
         NV&=0; /* hack ;( don't know exactly how it should be*/
   NV|=EXPREGS[0]|((EXPREGS[2]&0xF0)<<4);
   setchr1(A,NV);
 }
}

static void M45PW(uint32 A, uint8 V)
{
 V&=(EXPREGS[3]&0x3F)^0x3F;
 V|=EXPREGS[1];
 setprg8(A,V);
}

static DECLFW(M45Write)
{
 if(EXPREGS[3]&0x40)
 {
  WRAM[A-0x6000]=V;
  return;
 }
 EXPREGS[EXPREGS[4]]=V;
 EXPREGS[4]=(EXPREGS[4]+1)&3;
/* if(!EXPREGS[4]) */
/* {*/
/*   FCEU_printf("CHROR %02x, PRGOR %02x, CHRAND %02x, PRGAND %02x\n",EXPREGS[0],EXPREGS[1],EXPREGS[2],EXPREGS[3]);*/
/*   FCEU_printf("CHR0 %03x, CHR1 %03x, PRG0 %03x, PRG1 %03x\n",*/
/*               (0x00&((1<<((EXPREGS[2]&7)+1))-1))|(EXPREGS[0]|((EXPREGS[2]&0xF0)<<4)),*/
/*               (0xFF&((1<<((EXPREGS[2]&7)+1))-1))|(EXPREGS[0]|((EXPREGS[2]&0xF0)<<4)),*/
/*               (0x00&((EXPREGS[3]&0x3F)^0x3F))|(EXPREGS[1]),*/
/*               (0xFF&((EXPREGS[3]&0x3F)^0x3F))|(EXPREGS[1]));*/
/* }*/
 FixMMC3PRG(MMC3_cmd);
 FixMMC3CHR(MMC3_cmd);
}

static DECLFR(M45Read)
{
  uint32 addr = 1<<(EXPREGS[5]+4);     
  if(A&(addr|(addr-1)))
    return X.DB | 1;
  else
    return X.DB;
}
       
static void M45Reset(void)
{
 EXPREGS[0]=EXPREGS[1]=EXPREGS[2]=EXPREGS[3]=EXPREGS[4]=0;
 EXPREGS[5]++;
 EXPREGS[5] &= 7;
 MMC3RegReset();
}

static void M45Power(void)
{
 setchr8(0);
 GenMMC3Power();
 EXPREGS[0]=EXPREGS[1]=EXPREGS[2]=EXPREGS[3]=EXPREGS[4]=EXPREGS[5]=0;
 SetWriteHandler(0x5000,0x7FFF,M45Write);
 SetReadHandler(0x5000,0x5FFF,M45Read);
}

void Mapper45_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 cwrap=M45CW;
 pwrap=M45PW;
 info->Reset=M45Reset;
 info->Power=M45Power;
 AddExState(EXPREGS, 5, 0, "EXPR");
}

/* ---------------------------- Mapper 47 -------------------------------*/

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

static void M47Power(void)
{
 EXPREGS[0]=0;
 GenMMC3Power();
 SetWriteHandler(0x6000,0x7FFF,M47Write);
/* SetReadHandler(0x6000,0x7FFF,0);*/
}

void Mapper47_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, 0);
 pwrap=M47PW;
 cwrap=M47CW;
 info->Power=M47Power;
 AddExState(EXPREGS, 1, 0, "EXPR");
}

/* ---------------------------- Mapper 49 -------------------------------*/

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
 if(A001B&0x80)
 {
  EXPREGS[0]=V;
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
 }
}

static void M49Reset(void)
{
 EXPREGS[0]=0;
 MMC3RegReset();
}

static void M49Power(void)
{
 M49Reset();
 GenMMC3Power();
 SetWriteHandler(0x6000,0x7FFF,M49Write);
 SetReadHandler(0x6000,0x7FFF,0);
}

void Mapper49_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 0, 0);
 cwrap=M49CW;
 pwrap=M49PW;
 info->Reset=M49Reset;
 info->Power=M49Power;
 AddExState(EXPREGS, 1, 0, "EXPR");
}

/* ---------------------------- Mapper 52 -------------------------------*/

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

static DECLFW(M52Write)
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

static void M52Reset(void)
{
 EXPREGS[0]=EXPREGS[1]=0;
 MMC3RegReset();
}

static void M52Power(void)
{
 M52Reset();
 GenMMC3Power();
 SetWriteHandler(0x6000,0x7FFF,M52Write);
}

void Mapper52_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 cwrap=M52CW;
 pwrap=M52PW;
 info->Reset=M52Reset;
 info->Power=M52Power;
 AddExState(EXPREGS, 2, 0, "EXPR");
}

/* ---------------------------- Mapper 74 -------------------------------*/

static void M74CW(uint32 A, uint8 V)
{
  if((V==8)||(V==9)) /*Di 4 Ci - Ji Qi Ren Dai Zhan (As).nes, Ji Jia Zhan Shi (As).nes*/
    setchr1r(0x10,A,V);
  else
    setchr1r(0,A,V);
}

void Mapper74_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 cwrap=M74CW;
 CHRRAMSize=2048;
 CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSize);
 SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSize, 1);
 AddExState(CHRRAM, CHRRAMSize, 0, "CHRR");
}

/* ---------------------------- Mapper 114 ------------------------------*/

static uint8 cmdin;
uint8 m114_perm[8] = {0, 3, 1, 5, 6, 7, 2, 4};

static void M114PWRAP(uint32 A, uint8 V)
{
  if(EXPREGS[0]&0x80)
  {
/*    FCEU_printf("8000-C000:%02X\n",EXPREGS[0]&0xF);*/
    setprg16(0x8000,EXPREGS[0]&0xF);
    setprg16(0xC000,EXPREGS[0]&0xF);
  }
  else {
/*    FCEU_printf("%04X:%02X\n",A,V&0x3F);*/
    setprg8(A,V&0x3F);
  }
}

static DECLFW(M114Write)
{
  switch(A&0xE001)
  {
   case 0x8001: MMC3_CMDWrite(0xA000,V); break;
   case 0xA000: MMC3_CMDWrite(0x8000,(V&0xC0)|(m114_perm[V&7])); cmdin=1; break;
   case 0xC000: if(!cmdin) break; MMC3_CMDWrite(0x8001,V); cmdin=0; break;
   case 0xA001: IRQLatch=V; break;
   case 0xC001: IRQReload=1; break;
   case 0xE000: X6502_IRQEnd(FCEU_IQEXT);IRQa=0; break;
   case 0xE001: IRQa=1; break;
  }
}

static DECLFW(M114ExWrite)
{
  if(A<=0x7FFF)
  {
    EXPREGS[0]=V;
    FixMMC3PRG(MMC3_cmd);
  }
}

static void M114Power(void)
{
  GenMMC3Power();
  SetWriteHandler(0x8000,0xFFFF,M114Write);
  SetWriteHandler(0x5000,0x7FFF,M114ExWrite);
}

static void M114Reset(void)
{
  EXPREGS[0]=0;
  MMC3RegReset();
}

void Mapper114_Init(CartInfo *info)
{
  isRevB=0;
  GenMMC3_Init(info, 256, 256, 0, 0);
  pwrap=M114PWRAP;
  info->Power=M114Power;
  info->Reset=M114Reset;
  AddExState(EXPREGS, 1, 0, "EXPR");
  AddExState(&cmdin, 1, 0, "CMDIN");
}

/* ---------------------------- Mapper 115 KN-658 board ------------------------------*/

static void M115PW(uint32 A, uint8 V)
{
  if(EXPREGS[0]&0x80)
    setprg32(0x8000,(EXPREGS[0]&7)>>1);
  else
    setprg8(A,V);
}

static void M115CW(uint32 A, uint8 V)
{
  setchr1(A,(uint32)V|((EXPREGS[1]&1)<<8));
}

static DECLFW(M115Write)
{
/* FCEU_printf("%04x:%04x\n",A,V);*/
 if(A==0x5080) EXPREGS[2]=V;
 if(A==0x6000)
    EXPREGS[0]=V;
 else if(A==0x6001)
    EXPREGS[1]=V;
 FixMMC3PRG(MMC3_cmd);
}

static DECLFR(M115Read)
{
 return EXPREGS[2];
}

static void M115Power(void)
{
 GenMMC3Power();
 SetWriteHandler(0x4100,0x7FFF,M115Write);
 SetReadHandler(0x5000,0x5FFF,M115Read);
}

void Mapper115_Init(CartInfo *info)
{
 GenMMC3_Init(info, 128, 512, 0, 0);
 cwrap=M115CW;
 pwrap=M115PW;
 info->Power=M115Power;
 AddExState(EXPREGS, 2, 0, "EXPR");
}

/* ---------------------------- Mapper 118 ------------------------------*/

static uint8 PPUCHRBus;
static uint8 TKSMIR[8];

static void TKSPPU(uint32 A)
{
 A&=0x1FFF;
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

/* ---------------------------- Mapper 119 ------------------------------*/

static void TQWRAP(uint32 A, uint8 V)
{
 setchr1r((V&0x40)>>2,A,V&0x3F);
}

void Mapper119_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 64, 0, 0);
 cwrap=TQWRAP;
 CHRRAMSize=8192;
 CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSize);
 SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSize, 1);
}

/* ---------------------------- Mapper 134 ------------------------------*/

static void M134PW(uint32 A, uint8 V)
{
  setprg8(A,(V&0x1F)|((EXPREGS[0]&2)<<4));
}

static void M134CW(uint32 A, uint8 V)
{
  setchr1(A,(V&0xFF)|((EXPREGS[0]&0x20)<<3));
}

static DECLFW(M134Write)
{
  EXPREGS[0]=V;
  FixMMC3CHR(MMC3_cmd);
  FixMMC3PRG(MMC3_cmd);
}

static void M134Power(void)
{
 EXPREGS[0]=0;
 GenMMC3Power();
 SetWriteHandler(0x6001,0x6001,M134Write);
}

static void M134Reset(void)
{
 EXPREGS[0]=0;
 MMC3RegReset();
}

void Mapper134_Init(CartInfo *info)
{
 GenMMC3_Init(info, 256, 256, 0, 0);
 pwrap=M134PW;
 cwrap=M134CW;
 info->Power=M134Power;
 info->Reset=M134Reset;
 AddExState(EXPREGS, 4, 0, "EXPR");
}

/* ---------------------------- Mapper 165 ------------------------------*/

static void M165CW(uint32 A, uint8 V)
{
 if(V==0)
    setchr4r(0x10,A,0);
 else
    setchr4(A,V>>2);
}

static void M165PPUFD(void)
{
 if(EXPREGS[0]==0xFD)
 {
  M165CW(0x0000,DRegBuf[0]);
  M165CW(0x1000,DRegBuf[2]);
 }
}

static void M165PPUFE(void)
{
 if(EXPREGS[0]==0xFE)
 {
  M165CW(0x0000,DRegBuf[1]);
  M165CW(0x1000,DRegBuf[4]);
 }
}

static void M165CWM(uint32 A, uint8 V)
{
 if(((MMC3_cmd&0x7)==0)||((MMC3_cmd&0x7)==2))
   M165PPUFD();
 if(((MMC3_cmd&0x7)==1)||((MMC3_cmd&0x7)==4))
   M165PPUFE();
}

static void M165PPU(uint32 A)
{
 if((A&0x1FF0)==0x1FD0)
 {
  EXPREGS[0]=0xFD;
  M165PPUFD();
 } else if((A&0x1FF0)==0x1FE0)
 {
  EXPREGS[0]=0xFE;
  M165PPUFE();
 }
}

static void M165Power(void)
{
 EXPREGS[0]=0xFD;
 GenMMC3Power();
}

void Mapper165_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 128, 8, info->battery);
 cwrap=M165CWM;
 PPU_hook=M165PPU;
 info->Power=M165Power;
 CHRRAMSize = 4096;
 CHRRAM = (uint8*)FCEU_gmalloc(CHRRAMSize);
 SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSize, 1);
 AddExState(CHRRAM, CHRRAMSize, 0, "CHRR");
 AddExState(EXPREGS, 4, 0, "EXPR");
}

/* ---------------------------- Mapper 191 ------------------------------*/

static void M191CW(uint32 A, uint8 V)
{
  setchr1r((V&0x80)>>3,A,V);
}

void Mapper191_Init(CartInfo *info)
{
 GenMMC3_Init(info, 256, 256, 8, info->battery);
 cwrap=M191CW;
 CHRRAMSize=2048;
 CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSize);
 SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSize, 1);
 AddExState(CHRRAM, CHRRAMSize, 0, "CHRR");
}

/* ---------------------------- Mapper 192 -------------------------------*/

static void M192CW(uint32 A, uint8 V)
{
  if((V==8)||(V==9)||(V==0xA)||(V==0xB)) /*Ying Lie Qun Xia Zhuan (Chinese),*/
    setchr1r(0x10,A,V);
  else
    setchr1r(0,A,V);
}

void Mapper192_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 cwrap=M192CW;
 CHRRAMSize=4096;
 CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSize);
 SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSize, 1);
 AddExState(CHRRAM, CHRRAMSize, 0, "CHRR");
}

/* ---------------------------- Mapper 194 -------------------------------*/

static void M194CW(uint32 A, uint8 V)
{
  if(V<=1) /*Dai-2-Ji - Super Robot Taisen (As).nes*/
    setchr1r(0x10,A,V);
  else
    setchr1r(0,A,V);
}

void Mapper194_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 cwrap=M194CW;
 CHRRAMSize=2048;
 CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSize);
 SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSize, 1);
 AddExState(CHRRAM, CHRRAMSize, 0, "CHRR");
}

/* ---------------------------- Mapper 195 -------------------------------*/
static uint8 *wramtw;
static uint16 wramsize;

static void M195CW(uint32 A, uint8 V)
{
  if(V<=3) /* Crystalis (c).nes, Captain Tsubasa Vol 2 - Super Striker (C)*/
    setchr1r(0x10,A,V);
  else
    setchr1r(0,A,V);
}

static void M195Power(void)
{
 GenMMC3Power();
 setprg4r(0x10,0x5000,0);
 SetWriteHandler(0x5000,0x5fff,CartBW);
 SetReadHandler(0x5000,0x5fff,CartBR);
}

static void M195Close(void)
{
  if(wramtw)
    free(wramtw);
  wramtw=NULL;
}

void Mapper195_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 cwrap=M195CW;
 info->Power=M195Power;
 info->Close=M195Close;
 CHRRAMSize=4096;
 CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSize);
 SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSize, 1);
 wramsize=4096;
 wramtw=(uint8*)FCEU_gmalloc(wramsize);
 SetupCartPRGMapping(0x10, wramtw, wramsize, 1);
 AddExState(CHRRAM, CHRRAMSize, 0, "CHRR");
 AddExState(wramtw, wramsize, 0, "WRAMTW");
}

/* ---------------------------- Mapper 196 -------------------------------*/
/* MMC3 board with optional command address line connection, allows to */
/* make three-four different wirings to IRQ address lines and separately to*/
/* CMD address line, Mali Boss additionally check if wiring are correct for*/
/* game */

static DECLFW(Mapper196Write)
{
  if(A >= 0xC000) {
   A=(A&0xFFFE)|((A>>2)&1)|((A>>3)&1);
   MMC3_IRQWrite(A,V); 
  }
  else {
   A=(A&0xFFFE)|((A>>2)&1)|((A>>3)&1)|((A>>1)&1);
   MMC3_CMDWrite(A,V);
  }
}

static void Mapper196Power(void)
{
  GenMMC3Power();
  SetWriteHandler(0x8000,0xFFFF,Mapper196Write);
}

void Mapper196_Init(CartInfo *info)
{
  GenMMC3_Init(info, 128, 128, 0, 0);
  info->Power=Mapper196Power;
}

/* ---------------------------- Mapper 197 -------------------------------*/

static void M197CW(uint32 A, uint8 V)
{
  if(A==0x0000)
    setchr4(0x0000,V>>1);
  else if(A==0x1000)
    setchr2(0x1000,V);
  else if(A==0x1400)
    setchr2(0x1800,V);
}

void Mapper197_Init(CartInfo *info)
{
 GenMMC3_Init(info, 128, 512, 8, 0);
 cwrap=M197CW;
}

/* ---------------------------- Mapper 198 -------------------------------*/

static void M198PW(uint32 A, uint8 V)
{
  if(V>=0x50) /* Tenchi o Kurau II - Shokatsu Koumei Den (J) (C).nes*/
    setprg8(A,V&0x4F);
  else
    setprg8(A,V);
}

void Mapper198_Init(CartInfo *info)
{
 GenMMC3_Init(info, 1024, 256, 8, info->battery);
 pwrap=M198PW;
 info->Power=M195Power;
 info->Close=M195Close;
 wramsize=4096;
 wramtw=(uint8*)FCEU_gmalloc(wramsize);
 SetupCartPRGMapping(0x10, wramtw, wramsize, 1);
 AddExState(wramtw, wramsize, 0, "WRAMTW");
}

/* ---------------------------- Mapper 205 ------------------------------*/
/* GN-45 BOARD*/

static void M205PW(uint32 A, uint8 V)
{
/* GN-30A - начальная маска должна быть 1F + аппаратный переключатель на шине адреса*/
 setprg8(A,(V&0x0f)|EXPREGS[0]);
}

static void M205CW(uint32 A, uint8 V)
{
/* GN-30A - начальная маска должна быть FF*/
 setchr1(A,(V&0x7F)|(EXPREGS[0]<<3));
}

static DECLFW(M205Write)
{
 if(EXPREGS[2] == 0) {
   EXPREGS[0] = A & 0x30;
   EXPREGS[2] = A & 0x80;
   FixMMC3PRG(MMC3_cmd);
   FixMMC3CHR(MMC3_cmd);
 }
 else
   CartBW(A,V);
}

static void M205Reset(void)
{
 EXPREGS[0]=EXPREGS[2]=0;
 MMC3RegReset();
}

static void M205Power(void)
{
 GenMMC3Power();
 SetWriteHandler(0x6000,0x6fff,M205Write);
}

void Mapper205_Init(CartInfo *info)
{
 GenMMC3_Init(info, 256, 256, 8, 0);
 pwrap=M205PW;
 cwrap=M205CW;
 info->Power=M205Power;
 info->Reset=M205Reset;
 AddExState(EXPREGS, 1, 0, "EXPR");
}

/* ---------------------------- Mapper 245 ------------------------------*/

static void M245CW(uint32 A, uint8 V)
{
 if(!UNIFchrrama) /* Yong Zhe Dou E Long - Dragon Quest VI (As).nes NEEDS THIS for RAM cart*/
  setchr1(A,V&7);
 EXPREGS[0]=V;
 FixMMC3PRG(MMC3_cmd);
}

static void M245PW(uint32 A, uint8 V)
{
 setprg8(A,(V&0x3F)|((EXPREGS[0]&2)<<5));
}

static void M245Power(void)
{
 EXPREGS[0]=0;
 GenMMC3Power();
}

void Mapper245_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 cwrap=M245CW;
 pwrap=M245PW;
 info->Power=M245Power;
 AddExState(EXPREGS, 1, 0, "EXPR");
}

/* ---------------------------- Mapper 249 ------------------------------*/

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

static DECLFW(M249Write)
{
 EXPREGS[0]=V;
 FixMMC3PRG(MMC3_cmd);
 FixMMC3CHR(MMC3_cmd);
}

static void M249Power(void)
{
 EXPREGS[0]=0;
 GenMMC3Power();
 SetWriteHandler(0x5000,0x5000,M249Write);
}

void Mapper249_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 cwrap=M249CW;
 pwrap=M249PW;
 info->Power=M249Power;
 AddExState(EXPREGS, 1, 0, "EXPR");
}

/* ---------------------------- Mapper 250 ------------------------------*/

static DECLFW(M250Write)
{
 MMC3_CMDWrite((A&0xE000)|((A&0x400)>>10),A&0xFF);
}

static DECLFW(M250IRQWrite)
{
 MMC3_IRQWrite((A&0xE000)|((A&0x400)>>10),A&0xFF);
}

static void M250_Power(void)
{
 GenMMC3Power();
 SetWriteHandler(0x8000,0xBFFF,M250Write);
 SetWriteHandler(0xC000,0xFFFF,M250IRQWrite);
}

void Mapper250_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 info->Power=M250_Power;
}

/* ---------------------------- Mapper 254 ------------------------------*/

static DECLFR(MR254WRAM)
{
  if(EXPREGS[0])
    return WRAM[A-0x6000];
  else
    return WRAM[A-0x6000]^EXPREGS[1];
}

static DECLFW(M254Write)
{
 switch (A) {
  case 0x8000: EXPREGS[0]=0xff;
               break;
  case 0xA001: EXPREGS[1]=V;
 }
 MMC3_CMDWrite(A,V);
}

static void M254_Power(void)
{
 GenMMC3Power();
 SetWriteHandler(0x8000,0xBFFF,M254Write);
 SetReadHandler(0x6000,0x7FFF,MR254WRAM);
}

void Mapper254_Init(CartInfo *info)
{
 GenMMC3_Init(info, 128, 128, 8, info->battery);
 info->Power=M254_Power;
 AddExState(EXPREGS, 2, 0, "EXPR");
}

/* ---------------------------- UNIF Boards -----------------------------*/

void TBROM_Init(CartInfo *info)
{
 GenMMC3_Init(info, 64, 64, 0, 0);
}

void TEROM_Init(CartInfo *info)
{
 GenMMC3_Init(info, 32, 32, 0, 0);
}

void TFROM_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 64, 0, 0);
}

void TGROM_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 0, 0, 0);
}

void TKROM_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
}

void TLROM_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 0, 0);
}

void TSROM_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, 0);
}

void TLSROM_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, 0);
 cwrap=TKSWRAP;
 mwrap=GENNOMWRAP;
 PPU_hook=TKSPPU;
 AddExState(&PPUCHRBus, 1, 0, "PPUC");
}

void TKSROM_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 256, 8, info->battery);
 cwrap=TKSWRAP;
 mwrap=GENNOMWRAP;
 PPU_hook=TKSPPU;
 AddExState(&PPUCHRBus, 1, 0, "PPUC");
}

void TQROM_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 64, 0, 0);
 cwrap=TQWRAP;
 CHRRAMSize=8192;
 CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSize);
 SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSize, 1);
}

void HKROM_Init(CartInfo *info)
{
 GenMMC3_Init(info, 512, 512, 1, info->battery);
}
