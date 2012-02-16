/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2011 CaH4e3
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
 * SL12 Protected 3-in-1 mapper hardware (VRC2, MMC3, MMC1)
 * the same as 603-5052 board (TODO: add reading registers, merge)
 * SL1632 2-in-1 protected board, similar to SL12 (TODO: find difference)
 *
 * Known PCB:
 *
 * Garou Densetsu Special (G0904.PCB, Huang-1, GAL dip: W conf.)
 * Kart Fighter (008, Huang-1, GAL dip: W conf.)
 * Somari (008, C5052-13, GAL dip: P conf., GK2-P/GK2-V maskroms)
 * Somari (008, Huang-1, GAL dip: W conf., GK1-P/GK1-V maskroms)
 * AV Mei Shao Nv Zhan Shi ()
 * Samurai Spirits (Full version) (Huang-1, GAL dip: unk conf. GS-2A/GS-4A maskroms)
 * Contra Fighter (603-5052 board, C5052-3, GAL dip: unk conf. SC603-A/SCB603-B maskroms)
 *
 */

#include "mapinc.h"

static uint8 mode;
static uint8 vrc2_chr[8], vrc2_prg[2], vrc2_mirr;
static uint8 mmc3_regs[10], mmc3_ctrl, mmc3_mirr;
static uint8 IRQCount,IRQLatch,IRQa;
static uint8 IRQReload;
static uint8 mmc1_regs[4], mmc1_buffer, mmc1_shift;

static SFORMAT StateRegs[]=
{
  {&mode, 1, "MODE"},
  {vrc2_chr, 8, "VRC2CHR"},
  {vrc2_prg, 2, "VRC2PRG"},
  {&vrc2_mirr, 1, "VRC2MIR"},
  {mmc3_regs, 10, "MMC3REGS"},
  {&mmc3_ctrl, 1, "MMC3CTRL"},
  {&mmc3_mirr, 1, "MMC3MIR"},
  {&IRQReload, 1, "IRQR"},
  {&IRQCount, 1, "IRQC"},
  {&IRQLatch, 1, "IRQL"},
  {&IRQa, 1, "IRQA"},
  {mmc1_regs, 4, "MMC1REGS"},
  {&mmc1_buffer, 1, "MMC1BUF"},
  {&mmc1_shift, 1, "MMC1MIR"},
  {0}
};

static void SyncPRG(void)
{
  switch(mode & 3) {
   case 0:
     setprg8(0x8000, vrc2_prg[0]);
     setprg8(0xA000, vrc2_prg[1]);
     setprg8(0xC000, ~1);
     setprg8(0xE000, ~0);
     break;
   case 1: {
     uint32 swap = (mmc3_ctrl >> 5) & 2;
     setprg8(0x8000, mmc3_regs[6 + swap]);
     setprg8(0xA000, mmc3_regs[7]);
     setprg8(0xC000, mmc3_regs[6 + (swap ^ 2)]);
     setprg8(0xE000, mmc3_regs[9]);
     break;
   }
   case 2:
   case 3: {
     uint8 bank = mmc1_regs[3] & 0xF;
 	 if(mmc1_regs[0] & 8)
     {
       if(mmc1_regs[0] & 4)
       {
         setprg16(0x8000, bank);
         setprg16(0xC000, 0x0F);
       }
       else
       {
         setprg16(0x8000, 0);
         setprg16(0xC000, bank);
       }
     }
	 else
	   setprg32(0x8000, bank >> 1);
     break;
   }
  }
}

static void SyncCHR(void)
{
  uint32 base = (mode & 4) << 6;
  switch(mode & 3) {
   case 0:
     setchr1(0x0000, base|vrc2_chr[0]);
     setchr1(0x0400, base|vrc2_chr[1]);
     setchr1(0x0800, base|vrc2_chr[2]);
     setchr1(0x0c00, base|vrc2_chr[3]);
     setchr1(0x1000, base|vrc2_chr[4]);
     setchr1(0x1400, base|vrc2_chr[5]);
     setchr1(0x1800, base|vrc2_chr[6]);
     setchr1(0x1c00, base|vrc2_chr[7]);
     break;
   case 1: {
     uint32 swap = (mmc3_ctrl & 0x80) << 5;
     setchr1(0x0000 ^ swap, base|((mmc3_regs[0])&0xFE));
     setchr1(0x0400 ^ swap, base|(mmc3_regs[0]|1));
     setchr1(0x0800 ^ swap, base|((mmc3_regs[1])&0xFE));
     setchr1(0x0c00 ^ swap, base|(mmc3_regs[1]|1));
     setchr1(0x1000 ^ swap, base|mmc3_regs[2]);
     setchr1(0x1400 ^ swap, base|mmc3_regs[3]);
     setchr1(0x1800 ^ swap, base|mmc3_regs[4]);
     setchr1(0x1c00 ^ swap, base|mmc3_regs[5]);
     break;
   }
   case 2:
   case 3:
     if(mmc1_regs[0]&0x10)
     {
       setchr4(0x0000, mmc1_regs[1]);
       setchr4(0x1000, mmc1_regs[2]);
     }
     else
       setchr8(mmc1_regs[1] >> 1);
     break;
  }
}

static void SyncMIR(void)
{
  switch(mode & 3) {
   case 0: {
     setmirror((vrc2_mirr&1)^1);
     break;
   }
   case 1: {
     setmirror((mmc3_mirr&1)^1);
     break;
   }
   case 2:
   case 3: {
     switch(mmc1_regs[0]&3) {
       case 0: setmirror(MI_0); break;
       case 1: setmirror(MI_1); break;
       case 2: setmirror(MI_V); break;
       case 3: setmirror(MI_H); break;
     }
     break;
   }
  }
}

