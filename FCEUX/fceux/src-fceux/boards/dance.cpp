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
 * 
 * Street Dance (Dance pad) (Unl)
 */
 
#include "mapinc.h"

static uint8 reg4[16];
static uint8 regc[6];
static uint8 reg2000, mmc3cmd, pcm_enable = 0, pcm_irq = 0;
static int16 pcm_addr, pcm_size, pcm_latch, pcm_clock = 0xF6;
static writefunc old4011write, old4012write, old4013write, old4015write;
static readfunc old4015read;

static SFORMAT StateRegs[]=
{
  {reg4, 16, "reg4"},
  {regc, 6, "REGSC"},
  {&reg2000, 1, "REGS2"},
  {&pcm_enable, 1, "PCME"},
  {&pcm_irq, 1, "PCMIRQ"},
  {&pcm_addr, 2, "PCMADDR"},
  {&pcm_size, 2, "PCMSIZE"},
  {&pcm_latch, 2, "PCMLATCH"},
  {&pcm_clock, 2, "PCMCLOCK"},
  {&mmc3cmd, 1, "MMC3CMD"},
  {0}
};

static void Sync(void)
{
  uint8 cbase = reg2000 - ((reg4[0x0B]&4)?6:0);
  uint8 pbase = reg4[0x09] & 0xC0;
  
  setchr1(0x0000,cbase|(regc[0]&(~1)));  
  setchr1(0x0400,cbase|(regc[0]|1));  
  setchr1(0x0800,cbase|(regc[1]&(-1)));  
  setchr1(0x0c00,cbase|(regc[1]|1));  
  setchr1(0x1000,cbase|regc[2]);  
  setchr1(0x1400,cbase|regc[3]);  
  setchr1(0x1800,cbase|regc[4]);  
  setchr1(0x1c00,cbase|regc[5]);
  
  
  if(reg4[0x0B]&1)
  {
    setprg8(0x8000,reg4[0x07] + 0x20);
    setprg8(0xA000,reg4[0x08] + 0x20);
  }
  else
  {
    setprg8(0x8000,reg4[0x07] + pbase);
    setprg8(0xA000,reg4[0x08] + pbase);
  }

  setprg8(0xC000,reg4[0x09]);
  setprg8(0xE000,reg4[0x0A]);
}
      
static DECLFW(UNLDANCEWrite2)
{
  reg2000 = V;
  Sync();
  //FCEU_printf("write %04x:%04x\n",A,V);
}

static DECLFW(UNLDANCEWrite4)
{
  reg4[A & 0x0F] = V;
  Sync();
  //FCEU_printf("write %04x:%04x\n",A,V);
}

static DECLFW(UNLDANCEWrite8)
{
  if(A&1)
  {
     if(mmc3cmd<6)
       regc[mmc3cmd] = V;
     else
       reg4[0x07 + mmc3cmd - 6] = V;
  }
  else
      mmc3cmd = V & 7;
  Sync();
  //FCEU_printf("write %04x:%04x\n",A,V);
}

static DECLFW(UNLDANCEWrite4012)
{
  pcm_addr = V << 6;
  old4012write(A,V);
}

static DECLFW(UNLDANCEWrite4013)
{
  pcm_size = (V << 4) + 1;
  old4013write(A,V);
}

static DECLFW(UNLDANCEWrite4015)
{
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

static DECLFR(UNLDANCERead4015)
{
  return (old4015read(A) & 0x7F) | pcm_irq;
}

static void UNLDANCECpuHook(int a)
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

static void UNLDANCEPower(void)
{
  reg4[0x09]=0x3E;
  reg4[0x0A]=0x3F;
  SetupCartCHRMapping(0,PRGptr[0],512 * 1024,0);

  old4015read=GetReadHandler(0x4015);
  SetReadHandler(0x4015,0x4015,UNLDANCERead4015);
  SetReadHandler(0x8000,0xFFFF,CartBR);

  old4011write=GetWriteHandler(0x4011);
  old4012write=GetWriteHandler(0x4012);
  SetWriteHandler(0x4012,0x4012,UNLDANCEWrite4012);
  old4013write=GetWriteHandler(0x4013);
  SetWriteHandler(0x4013,0x4013,UNLDANCEWrite4013);
  old4015write=GetWriteHandler(0x4015);
  SetWriteHandler(0x4015,0x4015,UNLDANCEWrite4015);

  SetWriteHandler(0x201A,0x201A,UNLDANCEWrite2);
  SetWriteHandler(0x4100,0x410F,UNLDANCEWrite4);
  SetWriteHandler(0x8000,0x8001,UNLDANCEWrite8);
  Sync();
}

static void UNLDANCEReset(void)
{
  reg4[0x09]=0x3E;
  reg4[0x0A]=0x3F;
  Sync();
}

static void StateRestore(int version)
{
  Sync();
}

static void UNLDANCE_Init(CartInfo *info)
{
  info->Power=UNLDANCEPower;
  info->Reset=UNLDANCEReset;
  MapIRQHook=UNLDANCECpuHook;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
