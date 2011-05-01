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

static uint8 latches[2];
static uint8 CHRBanks[4];
static uint8 Mirroring;
static uint8 PRGBank;

static void latchcheck(uint32 VAddr)
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
       setchr4(0x0000, CHRBanks[0]);
       latches[0] = 0;
      }
      else if(l==0xE8)
      {
       setchr4(0x0000, CHRBanks[1]);
       latches[0] = 1;
      }
     }
     else
     {
      if(l==0xD8)
      {
       setchr4(0x1000, CHRBanks[2]);
       latches[1] = 0;
      }
      else if(l==0xE8)
      {
       setchr4(0x1000, CHRBanks[3]);
       latches[1] = 1;
      }
     }
}

static DECLFW(MMC2_PRG_Write)	// $Axxx
{
 PRGBank = V & 0xF;
 setprg8(0x8000, PRGBank);
}

static DECLFW(MMC4_PRG_Write)
{
 PRGBank = V & 0xF;
 setprg16(0x8000, PRGBank);
}

static void CHRSync(void)
{
	if(latches[0] == 0)
	 setchr4(0x0000, CHRBanks[0]);
	else
	 setchr4(0x0000, CHRBanks[1]);

	if(latches[1] == 0)
	 setchr4(0x1000, CHRBanks[2]);
	else
	 setchr4(0x1000, CHRBanks[3]);
}

static DECLFW(CommonWrite)
{
       switch(A&0xF000)
       {
        case 0xB000:
		CHRBanks[0] = V & 0x1F;
		CHRSync();
                break;
        case 0xC000:
		CHRBanks[1] = V & 0x1F;
		CHRSync();
                break;
        case 0xD000:
		CHRBanks[2] = V & 0x1F;
		CHRSync();
                break;
        case 0xE000:
		CHRBanks[3] = V & 0x1F;
		CHRSync();
		break;
        case 0xF000:
		setmirror((V & 1) ^ 1);
		Mirroring = V & 1;
                break;
        }
}

static void MMC2_Power(CartInfo *info)
{
 int x;

 latches[0] = latches[1] = 1;
 for(x = 0; x < 4; x++)
  CHRBanks[x] = 0x1F;
 CHRSync();

 PRGBank = 0;

 setprg8(0x8000, 0x00);
 setprg8(0xA000, 0x0D);
 setprg8(0xC000, 0x0E);
 setprg8(0xE000, 0x0F);
}

static int MMC2_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAYN(CHRBanks, 4, "CHRBanks"),
  SFVARN(PRGBank, "PRGBank"),
  SFARRAYN(latches, 2, "latches"),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MMC2");

 if(load)
 {
  CHRSync();
  setmirror(Mirroring ^ 1);
  setprg8(0x8000, PRGBank);  
 }

 return(ret);
}

// MMC2, also covers PEEOROM
int PNROM_Init(CartInfo *info)
{
 info->Power = MMC2_Power;
 info->StateAction = MMC2_StateAction;
 SetWriteHandler(0xA000,0xAFFF, MMC2_PRG_Write);
 SetWriteHandler(0xB000,0xFFFF, CommonWrite);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 PPU_hook = latchcheck;

 return(1);
}

static uint8 MMC4_WRAM[8192];

static void MMC4_Power(CartInfo *info)
{
 int x;

 latches[0] = latches[1] = 1;
 for(x = 0; x < 4; x++)
  CHRBanks[x] = 0x1F;
 CHRSync();
 PRGBank = 0;
 setprg16(0x8000, 0);
 setprg16(0xc000, 0xF);

 setprg8r(0x10, 0x6000, 0);

 if(!info->battery)
  memset(MMC4_WRAM, 0xFF, 8192);
}

static int MMC4_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAYN(CHRBanks, 4, "CHRBanks"),
  SFVARN(PRGBank, "PRGBank"),
  SFARRAYN(latches, 2, "latches"),
  SFARRAYN(MMC4_WRAM, 8192, "WRAM"),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MMC2");
 if(load)
 {
  CHRSync();
  setmirror(Mirroring ^ 1);
  setprg16(0x8000, PRGBank);
 }

 return(ret);
}

int MMC4_Init(CartInfo *info)
{
 info->StateAction = MMC4_StateAction;
 info->Power = MMC4_Power;
 SetWriteHandler(0xA000, 0xAFFF, MMC4_PRG_Write);
 SetWriteHandler(0xB000, 0xFFFF, CommonWrite);
 SetReadHandler(0x6000, 0xFFFF, CartBR);
 SetWriteHandler(0x6000, 0x7FFF, CartBW);
 SetupCartPRGMapping(0x10, MMC4_WRAM, 8192, 1);


 info->SaveGame[0] = MMC4_WRAM;
 info->SaveGameLen[0] = 8192;

 PPU_hook = latchcheck;
 return(1);
}