static void Sync(void)
{
  SyncPRG();
  SyncCHR();
  SyncMIR();
}

static DECLFW(UNLSL12ModeWrite)
{
/*  FCEU_printf("%04X:%02X\n",A,V);*/
  if((A & 0x4100) == 0x4100) {
    mode = V;
    if(A&1) { /* hacky hacky, there are two configuration modes on SOMARI HUANG-1 PCBs*/
              /* Solder pads with P1/P2 shorted called SOMARI P,*/
              /* Solder pads with W1/W2 shorted called SOMARI W*/
              /* Both identical 3-in-1 but W wanted MMC1 registers*/
              /* to be reset when switch to MMC1 mode P one - doesn't*/
              /* There is issue with W version of Somari at starting copyrights*/
      mmc1_regs[0] = 0xc;
      mmc1_regs[3] = 0;
      mmc1_buffer = 0;
      mmc1_shift = 0;
    }
    Sync();
  }
}

static DECLFW(UNLSL12Write)
{
/*  FCEU_printf("%04X:%02X\n",A,V);*/
  switch(mode & 3) {
   case 0: {
     if((A>=0xB000)&&(A<=0xE003))
     {
       int32 ind=((((A&2)|(A>>10))>>1)+2)&7;
       int32 sar=((A&1)<<2);
       vrc2_chr[ind]=(vrc2_chr[ind]&(0xF0>>sar))|((V&0x0F)<<sar);
       SyncCHR();
     }
     else
       switch(A&0xF000) {
        case 0x8000: vrc2_prg[0] = V; SyncPRG(); break;
        case 0xA000: vrc2_prg[1] = V; SyncPRG(); break;
        case 0x9000: vrc2_mirr = V; SyncMIR(); break;
       }
     break;
   }
   case 1: {
     switch(A & 0xE001) {
      case 0x8000: {
        uint8 old_ctrl = mmc3_ctrl;
        mmc3_ctrl = V;
        if((old_ctrl&0x40) != (mmc3_ctrl&0x40))
          SyncPRG();
        if((old_ctrl&0x80) != (mmc3_ctrl&0x80))
          SyncCHR();
        break;
      }
      case 0x8001:
        mmc3_regs[mmc3_ctrl & 7] = V;
        if((mmc3_ctrl & 7) < 6)
          SyncCHR();
        else
          SyncPRG();
        break;
      case 0xA000:
        mmc3_mirr = V;
        SyncMIR();
        break;
      case 0xC000:
        IRQLatch = V;
        break;
      case 0xC001:
        IRQReload = 1;
        break;
      case 0xE000:
        X6502_IRQEnd(FCEU_IQEXT);
        IRQa=0;
        break;
      case 0xE001:
        IRQa=1;
        break;
     }
     break;
   }
   case 2:
   case 3: {
     if(V & 0x80)
     {
       mmc1_regs[0] |= 0xc;
       mmc1_buffer = mmc1_shift = 0;
       SyncPRG();
     }
     else
     {
       uint8 n = (A >> 13) - 4;
       mmc1_buffer |=  (V & 1) << (mmc1_shift++);
       if(mmc1_shift == 5)
       {
         mmc1_regs[n] = mmc1_buffer;
         mmc1_buffer = mmc1_shift = 0;
         switch(n) {
          case 0: SyncMIR();
          case 2: SyncCHR(); 
          case 3:
          case 1: SyncPRG();
         }
       }
     }
     break;
   }
  }
}

static void UNLSL12HBIRQ(void)
{
  if((mode & 3) == 1)
  {
    int32 count = IRQCount;
    if(!count || IRQReload)
    {
      IRQCount = IRQLatch;
      IRQReload = 0;
    }
    else
      IRQCount--;
    if(!IRQCount)
    {
      if(IRQa)
        X6502_IRQBegin(FCEU_IQEXT);
    }
  }
}

static void StateRestore(int version)
{
  Sync();
}

static void UNLSL12Power(void)
{
  mode = 0;
  vrc2_chr[0] = ~0;
  vrc2_chr[1] = ~0;
  vrc2_chr[2] = ~0;
  vrc2_chr[3] = ~0; /* W conf. of Somari wanted CHR3 has to be set to BB bank (or similar), but doesn't do that directly*/
  vrc2_chr[4] = 4;
  vrc2_chr[5] = 5;
  vrc2_chr[6] = 6;
  vrc2_chr[7] = 7;
  vrc2_prg[0] = 0;
  vrc2_prg[1] = 1;
  vrc2_mirr = 0;
  mmc3_regs[0] = 0;
  mmc3_regs[1] = 2;
  mmc3_regs[2] = 4;
  mmc3_regs[3] = 5;
  mmc3_regs[4] = 6;
  mmc3_regs[5] = 7;
  mmc3_regs[6] = ~3;
  mmc3_regs[7] = ~2;
  mmc3_regs[8] = ~1;
  mmc3_regs[9] = ~0;
  mmc3_ctrl = mmc3_mirr = IRQCount = IRQLatch = IRQa = 0;
  mmc1_regs[0] = 0xc;
  mmc1_regs[1] = 0;
  mmc1_regs[2] = 0;
  mmc1_regs[3] = 0;
  mmc1_buffer = 0;
  mmc1_shift = 0;
  Sync();
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x4100,0x7FFF,UNLSL12ModeWrite);
  SetWriteHandler(0x8000,0xFFFF,UNLSL12Write);
}

void UNLSL12_Init(CartInfo *info)
{
  info->Power = UNLSL12Power;
  GameHBIRQHook = UNLSL12HBIRQ;
  GameStateRestore = StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
