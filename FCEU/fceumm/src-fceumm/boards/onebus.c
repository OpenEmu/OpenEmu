/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2007-2010 CaH4e3
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
 *
 * OneBus system
 * Street Dance (Dance pad) (Unl)
 * 101-in-1 Arcade Action II
 * DreamGEAR 75-in-1
 */

#include "mapinc.h"

static uint8 isDance;
static uint8 regs[16],regc[6];
static uint8 IRQCount,IRQLatch,IRQa, IRQReload, pcm_enable = 0, pcm_irq = 0;
static int16 pcm_addr, pcm_size, pcm_latch, pcm_clock = 0xF6;
static writefunc old4011write, old4012write, old4013write, old4015write;
static readfunc old4015read;

static SFORMAT StateRegs[]=
{
  {regc, 6, "REGC"},
  {regs, 16, "REGS"},
  {&IRQReload, 1, "IRQR"},
  {&IRQCount, 1, "IRQC"},
  {&IRQLatch, 1, "IRQL"},
  {&IRQa, 1, "IRQA"},
  {&pcm_enable, 1, "PCME"},
  {&pcm_irq, 1, "PCMIRQ"},
  {&pcm_addr, 2, "PCMADDR"},
  {&pcm_size, 2, "PCMSIZE"},
  {&pcm_latch, 2, "PCMLATCH"},
  {&pcm_clock, 2, "PCMCLOCK"},
  {0}
};

static void Sync(void)
{
  uint16 cswap = (regs[0xf] & 0x80) << 5;
  uint16 pswap = (regs[0xd]&1)?((regs[0xf] & 0x40) << 8):0;
  uint16 pbase = (regs[0]&0xf0)<<4;
  uint16 cbase = (((regs[0]&0x0f)<<8)|(regs[0xc]<<1)|((regs[0xd]&0xf8)>>3))<<3;
  uint16 pmask = 0x3f>>(regs[0xb]&0xf);

  setchr1(cswap^0x0000,cbase|(regc[0]&(~1)));
  setchr1(cswap^0x0400,cbase|(regc[0]|1));
  setchr1(cswap^0x0800,cbase|(regc[1]&(-1)));
  setchr1(cswap^0x0c00,cbase|(regc[1]|1));
  setchr1(cswap^0x1000,cbase|(regc[2]));
  setchr1(cswap^0x1400,cbase|(regc[3]));
  setchr1(cswap^0x1800,cbase|(regc[4]));
  setchr1(cswap^0x1c00,cbase|(regc[5]));

  if(regs[0xd]&2)
  {
    setprg8(pswap^0x8000, pbase|(regs[0x7]&pmask)|(regs[0xa]&(~pmask)));
    setprg8(      0xA000, pbase|(regs[0x8]&pmask)|(regs[0xa]&(~pmask)));
    setprg8(pswap^0xC000, pbase|(regs[0x9]&pmask)|(regs[0xa]&(~pmask)));
    setprg8(      0xE000, pbase|regs[0xa]);
  }
  else
  {
    setprg8(pswap^0x8000, pbase|(regs[0x7]&pmask)|(regs[0xa]&(~pmask)));
    setprg8(      0xA000, pbase|(regs[0x8]&pmask)|(regs[0xa]&(~pmask)));
    setprg8(pswap^0xC000, pbase|((~1)&pmask)|(regs[0xa]&(~pmask)));
    setprg8(      0xE000, pbase|((~0)&pmask)|(regs[0xa]&(~pmask)));
  }

  setmirror(regs[0xe]);
}

static DECLFW(UNLOneBusWrite20XX)
{
/*  FCEU_printf("PPU %04x:%04x\n",A,V);*/
  if(A == 0x201A)
    regs[0xd] = V;
  else if(A == 0x2018)
    regs[0xc] = V;
  Sync();
}

static DECLFW(UNLOneBusWriteExp)
{
/*  FCEU_printf("EXP %04x:%04x\n",A,V);*/
/*  switch(A & 0x0F)*/
/*  {*/
/*  case 2: pcm_latch = pcm_clock; FCEU_printf("write %04x:%04x\n",A,V); break;*/
/*  case 3: pcm_irqa = 0; X6502_IRQEnd(FCEU_IQEXT); pcm_irq = 0; FCEU_printf("write %04x:%04x\n",A,V); break;*/
/*  case 4: pcm_irqa = 1; FCEU_printf("write %04x:%04x\n",A,V); break;*/
/*  default:*/
   regs[A & 0x0F] = V;
   Sync();
/*  }*/
}

static DECLFW(UNLOneBusWriteDebug)
{
/*  FCEU_printf("write %04x:%04x\n",A,V);*/
}

static DECLFW(UNLOneBusWriteMMC)
{
/*  FCEU_printf("MMC %04x:%04x\n",A,V);*/
  switch(A&0xE001)
  {
  case 0x8000: regs[0xf] = V; Sync(); break;
  case 0x8001:
  {
    uint8 mask = 0xff, mmc3cmd = regs[0xf]&7;
    switch(mmc3cmd)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      if(regs[0xd]&4)
        mask = 0x0f;
      else
        mask >>= ((regs[0xb]&0xf0)>>4);
      regc[mmc3cmd] = V&mask;
      break;
    case 6:
    case 7:
      mask = (mask&0x3f)>>(regs[0xb]&0xf);
      regs[mmc3cmd+1] = (regs[mmc3cmd+1]&(~mask))|(V&mask);
      break;
    }

    Sync();
    break;
  }
  case 0xA000: regs[0xe] = (V & 1)^1; Sync(); break;
  case 0xC000: IRQLatch = V&0xfe; break;
  case 0xC001: IRQReload = 1; break;
  case 0xE000: X6502_IRQEnd(FCEU_IQEXT); IRQa = 0; break;
  case 0xE001: IRQa = 1; break;
  }
}

static void UNLOneBusIRQHook(void)
{
 int count = IRQCount;
 if(!count || IRQReload)
 {
    IRQCount = IRQLatch;
    IRQReload = 0;
 }
 else
    IRQCount--;
 if(count && !IRQCount)
 {
    if(IRQa)
       X6502_IRQBegin(FCEU_IQEXT);
 }
}

static DECLFW(UNLOneBusWriteAPU2)
{
/*  FCEU_printf("APU2 %04x:%04x\n",A,V);*/
   CartBW(A&0xffdf,V);
}

static DECLFW(UNLOneBusWrite4012)
{
/*  FCEU_printf("write %04x:%04x\n",A,V);*/
  pcm_addr = V << 6;
  old4012write(A,V);
}

static DECLFW(UNLOneBusWrite4013)
{
/*  FCEU_printf("write %04x:%04x\n",A,V);*/
  pcm_size = (V << 4) + 1;
  old4013write(A,V);
}

static DECLFW(UNLOneBusWrite4015)
{
/*  FCEU_printf("write %04x:%04x\n",A,V);*/
  pcm_enable = V&0x10;
  if(pcm_irq)
  {
    X6502_IRQEnd(FCEU_IQEXT);
    pcm_irq = 0;
  }
  if(pcm_enable)
    pcm_latch = pcm_clock;
  old4015write(A,V&0xEF);
}

static DECLFR(UNLOneBusRead4015)
{
  uint8 result = (old4015read(A) & 0x7F)|pcm_irq;
/*  FCEU_printf("read %04x, %02x\n",A,result);*/
  return result;
}

static void UNLOneBusCpuHook(int a)
{
  if(pcm_enable)
  {
    pcm_latch-=a;
    if(pcm_latch<=0)
    {
  	  pcm_latch+=pcm_clock;
  	  pcm_size--;
	  if(pcm_size<0)
	  {
	    pcm_irq = 0x80;
		pcm_enable = 0;
	    X6502_IRQBegin(FCEU_IQEXT);
 	  }
	  else
 	  {
	    uint8 raw_pcm = ARead[pcm_addr](pcm_addr) >> 1;
	    old4011write(0x4011,raw_pcm);
		pcm_addr++;
		pcm_addr&=0x7FFF;
	  }
    }
  }
}

static void UNLOneBusPower(void)
{
  IRQCount=IRQLatch=IRQa==0;
  regs[0]=regs[1]=regs[1]=regs[2]=regs[3]=regs[4]=regs[5]=regs[6]=0;
  regs[7]=regs[8]=regs[11]=regs[12]=regs[13]=regs[14]=regs[15]=0;
  regs[0x09]=0x3E;
  regs[0x0A]=0x3F;

  SetupCartCHRMapping(0,PRGptr[0],4096 * 1024,0);

  if(isDance) /* quick workaround, TODO: figure out how it works together*/
  {
    old4015read=GetReadHandler(0x4015);
    SetReadHandler(0x4015,0x4015,UNLOneBusRead4015);
    old4011write=GetWriteHandler(0x4011);
    old4012write=GetWriteHandler(0x4012);
    SetWriteHandler(0x4012,0x4012,UNLOneBusWrite4012);
    old4013write=GetWriteHandler(0x4013);
    SetWriteHandler(0x4013,0x4013,UNLOneBusWrite4013);
    old4015write=GetWriteHandler(0x4015);
    SetWriteHandler(0x4015,0x4015,UNLOneBusWrite4015);
  }

  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x2009,0x2fff,UNLOneBusWrite20XX);
/*  SetWriteHandler(0x4020,0xffff,UNLOneBusWriteDebug);*/
/*  SetWriteHandler(0x4020,0x4040,UNLOneBusWriteAPU2);*/
  SetWriteHandler(0x4100,0x410f,UNLOneBusWriteExp);
  SetWriteHandler(0x8000,0xefff,UNLOneBusWriteMMC);
  Sync();
}

static void UNLOneBusReset(void)
{
  IRQCount=IRQLatch=IRQa=0;
  regs[0]=regs[1]=regs[1]=regs[2]=regs[3]=regs[4]=regs[5]=regs[6]=0;
  regs[7]=regs[8]=regs[11]=regs[12]=regs[13]=regs[14]=regs[15]=0;
  regs[0x09]=0x3E;
  regs[0x0A]=0x3F;
  Sync();
}

static void StateRestore(int version)
{
  Sync();
}

void UNLOneBus_Init(CartInfo *info)
{
  isDance = 0;
  info->Power=UNLOneBusPower;
  info->Reset=UNLOneBusReset;
  GameHBIRQHook=UNLOneBusIRQHook;
/*  MapIRQHook=UNLOneBusCpuHook;*/
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}

void UNLDANCE_Init(CartInfo *info)
{
  isDance = 1;
  info->Power=UNLOneBusPower;
  info->Reset=UNLOneBusReset;
  GameHBIRQHook=UNLOneBusIRQHook;
  MapIRQHook=UNLOneBusCpuHook;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
